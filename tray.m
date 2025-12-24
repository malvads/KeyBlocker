/**
 * @file tray.m
 * @brief Implementation of the macOS system tray (menu bar) icon and menu.
 *
 * This file creates the status bar item, builds a custom menu with controls
 * (blocking switch, shortcut enable switch, and shortcut recording button),
 * and hooks into the application's lifecycle. It also integrates with the
 * keyboard event tap and shortcut recording subsystems via callbacks.
 */
#import <Cocoa/Cocoa.h>
#import <Carbon/Carbon.h>
#import <CoreServices/CoreServices.h>
#include "tray.h"
#include "keyboard.h"
#include "logger.h"
#include "version.h"

/**
 * @interface StatusBarDelegate
 * @brief Delegate for handling status bar menu actions.
 *
 * The delegate provides target/action methods for the custom views in the
 * status bar menu (switches and buttons) and helpers for updating UI text.
 */
@interface StatusBarDelegate : NSObject

/**
 * @brief Action invoked when the blocking switch is toggled.
 *
 * @param sender The NSSwitch that triggered the action.
 */
- (void)switchAction:(id)sender;

/**
 * @brief Action invoked when the shortcut-enable switch is toggled.
 *
 * @param sender The NSSwitch that triggered the action.
 */
- (void)shortcutSwitchAction:(id)sender;

/**
 * @brief Action invoked when the user clicks the record shortcut button.
 *
 * Begins recording a new unlock shortcut.
 *
 * @param sender The NSButton that triggered the action.
 */
- (void)recordShortcutAction:(id)sender;

/**
 * @brief Action invoked when the Quit menu item is selected.
 *
 * Terminates the application.
 *
 * @param sender The menu item that triggered the action.
 */
- (void)quitAction:(id)sender;

/**
 * @brief Updates the title and state of the shortcut record button.
 *
 * Should be called on the main thread when the recorded shortcut changes.
 */
- (void)updateShortcutButton;

/**
 * @brief Performs an asynchronous update check and updates the UI.
 */
- (void)checkForUpdates;

/**
 * @brief Opens the GitHub repository URL in the default browser.
 */
- (void)openVersionLink:(id)sender;
@end

/**
 * @interface AppDelegate
 * @brief Delegate for handling application lifecycle events.
 */
@interface AppDelegate : NSObject <NSApplicationDelegate>
@end

/**
 * @brief Global status bar item used to hold the menu.
 *
 * Owned by the application; used by update_tray_state and during setup.
 */
static NSStatusItem *statusItem;

/**
 * @brief Global instance of the StatusBarDelegate.
 *
 * Stored so actions set on UI controls can target a valid object.
 */
static StatusBarDelegate *trayDelegate;

/**
 * @brief Global instance of the AppDelegate.
 *
 * Stored so the application delegate is retained for the lifetime of the app.
 */
static AppDelegate *appDelegate;

/**
 * @brief Callback invoked by the keyboard/shortcut subsystem when a new
 * recorded shortcut is available.
 *
 * This is invoked from a background context, therefore it marshals a call to
 * the tray delegate's -updateShortcutButton method on the main thread and
 * requests the update in multiple runloop modes so the UI reflects changes
 * even during event tracking.
 *
 * @param flags The event modifier flags for the recorded shortcut.
 * @param keyCode The hardware key code for the recorded shortcut.
 */
void recorded_shortcut_callback(unsigned long long flags, unsigned short keyCode) {
    log_message(KB_LOG_LEVEL_INFO, "Background callback triggered. Updating UI in all runloop modes...");
    if (trayDelegate) {
        [trayDelegate performSelectorOnMainThread:@selector(updateShortcutButton) 
                                       withObject:nil 
                                    waitUntilDone:NO 
                                            modes:@[NSRunLoopCommonModes, NSEventTrackingRunLoopMode]];
    } else {
        log_message(KB_LOG_LEVEL_ERROR, "trayDelegate is nil in background callback!");
    }
}

/**
 * @brief Switch UI element that reflects/controls the keyboard blocking state.
 *
 * This is created and retained by the tray setup code and updated by
 * update_tray_state when the block state changes.
 */
static NSSwitch *blockingSwitch;

/**
 * @brief Switch UI element that enables/disables the unlock shortcut.
 */
static NSSwitch *shortcutSwitch;

/**
 * @brief Button UI element used for starting the shortcut recording flow and
 * displaying the currently recorded shortcut.
 */
static NSButton *recordButton;

/**
 * @brief Menu item displaying the current version or update status.
 */
static NSMenuItem *versionMenuItem;

/**
 * @brief Implementation of StatusBarDelegate.
 */
@implementation StatusBarDelegate

