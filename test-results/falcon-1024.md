# Benchmarks for Falcon-1024

## Setup

- **Device:** NUCLEO-H753ZI
- **Compiler:** arm-none-eabi-gcc 10.3.1
- **Public key size:** 1793
- **Private key size:** 2305

Communication as in [pqNTRUSign.md](pqNTRUSign.md).

## Key generation over 50 runs

| Compiler Preset | Time (ms) |
|-----------------|-----------|
| Release         |  711      |
| MinSizeRel      |  853      |
| Debug           |  2504     |

## Signature generation over 50 runs

| Compiler Preset | Time (ms) |
|-----------------|-----------|
| Release         |  134      |
| MinSizeRel      |  135      |
| Debug           |  218      |

## Memory footprint

### Overall usage

| Compiler Preset | Flash (bytes) | Total RAM (bytes) |
|-----------------|---------------|-------------------|
| Release         |  76400        |  128008           |
| MinSizeRel      |  60280        |  128002           |
| Debug           |  82132        |  128005           |

As the memory usage pattern is expected to be similar to [falcon-512](falcon-512.md),
`.obj` file analysis is omitted (buffer sizes will change and that will mostly be it).
