/*
 * Copyright (C) 2009 Nicolai Haehnle.
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

#include "radeon_program_pair.h"

#include "radeon_compiler.h"
#include "radeon_compiler_util.h"

#include "util/compiler.h"


/**
 * Finally rewrite ADD, MOV, MUL as the appropriate native instruction
 * and reverse the order of arguments for CMP.
 */
static void final_rewrite(struct rc_sub_instruction *inst)
{
	struct rc_src_register tmp;

	switch(inst->Opcode) {
	case RC_OPCODE_ADD:
		inst->SrcReg[2] = inst->SrcReg[1];
		inst->SrcReg[1].File = RC_FILE_NONE;
		inst->SrcReg[1].Swizzle = RC_SWIZZLE_1111;
		inst->SrcReg[1].Negate = RC_MASK_NONE;
		inst->Opcode = RC_OPCODE_MAD;
		break;
	case RC_OPCODE_CMP:
		tmp = inst->SrcReg[2];
		inst->SrcReg[2] = inst->SrcReg[0];
		inst->SrcReg[0] = tmp;
		break;
	case RC_OPCODE_MOV:
		/* AMD say we should use CMP.
		 * However, when we transform
		 *  KIL -r0;
		 * into
		 *  CMP tmp, -r0, -r0, 0;
		 *  KIL tmp;
		 * we get incorrect behaviour on R500 when r0 == 0.0.
		 * It appears that the R500 KIL hardware treats -0.0 as less
		 * than zero.
		 */
		inst->SrcReg[1].File = RC_FILE_NONE;
		inst->SrcReg[1].Swizzle = RC_SWIZZLE_1111;
		inst->SrcReg[2].File = RC_FILE_NONE;
		inst->SrcReg[2].Swizzle = RC_SWIZZLE_0000;
		inst->Opcode = RC_OPCODE_MAD;
		break;
	case RC_OPCODE_MUL:
		inst->SrcReg[2].File = RC_FILE_NONE;
		inst->SrcReg[2].Swizzle = RC_SWIZZLE_0000;
		inst->Opcode = RC_OPCODE_MAD;
		break;
	default:
		/* nothing to do */
		break;
	}
}


/**
 * Classify an instruction according to which ALUs etc. it needs
 */
static void classify_instruction(struct rc_sub_instruction * inst,
	int * needrgb, int * needalpha, int * istranscendent)
{
	*needrgb = (inst->DstReg.WriteMask & RC_MASK_XYZ) ? 1 : 0;
	*needalpha = (inst->DstReg.WriteMask & RC_MASK_W) ? 1 : 0;
	*istranscendent = 0;

	if (inst->WriteALUResult == RC_ALURESULT_X)
		*needrgb = 1;
	else if (inst->WriteALUResult == RC_ALURESULT_W)
		*needalpha = 1;

	switch(inst->Opcode) {
	case RC_OPCODE_ADD:
	case RC_OPCODE_CMP:
	case RC_OPCODE_CND:
	case RC_OPCODE_DDX:
	case RC_OPCODE_DDY:
	case RC_OPCODE_FRC:
	case RC_OPCODE_MAD:
	case RC_OPCODE_MAX:
	case RC_OPCODE_MIN:
	case RC_OPCODE_MOV:
	case RC_OPCODE_MUL:
		break;
	case RC_OPCODE_COS:
	case RC_OPCODE_EX2:
	case RC_OPCODE_LG2:
	case RC_OPCODE_RCP:
	case RC_OPCODE_RSQ:
	case RC_OPCODE_SIN:
		*istranscendent = 1;
		*needalpha = 1;
		break;
	case RC_OPCODE_DP4:
		*needalpha = 1;
		FALLTHROUGH;
	case RC_OPCODE_DP3:
		*needrgb = 1;
		break;
	default:
		break;
	}
}

static void src_uses(struct rc_src_register src, unsigned int * rgb,
							unsigned int * alpha)
{
	int j;
	for(j = 0; j < 4; ++j) {
		unsigned int swz = GET_SWZ(src.Swizzle, j);
		if (swz < 3)
			*rgb = 1;
		else if (swz < 4)
			*alpha = 1;
	}
}

/**
 * Fill the given ALU instruction's opcodes and source operands into the given pair,
 * if possible.
 */
static void set_pair_instruction(struct r300_fragment_program_compiler *c,
	struct rc_pair_instruction * pair,
	struct rc_sub_instruction * inst)
{
	int needrgb, needalpha, istranscendent;
	const struct rc_opcode_info * opcode;
	int i;

	memset(pair, 0, sizeof(struct rc_pair_instruction));