- (void)switchAction:(id)sender {
    if ([sender isKindOfClass:[NSSwitch class]]) {
        NSSwitch *sw = (NSSwitch *)sender;
        bool newState = (sw.state == NSControlStateValueOn);
        enableKeyboardBlock(newState);
        update_tray_state(newState);
    }
}

- (void)shortcutSwitchAction:(id)sender {
    if ([sender isKindOfClass:[NSSwitch class]]) {
        NSSwitch *sw = (NSSwitch *)sender;
        setShortcutEnabled(sw.state == NSControlStateValueOn);
    }
}

/**
 * @brief Build a human readable string representing the currently recorded
 * shortcut (modifiers + key).
 *
 * The string uses common macOS symbols for modifier keys. It attempts to
 * translate the recorded keyCode into a printable character using the
 * current keyboard layout; if no printable character can be produced the
 * numeric key code is shown in brackets.
 *
 * @return An autoreleased NSString describing the shortcut (e.g. "⌃⌥A").
 */
- (NSString *)stringForShortcut {
    unsigned long long flags = 0;
    unsigned short keyCode = 0;
    getShortcut(&flags, &keyCode);
    
    NSMutableString *s = [NSMutableString string];
    if (flags & kCGEventFlagMaskControl) [s appendString:@"⌃"];
    if (flags & kCGEventFlagMaskAlternate) [s appendString:@"⌥"];
    if (flags & kCGEventFlagMaskShift) [s appendString:@"⇧"];
    if (flags & kCGEventFlagMaskCommand) [s appendString:@"⌘"];
    
    TISInputSourceRef currentKeyboard = TISCopyCurrentKeyboardLayoutInputSource();
    CFDataRef layoutData = (CFDataRef)TISGetInputSourceProperty(currentKeyboard, kTISPropertyUnicodeKeyLayoutData);
    const UCKeyboardLayout *keyboardLayout = (const UCKeyboardLayout *)CFDataGetBytePtr(layoutData);

    UInt32 deadKeyState = 0;
    UniChar characters[4];
    UniCharCount actualLength;

    CGEventSourceRef source = CGEventSourceCreate(kCGEventSourceStateCombinedSessionState);
    UCKeyTranslate(keyboardLayout,
                   keyCode,
                   kUCKeyActionDisplay,
                   0,
                   (UInt32)CGEventSourceGetKeyboardType(source),
                   kUCKeyTranslateNoDeadKeysBit,
                   &deadKeyState,
                   sizeof(characters) / sizeof(characters[0]),
                   &actualLength,
                   characters);
    if (source) CFRelease(source);
    
    CFRelease(currentKeyboard);

    if (actualLength > 0) {
        NSString *keyStr = [NSString stringWithCharacters:characters length:actualLength];
        [s appendString:[keyStr uppercaseString]];
    } else {
        [s appendFormat:@"[%hu]", keyCode];
    }

    return s;
}

- (void)updateShortcutButton {
    log_message(KB_LOG_LEVEL_INFO, "Updating shortcut button UI.");
    if (recordButton) {
        NSString *newTitle = [self stringForShortcut];
        log_message(KB_LOG_LEVEL_INFO, "New shortcut title: %s", [newTitle UTF8String]);
        [recordButton setEnabled:YES];
        [recordButton setTitle:newTitle];
        [recordButton setNeedsDisplay:YES];
    } else {
        log_message(KB_LOG_LEVEL_ERROR, "recordButton is nil during update!");
    }
}

- (void)recordShortcutAction:(id)sender {
    log_message(KB_LOG_LEVEL_INFO, "Direct tray recording started (Background Refactor).");
    [recordButton setTitle:@"Press keys..."];
    [recordButton setEnabled:NO];
    
    startRecording();
}

- (void)quitAction:(id)sender {
    [NSApp terminate:nil];
}

- (void)checkForUpdates {
    log_message(KB_LOG_LEVEL_INFO, "Checking for updates...");
    
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
        enum UPDATE_STATUS status = is_update_available();
        
        dispatch_async(dispatch_get_main_queue(), ^{
            if (!versionMenuItem) return;
            
            if (status == UPDATE_STATUS_OUTDATED) {
                const char *remote = get_remote_version();
                NSString *title = [NSString stringWithFormat:@"Update Available (%s)", remote ? remote : "New Version"];
                [versionMenuItem setTitle:title];
                [versionMenuItem setAction:@selector(openVersionLink:)];
                [versionMenuItem setTarget:self];
                
                log_message(KB_LOG_LEVEL_INFO, "Update found: %s", remote);
                show_error_alert("Update Available", [[NSString stringWithFormat:@"A new version (%s) of KeyBlocker is available at GitHub.", remote ? remote : "New Version"] UTF8String]);
            } else if (status == UPDATE_STATUS_ERROR) {
                [versionMenuItem setTitle:[NSString stringWithFormat:@"(Check Fail) Version %s", KB_VERSION]];
                log_message(KB_LOG_LEVEL_ERROR, "Update check failed.");
                show_error_alert("Update Check Failed", "Unable to contact the update server. Please check your internet connection.");
            } else {
                [versionMenuItem setTitle:[NSString stringWithFormat:@"Version %s (Latest)", KB_VERSION]];
            }
        });
    });
}

