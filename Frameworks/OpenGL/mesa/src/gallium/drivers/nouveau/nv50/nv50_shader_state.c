/*
 * Copyright 2008 Ben Skeggs
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

#include "nv50/nv50_context.h"
#include "nv50/nv50_query_hw.h"

#include "nv50/nv50_compute.xml.h"

void
nv50_constbufs_validate(struct nv50_context *nv50)
{
   struct nouveau_pushbuf *push = nv50->base.pushbuf;
   unsigned s;

   for (s = 0; s < NV50_MAX_3D_SHADER_STAGES; ++s) {
      unsigned p;

      if (s == NV50_SHADER_STAGE_FRAGMENT)
         p = NV50_3D_SET_PROGRAM_CB_PROGRAM_FRAGMENT;
      else
      if (s == NV50_SHADER_STAGE_GEOMETRY)
         p = NV50_3D_SET_PROGRAM_CB_PROGRAM_GEOMETRY;
      else
         p = NV50_3D_SET_PROGRAM_CB_PROGRAM_VERTEX;

      while (nv50->constbuf_dirty[s]) {
         const unsigned i = (unsigned)ffs(nv50->constbuf_dirty[s]) - 1;

         assert(i < NV50_MAX_PIPE_CONSTBUFS);
         nv50->constbuf_dirty[s] &= ~(1 << i);

         if (nv50->constbuf[s][i].user) {
            const unsigned b = NV50_CB_PVP + s;
            unsigned start = 0;
            unsigned words = nv50->constbuf[s][0].size / 4;
            if (i) {
               NOUVEAU_ERR("user constbufs only supported in slot 0\n");
               continue;
            }
            if (!nv50->state.uniform_buffer_bound[s]) {
               nv50->state.uniform_buffer_bound[s] = true;
               BEGIN_NV04(push, NV50_3D(SET_PROGRAM_CB), 1);
               PUSH_DATA (push, (b << 12) | (i << 8) | p | 1);
            }
            while (words) {
               unsigned nr = MIN2(words, NV04_PFIFO_MAX_PACKET_LEN);

               PUSH_SPACE(push, nr + 3);
               BEGIN_NV04(push, NV50_3D(CB_ADDR), 1);
               PUSH_DATA (push, (start << 8) | b);
               BEGIN_NI04(push, NV50_3D(CB_DATA(0)), nr);
               PUSH_DATAp(push, &nv50->constbuf[s][0].u.data[start * 4], nr);

               start += nr;
               words -= nr;
            }
         } else {
            struct nv04_resource *res =
               nv04_resource(nv50->constbuf[s][i].u.buf);
            if (res) {
               /* TODO: allocate persistent bindings */
               const unsigned b = s * 16 + i;

               assert(nouveau_resource_mapped_by_gpu(&res->base));

               BEGIN_NV04(push, NV50_3D(CB_DEF_ADDRESS_HIGH), 3);
               PUSH_DATAh(push, res->address + nv50->constbuf[s][i].offset);
               PUSH_DATA (push, res->address + nv50->constbuf[s][i].offset);
               PUSH_DATA (push, (b << 16) |
                          (nv50->constbuf[s][i].size & 0xffff));
               BEGIN_NV04(push, NV50_3D(SET_PROGRAM_CB), 1);
               PUSH_DATA (push, (b << 12) | (i << 8) | p | 1);

               BCTX_REFN(nv50->bufctx_3d, 3D_CB(s, i), res, RD);

               nv50->cb_dirty = 1; /* Force cache flush for UBO. */
               res->cb_bindings[s] |= 1 << i;
            } else {
               BEGIN_NV04(push, NV50_3D(SET_PROGRAM_CB), 1);
               PUSH_DATA (push, (i << 8) | p | 0);
            }
            if (i == 0)
               nv50->state.uniform_buffer_bound[s] = false;
         }
      }
   }

   /* Invalidate all COMPUTE constbufs because they are aliased with 3D. */
   nv50->dirty_cp |= NV50_NEW_CP_CONSTBUF;
   nv50->constbuf_dirty[NV50_SHADER_STAGE_COMPUTE] |= nv50->constbuf_valid[NV50_SHADER_STAGE_COMPUTE];
   nv50->state.uniform_buffer_bound[NV50_SHADER_STAGE_COMPUTE] = false;
}

