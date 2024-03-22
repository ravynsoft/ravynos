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
 * OutputMerger.h --
 *    Functions that manipulate the output merger state.
 */

#ifndef OUTPUT_MERGER_H
#define OUTPUT_MERGER_H

#include "DriverIncludes.h"

SIZE_T APIENTRY CalcPrivateRenderTargetViewSize(
   D3D10DDI_HDEVICE hDevice,
   __in const D3D10DDIARG_CREATERENDERTARGETVIEW *pCreateRenderTargetView);

void APIENTRY CreateRenderTargetView(
   D3D10DDI_HDEVICE hDevice,
   __in const D3D10DDIARG_CREATERENDERTARGETVIEW *pCreateRenderTargetView,
   D3D10DDI_HRENDERTARGETVIEW hRenderTargetView,
   D3D10DDI_HRTRENDERTARGETVIEW hRTRenderTargetView);

void APIENTRY DestroyRenderTargetView(D3D10DDI_HDEVICE hDevice,
                             D3D10DDI_HRENDERTARGETVIEW hRenderTargetView);

void APIENTRY ClearRenderTargetView(D3D10DDI_HDEVICE hDevice,
                           D3D10DDI_HRENDERTARGETVIEW hRenderTargetView,
                           FLOAT pColorRGBA[4]);

SIZE_T APIENTRY CalcPrivateDepthStencilViewSize(
   D3D10DDI_HDEVICE hDevice,
   __in const D3D10DDIARG_CREATEDEPTHSTENCILVIEW *pCreateDepthStencilView);

void APIENTRY CreateDepthStencilView(
   D3D10DDI_HDEVICE hDevice,
   __in const D3D10DDIARG_CREATEDEPTHSTENCILVIEW *pCreateDepthStencilView,
   D3D10DDI_HDEPTHSTENCILVIEW hDepthStencilView,
   D3D10DDI_HRTDEPTHSTENCILVIEW hRTDepthStencilView);

void APIENTRY DestroyDepthStencilView(D3D10DDI_HDEVICE hDevice,
                             D3D10DDI_HDEPTHSTENCILVIEW hDepthStencilView);

void APIENTRY ClearDepthStencilView(D3D10DDI_HDEVICE hDevice,
                           D3D10DDI_HDEPTHSTENCILVIEW hDepthStencilView,
                           UINT Flags, FLOAT Depth, UINT8 Stencil);

SIZE_T APIENTRY CalcPrivateBlendStateSize(D3D10DDI_HDEVICE hDevice,
                                 __in const D3D10_DDI_BLEND_DESC *pBlendDesc);

SIZE_T APIENTRY CalcPrivateBlendStateSize1(D3D10DDI_HDEVICE hDevice,
                                 __in const D3D10_1_DDI_BLEND_DESC *pBlendDesc);

void APIENTRY CreateBlendState(D3D10DDI_HDEVICE hDevice,
                      __in const D3D10_DDI_BLEND_DESC *pBlendDesc,
                      D3D10DDI_HBLENDSTATE hBlendState,
                      D3D10DDI_HRTBLENDSTATE hRTBlendState);

void APIENTRY CreateBlendState1(D3D10DDI_HDEVICE hDevice,
                      __in const D3D10_1_DDI_BLEND_DESC *pBlendDesc,
                      D3D10DDI_HBLENDSTATE hBlendState,
                      D3D10DDI_HRTBLENDSTATE hRTBlendState);

void APIENTRY DestroyBlendState(D3D10DDI_HDEVICE hDevice, D3D10DDI_HBLENDSTATE hBlendState);

void APIENTRY SetBlendState(D3D10DDI_HDEVICE hDevice, D3D10DDI_HBLENDSTATE hState,
                   const FLOAT pBlendFactor[4], UINT SampleMask);

void APIENTRY SetRenderTargets(
   D3D10DDI_HDEVICE hDevice,
   __in_ecount (NumViews) const D3D10DDI_HRENDERTARGETVIEW *phRenderTargetView,
   UINT RTargets, UINT ClearTargets, D3D10DDI_HDEPTHSTENCILVIEW hDepthStencilView);

SIZE_T APIENTRY CalcPrivateDepthStencilStateSize(
   D3D10DDI_HDEVICE hDevice,
   __in const D3D10_DDI_DEPTH_STENCIL_DESC *pDepthStencilDesc);

void APIENTRY CreateDepthStencilState(
   D3D10DDI_HDEVICE hDevice,
   __in const D3D10_DDI_DEPTH_STENCIL_DESC *pDepthStencilDesc,
   D3D10DDI_HDEPTHSTENCILSTATE hDepthStencilState,
   D3D10DDI_HRTDEPTHSTENCILSTATE hRTDepthStencilState);

void APIENTRY DestroyDepthStencilState(D3D10DDI_HDEVICE hDevice,
                              D3D10DDI_HDEPTHSTENCILSTATE hDepthStencilState);

void APIENTRY SetDepthStencilState(D3D10DDI_HDEVICE hDevice,
                          D3D10DDI_HDEPTHSTENCILSTATE hState, UINT StencilRef);

#endif   /* OUTPUT_MERGER_H */
