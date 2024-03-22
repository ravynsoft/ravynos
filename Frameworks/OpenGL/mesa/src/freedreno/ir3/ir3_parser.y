/*
 * Copyright (c) 2013 Rob Clark <robclark@freedesktop.org>
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

%code requires {
#include "ir3/ir3_assembler.h"
#include "ir3/ir3_shader.h"

struct ir3 * ir3_parse(struct ir3_shader_variant *v,
		struct ir3_kernel_info *k, FILE *f);
}

%{
#define YYDEBUG 0

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "util/half_float.h"
#include "util/u_math.h"

#include "ir3/ir3.h"
#include "ir3/ir3_shader.h"
#include "ir3/instr-a3xx.h"

#include "ir3_parser.h"

#define swap(a, b) \
	do { __typeof(a) __tmp = (a); (a) = (b); (b) = __tmp; } while (0)

/* ir3 treats the abs/neg flags as separate flags for float vs integer,
 * but in the instruction encoding they are the same thing.  Tracking
 * them separately is only for the benefit of ir3 opt passes, and not
 * required here, so just use the float versions:
 */
#define IR3_REG_ABS     IR3_REG_FABS
#define IR3_REG_NEGATE  IR3_REG_FNEG

static struct ir3_kernel_info    *info;
static struct ir3_shader_variant *variant;
/* NOTE the assembler doesn't really use the ir3_block construction
 * like the compiler does.  Everything is treated as one large block.
 * Which might happen to contain flow control.  But since we don't
 * use any of the ir3 backend passes (sched, RA, etc) this doesn't
 * really matter.
 */
static struct ir3_block          *block;   /* current shader block */
static struct ir3_instruction    *instr;   /* current instruction */
static unsigned ip; /* current instruction pointer */
static struct hash_table *labels;

void *ir3_parser_dead_ctx;

static struct {
	unsigned flags;
	unsigned repeat;
	unsigned nop;
} iflags;

static struct {
	unsigned flags;
	unsigned wrmask;
} rflags;

int ir3_yyget_lineno(void);

static void new_label(const char *name)
{
	ralloc_steal(labels, (void *) name);
	_mesa_hash_table_insert(labels, name, (void *)(uintptr_t)ip);
}

static struct ir3_instruction * new_instr(opc_t opc)
{
	instr = ir3_instr_create(block, opc, 4, 6);
	instr->flags = iflags.flags;
	instr->repeat = iflags.repeat;
	instr->nop = iflags.nop;
	instr->line = ir3_yyget_lineno();
	iflags.flags = iflags.repeat = iflags.nop = 0;
	ip++;
	return instr;
}

static void new_shader(void)
{
	variant->ir = ir3_create(variant->compiler, variant);
	block = ir3_block_create(variant->ir);
	list_addtail(&block->node, &variant->ir->block_list);
	ip = 0;
	labels = _mesa_hash_table_create(variant, _mesa_hash_string, _mesa_key_string_equal);
	ir3_parser_dead_ctx = ralloc_context(NULL);
}

static type_t parse_type(const char **type)
{
	if (!strncmp("f16", *type, 3)) {
		*type += 3;
		return TYPE_F16;
	} else if (!strncmp("f32", *type, 3)) {
		*type += 3;
		return TYPE_F32;
	} else if (!strncmp("u16", *type, 3)) {
		*type += 3;
		return TYPE_U16;
	} else if (!strncmp("u32", *type, 3)) {
		*type += 3;
		return TYPE_U32;
	} else if (!strncmp("s16", *type, 3)) {
		*type += 3;
		return TYPE_S16;
	} else if (!strncmp("s32", *type, 3)) {
		*type += 3;
		return TYPE_S32;
	} else if (!strncmp("u8", *type, 2)) {
		*type += 2;
		return TYPE_U8;
	} else if (!strncmp("s8", *type, 2)) {
		*type += 2;
		return TYPE_S8;
	} else {
		assert(0);  /* shouldn't get here */
		return ~0;
	}
}

static struct ir3_instruction * parse_type_type(struct ir3_instruction *instr,
		const char *type_type)
{
	instr->cat1.src_type = parse_type(&type_type);
	instr->cat1.dst_type = parse_type(&type_type);
	return instr;
}

static struct ir3_register * new_src(int num, unsigned flags)
{
	struct ir3_register *reg;
	flags |= rflags.flags;
	if (num & 0x1)
		flags |= IR3_REG_HALF;
	reg = ir3_src_create(instr, num>>1, flags);
	reg->wrmask = MAX2(1, rflags.wrmask);
	rflags.flags = rflags.wrmask = 0;
	return reg;
}

static struct ir3_register * new_dst(int num, unsigned flags)
{
	struct ir3_register *reg;
	flags |= rflags.flags;
	if (num & 0x1)
		flags |= IR3_REG_HALF;
	reg = ir3_dst_create(instr, num>>1, flags);
	reg->wrmask = MAX2(1, rflags.wrmask);
	rflags.flags = rflags.wrmask = 0;
	return reg;
}

static struct ir3_register * dummy_dst(void)
{
	return new_dst(0, 0);
}

static void fixup_cat5_s2en(void)
{
	assert(opc_cat(instr->opc) == 5);
	if (!(instr->flags & IR3_INSTR_S2EN))
		return;
	/* For various reasons (ie. mainly to make the .s2en src easier to
	 * find, given that various different cat5 tex instructions can have
	 * different # of src registers), in ir3 the samp/tex src register
	 * is first, rather than last.  So we have to detect this case and
	 * fix things up.
	 */
	struct ir3_register *s2en_src = instr->srcs[instr->srcs_count - 1];

	if (instr->flags & IR3_INSTR_B)
		assert(!(s2en_src->flags & IR3_REG_HALF));
	else
		assert(s2en_src->flags & IR3_REG_HALF);

	for (int i = 0; i < instr->srcs_count - 1; i++) {
		instr->srcs[i+1] = instr->srcs[i];
	}
	instr->srcs[0] = s2en_src;
}

static void add_const(unsigned reg, unsigned c0, unsigned c1, unsigned c2, unsigned c3)
{
	struct ir3_const_state *const_state = ir3_const_state(variant);
	assert((reg & 0x7) == 0);
	int idx = reg >> (1 + 2); /* low bit is half vs full, next two bits are swiz */
	if (idx * 4 + 4 > const_state->immediates_size) {
		const_state->immediates = rerzalloc(const_state,
				const_state->immediates,
				__typeof__(const_state->immediates[0]),
				const_state->immediates_size,
				idx * 4 + 4);
		for (unsigned i = const_state->immediates_size; i < idx * 4; i++)
			const_state->immediates[i] = 0xd0d0d0d0;
		const_state->immediates_size = const_state->immediates_count = idx * 4 + 4;
	}
	const_state->immediates[idx * 4 + 0] = c0;
	const_state->immediates[idx * 4 + 1] = c1;
	const_state->immediates[idx * 4 + 2] = c2;
	const_state->immediates[idx * 4 + 3] = c3;
}

static void add_sysval(unsigned reg, unsigned compmask, gl_system_value sysval)
{
	unsigned n = variant->inputs_count++;
	variant->inputs[n].regid = reg;
	variant->inputs[n].sysval = true;
	variant->inputs[n].slot = sysval;
	variant->inputs[n].compmask = compmask;
	variant->total_in++;
}

static bool resolve_labels(void)
{
	int instr_ip = 0;
	foreach_instr (instr, &block->instr_list) {
		if (opc_cat(instr->opc) == 0 && instr->cat0.target_label) {
			struct hash_entry *entry = _mesa_hash_table_search(labels, instr->cat0.target_label);
			if (!entry) {
				fprintf(stderr, "unknown label %s\n", instr->cat0.target_label);
				return false;
			}
			int target_ip = (uintptr_t)entry->data;
			instr->cat0.immed = target_ip - instr_ip;
		}
		instr_ip++;
	}
	return true;
}

#ifdef YYDEBUG
int yydebug;
#endif

extern int yylex(void);
void ir3_yyset_lineno(int _line_number);
void ir3_yyset_input(FILE *f);

int yyparse(void);

static void yyerror(const char *error)
{
	fprintf(stderr, "error at line %d: %s\n", ir3_yyget_lineno(), error);
}

struct ir3 * ir3_parse(struct ir3_shader_variant *v,
		struct ir3_kernel_info *k, FILE *f)
{
	ir3_yyset_lineno(1);
	ir3_yyset_input(f);
#ifdef YYDEBUG
	yydebug = 1;
#endif
	info = k;
	variant = v;
	if (yyparse() || !resolve_labels()) {
		ir3_destroy(variant->ir);
		variant->ir = NULL;
	}
	ralloc_free(labels);
	ralloc_free(ir3_parser_dead_ctx);
	return variant->ir;
}
%}

%union {
	int tok;
	int num;
	uint32_t unum;
	uint64_t u64;
	double flt;
	const char *str;
	struct ir3_register *reg;
	struct {
		int start;
		int num;
	} range;
	type_t type;
}

%{
#if YYDEBUG
static void print_token(FILE *file, int type, YYSTYPE value)
{
	fprintf(file, "\ntype: %d\n", type);
}

#define YYPRINT(file, type, value) print_token(file, type, value)
#endif
%}

%token <num> T_INT
%token <unum> T_HEX
%token <flt> T_FLOAT
%token <str> T_IDENTIFIER
%token <num> T_REGISTER
%token <num> T_CONSTANT

