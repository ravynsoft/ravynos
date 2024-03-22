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

/**
 * \file builtin_types.cpp
 *
 * This contains _mesa_glsl_initialize_types(), a function which populates
 * a symbol table with the available built-in types for a particular language
 * version and set of enabled extensions.
 */

#include "compiler/glsl_types.h"
#include "glsl_parser_extras.h"
#include "util/macros.h"
#include "main/consts_exts.h"

static const struct glsl_struct_field gl_DepthRangeParameters_fields[] = {
   glsl_struct_field(&glsl_type_builtin_float, GLSL_PRECISION_HIGH, "near"),
   glsl_struct_field(&glsl_type_builtin_float, GLSL_PRECISION_HIGH, "far"),
   glsl_struct_field(&glsl_type_builtin_float, GLSL_PRECISION_HIGH, "diff"),
};

static const struct glsl_struct_field gl_PointParameters_fields[] = {
   glsl_struct_field(&glsl_type_builtin_float, "size"),
   glsl_struct_field(&glsl_type_builtin_float, "sizeMin"),
   glsl_struct_field(&glsl_type_builtin_float, "sizeMax"),
   glsl_struct_field(&glsl_type_builtin_float, "fadeThresholdSize"),
   glsl_struct_field(&glsl_type_builtin_float, "distanceConstantAttenuation"),
   glsl_struct_field(&glsl_type_builtin_float, "distanceLinearAttenuation"),
   glsl_struct_field(&glsl_type_builtin_float, "distanceQuadraticAttenuation"),
};

static const struct glsl_struct_field gl_MaterialParameters_fields[] = {
   glsl_struct_field(&glsl_type_builtin_vec4, "emission"),
   glsl_struct_field(&glsl_type_builtin_vec4, "ambient"),
   glsl_struct_field(&glsl_type_builtin_vec4, "diffuse"),
   glsl_struct_field(&glsl_type_builtin_vec4, "specular"),
   glsl_struct_field(&glsl_type_builtin_float, "shininess"),
};

static const struct glsl_struct_field gl_LightSourceParameters_fields[] = {
   glsl_struct_field(&glsl_type_builtin_vec4, "ambient"),
   glsl_struct_field(&glsl_type_builtin_vec4, "diffuse"),
   glsl_struct_field(&glsl_type_builtin_vec4, "specular"),
   glsl_struct_field(&glsl_type_builtin_vec4, "position"),
   glsl_struct_field(&glsl_type_builtin_vec4, "halfVector"),
   glsl_struct_field(&glsl_type_builtin_vec3, "spotDirection"),
   glsl_struct_field(&glsl_type_builtin_float, "spotCosCutoff"),
   glsl_struct_field(&glsl_type_builtin_float, "constantAttenuation"),
   glsl_struct_field(&glsl_type_builtin_float, "linearAttenuation"),
   glsl_struct_field(&glsl_type_builtin_float, "quadraticAttenuation"),
   glsl_struct_field(&glsl_type_builtin_float, "spotExponent"),
   glsl_struct_field(&glsl_type_builtin_float, "spotCutoff"),
};

static const struct glsl_struct_field gl_LightModelParameters_fields[] = {
   glsl_struct_field(&glsl_type_builtin_vec4, "ambient"),
};

static const struct glsl_struct_field gl_LightModelProducts_fields[] = {
   glsl_struct_field(&glsl_type_builtin_vec4, "sceneColor"),
};

static const struct glsl_struct_field gl_LightProducts_fields[] = {
   glsl_struct_field(&glsl_type_builtin_vec4, "ambient"),
   glsl_struct_field(&glsl_type_builtin_vec4, "diffuse"),
   glsl_struct_field(&glsl_type_builtin_vec4, "specular"),
};

static const struct glsl_struct_field gl_FogParameters_fields[] = {
   glsl_struct_field(&glsl_type_builtin_vec4, "color"),
   glsl_struct_field(&glsl_type_builtin_float, "density"),
   glsl_struct_field(&glsl_type_builtin_float, "start"),
   glsl_struct_field(&glsl_type_builtin_float, "end"),
   glsl_struct_field(&glsl_type_builtin_float, "scale"),
};

