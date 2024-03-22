/**************************************************************************
 *
 * Copyright 2009-2010 Chia-I Wu <olvaffe@gmail.com>
 * Copyright 2010-2011 LunarG, Inc.
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
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 **************************************************************************/

#include <assert.h>
#include <string.h>

#include "eglcurrent.h"
#include "eglimage.h"
#include "egllog.h"

static EGLint
_eglParseKHRImageAttribs(_EGLImageAttribs *attrs, _EGLDisplay *disp,
                         EGLint attr, EGLint val)
{
   switch (attr) {
   case EGL_IMAGE_PRESERVED_KHR:
      if (!disp->Extensions.KHR_image_base)
         return EGL_BAD_PARAMETER;

      attrs->ImagePreserved = val;
      break;

   case EGL_GL_TEXTURE_LEVEL_KHR:
      if (!disp->Extensions.KHR_gl_texture_2D_image)
         return EGL_BAD_PARAMETER;

      attrs->GLTextureLevel = val;
      break;
   case EGL_GL_TEXTURE_ZOFFSET_KHR:
      if (!disp->Extensions.KHR_gl_texture_3D_image)
         return EGL_BAD_PARAMETER;

      attrs->GLTextureZOffset = val;
      break;
   case EGL_PROTECTED_CONTENT_EXT:
      if (!disp->Extensions.EXT_protected_content &&
          !disp->Extensions.EXT_protected_surface)
         return EGL_BAD_PARAMETER;

      attrs->ProtectedContent = val;
      break;
   default:
      return EGL_BAD_PARAMETER;
   }

   return EGL_SUCCESS;
}

static EGLint
_eglParseMESADrmImageAttribs(_EGLImageAttribs *attrs, _EGLDisplay *disp,
                             EGLint attr, EGLint val)
{
   if (!disp->Extensions.MESA_drm_image)
      return EGL_BAD_PARAMETER;

   switch (attr) {
   case EGL_WIDTH:
      attrs->Width = val;
      break;
   case EGL_HEIGHT:
      attrs->Height = val;
      break;
   case EGL_DRM_BUFFER_FORMAT_MESA:
      attrs->DRMBufferFormatMESA = val;
      break;
   case EGL_DRM_BUFFER_USE_MESA:
      attrs->DRMBufferUseMESA = val;
      break;
   case EGL_DRM_BUFFER_STRIDE_MESA:
      attrs->DRMBufferStrideMESA = val;
      break;
   default:
      return EGL_BAD_PARAMETER;
   }

   return EGL_SUCCESS;
}

static EGLint
_eglParseWLBindWaylandDisplayAttribs(_EGLImageAttribs *attrs, _EGLDisplay *disp,
                                     EGLint attr, EGLint val)
{
   if (!disp->Extensions.WL_bind_wayland_display)
      return EGL_BAD_PARAMETER;

   switch (attr) {
   case EGL_WAYLAND_PLANE_WL:
      attrs->PlaneWL = val;
      break;
   default:
      return EGL_BAD_PARAMETER;
   }

   return EGL_SUCCESS;
}

