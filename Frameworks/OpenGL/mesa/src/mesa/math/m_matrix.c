/*
 * Mesa 3-D graphics library
 *
 * Copyright (C) 1999-2005  Brian Paul   All Rights Reserved.
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
 * \file m_matrix.c
 * Matrix operations.
 *
 * \note
 * -# 4x4 transformation matrices are stored in memory in column major order.
 * -# Points/vertices are to be thought of as column vectors.
 * -# Transformation of a point p by a matrix M is: p' = M * p
 */

#include <stddef.h>
#include <math.h>

#include "main/errors.h"
#include "util/glheader.h"
#include "main/macros.h"

#include "m_matrix.h"

#include "util/u_memory.h"


/**
 * \defgroup MatFlags MAT_FLAG_XXX-flags
 *
 * Bitmasks to indicate different kinds of 4x4 matrices in GLmatrix::flags
 */
/*@{*/
#define MAT_FLAG_IDENTITY       0     /**< is an identity matrix flag.
                                       *   (Not actually used - the identity
                                       *   matrix is identified by the absence
                                       *   of all other flags.)
                                       */
#define MAT_FLAG_GENERAL        0x1   /**< is a general matrix flag */
#define MAT_FLAG_ROTATION       0x2   /**< is a rotation matrix flag */
#define MAT_FLAG_TRANSLATION    0x4   /**< is a translation matrix flag */
#define MAT_FLAG_UNIFORM_SCALE  0x8   /**< is an uniform scaling matrix flag */
#define MAT_FLAG_GENERAL_SCALE  0x10  /**< is a general scaling matrix flag */
#define MAT_FLAG_GENERAL_3D     0x20  /**< general 3D matrix flag */
#define MAT_FLAG_PERSPECTIVE    0x40  /**< is a perspective proj matrix flag */
#define MAT_FLAG_SINGULAR       0x80  /**< is a singular matrix flag */
#define MAT_DIRTY_TYPE          0x100  /**< matrix type is dirty */
#define MAT_DIRTY_FLAGS         0x200  /**< matrix flags are dirty */
#define MAT_DIRTY_INVERSE       0x400  /**< matrix inverse is dirty */

/** angle preserving matrix flags mask */
#define MAT_FLAGS_ANGLE_PRESERVING (MAT_FLAG_ROTATION | \
                                    MAT_FLAG_TRANSLATION | \
                                    MAT_FLAG_UNIFORM_SCALE)

/** geometry related matrix flags mask */
#define MAT_FLAGS_GEOMETRY (MAT_FLAG_GENERAL | \
                            MAT_FLAG_ROTATION | \
                            MAT_FLAG_TRANSLATION | \
                            MAT_FLAG_UNIFORM_SCALE | \
                            MAT_FLAG_GENERAL_SCALE | \
                            MAT_FLAG_GENERAL_3D | \
                            MAT_FLAG_PERSPECTIVE | \
                            MAT_FLAG_SINGULAR)

/** length preserving matrix flags mask */
#define MAT_FLAGS_LENGTH_PRESERVING (MAT_FLAG_ROTATION | \
                                     MAT_FLAG_TRANSLATION)


/** 3D (non-perspective) matrix flags mask */
#define MAT_FLAGS_3D (MAT_FLAG_ROTATION | \
                      MAT_FLAG_TRANSLATION | \
                      MAT_FLAG_UNIFORM_SCALE | \
                      MAT_FLAG_GENERAL_SCALE | \
                      MAT_FLAG_GENERAL_3D)

/** dirty matrix flags mask */
#define MAT_DIRTY          (MAT_DIRTY_TYPE | \
                            MAT_DIRTY_FLAGS | \
                            MAT_DIRTY_INVERSE)

/*@}*/


/**
 * Test geometry related matrix flags.
 *
 * \param mat a pointer to a GLmatrix structure.
 * \param a flags mask.
 *
 * \returns non-zero if all geometry related matrix flags are contained within
 * the mask, or zero otherwise.
 */
#define TEST_MAT_FLAGS(mat, a)  \
    ((MAT_FLAGS_GEOMETRY & (~(a)) & ((mat)->flags) ) == 0)


/**********************************************************************/
/** \name Matrix multiplication */
/*@{*/

#define A(row,col)  a[(col<<2)+row]
#define B(row,col)  b[(col<<2)+row]
#define P(row,col)  product[(col<<2)+row]

/**
 * Perform a full 4x4 matrix multiplication.
 *
 * \param a matrix.
 * \param b matrix.
 * \param product will receive the product of \p a and \p b.
 *
 * \warning Is assumed that \p product != \p b. \p product == \p a is allowed.
 *
 * \note KW: 4*16 = 64 multiplications
 *
 * \author This \c matmul was contributed by Thomas Malik
 */
static void matmul4( GLfloat *product, const GLfloat *a, const GLfloat *b )
{
   GLint i;
   for (i = 0; i < 4; i++) {
      const GLfloat ai0=A(i,0),  ai1=A(i,1),  ai2=A(i,2),  ai3=A(i,3);
      P(i,0) = ai0 * B(0,0) + ai1 * B(1,0) + ai2 * B(2,0) + ai3 * B(3,0);
      P(i,1) = ai0 * B(0,1) + ai1 * B(1,1) + ai2 * B(2,1) + ai3 * B(3,1);
      P(i,2) = ai0 * B(0,2) + ai1 * B(1,2) + ai2 * B(2,2) + ai3 * B(3,2);
      P(i,3) = ai0 * B(0,3) + ai1 * B(1,3) + ai2 * B(2,3) + ai3 * B(3,3);
   }
}

/**
 * Multiply two matrices known to occupy only the top three rows, such
 * as typical model matrices, and orthogonal matrices.
 *
 * \param a matrix.
 * \param b matrix.
 * \param product will receive the product of \p a and \p b.
 */
static void matmul34( GLfloat *product, const GLfloat *a, const GLfloat *b )
{
   GLint i;
   for (i = 0; i < 3; i++) {
      const GLfloat ai0=A(i,0),  ai1=A(i,1),  ai2=A(i,2),  ai3=A(i,3);
      P(i,0) = ai0 * B(0,0) + ai1 * B(1,0) + ai2 * B(2,0);
      P(i,1) = ai0 * B(0,1) + ai1 * B(1,1) + ai2 * B(2,1);
      P(i,2) = ai0 * B(0,2) + ai1 * B(1,2) + ai2 * B(2,2);
      P(i,3) = ai0 * B(0,3) + ai1 * B(1,3) + ai2 * B(2,3) + ai3;
   }
   P(3,0) = 0;
   P(3,1) = 0;
   P(3,2) = 0;
   P(3,3) = 1;
}

