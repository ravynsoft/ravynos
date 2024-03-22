/*
 * Copyright 2003 VMware, Inc.
 * Copyright © 2006 Intel Corporation
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

/**
 * \file intel_debug.c
 *
 * Support for the INTEL_DEBUG environment variable, along with other
 * miscellaneous debugging code.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dev/intel_debug.h"
#include "git_sha1.h"
#include "util/macros.h"
#include "util/u_debug.h"
#include "util/u_math.h"
#include "c11/threads.h"

uint64_t intel_debug = 0;

#define DEBUG_NO16                (1ull << 16)
#define DEBUG_NO8                 (1ull << 20)
#define DEBUG_NO32                (1ull << 39)

static const struct debug_control debug_control[] = {
   { "tex",         DEBUG_TEXTURE},
   { "blit",        DEBUG_BLIT},
   { "fall",        DEBUG_PERF},
   { "perf",        DEBUG_PERF},
   { "perfmon",     DEBUG_PERFMON},
   { "bat",         DEBUG_BATCH},
   { "buf",         DEBUG_BUFMGR},
   { "fs",          DEBUG_WM },
   { "gs",          DEBUG_GS},
   { "sync",        DEBUG_SYNC},
   { "sf",          DEBUG_SF },
   { "submit",      DEBUG_SUBMIT },
   { "wm",          DEBUG_WM },
   { "urb",         DEBUG_URB },
   { "vs",          DEBUG_VS },
   { "clip",        DEBUG_CLIP },
   { "no16",        DEBUG_NO16 },
   { "blorp",       DEBUG_BLORP },
   { "nodualobj",   DEBUG_NO_DUAL_OBJECT_GS },
   { "optimizer",   DEBUG_OPTIMIZER },
   { "ann",         DEBUG_ANNOTATION },
   { "no8",         DEBUG_NO8 },
   { "no-oaconfig", DEBUG_NO_OACONFIG },
   { "spill_fs",    DEBUG_SPILL_FS },
   { "spill_vec4",  DEBUG_SPILL_VEC4 },
   { "cs",          DEBUG_CS },
   { "hex",         DEBUG_HEX },
   { "nocompact",   DEBUG_NO_COMPACTION },
   { "hs",          DEBUG_TCS },
   { "tcs",         DEBUG_TCS },
   { "ds",          DEBUG_TES },
   { "tes",         DEBUG_TES },
   { "l3",          DEBUG_L3 },
   { "do32",        DEBUG_DO32 },
   { "norbc",       DEBUG_NO_CCS },
   { "noccs",       DEBUG_NO_CCS },
   { "nohiz",       DEBUG_NO_HIZ },
   { "color",       DEBUG_COLOR },
   { "reemit",      DEBUG_REEMIT },
   { "soft64",      DEBUG_SOFT64 },
   { "bt",          DEBUG_BT },
   { "pc",          DEBUG_PIPE_CONTROL },
   { "nofc",        DEBUG_NO_FAST_CLEAR },
   { "no32",        DEBUG_NO32 },
   { "shaders",     DEBUG_WM | DEBUG_VS | DEBUG_TCS |
                    DEBUG_TES | DEBUG_GS | DEBUG_CS |
                    DEBUG_RT | DEBUG_TASK | DEBUG_MESH },
   { "rt",          DEBUG_RT },
   { "task",        DEBUG_TASK },
   { "mesh",        DEBUG_MESH },
   { "stall",       DEBUG_STALL },
   { "capture-all", DEBUG_CAPTURE_ALL },
   { "perf-symbol-names", DEBUG_PERF_SYMBOL_NAMES },
   { "swsb-stall",  DEBUG_SWSB_STALL },
   { "heaps",       DEBUG_HEAPS },
   { "isl",         DEBUG_ISL },
   { "sparse",      DEBUG_SPARSE },
   { "draw_bkp",    DEBUG_DRAW_BKP },
   { "bat-stats",   DEBUG_BATCH_STATS },
   { NULL,    0 }
};

uint64_t intel_simd = 0;

static const struct debug_control simd_control[] = {
   { "fs8",    DEBUG_FS_SIMD8 },
   { "fs16",   DEBUG_FS_SIMD16 },
   { "fs32",   DEBUG_FS_SIMD32 },
   { "fs2x8",  DEBUG_FS_SIMD2X8 },
   { "fs4x8",  DEBUG_FS_SIMD4X8 },
   { "fs2x16", DEBUG_FS_SIMD2X16 },
   { "cs8",    DEBUG_CS_SIMD8 },
   { "cs16",   DEBUG_CS_SIMD16 },
   { "cs32",   DEBUG_CS_SIMD32 },
   { "ts8",    DEBUG_TS_SIMD8 },
   { "ts16",   DEBUG_TS_SIMD16 },
   { "ts32",   DEBUG_TS_SIMD32 },
   { "ms8",    DEBUG_MS_SIMD8 },
   { "ms16",   DEBUG_MS_SIMD16 },
   { "ms32",   DEBUG_MS_SIMD32 },
   { "rt8",    DEBUG_RT_SIMD8 },
   { "rt16",   DEBUG_RT_SIMD16 },
   { "rt32",   DEBUG_RT_SIMD32 },
   { NULL,     0 }
};

uint64_t
intel_debug_flag_for_shader_stage(gl_shader_stage stage)
{
   uint64_t flags[] = {
      [MESA_SHADER_VERTEX] = DEBUG_VS,
      [MESA_SHADER_TESS_CTRL] = DEBUG_TCS,
      [MESA_SHADER_TESS_EVAL] = DEBUG_TES,
      [MESA_SHADER_GEOMETRY] = DEBUG_GS,
      [MESA_SHADER_FRAGMENT] = DEBUG_WM,
      [MESA_SHADER_COMPUTE] = DEBUG_CS,
      [MESA_SHADER_KERNEL] = DEBUG_CS,

      [MESA_SHADER_TASK]         = DEBUG_TASK,
      [MESA_SHADER_MESH]         = DEBUG_MESH,

      [MESA_SHADER_RAYGEN]       = DEBUG_RT,
      [MESA_SHADER_ANY_HIT]      = DEBUG_RT,
      [MESA_SHADER_CLOSEST_HIT]  = DEBUG_RT,
      [MESA_SHADER_MISS]         = DEBUG_RT,
      [MESA_SHADER_INTERSECTION] = DEBUG_RT,
      [MESA_SHADER_CALLABLE]     = DEBUG_RT,
   };
   return flags[stage];
}

#define DEBUG_FS_SIMD  (DEBUG_FS_SIMD8  | DEBUG_FS_SIMD16  | \
                        DEBUG_FS_SIMD32)
#define DEBUG_CS_SIMD  (DEBUG_CS_SIMD8  | DEBUG_CS_SIMD16  | DEBUG_CS_SIMD32)
#define DEBUG_TS_SIMD  (DEBUG_TS_SIMD8  | DEBUG_TS_SIMD16  | DEBUG_TS_SIMD32)
#define DEBUG_MS_SIMD  (DEBUG_MS_SIMD8  | DEBUG_MS_SIMD16  | DEBUG_MS_SIMD32)
#define DEBUG_RT_SIMD  (DEBUG_RT_SIMD8  | DEBUG_RT_SIMD16  | DEBUG_RT_SIMD32)

#define DEBUG_SIMD8_ALL \
   (DEBUG_FS_SIMD8  | \
    DEBUG_CS_SIMD8  | \
    DEBUG_TS_SIMD8  | \
    DEBUG_MS_SIMD8  | \
    DEBUG_RT_SIMD8)

#define DEBUG_SIMD16_ALL \
   (DEBUG_FS_SIMD16 | \
    DEBUG_CS_SIMD16 | \
    DEBUG_TS_SIMD16 | \
    DEBUG_MS_SIMD16 | \
    DEBUG_RT_SIMD16)

#define DEBUG_SIMD32_ALL \
   (DEBUG_FS_SIMD32 | \
    DEBUG_CS_SIMD32 | \
    DEBUG_TS_SIMD32 | \
    DEBUG_MS_SIMD32 | \
    DEBUG_RT_SIMD32)

static uint64_t intel_debug_batch_frame_start = 0;
static uint64_t intel_debug_batch_frame_stop = -1;

uint32_t intel_debug_bkp_before_draw_count = 0;
uint32_t intel_debug_bkp_after_draw_count = 0;

static void
brw_process_intel_debug_variable_once(void)
{
   intel_debug = parse_debug_string(getenv("INTEL_DEBUG"), debug_control);
   intel_simd = parse_debug_string(getenv("INTEL_SIMD_DEBUG"), simd_control);
   intel_debug_batch_frame_start =
      debug_get_num_option("INTEL_DEBUG_BATCH_FRAME_START", 0);
   intel_debug_batch_frame_stop =
      debug_get_num_option("INTEL_DEBUG_BATCH_FRAME_STOP", -1);

   intel_debug_bkp_before_draw_count =
      debug_get_num_option("INTEL_DEBUG_BKP_BEFORE_DRAW_COUNT", 0);
   intel_debug_bkp_after_draw_count =
      debug_get_num_option("INTEL_DEBUG_BKP_AFTER_DRAW_COUNT", 0);

   if (!(intel_simd & DEBUG_FS_SIMD))
      intel_simd |=   DEBUG_FS_SIMD;
   if (!(intel_simd & DEBUG_CS_SIMD))
      intel_simd |=   DEBUG_CS_SIMD;
   if (!(intel_simd & DEBUG_TS_SIMD))
      intel_simd |=   DEBUG_TS_SIMD;
   if (!(intel_simd & DEBUG_MS_SIMD))
      intel_simd |=   DEBUG_MS_SIMD;
   if (!(intel_simd & DEBUG_RT_SIMD))
      intel_simd |=   DEBUG_RT_SIMD;

   if (intel_debug & DEBUG_NO8)
      intel_simd &= ~DEBUG_SIMD8_ALL;
   if (intel_debug & DEBUG_NO16)
      intel_simd &= ~DEBUG_SIMD16_ALL;
   if (intel_debug & DEBUG_NO32)
      intel_simd &= ~DEBUG_SIMD32_ALL;
   intel_debug &= ~(DEBUG_NO8 | DEBUG_NO16 | DEBUG_NO32);
}

void
brw_process_intel_debug_variable(void)
{
   static once_flag process_intel_debug_variable_flag = ONCE_FLAG_INIT;

   call_once(&process_intel_debug_variable_flag,
             brw_process_intel_debug_variable_once);
}

static uint64_t debug_identifier[4] = {
   0xffeeddccbbaa9988,
   0x7766554433221100,
   0xffeeddccbbaa9988,
   0x7766554433221100,
};

void *
intel_debug_identifier(void)
{
   return debug_identifier;
}

uint32_t
intel_debug_identifier_size(void)
{
   return sizeof(debug_identifier);
}

uint32_t
intel_debug_write_identifiers(void *_output,
                              uint32_t output_size,
                              const char *driver_name)
{
   void *output = _output, *output_end = _output + output_size;

   assert(output_size > intel_debug_identifier_size());

   memcpy(output, intel_debug_identifier(), intel_debug_identifier_size());
   output += intel_debug_identifier_size();

   for (uint32_t id = INTEL_DEBUG_BLOCK_TYPE_DRIVER; id < INTEL_DEBUG_BLOCK_TYPE_MAX; id++) {
      switch (id) {
      case INTEL_DEBUG_BLOCK_TYPE_DRIVER: {
         struct intel_debug_block_driver driver_desc = {
            .base = {
               .type = id,
            },
         };
         int len = snprintf(output + sizeof(driver_desc),
                            output_end - (output + sizeof(driver_desc)),
                            "%s " PACKAGE_VERSION " build " MESA_GIT_SHA1,
                            driver_name);
         driver_desc.base.length = sizeof(driver_desc) + len + 1;
         memcpy(output, &driver_desc, sizeof(driver_desc));
         output += driver_desc.base.length;
         break;
      }

      case INTEL_DEBUG_BLOCK_TYPE_FRAME: {
         struct intel_debug_block_frame frame_desc = {
            .base = {
               .type = INTEL_DEBUG_BLOCK_TYPE_FRAME,
               .length = sizeof(frame_desc),
            },
         };
         memcpy(output, &frame_desc, sizeof(frame_desc));
         output += sizeof(frame_desc);
         break;
      }

      default:
         unreachable("Missing identifier write");
      }

      assert(output < output_end);
   }

   struct intel_debug_block_base end = {
      .type = INTEL_DEBUG_BLOCK_TYPE_END,
      .length = sizeof(end),
   };
   memcpy(output, &end, sizeof(end));
   output += sizeof(end);

   assert(output < output_end);

   /* Add at least a full aligned uint64_t of zero padding at the end
    * to make the identifiers easier to spot.
    */
   const unsigned unpadded_len = output - _output;
   const unsigned padding = ALIGN(unpadded_len + 8, 8) - unpadded_len;
   memset(output, 0, padding);
   output += padding;

   assert(output < output_end);

   /* Return the how many bytes where written, so that the rest of the buffer
    * can be used for other things.
    */
   return output - _output;
}

void *
intel_debug_get_identifier_block(void *_buffer,
                                 uint32_t buffer_size,
                                 enum intel_debug_block_type type)
{
   void *buffer = _buffer + intel_debug_identifier_size(),
      *end_buffer = _buffer + buffer_size;

   while (buffer < end_buffer) {
      struct intel_debug_block_base *item = buffer;

      if (item->type == type)
         return item;
      if (item->type == INTEL_DEBUG_BLOCK_TYPE_END)
         return NULL;

      buffer += item->length;
   }

   return NULL;
}

/**
 * Check if in valid frame range for batch dumping
 */
bool
intel_debug_batch_in_range(uint64_t frame_id)
{
   return frame_id >= intel_debug_batch_frame_start &&
          frame_id < intel_debug_batch_frame_stop;
}
