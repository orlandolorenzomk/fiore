# Fiore

A FreeBSD-based distribution for running Spring Boot applications safely in production.

---

## Project Overview

Fiore is a purpose-built FreeBSD distribution that provides a minimal, hardened environment for hosting Spring Boot services in production. Rather than layering deployment tooling on top of a general-purpose OS, Fiore ships as a cohesive system where the operating environment and application lifecycle management are designed together.

The result is a predictable, low-overhead platform that teams can deploy on bare metal or virtual machines — with safe deploys, automatic recovery, and basic traffic distribution built in from the start.

---

## Who Is It For

Fiore is aimed at teams that need a reliable production environment for Spring Boot services without the complexity or cost of large-scale cloud infrastructure.

- **Small companies and startups** that want a straightforward, self-hosted backend environment and don't need the full weight of a container orchestration platform.
- **Teams looking to reduce infrastructure costs** by running on bare metal or simple VMs instead of managed Kubernetes clusters or container-as-a-service cloud offerings.

If your stack is Spring Boot and your goal is to keep operations simple and costs predictable, Fiore is built for you.

---

## Features

- **Safe Deploys** — Rolling deployment strategy that minimizes downtime and supports fast rollback if a release fails health checks.
- **Automatic Restarts** — Monitors application processes and restarts them automatically on crash or unresponsive state.
- **Resource Monitoring** — Tracks CPU, memory, and basic JVM metrics per application instance, with configurable alerting thresholds.
- **Basic Load Balancing** — Distributes incoming traffic across multiple instances of the same application using a round-robin strategy.

---

## Getting Started

### Prerequisites

- A bare-metal server or virtual machine capable of running FreeBSD
- One or more packaged Spring Boot applications (`.jar`)
- SSH access to the target host

### Deploying Applications

1. Provision a host with the Fiore image (see the internal wiki for image download and installation instructions).

2. Define your services in a YAML configuration file:

   ```yaml
   services:
     my-service:
       jar: /path/to/service.jar
       envFile: /path/to/my-service.env  # optional
       port: 8080
       dependencies:
         - cache
       restartPolicy: on-failure
       metrics:
         enable: true
         collectHeap: true
         collectThreads: true
       logging:
         stdout: true
         file: /var/log/my-service.log
       lb:
         replicas: 3
         strategy: round-robin

     cache:
       jar: /path/to/cache.jar
       port: 6379
       restartPolicy: on-failure
       metrics:
         enable: false
       logging:
         stdout: true
         file: /var/log/cache.log
   ```

3. Deploy all services defined in the configuration:

   ```bash
   fiore deploy /path/to/config.yml
   ```

4. Check the status of running services:

   ```bash
   fiore status
   ```

5. View resource usage for a specific service:

   ```bash
   fiore monitor --name my-service
   ```

Refer to the internal wiki for image provisioning, full configuration reference, environment variable support, and advanced deployment options.

---

## License

Internal use only. All rights reserved.
