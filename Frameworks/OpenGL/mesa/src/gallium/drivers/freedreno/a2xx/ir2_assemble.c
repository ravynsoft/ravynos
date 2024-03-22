/*
 * Copyright (C) 2018 Jonathan Marek <jonathan@marek.ca>
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
 *    Jonathan Marek <jonathan@marek.ca>
 */

#include "ir2_private.h"

static unsigned
src_swizzle(struct ir2_context *ctx, struct ir2_src *src, unsigned ncomp)
{
   struct ir2_reg_component *comps;
   unsigned swiz = 0;

   switch (src->type) {
   case IR2_SRC_SSA:
   case IR2_SRC_REG:
      break;
   default:
      return src->swizzle;
   }
   /* we need to take into account where the components were allocated */
   comps = get_reg_src(ctx, src)->comp;
   for (int i = 0; i < ncomp; i++) {
      swiz |= swiz_set(comps[swiz_get(src->swizzle, i)].c, i);
   }
   return swiz;
}

/* alu instr need to take into how the output components are allocated */

/* scalar doesn't need to take into account dest swizzle */

static unsigned
alu_swizzle_scalar(struct ir2_context *ctx, struct ir2_src *reg)
{
   /* hardware seems to take from W, but swizzle everywhere just in case */
   return swiz_merge(src_swizzle(ctx, reg, 1), IR2_SWIZZLE_XXXX);
}

static unsigned
alu_swizzle(struct ir2_context *ctx, struct ir2_instr *instr,
            struct ir2_src *src)
{
   struct ir2_reg_component *comp = get_reg(instr)->comp;
   unsigned swiz0 = src_swizzle(ctx, src, src_ncomp(instr));
   unsigned swiz = 0;

   /* non per component special cases */
   switch (instr->alu.vector_opc) {
   case PRED_SETE_PUSHv ... PRED_SETGTE_PUSHv:
      return alu_swizzle_scalar(ctx, src);
   case DOT2ADDv:
   case DOT3v:
   case DOT4v:
   case CUBEv:
      return swiz0;
   default:
      break;
   }

   for (int i = 0, j = 0; i < dst_ncomp(instr); j++) {
      if (instr->alu.write_mask & 1 << j) {
         if (comp[j].c != 7)
            swiz |= swiz_set(i, comp[j].c);
         i++;
      }
   }
   return swiz_merge(swiz0, swiz);
}

static unsigned
alu_swizzle_scalar2(struct ir2_context *ctx, struct ir2_src *src, unsigned s1)
{
   /* hardware seems to take from ZW, but swizzle everywhere (ABAB) */
   unsigned s0 = swiz_get(src_swizzle(ctx, src, 1), 0);
   return swiz_merge(swiz_set(s0, 0) | swiz_set(s1, 1), IR2_SWIZZLE_XYXY);
}

/* write_mask needs to be transformed by allocation information */

static unsigned
alu_write_mask(struct ir2_context *ctx, struct ir2_instr *instr)
{
   struct ir2_reg_component *comp = get_reg(instr)->comp;
   unsigned write_mask = 0;

   for (int i = 0; i < 4; i++) {
      if (instr->alu.write_mask & 1 << i)
         write_mask |= 1 << comp[i].c;
   }

   return write_mask;
}

/* fetch instructions can swizzle dest, but src swizzle needs conversion */

static unsigned
fetch_swizzle(struct ir2_context *ctx, struct ir2_src *src, unsigned ncomp)
{
   unsigned alu_swiz = src_swizzle(ctx, src, ncomp);
   unsigned swiz = 0;
   for (int i = 0; i < ncomp; i++)
      swiz |= swiz_get(alu_swiz, i) << i * 2;
   return swiz;
}

static unsigned
fetch_dst_swiz(struct ir2_context *ctx, struct ir2_instr *instr)
{
   struct ir2_reg_component *comp = get_reg(instr)->comp;
   unsigned dst_swiz = 0xfff;
   for (int i = 0; i < dst_ncomp(instr); i++) {
      dst_swiz &= ~(7 << comp[i].c * 3);
      dst_swiz |= i << comp[i].c * 3;
   }
   return dst_swiz;
}

