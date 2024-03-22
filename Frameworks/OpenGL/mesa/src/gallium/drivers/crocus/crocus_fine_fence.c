#include "crocus_context.h"
#include "crocus_fine_fence.h"
#include "util/u_upload_mgr.h"

static void
crocus_fine_fence_reset(struct crocus_batch *batch)
{
   u_upload_alloc(batch->fine_fences.uploader,
                  0, sizeof(uint64_t), sizeof(uint64_t),
                  &batch->fine_fences.ref.offset, &batch->fine_fences.ref.res,
                  (void **)&batch->fine_fences.map);
   WRITE_ONCE(*batch->fine_fences.map, 0);
   batch->fine_fences.next++;
}

void
crocus_fine_fence_init(struct crocus_batch *batch)
{
   batch->fine_fences.ref.res = NULL;
   batch->fine_fences.next = 0;
   if (batch_has_fine_fence(batch))
      crocus_fine_fence_reset(batch);
}

static uint32_t
crocus_fine_fence_next(struct crocus_batch *batch)
{
   if (!batch_has_fine_fence(batch))
      return UINT32_MAX;

   uint32_t seqno = batch->fine_fences.next++;

   if (batch->fine_fences.next == 0)
      crocus_fine_fence_reset(batch);

   return seqno;
}

void
crocus_fine_fence_destroy(struct crocus_screen *screen,
                          struct crocus_fine_fence *fine)
{
   crocus_syncobj_reference(screen, &fine->syncobj, NULL);
   pipe_resource_reference(&fine->ref.res, NULL);
   free(fine);
}

struct crocus_fine_fence *
crocus_fine_fence_new(struct crocus_batch *batch, unsigned flags)
{
   struct crocus_fine_fence *fine = calloc(1, sizeof(*fine));
   if (!fine)
      return NULL;

   pipe_reference_init(&fine->reference, 1);

   fine->seqno = crocus_fine_fence_next(batch);

   crocus_syncobj_reference(batch->screen, &fine->syncobj,
                            crocus_batch_get_signal_syncobj(batch));

   if (!batch_has_fine_fence(batch))
      return fine;
   pipe_resource_reference(&fine->ref.res, batch->fine_fences.ref.res);
   fine->ref.offset = batch->fine_fences.ref.offset;
   fine->map = batch->fine_fences.map;
   fine->flags = flags;

   unsigned pc;
   if (flags & CROCUS_FENCE_TOP_OF_PIPE) {
      pc = PIPE_CONTROL_WRITE_IMMEDIATE | PIPE_CONTROL_CS_STALL;
   } else {
      pc = PIPE_CONTROL_WRITE_IMMEDIATE |
           PIPE_CONTROL_RENDER_TARGET_FLUSH |
           PIPE_CONTROL_TILE_CACHE_FLUSH |
           PIPE_CONTROL_DEPTH_CACHE_FLUSH |
           PIPE_CONTROL_DATA_CACHE_FLUSH;
   }
   crocus_emit_pipe_control_write(batch, "fence: fine", pc,
                                  crocus_resource_bo(fine->ref.res),
                                  fine->ref.offset,
                                  fine->seqno);

   return fine;
}
