/*
 * Copyright © 2012 Intel Corporation
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
 * \file enum_sizes.cpp
 * Validate the generated code in indirect_size.c
 *
 * The functions in indirect_size.c determine how many data values are
 * associated with each enumerant that can be passed to various OpenGL
 * functions.  Tests in this file probe each function in indirect_size.c with
 * each of the possible valid enums and verify that the correct size is
 * returned.  Tests in this file also probe each function in indirect_size.c
 * with a larger number of \b invalid enums and verify that zero is returned.
 */

#include <gtest/gtest.h>
#include <GL/gl.h>
extern "C" {
#include "indirect_size.h"
}

TEST(ValidEnumSizes, CallLists)
{
   EXPECT_EQ(1, __glCallLists_size(GL_BYTE));
   EXPECT_EQ(1, __glCallLists_size(GL_UNSIGNED_BYTE));
   EXPECT_EQ(2, __glCallLists_size(GL_SHORT));
   EXPECT_EQ(2, __glCallLists_size(GL_UNSIGNED_SHORT));
   EXPECT_EQ(2, __glCallLists_size(GL_2_BYTES));
   EXPECT_EQ(2, __glCallLists_size(GL_HALF_FLOAT));
   EXPECT_EQ(3, __glCallLists_size(GL_3_BYTES));
   EXPECT_EQ(4, __glCallLists_size(GL_INT));
   EXPECT_EQ(4, __glCallLists_size(GL_UNSIGNED_INT));
   EXPECT_EQ(4, __glCallLists_size(GL_FLOAT));
   EXPECT_EQ(4, __glCallLists_size(GL_4_BYTES));
}

TEST(InvalidEnumSizes, CallLists)
{
   for (unsigned i = 0; i < 0x10004; i++) {
      switch (i) {
      case GL_BYTE:
      case GL_UNSIGNED_BYTE:
      case GL_SHORT:
      case GL_UNSIGNED_SHORT:
      case GL_2_BYTES:
      case GL_HALF_FLOAT:
      case GL_3_BYTES:
      case GL_INT:
      case GL_UNSIGNED_INT:
      case GL_FLOAT:
      case GL_4_BYTES:
         break;
      default:
         EXPECT_EQ(0, __glCallLists_size(i)) << "i = 0x" <<
            std::setw(4) << std::setfill('0') << std::hex << i;
      }
   }
}

TEST(ValidEnumSizes, Fogfv)
{
   EXPECT_EQ(1, __glFogfv_size(GL_FOG_INDEX));
   EXPECT_EQ(1, __glFogfv_size(GL_FOG_DENSITY));
   EXPECT_EQ(1, __glFogfv_size(GL_FOG_START));
   EXPECT_EQ(1, __glFogfv_size(GL_FOG_END));
   EXPECT_EQ(1, __glFogfv_size(GL_FOG_MODE));
   EXPECT_EQ(1, __glFogfv_size(GL_FOG_OFFSET_VALUE_SGIX));
   EXPECT_EQ(1, __glFogfv_size(GL_FOG_DISTANCE_MODE_NV));
   EXPECT_EQ(4, __glFogfv_size(GL_FOG_COLOR));
}

TEST(InvalidEnumSizes, Fogfv)
{
   for (unsigned i = 0; i < 0x10004; i++) {
      switch (i) {
      case GL_FOG_INDEX:
      case GL_FOG_DENSITY:
      case GL_FOG_START:
      case GL_FOG_END:
      case GL_FOG_MODE:
      case GL_FOG_OFFSET_VALUE_SGIX:
      case GL_FOG_DISTANCE_MODE_NV:
      case GL_FOG_COLOR:
         break;
      default:
         EXPECT_EQ(0, __glFogfv_size(i)) << "i = 0x" <<
            std::setw(4) << std::setfill('0') << std::hex << i;
      }
   }
}

