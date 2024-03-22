/*
 * Copyright © 2011 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT.  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Authors:
 *    Benjamin Franzke <benjaminfranzke@googlemail.com>
 */

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#ifdef MAJOR_IN_MKDEV
#include <sys/mkdev.h>
#endif
#ifdef MAJOR_IN_SYSMACROS
#include <sys/sysmacros.h>
#endif
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

#include "gbm.h"
#include "gbmint.h"
#include "backend.h"

/** Returns the file description for the gbm device
 *
 * \return The fd that the struct gbm_device was created with
 */
GBM_EXPORT int
gbm_device_get_fd(struct gbm_device *gbm)
{
   return gbm->v0.fd;
}

/** Get the backend name for the given gbm device
 *
 * \return The backend name string - this belongs to the device and must not
 * be freed
 */
GBM_EXPORT const char *
gbm_device_get_backend_name(struct gbm_device *gbm)
{
   return gbm->v0.name;
}

/** Test if a format is supported for a given set of usage flags.
 *
 * \param gbm The created buffer manager
 * \param format The format to test
 * \param flags A bitmask of the usages to test the format against
 * \return 1 if the format is supported otherwise 0
 *
 * \sa enum gbm_bo_flags for the list of flags that the format can be
 * tested against
 *
 * \sa enum gbm_bo_format for the list of formats
 */
GBM_EXPORT int
gbm_device_is_format_supported(struct gbm_device *gbm,
                               uint32_t format, uint32_t flags)
{
   return gbm->v0.is_format_supported(gbm, format, flags);
}

/** Get the number of planes that are required for a given format+modifier
 *
 * \param gbm The gbm device returned from gbm_create_device()
 * \param format The format to query
 * \param modifier The modifier to query
 */
GBM_EXPORT int
gbm_device_get_format_modifier_plane_count(struct gbm_device *gbm,
                                           uint32_t format,
                                           uint64_t modifier)
{
   return gbm->v0.get_format_modifier_plane_count(gbm, format, modifier);
}

/** Destroy the gbm device and free all resources associated with it.
 *
 * Prior to calling this function all buffers and surfaces created with the
 * gbm device need to be destroyed.
 *
 * \param gbm The device created using gbm_create_device()
 */
GBM_EXPORT void
gbm_device_destroy(struct gbm_device *gbm)
{
   _gbm_device_destroy(gbm);
}

/** Create a gbm device for allocating buffers
 *
 * The file descriptor passed in is used by the backend to communicate with
 * platform for allocating the memory. For allocations using DRI this would be
 * the file descriptor returned when opening a device such as \c
 * /dev/dri/card0
 *
 * \param fd The file descriptor for a backend specific device
 * \return The newly created struct gbm_device. The resources associated with
 * the device should be freed with gbm_device_destroy() when it is no longer
 * needed. If the creation of the device failed NULL will be returned.
 */
GBM_EXPORT struct gbm_device *
gbm_create_device(int fd)
{
   struct gbm_device *gbm = NULL;
   struct stat buf;

   if (fd < 0 || fstat(fd, &buf) < 0 || !S_ISCHR(buf.st_mode)) {
      errno = EINVAL;
      return NULL;
   }

   gbm = _gbm_create_device(fd);
   if (gbm == NULL)
      return NULL;

   gbm->dummy = gbm_create_device;

   return gbm;
}

/** Get the width of the buffer object
 *
 * \param bo The buffer object
 * \return The width of the allocated buffer object
 *
 */
GBM_EXPORT uint32_t
gbm_bo_get_width(struct gbm_bo *bo)
{
   return bo->v0.width;
}

/** Get the height of the buffer object
 *
 * \param bo The buffer object
 * \return The height of the allocated buffer object
 */
GBM_EXPORT uint32_t
gbm_bo_get_height(struct gbm_bo *bo)
{
   return bo->v0.height;
}

/** Get the stride of the buffer object
 *
 * This is calculated by the backend when it does the allocation in
 * gbm_bo_create()
 *
 * \param bo The buffer object
 * \return The stride of the allocated buffer object in bytes
 */
GBM_EXPORT uint32_t
gbm_bo_get_stride(struct gbm_bo *bo)
{
   return gbm_bo_get_stride_for_plane(bo, 0);
}

/** Get the stride for the given plane
 *
 * \param bo The buffer object
 * \param plane for which you want the stride
 *
 * \sa gbm_bo_get_stride()
 */