#undef A
#undef B
#undef P

/* "m" must be a 4x4 matrix. Set it to the identity matrix. */
static void
matrix_set_identity(GLfloat *m)
{
   m[0] = m[5] = m[10] = m[15] = 1;
   m[1] = m[2] = m[3] = m[4] = m[6] = m[7] = 0;
   m[8] = m[9] = m[11] = m[12] = m[13] = m[14] = 0;
}

/**
 * Multiply a matrix by an array of floats with known properties.
 *
 * \param mat pointer to a GLmatrix structure containing the left multiplication
 * matrix, and that will receive the product result.
 * \param m right multiplication matrix array.
 * \param flags flags of the matrix \p m.
 *
 * Joins both flags and marks the type and inverse as dirty.  Calls matmul34()
 * if both matrices are 3D, or matmul4() otherwise.
 */
static void matrix_multf( GLmatrix *mat, const GLfloat *m, GLuint flags )
{
   mat->flags |= (flags | MAT_DIRTY_TYPE | MAT_DIRTY_INVERSE);

   if (TEST_MAT_FLAGS(mat, MAT_FLAGS_3D))
      matmul34( mat->m, mat->m, m );
   else
      matmul4( mat->m, mat->m, m );
}

/**
 * Matrix multiplication.
 *
 * \param dest destination matrix.
 * \param a left matrix.
 * \param b right matrix.
 *
 * Joins both flags and marks the type and inverse as dirty.  Calls matmul34()
 * if both matrices are 3D, or matmul4() otherwise.
 */
void
_math_matrix_mul_matrix( GLmatrix *dest, const GLmatrix *a, const GLmatrix *b )
{
   dest->flags = (a->flags |
                  b->flags |
                  MAT_DIRTY_TYPE |
                  MAT_DIRTY_INVERSE);

   if (TEST_MAT_FLAGS(dest, MAT_FLAGS_3D))
      matmul34( dest->m, a->m, b->m );
   else
      matmul4( dest->m, a->m, b->m );
}

/**
 * Matrix multiplication.
 *
 * \param dest left and destination matrix.
 * \param m right matrix array.
 *
 * Marks the matrix flags with general flag, and type and inverse dirty flags.
 * Calls matmul4() for the multiplication.
 */
void
_math_matrix_mul_floats( GLmatrix *dest, const GLfloat *m )
{
   dest->flags |= (MAT_FLAG_GENERAL |
                   MAT_DIRTY_TYPE |
                   MAT_DIRTY_INVERSE |
                   MAT_DIRTY_FLAGS);

   matmul4( dest->m, dest->m, m );
}

/*@}*/

/**
 * References an element of 4x4 matrix.
 *
 * \param m matrix array.
 * \param c column of the desired element.
 * \param r row of the desired element.
 *
 * \return value of the desired element.
 *
 * Calculate the linear storage index of the element and references it.
 */
#define MAT(m,r,c) (m)[(c)*4+(r)]


/**********************************************************************/
/** \name Matrix inversion */
/*@{*/

/**
 * Compute inverse of 4x4 transformation matrix.
 *
 * \param mat pointer to a GLmatrix structure. The matrix inverse will be
 * stored in the GLmatrix::inv attribute.
 *
 * \return GL_TRUE for success, GL_FALSE for failure (\p singular matrix).
 *
 * \author
 * Code contributed by Jacques Leroy jle@star.be
 *
 * Calculates the inverse matrix by performing the gaussian matrix reduction
 * with partial pivoting followed by back/substitution with the loops manually
 * unrolled.
 */
static GLboolean invert_matrix_general( GLmatrix *mat )
{
   return util_invert_mat4x4(mat->inv, mat->m);
}

/**
 * Compute inverse of a general 3d transformation matrix.
 *
 * \param mat pointer to a GLmatrix structure. The matrix inverse will be
 * stored in the GLmatrix::inv attribute.
 *
 * \return GL_TRUE for success, GL_FALSE for failure (\p singular matrix).
 *
 * \author Adapted from graphics gems II.
 *
 * Calculates the inverse of the upper left by first calculating its
 * determinant and multiplying it to the symmetric adjust matrix of each
 * element. Finally deals with the translation part by transforming the
 * original translation vector using by the calculated submatrix inverse.
 */
static GLboolean invert_matrix_3d_general( GLmatrix *mat )
{
   const GLfloat *in = mat->m;
   GLfloat *out = mat->inv;
   GLfloat pos, neg, t;
   GLfloat det;

   /* Calculate the determinant of upper left 3x3 submatrix and
    * determine if the matrix is singular.
    */
   pos = neg = 0.0;
   t =  MAT(in,0,0) * MAT(in,1,1) * MAT(in,2,2);
   if (t >= 0.0F) pos += t; else neg += t;

   t =  MAT(in,1,0) * MAT(in,2,1) * MAT(in,0,2);
   if (t >= 0.0F) pos += t; else neg += t;

   t =  MAT(in,2,0) * MAT(in,0,1) * MAT(in,1,2);
   if (t >= 0.0F) pos += t; else neg += t;

   t = -MAT(in,2,0) * MAT(in,1,1) * MAT(in,0,2);
   if (t >= 0.0F) pos += t; else neg += t;

   t = -MAT(in,1,0) * MAT(in,0,1) * MAT(in,2,2);
   if (t >= 0.0F) pos += t; else neg += t;

   t = -MAT(in,0,0) * MAT(in,2,1) * MAT(in,1,2);
   if (t >= 0.0F) pos += t; else neg += t;

   det = pos + neg;

   if (fabsf(det) < 1e-25F)
      return GL_FALSE;

   det = 1.0F / det;
   MAT(out,0,0) = (  (MAT(in,1,1)*MAT(in,2,2) - MAT(in,2,1)*MAT(in,1,2) )*det);
   MAT(out,0,1) = (- (MAT(in,0,1)*MAT(in,2,2) - MAT(in,2,1)*MAT(in,0,2) )*det);
   MAT(out,0,2) = (  (MAT(in,0,1)*MAT(in,1,2) - MAT(in,1,1)*MAT(in,0,2) )*det);
   MAT(out,1,0) = (- (MAT(in,1,0)*MAT(in,2,2) - MAT(in,2,0)*MAT(in,1,2) )*det);
   MAT(out,1,1) = (  (MAT(in,0,0)*MAT(in,2,2) - MAT(in,2,0)*MAT(in,0,2) )*det);
   MAT(out,1,2) = (- (MAT(in,0,0)*MAT(in,1,2) - MAT(in,1,0)*MAT(in,0,2) )*det);
   MAT(out,2,0) = (  (MAT(in,1,0)*MAT(in,2,1) - MAT(in,2,0)*MAT(in,1,1) )*det);
   MAT(out,2,1) = (- (MAT(in,0,0)*MAT(in,2,1) - MAT(in,2,0)*MAT(in,0,1) )*det);
   MAT(out,2,2) = (  (MAT(in,0,0)*MAT(in,1,1) - MAT(in,1,0)*MAT(in,0,1) )*det);

   /* Do the translation part */
   MAT(out,0,3) = - (MAT(in,0,3) * MAT(out,0,0) +
                     MAT(in,1,3) * MAT(out,0,1) +
                     MAT(in,2,3) * MAT(out,0,2) );
   MAT(out,1,3) = - (MAT(in,0,3) * MAT(out,1,0) +
                     MAT(in,1,3) * MAT(out,1,1) +
                     MAT(in,2,3) * MAT(out,1,2) );
   MAT(out,2,3) = - (MAT(in,0,3) * MAT(out,2,0) +
                     MAT(in,1,3) * MAT(out,2,1) +
                     MAT(in,2,3) * MAT(out,2,2) );

   return GL_TRUE;
}

