#include "iris_context.h"
#include "iris_fine_fence.h"
#include "util/u_upload_mgr.h"

static void
iris_fine_fence_reset(struct iris_batch *batch)
{
   u_upload_alloc(batch->fine_fences.uploader,
		  0, sizeof(uint64_t), sizeof(uint64_t),
                  &batch->fine_fences.ref.offset, &batch->fine_fences.ref.res,
                  (void **)&batch->fine_fences.map);
   WRITE_ONCE(*batch->fine_fences.map, 0);
   batch->fine_fences.next++;
}

void
iris_fine_fence_init(struct iris_batch *batch)
{
   batch->fine_fences.ref.res = NULL;
   batch->fine_fences.next = 0;
   iris_fine_fence_reset(batch);
}

static uint32_t
iris_fine_fence_next(struct iris_batch *batch)
{
   uint32_t seqno = batch->fine_fences.next++;

   if (batch->fine_fences.next == 0)
      iris_fine_fence_reset(batch);

   return seqno;
}

void
iris_fine_fence_destroy(struct iris_screen *screen,
                        struct iris_fine_fence *fine)
{
   iris_syncobj_reference(screen->bufmgr, &fine->syncobj, NULL);
   pipe_resource_reference(&fine->ref.res, NULL);
   free(fine);
}

struct iris_fine_fence *
iris_fine_fence_new(struct iris_batch *batch)
{
   struct iris_fine_fence *fine = calloc(1, sizeof(*fine));
   if (!fine)
      return NULL;

   pipe_reference_init(&fine->reference, 1);

   fine->seqno = iris_fine_fence_next(batch);

   iris_syncobj_reference(batch->screen->bufmgr, &fine->syncobj,
                          iris_batch_get_signal_syncobj(batch));

   pipe_resource_reference(&fine->ref.res, batch->fine_fences.ref.res);
   fine->ref.offset = batch->fine_fences.ref.offset;
   fine->map = batch->fine_fences.map;

   unsigned pc = PIPE_CONTROL_WRITE_IMMEDIATE |
                 PIPE_CONTROL_RENDER_TARGET_FLUSH |
                 PIPE_CONTROL_TILE_CACHE_FLUSH |
                 PIPE_CONTROL_DEPTH_CACHE_FLUSH |
                 PIPE_CONTROL_DATA_CACHE_FLUSH;

   if (batch->name == IRIS_BATCH_COMPUTE)
      pc &= ~PIPE_CONTROL_GRAPHICS_BITS;

   iris_emit_pipe_control_write(batch, "fence: fine", pc,
                                iris_resource_bo(fine->ref.res),
                                fine->ref.offset,
                                fine->seqno);

   return fine;
}
