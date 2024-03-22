/**************************************************************************
 *
 * Copyright 2007 VMware, Inc.
 * Copyright (c) 2008 VMware, Inc.
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

#include "compiler/nir/nir.h"


#include "main/context.h"
#include "main/macros.h"
#include "main/spirv_extensions.h"
#include "main/version.h"
#include "nir/nir_to_tgsi.h"

#include "pipe/p_context.h"
#include "pipe/p_defines.h"
#include "pipe/p_screen.h"
#include "tgsi/tgsi_from_mesa.h"
#include "util/u_math.h"
#include "util/u_memory.h"

#include "st_context.h"
#include "st_debug.h"
#include "st_extensions.h"
#include "st_format.h"


/*
 * Note: we use these function rather than the MIN2, MAX2, CLAMP macros to
 * avoid evaluating arguments (which are often function calls) more than once.
 */

static unsigned _min(unsigned a, unsigned b)
{
   return (a < b) ? a : b;
}

static float _maxf(float a, float b)
{
   return (a > b) ? a : b;
}

static int _clamp(int a, int min, int max)
{
   if (a < min)
      return min;
   else if (a > max)
      return max;
   else
      return a;
}


/**
 * Query driver to get implementation limits.
 * Note that we have to limit/clamp against Mesa's internal limits too.
 */
