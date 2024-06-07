# Benchmarks for Falcon-512

## Setup

- **Device:** NUCLEO-H753ZI
- **Compiler:** arm-none-eabi-gcc 10.3.1
- **Public key size:** 897
- **Private key size:** 1281

Communication as in [pqNTRUSign.md](pqNTRUSign.md).

## Key generation over 50 runs

| Compiler Preset | Time (ms) |
|-----------------|-----------|
| Release         | 266       |
| MinSizeRel      | 308       |
| Debug           | 808       |

## Signature generation over 50 runs

| Compiler Preset | Time (ms) |
|-----------------|-----------|
| Release         | 61        |
| MinSizeRel      | 61        |
| Debug           | 101       |

## Memory footprint

### Overall usage

| Compiler Preset | FLASH (bytes) | Total RAM (bytes) |
|-----------------|---------------|-------------------|
| Release         | 76292         | 64514             |
| MinSizeRel      | 60212         | 64514             |
| Debug           | 82048         | 64538             |

### Size of individual symbols in object files (Release)

| File                    | Symbol                             | FLASH (bytes: hex) | FLASH (kB) |
|-------------------------|------------------------------------|--------------------|------------|
| api.c.obj               | crypto_sign_keypair                | 000000f0           | 0.23       |
| api.c.obj               | crypto_sign                        | 000001f0           | 0.48       |
| api.c.obj               | crypto_sign_open                   | 000000e4           | 0.22       |
| codec.c.obj             | falcon_inner_modq_encode           | 00000094           | 0.15       |
| codec.c.obj             | falcon_inner_modq_decode           | 00000070           | 0.11       |
| codec.c.obj             | falcon_inner_trim_i8_encode        | 000000b8           | 0.18       |
| codec.c.obj             | falcon_inner_trim_i8_decode        | 000000b4           | 0.18       |
| codec.c.obj             | falcon_inner_comp_encode           | 000000c8           | 0.19       |
| codec.c.obj             | falcon_inner_comp_decode           | 0000009c           | 0.16       |
| codec.c.obj             | falcon_inner_max_FG_bits           | 0000000b           | 0.01       |
| codec.c.obj             | falcon_inner_max_fg_bits           | 0000000b           | 0.01       |
| common.c.obj            | falcon_inner_hash_to_point_vartime | 00000074           | 0.12       |
| common.c.obj            | falcon_inner_is_short              | 00000050           | 0.08       |
| common.c.obj            | falcon_inner_is_short_half         | 00000048           | 0.07       |
| common.c.obj            | l2bound                            | 0000002c           | 0.04       |
| common.c.obj            | overtab.0                          | 00000016           | 0.03       |
| fft.c.obj               | falcon_inner_FFT                   | 000001d0           | 0.45       |
| fft.c.obj               | falcon_inner_iFFT                  | 0000021c           | 0.53       |
| fft.c.obj               | falcon_inner_poly_add              | 0000002c           | 0.04       |
| fft.c.obj               | falcon_inner_poly_sub              | 00000030           | 0.05       |
| fft.c.obj               | falcon_inner_poly_neg              | 00000020           | 0.03       |
| fft.c.obj               | falcon_inner_poly_adj_fft          | 00000034           | 0.05       |
| fft.c.obj               | falcon_inner_poly_mul_fft          | 000000c8           | 0.20       |
| fft.c.obj               | falcon_inner_poly_muladj_fft       | 000000d4           | 0.21       |
| fft.c.obj               | falcon_inner_poly_mulselfadj_fft   | 00000078           | 0.12       |
| fft.c.obj               | falcon_inner_poly_mulconst         | 00000030           | 0.05       |
| fft.c.obj               | falcon_inner_poly_invnorm2_fft     | 000000ec           | 0.23       |
| fft.c.obj               | falcon_inner_poly_add_muladj_fft   | 000001a8           | 0.41       |
| fft.c.obj               | falcon_inner_poly_mul_autoadj_fft  | 00000050           | 0.08       |
| fft.c.obj               | falcon_inner_poly_div_autoadj_fft  | 0000006c           | 0.11       |
| fft.c.obj               | falcon_inner_poly_LDL_fft          | 0000020c           | 0.51       |
| fft.c.obj               | falcon_inner_poly_split_fft        | 000001e0           | 0.47       |
| fft.c.obj               | falcon_inner_poly_merge_fft        | 00000174           | 0.37       |
| fpr.c.obj               | falcon_inner_fpr_scaled            | 00000084           | 0.13       |
| fpr.c.obj               | falcon_inner_fpr_add               | 0000013c           | 0.31       |
| fpr.c.obj               | falcon_inner_fpr_mul               | 000000e8           | 0.23       |
| fpr.c.obj               | falcon_inner_fpr_div               | 00000640           | 1.56       |
| fpr.c.obj               | falcon_inner_fpr_sqrt              | 00000748           | 1.82       |
| fpr.c.obj               | falcon_inner_fpr_expm_p63          | 000004e8           | 1.23       |
| fpr.c.obj               | falcon_inner_fpr_gm_tab            | 00004000           | 16.00      |
| fpr.c.obj               | falcon_inner_fpr_p2_tab            | 00000058           | 0.09       |
| keygen.c.obj            | modp_R2                            | 000000d4           | 0.21       |
| keygen.c.obj            | modp_mkgm2                         | 0000034c           | 0.82       |
| keygen.c.obj            | poly_sub_scaled                    | 00000144           | 0.33       |
| keygen.c.obj            | poly_small_mkgauss                 | 00000110           | 0.26       |
| keygen.c.obj            | poly_big_to_fp                     | 000000f8           | 0.25       |
| keygen.c.obj            | modp_iNTT2_ext.part.0              | 0000021c           | 0.53       |
| keygen.c.obj            | modp_NTT2_ext.part.0.constprop.0   | 000000e0           | 0.22       |
| keygen.c.obj            | modp_iNTT2_ext.part.0.constprop.0  | 0000013c           | 0.31       |
| keygen.c.obj            | modp_iNTT2_ext.part.0.constprop.1  | 00000130           | 0.30       |
| keygen.c.obj            | poly_big_to_fp.constprop.0         | 00000080           | 0.13       |
| keygen.c.obj            | poly_big_to_fp.constprop.1         | 000000b4           | 0.18       |
| keygen.c.obj            | zint_rebuild_CRT.constprop.3       | 00000260           | 0.60       |
| keygen.c.obj            | poly_sub_scaled_ntt                | 00000498           | 1.15       |
| keygen.c.obj            | zint_co_reduce_mod                 | 00000228           | 0.54       |
| keygen.c.obj            | make_fg                            | 00000a40           | 2.56       |
| keygen.c.obj            | solve_NTRU_intermediate            | 00000f24           | 3.79       |
| keygen.c.obj            | falcon_inner_keygen                | 00002d1c           | 11.28      |
| keygen.c.obj            | BITLENGTH                          | 00000058           | 0.09       |
| keygen.c.obj            | MAX_BL_LARGE                       | 00000028           | 0.04       |
| keygen.c.obj            | MAX_BL_SMALL                       | 0000002c           | 0.05       |
| keygen.c.obj            | PRIMES                             | 00001878           | 6.12       |
| keygen.c.obj            | REV10                              | 00000800           | 2.00       |
| keygen.c.obj            | gauss_1024_12289                   | 000000d8           | 0.21       |
| rng.c.obj               | falcon_inner_prng_refill           | 00000344           | 0.82       |
| rng.c.obj               | falcon_inner_prng_init             | 0000001c           | 0.03       |
| shake.c.obj             | process_block                      | 000008b4           | 2.18       |
| shake.c.obj             | falcon_inner_i_shake256_init       | 00000018           | 0.03       |
| shake.c.obj             | falcon_inner_i_shake256_inject     | 000000d0           | 0.20       |
| shake.c.obj             | falcon_inner_i_shake256_flip       | 00000030           | 0.05       |
| shake.c.obj             | falcon_inner_i_shake256_extract    | 00000060           | 0.10       |
| sign.c.obj              | falcon_inner_gaussian0_sampler     | 000000c0           | 0.19       |
| sign.c.obj              | falcon_inner_sampler               | 000002d0           | 0.70       |
| sign.c.obj              | ffSampling_fft_dyntree.constprop.0 | 000001f0           | 0.48       |
| sign.c.obj              | falcon_inner_sign_dyn              | 00000770           | 1.86       |
| sign.c.obj              | dist.0                             | 000000d8           | 0.22       |
| sign.c.obj              | fpr_inv_sigma                      | 00000058           | 0.09       |
| sign.c.obj              | fpr_sigma_min                      | 00000058           | 0.09       |
| vrfy.c.obj              | mq_NTT                             | 000000d4           | 0.21       |
| vrfy.c.obj              | mq_iNTT                            | 00000138           | 0.30       |
| vrfy.c.obj              | falcon_inner_to_ntt_monty          | 00000054           | 0.09       |
| vrfy.c.obj              | falcon_inner_verify_raw            | 00000100           | 0.25       |
| vrfy.c.obj              | falcon_inner_compute_public        | 00000364           | 0.85       |
| vrfy.c.obj              | falcon_inner_complete_private      | 00000458           | 1.09       |
| vrfy.c.obj              | GMb                                | 00000800           | 2.00       |
| vrfy.c.obj              | iGMb                               | 00000800           | 2.00       |
| **TOTAL**               |                                    |                    | **73.34**  |

### Per component sums (Release)

| File                    | FLASH (kB) |
|-------------------------|------------|
| api.c.obj               | 0.93       |
| codec.c.obj             | 0.99       |
| common.c.obj            | 0.34       |
| fft.c.obj               | 3.91       |
| fpr.c.obj               | 21.37      |
| keygen.c.obj            | 31.97      |
| rng.c.obj               | 0.85       |
| shake.c.obj             | 2.56       |
| sign.c.obj              | 3.63       |
| vrfy.c.obj              | 6.79       |
| **TOTAL**               | **73.34**  |
