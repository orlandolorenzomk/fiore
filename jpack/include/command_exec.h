#ifndef COMMAND_EXEC_H
#define  COMMAND_EXEC_H

#define COMMAND_LIST        "list"    /* list installed versions of a package (use --remote for available upstream versions) */
#define COMMAND_INSTALL     "install" /* download and install a specific version of a package */
#define COMMAND_USE         "use"     /* switch the active version of an installed package */
#define COMMAND_DELETE      "delete"  /* remove a specific installed version of a package */

#define PACKAGE_JAVA        "java"    /* OpenJDK / JDK distributions */
#define PACKAGE_MVN         "mvn"     /* Apache Maven build tool */
#define PACKAGE_GRADLE      "gradle"  /* Gradle build tool */

#include <stdbool.h>

/**
 * @brief List versions of the given package.
 *
 * When @p list_remote is false, scans the local jpack installation
 * directory and prints every version that has been installed on this
 * system. When @p list_remote is true, queries the upstream distribution
 * and prints all versions available for download instead.
 *
 * @param package     The package to query (PACKAGE_JAVA | PACKAGE_MVN | PACKAGE_GRADLE).
 * @param list_remote If true, show remote available versions; if false, show local installed versions.
 */
void cmd_list(const char *package, bool list_remote);

/**
 * @brief Download and install a specific version of a package.
 *
 * Fetches the requested @p version of @p package from the upstream
 * distribution and installs it into the jpack-managed directory.
 * Does not switch the active version automatically; use cmd_use() for that.
 *
 * @param package The package to install (PACKAGE_JAVA | PACKAGE_MVN | PACKAGE_GRADLE).
 * @param version The version string to install (e.g. "17.0.2", "3.8.6").
 */
void cmd_install(const char *package, const char *version);

/**
 * @brief Switch the active version of an installed package.
 *
 * Updates the symlink (or equivalent) so that @p version of @p package
 * becomes the default one resolved in the user's PATH. The requested
 * version must already be installed via cmd_install().
 *
 * @param package The package to switch (PACKAGE_JAVA | PACKAGE_MVN | PACKAGE_GRADLE).
 * @param version The version string to activate (e.g. "17.0.2", "3.8.6").
 */
void cmd_use(const char *package, const char *version);

/**
 * @brief Remove a specific installed version of a package.
 *
 * Permanently deletes the installation directory for @p version of
 * @p package. If @p version is currently active, the active symlink
 * is also removed and the user must run cmd_use() to select another version.
 *
 * @param package The package to modify (PACKAGE_JAVA | PACKAGE_MVN | PACKAGE_GRADLE).
 * @param version The version string to delete (e.g. "17.0.2", "3.8.6").
 */
void cmd_del(const char *package, const char *version);

#endif // COMMAND_EXEC_H