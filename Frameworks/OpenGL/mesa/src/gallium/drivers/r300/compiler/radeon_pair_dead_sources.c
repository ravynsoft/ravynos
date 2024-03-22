/*
 * Copyright 2011 Tom Stellard <tstellar@gmail.com>
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

#include "radeon_compiler.h"
#include "radeon_compiler_util.h"
#include "radeon_opcodes.h"
#include "radeon_program_pair.h"

static void mark_used_presub(struct rc_pair_sub_instruction * sub)
{
	if (sub->Src[RC_PAIR_PRESUB_SRC].Used) {
		unsigned int presub_reg_count = rc_presubtract_src_reg_count(
					sub->Src[RC_PAIR_PRESUB_SRC].Index);
		unsigned int i;
		for (i = 0; i < presub_reg_count; i++) {
			sub->Src[i].Used = 1;
		}
	}
}

static void mark_used(
	struct rc_instruction * inst,
	struct rc_pair_sub_instruction * sub)
{
	unsigned int i;
	const struct rc_opcode_info * info = rc_get_opcode_info(sub->Opcode);
	for (i = 0; i < info->NumSrcRegs; i++) {
		unsigned int src_type = rc_source_type_swz(sub->Arg[i].Swizzle);
		if (src_type & RC_SOURCE_RGB) {
			inst->U.P.RGB.Src[sub->Arg[i].Source].Used = 1;
		}

		if (src_type & RC_SOURCE_ALPHA) {
			inst->U.P.Alpha.Src[sub->Arg[i].Source].Used = 1;
		}
	}
}

/**
 * This pass finds sources that are not used by their instruction and marks
 * them as unused. 
 */
void rc_pair_remove_dead_sources(struct radeon_compiler * c, void *user)
{
	struct rc_instruction * inst;
	for (inst = c->Program.Instructions.Next;
					inst != &c->Program.Instructions;
					inst = inst->Next) {
		unsigned int i;
		if (inst->Type == RC_INSTRUCTION_NORMAL)
			continue;

		/* Mark all sources as unused */
		for (i = 0; i < 4; i++) {
			inst->U.P.RGB.Src[i].Used = 0;
			inst->U.P.Alpha.Src[i].Used = 0;
		}
		mark_used(inst, &inst->U.P.RGB);
		mark_used(inst, &inst->U.P.Alpha);

		mark_used_presub(&inst->U.P.RGB);
		mark_used_presub(&inst->U.P.Alpha);
	}
}
