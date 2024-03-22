/**************************************************************************
 *
 * Copyright 2012-2021 VMware, Inc.
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
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE COPYRIGHT HOLDERS, AUTHORS AND/OR ITS SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 **************************************************************************/

/*
 * Shader.cpp --
 *    Functions that manipulate shader resources.
 */


#include "Shader.h"
#include "ShaderParse.h"
#include "State.h"
#include "Query.h"

#include "Debug.h"
#include "Format.h"

#include "tgsi/tgsi_ureg.h"
#include "util/u_gen_mipmap.h"
#include "util/u_sampler.h"
#include "util/format/u_format.h"


/*
 * ----------------------------------------------------------------------
 *
 * CreateEmptyShader --
 *
 *    Update the driver's currently bound constant buffers.
 *
 * ----------------------------------------------------------------------
 */

void *
CreateEmptyShader(Device *pDevice,
                  enum pipe_shader_type processor)
{
   struct pipe_context *pipe = pDevice->pipe;
   struct ureg_program *ureg;
   const struct tgsi_token *tokens;
   uint nr_tokens;

   if (processor == PIPE_SHADER_GEOMETRY) {
      return NULL;
   }

   ureg = ureg_create(processor);
   if (!ureg)
      return NULL;

   ureg_END(ureg);

   tokens = ureg_get_tokens(ureg, &nr_tokens);
   if (!tokens)
      return NULL;

   ureg_destroy(ureg);

   struct pipe_shader_state state;
   memset(&state, 0, sizeof state);
   state.tokens = tokens;

   void *handle;
   switch (processor) {
   case PIPE_SHADER_FRAGMENT:
      handle = pipe->create_fs_state(pipe, &state);
      break;
   case PIPE_SHADER_VERTEX:
      handle = pipe->create_vs_state(pipe, &state);
      break;
   case PIPE_SHADER_GEOMETRY:
      handle = pipe->create_gs_state(pipe, &state);
      break;
   default:
      handle = NULL;
      assert(0);
   }
   assert(handle);

   ureg_free_tokens(tokens);

   return handle;
}


/*
 * ----------------------------------------------------------------------
 *
 * CreateEmptyShader --
 *
 *    Update the driver's currently bound constant buffers.
 *
 * ----------------------------------------------------------------------
 */

void
DeleteEmptyShader(Device *pDevice,
                  enum pipe_shader_type processor, void *handle)
{
   struct pipe_context *pipe = pDevice->pipe;

   if (processor == PIPE_SHADER_GEOMETRY) {
      assert(handle == NULL);
      return;
   }

   assert(handle != NULL);
   switch (processor) {
   case PIPE_SHADER_FRAGMENT:
      pipe->delete_fs_state(pipe, handle);
      break;
   case PIPE_SHADER_VERTEX:
      pipe->delete_vs_state(pipe, handle);
      break;
   case PIPE_SHADER_GEOMETRY:
      pipe->delete_gs_state(pipe, handle);
      break;
   default:
      assert(0);
   }
}


/*
 * ----------------------------------------------------------------------
 *
 * SetConstantBuffers --
 *
 *    Update the driver's currently bound constant buffers.
 *
 * ----------------------------------------------------------------------
 */

static void
SetConstantBuffers(enum pipe_shader_type shader_type,    // IN
                   D3D10DDI_HDEVICE hDevice,             // IN
                   UINT StartBuffer,                     // IN
                   UINT NumBuffers,                      // IN
                   const D3D10DDI_HRESOURCE *phBuffers) // IN
{
   Device *pDevice = CastDevice(hDevice);
   struct pipe_context *pipe = pDevice->pipe;

   for (UINT i = 0; i < NumBuffers; i++) {
      struct pipe_constant_buffer cb;
      memset(&cb, 0, sizeof cb);
      cb.buffer = CastPipeResource(phBuffers[i]);
      cb.buffer_offset = 0;
      cb.buffer_size = cb.buffer ? cb.buffer->width0 : 0;
      pipe->set_constant_buffer(pipe,
                                shader_type,
                                StartBuffer + i,
                                false,
                                &cb);
   }
}


/*
 * ----------------------------------------------------------------------
 *
 * SetSamplers --
 *
 *    Update the driver's currently bound sampler state.
 *
 * ----------------------------------------------------------------------
 */

static void
SetSamplers(enum pipe_shader_type shader_type,     // IN
            D3D10DDI_HDEVICE hDevice,              // IN
            UINT Offset,                          // IN
            UINT NumSamplers,                       // IN
            const D3D10DDI_HSAMPLER *phSamplers)  // IN
{
   Device *pDevice = CastDevice(hDevice);
   struct pipe_context *pipe = pDevice->pipe;

   void **samplers = pDevice->samplers[shader_type];
   for (UINT i = 0; i < NumSamplers; i++) {
      assert(Offset + i < PIPE_MAX_SAMPLERS);
      samplers[Offset + i] = CastPipeSamplerState(phSamplers[i]);
   }

   pipe->bind_sampler_states(pipe, shader_type, 0, PIPE_MAX_SAMPLERS, samplers);
}


/*
 * ----------------------------------------------------------------------
 *
 * SetSamplers --
 *
 *    Update the driver's currently bound sampler state.
 *
 * ----------------------------------------------------------------------
 */

static void
SetShaderResources(enum pipe_shader_type shader_type,                  // IN
                   D3D10DDI_HDEVICE hDevice,                                   // IN
                   UINT Offset,                                                // IN
                   UINT NumViews,                                              // IN
                   const D3D10DDI_HSHADERRESOURCEVIEW *phShaderResourceViews)  // IN
{
   Device *pDevice = CastDevice(hDevice);
   struct pipe_context *pipe = pDevice->pipe;

   assert(Offset + NumViews <= D3D10_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT);

   struct pipe_sampler_view **sampler_views = pDevice->sampler_views[shader_type];
   for (UINT i = 0; i < NumViews; i++) {
      struct pipe_sampler_view *sampler_view =
            CastPipeShaderResourceView(phShaderResourceViews[i]);
      if (Offset + i < PIPE_MAX_SHADER_SAMPLER_VIEWS) {
         sampler_views[Offset + i] = sampler_view;
      } else {
         if (sampler_view) {
            LOG_UNSUPPORTED(true);
            break;
         }
      }
   }

   /*
    * XXX: Now that the semantics are actually the same in gallium, should
    * probably think about not updating all always... It should just work.
    */
   pipe->set_sampler_views(pipe, shader_type, 0, PIPE_MAX_SHADER_SAMPLER_VIEWS,
                           0, false, sampler_views);
}


