/*
 * Mesa 3-D graphics library
 *
 * Copyright (C) 1999-2008  Brian Paul   All Rights Reserved.
 * (C) Copyright IBM Corporation 2006
 * Copyright (C) 2009  VMware, Inc.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */


/**
 * \file arrayobj.c
 *
 * Implementation of Vertex Array Objects (VAOs), from OpenGL 3.1+ /
 * the GL_ARB_vertex_array_object extension.
 *
 * \todo
 * The code in this file borrows a lot from bufferobj.c.  There's a certain
 * amount of cruft left over from that origin that may be unnecessary.
 *
 * \author Ian Romanick <idr@us.ibm.com>
 * \author Brian Paul
 */


#include "util/glheader.h"
#include "hash.h"
#include "image.h"

#include "context.h"
#include "bufferobj.h"
#include "arrayobj.h"
#include "draw_validate.h"
#include "macros.h"
#include "mtypes.h"
#include "state.h"
#include "varray.h"
#include "util/bitscan.h"
#include "util/u_atomic.h"
#include "util/u_math.h"
#include "util/u_memory.h"
#include "api_exec_decl.h"

const GLubyte
_mesa_vao_attribute_map[ATTRIBUTE_MAP_MODE_MAX][VERT_ATTRIB_MAX] =
{
   /* ATTRIBUTE_MAP_MODE_IDENTITY
    *
    * Grab vertex processing attribute VERT_ATTRIB_POS from
    * the VAO attribute VERT_ATTRIB_POS, and grab vertex processing
    * attribute VERT_ATTRIB_GENERIC0 from the VAO attribute
    * VERT_ATTRIB_GENERIC0.
    */
   {
      VERT_ATTRIB_POS,                 /* VERT_ATTRIB_POS */
      VERT_ATTRIB_NORMAL,              /* VERT_ATTRIB_NORMAL */
      VERT_ATTRIB_COLOR0,              /* VERT_ATTRIB_COLOR0 */
      VERT_ATTRIB_COLOR1,              /* VERT_ATTRIB_COLOR1 */
      VERT_ATTRIB_FOG,                 /* VERT_ATTRIB_FOG */
      VERT_ATTRIB_COLOR_INDEX,         /* VERT_ATTRIB_COLOR_INDEX */
      VERT_ATTRIB_TEX0,                /* VERT_ATTRIB_TEX0 */
      VERT_ATTRIB_TEX1,                /* VERT_ATTRIB_TEX1 */
      VERT_ATTRIB_TEX2,                /* VERT_ATTRIB_TEX2 */
      VERT_ATTRIB_TEX3,                /* VERT_ATTRIB_TEX3 */
      VERT_ATTRIB_TEX4,                /* VERT_ATTRIB_TEX4 */
      VERT_ATTRIB_TEX5,                /* VERT_ATTRIB_TEX5 */
      VERT_ATTRIB_TEX6,                /* VERT_ATTRIB_TEX6 */
      VERT_ATTRIB_TEX7,                /* VERT_ATTRIB_TEX7 */
      VERT_ATTRIB_POINT_SIZE,          /* VERT_ATTRIB_POINT_SIZE */
      VERT_ATTRIB_GENERIC0,            /* VERT_ATTRIB_GENERIC0 */
      VERT_ATTRIB_GENERIC1,            /* VERT_ATTRIB_GENERIC1 */
      VERT_ATTRIB_GENERIC2,            /* VERT_ATTRIB_GENERIC2 */
      VERT_ATTRIB_GENERIC3,            /* VERT_ATTRIB_GENERIC3 */
      VERT_ATTRIB_GENERIC4,            /* VERT_ATTRIB_GENERIC4 */
      VERT_ATTRIB_GENERIC5,            /* VERT_ATTRIB_GENERIC5 */
      VERT_ATTRIB_GENERIC6,            /* VERT_ATTRIB_GENERIC6 */
      VERT_ATTRIB_GENERIC7,            /* VERT_ATTRIB_GENERIC7 */
      VERT_ATTRIB_GENERIC8,            /* VERT_ATTRIB_GENERIC8 */
      VERT_ATTRIB_GENERIC9,            /* VERT_ATTRIB_GENERIC9 */
      VERT_ATTRIB_GENERIC10,           /* VERT_ATTRIB_GENERIC10 */
      VERT_ATTRIB_GENERIC11,           /* VERT_ATTRIB_GENERIC11 */
      VERT_ATTRIB_GENERIC12,           /* VERT_ATTRIB_GENERIC12 */
      VERT_ATTRIB_GENERIC13,           /* VERT_ATTRIB_GENERIC13 */
      VERT_ATTRIB_GENERIC14,           /* VERT_ATTRIB_GENERIC14 */
      VERT_ATTRIB_GENERIC15,           /* VERT_ATTRIB_GENERIC15 */
      VERT_ATTRIB_EDGEFLAG,            /* VERT_ATTRIB_EDGEFLAG */
   },

   /* ATTRIBUTE_MAP_MODE_POSITION
    *
    * Grab vertex processing attribute VERT_ATTRIB_POS as well as
    * vertex processing attribute VERT_ATTRIB_GENERIC0 from the
    * VAO attribute VERT_ATTRIB_POS.
    */
   {
      VERT_ATTRIB_POS,                 /* VERT_ATTRIB_POS */
      VERT_ATTRIB_NORMAL,              /* VERT_ATTRIB_NORMAL */
      VERT_ATTRIB_COLOR0,              /* VERT_ATTRIB_COLOR0 */
      VERT_ATTRIB_COLOR1,              /* VERT_ATTRIB_COLOR1 */
      VERT_ATTRIB_FOG,                 /* VERT_ATTRIB_FOG */
      VERT_ATTRIB_COLOR_INDEX,         /* VERT_ATTRIB_COLOR_INDEX */
      VERT_ATTRIB_TEX0,                /* VERT_ATTRIB_TEX0 */
      VERT_ATTRIB_TEX1,                /* VERT_ATTRIB_TEX1 */
      VERT_ATTRIB_TEX2,                /* VERT_ATTRIB_TEX2 */
      VERT_ATTRIB_TEX3,                /* VERT_ATTRIB_TEX3 */
      VERT_ATTRIB_TEX4,                /* VERT_ATTRIB_TEX4 */
      VERT_ATTRIB_TEX5,                /* VERT_ATTRIB_TEX5 */
      VERT_ATTRIB_TEX6,                /* VERT_ATTRIB_TEX6 */
      VERT_ATTRIB_TEX7,                /* VERT_ATTRIB_TEX7 */
      VERT_ATTRIB_POINT_SIZE,          /* VERT_ATTRIB_POINT_SIZE */
      VERT_ATTRIB_POS,                 /* VERT_ATTRIB_GENERIC0 */
      VERT_ATTRIB_GENERIC1,            /* VERT_ATTRIB_GENERIC1 */
      VERT_ATTRIB_GENERIC2,            /* VERT_ATTRIB_GENERIC2 */
      VERT_ATTRIB_GENERIC3,            /* VERT_ATTRIB_GENERIC3 */
      VERT_ATTRIB_GENERIC4,            /* VERT_ATTRIB_GENERIC4 */
      VERT_ATTRIB_GENERIC5,            /* VERT_ATTRIB_GENERIC5 */
      VERT_ATTRIB_GENERIC6,            /* VERT_ATTRIB_GENERIC6 */
      VERT_ATTRIB_GENERIC7,            /* VERT_ATTRIB_GENERIC7 */
      VERT_ATTRIB_GENERIC8,            /* VERT_ATTRIB_GENERIC8 */
      VERT_ATTRIB_GENERIC9,            /* VERT_ATTRIB_GENERIC9 */
      VERT_ATTRIB_GENERIC10,           /* VERT_ATTRIB_GENERIC10 */
      VERT_ATTRIB_GENERIC11,           /* VERT_ATTRIB_GENERIC11 */
      VERT_ATTRIB_GENERIC12,           /* VERT_ATTRIB_GENERIC12 */
      VERT_ATTRIB_GENERIC13,           /* VERT_ATTRIB_GENERIC13 */
      VERT_ATTRIB_GENERIC14,           /* VERT_ATTRIB_GENERIC14 */
      VERT_ATTRIB_GENERIC15,           /* VERT_ATTRIB_GENERIC15 */
      VERT_ATTRIB_EDGEFLAG,            /* VERT_ATTRIB_EDGEFLAG */
   },

   /* ATTRIBUTE_MAP_MODE_GENERIC0
    *
    * Grab vertex processing attribute VERT_ATTRIB_POS as well as
    * vertex processing attribute VERT_ATTRIB_GENERIC0 from the
    * VAO attribute VERT_ATTRIB_GENERIC0.
    */
   {
      VERT_ATTRIB_GENERIC0,            /* VERT_ATTRIB_POS */
      VERT_ATTRIB_NORMAL,              /* VERT_ATTRIB_NORMAL */
      VERT_ATTRIB_COLOR0,              /* VERT_ATTRIB_COLOR0 */
      VERT_ATTRIB_COLOR1,              /* VERT_ATTRIB_COLOR1 */
      VERT_ATTRIB_FOG,                 /* VERT_ATTRIB_FOG */
      VERT_ATTRIB_COLOR_INDEX,         /* VERT_ATTRIB_COLOR_INDEX */
      VERT_ATTRIB_TEX0,                /* VERT_ATTRIB_TEX0 */
      VERT_ATTRIB_TEX1,                /* VERT_ATTRIB_TEX1 */
      VERT_ATTRIB_TEX2,                /* VERT_ATTRIB_TEX2 */
      VERT_ATTRIB_TEX3,                /* VERT_ATTRIB_TEX3 */
      VERT_ATTRIB_TEX4,                /* VERT_ATTRIB_TEX4 */
      VERT_ATTRIB_TEX5,                /* VERT_ATTRIB_TEX5 */
      VERT_ATTRIB_TEX6,                /* VERT_ATTRIB_TEX6 */
      VERT_ATTRIB_TEX7,                /* VERT_ATTRIB_TEX7 */
      VERT_ATTRIB_POINT_SIZE,          /* VERT_ATTRIB_POINT_SIZE */
      VERT_ATTRIB_GENERIC0,            /* VERT_ATTRIB_GENERIC0 */
      VERT_ATTRIB_GENERIC1,            /* VERT_ATTRIB_GENERIC1 */
      VERT_ATTRIB_GENERIC2,            /* VERT_ATTRIB_GENERIC2 */
      VERT_ATTRIB_GENERIC3,            /* VERT_ATTRIB_GENERIC3 */
      VERT_ATTRIB_GENERIC4,            /* VERT_ATTRIB_GENERIC4 */
      VERT_ATTRIB_GENERIC5,            /* VERT_ATTRIB_GENERIC5 */
      VERT_ATTRIB_GENERIC6,            /* VERT_ATTRIB_GENERIC6 */
      VERT_ATTRIB_GENERIC7,            /* VERT_ATTRIB_GENERIC7 */
      VERT_ATTRIB_GENERIC8,            /* VERT_ATTRIB_GENERIC8 */
      VERT_ATTRIB_GENERIC9,            /* VERT_ATTRIB_GENERIC9 */
      VERT_ATTRIB_GENERIC10,           /* VERT_ATTRIB_GENERIC10 */
      VERT_ATTRIB_GENERIC11,           /* VERT_ATTRIB_GENERIC11 */
      VERT_ATTRIB_GENERIC12,           /* VERT_ATTRIB_GENERIC12 */
      VERT_ATTRIB_GENERIC13,           /* VERT_ATTRIB_GENERIC13 */
      VERT_ATTRIB_GENERIC14,           /* VERT_ATTRIB_GENERIC14 */
      VERT_ATTRIB_GENERIC15,           /* VERT_ATTRIB_GENERIC15 */
      VERT_ATTRIB_EDGEFLAG,            /* VERT_ATTRIB_EDGEFLAG */
   }
};