void st_init_limits(struct pipe_screen *screen,
                    struct gl_constants *c, struct gl_extensions *extensions,
                    gl_api api)
{
   unsigned sh;
   bool can_ubo = true;
   int temp;

   c->MaxTextureSize = screen->get_param(screen, PIPE_CAP_MAX_TEXTURE_2D_SIZE);
   c->MaxTextureSize = MIN2(c->MaxTextureSize, 1 << (MAX_TEXTURE_LEVELS - 1));
   c->MaxTextureMbytes = MAX2(c->MaxTextureMbytes,
                              screen->get_param(screen, PIPE_CAP_MAX_TEXTURE_MB));

   c->Max3DTextureLevels
      = _min(screen->get_param(screen, PIPE_CAP_MAX_TEXTURE_3D_LEVELS),
            MAX_TEXTURE_LEVELS);
   extensions->OES_texture_3D = c->Max3DTextureLevels != 0;

   c->MaxCubeTextureLevels
      = _min(screen->get_param(screen, PIPE_CAP_MAX_TEXTURE_CUBE_LEVELS),
            MAX_TEXTURE_LEVELS);

   c->MaxTextureRectSize = _min(c->MaxTextureSize, MAX_TEXTURE_RECT_SIZE);

   c->MaxArrayTextureLayers
      = screen->get_param(screen, PIPE_CAP_MAX_TEXTURE_ARRAY_LAYERS);

   /* Define max viewport size and max renderbuffer size in terms of
    * max texture size (note: max tex RECT size = max tex 2D size).
    * If this isn't true for some hardware we'll need new PIPE_CAP_ queries.
    */
   c->MaxViewportWidth =
   c->MaxViewportHeight =
   c->MaxRenderbufferSize = c->MaxTextureRectSize;

   c->SubPixelBits =
      screen->get_param(screen, PIPE_CAP_RASTERIZER_SUBPIXEL_BITS);
   c->ViewportSubpixelBits =
      screen->get_param(screen, PIPE_CAP_VIEWPORT_SUBPIXEL_BITS);

   c->MaxDrawBuffers = c->MaxColorAttachments =
      _clamp(screen->get_param(screen, PIPE_CAP_MAX_RENDER_TARGETS),
             1, MAX_DRAW_BUFFERS);

   c->MaxDualSourceDrawBuffers =
      _clamp(screen->get_param(screen,
                               PIPE_CAP_MAX_DUAL_SOURCE_RENDER_TARGETS),
             0, MAX_DRAW_BUFFERS);

   c->MaxLineWidth =
      _maxf(1.0f, screen->get_paramf(screen, PIPE_CAPF_MAX_LINE_WIDTH));
   c->MaxLineWidthAA =
      _maxf(1.0f, screen->get_paramf(screen, PIPE_CAPF_MAX_LINE_WIDTH_AA));

   c->MinLineWidth = screen->get_paramf(screen, PIPE_CAPF_MIN_LINE_WIDTH);
   c->MinLineWidthAA = screen->get_paramf(screen, PIPE_CAPF_MIN_LINE_WIDTH_AA);
   c->LineWidthGranularity = screen->get_paramf(screen, PIPE_CAPF_LINE_WIDTH_GRANULARITY);

   c->MaxPointSize =
      _maxf(1.0f, screen->get_paramf(screen, PIPE_CAPF_MAX_POINT_SIZE));
   c->MaxPointSizeAA =
      _maxf(1.0f, screen->get_paramf(screen, PIPE_CAPF_MAX_POINT_SIZE_AA));

   c->MinPointSize = MAX2(screen->get_paramf(screen, PIPE_CAPF_MIN_POINT_SIZE), 0.01);
   c->MinPointSizeAA = MAX2(screen->get_paramf(screen, PIPE_CAPF_MIN_POINT_SIZE_AA), 0.01);
   c->PointSizeGranularity = screen->get_paramf(screen, PIPE_CAPF_POINT_SIZE_GRANULARITY);

   c->MaxTextureMaxAnisotropy =
      _maxf(2.0f,
            screen->get_paramf(screen, PIPE_CAPF_MAX_TEXTURE_ANISOTROPY));

   c->MaxTextureLodBias =
      screen->get_paramf(screen, PIPE_CAPF_MAX_TEXTURE_LOD_BIAS);

   c->QuadsFollowProvokingVertexConvention =
      screen->get_param(screen,
                        PIPE_CAP_QUADS_FOLLOW_PROVOKING_VERTEX_CONVENTION);

   c->MaxUniformBlockSize =
      screen->get_param(screen, PIPE_CAP_MAX_CONSTANT_BUFFER_SIZE_UINT);

   if (c->MaxUniformBlockSize < 16384) {
      can_ubo = false;
   }

   /* Round down to a multiple of 4 to make piglit happy. Bytes are not
    * addressible by UBOs anyway.
    */
   c->MaxUniformBlockSize &= ~3;

   c->HasFBFetch = screen->get_param(screen, PIPE_CAP_FBFETCH);

   c->SupportsReadingOutputs = screen->get_param(screen, PIPE_CAP_SHADER_CAN_READ_OUTPUTS);

   c->CombinedClipCullDistanceArrays = !screen->get_param(screen, PIPE_CAP_CULL_DISTANCE_NOCOMBINE);

   c->PointSizeFixed = screen->get_param(screen, PIPE_CAP_POINT_SIZE_FIXED);

   for (sh = 0; sh < PIPE_SHADER_TYPES; ++sh) {
      const gl_shader_stage stage = tgsi_processor_to_shader_stage(sh);
      struct gl_shader_compiler_options *options =
         &c->ShaderCompilerOptions[stage];
      struct gl_program_constants *pc = &c->Program[stage];

      if (screen->get_compiler_options)
         options->NirOptions = screen->get_compiler_options(screen, PIPE_SHADER_IR_NIR, sh);

      if (!options->NirOptions) {
         options->NirOptions =
            nir_to_tgsi_get_compiler_options(screen, PIPE_SHADER_IR_NIR, sh);
      }

      if (sh == PIPE_SHADER_COMPUTE) {
         if (!screen->get_param(screen, PIPE_CAP_COMPUTE))
            continue;
      }

      pc->MaxTextureImageUnits =
         _min(screen->get_shader_param(screen, sh,
                                       PIPE_SHADER_CAP_MAX_TEXTURE_SAMPLERS),
              MAX_TEXTURE_IMAGE_UNITS);

      pc->MaxInstructions =
      pc->MaxNativeInstructions =
         screen->get_shader_param(screen, sh, PIPE_SHADER_CAP_MAX_INSTRUCTIONS);
      pc->MaxAluInstructions =
      pc->MaxNativeAluInstructions =
         screen->get_shader_param(screen, sh,
                                  PIPE_SHADER_CAP_MAX_ALU_INSTRUCTIONS);
      pc->MaxTexInstructions =
      pc->MaxNativeTexInstructions =
         screen->get_shader_param(screen, sh,
                                  PIPE_SHADER_CAP_MAX_TEX_INSTRUCTIONS);
      pc->MaxTexIndirections =
      pc->MaxNativeTexIndirections =
         screen->get_shader_param(screen, sh,
                                  PIPE_SHADER_CAP_MAX_TEX_INDIRECTIONS);
      pc->MaxAttribs =
      pc->MaxNativeAttribs =
         screen->get_shader_param(screen, sh, PIPE_SHADER_CAP_MAX_INPUTS);
      pc->MaxTemps =
      pc->MaxNativeTemps =
         screen->get_shader_param(screen, sh, PIPE_SHADER_CAP_MAX_TEMPS);
      pc->MaxAddressRegs =
      pc->MaxNativeAddressRegs = sh == PIPE_SHADER_VERTEX ? 1 : 0;

      pc->MaxUniformComponents =
         screen->get_shader_param(screen, sh,
                                  PIPE_SHADER_CAP_MAX_CONST_BUFFER0_SIZE) / 4;

      /* reserve space in the default-uniform for lowered state */
      if (sh == PIPE_SHADER_VERTEX ||
          sh == PIPE_SHADER_TESS_EVAL ||
          sh == PIPE_SHADER_GEOMETRY) {

         if (!screen->get_param(screen, PIPE_CAP_CLIP_PLANES))
            pc->MaxUniformComponents -= 4 * MAX_CLIP_PLANES;

         if (!screen->get_param(screen, PIPE_CAP_POINT_SIZE_FIXED))
            pc->MaxUniformComponents -= 4;
      } else if (sh == PIPE_SHADER_FRAGMENT) {
         if (!screen->get_param(screen, PIPE_CAP_ALPHA_TEST))
            pc->MaxUniformComponents -= 4;
      }


      pc->MaxUniformComponents = MIN2(pc->MaxUniformComponents,
                                      MAX_UNIFORMS * 4);

      /* For ARB programs, prog_src_register::Index is a signed 13-bit number.
       * This gives us a limit of 4096 values - but we may need to generate
       * internal values in addition to what the source program uses.  So, we
       * drop the limit one step lower, to 2048, to be safe.
       */
      pc->MaxParameters =
      pc->MaxNativeParameters = MIN2(pc->MaxUniformComponents / 4, 2048);
      pc->MaxInputComponents =
         screen->get_shader_param(screen, sh, PIPE_SHADER_CAP_MAX_INPUTS) * 4;
      pc->MaxOutputComponents =
         screen->get_shader_param(screen, sh, PIPE_SHADER_CAP_MAX_OUTPUTS) * 4;


      pc->MaxUniformBlocks =
         screen->get_shader_param(screen, sh,
                                  PIPE_SHADER_CAP_MAX_CONST_BUFFERS);
      if (pc->MaxUniformBlocks)
         pc->MaxUniformBlocks -= 1; /* The first one is for ordinary uniforms. */
      pc->MaxUniformBlocks = _min(pc->MaxUniformBlocks, MAX_UNIFORM_BUFFERS);

      pc->MaxCombinedUniformComponents =
         pc->MaxUniformComponents +
         (uint64_t)c->MaxUniformBlockSize / 4 * pc->MaxUniformBlocks;

      pc->MaxShaderStorageBlocks =
         screen->get_shader_param(screen, sh,
                                  PIPE_SHADER_CAP_MAX_SHADER_BUFFERS);

      temp = screen->get_shader_param(screen, sh, PIPE_SHADER_CAP_MAX_HW_ATOMIC_COUNTERS);
      if (temp) {
         /*
          * for separate atomic counters get the actual hw limits
          * per stage on atomic counters and buffers
          */
         pc->MaxAtomicCounters = temp;
         pc->MaxAtomicBuffers = screen->get_shader_param(screen, sh, PIPE_SHADER_CAP_MAX_HW_ATOMIC_COUNTER_BUFFERS);
      } else if (pc->MaxShaderStorageBlocks) {
         pc->MaxAtomicCounters = MAX_ATOMIC_COUNTERS;
         /*
          * without separate atomic counters, reserve half of the available
          * SSBOs for atomic buffers, and the other half for normal SSBOs.
          */
         pc->MaxAtomicBuffers = pc->MaxShaderStorageBlocks / 2;
         pc->MaxShaderStorageBlocks -= pc->MaxAtomicBuffers;
      }
      pc->MaxImageUniforms =
         _min(screen->get_shader_param(screen, sh,
                                       PIPE_SHADER_CAP_MAX_SHADER_IMAGES),
              MAX_IMAGE_UNIFORMS);

      /* Gallium doesn't really care about local vs. env parameters so use the
       * same limits.
       */
      pc->MaxLocalParams = MIN2(pc->MaxParameters, MAX_PROGRAM_LOCAL_PARAMS);
      pc->MaxEnvParams = MIN2(pc->MaxParameters, MAX_PROGRAM_ENV_PARAMS);

      if (screen->get_shader_param(screen, sh, PIPE_SHADER_CAP_INTEGERS)) {
         pc->LowInt.RangeMin = 31;
         pc->LowInt.RangeMax = 30;
         pc->LowInt.Precision = 0;
         pc->MediumInt = pc->HighInt = pc->LowInt;

         if (screen->get_shader_param(screen, sh, PIPE_SHADER_CAP_INT16)) {
            pc->LowInt.RangeMin = 15;
            pc->LowInt.RangeMax = 14;
            pc->MediumInt = pc->LowInt;
         }
      }

      if (screen->get_shader_param(screen, sh, PIPE_SHADER_CAP_FP16)) {
         pc->LowFloat.RangeMin = 15;
         pc->LowFloat.RangeMax = 15;
         pc->LowFloat.Precision = 10;
         pc->MediumFloat = pc->LowFloat;
      }

      /* TODO: make these more fine-grained if anyone needs it */
      options->MaxIfDepth =
         screen->get_shader_param(screen, sh,
                                  PIPE_SHADER_CAP_MAX_CONTROL_FLOW_DEPTH);

      options->EmitNoMainReturn =
         !screen->get_shader_param(screen, sh, PIPE_SHADER_CAP_SUBROUTINES);

      options->EmitNoCont =
         !screen->get_shader_param(screen, sh,
                                   PIPE_SHADER_CAP_CONT_SUPPORTED);

      options->EmitNoIndirectInput =
         !screen->get_shader_param(screen, sh,
                                   PIPE_SHADER_CAP_INDIRECT_INPUT_ADDR);
      options->EmitNoIndirectOutput =
         !screen->get_shader_param(screen, sh,
                                   PIPE_SHADER_CAP_INDIRECT_OUTPUT_ADDR);
      options->EmitNoIndirectTemp =
         !screen->get_shader_param(screen, sh,
                                   PIPE_SHADER_CAP_INDIRECT_TEMP_ADDR);
      options->EmitNoIndirectUniform =
         !screen->get_shader_param(screen, sh,
                                   PIPE_SHADER_CAP_INDIRECT_CONST_ADDR);

      if (pc->MaxNativeInstructions &&
          (options->EmitNoIndirectUniform || pc->MaxUniformBlocks < 12)) {
         can_ubo = false;
      }

      if (!screen->get_param(screen, PIPE_CAP_NIR_COMPACT_ARRAYS))
         options->LowerCombinedClipCullDistance = true;

      if (sh == PIPE_SHADER_VERTEX || sh == PIPE_SHADER_GEOMETRY) {
         if (screen->get_param(screen, PIPE_CAP_VIEWPORT_TRANSFORM_LOWERED))
            options->LowerBuiltinVariablesXfb |= VARYING_BIT_POS;
         if (screen->get_param(screen, PIPE_CAP_PSIZ_CLAMPED))
            options->LowerBuiltinVariablesXfb |= VARYING_BIT_PSIZ;
      }

      options->LowerPrecisionFloat16 =
         screen->get_shader_param(screen, sh, PIPE_SHADER_CAP_FP16);
      options->LowerPrecisionDerivatives =
         screen->get_shader_param(screen, sh, PIPE_SHADER_CAP_FP16_DERIVATIVES);
      options->LowerPrecisionInt16 =
         screen->get_shader_param(screen, sh, PIPE_SHADER_CAP_INT16);
      options->LowerPrecisionConstants =
         screen->get_shader_param(screen, sh, PIPE_SHADER_CAP_GLSL_16BIT_CONSTS);
      options->LowerPrecisionFloat16Uniforms =
         screen->get_shader_param(screen, sh, PIPE_SHADER_CAP_FP16_CONST_BUFFERS);
   }

   c->MaxUserAssignableUniformLocations =
      c->Program[MESA_SHADER_VERTEX].MaxUniformComponents +
      c->Program[MESA_SHADER_TESS_CTRL].MaxUniformComponents +
      c->Program[MESA_SHADER_TESS_EVAL].MaxUniformComponents +
      c->Program[MESA_SHADER_GEOMETRY].MaxUniformComponents +
      c->Program[MESA_SHADER_FRAGMENT].MaxUniformComponents;

   c->GLSLLowerConstArrays =
      screen->get_param(screen, PIPE_CAP_PREFER_IMM_ARRAYS_AS_CONSTBUF);
   c->GLSLTessLevelsAsInputs =
      screen->get_param(screen, PIPE_CAP_GLSL_TESS_LEVELS_AS_INPUTS);
   c->PrimitiveRestartForPatches = false;

   c->MaxCombinedTextureImageUnits =
         _min(c->Program[MESA_SHADER_VERTEX].MaxTextureImageUnits +
              c->Program[MESA_SHADER_TESS_CTRL].MaxTextureImageUnits +
              c->Program[MESA_SHADER_TESS_EVAL].MaxTextureImageUnits +
              c->Program[MESA_SHADER_GEOMETRY].MaxTextureImageUnits +
              c->Program[MESA_SHADER_FRAGMENT].MaxTextureImageUnits +
              c->Program[MESA_SHADER_COMPUTE].MaxTextureImageUnits,
              MAX_COMBINED_TEXTURE_IMAGE_UNITS);

   /* This depends on program constants. */
   c->MaxTextureCoordUnits
      = _min(c->Program[MESA_SHADER_FRAGMENT].MaxTextureImageUnits,
             MAX_TEXTURE_COORD_UNITS);

   c->MaxTextureUnits =
      _min(c->Program[MESA_SHADER_FRAGMENT].MaxTextureImageUnits,
           c->MaxTextureCoordUnits);

   c->Program[MESA_SHADER_VERTEX].MaxAttribs =
      MIN2(c->Program[MESA_SHADER_VERTEX].MaxAttribs, 16);

   c->MaxVarying = screen->get_param(screen, PIPE_CAP_MAX_VARYINGS);
   c->MaxVarying = MIN2(c->MaxVarying, MAX_VARYING);
   c->MaxGeometryOutputVertices =
      screen->get_param(screen, PIPE_CAP_MAX_GEOMETRY_OUTPUT_VERTICES);
   c->MaxGeometryTotalOutputComponents =
      screen->get_param(screen, PIPE_CAP_MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS);
   c->MaxGeometryShaderInvocations =
      screen->get_param(screen, PIPE_CAP_MAX_GS_INVOCATIONS);
   c->MaxTessPatchComponents =
      MIN2(screen->get_param(screen, PIPE_CAP_MAX_SHADER_PATCH_VARYINGS),
           MAX_VARYING) * 4;

   c->MinProgramTexelOffset =
      screen->get_param(screen, PIPE_CAP_MIN_TEXEL_OFFSET);
   c->MaxProgramTexelOffset =
      screen->get_param(screen, PIPE_CAP_MAX_TEXEL_OFFSET);

   c->MaxProgramTextureGatherComponents =
      screen->get_param(screen, PIPE_CAP_MAX_TEXTURE_GATHER_COMPONENTS);
   c->MinProgramTextureGatherOffset =
      screen->get_param(screen, PIPE_CAP_MIN_TEXTURE_GATHER_OFFSET);
   c->MaxProgramTextureGatherOffset =
      screen->get_param(screen, PIPE_CAP_MAX_TEXTURE_GATHER_OFFSET);

   c->MaxTransformFeedbackBuffers =
      screen->get_param(screen, PIPE_CAP_MAX_STREAM_OUTPUT_BUFFERS);
   c->MaxTransformFeedbackBuffers = MIN2(c->MaxTransformFeedbackBuffers,
                                         MAX_FEEDBACK_BUFFERS);
   c->MaxTransformFeedbackSeparateComponents =
      screen->get_param(screen, PIPE_CAP_MAX_STREAM_OUTPUT_SEPARATE_COMPONENTS);
   c->MaxTransformFeedbackInterleavedComponents =
      screen->get_param(screen,
                        PIPE_CAP_MAX_STREAM_OUTPUT_INTERLEAVED_COMPONENTS);
   c->MaxVertexStreams =
      MAX2(1, screen->get_param(screen, PIPE_CAP_MAX_VERTEX_STREAMS));

   /* The vertex stream must fit into pipe_stream_output_info::stream */
   assert(c->MaxVertexStreams <= 4);

   c->MaxVertexAttribStride
      = screen->get_param(screen, PIPE_CAP_MAX_VERTEX_ATTRIB_STRIDE);

   /* The value cannot be larger than that since pipe_vertex_buffer::src_offset
    * is only 16 bits.
    */
   temp = screen->get_param(screen, PIPE_CAP_MAX_VERTEX_ELEMENT_SRC_OFFSET);
   c->MaxVertexAttribRelativeOffset = MIN2(0xffff, temp);

   c->GLSLSkipStrictMaxUniformLimitCheck =
      screen->get_param(screen, PIPE_CAP_TGSI_CAN_COMPACT_CONSTANTS);

   c->UniformBufferOffsetAlignment =
      screen->get_param(screen, PIPE_CAP_CONSTANT_BUFFER_OFFSET_ALIGNMENT);

   if (can_ubo) {
      extensions->ARB_uniform_buffer_object = GL_TRUE;
      c->MaxCombinedUniformBlocks = c->MaxUniformBufferBindings =
         c->Program[MESA_SHADER_VERTEX].MaxUniformBlocks +
         c->Program[MESA_SHADER_TESS_CTRL].MaxUniformBlocks +
         c->Program[MESA_SHADER_TESS_EVAL].MaxUniformBlocks +
         c->Program[MESA_SHADER_GEOMETRY].MaxUniformBlocks +
         c->Program[MESA_SHADER_FRAGMENT].MaxUniformBlocks +
         c->Program[MESA_SHADER_COMPUTE].MaxUniformBlocks;
      assert(c->MaxCombinedUniformBlocks <= MAX_COMBINED_UNIFORM_BUFFERS);
   }

   c->GLSLFragCoordIsSysVal =
      screen->get_param(screen, PIPE_CAP_FS_POSITION_IS_SYSVAL);
   c->GLSLPointCoordIsSysVal =
      screen->get_param(screen, PIPE_CAP_FS_POINT_IS_SYSVAL);
   c->GLSLFrontFacingIsSysVal =
      screen->get_param(screen, PIPE_CAP_FS_FACE_IS_INTEGER_SYSVAL);

   /* GL_ARB_get_program_binary */
   if (screen->get_disk_shader_cache && screen->get_disk_shader_cache(screen))
      c->NumProgramBinaryFormats = 1;
   /* GL_ARB_gl_spirv */
   if (screen->get_param(screen, PIPE_CAP_GL_SPIRV) &&
       (api == API_OPENGL_CORE || api == API_OPENGL_COMPAT))
      c->NumShaderBinaryFormats = 1;

   c->MaxAtomicBufferBindings =
      MAX2(c->Program[MESA_SHADER_FRAGMENT].MaxAtomicBuffers,
           c->Program[MESA_SHADER_COMPUTE].MaxAtomicBuffers);
   c->MaxAtomicBufferSize = ATOMIC_COUNTER_SIZE *
      MAX2(c->Program[MESA_SHADER_FRAGMENT].MaxAtomicCounters,
           c->Program[MESA_SHADER_COMPUTE].MaxAtomicCounters);

   c->MaxCombinedAtomicBuffers =
      MIN2(screen->get_param(screen,
                             PIPE_CAP_MAX_COMBINED_HW_ATOMIC_COUNTER_BUFFERS),
           MAX_COMBINED_ATOMIC_BUFFERS);
   if (!c->MaxCombinedAtomicBuffers) {
      c->MaxCombinedAtomicBuffers = MAX2(
         c->Program[MESA_SHADER_VERTEX].MaxAtomicBuffers +
         c->Program[MESA_SHADER_TESS_CTRL].MaxAtomicBuffers +
         c->Program[MESA_SHADER_TESS_EVAL].MaxAtomicBuffers +
         c->Program[MESA_SHADER_GEOMETRY].MaxAtomicBuffers +
         c->Program[MESA_SHADER_FRAGMENT].MaxAtomicBuffers,
         c->Program[MESA_SHADER_COMPUTE].MaxAtomicBuffers);
      assert(c->MaxCombinedAtomicBuffers <= MAX_COMBINED_ATOMIC_BUFFERS);
   }

   c->MaxCombinedAtomicCounters =
      screen->get_param(screen, PIPE_CAP_MAX_COMBINED_HW_ATOMIC_COUNTERS);
   if (!c->MaxCombinedAtomicCounters)
      c->MaxCombinedAtomicCounters = MAX_ATOMIC_COUNTERS;

   if (c->Program[MESA_SHADER_FRAGMENT].MaxAtomicBuffers) {
      extensions->ARB_shader_atomic_counters = GL_TRUE;
      extensions->ARB_shader_atomic_counter_ops = GL_TRUE;
   }

   c->MaxCombinedShaderOutputResources = c->MaxDrawBuffers;
   c->ShaderStorageBufferOffsetAlignment =
      screen->get_param(screen, PIPE_CAP_SHADER_BUFFER_OFFSET_ALIGNMENT);
   if (c->ShaderStorageBufferOffsetAlignment) {
      c->MaxCombinedShaderStorageBlocks =
         MIN2(screen->get_param(screen, PIPE_CAP_MAX_COMBINED_SHADER_BUFFERS),
              MAX_COMBINED_SHADER_STORAGE_BUFFERS);
      if (!c->MaxCombinedShaderStorageBlocks) {
         c->MaxCombinedShaderStorageBlocks = MAX2(
            c->Program[MESA_SHADER_VERTEX].MaxShaderStorageBlocks +
            c->Program[MESA_SHADER_TESS_CTRL].MaxShaderStorageBlocks +
            c->Program[MESA_SHADER_TESS_EVAL].MaxShaderStorageBlocks +
            c->Program[MESA_SHADER_GEOMETRY].MaxShaderStorageBlocks +
            c->Program[MESA_SHADER_FRAGMENT].MaxShaderStorageBlocks,
            c->Program[MESA_SHADER_COMPUTE].MaxShaderStorageBlocks);
         assert(c->MaxCombinedShaderStorageBlocks < MAX_COMBINED_SHADER_STORAGE_BUFFERS);
      }
      c->MaxShaderStorageBufferBindings = c->MaxCombinedShaderStorageBlocks;

      c->MaxCombinedShaderOutputResources +=
         c->MaxCombinedShaderStorageBlocks;
      c->MaxShaderStorageBlockSize =
         screen->get_param(screen, PIPE_CAP_MAX_SHADER_BUFFER_SIZE_UINT);
      if (c->Program[MESA_SHADER_FRAGMENT].MaxShaderStorageBlocks)
         extensions->ARB_shader_storage_buffer_object = GL_TRUE;
   }

   c->MaxCombinedImageUniforms =
         c->Program[MESA_SHADER_VERTEX].MaxImageUniforms +
         c->Program[MESA_SHADER_TESS_CTRL].MaxImageUniforms +
         c->Program[MESA_SHADER_TESS_EVAL].MaxImageUniforms +
         c->Program[MESA_SHADER_GEOMETRY].MaxImageUniforms +
         c->Program[MESA_SHADER_FRAGMENT].MaxImageUniforms +
         c->Program[MESA_SHADER_COMPUTE].MaxImageUniforms;
   c->MaxCombinedShaderOutputResources += c->MaxCombinedImageUniforms;
   c->MaxImageUnits = MAX_IMAGE_UNITS;
   if (c->Program[MESA_SHADER_FRAGMENT].MaxImageUniforms &&
       screen->get_param(screen, PIPE_CAP_IMAGE_STORE_FORMATTED)) {
      extensions->ARB_shader_image_load_store = GL_TRUE;
      extensions->ARB_shader_image_size = GL_TRUE;
   }

   /* ARB_framebuffer_no_attachments */
   c->MaxFramebufferWidth   = c->MaxViewportWidth;
   c->MaxFramebufferHeight  = c->MaxViewportHeight;
   /* NOTE: we cheat here a little by assuming that
    * PIPE_CAP_MAX_TEXTURE_ARRAY_LAYERS has the same
    * number of layers as we need, although we technically
    * could have more the generality is not really useful
    * in practicality.
    */
   c->MaxFramebufferLayers =
      screen->get_param(screen, PIPE_CAP_MAX_TEXTURE_ARRAY_LAYERS);

   c->MaxWindowRectangles =
      screen->get_param(screen, PIPE_CAP_MAX_WINDOW_RECTANGLES);

   c->SparseBufferPageSize =
      screen->get_param(screen, PIPE_CAP_SPARSE_BUFFER_PAGE_SIZE);

   c->AllowMappedBuffersDuringExecution =
      screen->get_param(screen, PIPE_CAP_ALLOW_MAPPED_BUFFERS_DURING_EXECUTION);

   c->UseSTD430AsDefaultPacking =
      screen->get_param(screen, PIPE_CAP_LOAD_CONSTBUF);

   c->MaxSubpixelPrecisionBiasBits =
      screen->get_param(screen, PIPE_CAP_MAX_CONSERVATIVE_RASTER_SUBPIXEL_PRECISION_BIAS);

   c->ConservativeRasterDilateRange[0] =
      screen->get_paramf(screen, PIPE_CAPF_MIN_CONSERVATIVE_RASTER_DILATE);
   c->ConservativeRasterDilateRange[1] =
      screen->get_paramf(screen, PIPE_CAPF_MAX_CONSERVATIVE_RASTER_DILATE);
   c->ConservativeRasterDilateGranularity =
      screen->get_paramf(screen, PIPE_CAPF_CONSERVATIVE_RASTER_DILATE_GRANULARITY);

   /* limit the max combined shader output resources to a driver limit */
   temp = screen->get_param(screen, PIPE_CAP_MAX_COMBINED_SHADER_OUTPUT_RESOURCES);
   if (temp > 0 && c->MaxCombinedShaderOutputResources > temp)
      c->MaxCombinedShaderOutputResources = temp;

   c->VertexBufferOffsetIsInt32 =
      screen->get_param(screen, PIPE_CAP_SIGNED_VERTEX_BUFFER_OFFSET);

   c->AllowDynamicVAOFastPath =
         screen->get_param(screen, PIPE_CAP_ALLOW_DYNAMIC_VAO_FASTPATH);

   c->glBeginEndBufferSize =
      screen->get_param(screen, PIPE_CAP_GL_BEGIN_END_BUFFER_SIZE);

   c->MaxSparseTextureSize =
      screen->get_param(screen, PIPE_CAP_MAX_SPARSE_TEXTURE_SIZE);
   c->MaxSparse3DTextureSize =
      screen->get_param(screen, PIPE_CAP_MAX_SPARSE_3D_TEXTURE_SIZE);
   c->MaxSparseArrayTextureLayers =
      screen->get_param(screen, PIPE_CAP_MAX_SPARSE_ARRAY_TEXTURE_LAYERS);
   c->SparseTextureFullArrayCubeMipmaps =
      screen->get_param(screen, PIPE_CAP_SPARSE_TEXTURE_FULL_ARRAY_CUBE_MIPMAPS);

   c->HardwareAcceleratedSelect =
      screen->get_param(screen, PIPE_CAP_HARDWARE_GL_SELECT);

   c->AllowGLThreadBufferSubDataOpt =
      screen->get_param(screen, PIPE_CAP_ALLOW_GLTHREAD_BUFFER_SUBDATA_OPT);
}


