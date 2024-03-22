/*
 * Copyright 2010 Jerome Glisse <glisse@freedesktop.org>
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
 */
#include "r600_asm.h"
#include "r600_sq.h"
#include "r600_opcodes.h"
#include "r600_formats.h"
#include "r600d.h"
#include "r600d_common.h"

#include <errno.h>
#include <string.h>
#include "compiler/shader_enums.h"
#include "util/u_memory.h"
#include "util/u_math.h"

#define NUM_OF_CYCLES 3
#define NUM_OF_COMPONENTS 4

static inline bool alu_writes(struct r600_bytecode_alu *alu)
{
	return alu->dst.write || alu->is_op3;
}

static inline unsigned int r600_bytecode_get_num_operands(const struct r600_bytecode_alu *alu)
{
	return r600_isa_alu(alu->op)->src_count;
}

static struct r600_bytecode_cf *r600_bytecode_cf(void)
{
	struct r600_bytecode_cf *cf = CALLOC_STRUCT(r600_bytecode_cf);

	if (!cf)
		return NULL;
	list_inithead(&cf->list);
	list_inithead(&cf->alu);
	list_inithead(&cf->vtx);
	list_inithead(&cf->tex);
	list_inithead(&cf->gds);
	return cf;
}

static struct r600_bytecode_alu *r600_bytecode_alu(void)
{
	struct r600_bytecode_alu *alu = CALLOC_STRUCT(r600_bytecode_alu);

	if (!alu)
		return NULL;
	list_inithead(&alu->list);
	return alu;
}

static struct r600_bytecode_vtx *r600_bytecode_vtx(void)
{
	struct r600_bytecode_vtx *vtx = CALLOC_STRUCT(r600_bytecode_vtx);

	if (!vtx)
		return NULL;
	list_inithead(&vtx->list);
	return vtx;
}

static struct r600_bytecode_tex *r600_bytecode_tex(void)
{
	struct r600_bytecode_tex *tex = CALLOC_STRUCT(r600_bytecode_tex);

	if (!tex)
		return NULL;
	list_inithead(&tex->list);
	return tex;
}

static struct r600_bytecode_gds *r600_bytecode_gds(void)
{
	struct r600_bytecode_gds *gds = CALLOC_STRUCT(r600_bytecode_gds);

	if (gds == NULL)
		return NULL;
	list_inithead(&gds->list);
	return gds;
}

static unsigned stack_entry_size(enum radeon_family chip) {
	/* Wavefront size:
	 *   64: R600/RV670/RV770/Cypress/R740/Barts/Turks/Caicos/
	 *       Aruba/Sumo/Sumo2/redwood/juniper
	 *   32: R630/R730/R710/Palm/Cedar
	 *   16: R610/Rs780
	 *
	 * Stack row size:
	 * 	Wavefront Size                        16  32  48  64
	 * 	Columns per Row (R6xx/R7xx/R8xx only)  8   8   4   4
	 * 	Columns per Row (R9xx+)                8   4   4   4 */

	switch (chip) {
	/* FIXME: are some chips missing here? */
	/* wavefront size 16 */
	case CHIP_RV610:
	case CHIP_RS780:
	case CHIP_RV620:
	case CHIP_RS880:
	/* wavefront size 32 */
	case CHIP_RV630:
	case CHIP_RV635:
	case CHIP_RV730:
	case CHIP_RV710:
	case CHIP_PALM:
	case CHIP_CEDAR:
		return 8;

	/* wavefront size 64 */
	default:
		return 4;
	}
}

void r600_bytecode_init(struct r600_bytecode *bc,
			enum amd_gfx_level gfx_level,
			enum radeon_family family,
			bool has_compressed_msaa_texturing)
{
	static unsigned next_shader_id = 0;

	bc->debug_id = ++next_shader_id;

	if ((gfx_level == R600) &&
	    (family != CHIP_RV670 && family != CHIP_RS780 && family != CHIP_RS880)) {
		bc->ar_handling = AR_HANDLE_RV6XX;

		/* Insert a nop after a relative temp write so that a read in
		 * the following instruction group gets the right value.  The
		 * r600 and EG ISA specs both say that read-after-rel-write of a
		 * register in the next instr group is illegal, but apparently
		 * that's not true on all chips (see commit
		 * c96b9834032952492efbd2d1f5511fe225704918).
		 */
		bc->r6xx_nop_after_rel_dst = 1;
	} else if (family == CHIP_RV770) {
		bc->ar_handling = AR_HANDLE_NORMAL;
		bc->r6xx_nop_after_rel_dst = 1;
	} else {
		bc->ar_handling = AR_HANDLE_NORMAL;
		bc->r6xx_nop_after_rel_dst = 0;
	}

	list_inithead(&bc->cf);
	bc->gfx_level = gfx_level;
	bc->family = family;
	bc->has_compressed_msaa_texturing = has_compressed_msaa_texturing;
	bc->stack.entry_size = stack_entry_size(family);
}

int r600_bytecode_add_cf(struct r600_bytecode *bc)
{
	struct r600_bytecode_cf *cf = r600_bytecode_cf();

	if (!cf)
		return -ENOMEM;
	list_addtail(&cf->list, &bc->cf);
	if (bc->cf_last) {
		cf->id = bc->cf_last->id + 2;
		if (bc->cf_last->eg_alu_extended) {
			/* take into account extended alu size */
			cf->id += 2;
			bc->ndw += 2;
		}
	}
	bc->cf_last = cf;
	bc->ncf++;
	bc->ndw += 2;
	bc->force_add_cf = 0;
	bc->ar_loaded = 0;
	return 0;
}

int r600_bytecode_add_output(struct r600_bytecode *bc,
		const struct r600_bytecode_output *output)
{
	int r;

	if (output->gpr >= bc->ngpr)
		bc->ngpr = output->gpr + 1;

	if (bc->cf_last && (bc->cf_last->op == output->op ||
		(bc->cf_last->op == CF_OP_EXPORT &&
		output->op == CF_OP_EXPORT_DONE)) &&
		output->type == bc->cf_last->output.type &&
		output->elem_size == bc->cf_last->output.elem_size &&
		output->swizzle_x == bc->cf_last->output.swizzle_x &&
		output->swizzle_y == bc->cf_last->output.swizzle_y &&
		output->swizzle_z == bc->cf_last->output.swizzle_z &&
		output->swizzle_w == bc->cf_last->output.swizzle_w &&
		output->comp_mask == bc->cf_last->output.comp_mask &&
		(output->burst_count + bc->cf_last->output.burst_count) <= 16) {

		if ((output->gpr + output->burst_count) == bc->cf_last->output.gpr &&
			(output->array_base + output->burst_count) == bc->cf_last->output.array_base) {

			bc->cf_last->op = bc->cf_last->output.op = output->op;
			bc->cf_last->output.gpr = output->gpr;
			bc->cf_last->output.array_base = output->array_base;
			bc->cf_last->output.burst_count += output->burst_count;
			return 0;

		} else if (output->gpr == (bc->cf_last->output.gpr + bc->cf_last->output.burst_count) &&
			output->array_base == (bc->cf_last->output.array_base + bc->cf_last->output.burst_count)) {

			bc->cf_last->op = bc->cf_last->output.op = output->op;
			bc->cf_last->output.burst_count += output->burst_count;
			return 0;
		}
	}

	r = r600_bytecode_add_cf(bc);
	if (r)
		return r;
	bc->cf_last->op = output->op;
	memcpy(&bc->cf_last->output, output, sizeof(struct r600_bytecode_output));
	bc->cf_last->barrier = 1;
	return 0;
}

int r600_bytecode_add_pending_output(struct r600_bytecode *bc,
		const struct r600_bytecode_output *output)
{
	assert(bc->n_pending_outputs + 1 < ARRAY_SIZE(bc->pending_outputs));
	bc->pending_outputs[bc->n_pending_outputs++] = *output;

	return 0;
}

void
r600_bytecode_add_ack(struct r600_bytecode *bc)
{
	bc->need_wait_ack = true;
}

int
r600_bytecode_wait_acks(struct r600_bytecode *bc)
{
	/* Store acks are an R700+ feature. */
	if (bc->gfx_level < R700)
		return 0;

	if (!bc->need_wait_ack)
		return 0;

	int ret = r600_bytecode_add_cfinst(bc, CF_OP_WAIT_ACK);
	if (ret != 0)
		return ret;

	struct r600_bytecode_cf *cf = bc->cf_last;
	cf->barrier = 1;
	/* Request a wait if the number of outstanding acks is > 0 */
	cf->cf_addr = 0;

	return 0;
}

uint32_t
r600_bytecode_write_export_ack_type(struct r600_bytecode *bc, bool indirect)
{
	if (bc->gfx_level >= R700) {
		if (indirect)
			return V_SQ_CF_ALLOC_EXPORT_WORD0_SQ_EXPORT_WRITE_IND_ACK_EG;
		else
			return V_SQ_CF_ALLOC_EXPORT_WORD0_SQ_EXPORT_WRITE_ACK_EG;
	} else {
		if (indirect)
			return V_SQ_CF_ALLOC_EXPORT_WORD0_SQ_EXPORT_WRITE_IND;
		else
			return V_SQ_CF_ALLOC_EXPORT_WORD0_SQ_EXPORT_WRITE;
	}
}

/* alu instructions that can only exits once per group */
static int is_alu_once_inst(struct r600_bytecode_alu *alu)
{
	return r600_isa_alu(alu->op)->flags & (AF_KILL | AF_PRED) || alu->is_lds_idx_op || alu->op == ALU_OP0_GROUP_BARRIER;
}

static int is_alu_reduction_inst(struct r600_bytecode *bc, struct r600_bytecode_alu *alu)
{
	return (r600_isa_alu(alu->op)->flags & AF_REPL) &&
			(r600_isa_alu_slots(bc->isa->hw_class, alu->op) == AF_4V);
}

static int is_alu_mova_inst(struct r600_bytecode_alu *alu)
{
	return r600_isa_alu(alu->op)->flags & AF_MOVA;
}

static int alu_uses_rel(struct r600_bytecode_alu *alu)
{
	unsigned num_src = r600_bytecode_get_num_operands(alu);
	unsigned src;

	if (alu->dst.rel) {
		return 1;
	}

	for (src = 0; src < num_src; ++src) {
		if (alu->src[src].rel) {
			return 1;
		}
	}
	return 0;
}

static int is_lds_read(int sel)
{
  return sel == EG_V_SQ_ALU_SRC_LDS_OQ_A_POP || sel == EG_V_SQ_ALU_SRC_LDS_OQ_B_POP;
}

static int alu_uses_lds(struct r600_bytecode_alu *alu)
{
	unsigned num_src = r600_bytecode_get_num_operands(alu);
	unsigned src;

	for (src = 0; src < num_src; ++src) {
		if (is_lds_read(alu->src[src].sel)) {
			return 1;
		}
	}
	return 0;
}

static int is_alu_64bit_inst(struct r600_bytecode_alu *alu)
{
	const struct alu_op_info *op = r600_isa_alu(alu->op);
	return (op->flags & AF_64);
}

static int is_alu_vec_unit_inst(struct r600_bytecode *bc, struct r600_bytecode_alu *alu)
{
	unsigned slots = r600_isa_alu_slots(bc->isa->hw_class, alu->op);
	return !(slots & AF_S);
}

static int is_alu_trans_unit_inst(struct r600_bytecode *bc, struct r600_bytecode_alu *alu)
{
	unsigned slots = r600_isa_alu_slots(bc->isa->hw_class, alu->op);
	return !(slots & AF_V);
}

/* alu instructions that can execute on any unit */
static int is_alu_any_unit_inst(struct r600_bytecode *bc, struct r600_bytecode_alu *alu)
{
	unsigned slots = r600_isa_alu_slots(bc->isa->hw_class, alu->op);
	return slots == AF_VS;
}

static int is_nop_inst(struct r600_bytecode_alu *alu)
{
	return alu->op == ALU_OP0_NOP;
}

static int assign_alu_units(struct r600_bytecode *bc, struct r600_bytecode_alu *alu_first,
			    struct r600_bytecode_alu *assignment[5])
{
	struct r600_bytecode_alu *alu;
	unsigned i, chan, trans;
	int max_slots = bc->gfx_level == CAYMAN ? 4 : 5;

	for (i = 0; i < max_slots; i++)
		assignment[i] = NULL;

	for (alu = alu_first; alu; alu = list_entry(alu->list.next, struct r600_bytecode_alu, list)) {
		chan = alu->dst.chan;
		if (max_slots == 4)
			trans = 0;
		else if (is_alu_trans_unit_inst(bc, alu))
			trans = 1;
		else if (is_alu_vec_unit_inst(bc, alu))
			trans = 0;
		else if (assignment[chan])
			trans = 1; /* Assume ALU_INST_PREFER_VECTOR. */
		else
			trans = 0;

		if (trans) {
			if (assignment[4]) {
				assert(0); /* ALU.Trans has already been allocated. */
				return -1;
			}
			assignment[4] = alu;
		} else {
                        if (assignment[chan]) {
			 	assert(0); /* ALU.chan has already been allocated. */
				return -1;
			}
			assignment[chan] = alu;
		}

		if (alu->last)
			break;
	}
	return 0;
}

struct alu_bank_swizzle {
	int	hw_gpr[NUM_OF_CYCLES][NUM_OF_COMPONENTS];
	int	hw_cfile_addr[4];
	int	hw_cfile_elem[4];
};

static const unsigned cycle_for_bank_swizzle_vec[][3] = {
	[SQ_ALU_VEC_012] = { 0, 1, 2 },
	[SQ_ALU_VEC_021] = { 0, 2, 1 },
	[SQ_ALU_VEC_120] = { 1, 2, 0 },
	[SQ_ALU_VEC_102] = { 1, 0, 2 },
	[SQ_ALU_VEC_201] = { 2, 0, 1 },
	[SQ_ALU_VEC_210] = { 2, 1, 0 }
};

