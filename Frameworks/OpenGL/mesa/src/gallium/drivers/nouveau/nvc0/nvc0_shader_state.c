/*
 * Copyright 2010 Christoph Bumiller
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#include "pipe/p_context.h"
#include "pipe/p_defines.h"
#include "pipe/p_state.h"
#include "util/u_inlines.h"

#include "nvc0/nvc0_context.h"
#include "nvc0/nvc0_query_hw.h"

#include "nvc0/nvc0_compute.xml.h"

static inline void
nvc0_program_update_context_state(struct nvc0_context *nvc0,
                                  struct nvc0_program *prog, int stage)
{
   if (prog && prog->need_tls) {
      const uint32_t flags = NV_VRAM_DOMAIN(&nvc0->screen->base) | NOUVEAU_BO_RDWR;
      if (!nvc0->state.tls_required)
         BCTX_REFN_bo(nvc0->bufctx_3d, 3D_TLS, flags, nvc0->screen->tls);
      nvc0->state.tls_required |= 1 << stage;
   } else {
      if (nvc0->state.tls_required == (1 << stage))
         nouveau_bufctx_reset(nvc0->bufctx_3d, NVC0_BIND_3D_TLS);
      nvc0->state.tls_required &= ~(1 << stage);
   }
}

static inline bool
nvc0_program_validate(struct nvc0_context *nvc0, struct nvc0_program *prog)
{
   if (prog->mem)
      return true;

   if (!prog->translated) {
      prog->translated = nvc0_program_translate(
         prog, nvc0->screen->base.device->chipset,
         nvc0->screen->base.disk_shader_cache, &nvc0->base.debug);
      if (!prog->translated)
         return false;
   }

   if (likely(prog->code_size))
      return nvc0_program_upload(nvc0, prog);
   return true; /* stream output info only */
}

void
nvc0_program_sp_start_id(struct nvc0_context *nvc0, int stage,
                         struct nvc0_program *prog)
{
   struct nouveau_pushbuf *push = nvc0->base.pushbuf;

   simple_mtx_assert_locked(&nvc0->screen->state_lock);
   if (nvc0->screen->eng3d->oclass < GV100_3D_CLASS) {
      BEGIN_NVC0(push, NVC0_3D(SP_START_ID(stage)), 1);
      PUSH_DATA (push, prog->code_base);
   } else {
      BEGIN_NVC0(push, SUBC_3D(GV100_3D_SP_ADDRESS_HIGH(stage)), 2);
      PUSH_DATAh(push, nvc0->screen->text->offset + prog->code_base);
      PUSH_DATA (push, nvc0->screen->text->offset + prog->code_base);
   }
}

void
nvc0_vertprog_validate(struct nvc0_context *nvc0)
{
   struct nouveau_pushbuf *push = nvc0->base.pushbuf;
   struct nvc0_program *vp = nvc0->vertprog;

   if (!nvc0_program_validate(nvc0, vp))
         return;
   nvc0_program_update_context_state(nvc0, vp, 0);

   BEGIN_NVC0(push, NVC0_3D(SP_SELECT(1)), 1);
   PUSH_DATA (push, 0x11);
   nvc0_program_sp_start_id(nvc0, 1, vp);
   BEGIN_NVC0(push, NVC0_3D(SP_GPR_ALLOC(1)), 1);
   PUSH_DATA (push, vp->num_gprs);

   // BEGIN_NVC0(push, NVC0_3D_(0x163c), 1);
   // PUSH_DATA (push, 0);
}