/**
 * Given a member \c x of struct gl_extensions, return offset of
 * \c x in bytes.
 */
#define o(x) offsetof(struct gl_extensions, x)


struct st_extension_cap_mapping {
   int extension_offset;
   int cap;
};

struct st_extension_format_mapping {
   int extension_offset[2];
   enum pipe_format format[32];

   /* If TRUE, at least one format must be supported for the extensions to be
    * advertised. If FALSE, all the formats must be supported. */
   GLboolean need_at_least_one;
};

/**
 * Enable extensions if certain pipe formats are supported by the driver.
 * What extensions will be enabled and what formats must be supported is
 * described by the array of st_extension_format_mapping.
 *
 * target and bind_flags are passed to is_format_supported.
 */
static void
init_format_extensions(struct pipe_screen *screen,
                       struct gl_extensions *extensions,
                       const struct st_extension_format_mapping *mapping,
                       unsigned num_mappings,
                       enum pipe_texture_target target,
                       unsigned bind_flags)
{
   GLboolean *extension_table = (GLboolean *) extensions;
   unsigned i;
   int j;
   int num_formats = ARRAY_SIZE(mapping->format);
   int num_ext = ARRAY_SIZE(mapping->extension_offset);

   for (i = 0; i < num_mappings; i++) {
      int num_supported = 0;

      /* Examine each format in the list. */
      for (j = 0; j < num_formats && mapping[i].format[j]; j++) {
         if (screen->is_format_supported(screen, mapping[i].format[j],
                                         target, 0, 0, bind_flags)) {
            num_supported++;
         }
      }

      if (!num_supported ||
          (!mapping[i].need_at_least_one && num_supported != j)) {
         continue;
      }

      /* Enable all extensions in the list. */
      for (j = 0; j < num_ext && mapping[i].extension_offset[j]; j++)
         extension_table[mapping[i].extension_offset[j]] = GL_TRUE;
   }
}


/**
 * Given a list of formats and bind flags, return the maximum number
 * of samples supported by any of those formats.
 */
static unsigned
get_max_samples_for_formats(struct pipe_screen *screen,
                            unsigned num_formats,
                            const enum pipe_format *formats,
                            unsigned max_samples,
                            unsigned bind)
{
   unsigned i, f;

   for (i = max_samples; i > 0; --i) {
      for (f = 0; f < num_formats; f++) {
         if (screen->is_format_supported(screen, formats[f],
                                         PIPE_TEXTURE_2D, i, i, bind)) {
            return i;
         }
      }
   }
   return 0;
}

static unsigned
get_max_samples_for_formats_advanced(struct pipe_screen *screen,
                                     unsigned num_formats,
                                     const enum pipe_format *formats,
                                     unsigned max_samples,
                                     unsigned num_storage_samples,
                                     unsigned bind)
{
   unsigned i, f;

   for (i = max_samples; i > 0; --i) {
      for (f = 0; f < num_formats; f++) {
         if (screen->is_format_supported(screen, formats[f], PIPE_TEXTURE_2D,
                                         i, num_storage_samples, bind)) {
            return i;
         }
      }
   }
   return 0;
}

/**
 * Use pipe_screen::get_param() to query PIPE_CAP_ values to determine
 * which GL extensions are supported.
 * Quite a few extensions are always supported because they are standard
 * features or can be built on top of other gallium features.
 * Some fine tuning may still be needed.
 */
