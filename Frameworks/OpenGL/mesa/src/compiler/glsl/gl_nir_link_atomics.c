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
#include "linker_util.h"
#include "gl_nir_linker.h"
#include "compiler/glsl/ir_uniform.h" /* for gl_uniform_storage */
#include "main/consts_exts.h"
#include "main/shader_types.h"
#include "util/glheader.h"

/**
 * This file do the common link for GLSL atomic counter uniforms, using NIR,
 * instead of IR as the counter-part glsl/link_uniforms.cpp
 */

struct active_atomic_counter_uniform {
   unsigned loc;
   nir_variable *var;
};

struct active_atomic_buffer {
   struct active_atomic_counter_uniform *uniforms;
   unsigned num_uniforms;
   unsigned uniform_buffer_size;
   unsigned stage_counter_references[MESA_SHADER_STAGES];
   unsigned size;
};

static void
add_atomic_counter(const void *ctx,
                   struct active_atomic_buffer *buffer,
                   unsigned uniform_loc,
                   nir_variable *var)
{
   if (buffer->num_uniforms >= buffer->uniform_buffer_size) {
      if (buffer->uniform_buffer_size == 0)
         buffer->uniform_buffer_size = 1;
      else
         buffer->uniform_buffer_size *= 2;
      buffer->uniforms = reralloc(ctx,
                                  buffer->uniforms,
                                  struct active_atomic_counter_uniform,
                                  buffer->uniform_buffer_size);
   }

   struct active_atomic_counter_uniform *uniform =
      buffer->uniforms + buffer->num_uniforms;
   uniform->loc = uniform_loc;
   uniform->var = var;
   buffer->num_uniforms++;
}

static void
process_atomic_variable(const struct glsl_type *t,
                        struct gl_shader_program *prog,
                        unsigned *uniform_loc,
                        nir_variable *var,
                        struct active_atomic_buffer *buffers,
                        unsigned *num_buffers,
                        int *offset,
                        unsigned shader_stage)
{
   /* FIXME: Arrays of arrays get counted separately. For example:
    * x1[3][3][2] = 9 uniforms, 18 atomic counters
    * x2[3][2]    = 3 uniforms, 6 atomic counters
    * x3[2]       = 1 uniform, 2 atomic counters
    *
    * However this code marks all the counters as active even when they
    * might not be used.
    */
   if (glsl_type_is_array(t) &&
       glsl_type_is_array(glsl_get_array_element(t))) {
      for (unsigned i = 0; i < glsl_get_length(t); i++) {
         process_atomic_variable(glsl_get_array_element(t),
                                 prog,
                                 uniform_loc,
                                 var,
                                 buffers, num_buffers,
                                 offset,
                                 shader_stage);
      }
   } else {
      struct active_atomic_buffer *buf = buffers + var->data.binding;
      struct gl_uniform_storage *const storage =
         &prog->data->UniformStorage[*uniform_loc];

      /* If this is the first time the buffer is used, increment
       * the counter of buffers used.
       */
      if (buf->size == 0)
         (*num_buffers)++;

      add_atomic_counter(buffers, /* ctx */
                         buf,
                         *uniform_loc,
                         var);

      /* When checking for atomic counters we should count every member in
       * an array as an atomic counter reference.
       */
      if (glsl_type_is_array(t))
         buf->stage_counter_references[shader_stage] += glsl_get_length(t);
      else
         buf->stage_counter_references[shader_stage]++;
      buf->size = MAX2(buf->size, *offset + glsl_atomic_size(t));

      storage->offset = *offset;
      *offset += glsl_atomic_size(t);

      (*uniform_loc)++;
   }
}