void
nvc0_fragprog_validate(struct nvc0_context *nvc0)
{
   struct nouveau_pushbuf *push = nvc0->base.pushbuf;
   struct nvc0_program *fp = nvc0->fragprog;
   struct pipe_rasterizer_state *rast = &nvc0->rast->pipe;

   if (fp->fp.force_persample_interp != rast->force_persample_interp) {
      /* Force the program to be reuploaded, which will trigger interp fixups
       * to get applied
       */
      if (fp->mem)
         nouveau_heap_free(&fp->mem);

      fp->fp.force_persample_interp = rast->force_persample_interp;
   }

   if (fp->fp.msaa != rast->multisample) {
      /* Force the program to be reuploaded, which will trigger interp fixups
       * to get applied
       */
      if (fp->mem)
         nouveau_heap_free(&fp->mem);

      fp->fp.msaa = rast->multisample;
   }

   /* Shade model works well enough when both colors follow it. However if one
    * (or both) is explicitly set, then we have to go the patching route.
    */
   bool has_explicit_color = fp->fp.colors &&
      (((fp->fp.colors & 1) && !fp->fp.color_interp[0]) ||
       ((fp->fp.colors & 2) && !fp->fp.color_interp[1]));
   bool hwflatshade = false;
   if (has_explicit_color && fp->fp.flatshade != rast->flatshade) {
      /* Force re-upload */
      if (fp->mem)
         nouveau_heap_free(&fp->mem);

      fp->fp.flatshade = rast->flatshade;

      /* Always smooth-shade in this mode, the shader will decide on its own
       * when to flat-shade.
       */
   } else if (!has_explicit_color) {
      hwflatshade = rast->flatshade;

      /* No need to binary-patch the shader each time, make sure that it's set
       * up for the default behaviour.
       */
      fp->fp.flatshade = 0;
   }

   if (hwflatshade != nvc0->state.flatshade) {
      nvc0->state.flatshade = hwflatshade;
      BEGIN_NVC0(push, NVC0_3D(SHADE_MODEL), 1);
      PUSH_DATA (push, hwflatshade ? NVC0_3D_SHADE_MODEL_FLAT :
                                     NVC0_3D_SHADE_MODEL_SMOOTH);
   }

   if (fp->mem && !(nvc0->dirty_3d & NVC0_NEW_3D_FRAGPROG)) {
      return;
   }

   if (!nvc0_program_validate(nvc0, fp))
         return;
   nvc0_program_update_context_state(nvc0, fp, 4);

   if (fp->fp.early_z != nvc0->state.early_z_forced) {
      nvc0->state.early_z_forced = fp->fp.early_z;
      IMMED_NVC0(push, NVC0_3D(FORCE_EARLY_FRAGMENT_TESTS), fp->fp.early_z);
   }
   if (fp->fp.post_depth_coverage != nvc0->state.post_depth_coverage) {
      nvc0->state.post_depth_coverage = fp->fp.post_depth_coverage;
      IMMED_NVC0(push, NVC0_3D(POST_DEPTH_COVERAGE),
                 fp->fp.post_depth_coverage);
   }

   BEGIN_NVC0(push, NVC0_3D(SP_SELECT(5)), 1);
   PUSH_DATA (push, 0x51);
   nvc0_program_sp_start_id(nvc0, 5, fp);
   BEGIN_NVC0(push, NVC0_3D(SP_GPR_ALLOC(5)), 1);
   PUSH_DATA (push, fp->num_gprs);

   BEGIN_NVC0(push, SUBC_3D(0x0360), 2);
   PUSH_DATA (push, 0x20164010);
   PUSH_DATA (push, 0x20);
   BEGIN_NVC0(push, NVC0_3D(ZCULL_TEST_MASK), 1);
   PUSH_DATA (push, fp->flags[0]);
}