/**
 * Look up the array object for the given ID.
 *
 * \returns
 * Either a pointer to the array object with the specified ID or \c NULL for
 * a non-existent ID.  The spec defines ID 0 as being technically
 * non-existent.
 */

struct gl_vertex_array_object *
_mesa_lookup_vao(struct gl_context *ctx, GLuint id)
{
   /* The ARB_direct_state_access specification says:
    *
    *    "<vaobj> is [compatibility profile:
    *     zero, indicating the default vertex array object, or]
    *     the name of the vertex array object."
    */
   if (id == 0) {
      if (_mesa_is_desktop_gl_compat(ctx))
         return ctx->Array.DefaultVAO;

      return NULL;
   } else {
      struct gl_vertex_array_object *vao;

      if (ctx->Array.LastLookedUpVAO &&
          ctx->Array.LastLookedUpVAO->Name == id) {
         vao = ctx->Array.LastLookedUpVAO;
      } else {
         vao = (struct gl_vertex_array_object *)
            _mesa_HashLookupLocked(ctx->Array.Objects, id);

         _mesa_reference_vao(ctx, &ctx->Array.LastLookedUpVAO, vao);
      }

      return vao;
   }
}


/**
 * Looks up the array object for the given ID.
 *
 * While _mesa_lookup_vao doesn't generate an error if the object does not
 * exist, this function comes in two variants.
 * If is_ext_dsa is false, this function generates a GL_INVALID_OPERATION
 * error if the array object does not exist. It also returns the default
 * array object when ctx is a compatibility profile context and id is zero.
 * If is_ext_dsa is true, 0 is not a valid name. If the name exists but
 * the object has never been bound, it is initialized.
 */
