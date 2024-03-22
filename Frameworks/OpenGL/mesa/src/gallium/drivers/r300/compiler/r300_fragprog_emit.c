/*
 * Copyright (C) 2005 Ben Skeggs.
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

/**
 * \file
 *
 * Emit the r300_fragment_program_code that can be understood by the hardware.
 * Input is a pre-transformed radeon_program.
 *
 * \author Ben Skeggs <darktama@iinet.net.au>
 *
 * \author Jerome Glisse <j.glisse@gmail.com>
 */

#include "r300_fragprog.h"

#include "r300_reg.h"

#include "radeon_program_pair.h"
#include "r300_fragprog_swizzle.h"

#include "util/compiler.h"


struct r300_emit_state {
	struct r300_fragment_program_compiler * compiler;

	unsigned current_node : 2;
	unsigned node_first_tex : 8;
	unsigned node_first_alu : 8;
	uint32_t node_flags;
};

#define PROG_CODE \
	struct r300_fragment_program_compiler *c = emit->compiler; \
	struct r300_fragment_program_code *code = &c->code->code.r300

#define error(fmt, args...) do {			\
		rc_error(&c->Base, "%s::%s(): " fmt "\n",	\
			__FILE__, __func__, ##args);	\
	} while(0)

static unsigned int get_msbs_alu(unsigned int bits)
{
	return (bits >> 6) & 0x7;
}

/**
 * @param lsbs The number of least significant bits
 */
static unsigned int get_msbs_tex(unsigned int bits, unsigned int lsbs)
{
	return (bits >> lsbs) & 0x15;
}

#define R400_EXT_GET_MSBS(x, lsbs, mask) (((x) >> lsbs) & mask)

/**
 * Mark a temporary register as used.
 */
static void use_temporary(struct r300_fragment_program_code *code, unsigned int index)
{
	if (index > code->pixsize)
		code->pixsize = index;
}

static unsigned int use_source(struct r300_fragment_program_code* code, struct rc_pair_instruction_source src)
{
	if (!src.Used)
		return 0;

	if (src.File == RC_FILE_CONSTANT) {
		return src.Index | (1 << 5);
	} else if (src.File == RC_FILE_TEMPORARY || src.File == RC_FILE_INPUT) {
		use_temporary(code, src.Index);
		return src.Index & 0x1f;
	}

	return 0;
}


static unsigned int translate_rgb_opcode(struct r300_fragment_program_compiler * c, rc_opcode opcode)
{
	switch(opcode) {
	case RC_OPCODE_CMP: return R300_ALU_OUTC_CMP;
	case RC_OPCODE_CND: return R300_ALU_OUTC_CND;
	case RC_OPCODE_DP3: return R300_ALU_OUTC_DP3;
	case RC_OPCODE_DP4: return R300_ALU_OUTC_DP4;
	case RC_OPCODE_FRC: return R300_ALU_OUTC_FRC;
	default:
		error("translate_rgb_opcode: Unknown opcode %s", rc_get_opcode_info(opcode)->Name);
		FALLTHROUGH;
	case RC_OPCODE_NOP:
		FALLTHROUGH;
	case RC_OPCODE_MAD: return R300_ALU_OUTC_MAD;
	case RC_OPCODE_MAX: return R300_ALU_OUTC_MAX;
	case RC_OPCODE_MIN: return R300_ALU_OUTC_MIN;
	case RC_OPCODE_REPL_ALPHA: return R300_ALU_OUTC_REPL_ALPHA;
	}
}

static unsigned int translate_alpha_opcode(struct r300_fragment_program_compiler * c, rc_opcode opcode)
{
	switch(opcode) {
	case RC_OPCODE_CMP: return R300_ALU_OUTA_CMP;
	case RC_OPCODE_CND: return R300_ALU_OUTA_CND;
	case RC_OPCODE_DP3: return R300_ALU_OUTA_DP4;
	case RC_OPCODE_DP4: return R300_ALU_OUTA_DP4;
	case RC_OPCODE_EX2: return R300_ALU_OUTA_EX2;
	case RC_OPCODE_FRC: return R300_ALU_OUTA_FRC;
	case RC_OPCODE_LG2: return R300_ALU_OUTA_LG2;
	default:
		error("translate_rgb_opcode: Unknown opcode %s", rc_get_opcode_info(opcode)->Name);
		FALLTHROUGH;
	case RC_OPCODE_NOP:
		FALLTHROUGH;
	case RC_OPCODE_MAD: return R300_ALU_OUTA_MAD;
	case RC_OPCODE_MAX: return R300_ALU_OUTA_MAX;
	case RC_OPCODE_MIN: return R300_ALU_OUTA_MIN;
	case RC_OPCODE_RCP: return R300_ALU_OUTA_RCP;
	case RC_OPCODE_RSQ: return R300_ALU_OUTA_RSQ;
	}
}