/*
 * ----------------------------------------------------------------------
 *
 * CalcPrivateShaderSize --
 *
 *    The CalcPrivateShaderSize function determines the size of
 *    the user-mode display driver's private region of memory
 *    (that is, the size of internal driver structures, not the
 *    size of the resource video memory) for a shader.
 *
 * ----------------------------------------------------------------------
 */

SIZE_T APIENTRY
CalcPrivateShaderSize(D3D10DDI_HDEVICE hDevice,                                  // IN
                      __in_ecount (pShaderCode[1]) const UINT *pShaderCode,      // IN
                      __in const D3D10DDIARG_STAGE_IO_SIGNATURES *pSignatures)   // IN
{
   return sizeof(Shader);
}


/*
 * ----------------------------------------------------------------------
 *
 * DestroyShader --
 *
 *    The DestroyShader function destroys the specified shader object.
 *    The shader object can be destoyed only if it is not currently
 *    bound to a display device.
 *
 * ----------------------------------------------------------------------
 */

void APIENTRY
DestroyShader(D3D10DDI_HDEVICE hDevice,   // IN
              D3D10DDI_HSHADER hShader)   // IN
{
   LOG_ENTRYPOINT();

   struct pipe_context *pipe = CastPipeContext(hDevice);
   Shader *pShader = CastShader(hShader);

   if (pShader->handle) {
      switch (pShader->type) {
      case PIPE_SHADER_FRAGMENT:
         pipe->delete_fs_state(pipe, pShader->handle);
         break;
      case PIPE_SHADER_VERTEX:
         pipe->delete_vs_state(pipe, pShader->handle);
         break;
      case PIPE_SHADER_GEOMETRY:
         pipe->delete_gs_state(pipe, pShader->handle);
         break;
      default:
         assert(0);
      }
   }

   if (pShader->state.tokens) {
      ureg_free_tokens(pShader->state.tokens);
   }
}


/*
 * ----------------------------------------------------------------------
 *
 * CalcPrivateSamplerSize --
 *
 *    The CalcPrivateSamplerSize function determines the size of the
 *    user-mode display driver's private region of memory (that is,
 *    the size of internal driver structures, not the size of the
 *    resource video memory) for a sampler.
 *
 * ----------------------------------------------------------------------
 */

SIZE_T APIENTRY
CalcPrivateSamplerSize(D3D10DDI_HDEVICE hDevice,                        // IN
                       __in const D3D10_DDI_SAMPLER_DESC *pSamplerDesc) // IN
{
   return sizeof(SamplerState);
}


static uint
translate_address_mode(D3D10_DDI_TEXTURE_ADDRESS_MODE AddressMode)
{
   switch (AddressMode) {
   case D3D10_DDI_TEXTURE_ADDRESS_WRAP:
      return PIPE_TEX_WRAP_REPEAT;
   case D3D10_DDI_TEXTURE_ADDRESS_MIRROR:
      return PIPE_TEX_WRAP_MIRROR_REPEAT;
   case D3D10_DDI_TEXTURE_ADDRESS_CLAMP:
      return PIPE_TEX_WRAP_CLAMP_TO_EDGE;
   case D3D10_DDI_TEXTURE_ADDRESS_BORDER:
      return PIPE_TEX_WRAP_CLAMP_TO_BORDER;
   case D3D10_DDI_TEXTURE_ADDRESS_MIRRORONCE:
      return PIPE_TEX_WRAP_MIRROR_CLAMP_TO_EDGE;
   default:
      assert(0);
      return PIPE_TEX_WRAP_REPEAT;
   }
}

static uint
translate_comparison(D3D10_DDI_COMPARISON_FUNC Func)
{
   switch (Func) {
   case D3D10_DDI_COMPARISON_NEVER:
      return PIPE_FUNC_NEVER;
   case D3D10_DDI_COMPARISON_LESS:
      return PIPE_FUNC_LESS;
   case D3D10_DDI_COMPARISON_EQUAL:
      return PIPE_FUNC_EQUAL;
   case D3D10_DDI_COMPARISON_LESS_EQUAL:
      return PIPE_FUNC_LEQUAL;
   case D3D10_DDI_COMPARISON_GREATER:
      return PIPE_FUNC_GREATER;
   case D3D10_DDI_COMPARISON_NOT_EQUAL:
      return PIPE_FUNC_NOTEQUAL;
   case D3D10_DDI_COMPARISON_GREATER_EQUAL:
      return PIPE_FUNC_GEQUAL;
   case D3D10_DDI_COMPARISON_ALWAYS:
      return PIPE_FUNC_ALWAYS;
   default:
      assert(0);
      return PIPE_FUNC_ALWAYS;
   }
}

static uint
translate_filter(D3D10_DDI_FILTER_TYPE Filter)
{
   switch (Filter) {
   case D3D10_DDI_FILTER_TYPE_POINT:
      return PIPE_TEX_FILTER_NEAREST;
   case D3D10_DDI_FILTER_TYPE_LINEAR:
      return PIPE_TEX_FILTER_LINEAR;
   default:
      assert(0);
      return PIPE_TEX_FILTER_NEAREST;
   }
}

static uint
translate_min_filter(D3D10_DDI_FILTER Filter)
{
   return translate_filter(D3D10_DDI_DECODE_MIN_FILTER(Filter));
}

static uint
translate_mag_filter(D3D10_DDI_FILTER Filter)
{
   return translate_filter(D3D10_DDI_DECODE_MAG_FILTER(Filter));
}

/* Gallium uses a different enum for mipfilters, to accomodate the GL
 * MIPFILTER_NONE mode.
 */
