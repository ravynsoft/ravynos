/**************************************************************************

Copyright 2002 VMware, Inc.
Copyright 2011 Dave Airlie (ARB_vertex_type_2_10_10_10_rev support)
All Rights Reserved.

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
on the rights to use, copy, modify, merge, publish, distribute, sub
license, and/or sell copies of the Software, and to permit persons to whom
the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice (including the next
paragraph) shall be included in all copies or substantial portions of the
Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
VMWARE AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
USE OR OTHER DEALINGS IN THE SOFTWARE.

**************************************************************************/

#include "util/format_r11g11b10f.h"
#include "main/varray.h"
#include "vbo_util.h"
#include "util/half_float.h"

#ifdef SUPPRESS_STATIC
#define static
#endif

/* ATTR */
#define ATTRI( A, N, V0, V1, V2, V3 ) \
    ATTR_UNION(A, N, GL_INT, uint32_t, INT_AS_UINT(V0), INT_AS_UINT(V1), \
        INT_AS_UINT(V2), INT_AS_UINT(V3))
#define ATTRUI( A, N, V0, V1, V2, V3 ) \
    ATTR_UNION(A, N, GL_UNSIGNED_INT, uint32_t, (uint32_t)(V0), (uint32_t)(V1), \
        (uint32_t)(V2), (uint32_t)(V3))
#define ATTRF( A, N, V0, V1, V2, V3 ) \
    ATTR_UNION(A, N, GL_FLOAT, uint32_t, FLOAT_AS_UINT(V0), FLOAT_AS_UINT(V1),\
        FLOAT_AS_UINT(V2), FLOAT_AS_UINT(V3))
#define ATTRD( A, N, V0, V1, V2, V3 ) \
    ATTR_UNION(A, N, GL_DOUBLE, uint64_t, DOUBLE_AS_UINT64(V0), \
        DOUBLE_AS_UINT64(V1), DOUBLE_AS_UINT64(V2), DOUBLE_AS_UINT64(V3))
#define ATTRUI64( A, N, V0, V1, V2, V3 ) \
    ATTR_UNION(A, N, GL_UNSIGNED_INT64_ARB, uint64_t, V0, V1, V2, V3)


/* float */
#define ATTR1FV( A, V ) ATTRF( A, 1, (V)[0], 0, 0, 1 )
#define ATTR2FV( A, V ) ATTRF( A, 2, (V)[0], (V)[1], 0, 1 )
#define ATTR3FV( A, V ) ATTRF( A, 3, (V)[0], (V)[1], (V)[2], 1 )
#define ATTR4FV( A, V ) ATTRF( A, 4, (V)[0], (V)[1], (V)[2], (V)[3] )

#define ATTR1F( A, X )          ATTRF( A, 1, X, 0, 0, 1 )
#define ATTR2F( A, X, Y )       ATTRF( A, 2, X, Y, 0, 1 )
#define ATTR3F( A, X, Y, Z )    ATTRF( A, 3, X, Y, Z, 1 )
#define ATTR4F( A, X, Y, Z, W ) ATTRF( A, 4, X, Y, Z, W )


/* half */
#define ATTR1HV( A, V ) ATTRF( A, 1, _mesa_half_to_float((uint16_t)(V)[0]), \
                               0, 0, 1 )
#define ATTR2HV( A, V ) ATTRF( A, 2, _mesa_half_to_float((uint16_t)(V)[0]), \
                               _mesa_half_to_float((uint16_t)(V)[1]), 0, 1 )
#define ATTR3HV( A, V ) ATTRF( A, 3, _mesa_half_to_float((uint16_t)(V)[0]), \
                               _mesa_half_to_float((uint16_t)(V)[1]), \
                               _mesa_half_to_float((uint16_t)(V)[2]), 1 )
#define ATTR4HV( A, V ) ATTRF( A, 4, _mesa_half_to_float((uint16_t)(V)[0]), \
                               _mesa_half_to_float((uint16_t)(V)[1]), \
                               _mesa_half_to_float((uint16_t)(V)[2]), \
                               _mesa_half_to_float((uint16_t)(V)[3]) )

#define ATTR1H( A, X )          ATTRF( A, 1, _mesa_half_to_float(X), 0, 0, 1 )
#define ATTR2H( A, X, Y )       ATTRF( A, 2, _mesa_half_to_float(X), \
                                       _mesa_half_to_float(Y), 0, 1 )
#define ATTR3H( A, X, Y, Z )    ATTRF( A, 3, _mesa_half_to_float(X), \
                                       _mesa_half_to_float(Y), \
                                       _mesa_half_to_float(Z), 1 )
#define ATTR4H( A, X, Y, Z, W ) ATTRF( A, 4, _mesa_half_to_float(X), \
                                       _mesa_half_to_float(Y), \
                                       _mesa_half_to_float(Z), \
                                       _mesa_half_to_float(W) )


/* int */
#define ATTR2IV( A, V ) ATTRI( A, 2, (V)[0], (V)[1], 0, 1 )
#define ATTR3IV( A, V ) ATTRI( A, 3, (V)[0], (V)[1], (V)[2], 1 )
#define ATTR4IV( A, V ) ATTRI( A, 4, (V)[0], (V)[1], (V)[2], (V)[3] )

#define ATTR1I( A, X )          ATTRI( A, 1, X, 0, 0, 1 )
#define ATTR2I( A, X, Y )       ATTRI( A, 2, X, Y, 0, 1 )
#define ATTR3I( A, X, Y, Z )    ATTRI( A, 3, X, Y, Z, 1 )
#define ATTR4I( A, X, Y, Z, W ) ATTRI( A, 4, X, Y, Z, W )


/* uint */
#define ATTR2UIV( A, V ) ATTRUI( A, 2, (V)[0], (V)[1], 0, 1 )
#define ATTR3UIV( A, V ) ATTRUI( A, 3, (V)[0], (V)[1], (V)[2], 1 )
#define ATTR4UIV( A, V ) ATTRUI( A, 4, (V)[0], (V)[1], (V)[2], (V)[3] )

#define ATTR1UI( A, X )          ATTRUI( A, 1, X, 0, 0, 1 )
#define ATTR2UI( A, X, Y )       ATTRUI( A, 2, X, Y, 0, 1 )
#define ATTR3UI( A, X, Y, Z )    ATTRUI( A, 3, X, Y, Z, 1 )
#define ATTR4UI( A, X, Y, Z, W ) ATTRUI( A, 4, X, Y, Z, W )

#define MAT_ATTR( A, N, V ) ATTRF( A, N, (V)[0], (V)[1], (V)[2], (V)[3] )

#define ATTRUI10_1( A, UI ) ATTRF( A, 1, (UI) & 0x3ff, 0, 0, 1 )
#define ATTRUI10_2( A, UI ) ATTRF( A, 2, (UI) & 0x3ff, ((UI) >> 10) & 0x3ff, 0, 1 )
#define ATTRUI10_3( A, UI ) ATTRF( A, 3, (UI) & 0x3ff, ((UI) >> 10) & 0x3ff, ((UI) >> 20) & 0x3ff, 1 )
#define ATTRUI10_4( A, UI ) ATTRF( A, 4, (UI) & 0x3ff, ((UI) >> 10) & 0x3ff, ((UI) >> 20) & 0x3ff, ((UI) >> 30) & 0x3 )

#define ATTRUI10N_1( A, UI ) ATTRF( A, 1, conv_ui10_to_norm_float((UI) & 0x3ff), 0, 0, 1 )
#define ATTRUI10N_2( A, UI ) ATTRF( A, 2, \
				   conv_ui10_to_norm_float((UI) & 0x3ff), \
				   conv_ui10_to_norm_float(((UI) >> 10) & 0x3ff), 0, 1 )
#define ATTRUI10N_3( A, UI ) ATTRF( A, 3, \
				   conv_ui10_to_norm_float((UI) & 0x3ff), \
				   conv_ui10_to_norm_float(((UI) >> 10) & 0x3ff), \
				   conv_ui10_to_norm_float(((UI) >> 20) & 0x3ff), 1 )
#define ATTRUI10N_4( A, UI ) ATTRF( A, 4, \
				   conv_ui10_to_norm_float((UI) & 0x3ff), \
				   conv_ui10_to_norm_float(((UI) >> 10) & 0x3ff), \
				   conv_ui10_to_norm_float(((UI) >> 20) & 0x3ff), \
				   conv_ui2_to_norm_float(((UI) >> 30) & 0x3) )

#define ATTRI10_1( A, I10 ) ATTRF( A, 1, conv_i10_to_i((I10) & 0x3ff), 0, 0, 1 )
#define ATTRI10_2( A, I10 ) ATTRF( A, 2, \
				conv_i10_to_i((I10) & 0x3ff),		\
				conv_i10_to_i(((I10) >> 10) & 0x3ff), 0, 1 )
#define ATTRI10_3( A, I10 ) ATTRF( A, 3, \
				conv_i10_to_i((I10) & 0x3ff),	    \
				conv_i10_to_i(((I10) >> 10) & 0x3ff), \
				conv_i10_to_i(((I10) >> 20) & 0x3ff), 1 )
#define ATTRI10_4( A, I10 ) ATTRF( A, 4, \
				conv_i10_to_i((I10) & 0x3ff),		\
				conv_i10_to_i(((I10) >> 10) & 0x3ff), \
				conv_i10_to_i(((I10) >> 20) & 0x3ff), \
				conv_i2_to_i(((I10) >> 30) & 0x3))


#define ATTRI10N_1(ctx, A, I10) ATTRF(A, 1, conv_i10_to_norm_float(ctx, (I10) & 0x3ff), 0, 0, 1 )
#define ATTRI10N_2(ctx, A, I10) ATTRF(A, 2, \
				conv_i10_to_norm_float(ctx, (I10) & 0x3ff),		\
				conv_i10_to_norm_float(ctx, ((I10) >> 10) & 0x3ff), 0, 1 )
#define ATTRI10N_3(ctx, A, I10) ATTRF(A, 3, \
				conv_i10_to_norm_float(ctx, (I10) & 0x3ff),	    \
				conv_i10_to_norm_float(ctx, ((I10) >> 10) & 0x3ff), \
				conv_i10_to_norm_float(ctx, ((I10) >> 20) & 0x3ff), 1 )
#define ATTRI10N_4(ctx, A, I10) ATTRF(A, 4, \
				conv_i10_to_norm_float(ctx, (I10) & 0x3ff),		\
				conv_i10_to_norm_float(ctx, ((I10) >> 10) & 0x3ff), \
				conv_i10_to_norm_float(ctx, ((I10) >> 20) & 0x3ff), \
				conv_i2_to_norm_float(ctx, ((I10) >> 30) & 0x3))

#define ATTR_UI(ctx, val, type, normalized, attr, arg) do {	\
   if ((type) == GL_UNSIGNED_INT_2_10_10_10_REV) {		\
      if (normalized) {						\
	 ATTRUI10N_##val((attr), (arg));			\
      } else {							\
	 ATTRUI10_##val((attr), (arg));				\
      }								\
   } else if ((type) == GL_INT_2_10_10_10_REV) {		\
      if (normalized) {						\
	 ATTRI10N_##val(ctx, (attr), (arg));			\
      } else {							\
	 ATTRI10_##val((attr), (arg));				\
      }								\
   } else if ((type) == GL_UNSIGNED_INT_10F_11F_11F_REV) {	\
      float res[4];						\
      res[3] = 1;                                               \
      r11g11b10f_to_float3((arg), res);				\
      ATTR##val##FV((attr), res);				\
   } else							\
      ERROR(GL_INVALID_VALUE);					\
   } while(0)

#define ATTR_UI_INDEX(ctx, val, type, normalized, index, arg) do {	\
      if ((index) == 0 && _mesa_attr_zero_aliases_vertex(ctx)) {	\
	 ATTR_UI(ctx, val, (type), normalized, 0, (arg));		\
      } else if ((index) < MAX_VERTEX_GENERIC_ATTRIBS) {		\
	 ATTR_UI(ctx, val, (type), normalized, VBO_ATTRIB_GENERIC0 + (index), (arg)); \
      } else								\
	 ERROR(GL_INVALID_VALUE);					\
   } while(0)


/* Doubles */
#define ATTR1DV( A, V ) ATTRD( A, 1, (V)[0], 0, 0, 1 )
#define ATTR2DV( A, V ) ATTRD( A, 2, (V)[0], (V)[1], 0, 1 )
#define ATTR3DV( A, V ) ATTRD( A, 3, (V)[0], (V)[1], (V)[2], 1 )
#define ATTR4DV( A, V ) ATTRD( A, 4, (V)[0], (V)[1], (V)[2], (V)[3] )

#define ATTR1D( A, X )          ATTRD( A, 1, X, 0, 0, 1 )
#define ATTR2D( A, X, Y )       ATTRD( A, 2, X, Y, 0, 1 )
#define ATTR3D( A, X, Y, Z )    ATTRD( A, 3, X, Y, Z, 1 )
#define ATTR4D( A, X, Y, Z, W ) ATTRD( A, 4, X, Y, Z, W )

#define ATTR1UIV64( A, V ) ATTRUI64( A, 1, (V)[0], 0, 0, 0 )
#define ATTR1UI64( A, X )  ATTRUI64( A, 1, X, 0, 0, 0 )


static void GLAPIENTRY
TAG(Vertex2f)(GLfloat x, GLfloat y)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR2F(VBO_ATTRIB_POS, x, y);
}

static void GLAPIENTRY
TAG(Vertex2fv)(const GLfloat * v)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR2FV(VBO_ATTRIB_POS, v);
}

static void GLAPIENTRY
TAG(Vertex3f)(GLfloat x, GLfloat y, GLfloat z)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR3F(VBO_ATTRIB_POS, x, y, z);
}

static void GLAPIENTRY
TAG(Vertex3fv)(const GLfloat * v)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR3FV(VBO_ATTRIB_POS, v);
}

static void GLAPIENTRY
TAG(Vertex4f)(GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR4F(VBO_ATTRIB_POS, x, y, z, w);
}

static void GLAPIENTRY
TAG(Vertex4fv)(const GLfloat * v)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR4FV(VBO_ATTRIB_POS, v);
}

static void GLAPIENTRY
TAG(VertexAttrib1fARB)(GLuint index, GLfloat x)
{
   GET_CURRENT_CONTEXT(ctx);
   if (is_vertex_position(ctx, index))
      ATTR1F(0, x);
   else if (index < MAX_VERTEX_GENERIC_ATTRIBS)
      ATTR1F(VBO_ATTRIB_GENERIC0 + index, x);
   else
      ERROR(GL_INVALID_VALUE);
}

static void GLAPIENTRY
TAG(VertexAttrib1fvARB)(GLuint index, const GLfloat * v)
{
   GET_CURRENT_CONTEXT(ctx);
   if (is_vertex_position(ctx, index))
      ATTR1FV(0, v);
   else if (index < MAX_VERTEX_GENERIC_ATTRIBS)
      ATTR1FV(VBO_ATTRIB_GENERIC0 + index, v);
   else
      ERROR(GL_INVALID_VALUE);
}

static void GLAPIENTRY
TAG(VertexAttrib2fARB)(GLuint index, GLfloat x, GLfloat y)
{
   GET_CURRENT_CONTEXT(ctx);
   if (is_vertex_position(ctx, index))
      ATTR2F(0, x, y);
   else if (index < MAX_VERTEX_GENERIC_ATTRIBS)
      ATTR2F(VBO_ATTRIB_GENERIC0 + index, x, y);
   else
      ERROR(GL_INVALID_VALUE);
}

static void GLAPIENTRY
TAG(VertexAttrib2fvARB)(GLuint index, const GLfloat * v)
{
   GET_CURRENT_CONTEXT(ctx);
   if (is_vertex_position(ctx, index))
      ATTR2FV(0, v);
   else if (index < MAX_VERTEX_GENERIC_ATTRIBS)
      ATTR2FV(VBO_ATTRIB_GENERIC0 + index, v);
   else
      ERROR(GL_INVALID_VALUE);
}

static void GLAPIENTRY
TAG(VertexAttrib3fARB)(GLuint index, GLfloat x, GLfloat y, GLfloat z)
{
   GET_CURRENT_CONTEXT(ctx);
   if (is_vertex_position(ctx, index))
      ATTR3F(0, x, y, z);
   else if (index < MAX_VERTEX_GENERIC_ATTRIBS)
      ATTR3F(VBO_ATTRIB_GENERIC0 + index, x, y, z);
   else
      ERROR(GL_INVALID_VALUE);
}

static void GLAPIENTRY
TAG(VertexAttrib3fvARB)(GLuint index, const GLfloat * v)
{
   GET_CURRENT_CONTEXT(ctx);
   if (is_vertex_position(ctx, index))
      ATTR3FV(0, v);
   else if (index < MAX_VERTEX_GENERIC_ATTRIBS)
      ATTR3FV(VBO_ATTRIB_GENERIC0 + index, v);
   else
      ERROR(GL_INVALID_VALUE);
}

