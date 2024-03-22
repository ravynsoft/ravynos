/**************************************************************************
 *
 * Copyright 2015 VMware, Inc.
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
 * IN NO EVENT SHALL VMWARE AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 **************************************************************************/

/**
 * No-op GL API functions.
 *
 * Some OpenGL apps (like Viewperf12) call wglGetProcAddress() to get
 * a pointer to an extension function, get a NULL pointer, but don't bother
 * to check for NULL before jumping through the pointer.  This causes a
 * crash.
 *
 * As a work-around we provide some no-op functions here to avoid those
 * crashes.
 */

#include <GL/gl.h>
#include "stw_nopfuncs.h"
#include "util/u_debug.h"


static void
warning(const char *name)
{
   /* use name+4 to skip "nop_" prefix */
   _debug_printf("Application calling unsupported %s function\n", name+4);
}

static void APIENTRY
nop_glBindMultiTextureEXT(GLenum texunit, GLenum target, GLuint texture)
{
   warning(__func__);
}

static void APIENTRY
nop_glColor3hNV(GLhalfNV red, GLhalfNV green, GLhalfNV blue)
{
   warning(__func__);
}

static void APIENTRY
nop_glColor3hvNV(const GLhalfNV *v)
{
   warning(__func__);
}

static void APIENTRY
nop_glColor4hNV(GLhalfNV red, GLhalfNV green, GLhalfNV blue, GLhalfNV alpha)
{
   warning(__func__);
}

static void APIENTRY
nop_glColor4hvNV(const GLhalfNV *v)
{
   warning(__func__);
}

static void APIENTRY
nop_glDisableClientStateIndexedEXT(GLenum array, GLuint index)
{
   warning(__func__);
}

static void APIENTRY
nop_glEnableClientStateIndexedEXT(GLenum array, GLuint index)
{
   warning(__func__);
}

static void APIENTRY
nop_glFogCoordhNV(GLhalfNV fog)
{
   warning(__func__);
}

static void APIENTRY
nop_glFogCoordhvNV(const GLhalfNV *fog)
{
   warning(__func__);
}

static void APIENTRY
nop_glGetNamedBufferParameterivEXT(GLuint buffer, GLenum pname, GLint *params)
{
   warning(__func__);
}

static void APIENTRY
nop_glGetNamedBufferSubDataEXT(GLuint buffer, GLintptr offset, GLsizeiptr size, void *data)
{
   warning(__func__);
}

static void *APIENTRY
nop_glMapNamedBufferEXT(GLuint buffer, GLenum access)
{
   warning(__func__);
   return NULL;
}

static void APIENTRY
nop_glMatrixLoadfEXT(GLenum mode, const GLfloat *m)
{
   warning(__func__);
}

static void APIENTRY
nop_glMatrixLoadIdentityEXT(GLenum mode)
{
   warning(__func__);
}

static void APIENTRY
nop_glMultiTexCoord1hNV(GLenum target, GLhalfNV s)
{
   warning(__func__);
}

static void APIENTRY
nop_glMultiTexCoord1hvNV(GLenum target, const GLhalfNV *v)
{
   warning(__func__);
}

static void APIENTRY
nop_glMultiTexCoord2hNV(GLenum target, GLhalfNV s, GLhalfNV t)
{
   warning(__func__);
}

static void APIENTRY
nop_glMultiTexCoord2hvNV(GLenum target, const GLhalfNV *v)
{
   warning(__func__);
}

static void APIENTRY
nop_glMultiTexCoord3hNV(GLenum target, GLhalfNV s, GLhalfNV t, GLhalfNV r)
{
   warning(__func__);
}

static void APIENTRY
nop_glMultiTexCoord3hvNV(GLenum target, const GLhalfNV *v)
{
   warning(__func__);
}

static void APIENTRY
nop_glMultiTexCoord4hNV(GLenum target, GLhalfNV s, GLhalfNV t, GLhalfNV r, GLhalfNV q)
{
   warning(__func__);
}

static void APIENTRY
nop_glMultiTexCoord4hvNV(GLenum target, const GLhalfNV *v)
{
   warning(__func__);
}

static void APIENTRY
nop_glMultiTexCoordPointerEXT(GLenum texunit, GLint size, GLenum type, GLsizei stride, const void *pointer)
{
   warning(__func__);
}

static void APIENTRY
nop_glMultiTexEnvfEXT(GLenum texunit, GLenum target, GLenum pname, GLfloat param)
{
   warning(__func__);
}

static void APIENTRY
nop_glMultiTexEnvfvEXT(GLenum texunit, GLenum target, GLenum pname, const GLfloat *params)
{
   warning(__func__);
}

static void APIENTRY
nop_glMultiTexEnviEXT(GLenum texunit, GLenum target, GLenum pname, GLint param)
{
   warning(__func__);
}

static void APIENTRY
nop_glMultiTexGenfvEXT(GLenum texunit, GLenum coord, GLenum pname, const GLfloat *params)
{
   warning(__func__);
}

