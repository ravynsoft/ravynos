/*
 * Copyright © 2011 Intel Corporation
 * Copyright © 2022 Valve Corporation
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
 * This lowering pass generates GLSL code that manually packs varyings into
 * vec4 slots, for the benefit of back-ends that don't support packed varyings
 * natively.
 *
 * For example, the following shader:
 *
 *   out mat3x2 foo;  // location=4, location_frac=0
 *   out vec3 bar[2]; // location=5, location_frac=2
 *
 *   main()
 *   {
 *     ...
 *   }
 *
 * Is rewritten to:
 *
 *   mat3x2 foo;
 *   vec3 bar[2];
 *   out vec4 packed4; // location=4, location_frac=0
 *   out vec4 packed5; // location=5, location_frac=0
 *   out vec4 packed6; // location=6, location_frac=0
 *
 *   main()
 *   {
 *     ...
 *     packed4.xy = foo[0];
 *     packed4.zw = foo[1];
 *     packed5.xy = foo[2];
 *     packed5.zw = bar[0].xy;
 *     packed6.x = bar[0].z;
 *     packed6.yzw = bar[1];
 *   }
 *
 * This lowering pass properly handles "double parking" of a varying vector
 * across two varying slots.  For example, in the code above, two of the
 * components of bar[0] are stored in packed5, and the remaining component is
 * stored in packed6.
 *
 * Note that in theory, the extra instructions may cause some loss of
 * performance.  However, hopefully in most cases the performance loss will
 * either be absorbed by a later optimization pass, or it will be offset by
 * memory bandwidth savings (because fewer varyings are used).
 *
 * This lowering pass also packs flat floats, ints, and uints together, by
 * using ivec4 as the base type of flat "varyings", and using appropriate
 * casts to convert floats and uints into ints.
 *
 * This lowering pass also handles varyings whose type is a struct or an array
 * of struct.  Structs are packed in order and with no gaps, so there may be a
 * performance penalty due to structure elements being double-parked.
 *
 * Lowering of geometry shader inputs is slightly more complex, since geometry
 * inputs are always arrays, so we need to lower arrays to arrays.  For
 * example, the following input:
 *
 *   in struct Foo {
 *     float f;
 *     vec3 v;
 *     vec2 a[2];
 *   } arr[3];         // location=4, location_frac=0
 *
 * Would get lowered like this if it occurred in a fragment shader:
 *
 *   struct Foo {
 *     float f;
 *     vec3 v;
 *     vec2 a[2];
 *   } arr[3];
 *   in vec4 packed4;  // location=4, location_frac=0
 *   in vec4 packed5;  // location=5, location_frac=0
 *   in vec4 packed6;  // location=6, location_frac=0
 *   in vec4 packed7;  // location=7, location_frac=0
 *   in vec4 packed8;  // location=8, location_frac=0
 *   in vec4 packed9;  // location=9, location_frac=0
 *
 *   main()
 *   {
 *     arr[0].f = packed4.x;
 *     arr[0].v = packed4.yzw;
 *     arr[0].a[0] = packed5.xy;
 *     arr[0].a[1] = packed5.zw;
 *     arr[1].f = packed6.x;
 *     arr[1].v = packed6.yzw;
 *     arr[1].a[0] = packed7.xy;
 *     arr[1].a[1] = packed7.zw;
 *     arr[2].f = packed8.x;
 *     arr[2].v = packed8.yzw;
 *     arr[2].a[0] = packed9.xy;
 *     arr[2].a[1] = packed9.zw;
 *     ...
 *   }
 *
 * But it would get lowered like this if it occurred in a geometry shader:
 *
 *   struct Foo {
 *     float f;
 *     vec3 v;
 *     vec2 a[2];
 *   } arr[3];
 *   in vec4 packed4[3];  // location=4, location_frac=0
 *   in vec4 packed5[3];  // location=5, location_frac=0
 *
 *   main()
 *   {
 *     arr[0].f = packed4[0].x;
 *     arr[0].v = packed4[0].yzw;
 *     arr[0].a[0] = packed5[0].xy;
 *     arr[0].a[1] = packed5[0].zw;
 *     arr[1].f = packed4[1].x;
 *     arr[1].v = packed4[1].yzw;
 *     arr[1].a[0] = packed5[1].xy;
 *     arr[1].a[1] = packed5[1].zw;
 *     arr[2].f = packed4[2].x;
 *     arr[2].v = packed4[2].yzw;
 *     arr[2].a[0] = packed5[2].xy;
 *     arr[2].a[1] = packed5[2].zw;
 *     ...
 *   }
 */

#include "nir.h"
#include "nir_builder.h"
#include "gl_nir.h"
#include "gl_nir_linker.h"
#include "program/prog_instruction.h"
#include "main/mtypes.h"

