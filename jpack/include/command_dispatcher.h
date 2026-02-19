#ifndef COMMAND_DISPATCHER_H
#define COMMAND_DISPATCHER_H

#include <stdbool.h>

void dispatch(const char *command, const char *package, const char *version, bool list_remote);

#endif // COMMAND_DISPATCHER_H