struct gl_vertex_array_object *
_mesa_lookup_vao_err(struct gl_context *ctx, GLuint id,
                     bool is_ext_dsa, const char *caller)
{
   /* The ARB_direct_state_access specification says:
    *
    *    "<vaobj> is [compatibility profile:
    *     zero, indicating the default vertex array object, or]
    *     the name of the vertex array object."
    */
   if (id == 0) {
      if (is_ext_dsa || _mesa_is_desktop_gl_core(ctx)) {
         _mesa_error(ctx, GL_INVALID_OPERATION,
                     "%s(zero is not valid vaobj name%s)",
                     caller,
                     is_ext_dsa ? "" : " in a core profile context");
         return NULL;
      }

      return ctx->Array.DefaultVAO;
   } else {
      struct gl_vertex_array_object *vao;

      if (ctx->Array.LastLookedUpVAO &&
          ctx->Array.LastLookedUpVAO->Name == id) {
         vao = ctx->Array.LastLookedUpVAO;
      } else {
         vao = (struct gl_vertex_array_object *)
            _mesa_HashLookupLocked(ctx->Array.Objects, id);

         /* The ARB_direct_state_access specification says:
          *
          *    "An INVALID_OPERATION error is generated if <vaobj> is not
          *     [compatibility profile: zero or] the name of an existing
          *     vertex array object."
          */
         if (!vao || (!is_ext_dsa && !vao->EverBound)) {
            _mesa_error(ctx, GL_INVALID_OPERATION,
                        "%s(non-existent vaobj=%u)", caller, id);
            return NULL;
         }

         /* The EXT_direct_state_access specification says:
         *
         *    "If the vertex array object named by the vaobj parameter has not
         *     been previously bound but has been generated (without subsequent
         *     deletion) by GenVertexArrays, the GL first creates a new state
         *     vector in the same manner as when BindVertexArray creates a new
         *     vertex array object."
         */
         if (vao && is_ext_dsa && !vao->EverBound)
            vao->EverBound = true;

         _mesa_reference_vao(ctx, &ctx->Array.LastLookedUpVAO, vao);
      }

      return vao;
   }
}


/**
 * For all the vertex binding points in the array object, unbind any pointers
 * to any buffer objects (VBOs).
 * This is done just prior to array object destruction.
 */
void
_mesa_unbind_array_object_vbos(struct gl_context *ctx,
                               struct gl_vertex_array_object *obj)
{
   GLuint i;

   for (i = 0; i < ARRAY_SIZE(obj->BufferBinding); i++)
      _mesa_reference_buffer_object(ctx, &obj->BufferBinding[i].BufferObj, NULL);
}


/**
 * Allocate and initialize a new vertex array object.
 */
struct gl_vertex_array_object *
_mesa_new_vao(struct gl_context *ctx, GLuint name)
{
   struct gl_vertex_array_object *obj = MALLOC_STRUCT(gl_vertex_array_object);
   if (obj)
      _mesa_initialize_vao(ctx, obj, name);
   return obj;
}


/**
 * Delete an array object.
 */
void
_mesa_delete_vao(struct gl_context *ctx, struct gl_vertex_array_object *obj)
{
   _mesa_unbind_array_object_vbos(ctx, obj);
   _mesa_reference_buffer_object(ctx, &obj->IndexBufferObj, NULL);
   free(obj->Label);
   free(obj);
}


/**
 * Set ptr to vao w/ reference counting.
 * Note: this should only be called from the _mesa_reference_vao()
 * inline function.
 */
void
_mesa_reference_vao_(struct gl_context *ctx,
                     struct gl_vertex_array_object **ptr,
                     struct gl_vertex_array_object *vao)
{
   assert(*ptr != vao);

   if (*ptr) {
      /* Unreference the old array object */
      struct gl_vertex_array_object *oldObj = *ptr;

      bool deleteFlag;
      if (oldObj->SharedAndImmutable) {
         deleteFlag = p_atomic_dec_zero(&oldObj->RefCount);
      } else {
         assert(oldObj->RefCount > 0);
         oldObj->RefCount--;
         deleteFlag = (oldObj->RefCount == 0);
      }

      if (deleteFlag)
         _mesa_delete_vao(ctx, oldObj);

      *ptr = NULL;
   }
   assert(!*ptr);

   if (vao) {
      /* reference new array object */
      if (vao->SharedAndImmutable) {
         p_atomic_inc(&vao->RefCount);
      } else {
         assert(vao->RefCount > 0);
         vao->RefCount++;
      }

      *ptr = vao;
   }
}


/**
 * Initialize a gl_vertex_array_object's arrays.
 */
void
_mesa_initialize_vao(struct gl_context *ctx,
                     struct gl_vertex_array_object *vao,
                     GLuint name)
{
   memcpy(vao, &ctx->Array.DefaultVAOState, sizeof(*vao));
   vao->Name = name;
}