static uint
translate_mip_filter(D3D10_DDI_FILTER Filter)
{
   switch (D3D10_DDI_DECODE_MIP_FILTER(Filter)) {
   case D3D10_DDI_FILTER_TYPE_POINT:
      return PIPE_TEX_MIPFILTER_NEAREST;
   case D3D10_DDI_FILTER_TYPE_LINEAR:
      return PIPE_TEX_MIPFILTER_LINEAR;
   default:
      assert(0);
      return PIPE_TEX_MIPFILTER_NEAREST;
   }
}

/*
 * ----------------------------------------------------------------------
 *
 * CreateSampler --
 *
 *    The CreateSampler function creates a sampler.
 *
 * ----------------------------------------------------------------------
 */

void APIENTRY
CreateSampler(D3D10DDI_HDEVICE hDevice,                        // IN
              __in const D3D10_DDI_SAMPLER_DESC *pSamplerDesc, // IN
              D3D10DDI_HSAMPLER hSampler,                      // IN
              D3D10DDI_HRTSAMPLER hRTSampler)                  // IN
{
   LOG_ENTRYPOINT();

   struct pipe_context *pipe = CastPipeContext(hDevice);
   SamplerState *pSamplerState = CastSamplerState(hSampler);

   struct pipe_sampler_state state;

   memset(&state, 0, sizeof state);

   /* d3d10 has seamless cube filtering always enabled */
   state.seamless_cube_map = 1;

   /* Wrapping modes. */
   state.wrap_s = translate_address_mode(pSamplerDesc->AddressU);
   state.wrap_t = translate_address_mode(pSamplerDesc->AddressV);
   state.wrap_r = translate_address_mode(pSamplerDesc->AddressW);

   /* Filtering */
   state.min_img_filter = translate_min_filter(pSamplerDesc->Filter);
   state.mag_img_filter = translate_mag_filter(pSamplerDesc->Filter);
   state.min_mip_filter = translate_mip_filter(pSamplerDesc->Filter);

   if (D3D10_DDI_DECODE_IS_ANISOTROPIC_FILTER(pSamplerDesc->Filter)) {
      state.max_anisotropy = pSamplerDesc->MaxAnisotropy;
   }

   /* XXX: Handle the following bit.
    */
   LOG_UNSUPPORTED(D3D10_DDI_DECODE_IS_TEXT_1BIT_FILTER(pSamplerDesc->Filter));

   /* Comparison. */
   if (D3D10_DDI_DECODE_IS_COMPARISON_FILTER(pSamplerDesc->Filter)) {
      state.compare_mode = PIPE_TEX_COMPARE_R_TO_TEXTURE;
      state.compare_func = translate_comparison(pSamplerDesc->ComparisonFunc);
   }

   /* Level of detail. */
   state.lod_bias = pSamplerDesc->MipLODBias;
   state.min_lod = pSamplerDesc->MinLOD;
   state.max_lod = pSamplerDesc->MaxLOD;

   /* Border color. */
   state.border_color.f[0] = pSamplerDesc->BorderColor[0];
   state.border_color.f[1] = pSamplerDesc->BorderColor[1];
   state.border_color.f[2] = pSamplerDesc->BorderColor[2];
   state.border_color.f[3] = pSamplerDesc->BorderColor[3];

   pSamplerState->handle = pipe->create_sampler_state(pipe, &state);
}


/*
 * ----------------------------------------------------------------------
 *
 * DestroySampler --
 *
 *    The DestroySampler function destroys the specified sampler object.
 *    The sampler object can be destoyed only if it is not currently
 *    bound to a display device.
 *
 * ----------------------------------------------------------------------
 */

void APIENTRY
DestroySampler(D3D10DDI_HDEVICE hDevice,     // IN
               D3D10DDI_HSAMPLER hSampler)   // IN
{
   LOG_ENTRYPOINT();

   struct pipe_context *pipe = CastPipeContext(hDevice);
   SamplerState *pSamplerState = CastSamplerState(hSampler);

   pipe->delete_sampler_state(pipe, pSamplerState->handle);
}


/*
 * ----------------------------------------------------------------------
 *
 * CreateVertexShader --
 *
 *    The CreateVertexShader function creates a vertex shader.
 *
 * ----------------------------------------------------------------------
 */

void APIENTRY
CreateVertexShader(D3D10DDI_HDEVICE hDevice,                                  // IN
                   __in_ecount (pShaderCode[1]) const UINT *pCode,            // IN
                   D3D10DDI_HSHADER hShader,                                  // IN
                   D3D10DDI_HRTSHADER hRTShader,                              // IN
                   __in const D3D10DDIARG_STAGE_IO_SIGNATURES *pSignatures)   // IN
{
   LOG_ENTRYPOINT();

   struct pipe_context *pipe = CastPipeContext(hDevice);
   Shader *pShader = CastShader(hShader);

   pShader->type = PIPE_SHADER_VERTEX;
   pShader->output_resolved = true;

   memset(&pShader->state, 0, sizeof pShader->state);
   pShader->state.tokens = Shader_tgsi_translate(pCode, pShader->output_mapping);

   pShader->handle = pipe->create_vs_state(pipe, &pShader->state);

}


/*
 * ----------------------------------------------------------------------
 *
 * VsSetShader --
 *
 *    The VsSetShader function sets the vertex shader code so that all
 *    of the subsequent drawing operations use that code.
 *
 * ----------------------------------------------------------------------
 */

void APIENTRY
VsSetShader(D3D10DDI_HDEVICE hDevice,  // IN
            D3D10DDI_HSHADER hShader)  // IN
{
   LOG_ENTRYPOINT();

   Device *pDevice = CastDevice(hDevice);
   struct pipe_context *pipe = pDevice->pipe;
   Shader *pShader = CastShader(hShader);
   void *state = CastPipeShader(hShader);

   pDevice->bound_vs = pShader;
   if (!state) {
      state = pDevice->empty_vs;
   }

   pipe->bind_vs_state(pipe, state);
}


/*
 * ----------------------------------------------------------------------
 *
 * VsSetShaderResources --
 *
 *    The VsSetShaderResources function sets resources for a
 *    vertex shader.
 *
 * ----------------------------------------------------------------------
 */