static EGLint
_eglParseEXTImageDmaBufImportAttribs(_EGLImageAttribs *attrs, _EGLDisplay *disp,
                                     EGLint attr, EGLint val)
{
   if (!disp->Extensions.EXT_image_dma_buf_import)
      return EGL_BAD_PARAMETER;

   switch (attr) {
   case EGL_WIDTH:
      attrs->Width = val;
      break;
   case EGL_HEIGHT:
      attrs->Height = val;
      break;
   case EGL_LINUX_DRM_FOURCC_EXT:
      attrs->DMABufFourCC.Value = val;
      attrs->DMABufFourCC.IsPresent = EGL_TRUE;
      break;
   case EGL_DMA_BUF_PLANE0_FD_EXT:
      attrs->DMABufPlaneFds[0].Value = val;
      attrs->DMABufPlaneFds[0].IsPresent = EGL_TRUE;
      break;
   case EGL_DMA_BUF_PLANE0_OFFSET_EXT:
      attrs->DMABufPlaneOffsets[0].Value = val;
      attrs->DMABufPlaneOffsets[0].IsPresent = EGL_TRUE;
      break;
   case EGL_DMA_BUF_PLANE0_PITCH_EXT:
      attrs->DMABufPlanePitches[0].Value = val;
      attrs->DMABufPlanePitches[0].IsPresent = EGL_TRUE;
      break;
   case EGL_DMA_BUF_PLANE1_FD_EXT:
      attrs->DMABufPlaneFds[1].Value = val;
      attrs->DMABufPlaneFds[1].IsPresent = EGL_TRUE;
      break;
   case EGL_DMA_BUF_PLANE1_OFFSET_EXT:
      attrs->DMABufPlaneOffsets[1].Value = val;
      attrs->DMABufPlaneOffsets[1].IsPresent = EGL_TRUE;
      break;
   case EGL_DMA_BUF_PLANE1_PITCH_EXT:
      attrs->DMABufPlanePitches[1].Value = val;
      attrs->DMABufPlanePitches[1].IsPresent = EGL_TRUE;
      break;
   case EGL_DMA_BUF_PLANE2_FD_EXT:
      attrs->DMABufPlaneFds[2].Value = val;
      attrs->DMABufPlaneFds[2].IsPresent = EGL_TRUE;
      break;
   case EGL_DMA_BUF_PLANE2_OFFSET_EXT:
      attrs->DMABufPlaneOffsets[2].Value = val;
      attrs->DMABufPlaneOffsets[2].IsPresent = EGL_TRUE;
      break;
   case EGL_DMA_BUF_PLANE2_PITCH_EXT:
      attrs->DMABufPlanePitches[2].Value = val;
      attrs->DMABufPlanePitches[2].IsPresent = EGL_TRUE;
      break;
   case EGL_YUV_COLOR_SPACE_HINT_EXT:
      if (val != EGL_ITU_REC601_EXT && val != EGL_ITU_REC709_EXT &&
          val != EGL_ITU_REC2020_EXT)
         return EGL_BAD_ATTRIBUTE;

      attrs->DMABufYuvColorSpaceHint.Value = val;
      attrs->DMABufYuvColorSpaceHint.IsPresent = EGL_TRUE;
      break;
   case EGL_SAMPLE_RANGE_HINT_EXT:
      if (val != EGL_YUV_FULL_RANGE_EXT && val != EGL_YUV_NARROW_RANGE_EXT)
         return EGL_BAD_ATTRIBUTE;

      attrs->DMABufSampleRangeHint.Value = val;
      attrs->DMABufSampleRangeHint.IsPresent = EGL_TRUE;
      break;
   case EGL_YUV_CHROMA_HORIZONTAL_SITING_HINT_EXT:
      if (val != EGL_YUV_CHROMA_SITING_0_EXT &&
          val != EGL_YUV_CHROMA_SITING_0_5_EXT)
         return EGL_BAD_ATTRIBUTE;

      attrs->DMABufChromaHorizontalSiting.Value = val;
      attrs->DMABufChromaHorizontalSiting.IsPresent = EGL_TRUE;
      break;
   case EGL_YUV_CHROMA_VERTICAL_SITING_HINT_EXT:
      if (val != EGL_YUV_CHROMA_SITING_0_EXT &&
          val != EGL_YUV_CHROMA_SITING_0_5_EXT)
         return EGL_BAD_ATTRIBUTE;

      attrs->DMABufChromaVerticalSiting.Value = val;
      attrs->DMABufChromaVerticalSiting.IsPresent = EGL_TRUE;
      break;
   default:
      return EGL_BAD_PARAMETER;
   }

   return EGL_SUCCESS;
}