/* @ headers (@const/@sampler/@uniform/@varying) */
%token <tok> T_A_LOCALSIZE
%token <tok> T_A_CONST
%token <tok> T_A_BUF
%token <tok> T_A_INVOCATIONID
%token <tok> T_A_WGID
%token <tok> T_A_NUMWG
%token <tok> T_A_BRANCHSTACK
%token <tok> T_A_IN
%token <tok> T_A_OUT
%token <tok> T_A_TEX
%token <tok> T_A_PVTMEM
%token <tok> T_A_EARLYPREAMBLE
/* todo, re-add @sampler/@uniform/@varying if needed someday */

/* src register flags */
%token <tok> T_ABSNEG
%token <tok> T_NEG
%token <tok> T_ABS
%token <tok> T_R
%token <tok> T_LAST

%token <tok> T_HR
%token <tok> T_HC

/* dst register flags */
%token <tok> T_EVEN
%token <tok> T_POS_INFINITY
%token <tok> T_NEG_INFINITY
%token <tok> T_EI
%token <num> T_WRMASK

/* Float LUT values accepted as immed: */
%token <num> T_FLUT_0_0
%token <num> T_FLUT_0_5
%token <num> T_FLUT_1_0
%token <num> T_FLUT_2_0
%token <num> T_FLUT_E
%token <num> T_FLUT_PI
%token <num> T_FLUT_INV_PI
%token <num> T_FLUT_INV_LOG2_E
%token <num> T_FLUT_LOG2_E
%token <num> T_FLUT_INV_LOG2_10
%token <num> T_FLUT_LOG2_10
%token <num> T_FLUT_4_0

/* instruction flags */
%token <tok> T_SY
%token <tok> T_SS
%token <tok> T_JP
%token <tok> T_EQ_FLAG
%token <tok> T_SAT
%token <num> T_RPT
%token <tok> T_UL
%token <tok> T_NOP

/* category 0: */
%token <tok> T_OP_NOP
%token <tok> T_OP_BR
%token <tok> T_OP_BRAO
%token <tok> T_OP_BRAA
%token <tok> T_OP_BRAC
%token <tok> T_OP_BANY
%token <tok> T_OP_BALL
%token <tok> T_OP_BRAX
%token <tok> T_OP_JUMP
%token <tok> T_OP_CALL
%token <tok> T_OP_RET
%token <tok> T_OP_KILL
%token <tok> T_OP_END
%token <tok> T_OP_EMIT
%token <tok> T_OP_CUT
%token <tok> T_OP_CHMASK
%token <tok> T_OP_CHSH
%token <tok> T_OP_FLOW_REV
%token <tok> T_OP_BKT
%token <tok> T_OP_STKS
%token <tok> T_OP_STKR
%token <tok> T_OP_XSET
%token <tok> T_OP_XCLR
%token <tok> T_OP_GETLAST
%token <tok> T_OP_GETONE
%token <tok> T_OP_DBG
%token <tok> T_OP_SHPS
%token <tok> T_OP_SHPE
%token <tok> T_OP_PREDT
%token <tok> T_OP_PREDF
%token <tok> T_OP_PREDE

/* category 1: */
%token <tok> T_OP_MOVMSK
%token <tok> T_OP_MOVA1
%token <tok> T_OP_MOVA
%token <tok> T_OP_MOV
%token <tok> T_OP_COV
%token <tok> T_OP_SWZ
%token <tok> T_OP_GAT
%token <tok> T_OP_SCT

/* category 2: */
%token <tok> T_OP_ADD_F
%token <tok> T_OP_MIN_F
%token <tok> T_OP_MAX_F
%token <tok> T_OP_MUL_F
%token <tok> T_OP_SIGN_F
%token <tok> T_OP_CMPS_F
%token <tok> T_OP_ABSNEG_F
%token <tok> T_OP_CMPV_F
%token <tok> T_OP_FLOOR_F
%token <tok> T_OP_CEIL_F
%token <tok> T_OP_RNDNE_F
%token <tok> T_OP_RNDAZ_F
%token <tok> T_OP_TRUNC_F
%token <tok> T_OP_ADD_U
%token <tok> T_OP_ADD_S
%token <tok> T_OP_SUB_U
%token <tok> T_OP_SUB_S
%token <tok> T_OP_CMPS_U
%token <tok> T_OP_CMPS_S
%token <tok> T_OP_MIN_U
%token <tok> T_OP_MIN_S
%token <tok> T_OP_MAX_U
%token <tok> T_OP_MAX_S
%token <tok> T_OP_ABSNEG_S
%token <tok> T_OP_AND_B
%token <tok> T_OP_OR_B
%token <tok> T_OP_NOT_B
%token <tok> T_OP_XOR_B
%token <tok> T_OP_CMPV_U
%token <tok> T_OP_CMPV_S
%token <tok> T_OP_MUL_U24
%token <tok> T_OP_MUL_S24
%token <tok> T_OP_MULL_U
%token <tok> T_OP_BFREV_B
%token <tok> T_OP_CLZ_S
%token <tok> T_OP_CLZ_B
%token <tok> T_OP_SHL_B
%token <tok> T_OP_SHR_B
%token <tok> T_OP_ASHR_B
%token <tok> T_OP_BARY_F
%token <tok> T_OP_FLAT_B
%token <tok> T_OP_MGEN_B
%token <tok> T_OP_GETBIT_B
%token <tok> T_OP_SETRM
%token <tok> T_OP_CBITS_B
%token <tok> T_OP_SHB
%token <tok> T_OP_MSAD

/* category 3: */
%token <tok> T_OP_MAD_U16
%token <tok> T_OP_MADSH_U16
%token <tok> T_OP_MAD_S16
%token <tok> T_OP_MADSH_M16
%token <tok> T_OP_MAD_U24
%token <tok> T_OP_MAD_S24
%token <tok> T_OP_MAD_F16
%token <tok> T_OP_MAD_F32
%token <tok> T_OP_SEL_B16
%token <tok> T_OP_SEL_B32
%token <tok> T_OP_SEL_S16
%token <tok> T_OP_SEL_S32
%token <tok> T_OP_SEL_F16
%token <tok> T_OP_SEL_F32
%token <tok> T_OP_SAD_S16
%token <tok> T_OP_SAD_S32
%token <tok> T_OP_SHRM
%token <tok> T_OP_SHLM
%token <tok> T_OP_SHRG
%token <tok> T_OP_SHLG
%token <tok> T_OP_ANDG
%token <tok> T_OP_DP2ACC
%token <tok> T_OP_DP4ACC
%token <tok> T_OP_WMM
%token <tok> T_OP_WMM_ACCU

/* category 4: */
%token <tok> T_OP_RCP
%token <tok> T_OP_RSQ
%token <tok> T_OP_LOG2
%token <tok> T_OP_EXP2
%token <tok> T_OP_SIN
%token <tok> T_OP_COS
%token <tok> T_OP_SQRT
%token <tok> T_OP_HRSQ
%token <tok> T_OP_HLOG2
%token <tok> T_OP_HEXP2

/* category 5: */
%token <tok> T_OP_ISAM
%token <tok> T_OP_ISAML
%token <tok> T_OP_ISAMM
%token <tok> T_OP_SAM
%token <tok> T_OP_SAMB
%token <tok> T_OP_SAML
%token <tok> T_OP_SAMGQ
%token <tok> T_OP_GETLOD
%token <tok> T_OP_CONV
%token <tok> T_OP_CONVM
%token <tok> T_OP_GETSIZE
%token <tok> T_OP_GETBUF
%token <tok> T_OP_GETPOS
%token <tok> T_OP_GETINFO
%token <tok> T_OP_DSX
%token <tok> T_OP_DSY
%token <tok> T_OP_GATHER4R
%token <tok> T_OP_GATHER4G
%token <tok> T_OP_GATHER4B
%token <tok> T_OP_GATHER4A
%token <tok> T_OP_SAMGP0
%token <tok> T_OP_SAMGP1
%token <tok> T_OP_SAMGP2
%token <tok> T_OP_SAMGP3
%token <tok> T_OP_DSXPP_1
%token <tok> T_OP_DSYPP_1
%token <tok> T_OP_RGETPOS
%token <tok> T_OP_RGETINFO
%token <tok> T_OP_BRCST_A
%token <tok> T_OP_QSHUFFLE_BRCST
%token <tok> T_OP_QSHUFFLE_H
%token <tok> T_OP_QSHUFFLE_V
%token <tok> T_OP_QSHUFFLE_DIAG
%token <tok> T_OP_TCINV