GBM_EXPORT uint32_t
gbm_bo_get_stride_for_plane(struct gbm_bo *bo, int plane)
{
   return bo->gbm->v0.bo_get_stride(bo, plane);
}

/** Get the format of the buffer object
 *
 * The format of the pixels in the buffer.
 *
 * \param bo The buffer object
 * \return The format of buffer object, one of the GBM_FORMAT_* codes
 */
GBM_EXPORT uint32_t
gbm_bo_get_format(struct gbm_bo *bo)
{
   return bo->v0.format;
}

/** Get the bit-per-pixel of the buffer object's format
 *
 * The bits-per-pixel of the buffer object's format.
 *
 * Note; The 'in-memory pixel' concept makes no sense for YUV formats
 * (pixels are the result of the combination of multiple memory sources:
 * Y, Cb & Cr; usually these are even in separate buffers), so YUV
 * formats are not supported by this function.
 *
 * \param bo The buffer object
 * \return The number of bits0per-pixel of the buffer object's format.
 */
GBM_EXPORT uint32_t
gbm_bo_get_bpp(struct gbm_bo *bo)
{
   switch (bo->v0.format) {
      default:
         return 0;
      case GBM_FORMAT_C8:
      case GBM_FORMAT_R8:
      case GBM_FORMAT_RGB332:
      case GBM_FORMAT_BGR233:
         return 8;
      case GBM_FORMAT_R16:
      case GBM_FORMAT_GR88:
      case GBM_FORMAT_XRGB4444:
      case GBM_FORMAT_XBGR4444:
      case GBM_FORMAT_RGBX4444:
      case GBM_FORMAT_BGRX4444:
      case GBM_FORMAT_ARGB4444:
      case GBM_FORMAT_ABGR4444:
      case GBM_FORMAT_RGBA4444:
      case GBM_FORMAT_BGRA4444:
      case GBM_FORMAT_XRGB1555:
      case GBM_FORMAT_XBGR1555:
      case GBM_FORMAT_RGBX5551:
      case GBM_FORMAT_BGRX5551:
      case GBM_FORMAT_ARGB1555:
      case GBM_FORMAT_ABGR1555:
      case GBM_FORMAT_RGBA5551:
      case GBM_FORMAT_BGRA5551:
      case GBM_FORMAT_RGB565:
      case GBM_FORMAT_BGR565:
         return 16;
      case GBM_FORMAT_RGB888:
      case GBM_FORMAT_BGR888:
         return 24;
      case GBM_FORMAT_RG1616:
      case GBM_FORMAT_GR1616:
      case GBM_FORMAT_XRGB8888:
      case GBM_FORMAT_XBGR8888:
      case GBM_FORMAT_RGBX8888:
      case GBM_FORMAT_BGRX8888:
      case GBM_FORMAT_ARGB8888:
      case GBM_FORMAT_ABGR8888:
      case GBM_FORMAT_RGBA8888:
      case GBM_FORMAT_BGRA8888:
      case GBM_FORMAT_XRGB2101010:
      case GBM_FORMAT_XBGR2101010:
      case GBM_FORMAT_RGBX1010102:
      case GBM_FORMAT_BGRX1010102:
      case GBM_FORMAT_ARGB2101010:
      case GBM_FORMAT_ABGR2101010:
      case GBM_FORMAT_RGBA1010102:
      case GBM_FORMAT_BGRA1010102:
         return 32;
      case GBM_FORMAT_XBGR16161616:
      case GBM_FORMAT_ABGR16161616:
      case GBM_FORMAT_XBGR16161616F:
      case GBM_FORMAT_ABGR16161616F:
         return 64;
   }
}

/** Get the offset for the data of the specified plane
 *
 * Extra planes, and even the first plane, may have an offset from the start of
 * the buffer object. This function will provide the offset for the given plane
 * to be used in various KMS APIs.
 *
 * \param bo The buffer object
 * \return The offset
 */
GBM_EXPORT uint32_t
gbm_bo_get_offset(struct gbm_bo *bo, int plane)
{
   return bo->gbm->v0.bo_get_offset(bo, plane);
}

/** Get the gbm device used to create the buffer object
 *
 * \param bo The buffer object
 * \return Returns the gbm device with which the buffer object was created
 */
GBM_EXPORT struct gbm_device *
gbm_bo_get_device(struct gbm_bo *bo)
{
	return bo->gbm;
}

