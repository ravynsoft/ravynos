/*
 * Copyright 2008 Corbin Simpson <MostAwesomeDude@gmail.com>
 *
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE COPYRIGHT OWNER(S) AND/OR ITS SUPPLIERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include "r500_fragprog.h"

#include <stdio.h>

#include "radeon_compiler_util.h"
#include "radeon_list.h"
#include "radeon_variable.h"
#include "r300_reg.h"

#include "util/compiler.h"

/**
 * Rewrite IF instructions to use the ALU result special register.
 */
static void r500_transform_IF_instr(
	struct radeon_compiler * c,
	struct rc_instruction * inst_if,
	struct rc_list * var_list)
{

	struct rc_variable * writer;
	struct rc_list * writer_list, * list_ptr;
	unsigned int generic_if = 0;
	unsigned int alu_chan;

	writer_list = rc_variable_list_get_writers(
			var_list, inst_if->Type, &inst_if->U.I.SrcReg[0]);
	if (!writer_list) {
		generic_if = 1;
	} else {

		/* Make sure it is safe for the writers to write to
		 * ALU Result */
		for (list_ptr = writer_list; list_ptr;
						list_ptr = list_ptr->Next) {
			struct rc_instruction * inst;
			writer = list_ptr->Item;
			/* We are going to modify the destination register
			 * of writer, so if it has a reader other than
			 * inst_if (aka ReaderCount > 1) we must fall back to
			 * our generic IF.
			 * If the writer has a lower IP than inst_if, this
			 * means that inst_if is above the writer in a loop.
			 * I'm not sure why this would ever happen, but
			 * if it does we want to make sure we fall back
			 * to our generic IF. */
			if (writer->ReaderCount > 1 || writer->Inst->IP < inst_if->IP) {
				generic_if = 1;
				break;
			}

			/* The ALU Result is not preserved across IF
			 * instructions, so if there is another IF
			 * instruction between writer and inst_if, then
			 * we need to fall back to generic IF. */
			for (inst = writer->Inst; inst != inst_if; inst = inst->Next) {
				const struct rc_opcode_info * info =
					rc_get_opcode_info(inst->U.I.Opcode);
				if (info->IsFlowControl) {
					generic_if = 1;
					break;
				}
			}
			if (generic_if) {
				break;
			}
		}
	}

	if (GET_SWZ(inst_if->U.I.SrcReg[0].Swizzle, 0) == RC_SWIZZLE_X) {
		alu_chan = RC_ALURESULT_X;
	} else {
		alu_chan = RC_ALURESULT_W;
	}
	if (generic_if) {
		struct rc_instruction * inst_mov =
				rc_insert_new_instruction(c, inst_if->Prev);

		inst_mov->U.I.Opcode = RC_OPCODE_MOV;
		inst_mov->U.I.DstReg.WriteMask = 0;
		inst_mov->U.I.DstReg.File = RC_FILE_NONE;
		inst_mov->U.I.ALUResultCompare = RC_COMPARE_FUNC_NOTEQUAL;
		inst_mov->U.I.WriteALUResult = alu_chan;
		inst_mov->U.I.SrcReg[0] = inst_if->U.I.SrcReg[0];
		if (alu_chan == RC_ALURESULT_X) {
			inst_mov->U.I.SrcReg[0].Swizzle = combine_swizzles4(
					inst_mov->U.I.SrcReg[0].Swizzle,
					RC_SWIZZLE_X, RC_SWIZZLE_UNUSED,
					RC_SWIZZLE_UNUSED, RC_SWIZZLE_UNUSED);
		} else {
			inst_mov->U.I.SrcReg[0].Swizzle = combine_swizzles4(
					inst_mov->U.I.SrcReg[0].Swizzle,
					RC_SWIZZLE_UNUSED, RC_SWIZZLE_UNUSED,
					RC_SWIZZLE_UNUSED, RC_SWIZZLE_Z);
		}
	} else {
		rc_compare_func compare_func = RC_COMPARE_FUNC_NEVER;
		unsigned int reverse_srcs = 0;
		unsigned int preserve_opcode = 0;
		for (list_ptr = writer_list; list_ptr;
						list_ptr = list_ptr->Next) {
			writer = list_ptr->Item;
			switch(writer->Inst->U.I.Opcode) {
			case RC_OPCODE_SEQ:
				compare_func = RC_COMPARE_FUNC_EQUAL;
				break;
			case RC_OPCODE_SNE:
				compare_func = RC_COMPARE_FUNC_NOTEQUAL;
				break;
			case RC_OPCODE_SLE:
				reverse_srcs = 1;
				FALLTHROUGH;
			case RC_OPCODE_SGE:
				compare_func = RC_COMPARE_FUNC_GEQUAL;
				break;
			case RC_OPCODE_SGT:
				reverse_srcs = 1;
				FALLTHROUGH;
			case RC_OPCODE_SLT:
				compare_func = RC_COMPARE_FUNC_LESS;
				break;
			default:
				compare_func = RC_COMPARE_FUNC_NOTEQUAL;
				preserve_opcode = 1;
				break;
			}
			if (!preserve_opcode) {
				writer->Inst->U.I.Opcode = RC_OPCODE_SUB;
			}
			writer->Inst->U.I.DstReg.WriteMask = 0;
			writer->Inst->U.I.DstReg.File = RC_FILE_NONE;
			writer->Inst->U.I.WriteALUResult = alu_chan;
			writer->Inst->U.I.ALUResultCompare = compare_func;
			if (reverse_srcs) {
				struct rc_src_register temp_src;
				temp_src = writer->Inst->U.I.SrcReg[0];
				writer->Inst->U.I.SrcReg[0] =
					writer->Inst->U.I.SrcReg[1];
				writer->Inst->U.I.SrcReg[1] = temp_src;
			}
		}
	}

