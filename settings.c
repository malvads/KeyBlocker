/**
 * @file settings.c
 * @brief Persistent application settings handling.
 *
 * This module implements a very small and dependency-free configuration
 * system based on a plain text key-value file. It is intentionally simple
 * to avoid pulling in external parsers (e.g. JSON libraries) and to keep
 * startup and runtime overhead minimal.
 */

#include "settings.h"
#include "logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief Name of the settings file stored on disk.
 *
 * The file is expected to contain one key=value pair per line.
 */
#define SETTINGS_FILE "settings.conf"

/**
 * @brief Default value indicating whether the unlock shortcut is enabled.
 */
#define DEFAULT_SHORTCUT_ENABLED true

/**
 * @brief Default modifier flags for the unlock shortcut.
 *
 * This value represents a CGEventFlags bitmask stored as an integer.
 */
#define DEFAULT_SHORTCUT_FLAGS 1179648

/**
 * @brief Default key code for the unlock shortcut.
 */
#define DEFAULT_SHORTCUT_KEYCODE 12

/**
 * @brief Default value indicating whether keyboard blocking is enabled.
 *
 * Blocking is intentionally disabled by default for safety reasons.
 */
#define DEFAULT_BLOCKING_ENABLED false

/**
 * @brief Loads application settings from disk.
 *
 * The settings file is parsed as a simple line-based key=value format.
 * Unknown keys are ignored. If the file does not exist, all fields are
 * initialized to safe default values.
 *
 * @note For safety reasons, the blocking state is never restored from disk
 *       and always falls back to the default (disabled).
 *
 * @param s Pointer to an app_settings_t structure to populate.
 */
void load_settings(app_settings_t *s) {
    if (!s) return;

    /* Initialize defaults */
    s->shortcut_enabled = DEFAULT_SHORTCUT_ENABLED;
    s->shortcut_flags = DEFAULT_SHORTCUT_FLAGS;
    s->shortcut_keycode = DEFAULT_SHORTCUT_KEYCODE;
    s->blocking_enabled = DEFAULT_BLOCKING_ENABLED;

    FILE *f = fopen(SETTINGS_FILE, "r");
    if (!f) {
        log_message(KB_LOG_LEVEL_INFO, "No settings file found, using defaults.");
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), f)) {
        char *key = strtok(line, "=");
        char *val = strtok(NULL, "\n");
        if (key && val) {
            if (strcmp(key, "shortcut_enabled") == 0) {
                s->shortcut_enabled = (atoi(val) != 0);
            } else if (strcmp(key, "shortcut_flags") == 0) {
                s->shortcut_flags = strtoull(val, NULL, 10);
            } else if (strcmp(key, "shortcut_keycode") == 0) {
                s->shortcut_keycode = (unsigned short)atoi(val);
            } else if (strcmp(key, "blocking_enabled") == 0) {
                /*
                 * For safety, never restore blocking state from disk.
                 * Always force the default value.
                 */
                s->blocking_enabled = DEFAULT_BLOCKING_ENABLED;
            }
        }
    }
    fclose(f);
    log_message(KB_LOG_LEVEL_INFO, "Settings loaded successfully.");
}

/**
 * @brief Saves application settings to disk.
 *
 * The settings are written using the same simple key=value format that
 * load_settings() expects. If the file cannot be opened, the function logs
 * an error and returns without modifying any state.
 *
 * @param s Pointer to the app_settings_t structure containing settings
 *          to persist.
 */
void save_settings(const app_settings_t *s) {
    if (!s) return;

    FILE *f = fopen(SETTINGS_FILE, "w");
    if (!f) {
        log_message(KB_LOG_LEVEL_ERROR, "Failed to open settings file for writing.");
        return;
    }

    /* Persist only non-dangerous settings */
    fprintf(f, "shortcut_enabled=%d\n", s->shortcut_enabled ? 1 : 0);
    fprintf(f, "shortcut_flags=%llu\n", (unsigned long long)s->shortcut_flags);
    fprintf(f, "shortcut_keycode=%hu\n", s->shortcut_keycode);
    fprintf(f, "blocking_enabled=%d\n", s->blocking_enabled ? 1 : 0);
    
    fclose(f);
    log_message(KB_LOG_LEVEL_DEBUG, "Settings saved.");
}
