/*
 * Copyright (C) 2014 Rob Clark <robclark@freedesktop.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Authors:
 *    Rob Clark <robclark@freedesktop.org>
 */

#include "pipe/p_state.h"
#include "util/format/u_format.h"
#include "util/u_inlines.h"
#include "util/u_memory.h"
#include "util/u_string.h"

#include "freedreno_program.h"

#include "fd4_emit.h"
#include "fd4_format.h"
#include "fd4_program.h"
#include "fd4_texture.h"

void
fd4_emit_shader(struct fd_ringbuffer *ring, const struct ir3_shader_variant *so)
{
   const struct ir3_info *si = &so->info;
   enum a4xx_state_block sb = fd4_stage2shadersb(so->type);
   enum a4xx_state_src src;
   uint32_t i, sz, *bin;

   if (FD_DBG(DIRECT)) {
      sz = si->sizedwords;
      src = SS4_DIRECT;
      bin = fd_bo_map(so->bo);
   } else {
      sz = 0;
      src = SS4_INDIRECT;
      bin = NULL;
   }

   OUT_PKT3(ring, CP_LOAD_STATE4, 2 + sz);
   OUT_RING(ring, CP_LOAD_STATE4_0_DST_OFF(0) |
                     CP_LOAD_STATE4_0_STATE_SRC(src) |
                     CP_LOAD_STATE4_0_STATE_BLOCK(sb) |
                     CP_LOAD_STATE4_0_NUM_UNIT(so->instrlen));
   if (bin) {
      OUT_RING(ring, CP_LOAD_STATE4_1_EXT_SRC_ADDR(0) |
                        CP_LOAD_STATE4_1_STATE_TYPE(ST4_SHADER));
   } else {
      OUT_RELOC(ring, so->bo, 0, CP_LOAD_STATE4_1_STATE_TYPE(ST4_SHADER), 0);
   }

   /* for how clever coverity is, it is sometimes rather dull, and
    * doesn't realize that the only case where bin==NULL, sz==0:
    */
   assume(bin || (sz == 0));

   for (i = 0; i < sz; i++) {
      OUT_RING(ring, bin[i]);
   }
}

struct stage {
   const struct ir3_shader_variant *v;
   const struct ir3_info *i;
   /* const sizes are in units of 4 * vec4 */
   uint8_t constoff;
   uint8_t constlen;
   /* instr sizes are in units of 16 instructions */
   uint8_t instroff;
   uint8_t instrlen;
};

enum { VS = 0, FS = 1, HS = 2, DS = 3, GS = 4, MAX_STAGES };

static void
setup_stages(struct fd4_emit *emit, struct stage *s)
{
   unsigned i;

   s[VS].v = fd4_emit_get_vp(emit);
   s[FS].v = fd4_emit_get_fp(emit);

   s[HS].v = s[DS].v = s[GS].v = NULL; /* for now */

   for (i = 0; i < MAX_STAGES; i++) {
      if (s[i].v) {
         s[i].i = &s[i].v->info;
         /* constlen is in units of 4 * vec4: */
         assert(s[i].v->constlen % 4 == 0);
         s[i].constlen = s[i].v->constlen / 4;
         /* instrlen is already in units of 16 instr.. although
          * probably we should ditch that and not make the compiler
          * care about instruction group size of a3xx vs a4xx
          */
         s[i].instrlen = s[i].v->instrlen;
      } else {
         s[i].i = NULL;
         s[i].constlen = 0;
         s[i].instrlen = 0;
      }
   }

   /* NOTE: at least for gles2, blob partitions VS at bottom of const
    * space and FS taking entire remaining space.  We probably don't
    * need to do that the same way, but for now mimic what the blob
    * does to make it easier to diff against register values from blob
    *
    * NOTE: if VS.instrlen + FS.instrlen > 64, then one or both shaders
    * is run from external memory.
    */
   if ((s[VS].instrlen + s[FS].instrlen) > 64) {
      /* prioritize FS for internal memory: */
      if (s[FS].instrlen < 64) {
         /* if FS can fit, kick VS out to external memory: */
         s[VS].instrlen = 0;
      } else if (s[VS].instrlen < 64) {
         /* otherwise if VS can fit, kick out FS: */
         s[FS].instrlen = 0;
      } else {
         /* neither can fit, run both from external memory: */
         s[VS].instrlen = 0;
         s[FS].instrlen = 0;
      }
   }
   s[VS].constlen = 66;
   s[FS].constlen = 128 - s[VS].constlen;
   s[VS].instroff = 0;
   s[VS].constoff = 0;
   s[FS].instroff = 64 - s[FS].instrlen;
   s[FS].constoff = s[VS].constlen;
   s[HS].instroff = s[DS].instroff = s[GS].instroff = s[FS].instroff;
   s[HS].constoff = s[DS].constoff = s[GS].constoff = s[FS].constoff;
}