	inst_if->U.I.SrcReg[0].File = RC_FILE_SPECIAL;
	inst_if->U.I.SrcReg[0].Index = RC_SPECIAL_ALU_RESULT;
	inst_if->U.I.SrcReg[0].Swizzle = RC_MAKE_SWIZZLE(
				RC_SWIZZLE_X, RC_SWIZZLE_UNUSED,
				RC_SWIZZLE_UNUSED, RC_SWIZZLE_UNUSED);
	inst_if->U.I.SrcReg[0].Negate = 0;
}

void r500_transform_IF(
	struct radeon_compiler * c,
	void *user)
{
	struct rc_list * var_list = rc_get_variables(c);

	struct rc_instruction * inst = c->Program.Instructions.Next;
	while(inst != &c->Program.Instructions) {
		struct rc_instruction * current = inst;
		inst = inst->Next;

		if (current->U.I.Opcode == RC_OPCODE_IF)
			r500_transform_IF_instr(c, current, var_list);
	}
}

static int r500_swizzle_is_native(rc_opcode opcode, struct rc_src_register reg)
{
	unsigned int relevant;
	int i;

	if (opcode == RC_OPCODE_TEX ||
	    opcode == RC_OPCODE_TXB ||
	    opcode == RC_OPCODE_TXP ||
	    opcode == RC_OPCODE_TXD ||
	    opcode == RC_OPCODE_TXL ||
	    opcode == RC_OPCODE_KIL) {
		if (reg.Abs)
			return 0;

		if (opcode == RC_OPCODE_KIL && (reg.Swizzle != RC_SWIZZLE_XYZW || reg.Negate != RC_MASK_NONE))
			return 0;

		for(i = 0; i < 4; ++i) {
			unsigned int swz = GET_SWZ(reg.Swizzle, i);
			if (swz == RC_SWIZZLE_UNUSED) {
				reg.Negate &= ~(1 << i);
				continue;
			}
			if (swz >= 4)
				return 0;
		}

		if (reg.Negate)
			return 0;

		return 1;
	} else if (opcode == RC_OPCODE_DDX || opcode == RC_OPCODE_DDY) {
		/* DDX/MDH and DDY/MDV explicitly ignore incoming swizzles;
		 * if it doesn't fit perfectly into a .xyzw case... */
		if (reg.Swizzle == RC_SWIZZLE_XYZW && !reg.Abs && !reg.Negate)
			return 1;

		return 0;
	} else {
		/* ALU instructions support almost everything */
		relevant = 0;
		for(i = 0; i < 3; ++i) {
			unsigned int swz = GET_SWZ(reg.Swizzle, i);
			if (swz != RC_SWIZZLE_UNUSED && swz != RC_SWIZZLE_ZERO)
				relevant |= 1 << i;
		}
		if ((reg.Negate & relevant) && ((reg.Negate & relevant) != relevant))
			return 0;

		return 1;
	}
}

/**
 * Split source register access.
 *
 * The only thing we *cannot* do in an ALU instruction is per-component
 * negation.
 */
