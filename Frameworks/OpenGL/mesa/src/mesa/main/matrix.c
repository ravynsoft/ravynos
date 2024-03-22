/*
 * Mesa 3-D graphics library
 *
 * Copyright (C) 1999-2008  Brian Paul   All Rights Reserved.
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
 * \file matrix.c
 * Matrix operations.
 *
 * \note
 * -# 4x4 transformation matrices are stored in memory in column major order.
 * -# Points/vertices are to be thought of as column vectors.
 * -# Transformation of a point p by a matrix M is: p' = M * p
 */


#include "util/glheader.h"

#include "context.h"
#include "enums.h"
#include "macros.h"
#include "matrix.h"
#include "mtypes.h"
#include "math/m_matrix.h"
#include "util/bitscan.h"
#include "api_exec_decl.h"


static struct gl_matrix_stack *
get_named_matrix_stack(struct gl_context *ctx, GLenum mode, const char* caller)
{
   switch (mode) {
   case GL_MODELVIEW:
      return &ctx->ModelviewMatrixStack;
   case GL_PROJECTION:
      return &ctx->ProjectionMatrixStack;
   case GL_TEXTURE:
      /* This error check is disabled because if we're called from
       * glPopAttrib() when the active texture unit is >= MaxTextureCoordUnits
       * we'll generate an unexpected error.
       * From the GL_ARB_vertex_shader spec it sounds like we should instead
       * do error checking in other places when we actually try to access
       * texture matrices beyond MaxTextureCoordUnits.
       */
#if 0
      if (ctx->Texture.CurrentUnit >= ctx->Const.MaxTextureCoordUnits) {
         _mesa_error(ctx, GL_INVALID_OPERATION,
                     "glMatrixMode(invalid tex unit %d)",
                     ctx->Texture.CurrentUnit);
         return;
      }
#endif
      assert(ctx->Texture.CurrentUnit < ARRAY_SIZE(ctx->TextureMatrixStack));
      return &ctx->TextureMatrixStack[ctx->Texture.CurrentUnit];
   case GL_MATRIX0_ARB:
   case GL_MATRIX1_ARB:
   case GL_MATRIX2_ARB:
   case GL_MATRIX3_ARB:
   case GL_MATRIX4_ARB:
   case GL_MATRIX5_ARB:
   case GL_MATRIX6_ARB:
   case GL_MATRIX7_ARB:
      if (_mesa_is_desktop_gl_compat(ctx)
          && (ctx->Extensions.ARB_vertex_program ||
              ctx->Extensions.ARB_fragment_program)) {
         const GLuint m = mode - GL_MATRIX0_ARB;
         if (m <= ctx->Const.MaxProgramMatrices)
            return &ctx->ProgramMatrixStack[m];
      }
      FALLTHROUGH;
   default:
      break;
   }
   if (mode >= GL_TEXTURE0 && mode < (GL_TEXTURE0 + ctx->Const.MaxTextureCoordUnits)) {
      return &ctx->TextureMatrixStack[mode - GL_TEXTURE0];
   }
   _mesa_error(ctx, GL_INVALID_ENUM, "%s", caller);
   return NULL;
}


static void matrix_frustum(struct gl_matrix_stack* stack,
                           GLdouble left, GLdouble right,
                           GLdouble bottom, GLdouble top,
                           GLdouble nearval, GLdouble farval,
                           const char* caller)
{
   GET_CURRENT_CONTEXT(ctx);
   if (nearval <= 0.0 ||
       farval <= 0.0 ||
       nearval == farval ||
       left == right ||
       top == bottom) {
      _mesa_error(ctx, GL_INVALID_VALUE, "%s", caller);
      return;
   }

   FLUSH_VERTICES(ctx, 0, 0);

   _math_matrix_frustum(stack->Top,
                        (GLfloat) left, (GLfloat) right,
                        (GLfloat) bottom, (GLfloat) top,
                        (GLfloat) nearval, (GLfloat) farval);
   stack->ChangedSincePush = true;
   ctx->NewState |= stack->DirtyFlag;
}


/**
 * Apply a perspective projection matrix.
 *
 * \param left left clipping plane coordinate.
 * \param right right clipping plane coordinate.
 * \param bottom bottom clipping plane coordinate.
 * \param top top clipping plane coordinate.
 * \param nearval distance to the near clipping plane.
 * \param farval distance to the far clipping plane.
 *
 * \sa glFrustum().
 *
 * Flushes vertices and validates parameters. Calls _math_matrix_frustum() with
 * the top matrix of the current matrix stack and sets
 * __struct gl_contextRec::NewState.
 */