/**
 * Compute the offset range for the provided binding.
 *
 * This is a helper function for the below.
 */
static void
compute_vbo_offset_range(const struct gl_vertex_array_object *vao,
                         const struct gl_vertex_buffer_binding *binding,
                         GLsizeiptr* min, GLsizeiptr* max)
{
   /* The function is meant to work on VBO bindings */
   assert(binding->BufferObj);

   /* Start with an inverted range of relative offsets. */
   GLuint min_offset = ~(GLuint)0;
   GLuint max_offset = 0;

   /* We work on the unmapped originaly VAO array entries. */
   GLbitfield mask = vao->Enabled & binding->_BoundArrays;
   /* The binding should be active somehow, not to return inverted ranges */
   assert(mask);
   while (mask) {
      const int i = u_bit_scan(&mask);
      const GLuint off = vao->VertexAttrib[i].RelativeOffset;
      min_offset = MIN2(off, min_offset);
      max_offset = MAX2(off, max_offset);
   }

   *min = binding->Offset + (GLsizeiptr)min_offset;
   *max = binding->Offset + (GLsizeiptr)max_offset;
}


/**
 * Update the unique binding and pos/generic0 map tracking in the vao.
 *
 * The idea is to build up information in the vao so that a consuming
 * backend can execute the following to set up buffer and vertex element
 * information:
 *
 * const GLbitfield inputs_read = VERT_BIT_ALL; // backend vp inputs
 *
 * // Attribute data is in a VBO.
 * GLbitfield vbomask = inputs_read & _mesa_draw_vbo_array_bits(ctx);
 * while (vbomask) {
 *    // The attribute index to start pulling a binding
 *    const gl_vert_attrib i = ffs(vbomask) - 1;
 *    const struct gl_vertex_buffer_binding *const binding
 *       = _mesa_draw_buffer_binding(vao, i);
 *
 *    <insert code to handle the vertex buffer object at binding>
 *
 *    const GLbitfield boundmask = _mesa_draw_bound_attrib_bits(binding);
 *    GLbitfield attrmask = vbomask & boundmask;
 *    assert(attrmask);
 *    // Walk attributes belonging to the binding
 *    while (attrmask) {
 *       const gl_vert_attrib attr = u_bit_scan(&attrmask);
 *       const struct gl_array_attributes *const attrib
 *          = _mesa_draw_array_attrib(vao, attr);
 *
 *       <insert code to handle the vertex element refering to the binding>
 *    }
 *    vbomask &= ~boundmask;
 * }
 *
 * // Process user space buffers
 * GLbitfield usermask = inputs_read & _mesa_draw_user_array_bits(ctx);
 * while (usermask) {
 *    // The attribute index to start pulling a binding
 *    const gl_vert_attrib i = ffs(usermask) - 1;
 *    const struct gl_vertex_buffer_binding *const binding
 *       = _mesa_draw_buffer_binding(vao, i);
 *
 *    <insert code to handle a set of interleaved user space arrays at binding>
 *
 *    const GLbitfield boundmask = _mesa_draw_bound_attrib_bits(binding);
 *    GLbitfield attrmask = usermask & boundmask;
 *    assert(attrmask);
 *    // Walk interleaved attributes with a common stride and instance divisor
 *    while (attrmask) {
 *       const gl_vert_attrib attr = u_bit_scan(&attrmask);
 *       const struct gl_array_attributes *const attrib
 *          = _mesa_draw_array_attrib(vao, attr);
 *
 *       <insert code to handle non vbo vertex arrays>
 *    }
 *    usermask &= ~boundmask;
 * }
 *
 * // Process values that should have better been uniforms in the application
 * GLbitfield curmask = inputs_read & _mesa_draw_current_bits(ctx);
 * while (curmask) {
 *    const gl_vert_attrib attr = u_bit_scan(&curmask);
 *    const struct gl_array_attributes *const attrib
 *       = _mesa_draw_current_attrib(ctx, attr);
 *
 *    <insert code to handle current values>
 * }
 *
 *
 * Note that the scan below must not incoporate any context state.
 * The rationale is that once a VAO is finalized it should not
 * be touched anymore. That means, do not incorporate the
 * gl_context::Array._DrawVAOEnabledAttribs bitmask into this scan.
 * A backend driver may further reduce the handled vertex processing
 * inputs based on their vertex shader inputs. But scanning for
 * collapsable binding points to reduce relocs is done based on the
 * enabled arrays.
 * Also VAOs may be shared between contexts due to their use in dlists
 * thus no context state should bleed into the VAO.
 */
void
_mesa_update_vao_derived_arrays(struct gl_context *ctx,
                                struct gl_vertex_array_object *vao)
{
   assert(!vao->IsDynamic);
   /* Make sure we do not run into problems with shared objects */
   assert(!vao->SharedAndImmutable);

   /* Limit used for common binding scanning below. */
   const GLsizeiptr MaxRelativeOffset =
      ctx->Const.MaxVertexAttribRelativeOffset;

   /* The gl_vertex_array_object::_AttributeMapMode denotes the way
    * VERT_ATTRIB_{POS,GENERIC0} mapping is done.
    *
    * This mapping is used to map between the OpenGL api visible
    * VERT_ATTRIB_* arrays to mesa driver arrayinputs or shader inputs.
    * The mapping only depends on the enabled bits of the
    * VERT_ATTRIB_{POS,GENERIC0} arrays and is tracked in the VAO.
    *
    * This map needs to be applied when finally translating to the bitmasks
    * as consumed by the driver backends. The duplicate scanning is here
    * can as well be done in the OpenGL API numbering without this map.
    */
   const gl_attribute_map_mode mode = vao->_AttributeMapMode;
   /* Enabled array bits. */
   const GLbitfield enabled = vao->Enabled;
   /* VBO array bits. */
   const GLbitfield vbos = vao->VertexAttribBufferMask;

   /* More than 4 updates turn the VAO to dynamic. */
   if (ctx->Const.AllowDynamicVAOFastPath && ++vao->NumUpdates > 4) {
      vao->IsDynamic = true;
      /* IsDynamic changes how vertex elements map to vertex buffers. */
      ctx->Array.NewVertexElements = true;
      return;
   }

