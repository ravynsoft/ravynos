/*
 * Copyright 2012 Nouveau Project
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
 *
 * Authors: Christoph Bumiller
 */

#include "nvc0/nvc0_context.h"
#include "nvc0/nve4_compute.h"

#include "nv50_ir_driver.h"

#include "drf.h"
#include "qmd.h"
#include "cla0c0qmd.h"
#include "clc0c0qmd.h"
#include "clc3c0qmd.h"

#define NVA0C0_QMDV00_06_VAL_SET(p,a...) NVVAL_MW_SET((p), NVA0C0, QMDV00_06, ##a)
#define NVA0C0_QMDV00_06_DEF_SET(p,a...) NVDEF_MW_SET((p), NVA0C0, QMDV00_06, ##a)
#define NVC0C0_QMDV02_01_VAL_SET(p,a...) NVVAL_MW_SET((p), NVC0C0, QMDV02_01, ##a)
#define NVC0C0_QMDV02_01_DEF_SET(p,a...) NVDEF_MW_SET((p), NVC0C0, QMDV02_01, ##a)
#define NVC3C0_QMDV02_02_VAL_SET(p,a...) NVVAL_MW_SET((p), NVC3C0, QMDV02_02, ##a)
#define NVC3C0_QMDV02_02_DEF_SET(p,a...) NVDEF_MW_SET((p), NVC3C0, QMDV02_02, ##a)

int
nve4_screen_compute_setup(struct nvc0_screen *screen,
                          struct nouveau_pushbuf *push)
{
   int i;
   uint32_t obj_class = screen->compute->oclass;
   uint64_t address;

   BEGIN_NVC0(push, SUBC_CP(NV01_SUBCHAN_OBJECT), 1);
   PUSH_DATA (push, screen->compute->oclass);

   BEGIN_NVC0(push, NVE4_CP(TEMP_ADDRESS_HIGH), 2);
   PUSH_DATAh(push, screen->tls->offset);
   PUSH_DATA (push, screen->tls->offset);
   /* No idea why there are 2. Divide size by 2 to be safe.
    * Actually this might be per-MP TEMP size and looks like I'm only using
    * 2 MPs instead of all 8.
    */
   BEGIN_NVC0(push, NVE4_CP(MP_TEMP_SIZE_HIGH(0)), 3);
   PUSH_DATAh(push, screen->tls->size / screen->mp_count);
   PUSH_DATA (push, (screen->tls->size / screen->mp_count) & ~0x7fff);
   PUSH_DATA (push, 0xff);
   if (obj_class < GV100_COMPUTE_CLASS) {
      BEGIN_NVC0(push, NVE4_CP(MP_TEMP_SIZE_HIGH(1)), 3);
      PUSH_DATAh(push, screen->tls->size / screen->mp_count);
      PUSH_DATA (push, (screen->tls->size / screen->mp_count) & ~0x7fff);
      PUSH_DATA (push, 0xff);
   }

   /* Unified address space ? Who needs that ? Certainly not OpenCL.
    *
    * FATAL: Buffers with addresses inside [0x1000000, 0x3000000] will NOT be
    *  accessible. We cannot prevent that at the moment, so expect failure.
    */
   if (obj_class < GV100_COMPUTE_CLASS) {
      BEGIN_NVC0(push, NVE4_CP(LOCAL_BASE), 1);
      PUSH_DATA (push, 0xff << 24);
      BEGIN_NVC0(push, NVE4_CP(SHARED_BASE), 1);
      PUSH_DATA (push, 0xfe << 24);

      BEGIN_NVC0(push, NVE4_CP(CODE_ADDRESS_HIGH), 2);
      PUSH_DATAh(push, screen->text->offset);
      PUSH_DATA (push, screen->text->offset);
   } else {
      BEGIN_NVC0(push, SUBC_CP(0x2a0), 2);
      PUSH_DATAh(push, 0xfeULL << 24);
      PUSH_DATA (push, 0xfeULL << 24);
      BEGIN_NVC0(push, SUBC_CP(0x7b0), 2);
      PUSH_DATAh(push, 0xffULL << 24);
      PUSH_DATA (push, 0xffULL << 24);
   }

   BEGIN_NVC0(push, SUBC_CP(0x0310), 1);
   PUSH_DATA (push, (obj_class >= NVF0_COMPUTE_CLASS) ? 0x400 : 0x300);

   /* NOTE: these do not affect the state used by the 3D object */
   BEGIN_NVC0(push, NVE4_CP(TIC_ADDRESS_HIGH), 3);
   PUSH_DATAh(push, screen->txc->offset);
   PUSH_DATA (push, screen->txc->offset);
   PUSH_DATA (push, NVC0_TIC_MAX_ENTRIES - 1);
   BEGIN_NVC0(push, NVE4_CP(TSC_ADDRESS_HIGH), 3);
   PUSH_DATAh(push, screen->txc->offset + 65536);
   PUSH_DATA (push, screen->txc->offset + 65536);
   PUSH_DATA (push, NVC0_TSC_MAX_ENTRIES - 1);

   if (obj_class >= NVF0_COMPUTE_CLASS) {
      /* The blob calls GK110_COMPUTE.FIRMWARE[0x6], along with the args (0x1)
       * passed with GK110_COMPUTE.GRAPH.SCRATCH[0x2]. This is currently
       * disabled because our firmware doesn't support these commands and the
       * GPU hangs if they are used. */
      BEGIN_NIC0(push, SUBC_CP(0x0248), 64);
      for (i = 63; i >= 0; i--)
         PUSH_DATA(push, 0x38000 | i);
      IMMED_NVC0(push, SUBC_CP(NV50_GRAPH_SERIALIZE), 0);
   }

   BEGIN_NVC0(push, NVE4_CP(TEX_CB_INDEX), 1);
   PUSH_DATA (push, 7); /* does not interfere with 3D */

   /* Disabling this UNK command avoid a read fault when using texelFetch()
    * from a compute shader for weird reasons.
   if (obj_class == NVF0_COMPUTE_CLASS)
      IMMED_NVC0(push, SUBC_CP(0x02c4), 1);
   */

   address = screen->uniform_bo->offset + NVC0_CB_AUX_INFO(5);