void GLAPIENTRY
_mesa_Frustum( GLdouble left, GLdouble right,
               GLdouble bottom, GLdouble top,
               GLdouble nearval, GLdouble farval )
{
   GET_CURRENT_CONTEXT(ctx);
   matrix_frustum(ctx->CurrentStack,
                  (GLfloat) left, (GLfloat) right,
			         (GLfloat) bottom, (GLfloat) top,
			         (GLfloat) nearval, (GLfloat) farval,
                  "glFrustum");
}


void GLAPIENTRY
_mesa_MatrixFrustumEXT( GLenum matrixMode,
                        GLdouble left, GLdouble right,
                        GLdouble bottom, GLdouble top,
                        GLdouble nearval, GLdouble farval )
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_matrix_stack *stack = get_named_matrix_stack(ctx, matrixMode,
                                                          "glMatrixFrustumEXT");
   if (!stack)
      return;

   matrix_frustum(stack,
                  (GLfloat) left, (GLfloat) right,
                  (GLfloat) bottom, (GLfloat) top,
                  (GLfloat) nearval, (GLfloat) farval,
                  "glMatrixFrustumEXT");
}


static void
matrix_ortho(struct gl_matrix_stack* stack,
             GLdouble left, GLdouble right,
             GLdouble bottom, GLdouble top,
             GLdouble nearval, GLdouble farval,
             const char* caller)
{
   GET_CURRENT_CONTEXT(ctx);

   if (MESA_VERBOSE & VERBOSE_API)
      _mesa_debug(ctx, "%s(%f, %f, %f, %f, %f, %f)\n", caller,
                  left, right, bottom, top, nearval, farval);

   if (left == right ||
       bottom == top ||
       nearval == farval)
   {
      _mesa_error( ctx,  GL_INVALID_VALUE, "%s", caller );
      return;
   }

   FLUSH_VERTICES(ctx, 0, 0);

   _math_matrix_ortho( stack->Top,
                       (GLfloat) left, (GLfloat) right,
             (GLfloat) bottom, (GLfloat) top,
             (GLfloat) nearval, (GLfloat) farval );
   stack->ChangedSincePush = true;
   ctx->NewState |= stack->DirtyFlag;
}


/**
 * Apply an orthographic projection matrix.
 *
 * \param left left clipping plane coordinate.
 * \param right right clipping plane coordinate.
 * \param bottom bottom clipping plane coordinate.
 * \param top top clipping plane coordinate.
 * \param nearval distance to the near clipping plane.
 * \param farval distance to the far clipping plane.
 *
 * \sa glOrtho().
 *
 * Flushes vertices and validates parameters. Calls _math_matrix_ortho() with
 * the top matrix of the current matrix stack and sets
 * __struct gl_contextRec::NewState.
 */
void GLAPIENTRY
_mesa_Ortho( GLdouble left, GLdouble right,
             GLdouble bottom, GLdouble top,
             GLdouble nearval, GLdouble farval )
{
   GET_CURRENT_CONTEXT(ctx);
   matrix_ortho(ctx->CurrentStack,
                (GLfloat) left, (GLfloat) right,
		          (GLfloat) bottom, (GLfloat) top,
		          (GLfloat) nearval, (GLfloat) farval,
                "glOrtho");
}


void GLAPIENTRY
_mesa_MatrixOrthoEXT( GLenum matrixMode,
                      GLdouble left, GLdouble right,
                      GLdouble bottom, GLdouble top,
                      GLdouble nearval, GLdouble farval )
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_matrix_stack *stack = get_named_matrix_stack(ctx, matrixMode,
                                                          "glMatrixOrthoEXT");
   if (!stack)
      return;

   matrix_ortho(stack,
                (GLfloat) left, (GLfloat) right,
                (GLfloat) bottom, (GLfloat) top,
                (GLfloat) nearval, (GLfloat) farval,
                "glMatrixOrthoEXT");
}


/**
 * Set the current matrix stack.
 *
 * \param mode matrix stack.
 *
 * \sa glMatrixMode().
 *
 * Flushes the vertices, validates the parameter and updates
 * __struct gl_contextRec::CurrentStack and gl_transform_attrib::MatrixMode
 * with the specified matrix stack.
 */