/**
 * Emit one paired ALU instruction.
 */
static int emit_alu(struct r300_emit_state * emit, struct rc_pair_instruction* inst)
{
	int ip;
	int j;
	PROG_CODE;

	if (code->alu.length >= c->Base.max_alu_insts) {
		/* rc_recompute_ips does not give an exact count, because it counts extra stuff
		 * like BEGINTEX, but here it is intended to be only approximative anyway,
		 * just to give some idea how close to the limit we are. */
		rc_error(&c->Base, "Too many ALU instructions used: %u, max: %u.\n",
		         rc_recompute_ips(&c->Base), c->Base.max_alu_insts);
		return 0;
	}

	ip = code->alu.length++;

	code->alu.inst[ip].rgb_inst = translate_rgb_opcode(c, inst->RGB.Opcode);
	code->alu.inst[ip].alpha_inst = translate_alpha_opcode(c, inst->Alpha.Opcode);

	for(j = 0; j < 3; ++j) {
		/* Set the RGB address */
		unsigned int src = use_source(code, inst->RGB.Src[j]);
		unsigned int arg;
		if (inst->RGB.Src[j].Index >= R300_PFS_NUM_TEMP_REGS)
			code->alu.inst[ip].r400_ext_addr |= R400_ADDR_EXT_RGB_MSB_BIT(j);

		code->alu.inst[ip].rgb_addr |= src << (6*j);

		/* Set the Alpha address */
		src = use_source(code, inst->Alpha.Src[j]);
		if (inst->Alpha.Src[j].Index >= R300_PFS_NUM_TEMP_REGS)
			code->alu.inst[ip].r400_ext_addr |= R400_ADDR_EXT_A_MSB_BIT(j);

		code->alu.inst[ip].alpha_addr |= src << (6*j);

		arg = r300FPTranslateRGBSwizzle(inst->RGB.Arg[j].Source, inst->RGB.Arg[j].Swizzle);
		arg |= inst->RGB.Arg[j].Abs << 6;
		arg |= inst->RGB.Arg[j].Negate << 5;
		code->alu.inst[ip].rgb_inst |= arg << (7*j);

		arg = r300FPTranslateAlphaSwizzle(inst->Alpha.Arg[j].Source, inst->Alpha.Arg[j].Swizzle);
		arg |= inst->Alpha.Arg[j].Abs << 6;
		arg |= inst->Alpha.Arg[j].Negate << 5;
		code->alu.inst[ip].alpha_inst |= arg << (7*j);
	}

	/* Presubtract */
	if (inst->RGB.Src[RC_PAIR_PRESUB_SRC].Used) {
		switch(inst->RGB.Src[RC_PAIR_PRESUB_SRC].Index) {
		case RC_PRESUB_BIAS:
			code->alu.inst[ip].rgb_inst |=
						R300_ALU_SRCP_1_MINUS_2_SRC0;
			break;
		case RC_PRESUB_ADD:
			code->alu.inst[ip].rgb_inst |=
						R300_ALU_SRCP_SRC1_PLUS_SRC0;
			break;
		case RC_PRESUB_SUB:
			code->alu.inst[ip].rgb_inst |=
						R300_ALU_SRCP_SRC1_MINUS_SRC0;
			break;
		case RC_PRESUB_INV:
			code->alu.inst[ip].rgb_inst |=
						R300_ALU_SRCP_1_MINUS_SRC0;
			break;
		default:
			break;
		}
	}

	if (inst->Alpha.Src[RC_PAIR_PRESUB_SRC].Used) {
		switch(inst->Alpha.Src[RC_PAIR_PRESUB_SRC].Index) {
		case RC_PRESUB_BIAS:
			code->alu.inst[ip].alpha_inst |=
						R300_ALU_SRCP_1_MINUS_2_SRC0;
			break;
		case RC_PRESUB_ADD:
			code->alu.inst[ip].alpha_inst |=
						R300_ALU_SRCP_SRC1_PLUS_SRC0;
			break;
		case RC_PRESUB_SUB:
			code->alu.inst[ip].alpha_inst |=
						R300_ALU_SRCP_SRC1_MINUS_SRC0;
			break;
		case RC_PRESUB_INV:
			code->alu.inst[ip].alpha_inst |=
						R300_ALU_SRCP_1_MINUS_SRC0;
			break;
		default:
			break;
		}
	}

	if (inst->RGB.Saturate)
		code->alu.inst[ip].rgb_inst |= R300_ALU_OUTC_CLAMP;
	if (inst->Alpha.Saturate)
		code->alu.inst[ip].alpha_inst |= R300_ALU_OUTA_CLAMP;

	if (inst->RGB.WriteMask) {
		use_temporary(code, inst->RGB.DestIndex);
		if (inst->RGB.DestIndex >= R300_PFS_NUM_TEMP_REGS)
			code->alu.inst[ip].r400_ext_addr |= R400_ADDRD_EXT_RGB_MSB_BIT;
		code->alu.inst[ip].rgb_addr |=
			((inst->RGB.DestIndex & 0x1f) << R300_ALU_DSTC_SHIFT) |
			(inst->RGB.WriteMask << R300_ALU_DSTC_REG_MASK_SHIFT);
	}
	if (inst->RGB.OutputWriteMask) {
		code->alu.inst[ip].rgb_addr |=
            (inst->RGB.OutputWriteMask << R300_ALU_DSTC_OUTPUT_MASK_SHIFT) |
            R300_RGB_TARGET(inst->RGB.Target);
		emit->node_flags |= R300_RGBA_OUT;
	}

	if (inst->Alpha.WriteMask) {
		use_temporary(code, inst->Alpha.DestIndex);
		if (inst->Alpha.DestIndex >= R300_PFS_NUM_TEMP_REGS)
			code->alu.inst[ip].r400_ext_addr |= R400_ADDRD_EXT_A_MSB_BIT;
		code->alu.inst[ip].alpha_addr |=
			((inst->Alpha.DestIndex & 0x1f) << R300_ALU_DSTA_SHIFT) |
			R300_ALU_DSTA_REG;
	}
	if (inst->Alpha.OutputWriteMask) {
		code->alu.inst[ip].alpha_addr |= R300_ALU_DSTA_OUTPUT |
            R300_ALPHA_TARGET(inst->Alpha.Target);
		emit->node_flags |= R300_RGBA_OUT;
	}
	if (inst->Alpha.DepthWriteMask) {
		code->alu.inst[ip].alpha_addr |= R300_ALU_DSTA_DEPTH;
		emit->node_flags |= R300_W_OUT;
		c->code->writes_depth = 1;
	}
	if (inst->Nop)
		code->alu.inst[ip].rgb_inst |= R300_ALU_INSERT_NOP;

	/* Handle Output Modifier
	 * According to the r300 docs, there is no RC_OMOD_DISABLE for r300 */
	if (inst->RGB.Omod) {
		if (inst->RGB.Omod == RC_OMOD_DISABLE) {
			rc_error(&c->Base, "RC_OMOD_DISABLE not supported");
		}
		code->alu.inst[ip].rgb_inst |=
			(inst->RGB.Omod << R300_ALU_OUTC_MOD_SHIFT);
	}
	if (inst->Alpha.Omod) {
		if (inst->Alpha.Omod == RC_OMOD_DISABLE) {
			rc_error(&c->Base, "RC_OMOD_DISABLE not supported");
		}
		code->alu.inst[ip].alpha_inst |=
			(inst->Alpha.Omod << R300_ALU_OUTC_MOD_SHIFT);
	}
	return 1;
}


