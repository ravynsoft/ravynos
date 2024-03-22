/*
 * Mesa 3-D graphics library
 *
 * Copyright (C) 1999-2008  Brian Paul   All Rights Reserved.
 * Copyright (C) 2009  VMware, Inc.   All Rights Reserved.
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


#ifndef MATRIX_H
#define MATRIX_H


#include "util/glheader.h"

struct gl_context;
struct gl_matrix_stack;

extern void
_mesa_load_identity_matrix(struct gl_context *ctx, struct gl_matrix_stack *s);

extern void
_mesa_load_matrix(struct gl_context *ctx, struct gl_matrix_stack *s,
                  const GLfloat *m);

extern void 
_mesa_init_matrix( struct gl_context * ctx );

extern void 
_mesa_init_transform( struct gl_context *ctx );

extern void
_mesa_free_matrix_data( struct gl_context *ctx );

extern void 
_mesa_update_modelview_project( struct gl_context *ctx, GLuint newstate );

/* "m" must be a 4x4 matrix. Return true if it's the identity matrix. */
static inline bool
_mesa_matrix_is_identity(const float *m)
{
   const uint32_t *u = (const uint32_t *)m;
   const uint32_t one = IEEE_ONE;

   /* This is faster than memcmp with static identity matrix. Instead of
    * comparing every non-diagonal element against zero, OR them and compare
    * the result. Verified with Viewperf13/Sw/teslaTower_shaded.
    */
   return u[0] == one && u[5] == one && u[10] == one && u[15] == one &&
          !(u[1] | u[2] | u[3] | u[4] | u[6] | u[7] | u[8] | u[9] | u[11] |
            u[12] | u[13] | u[14]);
}

#endif