/**
 * Compute inverse of a 3d transformation matrix.
 *
 * \param mat pointer to a GLmatrix structure. The matrix inverse will be
 * stored in the GLmatrix::inv attribute.
 *
 * \return GL_TRUE for success, GL_FALSE for failure (\p singular matrix).
 *
 * If the matrix is not an angle preserving matrix then calls
 * invert_matrix_3d_general for the actual calculation. Otherwise calculates
 * the inverse matrix analyzing and inverting each of the scaling, rotation and
 * translation parts.
 */
static GLboolean invert_matrix_3d( GLmatrix *mat )
{
   const GLfloat *in = mat->m;
   GLfloat *out = mat->inv;

   if (!TEST_MAT_FLAGS(mat, MAT_FLAGS_ANGLE_PRESERVING)) {
      return invert_matrix_3d_general( mat );
   }

   if (mat->flags & MAT_FLAG_UNIFORM_SCALE) {
      GLfloat scale = (MAT(in,0,0) * MAT(in,0,0) +
                       MAT(in,0,1) * MAT(in,0,1) +
                       MAT(in,0,2) * MAT(in,0,2));

      if (scale == 0.0F)
         return GL_FALSE;

      scale = 1.0F / scale;

      /* Transpose and scale the 3 by 3 upper-left submatrix. */
      MAT(out,0,0) = scale * MAT(in,0,0);
      MAT(out,1,0) = scale * MAT(in,0,1);
      MAT(out,2,0) = scale * MAT(in,0,2);
      MAT(out,0,1) = scale * MAT(in,1,0);
      MAT(out,1,1) = scale * MAT(in,1,1);
      MAT(out,2,1) = scale * MAT(in,1,2);
      MAT(out,0,2) = scale * MAT(in,2,0);
      MAT(out,1,2) = scale * MAT(in,2,1);
      MAT(out,2,2) = scale * MAT(in,2,2);
   }
   else if (mat->flags & MAT_FLAG_ROTATION) {
      /* Transpose the 3 by 3 upper-left submatrix. */
      MAT(out,0,0) = MAT(in,0,0);
      MAT(out,1,0) = MAT(in,0,1);
      MAT(out,2,0) = MAT(in,0,2);
      MAT(out,0,1) = MAT(in,1,0);
      MAT(out,1,1) = MAT(in,1,1);
      MAT(out,2,1) = MAT(in,1,2);
      MAT(out,0,2) = MAT(in,2,0);
      MAT(out,1,2) = MAT(in,2,1);
      MAT(out,2,2) = MAT(in,2,2);
   }
   else {
      /* pure translation */
      matrix_set_identity(out);
      MAT(out,0,3) = - MAT(in,0,3);
      MAT(out,1,3) = - MAT(in,1,3);
      MAT(out,2,3) = - MAT(in,2,3);
      return GL_TRUE;
   }

   if (mat->flags & MAT_FLAG_TRANSLATION) {
      /* Do the translation part */
      MAT(out,0,3) = - (MAT(in,0,3) * MAT(out,0,0) +
                        MAT(in,1,3) * MAT(out,0,1) +
                        MAT(in,2,3) * MAT(out,0,2) );
      MAT(out,1,3) = - (MAT(in,0,3) * MAT(out,1,0) +
                        MAT(in,1,3) * MAT(out,1,1) +
                        MAT(in,2,3) * MAT(out,1,2) );
      MAT(out,2,3) = - (MAT(in,0,3) * MAT(out,2,0) +
                        MAT(in,1,3) * MAT(out,2,1) +
                        MAT(in,2,3) * MAT(out,2,2) );
   }
   else {
      MAT(out,0,3) = MAT(out,1,3) = MAT(out,2,3) = 0.0;
   }

   return GL_TRUE;
}

/**
 * Compute inverse of an identity transformation matrix.
 *
 * \param mat pointer to a GLmatrix structure. The matrix inverse will be
 * stored in the GLmatrix::inv attribute.
 *
 * \return always GL_TRUE.
 *
 * Simply copies Identity into GLmatrix::inv.
 */
static GLboolean invert_matrix_identity( GLmatrix *mat )
{
   matrix_set_identity(mat->inv);
   return GL_TRUE;
}

/**
 * Compute inverse of a no-rotation 3d transformation matrix.
 *
 * \param mat pointer to a GLmatrix structure. The matrix inverse will be
 * stored in the GLmatrix::inv attribute.
 *
 * \return GL_TRUE for success, GL_FALSE for failure (\p singular matrix).
 *
 * Calculates the
 */
