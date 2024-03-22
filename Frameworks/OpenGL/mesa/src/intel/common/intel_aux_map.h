/*
 * Copyright (c) 2018 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#ifndef INTEL_AUX_MAP_H
#define INTEL_AUX_MAP_H

#include "intel_buffer_alloc.h"

#include "isl/isl.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Auxiliary surface mapping implementation
 *
 * These functions are implemented in common code shared by drivers.
 */

struct intel_aux_map_context;
struct intel_device_info;

#define INTEL_AUX_MAP_ENTRY_VALID_BIT    0x1ull

struct intel_aux_map_context *
intel_aux_map_init(void *driver_ctx,
                   struct intel_mapped_pinned_buffer_alloc *buffer_alloc,
                   const struct intel_device_info *devinfo);

uint32_t
intel_aux_map_get_alignment(struct intel_aux_map_context *ctx);

void
intel_aux_map_finish(struct intel_aux_map_context *ctx);

uint32_t
intel_aux_map_get_state_num(struct intel_aux_map_context *ctx);

/**
 * Returns the current number of buffers used by the aux-map tables
 *
 * When preparing to execute a new batch, use this function to determine how
 * many buffers will be required. More buffers may be added by concurrent
 * accesses of the aux-map functions, but they won't be required for since
 * they involve surfaces not used by this batch.
 */
uint32_t
intel_aux_map_get_num_buffers(struct intel_aux_map_context *ctx);

/**
 * Returns the mask of meta data address in L1 entry
 *
 * The mask value is effected by page size of meta data specific to a platform.
 */
uint64_t
intel_aux_get_meta_address_mask(struct intel_aux_map_context *ctx);

/**
 * Returns the ratio between the granularity of main surface and AUX data
 */
uint64_t
intel_aux_get_main_to_aux_ratio(struct intel_aux_map_context *ctx);

/**
 * Fill an array of exec_object2 with aux-map buffer handles
 *
 * The intel_aux_map_get_num_buffers call should be made, then the driver can
 * make sure the `obj` array is large enough before calling this function.
 */
void
intel_aux_map_fill_bos(struct intel_aux_map_context *ctx, void **driver_bos,
                       uint32_t max_bos);

uint64_t
intel_aux_map_get_base(struct intel_aux_map_context *ctx);

uint64_t
intel_aux_map_format_bits(enum isl_tiling tiling, enum isl_format format,
                          uint8_t plane);

uint64_t
intel_aux_map_format_bits_for_isl_surf(const struct isl_surf *isl_surf);

uint64_t *
intel_aux_map_get_entry(struct intel_aux_map_context *ctx,
                        uint64_t main_address,
                        uint64_t *aux_entry_address);

/* Fails if a mapping is attempted that would conflict with an existing one.
 * This increase the refcount of the mapped region if already mapped, sets it
 * to 1 otherwise.
 */
bool
intel_aux_map_add_mapping(struct intel_aux_map_context *ctx, uint64_t main_address,
                          uint64_t aux_address, uint64_t main_size_B,
                          uint64_t format_bits);

/* Decrease the refcount of a mapped region. When the refcount reaches 0, the
 * region is unmapped.
 */
void
intel_aux_map_del_mapping(struct intel_aux_map_context *ctx, uint64_t main_address,
                          uint64_t size);

/* Unmaps a region, refcount is reset to 0.
 */
void
intel_aux_map_unmap_range(struct intel_aux_map_context *ctx, uint64_t main_address,
                          uint64_t size);

#ifdef __cplusplus
}
#endif

#endif /* INTEL_AUX_MAP_H */