/**
 * Visitor that performs varying packing.  For each varying declared in the
 * shader, this visitor determines whether it needs to be packed.  If so, it
 * demotes it to an ordinary global, creates new packed varyings, and
 * generates assignments to convert between the original varying and the
 * packed varying.
 */
struct lower_packed_varyings_state
{
   const struct gl_constants *consts;

   struct gl_shader_program *prog;

   /**
    * Memory context used to allocate new instructions for the shader.
    */
   void *mem_ctx;

   /**
    * Number of generic varying slots which are used by this shader.  This is
    * used to allocate temporary intermediate data structures.  If any varying
    * used by this shader has a location greater than or equal to
    * VARYING_SLOT_VAR0 + locations_used, an assertion will fire.
    */
   unsigned locations_used;

   const uint8_t* components;

   /**
    * Array of pointers to the packed varyings that have been created for each
    * generic varying slot.  NULL entries in this array indicate varying slots
    * for which a packed varying has not been created yet.
    */
   nir_variable **packed_varyings;

   nir_shader *shader;

   nir_function_impl *impl;

   nir_builder b;

   /**
    * Type of varying which is being lowered in this pass (either
    * nir_var_shader_in or ir_var_shader_out).
    */
   nir_variable_mode mode;

   /**
    * If we are currently lowering geometry shader inputs, the number of input
    * vertices the geometry shader accepts.  Otherwise zero.
    */
   unsigned gs_input_vertices;

   bool disable_varying_packing;
   bool disable_xfb_packing;
   bool xfb_enabled;
   bool ifc_exposed_to_query_api;
};

bool
lower_packed_varying_needs_lowering(nir_shader *shader, nir_variable *var,
                                    bool xfb_enabled, bool disable_xfb_packing,
                                    bool disable_varying_packing)
{
   /* Things composed of vec4's, varyings with explicitly assigned
    * locations or varyings marked as must_be_shader_input (which might be used
    * by interpolateAt* functions) shouldn't be lowered. Everything else can be.
    */
   if (var->data.explicit_location || var->data.must_be_shader_input)
      return false;

   const struct glsl_type *type = var->type;
   if (nir_is_arrayed_io(var, shader->info.stage) || var->data.per_view) {
      assert(glsl_type_is_array(type));
      type = glsl_get_array_element(type);
   }

   /* Some drivers (e.g. panfrost) don't support packing of transform
    * feedback varyings.
    */
   if (disable_xfb_packing && var->data.is_xfb &&
       !(glsl_type_is_array(type) || glsl_type_is_struct(type) || glsl_type_is_matrix(type)) &&
       xfb_enabled)
      return false;

   /* Override disable_varying_packing if the var is only used by transform
    * feedback. Also override it if transform feedback is enabled and the
    * variable is an array, struct or matrix as the elements of these types
    * will always have the same interpolation and therefore are safe to pack.
    */
   if (disable_varying_packing && !var->data.is_xfb_only &&
       !((glsl_type_is_array(type) || glsl_type_is_struct(type) || glsl_type_is_matrix(type)) &&
         xfb_enabled))
      return false;

   type = glsl_without_array(type);
   if (glsl_get_vector_elements(type) == 4 && !glsl_type_is_64bit(type))
      return false;
   return true;
}

/**
 * If no packed varying has been created for the given varying location yet,
 * create it and add it to the shader.
 *
 * The newly created varying inherits its interpolation parameters from \c
 * unpacked_var.  Its base type is ivec4 if we are lowering a flat varying,
 * vec4 otherwise.
 */
static void
create_or_update_packed_varying(struct lower_packed_varyings_state *state,
                                nir_variable *unpacked_var,
                                const char *name, unsigned location,
                                unsigned slot, unsigned vertex_index)
{
   assert(slot < state->locations_used);
   if (state->packed_varyings[slot] == NULL) {
      assert(state->components[slot] != 0);
      assert(name);

      nir_variable *packed_var = rzalloc(state->shader, nir_variable);
      packed_var->name = ralloc_asprintf(packed_var, "packed:%s", name);
      packed_var->data.mode = state->mode;

      bool is_interpolation_flat =
         unpacked_var->data.interpolation == INTERP_MODE_FLAT ||
         glsl_contains_integer(unpacked_var->type) ||
         glsl_contains_double(unpacked_var->type);

      const struct glsl_type *packed_type;
      if (is_interpolation_flat)
         packed_type = glsl_vector_type(GLSL_TYPE_INT, state->components[slot]);
      else
         packed_type = glsl_vector_type(GLSL_TYPE_FLOAT, state->components[slot]);

      if (state->gs_input_vertices != 0) {
         packed_type =
            glsl_array_type(packed_type, state->gs_input_vertices, 0);
      }

      packed_var->type = packed_type;
      packed_var->data.centroid = unpacked_var->data.centroid;
      packed_var->data.sample = unpacked_var->data.sample;
      packed_var->data.patch = unpacked_var->data.patch;
      packed_var->data.interpolation = is_interpolation_flat ?
         (unsigned) INTERP_MODE_FLAT : unpacked_var->data.interpolation;
      packed_var->data.location = location;
      packed_var->data.precision = unpacked_var->data.precision;
      packed_var->data.always_active_io = unpacked_var->data.always_active_io;
      packed_var->data.stream = NIR_STREAM_PACKED;

      nir_shader_add_variable(state->shader, packed_var);
      state->packed_varyings[slot] = packed_var;
   } else {
      nir_variable *var = state->packed_varyings[slot];

      /* The slot needs to be marked as always active if any variable that got
       * packed there was.
       */
      var->data.always_active_io |= unpacked_var->data.always_active_io;

      /* For geometry shader inputs, only update the packed variable name the
       * first time we visit each component.
       */
      if (state->gs_input_vertices == 0 || vertex_index == 0) {
         assert(name);
         ralloc_asprintf_append((char **) &var->name, ",%s", name);
      }
   }
}

