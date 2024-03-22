/*
 * Copyright 2023 Alyssa Rosenzweig
 * SPDX-License-Identifier: MIT
 */

#ifndef __AGX_HELPERS_H
#define __AGX_HELPERS_H

#include <stdbool.h>
#include "asahi/compiler/agx_compile.h"
#include "asahi/layout/layout.h"
#include "pipe/p_defines.h"
#include "agx_pack.h"
#include "agx_ppp.h"

#define AGX_MAX_VIEWPORTS (16)

#define agx_push(ptr, T, cfg)                                                  \
   for (unsigned _loop = 0; _loop < 1; ++_loop, ptr += AGX_##T##_LENGTH)       \
      agx_pack(ptr, T, cfg)

static inline enum agx_sampler_states
agx_translate_sampler_state_count(unsigned count, bool extended)
{
   assert(count <= 17 && "max 17 sampler state registers supported");

   if (count == 0) {
      return AGX_SAMPLER_STATES_0;
   } else if (extended) {
      if (count <= 8)
         return AGX_SAMPLER_STATES_8_EXTENDED;
      else
         return AGX_SAMPLER_STATES_16_EXTENDED;
   } else {
      if (count <= 4)
         return AGX_SAMPLER_STATES_4_COMPACT;
      else if (count <= 8)
         return AGX_SAMPLER_STATES_8_COMPACT;
      else if (count <= 12)
         return AGX_SAMPLER_STATES_12_COMPACT;
      else
         return AGX_SAMPLER_STATES_16_COMPACT;
   }
}

/* Channels agree for RGBA but are weird for force 0/1 */

static inline enum agx_channel
agx_channel_from_pipe(enum pipe_swizzle in)
{
   STATIC_ASSERT((enum agx_channel)PIPE_SWIZZLE_X == AGX_CHANNEL_R);
   STATIC_ASSERT((enum agx_channel)PIPE_SWIZZLE_Y == AGX_CHANNEL_G);
   STATIC_ASSERT((enum agx_channel)PIPE_SWIZZLE_Z == AGX_CHANNEL_B);
   STATIC_ASSERT((enum agx_channel)PIPE_SWIZZLE_W == AGX_CHANNEL_A);
   STATIC_ASSERT((enum agx_channel)PIPE_SWIZZLE_0 & 0x4);
   STATIC_ASSERT((enum agx_channel)PIPE_SWIZZLE_1 & 0x4);
   STATIC_ASSERT((enum agx_channel)PIPE_SWIZZLE_NONE & 0x4);

   if ((in & 0x4) == 0)
      return (enum agx_channel)in;
   else if (in == PIPE_SWIZZLE_1)
      return AGX_CHANNEL_1;
   else
      return AGX_CHANNEL_0;
}

static inline enum agx_layout
agx_translate_layout(enum ail_tiling tiling)
{
   switch (tiling) {
   case AIL_TILING_TWIDDLED:
   case AIL_TILING_TWIDDLED_COMPRESSED:
      return AGX_LAYOUT_TWIDDLED;
   case AIL_TILING_LINEAR:
      return AGX_LAYOUT_LINEAR;
   }

   unreachable("Invalid tiling");
}

static inline enum agx_index_size
agx_translate_index_size(uint8_t size_B)
{
   /* Index sizes are encoded logarithmically */
   STATIC_ASSERT(__builtin_ctz(1) == AGX_INDEX_SIZE_U8);
   STATIC_ASSERT(__builtin_ctz(2) == AGX_INDEX_SIZE_U16);
   STATIC_ASSERT(__builtin_ctz(4) == AGX_INDEX_SIZE_U32);

   assert((size_B == 1) || (size_B == 2) || (size_B == 4));
   return __builtin_ctz(size_B);
}

static enum agx_pass_type
agx_pass_type_for_shader(struct agx_shader_info *info)
{
   if (info->reads_tib && info->writes_sample_mask)
      return AGX_PASS_TYPE_TRANSLUCENT_PUNCH_THROUGH;
   else if (info->reads_tib)
      return AGX_PASS_TYPE_TRANSLUCENT;
   else if (info->writes_sample_mask)
      return AGX_PASS_TYPE_PUNCH_THROUGH;
   else
      return AGX_PASS_TYPE_OPAQUE;
}

static enum agx_conservative_depth
agx_translate_depth_layout(enum gl_frag_depth_layout layout)
{
   switch (layout) {
   case FRAG_DEPTH_LAYOUT_ANY:
      return AGX_CONSERVATIVE_DEPTH_ANY;
   case FRAG_DEPTH_LAYOUT_LESS:
      return AGX_CONSERVATIVE_DEPTH_LESS;
   case FRAG_DEPTH_LAYOUT_GREATER:
      return AGX_CONSERVATIVE_DEPTH_GREATER;
   case FRAG_DEPTH_LAYOUT_UNCHANGED:
      return AGX_CONSERVATIVE_DEPTH_UNCHANGED;
   default:
      unreachable("depth layout should have been canonicalized");
   }
}

static void
agx_ppp_fragment_face_2(struct agx_ppp_update *ppp,
                        enum agx_object_type object_type,
                        struct agx_shader_info *info)
{
   agx_ppp_push(ppp, FRAGMENT_FACE_2, cfg) {
      cfg.object_type = object_type;
      cfg.conservative_depth = agx_translate_depth_layout(info->depth_layout);
   }
}

static inline uint32_t
agx_pack_line_width(float line_width)
{
   /* Line width is packed in a 4:4 fixed point format */
   unsigned line_width_fixed = ((unsigned)(line_width * 16.0f)) - 1;

   /* Clamp to maximum line width */
   return MIN2(line_width_fixed, 0xFF);
}

static enum agx_shade_model
agx_translate_shade_model(struct agx_varyings_fs *fs, unsigned binding,
                          bool first_provoking_vertex)
{
   if (fs->bindings[binding].smooth) {
      if (fs->bindings[binding].perspective)
         return AGX_SHADE_MODEL_PERSPECTIVE;
      else
         return AGX_SHADE_MODEL_LINEAR;
   } else {
      if (!first_provoking_vertex)
         return AGX_SHADE_MODEL_FLAT_VERTEX_2;
      else
         return AGX_SHADE_MODEL_FLAT_VERTEX_0;
   }
}

#endif