TEST(ValidEnumSizes, Lightfv)
{
   EXPECT_EQ(1, __glLightfv_size(GL_SPOT_EXPONENT));
   EXPECT_EQ(1, __glLightfv_size(GL_SPOT_CUTOFF));
   EXPECT_EQ(1, __glLightfv_size(GL_CONSTANT_ATTENUATION));
   EXPECT_EQ(1, __glLightfv_size(GL_LINEAR_ATTENUATION));
   EXPECT_EQ(1, __glLightfv_size(GL_QUADRATIC_ATTENUATION));
   EXPECT_EQ(3, __glLightfv_size(GL_SPOT_DIRECTION));
   EXPECT_EQ(4, __glLightfv_size(GL_AMBIENT));
   EXPECT_EQ(4, __glLightfv_size(GL_DIFFUSE));
   EXPECT_EQ(4, __glLightfv_size(GL_SPECULAR));
   EXPECT_EQ(4, __glLightfv_size(GL_POSITION));
}

TEST(InvalidEnumSizes, Lightfv)
{
   for (unsigned i = 0; i < 0x10004; i++) {
      switch (i) {
      case GL_SPOT_EXPONENT:
      case GL_SPOT_CUTOFF:
      case GL_CONSTANT_ATTENUATION:
      case GL_LINEAR_ATTENUATION:
      case GL_QUADRATIC_ATTENUATION:
      case GL_SPOT_DIRECTION:
      case GL_AMBIENT:
      case GL_DIFFUSE:
      case GL_SPECULAR:
      case GL_POSITION:
         break;
      default:
         EXPECT_EQ(0, __glLightfv_size(i)) << "i = 0x" <<
            std::setw(4) << std::setfill('0') << std::hex << i;
      }
   }
}

TEST(ValidEnumSizes, LightModelfv)
{
   EXPECT_EQ(1, __glLightModelfv_size(GL_LIGHT_MODEL_LOCAL_VIEWER));
   EXPECT_EQ(1, __glLightModelfv_size(GL_LIGHT_MODEL_TWO_SIDE));
   EXPECT_EQ(1, __glLightModelfv_size(GL_LIGHT_MODEL_COLOR_CONTROL));
   EXPECT_EQ(1, __glLightModelfv_size(GL_LIGHT_MODEL_COLOR_CONTROL_EXT));
   EXPECT_EQ(4, __glLightModelfv_size(GL_LIGHT_MODEL_AMBIENT));
}

TEST(InvalidEnumSizes, LightModelfv)
{
   for (unsigned i = 0; i < 0x10004; i++) {
      switch (i) {
      case GL_LIGHT_MODEL_LOCAL_VIEWER:
      case GL_LIGHT_MODEL_TWO_SIDE:
      case GL_LIGHT_MODEL_COLOR_CONTROL:
/*      case GL_LIGHT_MODEL_COLOR_CONTROL_EXT:*/
      case GL_LIGHT_MODEL_AMBIENT:
         break;
      default:
         EXPECT_EQ(0, __glLightModelfv_size(i)) << "i = 0x" <<
            std::setw(4) << std::setfill('0') << std::hex << i;
      }
   }
}

TEST(ValidEnumSizes, Materialfv)
{
   EXPECT_EQ(1, __glMaterialfv_size(GL_SHININESS));
   EXPECT_EQ(3, __glMaterialfv_size(GL_COLOR_INDEXES));
   EXPECT_EQ(4, __glMaterialfv_size(GL_AMBIENT));
   EXPECT_EQ(4, __glMaterialfv_size(GL_DIFFUSE));
   EXPECT_EQ(4, __glMaterialfv_size(GL_SPECULAR));
   EXPECT_EQ(4, __glMaterialfv_size(GL_EMISSION));
   EXPECT_EQ(4, __glMaterialfv_size(GL_AMBIENT_AND_DIFFUSE));
}