/**
 * Retrieve the packed varying corresponding to the given varying location.
 *
 * \param vertex_index: if we are lowering geometry shader inputs, then this
 * indicates which vertex we are currently lowering.  Otherwise it is ignored.
 */
static nir_deref_instr *
get_packed_varying_deref(struct lower_packed_varyings_state *state,
                         unsigned location, nir_variable *unpacked_var,
                         const char *name, unsigned vertex_index)
{
   unsigned slot = location - VARYING_SLOT_VAR0;
   assert(slot < state->locations_used);

   create_or_update_packed_varying(state, unpacked_var, name, location, slot,
                                   vertex_index);

   nir_deref_instr *deref =
      nir_build_deref_var(&state->b, state->packed_varyings[slot]);

   if (state->gs_input_vertices != 0) {
      /* When lowering GS inputs, the packed variable is an array, so we need
       * to dereference it using vertex_index.
       */
      nir_load_const_instr *c_idx =
         nir_load_const_instr_create(state->b.shader, 1, 32);
      c_idx->value[0].u32 = vertex_index;
      nir_builder_instr_insert(&state->b, &c_idx->instr);

      deref = nir_build_deref_array(&state->b, deref, &c_idx->def);
   }

   return deref;
}

struct packing_store_values {
   bool is_64bit;
   unsigned writemasks[2];
   nir_def *values[2];
   nir_deref_instr *deref;
};

/**
 * Make an ir_assignment from \c rhs to \c lhs, performing appropriate
 * bitcasts if necessary to match up types.
 *
 * This function is called when packing varyings.
 */
static struct packing_store_values *
bitwise_assign_pack(struct lower_packed_varyings_state *state,
                    nir_deref_instr *packed_deref,
                    nir_deref_instr *unpacked_deref,
                    const struct glsl_type *unpacked_type,
                    nir_def *value,
                    unsigned writemask)

{
   nir_variable *packed_var = nir_deref_instr_get_variable(packed_deref);

   enum glsl_base_type packed_base_type = glsl_get_base_type(packed_var->type);
   enum glsl_base_type unpacked_base_type = glsl_get_base_type(unpacked_type);

   struct packing_store_values *store_state =
      calloc(1, sizeof(struct packing_store_values));

   if (unpacked_base_type != packed_base_type) {
      /* Since we only mix types in flat varyings, and we always store flat
       * varyings as type ivec4, we need only produce conversions from (uint
       * or float) to int.
       */
      assert(packed_base_type == GLSL_TYPE_INT);
      switch (unpacked_base_type) {
      case GLSL_TYPE_UINT:
      case GLSL_TYPE_FLOAT:
         value = nir_mov(&state->b, value);
         break;
      case GLSL_TYPE_DOUBLE:
      case GLSL_TYPE_UINT64:
      case GLSL_TYPE_INT64:
         assert(glsl_get_vector_elements(unpacked_type) <= 2);
         if (glsl_get_vector_elements(unpacked_type) == 2) {
            assert(glsl_get_vector_elements(packed_var->type) == 4);

            unsigned swiz_x = 0;
            unsigned writemask = 0x3;
            nir_def *swizzle = nir_swizzle(&state->b, value, &swiz_x, 1);

            store_state->is_64bit = true;
            store_state->deref = packed_deref;
            store_state->values[0] = nir_unpack_64_2x32(&state->b, swizzle);
            store_state->writemasks[0] = writemask;

            unsigned swiz_y = 1;
            writemask = 0xc;
            swizzle = nir_swizzle(&state->b, value, &swiz_y, 1);

            store_state->deref = packed_deref;
            store_state->values[1] = nir_unpack_64_2x32(&state->b, swizzle);
            store_state->writemasks[1] = writemask;
            return store_state;
         } else {
            value = nir_unpack_64_2x32(&state->b, value);
         }
         break;
      case GLSL_TYPE_SAMPLER:
      case GLSL_TYPE_IMAGE:
         value = nir_unpack_64_2x32(&state->b, value);
         break;
      default:
         assert(!"Unexpected type conversion while lowering varyings");
         break;
      }
   }

   store_state->deref = packed_deref;
   store_state->values[0] = value;
   store_state->writemasks[0] = writemask;

   return store_state;
}