static void GLAPIENTRY
TAG(VertexAttrib4fARB)(GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
   GET_CURRENT_CONTEXT(ctx);
   if (is_vertex_position(ctx, index))
      ATTR4F(0, x, y, z, w);
   else if (index < MAX_VERTEX_GENERIC_ATTRIBS)
      ATTR4F(VBO_ATTRIB_GENERIC0 + index, x, y, z, w);
   else
      ERROR(GL_INVALID_VALUE);
}

static void GLAPIENTRY
TAG(VertexAttrib4fvARB)(GLuint index, const GLfloat * v)
{
   GET_CURRENT_CONTEXT(ctx);
   if (is_vertex_position(ctx, index))
      ATTR4FV(0, v);
   else if (index < MAX_VERTEX_GENERIC_ATTRIBS)
      ATTR4FV(VBO_ATTRIB_GENERIC0 + index, v);
   else
      ERROR(GL_INVALID_VALUE);
}



/* Integer-valued generic attributes.
 * XXX: the integers just get converted to floats at this time
 */
static void GLAPIENTRY
TAG(VertexAttribI1iEXT)(GLuint index, GLint x)
{
   GET_CURRENT_CONTEXT(ctx);
   if (is_vertex_position(ctx, index))
      ATTR1I(0, x);
   else if (index < MAX_VERTEX_GENERIC_ATTRIBS)
      ATTR1I(VBO_ATTRIB_GENERIC0 + index, x);
   else
      ERROR(GL_INVALID_VALUE);
}

static void GLAPIENTRY
TAG(VertexAttribI2iEXT)(GLuint index, GLint x, GLint y)
{
   GET_CURRENT_CONTEXT(ctx);
   if (is_vertex_position(ctx, index))
      ATTR2I(0, x, y);
   else if (index < MAX_VERTEX_GENERIC_ATTRIBS)
      ATTR2I(VBO_ATTRIB_GENERIC0 + index, x, y);
   else
      ERROR(GL_INVALID_VALUE);
}

static void GLAPIENTRY
TAG(VertexAttribI3iEXT)(GLuint index, GLint x, GLint y, GLint z)
{
   GET_CURRENT_CONTEXT(ctx);
   if (is_vertex_position(ctx, index))
      ATTR3I(0, x, y, z);
   else if (index < MAX_VERTEX_GENERIC_ATTRIBS)
      ATTR3I(VBO_ATTRIB_GENERIC0 + index, x, y, z);
   else
      ERROR(GL_INVALID_VALUE);
}

static void GLAPIENTRY
TAG(VertexAttribI4iEXT)(GLuint index, GLint x, GLint y, GLint z, GLint w)
{
   GET_CURRENT_CONTEXT(ctx);
   if (is_vertex_position(ctx, index))
      ATTR4I(0, x, y, z, w);
   else if (index < MAX_VERTEX_GENERIC_ATTRIBS)
      ATTR4I(VBO_ATTRIB_GENERIC0 + index, x, y, z, w);
   else
      ERROR(GL_INVALID_VALUE);
}

static void GLAPIENTRY
TAG(VertexAttribI2ivEXT)(GLuint index, const GLint *v)
{
   GET_CURRENT_CONTEXT(ctx);
   if (is_vertex_position(ctx, index))
      ATTR2IV(0, v);
   else if (index < MAX_VERTEX_GENERIC_ATTRIBS)
      ATTR2IV(VBO_ATTRIB_GENERIC0 + index, v);
   else
      ERROR(GL_INVALID_VALUE);
}

static void GLAPIENTRY
TAG(VertexAttribI3ivEXT)(GLuint index, const GLint *v)
{
   GET_CURRENT_CONTEXT(ctx);
   if (is_vertex_position(ctx, index))
      ATTR3IV(0, v);
   else if (index < MAX_VERTEX_GENERIC_ATTRIBS)
      ATTR3IV(VBO_ATTRIB_GENERIC0 + index, v);
   else
      ERROR(GL_INVALID_VALUE);
}

static void GLAPIENTRY
TAG(VertexAttribI4ivEXT)(GLuint index, const GLint *v)
{
   GET_CURRENT_CONTEXT(ctx);
   if (is_vertex_position(ctx, index))
      ATTR4IV(0, v);
   else if (index < MAX_VERTEX_GENERIC_ATTRIBS)
      ATTR4IV(VBO_ATTRIB_GENERIC0 + index, v);
   else
      ERROR(GL_INVALID_VALUE);
}



/* Unsigned integer-valued generic attributes.
 * XXX: the integers just get converted to floats at this time
 */
static void GLAPIENTRY
TAG(VertexAttribI1uiEXT)(GLuint index, GLuint x)
{
   GET_CURRENT_CONTEXT(ctx);
   if (is_vertex_position(ctx, index))
      ATTR1UI(0, x);
   else if (index < MAX_VERTEX_GENERIC_ATTRIBS)
      ATTR1UI(VBO_ATTRIB_GENERIC0 + index, x);
   else
      ERROR(GL_INVALID_VALUE);
}

static void GLAPIENTRY
TAG(VertexAttribI2uiEXT)(GLuint index, GLuint x, GLuint y)
{
   GET_CURRENT_CONTEXT(ctx);
   if (is_vertex_position(ctx, index))
      ATTR2UI(0, x, y);
   else if (index < MAX_VERTEX_GENERIC_ATTRIBS)
      ATTR2UI(VBO_ATTRIB_GENERIC0 + index, x, y);
   else
      ERROR(GL_INVALID_VALUE);
}

static void GLAPIENTRY
TAG(VertexAttribI3uiEXT)(GLuint index, GLuint x, GLuint y, GLuint z)
{
   GET_CURRENT_CONTEXT(ctx);
   if (is_vertex_position(ctx, index))
      ATTR3UI(0, x, y, z);
   else if (index < MAX_VERTEX_GENERIC_ATTRIBS)
      ATTR3UI(VBO_ATTRIB_GENERIC0 + index, x, y, z);
   else
      ERROR(GL_INVALID_VALUE);
}

static void GLAPIENTRY
TAG(VertexAttribI4uiEXT)(GLuint index, GLuint x, GLuint y, GLuint z, GLuint w)
{
   GET_CURRENT_CONTEXT(ctx);
   if (is_vertex_position(ctx, index))
      ATTR4UI(0, x, y, z, w);
   else if (index < MAX_VERTEX_GENERIC_ATTRIBS)
      ATTR4UI(VBO_ATTRIB_GENERIC0 + index, x, y, z, w);
   else
      ERROR(GL_INVALID_VALUE);
}

static void GLAPIENTRY
TAG(VertexAttribI2uivEXT)(GLuint index, const GLuint *v)
{
   GET_CURRENT_CONTEXT(ctx);
   if (is_vertex_position(ctx, index))
      ATTR2UIV(0, v);
   else if (index < MAX_VERTEX_GENERIC_ATTRIBS)
      ATTR2UIV(VBO_ATTRIB_GENERIC0 + index, v);
   else
      ERROR(GL_INVALID_VALUE);
}

static void GLAPIENTRY
TAG(VertexAttribI3uivEXT)(GLuint index, const GLuint *v)
{
   GET_CURRENT_CONTEXT(ctx);
   if (is_vertex_position(ctx, index))
      ATTR3UIV(0, v);
   else if (index < MAX_VERTEX_GENERIC_ATTRIBS)
      ATTR3UIV(VBO_ATTRIB_GENERIC0 + index, v);
   else
      ERROR(GL_INVALID_VALUE);
}

static void GLAPIENTRY
TAG(VertexAttribI4uivEXT)(GLuint index, const GLuint *v)
{
   GET_CURRENT_CONTEXT(ctx);
   if (is_vertex_position(ctx, index))
      ATTR4UIV(0, v);
   else if (index < MAX_VERTEX_GENERIC_ATTRIBS)
      ATTR4UIV(VBO_ATTRIB_GENERIC0 + index, v);
   else
      ERROR(GL_INVALID_VALUE);
}



/* These entrypoints are no longer used for NV_vertex_program but they are
 * used by the display list and other code specifically because of their
 * property of aliasing with the legacy Vertex, TexCoord, Normal, etc
 * attributes.  (See vbo_save_loopback.c)
 */
static void GLAPIENTRY
TAG(VertexAttrib1fNV)(GLuint index, GLfloat x)
{
   GET_CURRENT_CONTEXT(ctx);
   if (index < VBO_ATTRIB_MAX)
      ATTR1F(index, x);
}

static void GLAPIENTRY
TAG(VertexAttrib1fvNV)(GLuint index, const GLfloat * v)
{
   GET_CURRENT_CONTEXT(ctx);
   if (index < VBO_ATTRIB_MAX)
      ATTR1FV(index, v);
}

static void GLAPIENTRY
TAG(VertexAttrib2fNV)(GLuint index, GLfloat x, GLfloat y)
{
   GET_CURRENT_CONTEXT(ctx);
   if (index < VBO_ATTRIB_MAX)
      ATTR2F(index, x, y);
}

static void GLAPIENTRY
TAG(VertexAttrib2fvNV)(GLuint index, const GLfloat * v)
{
   GET_CURRENT_CONTEXT(ctx);
   if (index < VBO_ATTRIB_MAX)
      ATTR2FV(index, v);
}

static void GLAPIENTRY
TAG(VertexAttrib3fNV)(GLuint index, GLfloat x, GLfloat y, GLfloat z)
{
   GET_CURRENT_CONTEXT(ctx);
   if (index < VBO_ATTRIB_MAX)
      ATTR3F(index, x, y, z);
}

static void GLAPIENTRY
TAG(VertexAttrib3fvNV)(GLuint index,
 const GLfloat * v)
{
   GET_CURRENT_CONTEXT(ctx);
   if (index < VBO_ATTRIB_MAX)
      ATTR3FV(index, v);
}

static void GLAPIENTRY
TAG(VertexAttrib4fNV)(GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
   GET_CURRENT_CONTEXT(ctx);
   if (index < VBO_ATTRIB_MAX)
      ATTR4F(index, x, y, z, w);
}

static void GLAPIENTRY
TAG(VertexAttrib4fvNV)(GLuint index, const GLfloat * v)
{
   GET_CURRENT_CONTEXT(ctx);
   if (index < VBO_ATTRIB_MAX)
      ATTR4FV(index, v);
}

static void GLAPIENTRY
TAG(VertexP2ui)(GLenum type, GLuint value)
{
   GET_CURRENT_CONTEXT(ctx);
   ERROR_IF_NOT_PACKED_TYPE(ctx, type, "glVertexP2ui");
   ATTR_UI(ctx, 2, type, 0, VBO_ATTRIB_POS, value);
}

static void GLAPIENTRY
TAG(VertexP2uiv)(GLenum type, const GLuint *value)
{
   GET_CURRENT_CONTEXT(ctx);
   ERROR_IF_NOT_PACKED_TYPE(ctx, type, "glVertexP2uiv");
   ATTR_UI(ctx, 2, type, 0, VBO_ATTRIB_POS, value[0]);
}

static void GLAPIENTRY
TAG(VertexP3ui)(GLenum type, GLuint value)
{
   GET_CURRENT_CONTEXT(ctx);
   ERROR_IF_NOT_PACKED_TYPE(ctx, type, "glVertexP3ui");
   ATTR_UI(ctx, 3, type, 0, VBO_ATTRIB_POS, value);
}

static void GLAPIENTRY
TAG(VertexP3uiv)(GLenum type, const GLuint *value)
{
   GET_CURRENT_CONTEXT(ctx);
   ERROR_IF_NOT_PACKED_TYPE(ctx, type, "glVertexP3uiv");
   ATTR_UI(ctx, 3, type, 0, VBO_ATTRIB_POS, value[0]);
}

static void GLAPIENTRY
TAG(VertexP4ui)(GLenum type, GLuint value)
{
   GET_CURRENT_CONTEXT(ctx);
   ERROR_IF_NOT_PACKED_TYPE(ctx, type, "glVertexP4ui");
   ATTR_UI(ctx, 4, type, 0, VBO_ATTRIB_POS, value);
}

static void GLAPIENTRY
TAG(VertexP4uiv)(GLenum type, const GLuint *value)
{
   GET_CURRENT_CONTEXT(ctx);
   ERROR_IF_NOT_PACKED_TYPE(ctx, type, "glVertexP4uiv");
   ATTR_UI(ctx, 4, type, 0, VBO_ATTRIB_POS, value[0]);
}

static void GLAPIENTRY
TAG(VertexAttribP1ui)(GLuint index, GLenum type, GLboolean normalized,
		      GLuint value)
{
   GET_CURRENT_CONTEXT(ctx);
   ERROR_IF_NOT_PACKED_TYPE_EXT(ctx, type, "glVertexAttribP1ui");
   ATTR_UI_INDEX(ctx, 1, type, normalized, index, value);
}

static void GLAPIENTRY
TAG(VertexAttribP2ui)(GLuint index, GLenum type, GLboolean normalized,
		      GLuint value)
{
   GET_CURRENT_CONTEXT(ctx);
   ERROR_IF_NOT_PACKED_TYPE_EXT(ctx, type, "glVertexAttribP2ui");
   ATTR_UI_INDEX(ctx, 2, type, normalized, index, value);
}

static void GLAPIENTRY
TAG(VertexAttribP3ui)(GLuint index, GLenum type, GLboolean normalized,
		      GLuint value)
{
   GET_CURRENT_CONTEXT(ctx);
   ERROR_IF_NOT_PACKED_TYPE_EXT(ctx, type, "glVertexAttribP3ui");
   ATTR_UI_INDEX(ctx, 3, type, normalized, index, value);
}

static void GLAPIENTRY
TAG(VertexAttribP4ui)(GLuint index, GLenum type, GLboolean normalized,
		      GLuint value)
{
   GET_CURRENT_CONTEXT(ctx);
   ERROR_IF_NOT_PACKED_TYPE(ctx, type, "glVertexAttribP4ui");
   ATTR_UI_INDEX(ctx, 4, type, normalized, index, value);
}

static void GLAPIENTRY
TAG(VertexAttribP1uiv)(GLuint index, GLenum type, GLboolean normalized,
		       const GLuint *value)
{
   GET_CURRENT_CONTEXT(ctx);
   ERROR_IF_NOT_PACKED_TYPE_EXT(ctx, type, "glVertexAttribP1uiv");
   ATTR_UI_INDEX(ctx, 1, type, normalized, index, *value);
}

static void GLAPIENTRY
TAG(VertexAttribP2uiv)(GLuint index, GLenum type, GLboolean normalized,
		       const GLuint *value)
{
   GET_CURRENT_CONTEXT(ctx);
   ERROR_IF_NOT_PACKED_TYPE_EXT(ctx, type, "glVertexAttribP2uiv");
   ATTR_UI_INDEX(ctx, 2, type, normalized, index, *value);
}

static void GLAPIENTRY
TAG(VertexAttribP3uiv)(GLuint index, GLenum type, GLboolean normalized,
		       const GLuint *value)
{
   GET_CURRENT_CONTEXT(ctx);
   ERROR_IF_NOT_PACKED_TYPE_EXT(ctx, type, "glVertexAttribP3uiv");
   ATTR_UI_INDEX(ctx, 3, type, normalized, index, *value);
}

static void GLAPIENTRY
TAG(VertexAttribP4uiv)(GLuint index, GLenum type, GLboolean normalized,
		      const GLuint *value)
{
   GET_CURRENT_CONTEXT(ctx);
   ERROR_IF_NOT_PACKED_TYPE(ctx, type, "glVertexAttribP4uiv");
   ATTR_UI_INDEX(ctx, 4, type, normalized, index, *value);
}



static void GLAPIENTRY
TAG(VertexAttribL1d)(GLuint index, GLdouble x)
{
   GET_CURRENT_CONTEXT(ctx);
   if (is_vertex_position(ctx, index))
      ATTR1D(0, x);
   else if (index < MAX_VERTEX_GENERIC_ATTRIBS)
      ATTR1D(VBO_ATTRIB_GENERIC0 + index, x);
   else
      ERROR(GL_INVALID_VALUE);
}

static void GLAPIENTRY
TAG(VertexAttribL1dv)(GLuint index, const GLdouble * v)
{
   GET_CURRENT_CONTEXT(ctx);
   if (is_vertex_position(ctx, index))
      ATTR1DV(0, v);
   else if (index < MAX_VERTEX_GENERIC_ATTRIBS)
      ATTR1DV(VBO_ATTRIB_GENERIC0 + index, v);
   else
      ERROR(GL_INVALID_VALUE);
}

static void GLAPIENTRY
TAG(VertexAttribL2d)(GLuint index, GLdouble x, GLdouble y)
{
   GET_CURRENT_CONTEXT(ctx);
   if (is_vertex_position(ctx, index))
      ATTR2D(0, x, y);
   else if (index < MAX_VERTEX_GENERIC_ATTRIBS)
      ATTR2D(VBO_ATTRIB_GENERIC0 + index, x, y);
   else
      ERROR(GL_INVALID_VALUE);
}

