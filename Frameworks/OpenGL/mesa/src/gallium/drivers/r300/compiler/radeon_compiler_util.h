/*
 * Copyright 2010 Tom Stellard <tstellar@gmail.com>
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

#include "radeon_program_constants.h"

#ifndef RADEON_PROGRAM_UTIL_H
#define RADEON_PROGRAM_UTIL_H

#include <stdbool.h>

#include "radeon_opcodes.h"

struct radeon_compiler;
struct rc_instruction;
struct rc_pair_instruction;
struct rc_pair_sub_instruction;
struct rc_src_register;

unsigned int rc_swizzle_to_writemask(unsigned int swz);

rc_swizzle get_swz(unsigned int swz, rc_swizzle idx);

unsigned int rc_init_swizzle(unsigned int initial_value, unsigned int channels);

unsigned int combine_swizzles4(unsigned int src,
			       rc_swizzle swz_x, rc_swizzle swz_y,
			       rc_swizzle swz_z, rc_swizzle swz_w);

unsigned int combine_swizzles(unsigned int src, unsigned int swz);

rc_swizzle rc_mask_to_swizzle(unsigned int mask);

unsigned swizzle_mask(unsigned swizzle, unsigned mask);

unsigned int rc_adjust_channels(
	unsigned int old_swizzle,
	unsigned int conversion_swizzle);

void rc_pair_rewrite_writemask(
	struct rc_pair_sub_instruction * sub,
	unsigned int conversion_swizzle);

void rc_normal_rewrite_writemask(
	struct rc_instruction * inst,
	unsigned int conversion_swizzle);

unsigned int rc_rewrite_swizzle(
	unsigned int swizzle,
	unsigned int new_mask);

struct rc_src_register lmul_swizzle(unsigned int swizzle, struct rc_src_register srcreg);

void reset_srcreg(struct rc_src_register* reg);

unsigned int rc_src_reads_dst_mask(
		rc_register_file src_file,
		unsigned int src_idx,
		unsigned int src_swz,
		rc_register_file dst_file,
		unsigned int dst_idx,
		unsigned int dst_mask);

unsigned int rc_source_type_swz(unsigned int swizzle);

unsigned int rc_source_type_mask(unsigned int mask);

unsigned int rc_inst_can_use_presub(
	struct radeon_compiler * c,
	struct rc_instruction * inst,
	rc_presubtract_op presub_op,
	unsigned int presub_writemask,
	const struct rc_src_register * replace_reg,
	const struct rc_src_register * presub_src0,
	const struct rc_src_register * presub_src1);

int rc_get_max_index(
	struct radeon_compiler * c,
	rc_register_file file);

void rc_pair_remove_src(struct rc_instruction * inst,
	unsigned int src_type,
	unsigned int source);

rc_opcode rc_get_flow_control_inst(struct rc_instruction * inst);

struct rc_instruction * rc_match_endloop(struct rc_instruction * endloop);
struct rc_instruction * rc_match_bgnloop(struct rc_instruction * bgnloop);

unsigned int rc_make_conversion_swizzle(
	unsigned int old_mask,
	unsigned int new_mask);

unsigned int rc_src_reg_is_immediate(
	struct radeon_compiler * c,
	unsigned int file,
	unsigned int index);

float rc_get_constant_value(
	struct radeon_compiler * c,
	unsigned int index,
	unsigned int swizzle,
	unsigned int negate,
	unsigned int chan);

unsigned int rc_get_scalar_src_swz(unsigned int swizzle);

bool rc_inst_has_three_diff_temp_srcs(struct rc_instruction *inst);
#endif /* RADEON_PROGRAM_UTIL_H */