/**
 * Code to populate a symbol table with the built-in types available in a
 * particular shading language version.  The table below contains tags every
 * type with the GLSL/GLSL ES versions where it was introduced.
 *
 * @{
 */
#define T(TYPE, MIN_GL, MIN_ES) \
   { &glsl_type_builtin_##TYPE, MIN_GL, MIN_ES },

static const struct builtin_type_versions {
   const glsl_type *const type;
   int min_gl;
   int min_es;
} builtin_type_versions[] = {
   T(void,                            110, 100)
   T(bool,                            110, 100)
   T(bvec2,                           110, 100)
   T(bvec3,                           110, 100)
   T(bvec4,                           110, 100)
   T(int,                             110, 100)
   T(ivec2,                           110, 100)
   T(ivec3,                           110, 100)
   T(ivec4,                           110, 100)
   T(uint,                            130, 300)
   T(uvec2,                           130, 300)
   T(uvec3,                           130, 300)
   T(uvec4,                           130, 300)
   T(float,                           110, 100)
   T(vec2,                            110, 100)
   T(vec3,                            110, 100)
   T(vec4,                            110, 100)
   T(mat2,                            110, 100)
   T(mat3,                            110, 100)
   T(mat4,                            110, 100)
   T(mat2x3,                          120, 300)
   T(mat2x4,                          120, 300)
   T(mat3x2,                          120, 300)
   T(mat3x4,                          120, 300)
   T(mat4x2,                          120, 300)
   T(mat4x3,                          120, 300)

   T(double,                          400, 999)
   T(dvec2,                           400, 999)
   T(dvec3,                           400, 999)
   T(dvec4,                           400, 999)
   T(dmat2,                           400, 999)
   T(dmat3,                           400, 999)
   T(dmat4,                           400, 999)
   T(dmat2x3,                         400, 999)
   T(dmat2x4,                         400, 999)
   T(dmat3x2,                         400, 999)
   T(dmat3x4,                         400, 999)
   T(dmat4x2,                         400, 999)
   T(dmat4x3,                         400, 999)

   T(sampler1D,                       110, 999)
   T(sampler2D,                       110, 100)
   T(sampler3D,                       110, 300)
   T(samplerCube,                     110, 100)
   T(sampler1DArray,                  130, 999)
   T(sampler2DArray,                  130, 300)
   T(samplerCubeArray,                400, 320)
   T(sampler2DRect,                   140, 999)
   T(samplerBuffer,                   140, 320)
   T(sampler2DMS,                     150, 310)
   T(sampler2DMSArray,                150, 320)

   T(isampler1D,                      130, 999)
   T(isampler2D,                      130, 300)
   T(isampler3D,                      130, 300)
   T(isamplerCube,                    130, 300)
   T(isampler1DArray,                 130, 999)
   T(isampler2DArray,                 130, 300)
   T(isamplerCubeArray,               400, 320)
   T(isampler2DRect,                  140, 999)
   T(isamplerBuffer,                  140, 320)
   T(isampler2DMS,                    150, 310)
   T(isampler2DMSArray,               150, 320)

   T(usampler1D,                      130, 999)
   T(usampler2D,                      130, 300)
   T(usampler3D,                      130, 300)
   T(usamplerCube,                    130, 300)
   T(usampler1DArray,                 130, 999)
   T(usampler2DArray,                 130, 300)
   T(usamplerCubeArray,               400, 320)
   T(usampler2DRect,                  140, 999)
   T(usamplerBuffer,                  140, 320)
   T(usampler2DMS,                    150, 310)
   T(usampler2DMSArray,               150, 320)

   T(sampler1DShadow,                 110, 999)
   T(sampler2DShadow,                 110, 300)
   T(samplerCubeShadow,               130, 300)
   T(sampler1DArrayShadow,            130, 999)
   T(sampler2DArrayShadow,            130, 300)
   T(samplerCubeArrayShadow,          400, 320)
   T(sampler2DRectShadow,             140, 999)

   T(image1D,                         420, 999)
   T(image2D,                         420, 310)
   T(image3D,                         420, 310)
   T(image2DRect,                     420, 999)
   T(imageCube,                       420, 310)
   T(imageBuffer,                     420, 320)
   T(image1DArray,                    420, 999)
   T(image2DArray,                    420, 310)
   T(imageCubeArray,                  420, 320)
   T(image2DMS,                       420, 999)
   T(image2DMSArray,                  420, 999)
   T(iimage1D,                        420, 999)
   T(iimage2D,                        420, 310)
   T(iimage3D,                        420, 310)
   T(iimage2DRect,                    420, 999)
   T(iimageCube,                      420, 310)
   T(iimageBuffer,                    420, 320)
   T(iimage1DArray,                   420, 999)
   T(iimage2DArray,                   420, 310)
   T(iimageCubeArray,                 420, 320)
   T(iimage2DMS,                      420, 999)
   T(iimage2DMSArray,                 420, 999)
   T(uimage1D,                        420, 999)
   T(uimage2D,                        420, 310)
   T(uimage3D,                        420, 310)
   T(uimage2DRect,                    420, 999)
   T(uimageCube,                      420, 310)
   T(uimageBuffer,                    420, 320)
   T(uimage1DArray,                   420, 999)
   T(uimage2DArray,                   420, 310)
   T(uimageCubeArray,                 420, 320)
   T(uimage2DMS,                      420, 999)
   T(uimage2DMSArray,                 420, 999)

   T(atomic_uint,                     420, 310)
};

#undef T

static inline void
add_type(glsl_symbol_table *symbols, const glsl_type *const type)
{
   symbols->add_type(glsl_get_type_name(type), type);
}

/**
 * Populate the symbol table with available built-in types.
 */
void
_mesa_glsl_initialize_types(struct _mesa_glsl_parse_state *state)
{
   struct glsl_symbol_table *symbols = state->symbols;

   for (unsigned i = 0; i < ARRAY_SIZE(builtin_type_versions); i++) {
      const struct builtin_type_versions *const t = &builtin_type_versions[i];
      if (state->is_version(t->min_gl, t->min_es)) {
         add_type(symbols, t->type);
      }
   }

   /* Note: use glsl_type::get_struct_instance() to get the properly cached
    * copy of the struct types.
    */
   {
      #define GET_STRUCT_TYPE(NAME) glsl_struct_type(NAME##_fields, ARRAY_SIZE(NAME##_fields), #NAME, false /* packed */)

      if (state->is_version(110, 100)) {
         add_type(symbols, GET_STRUCT_TYPE(gl_DepthRangeParameters));
      }

      /* Add deprecated structure types.  While these were deprecated in 1.30,
      * they're still present.  We've removed them in 1.40+ (OpenGL 3.1+).
      */
      if (state->compat_shader || state->ARB_compatibility_enable) {
         add_type(symbols, GET_STRUCT_TYPE(gl_PointParameters));
         add_type(symbols, GET_STRUCT_TYPE(gl_MaterialParameters));
         add_type(symbols, GET_STRUCT_TYPE(gl_LightSourceParameters));
         add_type(symbols, GET_STRUCT_TYPE(gl_LightModelParameters));
         add_type(symbols, GET_STRUCT_TYPE(gl_LightModelProducts));
         add_type(symbols, GET_STRUCT_TYPE(gl_LightProducts));
         add_type(symbols, GET_STRUCT_TYPE(gl_FogParameters));
      };

      #undef GET_STRUCT_TYPE
   }

   /* Add types for enabled extensions.  They may have already been added
    * by the version-based loop, but attempting to add them a second time
    * is harmless.
    */
   if (state->ARB_texture_cube_map_array_enable ||
       state->EXT_texture_cube_map_array_enable ||
       state->OES_texture_cube_map_array_enable) {
      add_type(symbols, &glsl_type_builtin_samplerCubeArray);
      add_type(symbols, &glsl_type_builtin_samplerCubeArrayShadow);
      add_type(symbols, &glsl_type_builtin_isamplerCubeArray);
      add_type(symbols, &glsl_type_builtin_usamplerCubeArray);
   }

   if (state->ARB_texture_multisample_enable) {
      add_type(symbols, &glsl_type_builtin_sampler2DMS);
      add_type(symbols, &glsl_type_builtin_isampler2DMS);
      add_type(symbols, &glsl_type_builtin_usampler2DMS);
   }
   if (state->ARB_texture_multisample_enable ||
       state->OES_texture_storage_multisample_2d_array_enable) {
      add_type(symbols, &glsl_type_builtin_sampler2DMSArray);
      add_type(symbols, &glsl_type_builtin_isampler2DMSArray);
      add_type(symbols, &glsl_type_builtin_usampler2DMSArray);
   }

   if (state->ARB_texture_rectangle_enable) {
      add_type(symbols, &glsl_type_builtin_sampler2DRect);
      add_type(symbols, &glsl_type_builtin_sampler2DRectShadow);
   }

   if (state->EXT_gpu_shader4_enable) {
      add_type(symbols, &glsl_type_builtin_uint);
      add_type(symbols, &glsl_type_builtin_uvec2);
      add_type(symbols, &glsl_type_builtin_uvec3);
      add_type(symbols, &glsl_type_builtin_uvec4);

      add_type(symbols, &glsl_type_builtin_samplerCubeShadow);

      if (state->exts->EXT_texture_array) {
         add_type(symbols, &glsl_type_builtin_sampler1DArray);
         add_type(symbols, &glsl_type_builtin_sampler2DArray);
         add_type(symbols, &glsl_type_builtin_sampler1DArrayShadow);
         add_type(symbols, &glsl_type_builtin_sampler2DArrayShadow);
      }
      if (state->exts->EXT_texture_buffer_object) {
         add_type(symbols, &glsl_type_builtin_samplerBuffer);
      }

      if (state->exts->EXT_texture_integer) {
         add_type(symbols, &glsl_type_builtin_isampler1D);
         add_type(symbols, &glsl_type_builtin_isampler2D);
         add_type(symbols, &glsl_type_builtin_isampler3D);
         add_type(symbols, &glsl_type_builtin_isamplerCube);

         add_type(symbols, &glsl_type_builtin_usampler1D);
         add_type(symbols, &glsl_type_builtin_usampler2D);
         add_type(symbols, &glsl_type_builtin_usampler3D);
         add_type(symbols, &glsl_type_builtin_usamplerCube);

         if (state->exts->NV_texture_rectangle) {
            add_type(symbols, &glsl_type_builtin_isampler2DRect);
            add_type(symbols, &glsl_type_builtin_usampler2DRect);
         }
         if (state->exts->EXT_texture_array) {
            add_type(symbols, &glsl_type_builtin_isampler1DArray);
            add_type(symbols, &glsl_type_builtin_isampler2DArray);
            add_type(symbols, &glsl_type_builtin_usampler1DArray);
            add_type(symbols, &glsl_type_builtin_usampler2DArray);
         }
         if (state->exts->EXT_texture_buffer_object) {
            add_type(symbols, &glsl_type_builtin_isamplerBuffer);
            add_type(symbols, &glsl_type_builtin_usamplerBuffer);
         }
      }
   }

   if (state->EXT_texture_array_enable) {
      add_type(symbols, &glsl_type_builtin_sampler1DArray);
      add_type(symbols, &glsl_type_builtin_sampler2DArray);
      add_type(symbols, &glsl_type_builtin_sampler1DArrayShadow);
      add_type(symbols, &glsl_type_builtin_sampler2DArrayShadow);
   }

   if (state->OES_EGL_image_external_enable ||
       state->OES_EGL_image_external_essl3_enable) {
      add_type(symbols, &glsl_type_builtin_samplerExternalOES);
   }

   if (state->OES_texture_3D_enable) {
      add_type(symbols, &glsl_type_builtin_sampler3D);
   }

   if (state->ARB_shader_image_load_store_enable ||
       state->EXT_texture_cube_map_array_enable ||
       state->OES_texture_cube_map_array_enable) {
      add_type(symbols, &glsl_type_builtin_imageCubeArray);
      add_type(symbols, &glsl_type_builtin_iimageCubeArray);
      add_type(symbols, &glsl_type_builtin_uimageCubeArray);
   }

   if (state->ARB_shader_image_load_store_enable) {
      add_type(symbols, &glsl_type_builtin_image1D);
      add_type(symbols, &glsl_type_builtin_image2D);
      add_type(symbols, &glsl_type_builtin_image3D);
      add_type(symbols, &glsl_type_builtin_image2DRect);
      add_type(symbols, &glsl_type_builtin_imageCube);
      add_type(symbols, &glsl_type_builtin_imageBuffer);
      add_type(symbols, &glsl_type_builtin_image1DArray);
      add_type(symbols, &glsl_type_builtin_image2DArray);
      add_type(symbols, &glsl_type_builtin_image2DMS);
      add_type(symbols, &glsl_type_builtin_image2DMSArray);
      add_type(symbols, &glsl_type_builtin_iimage1D);
      add_type(symbols, &glsl_type_builtin_iimage2D);
      add_type(symbols, &glsl_type_builtin_iimage3D);
      add_type(symbols, &glsl_type_builtin_iimage2DRect);
      add_type(symbols, &glsl_type_builtin_iimageCube);
      add_type(symbols, &glsl_type_builtin_iimageBuffer);
      add_type(symbols, &glsl_type_builtin_iimage1DArray);
      add_type(symbols, &glsl_type_builtin_iimage2DArray);
      add_type(symbols, &glsl_type_builtin_iimage2DMS);
      add_type(symbols, &glsl_type_builtin_iimage2DMSArray);
      add_type(symbols, &glsl_type_builtin_uimage1D);
      add_type(symbols, &glsl_type_builtin_uimage2D);
      add_type(symbols, &glsl_type_builtin_uimage3D);
      add_type(symbols, &glsl_type_builtin_uimage2DRect);
      add_type(symbols, &glsl_type_builtin_uimageCube);
      add_type(symbols, &glsl_type_builtin_uimageBuffer);
      add_type(symbols, &glsl_type_builtin_uimage1DArray);
      add_type(symbols, &glsl_type_builtin_uimage2DArray);
      add_type(symbols, &glsl_type_builtin_uimage2DMS);
      add_type(symbols, &glsl_type_builtin_uimage2DMSArray);
   }

   if (state->EXT_texture_buffer_enable || state->OES_texture_buffer_enable) {
      add_type(symbols, &glsl_type_builtin_samplerBuffer);
      add_type(symbols, &glsl_type_builtin_isamplerBuffer);
      add_type(symbols, &glsl_type_builtin_usamplerBuffer);

      add_type(symbols, &glsl_type_builtin_imageBuffer);
      add_type(symbols, &glsl_type_builtin_iimageBuffer);
      add_type(symbols, &glsl_type_builtin_uimageBuffer);
   }

   if (state->has_atomic_counters()) {
      add_type(symbols, &glsl_type_builtin_atomic_uint);
   }

   if (state->ARB_gpu_shader_fp64_enable) {
      add_type(symbols, &glsl_type_builtin_double);
      add_type(symbols, &glsl_type_builtin_dvec2);
      add_type(symbols, &glsl_type_builtin_dvec3);
      add_type(symbols, &glsl_type_builtin_dvec4);
      add_type(symbols, &glsl_type_builtin_dmat2);
      add_type(symbols, &glsl_type_builtin_dmat3);
      add_type(symbols, &glsl_type_builtin_dmat4);
      add_type(symbols, &glsl_type_builtin_dmat2x3);
      add_type(symbols, &glsl_type_builtin_dmat2x4);
      add_type(symbols, &glsl_type_builtin_dmat3x2);
      add_type(symbols, &glsl_type_builtin_dmat3x4);
      add_type(symbols, &glsl_type_builtin_dmat4x2);
      add_type(symbols, &glsl_type_builtin_dmat4x3);
   }

   if (state->ARB_gpu_shader_int64_enable ||
       state->AMD_gpu_shader_int64_enable) {
      add_type(symbols, &glsl_type_builtin_int64_t);
      add_type(symbols, &glsl_type_builtin_i64vec2);
      add_type(symbols, &glsl_type_builtin_i64vec3);
      add_type(symbols, &glsl_type_builtin_i64vec4);

      add_type(symbols, &glsl_type_builtin_uint64_t);
      add_type(symbols, &glsl_type_builtin_u64vec2);
      add_type(symbols, &glsl_type_builtin_u64vec3);
      add_type(symbols, &glsl_type_builtin_u64vec4);
   }
}
/** @} */