static const unsigned cycle_for_bank_swizzle_scl[][3] = {
	[SQ_ALU_SCL_210] = { 2, 1, 0 },
	[SQ_ALU_SCL_122] = { 1, 2, 2 },
	[SQ_ALU_SCL_212] = { 2, 1, 2 },
	[SQ_ALU_SCL_221] = { 2, 2, 1 }
};

static void init_bank_swizzle(struct alu_bank_swizzle *bs)
{
	int i, cycle, component;
	/* set up gpr use */
	for (cycle = 0; cycle < NUM_OF_CYCLES; cycle++)
		for (component = 0; component < NUM_OF_COMPONENTS; component++)
			 bs->hw_gpr[cycle][component] = -1;
	for (i = 0; i < 4; i++)
		bs->hw_cfile_addr[i] = -1;
	for (i = 0; i < 4; i++)
		bs->hw_cfile_elem[i] = -1;
}

static int reserve_gpr(struct alu_bank_swizzle *bs, unsigned sel, unsigned chan, unsigned cycle)
{
	if (bs->hw_gpr[cycle][chan] == -1)
		bs->hw_gpr[cycle][chan] = sel;
	else if (bs->hw_gpr[cycle][chan] != (int)sel) {
		/* Another scalar operation has already used the GPR read port for the channel. */
		return -1;
	}
	return 0;
}

static int reserve_cfile(const struct r600_bytecode *bc,
			 struct alu_bank_swizzle *bs, unsigned sel, unsigned chan)
{
	int res, num_res = 4;
	if (bc->gfx_level >= R700) {
		num_res = 2;
		chan /= 2;
	}
	for (res = 0; res < num_res; ++res) {
		if (bs->hw_cfile_addr[res] == -1) {
			bs->hw_cfile_addr[res] = sel;
			bs->hw_cfile_elem[res] = chan;
			return 0;
		} else if (bs->hw_cfile_addr[res] == sel &&
			bs->hw_cfile_elem[res] == chan)
			return 0; /* Read for this scalar element already reserved, nothing to do here. */
	}
	/* All cfile read ports are used, cannot reference vector element. */
	return -1;
}

static int is_gpr(unsigned sel)
{
	return (sel <= 127);
}

/* CB constants start at 512, and get translated to a kcache index when ALU
 * clauses are constructed. Note that we handle kcache constants the same way
 * as (the now gone) cfile constants, is that really required? */
static int is_kcache(unsigned sel)
{
   return (sel > 511 && sel < 4607) || /* Kcache before translation. */
         (sel > 127 && sel < 192) || /* Kcache 0 & 1 after translation. */
         (sel > 256  && sel < 320);  /* Kcache 2 & 3 after translation (EG). */
}

static int is_const(int sel)
{
   return is_kcache(sel) ||
		(sel >= V_SQ_ALU_SRC_0 &&
		sel <= V_SQ_ALU_SRC_LITERAL);
}

static int check_vector(const struct r600_bytecode *bc, const struct r600_bytecode_alu *alu,
			struct alu_bank_swizzle *bs, int bank_swizzle)
{
	int r, src, num_src, sel, elem, cycle;

	num_src = r600_bytecode_get_num_operands(alu);
	for (src = 0; src < num_src; src++) {
		sel = alu->src[src].sel;
		elem = alu->src[src].chan;
		if (is_gpr(sel)) {
			cycle = cycle_for_bank_swizzle_vec[bank_swizzle][src];
			if (src == 1 && sel == alu->src[0].sel && elem == alu->src[0].chan)
				/* Nothing to do; special-case optimization,
				 * second source uses first source’s reservation. */
				continue;
			else {
				r = reserve_gpr(bs, sel, elem, cycle);
				if (r)
					return r;
			}
      } else if (is_kcache(sel)) {
			r = reserve_cfile(bc, bs, (alu->src[src].kc_bank<<16) + sel, elem);
			if (r)
				return r;
		}
		/* No restrictions on PV, PS, literal or special constants. */
	}
	return 0;
}

static int check_scalar(const struct r600_bytecode *bc, const struct r600_bytecode_alu *alu,
			struct alu_bank_swizzle *bs, int bank_swizzle)
{
	int r, src, num_src, const_count, sel, elem, cycle;

	num_src = r600_bytecode_get_num_operands(alu);
	for (const_count = 0, src = 0; src < num_src; ++src) {
		sel = alu->src[src].sel;
		elem = alu->src[src].chan;
		if (is_const(sel)) { /* Any constant, including literal and inline constants. */
			if (const_count >= 2)
				/* More than two references to a constant in
				 * transcendental operation. */
				return -1;
			else
				const_count++;
		}
      if (is_kcache(sel)) {
			r = reserve_cfile(bc, bs, (alu->src[src].kc_bank<<16) + sel, elem);
			if (r)
				return r;
		}
	}
	for (src = 0; src < num_src; ++src) {
		sel = alu->src[src].sel;
		elem = alu->src[src].chan;
		if (is_gpr(sel)) {
			cycle = cycle_for_bank_swizzle_scl[bank_swizzle][src];
			if (cycle < const_count)
				/* Cycle for GPR load conflicts with
				 * constant load in transcendental operation. */
				return -1;
			r = reserve_gpr(bs, sel, elem, cycle);
			if (r)
				return r;
		}
		/* PV PS restrictions */
		if (const_count && (sel == 254 || sel == 255)) {
			cycle = cycle_for_bank_swizzle_scl[bank_swizzle][src];
			if (cycle < const_count)
				return -1;
		}
	}
	return 0;
}

static int check_and_set_bank_swizzle(const struct r600_bytecode *bc,
				      struct r600_bytecode_alu *slots[5])
{
	struct alu_bank_swizzle bs;
	int bank_swizzle[5];
	int i, r = 0, forced = 1;
	bool scalar_only = bc->gfx_level == CAYMAN ? false : true;
	int max_slots = bc->gfx_level == CAYMAN ? 4 : 5;
	int max_checks = max_slots * 1000;

	for (i = 0; i < max_slots; i++) {
		if (slots[i]) {
			if (slots[i]->bank_swizzle_force) {
				slots[i]->bank_swizzle = slots[i]->bank_swizzle_force;
			} else {
				forced = 0;
			}
		}

		if (i < 4 && slots[i])
			scalar_only = false;
	}
	if (forced)
		return 0;

	/* Just check every possible combination of bank swizzle.
	 * Not very efficient, but works on the first try in most of the cases. */
	for (i = 0; i < 4; i++)
		if (!slots[i] || !slots[i]->bank_swizzle_force || slots[i]->is_lds_idx_op)
			bank_swizzle[i] = SQ_ALU_VEC_012;
		else
			bank_swizzle[i] = slots[i]->bank_swizzle;

	bank_swizzle[4] = SQ_ALU_SCL_210;

	while(bank_swizzle[4] <= SQ_ALU_SCL_221 && max_checks--) {
		init_bank_swizzle(&bs);
		if (scalar_only == false) {
			for (i = 0; i < 4; i++) {
				if (slots[i]) {
					r = check_vector(bc, slots[i], &bs, bank_swizzle[i]);
					if (r)
						break;
				}
			}
		} else
			r = 0;

		if (!r && max_slots == 5 && slots[4]) {
			r = check_scalar(bc, slots[4], &bs, bank_swizzle[4]);
		}
		if (!r) {
			for (i = 0; i < max_slots; i++) {
				if (slots[i])
					slots[i]->bank_swizzle = bank_swizzle[i];
			}
			return 0;
		}

		if (scalar_only) {
			bank_swizzle[4]++;
		} else {
			for (i = 0; i < max_slots; i++) {
				if (!slots[i] || (!slots[i]->bank_swizzle_force && !slots[i]->is_lds_idx_op)) {
					bank_swizzle[i]++;
					if (bank_swizzle[i] <= SQ_ALU_VEC_210)
						break;
					else if (i < max_slots - 1)
						bank_swizzle[i] = SQ_ALU_VEC_012;
					else
						return -1;
				}
			}
		}
	}

	/* Couldn't find a working swizzle. */
	return -1;
}

static int replace_gpr_with_pv_ps(struct r600_bytecode *bc,
				  struct r600_bytecode_alu *slots[5], struct r600_bytecode_alu *alu_prev)
{
	struct r600_bytecode_alu *prev[5];
	int gpr[5], chan[5];
	int i, j, r, src, num_src;
	int max_slots = bc->gfx_level == CAYMAN ? 4 : 5;

	r = assign_alu_units(bc, alu_prev, prev);
	if (r)
		return r;

	for (i = 0; i < max_slots; ++i) {
		if (prev[i] && alu_writes(prev[i]) && !prev[i]->dst.rel) {

			if (is_alu_64bit_inst(prev[i])) {
				gpr[i] = -1;
				continue;
			}

			gpr[i] = prev[i]->dst.sel;
			/* cube writes more than PV.X */
			if (is_alu_reduction_inst(bc, prev[i]))
				chan[i] = 0;
			else
				chan[i] = prev[i]->dst.chan;
		} else
			gpr[i] = -1;
	}

	for (i = 0; i < max_slots; ++i) {
		struct r600_bytecode_alu *alu = slots[i];
		if (!alu)
			continue;

		if (is_alu_64bit_inst(alu))
			continue;
		num_src = r600_bytecode_get_num_operands(alu);
		for (src = 0; src < num_src; ++src) {
			if (!is_gpr(alu->src[src].sel) || alu->src[src].rel)
				continue;

			if (bc->gfx_level < CAYMAN) {
				if (alu->src[src].sel == gpr[4] &&
				    alu->src[src].chan == chan[4] &&
				    alu_prev->pred_sel == alu->pred_sel) {
					alu->src[src].sel = V_SQ_ALU_SRC_PS;
					alu->src[src].chan = 0;
					continue;
				}
			}

			for (j = 0; j < 4; ++j) {
				if (alu->src[src].sel == gpr[j] &&
					alu->src[src].chan == j &&
				      alu_prev->pred_sel == alu->pred_sel) {
					alu->src[src].sel = V_SQ_ALU_SRC_PV;
					alu->src[src].chan = chan[j];
					break;
				}
			}
		}
	}

	return 0;
}

void r600_bytecode_special_constants(uint32_t value, unsigned *sel)
{
	switch(value) {
	case 0:
		*sel = V_SQ_ALU_SRC_0;
		break;
	case 1:
		*sel = V_SQ_ALU_SRC_1_INT;
		break;
	case -1:
		*sel = V_SQ_ALU_SRC_M_1_INT;
		break;
	case 0x3F800000: /* 1.0f */
		*sel = V_SQ_ALU_SRC_1;
		break;
	case 0x3F000000: /* 0.5f */
		*sel = V_SQ_ALU_SRC_0_5;
		break;
	default:
		*sel = V_SQ_ALU_SRC_LITERAL;
		break;
	}
}

/* compute how many literal are needed */
static int r600_bytecode_alu_nliterals(struct r600_bytecode_alu *alu,
				 uint32_t literal[4], unsigned *nliteral)
{
	unsigned num_src = r600_bytecode_get_num_operands(alu);
	unsigned i, j;

	for (i = 0; i < num_src; ++i) {
		if (alu->src[i].sel == V_SQ_ALU_SRC_LITERAL) {
			uint32_t value = alu->src[i].value;
			unsigned found = 0;
			for (j = 0; j < *nliteral; ++j) {
				if (literal[j] == value) {
					found = 1;
					break;
				}
			}
			if (!found) {
				if (*nliteral >= 4)
					return -EINVAL;
				literal[(*nliteral)++] = value;
			}
		}
	}
	return 0;
}

static void r600_bytecode_alu_adjust_literals(struct r600_bytecode_alu *alu,
					      uint32_t literal[4], unsigned nliteral)
{
	unsigned num_src = r600_bytecode_get_num_operands(alu);
	unsigned i, j;

	for (i = 0; i < num_src; ++i) {
		if (alu->src[i].sel == V_SQ_ALU_SRC_LITERAL) {
			uint32_t value = alu->src[i].value;
			for (j = 0; j < nliteral; ++j) {
				if (literal[j] == value) {
					alu->src[i].chan = j;
					break;
				}
			}
		}
	}
}

static int merge_inst_groups(struct r600_bytecode *bc, struct r600_bytecode_alu *slots[5],
			     struct r600_bytecode_alu *alu_prev)
{
	struct r600_bytecode_alu *prev[5];
	struct r600_bytecode_alu *result[5] = { NULL };

        uint8_t interp_xz = 0;

	uint32_t literal[4], prev_literal[4];
	unsigned nliteral = 0, prev_nliteral = 0;

	int i, j, r, src, num_src;
	int num_once_inst = 0;
	int have_mova = 0, have_rel = 0;
	int max_slots = bc->gfx_level == CAYMAN ? 4 : 5;

   bool has_dot = false;

	r = assign_alu_units(bc, alu_prev, prev);
	if (r)
		return r;

	for (i = 0; i < max_slots; ++i) {
		if (prev[i]) {
		      if (prev[i]->pred_sel)
			      return 0;
		      if (is_alu_once_inst(prev[i]))
			      return 0;
				has_dot |= prev[i]->op == ALU_OP2_DOT || prev[i]->op == ALU_OP2_DOT_IEEE;


                      if (prev[i]->op == ALU_OP1_INTERP_LOAD_P0)
                         interp_xz |= 3;
                      if (prev[i]->op == ALU_OP2_INTERP_X)
                         interp_xz |= 1;
                      if (prev[i]->op == ALU_OP2_INTERP_Z)
                         interp_xz |= 2;
		}
		if (slots[i]) {
			if (slots[i]->pred_sel)
				return 0;
			if (is_alu_once_inst(slots[i]))
				return 0;
         has_dot |= slots[i]->op == ALU_OP2_DOT || slots[i]->op == ALU_OP2_DOT_IEEE;
				return 0;
                        if (slots[i]->op == ALU_OP1_INTERP_LOAD_P0)
                           interp_xz |= 3;
                        if (slots[i]->op == ALU_OP2_INTERP_X)
                           interp_xz |= 1;
                        if (slots[i]->op == ALU_OP2_INTERP_Z)
                           interp_xz |= 2;
		}
                if (interp_xz == 3)
                   return 0;
	}

