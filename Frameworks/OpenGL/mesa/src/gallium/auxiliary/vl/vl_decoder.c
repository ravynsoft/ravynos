/**************************************************************************
 *
 * Copyright 2009 Younes Manton.
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
 * IN NO EVENT SHALL VMWARE AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 **************************************************************************/

#include "pipe/p_video_codec.h"

#include "util/u_video.h"

#include "vl_decoder.h"
#include "vl_mpeg12_decoder.h"

bool
vl_profile_supported(struct pipe_screen *screen, enum pipe_video_profile profile,
                     enum pipe_video_entrypoint entrypoint)
{
   assert(screen);
   switch (u_reduce_video_profile(profile)) {
      case PIPE_VIDEO_FORMAT_MPEG12:
         return entrypoint != PIPE_VIDEO_ENTRYPOINT_ENCODE;
      default:
         return false;
   }
}

int
vl_level_supported(struct pipe_screen *screen, enum pipe_video_profile profile)
{
   assert(screen);
   switch (profile) {
      case PIPE_VIDEO_PROFILE_MPEG1:
         return 0;
      case PIPE_VIDEO_PROFILE_MPEG2_SIMPLE:
      case PIPE_VIDEO_PROFILE_MPEG2_MAIN:
         return 3;
      default:
         return 0;
   }
}

struct pipe_video_codec *
vl_create_decoder(struct pipe_context *pipe,
                  const struct pipe_video_codec *templat)
{
   unsigned width = templat->width, height = templat->height;
   struct pipe_video_codec temp;
   bool pot_buffers;

   assert(pipe);
   assert(width > 0 && height > 0);
   
   pot_buffers = !pipe->screen->get_video_param
   (
      pipe->screen,
      templat->profile,
      templat->entrypoint,
      PIPE_VIDEO_CAP_NPOT_TEXTURES
   );

   temp = *templat;
   temp.width = pot_buffers ? util_next_power_of_two(width) : align(width, VL_MACROBLOCK_WIDTH);
   temp.height = pot_buffers ? util_next_power_of_two(height) : align(height, VL_MACROBLOCK_HEIGHT);

   switch (u_reduce_video_profile(temp.profile)) {
      case PIPE_VIDEO_FORMAT_MPEG12:
         return vl_create_mpeg12_decoder(pipe, &temp);

      default:
         return NULL;
   }
   return NULL;
}
