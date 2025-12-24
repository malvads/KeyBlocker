/**
 * @file logger.c
 * @brief Implementation of a simple logging utility with color-coded output.
 *
 * Provides functions to set/get global log levels and to log messages to
 * stdout with optional color highlighting depending on message severity.
 */

#include <stdio.h>
#include <stdarg.h>
#include "logger.h"

/** @brief Global variable storing the currently active log levels. */
static int g_kb_log_level = KB_LOG_LEVEL_INFO | KB_LOG_LEVEL_ERROR;

/**
 * @brief Sets the global log level.
 *
 * Only messages matching enabled levels will be output by log_message().
 *
 * @param level Bitmask of KB_LOG_LEVEL_FLAGS to enable.
 */
void set_kb_log_level(int level) {
    g_kb_log_level = level;
}

/**
 * @brief Returns the current global log level.
 *
 * @return Bitmask of currently enabled KB_LOG_LEVEL_FLAGS.
 */
int get_kb_log_level() {
    return g_kb_log_level;
}

/**
 * @brief Logs a formatted message to stdout if its level is enabled.
 *
 * The message is prefixed with a colored label based on the log level.
 * Supports printf-style formatting.
 *
 * @param level Log level of this message.
 * @param format printf-style format string.
 * @param ... Additional arguments for the format string.
 */
void log_message(enum KB_LOG_LEVEL_FLAGS level, const char *format, ...) {
    if (!(g_kb_log_level & level)) {
        return;
    }

    va_list args;
    va_start(args, format);

    if (level & KB_LOG_LEVEL_INFO) {
        printf(COLOR_GREEN "INFO: " COLOR_RESET);
    } else if (level & KB_LOG_LEVEL_ERROR) {
        printf(COLOR_RED "ERROR: " COLOR_RESET);
    } else if (level & KB_LOG_LEVEL_DEBUG) {
        printf(COLOR_BLUE "DEBUG: " COLOR_RESET);
    }

    vprintf(format, args);
    printf("\n");

    va_end(args);
}