   /* MS sample coordinate offsets: these do not work with _ALT modes ! */
   BEGIN_NVC0(push, NVE4_CP(UPLOAD_DST_ADDRESS_HIGH), 2);
   PUSH_DATAh(push, address + NVC0_CB_AUX_MS_INFO);
   PUSH_DATA (push, address + NVC0_CB_AUX_MS_INFO);
   BEGIN_NVC0(push, NVE4_CP(UPLOAD_LINE_LENGTH_IN), 2);
   PUSH_DATA (push, 64);
   PUSH_DATA (push, 1);
   BEGIN_1IC0(push, NVE4_CP(UPLOAD_EXEC), 17);
   PUSH_DATA (push, NVE4_COMPUTE_UPLOAD_EXEC_LINEAR | (0x20 << 1));
   PUSH_DATA (push, 0); /* 0 */
   PUSH_DATA (push, 0);
   PUSH_DATA (push, 1); /* 1 */
   PUSH_DATA (push, 0);
   PUSH_DATA (push, 0); /* 2 */
   PUSH_DATA (push, 1);
   PUSH_DATA (push, 1); /* 3 */
   PUSH_DATA (push, 1);
   PUSH_DATA (push, 2); /* 4 */
   PUSH_DATA (push, 0);
   PUSH_DATA (push, 3); /* 5 */
   PUSH_DATA (push, 0);
   PUSH_DATA (push, 2); /* 6 */
   PUSH_DATA (push, 1);
   PUSH_DATA (push, 3); /* 7 */
   PUSH_DATA (push, 1);

#ifdef NOUVEAU_NVE4_MP_TRAP_HANDLER
   BEGIN_NVC0(push, NVE4_CP(UPLOAD_DST_ADDRESS_HIGH), 2);
   PUSH_DATAh(push, screen->parm->offset + NVE4_CP_INPUT_TRAP_INFO_PTR);
   PUSH_DATA (push, screen->parm->offset + NVE4_CP_INPUT_TRAP_INFO_PTR);
   BEGIN_NVC0(push, NVE4_CP(UPLOAD_LINE_LENGTH_IN), 2);
   PUSH_DATA (push, 28);
   PUSH_DATA (push, 1);
   BEGIN_1IC0(push, NVE4_CP(UPLOAD_EXEC), 8);
   PUSH_DATA (push, 1);
   PUSH_DATA (push, screen->parm->offset + NVE4_CP_PARAM_TRAP_INFO);
   PUSH_DATAh(push, screen->parm->offset + NVE4_CP_PARAM_TRAP_INFO);
   PUSH_DATA (push, screen->tls->offset);
   PUSH_DATAh(push, screen->tls->offset);
   PUSH_DATA (push, screen->tls->size / 2); /* MP TEMP block size */
   PUSH_DATA (push, screen->tls->size / 2 / 64); /* warp TEMP block size */
   PUSH_DATA (push, 0); /* warp cfstack size */
#endif

   BEGIN_NVC0(push, NVE4_CP(FLUSH), 1);
   PUSH_DATA (push, NVE4_COMPUTE_FLUSH_CB);

   return 0;
}

static void
gm107_compute_validate_surfaces(struct nvc0_context *nvc0,
                                struct pipe_image_view *view, int slot)
{
   struct nv04_resource *res = nv04_resource(view->resource);
   struct nouveau_pushbuf *push = nvc0->base.pushbuf;
   struct nvc0_screen *screen = nvc0->screen;
   struct nouveau_bo *txc = nvc0->screen->txc;
   struct nv50_tic_entry *tic;
   uint64_t address;
   const int s = 5;

   tic = nv50_tic_entry(nvc0->images_tic[s][slot]);

   res = nv04_resource(tic->pipe.texture);
   nvc0_update_tic(nvc0, tic, res);

   if (tic->id < 0) {
      tic->id = nvc0_screen_tic_alloc(nvc0->screen, tic);

      /* upload the texture view */
      PUSH_SPACE(push, 16);
      BEGIN_NVC0(push, NVE4_CP(UPLOAD_DST_ADDRESS_HIGH), 2);
      PUSH_DATAh(push, txc->offset + (tic->id * 32));
      PUSH_DATA (push, txc->offset + (tic->id * 32));
      BEGIN_NVC0(push, NVE4_CP(UPLOAD_LINE_LENGTH_IN), 2);
      PUSH_DATA (push, 32);
      PUSH_DATA (push, 1);
      BEGIN_1IC0(push, NVE4_CP(UPLOAD_EXEC), 9);
      PUSH_DATA (push, NVE4_COMPUTE_UPLOAD_EXEC_LINEAR | (0x20 << 1));
      PUSH_DATAp(push, &tic->tic[0], 8);

      BEGIN_NIC0(push, NVE4_CP(TIC_FLUSH), 1);
      PUSH_DATA (push, (tic->id << 4) | 1);
   } else
   if (res->status & NOUVEAU_BUFFER_STATUS_GPU_WRITING) {
      BEGIN_NIC0(push, NVE4_CP(TEX_CACHE_CTL), 1);
      PUSH_DATA (push, (tic->id << 4) | 1);
   }
   nvc0->screen->tic.lock[tic->id / 32] |= 1 << (tic->id % 32);

   res->status &= ~NOUVEAU_BUFFER_STATUS_GPU_WRITING;
   res->status |=  NOUVEAU_BUFFER_STATUS_GPU_READING;

   BCTX_REFN(nvc0->bufctx_cp, CP_SUF, res, RD);

   address = screen->uniform_bo->offset + NVC0_CB_AUX_INFO(s);

   /* upload the texture handle */
   BEGIN_NVC0(push, NVE4_CP(UPLOAD_DST_ADDRESS_HIGH), 2);
   PUSH_DATAh(push, address + NVC0_CB_AUX_TEX_INFO(slot + 32));
   PUSH_DATA (push, address + NVC0_CB_AUX_TEX_INFO(slot + 32));
   BEGIN_NVC0(push, NVE4_CP(UPLOAD_LINE_LENGTH_IN), 2);
   PUSH_DATA (push, 4);
   PUSH_DATA (push, 0x1);
   BEGIN_1IC0(push, NVE4_CP(UPLOAD_EXEC), 2);
   PUSH_DATA (push, NVE4_COMPUTE_UPLOAD_EXEC_LINEAR | (0x20 << 1));
   PUSH_DATA (push, tic->id);

   BEGIN_NVC0(push, NVE4_CP(FLUSH), 1);
   PUSH_DATA (push, NVE4_COMPUTE_FLUSH_CB);
}

static void
nve4_compute_validate_surfaces(struct nvc0_context *nvc0)
{
   struct nouveau_pushbuf *push = nvc0->base.pushbuf;
   uint64_t address;
   const int s = 5;
   int i, j;

   if (!nvc0->images_dirty[s])
      return;

   address = nvc0->screen->uniform_bo->offset + NVC0_CB_AUX_INFO(s);

   for (i = 0; i < NVC0_MAX_IMAGES; ++i) {
      struct pipe_image_view *view = &nvc0->images[s][i];

      BEGIN_NVC0(push, NVE4_CP(UPLOAD_DST_ADDRESS_HIGH), 2);
      PUSH_DATAh(push, address + NVC0_CB_AUX_SU_INFO(i));
      PUSH_DATA (push, address + NVC0_CB_AUX_SU_INFO(i));
      BEGIN_NVC0(push, NVE4_CP(UPLOAD_LINE_LENGTH_IN), 2);
      PUSH_DATA (push, 16 * 4);
      PUSH_DATA (push, 0x1);
      BEGIN_1IC0(push, NVE4_CP(UPLOAD_EXEC), 1 + 16);
      PUSH_DATA (push, NVE4_COMPUTE_UPLOAD_EXEC_LINEAR | (0x20 << 1));

      if (view->resource) {
         struct nv04_resource *res = nv04_resource(view->resource);

         if (res->base.target == PIPE_BUFFER) {
            if (view->access & PIPE_IMAGE_ACCESS_WRITE)
               nvc0_mark_image_range_valid(view);
         }

         nve4_set_surface_info(push, view, nvc0);
         BCTX_REFN(nvc0->bufctx_cp, CP_SUF, res, RDWR);

         if (nvc0->screen->base.class_3d >= GM107_3D_CLASS)
            gm107_compute_validate_surfaces(nvc0, view, i);
      } else {
         for (j = 0; j < 16; j++)
            PUSH_DATA(push, 0);
      }
   }
}