/**
 * This function is called when unpacking varyings.
 */
static struct packing_store_values *
bitwise_assign_unpack(struct lower_packed_varyings_state *state,
                      nir_deref_instr *unpacked_deref,
                      nir_deref_instr *packed_deref,
                      const struct glsl_type *unpacked_type,
                      nir_def *value, unsigned writemask)
{
   nir_variable *packed_var = nir_deref_instr_get_variable(packed_deref);

   const struct glsl_type *packed_type = glsl_without_array(packed_var->type);
   enum glsl_base_type packed_base_type = glsl_get_base_type(packed_type);
   enum glsl_base_type unpacked_base_type = glsl_get_base_type(unpacked_type);

   struct packing_store_values *store_state =
      calloc(1, sizeof(struct packing_store_values));

   if (unpacked_base_type != packed_base_type) {
      /* Since we only mix types in flat varyings, and we always store flat
       * varyings as type ivec4, we need only produce conversions from int to
       * (uint or float).
       */
      assert(packed_base_type == GLSL_TYPE_INT);

      switch (unpacked_base_type) {
      case GLSL_TYPE_UINT:
      case GLSL_TYPE_FLOAT:
         value = nir_mov(&state->b, value);
         break;
      case GLSL_TYPE_DOUBLE:
      case GLSL_TYPE_UINT64:
      case GLSL_TYPE_INT64:
         assert(glsl_get_vector_elements(unpacked_type) <= 2);
         if (glsl_get_vector_elements(unpacked_type) == 2) {
            assert(glsl_get_vector_elements(packed_type) == 4);

            unsigned swiz_xy[2] = {0, 1};
            writemask = 1 << (ffs(writemask) - 1);

            store_state->is_64bit = true;
            store_state->deref = unpacked_deref;
            store_state->values[0] =
               nir_pack_64_2x32(&state->b,
                                nir_swizzle(&state->b, value, swiz_xy, 2));
            store_state->writemasks[0] = writemask;

            unsigned swiz_zw[2] = {2, 3};
            writemask = writemask << 1;

            store_state->deref = unpacked_deref;
            store_state->values[1] =
               nir_pack_64_2x32(&state->b,
                                nir_swizzle(&state->b, value, swiz_zw, 2));
            store_state->writemasks[1] = writemask;

            return store_state;
         } else {
            value = nir_pack_64_2x32(&state->b, value);
         }
         break;
      case GLSL_TYPE_SAMPLER:
      case GLSL_TYPE_IMAGE:
         value = nir_pack_64_2x32(&state->b, value);
         break;
      default:
         assert(!"Unexpected type conversion while lowering varyings");
         break;
      }
   }

   store_state->deref = unpacked_deref;
   store_state->values[0] = value;
   store_state->writemasks[0] = writemask;

   return store_state;
}

static void
create_store_deref(struct lower_packed_varyings_state *state,
                   nir_deref_instr *deref, nir_def *value,
                   unsigned writemask, bool is_64bit)
{
   /* If dest and value have different number of components pack the srcs
    * into a vector.
    */
   const struct glsl_type *type = glsl_without_array(deref->type);
   unsigned comps = glsl_get_vector_elements(type);
   if (value->num_components != comps) {
      nir_def *srcs[4];

      unsigned comp = 0;
      for (unsigned i = 0; i < comps; i++) {
         if (writemask & (1 << i)) {
            if (is_64bit && state->mode == nir_var_shader_in)
               srcs[i] = value;
            else
               srcs[i] = nir_swizzle(&state->b, value, &comp, 1);
            comp++;
         } else {
            srcs[i] = nir_undef(&state->b, 1,
                                    glsl_type_is_64bit(type) ? 64 : 32);
         }
      }
      value = nir_vec(&state->b, srcs, comps);
   }

   nir_store_deref(&state->b, deref, value, writemask);
}

static unsigned
lower_varying(struct lower_packed_varyings_state *state,
              nir_def *rhs_swizzle, unsigned writemask,
              const struct glsl_type *type, unsigned fine_location,
              nir_variable *unpacked_var, nir_deref_instr *unpacked_var_deref,
              const char *name, bool gs_input_toplevel, unsigned vertex_index);

