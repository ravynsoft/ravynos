/*
 * Mesa 3-D graphics library
 *
 * Copyright (C) 1999-2006  Brian Paul   All Rights Reserved.
 * Copyright (C) 2022 Advanced Micro Devices, Inc.
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

/* This lowers glDrawElementsBaseVertex into glBegin/glEnd draws.
 *
 * It's used by glthread when uploading non-VBO vertex arrays would take too
 * much time due to small numbers of vertices per draw where indices have very
 * large differences. (e.g. indices {0, 1000000} normally cause us to upload
 * (1000000 * stride) bytes to draw 2 vertices) This helps performance for
 * such pathological cases.
 */

#include "context.h"
#include "glthread_marshal.h"
#include "vbo/vbo_util.h"
#include "util/half_float.h"

#define FIXED_TO_FLOAT(x) \
   (int32_t)(CLAMP((x), -65536.0f, 65535.0f) * (double)0x10000)

#define UNPACK_RGB10A2_USCALED(x) { \
   (x) & 0x3ff, \
   ((x) >> 10) & 0x3ff, \
   ((x) >> 20) & 0x3ff, \
   ((x) >> 30) & 0x3 \
}

#define UNPACK_RGB10A2_SSCALED(x) { \
   conv_i10_to_i((x) & 0x3ff),         \
   conv_i10_to_i(((x) >> 10) & 0x3ff), \
   conv_i10_to_i(((x) >> 20) & 0x3ff), \
   conv_i2_to_i(((x) >> 30) & 0x3) \
}

#define UNPACK_RGB10A2_UNORM(x) { \
   conv_ui10_to_norm_float((x) & 0x3ff), \
   conv_ui10_to_norm_float(((x) >> 10) & 0x3ff), \
   conv_ui10_to_norm_float(((x) >> 20) & 0x3ff), \
   conv_ui2_to_norm_float(((x) >> 30) & 0x3) \
}

#define UNPACK_RGB10A2_SNORM(x) { \
   conv_i10_to_norm_float(ctx, (x) & 0x3ff), \
   conv_i10_to_norm_float(ctx, ((x) >> 10) & 0x3ff), \
   conv_i10_to_norm_float(ctx, ((x) >> 20) & 0x3ff), \
   conv_i2_to_norm_float(ctx, ((x) >> 30) & 0x3) \
}

#define UNPACK_BGR10A2_USCALED(x) { \
   ((x) >> 20) & 0x3ff, \
   ((x) >> 10) & 0x3ff, \
   (x) & 0x3ff, \
   ((x) >> 30) & 0x3 \
}

#define UNPACK_BGR10A2_SSCALED(x) { \
   conv_i10_to_i(((x) >> 20) & 0x3ff), \
   conv_i10_to_i(((x) >> 10) & 0x3ff), \
   conv_i10_to_i((x) & 0x3ff),         \
   conv_i2_to_i(((x) >> 30) & 0x3) \
}

#define UNPACK_BGR10A2_UNORM(x) { \
   conv_ui10_to_norm_float(((x) >> 20) & 0x3ff), \
   conv_ui10_to_norm_float(((x) >> 10) & 0x3ff), \
   conv_ui10_to_norm_float((x) & 0x3ff), \
   conv_ui2_to_norm_float(((x) >> 30) & 0x3) \
}

#define UNPACK_BGR10A2_SNORM(x) { \
   conv_i10_to_norm_float(ctx, ((x) >> 20) & 0x3ff), \
   conv_i10_to_norm_float(ctx, ((x) >> 10) & 0x3ff), \
   conv_i10_to_norm_float(ctx, (x) & 0x3ff), \
   conv_i2_to_norm_float(ctx, ((x) >> 30) & 0x3) \
}

#define UNPACK_BGRA8_UNORM(x) { \
   UBYTE_TO_FLOAT(((x) >> 16) & 0xff), \
   UBYTE_TO_FLOAT(((x) >> 8) & 0xff), \
   UBYTE_TO_FLOAT((x) & 0xff), \
   UBYTE_TO_FLOAT(((x) >> 24) & 0xff) \
}

#define TEMPLATE_FUNC1(t, src, type, dst, conv) \
   static void _mesa_wrapped_VertexAttrib##t##1##src(GLuint index, type *v) \
   { \
      _mesa_marshal_VertexAttrib##t##1##dst(index, conv(v[0])); \
   }

#define TEMPLATE_FUNC2(t, src, type, dst, conv) \
   static void _mesa_wrapped_VertexAttrib##t##2##src(GLuint index, type *v) \
   { \
      _mesa_marshal_VertexAttrib##t##2##dst(index, conv(v[0]), conv(v[1])); \
   }

#define TEMPLATE_FUNC3(t, src, type, dst, conv) \
   static void _mesa_wrapped_VertexAttrib##t##3##src(GLuint index, type *v) \
   { \
      _mesa_marshal_VertexAttrib##t##3##dst(index, conv(v[0]), conv(v[1]), \
                                             conv(v[2])); \
   }

#define TEMPLATE_FUNC4(t, src, type, dst, conv) \
   static void _mesa_wrapped_VertexAttrib##t##4##src(GLuint index, type *v) \
   { \
      _mesa_marshal_VertexAttrib##t##4##dst(index, conv(v[0]), conv(v[1]), \
                                            conv(v[2]), conv(v[3])); \
   }

#define SCALED false
#define NORMALIZED true

#define TEMPLATE_FUNCP(n, src, type, normalized) \
   static void _mesa_wrapped_VertexAttribP##n##src(GLuint index, GLuint *v) \
   { \
      _mesa_marshal_VertexAttribP##n##ui(index, type, normalized, v[0]); \
   }

