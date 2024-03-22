/*
 * Copyright 2023 Intel Corporation
 * SPDX-License-Identifier: MIT
 */

#ifndef MESA_NIR_CLC_H
#define MESA_NIR_CLC_H

#include "nir.h"

#ifdef __cplusplus
extern "C" {
#endif

struct disk_cache;
struct spirv_to_nir_options;

bool nir_can_find_libclc(unsigned ptr_bit_size);

nir_shader *
nir_load_libclc_shader(unsigned ptr_bit_size,
                       struct disk_cache *disk_cache,
                       const struct spirv_to_nir_options *spirv_options,
                       const nir_shader_compiler_options *nir_options,
                       bool optimize);

#ifdef __cplusplus
}
#endif

#endif /* MESA_NIR_CLC_H */
