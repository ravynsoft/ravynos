/**************************************************************************
 *
 * Copyright 2010 Younes Manton og Thomas Balling Sørensen.
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

#include "util/compiler.h"

#include "util/u_memory.h"
#include "util/u_debug.h"
#include "util/format/u_format.h"
#include "util/u_sampler.h"

#include "vdpau_private.h"

/**
 * Create a VdpDevice object for use with X11.
 */
PUBLIC VdpStatus
vdp_imp_device_create_x11(Display *display, int screen, VdpDevice *device,
                          VdpGetProcAddress **get_proc_address)
{
   struct pipe_screen *pscreen;
   struct pipe_resource *res, res_tmpl;
   struct pipe_sampler_view sv_tmpl;
   vlVdpDevice *dev = NULL;
   VdpStatus ret;

   if (!(display && device && get_proc_address))
      return VDP_STATUS_INVALID_POINTER;

   if (!vlCreateHTAB()) {
      ret = VDP_STATUS_RESOURCES;
      goto no_htab;
   }

   dev = CALLOC(1, sizeof(vlVdpDevice));
   if (!dev) {
      ret = VDP_STATUS_RESOURCES;
      goto no_dev;
   }

   pipe_reference_init(&dev->reference, 1);

   dev->vscreen = vl_dri3_screen_create(display, screen);
   if (!dev->vscreen)
      dev->vscreen = vl_dri2_screen_create(display, screen);
   if (!dev->vscreen)
      dev->vscreen = vl_xlib_swrast_screen_create(display, screen);
   if (!dev->vscreen) {
      ret = VDP_STATUS_RESOURCES;
      goto no_vscreen;
   }

   pscreen = dev->vscreen->pscreen;
   dev->context = pipe_create_multimedia_context(pscreen);
   if (!dev->context) {
      ret = VDP_STATUS_RESOURCES;
      goto no_context;
   }

   if (!pscreen->get_param(pscreen, PIPE_CAP_NPOT_TEXTURES)) {
      ret = VDP_STATUS_NO_IMPLEMENTATION;
      goto no_context;
   }

   memset(&res_tmpl, 0, sizeof(res_tmpl));

   res_tmpl.target = PIPE_TEXTURE_2D;
   res_tmpl.format = PIPE_FORMAT_R8G8B8A8_UNORM;
   res_tmpl.width0 = 1;
   res_tmpl.height0 = 1;
   res_tmpl.depth0 = 1;
   res_tmpl.array_size = 1;
   res_tmpl.bind = PIPE_BIND_SAMPLER_VIEW;
   res_tmpl.usage = PIPE_USAGE_DEFAULT;

   if (!CheckSurfaceParams(pscreen, &res_tmpl)) {
      ret = VDP_STATUS_NO_IMPLEMENTATION;
      goto no_resource;
   }

   res = pscreen->resource_create(pscreen, &res_tmpl);
   if (!res) {
      ret = VDP_STATUS_RESOURCES;
      goto no_resource;
   }

   memset(&sv_tmpl, 0, sizeof(sv_tmpl));
   u_sampler_view_default_template(&sv_tmpl, res, res->format);

   sv_tmpl.swizzle_r = PIPE_SWIZZLE_1;
   sv_tmpl.swizzle_g = PIPE_SWIZZLE_1;
   sv_tmpl.swizzle_b = PIPE_SWIZZLE_1;
   sv_tmpl.swizzle_a = PIPE_SWIZZLE_1;

   dev->dummy_sv = dev->context->create_sampler_view(dev->context, res, &sv_tmpl);
   pipe_resource_reference(&res, NULL);
   if (!dev->dummy_sv) {
      ret = VDP_STATUS_RESOURCES;
      goto no_resource;
   }

   *device = vlAddDataHTAB(dev);
   if (*device == 0) {
      ret = VDP_STATUS_ERROR;
      goto no_handle;
   }

   if (!vl_compositor_init(&dev->compositor, dev->context)) {
       ret = VDP_STATUS_ERROR;
       goto no_compositor;
   }

   (void) mtx_init(&dev->mutex, mtx_plain);

   *get_proc_address = &vlVdpGetProcAddress;

   return VDP_STATUS_OK;

no_compositor:
   vlRemoveDataHTAB(*device);
no_handle:
   pipe_sampler_view_reference(&dev->dummy_sv, NULL);
no_resource:
   dev->context->destroy(dev->context);
no_context:
   dev->vscreen->destroy(dev->vscreen);
no_vscreen:
   FREE(dev);
no_dev:
   vlDestroyHTAB();
no_htab:
   return ret;
}

