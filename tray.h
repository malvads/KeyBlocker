/**
 * @file tray.h
 * @brief Interface for the macOS system tray (menu bar) icon.
 */

#ifndef TRAY_H
#define TRAY_H

#include <stdbool.h>

/**
 * @brief Initializes and displays the system tray icon.
 */
void setup_tray_icon();

/**
 * @brief Updates the visual state (title/icon) of the tray.
 * @param active True if keyboard blocking is active.
 */
void update_tray_state(bool active);

/**
 * @brief Enters the main application run loop.
 */
void run_app();

/**
 * @brief Shows a critical error alert to the user.
 * @param title The title of the alert.
 * @param message The message body of the alert.
 */
void show_error_alert(const char *title, const char *message);

#endif
