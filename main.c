#include <stdio.h>
#include <string.h>
#include "keyboard.h"
#include "tray.h"
#include "logger.h"

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
 * @brief Initializes the keyboard blocking logic.
 * @return 1 on success, 0 on failure.
 */
static int init_keyboard() {
    kb_result_t result = setupKeyboardEventTap();
    if (result != KB_SUCCESS) {
        if (result == KB_ERROR_PERMISSION_DENIED) {
            log_message(KB_LOG_LEVEL_ERROR, "Accessibility permissions missing. Please enable them in System Settings.");
        } else {
            log_message(KB_LOG_LEVEL_ERROR, "Failed to initialize keyboard tap (Error code: %d).", result);
        }
        return 0;
    }
    log_message(KB_LOG_LEVEL_INFO, "Keyboard tap initialized successfully.");
    return 1;
}

/**
 * @brief Sets up the system tray icon.
 */
static void init_tray() {
    setup_tray_icon();
    log_message(KB_LOG_LEVEL_INFO, "Tray icon initialized successfully.");
}
static void run() {
    run_app();
    log_message(KB_LOG_LEVEL_INFO, "Keyboard Blocker running successfully.");
}

int main(int argc, char *argv[]) {
    int log_level = parse_arguments(argc, argv);
    set_kb_log_level(log_level);

    log_message(KB_LOG_LEVEL_INFO, "Keyboard blocker started.");
    if (!init_keyboard()) {
        return 1;
    }

    init_tray();
    run();

    return 0;
}
