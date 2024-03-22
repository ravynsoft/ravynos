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

#include "radeon_compiler.h"

#include <stdio.h>

#include "radeon_compiler_util.h"
#include "radeon_dataflow.h"
#include "radeon_program_alu.h"
#include "radeon_program_tex.h"
#include "radeon_rename_regs.h"
#include "radeon_remove_constants.h"
#include "radeon_variable.h"
#include "radeon_list.h"
#include "r300_fragprog.h"
#include "r300_fragprog_swizzle.h"
#include "r500_fragprog.h"


static void rc_rewrite_depth_out(struct radeon_compiler *cc, void *user)
{
	struct r300_fragment_program_compiler *c = (struct r300_fragment_program_compiler*)cc;
	struct rc_instruction *rci;

	for (rci = c->Base.Program.Instructions.Next; rci != &c->Base.Program.Instructions; rci = rci->Next) {
		struct rc_sub_instruction * inst = &rci->U.I;
		unsigned i;
		const struct rc_opcode_info *info = rc_get_opcode_info(inst->Opcode);

		if (inst->DstReg.File != RC_FILE_OUTPUT || inst->DstReg.Index != c->OutputDepth)
			continue;

		if (inst->DstReg.WriteMask & RC_MASK_Z) {
			inst->DstReg.WriteMask = RC_MASK_W;
		} else {
			inst->DstReg.WriteMask = 0;
			continue;
		}

		if (!info->IsComponentwise) {
			continue;
		}

		for (i = 0; i < info->NumSrcRegs; i++) {
			inst->SrcReg[i] = lmul_swizzle(RC_SWIZZLE_ZZZZ, inst->SrcReg[i]);
		}
	}
}

/**
 * This function will try to convert rgb instructions into alpha instructions
 * and vice versa. While this is already attempted during the pair scheduling,
 * it is much simpler to do it before pair conversion, so do it here at least for
 * the simple cases.
 *
 * Currently only math opcodes writing to rgb (and with no friends) are
 * converted to alpha.
 *
 * This function assumes all the instructions are still of type
 * RC_INSTRUCTION_NORMAL, the conversion is much simpler.
 *
 * Beware that this needs to be also called before doing presubtract, because
 * rc_get_variables can't get properly readers for normal instructions if presubtract
 * is present (it works fine for pair instructions).
 */
static void rc_convert_rgb_alpha(struct radeon_compiler *c, void *user)
{
	struct rc_list * variables;
	struct rc_list * var_ptr;

	variables = rc_get_variables(c);

	for (var_ptr = variables; var_ptr; var_ptr = var_ptr->Next) {
		struct rc_variable * var = var_ptr->Item;

		if (var->Inst->U.I.DstReg.File != RC_FILE_TEMPORARY) {
			continue;
		}

		/* Only rewrite scalar opcodes that are used separatelly for now. */
		if (var->Friend)
			continue;

		const struct rc_opcode_info * opcode = rc_get_opcode_info(var->Inst->U.I.Opcode);
		if (opcode->IsStandardScalar && var->Dst.WriteMask != RC_MASK_W) {
			unsigned index = rc_find_free_temporary(c);
			rc_variable_change_dst(var, index, RC_MASK_W);
		}
	}
}

void r3xx_compile_fragment_program(struct r300_fragment_program_compiler* c)
{
	int is_r500 = c->Base.is_r500;
	int opt = !c->Base.disable_optimizations;
	int alpha2one = c->state.alpha_to_one;

	/* Lists of instruction transformations. */
	struct radeon_program_transformation force_alpha_to_one[] = {
		{ &rc_force_output_alpha_to_one, c },
		{ NULL, NULL }
	};

	struct radeon_program_transformation rewrite_tex[] = {
		{ &radeonTransformTEX, c },
		{ NULL, NULL }
	};

	struct radeon_program_transformation native_rewrite_r500[] = {
		{ &radeonTransformALU, NULL },
		{ &radeonTransformDeriv, NULL },
		{ NULL, NULL }
	};

	struct radeon_program_transformation native_rewrite_r300[] = {
		{ &radeonTransformALU, NULL },
		{ &radeonStubDeriv, NULL },
		{ NULL, NULL }
	};

	/* List of compiler passes. */
	struct radeon_compiler_pass fs_list[] = {
		/* NAME				DUMP PREDICATE	FUNCTION			PARAM */
		{"rewrite depth out",		1, 1,		rc_rewrite_depth_out,		NULL},
		{"force alpha to one",		1, alpha2one,	rc_local_transform,		force_alpha_to_one},
		{"transform TEX",		1, 1,		rc_local_transform,		rewrite_tex},
		{"transform IF",		1, is_r500,	r500_transform_IF,		NULL},
		{"native rewrite",		1, is_r500,	rc_local_transform,		native_rewrite_r500},
		{"native rewrite",		1, !is_r500,	rc_local_transform,		native_rewrite_r300},
		{"deadcode",			1, opt,		rc_dataflow_deadcode,		NULL},
		{"convert rgb<->alpha",		1, opt,		rc_convert_rgb_alpha,		NULL},
		{"register rename",		1, !is_r500 || opt,		rc_rename_regs,			NULL},
		{"dataflow optimize",		1, opt,		rc_optimize,			NULL},
		{"inline literals",		1, is_r500 && opt,		rc_inline_literals,			NULL},
		{"dataflow swizzles",		1, 1,		rc_dataflow_swizzles,		NULL},
		{"dead constants",		1, 1,		rc_remove_unused_constants,	&c->code->constants_remap_table},
		{"pair translate",		1, 1,		rc_pair_translate,		NULL},
		{"pair scheduling",		1, 1,		rc_pair_schedule,		&opt},
		{"dead sources",		1, 1,		rc_pair_remove_dead_sources, NULL},
		{"register allocation",		1, 1,		rc_pair_regalloc,		&opt},
		{"final code validation",	0, 1,		rc_validate_final_shader,	NULL},
		{"machine code generation",	0, is_r500,	r500BuildFragmentProgramHwCode,	NULL},
		{"machine code generation",	0, !is_r500,	r300BuildFragmentProgramHwCode,	NULL},
		{"dump machine code",		0, is_r500  && (c->Base.Debug & RC_DBG_LOG), r500FragmentProgramDump, NULL},
		{"dump machine code",		0, !is_r500 && (c->Base.Debug & RC_DBG_LOG), r300FragmentProgramDump, NULL},
		{NULL, 0, 0, NULL, NULL}
	};

	c->Base.type = RC_FRAGMENT_PROGRAM;
	c->Base.SwizzleCaps = c->Base.is_r500 ? &r500_swizzle_caps : &r300_swizzle_caps;

	rc_run_compiler(&c->Base, fs_list);

	rc_constants_copy(&c->code->constants, &c->Base.Program.Constants);
}
