/*
 Copyright (C) Intel Corp.  2006.  All Rights Reserved.
 Intel funded Tungsten Graphics to
 develop this 3D driver.

 Permission is hereby granted, free of charge, to any person obtaining
 a copy of this software and associated documentation files (the
 "Software"), to deal in the Software without restriction, including
 without limitation the rights to use, copy, modify, merge, publish,
 distribute, sublicense, and/or sell copies of the Software, and to
 permit persons to whom the Software is furnished to do so, subject to
 the following conditions:

 The above copyright notice and this permission notice (including the
 next paragraph) shall be included in all copies or substantial
 portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 IN NO EVENT SHALL THE COPYRIGHT OWNER(S) AND/OR ITS SUPPLIERS BE
 LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

 **********************************************************************/
 /*
  * Authors:
  *   Keith Whitwell <keithw@vmware.com>
  */

#include "brw_compiler.h"
#include "brw_eu.h"
#include "brw_prim.h"

#include "dev/intel_debug.h"

#define MAX_GS_VERTS (4)

struct brw_ff_gs_compile {
   struct brw_codegen func;
   struct brw_ff_gs_prog_key key;
   struct brw_ff_gs_prog_data *prog_data;

   struct {
      struct brw_reg R0;

      /**
       * Register holding streamed vertex buffer pointers -- see the Sandy
       * Bridge PRM, volume 2 part 1, section 4.4.2 (GS Thread Payload
       * [DevSNB]).  These pointers are delivered in GRF 1.
       */
      struct brw_reg SVBI;

      struct brw_reg vertex[MAX_GS_VERTS];
      struct brw_reg header;
      struct brw_reg temp;

      /**
       * Register holding destination indices for streamed buffer writes.
       * Only used for SOL programs.
       */
      struct brw_reg destination_indices;
   } reg;

   /* Number of registers used to store vertex data */
   GLuint nr_regs;

   struct brw_vue_map vue_map;
};

/**
 * Allocate registers for GS.
 *
 * If sol_program is true, then:
 *
 * - The thread will be spawned with the "SVBI Payload Enable" bit set, so GRF
 *   1 needs to be set aside to hold the streamed vertex buffer indices.
 *
 * - The thread will need to use the destination_indices register.
 */
static void brw_ff_gs_alloc_regs(struct brw_ff_gs_compile *c,
                                 GLuint nr_verts,
                                 bool sol_program)
{
   GLuint i = 0,j;

   /* Register usage is static, precompute here:
    */
   c->reg.R0 = retype(brw_vec8_grf(i, 0), BRW_REGISTER_TYPE_UD); i++;

   /* Streamed vertex buffer indices */
   if (sol_program)
      c->reg.SVBI = retype(brw_vec8_grf(i++, 0), BRW_REGISTER_TYPE_UD);

   /* Payload vertices plus space for more generated vertices:
    */
   for (j = 0; j < nr_verts; j++) {
      c->reg.vertex[j] = brw_vec4_grf(i, 0);
      i += c->nr_regs;
   }

   c->reg.header = retype(brw_vec8_grf(i++, 0), BRW_REGISTER_TYPE_UD);
   c->reg.temp = retype(brw_vec8_grf(i++, 0), BRW_REGISTER_TYPE_UD);

   if (sol_program) {
      c->reg.destination_indices =
         retype(brw_vec4_grf(i++, 0), BRW_REGISTER_TYPE_UD);
   }

   c->prog_data->urb_read_length = c->nr_regs;
   c->prog_data->total_grf = i;
}


/**
 * Set up the initial value of c->reg.header register based on c->reg.R0.
 *
 * The following information is passed to the GS thread in R0, and needs to be
 * included in the first URB_WRITE or FF_SYNC message sent by the GS:
 *
 * - DWORD 0 [31:0] handle info (Gen4 only)
 * - DWORD 5 [7:0] FFTID
 * - DWORD 6 [31:0] Debug info
 * - DWORD 7 [31:0] Debug info
 *
 * This function sets up the above data by copying by copying the contents of
 * R0 to the header register.
 */
