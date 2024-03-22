/*
 * SGI FREE SOFTWARE LICENSE B (Version 2.0, Sept. 18, 2008)
 * Copyright (C) 1991-2000 Silicon Graphics, Inc. All Rights Reserved.
 *
 * SPDX-License-Identifier: SGI-B-2.0
 */

#ifndef GLX_USE_APPLEGL

#include "packrender.h"
#include "indirect.h"
#include "indirect_size.h"

/*
** This file contains routines that might need to be transported as
** GLXRender or GLXRenderLarge commands, and these commands don't
** use the pixel header.  See renderpix.c for those routines.
*/

void
__indirect_glMap1d(GLenum target, GLdouble u1, GLdouble u2, GLint stride,
                   GLint order, const GLdouble * pnts)
{
   __GLX_DECLARE_VARIABLES();
   GLint k;

   __GLX_LOAD_VARIABLES();
   k = __glMap1d_size(target);
   if (k == 0) {
      __glXSetError(gc, GL_INVALID_ENUM);
      return;
   }
   else if (stride < k || order <= 0) {
      __glXSetError(gc, GL_INVALID_VALUE);
      return;
   }
   compsize = k * order * __GLX_SIZE_FLOAT64;
   cmdlen = 28 + compsize;
   if (!gc->currentDpy)
      return;

   if (cmdlen <= gc->maxSmallRenderCommandSize) {
      /* Use GLXRender protocol to send small command */
      __GLX_BEGIN_VARIABLE(X_GLrop_Map1d, cmdlen);
      __GLX_PUT_DOUBLE(4, u1);
      __GLX_PUT_DOUBLE(12, u2);
      __GLX_PUT_LONG(20, target);
      __GLX_PUT_LONG(24, order);
      /*
       ** NOTE: the doubles that follow are not aligned because of 3
       ** longs preceeding
       */
      __glFillMap1d(k, order, stride, pnts, (pc + 28));
      __GLX_END(cmdlen);
   }
   else {
      /* Use GLXRenderLarge protocol to send command */
      __GLX_BEGIN_VARIABLE_LARGE(X_GLrop_Map1d, cmdlen + 4);
      __GLX_PUT_DOUBLE(8, u1);
      __GLX_PUT_DOUBLE(16, u2);
      __GLX_PUT_LONG(24, target);
      __GLX_PUT_LONG(28, order);

      /*
       ** NOTE: the doubles that follow are not aligned because of 3
       ** longs preceeding
       */
      if (stride != k) {
         GLubyte *buf;

         buf = malloc(compsize);
         if (!buf) {
            __glXSetError(gc, GL_OUT_OF_MEMORY);
            return;
         }
         __glFillMap1d(k, order, stride, pnts, buf);
         __glXSendLargeCommand(gc, pc, 32, buf, compsize);
         free((char *) buf);
      }
      else {
         /* Data is already packed.  Just send it out */
         __glXSendLargeCommand(gc, pc, 32, pnts, compsize);
      }
   }
}

void
__indirect_glMap1f(GLenum target, GLfloat u1, GLfloat u2, GLint stride,
                   GLint order, const GLfloat * pnts)
{
   __GLX_DECLARE_VARIABLES();
   GLint k;

   __GLX_LOAD_VARIABLES();
   k = __glMap1f_size(target);
   if (k == 0) {
      __glXSetError(gc, GL_INVALID_ENUM);
      return;
   }
   else if (stride < k || order <= 0) {
      __glXSetError(gc, GL_INVALID_VALUE);
      return;
   }
   compsize = k * order * __GLX_SIZE_FLOAT32;
   cmdlen = 20 + compsize;
   if (!gc->currentDpy)
      return;

   /*
    ** The order that arguments are packed is different from the order
    ** for glMap1d.
    */
   if (cmdlen <= gc->maxSmallRenderCommandSize) {
      /* Use GLXRender protocol to send small command */
      __GLX_BEGIN_VARIABLE(X_GLrop_Map1f, cmdlen);
      __GLX_PUT_LONG(4, target);
      __GLX_PUT_FLOAT(8, u1);
      __GLX_PUT_FLOAT(12, u2);
      __GLX_PUT_LONG(16, order);
      __glFillMap1f(k, order, stride, pnts, (GLubyte *) (pc + 20));
      __GLX_END(cmdlen);
   }
   else {
      /* Use GLXRenderLarge protocol to send command */
      __GLX_BEGIN_VARIABLE_LARGE(X_GLrop_Map1f, cmdlen + 4);
      __GLX_PUT_LONG(8, target);
      __GLX_PUT_FLOAT(12, u1);
      __GLX_PUT_FLOAT(16, u2);
      __GLX_PUT_LONG(20, order);

      if (stride != k) {
         GLubyte *buf;

         buf = malloc(compsize);
         if (!buf) {
            __glXSetError(gc, GL_OUT_OF_MEMORY);
            return;
         }
         __glFillMap1f(k, order, stride, pnts, buf);
         __glXSendLargeCommand(gc, pc, 24, buf, compsize);
         free((char *) buf);
      }
      else {
         /* Data is already packed.  Just send it out */
         __glXSendLargeCommand(gc, pc, 24, pnts, compsize);
      }
   }
}

