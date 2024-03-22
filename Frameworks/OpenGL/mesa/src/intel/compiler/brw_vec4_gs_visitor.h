/*
 * Copyright © 2013 Intel Corporation
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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/**
 * \file brw_vec4_gs_visitor.h
 *
 * Geometry-shader-specific code derived from the vec4_visitor class.
 */

#ifndef BRW_VEC4_GS_VISITOR_H
#define BRW_VEC4_GS_VISITOR_H

#include "brw_vec4.h"

#define MAX_GS_INPUT_VERTICES 6

#ifdef __cplusplus
namespace brw {

class vec4_gs_visitor : public vec4_visitor
{
public:
   vec4_gs_visitor(const struct brw_compiler *compiler,
                   const struct brw_compile_params *params,
                   struct brw_gs_compile *c,
                   struct brw_gs_prog_data *prog_data,
                   const nir_shader *shader,
                   bool no_spills,
                   bool debug_enabled);

protected:
   virtual void setup_payload();
   virtual void emit_prolog();
   virtual void emit_thread_end();
   virtual void emit_urb_write_header(int mrf);
   virtual vec4_instruction *emit_urb_write_opcode(bool complete);
   virtual void gs_emit_vertex(int stream_id);
   virtual void gs_end_primitive();
   virtual void nir_emit_intrinsic(nir_intrinsic_instr *instr);

protected:
   int setup_varying_inputs(int payload_reg, int attributes_per_reg);
   void emit_control_data_bits();
   void set_stream_control_data_bits(unsigned stream_id);

   src_reg vertex_count;
   src_reg control_data_bits;
   const struct brw_gs_compile * const c;
   struct brw_gs_prog_data * const gs_prog_data;
};

} /* namespace brw */
#endif /* __cplusplus */

#endif /* BRW_VEC4_GS_VISITOR_H */