#define TEMPLATE_FUNCUP(n, src, dst, unpack) \
   static void _mesa_wrapped_VertexAttribP##n##src(GLuint index, GLuint *v) \
   { \
      float fv[n] = unpack(v[0]); \
      _mesa_marshal_VertexAttrib##n##dst(index, fv); \
   }

#define TEMPLATE_FUNCUP_CTX(n, src, dst, unpack) \
   static void _mesa_wrapped_VertexAttribP##n##src(GLuint index, GLuint *v) \
   { \
      GET_CURRENT_CONTEXT(ctx); \
      float fv[n] = unpack(v[0]); \
      _mesa_marshal_VertexAttrib##n##dst(index, fv); \
   }

#define TEMPLATE_FUNC_ALL3(t, src, type, dst, conv) \
   TEMPLATE_FUNC1(t, src, type, dst, conv) \
   TEMPLATE_FUNC2(t, src, type, dst, conv) \
   TEMPLATE_FUNC3(t, src, type, dst, conv)

#define TEMPLATE_FUNC_ALL4(t, src, type, dst, conv) \
   TEMPLATE_FUNC_ALL3(t, src, type, dst, conv) \
   TEMPLATE_FUNC4(t, src, type, dst, conv)

/* We use NV attributes because they can set all non-generic attributes. */

/* Define VertexAttrib wrappers using template macros. */
TEMPLATE_FUNC_ALL4(, bvNV, GLbyte, sNV, )
TEMPLATE_FUNC_ALL4(, NbvNV, GLbyte, fNV, BYTE_TO_FLOAT)
TEMPLATE_FUNC_ALL4(, ubvNV, GLubyte, sNV, )
TEMPLATE_FUNC_ALL3(, NubvNV, GLubyte, fNV, UBYTE_TO_FLOAT) /* TODO: use VertexAttrib4ubNV */

TEMPLATE_FUNC_ALL3(, bv, GLbyte, s, )                     /* TODO: use VertexAttrib4bv */
TEMPLATE_FUNC_ALL3(, Nbv, GLbyte, fARB, BYTE_TO_FLOAT)    /* TODO: use VertexAttrib4Nb */
TEMPLATE_FUNC_ALL3(, ubv, GLubyte, s, )                   /* TODO: use VertexAttrib4ubv */
TEMPLATE_FUNC_ALL3(, Nubv, GLubyte, fARB, UBYTE_TO_FLOAT) /* TODO: use VertexAttrib4Nub */
TEMPLATE_FUNC_ALL3(I, bv, GLbyte, iEXT, )                 /* TODO: use VertexAttribI4bv */
TEMPLATE_FUNC_ALL3(I, ubv, GLubyte, uiEXT, )              /* TODO: use VertexAttribI4ubv */

TEMPLATE_FUNC_ALL4(, NsvNV, GLshort, fNV, SHORT_TO_FLOAT)
TEMPLATE_FUNC_ALL4(, usvNV, GLushort, fNV, )
TEMPLATE_FUNC_ALL4(, NusvNV, GLushort, fNV, USHORT_TO_FLOAT)

TEMPLATE_FUNC_ALL3(, Nsv, GLshort, fARB, SHORT_TO_FLOAT)  /* TODO: use VertexAttrib4Nsv */
TEMPLATE_FUNC_ALL3(, usv, GLushort, fARB, )               /* TODO: use VertexAttrib4usv */
TEMPLATE_FUNC_ALL3(, Nusv, GLushort, fARB, USHORT_TO_FLOAT) /* TODO: use VertexAttrib4Nusv */
TEMPLATE_FUNC_ALL3(I, sv, GLshort, iEXT, )                /* TODO: use VertexAttribI4sv */
TEMPLATE_FUNC_ALL3(I, usv, GLushort, uiEXT, )             /* TODO: use VertexAttribI4usv */

TEMPLATE_FUNC_ALL4(, ivNV, GLint, fNV, )
TEMPLATE_FUNC_ALL4(, NivNV, GLint, fNV, INT_TO_FLOAT)
TEMPLATE_FUNC_ALL4(, uivNV, GLuint, fNV, )
TEMPLATE_FUNC_ALL4(, NuivNV, GLuint, fNV, UINT_TO_FLOAT)

TEMPLATE_FUNC_ALL3(, iv, GLint, fARB, )
TEMPLATE_FUNC_ALL3(, Niv, GLint, fARB, INT_TO_FLOAT)
TEMPLATE_FUNC_ALL3(, uiv, GLuint, fARB, )
TEMPLATE_FUNC_ALL3(, Nuiv, GLuint, fARB, UINT_TO_FLOAT)

TEMPLATE_FUNC_ALL4(, xvNV, GLfixed, fNV, FIXED_TO_FLOAT)
TEMPLATE_FUNC_ALL4(, xv, GLfixed, fARB, FIXED_TO_FLOAT)

TEMPLATE_FUNC_ALL4(, hv, GLhalf, fARB, _mesa_half_to_float)

TEMPLATE_FUNC2(L, ui64v, GLuint64, d, UINT64_AS_DOUBLE)
TEMPLATE_FUNC3(L, ui64v, GLuint64, d, UINT64_AS_DOUBLE)
TEMPLATE_FUNC4(L, ui64v, GLuint64, d, UINT64_AS_DOUBLE)