void
fd4_program_emit(struct fd_ringbuffer *ring, struct fd4_emit *emit, int nr,
                 struct pipe_surface **bufs)
{
   struct stage s[MAX_STAGES];
   uint32_t pos_regid, posz_regid, psize_regid, color_regid[8];
   uint32_t face_regid, coord_regid, zwcoord_regid, samp_id_regid,
      samp_mask_regid, ij_regid[IJ_COUNT];
   enum a3xx_threadsize fssz;
   int constmode;
   int i, j;

   assert(nr <= ARRAY_SIZE(color_regid));

   if (emit->binning_pass)
      nr = 0;

   setup_stages(emit, s);

   fssz = (s[FS].i->double_threadsize) ? FOUR_QUADS : TWO_QUADS;

   /* blob seems to always use constmode currently: */
   constmode = 1;

   pos_regid = ir3_find_output_regid(s[VS].v, VARYING_SLOT_POS);
   if (pos_regid == regid(63, 0)) {
      /* hw dislikes when there is no position output, which can
       * happen for transform-feedback vertex shaders.  Just tell
       * the hw to use r0.x, with whatever random value is there:
       */
      pos_regid = regid(0, 0);
   }
   posz_regid = ir3_find_output_regid(s[FS].v, FRAG_RESULT_DEPTH);
   psize_regid = ir3_find_output_regid(s[VS].v, VARYING_SLOT_PSIZ);
   if (s[FS].v->color0_mrt) {
      color_regid[0] = color_regid[1] = color_regid[2] = color_regid[3] =
         color_regid[4] = color_regid[5] = color_regid[6] = color_regid[7] =
            ir3_find_output_regid(s[FS].v, FRAG_RESULT_COLOR);
   } else {
      color_regid[0] = ir3_find_output_regid(s[FS].v, FRAG_RESULT_DATA0);
      color_regid[1] = ir3_find_output_regid(s[FS].v, FRAG_RESULT_DATA1);
      color_regid[2] = ir3_find_output_regid(s[FS].v, FRAG_RESULT_DATA2);
      color_regid[3] = ir3_find_output_regid(s[FS].v, FRAG_RESULT_DATA3);
      color_regid[4] = ir3_find_output_regid(s[FS].v, FRAG_RESULT_DATA4);
      color_regid[5] = ir3_find_output_regid(s[FS].v, FRAG_RESULT_DATA5);
      color_regid[6] = ir3_find_output_regid(s[FS].v, FRAG_RESULT_DATA6);
      color_regid[7] = ir3_find_output_regid(s[FS].v, FRAG_RESULT_DATA7);
   }

   samp_id_regid = ir3_find_sysval_regid(s[FS].v, SYSTEM_VALUE_SAMPLE_ID);
   samp_mask_regid =
      ir3_find_sysval_regid(s[FS].v, SYSTEM_VALUE_SAMPLE_MASK_IN);
   face_regid = ir3_find_sysval_regid(s[FS].v, SYSTEM_VALUE_FRONT_FACE);
   coord_regid = ir3_find_sysval_regid(s[FS].v, SYSTEM_VALUE_FRAG_COORD);
   zwcoord_regid =
      (coord_regid == regid(63, 0)) ? regid(63, 0) : (coord_regid + 2);
   for (unsigned i = 0; i < ARRAY_SIZE(ij_regid); i++)
      ij_regid[i] = ir3_find_sysval_regid(
         s[FS].v, SYSTEM_VALUE_BARYCENTRIC_PERSP_PIXEL + i);

   /* we could probably divide this up into things that need to be
    * emitted if frag-prog is dirty vs if vert-prog is dirty..
    */

   OUT_PKT0(ring, REG_A4XX_HLSQ_UPDATE_CONTROL, 1);
   OUT_RING(ring, 0x00000003);

