#include "command_dispatcher.h"
#include "command_exec.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static bool is_package_valid(const char *package);
static bool is_command_valid(const char *command);

static const char *commands[] = {
    COMMAND_LIST,
    COMMAND_INSTALL,
    COMMAND_USE,
    COMMAND_DELETE
};

static const char *packages[] = {
    PACKAGE_JAVA,
    PACKAGE_MVN,
    PACKAGE_GRADLE
};

static const size_t num_commands = sizeof(commands) / sizeof(commands[0]);
static const size_t num_packages = sizeof(packages) / sizeof(packages[0]);

void dispatch(const char *command, const char *package, const char *version, bool list_remote) {
    if (!is_command_valid(command)) {
        printf("Command %s is not valid\n", command);
        return;
    }

    if (!is_package_valid(package)) {
        printf("Package %s is not valid\n", command);
        return;
    }

    if (strcmp(command, COMMAND_LIST) == 0) {
        cmd_list(package, list_remote);
    } else if (strcmp(command, COMMAND_USE) == 0) {
        cmd_use(package, version);
    } else if (strcmp(command, COMMAND_INSTALL) == 0) {
        cmd_install(package, version);
    } else if (strcmp(command, COMMAND_DELETE) == 0) {
        cmd_del(package, version);
    }
}

static bool is_package_valid(const char *package) {
    for (size_t i = 0; i < num_packages; i++)
        if (strcmp(package, packages[i]) == 0) 
            return true;
    return false;
}

static bool is_command_valid(const char *command) {
    for (size_t i = 0; i < num_commands; i++) 
        if (strcmp(command, commands[i]) == 0)
            return true;
    return false;
}