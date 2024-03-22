/*
 * Copyright © 2018 Intel Corporation
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
 */

#include "nir.h"
#include "nir_xfb_info.h"
#include "gl_nir_linker.h"
#include "linker_util.h"
#include "util/u_math.h"
#include "main/shader_types.h"
#include "main/consts_exts.h"

/**
 * This file does the linking of GLSL transform feedback using NIR.
 */

void
gl_nir_link_assign_xfb_resources(const struct gl_constants *consts,
                                 struct gl_shader_program *prog)
{
   /*
    * From ARB_gl_spirv spec:
    *
    *    "- If the *Xfb* Execution Mode is set, any output variable that is at
    *       least partially captured:
    *       * must be decorated with an *XfbBuffer*, declaring the capturing buffer
    *       * must have at least one captured output variable in the capturing
    *         buffer decorated with an *XfbStride* (and all such *XfbStride* values
    *         for the capturing buffer must be equal)
    *     - If the *Xfb* Execution Mode is set, any captured output:
    *       * must be a non-structure decorated with *Offset* or a member of a
    *         structure whose type member is decorated with *Offset*"
    *
    * Note the "must be", meaning that explicit buffer, offset and stride are
    * mandatory. So as this is intended to work with SPIR-V shaders we don't
    * need to calculate the offset or the stride.
    */

   struct gl_program *xfb_prog = prog->last_vert_prog;

   if (xfb_prog == NULL)
      return;

   /* free existing varyings, if any */
   for (unsigned i = 0; i < prog->TransformFeedback.NumVarying; i++)
      free(prog->TransformFeedback.VaryingNames[i]);
   free(prog->TransformFeedback.VaryingNames);

   nir_xfb_info *xfb_info = NULL;
   nir_xfb_varyings_info *varyings_info = NULL;

   /* Find last stage before fragment shader */
   for (int stage = MESA_SHADER_FRAGMENT - 1; stage >= 0; stage--) {
      struct gl_linked_shader *sh = prog->_LinkedShaders[stage];

      if (sh && stage != MESA_SHADER_TESS_CTRL) {
         nir_gather_xfb_info_with_varyings(sh->Program->nir, NULL, &varyings_info);
         xfb_info = sh->Program->nir->xfb_info;
         break;
      }
   }

   struct gl_transform_feedback_info *linked_xfb =
      rzalloc(xfb_prog, struct gl_transform_feedback_info);
   xfb_prog->sh.LinkedTransformFeedback = linked_xfb;

   if (!xfb_info) {
      prog->TransformFeedback.NumVarying = 0;
      linked_xfb->NumOutputs = 0;
      linked_xfb->NumVarying = 0;
      linked_xfb->ActiveBuffers = 0;
      return;
   }

   for (unsigned buf = 0; buf < MAX_FEEDBACK_BUFFERS; buf++)
      prog->TransformFeedback.BufferStride[buf] = xfb_info->buffers[buf].stride;

   prog->TransformFeedback.NumVarying = varyings_info->varying_count;
   prog->TransformFeedback.VaryingNames =
      malloc(sizeof(GLchar *) * varyings_info->varying_count);

   linked_xfb->Outputs =
      rzalloc_array(xfb_prog,
                    struct gl_transform_feedback_output,
                    xfb_info->output_count);
   linked_xfb->NumOutputs = xfb_info->output_count;

   linked_xfb->Varyings =
      rzalloc_array(xfb_prog,
                    struct gl_transform_feedback_varying_info,
                    varyings_info->varying_count);
   linked_xfb->NumVarying = varyings_info->varying_count;

   int buffer_index = 0; /* Corresponds to GL_TRANSFORM_FEEDBACK_BUFFER_INDEX */
   int xfb_buffer =
      (varyings_info->varying_count > 0) ?
      xfb_info->outputs[0].buffer : 0;

