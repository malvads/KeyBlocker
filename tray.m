/**
 * @file tray.m
 * @brief Implementation of the macOS system tray (menu bar) icon.
 */
#import <Cocoa/Cocoa.h>
#include "tray.h"
#include "keyboard.h"
#include "logger.h"

/**
 * @interface StatusBarDelegate
 * @brief Delegate for handling status bar menu actions.
 */
@interface StatusBarDelegate : NSObject
- (void)toggleAction:(id)sender;
- (void)quitAction:(id)sender;
@end

/**
 * @interface AppDelegate
 * @brief Delegate for handling application lifecycle events.
 */
@interface AppDelegate : NSObject <NSApplicationDelegate>
@end

/**
 * @brief Global variables for tray icon and delegates.
 */
static NSStatusItem *statusItem;
static StatusBarDelegate *trayDelegate;
static AppDelegate *appDelegate;
static NSMenuItem *toggleMenuItem;

/**
 * @brief Implementation of StatusBarDelegate.
 */
@implementation StatusBarDelegate

- (void)toggleAction:(id)sender {
    bool current = isKeyboardBlockEnabled();
    bool newState = !current;
    enableKeyboardBlock(newState);
    update_tray_state(newState);
}

- (void)quitAction:(id)sender {
    [NSApp terminate:nil];
}

@end

/**
 * @brief Implementation of AppDelegate.
 */
@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)notification {
    log_message(KB_LOG_LEVEL_INFO, "App Bundle Path: %s", [[[NSBundle mainBundle] bundlePath] UTF8String]);
    log_message(KB_LOG_LEVEL_DEBUG, "Application finished launching. Setting up tray icon.");
    
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
        statusItem.button.title = @"KB";
    }

    NSMenu *menu = [[NSMenu alloc] init];
    
    toggleMenuItem = [[NSMenuItem alloc] initWithTitle:@"Toggle Blocking (Disabled)"
                                                action:@selector(toggleAction:) 
                                         keyEquivalent:@""];
    [toggleMenuItem setTarget:trayDelegate];
    [menu addItem:toggleMenuItem];
    
    [menu addItem:[NSMenuItem separatorItem]];
    
    NSMenuItem *quitItem = [[NSMenuItem alloc] initWithTitle:@"Quit" 
                                                       action:@selector(quitAction:) 
                                                keyEquivalent:@"q"];
    [quitItem setTarget:trayDelegate];
    [menu addItem:quitItem];
    
    statusItem.menu = menu;
    
    update_tray_state(isKeyboardBlockEnabled());
}

@end

/**
 * @brief Sets up the tray icon and application delegate.
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
 * @brief Updates the menu item text based on the current state.
 */
void update_tray_state(bool active) {
    if (!statusItem) return;

    dispatch_async(dispatch_get_main_queue(), ^{
        if (toggleMenuItem) {
            toggleMenuItem.title = active ? @"Toggle Blocking (Enabled)" : @"Toggle Blocking (Disabled)";
        }
    });
}

/**
 * @brief Runs the main application event loop.
 */
void run_app() {
    log_message(KB_LOG_LEVEL_INFO, "Entering main event loop.");
    [NSApp run];
}
