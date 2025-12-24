/**
 * @file settings.h
 * @brief Definition of the application settings structure and persistence API.
 *
 * Provides a minimal, dependency-free interface to load and save application
 * settings using a simple key-value file format.
 */
#ifndef SETTINGS_H
#define SETTINGS_H

#include <stdbool.h>
/**
 * @brief Structure holding all configurable application settings.
 *
 * Fields:
 * - shortcut_enabled: whether the unlock shortcut is enabled
 * - shortcut_flags: modifier flags (CGEventFlags) for the shortcut
 * - shortcut_keycode: hardware key code for the shortcut
 * - blocking_enabled: whether keyboard blocking is currently enabled (not
 *   persisted for safety)
 */
typedef struct {
    bool shortcut_enabled;
    unsigned long long shortcut_flags;
    unsigned short shortcut_keycode;
    bool blocking_enabled;
} app_settings_t;

/**
 * @brief Load settings from persistent storage.
 *
 * If the settings file does not exist, default values are applied.
 * The blocking_enabled field is always set to its default value for safety.
 *
 * @param settings Pointer to the app_settings_t structure to populate.
 */
void load_settings(app_settings_t *settings);

/**
 * @brief Save settings to persistent storage.
 *
 * Only safe fields (shortcut settings) are written. Blocking state is not
 * persisted.
 *
 * @param settings Pointer to the app_settings_t structure to save.
 */
void save_settings(const app_settings_t *settings);

#endif