/**********************************************************
 * Copyright 2009-2023 VMware, Inc.  All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 **********************************************************/


#include "util/compiler.h"
#include "util/u_inlines.h"
#include "util/u_memory.h"
#include "util/format/u_format.h"

#include "vmw_context.h"
#include "vmw_screen.h"
#include "vmw_surface.h"
#include "vmw_buffer.h"
#include "svga_drm_public.h"
#include "svga3d_surfacedefs.h"

#include "frontend/drm_driver.h"

#include "vmwgfx_drm.h"
#include <xf86drm.h>

#include <stdio.h>
#include <fcntl.h>

struct dri1_api_version {
   int major;
   int minor;
   int patch_level;
};

static struct svga_winsys_surface *
vmw_drm_surface_from_handle(struct svga_winsys_screen *sws,
			    struct winsys_handle *whandle,
			    SVGA3dSurfaceFormat *format);

static struct svga_winsys_surface *
vmw_drm_gb_surface_from_handle(struct svga_winsys_screen *sws,
                               struct winsys_handle *whandle,
                               SVGA3dSurfaceFormat *format);
static bool
vmw_drm_surface_get_handle(struct svga_winsys_screen *sws,
			   struct svga_winsys_surface *surface,
			   unsigned stride,
			   struct winsys_handle *whandle);

static struct dri1_api_version drm_required = { 2, 1, 0 };
static struct dri1_api_version drm_compat = { 2, 0, 0 };

static bool
vmw_dri1_check_version(const struct dri1_api_version *cur,
		       const struct dri1_api_version *required,
		       const struct dri1_api_version *compat,
		       const char component[])
{
   if (cur->major > required->major && cur->major <= compat->major)
      return true;
   if (cur->major == required->major && cur->minor >= required->minor)
      return true;

   vmw_error("%s version failure.\n", component);
   vmw_error("%s version is %d.%d.%d and this driver can only work\n"
             "with versions %d.%d.x through %d.x.x.\n",
             component,
             cur->major, cur->minor, cur->patch_level,
             required->major, required->minor, compat->major);
   return false;
}

/* This is actually the entrypoint to the entire driver,
 * called by the target bootstrap code.
 */
struct svga_winsys_screen *
svga_drm_winsys_screen_create(int fd)
{
   struct vmw_winsys_screen *vws;
   struct dri1_api_version drm_ver;
   drmVersionPtr ver;

   ver = drmGetVersion(fd);
   if (ver == NULL)
      return NULL;

   drm_ver.major = ver->version_major;
   drm_ver.minor = ver->version_minor;
   drm_ver.patch_level = 0; /* ??? */

   drmFreeVersion(ver);
   if (!vmw_dri1_check_version(&drm_ver, &drm_required,
			       &drm_compat, "vmwgfx drm driver"))
      return NULL;

   vws = vmw_winsys_create(fd);
   if (!vws)
      goto out_no_vws;

   /* XXX do this properly */
   vws->base.surface_from_handle = vws->base.have_gb_objects ?
      vmw_drm_gb_surface_from_handle : vmw_drm_surface_from_handle;
   vws->base.surface_get_handle = vmw_drm_surface_get_handle;

   return &vws->base;

out_no_vws:
   return NULL;
}

/**
 * vmw_drm_gb_surface_from_handle - Create a shared surface
 *
 * @sws: Screen to register the surface with.
 * @whandle: struct winsys_handle identifying the kernel surface object
 * @format: On successful return points to a value describing the
 * surface format.
 *
 * Returns a refcounted pointer to a struct svga_winsys_surface
 * embedded in a struct vmw_svga_winsys_surface on success or NULL
 * on failure.
 */
static struct svga_winsys_surface *
vmw_drm_gb_surface_from_handle(struct svga_winsys_screen *sws,
                               struct winsys_handle *whandle,
                               SVGA3dSurfaceFormat *format)
{
    struct vmw_svga_winsys_surface *vsrf;
    struct svga_winsys_surface *ssrf;
    struct vmw_winsys_screen *vws = vmw_winsys_screen(sws);
    SVGA3dSurfaceAllFlags flags;
    uint32_t mip_levels;
    struct vmw_buffer_desc desc;
    struct pb_manager *provider = vws->pools.dma_base;
    struct pb_buffer *pb_buf;
    uint32_t handle;
    int ret;

    if (whandle->offset != 0) {
       fprintf(stderr, "Attempt to import unsupported winsys offset %u\n",
               whandle->offset);
       return NULL;
    }

    ret = vmw_ioctl_gb_surface_ref(vws, whandle, &flags, format,
                                   &mip_levels, &handle, &desc.region);

    if (ret) {
	fprintf(stderr, "Failed referencing shared surface. SID %d.\n"
		"Error %d (%s).\n",
		whandle->handle, ret, strerror(-ret));
	return NULL;
    }

    if (mip_levels != 1) {
       fprintf(stderr, "Incorrect number of mipmap levels on shared surface."
               " SID %d, levels %d\n",
               whandle->handle, mip_levels);
       goto out_mip;
    }

    vsrf = CALLOC_STRUCT(vmw_svga_winsys_surface);
    if (!vsrf)
	goto out_mip;

    pipe_reference_init(&vsrf->refcnt, 1);
    p_atomic_set(&vsrf->validated, 0);
    vsrf->screen = vws;
    vsrf->sid = handle;
    vsrf->size = vmw_region_size(desc.region);

    /*
     * Synchronize backing buffers of shared surfaces using the
     * kernel, since we don't pass fence objects around between
     * processes.
     */
    desc.pb_desc.alignment = 4096;
    desc.pb_desc.usage = VMW_BUFFER_USAGE_SHARED | VMW_BUFFER_USAGE_SYNC;
    pb_buf = provider->create_buffer(provider, vsrf->size, &desc.pb_desc);
    vsrf->buf = vmw_svga_winsys_buffer_wrap(pb_buf);
    if (!vsrf->buf)
       goto out_no_buf;
    ssrf = svga_winsys_surface(vsrf);

    return ssrf;

out_no_buf:
    FREE(vsrf);
out_mip:
    vmw_ioctl_region_destroy(desc.region);
    vmw_ioctl_surface_destroy(vws, whandle->handle);
    return NULL;
}

