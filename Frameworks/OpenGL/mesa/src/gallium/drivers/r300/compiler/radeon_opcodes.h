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

#ifndef RADEON_OPCODES_H
#define RADEON_OPCODES_H

#include <assert.h>

/**
 * Opcodes understood by the Radeon compiler.
 */
typedef enum {
	RC_OPCODE_NOP = 0,
	RC_OPCODE_ILLEGAL_OPCODE,

	/** vec4 instruction: dst.c = src0.c + src1.c; */
	RC_OPCODE_ADD,

	/** special instruction: load address register
	 * dst.x = floor(src.x), where dst must be an address register */
	RC_OPCODE_ARL,

	/** special instruction: load address register with round
	 * dst.x = round(src.x), where dst must be an address register */
	RC_OPCODE_ARR,

	/** vec4 instruction: dst.c = src0.c < 0.0 ? src1.c : src2.c */
	RC_OPCODE_CMP,

	/** vec4 instruction: dst.c = src2.c > 0.5 ? src0.c : src1.c */
	RC_OPCODE_CND,

	/** scalar instruction: dst = cos(src0.x) */
	RC_OPCODE_COS,

	/** special instruction: take vec4 partial derivative in X direction
	 * dst.c = d src0.c / dx */
	RC_OPCODE_DDX,

	/** special instruction: take vec4 partial derivative in Y direction
	 * dst.c = d src0.c / dy */
	RC_OPCODE_DDY,

	/** scalar instruction: dst = src0.x*src1.x + src0.y*src1.y */
	RC_OPCODE_DP2,

	/** scalar instruction: dst = src0.x*src1.x + src0.y*src1.y + src0.z*src1.z */
	RC_OPCODE_DP3,

	/** scalar instruction: dst = src0.x*src1.x + src0.y*src1.y + src0.z*src1.z + src0.w*src1.w */
	RC_OPCODE_DP4,

	/** special instruction, see ARB_fragment_program */
	RC_OPCODE_DST,

	/** scalar instruction: dst = 2**src0.x */
	RC_OPCODE_EX2,

	/** special instruction, see ARB_vertex_program */
	RC_OPCODE_EXP,

	/** vec4 instruction: dst.c = src0.c - floor(src0.c) */
	RC_OPCODE_FRC,

	/** special instruction: stop execution if any component of src0 is negative */
	RC_OPCODE_KIL,

	/** scalar instruction: dst = log_2(src0.x) */
	RC_OPCODE_LG2,

	/** special instruction, see ARB_vertex_program */
	RC_OPCODE_LIT,

	/** special instruction, see ARB_vertex_program */
	RC_OPCODE_LOG,

	/** vec4 instruction: dst.c = src0.c*src1.c + src2.c */
	RC_OPCODE_MAD,

	/** vec4 instruction: dst.c = max(src0.c, src1.c) */
	RC_OPCODE_MAX,

	/** vec4 instruction: dst.c = min(src0.c, src1.c) */
	RC_OPCODE_MIN,

	/** vec4 instruction: dst.c = src0.c */
	RC_OPCODE_MOV,

	/** vec4 instruction: dst.c = src0.c*src1.c */
	RC_OPCODE_MUL,

	/** scalar instruction: dst = src0.x ** src1.x */
	RC_OPCODE_POW,

	/** scalar instruction: dst = 1 / src0.x */
	RC_OPCODE_RCP,

	/** vec4 instruction: dst.c = floor(src0.c + 0.5) */
	RC_OPCODE_ROUND,

	/** scalar instruction: dst = 1 / sqrt(src0.x) */
	RC_OPCODE_RSQ,

	/** vec4 instruction: dst.c = (src0.c == src1.c) ? 1.0 : 0.0 */
	RC_OPCODE_SEQ,

	/** vec4 instruction: dst.c = (src0.c >= src1.c) ? 1.0 : 0.0 */
	RC_OPCODE_SGE,

	/** vec4 instruction: dst.c = (src0.c > src1.c) ? 1.0 : 0.0 */
	RC_OPCODE_SGT,

	/** scalar instruction: dst = sin(src0.x) */
	RC_OPCODE_SIN,

	/** vec4 instruction: dst.c = (src0.c <= src1.c) ? 1.0 : 0.0 */
	RC_OPCODE_SLE,

	/** vec4 instruction: dst.c = (src0.c < src1.c) ? 1.0 : 0.0 */
	RC_OPCODE_SLT,

	/** vec4 instruction: dst.c = (src0.c != src1.c) ? 1.0 : 0.0 */
	RC_OPCODE_SNE,

	/** vec4 instruction: dst.c = src0.c - src1.c */
	RC_OPCODE_SUB,

	RC_OPCODE_TEX,
	RC_OPCODE_TXB,
	RC_OPCODE_TXD,
	RC_OPCODE_TXL,
	RC_OPCODE_TXP,

	/** branch instruction:
	 * If src0.x != 0.0, continue with the next instruction;
	 * otherwise, jump to matching RC_OPCODE_ELSE or RC_OPCODE_ENDIF.
	 */
	RC_OPCODE_IF,

	/** branch instruction: jump to matching RC_OPCODE_ENDIF */
	RC_OPCODE_ELSE,

	/** branch instruction: has no effect */
	RC_OPCODE_ENDIF,
	
	RC_OPCODE_BGNLOOP,

	RC_OPCODE_BRK,

	RC_OPCODE_ENDLOOP,

	RC_OPCODE_CONT,

	/** special instruction, used in R300-R500 fragment program pair instructions
	 * indicates that the result of the alpha operation shall be replicated
	 * across all other channels */
	RC_OPCODE_REPL_ALPHA,

	/** special instruction, used in R300-R500 fragment programs
	 * to indicate the start of a block of texture instructions that
	 * can run simultaneously. */
	RC_OPCODE_BEGIN_TEX,

	/** Stop execution of the shader (GLSL discard) */
	RC_OPCODE_KILP,

	/* Vertex shader CF Instructions */
	RC_ME_PRED_SEQ,
	RC_ME_PRED_SGT,
	RC_ME_PRED_SGE,
	RC_ME_PRED_SNEQ,
	RC_ME_PRED_SET_CLR,
	RC_ME_PRED_SET_INV,
	RC_ME_PRED_SET_POP,
	RC_ME_PRED_SET_RESTORE,

	RC_VE_PRED_SEQ_PUSH,
	RC_VE_PRED_SGT_PUSH,
	RC_VE_PRED_SGE_PUSH,
	RC_VE_PRED_SNEQ_PUSH,

	MAX_RC_OPCODE
} rc_opcode;