void st_init_extensions(struct pipe_screen *screen,
                        struct gl_constants *consts,
                        struct gl_extensions *extensions,
                        struct st_config_options *options,
                        gl_api api)
{
   unsigned i;
   GLboolean *extension_table = (GLboolean *) extensions;

   static const struct st_extension_cap_mapping cap_mapping[] = {
      { o(ARB_base_instance),                PIPE_CAP_START_INSTANCE                   },
      { o(ARB_bindless_texture),             PIPE_CAP_BINDLESS_TEXTURE                 },
      { o(ARB_buffer_storage),               PIPE_CAP_BUFFER_MAP_PERSISTENT_COHERENT   },
      { o(ARB_clip_control),                 PIPE_CAP_CLIP_HALFZ                       },
      { o(ARB_color_buffer_float),           PIPE_CAP_VERTEX_COLOR_UNCLAMPED           },
      { o(ARB_conditional_render_inverted),  PIPE_CAP_CONDITIONAL_RENDER_INVERTED      },
      { o(ARB_copy_image),                   PIPE_CAP_COPY_BETWEEN_COMPRESSED_AND_PLAIN_FORMATS },
      { o(OES_copy_image),                   PIPE_CAP_COPY_BETWEEN_COMPRESSED_AND_PLAIN_FORMATS },
      { o(ARB_cull_distance),                PIPE_CAP_CULL_DISTANCE                    },
      { o(ARB_depth_clamp),                  PIPE_CAP_DEPTH_CLIP_DISABLE               },
      { o(ARB_derivative_control),           PIPE_CAP_FS_FINE_DERIVATIVE               },
      { o(ARB_draw_buffers_blend),           PIPE_CAP_INDEP_BLEND_FUNC                 },
      { o(ARB_draw_indirect),                PIPE_CAP_DRAW_INDIRECT                    },
      { o(ARB_draw_instanced),               PIPE_CAP_VS_INSTANCEID                    },
      { o(ARB_fragment_program_shadow),      PIPE_CAP_TEXTURE_SHADOW_MAP               },
      { o(ARB_framebuffer_object),           PIPE_CAP_MIXED_FRAMEBUFFER_SIZES          },
      { o(ARB_gpu_shader_int64),             PIPE_CAP_INT64                            },
      { o(ARB_gl_spirv),                     PIPE_CAP_GL_SPIRV                         },
      { o(ARB_indirect_parameters),          PIPE_CAP_MULTI_DRAW_INDIRECT_PARAMS       },
      { o(ARB_instanced_arrays),             PIPE_CAP_VERTEX_ELEMENT_INSTANCE_DIVISOR  },
      { o(ARB_occlusion_query2),             PIPE_CAP_OCCLUSION_QUERY                  },
      { o(ARB_pipeline_statistics_query),    PIPE_CAP_QUERY_PIPELINE_STATISTICS        },
      { o(ARB_pipeline_statistics_query),    PIPE_CAP_QUERY_PIPELINE_STATISTICS_SINGLE },
      { o(ARB_polygon_offset_clamp),         PIPE_CAP_POLYGON_OFFSET_CLAMP             },
      { o(ARB_post_depth_coverage),          PIPE_CAP_POST_DEPTH_COVERAGE              },
      { o(ARB_query_buffer_object),          PIPE_CAP_QUERY_BUFFER_OBJECT              },
      { o(ARB_robust_buffer_access_behavior), PIPE_CAP_ROBUST_BUFFER_ACCESS_BEHAVIOR   },
      { o(ARB_sample_shading),               PIPE_CAP_SAMPLE_SHADING                   },
      { o(ARB_sample_locations),             PIPE_CAP_PROGRAMMABLE_SAMPLE_LOCATIONS    },
      { o(ARB_seamless_cube_map),            PIPE_CAP_SEAMLESS_CUBE_MAP                },
      { o(ARB_shader_ballot),                PIPE_CAP_SHADER_BALLOT                    },
      { o(ARB_shader_clock),                 PIPE_CAP_SHADER_CLOCK                     },
      { o(ARB_shader_draw_parameters),       PIPE_CAP_DRAW_PARAMETERS                  },
      { o(ARB_shader_group_vote),            PIPE_CAP_SHADER_GROUP_VOTE                },
      { o(EXT_shader_image_load_formatted),  PIPE_CAP_IMAGE_LOAD_FORMATTED             },
      { o(EXT_shader_image_load_store),      PIPE_CAP_IMAGE_ATOMIC_INC_WRAP            },
      { o(ARB_shader_stencil_export),        PIPE_CAP_SHADER_STENCIL_EXPORT            },
      { o(ARB_shader_texture_image_samples), PIPE_CAP_TEXTURE_QUERY_SAMPLES            },
      { o(ARB_shader_texture_lod),           PIPE_CAP_FRAGMENT_SHADER_TEXTURE_LOD      },
      { o(ARB_shadow),                       PIPE_CAP_TEXTURE_SHADOW_MAP               },
      { o(ARB_sparse_buffer),                PIPE_CAP_SPARSE_BUFFER_PAGE_SIZE          },
      { o(ARB_sparse_texture),               PIPE_CAP_MAX_SPARSE_TEXTURE_SIZE          },
      { o(ARB_sparse_texture2),              PIPE_CAP_QUERY_SPARSE_TEXTURE_RESIDENCY   },
      { o(ARB_sparse_texture_clamp),         PIPE_CAP_CLAMP_SPARSE_TEXTURE_LOD         },
      { o(ARB_spirv_extensions),             PIPE_CAP_GL_SPIRV                         },
      { o(ARB_texture_buffer_object),        PIPE_CAP_TEXTURE_BUFFER_OBJECTS           },
      { o(ARB_texture_cube_map_array),       PIPE_CAP_CUBE_MAP_ARRAY                   },
      { o(ARB_texture_filter_minmax),        PIPE_CAP_SAMPLER_REDUCTION_MINMAX_ARB     },
      { o(ARB_texture_gather),               PIPE_CAP_MAX_TEXTURE_GATHER_COMPONENTS    },
      { o(ARB_texture_mirror_clamp_to_edge), PIPE_CAP_TEXTURE_MIRROR_CLAMP_TO_EDGE     },
      { o(ARB_texture_multisample),          PIPE_CAP_TEXTURE_MULTISAMPLE              },
      { o(ARB_texture_non_power_of_two),     PIPE_CAP_NPOT_TEXTURES                    },
      { o(ARB_texture_query_lod),            PIPE_CAP_TEXTURE_QUERY_LOD                },
      { o(ARB_texture_view),                 PIPE_CAP_SAMPLER_VIEW_TARGET              },
      { o(ARB_timer_query),                  PIPE_CAP_QUERY_TIMESTAMP                  },
      { o(ARB_transform_feedback2),          PIPE_CAP_STREAM_OUTPUT_PAUSE_RESUME       },
      { o(ARB_transform_feedback3),          PIPE_CAP_STREAM_OUTPUT_INTERLEAVE_BUFFERS },
      { o(ARB_transform_feedback_overflow_query), PIPE_CAP_QUERY_SO_OVERFLOW           },
      { o(ARB_fragment_shader_interlock),    PIPE_CAP_FRAGMENT_SHADER_INTERLOCK        },

      { o(EXT_blend_equation_separate),      PIPE_CAP_BLEND_EQUATION_SEPARATE          },
      { o(EXT_demote_to_helper_invocation),  PIPE_CAP_DEMOTE_TO_HELPER_INVOCATION      },
      { o(EXT_depth_bounds_test),            PIPE_CAP_DEPTH_BOUNDS_TEST                },
      { o(EXT_disjoint_timer_query),         PIPE_CAP_QUERY_TIMESTAMP                  },
      { o(EXT_draw_buffers2),                PIPE_CAP_INDEP_BLEND_ENABLE               },
      { o(EXT_memory_object),                PIPE_CAP_MEMOBJ                           },
#ifndef _WIN32
      { o(EXT_memory_object_fd),             PIPE_CAP_MEMOBJ                           },
#else
      { o(EXT_memory_object_win32),          PIPE_CAP_MEMOBJ                           },
#endif
      { o(EXT_multisampled_render_to_texture), PIPE_CAP_SURFACE_SAMPLE_COUNT           },
      { o(EXT_semaphore),                    PIPE_CAP_FENCE_SIGNAL                     },
#ifndef _WIN32
      { o(EXT_semaphore_fd),                 PIPE_CAP_FENCE_SIGNAL                     },
#else
      { o(EXT_semaphore_win32),              PIPE_CAP_FENCE_SIGNAL                     },
#endif
      { o(EXT_shader_samples_identical),     PIPE_CAP_SHADER_SAMPLES_IDENTICAL         },
      { o(EXT_texture_array),                PIPE_CAP_MAX_TEXTURE_ARRAY_LAYERS         },
      { o(EXT_texture_filter_anisotropic),   PIPE_CAP_ANISOTROPIC_FILTER               },
      { o(EXT_texture_filter_minmax),        PIPE_CAP_SAMPLER_REDUCTION_MINMAX         },
      { o(EXT_texture_mirror_clamp),         PIPE_CAP_TEXTURE_MIRROR_CLAMP             },
      { o(EXT_texture_shadow_lod),           PIPE_CAP_TEXTURE_SHADOW_LOD               },
      { o(EXT_texture_swizzle),              PIPE_CAP_TEXTURE_SWIZZLE                  },
      { o(EXT_transform_feedback),           PIPE_CAP_MAX_STREAM_OUTPUT_BUFFERS        },
      { o(EXT_window_rectangles),            PIPE_CAP_MAX_WINDOW_RECTANGLES            },

      { o(AMD_depth_clamp_separate),         PIPE_CAP_DEPTH_CLIP_DISABLE_SEPARATE      },
      { o(AMD_framebuffer_multisample_advanced), PIPE_CAP_FRAMEBUFFER_MSAA_CONSTRAINTS },
      { o(AMD_performance_monitor),          PIPE_CAP_PERFORMANCE_MONITOR              },
      { o(AMD_pinned_memory),                PIPE_CAP_RESOURCE_FROM_USER_MEMORY        },
      { o(ATI_meminfo),                      PIPE_CAP_QUERY_MEMORY_INFO                },
      { o(AMD_seamless_cubemap_per_texture), PIPE_CAP_SEAMLESS_CUBE_MAP_PER_TEXTURE    },
      { o(ATI_texture_mirror_once),          PIPE_CAP_TEXTURE_MIRROR_CLAMP             },
      { o(INTEL_conservative_rasterization), PIPE_CAP_CONSERVATIVE_RASTER_INNER_COVERAGE },
      { o(INTEL_shader_atomic_float_minmax), PIPE_CAP_ATOMIC_FLOAT_MINMAX              },
      { o(MESA_tile_raster_order),           PIPE_CAP_TILE_RASTER_ORDER                },
      { o(NV_alpha_to_coverage_dither_control), PIPE_CAP_ALPHA_TO_COVERAGE_DITHER_CONTROL },
      { o(NV_compute_shader_derivatives),    PIPE_CAP_COMPUTE_SHADER_DERIVATIVES       },
      { o(NV_conditional_render),            PIPE_CAP_CONDITIONAL_RENDER               },
      { o(NV_fill_rectangle),                PIPE_CAP_POLYGON_MODE_FILL_RECTANGLE      },
      { o(NV_primitive_restart),             PIPE_CAP_PRIMITIVE_RESTART                },
      { o(NV_shader_atomic_float),           PIPE_CAP_IMAGE_ATOMIC_FLOAT_ADD           },
      { o(NV_shader_atomic_int64),           PIPE_CAP_SHADER_ATOMIC_INT64              },
      { o(NV_texture_barrier),               PIPE_CAP_TEXTURE_BARRIER                  },
      { o(NV_viewport_array2),               PIPE_CAP_VIEWPORT_MASK                    },
      { o(NV_viewport_swizzle),              PIPE_CAP_VIEWPORT_SWIZZLE                 },
      { o(NVX_gpu_memory_info),              PIPE_CAP_QUERY_MEMORY_INFO                },

      { o(OES_standard_derivatives),         PIPE_CAP_FRAGMENT_SHADER_DERIVATIVES      },
      { o(OES_texture_float_linear),         PIPE_CAP_TEXTURE_FLOAT_LINEAR             },
      { o(OES_texture_half_float_linear),    PIPE_CAP_TEXTURE_HALF_FLOAT_LINEAR        },
      { o(OES_texture_view),                 PIPE_CAP_SAMPLER_VIEW_TARGET              },
      { o(INTEL_blackhole_render),           PIPE_CAP_FRONTEND_NOOP                    },
      { o(ARM_shader_framebuffer_fetch_depth_stencil), PIPE_CAP_FBFETCH_ZS             },
      { o(MESA_texture_const_bandwidth),     PIPE_CAP_HAS_CONST_BW                     },
   };

   /* Required: render target and sampler support */
   static const struct st_extension_format_mapping rendertarget_mapping[] = {
      { { o(ARB_texture_rgb10_a2ui) },
        { PIPE_FORMAT_R10G10B10A2_UINT,
          PIPE_FORMAT_B10G10R10A2_UINT },
         GL_TRUE }, /* at least one format must be supported */

      { { o(EXT_sRGB) },
        { PIPE_FORMAT_A8B8G8R8_SRGB,
          PIPE_FORMAT_B8G8R8A8_SRGB,
          PIPE_FORMAT_R8G8B8A8_SRGB },
         GL_TRUE }, /* at least one format must be supported */

      { { o(EXT_packed_float) },
        { PIPE_FORMAT_R11G11B10_FLOAT } },

      { { o(EXT_texture_integer) },
        { PIPE_FORMAT_R32G32B32A32_UINT,
          PIPE_FORMAT_R32G32B32A32_SINT } },

      { { o(ARB_texture_rg) },
        { PIPE_FORMAT_R8_UNORM,
          PIPE_FORMAT_R8G8_UNORM } },

      { { o(EXT_texture_norm16) },
        { PIPE_FORMAT_R16_UNORM,
          PIPE_FORMAT_R16G16_UNORM,
          PIPE_FORMAT_R16G16B16A16_UNORM } },

      { { o(EXT_render_snorm) },
        { PIPE_FORMAT_R8_SNORM,
          PIPE_FORMAT_R8G8_SNORM,
          PIPE_FORMAT_R8G8B8A8_SNORM,
          PIPE_FORMAT_R16_SNORM,
          PIPE_FORMAT_R16G16_SNORM,
          PIPE_FORMAT_R16G16B16A16_SNORM } },

      { { o(EXT_color_buffer_half_float) },
        { PIPE_FORMAT_R16_FLOAT,
          PIPE_FORMAT_R16G16_FLOAT,
          PIPE_FORMAT_R16G16B16A16_FLOAT } },

      { { o(EXT_color_buffer_float) },
        { PIPE_FORMAT_R16_FLOAT,
          PIPE_FORMAT_R16G16_FLOAT,
          PIPE_FORMAT_R16G16B16A16_FLOAT,
          PIPE_FORMAT_R32_FLOAT,
          PIPE_FORMAT_R32G32_FLOAT,
          PIPE_FORMAT_R32G32B32A32_FLOAT } },
   };

   /* Required: render target, sampler, and blending */
   static const struct st_extension_format_mapping rt_blendable[] = {
      { { o(EXT_float_blend) },
        { PIPE_FORMAT_R32G32B32A32_FLOAT } },
   };

   /* Required: depth stencil and sampler support */
   static const struct st_extension_format_mapping depthstencil_mapping[] = {
      { { o(ARB_depth_buffer_float) },
        { PIPE_FORMAT_Z32_FLOAT,
          PIPE_FORMAT_Z32_FLOAT_S8X24_UINT } },
   };

   /* Required: sampler support */
   static const struct st_extension_format_mapping texture_mapping[] = {
      { { o(OES_texture_float) },
        { PIPE_FORMAT_R32G32B32A32_FLOAT } },

      { { o(OES_texture_half_float) },
        { PIPE_FORMAT_R16G16B16A16_FLOAT } },

      { { o(ARB_texture_compression_rgtc) },
        { PIPE_FORMAT_RGTC1_UNORM,
          PIPE_FORMAT_RGTC1_SNORM,
          PIPE_FORMAT_RGTC2_UNORM,
          PIPE_FORMAT_RGTC2_SNORM } },

      /* RGTC software fallback support. */
      { { o(ARB_texture_compression_rgtc) },
        { PIPE_FORMAT_R8_UNORM,
          PIPE_FORMAT_R8_SNORM,
          PIPE_FORMAT_R8G8_UNORM,
          PIPE_FORMAT_R8G8_SNORM } },

      { { o(EXT_texture_compression_latc) },
        { PIPE_FORMAT_LATC1_UNORM,
          PIPE_FORMAT_LATC1_SNORM,
          PIPE_FORMAT_LATC2_UNORM,
          PIPE_FORMAT_LATC2_SNORM } },

      /* LATC software fallback support. */
      { { o(EXT_texture_compression_latc) },
        { PIPE_FORMAT_L8_UNORM,
          PIPE_FORMAT_L8_SNORM,
          PIPE_FORMAT_L8A8_UNORM,
          PIPE_FORMAT_L8A8_SNORM } },

      { { o(EXT_texture_compression_s3tc),
          o(ANGLE_texture_compression_dxt) },
        { PIPE_FORMAT_DXT1_RGB,
          PIPE_FORMAT_DXT1_RGBA,
          PIPE_FORMAT_DXT3_RGBA,
          PIPE_FORMAT_DXT5_RGBA } },

      /* S3TC software fallback support. */
      { { o(EXT_texture_compression_s3tc),
          o(ANGLE_texture_compression_dxt) },
        { PIPE_FORMAT_R8G8B8A8_UNORM } },

      { { o(EXT_texture_compression_s3tc_srgb) },
        { PIPE_FORMAT_DXT1_SRGB,
          PIPE_FORMAT_DXT1_SRGBA,
          PIPE_FORMAT_DXT3_SRGBA,
          PIPE_FORMAT_DXT5_SRGBA } },

      /* S3TC SRGB software fallback support. */
      { { o(EXT_texture_compression_s3tc_srgb) },
        { PIPE_FORMAT_R8G8B8A8_SRGB } },

      { { o(ARB_texture_compression_bptc) },
        { PIPE_FORMAT_BPTC_RGBA_UNORM,
          PIPE_FORMAT_BPTC_SRGBA,
          PIPE_FORMAT_BPTC_RGB_FLOAT,
          PIPE_FORMAT_BPTC_RGB_UFLOAT } },

      /* BPTC software fallback support. */
      { { o(ARB_texture_compression_bptc) },
        { PIPE_FORMAT_R8G8B8A8_UNORM,
          PIPE_FORMAT_R8G8B8A8_SRGB,
          PIPE_FORMAT_R16G16B16X16_FLOAT } },

      { { o(TDFX_texture_compression_FXT1) },
        { PIPE_FORMAT_FXT1_RGB,
          PIPE_FORMAT_FXT1_RGBA } },

      { { o(KHR_texture_compression_astc_ldr),
          o(KHR_texture_compression_astc_sliced_3d) },
        { PIPE_FORMAT_ASTC_4x4,
          PIPE_FORMAT_ASTC_5x4,
          PIPE_FORMAT_ASTC_5x5,
          PIPE_FORMAT_ASTC_6x5,
          PIPE_FORMAT_ASTC_6x6,
          PIPE_FORMAT_ASTC_8x5,
          PIPE_FORMAT_ASTC_8x6,
          PIPE_FORMAT_ASTC_8x8,
          PIPE_FORMAT_ASTC_10x5,
          PIPE_FORMAT_ASTC_10x6,
          PIPE_FORMAT_ASTC_10x8,
          PIPE_FORMAT_ASTC_10x10,
          PIPE_FORMAT_ASTC_12x10,
          PIPE_FORMAT_ASTC_12x12,
          PIPE_FORMAT_ASTC_4x4_SRGB,
          PIPE_FORMAT_ASTC_5x4_SRGB,
          PIPE_FORMAT_ASTC_5x5_SRGB,
          PIPE_FORMAT_ASTC_6x5_SRGB,
          PIPE_FORMAT_ASTC_6x6_SRGB,
          PIPE_FORMAT_ASTC_8x5_SRGB,
          PIPE_FORMAT_ASTC_8x6_SRGB,
          PIPE_FORMAT_ASTC_8x8_SRGB,
          PIPE_FORMAT_ASTC_10x5_SRGB,
          PIPE_FORMAT_ASTC_10x6_SRGB,
          PIPE_FORMAT_ASTC_10x8_SRGB,
          PIPE_FORMAT_ASTC_10x10_SRGB,
          PIPE_FORMAT_ASTC_12x10_SRGB,
          PIPE_FORMAT_ASTC_12x12_SRGB } },

      /* ASTC software fallback support. */
      { { o(KHR_texture_compression_astc_ldr),
          o(KHR_texture_compression_astc_sliced_3d) },
        { PIPE_FORMAT_R8G8B8A8_UNORM,
          PIPE_FORMAT_R8G8B8A8_SRGB } },

      { { o(EXT_texture_shared_exponent) },
        { PIPE_FORMAT_R9G9B9E5_FLOAT } },

      { { o(EXT_texture_snorm) },
        { PIPE_FORMAT_R8G8B8A8_SNORM } },

      { { o(EXT_texture_sRGB),
          o(EXT_texture_sRGB_decode) },
        { PIPE_FORMAT_A8B8G8R8_SRGB,
	  PIPE_FORMAT_B8G8R8A8_SRGB,
	  PIPE_FORMAT_A8R8G8B8_SRGB,
	  PIPE_FORMAT_R8G8B8A8_SRGB},
        GL_TRUE }, /* at least one format must be supported */

      { { o(EXT_texture_sRGB_R8) },
        { PIPE_FORMAT_R8_SRGB }, },

      { { o(EXT_texture_sRGB_RG8) },
        { PIPE_FORMAT_R8G8_SRGB }, },

      { { o(EXT_texture_type_2_10_10_10_REV) },
        { PIPE_FORMAT_R10G10B10A2_UNORM,
          PIPE_FORMAT_B10G10R10A2_UNORM },
         GL_TRUE }, /* at least one format must be supported */

      { { o(ATI_texture_compression_3dc) },
        { PIPE_FORMAT_LATC2_UNORM } },

      { { o(ATI_texture_compression_3dc) },
        { PIPE_FORMAT_L8A8_UNORM } },

      { { o(MESA_ycbcr_texture) },
        { PIPE_FORMAT_UYVY,
          PIPE_FORMAT_YUYV },
        GL_TRUE }, /* at least one format must be supported */

      { { o(OES_compressed_ETC1_RGB8_texture) },
        { PIPE_FORMAT_ETC1_RGB8,
          PIPE_FORMAT_R8G8B8A8_UNORM },
        GL_TRUE }, /* at least one format must be supported */

      { { o(ARB_stencil_texturing),
          o(ARB_texture_stencil8) },
        { PIPE_FORMAT_X24S8_UINT,
          PIPE_FORMAT_S8X24_UINT },
        GL_TRUE }, /* at least one format must be supported */

      { { o(AMD_compressed_ATC_texture) },
        { PIPE_FORMAT_ATC_RGB,
          PIPE_FORMAT_ATC_RGBA_EXPLICIT,
          PIPE_FORMAT_ATC_RGBA_INTERPOLATED } },
   };

   /* Required: vertex fetch support. */
   static const struct st_extension_format_mapping vertex_mapping[] = {
      { { o(EXT_vertex_array_bgra) },
        { PIPE_FORMAT_B8G8R8A8_UNORM } },
      { { o(ARB_vertex_type_2_10_10_10_rev) },
        { PIPE_FORMAT_R10G10B10A2_UNORM,
          PIPE_FORMAT_B10G10R10A2_UNORM,
          PIPE_FORMAT_R10G10B10A2_SNORM,
          PIPE_FORMAT_B10G10R10A2_SNORM,
          PIPE_FORMAT_R10G10B10A2_USCALED,
          PIPE_FORMAT_B10G10R10A2_USCALED,
          PIPE_FORMAT_R10G10B10A2_SSCALED,
          PIPE_FORMAT_B10G10R10A2_SSCALED } },
      { { o(ARB_vertex_type_10f_11f_11f_rev) },
        { PIPE_FORMAT_R11G11B10_FLOAT } },
   };

   static const struct st_extension_format_mapping tbo_rgb32[] = {
      { {o(ARB_texture_buffer_object_rgb32) },
        { PIPE_FORMAT_R32G32B32_FLOAT,
          PIPE_FORMAT_R32G32B32_UINT,
          PIPE_FORMAT_R32G32B32_SINT,
        } },
   };

   /* Expose the extensions which directly correspond to gallium caps. */
   for (i = 0; i < ARRAY_SIZE(cap_mapping); i++) {
      if (screen->get_param(screen, cap_mapping[i].cap)) {
         extension_table[cap_mapping[i].extension_offset] = GL_TRUE;
      }
   }

   /* EXT implies ARB here */
   if (extensions->EXT_texture_filter_minmax)
      extensions->ARB_texture_filter_minmax = GL_TRUE;

   /* Expose the extensions which directly correspond to gallium formats. */
   init_format_extensions(screen, extensions, rendertarget_mapping,
                          ARRAY_SIZE(rendertarget_mapping), PIPE_TEXTURE_2D,
                          PIPE_BIND_RENDER_TARGET | PIPE_BIND_SAMPLER_VIEW);
   init_format_extensions(screen, extensions, rt_blendable,
                          ARRAY_SIZE(rt_blendable), PIPE_TEXTURE_2D,
                          PIPE_BIND_RENDER_TARGET | PIPE_BIND_SAMPLER_VIEW |
                          PIPE_BIND_BLENDABLE);
   init_format_extensions(screen, extensions, depthstencil_mapping,
                          ARRAY_SIZE(depthstencil_mapping), PIPE_TEXTURE_2D,
                          PIPE_BIND_DEPTH_STENCIL | PIPE_BIND_SAMPLER_VIEW);
   init_format_extensions(screen, extensions, texture_mapping,
                          ARRAY_SIZE(texture_mapping), PIPE_TEXTURE_2D,
                          PIPE_BIND_SAMPLER_VIEW);
   init_format_extensions(screen, extensions, vertex_mapping,
                          ARRAY_SIZE(vertex_mapping), PIPE_BUFFER,
                          PIPE_BIND_VERTEX_BUFFER);

   /* Figure out GLSL support and set GLSLVersion to it. */
   consts->GLSLVersion = screen->get_param(screen, PIPE_CAP_GLSL_FEATURE_LEVEL);
   consts->GLSLVersionCompat =
      screen->get_param(screen, PIPE_CAP_GLSL_FEATURE_LEVEL_COMPATIBILITY);

   const unsigned ESSLVersion =
      screen->get_param(screen, PIPE_CAP_ESSL_FEATURE_LEVEL);
   const unsigned GLSLVersion =
      api == API_OPENGL_COMPAT ? consts->GLSLVersionCompat :
                                 consts->GLSLVersion;

   _mesa_override_glsl_version(consts);

   if (options->force_glsl_version > 0 &&
       options->force_glsl_version <= GLSLVersion) {
      consts->ForceGLSLVersion = options->force_glsl_version;
   }

   consts->ForceCompatShaders = options->force_compat_shaders;

   consts->AllowExtraPPTokens = options->allow_extra_pp_tokens;

   consts->AllowHigherCompatVersion = options->allow_higher_compat_version;
   consts->AllowGLSLCompatShaders = options->allow_glsl_compat_shaders;

   consts->ForceGLSLAbsSqrt = options->force_glsl_abs_sqrt;

   consts->AllowGLSLBuiltinVariableRedeclaration = options->allow_glsl_builtin_variable_redeclaration;

   consts->dri_config_options_sha1 = options->config_options_sha1;

   consts->AllowGLSLCrossStageInterpolationMismatch = options->allow_glsl_cross_stage_interpolation_mismatch;

   consts->DoDCEBeforeClipCullAnalysis = options->do_dce_before_clip_cull_analysis;

   consts->GLSLIgnoreWriteToReadonlyVar = options->glsl_ignore_write_to_readonly_var;

   consts->ForceMapBufferSynchronized = options->force_gl_map_buffer_synchronized;

   consts->PrimitiveRestartFixedIndex =
      screen->get_param(screen, PIPE_CAP_PRIMITIVE_RESTART_FIXED_INDEX);

   /* Technically we are turning on the EXT_gpu_shader5 extension,
    * ARB_gpu_shader5 does not exist in GLES, but this flag is what
    * switches on EXT_gpu_shader5:
    */
   if (api == API_OPENGLES2 && ESSLVersion >= 320)
      extensions->ARB_gpu_shader5 = GL_TRUE;

   if (GLSLVersion >= 400 && !options->disable_arb_gpu_shader5)
      extensions->ARB_gpu_shader5 = GL_TRUE;
   if (GLSLVersion >= 410)
      extensions->ARB_shader_precision = GL_TRUE;

   /* This extension needs full OpenGL 3.2, but we don't know if that's
    * supported at this point. Only check the GLSL version. */
   if (GLSLVersion >= 150 &&
       screen->get_param(screen, PIPE_CAP_VS_LAYER_VIEWPORT)) {
      extensions->AMD_vertex_shader_layer = GL_TRUE;
   }

   if (GLSLVersion >= 140) {
      /* Since GLSL 1.40 has support for all of the features of gpu_shader4,
       * we can always expose it if the driver can do 140. Supporting
       * gpu_shader4 on drivers without GLSL 1.40 is left for a future
       * pipe cap.
       */
      extensions->EXT_gpu_shader4 = GL_TRUE;
      extensions->EXT_texture_buffer_object = GL_TRUE;

      if (consts->MaxTransformFeedbackBuffers &&
          screen->get_param(screen, PIPE_CAP_SHADER_ARRAY_COMPONENTS))
         extensions->ARB_enhanced_layouts = GL_TRUE;
   }

   if (GLSLVersion >= 130) {
      consts->NativeIntegers = GL_TRUE;
      consts->MaxClipPlanes = 8;

      uint32_t drv_clip_planes = screen->get_param(screen, PIPE_CAP_CLIP_PLANES);
      /* only override for > 1 - 0 if none, 1 is MAX, >2 overrides MAX */
      if (drv_clip_planes > 1)
         consts->MaxClipPlanes = drv_clip_planes;

      /* Extensions that either depend on GLSL 1.30 or are a subset thereof. */
      extensions->ARB_conservative_depth = GL_TRUE;
      extensions->ARB_shading_language_packing = GL_TRUE;
      extensions->OES_depth_texture_cube_map = GL_TRUE;
      extensions->ARB_shading_language_420pack = GL_TRUE;
      extensions->ARB_texture_query_levels = GL_TRUE;

      extensions->ARB_shader_bit_encoding = GL_TRUE;

      extensions->EXT_shader_integer_mix = GL_TRUE;
      extensions->ARB_arrays_of_arrays = GL_TRUE;
      extensions->MESA_shader_integer_functions = GL_TRUE;

      if (screen->get_param(screen, PIPE_CAP_OPENCL_INTEGER_FUNCTIONS) &&
          screen->get_param(screen, PIPE_CAP_INTEGER_MULTIPLY_32X16)) {
         extensions->INTEL_shader_integer_functions2 = GL_TRUE;
      }
   } else {
      /* Optional integer support for GLSL 1.2. */
      if (screen->get_shader_param(screen, PIPE_SHADER_VERTEX,
                                   PIPE_SHADER_CAP_INTEGERS) &&
          screen->get_shader_param(screen, PIPE_SHADER_FRAGMENT,
                                   PIPE_SHADER_CAP_INTEGERS)) {
         consts->NativeIntegers = GL_TRUE;

         extensions->EXT_shader_integer_mix = GL_TRUE;
      }

      /* Integer textures make no sense before GLSL 1.30 */
      extensions->EXT_texture_integer = GL_FALSE;
      extensions->ARB_texture_rgb10_a2ui = GL_FALSE;
   }

   if (options->glsl_zero_init) {
      consts->GLSLZeroInit = 1;
   } else {
      consts->GLSLZeroInit = screen->get_param(screen, PIPE_CAP_GLSL_ZERO_INIT);
   }

   consts->ForceGLNamesReuse = options->force_gl_names_reuse;

   consts->ForceIntegerTexNearest = options->force_integer_tex_nearest;

   consts->VendorOverride = options->force_gl_vendor;
   consts->RendererOverride = options->force_gl_renderer;

   consts->UniformBooleanTrue = consts->NativeIntegers ? ~0U : fui(1.0f);

   /* Below are the cases which cannot be moved into tables easily. */

   /* The compatibility profile also requires GLSLVersionCompat >= 400. */
   if (screen->get_shader_param(screen, PIPE_SHADER_TESS_CTRL,
                                PIPE_SHADER_CAP_MAX_INSTRUCTIONS) > 0 &&
       (api != API_OPENGL_COMPAT || consts->GLSLVersionCompat >= 400)) {
      extensions->ARB_tessellation_shader = GL_TRUE;
   }

   /* OES_geometry_shader requires instancing */
   if ((GLSLVersion >= 400 || ESSLVersion >= 310) &&
       screen->get_shader_param(screen, PIPE_SHADER_GEOMETRY,
                                PIPE_SHADER_CAP_MAX_INSTRUCTIONS) > 0 &&
       consts->MaxGeometryShaderInvocations >= 32) {
      extensions->OES_geometry_shader = GL_TRUE;
   }

   /* Some hardware may not support indirect draws, but still wants ES
    * 3.1. This allows the extension to be enabled only in ES contexts to
    * avoid claiming hw support when there is none, and using a software
    * fallback for ES.
    */
   if (api == API_OPENGLES2 && ESSLVersion >= 310) {
      extensions->ARB_draw_indirect = GL_TRUE;
   }

   /* Needs PIPE_CAP_SAMPLE_SHADING + all the sample-related bits of
    * ARB_gpu_shader5. This enables all the per-sample shading ES extensions.
    */
   extensions->OES_sample_variables = extensions->ARB_sample_shading &&
      extensions->ARB_gpu_shader5;

   /* Maximum sample count. */
   {
      static const enum pipe_format color_formats[] = {
         PIPE_FORMAT_R8G8B8A8_UNORM,
         PIPE_FORMAT_B8G8R8A8_UNORM,
         PIPE_FORMAT_A8R8G8B8_UNORM,
         PIPE_FORMAT_A8B8G8R8_UNORM,
      };
      static const enum pipe_format depth_formats[] = {
         PIPE_FORMAT_Z16_UNORM,
         PIPE_FORMAT_Z24X8_UNORM,
         PIPE_FORMAT_X8Z24_UNORM,
         PIPE_FORMAT_Z32_UNORM,
         PIPE_FORMAT_Z32_FLOAT
      };
      static const enum pipe_format int_formats[] = {
         PIPE_FORMAT_R8G8B8A8_SINT
      };
      static const enum pipe_format void_formats[] = {
         PIPE_FORMAT_NONE
      };

      consts->MaxSamples =
         get_max_samples_for_formats(screen, ARRAY_SIZE(color_formats),
                                     color_formats, 16,
                                     PIPE_BIND_RENDER_TARGET);

      consts->MaxImageSamples =
         get_max_samples_for_formats(screen, ARRAY_SIZE(color_formats),
                                     color_formats, 16,
                                     PIPE_BIND_SHADER_IMAGE);

      consts->MaxColorTextureSamples =
         get_max_samples_for_formats(screen, ARRAY_SIZE(color_formats),
                                     color_formats, consts->MaxSamples,
                                     PIPE_BIND_SAMPLER_VIEW);

      consts->MaxDepthTextureSamples =
         get_max_samples_for_formats(screen, ARRAY_SIZE(depth_formats),
                                     depth_formats, consts->MaxSamples,
                                     PIPE_BIND_SAMPLER_VIEW);

      consts->MaxIntegerSamples =
         get_max_samples_for_formats(screen, ARRAY_SIZE(int_formats),
                                     int_formats, consts->MaxSamples,
                                     PIPE_BIND_SAMPLER_VIEW);

      /* ARB_framebuffer_no_attachments, assume max no. of samples 32 */
      consts->MaxFramebufferSamples =
         get_max_samples_for_formats(screen, ARRAY_SIZE(void_formats),
                                     void_formats, 32,
                                     PIPE_BIND_RENDER_TARGET);

      if (extensions->AMD_framebuffer_multisample_advanced) {
         /* AMD_framebuffer_multisample_advanced */
         /* This can be greater than storage samples. */
         consts->MaxColorFramebufferSamples =
            get_max_samples_for_formats_advanced(screen,
                                                ARRAY_SIZE(color_formats),
                                                color_formats, 16,
                                                consts->MaxSamples,
                                                PIPE_BIND_RENDER_TARGET);

         /* If the driver supports N color samples, it means it supports
          * N samples and N storage samples. N samples >= N storage
          * samples.
          */
         consts->MaxColorFramebufferStorageSamples = consts->MaxSamples;
         consts->MaxDepthStencilFramebufferSamples =
            consts->MaxDepthTextureSamples;

         assert(consts->MaxColorFramebufferSamples >=
                consts->MaxDepthStencilFramebufferSamples);
         assert(consts->MaxDepthStencilFramebufferSamples >=
                consts->MaxColorFramebufferStorageSamples);

         consts->NumSupportedMultisampleModes = 0;

         unsigned depth_samples_supported = 0;

         for (unsigned samples = 2;
              samples <= consts->MaxDepthStencilFramebufferSamples;
              samples++) {
            if (screen->is_format_supported(screen, PIPE_FORMAT_Z32_FLOAT,
                                            PIPE_TEXTURE_2D, samples, samples,
                                            PIPE_BIND_DEPTH_STENCIL))
               depth_samples_supported |= 1 << samples;
         }

         for (unsigned samples = 2;
              samples <= consts->MaxColorFramebufferSamples;
              samples++) {
            for (unsigned depth_samples = 2;
                 depth_samples <= samples; depth_samples++) {
               if (!(depth_samples_supported & (1 << depth_samples)))
                  continue;

               for (unsigned storage_samples = 2;
                    storage_samples <= depth_samples; storage_samples++) {
                  if (screen->is_format_supported(screen,
                                                  PIPE_FORMAT_R8G8B8A8_UNORM,
                                                  PIPE_TEXTURE_2D,
                                                  samples,
                                                  storage_samples,
                                                  PIPE_BIND_RENDER_TARGET)) {
                     unsigned i = consts->NumSupportedMultisampleModes;

                     assert(i < ARRAY_SIZE(consts->SupportedMultisampleModes));
                     consts->SupportedMultisampleModes[i].NumColorSamples =
                        samples;
                     consts->SupportedMultisampleModes[i].NumColorStorageSamples =
                        storage_samples;
                     consts->SupportedMultisampleModes[i].NumDepthStencilSamples =
                        depth_samples;
                     consts->NumSupportedMultisampleModes++;
                  }
               }
            }
         }
      }
   }

   if (consts->MaxSamples >= 2) {
      /* Real MSAA support */
      extensions->EXT_framebuffer_multisample = GL_TRUE;
      extensions->EXT_framebuffer_multisample_blit_scaled = GL_TRUE;
   }
   else if (consts->MaxSamples > 0 &&
            screen->get_param(screen, PIPE_CAP_FAKE_SW_MSAA)) {
      /* fake MSAA support */
      consts->FakeSWMSAA = GL_TRUE;
      extensions->EXT_framebuffer_multisample = GL_TRUE;
      extensions->EXT_framebuffer_multisample_blit_scaled = GL_TRUE;
      extensions->ARB_texture_multisample = GL_TRUE;
   }

   if (consts->MaxDualSourceDrawBuffers > 0 &&
       !options->disable_blend_func_extended)
      extensions->ARB_blend_func_extended = GL_TRUE;

   if (screen->get_param(screen, PIPE_CAP_QUERY_TIME_ELAPSED) ||
       extensions->ARB_timer_query) {
      extensions->EXT_timer_query = GL_TRUE;
   }

   if (extensions->ARB_transform_feedback2 &&
       extensions->ARB_draw_instanced) {
      extensions->ARB_transform_feedback_instanced = GL_TRUE;
   }
   if (options->force_glsl_extensions_warn)
      consts->ForceGLSLExtensionsWarn = 1;

   if (options->disable_glsl_line_continuations)
      consts->DisableGLSLLineContinuations = 1;

   if (options->disable_uniform_array_resize)
      consts->DisableUniformArrayResize = 1;

   consts->AliasShaderExtension = options->alias_shader_extension;

   if (options->allow_vertex_texture_bias)
      consts->AllowVertexTextureBias = GL_TRUE;

   if (options->allow_glsl_extension_directive_midshader)
      consts->AllowGLSLExtensionDirectiveMidShader = GL_TRUE;

   if (options->allow_glsl_120_subset_in_110)
      consts->AllowGLSL120SubsetIn110 = GL_TRUE;

   if (options->allow_glsl_builtin_const_expression)
      consts->AllowGLSLBuiltinConstantExpression = GL_TRUE;

   if (options->allow_glsl_relaxed_es)
      consts->AllowGLSLRelaxedES = GL_TRUE;

   consts->MinMapBufferAlignment =
      screen->get_param(screen, PIPE_CAP_MIN_MAP_BUFFER_ALIGNMENT);

   /* The OpenGL Compatibility profile requires arbitrary buffer swizzling. */
   if (api == API_OPENGL_COMPAT &&
       screen->get_param(screen, PIPE_CAP_BUFFER_SAMPLER_VIEW_RGBA_ONLY))
      extensions->ARB_texture_buffer_object = GL_FALSE;

   if (extensions->ARB_texture_buffer_object) {
      consts->MaxTextureBufferSize =
         screen->get_param(screen, PIPE_CAP_MAX_TEXEL_BUFFER_ELEMENTS_UINT);
      consts->TextureBufferOffsetAlignment =
         screen->get_param(screen, PIPE_CAP_TEXTURE_BUFFER_OFFSET_ALIGNMENT);

      if (consts->TextureBufferOffsetAlignment)
         extensions->ARB_texture_buffer_range = GL_TRUE;

      init_format_extensions(screen, extensions, tbo_rgb32,
                             ARRAY_SIZE(tbo_rgb32), PIPE_BUFFER,
                             PIPE_BIND_SAMPLER_VIEW);
   }

   extensions->OES_texture_buffer =
      consts->Program[MESA_SHADER_COMPUTE].MaxImageUniforms &&
      extensions->ARB_texture_buffer_object &&
      extensions->ARB_texture_buffer_range &&
      extensions->ARB_texture_buffer_object_rgb32;

   extensions->EXT_framebuffer_sRGB =
         screen->get_param(screen, PIPE_CAP_DEST_SURFACE_SRGB_CONTROL) &&
         extensions->EXT_sRGB;

   /* Unpacking a varying in the fragment shader costs 1 texture indirection.
    * If the number of available texture indirections is very limited, then we
    * prefer to disable varying packing rather than run the risk of varying
    * packing preventing a shader from running.
    */
   if (screen->get_shader_param(screen, PIPE_SHADER_FRAGMENT,
                                PIPE_SHADER_CAP_MAX_TEX_INDIRECTIONS) <= 8) {
      /* We can't disable varying packing if transform feedback is available,
       * because transform feedback code assumes a packed varying layout.
       */
      if (!extensions->EXT_transform_feedback)
         consts->DisableVaryingPacking = GL_TRUE;
   }

   if (!screen->get_param(screen, PIPE_CAP_PACKED_STREAM_OUTPUT))
      consts->DisableTransformFeedbackPacking = GL_TRUE;

   if (screen->get_param(screen, PIPE_CAP_PREFER_POT_ALIGNED_VARYINGS))
      consts->PreferPOTAlignedVaryings = GL_TRUE;

   unsigned max_fb_fetch_rts = screen->get_param(screen, PIPE_CAP_FBFETCH);
   bool coherent_fb_fetch =
      screen->get_param(screen, PIPE_CAP_FBFETCH_COHERENT);

   if (screen->get_param(screen, PIPE_CAP_BLEND_EQUATION_ADVANCED))
      extensions->KHR_blend_equation_advanced = true;

   if (max_fb_fetch_rts > 0) {
      extensions->KHR_blend_equation_advanced = true;
      extensions->KHR_blend_equation_advanced_coherent = coherent_fb_fetch;

      if (max_fb_fetch_rts >=
          screen->get_param(screen, PIPE_CAP_MAX_RENDER_TARGETS)) {
         extensions->EXT_shader_framebuffer_fetch_non_coherent = true;
         extensions->EXT_shader_framebuffer_fetch = coherent_fb_fetch;
      }
   }

   consts->MaxViewports = screen->get_param(screen, PIPE_CAP_MAX_VIEWPORTS);
   if (consts->MaxViewports >= 16) {
      if (GLSLVersion >= 400) {
         consts->ViewportBounds.Min = -32768.0;
         consts->ViewportBounds.Max = 32767.0;
      } else {
         consts->ViewportBounds.Min = -16384.0;
         consts->ViewportBounds.Max = 16383.0;
      }
      extensions->ARB_viewport_array = GL_TRUE;
      extensions->ARB_fragment_layer_viewport = GL_TRUE;
      if (extensions->AMD_vertex_shader_layer)
         extensions->AMD_vertex_shader_viewport_index = GL_TRUE;
   }

   if (extensions->AMD_vertex_shader_layer &&
       extensions->AMD_vertex_shader_viewport_index &&
       screen->get_param(screen, PIPE_CAP_TES_LAYER_VIEWPORT))
      extensions->ARB_shader_viewport_layer_array = GL_TRUE;

   /* ARB_framebuffer_no_attachments */
   if (screen->get_param(screen, PIPE_CAP_FRAMEBUFFER_NO_ATTACHMENT) &&
       ((consts->MaxSamples >= 4 && consts->MaxFramebufferLayers >= 2048) ||
        (consts->MaxFramebufferSamples >= consts->MaxSamples &&
         consts->MaxFramebufferLayers >= consts->MaxArrayTextureLayers)))
      extensions->ARB_framebuffer_no_attachments = GL_TRUE;

   /* GL_ARB_ES3_compatibility.
    * Check requirements for GLSL ES 3.00.
    */
   if (GLSLVersion >= 130 &&
       extensions->ARB_uniform_buffer_object &&
       (extensions->NV_primitive_restart ||
        consts->PrimitiveRestartFixedIndex) &&
       screen->get_shader_param(screen, PIPE_SHADER_VERTEX,
                                PIPE_SHADER_CAP_MAX_TEXTURE_SAMPLERS) >= 16 &&
       /* Requirements for ETC2 emulation. */
       screen->is_format_supported(screen, PIPE_FORMAT_R8G8B8A8_UNORM,
                                   PIPE_TEXTURE_2D, 0, 0,
                                   PIPE_BIND_SAMPLER_VIEW) &&
       screen->is_format_supported(screen, PIPE_FORMAT_R8G8B8A8_SRGB,
                                   PIPE_TEXTURE_2D, 0, 0,
                                   PIPE_BIND_SAMPLER_VIEW) &&
       screen->is_format_supported(screen, PIPE_FORMAT_R16_UNORM,
                                   PIPE_TEXTURE_2D, 0, 0,
                                   PIPE_BIND_SAMPLER_VIEW) &&
       screen->is_format_supported(screen, PIPE_FORMAT_R16G16_UNORM,
                                   PIPE_TEXTURE_2D, 0, 0,
                                   PIPE_BIND_SAMPLER_VIEW) &&
       screen->is_format_supported(screen, PIPE_FORMAT_R16_SNORM,
                                   PIPE_TEXTURE_2D, 0, 0,
                                   PIPE_BIND_SAMPLER_VIEW) &&
       screen->is_format_supported(screen, PIPE_FORMAT_R16G16_SNORM,
                                   PIPE_TEXTURE_2D, 0, 0,
                                   PIPE_BIND_SAMPLER_VIEW)) {
      extensions->ARB_ES3_compatibility = GL_TRUE;
   }

#ifdef HAVE_ST_VDPAU
   if (screen->get_video_param &&
       screen->get_video_param(screen, PIPE_VIDEO_PROFILE_UNKNOWN,
                               PIPE_VIDEO_ENTRYPOINT_BITSTREAM,
                               PIPE_VIDEO_CAP_SUPPORTS_INTERLACED)) {
      extensions->NV_vdpau_interop = GL_TRUE;
   }
#endif

   if (screen->get_param(screen, PIPE_CAP_DOUBLES)) {
      extensions->ARB_gpu_shader_fp64 = GL_TRUE;
      extensions->ARB_vertex_attrib_64bit = GL_TRUE;
   }

   if ((ST_DEBUG & DEBUG_GREMEDY) &&
       screen->get_param(screen, PIPE_CAP_STRING_MARKER))
      extensions->GREMEDY_string_marker = GL_TRUE;

   if (screen->get_param(screen, PIPE_CAP_COMPUTE)) {
      uint64_t grid_size[3], block_size[3];
      uint64_t max_local_size, max_threads_per_block;

      screen->get_compute_param(screen, PIPE_SHADER_IR_NIR,
                                 PIPE_COMPUTE_CAP_MAX_GRID_SIZE, grid_size);
      screen->get_compute_param(screen, PIPE_SHADER_IR_NIR,
                                 PIPE_COMPUTE_CAP_MAX_BLOCK_SIZE, block_size);
      screen->get_compute_param(screen, PIPE_SHADER_IR_NIR,
                                 PIPE_COMPUTE_CAP_MAX_THREADS_PER_BLOCK,
                                 &max_threads_per_block);
      screen->get_compute_param(screen, PIPE_SHADER_IR_NIR,
                                 PIPE_COMPUTE_CAP_MAX_LOCAL_SIZE,
                                 &max_local_size);

      consts->MaxComputeWorkGroupInvocations = max_threads_per_block;
      consts->MaxComputeSharedMemorySize = max_local_size;

      for (i = 0; i < 3; i++) {
         /* There are tests that fail if we report more that INT_MAX - 1. */
         consts->MaxComputeWorkGroupCount[i] = MIN2(grid_size[i], INT_MAX - 1);
         consts->MaxComputeWorkGroupSize[i] = block_size[i];
      }

      extensions->ARB_compute_shader =
         max_threads_per_block >= 1024 &&
         extensions->ARB_shader_image_load_store &&
         extensions->ARB_shader_atomic_counters;

      if (extensions->ARB_compute_shader) {
         uint64_t max_variable_threads_per_block = 0;

         screen->get_compute_param(screen, PIPE_SHADER_IR_NIR,
                                    PIPE_COMPUTE_CAP_MAX_VARIABLE_THREADS_PER_BLOCK,
                                    &max_variable_threads_per_block);

         for (i = 0; i < 3; i++) {
            /* Clamp the values to avoid having a local work group size
               * greater than the maximum number of invocations.
               */
            consts->MaxComputeVariableGroupSize[i] =
               MIN2(consts->MaxComputeWorkGroupSize[i],
                     max_variable_threads_per_block);
         }
         consts->MaxComputeVariableGroupInvocations =
            max_variable_threads_per_block;

         extensions->ARB_compute_variable_group_size =
            max_variable_threads_per_block > 0;
      }
   }

   extensions->ARB_texture_float =
      extensions->OES_texture_half_float &&
      extensions->OES_texture_float;

   if (extensions->EXT_texture_filter_anisotropic &&
       screen->get_paramf(screen, PIPE_CAPF_MAX_TEXTURE_ANISOTROPY) >= 16.0)
      extensions->ARB_texture_filter_anisotropic = GL_TRUE;

   extensions->KHR_robustness = extensions->ARB_robust_buffer_access_behavior;

   /* If we support ES 3.1, we support the ES3_1_compatibility ext. However
    * there's no clean way of telling whether we would support ES 3.1 from
    * here, so copy the condition from compute_version_es2 here. A lot of
    * these are redunant, but simpler to just have a (near-)exact copy here.
    */
   extensions->ARB_ES3_1_compatibility =
      consts->Program[MESA_SHADER_FRAGMENT].MaxImageUniforms &&
      extensions->ARB_ES3_compatibility &&
      extensions->ARB_arrays_of_arrays &&
      extensions->ARB_compute_shader &&
      extensions->ARB_draw_indirect &&
      extensions->ARB_explicit_uniform_location &&
      extensions->ARB_framebuffer_no_attachments &&
      extensions->ARB_shader_atomic_counters &&
      extensions->ARB_shader_image_load_store &&
      extensions->ARB_shader_image_size &&
      extensions->ARB_shader_storage_buffer_object &&
      extensions->ARB_shading_language_packing &&
      extensions->ARB_stencil_texturing &&
      extensions->ARB_texture_multisample &&
      extensions->ARB_gpu_shader5 &&
      extensions->EXT_shader_integer_mix;

   extensions->OES_texture_cube_map_array =
      (extensions->ARB_ES3_1_compatibility || ESSLVersion >= 310) &&
      extensions->OES_geometry_shader &&
      extensions->ARB_texture_cube_map_array;

   extensions->OES_viewport_array =
      (extensions->ARB_ES3_1_compatibility || ESSLVersion >= 310) &&
      extensions->OES_geometry_shader &&
      extensions->ARB_viewport_array;

   extensions->OES_primitive_bounding_box =
      extensions->ARB_ES3_1_compatibility || ESSLVersion >= 310;

   consts->NoPrimitiveBoundingBoxOutput = true;

   extensions->ANDROID_extension_pack_es31a =
      consts->Program[MESA_SHADER_FRAGMENT].MaxImageUniforms &&
      extensions->KHR_texture_compression_astc_ldr &&
      extensions->KHR_blend_equation_advanced &&
      extensions->OES_sample_variables &&
      extensions->ARB_texture_stencil8 &&
      extensions->ARB_texture_multisample &&
      extensions->OES_copy_image &&
      extensions->ARB_draw_buffers_blend &&
      extensions->OES_geometry_shader &&
      extensions->ARB_gpu_shader5 &&
      extensions->OES_primitive_bounding_box &&
      extensions->ARB_tessellation_shader &&
      extensions->OES_texture_buffer &&
      extensions->OES_texture_cube_map_array &&
      extensions->EXT_texture_sRGB_decode;

   /* Same deal as for ARB_ES3_1_compatibility - this has to be computed
    * before overall versions are selected. Also it's actually a subset of ES
    * 3.2, since it doesn't require ASTC or advanced blending.
    */
   extensions->ARB_ES3_2_compatibility =
      extensions->ARB_ES3_1_compatibility &&
      extensions->KHR_robustness &&
      extensions->ARB_copy_image &&
      extensions->ARB_draw_buffers_blend &&
      extensions->ARB_draw_elements_base_vertex &&
      extensions->OES_geometry_shader &&
      extensions->ARB_gpu_shader5 &&
      extensions->ARB_sample_shading &&
      extensions->ARB_tessellation_shader &&
      extensions->OES_texture_buffer &&
      extensions->ARB_texture_cube_map_array &&
      extensions->ARB_texture_stencil8 &&
      extensions->ARB_texture_multisample;

   if (screen->get_param(screen, PIPE_CAP_CONSERVATIVE_RASTER_POST_SNAP_TRIANGLES) &&
       screen->get_param(screen, PIPE_CAP_CONSERVATIVE_RASTER_POST_SNAP_POINTS_LINES) &&
       screen->get_param(screen, PIPE_CAP_CONSERVATIVE_RASTER_POST_DEPTH_COVERAGE)) {
      float max_dilate;
      bool pre_snap_triangles, pre_snap_points_lines;

      max_dilate = screen->get_paramf(screen, PIPE_CAPF_MAX_CONSERVATIVE_RASTER_DILATE);

      pre_snap_triangles =
         screen->get_param(screen, PIPE_CAP_CONSERVATIVE_RASTER_PRE_SNAP_TRIANGLES);
      pre_snap_points_lines =
         screen->get_param(screen, PIPE_CAP_CONSERVATIVE_RASTER_PRE_SNAP_POINTS_LINES);

      extensions->NV_conservative_raster =
         screen->get_param(screen, PIPE_CAP_MAX_CONSERVATIVE_RASTER_SUBPIXEL_PRECISION_BIAS) > 1;

      if (extensions->NV_conservative_raster) {
         extensions->NV_conservative_raster_dilate = max_dilate >= 0.75;
         extensions->NV_conservative_raster_pre_snap_triangles = pre_snap_triangles;
         extensions->NV_conservative_raster_pre_snap =
            pre_snap_triangles && pre_snap_points_lines;
      }
   }

   if (extensions->ARB_gl_spirv) {
      struct spirv_supported_capabilities *spirv_caps = &consts->SpirVCapabilities;

      spirv_caps->atomic_storage             = extensions->ARB_shader_atomic_counters;
      spirv_caps->demote_to_helper_invocation = extensions->EXT_demote_to_helper_invocation;
      spirv_caps->draw_parameters            = extensions->ARB_shader_draw_parameters;
      spirv_caps->derivative_group           = extensions->NV_compute_shader_derivatives;
      spirv_caps->float64                    = extensions->ARB_gpu_shader_fp64;
      spirv_caps->geometry_streams           = extensions->ARB_gpu_shader5;
      spirv_caps->image_ms_array             = extensions->ARB_shader_image_load_store &&
                                               consts->MaxImageSamples > 1;
      spirv_caps->image_read_without_format  = extensions->EXT_shader_image_load_formatted;
      spirv_caps->image_write_without_format = extensions->ARB_shader_image_load_store;
      spirv_caps->int64                      = extensions->ARB_gpu_shader_int64;
      spirv_caps->int64_atomics              = extensions->NV_shader_atomic_int64;
      spirv_caps->post_depth_coverage        = extensions->ARB_post_depth_coverage;
      spirv_caps->shader_clock               = extensions->ARB_shader_clock;
      spirv_caps->shader_viewport_index_layer = extensions->ARB_shader_viewport_layer_array;
      spirv_caps->stencil_export             = extensions->ARB_shader_stencil_export;
      spirv_caps->storage_image_ms           = extensions->ARB_shader_image_load_store &&
                                               consts->MaxImageSamples > 1;
      spirv_caps->subgroup_ballot            = extensions->ARB_shader_ballot;
      spirv_caps->subgroup_vote              = extensions->ARB_shader_group_vote;
      spirv_caps->tessellation               = extensions->ARB_tessellation_shader;
      spirv_caps->transform_feedback         = extensions->ARB_transform_feedback3;
      spirv_caps->variable_pointers          =
         screen->get_param(screen, PIPE_CAP_GL_SPIRV_VARIABLE_POINTERS);
      spirv_caps->integer_functions2         = extensions->INTEL_shader_integer_functions2;

      consts->SpirVExtensions = CALLOC_STRUCT(spirv_supported_extensions);
      _mesa_fill_supported_spirv_extensions(consts->SpirVExtensions, spirv_caps);
   }

   consts->AllowDrawOutOfOrder =
      api == API_OPENGL_COMPAT &&
      options->allow_draw_out_of_order &&
      screen->get_param(screen, PIPE_CAP_ALLOW_DRAW_OUT_OF_ORDER);
   consts->GLThreadNopCheckFramebufferStatus = options->glthread_nop_check_framebuffer_status;

   const struct nir_shader_compiler_options *nir_options =
      consts->ShaderCompilerOptions[MESA_SHADER_FRAGMENT].NirOptions;

   if (screen->get_shader_param(screen, PIPE_SHADER_FRAGMENT, PIPE_SHADER_CAP_INTEGERS) &&
       extensions->ARB_stencil_texturing &&
       screen->get_param(screen, PIPE_CAP_DOUBLES) &&
       !(nir_options->lower_doubles_options & nir_lower_fp64_full_software))
      extensions->NV_copy_depth_to_color = true;
}
