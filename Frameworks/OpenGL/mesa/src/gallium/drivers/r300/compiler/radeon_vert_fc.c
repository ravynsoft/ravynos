/*
 * Copyright 2012 Advanced Micro Devices, Inc.
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
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Author: Tom Stellard <thomas.stellard@amd.com>
 */

#include "radeon_compiler.h"
#include "radeon_compiler_util.h"
#include "radeon_dataflow.h"
#include "radeon_program.h"
#include "radeon_program_constants.h"

struct vert_fc_state {
	struct radeon_compiler *C;
	unsigned BranchDepth;
	unsigned LoopDepth;
	unsigned LoopsReserved;
	int PredStack[R500_PVS_MAX_LOOP_DEPTH];
	int PredicateReg;
};

static void build_pred_src(
	struct rc_src_register * src,
	struct vert_fc_state * fc_state)
{
	src->Swizzle = RC_MAKE_SWIZZLE(RC_SWIZZLE_UNUSED, RC_SWIZZLE_UNUSED,
					RC_SWIZZLE_UNUSED, RC_SWIZZLE_W);
	src->File = RC_FILE_TEMPORARY;
	src->Index = fc_state->PredicateReg;
}

static void build_pred_dst(
	struct rc_dst_register * dst,
	struct vert_fc_state * fc_state)
{
	dst->WriteMask = RC_MASK_W;
	dst->File = RC_FILE_TEMPORARY;
	dst->Index = fc_state->PredicateReg;
}

static void mark_write(void * userdata,	struct rc_instruction * inst,
		rc_register_file file,	unsigned int index, unsigned int mask)
{
	unsigned int * writemasks = userdata;

	if (file != RC_FILE_TEMPORARY)
		return;

	if (index >= R300_VS_MAX_TEMPS)
		return;

	writemasks[index] |= mask;
}

static int reserve_predicate_reg(struct vert_fc_state * fc_state)
{
	int i;
	unsigned int writemasks[RC_REGISTER_MAX_INDEX];
	struct rc_instruction * inst;
	memset(writemasks, 0, sizeof(writemasks));
	for(inst = fc_state->C->Program.Instructions.Next;
				inst != &fc_state->C->Program.Instructions;
				inst = inst->Next) {
		rc_for_all_writes_mask(inst, mark_write, writemasks);
	}

	for(i = 0; i < fc_state->C->max_temp_regs; i++) {
		/* Most of the control flow instructions only write the
		 * W component of the Predicate Register, but
		 * the docs say that ME_PRED_SET_CLR and
		 * ME_PRED_SET_RESTORE write all components of the
		 * register, so we must reserve a register that has
		 * all its components free. */
		if (!writemasks[i]) {
			fc_state->PredicateReg = i;
			break;
		}
	}
	if (i == fc_state->C->max_temp_regs) {
		rc_error(fc_state->C, "No free temporary to use for"
				" predicate stack counter.\n");
		return -1;
	}
	return 1;
}

static void lower_bgnloop(
	struct rc_instruction * inst,
	struct vert_fc_state * fc_state)
{
	struct rc_instruction * new_inst =
			rc_insert_new_instruction(fc_state->C, inst->Prev);

	if ((!fc_state->C->is_r500
		&& fc_state->LoopsReserved >= R300_VS_MAX_LOOP_DEPTH)
	     || fc_state->LoopsReserved >= R500_PVS_MAX_LOOP_DEPTH) {
		rc_error(fc_state->C, "Loops are nested too deep.");
		return;
	}

	if (fc_state->LoopDepth == 0 && fc_state->BranchDepth == 0) {
		if (fc_state->PredicateReg == -1) {
			if (reserve_predicate_reg(fc_state) == -1) {
				return;
			}
		}

		/* Initialize the predicate bit to true. */
		new_inst->U.I.Opcode = RC_ME_PRED_SEQ;
		build_pred_dst(&new_inst->U.I.DstReg, fc_state);
		new_inst->U.I.SrcReg[0].Index = 0;
		new_inst->U.I.SrcReg[0].File = RC_FILE_NONE;
		new_inst->U.I.SrcReg[0].Swizzle = RC_SWIZZLE_0000;
	} else {
		fc_state->PredStack[fc_state->LoopDepth] =
						fc_state->PredicateReg;
		/* Copy the current predicate value to this loop's
		 * predicate register */

		/* Use the old predicate value for src0 */
		build_pred_src(&new_inst->U.I.SrcReg[0], fc_state);

		/* Reserve this loop's predicate register */
		if (reserve_predicate_reg(fc_state) == -1) {
			return;
		}

		/* Copy the old predicate value to the new register */
		new_inst->U.I.Opcode = RC_OPCODE_ADD;
		build_pred_dst(&new_inst->U.I.DstReg, fc_state);
		new_inst->U.I.SrcReg[1].Index = 0;
		new_inst->U.I.SrcReg[1].File = RC_FILE_NONE;
		new_inst->U.I.SrcReg[1].Swizzle = RC_SWIZZLE_0000;
	}

}

