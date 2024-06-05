# Benchmarks for pqNTRUSign

## Setup

- **Device:** NUCLEO-H753ZI
- **Compiler:** arm-none-eabi-gcc 10.3.1
- **Public key size:** 2065
- **Private key size:** 2604

Lower key sizes will not be tested as the cryptography is already considered broken even in the NIST submission variant.
See relevant Wikipedia article on [NTRUSign](https://en.wikipedia.org/wiki/NTRUSign).

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

## Memory footprint

### Overall usage

The memory footprint is calculated by substracting the ammount
of memory used by a blank application from the memory used by the
application with the cryptographic library.

| Compiler Preset | Variant         | Flash (bytes) | DTCRAM (bytes) | BUF RAM (bytes) | Total RAM (bytes) |
|-----------------|-----------------|---------------|----------------|-----------------|-------------------|
| Release         | Gaussian distr. | 57696         | 8344           | 225856          | 234200            |
| MinSizeRel      | Gaussian distr. | 52316         | 8344           | 225853          | 234197            |
| Release         | Uniform distr.  | 57340         | 8344           | 225856          | 234200            |
| MinSizeRel      | Uniform distr.  | 52204         | 8344           | 225853          | 234197            |

As one may see, the differences between the variants are not significant and the MinSizeRel
preset obviously uses less FLASH memory. On average ~235kB of RAM and ~55kB of FLASH memory is used
by the cryptographic library.

### Detailed FLASH usage per function

| File                    | Function                       | FLASH (bytes: hex) | FLASH (kB) |
|-------------------------|--------------------------------|--------------------|------------|
| crypto_hash_sha512.c    | crypto_hashblocks_sha512       | 00007c80           | 000031.5   |
| crypto_hash_sha512.c    | crypto_hash_sha512             | 00000158           | 000000.3   |
| crypto_sream_salsa20.c  | crypto_core_salsa20            | 0000038c           | 000000.9   |
| crypto_sream_salsa20.c  | crypto_sream                   | 000000ec           | 000000.2   |
| DGS.c.obj               | DGS                            | 00000188           | 000000.4   |
| DGS.c.obj               | DDGS                           | 00000110           | 000000.3   |
| fastrandombytes.c.obj   | fastrandombytes                | 0000007c           | 000000.2   |
| fastrandombytes.c.obj   | rng_cleanup                    | 00000040           | 000000.1   |
| fastrandombytes.c.obj   | rng_init                       | 00000088           | 000000.2   |
| fastrandombytes.c.obj   | rng_uint16                     | 000000b0           | 000000.2   |
| fastrandombytes.c.obj   | rng_uint64                     | 0000012c           | 000000.3   |
| fastrandombytes.c.obj   | randpool                       | 00001000           | 000004.0   |
| fastrandombytes.c.obj   | randpos                        | 00000002           | 000000.0   |
| misc.c.obj              | max_norm                       | 00000040           | 000000.1   |
| misc.c.obj              | get_scala                      | 00000050           | 000000.1   |
| NTT.c.obj               | extendedEuclid                 | 000000f0           | 000000.2   |
| NTT.c.obj               | NTT                            | 000001bc           | 000000.4   |
| NTT.c.obj               | Inv_NTT                        | 00000150           | 000000.3   |
| NTT.c.obj               | InvMod                         | 0000017c           | 000000.3   |
| packing.c.obj           | tri_to_string                  | 0000016c           | 000000.3   |
| packing.c.obj           | string_to_tri                  | 00000290           | 000000.6   |
| packing.c.obj           | pack_public_key                | 0000008c           | 000000.2   |
| packing.c.obj           | unpack_public_key              | 00000064           | 000000.1   |
| packing.c.obj           | pack_secret_key                | 000001fc           | 000000.5   |
| packing.c.obj           | unpack_secret_key              | 0000020c           | 000000.5   |
| param.c.obj             | pq_get_param_set_by_id         | 0000001c           | 000000.0   |
| poly.c.obj              | karatsuba                      | 00000288           | 000000.7   |
| poly.c.obj              | is_balance                     | 0000001c           | 000000.0   |
| poly.c.obj              | pol_gen_flat                   | 000004b8           | 000001.2   |
| poly.c.obj              | binary_poly_gen                | 00000214           | 000000.5   |
| poly.c.obj              | pol_unidrnd_pZ                 | 000000b0           | 000000.2   |
| poly.c.obj              | pol_unidrnd                    | 00000090           | 000000.1   |
| poly.c.obj              | pol_unidrnd_with_seed          | 000000cc           | 000000.3   |
| poly.c.obj              | pol_inv_mod2                   | 00000330           | 000000.8   |
| poly.c.obj              | cmod                           | 0000004c           | 000000.1   |
| poly.c.obj              | pol_mul_coefficients           | 000000c4           | 000000.2   |
| poly.c.obj              | pol_mul_mod_p                  | 000000d0           | 000000.2   |
| pqNTRUSign.c.obj        | normf                          | 00000108           | 000000.3   |
| pqNTRUSign.c.obj        | keygen                         | 00000148           | 000000.3   |
| pqNTRUSign.c.obj        | rejection_sampling             | 000000ec           | 000000.2   |
| pqNTRUSign.c.obj        | challenge                      | 000000f4           | 000000.2   |
| pqNTRUSign.c.obj        | sign                           | 000002bc           | 000000.7   |
| pqNTRUSign.c.obj        | verify                         | 00000130           | 000000.3   |
| rng.c.obj               | _32_randombytes                | 00000040           | 000000.1   |
| rng.c.obj               | rng_init                       | 00000088           | 000000.2   |
| rng.c.obj               | rng_uint16                     | 000000b0           | 000000.2   |
| rng.c.obj               | rng_uint64                     | 0000012c           | 000000.3   |

#### Per component sums

| File                    | FLASH (kB) |
|-------------------------|------------|
| crypto_hash_sha512.c    | 000031.8   |
| crypto_sream_salsa20.c  | 000001.1   |
| DGS.c.obj               | 000000.7   |
| fastrandombytes.c.obj   | 000004.5   |
| misc.c.obj              | 000000.2   |
| NTT.c.obj               | 000000.9   |
| packing.c.obj           | 000001.2   |
| param.c.obj             | 000000.0   |
| poly.c.obj              | 000004.6   |
| pqNTRUSign.c.obj        | 000001.5   |
| rng.c.obj               | 000000.6   |
| **Total**               | 000047.1   |

As one can easily see, the SHA512 implementation takes the most FLASH memory.
Using the hardware hash module would be a good idea to reduce the memory footprint.

The resulting size would be arround 15kB.

#### Script used

```bash
for file in *.obj; do echo $file; arm-none-eabi-objdump -h $file  | tail +6 | grep -E "^.*[0-9]" | awk '{print $2 " " $3}' | column -t ; done
```

## Other findings

### Broken signatures

An interesting observation is that for some combinations of generated keys and messages, the signature generation
may result in a signature that is not valid. This is circumvented by checking the signature before sending it back
to the caller. It is therefore possible to receive an error response even if correct parameters were passed.
Researching the reason behind this behaviour is out of the scope of this project as a workaround is already in place.

### Lack of memory safety in `crypto_sign_open()`

So apparently the `crypto_sign_open()` function will cause out of bounds memory access if anything is found
after the padding block of `0004`'s. Won't fix, sorry.