static void brw_ff_gs_initialize_header(struct brw_ff_gs_compile *c)
{
   struct brw_codegen *p = &c->func;
   brw_MOV(p, c->reg.header, c->reg.R0);
}

/**
 * Overwrite DWORD 2 of c->reg.header with the given immediate unsigned value.
 *
 * In URB_WRITE messages, DWORD 2 contains the fields PrimType, PrimStart,
 * PrimEnd, Increment CL_INVOCATIONS, and SONumPrimsWritten, many of which we
 * need to be able to update on a per-vertex basis.
 */
static void brw_ff_gs_overwrite_header_dw2(struct brw_ff_gs_compile *c,
                                           unsigned dw2)
{
   struct brw_codegen *p = &c->func;
   brw_MOV(p, get_element_ud(c->reg.header, 2), brw_imm_ud(dw2));
}

/**
 * Overwrite DWORD 2 of c->reg.header with the primitive type from c->reg.R0.
 *
 * When the thread is spawned, GRF 0 contains the primitive type in bits 4:0
 * of DWORD 2.  URB_WRITE messages need the primitive type in bits 6:2 of
 * DWORD 2.  So this function extracts the primitive type field, bitshifts it
 * appropriately, and stores it in c->reg.header.
 */
static void brw_ff_gs_overwrite_header_dw2_from_r0(struct brw_ff_gs_compile *c)
{
   struct brw_codegen *p = &c->func;
   brw_AND(p, get_element_ud(c->reg.header, 2), get_element_ud(c->reg.R0, 2),
           brw_imm_ud(0x1f));
   brw_SHL(p, get_element_ud(c->reg.header, 2),
           get_element_ud(c->reg.header, 2), brw_imm_ud(2));
}

/**
 * Apply an additive offset to DWORD 2 of c->reg.header.
 *
 * This is used to set/unset the "PrimStart" and "PrimEnd" flags appropriately
 * for each vertex.
 */
static void brw_ff_gs_offset_header_dw2(struct brw_ff_gs_compile *c,
                                        int offset)
{
   struct brw_codegen *p = &c->func;
   brw_ADD(p, get_element_d(c->reg.header, 2), get_element_d(c->reg.header, 2),
           brw_imm_d(offset));
}


/**
 * Emit a vertex using the URB_WRITE message.  Use the contents of
 * c->reg.header for the message header, and the registers starting at \c vert
 * for the vertex data.
 *
 * If \c last is true, then this is the last vertex, so no further URB space
 * should be allocated, and this message should end the thread.
 *
 * If \c last is false, then a new URB entry will be allocated, and its handle
 * will be stored in DWORD 0 of c->reg.header for use in the next URB_WRITE
 * message.
 */
static void brw_ff_gs_emit_vue(struct brw_ff_gs_compile *c,
                               struct brw_reg vert,
                               bool last)
{
   struct brw_codegen *p = &c->func;
   int write_offset = 0;
   bool complete = false;

   do {
      /* We can't write more than 14 registers at a time to the URB */
      int write_len = MIN2(c->nr_regs - write_offset, 14);
      if (write_len == c->nr_regs - write_offset)
         complete = true;

      /* Copy the vertex from vertn into m1..mN+1:
       */
      brw_copy8(p, brw_message_reg(1), offset(vert, write_offset), write_len);

      /* Send the vertex data to the URB.  If this is the last write for this
       * vertex, then we mark it as complete, and either end the thread or
       * allocate another vertex URB entry (depending whether this is the last
       * vertex).
       */
      enum brw_urb_write_flags flags;
      if (!complete)
         flags = BRW_URB_WRITE_NO_FLAGS;
      else if (last)
         flags = BRW_URB_WRITE_EOT_COMPLETE;
      else
         flags = BRW_URB_WRITE_ALLOCATE_COMPLETE;
      brw_urb_WRITE(p,
                    (flags & BRW_URB_WRITE_ALLOCATE) ? c->reg.temp
                    : retype(brw_null_reg(), BRW_REGISTER_TYPE_UD),
                    0,
                    c->reg.header,
                    flags,
                    write_len + 1, /* msg length */
                    (flags & BRW_URB_WRITE_ALLOCATE) ? 1
                    : 0, /* response length */
                    write_offset,  /* urb offset */
                    BRW_URB_SWIZZLE_NONE);
      write_offset += write_len;
   } while (!complete);

   if (!last) {
      brw_MOV(p, get_element_ud(c->reg.header, 0),
              get_element_ud(c->reg.temp, 0));
   }
}

