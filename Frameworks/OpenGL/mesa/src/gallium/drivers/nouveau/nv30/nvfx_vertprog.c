#include <strings.h>
#include "pipe/p_context.h"
#include "pipe/p_defines.h"
#include "pipe/p_state.h"
#include "util/compiler.h"
#include "util/u_dynarray.h"
#include "util/u_debug.h"
#include "util/u_memory.h"

#include "pipe/p_shader_tokens.h"
#include "tgsi/tgsi_parse.h"
#include "tgsi/tgsi_dump.h"

#include "draw/draw_context.h"

#include "nv_object.xml.h"
#include "nouveau_debug.h"
#include "nv30/nv30-40_3d.xml.h"
#include "nv30/nv30_state.h"

/* TODO (at least...):
 *  1. Indexed consts  + ARL
 *  3. NV_vp11, NV_vp2, NV_vp3 features
 *       - extra arith opcodes
 *       - branching
 *       - texture sampling
 *       - indexed attribs
 *       - indexed results
 *  4. bugs
 */

#include "nv30/nv30_vertprog.h"
#include "nv30/nv40_vertprog.h"

struct nvfx_loop_entry {
   unsigned brk_target;
   unsigned cont_target;
};

struct nvfx_vpc {
   struct pipe_shader_state pipe;
   struct nv30_vertprog *vp;
   struct tgsi_shader_info* info;

   struct nv30_vertprog_exec *vpi;

   unsigned r_temps;
   unsigned r_temps_discard;
   struct nvfx_reg r_result[PIPE_MAX_SHADER_OUTPUTS];
   struct nvfx_reg *r_address;
   struct nvfx_reg *r_temp;
   struct nvfx_reg *r_const;
   struct nvfx_reg r_0_1;

   struct nvfx_reg *imm;
   unsigned nr_imm;

   int hpos_idx;
   int cvtx_idx;

   unsigned is_nv4x;

   struct util_dynarray label_relocs;
   struct util_dynarray loop_stack;
};

static struct nvfx_reg
temp(struct nvfx_vpc *vpc)
{
   int idx = ffs(~vpc->r_temps) - 1;

   if (idx < 0 || (!vpc->is_nv4x && idx >= 16)) {
      NOUVEAU_ERR("out of temps!!\n");
      return nvfx_reg(NVFXSR_TEMP, 0);
   }

   vpc->r_temps |= (1 << idx);
   vpc->r_temps_discard |= (1 << idx);
   return nvfx_reg(NVFXSR_TEMP, idx);
}

static inline void
release_temps(struct nvfx_vpc *vpc)
{
   vpc->r_temps &= ~vpc->r_temps_discard;
   vpc->r_temps_discard = 0;
}

static struct nvfx_reg
constant(struct nvfx_vpc *vpc, int pipe, float x, float y, float z, float w)
{
   struct nv30_vertprog *vp = vpc->vp;
   struct nv30_vertprog_data *vpd;
   int idx;

   if (pipe >= 0) {
      for (idx = 0; idx < vp->nr_consts; idx++) {
         if (vp->consts[idx].index == pipe)
            return nvfx_reg(NVFXSR_CONST, idx);
      }
   }

   idx = vp->nr_consts++;
   vp->consts = realloc(vp->consts, sizeof(*vpd) * vp->nr_consts);
   vpd = &vp->consts[idx];

   vpd->index = pipe;
   vpd->value[0] = x;
   vpd->value[1] = y;
   vpd->value[2] = z;
   vpd->value[3] = w;
   return nvfx_reg(NVFXSR_CONST, idx);
}

