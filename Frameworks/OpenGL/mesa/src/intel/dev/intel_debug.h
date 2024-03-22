/*
 * Copyright 2003 VMware, Inc.
 * Copyright © 2007 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE COPYRIGHT OWNER(S) AND/OR ITS SUPPLIERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef INTEL_DEBUG_H
#define INTEL_DEBUG_H

#include <stdint.h>
#include "compiler/shader_enums.h"
#include "util/macros.h"

#ifdef __cplusplus
extern "C" {
#endif
/**
 * \file intel_debug.h
 *
 * Basic INTEL_DEBUG environment variable handling.  This file defines the
 * list of debugging flags, as well as some macros for handling them.
 */

extern uint64_t intel_debug;

/* Returns 0/1, not the matching bit mask. */
#define INTEL_DEBUG(flags)        unlikely(intel_debug & (flags))

#define DEBUG_TEXTURE             (1ull <<  0)
#define DEBUG_BLIT                (1ull <<  1)
#define DEBUG_PERF                (1ull <<  2)
#define DEBUG_PERFMON             (1ull <<  3)
#define DEBUG_BATCH               (1ull <<  4)
#define DEBUG_BUFMGR              (1ull <<  5)
#define DEBUG_GS                  (1ull <<  6)
#define DEBUG_SYNC                (1ull <<  7)
#define DEBUG_SF                  (1ull <<  8)
#define DEBUG_SUBMIT              (1ull <<  9)
#define DEBUG_WM                  (1ull << 10)
#define DEBUG_URB                 (1ull << 11)
#define DEBUG_VS                  (1ull << 12)
#define DEBUG_CLIP                (1ull << 13)
#define DEBUG_STALL               (1ull << 14)
#define DEBUG_BLORP               (1ull << 15)
/* reserved                       (1ull << 16) */
#define DEBUG_NO_DUAL_OBJECT_GS   (1ull << 17)
#define DEBUG_OPTIMIZER           (1ull << 18)
#define DEBUG_ANNOTATION          (1ull << 19)
/* reserved                       (1ull << 20) */
#define DEBUG_NO_OACONFIG         (1ull << 21)
#define DEBUG_SPILL_FS            (1ull << 22)
#define DEBUG_SPILL_VEC4          (1ull << 23)
#define DEBUG_CS                  (1ull << 24)
#define DEBUG_HEX                 (1ull << 25)
#define DEBUG_NO_COMPACTION       (1ull << 26)
#define DEBUG_TCS                 (1ull << 27)
#define DEBUG_TES                 (1ull << 28)
#define DEBUG_L3                  (1ull << 29)
#define DEBUG_DO32                (1ull << 30)
#define DEBUG_NO_CCS              (1ull << 31)
#define DEBUG_NO_HIZ              (1ull << 32)
#define DEBUG_COLOR               (1ull << 33)
#define DEBUG_REEMIT              (1ull << 34)
#define DEBUG_SOFT64              (1ull << 35)
#define DEBUG_BT                  (1ull << 36)
#define DEBUG_PIPE_CONTROL        (1ull << 37)
#define DEBUG_NO_FAST_CLEAR       (1ull << 38)
/* reserved                       (1ull << 39) */
#define DEBUG_RT                  (1ull << 40)
#define DEBUG_TASK                (1ull << 41)
#define DEBUG_MESH                (1ull << 42)
#define DEBUG_CAPTURE_ALL         (1ull << 43)
#define DEBUG_PERF_SYMBOL_NAMES   (1ull << 44)
#define DEBUG_SWSB_STALL          (1ull << 45)
#define DEBUG_HEAPS               (1ull << 46)
#define DEBUG_ISL                 (1ull << 47)
#define DEBUG_SPARSE              (1ull << 48)
#define DEBUG_DRAW_BKP            (1ull << 49)
#define DEBUG_BATCH_STATS         (1ull << 50)

#define DEBUG_ANY                 (~0ull)

/* These flags are not compatible with the disk shader cache */
#define DEBUG_DISK_CACHE_DISABLE_MASK 0

