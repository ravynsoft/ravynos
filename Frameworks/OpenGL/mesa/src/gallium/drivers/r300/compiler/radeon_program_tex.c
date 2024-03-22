/*
 * Copyright (C) 2010 Corbin Simpson
 * Copyright (C) 2010 Marek Olšák <maraeo@gmail.com>
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

#include "radeon_program_tex.h"

#include "radeon_compiler_util.h"

/* Series of transformations to be done on textures. */

static struct rc_src_register shadow_fail_value(struct r300_fragment_program_compiler *compiler,
						int tmu)
{
	struct rc_src_register reg = { 0, 0, 0, 0, 0, 0 };

	reg.File = RC_FILE_NONE;
	reg.Swizzle = combine_swizzles(RC_SWIZZLE_0000,
				compiler->state.unit[tmu].texture_swizzle);
	return reg;
}

static struct rc_src_register shadow_pass_value(struct r300_fragment_program_compiler *compiler,
						int tmu)
{
	struct rc_src_register reg = { 0, 0, 0, 0, 0, 0 };

	reg.File = RC_FILE_NONE;
	reg.Swizzle = combine_swizzles(RC_SWIZZLE_1111,
				compiler->state.unit[tmu].texture_swizzle);
	return reg;
}

static void scale_texcoords(struct r300_fragment_program_compiler *compiler,
			    struct rc_instruction *inst,
			    unsigned state_constant)
{
	struct rc_instruction *inst_mov;

	unsigned temp = rc_find_free_temporary(&compiler->Base);

	inst_mov = rc_insert_new_instruction(&compiler->Base, inst->Prev);

	inst_mov->U.I.Opcode = RC_OPCODE_MUL;
	inst_mov->U.I.DstReg.File = RC_FILE_TEMPORARY;
	inst_mov->U.I.DstReg.Index = temp;
	inst_mov->U.I.SrcReg[0] = inst->U.I.SrcReg[0];
	inst_mov->U.I.SrcReg[1].File = RC_FILE_CONSTANT;
	inst_mov->U.I.SrcReg[1].Index =
			rc_constants_add_state(&compiler->Base.Program.Constants,
					       state_constant, inst->U.I.TexSrcUnit);

	reset_srcreg(&inst->U.I.SrcReg[0]);
	inst->U.I.SrcReg[0].File = RC_FILE_TEMPORARY;
	inst->U.I.SrcReg[0].Index = temp;
}

static void projective_divide(struct r300_fragment_program_compiler *compiler,
			      struct rc_instruction *inst)
{
	struct rc_instruction *inst_mul, *inst_rcp;

	unsigned temp = rc_find_free_temporary(&compiler->Base);

	inst_rcp = rc_insert_new_instruction(&compiler->Base, inst->Prev);
	inst_rcp->U.I.Opcode = RC_OPCODE_RCP;
	inst_rcp->U.I.DstReg.File = RC_FILE_TEMPORARY;
	inst_rcp->U.I.DstReg.Index = temp;
	inst_rcp->U.I.DstReg.WriteMask = RC_MASK_W;
	inst_rcp->U.I.SrcReg[0] = inst->U.I.SrcReg[0];
	/* Because the input can be arbitrarily swizzled,
	 * read the component mapped to W. */
	inst_rcp->U.I.SrcReg[0].Swizzle =
		RC_MAKE_SWIZZLE_SMEAR(GET_SWZ(inst->U.I.SrcReg[0].Swizzle, 3));

	inst_mul = rc_insert_new_instruction(&compiler->Base, inst->Prev);
	inst_mul->U.I.Opcode = RC_OPCODE_MUL;
	inst_mul->U.I.DstReg.File = RC_FILE_TEMPORARY;
	inst_mul->U.I.DstReg.Index = temp;
	inst_mul->U.I.SrcReg[0] = inst->U.I.SrcReg[0];
	inst_mul->U.I.SrcReg[1].File = RC_FILE_TEMPORARY;
	inst_mul->U.I.SrcReg[1].Index = temp;
	inst_mul->U.I.SrcReg[1].Swizzle = RC_SWIZZLE_WWWW;