void
nvc0_tctlprog_validate(struct nvc0_context *nvc0)
{
   struct nouveau_pushbuf *push = nvc0->base.pushbuf;
   struct nvc0_program *tp = nvc0->tctlprog;

   if (tp && nvc0_program_validate(nvc0, tp)) {
      if (tp->tp.tess_mode != ~0) {
         BEGIN_NVC0(push, NVC0_3D(TESS_MODE), 1);
         PUSH_DATA (push, tp->tp.tess_mode);
      }
      BEGIN_NVC0(push, NVC0_3D(SP_SELECT(2)), 1);
      PUSH_DATA (push, 0x21);
      nvc0_program_sp_start_id(nvc0, 2, tp);
      BEGIN_NVC0(push, NVC0_3D(SP_GPR_ALLOC(2)), 1);
      PUSH_DATA (push, tp->num_gprs);
   } else {
      tp = nvc0->tcp_empty;
      /* not a whole lot we can do to handle this failure */
      if (!nvc0_program_validate(nvc0, tp))
         assert(!"unable to validate empty tcp");
      BEGIN_NVC0(push, NVC0_3D(SP_SELECT(2)), 1);
      PUSH_DATA (push, 0x20);
      nvc0_program_sp_start_id(nvc0, 2, tp);
   }
   nvc0_program_update_context_state(nvc0, tp, 1);
}

void
nvc0_tevlprog_validate(struct nvc0_context *nvc0)
{
   struct nouveau_pushbuf *push = nvc0->base.pushbuf;
   struct nvc0_program *tp = nvc0->tevlprog;

   if (tp && nvc0_program_validate(nvc0, tp)) {
      if (tp->tp.tess_mode != ~0) {
         BEGIN_NVC0(push, NVC0_3D(TESS_MODE), 1);
         PUSH_DATA (push, tp->tp.tess_mode);
      }
      BEGIN_NVC0(push, NVC0_3D(MACRO_TEP_SELECT), 1);
      PUSH_DATA (push, 0x31);
      nvc0_program_sp_start_id(nvc0, 3, tp);
      BEGIN_NVC0(push, NVC0_3D(SP_GPR_ALLOC(3)), 1);
      PUSH_DATA (push, tp->num_gprs);
   } else {
      BEGIN_NVC0(push, NVC0_3D(MACRO_TEP_SELECT), 1);
      PUSH_DATA (push, 0x30);
   }
   nvc0_program_update_context_state(nvc0, tp, 2);
}

void
nvc0_gmtyprog_validate(struct nvc0_context *nvc0)
{
   struct nouveau_pushbuf *push = nvc0->base.pushbuf;
   struct nvc0_program *gp = nvc0->gmtyprog;

   /* we allow GPs with no code for specifying stream output state only */
   if (gp && nvc0_program_validate(nvc0, gp) && gp->code_size) {
      BEGIN_NVC0(push, NVC0_3D(MACRO_GP_SELECT), 1);
      PUSH_DATA (push, 0x41);
      nvc0_program_sp_start_id(nvc0, 4, gp);
      BEGIN_NVC0(push, NVC0_3D(SP_GPR_ALLOC(4)), 1);
      PUSH_DATA (push, gp->num_gprs);
   } else {
      BEGIN_NVC0(push, NVC0_3D(MACRO_GP_SELECT), 1);
      PUSH_DATA (push, 0x40);
   }
   nvc0_program_update_context_state(nvc0, gp, 3);
}

void
nvc0_compprog_validate(struct nvc0_context *nvc0)
{
   struct nouveau_pushbuf *push = nvc0->base.pushbuf;
   struct nvc0_program *cp = nvc0->compprog;

   if (cp && !nvc0_program_validate(nvc0, cp))
      return;

   BEGIN_NVC0(push, NVC0_CP(FLUSH), 1);
   PUSH_DATA (push, NVC0_COMPUTE_FLUSH_CODE);
}

void
nvc0_layer_validate(struct nvc0_context *nvc0)
{
   struct nouveau_pushbuf *push = nvc0->base.pushbuf;
   struct nvc0_program *last;
   bool prog_selects_layer = false;
   bool layer_viewport_relative = false;

   if (nvc0->gmtyprog)
      last = nvc0->gmtyprog;
   else if (nvc0->tevlprog)
      last = nvc0->tevlprog;
   else
      last = nvc0->vertprog;

   if (last) {
      prog_selects_layer = !!(last->hdr[13] & (1 << 9));
      layer_viewport_relative = last->vp.layer_viewport_relative;
   }

   BEGIN_NVC0(push, NVC0_3D(LAYER), 1);
   PUSH_DATA (push, prog_selects_layer ? NVC0_3D_LAYER_USE_GP : 0);
   if (nvc0->screen->eng3d->oclass >= GM200_3D_CLASS) {
      IMMED_NVC0(push, NVC0_3D(LAYER_VIEWPORT_RELATIVE),
                 layer_viewport_relative);
   }
}