void APIENTRY
VsSetShaderResources(D3D10DDI_HDEVICE hDevice,                                   // IN
                     UINT Offset,                                                // IN
                     UINT NumViews,                                              // IN
                     __in_ecount (NumViews)
                     const D3D10DDI_HSHADERRESOURCEVIEW *phShaderResourceViews)  // IN
{
   LOG_ENTRYPOINT();

   SetShaderResources(PIPE_SHADER_VERTEX, hDevice, Offset, NumViews, phShaderResourceViews);

}


/*
 * ----------------------------------------------------------------------
 *
 * VsSetConstantBuffers --
 *
 *    The VsSetConstantBuffers function sets constant buffers
 *    for a vertex shader.
 *
 * ----------------------------------------------------------------------
 */

void APIENTRY
VsSetConstantBuffers(D3D10DDI_HDEVICE hDevice,                                      // IN
                     UINT StartBuffer,                                              // IN
                     UINT NumBuffers,                                               // IN
                     __in_ecount (NumBuffers) const D3D10DDI_HRESOURCE *phBuffers)  // IN
{
   LOG_ENTRYPOINT();

   SetConstantBuffers(PIPE_SHADER_VERTEX,
                      hDevice, StartBuffer, NumBuffers, phBuffers);
}


/*
 * ----------------------------------------------------------------------
 *
 * VsSetSamplers --
 *
 *    The VsSetSamplers function sets samplers for a vertex shader.
 *
 * ----------------------------------------------------------------------
 */

void APIENTRY
VsSetSamplers(D3D10DDI_HDEVICE hDevice,                                       // IN
              UINT Offset,                                                    // IN
              UINT NumSamplers,                                               // IN
              __in_ecount (NumSamplers) const D3D10DDI_HSAMPLER *phSamplers)  // IN
{
   LOG_ENTRYPOINT();

   SetSamplers(PIPE_SHADER_VERTEX, hDevice, Offset, NumSamplers, phSamplers);

}


/*
 * ----------------------------------------------------------------------
 *
 * CreateGeometryShader --
 *
 *    The CreateGeometryShader function creates a geometry shader.
 *
 * ----------------------------------------------------------------------
 */

void APIENTRY
CreateGeometryShader(D3D10DDI_HDEVICE hDevice,                                // IN
                     __in_ecount (pShaderCode[1]) const UINT *pShaderCode,    // IN
                     D3D10DDI_HSHADER hShader,                                // IN
                     D3D10DDI_HRTSHADER hRTShader,                            // IN
                     __in const D3D10DDIARG_STAGE_IO_SIGNATURES *pSignatures) // IN
{
   LOG_ENTRYPOINT();

   struct pipe_context *pipe = CastPipeContext(hDevice);
   Shader *pShader = CastShader(hShader);

   pShader->type = PIPE_SHADER_GEOMETRY;
   pShader->output_resolved = true;

   memset(&pShader->state, 0, sizeof pShader->state);
   pShader->state.tokens = Shader_tgsi_translate(pShaderCode, pShader->output_mapping);

   pShader->handle = pipe->create_gs_state(pipe, &pShader->state);
}


/*
 * ----------------------------------------------------------------------
 *
 * GsSetShader --
 *
 *    The GsSetShader function sets the geometry shader code so that
 *    all of the subsequent drawing operations use that code.
 *
 * ----------------------------------------------------------------------
 */

void APIENTRY
GsSetShader(D3D10DDI_HDEVICE hDevice,  // IN
            D3D10DDI_HSHADER hShader)  // IN
{
   LOG_ENTRYPOINT();

   Device *pDevice = CastDevice(hDevice);
   struct pipe_context *pipe = CastPipeContext(hDevice);
   void *state = CastPipeShader(hShader);
   Shader *pShader = CastShader(hShader);

   assert(pipe->bind_gs_state);

   if (pShader && !pShader->state.tokens) {
      pDevice->bound_empty_gs = pShader;
   } else {
      pDevice->bound_empty_gs = NULL;
      pipe->bind_gs_state(pipe, state);
   }
}


/*
 * ----------------------------------------------------------------------
 *
 * GsSetShaderResources --
 *
 *    The GsSetShaderResources function sets resources for a
 *    geometry shader.
 *
 * ----------------------------------------------------------------------
 */

void APIENTRY
GsSetShaderResources(D3D10DDI_HDEVICE hDevice,                                   // IN
                     UINT Offset,                                                // IN
                     UINT NumViews,                                              // IN
                     __in_ecount (NumViews)
                     const D3D10DDI_HSHADERRESOURCEVIEW *phShaderResourceViews)  // IN
{
   LOG_ENTRYPOINT();

   SetShaderResources(PIPE_SHADER_GEOMETRY, hDevice, Offset, NumViews, phShaderResourceViews);
}


/*
 * ----------------------------------------------------------------------
 *
 * GsSetConstantBuffers --
 *
 *    The GsSetConstantBuffers function sets constant buffers for
 *    a geometry shader.
 *
 * ----------------------------------------------------------------------
 */

void APIENTRY
GsSetConstantBuffers(D3D10DDI_HDEVICE hDevice,                                      // IN
                     UINT StartBuffer,                                              // IN
                     UINT NumBuffers,                                               // IN
                     __in_ecount (NumBuffers) const D3D10DDI_HRESOURCE *phBuffers)  // IN
{
   LOG_ENTRYPOINT();

   SetConstantBuffers(PIPE_SHADER_GEOMETRY,
                      hDevice, StartBuffer, NumBuffers, phBuffers);
}


/*
 * ----------------------------------------------------------------------
 *
 * GsSetSamplers --
 *
 *    The GsSetSamplers function sets samplers for a geometry shader.
 *
 * ----------------------------------------------------------------------
 */

void APIENTRY
GsSetSamplers(D3D10DDI_HDEVICE hDevice,                                       // IN
              UINT Offset,                                                    // IN
              UINT NumSamplers,                                               // IN
              __in_ecount (NumSamplers) const D3D10DDI_HSAMPLER *phSamplers)  // IN
{
   LOG_ENTRYPOINT();

   SetSamplers(PIPE_SHADER_GEOMETRY, hDevice, Offset, NumSamplers, phSamplers);
}


