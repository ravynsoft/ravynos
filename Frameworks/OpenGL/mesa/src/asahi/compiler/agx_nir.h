/*
 * Copyright 2023 Alyssa Rosenzweig
 * SPDX-License-Identifier: MIT
 */
#ifndef AGX_NIR_H
#define AGX_NIR_H

#include <stdbool.h>

struct nir_shader;

bool agx_nir_lower_interpolation(struct nir_shader *s);
bool agx_nir_opt_ixor_bcsel(struct nir_shader *shader);
bool agx_nir_lower_algebraic_late(struct nir_shader *shader);
bool agx_nir_fuse_algebraic_late(struct nir_shader *shader);
bool agx_nir_fence_images(struct nir_shader *shader);
bool agx_nir_lower_multisampled_image_store(struct nir_shader *s);
void agx_nir_lower_layer(struct nir_shader *s);
void agx_nir_lower_cull_distance_vs(struct nir_shader *s);
void agx_nir_lower_subgroups(struct nir_shader *s);

#endif
