/*
 * Copyright (C) 2023 Amazon.com, Inc. or its affiliates
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
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef __PAN_AFBC_CSO_H__
#define __PAN_AFBC_CSO_H__

#include "util/hash_table.h"

#include "panfrost/util/pan_ir.h"
#include "pan_texture.h"

struct panfrost_context;
struct panfrost_resource;
struct panfrost_screen;

struct pan_afbc_shader_key {
   unsigned bpp;
   unsigned align;
   bool tiled;
};

struct pan_afbc_shader_data {
   struct pan_afbc_shader_key key;
   void *size_cso;
   void *pack_cso;
};

struct pan_afbc_shaders {
   struct hash_table *shaders;
   pthread_mutex_t lock;
};

struct pan_afbc_block_info {
   uint32_t size;
   uint32_t offset;
};

struct panfrost_afbc_size_info {
   mali_ptr src;
   mali_ptr metadata;
} PACKED;

struct panfrost_afbc_pack_info {
   mali_ptr src;
   mali_ptr dst;
   mali_ptr metadata;
   uint32_t header_size;
   uint32_t src_stride;
   uint32_t dst_stride;
   uint32_t padding[3]; // FIXME
} PACKED;

void panfrost_afbc_context_init(struct panfrost_context *ctx);
void panfrost_afbc_context_destroy(struct panfrost_context *ctx);

struct pan_afbc_shader_data *
panfrost_afbc_get_shaders(struct panfrost_context *ctx,
                          struct panfrost_resource *rsrc, unsigned align);

#endif