/** Get the handle of the buffer object
 *
 * This is stored in the platform generic union gbm_bo_handle type. However
 * the format of this handle is platform specific.
 *
 * \param bo The buffer object
 * \return Returns the handle of the allocated buffer object
 */
GBM_EXPORT union gbm_bo_handle
gbm_bo_get_handle(struct gbm_bo *bo)
{
   return bo->v0.handle;
}

/** Get a DMA-BUF file descriptor for the buffer object
 *
 * This function creates a DMA-BUF (also known as PRIME) file descriptor
 * handle for the buffer object.  Each call to gbm_bo_get_fd() returns a new
 * file descriptor and the caller is responsible for closing the file
 * descriptor.

 * \param bo The buffer object
 * \return Returns a file descriptor referring to the underlying buffer or -1
 * if an error occurs.
 */
GBM_EXPORT int
gbm_bo_get_fd(struct gbm_bo *bo)
{
   return bo->gbm->v0.bo_get_fd(bo);
}

/** Get the number of planes for the given bo.
 *
 * \param bo The buffer object
 * \return The number of planes
 */
GBM_EXPORT int
gbm_bo_get_plane_count(struct gbm_bo *bo)
{
   return bo->gbm->v0.bo_get_planes(bo);
}

/** Get the handle for the specified plane of the buffer object
 *
 * This function gets the handle for any plane associated with the BO. When
 * dealing with multi-planar formats, or formats which might have implicit
 * planes based on different underlying hardware it is necessary for the client
 * to be able to get this information to pass to the DRM.
 *
 * \param bo The buffer object
 * \param plane the plane to get a handle for
 *
 * \sa gbm_bo_get_handle()
 */
GBM_EXPORT union gbm_bo_handle
gbm_bo_get_handle_for_plane(struct gbm_bo *bo, int plane)
{
   return bo->gbm->v0.bo_get_handle(bo, plane);
}

/** Get a DMA-BUF file descriptor for the specified plane of the buffer object
 *
 * This function creates a DMA-BUF (also known as PRIME) file descriptor
 * handle for the specified plane of the buffer object.  Each call to
 * gbm_bo_get_fd_for_plane() returns a new file descriptor and the caller is
 * responsible for closing the file descriptor.

 * \param bo The buffer object
 * \param plane The plane to get a DMA-BUF for
 * \return Returns a file descriptor referring to the underlying buffer or -1
 * if an error occurs.
 *
 * \sa gbm_bo_get_fd()
 */
GBM_EXPORT int
gbm_bo_get_fd_for_plane(struct gbm_bo *bo, int plane)
{
   return bo->gbm->v0.bo_get_plane_fd(bo, plane);
}

/**
 * Get the chosen modifier for the buffer object
 *
 * This function returns the modifier that was chosen for the object. These
 * properties may be generic, or platform/implementation dependent.
 *
 * \param bo The buffer object
 * \return Returns the selected modifier (chosen by the implementation) for the
 * BO.
 * \sa gbm_bo_create_with_modifiers() where possible modifiers are set
 * \sa gbm_surface_create_with_modifiers() where possible modifiers are set
 * \sa define DRM_FORMAT_MOD_* in drm_fourcc.h for possible modifiers
 */
GBM_EXPORT uint64_t
gbm_bo_get_modifier(struct gbm_bo *bo)
{
   return bo->gbm->v0.bo_get_modifier(bo);
}

/** Write data into the buffer object
 *
 * If the buffer object was created with the GBM_BO_USE_WRITE flag,
 * this function can be used to write data into the buffer object.  The
 * data is copied directly into the object and it's the responsibility
 * of the caller to make sure the data represents valid pixel data,
 * according to the width, height, stride and format of the buffer object.
 *
 * \param bo The buffer object
 * \param buf The data to write
 * \param count The number of bytes to write
 * \return Returns 0 on success, otherwise -1 is returned an errno set
 */
GBM_EXPORT int
gbm_bo_write(struct gbm_bo *bo, const void *buf, size_t count)
{
   return bo->gbm->v0.bo_write(bo, buf, count);
}

/** Set the user data associated with a buffer object
 *
 * \param bo The buffer object
 * \param data The data to associate to the buffer object
 * \param destroy_user_data A callback (which may be %NULL) that will be
 * called prior to the buffer destruction
 */
GBM_EXPORT void
gbm_bo_set_user_data(struct gbm_bo *bo, void *data,
		     void (*destroy_user_data)(struct gbm_bo *, void *))
{
   bo->v0.user_data = data;
   bo->v0.destroy_user_data = destroy_user_data;
}

