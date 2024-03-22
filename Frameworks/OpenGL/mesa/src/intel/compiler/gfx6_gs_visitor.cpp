/*
 * Copyright © 2014 Intel Corporation
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
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
 * This code is based on original work by Ilia Mirkin.
 */

/**
 * \file gfx6_gs_visitor.cpp
 *
 * Gfx6 geometry shader implementation
 */

#include "gfx6_gs_visitor.h"
#include "brw_eu.h"
#include "brw_prim.h"

namespace brw {

void
gfx6_gs_visitor::emit_prolog()
{
   vec4_gs_visitor::emit_prolog();

   /* Gfx6 geometry shaders require to allocate an initial VUE handle via
    * FF_SYNC message, however the documentation remarks that only one thread
    * can write to the URB simultaneously and the FF_SYNC message provides the
    * synchronization mechanism for this, so using this message effectively
    * stalls the thread until it is its turn to write to the URB. Because of
    * this, the best way to implement geometry shader algorithms in gfx6 is to
    * execute the algorithm before the FF_SYNC message to maximize parallelism.
    *
    * To achieve this we buffer the geometry shader outputs for each emitted
    * vertex in vertex_output during operation. Then, when we have processed
    * the last vertex (that is, at thread end time), we send the FF_SYNC
    * message to allocate the initial VUE handle and write all buffered vertex
    * data to the URB in one go.
    *
    * For each emitted vertex, vertex_output will hold vue_map.num_slots
    * data items plus one additional item to hold required flags
    * (PrimType, PrimStart, PrimEnd, as expected by the URB_WRITE message)
    * which come right after the data items for that vertex. Vertex data and
    * flags for the next vertex come right after the data items and flags for
    * the previous vertex.
    */
   this->current_annotation = "gfx6 prolog";
   this->vertex_output = src_reg(this,
                                 glsl_uint_type(),
                                 (prog_data->vue_map.num_slots + 1) *
                                 nir->info.gs.vertices_out);
   this->vertex_output_offset = src_reg(this, glsl_uint_type());
   emit(MOV(dst_reg(this->vertex_output_offset), brw_imm_ud(0u)));

   /* MRF 1 will be the header for all messages (FF_SYNC and URB_WRITES),
    * so initialize it once to R0.
    */
   vec4_instruction *inst = emit(MOV(dst_reg(MRF, 1),
                                     retype(brw_vec8_grf(0, 0),
                                            BRW_REGISTER_TYPE_UD)));
   inst->force_writemask_all = true;

   /* This will be used as a temporary to store writeback data of FF_SYNC
    * and URB_WRITE messages.
    */
   this->temp = src_reg(this, glsl_uint_type());

   /* This will be used to know when we are processing the first vertex of
    * a primitive. We will set this to URB_WRITE_PRIM_START only when we know
    * that we are processing the first vertex in the primitive and to zero
    * otherwise. This way we can use its value directly in the URB write
    * headers.
    */
   this->first_vertex = src_reg(this, glsl_uint_type());
   emit(MOV(dst_reg(this->first_vertex), brw_imm_ud(URB_WRITE_PRIM_START)));

   /* The FF_SYNC message requires to know the number of primitives generated,
    * so keep a counter for this.
    */
   this->prim_count = src_reg(this, glsl_uint_type());
   emit(MOV(dst_reg(this->prim_count), brw_imm_ud(0u)));

   if (gs_prog_data->num_transform_feedback_bindings) {
      /* Create a virtual register to hold destination indices in SOL */
      this->destination_indices = src_reg(this, glsl_uvec4_type());
      /* Create a virtual register to hold number of written primitives */
      this->sol_prim_written = src_reg(this, glsl_uint_type());
      /* Create a virtual register to hold Streamed Vertex Buffer Indices */
      this->svbi = src_reg(this, glsl_uvec4_type());
      /* Create a virtual register to hold max values of SVBI */
      this->max_svbi = src_reg(this, glsl_uvec4_type());
      emit(MOV(dst_reg(this->max_svbi),
               src_reg(retype(brw_vec1_grf(1, 4), BRW_REGISTER_TYPE_UD))));
   }

   /* PrimitveID is delivered in r0.1 of the thread payload. If the program
    * needs it we have to move it to a separate register where we can map
    * the attribute.
    *
    * Notice that we cannot use a virtual register for this, because we need to
    * map all input attributes to hardware registers in setup_payload(),
    * which happens before virtual registers are mapped to hardware registers.
    * We could work around that issue if we were able to compute the first
    * non-payload register here and move the PrimitiveID information to that
    * register, but we can't because at this point we don't know the final
    * number uniforms that will be included in the payload.
    *
    * So, what we do is to place PrimitiveID information in r1, which is always
    * delivered as part of the payload, but its only populated with data
    * relevant for transform feedback when we set GFX6_GS_SVBI_PAYLOAD_ENABLE
    * in the 3DSTATE_GS state packet. That information can be obtained by other
    * means though, so we can safely use r1 for this purpose.
    */
   if (gs_prog_data->include_primitive_id) {
      this->primitive_id =
         src_reg(retype(brw_vec8_grf(1, 0), BRW_REGISTER_TYPE_UD));
      emit(GS_OPCODE_SET_PRIMITIVE_ID, dst_reg(this->primitive_id));
   }
}

void
gfx6_gs_visitor::gs_emit_vertex(int stream_id)
{
   this->current_annotation = "gfx6 emit vertex";

   /* Buffer all output slots for this vertex in vertex_output */
   for (int slot = 0; slot < prog_data->vue_map.num_slots; ++slot) {
      int varying = prog_data->vue_map.slot_to_varying[slot];
      if (varying != VARYING_SLOT_PSIZ) {
         dst_reg dst(this->vertex_output);
         dst.reladdr = ralloc(mem_ctx, src_reg);
         memcpy(dst.reladdr, &this->vertex_output_offset, sizeof(src_reg));
         emit_urb_slot(dst, varying);
      } else {
         /* The PSIZ slot can pack multiple varyings in different channels
          * and emit_urb_slot() will produce a MOV instruction for each of
          * them. Since we are writing to an array, that will translate to
          * possibly multiple MOV instructions with an array destination and
          * each will generate a scratch write with the same offset into
          * scratch space (thus, each one overwriting the previous). This is
          * not what we want. What we will do instead is emit PSIZ to a
          * a regular temporary register, then move that register into the
          * array. This way we only have one instruction with an array
          * destination and we only produce a single scratch write.
          */
         dst_reg tmp = dst_reg(src_reg(this, glsl_uvec4_type()));
         emit_urb_slot(tmp, varying);
         dst_reg dst(this->vertex_output);
         dst.reladdr = ralloc(mem_ctx, src_reg);
         memcpy(dst.reladdr, &this->vertex_output_offset, sizeof(src_reg));
         vec4_instruction *inst = emit(MOV(dst, src_reg(tmp)));
         inst->force_writemask_all = true;
      }

      emit(ADD(dst_reg(this->vertex_output_offset),
               this->vertex_output_offset, brw_imm_ud(1u)));
   }

   /* Now buffer flags for this vertex */
   dst_reg dst(this->vertex_output);
   dst.reladdr = ralloc(mem_ctx, src_reg);
   memcpy(dst.reladdr, &this->vertex_output_offset, sizeof(src_reg));
   if (nir->info.gs.output_primitive == MESA_PRIM_POINTS) {
      /* If we are outputting points, then every vertex has PrimStart and
       * PrimEnd set.
       */
      emit(MOV(dst, brw_imm_d((_3DPRIM_POINTLIST << URB_WRITE_PRIM_TYPE_SHIFT) |
                              URB_WRITE_PRIM_START | URB_WRITE_PRIM_END)));
      emit(ADD(dst_reg(this->prim_count), this->prim_count, brw_imm_ud(1u)));
   } else {
      /* Otherwise, we can only set the PrimStart flag, which we have stored
       * in the first_vertex register. We will have to wait until we execute
       * EndPrimitive() or we end the thread to set the PrimEnd flag on a
       * vertex.
       */
      emit(OR(dst, this->first_vertex,
              brw_imm_ud(gs_prog_data->output_topology <<
                         URB_WRITE_PRIM_TYPE_SHIFT)));
      emit(MOV(dst_reg(this->first_vertex), brw_imm_ud(0u)));
   }
   emit(ADD(dst_reg(this->vertex_output_offset),
            this->vertex_output_offset, brw_imm_ud(1u)));
}

void
gfx6_gs_visitor::gs_end_primitive()
{
   this->current_annotation = "gfx6 end primitive";
   /* Calling EndPrimitive() is optional for point output. In this case we set
    * the PrimEnd flag when we process EmitVertex().
    */
   if (nir->info.gs.output_primitive == MESA_PRIM_POINTS)
      return;

   /* Otherwise we know that the last vertex we have processed was the last
    * vertex in the primitive and we need to set its PrimEnd flag, so do this
    * unless we haven't emitted that vertex at all (vertex_count != 0).
    *
    * Notice that we have already incremented vertex_count when we processed
    * the last emit_vertex, so we need to take that into account in the
    * comparison below (hence the num_output_vertices + 1 in the comparison
    * below).
    */
   unsigned num_output_vertices = nir->info.gs.vertices_out;
   emit(CMP(dst_null_ud(), this->vertex_count,
            brw_imm_ud(num_output_vertices + 1), BRW_CONDITIONAL_L));
   vec4_instruction *inst = emit(CMP(dst_null_ud(),
                                     this->vertex_count, brw_imm_ud(0u),
                                     BRW_CONDITIONAL_NEQ));
   inst->predicate = BRW_PREDICATE_NORMAL;
   emit(IF(BRW_PREDICATE_NORMAL));
   {
      /* vertex_output_offset is already pointing at the first entry of the
       * next vertex. So subtract 1 to modify the flags for the previous
       * vertex.
       */
      src_reg offset(this, glsl_uint_type());
      emit(ADD(dst_reg(offset), this->vertex_output_offset, brw_imm_d(-1)));

      src_reg dst(this->vertex_output);
      dst.reladdr = ralloc(mem_ctx, src_reg);
      memcpy(dst.reladdr, &offset, sizeof(src_reg));

      emit(OR(dst_reg(dst), dst, brw_imm_d(URB_WRITE_PRIM_END)));
      emit(ADD(dst_reg(this->prim_count), this->prim_count, brw_imm_ud(1u)));

      /* Set the first vertex flag to indicate that the next vertex will start
       * a primitive.
       */
      emit(MOV(dst_reg(this->first_vertex), brw_imm_d(URB_WRITE_PRIM_START)));
   }
   emit(BRW_OPCODE_ENDIF);
}

void
gfx6_gs_visitor::emit_urb_write_header(int mrf)
{
   this->current_annotation = "gfx6 urb header";
   /* Compute offset of the flags for the current vertex in vertex_output and
    * write them in dw2 of the message header.
    *
    * Notice that by the time that emit_thread_end() calls here
    * vertex_output_offset should point to the first data item of the current
    * vertex in vertex_output, thus we only need to add the number of output
    * slots per vertex to that offset to obtain the flags data offset.
    */
   src_reg flags_offset(this, glsl_uint_type());
   emit(ADD(dst_reg(flags_offset),
            this->vertex_output_offset,
            brw_imm_d(prog_data->vue_map.num_slots)));

   src_reg flags_data(this->vertex_output);
   flags_data.reladdr = ralloc(mem_ctx, src_reg);
   memcpy(flags_data.reladdr, &flags_offset, sizeof(src_reg));

   emit(GS_OPCODE_SET_DWORD_2, dst_reg(MRF, mrf), flags_data);
}

static unsigned
align_interleaved_urb_mlen(unsigned mlen)
{
   /* URB data written (does not include the message header reg) must
    * be a multiple of 256 bits, or 2 VS registers.  See vol5c.5,
    * section 5.4.3.2.2: URB_INTERLEAVED.
    */
   if ((mlen % 2) != 1)
      mlen++;
   return mlen;
}

void
gfx6_gs_visitor::emit_snb_gs_urb_write_opcode(bool complete, int base_mrf,
                                              int last_mrf, int urb_offset)
{
   vec4_instruction *inst = NULL;

   if (!complete) {
      /* If the vertex is not complete we don't have to do anything special */
      inst = emit(VEC4_GS_OPCODE_URB_WRITE);
      inst->urb_write_flags = BRW_URB_WRITE_NO_FLAGS;
   } else {
      /* Otherwise we always request to allocate a new VUE handle. If this is
       * the last write before the EOT message and the new handle never gets
       * used it will be dereferenced when we send the EOT message. This is
       * necessary to avoid different setups for the EOT message (one for the
       * case when there is no output and another for the case when there is)
       * which would require to end the program with an IF/ELSE/ENDIF block,
       * something we do not want.
       */
      inst = emit(VEC4_GS_OPCODE_URB_WRITE_ALLOCATE);
      inst->urb_write_flags = BRW_URB_WRITE_COMPLETE;
      inst->dst = dst_reg(MRF, base_mrf);
      inst->src[0] = this->temp;
   }

   inst->base_mrf = base_mrf;
   inst->mlen = align_interleaved_urb_mlen(last_mrf - base_mrf);
   inst->offset = urb_offset;
}

void
gfx6_gs_visitor::emit_thread_end()
{
   /* Make sure the current primitive is ended: we know it is not ended when
    * first_vertex is not zero. This is only relevant for outputs other than
    * points because in the point case we set PrimEnd on all vertices.
    */
   if (nir->info.gs.output_primitive != MESA_PRIM_POINTS) {
      emit(CMP(dst_null_ud(), this->first_vertex, brw_imm_ud(0u), BRW_CONDITIONAL_Z));
      emit(IF(BRW_PREDICATE_NORMAL));
      gs_end_primitive();
      emit(BRW_OPCODE_ENDIF);
   }

   /* Here we have to:
    * 1) Emit an FF_SYNC message to obtain an initial VUE handle.
    * 2) Loop over all buffered vertex data and write it to corresponding
    *    URB entries.
    * 3) Allocate new VUE handles for all vertices other than the first.
    * 4) Send a final EOT message.
    */

   /* MRF 0 is reserved for the debugger, so start with message header
    * in MRF 1.
    */
   int base_mrf = 1;

   /* In the process of generating our URB write message contents, we
    * may need to unspill a register or load from an array.  Those
    * reads would use MRFs 21..23
    */
   int max_usable_mrf = FIRST_SPILL_MRF(devinfo->ver);

   /* Issue the FF_SYNC message and obtain the initial VUE handle. */
   this->current_annotation = "gfx6 thread end: ff_sync";

   vec4_instruction *inst = NULL;
   if (gs_prog_data->num_transform_feedback_bindings) {
      src_reg sol_temp(this, glsl_uvec4_type());
      emit(GS_OPCODE_FF_SYNC_SET_PRIMITIVES,
           dst_reg(this->svbi),
           this->vertex_count,
           this->prim_count,
           sol_temp);
      inst = emit(GS_OPCODE_FF_SYNC,
                  dst_reg(this->temp), this->prim_count, this->svbi);
   } else {
      inst = emit(GS_OPCODE_FF_SYNC,
                  dst_reg(this->temp), this->prim_count, brw_imm_ud(0u));
   }
   inst->base_mrf = base_mrf;

   emit(CMP(dst_null_ud(), this->vertex_count, brw_imm_ud(0u), BRW_CONDITIONAL_G));
   emit(IF(BRW_PREDICATE_NORMAL));
   {
      /* Loop over all buffered vertices and emit URB write messages */
      this->current_annotation = "gfx6 thread end: urb writes init";
      src_reg vertex(this, glsl_uint_type());
      emit(MOV(dst_reg(vertex), brw_imm_ud(0u)));
      emit(MOV(dst_reg(this->vertex_output_offset), brw_imm_ud(0u)));

      this->current_annotation = "gfx6 thread end: urb writes";
      emit(BRW_OPCODE_DO);
      {
         emit(CMP(dst_null_d(), vertex, this->vertex_count, BRW_CONDITIONAL_GE));
         inst = emit(BRW_OPCODE_BREAK);
         inst->predicate = BRW_PREDICATE_NORMAL;

         /* First we prepare the message header */
         emit_urb_write_header(base_mrf);

         /* Then add vertex data to the message in interleaved fashion */
         int slot = 0;
         bool complete = false;
         do {
            int mrf = base_mrf + 1;

            /* URB offset is in URB row increments, and each of our MRFs is half
             * of one of those, since we're doing interleaved writes.
             */
            int urb_offset = slot / 2;

            for (; slot < prog_data->vue_map.num_slots; ++slot) {
               int varying = prog_data->vue_map.slot_to_varying[slot];
               current_annotation = output_reg_annotation[varying];

               /* Compute offset of this slot for the current vertex
                * in vertex_output
                */
               src_reg data(this->vertex_output);
               data.reladdr = ralloc(mem_ctx, src_reg);
               memcpy(data.reladdr, &this->vertex_output_offset,
                      sizeof(src_reg));

               /* Copy this slot to the appropriate message register */
               dst_reg reg = dst_reg(MRF, mrf);
               reg.type = output_reg[varying][0].type;
               data.type = reg.type;
               inst = emit(MOV(reg, data));
               inst->force_writemask_all = true;

               mrf++;
               emit(ADD(dst_reg(this->vertex_output_offset),
                        this->vertex_output_offset, brw_imm_ud(1u)));

               /* If this was max_usable_mrf, we can't fit anything more into
                * this URB WRITE. Same if we reached the max. message length.
                */
               if (mrf > max_usable_mrf ||
                   align_interleaved_urb_mlen(mrf - base_mrf + 1) > BRW_MAX_MSG_LENGTH) {
                  slot++;
                  break;
               }
            }

            complete = slot >= prog_data->vue_map.num_slots;
            emit_snb_gs_urb_write_opcode(complete, base_mrf, mrf, urb_offset);
         } while (!complete);

         /* Skip over the flags data item so that vertex_output_offset points
          * to the first data item of the next vertex, so that we can start
          * writing the next vertex.
          */
         emit(ADD(dst_reg(this->vertex_output_offset),
                  this->vertex_output_offset, brw_imm_ud(1u)));

         emit(ADD(dst_reg(vertex), vertex, brw_imm_ud(1u)));
      }
      emit(BRW_OPCODE_WHILE);

      if (gs_prog_data->num_transform_feedback_bindings)
         xfb_write();
   }
   emit(BRW_OPCODE_ENDIF);

   /* Finally, emit EOT message.
    *
    * In gfx6 we need to end the thread differently depending on whether we have
    * emitted at least one vertex or not. In case we did, the EOT message must
    * always include the COMPLETE flag or else the GPU hangs. If we have not
    * produced any output we can't use the COMPLETE flag.
    *
    * However, this would lead us to end the program with an ENDIF opcode,
    * which we want to avoid, so what we do is that we always request a new
    * VUE handle every time, even if GS produces no output.
    * With this we make sure that whether we have emitted at least one vertex
    * or none at all, we have to finish the thread without writing to the URB,
    * which works for both cases by setting the COMPLETE and UNUSED flags in
    * the EOT message.
    */
   this->current_annotation = "gfx6 thread end: EOT";

   if (gs_prog_data->num_transform_feedback_bindings) {
      /* When emitting EOT, set SONumPrimsWritten Increment Value. */
      src_reg data(this, glsl_uint_type());
      emit(AND(dst_reg(data), this->sol_prim_written, brw_imm_ud(0xffffu)));
      emit(SHL(dst_reg(data), data, brw_imm_ud(16u)));
      emit(GS_OPCODE_SET_DWORD_2, dst_reg(MRF, base_mrf), data);
   }

   inst = emit(GS_OPCODE_THREAD_END);
   inst->urb_write_flags = BRW_URB_WRITE_COMPLETE | BRW_URB_WRITE_UNUSED;
   inst->base_mrf = base_mrf;
   inst->mlen = 1;
}

void
gfx6_gs_visitor::setup_payload()
{
   int attribute_map[BRW_VARYING_SLOT_COUNT * MAX_GS_INPUT_VERTICES];

   /* Attributes are going to be interleaved, so one register contains two
    * attribute slots.
    */
   int attributes_per_reg = 2;

   /* If a geometry shader tries to read from an input that wasn't written by
    * the vertex shader, that produces undefined results, but it shouldn't
    * crash anything.  So initialize attribute_map to zeros--that ensures that
    * these undefined results are read from r0.
    */
   memset(attribute_map, 0, sizeof(attribute_map));

   int reg = 0;

   /* The payload always contains important data in r0. */
   reg++;

   /* r1 is always part of the payload and it holds information relevant
    * for transform feedback when we set the GFX6_GS_SVBI_PAYLOAD_ENABLE bit in
    * the 3DSTATE_GS packet. We will overwrite it with the PrimitiveID
    * information (and move the original value to a virtual register if
    * necessary).
    */
   if (gs_prog_data->include_primitive_id)
      attribute_map[VARYING_SLOT_PRIMITIVE_ID] = attributes_per_reg * reg;
   reg++;

   reg = setup_uniforms(reg);

   reg = setup_varying_inputs(reg, attributes_per_reg);

   this->first_non_payload_grf = reg;
}

void
gfx6_gs_visitor::xfb_write()
{
   unsigned num_verts;

   switch (gs_prog_data->output_topology) {
   case _3DPRIM_POINTLIST:
      num_verts = 1;
      break;
   case _3DPRIM_LINELIST:
   case _3DPRIM_LINESTRIP:
   case _3DPRIM_LINELOOP:
      num_verts = 2;
      break;
   case _3DPRIM_TRILIST:
   case _3DPRIM_TRIFAN:
   case _3DPRIM_TRISTRIP:
   case _3DPRIM_RECTLIST:
      num_verts = 3;
      break;
   case _3DPRIM_QUADLIST:
   case _3DPRIM_QUADSTRIP:
   case _3DPRIM_POLYGON:
      num_verts = 3;
      break;
   default:
      unreachable("Unexpected primitive type in Gfx6 SOL program.");
   }

   this->current_annotation = "gfx6 thread end: svb writes init";

   emit(MOV(dst_reg(this->vertex_output_offset), brw_imm_ud(0u)));
   emit(MOV(dst_reg(this->sol_prim_written), brw_imm_ud(0u)));

   /* Check that at least one primitive can be written
    *
    * Note: since we use the binding table to keep track of buffer offsets
    * and stride, the GS doesn't need to keep track of a separate pointer
    * into each buffer; it uses a single pointer which increments by 1 for
    * each vertex.  So we use SVBI0 for this pointer, regardless of whether
    * transform feedback is in interleaved or separate attribs mode.
    */
   src_reg sol_temp(this, glsl_uvec4_type());
   emit(ADD(dst_reg(sol_temp), this->svbi, brw_imm_ud(num_verts)));

   /* Compare SVBI calculated number with the maximum value, which is
    * in R1.4 (previously saved in this->max_svbi) for gfx6.
    */
   emit(CMP(dst_null_d(), sol_temp, this->max_svbi, BRW_CONDITIONAL_LE));
   emit(IF(BRW_PREDICATE_NORMAL));
   {
      vec4_instruction *inst = emit(MOV(dst_reg(destination_indices),
                                        brw_imm_vf4(brw_float_to_vf(0.0),
                                                    brw_float_to_vf(1.0),
                                                    brw_float_to_vf(2.0),
                                                    brw_float_to_vf(0.0))));
      inst->force_writemask_all = true;

      emit(ADD(dst_reg(this->destination_indices),
               this->destination_indices,
               this->svbi));
   }
   emit(BRW_OPCODE_ENDIF);

   /* Write transform feedback data for all processed vertices. */
   for (int i = 0; i < (int)nir->info.gs.vertices_out; i++) {
      emit(MOV(dst_reg(sol_temp), brw_imm_d(i)));
      emit(CMP(dst_null_d(), sol_temp, this->vertex_count,
               BRW_CONDITIONAL_L));
      emit(IF(BRW_PREDICATE_NORMAL));
      {
         xfb_program(i, num_verts);
      }
      emit(BRW_OPCODE_ENDIF);
   }
}

void
gfx6_gs_visitor::xfb_program(unsigned vertex, unsigned num_verts)
{
   unsigned binding;
   unsigned num_bindings = gs_prog_data->num_transform_feedback_bindings;
   src_reg sol_temp(this, glsl_uvec4_type());

   /* Check for buffer overflow: we need room to write the complete primitive
    * (all vertices). Otherwise, avoid writing any vertices for it
    */
   emit(ADD(dst_reg(sol_temp), this->sol_prim_written, brw_imm_ud(1u)));
   emit(MUL(dst_reg(sol_temp), sol_temp, brw_imm_ud(num_verts)));
   emit(ADD(dst_reg(sol_temp), sol_temp, this->svbi));
   emit(CMP(dst_null_d(), sol_temp, this->max_svbi, BRW_CONDITIONAL_LE));
   emit(IF(BRW_PREDICATE_NORMAL));
   {
      /* Avoid overwriting MRF 1 as it is used as URB write message header */
      dst_reg mrf_reg(MRF, 2);

      this->current_annotation = "gfx6: emit SOL vertex data";
      /* For each vertex, generate code to output each varying using the
       * appropriate binding table entry.
       */
      for (binding = 0; binding < num_bindings; ++binding) {
         unsigned char varying =
            gs_prog_data->transform_feedback_bindings[binding];

         /* Set up the correct destination index for this vertex */
         vec4_instruction *inst = emit(GS_OPCODE_SVB_SET_DST_INDEX,
                                       mrf_reg,
                                       this->destination_indices);
         inst->sol_vertex = vertex % num_verts;

         /* From the Sandybridge PRM, Volume 2, Part 1, Section 4.5.1:
          *
          *   "Prior to End of Thread with a URB_WRITE, the kernel must
          *   ensure that all writes are complete by sending the final
          *   write as a committed write."
          */
         bool final_write = binding == (unsigned) num_bindings - 1 &&
                            inst->sol_vertex == num_verts - 1;

         /* Compute offset of this varying for the current vertex
          * in vertex_output
          */
         this->current_annotation = output_reg_annotation[varying];
         src_reg data(this->vertex_output);
         data.reladdr = ralloc(mem_ctx, src_reg);
         int offset = get_vertex_output_offset_for_varying(vertex, varying);
         emit(MOV(dst_reg(this->vertex_output_offset), brw_imm_d(offset)));
         memcpy(data.reladdr, &this->vertex_output_offset, sizeof(src_reg));
         data.type = output_reg[varying][0].type;
         data.swizzle = gs_prog_data->transform_feedback_swizzles[binding];

         /* Write data */
         inst = emit(GS_OPCODE_SVB_WRITE, mrf_reg, data, sol_temp);
         inst->sol_binding = binding;
         inst->sol_final_write = final_write;

         if (final_write) {
            /* This is the last vertex of the primitive, then increment
             * SO num primitive counter and destination indices.
             */
            emit(ADD(dst_reg(this->destination_indices),
                     this->destination_indices,
                     brw_imm_ud(num_verts)));
            emit(ADD(dst_reg(this->sol_prim_written),
                     this->sol_prim_written, brw_imm_ud(1u)));
         }

      }
      this->current_annotation = NULL;
   }
   emit(BRW_OPCODE_ENDIF);
}

int
gfx6_gs_visitor::get_vertex_output_offset_for_varying(int vertex, int varying)
{
   /* Find the output slot assigned to this varying.
    *
    * VARYING_SLOT_LAYER and VARYING_SLOT_VIEWPORT are packed in the same slot
    * as VARYING_SLOT_PSIZ.
    */
   if (varying == VARYING_SLOT_LAYER || varying == VARYING_SLOT_VIEWPORT)
      varying = VARYING_SLOT_PSIZ;
   int slot = prog_data->vue_map.varying_to_slot[varying];

   if (slot < 0) {
      /* This varying does not exist in the VUE so we are not writing to it
       * and its value is undefined. We still want to return a valid offset
       * into vertex_output though, to prevent any out-of-bound accesses into
       * the vertex_output array. Since the value for this varying is undefined
       * we don't really care for the value we assign to it, so any offset
       * within the limits of vertex_output will do.
       */
      slot = 0;
   }

   return vertex * (prog_data->vue_map.num_slots + 1) + slot;
}

} /* namespace brw */