   OUT_PKT0(ring, REG_A4XX_HLSQ_CONTROL_0_REG, 5);
   OUT_RING(ring, A4XX_HLSQ_CONTROL_0_REG_FSTHREADSIZE(fssz) |
                     A4XX_HLSQ_CONTROL_0_REG_CONSTMODE(constmode) |
                     A4XX_HLSQ_CONTROL_0_REG_FSSUPERTHREADENABLE |
                     /* NOTE:  I guess SHADERRESTART and CONSTFULLUPDATE maybe
                      * flush some caches? I think we only need to set those
                      * bits if we have updated const or shader..
                      */
                     A4XX_HLSQ_CONTROL_0_REG_SPSHADERRESTART |
                     A4XX_HLSQ_CONTROL_0_REG_SPCONSTFULLUPDATE);
   OUT_RING(ring, A4XX_HLSQ_CONTROL_1_REG_VSTHREADSIZE(TWO_QUADS) |
                     A4XX_HLSQ_CONTROL_1_REG_VSSUPERTHREADENABLE |
                     A4XX_HLSQ_CONTROL_1_REG_COORDREGID(coord_regid) |
                     A4XX_HLSQ_CONTROL_1_REG_ZWCOORDREGID(zwcoord_regid));
   OUT_RING(ring, A4XX_HLSQ_CONTROL_2_REG_PRIMALLOCTHRESHOLD(63) |
                     A4XX_HLSQ_CONTROL_2_REG_SAMPLEID_REGID(samp_id_regid) |
                     A4XX_HLSQ_CONTROL_2_REG_SAMPLEMASK_REGID(samp_mask_regid) |
                     A4XX_HLSQ_CONTROL_2_REG_FACEREGID(face_regid));
   /* XXX left out centroid/sample for now */
   OUT_RING(
      ring,
      A4XX_HLSQ_CONTROL_3_REG_IJ_PERSP_PIXEL(ij_regid[IJ_PERSP_PIXEL]) |
         A4XX_HLSQ_CONTROL_3_REG_IJ_LINEAR_PIXEL(ij_regid[IJ_LINEAR_PIXEL]) |
         A4XX_HLSQ_CONTROL_3_REG_IJ_PERSP_CENTROID(
            ij_regid[IJ_PERSP_CENTROID]) |
         A4XX_HLSQ_CONTROL_3_REG_IJ_LINEAR_CENTROID(
            ij_regid[IJ_LINEAR_CENTROID]));
   OUT_RING(ring, 0x00fcfcfc); /* XXX HLSQ_CONTROL_4 */

   OUT_PKT0(ring, REG_A4XX_HLSQ_VS_CONTROL_REG, 5);
   OUT_RING(ring,
            A4XX_HLSQ_VS_CONTROL_REG_CONSTLENGTH(s[VS].constlen) |
               A4XX_HLSQ_VS_CONTROL_REG_CONSTOBJECTOFFSET(s[VS].constoff) |
               COND(s[VS].v && s[VS].v->has_ssbo, A4XX_HLSQ_VS_CONTROL_REG_SSBO_ENABLE) |
               COND(s[VS].v, A4XX_HLSQ_VS_CONTROL_REG_ENABLED) |
               A4XX_HLSQ_VS_CONTROL_REG_INSTRLENGTH(s[VS].instrlen) |
               A4XX_HLSQ_VS_CONTROL_REG_SHADEROBJOFFSET(s[VS].instroff));
   OUT_RING(ring,
            A4XX_HLSQ_FS_CONTROL_REG_CONSTLENGTH(s[FS].constlen) |
               A4XX_HLSQ_FS_CONTROL_REG_CONSTOBJECTOFFSET(s[FS].constoff) |
               COND(s[FS].v && s[FS].v->has_ssbo, A4XX_HLSQ_FS_CONTROL_REG_SSBO_ENABLE) |
               COND(s[FS].v, A4XX_HLSQ_FS_CONTROL_REG_ENABLED) |
               A4XX_HLSQ_FS_CONTROL_REG_INSTRLENGTH(s[FS].instrlen) |
               A4XX_HLSQ_FS_CONTROL_REG_SHADEROBJOFFSET(s[FS].instroff));
   OUT_RING(ring,
            A4XX_HLSQ_HS_CONTROL_REG_CONSTLENGTH(s[HS].constlen) |
               A4XX_HLSQ_HS_CONTROL_REG_CONSTOBJECTOFFSET(s[HS].constoff) |
               COND(s[HS].v && s[HS].v->has_ssbo, A4XX_HLSQ_HS_CONTROL_REG_SSBO_ENABLE) |
               A4XX_HLSQ_HS_CONTROL_REG_INSTRLENGTH(s[HS].instrlen) |
               A4XX_HLSQ_HS_CONTROL_REG_SHADEROBJOFFSET(s[HS].instroff));
   OUT_RING(ring,
            A4XX_HLSQ_DS_CONTROL_REG_CONSTLENGTH(s[DS].constlen) |
               A4XX_HLSQ_DS_CONTROL_REG_CONSTOBJECTOFFSET(s[DS].constoff) |
               COND(s[DS].v && s[DS].v->has_ssbo, A4XX_HLSQ_DS_CONTROL_REG_SSBO_ENABLE) |
               A4XX_HLSQ_DS_CONTROL_REG_INSTRLENGTH(s[DS].instrlen) |
               A4XX_HLSQ_DS_CONTROL_REG_SHADEROBJOFFSET(s[DS].instroff));
   OUT_RING(ring,
            A4XX_HLSQ_GS_CONTROL_REG_CONSTLENGTH(s[GS].constlen) |
               A4XX_HLSQ_GS_CONTROL_REG_CONSTOBJECTOFFSET(s[GS].constoff) |
               COND(s[GS].v && s[GS].v->has_ssbo, A4XX_HLSQ_GS_CONTROL_REG_SSBO_ENABLE) |
               A4XX_HLSQ_GS_CONTROL_REG_INSTRLENGTH(s[GS].instrlen) |
               A4XX_HLSQ_GS_CONTROL_REG_SHADEROBJOFFSET(s[GS].instroff));

