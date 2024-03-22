/*
 * Copyright © 2020 Google, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "util/log.h"
#include "util/u_math.h"

#include "ir3/ir3.h"
#include "ir3/ir3_shader.h"
#include "ir3/instr-a3xx.h"  // TODO move opc's and other useful things to ir3-instr.h or so

#include "isa.h"

struct bitset_params;

struct encode_state {
	unsigned gen;

	struct ir3_compiler *compiler;

	/**
	 * The instruction which is currently being encoded
	 */
	struct ir3_instruction *instr;
};

/*
 * Helpers defining how to map from ir3_instruction/ir3_register/etc to fields
 * to be encoded:
 */

static inline bool
extract_SRC1_R(struct ir3_instruction *instr)
{
	if (instr->nop) {
		assert(!instr->repeat);
		return instr->nop & 0x1;
	}
	return !!(instr->srcs[0]->flags & IR3_REG_R);
}

static inline bool
extract_SRC2_R(struct ir3_instruction *instr)
{
	if (instr->nop) {
		assert(!instr->repeat);
		return (instr->nop >> 1) & 0x1;
	}
	/* src2 does not appear in all cat2, but SRC2_R does (for nop encoding) */
	if (instr->srcs_count > 1)
		return !!(instr->srcs[1]->flags & IR3_REG_R);
	return 0;
}

static inline opc_t
__instruction_case(struct encode_state *s, struct ir3_instruction *instr)
{
	/*
	 * Temporary hack.. the new world doesn't map opcodes directly to hw
	 * encoding, so there are some cases where we need to fixup the opc
	 * to match what the encoder expects.  Eventually this will go away
	 * once we completely transition away from the packed-struct encoding/
	 * decoding and split up things which are logically different
	 * instructions
	 */
	if (instr->opc == OPC_B) {
		switch (instr->cat0.brtype) {
		case BRANCH_PLAIN:
			return OPC_BR;
		case BRANCH_OR:
			return OPC_BRAO;
		case BRANCH_AND:
			return OPC_BRAA;
		case BRANCH_CONST:
			return OPC_BRAC;
		case BRANCH_ANY:
			return OPC_BANY;
		case BRANCH_ALL:
			return OPC_BALL;
		case BRANCH_X:
			return OPC_BRAX;
		}
	} else if (instr->opc == OPC_MOV) {
		struct ir3_register *src = instr->srcs[0];
		if (src->flags & IR3_REG_IMMED) {
			return OPC_MOV_IMMED;
		} if (src->flags & IR3_REG_RELATIV) {
			if (src->flags & IR3_REG_CONST) {
				return OPC_MOV_RELCONST;
			} else {
				return OPC_MOV_RELGPR;
			}
		} else if (src->flags & IR3_REG_CONST) {
			return OPC_MOV_CONST;
		} else {
			return OPC_MOV_GPR;
		}
	} else if (instr->opc == OPC_DEMOTE) {
		return OPC_KILL;
	} else if (s->compiler->gen >= 6) {
		if (instr->opc == OPC_RESINFO) {
			return OPC_RESINFO_B;
		} else if (instr->opc == OPC_LDIB) {
			return OPC_LDIB_B;
		} else if (instr->opc == OPC_STIB) {
			return OPC_STIB_B;
		}
	}
	return instr->opc;
}

static inline unsigned
extract_ABSNEG(struct ir3_register *reg)
{
	// TODO generate enums for this:
	if (reg->flags & (IR3_REG_FNEG | IR3_REG_SNEG | IR3_REG_BNOT)) {
		if (reg->flags & (IR3_REG_FABS | IR3_REG_SABS)) {
			return 3; // ABSNEG
		} else {
			return 1; // NEG
		}
	} else if (reg->flags & (IR3_REG_FABS | IR3_REG_SABS)) {
		return 2; // ABS
	} else {
		return 0;
	}
}

static inline int32_t
extract_reg_iim(struct ir3_register *reg)
{
   assert(reg->flags & IR3_REG_IMMED);
   return reg->iim_val;
}

static inline uint32_t
extract_reg_uim(struct ir3_register *reg)
{
   assert(reg->flags & IR3_REG_IMMED);
   return reg->uim_val;
}

/**
 * This is a bit messy, to deal with the fact that the optional "s2en"
 * src is the first src, shifting everything else up by one.
 *
 * TODO revisit this once legacy 'packed struct' encoding is gone
 */
static inline struct ir3_register *
extract_cat5_SRC(struct ir3_instruction *instr, unsigned n)
{
	if (instr->flags & IR3_INSTR_S2EN) {
		n++;
	}
	if (n < instr->srcs_count)
		return instr->srcs[n];
	return NULL;
}

static inline bool
extract_cat5_FULL(struct ir3_instruction *instr)
{
	struct ir3_register *reg = extract_cat5_SRC(instr, 0);
	/* some cat5 have zero src regs, in which case 'FULL' is false */
	if (!reg)
		return false;
	return !(reg->flags & IR3_REG_HALF);
}