/* category 6: */
%token <tok> T_OP_LDG
%token <tok> T_OP_LDG_A
%token <tok> T_OP_LDL
%token <tok> T_OP_LDP
%token <tok> T_OP_STG
%token <tok> T_OP_STG_A
%token <tok> T_OP_STL
%token <tok> T_OP_STP
%token <tok> T_OP_LDIB
%token <tok> T_OP_G2L
%token <tok> T_OP_L2G
%token <tok> T_OP_PREFETCH
%token <tok> T_OP_LDLW
%token <tok> T_OP_STLW
%token <tok> T_OP_RESFMT
%token <tok> T_OP_RESINFO
%token <tok> T_OP_ATOMIC_ADD
%token <tok> T_OP_ATOMIC_SUB
%token <tok> T_OP_ATOMIC_XCHG
%token <tok> T_OP_ATOMIC_INC
%token <tok> T_OP_ATOMIC_DEC
%token <tok> T_OP_ATOMIC_CMPXCHG
%token <tok> T_OP_ATOMIC_MIN
%token <tok> T_OP_ATOMIC_MAX
%token <tok> T_OP_ATOMIC_AND
%token <tok> T_OP_ATOMIC_OR
%token <tok> T_OP_ATOMIC_XOR
%token <tok> T_OP_RESINFO_B
%token <tok> T_OP_LDIB_B
%token <tok> T_OP_STIB_B
%token <tok> T_OP_ATOMIC_B_ADD
%token <tok> T_OP_ATOMIC_B_SUB
%token <tok> T_OP_ATOMIC_B_XCHG
%token <tok> T_OP_ATOMIC_B_INC
%token <tok> T_OP_ATOMIC_B_DEC
%token <tok> T_OP_ATOMIC_B_CMPXCHG
%token <tok> T_OP_ATOMIC_B_MIN
%token <tok> T_OP_ATOMIC_B_MAX
%token <tok> T_OP_ATOMIC_B_AND
%token <tok> T_OP_ATOMIC_B_OR
%token <tok> T_OP_ATOMIC_B_XOR
%token <tok> T_OP_ATOMIC_S_ADD
%token <tok> T_OP_ATOMIC_S_SUB
%token <tok> T_OP_ATOMIC_S_XCHG
%token <tok> T_OP_ATOMIC_S_INC
%token <tok> T_OP_ATOMIC_S_DEC
%token <tok> T_OP_ATOMIC_S_CMPXCHG
%token <tok> T_OP_ATOMIC_S_MIN
%token <tok> T_OP_ATOMIC_S_MAX
%token <tok> T_OP_ATOMIC_S_AND
%token <tok> T_OP_ATOMIC_S_OR
%token <tok> T_OP_ATOMIC_S_XOR
%token <tok> T_OP_ATOMIC_G_ADD
%token <tok> T_OP_ATOMIC_G_SUB
%token <tok> T_OP_ATOMIC_G_XCHG
%token <tok> T_OP_ATOMIC_G_INC
%token <tok> T_OP_ATOMIC_G_DEC
%token <tok> T_OP_ATOMIC_G_CMPXCHG
%token <tok> T_OP_ATOMIC_G_MIN
%token <tok> T_OP_ATOMIC_G_MAX
%token <tok> T_OP_ATOMIC_G_AND
%token <tok> T_OP_ATOMIC_G_OR
%token <tok> T_OP_ATOMIC_G_XOR
%token <tok> T_OP_LDGB
%token <tok> T_OP_STGB
%token <tok> T_OP_STIB
%token <tok> T_OP_LDC
%token <tok> T_OP_LDLV
%token <tok> T_OP_GETSPID
%token <tok> T_OP_GETWID
%token <tok> T_OP_GETFIBERID
%token <tok> T_OP_STC
%token <tok> T_OP_STSC

/* category 7: */
%token <tok> T_OP_BAR
%token <tok> T_OP_FENCE
%token <tok> T_OP_SLEEP
%token <tok> T_OP_ICINV
%token <tok> T_OP_DCCLN
%token <tok> T_OP_DCINV
%token <tok> T_OP_DCFLU
%token <tok> T_OP_CCINV
%token <tok> T_OP_LOCK
%token <tok> T_OP_UNLOCK
%token <tok> T_OP_ALIAS

%token <u64> T_RAW

%token <tok> T_OP_PRINT

/* type qualifiers: */
%token <tok> T_TYPE_F16
%token <tok> T_TYPE_F32
%token <tok> T_TYPE_U16
%token <tok> T_TYPE_U32
%token <tok> T_TYPE_S16
%token <tok> T_TYPE_S32
%token <tok> T_TYPE_U8
%token <tok> T_TYPE_S8

%token <tok> T_UNTYPED
%token <tok> T_TYPED

%token <tok> T_MIXED
%token <tok> T_UNSIGNED
%token <tok> T_LOW
%token <tok> T_HIGH

%token <tok> T_1D
%token <tok> T_2D
%token <tok> T_3D
%token <tok> T_4D

/* condition qualifiers: */
%token <tok> T_LT
%token <tok> T_LE
%token <tok> T_GT
%token <tok> T_GE
%token <tok> T_EQ
%token <tok> T_NE

%token <tok> T_S2EN
%token <tok> T_SAMP
%token <tok> T_TEX
%token <tok> T_BASE
%token <tok> T_OFFSET
%token <tok> T_UNIFORM
%token <tok> T_NONUNIFORM
%token <tok> T_IMM

%token <tok> T_NAN
%token <tok> T_INF
%token <num> T_A0
%token <num> T_A1
%token <num> T_P0
%token <num> T_W
%token <str> T_CAT1_TYPE_TYPE
%token <str> T_INSTR_TYPE

%token <tok> T_MOD_TEX
%token <tok> T_MOD_MEM
%token <tok> T_MOD_RT

%type <num> integer offset
%type <num> flut_immed
%type <flt> float
%type <reg> src dst const
%type <tok> cat1_opc
%type <tok> cat2_opc_1src cat2_opc_2src_cnd cat2_opc_2src
%type <tok> cat3_opc
%type <tok> cat4_opc
%type <tok> cat5_opc cat5_samp cat5_tex cat5_type
%type <type> type
%type <unum> const_val

%error-verbose

%start shader

%%

shader:            { new_shader(); } headers instrs

headers:           
|                  header headers

header:            localsize_header
|                  const_header
|                  buf_header
|                  invocationid_header
|                  wgid_header
|                  numwg_header
|                  branchstack_header
|                  in_header
|                  out_header
|                  tex_header
|                  pvtmem_header
|                  earlypreamble_header

const_val:         T_FLOAT   { $$ = fui($1); }
|                  T_INT     { $$ = $1;      }
|                  '-' T_INT { $$ = -$2;     }
|                  T_HEX     { $$ = $1;      }

localsize_header:  T_A_LOCALSIZE const_val ',' const_val ',' const_val {
                       variant->local_size[0] = $2;
                       variant->local_size[1] = $4;
                       variant->local_size[2] = $6;
}

const_header:      T_A_CONST '(' T_CONSTANT ')' const_val ',' const_val ',' const_val ',' const_val {
                       add_const($3, $5, $7, $9, $11);
}

buf_header_addr_reg:
                   '(' T_CONSTANT ')' {
                       assert(($2 & 0x1) == 0);  /* half-reg not allowed */
                       unsigned reg = $2 >> 1;

                       info->buf_addr_regs[info->num_bufs - 1] = reg;
                       /* reserve space in immediates for the actual value to be plugged in later: */
                       add_const($2, 0, 0, 0, 0);
}
|

buf_header:        T_A_BUF const_val {
                       int idx = info->num_bufs++;
                       assert(idx < MAX_BUFS);
                       info->buf_sizes[idx] = $2;
} buf_header_addr_reg

invocationid_header: T_A_INVOCATIONID '(' T_REGISTER ')' {
                       assert(($3 & 0x1) == 0);  /* half-reg not allowed */
                       unsigned reg = $3 >> 1;
                       add_sysval(reg, 0x7, SYSTEM_VALUE_LOCAL_INVOCATION_ID);
}

wgid_header:       T_A_WGID '(' T_REGISTER ')' {
                       assert(($3 & 0x1) == 0);  /* half-reg not allowed */
                       unsigned reg = $3 >> 1;
                       assert(variant->compiler->gen >= 5);
                       assert(reg >= regid(48, 0)); /* must be a high reg */
                       add_sysval(reg, 0x7, SYSTEM_VALUE_WORKGROUP_ID);
}
|                  T_A_WGID '(' T_CONSTANT ')' {
                       assert(($3 & 0x1) == 0);  /* half-reg not allowed */
                       unsigned reg = $3 >> 1;
                       assert(variant->compiler->gen < 5);
                       info->wgid = reg;
}

numwg_header:      T_A_NUMWG '(' T_CONSTANT ')' {
                       assert(($3 & 0x1) == 0);  /* half-reg not allowed */
                       unsigned reg = $3 >> 1;
                       info->numwg = reg;
                       /* reserve space in immediates for the actual value to be plugged in later: */
                       if (variant->compiler->gen >= 5)
                          add_const($3, 0, 0, 0, 0);
}

branchstack_header: T_A_BRANCHSTACK const_val { variant->branchstack = $2; }

pvtmem_header: T_A_PVTMEM const_val { variant->pvtmem_size = $2; }

earlypreamble_header: T_A_EARLYPREAMBLE { info->early_preamble = 1; }

/* Stubs for now */
in_header:         T_A_IN '(' T_REGISTER ')' T_IDENTIFIER '(' T_IDENTIFIER '=' integer ')' { }

out_header:        T_A_OUT '(' T_REGISTER ')' T_IDENTIFIER '(' T_IDENTIFIER '=' integer ')' { }

tex_header:        T_A_TEX '(' T_REGISTER ')'
                       T_IDENTIFIER '=' integer ',' /* src */
                       T_IDENTIFIER '=' integer ',' /* samp */
                       T_IDENTIFIER '=' integer ',' /* tex */
                       T_IDENTIFIER '=' integer ',' /* wrmask */
                       T_IDENTIFIER '=' integer     /* cmd */ { }

iflag:             T_SY   { iflags.flags |= IR3_INSTR_SY; }
|                  T_SS   { iflags.flags |= IR3_INSTR_SS; }
|                  T_JP   { iflags.flags |= IR3_INSTR_JP; }
|                  T_EQ_FLAG { iflags.flags |= IR3_INSTR_EQ; }
|                  T_SAT  { iflags.flags |= IR3_INSTR_SAT; }
|                  T_RPT  { iflags.repeat = $1; }
|                  T_UL   { iflags.flags |= IR3_INSTR_UL; }
|                  T_NOP  { iflags.nop = $1; }

iflags:
|                  iflag iflags

instrs:            instrs instr
|                  instr