   OUT_PKT0(ring, REG_A4XX_SP_SP_CTRL_REG, 1);
   OUT_RING(ring,
            0x140010 | /* XXX */
               COND(emit->binning_pass, A4XX_SP_SP_CTRL_REG_BINNING_PASS));

   OUT_PKT0(ring, REG_A4XX_SP_INSTR_CACHE_CTRL, 1);
   OUT_RING(ring, 0x7f | /* XXX */
                     COND(s[VS].instrlen, A4XX_SP_INSTR_CACHE_CTRL_VS_BUFFER) |
                     COND(s[FS].instrlen, A4XX_SP_INSTR_CACHE_CTRL_FS_BUFFER) |
                     COND(s[VS].instrlen && s[FS].instrlen,
                          A4XX_SP_INSTR_CACHE_CTRL_INSTR_BUFFER));

   OUT_PKT0(ring, REG_A4XX_SP_VS_LENGTH_REG, 1);
   OUT_RING(ring, s[VS].v->instrlen); /* SP_VS_LENGTH_REG */

   OUT_PKT0(ring, REG_A4XX_SP_VS_CTRL_REG0, 3);
   OUT_RING(
      ring,
      A4XX_SP_VS_CTRL_REG0_THREADMODE(MULTI) |
         A4XX_SP_VS_CTRL_REG0_HALFREGFOOTPRINT(s[VS].i->max_half_reg + 1) |
         A4XX_SP_VS_CTRL_REG0_FULLREGFOOTPRINT(s[VS].i->max_reg + 1) |
         A4XX_SP_VS_CTRL_REG0_INOUTREGOVERLAP(0) |
         A4XX_SP_VS_CTRL_REG0_THREADSIZE(TWO_QUADS) |
         A4XX_SP_VS_CTRL_REG0_SUPERTHREADMODE |
         COND(s[VS].v->need_pixlod, A4XX_SP_VS_CTRL_REG0_PIXLODENABLE));
   OUT_RING(ring,
            A4XX_SP_VS_CTRL_REG1_CONSTLENGTH(s[VS].constlen) |
               A4XX_SP_VS_CTRL_REG1_INITIALOUTSTANDING(s[VS].v->total_in));
   OUT_RING(ring, A4XX_SP_VS_PARAM_REG_POSREGID(pos_regid) |
                     A4XX_SP_VS_PARAM_REG_PSIZEREGID(psize_regid) |
                     A4XX_SP_VS_PARAM_REG_TOTALVSOUTVAR(s[FS].v->varying_in));

