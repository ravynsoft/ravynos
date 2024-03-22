/*
 * Copyright © 2015 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/**
 * \name mesa_formats.cpp
 *
 * Verify that all mesa formats are handled in certain functions and that
 * the format info table is sane.
 *
 */

#include <gtest/gtest.h>

#include "main/formats.h"
#include "main/glformats.h"
#include "main/format_unpack.h"
#include "main/format_pack.h"

// Test fixture for Format tests.
class MesaFormatsTest : public ::testing::Test {
};

/**
 * Debug/test: check that all uncompressed formats are handled in the
 * _mesa_uncompressed_format_to_type_and_comps() function. When new pixel
 * formats are added to Mesa, that function needs to be updated.
 */
TEST_F(MesaFormatsTest, FormatTypeAndComps)
{
   for (int fi = MESA_FORMAT_NONE + 1; fi < MESA_FORMAT_COUNT; ++fi) {
      mesa_format f = (mesa_format) fi;
      SCOPED_TRACE(_mesa_get_format_name(f));

      if (!_mesa_get_format_name(f))
         continue;

      /* This function will emit a problem/warning if the format is
       * not handled.
       */
      if (!_mesa_is_format_compressed(f)) {
         GLenum datatype = 0;
         GLuint comps = 0;

         /* If the datatype is zero, the format was not handled */
         _mesa_uncompressed_format_to_type_and_comps(f, &datatype, &comps);
         EXPECT_NE(datatype, (GLenum)0);
      }

   }
}

/**
 * Do sanity checking of the format info table.
 */
TEST_F(MesaFormatsTest, FormatSanity)
{
   for (int fi = 0; fi < MESA_FORMAT_COUNT; ++fi) {
      mesa_format f = (mesa_format) fi;
      SCOPED_TRACE(_mesa_get_format_name(f));
      if (!_mesa_get_format_name(f))
         continue;

      GLenum datatype = _mesa_get_format_datatype(f);
      GLint r = _mesa_get_format_bits(f, GL_RED_BITS);
      GLint g = _mesa_get_format_bits(f, GL_GREEN_BITS);
      GLint b = _mesa_get_format_bits(f, GL_BLUE_BITS);
      GLint a = _mesa_get_format_bits(f, GL_ALPHA_BITS);
      GLint l = _mesa_get_format_bits(f, GL_TEXTURE_LUMINANCE_SIZE);
      GLint i = _mesa_get_format_bits(f, GL_TEXTURE_INTENSITY_SIZE);

      /* Note: Z32_FLOAT_X24S8 has datatype of GL_NONE */
      EXPECT_TRUE(datatype == GL_NONE ||
                  datatype == GL_UNSIGNED_NORMALIZED ||
                  datatype == GL_SIGNED_NORMALIZED ||
                  datatype == GL_UNSIGNED_INT ||
                  datatype == GL_INT ||
                  datatype == GL_FLOAT);

      if (r > 0 && !_mesa_is_format_compressed(f)) {
         GLint bytes = _mesa_get_format_bytes(f);
         EXPECT_LE((r+g+b+a) / 8, bytes);
      }

      /* Determines if the base format has a channel [rgba] or property [li].
      * >  indicates existance
      * == indicates non-existance
      */
      #define HAS_PROP(rop,gop,bop,aop,lop,iop) \
         do { \
            EXPECT_TRUE(r rop 0); \
            EXPECT_TRUE(g gop 0); \
            EXPECT_TRUE(b bop 0); \
            EXPECT_TRUE(a aop 0); \
            EXPECT_TRUE(l lop 0); \
            EXPECT_TRUE(i iop 0); \
         } while(0)

      switch (_mesa_get_format_base_format(f)) {
      case GL_RGBA:
         HAS_PROP(>,>,>,>,==,==);
         break;
      case GL_RGB:
         HAS_PROP(>,>,>,==,==,==);
         break;
      case GL_RG:
         HAS_PROP(>,>,==,==,==,==);
         break;
      case GL_RED:
         HAS_PROP(>,==,==,==,==,==);
         break;
      case GL_LUMINANCE:
         HAS_PROP(==,==,==,==,>,==);
         break;
      case GL_INTENSITY:
         HAS_PROP(==,==,==,==,==,>);
         break;
      default:
         break;
      }

      #undef HAS_PROP

   }
}

TEST_F(MesaFormatsTest, IntensityToRed)
{
   EXPECT_EQ(_mesa_get_intensity_format_red(MESA_FORMAT_I_UNORM8),
             MESA_FORMAT_R_UNORM8);
   EXPECT_EQ(_mesa_get_intensity_format_red(MESA_FORMAT_I_SINT32),
             MESA_FORMAT_R_SINT32);
   EXPECT_EQ(_mesa_get_intensity_format_red(MESA_FORMAT_R8G8B8A8_UNORM),
             MESA_FORMAT_R8G8B8A8_UNORM);
}

static mesa_format fffat_wrap(GLenum format, GLenum type)
{
   uint32_t f = _mesa_format_from_format_and_type(format, type);
   if (_mesa_format_is_mesa_array_format(f))
      f = _mesa_format_from_array_format((mesa_array_format)f);
   return (mesa_format)f;
}