/**
 * Finish the current node without advancing to the next one.
 */
static int finish_node(struct r300_emit_state * emit)
{
	struct r300_fragment_program_compiler * c = emit->compiler;
	struct r300_fragment_program_code *code = &emit->compiler->code->code.r300;
	unsigned alu_offset;
	unsigned alu_end;
	unsigned tex_offset;
	unsigned tex_end;

	unsigned int alu_offset_msbs, alu_end_msbs;

	if (code->alu.length == emit->node_first_alu) {
		/* Generate a single NOP for this node */
		struct rc_pair_instruction inst;
		memset(&inst, 0, sizeof(inst));
		if (!emit_alu(emit, &inst))
			return 0;
	}

	alu_offset = emit->node_first_alu;
	alu_end = code->alu.length - alu_offset - 1;
	tex_offset = emit->node_first_tex;
	tex_end = code->tex.length - tex_offset - 1;

	if (code->tex.length == emit->node_first_tex) {
		if (emit->current_node > 0) {
			error("Node %i has no TEX instructions", emit->current_node);
			return 0;
		}

		tex_end = 0;
	} else {
		if (emit->current_node == 0)
			code->config |= R300_PFS_CNTL_FIRST_NODE_HAS_TEX;
	}

	/* Write the config register.
	 * Note: The order in which the words for each node are written
	 * is not correct here and needs to be fixed up once we're entirely
	 * done
	 *
	 * Also note that the register specification from AMD is slightly
	 * incorrect in its description of this register. */
	code->code_addr[emit->current_node]  =
			((alu_offset << R300_ALU_START_SHIFT)
				& R300_ALU_START_MASK)
			| ((alu_end << R300_ALU_SIZE_SHIFT)
				& R300_ALU_SIZE_MASK)
			| ((tex_offset << R300_TEX_START_SHIFT)
				& R300_TEX_START_MASK)
			| ((tex_end << R300_TEX_SIZE_SHIFT)
				& R300_TEX_SIZE_MASK)
			| emit->node_flags
			| (get_msbs_tex(tex_offset, 5)
				<< R400_TEX_START_MSB_SHIFT)
			| (get_msbs_tex(tex_end, 5)
				<< R400_TEX_SIZE_MSB_SHIFT)
			;

	/* Write r400 extended instruction fields.  These will be ignored on
	 * r300 cards.  */
	alu_offset_msbs = get_msbs_alu(alu_offset);
	alu_end_msbs = get_msbs_alu(alu_end);
	switch(emit->current_node) {
	case 0:
		code->r400_code_offset_ext |=
			alu_offset_msbs << R400_ALU_START3_MSB_SHIFT
			| alu_end_msbs << R400_ALU_SIZE3_MSB_SHIFT;
		break;
	case 1:
		code->r400_code_offset_ext |=
			alu_offset_msbs << R400_ALU_START2_MSB_SHIFT
			| alu_end_msbs << R400_ALU_SIZE2_MSB_SHIFT;
		break;
	case 2:
		code->r400_code_offset_ext |=
			alu_offset_msbs << R400_ALU_START1_MSB_SHIFT
			| alu_end_msbs << R400_ALU_SIZE1_MSB_SHIFT;
		break;
	case 3:
		code->r400_code_offset_ext |=
			alu_offset_msbs << R400_ALU_START0_MSB_SHIFT
			| alu_end_msbs << R400_ALU_SIZE0_MSB_SHIFT;
		break;
	}
	return 1;
}


