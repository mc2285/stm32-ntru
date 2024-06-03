# Benchmarks for pqNTRUSign

## Setup

- **Device:** NUCLEO-H753ZI
- **Compiler:** arm-none-eabi-gcc 10.3.1

To communicate with the device, a serial terminal is required.

On linux the device is usually located at `/dev/ttyACMx`. See `journalctl -efn10` for the exact device name.

Example command to start a serial terminal:

```bash
picocom --omap crcrlf --echo -b 115200 /dev/ttyACM0
```

All the commands are expected to be terminated with a newline or carret return **and** newline.

## Key generation over 25 runs

| Compiler Preset | Time (ms) |
|-----------------|-----------|
| Release         | 818       |
| MinSizeRel      | 875       |
| Debug           | 1465      |

### Running the keygen benchmark

In a serial terminal, run the following command:

`AT+R`

## Sign over 25 runs

| Variant         | Compiler Preset | Time (ms) |
|-----------------|-----------------|-----------|
| Gaussian distr. | Release         | 1598      |
| Gaussian distr. | MinSizeRel      | 2029      |
| Gaussian distr. | Debug           | 7596      |
| Uniform distr.  | Release         | 0.000     |
| Uniform distr.  | MinSizeRel      | 0.000     |
| Uniform distr.  | Debug           | 0.000     |

### Running the sign benchmark

In a serial terminal, run the following command:

`AT+B`