   struct ir3_shader_linkage l = {0};
   ir3_link_shaders(&l, s[VS].v, s[FS].v, false);

   for (i = 0, j = 0; (i < 16) && (j < l.cnt); i++) {
      uint32_t reg = 0;

      OUT_PKT0(ring, REG_A4XX_SP_VS_OUT_REG(i), 1);

      reg |= A4XX_SP_VS_OUT_REG_A_REGID(l.var[j].regid);
      reg |= A4XX_SP_VS_OUT_REG_A_COMPMASK(l.var[j].compmask);
      j++;

      reg |= A4XX_SP_VS_OUT_REG_B_REGID(l.var[j].regid);
      reg |= A4XX_SP_VS_OUT_REG_B_COMPMASK(l.var[j].compmask);
      j++;

      OUT_RING(ring, reg);
   }

   for (i = 0, j = 0; (i < 8) && (j < l.cnt); i++) {
      uint32_t reg = 0;

      OUT_PKT0(ring, REG_A4XX_SP_VS_VPC_DST_REG(i), 1);

      reg |= A4XX_SP_VS_VPC_DST_REG_OUTLOC0(l.var[j++].loc + 8);
      reg |= A4XX_SP_VS_VPC_DST_REG_OUTLOC1(l.var[j++].loc + 8);
      reg |= A4XX_SP_VS_VPC_DST_REG_OUTLOC2(l.var[j++].loc + 8);
      reg |= A4XX_SP_VS_VPC_DST_REG_OUTLOC3(l.var[j++].loc + 8);

      OUT_RING(ring, reg);
   }

   OUT_PKT0(ring, REG_A4XX_SP_VS_OBJ_OFFSET_REG, 2);
   OUT_RING(ring, A4XX_SP_VS_OBJ_OFFSET_REG_CONSTOBJECTOFFSET(s[VS].constoff) |
                     A4XX_SP_VS_OBJ_OFFSET_REG_SHADEROBJOFFSET(s[VS].instroff));
   OUT_RELOC(ring, s[VS].v->bo, 0, 0, 0); /* SP_VS_OBJ_START_REG */

   if (emit->binning_pass) {
      OUT_PKT0(ring, REG_A4XX_SP_FS_LENGTH_REG, 1);
      OUT_RING(ring, 0x00000000); /* SP_FS_LENGTH_REG */

      OUT_PKT0(ring, REG_A4XX_SP_FS_CTRL_REG0, 2);
      OUT_RING(ring,
               A4XX_SP_FS_CTRL_REG0_THREADMODE(MULTI) |
                  COND(s[FS].v->total_in > 0, A4XX_SP_FS_CTRL_REG0_VARYING) |
                  A4XX_SP_FS_CTRL_REG0_HALFREGFOOTPRINT(0) |
                  A4XX_SP_FS_CTRL_REG0_FULLREGFOOTPRINT(0) |
                  A4XX_SP_FS_CTRL_REG0_INOUTREGOVERLAP(1) |
                  A4XX_SP_FS_CTRL_REG0_THREADSIZE(fssz) |
                  A4XX_SP_FS_CTRL_REG0_SUPERTHREADMODE);
      OUT_RING(ring,
               A4XX_SP_FS_CTRL_REG1_CONSTLENGTH(s[FS].constlen) | 0x80000000);

      OUT_PKT0(ring, REG_A4XX_SP_FS_OBJ_OFFSET_REG, 2);
      OUT_RING(ring,
               A4XX_SP_FS_OBJ_OFFSET_REG_CONSTOBJECTOFFSET(s[FS].constoff) |
                  A4XX_SP_FS_OBJ_OFFSET_REG_SHADEROBJOFFSET(s[FS].instroff));
      OUT_RING(ring, 0x00000000);
   } else {
      OUT_PKT0(ring, REG_A4XX_SP_FS_LENGTH_REG, 1);
      OUT_RING(ring, s[FS].v->instrlen); /* SP_FS_LENGTH_REG */

      OUT_PKT0(ring, REG_A4XX_SP_FS_CTRL_REG0, 2);
      OUT_RING(
         ring,
         A4XX_SP_FS_CTRL_REG0_THREADMODE(MULTI) |
            COND(s[FS].v->total_in > 0, A4XX_SP_FS_CTRL_REG0_VARYING) |
            A4XX_SP_FS_CTRL_REG0_HALFREGFOOTPRINT(s[FS].i->max_half_reg + 1) |
            A4XX_SP_FS_CTRL_REG0_FULLREGFOOTPRINT(s[FS].i->max_reg + 1) |
            A4XX_SP_FS_CTRL_REG0_INOUTREGOVERLAP(1) |
            A4XX_SP_FS_CTRL_REG0_THREADSIZE(fssz) |
            A4XX_SP_FS_CTRL_REG0_SUPERTHREADMODE |
            COND(s[FS].v->need_pixlod, A4XX_SP_FS_CTRL_REG0_PIXLODENABLE));
      OUT_RING(ring,
               A4XX_SP_FS_CTRL_REG1_CONSTLENGTH(s[FS].constlen) |
                  0x80000000 | /* XXX */
                  COND(s[FS].v->frag_face, A4XX_SP_FS_CTRL_REG1_FACENESS) |
                  COND(s[FS].v->total_in > 0, A4XX_SP_FS_CTRL_REG1_VARYING) |
                  COND(s[FS].v->fragcoord_compmask != 0,
                       A4XX_SP_FS_CTRL_REG1_FRAGCOORD));

      OUT_PKT0(ring, REG_A4XX_SP_FS_OBJ_OFFSET_REG, 2);
      OUT_RING(ring,
               A4XX_SP_FS_OBJ_OFFSET_REG_CONSTOBJECTOFFSET(s[FS].constoff) |
                  A4XX_SP_FS_OBJ_OFFSET_REG_SHADEROBJOFFSET(s[FS].instroff));
      OUT_RELOC(ring, s[FS].v->bo, 0, 0, 0); /* SP_FS_OBJ_START_REG */
   }

