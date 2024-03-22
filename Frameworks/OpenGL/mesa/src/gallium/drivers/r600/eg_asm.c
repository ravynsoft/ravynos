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
#include "r600_opcodes.h"
#include "r600_shader_common.h"

#include "util/u_memory.h"
#include "eg_sq.h"
#include <errno.h>

int eg_bytecode_cf_build(struct r600_bytecode *bc, struct r600_bytecode_cf *cf)
{
	unsigned id = cf->id;

	if (cf->op == CF_NATIVE) {
		bc->bytecode[id++] = cf->isa[0];
		bc->bytecode[id++] = cf->isa[1];
	} else {
		const struct cf_op_info *cfop = r600_isa_cf(cf->op);
		unsigned opcode = r600_isa_cf_opcode(bc->isa->hw_class, cf->op);
		if (cfop->flags & CF_ALU) { /* ALU clauses */

			/* prepend ALU_EXTENDED if we need more than 2 kcache sets */
			if (cf->eg_alu_extended) {
				bc->bytecode[id++] =
						S_SQ_CF_ALU_WORD0_EXT_KCACHE_BANK_INDEX_MODE0(cf->kcache[0].index_mode) |
						S_SQ_CF_ALU_WORD0_EXT_KCACHE_BANK_INDEX_MODE1(cf->kcache[1].index_mode) |
						S_SQ_CF_ALU_WORD0_EXT_KCACHE_BANK_INDEX_MODE2(cf->kcache[2].index_mode) |
						S_SQ_CF_ALU_WORD0_EXT_KCACHE_BANK_INDEX_MODE3(cf->kcache[3].index_mode) |
						S_SQ_CF_ALU_WORD0_EXT_KCACHE_BANK2(cf->kcache[2].bank) |
						S_SQ_CF_ALU_WORD0_EXT_KCACHE_BANK3(cf->kcache[3].bank) |
						S_SQ_CF_ALU_WORD0_EXT_KCACHE_MODE2(cf->kcache[2].mode);
				bc->bytecode[id++] =
						S_SQ_CF_ALU_WORD1_EXT_CF_INST(
							r600_isa_cf_opcode(bc->isa->hw_class, CF_OP_ALU_EXT)) |
						S_SQ_CF_ALU_WORD1_EXT_KCACHE_MODE3(cf->kcache[3].mode) |
						S_SQ_CF_ALU_WORD1_EXT_KCACHE_ADDR2(cf->kcache[2].addr) |
						S_SQ_CF_ALU_WORD1_EXT_KCACHE_ADDR3(cf->kcache[3].addr) |
						S_SQ_CF_ALU_WORD1_EXT_BARRIER(1);
			}
			bc->bytecode[id++] = S_SQ_CF_ALU_WORD0_ADDR(cf->addr >> 1) |
					S_SQ_CF_ALU_WORD0_KCACHE_MODE0(cf->kcache[0].mode) |
					S_SQ_CF_ALU_WORD0_KCACHE_BANK0(cf->kcache[0].bank) |
					S_SQ_CF_ALU_WORD0_KCACHE_BANK1(cf->kcache[1].bank);
			bc->bytecode[id++] = S_SQ_CF_ALU_WORD1_CF_INST(opcode) |
					S_SQ_CF_ALU_WORD1_KCACHE_MODE1(cf->kcache[1].mode) |
					S_SQ_CF_ALU_WORD1_KCACHE_ADDR0(cf->kcache[0].addr) |
					S_SQ_CF_ALU_WORD1_KCACHE_ADDR1(cf->kcache[1].addr) |
					S_SQ_CF_ALU_WORD1_BARRIER(1) |
					S_SQ_CF_ALU_WORD1_COUNT((cf->ndw / 2) - 1);
		} else if (cfop->flags & CF_CLAUSE) {
			/* CF_TEX/VTX (CF_ALU already handled above) */
			bc->bytecode[id++] = S_SQ_CF_WORD0_ADDR(cf->addr >> 1);
			bc->bytecode[id] = S_SQ_CF_WORD1_CF_INST(opcode) |
					S_SQ_CF_WORD1_BARRIER(1) |
					S_SQ_CF_WORD1_VALID_PIXEL_MODE(cf->vpm) |
					S_SQ_CF_WORD1_COUNT((cf->ndw / 4) - 1);
			if (bc->gfx_level == EVERGREEN) /* no EOP on cayman */
				bc->bytecode[id] |= S_SQ_CF_ALLOC_EXPORT_WORD1_END_OF_PROGRAM(cf->end_of_program);
			id++;
		} else if (cfop->flags & CF_EXP) {
			/* EXPORT instructions */
			bc->bytecode[id++] = S_SQ_CF_ALLOC_EXPORT_WORD0_RW_GPR(cf->output.gpr) |
					S_SQ_CF_ALLOC_EXPORT_WORD0_ELEM_SIZE(cf->output.elem_size) |
					S_SQ_CF_ALLOC_EXPORT_WORD0_ARRAY_BASE(cf->output.array_base) |
					S_SQ_CF_ALLOC_EXPORT_WORD0_TYPE(cf->output.type) |
					S_SQ_CF_ALLOC_EXPORT_WORD0_INDEX_GPR(cf->output.index_gpr);
			bc->bytecode[id] =
					S_SQ_CF_ALLOC_EXPORT_WORD1_BURST_COUNT(cf->output.burst_count - 1) |
					S_SQ_CF_ALLOC_EXPORT_WORD1_SWIZ_SEL_X(cf->output.swizzle_x) |
					S_SQ_CF_ALLOC_EXPORT_WORD1_SWIZ_SEL_Y(cf->output.swizzle_y) |
					S_SQ_CF_ALLOC_EXPORT_WORD1_SWIZ_SEL_Z(cf->output.swizzle_z) |
					S_SQ_CF_ALLOC_EXPORT_WORD1_SWIZ_SEL_W(cf->output.swizzle_w) |
					S_SQ_CF_ALLOC_EXPORT_WORD1_BARRIER(cf->barrier) |
					S_SQ_CF_ALLOC_EXPORT_WORD1_MARK(cf->mark) |
					S_SQ_CF_ALLOC_EXPORT_WORD1_CF_INST(opcode);

			if (bc->gfx_level == EVERGREEN) /* no EOP on cayman */
				bc->bytecode[id] |= S_SQ_CF_ALLOC_EXPORT_WORD1_END_OF_PROGRAM(cf->end_of_program);
			id++;
		} else if (cfop->flags & CF_RAT) {
			bc->bytecode[id++] = S_SQ_CF_ALLOC_EXPORT_WORD0_RAT_RAT_ID(cf->rat.id) |
					S_SQ_CF_ALLOC_EXPORT_WORD0_RAT_RAT_INST(cf->rat.inst) |
					S_SQ_CF_ALLOC_EXPORT_WORD0_RAT_RAT_INDEX_MODE(cf->rat.index_mode) |
					S_SQ_CF_ALLOC_EXPORT_WORD0_RW_GPR(cf->output.gpr) |
					S_SQ_CF_ALLOC_EXPORT_WORD0_ELEM_SIZE(cf->output.elem_size) |
					S_SQ_CF_ALLOC_EXPORT_WORD0_TYPE(cf->output.type) |
					S_SQ_CF_ALLOC_EXPORT_WORD0_INDEX_GPR(cf->output.index_gpr);
			bc->bytecode[id] = S_SQ_CF_ALLOC_EXPORT_WORD1_BURST_COUNT(cf->output.burst_count - 1) |
					S_SQ_CF_ALLOC_EXPORT_WORD1_BARRIER(cf->barrier) |
					S_SQ_CF_ALLOC_EXPORT_WORD1_MARK(cf->mark) |
			                S_SQ_CF_ALLOC_EXPORT_WORD1_VALID_PIXEL_MODE(cf->vpm) |
					S_SQ_CF_ALLOC_EXPORT_WORD1_CF_INST(opcode) |
					S_SQ_CF_ALLOC_EXPORT_WORD1_BUF_COMP_MASK(cf->output.comp_mask) |
					S_SQ_CF_ALLOC_EXPORT_WORD1_BUF_ARRAY_SIZE(cf->output.array_size) |
					S_SQ_CF_ALLOC_EXPORT_WORD1_MARK(cf->output.mark);
			if (bc->gfx_level == EVERGREEN) /* no EOP on cayman */
				bc->bytecode[id] |= S_SQ_CF_ALLOC_EXPORT_WORD1_END_OF_PROGRAM(cf->end_of_program);
			id++;

		} else if (cfop->flags & CF_MEM) {
			/* MEM_STREAM, MEM_RING instructions */
			bc->bytecode[id++] = S_SQ_CF_ALLOC_EXPORT_WORD0_RW_GPR(cf->output.gpr) |
					S_SQ_CF_ALLOC_EXPORT_WORD0_ELEM_SIZE(cf->output.elem_size) |
					S_SQ_CF_ALLOC_EXPORT_WORD0_ARRAY_BASE(cf->output.array_base) |
					S_SQ_CF_ALLOC_EXPORT_WORD0_TYPE(cf->output.type) |
					S_SQ_CF_ALLOC_EXPORT_WORD0_INDEX_GPR(cf->output.index_gpr);
			bc->bytecode[id] = S_SQ_CF_ALLOC_EXPORT_WORD1_BURST_COUNT(cf->output.burst_count - 1) |
					S_SQ_CF_ALLOC_EXPORT_WORD1_BARRIER(cf->barrier) |
					S_SQ_CF_ALLOC_EXPORT_WORD1_MARK(cf->mark) |
					S_SQ_CF_ALLOC_EXPORT_WORD1_CF_INST(opcode) |
					S_SQ_CF_ALLOC_EXPORT_WORD1_BUF_COMP_MASK(cf->output.comp_mask) |
					S_SQ_CF_ALLOC_EXPORT_WORD1_BUF_ARRAY_SIZE(cf->output.array_size);
			if (bc->gfx_level == EVERGREEN) /* no EOP on cayman */
				bc->bytecode[id] |= S_SQ_CF_ALLOC_EXPORT_WORD1_END_OF_PROGRAM(cf->end_of_program);
			id++;
		} else {
			/* other instructions */
			bc->bytecode[id++] = S_SQ_CF_WORD0_ADDR(cf->cf_addr >> 1);
			bc->bytecode[id] = S_SQ_CF_WORD1_CF_INST(opcode) |
					S_SQ_CF_WORD1_VALID_PIXEL_MODE(cf->vpm) |
					S_SQ_CF_WORD1_BARRIER(1) |
					S_SQ_CF_WORD1_COND(cf->cond) |
					S_SQ_CF_WORD1_POP_COUNT(cf->pop_count) |
					S_SQ_CF_WORD1_COUNT(cf->count);
			if (bc->gfx_level == EVERGREEN) /* no EOP on cayman */
				bc->bytecode[id] |= S_SQ_CF_ALLOC_EXPORT_WORD1_END_OF_PROGRAM(cf->end_of_program);
			id++;
		}
	}
	return 0;
}

