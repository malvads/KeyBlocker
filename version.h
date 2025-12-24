#ifndef VERSION_H
#define VERSION_H

/**
 * @file version.h
 * @brief Defines software versioning and update checking functionality.
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @enum UPDATE_STATUS
 * @brief Represents the status of the software update.
 */
enum UPDATE_STATUS {
    UPDATE_STATUS_CORRECT = 0,  /**< Software is up-to-date */
    UPDATE_STATUS_OUTDATED = 1, /**< Software version is outdated */
    UPDATE_STATUS_ERROR = 2     /**< Error occurred while checking for updates */
};

/**
 * @brief Current version of the software.
 *
 * This variable should be defined in the corresponding source file
 * and holds the version string of the currently compiled software.
 */
extern const char *KB_VERSION;

/**
 * @brief Get the current software version.
 *
 * @return A string representing the current version.
 */
const char *get_version();

/**
 * @brief Get the latest available version from a remote source.
 *
 * @return A string representing the latest remote version.
 */
const char *get_remote_version();

/**
 * @brief Check if a software update is available.
 *
 * This function compares the current software version with the remote version
 * and returns an appropriate update status.
 *
 * @return UPDATE_STATUS indicating if the software is up-to-date, outdated, or if an error occurred.
 */
enum UPDATE_STATUS is_update_available();

#ifdef __cplusplus
}
#endif

#endif // VERSION_H