/**
 * Recursively pack or unpack a varying for which we need to iterate over its
 * constituent elements.
 * This takes care of both arrays and matrices.
 *
 * \param gs_input_toplevel should be set to true if we are lowering geometry
 * shader inputs, and we are currently lowering the whole input variable
 * (i.e. we are lowering the array whose index selects the vertex).
 *
 * \param vertex_index: if we are lowering geometry shader inputs, and the
 * level of the array that we are currently lowering is *not* the top level,
 * then this indicates which vertex we are currently lowering.  Otherwise it
 * is ignored.
 */
static unsigned
lower_arraylike(struct lower_packed_varyings_state *state,
                nir_def *rhs_swizzle, unsigned writemask,
                const struct glsl_type *type, unsigned fine_location,
                nir_variable *unpacked_var, nir_deref_instr *unpacked_var_deref,
                const char *name, bool gs_input_toplevel, unsigned vertex_index)
{
   unsigned array_size = glsl_get_length(type);
   unsigned dmul = glsl_type_is_64bit(glsl_without_array(type)) ? 2 : 1;
   if (array_size * dmul + fine_location % 4 > 4) {
      fine_location = ALIGN_POT(fine_location, dmul);
   }

   type = glsl_get_array_element(type);
   for (unsigned i = 0; i < array_size; i++) {
      nir_load_const_instr *c_idx =
         nir_load_const_instr_create(state->b.shader, 1, 32);
      c_idx->value[0].u32 = i;
      nir_builder_instr_insert(&state->b, &c_idx->instr);

      nir_deref_instr *unpacked_array_deref =
         nir_build_deref_array(&state->b, unpacked_var_deref, &c_idx->def);

      if (gs_input_toplevel) {
         /* Geometry shader inputs are a special case.  Instead of storing
          * each element of the array at a different location, all elements
          * are at the same location, but with a different vertex index.
          */
         (void) lower_varying(state, rhs_swizzle, writemask, type, fine_location,
                              unpacked_var, unpacked_array_deref, name, false, i);
      } else {
         char *subscripted_name = name ?
            ralloc_asprintf(state->mem_ctx, "%s[%d]", name, i) : NULL;
         fine_location =
            lower_varying(state, rhs_swizzle, writemask, type, fine_location,
                          unpacked_var, unpacked_array_deref,
                          subscripted_name, false, vertex_index);
      }
   }

   return fine_location;
}

/**
 * Recursively pack or unpack the given varying (or portion of a varying) by
 * traversing all of its constituent vectors.
 *
 * \param fine_location is the location where the first constituent vector
 * should be packed--the word "fine" indicates that this location is expressed
 * in multiples of a float, rather than multiples of a vec4 as is used
 * elsewhere in Mesa.
 *
 * \param gs_input_toplevel should be set to true if we are lowering geometry
 * shader inputs, and we are currently lowering the whole input variable
 * (i.e. we are lowering the array whose index selects the vertex).
 *
 * \param vertex_index: if we are lowering geometry shader inputs, and the
 * level of the array that we are currently lowering is *not* the top level,
 * then this indicates which vertex we are currently lowering.  Otherwise it
 * is ignored.
 *
 * \return the location where the next constituent vector (after this one)
 * should be packed.
 */
