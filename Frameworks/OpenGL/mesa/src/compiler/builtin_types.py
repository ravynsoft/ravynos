# Copyright © 2013 Intel Corporation
# SPDX-License-Identifier: MIT

BUILTIN_TYPES = []

def simple_type(name, gl_type, base_type, rows, cols):
    BUILTIN_TYPES.append({
        "name": name,
        "gl_type": gl_type,
        "base_type": base_type,
        "vector_elements": rows,
        "matrix_columns": cols,
    })

def sampler_type(name, gl_type, base_type, dim, shadow, array, sampled_type):
    BUILTIN_TYPES.append({
        "name": name,
        "gl_type": gl_type,
        "base_type": base_type,
        "sampler_dimensionality": dim,
        "sampler_shadow": shadow,
        "sampler_array": array,
        "sampled_type": sampled_type,
        "vector_elements": 1,
        "matrix_columns": 1,
    })

def vector_type(base_name, vec_name, base_type, gl_type, extra_gl_type=None):
    if extra_gl_type is None:
        extra_gl_type = ""
    simple_type(base_name, gl_type + extra_gl_type, base_type, 1, 1)
    simple_type(vec_name + "2", gl_type + "_VEC2" + extra_gl_type, base_type, 2, 1)
    simple_type(vec_name + "3", gl_type + "_VEC3" + extra_gl_type, base_type, 3, 1)
    simple_type(vec_name + "4", gl_type + "_VEC4" + extra_gl_type, base_type, 4, 1)
    simple_type(vec_name + "5", None, base_type, 5, 1)
    simple_type(vec_name + "8", None, base_type, 8, 1)
    simple_type(vec_name + "16", None, base_type, 16, 1)

simple_type("error",  "GL_INVALID_ENUM", "GLSL_TYPE_ERROR", 0, 0)
simple_type("void",   "GL_INVALID_ENUM", "GLSL_TYPE_VOID",  0, 0)

vector_type("bool",      "bvec",   "GLSL_TYPE_BOOL",    "GL_BOOL")
vector_type("int",       "ivec",   "GLSL_TYPE_INT",     "GL_INT")
vector_type("uint",      "uvec",   "GLSL_TYPE_UINT",    "GL_UNSIGNED_INT")
vector_type("float",     "vec",    "GLSL_TYPE_FLOAT",   "GL_FLOAT")
vector_type("float16_t", "f16vec", "GLSL_TYPE_FLOAT16", "GL_FLOAT16", "_NV")
vector_type("double",    "dvec",   "GLSL_TYPE_DOUBLE",  "GL_DOUBLE")
vector_type("int64_t",   "i64vec", "GLSL_TYPE_INT64",   "GL_INT64", "_ARB")
vector_type("uint64_t",  "u64vec", "GLSL_TYPE_UINT64",  "GL_UNSIGNED_INT64", "_ARB")
vector_type("int16_t",   "i16vec", "GLSL_TYPE_INT16",   "GL_INT16", "_NV")
vector_type("uint16_t",  "u16vec", "GLSL_TYPE_UINT16",  "GL_UNSIGNED_INT16", "_NV")
vector_type("int8_t",    "i8vec",  "GLSL_TYPE_INT8",    "GL_INT8", "_NV")
vector_type("uint8_t",   "u8vec",  "GLSL_TYPE_UINT8",   "GL_UNSIGNED_INT8", "_NV")

simple_type("mat2",   "GL_FLOAT_MAT2",   "GLSL_TYPE_FLOAT", 2, 2)
simple_type("mat3",   "GL_FLOAT_MAT3",   "GLSL_TYPE_FLOAT", 3, 3)
simple_type("mat4",   "GL_FLOAT_MAT4",   "GLSL_TYPE_FLOAT", 4, 4)

simple_type("mat2x3", "GL_FLOAT_MAT2x3", "GLSL_TYPE_FLOAT", 3, 2)
simple_type("mat2x4", "GL_FLOAT_MAT2x4", "GLSL_TYPE_FLOAT", 4, 2)
simple_type("mat3x2", "GL_FLOAT_MAT3x2", "GLSL_TYPE_FLOAT", 2, 3)
simple_type("mat3x4", "GL_FLOAT_MAT3x4", "GLSL_TYPE_FLOAT", 4, 3)
simple_type("mat4x2", "GL_FLOAT_MAT4x2", "GLSL_TYPE_FLOAT", 2, 4)
simple_type("mat4x3", "GL_FLOAT_MAT4x3", "GLSL_TYPE_FLOAT", 3, 4)

simple_type("f16mat2",   "GL_FLOAT16_MAT2_AMD",   "GLSL_TYPE_FLOAT16", 2, 2)
simple_type("f16mat3",   "GL_FLOAT16_MAT3_AMD",   "GLSL_TYPE_FLOAT16", 3, 3)
simple_type("f16mat4",   "GL_FLOAT16_MAT4_AMD",   "GLSL_TYPE_FLOAT16", 4, 4)

simple_type("f16mat2x3", "GL_FLOAT16_MAT2x3_AMD", "GLSL_TYPE_FLOAT16", 3, 2)
simple_type("f16mat2x4", "GL_FLOAT16_MAT2x4_AMD", "GLSL_TYPE_FLOAT16", 4, 2)
simple_type("f16mat3x2", "GL_FLOAT16_MAT3x2_AMD", "GLSL_TYPE_FLOAT16", 2, 3)
simple_type("f16mat3x4", "GL_FLOAT16_MAT3x4_AMD", "GLSL_TYPE_FLOAT16", 4, 3)
simple_type("f16mat4x2", "GL_FLOAT16_MAT4x2_AMD", "GLSL_TYPE_FLOAT16", 2, 4)
simple_type("f16mat4x3", "GL_FLOAT16_MAT4x3_AMD", "GLSL_TYPE_FLOAT16", 3, 4)

