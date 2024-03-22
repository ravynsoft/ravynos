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
#include "radeon_swizzle.h"
#include "util/u_bitcast.h"
#include <stdio.h>

#define VERBOSE 0

#define DBG(...) do { if (VERBOSE) fprintf(stderr, __VA_ARGS__); } while(0)

/* IEEE-754:
 * 22:0 mantissa
 * 30:23 exponent
 * 31 sign
 *
 * R300:
 * 0:2 mantissa
 * 3:6 exponent (bias 7)
 */
static int ieee_754_to_r300_float(float f, unsigned char *r300_float_out)
{
	unsigned float_bits = u_bitcast_f2u(f);
	/* XXX: Handle big-endian */
	unsigned mantissa = float_bits &         0x007fffff;
	unsigned biased_exponent = (float_bits & 0x7f800000) >> 23;
	unsigned negate = !!(float_bits &         0x80000000);
	int exponent = biased_exponent - 127;
	unsigned mantissa_mask = 0xff8fffff;
	unsigned r300_exponent, r300_mantissa;

	DBG("Converting %f (0x%x) to 7-bit:\n", f, float_bits);
	DBG("Raw exponent = %d\n", exponent);

	if (exponent < -7 || exponent > 8) {
		DBG("Failed exponent out of range\n\n");
		return 0;
	}

	if (mantissa & mantissa_mask) {
		DBG("Failed mantissa has too many bits:\n"
			"mantissa=0x%x mantissa_mask=0x%x, and=0x%x\n\n",
			mantissa, mantissa_mask,
			mantissa & mantissa_mask);
		return 0;
	}

	r300_exponent = exponent + 7;
	r300_mantissa = (mantissa & ~mantissa_mask) >> 20;
	*r300_float_out = r300_mantissa | (r300_exponent << 3);

	DBG("Success! r300_float = 0x%x\n\n", *r300_float_out);

	if (negate)
		return -1;
	else
		return 1;
}

void rc_inline_literals(struct radeon_compiler *c, void *user)
{
	struct rc_instruction * inst;

	for(inst = c->Program.Instructions.Next;
					inst != &c->Program.Instructions;
					inst = inst->Next) {
		const struct rc_opcode_info * info =
					rc_get_opcode_info(inst->U.I.Opcode);

		unsigned src_idx;
		struct rc_constant * constant;
		float float_value;
		unsigned char r300_float = 0;
		int ret;

		/* XXX: Handle presub */

		/* We aren't using rc_for_all_reads_src here, because presub
		 * sources need to be handled differently. */
		for (src_idx = 0; src_idx < info->NumSrcRegs; src_idx++) {
			unsigned use_literal = 0;
			unsigned swz, chan;
			struct rc_src_register src_reg = inst->U.I.SrcReg[src_idx];
			if (src_reg.File != RC_FILE_CONSTANT) {
				continue;
			}
			constant =
				&c->Program.Constants.Constants[src_reg.Index];
			if (constant->Type != RC_CONSTANT_IMMEDIATE) {
				continue;
			}
			for (chan = 0; chan < 4; chan++) {
				unsigned char r300_float_tmp;
				swz = GET_SWZ(src_reg.Swizzle, chan);
				if (swz >= RC_SWIZZLE_ZERO) {
					continue;
				}
				float_value = constant->u.Immediate[swz];
				ret = ieee_754_to_r300_float(float_value,
								&r300_float_tmp);
				if (!ret || (use_literal &&
						r300_float != r300_float_tmp)) {
					use_literal = 0;
					break;
				}

				if (ret == -1 && src_reg.Abs) {
					use_literal = 0;
					break;
				}

				if (!use_literal) {
					r300_float = r300_float_tmp;
					use_literal = 1;
				}

				/* Use RC_SWIZZLE_W for the inline constant, so
				 * it will become one of the alpha sources. */
				SET_SWZ(src_reg.Swizzle, chan, RC_SWIZZLE_W);
				if (ret == -1) {
					src_reg.Negate ^= (1 << chan);
				}
			}

			src_reg.File = RC_FILE_INLINE;
			src_reg.Index = r300_float;
			if (!use_literal || !c->SwizzleCaps->IsNative(inst->U.I.Opcode, src_reg)) {
				continue;
			}
			inst->U.I.SrcReg[src_idx] = src_reg;
		}
	}
}
