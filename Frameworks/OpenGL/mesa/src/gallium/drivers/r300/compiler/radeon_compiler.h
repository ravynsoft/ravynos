/*
 * Copyright 2009 Nicolai Hähnle <nhaehnle@gmail.com>
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
 * USE OR OTHER DEALINGS IN THE SOFTWARE. */

#ifndef RADEON_COMPILER_H
#define RADEON_COMPILER_H

#include <stdbool.h>

#include "memory_pool.h"
#include "radeon_code.h"
#include "radeon_program.h"

#define RC_DBG_LOG        (1 << 0)

struct rc_swizzle_caps;

enum rc_program_type {
	RC_VERTEX_PROGRAM,
	RC_FRAGMENT_PROGRAM,
	RC_NUM_PROGRAM_TYPES
};

struct radeon_compiler {
	struct memory_pool Pool;
	struct rc_program Program;
	const struct rc_regalloc_state *regalloc_state;
	struct util_debug_callback *debug;
	enum rc_program_type type;
	unsigned Debug:2;
	unsigned Error:1;
	char * ErrorMsg;

	/* Hardware specification. */
	unsigned is_r400:1;
	unsigned is_r500:1;
	unsigned has_half_swizzles:1;
	unsigned has_presub:1;
	unsigned has_omod:1;
	unsigned disable_optimizations:1;
	unsigned max_temp_regs;
	unsigned max_constants;
	int max_alu_insts;
	unsigned max_tex_insts;

	int max_temp_index;

	/* Whether to remove unused constants and empty holes in constant space. */
	unsigned remove_unused_constants:1;

	/**
	 * Variables used internally, not be touched by callers
	 * of the compiler
	 */
	/*@{*/
	const struct rc_swizzle_caps * SwizzleCaps;
	/*@}*/
};

void rc_init(struct radeon_compiler * c, const struct rc_regalloc_state *rs);
void rc_destroy(struct radeon_compiler * c);

void rc_debug(struct radeon_compiler * c, const char * fmt, ...);
void rc_error(struct radeon_compiler * c, const char * fmt, ...);

int rc_if_fail_helper(struct radeon_compiler * c, const char * file, int line, const char * assertion);

/**
 * This macro acts like an if-statement that can be used to implement
 * non-aborting assertions in the compiler.
 *
 * It checks whether \p cond is true. If not, an internal compiler error is
 * flagged and the if-clause is run.
 *
 * A typical use-case would be:
 *
 *  if (rc_assert(c, condition-that-must-be-true))
 *  	return;
 */
#define rc_assert(c, cond) \
	(!(cond) && rc_if_fail_helper(c, __FILE__, __LINE__, #cond))

void rc_calculate_inputs_outputs(struct radeon_compiler * c);

void rc_copy_output(struct radeon_compiler * c, unsigned output, unsigned dup_output);
void rc_transform_fragment_wpos(struct radeon_compiler * c, unsigned wpos, unsigned new_input,
                                int full_vtransform);
void rc_transform_fragment_face(struct radeon_compiler *c, unsigned face);

struct r300_fragment_program_compiler {
	struct radeon_compiler Base;
	struct rX00_fragment_program_code *code;
	/* Optional transformations and features. */
	struct r300_fragment_program_external_state state;
	/* Register corresponding to the depthbuffer. */
	unsigned OutputDepth;
	/* Registers corresponding to the four colorbuffers. */
	unsigned OutputColor[4];

	void * UserData;
	void (*AllocateHwInputs)(
		struct r300_fragment_program_compiler * c,
		void (*allocate)(void * data, unsigned input, unsigned hwreg),
		void * mydata);
};

void r3xx_compile_fragment_program(struct r300_fragment_program_compiler* c);

struct r300_vertex_program_compiler {
	struct radeon_compiler Base;
	struct r300_vertex_program_code *code;
	uint32_t RequiredOutputs;

	void * UserData;
	void (*SetHwInputOutput)(struct r300_vertex_program_compiler * c);

};

void r3xx_compile_vertex_program(struct r300_vertex_program_compiler* c);
void rc_vert_fc(struct radeon_compiler *compiler, void *user);
void r300_vertex_program_dump(struct radeon_compiler *compiler, void *user);

struct radeon_compiler_pass {
	const char *name;	/* Name of the pass. */
	int dump;		/* Dump the program if Debug == 1? */
	int predicate;		/* Run this pass? */
	void (*run)(struct radeon_compiler *c, void *user); /* The main entrypoint. */
	void *user;		/* Optional parameter which is passed to the run function. */
};

struct rc_program_stats {
	unsigned num_cycles;
	unsigned num_consts;
	unsigned num_insts;
	unsigned num_fc_insts;
	unsigned num_tex_insts;
	unsigned num_rgb_insts;
	unsigned num_alpha_insts;
	unsigned num_pred_insts;
	unsigned num_presub_ops;
	unsigned num_temp_regs;
	unsigned num_omod_ops;
	unsigned num_inline_literals;
	unsigned num_loops;
};

void rc_get_stats(struct radeon_compiler *c, struct rc_program_stats *s);

/* Executes a list of compiler passes given in the parameter 'list'. */
bool rc_run_compiler_passes(struct radeon_compiler *c, struct radeon_compiler_pass *list);
void rc_run_compiler(struct radeon_compiler *c, struct radeon_compiler_pass *list);
void rc_validate_final_shader(struct radeon_compiler *c, void *user);

#endif /* RADEON_COMPILER_H */