static EGLint
_eglParseEXTImageDmaBufImportModifiersAttribs(_EGLImageAttribs *attrs,
                                              _EGLDisplay *disp, EGLint attr,
                                              EGLint val)
{
   if (!disp->Extensions.EXT_image_dma_buf_import_modifiers)
      return EGL_BAD_PARAMETER;

   switch (attr) {
   case EGL_DMA_BUF_PLANE0_MODIFIER_LO_EXT:
      attrs->DMABufPlaneModifiersLo[0].Value = val;
      attrs->DMABufPlaneModifiersLo[0].IsPresent = EGL_TRUE;
      break;
   case EGL_DMA_BUF_PLANE0_MODIFIER_HI_EXT:
      attrs->DMABufPlaneModifiersHi[0].Value = val;
      attrs->DMABufPlaneModifiersHi[0].IsPresent = EGL_TRUE;
      break;
   case EGL_DMA_BUF_PLANE1_MODIFIER_LO_EXT:
      attrs->DMABufPlaneModifiersLo[1].Value = val;
      attrs->DMABufPlaneModifiersLo[1].IsPresent = EGL_TRUE;
      break;
   case EGL_DMA_BUF_PLANE1_MODIFIER_HI_EXT:
      attrs->DMABufPlaneModifiersHi[1].Value = val;
      attrs->DMABufPlaneModifiersHi[1].IsPresent = EGL_TRUE;
      break;
   case EGL_DMA_BUF_PLANE2_MODIFIER_LO_EXT:
      attrs->DMABufPlaneModifiersLo[2].Value = val;
      attrs->DMABufPlaneModifiersLo[2].IsPresent = EGL_TRUE;
      break;
   case EGL_DMA_BUF_PLANE2_MODIFIER_HI_EXT:
      attrs->DMABufPlaneModifiersHi[2].Value = val;
      attrs->DMABufPlaneModifiersHi[2].IsPresent = EGL_TRUE;
      break;
   case EGL_DMA_BUF_PLANE3_FD_EXT:
      attrs->DMABufPlaneFds[3].Value = val;
      attrs->DMABufPlaneFds[3].IsPresent = EGL_TRUE;
      break;
   case EGL_DMA_BUF_PLANE3_OFFSET_EXT:
      attrs->DMABufPlaneOffsets[3].Value = val;
      attrs->DMABufPlaneOffsets[3].IsPresent = EGL_TRUE;
      break;
   case EGL_DMA_BUF_PLANE3_PITCH_EXT:
      attrs->DMABufPlanePitches[3].Value = val;
      attrs->DMABufPlanePitches[3].IsPresent = EGL_TRUE;
      break;
   case EGL_DMA_BUF_PLANE3_MODIFIER_LO_EXT:
      attrs->DMABufPlaneModifiersLo[3].Value = val;
      attrs->DMABufPlaneModifiersLo[3].IsPresent = EGL_TRUE;
      break;
   case EGL_DMA_BUF_PLANE3_MODIFIER_HI_EXT:
      attrs->DMABufPlaneModifiersHi[3].Value = val;
      attrs->DMABufPlaneModifiersHi[3].IsPresent = EGL_TRUE;
      break;
   default:
      return EGL_BAD_PARAMETER;
   }

   return EGL_SUCCESS;
}

/**
 * Parse the list of image attributes.
 *
 * Returns EGL_TRUE on success and EGL_FALSE otherwise.
 * Function calls _eglError to set the correct error code.
 */
EGLBoolean
_eglParseImageAttribList(_EGLImageAttribs *attrs, _EGLDisplay *disp,
                         const EGLint *attrib_list)
{
   EGLint i, err;

   memset(attrs, 0, sizeof(*attrs));

   if (!attrib_list)
      return EGL_TRUE;

   for (i = 0; attrib_list[i] != EGL_NONE; i++) {
      EGLint attr = attrib_list[i++];
      EGLint val = attrib_list[i];

      err = _eglParseKHRImageAttribs(attrs, disp, attr, val);
      if (err == EGL_SUCCESS)
         continue;

      err = _eglParseMESADrmImageAttribs(attrs, disp, attr, val);
      if (err == EGL_SUCCESS)
         continue;

      err = _eglParseWLBindWaylandDisplayAttribs(attrs, disp, attr, val);
      if (err == EGL_SUCCESS)
         continue;

      err = _eglParseEXTImageDmaBufImportAttribs(attrs, disp, attr, val);
      if (err == EGL_SUCCESS)
         continue;

      /* EXT_image_dma_buf_import states that if invalid value is provided for
       * its attributes, we should return EGL_BAD_ATTRIBUTE.
       * Bail out ASAP, since follow-up calls can return another EGL_BAD error.
       */
      if (err == EGL_BAD_ATTRIBUTE)
         return _eglError(err, __func__);

      err =
         _eglParseEXTImageDmaBufImportModifiersAttribs(attrs, disp, attr, val);
      if (err == EGL_SUCCESS)
         continue;

      return _eglError(err, __func__);
   }

   return EGL_TRUE;
}