#if 0
void eg_bytecode_export_read(struct r600_bytecode *bc,
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
			G_SQ_CF_ALLOC_EXPORT_WORD1_CF_INST(word1), /* is_cf_alu = */ 0 );
	output->barrier = G_SQ_CF_ALLOC_EXPORT_WORD1_BARRIER(word1);
	output->array_size = G_SQ_CF_ALLOC_EXPORT_WORD1_BUF_ARRAY_SIZE(word1);
	output->comp_mask = G_SQ_CF_ALLOC_EXPORT_WORD1_BUF_COMP_MASK(word1);
}
#endif

int eg_bytecode_gds_build(struct r600_bytecode *bc, struct r600_bytecode_gds *gds, unsigned id)
{
	unsigned gds_op = (r600_isa_fetch_opcode(bc->isa->hw_class, gds->op) >> 8) & 0x3f;
	unsigned opcode;
	if (gds->op == FETCH_OP_TF_WRITE) {
		opcode = 5;
		gds_op = 0;
	} else
		opcode = 4;
	bc->bytecode[id++] = S_SQ_MEM_GDS_WORD0_MEM_INST(2) |
		S_SQ_MEM_GDS_WORD0_MEM_OP(opcode) |
		S_SQ_MEM_GDS_WORD0_SRC_GPR(gds->src_gpr) |
		S_SQ_MEM_GDS_WORD0_SRC_REL(gds->src_rel) |
		S_SQ_MEM_GDS_WORD0_SRC_SEL_X(gds->src_sel_x) |
		S_SQ_MEM_GDS_WORD0_SRC_SEL_Y(gds->src_sel_y) |
		S_SQ_MEM_GDS_WORD0_SRC_SEL_Z(gds->src_sel_z);

	bc->bytecode[id++] = S_SQ_MEM_GDS_WORD1_DST_GPR(gds->dst_gpr) |
		S_SQ_MEM_GDS_WORD1_DST_REL(gds->dst_rel) |
		S_SQ_MEM_GDS_WORD1_GDS_OP(gds_op) |
		S_SQ_MEM_GDS_WORD1_SRC_GPR(gds->src_gpr2) |
		S_SQ_MEM_GDS_WORD1_UAV_INDEX_MODE(gds->uav_index_mode) |
		S_SQ_MEM_GDS_WORD1_UAV_ID(gds->uav_id) |
		S_SQ_MEM_GDS_WORD1_ALLOC_CONSUME(gds->alloc_consume) |
		S_SQ_MEM_GDS_WORD1_BCAST_FIRST_REQ(gds->bcast_first_req);

	bc->bytecode[id++] = S_SQ_MEM_GDS_WORD2_DST_SEL_X(gds->dst_sel_x) |
		S_SQ_MEM_GDS_WORD2_DST_SEL_Y(gds->dst_sel_y) |
		S_SQ_MEM_GDS_WORD2_DST_SEL_Z(gds->dst_sel_z) |
		S_SQ_MEM_GDS_WORD2_DST_SEL_W(gds->dst_sel_w);
	return 0;
}

