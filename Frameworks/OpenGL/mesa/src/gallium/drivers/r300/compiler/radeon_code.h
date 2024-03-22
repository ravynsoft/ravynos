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

#ifndef RADEON_CODE_H
#define RADEON_CODE_H

#include <stdint.h>

#define R300_PFS_MAX_ALU_INST     64
#define R300_PFS_MAX_TEX_INST     32
#define R300_PFS_MAX_TEX_INDIRECT 4
#define R300_PFS_NUM_TEMP_REGS    32
#define R300_PFS_NUM_CONST_REGS   32

#define R400_PFS_MAX_ALU_INST     512
#define R400_PFS_MAX_TEX_INST     512

#define R500_PFS_MAX_INST         512
#define R500_PFS_NUM_TEMP_REGS    128
#define R500_PFS_NUM_CONST_REGS   256
#define R500_PFS_MAX_BRANCH_DEPTH_FULL 32
#define R500_PFS_MAX_BRANCH_DEPTH_PARTIAL 4

/* The r500 maximum depth is not just for loops, but any combination of loops
 * and subroutine jumps. */
#define R500_PVS_MAX_LOOP_DEPTH 8

#define STATE_R300_WINDOW_DIMENSION (STATE_INTERNAL_DRIVER+0)

enum {
	/**
	 * External constants are constants whose meaning is unknown to this
	 * compiler. For example, a Mesa gl_program's constants are turned
	 * into external constants.
	 */
	RC_CONSTANT_EXTERNAL = 0,

	RC_CONSTANT_IMMEDIATE,

	/**
	 * Constant referring to state that is known by this compiler,
	 * see RC_STATE_xxx, i.e. *not* arbitrary Mesa (or other) state.
	 */
	RC_CONSTANT_STATE
};

enum {
	RC_STATE_SHADOW_AMBIENT = 0,

	RC_STATE_R300_WINDOW_DIMENSION,
	RC_STATE_R300_TEXRECT_FACTOR,
	RC_STATE_R300_TEXSCALE_FACTOR,
	RC_STATE_R300_VIEWPORT_SCALE,
	RC_STATE_R300_VIEWPORT_OFFSET
};

struct rc_constant {
	unsigned Type:2; /**< RC_CONSTANT_xxx */
	unsigned Size:3;

	union {
		unsigned External;
		float Immediate[4];
		unsigned State[2];
	} u;
};

struct rc_constant_list {
	struct rc_constant * Constants;
	unsigned Count;

	unsigned _Reserved;
};

void rc_constants_init(struct rc_constant_list * c);
void rc_constants_copy(struct rc_constant_list * dst, struct rc_constant_list * src);
void rc_constants_destroy(struct rc_constant_list * c);
unsigned rc_constants_add(struct rc_constant_list * c, struct rc_constant * constant);
unsigned rc_constants_add_state(struct rc_constant_list * c, unsigned state1, unsigned state2);
unsigned rc_constants_add_immediate_vec4(struct rc_constant_list * c, const float * data);
unsigned rc_constants_add_immediate_scalar(struct rc_constant_list * c, float data, unsigned * swizzle);
void rc_constants_print(struct rc_constant_list * c);

/**
 * Compare functions.
 *
 * \note By design, RC_COMPARE_FUNC_xxx + GL_NEVER gives you
 * the correct GL compare function.
 */
typedef enum {
	RC_COMPARE_FUNC_NEVER = 0,
	RC_COMPARE_FUNC_LESS,
	RC_COMPARE_FUNC_EQUAL,
	RC_COMPARE_FUNC_LEQUAL,
	RC_COMPARE_FUNC_GREATER,
	RC_COMPARE_FUNC_NOTEQUAL,
	RC_COMPARE_FUNC_GEQUAL,
	RC_COMPARE_FUNC_ALWAYS
} rc_compare_func;

/**
 * Coordinate wrapping modes.
 *
 * These are not quite the same as their GL counterparts yet.
 */
typedef enum {
	RC_WRAP_NONE = 0,
	RC_WRAP_REPEAT,
	RC_WRAP_MIRRORED_REPEAT,
	RC_WRAP_MIRRORED_CLAMP
} rc_wrap_mode;

/**
 * Stores state that influences the compilation of a fragment program.
 */
struct r300_fragment_program_external_state {
	struct {
		/**
		 * This field contains swizzle for some lowering passes
		 * (shadow comparison, unorm->snorm conversion)
		 */
		unsigned texture_swizzle:12;

		/**
		 * If the sampler is used as a shadow sampler,
		 * this field specifies the compare function.
		 *
		 * Otherwise, this field is \ref RC_COMPARE_FUNC_NEVER (aka 0).
		 * \sa rc_compare_func
		 */
		unsigned texture_compare_func : 3;