/* Thankfully, textures with samplers follow the normal rules. */
static void
nve4_compute_validate_samplers(struct nvc0_context *nvc0)
{
   bool need_flush = nve4_validate_tsc(nvc0, 5);
   if (need_flush) {
      BEGIN_NVC0(nvc0->base.pushbuf, NVE4_CP(TSC_FLUSH), 1);
      PUSH_DATA (nvc0->base.pushbuf, 0);
   }

   /* Invalidate all 3D samplers because they are aliased. */
   for (int s = 0; s < 5; s++)
      nvc0->samplers_dirty[s] = ~0;
   nvc0->dirty_3d |= NVC0_NEW_3D_SAMPLERS;
}

/* (Code duplicated at bottom for various non-convincing reasons.
 *  E.g. we might want to use the COMPUTE subchannel to upload TIC/TSC
 *  entries to avoid a subchannel switch.
 *  Same for texture cache flushes.
 *  Also, the bufctx differs, and more IFs in the 3D version looks ugly.)
 */
static void nve4_compute_validate_textures(struct nvc0_context *);

static void
nve4_compute_set_tex_handles(struct nvc0_context *nvc0)
{
   struct nouveau_pushbuf *push = nvc0->base.pushbuf;
   struct nvc0_screen *screen = nvc0->screen;
   uint64_t address;
   const unsigned s = nvc0_shader_stage(PIPE_SHADER_COMPUTE);
   unsigned i, n;
   uint32_t dirty = nvc0->textures_dirty[s] | nvc0->samplers_dirty[s];

   if (!dirty)
      return;
   i = ffs(dirty) - 1;
   n = util_logbase2(dirty) + 1 - i;
   assert(n);

   address = screen->uniform_bo->offset + NVC0_CB_AUX_INFO(s);

   BEGIN_NVC0(push, NVE4_CP(UPLOAD_DST_ADDRESS_HIGH), 2);
   PUSH_DATAh(push, address + NVC0_CB_AUX_TEX_INFO(i));
   PUSH_DATA (push, address + NVC0_CB_AUX_TEX_INFO(i));
   BEGIN_NVC0(push, NVE4_CP(UPLOAD_LINE_LENGTH_IN), 2);
   PUSH_DATA (push, n * 4);
   PUSH_DATA (push, 0x1);
   BEGIN_1IC0(push, NVE4_CP(UPLOAD_EXEC), 1 + n);
   PUSH_DATA (push, NVE4_COMPUTE_UPLOAD_EXEC_LINEAR | (0x20 << 1));
   PUSH_DATAp(push, &nvc0->tex_handles[s][i], n);

   BEGIN_NVC0(push, NVE4_CP(FLUSH), 1);
   PUSH_DATA (push, NVE4_COMPUTE_FLUSH_CB);

   nvc0->textures_dirty[s] = 0;
   nvc0->samplers_dirty[s] = 0;
}

static void
nve4_compute_validate_constbufs(struct nvc0_context *nvc0)
{
   struct nouveau_pushbuf *push = nvc0->base.pushbuf;
   const int s = 5;

   while (nvc0->constbuf_dirty[s]) {
      int i = ffs(nvc0->constbuf_dirty[s]) - 1;
      nvc0->constbuf_dirty[s] &= ~(1 << i);

      if (nvc0->constbuf[s][i].user) {
         struct nouveau_bo *bo = nvc0->screen->uniform_bo;
         const unsigned base = NVC0_CB_USR_INFO(s);
         const unsigned size = nvc0->constbuf[s][0].size;
         assert(i == 0); /* we really only want OpenGL uniforms here */
         assert(nvc0->constbuf[s][0].u.data);

         BEGIN_NVC0(push, NVE4_CP(UPLOAD_DST_ADDRESS_HIGH), 2);
         PUSH_DATAh(push, bo->offset + base);
         PUSH_DATA (push, bo->offset + base);
         BEGIN_NVC0(push, NVE4_CP(UPLOAD_LINE_LENGTH_IN), 2);
         PUSH_DATA (push, size);
         PUSH_DATA (push, 0x1);
         BEGIN_1IC0(push, NVE4_CP(UPLOAD_EXEC), 1 + (size / 4));
         PUSH_DATA (push, NVE4_COMPUTE_UPLOAD_EXEC_LINEAR | (0x20 << 1));
         PUSH_DATAp(push, nvc0->constbuf[s][0].u.data, size / 4);
      }
      else {
         struct nv04_resource *res =
            nv04_resource(nvc0->constbuf[s][i].u.buf);
         if (res) {
            uint64_t address
               = nvc0->screen->uniform_bo->offset + NVC0_CB_AUX_INFO(s);

            /* constbufs above 0 will are fetched via ubo info in the shader */
            if (i > 0) {
               BEGIN_NVC0(push, NVE4_CP(UPLOAD_DST_ADDRESS_HIGH), 2);
               PUSH_DATAh(push, address + NVC0_CB_AUX_UBO_INFO(i - 1));
               PUSH_DATA (push, address + NVC0_CB_AUX_UBO_INFO(i - 1));
               BEGIN_NVC0(push, NVE4_CP(UPLOAD_LINE_LENGTH_IN), 2);
               PUSH_DATA (push, 4 * 4);
               PUSH_DATA (push, 0x1);
               BEGIN_1IC0(push, NVE4_CP(UPLOAD_EXEC), 1 + 4);
               PUSH_DATA (push, NVE4_COMPUTE_UPLOAD_EXEC_LINEAR | (0x20 << 1));

               PUSH_DATA (push, res->address + nvc0->constbuf[s][i].offset);
               PUSH_DATAh(push, res->address + nvc0->constbuf[s][i].offset);
               PUSH_DATA (push, nvc0->constbuf[s][i].size);
               PUSH_DATA (push, 0);
            }

            BCTX_REFN(nvc0->bufctx_cp, CP_CB(i), res, RD);
            res->cb_bindings[s] |= 1 << i;
         }
      }
   }

   BEGIN_NVC0(push, NVE4_CP(FLUSH), 1);
   PUSH_DATA (push, NVE4_COMPUTE_FLUSH_CB);
}