static GLboolean invert_matrix_3d_no_rot( GLmatrix *mat )
{
   const GLfloat *in = mat->m;
   GLfloat *out = mat->inv;

   if (MAT(in,0,0) == 0 || MAT(in,1,1) == 0 || MAT(in,2,2) == 0 )
      return GL_FALSE;

   matrix_set_identity(out);
   MAT(out,0,0) = 1.0F / MAT(in,0,0);
   MAT(out,1,1) = 1.0F / MAT(in,1,1);
   MAT(out,2,2) = 1.0F / MAT(in,2,2);

   if (mat->flags & MAT_FLAG_TRANSLATION) {
      MAT(out,0,3) = - (MAT(in,0,3) * MAT(out,0,0));
      MAT(out,1,3) = - (MAT(in,1,3) * MAT(out,1,1));
      MAT(out,2,3) = - (MAT(in,2,3) * MAT(out,2,2));
   }

   return GL_TRUE;
}

/**
 * Compute inverse of a no-rotation 2d transformation matrix.
 *
 * \param mat pointer to a GLmatrix structure. The matrix inverse will be
 * stored in the GLmatrix::inv attribute.
 *
 * \return GL_TRUE for success, GL_FALSE for failure (\p singular matrix).
 *
 * Calculates the inverse matrix by applying the inverse scaling and
 * translation to the identity matrix.
 */
static GLboolean invert_matrix_2d_no_rot( GLmatrix *mat )
{
   const GLfloat *in = mat->m;
   GLfloat *out = mat->inv;

   if (MAT(in,0,0) == 0 || MAT(in,1,1) == 0)
      return GL_FALSE;

   matrix_set_identity(out);
   MAT(out,0,0) = 1.0F / MAT(in,0,0);
   MAT(out,1,1) = 1.0F / MAT(in,1,1);

   if (mat->flags & MAT_FLAG_TRANSLATION) {
      MAT(out,0,3) = - (MAT(in,0,3) * MAT(out,0,0));
      MAT(out,1,3) = - (MAT(in,1,3) * MAT(out,1,1));
   }

   return GL_TRUE;
}

/**
 * Matrix inversion function pointer type.
 */
typedef GLboolean (*inv_mat_func)( GLmatrix *mat );

/**
 * Table of the matrix inversion functions according to the matrix type.
 */
static inv_mat_func inv_mat_tab[7] = {
   invert_matrix_general,
   invert_matrix_identity,
   invert_matrix_3d_no_rot,
   invert_matrix_general,
   invert_matrix_3d,        /* lazy! */
   invert_matrix_2d_no_rot,
   invert_matrix_3d
};

/**
 * Compute inverse of a transformation matrix.
 *
 * \param mat pointer to a GLmatrix structure. The matrix inverse will be
 * stored in the GLmatrix::inv attribute.
 *
 * \return GL_TRUE for success, GL_FALSE for failure (\p singular matrix).
 *
 * Calls the matrix inversion function in inv_mat_tab corresponding to the
 * given matrix type.  In case of failure, updates the MAT_FLAG_SINGULAR flag,
 * and copies the identity matrix into GLmatrix::inv.
 */
static GLboolean matrix_invert( GLmatrix *mat )
{
   if (inv_mat_tab[mat->type](mat)) {
      mat->flags &= ~MAT_FLAG_SINGULAR;
      return GL_TRUE;
   } else {
      mat->flags |= MAT_FLAG_SINGULAR;
      matrix_set_identity(mat->inv);
      return GL_FALSE;
   }
}

/*@}*/


/**********************************************************************/
/** \name Matrix generation */
/*@{*/

/**
 * Generate a 4x4 transformation matrix from glRotate parameters, and
 * post-multiply the input matrix by it.
 *
 * \author
 * This function was contributed by Erich Boleyn (erich@uruk.org).
 * Optimizations contributed by Rudolf Opalla (rudi@khm.de).
 */
void
_math_matrix_rotate( GLmatrix *mat,
                     GLfloat angle, GLfloat x, GLfloat y, GLfloat z )
{
   GLfloat xx, yy, zz, xy, yz, zx, xs, ys, zs, one_c, s, c;
   GLfloat m[16];
   GLboolean optimized;

   s = sinf( angle * M_PI / 180.0 );
   c = cosf( angle * M_PI / 180.0 );

   matrix_set_identity(m);
   optimized = GL_FALSE;

#define M(row,col)  m[col*4+row]

   if (x == 0.0F) {
      if (y == 0.0F) {
         if (z != 0.0F) {
            optimized = GL_TRUE;
            /* rotate only around z-axis */
            M(0,0) = c;
            M(1,1) = c;
            if (z < 0.0F) {
               M(0,1) = s;
               M(1,0) = -s;
            }
            else {
               M(0,1) = -s;
               M(1,0) = s;
            }
         }
      }
      else if (z == 0.0F) {
         optimized = GL_TRUE;
         /* rotate only around y-axis */
         M(0,0) = c;
         M(2,2) = c;
         if (y < 0.0F) {
            M(0,2) = -s;
            M(2,0) = s;
         }
         else {
            M(0,2) = s;
            M(2,0) = -s;
         }
      }
   }
   else if (y == 0.0F) {
      if (z == 0.0F) {
         optimized = GL_TRUE;
         /* rotate only around x-axis */
         M(1,1) = c;
         M(2,2) = c;
         if (x < 0.0F) {
            M(1,2) = s;
            M(2,1) = -s;
         }
         else {
            M(1,2) = -s;
            M(2,1) = s;
         }
      }
   }

   if (!optimized) {
      const GLfloat mag = sqrtf(x * x + y * y + z * z);

      if (mag <= 1.0e-4F) {
         /* no rotation, leave mat as-is */
         return;
      }

      x /= mag;
      y /= mag;
      z /= mag;


      /*
       *     Arbitrary axis rotation matrix.
       *
       *  This is composed of 5 matrices, Rz, Ry, T, Ry', Rz', multiplied
       *  like so:  Rz * Ry * T * Ry' * Rz'.  T is the final rotation
       *  (which is about the X-axis), and the two composite transforms
       *  Ry' * Rz' and Rz * Ry are (respectively) the rotations necessary
       *  from the arbitrary axis to the X-axis then back.  They are
       *  all elementary rotations.
       *
       *  Rz' is a rotation about the Z-axis, to bring the axis vector
       *  into the x-z plane.  Then Ry' is applied, rotating about the
       *  Y-axis to bring the axis vector parallel with the X-axis.  The
       *  rotation about the X-axis is then performed.  Ry and Rz are
       *  simply the respective inverse transforms to bring the arbitrary
       *  axis back to its original orientation.  The first transforms
       *  Rz' and Ry' are considered inverses, since the data from the
       *  arbitrary axis gives you info on how to get to it, not how
       *  to get away from it, and an inverse must be applied.
       *
       *  The basic calculation used is to recognize that the arbitrary
       *  axis vector (x, y, z), since it is of unit length, actually
       *  represents the sines and cosines of the angles to rotate the
       *  X-axis to the same orientation, with theta being the angle about
       *  Z and phi the angle about Y (in the order described above)
       *  as follows:
       *
       *  cos ( theta ) = x / sqrt ( 1 - z^2 )
       *  sin ( theta ) = y / sqrt ( 1 - z^2 )
       *
       *  cos ( phi ) = sqrt ( 1 - z^2 )
       *  sin ( phi ) = z
       *
       *  Note that cos ( phi ) can further be inserted to the above
       *  formulas:
       *
       *  cos ( theta ) = x / cos ( phi )
       *  sin ( theta ) = y / sin ( phi )
       *
       *  ...etc.  Because of those relations and the standard trigonometric
       *  relations, it is pssible to reduce the transforms down to what
       *  is used below.  It may be that any primary axis chosen will give the
       *  same results (modulo a sign convention) using thie method.
       *
       *  Particularly nice is to notice that all divisions that might
       *  have caused trouble when parallel to certain planes or
       *  axis go away with care paid to reducing the expressions.
       *  After checking, it does perform correctly under all cases, since
       *  in all the cases of division where the denominator would have
       *  been zero, the numerator would have been zero as well, giving
       *  the expected result.
       */

      xx = x * x;
      yy = y * y;
      zz = z * z;
      xy = x * y;
      yz = y * z;
      zx = z * x;
      xs = x * s;
      ys = y * s;
      zs = z * s;
      one_c = 1.0F - c;

      /* We already hold the identity-matrix so we can skip some statements */
      M(0,0) = (one_c * xx) + c;
      M(0,1) = (one_c * xy) - zs;
      M(0,2) = (one_c * zx) + ys;
/*    M(0,3) = 0.0F; */

      M(1,0) = (one_c * xy) + zs;
      M(1,1) = (one_c * yy) + c;
      M(1,2) = (one_c * yz) - xs;
/*    M(1,3) = 0.0F; */

      M(2,0) = (one_c * zx) - ys;
      M(2,1) = (one_c * yz) + xs;
      M(2,2) = (one_c * zz) + c;
/*    M(2,3) = 0.0F; */

/*
      M(3,0) = 0.0F;
      M(3,1) = 0.0F;
      M(3,2) = 0.0F;
      M(3,3) = 1.0F;
*/
   }
#undef M

   matrix_multf( mat, m, MAT_FLAG_ROTATION );
}