static unsigned
lower_varying(struct lower_packed_varyings_state *state,
              nir_def *rhs_swizzle, unsigned writemask,
              const struct glsl_type *type, unsigned fine_location,
              nir_variable *unpacked_var, nir_deref_instr *unpacked_var_deref,
              const char *name, bool gs_input_toplevel, unsigned vertex_index)
{
   unsigned dmul = glsl_type_is_64bit(type) ? 2 : 1;
   /* When gs_input_toplevel is set, we should be looking at a geometry shader
    * input array.
    */
   assert(!gs_input_toplevel || glsl_type_is_array(type));

   if (glsl_type_is_struct(type)) {
      unsigned struct_len = glsl_get_length(type);
      for (unsigned i = 0; i < struct_len; i++) {
         const char *field_name = glsl_get_struct_elem_name(type, i);
         char *deref_name = name ?
            ralloc_asprintf(state->mem_ctx, "%s.%s", name, field_name) :
            NULL;
         const struct glsl_type *field_type = glsl_get_struct_field(type, i);

         nir_deref_instr *unpacked_struct_deref =
            nir_build_deref_struct(&state->b, unpacked_var_deref, i);
         fine_location = lower_varying(state, rhs_swizzle, writemask, field_type,
                                       fine_location, unpacked_var,
                                       unpacked_struct_deref, deref_name,
                                       false, vertex_index);
      }

      return fine_location;
   } else if (glsl_type_is_array(type)) {
      /* Arrays are packed/unpacked by considering each array element in
       * sequence.
       */
      return lower_arraylike(state, rhs_swizzle, writemask, type, fine_location,
                             unpacked_var, unpacked_var_deref, name,
                             gs_input_toplevel, vertex_index);
   } else if (glsl_type_is_matrix(type)) {
      /* Matrices are packed/unpacked by considering each column vector in
       * sequence.
       */
      return lower_arraylike(state, rhs_swizzle, writemask, type, fine_location,
                             unpacked_var, unpacked_var_deref, name, false,
                             vertex_index);
   } else if (glsl_get_vector_elements(type) * dmul + fine_location % 4 > 4) {
      /* We don't have code to split up 64bit variable between two
       * varying slots, instead we add padding if necessary.
       */
      unsigned aligned_fine_location = ALIGN_POT(fine_location, dmul);
      if (aligned_fine_location != fine_location) {
         return lower_varying(state, rhs_swizzle, writemask, type,
                              aligned_fine_location, unpacked_var,
                              unpacked_var_deref, name, false, vertex_index);
      }

      /* This vector is going to be "double parked" across two varying slots,
       * so handle it as two separate assignments. For doubles, a dvec3/dvec4
       * can end up being spread over 3 slots. However the second splitting
       * will happen later, here we just always want to split into 2.
       */
      unsigned left_components, right_components;
      unsigned left_swizzle_values[4] = { 0, 0, 0, 0 };
      unsigned right_swizzle_values[4] = { 0, 0, 0, 0 };
      char left_swizzle_name[4] = { 0, 0, 0, 0 };
      char right_swizzle_name[4] = { 0, 0, 0, 0 };

      left_components = 4 - fine_location % 4;
      if (glsl_type_is_64bit(type)) {
         left_components /= 2;
         assert(left_components > 0);
      }
      right_components = glsl_get_vector_elements(type) - left_components;

      /* If set use previously set writemask to offset the following
       * swizzle/writemasks. This can happen when spliting a dvec, etc across
       * slots.
       */
      unsigned offset = 0;
      if (writemask) {
         for (unsigned i = 0; i < left_components; i++) {
            /* Keep going until we find the first component of the write */
            if (!(writemask & (1 << i))) {
               offset++;
            } else
               break;
         }
      }

      for (unsigned i = 0; i < left_components; i++) {
         left_swizzle_values[i] = i + offset;
         left_swizzle_name[i] = "xyzw"[i + offset];
      }
      for (unsigned i = 0; i < right_components; i++) {
         right_swizzle_values[i] = i + left_components + offset;
         right_swizzle_name[i] = "xyzw"[i + left_components + offset];
      }

      if (left_components) {
         char *left_name = name ?
            ralloc_asprintf(state->mem_ctx, "%s.%s", name, left_swizzle_name) :
            NULL;

         nir_def *left_swizzle = NULL;
         unsigned left_writemask = ~0u;
         if (state->mode == nir_var_shader_out) {
            nir_def *ssa_def = rhs_swizzle ?
               rhs_swizzle : nir_load_deref(&state->b, unpacked_var_deref);
            left_swizzle =
               nir_swizzle(&state->b, ssa_def,
                           left_swizzle_values, left_components);
         } else {
            left_writemask = ((1 << left_components) - 1) << offset;
         }

         const struct glsl_type *swiz_type =
            glsl_vector_type(glsl_get_base_type(type), left_components);
         fine_location = lower_varying(state, left_swizzle, left_writemask, swiz_type,
                                       fine_location, unpacked_var, unpacked_var_deref,
                                       left_name, false, vertex_index);
      } else {
         /* Top up the fine location to the next slot */
         fine_location++;
      }

      char *right_name = name ?
         ralloc_asprintf(state->mem_ctx, "%s.%s", name, right_swizzle_name) :
         NULL;

      nir_def *right_swizzle = NULL;
      unsigned right_writemask = ~0u;
      if (state->mode == nir_var_shader_out) {
        nir_def *ssa_def = rhs_swizzle ?
           rhs_swizzle : nir_load_deref(&state->b, unpacked_var_deref);
        right_swizzle =
           nir_swizzle(&state->b, ssa_def,
                       right_swizzle_values, right_components);
      } else {
         right_writemask = ((1 << right_components) - 1) << (left_components + offset);
      }

      const struct glsl_type *swiz_type =
         glsl_vector_type(glsl_get_base_type(type), right_components);
      return lower_varying(state, right_swizzle, right_writemask, swiz_type,
                           fine_location, unpacked_var, unpacked_var_deref,
                           right_name, false, vertex_index);
   } else {
      /* No special handling is necessary; (un)pack the old varying (now temp)
       * from/into the new packed varying.
       */
      unsigned components = glsl_get_vector_elements(type) * dmul;
      unsigned location = fine_location / 4;
      unsigned location_frac = fine_location % 4;

      assert(state->components[location - VARYING_SLOT_VAR0] >= components);
      nir_deref_instr *packed_deref =
         get_packed_varying_deref(state, location, unpacked_var, name,
                                  vertex_index);

      nir_variable *packed_var =
         state->packed_varyings[location - VARYING_SLOT_VAR0];
      if (unpacked_var->data.stream != 0) {
         assert(unpacked_var->data.stream < 4);
         for (unsigned i = 0; i < components; ++i) {
            packed_var->data.stream |=
               unpacked_var->data.stream << (2 * (location_frac + i));
         }
      }

      struct packing_store_values *store_value;
      if (state->mode == nir_var_shader_out) {
         unsigned writemask = ((1 << components) - 1) << location_frac;
         nir_def *value = rhs_swizzle ? rhs_swizzle :
            nir_load_deref(&state->b, unpacked_var_deref);

         store_value =
            bitwise_assign_pack(state, packed_deref, unpacked_var_deref, type,
                                value, writemask);
      } else {
         unsigned swizzle_values[4] = { 0, 0, 0, 0 };
         for (unsigned i = 0; i < components; ++i) {
            swizzle_values[i] = i + location_frac;
         }

         nir_def *ssa_def = &packed_deref->def;
         ssa_def = nir_load_deref(&state->b, packed_deref);
         nir_def *swizzle =
            nir_swizzle(&state->b, ssa_def, swizzle_values, components);

         store_value = bitwise_assign_unpack(state, unpacked_var_deref,
                                             packed_deref, type, swizzle,
                                             writemask);
      }

      create_store_deref(state, store_value->deref, store_value->values[0],
                         store_value->writemasks[0], store_value->is_64bit);
      if (store_value->is_64bit) {
         create_store_deref(state, store_value->deref, store_value->values[1],
                            store_value->writemasks[1], store_value->is_64bit);
      }

      free(store_value);
      return fine_location + components;
   }
}