instr:             iflags cat0_instr
|                  iflags cat1_instr
|                  iflags cat2_instr
|                  iflags cat3_instr
|                  iflags cat4_instr
|                  iflags cat5_instr { fixup_cat5_s2en(); }
|                  iflags cat6_instr
|                  iflags cat7_instr
|                  raw_instr
|                  meta_print
|                  label

label:             T_IDENTIFIER ':' { new_label($1); }

cat0_src1:         '!' T_P0        { instr->cat0.inv1 = true; instr->cat0.comp1 = $2 >> 1; }
|                  T_P0            { instr->cat0.comp1 = $1 >> 1; }

cat0_src2:         '!' T_P0        { instr->cat0.inv2 = true; instr->cat0.comp2 = $2 >> 1; }
|                  T_P0            { instr->cat0.comp2 = $1 >> 1; }

cat0_immed:        '#' integer     { instr->cat0.immed = $2; }
|                  '#' T_IDENTIFIER { ralloc_steal(instr, (void *)$2); instr->cat0.target_label = $2; }

cat0_instr:        T_OP_NOP        { new_instr(OPC_NOP); }
|                  T_OP_BR         { new_instr(OPC_B)->cat0.brtype = BRANCH_PLAIN; } cat0_src1 ',' cat0_immed
|                  T_OP_BRAO       { new_instr(OPC_B)->cat0.brtype = BRANCH_OR;    } cat0_src1 ',' cat0_src2 ',' cat0_immed
|                  T_OP_BRAA       { new_instr(OPC_B)->cat0.brtype = BRANCH_AND;    } cat0_src1 ',' cat0_src2 ',' cat0_immed
|                  T_OP_BRAC '.' integer { new_instr(OPC_B)->cat0.brtype = BRANCH_CONST; instr->cat0.idx = $3; } cat0_immed
|                  T_OP_BANY       { new_instr(OPC_B)->cat0.brtype = BRANCH_ANY; } cat0_src1 ',' cat0_immed
|                  T_OP_BALL       { new_instr(OPC_B)->cat0.brtype = BRANCH_ALL; } cat0_src1 ',' cat0_immed
|                  T_OP_BRAX       { new_instr(OPC_B)->cat0.brtype = BRANCH_X; } cat0_immed
|                  T_OP_JUMP       { new_instr(OPC_JUMP); }  cat0_immed
|                  T_OP_CALL       { new_instr(OPC_CALL); }  cat0_immed
|                  T_OP_RET        { new_instr(OPC_RET); }
|                  T_OP_KILL       { new_instr(OPC_KILL); }  cat0_src1
|                  T_OP_END        { new_instr(OPC_END); }
|                  T_OP_EMIT       { new_instr(OPC_EMIT); }
|                  T_OP_CUT        { new_instr(OPC_CUT); }
|                  T_OP_CHMASK     { new_instr(OPC_CHMASK); }
|                  T_OP_CHSH       { new_instr(OPC_CHSH); }
|                  T_OP_FLOW_REV   { new_instr(OPC_FLOW_REV); }
|                  T_OP_BKT        { new_instr(OPC_BKT); }      cat0_immed
|                  T_OP_STKS       { new_instr(OPC_STKS); }
|                  T_OP_STKR       { new_instr(OPC_STKR); }
|                  T_OP_XSET       { new_instr(OPC_XSET); }
|                  T_OP_XCLR       { new_instr(OPC_XCLR); }
|                  T_OP_GETONE     { new_instr(OPC_GETONE); }   cat0_immed
|                  T_OP_DBG        { new_instr(OPC_DBG); }
|                  T_OP_SHPS       { new_instr(OPC_SHPS); }     cat0_immed
|                  T_OP_SHPE       { new_instr(OPC_SHPE); }
|                  T_OP_PREDT      { new_instr(OPC_PREDT); }    cat0_src1
|                  T_OP_PREDF      { new_instr(OPC_PREDF); }    cat0_src1
|                  T_OP_PREDE      { new_instr(OPC_PREDE); }
|                  T_OP_GETLAST '.' T_W { new_instr(OPC_GETLAST); }   cat0_immed

cat1_opc:          T_OP_MOV '.' T_CAT1_TYPE_TYPE {
                       parse_type_type(new_instr(OPC_MOV), $3);
}
|                  T_OP_COV '.' T_CAT1_TYPE_TYPE {
                       parse_type_type(new_instr(OPC_MOV), $3);
}

cat1_src:          src_reg_or_const_or_rel
|                  immediate_cat1

cat1_movmsk:       T_OP_MOVMSK '.' T_W {
                       new_instr(OPC_MOVMSK);
                       instr->cat1.src_type = TYPE_U32;
                       instr->cat1.dst_type = TYPE_U32;
                   } dst_reg {
                       if (($3 % 32) != 0)
                          yyerror("w# must be multiple of 32");
                       if ($3 < 32)
                          yyerror("w# must be at least 32");

                       int num = $3 / 32;

                       instr->repeat = num - 1;
                       instr->dsts[0]->wrmask = (1 << num) - 1;
                   }

cat1_mova1:        T_OP_MOVA1 T_A1 ',' {
                       new_instr(OPC_MOV);
                       instr->cat1.src_type = TYPE_U16;
                       instr->cat1.dst_type = TYPE_U16;
                       new_dst((61 << 3) + 2, IR3_REG_HALF);
                   } cat1_src

cat1_mova:         T_OP_MOVA T_A0 ',' {
                       new_instr(OPC_MOV);
                       instr->cat1.src_type = TYPE_S16;
                       instr->cat1.dst_type = TYPE_S16;
                       new_dst((61 << 3), IR3_REG_HALF);
                   } cat1_src

cat1_swz:          T_OP_SWZ '.' T_CAT1_TYPE_TYPE { parse_type_type(new_instr(OPC_SWZ), $3); } dst_reg ',' dst_reg ',' src_reg ',' src_reg

cat1_gat:          T_OP_GAT '.' T_CAT1_TYPE_TYPE { parse_type_type(new_instr(OPC_GAT), $3); } dst_reg ',' src_reg ',' src_reg ',' src_reg ',' src_reg

cat1_sct:          T_OP_SCT '.' T_CAT1_TYPE_TYPE { parse_type_type(new_instr(OPC_SCT), $3); } dst_reg ',' dst_reg ',' dst_reg ',' dst_reg ',' src_reg

                   /* NOTE: cat1 can also *write* to relative gpr */
cat1_instr:        cat1_movmsk
|                  cat1_mova1
|                  cat1_mova
|                  cat1_swz
|                  cat1_gat
|                  cat1_sct
|                  cat1_opc dst_reg ',' cat1_src
|                  cat1_opc relative_gpr_dst ',' cat1_src

cat2_opc_1src:     T_OP_ABSNEG_F  { new_instr(OPC_ABSNEG_F); }
|                  T_OP_ABSNEG_S  { new_instr(OPC_ABSNEG_S); }
|                  T_OP_CLZ_B     { new_instr(OPC_CLZ_B); }
|                  T_OP_CLZ_S     { new_instr(OPC_CLZ_S); }
|                  T_OP_SIGN_F    { new_instr(OPC_SIGN_F); }
|                  T_OP_FLOOR_F   { new_instr(OPC_FLOOR_F); }
|                  T_OP_CEIL_F    { new_instr(OPC_CEIL_F); }
|                  T_OP_RNDNE_F   { new_instr(OPC_RNDNE_F); }
|                  T_OP_RNDAZ_F   { new_instr(OPC_RNDAZ_F); }
|                  T_OP_TRUNC_F   { new_instr(OPC_TRUNC_F); }
|                  T_OP_NOT_B     { new_instr(OPC_NOT_B); }
|                  T_OP_BFREV_B   { new_instr(OPC_BFREV_B); }
|                  T_OP_SETRM     { new_instr(OPC_SETRM); }
|                  T_OP_CBITS_B   { new_instr(OPC_CBITS_B); }

cat2_opc_2src_cnd: T_OP_CMPS_F    { new_instr(OPC_CMPS_F); }
|                  T_OP_CMPS_U    { new_instr(OPC_CMPS_U); }
|                  T_OP_CMPS_S    { new_instr(OPC_CMPS_S); }
|                  T_OP_CMPV_F    { new_instr(OPC_CMPV_F); }
|                  T_OP_CMPV_U    { new_instr(OPC_CMPV_U); }
|                  T_OP_CMPV_S    { new_instr(OPC_CMPV_S); }

cat2_opc_2src:     T_OP_ADD_F     { new_instr(OPC_ADD_F); }
|                  T_OP_MIN_F     { new_instr(OPC_MIN_F); }
|                  T_OP_MAX_F     { new_instr(OPC_MAX_F); }
|                  T_OP_MUL_F     { new_instr(OPC_MUL_F); }
|                  T_OP_ADD_U     { new_instr(OPC_ADD_U); }
|                  T_OP_ADD_S     { new_instr(OPC_ADD_S); }
|                  T_OP_SUB_U     { new_instr(OPC_SUB_U); }
|                  T_OP_SUB_S     { new_instr(OPC_SUB_S); }
|                  T_OP_MIN_U     { new_instr(OPC_MIN_U); }
|                  T_OP_MIN_S     { new_instr(OPC_MIN_S); }
|                  T_OP_MAX_U     { new_instr(OPC_MAX_U); }
|                  T_OP_MAX_S     { new_instr(OPC_MAX_S); }
|                  T_OP_AND_B     { new_instr(OPC_AND_B); }
|                  T_OP_OR_B      { new_instr(OPC_OR_B); }
|                  T_OP_XOR_B     { new_instr(OPC_XOR_B); }
|                  T_OP_MUL_U24   { new_instr(OPC_MUL_U24); }
|                  T_OP_MUL_S24   { new_instr(OPC_MUL_S24); }
|                  T_OP_MULL_U    { new_instr(OPC_MULL_U); }
|                  T_OP_SHL_B     { new_instr(OPC_SHL_B); }
|                  T_OP_SHR_B     { new_instr(OPC_SHR_B); }
|                  T_OP_ASHR_B    { new_instr(OPC_ASHR_B); }
|                  T_OP_BARY_F    { new_instr(OPC_BARY_F); }
|                  T_OP_FLAT_B    { new_instr(OPC_FLAT_B); }
|                  T_OP_MGEN_B    { new_instr(OPC_MGEN_B); }
|                  T_OP_GETBIT_B  { new_instr(OPC_GETBIT_B); }
|                  T_OP_SHB       { new_instr(OPC_SHB); }
|                  T_OP_MSAD      { new_instr(OPC_MSAD); }