TEMPLATE_FUNCP(4, _rgb10a2_sscaled, GL_INT_2_10_10_10_REV, SCALED)
TEMPLATE_FUNCP(4, _rgb10a2_snorm, GL_INT_2_10_10_10_REV, NORMALIZED)
TEMPLATE_FUNCP(4, _rgb10a2_uscaled, GL_UNSIGNED_INT_2_10_10_10_REV, SCALED)
TEMPLATE_FUNCP(4, _rgb10a2_unorm, GL_UNSIGNED_INT_2_10_10_10_REV, NORMALIZED)
TEMPLATE_FUNCP(3, _rg11b10_float, GL_UNSIGNED_INT_10F_11F_11F_REV, SCALED)

TEMPLATE_FUNCUP(4, NV_rgb10a2_uscaled, fvNV, UNPACK_RGB10A2_USCALED)
TEMPLATE_FUNCUP(4, NV_rgb10a2_sscaled, fvNV, UNPACK_RGB10A2_SSCALED)
TEMPLATE_FUNCUP(4, NV_rgb10a2_unorm, fvNV, UNPACK_RGB10A2_UNORM)
TEMPLATE_FUNCUP_CTX(4, NV_rgb10a2_snorm, fvNV, UNPACK_RGB10A2_SNORM)

TEMPLATE_FUNCUP(4, NV_bgr10a2_uscaled, fvNV, UNPACK_BGR10A2_USCALED)
TEMPLATE_FUNCUP(4, NV_bgr10a2_sscaled, fvNV, UNPACK_BGR10A2_SSCALED)
TEMPLATE_FUNCUP(4, NV_bgr10a2_unorm, fvNV, UNPACK_BGR10A2_UNORM)
TEMPLATE_FUNCUP_CTX(4, NV_bgr10a2_snorm, fvNV, UNPACK_BGR10A2_SNORM)
TEMPLATE_FUNCUP(4, NV_bgra8_unorm, fvNV, UNPACK_BGRA8_UNORM)

TEMPLATE_FUNCUP(4, _bgr10a2_uscaled, fvARB, UNPACK_BGR10A2_USCALED)
TEMPLATE_FUNCUP(4, _bgr10a2_sscaled, fvARB, UNPACK_BGR10A2_SSCALED)
TEMPLATE_FUNCUP(4, _bgr10a2_unorm, fvARB, UNPACK_BGR10A2_UNORM)
TEMPLATE_FUNCUP_CTX(4, _bgr10a2_snorm, fvARB, UNPACK_BGR10A2_SNORM)
TEMPLATE_FUNCUP(4, _bgra8_unorm, fvARB, UNPACK_BGRA8_UNORM)

typedef void (GLAPIENTRY *attrib_func)(GLuint indx, const void *data);