void GLAPIENTRY
_mesa_MatrixMode( GLenum mode )
{
   struct gl_matrix_stack * stack;
   GET_CURRENT_CONTEXT(ctx);

   if (ctx->Transform.MatrixMode == mode && mode != GL_TEXTURE)
      return;

   if (mode >= GL_TEXTURE0 && mode < (GL_TEXTURE0 + ctx->Const.MaxTextureCoordUnits)) {
      stack = NULL;
   } else {
      stack = get_named_matrix_stack(ctx, mode, "glMatrixMode");
   }

   if (stack) {
      ctx->CurrentStack = stack;
      ctx->Transform.MatrixMode = mode;
      ctx->PopAttribState |= GL_TRANSFORM_BIT;
   }
}


static void
push_matrix(struct gl_context *ctx, struct gl_matrix_stack *stack,
            GLenum matrixMode, const char *func)
{
   if (stack->Depth + 1 >= stack->MaxDepth) {
      if (ctx->Transform.MatrixMode == GL_TEXTURE) {
         _mesa_error(ctx, GL_STACK_OVERFLOW, "%s(mode=GL_TEXTURE, unit=%d)",
                     func, ctx->Texture.CurrentUnit);
      } else {
         _mesa_error(ctx, GL_STACK_OVERFLOW, "%s(mode=%s)",
                     func, _mesa_enum_to_string(matrixMode));
      }
      return;
   }

   if (stack->Depth + 1 >= stack->StackSize) {
      unsigned new_stack_size = stack->StackSize * 2;
      unsigned i;
      GLmatrix *new_stack = realloc(stack->Stack,
                                    sizeof(*new_stack) * new_stack_size);

      if (!new_stack) {
         _mesa_error(ctx, GL_OUT_OF_MEMORY, "%s", func);
         return;
      }

      for (i = stack->StackSize; i < new_stack_size; i++)
         _math_matrix_ctr(&new_stack[i]);

      stack->Stack = new_stack;
      stack->StackSize = new_stack_size;
   }

   _math_matrix_push_copy(&stack->Stack[stack->Depth + 1],
                          &stack->Stack[stack->Depth]);
   stack->Depth++;
   stack->Top = &(stack->Stack[stack->Depth]);
   stack->ChangedSincePush = false;
}


/**
 * Push the current matrix stack.
 *
 * \sa glPushMatrix().
 *
 * Verifies the current matrix stack is not full, and duplicates the top-most
 * matrix in the stack.
 * Marks __struct gl_contextRec::NewState with the stack dirty flag.
 */
void GLAPIENTRY
_mesa_PushMatrix( void )
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_matrix_stack *stack = ctx->CurrentStack;

   if (MESA_VERBOSE&VERBOSE_API)
      _mesa_debug(ctx, "glPushMatrix %s\n",
                  _mesa_enum_to_string(ctx->Transform.MatrixMode));

   push_matrix(ctx, stack, ctx->Transform.MatrixMode, "glPushMatrix");
}


void GLAPIENTRY
_mesa_MatrixPushEXT( GLenum matrixMode )
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_matrix_stack *stack = get_named_matrix_stack(ctx, matrixMode,
                                                          "glMatrixPushEXT");
   ASSERT_OUTSIDE_BEGIN_END(ctx);
   if (stack)
      push_matrix(ctx, stack, matrixMode, "glMatrixPushEXT");
}


static GLboolean
pop_matrix( struct gl_context *ctx, struct gl_matrix_stack *stack )
{
   if (stack->Depth == 0)
      return GL_FALSE;

   stack->Depth--;

   /* If the popped matrix is the same as the current one, treat it as
    * a no-op change.
    */
   if (stack->ChangedSincePush &&
       memcmp(stack->Top, &stack->Stack[stack->Depth],
              sizeof(GLmatrix))) {
      FLUSH_VERTICES(ctx, 0, 0);
      ctx->NewState |= stack->DirtyFlag;
   }

   stack->Top = &(stack->Stack[stack->Depth]);
   stack->ChangedSincePush = true;
   return GL_TRUE;
}


/**
 * Pop the current matrix stack.
 *
 * \sa glPopMatrix().
 *
 * Flushes the vertices, verifies the current matrix stack is not empty, and
 * moves the stack head down.
 * Marks __struct gl_contextRec::NewState with the dirty stack flag.
 */
void GLAPIENTRY
_mesa_PopMatrix( void )
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_matrix_stack *stack = ctx->CurrentStack;

   if (MESA_VERBOSE&VERBOSE_API)
      _mesa_debug(ctx, "glPopMatrix %s\n",
                  _mesa_enum_to_string(ctx->Transform.MatrixMode));

   if (!pop_matrix(ctx, stack)) {
      if (ctx->Transform.MatrixMode == GL_TEXTURE) {
         _mesa_error(ctx, GL_STACK_UNDERFLOW,
                     "glPopMatrix(mode=GL_TEXTURE, unit=%d)",
                      ctx->Texture.CurrentUnit);
      }
      else {
         _mesa_error(ctx, GL_STACK_UNDERFLOW, "glPopMatrix(mode=%s)",
                     _mesa_enum_to_string(ctx->Transform.MatrixMode));
      }
   }
}