cond:              T_LT           { instr->cat2.condition = IR3_COND_LT; }
|                  T_LE           { instr->cat2.condition = IR3_COND_LE; }
|                  T_GT           { instr->cat2.condition = IR3_COND_GT; }
|                  T_GE           { instr->cat2.condition = IR3_COND_GE; }
|                  T_EQ           { instr->cat2.condition = IR3_COND_EQ; }
|                  T_NE           { instr->cat2.condition = IR3_COND_NE; }

cat2_instr:        cat2_opc_1src dst_reg ',' src_reg_or_const_or_rel_or_imm
|                  cat2_opc_2src_cnd '.' cond dst_reg ',' src_reg_or_const_or_rel_or_imm ',' src_reg_or_const_or_rel_or_imm
|                  cat2_opc_2src dst_reg ',' src_reg_or_const_or_rel_or_imm ',' src_reg_or_const_or_rel_or_imm

cat3_dp_signedness:'.' T_MIXED   { instr->cat3.signedness = IR3_SRC_MIXED; }
|                  '.' T_UNSIGNED{ instr->cat3.signedness = IR3_SRC_UNSIGNED; }

cat3_dp_pack:      '.' T_LOW     { instr->cat3.packed = IR3_SRC_PACKED_LOW; }
|                  '.' T_HIGH    { instr->cat3.packed = IR3_SRC_PACKED_HIGH; }

cat3_opc:          T_OP_MAD_U16   { new_instr(OPC_MAD_U16); }
|                  T_OP_MADSH_U16 { new_instr(OPC_MADSH_U16); }
|                  T_OP_MAD_S16   { new_instr(OPC_MAD_S16); }
|                  T_OP_MADSH_M16 { new_instr(OPC_MADSH_M16); }
|                  T_OP_MAD_U24   { new_instr(OPC_MAD_U24); }
|                  T_OP_MAD_S24   { new_instr(OPC_MAD_S24); }
|                  T_OP_MAD_F16   { new_instr(OPC_MAD_F16); }
|                  T_OP_MAD_F32   { new_instr(OPC_MAD_F32); }
|                  T_OP_SEL_B16   { new_instr(OPC_SEL_B16); }
|                  T_OP_SEL_B32   { new_instr(OPC_SEL_B32); }
|                  T_OP_SEL_S16   { new_instr(OPC_SEL_S16); }
|                  T_OP_SEL_S32   { new_instr(OPC_SEL_S32); }
|                  T_OP_SEL_F16   { new_instr(OPC_SEL_F16); }
|                  T_OP_SEL_F32   { new_instr(OPC_SEL_F32); }
|                  T_OP_SAD_S16   { new_instr(OPC_SAD_S16); }
|                  T_OP_SAD_S32   { new_instr(OPC_SAD_S32); }

cat3_imm_reg_opc:  T_OP_SHRM      { new_instr(OPC_SHRM); }
|                  T_OP_SHLM      { new_instr(OPC_SHLM); }
|                  T_OP_SHRG      { new_instr(OPC_SHRG); }
|                  T_OP_SHLG      { new_instr(OPC_SHLG); }
|                  T_OP_ANDG      { new_instr(OPC_ANDG); }

cat3_wmm:          T_OP_WMM       { new_instr(OPC_WMM); }
|                  T_OP_WMM_ACCU  { new_instr(OPC_WMM_ACCU); }

cat3_dp:           T_OP_DP2ACC    { new_instr(OPC_DP2ACC); }
|                  T_OP_DP4ACC    { new_instr(OPC_DP4ACC); }

cat3_instr:        cat3_opc dst_reg ',' src_reg_or_const_or_rel ',' src_reg_or_const ',' src_reg_or_const_or_rel
|                  cat3_imm_reg_opc dst_reg ',' src_reg_or_rel_or_imm ',' src_reg_or_const ',' src_reg_or_rel_or_imm
|                  cat3_wmm         dst_reg ',' src_reg_gpr ',' src_reg ',' immediate
|                  cat3_dp cat3_dp_signedness cat3_dp_pack dst_reg ',' src_reg_or_rel_or_imm ',' src_reg_or_const ',' src_reg_or_rel_or_imm

cat4_opc:          T_OP_RCP       { new_instr(OPC_RCP); }
|                  T_OP_RSQ       { new_instr(OPC_RSQ); }
|                  T_OP_LOG2      { new_instr(OPC_LOG2); }
|                  T_OP_EXP2      { new_instr(OPC_EXP2); }
|                  T_OP_SIN       { new_instr(OPC_SIN); }
|                  T_OP_COS       { new_instr(OPC_COS); }
|                  T_OP_SQRT      { new_instr(OPC_SQRT); }
|                  T_OP_HRSQ      { new_instr(OPC_HRSQ); }
|                  T_OP_HLOG2     { new_instr(OPC_HLOG2); }
|                  T_OP_HEXP2     { new_instr(OPC_HEXP2); }

cat4_instr:        cat4_opc dst_reg ',' src_reg_or_const_or_rel_or_imm

cat5_opc_dsxypp:   T_OP_DSXPP_1   { new_instr(OPC_DSXPP_1)->cat5.type = TYPE_F32; }
|                  T_OP_DSYPP_1   { new_instr(OPC_DSYPP_1)->cat5.type = TYPE_F32; }

cat5_opc:          T_OP_ISAM      { new_instr(OPC_ISAM); }
|                  T_OP_ISAML     { new_instr(OPC_ISAML); }
|                  T_OP_ISAMM     { new_instr(OPC_ISAMM); }
|                  T_OP_SAM       { new_instr(OPC_SAM); }
|                  T_OP_SAMB      { new_instr(OPC_SAMB); }
|                  T_OP_SAML      { new_instr(OPC_SAML); }
|                  T_OP_SAMGQ     { new_instr(OPC_SAMGQ); }
|                  T_OP_GETLOD    { new_instr(OPC_GETLOD); }
|                  T_OP_CONV      { new_instr(OPC_CONV); }
|                  T_OP_CONVM     { new_instr(OPC_CONVM); }
|                  T_OP_GETSIZE   { new_instr(OPC_GETSIZE); }
|                  T_OP_GETBUF    { new_instr(OPC_GETBUF); }
|                  T_OP_GETPOS    { new_instr(OPC_GETPOS); }
|                  T_OP_GETINFO   { new_instr(OPC_GETINFO); }
|                  T_OP_DSX       { new_instr(OPC_DSX); }
|                  T_OP_DSY       { new_instr(OPC_DSY); }
|                  T_OP_GATHER4R  { new_instr(OPC_GATHER4R); }
|                  T_OP_GATHER4G  { new_instr(OPC_GATHER4G); }
|                  T_OP_GATHER4B  { new_instr(OPC_GATHER4B); }
|                  T_OP_GATHER4A  { new_instr(OPC_GATHER4A); }
|                  T_OP_SAMGP0    { new_instr(OPC_SAMGP0); }
|                  T_OP_SAMGP1    { new_instr(OPC_SAMGP1); }
|                  T_OP_SAMGP2    { new_instr(OPC_SAMGP2); }
|                  T_OP_SAMGP3    { new_instr(OPC_SAMGP3); }
|                  T_OP_RGETPOS   { new_instr(OPC_RGETPOS); }
|                  T_OP_RGETINFO  { new_instr(OPC_RGETINFO); }
|                  T_OP_BRCST_A   { new_instr(OPC_BRCST_ACTIVE); }
|                  T_OP_QSHUFFLE_BRCST { new_instr(OPC_QUAD_SHUFFLE_BRCST); }
|                  T_OP_QSHUFFLE_H     { new_instr(OPC_QUAD_SHUFFLE_HORIZ); }
|                  T_OP_QSHUFFLE_V     { new_instr(OPC_QUAD_SHUFFLE_VERT); }
|                  T_OP_QSHUFFLE_DIAG  { new_instr(OPC_QUAD_SHUFFLE_DIAG); }

cat5_flag:         '.' T_3D       { instr->flags |= IR3_INSTR_3D; }
|                  '.' 'a'        { instr->flags |= IR3_INSTR_A; }
|                  '.' 'o'        { instr->flags |= IR3_INSTR_O; }
|                  '.' 'p'        { instr->flags |= IR3_INSTR_P; }
|                  '.' 's'        { instr->flags |= IR3_INSTR_S; }
|                  '.' T_S2EN     { instr->flags |= IR3_INSTR_S2EN; }
|                  '.' T_UNIFORM  { }
|                  '.' T_NONUNIFORM  { instr->flags |= IR3_INSTR_NONUNIF; }
|                  '.' T_BASE     { instr->flags |= IR3_INSTR_B; instr->cat5.tex_base = $2; }
|                  '.' T_W        { instr->cat5.cluster_size = $2; }
cat5_flags:
|                  cat5_flag cat5_flags