static void GLAPIENTRY
TAG(VertexAttribL2dv)(GLuint index, const GLdouble * v)
{
   GET_CURRENT_CONTEXT(ctx);
   if (is_vertex_position(ctx, index))
      ATTR2DV(0, v);
   else if (index < MAX_VERTEX_GENERIC_ATTRIBS)
      ATTR2DV(VBO_ATTRIB_GENERIC0 + index, v);
   else
      ERROR(GL_INVALID_VALUE);
}

static void GLAPIENTRY
TAG(VertexAttribL3d)(GLuint index, GLdouble x, GLdouble y, GLdouble z)
{
   GET_CURRENT_CONTEXT(ctx);
   if (is_vertex_position(ctx, index))
      ATTR3D(0, x, y, z);
   else if (index < MAX_VERTEX_GENERIC_ATTRIBS)
      ATTR3D(VBO_ATTRIB_GENERIC0 + index, x, y, z);
   else
      ERROR(GL_INVALID_VALUE);
}

static void GLAPIENTRY
TAG(VertexAttribL3dv)(GLuint index, const GLdouble * v)
{
   GET_CURRENT_CONTEXT(ctx);
   if (is_vertex_position(ctx, index))
      ATTR3DV(0, v);
   else if (index < MAX_VERTEX_GENERIC_ATTRIBS)
      ATTR3DV(VBO_ATTRIB_GENERIC0 + index, v);
   else
      ERROR(GL_INVALID_VALUE);
}

static void GLAPIENTRY
TAG(VertexAttribL4d)(GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
   GET_CURRENT_CONTEXT(ctx);
   if (is_vertex_position(ctx, index))
      ATTR4D(0, x, y, z, w);
   else if (index < MAX_VERTEX_GENERIC_ATTRIBS)
      ATTR4D(VBO_ATTRIB_GENERIC0 + index, x, y, z, w);
   else
      ERROR(GL_INVALID_VALUE);
}

static void GLAPIENTRY
TAG(VertexAttribL4dv)(GLuint index, const GLdouble * v)
{
   GET_CURRENT_CONTEXT(ctx);
   if (is_vertex_position(ctx, index))
      ATTR4DV(0, v);
   else if (index < MAX_VERTEX_GENERIC_ATTRIBS)
      ATTR4DV(VBO_ATTRIB_GENERIC0 + index, v);
   else
      ERROR(GL_INVALID_VALUE);
}

static void GLAPIENTRY
TAG(VertexAttribL1ui64ARB)(GLuint index, GLuint64EXT x)
{
   GET_CURRENT_CONTEXT(ctx);
   if (is_vertex_position(ctx, index))
      ATTR1UI64(0, x);
   else if (index < MAX_VERTEX_GENERIC_ATTRIBS)
      ATTR1UI64(VBO_ATTRIB_GENERIC0 + index, x);
   else
      ERROR(GL_INVALID_VALUE);
}

static void GLAPIENTRY
TAG(VertexAttribL1ui64vARB)(GLuint index, const GLuint64EXT *v)
{
   GET_CURRENT_CONTEXT(ctx);
   if (is_vertex_position(ctx, index))
      ATTR1UIV64(0, v);
   else if (index < MAX_VERTEX_GENERIC_ATTRIBS)
      ATTR1UIV64(VBO_ATTRIB_GENERIC0 + index, v);
   else
      ERROR(GL_INVALID_VALUE);
}

/* GL_NV_half_float */
static void GLAPIENTRY
TAG(Vertex2hNV)(GLhalfNV x, GLhalfNV y)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR2H(VBO_ATTRIB_POS, x, y);
}

static void GLAPIENTRY
TAG(Vertex2hvNV)(const GLhalfNV * v)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR2HV(VBO_ATTRIB_POS, v);
}

static void GLAPIENTRY
TAG(Vertex3hNV)(GLhalfNV x, GLhalfNV y, GLhalfNV z)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR3H(VBO_ATTRIB_POS, x, y, z);
}

static void GLAPIENTRY
TAG(Vertex3hvNV)(const GLhalfNV * v)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR3HV(VBO_ATTRIB_POS, v);
}

static void GLAPIENTRY
TAG(Vertex4hNV)(GLhalfNV x, GLhalfNV y, GLhalfNV z, GLhalfNV w)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR4H(VBO_ATTRIB_POS, x, y, z, w);
}

static void GLAPIENTRY
TAG(Vertex4hvNV)(const GLhalfNV * v)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR4HV(VBO_ATTRIB_POS, v);
}

static void GLAPIENTRY
TAG(VertexAttrib1hNV)(GLuint index, GLhalfNV x)
{
   GET_CURRENT_CONTEXT(ctx);
   if (is_vertex_position(ctx, index))
      ATTR1H(0, x);
   else if (index < MAX_VERTEX_GENERIC_ATTRIBS)
      ATTR1H(VBO_ATTRIB_GENERIC0 + index, x);
   else
      ERROR(GL_INVALID_VALUE);
}

static void GLAPIENTRY
TAG(VertexAttrib2hNV)(GLuint index, GLhalfNV x, GLhalfNV y)
{
   GET_CURRENT_CONTEXT(ctx);
   if (is_vertex_position(ctx, index))
      ATTR2H(0, x, y);
   else if (index < MAX_VERTEX_GENERIC_ATTRIBS)
      ATTR2H(VBO_ATTRIB_GENERIC0 + index, x, y);
   else
      ERROR(GL_INVALID_VALUE);
}

static void GLAPIENTRY
TAG(VertexAttrib3hNV)(GLuint index, GLhalfNV x, GLhalfNV y, GLhalfNV z)
{
   GET_CURRENT_CONTEXT(ctx);
   if (is_vertex_position(ctx, index))
      ATTR3H(0, x, y, z);
   else if (index < MAX_VERTEX_GENERIC_ATTRIBS)
      ATTR3H(VBO_ATTRIB_GENERIC0 + index, x, y, z);
   else
      ERROR(GL_INVALID_VALUE);
}

static void GLAPIENTRY
TAG(VertexAttrib4hNV)(GLuint index, GLhalfNV x, GLhalfNV y, GLhalfNV z, GLhalfNV w)
{
   GET_CURRENT_CONTEXT(ctx);
   if (is_vertex_position(ctx, index))
      ATTR4H(0, x, y, z, w);
   else if (index < MAX_VERTEX_GENERIC_ATTRIBS)
      ATTR4H(VBO_ATTRIB_GENERIC0 + index, x, y, z, w);
   else
      ERROR(GL_INVALID_VALUE);
}

static void GLAPIENTRY
TAG(VertexAttrib1hvNV)(GLuint index, const GLhalfNV * v)
{
   GET_CURRENT_CONTEXT(ctx);
   if (is_vertex_position(ctx, index))
      ATTR1HV(0, v);
   else if (index < MAX_VERTEX_GENERIC_ATTRIBS)
      ATTR1HV(VBO_ATTRIB_GENERIC0 + index, v);
   else
      ERROR(GL_INVALID_VALUE);
}

static void GLAPIENTRY
TAG(VertexAttrib2hvNV)(GLuint index, const GLhalfNV * v)
{
   GET_CURRENT_CONTEXT(ctx);
   if (is_vertex_position(ctx, index))
      ATTR2HV(0, v);
   else if (index < MAX_VERTEX_GENERIC_ATTRIBS)
      ATTR2HV(VBO_ATTRIB_GENERIC0 + index, v);
   else
      ERROR(GL_INVALID_VALUE);
}

static void GLAPIENTRY
TAG(VertexAttrib3hvNV)(GLuint index, const GLhalfNV * v)
{
   GET_CURRENT_CONTEXT(ctx);
   if (is_vertex_position(ctx, index))
      ATTR3HV(0, v);
   else if (index < MAX_VERTEX_GENERIC_ATTRIBS)
      ATTR3HV(VBO_ATTRIB_GENERIC0 + index, v);
   else
      ERROR(GL_INVALID_VALUE);
}

static void GLAPIENTRY
TAG(VertexAttrib4hvNV)(GLuint index, const GLhalfNV * v)
{
   GET_CURRENT_CONTEXT(ctx);
   if (is_vertex_position(ctx, index))
      ATTR4HV(0, v);
   else if (index < MAX_VERTEX_GENERIC_ATTRIBS)
      ATTR4HV(VBO_ATTRIB_GENERIC0 + index, v);
   else
      ERROR(GL_INVALID_VALUE);
}

static void GLAPIENTRY
TAG(VertexAttribs1hvNV)(GLuint index, GLsizei n, const GLhalfNV *v)
{
   GET_CURRENT_CONTEXT(ctx);
   n = MIN2(n, VBO_ATTRIB_MAX - index);
   for (GLint i = n - 1; i >= 0; i--)
      ATTR1H(index + i, v[i]);
}

static void GLAPIENTRY
TAG(VertexAttribs2hvNV)(GLuint index, GLsizei n, const GLhalfNV *v)
{
   GET_CURRENT_CONTEXT(ctx);
   n = MIN2(n, VBO_ATTRIB_MAX - index);
   for (GLint i = n - 1; i >= 0; i--)
      ATTR2H(index + i, v[2 * i], v[2 * i + 1]);
}

static void GLAPIENTRY
TAG(VertexAttribs3hvNV)(GLuint index, GLsizei n, const GLhalfNV *v)
{
   GET_CURRENT_CONTEXT(ctx);
   n = MIN2(n, VBO_ATTRIB_MAX - index);
   for (GLint i = n - 1; i >= 0; i--)
      ATTR3H(index + i, v[3 * i], v[3 * i + 1], v[3 * i + 2]);
}


static void GLAPIENTRY
TAG(VertexAttribs4hvNV)(GLuint index, GLsizei n, const GLhalfNV *v)
{
   GET_CURRENT_CONTEXT(ctx);
   n = MIN2(n, VBO_ATTRIB_MAX - index);
   for (GLint i = n - 1; i >= 0; i--)
      ATTR4H(index + i, v[4 * i], v[4 * i + 1], v[4 * i + 2], v[4 * i + 3]);
}

static void GLAPIENTRY
TAG(Vertex2d)(GLdouble x, GLdouble y)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR2F(VBO_ATTRIB_POS, (GLfloat) x, (GLfloat) y);
}

static void GLAPIENTRY
TAG(Vertex2i)(GLint x, GLint y)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR2F(VBO_ATTRIB_POS, (GLfloat) x, (GLfloat) y);
}

static void GLAPIENTRY
TAG(Vertex2s)(GLshort x, GLshort y)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR2F(VBO_ATTRIB_POS, (GLfloat) x, (GLfloat) y);
}

static void GLAPIENTRY
TAG(Vertex3d)(GLdouble x, GLdouble y, GLdouble z)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR3F(VBO_ATTRIB_POS, (GLfloat) x, (GLfloat) y, (GLfloat) z);
}

static void GLAPIENTRY
TAG(Vertex3i)(GLint x, GLint y, GLint z)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR3F(VBO_ATTRIB_POS, (GLfloat) x, (GLfloat) y, (GLfloat) z);
}

static void GLAPIENTRY
TAG(Vertex3s)(GLshort x, GLshort y, GLshort z)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR3F(VBO_ATTRIB_POS, (GLfloat) x, (GLfloat) y, (GLfloat) z);
}

static void GLAPIENTRY
TAG(Vertex4d)(GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR4F(VBO_ATTRIB_POS, (GLfloat) x, (GLfloat) y, (GLfloat) z, (GLfloat) w);
}

static void GLAPIENTRY
TAG(Vertex4i)(GLint x, GLint y, GLint z, GLint w)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR4F(VBO_ATTRIB_POS, (GLfloat) x, (GLfloat) y, (GLfloat) z, (GLfloat) w);
}

static void GLAPIENTRY
TAG(Vertex4s)(GLshort x, GLshort y, GLshort z, GLshort w)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR4F(VBO_ATTRIB_POS, (GLfloat) x, (GLfloat) y, (GLfloat) z, (GLfloat) w);
}

static void GLAPIENTRY
TAG(Vertex2dv)(const GLdouble *v)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR2F(VBO_ATTRIB_POS, (GLfloat) v[0], (GLfloat) v[1]);
}

static void GLAPIENTRY
TAG(Vertex2iv)(const GLint *v)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR2F(VBO_ATTRIB_POS, (GLfloat) v[0], (GLfloat) v[1]);
}

static void GLAPIENTRY
TAG(Vertex2sv)(const GLshort *v)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR2F(VBO_ATTRIB_POS, (GLfloat) v[0], (GLfloat) v[1]);
}

static void GLAPIENTRY
TAG(Vertex3dv)(const GLdouble *v)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR3F(VBO_ATTRIB_POS, (GLfloat) v[0], (GLfloat) v[1], (GLfloat) v[2]);
}

static void GLAPIENTRY
TAG(Vertex3iv)(const GLint *v)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR3F(VBO_ATTRIB_POS, (GLfloat) v[0], (GLfloat) v[1], (GLfloat) v[2]);
}

static void GLAPIENTRY
TAG(Vertex3sv)(const GLshort *v)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR3F(VBO_ATTRIB_POS, (GLfloat) v[0], (GLfloat) v[1], (GLfloat) v[2]);
}

static void GLAPIENTRY
TAG(Vertex4dv)(const GLdouble *v)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR4F(VBO_ATTRIB_POS, (GLfloat) v[0], (GLfloat) v[1],
         (GLfloat) v[2], (GLfloat) v[3]);
}

static void GLAPIENTRY
TAG(Vertex4iv)(const GLint *v)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR4F(VBO_ATTRIB_POS, (GLfloat) v[0], (GLfloat) v[1],
         (GLfloat) v[2], (GLfloat) v[3]);
}

static void GLAPIENTRY
TAG(Vertex4sv)(const GLshort *v)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR4F(VBO_ATTRIB_POS, (GLfloat) v[0], (GLfloat) v[1],
         (GLfloat) v[2], (GLfloat) v[3]);
}

/*
 * GL_NV_vertex_program:
 * Note that attribute indexes DO alias conventional vertex attributes.
 */

static void GLAPIENTRY
TAG(VertexAttrib1sNV)(GLuint index, GLshort x)
{
   GET_CURRENT_CONTEXT(ctx);
   if (index < VBO_ATTRIB_MAX) ATTR1F(index, (GLfloat) x);
}

static void GLAPIENTRY
TAG(VertexAttrib1dNV)(GLuint index, GLdouble x)
{
   GET_CURRENT_CONTEXT(ctx);
   if (index < VBO_ATTRIB_MAX) ATTR1F(index, (GLfloat) x);
}

static void GLAPIENTRY
TAG(VertexAttrib2sNV)(GLuint index, GLshort x, GLshort y)
{
   GET_CURRENT_CONTEXT(ctx);
   if (index < VBO_ATTRIB_MAX) ATTR2F(index, (GLfloat) x, y);
}

static void GLAPIENTRY
TAG(VertexAttrib2dNV)(GLuint index, GLdouble x, GLdouble y)
{
   GET_CURRENT_CONTEXT(ctx);
   if (index < VBO_ATTRIB_MAX) ATTR2F(index, (GLfloat) x, (GLfloat) y);
}

static void GLAPIENTRY
TAG(VertexAttrib3sNV)(GLuint index, GLshort x, GLshort y, GLshort z)
{
   GET_CURRENT_CONTEXT(ctx);
   if (index < VBO_ATTRIB_MAX) ATTR3F(index, (GLfloat) x, (GLfloat) y, (GLfloat) z);
}

static void GLAPIENTRY
TAG(VertexAttrib3dNV)(GLuint index, GLdouble x, GLdouble y, GLdouble z)
{
   GET_CURRENT_CONTEXT(ctx);
   if (index < VBO_ATTRIB_MAX) ATTR4F(index, (GLfloat) x, (GLfloat) y, (GLfloat) z, 1.0F);
}

static void GLAPIENTRY
TAG(VertexAttrib4sNV)(GLuint index, GLshort x, GLshort y, GLshort z, GLshort w)
{
   GET_CURRENT_CONTEXT(ctx);
   if (index < VBO_ATTRIB_MAX) ATTR4F(index, (GLfloat) x, (GLfloat) y, (GLfloat) z, (GLfloat) w);
}

static void GLAPIENTRY
TAG(VertexAttrib4dNV)(GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
   GET_CURRENT_CONTEXT(ctx);
   if (index < VBO_ATTRIB_MAX) ATTR4F(index, (GLfloat) x, (GLfloat) y, (GLfloat) z, (GLfloat) w);
}

static void GLAPIENTRY
TAG(VertexAttrib4ubNV)(GLuint index, GLubyte x, GLubyte y, GLubyte z, GLubyte w)
{
   GET_CURRENT_CONTEXT(ctx);
   if (index < VBO_ATTRIB_MAX) ATTR4F(index, UBYTE_TO_FLOAT(x), UBYTE_TO_FLOAT(y),
                                      UBYTE_TO_FLOAT(z), UBYTE_TO_FLOAT(w));
}