TEST(InvalidEnumSizes, Materialfv)
{
   for (unsigned i = 0; i < 0x10004; i++) {
      switch (i) {
      case GL_SHININESS:
      case GL_COLOR_INDEXES:
      case GL_AMBIENT:
      case GL_DIFFUSE:
      case GL_SPECULAR:
      case GL_EMISSION:
      case GL_AMBIENT_AND_DIFFUSE:
         break;
      default:
         EXPECT_EQ(0, __glMaterialfv_size(i)) << "i = 0x" <<
            std::setw(4) << std::setfill('0') << std::hex << i;
      }
   }
}

TEST(ValidEnumSizes, TexParameterfv)
{
   EXPECT_EQ(1, __glTexParameterfv_size(GL_TEXTURE_MAG_FILTER));
   EXPECT_EQ(1, __glTexParameterfv_size(GL_TEXTURE_MIN_FILTER));
   EXPECT_EQ(1, __glTexParameterfv_size(GL_TEXTURE_WRAP_S));
   EXPECT_EQ(1, __glTexParameterfv_size(GL_TEXTURE_WRAP_T));
   EXPECT_EQ(1, __glTexParameterfv_size(GL_TEXTURE_PRIORITY));
   EXPECT_EQ(1, __glTexParameterfv_size(GL_TEXTURE_WRAP_R));
   EXPECT_EQ(1, __glTexParameterfv_size(GL_TEXTURE_COMPARE_FAIL_VALUE_ARB));
   EXPECT_EQ(1, __glTexParameterfv_size(GL_SHADOW_AMBIENT_SGIX));
   EXPECT_EQ(1, __glTexParameterfv_size(GL_TEXTURE_MIN_LOD));
   EXPECT_EQ(1, __glTexParameterfv_size(GL_TEXTURE_MAX_LOD));
   EXPECT_EQ(1, __glTexParameterfv_size(GL_TEXTURE_BASE_LEVEL));
   EXPECT_EQ(1, __glTexParameterfv_size(GL_TEXTURE_MAX_LEVEL));
   EXPECT_EQ(1, __glTexParameterfv_size(GL_TEXTURE_CLIPMAP_FRAME_SGIX));
   EXPECT_EQ(1, __glTexParameterfv_size(GL_TEXTURE_LOD_BIAS_S_SGIX));
   EXPECT_EQ(1, __glTexParameterfv_size(GL_TEXTURE_LOD_BIAS_T_SGIX));
   EXPECT_EQ(1, __glTexParameterfv_size(GL_TEXTURE_LOD_BIAS_R_SGIX));
   EXPECT_EQ(1, __glTexParameterfv_size(GL_GENERATE_MIPMAP));
   EXPECT_EQ(1, __glTexParameterfv_size(GL_GENERATE_MIPMAP_SGIS));
   EXPECT_EQ(1, __glTexParameterfv_size(GL_TEXTURE_COMPARE_SGIX));
   EXPECT_EQ(1, __glTexParameterfv_size(GL_TEXTURE_COMPARE_OPERATOR_SGIX));
   EXPECT_EQ(1, __glTexParameterfv_size(GL_TEXTURE_MAX_CLAMP_S_SGIX));
   EXPECT_EQ(1, __glTexParameterfv_size(GL_TEXTURE_MAX_CLAMP_T_SGIX));
   EXPECT_EQ(1, __glTexParameterfv_size(GL_TEXTURE_MAX_CLAMP_R_SGIX));
   EXPECT_EQ(1, __glTexParameterfv_size(GL_TEXTURE_MAX_ANISOTROPY_EXT));
   EXPECT_EQ(1, __glTexParameterfv_size(GL_TEXTURE_LOD_BIAS));
   EXPECT_EQ(1, __glTexParameterfv_size(GL_TEXTURE_LOD_BIAS_EXT));
   EXPECT_EQ(1, __glTexParameterfv_size(GL_TEXTURE_STORAGE_HINT_APPLE));
   EXPECT_EQ(1, __glTexParameterfv_size(GL_STORAGE_PRIVATE_APPLE));
   EXPECT_EQ(1, __glTexParameterfv_size(GL_STORAGE_CACHED_APPLE));
   EXPECT_EQ(1, __glTexParameterfv_size(GL_STORAGE_SHARED_APPLE));
   EXPECT_EQ(1, __glTexParameterfv_size(GL_DEPTH_TEXTURE_MODE));
   EXPECT_EQ(1, __glTexParameterfv_size(GL_DEPTH_TEXTURE_MODE_ARB));
   EXPECT_EQ(1, __glTexParameterfv_size(GL_TEXTURE_COMPARE_MODE));
   EXPECT_EQ(1, __glTexParameterfv_size(GL_TEXTURE_COMPARE_MODE_ARB));
   EXPECT_EQ(1, __glTexParameterfv_size(GL_TEXTURE_COMPARE_FUNC));
   EXPECT_EQ(1, __glTexParameterfv_size(GL_TEXTURE_COMPARE_FUNC_ARB));
   EXPECT_EQ(1, __glTexParameterfv_size(GL_TEXTURE_UNSIGNED_REMAP_MODE_NV));
   EXPECT_EQ(2, __glTexParameterfv_size(GL_TEXTURE_CLIPMAP_CENTER_SGIX));
   EXPECT_EQ(2, __glTexParameterfv_size(GL_TEXTURE_CLIPMAP_OFFSET_SGIX));
   EXPECT_EQ(3, __glTexParameterfv_size(GL_TEXTURE_CLIPMAP_VIRTUAL_DEPTH_SGIX));
   EXPECT_EQ(4, __glTexParameterfv_size(GL_TEXTURE_BORDER_COLOR));
   EXPECT_EQ(4, __glTexParameterfv_size(GL_POST_TEXTURE_FILTER_BIAS_SGIX));
   EXPECT_EQ(4, __glTexParameterfv_size(GL_POST_TEXTURE_FILTER_SCALE_SGIX));
}