static bool
nv50_program_validate(struct nv50_context *nv50, struct nv50_program *prog)
{
   if (!prog->translated) {
      prog->translated = nv50_program_translate(
         prog, nv50->screen->base.device->chipset, &nv50->base.debug);
      if (!prog->translated)
         return false;
   } else
   if (prog->mem)
      return true;

   simple_mtx_assert_locked(&nv50->screen->state_lock);
   return nv50_program_upload_code(nv50, prog);
}

static inline void
nv50_program_update_context_state(struct nv50_context *nv50,
                                  struct nv50_program *prog, int stage)
{
   const unsigned flags = NOUVEAU_BO_VRAM | NOUVEAU_BO_RDWR;

   if (prog && prog->tls_space) {
      if (nv50->state.new_tls_space)
         nouveau_bufctx_reset(nv50->bufctx_3d, NV50_BIND_3D_TLS);
      if (!nv50->state.tls_required || nv50->state.new_tls_space)
         BCTX_REFN_bo(nv50->bufctx_3d, 3D_TLS, flags, nv50->screen->tls_bo);
      nv50->state.new_tls_space = false;
      nv50->state.tls_required |= 1 << stage;
   } else {
      if (nv50->state.tls_required == (1 << stage))
         nouveau_bufctx_reset(nv50->bufctx_3d, NV50_BIND_3D_TLS);
      nv50->state.tls_required &= ~(1 << stage);
   }
}

void
nv50_vertprog_validate(struct nv50_context *nv50)
{
   struct nouveau_pushbuf *push = nv50->base.pushbuf;
   struct nv50_program *vp = nv50->vertprog;

   if (!nv50_program_validate(nv50, vp))
         return;
   nv50_program_update_context_state(nv50, vp, 0);

   BEGIN_NV04(push, NV50_3D(VP_ATTR_EN(0)), 2);
   PUSH_DATA (push, vp->vp.attrs[0]);
   PUSH_DATA (push, vp->vp.attrs[1]);
   BEGIN_NV04(push, NV50_3D(VP_REG_ALLOC_RESULT), 1);
   PUSH_DATA (push, vp->max_out);
   BEGIN_NV04(push, NV50_3D(VP_REG_ALLOC_TEMP), 1);
   PUSH_DATA (push, vp->max_gpr);
   BEGIN_NV04(push, NV50_3D(VP_START_ID), 1);
   PUSH_DATA (push, vp->code_base);
}