   /* Walk those enabled arrays that have a real vbo attached */
   GLbitfield mask = enabled;
   while (mask) {
      /* Do not use u_bit_scan as we can walk multiple attrib arrays at once */
      const int i = ffs(mask) - 1;
      /* The binding from the first to be processed attribute. */
      const GLuint bindex = vao->VertexAttrib[i].BufferBindingIndex;
      struct gl_vertex_buffer_binding *binding = &vao->BufferBinding[bindex];

      /* The scan goes different for user space arrays than vbos */
      if (binding->BufferObj) {
         /* The bound arrays. */
         const GLbitfield bound = enabled & binding->_BoundArrays;

         /* Start this current effective binding with the actual bound arrays */
         GLbitfield eff_bound_arrays = bound;

         /*
          * If there is nothing left to scan just update the effective binding
          * information. If the VAO is already only using a single binding point
          * we end up here. So the overhead of this scan for an application
          * carefully preparing the VAO for draw is low.
          */

         GLbitfield scanmask = mask & vbos & ~bound;
         /* Is there something left to scan? */
         if (scanmask == 0) {
            /* Just update the back reference from the attrib to the binding and
             * the effective offset.
             */
            GLbitfield attrmask = eff_bound_arrays;
            while (attrmask) {
               const int j = u_bit_scan(&attrmask);
               struct gl_array_attributes *attrib2 = &vao->VertexAttrib[j];

               /* Update the index into the common binding point and offset */
               attrib2->_EffBufferBindingIndex = bindex;
               attrib2->_EffRelativeOffset = attrib2->RelativeOffset;
               assert(attrib2->_EffRelativeOffset <= MaxRelativeOffset);
            }
            /* Finally this is the set of effectively bound arrays with the
             * original binding offset.
             */
            binding->_EffOffset = binding->Offset;
            /* The bound arrays past the VERT_ATTRIB_{POS,GENERIC0} mapping. */
            binding->_EffBoundArrays =
               _mesa_vao_enable_to_vp_inputs(mode, eff_bound_arrays);

         } else {
            /* In the VBO case, scan for attribute/binding
             * combinations with relative bindings in the range of
             * [0, ctx->Const.MaxVertexAttribRelativeOffset].
             * Note that this does also go beyond just interleaved arrays
             * as long as they use the same VBO, binding parameters and the
             * offsets stay within bounds that the backend still can handle.
             */

            GLsizeiptr min_offset, max_offset;
            compute_vbo_offset_range(vao, binding, &min_offset, &max_offset);
            assert(max_offset <= min_offset + MaxRelativeOffset);

            /* Now scan. */
            while (scanmask) {
               /* Do not use u_bit_scan as we can walk multiple
                * attrib arrays at once
                */
               const int j = ffs(scanmask) - 1;
               const struct gl_array_attributes *attrib2 =
                  &vao->VertexAttrib[j];
               const struct gl_vertex_buffer_binding *binding2 =
                  &vao->BufferBinding[attrib2->BufferBindingIndex];

               /* Remove those attrib bits from the mask that are bound to the
                * same effective binding point.
                */
               const GLbitfield bound2 = enabled & binding2->_BoundArrays;
               scanmask &= ~bound2;

               /* Check if we have an identical binding */
               if (binding->Stride != binding2->Stride)
                  continue;
               if (binding->InstanceDivisor != binding2->InstanceDivisor)
                  continue;
               if (binding->BufferObj != binding2->BufferObj)
                  continue;
               /* Check if we can fold both bindings into a common binding */
               GLsizeiptr min_offset2, max_offset2;
               compute_vbo_offset_range(vao, binding2,
                                        &min_offset2, &max_offset2);
               /* If the relative offset is within the limits ... */
               if (min_offset + MaxRelativeOffset < max_offset2)
                  continue;
               if (min_offset2 + MaxRelativeOffset < max_offset)
                  continue;
               /* ... add this array to the effective binding */
               eff_bound_arrays |= bound2;
               min_offset = MIN2(min_offset, min_offset2);
               max_offset = MAX2(max_offset, max_offset2);
               assert(max_offset <= min_offset + MaxRelativeOffset);
            }

            /* Update the back reference from the attrib to the binding */
            GLbitfield attrmask = eff_bound_arrays;
            while (attrmask) {
               const int j = u_bit_scan(&attrmask);
               struct gl_array_attributes *attrib2 = &vao->VertexAttrib[j];
               const struct gl_vertex_buffer_binding *binding2 =
                  &vao->BufferBinding[attrib2->BufferBindingIndex];

               /* Update the index into the common binding point and offset */
               attrib2->_EffBufferBindingIndex = bindex;
               attrib2->_EffRelativeOffset =
                  binding2->Offset + attrib2->RelativeOffset - min_offset;
               assert(attrib2->_EffRelativeOffset <= MaxRelativeOffset);
            }
            /* Finally this is the set of effectively bound arrays */
            binding->_EffOffset = min_offset;
            /* The bound arrays past the VERT_ATTRIB_{POS,GENERIC0} mapping. */
            binding->_EffBoundArrays =
               _mesa_vao_enable_to_vp_inputs(mode, eff_bound_arrays);
         }

         /* Mark all the effective bound arrays as processed. */
         mask &= ~eff_bound_arrays;

      } else {
         /* Scanning of common bindings for user space arrays.
          */

         const struct gl_array_attributes *attrib = &vao->VertexAttrib[i];
         const GLbitfield bound = VERT_BIT(i);

         /* Note that user space array pointers can only happen using a one
          * to one binding point to array mapping.
          * The OpenGL 4.x/ARB_vertex_attrib_binding api does not support
          * user space arrays collected at multiple binding points.
          * The only provider of user space interleaved arrays with a single
          * binding point is the mesa internal vbo module. But that one
          * provides a perfect interleaved set of arrays.
          *
          * If this would not be true we would potentially get attribute arrays
          * with user space pointers that may not lie within the
          * MaxRelativeOffset range but still attached to a single binding.
          * Then we would need to store the effective attribute and binding
          * grouping information in a seperate array beside
          * gl_array_attributes/gl_vertex_buffer_binding.
          */
         assert(util_bitcount(binding->_BoundArrays & vao->Enabled) == 1
                || (vao->Enabled & ~binding->_BoundArrays) == 0);

         /* Start this current effective binding with the array */
         GLbitfield eff_bound_arrays = bound;

         const GLubyte *ptr = attrib->Ptr;
         unsigned vertex_end = attrib->Format._ElementSize;

         /* Walk other user space arrays and see which are interleaved
          * using the same binding parameters.
          */
         GLbitfield scanmask = mask & ~vbos & ~bound;
         while (scanmask) {
            const int j = u_bit_scan(&scanmask);
            const struct gl_array_attributes *attrib2 = &vao->VertexAttrib[j];
            const struct gl_vertex_buffer_binding *binding2 =
               &vao->BufferBinding[attrib2->BufferBindingIndex];

            /* See the comment at the same assert above. */
            assert(util_bitcount(binding2->_BoundArrays & vao->Enabled) == 1
                   || (vao->Enabled & ~binding->_BoundArrays) == 0);

            /* Check if we have an identical binding */
            if (binding->Stride != binding2->Stride)
               continue;
            if (binding->InstanceDivisor != binding2->InstanceDivisor)
               continue;
            if (ptr <= attrib2->Ptr) {
               if (ptr + binding->Stride < attrib2->Ptr +
                   attrib2->Format._ElementSize)
                  continue;
               unsigned end = attrib2->Ptr + attrib2->Format._ElementSize - ptr;
               vertex_end = MAX2(vertex_end, end);
            } else {
               if (attrib2->Ptr + binding->Stride < ptr + vertex_end)
                  continue;
               vertex_end += (GLsizei)(ptr - attrib2->Ptr);
               ptr = attrib2->Ptr;
            }

            /* User space buffer object */
            assert(!binding2->BufferObj);

            eff_bound_arrays |= VERT_BIT(j);
         }

         /* Update the back reference from the attrib to the binding */
         GLbitfield attrmask = eff_bound_arrays;
         while (attrmask) {
            const int j = u_bit_scan(&attrmask);
            struct gl_array_attributes *attrib2 = &vao->VertexAttrib[j];

            /* Update the index into the common binding point and the offset */
            attrib2->_EffBufferBindingIndex = bindex;
            attrib2->_EffRelativeOffset = attrib2->Ptr - ptr;
            assert(attrib2->_EffRelativeOffset <= binding->Stride);
         }
         /* Finally this is the set of effectively bound arrays */
         binding->_EffOffset = (GLintptr)ptr;
         /* The bound arrays past the VERT_ATTRIB_{POS,GENERIC0} mapping. */
         binding->_EffBoundArrays =
            _mesa_vao_enable_to_vp_inputs(mode, eff_bound_arrays);

         /* Mark all the effective bound arrays as processed. */
         mask &= ~eff_bound_arrays;
      }
   }

#ifndef NDEBUG
   /* Make sure the above code works as expected. */
   for (gl_vert_attrib attr = 0; attr < VERT_ATTRIB_MAX; ++attr) {
      /* Query the original api defined attrib/binding information ... */
      const unsigned char *const map =_mesa_vao_attribute_map[mode];
      if (vao->Enabled & VERT_BIT(map[attr])) {
         const struct gl_array_attributes *attrib =
            &vao->VertexAttrib[map[attr]];
         const struct gl_vertex_buffer_binding *binding =
            &vao->BufferBinding[attrib->BufferBindingIndex];
         /* ... and compare that with the computed attrib/binding */
         const struct gl_vertex_buffer_binding *binding2 =
            &vao->BufferBinding[attrib->_EffBufferBindingIndex];
         assert(binding->Stride == binding2->Stride);
         assert(binding->InstanceDivisor == binding2->InstanceDivisor);
         assert(binding->BufferObj == binding2->BufferObj);
         if (binding->BufferObj) {
            assert(attrib->_EffRelativeOffset <= MaxRelativeOffset);
            assert(binding->Offset + attrib->RelativeOffset ==
                   binding2->_EffOffset + attrib->_EffRelativeOffset);
         } else {
            assert(attrib->_EffRelativeOffset < binding->Stride);
            assert((GLintptr)attrib->Ptr ==
                   binding2->_EffOffset + attrib->_EffRelativeOffset);
         }
      }
   }
#endif
}