static void APIENTRY
nop_glMultiTexGeniEXT(GLenum texunit, GLenum coord, GLenum pname, GLint param)
{
   warning(__func__);
}

static void APIENTRY
nop_glNamedBufferDataEXT(GLuint buffer, GLsizeiptr size, const void *data, GLenum usage)
{
   warning(__func__);
}

static void APIENTRY
nop_glNamedBufferSubDataEXT(GLuint buffer, GLintptr offset, GLsizeiptr size, const void *data)
{
   warning(__func__);
}

static void APIENTRY
nop_glNamedProgramLocalParameter4fvEXT(GLuint program, GLenum target, GLuint index, const GLfloat *params)
{
   warning(__func__);
}

static void APIENTRY
nop_glNamedProgramLocalParameters4fvEXT(GLuint program, GLenum target, GLuint index, GLsizei count, const GLfloat *params)
{
   warning(__func__);
}

static void APIENTRY
nop_glNormal3hNV(GLhalfNV nx, GLhalfNV ny, GLhalfNV nz)
{
   warning(__func__);
}

static void APIENTRY
nop_glNormal3hvNV(const GLhalfNV *v)
{
   warning(__func__);
}

static void APIENTRY
nop_glPatchParameterfv(GLenum pname, const GLfloat *values)
{
   warning(__func__);
}

static void APIENTRY
nop_glPatchParameteri(GLenum pname, GLint value)
{
   warning(__func__);
}

static void APIENTRY
nop_glSecondaryColor3hNV(GLhalfNV red, GLhalfNV green, GLhalfNV blue)
{
   warning(__func__);
}

static void APIENTRY
nop_glSecondaryColor3hvNV(const GLhalfNV *v)
{
   warning(__func__);
}

static void APIENTRY
nop_glTexCoord1hNV(GLhalfNV s)
{
   warning(__func__);
}

static void APIENTRY
nop_glTexCoord1hvNV(const GLhalfNV *v)
{
   warning(__func__);
}

static void APIENTRY
nop_glTexCoord2hNV(GLhalfNV s, GLhalfNV t)
{
   warning(__func__);
}

static void APIENTRY
nop_glTexCoord2hvNV(const GLhalfNV *v)
{
   warning(__func__);
}

static void APIENTRY
nop_glTexCoord3hNV(GLhalfNV s, GLhalfNV t, GLhalfNV r)
{
   warning(__func__);
}

static void APIENTRY
nop_glTexCoord3hvNV(const GLhalfNV *v)
{
   warning(__func__);
}

static void APIENTRY
nop_glTexCoord4hNV(GLhalfNV s, GLhalfNV t, GLhalfNV r, GLhalfNV q)
{
   warning(__func__);
}

static void APIENTRY
nop_glTexCoord4hvNV(const GLhalfNV *v)
{
   warning(__func__);
}

static void APIENTRY
nop_glTextureParameterfEXT(GLuint texture, GLenum target, GLenum pname, GLfloat param)
{
   warning(__func__);
}

static void APIENTRY
nop_glTextureParameterfvEXT(GLuint texture, GLenum target, GLenum pname, const GLfloat *params)
{
   warning(__func__);
}

static void APIENTRY
nop_glTextureParameteriEXT(GLuint texture, GLenum target, GLenum pname, GLint param)
{
   warning(__func__);
}

static GLboolean APIENTRY
nop_glUnmapNamedBufferEXT(GLuint buffer)
{
   warning(__func__);
   return GL_FALSE;
}

static void APIENTRY
nop_glVertex2hNV(GLhalfNV x, GLhalfNV y)
{
   warning(__func__);
}

static void APIENTRY
nop_glVertex2hvNV(const GLhalfNV *v)
{
   warning(__func__);
}

static void APIENTRY
nop_glVertex3hNV(GLhalfNV x, GLhalfNV y, GLhalfNV z)
{
   warning(__func__);
}

static void APIENTRY
nop_glVertex3hvNV(const GLhalfNV *v)
{
   warning(__func__);
}

static void APIENTRY
nop_glVertex4hNV(GLhalfNV x, GLhalfNV y, GLhalfNV z, GLhalfNV w)
{
   warning(__func__);
}

static void APIENTRY
nop_glVertex4hvNV(const GLhalfNV *v)
{
   warning(__func__);
}


