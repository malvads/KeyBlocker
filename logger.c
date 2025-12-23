/**
 * @file logger.c
 * @brief Implementation of the logging utility.
 */

#include <stdio.h>
#include <stdarg.h>
#include "logger.h"

/** @brief Global variable to store active log levels. */
static int g_kb_log_level = KB_LOG_LEVEL_INFO | KB_LOG_LEVEL_ERROR;

void set_kb_log_level(int level) {
    g_kb_log_level = level;
}

int get_kb_log_level() {
    return g_kb_log_level;
}

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
