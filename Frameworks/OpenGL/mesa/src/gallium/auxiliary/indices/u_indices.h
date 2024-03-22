/*
 * Copyright 2009 VMware, Inc.
 * All Rights Reserved.
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
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.  IN NO EVENT SHALL
 * VMWARE AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef U_INDICES_H
#define U_INDICES_H

#include "util/compiler.h"
#include "pipe/p_defines.h"

/* First/last provoking vertex */
#define PV_FIRST      0
#define PV_LAST       1
#define PV_COUNT      2

/* primitive restart disable/enable flags */
#define PR_DISABLE 0
#define PR_ENABLE 1
#define PR_COUNT 2


/**
 * Index translator function (for glDrawElements() case)
 *
 * \param in     the input index buffer
 * \param start  the index of the first vertex (pipe_draw_info::start)
 * \param nr     the number of vertices (pipe_draw_info::count)
 * \param out    output buffer big enough for nr vertices (of
 *    @out_index_size bytes each)
 */
typedef void (*u_translate_func)( const void *in,
                                  unsigned start,
                                  unsigned in_nr,
                                  unsigned out_nr,
                                  unsigned restart_index,
                                  void *out );

/**
 * Index generator function (for glDrawArrays() case)
 *
 * \param start  the index of the first vertex (pipe_draw_info::start)
 * \param nr     the number of vertices (pipe_draw_info::count)
 * \param out    output buffer big enough for nr vertices (of
 *    @out_index_size bytes each)
 */
typedef void (*u_generate_func)( unsigned start,
                                 unsigned nr,
                                 void *out );


/* Return codes describe the translate/generate operation.  Caller may
 * be able to reuse translated indices under some circumstances.
 */
enum indices_mode {
   U_TRANSLATE_ERROR = -1,
   U_TRANSLATE_NORMAL = 1,
   U_TRANSLATE_MEMCPY = 2,
   U_GENERATE_LINEAR  = 3,
   U_GENERATE_REUSABLE= 4,
   U_GENERATE_ONE_OFF = 5,
};

void u_index_init( void );

/* returns the primitive type resulting from index translation */
enum mesa_prim
u_index_prim_type_convert(unsigned hw_mask, enum mesa_prim prim, bool pv_matches);

static inline unsigned
u_index_size_convert(unsigned index_size)
{
   return (index_size == 4) ? 4 : 2;
}

unsigned
u_index_count_converted_indices(unsigned hw_mask, bool pv_matches, enum mesa_prim prim, unsigned nr);

/**
 * For indexed drawing, this function determines what kind of primitive
 * transformation is needed (if any) for handling:
 * - unsupported primitive types (such as MESA_PRIM_POLYGON)
 * - changing the provoking vertex
 * - primitive restart
 * - index size (1 byte, 2 byte or 4 byte indexes)
 */
enum indices_mode
u_index_translator(unsigned hw_mask,
                   enum mesa_prim prim,
                   unsigned in_index_size,
                   unsigned nr,
                   unsigned in_pv,   /* API */
                   unsigned out_pv,  /* hardware */
                   unsigned prim_restart,
                   enum mesa_prim *out_prim,
                   unsigned *out_index_size,
                   unsigned *out_nr,
                   u_translate_func *out_translate);


/**
 * For non-indexed drawing, this function determines what kind of primitive
 * transformation is needed (see above).
 *
 * Note that even when generating it is necessary to know what the
 * API's PV is, as the indices generated will depend on whether it is
 * the same as hardware or not, and in the case of triangle strips,
 * whether it is first or last.
 */
enum indices_mode
u_index_generator(unsigned hw_mask,
                  enum mesa_prim prim,
                  unsigned start,
                  unsigned nr,
                  unsigned in_pv,   /* API */
                  unsigned out_pv,  /* hardware */
                  enum mesa_prim *out_prim,
                  unsigned *out_index_size,
                  unsigned *out_nr,
                  u_generate_func *out_generate);


void u_unfilled_init( void );

/**
 * If the driver can't handle "unfilled" primitives (i.e. drawing triangle
 * primitives as 3 lines or 3 points) this function can be used to translate
 * an indexed primitive into a new indexed primitive to draw as lines or
 * points.
 */
enum indices_mode
u_unfilled_translator(enum mesa_prim prim,
                      unsigned in_index_size,
                      unsigned nr,
                      unsigned unfilled_mode,
                      enum mesa_prim *out_prim,
                      unsigned *out_index_size,
                      unsigned *out_nr,
                      u_translate_func *out_translate);

/**
 * As above, but for non-indexed (array) primitives.
 */
enum indices_mode
u_unfilled_generator(enum mesa_prim prim,
                     unsigned start,
                     unsigned nr,
                     unsigned unfilled_mode,
                     enum mesa_prim *out_prim,
                     unsigned *out_index_size,
                     unsigned *out_nr,
                     u_generate_func *out_generate);

#endif
