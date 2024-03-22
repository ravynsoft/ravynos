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
 * InputAssembly.h --
 *    Functions that manipulate the input assembly stage.
 */

#ifndef INPUT_ASSEMBLY_H
#define INPUT_ASSEMBLY_H

#include "DriverIncludes.h"

void APIENTRY IaSetTopology(D3D10DDI_HDEVICE hDevice,
                   D3D10_DDI_PRIMITIVE_TOPOLOGY PrimitiveTopology);

void APIENTRY IaSetVertexBuffers(D3D10DDI_HDEVICE hDevice, UINT StartBuffer, UINT NumBuffers,
                        __in_ecount (NumBuffers) const D3D10DDI_HRESOURCE *phBuffers,
                        __in_ecount (NumBuffers) const UINT *pStrides,
                        __in_ecount (NumBuffers) const UINT *pOffsets);

void APIENTRY IaSetIndexBuffer(D3D10DDI_HDEVICE hDevice, D3D10DDI_HRESOURCE hBuffer,
                      DXGI_FORMAT Format, UINT Offset);
void APIENTRY IaSetInputLayout(D3D10DDI_HDEVICE hDevice, D3D10DDI_HELEMENTLAYOUT hInputLayout);

SIZE_T APIENTRY CalcPrivateElementLayoutSize(
   D3D10DDI_HDEVICE hDevice,
   __in const D3D10DDIARG_CREATEELEMENTLAYOUT *pCreateElementLayout);

void APIENTRY CreateElementLayout(
   D3D10DDI_HDEVICE hDevice,
   __in const D3D10DDIARG_CREATEELEMENTLAYOUT *pCreateElementLayout,
   D3D10DDI_HELEMENTLAYOUT hElementLayout,
   D3D10DDI_HRTELEMENTLAYOUT hRTElementLayout);

void APIENTRY DestroyElementLayout(D3D10DDI_HDEVICE hDevice,
                         D3D10DDI_HELEMENTLAYOUT hElementLayout);

#endif   /* INPUT_ASSEMBLY_H */
