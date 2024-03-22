/*
 * Copyright 2023 Alyssa Rosenzweig
 * Copyright 2022 Collabora Ltd.
 * SPDX-License-Identifier: MIT
 */

#include "compiler/nir/nir_builder.h"
#include "compiler/nir/nir_xfb_info.h"
#include "pipe/p_defines.h"
#include "util/u_draw.h"
#include "util/u_dump.h"
#include "util/u_inlines.h"
#include "util/u_prim.h"
#include "agx_state.h"

static struct pipe_stream_output_target *
agx_create_stream_output_target(struct pipe_context *pctx,
                                struct pipe_resource *prsc,
                                unsigned buffer_offset, unsigned buffer_size)
{
   struct agx_streamout_target *target =
      rzalloc(pctx, struct agx_streamout_target);

   if (!target)
      return NULL;

   pipe_reference_init(&target->base.reference, 1);
   pipe_resource_reference(&target->base.buffer, prsc);

   target->base.context = pctx;
   target->base.buffer_offset = buffer_offset;
   target->base.buffer_size = buffer_size;

   uint32_t zero = 0;
   target->offset = pipe_buffer_create_with_data(pctx, PIPE_BIND_GLOBAL,
                                                 PIPE_USAGE_DEFAULT, 4, &zero);

   return &target->base;
}

static void
agx_stream_output_target_destroy(struct pipe_context *pctx,
                                 struct pipe_stream_output_target *target)
{
   pipe_resource_reference(&target->buffer, NULL);
   ralloc_free(target);
}

static void
agx_set_stream_output_targets(struct pipe_context *pctx, unsigned num_targets,
                              struct pipe_stream_output_target **targets,
                              const unsigned *offsets)
{
   struct agx_context *ctx = agx_context(pctx);
   struct agx_streamout *so = &ctx->streamout;

   assert(num_targets <= ARRAY_SIZE(so->targets));

   for (unsigned i = 0; i < num_targets; i++) {
      /* From the Gallium documentation:
       *
       *    -1 means the buffer should be appended to, and everything else sets
       *    the internal offset.
       *
       * We append regardless, so just check for != -1. Yes, using a negative
       * sentinel value with an unsigned type is bananas. But it's in the
       * Gallium contract and it will work out fine. Probably should be
       * redefined to be ~0 instead of -1 but it doesn't really matter.
       */
      if (offsets[i] != -1 && targets[i] != NULL) {
         pipe_buffer_write(pctx, agx_so_target(targets[i])->offset, 0, 4,
                           &offsets[i]);
      }

      pipe_so_target_reference(&so->targets[i], targets[i]);
   }

   for (unsigned i = num_targets; i < so->num_targets; i++)
      pipe_so_target_reference(&so->targets[i], NULL);

   so->num_targets = num_targets;
}

static struct pipe_stream_output_target *
get_target(struct agx_context *ctx, unsigned buffer)
{
   if (buffer < ctx->streamout.num_targets)
      return ctx->streamout.targets[buffer];
   else
      return NULL;
}

/*
 * Return the address of the indexed streamout buffer. This will be
 * pushed into the streamout shader.
 */
uint64_t
agx_batch_get_so_address(struct agx_batch *batch, unsigned buffer,
                         uint32_t *size)
{
   struct pipe_stream_output_target *target = get_target(batch->ctx, buffer);

   /* If there's no target, don't write anything */
   if (!target) {
      *size = 0;
      return 0;
   }

   /* Otherwise, write the target */
   struct agx_resource *rsrc = agx_resource(target->buffer);
   agx_batch_writes(batch, rsrc, 0);

   *size = target->buffer_size;
   return rsrc->bo->ptr.gpu + target->buffer_offset;
}

void
agx_draw_vbo_from_xfb(struct pipe_context *pctx,
                      const struct pipe_draw_info *info, unsigned drawid_offset,
                      const struct pipe_draw_indirect_info *indirect)
{
   perf_debug_ctx(agx_context(pctx), "draw auto");

   struct agx_streamout_target *so =
      agx_so_target(indirect->count_from_stream_output);

   unsigned offset_B;
   pipe_buffer_read(pctx, so->offset, 0, 4, &offset_B);

   unsigned count = offset_B / so->stride;

   /* XXX: Probably need to divide here */

   struct pipe_draw_start_count_bias draw = {
      .start = 0,
      .count = count,
   };

   pctx->draw_vbo(pctx, info, drawid_offset, NULL, &draw, 1);
}

static uint32_t
xfb_prims_for_vertices(enum mesa_prim mode, unsigned verts)
{
   uint32_t prims = u_decomposed_prims_for_vertices(mode, verts);

   /* The GL spec isn't super clear about this, but it implies that quads are
    * supposed to be tessellated into primitives and piglit
    * (ext_transform_feedback-tessellation quads) checks this.
    */
   if (u_decomposed_prim(mode) == MESA_PRIM_QUADS)
      prims *= 2;

   return prims;
}

/*
 * Count generated primitives on the CPU for transform feedback. This only works
 * in the absence of indirect draws, geometry shaders, or tessellation.
 */
void
agx_primitives_update_direct(struct agx_context *ctx,
                             const struct pipe_draw_info *info,
                             const struct pipe_draw_start_count_bias *draw)
{
   assert(ctx->active_queries && ctx->prims_generated[0] && "precondition");
   assert(!ctx->stage[PIPE_SHADER_GEOMETRY].shader &&
          "Geometry shaders use their own counting");

   ctx->prims_generated[0]->value +=
      xfb_prims_for_vertices(info->mode, draw->count);
}

void
agx_init_streamout_functions(struct pipe_context *ctx)
{
   ctx->create_stream_output_target = agx_create_stream_output_target;
   ctx->stream_output_target_destroy = agx_stream_output_target_destroy;
   ctx->set_stream_output_targets = agx_set_stream_output_targets;
}