   OUT_PKT0(ring, REG_A4XX_SP_HS_OBJ_OFFSET_REG, 1);
   OUT_RING(ring, A4XX_SP_HS_OBJ_OFFSET_REG_CONSTOBJECTOFFSET(s[HS].constoff) |
                     A4XX_SP_HS_OBJ_OFFSET_REG_SHADEROBJOFFSET(s[HS].instroff));

   OUT_PKT0(ring, REG_A4XX_SP_DS_OBJ_OFFSET_REG, 1);
   OUT_RING(ring, A4XX_SP_DS_OBJ_OFFSET_REG_CONSTOBJECTOFFSET(s[DS].constoff) |
                     A4XX_SP_DS_OBJ_OFFSET_REG_SHADEROBJOFFSET(s[DS].instroff));

   OUT_PKT0(ring, REG_A4XX_SP_GS_OBJ_OFFSET_REG, 1);
   OUT_RING(ring, A4XX_SP_GS_OBJ_OFFSET_REG_CONSTOBJECTOFFSET(s[GS].constoff) |
                     A4XX_SP_GS_OBJ_OFFSET_REG_SHADEROBJOFFSET(s[GS].instroff));

   OUT_PKT0(ring, REG_A4XX_GRAS_CNTL, 1);
   OUT_RING(ring,
            CONDREG(face_regid, A4XX_GRAS_CNTL_IJ_PERSP) |
               CONDREG(zwcoord_regid, A4XX_GRAS_CNTL_IJ_PERSP) |
               CONDREG(ij_regid[IJ_PERSP_PIXEL], A4XX_GRAS_CNTL_IJ_PERSP) |
               CONDREG(ij_regid[IJ_LINEAR_PIXEL], A4XX_GRAS_CNTL_IJ_LINEAR) |
               CONDREG(ij_regid[IJ_PERSP_CENTROID], A4XX_GRAS_CNTL_IJ_PERSP));

   OUT_PKT0(ring, REG_A4XX_RB_RENDER_CONTROL2, 1);
   OUT_RING(
      ring,
      A4XX_RB_RENDER_CONTROL2_MSAA_SAMPLES(0) |
         CONDREG(ij_regid[IJ_PERSP_PIXEL],
                 A4XX_RB_RENDER_CONTROL2_IJ_PERSP_PIXEL) |
         CONDREG(ij_regid[IJ_PERSP_CENTROID],
                 A4XX_RB_RENDER_CONTROL2_IJ_PERSP_CENTROID) |
         CONDREG(ij_regid[IJ_LINEAR_PIXEL], A4XX_RB_RENDER_CONTROL2_SIZE) |
         CONDREG(samp_id_regid, A4XX_RB_RENDER_CONTROL2_SAMPLEID) |
         COND(s[FS].v->frag_face, A4XX_RB_RENDER_CONTROL2_FACENESS) |
         CONDREG(samp_mask_regid, A4XX_RB_RENDER_CONTROL2_SAMPLEMASK) |
         COND(s[FS].v->fragcoord_compmask != 0,
              A4XX_RB_RENDER_CONTROL2_COORD_MASK(s[FS].v->fragcoord_compmask)));