void GLAPIENTRY
_mesa_MatrixPopEXT( GLenum matrixMode )
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_matrix_stack *stack = get_named_matrix_stack(ctx, matrixMode,
                                                          "glMatrixPopEXT");
   if (!stack)
      return;

   if (!pop_matrix(ctx, stack)) {
      if (matrixMode == GL_TEXTURE) {
         _mesa_error(ctx, GL_STACK_UNDERFLOW,
                     "glMatrixPopEXT(mode=GL_TEXTURE, unit=%d)",
                      ctx->Texture.CurrentUnit);
      }
      else {
         _mesa_error(ctx, GL_STACK_UNDERFLOW, "glMatrixPopEXT(mode=%s)",
                     _mesa_enum_to_string(matrixMode));
      }
   }
}


void
_mesa_load_identity_matrix(struct gl_context *ctx, struct gl_matrix_stack *stack)
{
   FLUSH_VERTICES(ctx, 0, 0);

   _math_matrix_set_identity(stack->Top);
   stack->ChangedSincePush = true;
   ctx->NewState |= stack->DirtyFlag;
}


/**
 * Replace the current matrix with the identity matrix.
 *
 * \sa glLoadIdentity().
 *
 * Flushes the vertices and calls _math_matrix_set_identity() with the
 * top-most matrix in the current stack.
 * Marks __struct gl_contextRec::NewState with the stack dirty flag.
 */
void GLAPIENTRY
_mesa_LoadIdentity( void )
{
   GET_CURRENT_CONTEXT(ctx);

   if (MESA_VERBOSE & VERBOSE_API)
      _mesa_debug(ctx, "glLoadIdentity()\n");

   _mesa_load_identity_matrix(ctx, ctx->CurrentStack);
}


void GLAPIENTRY
_mesa_MatrixLoadIdentityEXT( GLenum matrixMode )
{
   struct gl_matrix_stack *stack;
   GET_CURRENT_CONTEXT(ctx);
   stack = get_named_matrix_stack(ctx, matrixMode, "glMatrixLoadIdentityEXT");
   if (!stack)
      return;

   _mesa_load_identity_matrix(ctx, stack);
}


void
_mesa_load_matrix(struct gl_context *ctx, struct gl_matrix_stack *stack,
                  const GLfloat *m)
{
   if (memcmp(m, stack->Top->m, 16 * sizeof(GLfloat)) != 0) {
      FLUSH_VERTICES(ctx, 0, 0);
      _math_matrix_loadf(stack->Top, m);
      stack->ChangedSincePush = true;
      ctx->NewState |= stack->DirtyFlag;
   }
}


static void
matrix_load(struct gl_context *ctx, struct gl_matrix_stack *stack,
            const GLfloat *m, const char* caller)
{
   if (!m) return;
   if (MESA_VERBOSE & VERBOSE_API)
      _mesa_debug(ctx,
          "%s(%f %f %f %f, %f %f %f %f, %f %f %f %f, %f %f %f %f\n",
          caller,
          m[0], m[4], m[8], m[12],
          m[1], m[5], m[9], m[13],
          m[2], m[6], m[10], m[14],
          m[3], m[7], m[11], m[15]);

   _mesa_load_matrix(ctx, stack, m);
}


/**
 * Replace the current matrix with a given matrix.
 *
 * \param m matrix.
 *
 * \sa glLoadMatrixf().
 *
 * Flushes the vertices and calls _math_matrix_loadf() with the top-most
 * matrix in the current stack and the given matrix.
 * Marks __struct gl_contextRec::NewState with the dirty stack flag.
 */
void GLAPIENTRY
_mesa_LoadMatrixf( const GLfloat *m )
{
   GET_CURRENT_CONTEXT(ctx);
   matrix_load(ctx, ctx->CurrentStack, m, "glLoadMatrix");
}


/**
 * Replace the named matrix with a given matrix.
 *
 * \param matrixMode matrix to replace
 * \param m matrix
 *
 * \sa glLoadMatrixf().
 */