	for (i = 0; i < max_slots; ++i) {
		struct r600_bytecode_alu *alu;

		if (num_once_inst > 0)
		   return 0;

		/* check number of literals */
		if (prev[i]) {
			if (r600_bytecode_alu_nliterals(prev[i], literal, &nliteral))
				return 0;
			if (r600_bytecode_alu_nliterals(prev[i], prev_literal, &prev_nliteral))
				return 0;
			if (is_alu_mova_inst(prev[i])) {
				if (have_rel)
					return 0;
				have_mova = 1;
			}

			if (alu_uses_rel(prev[i])) {
				if (have_mova) {
					return 0;
				}
				have_rel = 1;
			}
			if (alu_uses_lds(prev[i]))
				return 0;

			num_once_inst += is_alu_once_inst(prev[i]);
		}
		if (slots[i] && r600_bytecode_alu_nliterals(slots[i], literal, &nliteral))
			return 0;

		/* Let's check used slots. */
		if (prev[i] && !slots[i]) {
			result[i] = prev[i];
			continue;
		} else if (prev[i] && slots[i]) {
			if (max_slots == 5 && !has_dot && result[4] == NULL && prev[4] == NULL && slots[4] == NULL) {
				/* Trans unit is still free try to use it. */
				if (is_alu_any_unit_inst(bc, slots[i]) && !alu_uses_lds(slots[i])) {
					result[i] = prev[i];
					result[4] = slots[i];
				} else if (is_alu_any_unit_inst(bc, prev[i])) {
					if (slots[i]->dst.sel == prev[i]->dst.sel &&
					    alu_writes(slots[i]) &&
					    alu_writes(prev[i]))
						return 0;

					result[i] = slots[i];
					result[4] = prev[i];
				} else
					return 0;
			} else
				return 0;
		} else if(!slots[i]) {
			continue;
		} else {
			if (max_slots == 5 && slots[i] && prev[4] &&
					slots[i]->dst.sel == prev[4]->dst.sel &&
					slots[i]->dst.chan == prev[4]->dst.chan &&
					alu_writes(slots[i]) &&
					alu_writes(prev[4]))
				return 0;

			result[i] = slots[i];
		}

		alu = slots[i];
		num_once_inst += is_alu_once_inst(alu);

		/* don't reschedule NOPs */
		if (is_nop_inst(alu))
			return 0;

		if (is_alu_mova_inst(alu)) {
			if (have_rel) {
				return 0;
			}
			have_mova = 1;
		}

		if (alu_uses_rel(alu)) {
			if (have_mova) {
				return 0;
			}
			have_rel = 1;
		}

		if (alu->op == ALU_OP0_SET_CF_IDX0 ||
			alu->op == ALU_OP0_SET_CF_IDX1)
			return 0; /* data hazard with MOVA */

		/* Let's check source gprs */
		num_src = r600_bytecode_get_num_operands(alu);
		for (src = 0; src < num_src; ++src) {

			/* Constants don't matter. */
			if (!is_gpr(alu->src[src].sel))
				continue;

			for (j = 0; j < max_slots; ++j) {
				if (!prev[j] || !alu_writes(prev[j]))
					continue;

				/* If it's relative then we can't determine which gpr is really used. */
				if (prev[j]->dst.chan == alu->src[src].chan &&
					(prev[j]->dst.sel == alu->src[src].sel ||
					prev[j]->dst.rel || alu->src[src].rel))
					return 0;
			}
		}
	}

	/* more than one PRED_ or KILL_ ? */
	if (num_once_inst > 1)
		return 0;

	/* check if the result can still be swizzlet */
	r = check_and_set_bank_swizzle(bc, result);
	if (r)
		return 0;

	/* looks like everything worked out right, apply the changes */

	/* undo adding previous literals */
	bc->cf_last->ndw -= align(prev_nliteral, 2);

	/* sort instructions */
	for (i = 0; i < max_slots; ++i) {
		slots[i] = result[i];
		if (result[i]) {
			list_del(&result[i]->list);
			result[i]->last = 0;
			list_addtail(&result[i]->list, &bc->cf_last->alu);
		}
	}

	/* determine new last instruction */
	list_entry(bc->cf_last->alu.prev, struct r600_bytecode_alu, list)->last = 1;

	/* determine new first instruction */
	for (i = 0; i < max_slots; ++i) {
		if (result[i]) {
			bc->cf_last->curr_bs_head = result[i];
			break;
		}
	}

	bc->cf_last->prev_bs_head = bc->cf_last->prev2_bs_head;
	bc->cf_last->prev2_bs_head = NULL;

	return 0;
}

/* we'll keep kcache sets sorted by bank & addr */
static int r600_bytecode_alloc_kcache_line(struct r600_bytecode *bc,
		struct r600_bytecode_kcache *kcache,
		unsigned bank, unsigned line, unsigned index_mode)
{
	int i, kcache_banks = bc->gfx_level >= EVERGREEN ? 4 : 2;

	for (i = 0; i < kcache_banks; i++) {
		if (kcache[i].mode) {
			int d;

			if (kcache[i].bank < bank)
				continue;

			if ((kcache[i].bank == bank && kcache[i].addr > line+1) ||
					kcache[i].bank > bank) {
				/* try to insert new line */
				if (kcache[kcache_banks-1].mode) {
					/* all sets are in use */
					return -ENOMEM;
				}

				memmove(&kcache[i+1],&kcache[i], (kcache_banks-i-1)*sizeof(struct r600_bytecode_kcache));
				kcache[i].mode = V_SQ_CF_KCACHE_LOCK_1;
				kcache[i].bank = bank;
				kcache[i].addr = line;
				kcache[i].index_mode = index_mode;
				return 0;
			}

			d = line - kcache[i].addr;

			if (d == -1) {
				kcache[i].addr--;
				if (kcache[i].mode == V_SQ_CF_KCACHE_LOCK_2) {
					/* we are prepending the line to the current set,
					 * discarding the existing second line,
					 * so we'll have to insert line+2 after it */
					line += 2;
					continue;
				} else if (kcache[i].mode == V_SQ_CF_KCACHE_LOCK_1) {
					kcache[i].mode = V_SQ_CF_KCACHE_LOCK_2;
					return 0;
				} else {
					/* V_SQ_CF_KCACHE_LOCK_LOOP_INDEX is not supported */
					return -ENOMEM;
				}
			} else if (d == 1) {
				kcache[i].mode = V_SQ_CF_KCACHE_LOCK_2;
				return 0;
			} else if (d == 0)
				return 0;
		} else { /* free kcache set - use it */
			kcache[i].mode = V_SQ_CF_KCACHE_LOCK_1;
			kcache[i].bank = bank;
			kcache[i].addr = line;
			kcache[i].index_mode = index_mode;
			return 0;
		}
	}
	return -ENOMEM;
}

static int r600_bytecode_alloc_inst_kcache_lines(struct r600_bytecode *bc,
		struct r600_bytecode_kcache *kcache,
		struct r600_bytecode_alu *alu)
{
	int i, r;

	for (i = 0; i < 3; i++) {
		unsigned bank, line, sel = alu->src[i].sel, index_mode;

		if (sel < 512)
			continue;

		bank = alu->src[i].kc_bank;
		assert(bank < R600_MAX_ALU_CONST_BUFFERS);
		line = (sel-512)>>4;
		index_mode = alu->src[i].kc_rel;

		if ((r = r600_bytecode_alloc_kcache_line(bc, kcache, bank, line, index_mode)))
			return r;
	}
	return 0;
}

static int r600_bytecode_assign_kcache_banks(
		struct r600_bytecode_alu *alu,
		struct r600_bytecode_kcache * kcache)
{
	int i, j;

	/* Alter the src operands to refer to the kcache. */
	for (i = 0; i < 3; ++i) {
		static const unsigned int base[] = {128, 160, 256, 288};
		unsigned int line, sel = alu->src[i].sel, found = 0;

		if (sel < 512)
			continue;

		sel -= 512;
		line = sel>>4;

		for (j = 0; j < 4 && !found; ++j) {
			switch (kcache[j].mode) {
			case V_SQ_CF_KCACHE_NOP:
			case V_SQ_CF_KCACHE_LOCK_LOOP_INDEX:
				R600_ASM_ERR("unexpected kcache line mode\n");
				return -ENOMEM;
			default:
				if (kcache[j].bank == alu->src[i].kc_bank &&
						kcache[j].addr <= line &&
						line < kcache[j].addr + kcache[j].mode) {
					alu->src[i].sel = sel - (kcache[j].addr<<4);
					alu->src[i].sel += base[j];
					found=1;
			    }
			}
		}
	}
	return 0;
}

static int r600_bytecode_alloc_kcache_lines(struct r600_bytecode *bc,
		struct r600_bytecode_alu *alu,
		unsigned type)
{
	struct r600_bytecode_kcache kcache_sets[4];
	struct r600_bytecode_kcache *kcache = kcache_sets;
	int r;

	memcpy(kcache, bc->cf_last->kcache, 4 * sizeof(struct r600_bytecode_kcache));

	if ((r = r600_bytecode_alloc_inst_kcache_lines(bc, kcache, alu))) {
		/* can't alloc, need to start new clause */

		/* Make sure the CF ends with an "last" instruction when
		 * we split an ALU group because of a new CF */
		if (!list_is_empty(&bc->cf_last->alu))  {
			struct r600_bytecode_alu *last_submitted =
				list_last_entry(&bc->cf_last->alu, struct r600_bytecode_alu, list);
				last_submitted->last = 1;
		}

		if ((r = r600_bytecode_add_cf(bc))) {
			return r;
		}
		bc->cf_last->op = type;

		/* retry with the new clause */
		kcache = bc->cf_last->kcache;
		if ((r = r600_bytecode_alloc_inst_kcache_lines(bc, kcache, alu))) {
			/* can't alloc again- should never happen */
			return r;
		}
	} else {
		/* update kcache sets */
		memcpy(bc->cf_last->kcache, kcache, 4 * sizeof(struct r600_bytecode_kcache));
	}

	/* if we actually used more than 2 kcache sets, or have relative indexing - use ALU_EXTENDED on eg+ */
	if (kcache[2].mode != V_SQ_CF_KCACHE_NOP ||
		kcache[0].index_mode || kcache[1].index_mode || kcache[2].index_mode || kcache[3].index_mode) {
		if (bc->gfx_level < EVERGREEN)
			return -ENOMEM;
		bc->cf_last->eg_alu_extended = 1;
	}

	return 0;
}

static int insert_nop_r6xx(struct r600_bytecode *bc, int max_slots)
{
	struct r600_bytecode_alu alu;
	int r, i;

	for (i = 0; i < max_slots; i++) {
		memset(&alu, 0, sizeof(alu));
		alu.op = ALU_OP0_NOP;
		alu.src[0].chan = i & 3;
		alu.dst.chan = i & 3;
		alu.last = (i == max_slots - 1);
		r = r600_bytecode_add_alu(bc, &alu);
		if (r)
			return r;
	}
	return 0;
}

/* load AR register from gpr (bc->ar_reg) with MOVA_INT */
static int load_ar_r6xx(struct r600_bytecode *bc, bool for_src)
{
	struct r600_bytecode_alu alu;
	int r;

	if (bc->ar_loaded)
		return 0;

	/* hack to avoid making MOVA the last instruction in the clause */
	if (bc->cf_last == NULL || (bc->cf_last->ndw>>1) >= 110)
		bc->force_add_cf = 1;
   else if (for_src) {
      insert_nop_r6xx(bc, 4);
      bc->nalu_groups++;
   }

	memset(&alu, 0, sizeof(alu));
	alu.op = ALU_OP1_MOVA_GPR_INT;
	alu.src[0].sel = bc->ar_reg;
	alu.src[0].chan = bc->ar_chan;
	alu.last = 1;
	alu.index_mode = INDEX_MODE_LOOP;
	r = r600_bytecode_add_alu(bc, &alu);
	if (r)
		return r;

	/* no requirement to set uses waterfall on MOVA_GPR_INT */
	bc->ar_loaded = 1;
	return 0;
}

/* load AR register from gpr (bc->ar_reg) with MOVA_INT */
int r600_load_ar(struct r600_bytecode *bc, bool for_src)
{
	struct r600_bytecode_alu alu;
	int r;

	if (bc->ar_handling)
		return load_ar_r6xx(bc, for_src);

	if (bc->ar_loaded)
		return 0;

	/* hack to avoid making MOVA the last instruction in the clause */
	if (bc->cf_last == NULL || (bc->cf_last->ndw>>1) >= 110)
		bc->force_add_cf = 1;

	memset(&alu, 0, sizeof(alu));
	alu.op = ALU_OP1_MOVA_INT;
	alu.src[0].sel = bc->ar_reg;
	alu.src[0].chan = bc->ar_chan;
	alu.last = 1;
	r = r600_bytecode_add_alu(bc, &alu);
	if (r)
		return r;

	bc->cf_last->r6xx_uses_waterfall = 1;
	bc->ar_loaded = 1;
	return 0;
}