/**
 * Apply a perspective projection matrix.
 *
 * \param mat matrix to apply the projection.
 * \param left left clipping plane coordinate.
 * \param right right clipping plane coordinate.
 * \param bottom bottom clipping plane coordinate.
 * \param top top clipping plane coordinate.
 * \param nearval distance to the near clipping plane.
 * \param farval distance to the far clipping plane.
 *
 * Creates the projection matrix and multiplies it with \p mat, marking the
 * MAT_FLAG_PERSPECTIVE flag.
 */
void
_math_matrix_frustum( GLmatrix *mat,
                      GLfloat left, GLfloat right,
                      GLfloat bottom, GLfloat top,
                      GLfloat nearval, GLfloat farval )
{
   GLfloat x, y, a, b, c, d;
   GLfloat m[16];

   x = (2.0F*nearval) / (right-left);
   y = (2.0F*nearval) / (top-bottom);
   a = (right+left) / (right-left);
   b = (top+bottom) / (top-bottom);
   c = -(farval+nearval) / ( farval-nearval);
   d = -(2.0F*farval*nearval) / (farval-nearval);  /* error? */

#define M(row,col)  m[col*4+row]
   M(0,0) = x;     M(0,1) = 0.0F;  M(0,2) = a;      M(0,3) = 0.0F;
   M(1,0) = 0.0F;  M(1,1) = y;     M(1,2) = b;      M(1,3) = 0.0F;
   M(2,0) = 0.0F;  M(2,1) = 0.0F;  M(2,2) = c;      M(2,3) = d;
   M(3,0) = 0.0F;  M(3,1) = 0.0F;  M(3,2) = -1.0F;  M(3,3) = 0.0F;
#undef M

   matrix_multf( mat, m, MAT_FLAG_PERSPECTIVE );
}

/**
 * Create an orthographic projection matrix.
 *
 * \param m float array in which to store the project matrix
 * \param left left clipping plane coordinate.
 * \param right right clipping plane coordinate.
 * \param bottom bottom clipping plane coordinate.
 * \param top top clipping plane coordinate.
 * \param nearval distance to the near clipping plane.
 * \param farval distance to the far clipping plane.
 *
 * Creates the projection matrix and stored the values in \p m.  As with other
 * OpenGL matrices, the data is stored in column-major ordering.
 */
void
_math_float_ortho(float *m,
                  float left, float right,
                  float bottom, float top,
                  float nearval, float farval)
{
#define M(row,col)  m[col*4+row]
   M(0,0) = 2.0F / (right-left);
   M(0,1) = 0.0F;
   M(0,2) = 0.0F;
   M(0,3) = -(right+left) / (right-left);

   M(1,0) = 0.0F;
   M(1,1) = 2.0F / (top-bottom);
   M(1,2) = 0.0F;
   M(1,3) = -(top+bottom) / (top-bottom);

   M(2,0) = 0.0F;
   M(2,1) = 0.0F;
   M(2,2) = -2.0F / (farval-nearval);
   M(2,3) = -(farval+nearval) / (farval-nearval);

   M(3,0) = 0.0F;
   M(3,1) = 0.0F;
   M(3,2) = 0.0F;
   M(3,3) = 1.0F;
#undef M
}

/**
 * Apply an orthographic projection matrix.
 *
 * \param mat matrix to apply the projection.
 * \param left left clipping plane coordinate.
 * \param right right clipping plane coordinate.
 * \param bottom bottom clipping plane coordinate.
 * \param top top clipping plane coordinate.
 * \param nearval distance to the near clipping plane.
 * \param farval distance to the far clipping plane.
 *
 * Creates the projection matrix and multiplies it with \p mat, marking the
 * MAT_FLAG_GENERAL_SCALE and MAT_FLAG_TRANSLATION flags.
 */