static void GLAPIENTRY
TAG(VertexAttrib1svNV)(GLuint index, const GLshort *v)
{
   GET_CURRENT_CONTEXT(ctx);
   if (index < VBO_ATTRIB_MAX) ATTR1F(index, (GLfloat) v[0]);
}

static void GLAPIENTRY
TAG(VertexAttrib1dvNV)(GLuint index, const GLdouble *v)
{
   GET_CURRENT_CONTEXT(ctx);
   if (index < VBO_ATTRIB_MAX) ATTR1F(index, (GLfloat) v[0]);
}

static void GLAPIENTRY
TAG(VertexAttrib2svNV)(GLuint index, const GLshort *v)
{
   GET_CURRENT_CONTEXT(ctx);
   if (index < VBO_ATTRIB_MAX) ATTR2F(index, (GLfloat) v[0], (GLfloat) v[1]);
}

static void GLAPIENTRY
TAG(VertexAttrib2dvNV)(GLuint index, const GLdouble *v)
{
   GET_CURRENT_CONTEXT(ctx);
   if (index < VBO_ATTRIB_MAX) ATTR2F(index, (GLfloat) v[0], (GLfloat) v[1]);
}

static void GLAPIENTRY
TAG(VertexAttrib3svNV)(GLuint index, const GLshort *v)
{
   GET_CURRENT_CONTEXT(ctx);
   if (index < VBO_ATTRIB_MAX) ATTR3F(index, (GLfloat) v[0], (GLfloat) v[1], (GLfloat) v[2]);
}

static void GLAPIENTRY
TAG(VertexAttrib3dvNV)(GLuint index, const GLdouble *v)
{
   GET_CURRENT_CONTEXT(ctx);
   if (index < VBO_ATTRIB_MAX) ATTR3F(index, (GLfloat) v[0], (GLfloat) v[1], (GLfloat) v[2]);
}

static void GLAPIENTRY
TAG(VertexAttrib4svNV)(GLuint index, const GLshort *v)
{
   GET_CURRENT_CONTEXT(ctx);
   if (index < VBO_ATTRIB_MAX) ATTR4F(index, (GLfloat) v[0], (GLfloat) v[1], (GLfloat) v[2],
         (GLfloat)v[3]);
}

static void GLAPIENTRY
TAG(VertexAttrib4dvNV)(GLuint index, const GLdouble *v)
{
   GET_CURRENT_CONTEXT(ctx);
   if (index < VBO_ATTRIB_MAX) ATTR4F(index, (GLfloat) v[0], (GLfloat) v[1], (GLfloat) v[2], (GLfloat) v[3]);
}

static void GLAPIENTRY
TAG(VertexAttrib4ubvNV)(GLuint index, const GLubyte *v)
{
   GET_CURRENT_CONTEXT(ctx);
   if (index < VBO_ATTRIB_MAX) ATTR4F(index, UBYTE_TO_FLOAT(v[0]), UBYTE_TO_FLOAT(v[1]),
         UBYTE_TO_FLOAT(v[2]), UBYTE_TO_FLOAT(v[3]));
}


static void GLAPIENTRY
TAG(VertexAttribs1svNV)(GLuint index, GLsizei n, const GLshort *v)
{
   GET_CURRENT_CONTEXT(ctx);
   GLint i;
   n = MIN2(n, VBO_ATTRIB_MAX - index);
   for (i = n - 1; i >= 0; i--)
      ATTR1F(index + i, (GLfloat) v[i]);
}

static void GLAPIENTRY
TAG(VertexAttribs1fvNV)(GLuint index, GLsizei n, const GLfloat *v)
{
   GET_CURRENT_CONTEXT(ctx);
   GLint i;
   n = MIN2(n, VBO_ATTRIB_MAX - index);
   for (i = n - 1; i >= 0; i--)
      ATTR1F(index + i, v[i]);
}

static void GLAPIENTRY
TAG(VertexAttribs1dvNV)(GLuint index, GLsizei n, const GLdouble *v)
{
   GET_CURRENT_CONTEXT(ctx);
   GLint i;
   n = MIN2(n, VBO_ATTRIB_MAX - index);
   for (i = n - 1; i >= 0; i--)
      ATTR1F(index + i, (GLfloat) v[i]);
}

static void GLAPIENTRY
TAG(VertexAttribs2svNV)(GLuint index, GLsizei n, const GLshort *v)
{
   GET_CURRENT_CONTEXT(ctx);
   GLint i;
   n = MIN2(n, VBO_ATTRIB_MAX - index);
   for (i = n - 1; i >= 0; i--)
      ATTR2F(index + i, (GLfloat) v[2 * i], (GLfloat) v[2 * i + 1]);
}

static void GLAPIENTRY
TAG(VertexAttribs2fvNV)(GLuint index, GLsizei n, const GLfloat *v)
{
   GET_CURRENT_CONTEXT(ctx);
   GLint i;
   n = MIN2(n, VBO_ATTRIB_MAX - index);
   for (i = n - 1; i >= 0; i--)
      ATTR2F(index + i, v[2 * i], v[2 * i + 1]);
}

static void GLAPIENTRY
TAG(VertexAttribs2dvNV)(GLuint index, GLsizei n, const GLdouble *v)
{
   GET_CURRENT_CONTEXT(ctx);
   GLint i;
   n = MIN2(n, VBO_ATTRIB_MAX - index);
   for (i = n - 1; i >= 0; i--)
      ATTR2F(index + i, (GLfloat) v[2 * i], (GLfloat) v[2 * i + 1]);
}

static void GLAPIENTRY
TAG(VertexAttribs3svNV)(GLuint index, GLsizei n, const GLshort *v)
{
   GET_CURRENT_CONTEXT(ctx);
   GLint i;
   n = MIN2(n, VBO_ATTRIB_MAX - index);
   for (i = n - 1; i >= 0; i--)
      ATTR3F(index + i, (GLfloat) v[3 * i], (GLfloat) v[3 * i + 1], (GLfloat) v[3 * i + 2]);
}

static void GLAPIENTRY
TAG(VertexAttribs3fvNV)(GLuint index, GLsizei n, const GLfloat *v)
{
   GET_CURRENT_CONTEXT(ctx);
   GLint i;
   n = MIN2(n, VBO_ATTRIB_MAX - index);
   for (i = n - 1; i >= 0; i--)
      ATTR3F(index + i, v[3 * i], v[3 * i + 1], v[3 * i + 2]);
}

static void GLAPIENTRY
TAG(VertexAttribs3dvNV)(GLuint index, GLsizei n, const GLdouble *v)
{
   GET_CURRENT_CONTEXT(ctx);
   GLint i;
   n = MIN2(n, VBO_ATTRIB_MAX - index);
   for (i = n - 1; i >= 0; i--)
      ATTR3F(index + i, (GLfloat) v[3 * i], (GLfloat) v[3 * i + 1], (GLfloat) v[3 * i + 2]);
}

static void GLAPIENTRY
TAG(VertexAttribs4svNV)(GLuint index, GLsizei n, const GLshort *v)
{
   GET_CURRENT_CONTEXT(ctx);
   GLint i;
   n = MIN2(n, VBO_ATTRIB_MAX - index);
   for (i = n - 1; i >= 0; i--)
      ATTR4F(index + i, (GLfloat) v[4 * i], (GLfloat) v[4 * i + 1], (GLfloat) v[4 * i + 2], (GLfloat) v[4 * i + 3]);
}

static void GLAPIENTRY
TAG(VertexAttribs4fvNV)(GLuint index, GLsizei n, const GLfloat *v)
{
   GET_CURRENT_CONTEXT(ctx);
   GLint i;
   n = MIN2(n, VBO_ATTRIB_MAX - index);
   for (i = n - 1; i >= 0; i--)
      ATTR4F(index + i, v[4 * i], v[4 * i + 1], v[4 * i + 2], v[4 * i + 3]);
}

static void GLAPIENTRY
TAG(VertexAttribs4dvNV)(GLuint index, GLsizei n, const GLdouble *v)
{
   GET_CURRENT_CONTEXT(ctx);
   GLint i;
   n = MIN2(n, VBO_ATTRIB_MAX - index);
   for (i = n - 1; i >= 0; i--)
      ATTR4F(index + i, (GLfloat) v[4 * i], (GLfloat) v[4 * i + 1], (GLfloat) v[4 * i + 2], (GLfloat) v[4 * i + 3]);
}

static void GLAPIENTRY
TAG(VertexAttribs4ubvNV)(GLuint index, GLsizei n, const GLubyte *v)
{
   GET_CURRENT_CONTEXT(ctx);
   GLint i;
   n = MIN2(n, VBO_ATTRIB_MAX - index);
   for (i = n - 1; i >= 0; i--)
      ATTR4F(index + i, UBYTE_TO_FLOAT(v[4 * i]), UBYTE_TO_FLOAT(v[4 * i + 1]),
             UBYTE_TO_FLOAT(v[4 * i + 2]), UBYTE_TO_FLOAT(v[4 * i + 3]));
}


/*
 * GL_ARB_vertex_program
 * Note that attribute indexes do NOT alias conventional attributes.
 */

static void GLAPIENTRY
TAG(VertexAttrib1s)(GLuint index, GLshort x)
{
   GET_CURRENT_CONTEXT(ctx);
   if (is_vertex_position(ctx, index))
      ATTR1F(0, (GLfloat) x);
   else if (index < MAX_VERTEX_GENERIC_ATTRIBS)
      ATTR1F(VBO_ATTRIB_GENERIC0 + index, (GLfloat) x);
   else
      ERROR(GL_INVALID_VALUE);
}

static void GLAPIENTRY
TAG(VertexAttrib1d)(GLuint index, GLdouble x)
{
   GET_CURRENT_CONTEXT(ctx);
   if (is_vertex_position(ctx, index))
      ATTR1F(0, (GLfloat) x);
   else if (index < MAX_VERTEX_GENERIC_ATTRIBS)
      ATTR1F(VBO_ATTRIB_GENERIC0 + index, (GLfloat) x);
   else
      ERROR(GL_INVALID_VALUE);
}

static void GLAPIENTRY
TAG(VertexAttrib2s)(GLuint index, GLshort x, GLshort y)
{
   GET_CURRENT_CONTEXT(ctx);
   if (is_vertex_position(ctx, index))
      ATTR2F(0, (GLfloat) x, (GLfloat) y);
   else if (index < MAX_VERTEX_GENERIC_ATTRIBS)
      ATTR2F(VBO_ATTRIB_GENERIC0 + index, (GLfloat) x, (GLfloat) y);
   else
      ERROR(GL_INVALID_VALUE);
}

static void GLAPIENTRY
TAG(VertexAttrib2d)(GLuint index, GLdouble x, GLdouble y)
{
   GET_CURRENT_CONTEXT(ctx);
   if (is_vertex_position(ctx, index))
      ATTR2F(0, (GLfloat) x, (GLfloat) y);
   else if (index < MAX_VERTEX_GENERIC_ATTRIBS)
      ATTR2F(VBO_ATTRIB_GENERIC0 + index, (GLfloat) x, (GLfloat) y);
   else
      ERROR(GL_INVALID_VALUE);
}

static void GLAPIENTRY
TAG(VertexAttrib3s)(GLuint index, GLshort x, GLshort y, GLshort z)
{
   GET_CURRENT_CONTEXT(ctx);
   if (is_vertex_position(ctx, index))
      ATTR3F(0, (GLfloat) x, (GLfloat) y, (GLfloat) z);
   else if (index < MAX_VERTEX_GENERIC_ATTRIBS)
      ATTR3F(VBO_ATTRIB_GENERIC0 + index, (GLfloat) x, (GLfloat) y, (GLfloat) z);
   else
      ERROR(GL_INVALID_VALUE);
}

static void GLAPIENTRY
TAG(VertexAttrib3d)(GLuint index, GLdouble x, GLdouble y, GLdouble z)
{
   GET_CURRENT_CONTEXT(ctx);
   if (is_vertex_position(ctx, index))
      ATTR3F(0, (GLfloat) x, (GLfloat) y, (GLfloat) z);
   else if (index < MAX_VERTEX_GENERIC_ATTRIBS)
      ATTR3F(VBO_ATTRIB_GENERIC0 + index, (GLfloat) x, (GLfloat) y, (GLfloat) z);
   else
      ERROR(GL_INVALID_VALUE);
}

static void GLAPIENTRY
TAG(VertexAttrib4s)(GLuint index, GLshort x, GLshort y, GLshort z, GLshort w)
{
   GET_CURRENT_CONTEXT(ctx);
   if (is_vertex_position(ctx, index))
      ATTR4F(0, (GLfloat) x, (GLfloat) y, (GLfloat) z, (GLfloat) w);
   else if (index < MAX_VERTEX_GENERIC_ATTRIBS)
      ATTR4F(VBO_ATTRIB_GENERIC0 + index, (GLfloat) x, (GLfloat) y, (GLfloat) z, (GLfloat) w);
   else
      ERROR(GL_INVALID_VALUE);
}

static void GLAPIENTRY
TAG(VertexAttrib4d)(GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
   GET_CURRENT_CONTEXT(ctx);
   if (is_vertex_position(ctx, index))
      ATTR4F(0, (GLfloat) x, (GLfloat) y, (GLfloat) z, (GLfloat) w);
   else if (index < MAX_VERTEX_GENERIC_ATTRIBS)
      ATTR4F(VBO_ATTRIB_GENERIC0 + index, (GLfloat) x, (GLfloat) y, (GLfloat) z, (GLfloat) w);
   else
      ERROR(GL_INVALID_VALUE);
}

static void GLAPIENTRY
TAG(VertexAttrib1sv)(GLuint index, const GLshort *v)
{
   GET_CURRENT_CONTEXT(ctx);
   if (is_vertex_position(ctx, index))
      ATTR1F(0, (GLfloat) v[0]);
   else if (index < MAX_VERTEX_GENERIC_ATTRIBS)
      ATTR1F(VBO_ATTRIB_GENERIC0 + index, (GLfloat) v[0]);
   else
      ERROR(GL_INVALID_VALUE);
}

static void GLAPIENTRY
TAG(VertexAttrib1dv)(GLuint index, const GLdouble *v)
{
   GET_CURRENT_CONTEXT(ctx);
   if (is_vertex_position(ctx, index))
      ATTR1F(0, (GLfloat) v[0]);
   else if (index < MAX_VERTEX_GENERIC_ATTRIBS)
      ATTR1F(VBO_ATTRIB_GENERIC0 + index, (GLfloat) v[0]);
   else
      ERROR(GL_INVALID_VALUE);
}

static void GLAPIENTRY
TAG(VertexAttrib2sv)(GLuint index, const GLshort *v)
{
   GET_CURRENT_CONTEXT(ctx);
   if (is_vertex_position(ctx, index))
      ATTR2F(0, (GLfloat) v[0], (GLfloat) v[1]);
   else if (index < MAX_VERTEX_GENERIC_ATTRIBS)
      ATTR2F(VBO_ATTRIB_GENERIC0 + index, (GLfloat) v[0], (GLfloat) v[1]);
   else
      ERROR(GL_INVALID_VALUE);
}

static void GLAPIENTRY
TAG(VertexAttrib2dv)(GLuint index, const GLdouble *v)
{
   GET_CURRENT_CONTEXT(ctx);
   if (is_vertex_position(ctx, index))
      ATTR2F(0, (GLfloat) v[0], (GLfloat) v[1]);
   else if (index < MAX_VERTEX_GENERIC_ATTRIBS)
      ATTR2F(VBO_ATTRIB_GENERIC0 + index, (GLfloat) v[0], (GLfloat) v[1]);
   else
      ERROR(GL_INVALID_VALUE);
}

static void GLAPIENTRY
TAG(VertexAttrib3sv)(GLuint index, const GLshort *v)
{
   GET_CURRENT_CONTEXT(ctx);
   if (is_vertex_position(ctx, index))
      ATTR3F(0, (GLfloat) v[0], (GLfloat) v[1], (GLfloat) v[2]);
   else if (index < MAX_VERTEX_GENERIC_ATTRIBS)
      ATTR3F(VBO_ATTRIB_GENERIC0 + index, (GLfloat) v[0], (GLfloat) v[1], (GLfloat) v[2]);
   else
      ERROR(GL_INVALID_VALUE);
}

static void GLAPIENTRY
TAG(VertexAttrib3dv)(GLuint index, const GLdouble *v)
{
   GET_CURRENT_CONTEXT(ctx);
   if (is_vertex_position(ctx, index))
      ATTR3F(0, (GLfloat) v[0], (GLfloat) v[1], (GLfloat) v[2]);
   else if (index < MAX_VERTEX_GENERIC_ATTRIBS)
      ATTR3F(VBO_ATTRIB_GENERIC0 + index, (GLfloat) v[0], (GLfloat) v[1], (GLfloat) v[2]);
   else
      ERROR(GL_INVALID_VALUE);
}

