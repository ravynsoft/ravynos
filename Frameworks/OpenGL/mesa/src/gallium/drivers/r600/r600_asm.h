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
#ifndef R600_ASM_H
#define R600_ASM_H

#include "util/format/u_format.h"
#include "util/list.h"
#include "amd_family.h"
#include "r600_isa.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#define R600_ASM_ERR(fmt, args...) \
	fprintf(stderr, "EE %s:%d %s - " fmt, __FILE__, __LINE__, __func__, ##args)

struct r600_bytecode_alu_src {
	unsigned			sel;
	unsigned			chan;
	unsigned			neg;
	unsigned			abs;
	unsigned			rel;
	unsigned			kc_bank;
	unsigned			kc_rel;
	uint32_t			value;
};

struct r600_bytecode_alu_dst {
	unsigned			sel;
	unsigned			chan;
	unsigned			clamp;
	unsigned			write;
	unsigned			rel;
};

struct r600_bytecode_alu {
	struct list_head		list;
	struct r600_bytecode_alu_src		src[3];
	struct r600_bytecode_alu_dst		dst;
	unsigned			op;
	unsigned			last;
	unsigned			is_op3;
	unsigned			is_lds_idx_op;
	unsigned			execute_mask;
	unsigned			update_pred;
	unsigned			pred_sel;
	unsigned			bank_swizzle;
	unsigned			bank_swizzle_force;
	unsigned			omod;
	unsigned                        index_mode;
	unsigned                        lds_idx;
};

struct r600_bytecode_tex {
	struct list_head		list;
	unsigned			op;
	unsigned			inst_mod;
	unsigned			resource_id;
	unsigned			src_gpr;
	unsigned			src_rel;
	unsigned			dst_gpr;
	unsigned			dst_rel;
	unsigned			dst_sel_x;
	unsigned			dst_sel_y;
	unsigned			dst_sel_z;
	unsigned			dst_sel_w;
	unsigned			lod_bias;
	unsigned			coord_type_x;
	unsigned			coord_type_y;
	unsigned			coord_type_z;
	unsigned			coord_type_w;
	int				offset_x;
	int				offset_y;
	int				offset_z;
	unsigned			sampler_id;
	unsigned			src_sel_x;
	unsigned			src_sel_y;
	unsigned			src_sel_z;
	unsigned			src_sel_w;
	/* indexed samplers/resources only on evergreen/cayman */
	unsigned			sampler_index_mode;
	unsigned			resource_index_mode;
};

struct r600_bytecode_vtx {
	struct list_head		list;
	unsigned			op;
	unsigned			fetch_type;
	unsigned			buffer_id;
	unsigned			src_gpr;
	unsigned			src_sel_x;
	unsigned			mega_fetch_count;
	unsigned			dst_gpr;
	unsigned			dst_sel_x;
	unsigned			dst_sel_y;
	unsigned			dst_sel_z;
	unsigned			dst_sel_w;
	unsigned			use_const_fields;
	unsigned			data_format;
	unsigned			num_format_all;
	unsigned			format_comp_all;
	unsigned			srf_mode_all;
	unsigned			offset;
	unsigned			endian;
	unsigned			buffer_index_mode;

	// READ_SCRATCH fields
	unsigned			uncached;
	unsigned			indexed;
	unsigned			src_sel_y;
	unsigned			src_rel;
	unsigned			elem_size;
	unsigned			array_size;
	unsigned			array_base;
	unsigned			burst_count;
	unsigned			dst_rel;
};

struct r600_bytecode_gds {
	struct list_head		list;
	unsigned			op;
	unsigned			src_gpr;
	unsigned			src_rel;
	unsigned			src_sel_x;
	unsigned			src_sel_y;
	unsigned			src_sel_z;
	unsigned			src_gpr2;
	unsigned			dst_gpr;
	unsigned			dst_rel;
	unsigned			dst_sel_x;
	unsigned			dst_sel_y;
	unsigned			dst_sel_z;
	unsigned			dst_sel_w;
	unsigned			uav_index_mode;
	unsigned                        uav_id;
	unsigned                        alloc_consume;
	unsigned                        bcast_first_req;
};

struct r600_bytecode_output {
	unsigned			array_base;
	unsigned			array_size;
	unsigned			comp_mask;
	unsigned			type;

	unsigned			op;

	unsigned			elem_size;
	unsigned			gpr;
	unsigned			swizzle_x;
	unsigned			swizzle_y;
	unsigned			swizzle_z;
	unsigned			swizzle_w;
	unsigned			burst_count;
	unsigned			index_gpr;
	unsigned			mark; /* used by MEM_SCRATCH */
};

struct r600_bytecode_rat {
	unsigned			id;
	unsigned			inst;
	unsigned			index_mode;
};

struct r600_bytecode_kcache {
	unsigned			bank;
	unsigned			mode;
	unsigned			addr;
	unsigned			index_mode;
};

struct r600_bytecode_cf {
	struct list_head		list;

	unsigned			op;
	unsigned			addr;
	unsigned			ndw;
	unsigned			id;
	unsigned			cond;
	unsigned			pop_count;
	unsigned			count;
	unsigned			cf_addr; /* control flow addr */
	struct r600_bytecode_kcache		kcache[4];
	unsigned			r6xx_uses_waterfall;
	unsigned			eg_alu_extended;
	unsigned			barrier;
	unsigned			end_of_program;
	unsigned                        mark;
	unsigned                        vpm;
	struct list_head		alu;
	struct list_head		tex;
	struct list_head		vtx;
	struct list_head		gds;
	struct r600_bytecode_output		output;
	struct r600_bytecode_rat		rat;
	struct r600_bytecode_alu		*curr_bs_head;
	struct r600_bytecode_alu		*prev_bs_head;
	struct r600_bytecode_alu		*prev2_bs_head;
	unsigned isa[2];
	unsigned nlds_read;
	unsigned nqueue_read;
	unsigned clause_local_written;
};

#define FC_NONE				0
#define FC_IF				1
#define FC_LOOP				2
#define FC_REP				3
#define FC_PUSH_VPM			4
#define FC_PUSH_WQM			5

struct r600_cf_stack_entry {
	int				type;
	struct r600_bytecode_cf		*start;
	struct r600_bytecode_cf		**mid; /* used to store the else point */
	int				num_mid;
};

#define SQ_MAX_CALL_DEPTH 0x00000020

#define AR_HANDLE_NORMAL 0
#define AR_HANDLE_RV6XX 1 /* except RV670 */

struct r600_stack_info {
	/* current level of non-WQM PUSH operations
	 * (PUSH, PUSH_ELSE, ALU_PUSH_BEFORE) */
	int push;
	/* current level of WQM PUSH operations
	 * (PUSH, PUSH_ELSE, PUSH_WQM) */
	int push_wqm;
	/* current loop level */
	int loop;

	/* required depth */
	int max_entries;
	/* subentries per entry */
	int entry_size;
};

struct r600_bytecode {
	enum amd_gfx_level			gfx_level;
	enum radeon_family		family;
	bool				has_compressed_msaa_texturing;
	int				type;
	struct list_head		cf;
	struct r600_bytecode_cf		*cf_last;
	unsigned			ndw;
	unsigned			ncf;
	unsigned			nalu_groups;
	unsigned			ngpr;
	unsigned			nstack;
	unsigned			nlds_dw;
	unsigned			nresource;
	unsigned			force_add_cf;
	uint32_t			*bytecode;
	uint32_t			fc_sp;
	struct r600_cf_stack_entry	fc_stack[256];
	struct r600_stack_info		stack;
	unsigned	ar_loaded;
	unsigned	ar_reg;
	unsigned	ar_chan;
	unsigned        ar_handling;
	unsigned        r6xx_nop_after_rel_dst;
	bool            index_loaded[2];
	unsigned        index_reg[2]; /* indexing register CF_INDEX_[01] */
	unsigned        index_reg_chan[2]; /* indexing register channel CF_INDEX_[01] */
	unsigned        debug_id;
	struct r600_isa* isa;
	struct r600_bytecode_output pending_outputs[5];
	int n_pending_outputs;
	bool			need_wait_ack; /* emit a pending WAIT_ACK prior to control flow */
	bool			precise;
};

/* eg_asm.c */
int eg_bytecode_cf_build(struct r600_bytecode *bc, struct r600_bytecode_cf *cf);
int eg_bytecode_gds_build(struct r600_bytecode *bc, struct r600_bytecode_gds *gds, unsigned id);
int eg_bytecode_alu_build(struct r600_bytecode *bc,
			  struct r600_bytecode_alu *alu, unsigned id);
/* r600_asm.c */
void r600_bytecode_init(struct r600_bytecode *bc,
			enum amd_gfx_level gfx_level,
			enum radeon_family family,
			bool has_compressed_msaa_texturing);
void r600_bytecode_clear(struct r600_bytecode *bc);
int r600_bytecode_add_alu(struct r600_bytecode *bc,
		const struct r600_bytecode_alu *alu);
int r600_bytecode_add_vtx(struct r600_bytecode *bc,
		const struct r600_bytecode_vtx *vtx);
int r600_bytecode_add_vtx_tc(struct r600_bytecode *bc,
			     const struct r600_bytecode_vtx *vtx);
int r600_bytecode_add_tex(struct r600_bytecode *bc,
		const struct r600_bytecode_tex *tex);
int r600_bytecode_add_gds(struct r600_bytecode *bc,
		const struct r600_bytecode_gds *gds);
int r600_bytecode_add_output(struct r600_bytecode *bc,
		const struct r600_bytecode_output *output);
int r600_bytecode_add_pending_output(struct r600_bytecode *bc,
		const struct r600_bytecode_output *output);

void r600_bytecode_add_ack(struct r600_bytecode *bc);
int r600_bytecode_wait_acks(struct r600_bytecode *bc);
uint32_t r600_bytecode_write_export_ack_type(struct r600_bytecode *bc, bool indirect);

int r600_bytecode_build(struct r600_bytecode *bc);
int r600_bytecode_add_cf(struct r600_bytecode *bc);
int r600_bytecode_add_cfinst(struct r600_bytecode *bc,
		unsigned op);
int r600_bytecode_add_alu_type(struct r600_bytecode *bc,
		const struct r600_bytecode_alu *alu, unsigned type);
void r600_bytecode_special_constants(uint32_t value, unsigned *sel);
void r600_bytecode_disasm(struct r600_bytecode *bc);
void r600_bytecode_alu_read(struct r600_bytecode *bc,
		struct r600_bytecode_alu *alu, uint32_t word0, uint32_t word1);
int r600_load_ar(struct r600_bytecode *bc, bool for_src);

int cm_bytecode_add_cf_end(struct r600_bytecode *bc);

/* r700_asm.c */
void r700_bytecode_cf_vtx_build(uint32_t *bytecode,
		const struct r600_bytecode_cf *cf);
int r700_bytecode_alu_build(struct r600_bytecode *bc,
		struct r600_bytecode_alu *alu, unsigned id);
void r700_bytecode_alu_read(struct r600_bytecode *bc,
		struct r600_bytecode_alu *alu, uint32_t word0, uint32_t word1);
int r700_bytecode_fetch_mem_build(struct r600_bytecode *bc,
		struct r600_bytecode_vtx *mem, unsigned id);

void r600_bytecode_export_read(struct r600_bytecode *bc,
		struct r600_bytecode_output *output, uint32_t word0, uint32_t word1);
void eg_bytecode_export_read(struct r600_bytecode *bc,
		struct r600_bytecode_output *output, uint32_t word0, uint32_t word1);

void r600_vertex_data_type(enum pipe_format pformat, unsigned *format,
			   unsigned *num_format, unsigned *format_comp, unsigned *endian);

int r600_load_ar(struct r600_bytecode *bc, bool for_src);

static inline int fp64_switch(int i)
{
	switch (i) {
	case 0:
		return 1;
	case 1:
		return 0;
	case 2:
		return 3;
	case 3:
		return 2;
	}
	return 0;
}

#ifdef __cplusplus
}
#endif

#endif
