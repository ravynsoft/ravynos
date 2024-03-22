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

#ifndef RADEON_SWIZZLE_H
#define RADEON_SWIZZLE_H

#include "radeon_program.h"

struct rc_swizzle_split {
	unsigned char NumPhases;
	unsigned char Phase[4];
};

/**
 * Describe the swizzling capability of target hardware.
 */
struct rc_swizzle_caps {
	/**
	 * Check whether the given swizzle, absolute and negate combination
	 * can be implemented natively by the hardware for this opcode.
	 *
	 * \return 1 if the swizzle is native for the given opcode
	 */
	int (*IsNative)(rc_opcode opcode, struct rc_src_register reg);

	/**
	 * Determine how to split access to the masked channels of the
	 * given source register to obtain ALU-native swizzles.
	 */
	void (*Split)(struct rc_src_register reg, unsigned int mask, struct rc_swizzle_split * split);
};

extern const struct rc_swizzle_caps r300_vertprog_swizzle_caps;

#endif /* RADEON_SWIZZLE_H */