/* indexing: [gltype & 0x3f][normalized][size - 1] */
static const attrib_func legacy_rgba_attrib_funcs[][2][4] = {
   { /* GL_BYTE */
      {
         (attrib_func)_mesa_wrapped_VertexAttrib1bvNV,
         (attrib_func)_mesa_wrapped_VertexAttrib2bvNV,
         (attrib_func)_mesa_wrapped_VertexAttrib3bvNV,
         (attrib_func)_mesa_wrapped_VertexAttrib4bvNV,
      },
      {
         (attrib_func)_mesa_wrapped_VertexAttrib1NbvNV,
         (attrib_func)_mesa_wrapped_VertexAttrib2NbvNV,
         (attrib_func)_mesa_wrapped_VertexAttrib3NbvNV,
         (attrib_func)_mesa_wrapped_VertexAttrib4NbvNV,
      },
   },
   { /* GL_UNSIGNED_BYTE */
      {
         (attrib_func)_mesa_wrapped_VertexAttrib1ubvNV,
         (attrib_func)_mesa_wrapped_VertexAttrib2ubvNV,
         (attrib_func)_mesa_wrapped_VertexAttrib3ubvNV,
         (attrib_func)_mesa_wrapped_VertexAttrib4ubvNV,
      },
      {
         (attrib_func)_mesa_wrapped_VertexAttrib1NubvNV,
         (attrib_func)_mesa_wrapped_VertexAttrib2NubvNV,
         (attrib_func)_mesa_wrapped_VertexAttrib3NubvNV,
         (attrib_func)_mesa_marshal_VertexAttrib4ubvNV, /* always normalized */
      },
   },
   { /* GL_SHORT */
      {
         (attrib_func)_mesa_marshal_VertexAttrib1svNV,
         (attrib_func)_mesa_marshal_VertexAttrib2svNV,
         (attrib_func)_mesa_marshal_VertexAttrib3svNV,
         (attrib_func)_mesa_marshal_VertexAttrib4svNV,
      },
      {
         (attrib_func)_mesa_wrapped_VertexAttrib1NsvNV,
         (attrib_func)_mesa_wrapped_VertexAttrib2NsvNV,
         (attrib_func)_mesa_wrapped_VertexAttrib3NsvNV,
         (attrib_func)_mesa_wrapped_VertexAttrib4NsvNV,
      },
   },
   { /* GL_UNSIGNED_SHORT */
      {
         (attrib_func)_mesa_wrapped_VertexAttrib1usvNV,
         (attrib_func)_mesa_wrapped_VertexAttrib2usvNV,
         (attrib_func)_mesa_wrapped_VertexAttrib3usvNV,
         (attrib_func)_mesa_wrapped_VertexAttrib4usvNV,
      },
      {
         (attrib_func)_mesa_wrapped_VertexAttrib1NusvNV,
         (attrib_func)_mesa_wrapped_VertexAttrib2NusvNV,
         (attrib_func)_mesa_wrapped_VertexAttrib3NusvNV,
         (attrib_func)_mesa_wrapped_VertexAttrib4NusvNV,
      },
   },
   { /* GL_INT */
      {
         (attrib_func)_mesa_wrapped_VertexAttrib1ivNV,
         (attrib_func)_mesa_wrapped_VertexAttrib2ivNV,
         (attrib_func)_mesa_wrapped_VertexAttrib3ivNV,
         (attrib_func)_mesa_wrapped_VertexAttrib4ivNV,
      },
      {
         (attrib_func)_mesa_wrapped_VertexAttrib1NivNV,
         (attrib_func)_mesa_wrapped_VertexAttrib2NivNV,
         (attrib_func)_mesa_wrapped_VertexAttrib3NivNV,
         (attrib_func)_mesa_wrapped_VertexAttrib4NivNV,
      },
   },
   { /* GL_UNSIGNED_INT */
      {
         (attrib_func)_mesa_wrapped_VertexAttrib1uivNV,
         (attrib_func)_mesa_wrapped_VertexAttrib2uivNV,
         (attrib_func)_mesa_wrapped_VertexAttrib3uivNV,
         (attrib_func)_mesa_wrapped_VertexAttrib4uivNV,
      },
      {
         (attrib_func)_mesa_wrapped_VertexAttrib1NuivNV,
         (attrib_func)_mesa_wrapped_VertexAttrib2NuivNV,
         (attrib_func)_mesa_wrapped_VertexAttrib3NuivNV,
         (attrib_func)_mesa_wrapped_VertexAttrib4NuivNV,
       },
   },
   { /* GL_FLOAT */
      {
         (attrib_func)_mesa_marshal_VertexAttrib1fvNV,
         (attrib_func)_mesa_marshal_VertexAttrib2fvNV,
         (attrib_func)_mesa_marshal_VertexAttrib3fvNV,
         (attrib_func)_mesa_marshal_VertexAttrib4fvNV,
      },
      {
         (attrib_func)_mesa_marshal_VertexAttrib1fvNV,
         (attrib_func)_mesa_marshal_VertexAttrib2fvNV,
         (attrib_func)_mesa_marshal_VertexAttrib3fvNV,
         (attrib_func)_mesa_marshal_VertexAttrib4fvNV,
      },
   },
   {{0}}, /* GL_2_BYTES */
   {{0}}, /* GL_3_BYTES */
   {{0}}, /* GL_4_BYTES */
   { /* GL_DOUBLE */
      {
         (attrib_func)_mesa_marshal_VertexAttrib1dvNV,
         (attrib_func)_mesa_marshal_VertexAttrib2dvNV,
         (attrib_func)_mesa_marshal_VertexAttrib3dvNV,
         (attrib_func)_mesa_marshal_VertexAttrib4dvNV,
      },
      {
         (attrib_func)_mesa_marshal_VertexAttrib1dvNV,
         (attrib_func)_mesa_marshal_VertexAttrib2dvNV,
         (attrib_func)_mesa_marshal_VertexAttrib3dvNV,
         (attrib_func)_mesa_marshal_VertexAttrib4dvNV,
      },
   },
   { /* GL_HALF_FLOAT */
      {
         (attrib_func)_mesa_marshal_VertexAttrib1hvNV,
         (attrib_func)_mesa_marshal_VertexAttrib2hvNV,
         (attrib_func)_mesa_marshal_VertexAttrib3hvNV,
         (attrib_func)_mesa_marshal_VertexAttrib4hvNV,
      },
      {
         (attrib_func)_mesa_marshal_VertexAttrib1hvNV,
         (attrib_func)_mesa_marshal_VertexAttrib2hvNV,
         (attrib_func)_mesa_marshal_VertexAttrib3hvNV,
         (attrib_func)_mesa_marshal_VertexAttrib4hvNV,
      },
   },
   { /* GL_FIXED */
      {
         (attrib_func)_mesa_wrapped_VertexAttrib1xvNV,
         (attrib_func)_mesa_wrapped_VertexAttrib2xvNV,
         (attrib_func)_mesa_wrapped_VertexAttrib3xvNV,
         (attrib_func)_mesa_wrapped_VertexAttrib4xvNV,
      },
      {
         (attrib_func)_mesa_wrapped_VertexAttrib1xvNV,
         (attrib_func)_mesa_wrapped_VertexAttrib2xvNV,
         (attrib_func)_mesa_wrapped_VertexAttrib3xvNV,
         (attrib_func)_mesa_wrapped_VertexAttrib4xvNV,
      },
   },
   {{0}}, /* unused (13) */
   {{0}}, /* unused (14) */
   {{0}}, /* unused (15) */
   {{0}}, /* unused (16) */
   {{0}}, /* unused (17) */
   {{0}}, /* unused (18) */
   {{0}}, /* unused (19) */
   {{0}}, /* unused (20) */
   {{0}}, /* unused (21) */
   {{0}}, /* unused (22) */
   {{0}}, /* unused (23) */
   {{0}}, /* unused (24) */
   {{0}}, /* unused (25) */
   {{0}}, /* unused (26) */
   {{0}}, /* unused (27) */
   {{0}}, /* unused (28) */
   {{0}}, /* unused (29) */
   {{0}}, /* unused (30) */
   { /* GL_INT_2_10_10_10_REV */
      {
         0,
         0,
         0,
         (attrib_func)_mesa_wrapped_VertexAttribP4NV_rgb10a2_sscaled,
      },
      {
         0,
         0,
         0,
         (attrib_func)_mesa_wrapped_VertexAttribP4NV_rgb10a2_snorm,
      },
   },
   {{0}}, /* unused (32) */
   { /* GL_HALF_FLOAT_OES */
      {
         (attrib_func)_mesa_marshal_VertexAttrib1hvNV,
         (attrib_func)_mesa_marshal_VertexAttrib2hvNV,
         (attrib_func)_mesa_marshal_VertexAttrib3hvNV,
         (attrib_func)_mesa_marshal_VertexAttrib4hvNV,
      },
      {
         (attrib_func)_mesa_marshal_VertexAttrib1hvNV,
         (attrib_func)_mesa_marshal_VertexAttrib2hvNV,
         (attrib_func)_mesa_marshal_VertexAttrib3hvNV,
         (attrib_func)_mesa_marshal_VertexAttrib4hvNV,
      },
   },
   {{0}}, /* unused (34) */
   {{0}}, /* unused (35) */
   {{0}}, /* unused (36) */
   {{0}}, /* unused (37) */
   {{0}}, /* unused (38) */
   {{0}}, /* unused (39) */
   { /* GL_UNSIGNED_INT_2_10_10_10_REV */
      {
         0,
         0,
         0,
         (attrib_func)_mesa_wrapped_VertexAttribP4NV_rgb10a2_uscaled,
      },
      {
         0,
         0,
         0,
         (attrib_func)_mesa_wrapped_VertexAttribP4NV_rgb10a2_unorm,
      },
   },
};