static void
nve4_compute_validate_buffers(struct nvc0_context *nvc0)
{
   struct nouveau_pushbuf *push = nvc0->base.pushbuf;
   uint64_t address;
   const int s = 5;
   int i;

   address = nvc0->screen->uniform_bo->offset + NVC0_CB_AUX_INFO(s);

   BEGIN_NVC0(push, NVE4_CP(UPLOAD_DST_ADDRESS_HIGH), 2);
   PUSH_DATAh(push, address + NVC0_CB_AUX_BUF_INFO(0));
   PUSH_DATA (push, address + NVC0_CB_AUX_BUF_INFO(0));
   BEGIN_NVC0(push, NVE4_CP(UPLOAD_LINE_LENGTH_IN), 2);
   PUSH_DATA (push, 4 * NVC0_MAX_BUFFERS * 4);
   PUSH_DATA (push, 0x1);
   BEGIN_1IC0(push, NVE4_CP(UPLOAD_EXEC), 1 + 4 * NVC0_MAX_BUFFERS);
   PUSH_DATA (push, NVE4_COMPUTE_UPLOAD_EXEC_LINEAR | (0x20 << 1));

   for (i = 0; i < NVC0_MAX_BUFFERS; i++) {
      if (nvc0->buffers[s][i].buffer) {
         struct nv04_resource *res =
            nv04_resource(nvc0->buffers[s][i].buffer);
         PUSH_DATA (push, res->address + nvc0->buffers[s][i].buffer_offset);
         PUSH_DATAh(push, res->address + nvc0->buffers[s][i].buffer_offset);
         PUSH_DATA (push, nvc0->buffers[s][i].buffer_size);
         PUSH_DATA (push, 0);
         BCTX_REFN(nvc0->bufctx_cp, CP_BUF, res, RDWR);
         util_range_add(&res->base, &res->valid_buffer_range,
                        nvc0->buffers[s][i].buffer_offset,
                        nvc0->buffers[s][i].buffer_offset +
                        nvc0->buffers[s][i].buffer_size);
      } else {
         PUSH_DATA (push, 0);
         PUSH_DATA (push, 0);
         PUSH_DATA (push, 0);
         PUSH_DATA (push, 0);
      }
   }
}

static struct nvc0_state_validate
validate_list_cp[] = {
   { nvc0_compprog_validate,              NVC0_NEW_CP_PROGRAM     },
   { nve4_compute_validate_textures,      NVC0_NEW_CP_TEXTURES    },
   { nve4_compute_validate_samplers,      NVC0_NEW_CP_SAMPLERS    },
   { nve4_compute_set_tex_handles,        NVC0_NEW_CP_TEXTURES |
                                          NVC0_NEW_CP_SAMPLERS    },
   { nve4_compute_validate_surfaces,      NVC0_NEW_CP_SURFACES    },
   { nvc0_compute_validate_globals,       NVC0_NEW_CP_GLOBALS     },
   { nve4_compute_validate_buffers,       NVC0_NEW_CP_BUFFERS     },
   { nve4_compute_validate_constbufs,     NVC0_NEW_CP_CONSTBUF    },
};

static bool
nve4_state_validate_cp(struct nvc0_context *nvc0, uint32_t mask)
{
   bool ret;

   ret = nvc0_state_validate(nvc0, mask, validate_list_cp,
                             ARRAY_SIZE(validate_list_cp), &nvc0->dirty_cp,
                             nvc0->bufctx_cp);

   if (unlikely(nvc0->state.flushed))
      nvc0_bufctx_fence(nvc0, nvc0->bufctx_cp, true);
   return ret;
}

static void
nve4_compute_upload_input(struct nvc0_context *nvc0,
                          const struct pipe_grid_info *info)
{
   struct nvc0_screen *screen = nvc0->screen;
   struct nouveau_pushbuf *push = nvc0->base.pushbuf;
   struct nvc0_program *cp = nvc0->compprog;
   uint64_t address;

   address = screen->uniform_bo->offset + NVC0_CB_AUX_INFO(5);

   if (cp->parm_size) {
      BEGIN_NVC0(push, NVE4_CP(UPLOAD_DST_ADDRESS_HIGH), 2);
      PUSH_DATAh(push, screen->uniform_bo->offset + NVC0_CB_USR_INFO(5));
      PUSH_DATA (push, screen->uniform_bo->offset + NVC0_CB_USR_INFO(5));
      BEGIN_NVC0(push, NVE4_CP(UPLOAD_LINE_LENGTH_IN), 2);
      PUSH_DATA (push, cp->parm_size);
      PUSH_DATA (push, 0x1);
      BEGIN_1IC0(push, NVE4_CP(UPLOAD_EXEC), 1 + DIV_ROUND_UP(cp->parm_size, 4));
      PUSH_DATA (push, NVE4_COMPUTE_UPLOAD_EXEC_LINEAR | (0x20 << 1));
      PUSH_DATAb(push, info->input, cp->parm_size);
   }
   BEGIN_NVC0(push, NVE4_CP(UPLOAD_DST_ADDRESS_HIGH), 2);
   PUSH_DATAh(push, address + NVC0_CB_AUX_GRID_INFO(0));
   PUSH_DATA (push, address + NVC0_CB_AUX_GRID_INFO(0));
   BEGIN_NVC0(push, NVE4_CP(UPLOAD_LINE_LENGTH_IN), 2);
   PUSH_DATA (push, 8 * 4);
   PUSH_DATA (push, 0x1);

   if (unlikely(info->indirect)) {
      struct nv04_resource *res = nv04_resource(info->indirect);
      uint32_t offset = res->offset + info->indirect_offset;

      PUSH_SPACE_EX(push, 32, 0, 1);
      PUSH_REF1(push, res->bo, NOUVEAU_BO_RD | res->domain);

      BEGIN_1IC0(push, NVE4_CP(UPLOAD_EXEC), 1 + 8);
      PUSH_DATA (push, NVE4_COMPUTE_UPLOAD_EXEC_LINEAR | (0x20 << 1));
      PUSH_DATAp(push, info->block, 3);
      nouveau_pushbuf_data(push, res->bo, offset,
                           NVC0_IB_ENTRY_1_NO_PREFETCH | 3 * 4);
   } else {
      BEGIN_1IC0(push, NVE4_CP(UPLOAD_EXEC), 1 + 8);
      PUSH_DATA (push, NVE4_COMPUTE_UPLOAD_EXEC_LINEAR | (0x20 << 1));
      PUSH_DATAp(push, info->block, 3);
      PUSH_DATAp(push, info->grid, 3);
   }
   PUSH_DATA (push, 0);
   PUSH_DATA (push, info->work_dim);

   BEGIN_NVC0(push, NVE4_CP(FLUSH), 1);
   PUSH_DATA (push, NVE4_COMPUTE_FLUSH_CB);
}

static inline void
gp100_cp_launch_desc_set_cb(uint32_t *qmd, unsigned index,
                            struct nouveau_bo *bo, uint32_t base, uint32_t size)
{
   uint64_t address = bo->offset + base;

   assert(index < 8);
   assert(!(base & 0xff));

