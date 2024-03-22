/**************************************************************************
 *
 * Copyright 2010 Younes Manton & Thomas Balling Sørensen.
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

#include <assert.h>

#include "util/u_memory.h"

#include "vdpau_private.h"

static void* ftab[67] =
{
   &vlVdpGetErrorString, /* VDP_FUNC_ID_GET_ERROR_STRING */
   &vlVdpGetProcAddress, /* VDP_FUNC_ID_GET_PROC_ADDRESS */
   &vlVdpGetApiVersion, /* VDP_FUNC_ID_GET_API_VERSION */
   NULL, /* DUMMY */
   &vlVdpGetInformationString, /* VDP_FUNC_ID_GET_INFORMATION_STRING */
   &vlVdpDeviceDestroy, /* VDP_FUNC_ID_DEVICE_DESTROY */
   &vlVdpGenerateCSCMatrix, /* VDP_FUNC_ID_GENERATE_CSC_MATRIX */
   &vlVdpVideoSurfaceQueryCapabilities, /* VDP_FUNC_ID_VIDEO_SURFACE_QUERY_CAPABILITIES */
   &vlVdpVideoSurfaceQueryGetPutBitsYCbCrCapabilities, /* VDP_FUNC_ID_VIDEO_SURFACE_QUERY_GET_PUT_BITS_Y_CB_CR_CAPABILITIES */
   &vlVdpVideoSurfaceCreate, /* VDP_FUNC_ID_VIDEO_SURFACE_CREATE */
   &vlVdpVideoSurfaceDestroy, /* VDP_FUNC_ID_VIDEO_SURFACE_DESTROY */
   &vlVdpVideoSurfaceGetParameters, /* VDP_FUNC_ID_VIDEO_SURFACE_GET_PARAMETERS */
   &vlVdpVideoSurfaceGetBitsYCbCr, /* VDP_FUNC_ID_VIDEO_SURFACE_GET_BITS_Y_CB_CR */
   &vlVdpVideoSurfacePutBitsYCbCr, /* VDP_FUNC_ID_VIDEO_SURFACE_PUT_BITS_Y_CB_CR */
   &vlVdpOutputSurfaceQueryCapabilities, /* VDP_FUNC_ID_OUTPUT_SURFACE_QUERY_CAPABILITIES */
   &vlVdpOutputSurfaceQueryGetPutBitsNativeCapabilities, /* VDP_FUNC_ID_OUTPUT_SURFACE_QUERY_GET_PUT_BITS_NATIVE_CAPABILITIES */
   &vlVdpOutputSurfaceQueryPutBitsIndexedCapabilities, /* VDP_FUNC_ID_OUTPUT_SURFACE_QUERY_PUT_BITS_INDEXED_CAPABILITIES */
   &vlVdpOutputSurfaceQueryPutBitsYCbCrCapabilities, /* VDP_FUNC_ID_OUTPUT_SURFACE_QUERY_PUT_BITS_Y_CB_CR_CAPABILITIES */
   &vlVdpOutputSurfaceCreate, /* VDP_FUNC_ID_OUTPUT_SURFACE_CREATE */
   &vlVdpOutputSurfaceDestroy, /* VDP_FUNC_ID_OUTPUT_SURFACE_DESTROY */
   &vlVdpOutputSurfaceGetParameters, /* VDP_FUNC_ID_OUTPUT_SURFACE_GET_PARAMETERS */
   &vlVdpOutputSurfaceGetBitsNative, /* VDP_FUNC_ID_OUTPUT_SURFACE_GET_BITS_NATIVE */
   &vlVdpOutputSurfacePutBitsNative, /* VDP_FUNC_ID_OUTPUT_SURFACE_PUT_BITS_NATIVE */
   &vlVdpOutputSurfacePutBitsIndexed, /* VDP_FUNC_ID_OUTPUT_SURFACE_PUT_BITS_INDEXED */
   &vlVdpOutputSurfacePutBitsYCbCr, /* VDP_FUNC_ID_OUTPUT_SURFACE_PUT_BITS_Y_CB_CR */
   &vlVdpBitmapSurfaceQueryCapabilities, /* VDP_FUNC_ID_BITMAP_SURFACE_QUERY_CAPABILITIES */
   &vlVdpBitmapSurfaceCreate, /* VDP_FUNC_ID_BITMAP_SURFACE_CREATE */
   &vlVdpBitmapSurfaceDestroy, /* VDP_FUNC_ID_BITMAP_SURFACE_DESTROY */
   &vlVdpBitmapSurfaceGetParameters, /* VDP_FUNC_ID_BITMAP_SURFACE_GET_PARAMETERS */
   &vlVdpBitmapSurfacePutBitsNative, /* VDP_FUNC_ID_BITMAP_SURFACE_PUT_BITS_NATIVE */
   NULL, /* DUMMY */
   NULL, /* DUMMY */
   NULL, /* DUMMY */
   &vlVdpOutputSurfaceRenderOutputSurface, /* VDP_FUNC_ID_OUTPUT_SURFACE_RENDER_OUTPUT_SURFACE */
   &vlVdpOutputSurfaceRenderBitmapSurface, /* VDP_FUNC_ID_OUTPUT_SURFACE_RENDER_BITMAP_SURFACE */
   NULL, /* VDP_FUNC_ID_OUTPUT_SURFACE_RENDER_VIDEO_SURFACE_LUMA */
   &vlVdpDecoderQueryCapabilities, /* VDP_FUNC_ID_DECODER_QUERY_CAPABILITIES */
   &vlVdpDecoderCreate, /* VDP_FUNC_ID_DECODER_CREATE */
   &vlVdpDecoderDestroy, /* VDP_FUNC_ID_DECODER_DESTROY */
   &vlVdpDecoderGetParameters, /* VDP_FUNC_ID_DECODER_GET_PARAMETERS */
   &vlVdpDecoderRender, /* VDP_FUNC_ID_DECODER_RENDER */
   &vlVdpVideoMixerQueryFeatureSupport, /* VDP_FUNC_ID_VIDEO_MIXER_QUERY_FEATURE_SUPPORT */
   &vlVdpVideoMixerQueryParameterSupport, /* VDP_FUNC_ID_VIDEO_MIXER_QUERY_PARAMETER_SUPPORT */
   &vlVdpVideoMixerQueryAttributeSupport, /* VDP_FUNC_ID_VIDEO_MIXER_QUERY_ATTRIBUTE_SUPPORT */
   &vlVdpVideoMixerQueryParameterValueRange, /* VDP_FUNC_ID_VIDEO_MIXER_QUERY_PARAMETER_VALUE_RANGE */
   &vlVdpVideoMixerQueryAttributeValueRange, /* VDP_FUNC_ID_VIDEO_MIXER_QUERY_ATTRIBUTE_VALUE_RANGE */
   &vlVdpVideoMixerCreate, /* VDP_FUNC_ID_VIDEO_MIXER_CREATE */
   &vlVdpVideoMixerSetFeatureEnables, /* VDP_FUNC_ID_VIDEO_MIXER_SET_FEATURE_ENABLES */
   &vlVdpVideoMixerSetAttributeValues, /* VDP_FUNC_ID_VIDEO_MIXER_SET_ATTRIBUTE_VALUES */
   &vlVdpVideoMixerGetFeatureSupport, /* VDP_FUNC_ID_VIDEO_MIXER_GET_FEATURE_SUPPORT */
   &vlVdpVideoMixerGetFeatureEnables, /* VDP_FUNC_ID_VIDEO_MIXER_GET_FEATURE_ENABLES */
   &vlVdpVideoMixerGetParameterValues, /* VDP_FUNC_ID_VIDEO_MIXER_GET_PARAMETER_VALUES */
   &vlVdpVideoMixerGetAttributeValues, /* VDP_FUNC_ID_VIDEO_MIXER_GET_ATTRIBUTE_VALUES */
   &vlVdpVideoMixerDestroy, /* VDP_FUNC_ID_VIDEO_MIXER_DESTROY */
   &vlVdpVideoMixerRender, /* VDP_FUNC_ID_VIDEO_MIXER_RENDER */
   &vlVdpPresentationQueueTargetDestroy, /* VDP_FUNC_ID_PRESENTATION_QUEUE_TARGET_DESTROY */
   &vlVdpPresentationQueueCreate, /* VDP_FUNC_ID_PRESENTATION_QUEUE_CREATE */
   &vlVdpPresentationQueueDestroy, /* VDP_FUNC_ID_PRESENTATION_QUEUE_DESTROY */
   &vlVdpPresentationQueueSetBackgroundColor, /* VDP_FUNC_ID_PRESENTATION_QUEUE_SET_BACKGROUND_COLOR */
   &vlVdpPresentationQueueGetBackgroundColor, /* VDP_FUNC_ID_PRESENTATION_QUEUE_GET_BACKGROUND_COLOR */
   NULL, /* DUMMY */
   NULL, /* DUMMY */
   &vlVdpPresentationQueueGetTime, /* VDP_FUNC_ID_PRESENTATION_QUEUE_GET_TIME */
   &vlVdpPresentationQueueDisplay, /* VDP_FUNC_ID_PRESENTATION_QUEUE_DISPLAY */
   &vlVdpPresentationQueueBlockUntilSurfaceIdle, /* VDP_FUNC_ID_PRESENTATION_QUEUE_BLOCK_UNTIL_SURFACE_IDLE */
   &vlVdpPresentationQueueQuerySurfaceStatus, /* VDP_FUNC_ID_PRESENTATION_QUEUE_QUERY_SURFACE_STATUS */
   &vlVdpPreemptionCallbackRegister  /* VDP_FUNC_ID_PREEMPTION_CALLBACK_REGISTER */
};

