/*
 * Mesa 3-D graphics library
 *
 * Copyright (C) 1999-2006  Brian Paul   All Rights Reserved.
 * Copyright (C) 2011  VMware, Inc.  All Rights Reserved.
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
 * GLvertexformat no-op functions.  Used in out-of-memory situations.
 */

#ifdef _WIN32
#define NOGDI
#endif

#include "util/glheader.h"
#include "main/context.h"
#include "main/dispatch.h"
#include "main/dlist.h"
#include "main/eval.h"
#include "vbo_attrib.h"
#include "api_exec_decl.h"

static void GLAPIENTRY
_mesa_noop_Materialfv(GLenum face, GLenum pname, const GLfloat * params)
{
}

static void GLAPIENTRY
_mesa_noop_EvalCoord1f(GLfloat a)
{
}

static void GLAPIENTRY
_mesa_noop_EvalCoord1fv(const GLfloat * v)
{
}

static void GLAPIENTRY
_mesa_noop_EvalCoord2f(GLfloat a, GLfloat b)
{
}

static void GLAPIENTRY
_mesa_noop_EvalCoord2fv(const GLfloat * v)
{
}

static void GLAPIENTRY
_mesa_noop_EvalPoint1(GLint a)
{
}

static void GLAPIENTRY
_mesa_noop_EvalPoint2(GLint a, GLint b)
{
}

static void GLAPIENTRY
_mesa_noop_ArrayElement(GLint elem)
{
}


static void GLAPIENTRY
_mesa_noop_Begin(GLenum mode)
{
}

static void GLAPIENTRY
_mesa_noop_End(void)
{
}

static void GLAPIENTRY
_mesa_noop_PrimitiveRestartNV(void)
{
}

/**
 * If index=0, does glVertexAttrib*() alias glVertex() to emit a vertex?
 * It depends on a few things, including whether we're inside or outside
 * of glBegin/glEnd.
 */
static inline bool
is_vertex_position(const struct gl_context *ctx, GLuint index)
{
   return false; /* it doesn't matter for noop */
}

#define ATTR_UNION(A, N, T, C, V0, V1, V2, V3) do { (void)ctx; (void)(A); } while(0)
#define ERROR(err) _mesa_error(ctx, err, __func__)
#define TAG(x) _mesa_noop_##x

#include "vbo_attrib_tmp.h"


/**
 * Build a vertexformat of functions that are no-ops.
 * These are used in out-of-memory situations when we have no VBO
 * to put the vertex data into.
 */
void
vbo_install_exec_vtxfmt_noop(struct gl_context *ctx)
{
#define NAME_AE(x) _mesa_noop_##x
#define NAME_CALLLIST(x) _mesa_##x
#define NAME(x) _mesa_noop_##x
#define NAME_ES(x) _mesa_noop_##x

   struct _glapi_table *tab = ctx->Dispatch.Exec;
   #include "api_beginend_init.h"

   if (ctx->Dispatch.BeginEnd) {
      tab = ctx->Dispatch.BeginEnd;
      #include "api_beginend_init.h"
   }

   if (ctx->Dispatch.HWSelectModeBeginEnd) {
      tab = ctx->Dispatch.HWSelectModeBeginEnd;
      #include "api_beginend_init.h"
   }
}


void
vbo_install_save_vtxfmt_noop(struct gl_context *ctx)
{
   struct _glapi_table *tab = ctx->Dispatch.Save;
   #include "api_beginend_init.h"
}

/**
 * Is the given dispatch table using the no-op functions?
 */
GLboolean
_mesa_using_noop_vtxfmt(const struct _glapi_table *dispatch)
{
   return GET_Begin((struct _glapi_table *) dispatch) == _mesa_noop_Begin;
}