	classify_instruction(inst, &needrgb, &needalpha, &istranscendent);

	if (needrgb) {
		if (istranscendent)
			pair->RGB.Opcode = RC_OPCODE_REPL_ALPHA;
		else
			pair->RGB.Opcode = inst->Opcode;
		if (inst->SaturateMode == RC_SATURATE_ZERO_ONE)
			pair->RGB.Saturate = 1;
	}
	if (needalpha) {
		pair->Alpha.Opcode = inst->Opcode;
		if (inst->SaturateMode == RC_SATURATE_ZERO_ONE)
			pair->Alpha.Saturate = 1;
	}

	opcode = rc_get_opcode_info(inst->Opcode);

	/* Presubtract handling:
	 * We need to make sure that the values used by the presubtract
	 * operation end up in src0 or src1. */
	if(inst->PreSub.Opcode != RC_PRESUB_NONE) {
		/* rc_pair_alloc_source() will fill in data for
		 * pair->{RGB,ALPHA}.Src[RC_PAIR_PRESUB_SRC] */
		int j;
		for(j = 0; j < 3; j++) {
			int src_regs;
			if(inst->SrcReg[j].File != RC_FILE_PRESUB)
				continue;

			src_regs = rc_presubtract_src_reg_count(
							inst->PreSub.Opcode);
			for(i = 0; i < src_regs; i++) {
				unsigned int rgb = 0;
				unsigned int alpha = 0;
				src_uses(inst->SrcReg[j], &rgb, &alpha);
				if(rgb) {
					pair->RGB.Src[i].File =
						inst->PreSub.SrcReg[i].File;
					pair->RGB.Src[i].Index =
						inst->PreSub.SrcReg[i].Index;
					pair->RGB.Src[i].Used = 1;
				}
				if(alpha) {
					pair->Alpha.Src[i].File =
						inst->PreSub.SrcReg[i].File;
					pair->Alpha.Src[i].Index =
						inst->PreSub.SrcReg[i].Index;
					pair->Alpha.Src[i].Used = 1;
				}
			}
		}
	}

	for(i = 0; i < opcode->NumSrcRegs; ++i) {
		int source;
		if (needrgb && !istranscendent) {
			unsigned int srcrgb = 0;
			unsigned int srcalpha = 0;
			unsigned int srcmask = 0;
			int j;
			/* We don't care about the alpha channel here.  We only
			 * want the part of the swizzle that writes to rgb,
			 * since we are creating an rgb instruction. */
			for(j = 0; j < 3; ++j) {
				unsigned int swz = GET_SWZ(inst->SrcReg[i].Swizzle, j);

				if (swz < RC_SWIZZLE_W)
					srcrgb = 1;
				else if (swz == RC_SWIZZLE_W)
					srcalpha = 1;

				/* We check for ZERO here as well because otherwise the zero
				 * sign (which doesn't matter and we already ignore it previously
				 * when checking for valid swizzle) could mess up the final negate sign.
				 * Example problematic pattern where this would be produced is:
				 *   CONST[1] FLT32 {   0.0000,     0.0000,    -4.0000,     0.0000}
				 *   ADD temp[0].xyz, const[0].xyz_, -const[1].z00_;
				 *
				 * after inline literals would become:
				 *   ADD temp[0].xyz, const[0].xyz_, 4.000000 (0x48).w-0-0-_;
				 *
				 * and after pair translate:
				 *   src0.xyz = const[0], src0.w = 4.000000 (0x48)
				 *   MAD temp[0].xyz, src0.xyz, src0.111, src0.w00
				 *
				 * Without the zero check there would be -src0.w00.
				 */
				if (swz < RC_SWIZZLE_UNUSED && swz != RC_SWIZZLE_ZERO)
					srcmask |= 1 << j;
			}
			source = rc_pair_alloc_source(pair, srcrgb, srcalpha,
							inst->SrcReg[i].File, inst->SrcReg[i].Index);
			if (source < 0) {
				rc_error(&c->Base, "Failed to translate "
							"rgb instruction.\n");
				return;
			}
			pair->RGB.Arg[i].Source = source;
			pair->RGB.Arg[i].Swizzle =
				rc_init_swizzle(inst->SrcReg[i].Swizzle, 3);
			pair->RGB.Arg[i].Abs = inst->SrcReg[i].Abs;
			pair->RGB.Arg[i].Negate = !!(srcmask & inst->SrcReg[i].Negate & (RC_MASK_X | RC_MASK_Y | RC_MASK_Z));
		}
		if (needalpha) {
			unsigned int srcrgb = 0;
			unsigned int srcalpha = 0;
			unsigned int swz;
			if (istranscendent) {
				swz = rc_get_scalar_src_swz(inst->SrcReg[i].Swizzle);
			} else {
				swz = GET_SWZ(inst->SrcReg[i].Swizzle, 3);
			}

			if (swz < 3)
				srcrgb = 1;
			else if (swz < 4)
				srcalpha = 1;
			source = rc_pair_alloc_source(pair, srcrgb, srcalpha,
							inst->SrcReg[i].File, inst->SrcReg[i].Index);
			if (source < 0) {
				rc_error(&c->Base, "Failed to translate "
							"alpha instruction.\n");
				return;
			}
			pair->Alpha.Arg[i].Source = source;
			pair->Alpha.Arg[i].Swizzle = rc_init_swizzle(swz, 1);
			pair->Alpha.Arg[i].Abs = inst->SrcReg[i].Abs;

			if (istranscendent) {
				pair->Alpha.Arg[i].Negate =
					!!(inst->SrcReg[i].Negate &
							inst->DstReg.WriteMask);
			} else {
				pair->Alpha.Arg[i].Negate =
					!!(inst->SrcReg[i].Negate & RC_MASK_W);
			}
		}
	}

