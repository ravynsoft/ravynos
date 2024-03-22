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

#ifndef RADEON_PROGRAM_CONSTANTS_H
#define RADEON_PROGRAM_CONSTANTS_H

typedef enum {
	RC_SATURATE_NONE = 0,
	RC_SATURATE_ZERO_ONE,
	RC_SATURATE_MINUS_PLUS_ONE
} rc_saturate_mode;

typedef enum {
	RC_TEXTURE_2D_ARRAY,
	RC_TEXTURE_1D_ARRAY,
	RC_TEXTURE_CUBE,
	RC_TEXTURE_3D,
	RC_TEXTURE_RECT,
	RC_TEXTURE_2D,
	RC_TEXTURE_1D
} rc_texture_target;

typedef enum {
	/**
	 * Used to indicate unused register descriptions and
	 * source register that use a constant swizzle.
	 */
	RC_FILE_NONE = 0,
	RC_FILE_TEMPORARY,

	/**
	 * Input register.
	 *
	 * \note The compiler attaches no implicit semantics to input registers.
	 * Fragment/vertex program specific semantics must be defined explicitly
	 * using the appropriate compiler interfaces.
	 */
	RC_FILE_INPUT,

	/**
	 * Output register.
	 *
	 * \note The compiler attaches no implicit semantics to input registers.
	 * Fragment/vertex program specific semantics must be defined explicitly
	 * using the appropriate compiler interfaces.
	 */
	RC_FILE_OUTPUT,
	RC_FILE_ADDRESS,

	/**
	 * Indicates a constant from the \ref rc_constant_list .
	 */
	RC_FILE_CONSTANT,

	/**
	 * Indicates a special register, see RC_SPECIAL_xxx.
	 */
	RC_FILE_SPECIAL,

	/**
	 * Indicates this register should use the result of the presubtract
	 * operation.
	 */
	RC_FILE_PRESUB,

	/**
	 * Indicates that the source index has been encoded as a 7-bit float.
	 */
	RC_FILE_INLINE
} rc_register_file;

enum {
	/** R500 fragment program ALU result "register" */
	RC_SPECIAL_ALU_RESULT = 0,

	/** Must be last */
	RC_NUM_SPECIAL_REGISTERS
};

#define RC_REGISTER_INDEX_BITS 11
#define RC_REGISTER_MAX_INDEX (1 << RC_REGISTER_INDEX_BITS)

typedef enum {
	RC_SWIZZLE_X = 0,
	RC_SWIZZLE_Y,
	RC_SWIZZLE_Z,
	RC_SWIZZLE_W,
	RC_SWIZZLE_ZERO,
	RC_SWIZZLE_ONE,
	RC_SWIZZLE_HALF,
	RC_SWIZZLE_UNUSED
} rc_swizzle;

static inline int is_swizzle_inline_constant(rc_swizzle swizzle){
	return swizzle >= RC_SWIZZLE_ZERO;

}

#define RC_MAKE_SWIZZLE(a,b,c,d) (((a)<<0) | ((b)<<3) | ((c)<<6) | ((d)<<9))
#define RC_MAKE_SWIZZLE_SMEAR(a) RC_MAKE_SWIZZLE((a),(a),(a),(a))
#define GET_SWZ(swz, idx)      (((swz) >> ((idx)*3)) & 0x7)
#define GET_BIT(msk, idx)      (((msk) >> (idx)) & 0x1)
#define SET_SWZ(swz, idx, newv) \
	do { \
		(swz) = ((swz) & ~(7 << ((idx)*3))) | ((newv) << ((idx)*3)); \
	} while(0)

