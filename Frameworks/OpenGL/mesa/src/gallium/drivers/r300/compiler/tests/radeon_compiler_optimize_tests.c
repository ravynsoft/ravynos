/*
 * Copyright 2013 Advanced Micro Devices, Inc.
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

#include <stdio.h>
#include "radeon_compiler.h"
#include "radeon_dataflow.h"

#include "r300_compiler_tests.h"
#include "rc_test_helpers.h"
#include "unit_test.h"

static unsigned test_rc_optimize(
	struct test_result * result,
	struct radeon_compiler * c,
	const char * filename)
{
	struct rc_test_file test_file;

	test_begin(result);

	if (!load_program(c, &test_file, filename)) {
		fprintf(stderr, "Failed to load program\n");
		return 0;
	}

	rc_optimize(c, NULL);
	return 1;
}

static void test_runner_rc_optimize(struct test_result * result)
{
	unsigned pass = 1;
	struct radeon_compiler c;
	struct rc_instruction *inst;
	struct rc_instruction *inst_list[3];
	unsigned inst_count = 0;
	float const0[4] = {2.0f, 0.0f, 0.0f, 0.0f};

	init_compiler(&c, RC_FRAGMENT_PROGRAM, 1, 0);

	rc_constants_add_immediate_vec4(&c.Program.Constants, const0);

	test_rc_optimize(result, &c, "omod_two_writers.test");

	for(inst = c.Program.Instructions.Next;
				inst != &c.Program.Instructions;
				inst = inst->Next, inst_count++) {
		inst_list[inst_count] = inst;
	}

	if (inst_list[0]->U.I.Omod != RC_OMOD_MUL_2 ||
			inst_list[1]->U.I.Omod != RC_OMOD_MUL_2 ||
			inst_list[2]->U.I.Opcode != RC_OPCODE_MOV) {
		pass = 0;
	}

	test_check(result, pass);

	destroy_compiler(&c);
}

unsigned radeon_compiler_optimize_run_tests()
{
	static struct test tests[] = {
		{"rc_optimize() => peephole_mul_omod()", test_runner_rc_optimize},
		{NULL, NULL}
	};
	return run_tests(tests);
}
