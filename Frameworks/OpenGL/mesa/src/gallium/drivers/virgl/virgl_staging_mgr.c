/*
 * Copyright 2009 VMware, Inc.
 * Copyright 2019 Collabora Ltd.
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

#include "pipe/p_defines.h"
#include "util/u_inlines.h"
#include "pipe/p_context.h"
#include "util/u_memory.h"
#include "util/u_math.h"

#include "virgl_staging_mgr.h"
#include "virgl_resource.h"

static bool
virgl_staging_alloc_buffer(struct virgl_staging_mgr *staging, unsigned min_size)
{
   struct virgl_winsys *vws = staging->vws;
   unsigned size;

   /* Release the old buffer, if present:
    */
   vws->resource_reference(vws, &staging->hw_res, NULL);

   /* Allocate a new one:
    */
   size = align(MAX2(staging->default_size, min_size), 4096);

   staging->hw_res = vws->resource_create(vws,
                                          PIPE_BUFFER,
                                          NULL,
                                          PIPE_FORMAT_R8_UNORM,
                                          VIRGL_BIND_STAGING,
                                          size,  /* width */
                                          1,     /* height */
                                          1,     /* depth */
                                          1,     /* array_size */
                                          0,     /* last_level */
                                          0,     /* nr_samples */
                                          0,     /* flags */
                                          size); /* size */
   if (staging->hw_res == NULL)
      return false;

   staging->map = vws->resource_map(vws, staging->hw_res);
   if (staging->map == NULL) {
      vws->resource_reference(vws, &staging->hw_res, NULL);
      return false;
   }

   staging->offset = 0;
   staging->size = size;

   return true;
}

void
virgl_staging_init(struct virgl_staging_mgr *staging, struct pipe_context *pipe,
                   unsigned default_size)
{
   memset(staging, 0, sizeof(*staging));

   staging->vws = virgl_screen(pipe->screen)->vws;
   staging->default_size = default_size;
}

void
virgl_staging_destroy(struct virgl_staging_mgr *staging)
{
   struct virgl_winsys *vws = staging->vws;
   vws->resource_reference(vws, &staging->hw_res, NULL);
}

bool
virgl_staging_alloc(struct virgl_staging_mgr *staging,
                    unsigned size,
                    unsigned alignment,
                    unsigned *out_offset,
                    struct virgl_hw_res **outbuf,
                    void **ptr)
{
   struct virgl_winsys *vws = staging->vws;
   unsigned offset = align(staging->offset, alignment);

   assert(out_offset);
   assert(outbuf);
   assert(ptr);
   assert(size);

   /* Make sure we have enough space in the staging buffer
    * for the sub-allocation.
    */
   if (offset + size > staging->size) {
      if (unlikely(!virgl_staging_alloc_buffer(staging, size))) {
         *out_offset = ~0;
         vws->resource_reference(vws, outbuf, NULL);
         *ptr = NULL;
         return false;
      }

      offset = 0;
   }

   assert(staging->size);
   assert(staging->hw_res);
   assert(staging->map);
   assert(offset < staging->size);
   assert(offset + size <= staging->size);

   /* Emit the return values: */
   *ptr = staging->map + offset;
   vws->resource_reference(vws, outbuf, staging->hw_res);
   *out_offset = offset;

   staging->offset = offset + size;

   return true;
}