void
nv50_fragprog_validate(struct nv50_context *nv50)
{
   struct nouveau_pushbuf *push = nv50->base.pushbuf;
   struct nv50_program *fp = nv50->fragprog;
   struct pipe_rasterizer_state *rast = &nv50->rast->pipe;

   if (!fp || !rast)
      return;

   if (nv50->zsa && nv50->zsa->pipe.alpha_enabled) {
      struct pipe_framebuffer_state *fb = &nv50->framebuffer;
      bool blendable = fb->nr_cbufs == 0 || !fb->cbufs[0] ||
         nv50->screen->base.base.is_format_supported(
               &nv50->screen->base.base,
               fb->cbufs[0]->format,
               fb->cbufs[0]->texture->target,
               fb->cbufs[0]->texture->nr_samples,
               fb->cbufs[0]->texture->nr_storage_samples,
               PIPE_BIND_BLENDABLE);
      /* If we already have alphatest code, we have to keep updating
       * it. However we only have to have different code if the current RT0 is
       * non-blendable. Otherwise we just set it to always pass and use the
       * hardware alpha test.
       */
      if (fp->fp.alphatest || !blendable) {
         uint8_t alphatest = PIPE_FUNC_ALWAYS + 1;
         if (!blendable)
            alphatest = nv50->zsa->pipe.alpha_func + 1;
         if (!fp->fp.alphatest)
            nv50_program_destroy(nv50, fp);
         else if (fp->mem && fp->fp.alphatest != alphatest)
            nouveau_heap_free(&fp->mem);

         fp->fp.alphatest = alphatest;
      }
   } else if (fp->fp.alphatest && fp->fp.alphatest != PIPE_FUNC_ALWAYS + 1) {
      /* Alpha test is disabled but we have a shader where it's filled
       * in. Make sure to reset the function to 'always', otherwise it'll end
       * up discarding fragments incorrectly.
       */
      if (fp->mem)
         nouveau_heap_free(&fp->mem);

      fp->fp.alphatest = PIPE_FUNC_ALWAYS + 1;
   }

   if (fp->fp.force_persample_interp != rast->force_persample_interp) {
      /* Force the program to be reuploaded, which will trigger interp fixups
       * to get applied
       */
      if (fp->mem)
         nouveau_heap_free(&fp->mem);

      fp->fp.force_persample_interp = rast->force_persample_interp;
   }

   if (fp->mem && !(nv50->dirty_3d & (NV50_NEW_3D_FRAGPROG | NV50_NEW_3D_MIN_SAMPLES)))
      return;

   if (!nv50_program_validate(nv50, fp))
      return;
   nv50_program_update_context_state(nv50, fp, 1);

   BEGIN_NV04(push, NV50_3D(FP_REG_ALLOC_TEMP), 1);
   PUSH_DATA (push, fp->max_gpr);
   BEGIN_NV04(push, NV50_3D(FP_RESULT_COUNT), 1);
   PUSH_DATA (push, fp->max_out);
   BEGIN_NV04(push, NV50_3D(FP_CONTROL), 1);
   PUSH_DATA (push, fp->fp.flags[0]);
   BEGIN_NV04(push, NV50_3D(FP_CTRL_UNK196C), 1);
   PUSH_DATA (push, fp->fp.flags[1]);
   BEGIN_NV04(push, NV50_3D(FP_START_ID), 1);
   PUSH_DATA (push, fp->code_base);

   if (nv50->screen->tesla->oclass >= NVA3_3D_CLASS) {
      BEGIN_NV04(push, SUBC_3D(NVA3_3D_FP_MULTISAMPLE), 1);
      if (nv50->min_samples > 1 || fp->fp.has_samplemask)
         PUSH_DATA(push,
                   NVA3_3D_FP_MULTISAMPLE_FORCE_PER_SAMPLE |
                   (NVA3_3D_FP_MULTISAMPLE_EXPORT_SAMPLE_MASK *
                    fp->fp.has_samplemask));
      else
         PUSH_DATA(push, 0);
   }
}

void
nv50_gmtyprog_validate(struct nv50_context *nv50)
{
   struct nouveau_pushbuf *push = nv50->base.pushbuf;
   struct nv50_program *gp = nv50->gmtyprog;

   if (gp) {
      if (!nv50_program_validate(nv50, gp))
         return;
      BEGIN_NV04(push, NV50_3D(GP_REG_ALLOC_TEMP), 1);
      PUSH_DATA (push, gp->max_gpr);
      BEGIN_NV04(push, NV50_3D(GP_REG_ALLOC_RESULT), 1);
      PUSH_DATA (push, gp->max_out);
      BEGIN_NV04(push, NV50_3D(GP_OUTPUT_PRIMITIVE_TYPE), 1);
      PUSH_DATA (push, gp->gp.prim_type);
      BEGIN_NV04(push, NV50_3D(GP_VERTEX_OUTPUT_COUNT), 1);
      PUSH_DATA (push, gp->gp.vert_count);
      BEGIN_NV04(push, NV50_3D(GP_START_ID), 1);
      PUSH_DATA (push, gp->code_base);

      nv50->state.prim_size = gp->gp.prim_type; /* enum matches vertex count */
   }
   nv50_program_update_context_state(nv50, gp, 2);

   /* GP_ENABLE is updated in linkage validation */
}

void
nv50_compprog_validate(struct nv50_context *nv50)
{
   struct nouveau_pushbuf *push = nv50->base.pushbuf;
   struct nv50_program *cp = nv50->compprog;

   if (cp && !nv50_program_validate(nv50, cp))
      return;

   BEGIN_NV04(push, NV50_CP(CODE_CB_FLUSH), 1);
   PUSH_DATA (push, 0);
}