/* These flags may affect program generation */
#define DEBUG_DISK_CACHE_MASK \
   (DEBUG_NO_DUAL_OBJECT_GS | DEBUG_SPILL_FS | \
   DEBUG_SPILL_VEC4 | DEBUG_NO_COMPACTION | DEBUG_DO32 | DEBUG_SOFT64)

extern uint64_t intel_simd;
extern uint32_t intel_debug_bkp_before_draw_count;
extern uint32_t intel_debug_bkp_after_draw_count;

#define INTEL_SIMD(type, size)        (!!(intel_simd & (DEBUG_ ## type ## _SIMD ## size)))

/* VS, TCS, TES and GS stages are dispatched in one size */
#define DEBUG_FS_SIMD8    (1ull << 0)
#define DEBUG_FS_SIMD16   (1ull << 1)
#define DEBUG_FS_SIMD32   (1ull << 2)
#define DEBUG_FS_SIMD2X8  (1ull << 3)
#define DEBUG_FS_SIMD4X8  (1ull << 4)
#define DEBUG_FS_SIMD2X16 (1ull << 5)

#define DEBUG_CS_SIMD8    (1ull << 6)
#define DEBUG_CS_SIMD16   (1ull << 7)
#define DEBUG_CS_SIMD32   (1ull << 8)

#define DEBUG_TS_SIMD8    (1ull << 9)
#define DEBUG_TS_SIMD16   (1ull << 10)
#define DEBUG_TS_SIMD32   (1ull << 11)

#define DEBUG_MS_SIMD8    (1ull << 12)
#define DEBUG_MS_SIMD16   (1ull << 13)
#define DEBUG_MS_SIMD32   (1ull << 14)

#define DEBUG_RT_SIMD8    (1ull << 15)
#define DEBUG_RT_SIMD16   (1ull << 16)
#define DEBUG_RT_SIMD32   (1ull << 17)

#define SIMD_DISK_CACHE_MASK ((1ull << 18) - 1)

#ifdef HAVE_ANDROID_PLATFORM
#define LOG_TAG "INTEL-MESA"
#if ANDROID_API_LEVEL >= 26
#include <log/log.h>
#else
#include <cutils/log.h>
#endif /* use log/log.h start from android 8 major version */
#ifndef ALOGW
#define ALOGW LOGW
#endif
#define dbg_printf(...)	ALOGW(__VA_ARGS__)
#else
#define dbg_printf(...)	fprintf(stderr, __VA_ARGS__)
#endif /* HAVE_ANDROID_PLATFORM */

#define DBG(...) do {                  \
   if (INTEL_DEBUG(FILE_DEBUG_FLAG))   \
      dbg_printf(__VA_ARGS__);         \
} while(0)

extern uint64_t intel_debug_flag_for_shader_stage(gl_shader_stage stage);

extern void brw_process_intel_debug_variable(void);

/* Below is a list of structure located in the identifier buffer. The driver
 * can fill those in for debug purposes.
 */

enum intel_debug_block_type {
   /* End of the debug blocks */
   INTEL_DEBUG_BLOCK_TYPE_END = 1,

   /* Driver identifier (struct intel_debug_block_driver) */
   INTEL_DEBUG_BLOCK_TYPE_DRIVER,

   /* Frame identifier (struct intel_debug_block_frame) */
   INTEL_DEBUG_BLOCK_TYPE_FRAME,

   /* Internal, never to be written out */
   INTEL_DEBUG_BLOCK_TYPE_MAX,
};

struct intel_debug_block_base {
   uint32_t type; /* enum intel_debug_block_type */
   uint32_t length; /* inclusive of this structure size */
};

struct intel_debug_block_driver {
   struct intel_debug_block_base base;
   uint8_t description[];
};

struct intel_debug_block_frame {
   struct intel_debug_block_base base;
   uint64_t frame_id;
};

extern void *intel_debug_identifier(void);
extern uint32_t intel_debug_identifier_size(void);

extern uint32_t intel_debug_write_identifiers(void *output,
                                              uint32_t output_size,
                                              const char *driver_name);

extern void *intel_debug_get_identifier_block(void *buffer,
                                              uint32_t buffer_size,
                                              enum intel_debug_block_type type);

bool intel_debug_batch_in_range(uint64_t frame_id);

#ifdef __cplusplus
}
#endif

#endif /* INTEL_DEBUG_H */