/**
 * Create a VdpPresentationQueueTarget for use with X11.
 */
VdpStatus
vlVdpPresentationQueueTargetCreateX11(VdpDevice device, Drawable drawable,
                                      VdpPresentationQueueTarget *target)
{
   vlVdpPresentationQueueTarget *pqt;
   VdpStatus ret;

   if (!drawable)
      return VDP_STATUS_INVALID_HANDLE;

   vlVdpDevice *dev = vlGetDataHTAB(device);
   if (!dev)
      return VDP_STATUS_INVALID_HANDLE;

   pqt = CALLOC(1, sizeof(vlVdpPresentationQueueTarget));
   if (!pqt)
      return VDP_STATUS_RESOURCES;

   DeviceReference(&pqt->device, dev);
   pqt->drawable = drawable;

   *target = vlAddDataHTAB(pqt);
   if (*target == 0) {
      ret = VDP_STATUS_ERROR;
      goto no_handle;
   }

   return VDP_STATUS_OK;

no_handle:
   FREE(pqt);
   return ret;
}

/**
 * Destroy a VdpPresentationQueueTarget.
 */
VdpStatus
vlVdpPresentationQueueTargetDestroy(VdpPresentationQueueTarget presentation_queue_target)
{
   vlVdpPresentationQueueTarget *pqt;

   pqt = vlGetDataHTAB(presentation_queue_target);
   if (!pqt)
      return VDP_STATUS_INVALID_HANDLE;

   vlRemoveDataHTAB(presentation_queue_target);
   DeviceReference(&pqt->device, NULL);
   FREE(pqt);

   return VDP_STATUS_OK;
}

/**
 * Destroy a VdpDevice.
 */
VdpStatus
vlVdpDeviceDestroy(VdpDevice device)
{
   vlVdpDevice *dev = vlGetDataHTAB(device);
   if (!dev)
      return VDP_STATUS_INVALID_HANDLE;

   vlRemoveDataHTAB(device);
   DeviceReference(&dev, NULL);

   return VDP_STATUS_OK;
}

/**
 * Free a VdpDevice.
 */
void
vlVdpDeviceFree(vlVdpDevice *dev)
{
   mtx_destroy(&dev->mutex);
   vl_compositor_cleanup(&dev->compositor);
   pipe_sampler_view_reference(&dev->dummy_sv, NULL);
   dev->context->destroy(dev->context);
   dev->vscreen->destroy(dev->vscreen);
   FREE(dev);
   vlDestroyHTAB();
}

/**
 * Retrieve a VDPAU function pointer.
 */
VdpStatus
vlVdpGetProcAddress(VdpDevice device, VdpFuncId function_id, void **function_pointer)
{
   vlVdpDevice *dev = vlGetDataHTAB(device);
   if (!dev)
      return VDP_STATUS_INVALID_HANDLE;

   if (!function_pointer)
      return VDP_STATUS_INVALID_POINTER;

   if (!vlGetFuncFTAB(function_id, function_pointer))
      return VDP_STATUS_INVALID_FUNC_ID;

   VDPAU_MSG(VDPAU_TRACE, "[VDPAU] Got proc address %p for id %d\n", *function_pointer, function_id);

   return VDP_STATUS_OK;
}

#define _ERROR_TYPE(TYPE,STRING) case TYPE: return STRING;

/**
 * Retrieve a string describing an error code.
 */
