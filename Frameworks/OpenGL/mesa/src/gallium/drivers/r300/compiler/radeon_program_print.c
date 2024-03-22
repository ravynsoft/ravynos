/*
 * Copyright 2009 Nicolai Hähnle <nhaehnle@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHOR(S) AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE. */

#include "util/u_math.h"
#include "radeon_program.h"

#include <stdio.h>

static const char * textarget_to_string(rc_texture_target target)
{
	switch(target) {
	case RC_TEXTURE_2D_ARRAY: return "2D_ARRAY";
	case RC_TEXTURE_1D_ARRAY: return "1D_ARRAY";
	case RC_TEXTURE_CUBE: return "CUBE";
	case RC_TEXTURE_3D: return "3D";
	case RC_TEXTURE_RECT: return "RECT";
	case RC_TEXTURE_2D: return "2D";
	case RC_TEXTURE_1D: return "1D";
	default: return "BAD_TEXTURE_TARGET";
	}
}

static const char * presubtract_op_to_string(rc_presubtract_op op)
{
	switch(op) {
	case RC_PRESUB_NONE:
		return "NONE";
	case RC_PRESUB_BIAS:
		return "(1 - 2 * src0)";
	case RC_PRESUB_SUB:
		return "(src1 - src0)";
	case RC_PRESUB_ADD:
		return "(src1 + src0)";
	case RC_PRESUB_INV:
		return "(1 - src0)";
	default:
		return "BAD_PRESUBTRACT_OP";
	}
}

static void print_omod_op(FILE * f, rc_omod_op op)
{
	const char * omod_str;

	switch(op) {
	case RC_OMOD_MUL_1:
	case RC_OMOD_DISABLE:
		return;
	case RC_OMOD_MUL_2:
		omod_str = "* 2";
		break;
	case RC_OMOD_MUL_4:
		omod_str = "* 4";
		break;
	case RC_OMOD_MUL_8:
		omod_str = "* 8";
		break;
	case RC_OMOD_DIV_2:
		omod_str = "/ 2";
		break;
	case RC_OMOD_DIV_4:
		omod_str = "/ 4";
		break;
	case RC_OMOD_DIV_8:
		omod_str = "/ 8";
		break;
	default:
		return;
	}
	fprintf(f, " %s", omod_str);
}

static void rc_print_comparefunc(FILE * f, const char * lhs, rc_compare_func func, const char * rhs)
{
	if (func == RC_COMPARE_FUNC_NEVER) {
		fprintf(f, "false");
	} else if (func == RC_COMPARE_FUNC_ALWAYS) {
		fprintf(f, "true");
	} else {
		const char * op;
		switch(func) {
		case RC_COMPARE_FUNC_LESS: op = "<"; break;
		case RC_COMPARE_FUNC_EQUAL: op = "=="; break;
		case RC_COMPARE_FUNC_LEQUAL: op = "<="; break;
		case RC_COMPARE_FUNC_GREATER: op = ">"; break;
		case RC_COMPARE_FUNC_NOTEQUAL: op = "!="; break;
		case RC_COMPARE_FUNC_GEQUAL: op = ">="; break;
		default: op = "???"; break;
		}
		fprintf(f, "%s %s %s", lhs, op, rhs);
	}
}

static void rc_print_inline_float(FILE * f, int index)
{
	int r300_exponent = (index >> 3) & 0xf;
	unsigned r300_mantissa = index & 0x7;
	unsigned float_exponent;
	unsigned real_float;

	r300_exponent -= 7;
	float_exponent = r300_exponent + 127;
	real_float = (r300_mantissa << 20) | (float_exponent << 23);

	fprintf(f, "%f (0x%x)", uif(real_float), index);

}

static void rc_print_register(FILE * f, rc_register_file file, int index, unsigned int reladdr)
{
	if (file == RC_FILE_NONE) {
		fprintf(f, "none");
	} else if (file == RC_FILE_SPECIAL) {
		switch(index) {
		case RC_SPECIAL_ALU_RESULT: fprintf(f, "aluresult"); break;
		default: fprintf(f, "special[%i]", index); break;
		}
	} else if (file == RC_FILE_INLINE) {
		rc_print_inline_float(f, index);
	} else {
		const char * filename;
		switch(file) {
		case RC_FILE_TEMPORARY: filename = "temp"; break;
		case RC_FILE_INPUT: filename = "input"; break;
		case RC_FILE_OUTPUT: filename = "output"; break;
		case RC_FILE_ADDRESS: filename = "addr"; break;
		case RC_FILE_CONSTANT: filename = "const"; break;
		default: filename = "BAD FILE"; break;
		}
		fprintf(f, "%s[%i%s]", filename, index, reladdr ? " + addr[0]" : "");
	}
}