simple_type("dmat2",   "GL_DOUBLE_MAT2",   "GLSL_TYPE_DOUBLE", 2, 2)
simple_type("dmat3",   "GL_DOUBLE_MAT3",   "GLSL_TYPE_DOUBLE", 3, 3)
simple_type("dmat4",   "GL_DOUBLE_MAT4",   "GLSL_TYPE_DOUBLE", 4, 4)

simple_type("dmat2x3", "GL_DOUBLE_MAT2x3", "GLSL_TYPE_DOUBLE", 3, 2)
simple_type("dmat2x4", "GL_DOUBLE_MAT2x4", "GLSL_TYPE_DOUBLE", 4, 2)
simple_type("dmat3x2", "GL_DOUBLE_MAT3x2", "GLSL_TYPE_DOUBLE", 2, 3)
simple_type("dmat3x4", "GL_DOUBLE_MAT3x4", "GLSL_TYPE_DOUBLE", 4, 3)
simple_type("dmat4x2", "GL_DOUBLE_MAT4x2", "GLSL_TYPE_DOUBLE", 2, 4)
simple_type("dmat4x3", "GL_DOUBLE_MAT4x3", "GLSL_TYPE_DOUBLE", 3, 4)

simple_type("atomic_uint", "GL_UNSIGNED_INT_ATOMIC_COUNTER", "GLSL_TYPE_ATOMIC_UINT", 1, 1)

sampler_type("sampler",           "GL_SAMPLER_1D",                   "GLSL_TYPE_SAMPLER", "GLSL_SAMPLER_DIM_1D",   0, 0, "GLSL_TYPE_VOID")
sampler_type("sampler1D",         "GL_SAMPLER_1D",                   "GLSL_TYPE_SAMPLER", "GLSL_SAMPLER_DIM_1D",   0, 0, "GLSL_TYPE_FLOAT")
sampler_type("sampler2D",         "GL_SAMPLER_2D",                   "GLSL_TYPE_SAMPLER", "GLSL_SAMPLER_DIM_2D",   0, 0, "GLSL_TYPE_FLOAT")
sampler_type("sampler3D",         "GL_SAMPLER_3D",                   "GLSL_TYPE_SAMPLER", "GLSL_SAMPLER_DIM_3D",   0, 0, "GLSL_TYPE_FLOAT")
sampler_type("samplerCube",       "GL_SAMPLER_CUBE",                 "GLSL_TYPE_SAMPLER", "GLSL_SAMPLER_DIM_CUBE", 0, 0, "GLSL_TYPE_FLOAT")
sampler_type("sampler1DArray",    "GL_SAMPLER_1D_ARRAY",             "GLSL_TYPE_SAMPLER", "GLSL_SAMPLER_DIM_1D",   0, 1, "GLSL_TYPE_FLOAT")
sampler_type("sampler2DArray",    "GL_SAMPLER_2D_ARRAY",             "GLSL_TYPE_SAMPLER", "GLSL_SAMPLER_DIM_2D",   0, 1, "GLSL_TYPE_FLOAT")
sampler_type("samplerCubeArray",  "GL_SAMPLER_CUBE_MAP_ARRAY",       "GLSL_TYPE_SAMPLER", "GLSL_SAMPLER_DIM_CUBE", 0, 1, "GLSL_TYPE_FLOAT")
sampler_type("sampler2DRect",     "GL_SAMPLER_2D_RECT",              "GLSL_TYPE_SAMPLER", "GLSL_SAMPLER_DIM_RECT", 0, 0, "GLSL_TYPE_FLOAT")
sampler_type("samplerBuffer",     "GL_SAMPLER_BUFFER",               "GLSL_TYPE_SAMPLER", "GLSL_SAMPLER_DIM_BUF",  0, 0, "GLSL_TYPE_FLOAT")
sampler_type("sampler2DMS",       "GL_SAMPLER_2D_MULTISAMPLE",       "GLSL_TYPE_SAMPLER", "GLSL_SAMPLER_DIM_MS",   0, 0, "GLSL_TYPE_FLOAT")
sampler_type("sampler2DMSArray",  "GL_SAMPLER_2D_MULTISAMPLE_ARRAY", "GLSL_TYPE_SAMPLER", "GLSL_SAMPLER_DIM_MS",   0, 1, "GLSL_TYPE_FLOAT")

