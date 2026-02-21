# Supervisor

A lightweight process supervisor for Spring Boot applications, written in C. Part of the [Fiore](../README.md) platform.

---

## Overview

The Supervisor is the process lifecycle manager at the core of Fiore. It launches Spring Boot JARs as detached background processes, tracks their runtime state in a persistent process table, and automatically restarts them based on a configurable restart policy.

It is designed to be simple, with no runtime dependencies beyond a standard C library and a JVM on the target host.

---

## Project Structure

```
supervisor/
├── src/
│   ├── main.c            # CLI entry point and command dispatch
│   ├── supervisor.c      # Process lifecycle: start, stop, restart, status, monitor
│   ├── process_table.c   # Persistent linked-list process table (binary file)
│   └── logger.c          # Append-only file logger
├── include/
│   ├── supervisor.h
│   ├── process_table.h
│   └── logger.h
├── state/
│   └── processes.dat     # Binary process table persisted across invocations
├── logs/
│   ├── supervisor.log    # Internal supervisor log
│   └── process_table.log # Process table operation log
├── bin/
│   └── supervisor        # Compiled binary
├── Makefile
└── deploy.sh             # Build-and-deploy script (rsync + gmake over SSH)
```

---

## Building

Requires a C11-capable compiler (`cc`) and GNU Make (`gmake` on FreeBSD).

```bash
gmake
```

The compiled binary is placed at `bin/supervisor`.

To install it system-wide:

```bash
gmake install   # installs to /usr/local/bin/supervisor
```

To clean build artefacts:

```bash
gmake clean
```

---

## Usage

```
supervisor start   <name> <jar> [--port <port>] [--restart never|on-failure|always] [--env <file>] [--log <file>]
supervisor stop    <name>
supervisor restart <name>
supervisor status  [<name>]
supervisor list
supervisor monitor
supervisor remove  <name>
```

### Commands

| Command | Description |
|---|---|
| `start` | Launch a JAR as a managed background process. |
| `stop` | Send SIGTERM to a running process (escalates to SIGKILL after a grace period). |
| `restart` | Stop then re-launch a process, incrementing its restart counter. |
| `status` | Print live status for one service, or a table for all services. |
| `list` | List all registered services with their current running state. |
| `monitor` | Check all processes once and restart any that are down, according to their restart policy. |
| `remove` | Stop a service (if running) and remove it from the process table entirely. |

### Options for `start`

| Flag | Description |
|---|---|
| `--port <port>` | Port passed to Spring Boot via `--server.port=<port>`. |
| `--restart <policy>` | One of `never`, `on-failure` (default), or `always`. |
| `--env <file>` | Path to a `.env` file loaded into the process environment before exec. |
| `--log <file>` | Path to a log file where the process's stdout and stderr are written. |

---

## Examples

**Start a service:**
```bash
supervisor start my-service /opt/apps/my-service.jar \
    --port 8080 \
    --restart on-failure \
    --env /opt/apps/my-service.env \
    --log /var/log/my-service.log
```

**Check status of all services:**
```bash
supervisor status
```

**Watch and restart crashed processes:**
```bash
supervisor monitor
```

**Stop and remove a service:**
```bash
supervisor stop my-service
supervisor remove my-service
```

---

## Restart Policies

| Policy | Behaviour |
|---|---|
| `never` | The process is never restarted after it exits. |
| `on-failure` | The process is restarted only if it exits with a non-zero status (default). |
| `always` | The process is always restarted regardless of exit status. |

Restart policies are applied by the `monitor` command, which is intended to be run periodically (e.g. from a cron job or a supervising daemon).

---

## Deployment

`deploy.sh` builds the binary locally, syncs the source tree to the remote Fiore host over rsync, recompiles it there with `gmake`, and runs the given command — all in one step.

```bash
./deploy.sh start my-service /root/my-service.jar \
    --port 8080 \
    --restart on-failure \
    --env /root/my-service.env \
    --log /var/log/my-service.log
```

The remote host, user, and directory are configured at the top of `deploy.sh`.

---

## License

Internal use only. All rights reserved.
