/*
 * SGI FREE SOFTWARE LICENSE B (Version 2.0, Sept. 18, 2008)
 * Copyright (C) 1991-2000 Silicon Graphics, Inc. All Rights Reserved.
 *
 * SPDX-License-Identifier: SGI-B-2.0
 */

#include "packrender.h"

/*
** Routines to pack evaluator maps into the transport buffer.  Maps are
** allowed to have extra arbitrary data, so these routines extract just
** the information that the GL needs.
*/

void
__glFillMap1f(GLint k, GLint order, GLint stride,
              const GLfloat * points, GLubyte * pc)
{
   if (stride == k) {
      /* Just copy the data */
      __GLX_PUT_FLOAT_ARRAY(0, points, order * k);
   }
   else {
      GLint i;

      for (i = 0; i < order; i++) {
         __GLX_PUT_FLOAT_ARRAY(0, points, k);
         points += stride;
         pc += k * __GLX_SIZE_FLOAT32;
      }
   }
}

void
__glFillMap1d(GLint k, GLint order, GLint stride,
              const GLdouble * points, GLubyte * pc)
{
   if (stride == k) {
      /* Just copy the data */
      __GLX_PUT_DOUBLE_ARRAY(0, points, order * k);
   }
   else {
      GLint i;
      for (i = 0; i < order; i++) {
         __GLX_PUT_DOUBLE_ARRAY(0, points, k);
         points += stride;
         pc += k * __GLX_SIZE_FLOAT64;
      }
   }
}

void
__glFillMap2f(GLint k, GLint majorOrder, GLint minorOrder,
              GLint majorStride, GLint minorStride,
              const GLfloat * points, GLfloat * data)
{
   GLint i, j, x;

   if ((minorStride == k) && (majorStride == minorOrder * k)) {
      /* Just copy the data */
      __GLX_MEM_COPY(data, points, majorOrder * majorStride *
                     __GLX_SIZE_FLOAT32);
      return;
   }
   for (i = 0; i < majorOrder; i++) {
      for (j = 0; j < minorOrder; j++) {
         for (x = 0; x < k; x++) {
            data[x] = points[x];
         }
         points += minorStride;
         data += k;
      }
      points += majorStride - minorStride * minorOrder;
   }
}

void
__glFillMap2d(GLint k, GLint majorOrder, GLint minorOrder,
              GLint majorStride, GLint minorStride,
              const GLdouble * points, GLdouble * data)
{
   int i, j, x;

   if ((minorStride == k) && (majorStride == minorOrder * k)) {
      /* Just copy the data */
      __GLX_MEM_COPY(data, points, majorOrder * majorStride *
                     __GLX_SIZE_FLOAT64);
      return;
   }

#ifdef __GLX_ALIGN64
   x = k * __GLX_SIZE_FLOAT64;
#endif
   for (i = 0; i < majorOrder; i++) {
      for (j = 0; j < minorOrder; j++) {
#ifdef __GLX_ALIGN64
         __GLX_MEM_COPY(data, points, x);
#else
         for (x = 0; x < k; x++) {
            data[x] = points[x];
         }
#endif
         points += minorStride;
         data += k;
      }
      points += majorStride - minorStride * minorOrder;
   }
}
