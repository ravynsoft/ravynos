#include "pipe/p_context.h"
#include "util/u_surface.h"
#include "util/u_inlines.h"
#include "util/u_transfer.h"
#include "util/u_memory.h"

void u_default_buffer_subdata(struct pipe_context *pipe,
                              struct pipe_resource *resource,
                              unsigned usage, unsigned offset,
                              unsigned size, const void *data)
{
   struct pipe_transfer *transfer = NULL;
   struct pipe_box box;
   uint8_t *map = NULL;

   assert(!(usage & PIPE_MAP_READ));

   /* the write flag is implicit by the nature of buffer_subdata */
   usage |= PIPE_MAP_WRITE;

   /* buffer_subdata implicitly discards the rewritten buffer range.
    * PIPE_MAP_DIRECTLY supresses that.
    */
   if (!(usage & PIPE_MAP_DIRECTLY)) {
      if (offset == 0 && size == resource->width0) {
         usage |= PIPE_MAP_DISCARD_WHOLE_RESOURCE;
      } else {
         usage |= PIPE_MAP_DISCARD_RANGE;
      }
   }

   u_box_1d(offset, size, &box);

   map = pipe->buffer_map(pipe, resource, 0, usage, &box, &transfer);
   if (!map)
      return;

   memcpy(map, data, size);
   pipe_buffer_unmap(pipe, transfer);
}

void u_default_clear_buffer(struct pipe_context *pipe,
                            struct pipe_resource *resource,
                            unsigned offset, unsigned size,
                            const void *clear_value,
                            int clear_value_size)
{
   struct pipe_transfer *transfer = NULL;
   struct pipe_box box;
   uint8_t *map = NULL;

   /* the write flag is implicit by the nature of buffer_subdata */
   unsigned usage = PIPE_MAP_WRITE;

   /* clear_buffer implicitly discards the rewritten buffer range. */
   if (offset == 0 && size == resource->width0) {
      usage |= PIPE_MAP_DISCARD_WHOLE_RESOURCE;
   } else {
      usage |= PIPE_MAP_DISCARD_RANGE;
   }

   u_box_1d(offset, size, &box);

   map = pipe->buffer_map(pipe, resource, 0, usage, &box, &transfer);
   if (!map)
      return;

   assert(clear_value_size > 0);
   for (unsigned off = 0; off < size; off += clear_value_size)
      memcpy(map + off, clear_value, MIN2(clear_value_size, size - off));
   pipe_buffer_unmap(pipe, transfer);
}

void u_default_texture_subdata(struct pipe_context *pipe,
                               struct pipe_resource *resource,
                               unsigned level,
                               unsigned usage,
                               const struct pipe_box *box,
                               const void *data,
                               unsigned stride,
                               uintptr_t layer_stride)
{
   struct pipe_transfer *transfer = NULL;
   const uint8_t *src_data = data;
   uint8_t *map = NULL;

   assert(!(usage & PIPE_MAP_READ));

   /* the write flag is implicit by the nature of texture_subdata */
   usage |= PIPE_MAP_WRITE;

   /* texture_subdata implicitly discards the rewritten buffer range */
   usage |= PIPE_MAP_DISCARD_RANGE;

   map = pipe->texture_map(pipe,
                            resource,
                            level,
                            usage,
                            box, &transfer);
   if (!map)
      return;

   util_copy_box(map,
                 resource->format,
                 transfer->stride, /* bytes */
                 transfer->layer_stride, /* bytes */
                 0, 0, 0,
                 box->width,
                 box->height,
                 box->depth,
                 src_data,
                 stride,       /* bytes */
                 layer_stride, /* bytes */
                 0, 0, 0);

   pipe_texture_unmap(pipe, transfer);
}

void u_default_transfer_flush_region(UNUSED struct pipe_context *pipe,
                                     UNUSED struct pipe_transfer *transfer,
                                     UNUSED const struct pipe_box *box)
{
   /* This is a no-op implementation, nothing to do.
    */
}
