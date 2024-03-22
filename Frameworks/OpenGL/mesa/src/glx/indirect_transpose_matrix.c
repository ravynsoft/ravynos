/*
 * (C) Copyright IBM Corporation 2004
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
 * THE COPYRIGHT HOLDERS AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <GL/gl.h>
#include "indirect.h"

static void
TransposeMatrixf(const GLfloat s[16], GLfloat d[16])
{
   int i, j;
   for (i = 0; i < 4; i++) {
      for (j = 0; j < 4; j++) {
         d[i * 4 + j] = s[j * 4 + i];
      }
   }
}

static void
TransposeMatrixd(const GLdouble s[16], GLdouble d[16])
{
   int i, j;
   for (i = 0; i < 4; i++) {
      for (j = 0; j < 4; j++) {
         d[i * 4 + j] = s[j * 4 + i];
      }
   }
}


void
__indirect_glLoadTransposeMatrixd(const GLdouble * m)
{
   GLdouble mt[16];

   TransposeMatrixd(m, mt);
   __indirect_glLoadMatrixd(mt);
}

void
__indirect_glLoadTransposeMatrixf(const GLfloat * m)
{
   GLfloat mt[16];

   TransposeMatrixf(m, mt);
   __indirect_glLoadMatrixf(mt);
}

void
__indirect_glMultTransposeMatrixd(const GLdouble * m)
{
   GLdouble mt[16];

   TransposeMatrixd(m, mt);
   __indirect_glMultMatrixd(mt);
}

void
__indirect_glMultTransposeMatrixf(const GLfloat * m)
{
   GLfloat mt[16];

   TransposeMatrixf(m, mt);
   __indirect_glMultMatrixf(mt);
}
