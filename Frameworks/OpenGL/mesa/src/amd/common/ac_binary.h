/*
 * Copyright 2014 Advanced Micro Devices, Inc.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef AC_BINARY_H
#define AC_BINARY_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct radeon_info;

struct ac_shader_config {
   unsigned num_sgprs;
   unsigned num_vgprs;
   unsigned num_shared_vgprs; /* GFX10: number of VGPRs shared between half-waves */
   unsigned spilled_sgprs;
   unsigned spilled_vgprs;
   unsigned lds_size; /* in HW allocation units; i.e 256 bytes on SI, 512 bytes on CI+ */
   unsigned spi_ps_input_ena;
   unsigned spi_ps_input_addr;
   unsigned float_mode;
   unsigned scratch_bytes_per_wave;
   unsigned rsrc1;
   unsigned rsrc2;
   unsigned rsrc3;
};

void ac_parse_shader_binary_config(const char *data, size_t nbytes, unsigned wave_size,
                                   const struct radeon_info *info, struct ac_shader_config *conf);

unsigned ac_align_shader_binary_for_prefetch(const struct radeon_info *info, unsigned size);

#ifdef __cplusplus
}
#endif

#endif /* AC_BINARY_H */
