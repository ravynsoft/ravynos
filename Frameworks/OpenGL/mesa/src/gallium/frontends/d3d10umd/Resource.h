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
 * Resource.h --
 *    Functions that manipulate GPU resources.
 */

#ifndef RESOURCE_H
#define RESOURCE_H

#include "DriverIncludes.h"

SIZE_T APIENTRY CalcPrivateResourceSize(
   D3D10DDI_HDEVICE hDevice,
   __in const D3D10DDIARG_CREATERESOURCE *pCreateResource);

void APIENTRY CreateResource(
   D3D10DDI_HDEVICE hDevice,
   __in const D3D10DDIARG_CREATERESOURCE *pCreateResource,
   D3D10DDI_HRESOURCE hResource,
   D3D10DDI_HRTRESOURCE hRTResource);

SIZE_T APIENTRY CalcPrivateOpenedResourceSize(
   D3D10DDI_HDEVICE hDevice,
   __in const D3D10DDIARG_OPENRESOURCE *pOpenResource);

void APIENTRY OpenResource(
   D3D10DDI_HDEVICE hDevice,
   __in const D3D10DDIARG_OPENRESOURCE *pOpenResource,
   D3D10DDI_HRESOURCE hResource,
   D3D10DDI_HRTRESOURCE hRTResource);

void APIENTRY DestroyResource(D3D10DDI_HDEVICE hDevice, D3D10DDI_HRESOURCE hResource);

void APIENTRY ResourceMap(D3D10DDI_HDEVICE hDevice, D3D10DDI_HRESOURCE hResource,
                 UINT SubResource, D3D10_DDI_MAP DDIMap, UINT Flags,
                 __out D3D10DDI_MAPPED_SUBRESOURCE *pMappedSubResource);

void APIENTRY ResourceUnmap(D3D10DDI_HDEVICE hDevice, D3D10DDI_HRESOURCE hResource,
                   UINT SubResource);

void APIENTRY ResourceCopy(D3D10DDI_HDEVICE hDevice, D3D10DDI_HRESOURCE hDstResource,
                  D3D10DDI_HRESOURCE hSrcResource);

void APIENTRY ResourceCopyRegion(D3D10DDI_HDEVICE hDevice, D3D10DDI_HRESOURCE hResource,
                        UINT DstSubResource, UINT DstX, UINT DstY, UINT DstZ,
                        D3D10DDI_HRESOURCE hSrcResource, UINT SrcSubResource,
                        __in_opt const D3D10_DDI_BOX *pSrcBox);

void APIENTRY ResourceResolveSubResource(D3D10DDI_HDEVICE hDevice,
                                D3D10DDI_HRESOURCE hDstResource,
                                UINT DstSubResource,
                                D3D10DDI_HRESOURCE hSrcResource,
                                UINT SrcSubResource,
                                DXGI_FORMAT ResolveFormat);

BOOL APIENTRY ResourceIsStagingBusy(D3D10DDI_HDEVICE hDevice,
                           D3D10DDI_HRESOURCE hResource);

void APIENTRY ResourceReadAfterWriteHazard(D3D10DDI_HDEVICE hDevice,
                                  D3D10DDI_HRESOURCE hResource);

void APIENTRY ResourceUpdateSubResourceUP(
   D3D10DDI_HDEVICE hDevice, D3D10DDI_HRESOURCE hResource,
   UINT DstSubResource, __in_opt const D3D10_DDI_BOX *pDstBox,
   __in const void *pSysMemUP, UINT RowPitch, UINT DepthPitch);

#endif   /* RESOURCE_H */
