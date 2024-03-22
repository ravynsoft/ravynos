/**************************************************************************
 *
 * Copyright 2007 VMware, Inc.
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
#ifndef SP_SETUP_H
#define SP_SETUP_H

struct setup_context;
struct softpipe_context;

/**
 * Attribute interpolation mode
 */
enum sp_interp_mode {
   SP_INTERP_POS,       /**< special case for frag position */
   SP_INTERP_CONSTANT,
   SP_INTERP_LINEAR,
   SP_INTERP_PERSPECTIVE
};


struct sp_setup_info {
   unsigned valid;
   struct {
      unsigned interp:8;      /**< SP_INTERP_X */
      int src_index:8;
   } attrib[PIPE_MAX_SHADER_OUTPUTS];
};

void 
sp_setup_tri(struct setup_context *setup,
             const float (*v0)[4],
             const float (*v1)[4],
             const float (*v2)[4]);

void
sp_setup_line(struct setup_context *setup,
           const float (*v0)[4],
           const float (*v1)[4]);

void
sp_setup_point( struct setup_context *setup,
             const float (*v0)[4] );

static inline unsigned
sp_clamp_viewport_idx(int idx)
{
   return (PIPE_MAX_VIEWPORTS > idx && idx >= 0) ? idx : 0;
}

struct setup_context *sp_setup_create_context( struct softpipe_context *softpipe );
void sp_setup_prepare( struct setup_context *setup );
void sp_setup_destroy_context( struct setup_context *setup );

#endif