/* indexing: [type & 0x3][normalized] */
static const attrib_func legacy_bgra_attrib_funcs[4][2] = {
   { /* GL_UNSIGNED_INT_2_10_10_10_REV */
      (attrib_func)_mesa_wrapped_VertexAttribP4NV_bgr10a2_uscaled,
      (attrib_func)_mesa_wrapped_VertexAttribP4NV_bgr10a2_unorm,
   },
   { /* GL_UNSIGNED_BYTE */
      0,
      (attrib_func)_mesa_wrapped_VertexAttribP4NV_bgra8_unorm,
   },
   {0}, /* unused (2) */
   { /* GL_INT_2_10_10_10_REV */
      (attrib_func)_mesa_wrapped_VertexAttribP4NV_bgr10a2_sscaled,
      (attrib_func)_mesa_wrapped_VertexAttribP4NV_bgr10a2_snorm,
   }
};

/* indexing: [(gltype & 0x3f) | (double << 5)][integer*2 + normalized][size - 1] */
static const attrib_func generic_rgba_attrib_funcs[][4][4] = {
   { /* GL_BYTE */
      {
         (attrib_func)_mesa_wrapped_VertexAttrib1bv,
         (attrib_func)_mesa_wrapped_VertexAttrib2bv,
         (attrib_func)_mesa_wrapped_VertexAttrib3bv,
         (attrib_func)_mesa_marshal_VertexAttrib4bv,
      },
      {
         (attrib_func)_mesa_wrapped_VertexAttrib1Nbv,
         (attrib_func)_mesa_wrapped_VertexAttrib2Nbv,
         (attrib_func)_mesa_wrapped_VertexAttrib3Nbv,
         (attrib_func)_mesa_marshal_VertexAttrib4Nbv,
      },
      {
         (attrib_func)_mesa_wrapped_VertexAttribI1bv,
         (attrib_func)_mesa_wrapped_VertexAttribI2bv,
         (attrib_func)_mesa_wrapped_VertexAttribI3bv,
         (attrib_func)_mesa_marshal_VertexAttribI4bv,
      },
   },
   { /* GL_UNSIGNED_BYTE */
      {
         (attrib_func)_mesa_wrapped_VertexAttrib1ubv,
         (attrib_func)_mesa_wrapped_VertexAttrib2ubv,
         (attrib_func)_mesa_wrapped_VertexAttrib3ubv,
         (attrib_func)_mesa_marshal_VertexAttrib4ubv,
      },
      {
         (attrib_func)_mesa_wrapped_VertexAttrib1Nubv,
         (attrib_func)_mesa_wrapped_VertexAttrib2Nubv,
         (attrib_func)_mesa_wrapped_VertexAttrib3Nubv,
         (attrib_func)_mesa_marshal_VertexAttrib4Nubv,
      },
      {
         (attrib_func)_mesa_wrapped_VertexAttribI1ubv,
         (attrib_func)_mesa_wrapped_VertexAttribI2ubv,
         (attrib_func)_mesa_wrapped_VertexAttribI3ubv,
         (attrib_func)_mesa_marshal_VertexAttribI4ubv,
      },
   },
   { /* GL_SHORT */
      {
         (attrib_func)_mesa_marshal_VertexAttrib1sv,
         (attrib_func)_mesa_marshal_VertexAttrib2sv,
         (attrib_func)_mesa_marshal_VertexAttrib3sv,
         (attrib_func)_mesa_marshal_VertexAttrib4sv,
      },
      {
         (attrib_func)_mesa_wrapped_VertexAttrib1Nsv,
         (attrib_func)_mesa_wrapped_VertexAttrib2Nsv,
         (attrib_func)_mesa_wrapped_VertexAttrib3Nsv,
         (attrib_func)_mesa_marshal_VertexAttrib4Nsv,
      },
      {
         (attrib_func)_mesa_wrapped_VertexAttribI1sv,
         (attrib_func)_mesa_wrapped_VertexAttribI2sv,
         (attrib_func)_mesa_wrapped_VertexAttribI3sv,
         (attrib_func)_mesa_marshal_VertexAttribI4sv,
      },
   },
   { /* GL_UNSIGNED_SHORT */
      {
         (attrib_func)_mesa_wrapped_VertexAttrib1usv,
         (attrib_func)_mesa_wrapped_VertexAttrib2usv,
         (attrib_func)_mesa_wrapped_VertexAttrib3usv,
         (attrib_func)_mesa_marshal_VertexAttrib4usv,
      },
      {
         (attrib_func)_mesa_wrapped_VertexAttrib1Nusv,
         (attrib_func)_mesa_wrapped_VertexAttrib2Nusv,
         (attrib_func)_mesa_wrapped_VertexAttrib3Nusv,
         (attrib_func)_mesa_marshal_VertexAttrib4Nusv,
      },
      {
         (attrib_func)_mesa_wrapped_VertexAttribI1usv,
         (attrib_func)_mesa_wrapped_VertexAttribI2usv,
         (attrib_func)_mesa_wrapped_VertexAttribI3usv,
         (attrib_func)_mesa_marshal_VertexAttribI4usv,
      },
   },
   { /* GL_INT */
      {
         (attrib_func)_mesa_wrapped_VertexAttrib1iv,
         (attrib_func)_mesa_wrapped_VertexAttrib2iv,
         (attrib_func)_mesa_wrapped_VertexAttrib3iv,
         (attrib_func)_mesa_marshal_VertexAttrib4iv,
      },
      {
         (attrib_func)_mesa_wrapped_VertexAttrib1Niv,
         (attrib_func)_mesa_wrapped_VertexAttrib2Niv,
         (attrib_func)_mesa_wrapped_VertexAttrib3Niv,
         (attrib_func)_mesa_marshal_VertexAttrib4Niv,
      },
      {
         (attrib_func)_mesa_marshal_VertexAttribI1iv,
         (attrib_func)_mesa_marshal_VertexAttribI2ivEXT,
         (attrib_func)_mesa_marshal_VertexAttribI3ivEXT,
         (attrib_func)_mesa_marshal_VertexAttribI4ivEXT,
      },
   },
   { /* GL_UNSIGNED_INT */
      {
         (attrib_func)_mesa_wrapped_VertexAttrib1uiv,
         (attrib_func)_mesa_wrapped_VertexAttrib2uiv,
         (attrib_func)_mesa_wrapped_VertexAttrib3uiv,
         (attrib_func)_mesa_marshal_VertexAttrib4uiv,
      },
      {
         (attrib_func)_mesa_wrapped_VertexAttrib1Nuiv,
         (attrib_func)_mesa_wrapped_VertexAttrib2Nuiv,
         (attrib_func)_mesa_wrapped_VertexAttrib3Nuiv,
         (attrib_func)_mesa_marshal_VertexAttrib4Nuiv,
      },
      {
         (attrib_func)_mesa_marshal_VertexAttribI1uiv,
         (attrib_func)_mesa_marshal_VertexAttribI2uivEXT,
         (attrib_func)_mesa_marshal_VertexAttribI3uivEXT,
         (attrib_func)_mesa_marshal_VertexAttribI4uivEXT,
      },
   },
   { /* GL_FLOAT */
      {
         (attrib_func)_mesa_marshal_VertexAttrib1fvARB,
         (attrib_func)_mesa_marshal_VertexAttrib2fvARB,
         (attrib_func)_mesa_marshal_VertexAttrib3fvARB,
         (attrib_func)_mesa_marshal_VertexAttrib4fvARB,
      },
      {
         (attrib_func)_mesa_marshal_VertexAttrib1fvARB,
         (attrib_func)_mesa_marshal_VertexAttrib2fvARB,
         (attrib_func)_mesa_marshal_VertexAttrib3fvARB,
         (attrib_func)_mesa_marshal_VertexAttrib4fvARB,
      },
   },
   {{0}}, /* GL_2_BYTES */
   {{0}}, /* GL_3_BYTES */
   {{0}}, /* GL_4_BYTES */
   { /* GL_DOUBLE */
      {
         (attrib_func)_mesa_marshal_VertexAttrib1dv,
         (attrib_func)_mesa_marshal_VertexAttrib2dv,
         (attrib_func)_mesa_marshal_VertexAttrib3dv,
         (attrib_func)_mesa_marshal_VertexAttrib4dv,
      },
      {
         (attrib_func)_mesa_marshal_VertexAttrib1dv,
         (attrib_func)_mesa_marshal_VertexAttrib2dv,
         (attrib_func)_mesa_marshal_VertexAttrib3dv,
         (attrib_func)_mesa_marshal_VertexAttrib4dv,
      },
   },
   { /* GL_HALF_FLOAT */
      {
         (attrib_func)_mesa_wrapped_VertexAttrib1hv,
         (attrib_func)_mesa_wrapped_VertexAttrib2hv,
         (attrib_func)_mesa_wrapped_VertexAttrib3hv,
         (attrib_func)_mesa_wrapped_VertexAttrib4hv,
      },
      {
         (attrib_func)_mesa_wrapped_VertexAttrib1hv,
         (attrib_func)_mesa_wrapped_VertexAttrib2hv,
         (attrib_func)_mesa_wrapped_VertexAttrib3hv,
         (attrib_func)_mesa_wrapped_VertexAttrib4hv,
      },
   },
   { /* GL_FIXED */
       {
         (attrib_func)_mesa_wrapped_VertexAttrib1xv,
         (attrib_func)_mesa_wrapped_VertexAttrib2xv,
         (attrib_func)_mesa_wrapped_VertexAttrib3xv,
         (attrib_func)_mesa_wrapped_VertexAttrib4xv,
      },
      {
         (attrib_func)_mesa_wrapped_VertexAttrib1xv,
         (attrib_func)_mesa_wrapped_VertexAttrib2xv,
         (attrib_func)_mesa_wrapped_VertexAttrib3xv,
         (attrib_func)_mesa_wrapped_VertexAttrib4xv,
      },
   },
   {{0}}, /* unused (13) */
   {{0}}, /* unused (14) */
   {{0}}, /* unused (15) */
   {{0}}, /* unused (16) */
   {{0}}, /* unused (17) */
   {{0}}, /* unused (18) */
   {{0}}, /* unused (19) */
   {{0}}, /* unused (20) */
   {{0}}, /* unused (21) */
   {{0}}, /* unused (22) */
   {{0}}, /* unused (23) */
   {{0}}, /* unused (24) */
   {{0}}, /* unused (25) */
   {{0}}, /* unused (26) */
   {{0}}, /* unused (27) */
   {{0}}, /* unused (28) */
   {{0}}, /* unused (29) */
   {{0}}, /* unused (30) */
   { /* GL_INT_2_10_10_10_REV */
      {
         0,
         0,
         0,
         (attrib_func)_mesa_wrapped_VertexAttribP4_rgb10a2_sscaled,
      },
      {
         0,
         0,
         0,
         (attrib_func)_mesa_wrapped_VertexAttribP4_rgb10a2_snorm,
      },
   },
   {{0}}, /* unused (32) */
   { /* GL_HALF_FLOAT_OES */
      {
         (attrib_func)_mesa_wrapped_VertexAttrib1hv,
         (attrib_func)_mesa_wrapped_VertexAttrib2hv,
         (attrib_func)_mesa_wrapped_VertexAttrib3hv,
         (attrib_func)_mesa_wrapped_VertexAttrib4hv,
      },
      {
         (attrib_func)_mesa_wrapped_VertexAttrib1hv,
         (attrib_func)_mesa_wrapped_VertexAttrib2hv,
         (attrib_func)_mesa_wrapped_VertexAttrib3hv,
         (attrib_func)_mesa_wrapped_VertexAttrib4hv,
      },
   },
   {{0}}, /* unused (34) */
   {{0}}, /* unused (35) */
   {{0}}, /* unused (36) */
   {{0}}, /* unused (37) */
   {{0}}, /* unused (38) */
   {{0}}, /* unused (39) */
   { /* GL_UNSIGNED_INT_2_10_10_10_REV */
      {
         0,
         0,
         0,
         (attrib_func)_mesa_wrapped_VertexAttribP4_rgb10a2_uscaled,
      },
      {
         0,
         0,
         0,
         (attrib_func)_mesa_wrapped_VertexAttribP4_rgb10a2_unorm,
      },
   },
   {{0}}, /* unused (41) */
   { /* GL_DOUBLE | (doubles << 5) (real double) */
      {
         (attrib_func)_mesa_marshal_VertexAttribL1dv,
         (attrib_func)_mesa_marshal_VertexAttribL2dv,
         (attrib_func)_mesa_marshal_VertexAttribL3dv,
         (attrib_func)_mesa_marshal_VertexAttribL4dv,
      },
   },
   {{0}}, /* unused (43) */
   {{0}}, /* unused (44) */
   {{0}}, /* unused (45) */
   {{0}}, /* unused (46) */
   { /* GL_UNSIGNED_INT64_ARB | (doubles << 5) (doubles is always true) */
     {0},
     {0},
     {
        (attrib_func)_mesa_marshal_VertexAttribL1ui64vARB,
        (attrib_func)_mesa_wrapped_VertexAttribL2ui64v,
        (attrib_func)_mesa_wrapped_VertexAttribL3ui64v,
        (attrib_func)_mesa_wrapped_VertexAttribL4ui64v,
     },
   },
   {{0}}, /* unused (48) */
   {{0}}, /* unused (49) */
   {{0}}, /* unused (50) */
   {{0}}, /* unused (51) */
   {{0}}, /* unused (52) */
   {{0}}, /* unused (53) */
   {{0}}, /* unused (54) */
   {{0}}, /* unused (55) */
   {{0}}, /* unused (56) */
   {{0}}, /* unused (57) */
   {{0}}, /* unused (58) */
   { /* GL_UNSIGNED_INT_10F_11F_11F_REV */
      {
         0,
         0,
         (attrib_func)_mesa_wrapped_VertexAttribP3_rg11b10_float,
         0
      },
      {
         0,
         0,
         (attrib_func)_mesa_wrapped_VertexAttribP3_rg11b10_float,
         0
      },
   },
};

