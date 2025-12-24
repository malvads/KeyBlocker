/**
 * @file keyboard.h
 * @brief Interface for intercepting, blocking, and managing keyboard events.
 *
 * Provides functions for installing a keyboard event tap, enabling/disabling
 * blocking, managing an emergency shortcut, and recording key combinations.
 */

#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdbool.h>

/**
 * @brief Result codes returned by keyboard operations.
 */
typedef enum {
    KB_SUCCESS = 0,             /**< Operation successful */
    KB_ERROR_PERMISSION_DENIED, /**< Accessibility permissions missing */
    KB_ERROR_EVENT_TAP_FAILED,  /**< Failed to create event tap */
    KB_ERROR_ALREADY_STARTED    /**< Session already initialized */
} kb_result_t;

/**
 * @brief Initializes the keyboard event tap.
 *
 * Sets up a low-level hook to intercept keyboard events.
 *
 * @return KB_SUCCESS on success, or an appropriate error code.
 */
kb_result_t setupKeyboardEventTap(void);

/**
 * @brief Enables or disables keyboard blocking.
 *
 * When enabled, all keyboard events are intercepted and suppressed.
 *
 * @param on True to block events, false to allow them.
 */
void enableKeyboardBlock(bool on);

/**
 * @brief Checks if keyboard blocking is currently active.
 *
 * @return True if keyboard events are being blocked, false otherwise.
 */
bool isKeyboardBlockEnabled(void);

/**
 * @brief Cleans up resources used by the keyboard blocker.
 *
 * Removes any installed event taps and frees internal resources.
 */
void cleanup_keyboard(void);

/**
 * @brief Enables or disables the emergency unlock shortcut.
 *
 * @param enabled True to enable the shortcut, false to disable.
 */
void setShortcutEnabled(bool enabled);

/**
 * @brief Checks whether the emergency shortcut is enabled.
 *
 * @return True if the shortcut is enabled, false otherwise.
 */
bool isShortcutEnabled(void);

/**
 * @brief Sets the key combination for the emergency shortcut.
 *
 * @param flags Modifier flags (e.g., Command, Control, Shift).
 * @param keyCode Hardware key code for the main key.
 */
void setShortcut(unsigned long long flags, unsigned short keyCode);

/**
 * @brief Retrieves the current key combination for the emergency shortcut.
 *
 * @param flags Output pointer for modifier flags.
 * @param keyCode Output pointer for the main key code.
 */
void getShortcut(unsigned long long *flags, unsigned short *keyCode);

/**
 * @brief Sets the callback to invoke when a shortcut is recorded.
 *
 * @param callback Function pointer that receives flags and keyCode.
 */
void setRecordingCallback(void (*callback)(unsigned long long flags, unsigned short keyCode));

/**
 * @brief Starts recording a one-time emergency shortcut.
 *
 * This will capture the next key combination pressed by the user.
 */
void startRecording(void);

#endif