static inline cat5_desc_mode_t
extract_cat5_DESC_MODE(struct ir3_instruction *instr)
{
	assert(instr->flags & (IR3_INSTR_S2EN | IR3_INSTR_B));
	if (instr->flags & IR3_INSTR_S2EN) {
		if (instr->flags & IR3_INSTR_B) {
			if (instr->flags & IR3_INSTR_A1EN) {
				if (instr->flags & IR3_INSTR_NONUNIF) {
					return CAT5_BINDLESS_A1_NONUNIFORM;
				} else {
					return CAT5_BINDLESS_A1_UNIFORM;
				}
			} else if (instr->flags & IR3_INSTR_NONUNIF) {
				return CAT5_BINDLESS_NONUNIFORM;
			} else {
				return CAT5_BINDLESS_UNIFORM;
			}
		} else {
			if (instr->flags & IR3_INSTR_NONUNIF)
				return CAT5_NONUNIFORM;
			else
				return CAT5_UNIFORM;
		}
		assert(!(instr->cat5.samp | instr->cat5.tex));
	} else if (instr->flags & IR3_INSTR_B) {
		if (instr->flags & IR3_INSTR_A1EN) {
			return CAT5_BINDLESS_A1_IMM;
		} else {
			return CAT5_BINDLESS_IMM;
		}
	}
	return 0;
}

static inline unsigned
extract_cat6_DESC_MODE(struct ir3_instruction *instr)
{
	struct ir3_register *ssbo = instr->srcs[0];
	if (ssbo->flags & IR3_REG_IMMED) {
		return 0; // todo enum
	} else if (instr->flags & IR3_INSTR_NONUNIF) {
		return 2; // todo enum
	} else {
		return 1; // todo enum
	}
}

/**
 * This is a bit messy, for legacy (pre-bindless) atomic instructions,
 * the .g (global) variety have SSBO as first src and everything else
 * shifted up by one.
 *
 * TODO revisit this once legacy 'packed struct' encoding is gone
 */
static inline struct ir3_register *
extract_cat6_SRC(struct ir3_instruction *instr, unsigned n)
{
	if (is_global_a3xx_atomic(instr->opc)) {
		n++;
	}
	assert(n < instr->srcs_count);
	return instr->srcs[n];
}

typedef enum {
	REG_MULITSRC_IMMED,
	REG_MULTISRC_IMMED_FLUT_FULL,
	REG_MULTISRC_IMMED_FLUT_HALF,
	REG_MULTISRC_GPR,
	REG_MULTISRC_CONST,
	REG_MULTISRC_RELATIVE_GPR,
	REG_MULTISRC_RELATIVE_CONST,
} reg_multisrc_t;

static inline reg_multisrc_t
__multisrc_case(struct encode_state *s, struct ir3_register *reg)
{
	if (reg->flags & IR3_REG_IMMED) {
		assert(opc_cat(s->instr->opc) == 2);
		if (ir3_cat2_int(s->instr->opc)) {
			return REG_MULITSRC_IMMED;
		} else if (reg->flags & IR3_REG_HALF) {
			return REG_MULTISRC_IMMED_FLUT_HALF;
		} else {
			return REG_MULTISRC_IMMED_FLUT_FULL;
		}
	} else if (reg->flags & IR3_REG_RELATIV) {
		if (reg->flags & IR3_REG_CONST) {
			return REG_MULTISRC_RELATIVE_CONST;
		} else {
			return REG_MULTISRC_RELATIVE_GPR;
		}
	} else if (reg->flags & IR3_REG_CONST) {
		return REG_MULTISRC_CONST;
	} else {
		return REG_MULTISRC_GPR;
	}
}

typedef enum {
	REG_CAT3_SRC_GPR,
	REG_CAT3_SRC_CONST_OR_IMMED,
	REG_CAT3_SRC_RELATIVE_GPR,
	REG_CAT3_SRC_RELATIVE_CONST,
} reg_cat3_src_t;

static inline reg_cat3_src_t
__cat3_src_case(struct encode_state *s, struct ir3_register *reg)
{
	if (reg->flags & IR3_REG_RELATIV) {
		if (reg->flags & IR3_REG_CONST) {
			return REG_CAT3_SRC_RELATIVE_CONST;
		} else {
			return REG_CAT3_SRC_RELATIVE_GPR;
		}
	} else if (reg->flags & (IR3_REG_CONST | IR3_REG_IMMED)) {
		return REG_CAT3_SRC_CONST_OR_IMMED;
	} else {
		return REG_CAT3_SRC_GPR;
	}
}

typedef enum {
   STC_DST_IMM,
   STC_DST_A1
} stc_dst_t;

static inline stc_dst_t
__stc_dst_case(struct encode_state *s, struct ir3_instruction *instr)
{
   return (instr->flags & IR3_INSTR_A1EN) ? STC_DST_A1 : STC_DST_IMM;
}

#include "encode.h"


void *
isa_assemble(struct ir3_shader_variant *v)
{
	BITSET_WORD *ptr, *instrs;
	const struct ir3_info *info = &v->info;
	struct ir3 *shader = v->ir;

	ptr = instrs = rzalloc_size(v, info->size);

	foreach_block (block, &shader->block_list) {
		foreach_instr (instr, &block->instr_list) {
			struct encode_state s = {
				.gen = shader->compiler->gen * 100,
				.compiler = shader->compiler,
				.instr = instr,
			};

			bitmask_t encoded;
			if (instr->opc == OPC_META_RAW) {
				encoded = uint64_t_to_bitmask(instr->raw.value);
			} else {
				encoded = encode__instruction(&s, NULL, instr);
			}
			store_instruction(instrs, encoded);
			instrs += BITMASK_WORDS;
		}
	}

	return ptr;
}