   NVC0C0_QMDV02_01_VAL_SET(qmd, CONSTANT_BUFFER_ADDR_LOWER, index, address);
   NVC0C0_QMDV02_01_VAL_SET(qmd, CONSTANT_BUFFER_ADDR_UPPER, index, address >> 32);
   NVC0C0_QMDV02_01_VAL_SET(qmd, CONSTANT_BUFFER_SIZE_SHIFTED4, index,
                                 DIV_ROUND_UP(size, 16));
   NVC0C0_QMDV02_01_DEF_SET(qmd, CONSTANT_BUFFER_VALID, index, TRUE);
}

static inline void
nve4_cp_launch_desc_set_cb(uint32_t *qmd, unsigned index, struct nouveau_bo *bo,
                           uint32_t base, uint32_t size)
{
   uint64_t address = bo->offset + base;

   assert(index < 8);
   assert(!(base & 0xff));

   NVA0C0_QMDV00_06_VAL_SET(qmd, CONSTANT_BUFFER_ADDR_LOWER, index, address);
   NVA0C0_QMDV00_06_VAL_SET(qmd, CONSTANT_BUFFER_ADDR_UPPER, index, address >> 32);
   NVA0C0_QMDV00_06_VAL_SET(qmd, CONSTANT_BUFFER_SIZE, index, size);
   NVA0C0_QMDV00_06_DEF_SET(qmd, CONSTANT_BUFFER_VALID, index, TRUE);
}

static void
nve4_compute_setup_buf_cb(struct nvc0_context *nvc0, bool gp100, void *desc)
{
   // only user constant buffers 0-6 can be put in the descriptor, the rest are
   // loaded through global memory
   for (int i = 0; i <= 6; i++) {
      if (nvc0->constbuf[5][i].user || !nvc0->constbuf[5][i].u.buf)
         continue;

      struct nv04_resource *res =
         nv04_resource(nvc0->constbuf[5][i].u.buf);

      uint32_t base = res->offset + nvc0->constbuf[5][i].offset;
      uint32_t size = nvc0->constbuf[5][i].size;
      if (gp100)
         gp100_cp_launch_desc_set_cb(desc, i, res->bo, base, size);
      else
         nve4_cp_launch_desc_set_cb(desc, i, res->bo, base, size);
   }

   // there is no need to do FLUSH(NVE4_COMPUTE_FLUSH_CB) because
   // nve4_compute_upload_input() does it later
}

static void
nve4_compute_setup_launch_desc(struct nvc0_context *nvc0, uint32_t *qmd,
                               const struct pipe_grid_info *info)
{
   const struct nvc0_screen *screen = nvc0->screen;
   const struct nvc0_program *cp = nvc0->compprog;
   uint32_t shared_size = cp->cp.smem_size + info->variable_shared_mem;

   NVA0C0_QMDV00_06_DEF_SET(qmd, INVALIDATE_TEXTURE_HEADER_CACHE, TRUE);
   NVA0C0_QMDV00_06_DEF_SET(qmd, INVALIDATE_TEXTURE_SAMPLER_CACHE, TRUE);
   NVA0C0_QMDV00_06_DEF_SET(qmd, INVALIDATE_TEXTURE_DATA_CACHE, TRUE);
   NVA0C0_QMDV00_06_DEF_SET(qmd, INVALIDATE_SHADER_DATA_CACHE, TRUE);
   NVA0C0_QMDV00_06_DEF_SET(qmd, INVALIDATE_SHADER_CONSTANT_CACHE, TRUE);
   NVA0C0_QMDV00_06_DEF_SET(qmd, RELEASE_MEMBAR_TYPE, FE_SYSMEMBAR);
   NVA0C0_QMDV00_06_DEF_SET(qmd, CWD_MEMBAR_TYPE, L1_SYSMEMBAR);
   NVA0C0_QMDV00_06_DEF_SET(qmd, API_VISIBLE_CALL_LIMIT, NO_CHECK);
   NVA0C0_QMDV00_06_VAL_SET(qmd, SASS_VERSION, 0x30);

   NVA0C0_QMDV00_06_VAL_SET(qmd, PROGRAM_OFFSET, cp->code_base);

   NVA0C0_QMDV00_06_VAL_SET(qmd, CTA_RASTER_WIDTH, info->grid[0]);
   NVA0C0_QMDV00_06_VAL_SET(qmd, CTA_RASTER_HEIGHT, info->grid[1]);
   NVA0C0_QMDV00_06_VAL_SET(qmd, CTA_RASTER_DEPTH, info->grid[2]);
   NVA0C0_QMDV00_06_VAL_SET(qmd, CTA_THREAD_DIMENSION0, info->block[0]);
   NVA0C0_QMDV00_06_VAL_SET(qmd, CTA_THREAD_DIMENSION1, info->block[1]);
   NVA0C0_QMDV00_06_VAL_SET(qmd, CTA_THREAD_DIMENSION2, info->block[2]);

   NVA0C0_QMDV00_06_VAL_SET(qmd, SHARED_MEMORY_SIZE, align(shared_size, 0x100));
   NVA0C0_QMDV00_06_VAL_SET(qmd, SHADER_LOCAL_MEMORY_LOW_SIZE, cp->hdr[1] & 0xfffff0);
   NVA0C0_QMDV00_06_VAL_SET(qmd, SHADER_LOCAL_MEMORY_HIGH_SIZE, 0);
   NVA0C0_QMDV00_06_VAL_SET(qmd, SHADER_LOCAL_MEMORY_CRS_SIZE, 0x800);

   if (shared_size > (32 << 10))
      NVA0C0_QMDV00_06_DEF_SET(qmd, L1_CONFIGURATION,
                                    DIRECTLY_ADDRESSABLE_MEMORY_SIZE_48KB);
   else
   if (shared_size > (16 << 10))
      NVA0C0_QMDV00_06_DEF_SET(qmd, L1_CONFIGURATION,
                                    DIRECTLY_ADDRESSABLE_MEMORY_SIZE_32KB);
   else
      NVA0C0_QMDV00_06_DEF_SET(qmd, L1_CONFIGURATION,
                                    DIRECTLY_ADDRESSABLE_MEMORY_SIZE_16KB);

   NVA0C0_QMDV00_06_VAL_SET(qmd, REGISTER_COUNT, cp->num_gprs);
   NVA0C0_QMDV00_06_VAL_SET(qmd, BARRIER_COUNT, cp->num_barriers);

   // Only bind user uniforms and the driver constant buffer through the
   // launch descriptor because UBOs are sticked to the driver cb to avoid the
   // limitation of 8 CBs.
   if (nvc0->constbuf[5][0].user || cp->parm_size) {
      nve4_cp_launch_desc_set_cb(qmd, 0, screen->uniform_bo,
                                 NVC0_CB_USR_INFO(5), 1 << 16);

      // Later logic will attempt to bind a real buffer at position 0. That
      // should not happen if we've bound a user buffer.
      assert(nvc0->constbuf[5][0].user || !nvc0->constbuf[5][0].u.buf);
   }
   nve4_cp_launch_desc_set_cb(qmd, 7, screen->uniform_bo,
                              NVC0_CB_AUX_INFO(5), 1 << 11);

   nve4_compute_setup_buf_cb(nvc0, false, qmd);
}

