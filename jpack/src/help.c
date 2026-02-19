#include "help.h"
#include <stdio.h>

void print_help() {
    printf(BOLD CYAN "jpack" RESET " - Java package manager\n\n");

    printf(BOLD "Usage:\n" RESET);
    printf("  " GREEN "jpack list <package>" RESET "        List available versions of a package\n");
    printf("  " GREEN "jpack install <package> <version>" RESET "  Install a specific version\n");
    printf("  " GREEN "jpack use <package> <version>" RESET "      Switch to a specific version\n");
    printf("  " GREEN "jpack del <package> <version>" RESET "      Delete a specific version\n\n");

    printf(BOLD "Packages:\n" RESET);
    printf("  java, mvn, gradle\n\n");

    printf(BOLD "Examples:\n" RESET);
    printf("  " YELLOW "jpack list java" RESET "\n");
    printf("  " YELLOW "jpack install mvn 3.8.6" RESET "\n");
    printf("  " YELLOW "jpack use gradle 7.2" RESET "\n");
    printf("  " YELLOW "jpack del java 17.0.2" RESET "\n");
}
