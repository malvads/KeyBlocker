/**
 * @file main.c
 * @brief Entry point for the macOS keyboard blocker application (Cocoa mode).
 *
 * Handles command-line argument parsing, logging setup, and initializes the
 * system tray UI and event loop.
 */

#include <stdio.h>
#include <string.h>
#include "keyboard.h"
#include "tray.h"
#include "logger.h"
#include "version.h"

/**
 * @brief Parses command-line arguments to determine the logging level.
 *
 * Recognizes:
 * - `-v` or `--verbose`: enables debug logging
 * - `--log-level <level>`: sets logging level explicitly (debug, info, error)
 *
 * @param argc Argument count
 * @param argv Argument vector
 * @return Combined bitmask of logging levels to enable
 */
static int parse_arguments(int argc, char *argv[]) {
    int log_level = KB_LOG_LEVEL_INFO | KB_LOG_LEVEL_ERROR;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--verbose") == 0) {
            log_level |= KB_LOG_LEVEL_DEBUG;
        } 
        else if (strcmp(argv[i], "--log-level") == 0 && i + 1 < argc) {
            if (strcmp(argv[i + 1], "debug") == 0) {
                log_level = KB_LOG_LEVEL_ALL;
            } 
            else if (strcmp(argv[i + 1], "info") == 0) {
                log_level = KB_LOG_LEVEL_INFO | KB_LOG_LEVEL_ERROR;
            } 
            else if (strcmp(argv[i + 1], "error") == 0) {
                log_level = KB_LOG_LEVEL_ERROR;
            }
            i++; 
        }
    }

    return log_level;
}

/**
 * @brief Initializes the macOS system tray icon and menu.
 *
 * Wraps the `setup_tray_icon` call and logs successful initialization.
 */
static void init_tray() {
    setup_tray_icon();
    log_message(KB_LOG_LEVEL_INFO, "Tray icon initialized successfully.");
}

/**
 * @brief Starts the Cocoa application run loop.
 *
 * This function will not return until the application exits. Logs when the
 * event loop has started.
 */
static void run() {
    run_app();
    log_message(KB_LOG_LEVEL_INFO, "Keyboard Blocker running successfully.");
}


/**
 * @brief Main entry point of the keyboard blocker application.
 *
 * - Parses command-line arguments
 * - Sets up logging
 * - Initializes the tray icon
 * - Runs the main Cocoa event loop
 *
 * @param argc Argument count
 * @param argv Argument vector
 * @return Exit status code (0 on success)
 */
int main(int argc, char *argv[]) {
    int log_level = parse_arguments(argc, argv);
    set_kb_log_level(log_level);

    log_message(KB_LOG_LEVEL_INFO, "Keyboard blocker starting (Cocoa Mode)...");
    log_message(KB_LOG_LEVEL_INFO, "Current version: %s", KB_VERSION);

    init_tray();
    run();

    return 0;
}