char const *
vlVdpGetErrorString (VdpStatus status)
{
   switch (status) {
   _ERROR_TYPE(VDP_STATUS_OK,"The operation completed successfully; no error.");
   _ERROR_TYPE(VDP_STATUS_NO_IMPLEMENTATION,"No backend implementation could be loaded.");
   _ERROR_TYPE(VDP_STATUS_DISPLAY_PREEMPTED,"The display was preempted, or a fatal error occurred. The application must re-initialize VDPAU.");
   _ERROR_TYPE(VDP_STATUS_INVALID_HANDLE,"An invalid handle value was provided. Either the handle does not exist at all, or refers to an object of an incorrect type.");
   _ERROR_TYPE(VDP_STATUS_INVALID_POINTER,"An invalid pointer was provided. Typically, this means that a NULL pointer was provided for an 'output' parameter.");
   _ERROR_TYPE(VDP_STATUS_INVALID_CHROMA_TYPE,"An invalid/unsupported VdpChromaType value was supplied.");
   _ERROR_TYPE(VDP_STATUS_INVALID_Y_CB_CR_FORMAT,"An invalid/unsupported VdpYCbCrFormat value was supplied.");
   _ERROR_TYPE(VDP_STATUS_INVALID_RGBA_FORMAT,"An invalid/unsupported VdpRGBAFormat value was supplied.");
   _ERROR_TYPE(VDP_STATUS_INVALID_INDEXED_FORMAT,"An invalid/unsupported VdpIndexedFormat value was supplied.");
   _ERROR_TYPE(VDP_STATUS_INVALID_COLOR_STANDARD,"An invalid/unsupported VdpColorStandard value was supplied.");
   _ERROR_TYPE(VDP_STATUS_INVALID_COLOR_TABLE_FORMAT,"An invalid/unsupported VdpColorTableFormat value was supplied.");
   _ERROR_TYPE(VDP_STATUS_INVALID_BLEND_FACTOR,"An invalid/unsupported VdpOutputSurfaceRenderBlendFactor value was supplied.");
   _ERROR_TYPE(VDP_STATUS_INVALID_BLEND_EQUATION,"An invalid/unsupported VdpOutputSurfaceRenderBlendEquation value was supplied.");
   _ERROR_TYPE(VDP_STATUS_INVALID_FLAG,"An invalid/unsupported flag value/combination was supplied.");
   _ERROR_TYPE(VDP_STATUS_INVALID_DECODER_PROFILE,"An invalid/unsupported VdpDecoderProfile value was supplied.");
   _ERROR_TYPE(VDP_STATUS_INVALID_VIDEO_MIXER_FEATURE,"An invalid/unsupported VdpVideoMixerFeature value was supplied.");
   _ERROR_TYPE(VDP_STATUS_INVALID_VIDEO_MIXER_PARAMETER,"An invalid/unsupported VdpVideoMixerParameter value was supplied.");
   _ERROR_TYPE(VDP_STATUS_INVALID_VIDEO_MIXER_ATTRIBUTE,"An invalid/unsupported VdpVideoMixerAttribute value was supplied.");
   _ERROR_TYPE(VDP_STATUS_INVALID_VIDEO_MIXER_PICTURE_STRUCTURE,"An invalid/unsupported VdpVideoMixerPictureStructure value was supplied.");
   _ERROR_TYPE(VDP_STATUS_INVALID_FUNC_ID,"An invalid/unsupported VdpFuncId value was supplied.");
   _ERROR_TYPE(VDP_STATUS_INVALID_SIZE,"The size of a supplied object does not match the object it is being used with.\
      For example, a VdpVideoMixer is configured to process VdpVideoSurface objects of a specific size.\
      If presented with a VdpVideoSurface of a different size, this error will be raised.");
   _ERROR_TYPE(VDP_STATUS_INVALID_VALUE,"An invalid/unsupported value was supplied.\
      This is a catch-all error code for values of type other than those with a specific error code.");
   _ERROR_TYPE(VDP_STATUS_INVALID_STRUCT_VERSION,"An invalid/unsupported structure version was specified in a versioned structure. \
      This implies that the implementation is older than the header file the application was built against.");
   _ERROR_TYPE(VDP_STATUS_RESOURCES,"The system does not have enough resources to complete the requested operation at this time.");
   _ERROR_TYPE(VDP_STATUS_HANDLE_DEVICE_MISMATCH,"The set of handles supplied are not all related to the same VdpDevice.When performing operations \
      that operate on multiple surfaces, such as VdpOutputSurfaceRenderOutputSurface or VdpVideoMixerRender, \
      all supplied surfaces must have been created within the context of the same VdpDevice object. \
      This error is raised if they were not.");
   _ERROR_TYPE(VDP_STATUS_ERROR,"A catch-all error, used when no other error code applies.");
   default: return "Unknown Error";
   }
}

void
vlVdpDefaultSamplerViewTemplate(struct pipe_sampler_view *templ, struct pipe_resource *res)
{
   const struct util_format_description *desc;

   memset(templ, 0, sizeof(*templ));
   u_sampler_view_default_template(templ, res, res->format);

   desc = util_format_description(res->format);
   if (desc->swizzle[0] == PIPE_SWIZZLE_0)
      templ->swizzle_r = PIPE_SWIZZLE_1;
   if (desc->swizzle[1] == PIPE_SWIZZLE_0)
      templ->swizzle_g = PIPE_SWIZZLE_1;
   if (desc->swizzle[2] == PIPE_SWIZZLE_0)
      templ->swizzle_b = PIPE_SWIZZLE_1;
   if (desc->swizzle[3] == PIPE_SWIZZLE_0)
      templ->swizzle_a = PIPE_SWIZZLE_1;
}