static void r500_swizzle_split(struct rc_src_register src, unsigned int usemask,
		struct rc_swizzle_split * split)
{
	unsigned int negatebase[2] = { 0, 0 };
	int i;

	for(i = 0; i < 4; ++i) {
		unsigned int swz = GET_SWZ(src.Swizzle, i);
		if (swz == RC_SWIZZLE_UNUSED || !GET_BIT(usemask, i))
			continue;
		negatebase[GET_BIT(src.Negate, i)] |= 1 << i;
	}

	split->NumPhases = 0;

	for(i = 0; i <= 1; ++i) {
		if (!negatebase[i])
			continue;

		split->Phase[split->NumPhases++] = negatebase[i];
	}
}

const struct rc_swizzle_caps r500_swizzle_caps = {
	.IsNative = r500_swizzle_is_native,
	.Split = r500_swizzle_split
};

static char *toswiz(int swiz_val) {
  switch(swiz_val) {
  case 0: return "R";
  case 1: return "G";
  case 2: return "B";
  case 3: return "A";
  case 4: return "0";
  case 5: return "H";
  case 6: return "1";
  case 7: return "U";
  }
  return NULL;
}

static char *toop(int op_val)
{
  char *str = NULL;
  switch (op_val) {
  case 0: str = "MAD"; break;
  case 1: str = "DP3"; break;
  case 2: str = "DP4"; break;
  case 3: str = "D2A"; break;
  case 4: str = "MIN"; break;
  case 5: str = "MAX"; break;
  case 6: str = "Reserved"; break;
  case 7: str = "CND"; break;
  case 8: str = "CMP"; break;
  case 9: str = "FRC"; break;
  case 10: str = "SOP"; break;
  case 11: str = "MDH"; break;
  case 12: str = "MDV"; break;
  }
  return str;
}

static char *to_alpha_op(int op_val)
{
  char *str = NULL;
  switch (op_val) {
  case 0: str = "MAD"; break;
  case 1: str = "DP"; break;
  case 2: str = "MIN"; break;
  case 3: str = "MAX"; break;
  case 4: str = "Reserved"; break;
  case 5: str = "CND"; break;
  case 6: str = "CMP"; break;
  case 7: str = "FRC"; break;
  case 8: str = "EX2"; break;
  case 9: str = "LN2"; break;
  case 10: str = "RCP"; break;
  case 11: str = "RSQ"; break;
  case 12: str = "SIN"; break;
  case 13: str = "COS"; break;
  case 14: str = "MDH"; break;
  case 15: str = "MDV"; break;
  }
  return str;
}

static char *to_mask(int val)
{
  char *str = NULL;
  switch(val) {
  case 0: str = "NONE"; break;
  case 1: str = "R"; break;
  case 2: str = "G"; break;
  case 3: str = "RG"; break;
  case 4: str = "B"; break;
  case 5: str = "RB"; break;
  case 6: str = "GB"; break;
  case 7: str = "RGB"; break;
  case 8: str = "A"; break;
  case 9: str = "AR"; break;
  case 10: str = "AG"; break;
  case 11: str = "ARG"; break;
  case 12: str = "AB"; break;
  case 13: str = "ARB"; break;
  case 14: str = "AGB"; break;
  case 15: str = "ARGB"; break;
  }
  return str;
}

static char *to_texop(int val)
{
  switch(val) {
  case 0: return "NOP";
  case 1: return "LD";
  case 2: return "TEXKILL";
  case 3: return "PROJ";
  case 4: return "LODBIAS";
  case 5: return "LOD";
  case 6: return "DXDY";
  }
  return NULL;
}