	/* Destination handling */
	if (inst->DstReg.File == RC_FILE_OUTPUT) {
        if (inst->DstReg.Index == c->OutputDepth) {
            pair->Alpha.DepthWriteMask |= GET_BIT(inst->DstReg.WriteMask, 3);
        } else {
            for (i = 0; i < 4; i++) {
                if (inst->DstReg.Index == c->OutputColor[i]) {
                    pair->RGB.Target = i;
                    pair->Alpha.Target = i;
                    pair->RGB.OutputWriteMask |=
                        inst->DstReg.WriteMask & RC_MASK_XYZ;
                    pair->Alpha.OutputWriteMask |=
                        GET_BIT(inst->DstReg.WriteMask, 3);
                    break;
                }
            }
        }
	} else {
		if (needrgb) {
			pair->RGB.DestIndex = inst->DstReg.Index;
			pair->RGB.WriteMask |= inst->DstReg.WriteMask & RC_MASK_XYZ;
		}

		if (needalpha) {
			pair->Alpha.WriteMask |= (GET_BIT(inst->DstReg.WriteMask, 3) << 3);
			if (pair->Alpha.WriteMask) {
				pair->Alpha.DestIndex = inst->DstReg.Index;
			}
		}
	}

	if (needrgb) {
		pair->RGB.Omod = inst->Omod;
	}
	if (needalpha) {
		pair->Alpha.Omod = inst->Omod;
	}

	if (inst->WriteALUResult) {
		pair->WriteALUResult = inst->WriteALUResult;
		pair->ALUResultCompare = inst->ALUResultCompare;
	}
}


static void check_opcode_support(struct r300_fragment_program_compiler *c,
				 struct rc_sub_instruction *inst)
{
	const struct rc_opcode_info * opcode = rc_get_opcode_info(inst->Opcode);

	if (opcode->HasDstReg) {
		if (inst->SaturateMode == RC_SATURATE_MINUS_PLUS_ONE) {
			rc_error(&c->Base, "Fragment program does not support signed Saturate.\n");
			return;
		}
	}

	for (unsigned i = 0; i < opcode->NumSrcRegs; i++) {
		if (inst->SrcReg[i].RelAddr) {
			rc_error(&c->Base, "Fragment program does not support relative addressing "
				 " of source operands.\n");
			return;
		}
	}
}


/**
 * Translate all ALU instructions into corresponding pair instructions,
 * performing no other changes.
 */
void rc_pair_translate(struct radeon_compiler *cc, void *user)
{
	struct r300_fragment_program_compiler *c = (struct r300_fragment_program_compiler*)cc;

	for(struct rc_instruction * inst = c->Base.Program.Instructions.Next;
	    inst != &c->Base.Program.Instructions;
	    inst = inst->Next) {
		const struct rc_opcode_info * opcode;
		struct rc_sub_instruction copy;

		if (inst->Type != RC_INSTRUCTION_NORMAL)
			continue;

		opcode = rc_get_opcode_info(inst->U.I.Opcode);

		if (opcode->HasTexture || opcode->IsFlowControl || opcode->Opcode == RC_OPCODE_KIL)
			continue;

		copy = inst->U.I;

		check_opcode_support(c, &copy);

		final_rewrite(&copy);
		inst->Type = RC_INSTRUCTION_PAIR;
		set_pair_instruction(c, &inst->U.P, &copy);
	}
}