/** Get the user data associated with a buffer object
 *
 * \param bo The buffer object
 * \return Returns the user data associated with the buffer object or %NULL
 * if no data was associated with it
 *
 * \sa gbm_bo_set_user_data()
 */
GBM_EXPORT void *
gbm_bo_get_user_data(struct gbm_bo *bo)
{
   return bo->v0.user_data;
}

/**
 * Destroys the given buffer object and frees all resources associated with
 * it.
 *
 * \param bo The buffer object
 */
GBM_EXPORT void
gbm_bo_destroy(struct gbm_bo *bo)
{
   if (bo->v0.destroy_user_data)
      bo->v0.destroy_user_data(bo, bo->v0.user_data);

   bo->gbm->v0.bo_destroy(bo);
}

/**
 * Allocate a buffer object for the given dimensions
 *
 * \param gbm The gbm device returned from gbm_create_device()
 * \param width The width for the buffer
 * \param height The height for the buffer
 * \param format The format to use for the buffer, from GBM_FORMAT_* or
 * GBM_BO_FORMAT_* tokens
 * \param flags The union of the usage flags for this buffer
 *
 * \return A newly allocated buffer that should be freed with gbm_bo_destroy()
 * when no longer needed. If an error occurs during allocation %NULL will be
 * returned and errno set.
 *
 * \sa enum gbm_bo_flags for the list of usage flags
 */
GBM_EXPORT struct gbm_bo *
gbm_bo_create(struct gbm_device *gbm,
              uint32_t width, uint32_t height,
              uint32_t format, uint32_t flags)
{
   if (width == 0 || height == 0) {
      errno = EINVAL;
      return NULL;
   }

   return gbm->v0.bo_create(gbm, width, height, format, flags, NULL, 0);
}

GBM_EXPORT struct gbm_bo *
gbm_bo_create_with_modifiers(struct gbm_device *gbm,
                             uint32_t width, uint32_t height,
                             uint32_t format,
                             const uint64_t *modifiers,
                             const unsigned int count)
{
   uint32_t flags = 0;

   /*
    * ABI version 1 added the modifiers+flags capability. Backends from
    * prior versions may fail if "unknown" flags are provided along with
    * modifiers, but assume scanout is required when modifiers are used.
    * Newer backends expect scanout to be explicitly requested if required,
    * but applications using this older interface rely on the older implied
    * requirement, so that behavior must be preserved.
    */
   if (gbm->v0.backend_version >= 1) {
      flags |= GBM_BO_USE_SCANOUT;
   }

   return gbm_bo_create_with_modifiers2(gbm, width, height, format, modifiers,
                                        count, flags);
}

GBM_EXPORT struct gbm_bo *
gbm_bo_create_with_modifiers2(struct gbm_device *gbm,
                              uint32_t width, uint32_t height,
                              uint32_t format,
                              const uint64_t *modifiers,
                              const unsigned int count,
                              uint32_t flags)
{
   if (width == 0 || height == 0) {
      errno = EINVAL;
      return NULL;
   }

   if ((count && !modifiers) || (modifiers && !count)) {
      errno = EINVAL;
      return NULL;
   }

   if (modifiers && (flags & GBM_BO_USE_LINEAR)) {
      errno = EINVAL;
      return NULL;
   }

   return gbm->v0.bo_create(gbm, width, height, format, flags, modifiers, count);
}

/**
 * Create a gbm buffer object from a foreign object
 *
 * This function imports a foreign object and creates a new gbm bo for it.
 * This enables using the foreign object with a display API such as KMS.
 * Currently these types of foreign objects are supported, indicated by the type
 * argument:
 *
 *   GBM_BO_IMPORT_WL_BUFFER
 *   GBM_BO_IMPORT_EGL_IMAGE
 *   GBM_BO_IMPORT_FD
 *   GBM_BO_IMPORT_FD_MODIFIER
 *
 * The gbm bo shares the underlying pixels but its life-time is
 * independent of the foreign object.
 *
 * \param gbm The gbm device returned from gbm_create_device()
 * \param type The type of object we're importing
 * \param buffer Pointer to the external object
 * \param flags The union of the usage flags for this buffer
 *
 * \return A newly allocated buffer object that should be freed with
 * gbm_bo_destroy() when no longer needed. On error, %NULL is returned
 * and errno is set.
 *
 * \sa enum gbm_bo_flags for the list of usage flags
 */
