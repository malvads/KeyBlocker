/**
 * @file logger.h
 * @brief Simple logging utility with color support and multiple log levels.
 *
 * Provides functions for setting global log level, querying current level,
 * and logging formatted messages to stdout with optional color-coded output.
 */

#ifndef LOGGER_H
#define LOGGER_H

/** ANSI color codes for terminal output. */
#define COLOR_RESET   "\x1b[0m"
#define COLOR_RED     "\x1b[31m"
#define COLOR_GREEN   "\x1b[32m"
#define COLOR_YELLOW  "\x1b[33m"
#define COLOR_BLUE    "\x1b[34m"

/**
 * @brief Log level flags for categorizing messages.
 */
enum KB_LOG_LEVEL_FLAGS {
    KB_LOG_LEVEL_NONE  = 0,         /**< No logging */
    KB_LOG_LEVEL_INFO  = 1 << 0,    /**< General information */
    KB_LOG_LEVEL_ERROR = 1 << 1,    /**< Error messages */
    KB_LOG_LEVEL_DEBUG = 1 << 2,    /**< Debugging traces */
    KB_LOG_LEVEL_ALL   = 0xFF       /**< All levels enabled */
};

/**
 * @brief Sets the global log level for filtering messages.
 *
 * Only messages with levels enabled in this bitmask will be output.
 *
 * @param level Bitmask of KB_LOG_LEVEL_FLAGS to enable.
 */
void set_kb_log_level(int level);

/**
 * @brief Retrieves the current global log level bitmask.
 *
 * @return Active log level bitmask.
 */
int get_kb_log_level();

/**
 * @brief Logs a formatted message to stdout if the level is enabled.
 *
 * Supports printf-style formatting and color-coded output depending on level.
 *
 * @param level Log level of this message.
 * @param format printf-style format string.
 * @param ... Additional arguments corresponding to the format string.
 */
void log_message(enum KB_LOG_LEVEL_FLAGS level, const char *format, ...);

#endif