TEST_F(MesaFormatsTest, FormatFromFormatAndType)
{
   EXPECT_EQ(fffat_wrap(GL_RGBA, GL_SHORT),
             MESA_FORMAT_RGBA_SNORM16);
   EXPECT_EQ(fffat_wrap(GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT),
             MESA_FORMAT_Z_UNORM16);
   EXPECT_EQ(fffat_wrap(GL_STENCIL_INDEX, GL_UNSIGNED_BYTE),
             MESA_FORMAT_S_UINT8);

   /* Should return an array format, but not a proper MESA_FORMAT. */
   EXPECT_TRUE(_mesa_format_is_mesa_array_format(_mesa_format_from_format_and_type(GL_DEPTH_COMPONENT,
                                                                                   GL_BYTE)));
}

TEST_F(MesaFormatsTest, FormatMatchesFormatAndType)
{
   EXPECT_TRUE(_mesa_format_matches_format_and_type(MESA_FORMAT_RGBA_UNORM16,
                                                    GL_RGBA,
                                                    GL_UNSIGNED_SHORT, false,
                                                    NULL));
   EXPECT_TRUE(_mesa_format_matches_format_and_type(MESA_FORMAT_S_UINT8,
                                                    GL_STENCIL_INDEX,
                                                    GL_UNSIGNED_BYTE, false,
                                                    NULL));
   EXPECT_TRUE(_mesa_format_matches_format_and_type(MESA_FORMAT_Z_UNORM16,
                                                    GL_DEPTH_COMPONENT,
                                                    GL_UNSIGNED_SHORT, false,
                                                    NULL));
}

static uint32_t
test_unpack_r8i(int8_t val)
{
   uint32_t result[4];
   _mesa_unpack_uint_rgba_row(MESA_FORMAT_R_SINT8, 1, &val, &result);
   return result[0];
}

static uint32_t
test_unpack_r32ui(uint32_t val)
{
   uint32_t result[4];
   _mesa_unpack_uint_rgba_row(MESA_FORMAT_R_UINT32, 1, &val, &result);
   return result[0];
}

TEST_F(MesaFormatsTest, UnpackRGBAUintRow)
{
   EXPECT_EQ(test_unpack_r8i(0), 0);
   EXPECT_EQ(test_unpack_r8i(1), 1);
   EXPECT_EQ(test_unpack_r8i(0xff), 0xffffffff);
   EXPECT_EQ(test_unpack_r32ui(0), 0);
   EXPECT_EQ(test_unpack_r32ui(0xffffffff), 0xffffffff);
}

TEST_F(MesaFormatsTest, UnpackRGBAUbyteRowRGBA32F)
{
   float val[4] = {0, 0.5, -1, 2};
   uint8_t result[4];
   _mesa_unpack_ubyte_rgba_row(MESA_FORMAT_RGBA_FLOAT32, 1, &val, &result);
   EXPECT_EQ(result[0], 0);
   EXPECT_EQ(result[1], 0x80);
   EXPECT_EQ(result[2], 0);
   EXPECT_EQ(result[3], 0xff);
}

TEST_F(MesaFormatsTest, UnpackRGBAUbyteRowRGBA4)
{
   uint16_t val = (1 << 0) | (0x3f << 5) | (0x10 << 11);
   uint8_t result[4];
   _mesa_unpack_ubyte_rgba_row(MESA_FORMAT_R5G6B5_UNORM, 1, &val, &result);
   EXPECT_EQ(result[0], 0x08);
   EXPECT_EQ(result[1], 0xff);
   EXPECT_EQ(result[2], 0x84);
   EXPECT_EQ(result[3], 0xff);
}

static float
test_unpack_floatz_z32f(float val)
{
   float result;
   _mesa_unpack_float_z_row(MESA_FORMAT_Z_FLOAT32, 1, &val, &result);
   return result;
}

TEST_F(MesaFormatsTest, UnpackFloatZRow)
{
   EXPECT_EQ(test_unpack_floatz_z32f(0.5), 0.5);
   EXPECT_EQ(test_unpack_floatz_z32f(-1.0), -1.0);
   EXPECT_EQ(test_unpack_floatz_z32f(2.0), 2.0);
}

static uint32_t
test_unpack_uintz_z32f(float val)
{
   uint32_t result;
   _mesa_unpack_uint_z_row(MESA_FORMAT_Z_FLOAT32, 1, &val, &result);
   return result;
}

TEST_F(MesaFormatsTest, UnpackUintZRow)
{
   EXPECT_EQ(test_unpack_uintz_z32f(0.5), 0x7fffffff);
   EXPECT_EQ(test_unpack_uintz_z32f(-1.0), 0);
   EXPECT_EQ(test_unpack_uintz_z32f(2.0), 0xffffffff);
}

/* It's easy to have precision issues packing 32-bit floats to unorm. */
TEST_F(MesaFormatsTest, PackFloatZ)
{
   float val = 0.571428597f;
   uint32_t result;
   _mesa_pack_float_z_row(MESA_FORMAT_Z_UNORM32, 1, &val, &result);
   EXPECT_EQ(result, 0x924924ff);
}

TEST_F(MesaFormatsTest, PackUbyteRGBARounding)
{
   for (int i = 0; i <= 255; i++) {
      uint8_t val[4] = {(uint8_t)i, 0, 0, 0};
      uint16_t result;
      _mesa_pack_ubyte_rgba_row(MESA_FORMAT_R5G6B5_UNORM, 1, val, &result);
      EXPECT_EQ(result, (i * 31 + 127) / 255);
   }
}