void r500FragmentProgramDump(struct radeon_compiler *c, void *user)
{
  struct r300_fragment_program_compiler *compiler = (struct r300_fragment_program_compiler*)c;
  struct r500_fragment_program_code *code = &compiler->code->code.r500;
  int n, i;
  uint32_t inst;
  uint32_t inst0;
  char *str = NULL;
  fprintf(stderr, "R500 Fragment Program:\n--------\n");

  for (n = 0; n < code->inst_end+1; n++) {
    inst0 = inst = code->inst[n].inst0;
    fprintf(stderr,"%d\t0:CMN_INST   0x%08x:", n, inst);
    switch(inst & 0x3) {
    case R500_INST_TYPE_ALU: str = "ALU"; break;
    case R500_INST_TYPE_OUT: str = "OUT"; break;
    case R500_INST_TYPE_FC: str = "FC"; break;
    case R500_INST_TYPE_TEX: str = "TEX"; break;
    }
    fprintf(stderr,"%s %s %s %s %s ", str,
	    inst & R500_INST_TEX_SEM_WAIT ? "TEX_WAIT" : "",
	    inst & R500_INST_LAST ? "LAST" : "",
	    inst & R500_INST_NOP ? "NOP" : "",
	    inst & R500_INST_ALU_WAIT ? "ALU WAIT" : "");
    fprintf(stderr,"wmask: %s omask: %s\n", to_mask((inst >> 11) & 0xf),
	    to_mask((inst >> 15) & 0xf));

    switch(inst0 & 0x3) {
    case R500_INST_TYPE_ALU:
    case R500_INST_TYPE_OUT:
      fprintf(stderr,"\t1:RGB_ADDR   0x%08x:", code->inst[n].inst1);
      inst = code->inst[n].inst1;

      fprintf(stderr,"Addr0: %d%c, Addr1: %d%c, Addr2: %d%c, srcp:%d\n",
	      inst & 0xff, (inst & (1<<8)) ? 'c' : 't',
	      (inst >> 10) & 0xff, (inst & (1<<18)) ? 'c' : 't',
	      (inst >> 20) & 0xff, (inst & (1<<28)) ? 'c' : 't',
	      (inst >> 30));

      fprintf(stderr,"\t2:ALPHA_ADDR 0x%08x:", code->inst[n].inst2);
      inst = code->inst[n].inst2;
      fprintf(stderr,"Addr0: %d%c, Addr1: %d%c, Addr2: %d%c, srcp:%d\n",
	      inst & 0xff, (inst & (1<<8)) ? 'c' : 't',
	      (inst >> 10) & 0xff, (inst & (1<<18)) ? 'c' : 't',
	      (inst >> 20) & 0xff, (inst & (1<<28)) ? 'c' : 't',
	      (inst >> 30));
      fprintf(stderr,"\t3 RGB_INST:  0x%08x:", code->inst[n].inst3);
      inst = code->inst[n].inst3;
      fprintf(stderr,"rgb_A_src:%d %s/%s/%s %d rgb_B_src:%d %s/%s/%s %d targ: %d\n",
	      (inst) & 0x3, toswiz((inst >> 2) & 0x7), toswiz((inst >> 5) & 0x7), toswiz((inst >> 8) & 0x7),
	      (inst >> 11) & 0x3,
	      (inst >> 13) & 0x3, toswiz((inst >> 15) & 0x7), toswiz((inst >> 18) & 0x7), toswiz((inst >> 21) & 0x7),
	      (inst >> 24) & 0x3, (inst >> 29) & 0x3);


      fprintf(stderr,"\t4 ALPHA_INST:0x%08x:", code->inst[n].inst4);
      inst = code->inst[n].inst4;
      fprintf(stderr,"%s dest:%d%s alp_A_src:%d %s %d alp_B_src:%d %s %d targ %d w:%d\n", to_alpha_op(inst & 0xf),
	      (inst >> 4) & 0x7f, inst & (1<<11) ? "(rel)":"",
	      (inst >> 12) & 0x3, toswiz((inst >> 14) & 0x7), (inst >> 17) & 0x3,
	      (inst >> 19) & 0x3, toswiz((inst >> 21) & 0x7), (inst >> 24) & 0x3,
	      (inst >> 29) & 0x3,
	      (inst >> 31) & 0x1);

      fprintf(stderr,"\t5 RGBA_INST: 0x%08x:", code->inst[n].inst5);
      inst = code->inst[n].inst5;
      fprintf(stderr,"%s dest:%d%s rgb_C_src:%d %s/%s/%s %d alp_C_src:%d %s %d\n", toop(inst & 0xf),
	      (inst >> 4) & 0x7f, inst & (1<<11) ? "(rel)":"",
	      (inst >> 12) & 0x3, toswiz((inst >> 14) & 0x7), toswiz((inst >> 17) & 0x7), toswiz((inst >> 20) & 0x7),
	      (inst >> 23) & 0x3,
	      (inst >> 25) & 0x3, toswiz((inst >> 27) & 0x7), (inst >> 30) & 0x3);
      break;
    case R500_INST_TYPE_FC:
      fprintf(stderr, "\t2:FC_INST    0x%08x:", code->inst[n].inst2);
      inst = code->inst[n].inst2;
      /* JUMP_FUNC JUMP_ANY*/
      fprintf(stderr, "0x%02x %1x ", inst >> 8 & 0xff,
          (inst & R500_FC_JUMP_ANY) >> 5);
      
      /* OP */
      switch(inst & 0x7){
      case R500_FC_OP_JUMP:
      	fprintf(stderr, "JUMP");
        break;
      case R500_FC_OP_LOOP:
        fprintf(stderr, "LOOP");
        break;
      case R500_FC_OP_ENDLOOP:
        fprintf(stderr, "ENDLOOP");
        break;
      case R500_FC_OP_REP:
        fprintf(stderr, "REP");
        break;
      case R500_FC_OP_ENDREP:
        fprintf(stderr, "ENDREP");
        break;
      case R500_FC_OP_BREAKLOOP:
        fprintf(stderr, "BREAKLOOP");
        break;
      case R500_FC_OP_BREAKREP:
        fprintf(stderr, "BREAKREP");
	break;
      case R500_FC_OP_CONTINUE:
        fprintf(stderr, "CONTINUE");
        break;
      }
      fprintf(stderr," "); 
      /* A_OP */
      switch(inst & (0x3 << 6)){
      case R500_FC_A_OP_NONE:
        fprintf(stderr, "NONE");
        break;
      case R500_FC_A_OP_POP:
	fprintf(stderr, "POP");
        break;
      case R500_FC_A_OP_PUSH:
        fprintf(stderr, "PUSH");
        break;
      }
      /* B_OP0 B_OP1 */
      for(i=0; i<2; i++){
        fprintf(stderr, " ");
        switch(inst & (0x3 << (24 + (i * 2)))){
        /* R500_FC_B_OP0_NONE 
	 * R500_FC_B_OP1_NONE */
	case 0:
          fprintf(stderr, "NONE");
          break;
        case R500_FC_B_OP0_DECR:
        case R500_FC_B_OP1_DECR:
          fprintf(stderr, "DECR");
          break;
        case R500_FC_B_OP0_INCR:
        case R500_FC_B_OP1_INCR:
          fprintf(stderr, "INCR");
          break;
        }
      }
      /*POP_CNT B_ELSE */
      fprintf(stderr, " %d %1x", (inst >> 16) & 0x1f, (inst & R500_FC_B_ELSE) >> 4);
      inst = code->inst[n].inst3;
      /* JUMP_ADDR */
      fprintf(stderr, " %d", inst >> 16);
      
      if(code->inst[n].inst2 & R500_FC_IGNORE_UNCOVERED){
        fprintf(stderr, " IGN_UNC");
      }
      inst = code->inst[n].inst3;
      fprintf(stderr, "\n\t3:FC_ADDR    0x%08x:", inst);
      fprintf(stderr, "BOOL: 0x%02x, INT: 0x%02x, JUMP_ADDR: %d, JMP_GLBL: %1x\n",
      inst & 0x1f, (inst >> 8) & 0x1f, (inst >> 16) & 0x1ff, inst >> 31); 
      break;
    case R500_INST_TYPE_TEX:
      inst = code->inst[n].inst1;
      fprintf(stderr,"\t1:TEX_INST:  0x%08x: id: %d op:%s, %s, %s %s\n", inst, (inst >> 16) & 0xf,
	      to_texop((inst >> 22) & 0x7), (inst & (1<<25)) ? "ACQ" : "",
	      (inst & (1<<26)) ? "IGNUNC" : "", (inst & (1<<27)) ? "UNSCALED" : "SCALED");
      inst = code->inst[n].inst2;
      fprintf(stderr,"\t2:TEX_ADDR:  0x%08x: src: %d%s %s/%s/%s/%s dst: %d%s %s/%s/%s/%s\n", inst,
	      inst & 127, inst & (1<<7) ? "(rel)" : "",
	      toswiz((inst >> 8) & 0x3), toswiz((inst >> 10) & 0x3),
	      toswiz((inst >> 12) & 0x3), toswiz((inst >> 14) & 0x3),
	      (inst >> 16) & 127, inst & (1<<23) ? "(rel)" : "",
	      toswiz((inst >> 24) & 0x3), toswiz((inst >> 26) & 0x3),
	      toswiz((inst >> 28) & 0x3), toswiz((inst >> 30) & 0x3));

      fprintf(stderr,"\t3:TEX_DXDY:  0x%08x\n", code->inst[n].inst3);
      break;
    }
    fprintf(stderr,"\n");
  }

}