int r600_bytecode_add_alu_type(struct r600_bytecode *bc,
		const struct r600_bytecode_alu *alu, unsigned type)
{
	struct r600_bytecode_alu *nalu = r600_bytecode_alu();
	struct r600_bytecode_alu *lalu;
	int i, r;

	if (!nalu)
		return -ENOMEM;
	memcpy(nalu, alu, sizeof(struct r600_bytecode_alu));

	if (alu->is_op3) {
		/* will fail later since alu does not support it. */
		assert(!alu->src[0].abs && !alu->src[1].abs && !alu->src[2].abs);
	}

	if (bc->cf_last != NULL && bc->cf_last->op != type) {
		/* check if we could add it anyway */
		if ((bc->cf_last->op == CF_OP_ALU && type == CF_OP_ALU_PUSH_BEFORE) ||
		 	(bc->cf_last->op == CF_OP_ALU_PUSH_BEFORE && type == CF_OP_ALU)) {
		 	LIST_FOR_EACH_ENTRY(lalu, &bc->cf_last->alu, list) {
		 		if (lalu->execute_mask) {
                                        assert(bc->force_add_cf || !"no force cf");
					bc->force_add_cf = 1;
					break;
				}
		 		type = CF_OP_ALU_PUSH_BEFORE;
			}
		} else  {
                   assert(bc->force_add_cf ||!"no force cf");
			bc->force_add_cf = 1;
                }
	}

	/* cf can contains only alu or only vtx or only tex */
	if (bc->cf_last == NULL || bc->force_add_cf) {
               if (bc->cf_last && bc->cf_last->curr_bs_head)
                  bc->cf_last->curr_bs_head->last = 1;
		r = r600_bytecode_add_cf(bc);
		if (r) {
			free(nalu);
			return r;
		}
	}
	bc->cf_last->op = type;

	if (bc->gfx_level >= EVERGREEN) {
		for (i = 0; i < 3; i++)
			if (nalu->src[i].kc_bank &&  nalu->src[i].kc_rel)
				assert(bc->index_loaded[nalu->src[i].kc_rel - 1]);
	}

	/* Check AR usage and load it if required */
	for (i = 0; i < 3; i++)
		if (nalu->src[i].rel && !bc->ar_loaded)
			r600_load_ar(bc, true);

	if (nalu->dst.rel && !bc->ar_loaded)
		r600_load_ar(bc, false);

	/* Setup the kcache for this ALU instruction. This will start a new
	 * ALU clause if needed. */
	if ((r = r600_bytecode_alloc_kcache_lines(bc, nalu, type))) {
		free(nalu);
		return r;
	}

	if (!bc->cf_last->curr_bs_head) {
		bc->cf_last->curr_bs_head = nalu;
	}
	/* number of gpr == the last gpr used in any alu */
	for (i = 0; i < 3; i++) {
		if (nalu->src[i].sel >= bc->ngpr && nalu->src[i].sel < 123) {
			bc->ngpr = nalu->src[i].sel + 1;
		}
		if (nalu->src[i].sel == V_SQ_ALU_SRC_LITERAL)
			r600_bytecode_special_constants(nalu->src[i].value,
				&nalu->src[i].sel);
	}
	if (nalu->dst.write && nalu->dst.sel >= bc->ngpr && nalu->dst.sel < 123) {
		bc->ngpr = nalu->dst.sel + 1;
	}
	list_addtail(&nalu->list, &bc->cf_last->alu);
	/* each alu use 2 dwords */
	bc->cf_last->ndw += 2;
	bc->ndw += 2;

	/* process cur ALU instructions for bank swizzle */
	if (nalu->last) {
		uint32_t literal[4];
		unsigned nliteral;
		struct r600_bytecode_alu *slots[5];
		int max_slots = bc->gfx_level == CAYMAN ? 4 : 5;
		r = assign_alu_units(bc, bc->cf_last->curr_bs_head, slots);
		if (r)
			return r;

		if (bc->cf_last->prev_bs_head) {
         struct r600_bytecode_alu *cur_prev_head = bc->cf_last->prev_bs_head;
			r = merge_inst_groups(bc, slots, cur_prev_head);
			if (r)
				return r;
         if (cur_prev_head != bc->cf_last->prev_bs_head)
            bc->nalu_groups--;
		}

		if (bc->cf_last->prev_bs_head) {
			r = replace_gpr_with_pv_ps(bc, slots, bc->cf_last->prev_bs_head);
			if (r)
				return r;
		}

		r = check_and_set_bank_swizzle(bc, slots);
		if (r)
			return r;

		for (i = 0, nliteral = 0; i < max_slots; i++) {
			if (slots[i]) {
				r = r600_bytecode_alu_nliterals(slots[i], literal, &nliteral);
				if (r)
					return r;
			}
		}
		bc->cf_last->ndw += align(nliteral, 2);

		bc->cf_last->prev2_bs_head = bc->cf_last->prev_bs_head;
		bc->cf_last->prev_bs_head = bc->cf_last->curr_bs_head;
		bc->cf_last->curr_bs_head = NULL;

		bc->nalu_groups++;

		if (bc->r6xx_nop_after_rel_dst) {
			for (int i = 0; i < max_slots; ++i) {
				if (slots[i] && slots[i]->dst.rel) {
					insert_nop_r6xx(bc, max_slots);
					bc->nalu_groups++;
					break;
				}
			}
		}
	}

	/* Might need to insert spill write ops after current clause */
	if (nalu->last && bc->n_pending_outputs) {
		while (bc->n_pending_outputs) {
			r = r600_bytecode_add_output(bc, &bc->pending_outputs[--bc->n_pending_outputs]);
			if (r)
				return r;
		}
	}

	return 0;
}

int r600_bytecode_add_alu(struct r600_bytecode *bc, const struct r600_bytecode_alu *alu)
{
	return r600_bytecode_add_alu_type(bc, alu, CF_OP_ALU);
}

static unsigned r600_bytecode_num_tex_and_vtx_instructions(const struct r600_bytecode *bc)
{
	switch (bc->gfx_level) {
	case R600:
		return 8;

	case R700:
	case EVERGREEN:
	case CAYMAN:
		return 16;

	default:
		R600_ASM_ERR("Unknown gfx level %d.\n", bc->gfx_level);
		return 8;
	}
}

static inline bool last_inst_was_not_vtx_fetch(struct r600_bytecode *bc, bool use_tc)
{
	return !((r600_isa_cf(bc->cf_last->op)->flags & CF_FETCH) &&
		 bc->cf_last->op != CF_OP_GDS &&
		 (bc->gfx_level == CAYMAN || use_tc ||
		  bc->cf_last->op != CF_OP_TEX));
}

static int r600_bytecode_add_vtx_internal(struct r600_bytecode *bc, const struct r600_bytecode_vtx *vtx,
					  bool use_tc)
{
	struct r600_bytecode_vtx *nvtx = r600_bytecode_vtx();
	int r;

	if (!nvtx)
		return -ENOMEM;
	memcpy(nvtx, vtx, sizeof(struct r600_bytecode_vtx));

	if (bc->gfx_level >= EVERGREEN) {
		assert(!vtx->buffer_index_mode ||
		       bc->index_loaded[vtx->buffer_index_mode - 1]);
	}


	/* cf can contains only alu or only vtx or only tex */
	if (bc->cf_last == NULL ||
	    last_inst_was_not_vtx_fetch(bc, use_tc) ||
	    bc->force_add_cf) {
		r = r600_bytecode_add_cf(bc);
		if (r) {
			free(nvtx);
			return r;
		}
		switch (bc->gfx_level) {
		case R600:
		case R700:
			bc->cf_last->op = CF_OP_VTX;
			break;
		case EVERGREEN:
			if (use_tc)
				bc->cf_last->op = CF_OP_TEX;
			else
				bc->cf_last->op = CF_OP_VTX;
			break;
		case CAYMAN:
			bc->cf_last->op = CF_OP_TEX;
			break;
		default:
			R600_ASM_ERR("Unknown gfx level %d.\n", bc->gfx_level);
			free(nvtx);
			return -EINVAL;
		}
	}
	list_addtail(&nvtx->list, &bc->cf_last->vtx);
	/* each fetch use 4 dwords */
	bc->cf_last->ndw += 4;
	bc->ndw += 4;
	if ((bc->cf_last->ndw / 4) >= r600_bytecode_num_tex_and_vtx_instructions(bc))
		bc->force_add_cf = 1;

	bc->ngpr = MAX2(bc->ngpr, vtx->src_gpr + 1);
	bc->ngpr = MAX2(bc->ngpr, vtx->dst_gpr + 1);

	return 0;
}

int r600_bytecode_add_vtx(struct r600_bytecode *bc, const struct r600_bytecode_vtx *vtx)
{
	return r600_bytecode_add_vtx_internal(bc, vtx, false);
}

int r600_bytecode_add_vtx_tc(struct r600_bytecode *bc, const struct r600_bytecode_vtx *vtx)
{
	return r600_bytecode_add_vtx_internal(bc, vtx, true);
}

int r600_bytecode_add_tex(struct r600_bytecode *bc, const struct r600_bytecode_tex *tex)
{
	struct r600_bytecode_tex *ntex = r600_bytecode_tex();
	int r;

	if (!ntex)
		return -ENOMEM;
	memcpy(ntex, tex, sizeof(struct r600_bytecode_tex));

	if (bc->gfx_level >= EVERGREEN) {
		assert(!tex->sampler_index_mode ||
		       bc->index_loaded[tex->sampler_index_mode - 1]);
		assert(!tex->resource_index_mode ||
                       bc->index_loaded[tex->resource_index_mode - 1]);
	}

	/* we can't fetch data und use it as texture lookup address in the same TEX clause */
	if (bc->cf_last != NULL &&
		bc->cf_last->op == CF_OP_TEX) {
		struct r600_bytecode_tex *ttex;
                uint8_t use_mask = ((1 << ntex->src_sel_x) |
                                    (1 << ntex->src_sel_y) |
                                    (1 << ntex->src_sel_z) |
                                    (1 << ntex->src_sel_w)) & 0xf;

		LIST_FOR_EACH_ENTRY(ttex, &bc->cf_last->tex, list) {
			if (ttex->dst_gpr == ntex->src_gpr) {
                           uint8_t write_mask = (ttex->dst_sel_x < 6 ? 1 : 0) |
                                                (ttex->dst_sel_y < 6 ? 2 : 0) |
                                                (ttex->dst_sel_z < 6 ? 4 : 0) |
                                                (ttex->dst_sel_w < 6 ? 8 : 0);
                           if (use_mask & write_mask) {
                              bc->force_add_cf = 1;
                              break;
                           }
			}
		}
		/* vtx instrs get inserted after tex, so make sure we aren't moving the tex
		 * before (say) the instr fetching the texcoord.
		 */
		if (!list_is_empty(&bc->cf_last->vtx))
			bc->force_add_cf = 1;

		/* slight hack to make gradients always go into same cf */
		if (ntex->op == FETCH_OP_SET_GRADIENTS_H)
			bc->force_add_cf = 1;
	}

	/* cf can contains only alu or only vtx or only tex */
	if (bc->cf_last == NULL ||
		bc->cf_last->op != CF_OP_TEX ||
	        bc->force_add_cf) {
		r = r600_bytecode_add_cf(bc);
		if (r) {
			free(ntex);
			return r;
		}
		bc->cf_last->op = CF_OP_TEX;
	}
	if (ntex->src_gpr >= bc->ngpr) {
		bc->ngpr = ntex->src_gpr + 1;
	}
	if (ntex->dst_gpr >= bc->ngpr) {
		bc->ngpr = ntex->dst_gpr + 1;
	}
	list_addtail(&ntex->list, &bc->cf_last->tex);
	/* each texture fetch use 4 dwords */
	bc->cf_last->ndw += 4;
	bc->ndw += 4;
	if ((bc->cf_last->ndw / 4) >= r600_bytecode_num_tex_and_vtx_instructions(bc))
		bc->force_add_cf = 1;
	return 0;
}

int r600_bytecode_add_gds(struct r600_bytecode *bc, const struct r600_bytecode_gds *gds)
{
	struct r600_bytecode_gds *ngds = r600_bytecode_gds();
	int r;

	if (ngds == NULL)
		return -ENOMEM;
	memcpy(ngds, gds, sizeof(struct r600_bytecode_gds));

	if (bc->gfx_level >= EVERGREEN) {
		assert(!gds->uav_index_mode ||
		       bc->index_loaded[gds->uav_index_mode - 1]);
	}

	if (bc->cf_last == NULL ||
	    bc->cf_last->op != CF_OP_GDS ||
	    bc->force_add_cf) {
		r = r600_bytecode_add_cf(bc);
		if (r) {
			free(ngds);
			return r;
		}
		bc->cf_last->op = CF_OP_GDS;
	}

	list_addtail(&ngds->list, &bc->cf_last->gds);
	bc->cf_last->ndw += 4; /* each GDS uses 4 dwords */
	if ((bc->cf_last->ndw / 4) >= r600_bytecode_num_tex_and_vtx_instructions(bc))
		bc->force_add_cf = 1;
	return 0;
}

int r600_bytecode_add_cfinst(struct r600_bytecode *bc, unsigned op)
{
	int r;

	/* Emit WAIT_ACK before control flow to ensure pending writes are always acked. */
	if (op != CF_OP_WAIT_ACK && op != CF_OP_MEM_SCRATCH)
		r600_bytecode_wait_acks(bc);

	r = r600_bytecode_add_cf(bc);
	if (r)
		return r;

	bc->cf_last->cond = V_SQ_CF_COND_ACTIVE;
	bc->cf_last->op = op;
	return 0;
}

int cm_bytecode_add_cf_end(struct r600_bytecode *bc)
{
	return r600_bytecode_add_cfinst(bc, CF_OP_CF_END);
}