static void rc_print_mask(FILE * f, unsigned int mask)
{
	if (mask & RC_MASK_X) fprintf(f, "x");
	if (mask & RC_MASK_Y) fprintf(f, "y");
	if (mask & RC_MASK_Z) fprintf(f, "z");
	if (mask & RC_MASK_W) fprintf(f, "w");
}

static void rc_print_dst_register(FILE * f, struct rc_dst_register dst)
{
	rc_print_register(f, dst.File, dst.Index, 0);
	if (dst.WriteMask != RC_MASK_XYZW) {
		fprintf(f, ".");
		rc_print_mask(f, dst.WriteMask);
	}
}

static char rc_swizzle_char(unsigned int swz)
{
	switch(swz) {
	case RC_SWIZZLE_X: return 'x';
	case RC_SWIZZLE_Y: return 'y';
	case RC_SWIZZLE_Z: return 'z';
	case RC_SWIZZLE_W: return 'w';
	case RC_SWIZZLE_ZERO: return '0';
	case RC_SWIZZLE_ONE: return '1';
	case RC_SWIZZLE_HALF: return 'H';
	case RC_SWIZZLE_UNUSED: return '_';
	}
	fprintf(stderr, "bad swz: %u\n", swz);
	return '?';
}

static void rc_print_swizzle(FILE * f, unsigned int swizzle, unsigned int negate)
{
	unsigned int comp;
	for(comp = 0; comp < 4; ++comp) {
		rc_swizzle swz = GET_SWZ(swizzle, comp);
		if (GET_BIT(negate, comp))
			fprintf(f, "-");
		fprintf(f, "%c", rc_swizzle_char(swz));
	}
}

static void rc_print_presub_instruction(FILE * f,
					struct rc_presub_instruction inst)
{
	fprintf(f,"(");
	switch(inst.Opcode){
	case RC_PRESUB_BIAS:
		fprintf(f, "1 - 2 * ");
		rc_print_register(f, inst.SrcReg[0].File,
				inst.SrcReg[0].Index,inst.SrcReg[0].RelAddr);
		break;
	case RC_PRESUB_SUB:
		rc_print_register(f, inst.SrcReg[1].File,
				inst.SrcReg[1].Index,inst.SrcReg[1].RelAddr);
		fprintf(f, " - ");
		rc_print_register(f, inst.SrcReg[0].File,
				inst.SrcReg[0].Index,inst.SrcReg[0].RelAddr);
		break;
	case RC_PRESUB_ADD:
		rc_print_register(f, inst.SrcReg[1].File,
				inst.SrcReg[1].Index,inst.SrcReg[1].RelAddr);
		fprintf(f, " + ");
		rc_print_register(f, inst.SrcReg[0].File,
				inst.SrcReg[0].Index,inst.SrcReg[0].RelAddr);
		break;
	case RC_PRESUB_INV:
		fprintf(f, "1 - ");
		rc_print_register(f, inst.SrcReg[0].File,
				inst.SrcReg[0].Index,inst.SrcReg[0].RelAddr);
		break;
	default:
		break;
	}
	fprintf(f, ")");
}

static void rc_print_src_register(FILE * f, struct rc_instruction * inst,
						struct rc_src_register src)
{
	int trivial_negate = (src.Negate == RC_MASK_NONE || src.Negate == RC_MASK_XYZW);

	if (src.Negate == RC_MASK_XYZW)
		fprintf(f, "-");
	if (src.Abs)
		fprintf(f, "|");

	if(src.File == RC_FILE_PRESUB)
		rc_print_presub_instruction(f, inst->U.I.PreSub);
	else
		rc_print_register(f, src.File, src.Index, src.RelAddr);

	if (src.Abs && !trivial_negate)
		fprintf(f, "|");

	if (src.Swizzle != RC_SWIZZLE_XYZW || !trivial_negate) {
		fprintf(f, ".");
		rc_print_swizzle(f, src.Swizzle, trivial_negate ? 0 : src.Negate);
	}

	if (src.Abs && trivial_negate)
		fprintf(f, "|");
}

static unsigned update_branch_depth(rc_opcode opcode, unsigned *branch_depth)
{
	switch (opcode) {
	case RC_OPCODE_IF:
	case RC_OPCODE_BGNLOOP:
		return (*branch_depth)++ * 2;

	case RC_OPCODE_ENDIF:
	case RC_OPCODE_ENDLOOP:
		assert(*branch_depth > 0);
		return --(*branch_depth) * 2;

	case RC_OPCODE_ELSE:
		assert(*branch_depth > 0);
		return (*branch_depth - 1) * 2;

	default:
		return *branch_depth * 2;
	}
}

