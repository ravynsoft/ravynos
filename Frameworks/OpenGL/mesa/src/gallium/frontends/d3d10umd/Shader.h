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
 * Shader.h --
 *    Functions that manipulate shader resources.
 */

#ifndef SHADER_H
#define SHADER_H

#include "DriverIncludes.h"

struct Device;
struct Shader;

void *
CreateEmptyShader(Device *pDevice,
                  enum pipe_shader_type processor);

void
DeleteEmptyShader(Device *pDevice,
                  enum pipe_shader_type processor, void *handle);

unsigned
ShaderFindOutputMapping(Shader *shader, unsigned registerIndex);

SIZE_T APIENTRY
CalcPrivateShaderSize(D3D10DDI_HDEVICE hDevice,
                      __in_ecount (pShaderCode[1]) const UINT *pShaderCode,
                      __in const D3D10DDIARG_STAGE_IO_SIGNATURES *pSignatures);

void APIENTRY
DestroyShader(D3D10DDI_HDEVICE hDevice, D3D10DDI_HSHADER hShader);

SIZE_T APIENTRY
CalcPrivateSamplerSize(D3D10DDI_HDEVICE hDevice,
                       __in const D3D10_DDI_SAMPLER_DESC *pSamplerDesc);

void APIENTRY CreateSampler(D3D10DDI_HDEVICE hDevice,
                   __in const D3D10_DDI_SAMPLER_DESC *pSamplerDesc,
                   D3D10DDI_HSAMPLER hSampler, D3D10DDI_HRTSAMPLER hRTSampler);

void APIENTRY DestroySampler(D3D10DDI_HDEVICE hDevice, D3D10DDI_HSAMPLER hSampler);

void APIENTRY CreateVertexShader(D3D10DDI_HDEVICE hDevice,
                        __in_ecount (pShaderCode[1]) const UINT *pCode,
                        D3D10DDI_HSHADER hShader, D3D10DDI_HRTSHADER hRTShader,
                        __in const D3D10DDIARG_STAGE_IO_SIGNATURES *pSignatures);

void APIENTRY VsSetShader(D3D10DDI_HDEVICE hDevice, D3D10DDI_HSHADER hShader);

void APIENTRY VsSetShaderResources(
   D3D10DDI_HDEVICE hDevice, UINT Offset, UINT NumViews,
   __in_ecount (NumViews) const D3D10DDI_HSHADERRESOURCEVIEW *phShaderResourceViews);
void APIENTRY VsSetConstantBuffers(D3D10DDI_HDEVICE hDevice, UINT StartBuffer, UINT NumBuffers,
                          __in_ecount (NumBuffers) const D3D10DDI_HRESOURCE *phBuffers);

void APIENTRY VsSetSamplers(D3D10DDI_HDEVICE hDevice, UINT Offset, UINT NumSamplers,
                   __in_ecount (NumSamplers) const D3D10DDI_HSAMPLER *phSamplers);

void APIENTRY CreateGeometryShader(D3D10DDI_HDEVICE hDevice,
                          __in_ecount (pShaderCode[1]) const UINT *pCode,
                          D3D10DDI_HSHADER hShader, D3D10DDI_HRTSHADER hRTShader,
                          __in const D3D10DDIARG_STAGE_IO_SIGNATURES *pSignatures);

void APIENTRY GsSetShader(D3D10DDI_HDEVICE hDevice, D3D10DDI_HSHADER hShader);

void APIENTRY GsSetShaderResources(
   D3D10DDI_HDEVICE hDevice, UINT Offset, UINT NumViews,
   __in_ecount (NumViews) const D3D10DDI_HSHADERRESOURCEVIEW *phShaderResourceViews);

void APIENTRY GsSetConstantBuffers(D3D10DDI_HDEVICE hDevice, UINT StartBuffer, UINT NumBuffers,
                          __in_ecount (NumBuffers) const D3D10DDI_HRESOURCE *phBuffers);

void APIENTRY GsSetSamplers(D3D10DDI_HDEVICE hDevice, UINT Offset, UINT NumSamplers,
                   __in_ecount (NumSamplers) const D3D10DDI_HSAMPLER *phSamplers);