- (void)openVersionLink:(id)sender {
    [[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:@"https://github.com/malvads/KeyBlocker"]];
}

@end

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)notification {
    log_message(KB_LOG_LEVEL_INFO, "Application finished launching. Setting up UI...");

    /* Register the callback that will be invoked when a shortcut is recorded. */
    setRecordingCallback(recorded_shortcut_callback);

    /* Initialize the keyboard event tap used to block or monitor keys. */
    kb_result_t result = setupKeyboardEventTap();
    if (result != KB_SUCCESS) {
        if (result == KB_ERROR_PERMISSION_DENIED) {
            const char *msg = "Accessibility permissions missing.\n\nPlease enable them in:\nSystem Settings > Privacy & Security > Accessibility\n\nThen restart the app.";
            show_error_alert("Permission Required", msg);
        } else {
            char buffer[256];
            snprintf(buffer, sizeof(buffer), "Failed to initialize keyboard tap (Error code: %d).", result);
            show_error_alert("Critical Error", buffer);
        }
        [NSApp terminate:nil];
        return;
    }

    /* Create and configure the status bar item and its icon. */
    statusItem = [[NSStatusBar systemStatusBar] statusItemWithLength:NSVariableStatusItemLength];    
    NSImage *image = [NSImage imageNamed:@"tray"];
    
    if (!image) {
        image = [[NSImage alloc] initWithContentsOfFile:@"assets/tray.png"];
    }

    if (image) {
        [image setSize:NSMakeSize(18, 18)];
        [image setTemplate:YES];
        statusItem.button.image = image;
    } else {
        log_message(KB_LOG_LEVEL_ERROR, "Could not find tray icon.");
    }
    
    if (!statusItem.button.image) {
        statusItem.button.title = @"KB";
    }

    /* Build the custom menu containing switches and a record button. */
    NSMenu *menu = [[NSMenu alloc] init];
    
    NSMenuItem *switchItem = [[NSMenuItem alloc] initWithTitle:@"Blocking" action:nil keyEquivalent:@""];
    
    NSView *customView = [[NSView alloc] initWithFrame:NSMakeRect(0, 0, 220, 30)];
    
    NSTextField *label = [NSTextField labelWithString:@"Keyboard Blocking"];
    [label setFrame:NSMakeRect(18, 5, 140, 20)];
    [label setFont:[NSFont systemFontOfSize:13]];
    [customView addSubview:label];
    
    blockingSwitch = [[NSSwitch alloc] initWithFrame:NSMakeRect(165, 5, 45, 20)];
    [blockingSwitch setTarget:trayDelegate];
    [blockingSwitch setAction:@selector(switchAction:)];
    [blockingSwitch setState:isKeyboardBlockEnabled() ? NSControlStateValueOn : NSControlStateValueOff];
    [customView addSubview:blockingSwitch];
    
    [switchItem setView:customView];
    [menu addItem:switchItem];
    
    [menu addItem:[NSMenuItem separatorItem]];
    
    NSMenuItem *shortcutSwitchItem = [[NSMenuItem alloc] initWithTitle:@"Allow Unlock Shortcut" action:nil keyEquivalent:@""];
    NSView *shortcutView = [[NSView alloc] initWithFrame:NSMakeRect(0, 0, 220, 30)];
    
    NSTextField *shortcutLabelText = [NSTextField labelWithString:@"Enable Shortcut"];
    [shortcutLabelText setFrame:NSMakeRect(18, 5, 140, 20)];
    [shortcutLabelText setFont:[NSFont systemFontOfSize:13]];
    [shortcutView addSubview:shortcutLabelText];
    
    shortcutSwitch = [[NSSwitch alloc] initWithFrame:NSMakeRect(165, 5, 45, 20)];
    [shortcutSwitch setTarget:trayDelegate];
    [shortcutSwitch setAction:@selector(shortcutSwitchAction:)];
    [shortcutSwitch setState:isShortcutEnabled() ? NSControlStateValueOn : NSControlStateValueOff];
    [shortcutView addSubview:shortcutSwitch];
    
    [shortcutSwitchItem setView:shortcutView];
    [menu addItem:shortcutSwitchItem];
    
    NSMenuItem *recordItem = [[NSMenuItem alloc] initWithTitle:@"Shortcut" action:nil keyEquivalent:@""];
    NSView *recordView = [[NSView alloc] initWithFrame:NSMakeRect(0, 0, 220, 30)];
    
    NSTextField *recordLabel = [NSTextField labelWithString:@"Unlock Shortcut"];
    [recordLabel setFrame:NSMakeRect(18, 5, 100, 20)];
    [recordLabel setFont:[NSFont systemFontOfSize:13]];
    [recordView addSubview:recordLabel];
    
    recordButton = [[NSButton alloc] initWithFrame:NSMakeRect(120, 5, 90, 20)];
    [recordButton setBezelStyle:NSBezelStyleRounded];
    [recordButton setTitle:[trayDelegate stringForShortcut]];
    [recordButton setTarget:trayDelegate];
    [recordButton setAction:@selector(recordShortcutAction:)];
    [recordButton setFont:[NSFont systemFontOfSize:11]];
    [recordView addSubview:recordButton];
    
    [recordItem setView:recordView];
    [recordItem setEnabled:YES];
    [menu addItem:recordItem];
    
    [menu addItem:[NSMenuItem separatorItem]];

    versionMenuItem = [[NSMenuItem alloc] initWithTitle:[NSString stringWithFormat:@"Version %s", KB_VERSION] action:nil keyEquivalent:@""];
    [versionMenuItem setEnabled:YES];
    [menu addItem:versionMenuItem];

    [menu addItem:[NSMenuItem separatorItem]];
    
    NSMenuItem *quitItem = [[NSMenuItem alloc] initWithTitle:@"Quit" 
                                                       action:@selector(quitAction:) 
                                                 keyEquivalent:@"q"];
    [quitItem setTarget:trayDelegate];
    [menu addItem:quitItem];
    
    statusItem.menu = menu;

    // Trigger update check
    [trayDelegate checkForUpdates];
}

