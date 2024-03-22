/*
 * Copyright © 2013 Intel Corporation
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
#include <gtest/gtest.h>
#include "util/compiler.h"
#include "main/macros.h"
#include "ir.h"

/**
 * \file sampler_types_test.cpp
 *
 * Test that built-in sampler types have the right properties.
 */

#define ARRAY    EXPECT_TRUE(type->sampler_array);
#define NONARRAY EXPECT_FALSE(type->sampler_array);
#define SHADOW   EXPECT_TRUE(type->sampler_shadow);
#define COLOR    EXPECT_FALSE(type->sampler_shadow);

#define T(TYPE, DIM, DATA_TYPE, ARR, SHAD, COMPS)           \
TEST(sampler_types, TYPE)                                   \
{                                                           \
   const glsl_type *type = &glsl_type_builtin_##TYPE;        \
   EXPECT_EQ(GLSL_TYPE_SAMPLER, type->base_type);           \
   EXPECT_EQ(DIM, type->sampler_dimensionality);            \
   EXPECT_EQ(DATA_TYPE, type->sampled_type);                \
   ARR;                                                     \
   SHAD;                                                    \
   EXPECT_EQ(COMPS, glsl_get_sampler_coordinate_components(type));         \
}

T( sampler1D,        GLSL_SAMPLER_DIM_1D,   GLSL_TYPE_FLOAT, NONARRAY, COLOR,  1)
T( sampler2D,        GLSL_SAMPLER_DIM_2D,   GLSL_TYPE_FLOAT, NONARRAY, COLOR,  2)
T( sampler3D,        GLSL_SAMPLER_DIM_3D,   GLSL_TYPE_FLOAT, NONARRAY, COLOR,  3)
T( samplerCube,      GLSL_SAMPLER_DIM_CUBE, GLSL_TYPE_FLOAT, NONARRAY, COLOR,  3)
T( sampler1DArray,   GLSL_SAMPLER_DIM_1D,   GLSL_TYPE_FLOAT, ARRAY,    COLOR,  2)
T( sampler2DArray,   GLSL_SAMPLER_DIM_2D,   GLSL_TYPE_FLOAT, ARRAY,    COLOR,  3)
T( samplerCubeArray, GLSL_SAMPLER_DIM_CUBE, GLSL_TYPE_FLOAT, ARRAY,    COLOR,  4)
T( sampler2DRect,    GLSL_SAMPLER_DIM_RECT, GLSL_TYPE_FLOAT, NONARRAY, COLOR,  2)
T( samplerBuffer,    GLSL_SAMPLER_DIM_BUF,  GLSL_TYPE_FLOAT, NONARRAY, COLOR,  1)
T( sampler2DMS,      GLSL_SAMPLER_DIM_MS,   GLSL_TYPE_FLOAT, NONARRAY, COLOR,  2)
T( sampler2DMSArray, GLSL_SAMPLER_DIM_MS,   GLSL_TYPE_FLOAT, ARRAY,    COLOR,  3)
T(isampler1D,        GLSL_SAMPLER_DIM_1D,   GLSL_TYPE_INT,   NONARRAY, COLOR,  1)
T(isampler2D,        GLSL_SAMPLER_DIM_2D,   GLSL_TYPE_INT,   NONARRAY, COLOR,  2)
T(isampler3D,        GLSL_SAMPLER_DIM_3D,   GLSL_TYPE_INT,   NONARRAY, COLOR,  3)
T(isamplerCube,      GLSL_SAMPLER_DIM_CUBE, GLSL_TYPE_INT,   NONARRAY, COLOR,  3)
T(isampler1DArray,   GLSL_SAMPLER_DIM_1D,   GLSL_TYPE_INT,   ARRAY,    COLOR,  2)
T(isampler2DArray,   GLSL_SAMPLER_DIM_2D,   GLSL_TYPE_INT,   ARRAY,    COLOR,  3)
T(isamplerCubeArray, GLSL_SAMPLER_DIM_CUBE, GLSL_TYPE_INT,   ARRAY,    COLOR,  4)
T(isampler2DRect,    GLSL_SAMPLER_DIM_RECT, GLSL_TYPE_INT,   NONARRAY, COLOR,  2)
T(isamplerBuffer,    GLSL_SAMPLER_DIM_BUF,  GLSL_TYPE_INT,   NONARRAY, COLOR,  1)
T(isampler2DMS,      GLSL_SAMPLER_DIM_MS,   GLSL_TYPE_INT,   NONARRAY, COLOR,  2)
T(isampler2DMSArray, GLSL_SAMPLER_DIM_MS,   GLSL_TYPE_INT,   ARRAY,    COLOR,  3)
T(usampler1D,        GLSL_SAMPLER_DIM_1D,   GLSL_TYPE_UINT,  NONARRAY, COLOR,  1)
T(usampler2D,        GLSL_SAMPLER_DIM_2D,   GLSL_TYPE_UINT,  NONARRAY, COLOR,  2)
T(usampler3D,        GLSL_SAMPLER_DIM_3D,   GLSL_TYPE_UINT,  NONARRAY, COLOR,  3)
T(usamplerCube,      GLSL_SAMPLER_DIM_CUBE, GLSL_TYPE_UINT,  NONARRAY, COLOR,  3)
T(usampler1DArray,   GLSL_SAMPLER_DIM_1D,   GLSL_TYPE_UINT,  ARRAY,    COLOR,  2)
T(usampler2DArray,   GLSL_SAMPLER_DIM_2D,   GLSL_TYPE_UINT,  ARRAY,    COLOR,  3)
T(usamplerCubeArray, GLSL_SAMPLER_DIM_CUBE, GLSL_TYPE_UINT,  ARRAY,    COLOR,  4)
T(usampler2DRect,    GLSL_SAMPLER_DIM_RECT, GLSL_TYPE_UINT,  NONARRAY, COLOR,  2)
T(usamplerBuffer,    GLSL_SAMPLER_DIM_BUF,  GLSL_TYPE_UINT,  NONARRAY, COLOR,  1)
T(usampler2DMS,      GLSL_SAMPLER_DIM_MS,   GLSL_TYPE_UINT,  NONARRAY, COLOR,  2)
T(usampler2DMSArray, GLSL_SAMPLER_DIM_MS,   GLSL_TYPE_UINT,  ARRAY,    COLOR,  3)

