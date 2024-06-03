# Benchmarks for pqNTRUSign

## Setup

- **Device:** NUCLEO-H753ZI
- **Compiler:** arm-none-eabi-gcc 10.3.1
- **Public key size:** 2065
- **Private key size:** 2604

Lower key sizes will not be tested as the cryptography is already considered broken even in the NIST submission variant. See relevant Wikipedia article on [NTRUSign](https://en.wikipedia.org/wiki/NTRUSign).

To communicate with the device, a serial terminal is required.
On Linux the device is usually located at `/dev/ttyACMx`. See `journalctl -efn10` for the exact device name.

Example command to start a serial terminal:

```bash
picocom --omap crcrlf --echo -b 115200 /dev/ttyACM0
```

All the `AT+x` commands are expected to be terminated with a newline or carret return **and** newline.

## Key generation over 25 runs

| Compiler Preset | Time (ms) |
|-----------------|-----------|
| Release         | 818       |
| MinSizeRel      | 875       |
| Debug           | 1465      |

### Running the keygen benchmark

In a serial terminal, run the following command:

`AT+R`

## Signature generation over 25 runs

| Variant         | Compiler Preset | Time (ms) |
|-----------------|-----------------|-----------|
| Gaussian distr. | Debug           | 7596      |
| Gaussian distr. | MinSizeRel      | 2029      |
| Gaussian distr. | Release         | 1598      |
| Uniform distr.  | Debug           | 2749      |
| Uniform distr.  | MinSizeRel      | 627       |
| Uniform distr.  | Release         | 522       |

### Running the signature benchmark

In a serial terminal, run the following command:

`AT+B`