void
_mesa_set_vao_immutable(struct gl_context *ctx,
                        struct gl_vertex_array_object *vao)
{
   _mesa_update_vao_derived_arrays(ctx, vao);
   vao->SharedAndImmutable = true;
}


/**
 * Map buffer objects used in attribute arrays.
 */
void
_mesa_vao_map_arrays(struct gl_context *ctx, struct gl_vertex_array_object *vao,
                     GLbitfield access)
{
   GLbitfield mask = vao->Enabled & vao->VertexAttribBufferMask;
   while (mask) {
      /* Do not use u_bit_scan as we can walk multiple attrib arrays at once */
      const gl_vert_attrib attr = ffs(mask) - 1;
      const GLubyte bindex = vao->VertexAttrib[attr].BufferBindingIndex;
      struct gl_vertex_buffer_binding *binding = &vao->BufferBinding[bindex];
      mask &= ~binding->_BoundArrays;

      struct gl_buffer_object *bo = binding->BufferObj;
      assert(bo);
      if (_mesa_bufferobj_mapped(bo, MAP_INTERNAL))
         continue;

      _mesa_bufferobj_map_range(ctx, 0, bo->Size, access, bo, MAP_INTERNAL);
   }
}


/**
 * Map buffer objects used in the vao, attribute arrays and index buffer.
 */
void
_mesa_vao_map(struct gl_context *ctx, struct gl_vertex_array_object *vao,
              GLbitfield access)
{
   struct gl_buffer_object *bo = vao->IndexBufferObj;

   /* map the index buffer, if there is one, and not already mapped */
   if (bo && !_mesa_bufferobj_mapped(bo, MAP_INTERNAL))
      _mesa_bufferobj_map_range(ctx, 0, bo->Size, access, bo, MAP_INTERNAL);

   _mesa_vao_map_arrays(ctx, vao, access);
}


