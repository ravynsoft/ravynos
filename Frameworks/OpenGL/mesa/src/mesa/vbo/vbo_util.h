/**************************************************************************
 *
 * Copyright 2002 VMware, Inc.
 * Copyright 2011 Dave Airlie (ARB_vertex_type_2_10_10_10_rev support)
 * Copyright 2020 Advanced Micro Devices, Inc.
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

#ifndef VBO_UTIL_H
#define VBO_UTIL_H

static inline float conv_ui10_to_norm_float(unsigned ui10)
{
   return ui10 / 1023.0f;
}

static inline float conv_ui2_to_norm_float(unsigned ui2)
{
   return ui2 / 3.0f;
}

struct attr_bits_10 {signed int x:10;};
struct attr_bits_2 {signed int x:2;};

static inline float conv_i10_to_i(int i10)
{
   struct attr_bits_10 val;
   val.x = i10;
   return (float)val.x;
}

static inline float conv_i2_to_i(int i2)
{
   struct attr_bits_2 val;
   val.x = i2;
   return (float)val.x;
}

static inline float conv_i10_to_norm_float(const struct gl_context *ctx, int i10)
{
   struct attr_bits_10 val;
   val.x = i10;

   /* Traditionally, OpenGL has had two equations for converting from
    * normalized fixed-point data to floating-point data.  In the OpenGL 3.2
    * specification, these are equations 2.2 and 2.3, respectively:
    *
    *    f = (2c + 1)/(2^b - 1).                                (2.2)
    *
    * Comments below this equation state: "In general, this representation is
    * used for signed normalized fixed-point parameters in GL commands, such
    * as vertex attribute values."  Which is what we're doing here.
    *
    *    f = max{c/(2^(b-1) - 1), -1.0}                         (2.3)
    *
    * Comments below this equation state: "In general, this representation is
    * used for signed normalized fixed-point texture or floating point values."
    *
    * OpenGL 4.2+ and ES 3.0 remedy this and state that equation 2.3 (above)
    * is used in every case.  They remove equation 2.2 completely.
    */
   if (_mesa_is_gles3(ctx) ||
       (_mesa_is_desktop_gl(ctx) && ctx->Version >= 42)) {
      /* Equation 2.3 above. */
      float f = ((float) val.x) / 511.0F;
      return MAX2(f, -1.0f);
   } else {
      /* Equation 2.2 above. */
      return (2.0F * (float)val.x + 1.0F) * (1.0F  / 1023.0F);
   }
}

static inline float conv_i2_to_norm_float(const struct gl_context *ctx, int i2)
{
   struct attr_bits_2 val;
   val.x = i2;

   if (_mesa_is_gles3(ctx) ||
       (_mesa_is_desktop_gl(ctx) && ctx->Version >= 42)) {
      /* Equation 2.3 above. */
      float f = (float) val.x;
      return MAX2(f, -1.0f);
   } else {
      /* Equation 2.2 above. */
      return (2.0F * (float)val.x + 1.0F) * (1.0F / 3.0F);
   }
}

#define ERROR_IF_NOT_PACKED_TYPE(ctx, type, func) \
   if (type != GL_INT_2_10_10_10_REV && type != GL_UNSIGNED_INT_2_10_10_10_REV) { \
      _mesa_error(ctx, GL_INVALID_ENUM, "%s(type)", func); \
      return; \
   }

/* Extended version of ERROR_IF_NOT_PACKED_TYPE which also
 * accepts GL_UNSIGNED_INT_10F_11F_11F_REV.
 *
 * Only used for VertexAttribP[123]ui[v]; VertexAttribP4* cannot use this type,
 * and neither can legacy vertex attribs.
 */
#define ERROR_IF_NOT_PACKED_TYPE_EXT(ctx, type, func) \
   if (type != GL_INT_2_10_10_10_REV && type != GL_UNSIGNED_INT_2_10_10_10_REV && \
       type != GL_UNSIGNED_INT_10F_11F_11F_REV) { \
      _mesa_error(ctx, GL_INVALID_ENUM, "%s(type)", func); \
      return; \
   }

#endif