static struct svga_winsys_surface *
vmw_drm_surface_from_handle(struct svga_winsys_screen *sws,
                            struct winsys_handle *whandle,
			    SVGA3dSurfaceFormat *format)
{
    struct vmw_svga_winsys_surface *vsrf;
    struct svga_winsys_surface *ssrf;
    struct vmw_winsys_screen *vws = vmw_winsys_screen(sws);
    union drm_vmw_surface_reference_arg arg;
    struct drm_vmw_surface_arg *req = &arg.req;
    struct drm_vmw_surface_create_req *rep = &arg.rep;
    uint32_t handle = 0;
    struct drm_vmw_size size;
    SVGA3dSize base_size;
    int ret;
    int i;

    if (whandle->offset != 0) {
       fprintf(stderr, "Attempt to import unsupported winsys offset %u\n",
               whandle->offset);
       return NULL;
    }

    switch (whandle->type) {
    case WINSYS_HANDLE_TYPE_SHARED:
    case WINSYS_HANDLE_TYPE_KMS:
       handle = whandle->handle;
       break;
    case WINSYS_HANDLE_TYPE_FD:
       ret = drmPrimeFDToHandle(vws->ioctl.drm_fd, whandle->handle,
                                &handle);
       if (ret) {
	  vmw_error("Failed to get handle from prime fd %d.\n",
		    (int) whandle->handle);
	  return NULL;
       }
       break;
    default:
       vmw_error("Attempt to import unsupported handle type %d.\n",
                 whandle->type);
       return NULL;
    }

    memset(&arg, 0, sizeof(arg));
    req->sid = handle;
    rep->size_addr = (unsigned long)&size;

    ret = drmCommandWriteRead(vws->ioctl.drm_fd, DRM_VMW_REF_SURFACE,
			      &arg, sizeof(arg));

    /*
     * Need to close the handle we got from prime.
     */
    if (whandle->type == WINSYS_HANDLE_TYPE_FD)
       vmw_ioctl_surface_destroy(vws, handle);

    if (ret) {
       /*
        * Any attempt to share something other than a surface, like a dumb
        * kms buffer, should fail here.
        */
       vmw_error("Failed referencing shared surface. SID %d.\n"
                 "Error %d (%s).\n",
                 handle, ret, strerror(-ret));
       return NULL;
    }

    if (rep->mip_levels[0] != 1) {
        vmw_error("Incorrect number of mipmap levels on shared surface."
                  " SID %d, levels %d\n",
                  handle, rep->mip_levels[0]);
	goto out_mip;
    }

    for (i=1; i < DRM_VMW_MAX_SURFACE_FACES; ++i) {
	if (rep->mip_levels[i] != 0) {
            vmw_error("Incorrect number of faces levels on shared surface."
                      " SID %d, face %d present.\n",
                      handle, i);
	    goto out_mip;
	}
   }

    vsrf = CALLOC_STRUCT(vmw_svga_winsys_surface);
    if (!vsrf)
	goto out_mip;

    pipe_reference_init(&vsrf->refcnt, 1);
    p_atomic_set(&vsrf->validated, 0);
    vsrf->screen = vws;
    vsrf->sid = handle;
    ssrf = svga_winsys_surface(vsrf);
    *format = rep->format;

    /* Estimate usage, for early flushing. */

    base_size.width = size.width;
    base_size.height = size.height;
    base_size.depth = size.depth;
    vsrf->size = svga3dsurface_get_serialized_size(rep->format, base_size,
                                                   rep->mip_levels[0],
                                                   false);

    return ssrf;

out_mip:
    vmw_ioctl_surface_destroy(vws, handle);

    return NULL;
}

static bool
vmw_drm_surface_get_handle(struct svga_winsys_screen *sws,
			   struct svga_winsys_surface *surface,
			   unsigned stride,
			   struct winsys_handle *whandle)
{
    struct vmw_winsys_screen *vws = vmw_winsys_screen(sws);
    struct vmw_svga_winsys_surface *vsrf;
    int ret;

    if (!surface)
	return false;

    vsrf = vmw_svga_winsys_surface(surface);
    whandle->handle = vsrf->sid;
    whandle->stride = stride;
    whandle->offset = 0;

    switch (whandle->type) {
    case WINSYS_HANDLE_TYPE_SHARED:
    case WINSYS_HANDLE_TYPE_KMS:
       whandle->handle = vsrf->sid;
       break;
    case WINSYS_HANDLE_TYPE_FD:
       ret = drmPrimeHandleToFD(vws->ioctl.drm_fd, vsrf->sid, DRM_CLOEXEC,
				(int *)&whandle->handle);
       if (ret) {
	  vmw_error("Failed to get file descriptor from prime.\n");
	  return false;
       }
       break;
    default:
       vmw_error("Attempt to export unsupported handle type %d.\n",
		 whandle->type);
       return false;
    }

    return true;
}
