/*
 * (C) Copyright IBM Corporation 2004, 2005
 * All Rights Reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sub license,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.  IN NO EVENT SHALL
 * IBM,
 * AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef _INDIRECT_VA_PRIVATE_
#define _INDIRECT_VA_PRIVATE_

/**
 * \file indirect_va_private.h
 *
 * \author Ian Romanick <idr@us.ibm.com>
 */

#include <inttypes.h>

#include "glxclient.h"
#include "indirect.h"
#include <GL/glxproto.h>


/**
 * State descriptor for a single array of vertex data.
 */
struct array_state
{
    /**
     * Pointer to the application supplied data.
     */
   const void *data;

    /**
     * Enum representing the type of the application supplied data.
     */
   GLenum data_type;

    /**
     * Stride value supplied by the application.  This value is not used
     * internally.  It is only kept so that it can be queried by the
     * application using glGet*v.
     */
   GLsizei user_stride;

    /**
     * Calculated size, in bytes, of a single element in the array.  This
     * is calculated based on \c count and the size of the data type
     * represented by \c data_type.
     */
   GLsizei element_size;

    /**
     * Actual byte-stride from one element to the next.  This value will
     * be equal to either \c user_stride or \c element_stride.
     */
   GLsizei true_stride;

    /**
     * Number of data values in each element.
     */
   GLint count;

    /**
     * "Normalized" data is on the range [0,1] (unsigned) or [-1,1] (signed).
     * This is used for mapping integral types to floating point types.
     */
   GLboolean normalized;

    /**
     * Pre-calculated GLX protocol command header.
     * This contains two 16-bit words: the command length and the command
     * opcode.
     */
   uint16_t header[2];

    /**
     * Set to \c GL_TRUE if this array is enabled.  Otherwise, it is set
     * to \c GL_FALSE.
     */
   GLboolean enabled;

    /**
     * For multi-arrayed data (e.g., texture coordinates, generic vertex
     * program attributes, etc.), this specifies which array this is.
     */
   unsigned index;

    /**
     * Per-array-type key.  For most arrays, this will be the GL enum for
     * that array (e.g., GL_VERTEX_ARRAY for vertex data, GL_NORMAL_ARRAY
     * for normal data, GL_TEXTURE_COORD_ARRAY for texture coordinate data,
     * etc.).
     */
   GLenum key;

    /**
     * If this array can be used with the "classic" \c glDrawArrays protocol,
     * this is set to \c GL_TRUE.  Otherwise, it is set to \c GL_FALSE.
     */
   GLboolean old_DrawArrays_possible;
};


/**
 * Array state that is pushed / popped by \c glPushClientAttrib and
 * \c glPopClientAttrib.
 */
struct array_stack_state
{
    /**
     * Pointer to the application supplied data.
     */
   const void *data;

    /**
     * Enum representing the type of the application supplied data.
     */
   GLenum data_type;

    /**
     * Stride value supplied by the application.  This value is not used
     * internally.  It is only kept so that it can be queried by the
     * application using glGet*v.
     */
   GLsizei user_stride;

    /**
     * Number of data values in each element.
     */
   GLint count;

    /**
     * Per-array-type key.  For most arrays, this will be the GL enum for
     * that array (e.g., GL_VERTEX_ARRAY for vertex data, GL_NORMAL_ARRAY
     * for normal data, GL_TEXTURE_COORD_ARRAY for texture coordinate data,
     * etc.).
     */
   GLenum key;

    /**
     * For multi-arrayed data (e.g., texture coordinates, generic vertex
     * program attributes, etc.), this specifies which array this is.
     */
   unsigned index;

    /**
     * Set to \c GL_TRUE if this array is enabled.  Otherwise, it is set
     * to \c GL_FALSE.
     */
   GLboolean enabled;
};


/**
 * Collection of all the vertex array state.
 */
struct array_state_vector
{
    /**
     * Number of arrays tracked by \c ::arrays.
     */
   size_t num_arrays;

    /**
     * Array of vertex array state.  This array contains all of the valid
     * vertex arrays.  If a vertex array isn't in this array, then it isn't
     * valid.  For example, if an implementation does not support
     * EXT_fog_coord, there won't be a GL_FOG_COORD_ARRAY entry in this
     * array.
     */
   struct array_state *arrays;