static struct active_atomic_buffer *
find_active_atomic_counters(const struct gl_constants *consts,
                            struct gl_shader_program *prog,
                            unsigned *num_buffers)
{
   struct active_atomic_buffer *buffers =
      rzalloc_array(NULL, /* ctx */
                    struct active_atomic_buffer,
                    consts->MaxAtomicBufferBindings);
   *num_buffers = 0;

   for (unsigned i = 0; i < MESA_SHADER_STAGES; ++i) {
      struct gl_linked_shader *sh = prog->_LinkedShaders[i];
      if (sh == NULL)
         continue;

      nir_shader *nir = sh->Program->nir;

      nir_foreach_uniform_variable(var, nir) {
         if (!glsl_contains_atomic(var->type))
            continue;

         int offset = var->data.offset;
         unsigned uniform_loc = var->data.location;

         process_atomic_variable(var->type,
                                 prog,
                                 &uniform_loc,
                                 var,
                                 buffers,
                                 num_buffers,
                                 &offset,
                                 i);
      }
   }

   return buffers;
}

static bool
check_atomic_counters_overlap(const nir_variable *x, const nir_variable *y)
{
   return ((x->data.offset >= y->data.offset &&
            x->data.offset < y->data.offset + glsl_atomic_size(y->type)) ||
           (y->data.offset >= x->data.offset &&
            y->data.offset < x->data.offset + glsl_atomic_size(x->type)));
}

static int
cmp_active_counter_offsets(const void *a, const void *b)
{
   const struct active_atomic_counter_uniform *const first =
      (struct active_atomic_counter_uniform *) a;
   const struct active_atomic_counter_uniform *const second =
      (struct active_atomic_counter_uniform *) b;

   return first->var->data.offset - second->var->data.offset;
}

void
gl_nir_link_assign_atomic_counter_resources(const struct gl_constants *consts,
                                            struct gl_shader_program *prog)
{
   unsigned num_buffers;
   unsigned num_atomic_buffers[MESA_SHADER_STAGES] = {0};
   struct active_atomic_buffer *abs =
      find_active_atomic_counters(consts, prog, &num_buffers);

   prog->data->AtomicBuffers =
      rzalloc_array(prog->data, struct gl_active_atomic_buffer, num_buffers);
   prog->data->NumAtomicBuffers = num_buffers;

   unsigned buffer_idx = 0;
   for (unsigned binding = 0;
        binding < consts->MaxAtomicBufferBindings;
        binding++) {

      /* If the binding was not used, skip.
       */
      if (abs[binding].size == 0)
         continue;

      struct active_atomic_buffer *ab = abs + binding;
      struct gl_active_atomic_buffer *mab =
         prog->data->AtomicBuffers + buffer_idx;

      /* Assign buffer-specific fields. */
      mab->Binding = binding;
      mab->MinimumSize = ab->size;
      mab->Uniforms = rzalloc_array(prog->data->AtomicBuffers, GLuint,
                                    ab->num_uniforms);
      mab->NumUniforms = ab->num_uniforms;

      /* Assign counter-specific fields. */
      for (unsigned j = 0; j < ab->num_uniforms; j++) {
         nir_variable *var = ab->uniforms[j].var;
         struct gl_uniform_storage *storage =
            &prog->data->UniformStorage[ab->uniforms[j].loc];

         mab->Uniforms[j] = ab->uniforms[j].loc;

         storage->atomic_buffer_index = buffer_idx;
         storage->offset = var->data.offset;
         if (glsl_type_is_array(var->type)) {
            const struct glsl_type *without_array =
               glsl_without_array(var->type);
            storage->array_stride = glsl_atomic_size(without_array);
         } else {
            storage->array_stride = 0;
         }
         if (!glsl_type_is_matrix(var->type))
            storage->matrix_stride = 0;
      }

      /* Assign stage-specific fields. */
      for (unsigned stage = 0; stage < MESA_SHADER_STAGES; ++stage) {
         if (ab->stage_counter_references[stage]) {
            mab->StageReferences[stage] = GL_TRUE;
            num_atomic_buffers[stage]++;
         } else {
            mab->StageReferences[stage] = GL_FALSE;
         }
      }

      buffer_idx++;
   }