static void
nv50_sprite_coords_validate(struct nv50_context *nv50)
{
   struct nouveau_pushbuf *push = nv50->base.pushbuf;
   uint32_t pntc[8], mode;
   struct nv50_program *fp = nv50->fragprog;
   unsigned i, c;
   unsigned m = (nv50->state.interpolant_ctrl >> 8) & 0xff;

   if (!nv50->rast->pipe.point_quad_rasterization) {
      if (nv50->state.point_sprite) {
         BEGIN_NV04(push, NV50_3D(POINT_COORD_REPLACE_MAP(0)), 8);
         for (i = 0; i < 8; ++i)
            PUSH_DATA(push, 0);

         nv50->state.point_sprite = false;
      }
      return;
   } else {
      nv50->state.point_sprite = true;
   }

   memset(pntc, 0, sizeof(pntc));

   for (i = 0; i < fp->in_nr; i++) {
      unsigned n = util_bitcount(fp->in[i].mask);

      if (fp->in[i].sn != TGSI_SEMANTIC_GENERIC) {
         m += n;
         continue;
      }
      if (!(nv50->rast->pipe.sprite_coord_enable & (1 << fp->in[i].si))) {
         m += n;
         continue;
      }

      for (c = 0; c < 4; ++c) {
         if (fp->in[i].mask & (1 << c)) {
            pntc[m / 8] |= (c + 1) << ((m % 8) * 4);
            ++m;
         }
      }
   }

   if (nv50->rast->pipe.sprite_coord_mode == PIPE_SPRITE_COORD_LOWER_LEFT)
      mode = 0x00;
   else
      mode = 0x10;

   BEGIN_NV04(push, NV50_3D(POINT_SPRITE_CTRL), 1);
   PUSH_DATA (push, mode);

   BEGIN_NV04(push, NV50_3D(POINT_COORD_REPLACE_MAP(0)), 8);
   PUSH_DATAp(push, pntc, 8);
}

/* Validate state derived from shaders and the rasterizer cso. */
void
nv50_validate_derived_rs(struct nv50_context *nv50)
{
   struct nouveau_pushbuf *push = nv50->base.pushbuf;
   uint32_t color, psize;

   nv50_sprite_coords_validate(nv50);

   if (nv50->state.rasterizer_discard != nv50->rast->pipe.rasterizer_discard) {
      nv50->state.rasterizer_discard = nv50->rast->pipe.rasterizer_discard;
      BEGIN_NV04(push, NV50_3D(RASTERIZE_ENABLE), 1);
      PUSH_DATA (push, !nv50->rast->pipe.rasterizer_discard);
   }

   if (nv50->dirty_3d & NV50_NEW_3D_FRAGPROG)
      return;
   psize = nv50->state.semantic_psize & ~NV50_3D_SEMANTIC_PTSZ_PTSZ_EN__MASK;
   color = nv50->state.semantic_color & ~NV50_3D_SEMANTIC_COLOR_CLMP_EN;

   if (nv50->rast->pipe.clamp_vertex_color)
      color |= NV50_3D_SEMANTIC_COLOR_CLMP_EN;

   if (color != nv50->state.semantic_color) {
      nv50->state.semantic_color = color;
      BEGIN_NV04(push, NV50_3D(SEMANTIC_COLOR), 1);
      PUSH_DATA (push, color);
   }

   if (nv50->rast->pipe.point_size_per_vertex)
      psize |= NV50_3D_SEMANTIC_PTSZ_PTSZ_EN__MASK;

   if (psize != nv50->state.semantic_psize) {
      nv50->state.semantic_psize = psize;
      BEGIN_NV04(push, NV50_3D(SEMANTIC_PTSZ), 1);
      PUSH_DATA (push, psize);
   }
}

static int
nv50_vec4_map(uint8_t *map, int mid, uint32_t lin[4],
              struct nv50_varying *in, struct nv50_varying *out)
{
   int c;
   uint8_t mv = out->mask, mf = in->mask, oid = out->hw;

   for (c = 0; c < 4; ++c) {
      if (mf & 1) {
         if (in->linear)
            lin[mid / 32] |= 1 << (mid % 32);
         if (mv & 1)
            map[mid] = oid;
         else
         if (c == 3)
            map[mid] |= 1;
         ++mid;
      }

      oid += mv & 1;
      mf >>= 1;
      mv >>= 1;
   }

   return mid;
}