static void rc_print_normal_instruction(FILE * f, struct rc_instruction * inst, unsigned *branch_depth)
{
	const struct rc_opcode_info * opcode = rc_get_opcode_info(inst->U.I.Opcode);
	unsigned int reg;
	unsigned spaces = update_branch_depth(inst->U.I.Opcode, branch_depth);

	for (unsigned i = 0; i < spaces; i++)
		fprintf(f, " ");

	fprintf(f, "%s", opcode->Name);

	switch(inst->U.I.SaturateMode) {
	case RC_SATURATE_NONE: break;
	case RC_SATURATE_ZERO_ONE: fprintf(f, "_SAT"); break;
	case RC_SATURATE_MINUS_PLUS_ONE: fprintf(f, "_SAT2"); break;
	default: fprintf(f, "_BAD_SAT"); break;
	}

	if (opcode->HasDstReg) {
		fprintf(f, " ");
		rc_print_dst_register(f, inst->U.I.DstReg);
		print_omod_op(f, inst->U.I.Omod);
		if (opcode->NumSrcRegs)
			fprintf(f, ",");
	}

	for(reg = 0; reg < opcode->NumSrcRegs; ++reg) {
		if (reg > 0)
			fprintf(f, ",");
		fprintf(f, " ");
		rc_print_src_register(f, inst, inst->U.I.SrcReg[reg]);
	}

	if (opcode->HasTexture) {
		fprintf(f, ", %s%s[%u]%s%s",
			textarget_to_string(inst->U.I.TexSrcTarget),
			inst->U.I.TexShadow ? "SHADOW" : "",
			inst->U.I.TexSrcUnit,
			inst->U.I.TexSemWait ? " SEM_WAIT" : "",
			inst->U.I.TexSemAcquire ? " SEM_ACQUIRE" : "");
	}

	fprintf(f, ";");

	if (inst->U.I.WriteALUResult) {
		fprintf(f, " [aluresult = (");
		rc_print_comparefunc(f,
			(inst->U.I.WriteALUResult == RC_ALURESULT_X) ? "x" : "w",
			inst->U.I.ALUResultCompare, "0");
		fprintf(f, ")]");
	}

	if (inst->U.I.DstReg.Pred == RC_PRED_SET) {
		fprintf(f, " PRED_SET");
	} else if (inst->U.I.DstReg.Pred == RC_PRED_INV) {
		fprintf(f, " PRED_INV");
	}

	fprintf(f, "\n");
}