/**
 * Send an FF_SYNC message to ensure that all previously spawned GS threads
 * have finished sending primitives down the pipeline, and to allocate a URB
 * entry for the first output vertex.  Only needed on Ironlake+.
 *
 * This function modifies c->reg.header: in DWORD 1, it stores num_prim (which
 * is needed by the FF_SYNC message), and in DWORD 0, it stores the handle to
 * the allocated URB entry (which will be needed by the URB_WRITE meesage that
 * follows).
 */
static void brw_ff_gs_ff_sync(struct brw_ff_gs_compile *c, int num_prim)
{
   struct brw_codegen *p = &c->func;

   brw_MOV(p, get_element_ud(c->reg.header, 1), brw_imm_ud(num_prim));
   brw_ff_sync(p,
               c->reg.temp,
               0,
               c->reg.header,
               1, /* allocate */
               1, /* response length */
               0 /* eot */);
   brw_MOV(p, get_element_ud(c->reg.header, 0),
           get_element_ud(c->reg.temp, 0));
}


static void
brw_ff_gs_quads(struct brw_ff_gs_compile *c,
		const struct brw_ff_gs_prog_key *key)
{
   brw_ff_gs_alloc_regs(c, 4, false);
   brw_ff_gs_initialize_header(c);
   /* Use polygons for correct edgeflag behaviour. Note that vertex 3
    * is the PV for quads, but vertex 0 for polygons:
    */
   if (c->func.devinfo->ver == 5)
      brw_ff_gs_ff_sync(c, 1);
   brw_ff_gs_overwrite_header_dw2(
      c, ((_3DPRIM_POLYGON << URB_WRITE_PRIM_TYPE_SHIFT)
          | URB_WRITE_PRIM_START));
   if (key->pv_first) {
      brw_ff_gs_emit_vue(c, c->reg.vertex[0], 0);
      brw_ff_gs_overwrite_header_dw2(
         c, _3DPRIM_POLYGON << URB_WRITE_PRIM_TYPE_SHIFT);
      brw_ff_gs_emit_vue(c, c->reg.vertex[1], 0);
      brw_ff_gs_emit_vue(c, c->reg.vertex[2], 0);
      brw_ff_gs_overwrite_header_dw2(
         c, ((_3DPRIM_POLYGON << URB_WRITE_PRIM_TYPE_SHIFT)
             | URB_WRITE_PRIM_END));
      brw_ff_gs_emit_vue(c, c->reg.vertex[3], 1);
   }
   else {
      brw_ff_gs_emit_vue(c, c->reg.vertex[3], 0);
      brw_ff_gs_overwrite_header_dw2(
         c, _3DPRIM_POLYGON << URB_WRITE_PRIM_TYPE_SHIFT);
      brw_ff_gs_emit_vue(c, c->reg.vertex[0], 0);
      brw_ff_gs_emit_vue(c, c->reg.vertex[1], 0);
      brw_ff_gs_overwrite_header_dw2(
         c, ((_3DPRIM_POLYGON << URB_WRITE_PRIM_TYPE_SHIFT)
             | URB_WRITE_PRIM_END));
      brw_ff_gs_emit_vue(c, c->reg.vertex[2], 1);
   }
}