void
nv50_fp_linkage_validate(struct nv50_context *nv50)
{
   struct nouveau_pushbuf *push = nv50->base.pushbuf;
   struct nv50_program *vp = nv50->gmtyprog ? nv50->gmtyprog : nv50->vertprog;
   struct nv50_program *fp = nv50->fragprog;
   struct nv50_varying dummy;
   int i, n, c, m;
   uint32_t primid = 0;
   uint32_t layerid = 0;
   uint32_t viewportid = 0;
   uint32_t psiz = 0x000;
   uint32_t interp = fp->fp.interp;
   uint32_t colors = fp->fp.colors;
   uint32_t clpd_nr = util_last_bit(vp->vp.clip_enable | vp->vp.cull_enable);
   uint32_t lin[4];
   uint8_t map[64];
   uint8_t so_map[64];

   if (!(nv50->dirty_3d & (NV50_NEW_3D_VERTPROG |
                           NV50_NEW_3D_FRAGPROG |
                           NV50_NEW_3D_GMTYPROG))) {
      uint8_t bfc, ffc;
      ffc = (nv50->state.semantic_color & NV50_3D_SEMANTIC_COLOR_FFC0_ID__MASK);
      bfc = (nv50->state.semantic_color & NV50_3D_SEMANTIC_COLOR_BFC0_ID__MASK)
         >> 8;
      if (nv50->rast->pipe.light_twoside == ((ffc == bfc) ? 0 : 1))
         return;
   }

   memset(lin, 0x00, sizeof(lin));

   /* XXX: in buggy-endian mode, is the first element of map (u32)0x000000xx
    *  or is it the first byte ?
    */
   memset(map, nv50->gmtyprog ? 0x80 : 0x40, sizeof(map));

   dummy.mask = 0xf; /* map all components of HPOS */
   dummy.linear = 0;
   m = nv50_vec4_map(map, 0, lin, &dummy, &vp->out[0]);

   for (c = 0; c < clpd_nr; ++c)
      map[m++] = vp->vp.clpd[c / 4] + (c % 4);

   colors |= m << 8; /* adjust BFC0 id */

   dummy.mask = 0x0;

   /* if light_twoside is active, FFC0_ID == BFC0_ID is invalid */
   if (nv50->rast->pipe.light_twoside) {
      for (i = 0; i < 2; ++i) {
         n = vp->vp.bfc[i];
         if (fp->vp.bfc[i] >= fp->in_nr)
            continue;
         m = nv50_vec4_map(map, m, lin, &fp->in[fp->vp.bfc[i]],
                           (n < vp->out_nr) ? &vp->out[n] : &dummy);
      }
   }
   colors += m - 4; /* adjust FFC0 id */
   interp |= m << 8; /* set map id where 'normal' FP inputs start */

   for (i = 0; i < fp->in_nr; ++i) {
      for (n = 0; n < vp->out_nr; ++n)
         if (vp->out[n].sn == fp->in[i].sn &&
             vp->out[n].si == fp->in[i].si)
            break;
      switch (fp->in[i].sn) {
      case TGSI_SEMANTIC_PRIMID:
         primid = m;
         break;
      case TGSI_SEMANTIC_LAYER:
         layerid = m;
         break;
      case TGSI_SEMANTIC_VIEWPORT_INDEX:
         viewportid = m;
         break;
      }
      m = nv50_vec4_map(map, m, lin,
                        &fp->in[i], (n < vp->out_nr) ? &vp->out[n] : &dummy);
   }

   if (vp->gp.has_layer && !layerid) {
      layerid = m;
      map[m++] = vp->gp.layerid;
   }

   if (vp->gp.has_viewport && !viewportid) {
      viewportid = m;
      map[m++] = vp->gp.viewportid;
   }

   if (nv50->rast->pipe.point_size_per_vertex) {
      psiz = (m << 4) | 1;
      map[m++] = vp->vp.psiz;
   }

   if (nv50->rast->pipe.clamp_vertex_color)
      colors |= NV50_3D_SEMANTIC_COLOR_CLMP_EN;

   if (unlikely(vp->so)) {
      /* Slot i in STRMOUT_MAP specifies the offset where slot i in RESULT_MAP
       * gets written.
       *
       * TODO:
       * Inverting vp->so->map (output -> offset) would probably speed this up.
       */
      memset(so_map, 0, sizeof(so_map));
      for (i = 0; i < vp->so->map_size; ++i) {
         if (vp->so->map[i] == 0xff)
            continue;
         for (c = 0; c < m; ++c)
            if (map[c] == vp->so->map[i] && !so_map[c])
               break;
         if (c == m) {
            c = m;
            map[m++] = vp->so->map[i];
         }
         so_map[c] = 0x80 | i;
      }
      for (c = m; c & 3; ++c)
         so_map[c] = 0;
   }

   n = (m + 3) / 4;
   assert(m <= 64);

   if (unlikely(nv50->gmtyprog)) {
      BEGIN_NV04(push, NV50_3D(GP_RESULT_MAP_SIZE), 1);
      PUSH_DATA (push, m);
      BEGIN_NV04(push, NV50_3D(GP_RESULT_MAP(0)), n);
      PUSH_DATAp(push, map, n);
   } else {
      BEGIN_NV04(push, NV50_3D(VP_GP_BUILTIN_ATTR_EN), 1);
      PUSH_DATA (push, vp->vp.attrs[2] | fp->vp.attrs[2]);

      BEGIN_NV04(push, NV50_3D(SEMANTIC_PRIM_ID), 1);
      PUSH_DATA (push, primid);

      assert(m > 0);
      BEGIN_NV04(push, NV50_3D(VP_RESULT_MAP_SIZE), 1);
      PUSH_DATA (push, m);
      BEGIN_NV04(push, NV50_3D(VP_RESULT_MAP(0)), n);
      PUSH_DATAp(push, map, n);
   }

   BEGIN_NV04(push, NV50_3D(GP_VIEWPORT_ID_ENABLE), 5);
   PUSH_DATA (push, vp->gp.has_viewport);
   PUSH_DATA (push, colors);
   PUSH_DATA (push, (clpd_nr << 8) | 4);
   PUSH_DATA (push, layerid);
   PUSH_DATA (push, psiz);

   BEGIN_NV04(push, NV50_3D(SEMANTIC_VIEWPORT), 1);
   PUSH_DATA (push, viewportid);

   BEGIN_NV04(push, NV50_3D(LAYER), 1);
   PUSH_DATA (push, vp->gp.has_layer << 16);

   BEGIN_NV04(push, NV50_3D(FP_INTERPOLANT_CTRL), 1);
   PUSH_DATA (push, interp);

   nv50->state.interpolant_ctrl = interp;

   nv50->state.semantic_color = colors;
   nv50->state.semantic_psize = psiz;

   BEGIN_NV04(push, NV50_3D(NOPERSPECTIVE_BITMAP(0)), 4);
   PUSH_DATAp(push, lin, 4);

   BEGIN_NV04(push, NV50_3D(GP_ENABLE), 1);
   PUSH_DATA (push, nv50->gmtyprog ? 1 : 0);

   if (vp->so) {
      BEGIN_NV04(push, NV50_3D(STRMOUT_MAP(0)), n);
      PUSH_DATAp(push, so_map, n);
   }
}