TEST(InvalidEnumSizes, TexParameterfv)
{
   for (unsigned i = 0; i < 0x10004; i++) {
      switch (i) {
      case GL_TEXTURE_MAG_FILTER:
      case GL_TEXTURE_MIN_FILTER:
      case GL_TEXTURE_WRAP_S:
      case GL_TEXTURE_WRAP_T:
      case GL_TEXTURE_PRIORITY:
      case GL_TEXTURE_WRAP_R:
      case GL_TEXTURE_COMPARE_FAIL_VALUE_ARB:
/*      case GL_SHADOW_AMBIENT_SGIX:*/
      case GL_TEXTURE_MIN_LOD:
      case GL_TEXTURE_MAX_LOD:
      case GL_TEXTURE_BASE_LEVEL:
      case GL_TEXTURE_MAX_LEVEL:
      case GL_TEXTURE_CLIPMAP_FRAME_SGIX:
      case GL_TEXTURE_LOD_BIAS_S_SGIX:
      case GL_TEXTURE_LOD_BIAS_T_SGIX:
      case GL_TEXTURE_LOD_BIAS_R_SGIX:
      case GL_GENERATE_MIPMAP:
/*      case GL_GENERATE_MIPMAP_SGIS:*/
      case GL_TEXTURE_COMPARE_SGIX:
      case GL_TEXTURE_COMPARE_OPERATOR_SGIX:
      case GL_TEXTURE_MAX_CLAMP_S_SGIX:
      case GL_TEXTURE_MAX_CLAMP_T_SGIX:
      case GL_TEXTURE_MAX_CLAMP_R_SGIX:
      case GL_TEXTURE_MAX_ANISOTROPY_EXT:
      case GL_TEXTURE_LOD_BIAS:
/*      case GL_TEXTURE_LOD_BIAS_EXT:*/
      case GL_TEXTURE_STORAGE_HINT_APPLE:
      case GL_STORAGE_PRIVATE_APPLE:
      case GL_STORAGE_CACHED_APPLE:
      case GL_STORAGE_SHARED_APPLE:
      case GL_DEPTH_TEXTURE_MODE:
/*      case GL_DEPTH_TEXTURE_MODE_ARB:*/
      case GL_TEXTURE_COMPARE_MODE:
/*      case GL_TEXTURE_COMPARE_MODE_ARB:*/
      case GL_TEXTURE_COMPARE_FUNC:
/*      case GL_TEXTURE_COMPARE_FUNC_ARB:*/
      case GL_TEXTURE_UNSIGNED_REMAP_MODE_NV:
      case GL_TEXTURE_CLIPMAP_CENTER_SGIX:
      case GL_TEXTURE_CLIPMAP_OFFSET_SGIX:
      case GL_TEXTURE_CLIPMAP_VIRTUAL_DEPTH_SGIX:
      case GL_TEXTURE_BORDER_COLOR:
      case GL_POST_TEXTURE_FILTER_BIAS_SGIX:
      case GL_POST_TEXTURE_FILTER_SCALE_SGIX:
         break;
      default:
         EXPECT_EQ(0, __glTexParameterfv_size(i)) << "i = 0x" <<
            std::setw(4) << std::setfill('0') << std::hex << i;
      }
   }
}