/* register / export # for instr */
static unsigned
dst_to_reg(struct ir2_context *ctx, struct ir2_instr *instr)
{
   if (is_export(instr))
      return instr->alu.export;

   return get_reg(instr)->idx;
}

/* register # for src */
static unsigned
src_to_reg(struct ir2_context *ctx, struct ir2_src *src)
{
   return get_reg_src(ctx, src)->idx;
}

static unsigned
src_reg_byte(struct ir2_context *ctx, struct ir2_src *src)
{
   if (src->type == IR2_SRC_CONST) {
      assert(!src->abs); /* no abs bit for const */
      return src->num;
   }
   return src_to_reg(ctx, src) | (src->abs ? 0x80 : 0);
}

/* produce the 12 byte binary instruction for a given sched_instr */
static void
fill_instr(struct ir2_context *ctx, struct ir2_sched_instr *sched, instr_t *bc,
           bool *is_fetch)
{
   struct ir2_instr *instr = sched->instr, *instr_s, *instr_v;

   *bc = (instr_t){};

   if (instr && instr->type == IR2_FETCH) {
      *is_fetch = true;

      bc->fetch.opc = instr->fetch.opc;
      bc->fetch.pred_select = !!instr->pred;
      bc->fetch.pred_condition = instr->pred & 1;

      struct ir2_src *src = instr->src;

      if (instr->fetch.opc == VTX_FETCH) {
         instr_fetch_vtx_t *vtx = &bc->fetch.vtx;

         assert(instr->fetch.vtx.const_idx <= 0x1f);
         assert(instr->fetch.vtx.const_idx_sel <= 0x3);

         vtx->src_reg = src_to_reg(ctx, src);
         vtx->src_swiz = fetch_swizzle(ctx, src, 1);
         vtx->dst_reg = dst_to_reg(ctx, instr);
         vtx->dst_swiz = fetch_dst_swiz(ctx, instr);

         vtx->must_be_one = 1;
         vtx->const_index = instr->fetch.vtx.const_idx;
         vtx->const_index_sel = instr->fetch.vtx.const_idx_sel;

         /* other fields will be patched */

         /* XXX seems like every FETCH but the first has
          * this bit set:
          */
         vtx->reserved3 = instr->idx ? 0x1 : 0x0;
         vtx->reserved0 = instr->idx ? 0x2 : 0x3;
      } else if (instr->fetch.opc == TEX_FETCH) {
         instr_fetch_tex_t *tex = &bc->fetch.tex;

         tex->src_reg = src_to_reg(ctx, src);
         tex->src_swiz = fetch_swizzle(ctx, src, 3);
         tex->dst_reg = dst_to_reg(ctx, instr);
         tex->dst_swiz = fetch_dst_swiz(ctx, instr);
         /* tex->const_idx = patch_fetches */
         tex->mag_filter = TEX_FILTER_USE_FETCH_CONST;
         tex->min_filter = TEX_FILTER_USE_FETCH_CONST;
         tex->mip_filter = TEX_FILTER_USE_FETCH_CONST;
         tex->aniso_filter = ANISO_FILTER_USE_FETCH_CONST;
         tex->arbitrary_filter = ARBITRARY_FILTER_USE_FETCH_CONST;
         tex->vol_mag_filter = TEX_FILTER_USE_FETCH_CONST;
         tex->vol_min_filter = TEX_FILTER_USE_FETCH_CONST;
         tex->use_comp_lod = ctx->so->type == MESA_SHADER_FRAGMENT;
         tex->use_reg_lod = instr->src_count == 2;
         tex->sample_location = SAMPLE_CENTER;
         tex->tx_coord_denorm = instr->fetch.tex.is_rect;
      } else if (instr->fetch.opc == TEX_SET_TEX_LOD) {
         instr_fetch_tex_t *tex = &bc->fetch.tex;

         tex->src_reg = src_to_reg(ctx, src);
         tex->src_swiz = fetch_swizzle(ctx, src, 1);
         tex->dst_reg = 0;
         tex->dst_swiz = 0xfff;

         tex->mag_filter = TEX_FILTER_USE_FETCH_CONST;
         tex->min_filter = TEX_FILTER_USE_FETCH_CONST;
         tex->mip_filter = TEX_FILTER_USE_FETCH_CONST;
         tex->aniso_filter = ANISO_FILTER_USE_FETCH_CONST;
         tex->arbitrary_filter = ARBITRARY_FILTER_USE_FETCH_CONST;
         tex->vol_mag_filter = TEX_FILTER_USE_FETCH_CONST;
         tex->vol_min_filter = TEX_FILTER_USE_FETCH_CONST;
         tex->use_comp_lod = 1;
         tex->use_reg_lod = 0;
         tex->sample_location = SAMPLE_CENTER;
      } else {
         assert(0);
      }
      return;
   }

   instr_v = sched->instr;
   instr_s = sched->instr_s;

   if (instr_v) {
      struct ir2_src src1, src2, *src3;

      src1 = instr_v->src[0];
      src2 = instr_v->src[instr_v->src_count > 1];
      src3 = instr_v->src_count == 3 ? &instr_v->src[2] : NULL;

      bc->alu.vector_opc = instr_v->alu.vector_opc;
      bc->alu.vector_write_mask = alu_write_mask(ctx, instr_v);
      bc->alu.vector_dest = dst_to_reg(ctx, instr_v);
      bc->alu.vector_clamp = instr_v->alu.saturate;
      bc->alu.export_data = instr_v->alu.export >= 0;

      /* single operand SETEv, use 0.0f as src2 */
      if (instr_v->src_count == 1 &&
          (bc->alu.vector_opc == SETEv || bc->alu.vector_opc == SETNEv ||
           bc->alu.vector_opc == SETGTv || bc->alu.vector_opc == SETGTEv))
         src2 = ir2_zero(ctx);

      /* export32 instr for a20x hw binning has this bit set..
       * it seems to do more than change the base address of constants
       * XXX this is a hack
       */
      bc->alu.relative_addr =
         (bc->alu.export_data && bc->alu.vector_dest == 32);

      bc->alu.src1_reg_byte = src_reg_byte(ctx, &src1);
      bc->alu.src1_swiz = alu_swizzle(ctx, instr_v, &src1);
      bc->alu.src1_reg_negate = src1.negate;
      bc->alu.src1_sel = src1.type != IR2_SRC_CONST;

      bc->alu.src2_reg_byte = src_reg_byte(ctx, &src2);
      bc->alu.src2_swiz = alu_swizzle(ctx, instr_v, &src2);
      bc->alu.src2_reg_negate = src2.negate;
      bc->alu.src2_sel = src2.type != IR2_SRC_CONST;

      if (src3) {
         bc->alu.src3_reg_byte = src_reg_byte(ctx, src3);
         bc->alu.src3_swiz = alu_swizzle(ctx, instr_v, src3);
         bc->alu.src3_reg_negate = src3->negate;
         bc->alu.src3_sel = src3->type != IR2_SRC_CONST;
      }

      bc->alu.pred_select = instr_v->pred;
   }

   if (instr_s) {
      struct ir2_src *src = instr_s->src;

      bc->alu.scalar_opc = instr_s->alu.scalar_opc;
      bc->alu.scalar_write_mask = alu_write_mask(ctx, instr_s);
      bc->alu.scalar_dest = dst_to_reg(ctx, instr_s);
      bc->alu.scalar_clamp = instr_s->alu.saturate;
      bc->alu.export_data = instr_s->alu.export >= 0;

      if (instr_s->src_count == 1) {
         bc->alu.src3_reg_byte = src_reg_byte(ctx, src);
         bc->alu.src3_swiz = alu_swizzle_scalar(ctx, src);
         bc->alu.src3_reg_negate = src->negate;
         bc->alu.src3_sel = src->type != IR2_SRC_CONST;
      } else {
         assert(instr_s->src_count == 2);

         bc->alu.src3_reg_byte = src_reg_byte(ctx, src);
         bc->alu.src3_swiz =
            alu_swizzle_scalar2(ctx, src, instr_s->alu.src1_swizzle);
         bc->alu.src3_reg_negate = src->negate;
         bc->alu.src3_sel = src->type != IR2_SRC_CONST;
         ;
      }

      if (instr_v)
         assert(instr_s->pred == instr_v->pred);
      bc->alu.pred_select = instr_s->pred;
   }

   *is_fetch = false;
   return;
}

