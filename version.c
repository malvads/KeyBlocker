/**
 * @file version.c
 * @brief Handles software versioning and update checks for KeyBlocker.
 */
#include "version.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief Current software version.
 */
const char *KB_VERSION = "1.2";

/** 
 * @brief URL of the remote version file.
 */
#define REMOTE_VERSION_URL "https://raw.githubusercontent.com/malvads/KeyBlocker/main/version.c"

/**
 * @brief Get the current software version.
 *
 * @return A string containing the current version.
 */
const char *get_version() {
    return KB_VERSION;
}

/**
 * @brief Retrieve the latest version from the remote source.
 *
 * This function uses `curl` to fetch the remote `version.h` file
 * and parses the `KB_VERSION` string from it.
 *
 * @return A string containing the remote version, or NULL if an error occurs.
 */
const char *get_remote_version() {
    static char version[64];
    FILE *fp = popen("curl -s " REMOTE_VERSION_URL " | grep KB_VERSION", "r");
    if (!fp) return NULL;

    if (fgets(version, sizeof(version), fp)) {
        char *start = strchr(version, '"');
        char *end = NULL;
        if (start) end = strchr(start + 1, '"');

        pclose(fp);

        if (!start || !end) return NULL;
        *end = '\0';
        return start + 1;
    }

    pclose(fp);
    return NULL;
}

/**
 * @brief Compare two semantic version strings numerically.
 *
 * Returns:
 *  -1 if a < b
 *   0 if a == b
 *   1 if a > b
 */
int compare_versions(const char *a, const char *b) {
    while (*a != '\0' || *b != '\0') {
        long va = 0, vb = 0;

        if (*a != '\0') {
            char *nexta;
            va = strtol(a, &nexta, 10);
            a = (*nexta == '.') ? nexta + 1 : nexta;
        }

        if (*b != '\0') {
            char *nextb;
            vb = strtol(b, &nextb, 10);
            b = (*nextb == '.') ? nextb + 1 : nextb;
        }

        if (va < vb) return -1;
        if (va > vb) return 1;
    }

    return 0;
}

/**
 * @brief Check whether a software update is available.
 *
 * Compares the current version with the remote version numerically.
 *
 * @return UPDATE_STATUS_CORRECT if up-to-date,
 *         UPDATE_STATUS_OUTDATED if a newer version exists,
 *         UPDATE_STATUS_ERROR if unable to check.
 */
enum UPDATE_STATUS is_update_available() {
    const char *remote_version = get_remote_version();
    if (!remote_version) return UPDATE_STATUS_ERROR;

    int cmp = compare_versions(remote_version, KB_VERSION);
    return (cmp > 0) ? UPDATE_STATUS_OUTDATED : UPDATE_STATUS_CORRECT;
}