/* indexing: [type & 0x3][normalized] */
static const attrib_func generic_bgra_attrib_funcs[4][2] = {
   { /* GL_UNSIGNED_INT_2_10_10_10_REV */
      (attrib_func)_mesa_wrapped_VertexAttribP4_bgr10a2_uscaled,
      (attrib_func)_mesa_wrapped_VertexAttribP4_bgr10a2_unorm,
   },
   { /* GL_UNSIGNED_BYTE */
      0,
      (attrib_func)_mesa_wrapped_VertexAttribP4_bgra8_unorm,
   },
   {0}, /* unused (2) */
   { /* GL_INT_2_10_10_10_REV */
      (attrib_func)_mesa_wrapped_VertexAttribP4_bgr10a2_sscaled,
      (attrib_func)_mesa_wrapped_VertexAttribP4_bgr10a2_snorm,
   }
};

/*
 * Return VertexAttrib*NV function pointer matching the provided vertex format.
 */
static inline attrib_func
get_legacy_func(union gl_vertex_format_user format)
{
   if (format.Bgra)
      return legacy_bgra_attrib_funcs[format.Type & 0x3][format.Normalized];

   int type = format.Type & 0x3f;

   assert(type < ARRAY_SIZE(legacy_rgba_attrib_funcs));
   return legacy_rgba_attrib_funcs[type][format.Normalized][format.Size - 1];
}