TEST(ValidEnumSizes, TexEnvfv)
{
   EXPECT_EQ(1, __glTexEnvfv_size(GL_ALPHA_SCALE));
   EXPECT_EQ(1, __glTexEnvfv_size(GL_TEXTURE_ENV_MODE));
   EXPECT_EQ(1, __glTexEnvfv_size(GL_TEXTURE_LOD_BIAS));
   EXPECT_EQ(1, __glTexEnvfv_size(GL_COMBINE_RGB));
   EXPECT_EQ(1, __glTexEnvfv_size(GL_COMBINE_ALPHA));
   EXPECT_EQ(1, __glTexEnvfv_size(GL_RGB_SCALE));
   EXPECT_EQ(1, __glTexEnvfv_size(GL_SOURCE0_RGB));
   EXPECT_EQ(1, __glTexEnvfv_size(GL_SOURCE1_RGB));
   EXPECT_EQ(1, __glTexEnvfv_size(GL_SOURCE2_RGB));
   EXPECT_EQ(1, __glTexEnvfv_size(GL_SOURCE3_RGB_NV));
   EXPECT_EQ(1, __glTexEnvfv_size(GL_SOURCE0_ALPHA));
   EXPECT_EQ(1, __glTexEnvfv_size(GL_SOURCE1_ALPHA));
   EXPECT_EQ(1, __glTexEnvfv_size(GL_SOURCE2_ALPHA));
   EXPECT_EQ(1, __glTexEnvfv_size(GL_SOURCE3_ALPHA_NV));
   EXPECT_EQ(1, __glTexEnvfv_size(GL_OPERAND0_RGB));
   EXPECT_EQ(1, __glTexEnvfv_size(GL_OPERAND1_RGB));
   EXPECT_EQ(1, __glTexEnvfv_size(GL_OPERAND2_RGB));
   EXPECT_EQ(1, __glTexEnvfv_size(GL_OPERAND3_RGB_NV));
   EXPECT_EQ(1, __glTexEnvfv_size(GL_OPERAND0_ALPHA));
   EXPECT_EQ(1, __glTexEnvfv_size(GL_OPERAND1_ALPHA));
   EXPECT_EQ(1, __glTexEnvfv_size(GL_OPERAND2_ALPHA));
   EXPECT_EQ(1, __glTexEnvfv_size(GL_OPERAND3_ALPHA_NV));
   EXPECT_EQ(1, __glTexEnvfv_size(GL_BUMP_TARGET_ATI));
   EXPECT_EQ(1, __glTexEnvfv_size(GL_COORD_REPLACE_ARB));
   EXPECT_EQ(1, __glTexEnvfv_size(GL_COORD_REPLACE_NV));
   EXPECT_EQ(4, __glTexEnvfv_size(GL_TEXTURE_ENV_COLOR));
}