/* Recursively pack varying. */
static void
pack_output_var(struct lower_packed_varyings_state *state, nir_variable *var)
{
   nir_deref_instr *unpacked_var_deref = nir_build_deref_var(&state->b, var);
   lower_varying(state, NULL, ~0u, var->type,
                 var->data.location * 4 + var->data.location_frac,
                 var, unpacked_var_deref, var->name,
                 state->gs_input_vertices != 0, 0);
}

static void
lower_output_var(struct lower_packed_varyings_state *state, nir_variable *var)
{
   if (var->data.mode != state->mode ||
       var->data.location < VARYING_SLOT_VAR0 ||
       !lower_packed_varying_needs_lowering(state->shader, var,
                                            state->xfb_enabled,
                                            state->disable_xfb_packing,
                                            state->disable_varying_packing))
      return;

      /* Skip any new packed varyings we just added */
   if (strncmp("packed:", var->name, 7) == 0)
      return;

   /* This lowering pass is only capable of packing floats and ints
    * together when their interpolation mode is "flat".  Treat integers as
    * being flat when the interpolation mode is none.
    */
   assert(var->data.interpolation == INTERP_MODE_FLAT ||
          var->data.interpolation == INTERP_MODE_NONE ||
          !glsl_contains_integer(var->type));

   if (state->prog->SeparateShader && state->ifc_exposed_to_query_api) {
      struct set *resource_set = _mesa_pointer_set_create(NULL);

      nir_add_packed_var_to_resource_list(state->consts, state->prog,
                                          resource_set, var,
                                          state->shader->info.stage,
                                          GL_PROGRAM_OUTPUT);

      _mesa_set_destroy(resource_set, NULL);
   }

   /* Change the old varying into an ordinary global. */
   var->data.mode = nir_var_shader_temp;

   nir_foreach_block(block, state->impl) {
      if (state->shader->info.stage != MESA_SHADER_GEOMETRY) {
         /* For shaders other than geometry, outputs need to be lowered before
          * each return statement and at the end of main()
          */
         if (nir_block_ends_in_return_or_halt(block)) {
            state->b.cursor = nir_before_instr(nir_block_last_instr(block));
            pack_output_var(state, var);
         } else if (block == nir_impl_last_block(state->impl)) {
            state->b.cursor = nir_after_block(block);
            pack_output_var(state, var);
         }
      } else {
        /* For geometry shaders, outputs need to be lowered before each call
         * to EmitVertex()
         */
         nir_foreach_instr_safe(instr, block) {
            if (instr->type != nir_instr_type_intrinsic)
               continue;

            nir_intrinsic_instr *intrin = nir_instr_as_intrinsic(instr);
            if (intrin->intrinsic != nir_intrinsic_emit_vertex)
               continue;

            state->b.cursor = nir_before_instr(instr);
            pack_output_var(state, var);
         }
      }
   }
}