/*
 * Return VertexAttrib*ARB function pointer matching the provided vertex format.
 */
static inline attrib_func
get_generic_func(union gl_vertex_format_user format)
{
   if (format.Bgra)
      return generic_bgra_attrib_funcs[format.Type & 0x3][format.Normalized];

   int type = (format.Type & 0x3f) | ((int)format.Doubles << 5);
   int mod = format.Integer * 2 + format.Normalized;

   assert(type < ARRAY_SIZE(generic_rgba_attrib_funcs));
   return generic_rgba_attrib_funcs[type][mod][format.Size - 1];
}

static inline const uint8_t *
attrib_addr(const struct glthread_vao *vao,
            const struct glthread_attrib *array)
{
   return (const uint8_t*)vao->Attrib[array->BufferIndex].Pointer +
          array->RelativeOffset;
}

static inline unsigned
attrib_stride(const struct glthread_vao *vao,
              const struct glthread_attrib *array)
{
   return vao->Attrib[array->BufferIndex].Stride;
}

struct attrib_info {
   attrib_func marshal; /* glVertex4fv, etc. */
   const uint8_t *ptr;  /* vertex array pointer at the first vertex */
   uint16_t stride;
   uint8_t attrib;      /* VERT_ATTRIB_* */
};

/**
 * Convert glDrawElements into glBegin/End.
 *
 * We use this when we need to upload non-VBO vertices and the vertex range
 * to upload is much greater than the draw vertex count, which would cause
 * the upload to take too much time. We can get better performance if we
 * read each vertex from user memory and push it through glBegin/End.
 *
 * This assumes: No buffer objects, no instancing, no primitive restart.
 */
