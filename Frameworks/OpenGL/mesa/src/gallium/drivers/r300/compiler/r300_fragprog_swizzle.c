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

/**
 * @file
 * Utilities to deal with the somewhat odd restriction on R300 fragment
 * program swizzles.
 */

#include "r300_fragprog_swizzle.h"

#include <stdio.h>

#include "util/macros.h"

#include "r300_reg.h"
#include "radeon_compiler.h"

#define MAKE_SWZ3(x, y, z) (RC_MAKE_SWIZZLE(RC_SWIZZLE_##x, RC_SWIZZLE_##y, RC_SWIZZLE_##z, RC_SWIZZLE_ZERO))

struct swizzle_data {
	unsigned int hash; /**< swizzle value this matches */
	unsigned int base; /**< base value for hw swizzle */
	unsigned int stride; /**< difference in base between arg0/1/2 */
	unsigned int srcp_stride; /**< difference in base between arg0/scrp */
};

static const struct swizzle_data native_swizzles[] = {
	{MAKE_SWZ3(X, Y, Z), R300_ALU_ARGC_SRC0C_XYZ, 4, 15},
	{MAKE_SWZ3(X, X, X), R300_ALU_ARGC_SRC0C_XXX, 4, 15},
	{MAKE_SWZ3(Y, Y, Y), R300_ALU_ARGC_SRC0C_YYY, 4, 15},
	{MAKE_SWZ3(Z, Z, Z), R300_ALU_ARGC_SRC0C_ZZZ, 4, 15},
	{MAKE_SWZ3(W, W, W), R300_ALU_ARGC_SRC0A, 1, 7},
	{MAKE_SWZ3(Y, Z, X), R300_ALU_ARGC_SRC0C_YZX, 1, 0},
	{MAKE_SWZ3(Z, X, Y), R300_ALU_ARGC_SRC0C_ZXY, 1, 0},
	{MAKE_SWZ3(W, Z, Y), R300_ALU_ARGC_SRC0CA_WZY, 1, 0},
	{MAKE_SWZ3(ONE, ONE, ONE), R300_ALU_ARGC_ONE, 0, 0},
	{MAKE_SWZ3(ZERO, ZERO, ZERO), R300_ALU_ARGC_ZERO, 0, 0},
	{MAKE_SWZ3(HALF, HALF, HALF), R300_ALU_ARGC_HALF, 0, 0}
};

static const int num_native_swizzles = ARRAY_SIZE(native_swizzles);
/* Only swizzles with srcp_stride != 0 can be used for presub, so
 * just the first five from the list. */
static const int num_presub_swizzles = 5;

/**
 * Find a native RGB swizzle that matches the given swizzle.
 * Returns 0 if none found.
 */
static const struct swizzle_data* lookup_native_swizzle(unsigned int swizzle)
{
	int i, comp;

	for(i = 0; i < num_native_swizzles; ++i) {
		const struct swizzle_data* sd = &native_swizzles[i];
		for(comp = 0; comp < 3; ++comp) {
			unsigned int swz = GET_SWZ(swizzle, comp);
			if (swz == RC_SWIZZLE_UNUSED)
				continue;
			if (swz != GET_SWZ(sd->hash, comp))
				break;
		}
		if (comp == 3)
			return sd;
	}

	return NULL;
}

/**
 * Determines if the given swizzle is valid for r300/r400.  In most situations
 * it is better to use r300_swizzle_is_native() which can be accessed via
 * struct radeon_compiler *c; c->SwizzleCaps->IsNative().
 */
int r300_swizzle_is_native_basic(unsigned int swizzle)
{
	if(lookup_native_swizzle(swizzle))
		return 1;
	else
		return 0;
}

/**
 * Check whether the given instruction supports the swizzle and negate
 * combinations in the given source register.
 */