void GLAPIENTRY
_mesa_MatrixLoadfEXT( GLenum matrixMode, const GLfloat *m )
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_matrix_stack * stack =
      get_named_matrix_stack(ctx, matrixMode, "glMatrixLoadfEXT");
   if (!stack)
      return;

   matrix_load(ctx, stack, m, "glMatrixLoadfEXT");
}


static void
matrix_mult(struct gl_matrix_stack *stack, const GLfloat *m, const char* caller)
{
   GET_CURRENT_CONTEXT(ctx);

   /* glthread filters out identity matrices, so don't do it again. */
   if (!m || (!ctx->GLThread.enabled && _mesa_matrix_is_identity(m)))
      return;

   if (MESA_VERBOSE & VERBOSE_API)
      _mesa_debug(ctx,
          "%s(%f %f %f %f, %f %f %f %f, %f %f %f %f, %f %f %f %f\n",
          caller,
          m[0], m[4], m[8], m[12],
          m[1], m[5], m[9], m[13],
          m[2], m[6], m[10], m[14],
          m[3], m[7], m[11], m[15]);

   FLUSH_VERTICES(ctx, 0, 0);
   _math_matrix_mul_floats(stack->Top, m);
   stack->ChangedSincePush = true;
   ctx->NewState |= stack->DirtyFlag;
}


/**
 * Multiply the current matrix with a given matrix.
 *
 * \param m matrix.
 *
 * \sa glMultMatrixf().
 *
 * Flushes the vertices and calls _math_matrix_mul_floats() with the top-most
 * matrix in the current stack and the given matrix. Marks
 * __struct gl_contextRec::NewState with the dirty stack flag.
 */
void GLAPIENTRY
_mesa_MultMatrixf( const GLfloat *m )
{
   GET_CURRENT_CONTEXT(ctx);
   matrix_mult(ctx->CurrentStack, m, "glMultMatrix");
}


void GLAPIENTRY
_mesa_MatrixMultfEXT( GLenum matrixMode, const GLfloat *m )
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_matrix_stack * stack =
      get_named_matrix_stack(ctx, matrixMode, "glMatrixMultfEXT");
   if (!stack)
      return;

   matrix_mult(stack, m, "glMultMatrix");
}


static void
matrix_rotate(struct gl_matrix_stack *stack, GLfloat angle,
              GLfloat x, GLfloat y, GLfloat z, const char* caller)
{
   GET_CURRENT_CONTEXT(ctx);

   FLUSH_VERTICES(ctx, 0, 0);
   if (angle != 0.0F) {
      _math_matrix_rotate(stack->Top, angle, x, y, z);
      stack->ChangedSincePush = true;
      ctx->NewState |=stack->DirtyFlag;
   }
}


/**
 * Multiply the current matrix with a rotation matrix.
 *
 * \param angle angle of rotation, in degrees.
 * \param x rotation vector x coordinate.
 * \param y rotation vector y coordinate.
 * \param z rotation vector z coordinate.
 *
 * \sa glRotatef().
 *
 * Flushes the vertices and calls _math_matrix_rotate() with the top-most
 * matrix in the current stack and the given parameters. Marks
 * __struct gl_contextRec::NewState with the dirty stack flag.
 */
void GLAPIENTRY
_mesa_Rotatef( GLfloat angle, GLfloat x, GLfloat y, GLfloat z )
{
   GET_CURRENT_CONTEXT(ctx);
   matrix_rotate(ctx->CurrentStack, angle, x, y, z, "glRotatef");
}


void GLAPIENTRY
_mesa_MatrixRotatefEXT( GLenum matrixMode, GLfloat angle, GLfloat x, GLfloat y, GLfloat z )
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_matrix_stack *stack =
      get_named_matrix_stack(ctx, matrixMode, "glMatrixRotatefEXT");
   if (!stack)
      return;

   matrix_rotate(stack, angle, x, y, z, "glMatrixRotatefEXT");
}


/**
 * Multiply the current matrix with a general scaling matrix.
 *
 * \param x x axis scale factor.
 * \param y y axis scale factor.
 * \param z z axis scale factor.
 *
 * \sa glScalef().
 *
 * Flushes the vertices and calls _math_matrix_scale() with the top-most
 * matrix in the current stack and the given parameters. Marks
 * __struct gl_contextRec::NewState with the dirty stack flag.
 */
void GLAPIENTRY
_mesa_Scalef( GLfloat x, GLfloat y, GLfloat z )
{
   GET_CURRENT_CONTEXT(ctx);

   FLUSH_VERTICES(ctx, 0, 0);
   _math_matrix_scale( ctx->CurrentStack->Top, x, y, z);
   ctx->CurrentStack->ChangedSincePush = true;
   ctx->NewState |= ctx->CurrentStack->DirtyFlag;
}