static void GLAPIENTRY
TAG(VertexAttrib4sv)(GLuint index, const GLshort *v)
{
   GET_CURRENT_CONTEXT(ctx);
   if (is_vertex_position(ctx, index))
      ATTR4F(0, (GLfloat) v[0], (GLfloat) v[1], (GLfloat) v[2], (GLfloat)v[3]);
   else if (index < MAX_VERTEX_GENERIC_ATTRIBS)
      ATTR4F(VBO_ATTRIB_GENERIC0 + index, (GLfloat) v[0], (GLfloat) v[1], (GLfloat) v[2], (GLfloat)v[3]);
   else
      ERROR(GL_INVALID_VALUE);
}

static void GLAPIENTRY
TAG(VertexAttrib4dv)(GLuint index, const GLdouble *v)
{
   GET_CURRENT_CONTEXT(ctx);
   if (is_vertex_position(ctx, index))
      ATTR4F(0, (GLfloat) v[0], (GLfloat) v[1], (GLfloat) v[2], (GLfloat)v[3]);
   else if (index < MAX_VERTEX_GENERIC_ATTRIBS)
      ATTR4F(VBO_ATTRIB_GENERIC0 + index, (GLfloat) v[0], (GLfloat) v[1], (GLfloat) v[2], (GLfloat)v[3]);
   else
      ERROR(GL_INVALID_VALUE);
}

static void GLAPIENTRY
TAG(VertexAttrib4bv)(GLuint index, const GLbyte * v)
{
   GET_CURRENT_CONTEXT(ctx);
   if (is_vertex_position(ctx, index))
      ATTR4F(0, (GLfloat) v[0], (GLfloat) v[1], (GLfloat) v[2], (GLfloat)v[3]);
   else if (index < MAX_VERTEX_GENERIC_ATTRIBS)
      ATTR4F(VBO_ATTRIB_GENERIC0 + index, (GLfloat) v[0], (GLfloat) v[1], (GLfloat) v[2], (GLfloat)v[3]);
   else
      ERROR(GL_INVALID_VALUE);
}

static void GLAPIENTRY
TAG(VertexAttrib4iv)(GLuint index, const GLint * v)
{
   GET_CURRENT_CONTEXT(ctx);
   if (is_vertex_position(ctx, index))
      ATTR4F(0, (GLfloat) v[0], (GLfloat) v[1], (GLfloat) v[2], (GLfloat)v[3]);
   else if (index < MAX_VERTEX_GENERIC_ATTRIBS)
      ATTR4F(VBO_ATTRIB_GENERIC0 + index, (GLfloat) v[0], (GLfloat) v[1], (GLfloat) v[2], (GLfloat)v[3]);
   else
      ERROR(GL_INVALID_VALUE);
}

static void GLAPIENTRY
TAG(VertexAttrib4ubv)(GLuint index, const GLubyte * v)
{
   GET_CURRENT_CONTEXT(ctx);
   if (is_vertex_position(ctx, index))
      ATTR4F(0, (GLfloat) v[0], (GLfloat) v[1], (GLfloat) v[2], (GLfloat)v[3]);
   else if (index < MAX_VERTEX_GENERIC_ATTRIBS)
      ATTR4F(VBO_ATTRIB_GENERIC0 + index, (GLfloat) v[0], (GLfloat) v[1], (GLfloat) v[2], (GLfloat)v[3]);
   else
      ERROR(GL_INVALID_VALUE);
}

static void GLAPIENTRY
TAG(VertexAttrib4usv)(GLuint index, const GLushort * v)
{
   GET_CURRENT_CONTEXT(ctx);
   if (is_vertex_position(ctx, index))
      ATTR4F(0, (GLfloat) v[0], (GLfloat) v[1], (GLfloat) v[2], (GLfloat)v[3]);
   else if (index < MAX_VERTEX_GENERIC_ATTRIBS)
      ATTR4F(VBO_ATTRIB_GENERIC0 + index, (GLfloat) v[0], (GLfloat) v[1], (GLfloat) v[2], (GLfloat)v[3]);
   else
      ERROR(GL_INVALID_VALUE);
}

static void GLAPIENTRY
TAG(VertexAttrib4uiv)(GLuint index, const GLuint * v)
{
   GET_CURRENT_CONTEXT(ctx);
   if (is_vertex_position(ctx, index))
      ATTR4F(0, (GLfloat) v[0], (GLfloat) v[1], (GLfloat) v[2], (GLfloat)v[3]);
   else if (index < MAX_VERTEX_GENERIC_ATTRIBS)
      ATTR4F(VBO_ATTRIB_GENERIC0 + index, (GLfloat) v[0], (GLfloat) v[1], (GLfloat) v[2], (GLfloat)v[3]);
   else
      ERROR(GL_INVALID_VALUE);
}

static void GLAPIENTRY
TAG(VertexAttrib4Nbv)(GLuint index, const GLbyte * v)
{
   GET_CURRENT_CONTEXT(ctx);
   if (is_vertex_position(ctx, index))
      ATTR4F(0, BYTE_TO_FLOAT(v[0]), BYTE_TO_FLOAT(v[1]), BYTE_TO_FLOAT(v[2]), BYTE_TO_FLOAT(v[3]));
   else if (index < MAX_VERTEX_GENERIC_ATTRIBS)
      ATTR4F(VBO_ATTRIB_GENERIC0 + index, BYTE_TO_FLOAT(v[0]), BYTE_TO_FLOAT(v[1]), BYTE_TO_FLOAT(v[2]), BYTE_TO_FLOAT(v[3]));
   else
      ERROR(GL_INVALID_VALUE);
}

static void GLAPIENTRY
TAG(VertexAttrib4Nsv)(GLuint index, const GLshort * v)
{
   GET_CURRENT_CONTEXT(ctx);
   if (is_vertex_position(ctx, index))
      ATTR4F(0, SHORT_TO_FLOAT(v[0]), SHORT_TO_FLOAT(v[1]), SHORT_TO_FLOAT(v[2]), SHORT_TO_FLOAT(v[3]));
   else if (index < MAX_VERTEX_GENERIC_ATTRIBS)
      ATTR4F(VBO_ATTRIB_GENERIC0 + index, SHORT_TO_FLOAT(v[0]), SHORT_TO_FLOAT(v[1]), SHORT_TO_FLOAT(v[2]), SHORT_TO_FLOAT(v[3]));
   else
      ERROR(GL_INVALID_VALUE);
}

static void GLAPIENTRY
TAG(VertexAttrib4Niv)(GLuint index, const GLint * v)
{
   GET_CURRENT_CONTEXT(ctx);
   if (is_vertex_position(ctx, index))
      ATTR4F(0, INT_TO_FLOAT(v[0]), INT_TO_FLOAT(v[1]), INT_TO_FLOAT(v[2]), INT_TO_FLOAT(v[3]));
   else if (index < MAX_VERTEX_GENERIC_ATTRIBS)
      ATTR4F(VBO_ATTRIB_GENERIC0 + index, INT_TO_FLOAT(v[0]), INT_TO_FLOAT(v[1]), INT_TO_FLOAT(v[2]), INT_TO_FLOAT(v[3]));
   else
      ERROR(GL_INVALID_VALUE);
}

static void GLAPIENTRY
TAG(VertexAttrib4Nub)(GLuint index, GLubyte x, GLubyte y, GLubyte z, GLubyte w)
{
   GET_CURRENT_CONTEXT(ctx);
   if (is_vertex_position(ctx, index))
      ATTR4F(0, UBYTE_TO_FLOAT(x), UBYTE_TO_FLOAT(y), UBYTE_TO_FLOAT(z), UBYTE_TO_FLOAT(w));
   else if (index < MAX_VERTEX_GENERIC_ATTRIBS)
      ATTR4F(VBO_ATTRIB_GENERIC0 + index, UBYTE_TO_FLOAT(x), UBYTE_TO_FLOAT(y), UBYTE_TO_FLOAT(z), UBYTE_TO_FLOAT(w));
   else
      ERROR(GL_INVALID_VALUE);
}

static void GLAPIENTRY
TAG(VertexAttrib4Nubv)(GLuint index, const GLubyte * v)
{
   GET_CURRENT_CONTEXT(ctx);
   if (is_vertex_position(ctx, index))
      ATTR4F(0, UBYTE_TO_FLOAT(v[0]), UBYTE_TO_FLOAT(v[1]), UBYTE_TO_FLOAT(v[2]), UBYTE_TO_FLOAT(v[3]));
   else if (index < MAX_VERTEX_GENERIC_ATTRIBS)
      ATTR4F(VBO_ATTRIB_GENERIC0 + index, UBYTE_TO_FLOAT(v[0]), UBYTE_TO_FLOAT(v[1]), UBYTE_TO_FLOAT(v[2]), UBYTE_TO_FLOAT(v[3]));
   else
      ERROR(GL_INVALID_VALUE);
}

static void GLAPIENTRY
TAG(VertexAttrib4Nusv)(GLuint index, const GLushort * v)
{
   GET_CURRENT_CONTEXT(ctx);
   if (is_vertex_position(ctx, index))
      ATTR4F(0, USHORT_TO_FLOAT(v[0]), USHORT_TO_FLOAT(v[1]), USHORT_TO_FLOAT(v[2]), USHORT_TO_FLOAT(v[3]));
   else if (index < MAX_VERTEX_GENERIC_ATTRIBS)
      ATTR4F(VBO_ATTRIB_GENERIC0 + index, USHORT_TO_FLOAT(v[0]), USHORT_TO_FLOAT(v[1]), USHORT_TO_FLOAT(v[2]), USHORT_TO_FLOAT(v[3]));
   else
      ERROR(GL_INVALID_VALUE);
}

static void GLAPIENTRY
TAG(VertexAttrib4Nuiv)(GLuint index, const GLuint * v)
{
   GET_CURRENT_CONTEXT(ctx);
   if (is_vertex_position(ctx, index))
      ATTR4F(0, UINT_TO_FLOAT(v[0]), UINT_TO_FLOAT(v[1]), UINT_TO_FLOAT(v[2]), UINT_TO_FLOAT(v[3]));
   else if (index < MAX_VERTEX_GENERIC_ATTRIBS)
      ATTR4F(VBO_ATTRIB_GENERIC0 + index, UINT_TO_FLOAT(v[0]), UINT_TO_FLOAT(v[1]), UINT_TO_FLOAT(v[2]), UINT_TO_FLOAT(v[3]));
   else
      ERROR(GL_INVALID_VALUE);
}

/**
 * GL_EXT_gpu_shader / GL 3.0 signed/unsigned integer-valued attributes.
 * Note that attribute indexes do NOT alias conventional attributes.
 */

static void GLAPIENTRY
TAG(VertexAttribI1iv)(GLuint index, const GLint *v)
{
   GET_CURRENT_CONTEXT(ctx);
   if (is_vertex_position(ctx, index))
      ATTR1I(0, v[0]);
   else if (index < MAX_VERTEX_GENERIC_ATTRIBS)
      ATTR1I(VBO_ATTRIB_GENERIC0 + index, v[0]);
   else
      ERROR(GL_INVALID_VALUE);
}

static void GLAPIENTRY
TAG(VertexAttribI1uiv)(GLuint index, const GLuint *v)
{
   GET_CURRENT_CONTEXT(ctx);
   if (is_vertex_position(ctx, index))
      ATTR1UI(0, v[0]);
   else if (index < MAX_VERTEX_GENERIC_ATTRIBS)
      ATTR1UI(VBO_ATTRIB_GENERIC0 + index, v[0]);
   else
      ERROR(GL_INVALID_VALUE);
}

static void GLAPIENTRY
TAG(VertexAttribI4bv)(GLuint index, const GLbyte *v)
{
   GET_CURRENT_CONTEXT(ctx);
   if (is_vertex_position(ctx, index))
      ATTR4I(0, v[0], v[1], v[2], v[3]);
   else if (index < MAX_VERTEX_GENERIC_ATTRIBS)
      ATTR4I(VBO_ATTRIB_GENERIC0 + index, v[0], v[1], v[2], v[3]);
   else
      ERROR(GL_INVALID_VALUE);
}

static void GLAPIENTRY
TAG(VertexAttribI4sv)(GLuint index, const GLshort *v)
{
   GET_CURRENT_CONTEXT(ctx);
   if (is_vertex_position(ctx, index))
      ATTR4I(0, v[0], v[1], v[2], v[3]);
   else if (index < MAX_VERTEX_GENERIC_ATTRIBS)
      ATTR4I(VBO_ATTRIB_GENERIC0 + index, v[0], v[1], v[2], v[3]);
   else
      ERROR(GL_INVALID_VALUE);
}

static void GLAPIENTRY
TAG(VertexAttribI4ubv)(GLuint index, const GLubyte *v)
{
   GET_CURRENT_CONTEXT(ctx);
   if (is_vertex_position(ctx, index))
      ATTR4UI(0, v[0], v[1], v[2], v[3]);
   else if (index < MAX_VERTEX_GENERIC_ATTRIBS)
      ATTR4UI(VBO_ATTRIB_GENERIC0 + index, v[0], v[1], v[2], v[3]);
   else
      ERROR(GL_INVALID_VALUE);
}

static void GLAPIENTRY
TAG(VertexAttribI4usv)(GLuint index, const GLushort *v)
{
   GET_CURRENT_CONTEXT(ctx);
   if (is_vertex_position(ctx, index))
      ATTR4UI(0, v[0], v[1], v[2], v[3]);
   else if (index < MAX_VERTEX_GENERIC_ATTRIBS)
      ATTR4UI(VBO_ATTRIB_GENERIC0 + index, v[0], v[1], v[2], v[3]);
   else
      ERROR(GL_INVALID_VALUE);
}

/* define this macro to exclude folllowing none-vertex functions */
#ifndef HW_SELECT_MODE

static void GLAPIENTRY
TAG(TexCoord1f)(GLfloat x)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR1F(VBO_ATTRIB_TEX0, x);
}

static void GLAPIENTRY
TAG(TexCoord1fv)(const GLfloat * v)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR1FV(VBO_ATTRIB_TEX0, v);
}

static void GLAPIENTRY
TAG(TexCoord2f)(GLfloat x, GLfloat y)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR2F(VBO_ATTRIB_TEX0, x, y);
}

static void GLAPIENTRY
TAG(TexCoord2fv)(const GLfloat * v)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR2FV(VBO_ATTRIB_TEX0, v);
}

static void GLAPIENTRY
TAG(TexCoord3f)(GLfloat x, GLfloat y, GLfloat z)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR3F(VBO_ATTRIB_TEX0, x, y, z);
}

static void GLAPIENTRY
TAG(TexCoord3fv)(const GLfloat * v)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR3FV(VBO_ATTRIB_TEX0, v);
}

static void GLAPIENTRY
TAG(TexCoord4f)(GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR4F(VBO_ATTRIB_TEX0, x, y, z, w);
}

static void GLAPIENTRY
TAG(TexCoord4fv)(const GLfloat * v)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR4FV(VBO_ATTRIB_TEX0, v);
}



static void GLAPIENTRY
TAG(Normal3f)(GLfloat x, GLfloat y, GLfloat z)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR3F(VBO_ATTRIB_NORMAL, x, y, z);
}

static void GLAPIENTRY
TAG(Normal3fv)(const GLfloat * v)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR3FV(VBO_ATTRIB_NORMAL, v);
}



static void GLAPIENTRY
TAG(FogCoordfEXT)(GLfloat x)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR1F(VBO_ATTRIB_FOG, x);
}



static void GLAPIENTRY
TAG(FogCoordfvEXT)(const GLfloat * v)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR1FV(VBO_ATTRIB_FOG, v);
}

static void GLAPIENTRY
TAG(Color3f)(GLfloat x, GLfloat y, GLfloat z)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR3F(VBO_ATTRIB_COLOR0, x, y, z);
}

static void GLAPIENTRY
TAG(Color3fv)(const GLfloat * v)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR3FV(VBO_ATTRIB_COLOR0, v);
}

static void GLAPIENTRY
TAG(Color4f)(GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR4F(VBO_ATTRIB_COLOR0, x, y, z, w);
}

static void GLAPIENTRY
TAG(Color4fv)(const GLfloat * v)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR4FV(VBO_ATTRIB_COLOR0, v);
}



static void GLAPIENTRY
TAG(SecondaryColor3fEXT)(GLfloat x, GLfloat y, GLfloat z)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR3F(VBO_ATTRIB_COLOR1, x, y, z);
}

static void GLAPIENTRY
TAG(SecondaryColor3fvEXT)(const GLfloat * v)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR3FV(VBO_ATTRIB_COLOR1, v);
}



static void GLAPIENTRY
TAG(EdgeFlag)(GLboolean b)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR1F(VBO_ATTRIB_EDGEFLAG, (GLfloat) b);
}



static void GLAPIENTRY
TAG(Indexf)(GLfloat f)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR1F(VBO_ATTRIB_COLOR_INDEX, f);
}

