/*
 * Copyright © 2022 Intel Corporation
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
#ifndef INTEL_HANG_DUMP_H
#define INTEL_HANG_DUMP_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/**
 * This files contains a format description for data saved in a hang dump.
 * This allows us to replay the dump later on simulation.
 */

/* TODO: Consider compression? ZSTD_error_dstSize_tooSmall */

#define INTEL_HANG_DUMP_VERSION (1)
#define INTEL_HANG_DUMP_MAGIC   (0x4245012345676463)

enum intel_hang_dump_block_type {
   INTEL_HANG_DUMP_BLOCK_TYPE_HEADER    = 1,
   INTEL_HANG_DUMP_BLOCK_TYPE_BO        = 2,
   INTEL_HANG_DUMP_BLOCK_TYPE_MAP       = 3,
   INTEL_HANG_DUMP_BLOCK_TYPE_EXEC      = 4,
   INTEL_HANG_DUMP_BLOCK_TYPE_HW_IMAGE  = 5,
};

struct intel_hang_dump_block_base {
   uint32_t type; /* enum intel_hang_dump_block_type */

   uint32_t pad;
};

struct intel_hang_dump_block_header {
   struct intel_hang_dump_block_base base;

   uint64_t magic;

   uint32_t version;
   uint32_t pad;
};

struct intel_hang_dump_block_bo {
   struct intel_hang_dump_block_base base;

   /* Helpful */
   char name[64];

   /* PPGTT location */
   uint64_t offset;

   /* Buffer size */
   uint64_t size;

   /* Data follows */
};

struct intel_hang_dump_block_map {
   struct intel_hang_dump_block_base base;

   /* Helpful */
   char name[64];

   /* PPGTT location */
   uint64_t offset;

   /* Buffer size */
   uint64_t size;
};

struct intel_hang_dump_block_exec {
   struct intel_hang_dump_block_base base;

   /* PPGTT location */
   uint64_t offset;
};

struct intel_hang_dump_block_hw_image {
   struct intel_hang_dump_block_base base;

   /* Buffer size */
   uint64_t size;

   /* Data follows */
};

union intel_hang_dump_block_all {
   struct intel_hang_dump_block_base     base;
   struct intel_hang_dump_block_header   header;
   struct intel_hang_dump_block_bo       bo;
   struct intel_hang_dump_block_map      map;
   struct intel_hang_dump_block_exec     exec;
   struct intel_hang_dump_block_hw_image hw_img;
};

#ifdef __cplusplus
}
#endif

#endif /* INTEL_HANG_DUMP_H */