- (void)applicationWillTerminate:(NSNotification *)notification {
    log_message(KB_LOG_LEVEL_INFO, "Application will terminate. Cleaning up...");
    cleanup_keyboard();
}

@end

/**
 * @brief Sets up the tray icon and application delegate.
 *
 * This function must be called early in the program initialization. It
 * configures the shared NSApplication, sets the activation policy so no dock
 * icon appears, and creates instances of the delegate objects used by the
 * status bar UI.
 */
void setup_tray_icon() {
    log_message(KB_LOG_LEVEL_DEBUG, "Preparing environment.");
    
    [NSApplication sharedApplication];
    [NSApp setActivationPolicy:NSApplicationActivationPolicyAccessory];
    
    trayDelegate = [[StatusBarDelegate alloc] init];
    appDelegate = [[AppDelegate alloc] init];
    [NSApp setDelegate:appDelegate];
}

/**
 * @brief Updates the menu item UI to reflect the current blocking state.
 *
 * This function schedules a main-queue update so it can be safely called
 * from any thread.
 *
 * @param active true if keyboard blocking is currently enabled; false
 * otherwise.
 */
void update_tray_state(bool active) {
    if (!statusItem) return;

    dispatch_async(dispatch_get_main_queue(), ^{
        if (blockingSwitch) {
            blockingSwitch.state = active ? NSControlStateValueOn : NSControlStateValueOff;
        }
    });
}

/**
 * @brief Runs the application's main event loop.
 *
 * After calling this function the app enters the Cocoa run loop and will not
 * return until the application exits.
 */
void run_app() {
    log_message(KB_LOG_LEVEL_INFO, "Entering main event loop.");
    [NSApp run];
}

/**
 * @brief Shows a modal critical error alert using NSAlert.
 *
 * This is a simple helper used during startup to show fatal errors such as
 * missing permissions or initialization failures.
 *
 * @param title   UTF-8 NUL-terminated C string for the alert title.
 * @param message UTF-8 NUL-terminated C string for the alert informative
 *                text.
 */
void show_error_alert(const char *title, const char *message) {
    NSString *nsTitle = [NSString stringWithUTF8String:title];
    NSString *nsMessage = [NSString stringWithUTF8String:message];

    NSAlert *alert = [[NSAlert alloc] init];
    [alert setMessageText:nsTitle];
    [alert setInformativeText:nsMessage];
    [alert addButtonWithTitle:@"OK"];
    [alert setAlertStyle:NSAlertStyleCritical];
    
    [NSApp activateIgnoringOtherApps:YES];
    [alert runModal];
}
