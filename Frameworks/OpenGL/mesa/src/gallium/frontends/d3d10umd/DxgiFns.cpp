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
 * DxgiFns.cpp --
 *    DXGI related functions.
 */

#include <stdio.h>

#include "DxgiFns.h"
#include "Format.h"
#include "State.h"

#include "Debug.h"

#include "util/format/u_format.h"


/*
 * ----------------------------------------------------------------------
 *
 * _Present --
 *
 *    This is turned into kernel callbacks rather than directly emitted
 *    as fifo packets.
 *
 * ----------------------------------------------------------------------
 */

HRESULT APIENTRY
_Present(DXGI_DDI_ARG_PRESENT *pPresentData)
{

   LOG_ENTRYPOINT();

   struct pipe_context *pipe = CastPipeDevice(pPresentData->hDevice);
   Resource *pSrcResource = CastResource(pPresentData->hSurfaceToPresent);

   D3DKMT_PRESENT *pPresentInfo = (D3DKMT_PRESENT *)pPresentData->pDXGIContext;

   HWND hWnd = pPresentInfo->hWindow;

   if (0) {
      DebugPrintf("  hWindow = 0x%08lx\n", pPresentInfo->hWindow);
      if (pPresentInfo->Flags.SrcRectValid) {
         DebugPrintf("  SrcRect.left = %li\n", pPresentInfo->SrcRect.left);
         DebugPrintf("  SrcRect.top = %li\n", pPresentInfo->SrcRect.top);
         DebugPrintf("  SrcRect.right = %li\n", pPresentInfo->SrcRect.right);
         DebugPrintf("  SrcRect.bottom = %li\n", pPresentInfo->SrcRect.bottom);
      }
      if (pPresentInfo->Flags.DstRectValid) {
         DebugPrintf("  DstRect.left = %li\n", pPresentInfo->DstRect.left);
         DebugPrintf("  DstRect.top = %li\n", pPresentInfo->DstRect.top);
         DebugPrintf("  DstRect.right = %li\n", pPresentInfo->DstRect.right);
         DebugPrintf("  DstRect.bottom = %li\n", pPresentInfo->DstRect.bottom);
      }
   }

   RECT rect;
   if (!GetClientRect(hWnd, &rect)) {
      DebugPrintf("Invalid window.\n");
      return S_OK;
   }

   int windowWidth  = rect.right  - rect.left;
   int windowHeight = rect.bottom - rect.top;

   HDC hDC = GetDC(hWnd);

   unsigned w = pSrcResource->resource->width0;
   unsigned h = pSrcResource->resource->height0;

   void *map;
   struct pipe_transfer *transfer;
   map = pipe_texture_map(pipe,
                          pSrcResource->resource,
                          0, 0, PIPE_MAP_READ,
                          0, 0, w, h,
                          &transfer);
   if (map) {

      BITMAPINFO bmi;

      memset(&bmi, 0, sizeof bmi);
      bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
      bmi.bmiHeader.biWidth = w;
      bmi.bmiHeader.biHeight= -(long)h;
      bmi.bmiHeader.biPlanes = 1;
      bmi.bmiHeader.biBitCount = 32;
      bmi.bmiHeader.biCompression = BI_RGB;
      bmi.bmiHeader.biSizeImage = 0;
      bmi.bmiHeader.biXPelsPerMeter = 0;
      bmi.bmiHeader.biYPelsPerMeter = 0;
      bmi.bmiHeader.biClrUsed = 0;
      bmi.bmiHeader.biClrImportant = 0;

      DWORD *pixels = NULL;

      // http://www.daniweb.com/software-development/cpp/code/241875/fast-animation-with-the-windows-gdi

      HBITMAP hBmp = CreateDIBSection(hDC, &bmi, DIB_RGB_COLORS, (void**)&pixels, NULL, 0);

      util_format_translate(
            PIPE_FORMAT_B8G8R8X8_UNORM,
            (void *)pixels, w * 4,
            0, 0,
            pSrcResource->resource->format,
            map, transfer->stride,
            0, 0, w, h);

      if (0) {
         /*
          * Save a BMP for debugging.
          */

         FILE *fp = fopen("present.bmp", "wb");
         if (fp) {
            BITMAPFILEHEADER bmf;
            bmf.bfType = 0x4d42;
            bmf.bfSize = sizeof bmf + sizeof bmi + h * w * 4;
            bmf.bfReserved1 = 0;
            bmf.bfReserved2 = 0;
            bmf.bfOffBits = sizeof bmf + sizeof bmi;

            fwrite(&bmf, sizeof bmf, 1, fp);
            fwrite(&bmi, sizeof bmi, 1, fp);
            fwrite(pixels, h, w * 4, fp);
            fclose(fp);
         }
      }

      HDC hdcMem;
      hdcMem = CreateCompatibleDC(hDC);
      HBITMAP hbmOld = (HBITMAP)SelectObject(hdcMem, hBmp);

      int iStretchMode = SetStretchBltMode(hDC, HALFTONE);

      StretchBlt(hDC, 0, 0, windowWidth, windowHeight,
                 hdcMem, 0, 0, w, h,
                 SRCCOPY);

      if (iStretchMode) {
         SetStretchBltMode(hDC, iStretchMode);
      }

      SelectObject(hdcMem, hbmOld);
      DeleteDC(hdcMem);
      DeleteObject(hBmp);

      pipe_texture_unmap(pipe, transfer);
   }

   ReleaseDC(hWnd, hDC);

   return S_OK;
}


