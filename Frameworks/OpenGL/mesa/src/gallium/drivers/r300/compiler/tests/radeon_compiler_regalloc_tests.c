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

#include "radeon_program_pair.h"

#include "r300_compiler_tests.h"
#include "rc_test_helpers.h"
#include "unit_test.h"

static void dummy_allocate_hw_inputs(
	struct r300_fragment_program_compiler * c,
	void (*allocate)(void * data, unsigned input, unsigned hwreg),
	void * mydata)
{
	unsigned i;
	for (i = 0; i < 10; i++) {
		allocate(mydata, i, i);
	}
}

static void test_runner_rc_regalloc(
	struct test_result *result,
	struct radeon_compiler *c,
	const char *filename)
{
	struct rc_test_file test_file;
	unsigned optimizations = 1;
	unsigned do_full_regalloc = 1;
	struct rc_instruction *inst;
	unsigned pass = 1;

	test_begin(result);

	if (!load_program(c, &test_file, filename)) {
		fprintf(stderr, "Failed to load program\n");
	}

	rc_pair_translate(c, NULL);
	rc_pair_schedule(c, &optimizations);
	rc_pair_remove_dead_sources(c, NULL);
	rc_pair_regalloc(c, &do_full_regalloc);

	for(inst = c->Program.Instructions.Next;
				inst != &c->Program.Instructions;
				inst = inst->Next) {
		if (inst->Type == RC_INSTRUCTION_NORMAL &&
				inst->U.I.Opcode != RC_OPCODE_BEGIN_TEX) {
			if (GET_SWZ(inst->U.I.SrcReg[0].Swizzle, 0)
							!= RC_SWIZZLE_X) {
				pass = 0;
			}
		}
	}

	test_check(result, pass);
}

static void tex_1d_swizzle(struct test_result *result)
{
	struct r300_fragment_program_compiler c;

	memset(&c, 0, sizeof(c));
	init_compiler(&c.Base, RC_FRAGMENT_PROGRAM, 0, 0);
	c.AllocateHwInputs = dummy_allocate_hw_inputs;

	test_runner_rc_regalloc(result, &c.Base, "regalloc_tex_1d_swizzle.test");

	destroy_compiler(&c.Base);
}

unsigned radeon_compiler_regalloc_run_tests()
{
	static struct test tests[] = {
		{"rc_pair_regalloc() => TEX 1D Swizzle - r300", tex_1d_swizzle },
		{NULL, NULL}
	};
	return run_tests(tests);
}