sampler_type("isampler1D",        "GL_INT_SAMPLER_1D",                   "GLSL_TYPE_SAMPLER", "GLSL_SAMPLER_DIM_1D",   0, 0, "GLSL_TYPE_INT")
sampler_type("isampler2D",        "GL_INT_SAMPLER_2D",                   "GLSL_TYPE_SAMPLER", "GLSL_SAMPLER_DIM_2D",   0, 0, "GLSL_TYPE_INT")
sampler_type("isampler3D",        "GL_INT_SAMPLER_3D",                   "GLSL_TYPE_SAMPLER", "GLSL_SAMPLER_DIM_3D",   0, 0, "GLSL_TYPE_INT")
sampler_type("isamplerCube",      "GL_INT_SAMPLER_CUBE",                 "GLSL_TYPE_SAMPLER", "GLSL_SAMPLER_DIM_CUBE", 0, 0, "GLSL_TYPE_INT")
sampler_type("isampler1DArray",   "GL_INT_SAMPLER_1D_ARRAY",             "GLSL_TYPE_SAMPLER", "GLSL_SAMPLER_DIM_1D",   0, 1, "GLSL_TYPE_INT")
sampler_type("isampler2DArray",   "GL_INT_SAMPLER_2D_ARRAY",             "GLSL_TYPE_SAMPLER", "GLSL_SAMPLER_DIM_2D",   0, 1, "GLSL_TYPE_INT")
sampler_type("isamplerCubeArray", "GL_INT_SAMPLER_CUBE_MAP_ARRAY",       "GLSL_TYPE_SAMPLER", "GLSL_SAMPLER_DIM_CUBE", 0, 1, "GLSL_TYPE_INT")
sampler_type("isampler2DRect",    "GL_INT_SAMPLER_2D_RECT",              "GLSL_TYPE_SAMPLER", "GLSL_SAMPLER_DIM_RECT", 0, 0, "GLSL_TYPE_INT")
sampler_type("isamplerBuffer",    "GL_INT_SAMPLER_BUFFER",               "GLSL_TYPE_SAMPLER", "GLSL_SAMPLER_DIM_BUF",  0, 0, "GLSL_TYPE_INT")
sampler_type("isampler2DMS",      "GL_INT_SAMPLER_2D_MULTISAMPLE",       "GLSL_TYPE_SAMPLER", "GLSL_SAMPLER_DIM_MS",   0, 0, "GLSL_TYPE_INT")
sampler_type("isampler2DMSArray", "GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY", "GLSL_TYPE_SAMPLER", "GLSL_SAMPLER_DIM_MS",   0, 1, "GLSL_TYPE_INT")

sampler_type("usampler1D",        "GL_UNSIGNED_INT_SAMPLER_1D",                   "GLSL_TYPE_SAMPLER", "GLSL_SAMPLER_DIM_1D",   0, 0, "GLSL_TYPE_UINT")
sampler_type("usampler2D",        "GL_UNSIGNED_INT_SAMPLER_2D",                   "GLSL_TYPE_SAMPLER", "GLSL_SAMPLER_DIM_2D",   0, 0, "GLSL_TYPE_UINT")
sampler_type("usampler3D",        "GL_UNSIGNED_INT_SAMPLER_3D",                   "GLSL_TYPE_SAMPLER", "GLSL_SAMPLER_DIM_3D",   0, 0, "GLSL_TYPE_UINT")
sampler_type("usamplerCube",      "GL_UNSIGNED_INT_SAMPLER_CUBE",                 "GLSL_TYPE_SAMPLER", "GLSL_SAMPLER_DIM_CUBE", 0, 0, "GLSL_TYPE_UINT")
sampler_type("usampler1DArray",   "GL_UNSIGNED_INT_SAMPLER_1D_ARRAY",             "GLSL_TYPE_SAMPLER", "GLSL_SAMPLER_DIM_1D",   0, 1, "GLSL_TYPE_UINT")
sampler_type("usampler2DArray",   "GL_UNSIGNED_INT_SAMPLER_2D_ARRAY",             "GLSL_TYPE_SAMPLER", "GLSL_SAMPLER_DIM_2D",   0, 1, "GLSL_TYPE_UINT")
sampler_type("usamplerCubeArray", "GL_UNSIGNED_INT_SAMPLER_CUBE_MAP_ARRAY",       "GLSL_TYPE_SAMPLER", "GLSL_SAMPLER_DIM_CUBE", 0, 1, "GLSL_TYPE_UINT")
sampler_type("usampler2DRect",    "GL_UNSIGNED_INT_SAMPLER_2D_RECT",              "GLSL_TYPE_SAMPLER", "GLSL_SAMPLER_DIM_RECT", 0, 0, "GLSL_TYPE_UINT")
sampler_type("usamplerBuffer",    "GL_UNSIGNED_INT_SAMPLER_BUFFER",               "GLSL_TYPE_SAMPLER", "GLSL_SAMPLER_DIM_BUF",  0, 0, "GLSL_TYPE_UINT")
sampler_type("usampler2DMS",      "GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE",       "GLSL_TYPE_SAMPLER", "GLSL_SAMPLER_DIM_MS",   0, 0, "GLSL_TYPE_UINT")
sampler_type("usampler2DMSArray", "GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY", "GLSL_TYPE_SAMPLER", "GLSL_SAMPLER_DIM_MS",   0, 1, "GLSL_TYPE_UINT")

sampler_type("samplerShadow",          "GL_SAMPLER_1D_SHADOW",             "GLSL_TYPE_SAMPLER", "GLSL_SAMPLER_DIM_1D",       1, 0, "GLSL_TYPE_VOID")
sampler_type("sampler1DShadow",        "GL_SAMPLER_1D_SHADOW",             "GLSL_TYPE_SAMPLER", "GLSL_SAMPLER_DIM_1D",       1, 0, "GLSL_TYPE_FLOAT")
sampler_type("sampler2DShadow",        "GL_SAMPLER_2D_SHADOW",             "GLSL_TYPE_SAMPLER", "GLSL_SAMPLER_DIM_2D",       1, 0, "GLSL_TYPE_FLOAT")
sampler_type("samplerCubeShadow",      "GL_SAMPLER_CUBE_SHADOW",           "GLSL_TYPE_SAMPLER", "GLSL_SAMPLER_DIM_CUBE",     1, 0, "GLSL_TYPE_FLOAT")
sampler_type("sampler1DArrayShadow",   "GL_SAMPLER_1D_ARRAY_SHADOW",       "GLSL_TYPE_SAMPLER", "GLSL_SAMPLER_DIM_1D",       1, 1, "GLSL_TYPE_FLOAT")
sampler_type("sampler2DArrayShadow",   "GL_SAMPLER_2D_ARRAY_SHADOW",       "GLSL_TYPE_SAMPLER", "GLSL_SAMPLER_DIM_2D",       1, 1, "GLSL_TYPE_FLOAT")
sampler_type("samplerCubeArrayShadow", "GL_SAMPLER_CUBE_MAP_ARRAY_SHADOW", "GLSL_TYPE_SAMPLER", "GLSL_SAMPLER_DIM_CUBE",     1, 1, "GLSL_TYPE_FLOAT")
sampler_type("sampler2DRectShadow",    "GL_SAMPLER_2D_RECT_SHADOW",        "GLSL_TYPE_SAMPLER", "GLSL_SAMPLER_DIM_RECT",     1, 0, "GLSL_TYPE_FLOAT")