static void GLAPIENTRY
TAG(Indexfv)(const GLfloat * f)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR1FV(VBO_ATTRIB_COLOR_INDEX, f);
}



static void GLAPIENTRY
TAG(MultiTexCoord1fARB)(GLenum target, GLfloat x)
{
   GET_CURRENT_CONTEXT(ctx);
   GLuint attr = (target & 0x7) + VBO_ATTRIB_TEX0;
   ATTR1F(attr, x);
}

static void GLAPIENTRY
TAG(MultiTexCoord1fvARB)(GLenum target, const GLfloat * v)
{
   GET_CURRENT_CONTEXT(ctx);
   GLuint attr = (target & 0x7) + VBO_ATTRIB_TEX0;
   ATTR1FV(attr, v);
}

static void GLAPIENTRY
TAG(MultiTexCoord2fARB)(GLenum target, GLfloat x, GLfloat y)
{
   GET_CURRENT_CONTEXT(ctx);
   GLuint attr = (target & 0x7) + VBO_ATTRIB_TEX0;
   ATTR2F(attr, x, y);
}

static void GLAPIENTRY
TAG(MultiTexCoord2fvARB)(GLenum target, const GLfloat * v)
{
   GET_CURRENT_CONTEXT(ctx);
   GLuint attr = (target & 0x7) + VBO_ATTRIB_TEX0;
   ATTR2FV(attr, v);
}

static void GLAPIENTRY
TAG(MultiTexCoord3fARB)(GLenum target, GLfloat x, GLfloat y, GLfloat z)
{
   GET_CURRENT_CONTEXT(ctx);
   GLuint attr = (target & 0x7) + VBO_ATTRIB_TEX0;
   ATTR3F(attr, x, y, z);
}

static void GLAPIENTRY
TAG(MultiTexCoord3fvARB)(GLenum target, const GLfloat * v)
{
   GET_CURRENT_CONTEXT(ctx);
   GLuint attr = (target & 0x7) + VBO_ATTRIB_TEX0;
   ATTR3FV(attr, v);
}

static void GLAPIENTRY
TAG(MultiTexCoord4fARB)(GLenum target, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
   GET_CURRENT_CONTEXT(ctx);
   GLuint attr = (target & 0x7) + VBO_ATTRIB_TEX0;
   ATTR4F(attr, x, y, z, w);
}

static void GLAPIENTRY
TAG(MultiTexCoord4fvARB)(GLenum target, const GLfloat * v)
{
   GET_CURRENT_CONTEXT(ctx);
   GLuint attr = (target & 0x7) + VBO_ATTRIB_TEX0;
   ATTR4FV(attr, v);
}

static void GLAPIENTRY
TAG(TexCoordP1ui)(GLenum type, GLuint coords)
{
   GET_CURRENT_CONTEXT(ctx);
   ERROR_IF_NOT_PACKED_TYPE(ctx, type, "glTexCoordP1ui");
   ATTR_UI(ctx, 1, type, 0, VBO_ATTRIB_TEX0, coords);
}

static void GLAPIENTRY
TAG(TexCoordP1uiv)(GLenum type, const GLuint *coords)
{
   GET_CURRENT_CONTEXT(ctx);
   ERROR_IF_NOT_PACKED_TYPE(ctx, type, "glTexCoordP1uiv");
   ATTR_UI(ctx, 1, type, 0, VBO_ATTRIB_TEX0, coords[0]);
}

static void GLAPIENTRY
TAG(TexCoordP2ui)(GLenum type, GLuint coords)
{
   GET_CURRENT_CONTEXT(ctx);
   ERROR_IF_NOT_PACKED_TYPE(ctx, type, "glTexCoordP2ui");
   ATTR_UI(ctx, 2, type, 0, VBO_ATTRIB_TEX0, coords);
}

static void GLAPIENTRY
TAG(TexCoordP2uiv)(GLenum type, const GLuint *coords)
{
   GET_CURRENT_CONTEXT(ctx);
   ERROR_IF_NOT_PACKED_TYPE(ctx, type, "glTexCoordP2uiv");
   ATTR_UI(ctx, 2, type, 0, VBO_ATTRIB_TEX0, coords[0]);
}

static void GLAPIENTRY
TAG(TexCoordP3ui)(GLenum type, GLuint coords)
{
   GET_CURRENT_CONTEXT(ctx);
   ERROR_IF_NOT_PACKED_TYPE(ctx, type, "glTexCoordP3ui");
   ATTR_UI(ctx, 3, type, 0, VBO_ATTRIB_TEX0, coords);
}

static void GLAPIENTRY
TAG(TexCoordP3uiv)(GLenum type, const GLuint *coords)
{
   GET_CURRENT_CONTEXT(ctx);
   ERROR_IF_NOT_PACKED_TYPE(ctx, type, "glTexCoordP3uiv");
   ATTR_UI(ctx, 3, type, 0, VBO_ATTRIB_TEX0, coords[0]);
}

static void GLAPIENTRY
TAG(TexCoordP4ui)(GLenum type, GLuint coords)
{
   GET_CURRENT_CONTEXT(ctx);
   ERROR_IF_NOT_PACKED_TYPE(ctx, type, "glTexCoordP4ui");
   ATTR_UI(ctx, 4, type, 0, VBO_ATTRIB_TEX0, coords);
}

static void GLAPIENTRY
TAG(TexCoordP4uiv)(GLenum type, const GLuint *coords)
{
   GET_CURRENT_CONTEXT(ctx);
   ERROR_IF_NOT_PACKED_TYPE(ctx, type, "glTexCoordP4uiv");
   ATTR_UI(ctx, 4, type, 0, VBO_ATTRIB_TEX0, coords[0]);
}

static void GLAPIENTRY
TAG(MultiTexCoordP1ui)(GLenum target, GLenum type, GLuint coords)
{
   GET_CURRENT_CONTEXT(ctx);
   GLuint attr = (target & 0x7) + VBO_ATTRIB_TEX0;
   ERROR_IF_NOT_PACKED_TYPE(ctx, type, "glMultiTexCoordP1ui");
   ATTR_UI(ctx, 1, type, 0, attr, coords);
}

static void GLAPIENTRY
TAG(MultiTexCoordP1uiv)(GLenum target, GLenum type, const GLuint *coords)
{
   GET_CURRENT_CONTEXT(ctx);
   GLuint attr = (target & 0x7) + VBO_ATTRIB_TEX0;
   ERROR_IF_NOT_PACKED_TYPE(ctx, type, "glMultiTexCoordP1uiv");
   ATTR_UI(ctx, 1, type, 0, attr, coords[0]);
}

static void GLAPIENTRY
TAG(MultiTexCoordP2ui)(GLenum target, GLenum type, GLuint coords)
{
   GET_CURRENT_CONTEXT(ctx);
   GLuint attr = (target & 0x7) + VBO_ATTRIB_TEX0;
   ERROR_IF_NOT_PACKED_TYPE(ctx, type, "glMultiTexCoordP2ui");
   ATTR_UI(ctx, 2, type, 0, attr, coords);
}

static void GLAPIENTRY
TAG(MultiTexCoordP2uiv)(GLenum target, GLenum type, const GLuint *coords)
{
   GET_CURRENT_CONTEXT(ctx);
   GLuint attr = (target & 0x7) + VBO_ATTRIB_TEX0;
   ERROR_IF_NOT_PACKED_TYPE(ctx, type, "glMultiTexCoordP2uiv");
   ATTR_UI(ctx, 2, type, 0, attr, coords[0]);
}

static void GLAPIENTRY
TAG(MultiTexCoordP3ui)(GLenum target, GLenum type, GLuint coords)
{
   GET_CURRENT_CONTEXT(ctx);
   GLuint attr = (target & 0x7) + VBO_ATTRIB_TEX0;
   ERROR_IF_NOT_PACKED_TYPE(ctx, type, "glMultiTexCoordP3ui");
   ATTR_UI(ctx, 3, type, 0, attr, coords);
}

static void GLAPIENTRY
TAG(MultiTexCoordP3uiv)(GLenum target, GLenum type, const GLuint *coords)
{
   GET_CURRENT_CONTEXT(ctx);
   GLuint attr = (target & 0x7) + VBO_ATTRIB_TEX0;
   ERROR_IF_NOT_PACKED_TYPE(ctx, type, "glMultiTexCoordP3uiv");
   ATTR_UI(ctx, 3, type, 0, attr, coords[0]);
}

static void GLAPIENTRY
TAG(MultiTexCoordP4ui)(GLenum target, GLenum type, GLuint coords)
{
   GET_CURRENT_CONTEXT(ctx);
   GLuint attr = (target & 0x7) + VBO_ATTRIB_TEX0;
   ERROR_IF_NOT_PACKED_TYPE(ctx, type, "glMultiTexCoordP4ui");
   ATTR_UI(ctx, 4, type, 0, attr, coords);
}

static void GLAPIENTRY
TAG(MultiTexCoordP4uiv)(GLenum target, GLenum type, const GLuint *coords)
{
   GET_CURRENT_CONTEXT(ctx);
   GLuint attr = (target & 0x7) + VBO_ATTRIB_TEX0;
   ERROR_IF_NOT_PACKED_TYPE(ctx, type, "glMultiTexCoordP4uiv");
   ATTR_UI(ctx, 4, type, 0, attr, coords[0]);
}

static void GLAPIENTRY
TAG(NormalP3ui)(GLenum type, GLuint coords)
{
   GET_CURRENT_CONTEXT(ctx);
   ERROR_IF_NOT_PACKED_TYPE(ctx, type, "glNormalP3ui");
   ATTR_UI(ctx, 3, type, 1, VBO_ATTRIB_NORMAL, coords);
}

static void GLAPIENTRY
TAG(NormalP3uiv)(GLenum type, const GLuint *coords)
{
   GET_CURRENT_CONTEXT(ctx);
   ERROR_IF_NOT_PACKED_TYPE(ctx, type, "glNormalP3uiv");
   ATTR_UI(ctx, 3, type, 1, VBO_ATTRIB_NORMAL, coords[0]);
}

static void GLAPIENTRY
TAG(ColorP3ui)(GLenum type, GLuint color)
{
   GET_CURRENT_CONTEXT(ctx);
   ERROR_IF_NOT_PACKED_TYPE(ctx, type, "glColorP3ui");
   ATTR_UI(ctx, 3, type, 1, VBO_ATTRIB_COLOR0, color);
}

static void GLAPIENTRY
TAG(ColorP3uiv)(GLenum type, const GLuint *color)
{
   GET_CURRENT_CONTEXT(ctx);
   ERROR_IF_NOT_PACKED_TYPE(ctx, type, "glColorP3uiv");
   ATTR_UI(ctx, 3, type, 1, VBO_ATTRIB_COLOR0, color[0]);
}

static void GLAPIENTRY
TAG(ColorP4ui)(GLenum type, GLuint color)
{
   GET_CURRENT_CONTEXT(ctx);
   ERROR_IF_NOT_PACKED_TYPE(ctx, type, "glColorP4ui");
   ATTR_UI(ctx, 4, type, 1, VBO_ATTRIB_COLOR0, color);
}

static void GLAPIENTRY
TAG(ColorP4uiv)(GLenum type, const GLuint *color)
{
   GET_CURRENT_CONTEXT(ctx);
   ERROR_IF_NOT_PACKED_TYPE(ctx, type, "glColorP4uiv");
   ATTR_UI(ctx, 4, type, 1, VBO_ATTRIB_COLOR0, color[0]);
}

static void GLAPIENTRY
TAG(SecondaryColorP3ui)(GLenum type, GLuint color)
{
   GET_CURRENT_CONTEXT(ctx);
   ERROR_IF_NOT_PACKED_TYPE(ctx, type, "glSecondaryColorP3ui");
   ATTR_UI(ctx, 3, type, 1, VBO_ATTRIB_COLOR1, color);
}

static void GLAPIENTRY
TAG(SecondaryColorP3uiv)(GLenum type, const GLuint *color)
{
   GET_CURRENT_CONTEXT(ctx);
   ERROR_IF_NOT_PACKED_TYPE(ctx, type, "glSecondaryColorP3uiv");
   ATTR_UI(ctx, 3, type, 1, VBO_ATTRIB_COLOR1, color[0]);
}

static void GLAPIENTRY
TAG(Normal3hNV)(GLhalfNV x, GLhalfNV y, GLhalfNV z)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR3H(VBO_ATTRIB_NORMAL, x, y, z);
}

static void GLAPIENTRY
TAG(Normal3hvNV)(const GLhalfNV * v)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR3HV(VBO_ATTRIB_NORMAL, v);
}



static void GLAPIENTRY
TAG(Color3hNV)(GLhalfNV x, GLhalfNV y, GLhalfNV z)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR3H(VBO_ATTRIB_COLOR0, x, y, z);
}

static void GLAPIENTRY
TAG(Color3hvNV)(const GLhalfNV * v)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR3HV(VBO_ATTRIB_COLOR0, v);
}

static void GLAPIENTRY
TAG(Color4hNV)(GLhalfNV x, GLhalfNV y, GLhalfNV z, GLhalfNV w)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR4H(VBO_ATTRIB_COLOR0, x, y, z, w);
}

static void GLAPIENTRY
TAG(Color4hvNV)(const GLhalfNV * v)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR4HV(VBO_ATTRIB_COLOR0, v);
}



static void GLAPIENTRY
TAG(TexCoord1hNV)(GLhalfNV x)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR1H(VBO_ATTRIB_TEX0, x);
}

static void GLAPIENTRY
TAG(TexCoord1hvNV)(const GLhalfNV * v)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR1HV(VBO_ATTRIB_TEX0, v);
}

static void GLAPIENTRY
TAG(TexCoord2hNV)(GLhalfNV x, GLhalfNV y)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR2H(VBO_ATTRIB_TEX0, x, y);
}

static void GLAPIENTRY
TAG(TexCoord2hvNV)(const GLhalfNV * v)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR2HV(VBO_ATTRIB_TEX0, v);
}

static void GLAPIENTRY
TAG(TexCoord3hNV)(GLhalfNV x, GLhalfNV y, GLhalfNV z)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR3H(VBO_ATTRIB_TEX0, x, y, z);
}

static void GLAPIENTRY
TAG(TexCoord3hvNV)(const GLhalfNV * v)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR3HV(VBO_ATTRIB_TEX0, v);
}

static void GLAPIENTRY
TAG(TexCoord4hNV)(GLhalfNV x, GLhalfNV y, GLhalfNV z, GLhalfNV w)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR4H(VBO_ATTRIB_TEX0, x, y, z, w);
}

static void GLAPIENTRY
TAG(TexCoord4hvNV)(const GLhalfNV * v)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR4HV(VBO_ATTRIB_TEX0, v);
}



static void GLAPIENTRY
TAG(MultiTexCoord1hNV)(GLenum target, GLhalfNV x)
{
   GET_CURRENT_CONTEXT(ctx);
   GLuint attr = (target & 0x7) + VBO_ATTRIB_TEX0;
   ATTR1H(attr, x);
}

static void GLAPIENTRY
TAG(MultiTexCoord1hvNV)(GLenum target, const GLhalfNV * v)
{
   GET_CURRENT_CONTEXT(ctx);
   GLuint attr = (target & 0x7) + VBO_ATTRIB_TEX0;
   ATTR1HV(attr, v);
}

static void GLAPIENTRY
TAG(MultiTexCoord2hNV)(GLenum target, GLhalfNV x, GLhalfNV y)
{
   GET_CURRENT_CONTEXT(ctx);
   GLuint attr = (target & 0x7) + VBO_ATTRIB_TEX0;
   ATTR2H(attr, x, y);
}

static void GLAPIENTRY
TAG(MultiTexCoord2hvNV)(GLenum target, const GLhalfNV * v)
{
   GET_CURRENT_CONTEXT(ctx);
   GLuint attr = (target & 0x7) + VBO_ATTRIB_TEX0;
   ATTR2HV(attr, v);
}

static void GLAPIENTRY
TAG(MultiTexCoord3hNV)(GLenum target, GLhalfNV x, GLhalfNV y, GLhalfNV z)
{
   GET_CURRENT_CONTEXT(ctx);
   GLuint attr = (target & 0x7) + VBO_ATTRIB_TEX0;
   ATTR3H(attr, x, y, z);
}

static void GLAPIENTRY
TAG(MultiTexCoord3hvNV)(GLenum target, const GLhalfNV * v)
{
   GET_CURRENT_CONTEXT(ctx);
   GLuint attr = (target & 0x7) + VBO_ATTRIB_TEX0;
   ATTR3HV(attr, v);
}

static void GLAPIENTRY
TAG(MultiTexCoord4hNV)(GLenum target, GLhalfNV x, GLhalfNV y, GLhalfNV z, GLhalfNV w)
{
   GET_CURRENT_CONTEXT(ctx);
   GLuint attr = (target & 0x7) + VBO_ATTRIB_TEX0;
   ATTR4H(attr, x, y, z, w);
}