void
_math_matrix_ortho( GLmatrix *mat,
                    GLfloat left, GLfloat right,
                    GLfloat bottom, GLfloat top,
                    GLfloat nearval, GLfloat farval )
{
   GLfloat m[16];

   _math_float_ortho(m, left, right, bottom, top, nearval, farval);
   matrix_multf( mat, m, (MAT_FLAG_GENERAL_SCALE|MAT_FLAG_TRANSLATION));
}

/**
 * Multiply a matrix with a general scaling matrix.
 *
 * \param mat matrix.
 * \param x x axis scale factor.
 * \param y y axis scale factor.
 * \param z z axis scale factor.
 *
 * Multiplies in-place the elements of \p mat by the scale factors. Checks if
 * the scales factors are roughly the same, marking the MAT_FLAG_UNIFORM_SCALE
 * flag, or MAT_FLAG_GENERAL_SCALE. Marks the MAT_DIRTY_TYPE and
 * MAT_DIRTY_INVERSE dirty flags.
 */
void
_math_matrix_scale( GLmatrix *mat, GLfloat x, GLfloat y, GLfloat z )
{
   GLfloat *m = mat->m;
   m[0] *= x;   m[4] *= y;   m[8]  *= z;
   m[1] *= x;   m[5] *= y;   m[9]  *= z;
   m[2] *= x;   m[6] *= y;   m[10] *= z;
   m[3] *= x;   m[7] *= y;   m[11] *= z;

   if (fabsf(x - y) < 1e-8F && fabsf(x - z) < 1e-8F)
      mat->flags |= MAT_FLAG_UNIFORM_SCALE;
   else
      mat->flags |= MAT_FLAG_GENERAL_SCALE;

   mat->flags |= (MAT_DIRTY_TYPE |
                  MAT_DIRTY_INVERSE);
}

/**
 * Multiply a matrix with a translation matrix.
 *
 * \param mat matrix.
 * \param x translation vector x coordinate.
 * \param y translation vector y coordinate.
 * \param z translation vector z coordinate.
 *
 * Adds the translation coordinates to the elements of \p mat in-place.  Marks
 * the MAT_FLAG_TRANSLATION flag, and the MAT_DIRTY_TYPE and MAT_DIRTY_INVERSE
 * dirty flags.
 */
void
_math_matrix_translate( GLmatrix *mat, GLfloat x, GLfloat y, GLfloat z )
{
   GLfloat *m = mat->m;
   m[12] = m[0] * x + m[4] * y + m[8]  * z + m[12];
   m[13] = m[1] * x + m[5] * y + m[9]  * z + m[13];
   m[14] = m[2] * x + m[6] * y + m[10] * z + m[14];
   m[15] = m[3] * x + m[7] * y + m[11] * z + m[15];

   mat->flags |= (MAT_FLAG_TRANSLATION |
                  MAT_DIRTY_TYPE |
                  MAT_DIRTY_INVERSE);
}


/**
 * Set matrix to do viewport and depthrange mapping.
 * Transforms Normalized Device Coords to window/Z values.
 */
void
_math_matrix_viewport(GLmatrix *m, const float scale[3],
                      const float translate[3], double depthMax)
{
   m->m[0] = scale[0];
   m->m[5] = scale[1];
   m->m[10] = depthMax*scale[2];
   m->m[12] = translate[0];
   m->m[13] = translate[1];
   m->m[14] = depthMax*translate[2];
   m->flags = MAT_FLAG_GENERAL_SCALE | MAT_FLAG_TRANSLATION;
   m->type = MATRIX_3D_NO_ROT;
}


/**
 * Set a matrix to the identity matrix.
 *
 * \param mat matrix.
 *
 * Copies ::Identity into \p GLmatrix::m, and into GLmatrix::inv if not NULL.
 * Sets the matrix type to identity, and clear the dirty flags.
 */
void
_math_matrix_set_identity( GLmatrix *mat )
{
   matrix_set_identity(mat->m);
   matrix_set_identity(mat->inv);

   mat->type = MATRIX_IDENTITY;
   mat->flags &= ~(MAT_DIRTY_FLAGS|
                   MAT_DIRTY_TYPE|
                   MAT_DIRTY_INVERSE);
}

/*@}*/


/**********************************************************************/
/** \name Matrix analysis */
/*@{*/

#define ZERO(x) (1<<x)
#define ONE(x)  (1<<(x+16))

#define MASK_NO_TRX      (ZERO(12) | ZERO(13) | ZERO(14))
#define MASK_NO_2D_SCALE ( ONE(0)  | ONE(5))

#define MASK_IDENTITY    ( ONE(0)  | ZERO(4)  | ZERO(8)  | ZERO(12) |\
                          ZERO(1)  |  ONE(5)  | ZERO(9)  | ZERO(13) |\
                          ZERO(2)  | ZERO(6)  |  ONE(10) | ZERO(14) |\
                          ZERO(3)  | ZERO(7)  | ZERO(11) |  ONE(15) )

#define MASK_2D_NO_ROT   (           ZERO(4)  | ZERO(8)  |           \
                          ZERO(1)  |            ZERO(9)  |           \
                          ZERO(2)  | ZERO(6)  |  ONE(10) | ZERO(14) |\
                          ZERO(3)  | ZERO(7)  | ZERO(11) |  ONE(15) )

#define MASK_2D          (                      ZERO(8)  |           \
                                                ZERO(9)  |           \
                          ZERO(2)  | ZERO(6)  |  ONE(10) | ZERO(14) |\
                          ZERO(3)  | ZERO(7)  | ZERO(11) |  ONE(15) )


#define MASK_3D_NO_ROT   (           ZERO(4)  | ZERO(8)  |           \
                          ZERO(1)  |            ZERO(9)  |           \
                          ZERO(2)  | ZERO(6)  |                      \
                          ZERO(3)  | ZERO(7)  | ZERO(11) |  ONE(15) )

#define MASK_3D          (                                           \
                                                                     \
                                                                     \
                          ZERO(3)  | ZERO(7)  | ZERO(11) |  ONE(15) )


#define MASK_PERSPECTIVE (           ZERO(4)  |            ZERO(12) |\
                          ZERO(1)  |                       ZERO(13) |\
                          ZERO(2)  | ZERO(6)  |                      \
                          ZERO(3)  | ZERO(7)  |            ZERO(15) )

#define SQ(x) ((x)*(x))

/**
 * Determine type and flags from scratch.
 *
 * \param mat matrix.
 *
 * This is expensive enough to only want to do it once.
 */