cat5_samp:         T_SAMP         { instr->cat5.samp = $1; }
cat5_tex:          T_TEX          { instr->cat5.tex = $1; }
cat5_type:         '(' type ')'   { instr->cat5.type = $2; }
cat5_a1:           src_reg        { instr->flags |= IR3_INSTR_A1EN; }

cat5_instr:        cat5_opc_dsxypp cat5_flags dst_reg ',' src_reg
|                  cat5_opc cat5_flags cat5_type dst_reg ',' src_reg ',' src_reg ',' src_reg
|                  cat5_opc cat5_flags cat5_type dst_reg ',' src_reg ',' src_reg ',' cat5_samp ',' cat5_tex
|                  cat5_opc cat5_flags cat5_type dst_reg ',' src_reg ',' src_reg ',' cat5_samp ',' cat5_a1
|                  cat5_opc cat5_flags cat5_type dst_reg ',' src_reg ',' src_reg ',' cat5_tex ',' cat5_a1
|                  cat5_opc cat5_flags cat5_type dst_reg ',' src_reg ',' src_reg ',' cat5_samp
|                  cat5_opc cat5_flags cat5_type dst_reg ',' src_reg ',' src_reg ',' cat5_tex
|                  cat5_opc cat5_flags cat5_type dst_reg ',' src_reg ',' src_reg
|                  cat5_opc cat5_flags cat5_type dst_reg ',' src_reg ',' cat5_samp ',' cat5_tex
|                  cat5_opc cat5_flags cat5_type dst_reg ',' src_reg ',' cat5_samp ',' cat5_a1
|                  cat5_opc cat5_flags cat5_type dst_reg ',' src_reg ',' cat5_tex ',' cat5_a1
|                  cat5_opc cat5_flags cat5_type dst_reg ',' src_reg ',' cat5_samp
|                  cat5_opc cat5_flags cat5_type dst_reg ',' src_reg ',' cat5_tex
|                  cat5_opc cat5_flags cat5_type dst_reg ',' src_reg
|                  cat5_opc cat5_flags cat5_type dst_reg ',' cat5_samp ',' cat5_tex
|                  cat5_opc cat5_flags cat5_type dst_reg ',' cat5_samp
|                  cat5_opc cat5_flags cat5_type dst_reg ',' cat5_tex
|                  cat5_opc cat5_flags cat5_type dst_reg
|                  T_OP_TCINV { new_instr(OPC_TCINV); }

cat6_typed:        '.' T_UNTYPED  { instr->cat6.typed = 0; }
|                  '.' T_TYPED    { instr->cat6.typed = 1; }

cat6_dim:          '.' T_1D  { instr->cat6.d = 1; }
|                  '.' T_2D  { instr->cat6.d = 2; }
|                  '.' T_3D  { instr->cat6.d = 3; }
|                  '.' T_4D  { instr->cat6.d = 4; }

cat6_type:         '.' type  { instr->cat6.type = $2; }
cat6_imm_offset:   offset    { new_src(0, IR3_REG_IMMED)->iim_val = $1; }
cat6_offset:       cat6_imm_offset
|                  '+' src
cat6_dst_offset:   offset    { instr->cat6.dst_offset = $1; }
|                  '+' src

cat6_immed:        integer   { instr->cat6.iim_val = $1; }

cat6_a6xx_global_address_pt3:
                   '<' '<' integer offset '<' '<' integer {
                        assert($7 == 2);
                        new_src(0, IR3_REG_IMMED)->uim_val = $3 - 2;
                        new_src(0, IR3_REG_IMMED)->uim_val = $4;
                   }
|                  '+' cat6_reg_or_immed {
                        // Dummy src to smooth the difference between a6xx and a7xx
                        new_src(0, IR3_REG_IMMED)->uim_val = 0;
                   }

cat6_a6xx_global_address_pt2:
                   '(' src offset ')' '<' '<' integer {
                        assert($7 == 2);
                        new_src(0, IR3_REG_IMMED)->uim_val = 0;
                        new_src(0, IR3_REG_IMMED)->uim_val = $3;
                   }

|                  src cat6_a6xx_global_address_pt3

cat6_a6xx_global_address:
                   src_reg_or_const '+' cat6_a6xx_global_address_pt2

cat6_load:         T_OP_LDG   { new_instr(OPC_LDG); }   cat6_type dst_reg ',' 'g' '[' src cat6_offset ']' ',' immediate
|                  T_OP_LDG_A { new_instr(OPC_LDG_A); } cat6_type dst_reg ',' 'g' '[' cat6_a6xx_global_address ']' ',' immediate
|                  T_OP_LDP   { new_instr(OPC_LDP); }   cat6_type dst_reg ',' 'p' '[' src cat6_offset ']' ',' immediate
|                  T_OP_LDL   { new_instr(OPC_LDL); }   cat6_type dst_reg ',' 'l' '[' src cat6_offset ']' ',' immediate
|                  T_OP_LDLW  { new_instr(OPC_LDLW); }  cat6_type dst_reg ',' 'l' '[' src cat6_offset ']' ',' immediate
|                  T_OP_LDLV  { new_instr(OPC_LDLV); }  cat6_type dst_reg ',' 'l' '[' integer ']' {
                       new_src(0, IR3_REG_IMMED)->iim_val = $8;
                   } ',' immediate

cat6_store:        T_OP_STG   { new_instr(OPC_STG); dummy_dst(); }   cat6_type 'g' '[' src cat6_imm_offset ']' ',' src ',' immediate
|                  T_OP_STG_A { new_instr(OPC_STG_A); dummy_dst(); } cat6_type 'g' '[' cat6_a6xx_global_address ']' ',' src ',' immediate
|                  T_OP_STP  { new_instr(OPC_STP); dummy_dst(); }  cat6_type 'p' '[' src cat6_dst_offset ']' ',' src ',' immediate
|                  T_OP_STL  { new_instr(OPC_STL); dummy_dst(); }  cat6_type 'l' '[' src cat6_dst_offset ']' ',' src ',' immediate
|                  T_OP_STLW { new_instr(OPC_STLW); dummy_dst(); } cat6_type 'l' '[' src cat6_dst_offset ']' ',' src ',' immediate

cat6_loadib:       T_OP_LDIB { new_instr(OPC_LDIB); } cat6_typed cat6_dim cat6_type '.' cat6_immed dst_reg ',' 'g' '[' immediate ']' ',' src ',' src
cat6_storeib:      T_OP_STIB { new_instr(OPC_STIB); dummy_dst(); } cat6_typed cat6_dim cat6_type '.' cat6_immed'g' '[' immediate ']' ',' src ',' src ',' src

cat6_prefetch:     T_OP_PREFETCH { new_instr(OPC_PREFETCH); new_dst(0,0); /* dummy dst */ } 'g' '[' src cat6_offset ']' ',' cat6_immed

cat6_atomic_opc:   T_OP_ATOMIC_ADD     { new_instr(OPC_ATOMIC_ADD); }
|                  T_OP_ATOMIC_SUB     { new_instr(OPC_ATOMIC_SUB); }
|                  T_OP_ATOMIC_XCHG    { new_instr(OPC_ATOMIC_XCHG); }
|                  T_OP_ATOMIC_INC     { new_instr(OPC_ATOMIC_INC); }
|                  T_OP_ATOMIC_DEC     { new_instr(OPC_ATOMIC_DEC); }
|                  T_OP_ATOMIC_CMPXCHG { new_instr(OPC_ATOMIC_CMPXCHG); }
|                  T_OP_ATOMIC_MIN     { new_instr(OPC_ATOMIC_MIN); }
|                  T_OP_ATOMIC_MAX     { new_instr(OPC_ATOMIC_MAX); }
|                  T_OP_ATOMIC_AND     { new_instr(OPC_ATOMIC_AND); }
|                  T_OP_ATOMIC_OR      { new_instr(OPC_ATOMIC_OR); }
|                  T_OP_ATOMIC_XOR     { new_instr(OPC_ATOMIC_XOR); }

cat6_a3xx_atomic_opc:   T_OP_ATOMIC_S_ADD     { new_instr(OPC_ATOMIC_S_ADD); }
|                       T_OP_ATOMIC_S_SUB     { new_instr(OPC_ATOMIC_S_SUB); }
|                       T_OP_ATOMIC_S_XCHG    { new_instr(OPC_ATOMIC_S_XCHG); }
|                       T_OP_ATOMIC_S_INC     { new_instr(OPC_ATOMIC_S_INC); }
|                       T_OP_ATOMIC_S_DEC     { new_instr(OPC_ATOMIC_S_DEC); }
|                       T_OP_ATOMIC_S_CMPXCHG { new_instr(OPC_ATOMIC_S_CMPXCHG); }
|                       T_OP_ATOMIC_S_MIN     { new_instr(OPC_ATOMIC_S_MIN); }
|                       T_OP_ATOMIC_S_MAX     { new_instr(OPC_ATOMIC_S_MAX); }
|                       T_OP_ATOMIC_S_AND     { new_instr(OPC_ATOMIC_S_AND); }
|                       T_OP_ATOMIC_S_OR      { new_instr(OPC_ATOMIC_S_OR); }
|                       T_OP_ATOMIC_S_XOR     { new_instr(OPC_ATOMIC_S_XOR); }