static void
gp100_compute_setup_launch_desc(struct nvc0_context *nvc0, uint32_t *qmd,
                                const struct pipe_grid_info *info)
{
   const struct nvc0_screen *screen = nvc0->screen;
   const struct nvc0_program *cp = nvc0->compprog;
   uint32_t shared_size = cp->cp.smem_size + info->variable_shared_mem;

   NVC0C0_QMDV02_01_VAL_SET(qmd, SM_GLOBAL_CACHING_ENABLE, 1);
   NVC0C0_QMDV02_01_DEF_SET(qmd, RELEASE_MEMBAR_TYPE, FE_SYSMEMBAR);
   NVC0C0_QMDV02_01_DEF_SET(qmd, CWD_MEMBAR_TYPE, L1_SYSMEMBAR);
   NVC0C0_QMDV02_01_DEF_SET(qmd, API_VISIBLE_CALL_LIMIT, NO_CHECK);

   NVC0C0_QMDV02_01_VAL_SET(qmd, PROGRAM_OFFSET, cp->code_base);

   NVC0C0_QMDV02_01_VAL_SET(qmd, CTA_RASTER_WIDTH, info->grid[0]);
   NVC0C0_QMDV02_01_VAL_SET(qmd, CTA_RASTER_HEIGHT, info->grid[1]);
   NVC0C0_QMDV02_01_VAL_SET(qmd, CTA_RASTER_DEPTH, info->grid[2]);
   NVC0C0_QMDV02_01_VAL_SET(qmd, CTA_THREAD_DIMENSION0, info->block[0]);
   NVC0C0_QMDV02_01_VAL_SET(qmd, CTA_THREAD_DIMENSION1, info->block[1]);
   NVC0C0_QMDV02_01_VAL_SET(qmd, CTA_THREAD_DIMENSION2, info->block[2]);

   NVC0C0_QMDV02_01_VAL_SET(qmd, SHARED_MEMORY_SIZE, align(shared_size, 0x100));
   NVC0C0_QMDV02_01_VAL_SET(qmd, SHADER_LOCAL_MEMORY_LOW_SIZE, cp->hdr[1] & 0xfffff0);
   NVC0C0_QMDV02_01_VAL_SET(qmd, SHADER_LOCAL_MEMORY_HIGH_SIZE, 0);
   NVC0C0_QMDV02_01_VAL_SET(qmd, SHADER_LOCAL_MEMORY_CRS_SIZE, 0x800);

   NVC0C0_QMDV02_01_VAL_SET(qmd, REGISTER_COUNT, cp->num_gprs);
   NVC0C0_QMDV02_01_VAL_SET(qmd, BARRIER_COUNT, cp->num_barriers);

   // Only bind user uniforms and the driver constant buffer through the
   // launch descriptor because UBOs are sticked to the driver cb to avoid the
   // limitation of 8 CBs.
   if (nvc0->constbuf[5][0].user || cp->parm_size) {
      gp100_cp_launch_desc_set_cb(qmd, 0, screen->uniform_bo,
                                  NVC0_CB_USR_INFO(5), 1 << 16);

      // Later logic will attempt to bind a real buffer at position 0. That
      // should not happen if we've bound a user buffer.
      assert(nvc0->constbuf[5][0].user || !nvc0->constbuf[5][0].u.buf);
   }
   gp100_cp_launch_desc_set_cb(qmd, 7, screen->uniform_bo,
                               NVC0_CB_AUX_INFO(5), 1 << 11);

   nve4_compute_setup_buf_cb(nvc0, true, qmd);
}

static int
gv100_sm_config_smem_size(u32 size)
{
   if      (size > 64 * 1024) size = 96 * 1024;
   else if (size > 32 * 1024) size = 64 * 1024;
   else if (size > 16 * 1024) size = 32 * 1024;
   else if (size >  8 * 1024) size = 16 * 1024;
   else                       size =  8 * 1024;
   return (size / 4096) + 1;
}

static void
gv100_compute_setup_launch_desc(struct nvc0_context *nvc0, u32 *qmd,
                                const struct pipe_grid_info *info)
{
   struct nvc0_program *cp = nvc0->compprog;
   struct nvc0_screen *screen = nvc0->screen;
   uint64_t entry = screen->text->offset + cp->code_base;
   uint32_t shared_size = cp->cp.smem_size + info->variable_shared_mem;

   NVC3C0_QMDV02_02_VAL_SET(qmd, SM_GLOBAL_CACHING_ENABLE, 1);
   NVC3C0_QMDV02_02_DEF_SET(qmd, API_VISIBLE_CALL_LIMIT, NO_CHECK);
   NVC3C0_QMDV02_02_DEF_SET(qmd, SAMPLER_INDEX, INDEPENDENTLY);
   NVC3C0_QMDV02_02_VAL_SET(qmd, SHARED_MEMORY_SIZE, align(shared_size, 0x100));
   NVC3C0_QMDV02_02_VAL_SET(qmd, SHADER_LOCAL_MEMORY_LOW_SIZE, cp->hdr[1] & 0xfffff0);
   NVC3C0_QMDV02_02_VAL_SET(qmd, SHADER_LOCAL_MEMORY_HIGH_SIZE, 0);
   NVC3C0_QMDV02_02_VAL_SET(qmd, MIN_SM_CONFIG_SHARED_MEM_SIZE,
                                  gv100_sm_config_smem_size(8 * 1024));
   NVC3C0_QMDV02_02_VAL_SET(qmd, MAX_SM_CONFIG_SHARED_MEM_SIZE,
                                  gv100_sm_config_smem_size(96 * 1024));
   NVC3C0_QMDV02_02_VAL_SET(qmd, QMD_VERSION, 2);
   NVC3C0_QMDV02_02_VAL_SET(qmd, QMD_MAJOR_VERSION, 2);
   NVC3C0_QMDV02_02_VAL_SET(qmd, TARGET_SM_CONFIG_SHARED_MEM_SIZE,
                                  gv100_sm_config_smem_size(shared_size));

   NVC3C0_QMDV02_02_VAL_SET(qmd, CTA_RASTER_WIDTH, info->grid[0]);
   NVC3C0_QMDV02_02_VAL_SET(qmd, CTA_RASTER_HEIGHT, info->grid[1]);
   NVC3C0_QMDV02_02_VAL_SET(qmd, CTA_RASTER_DEPTH, info->grid[2]);
   NVC3C0_QMDV02_02_VAL_SET(qmd, CTA_THREAD_DIMENSION0, info->block[0]);
   NVC3C0_QMDV02_02_VAL_SET(qmd, CTA_THREAD_DIMENSION1, info->block[1]);
   NVC3C0_QMDV02_02_VAL_SET(qmd, CTA_THREAD_DIMENSION2, info->block[2]);
   NVC3C0_QMDV02_02_VAL_SET(qmd, REGISTER_COUNT_V, cp->num_gprs);
   NVC3C0_QMDV02_02_VAL_SET(qmd, BARRIER_COUNT, cp->num_barriers);

   // Only bind user uniforms and the driver constant buffer through the
   // launch descriptor because UBOs are sticked to the driver cb to avoid the
   // limitation of 8 CBs.
   if (nvc0->constbuf[5][0].user || cp->parm_size) {
      gp100_cp_launch_desc_set_cb(qmd, 0, screen->uniform_bo,
                                  NVC0_CB_USR_INFO(5), 1 << 16);

      // Later logic will attempt to bind a real buffer at position 0. That
      // should not happen if we've bound a user buffer.
      assert(nvc0->constbuf[5][0].user || !nvc0->constbuf[5][0].u.buf);
   }
   gp100_cp_launch_desc_set_cb(qmd, 7, screen->uniform_bo,
                               NVC0_CB_AUX_INFO(5), 1 << 11);

   nve4_compute_setup_buf_cb(nvc0, true, qmd);

   NVC3C0_QMDV02_02_VAL_SET(qmd, PROGRAM_ADDRESS_LOWER, entry & 0xffffffff);
   NVC3C0_QMDV02_02_VAL_SET(qmd, PROGRAM_ADDRESS_UPPER, entry >> 32);
}