static void analyse_from_scratch( GLmatrix *mat )
{
   const GLfloat *m = mat->m;
   GLuint mask = 0;
   GLuint i;

   for (i = 0 ; i < 16 ; i++) {
      if (m[i] == 0.0F) mask |= (1<<i);
   }

   if (m[0] == 1.0F) mask |= (1<<16);
   if (m[5] == 1.0F) mask |= (1<<21);
   if (m[10] == 1.0F) mask |= (1<<26);
   if (m[15] == 1.0F) mask |= (1<<31);

   mat->flags &= ~MAT_FLAGS_GEOMETRY;

   /* Check for translation - no-one really cares
    */
   if ((mask & MASK_NO_TRX) != MASK_NO_TRX)
      mat->flags |= MAT_FLAG_TRANSLATION;

   /* Do the real work
    */
   if (mask == (GLuint) MASK_IDENTITY) {
      mat->type = MATRIX_IDENTITY;
   }
   else if ((mask & MASK_2D_NO_ROT) == (GLuint) MASK_2D_NO_ROT) {
      mat->type = MATRIX_2D_NO_ROT;

      if ((mask & MASK_NO_2D_SCALE) != MASK_NO_2D_SCALE)
         mat->flags |= MAT_FLAG_GENERAL_SCALE;
   }
   else if ((mask & MASK_2D) == (GLuint) MASK_2D) {
      GLfloat mm = DOT2(m, m);
      GLfloat m4m4 = DOT2(m+4,m+4);
      GLfloat mm4 = DOT2(m,m+4);

      mat->type = MATRIX_2D;

      /* Check for scale */
      if (SQ(mm-1) > SQ(1e-6F) ||
          SQ(m4m4-1) > SQ(1e-6F))
         mat->flags |= MAT_FLAG_GENERAL_SCALE;

      /* Check for rotation */
      if (SQ(mm4) > SQ(1e-6F))
         mat->flags |= MAT_FLAG_GENERAL_3D;
      else
         mat->flags |= MAT_FLAG_ROTATION;

   }
   else if ((mask & MASK_3D_NO_ROT) == (GLuint) MASK_3D_NO_ROT) {
      mat->type = MATRIX_3D_NO_ROT;

      /* Check for scale */
      if (SQ(m[0]-m[5]) < SQ(1e-6F) &&
          SQ(m[0]-m[10]) < SQ(1e-6F)) {
         if (SQ(m[0]-1.0F) > SQ(1e-6F)) {
            mat->flags |= MAT_FLAG_UNIFORM_SCALE;
         }
      }
      else {
         mat->flags |= MAT_FLAG_GENERAL_SCALE;
      }
   }
   else if ((mask & MASK_3D) == (GLuint) MASK_3D) {
      GLfloat c1 = DOT3(m,m);
      GLfloat c2 = DOT3(m+4,m+4);
      GLfloat c3 = DOT3(m+8,m+8);
      GLfloat d1 = DOT3(m, m+4);
      GLfloat cp[3];

      mat->type = MATRIX_3D;

      /* Check for scale */
      if (SQ(c1-c2) < SQ(1e-6F) && SQ(c1-c3) < SQ(1e-6F)) {
         if (SQ(c1-1.0F) > SQ(1e-6F))
            mat->flags |= MAT_FLAG_UNIFORM_SCALE;
         /* else no scale at all */
      }
      else {
         mat->flags |= MAT_FLAG_GENERAL_SCALE;
      }

      /* Check for rotation */
      if (SQ(d1) < SQ(1e-6F)) {
         CROSS3( cp, m, m+4 );
         SUB_3V( cp, cp, (m+8) );
         if (LEN_SQUARED_3FV(cp) < SQ(1e-6F))
            mat->flags |= MAT_FLAG_ROTATION;
         else
            mat->flags |= MAT_FLAG_GENERAL_3D;
      }
      else {
         mat->flags |= MAT_FLAG_GENERAL_3D; /* shear, etc */
      }
   }
   else if ((mask & MASK_PERSPECTIVE) == MASK_PERSPECTIVE && m[11]==-1.0F) {
      mat->type = MATRIX_PERSPECTIVE;
      mat->flags |= MAT_FLAG_GENERAL;
   }
   else {
      mat->type = MATRIX_GENERAL;
      mat->flags |= MAT_FLAG_GENERAL;
   }
}

/**
 * Analyze a matrix given that its flags are accurate.
 *
 * This is the more common operation, hopefully.
 */
static void analyse_from_flags( GLmatrix *mat )
{
   const GLfloat *m = mat->m;

   if (TEST_MAT_FLAGS(mat, 0)) {
      mat->type = MATRIX_IDENTITY;
   }
   else if (TEST_MAT_FLAGS(mat, (MAT_FLAG_TRANSLATION |
                                 MAT_FLAG_UNIFORM_SCALE |
                                 MAT_FLAG_GENERAL_SCALE))) {
      if ( m[10]==1.0F && m[14]==0.0F ) {
         mat->type = MATRIX_2D_NO_ROT;
      }
      else {
         mat->type = MATRIX_3D_NO_ROT;
      }
   }
   else if (TEST_MAT_FLAGS(mat, MAT_FLAGS_3D)) {
      if (                                 m[ 8]==0.0F
            &&                             m[ 9]==0.0F
            && m[2]==0.0F && m[6]==0.0F && m[10]==1.0F && m[14]==0.0F) {
         mat->type = MATRIX_2D;
      }
      else {
         mat->type = MATRIX_3D;
      }
   }
   else if (                 m[4]==0.0F                 && m[12]==0.0F
            && m[1]==0.0F                               && m[13]==0.0F
            && m[2]==0.0F && m[6]==0.0F
            && m[3]==0.0F && m[7]==0.0F && m[11]==-1.0F && m[15]==0.0F) {
      mat->type = MATRIX_PERSPECTIVE;
   }
   else {
      mat->type = MATRIX_GENERAL;
   }
}

/**
 * Analyze and update a matrix.
 *
 * \param mat matrix.
 *
 * If the matrix type is dirty then calls either analyse_from_scratch() or
 * analyse_from_flags() to determine its type, according to whether the flags
 * are dirty or not, respectively. If the matrix has an inverse and it's dirty
 * then calls matrix_invert(). Finally clears the dirty flags.
 */