static void* ftab_winsys[1] =
{
   &vlVdpPresentationQueueTargetCreateX11  /* VDP_FUNC_ID_PRESENTATION_QUEUE_TARGET_CREATE_X11 */
};

static void* ftab_driver[4] =
{
   &vlVdpVideoSurfaceGallium, /* VDP_FUNC_ID_SURFACE_GALLIUM */
   &vlVdpOutputSurfaceGallium, /* VDP_FUNC_ID_OUTPUT_SURFACE_GALLIUM */
   &vlVdpVideoSurfaceDMABuf, /* VDP_FUNC_ID_VIDEO_SURFACE_DMA_BUF */
   &vlVdpOutputSurfaceDMABuf /* VDP_FUNC_ID_OUTPUT_SURFACE_DMA_BUF */
};

bool vlGetFuncFTAB(VdpFuncId function_id, void **func)
{
   assert(func);
   *func = NULL;

   if (function_id < VDP_FUNC_ID_BASE_WINSYS) {
      if (function_id < ARRAY_SIZE(ftab))
         *func = ftab[function_id];

   } else if (function_id < VDP_FUNC_ID_BASE_DRIVER) {
      function_id -= VDP_FUNC_ID_BASE_WINSYS;
      if (function_id < ARRAY_SIZE(ftab_winsys))
         *func = ftab_winsys[function_id];

   } else {
      function_id -= VDP_FUNC_ID_BASE_DRIVER;
      if (function_id < ARRAY_SIZE(ftab_driver))
         *func = ftab_driver[function_id];
   }

   return *func != NULL;
}