void GLAPIENTRY
_mesa_MatrixScalefEXT( GLenum matrixMode, GLfloat x, GLfloat y, GLfloat z )
{
   struct gl_matrix_stack *stack;
   GET_CURRENT_CONTEXT(ctx);

   stack = get_named_matrix_stack(ctx, matrixMode, "glMatrixScalefEXT");
   if (!stack)
      return;

   FLUSH_VERTICES(ctx, 0, 0);
   _math_matrix_scale(stack->Top, x, y, z);
   stack->ChangedSincePush = true;
   ctx->NewState |= stack->DirtyFlag;
}


/**
 * Multiply the current matrix with a translation matrix.
 *
 * \param x translation vector x coordinate.
 * \param y translation vector y coordinate.
 * \param z translation vector z coordinate.
 *
 * \sa glTranslatef().
 *
 * Flushes the vertices and calls _math_matrix_translate() with the top-most
 * matrix in the current stack and the given parameters. Marks
 * __struct gl_contextRec::NewState with the dirty stack flag.
 */
void GLAPIENTRY
_mesa_Translatef( GLfloat x, GLfloat y, GLfloat z )
{
   GET_CURRENT_CONTEXT(ctx);

   FLUSH_VERTICES(ctx, 0, 0);
   _math_matrix_translate( ctx->CurrentStack->Top, x, y, z);
   ctx->CurrentStack->ChangedSincePush = true;
   ctx->NewState |= ctx->CurrentStack->DirtyFlag;
}


void GLAPIENTRY
_mesa_MatrixTranslatefEXT( GLenum matrixMode, GLfloat x, GLfloat y, GLfloat z )
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_matrix_stack *stack =
      get_named_matrix_stack(ctx, matrixMode, "glMatrixTranslatefEXT");
   if (!stack)
      return;

   FLUSH_VERTICES(ctx, 0, 0);
   _math_matrix_translate(stack->Top, x, y, z);
   stack->ChangedSincePush = true;
   ctx->NewState |= stack->DirtyFlag;
}


void GLAPIENTRY
_mesa_LoadMatrixd( const GLdouble *m )
{
   GLint i;
   GLfloat f[16];
   if (!m) return;
   for (i = 0; i < 16; i++)
      f[i] = (GLfloat) m[i];
   _mesa_LoadMatrixf(f);
}


void GLAPIENTRY
_mesa_MatrixLoaddEXT( GLenum matrixMode, const GLdouble *m )
{
   GLfloat f[16];
   if (!m) return;
   for (unsigned i = 0; i < 16; i++)
      f[i] = (GLfloat) m[i];
   _mesa_MatrixLoadfEXT(matrixMode, f);
}


void GLAPIENTRY
_mesa_MultMatrixd( const GLdouble *m )
{
   GLint i;
   GLfloat f[16];
   if (!m) return;
   for (i = 0; i < 16; i++)
      f[i] = (GLfloat) m[i];
   _mesa_MultMatrixf( f );
}


void GLAPIENTRY
_mesa_MatrixMultdEXT( GLenum matrixMode, const GLdouble *m )
{
   GLfloat f[16];
   if (!m) return;
   for (unsigned i = 0; i < 16; i++)
      f[i] = (GLfloat) m[i];
   _mesa_MatrixMultfEXT(matrixMode, f);
}


void GLAPIENTRY
_mesa_Rotated( GLdouble angle, GLdouble x, GLdouble y, GLdouble z )
{
   _mesa_Rotatef((GLfloat) angle, (GLfloat) x, (GLfloat) y, (GLfloat) z);
}


void GLAPIENTRY
_mesa_MatrixRotatedEXT( GLenum matrixMode, GLdouble angle,
      GLdouble x, GLdouble y, GLdouble z )
{
   _mesa_MatrixRotatefEXT(matrixMode, (GLfloat) angle,
         (GLfloat) x, (GLfloat) y, (GLfloat) z);
}


void GLAPIENTRY
_mesa_Scaled( GLdouble x, GLdouble y, GLdouble z )
{
   _mesa_Scalef((GLfloat) x, (GLfloat) y, (GLfloat) z);
}


void GLAPIENTRY
_mesa_MatrixScaledEXT( GLenum matrixMode, GLdouble x, GLdouble y, GLdouble z )
{
   _mesa_MatrixScalefEXT(matrixMode, (GLfloat) x, (GLfloat) y, (GLfloat) z);
}