static void
brw_ff_gs_quad_strip(struct brw_ff_gs_compile *c,
                     const struct brw_ff_gs_prog_key *key)
{
   brw_ff_gs_alloc_regs(c, 4, false);
   brw_ff_gs_initialize_header(c);

   if (c->func.devinfo->ver == 5)
      brw_ff_gs_ff_sync(c, 1);
   brw_ff_gs_overwrite_header_dw2(
      c, ((_3DPRIM_POLYGON << URB_WRITE_PRIM_TYPE_SHIFT)
          | URB_WRITE_PRIM_START));
   if (key->pv_first) {
      brw_ff_gs_emit_vue(c, c->reg.vertex[0], 0);
      brw_ff_gs_overwrite_header_dw2(
         c, _3DPRIM_POLYGON << URB_WRITE_PRIM_TYPE_SHIFT);
      brw_ff_gs_emit_vue(c, c->reg.vertex[1], 0);
      brw_ff_gs_emit_vue(c, c->reg.vertex[2], 0);
      brw_ff_gs_overwrite_header_dw2(
         c, ((_3DPRIM_POLYGON << URB_WRITE_PRIM_TYPE_SHIFT)
             | URB_WRITE_PRIM_END));
      brw_ff_gs_emit_vue(c, c->reg.vertex[3], 1);
   }
   else {
      brw_ff_gs_emit_vue(c, c->reg.vertex[2], 0);
      brw_ff_gs_overwrite_header_dw2(
         c, _3DPRIM_POLYGON << URB_WRITE_PRIM_TYPE_SHIFT);
      brw_ff_gs_emit_vue(c, c->reg.vertex[3], 0);
      brw_ff_gs_emit_vue(c, c->reg.vertex[0], 0);
      brw_ff_gs_overwrite_header_dw2(
         c, ((_3DPRIM_POLYGON << URB_WRITE_PRIM_TYPE_SHIFT)
             | URB_WRITE_PRIM_END));
      brw_ff_gs_emit_vue(c, c->reg.vertex[1], 1);
   }
}

static void brw_ff_gs_lines(struct brw_ff_gs_compile *c)
{
   brw_ff_gs_alloc_regs(c, 2, false);
   brw_ff_gs_initialize_header(c);

   if (c->func.devinfo->ver == 5)
      brw_ff_gs_ff_sync(c, 1);
   brw_ff_gs_overwrite_header_dw2(
      c, ((_3DPRIM_LINESTRIP << URB_WRITE_PRIM_TYPE_SHIFT)
          | URB_WRITE_PRIM_START));
   brw_ff_gs_emit_vue(c, c->reg.vertex[0], 0);
   brw_ff_gs_overwrite_header_dw2(
      c, ((_3DPRIM_LINESTRIP << URB_WRITE_PRIM_TYPE_SHIFT)
          | URB_WRITE_PRIM_END));
   brw_ff_gs_emit_vue(c, c->reg.vertex[1], 1);
}

/**
 * Generate the geometry shader program used on Gen6 to perform stream output
 * (transform feedback).
 */