static inline void *
nve4_compute_alloc_launch_desc(struct nouveau_context *nv,
                               struct nouveau_bo **pbo, uint64_t *pgpuaddr)
{
   uint8_t *ptr = nouveau_scratch_get(nv, 512, pgpuaddr, pbo);
   if (!ptr)
      return NULL;
   if (*pgpuaddr & 255) {
      unsigned adj = 256 - (*pgpuaddr & 255);
      ptr += adj;
      *pgpuaddr += adj;
   }
   memset(ptr, 0x00, 256);
   return ptr;
}

static void
nve4_upload_indirect_desc(struct nouveau_pushbuf *push,
                          struct nv04_resource *res,  uint64_t gpuaddr,
                          uint32_t length, uint32_t bo_offset)
{
   BEGIN_NVC0(push, NVE4_CP(UPLOAD_DST_ADDRESS_HIGH), 2);
   PUSH_DATAh(push, gpuaddr);
   PUSH_DATA (push, gpuaddr);
   BEGIN_NVC0(push, NVE4_CP(UPLOAD_LINE_LENGTH_IN), 2);
   PUSH_DATA (push, length);
   PUSH_DATA (push, 1);

   PUSH_SPACE_EX(push, 32, 0, 1);
   PUSH_REF1(push, res->bo, NOUVEAU_BO_RD | res->domain);

   BEGIN_1IC0(push, NVE4_CP(UPLOAD_EXEC), 1 + (length / 4));
   PUSH_DATA (push, NVE4_COMPUTE_UPLOAD_EXEC_LINEAR | (0x08 << 1));
   nouveau_pushbuf_data(push, res->bo, bo_offset,
                        NVC0_IB_ENTRY_1_NO_PREFETCH | length);
}

void
nve4_launch_grid(struct pipe_context *pipe, const struct pipe_grid_info *info)
{
   struct nvc0_context *nvc0 = nvc0_context(pipe);
   struct nvc0_screen *screen = nvc0->screen;
   struct nouveau_pushbuf *push = nvc0->base.pushbuf;
   void *desc;
   uint64_t desc_gpuaddr;
   struct nouveau_bo *desc_bo;
   int ret;

   desc = nve4_compute_alloc_launch_desc(&nvc0->base, &desc_bo, &desc_gpuaddr);
   if (!desc) {
      ret = -1;
      goto out;
   }
   BCTX_REFN_bo(nvc0->bufctx_cp, CP_DESC, NOUVEAU_BO_GART | NOUVEAU_BO_RD,
                desc_bo);

   list_for_each_entry(struct nvc0_resident, resident, &nvc0->tex_head, list) {
      nvc0_add_resident(nvc0->bufctx_cp, NVC0_BIND_CP_BINDLESS, resident->buf,
                        resident->flags);
   }

   list_for_each_entry(struct nvc0_resident, resident, &nvc0->img_head, list) {
      nvc0_add_resident(nvc0->bufctx_cp, NVC0_BIND_CP_BINDLESS, resident->buf,
                        resident->flags);
   }

   simple_mtx_lock(&screen->state_lock);
   ret = !nve4_state_validate_cp(nvc0, ~0);
   if (ret)
      goto out_unlock;

   if (nvc0->screen->compute->oclass >= GV100_COMPUTE_CLASS)
      gv100_compute_setup_launch_desc(nvc0, desc, info);
   else
   if (nvc0->screen->compute->oclass >= GP100_COMPUTE_CLASS)
      gp100_compute_setup_launch_desc(nvc0, desc, info);
   else
      nve4_compute_setup_launch_desc(nvc0, desc, info);

   nve4_compute_upload_input(nvc0, info);

#ifndef NDEBUG
   if (debug_get_num_option("NV50_PROG_DEBUG", 0)) {
      debug_printf("Queue Meta Data:\n");
      if (nvc0->screen->compute->oclass >= GV100_COMPUTE_CLASS)
         NVC3C0QmdDump_V02_02(desc);
      else
      if (nvc0->screen->compute->oclass >= GP100_COMPUTE_CLASS)
         NVC0C0QmdDump_V02_01(desc);
      else
         NVA0C0QmdDump_V00_06(desc);
   }
#endif

   if (unlikely(info->indirect)) {
      struct nv04_resource *res = nv04_resource(info->indirect);
      uint32_t offset = res->offset + info->indirect_offset;

      /* upload the descriptor */
      BEGIN_NVC0(push, NVE4_CP(UPLOAD_DST_ADDRESS_HIGH), 2);
      PUSH_DATAh(push, desc_gpuaddr);
      PUSH_DATA (push, desc_gpuaddr);
      BEGIN_NVC0(push, NVE4_CP(UPLOAD_LINE_LENGTH_IN), 2);
      PUSH_DATA (push, 256);
      PUSH_DATA (push, 1);
      BEGIN_1IC0(push, NVE4_CP(UPLOAD_EXEC), 1 + (256 / 4));
      PUSH_DATA (push, NVE4_COMPUTE_UPLOAD_EXEC_LINEAR | (0x08 << 1));
      PUSH_DATAp(push, (const uint32_t *)desc, 256 / 4);

      if (nvc0->screen->compute->oclass >= GP100_COMPUTE_CLASS) {
         nve4_upload_indirect_desc(push, res, desc_gpuaddr + 48, 12, offset);
      } else {
         /* overwrite griddim_x and griddim_y as two 32-bits integers even
          * if griddim_y must be a 16-bits integer */
         nve4_upload_indirect_desc(push, res, desc_gpuaddr + 48, 8, offset);

         /* overwrite the 16 high bits of griddim_y with griddim_z because
          * we need (z << 16) | x */
         nve4_upload_indirect_desc(push, res, desc_gpuaddr + 54, 4, offset + 8);
      }
   }

   /* upload descriptor and flush */
   PUSH_SPACE_EX(push, 32, 1, 0);
   PUSH_REF1(push, screen->text, NV_VRAM_DOMAIN(&screen->base) | NOUVEAU_BO_RD);
   BEGIN_NVC0(push, NVE4_CP(LAUNCH_DESC_ADDRESS), 1);
   PUSH_DATA (push, desc_gpuaddr >> 8);
   if (screen->compute->oclass < GA102_COMPUTE_CLASS) {
      BEGIN_NVC0(push, NVE4_CP(LAUNCH), 1);
      PUSH_DATA (push, 0x3);
   } else {
      BEGIN_NIC0(push, SUBC_CP(0x02c0), 2);
      PUSH_DATA (push, 1);
      PUSH_DATA (push, 2);
   }
   BEGIN_NVC0(push, SUBC_CP(NV50_GRAPH_SERIALIZE), 1);
   PUSH_DATA (push, 0);

   nvc0_update_compute_invocations_counter(nvc0, info);

out_unlock:
   PUSH_KICK(push);
   simple_mtx_unlock(&screen->state_lock);

out:
   if (ret)
      NOUVEAU_ERR("Failed to launch grid !\n");
   nouveau_scratch_done(&nvc0->base);
   nouveau_bufctx_reset(nvc0->bufctx_cp, NVC0_BIND_CP_DESC);
   nouveau_bufctx_reset(nvc0->bufctx_cp, NVC0_BIND_CP_BINDLESS);
}