    /**
     * Number of currently enabled client-side arrays.  The value of this 
     * field is only valid if \c array_info_cache_valid is true.
     */
   size_t enabled_client_array_count;

    /**
     * \name ARRAY_INFO cache.
     * 
     * These fields track the state of the ARRAY_INFO cache.  The
     * \c array_info_cache_size is the size of the actual data stored in
     * \c array_info_cache.  \c array_info_cache_buffer_size is the size of
     * the buffer.  This will always be greater than or equal to
     * \c array_info_cache_size.
     *
     * \note
     * There are some bytes of extra data before \c array_info_cache that is
     * used to hold the header for RenderLarge commands.  This is
     * \b not included in \c array_info_cache_size or
     * \c array_info_cache_buffer_size.  \c array_info_cache_base stores a
     * pointer to the true start of the buffer (i.e., what malloc returned).
     */
   /*@{ */
   size_t array_info_cache_size;
   size_t array_info_cache_buffer_size;
   void *array_info_cache;
   void *array_info_cache_base;
   /*@} */


    /**
     * Is the cache of ARRAY_INFO data valid?  The cache can become invalid
     * when one of several state changes occur.  Among these changes are
     * modifying the array settings for an enabled array and enabling /
     * disabling an array.
     */
   GLboolean array_info_cache_valid;

    /**
     * Is it possible to use the GL 1.1 / EXT_vertex_arrays protocol?  Use
     * of this protocol is disabled with really old servers (i.e., servers
     * that don't support GL 1.1 or EXT_vertex_arrays) or when an environment
     * variable is set.
     * 
     * \todo
     * GL 1.1 and EXT_vertex_arrays use identical protocol, but have different
     * opcodes for \c glDrawArrays.  For servers that advertise one or the
     * other, there should be a way to select which opcode to use.
     */
   GLboolean old_DrawArrays_possible;

    /**
     * Is it possible to use the new GL X.X / ARB_vertex_buffer_object
     * protocol?
     * 
     * \todo
     * This protocol has not yet been defined by the ARB, but is currently a
     * work in progress.  This field is a place-holder.
     */
   GLboolean new_DrawArrays_possible;

    /**
     * Active texture unit set by \c glClientActiveTexture.
     * 
     * \sa __glXGetActiveTextureUnit
     */
   unsigned active_texture_unit;

    /**
     * Number of supported texture units.  Even if ARB_multitexture /
     * GL 1.3 are not supported, this will be at least 1.  When multitexture
     * is supported, this will be the value queried by calling
     * \c glGetIntegerv with \c GL_MAX_TEXTURE_UNITS.
     * 
     * \todo
     * Investigate if this should be the value of \c GL_MAX_TEXTURE_COORDS
     * instead (if GL 2.0 / ARB_fragment_shader / ARB_fragment_program /
     * NV_fragment_program are supported).
     */
   unsigned num_texture_units;

    /**
     * Number of generic vertex program attribs.  If GL_ARB_vertex_program
     * is not supported, this will be zero.  Otherwise it will be the value
     * queries by calling \c glGetProgramiv with \c GL_VERTEX_PROGRAM_ARB
     * and \c GL_MAX_PROGRAM_ATTRIBS_ARB.
     */
   unsigned num_vertex_program_attribs;

    /**
     * \n Methods for implementing various GL functions.
     * 
     * These method pointers are only valid \c array_info_cache_valid is set.
     * When each function starts, it much check \c array_info_cache_valid.
     * If it is not set, it must call \c fill_array_info_cache and call
     * the new method.
     * 
     * \sa fill_array_info_cache
     * 
     * \todo
     * Write code to plug these functions directly into the dispatch table.
     */
   /*@{ */
   void (*DrawArrays) (GLenum, GLint, GLsizei);
   void (*DrawElements) (GLenum mode, GLsizei count, GLenum type,
                         const GLvoid * indices);
   /*@} */

   struct array_stack_state *stack;
   unsigned active_texture_unit_stack[__GL_CLIENT_ATTRIB_STACK_DEPTH];
   unsigned stack_index;
};

#endif /* _INDIRECT_VA_PRIVATE_ */
