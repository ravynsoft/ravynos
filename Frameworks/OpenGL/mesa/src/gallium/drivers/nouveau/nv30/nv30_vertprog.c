/*
 * Copyright 2012 Red Hat Inc.
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
 * Authors: Ben Skeggs
 *
 */

#include "draw/draw_context.h"
#include "util/u_dynarray.h"
#include "tgsi/tgsi_parse.h"
#include "nir/nir_to_tgsi.h"

#include "nv_object.xml.h"
#include "nv30/nv30-40_3d.xml.h"
#include "nv30/nv30_context.h"
#include "nv30/nvfx_shader.h"
#include "nv30/nv30_state.h"
#include "nv30/nv30_winsys.h"

static void
nv30_vertprog_destroy(struct nv30_vertprog *vp)
{
   util_dynarray_fini(&vp->branch_relocs);
   nouveau_heap_free(&vp->exec);
   FREE(vp->insns);
   vp->insns = NULL;
   vp->nr_insns = 0;

   util_dynarray_fini(&vp->const_relocs);
   nouveau_heap_free(&vp->data);
   FREE(vp->consts);
   vp->consts = NULL;
   vp->nr_consts = 0;

   vp->translated = false;
}

void
nv30_vertprog_validate(struct nv30_context *nv30)
{
   struct nouveau_pushbuf *push = nv30->base.pushbuf;
   struct nouveau_object *eng3d = nv30->screen->eng3d;
   struct nv30_vertprog *vp = nv30->vertprog.program;
   struct nv30_fragprog *fp = nv30->fragprog.program;
   bool upload_code = false;
   bool upload_data = false;
   unsigned i;

   if (nv30->dirty & NV30_NEW_FRAGPROG) {
      if (memcmp(vp->texcoord, fp->texcoord, sizeof(vp->texcoord))) {
         if (vp->translated)
            nv30_vertprog_destroy(vp);
         memcpy(vp->texcoord, fp->texcoord, sizeof(vp->texcoord));
      }
   }

   if (nv30->rast && nv30->rast->pipe.clip_plane_enable != vp->enabled_ucps) {
      vp->enabled_ucps = nv30->rast->pipe.clip_plane_enable;
      if (vp->translated)
         nv30_vertprog_destroy(vp);
   }

   if (!vp->translated) {
      vp->translated = _nvfx_vertprog_translate(eng3d->oclass, vp);
      if (!vp->translated) {
         nv30->draw_flags |= NV30_NEW_VERTPROG;
         return;
      }
      nv30->dirty |= NV30_NEW_VERTPROG;
   }

   if (!vp->exec) {
      struct nouveau_heap *heap = nv30->screen->vp_exec_heap;
      struct nv30_shader_reloc *reloc = vp->branch_relocs.data;
      unsigned nr_reloc = vp->branch_relocs.size / sizeof(*reloc);
      uint32_t *inst, target;

      if (nouveau_heap_alloc(heap, vp->nr_insns, &vp->exec, &vp->exec)) {
         while (heap->next && heap->size < vp->nr_insns) {
            struct nouveau_heap **evict = heap->next->priv;
            nouveau_heap_free(evict);
         }

         if (nouveau_heap_alloc(heap, vp->nr_insns, &vp->exec, &vp->exec)) {
            nv30->draw_flags |= NV30_NEW_VERTPROG;
            return;
         }
      }

      if (eng3d->oclass < NV40_3D_CLASS) {
         while (nr_reloc--) {
            inst     = vp->insns[reloc->location].data;
            target   = vp->exec->start + reloc->target;

            inst[2] &= ~0x000007fc;
            inst[2] |= target << 2;
            reloc++;
         }
      } else {
         while (nr_reloc--) {
            inst     = vp->insns[reloc->location].data;
            target   = vp->exec->start + reloc->target;

            inst[2] &= ~0x0000003f;
            inst[2] |= target >> 3;
            inst[3] &= ~0xe0000000;
            inst[3] |= target << 29;
            reloc++;
         }
      }

      upload_code = true;
   }

   if (vp->nr_consts && !vp->data) {
      struct nouveau_heap *heap = nv30->screen->vp_data_heap;
      struct nv30_shader_reloc *reloc = vp->const_relocs.data;
      unsigned nr_reloc = vp->const_relocs.size / sizeof(*reloc);
      uint32_t *inst, target;

      if (nouveau_heap_alloc(heap, vp->nr_consts, vp, &vp->data)) {
         while (heap->next && heap->size < vp->nr_consts) {
            struct nv30_vertprog *evp = heap->next->priv;
            nouveau_heap_free(&evp->data);
         }

         if (nouveau_heap_alloc(heap, vp->nr_consts, vp, &vp->data)) {
            nv30->draw_flags |= NV30_NEW_VERTPROG;
            return;
         }
      }

      if (eng3d->oclass < NV40_3D_CLASS) {
         while (nr_reloc--) {
            inst     = vp->insns[reloc->location].data;
            target   = vp->data->start + reloc->target;

            inst[1] &= ~0x0007fc000;
            inst[1] |= (target & 0x1ff) << 14;
            reloc++;
         }
      } else {
         while (nr_reloc--) {
            inst     = vp->insns[reloc->location].data;
            target   = vp->data->start + reloc->target;

            inst[1] &= ~0x0001ff000;
            inst[1] |= (target & 0x1ff) << 12;
            reloc++;
         }
      }

      upload_code = true;
      upload_data = true;
   }

   if (vp->nr_consts) {
      struct nv04_resource *res = nv04_resource(nv30->vertprog.constbuf);

      for (i = 0; i < vp->nr_consts; i++) {
         struct nv30_vertprog_data *data = &vp->consts[i];

         if (data->index < 0) {
            if (!upload_data)
               continue;
         } else {
            float *constbuf = (float *)res->data;
            if (!upload_data &&
                !memcmp(data->value, &constbuf[data->index * 4], 16))
               continue;
            memcpy(data->value, &constbuf[data->index * 4], 16);
         }

         BEGIN_NV04(push, NV30_3D(VP_UPLOAD_CONST_ID), 5);
         PUSH_DATA (push, vp->data->start + i);
         PUSH_DATAp(push, data->value, 4);
      }
   }

   if (upload_code) {
      BEGIN_NV04(push, NV30_3D(VP_UPLOAD_FROM_ID), 1);
      PUSH_DATA (push, vp->exec->start);
      for (i = 0; i < vp->nr_insns; i++) {
         BEGIN_NV04(push, NV30_3D(VP_UPLOAD_INST(0)), 4);
         PUSH_DATAp(push, vp->insns[i].data, 4);
      }
   }

   if (nv30->dirty & (NV30_NEW_VERTPROG | NV30_NEW_FRAGPROG)) {
      BEGIN_NV04(push, NV30_3D(VP_START_FROM_ID), 1);
      PUSH_DATA (push, vp->exec->start);
      if (eng3d->oclass < NV40_3D_CLASS) {
         BEGIN_NV04(push, NV30_3D(ENGINE), 1);
         PUSH_DATA (push, 0x00000013); /* vp instead of ff, somehow */
      } else {
         BEGIN_NV04(push, NV40_3D(VP_ATTRIB_EN), 2);
         PUSH_DATA (push, vp->ir);
         PUSH_DATA (push, vp->or | fp->vp_or);
         BEGIN_NV04(push, NV30_3D(ENGINE), 1);
         PUSH_DATA (push, 0x00000011);
      }
   }
}