/* common to all 3 families */
static int r600_bytecode_vtx_build(struct r600_bytecode *bc, struct r600_bytecode_vtx *vtx, unsigned id)
{
	if (r600_isa_fetch(vtx->op)->flags & FF_MEM)
		return r700_bytecode_fetch_mem_build(bc, vtx, id);
	bc->bytecode[id] = S_SQ_VTX_WORD0_VTX_INST(r600_isa_fetch_opcode(bc->isa->hw_class, vtx->op)) |
			S_SQ_VTX_WORD0_BUFFER_ID(vtx->buffer_id) |
			S_SQ_VTX_WORD0_FETCH_TYPE(vtx->fetch_type) |
			S_SQ_VTX_WORD0_SRC_GPR(vtx->src_gpr) |
			S_SQ_VTX_WORD0_SRC_SEL_X(vtx->src_sel_x);
	if (bc->gfx_level < CAYMAN)
		bc->bytecode[id] |= S_SQ_VTX_WORD0_MEGA_FETCH_COUNT(vtx->mega_fetch_count);
	id++;
	bc->bytecode[id++] = S_SQ_VTX_WORD1_DST_SEL_X(vtx->dst_sel_x) |
				S_SQ_VTX_WORD1_DST_SEL_Y(vtx->dst_sel_y) |
				S_SQ_VTX_WORD1_DST_SEL_Z(vtx->dst_sel_z) |
				S_SQ_VTX_WORD1_DST_SEL_W(vtx->dst_sel_w) |
				S_SQ_VTX_WORD1_USE_CONST_FIELDS(vtx->use_const_fields) |
				S_SQ_VTX_WORD1_DATA_FORMAT(vtx->data_format) |
				S_SQ_VTX_WORD1_NUM_FORMAT_ALL(vtx->num_format_all) |
				S_SQ_VTX_WORD1_FORMAT_COMP_ALL(vtx->format_comp_all) |
				S_SQ_VTX_WORD1_SRF_MODE_ALL(vtx->srf_mode_all) |
				S_SQ_VTX_WORD1_GPR_DST_GPR(vtx->dst_gpr);
	bc->bytecode[id] = S_SQ_VTX_WORD2_OFFSET(vtx->offset)|
				S_SQ_VTX_WORD2_ENDIAN_SWAP(vtx->endian);
	if (bc->gfx_level >= EVERGREEN)
		bc->bytecode[id] |= ((vtx->buffer_index_mode & 0x3) << 21); // S_SQ_VTX_WORD2_BIM(vtx->buffer_index_mode);
	if (bc->gfx_level < CAYMAN)
		bc->bytecode[id] |= S_SQ_VTX_WORD2_MEGA_FETCH(1);
	id++;
	bc->bytecode[id++] = 0;
	return 0;
}

/* common to all 3 families */
static int r600_bytecode_tex_build(struct r600_bytecode *bc, struct r600_bytecode_tex *tex, unsigned id)
{
	bc->bytecode[id] = S_SQ_TEX_WORD0_TEX_INST(
					r600_isa_fetch_opcode(bc->isa->hw_class, tex->op)) |
			    EG_S_SQ_TEX_WORD0_INST_MOD(tex->inst_mod) |
				S_SQ_TEX_WORD0_RESOURCE_ID(tex->resource_id) |
				S_SQ_TEX_WORD0_SRC_GPR(tex->src_gpr) |
				S_SQ_TEX_WORD0_SRC_REL(tex->src_rel);
	if (bc->gfx_level >= EVERGREEN)
		bc->bytecode[id] |= ((tex->sampler_index_mode & 0x3) << 27) | // S_SQ_TEX_WORD0_SIM(tex->sampler_index_mode);
				((tex->resource_index_mode & 0x3) << 25); // S_SQ_TEX_WORD0_RIM(tex->resource_index_mode)
	id++;
	bc->bytecode[id++] = S_SQ_TEX_WORD1_DST_GPR(tex->dst_gpr) |
				S_SQ_TEX_WORD1_DST_REL(tex->dst_rel) |
				S_SQ_TEX_WORD1_DST_SEL_X(tex->dst_sel_x) |
				S_SQ_TEX_WORD1_DST_SEL_Y(tex->dst_sel_y) |
				S_SQ_TEX_WORD1_DST_SEL_Z(tex->dst_sel_z) |
				S_SQ_TEX_WORD1_DST_SEL_W(tex->dst_sel_w) |
				S_SQ_TEX_WORD1_LOD_BIAS(tex->lod_bias) |
				S_SQ_TEX_WORD1_COORD_TYPE_X(tex->coord_type_x) |
				S_SQ_TEX_WORD1_COORD_TYPE_Y(tex->coord_type_y) |
				S_SQ_TEX_WORD1_COORD_TYPE_Z(tex->coord_type_z) |
				S_SQ_TEX_WORD1_COORD_TYPE_W(tex->coord_type_w);
	bc->bytecode[id++] = S_SQ_TEX_WORD2_OFFSET_X(tex->offset_x) |
				S_SQ_TEX_WORD2_OFFSET_Y(tex->offset_y) |
				S_SQ_TEX_WORD2_OFFSET_Z(tex->offset_z) |
				S_SQ_TEX_WORD2_SAMPLER_ID(tex->sampler_id) |
				S_SQ_TEX_WORD2_SRC_SEL_X(tex->src_sel_x) |
				S_SQ_TEX_WORD2_SRC_SEL_Y(tex->src_sel_y) |
				S_SQ_TEX_WORD2_SRC_SEL_Z(tex->src_sel_z) |
				S_SQ_TEX_WORD2_SRC_SEL_W(tex->src_sel_w);
	bc->bytecode[id++] = 0;
	return 0;
}

/* r600 only, r700/eg bits in r700_asm.c */
static int r600_bytecode_alu_build(struct r600_bytecode *bc, struct r600_bytecode_alu *alu, unsigned id)
{
	unsigned opcode = r600_isa_alu_opcode(bc->isa->hw_class, alu->op);

	/* don't replace gpr by pv or ps for destination register */
	bc->bytecode[id++] = S_SQ_ALU_WORD0_SRC0_SEL(alu->src[0].sel) |
				S_SQ_ALU_WORD0_SRC0_REL(alu->src[0].rel) |
				S_SQ_ALU_WORD0_SRC0_CHAN(alu->src[0].chan) |
				S_SQ_ALU_WORD0_SRC0_NEG(alu->src[0].neg) |
				S_SQ_ALU_WORD0_SRC1_SEL(alu->src[1].sel) |
				S_SQ_ALU_WORD0_SRC1_REL(alu->src[1].rel) |
				S_SQ_ALU_WORD0_SRC1_CHAN(alu->src[1].chan) |
				S_SQ_ALU_WORD0_SRC1_NEG(alu->src[1].neg) |
				S_SQ_ALU_WORD0_INDEX_MODE(alu->index_mode) |
				S_SQ_ALU_WORD0_PRED_SEL(alu->pred_sel) |
				S_SQ_ALU_WORD0_LAST(alu->last);

	if (alu->is_op3) {
		assert(!alu->src[0].abs && !alu->src[1].abs && !alu->src[2].abs);
		bc->bytecode[id++] = S_SQ_ALU_WORD1_DST_GPR(alu->dst.sel) |
					S_SQ_ALU_WORD1_DST_CHAN(alu->dst.chan) |
					S_SQ_ALU_WORD1_DST_REL(alu->dst.rel) |
					S_SQ_ALU_WORD1_CLAMP(alu->dst.clamp) |
					S_SQ_ALU_WORD1_OP3_SRC2_SEL(alu->src[2].sel) |
					S_SQ_ALU_WORD1_OP3_SRC2_REL(alu->src[2].rel) |
					S_SQ_ALU_WORD1_OP3_SRC2_CHAN(alu->src[2].chan) |
					S_SQ_ALU_WORD1_OP3_SRC2_NEG(alu->src[2].neg) |
					S_SQ_ALU_WORD1_OP3_ALU_INST(opcode) |
					S_SQ_ALU_WORD1_BANK_SWIZZLE(alu->bank_swizzle);
	} else {
		bc->bytecode[id++] = S_SQ_ALU_WORD1_DST_GPR(alu->dst.sel) |
					S_SQ_ALU_WORD1_DST_CHAN(alu->dst.chan) |
					S_SQ_ALU_WORD1_DST_REL(alu->dst.rel) |
					S_SQ_ALU_WORD1_CLAMP(alu->dst.clamp) |
					S_SQ_ALU_WORD1_OP2_SRC0_ABS(alu->src[0].abs) |
					S_SQ_ALU_WORD1_OP2_SRC1_ABS(alu->src[1].abs) |
					S_SQ_ALU_WORD1_OP2_WRITE_MASK(alu->dst.write) |
					S_SQ_ALU_WORD1_OP2_OMOD(alu->omod) |
					S_SQ_ALU_WORD1_OP2_ALU_INST(opcode) |
					S_SQ_ALU_WORD1_BANK_SWIZZLE(alu->bank_swizzle) |
					S_SQ_ALU_WORD1_OP2_UPDATE_EXECUTE_MASK(alu->execute_mask) |
					S_SQ_ALU_WORD1_OP2_UPDATE_PRED(alu->update_pred);
	}
	return 0;
}

static void r600_bytecode_cf_vtx_build(uint32_t *bytecode, const struct r600_bytecode_cf *cf)
{
	*bytecode++ = S_SQ_CF_WORD0_ADDR(cf->addr >> 1);
	*bytecode++ = S_SQ_CF_WORD1_CF_INST(r600_isa_cf_opcode(ISA_CC_R600, cf->op)) |
			S_SQ_CF_WORD1_BARRIER(1) |
			S_SQ_CF_WORD1_COUNT((cf->ndw / 4) - 1)|
			S_SQ_CF_WORD1_END_OF_PROGRAM(cf->end_of_program);
}

/* common for r600/r700 - eg in eg_asm.c */
static int r600_bytecode_cf_build(struct r600_bytecode *bc, struct r600_bytecode_cf *cf)
{
	unsigned id = cf->id;
	const struct cf_op_info *cfop = r600_isa_cf(cf->op);
	unsigned opcode = r600_isa_cf_opcode(bc->isa->hw_class, cf->op);


	if (cf->op == CF_NATIVE) {
		bc->bytecode[id++] = cf->isa[0];
		bc->bytecode[id++] = cf->isa[1];
	} else if (cfop->flags & CF_ALU) {
		bc->bytecode[id++] = S_SQ_CF_ALU_WORD0_ADDR(cf->addr >> 1) |
			S_SQ_CF_ALU_WORD0_KCACHE_MODE0(cf->kcache[0].mode) |
			S_SQ_CF_ALU_WORD0_KCACHE_BANK0(cf->kcache[0].bank) |
			S_SQ_CF_ALU_WORD0_KCACHE_BANK1(cf->kcache[1].bank);

		bc->bytecode[id++] = S_SQ_CF_ALU_WORD1_CF_INST(opcode) |
			S_SQ_CF_ALU_WORD1_KCACHE_MODE1(cf->kcache[1].mode) |
			S_SQ_CF_ALU_WORD1_KCACHE_ADDR0(cf->kcache[0].addr) |
			S_SQ_CF_ALU_WORD1_KCACHE_ADDR1(cf->kcache[1].addr) |
					S_SQ_CF_ALU_WORD1_BARRIER(1) |
					S_SQ_CF_ALU_WORD1_USES_WATERFALL(bc->gfx_level == R600 ? cf->r6xx_uses_waterfall : 0) |
					S_SQ_CF_ALU_WORD1_COUNT((cf->ndw / 2) - 1);
	} else if (cfop->flags & CF_FETCH) {
		if (bc->gfx_level == R700)
			r700_bytecode_cf_vtx_build(&bc->bytecode[id], cf);
		else
			r600_bytecode_cf_vtx_build(&bc->bytecode[id], cf);
	} else if (cfop->flags & CF_EXP) {
		bc->bytecode[id++] = S_SQ_CF_ALLOC_EXPORT_WORD0_RW_GPR(cf->output.gpr) |
			S_SQ_CF_ALLOC_EXPORT_WORD0_ELEM_SIZE(cf->output.elem_size) |
			S_SQ_CF_ALLOC_EXPORT_WORD0_ARRAY_BASE(cf->output.array_base) |
			S_SQ_CF_ALLOC_EXPORT_WORD0_TYPE(cf->output.type) |
			S_SQ_CF_ALLOC_EXPORT_WORD0_INDEX_GPR(cf->output.index_gpr);
		bc->bytecode[id++] = S_SQ_CF_ALLOC_EXPORT_WORD1_BURST_COUNT(cf->output.burst_count - 1) |
			S_SQ_CF_ALLOC_EXPORT_WORD1_SWIZ_SEL_X(cf->output.swizzle_x) |
			S_SQ_CF_ALLOC_EXPORT_WORD1_SWIZ_SEL_Y(cf->output.swizzle_y) |
			S_SQ_CF_ALLOC_EXPORT_WORD1_SWIZ_SEL_Z(cf->output.swizzle_z) |
			S_SQ_CF_ALLOC_EXPORT_WORD1_SWIZ_SEL_W(cf->output.swizzle_w) |
			S_SQ_CF_ALLOC_EXPORT_WORD1_BARRIER(cf->barrier) |
			S_SQ_CF_ALLOC_EXPORT_WORD1_CF_INST(opcode) |
			S_SQ_CF_ALLOC_EXPORT_WORD1_END_OF_PROGRAM(cf->end_of_program);
	} else if (cfop->flags & CF_MEM) {
		bc->bytecode[id++] = S_SQ_CF_ALLOC_EXPORT_WORD0_RW_GPR(cf->output.gpr) |
			S_SQ_CF_ALLOC_EXPORT_WORD0_ELEM_SIZE(cf->output.elem_size) |
			S_SQ_CF_ALLOC_EXPORT_WORD0_ARRAY_BASE(cf->output.array_base) |
			S_SQ_CF_ALLOC_EXPORT_WORD0_TYPE(cf->output.type) |
			S_SQ_CF_ALLOC_EXPORT_WORD0_INDEX_GPR(cf->output.index_gpr);
		bc->bytecode[id++] = S_SQ_CF_ALLOC_EXPORT_WORD1_BURST_COUNT(cf->output.burst_count - 1) |
			S_SQ_CF_ALLOC_EXPORT_WORD1_BARRIER(cf->barrier) |
			S_SQ_CF_ALLOC_EXPORT_WORD1_CF_INST(opcode) |
			S_SQ_CF_ALLOC_EXPORT_WORD1_END_OF_PROGRAM(cf->end_of_program) |
			S_SQ_CF_ALLOC_EXPORT_WORD1_BUF_ARRAY_SIZE(cf->output.array_size) |
			S_SQ_CF_ALLOC_EXPORT_WORD1_BUF_COMP_MASK(cf->output.comp_mask);
	} else {
		bc->bytecode[id++] = S_SQ_CF_WORD0_ADDR(cf->cf_addr >> 1);
		bc->bytecode[id++] = S_SQ_CF_WORD1_CF_INST(opcode) |
					S_SQ_CF_WORD1_BARRIER(1) |
			                S_SQ_CF_WORD1_COND(cf->cond) |
			                S_SQ_CF_WORD1_POP_COUNT(cf->pop_count) |
					S_SQ_CF_WORD1_END_OF_PROGRAM(cf->end_of_program);
	}
	return 0;
}