struct rc_opcode_info {
	rc_opcode Opcode;
	const char * Name;

	/** true if the instruction reads from a texture.
	 *
	 * \note This is false for the KIL instruction, even though KIL is
	 * a texture instruction from a hardware point of view. */
	unsigned int HasTexture:1;

	unsigned int NumSrcRegs:2;
	unsigned int HasDstReg:1;

	/** true if this instruction affects control flow */
	unsigned int IsFlowControl:1;

	/** true if this is a vector instruction that operates on components in parallel
	 * without any cross-component interaction */
	unsigned int IsComponentwise:1;

	/** true if this instruction sources only its operands X components
	 * to compute one result which is smeared across all output channels */
	unsigned int IsStandardScalar:1;
};

extern const struct rc_opcode_info rc_opcodes[MAX_RC_OPCODE];

static inline const struct rc_opcode_info * rc_get_opcode_info(rc_opcode opcode)
{
	assert((unsigned int)opcode < MAX_RC_OPCODE);
	assert(rc_opcodes[opcode].Opcode == opcode);

	return &rc_opcodes[opcode];
}

struct rc_instruction;

void rc_compute_sources_for_writemask(
		const struct rc_instruction *inst,
		unsigned int writemask,
		unsigned int *srcmasks);

#endif /* RADEON_OPCODES_H */