TEST(InvalidEnumSizes, TexEnvfv)
{
   for (unsigned i = 0; i < 0x10004; i++) {
      switch (i) {
      case GL_ALPHA_SCALE:
      case GL_TEXTURE_ENV_MODE:
      case GL_TEXTURE_LOD_BIAS:
      case GL_COMBINE_RGB:
      case GL_COMBINE_ALPHA:
      case GL_RGB_SCALE:
      case GL_SOURCE0_RGB:
      case GL_SOURCE1_RGB:
      case GL_SOURCE2_RGB:
      case GL_SOURCE3_RGB_NV:
      case GL_SOURCE0_ALPHA:
      case GL_SOURCE1_ALPHA:
      case GL_SOURCE2_ALPHA:
      case GL_SOURCE3_ALPHA_NV:
      case GL_OPERAND0_RGB:
      case GL_OPERAND1_RGB:
      case GL_OPERAND2_RGB:
      case GL_OPERAND3_RGB_NV:
      case GL_OPERAND0_ALPHA:
      case GL_OPERAND1_ALPHA:
      case GL_OPERAND2_ALPHA:
      case GL_OPERAND3_ALPHA_NV:
      case GL_BUMP_TARGET_ATI:
      case GL_COORD_REPLACE_ARB:
/*      case GL_COORD_REPLACE_NV:*/
      case GL_TEXTURE_ENV_COLOR:
         break;
      default:
         EXPECT_EQ(0, __glTexEnvfv_size(i)) << "i = 0x" <<
            std::setw(4) << std::setfill('0') << std::hex << i;
      }
   }
}

TEST(ValidEnumSizes, TexGendv)
{
   EXPECT_EQ(1, __glTexGendv_size(GL_TEXTURE_GEN_MODE));
   EXPECT_EQ(4, __glTexGendv_size(GL_OBJECT_PLANE));
   EXPECT_EQ(4, __glTexGendv_size(GL_EYE_PLANE));
}

TEST(InvalidEnumSizes, TexGendv)
{
   for (unsigned i = 0; i < 0x10004; i++) {
      switch (i) {
      case GL_TEXTURE_GEN_MODE:
      case GL_OBJECT_PLANE:
      case GL_EYE_PLANE:
         break;
      default:
         EXPECT_EQ(0, __glTexGendv_size(i)) << "i = 0x" <<
            std::setw(4) << std::setfill('0') << std::hex << i;
      }
   }
}

TEST(ValidEnumSizes, Map1d)
{
   EXPECT_EQ(1, __glMap1d_size(GL_MAP1_INDEX));
   EXPECT_EQ(1, __glMap1d_size(GL_MAP1_TEXTURE_COORD_1));
   EXPECT_EQ(2, __glMap1d_size(GL_MAP1_TEXTURE_COORD_2));
   EXPECT_EQ(3, __glMap1d_size(GL_MAP1_NORMAL));
   EXPECT_EQ(3, __glMap1d_size(GL_MAP1_TEXTURE_COORD_3));
   EXPECT_EQ(3, __glMap1d_size(GL_MAP1_VERTEX_3));
   EXPECT_EQ(4, __glMap1d_size(GL_MAP1_COLOR_4));
   EXPECT_EQ(4, __glMap1d_size(GL_MAP1_TEXTURE_COORD_4));
   EXPECT_EQ(4, __glMap1d_size(GL_MAP1_VERTEX_4));
}

TEST(InvalidEnumSizes, Map1d)
{
   for (unsigned i = 0; i < 0x10004; i++) {
      switch (i) {
      case GL_MAP1_INDEX:
      case GL_MAP1_TEXTURE_COORD_1:
      case GL_MAP1_TEXTURE_COORD_2:
      case GL_MAP1_NORMAL:
      case GL_MAP1_TEXTURE_COORD_3:
      case GL_MAP1_VERTEX_3:
      case GL_MAP1_COLOR_4:
      case GL_MAP1_TEXTURE_COORD_4:
      case GL_MAP1_VERTEX_4:
         break;
      default:
         EXPECT_EQ(0, __glMap1d_size(i)) << "i = 0x" <<
            std::setw(4) << std::setfill('0') << std::hex << i;
      }
   }
}