PROC
stw_get_nop_function(const char *name)
{
   struct {
      const char *name;
      PROC p;
   } table[] = {
      { "glBindMultiTextureEXT", (PROC) nop_glBindMultiTextureEXT },
      { "glColor3hNV", (PROC) nop_glColor3hNV },
      { "glColor3hvNV", (PROC) nop_glColor3hvNV },
      { "glColor4hNV", (PROC) nop_glColor4hNV },
      { "glColor4hvNV", (PROC) nop_glColor4hvNV },
      { "glDisableClientStateIndexedEXT", (PROC) nop_glDisableClientStateIndexedEXT },
      { "glEnableClientStateIndexedEXT", (PROC) nop_glEnableClientStateIndexedEXT },
      { "glFogCoordhNV", (PROC) nop_glFogCoordhNV },
      { "glFogCoordhvNV", (PROC) nop_glFogCoordhvNV },
      { "glGetNamedBufferParameterivEXT", (PROC) nop_glGetNamedBufferParameterivEXT },
      { "glGetNamedBufferSubDataEXT", (PROC) nop_glGetNamedBufferSubDataEXT },
      { "glMapNamedBufferEXT", (PROC) nop_glMapNamedBufferEXT },
      { "glMatrixLoadfEXT", (PROC) nop_glMatrixLoadfEXT },
      { "glMatrixLoadIdentityEXT", (PROC) nop_glMatrixLoadIdentityEXT },
      { "glMultiTexCoord1hNV", (PROC) nop_glMultiTexCoord1hNV },
      { "glMultiTexCoord1hvNV", (PROC) nop_glMultiTexCoord1hvNV },
      { "glMultiTexCoord2hNV", (PROC) nop_glMultiTexCoord2hNV },
      { "glMultiTexCoord2hvNV", (PROC) nop_glMultiTexCoord2hvNV },
      { "glMultiTexCoord3hNV", (PROC) nop_glMultiTexCoord3hNV },
      { "glMultiTexCoord3hvNV", (PROC) nop_glMultiTexCoord3hvNV },
      { "glMultiTexCoord4hNV", (PROC) nop_glMultiTexCoord4hNV },
      { "glMultiTexCoord4hvNV", (PROC) nop_glMultiTexCoord4hvNV },
      { "glMultiTexCoordPointerEXT", (PROC) nop_glMultiTexCoordPointerEXT },
      { "glMultiTexEnvfEXT", (PROC) nop_glMultiTexEnvfEXT },
      { "glMultiTexEnvfvEXT", (PROC) nop_glMultiTexEnvfvEXT },
      { "glMultiTexEnviEXT", (PROC) nop_glMultiTexEnviEXT },
      { "glMultiTexGenfvEXT", (PROC) nop_glMultiTexGenfvEXT },
      { "glMultiTexGeniEXT", (PROC) nop_glMultiTexGeniEXT },
      { "glNamedBufferDataEXT", (PROC) nop_glNamedBufferDataEXT },
      { "glNamedBufferSubDataEXT", (PROC) nop_glNamedBufferSubDataEXT },
      { "glNamedProgramLocalParameter4fvEXT", (PROC) nop_glNamedProgramLocalParameter4fvEXT },
      { "glNamedProgramLocalParameters4fvEXT", (PROC) nop_glNamedProgramLocalParameters4fvEXT },
      { "glNormal3hNV", (PROC) nop_glNormal3hNV },
      { "glNormal3hvNV", (PROC) nop_glNormal3hvNV },
      { "glPatchParameterfv", (PROC) nop_glPatchParameterfv },
      { "glPatchParameteri", (PROC) nop_glPatchParameteri },
      { "glSecondaryColor3hNV", (PROC) nop_glSecondaryColor3hNV },
      { "glSecondaryColor3hvNV", (PROC) nop_glSecondaryColor3hvNV },
      { "glTexCoord1hNV", (PROC) nop_glTexCoord1hNV },
      { "glTexCoord1hvNV", (PROC) nop_glTexCoord1hvNV },
      { "glTexCoord2hNV", (PROC) nop_glTexCoord2hNV },
      { "glTexCoord2hvNV", (PROC) nop_glTexCoord2hvNV },
      { "glTexCoord3hNV", (PROC) nop_glTexCoord3hNV },
      { "glTexCoord3hvNV", (PROC) nop_glTexCoord3hvNV },
      { "glTexCoord4hNV", (PROC) nop_glTexCoord4hNV },
      { "glTexCoord4hvNV", (PROC) nop_glTexCoord4hvNV },
      { "glTextureParameterfEXT", (PROC) nop_glTextureParameterfEXT },
      { "glTextureParameterfvEXT", (PROC) nop_glTextureParameterfvEXT },
      { "glTextureParameteriEXT", (PROC) nop_glTextureParameteriEXT },
      { "glUnmapNamedBufferEXT", (PROC) nop_glUnmapNamedBufferEXT },
      { "glVertex2hNV", (PROC) nop_glVertex2hNV },
      { "glVertex2hvNV", (PROC) nop_glVertex2hvNV },
      { "glVertex3hNV", (PROC) nop_glVertex3hNV },
      { "glVertex3hvNV", (PROC) nop_glVertex3hvNV },
      { "glVertex4hNV", (PROC) nop_glVertex4hNV },
      { "glVertex4hvNV", (PROC) nop_glVertex4hvNV },
      { NULL, NULL }
   };

   int i;

   for (i = 0; table[i].name; i++) {
      if (strcmp(table[i].name, name) == 0)
         return table[i].p;
   }
   return NULL;
}
