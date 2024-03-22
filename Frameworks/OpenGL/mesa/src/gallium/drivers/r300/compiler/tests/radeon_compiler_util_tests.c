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

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "radeon_compiler_util.h"
#include "radeon_program.h"

#include "r300_compiler_tests.h"
#include "rc_test_helpers.h"
#include "unit_test.h"

static void test_rc_inst_can_use_presub(
	struct test_result * result,
	int expected,
	const char * add_str,
	const char * replace_str)
{
	struct rc_instruction add_inst, replace_inst;
	int ret;

	struct r300_fragment_program_compiler c = {};
	init_compiler(&c.Base, RC_FRAGMENT_PROGRAM, 0, 0);

	test_begin(result);
	init_rc_normal_instruction(&add_inst, add_str);
	init_rc_normal_instruction(&replace_inst, replace_str);

	ret = rc_inst_can_use_presub(&c.Base, &replace_inst, RC_PRESUB_ADD, 0,
			&replace_inst.U.I.SrcReg[0],
			&add_inst.U.I.SrcReg[0], &add_inst.U.I.SrcReg[1]);

	test_check(result, ret == expected);

	destroy_compiler(&c.Base);
}

static void test_runner_rc_inst_can_use_presub(struct test_result * result)
{

	/* This tests the case where the source being replace has the same
	 * register file and register index as another source register in the
	 * CMP instruction.  A previous version of this function was ignoring
	 * all registers that shared the same file and index as the replacement
	 * register when counting the number of source selects.
	 *
	 * https://bugs.freedesktop.org/show_bug.cgi?id=36527
	 */
	test_rc_inst_can_use_presub(result, 0,
		"ADD temp[0].z, temp[6].__x_, const[1].__x_;",
		"CMP temp[0].y, temp[0]._z__, const[0]._z__, temp[0]._y__;");


	/* Testing a random case that should fail
	 *
	 * https://bugs.freedesktop.org/show_bug.cgi?id=36527
	 */
	test_rc_inst_can_use_presub(result, 0,
		"ADD temp[3], temp[1], temp[2];",
		"MAD temp[1], temp[0], const[0].xxxx, -temp[3];");

	/* This tests the case where the arguments of the ADD
	 * instruction share the same register file and index.  Normally, we
	 * would need only one source select for these two arguments, but since
	 * they will be part of a presubtract operation we need to use the two
	 * source selects that the presubtract instruction expects
	 * (src0 and src1).
	 *
	 * https://bugs.freedesktop.org/show_bug.cgi?id=36527
	 */
	test_rc_inst_can_use_presub(result, 0,
		"ADD temp[3].x, temp[0].x___, temp[0].x___;",
		"MAD temp[0].xyz, temp[2].xyz_, -temp[3].xxx_, input[5].xyz_;");
}

unsigned radeon_compiler_util_run_tests()
{
	static struct test tests[] = {
		{"rc_inst_can_use_presub()", test_runner_rc_inst_can_use_presub},
		{NULL, NULL}
	};
	return run_tests(tests);
}