sampler_type("samplerExternalOES",     "GL_SAMPLER_EXTERNAL_OES",          "GLSL_TYPE_SAMPLER", "GLSL_SAMPLER_DIM_EXTERNAL", 0, 0, "GLSL_TYPE_FLOAT")

sampler_type("texture1D",         "GL_SAMPLER_1D",                   "GLSL_TYPE_TEXTURE", "GLSL_SAMPLER_DIM_1D",   0, 0, "GLSL_TYPE_FLOAT")
sampler_type("texture2D",         "GL_SAMPLER_2D",                   "GLSL_TYPE_TEXTURE", "GLSL_SAMPLER_DIM_2D",   0, 0, "GLSL_TYPE_FLOAT")
sampler_type("texture3D",         "GL_SAMPLER_3D",                   "GLSL_TYPE_TEXTURE", "GLSL_SAMPLER_DIM_3D",   0, 0, "GLSL_TYPE_FLOAT")
sampler_type("textureCube",       "GL_SAMPLER_CUBE",                 "GLSL_TYPE_TEXTURE", "GLSL_SAMPLER_DIM_CUBE", 0, 0, "GLSL_TYPE_FLOAT")
sampler_type("texture1DArray",    "GL_SAMPLER_1D_ARRAY",             "GLSL_TYPE_TEXTURE", "GLSL_SAMPLER_DIM_1D",   0, 1, "GLSL_TYPE_FLOAT")
sampler_type("texture2DArray",    "GL_SAMPLER_2D_ARRAY",             "GLSL_TYPE_TEXTURE", "GLSL_SAMPLER_DIM_2D",   0, 1, "GLSL_TYPE_FLOAT")
sampler_type("textureCubeArray",  "GL_SAMPLER_CUBE_MAP_ARRAY",       "GLSL_TYPE_TEXTURE", "GLSL_SAMPLER_DIM_CUBE", 0, 1, "GLSL_TYPE_FLOAT")
sampler_type("texture2DRect",     "GL_SAMPLER_2D_RECT",              "GLSL_TYPE_TEXTURE", "GLSL_SAMPLER_DIM_RECT", 0, 0, "GLSL_TYPE_FLOAT")
sampler_type("textureBuffer",     "GL_SAMPLER_BUFFER",               "GLSL_TYPE_TEXTURE", "GLSL_SAMPLER_DIM_BUF",  0, 0, "GLSL_TYPE_FLOAT")
sampler_type("texture2DMS",       "GL_SAMPLER_2D_MULTISAMPLE",       "GLSL_TYPE_TEXTURE", "GLSL_SAMPLER_DIM_MS",   0, 0, "GLSL_TYPE_FLOAT")
sampler_type("texture2DMSArray",  "GL_SAMPLER_2D_MULTISAMPLE_ARRAY", "GLSL_TYPE_TEXTURE", "GLSL_SAMPLER_DIM_MS",   0, 1, "GLSL_TYPE_FLOAT")

sampler_type("itexture1D",        "GL_INT_SAMPLER_1D",                   "GLSL_TYPE_TEXTURE", "GLSL_SAMPLER_DIM_1D",   0, 0, "GLSL_TYPE_INT")
sampler_type("itexture2D",        "GL_INT_SAMPLER_2D",                   "GLSL_TYPE_TEXTURE", "GLSL_SAMPLER_DIM_2D",   0, 0, "GLSL_TYPE_INT")
sampler_type("itexture3D",        "GL_INT_SAMPLER_3D",                   "GLSL_TYPE_TEXTURE", "GLSL_SAMPLER_DIM_3D",   0, 0, "GLSL_TYPE_INT")
sampler_type("itextureCube",      "GL_INT_SAMPLER_CUBE",                 "GLSL_TYPE_TEXTURE", "GLSL_SAMPLER_DIM_CUBE", 0, 0, "GLSL_TYPE_INT")
sampler_type("itexture1DArray",   "GL_INT_SAMPLER_1D_ARRAY",             "GLSL_TYPE_TEXTURE", "GLSL_SAMPLER_DIM_1D",   0, 1, "GLSL_TYPE_INT")
sampler_type("itexture2DArray",   "GL_INT_SAMPLER_2D_ARRAY",             "GLSL_TYPE_TEXTURE", "GLSL_SAMPLER_DIM_2D",   0, 1, "GLSL_TYPE_INT")
sampler_type("itextureCubeArray", "GL_INT_SAMPLER_CUBE_MAP_ARRAY",       "GLSL_TYPE_TEXTURE", "GLSL_SAMPLER_DIM_CUBE", 0, 1, "GLSL_TYPE_INT")
sampler_type("itexture2DRect",    "GL_INT_SAMPLER_2D_RECT",              "GLSL_TYPE_TEXTURE", "GLSL_SAMPLER_DIM_RECT", 0, 0, "GLSL_TYPE_INT")
sampler_type("itextureBuffer",    "GL_INT_SAMPLER_BUFFER",               "GLSL_TYPE_TEXTURE", "GLSL_SAMPLER_DIM_BUF",  0, 0, "GLSL_TYPE_INT")
sampler_type("itexture2DMS",      "GL_INT_SAMPLER_2D_MULTISAMPLE",       "GLSL_TYPE_TEXTURE", "GLSL_SAMPLER_DIM_MS",   0, 0, "GLSL_TYPE_INT")
sampler_type("itexture2DMSArray", "GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY", "GLSL_TYPE_TEXTURE", "GLSL_SAMPLER_DIM_MS",   0, 1, "GLSL_TYPE_INT")