static unsigned
write_cfs(struct ir2_context *ctx, instr_cf_t *cfs, unsigned cf_idx,
          instr_cf_alloc_t *alloc, instr_cf_exec_t *exec)
{
   assert(exec->count);

   if (alloc)
      cfs[cf_idx++].alloc = *alloc;

   /* for memory alloc offset for patching */
   if (alloc && alloc->buffer_select == SQ_MEMORY &&
       ctx->info->mem_export_ptr == -1)
      ctx->info->mem_export_ptr = cf_idx / 2 * 3;

   cfs[cf_idx++].exec = *exec;
   exec->address += exec->count;
   exec->serialize = 0;
   exec->count = 0;

   return cf_idx;
}

/* assemble the final shader */
void
assemble(struct ir2_context *ctx, bool binning)
{
   /* hw seems to have a limit of 384 (num_cf/2+num_instr <= 384)
    * address is 9 bits so could it be 512 ?
    */
   instr_cf_t cfs[384];
   instr_t bytecode[384], bc;
   unsigned block_addr[128];
   unsigned num_cf = 0;

   /* CF instr state */
   instr_cf_exec_t exec = {.opc = EXEC};
   instr_cf_alloc_t alloc = {.opc = ALLOC};

   int sync_id, sync_id_prev = -1;
   bool is_fetch = false;
   bool need_sync = true;
   bool need_alloc = false;
   unsigned block_idx = 0;

   ctx->info->mem_export_ptr = -1;
   ctx->info->num_fetch_instrs = 0;

   /* vertex shader always needs to allocate at least one parameter
    * if it will never happen,
    */
   if (ctx->so->type == MESA_SHADER_VERTEX && ctx->f->inputs_count == 0) {
      alloc.buffer_select = SQ_PARAMETER_PIXEL;
      cfs[num_cf++].alloc = alloc;
   }

   block_addr[0] = 0;

   for (int i = 0, j = 0; j < ctx->instr_sched_count; j++) {
      struct ir2_instr *instr = ctx->instr_sched[j].instr;

      /* catch IR2_CF since it isn't a regular instruction */
      if (instr && instr->type == IR2_CF) {
         assert(!need_alloc); /* XXX */

         /* flush any exec cf before inserting jmp */
         if (exec.count)
            num_cf = write_cfs(ctx, cfs, num_cf, NULL, &exec);

         cfs[num_cf++].jmp_call = (instr_cf_jmp_call_t){
            .opc = COND_JMP,
            .address = instr->cf.block_idx, /* will be fixed later */
            .force_call = !instr->pred,
            .predicated_jmp = 1,
            .direction = instr->cf.block_idx > instr->block_idx,
            .condition = instr->pred & 1,
         };
         continue;
      }

      /* fill the 3 dwords for the instruction */
      fill_instr(ctx, &ctx->instr_sched[j], &bc, &is_fetch);

      /* we need to sync between ALU/VTX_FETCH/TEX_FETCH types */
      sync_id = 0;
      if (is_fetch)
         sync_id = bc.fetch.opc == VTX_FETCH ? 1 : 2;

      need_sync = sync_id != sync_id_prev;
      sync_id_prev = sync_id;

      unsigned block;
      {

         if (ctx->instr_sched[j].instr)
            block = ctx->instr_sched[j].instr->block_idx;
         else
            block = ctx->instr_sched[j].instr_s->block_idx;

         assert(block_idx <= block);
      }

      /* info for patching */
      if (is_fetch) {
         struct ir2_fetch_info *info =
            &ctx->info->fetch_info[ctx->info->num_fetch_instrs++];
         info->offset = i * 3; /* add cf offset later */

         if (bc.fetch.opc == VTX_FETCH) {
            info->vtx.dst_swiz = bc.fetch.vtx.dst_swiz;
         } else if (bc.fetch.opc == TEX_FETCH) {
            info->tex.samp_id = instr->fetch.tex.samp_id;
            info->tex.src_swiz = bc.fetch.tex.src_swiz;
         } else {
            ctx->info->num_fetch_instrs--;
         }
      }

      /* exec cf after 6 instr or when switching between fetch / alu */
      if (exec.count == 6 ||
          (exec.count && (need_sync || block != block_idx))) {
         num_cf =
            write_cfs(ctx, cfs, num_cf, need_alloc ? &alloc : NULL, &exec);
         need_alloc = false;
      }

      /* update block_addrs for jmp patching */
      while (block_idx < block)
         block_addr[++block_idx] = num_cf;

      /* export - fill alloc cf */
      if (!is_fetch && bc.alu.export_data) {
         /* get the export buffer from either vector/scalar dest */
         instr_alloc_type_t buffer = export_buf(bc.alu.vector_dest);
         if (bc.alu.scalar_write_mask) {
            if (bc.alu.vector_write_mask)
               assert(buffer == export_buf(bc.alu.scalar_dest));
            buffer = export_buf(bc.alu.scalar_dest);
         }

         /* flush previous alloc if the buffer changes */
         bool need_new_alloc = buffer != alloc.buffer_select;

         /* memory export always in 32/33 pair, new alloc on 32 */
         if (bc.alu.vector_dest == 32)
            need_new_alloc = true;

         if (need_new_alloc && exec.count) {
            num_cf =
               write_cfs(ctx, cfs, num_cf, need_alloc ? &alloc : NULL, &exec);
            need_alloc = false;
         }

         need_alloc |= need_new_alloc;

         alloc.size = 0;
         alloc.buffer_select = buffer;

         if (buffer == SQ_PARAMETER_PIXEL &&
             ctx->so->type == MESA_SHADER_VERTEX)
            alloc.size = ctx->f->inputs_count - 1;

         if (buffer == SQ_POSITION)
            alloc.size = ctx->so->writes_psize;
      }

      if (is_fetch)
         exec.serialize |= 0x1 << exec.count * 2;
      if (need_sync)
         exec.serialize |= 0x2 << exec.count * 2;

      need_sync = false;
      exec.count += 1;
      bytecode[i++] = bc;
   }

   /* final exec cf */
   exec.opc = EXEC_END;
   num_cf = write_cfs(ctx, cfs, num_cf, need_alloc ? &alloc : NULL, &exec);

   /* insert nop to get an even # of CFs */
   if (num_cf % 2)
      cfs[num_cf++] = (instr_cf_t){.opc = NOP};

   /* patch cf addrs */
   for (int idx = 0; idx < num_cf; idx++) {
      switch (cfs[idx].opc) {
      case NOP:
      case ALLOC:
         break;
      case EXEC:
      case EXEC_END:
         cfs[idx].exec.address += num_cf / 2;
         break;
      case COND_JMP:
         cfs[idx].jmp_call.address = block_addr[cfs[idx].jmp_call.address];
         break;
      default:
         assert(0);
      }
   }

   /* concatenate cfs and alu/fetch */
   uint32_t cfdwords = num_cf / 2 * 3;
   uint32_t alufetchdwords = exec.address * 3;
   uint32_t sizedwords = cfdwords + alufetchdwords;
   uint32_t *dwords = malloc(sizedwords * 4);
   assert(dwords);
   memcpy(dwords, cfs, cfdwords * 4);
   memcpy(&dwords[cfdwords], bytecode, alufetchdwords * 4);

   /* finalize ir2_shader_info */
   ctx->info->dwords = dwords;
   ctx->info->sizedwords = sizedwords;
   for (int i = 0; i < ctx->info->num_fetch_instrs; i++)
      ctx->info->fetch_info[i].offset += cfdwords;

   if (FD_DBG(DISASM)) {
      DBG("disassemble: type=%d", ctx->so->type);
      disasm_a2xx(dwords, sizedwords, 0, ctx->so->type);
   }
}