int r600_bytecode_build(struct r600_bytecode *bc)
{
	struct r600_bytecode_cf *cf;
	struct r600_bytecode_alu *alu;
	struct r600_bytecode_vtx *vtx;
	struct r600_bytecode_tex *tex;
	struct r600_bytecode_gds *gds;
	uint32_t literal[4];
	unsigned nliteral;
	unsigned addr;
	int i, r;

	if (!bc->nstack) { // If not 0, Stack_size already provided by llvm
		if (bc->stack.max_entries)
			bc->nstack = bc->stack.max_entries;
		else if (bc->type == PIPE_SHADER_VERTEX ||
			 bc->type == PIPE_SHADER_TESS_EVAL ||
			 bc->type == PIPE_SHADER_TESS_CTRL)
			bc->nstack = 1;
	}

	/* first path compute addr of each CF block */
	/* addr start after all the CF instructions */
	addr = bc->cf_last->id + 2;
	LIST_FOR_EACH_ENTRY(cf, &bc->cf, list) {
		if (r600_isa_cf(cf->op)->flags & CF_FETCH) {
			addr += 3;
			addr &= 0xFFFFFFFCUL;
		}
		cf->addr = addr;
		addr += cf->ndw;
		bc->ndw = cf->addr + cf->ndw;
	}
	free(bc->bytecode);
	bc->bytecode = calloc(4, bc->ndw);
	if (bc->bytecode == NULL)
		return -ENOMEM;
	LIST_FOR_EACH_ENTRY(cf, &bc->cf, list) {
		const struct cf_op_info *cfop = r600_isa_cf(cf->op);
		addr = cf->addr;
		if (bc->gfx_level >= EVERGREEN)
			r = eg_bytecode_cf_build(bc, cf);
		else
			r = r600_bytecode_cf_build(bc, cf);
		if (r)
			return r;
		if (cfop->flags & CF_ALU) {
			nliteral = 0;
			memset(literal, 0, sizeof(literal));
			LIST_FOR_EACH_ENTRY(alu, &cf->alu, list) {
				r = r600_bytecode_alu_nliterals(alu, literal, &nliteral);
				if (r)
					return r;
				r600_bytecode_alu_adjust_literals(alu, literal, nliteral);
				r600_bytecode_assign_kcache_banks(alu, cf->kcache);

				switch(bc->gfx_level) {
				case R600:
					r = r600_bytecode_alu_build(bc, alu, addr);
					break;
				case R700:
					r = r700_bytecode_alu_build(bc, alu, addr);
					break;
				case EVERGREEN:
				case CAYMAN:
					r = eg_bytecode_alu_build(bc, alu, addr);
					break;
				default:
					R600_ASM_ERR("unknown gfx level %d.\n", bc->gfx_level);
					return -EINVAL;
				}
				if (r)
					return r;
				addr += 2;
				if (alu->last) {
					for (i = 0; i < align(nliteral, 2); ++i) {
						bc->bytecode[addr++] = literal[i];
					}
					nliteral = 0;
					memset(literal, 0, sizeof(literal));
				}
			}
		} else if (cf->op == CF_OP_VTX) {
			LIST_FOR_EACH_ENTRY(vtx, &cf->vtx, list) {
				r = r600_bytecode_vtx_build(bc, vtx, addr);
				if (r)
					return r;
				addr += 4;
			}
		} else if (cf->op == CF_OP_GDS) {
			assert(bc->gfx_level >= EVERGREEN);
			LIST_FOR_EACH_ENTRY(gds, &cf->gds, list) {
				r = eg_bytecode_gds_build(bc, gds, addr);
				if (r)
					return r;
				addr += 4;
			}
		} else if (cf->op == CF_OP_TEX) {
			LIST_FOR_EACH_ENTRY(vtx, &cf->vtx, list) {
				assert(bc->gfx_level >= EVERGREEN);
				r = r600_bytecode_vtx_build(bc, vtx, addr);
				if (r)
					return r;
				addr += 4;
			}
			LIST_FOR_EACH_ENTRY(tex, &cf->tex, list) {
				r = r600_bytecode_tex_build(bc, tex, addr);
				if (r)
					return r;
				addr += 4;
			}
		}
	}
	return 0;
}

void r600_bytecode_clear(struct r600_bytecode *bc)
{
	struct r600_bytecode_cf *cf = NULL, *next_cf;

	free(bc->bytecode);
	bc->bytecode = NULL;

	LIST_FOR_EACH_ENTRY_SAFE(cf, next_cf, &bc->cf, list) {
		struct r600_bytecode_alu *alu = NULL, *next_alu;
		struct r600_bytecode_tex *tex = NULL, *next_tex;
		struct r600_bytecode_tex *vtx = NULL, *next_vtx;
		struct r600_bytecode_gds *gds = NULL, *next_gds;

		LIST_FOR_EACH_ENTRY_SAFE(alu, next_alu, &cf->alu, list) {
			free(alu);
		}

		list_inithead(&cf->alu);

		LIST_FOR_EACH_ENTRY_SAFE(tex, next_tex, &cf->tex, list) {
			free(tex);
		}

		list_inithead(&cf->tex);

		LIST_FOR_EACH_ENTRY_SAFE(vtx, next_vtx, &cf->vtx, list) {
			free(vtx);
		}

		list_inithead(&cf->vtx);

		LIST_FOR_EACH_ENTRY_SAFE(gds, next_gds, &cf->gds, list) {
			free(gds);
		}

		list_inithead(&cf->gds);

		free(cf);
	}

	list_inithead(&cf->list);
}

static int print_swizzle(unsigned swz)
{
	const char * swzchars = "xyzw01?_";
	assert(swz<8 && swz != 6);
	return fprintf(stderr, "%c", swzchars[swz]);
}

static int print_sel(unsigned sel, unsigned rel, unsigned index_mode,
		unsigned need_brackets)
{
	int o = 0;
	if (rel && index_mode >= 5 && sel < 128)
		o += fprintf(stderr, "G");
	if (rel || need_brackets) {
		o += fprintf(stderr, "[");
	}
	o += fprintf(stderr, "%d", sel);
	if (rel) {
		if (index_mode == 0 || index_mode == 6)
			o += fprintf(stderr, "+AR");
		else if (index_mode == 4)
			o += fprintf(stderr, "+AL");
	}
	if (rel || need_brackets) {
		o += fprintf(stderr, "]");
	}
	return o;
}

static int print_dst(struct r600_bytecode_alu *alu)
{
	int o = 0;
	unsigned sel = alu->dst.sel;
	char reg_char = 'R';
	if (sel >= 128 - 4) { /* clause temporary gpr */
		sel -= 128 - 4;
		reg_char = 'T';
	}

	if (alu_writes(alu)) {
		o += fprintf(stderr, "%c", reg_char);
		o += print_sel(sel, alu->dst.rel, alu->index_mode, 0);
	} else {
		o += fprintf(stderr, "__");
	}
	o += fprintf(stderr, ".");
	o += print_swizzle(alu->dst.chan);
	return o;
}

static int print_src(struct r600_bytecode_alu *alu, unsigned idx)
{
	int o = 0;
	struct r600_bytecode_alu_src *src = &alu->src[idx];
	unsigned sel = src->sel, need_sel = 1, need_chan = 1, need_brackets = 0;

	if (src->neg)
		o += fprintf(stderr,"-");
	if (src->abs)
		o += fprintf(stderr,"|");

	if (sel < 128 - 4) {
		o += fprintf(stderr, "R");
	} else if (sel < 128) {
		o += fprintf(stderr, "T");
		sel -= 128 - 4;
	} else if (sel < 160) {
		o += fprintf(stderr, "KC0");
		need_brackets = 1;
		sel -= 128;
	} else if (sel < 192) {
		o += fprintf(stderr, "KC1");
		need_brackets = 1;
		sel -= 160;
	} else if (sel >= 512) {
		o += fprintf(stderr, "C%d", src->kc_bank);
		need_brackets = 1;
		sel -= 512;
	} else if (sel >= 448) {
		o += fprintf(stderr, "Param");
		sel -= 448;
		need_chan = 0;
	} else if (sel >= 288) {
		o += fprintf(stderr, "KC3");
		need_brackets = 1;
		sel -= 288;
	} else if (sel >= 256) {
		o += fprintf(stderr, "KC2");
		need_brackets = 1;
		sel -= 256;
	} else {
		need_sel = 0;
		need_chan = 0;
		switch (sel) {
		case EG_V_SQ_ALU_SRC_LDS_DIRECT_A:
			o += fprintf(stderr, "LDS_A[0x%08X]", src->value);
			break;
		case EG_V_SQ_ALU_SRC_LDS_DIRECT_B:
			o += fprintf(stderr, "LDS_B[0x%08X]", src->value);
			break;
		case EG_V_SQ_ALU_SRC_LDS_OQ_A:
			o += fprintf(stderr, "LDS_OQ_A");
			need_chan = 1;
			break;
		case EG_V_SQ_ALU_SRC_LDS_OQ_B:
			o += fprintf(stderr, "LDS_OQ_B");
			need_chan = 1;
			break;
		case EG_V_SQ_ALU_SRC_LDS_OQ_A_POP:
			o += fprintf(stderr, "LDS_OQ_A_POP");
			need_chan = 1;
			break;
		case EG_V_SQ_ALU_SRC_LDS_OQ_B_POP:
			o += fprintf(stderr, "LDS_OQ_B_POP");
			need_chan = 1;
			break;
		case EG_V_SQ_ALU_SRC_TIME_LO:
			o += fprintf(stderr, "TIME_LO");
			break;
		case EG_V_SQ_ALU_SRC_TIME_HI:
			o += fprintf(stderr, "TIME_HI");
			break;
		case EG_V_SQ_ALU_SRC_SE_ID:
			o += fprintf(stderr, "SE_ID");
			break;
		case EG_V_SQ_ALU_SRC_SIMD_ID:
			o += fprintf(stderr, "SIMD_ID");
			break;
		case EG_V_SQ_ALU_SRC_HW_WAVE_ID:
			o += fprintf(stderr, "HW_WAVE_ID");
			break;
		case V_SQ_ALU_SRC_PS:
			o += fprintf(stderr, "PS");
			break;
		case V_SQ_ALU_SRC_PV:
			o += fprintf(stderr, "PV");
			need_chan = 1;
			break;
		case V_SQ_ALU_SRC_LITERAL:
			{
				const uint32_t value_uint32 = src->value;
				float value_float;
				memcpy(&value_float, &value_uint32, sizeof(float));
				o += fprintf(stderr, "[0x%08X %f]", value_uint32, value_float);
			}
			break;
		case V_SQ_ALU_SRC_0_5:
			o += fprintf(stderr, "0.5");
			break;
		case V_SQ_ALU_SRC_M_1_INT:
			o += fprintf(stderr, "-1");
			break;
		case V_SQ_ALU_SRC_1_INT:
			o += fprintf(stderr, "1");
			break;
		case V_SQ_ALU_SRC_1:
			o += fprintf(stderr, "1.0");
			break;
		case V_SQ_ALU_SRC_0:
			o += fprintf(stderr, "0");
			break;
		default:
			o += fprintf(stderr, "??IMM_%d", sel);
			break;
		}
	}

	if (need_sel)
		o += print_sel(sel, src->rel, alu->index_mode, need_brackets);

	if (need_chan) {
		o += fprintf(stderr, ".");
		o += print_swizzle(src->chan);
	}

	if (src->abs)
		o += fprintf(stderr,"|");

	return o;
}

static int print_indent(int p, int c)
{
	int o = 0;
	while (p++ < c)
		o += fprintf(stderr, " ");
	return o;
}