static int
nv50_vp_gp_mapping(uint8_t *map, int m,
                   struct nv50_program *vp, struct nv50_program *gp)
{
   int i, j, c;

   for (i = 0; i < gp->in_nr; ++i) {
      uint8_t oid = 0, mv = 0, mg = gp->in[i].mask;

      for (j = 0; j < vp->out_nr; ++j) {
         if (vp->out[j].sn == gp->in[i].sn &&
             vp->out[j].si == gp->in[i].si) {
            mv = vp->out[j].mask;
            oid = vp->out[j].hw;
            break;
         }
      }

      for (c = 0; c < 4; ++c, mv >>= 1, mg >>= 1) {
         if (mg & mv & 1)
            map[m++] = oid;
         else
         if (mg & 1)
            map[m++] = (c == 3) ? 0x41 : 0x40;
         oid += mv & 1;
      }
   }
   if (!m)
      map[m++] = 0;
   return m;
}

void
nv50_gp_linkage_validate(struct nv50_context *nv50)
{
   struct nouveau_pushbuf *push = nv50->base.pushbuf;
   struct nv50_program *vp = nv50->vertprog;
   struct nv50_program *gp = nv50->gmtyprog;
   int m = 0;
   int n;
   uint8_t map[64];

   if (!gp)
      return;
   memset(map, 0, sizeof(map));

   m = nv50_vp_gp_mapping(map, m, vp, gp);

   n = (m + 3) / 4;

   BEGIN_NV04(push, NV50_3D(VP_GP_BUILTIN_ATTR_EN), 1);
   PUSH_DATA (push, vp->vp.attrs[2] | gp->vp.attrs[2]);

   assert(m > 0);
   BEGIN_NV04(push, NV50_3D(VP_RESULT_MAP_SIZE), 1);
   PUSH_DATA (push, m);
   BEGIN_NV04(push, NV50_3D(VP_RESULT_MAP(0)), n);
   PUSH_DATAp(push, map, n);
}