   OUT_PKT0(ring, REG_A4XX_RB_FS_OUTPUT_REG, 1);
   OUT_RING(ring,
            A4XX_RB_FS_OUTPUT_REG_MRT(nr) |
               COND(s[FS].v->writes_pos, A4XX_RB_FS_OUTPUT_REG_FRAG_WRITES_Z));

   OUT_PKT0(ring, REG_A4XX_SP_FS_OUTPUT_REG, 1);
   OUT_RING(ring,
            A4XX_SP_FS_OUTPUT_REG_MRT(nr) |
               COND(s[FS].v->writes_pos, A4XX_SP_FS_OUTPUT_REG_DEPTH_ENABLE) |
               A4XX_SP_FS_OUTPUT_REG_DEPTH_REGID(posz_regid));

   OUT_PKT0(ring, REG_A4XX_SP_FS_MRT_REG(0), 8);
   for (i = 0; i < 8; i++) {
      enum a4xx_color_fmt format = 0;
      bool srgb = false;
      bool uint = false;
      bool sint = false;
      if (i < nr) {
         format = fd4_emit_format(bufs[i]);
         if (bufs[i]) {
            if (!emit->no_decode_srgb)
               srgb = util_format_is_srgb(bufs[i]->format);
            uint = util_format_is_pure_uint(bufs[i]->format);
            sint = util_format_is_pure_sint(bufs[i]->format);
         }
      }
      OUT_RING(ring, A4XX_SP_FS_MRT_REG_REGID(color_regid[i]) |
                        A4XX_SP_FS_MRT_REG_MRTFORMAT(format) |
                        COND(srgb, A4XX_SP_FS_MRT_REG_COLOR_SRGB) |
                        COND(uint, A4XX_SP_FS_MRT_REG_COLOR_UINT) |
                        COND(sint, A4XX_SP_FS_MRT_REG_COLOR_SINT) |
                        COND(color_regid[i] & HALF_REG_ID,
                             A4XX_SP_FS_MRT_REG_HALF_PRECISION));
   }