static void
gfx6_sol_program(struct brw_ff_gs_compile *c, const struct brw_ff_gs_prog_key *key,
                 unsigned num_verts, bool check_edge_flags)
{
   struct brw_codegen *p = &c->func;
   brw_inst *inst;
   c->prog_data->svbi_postincrement_value = num_verts;

   brw_ff_gs_alloc_regs(c, num_verts, true);
   brw_ff_gs_initialize_header(c);

   if (key->num_transform_feedback_bindings > 0) {
      unsigned vertex, binding;
      struct brw_reg destination_indices_uw =
         vec8(retype(c->reg.destination_indices, BRW_REGISTER_TYPE_UW));

      /* Note: since we use the binding table to keep track of buffer offsets
       * and stride, the GS doesn't need to keep track of a separate pointer
       * into each buffer; it uses a single pointer which increments by 1 for
       * each vertex.  So we use SVBI0 for this pointer, regardless of whether
       * transform feedback is in interleaved or separate attribs mode.
       *
       * Make sure that the buffers have enough room for all the vertices.
       */
      brw_ADD(p, get_element_ud(c->reg.temp, 0),
                 get_element_ud(c->reg.SVBI, 0), brw_imm_ud(num_verts));
      brw_CMP(p, vec1(brw_null_reg()), BRW_CONDITIONAL_LE,
                 get_element_ud(c->reg.temp, 0),
                 get_element_ud(c->reg.SVBI, 4));
      brw_IF(p, BRW_EXECUTE_1);

      /* Compute the destination indices to write to.  Usually we use SVBI[0]
       * + (0, 1, 2).  However, for odd-numbered triangles in tristrips, the
       * vertices come down the pipeline in reversed winding order, so we need
       * to flip the order when writing to the transform feedback buffer.  To
       * ensure that flatshading accuracy is preserved, we need to write them
       * in order SVBI[0] + (0, 2, 1) if we're using the first provoking
       * vertex convention, and in order SVBI[0] + (1, 0, 2) if we're using
       * the last provoking vertex convention.
       *
       * Note: since brw_imm_v can only be used in instructions in
       * packed-word execution mode, and SVBI is a double-word, we need to
       * first move the appropriate immediate constant ((0, 1, 2), (0, 2, 1),
       * or (1, 0, 2)) to the destination_indices register, and then add SVBI
       * using a separate instruction.  Also, since the immediate constant is
       * expressed as packed words, and we need to load double-words into
       * destination_indices, we need to intersperse zeros to fill the upper
       * halves of each double-word.
       */
      brw_MOV(p, destination_indices_uw,
              brw_imm_v(0x00020100)); /* (0, 1, 2) */
      if (num_verts == 3) {
         /* Get primitive type into temp register. */
         brw_AND(p, get_element_ud(c->reg.temp, 0),
                 get_element_ud(c->reg.R0, 2), brw_imm_ud(0x1f));

         /* Test if primitive type is TRISTRIP_REVERSE.  We need to do this as
          * an 8-wide comparison so that the conditional MOV that follows
          * moves all 8 words correctly.
          */
         brw_CMP(p, vec8(brw_null_reg()), BRW_CONDITIONAL_EQ,
                 get_element_ud(c->reg.temp, 0),
                 brw_imm_ud(_3DPRIM_TRISTRIP_REVERSE));

         /* If so, then overwrite destination_indices_uw with the appropriate
          * reordering.
          */
         inst = brw_MOV(p, destination_indices_uw,
                        brw_imm_v(key->pv_first ? 0x00010200    /* (0, 2, 1) */
                                                : 0x00020001)); /* (1, 0, 2) */
         brw_inst_set_pred_control(p->devinfo, inst, BRW_PREDICATE_NORMAL);
      }

      assert(c->reg.destination_indices.width == BRW_EXECUTE_4);
      brw_push_insn_state(p);
      brw_set_default_exec_size(p, BRW_EXECUTE_4);
      brw_ADD(p, c->reg.destination_indices,
              c->reg.destination_indices, get_element_ud(c->reg.SVBI, 0));
      brw_pop_insn_state(p);
      /* For each vertex, generate code to output each varying using the
       * appropriate binding table entry.
       */
      for (vertex = 0; vertex < num_verts; ++vertex) {
         /* Set up the correct destination index for this vertex */
         brw_MOV(p, get_element_ud(c->reg.header, 5),
                 get_element_ud(c->reg.destination_indices, vertex));

         for (binding = 0; binding < key->num_transform_feedback_bindings;
              ++binding) {
            unsigned char varying =
               key->transform_feedback_bindings[binding];
            unsigned char slot = c->vue_map.varying_to_slot[varying];
            /* From the Sandybridge PRM, Volume 2, Part 1, Section 4.5.1:
             *
             *   "Prior to End of Thread with a URB_WRITE, the kernel must
             *   ensure that all writes are complete by sending the final
             *   write as a committed write."
             */
            bool final_write =
               binding == key->num_transform_feedback_bindings - 1 &&
               vertex == num_verts - 1;
            struct brw_reg vertex_slot = c->reg.vertex[vertex];
            vertex_slot.nr += slot / 2;
            vertex_slot.subnr = (slot % 2) * 16;
            /* gl_PointSize is stored in VARYING_SLOT_PSIZ.w. */
            vertex_slot.swizzle = varying == VARYING_SLOT_PSIZ
               ? BRW_SWIZZLE_WWWW : key->transform_feedback_swizzles[binding];
            brw_set_default_access_mode(p, BRW_ALIGN_16);
            brw_push_insn_state(p);
            brw_set_default_exec_size(p, BRW_EXECUTE_4);

            brw_MOV(p, stride(c->reg.header, 4, 4, 1),
                    retype(vertex_slot, BRW_REGISTER_TYPE_UD));
            brw_pop_insn_state(p);

            brw_set_default_access_mode(p, BRW_ALIGN_1);
            brw_svb_write(p,
                          final_write ? c->reg.temp : brw_null_reg(), /* dest */
                          1, /* msg_reg_nr */
                          c->reg.header, /* src0 */
                          BRW_GFX6_SOL_BINDING_START + binding, /* binding_table_index */
                          final_write); /* send_commit_msg */
         }
      }
      brw_ENDIF(p);

      /* Now, reinitialize the header register from R0 to restore the parts of
       * the register that we overwrote while streaming out transform feedback
       * data.
       */
      brw_ff_gs_initialize_header(c);

      /* Finally, wait for the write commit to occur so that we can proceed to
       * other things safely.
       *
       * From the Sandybridge PRM, Volume 4, Part 1, Section 3.3:
       *
       *   The write commit does not modify the destination register, but
       *   merely clears the dependency associated with the destination
       *   register. Thus, a simple “mov” instruction using the register as a
       *   source is sufficient to wait for the write commit to occur.
       */
      brw_MOV(p, c->reg.temp, c->reg.temp);
   }

   brw_ff_gs_ff_sync(c, 1);

   brw_ff_gs_overwrite_header_dw2_from_r0(c);
   switch (num_verts) {
   case 1:
      brw_ff_gs_offset_header_dw2(c,
                                  URB_WRITE_PRIM_START | URB_WRITE_PRIM_END);
      brw_ff_gs_emit_vue(c, c->reg.vertex[0], true);
      break;
   case 2:
      brw_ff_gs_offset_header_dw2(c, URB_WRITE_PRIM_START);
      brw_ff_gs_emit_vue(c, c->reg.vertex[0], false);
      brw_ff_gs_offset_header_dw2(c,
                                  URB_WRITE_PRIM_END - URB_WRITE_PRIM_START);
      brw_ff_gs_emit_vue(c, c->reg.vertex[1], true);
      break;
   case 3:
      if (check_edge_flags) {
         /* Only emit vertices 0 and 1 if this is the first triangle of the
          * polygon.  Otherwise they are redundant.
          */
         brw_AND(p, retype(brw_null_reg(), BRW_REGISTER_TYPE_UD),
                 get_element_ud(c->reg.R0, 2),
                 brw_imm_ud(BRW_GS_EDGE_INDICATOR_0));
         brw_inst_set_cond_modifier(p->devinfo, brw_last_inst, BRW_CONDITIONAL_NZ);
         brw_IF(p, BRW_EXECUTE_1);
      }
      brw_ff_gs_offset_header_dw2(c, URB_WRITE_PRIM_START);
      brw_ff_gs_emit_vue(c, c->reg.vertex[0], false);
      brw_ff_gs_offset_header_dw2(c, -URB_WRITE_PRIM_START);
      brw_ff_gs_emit_vue(c, c->reg.vertex[1], false);
      if (check_edge_flags) {
         brw_ENDIF(p);
         /* Only emit vertex 2 in PRIM_END mode if this is the last triangle
          * of the polygon.  Otherwise leave the primitive incomplete because
          * there are more polygon vertices coming.
          */
         brw_AND(p, retype(brw_null_reg(), BRW_REGISTER_TYPE_UD),
                 get_element_ud(c->reg.R0, 2),
                 brw_imm_ud(BRW_GS_EDGE_INDICATOR_1));
         brw_inst_set_cond_modifier(p->devinfo, brw_last_inst, BRW_CONDITIONAL_NZ);
         brw_set_default_predicate_control(p, BRW_PREDICATE_NORMAL);
      }
      brw_ff_gs_offset_header_dw2(c, URB_WRITE_PRIM_END);
      brw_set_default_predicate_control(p, BRW_PREDICATE_NONE);
      brw_ff_gs_emit_vue(c, c->reg.vertex[2], true);
      break;
   }
}