/**
 * Unmap buffer objects used in attribute arrays.
 */
void
_mesa_vao_unmap_arrays(struct gl_context *ctx,
                       struct gl_vertex_array_object *vao)
{
   GLbitfield mask = vao->Enabled & vao->VertexAttribBufferMask;
   while (mask) {
      /* Do not use u_bit_scan as we can walk multiple attrib arrays at once */
      const gl_vert_attrib attr = ffs(mask) - 1;
      const GLubyte bindex = vao->VertexAttrib[attr].BufferBindingIndex;
      struct gl_vertex_buffer_binding *binding = &vao->BufferBinding[bindex];
      mask &= ~binding->_BoundArrays;

      struct gl_buffer_object *bo = binding->BufferObj;
      assert(bo);
      if (!_mesa_bufferobj_mapped(bo, MAP_INTERNAL))
         continue;

      _mesa_bufferobj_unmap(ctx, bo, MAP_INTERNAL);
   }
}


/**
 * Unmap buffer objects used in the vao, attribute arrays and index buffer.
 */
void
_mesa_vao_unmap(struct gl_context *ctx, struct gl_vertex_array_object *vao)
{
   struct gl_buffer_object *bo = vao->IndexBufferObj;

   /* unmap the index buffer, if there is one, and still mapped */
   if (bo && _mesa_bufferobj_mapped(bo, MAP_INTERNAL))
      _mesa_bufferobj_unmap(ctx, bo, MAP_INTERNAL);

   _mesa_vao_unmap_arrays(ctx, vao);
}


/**********************************************************************/
/* API Functions                                                      */
/**********************************************************************/


/**
 * ARB version of glBindVertexArray()
 */
static ALWAYS_INLINE void
bind_vertex_array(struct gl_context *ctx, GLuint id, bool no_error)
{
   struct gl_vertex_array_object *const oldObj = ctx->Array.VAO;
   struct gl_vertex_array_object *newObj = NULL;

   assert(oldObj != NULL);

   if (oldObj->Name == id)
      return;   /* rebinding the same array object- no change */

   /*
    * Get pointer to new array object (newObj)
    */
   if (id == 0) {
      /* The spec says there is no array object named 0, but we use
       * one internally because it simplifies things.
       */
      newObj = ctx->Array.DefaultVAO;
   }
   else {
      /* non-default array object */
      newObj = _mesa_lookup_vao(ctx, id);
      if (!no_error && !newObj) {
         _mesa_error(ctx, GL_INVALID_OPERATION,
                     "glBindVertexArray(non-gen name)");
         return;
      }

      newObj->EverBound = GL_TRUE;
   }

   _mesa_reference_vao(ctx, &ctx->Array.VAO, newObj);
   _mesa_set_draw_vao(ctx, newObj);

   /* Update the valid-to-render state if binding on unbinding default VAO
    * if drawing with the default VAO is invalid.
    */
   if (_mesa_is_desktop_gl_core(ctx) &&
       (oldObj == ctx->Array.DefaultVAO) != (newObj == ctx->Array.DefaultVAO))
      _mesa_update_valid_to_render_state(ctx);
}


void GLAPIENTRY
_mesa_BindVertexArray_no_error(GLuint id)
{
   GET_CURRENT_CONTEXT(ctx);
   bind_vertex_array(ctx, id, true);
}


void GLAPIENTRY
_mesa_BindVertexArray(GLuint id)
{
   GET_CURRENT_CONTEXT(ctx);
   bind_vertex_array(ctx, id, false);
}


/**
 * Delete a set of array objects.
 *
 * \param n      Number of array objects to delete.
 * \param ids    Array of \c n array object IDs.
 */
static void
delete_vertex_arrays(struct gl_context *ctx, GLsizei n, const GLuint *ids)
{
   GLsizei i;

   for (i = 0; i < n; i++) {
      /* IDs equal to 0 should be silently ignored. */
      if (!ids[i])
         continue;

      struct gl_vertex_array_object *obj = _mesa_lookup_vao(ctx, ids[i]);

      if (obj) {
         assert(obj->Name == ids[i]);

         /* If the array object is currently bound, the spec says "the binding
          * for that object reverts to zero and the default vertex array
          * becomes current."
          */
         if (obj == ctx->Array.VAO)
            _mesa_BindVertexArray_no_error(0);

         /* The ID is immediately freed for re-use */
         _mesa_HashRemoveLocked(ctx->Array.Objects, obj->Name);

         if (ctx->Array.LastLookedUpVAO == obj)
            _mesa_reference_vao(ctx, &ctx->Array.LastLookedUpVAO, NULL);

         /* Unreference the array object.
          * If refcount hits zero, the object will be deleted.
          */
         _mesa_reference_vao(ctx, &obj, NULL);
      }
   }
}


void GLAPIENTRY
_mesa_DeleteVertexArrays_no_error(GLsizei n, const GLuint *ids)
{
   GET_CURRENT_CONTEXT(ctx);
   delete_vertex_arrays(ctx, n, ids);
}


void GLAPIENTRY
_mesa_DeleteVertexArrays(GLsizei n, const GLuint *ids)
{
   GET_CURRENT_CONTEXT(ctx);

   if (n < 0) {
      _mesa_error(ctx, GL_INVALID_VALUE, "glDeleteVertexArray(n)");
      return;
   }

   delete_vertex_arrays(ctx, n, ids);
}


/**
 * Generate a set of unique array object IDs and store them in \c arrays.
 * Helper for _mesa_GenVertexArrays() and _mesa_CreateVertexArrays()
 * below.
 *
 * \param n       Number of IDs to generate.
 * \param arrays  Array of \c n locations to store the IDs.
 * \param create  Indicates that the objects should also be created.
 * \param func    The name of the GL entry point.
 */