   /* Store a list pointers to atomic buffers per stage and store the index
    * to the intra-stage buffer list in uniform storage.
    */
   for (unsigned stage = 0; stage < MESA_SHADER_STAGES; ++stage) {
      if (prog->_LinkedShaders[stage] == NULL ||
          num_atomic_buffers[stage] <= 0)
         continue;

      struct gl_program *gl_prog = prog->_LinkedShaders[stage]->Program;
      gl_prog->info.num_abos = num_atomic_buffers[stage];
      gl_prog->sh.AtomicBuffers =
         rzalloc_array(gl_prog,
                       struct gl_active_atomic_buffer *,
                       num_atomic_buffers[stage]);

      gl_prog->nir->info.num_abos = num_atomic_buffers[stage];

      unsigned intra_stage_idx = 0;
      for (unsigned i = 0; i < num_buffers; i++) {
         struct gl_active_atomic_buffer *atomic_buffer =
            &prog->data->AtomicBuffers[i];
         if (!atomic_buffer->StageReferences[stage])
            continue;

         gl_prog->sh.AtomicBuffers[intra_stage_idx] = atomic_buffer;

         for (unsigned u = 0; u < atomic_buffer->NumUniforms; u++) {
            GLuint uniform_loc = atomic_buffer->Uniforms[u];
            struct gl_opaque_uniform_index *opaque =
               prog->data->UniformStorage[uniform_loc].opaque + stage;
            opaque->index = intra_stage_idx;
            opaque->active = true;
         }

         intra_stage_idx++;
      }
   }

   assert(buffer_idx == num_buffers);

   ralloc_free(abs);
}

void
gl_nir_link_check_atomic_counter_resources(const struct gl_constants *consts,
                                           struct gl_shader_program *prog)
{
   unsigned num_buffers;
   struct active_atomic_buffer *abs =
      find_active_atomic_counters(consts, prog, &num_buffers);
   unsigned atomic_counters[MESA_SHADER_STAGES] = {0};
   unsigned atomic_buffers[MESA_SHADER_STAGES] = {0};
   unsigned total_atomic_counters = 0;
   unsigned total_atomic_buffers = 0;

   /* Sum the required resources.  Note that this counts buffers and
    * counters referenced by several shader stages multiple times
    * against the combined limit -- That's the behavior the spec
    * requires.
    */
   for (unsigned i = 0; i < consts->MaxAtomicBufferBindings; i++) {
      if (abs[i].size == 0)
         continue;

      qsort(abs[i].uniforms, abs[i].num_uniforms,
            sizeof(struct active_atomic_counter_uniform),
            cmp_active_counter_offsets);

      for (unsigned j = 1; j < abs[i].num_uniforms; j++) {
         /* If an overlapping counter found, it must be a reference to the
          * same counter from a different shader stage.
          */
         if (check_atomic_counters_overlap(abs[i].uniforms[j-1].var,
                                           abs[i].uniforms[j].var)
             && strcmp(abs[i].uniforms[j-1].var->name,
                       abs[i].uniforms[j].var->name) != 0) {
            linker_error(prog, "Atomic counter %s declared at offset %d "
                         "which is already in use.",
                         abs[i].uniforms[j].var->name,
                         abs[i].uniforms[j].var->data.offset);
         }
      }

      for (unsigned j = 0; j < MESA_SHADER_STAGES; ++j) {
         const unsigned n = abs[i].stage_counter_references[j];

         if (n) {
            atomic_counters[j] += n;
            total_atomic_counters += n;
            atomic_buffers[j]++;
            total_atomic_buffers++;
         }
      }
   }

   /* Check that they are within the supported limits. */
   for (unsigned i = 0; i < MESA_SHADER_STAGES; i++) {
      if (atomic_counters[i] > consts->Program[i].MaxAtomicCounters)
         linker_error(prog, "Too many %s shader atomic counters",
                      _mesa_shader_stage_to_string(i));

      if (atomic_buffers[i] > consts->Program[i].MaxAtomicBuffers)
         linker_error(prog, "Too many %s shader atomic counter buffers",
                      _mesa_shader_stage_to_string(i));
   }

   if (total_atomic_counters > consts->MaxCombinedAtomicCounters)
      linker_error(prog, "Too many combined atomic counters");

   if (total_atomic_buffers > consts->MaxCombinedAtomicBuffers)
      linker_error(prog, "Too many combined atomic buffers");

   ralloc_free(abs);
}
