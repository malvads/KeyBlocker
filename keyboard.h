/**
 * @file keyboard.h
 * @brief Logic for intercepting and blocking keyboard events.
 */

#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdbool.h>

/**
 * @brief Result codes for keyboard operations.
 */
typedef enum {
    KB_SUCCESS = 0,             /**< Operation successful */
    KB_ERROR_PERMISSION_DENIED, /**< Accessibility permissions missing */
    KB_ERROR_EVENT_TAP_FAILED,   /**< Failed to create event tap */
    KB_ERROR_ALREADY_STARTED    /**< Session already initialized */
} kb_result_t;

/**
 * @brief Initializes the keyboard event tap.
 * @return KB_SUCCESS on success, or an error code.
 */
kb_result_t setupKeyboardEventTap(void);

/**
 * @brief Enables or disables the keyboard blocking.
 * @param on True to block, false to pass events through.
 */
void enableKeyboardBlock(bool on);

/**
 * @brief Checks if keyboard blocking is currently active.
 * @return True if enabled.
 */
bool isKeyboardBlockEnabled(void);

/**
 * @brief Cleans up resources used by the keyboard blocker.
 */
void cleanup_keyboard(void);

#endif
