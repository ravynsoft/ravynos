/*
 * Copyright © 2018 Intel Corporation
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

#ifndef IRIS_BINDER_DOT_H
#define IRIS_BINDER_DOT_H

#include <stdint.h>
#include <stdbool.h>
#include "compiler/shader_enums.h"

struct iris_bo;
struct iris_batch;
struct iris_bufmgr;
struct iris_compiled_shader;
struct iris_context;

struct iris_binder
{
   struct iris_bo *bo;
   void *map;

   /** Required alignment for each binding table in bytes */
   uint32_t alignment;

   /** Binding table size in bytes */
   uint32_t size;

   /** Insert new entries at this offset (in bytes) */
   uint32_t insert_point;

   /**
    * Last assigned offset for each shader stage's binding table.
    * Zero is considered invalid and means there's no binding table.
    */
   uint32_t bt_offset[MESA_SHADER_STAGES];
};

void iris_init_binder(struct iris_context *ice);
void iris_destroy_binder(struct iris_binder *binder);
uint32_t iris_binder_reserve(struct iris_context *ice, unsigned size);
void iris_binder_reserve_3d(struct iris_context *ice);
void iris_binder_reserve_compute(struct iris_context *ice);

#endif