static int r300_swizzle_is_native(rc_opcode opcode, struct rc_src_register reg)
{
	const struct swizzle_data* sd;
	unsigned int relevant;
	int j;

	if (opcode == RC_OPCODE_KIL ||
	    opcode == RC_OPCODE_TEX ||
	    opcode == RC_OPCODE_TXB ||
	    opcode == RC_OPCODE_TXP) {
		if (reg.Abs || reg.Negate)
			return 0;

		for(j = 0; j < 4; ++j) {
			unsigned int swz = GET_SWZ(reg.Swizzle, j);
			if (swz == RC_SWIZZLE_UNUSED)
				continue;
			if (swz != j)
				return 0;
		}

		return 1;
	}

	relevant = 0;

	for(j = 0; j < 3; ++j)
		if (GET_SWZ(reg.Swizzle, j) != RC_SWIZZLE_UNUSED)
			relevant |= 1 << j;

	if ((reg.Negate & relevant) && ((reg.Negate & relevant) != relevant))
		return 0;

	sd = lookup_native_swizzle(reg.Swizzle);
	if (!sd || (reg.File == RC_FILE_PRESUB && sd->srcp_stride == 0))
		return 0;

	return 1;
}


static void r300_swizzle_split(
		struct rc_src_register src, unsigned int mask,
		struct rc_swizzle_split * split)
{
	split->NumPhases = 0;

	while(mask) {
		unsigned int best_matchcount = 0;
		unsigned int best_matchmask = 0;
		int i, comp;

		unsigned num_swizzles = src.File == RC_FILE_PRESUB ? num_presub_swizzles : num_native_swizzles;

		for(i = 0; i < num_swizzles; ++i) {
			const struct swizzle_data *sd = &native_swizzles[i];
			unsigned int matchcount = 0;
			unsigned int matchmask = 0;
			for(comp = 0; comp < 3; ++comp) {
				unsigned int swz;
				if (!GET_BIT(mask, comp))
					continue;
				swz = GET_SWZ(src.Swizzle, comp);
				if (swz == RC_SWIZZLE_UNUSED)
					continue;
				if (swz == GET_SWZ(sd->hash, comp)) {
					/* check if the negate bit of current component
					 * is the same for already matched components */
					if (matchmask && (!!(src.Negate & matchmask) != !!(src.Negate & (1 << comp))))
						continue;

					matchcount++;
					matchmask |= 1 << comp;
				}
			}
			if (matchcount > best_matchcount) {
				best_matchcount = matchcount;
				best_matchmask = matchmask;
				if (matchmask == (mask & RC_MASK_XYZ))
					break;
			}
		}

		if (mask & RC_MASK_W)
			best_matchmask |= RC_MASK_W;

		split->Phase[split->NumPhases++] = best_matchmask;
		mask &= ~best_matchmask;
	}
}

const struct rc_swizzle_caps r300_swizzle_caps = {
	.IsNative = r300_swizzle_is_native,
	.Split = r300_swizzle_split
};


/**
 * Translate an RGB (XYZ) swizzle into the hardware code for the given
 * instruction source.
 */
unsigned int r300FPTranslateRGBSwizzle(unsigned int src, unsigned int swizzle)
{
	const struct swizzle_data* sd = lookup_native_swizzle(swizzle);

	if (!sd || (src == RC_PAIR_PRESUB_SRC && sd->srcp_stride == 0)) {
		fprintf(stderr, "Not a native swizzle: %08x\n", swizzle);
		return 0;
	}

	if (src == RC_PAIR_PRESUB_SRC) {
		return sd->base + sd->srcp_stride;
	} else {
		return sd->base + src*sd->stride;
	}
}


/**
 * Translate an Alpha (W) swizzle into the hardware code for the given
 * instruction source.
 */
unsigned int r300FPTranslateAlphaSwizzle(unsigned int src, unsigned int swizzle)
{
	unsigned int swz = GET_SWZ(swizzle, 0);
	if (src == RC_PAIR_PRESUB_SRC) {
		return R300_ALU_ARGA_SRCP_X + swz;
	}
	if (swz < 3)
		return swz + 3*src;

	switch(swz) {
	case RC_SWIZZLE_W: return R300_ALU_ARGA_SRC0A + src;
	case RC_SWIZZLE_ONE: return R300_ALU_ARGA_ONE;
	case RC_SWIZZLE_ZERO: return R300_ALU_ARGA_ZERO;
	case RC_SWIZZLE_HALF: return R300_ALU_ARGA_HALF;
	default: return R300_ALU_ARGA_ONE;
	}
}