SIZE_T APIENTRY CalcPrivateGeometryShaderWithStreamOutput(
   D3D10DDI_HDEVICE hDevice,
   __in const D3D10DDIARG_CREATEGEOMETRYSHADERWITHSTREAMOUTPUT *pCreateGeometryShaderWithStreamOutput,
   __in const D3D10DDIARG_STAGE_IO_SIGNATURES *pSignatures);

void APIENTRY CreateGeometryShaderWithStreamOutput(
   D3D10DDI_HDEVICE hDevice,
   __in const D3D10DDIARG_CREATEGEOMETRYSHADERWITHSTREAMOUTPUT *pCreateGeometryShaderWithStreamOutput,
   D3D10DDI_HSHADER hShader, D3D10DDI_HRTSHADER hRTShader,
   __in const D3D10DDIARG_STAGE_IO_SIGNATURES *pSignatures);

void APIENTRY SoSetTargets(D3D10DDI_HDEVICE hDevice, UINT SOTargets, UINT ClearTargets,
                  __in_ecount (SOTargets) const D3D10DDI_HRESOURCE *phResource,
                  __in_ecount (SOTargets) const UINT *pOffsets);

void APIENTRY CreatePixelShader(D3D10DDI_HDEVICE hDevice,
                       __in_ecount (pShaderCode[1]) const UINT *pCode,
                       D3D10DDI_HSHADER hShader, D3D10DDI_HRTSHADER hRTShader,
                       __in const D3D10DDIARG_STAGE_IO_SIGNATURES *pSignatures);
void APIENTRY PsSetShader(D3D10DDI_HDEVICE hDevice, D3D10DDI_HSHADER hShader);

void APIENTRY PsSetShaderResources(
   D3D10DDI_HDEVICE hDevice, UINT Offset, UINT NumViews,
   __in_ecount (NumViews) const D3D10DDI_HSHADERRESOURCEVIEW *phShaderResourceViews);

void APIENTRY PsSetConstantBuffers(D3D10DDI_HDEVICE hDevice, UINT StartBuffer, UINT NumBuffers,
                          __in_ecount (NumBuffers) const D3D10DDI_HRESOURCE *phBuffers);

void APIENTRY PsSetSamplers(D3D10DDI_HDEVICE hDevice, UINT Offset, UINT NumSamplers,
                   __in_ecount (NumSamplers) const D3D10DDI_HSAMPLER *phSamplers);

void APIENTRY ShaderResourceViewReadAfterWriteHazard(
   D3D10DDI_HDEVICE hDevice, D3D10DDI_HSHADERRESOURCEVIEW hShaderResourceView,
   D3D10DDI_HRESOURCE hResource);

SIZE_T APIENTRY CalcPrivateShaderResourceViewSize(
   D3D10DDI_HDEVICE hDevice,
   __in const D3D10DDIARG_CREATESHADERRESOURCEVIEW *pCreateShaderResourceView);

void APIENTRY CreateShaderResourceView(
   D3D10DDI_HDEVICE hDevice,
   __in const D3D10DDIARG_CREATESHADERRESOURCEVIEW *pCreateShaderResourceView,
   D3D10DDI_HSHADERRESOURCEVIEW hShaderResourceView,
   D3D10DDI_HRTSHADERRESOURCEVIEW hRTShaderResourceView);

SIZE_T APIENTRY CalcPrivateShaderResourceViewSize1(
   D3D10DDI_HDEVICE hDevice,
   __in const D3D10_1DDIARG_CREATESHADERRESOURCEVIEW *pCreateShaderResourceView);

void APIENTRY CreateShaderResourceView1(
   D3D10DDI_HDEVICE hDevice,
   __in const D3D10_1DDIARG_CREATESHADERRESOURCEVIEW *pCreateShaderResourceView,
   D3D10DDI_HSHADERRESOURCEVIEW hShaderResourceView,
   D3D10DDI_HRTSHADERRESOURCEVIEW hRTShaderResourceView);

void APIENTRY DestroyShaderResourceView(D3D10DDI_HDEVICE hDevice,
                               D3D10DDI_HSHADERRESOURCEVIEW hShaderResourceView);

void APIENTRY GenMips(D3D10DDI_HDEVICE hDevice,
             D3D10DDI_HSHADERRESOURCEVIEW hShaderResourceView);

#endif   /* SHADER_H */
