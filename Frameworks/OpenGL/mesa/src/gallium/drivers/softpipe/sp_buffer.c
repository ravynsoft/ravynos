/*
 * Copyright 2016 Red Hat.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHOR(S) AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "sp_context.h"
#include "sp_buffer.h"
#include "sp_texture.h"

#include "util/format/u_format.h"

static void *
sp_tgsi_ssbo_lookup(const struct tgsi_buffer *buffer,
                    uint32_t unit,
                    uint32_t *size)
{
   struct sp_tgsi_buffer *sp_buf = (struct sp_tgsi_buffer *)buffer;

   *size = 0;
   if (unit >= PIPE_MAX_SHADER_BUFFERS)
      return NULL;

   struct pipe_shader_buffer *bview = &sp_buf->sp_bview[unit];
   /* Sanity check the view size is within our buffer. */
   if (!bview->buffer ||
       bview->buffer_offset > bview->buffer->width0 ||
       bview->buffer_size > bview->buffer->width0 - bview->buffer_offset) {
      return NULL;
   }

   struct softpipe_resource *spr = softpipe_resource(bview->buffer);
   *size = bview->buffer_size;
   return (char *)spr->data + bview->buffer_offset;
}

struct sp_tgsi_buffer *
sp_create_tgsi_buffer(void)
{
   struct sp_tgsi_buffer *buf = CALLOC_STRUCT(sp_tgsi_buffer);
   if (!buf)
      return NULL;

   buf->base.lookup = sp_tgsi_ssbo_lookup;
   return buf;
};