/**
 * Begin a block of texture instructions.
 * Create the necessary indirection.
 */
static int begin_tex(struct r300_emit_state * emit)
{
	PROG_CODE;

	if (code->alu.length == emit->node_first_alu &&
	    code->tex.length == emit->node_first_tex) {
		return 1;
	}

	if (emit->current_node == 3) {
		error("Too many texture indirections");
		return 0;
	}

	if (!finish_node(emit))
		return 0;

	emit->current_node++;
	emit->node_first_tex = code->tex.length;
	emit->node_first_alu = code->alu.length;
	emit->node_flags = 0;
	return 1;
}


static int emit_tex(struct r300_emit_state * emit, struct rc_instruction * inst)
{
	unsigned int unit;
	unsigned int dest;
	unsigned int opcode;
	PROG_CODE;

	if (code->tex.length >= emit->compiler->Base.max_tex_insts) {
		error("Too many TEX instructions");
		return 0;
	}

	unit = inst->U.I.TexSrcUnit;
	dest = inst->U.I.DstReg.Index;

	switch(inst->U.I.Opcode) {
	case RC_OPCODE_KIL: opcode = R300_TEX_OP_KIL; break;
	case RC_OPCODE_TEX: opcode = R300_TEX_OP_LD; break;
	case RC_OPCODE_TXB: opcode = R300_TEX_OP_TXB; break;
	case RC_OPCODE_TXP: opcode = R300_TEX_OP_TXP; break;
	default:
		error("Unknown texture opcode %s", rc_get_opcode_info(inst->U.I.Opcode)->Name);
		return 0;
	}

	if (inst->U.I.Opcode == RC_OPCODE_KIL) {
		unit = 0;
		dest = 0;
	} else {
		use_temporary(code, dest);
	}

	use_temporary(code, inst->U.I.SrcReg[0].Index);

	code->tex.inst[code->tex.length++] =
		((inst->U.I.SrcReg[0].Index << R300_SRC_ADDR_SHIFT)
			& R300_SRC_ADDR_MASK)
		| ((dest << R300_DST_ADDR_SHIFT)
			& R300_DST_ADDR_MASK)
		| (unit << R300_TEX_ID_SHIFT)
		| (opcode << R300_TEX_INST_SHIFT)
		| (inst->U.I.SrcReg[0].Index >= R300_PFS_NUM_TEMP_REGS ?
			R400_SRC_ADDR_EXT_BIT : 0)
		| (dest >= R300_PFS_NUM_TEMP_REGS ?
			R400_DST_ADDR_EXT_BIT : 0)
		;
	return 1;
}