   if (emit->binning_pass) {
      OUT_PKT0(ring, REG_A4XX_VPC_ATTR, 2);
      OUT_RING(ring, A4XX_VPC_ATTR_THRDASSIGN(1) | 0x40000000 | /* XXX */
                        COND(s[VS].v->writes_psize, A4XX_VPC_ATTR_PSIZE));
      OUT_RING(ring, 0x00000000);
   } else {
      uint32_t vinterp[8], vpsrepl[8];

      memset(vinterp, 0, sizeof(vinterp));
      memset(vpsrepl, 0, sizeof(vpsrepl));

      /* looks like we need to do int varyings in the frag
       * shader on a4xx (no flatshad reg?  or a420.0 bug?):
       *
       *    (sy)(ss)nop
       *    (sy)ldlv.u32 r0.x,l[r0.x], 1
       *    ldlv.u32 r0.y,l[r0.x+1], 1
       *    (ss)bary.f (ei)r63.x, 0, r0.x
       *    (ss)(rpt1)cov.s32f16 hr0.x, (r)r0.x
       *    (rpt5)nop
       *    sam (f16)(xyzw)hr0.x, hr0.x, s#0, t#0
       *
       * Possibly on later a4xx variants we'll be able to use
       * something like the code below instead of workaround
       * in the shader:
       */
      /* figure out VARYING_INTERP / VARYING_PS_REPL register values: */
      for (j = -1;
           (j = ir3_next_varying(s[FS].v, j)) < (int)s[FS].v->inputs_count;) {
         /* NOTE: varyings are packed, so if compmask is 0xb
          * then first, third, and fourth component occupy
          * three consecutive varying slots:
          */
         unsigned compmask = s[FS].v->inputs[j].compmask;

         uint32_t inloc = s[FS].v->inputs[j].inloc;

         if (s[FS].v->inputs[j].flat ||
             (s[FS].v->inputs[j].rasterflat && emit->rasterflat)) {
            uint32_t loc = inloc;

            for (i = 0; i < 4; i++) {
               if (compmask & (1 << i)) {
                  vinterp[loc / 16] |= 1 << ((loc % 16) * 2);
                  // flatshade[loc / 32] |= 1 << (loc % 32);
                  loc++;
               }
            }
         }

         bool coord_mode = emit->sprite_coord_mode;
         if (ir3_point_sprite(s[FS].v, j, emit->sprite_coord_enable,
                              &coord_mode)) {
            /* mask is two 2-bit fields, where:
             *   '01' -> S
             *   '10' -> T
             *   '11' -> 1 - T  (flip mode)
             */
            unsigned mask = coord_mode ? 0b1101 : 0b1001;
            uint32_t loc = inloc;
            if (compmask & 0x1) {
               vpsrepl[loc / 16] |= ((mask >> 0) & 0x3) << ((loc % 16) * 2);
               loc++;
            }
            if (compmask & 0x2) {
               vpsrepl[loc / 16] |= ((mask >> 2) & 0x3) << ((loc % 16) * 2);
               loc++;
            }
            if (compmask & 0x4) {
               /* .z <- 0.0f */
               vinterp[loc / 16] |= 0b10 << ((loc % 16) * 2);
               loc++;
            }
            if (compmask & 0x8) {
               /* .w <- 1.0f */
               vinterp[loc / 16] |= 0b11 << ((loc % 16) * 2);
               loc++;
            }
         }
      }

      OUT_PKT0(ring, REG_A4XX_VPC_ATTR, 2);
      OUT_RING(ring, A4XX_VPC_ATTR_TOTALATTR(s[FS].v->total_in) |
                        A4XX_VPC_ATTR_THRDASSIGN(1) |
                        COND(s[FS].v->total_in > 0, A4XX_VPC_ATTR_ENABLE) |
                        0x40000000 | /* XXX */
                        COND(s[VS].v->writes_psize, A4XX_VPC_ATTR_PSIZE));
      OUT_RING(ring, A4XX_VPC_PACK_NUMFPNONPOSVAR(s[FS].v->total_in) |
                        A4XX_VPC_PACK_NUMNONPOSVSVAR(s[FS].v->total_in));

      OUT_PKT0(ring, REG_A4XX_VPC_VARYING_INTERP_MODE(0), 8);
      for (i = 0; i < 8; i++)
         OUT_RING(ring, vinterp[i]); /* VPC_VARYING_INTERP[i].MODE */

      OUT_PKT0(ring, REG_A4XX_VPC_VARYING_PS_REPL_MODE(0), 8);
      for (i = 0; i < 8; i++)
         OUT_RING(ring, vpsrepl[i]); /* VPC_VARYING_PS_REPL[i] */
   }

   if (s[VS].instrlen)
      fd4_emit_shader(ring, s[VS].v);

   if (!emit->binning_pass)
      if (s[FS].instrlen)
         fd4_emit_shader(ring, s[FS].v);
}

static struct ir3_program_state *
fd4_program_create(void *data, const struct ir3_shader_variant *bs,
                   const struct ir3_shader_variant *vs,
                   const struct ir3_shader_variant *hs,
                   const struct ir3_shader_variant *ds, 
                   const struct ir3_shader_variant *gs,
                   const struct ir3_shader_variant *fs,
                   const struct ir3_cache_key *key) in_dt
{
   struct fd_context *ctx = fd_context(data);
   struct fd4_program_state *state = CALLOC_STRUCT(fd4_program_state);

   tc_assert_driver_thread(ctx->tc);

   state->bs = bs;
   state->vs = vs;
   state->fs = fs;

   return &state->base;
}

static void
fd4_program_destroy(void *data, struct ir3_program_state *state)
{
   struct fd4_program_state *so = fd4_program_state(state);
   free(so);
}

static const struct ir3_cache_funcs cache_funcs = {
   .create_state = fd4_program_create,
   .destroy_state = fd4_program_destroy,
};

void
fd4_prog_init(struct pipe_context *pctx)
{
   struct fd_context *ctx = fd_context(pctx);

   ctx->shader_cache = ir3_cache_create(&cache_funcs, ctx);
   ir3_prog_init(pctx);
   fd_prog_init(pctx);
}