const char *rat_instr_name[] = {
   "NOP",
   "STORE_TYPED",
   "STORE_RAW",
   "STORE_RAW_FDENORM",
   "CMP_XCHG_INT",
   "CMP_XCHG_FLT",
   "CMP_XCHG_FDENORM",
   "ADD",
   "SUB",
   "RSUB",
   "MIN_INT",
   "MIN_UINT",
   "MAX_INT",
   "MAX_UINT",
   "AND",
   "OR",
   "XOR",
   "MSKOR",
   "INC_UINT",
   "DEC_UINT",
   "RESERVED20",
   "RESERVED21",
   "RESERVED22",
   "RESERVED23",
   "RESERVED24",
   "RESERVED25",
   "RESERVED26",
   "RESERVED27",
   "RESERVED28",
   "RESERVED29",
   "RESERVED30",
   "RESERVED31",
   "NOP_RTN",
   "RESERVED33",
   "XCHG_RTN",
   "XCHG_FDENORM_RTN",
   "CMPXCHG_INT_RTN",
   "CMPXCHG_FLT_RTN",
   "CMPXCHG_FDENORM_RTN",
   "ADD_RTN",
   "SUB_RTN",
   "RSUB_RTN",
   "MIN_INT_RTN",
   "MIN_UINT_RTN",
   "MAX_INT_RTN",
   "MAX_UINT_RTN",
   "AND_RTN",
   "OR_RTN",
   "XOR_RTN",
   "MSKOR_RTN",
   "INC_UINT_RTN",
   "DEC_UINT_RTN",
};


void r600_bytecode_disasm(struct r600_bytecode *bc)
{
	const char *index_mode[] = {"CF_INDEX_NONE", "CF_INDEX_0", "CF_INDEX_1"};
	static int index = 0;
	struct r600_bytecode_cf *cf = NULL;
	struct r600_bytecode_alu *alu = NULL;
	struct r600_bytecode_vtx *vtx = NULL;
	struct r600_bytecode_tex *tex = NULL;
	struct r600_bytecode_gds *gds = NULL;

	unsigned id, ngr = 0, last;
	uint32_t literal[4];
	unsigned nliteral;
	char chip = '6';

	switch (bc->gfx_level) {
	case R700:
		chip = '7';
		break;
	case EVERGREEN:
		chip = 'E';
		break;
	case CAYMAN:
		chip = 'C';
		break;
	case R600:
	default:
		chip = '6';
		break;
	}
	fprintf(stderr, "bytecode %d dw -- %d gprs -- %d nstack -------------\n",
	        bc->ndw, bc->ngpr, bc->nstack);
	fprintf(stderr, "shader %d -- %c\n", index++, chip);

	LIST_FOR_EACH_ENTRY(cf, &bc->cf, list) {
		id = cf->id;
		if (cf->op == CF_NATIVE) {
			fprintf(stderr, "%04d %08X %08X CF_NATIVE\n", id, bc->bytecode[id],
					bc->bytecode[id + 1]);
		} else {
			const struct cf_op_info *cfop = r600_isa_cf(cf->op);
			if (cfop->flags & CF_ALU) {
				if (cf->eg_alu_extended) {
					fprintf(stderr, "%04d %08X %08X  %s\n", id, bc->bytecode[id],
							bc->bytecode[id + 1], "ALU_EXT");
					id += 2;
				}
				fprintf(stderr, "%04d %08X %08X  %s ", id, bc->bytecode[id],
						bc->bytecode[id + 1], cfop->name);
				fprintf(stderr, "%d @%d ", cf->ndw / 2, cf->addr);
				for (int i = 0; i < 4; ++i) {
					if (cf->kcache[i].mode) {
						int c_start = (cf->kcache[i].addr << 4);
						int c_end = c_start + (cf->kcache[i].mode << 4);
						fprintf(stderr, "KC%d[CB%d:%d-%d%s%s] ",
						        i, cf->kcache[i].bank, c_start, c_end,
						        cf->kcache[i].index_mode ? " " : "",
						        cf->kcache[i].index_mode ? index_mode[cf->kcache[i].index_mode] : "");
					}
				}
				fprintf(stderr, "\n");
			} else if (cfop->flags & CF_FETCH) {
				fprintf(stderr, "%04d %08X %08X  %s ", id, bc->bytecode[id],
						bc->bytecode[id + 1], cfop->name);
				fprintf(stderr, "%d @%d ", cf->ndw / 4, cf->addr);
				if (cf->vpm)
					fprintf(stderr, "VPM ");
				if (cf->end_of_program)
					fprintf(stderr, "EOP ");
				fprintf(stderr, "\n");

			} else if (cfop->flags & CF_EXP) {
				int o = 0;
				const char *exp_type[] = {"PIXEL", "POS  ", "PARAM"};
				o += fprintf(stderr, "%04d %08X %08X  %s ", id, bc->bytecode[id],
						bc->bytecode[id + 1], cfop->name);
				o += print_indent(o, 43);
				o += fprintf(stderr, "%s ", exp_type[cf->output.type]);
				if (cf->output.burst_count > 1) {
					o += fprintf(stderr, "%d-%d ", cf->output.array_base,
							cf->output.array_base + cf->output.burst_count - 1);

					o += print_indent(o, 55);
					o += fprintf(stderr, "R%d-%d.", cf->output.gpr,
							cf->output.gpr + cf->output.burst_count - 1);
				} else {
					o += fprintf(stderr, "%d ", cf->output.array_base);
					o += print_indent(o, 55);
					o += fprintf(stderr, "R%d.", cf->output.gpr);
				}

				o += print_swizzle(cf->output.swizzle_x);
				o += print_swizzle(cf->output.swizzle_y);
				o += print_swizzle(cf->output.swizzle_z);
				o += print_swizzle(cf->output.swizzle_w);

				print_indent(o, 67);

				fprintf(stderr, " ES:%X ", cf->output.elem_size);
				if (cf->mark)
					fprintf(stderr, "MARK ");
				if (!cf->barrier)
					fprintf(stderr, "NO_BARRIER ");
				if (cf->end_of_program)
					fprintf(stderr, "EOP ");
				fprintf(stderr, "\n");
			} else if (r600_isa_cf(cf->op)->flags & CF_MEM) {
				int o = 0;
				const char *exp_type_r600[] = {"WRITE", "WRITE_IND", "READ",
				                               "READ_IND"};
				const char *exp_type_r700[] = {"WRITE", "WRITE_IND", "WRITE_ACK",
				                               "WRITE_IND_ACK"};

				const char **exp_type = bc->gfx_level >= R700 ?
                                       exp_type_r700 : exp_type_r600;

				o += fprintf(stderr, "%04d %08X %08X  %s ", id,
						bc->bytecode[id], bc->bytecode[id + 1], cfop->name);
				o += print_indent(o, 43);
				o += fprintf(stderr, "%s ", exp_type[cf->output.type]);

				if (r600_isa_cf(cf->op)->flags & CF_RAT) {
					o += fprintf(stderr, "RAT%d", cf->rat.id);
					if (cf->rat.index_mode) {
						o += fprintf(stderr, "[IDX%d]", cf->rat.index_mode - 1);
					}
               assert(ARRAY_SIZE(rat_instr_name) > cf->rat.inst);
					o += fprintf(stderr, " %s ", rat_instr_name[cf->rat.inst]);
				}

				if (cf->output.burst_count > 1) {
					o += fprintf(stderr, "%d-%d ", cf->output.array_base,
							cf->output.array_base + cf->output.burst_count - 1);
					o += print_indent(o, 55);
					o += fprintf(stderr, "R%d-%d.", cf->output.gpr,
							cf->output.gpr + cf->output.burst_count - 1);
				} else {
					o += fprintf(stderr, "%d ", cf->output.array_base);
					o += print_indent(o, 55);
					o += fprintf(stderr, "R%d.", cf->output.gpr);
				}
				for (int i = 0; i < 4; ++i) {
					if (cf->output.comp_mask & (1 << i))
						o += print_swizzle(i);
					else
						o += print_swizzle(7);
				}

				if (cf->output.type == V_SQ_CF_ALLOC_EXPORT_WORD0_SQ_EXPORT_WRITE_IND ||
				    cf->output.type == V_SQ_CF_ALLOC_EXPORT_WORD0_SQ_EXPORT_READ_IND)
					o += fprintf(stderr, " R%d.xyz", cf->output.index_gpr);

				o += print_indent(o, 67);

				fprintf(stderr, " ES:%i ", cf->output.elem_size);
				if (cf->output.array_size != 0xFFF)
					fprintf(stderr, "AS:%i ", cf->output.array_size);
				if (cf->mark)
					fprintf(stderr, "MARK ");
				if (!cf->barrier)
					fprintf(stderr, "NO_BARRIER ");
				if (cf->end_of_program)
					fprintf(stderr, "EOP ");

				if (cf->output.mark)
					fprintf(stderr, "MARK ");

				fprintf(stderr, "\n");
			} else {
				fprintf(stderr, "%04d %08X %08X  %s ", id, bc->bytecode[id],
						bc->bytecode[id + 1], cfop->name);
				fprintf(stderr, "@%d ", cf->cf_addr);
				if (cf->cond)
					fprintf(stderr, "CND:%X ", cf->cond);
				if (cf->pop_count)
					fprintf(stderr, "POP:%X ", cf->pop_count);
				if (cf->count && (cfop->flags & CF_EMIT))
					fprintf(stderr, "STREAM%d ", cf->count);
				if (cf->vpm)
					fprintf(stderr, "VPM ");
				if (cf->end_of_program)
					fprintf(stderr, "EOP ");
				fprintf(stderr, "\n");
			}
		}

		id = cf->addr;
		nliteral = 0;
		last = 1;
		int chan_mask = 0;
		LIST_FOR_EACH_ENTRY(alu, &cf->alu, list) {
			const char chan[] = "xyzwt";
			const char *omod_str[] = {"","*2","*4","/2"};
			const struct alu_op_info *aop = r600_isa_alu(alu->op);
			int o = 0;

			r600_bytecode_alu_nliterals(alu, literal, &nliteral);
			o += fprintf(stderr, " %04d %08X %08X  ", id, bc->bytecode[id], bc->bytecode[id+1]);
			if (last)
				o += fprintf(stderr, "%4d ", ++ngr);
			else
				o += fprintf(stderr, "     ");

			if ((chan_mask & (1 << alu->dst.chan)) ||
				((aop->slots[bc->isa->hw_class] == AF_S) && !(bc->isa->hw_class == ISA_CC_CAYMAN)))
				o += fprintf(stderr, "t:");
			else
				o += fprintf(stderr, "%c:", chan[alu->dst.chan]);
			chan_mask |= 1 << alu->dst.chan;

			o += fprintf(stderr, "%c%c %c ", alu->execute_mask ? 'M':' ',
					alu->update_pred ? 'P':' ',
					alu->pred_sel ? alu->pred_sel==2 ? '0':'1':' ');

			o += fprintf(stderr, "%s%s%s ", aop->name,
					omod_str[alu->omod], alu->dst.clamp ? "_sat":"");

			o += print_indent(o,60);
			if (bc->isa->hw_class == ISA_CC_CAYMAN && alu->op == ALU_OP1_MOVA_INT) {
				switch (alu->dst.sel) {
				case 0: fprintf(stderr, "AR"); break;
				case 2: fprintf(stderr, "CF_IDX0"); break;
				case 3: fprintf(stderr, "CF_IDX1"); break;
				}
			} else {
				o += print_dst(alu);
			}
			for (int i = 0; i < aop->src_count; ++i) {
				o += fprintf(stderr, i == 0 ? ",  ": ", ");
				o += print_src(alu, i);
			}

			if (alu->bank_swizzle) {
				o += print_indent(o,75);
				o += fprintf(stderr, "  BS:%d", alu->bank_swizzle);
			}

			fprintf(stderr, "\n");
			id += 2;

			if (alu->last) {
				for (unsigned i = 0; i < nliteral; i++, id++) {
					float *f = (float*)(bc->bytecode + id);
					o = fprintf(stderr, " %04d %08X", id, bc->bytecode[id]);
					print_indent(o, 60);
					fprintf(stderr, " %f (%d)\n", *f, *(bc->bytecode + id));
				}
				id += nliteral & 1;
				nliteral = 0;
				chan_mask = 0;
			}
			last = alu->last;
		}

		LIST_FOR_EACH_ENTRY(tex, &cf->tex, list) {
			int o = 0;
			o += fprintf(stderr, " %04d %08X %08X %08X   ", id, bc->bytecode[id],
					bc->bytecode[id + 1], bc->bytecode[id + 2]);

			o += fprintf(stderr, "%s ", r600_isa_fetch(tex->op)->name);

			o += print_indent(o, 50);

			o += fprintf(stderr, "R%d.", tex->dst_gpr);
			o += print_swizzle(tex->dst_sel_x);
			o += print_swizzle(tex->dst_sel_y);
			o += print_swizzle(tex->dst_sel_z);
			o += print_swizzle(tex->dst_sel_w);

			o += fprintf(stderr, ", R%d.", tex->src_gpr);
			o += print_swizzle(tex->src_sel_x);
			o += print_swizzle(tex->src_sel_y);
			o += print_swizzle(tex->src_sel_z);
			o += print_swizzle(tex->src_sel_w);

			o += fprintf(stderr, ",  RID:%d ", tex->resource_id);
                        if (tex->resource_index_mode)
				fprintf(stderr, "RQ_%s", index_mode[tex->resource_index_mode]);

			o += fprintf(stderr, ", SID:%d  ", tex->sampler_id);

			if (tex->sampler_index_mode)
				fprintf(stderr, "SQ_%s ", index_mode[tex->sampler_index_mode]);



			if (tex->lod_bias)
				fprintf(stderr, "LB:%d ", tex->lod_bias);

			fprintf(stderr, "CT:%c%c%c%c ",
					tex->coord_type_x ? 'N' : 'U',
					tex->coord_type_y ? 'N' : 'U',
					tex->coord_type_z ? 'N' : 'U',
					tex->coord_type_w ? 'N' : 'U');

			if (tex->offset_x)
				fprintf(stderr, "OX:%d ", tex->offset_x);
			if (tex->offset_y)
				fprintf(stderr, "OY:%d ", tex->offset_y);
			if (tex->offset_z)
				fprintf(stderr, "OZ:%d ", tex->offset_z);

			id += 4;
			fprintf(stderr, "\n");
		}

		LIST_FOR_EACH_ENTRY(vtx, &cf->vtx, list) {
			int o = 0;
			const char * fetch_type[] = {"VERTEX", "INSTANCE", ""};
			o += fprintf(stderr, " %04d %08X %08X %08X   ", id, bc->bytecode[id],
					bc->bytecode[id + 1], bc->bytecode[id + 2]);

			o += fprintf(stderr, "%s ", r600_isa_fetch(vtx->op)->name);

			o += print_indent(o, 50);

			o += fprintf(stderr, "R%d.", vtx->dst_gpr);
			o += print_swizzle(vtx->dst_sel_x);
			o += print_swizzle(vtx->dst_sel_y);
			o += print_swizzle(vtx->dst_sel_z);
			o += print_swizzle(vtx->dst_sel_w);

			o += fprintf(stderr, ", R%d.", vtx->src_gpr);
			o += print_swizzle(vtx->src_sel_x);
			if (r600_isa_fetch(vtx->op)->flags & FF_MEM)
				o += print_swizzle(vtx->src_sel_y);

			if (vtx->offset)
				fprintf(stderr, " +%db", vtx->offset);

			o += print_indent(o, 55);

			fprintf(stderr, ",  RID:%d ", vtx->buffer_id);

			fprintf(stderr, "%s ", fetch_type[vtx->fetch_type]);

			if (bc->gfx_level < CAYMAN && vtx->mega_fetch_count)
				fprintf(stderr, "MFC:%d ", vtx->mega_fetch_count);

			if (bc->gfx_level >= EVERGREEN && vtx->buffer_index_mode)
				fprintf(stderr, "SQ_%s ", index_mode[vtx->buffer_index_mode]);

			if (r600_isa_fetch(vtx->op)->flags & FF_MEM) {
				if (vtx->uncached)
					fprintf(stderr, "UNCACHED ");
				if (vtx->indexed)
					fprintf(stderr, "INDEXED:%d ", vtx->indexed);

				fprintf(stderr, "ELEM_SIZE:%d ", vtx->elem_size);
				if (vtx->burst_count)
					fprintf(stderr, "BURST_COUNT:%d ", vtx->burst_count);
				fprintf(stderr, "ARRAY_BASE:%d ", vtx->array_base);
				fprintf(stderr, "ARRAY_SIZE:%d ", vtx->array_size);
			}

			fprintf(stderr, "UCF:%d ", vtx->use_const_fields);
			fprintf(stderr, "FMT(DTA:%d ", vtx->data_format);
			fprintf(stderr, "NUM:%d ", vtx->num_format_all);
			fprintf(stderr, "COMP:%d ", vtx->format_comp_all);
			fprintf(stderr, "MODE:%d)\n", vtx->srf_mode_all);

			id += 4;
		}

		LIST_FOR_EACH_ENTRY(gds, &cf->gds, list) {
			UNUSED int o = 0;
			o += fprintf(stderr, " %04d %08X %08X %08X   ", id, bc->bytecode[id],
					bc->bytecode[id + 1], bc->bytecode[id + 2]);

			o += fprintf(stderr, "%s ", r600_isa_fetch(gds->op)->name);

			if (gds->op != FETCH_OP_TF_WRITE) {
				o += fprintf(stderr, "R%d.", gds->dst_gpr);
				o += print_swizzle(gds->dst_sel_x);
				o += print_swizzle(gds->dst_sel_y);
				o += print_swizzle(gds->dst_sel_z);
				o += print_swizzle(gds->dst_sel_w);
			}

			o += fprintf(stderr, ", R%d.", gds->src_gpr);
			o += print_swizzle(gds->src_sel_x);
			o += print_swizzle(gds->src_sel_y);
			o += print_swizzle(gds->src_sel_z);

			if (gds->op != FETCH_OP_TF_WRITE) {
				o += fprintf(stderr, ", R%d.", gds->src_gpr2);
			}
			if (gds->alloc_consume) {
				o += fprintf(stderr, " UAV: %d", gds->uav_id);
				if (gds->uav_index_mode)
					o += fprintf(stderr, "[%s]", index_mode[gds->uav_index_mode]);
			}
			fprintf(stderr, "\n");
			id += 4;
		}
	}

	fprintf(stderr, "--------------------------------------\n");
}