void
_math_matrix_analyse( GLmatrix *mat )
{
   if (mat->flags & MAT_DIRTY_TYPE) {
      if (mat->flags & MAT_DIRTY_FLAGS)
         analyse_from_scratch( mat );
      else
         analyse_from_flags( mat );
   }

   if (mat->flags & MAT_DIRTY_INVERSE) {
      matrix_invert( mat );
      mat->flags &= ~MAT_DIRTY_INVERSE;
   }

   mat->flags &= ~(MAT_DIRTY_FLAGS | MAT_DIRTY_TYPE);
}

/*@}*/


/**
 * Test if the given matrix preserves vector lengths.
 */
GLboolean
_math_matrix_is_length_preserving( const GLmatrix *m )
{
   return TEST_MAT_FLAGS( m, MAT_FLAGS_LENGTH_PRESERVING);
}

GLboolean
_math_matrix_is_dirty( const GLmatrix *m )
{
   return (m->flags & MAT_DIRTY) ? GL_TRUE : GL_FALSE;
}


/**********************************************************************/
/** \name Matrix setup */
/*@{*/

/**
 * Copy a matrix.
 *
 * \param to destination matrix.
 * \param from source matrix.
 *
 * Copies all fields in GLmatrix, creating an inverse array if necessary.
 */
void
_math_matrix_copy( GLmatrix *to, const GLmatrix *from )
{
   memcpy(to->m, from->m, 16 * sizeof(GLfloat));
   memcpy(to->inv, from->inv, 16 * sizeof(GLfloat));
   to->flags = from->flags;
   to->type = from->type;
}

/**
 * Copy a matrix as part of glPushMatrix.
 *
 * The makes the source matrix canonical (inverse and flags are up-to-date),
 * so that later glPopMatrix is evaluated as a no-op if there is no state
 * change.
 *
 * It this wasn't done, a draw call would canonicalize the matrix, which
 * would make it different from the pushed one and so glPopMatrix wouldn't be
 * recognized as a no-op.
 */
void
_math_matrix_push_copy(GLmatrix *to, GLmatrix *from)
{
   if (from->flags & MAT_DIRTY)
      _math_matrix_analyse(from);

   _math_matrix_copy(to, from);
}

/**
 * Loads a matrix array into GLmatrix.
 *
 * \param m matrix array.
 * \param mat matrix.
 *
 * Copies \p m into GLmatrix::m and marks the MAT_FLAG_GENERAL and MAT_DIRTY
 * flags.
 */
void
_math_matrix_loadf( GLmatrix *mat, const GLfloat *m )
{
   memcpy( mat->m, m, 16*sizeof(GLfloat) );
   mat->flags = (MAT_FLAG_GENERAL | MAT_DIRTY);
}

/**
 * Matrix constructor.
 *
 * \param m matrix.
 *
 * Initialize the GLmatrix fields.
 */
void
_math_matrix_ctr( GLmatrix *m )
{
   memset(m, 0, sizeof(*m));
   matrix_set_identity(m->m);
   matrix_set_identity(m->inv);
   m->type = MATRIX_IDENTITY;
   m->flags = 0;
}

/*@}*/


/**********************************************************************/
/** \name Matrix transpose */
/*@{*/

/**
 * Transpose a GLfloat matrix.
 *
 * \param to destination array.
 * \param from source array.
 */
void
_math_transposef( GLfloat to[16], const GLfloat from[16] )
{
   to[0] = from[0];
   to[1] = from[4];
   to[2] = from[8];
   to[3] = from[12];
   to[4] = from[1];
   to[5] = from[5];
   to[6] = from[9];
   to[7] = from[13];
   to[8] = from[2];
   to[9] = from[6];
   to[10] = from[10];
   to[11] = from[14];
   to[12] = from[3];
   to[13] = from[7];
   to[14] = from[11];
   to[15] = from[15];
}

/**
 * Transpose a GLdouble matrix.
 *
 * \param to destination array.
 * \param from source array.
 */
void
_math_transposed( GLdouble to[16], const GLdouble from[16] )
{
   to[0] = from[0];
   to[1] = from[4];
   to[2] = from[8];
   to[3] = from[12];
   to[4] = from[1];
   to[5] = from[5];
   to[6] = from[9];
   to[7] = from[13];
   to[8] = from[2];
   to[9] = from[6];
   to[10] = from[10];
   to[11] = from[14];
   to[12] = from[3];
   to[13] = from[7];
   to[14] = from[11];
   to[15] = from[15];
}

/**
 * Transpose a GLdouble matrix and convert to GLfloat.
 *
 * \param to destination array.
 * \param from source array.
 */
void
_math_transposefd( GLfloat to[16], const GLdouble from[16] )
{
   to[0] = (GLfloat) from[0];
   to[1] = (GLfloat) from[4];
   to[2] = (GLfloat) from[8];
   to[3] = (GLfloat) from[12];
   to[4] = (GLfloat) from[1];
   to[5] = (GLfloat) from[5];
   to[6] = (GLfloat) from[9];
   to[7] = (GLfloat) from[13];
   to[8] = (GLfloat) from[2];
   to[9] = (GLfloat) from[6];
   to[10] = (GLfloat) from[10];
   to[11] = (GLfloat) from[14];
   to[12] = (GLfloat) from[3];
   to[13] = (GLfloat) from[7];
   to[14] = (GLfloat) from[11];
   to[15] = (GLfloat) from[15];
}

/*@}*/


/**
 * Transform a 4-element row vector (1x4 matrix) by a 4x4 matrix.  This
 * function is used for transforming clipping plane equations and spotlight
 * directions.
 * Mathematically,  u = v * m.
 * Input:  v - input vector
 *         m - transformation matrix
 * Output:  u - transformed vector
 */
void
_mesa_transform_vector( GLfloat u[4], const GLfloat v[4], const GLfloat m[16] )
{
   const GLfloat v0 = v[0], v1 = v[1], v2 = v[2], v3 = v[3];
#define M(row,col)  m[row + col*4]
   u[0] = v0 * M(0,0) + v1 * M(1,0) + v2 * M(2,0) + v3 * M(3,0);
   u[1] = v0 * M(0,1) + v1 * M(1,1) + v2 * M(2,1) + v3 * M(3,1);
   u[2] = v0 * M(0,2) + v1 * M(1,2) + v2 * M(2,2) + v3 * M(3,2);
   u[3] = v0 * M(0,3) + v1 * M(1,3) + v2 * M(2,3) + v3 * M(3,3);
#undef M
}
