/* -*- mesa-c++  -*-
 *
 * Copyright (c) 2018 Collabora LTD
 *
 * Author: Gert Wollny <gert.wollny@collabora.com>
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

#include "r600_dump.h"
#include "r600_shader.h"
#include "tgsi/tgsi_strings.h"

void print_shader_info(FILE *f , int id, struct r600_shader *shader)
{

#define PRINT_INT_MEMBER(NAME) \
   if (shader-> NAME) fprintf(f, "  shader->"  #NAME  "=%d;\n", shader-> NAME)
#define PRINT_UINT_MEMBER(NAME) \
   if (shader-> NAME) fprintf(f, "  shader->"  #NAME  "=%u;\n", (unsigned)shader-> NAME)

#define PRINT_INT_ARRAY_ELM(NAME, ELM) \
   if (shader->NAME[i].ELM) fprintf(f, "  shader->"  #NAME "[%d]." #ELM "=%d;\n", i, (int)shader->NAME[i].ELM)
#define PRINT_UINT_ARRAY_ELM(NAME, ELM) \
   if (shader->NAME[i].ELM) fprintf(f, "  shader->"  #NAME "[%d]." #ELM" =%u;\n", i, (unsigned)shader->NAME[i].ELM)
#define PRINT_BOOL_ARRAY_ELM(NAME, ELM) \
   if (shader->NAME[i].ELM) fprintf(f, "  shader->"  #NAME "[%d]." #ELM "=%s;\n", i, shader->NAME[i].ELM ? "true" : "false")

   fprintf(f, "#include \"gallium/drivers/r600/r600_shader.h\"\n");
   fprintf(f, "void shader_%d_fill_data(struct r600_shader *shader)\n{\n", id);
   fprintf(f, "  memset(shader, 0, sizeof(struct r600_shader));\n");

   PRINT_UINT_MEMBER(processor_type);
   PRINT_UINT_MEMBER(ninput);
   PRINT_UINT_MEMBER(noutput);
   PRINT_UINT_MEMBER(nhwatomic);
   PRINT_UINT_MEMBER(nlds);
   PRINT_UINT_MEMBER(nsys_inputs);
   PRINT_UINT_MEMBER(highest_export_param);

   for (unsigned i = 0; i < shader->ninput; ++i) {
      PRINT_INT_ARRAY_ELM(input, varying_slot);
      PRINT_INT_ARRAY_ELM(input, system_value);
      PRINT_UINT_ARRAY_ELM(input, gpr);
      PRINT_INT_ARRAY_ELM(input, spi_sid);
      PRINT_UINT_ARRAY_ELM(input, interpolate);
      PRINT_UINT_ARRAY_ELM(input, ij_index);
      PRINT_UINT_ARRAY_ELM(input, interpolate_location); //  TGSI_INTERPOLATE_LOC_CENTER, CENTROID, SAMPLE
      PRINT_UINT_ARRAY_ELM(input, lds_pos); /* for evergreen */
      PRINT_INT_ARRAY_ELM(input, ring_offset);
      PRINT_BOOL_ARRAY_ELM(input, uses_interpolate_at_centroid);
   }

   for (unsigned i = 0; i < shader->noutput; ++i) {
      PRINT_INT_ARRAY_ELM(output, varying_slot);
      PRINT_INT_ARRAY_ELM(output, frag_result);
      PRINT_UINT_ARRAY_ELM(output, gpr);
      PRINT_INT_ARRAY_ELM(output, spi_sid);
      PRINT_UINT_ARRAY_ELM(output, write_mask);
      PRINT_INT_ARRAY_ELM(output, export_param);
      PRINT_INT_ARRAY_ELM(output, ring_offset);
   }

   for (unsigned i = 0; i < shader->nhwatomic; ++i) {
      PRINT_UINT_ARRAY_ELM(atomics, start);
      PRINT_UINT_ARRAY_ELM(atomics, end);
      PRINT_UINT_ARRAY_ELM(atomics, buffer_id);
      PRINT_UINT_ARRAY_ELM(atomics, hw_idx);
   }

   PRINT_UINT_MEMBER(nhwatomic_ranges);
   PRINT_UINT_MEMBER(uses_kill);
   PRINT_UINT_MEMBER(fs_write_all);
   PRINT_UINT_MEMBER(two_side);
   PRINT_UINT_MEMBER(needs_scratch_space);
   /* Real number of ps color exports compiled in the bytecode */
   PRINT_UINT_MEMBER(nr_ps_color_exports);
   PRINT_UINT_MEMBER(ps_color_export_mask);
   PRINT_UINT_MEMBER(ps_export_highest);
   /* bit n is set if the shader writes gl_ClipDistance[n] */
   PRINT_UINT_MEMBER(cc_dist_mask);
   PRINT_UINT_MEMBER(clip_dist_write);
   PRINT_UINT_MEMBER(cull_dist_write);
   PRINT_UINT_MEMBER(vs_position_window_space);
   /* flag is set if the shader writes VS_OUT_MISC_VEC (e.g. for PSIZE) */
   PRINT_UINT_MEMBER(vs_out_misc_write);
   PRINT_UINT_MEMBER(vs_out_point_size);
   PRINT_UINT_MEMBER(vs_out_layer);
   PRINT_UINT_MEMBER(vs_out_viewport);
   PRINT_UINT_MEMBER(vs_out_edgeflag);
   PRINT_UINT_MEMBER(has_txq_cube_array_z_comp);
   PRINT_UINT_MEMBER(uses_tex_buffers);
   PRINT_UINT_MEMBER(gs_prim_id_input);
   PRINT_UINT_MEMBER(gs_tri_strip_adj_fix);
   PRINT_UINT_MEMBER(ps_conservative_z);

   /* Size in bytes of a data item in the ring(s) (single vertex data).
      Stages with only one ring items 123 will be set to 0. */

   PRINT_UINT_MEMBER(ring_item_sizes[0]);
   PRINT_UINT_MEMBER(ring_item_sizes[1]);
   PRINT_UINT_MEMBER(ring_item_sizes[2]);
   PRINT_UINT_MEMBER(ring_item_sizes[3]);

   PRINT_UINT_MEMBER(indirect_files);
   PRINT_UINT_MEMBER(max_arrays);
   PRINT_UINT_MEMBER(num_arrays);
   PRINT_UINT_MEMBER(vs_as_es);
   PRINT_UINT_MEMBER(vs_as_ls);
   PRINT_UINT_MEMBER(vs_as_gs_a);
   PRINT_UINT_MEMBER(tes_as_es);
   PRINT_UINT_MEMBER(tcs_prim_mode);

   if (shader->num_arrays > 0) {
      fprintf(stderr, "  shader->arrays = new r600_shader_array[%d];\n", shader->num_arrays);
      for (unsigned i = 0; i  < shader->num_arrays; ++i) {
         PRINT_UINT_ARRAY_ELM(arrays, gpr_start);
         PRINT_UINT_ARRAY_ELM(arrays, gpr_count);
         PRINT_UINT_ARRAY_ELM(arrays, comp_mask);
      }
   }

   PRINT_UINT_MEMBER(uses_doubles);
   PRINT_UINT_MEMBER(uses_atomics);
   PRINT_UINT_MEMBER(uses_images);
   PRINT_UINT_MEMBER(uses_helper_invocation);
   PRINT_UINT_MEMBER(atomic_base);
   PRINT_UINT_MEMBER(rat_base);
   PRINT_UINT_MEMBER(image_size_const_offset);

   fprintf(f, "}\n");
}