	reset_srcreg(&inst->U.I.SrcReg[0]);
	inst->U.I.Opcode = RC_OPCODE_TEX;
	inst->U.I.SrcReg[0].File = RC_FILE_TEMPORARY;
	inst->U.I.SrcReg[0].Index = temp;
}

/**
 * Transform TEX, TXP, TXB, and KIL instructions in the following ways:
 *  - implement texture compare (shadow extensions)
 *  - extract non-native source / destination operands
 *  - premultiply texture coordinates for RECT
 *  - extract operand swizzles
 *  - introduce a temporary register when write masks are needed
 */
int radeonTransformTEX(
	struct radeon_compiler * c,
	struct rc_instruction * inst,
	void* data)
{
	struct r300_fragment_program_compiler *compiler =
		(struct r300_fragment_program_compiler*)data;
	rc_wrap_mode wrapmode = compiler->state.unit[inst->U.I.TexSrcUnit].wrap_mode;
	int is_rect = inst->U.I.TexSrcTarget == RC_TEXTURE_RECT;

	if (inst->U.I.Opcode != RC_OPCODE_TEX &&
		inst->U.I.Opcode != RC_OPCODE_TXB &&
		inst->U.I.Opcode != RC_OPCODE_TXP &&
		inst->U.I.Opcode != RC_OPCODE_TXD &&
		inst->U.I.Opcode != RC_OPCODE_TXL &&
		inst->U.I.Opcode != RC_OPCODE_KIL)
		return 0;

	/* ARB_shadow & EXT_shadow_funcs */
	if (inst->U.I.Opcode != RC_OPCODE_KIL &&
		((c->Program.ShadowSamplers & (1U << inst->U.I.TexSrcUnit)) ||
		 (compiler->state.unit[inst->U.I.TexSrcUnit].compare_mode_enabled))) {
		rc_compare_func comparefunc = compiler->state.unit[inst->U.I.TexSrcUnit].texture_compare_func;

		if (comparefunc == RC_COMPARE_FUNC_NEVER || comparefunc == RC_COMPARE_FUNC_ALWAYS) {
			inst->U.I.Opcode = RC_OPCODE_MOV;

			if (comparefunc == RC_COMPARE_FUNC_ALWAYS) {
				inst->U.I.SrcReg[0] = shadow_pass_value(compiler, inst->U.I.TexSrcUnit);
			} else {
				inst->U.I.SrcReg[0] = shadow_fail_value(compiler, inst->U.I.TexSrcUnit);
			}

			return 1;
		} else {
			struct rc_instruction * inst_rcp = NULL;
			struct rc_instruction *inst_mul, *inst_add, *inst_cmp;
			unsigned tmp_texsample;
			unsigned tmp_sum;
			int pass, fail;

			/* Save the output register. */
			struct rc_dst_register output_reg = inst->U.I.DstReg;
			unsigned saturate_mode = inst->U.I.SaturateMode;

			/* Redirect TEX to a new temp. */
			tmp_texsample = rc_find_free_temporary(c);
			inst->U.I.SaturateMode = 0;
			inst->U.I.DstReg.File = RC_FILE_TEMPORARY;
			inst->U.I.DstReg.Index = tmp_texsample;
			inst->U.I.DstReg.WriteMask = RC_MASK_XYZW;

			tmp_sum = rc_find_free_temporary(c);

			if (inst->U.I.Opcode == RC_OPCODE_TXP) {
				/* Compute 1/W. */
				inst_rcp = rc_insert_new_instruction(c, inst);
				inst_rcp->U.I.Opcode = RC_OPCODE_RCP;
				inst_rcp->U.I.DstReg.File = RC_FILE_TEMPORARY;
				inst_rcp->U.I.DstReg.Index = tmp_sum;
				inst_rcp->U.I.DstReg.WriteMask = RC_MASK_W;
				inst_rcp->U.I.SrcReg[0] = inst->U.I.SrcReg[0];
				inst_rcp->U.I.SrcReg[0].Swizzle =
					RC_MAKE_SWIZZLE_SMEAR(GET_SWZ(inst->U.I.SrcReg[0].Swizzle, 3));
			}

			/* Divide Z by W (if it's TXP) and saturate. */
			inst_mul = rc_insert_new_instruction(c, inst_rcp ? inst_rcp : inst);
			inst_mul->U.I.Opcode = inst->U.I.Opcode == RC_OPCODE_TXP ? RC_OPCODE_MUL : RC_OPCODE_MOV;
			inst_mul->U.I.DstReg.File = RC_FILE_TEMPORARY;
			inst_mul->U.I.DstReg.Index = tmp_sum;
			inst_mul->U.I.DstReg.WriteMask = RC_MASK_W;
			inst_mul->U.I.SaturateMode = RC_SATURATE_ZERO_ONE;
			inst_mul->U.I.SrcReg[0] = inst->U.I.SrcReg[0];
			inst_mul->U.I.SrcReg[0].Swizzle =
				RC_MAKE_SWIZZLE_SMEAR(GET_SWZ(inst->U.I.SrcReg[0].Swizzle, 2));
			if (inst->U.I.Opcode == RC_OPCODE_TXP) {
				inst_mul->U.I.SrcReg[1].File = RC_FILE_TEMPORARY;
				inst_mul->U.I.SrcReg[1].Index = tmp_sum;
				inst_mul->U.I.SrcReg[1].Swizzle = RC_SWIZZLE_WWWW;
			}

			/* Add the depth texture value. */
			inst_add = rc_insert_new_instruction(c, inst_mul);
			inst_add->U.I.Opcode = RC_OPCODE_ADD;
			inst_add->U.I.DstReg.File = RC_FILE_TEMPORARY;
			inst_add->U.I.DstReg.Index = tmp_sum;
			inst_add->U.I.DstReg.WriteMask = RC_MASK_W;
			inst_add->U.I.SrcReg[0].File = RC_FILE_TEMPORARY;
			inst_add->U.I.SrcReg[0].Index = tmp_sum;
			inst_add->U.I.SrcReg[0].Swizzle = RC_SWIZZLE_WWWW;
			inst_add->U.I.SrcReg[1].File = RC_FILE_TEMPORARY;
			inst_add->U.I.SrcReg[1].Index = tmp_texsample;
			inst_add->U.I.SrcReg[1].Swizzle = RC_SWIZZLE_XXXX;

			/* Note that SrcReg[0] is r, SrcReg[1] is tex and:
			 *   LESS:    r  < tex  <=>      -tex+r < 0
			 *   GEQUAL:  r >= tex  <=> not (-tex+r < 0)
			 *   GREATER: r  > tex  <=>       tex-r < 0
			 *   LEQUAL:  r <= tex  <=> not ( tex-r < 0)
			 *   EQUAL:   GEQUAL
			 *   NOTEQUAL:LESS
			 */

			/* This negates either r or tex: */
			if (comparefunc == RC_COMPARE_FUNC_LESS || comparefunc == RC_COMPARE_FUNC_GEQUAL ||
			    comparefunc == RC_COMPARE_FUNC_EQUAL || comparefunc == RC_COMPARE_FUNC_NOTEQUAL)
				inst_add->U.I.SrcReg[1].Negate = inst_add->U.I.SrcReg[1].Negate ^ RC_MASK_XYZW;
			else
				inst_add->U.I.SrcReg[0].Negate = inst_add->U.I.SrcReg[0].Negate ^ RC_MASK_XYZW;

			/* This negates the whole expression: */
			if (comparefunc == RC_COMPARE_FUNC_LESS || comparefunc == RC_COMPARE_FUNC_GREATER ||
			    comparefunc == RC_COMPARE_FUNC_NOTEQUAL) {
				pass = 1;
				fail = 2;
			} else {
				pass = 2;
				fail = 1;
			}

			inst_cmp = rc_insert_new_instruction(c, inst_add);
			inst_cmp->U.I.Opcode = RC_OPCODE_CMP;
			inst_cmp->U.I.SaturateMode = saturate_mode;
			inst_cmp->U.I.DstReg = output_reg;
			inst_cmp->U.I.SrcReg[0].File = RC_FILE_TEMPORARY;
			inst_cmp->U.I.SrcReg[0].Index = tmp_sum;
			inst_cmp->U.I.SrcReg[0].Swizzle =
					combine_swizzles(RC_SWIZZLE_WWWW,
							 compiler->state.unit[inst->U.I.TexSrcUnit].texture_swizzle);
			inst_cmp->U.I.SrcReg[pass] = shadow_pass_value(compiler, inst->U.I.TexSrcUnit);
			inst_cmp->U.I.SrcReg[fail] = shadow_fail_value(compiler, inst->U.I.TexSrcUnit);

			assert(tmp_texsample != tmp_sum);
		}
	}

	/* R300 cannot sample from rectangles and the wrap mode fallback needs
	 * normalized coordinates anyway. */
	if (inst->U.I.Opcode != RC_OPCODE_KIL &&
	    is_rect && (!c->is_r500 || wrapmode != RC_WRAP_NONE)) {
		scale_texcoords(compiler, inst, RC_STATE_R300_TEXRECT_FACTOR);
		inst->U.I.TexSrcTarget = RC_TEXTURE_2D;
	}

	/* Divide by W if needed. */
	if (inst->U.I.Opcode == RC_OPCODE_TXP &&
	    (wrapmode == RC_WRAP_REPEAT || wrapmode == RC_WRAP_MIRRORED_REPEAT ||
	     compiler->state.unit[inst->U.I.TexSrcUnit].clamp_and_scale_before_fetch)) {
		projective_divide(compiler, inst);
	}

	/* Texture wrap modes don't work on NPOT textures.
	 *
	 * Non-wrapped/clamped texcoords with NPOT are free in HW. Repeat and
	 * mirroring are not. If we need to repeat, we do:
	 *
	 * MUL temp, texcoord, <scaling factor constant>
	 * FRC temp, temp ; Discard integer portion of coords
	 *
	 * This gives us coords in [0, 1].
	 *
	 * Mirroring is trickier. We're going to start out like repeat:
	 *
	 * MUL temp, texcoord, <scaling factor constant> ; De-mirror across axes
	 * MUL temp, temp, 0.5 ; Pattern repeats in [0, 2]
	 *                            ; so scale to [0, 1]
	 * FRC temp, temp ; Make the pattern repeat
	 * MAD temp, temp, 2, -1 ; Move the pattern to [-1, 1]
	 * ADD temp, 1, -abs(temp) ; Now comes a neat trick: use abs to mirror the pattern.
	 *				; The pattern is backwards, so reverse it (1-x).
	 *
	 * This gives us coords in [0, 1].
	 *
	 * ~ C & M. ;)
	 */
	if (inst->U.I.Opcode != RC_OPCODE_KIL &&
	    wrapmode != RC_WRAP_NONE) {
		struct rc_instruction *inst_mov;
		unsigned temp = rc_find_free_temporary(c);

		if (wrapmode == RC_WRAP_REPEAT) {
			/* Both instructions will be paired up. */
			struct rc_instruction *inst_frc = rc_insert_new_instruction(c, inst->Prev);

			inst_frc->U.I.Opcode = RC_OPCODE_FRC;
			inst_frc->U.I.DstReg.File = RC_FILE_TEMPORARY;
			inst_frc->U.I.DstReg.Index = temp;
			inst_frc->U.I.DstReg.WriteMask = RC_MASK_XYZ;
			inst_frc->U.I.SrcReg[0] = inst->U.I.SrcReg[0];
		} else if (wrapmode == RC_WRAP_MIRRORED_REPEAT) {
			/*
			 * Function:
			 *   f(v) = 1 - abs(frac(v * 0.5) * 2 - 1)
			 *
			 * Code:
			 *   MUL temp, src0, 0.5
			 *   FRC temp, temp
			 *   MAD temp, temp, 2, -1
			 *   ADD temp, 1, -abs(temp)
			 */

			struct rc_instruction *inst_mul, *inst_frc, *inst_mad, *inst_add;
			unsigned two, two_swizzle;

			inst_mul = rc_insert_new_instruction(c, inst->Prev);

			inst_mul->U.I.Opcode = RC_OPCODE_MUL;
			inst_mul->U.I.DstReg.File = RC_FILE_TEMPORARY;
			inst_mul->U.I.DstReg.Index = temp;
			inst_mul->U.I.DstReg.WriteMask = RC_MASK_XYZ;
			inst_mul->U.I.SrcReg[0] = inst->U.I.SrcReg[0];
			inst_mul->U.I.SrcReg[1].Swizzle = RC_SWIZZLE_HHHH;

			inst_frc = rc_insert_new_instruction(c, inst->Prev);

			inst_frc->U.I.Opcode = RC_OPCODE_FRC;
			inst_frc->U.I.DstReg.File = RC_FILE_TEMPORARY;
			inst_frc->U.I.DstReg.Index = temp;
			inst_frc->U.I.DstReg.WriteMask = RC_MASK_XYZ;
			inst_frc->U.I.SrcReg[0].File = RC_FILE_TEMPORARY;
			inst_frc->U.I.SrcReg[0].Index = temp;
			inst_frc->U.I.SrcReg[0].Swizzle = RC_SWIZZLE_XYZ0;

			two = rc_constants_add_immediate_scalar(&c->Program.Constants, 2, &two_swizzle);
			inst_mad = rc_insert_new_instruction(c, inst->Prev);

			inst_mad->U.I.Opcode = RC_OPCODE_MAD;
			inst_mad->U.I.DstReg.File = RC_FILE_TEMPORARY;
			inst_mad->U.I.DstReg.Index = temp;
			inst_mad->U.I.DstReg.WriteMask = RC_MASK_XYZ;
			inst_mad->U.I.SrcReg[0].File = RC_FILE_TEMPORARY;
			inst_mad->U.I.SrcReg[0].Index = temp;
			inst_mad->U.I.SrcReg[0].Swizzle = RC_SWIZZLE_XYZ0;
			inst_mad->U.I.SrcReg[1].File = RC_FILE_CONSTANT;
			inst_mad->U.I.SrcReg[1].Index = two;
			inst_mad->U.I.SrcReg[1].Swizzle = two_swizzle;
			inst_mad->U.I.SrcReg[2].Swizzle = RC_SWIZZLE_1111;
			inst_mad->U.I.SrcReg[2].Negate = RC_MASK_XYZ;

			inst_add = rc_insert_new_instruction(c, inst->Prev);

			inst_add->U.I.Opcode = RC_OPCODE_ADD;
			inst_add->U.I.DstReg.File = RC_FILE_TEMPORARY;
			inst_add->U.I.DstReg.Index = temp;
			inst_add->U.I.DstReg.WriteMask = RC_MASK_XYZ;
			inst_add->U.I.SrcReg[0].Swizzle = RC_SWIZZLE_1111;
			inst_add->U.I.SrcReg[1].File = RC_FILE_TEMPORARY;
			inst_add->U.I.SrcReg[1].Index = temp;
			inst_add->U.I.SrcReg[1].Swizzle = RC_SWIZZLE_XYZ0;
			inst_add->U.I.SrcReg[1].Abs = 1;
			inst_add->U.I.SrcReg[1].Negate = RC_MASK_XYZ;
		} else if (wrapmode == RC_WRAP_MIRRORED_CLAMP) {
			/*
			 * Mirrored clamp modes are bloody simple, we just use abs
			 * to mirror [0, 1] into [-1, 0]. This works for
			 * all modes i.e. CLAMP, CLAMP_TO_EDGE, and CLAMP_TO_BORDER.
			 */
			struct rc_instruction *inst_mov;

			inst_mov = rc_insert_new_instruction(c, inst->Prev);

			inst_mov->U.I.Opcode = RC_OPCODE_MOV;
			inst_mov->U.I.DstReg.File = RC_FILE_TEMPORARY;
			inst_mov->U.I.DstReg.Index = temp;
			inst_mov->U.I.DstReg.WriteMask = RC_MASK_XYZ;
			inst_mov->U.I.SrcReg[0] = inst->U.I.SrcReg[0];
			inst_mov->U.I.SrcReg[0].Abs = 1;
		}

		/* Preserve W for TXP/TXB. */
		inst_mov = rc_insert_new_instruction(c, inst->Prev);

		inst_mov->U.I.Opcode = RC_OPCODE_MOV;
		inst_mov->U.I.DstReg.File = RC_FILE_TEMPORARY;
		inst_mov->U.I.DstReg.Index = temp;
		inst_mov->U.I.DstReg.WriteMask = RC_MASK_W;
		inst_mov->U.I.SrcReg[0] = inst->U.I.SrcReg[0];

		reset_srcreg(&inst->U.I.SrcReg[0]);
		inst->U.I.SrcReg[0].File = RC_FILE_TEMPORARY;
		inst->U.I.SrcReg[0].Index = temp;
	}

	/* NPOT -> POT conversion for 3D textures. */
	if (inst->U.I.Opcode != RC_OPCODE_KIL &&
	    compiler->state.unit[inst->U.I.TexSrcUnit].clamp_and_scale_before_fetch) {
		struct rc_instruction *inst_mov;
		unsigned temp = rc_find_free_temporary(c);

		/* Saturate XYZ. */
		inst_mov = rc_insert_new_instruction(c, inst->Prev);
		inst_mov->U.I.Opcode = RC_OPCODE_MOV;
		inst_mov->U.I.SaturateMode = RC_SATURATE_ZERO_ONE;
		inst_mov->U.I.DstReg.File = RC_FILE_TEMPORARY;
		inst_mov->U.I.DstReg.Index = temp;
		inst_mov->U.I.DstReg.WriteMask = RC_MASK_XYZ;
		inst_mov->U.I.SrcReg[0] = inst->U.I.SrcReg[0];

		/* Copy W. */
		inst_mov = rc_insert_new_instruction(c, inst->Prev);
		inst_mov->U.I.Opcode = RC_OPCODE_MOV;
		inst_mov->U.I.DstReg.File = RC_FILE_TEMPORARY;
		inst_mov->U.I.DstReg.Index = temp;
		inst_mov->U.I.DstReg.WriteMask = RC_MASK_W;
		inst_mov->U.I.SrcReg[0] = inst->U.I.SrcReg[0];

		reset_srcreg(&inst->U.I.SrcReg[0]);
		inst->U.I.SrcReg[0].File = RC_FILE_TEMPORARY;
		inst->U.I.SrcReg[0].Index = temp;

		scale_texcoords(compiler, inst, RC_STATE_R300_TEXSCALE_FACTOR);
	}

	/* Cannot write texture to output registers or with saturate (all chips),
	 * or with masks (non-r500). */
	if (inst->U.I.Opcode != RC_OPCODE_KIL &&
		(inst->U.I.DstReg.File != RC_FILE_TEMPORARY ||
		 inst->U.I.SaturateMode ||
		 (!c->is_r500 && inst->U.I.DstReg.WriteMask != RC_MASK_XYZW))) {
		struct rc_instruction * inst_mov = rc_insert_new_instruction(c, inst);

		inst_mov->U.I.Opcode = RC_OPCODE_MOV;
		inst_mov->U.I.SaturateMode = inst->U.I.SaturateMode;
		inst_mov->U.I.DstReg = inst->U.I.DstReg;
		inst_mov->U.I.SrcReg[0].File = RC_FILE_TEMPORARY;
		inst_mov->U.I.SrcReg[0].Index = rc_find_free_temporary(c);

		inst->U.I.SaturateMode = 0;
		inst->U.I.DstReg.File = RC_FILE_TEMPORARY;
		inst->U.I.DstReg.Index = inst_mov->U.I.SrcReg[0].Index;
		inst->U.I.DstReg.WriteMask = RC_MASK_XYZW;
	}

	/* Cannot read texture coordinate from constants file */
	if (inst->U.I.SrcReg[0].File != RC_FILE_TEMPORARY && inst->U.I.SrcReg[0].File != RC_FILE_INPUT) {
		struct rc_instruction * inst_mov = rc_insert_new_instruction(c, inst->Prev);

		inst_mov->U.I.Opcode = RC_OPCODE_MOV;
		inst_mov->U.I.DstReg.File = RC_FILE_TEMPORARY;
		inst_mov->U.I.DstReg.Index = rc_find_free_temporary(c);
		inst_mov->U.I.SrcReg[0] = inst->U.I.SrcReg[0];

		reset_srcreg(&inst->U.I.SrcReg[0]);
		inst->U.I.SrcReg[0].File = RC_FILE_TEMPORARY;
		inst->U.I.SrcReg[0].Index = inst_mov->U.I.DstReg.Index;
	}

	return 1;
}