/*
 * ----------------------------------------------------------------------
 *
 * CalcPrivateGeometryShaderWithStreamOutput --
 *
 *    The CalcPrivateGeometryShaderWithStreamOutput function determines
 *    the size of the user-mode display driver's private region of memory
 *    (that is, the size of internal driver structures, not the size of
 *    the resource video memory) for a geometry shader with stream output.
 *
 * ----------------------------------------------------------------------
 */

SIZE_T APIENTRY
CalcPrivateGeometryShaderWithStreamOutput(
   D3D10DDI_HDEVICE hDevice,                                                                             // IN
   __in const D3D10DDIARG_CREATEGEOMETRYSHADERWITHSTREAMOUTPUT *pCreateGeometryShaderWithStreamOutput,   // IN
   __in const D3D10DDIARG_STAGE_IO_SIGNATURES *pSignatures)                                              // IN
{
   LOG_ENTRYPOINT();
   return sizeof(Shader);
}


/*
 * ----------------------------------------------------------------------
 *
 * CreateGeometryShaderWithStreamOutput --
 *
 *    The CreateGeometryShaderWithStreamOutput function creates a
 *    geometry shader with stream output.
 *
 * ----------------------------------------------------------------------
 */

void APIENTRY
CreateGeometryShaderWithStreamOutput(
   D3D10DDI_HDEVICE hDevice,                                                                             // IN
   __in const D3D10DDIARG_CREATEGEOMETRYSHADERWITHSTREAMOUTPUT *pData,   // IN
   D3D10DDI_HSHADER hShader,                                                                             // IN
   D3D10DDI_HRTSHADER hRTShader,                                                                         // IN
   __in const D3D10DDIARG_STAGE_IO_SIGNATURES *pSignatures)                                              // IN
{
   LOG_ENTRYPOINT();

   struct pipe_context *pipe = CastPipeContext(hDevice);
   Shader *pShader = CastShader(hShader);
   int total_components[PIPE_MAX_SO_BUFFERS] = {0};
   unsigned num_holes = 0;
   bool all_slot_zero = true;

   pShader->type = PIPE_SHADER_GEOMETRY;

   memset(&pShader->state, 0, sizeof pShader->state);
   if (pData->pShaderCode) {
      pShader->state.tokens = Shader_tgsi_translate(pData->pShaderCode,
                                                    pShader->output_mapping);
   }
   pShader->output_resolved = (pShader->state.tokens != NULL);

   for (unsigned i = 0; i < pData->NumEntries; ++i) {
      CONST D3D10DDIARG_STREAM_OUTPUT_DECLARATION_ENTRY* pOutputStreamDecl =
            &pData->pOutputStreamDecl[i];
      BYTE RegisterMask = pOutputStreamDecl->RegisterMask;
      unsigned start_component = 0;
      unsigned num_components = 0;
      if (RegisterMask) {
         while ((RegisterMask & 1) == 0) {
            ++start_component;
            RegisterMask >>= 1;
         }
         while (RegisterMask) {
            ++num_components;
            RegisterMask >>= 1;
         }
         assert(start_component < 4);
         assert(1 <= num_components && num_components <= 4);
         LOG_UNSUPPORTED(((1 << num_components) - 1) << start_component !=
                         pOutputStreamDecl->RegisterMask);
      }

      if (pOutputStreamDecl->RegisterIndex == 0xffffffff) {
         ++num_holes;
      } else {
         unsigned idx = i - num_holes;
         pShader->state.stream_output.output[idx].start_component =
            start_component;
         pShader->state.stream_output.output[idx].num_components =
            num_components;
         pShader->state.stream_output.output[idx].output_buffer =
            pOutputStreamDecl->OutputSlot;
         pShader->state.stream_output.output[idx].register_index =
            ShaderFindOutputMapping(pShader, pOutputStreamDecl->RegisterIndex);
         pShader->state.stream_output.output[idx].dst_offset =
            total_components[pOutputStreamDecl->OutputSlot];
         if (pOutputStreamDecl->OutputSlot != 0)
            all_slot_zero = false;
      }
      total_components[pOutputStreamDecl->OutputSlot] += num_components;
   }
   pShader->state.stream_output.num_outputs = pData->NumEntries - num_holes;
   for (unsigned i = 0; i < PIPE_MAX_SO_BUFFERS; ++i) {
      /* stream_output.stride[i] is in dwords */
      if (all_slot_zero) {
         pShader->state.stream_output.stride[i] =
            pData->StreamOutputStrideInBytes / sizeof(float);
      } else {
         pShader->state.stream_output.stride[i] = total_components[i];
      }
   }

   pShader->handle = pipe->create_gs_state(pipe, &pShader->state);
}


/*
 * ----------------------------------------------------------------------
 *
 * SoSetTargets --
 *
 *    The SoSetTargets function sets stream output target resources.
 *
 * ----------------------------------------------------------------------
 */

void APIENTRY
SoSetTargets(D3D10DDI_HDEVICE hDevice,                                     // IN
             UINT SOTargets,                                               // IN
             UINT ClearTargets,                                            // IN
             __in_ecount (SOTargets) const D3D10DDI_HRESOURCE *phResource, // IN
             __in_ecount (SOTargets) const UINT *pOffsets)                 // IN
{
   unsigned i;

   LOG_ENTRYPOINT();

   Device *pDevice = CastDevice(hDevice);
   struct pipe_context *pipe = pDevice->pipe;

   assert(SOTargets + ClearTargets <= PIPE_MAX_SO_BUFFERS);

   for (i = 0; i < SOTargets; ++i) {
      Resource *resource = CastResource(phResource[i]);
      struct pipe_resource *buffer = CastPipeResource(phResource[i]);
      struct pipe_stream_output_target *so_target =
         resource ? resource->so_target : NULL;

      if (buffer) {
         unsigned buffer_size = buffer->width0;

         if (!so_target ||
             so_target->buffer != buffer ||
             so_target->buffer_size != buffer_size) {
            if (so_target) {
               pipe_so_target_reference(&so_target, NULL);
            }
            so_target = pipe->create_stream_output_target(pipe, buffer,
                                                          0,/*buffer offset*/
                                                          buffer_size);
            resource->so_target = so_target;
         }
      }
      pDevice->so_targets[i] = so_target;
   }

   for (i = 0; i < ClearTargets; ++i) {
      pDevice->so_targets[SOTargets + i] = NULL;
   }

   if (!pipe->set_stream_output_targets) {
      LOG_UNSUPPORTED(pipe->set_stream_output_targets);
      return;
   }

   pipe->set_stream_output_targets(pipe, SOTargets, pDevice->so_targets,
                                   pOffsets);
}