void
__indirect_glMap2d(GLenum target, GLdouble u1, GLdouble u2, GLint ustr,
                   GLint uord, GLdouble v1, GLdouble v2, GLint vstr,
                   GLint vord, const GLdouble * pnts)
{
   __GLX_DECLARE_VARIABLES();
   GLint k;

   __GLX_LOAD_VARIABLES();
   k = __glMap2d_size(target);
   if (k == 0) {
      __glXSetError(gc, GL_INVALID_ENUM);
      return;
   }
   else if (vstr < k || ustr < k || vord <= 0 || uord <= 0) {
      __glXSetError(gc, GL_INVALID_VALUE);
      return;
   }
   compsize = k * uord * vord * __GLX_SIZE_FLOAT64;
   cmdlen = 48 + compsize;
   if (!gc->currentDpy)
      return;

   if (cmdlen <= gc->maxSmallRenderCommandSize) {
      /* Use GLXRender protocol to send small command */
      __GLX_BEGIN_VARIABLE(X_GLrop_Map2d, cmdlen);
      __GLX_PUT_DOUBLE(4, u1);
      __GLX_PUT_DOUBLE(12, u2);
      __GLX_PUT_DOUBLE(20, v1);
      __GLX_PUT_DOUBLE(28, v2);
      __GLX_PUT_LONG(36, target);
      __GLX_PUT_LONG(40, uord);
      __GLX_PUT_LONG(44, vord);
      /*
       ** Pack into a u-major ordering.
       ** NOTE: the doubles that follow are not aligned because of 5
       ** longs preceeding
       */
      __glFillMap2d(k, uord, vord, ustr, vstr, pnts, (GLdouble *) (pc + 48));
      __GLX_END(cmdlen);
   }
   else {
      /* Use GLXRenderLarge protocol to send command */
      __GLX_BEGIN_VARIABLE_LARGE(X_GLrop_Map2d, cmdlen + 4);
      __GLX_PUT_DOUBLE(8, u1);
      __GLX_PUT_DOUBLE(16, u2);
      __GLX_PUT_DOUBLE(24, v1);
      __GLX_PUT_DOUBLE(32, v2);
      __GLX_PUT_LONG(40, target);
      __GLX_PUT_LONG(44, uord);
      __GLX_PUT_LONG(48, vord);

      /*
       ** NOTE: the doubles that follow are not aligned because of 5
       ** longs preceeding
       */
      if ((vstr != k) || (ustr != k * vord)) {
         GLdouble *buf;

         buf = malloc(compsize);
         if (!buf) {
            __glXSetError(gc, GL_OUT_OF_MEMORY);
            return;
         }
         /*
          ** Pack into a u-major ordering.
          */
         __glFillMap2d(k, uord, vord, ustr, vstr, pnts, buf);
         __glXSendLargeCommand(gc, pc, 52, buf, compsize);
         free((char *) buf);
      }
      else {
         /* Data is already packed.  Just send it out */
         __glXSendLargeCommand(gc, pc, 52, pnts, compsize);
      }
   }
}