void r600_vertex_data_type(enum pipe_format pformat,
				  unsigned *format,
				  unsigned *num_format, unsigned *format_comp, unsigned *endian)
{
	const struct util_format_description *desc;
	unsigned i;

	*format = 0;
	*num_format = 0;
	*format_comp = 0;
	*endian = ENDIAN_NONE;

	if (pformat == PIPE_FORMAT_R11G11B10_FLOAT) {
		*format = FMT_10_11_11_FLOAT;
		*endian = r600_endian_swap(32);
		return;
	}

	if (pformat == PIPE_FORMAT_B5G6R5_UNORM) {
		*format = FMT_5_6_5;
		*endian = r600_endian_swap(16);
		return;
	}

	if (pformat == PIPE_FORMAT_B5G5R5A1_UNORM) {
		*format = FMT_1_5_5_5;
		*endian = r600_endian_swap(16);
		return;
	}

	if (pformat == PIPE_FORMAT_A1B5G5R5_UNORM) {
		*format = FMT_5_5_5_1;
		return;
	}

	desc = util_format_description(pformat);
	if (desc->layout != UTIL_FORMAT_LAYOUT_PLAIN) {
		goto out_unknown;
	}

	i = util_format_get_first_non_void_channel(pformat);

	*endian = r600_endian_swap(desc->channel[i].size);

	switch (desc->channel[i].type) {
	/* Half-floats, floats, ints */
	case UTIL_FORMAT_TYPE_FLOAT:
		switch (desc->channel[i].size) {
		case 16:
			switch (desc->nr_channels) {
			case 1:
				*format = FMT_16_FLOAT;
				break;
			case 2:
				*format = FMT_16_16_FLOAT;
				break;
			case 3:
			case 4:
				*format = FMT_16_16_16_16_FLOAT;
				break;
			}
			break;
		case 32:
			switch (desc->nr_channels) {
			case 1:
				*format = FMT_32_FLOAT;
				break;
			case 2:
				*format = FMT_32_32_FLOAT;
				break;
			case 3:
				*format = FMT_32_32_32_FLOAT;
				break;
			case 4:
				*format = FMT_32_32_32_32_FLOAT;
				break;
			}
			break;
		default:
			goto out_unknown;
		}
		break;
		/* Unsigned ints */
	case UTIL_FORMAT_TYPE_UNSIGNED:
		/* Signed ints */
	case UTIL_FORMAT_TYPE_SIGNED:
		switch (desc->channel[i].size) {
		case 4:
			switch (desc->nr_channels) {
			case 2:
				*format = FMT_4_4;
				break;
			case 4:
				*format = FMT_4_4_4_4;
				break;
			}
			break;
		case 8:
			switch (desc->nr_channels) {
			case 1:
				*format = FMT_8;
				break;
			case 2:
				*format = FMT_8_8;
				break;
			case 3:
			case 4:
				*format = FMT_8_8_8_8;
				break;
			}
			break;
		case 10:
			if (desc->nr_channels != 4)
				goto out_unknown;

			*format = FMT_2_10_10_10;
			break;
		case 16:
			switch (desc->nr_channels) {
			case 1:
				*format = FMT_16;
				break;
			case 2:
				*format = FMT_16_16;
				break;
			case 3:
			case 4:
				*format = FMT_16_16_16_16;
				break;
			}
			break;
		case 32:
			switch (desc->nr_channels) {
			case 1:
				*format = FMT_32;
				break;
			case 2:
				*format = FMT_32_32;
				break;
			case 3:
				*format = FMT_32_32_32;
				break;
			case 4:
				*format = FMT_32_32_32_32;
				break;
			}
			break;
		default:
			goto out_unknown;
		}
		break;
	default:
		goto out_unknown;
	}

	if (desc->channel[i].type == UTIL_FORMAT_TYPE_SIGNED) {
		*format_comp = 1;
	}

	*num_format = 0;
	if (desc->channel[i].type == UTIL_FORMAT_TYPE_UNSIGNED ||
	    desc->channel[i].type == UTIL_FORMAT_TYPE_SIGNED) {
		if (!desc->channel[i].normalized) {
			if (desc->channel[i].pure_integer)
				*num_format = 1;
			else
				*num_format = 2;
		}
	}
	return;
out_unknown:
	R600_ASM_ERR("unsupported vertex format %s\n", util_format_name(pformat));
}

void r600_bytecode_alu_read(struct r600_bytecode *bc,
		struct r600_bytecode_alu *alu, uint32_t word0, uint32_t word1)
{
	/* WORD0 */
	alu->src[0].sel = G_SQ_ALU_WORD0_SRC0_SEL(word0);
	alu->src[0].rel = G_SQ_ALU_WORD0_SRC0_REL(word0);
	alu->src[0].chan = G_SQ_ALU_WORD0_SRC0_CHAN(word0);
	alu->src[0].neg = G_SQ_ALU_WORD0_SRC0_NEG(word0);
	alu->src[1].sel = G_SQ_ALU_WORD0_SRC1_SEL(word0);
	alu->src[1].rel = G_SQ_ALU_WORD0_SRC1_REL(word0);
	alu->src[1].chan = G_SQ_ALU_WORD0_SRC1_CHAN(word0);
	alu->src[1].neg = G_SQ_ALU_WORD0_SRC1_NEG(word0);
	alu->index_mode = G_SQ_ALU_WORD0_INDEX_MODE(word0);
	alu->pred_sel = G_SQ_ALU_WORD0_PRED_SEL(word0);
	alu->last = G_SQ_ALU_WORD0_LAST(word0);

	/* WORD1 */
	alu->bank_swizzle = G_SQ_ALU_WORD1_BANK_SWIZZLE(word1);
	if (alu->bank_swizzle)
		alu->bank_swizzle_force = alu->bank_swizzle;
	alu->dst.sel = G_SQ_ALU_WORD1_DST_GPR(word1);
	alu->dst.rel = G_SQ_ALU_WORD1_DST_REL(word1);
	alu->dst.chan = G_SQ_ALU_WORD1_DST_CHAN(word1);
	alu->dst.clamp = G_SQ_ALU_WORD1_CLAMP(word1);
	if (G_SQ_ALU_WORD1_ENCODING(word1)) /*ALU_DWORD1_OP3*/
	{
		alu->is_op3 = 1;
		alu->src[2].sel = G_SQ_ALU_WORD1_OP3_SRC2_SEL(word1);
		alu->src[2].rel = G_SQ_ALU_WORD1_OP3_SRC2_REL(word1);
		alu->src[2].chan = G_SQ_ALU_WORD1_OP3_SRC2_CHAN(word1);
		alu->src[2].neg = G_SQ_ALU_WORD1_OP3_SRC2_NEG(word1);
		alu->op = r600_isa_alu_by_opcode(bc->isa,
				G_SQ_ALU_WORD1_OP3_ALU_INST(word1), /* is_op3 = */ 1);

	}
	else /*ALU_DWORD1_OP2*/
	{
		alu->src[0].abs = G_SQ_ALU_WORD1_OP2_SRC0_ABS(word1);
		alu->src[1].abs = G_SQ_ALU_WORD1_OP2_SRC1_ABS(word1);
		alu->op = r600_isa_alu_by_opcode(bc->isa,
				G_SQ_ALU_WORD1_OP2_ALU_INST(word1), /* is_op3 = */ 0);
		alu->omod = G_SQ_ALU_WORD1_OP2_OMOD(word1);
		alu->dst.write = G_SQ_ALU_WORD1_OP2_WRITE_MASK(word1);
		alu->update_pred = G_SQ_ALU_WORD1_OP2_UPDATE_PRED(word1);
		alu->execute_mask =
			G_SQ_ALU_WORD1_OP2_UPDATE_EXECUTE_MASK(word1);
	}
}

#if 0
void r600_bytecode_export_read(struct r600_bytecode *bc,
		struct r600_bytecode_output *output, uint32_t word0, uint32_t word1)
{
	output->array_base = G_SQ_CF_ALLOC_EXPORT_WORD0_ARRAY_BASE(word0);
	output->type = G_SQ_CF_ALLOC_EXPORT_WORD0_TYPE(word0);
	output->gpr = G_SQ_CF_ALLOC_EXPORT_WORD0_RW_GPR(word0);
	output->elem_size = G_SQ_CF_ALLOC_EXPORT_WORD0_ELEM_SIZE(word0);

	output->swizzle_x = G_SQ_CF_ALLOC_EXPORT_WORD1_SWIZ_SEL_X(word1);
	output->swizzle_y = G_SQ_CF_ALLOC_EXPORT_WORD1_SWIZ_SEL_Y(word1);
	output->swizzle_z = G_SQ_CF_ALLOC_EXPORT_WORD1_SWIZ_SEL_Z(word1);
	output->swizzle_w = G_SQ_CF_ALLOC_EXPORT_WORD1_SWIZ_SEL_W(word1);
	output->burst_count = G_SQ_CF_ALLOC_EXPORT_WORD1_BURST_COUNT(word1);
	output->end_of_program = G_SQ_CF_ALLOC_EXPORT_WORD1_END_OF_PROGRAM(word1);
    output->op = r600_isa_cf_by_opcode(bc->isa,
			G_SQ_CF_ALLOC_EXPORT_WORD1_CF_INST(word1), 0);
	output->barrier = G_SQ_CF_ALLOC_EXPORT_WORD1_BARRIER(word1);
	output->array_size = G_SQ_CF_ALLOC_EXPORT_WORD1_BUF_ARRAY_SIZE(word1);
	output->comp_mask = G_SQ_CF_ALLOC_EXPORT_WORD1_BUF_COMP_MASK(word1);
}
#endif