GBM_EXPORT struct gbm_bo *
gbm_bo_import(struct gbm_device *gbm,
              uint32_t type, void *buffer, uint32_t flags)
{
   return gbm->v0.bo_import(gbm, type, buffer, flags);
}

/**
 * Map a region of a gbm buffer object for cpu access
 *
 * This function maps a region of a gbm bo for cpu read and/or write
 * access.
 *
 * The mapping exposes a linear view of the buffer object even if the buffer
 * has a non-linear modifier.
 *
 * This function may require intermediate buffer copies (ie. it may be slow).
 *
 * \param bo The buffer object
 * \param x The X (top left origin) starting position of the mapped region for
 * the buffer
 * \param y The Y (top left origin) starting position of the mapped region for
 * the buffer
 * \param width The width of the mapped region for the buffer
 * \param height The height of the mapped region for the buffer
 * \param flags The union of the GBM_BO_TRANSFER_* flags for this buffer
 * \param stride Ptr for returned stride in bytes of the mapped region
 * \param map_data Returned opaque ptr for the mapped region
 *
 * \return Address of the mapped buffer that should be unmapped with
 * gbm_bo_unmap() when no longer needed. On error, %NULL is returned
 * and errno is set.
 *
 * \sa enum gbm_bo_transfer_flags for the list of flags
 */
GBM_EXPORT void *
gbm_bo_map(struct gbm_bo *bo,
              uint32_t x, uint32_t y,
              uint32_t width, uint32_t height,
              uint32_t flags, uint32_t *stride, void **map_data)
{
   if (!bo || width == 0 || height == 0 || !stride || !map_data) {
      errno = EINVAL;
      return NULL;
   }

   return bo->gbm->v0.bo_map(bo, x, y, width, height,
                             flags, stride, map_data);
}

/**
 * Unmap a previously mapped region of a gbm buffer object
 *
 * This function unmaps a region of a gbm bo for cpu read and/or write
 * access.
 *
 * \param bo The buffer object
 * \param map_data opaque ptr returned from prior gbm_bo_map
 */
GBM_EXPORT void
gbm_bo_unmap(struct gbm_bo *bo, void *map_data)
{
   bo->gbm->v0.bo_unmap(bo, map_data);
}

/**
 * Allocate a surface object
 *
 * \param gbm The gbm device returned from gbm_create_device()
 * \param width The width for the surface
 * \param height The height for the surface
 * \param format The format to use for the surface
 *
 * \return A newly allocated surface that should be freed with
 * gbm_surface_destroy() when no longer needed. If an error occurs
 * during allocation %NULL will be returned.
 *
 * \sa enum gbm_bo_format for the list of formats
 */
GBM_EXPORT struct gbm_surface *
gbm_surface_create(struct gbm_device *gbm,
                   uint32_t width, uint32_t height,
		   uint32_t format, uint32_t flags)
{
   return gbm->v0.surface_create(gbm, width, height, format, flags, NULL, 0);
}

GBM_EXPORT struct gbm_surface *
gbm_surface_create_with_modifiers(struct gbm_device *gbm,
                                  uint32_t width, uint32_t height,
                                  uint32_t format,
                                  const uint64_t *modifiers,
                                  const unsigned int count)
{
   uint32_t flags = 0;

   /*
    * ABI version 1 added the modifiers+flags capability. Backends from
    * prior versions may fail if "unknown" flags are provided along with
    * modifiers, but assume scanout is required when modifiers are used.
    * Newer backends expect scanout to be explicitly requested if required,
    * but applications using this older interface rely on the older implied
    * requirement, so that behavior must be preserved.
    */
   if (gbm->v0.backend_version >= 1) {
      flags |= GBM_BO_USE_SCANOUT;
   }

   return gbm_surface_create_with_modifiers2(gbm, width, height, format,
                                             modifiers, count,
                                             flags);
}

GBM_EXPORT struct gbm_surface *
gbm_surface_create_with_modifiers2(struct gbm_device *gbm,
                                   uint32_t width, uint32_t height,
                                   uint32_t format,
                                   const uint64_t *modifiers,
                                   const unsigned int count,
                                   uint32_t flags)
{
   if ((count && !modifiers) || (modifiers && !count)) {
      errno = EINVAL;
      return NULL;
   }

   if (modifiers && (flags & GBM_BO_USE_LINEAR)) {
      errno = EINVAL;
      return NULL;
   }

   return gbm->v0.surface_create(gbm, width, height, format, flags,
                                 modifiers, count);
}