sampler_type("utexture1D",        "GL_UNSIGNED_INT_SAMPLER_1D",                   "GLSL_TYPE_TEXTURE", "GLSL_SAMPLER_DIM_1D",   0, 0, "GLSL_TYPE_UINT")
sampler_type("utexture2D",        "GL_UNSIGNED_INT_SAMPLER_2D",                   "GLSL_TYPE_TEXTURE", "GLSL_SAMPLER_DIM_2D",   0, 0, "GLSL_TYPE_UINT")
sampler_type("utexture3D",        "GL_UNSIGNED_INT_SAMPLER_3D",                   "GLSL_TYPE_TEXTURE", "GLSL_SAMPLER_DIM_3D",   0, 0, "GLSL_TYPE_UINT")
sampler_type("utextureCube",      "GL_UNSIGNED_INT_SAMPLER_CUBE",                 "GLSL_TYPE_TEXTURE", "GLSL_SAMPLER_DIM_CUBE", 0, 0, "GLSL_TYPE_UINT")
sampler_type("utexture1DArray",   "GL_UNSIGNED_INT_SAMPLER_1D_ARRAY",             "GLSL_TYPE_TEXTURE", "GLSL_SAMPLER_DIM_1D",   0, 1, "GLSL_TYPE_UINT")
sampler_type("utexture2DArray",   "GL_UNSIGNED_INT_SAMPLER_2D_ARRAY",             "GLSL_TYPE_TEXTURE", "GLSL_SAMPLER_DIM_2D",   0, 1, "GLSL_TYPE_UINT")
sampler_type("utextureCubeArray", "GL_UNSIGNED_INT_SAMPLER_CUBE_MAP_ARRAY",       "GLSL_TYPE_TEXTURE", "GLSL_SAMPLER_DIM_CUBE", 0, 1, "GLSL_TYPE_UINT")
sampler_type("utexture2DRect",    "GL_UNSIGNED_INT_SAMPLER_2D_RECT",              "GLSL_TYPE_TEXTURE", "GLSL_SAMPLER_DIM_RECT", 0, 0, "GLSL_TYPE_UINT")
sampler_type("utextureBuffer",    "GL_UNSIGNED_INT_SAMPLER_BUFFER",               "GLSL_TYPE_TEXTURE", "GLSL_SAMPLER_DIM_BUF",  0, 0, "GLSL_TYPE_UINT")
sampler_type("utexture2DMS",      "GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE",       "GLSL_TYPE_TEXTURE", "GLSL_SAMPLER_DIM_MS",   0, 0, "GLSL_TYPE_UINT")
sampler_type("utexture2DMSArray", "GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY", "GLSL_TYPE_TEXTURE", "GLSL_SAMPLER_DIM_MS",   0, 1, "GLSL_TYPE_UINT")

sampler_type("textureExternalOES",     "GL_SAMPLER_EXTERNAL_OES",          "GLSL_TYPE_TEXTURE", "GLSL_SAMPLER_DIM_EXTERNAL", 0, 0, "GLSL_TYPE_FLOAT")

# OpenCL image types
sampler_type("vtexture1D",        "GL_SAMPLER_1D",       "GLSL_TYPE_TEXTURE", "GLSL_SAMPLER_DIM_1D",  0, 0, "GLSL_TYPE_VOID")
sampler_type("vtexture2D",        "GL_SAMPLER_2D",       "GLSL_TYPE_TEXTURE", "GLSL_SAMPLER_DIM_2D",  0, 0, "GLSL_TYPE_VOID")
sampler_type("vtexture3D",        "GL_SAMPLER_3D",       "GLSL_TYPE_TEXTURE", "GLSL_SAMPLER_DIM_3D",  0, 0, "GLSL_TYPE_VOID")
sampler_type("vtexture1DArray",   "GL_SAMPLER_1D_ARRAY", "GLSL_TYPE_TEXTURE", "GLSL_SAMPLER_DIM_1D",  0, 1, "GLSL_TYPE_VOID")
sampler_type("vtexture2DArray",   "GL_SAMPLER_2D_ARRAY", "GLSL_TYPE_TEXTURE", "GLSL_SAMPLER_DIM_2D",  0, 1, "GLSL_TYPE_VOID")
sampler_type("vtextureBuffer",    "GL_SAMPLER_BUFFER",   "GLSL_TYPE_TEXTURE", "GLSL_SAMPLER_DIM_BUF", 0, 0, "GLSL_TYPE_VOID")