void
_mesa_glthread_UnrollDrawElements(struct gl_context *ctx,
                                  GLenum mode, GLsizei count, GLenum type,
                                  const GLvoid *indices, GLint basevertex)
{
   /* First gather all glVertex(Attrib) function pointers and attribute
    * information, and then execute them between glBegin/End for every vertex.
    */
   const struct glthread_vao *vao = ctx->GLThread.CurrentVAO;
   struct attrib_info attribs[VERT_ATTRIB_MAX];
   unsigned num_attribs = 0;

   /* Gather glColor, glTexCoord etc. functions for non-generic attributes. */
   GLbitfield mask = (VERT_BIT_FF_ALL & ~VERT_BIT_POS) & vao->Enabled;
   while (mask) {
      const gl_vert_attrib attrib = u_bit_scan(&mask);
      struct attrib_info *info = &attribs[num_attribs];
      const struct glthread_attrib *attr = &vao->Attrib[attrib];

      info->marshal = get_legacy_func(attr->Format);
      info->attrib = attrib;
      info->ptr = attrib_addr(vao, attr);
      info->stride = attrib_stride(vao, attr);
      num_attribs++;
   }

   /* Gather glVertexAttrib functions for generic attributes. */
   mask = (VERT_BIT_GENERIC_ALL & ~VERT_BIT_GENERIC0) & vao->Enabled;
   while (mask) {
      const gl_vert_attrib attrib = u_bit_scan(&mask);
      struct attrib_info *info = &attribs[num_attribs];
      const struct glthread_attrib *attr = &vao->Attrib[attrib];

      info->marshal = get_generic_func(attr->Format);
      info->attrib = attrib - VERT_ATTRIB_GENERIC0;
      info->ptr = attrib_addr(vao, attr);
      info->stride = attrib_stride(vao, attr);
      num_attribs++;
   }

   /* Finally, vertex position. */
   if (vao->Enabled & VERT_BIT_GENERIC0) {
      struct attrib_info *info = &attribs[num_attribs];
      const struct glthread_attrib *attr =
            &vao->Attrib[VERT_ATTRIB_GENERIC0];

      info->marshal = get_generic_func(attr->Format);
      info->attrib = 0;
      info->ptr = attrib_addr(vao, attr);
      info->stride = attrib_stride(vao, attr);
      num_attribs++;
   } else if (vao->Enabled & VERT_BIT_POS) {
      struct attrib_info *info = &attribs[num_attribs];
      const struct glthread_attrib *attr =
            &vao->Attrib[VERT_ATTRIB_POS];

      info->marshal = get_legacy_func(attr->Format);
      info->attrib = VERT_ATTRIB_POS;
      info->ptr = attrib_addr(vao, attr);
      info->stride = attrib_stride(vao, attr);
      num_attribs++;
   }

   /* Convert the draw into glBegin/End. */
   _mesa_marshal_Begin(mode);

   switch (type) {
   case GL_UNSIGNED_BYTE: {
      const uint8_t *ub = indices;
      for (int i = 0; i < count; i++) {
         for (unsigned a = 0; a < num_attribs; a++) {
            struct attrib_info *info = &attribs[a];
            unsigned index = ub[i] + basevertex;

            info->marshal(info->attrib, info->ptr + info->stride * index);
         }
      }
      break;
   }
   case GL_UNSIGNED_SHORT: {
      const uint16_t *us = indices;
      for (int i = 0; i < count; i++) {
         for (unsigned a = 0; a < num_attribs; a++) {
            struct attrib_info *info = &attribs[a];
            unsigned index = us[i] + basevertex;

            info->marshal(info->attrib, info->ptr + info->stride * index);
         }
      }
      break;
   }
   case GL_UNSIGNED_INT: {
      const uint32_t *ui = indices;
      for (int i = 0; i < count; i++) {
         for (unsigned a = 0; a < num_attribs; a++) {
            struct attrib_info *info = &attribs[a];
            unsigned index = ui[i] + basevertex;

            info->marshal(info->attrib, info->ptr + info->stride * index);
         }
      }
      break;
   }
   }

   _mesa_marshal_End();
}