static void GLAPIENTRY
TAG(MultiTexCoord4hvNV)(GLenum target, const GLhalfNV * v)
{
   GET_CURRENT_CONTEXT(ctx);
   GLuint attr = (target & 0x7) + VBO_ATTRIB_TEX0;
   ATTR4HV(attr, v);
}


static void GLAPIENTRY
TAG(FogCoordhNV)(GLhalf x)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR1H(VBO_ATTRIB_FOG, x);
}

static void GLAPIENTRY
TAG(FogCoordhvNV)(const GLhalf * v)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR1HV(VBO_ATTRIB_FOG, v);
}



static void GLAPIENTRY
TAG(SecondaryColor3hNV)(GLhalfNV x, GLhalfNV y, GLhalfNV z)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR3H(VBO_ATTRIB_COLOR1, x, y, z);
}

static void GLAPIENTRY
TAG(SecondaryColor3hvNV)(const GLhalfNV * v)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR3HV(VBO_ATTRIB_COLOR1, v);
}


static void GLAPIENTRY
TAG(Color3b)(GLbyte red, GLbyte green, GLbyte blue)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR4F(VBO_ATTRIB_COLOR0, BYTE_TO_FLOAT(red),
          BYTE_TO_FLOAT(green),
          BYTE_TO_FLOAT(blue),
          1.0);
}

static void GLAPIENTRY
TAG(Color3d)(GLdouble red, GLdouble green, GLdouble blue)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR4F(VBO_ATTRIB_COLOR0, (GLfloat) red, (GLfloat) green, (GLfloat) blue, 1.0);
}

static void GLAPIENTRY
TAG(Color3i)(GLint red, GLint green, GLint blue)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR4F(VBO_ATTRIB_COLOR0, INT_TO_FLOAT(red), INT_TO_FLOAT(green),
          INT_TO_FLOAT(blue), 1.0);
}

static void GLAPIENTRY
TAG(Color3s)(GLshort red, GLshort green, GLshort blue)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR4F(VBO_ATTRIB_COLOR0, SHORT_TO_FLOAT(red), SHORT_TO_FLOAT(green),
          SHORT_TO_FLOAT(blue), 1.0);
}

static void GLAPIENTRY
TAG(Color3ui)(GLuint red, GLuint green, GLuint blue)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR4F(VBO_ATTRIB_COLOR0, UINT_TO_FLOAT(red), UINT_TO_FLOAT(green),
          UINT_TO_FLOAT(blue), 1.0);
}

static void GLAPIENTRY
TAG(Color3us)(GLushort red, GLushort green, GLushort blue)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR4F(VBO_ATTRIB_COLOR0, USHORT_TO_FLOAT(red), USHORT_TO_FLOAT(green),
          USHORT_TO_FLOAT(blue), 1.0);
}

static void GLAPIENTRY
TAG(Color3ub)(GLubyte red, GLubyte green, GLubyte blue)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR4F(VBO_ATTRIB_COLOR0, UBYTE_TO_FLOAT(red), UBYTE_TO_FLOAT(green),
          UBYTE_TO_FLOAT(blue), 1.0);
}


static void GLAPIENTRY
TAG(Color3bv)(const GLbyte *v)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR4F(VBO_ATTRIB_COLOR0, BYTE_TO_FLOAT(v[0]), BYTE_TO_FLOAT(v[1]),
         BYTE_TO_FLOAT(v[2]), 1.0);
}

static void GLAPIENTRY
TAG(Color3dv)(const GLdouble *v)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR4F(VBO_ATTRIB_COLOR0, (GLfloat) v[0], (GLfloat) v[1], (GLfloat) v[2], 1.0);
}

static void GLAPIENTRY
TAG(Color3iv)(const GLint *v)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR4F(VBO_ATTRIB_COLOR0, INT_TO_FLOAT(v[0]), INT_TO_FLOAT(v[1]),
         INT_TO_FLOAT(v[2]), 1.0);
}

static void GLAPIENTRY
TAG(Color3sv)(const GLshort *v)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR4F(VBO_ATTRIB_COLOR0, SHORT_TO_FLOAT(v[0]), SHORT_TO_FLOAT(v[1]),
         SHORT_TO_FLOAT(v[2]), 1.0);
}

static void GLAPIENTRY
TAG(Color3uiv)(const GLuint *v)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR4F(VBO_ATTRIB_COLOR0, UINT_TO_FLOAT(v[0]), UINT_TO_FLOAT(v[1]),
         UINT_TO_FLOAT(v[2]), 1.0);
}

static void GLAPIENTRY
TAG(Color3usv)(const GLushort *v)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR4F(VBO_ATTRIB_COLOR0, USHORT_TO_FLOAT(v[0]), USHORT_TO_FLOAT(v[1]),
         USHORT_TO_FLOAT(v[2]), 1.0);
}

static void GLAPIENTRY
TAG(Color3ubv)(const GLubyte *v)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR4F(VBO_ATTRIB_COLOR0, UBYTE_TO_FLOAT(v[0]), UBYTE_TO_FLOAT(v[1]),
         UBYTE_TO_FLOAT(v[2]), 1.0);
}


static void GLAPIENTRY
TAG(Color4b)(GLbyte red, GLbyte green, GLbyte blue,
              GLbyte alpha)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR4F(VBO_ATTRIB_COLOR0, BYTE_TO_FLOAT(red), BYTE_TO_FLOAT(green),
          BYTE_TO_FLOAT(blue), BYTE_TO_FLOAT(alpha));
}

static void GLAPIENTRY
TAG(Color4d)(GLdouble red, GLdouble green, GLdouble blue,
              GLdouble alpha)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR4F(VBO_ATTRIB_COLOR0, (GLfloat) red, (GLfloat) green, (GLfloat) blue, (GLfloat) alpha);
}

static void GLAPIENTRY
TAG(Color4i)(GLint red, GLint green, GLint blue, GLint alpha)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR4F(VBO_ATTRIB_COLOR0, INT_TO_FLOAT(red), INT_TO_FLOAT(green),
          INT_TO_FLOAT(blue), INT_TO_FLOAT(alpha));
}

static void GLAPIENTRY
TAG(Color4s)(GLshort red, GLshort green, GLshort blue,
              GLshort alpha)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR4F(VBO_ATTRIB_COLOR0, SHORT_TO_FLOAT(red), SHORT_TO_FLOAT(green),
          SHORT_TO_FLOAT(blue), SHORT_TO_FLOAT(alpha));
}

static void GLAPIENTRY
TAG(Color4ui)(GLuint red, GLuint green, GLuint blue, GLuint alpha)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR4F(VBO_ATTRIB_COLOR0, UINT_TO_FLOAT(red), UINT_TO_FLOAT(green),
          UINT_TO_FLOAT(blue), UINT_TO_FLOAT(alpha));
}

static void GLAPIENTRY
TAG(Color4us)(GLushort red, GLushort green, GLushort blue, GLushort alpha)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR4F(VBO_ATTRIB_COLOR0, USHORT_TO_FLOAT(red), USHORT_TO_FLOAT(green),
          USHORT_TO_FLOAT(blue), USHORT_TO_FLOAT(alpha));
}

static void GLAPIENTRY
TAG(Color4ub)(GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR4F(VBO_ATTRIB_COLOR0, UBYTE_TO_FLOAT(red), UBYTE_TO_FLOAT(green),
          UBYTE_TO_FLOAT(blue), UBYTE_TO_FLOAT(alpha));
}


static void GLAPIENTRY
TAG(Color4iv)(const GLint *v)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR4F(VBO_ATTRIB_COLOR0, INT_TO_FLOAT(v[0]), INT_TO_FLOAT(v[1]),
         INT_TO_FLOAT(v[2]), INT_TO_FLOAT(v[3]));
}


static void GLAPIENTRY
TAG(Color4bv)(const GLbyte *v)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR4F(VBO_ATTRIB_COLOR0, BYTE_TO_FLOAT(v[0]), BYTE_TO_FLOAT(v[1]),
         BYTE_TO_FLOAT(v[2]), BYTE_TO_FLOAT(v[3]));
}

static void GLAPIENTRY
TAG(Color4dv)(const GLdouble *v)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR4F(VBO_ATTRIB_COLOR0, (GLfloat) v[0], (GLfloat) v[1], (GLfloat) v[2], (GLfloat) v[3]);
}


static void GLAPIENTRY
TAG(Color4sv)(const GLshort *v)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR4F(VBO_ATTRIB_COLOR0, SHORT_TO_FLOAT(v[0]), SHORT_TO_FLOAT(v[1]),
         SHORT_TO_FLOAT(v[2]), SHORT_TO_FLOAT(v[3]));
}


static void GLAPIENTRY
TAG(Color4uiv)(const GLuint *v)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR4F(VBO_ATTRIB_COLOR0, UINT_TO_FLOAT(v[0]), UINT_TO_FLOAT(v[1]),
         UINT_TO_FLOAT(v[2]), UINT_TO_FLOAT(v[3]));
}

static void GLAPIENTRY
TAG(Color4usv)(const GLushort *v)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR4F(VBO_ATTRIB_COLOR0, USHORT_TO_FLOAT(v[0]), USHORT_TO_FLOAT(v[1]),
         USHORT_TO_FLOAT(v[2]), USHORT_TO_FLOAT(v[3]));
}

static void GLAPIENTRY
TAG(Color4ubv)(const GLubyte *v)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR4F(VBO_ATTRIB_COLOR0, UBYTE_TO_FLOAT(v[0]), UBYTE_TO_FLOAT(v[1]),
         UBYTE_TO_FLOAT(v[2]), UBYTE_TO_FLOAT(v[3]));
}


static void GLAPIENTRY
TAG(FogCoordd)(GLdouble d)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR1F(VBO_ATTRIB_FOG, (GLfloat) d);
}

static void GLAPIENTRY
TAG(FogCoorddv)(const GLdouble *v)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR1F(VBO_ATTRIB_FOG, (GLfloat) *v);
}


static void GLAPIENTRY
TAG(Indexd)(GLdouble c)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR1F(VBO_ATTRIB_COLOR_INDEX, (GLfloat) c);
}

static void GLAPIENTRY
TAG(Indexi)(GLint c)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR1F(VBO_ATTRIB_COLOR_INDEX, (GLfloat) c);
}

static void GLAPIENTRY
TAG(Indexs)(GLshort c)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR1F(VBO_ATTRIB_COLOR_INDEX, (GLfloat) c);
}

static void GLAPIENTRY
TAG(Indexub)(GLubyte c)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR1F(VBO_ATTRIB_COLOR_INDEX, (GLfloat) c);
}

static void GLAPIENTRY
TAG(Indexdv)(const GLdouble *c)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR1F(VBO_ATTRIB_COLOR_INDEX, (GLfloat) *c);
}

static void GLAPIENTRY
TAG(Indexiv)(const GLint *c)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR1F(VBO_ATTRIB_COLOR_INDEX, (GLfloat) *c);
}

static void GLAPIENTRY
TAG(Indexsv)(const GLshort *c)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR1F(VBO_ATTRIB_COLOR_INDEX, (GLfloat) *c);
}

static void GLAPIENTRY
TAG(Indexubv)(const GLubyte *c)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR1F(VBO_ATTRIB_COLOR_INDEX, (GLfloat) *c);
}


static void GLAPIENTRY
TAG(EdgeFlagv)(const GLboolean *flag)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR1F(VBO_ATTRIB_EDGEFLAG, (GLfloat)*flag);
}


static void GLAPIENTRY
TAG(Normal3b)(GLbyte nx, GLbyte ny, GLbyte nz)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR3F(VBO_ATTRIB_NORMAL, BYTE_TO_FLOAT(nx), BYTE_TO_FLOAT(ny), BYTE_TO_FLOAT(nz));
}

static void GLAPIENTRY
TAG(Normal3d)(GLdouble nx, GLdouble ny, GLdouble nz)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR3F(VBO_ATTRIB_NORMAL, (GLfloat) nx, (GLfloat) ny, (GLfloat) nz);
}

static void GLAPIENTRY
TAG(Normal3i)(GLint nx, GLint ny, GLint nz)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR3F(VBO_ATTRIB_NORMAL, INT_TO_FLOAT(nx), INT_TO_FLOAT(ny), INT_TO_FLOAT(nz));
}

static void GLAPIENTRY
TAG(Normal3s)(GLshort nx, GLshort ny, GLshort nz)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR3F(VBO_ATTRIB_NORMAL, SHORT_TO_FLOAT(nx), SHORT_TO_FLOAT(ny), SHORT_TO_FLOAT(nz));
}

static void GLAPIENTRY
TAG(Normal3bv)(const GLbyte *v)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR3F(VBO_ATTRIB_NORMAL, BYTE_TO_FLOAT(v[0]), BYTE_TO_FLOAT(v[1]), BYTE_TO_FLOAT(v[2]));
}

static void GLAPIENTRY
TAG(Normal3dv)(const GLdouble *v)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR3F(VBO_ATTRIB_NORMAL, (GLfloat) v[0], (GLfloat) v[1], (GLfloat) v[2]);
}

static void GLAPIENTRY
TAG(Normal3iv)(const GLint *v)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR3F(VBO_ATTRIB_NORMAL, INT_TO_FLOAT(v[0]), INT_TO_FLOAT(v[1]), INT_TO_FLOAT(v[2]));
}

static void GLAPIENTRY
TAG(Normal3sv)(const GLshort *v)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR3F(VBO_ATTRIB_NORMAL, SHORT_TO_FLOAT(v[0]), SHORT_TO_FLOAT(v[1]), SHORT_TO_FLOAT(v[2]));
}

static void GLAPIENTRY
TAG(TexCoord1d)(GLdouble s)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR1F(VBO_ATTRIB_TEX0, (GLfloat) s);
}

static void GLAPIENTRY
TAG(TexCoord1i)(GLint s)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR1F(VBO_ATTRIB_TEX0, (GLfloat) s);
}

static void GLAPIENTRY
TAG(TexCoord1s)(GLshort s)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR1F(VBO_ATTRIB_TEX0, (GLfloat) s);
}

static void GLAPIENTRY
TAG(TexCoord2d)(GLdouble s, GLdouble t)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR2F(VBO_ATTRIB_TEX0, (GLfloat) s,(GLfloat) t);
}

static void GLAPIENTRY
TAG(TexCoord2s)(GLshort s, GLshort t)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR2F(VBO_ATTRIB_TEX0, (GLfloat) s,(GLfloat) t);
}

static void GLAPIENTRY
TAG(TexCoord2i)(GLint s, GLint t)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR2F(VBO_ATTRIB_TEX0, (GLfloat) s,(GLfloat) t);
}

static void GLAPIENTRY
TAG(TexCoord3d)(GLdouble s, GLdouble t, GLdouble r)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR3F(VBO_ATTRIB_TEX0, (GLfloat) s,(GLfloat) t,(GLfloat) r);
}

static void GLAPIENTRY
TAG(TexCoord3i)(GLint s, GLint t, GLint r)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR3F(VBO_ATTRIB_TEX0, (GLfloat) s,(GLfloat) t,(GLfloat) r);
}

static void GLAPIENTRY
TAG(TexCoord3s)(GLshort s, GLshort t, GLshort r)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR3F(VBO_ATTRIB_TEX0, (GLfloat) s,(GLfloat) t,(GLfloat) r);
}

static void GLAPIENTRY
TAG(TexCoord4d)(GLdouble s, GLdouble t, GLdouble r, GLdouble q)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR4F(VBO_ATTRIB_TEX0, (GLfloat) s,(GLfloat) t,(GLfloat) r,(GLfloat) q);
}

static void GLAPIENTRY
TAG(TexCoord4i)(GLint s, GLint t, GLint r, GLint q)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR4F(VBO_ATTRIB_TEX0, (GLfloat) s,(GLfloat) t,(GLfloat) r,(GLfloat) q);
}

static void GLAPIENTRY
TAG(TexCoord4s)(GLshort s, GLshort t, GLshort r, GLshort q)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR4F(VBO_ATTRIB_TEX0, (GLfloat) s,(GLfloat) t,(GLfloat) r,(GLfloat) q);
}

static void GLAPIENTRY
TAG(TexCoord1dv)(const GLdouble *v)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR1F(VBO_ATTRIB_TEX0, (GLfloat) v[0]);
}

static void GLAPIENTRY
TAG(TexCoord1iv)(const GLint *v)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR1F(VBO_ATTRIB_TEX0, (GLfloat) v[0]);
}

static void GLAPIENTRY
TAG(TexCoord1sv)(const GLshort *v)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR1F(VBO_ATTRIB_TEX0, (GLfloat) v[0]);
}

static void GLAPIENTRY
TAG(TexCoord2dv)(const GLdouble *v)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR2F(VBO_ATTRIB_TEX0, (GLfloat) v[0],(GLfloat) v[1]);
}

