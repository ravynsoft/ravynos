/*
 * (C) Copyright IBM Corporation 2005
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

#include <inttypes.h>
#include <GL/gl.h>
#include "indirect.h"
#include "glxclient.h"
#include "indirect_vertex_array.h"
#include <GL/glxproto.h>

#if !defined(__GNUC__)
#  define __builtin_expect(x, y) x
#endif

static void
do_vertex_attrib_enable(GLuint index, GLboolean val)
{
   struct glx_context *gc = __glXGetCurrentContext();
   __GLXattribute *state = (__GLXattribute *) (gc->client_state_private);

   if (!__glXSetArrayEnable(state, GL_VERTEX_ATTRIB_ARRAY_POINTER_ARB,
                            index, val)) {
      __glXSetError(gc, GL_INVALID_ENUM);
   }
}


void
__indirect_glEnableVertexAttribArray(GLuint index)
{
   do_vertex_attrib_enable(index, GL_TRUE);
}


void
__indirect_glDisableVertexAttribArray(GLuint index)
{
   do_vertex_attrib_enable(index, GL_FALSE);
}


static void
get_parameter(unsigned opcode, unsigned size, GLenum target, GLuint index,
              void *params)
{
   struct glx_context *const gc = __glXGetCurrentContext();
   Display *const dpy = gc->currentDpy;
   const GLuint cmdlen = 12;

   if (__builtin_expect(dpy != NULL, 1)) {
      GLubyte const *pc = __glXSetupVendorRequest(gc,
                                                  X_GLXVendorPrivateWithReply,
                                                  opcode, cmdlen);

      *((GLenum *) (pc + 0)) = target;
      *((GLuint *) (pc + 4)) = index;
      *((GLuint *) (pc + 8)) = 0;

      (void) __glXReadReply(dpy, size, params, GL_FALSE);
      UnlockDisplay(dpy);
      SyncHandle();
   }
   return;
}


void
__indirect_glGetProgramEnvParameterfvARB(GLenum target, GLuint index,
                                         GLfloat * params)
{
   get_parameter(1296, 4, target, index, params);
}


void
__indirect_glGetProgramEnvParameterdvARB(GLenum target, GLuint index,
                                         GLdouble * params)
{
   get_parameter(1297, 8, target, index, params);
}


void
__indirect_glGetProgramLocalParameterfvARB(GLenum target, GLuint index,
                                           GLfloat * params)
{
   get_parameter(1305, 4, target, index, params);
}


void
__indirect_glGetProgramLocalParameterdvARB(GLenum target, GLuint index,
                                           GLdouble * params)
{
   get_parameter(1306, 8, target, index, params);
}


void
__indirect_glGetVertexAttribPointerv(GLuint index, GLenum pname,
                                       GLvoid ** pointer)
{
   struct glx_context *const gc = __glXGetCurrentContext();
   __GLXattribute *state = (__GLXattribute *) (gc->client_state_private);

   if (pname != GL_VERTEX_ATTRIB_ARRAY_POINTER_ARB) {
      __glXSetError(gc, GL_INVALID_ENUM);
   }

   if (!__glXGetArrayPointer(state, GL_VERTEX_ATTRIB_ARRAY_POINTER_ARB,
                             index, pointer)) {
      __glXSetError(gc, GL_INVALID_VALUE);
   }
}


/**
 * Get the selected attribute from the vertex array state vector.
 * 
 * \returns
 * On success \c GL_TRUE is returned.  Otherwise, \c GL_FALSE is returned.
 */
