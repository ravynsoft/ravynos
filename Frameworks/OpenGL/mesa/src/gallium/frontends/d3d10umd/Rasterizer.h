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
 * Rasterizer.h --
 *    Functions that manipulate rasterizer state.
 */

#ifndef RASTERIZER_H
#define RASTERIZER_H

#include "DriverIncludes.h"

void APIENTRY SetViewports(
   D3D10DDI_HDEVICE hDevice, UINT NumViewports, UINT ClearViewports,
   __in_ecount (NumViewports) const D3D10_DDI_VIEWPORT *pViewports);

void APIENTRY SetScissorRects(
   D3D10DDI_HDEVICE hDevice, UINT NumScissorRects, UINT ClearScissorRects,
   __in_ecount (NumRects) const D3D10_DDI_RECT *pRects);

SIZE_T APIENTRY CalcPrivateRasterizerStateSize(
   D3D10DDI_HDEVICE hDevice,
   __in const D3D10_DDI_RASTERIZER_DESC *pRasterizerDesc);

void APIENTRY CreateRasterizerState(
   D3D10DDI_HDEVICE hDevice,
   __in const D3D10_DDI_RASTERIZER_DESC *pRasterizerDesc,
   D3D10DDI_HRASTERIZERSTATE hRasterizerState,
   D3D10DDI_HRTRASTERIZERSTATE hRTRasterizerState);

void APIENTRY DestroyRasterizerState(D3D10DDI_HDEVICE hDevice,
                            D3D10DDI_HRASTERIZERSTATE hRasterizerState);

void APIENTRY SetRasterizerState(D3D10DDI_HDEVICE hDevice,
                        D3D10DDI_HRASTERIZERSTATE hRasterizerState);

#endif   /* RASTERIZER_H */