sampler_type("image1D",         "GL_IMAGE_1D",                                "GLSL_TYPE_IMAGE", "GLSL_SAMPLER_DIM_1D",     0, 0, "GLSL_TYPE_FLOAT")
sampler_type("image2D",         "GL_IMAGE_2D",                                "GLSL_TYPE_IMAGE", "GLSL_SAMPLER_DIM_2D",     0, 0, "GLSL_TYPE_FLOAT")
sampler_type("image3D",         "GL_IMAGE_3D",                                "GLSL_TYPE_IMAGE", "GLSL_SAMPLER_DIM_3D",     0, 0, "GLSL_TYPE_FLOAT")
sampler_type("image2DRect",     "GL_IMAGE_2D_RECT",                           "GLSL_TYPE_IMAGE", "GLSL_SAMPLER_DIM_RECT",   0, 0, "GLSL_TYPE_FLOAT")
sampler_type("imageCube",       "GL_IMAGE_CUBE",                              "GLSL_TYPE_IMAGE", "GLSL_SAMPLER_DIM_CUBE",   0, 0, "GLSL_TYPE_FLOAT")
sampler_type("imageBuffer",     "GL_IMAGE_BUFFER",                            "GLSL_TYPE_IMAGE", "GLSL_SAMPLER_DIM_BUF",    0, 0, "GLSL_TYPE_FLOAT")
sampler_type("image1DArray",    "GL_IMAGE_1D_ARRAY",                          "GLSL_TYPE_IMAGE", "GLSL_SAMPLER_DIM_1D",     0, 1, "GLSL_TYPE_FLOAT")
sampler_type("image2DArray",    "GL_IMAGE_2D_ARRAY",                          "GLSL_TYPE_IMAGE", "GLSL_SAMPLER_DIM_2D",     0, 1, "GLSL_TYPE_FLOAT")
sampler_type("imageCubeArray",  "GL_IMAGE_CUBE_MAP_ARRAY",                    "GLSL_TYPE_IMAGE", "GLSL_SAMPLER_DIM_CUBE",   0, 1, "GLSL_TYPE_FLOAT")
sampler_type("image2DMS",       "GL_IMAGE_2D_MULTISAMPLE",                    "GLSL_TYPE_IMAGE", "GLSL_SAMPLER_DIM_MS",     0, 0, "GLSL_TYPE_FLOAT")
sampler_type("image2DMSArray",  "GL_IMAGE_2D_MULTISAMPLE_ARRAY",              "GLSL_TYPE_IMAGE", "GLSL_SAMPLER_DIM_MS",     0, 1, "GLSL_TYPE_FLOAT")
sampler_type("iimage1D",        "GL_INT_IMAGE_1D",                            "GLSL_TYPE_IMAGE", "GLSL_SAMPLER_DIM_1D",     0, 0, "GLSL_TYPE_INT")
sampler_type("iimage2D",        "GL_INT_IMAGE_2D",                            "GLSL_TYPE_IMAGE", "GLSL_SAMPLER_DIM_2D",     0, 0, "GLSL_TYPE_INT")
sampler_type("iimage3D",        "GL_INT_IMAGE_3D",                            "GLSL_TYPE_IMAGE", "GLSL_SAMPLER_DIM_3D",     0, 0, "GLSL_TYPE_INT")
sampler_type("iimage2DRect",    "GL_INT_IMAGE_2D_RECT",                       "GLSL_TYPE_IMAGE", "GLSL_SAMPLER_DIM_RECT",   0, 0, "GLSL_TYPE_INT")
sampler_type("iimageCube",      "GL_INT_IMAGE_CUBE",                          "GLSL_TYPE_IMAGE", "GLSL_SAMPLER_DIM_CUBE",   0, 0, "GLSL_TYPE_INT")
sampler_type("iimageBuffer",    "GL_INT_IMAGE_BUFFER",                        "GLSL_TYPE_IMAGE", "GLSL_SAMPLER_DIM_BUF",    0, 0, "GLSL_TYPE_INT")
sampler_type("iimage1DArray",   "GL_INT_IMAGE_1D_ARRAY",                      "GLSL_TYPE_IMAGE", "GLSL_SAMPLER_DIM_1D",     0, 1, "GLSL_TYPE_INT")
sampler_type("iimage2DArray",   "GL_INT_IMAGE_2D_ARRAY",                      "GLSL_TYPE_IMAGE", "GLSL_SAMPLER_DIM_2D",     0, 1, "GLSL_TYPE_INT")
sampler_type("iimageCubeArray", "GL_INT_IMAGE_CUBE_MAP_ARRAY",                "GLSL_TYPE_IMAGE", "GLSL_SAMPLER_DIM_CUBE",   0, 1, "GLSL_TYPE_INT")
sampler_type("iimage2DMS",      "GL_INT_IMAGE_2D_MULTISAMPLE",                "GLSL_TYPE_IMAGE", "GLSL_SAMPLER_DIM_MS",     0, 0, "GLSL_TYPE_INT")
sampler_type("iimage2DMSArray", "GL_INT_IMAGE_2D_MULTISAMPLE_ARRAY",          "GLSL_TYPE_IMAGE", "GLSL_SAMPLER_DIM_MS",     0, 1, "GLSL_TYPE_INT")
sampler_type("uimage1D",        "GL_UNSIGNED_INT_IMAGE_1D",                   "GLSL_TYPE_IMAGE", "GLSL_SAMPLER_DIM_1D",     0, 0, "GLSL_TYPE_UINT")
sampler_type("uimage2D",        "GL_UNSIGNED_INT_IMAGE_2D",                   "GLSL_TYPE_IMAGE", "GLSL_SAMPLER_DIM_2D",     0, 0, "GLSL_TYPE_UINT")
sampler_type("uimage3D",        "GL_UNSIGNED_INT_IMAGE_3D",                   "GLSL_TYPE_IMAGE", "GLSL_SAMPLER_DIM_3D",     0, 0, "GLSL_TYPE_UINT")
sampler_type("uimage2DRect",    "GL_UNSIGNED_INT_IMAGE_2D_RECT",              "GLSL_TYPE_IMAGE", "GLSL_SAMPLER_DIM_RECT",   0, 0, "GLSL_TYPE_UINT")
sampler_type("uimageCube",      "GL_UNSIGNED_INT_IMAGE_CUBE",                 "GLSL_TYPE_IMAGE", "GLSL_SAMPLER_DIM_CUBE",   0, 0, "GLSL_TYPE_UINT")
sampler_type("uimageBuffer",    "GL_UNSIGNED_INT_IMAGE_BUFFER",               "GLSL_TYPE_IMAGE", "GLSL_SAMPLER_DIM_BUF",    0, 0, "GLSL_TYPE_UINT")
sampler_type("uimage1DArray",   "GL_UNSIGNED_INT_IMAGE_1D_ARRAY",             "GLSL_TYPE_IMAGE", "GLSL_SAMPLER_DIM_1D",     0, 1, "GLSL_TYPE_UINT")
sampler_type("uimage2DArray",   "GL_UNSIGNED_INT_IMAGE_2D_ARRAY",             "GLSL_TYPE_IMAGE", "GLSL_SAMPLER_DIM_2D",     0, 1, "GLSL_TYPE_UINT")
sampler_type("uimageCubeArray", "GL_UNSIGNED_INT_IMAGE_CUBE_MAP_ARRAY",       "GLSL_TYPE_IMAGE", "GLSL_SAMPLER_DIM_CUBE",   0, 1, "GLSL_TYPE_UINT")
sampler_type("uimage2DMS",      "GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE",       "GLSL_TYPE_IMAGE", "GLSL_SAMPLER_DIM_MS",     0, 0, "GLSL_TYPE_UINT")
sampler_type("uimage2DMSArray", "GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE_ARRAY", "GLSL_TYPE_IMAGE", "GLSL_SAMPLER_DIM_MS",     0, 1, "GLSL_TYPE_UINT")
sampler_type("i64image1D",        "GL_INT_IMAGE_1D",                          "GLSL_TYPE_IMAGE", "GLSL_SAMPLER_DIM_1D",     0, 0, "GLSL_TYPE_INT64")
sampler_type("i64image2D",        "GL_INT_IMAGE_2D",                          "GLSL_TYPE_IMAGE", "GLSL_SAMPLER_DIM_2D",     0, 0, "GLSL_TYPE_INT64")
sampler_type("i64image3D",        "GL_INT_IMAGE_3D",                          "GLSL_TYPE_IMAGE", "GLSL_SAMPLER_DIM_3D",     0, 0, "GLSL_TYPE_INT64")
sampler_type("i64image2DRect",    "GL_INT_IMAGE_2D_RECT",                     "GLSL_TYPE_IMAGE", "GLSL_SAMPLER_DIM_RECT",   0, 0, "GLSL_TYPE_INT64")
sampler_type("i64imageCube",      "GL_INT_IMAGE_CUBE",                        "GLSL_TYPE_IMAGE", "GLSL_SAMPLER_DIM_CUBE",   0, 0, "GLSL_TYPE_INT64")
sampler_type("i64imageBuffer",    "GL_INT_IMAGE_BUFFER",                      "GLSL_TYPE_IMAGE", "GLSL_SAMPLER_DIM_BUF",    0, 0, "GLSL_TYPE_INT64")
sampler_type("i64image1DArray",   "GL_INT_IMAGE_1D_ARRAY",                    "GLSL_TYPE_IMAGE", "GLSL_SAMPLER_DIM_1D",     0, 1, "GLSL_TYPE_INT64")
sampler_type("i64image2DArray",   "GL_INT_IMAGE_2D_ARRAY",                    "GLSL_TYPE_IMAGE", "GLSL_SAMPLER_DIM_2D",     0, 1, "GLSL_TYPE_INT64")
sampler_type("i64imageCubeArray", "GL_INT_IMAGE_CUBE_MAP_ARRAY",              "GLSL_TYPE_IMAGE", "GLSL_SAMPLER_DIM_CUBE",   0, 1, "GLSL_TYPE_INT64")
sampler_type("i64image2DMS",      "GL_INT_IMAGE_2D_MULTISAMPLE",              "GLSL_TYPE_IMAGE", "GLSL_SAMPLER_DIM_MS",     0, 0, "GLSL_TYPE_INT64")
sampler_type("i64image2DMSArray", "GL_INT_IMAGE_2D_MULTISAMPLE_ARRAY",        "GLSL_TYPE_IMAGE", "GLSL_SAMPLER_DIM_MS",     0, 1, "GLSL_TYPE_INT64")
sampler_type("u64image1D",        "GL_UNSIGNED_INT_IMAGE_1D",                 "GLSL_TYPE_IMAGE", "GLSL_SAMPLER_DIM_1D",     0, 0, "GLSL_TYPE_UINT64")
sampler_type("u64image2D",        "GL_UNSIGNED_INT_IMAGE_2D",                 "GLSL_TYPE_IMAGE", "GLSL_SAMPLER_DIM_2D",     0, 0, "GLSL_TYPE_UINT64")
sampler_type("u64image3D",        "GL_UNSIGNED_INT_IMAGE_3D",                 "GLSL_TYPE_IMAGE", "GLSL_SAMPLER_DIM_3D",     0, 0, "GLSL_TYPE_UINT64")
sampler_type("u64image2DRect",    "GL_UNSIGNED_INT_IMAGE_2D_RECT",            "GLSL_TYPE_IMAGE", "GLSL_SAMPLER_DIM_RECT",   0, 0, "GLSL_TYPE_UINT64")
sampler_type("u64imageCube",      "GL_UNSIGNED_INT_IMAGE_CUBE",               "GLSL_TYPE_IMAGE", "GLSL_SAMPLER_DIM_CUBE",   0, 0, "GLSL_TYPE_UINT64")
sampler_type("u64imageBuffer",    "GL_UNSIGNED_INT_IMAGE_BUFFER",             "GLSL_TYPE_IMAGE", "GLSL_SAMPLER_DIM_BUF",    0, 0, "GLSL_TYPE_UINT64")
sampler_type("u64image1DArray",   "GL_UNSIGNED_INT_IMAGE_1D_ARRAY",           "GLSL_TYPE_IMAGE", "GLSL_SAMPLER_DIM_1D",     0, 1, "GLSL_TYPE_UINT64")
sampler_type("u64image2DArray",   "GL_UNSIGNED_INT_IMAGE_2D_ARRAY",           "GLSL_TYPE_IMAGE", "GLSL_SAMPLER_DIM_2D",     0, 1, "GLSL_TYPE_UINT64")
sampler_type("u64imageCubeArray", "GL_UNSIGNED_INT_IMAGE_CUBE_MAP_ARRAY",     "GLSL_TYPE_IMAGE", "GLSL_SAMPLER_DIM_CUBE",   0, 1, "GLSL_TYPE_UINT64")
sampler_type("u64image2DMS",      "GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE",     "GLSL_TYPE_IMAGE", "GLSL_SAMPLER_DIM_MS",     0, 0, "GLSL_TYPE_UINT64")
sampler_type("u64image2DMSArray", "GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE_ARRAY", "GLSL_TYPE_IMAGE", "GLSL_SAMPLER_DIM_MS",     0, 1, "GLSL_TYPE_UINT64")