cat6_a6xx_atomic_opc:   T_OP_ATOMIC_G_ADD     { new_instr(OPC_ATOMIC_G_ADD); }
|                       T_OP_ATOMIC_G_SUB     { new_instr(OPC_ATOMIC_G_SUB); }
|                       T_OP_ATOMIC_G_XCHG    { new_instr(OPC_ATOMIC_G_XCHG); }
|                       T_OP_ATOMIC_G_INC     { new_instr(OPC_ATOMIC_G_INC); }
|                       T_OP_ATOMIC_G_DEC     { new_instr(OPC_ATOMIC_G_DEC); }
|                       T_OP_ATOMIC_G_CMPXCHG { new_instr(OPC_ATOMIC_G_CMPXCHG); }
|                       T_OP_ATOMIC_G_MIN     { new_instr(OPC_ATOMIC_G_MIN); }
|                       T_OP_ATOMIC_G_MAX     { new_instr(OPC_ATOMIC_G_MAX); }
|                       T_OP_ATOMIC_G_AND     { new_instr(OPC_ATOMIC_G_AND); }
|                       T_OP_ATOMIC_G_OR      { new_instr(OPC_ATOMIC_G_OR); }
|                       T_OP_ATOMIC_G_XOR     { new_instr(OPC_ATOMIC_G_XOR); }

cat6_a3xx_atomic_s: cat6_a3xx_atomic_opc cat6_typed cat6_dim cat6_type '.' cat6_immed '.' 'g' dst_reg ',' 'g' '[' cat6_reg_or_immed ']' ',' src ',' src ',' src

cat6_a6xx_atomic_g: cat6_a6xx_atomic_opc cat6_typed cat6_dim cat6_type '.' cat6_immed '.' 'g' dst_reg ',' src ',' src

cat6_atomic_l:     cat6_atomic_opc cat6_typed cat6_dim cat6_type '.' cat6_immed '.' 'l' dst_reg ',' 'l' '[' cat6_reg_or_immed ']' ',' src

cat6_atomic:       cat6_atomic_l
|                  cat6_a3xx_atomic_s
|                  cat6_a6xx_atomic_g

cat6_ibo_opc_1src: T_OP_RESINFO   { new_instr(OPC_RESINFO); }

cat6_ibo_opc_ldgb: T_OP_LDGB      { new_instr(OPC_LDGB); }
cat6_ibo_opc_stgb: T_OP_STGB      { new_instr(OPC_STGB); }

cat6_ibo:          cat6_ibo_opc_1src cat6_type cat6_dim dst_reg ',' 'g' '[' cat6_reg_or_immed ']'
|                  cat6_ibo_opc_ldgb cat6_typed cat6_dim cat6_type '.' cat6_immed dst_reg ',' 'g' '[' cat6_reg_or_immed ']' ',' src ',' src
|                  cat6_ibo_opc_stgb cat6_typed cat6_dim cat6_type '.' cat6_immed { dummy_dst(); } 'g' '[' cat6_reg_or_immed ']' ',' src ',' cat6_reg_or_immed ',' src

cat6_id_opc:
                   T_OP_GETSPID { new_instr(OPC_GETSPID); }
|                  T_OP_GETWID  { new_instr(OPC_GETWID); }
|                  T_OP_GETFIBERID { new_instr(OPC_GETFIBERID); }

cat6_id:           cat6_id_opc cat6_type dst_reg

cat6_bindless_base:
|                  '.' T_BASE { instr->flags |= IR3_INSTR_B; instr->cat6.base = $2; }

cat6_bindless_mode: T_IMM cat6_bindless_base
|                  T_UNIFORM cat6_bindless_base
|                  T_NONUNIFORM cat6_bindless_base { instr->flags |= IR3_INSTR_NONUNIF; }

cat6_reg_or_immed: src
|                  integer { new_src(0, IR3_REG_IMMED)->iim_val = $1; }

cat6_bindless_ibo_opc_1src: T_OP_RESINFO_B       { new_instr(OPC_RESINFO); }

cat6_bindless_ibo_opc_2src: T_OP_ATOMIC_B_ADD        { new_instr(OPC_ATOMIC_B_ADD); dummy_dst(); }
|                  T_OP_ATOMIC_B_SUB        { new_instr(OPC_ATOMIC_B_SUB); dummy_dst(); }
|                  T_OP_ATOMIC_B_XCHG       { new_instr(OPC_ATOMIC_B_XCHG); dummy_dst(); }
|                  T_OP_ATOMIC_B_INC        { new_instr(OPC_ATOMIC_B_INC); dummy_dst(); }
|                  T_OP_ATOMIC_B_DEC        { new_instr(OPC_ATOMIC_B_DEC); dummy_dst(); }
|                  T_OP_ATOMIC_B_CMPXCHG    { new_instr(OPC_ATOMIC_B_CMPXCHG); dummy_dst(); }
|                  T_OP_ATOMIC_B_MIN        { new_instr(OPC_ATOMIC_B_MIN); dummy_dst(); }
|                  T_OP_ATOMIC_B_MAX        { new_instr(OPC_ATOMIC_B_MAX); dummy_dst(); }
|                  T_OP_ATOMIC_B_AND        { new_instr(OPC_ATOMIC_B_AND); dummy_dst(); }
|                  T_OP_ATOMIC_B_OR         { new_instr(OPC_ATOMIC_B_OR); dummy_dst(); }
|                  T_OP_ATOMIC_B_XOR        { new_instr(OPC_ATOMIC_B_XOR); dummy_dst(); }
|                  T_OP_STIB_B              { new_instr(OPC_STIB); dummy_dst(); }

cat6_bindless_ibo_opc_2src_dst: T_OP_LDIB_B              { new_instr(OPC_LDIB); }

cat6_bindless_ibo: cat6_bindless_ibo_opc_1src cat6_typed cat6_dim cat6_type '.' cat6_immed '.' cat6_bindless_mode dst_reg ',' cat6_reg_or_immed
|                  cat6_bindless_ibo_opc_2src cat6_typed cat6_dim cat6_type '.' cat6_immed '.' cat6_bindless_mode src_reg ',' cat6_reg_or_immed ',' cat6_reg_or_immed { swap(instr->srcs[0], instr->srcs[2]); }
|                  cat6_bindless_ibo_opc_2src_dst cat6_typed cat6_dim cat6_type '.' cat6_immed '.' cat6_bindless_mode dst_reg ',' cat6_reg_or_immed ',' cat6_reg_or_immed { swap(instr->srcs[0], instr->srcs[1]); }

cat6_bindless_ldc_opc: T_OP_LDC  { new_instr(OPC_LDC); }

/* This is separated from the opcode to avoid lookahead/shift-reduce conflicts */
cat6_bindless_ldc_middle:
                        T_OFFSET '.' cat6_immed '.' cat6_bindless_mode dst_reg { instr->cat6.d = $1; }
|                       cat6_immed '.' 'k' '.' cat6_bindless_mode 'c' '[' T_A1 ']' { instr->opc = OPC_LDC_K; }

cat6_bindless_ldc: cat6_bindless_ldc_opc '.' cat6_bindless_ldc_middle ',' cat6_reg_or_immed ',' cat6_reg_or_immed {
                      instr->cat6.type = TYPE_U32;
                      /* TODO cleanup ir3 src order: */
                      swap(instr->srcs[0], instr->srcs[1]);
                   }

stc_dst:          integer { new_src(0, IR3_REG_IMMED)->iim_val = $1; }
|                 T_A1 { new_src(0, IR3_REG_IMMED)->iim_val = 0; instr->flags |= IR3_INSTR_A1EN; }
|                 T_A1 '+' integer { new_src(0, IR3_REG_IMMED)->iim_val = $3; instr->flags |= IR3_INSTR_A1EN; }

cat6_stc:
              T_OP_STC  { new_instr(OPC_STC); }  cat6_type 'c' '[' stc_dst ']' ',' src_reg ',' cat6_immed
|             T_OP_STSC { new_instr(OPC_STSC); } cat6_type 'c' '[' stc_dst ']' ',' immediate ',' cat6_immed

cat6_todo:         T_OP_G2L                 { new_instr(OPC_G2L); }
|                  T_OP_L2G                 { new_instr(OPC_L2G); }
|                  T_OP_RESFMT              { new_instr(OPC_RESFMT); }

cat6_instr:        cat6_load
|                  cat6_loadib
|                  cat6_store
|                  cat6_storeib
|                  cat6_prefetch
|                  cat6_atomic
|                  cat6_ibo
|                  cat6_id
|                  cat6_bindless_ldc
|                  cat6_bindless_ibo
|                  cat6_stc
|                  cat6_todo

cat7_scope:        '.' 'w'  { instr->cat7.w = true; }
|                  '.' 'r'  { instr->cat7.r = true; }
|                  '.' 'l'  { instr->cat7.l = true; }
|                  '.' 'g'  { instr->cat7.g = true; }

cat7_scopes:
|                  cat7_scope cat7_scopes

cat7_barrier:      T_OP_BAR                { new_instr(OPC_BAR); } cat7_scopes
|                  T_OP_FENCE              { new_instr(OPC_FENCE); } cat7_scopes

cat7_data_cache:   T_OP_DCCLN              { new_instr(OPC_DCCLN); }
|                  T_OP_DCINV              { new_instr(OPC_DCINV); }
|                  T_OP_DCFLU              { new_instr(OPC_DCFLU); }

cat7_alias_src:    src_reg_or_const
|                  immediate_cat1

cat7_alias_scope: T_MOD_TEX	{ instr->cat7.alias_scope = ALIAS_TEX; }
|                 T_MOD_MEM	{ instr->cat7.alias_scope = ALIAS_MEM; }
|                 T_MOD_RT	{ instr->cat7.alias_scope = ALIAS_RT; }