int eg_bytecode_alu_build(struct r600_bytecode *bc, struct r600_bytecode_alu *alu, unsigned id)
{
	if (alu->is_lds_idx_op) {
		assert(!alu->src[0].abs && !alu->src[1].abs && !alu->src[2].abs);
		assert(!alu->src[0].neg && !alu->src[1].neg && !alu->src[2].neg);
		bc->bytecode[id++] = S_SQ_ALU_WORD0_SRC0_SEL(alu->src[0].sel) |
			S_SQ_ALU_WORD0_SRC0_REL(alu->src[0].rel) |
			S_SQ_ALU_WORD0_SRC0_CHAN(alu->src[0].chan) |
			S_SQ_ALU_WORD0_LDS_IDX_OP_IDX_OFFSET_4(alu->lds_idx >> 4) |
			S_SQ_ALU_WORD0_SRC1_SEL(alu->src[1].sel) |
			S_SQ_ALU_WORD0_SRC1_REL(alu->src[1].rel) |
			S_SQ_ALU_WORD0_SRC1_CHAN(alu->src[1].chan) |
			S_SQ_ALU_WORD0_LDS_IDX_OP_IDX_OFFSET_5(alu->lds_idx >> 5) |
			S_SQ_ALU_WORD0_INDEX_MODE(alu->index_mode) |
			S_SQ_ALU_WORD0_PRED_SEL(alu->pred_sel) |
			S_SQ_ALU_WORD0_LAST(alu->last);
	} else {
		bc->bytecode[id++] = S_SQ_ALU_WORD0_SRC0_SEL(alu->src[0].sel) |
			S_SQ_ALU_WORD0_SRC0_REL(alu->src[0].rel) |
			S_SQ_ALU_WORD0_SRC0_CHAN(alu->src[0].chan) |
			S_SQ_ALU_WORD0_SRC0_NEG(alu->src[0].neg) |
			S_SQ_ALU_WORD0_SRC1_SEL(alu->src[1].sel) |
			S_SQ_ALU_WORD0_SRC1_REL(alu->src[1].rel) |
			S_SQ_ALU_WORD0_SRC1_CHAN(alu->src[1].chan) |
			S_SQ_ALU_WORD0_SRC1_NEG(alu->src[1].neg) |
			S_SQ_ALU_WORD0_PRED_SEL(alu->pred_sel) |
			S_SQ_ALU_WORD0_LAST(alu->last);
	}

	/* don't replace gpr by pv or ps for destination register */
	if (alu->is_lds_idx_op) {
		unsigned lds_op = r600_isa_alu_opcode(bc->isa->hw_class, alu->op);
		bc->bytecode[id++] =
			S_SQ_ALU_WORD1_OP3_SRC2_SEL(alu->src[2].sel) |
			S_SQ_ALU_WORD1_OP3_SRC2_REL(alu->src[2].rel) |
			S_SQ_ALU_WORD1_OP3_SRC2_CHAN(alu->src[2].chan) |
			S_SQ_ALU_WORD1_LDS_IDX_OP_IDX_OFFSET_1(alu->lds_idx >> 1) |

			S_SQ_ALU_WORD1_OP3_ALU_INST(lds_op & 0xff) |
			S_SQ_ALU_WORD1_BANK_SWIZZLE(alu->bank_swizzle) |
			S_SQ_ALU_WORD1_LDS_IDX_OP_LDS_OP((lds_op >> 8) & 0xff) |
			S_SQ_ALU_WORD1_LDS_IDX_OP_IDX_OFFSET_0(alu->lds_idx) |
			S_SQ_ALU_WORD1_LDS_IDX_OP_IDX_OFFSET_2(alu->lds_idx >> 2) |
			S_SQ_ALU_WORD1_DST_CHAN(alu->dst.chan) |
			S_SQ_ALU_WORD1_LDS_IDX_OP_IDX_OFFSET_3(alu->lds_idx >> 3);

	} else if (alu->is_op3) {
		assert(!alu->src[0].abs && !alu->src[1].abs && !alu->src[2].abs);
		bc->bytecode[id++] = S_SQ_ALU_WORD1_DST_GPR(alu->dst.sel) |
					S_SQ_ALU_WORD1_DST_CHAN(alu->dst.chan) |
			                S_SQ_ALU_WORD1_DST_REL(alu->dst.rel) |
			                S_SQ_ALU_WORD1_CLAMP(alu->dst.clamp) |
					S_SQ_ALU_WORD1_OP3_SRC2_SEL(alu->src[2].sel) |
					S_SQ_ALU_WORD1_OP3_SRC2_REL(alu->src[2].rel) |
					S_SQ_ALU_WORD1_OP3_SRC2_CHAN(alu->src[2].chan) |
					S_SQ_ALU_WORD1_OP3_SRC2_NEG(alu->src[2].neg) |
					S_SQ_ALU_WORD1_OP3_ALU_INST(r600_isa_alu_opcode(bc->isa->hw_class, alu->op)) |
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
					S_SQ_ALU_WORD1_OP2_ALU_INST(r600_isa_alu_opcode(bc->isa->hw_class, alu->op)) |
					S_SQ_ALU_WORD1_BANK_SWIZZLE(alu->bank_swizzle) |
			                S_SQ_ALU_WORD1_OP2_UPDATE_EXECUTE_MASK(alu->execute_mask) |
			                S_SQ_ALU_WORD1_OP2_UPDATE_PRED(alu->update_pred);
	}
	return 0;
}
