/*
 * Copyright (C) 2008-2009 Nicolai Haehnle.
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

#include "radeon_compiler_util.h"

#include <stdlib.h>

/**
 * Return the source slot where we installed the given register access,
 * or -1 if no slot was free anymore.
 */
int rc_pair_alloc_source(struct rc_pair_instruction *pair,
	unsigned int rgb, unsigned int alpha,
	rc_register_file file, unsigned int index)
{
	int candidate = -1;
	int candidate_quality = -1;
	unsigned int alpha_used = 0;
	unsigned int rgb_used = 0;
	int i;

	if ((!rgb && !alpha) || file == RC_FILE_NONE)
		return 0;

	/* Make sure only one presubtract operation is used per instruction. */
	if (file == RC_FILE_PRESUB) {
		if (rgb && pair->RGB.Src[RC_PAIR_PRESUB_SRC].Used
			&& index != pair->RGB.Src[RC_PAIR_PRESUB_SRC].Index) {
				return -1;
		}

		if (alpha && pair->Alpha.Src[RC_PAIR_PRESUB_SRC].Used
			&& index != pair->Alpha.Src[RC_PAIR_PRESUB_SRC].Index) {
				return -1;
		}
	}

	for(i = 0; i < 3; ++i) {
		int q = 0;
		if (rgb) {
			if (pair->RGB.Src[i].Used) {
				if (pair->RGB.Src[i].File != file ||
				    pair->RGB.Src[i].Index != index) {
					rgb_used++;
					continue;
				}
				q++;
			}
		}
		if (alpha) {
			if (pair->Alpha.Src[i].Used) {
				if (pair->Alpha.Src[i].File != file ||
				    pair->Alpha.Src[i].Index != index) {
					alpha_used++;
					continue;
				}
				q++;
			}
		}
		if (q > candidate_quality) {
			candidate_quality = q;
			candidate = i;
		}
	}

	if (file == RC_FILE_PRESUB) {
		candidate = RC_PAIR_PRESUB_SRC;
	} else if (candidate < 0 || (rgb && rgb_used > 2)
			|| (alpha && alpha_used > 2)) {
		return -1;
	}

	/* candidate >= 0 */

	if (rgb) {
		pair->RGB.Src[candidate].Used = 1;
		pair->RGB.Src[candidate].File = file;
		pair->RGB.Src[candidate].Index = index;
		if (candidate == RC_PAIR_PRESUB_SRC) {
			/* For registers with the RC_FILE_PRESUB file,
			 * the index stores the presubtract op. */
			int src_regs = rc_presubtract_src_reg_count(index);
			for(i = 0; i < src_regs; i++) {
				pair->RGB.Src[i].Used = 1;
			}
		}
	}
	if (alpha) {
		pair->Alpha.Src[candidate].Used = 1;
		pair->Alpha.Src[candidate].File = file;
		pair->Alpha.Src[candidate].Index = index;
		if (candidate == RC_PAIR_PRESUB_SRC) {
			/* For registers with the RC_FILE_PRESUB file,
			 * the index stores the presubtract op. */
			int src_regs = rc_presubtract_src_reg_count(index);
			for(i=0; i < src_regs; i++) {
				pair->Alpha.Src[i].Used = 1;
			}
		}
	}

	return candidate;
}

static void pair_foreach_source_callback(
	struct rc_pair_instruction * pair,
	void * data,
	rc_pair_foreach_src_fn cb,
	unsigned int swz,
	unsigned int src)
{
	/* swz > 3 means that the swizzle is either not used, or a constant
	 * swizzle (e.g. 0, 1, 0.5). */
	if(swz > 3)
		return;

	if(swz == RC_SWIZZLE_W) {
		if (src == RC_PAIR_PRESUB_SRC) {
			unsigned int i;
			unsigned int src_count = rc_presubtract_src_reg_count(
				pair->Alpha.Src[RC_PAIR_PRESUB_SRC].Index);
			for(i = 0; i < src_count; i++) {
				cb(data, &pair->Alpha.Src[i]);
			}
		} else {
			cb(data, &pair->Alpha.Src[src]);
		}
	} else {
		if (src == RC_PAIR_PRESUB_SRC) {
			unsigned int i;
			unsigned int src_count = rc_presubtract_src_reg_count(
				pair->RGB.Src[RC_PAIR_PRESUB_SRC].Index);
			for(i = 0; i < src_count; i++) {
				cb(data, &pair->RGB.Src[i]);
			}
		}
		else {
			cb(data, &pair->RGB.Src[src]);
		}
	}
}

void rc_pair_foreach_source_that_alpha_reads(
	struct rc_pair_instruction * pair,
	void * data,
	rc_pair_foreach_src_fn cb)
{
	unsigned int i;
	const struct rc_opcode_info * info =
				rc_get_opcode_info(pair->Alpha.Opcode);
	for(i = 0; i < info->NumSrcRegs; i++) {
		pair_foreach_source_callback(pair, data, cb,
					GET_SWZ(pair->Alpha.Arg[i].Swizzle, 0),
					pair->Alpha.Arg[i].Source);
	}
}

void rc_pair_foreach_source_that_rgb_reads(
	struct rc_pair_instruction * pair,
	void * data,
	rc_pair_foreach_src_fn cb)
{
	unsigned int i;
	const struct rc_opcode_info * info =
				rc_get_opcode_info(pair->RGB.Opcode);
	for(i = 0; i < info->NumSrcRegs; i++) {
		unsigned int chan;
		unsigned int swz = RC_SWIZZLE_UNUSED;
		/* Find a swizzle that is either X,Y,Z,or W.  We assume here
		 * that if one channel swizzles X,Y, or Z, then none of the
		 * other channels swizzle W, and vice-versa. */
		for(chan = 0; chan < 4; chan++) {
			swz = GET_SWZ(pair->RGB.Arg[i].Swizzle, chan);
			if(swz == RC_SWIZZLE_X || swz == RC_SWIZZLE_Y
			|| swz == RC_SWIZZLE_Z || swz == RC_SWIZZLE_W)
				continue;
		}
		pair_foreach_source_callback(pair, data, cb,
					swz,
					pair->RGB.Arg[i].Source);
	}
}

struct rc_pair_instruction_source * rc_pair_get_src(
	struct rc_pair_instruction * pair_inst,
	struct rc_pair_instruction_arg * arg)
{
	unsigned int type;

	type = rc_source_type_swz(arg->Swizzle);

	if (type & RC_SOURCE_RGB) {
		return &pair_inst->RGB.Src[arg->Source];
	} else if (type & RC_SOURCE_ALPHA) {
		return &pair_inst->Alpha.Src[arg->Source];
	} else {
		return NULL;
	}
}

int rc_pair_get_src_index(
	struct rc_pair_instruction * pair_inst,
	struct rc_pair_instruction_source * src)
{
	int i;
	for (i = 0; i < 3; i++) {
		if (&pair_inst->RGB.Src[i] == src
			|| &pair_inst->Alpha.Src[i] == src) {
			return i;
		}
	}
	return -1;
}