T(sampler1DShadow,   GLSL_SAMPLER_DIM_1D,   GLSL_TYPE_FLOAT, NONARRAY, SHADOW, 1)
T(sampler2DShadow,   GLSL_SAMPLER_DIM_2D,   GLSL_TYPE_FLOAT, NONARRAY, SHADOW, 2)
T(samplerCubeShadow, GLSL_SAMPLER_DIM_CUBE, GLSL_TYPE_FLOAT, NONARRAY, SHADOW, 3)

T(sampler1DArrayShadow,
  GLSL_SAMPLER_DIM_1D, GLSL_TYPE_FLOAT, ARRAY, SHADOW, 2)
T(sampler2DArrayShadow,
  GLSL_SAMPLER_DIM_2D, GLSL_TYPE_FLOAT, ARRAY, SHADOW, 3)
T(samplerCubeArrayShadow,
  GLSL_SAMPLER_DIM_CUBE, GLSL_TYPE_FLOAT, ARRAY, SHADOW, 4)
T(sampler2DRectShadow,
  GLSL_SAMPLER_DIM_RECT, GLSL_TYPE_FLOAT, NONARRAY, SHADOW, 2)

T(samplerExternalOES,
  GLSL_SAMPLER_DIM_EXTERNAL, GLSL_TYPE_FLOAT, NONARRAY, COLOR, 2)
