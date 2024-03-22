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

#include "radeon_compiler.h"
#include "radeon_code.h"
#include "r300_reg.h"

#include <stdio.h>

static const char* r300_vs_ve_ops[] = {
	/* R300 vector ops */
	"                 VE_NO_OP",
	"           VE_DOT_PRODUCT",
	"              VE_MULTIPLY",
	"                   VE_ADD",
	"          VE_MULTIPLY_ADD",
	"       VE_DISTANCE_FACTOR",
	"              VE_FRACTION",
	"               VE_MAXIMUM",
	"               VE_MINIMUM",
	"VE_SET_GREATER_THAN_EQUAL",
	"         VE_SET_LESS_THAN",
	"        VE_MULTIPLYX2_ADD",
	"        VE_MULTIPLY_CLAMP",
	"            VE_FLT2FIX_DX",
	"        VE_FLT2FIX_DX_RND",
	/* R500 vector ops */
	"      VE_PRED_SET_EQ_PUSH",
	"      VE_PRED_SET_GT_PUSH",
	"     VE_PRED_SET_GTE_PUSH",
	"     VE_PRED_SET_NEQ_PUSH",
	"         VE_COND_WRITE_EQ",
	"         VE_COND_WRITE_GT",
	"        VE_COND_WRITE_GTE",
	"        VE_COND_WRITE_NEQ",
	"           VE_COND_MUX_EQ",
	"           VE_COND_MUX_GT",
	"          VE_COND_MUX_GTE",
	"      VE_SET_GREATER_THAN",
	"             VE_SET_EQUAL",
	"         VE_SET_NOT_EQUAL",
	"               (reserved)",
	"               (reserved)",
	"               (reserved)",
};

static const char* r300_vs_me_ops[] = {
	/* R300 math ops */
	"                 ME_NO_OP",
	"          ME_EXP_BASE2_DX",
	"          ME_LOG_BASE2_DX",
	"          ME_EXP_BASEE_FF",
	"        ME_LIGHT_COEFF_DX",
	"         ME_POWER_FUNC_FF",
	"              ME_RECIP_DX",
	"              ME_RECIP_FF",
	"         ME_RECIP_SQRT_DX",
	"         ME_RECIP_SQRT_FF",
	"              ME_MULTIPLY",
	"     ME_EXP_BASE2_FULL_DX",
	"     ME_LOG_BASE2_FULL_DX",
	" ME_POWER_FUNC_FF_CLAMP_B",
	"ME_POWER_FUNC_FF_CLAMP_B1",
	"ME_POWER_FUNC_FF_CLAMP_01",
	"                   ME_SIN",
	"                   ME_COS",
	/* R500 math ops */
	"        ME_LOG_BASE2_IEEE",
	"            ME_RECIP_IEEE",
	"       ME_RECIP_SQRT_IEEE",
	"           ME_PRED_SET_EQ",
	"           ME_PRED_SET_GT",
	"          ME_PRED_SET_GTE",
	"          ME_PRED_SET_NEQ",
	"          ME_PRED_SET_CLR",
	"          ME_PRED_SET_INV",
	"          ME_PRED_SET_POP",
	"      ME_PRED_SET_RESTORE",
	"               (reserved)",
	"               (reserved)",
	"               (reserved)",
};

/* XXX refactor to avoid clashing symbols */
static const char* r300_vs_src_debug[] = {
	"t",
	"i",
	"c",
	"a",
};

static const char* r300_vs_dst_debug[] = {
	"t",
	"a0",
	"o",
	"ox",
	"a",
	"i",
	"u",
	"u",
};

static const char* r300_vs_swiz_debug[] = {
	"X",
	"Y",
	"Z",
	"W",
	"0",
	"1",
	"U",
	"U",
};


static void r300_vs_op_dump(uint32_t op)
{
	fprintf(stderr, " dst: %d%s op: ",
			(op >> 13) & 0x7f, r300_vs_dst_debug[(op >> 8) & 0x7]);
	if ((op >> PVS_DST_PRED_ENABLE_SHIFT) & 0x1) {
		fprintf(stderr, "PRED %u",
				(op >> PVS_DST_PRED_SENSE_SHIFT) & 0x1);
	}
	if (op & 0x80) {
		if (op & 0x1) {
			fprintf(stderr, "PVS_MACRO_OP_2CLK_M2X_ADD\n");
		} else {
			fprintf(stderr, "   PVS_MACRO_OP_2CLK_MADD\n");
		}
	} else if (op & 0x40) {
		fprintf(stderr, "%s\n", r300_vs_me_ops[op & 0x1f]);
	} else {
		fprintf(stderr, "%s\n", r300_vs_ve_ops[op & 0x1f]);
	}
}

static void r300_vs_src_dump(uint32_t src)
{
	fprintf(stderr, " reg: %d%s swiz: %s%s/%s%s/%s%s/%s%s\n",
			(src >> 5) & 0xff, r300_vs_src_debug[src & 0x3],
			src & (1 << 25) ? "-" : " ",
			r300_vs_swiz_debug[(src >> 13) & 0x7],
			src & (1 << 26) ? "-" : " ",
			r300_vs_swiz_debug[(src >> 16) & 0x7],
			src & (1 << 27) ? "-" : " ",
			r300_vs_swiz_debug[(src >> 19) & 0x7],
			src & (1 << 28) ? "-" : " ",
			r300_vs_swiz_debug[(src >> 22) & 0x7]);
}

void r300_vertex_program_dump(struct radeon_compiler *compiler, void *user)
{
	struct r300_vertex_program_compiler *c = (struct r300_vertex_program_compiler*)compiler;
	struct r300_vertex_program_code * vs = c->code;
	unsigned instrcount = vs->length / 4;
	unsigned i;

	fprintf(stderr, "Final vertex program code:\n");

	for(i = 0; i < instrcount; i++) {
		unsigned offset = i*4;
		unsigned src;

		fprintf(stderr, "%d: op: 0x%08x", i, vs->body.d[offset]);
		r300_vs_op_dump(vs->body.d[offset]);

		for(src = 0; src < 3; ++src) {
			fprintf(stderr, " src%i: 0x%08x", src, vs->body.d[offset+1+src]);
			r300_vs_src_dump(vs->body.d[offset+1+src]);
		}
	}

	fprintf(stderr, "Flow Control Ops: 0x%08x\n",vs->fc_ops);
	for(i = 0; i < vs->num_fc_ops; i++) {
		unsigned is_loop = 0;
		switch((vs->fc_ops >> (i * 2)) & 0x3 ) {
		case 0: fprintf(stderr, "NOP"); break;
		case 1: fprintf(stderr, "JUMP"); break;
		case 2: fprintf(stderr, "LOOP"); is_loop = 1; break;
		case 3: fprintf(stderr, "JSR"); break;
		}
		if (c->Base.is_r500) {
			fprintf(stderr,": uw-> 0x%08x lw-> 0x%08x "
							"loop data->0x%08x\n",
				vs->fc_op_addrs.r500[i].uw,
				vs->fc_op_addrs.r500[i].lw,
				vs->fc_loop_index[i]);
			if (is_loop) {
				fprintf(stderr, "Before = %u First = %u Last = %u\n",
					vs->fc_op_addrs.r500[i].lw & 0xffff,
					(vs->fc_op_addrs.r500[i].uw >> 16) & 0xffff,
					vs->fc_op_addrs.r500[i].uw  & 0xffff);
			}
		} else {
			fprintf(stderr,": 0x%08x\n", vs->fc_op_addrs.r300[i]);
		}
	}
}