static void lower_brk(
	struct rc_instruction * inst,
	struct vert_fc_state * fc_state)
{
	if (fc_state->LoopDepth == 1) {
		inst->U.I.Opcode = RC_OPCODE_RCP;
		inst->U.I.DstReg.Pred = RC_PRED_SET;
		inst->U.I.SrcReg[0].Index = 0;
		inst->U.I.SrcReg[0].File = RC_FILE_NONE;
		inst->U.I.SrcReg[0].Swizzle = RC_SWIZZLE_0000;
	} else {
		inst->U.I.Opcode = RC_ME_PRED_SET_CLR;
		inst->U.I.DstReg.Pred = RC_PRED_SET;
	}

	build_pred_dst(&inst->U.I.DstReg, fc_state);
}

static void lower_endloop(
	struct rc_instruction * inst,
	struct vert_fc_state * fc_state)
{
	struct rc_instruction * new_inst =
			rc_insert_new_instruction(fc_state->C, inst);

	new_inst->U.I.Opcode = RC_ME_PRED_SET_RESTORE;
	build_pred_dst(&new_inst->U.I.DstReg, fc_state);
	/* Restore the previous predicate register. */
	fc_state->PredicateReg = fc_state->PredStack[fc_state->LoopDepth - 1];
	build_pred_src(&new_inst->U.I.SrcReg[0], fc_state);
}

static void lower_if(
	struct rc_instruction * inst,
	struct vert_fc_state * fc_state)
{
	/* Reserve a temporary to use as our predicate stack counter, if we
	 * don't already have one. */
	if (fc_state->PredicateReg == -1) {
		/* If we are inside a loop, the Predicate Register should
		 * have already been defined. */
		assert(fc_state->LoopDepth == 0);

		if (reserve_predicate_reg(fc_state) == -1) {
			return;
		}
	}

	if (fc_state->BranchDepth == 0 && fc_state->LoopDepth == 0) {
		inst->U.I.Opcode = RC_ME_PRED_SNEQ;
	} else {
		unsigned swz;
		inst->U.I.Opcode = RC_VE_PRED_SNEQ_PUSH;
		memcpy(&inst->U.I.SrcReg[1], &inst->U.I.SrcReg[0],
						sizeof(inst->U.I.SrcReg[1]));
		swz = rc_get_scalar_src_swz(inst->U.I.SrcReg[1].Swizzle);
		/* VE_PRED_SNEQ_PUSH needs to the branch condition to be in the
		 * w component */
		inst->U.I.SrcReg[1].Swizzle = RC_MAKE_SWIZZLE(RC_SWIZZLE_UNUSED,
				RC_SWIZZLE_UNUSED, RC_SWIZZLE_UNUSED, swz);
		build_pred_src(&inst->U.I.SrcReg[0], fc_state);
	}
	build_pred_dst(&inst->U.I.DstReg, fc_state);
}

void rc_vert_fc(struct radeon_compiler *c, void *user)
{
	struct rc_instruction * inst;
	struct vert_fc_state fc_state;

	memset(&fc_state, 0, sizeof(fc_state));
	fc_state.PredicateReg = -1;
	fc_state.C = c;

	for(inst = c->Program.Instructions.Next;
					inst != &c->Program.Instructions;
					inst = inst->Next) {

		switch (inst->U.I.Opcode) {

		case RC_OPCODE_BGNLOOP:
			lower_bgnloop(inst, &fc_state);
			fc_state.LoopDepth++;
			break;

		case RC_OPCODE_BRK:
			lower_brk(inst, &fc_state);
			break;

		case RC_OPCODE_ENDLOOP:
			if (fc_state.BranchDepth != 0
					|| fc_state.LoopDepth != 1) {
				lower_endloop(inst, &fc_state);
				/* Skip the new PRED_RESTORE */
				inst = inst->Next;
			}
			fc_state.LoopDepth--;
			break;
		case RC_OPCODE_IF:
			lower_if(inst, &fc_state);
			fc_state.BranchDepth++;
			break;

		case RC_OPCODE_ELSE:
			inst->U.I.Opcode = RC_ME_PRED_SET_INV;
			build_pred_dst(&inst->U.I.DstReg, &fc_state);
			build_pred_src(&inst->U.I.SrcReg[0], &fc_state);
			break;

		case RC_OPCODE_ENDIF:
			/* TODO: If LoopDepth == 1 and there is only a single break
			 * we can optimize out the endif just after the break. However
			 * previous attempts were buggy, so keep it simple for now.
			 */
			inst->U.I.Opcode = RC_ME_PRED_SET_POP;
			build_pred_dst(&inst->U.I.DstReg, &fc_state);
			build_pred_src(&inst->U.I.SrcReg[0], &fc_state);
			fc_state.BranchDepth--;
			break;

		default:
			if (fc_state.BranchDepth || fc_state.LoopDepth) {
				inst->U.I.DstReg.Pred = RC_PRED_SET;
			}
			break;
		}

		if (c->Error) {
			return;
		}
	}
}
