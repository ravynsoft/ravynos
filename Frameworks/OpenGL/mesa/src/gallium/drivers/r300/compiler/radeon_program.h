/*
 * Copyright (C) 2008 Nicolai Haehnle.
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

#ifndef __RADEON_PROGRAM_H_
#define __RADEON_PROGRAM_H_

#include <stdint.h>
#include <string.h>

#include "radeon_opcodes.h"
#include "radeon_code.h"
#include "radeon_program_constants.h"
#include "radeon_program_pair.h"

struct radeon_compiler;

struct rc_src_register {
	unsigned int File:4;

	/** Negative values may be used for relative addressing. */
	unsigned int Index:RC_REGISTER_INDEX_BITS;
	unsigned int RelAddr:1;

	unsigned int Swizzle:12;

	/** Take the component-wise absolute value */
	unsigned int Abs:1;

	/** Post-Abs negation. */
	unsigned int Negate:4;
};

struct rc_dst_register {
	unsigned int File:3;
	unsigned int Index:RC_REGISTER_INDEX_BITS;
	unsigned int WriteMask:4;
	unsigned int Pred:2;
};

struct rc_presub_instruction {
	rc_presubtract_op Opcode;
	struct rc_src_register SrcReg[2];
};

/**
 * Instructions are maintained by the compiler in a doubly linked list
 * of these structures.
 *
 * This instruction format is intended to be expanded for hardware-specific
 * trickery. At different stages of compilation, a different set of
 * instruction types may be valid.
 */
struct rc_sub_instruction {
	struct rc_src_register SrcReg[3];
	struct rc_dst_register DstReg;

	/**
	 * Opcode of this instruction, according to \ref rc_opcode enums.
	 */
	unsigned int Opcode:8;

	/**
	 * Saturate each value of the result to the range [0,1] or [-1,1],
	 * according to \ref rc_saturate_mode enums.
	 */
	unsigned int SaturateMode:2;

	/**
	 * Writing to the special register RC_SPECIAL_ALU_RESULT
	 */
	/*@{*/
	unsigned int WriteALUResult:2;
	unsigned int ALUResultCompare:3;
	/*@}*/

	/**
	 * \name Extra fields for TEX, TXB, TXD, TXL, TXP instructions.
	 */
	/*@{*/
	/** Source texture unit. */
	unsigned int TexSrcUnit:5;

	/** Source texture target, one of the \ref rc_texture_target enums */
	unsigned int TexSrcTarget:3;

	/** True if tex instruction should do shadow comparison */
	unsigned int TexShadow:1;

	/**/
	unsigned int TexSemWait:1;
	unsigned int TexSemAcquire:1;

	/**R500 Only.  How to swizzle the result of a TEX lookup*/
	unsigned int TexSwizzle:12;
	/*@}*/

	/** This holds information about the presubtract operation used by
	 * this instruction. */
	struct rc_presub_instruction PreSub;

	rc_omod_op Omod;
};

typedef enum {
	RC_INSTRUCTION_NORMAL = 0,
	RC_INSTRUCTION_PAIR
} rc_instruction_type;

struct rc_instruction {
	struct rc_instruction * Prev;
	struct rc_instruction * Next;

	rc_instruction_type Type;
	union {
		struct rc_sub_instruction I;
		struct rc_pair_instruction P;
	} U;

	/**
	 * Warning: IPs are not stable. If you want to use them,
	 * you need to recompute them at the beginning of each pass
	 * using \ref rc_recompute_ips
	 */
	unsigned int IP;
};

struct rc_program {
	/**
	 * Instructions.Next points to the first instruction,
	 * Instructions.Prev points to the last instruction.
	 */
	struct rc_instruction Instructions;

	/* Long term, we should probably remove InputsRead & OutputsWritten,
	 * since updating dependent state can be fragile, and they aren't
	 * actually used very often. */
	uint32_t InputsRead;
	uint32_t OutputsWritten;
	uint32_t ShadowSamplers; /**< Texture units used for shadow sampling. */

	struct rc_constant_list Constants;
};

/**
 * A transformation that can be passed to \ref rc_local_transform.
 *
 * The function will be called once for each instruction.
 * It has to either emit the appropriate transformed code for the instruction
 * and return true, or return false if it doesn't understand the
 * instruction.
 *
 * The function gets passed the userData as last parameter.
 */
struct radeon_program_transformation {
	int (*function)(
		struct radeon_compiler*,
		struct rc_instruction*,
		void*);
	void *userData;
};

void rc_local_transform(
	struct radeon_compiler *c,
	void *user);

void rc_get_used_temporaries(
	struct radeon_compiler * c,
	unsigned char * used,
	unsigned int used_length);

int rc_find_free_temporary_list(
	struct radeon_compiler * c,
	unsigned char * used,
	unsigned int used_length,
	unsigned int mask);

unsigned int rc_find_free_temporary(struct radeon_compiler * c);

struct rc_instruction *rc_alloc_instruction(struct radeon_compiler * c);
struct rc_instruction *rc_insert_new_instruction(struct radeon_compiler * c, struct rc_instruction * after);
void rc_insert_instruction(struct rc_instruction * after, struct rc_instruction * inst);
void rc_remove_instruction(struct rc_instruction * inst);

unsigned int rc_recompute_ips(struct radeon_compiler * c);

void rc_print_program(const struct rc_program *prog);

rc_swizzle rc_mask_to_swizzle(unsigned int mask);
#endif