/*
 * ----------------------------------------------------------------------
 *
 * CreatePixelShader --
 *
 *    The CreatePixelShader function converts pixel shader code into a
 *    hardware-specific format and associates this code with a
 *    shader handle.
 *
 * ----------------------------------------------------------------------
 */

void APIENTRY
CreatePixelShader(D3D10DDI_HDEVICE hDevice,                                // IN
                  __in_ecount (pShaderCode[1]) const UINT *pShaderCode,    // IN
                  D3D10DDI_HSHADER hShader,                                // IN
                  D3D10DDI_HRTSHADER hRTShader,                            // IN
                  __in const D3D10DDIARG_STAGE_IO_SIGNATURES *pSignatures) // IN
{
   LOG_ENTRYPOINT();

   struct pipe_context *pipe = CastPipeContext(hDevice);
   Shader *pShader = CastShader(hShader);

   pShader->type = PIPE_SHADER_FRAGMENT;
   pShader->output_resolved = true;

   memset(&pShader->state, 0, sizeof pShader->state);
   pShader->state.tokens = Shader_tgsi_translate(pShaderCode,
                                                 pShader->output_mapping);

   pShader->handle = pipe->create_fs_state(pipe, &pShader->state);

}


/*
 * ----------------------------------------------------------------------
 *
 * PsSetShader --
 *
 *    The PsSetShader function sets a pixel shader to be used
 *    in all drawing operations.
 *
 * ----------------------------------------------------------------------
 */

void APIENTRY
PsSetShader(D3D10DDI_HDEVICE hDevice,  // IN
            D3D10DDI_HSHADER hShader)  // IN
{
   LOG_ENTRYPOINT();

   Device *pDevice = CastDevice(hDevice);
   struct pipe_context *pipe = pDevice->pipe;
   void *state = CastPipeShader(hShader);

   if (!state) {
      state = pDevice->empty_fs;
   }

   pipe->bind_fs_state(pipe, state);
}


/*
 * ----------------------------------------------------------------------
 *
 * PsSetShaderResources --
 *
 *    The PsSetShaderResources function sets resources for a pixel shader.
 *
 * ----------------------------------------------------------------------
 */

void APIENTRY
PsSetShaderResources(D3D10DDI_HDEVICE hDevice,                                   // IN
                     UINT Offset,                                                // IN
                     UINT NumViews,                                              // IN
                     __in_ecount (NumViews)
                     const D3D10DDI_HSHADERRESOURCEVIEW *phShaderResourceViews)  // IN
{
   LOG_ENTRYPOINT();

   SetShaderResources(PIPE_SHADER_FRAGMENT, hDevice, Offset, NumViews, phShaderResourceViews);
}


/*
 * ----------------------------------------------------------------------
 *
 * PsSetConstantBuffers --
 *
 *    The PsSetConstantBuffers function sets constant buffers for
 *    a pixel shader.
 *
 * ----------------------------------------------------------------------
 */

void APIENTRY
PsSetConstantBuffers(D3D10DDI_HDEVICE hDevice,                                      // IN
                     UINT StartBuffer,                                              // IN
                     UINT NumBuffers,                                               // IN
                     __in_ecount (NumBuffers) const D3D10DDI_HRESOURCE *phBuffers)  // IN
{
   LOG_ENTRYPOINT();

   SetConstantBuffers(PIPE_SHADER_FRAGMENT,
                      hDevice, StartBuffer, NumBuffers, phBuffers);
}

/*
 * ----------------------------------------------------------------------
 *
 * PsSetSamplers --
 *
 *    The PsSetSamplers function sets samplers for a pixel shader.
 *
 * ----------------------------------------------------------------------
 */

void APIENTRY
PsSetSamplers(D3D10DDI_HDEVICE hDevice,                                       // IN
              UINT Offset,                                                    // IN
              UINT NumSamplers,                                               // IN
              __in_ecount (NumSamplers) const D3D10DDI_HSAMPLER *phSamplers)  // IN
{
   LOG_ENTRYPOINT();

   SetSamplers(PIPE_SHADER_FRAGMENT, hDevice, Offset, NumSamplers, phSamplers);
}


/*
 * ----------------------------------------------------------------------
 *
 * ShaderResourceViewReadAfterWriteHazard --
 *
 *    The ShaderResourceViewReadAfterWriteHazard function informs
 *    the usermode display driver that the specified resource was
 *    used as an output from the graphics processing unit (GPU)
 *    and that the resource will be used as an input to the GPU.
 *    A shader resource view is also provided to indicate which
 *    view caused the hazard.
 *
 * ----------------------------------------------------------------------
 */

void APIENTRY
ShaderResourceViewReadAfterWriteHazard(D3D10DDI_HDEVICE hDevice,                          // IN
                                       D3D10DDI_HSHADERRESOURCEVIEW hShaderResourceView,  // IN
                                       D3D10DDI_HRESOURCE hResource)                      // IN
{
   LOG_ENTRYPOINT();

   /* Not actually necessary */
}


/*
 * ----------------------------------------------------------------------
 *
 * CalcPrivateShaderResourceViewSize --
 *
 *    The CalcPrivateShaderResourceViewSize function determines the size
 *    of the usermode display driver's private region of memory
 *    (that is, the size of internal driver structures, not the size of
 *    the resource video memory) for a shader resource view.
 *
 * ----------------------------------------------------------------------
 */