#define NVE4_TIC_ENTRY_INVALID 0x000fffff

static void
nve4_compute_validate_textures(struct nvc0_context *nvc0)
{
   struct nouveau_bo *txc = nvc0->screen->txc;
   struct nouveau_pushbuf *push = nvc0->base.pushbuf;
   const unsigned s = 5;
   unsigned i;
   uint32_t commands[2][32];
   unsigned n[2] = { 0, 0 };

   for (i = 0; i < nvc0->num_textures[s]; ++i) {
      struct nv50_tic_entry *tic = nv50_tic_entry(nvc0->textures[s][i]);
      struct nv04_resource *res;
      const bool dirty = !!(nvc0->textures_dirty[s] & (1 << i));

      if (!tic) {
         nvc0->tex_handles[s][i] |= NVE4_TIC_ENTRY_INVALID;
         continue;
      }
      res = nv04_resource(tic->pipe.texture);
      nvc0_update_tic(nvc0, tic, res);

      if (tic->id < 0) {
         tic->id = nvc0_screen_tic_alloc(nvc0->screen, tic);

         PUSH_SPACE(push, 16);
         BEGIN_NVC0(push, NVE4_CP(UPLOAD_DST_ADDRESS_HIGH), 2);
         PUSH_DATAh(push, txc->offset + (tic->id * 32));
         PUSH_DATA (push, txc->offset + (tic->id * 32));
         BEGIN_NVC0(push, NVE4_CP(UPLOAD_LINE_LENGTH_IN), 2);
         PUSH_DATA (push, 32);
         PUSH_DATA (push, 1);
         BEGIN_1IC0(push, NVE4_CP(UPLOAD_EXEC), 9);
         PUSH_DATA (push, NVE4_COMPUTE_UPLOAD_EXEC_LINEAR | (0x20 << 1));
         PUSH_DATAp(push, &tic->tic[0], 8);

         commands[0][n[0]++] = (tic->id << 4) | 1;
      } else
      if (res->status & NOUVEAU_BUFFER_STATUS_GPU_WRITING) {
         commands[1][n[1]++] = (tic->id << 4) | 1;
      }
      nvc0->screen->tic.lock[tic->id / 32] |= 1 << (tic->id % 32);

      res->status &= ~NOUVEAU_BUFFER_STATUS_GPU_WRITING;
      res->status |=  NOUVEAU_BUFFER_STATUS_GPU_READING;

      nvc0->tex_handles[s][i] &= ~NVE4_TIC_ENTRY_INVALID;
      nvc0->tex_handles[s][i] |= tic->id;
      if (dirty)
         BCTX_REFN(nvc0->bufctx_cp, CP_TEX(i), res, RD);
   }
   for (; i < nvc0->state.num_textures[s]; ++i) {
      nvc0->tex_handles[s][i] |= NVE4_TIC_ENTRY_INVALID;
      nvc0->textures_dirty[s] |= 1 << i;
   }

   if (n[0]) {
      BEGIN_NIC0(push, NVE4_CP(TIC_FLUSH), n[0]);
      PUSH_DATAp(push, commands[0], n[0]);
   }
   if (n[1]) {
      BEGIN_NIC0(push, NVE4_CP(TEX_CACHE_CTL), n[1]);
      PUSH_DATAp(push, commands[1], n[1]);
   }

   nvc0->state.num_textures[s] = nvc0->num_textures[s];

   /* Invalidate all 3D textures because they are aliased. */
   for (int s = 0; s < 5; s++) {
      for (int i = 0; i < nvc0->num_textures[s]; i++)
         nouveau_bufctx_reset(nvc0->bufctx_3d, NVC0_BIND_3D_TEX(s, i));
      nvc0->textures_dirty[s] = ~0;
   }
   nvc0->dirty_3d |= NVC0_NEW_3D_TEXTURES;
}

#ifdef NOUVEAU_NVE4_MP_TRAP_HANDLER
static void
nve4_compute_trap_info(struct nvc0_context *nvc0)
{
   struct nvc0_screen *screen = nvc0->screen;
   struct nouveau_bo *bo = screen->parm;
   int ret, i;
   volatile struct nve4_mp_trap_info *info;
   uint8_t *map;

   ret = BO_MAP(&screen->base, bo, NOUVEAU_BO_RDWR, nvc0->base.client);
   if (ret)
      return;
   map = (uint8_t *)bo->map;
   info = (volatile struct nve4_mp_trap_info *)(map + NVE4_CP_PARAM_TRAP_INFO);

   if (info->lock) {
      debug_printf("trapstat = %08x\n", info->trapstat);
      debug_printf("warperr = %08x\n", info->warperr);
      debug_printf("PC = %x\n", info->pc);
      debug_printf("tid = %u %u %u\n",
                   info->tid[0], info->tid[1], info->tid[2]);
      debug_printf("ctaid = %u %u %u\n",
                   info->ctaid[0], info->ctaid[1], info->ctaid[2]);
      for (i = 0; i <= 63; ++i)
         debug_printf("$r%i = %08x\n", i, info->r[i]);
      for (i = 0; i <= 6; ++i)
         debug_printf("$p%i = %i\n", i, (info->flags >> i) & 1);
      debug_printf("$c = %x\n", info->flags >> 12);
   }
   info->lock = 0;
}
#endif
