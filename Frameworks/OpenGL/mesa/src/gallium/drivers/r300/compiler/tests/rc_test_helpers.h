/*
 * Copyright 2011 Tom Stellard <tstellar@gmail.com>
 * Copyright 2013 Advanced Micro Devices, Inc.
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
 * Author: Tom Stellard <thomas.stellard@amd.com>
 */

#include "radeon_compiler.h"

struct rc_test_file {
	unsigned num_input_lines;
	char **input;
	unsigned num_expected_lines;
	char **expected;
};

int init_rc_normal_src(
	struct rc_instruction * inst,
	unsigned int src_index,
	const char * src_str);

int init_rc_normal_dst(
	struct rc_instruction * inst,
	const char * dst_str);

int parse_rc_normal_instruction(
	struct rc_instruction * inst,
	const char * inst_str);

int parse_constant(unsigned *index, float *data, const char *const_str);

int init_rc_normal_instruction(
	struct rc_instruction * inst,
	const char * inst_str);

void add_instruction(struct radeon_compiler *c, const char * inst_string);

int add_constant(struct radeon_compiler *c, const char *const_str);

void init_compiler(
	struct radeon_compiler *c,
	enum rc_program_type program_type,
	unsigned is_r500,
	unsigned is_r400);

void destroy_compiler(struct radeon_compiler *c);

unsigned load_program(
	struct radeon_compiler *c,
	struct rc_test_file *test,
	const char *filename);
