/**
 * @file keyboard.c
 * @brief Implementation of keyboard event interception and blocking using CoreGraphics.
 *
 * Provides a low-level event tap to block keyboard input, manage an emergency
 * unlock shortcut, record key combinations, and synchronize settings.
 */

#include "keyboard.h"
#include "settings.h"
#include <ApplicationServices/ApplicationServices.h>
#include <Carbon/Carbon.h>
#include "logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#ifndef kCGEventSystemDefined
#define kCGEventSystemDefined 14
#endif

/**
 * @brief Internal context for managing keyboard state.
 */
typedef struct {
    CFMachPortRef eventTap;                 /**< Event tap reference */
    CFRunLoopSourceRef runLoopSource;      /**< Run loop source for the tap */
    bool enabled;                           /**< Whether blocking is active */
    bool shortcutEnabled;                   /**< Whether the emergency shortcut is active */
    bool recording;                         /**< Whether recording a new shortcut */
    CGEventFlags shortcutFlags;             /**< Modifier flags for shortcut */
    CGKeyCode shortcutKeyCode;              /**< Key code for shortcut */
    void (*recordingCallback)(unsigned long long, unsigned short); /**< Callback when recording completes */
    pthread_t thread;                        /**< Background thread running the event tap */
} kb_context_t;

/** @brief Global context instance. */
static kb_context_t *g_context = NULL;
/** @brief Global callback for recording shortcuts. */
static void (*g_recording_callback)(unsigned long long, unsigned short) = NULL;

/** Forward declaration for tray update function */
extern void update_tray_state(bool active);

/**
 * @brief Synchronizes context settings to disk.
 */
static void sync_and_save_settings(void) {
    if (!g_context) return;
    app_settings_t s;
    s.shortcut_enabled = g_context->shortcutEnabled;
    s.shortcut_flags = (unsigned long long)g_context->shortcutFlags;
    s.shortcut_keycode = (unsigned short)g_context->shortcutKeyCode;
    s.blocking_enabled = g_context->enabled;
    save_settings(&s);
}

/**
 * @brief Keyboard event callback.
 *
 * Handles blocking, shortcut detection, and one-shot recording.
 *
 * @param proxy Unused event tap proxy.
 * @param type Type of the keyboard event.
 * @param event The keyboard event.
 * @param refcon Pointer to kb_context_t.
 * @return NULL to block the event, or the original event to allow.
 */
CGEventRef keyboardCallback(CGEventTapProxy proxy, CGEventType type, CGEventRef event, void *refcon) {
    kb_context_t *ctx = (kb_context_t *)refcon;
    if (!ctx) return event;

    /* Handle one-shot recording */
    if (ctx->recording) {
        if (type == kCGEventKeyDown) {
            CGEventFlags flags = CGEventGetFlags(event);
            CGKeyCode keyCode = (CGKeyCode)CGEventGetIntegerValueField(event, kCGKeyboardEventKeycode);
            CGEventFlags cleanFlags = flags & (kCGEventFlagMaskCommand | kCGEventFlagMaskShift | 
                                              kCGEventFlagMaskAlternate | kCGEventFlagMaskControl);
            ctx->shortcutFlags = cleanFlags;
            ctx->shortcutKeyCode = keyCode;
            ctx->recording = false;
            sync_and_save_settings();
            log_message(KB_LOG_LEVEL_INFO, "Shortcut recorded and saved.");
            if (ctx->recordingCallback) {
                log_message(KB_LOG_LEVEL_INFO, "Shortcut flags: %llu, KeyCode: %hu", (unsigned long long)cleanFlags, (unsigned short)keyCode);
                ctx->recordingCallback((unsigned long long)cleanFlags, (unsigned short)keyCode);
            }
        }
        return event;
    }

    /* Handle emergency shortcut */
    if (ctx->shortcutEnabled && type == kCGEventKeyDown) {
        CGEventFlags flags = CGEventGetFlags(event);
        CGKeyCode keyCode = (CGKeyCode)CGEventGetIntegerValueField(event, kCGKeyboardEventKeycode);
        CGEventFlags cleanFlags = flags & (kCGEventFlagMaskCommand | kCGEventFlagMaskShift | 
                                          kCGEventFlagMaskAlternate | kCGEventFlagMaskControl);
        if (cleanFlags == ctx->shortcutFlags && keyCode == ctx->shortcutKeyCode) {
            log_message(KB_LOG_LEVEL_INFO, "Emergency shortcut detected. Disabling block.");
            ctx->enabled = false;
            update_tray_state(false);
            return event; 
        }
    }

    /* Block events if enabled */
    if (!ctx->enabled) return event;
    if (type == kCGEventKeyDown || type == kCGEventKeyUp || type == kCGEventFlagsChanged || type == kCGEventSystemDefined) {
        log_message(KB_LOG_LEVEL_DEBUG, "Keyboard event blocked");
        return NULL;
    }
    return event;
}

/**
 * @brief Thread function that runs the event tap.
 *
 * @param arg Pointer to kb_context_t
 * @return Always NULL
 */
