#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "help.h"
#include "command_dispatcher.h"
#include "command_exec.h"

int main(int argc, char *argv[]) {
    for (int i = 0; i < argc; i++) {
        printf("arg[%d]=%s\n", i, argv[i]);
    }

    if (argc < 3) {
        fprintf(stderr, "ERROR: Invalid usage of jpack\n\n");
        print_help();
        return EXIT_FAILURE;
    }

    const char *command = argv[1];
    const char *package = argv[2];
    const char *version = NULL;
    bool list_remote = false;

    if (strcmp(command, "list") == 0) {
        for (int i = 1; i < argc; i++) {
            if (strcmp(argv[i], "--remote") == 0) {
                list_remote = true;
                break;
            }
        }
    } else if (argc >= 4) {
        version = argv[3];
    }

    dispatch(command, package, version, list_remote);

    return EXIT_SUCCESS;
}
