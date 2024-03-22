/*
 * Copyright (c) 2015 Intel Corporation
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

#ifndef INTEL_L3_CONFIG_H
#define INTEL_L3_CONFIG_H

#include <stdio.h>

#include "dev/intel_device_info.h"

/**
 * Chunk of L3 cache reserved for some specific purpose.
 */
enum intel_l3_partition {
   /** Shared local memory. */
   INTEL_L3P_SLM = 0,
   /** Unified return buffer. */
   INTEL_L3P_URB,
   /** Union of DC and RO. */
   INTEL_L3P_ALL,
   /** Data cluster RW partition. */
   INTEL_L3P_DC,
   /** Union of IS, C and T. */
   INTEL_L3P_RO,
   /** Instruction and state cache. */
   INTEL_L3P_IS,
   /** Constant cache. */
   INTEL_L3P_C,
   /** Texture cache. */
   INTEL_L3P_T,
   /** Unified tile cache. */
   INTEL_L3P_TC,
   /** Number of supported L3 partitions. */
   INTEL_NUM_L3P
};

/**
 * L3 configuration represented as the number of ways allocated for each
 * partition.  \sa get_l3_way_size().
 */
struct intel_l3_config {
   unsigned n[INTEL_NUM_L3P];
};

/**
 * L3 configuration represented as a vector of weights giving the desired
 * relative size of each partition.  The scale is arbitrary, only the ratios
 * between weights will have an influence on the selection of the closest L3
 * configuration.
 */
struct intel_l3_weights {
   float w[INTEL_NUM_L3P];
};

float intel_diff_l3_weights(struct intel_l3_weights w0, struct intel_l3_weights w1);

struct intel_l3_weights
intel_get_default_l3_weights(const struct intel_device_info *devinfo,
                             bool needs_dc, bool needs_slm);

struct intel_l3_weights
intel_get_l3_config_weights(const struct intel_l3_config *cfg);

const struct intel_l3_config *
intel_get_default_l3_config(const struct intel_device_info *devinfo);

const struct intel_l3_config *
intel_get_l3_config(const struct intel_device_info *devinfo,
                    struct intel_l3_weights w0);

unsigned
intel_get_l3_config_urb_size(const struct intel_device_info *devinfo,
                             const struct intel_l3_config *cfg);

unsigned
intel_get_l3_partition_size(const struct intel_device_info *devinfo,
                            const struct intel_l3_config *cfg,
                            enum intel_l3_partition i);

void intel_dump_l3_config(const struct intel_l3_config *cfg, FILE *fp);

enum intel_urb_deref_block_size {
   INTEL_URB_DEREF_BLOCK_SIZE_32         = 0,
   INTEL_URB_DEREF_BLOCK_SIZE_PER_POLY   = 1,
   INTEL_URB_DEREF_BLOCK_SIZE_8          = 2,
   INTEL_URB_DEREF_BLOCK_SIZE_MESH       = 3,
};

void intel_get_urb_config(const struct intel_device_info *devinfo,
                          const struct intel_l3_config *l3_cfg,
                          bool tess_present, bool gs_present,
                          const unsigned entry_size[4],
                          unsigned entries[4], unsigned start[4],
                          enum intel_urb_deref_block_size *deref_block_size,
                          bool *constrained);

struct intel_mesh_urb_allocation {
   unsigned task_entries;
   unsigned task_entry_size_64b;
   unsigned task_starting_address_8kb;

   unsigned mesh_entries;
   unsigned mesh_entry_size_64b;
   unsigned mesh_starting_address_8kb;

   enum intel_urb_deref_block_size deref_block_size;
};

struct intel_mesh_urb_allocation
intel_get_mesh_urb_config(const struct intel_device_info *devinfo,
                          const struct intel_l3_config *l3_cfg,
                          unsigned tue_size_dw, unsigned mue_size_dw);

#endif /* INTEL_L3_CONFIG_H */
