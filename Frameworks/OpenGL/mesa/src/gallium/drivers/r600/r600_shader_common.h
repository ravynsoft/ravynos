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

#ifndef R600_SHADER_COMMON_H
#define R600_SHADER_COMMON_H

#include "r600_asm.h"

#include "compiler/shader_enums.h"


#ifdef __cplusplus
extern "C" {
#endif

/* Valid shader configurations:
 *
 * API shaders       VS | TCS | TES | GS |pass| PS
 * are compiled as:     |     |     |    |thru|
 *                      |     |     |    |    |
 * Only VS & PS:     VS | --  | --  | -- | -- | PS
 * With GS:          ES | --  | --  | GS | VS | PS
 * With Tessel.:     LS | HS  | VS  | -- | -- | PS
 * With both:        LS | HS  | ES  | GS | VS | PS
 */

struct r600_shader_io {
	gl_varying_slot		varying_slot;
	gl_system_value		system_value; /* Input only */
	gl_frag_result		frag_result;
	unsigned		gpr;
	int			spi_sid;
	unsigned		interpolate;
	unsigned		ij_index;
	unsigned		interpolate_location; //  TGSI_INTERPOLATE_LOC_CENTER, CENTROID, SAMPLE
	unsigned		lds_pos; /* for evergreen */
	unsigned		write_mask;
	int			export_param; /* Output only */
	int			ring_offset;
	unsigned		uses_interpolate_at_centroid;
};

struct r600_shader_atomic {
	unsigned start, end;
	unsigned buffer_id;
	unsigned hw_idx;
};

#define R600_SHADER_MAX_INPUTS (32 /* generic */ + 32 /* patch */ + 16 /* others */)
#define R600_SHADER_MAX_OUTPUTS (32 /* generic */ + 32 /* patch */ + 16 /* others */)

struct r600_shader {
	unsigned		processor_type;
	struct r600_bytecode		bc;
	unsigned		ninput;
	unsigned		noutput;
	unsigned                nhwatomic;
	unsigned		nlds;
	unsigned		nsys_inputs;
	unsigned		highest_export_param;
	struct r600_shader_io	input[R600_SHADER_MAX_INPUTS];
	struct r600_shader_io	output[R600_SHADER_MAX_OUTPUTS];
	struct r600_shader_atomic atomics[8];
	unsigned                nhwatomic_ranges;
	bool			uses_kill;
	bool			fs_write_all;
	bool			two_side;
	bool			needs_scratch_space;
	/* Real number of ps color exports compiled in the bytecode */
	unsigned		nr_ps_color_exports;
	unsigned                ps_color_export_mask;
	unsigned                ps_export_highest;
	/* bit n is set if the shader writes gl_ClipDistance[n] */
	unsigned		cc_dist_mask;
	unsigned		clip_dist_write;
	unsigned                cull_dist_write;
	bool			vs_position_window_space;
	/* flag is set if the shader writes VS_OUT_MISC_VEC (e.g. for PSIZE) */
	bool			vs_out_misc_write;
	bool			vs_out_point_size;
	bool			vs_out_layer;
	bool			vs_out_viewport;
	bool			vs_out_edgeflag;
	bool			has_txq_cube_array_z_comp;
	bool			uses_tex_buffers;
	bool                 gs_prim_id_input;
	bool                 gs_tri_strip_adj_fix;
	uint8_t			ps_conservative_z;

	/* Size in bytes of a data item in the ring(s) (single vertex data).
	   Stages with only one ring items 123 will be set to 0. */
	unsigned		ring_item_sizes[4];

	unsigned		indirect_files;
	unsigned		max_arrays;
	unsigned		num_arrays;
	unsigned		vs_as_es;
	unsigned		vs_as_ls;
	unsigned		vs_as_gs_a;
	unsigned                tes_as_es;
	unsigned                tcs_prim_mode;
	unsigned                num_loops;

	struct r600_shader_array * arrays;

	bool			uses_doubles;
	bool                 uses_atomics;
	bool			uses_images;
	bool			uses_helper_invocation;
	bool			uses_interpolate_at_sample;
	uint8_t                 atomic_base;
	uint8_t			rat_base;
	uint8_t                 image_size_const_offset;
        bool			disable_sb;
};

union r600_shader_key {
	struct {
		unsigned	nr_cbufs:4;
		unsigned        first_atomic_counter:4;
		unsigned        image_size_const_offset:5;
		unsigned	color_two_side:1;
		unsigned	alpha_to_one:1;
		unsigned        apply_sample_id_mask:1;
		unsigned        dual_source_blend:1;
	} ps;
	struct {
		unsigned        first_atomic_counter:4;
		unsigned	as_es:1; /* export shader */
		unsigned	as_ls:1; /* local shader */
		unsigned	as_gs_a:1;
	} vs;
	struct {
		unsigned        first_atomic_counter:4;
		unsigned	as_es:1;
	} tes;
	struct {
		unsigned        first_atomic_counter:4;
		unsigned	prim_mode:3;
	} tcs;
	struct {
		unsigned        first_atomic_counter:4;
		unsigned        tri_strip_adj_fix:1;
	} gs;
};

struct r600_shader_array {
	unsigned gpr_start;
	unsigned gpr_count;
	unsigned comp_mask;
};

#ifdef __cplusplus
}  // extern "C"
#endif


#endif