/**
 * Destroys the given surface and frees all resources associated with it.
 *
 * Prior to calling this function all buffers locked with
 * gbm_surface_lock_front_buffer() need to be released and the associated
 * EGL surface destroyed.
 *
 * \param surf The surface
 */
GBM_EXPORT void
gbm_surface_destroy(struct gbm_surface *surf)
{
   surf->gbm->v0.surface_destroy(surf);
}

/**
 * Lock the surface's current front buffer
 *
 * Lock rendering to the surface's current front buffer until it is
 * released with gbm_surface_release_buffer().
 *
 * This function must be called exactly once after calling
 * eglSwapBuffers.  Calling it before any eglSwapBuffer has happened
 * on the surface or two or more times after eglSwapBuffers is an error.
 *
 * \param surf The surface
 *
 * \return A buffer object representing the front buffer that should be
 * released with gbm_surface_release_buffer() when no longer needed and before
 * the associated EGL surface gets destroyed. The implementation is free to
 * reuse buffers released with gbm_surface_release_buffer() so this bo should
 * not be destroyed using gbm_bo_destroy(). If an error occurs this function
 * returns %NULL.
 */
GBM_EXPORT struct gbm_bo *
gbm_surface_lock_front_buffer(struct gbm_surface *surf)
{
   return surf->gbm->v0.surface_lock_front_buffer(surf);
}

/**
 * Release a locked buffer obtained with gbm_surface_lock_front_buffer()
 *
 * Returns the underlying buffer to the gbm surface.  Releasing a bo
 * will typically make gbm_surface_has_free_buffer() return 1 and thus
 * allow rendering the next frame, but not always. The implementation
 * may choose to destroy the bo immediately or reuse it, in which case
 * the user data associated with it is unchanged.
 *
 * \param surf The surface
 * \param bo The buffer object
 */
GBM_EXPORT void
gbm_surface_release_buffer(struct gbm_surface *surf, struct gbm_bo *bo)
{
   surf->gbm->v0.surface_release_buffer(surf, bo);
}

/**
 * Return whether or not a surface has free (non-locked) buffers
 *
 * Before starting a new frame, the surface must have a buffer
 * available for rendering.  Initially, a gbm surface will have a free
 * buffer, but after one or more buffers have been locked (\sa
 * gbm_surface_lock_front_buffer()), the application must check for a
 * free buffer before rendering.
 *
 * If a surface doesn't have a free buffer, the application must
 * return a buffer to the surface using gbm_surface_release_buffer()
 * and after that, the application can query for free buffers again.
 *
 * \param surf The surface
 * \return 1 if the surface has free buffers, 0 otherwise
 */
GBM_EXPORT int
gbm_surface_has_free_buffers(struct gbm_surface *surf)
{
   return surf->gbm->v0.surface_has_free_buffers(surf);
}

/* The two GBM_BO_FORMAT_[XA]RGB8888 formats alias the GBM_FORMAT_*
 * formats of the same name. We want to accept them whenever someone
 * has a GBM format, but never return them to the user. */
static uint32_t
format_canonicalize(uint32_t gbm_format)
{
   switch (gbm_format) {
   case GBM_BO_FORMAT_XRGB8888:
      return GBM_FORMAT_XRGB8888;
   case GBM_BO_FORMAT_ARGB8888:
      return GBM_FORMAT_ARGB8888;
   default:
      return gbm_format;
   }
}

/**
 * Returns a string representing the fourcc format name.
 *
 * \param desc Caller-provided storage for the format name string.
 * \return String containing the fourcc of the format.
 */
GBM_EXPORT char *
gbm_format_get_name(uint32_t gbm_format, struct gbm_format_name_desc *desc)
{
   gbm_format = format_canonicalize(gbm_format);

   desc->name[0] = gbm_format;
   desc->name[1] = gbm_format >> 8;
   desc->name[2] = gbm_format >> 16;
   desc->name[3] = gbm_format >> 24;
   desc->name[4] = 0;

   return desc->name;
}

/**
 * A global table of functions and global variables defined in the core GBM
 * code that need to be accessed directly by GBM backends.
 */
struct gbm_core gbm_core = {
   .v0.core_version = GBM_BACKEND_ABI_VERSION,
   .v0.format_canonicalize = format_canonicalize,
};