TEST(ValidEnumSizes, Map2d)
{
   EXPECT_EQ(1, __glMap2d_size(GL_MAP2_INDEX));
   EXPECT_EQ(1, __glMap2d_size(GL_MAP2_TEXTURE_COORD_1));
   EXPECT_EQ(2, __glMap2d_size(GL_MAP2_TEXTURE_COORD_2));
   EXPECT_EQ(3, __glMap2d_size(GL_MAP2_NORMAL));
   EXPECT_EQ(3, __glMap2d_size(GL_MAP2_TEXTURE_COORD_3));
   EXPECT_EQ(3, __glMap2d_size(GL_MAP2_VERTEX_3));
   EXPECT_EQ(4, __glMap2d_size(GL_MAP2_COLOR_4));
   EXPECT_EQ(4, __glMap2d_size(GL_MAP2_TEXTURE_COORD_4));
   EXPECT_EQ(4, __glMap2d_size(GL_MAP2_VERTEX_4));
}

TEST(InvalidEnumSizes, Map2d)
{
   for (unsigned i = 0; i < 0x10004; i++) {
      switch (i) {
      case GL_MAP2_INDEX:
      case GL_MAP2_TEXTURE_COORD_1:
      case GL_MAP2_TEXTURE_COORD_2:
      case GL_MAP2_NORMAL:
      case GL_MAP2_TEXTURE_COORD_3:
      case GL_MAP2_VERTEX_3:
      case GL_MAP2_COLOR_4:
      case GL_MAP2_TEXTURE_COORD_4:
      case GL_MAP2_VERTEX_4:
         break;
      default:
         EXPECT_EQ(0, __glMap2d_size(i)) << "i = 0x" <<
            std::setw(4) << std::setfill('0') << std::hex << i;
      }
   }
}

TEST(ValidEnumSizes, ColorTableParameterfv)
{
   EXPECT_EQ(4, __glColorTableParameterfv_size(GL_COLOR_TABLE_SCALE));
   EXPECT_EQ(4, __glColorTableParameterfv_size(GL_COLOR_TABLE_BIAS));
}

TEST(InvalidEnumSizes, ColorTableParameterfv)
{
   for (unsigned i = 0; i < 0x10004; i++) {
      switch (i) {
      case GL_COLOR_TABLE_SCALE:
      case GL_COLOR_TABLE_BIAS:
         break;
      default:
         EXPECT_EQ(0, __glColorTableParameterfv_size(i)) << "i = 0x" <<
            std::setw(4) << std::setfill('0') << std::hex << i;
      }
   }
}

TEST(ValidEnumSizes, ConvolutionParameterfv)
{
   EXPECT_EQ(1, __glConvolutionParameterfv_size(GL_CONVOLUTION_BORDER_MODE));
   EXPECT_EQ(1, __glConvolutionParameterfv_size(GL_CONVOLUTION_BORDER_MODE_EXT));
   EXPECT_EQ(4, __glConvolutionParameterfv_size(GL_CONVOLUTION_FILTER_SCALE));
   EXPECT_EQ(4, __glConvolutionParameterfv_size(GL_CONVOLUTION_FILTER_SCALE_EXT));
   EXPECT_EQ(4, __glConvolutionParameterfv_size(GL_CONVOLUTION_FILTER_BIAS));
   EXPECT_EQ(4, __glConvolutionParameterfv_size(GL_CONVOLUTION_FILTER_BIAS_EXT));
   EXPECT_EQ(4, __glConvolutionParameterfv_size(GL_CONVOLUTION_BORDER_COLOR));
   EXPECT_EQ(4, __glConvolutionParameterfv_size(GL_CONVOLUTION_BORDER_COLOR_HP));
}