		/**
		 * No matter what the sampler type is,
		 * this field turns it into a shadow sampler.
		 */
		unsigned compare_mode_enabled : 1;

		/**
		 * This field specifies wrapping modes for the sampler.
		 *
		 * If this field is \ref RC_WRAP_NONE (aka 0), no wrapping maths
		 * will be performed on the coordinates.
		 */
		unsigned wrap_mode : 3;

		/**
		 * The coords are scaled after applying the wrap mode emulation
		 * and right before texture fetch. The scaling factor is given by
		 * RC_STATE_R300_TEXSCALE_FACTOR. */
		unsigned clamp_and_scale_before_fetch : 1;
	} unit[16];

	unsigned alpha_to_one:1;
};



struct r300_fragment_program_node {
	int tex_offset; /**< first tex instruction */
	int tex_end; /**< last tex instruction, relative to tex_offset */
	int alu_offset; /**< first ALU instruction */
	int alu_end; /**< last ALU instruction, relative to alu_offset */
	int flags;
};

/**
 * Stores an R300 fragment program in its compiled-to-hardware form.
 */
struct r300_fragment_program_code {
	struct {
		unsigned int length; /**< total # of texture instructions used */
		uint32_t inst[R400_PFS_MAX_TEX_INST];
	} tex;

	struct {
		unsigned int length; /**< total # of ALU instructions used */
		struct {
			uint32_t rgb_inst;
			uint32_t rgb_addr;
			uint32_t alpha_inst;
			uint32_t alpha_addr;
			uint32_t r400_ext_addr;
		} inst[R400_PFS_MAX_ALU_INST];
	} alu;

	uint32_t config; /* US_CONFIG */
	uint32_t pixsize; /* US_PIXSIZE */
	uint32_t code_offset; /* US_CODE_OFFSET */
	uint32_t r400_code_offset_ext; /* US_CODE_EXT */
	uint32_t code_addr[4]; /* US_CODE_ADDR */
	/*US_CODE_BANK.R390_MODE: Enables 512 instructions and 64 temporaries
	 * for r400 cards */
	unsigned int r390_mode:1;
};


struct r500_fragment_program_code {
	struct {
		uint32_t inst0;
		uint32_t inst1;
		uint32_t inst2;
		uint32_t inst3;
		uint32_t inst4;
		uint32_t inst5;
	} inst[R500_PFS_MAX_INST];

	int inst_end; /* Number of instructions - 1; also, last instruction to be executed */

	int max_temp_idx;

	uint32_t us_fc_ctrl;

	uint32_t int_constants[32];
	uint32_t int_constant_count;
};

struct rX00_fragment_program_code {
	union {
		struct r300_fragment_program_code r300;
		struct r500_fragment_program_code r500;
	} code;

	unsigned writes_depth:1;

	struct rc_constant_list constants;
	unsigned *constants_remap_table;
};


#define R300_VS_MAX_ALU		256
#define R300_VS_MAX_ALU_DWORDS  (R300_VS_MAX_ALU * 4)
#define R500_VS_MAX_ALU	        1024
#define R500_VS_MAX_ALU_DWORDS  (R500_VS_MAX_ALU * 4)
#define R300_VS_MAX_TEMPS	32
/* This is the max for all chipsets (r300-r500) */
#define R300_VS_MAX_FC_OPS 16
#define R300_VS_MAX_LOOP_DEPTH 1

#define VSF_MAX_INPUTS 32
#define VSF_MAX_OUTPUTS 32

struct r300_vertex_program_code {
	int length;
	union {
		uint32_t d[R500_VS_MAX_ALU_DWORDS];
		float f[R500_VS_MAX_ALU_DWORDS];
	} body;

	int pos_end;
	int num_temporaries;	/* Number of temp vars used by program */
	int inputs[VSF_MAX_INPUTS];
	int outputs[VSF_MAX_OUTPUTS];
	unsigned last_input_read;
	unsigned last_pos_write;

	struct rc_constant_list constants;
	unsigned *constants_remap_table;

	uint32_t InputsRead;
	uint32_t OutputsWritten;

	unsigned int num_fc_ops;
	uint32_t fc_ops;
	union {
	        uint32_t r300[R300_VS_MAX_FC_OPS];
		struct {
			uint32_t lw;
			uint32_t uw;
		} r500[R300_VS_MAX_FC_OPS];
	} fc_op_addrs;
	int32_t fc_loop_index[R300_VS_MAX_FC_OPS];
};

#endif /* RADEON_CODE_H */