SIZE_T APIENTRY
CalcPrivateShaderResourceViewSize(
   D3D10DDI_HDEVICE hDevice,                                                     // IN
   __in const D3D10DDIARG_CREATESHADERRESOURCEVIEW *pCreateSRView)   // IN
{
   return sizeof(ShaderResourceView);
}


/*
 * ----------------------------------------------------------------------
 *
 * CalcPrivateShaderResourceViewSize --
 *
 *    The CalcPrivateShaderResourceViewSize function determines the size
 *    of the usermode display driver's private region of memory
 *    (that is, the size of internal driver structures, not the size of
 *    the resource video memory) for a shader resource view.
 *
 * ----------------------------------------------------------------------
 */

SIZE_T APIENTRY
CalcPrivateShaderResourceViewSize1(
   D3D10DDI_HDEVICE hDevice,                                                     // IN
   __in const D3D10_1DDIARG_CREATESHADERRESOURCEVIEW *pCreateSRView)   // IN
{
   return sizeof(ShaderResourceView);
}


/*
 * ----------------------------------------------------------------------
 *
 * CreateShaderResourceView --
 *
 *    The CreateShaderResourceView function creates a shader
 *    resource view.
 *
 * ----------------------------------------------------------------------
 */

void APIENTRY
CreateShaderResourceView(
   D3D10DDI_HDEVICE hDevice,                                                     // IN
   __in const D3D10DDIARG_CREATESHADERRESOURCEVIEW *pCreateSRView,   // IN
   D3D10DDI_HSHADERRESOURCEVIEW hShaderResourceView,                             // IN
   D3D10DDI_HRTSHADERRESOURCEVIEW hRTShaderResourceView)                         // IN
{
   LOG_ENTRYPOINT();

   struct pipe_context *pipe = CastPipeContext(hDevice);
   ShaderResourceView *pSRView = CastShaderResourceView(hShaderResourceView);
   struct pipe_resource *resource;
   enum pipe_format format;

   struct pipe_sampler_view desc;
   memset(&desc, 0, sizeof desc);
   resource = CastPipeResource(pCreateSRView->hDrvResource);
   format = FormatTranslate(pCreateSRView->Format, false);

   u_sampler_view_default_template(&desc,
                                   resource,
                                   format);

   switch (pCreateSRView->ResourceDimension) {
   case D3D10DDIRESOURCE_BUFFER: {
         const struct util_format_description *fdesc = util_format_description(format);
         desc.u.buf.offset = pCreateSRView->Buffer.FirstElement *
                             (fdesc->block.bits / 8) * fdesc->block.width;
         desc.u.buf.size = pCreateSRView->Buffer.NumElements *
                             (fdesc->block.bits / 8) * fdesc->block.width;
      }
      break;
   case D3D10DDIRESOURCE_TEXTURE1D:
      desc.u.tex.first_level = pCreateSRView->Tex1D.MostDetailedMip;
      desc.u.tex.last_level = pCreateSRView->Tex1D.MipLevels - 1 + desc.u.tex.first_level;
      desc.u.tex.first_layer = pCreateSRView->Tex1D.FirstArraySlice;
      desc.u.tex.last_layer = pCreateSRView->Tex1D.ArraySize - 1 + desc.u.tex.first_layer;
      assert(pCreateSRView->Tex1D.MipLevels != 0 && pCreateSRView->Tex1D.MipLevels != (UINT)-1);
      assert(pCreateSRView->Tex1D.ArraySize != 0 && pCreateSRView->Tex1D.ArraySize != (UINT)-1);
      break;
   case D3D10DDIRESOURCE_TEXTURE2D:
      desc.u.tex.first_level = pCreateSRView->Tex2D.MostDetailedMip;
      desc.u.tex.last_level = pCreateSRView->Tex2D.MipLevels - 1 + desc.u.tex.first_level;
      desc.u.tex.first_layer = pCreateSRView->Tex2D.FirstArraySlice;
      desc.u.tex.last_layer = pCreateSRView->Tex2D.ArraySize - 1 + desc.u.tex.first_layer;
      assert(pCreateSRView->Tex2D.MipLevels != 0 && pCreateSRView->Tex2D.MipLevels != (UINT)-1);
      assert(pCreateSRView->Tex2D.ArraySize != 0 && pCreateSRView->Tex2D.ArraySize != (UINT)-1);
      break;
   case D3D10DDIRESOURCE_TEXTURE3D:
      desc.u.tex.first_level = pCreateSRView->Tex3D.MostDetailedMip;
      desc.u.tex.last_level = pCreateSRView->Tex3D.MipLevels - 1 + desc.u.tex.first_level;
      /* layer info filled in by default_template */
      assert(pCreateSRView->Tex3D.MipLevels != 0 && pCreateSRView->Tex3D.MipLevels != (UINT)-1);
      break;
   case D3D10DDIRESOURCE_TEXTURECUBE:
      desc.u.tex.first_level = pCreateSRView->TexCube.MostDetailedMip;
      desc.u.tex.last_level = pCreateSRView->TexCube.MipLevels - 1 + desc.u.tex.first_level;
      /* layer info filled in by default_template */
      assert(pCreateSRView->TexCube.MipLevels != 0 && pCreateSRView->TexCube.MipLevels != (UINT)-1);
      break;
   default:
      assert(0);
      return;
   }

   pSRView->handle = pipe->create_sampler_view(pipe, resource, &desc);
}


/*
 * ----------------------------------------------------------------------
 *
 * CreateShaderResourceView1 --
 *
 *    The CreateShaderResourceView function creates a shader
 *    resource view.
 *
 * ----------------------------------------------------------------------
 */

