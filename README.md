# ProteusKernel

**Multi-Agent Coordination Engine**

A lightweight C++ toolkit for distributed agent discovery, heartbeat mesh networking, and consensus arbitration. Built for developers who need sovereign infrastructure for multi-LLM or autonomous agent systems.

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://en.cppreference.com/w/cpp/17)
[![OpenSSL 3.0](https://img.shields.io/badge/OpenSSL-3.0-green.svg)](https://www.openssl.org/)

---

## Overview

ProteusKernel provides five focused utilities for building agent swarms:

| Binary | Purpose |
|--------|---------|
| `pk_heartbeat` | UDP heartbeat daemon for mesh discovery and liveness detection |
| `pk_zayden` | Multi-model API orchestrator and request router |
| `pk_gotem` | SHA-256 hashing utility for message integrity |
| `pk_swarm` | Core swarm logic — agent coordination and state synchronization |
| `pk_push` | State propagation and broadcast dispatcher |

**Key feature:** The **Truce Protocol** — a lightweight negotiation mechanism for resolving goal conflicts between autonomous agents via reputation scoring and structured escalation.

---

## Quick Start

### Build

```bash
git clone https://github.com/Admin135158/ProteusKernel.git
cd ProteusKernel
make clean && make
```

### Run

```bash
./pk_heartbeat --port 9161

./pk_swarm --target 127.0.0.1:9161
```

### Dependencies

- **OpenSSL 3.0+** (EVP API for cryptographic operations)
- **pthread** (POSIX threading)
- **C++17** compiler (GCC 8+, Clang 7+, MSVC 2019+)

---

## Architecture

```
ProteusKernel/
├── src/
│   ├── pk_heartbeat.cpp      # UDP gossip/discovery
│   ├── pk_zayden.cpp         # Multi-model routing
│   ├── pk_gotem.cpp          # SHA-256 integrity (EVP API)
│   ├── pk_swarm.cpp          # Swarm coordination
│   ├── pk_push.cpp           # State broadcast
│   └── truce/
│       ├── truce_types.h
│       ├── truce_protocol.h
│       └── truce_protocol.cpp
├── include/                  # Public headers
├── scripts/                  # Build and deploy helpers
├── Makefile
└── README.md
```

---

## Truce Protocol

The Truce Protocol (`C_social(t)`) is an experimental mechanism for multi-agent conflict resolution. When agents with incompatible goals encounter each other, the protocol:

1. **Negotiates** — exchanges goal hashes and reputation scores
2. **Merges** — attempts deterministic goal unification
3. **Escalates** — surfaces unresolved conflicts to human operators

**Status:** Implemented. Under active validation against multi-agent adversarial benchmarks.

> **Note:** The Truce Protocol was developed in response to observed multi-agent conflict behaviors in LLM swarms. It is an independent implementation, not affiliated with or validated by Anthropic.

---

## Research

ProteusKernel is informed by ongoing theoretical work on observer-relative frameworks for autonomous systems. These research directions are documented separately and are **not** peer-reviewed science:

- **FTCoE** (Fundamental Theory of Conscious Energy) — OSF registration [a3bwg](https://osf.io/a3bwg)
- **GORF** (Geometric Ollin Resonance Framework) — mathematical models for system coherence
- **GGSE** (Garcia–González Cognitive Engine) — cognitive architecture for agent self-modeling

See [`docs/RESEARCH.md`](docs/RESEARCH.md) for the theoretical supplement. The engineering code in this repo does not depend on these frameworks.

---

## Contributing

We welcome contributions that improve the engineering: performance, security, portability, and documentation.

- **Bug reports:** Open an issue with reproduction steps
- **Feature requests:** Open an issue with use case and proposed API
- **Pull requests:** Fork, branch, test with `make clean && make`, submit

See [`CONTRIBUTING.md`](CONTRIBUTING.md) for details.

---

## Security

- Report vulnerabilities to **fernaathebeast@gmail.com**
- 90-day responsible disclosure policy
- See [`SECURITY.md`](SECURITY.md) for details

---

## License

MIT License — see [`LICENSE`](LICENSE)

Copyright (c) 2026 Fernando De Jesus Garcia Gonzalez

---

## Acknowledgments

- **Deathburgerz013** — early contributor, HOLO-Invariant bridge research
- **Google Developer Program** — tooling and cloud credits
- **NVIDIA Developer** — GPU acceleration research

```