#define RC_SWIZZLE_XYZW RC_MAKE_SWIZZLE(RC_SWIZZLE_X, RC_SWIZZLE_Y, RC_SWIZZLE_Z, RC_SWIZZLE_W)
#define RC_SWIZZLE_XYZ0 RC_MAKE_SWIZZLE(RC_SWIZZLE_X, RC_SWIZZLE_Y, RC_SWIZZLE_Z, RC_SWIZZLE_ZERO)
#define RC_SWIZZLE_XYZ1 RC_MAKE_SWIZZLE(RC_SWIZZLE_X, RC_SWIZZLE_Y, RC_SWIZZLE_Z, RC_SWIZZLE_ONE)
#define RC_SWIZZLE_XYZZ RC_MAKE_SWIZZLE(RC_SWIZZLE_X, RC_SWIZZLE_Y, RC_SWIZZLE_Z, RC_SWIZZLE_Z)
#define RC_SWIZZLE_XXXX RC_MAKE_SWIZZLE_SMEAR(RC_SWIZZLE_X)
#define RC_SWIZZLE_YYYY RC_MAKE_SWIZZLE_SMEAR(RC_SWIZZLE_Y)
#define RC_SWIZZLE_ZZZZ RC_MAKE_SWIZZLE_SMEAR(RC_SWIZZLE_Z)
#define RC_SWIZZLE_WWWW RC_MAKE_SWIZZLE_SMEAR(RC_SWIZZLE_W)
#define RC_SWIZZLE_0000 RC_MAKE_SWIZZLE_SMEAR(RC_SWIZZLE_ZERO)
#define RC_SWIZZLE_1111 RC_MAKE_SWIZZLE_SMEAR(RC_SWIZZLE_ONE)
#define RC_SWIZZLE_HHHH RC_MAKE_SWIZZLE_SMEAR(RC_SWIZZLE_HALF)
#define RC_SWIZZLE_UUUU RC_MAKE_SWIZZLE_SMEAR(RC_SWIZZLE_UNUSED)

/**
 * \name Bitmasks for components of vectors.
 *
 * Used for write masks, negation masks, etc.
 */
/*@{*/
#define RC_MASK_NONE 0
#define RC_MASK_X 1
#define RC_MASK_Y 2
#define RC_MASK_Z 4
#define RC_MASK_W 8
#define RC_MASK_XY (RC_MASK_X|RC_MASK_Y)
#define RC_MASK_XYZ (RC_MASK_X|RC_MASK_Y|RC_MASK_Z)
#define RC_MASK_XYW (RC_MASK_X|RC_MASK_Y|RC_MASK_W)
#define RC_MASK_XYZW (RC_MASK_X|RC_MASK_Y|RC_MASK_Z|RC_MASK_W)
/*@}*/

typedef enum {
	RC_ALURESULT_NONE = 0,
	RC_ALURESULT_X,
	RC_ALURESULT_W
} rc_write_aluresult;

typedef enum {
	RC_PRESUB_NONE = 0,

	/** 1 - 2 * src0 */
	RC_PRESUB_BIAS,

	/** src1 - src0 */
	RC_PRESUB_SUB,

	/** src1 + src0 */
	RC_PRESUB_ADD,

	/** 1 - src0 */
	RC_PRESUB_INV
} rc_presubtract_op;

typedef enum {
	RC_OMOD_MUL_1,
	RC_OMOD_MUL_2,
	RC_OMOD_MUL_4,
	RC_OMOD_MUL_8,
	RC_OMOD_DIV_2,
	RC_OMOD_DIV_4,
	RC_OMOD_DIV_8,
	RC_OMOD_DISABLE
} rc_omod_op;

static inline int rc_presubtract_src_reg_count(rc_presubtract_op op){
	switch(op){
	case RC_PRESUB_BIAS:
	case RC_PRESUB_INV:
		return 1;
	case RC_PRESUB_ADD:
	case RC_PRESUB_SUB:
		return 2;
	default:
		return 0;
	}
}

#define RC_SOURCE_NONE  0x0
#define RC_SOURCE_RGB   0x1
#define RC_SOURCE_ALPHA 0x2

typedef enum {
	RC_PRED_DISABLED,
	RC_PRED_SET,
	RC_PRED_INV
} rc_predicate_mode;

#endif /* RADEON_PROGRAM_CONSTANTS_H */