   for (unsigned i = 0; i < varyings_info->varying_count; i++) {
      nir_xfb_varying_info *xfb_varying = &varyings_info->varyings[i];

      /* From ARB_gl_spirv spec:
       *
       *    "19. How should the program interface query operations behave for
       *         program objects created from SPIR-V shaders?
       *
       *     DISCUSSION: we previously said we didn't need reflection to work
       *     for SPIR-V shaders (at least for the first version), however we
       *     are left with specifying how it should "not work". The primary
       *     issue is that SPIR-V binaries are not required to have names
       *     associated with variables. They can be associated in debug
       *     information, but there is no requirement for that to be present,
       *     and it should not be relied upon."
       *
       *     Options:"
       *
       *     <skip>
       *
       *     "RESOLVED.  Pick (c), but also allow debug names to be returned
       *      if an implementation wants to."
       *
       * So names are considered optional debug info, so the linker needs to
       * work without them, and returning them is optional. For simplicity at
       * this point we are ignoring names
       */
      prog->TransformFeedback.VaryingNames[i] = NULL;

      if (xfb_buffer != xfb_varying->buffer) {
         buffer_index++;
         xfb_buffer = xfb_varying->buffer;
      }

      struct gl_transform_feedback_varying_info *varying =
         linked_xfb->Varyings + i;

      /* ARB_gl_spirv: see above. */
      varying->name.string = NULL;
      resource_name_updated(&varying->name);
      varying->Type = glsl_get_gl_type(xfb_varying->type);
      varying->BufferIndex = buffer_index;
      varying->Size = glsl_type_is_array(xfb_varying->type) ?
         glsl_get_length(xfb_varying->type) : 1;
      varying->Offset = xfb_varying->offset;
   }

   for (unsigned i = 0; i < xfb_info->output_count; i++) {
      nir_xfb_output_info *xfb_output = &xfb_info->outputs[i];

      struct gl_transform_feedback_output *output =
         linked_xfb->Outputs + i;

      output->OutputRegister = xfb_output->location;
      output->OutputBuffer = xfb_output->buffer;
      output->NumComponents = util_bitcount(xfb_output->component_mask);
      output->StreamId = xfb_info->buffer_to_stream[xfb_output->buffer];
      output->DstOffset = xfb_output->offset / 4;
      output->ComponentOffset = xfb_output->component_offset;
   }

   /* Make sure MaxTransformFeedbackBuffers is <= 32 so the bitmask for
    * tracking the number of buffers doesn't overflow.
    */
   unsigned buffers = 0;
   assert(consts->MaxTransformFeedbackBuffers <= sizeof(buffers) * 8);

   for (unsigned buf = 0; buf < MAX_FEEDBACK_BUFFERS; buf++) {
      if (xfb_info->buffers[buf].stride > 0) {
         linked_xfb->Buffers[buf].Stride = xfb_info->buffers[buf].stride / 4;
         linked_xfb->Buffers[buf].NumVaryings = xfb_info->buffers[buf].varying_count;
         buffers |= 1 << buf;
      }
   }

   linked_xfb->ActiveBuffers = buffers;

   ralloc_free(varyings_info);
}

struct nir_xfb_info *
gl_to_nir_xfb_info(struct gl_transform_feedback_info *info, void *mem_ctx)
{
   if (info == NULL || info->NumOutputs == 0)
      return NULL;

   nir_xfb_info *xfb =
      rzalloc_size(mem_ctx, nir_xfb_info_size(info->NumOutputs));

   xfb->output_count = info->NumOutputs;

   for (unsigned i = 0; i < MAX_FEEDBACK_BUFFERS; i++) {
      xfb->buffers[i].stride = info->Buffers[i].Stride * 4;
      xfb->buffers[i].varying_count = info->Buffers[i].NumVaryings;
      xfb->buffer_to_stream[i] = info->Buffers[i].Stream;
   }

   for (unsigned i = 0; i < info->NumOutputs; i++) {
      xfb->outputs[i].buffer = info->Outputs[i].OutputBuffer;
      xfb->outputs[i].offset = info->Outputs[i].DstOffset * 4;
      xfb->outputs[i].location = info->Outputs[i].OutputRegister;
      xfb->outputs[i].component_offset = info->Outputs[i].ComponentOffset;
      xfb->outputs[i].component_mask =
         BITFIELD_RANGE(info->Outputs[i].ComponentOffset,
                        info->Outputs[i].NumComponents);
      xfb->buffers_written |= BITFIELD_BIT(info->Outputs[i].OutputBuffer);
      xfb->streams_written |= BITFIELD_BIT(info->Outputs[i].StreamId);
   }

   return xfb;
}