const unsigned *
brw_compile_ff_gs_prog(struct brw_compiler *compiler,
		       void *mem_ctx,
		       const struct brw_ff_gs_prog_key *key,
		       struct brw_ff_gs_prog_data *prog_data,
		       struct brw_vue_map *vue_map,
		       unsigned *final_assembly_size)
{
   struct brw_ff_gs_compile c;
   const GLuint *program;

   memset(&c, 0, sizeof(c));

   c.key = *key;
   c.vue_map = *vue_map;
   c.nr_regs = (c.vue_map.num_slots + 1)/2;
   c.prog_data = prog_data;

   mem_ctx = ralloc_context(NULL);

   /* Begin the compilation:
    */
   brw_init_codegen(&compiler->isa, &c.func, mem_ctx);

   c.func.single_program_flow = 1;

   /* For some reason the thread is spawned with only 4 channels
    * unmasked.
    */
   brw_set_default_mask_control(&c.func, BRW_MASK_DISABLE);

   if (compiler->devinfo->ver >= 6) {
      unsigned num_verts;
      bool check_edge_flag;
      /* On Sandybridge, we use the GS for implementing transform feedback
       * (called "Stream Out" in the PRM).
       */
      switch (key->primitive) {
      case _3DPRIM_POINTLIST:
         num_verts = 1;
         check_edge_flag = false;
         break;
      case _3DPRIM_LINELIST:
      case _3DPRIM_LINESTRIP:
      case _3DPRIM_LINELOOP:
         num_verts = 2;
         check_edge_flag = false;
         break;
      case _3DPRIM_TRILIST:
      case _3DPRIM_TRIFAN:
      case _3DPRIM_TRISTRIP:
      case _3DPRIM_RECTLIST:
         num_verts = 3;
         check_edge_flag = false;
         break;
      case _3DPRIM_QUADLIST:
      case _3DPRIM_QUADSTRIP:
      case _3DPRIM_POLYGON:
         num_verts = 3;
         check_edge_flag = true;
         break;
      default:
         unreachable("Unexpected primitive type in Gen6 SOL program.");
      }
      gfx6_sol_program(&c, key, num_verts, check_edge_flag);
   } else {
      /* On Gen4-5, we use the GS to decompose certain types of primitives.
       * Note that primitives which don't require a GS program have already
       * been weeded out by now.
       */
      switch (key->primitive) {
      case _3DPRIM_QUADLIST:
         brw_ff_gs_quads( &c, key );
         break;
      case _3DPRIM_QUADSTRIP:
         brw_ff_gs_quad_strip( &c, key );
         break;
      case _3DPRIM_LINELOOP:
         brw_ff_gs_lines( &c );
         break;
      default:
         return NULL;
      }
   }

   brw_compact_instructions(&c.func, 0, NULL);

   /* get the program
    */
   program = brw_get_program(&c.func, final_assembly_size);

   if (INTEL_DEBUG(DEBUG_GS)) {
      fprintf(stderr, "gs:\n");
      brw_disassemble_with_labels(&compiler->isa, c.func.store,
                                  0, *final_assembly_size, stderr);
      fprintf(stderr, "\n");
    }

   return program;
}