static void GLAPIENTRY
TAG(TexCoord2iv)(const GLint *v)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR2F(VBO_ATTRIB_TEX0, (GLfloat) v[0],(GLfloat) v[1]);
}

static void GLAPIENTRY
TAG(TexCoord2sv)(const GLshort *v)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR2F(VBO_ATTRIB_TEX0, (GLfloat) v[0],(GLfloat) v[1]);
}

static void GLAPIENTRY
TAG(TexCoord3dv)(const GLdouble *v)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR3F(VBO_ATTRIB_TEX0, (GLfloat) v[0],(GLfloat) v[1],(GLfloat) v[2]);
}

static void GLAPIENTRY
TAG(TexCoord3iv)(const GLint *v)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR3F(VBO_ATTRIB_TEX0, (GLfloat) v[0],(GLfloat) v[1],(GLfloat) v[2]);
}

static void GLAPIENTRY
TAG(TexCoord3sv)(const GLshort *v)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR3F(VBO_ATTRIB_TEX0, (GLfloat) v[0],(GLfloat) v[1],(GLfloat) v[2]);
}

static void GLAPIENTRY
TAG(TexCoord4dv)(const GLdouble *v)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR4F(VBO_ATTRIB_TEX0, (GLfloat) v[0],(GLfloat) v[1],(GLfloat) v[2],(GLfloat) v[3]);
}

static void GLAPIENTRY
TAG(TexCoord4iv)(const GLint *v)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR4F(VBO_ATTRIB_TEX0, (GLfloat) v[0],(GLfloat) v[1],(GLfloat) v[2],(GLfloat) v[3]);
}

static void GLAPIENTRY
TAG(TexCoord4sv)(const GLshort *v)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR4F(VBO_ATTRIB_TEX0, (GLfloat) v[0],(GLfloat) v[1],(GLfloat) v[2],(GLfloat) v[3]);
}

static void GLAPIENTRY
TAG(MultiTexCoord1d)(GLenum target, GLdouble s)
{
   GET_CURRENT_CONTEXT(ctx);
   GLuint attr = (target & 0x7) + VBO_ATTRIB_TEX0;
   ATTR1F(attr, (GLfloat) s);
}

static void GLAPIENTRY
TAG(MultiTexCoord1dv)(GLenum target, const GLdouble *v)
{
   GET_CURRENT_CONTEXT(ctx);
   GLuint attr = (target & 0x7) + VBO_ATTRIB_TEX0;
   ATTR1F(attr, (GLfloat) v[0]);
}

static void GLAPIENTRY
TAG(MultiTexCoord1i)(GLenum target, GLint s)
{
   GET_CURRENT_CONTEXT(ctx);
   GLuint attr = (target & 0x7) + VBO_ATTRIB_TEX0;
   ATTR1F(attr, (GLfloat) s);
}

static void GLAPIENTRY
TAG(MultiTexCoord1iv)(GLenum target, const GLint *v)
{
   GET_CURRENT_CONTEXT(ctx);
   GLuint attr = (target & 0x7) + VBO_ATTRIB_TEX0;
   ATTR1F(attr, (GLfloat) v[0]);
}

static void GLAPIENTRY
TAG(MultiTexCoord1s)(GLenum target, GLshort s)
{
   GET_CURRENT_CONTEXT(ctx);
   GLuint attr = (target & 0x7) + VBO_ATTRIB_TEX0;
   ATTR1F(attr, (GLfloat) s);
}

static void GLAPIENTRY
TAG(MultiTexCoord1sv)(GLenum target, const GLshort *v)
{
   GET_CURRENT_CONTEXT(ctx);
   GLuint attr = (target & 0x7) + VBO_ATTRIB_TEX0;
   ATTR1F(attr, (GLfloat) v[0]);
}

static void GLAPIENTRY
TAG(MultiTexCoord2d)(GLenum target, GLdouble s, GLdouble t)
{
   GET_CURRENT_CONTEXT(ctx);
   GLuint attr = (target & 0x7) + VBO_ATTRIB_TEX0;
   ATTR2F(attr, (GLfloat) s, (GLfloat) t);
}

static void GLAPIENTRY
TAG(MultiTexCoord2dv)(GLenum target, const GLdouble *v)
{
   GET_CURRENT_CONTEXT(ctx);
   GLuint attr = (target & 0x7) + VBO_ATTRIB_TEX0;
   ATTR2F(attr, (GLfloat) v[0], (GLfloat) v[1]);
}

static void GLAPIENTRY
TAG(MultiTexCoord2i)(GLenum target, GLint s, GLint t)
{
   GET_CURRENT_CONTEXT(ctx);
   GLuint attr = (target & 0x7) + VBO_ATTRIB_TEX0;
   ATTR2F(attr, (GLfloat) s, (GLfloat) t);
}

static void GLAPIENTRY
TAG(MultiTexCoord2iv)(GLenum target, const GLint *v)
{
   GET_CURRENT_CONTEXT(ctx);
   GLuint attr = (target & 0x7) + VBO_ATTRIB_TEX0;
   ATTR2F(attr, (GLfloat) v[0], (GLfloat) v[1]);
}

static void GLAPIENTRY
TAG(MultiTexCoord2s)(GLenum target, GLshort s, GLshort t)
{
   GET_CURRENT_CONTEXT(ctx);
   GLuint attr = (target & 0x7) + VBO_ATTRIB_TEX0;
   ATTR2F(attr, (GLfloat) s, (GLfloat) t);
}

static void GLAPIENTRY
TAG(MultiTexCoord2sv)(GLenum target, const GLshort *v)
{
   GET_CURRENT_CONTEXT(ctx);
   GLuint attr = (target & 0x7) + VBO_ATTRIB_TEX0;
   ATTR2F(attr, (GLfloat) v[0], (GLfloat) v[1]);
}

static void GLAPIENTRY
TAG(MultiTexCoord3d)(GLenum target, GLdouble s, GLdouble t, GLdouble r)
{
   GET_CURRENT_CONTEXT(ctx);
   GLuint attr = (target & 0x7) + VBO_ATTRIB_TEX0;
   ATTR3F(attr, (GLfloat) s, (GLfloat) t, (GLfloat) r);
}

static void GLAPIENTRY
TAG(MultiTexCoord3dv)(GLenum target, const GLdouble *v)
{
   GET_CURRENT_CONTEXT(ctx);
   GLuint attr = (target & 0x7) + VBO_ATTRIB_TEX0;
   ATTR3F(attr, (GLfloat) v[0], (GLfloat) v[1], (GLfloat) v[2]);
}

static void GLAPIENTRY
TAG(MultiTexCoord3i)(GLenum target, GLint s, GLint t, GLint r)
{
   GET_CURRENT_CONTEXT(ctx);
   GLuint attr = (target & 0x7) + VBO_ATTRIB_TEX0;
   ATTR3F(attr, (GLfloat) s, (GLfloat) t, (GLfloat) r);
}

static void GLAPIENTRY
TAG(MultiTexCoord3iv)(GLenum target, const GLint *v)
{
   GET_CURRENT_CONTEXT(ctx);
   GLuint attr = (target & 0x7) + VBO_ATTRIB_TEX0;
   ATTR3F(attr, (GLfloat) v[0], (GLfloat) v[1], (GLfloat) v[2]);
}

static void GLAPIENTRY
TAG(MultiTexCoord3s)(GLenum target, GLshort s, GLshort t, GLshort r)
{
   GET_CURRENT_CONTEXT(ctx);
   GLuint attr = (target & 0x7) + VBO_ATTRIB_TEX0;
   ATTR3F(attr, (GLfloat) s, (GLfloat) t, (GLfloat) r);
}

static void GLAPIENTRY
TAG(MultiTexCoord3sv)(GLenum target, const GLshort *v)
{
   GET_CURRENT_CONTEXT(ctx);
   GLuint attr = (target & 0x7) + VBO_ATTRIB_TEX0;
   ATTR3F(attr, (GLfloat) v[0], (GLfloat) v[1], (GLfloat) v[2]);
}

static void GLAPIENTRY
TAG(MultiTexCoord4d)(GLenum target, GLdouble s, GLdouble t, GLdouble r, GLdouble q)
{
   GET_CURRENT_CONTEXT(ctx);
   GLuint attr = (target & 0x7) + VBO_ATTRIB_TEX0;
   ATTR4F(attr, (GLfloat) s, (GLfloat) t, (GLfloat) r, (GLfloat) q);
}

static void GLAPIENTRY
TAG(MultiTexCoord4dv)(GLenum target, const GLdouble *v)
{
   GET_CURRENT_CONTEXT(ctx);
   GLuint attr = (target & 0x7) + VBO_ATTRIB_TEX0;
   ATTR4F(attr, (GLfloat) v[0], (GLfloat) v[1], (GLfloat) v[2], (GLfloat) v[3]);
}

static void GLAPIENTRY
TAG(MultiTexCoord4i)(GLenum target, GLint s, GLint t, GLint r, GLint q)
{
   GET_CURRENT_CONTEXT(ctx);
   GLuint attr = (target & 0x7) + VBO_ATTRIB_TEX0;
   ATTR4F(attr, (GLfloat) s, (GLfloat) t, (GLfloat) r, (GLfloat) q);
}

static void GLAPIENTRY
TAG(MultiTexCoord4iv)(GLenum target, const GLint *v)
{
   GET_CURRENT_CONTEXT(ctx);
   GLuint attr = (target & 0x7) + VBO_ATTRIB_TEX0;
   ATTR4F(attr, (GLfloat) v[0], (GLfloat) v[1], (GLfloat) v[2], (GLfloat) v[3]);
}

static void GLAPIENTRY
TAG(MultiTexCoord4s)(GLenum target, GLshort s, GLshort t, GLshort r, GLshort q)
{
   GET_CURRENT_CONTEXT(ctx);
   GLuint attr = (target & 0x7) + VBO_ATTRIB_TEX0;
   ATTR4F(attr, (GLfloat) s, (GLfloat) t, (GLfloat) r, (GLfloat) q);
}

static void GLAPIENTRY
TAG(MultiTexCoord4sv)(GLenum target, const GLshort *v)
{
   GET_CURRENT_CONTEXT(ctx);
   GLuint attr = (target & 0x7) + VBO_ATTRIB_TEX0;
   ATTR4F(attr, (GLfloat) v[0], (GLfloat) v[1], (GLfloat) v[2], (GLfloat) v[3]);
}


static void GLAPIENTRY
TAG(EvalCoord2dv)(const GLdouble *u)
{
   TAG(EvalCoord2f)((GLfloat) u[0], (GLfloat) u[1]);
}

static void GLAPIENTRY
TAG(EvalCoord2d)(GLdouble u, GLdouble v)
{
   TAG(EvalCoord2f)((GLfloat) u, (GLfloat) v);
}

static void GLAPIENTRY
TAG(EvalCoord1dv)(const GLdouble *u)
{
   TAG(EvalCoord1f)((GLfloat) *u);
}

static void GLAPIENTRY
TAG(EvalCoord1d)(GLdouble u)
{
   TAG(EvalCoord1f)((GLfloat) u);
}


static void GLAPIENTRY
TAG(Materialf)(GLenum face, GLenum pname, GLfloat param)
{
   GLfloat fparam[4];
   fparam[0] = param;
   TAG(Materialfv)(face, pname, fparam);
}

static void GLAPIENTRY
TAG(Materiali)(GLenum face, GLenum pname, GLint param)
{
   GLfloat p[4];
   p[0] = (GLfloat) param;
   TAG(Materialfv)(face, pname, p);
}

static void GLAPIENTRY
TAG(Materialiv)(GLenum face, GLenum pname, const GLint *params)
{
   GLfloat fparam[4];
   switch (pname) {
   case GL_AMBIENT:
   case GL_DIFFUSE:
   case GL_SPECULAR:
   case GL_EMISSION:
   case GL_AMBIENT_AND_DIFFUSE:
      fparam[0] = INT_TO_FLOAT(params[0]);
      fparam[1] = INT_TO_FLOAT(params[1]);
      fparam[2] = INT_TO_FLOAT(params[2]);
      fparam[3] = INT_TO_FLOAT(params[3]);
      break;
   case GL_SHININESS:
      fparam[0] = (GLfloat) params[0];
      break;
   case GL_COLOR_INDEXES:
      fparam[0] = (GLfloat) params[0];
      fparam[1] = (GLfloat) params[1];
      fparam[2] = (GLfloat) params[2];
      break;
   default:
      ;
   }
   TAG(Materialfv)(face, pname, fparam);
}


static void GLAPIENTRY
TAG(SecondaryColor3b)(GLbyte red, GLbyte green, GLbyte blue)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR3F(VBO_ATTRIB_COLOR1, BYTE_TO_FLOAT(red),
          BYTE_TO_FLOAT(green),
          BYTE_TO_FLOAT(blue));
}

static void GLAPIENTRY
TAG(SecondaryColor3d)(GLdouble red, GLdouble green, GLdouble blue)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR3F(VBO_ATTRIB_COLOR1, (GLfloat) red, (GLfloat) green, (GLfloat) blue);
}

static void GLAPIENTRY
TAG(SecondaryColor3i)(GLint red, GLint green, GLint blue)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR3F(VBO_ATTRIB_COLOR1, INT_TO_FLOAT(red),
          INT_TO_FLOAT(green),
          INT_TO_FLOAT(blue));
}

static void GLAPIENTRY
TAG(SecondaryColor3s)(GLshort red, GLshort green, GLshort blue)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR3F(VBO_ATTRIB_COLOR1, SHORT_TO_FLOAT(red),
          SHORT_TO_FLOAT(green),
          SHORT_TO_FLOAT(blue));
}

static void GLAPIENTRY
TAG(SecondaryColor3ui)(GLuint red, GLuint green, GLuint blue)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR3F(VBO_ATTRIB_COLOR1, UINT_TO_FLOAT(red),
          UINT_TO_FLOAT(green),
          UINT_TO_FLOAT(blue));
}

static void GLAPIENTRY
TAG(SecondaryColor3us)(GLushort red, GLushort green, GLushort blue)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR3F(VBO_ATTRIB_COLOR1, USHORT_TO_FLOAT(red),
          USHORT_TO_FLOAT(green),
          USHORT_TO_FLOAT(blue));
}

static void GLAPIENTRY
TAG(SecondaryColor3ub)(GLubyte red, GLubyte green, GLubyte blue)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR3F(VBO_ATTRIB_COLOR1, UBYTE_TO_FLOAT(red),
          UBYTE_TO_FLOAT(green),
          UBYTE_TO_FLOAT(blue));
}

static void GLAPIENTRY
TAG(SecondaryColor3bv)(const GLbyte *v)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR3F(VBO_ATTRIB_COLOR1, BYTE_TO_FLOAT(v[0]),
         BYTE_TO_FLOAT(v[1]),
         BYTE_TO_FLOAT(v[2]));
}

static void GLAPIENTRY
TAG(SecondaryColor3dv)(const GLdouble *v)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR3F(VBO_ATTRIB_COLOR1, (GLfloat) v[0], (GLfloat) v[1], (GLfloat) v[2]);
}

static void GLAPIENTRY
TAG(SecondaryColor3iv)(const GLint *v)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR3F(VBO_ATTRIB_COLOR1, INT_TO_FLOAT(v[0]),
         INT_TO_FLOAT(v[1]),
         INT_TO_FLOAT(v[2]));
}

static void GLAPIENTRY
TAG(SecondaryColor3sv)(const GLshort *v)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR3F(VBO_ATTRIB_COLOR1, SHORT_TO_FLOAT(v[0]),
         SHORT_TO_FLOAT(v[1]),
         SHORT_TO_FLOAT(v[2]));
}

static void GLAPIENTRY
TAG(SecondaryColor3uiv)(const GLuint *v)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR3F(VBO_ATTRIB_COLOR1, UINT_TO_FLOAT(v[0]),
         UINT_TO_FLOAT(v[1]),
         UINT_TO_FLOAT(v[2]));
}

static void GLAPIENTRY
TAG(SecondaryColor3usv)(const GLushort *v)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR3F(VBO_ATTRIB_COLOR1, USHORT_TO_FLOAT(v[0]),
         USHORT_TO_FLOAT(v[1]),
         USHORT_TO_FLOAT(v[2]));
}

static void GLAPIENTRY
TAG(SecondaryColor3ubv)(const GLubyte *v)
{
   GET_CURRENT_CONTEXT(ctx);
   ATTR3F(VBO_ATTRIB_COLOR1, UBYTE_TO_FLOAT(v[0]),
         UBYTE_TO_FLOAT(v[1]),
         UBYTE_TO_FLOAT(v[2]));
}

#endif /* HW_SELECT_MODE */

#undef ATTR1FV
#undef ATTR2FV
#undef ATTR3FV
#undef ATTR4FV

#undef ATTR1F
#undef ATTR2F
#undef ATTR3F
#undef ATTR4F

#undef ATTR_UI

#ifdef SUPPRESS_STATIC
#undef static
#endif

#undef MAT