/*
 * ----------------------------------------------------------------------
 *
 * _GetGammaCaps --
 *
 *    Return gamma capabilities.
 *
 * ----------------------------------------------------------------------
 */

HRESULT APIENTRY
_GetGammaCaps( DXGI_DDI_ARG_GET_GAMMA_CONTROL_CAPS *GetCaps )
{
   LOG_ENTRYPOINT();

   DXGI_GAMMA_CONTROL_CAPABILITIES *pCaps;

   pCaps = GetCaps->pGammaCapabilities;

   pCaps->ScaleAndOffsetSupported = false;
   pCaps->MinConvertedValue = 0.0;
   pCaps->MaxConvertedValue = 1.0;
   pCaps->NumGammaControlPoints = 17;

   for (UINT i = 0; i < pCaps->NumGammaControlPoints; i++) {
      pCaps->ControlPointPositions[i] = (float)i / (float)(pCaps->NumGammaControlPoints - 1);
   }

   return S_OK;
}


/*
 * ----------------------------------------------------------------------
 *
 * _SetDisplayMode --
 *
 *    Set the resource that is used to scan out to the display.
 *
 * ----------------------------------------------------------------------
 */

HRESULT APIENTRY
_SetDisplayMode( DXGI_DDI_ARG_SETDISPLAYMODE *SetDisplayMode )
{
   LOG_UNSUPPORTED_ENTRYPOINT();

   return S_OK;
}


/*
 * ----------------------------------------------------------------------
 *
 * _SetResourcePriority --
 *
 * ----------------------------------------------------------------------
 */

HRESULT APIENTRY
_SetResourcePriority( DXGI_DDI_ARG_SETRESOURCEPRIORITY *SetResourcePriority )
{
   LOG_ENTRYPOINT();

   /* ignore */

   return S_OK;
}


/*
 * ----------------------------------------------------------------------
 *
 * _QueryResourceResidency --
 *
 * ----------------------------------------------------------------------
 */

HRESULT APIENTRY
_QueryResourceResidency( DXGI_DDI_ARG_QUERYRESOURCERESIDENCY *QueryResourceResidency )
{
   LOG_ENTRYPOINT();

   for (UINT i = 0; i < QueryResourceResidency->Resources; ++i) {
      QueryResourceResidency->pStatus[i] = DXGI_DDI_RESIDENCY_FULLY_RESIDENT;
   }

   return S_OK;
}


/*
 * ----------------------------------------------------------------------
 *
 * _RotateResourceIdentities --
 *
 *    Rotate a list of resources by recreating their views with
 *    the updated rotations.
 *
 * ----------------------------------------------------------------------
 */

HRESULT APIENTRY
_RotateResourceIdentities( DXGI_DDI_ARG_ROTATE_RESOURCE_IDENTITIES *RotateResourceIdentities )
{
   LOG_ENTRYPOINT();

   if (RotateResourceIdentities->Resources <= 1) {
      return S_OK;
   }

   struct pipe_context *pipe = CastPipeDevice(RotateResourceIdentities->hDevice);
   struct pipe_screen *screen = pipe->screen;

   struct pipe_resource *resource0 = CastPipeResource(RotateResourceIdentities->pResources[0]);

   assert(resource0);
   LOG_UNSUPPORTED(resource0->last_level);

   /*
    * XXX: Copying is not very efficient, but it is much simpler than the
    * alternative of recreating all views.
    */

   struct pipe_resource *temp_resource;
   temp_resource = screen->resource_create(screen, resource0);
   assert(temp_resource);
   if (!temp_resource) {
      return E_OUTOFMEMORY;
   }

   struct pipe_box src_box;
   src_box.x = 0;
   src_box.y = 0;
   src_box.z = 0;
   src_box.width  = resource0->width0;
   src_box.height = resource0->height0;
   src_box.depth  = resource0->depth0;

   for (UINT i = 0; i < RotateResourceIdentities->Resources + 1; ++i) {
      struct pipe_resource *src_resource;
      struct pipe_resource *dst_resource;

      if (i < RotateResourceIdentities->Resources) {
         src_resource = CastPipeResource(RotateResourceIdentities->pResources[i]);
      } else {
         src_resource = temp_resource;
      }

      if (i > 0) {
         dst_resource = CastPipeResource(RotateResourceIdentities->pResources[i - 1]);
      } else {
         dst_resource = temp_resource;
      }

      assert(dst_resource);
      assert(src_resource);

      pipe->resource_copy_region(pipe,
                                 dst_resource,
                                 0, // dst_level
                                 0, 0, 0, // dst_x,y,z
                                 src_resource,
                                 0, // src_level
                                 &src_box);
   }

   pipe_resource_reference(&temp_resource, NULL);

   return S_OK;
}


/*
 * ----------------------------------------------------------------------
 *
 * _Blt --
 *
 *    Do a blt between two subresources. Apply MSAA resolve, format
 *    conversion and stretching.
 *
 * ----------------------------------------------------------------------
 */

HRESULT APIENTRY
_Blt(DXGI_DDI_ARG_BLT *Blt)
{
   LOG_UNSUPPORTED_ENTRYPOINT();

   return S_OK;
}