void
nv50_stream_output_validate(struct nv50_context *nv50)
{
   struct nouveau_pushbuf *push = nv50->base.pushbuf;
   struct nv50_stream_output_state *so;
   uint32_t ctrl;
   unsigned i;
   unsigned prims = ~0;

   so = nv50->gmtyprog ? nv50->gmtyprog->so : nv50->vertprog->so;

   BEGIN_NV04(push, NV50_3D(STRMOUT_ENABLE), 1);
   PUSH_DATA (push, 0);
   if (!so || !nv50->num_so_targets) {
      if (nv50->screen->base.class_3d < NVA0_3D_CLASS) {
         BEGIN_NV04(push, NV50_3D(STRMOUT_PRIMITIVE_LIMIT), 1);
         PUSH_DATA (push, 0);
      }
      BEGIN_NV04(push, NV50_3D(STRMOUT_PARAMS_LATCH), 1);
      PUSH_DATA (push, 1);
      return;
   }

   /* previous TFB needs to complete */
   if (nv50->screen->base.class_3d < NVA0_3D_CLASS) {
      BEGIN_NV04(push, SUBC_3D(NV50_GRAPH_SERIALIZE), 1);
      PUSH_DATA (push, 0);
   }

   ctrl = so->ctrl;
   if (nv50->screen->base.class_3d >= NVA0_3D_CLASS)
      ctrl |= NVA0_3D_STRMOUT_BUFFERS_CTRL_LIMIT_MODE_OFFSET;

   BEGIN_NV04(push, NV50_3D(STRMOUT_BUFFERS_CTRL), 1);
   PUSH_DATA (push, ctrl);

   for (i = 0; i < nv50->num_so_targets; ++i) {
      struct nv50_so_target *targ = nv50_so_target(nv50->so_target[i]);
      struct nv04_resource *buf = nv04_resource(targ->pipe.buffer);

      const unsigned n = nv50->screen->base.class_3d >= NVA0_3D_CLASS ? 4 : 3;

      uint32_t so_used = 0;

      if (!targ->clean) {
         if (n == 4)
            nv84_hw_query_fifo_wait(push, nv50_query(targ->pq));
         else
            so_used = nv50->so_used[i];
      }
      BEGIN_NV04(push, NV50_3D(STRMOUT_ADDRESS_HIGH(i)), n);
      PUSH_DATAh(push, buf->address + targ->pipe.buffer_offset + so_used);
      PUSH_DATA (push, buf->address + targ->pipe.buffer_offset + so_used);
      PUSH_DATA (push, so->num_attribs[i]);
      if (n == 4) {
         PUSH_DATA(push, targ->pipe.buffer_size);
         if (!targ->clean) {
            assert(targ->pq);
            nv50_hw_query_pushbuf_submit(nv50, NVA0_3D_STRMOUT_OFFSET(i),
                                         nv50_query(targ->pq), 0x4);
         } else {
            BEGIN_NV04(push, NVA0_3D(STRMOUT_OFFSET(i)), 1);
            PUSH_DATA(push, 0);
            targ->clean = false;
         }
      } else {
         const unsigned limit = (targ->pipe.buffer_size - so_used) /
            (so->stride[i] * nv50->state.prim_size);
         prims = MIN2(prims, limit);
         targ->clean = false;
      }
      targ->stride = so->stride[i];
      BCTX_REFN(nv50->bufctx_3d, 3D_SO, buf, WR);
   }
   if (prims != ~0) {
      BEGIN_NV04(push, NV50_3D(STRMOUT_PRIMITIVE_LIMIT), 1);
      PUSH_DATA (push, prims);
   }
   BEGIN_NV04(push, NV50_3D(STRMOUT_PARAMS_LATCH), 1);
   PUSH_DATA (push, 1);
   BEGIN_NV04(push, NV50_3D(STRMOUT_ENABLE), 1);
   PUSH_DATA (push, 1);
}