void GLAPIENTRY
_mesa_Translated( GLdouble x, GLdouble y, GLdouble z )
{
   _mesa_Translatef((GLfloat) x, (GLfloat) y, (GLfloat) z);
}


void GLAPIENTRY
_mesa_MatrixTranslatedEXT( GLenum matrixMode, GLdouble x, GLdouble y, GLdouble z )
{
   _mesa_MatrixTranslatefEXT(matrixMode, (GLfloat) x, (GLfloat) y, (GLfloat) z);
}


void GLAPIENTRY
_mesa_LoadTransposeMatrixf( const GLfloat *m )
{
   GLfloat tm[16];
   if (!m) return;
   _math_transposef(tm, m);
   _mesa_LoadMatrixf(tm);
}

void GLAPIENTRY
_mesa_MatrixLoadTransposefEXT( GLenum matrixMode, const GLfloat *m )
{
   GLfloat tm[16];
   if (!m) return;
   _math_transposef(tm, m);
   _mesa_MatrixLoadfEXT(matrixMode, tm);
}

void GLAPIENTRY
_mesa_LoadTransposeMatrixd( const GLdouble *m )
{
   GLfloat tm[16];
   if (!m) return;
   _math_transposefd(tm, m);
   _mesa_LoadMatrixf(tm);
}

void GLAPIENTRY
_mesa_MatrixLoadTransposedEXT( GLenum matrixMode, const GLdouble *m )
{
   GLfloat tm[16];
   if (!m) return;
   _math_transposefd(tm, m);
   _mesa_MatrixLoadfEXT(matrixMode, tm);
}

void GLAPIENTRY
_mesa_MultTransposeMatrixf( const GLfloat *m )
{
   GLfloat tm[16];
   if (!m) return;
   _math_transposef(tm, m);
   _mesa_MultMatrixf(tm);
}

void GLAPIENTRY
_mesa_MatrixMultTransposefEXT( GLenum matrixMode, const GLfloat *m )
{
   GLfloat tm[16];
   if (!m) return;
   _math_transposef(tm, m);
   _mesa_MatrixMultfEXT(matrixMode, tm);
}

void GLAPIENTRY
_mesa_MultTransposeMatrixd( const GLdouble *m )
{
   GLfloat tm[16];
   if (!m) return;
   _math_transposefd(tm, m);
   _mesa_MultMatrixf(tm);
}

void GLAPIENTRY
_mesa_MatrixMultTransposedEXT( GLenum matrixMode, const GLdouble *m )
{
   GLfloat tm[16];
   if (!m) return;
   _math_transposefd(tm, m);
   _mesa_MatrixMultfEXT(matrixMode, tm);
}

/**********************************************************************/
/** \name State management */
/*@{*/


/**
 * Update the projection matrix stack.
 *
 * \param ctx GL context.
 *
 * Recomputes user clip positions if necessary.
 *
 * \note This routine references __struct gl_contextRec::Tranform attribute
 * values to compute userclip positions in clip space, but is only called on
 * _NEW_PROJECTION.  The _mesa_ClipPlane() function keeps these values up to
 * date across changes to the __struct gl_contextRec::Transform attributes.
 */
static void
update_projection( struct gl_context *ctx )
{
   /* Recompute clip plane positions in clipspace.  This is also done
    * in _mesa_ClipPlane().
    */
   GLbitfield mask = ctx->Transform.ClipPlanesEnabled;

   if (mask) {
      /* make sure the inverse is up to date */
      _math_matrix_analyse(ctx->ProjectionMatrixStack.Top);

      do {
         const int p = u_bit_scan(&mask);

         _mesa_transform_vector(ctx->Transform._ClipUserPlane[p],
                                ctx->Transform.EyeUserPlane[p],
                                ctx->ProjectionMatrixStack.Top->inv);
      } while (mask);
   }
}


/**
 * Updates the combined modelview-projection matrix.
 *
 * \param ctx GL context.
 * \param new_state new state bit mask.
 *
 * If there is a new model view matrix then analyzes it. If there is a new
 * projection matrix, updates it. Finally calls
 * calculate_model_project_matrix() to recalculate the modelview-projection
 * matrix.
 */
void _mesa_update_modelview_project( struct gl_context *ctx, GLuint new_state )
{
   if (new_state & _NEW_MODELVIEW)
      _math_matrix_analyse( ctx->ModelviewMatrixStack.Top );

   if (new_state & _NEW_PROJECTION)
      update_projection( ctx );

   /* Calculate ModelViewMatrix * ProjectionMatrix. */
   _math_matrix_mul_matrix(&ctx->_ModelProjectMatrix,
                           ctx->ProjectionMatrixStack.Top,
                           ctx->ModelviewMatrixStack.Top);
}

