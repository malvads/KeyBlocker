/**
 * @file logger.h
 * @brief Simple logging utility with color support and levels.
 */

#ifndef LOGGER_H
#define LOGGER_H

// ANSI Color Codes for terminal output
#define COLOR_RESET   "\x1b[0m"
#define COLOR_RED     "\x1b[31m"
#define COLOR_GREEN   "\x1b[32m"
#define COLOR_YELLOW  "\x1b[33m"
#define COLOR_BLUE    "\x1b[34m"

/**
 * @brief Log level flags.
 */
enum KB_LOG_LEVEL_FLAGS {
    KB_LOG_LEVEL_NONE  = 0,         /**< No logging */
    KB_LOG_LEVEL_INFO  = 1 << 0,    /**< General information */
    KB_LOG_LEVEL_ERROR = 1 << 1,    /**< Error messages */
    KB_LOG_LEVEL_DEBUG = 1 << 2,    /**< Debugging traces */
    KB_LOG_LEVEL_ALL   = 0xFF       /**< All levels enabled */
};

/**
 * @brief Sets the global log level.
 * @param level Bitmask of KB_LOG_LEVEL_FLAGS.
 */
void set_kb_log_level(int level);

/**
 * @brief Retrieves the current log level bitmask.
 * @return The active log levels.
 */
int get_kb_log_level();

/**
 * @brief Logs a formatted message to stdout.
 * @param level The level of this specific message.
 * @param format Printf-style format string.
 * @param ... Additional arguments for the format string.
 */
void log_message(enum KB_LOG_LEVEL_FLAGS level, const char *format, ...);

#endif