void print_pipe_info(FILE *f, struct tgsi_shader_info *shader)
{
   PRINT_UINT_MEMBER(shader_buffers_load);
   PRINT_UINT_MEMBER(shader_buffers_store);
   PRINT_UINT_MEMBER(shader_buffers_atomic);
   PRINT_UINT_MEMBER(writes_memory);
   PRINT_UINT_MEMBER(file_mask[TGSI_FILE_HW_ATOMIC]);
   PRINT_UINT_MEMBER(file_count[TGSI_FILE_HW_ATOMIC]);

   for(unsigned int i = 0; i < TGSI_PROPERTY_COUNT; ++i) {
      if (shader->properties[i] != 0)
	 fprintf(stderr, "PROP: %s = %d\n", tgsi_property_names[i], shader->properties[i]);
   }

#define PRINT_UINT_ARRAY_MEMBER(M, IDX) \
   if (shader-> M [ IDX ])  fprintf(f, #M "[%d] = %d\n",  IDX, (unsigned) shader-> M [ IDX ]);

   for (int i = 0; i < shader->num_inputs; ++i) {
      PRINT_UINT_ARRAY_MEMBER(input_semantic_name, i); /**< TGSI_SEMANTIC_x */
      PRINT_UINT_ARRAY_MEMBER(input_semantic_index, i);
      PRINT_UINT_ARRAY_MEMBER(input_interpolate, i);
      PRINT_UINT_ARRAY_MEMBER(input_interpolate_loc, i);
      PRINT_UINT_ARRAY_MEMBER(input_usage_mask, i);
   }

   for (int i = 0; i < shader->num_outputs; ++i) {
      PRINT_UINT_ARRAY_MEMBER(output_semantic_name, i);
      PRINT_UINT_ARRAY_MEMBER(output_semantic_index, i);
      PRINT_UINT_ARRAY_MEMBER(output_usagemask, i);
      PRINT_UINT_ARRAY_MEMBER(output_streams, i);
   }

   for (int i = 0; i < shader->num_system_values; ++i)
      PRINT_UINT_ARRAY_MEMBER(system_value_semantic_name, i);

   PRINT_UINT_MEMBER(reads_pervertex_outputs);
   PRINT_UINT_MEMBER(reads_perpatch_outputs);
   PRINT_UINT_MEMBER(reads_tessfactor_outputs);
}
