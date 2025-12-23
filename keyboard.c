/**
 * @file keyboard.c
 * @brief Implementation of keyboard event interception using CoreGraphics.
 */

#include "keyboard.h"
#include <ApplicationServices/ApplicationServices.h>
#include <Carbon/Carbon.h>
#include "logger.h"
#include <stdio.h>
#include <stdlib.h>

#ifndef kCGEventSystemDefined
#define kCGEventSystemDefined 14
#endif

/**
 * @brief Internal state for the keyboard blocker.
 */
typedef struct {
    CFMachPortRef eventTap;
    CFRunLoopSourceRef runLoopSource;
    bool enabled;
} kb_context_t;

/** @brief Global context instance. */
static kb_context_t *g_context = NULL;

/**
 * @brief Callback invoked for every keyboard event.
 * @return NULL to block the event, or the event itself to let it pass.
 */
CGEventRef keyboardCallback(CGEventTapProxy proxy,
                             CGEventType type,
                             CGEventRef event,
                             void *refcon) {
    kb_context_t *ctx = (kb_context_t *)refcon;
    if (!ctx || !ctx->enabled) return event;

    if (type == kCGEventKeyDown || type == kCGEventKeyUp || type == kCGEventFlagsChanged || type == kCGEventSystemDefined) {
        log_message(KB_LOG_LEVEL_DEBUG, "Keyboard event blocked");
        return NULL;
    }
    return event;
}

kb_result_t setupKeyboardEventTap(void) {
    if (g_context) return KB_ERROR_ALREADY_STARTED; 

    g_context = (kb_context_t *)calloc(1, sizeof(kb_context_t));
    if (!g_context) return KB_ERROR_EVENT_TAP_FAILED;

    CGEventMask eventMask = CGEventMaskBit(kCGEventKeyDown) | 
                            CGEventMaskBit(kCGEventKeyUp) | 
                            CGEventMaskBit(kCGEventFlagsChanged) | 
                            CGEventMaskBit(kCGEventSystemDefined);

    g_context->eventTap = CGEventTapCreate(kCGSessionEventTap,
                                           kCGHeadInsertEventTap,
                                           kCGEventTapOptionDefault,
                                           eventMask,
                                           keyboardCallback,
                                           g_context);

    if (!g_context->eventTap) {
        log_message(KB_LOG_LEVEL_ERROR, "Failed to create event tap. Check Accessibility permissions.");
        free(g_context);
        g_context = NULL;
        return KB_ERROR_PERMISSION_DENIED;
    }

    g_context->runLoopSource = CFMachPortCreateRunLoopSource(kCFAllocatorDefault, g_context->eventTap, 0);
    CFRunLoopAddSource(CFRunLoopGetCurrent(), g_context->runLoopSource, kCFRunLoopCommonModes);
    CGEventTapEnable(g_context->eventTap, true);
    
    log_message(KB_LOG_LEVEL_INFO, "Event tap created successfully.");    
    return KB_SUCCESS;
}

void enableKeyboardBlock(bool on) {
    if (g_context) {
        g_context->enabled = on;
        log_message(KB_LOG_LEVEL_INFO, "Keyboard block status updated: %s", on ? "ACTIVE" : "INACTIVE");
    }
}

bool isKeyboardBlockEnabled(void) {
    return g_context ? g_context->enabled : false;
}