static GLboolean
get_attrib_array_data(__GLXattribute * state, GLuint index, GLenum cap,
                      GLintptr * data)
{
   GLboolean retval = GL_FALSE;
   const GLenum attrib = GL_VERTEX_ATTRIB_ARRAY_POINTER_ARB;

   switch (cap) {
   case GL_VERTEX_ATTRIB_ARRAY_ENABLED_ARB:
      retval = __glXGetArrayEnable(state, attrib, index, data);
      break;

   case GL_VERTEX_ATTRIB_ARRAY_SIZE_ARB:
      retval = __glXGetArraySize(state, attrib, index, data);
      break;

   case GL_VERTEX_ATTRIB_ARRAY_STRIDE_ARB:
      retval = __glXGetArrayStride(state, attrib, index, data);
      break;

   case GL_VERTEX_ATTRIB_ARRAY_TYPE_ARB:
      retval = __glXGetArrayType(state, attrib, index, data);
      break;

   case GL_VERTEX_ATTRIB_ARRAY_NORMALIZED_ARB:
      retval = __glXGetArrayNormalized(state, attrib, index, data);
      break;
   }


   return retval;
}


static void
get_vertex_attrib(struct glx_context * gc, unsigned vop,
                  GLuint index, GLenum pname, xReply * reply)
{
   Display *const dpy = gc->currentDpy;
   GLubyte *const pc = __glXSetupVendorRequest(gc,
                                               X_GLXVendorPrivateWithReply,
                                               vop, 8);

   *((uint32_t *) (pc + 0)) = index;
   *((uint32_t *) (pc + 4)) = pname;

   (void) _XReply(dpy, reply, 0, False);
}


void
__indirect_glGetVertexAttribiv(GLuint index, GLenum pname, GLint * params)
{
   struct glx_context *const gc = __glXGetCurrentContext();
   Display *const dpy = gc->currentDpy;
   __GLXattribute *state = (__GLXattribute *) (gc->client_state_private);
   xGLXSingleReply reply;


   get_vertex_attrib(gc, 1303, index, pname, (xReply *) & reply);

   if (reply.size != 0) {
      GLintptr data;


      if (get_attrib_array_data(state, index, pname, &data)) {
         *params = (GLint) data;
      }
      else {
         if (reply.size == 1) {
            *params = (GLint) reply.pad3;
         }
         else {
            _XRead(dpy, (void *) params, 4 * reply.size);
         }
      }
   }

   UnlockDisplay(dpy);
   SyncHandle();
}


void
__indirect_glGetVertexAttribfv(GLuint index, GLenum pname,
                                  GLfloat * params)
{
   struct glx_context *const gc = __glXGetCurrentContext();
   Display *const dpy = gc->currentDpy;
   __GLXattribute *state = (__GLXattribute *) (gc->client_state_private);
   xGLXSingleReply reply;


   get_vertex_attrib(gc, 1302, index, pname, (xReply *) & reply);

   if (reply.size != 0) {
      GLintptr data;


      if (get_attrib_array_data(state, index, pname, &data)) {
         *params = (GLfloat) data;
      }
      else {
         if (reply.size == 1) {
            (void) memcpy(params, &reply.pad3, sizeof(GLfloat));
         }
         else {
            _XRead(dpy, (void *) params, 4 * reply.size);
         }
      }
   }

   UnlockDisplay(dpy);
   SyncHandle();
}


void
__indirect_glGetVertexAttribdv(GLuint index, GLenum pname,
                                  GLdouble * params)
{
   struct glx_context *const gc = __glXGetCurrentContext();
   Display *const dpy = gc->currentDpy;
   __GLXattribute *state = (__GLXattribute *) (gc->client_state_private);
   xGLXSingleReply reply;


   get_vertex_attrib(gc, 1301, index, pname, (xReply *) & reply);

   if (reply.size != 0) {
      GLintptr data;


      if (get_attrib_array_data(state, index, pname, &data)) {
         *params = (GLdouble) data;
      }
      else {
         if (reply.size == 1) {
            (void) memcpy(params, &reply.pad3, sizeof(GLdouble));
         }
         else {
            _XRead(dpy, (void *) params, 8 * reply.size);
         }
      }
   }

   UnlockDisplay(dpy);
   SyncHandle();
}