void
__indirect_glMap2f(GLenum target, GLfloat u1, GLfloat u2, GLint ustr,
                   GLint uord, GLfloat v1, GLfloat v2, GLint vstr, GLint vord,
                   const GLfloat * pnts)
{
   __GLX_DECLARE_VARIABLES();
   GLint k;

   __GLX_LOAD_VARIABLES();
   k = __glMap2f_size(target);
   if (k == 0) {
      __glXSetError(gc, GL_INVALID_ENUM);
      return;
   }
   else if (vstr < k || ustr < k || vord <= 0 || uord <= 0) {
      __glXSetError(gc, GL_INVALID_VALUE);
      return;
   }
   compsize = k * uord * vord * __GLX_SIZE_FLOAT32;
   cmdlen = 32 + compsize;
   if (!gc->currentDpy)
      return;

   /*
    ** The order that arguments are packed is different from the order
    ** for glMap2d.
    */
   if (cmdlen <= gc->maxSmallRenderCommandSize) {
      /* Use GLXRender protocol to send small command */
      __GLX_BEGIN_VARIABLE(X_GLrop_Map2f, cmdlen);
      __GLX_PUT_LONG(4, target);
      __GLX_PUT_FLOAT(8, u1);
      __GLX_PUT_FLOAT(12, u2);
      __GLX_PUT_LONG(16, uord);
      __GLX_PUT_FLOAT(20, v1);
      __GLX_PUT_FLOAT(24, v2);
      __GLX_PUT_LONG(28, vord);
      /*
       ** Pack into a u-major ordering.
       */
      __glFillMap2f(k, uord, vord, ustr, vstr, pnts, (GLfloat *) (pc + 32));
      __GLX_END(cmdlen);
   }
   else {
      /* Use GLXRenderLarge protocol to send command */
      __GLX_BEGIN_VARIABLE_LARGE(X_GLrop_Map2f, cmdlen + 4);
      __GLX_PUT_LONG(8, target);
      __GLX_PUT_FLOAT(12, u1);
      __GLX_PUT_FLOAT(16, u2);
      __GLX_PUT_LONG(20, uord);
      __GLX_PUT_FLOAT(24, v1);
      __GLX_PUT_FLOAT(28, v2);
      __GLX_PUT_LONG(32, vord);

      if ((vstr != k) || (ustr != k * vord)) {
         GLfloat *buf;

         buf = malloc(compsize);
         if (!buf) {
            __glXSetError(gc, GL_OUT_OF_MEMORY);
            return;
         }
         /*
          ** Pack into a u-major ordering.
          */
         __glFillMap2f(k, uord, vord, ustr, vstr, pnts, buf);
         __glXSendLargeCommand(gc, pc, 36, buf, compsize);
         free((char *) buf);
      }
      else {
         /* Data is already packed.  Just send it out */
         __glXSendLargeCommand(gc, pc, 36, pnts, compsize);
      }
   }
}

void
__indirect_glEnable(GLenum cap)
{
   __GLX_DECLARE_VARIABLES();

   __GLX_LOAD_VARIABLES();
   if (!gc->currentDpy)
      return;

   switch (cap) {
   case GL_COLOR_ARRAY:
   case GL_EDGE_FLAG_ARRAY:
   case GL_INDEX_ARRAY:
   case GL_NORMAL_ARRAY:
   case GL_TEXTURE_COORD_ARRAY:
   case GL_VERTEX_ARRAY:
   case GL_SECONDARY_COLOR_ARRAY:
   case GL_FOG_COORD_ARRAY:
      __indirect_glEnableClientState(cap);
      return;
   default:
      break;
   }

   __GLX_BEGIN(X_GLrop_Enable, 8);
   __GLX_PUT_LONG(4, cap);
   __GLX_END(8);
}

void
__indirect_glDisable(GLenum cap)
{
   __GLX_DECLARE_VARIABLES();

   __GLX_LOAD_VARIABLES();
   if (!gc->currentDpy)
      return;

   switch (cap) {
   case GL_COLOR_ARRAY:
   case GL_EDGE_FLAG_ARRAY:
   case GL_INDEX_ARRAY:
   case GL_NORMAL_ARRAY:
   case GL_TEXTURE_COORD_ARRAY:
   case GL_VERTEX_ARRAY:
   case GL_SECONDARY_COLOR_ARRAY:
   case GL_FOG_COORD_ARRAY:
      __indirect_glDisableClientState(cap);
      return;
   default:
      break;
   }

   __GLX_BEGIN(X_GLrop_Disable, 8);
   __GLX_PUT_LONG(4, cap);
   __GLX_END(8);
}

#endif
