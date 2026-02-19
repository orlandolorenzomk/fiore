#include "command_exec.h"
#include "ftp_client.h"

#include <string.h>
#include <stdio.h>

static bool exec_java_list(const char *package, bool list_remote);
// static bool exec_java_install();
// static bool exec_java_use();
// static bool exec_java_del();

static bool exec_mvn_list(const char *package, bool list_remote);
// static bool exec_mvn_install();
// static bool exec_mvn_use();
// static bool exec_mvn_del();

static bool exec_gradle_list(const char *package, bool list_remote);
// static bool exec_gradle_install();
// static bool exec_gradle_use();
// static bool exec_gradle_del();

/* =============================== BEGIN CMD LIST =============================== */
void cmd_list(const char *package, bool list_remote) {
    if (strcmp(package, PACKAGE_JAVA) == 0) {
        exec_java_list(package, list_remote);
    } else if (strcmp(package, PACKAGE_MVN) == 0) {
        exec_mvn_list(package, list_remote);
    } else if (strcmp(package, PACKAGE_GRADLE) == 0) {
        exec_gradle_list(package, list_remote);
    }
}

static bool exec_java_list(const char *package, bool list_remote) {
    (void) list_remote;
    char remote_path[256];
    snprintf(remote_path, sizeof(remote_path), "%s/%s/", FTP_USER, package);
    ftp_list_dir(remote_path);
    return true;
}

static bool exec_mvn_list(const char *package, bool list_remote) {
    (void) list_remote;
    char remote_path[256];
    snprintf(remote_path, sizeof(remote_path), "%s/%s/", FTP_USER, package);
    ftp_list_dir(remote_path);
    return true;
}

static bool exec_gradle_list(const char *package, bool list_remote) {
    (void) list_remote;
    char remote_path[256];
    snprintf(remote_path, sizeof(remote_path), "%s/%s/", FTP_USER, package);
    ftp_list_dir(remote_path);
    return true;
}
/* =============================== END CMD LIST   =============================== */

/* =============================== BEGIN CMD INSTALL =============================== */
void cmd_install(const char *package, const char *version) {

}

// static bool exec_java_install() {

// }

// static bool exec_mvn_install() {

// }

// static bool exec_gradle_install() {

// }
/* =============================== END CMD INSTALL   =============================== */

/* =============================== BEGIN CMD USE =============================== */
void cmd_use(const char *package, const char *version) {

}

// static bool exec_java_use() {

// }

// static bool exec_mvn_use() {

// }

// static bool exec_gradle_use() {

// }
/* =============================== END CMD USE   =============================== */

/* =============================== BEGIN CMD DEL =============================== */
void cmd_del(const char *package, const char *version) {

}

// static bool exec_java_del() {

// }

// static bool exec_mvn_del() {

// }

// static bool exec_gradle_del() {

// }
/* =============================== END CMD DEL   =============================== */