static void
gen_vertex_arrays(struct gl_context *ctx, GLsizei n, GLuint *arrays,
                  bool create, const char *func)
{
   GLint i;

   if (!arrays)
      return;

   _mesa_HashFindFreeKeys(ctx->Array.Objects, arrays, n);

   /* For the sake of simplicity we create the array objects in both
    * the Gen* and Create* cases.  The only difference is the value of
    * EverBound, which is set to true in the Create* case.
    */
   for (i = 0; i < n; i++) {
      struct gl_vertex_array_object *obj;

      obj = _mesa_new_vao(ctx, arrays[i]);
      if (!obj) {
         _mesa_error(ctx, GL_OUT_OF_MEMORY, "%s", func);
         return;
      }
      obj->EverBound = create;
      _mesa_HashInsertLocked(ctx->Array.Objects, obj->Name, obj, true);
   }
}


static void
gen_vertex_arrays_err(struct gl_context *ctx, GLsizei n, GLuint *arrays,
                      bool create, const char *func)
{
   if (n < 0) {
      _mesa_error(ctx, GL_INVALID_VALUE, "%s(n < 0)", func);
      return;
   }

   gen_vertex_arrays(ctx, n, arrays, create, func);
}


/**
 * ARB version of glGenVertexArrays()
 * All arrays will be required to live in VBOs.
 */
void GLAPIENTRY
_mesa_GenVertexArrays_no_error(GLsizei n, GLuint *arrays)
{
   GET_CURRENT_CONTEXT(ctx);
   gen_vertex_arrays(ctx, n, arrays, false, "glGenVertexArrays");
}


void GLAPIENTRY
_mesa_GenVertexArrays(GLsizei n, GLuint *arrays)
{
   GET_CURRENT_CONTEXT(ctx);
   gen_vertex_arrays_err(ctx, n, arrays, false, "glGenVertexArrays");
}


/**
 * ARB_direct_state_access
 * Generates ID's and creates the array objects.
 */
void GLAPIENTRY
_mesa_CreateVertexArrays_no_error(GLsizei n, GLuint *arrays)
{
   GET_CURRENT_CONTEXT(ctx);
   gen_vertex_arrays(ctx, n, arrays, true, "glCreateVertexArrays");
}


void GLAPIENTRY
_mesa_CreateVertexArrays(GLsizei n, GLuint *arrays)
{
   GET_CURRENT_CONTEXT(ctx);
   gen_vertex_arrays_err(ctx, n, arrays, true, "glCreateVertexArrays");
}


/**
 * Determine if ID is the name of an array object.
 *
 * \param id  ID of the potential array object.
 * \return  \c GL_TRUE if \c id is the name of a array object,
 *          \c GL_FALSE otherwise.
 */
GLboolean GLAPIENTRY
_mesa_IsVertexArray( GLuint id )
{
   struct gl_vertex_array_object * obj;
   GET_CURRENT_CONTEXT(ctx);
   ASSERT_OUTSIDE_BEGIN_END_WITH_RETVAL(ctx, GL_FALSE);

   obj = _mesa_lookup_vao(ctx, id);

   return obj != NULL && obj->EverBound;
}


/**
 * Sets the element array buffer binding of a vertex array object.
 *
 * This is the ARB_direct_state_access equivalent of
 * glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer).
 */
static ALWAYS_INLINE void
vertex_array_element_buffer(struct gl_context *ctx, GLuint vaobj, GLuint buffer,
                            bool no_error)
{
   struct gl_vertex_array_object *vao;
   struct gl_buffer_object *bufObj;

   ASSERT_OUTSIDE_BEGIN_END(ctx);

   if (!no_error) {
      /* The GL_ARB_direct_state_access specification says:
       *
       *    "An INVALID_OPERATION error is generated by
       *     VertexArrayElementBuffer if <vaobj> is not [compatibility profile:
       *     zero or] the name of an existing vertex array object."
       */
      vao =_mesa_lookup_vao_err(ctx, vaobj, false, "glVertexArrayElementBuffer");
      if (!vao)
         return;
   } else {
      vao = _mesa_lookup_vao(ctx, vaobj);
   }

   if (buffer != 0) {
      if (!no_error) {
         /* The GL_ARB_direct_state_access specification says:
          *
          *    "An INVALID_OPERATION error is generated if <buffer> is not zero
          *     or the name of an existing buffer object."
          */
         bufObj = _mesa_lookup_bufferobj_err(ctx, buffer,
                                             "glVertexArrayElementBuffer");
      } else {
         bufObj = _mesa_lookup_bufferobj(ctx, buffer);
      }

      if (!bufObj)
         return;
   } else {
      bufObj = NULL;
   }

   _mesa_reference_buffer_object(ctx, &vao->IndexBufferObj, bufObj);
}


void GLAPIENTRY
_mesa_VertexArrayElementBuffer_no_error(GLuint vaobj, GLuint buffer)
{
   GET_CURRENT_CONTEXT(ctx);
   vertex_array_element_buffer(ctx, vaobj, buffer, true);
}


void GLAPIENTRY
_mesa_VertexArrayElementBuffer(GLuint vaobj, GLuint buffer)
{
   GET_CURRENT_CONTEXT(ctx);
   vertex_array_element_buffer(ctx, vaobj, buffer, false);
}


void GLAPIENTRY
_mesa_GetVertexArrayiv(GLuint vaobj, GLenum pname, GLint *param)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_vertex_array_object *vao;

   ASSERT_OUTSIDE_BEGIN_END(ctx);

   /* The GL_ARB_direct_state_access specification says:
    *
    *   "An INVALID_OPERATION error is generated if <vaobj> is not
    *    [compatibility profile: zero or] the name of an existing
    *    vertex array object."
    */
   vao = _mesa_lookup_vao_err(ctx, vaobj, false, "glGetVertexArrayiv");
   if (!vao)
      return;

   /* The GL_ARB_direct_state_access specification says:
    *
    *   "An INVALID_ENUM error is generated if <pname> is not
    *    ELEMENT_ARRAY_BUFFER_BINDING."
    */
   if (pname != GL_ELEMENT_ARRAY_BUFFER_BINDING) {
      _mesa_error(ctx, GL_INVALID_ENUM,
                  "glGetVertexArrayiv(pname != "
                  "GL_ELEMENT_ARRAY_BUFFER_BINDING)");
      return;
   }

   param[0] = vao->IndexBufferObj ? vao->IndexBufferObj->Name : 0;
}