static void *keyboard_thread_func(void *arg) {
    kb_context_t *ctx = (kb_context_t *)arg;
    CGEventMask eventMask = CGEventMaskBit(kCGEventKeyDown) | 
                            CGEventMaskBit(kCGEventKeyUp) | 
                            CGEventMaskBit(kCGEventFlagsChanged) | 
                            CGEventMaskBit(kCGEventSystemDefined);
    ctx->eventTap = CGEventTapCreate(kCGSessionEventTap, kCGHeadInsertEventTap, kCGEventTapOptionDefault, eventMask, keyboardCallback, ctx);
    if (!ctx->eventTap) {
        log_message(KB_LOG_LEVEL_ERROR, "Failed to create event tap. Check Accessibility permissions.");
        return NULL;
    }
    ctx->runLoopSource = CFMachPortCreateRunLoopSource(kCFAllocatorDefault, ctx->eventTap, 0);
    CFRunLoopAddSource(CFRunLoopGetCurrent(), ctx->runLoopSource, kCFRunLoopCommonModes);
    CGEventTapEnable(ctx->eventTap, true);
    log_message(KB_LOG_LEVEL_INFO, "Event tap created successfully in background thread.");    
    CFRunLoopRun();
    return NULL;
}

/**
 * @brief Loads default keyboard-related settings from persistence.
 */
void loadDefaultKeyboardSettings(void) {
    app_settings_t s;
    load_settings(&s);
    g_context->enabled = s.blocking_enabled;
    g_context->shortcutEnabled = s.shortcut_enabled;
    g_context->recording = false;
    g_context->shortcutFlags = (CGEventFlags)s.shortcut_flags;
    g_context->shortcutKeyCode = (CGKeyCode)s.shortcut_keycode;
}

/**
 * @brief Initializes the keyboard event tap and background thread.
 *
 * @return KB_SUCCESS on success or an error code.
 */
kb_result_t setupKeyboardEventTap(void) {
    if (g_context) return KB_ERROR_ALREADY_STARTED; 
    g_context = (kb_context_t *)calloc(1, sizeof(kb_context_t));
    if (!g_context) return KB_ERROR_EVENT_TAP_FAILED;
    loadDefaultKeyboardSettings();
    if (pthread_create(&g_context->thread, NULL, keyboard_thread_func, g_context) != 0) {
        log_message(KB_LOG_LEVEL_ERROR, "Failed to create keyboard thread.");
        free(g_context);
        g_context = NULL;
        return KB_ERROR_EVENT_TAP_FAILED;
    }
    g_context->recordingCallback = g_recording_callback;
    return KB_SUCCESS;
}

/**
 * @brief Enables or disables keyboard blocking.
 *
 * @param on True to block, false to pass events through.
 */
void enableKeyboardBlock(bool on) {
    if (g_context) {
        g_context->enabled = on;
        sync_and_save_settings();
        log_message(KB_LOG_LEVEL_INFO, "Keyboard block status updated: %s", on ? "ACTIVE" : "INACTIVE");
    }
}

/**
 * @brief Returns whether keyboard blocking is currently enabled.
 *
 * @return True if blocking, false otherwise.
 */
bool isKeyboardBlockEnabled(void) {
    return g_context ? g_context->enabled : false;
}

/**
 * @brief Enables or disables the emergency shortcut.
 */
void setShortcutEnabled(bool enabled) {
    if (g_context) {
        g_context->shortcutEnabled = enabled;
        sync_and_save_settings();
    }
}

/**
 * @brief Returns whether the emergency shortcut is enabled.
 */
bool isShortcutEnabled(void) {
    return g_context ? g_context->shortcutEnabled : false;
}

/**
 * @brief Sets the key combination for the emergency shortcut.
 */
void setShortcut(unsigned long long flags, unsigned short keyCode) {
    if (g_context) {
        g_context->shortcutFlags = (CGEventFlags)flags;
        g_context->shortcutKeyCode = (CGKeyCode)keyCode;
        sync_and_save_settings();
    }
}

/**
 * @brief Retrieves the current key combination for the emergency shortcut.
 */
void getShortcut(unsigned long long *flags, unsigned short *keyCode) {
    if (g_context) {
        if (flags) *flags = (unsigned long long)g_context->shortcutFlags;
        if (keyCode) *keyCode = (unsigned short)g_context->shortcutKeyCode;
    }
}

/**
 * @brief Sets the callback to be invoked when a shortcut is recorded.
 */
void setRecordingCallback(void (*callback)(unsigned long long flags, unsigned short keyCode)) {
    g_recording_callback = callback;
    if (g_context) {
        g_context->recordingCallback = callback;
    }
}

/**
 * @brief Starts recording a one-shot emergency shortcut.
 */
void startRecording(void) {
    if (g_context) {
        g_context->recording = true;
        log_message(KB_LOG_LEVEL_DEBUG, "Recording mode: ON (one-shot)");
    }
}

/**
 * @brief Cleans up keyboard resources, including event taps and threads.
 */
void cleanup_keyboard(void) {
    if (!g_context) return;
    if (g_context->runLoopSource) {
        CFRunLoopRemoveSource(CFRunLoopGetCurrent(), g_context->runLoopSource, kCFRunLoopCommonModes);
        CFRelease(g_context->runLoopSource);
        g_context->runLoopSource = NULL;
    }
    if (g_context->eventTap) {
        CGEventTapEnable(g_context->eventTap, false);
        CFRelease(g_context->eventTap);
        g_context->eventTap = NULL;
    }
    free(g_context);
    g_context = NULL;
    log_message(KB_LOG_LEVEL_INFO, "Keyboard blocker resources cleaned up.");
}