void
nvc0_tfb_validate(struct nvc0_context *nvc0)
{
   struct nouveau_pushbuf *push = nvc0->base.pushbuf;
   struct nvc0_transform_feedback_state *tfb;
   unsigned b;

   if (nvc0->gmtyprog) tfb = nvc0->gmtyprog->tfb;
   else
   if (nvc0->tevlprog) tfb = nvc0->tevlprog->tfb;
   else
      tfb = nvc0->vertprog->tfb;

   IMMED_NVC0(push, NVC0_3D(TFB_ENABLE), (tfb && nvc0->num_tfbbufs) ? 1 : 0);

   if (tfb && tfb != nvc0->state.tfb) {
      for (b = 0; b < 4; ++b) {
         if (tfb->varying_count[b]) {
            unsigned n = (tfb->varying_count[b] + 3) / 4;

            BEGIN_NVC0(push, NVC0_3D(TFB_STREAM(b)), 3);
            PUSH_DATA (push, tfb->stream[b]);
            PUSH_DATA (push, tfb->varying_count[b]);
            PUSH_DATA (push, tfb->stride[b]);
            BEGIN_NVC0(push, NVC0_3D(TFB_VARYING_LOCS(b, 0)), n);
            PUSH_DATAp(push, tfb->varying_index[b], n);

            if (nvc0->tfbbuf[b])
               nvc0_so_target(nvc0->tfbbuf[b])->stride = tfb->stride[b];
         } else {
            IMMED_NVC0(push, NVC0_3D(TFB_VARYING_COUNT(b)), 0);
         }
      }
   }

   simple_mtx_assert_locked(&nvc0->screen->state_lock);
   nvc0->state.tfb = tfb;

   if (!(nvc0->dirty_3d & NVC0_NEW_3D_TFB_TARGETS))
      return;

   for (b = 0; b < nvc0->num_tfbbufs; ++b) {
      struct nvc0_so_target *targ = nvc0_so_target(nvc0->tfbbuf[b]);
      struct nv04_resource *buf;

      if (targ && tfb)
         targ->stride = tfb->stride[b];

      if (!targ || !targ->stride) {
         IMMED_NVC0(push, NVC0_3D(TFB_BUFFER_ENABLE(b)), 0);
         continue;
      }

      buf = nv04_resource(targ->pipe.buffer);

      BCTX_REFN(nvc0->bufctx_3d, 3D_TFB, buf, WR);

      if (!(nvc0->tfbbuf_dirty & (1 << b)))
         continue;

      if (!targ->clean)
         nvc0_hw_query_fifo_wait(nvc0, nvc0_query(targ->pq));
      PUSH_SPACE_EX(push, 0, 0, 1);
      BEGIN_NVC0(push, NVC0_3D(TFB_BUFFER_ENABLE(b)), 5);
      PUSH_DATA (push, 1);
      PUSH_DATAh(push, buf->address + targ->pipe.buffer_offset);
      PUSH_DATA (push, buf->address + targ->pipe.buffer_offset);
      PUSH_DATA (push, targ->pipe.buffer_size);
      if (!targ->clean) {
         nvc0_hw_query_pushbuf_submit(push, nvc0_query(targ->pq), 0x4);
      } else {
         PUSH_DATA(push, 0); /* TFB_BUFFER_OFFSET */
         targ->clean = false;
      }
   }
   for (; b < 4; ++b)
      IMMED_NVC0(push, NVC0_3D(TFB_BUFFER_ENABLE(b)), 0);
}