static void *
nv30_vp_state_create(struct pipe_context *pipe,
                     const struct pipe_shader_state *cso)
{
   struct nv30_vertprog *vp = CALLOC_STRUCT(nv30_vertprog);
   if (!vp)
      return NULL;

   if (cso->type == PIPE_SHADER_IR_NIR) {
      vp->pipe.tokens = nir_to_tgsi(cso->ir.nir, pipe->screen);
   } else {
      assert(cso->type == PIPE_SHADER_IR_TGSI);
      /* we need to keep a local copy of the tokens */
      vp->pipe.tokens = tgsi_dup_tokens(cso->tokens);
   }

   tgsi_scan_shader(vp->pipe.tokens, &vp->info);
   return vp;
}

static void
nv30_vp_state_delete(struct pipe_context *pipe, void *hwcso)
{
   struct nv30_vertprog *vp = hwcso;

   if (vp->translated)
      nv30_vertprog_destroy(vp);

   if (vp->draw)
      draw_delete_vertex_shader(nv30_context(pipe)->draw, vp->draw);

   FREE((void *)vp->pipe.tokens);
   FREE(vp);
}

static void
nv30_vp_state_bind(struct pipe_context *pipe, void *hwcso)
{
   struct nv30_context *nv30 = nv30_context(pipe);

   nv30->vertprog.program = hwcso;
   nv30->dirty |= NV30_NEW_VERTPROG;
}

void
nv30_vertprog_init(struct pipe_context *pipe)
{
   pipe->create_vs_state = nv30_vp_state_create;
   pipe->bind_vs_state = nv30_vp_state_bind;
   pipe->delete_vs_state = nv30_vp_state_delete;
}