/*@}*/


/**********************************************************************/
/** Matrix stack initialization */
/*@{*/


/**
 * Initialize a matrix stack.
 *
 * \param stack matrix stack.
 * \param maxDepth maximum stack depth.
 * \param dirtyFlag dirty flag.
 *
 * Allocates an array of \p maxDepth elements for the matrix stack and calls
 * _math_matrix_ctr() for each element to initialize it.
 */
static void
init_matrix_stack(struct gl_matrix_stack *stack,
                  GLuint maxDepth, GLuint dirtyFlag)
{
   stack->Depth = 0;
   stack->MaxDepth = maxDepth;
   stack->DirtyFlag = dirtyFlag;
   /* The stack will be dynamically resized at glPushMatrix() time */
   stack->Stack = calloc(1, sizeof(GLmatrix));
   stack->StackSize = 1;
   _math_matrix_ctr(&stack->Stack[0]);
   stack->Top = stack->Stack;
   stack->ChangedSincePush = false;
}

/**
 * Free matrix stack.
 *
 * \param stack matrix stack.
 */
static void
free_matrix_stack( struct gl_matrix_stack *stack )
{
   free(stack->Stack);
   stack->Stack = stack->Top = NULL;
   stack->StackSize = 0;
}

/*@}*/


/**********************************************************************/
/** \name Initialization */
/*@{*/


/**
 * Initialize the context matrix data.
 *
 * \param ctx GL context.
 *
 * Initializes each of the matrix stacks and the combined modelview-projection
 * matrix.
 */
void _mesa_init_matrix( struct gl_context * ctx )
{
   GLuint i;

   /* Initialize matrix stacks */
   init_matrix_stack(&ctx->ModelviewMatrixStack, MAX_MODELVIEW_STACK_DEPTH,
                     _NEW_MODELVIEW);
   init_matrix_stack(&ctx->ProjectionMatrixStack, MAX_PROJECTION_STACK_DEPTH,
                     _NEW_PROJECTION);
   for (i = 0; i < ARRAY_SIZE(ctx->TextureMatrixStack); i++)
      init_matrix_stack(&ctx->TextureMatrixStack[i], MAX_TEXTURE_STACK_DEPTH,
                        _NEW_TEXTURE_MATRIX);
   for (i = 0; i < ARRAY_SIZE(ctx->ProgramMatrixStack); i++)
      init_matrix_stack(&ctx->ProgramMatrixStack[i],
		        MAX_PROGRAM_MATRIX_STACK_DEPTH, _NEW_TRACK_MATRIX);
   ctx->CurrentStack = &ctx->ModelviewMatrixStack;

   /* Init combined Modelview*Projection matrix */
   _math_matrix_ctr( &ctx->_ModelProjectMatrix );
}


/**
 * Free the context matrix data.
 *
 * \param ctx GL context.
 *
 * Frees each of the matrix stacks.
 */
void _mesa_free_matrix_data( struct gl_context *ctx )
{
   GLuint i;

   free_matrix_stack(&ctx->ModelviewMatrixStack);
   free_matrix_stack(&ctx->ProjectionMatrixStack);
   for (i = 0; i < ARRAY_SIZE(ctx->TextureMatrixStack); i++)
      free_matrix_stack(&ctx->TextureMatrixStack[i]);
   for (i = 0; i < ARRAY_SIZE(ctx->ProgramMatrixStack); i++)
      free_matrix_stack(&ctx->ProgramMatrixStack[i]);

}


/**
 * Initialize the context transform attribute group.
 *
 * \param ctx GL context.
 *
 * \todo Move this to a new file with other 'transform' routines.
 */
void _mesa_init_transform( struct gl_context *ctx )
{
   GLuint i;

   /* Transformation group */
   ctx->Transform.MatrixMode = GL_MODELVIEW;
   ctx->Transform.Normalize = GL_FALSE;
   ctx->Transform.RescaleNormals = GL_FALSE;
   ctx->Transform.RasterPositionUnclipped = GL_FALSE;
   for (i=0;i<ctx->Const.MaxClipPlanes;i++) {
      ASSIGN_4V( ctx->Transform.EyeUserPlane[i], 0.0, 0.0, 0.0, 0.0 );
   }
   ctx->Transform.ClipPlanesEnabled = 0;
}


/*@}*/