#define arith(s,t,o,d,m,s0,s1,s2) \
   nvfx_insn((s), (NVFX_VP_INST_SLOT_##t << 7) | NVFX_VP_INST_##t##_OP_##o, -1, (d), (m), (s0), (s1), (s2))

static void
emit_src(struct nvfx_vpc *vpc, uint32_t *hw,
         int pos, struct nvfx_src src)
{
   struct nv30_vertprog *vp = vpc->vp;
   uint32_t sr = 0;
   struct nvfx_relocation reloc;

   switch (src.reg.type) {
   case NVFXSR_TEMP:
      sr |= (NVFX_VP(SRC_REG_TYPE_TEMP) << NVFX_VP(SRC_REG_TYPE_SHIFT));
      sr |= (src.reg.index << NVFX_VP(SRC_TEMP_SRC_SHIFT));
      break;
   case NVFXSR_INPUT:
      sr |= (NVFX_VP(SRC_REG_TYPE_INPUT) <<
             NVFX_VP(SRC_REG_TYPE_SHIFT));
      vp->ir |= (1 << src.reg.index);
      hw[1] |= (src.reg.index << NVFX_VP(INST_INPUT_SRC_SHIFT));
      break;
   case NVFXSR_CONST:
      sr |= (NVFX_VP(SRC_REG_TYPE_CONST) <<
             NVFX_VP(SRC_REG_TYPE_SHIFT));
      if (src.reg.index < 256 && src.reg.index >= -256) {
         reloc.location = vp->nr_insns - 1;
         reloc.target = src.reg.index;
         util_dynarray_append(&vp->const_relocs, struct nvfx_relocation, reloc);
      } else {
         hw[1] |= (src.reg.index << NVFX_VP(INST_CONST_SRC_SHIFT)) &
               NVFX_VP(INST_CONST_SRC_MASK);
      }
      break;
   case NVFXSR_NONE:
      sr |= (NVFX_VP(SRC_REG_TYPE_INPUT) <<
             NVFX_VP(SRC_REG_TYPE_SHIFT));
      break;
   default:
      assert(0);
   }

   if (src.negate)
      sr |= NVFX_VP(SRC_NEGATE);

   if (src.abs)
      hw[0] |= (1 << (21 + pos));

   sr |= ((src.swz[0] << NVFX_VP(SRC_SWZ_X_SHIFT)) |
          (src.swz[1] << NVFX_VP(SRC_SWZ_Y_SHIFT)) |
          (src.swz[2] << NVFX_VP(SRC_SWZ_Z_SHIFT)) |
          (src.swz[3] << NVFX_VP(SRC_SWZ_W_SHIFT)));

   if(src.indirect) {
      if(src.reg.type == NVFXSR_CONST)
         hw[3] |= NVFX_VP(INST_INDEX_CONST);
      else if(src.reg.type == NVFXSR_INPUT)
         hw[0] |= NVFX_VP(INST_INDEX_INPUT);
      else
         assert(0);

      if(src.indirect_reg)
         hw[0] |= NVFX_VP(INST_ADDR_REG_SELECT_1);
      hw[0] |= src.indirect_swz << NVFX_VP(INST_ADDR_SWZ_SHIFT);
   }

   switch (pos) {
   case 0:
      hw[1] |= ((sr & NVFX_VP(SRC0_HIGH_MASK)) >>
           NVFX_VP(SRC0_HIGH_SHIFT)) << NVFX_VP(INST_SRC0H_SHIFT);
      hw[2] |= (sr & NVFX_VP(SRC0_LOW_MASK)) <<
           NVFX_VP(INST_SRC0L_SHIFT);
      break;
   case 1:
      hw[2] |= sr << NVFX_VP(INST_SRC1_SHIFT);
      break;
   case 2:
      hw[2] |= ((sr & NVFX_VP(SRC2_HIGH_MASK)) >>
           NVFX_VP(SRC2_HIGH_SHIFT)) << NVFX_VP(INST_SRC2H_SHIFT);
      hw[3] |= (sr & NVFX_VP(SRC2_LOW_MASK)) <<
           NVFX_VP(INST_SRC2L_SHIFT);
      break;
   default:
      assert(0);
   }
}

static void
emit_dst(struct nvfx_vpc *vpc, uint32_t *hw,
         int slot, struct nvfx_reg dst)
{
   struct nv30_vertprog *vp = vpc->vp;

   switch (dst.type) {
   case NVFXSR_NONE:
      if(!vpc->is_nv4x)
         hw[0] |= NV30_VP_INST_DEST_TEMP_ID_MASK;
      else {
         hw[3] |= NV40_VP_INST_DEST_MASK;
         if (slot == 0)
            hw[0] |= NV40_VP_INST_VEC_DEST_TEMP_MASK;
         else
            hw[3] |= NV40_VP_INST_SCA_DEST_TEMP_MASK;
      }
      break;
   case NVFXSR_TEMP:
      if(!vpc->is_nv4x)
         hw[0] |= (dst.index << NV30_VP_INST_DEST_TEMP_ID_SHIFT);
      else {
         hw[3] |= NV40_VP_INST_DEST_MASK;
         if (slot == 0)
            hw[0] |= (dst.index << NV40_VP_INST_VEC_DEST_TEMP_SHIFT);
         else
            hw[3] |= (dst.index << NV40_VP_INST_SCA_DEST_TEMP_SHIFT);
      }
      break;
   case NVFXSR_OUTPUT:
      /* TODO: this may be wrong because on nv30 COL0 and BFC0 are swapped */
      if(vpc->is_nv4x) {
         switch (dst.index) {
         case NV30_VP_INST_DEST_CLP(0):
            dst.index = NVFX_VP(INST_DEST_FOGC);
            vp->or   |= (1 << 6);
            break;
         case NV30_VP_INST_DEST_CLP(1):
            dst.index = NVFX_VP(INST_DEST_FOGC);
            vp->or   |= (1 << 7);
            break;
         case NV30_VP_INST_DEST_CLP(2):
            dst.index = NVFX_VP(INST_DEST_FOGC);
            vp->or   |= (1 << 8);
            break;
         case NV30_VP_INST_DEST_CLP(3):
            dst.index = NVFX_VP(INST_DEST_PSZ);
            vp->or   |= (1 << 9);
            break;
         case NV30_VP_INST_DEST_CLP(4):
            dst.index = NVFX_VP(INST_DEST_PSZ);
            vp->or   |= (1 << 10);
            break;
         case NV30_VP_INST_DEST_CLP(5):
            dst.index = NVFX_VP(INST_DEST_PSZ);
            vp->or   |= (1 << 11);
            break;
         case NV40_VP_INST_DEST_COL0: vp->or |= (1 << 0); break;
         case NV40_VP_INST_DEST_COL1: vp->or |= (1 << 1); break;
         case NV40_VP_INST_DEST_BFC0: vp->or |= (1 << 2); break;
         case NV40_VP_INST_DEST_BFC1: vp->or |= (1 << 3); break;
         case NV40_VP_INST_DEST_FOGC: vp->or |= (1 << 4); break;
         case NV40_VP_INST_DEST_PSZ : vp->or |= (1 << 5); break;
         }
      }

      if(!vpc->is_nv4x) {
         hw[3] |= (dst.index << NV30_VP_INST_DEST_SHIFT);
         hw[0] |= NV30_VP_INST_VEC_DEST_TEMP_MASK;

         /*XXX: no way this is entirely correct, someone needs to
          *     figure out what exactly it is.
          */
         hw[3] |= 0x800;
      } else {
         hw[3] |= (dst.index << NV40_VP_INST_DEST_SHIFT);
         if (slot == 0) {
            hw[0] |= NV40_VP_INST_VEC_RESULT;
            hw[0] |= NV40_VP_INST_VEC_DEST_TEMP_MASK;
         } else {
            hw[3] |= NV40_VP_INST_SCA_RESULT;
            hw[3] |= NV40_VP_INST_SCA_DEST_TEMP_MASK;
         }
      }
      break;
   default:
      assert(0);
   }
}

static void
nvfx_vp_emit(struct nvfx_vpc *vpc, struct nvfx_insn insn)
{
   struct nv30_vertprog *vp = vpc->vp;
   unsigned slot = insn.op >> 7;
   unsigned op = insn.op & 0x7f;
   uint32_t *hw;

   vp->insns = realloc(vp->insns, ++vp->nr_insns * sizeof(*vpc->vpi));
   vpc->vpi = &vp->insns[vp->nr_insns - 1];
   memset(vpc->vpi, 0, sizeof(*vpc->vpi));

   hw = vpc->vpi->data;

   if (insn.cc_test != NVFX_COND_TR)
      hw[0] |= NVFX_VP(INST_COND_TEST_ENABLE);
   hw[0] |= (insn.cc_test << NVFX_VP(INST_COND_SHIFT));
   hw[0] |= ((insn.cc_swz[0] << NVFX_VP(INST_COND_SWZ_X_SHIFT)) |
             (insn.cc_swz[1] << NVFX_VP(INST_COND_SWZ_Y_SHIFT)) |
             (insn.cc_swz[2] << NVFX_VP(INST_COND_SWZ_Z_SHIFT)) |
             (insn.cc_swz[3] << NVFX_VP(INST_COND_SWZ_W_SHIFT)));
   if(insn.cc_update)
      hw[0] |= NVFX_VP(INST_COND_UPDATE_ENABLE);

   if(insn.sat) {
      assert(vpc->is_nv4x);
      if(vpc->is_nv4x)
         hw[0] |= NV40_VP_INST_SATURATE;
   }

   if(!vpc->is_nv4x) {
      if(slot == 0)
         hw[1] |= (op << NV30_VP_INST_VEC_OPCODE_SHIFT);
      else {
         hw[0] |= ((op >> 4) << NV30_VP_INST_SCA_OPCODEH_SHIFT);
         hw[1] |= ((op & 0xf) << NV30_VP_INST_SCA_OPCODEL_SHIFT);
      }
//      hw[3] |= NVFX_VP(INST_SCA_DEST_TEMP_MASK);
//      hw[3] |= (mask << NVFX_VP(INST_VEC_WRITEMASK_SHIFT));

      if (insn.dst.type == NVFXSR_OUTPUT) {
         if (slot)
            hw[3] |= (insn.mask << NV30_VP_INST_SDEST_WRITEMASK_SHIFT);
         else
            hw[3] |= (insn.mask << NV30_VP_INST_VDEST_WRITEMASK_SHIFT);
      } else {
         if (slot)
            hw[3] |= (insn.mask << NV30_VP_INST_STEMP_WRITEMASK_SHIFT);
         else
            hw[3] |= (insn.mask << NV30_VP_INST_VTEMP_WRITEMASK_SHIFT);
      }
    } else {
      if (slot == 0) {
         hw[1] |= (op << NV40_VP_INST_VEC_OPCODE_SHIFT);
         hw[3] |= NV40_VP_INST_SCA_DEST_TEMP_MASK;
         hw[3] |= (insn.mask << NV40_VP_INST_VEC_WRITEMASK_SHIFT);
       } else {
         hw[1] |= (op << NV40_VP_INST_SCA_OPCODE_SHIFT);
         hw[0] |= NV40_VP_INST_VEC_DEST_TEMP_MASK ;
         hw[3] |= (insn.mask << NV40_VP_INST_SCA_WRITEMASK_SHIFT);
      }
   }

   emit_dst(vpc, hw, slot, insn.dst);
   emit_src(vpc, hw, 0, insn.src[0]);
   emit_src(vpc, hw, 1, insn.src[1]);
   emit_src(vpc, hw, 2, insn.src[2]);

//   if(insn.src[0].indirect || op == NVFX_VP_INST_VEC_OP_ARL)
//      hw[3] |= NV40_VP_INST_SCA_RESULT;
}

static inline struct nvfx_src
tgsi_src(struct nvfx_vpc *vpc, const struct tgsi_full_src_register *fsrc) {
   struct nvfx_src src;

   switch (fsrc->Register.File) {
   case TGSI_FILE_INPUT:
      src.reg = nvfx_reg(NVFXSR_INPUT, fsrc->Register.Index);
      break;
   case TGSI_FILE_CONSTANT:
      if(fsrc->Register.Indirect) {
         src.reg = vpc->r_const[0];
         src.reg.index = fsrc->Register.Index;
      } else {
         src.reg = vpc->r_const[fsrc->Register.Index];
      }
      break;
   case TGSI_FILE_IMMEDIATE:
      src.reg = vpc->imm[fsrc->Register.Index];
      break;
   case TGSI_FILE_TEMPORARY:
      src.reg = vpc->r_temp[fsrc->Register.Index];
      break;
   default:
      NOUVEAU_ERR("bad src file\n");
      src.reg.index = 0;
      src.reg.type = -1;
      break;
   }

   src.abs = fsrc->Register.Absolute;
   src.negate = fsrc->Register.Negate;
   src.swz[0] = fsrc->Register.SwizzleX;
   src.swz[1] = fsrc->Register.SwizzleY;
   src.swz[2] = fsrc->Register.SwizzleZ;
   src.swz[3] = fsrc->Register.SwizzleW;
   src.indirect = 0;
   src.indirect_reg = 0;
   src.indirect_swz = 0;

   if(fsrc->Register.Indirect) {
      if(fsrc->Indirect.File == TGSI_FILE_ADDRESS &&
         (fsrc->Register.File == TGSI_FILE_CONSTANT ||
          fsrc->Register.File == TGSI_FILE_INPUT)) {
         src.indirect = 1;
         src.indirect_reg = fsrc->Indirect.Index;
         src.indirect_swz = fsrc->Indirect.Swizzle;
      } else {
         src.reg.index = 0;
         src.reg.type = -1;
      }
   }

   return src;
}

static inline struct nvfx_reg
tgsi_dst(struct nvfx_vpc *vpc, const struct tgsi_full_dst_register *fdst) {
   struct nvfx_reg dst;

   switch (fdst->Register.File) {
   case TGSI_FILE_NULL:
      dst = nvfx_reg(NVFXSR_NONE, 0);
      break;
   case TGSI_FILE_OUTPUT:
      dst = vpc->r_result[fdst->Register.Index];
      break;
   case TGSI_FILE_TEMPORARY:
      dst = vpc->r_temp[fdst->Register.Index];
      break;
   case TGSI_FILE_ADDRESS:
      dst = vpc->r_address[fdst->Register.Index];
      break;
   default:
      NOUVEAU_ERR("bad dst file %i\n", fdst->Register.File);
      dst.index = 0;
      dst.type = 0;
      break;
   }

   return dst;
}

static inline int
tgsi_mask(uint tgsi)
{
   int mask = 0;

   if (tgsi & TGSI_WRITEMASK_X) mask |= NVFX_VP_MASK_X;
   if (tgsi & TGSI_WRITEMASK_Y) mask |= NVFX_VP_MASK_Y;
   if (tgsi & TGSI_WRITEMASK_Z) mask |= NVFX_VP_MASK_Z;
   if (tgsi & TGSI_WRITEMASK_W) mask |= NVFX_VP_MASK_W;
   return mask;
}

static bool
nvfx_vertprog_parse_instruction(struct nvfx_vpc *vpc,
            unsigned idx, const struct tgsi_full_instruction *finst)
{
   struct nvfx_src src[3], tmp;
   struct nvfx_reg dst;
   struct nvfx_reg final_dst;
   struct nvfx_src none = nvfx_src(nvfx_reg(NVFXSR_NONE, 0));
   struct nvfx_insn insn;
   struct nvfx_relocation reloc;
   struct nvfx_loop_entry loop;
   bool sat = false;
   int mask;
   int ai = -1, ci = -1, ii = -1;
   int i;
   unsigned sub_depth = 0;

   for (i = 0; i < finst->Instruction.NumSrcRegs; i++) {
      const struct tgsi_full_src_register *fsrc;

      fsrc = &finst->Src[i];
      if (fsrc->Register.File == TGSI_FILE_TEMPORARY) {
         src[i] = tgsi_src(vpc, fsrc);
      }
   }

   for (i = 0; i < finst->Instruction.NumSrcRegs; i++) {
      const struct tgsi_full_src_register *fsrc;

      fsrc = &finst->Src[i];

      switch (fsrc->Register.File) {
      case TGSI_FILE_INPUT:
         if (ai == -1 || ai == fsrc->Register.Index) {
            ai = fsrc->Register.Index;
            src[i] = tgsi_src(vpc, fsrc);
         } else {
            src[i] = nvfx_src(temp(vpc));
            nvfx_vp_emit(vpc, arith(0, VEC, MOV, src[i].reg, NVFX_VP_MASK_ALL,
                         tgsi_src(vpc, fsrc), none, none));
         }
         break;
      case TGSI_FILE_CONSTANT:
         if ((ci == -1 && ii == -1) ||
             ci == fsrc->Register.Index) {
            ci = fsrc->Register.Index;
            src[i] = tgsi_src(vpc, fsrc);
         } else {
            src[i] = nvfx_src(temp(vpc));
            nvfx_vp_emit(vpc, arith(0, VEC, MOV, src[i].reg, NVFX_VP_MASK_ALL,
                         tgsi_src(vpc, fsrc), none, none));
         }
         break;
      case TGSI_FILE_IMMEDIATE:
         if ((ci == -1 && ii == -1) ||
             ii == fsrc->Register.Index) {
            ii = fsrc->Register.Index;
            src[i] = tgsi_src(vpc, fsrc);
         } else {
            src[i] = nvfx_src(temp(vpc));
            nvfx_vp_emit(vpc, arith(0, VEC, MOV, src[i].reg, NVFX_VP_MASK_ALL,
                         tgsi_src(vpc, fsrc), none, none));
         }
         break;
      case TGSI_FILE_TEMPORARY:
         /* handled above */
         break;
      default:
         NOUVEAU_ERR("bad src file\n");
         return false;
      }
   }

   for (i = 0; i < finst->Instruction.NumSrcRegs; i++) {
      if(src[i].reg.type < 0)
         return false;
   }

   if(finst->Dst[0].Register.File == TGSI_FILE_ADDRESS &&
      finst->Instruction.Opcode != TGSI_OPCODE_ARL)
      return false;

   final_dst = dst  = tgsi_dst(vpc, &finst->Dst[0]);
   mask = tgsi_mask(finst->Dst[0].Register.WriteMask);
   if(finst->Instruction.Saturate) {
      assert(finst->Instruction.Opcode != TGSI_OPCODE_ARL);
      if (vpc->is_nv4x)
         sat = true;
      else
      if(dst.type != NVFXSR_TEMP)
         dst = temp(vpc);
   }

   switch (finst->Instruction.Opcode) {
   case TGSI_OPCODE_ADD:
      nvfx_vp_emit(vpc, arith(sat, VEC, ADD, dst, mask, src[0], none, src[1]));
      break;
   case TGSI_OPCODE_ARL:
      nvfx_vp_emit(vpc, arith(0, VEC, ARL, dst, mask, src[0], none, none));
      break;
   case TGSI_OPCODE_CEIL:
      tmp = nvfx_src(temp(vpc));
      nvfx_vp_emit(vpc, arith(0, VEC, FLR, tmp.reg, mask, neg(src[0]), none, none));
      nvfx_vp_emit(vpc, arith(sat, VEC, MOV, dst, mask, neg(tmp), none, none));
      break;
   case TGSI_OPCODE_CMP:
      insn = arith(0, VEC, MOV, none.reg, mask, src[0], none, none);
      insn.cc_update = 1;
      nvfx_vp_emit(vpc, insn);

      insn = arith(sat, VEC, MOV, dst, mask, src[2], none, none);
      insn.cc_test = NVFX_COND_GE;
      nvfx_vp_emit(vpc, insn);

      insn = arith(sat, VEC, MOV, dst, mask, src[1], none, none);
      insn.cc_test = NVFX_COND_LT;
      nvfx_vp_emit(vpc, insn);
      break;
   case TGSI_OPCODE_COS:
      nvfx_vp_emit(vpc, arith(sat, SCA, COS, dst, mask, none, none, src[0]));
      break;
   case TGSI_OPCODE_DP2:
      tmp = nvfx_src(temp(vpc));
      nvfx_vp_emit(vpc, arith(0, VEC, MUL, tmp.reg, NVFX_VP_MASK_X | NVFX_VP_MASK_Y, src[0], src[1], none));
      nvfx_vp_emit(vpc, arith(sat, VEC, ADD, dst, mask, swz(tmp, X, X, X, X), none, swz(tmp, Y, Y, Y, Y)));
      break;
   case TGSI_OPCODE_DP3:
      nvfx_vp_emit(vpc, arith(sat, VEC, DP3, dst, mask, src[0], src[1], none));
      break;
   case TGSI_OPCODE_DP4:
      nvfx_vp_emit(vpc, arith(sat, VEC, DP4, dst, mask, src[0], src[1], none));
      break;
   case TGSI_OPCODE_DST:
      nvfx_vp_emit(vpc, arith(sat, VEC, DST, dst, mask, src[0], src[1], none));
      break;
   case TGSI_OPCODE_EX2:
      nvfx_vp_emit(vpc, arith(sat, SCA, EX2, dst, mask, none, none, src[0]));
      break;
   case TGSI_OPCODE_EXP:
      nvfx_vp_emit(vpc, arith(sat, SCA, EXP, dst, mask, none, none, src[0]));
      break;
   case TGSI_OPCODE_FLR:
      nvfx_vp_emit(vpc, arith(sat, VEC, FLR, dst, mask, src[0], none, none));
      break;
   case TGSI_OPCODE_FRC:
      nvfx_vp_emit(vpc, arith(sat, VEC, FRC, dst, mask, src[0], none, none));
      break;
   case TGSI_OPCODE_LG2:
      nvfx_vp_emit(vpc, arith(sat, SCA, LG2, dst, mask, none, none, src[0]));
      break;
   case TGSI_OPCODE_LIT:
      nvfx_vp_emit(vpc, arith(sat, SCA, LIT, dst, mask, none, none, src[0]));
      break;
   case TGSI_OPCODE_LOG:
      nvfx_vp_emit(vpc, arith(sat, SCA, LOG, dst, mask, none, none, src[0]));
      break;
   case TGSI_OPCODE_LRP:
      tmp = nvfx_src(temp(vpc));
      nvfx_vp_emit(vpc, arith(0, VEC, MAD, tmp.reg, mask, neg(src[0]), src[2], src[2]));
      nvfx_vp_emit(vpc, arith(sat, VEC, MAD, dst, mask, src[0], src[1], tmp));
      break;
   case TGSI_OPCODE_MAD:
      nvfx_vp_emit(vpc, arith(sat, VEC, MAD, dst, mask, src[0], src[1], src[2]));
      break;
   case TGSI_OPCODE_MAX:
      nvfx_vp_emit(vpc, arith(sat, VEC, MAX, dst, mask, src[0], src[1], none));
      break;
   case TGSI_OPCODE_MIN:
      nvfx_vp_emit(vpc, arith(sat, VEC, MIN, dst, mask, src[0], src[1], none));
      break;
   case TGSI_OPCODE_MOV:
      nvfx_vp_emit(vpc, arith(sat, VEC, MOV, dst, mask, src[0], none, none));
      break;
   case TGSI_OPCODE_MUL:
      nvfx_vp_emit(vpc, arith(sat, VEC, MUL, dst, mask, src[0], src[1], none));
      break;
   case TGSI_OPCODE_NOP:
      break;
   case TGSI_OPCODE_POW:
      tmp = nvfx_src(temp(vpc));
      nvfx_vp_emit(vpc, arith(0, SCA, LG2, tmp.reg, NVFX_VP_MASK_X, none, none, swz(src[0], X, X, X, X)));
      nvfx_vp_emit(vpc, arith(0, VEC, MUL, tmp.reg, NVFX_VP_MASK_X, swz(tmp, X, X, X, X), swz(src[1], X, X, X, X), none));
      nvfx_vp_emit(vpc, arith(sat, SCA, EX2, dst, mask, none, none, swz(tmp, X, X, X, X)));
      break;
   case TGSI_OPCODE_RCP:
      nvfx_vp_emit(vpc, arith(sat, SCA, RCP, dst, mask, none, none, src[0]));
      break;
   case TGSI_OPCODE_RSQ:
      nvfx_vp_emit(vpc, arith(sat, SCA, RSQ, dst, mask, none, none, abs(src[0])));
      break;
   case TGSI_OPCODE_SEQ:
      nvfx_vp_emit(vpc, arith(sat, VEC, SEQ, dst, mask, src[0], src[1], none));
      break;
   case TGSI_OPCODE_SGE:
      nvfx_vp_emit(vpc, arith(sat, VEC, SGE, dst, mask, src[0], src[1], none));
      break;
   case TGSI_OPCODE_SGT:
      nvfx_vp_emit(vpc, arith(sat, VEC, SGT, dst, mask, src[0], src[1], none));
      break;
   case TGSI_OPCODE_SIN:
      nvfx_vp_emit(vpc, arith(sat, SCA, SIN, dst, mask, none, none, src[0]));
      break;
   case TGSI_OPCODE_SLE:
      nvfx_vp_emit(vpc, arith(sat, VEC, SLE, dst, mask, src[0], src[1], none));
      break;
   case TGSI_OPCODE_SLT:
      nvfx_vp_emit(vpc, arith(sat, VEC, SLT, dst, mask, src[0], src[1], none));
      break;
   case TGSI_OPCODE_SNE:
      nvfx_vp_emit(vpc, arith(sat, VEC, SNE, dst, mask, src[0], src[1], none));
      break;
   case TGSI_OPCODE_SSG:
      nvfx_vp_emit(vpc, arith(sat, VEC, SSG, dst, mask, src[0], none, none));
      break;
   case TGSI_OPCODE_TRUNC:
      tmp = nvfx_src(temp(vpc));
      insn = arith(0, VEC, MOV, none.reg, mask, src[0], none, none);
      insn.cc_update = 1;
      nvfx_vp_emit(vpc, insn);

      nvfx_vp_emit(vpc, arith(0, VEC, FLR, tmp.reg, mask, abs(src[0]), none, none));
      nvfx_vp_emit(vpc, arith(sat, VEC, MOV, dst, mask, tmp, none, none));

      insn = arith(sat, VEC, MOV, dst, mask, neg(tmp), none, none);
      insn.cc_test = NVFX_COND_LT;
      nvfx_vp_emit(vpc, insn);
      break;
   case TGSI_OPCODE_IF:
      insn = arith(0, VEC, MOV, none.reg, NVFX_VP_MASK_X, src[0], none, none);
      insn.cc_update = 1;
      nvfx_vp_emit(vpc, insn);

      reloc.location = vpc->vp->nr_insns;
      reloc.target = finst->Label.Label + 1;
      util_dynarray_append(&vpc->label_relocs, struct nvfx_relocation, reloc);

      insn = arith(0, SCA, BRA, none.reg, 0, none, none, none);
      insn.cc_test = NVFX_COND_EQ;
      insn.cc_swz[0] = insn.cc_swz[1] = insn.cc_swz[2] = insn.cc_swz[3] = 0;
      nvfx_vp_emit(vpc, insn);
      break;
   case TGSI_OPCODE_ELSE:
   case TGSI_OPCODE_CAL:
      reloc.location = vpc->vp->nr_insns;
      reloc.target = finst->Label.Label;
      util_dynarray_append(&vpc->label_relocs, struct nvfx_relocation, reloc);

      if(finst->Instruction.Opcode == TGSI_OPCODE_CAL)
         insn = arith(0, SCA, CAL, none.reg, 0, none, none, none);
      else
         insn = arith(0, SCA, BRA, none.reg, 0, none, none, none);
      nvfx_vp_emit(vpc, insn);
      break;
   case TGSI_OPCODE_RET:
      if(sub_depth || !vpc->vp->enabled_ucps) {
         tmp = none;
         tmp.swz[0] = tmp.swz[1] = tmp.swz[2] = tmp.swz[3] = 0;
         nvfx_vp_emit(vpc, arith(0, SCA, RET, none.reg, 0, none, none, tmp));
      } else {
         reloc.location = vpc->vp->nr_insns;
         reloc.target = vpc->info->num_instructions;
         util_dynarray_append(&vpc->label_relocs, struct nvfx_relocation, reloc);
         nvfx_vp_emit(vpc, arith(0, SCA, BRA, none.reg, 0, none, none, none));
      }
      break;
   case TGSI_OPCODE_BGNSUB:
      ++sub_depth;
      break;
   case TGSI_OPCODE_ENDSUB:
      --sub_depth;
      break;
   case TGSI_OPCODE_ENDIF:
      /* nothing to do here */
      break;
   case TGSI_OPCODE_BGNLOOP:
      loop.cont_target = idx;
      loop.brk_target = finst->Label.Label + 1;
      util_dynarray_append(&vpc->loop_stack, struct nvfx_loop_entry, loop);
      break;
   case TGSI_OPCODE_ENDLOOP:
      loop = util_dynarray_pop(&vpc->loop_stack, struct nvfx_loop_entry);

      reloc.location = vpc->vp->nr_insns;
      reloc.target = loop.cont_target;
      util_dynarray_append(&vpc->label_relocs, struct nvfx_relocation, reloc);

      nvfx_vp_emit(vpc, arith(0, SCA, BRA, none.reg, 0, none, none, none));
      break;
   case TGSI_OPCODE_CONT:
      loop = util_dynarray_top(&vpc->loop_stack, struct nvfx_loop_entry);

      reloc.location = vpc->vp->nr_insns;
      reloc.target = loop.cont_target;
      util_dynarray_append(&vpc->label_relocs, struct nvfx_relocation, reloc);

      nvfx_vp_emit(vpc, arith(0, SCA, BRA, none.reg, 0, none, none, none));
      break;
   case TGSI_OPCODE_BRK:
      loop = util_dynarray_top(&vpc->loop_stack, struct nvfx_loop_entry);

      reloc.location = vpc->vp->nr_insns;
      reloc.target = loop.brk_target;
      util_dynarray_append(&vpc->label_relocs, struct nvfx_relocation, reloc);

      nvfx_vp_emit(vpc, arith(0, SCA, BRA, none.reg, 0, none, none, none));
      break;
   case TGSI_OPCODE_END:
      assert(!sub_depth);
      if(vpc->vp->enabled_ucps) {
         if(idx != (vpc->info->num_instructions - 1)) {
            reloc.location = vpc->vp->nr_insns;
            reloc.target = vpc->info->num_instructions;
            util_dynarray_append(&vpc->label_relocs, struct nvfx_relocation, reloc);
            nvfx_vp_emit(vpc, arith(0, SCA, BRA, none.reg, 0, none, none, none));
         }
      } else {
         if(vpc->vp->nr_insns)
            vpc->vp->insns[vpc->vp->nr_insns - 1].data[3] |= NVFX_VP_INST_LAST;
         nvfx_vp_emit(vpc, arith(0, VEC, NOP, none.reg, 0, none, none, none));
         vpc->vp->insns[vpc->vp->nr_insns - 1].data[3] |= NVFX_VP_INST_LAST;
      }
      break;
   default:
      NOUVEAU_ERR("invalid opcode %d\n", finst->Instruction.Opcode);
      return false;
   }

   if(finst->Instruction.Saturate && !vpc->is_nv4x) {
      if (!vpc->r_0_1.type)
         vpc->r_0_1 = constant(vpc, -1, 0, 1, 0, 0);
      nvfx_vp_emit(vpc, arith(0, VEC, MAX, dst, mask, nvfx_src(dst), swz(nvfx_src(vpc->r_0_1), X, X, X, X), none));
      nvfx_vp_emit(vpc, arith(0, VEC, MIN, final_dst, mask, nvfx_src(dst), swz(nvfx_src(vpc->r_0_1), Y, Y, Y, Y), none));
   }

   release_temps(vpc);
   return true;
}

static bool
nvfx_vertprog_parse_decl_output(struct nvfx_vpc *vpc,
                                const struct tgsi_full_declaration *fdec)
{
   unsigned num_texcoords = vpc->is_nv4x ? 10 : 8;
   unsigned idx = fdec->Range.First;
   unsigned semantic_index = fdec->Semantic.Index;
   int hw = 0, i;

   switch (fdec->Semantic.Name) {
   case TGSI_SEMANTIC_POSITION:
      hw = NVFX_VP(INST_DEST_POS);
      vpc->hpos_idx = idx;
      break;
   case TGSI_SEMANTIC_CLIPVERTEX:
      vpc->r_result[idx] = temp(vpc);
      vpc->r_temps_discard = 0;
      vpc->cvtx_idx = idx;
      return true;
   case TGSI_SEMANTIC_COLOR:
      if (fdec->Semantic.Index == 0) {
         hw = NVFX_VP(INST_DEST_COL0);
      } else
      if (fdec->Semantic.Index == 1) {
         hw = NVFX_VP(INST_DEST_COL1);
      } else {
         NOUVEAU_ERR("bad colour semantic index\n");
         return false;
      }
      break;
   case TGSI_SEMANTIC_BCOLOR:
      if (fdec->Semantic.Index == 0) {
         hw = NVFX_VP(INST_DEST_BFC0);
      } else
      if (fdec->Semantic.Index == 1) {
         hw = NVFX_VP(INST_DEST_BFC1);
      } else {
         NOUVEAU_ERR("bad bcolour semantic index\n");
         return false;
      }
      break;
   case TGSI_SEMANTIC_FOG:
      hw = NVFX_VP(INST_DEST_FOGC);
      break;
   case TGSI_SEMANTIC_PSIZE:
      hw = NVFX_VP(INST_DEST_PSZ);
      break;
   case TGSI_SEMANTIC_GENERIC:
      /* this is really an identifier for VP/FP linkage */
      semantic_index += 8;
      FALLTHROUGH;
   case TGSI_SEMANTIC_TEXCOORD:
      for (i = 0; i < num_texcoords; i++) {
         if (vpc->vp->texcoord[i] == semantic_index) {
            hw = NVFX_VP(INST_DEST_TC(i));
            break;
         }
      }

      if (i == num_texcoords) {
         vpc->r_result[idx] = nvfx_reg(NVFXSR_NONE, 0);
         return true;
      }
      break;
   case TGSI_SEMANTIC_EDGEFLAG:
      vpc->r_result[idx] = nvfx_reg(NVFXSR_NONE, 0);
      return true;
   default:
      NOUVEAU_ERR("bad output semantic\n");
      return false;
   }

   vpc->r_result[idx] = nvfx_reg(NVFXSR_OUTPUT, hw);
   return true;
}

static bool
nvfx_vertprog_prepare(struct nvfx_vpc *vpc)
{
   struct tgsi_parse_context p;
   int high_const = -1, high_temp = -1, high_addr = -1, nr_imm = 0, i;

   tgsi_parse_init(&p, vpc->pipe.tokens);
   while (!tgsi_parse_end_of_tokens(&p)) {
      const union tgsi_full_token *tok = &p.FullToken;

      tgsi_parse_token(&p);
      switch(tok->Token.Type) {
      case TGSI_TOKEN_TYPE_IMMEDIATE:
         nr_imm++;
         break;
      case TGSI_TOKEN_TYPE_DECLARATION:
      {
         const struct tgsi_full_declaration *fdec;

         fdec = &p.FullToken.FullDeclaration;
         switch (fdec->Declaration.File) {
         case TGSI_FILE_TEMPORARY:
            if (fdec->Range.Last > high_temp) {
               high_temp =
                  fdec->Range.Last;
            }
            break;
         case TGSI_FILE_ADDRESS:
            if (fdec->Range.Last > high_addr) {
               high_addr =
                  fdec->Range.Last;
            }
            break;
         case TGSI_FILE_CONSTANT:
            if (fdec->Range.Last > high_const) {
               high_const =
                     fdec->Range.Last;
            }
            break;
         case TGSI_FILE_OUTPUT:
            if (!nvfx_vertprog_parse_decl_output(vpc, fdec))
               return false;
            break;
         default:
            break;
         }
      }
         break;
      default:
         break;
      }
   }
   tgsi_parse_free(&p);

   if (nr_imm) {
      vpc->imm = CALLOC(nr_imm, sizeof(struct nvfx_reg));
      assert(vpc->imm);
   }

   if (++high_temp) {
      vpc->r_temp = CALLOC(high_temp, sizeof(struct nvfx_reg));
      for (i = 0; i < high_temp; i++)
         vpc->r_temp[i] = temp(vpc);
   }

   if (++high_addr) {
      vpc->r_address = CALLOC(high_addr, sizeof(struct nvfx_reg));
      for (i = 0; i < high_addr; i++)
         vpc->r_address[i] = nvfx_reg(NVFXSR_TEMP, i);
   }

   if(++high_const) {
      vpc->r_const = CALLOC(high_const, sizeof(struct nvfx_reg));
      for (i = 0; i < high_const; i++)
         vpc->r_const[i] = constant(vpc, i, 0, 0, 0, 0);
   }

   vpc->r_temps_discard = 0;
   return true;
}

DEBUG_GET_ONCE_BOOL_OPTION(nvfx_dump_vp, "NVFX_DUMP_VP", false)

bool
_nvfx_vertprog_translate(uint16_t oclass, struct nv30_vertprog *vp)
{
   struct tgsi_parse_context parse;
   struct nvfx_vpc *vpc = NULL;
   struct nvfx_src none = nvfx_src(nvfx_reg(NVFXSR_NONE, 0));
   struct util_dynarray insns;
   int i, ucps;

   vp->translated = false;
   vp->nr_insns = 0;
   vp->nr_consts = 0;

   vpc = CALLOC_STRUCT(nvfx_vpc);
   if (!vpc)
      return false;
   vpc->is_nv4x = (oclass >= NV40_3D_CLASS) ? ~0 : 0;
   vpc->vp   = vp;
   vpc->pipe = vp->pipe;
   vpc->info = &vp->info;
   vpc->cvtx_idx = -1;

   if (!nvfx_vertprog_prepare(vpc)) {
      FREE(vpc);
      return false;
   }

   /* Redirect post-transform vertex position to a temp if user clip
    * planes are enabled.  We need to append code to the vtxprog
    * to handle clip planes later.
    */
   if (vp->enabled_ucps && vpc->cvtx_idx < 0)  {
      vpc->r_result[vpc->hpos_idx] = temp(vpc);
      vpc->r_temps_discard = 0;
      vpc->cvtx_idx = vpc->hpos_idx;
   }

   util_dynarray_init(&insns, NULL);

   tgsi_parse_init(&parse, vp->pipe.tokens);
   while (!tgsi_parse_end_of_tokens(&parse)) {
      tgsi_parse_token(&parse);

      switch (parse.FullToken.Token.Type) {
      case TGSI_TOKEN_TYPE_IMMEDIATE:
      {
         const struct tgsi_full_immediate *imm;

         imm = &parse.FullToken.FullImmediate;
         assert(imm->Immediate.DataType == TGSI_IMM_FLOAT32);
         assert(imm->Immediate.NrTokens == 4 + 1);
         vpc->imm[vpc->nr_imm++] =
            constant(vpc, -1,
                imm->u[0].Float,
                imm->u[1].Float,
                imm->u[2].Float,
                imm->u[3].Float);
      }
         break;
      case TGSI_TOKEN_TYPE_INSTRUCTION:
      {
         const struct tgsi_full_instruction *finst;
         unsigned idx = insns.size >> 2;
         util_dynarray_append(&insns, unsigned, vp->nr_insns);
         finst = &parse.FullToken.FullInstruction;
         if (!nvfx_vertprog_parse_instruction(vpc, idx, finst))
            goto out;
      }
         break;
      default:
         break;
      }
   }

   util_dynarray_append(&insns, unsigned, vp->nr_insns);

   for(unsigned i = 0; i < vpc->label_relocs.size; i += sizeof(struct nvfx_relocation))
   {
      struct nvfx_relocation* label_reloc = (struct nvfx_relocation*)((char*)vpc->label_relocs.data + i);
      struct nvfx_relocation hw_reloc;

      hw_reloc.location = label_reloc->location;
      hw_reloc.target = ((unsigned*)insns.data)[label_reloc->target];

      //debug_printf("hw %u -> tgsi %u = hw %u\n", hw_reloc.location, label_reloc->target, hw_reloc.target);

      util_dynarray_append(&vp->branch_relocs, struct nvfx_relocation, hw_reloc);
   }
   util_dynarray_fini(&insns);
   util_dynarray_trim(&vp->branch_relocs);

   /* XXX: what if we add a RET before?!  make sure we jump here...*/

   /* Write out HPOS if it was redirected to a temp earlier */
   if (vpc->r_result[vpc->hpos_idx].type != NVFXSR_OUTPUT) {
      struct nvfx_reg hpos = nvfx_reg(NVFXSR_OUTPUT,
                  NVFX_VP(INST_DEST_POS));
      struct nvfx_src htmp = nvfx_src(vpc->r_result[vpc->hpos_idx]);

      nvfx_vp_emit(vpc, arith(0, VEC, MOV, hpos, NVFX_VP_MASK_ALL, htmp, none, none));
   }

   /* Insert code to handle user clip planes */
   ucps = vp->enabled_ucps;
   while (ucps) {
      int i = ffs(ucps) - 1; ucps &= ~(1 << i);
      struct nvfx_reg cdst = nvfx_reg(NVFXSR_OUTPUT, NV30_VP_INST_DEST_CLP(i));
      struct nvfx_src ceqn = nvfx_src(nvfx_reg(NVFXSR_CONST, 512 + i));
      struct nvfx_src htmp = nvfx_src(vpc->r_result[vpc->cvtx_idx]);
      unsigned mask;

      if(vpc->is_nv4x)
      {
         switch (i) {
         case 0: case 3: mask = NVFX_VP_MASK_Y; break;
         case 1: case 4: mask = NVFX_VP_MASK_Z; break;
         case 2: case 5: mask = NVFX_VP_MASK_W; break;
         default:
            NOUVEAU_ERR("invalid clip dist #%d\n", i);
            goto out;
         }
      }
      else
         mask = NVFX_VP_MASK_X;

      nvfx_vp_emit(vpc, arith(0, VEC, DP4, cdst, mask, htmp, ceqn, none));
   }

   if (vpc->vp->nr_insns)
      vpc->vp->insns[vpc->vp->nr_insns - 1].data[3] |= NVFX_VP_INST_LAST;

   if(debug_get_option_nvfx_dump_vp())
   {
      debug_printf("\n");
      tgsi_dump(vpc->pipe.tokens, 0);

      debug_printf("\n%s vertex program:\n", vpc->is_nv4x ? "nv4x" : "nv3x");
      for (i = 0; i < vp->nr_insns; i++)
         debug_printf("%3u: %08x %08x %08x %08x\n", i, vp->insns[i].data[0], vp->insns[i].data[1], vp->insns[i].data[2], vp->insns[i].data[3]);
      debug_printf("\n");
   }

   vp->translated = true;

out:
   tgsi_parse_free(&parse);
   if (vpc) {
      util_dynarray_fini(&vpc->label_relocs);
      util_dynarray_fini(&vpc->loop_stack);
      FREE(vpc->r_temp);
      FREE(vpc->r_address);
      FREE(vpc->r_const);
      FREE(vpc->imm);
      FREE(vpc);
   }

   return vp->translated;
}