TEST(InvalidEnumSizes, ConvolutionParameterfv)
{
   for (unsigned i = 0; i < 0x10004; i++) {
      switch (i) {
      case GL_CONVOLUTION_BORDER_MODE:
/*      case GL_CONVOLUTION_BORDER_MODE_EXT:*/
      case GL_CONVOLUTION_FILTER_SCALE:
/*      case GL_CONVOLUTION_FILTER_SCALE_EXT:*/
      case GL_CONVOLUTION_FILTER_BIAS:
/*      case GL_CONVOLUTION_FILTER_BIAS_EXT:*/
      case GL_CONVOLUTION_BORDER_COLOR:
/*      case GL_CONVOLUTION_BORDER_COLOR_HP:*/
         break;
      default:
         EXPECT_EQ(0, __glConvolutionParameterfv_size(i)) << "i = 0x" <<
            std::setw(4) << std::setfill('0') << std::hex << i;
      }
   }
}

TEST(ValidEnumSizes, PointParameterfv)
{
   EXPECT_EQ(1, __glPointParameterfv_size(GL_POINT_SIZE_MIN));
   EXPECT_EQ(1, __glPointParameterfv_size(GL_POINT_SIZE_MIN_ARB));
   EXPECT_EQ(1, __glPointParameterfv_size(GL_POINT_SIZE_MIN_SGIS));
   EXPECT_EQ(1, __glPointParameterfv_size(GL_POINT_SIZE_MAX));
   EXPECT_EQ(1, __glPointParameterfv_size(GL_POINT_SIZE_MAX_ARB));
   EXPECT_EQ(1, __glPointParameterfv_size(GL_POINT_SIZE_MAX_SGIS));
   EXPECT_EQ(1, __glPointParameterfv_size(GL_POINT_FADE_THRESHOLD_SIZE));
   EXPECT_EQ(1, __glPointParameterfv_size(GL_POINT_FADE_THRESHOLD_SIZE_ARB));
   EXPECT_EQ(1, __glPointParameterfv_size(GL_POINT_FADE_THRESHOLD_SIZE_SGIS));
   EXPECT_EQ(1, __glPointParameterfv_size(GL_POINT_SPRITE_R_MODE_NV));
   EXPECT_EQ(1, __glPointParameterfv_size(GL_POINT_SPRITE_COORD_ORIGIN));
   EXPECT_EQ(3, __glPointParameterfv_size(GL_POINT_DISTANCE_ATTENUATION));
   EXPECT_EQ(3, __glPointParameterfv_size(GL_POINT_DISTANCE_ATTENUATION_ARB));
}

TEST(InvalidEnumSizes, PointParameterfv)
{
   for (unsigned i = 0; i < 0x10004; i++) {
      switch (i) {
      case GL_POINT_SIZE_MIN:
/*      case GL_POINT_SIZE_MIN_ARB:*/
/*      case GL_POINT_SIZE_MIN_SGIS:*/
      case GL_POINT_SIZE_MAX:
/*      case GL_POINT_SIZE_MAX_ARB:*/
/*      case GL_POINT_SIZE_MAX_SGIS:*/
      case GL_POINT_FADE_THRESHOLD_SIZE:
/*      case GL_POINT_FADE_THRESHOLD_SIZE_ARB:*/
/*      case GL_POINT_FADE_THRESHOLD_SIZE_SGIS:*/
      case GL_POINT_SPRITE_R_MODE_NV:
      case GL_POINT_SPRITE_COORD_ORIGIN:
      case GL_POINT_DISTANCE_ATTENUATION:
/*      case GL_POINT_DISTANCE_ATTENUATION_ARB:*/
         break;
      default:
         EXPECT_EQ(0, __glPointParameterfv_size(i)) << "i = 0x" <<
            std::setw(4) << std::setfill('0') << std::hex << i;
      }
   }
}