/**
 * Final compilation step: Turn the intermediate radeon_program into
 * machine-readable instructions.
 */
void r300BuildFragmentProgramHwCode(struct radeon_compiler *c, void *user)
{
	struct r300_fragment_program_compiler *compiler = (struct r300_fragment_program_compiler*)c;
	struct r300_emit_state emit;
	struct r300_fragment_program_code *code = &compiler->code->code.r300;
	unsigned int tex_end;

	memset(&emit, 0, sizeof(emit));
	emit.compiler = compiler;

	memset(code, 0, sizeof(struct r300_fragment_program_code));

	for(struct rc_instruction * inst = compiler->Base.Program.Instructions.Next;
	    inst != &compiler->Base.Program.Instructions && !compiler->Base.Error;
	    inst = inst->Next) {
		if (inst->Type == RC_INSTRUCTION_NORMAL) {
			if (inst->U.I.Opcode == RC_OPCODE_BEGIN_TEX) {
				begin_tex(&emit);
				continue;
			}

			emit_tex(&emit, inst);
		} else {
			emit_alu(&emit, &inst->U.P);
		}
	}

	if (code->pixsize >= compiler->Base.max_temp_regs)
		rc_error(&compiler->Base, "Too many hardware temporaries used.\n");

	if (compiler->Base.Error)
		return;

	/* Finish the program */
	finish_node(&emit);

	code->config |= emit.current_node; /* FIRST_NODE_HAS_TEX set by finish_node */

	/* Set r400 extended instruction fields.  These values will be ignored
	 * on r300 cards. */
	code->r400_code_offset_ext |=
		(get_msbs_alu(0)
				<< R400_ALU_OFFSET_MSB_SHIFT)
		| (get_msbs_alu(code->alu.length - 1)
				<< R400_ALU_SIZE_MSB_SHIFT);

	tex_end = code->tex.length ? code->tex.length - 1 : 0;
	code->code_offset =
		((0 << R300_PFS_CNTL_ALU_OFFSET_SHIFT)
			& R300_PFS_CNTL_ALU_OFFSET_MASK)
		| (((code->alu.length - 1) << R300_PFS_CNTL_ALU_END_SHIFT)
			& R300_PFS_CNTL_ALU_END_MASK)
		| ((0 << R300_PFS_CNTL_TEX_OFFSET_SHIFT)
			& R300_PFS_CNTL_TEX_OFFSET_MASK)
		| ((tex_end << R300_PFS_CNTL_TEX_END_SHIFT)
			& R300_PFS_CNTL_TEX_END_MASK)
		| (get_msbs_tex(0, 5) << R400_TEX_START_MSB_SHIFT)
		| (get_msbs_tex(tex_end, 6) << R400_TEX_SIZE_MSB_SHIFT)
		;

	if (emit.current_node < 3) {
		int shift = 3 - emit.current_node;
		int i;
		for(i = emit.current_node; i >= 0; --i)
			code->code_addr[shift + i] = code->code_addr[i];
		for(i = 0; i < shift; ++i)
			code->code_addr[i] = 0;
	}

	if (code->pixsize >= R300_PFS_NUM_TEMP_REGS
	    || code->alu.length > R300_PFS_MAX_ALU_INST
	    || code->tex.length > R300_PFS_MAX_TEX_INST) {

		code->r390_mode = 1;
	}
}