# OpenCL image types
sampler_type("vbuffer", "GL_IMAGE_BUFFER", "GLSL_TYPE_IMAGE", "GLSL_SAMPLER_DIM_BUF", 0, 0, "GLSL_TYPE_VOID")
sampler_type("vimage1D", "GL_IMAGE_1D", "GLSL_TYPE_IMAGE", "GLSL_SAMPLER_DIM_1D", 0, 0, "GLSL_TYPE_VOID")
sampler_type("vimage2D", "GL_IMAGE_2D", "GLSL_TYPE_IMAGE", "GLSL_SAMPLER_DIM_2D", 0, 0, "GLSL_TYPE_VOID")
sampler_type("vimage3D", "GL_IMAGE_3D", "GLSL_TYPE_IMAGE", "GLSL_SAMPLER_DIM_3D", 0, 0, "GLSL_TYPE_VOID")
sampler_type("vimage1DArray", "GL_IMAGE_1D_ARRAY", "GLSL_TYPE_IMAGE", "GLSL_SAMPLER_DIM_1D", 0, 1, "GLSL_TYPE_VOID")
sampler_type("vimage2DArray", "GL_IMAGE_2D_ARRAY", "GLSL_TYPE_IMAGE", "GLSL_SAMPLER_DIM_2D", 0, 1, "GLSL_TYPE_VOID")