void APIENTRY
CreateShaderResourceView1(
   D3D10DDI_HDEVICE hDevice,                                                     // IN
   __in const D3D10_1DDIARG_CREATESHADERRESOURCEVIEW *pCreateSRView,   // IN
   D3D10DDI_HSHADERRESOURCEVIEW hShaderResourceView,                             // IN
   D3D10DDI_HRTSHADERRESOURCEVIEW hRTShaderResourceView)                         // IN
{
   LOG_ENTRYPOINT();

   struct pipe_context *pipe = CastPipeContext(hDevice);
   ShaderResourceView *pSRView = CastShaderResourceView(hShaderResourceView);
   struct pipe_resource *resource;
   enum pipe_format format;

   struct pipe_sampler_view desc;
   memset(&desc, 0, sizeof desc);
   resource = CastPipeResource(pCreateSRView->hDrvResource);
   format = FormatTranslate(pCreateSRView->Format, false);

   u_sampler_view_default_template(&desc,
                                   resource,
                                   format);

   switch (pCreateSRView->ResourceDimension) {
   case D3D10DDIRESOURCE_BUFFER: {
         const struct util_format_description *fdesc = util_format_description(format);
         desc.u.buf.offset = pCreateSRView->Buffer.FirstElement *
                             (fdesc->block.bits / 8) * fdesc->block.width;
         desc.u.buf.size = pCreateSRView->Buffer.NumElements *
                             (fdesc->block.bits / 8) * fdesc->block.width;
      }
      break;
   case D3D10DDIRESOURCE_TEXTURE1D:
      desc.u.tex.first_level = pCreateSRView->Tex1D.MostDetailedMip;
      desc.u.tex.last_level = pCreateSRView->Tex1D.MipLevels - 1 + desc.u.tex.first_level;
      desc.u.tex.first_layer = pCreateSRView->Tex1D.FirstArraySlice;
      desc.u.tex.last_layer = pCreateSRView->Tex1D.ArraySize - 1 + desc.u.tex.first_layer;
      assert(pCreateSRView->Tex1D.MipLevels != 0 && pCreateSRView->Tex1D.MipLevels != (UINT)-1);
      assert(pCreateSRView->Tex1D.ArraySize != 0 && pCreateSRView->Tex1D.ArraySize != (UINT)-1);
      break;
   case D3D10DDIRESOURCE_TEXTURE2D:
      desc.u.tex.first_level = pCreateSRView->Tex2D.MostDetailedMip;
      desc.u.tex.last_level = pCreateSRView->Tex2D.MipLevels - 1 + desc.u.tex.first_level;
      desc.u.tex.first_layer = pCreateSRView->Tex2D.FirstArraySlice;
      desc.u.tex.last_layer = pCreateSRView->Tex2D.ArraySize - 1 + desc.u.tex.first_layer;
      assert(pCreateSRView->Tex2D.MipLevels != 0 && pCreateSRView->Tex2D.MipLevels != (UINT)-1);
      assert(pCreateSRView->Tex2D.ArraySize != 0 && pCreateSRView->Tex2D.ArraySize != (UINT)-1);
      break;
   case D3D10DDIRESOURCE_TEXTURE3D:
      desc.u.tex.first_level = pCreateSRView->Tex3D.MostDetailedMip;
      desc.u.tex.last_level = pCreateSRView->Tex3D.MipLevels - 1 + desc.u.tex.first_level;
      /* layer info filled in by default_template */
      assert(pCreateSRView->Tex3D.MipLevels != 0 && pCreateSRView->Tex3D.MipLevels != (UINT)-1);
      break;
   case D3D10DDIRESOURCE_TEXTURECUBE:
      desc.u.tex.first_level = pCreateSRView->TexCube.MostDetailedMip;
      desc.u.tex.last_level = pCreateSRView->TexCube.MipLevels - 1 + desc.u.tex.first_level;
      desc.u.tex.first_layer = pCreateSRView->TexCube.First2DArrayFace;
      desc.u.tex.last_layer = 6*pCreateSRView->TexCube.NumCubes - 1 +
                              pCreateSRView->TexCube.First2DArrayFace;
      assert(pCreateSRView->TexCube.MipLevels != 0 && pCreateSRView->TexCube.MipLevels != (UINT)-1);
      break;
   default:
      assert(0);
      return;
   }

   pSRView->handle = pipe->create_sampler_view(pipe, resource, &desc);
}


/*
 * ----------------------------------------------------------------------
 *
 * DestroyShaderResourceView --
 *
 *    The DestroyShaderResourceView function destroys the specified
 *    shader resource view object. The shader resource view object
 *    can be destoyed only if it is not currently bound to a
 *    display device.
 *
 * ----------------------------------------------------------------------
 */

void APIENTRY
DestroyShaderResourceView(D3D10DDI_HDEVICE hDevice,                           // IN
                          D3D10DDI_HSHADERRESOURCEVIEW hShaderResourceView)   // IN
{
   LOG_ENTRYPOINT();

   ShaderResourceView *pSRView = CastShaderResourceView(hShaderResourceView);

   pipe_sampler_view_reference(&pSRView->handle, NULL);
}


/*
 * ----------------------------------------------------------------------
 *
 * GenMips --
 *
 *    The GenMips function generates the lower MIP-map levels
 *    on the specified shader-resource view.
 *
 * ----------------------------------------------------------------------
 */

void APIENTRY
GenMips(D3D10DDI_HDEVICE hDevice,                           // IN
        D3D10DDI_HSHADERRESOURCEVIEW hShaderResourceView)   // IN
{
   LOG_ENTRYPOINT();

   Device *pDevice = CastDevice(hDevice);
   if (!CheckPredicate(pDevice)) {
      return;
   }

   struct pipe_context *pipe = pDevice->pipe;
   struct pipe_sampler_view *sampler_view = CastPipeShaderResourceView(hShaderResourceView);

   util_gen_mipmap(pipe,
                   sampler_view->texture,
                   sampler_view->format,
                   sampler_view->u.tex.first_level,
                   sampler_view->u.tex.last_level,
                   sampler_view->u.tex.first_layer,
                   sampler_view->u.tex.last_layer,
                   PIPE_TEX_FILTER_LINEAR);
}


unsigned
ShaderFindOutputMapping(Shader *shader, unsigned registerIndex)
{
   if (!shader || !shader->state.tokens)
      return registerIndex;

   for (unsigned i = 0; i < PIPE_MAX_SHADER_OUTPUTS; ++i) {
      if (shader->output_mapping[i] == registerIndex)
         return i;
   }
   return registerIndex;
}