static void
lower_packed_outputs(struct lower_packed_varyings_state *state)
{
   nir_foreach_shader_out_variable_safe(var, state->shader) {
      lower_output_var(state, var);
   }
}

static void
lower_packed_inputs(struct lower_packed_varyings_state *state)
{
   /* Shader inputs need to be lowered at the beginning of main() so set bulder
    * cursor to insert packing code at the start of the main function.
    */
   state->b.cursor = nir_before_impl(state->impl);

   /* insert new varyings, lower old ones to locals and add unpacking code a
    * the start of the shader.
    */
   nir_foreach_shader_in_variable_safe(var, state->shader) {
      if (var->data.mode != state->mode ||
          var->data.location < VARYING_SLOT_VAR0 ||
          !lower_packed_varying_needs_lowering(state->shader, var,
                                               state->xfb_enabled,
                                               state->disable_xfb_packing,
                                               state->disable_varying_packing))
         continue;

      /* Skip any new packed varyings we just added */
      if (strncmp("packed:", var->name, 7) == 0)
         continue;

      /* This lowering pass is only capable of packing floats and ints
       * together when their interpolation mode is "flat".  Treat integers as
       * being flat when the interpolation mode is none.
       */
      assert(var->data.interpolation == INTERP_MODE_FLAT ||
             var->data.interpolation == INTERP_MODE_NONE ||
             !glsl_contains_integer(var->type));

      /* Program interface needs to expose varyings in case of SSO. Add the
       * variable for program resource list before it gets modified and lost.
       */
      if (state->prog->SeparateShader && state->ifc_exposed_to_query_api) {
         struct set *resource_set = _mesa_pointer_set_create(NULL);

         nir_add_packed_var_to_resource_list(state->consts, state->prog,
                                             resource_set, var,
                                             state->shader->info.stage,
                                             GL_PROGRAM_INPUT);

         _mesa_set_destroy(resource_set, NULL);
      }

      /* Change the old varying into an ordinary global. */
      var->data.mode = nir_var_shader_temp;

      /* Recursively unpack varying. */
      nir_deref_instr *unpacked_var_deref = nir_build_deref_var(&state->b, var);
      lower_varying(state, NULL, ~0u, var->type,
                    var->data.location * 4 + var->data.location_frac,
                    var, unpacked_var_deref, var->name,
                    state->gs_input_vertices != 0, 0);
   }
}

void
gl_nir_lower_packed_varyings(const struct gl_constants *consts,
                             struct gl_shader_program *prog,
                             void *mem_ctx, unsigned locations_used,
                             const uint8_t *components,
                             nir_variable_mode mode, unsigned gs_input_vertices,
                             struct gl_linked_shader *linked_shader,
                             bool disable_varying_packing,
                             bool disable_xfb_packing, bool xfb_enabled)
{
   struct lower_packed_varyings_state state;
   nir_shader *shader = linked_shader->Program->nir;
   nir_function_impl *impl = nir_shader_get_entrypoint(shader);

   assert(shader->info.stage != MESA_SHADER_COMPUTE);

   /* assert that functions have been inlined before packing is called */
   nir_foreach_function(f, shader) {
      assert(f->impl == impl);
   }

   state.b = nir_builder_create(impl);
   state.consts = consts;
   state.prog = prog;
   state.mem_ctx = mem_ctx;
   state.shader = shader;
   state.impl = impl;
   state.locations_used = locations_used;
   state.components = components;
   state.mode = mode;
   state.gs_input_vertices = gs_input_vertices;
   state.disable_varying_packing = disable_varying_packing;
   state.disable_xfb_packing = disable_xfb_packing;
   state.xfb_enabled = xfb_enabled;
   state.packed_varyings =
      (nir_variable **) rzalloc_array_size(mem_ctx, sizeof(nir_variable *),
                                           locations_used);

   /* Determine if the shader interface is exposed to api query */
   struct gl_linked_shader *linked_shaders[MESA_SHADER_STAGES];
   unsigned num_shaders = 0;
   for (unsigned i = 0; i < MESA_SHADER_STAGES; i++) {
      if (prog->_LinkedShaders[i])
         linked_shaders[num_shaders++] = prog->_LinkedShaders[i];
   }

   if (mode == nir_var_shader_in) {
      state.ifc_exposed_to_query_api = linked_shaders[0] == linked_shader;
      lower_packed_inputs(&state);
   } else {
      state.ifc_exposed_to_query_api =
         linked_shaders[num_shaders - 1] == linked_shader;
      lower_packed_outputs(&state);
   }

   nir_lower_global_vars_to_local(shader);
   nir_fixup_deref_modes(shader);
}