sampler_type("subpassInput",           "0",                                   "GLSL_TYPE_IMAGE", "GLSL_SAMPLER_DIM_SUBPASS",    0, 0, "GLSL_TYPE_FLOAT")
sampler_type("subpassInputMS",         "0",                                   "GLSL_TYPE_IMAGE", "GLSL_SAMPLER_DIM_SUBPASS_MS", 0, 0, "GLSL_TYPE_FLOAT")
sampler_type("isubpassInput",          "0",                                   "GLSL_TYPE_IMAGE", "GLSL_SAMPLER_DIM_SUBPASS",    0, 0, "GLSL_TYPE_INT")
sampler_type("isubpassInputMS",        "0",                                   "GLSL_TYPE_IMAGE", "GLSL_SAMPLER_DIM_SUBPASS_MS", 0, 0, "GLSL_TYPE_INT")
sampler_type("usubpassInput",          "0",                                   "GLSL_TYPE_IMAGE", "GLSL_SAMPLER_DIM_SUBPASS",    0, 0, "GLSL_TYPE_UINT")
sampler_type("usubpassInputMS",        "0",                                   "GLSL_TYPE_IMAGE", "GLSL_SAMPLER_DIM_SUBPASS_MS", 0, 0, "GLSL_TYPE_UINT")
sampler_type("textureSubpassInput",    "0",                                 "GLSL_TYPE_TEXTURE", "GLSL_SAMPLER_DIM_SUBPASS",    0, 0, "GLSL_TYPE_FLOAT")
sampler_type("textureSubpassInputMS",  "0",                                 "GLSL_TYPE_TEXTURE", "GLSL_SAMPLER_DIM_SUBPASS_MS", 0, 0, "GLSL_TYPE_FLOAT")
sampler_type("itextureSubpassInput",   "0",                                 "GLSL_TYPE_TEXTURE", "GLSL_SAMPLER_DIM_SUBPASS",    0, 0, "GLSL_TYPE_INT")
sampler_type("itextureSubpassInputMS", "0",                                 "GLSL_TYPE_TEXTURE", "GLSL_SAMPLER_DIM_SUBPASS_MS", 0, 0, "GLSL_TYPE_INT")
sampler_type("utextureSubpassInput",   "0",                                 "GLSL_TYPE_TEXTURE", "GLSL_SAMPLER_DIM_SUBPASS",    0, 0, "GLSL_TYPE_UINT")
sampler_type("utextureSubpassInputMS", "0",                                 "GLSL_TYPE_TEXTURE", "GLSL_SAMPLER_DIM_SUBPASS_MS", 0, 0, "GLSL_TYPE_UINT")