static void rc_print_pair_instruction(FILE * f, struct rc_instruction * fullinst, unsigned *branch_depth)
{
	struct rc_pair_instruction * inst = &fullinst->U.P;
	int printedsrc = 0;
	unsigned spaces = update_branch_depth(inst->RGB.Opcode != RC_OPCODE_NOP ?
					      inst->RGB.Opcode : inst->Alpha.Opcode, branch_depth);

	for (unsigned i = 0; i < spaces; i++)
		fprintf(f, " ");

	for(unsigned int src = 0; src < 3; ++src) {
		if (inst->RGB.Src[src].Used) {
			if (printedsrc)
				fprintf(f, ", ");
			fprintf(f, "src%i.xyz = ", src);
			rc_print_register(f, inst->RGB.Src[src].File, inst->RGB.Src[src].Index, 0);
			printedsrc = 1;
		}
		if (inst->Alpha.Src[src].Used) {
			if (printedsrc)
				fprintf(f, ", ");
			fprintf(f, "src%i.w = ", src);
			rc_print_register(f, inst->Alpha.Src[src].File, inst->Alpha.Src[src].Index, 0);
			printedsrc = 1;
		}
	}
	if(inst->RGB.Src[RC_PAIR_PRESUB_SRC].Used) {
		fprintf(f, ", srcp.xyz = %s",
			presubtract_op_to_string(
					inst->RGB.Src[RC_PAIR_PRESUB_SRC].Index));
	}
	if(inst->Alpha.Src[RC_PAIR_PRESUB_SRC].Used) {
		fprintf(f, ", srcp.w = %s",
			presubtract_op_to_string(
					inst->Alpha.Src[RC_PAIR_PRESUB_SRC].Index));
	}
	if (inst->SemWait) {
		fprintf(f, " SEM_WAIT");
	}
	fprintf(f, "\n");

	if (inst->RGB.Opcode != RC_OPCODE_NOP) {
		const struct rc_opcode_info * opcode = rc_get_opcode_info(inst->RGB.Opcode);

		for (unsigned i = 0; i < spaces; i++)
			fprintf(f, " ");

		fprintf(f, "     %s%s", opcode->Name, inst->RGB.Saturate ? "_SAT" : "");
		if (inst->RGB.WriteMask)
			fprintf(f, " temp[%i].%s%s%s", inst->RGB.DestIndex,
				(inst->RGB.WriteMask & 1) ? "x" : "",
				(inst->RGB.WriteMask & 2) ? "y" : "",
				(inst->RGB.WriteMask & 4) ? "z" : "");
		if (inst->RGB.OutputWriteMask)
			fprintf(f, " color[%i].%s%s%s", inst->RGB.Target,
				(inst->RGB.OutputWriteMask & 1) ? "x" : "",
				(inst->RGB.OutputWriteMask & 2) ? "y" : "",
				(inst->RGB.OutputWriteMask & 4) ? "z" : "");
		if (inst->WriteALUResult == RC_ALURESULT_X)
			fprintf(f, " aluresult");

		print_omod_op(f, inst->RGB.Omod);

		for(unsigned int arg = 0; arg < opcode->NumSrcRegs; ++arg) {
			const char* abs = inst->RGB.Arg[arg].Abs ? "|" : "";
			const char* neg = inst->RGB.Arg[arg].Negate ? "-" : "";
			fprintf(f, ", %s%ssrc", neg, abs);
			if(inst->RGB.Arg[arg].Source == RC_PAIR_PRESUB_SRC)
				fprintf(f,"p");
			else
				fprintf(f,"%d", inst->RGB.Arg[arg].Source);
			fprintf(f,".%c%c%c%s",
				rc_swizzle_char(GET_SWZ(inst->RGB.Arg[arg].Swizzle, 0)),
				rc_swizzle_char(GET_SWZ(inst->RGB.Arg[arg].Swizzle, 1)),
				rc_swizzle_char(GET_SWZ(inst->RGB.Arg[arg].Swizzle, 2)),
				abs);
		}
		fprintf(f, "\n");
	}

	if (inst->Alpha.Opcode != RC_OPCODE_NOP) {
		const struct rc_opcode_info * opcode = rc_get_opcode_info(inst->Alpha.Opcode);

		for (unsigned i = 0; i < spaces; i++)
			fprintf(f, " ");

		fprintf(f, "     %s%s", opcode->Name, inst->Alpha.Saturate ? "_SAT" : "");
		if (inst->Alpha.WriteMask)
			fprintf(f, " temp[%i].w", inst->Alpha.DestIndex);
		if (inst->Alpha.OutputWriteMask)
			fprintf(f, " color[%i].w", inst->Alpha.Target);
		if (inst->Alpha.DepthWriteMask)
			fprintf(f, " depth.w");
		if (inst->WriteALUResult == RC_ALURESULT_W)
			fprintf(f, " aluresult");

		print_omod_op(f, inst->Alpha.Omod);

		for(unsigned int arg = 0; arg < opcode->NumSrcRegs; ++arg) {
			const char* abs = inst->Alpha.Arg[arg].Abs ? "|" : "";
			const char* neg = inst->Alpha.Arg[arg].Negate ? "-" : "";
			fprintf(f, ", %s%ssrc", neg, abs);
			if(inst->Alpha.Arg[arg].Source == RC_PAIR_PRESUB_SRC)
				fprintf(f,"p");
			else
				fprintf(f,"%d", inst->Alpha.Arg[arg].Source);
			fprintf(f,".%c%s",
				rc_swizzle_char(GET_SWZ(inst->Alpha.Arg[arg].Swizzle, 0)), abs);
		}
		fprintf(f, "\n");
	}

	if (inst->WriteALUResult) {
		for (unsigned i = 0; i < spaces; i++)
			fprintf(f, " ");

		fprintf(f, "      [aluresult = (");
		rc_print_comparefunc(f, "result", inst->ALUResultCompare, "0");
		fprintf(f, ")]\n");
	}
}

/**
 * Print program to stderr, default options.
 */
void rc_print_program(const struct rc_program *prog)
{
	unsigned int linenum = 0;
	unsigned branch_depth = 0;
	struct rc_instruction *inst;

	fprintf(stderr, "# Radeon Compiler Program\n");

	for(inst = prog->Instructions.Next; inst != &prog->Instructions; inst = inst->Next) {
		fprintf(stderr, "%3d: ", linenum);

		if (inst->Type == RC_INSTRUCTION_PAIR)
			rc_print_pair_instruction(stderr, inst, &branch_depth);
		else
			rc_print_normal_instruction(stderr, inst, &branch_depth);

		linenum++;
	}
}
