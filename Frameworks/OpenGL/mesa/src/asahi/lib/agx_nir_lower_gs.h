/*
 * Copyright 2023 Alyssa Rosenzweig
 * SPDX-License-Identifier: MIT
 */

#ifndef __AGX_NIR_LOWER_GS_H
#define __AGX_NIR_LOWER_GS_H

#include <stdbool.h>

struct nir_shader;
struct agx_ia_key;
enum mesa_prim;

void agx_nir_lower_ia(struct nir_shader *s, struct agx_ia_key *ia);

void agx_nir_lower_multidraw(struct nir_shader *s, struct agx_ia_key *key);

void agx_nir_lower_gs(struct nir_shader *gs, struct nir_shader *vs,
                      const struct nir_shader *libagx, struct agx_ia_key *ia,
                      bool rasterizer_discard, struct nir_shader **gs_count,
                      struct nir_shader **gs_copy, struct nir_shader **pre_gs,
                      enum mesa_prim *out_mode, unsigned *out_count_words);

struct nir_shader *agx_nir_prefix_sum_gs(const struct nir_shader *libagx,
                                         unsigned words);

struct nir_shader *agx_nir_gs_setup_indirect(const struct nir_shader *libagx,
                                             enum mesa_prim prim,
                                             bool multidraw);

struct nir_shader *agx_nir_unroll_restart(const struct nir_shader *libagx,
                                          enum mesa_prim prim,
                                          unsigned index_size_B);

#endif