cat7_instr:        cat7_barrier
|                  cat7_data_cache
|                  T_OP_SLEEP              { new_instr(OPC_SLEEP); }
|                  T_OP_CCINV              { new_instr(OPC_CCINV); }
|                  T_OP_ICINV              { new_instr(OPC_ICINV); }
|                  T_OP_LOCK               { new_instr(OPC_LOCK); }
|                  T_OP_UNLOCK             { new_instr(OPC_UNLOCK); }
|                  T_OP_ALIAS {
                       /* TODO: handle T_INSTR_TYPE */
                       new_instr(OPC_ALIAS);
                   } '.' cat7_alias_scope '.' T_INSTR_TYPE '.' integer dst_reg ',' cat7_alias_src {
                       new_src(0, IR3_REG_IMMED)->uim_val = $8;
                   }

raw_instr: T_RAW   {new_instr(OPC_META_RAW)->raw.value = $1;}

meta_print: T_OP_PRINT T_REGISTER ',' T_REGISTER {
	/* low */
	new_instr(OPC_MOV);
	instr->cat1.src_type = TYPE_U32;
	instr->cat1.dst_type = TYPE_U32;
	new_dst($2, IR3_REG_R);
	new_src(0, IR3_REG_IMMED)->uim_val = info->shader_print_buffer_iova & 0xffffffff;

	/* high */
	new_instr(OPC_MOV);
	instr->cat1.src_type = TYPE_U32;
	instr->cat1.dst_type = TYPE_U32;
	new_dst($2 + 2, IR3_REG_R);
	new_src(0, IR3_REG_IMMED)->uim_val = info->shader_print_buffer_iova >> 32;

	/* offset */
	new_instr(OPC_MOV);
	instr->cat1.src_type = TYPE_U32;
	instr->cat1.dst_type = TYPE_U32;
	new_dst($2 + 4, IR3_REG_R);
	new_src(0, IR3_REG_IMMED)->uim_val = 4;

	new_instr(OPC_NOP);
	instr->repeat = 5;

	/* Increment and get current offset into print buffer */
	new_instr(OPC_ATOMIC_G_ADD);
	instr->cat6.d = 1;
	instr->cat6.typed = 0;
	instr->cat6.type = TYPE_U32;
	instr->cat6.iim_val = 1;

	new_dst($2, IR3_REG_R);
	new_src($2, IR3_REG_R);
	new_src($2 + 4, IR3_REG_R);

	/* Store the value */
	new_instr(OPC_STG);
	dummy_dst();
	instr->cat6.type = TYPE_U32;
	instr->flags = IR3_INSTR_SY;
	new_src($2, IR3_REG_R);
	new_src(0, IR3_REG_IMMED)->iim_val = 0;
	new_src($4, IR3_REG_R);
	new_src(0, IR3_REG_IMMED)->iim_val = 1;

	new_instr(OPC_NOP);
	instr->flags = IR3_INSTR_SY;
}

src:               T_REGISTER     { $$ = new_src($1, 0); }
|                  T_A0           { $$ = new_src((61 << 3), IR3_REG_HALF); }
|                  T_A1           { $$ = new_src((61 << 3) + 1, IR3_REG_HALF); }
|                  T_P0           { $$ = new_src((62 << 3) + $1, 0); }

dst:               T_REGISTER     { $$ = new_dst($1, 0); }
|                  T_A0           { $$ = new_dst((61 << 3), IR3_REG_HALF); }
|                  T_A1           { $$ = new_dst((61 << 3) + 1, IR3_REG_HALF); }
|                  T_P0           { $$ = new_dst((62 << 3) + $1, 0); }

const:             T_CONSTANT     { $$ = new_src($1, IR3_REG_CONST); }

dst_reg_flag:      T_EVEN         { instr->cat1.round = ROUND_EVEN; }
|                  T_POS_INFINITY { instr->cat1.round = ROUND_POS_INF; }
|                  T_NEG_INFINITY { instr->cat1.round = ROUND_NEG_INF; }
|                  T_EI           { rflags.flags |= IR3_REG_EI; }
|                  T_WRMASK       { rflags.wrmask = $1; }

dst_reg_flags:     dst_reg_flag
|                  dst_reg_flag dst_reg_flags

                   /* note: destination registers are always incremented in repeat */
dst_reg:           dst                 { $1->flags |= IR3_REG_R; }
|                  dst_reg_flags dst   { $2->flags |= IR3_REG_R; }

src_reg_flag:      T_ABSNEG       { rflags.flags |= IR3_REG_ABS|IR3_REG_NEGATE; }
|                  T_NEG          { rflags.flags |= IR3_REG_NEGATE; }
|                  T_ABS          { rflags.flags |= IR3_REG_ABS; }
|                  T_R            { rflags.flags |= IR3_REG_R; }
|                  T_LAST         { rflags.flags |= IR3_REG_LAST_USE; }

src_reg_flags:     src_reg_flag
|                  src_reg_flag src_reg_flags

src_reg:           src
|                  src_reg_flags src

src_reg_gpr:       src_reg
|                  relative_gpr_src

src_const:         const
|                  src_reg_flags const

src_reg_or_const:  src_reg
|                  src_const

src_reg_or_const_or_rel: src_reg_or_const
|                  relative
|                  src_reg_flags relative

src_reg_or_const_or_rel_or_imm: src_reg_or_const_or_rel
|                  src_reg_flags immediate
|                  immediate

src_reg_or_rel_or_imm: src_reg
|                  relative
|                  immediate

offset:            { $$ = 0; }
|                  '+' integer { $$ = $2; }
|                  '-' integer { $$ = -$2; }

relative_gpr_src:  'r' '<' T_A0 offset '>'  { new_src(0, IR3_REG_RELATIV)->array.offset = $4; }
|                  T_HR '<' T_A0 offset '>'  { new_src(0, IR3_REG_RELATIV | IR3_REG_HALF)->array.offset = $4; }

relative_gpr_dst:  'r' '<' T_A0 offset '>'  { new_dst(0, IR3_REG_RELATIV)->array.offset = $4; }
|                  T_HR '<' T_A0 offset '>'  { new_dst(0, IR3_REG_RELATIV | IR3_REG_HALF)->array.offset = $4; }

relative_const:    'c' '<' T_A0 offset '>'  { new_src(0, IR3_REG_RELATIV | IR3_REG_CONST)->array.offset = $4; }
|                  T_HC '<' T_A0 offset '>'  { new_src(0, IR3_REG_RELATIV | IR3_REG_CONST | IR3_REG_HALF)->array.offset = $4; }

relative:          relative_gpr_src
|                  relative_const

/* cat1 immediates differ slighly in the floating point case from the cat2
 * case which can only encode certain predefined values (ie. and index into
 * the FLUT table)
 */
immediate_cat1:    integer             { new_src(0, IR3_REG_IMMED)->iim_val = type_size(instr->cat1.src_type) < 32 ? $1 & 0xffff : $1; }
|                  '(' integer ')'     { new_src(0, IR3_REG_IMMED)->fim_val = $2; }
|                  '(' float ')'       { new_src(0, IR3_REG_IMMED)->fim_val = $2; }
|                  'h' '(' integer ')' { new_src(0, IR3_REG_IMMED | IR3_REG_HALF)->iim_val = $3 & 0xffff; }
|                  'h' '(' float ')'   { new_src(0, IR3_REG_IMMED | IR3_REG_HALF)->uim_val = _mesa_float_to_half($3); }
|                  '(' T_NAN ')'       { new_src(0, IR3_REG_IMMED)->fim_val = NAN; }
|                  '(' T_INF ')'       { new_src(0, IR3_REG_IMMED)->fim_val = INFINITY; }

immediate:         integer             { new_src(0, IR3_REG_IMMED)->iim_val = $1; }
|                  '(' integer ')'     { new_src(0, IR3_REG_IMMED)->fim_val = $2; }
|                  flut_immed          { new_src(0, IR3_REG_IMMED)->uim_val = $1; }
|                  'h' '(' integer ')' { new_src(0, IR3_REG_IMMED | IR3_REG_HALF)->iim_val = $3; }
|                  'h' flut_immed      { new_src(0, IR3_REG_IMMED | IR3_REG_HALF)->uim_val = $2; }

/* Float LUT values accepted as immed: */
flut_immed:        T_FLUT_0_0
|                  T_FLUT_0_5
|                  T_FLUT_1_0
|                  T_FLUT_2_0
|                  T_FLUT_E
|                  T_FLUT_PI
|                  T_FLUT_INV_PI
|                  T_FLUT_INV_LOG2_E
|                  T_FLUT_LOG2_E
|                  T_FLUT_INV_LOG2_10
|                  T_FLUT_LOG2_10
|                  T_FLUT_4_0

integer:           T_INT       { $$ = $1; }
|                  '-' T_INT   { $$ = -$2; }
|                  T_HEX       { $$ = $1; }
|                  '-' T_HEX   { $$ = -$2; }

float:             T_FLOAT     { $$ = $1; }
|                  '-' T_FLOAT { $$ = -$2; }

type:              T_TYPE_F16  { $$ = TYPE_F16; }
|                  T_TYPE_F32  { $$ = TYPE_F32; }
|                  T_TYPE_U16  { $$ = TYPE_U16; }
|                  T_TYPE_U32  { $$ = TYPE_U32; }
|                  T_TYPE_S16  { $$ = TYPE_S16; }
|                  T_TYPE_S32  { $$ = TYPE_S32; }
|                  T_TYPE_U8   { $$ = TYPE_U8;  }
|                  T_TYPE_S8   { $$ = TYPE_S8;  }
