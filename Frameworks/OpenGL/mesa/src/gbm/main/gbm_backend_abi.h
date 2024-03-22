/*
 * Copyright © 2011 Intel Corporation
 * Copyright © 2021 NVIDIA Corporation
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
 *    James Jones <jajones@nvidia.com>
 */

#ifndef GBM_BACKEND_ABI_H_
#define GBM_BACKEND_ABI_H_

#include "gbm.h"

/**
 * \file gbm_backend_abi.h
 * \brief ABI between the GBM loader and its backends
 */

struct gbm_backend_desc;

/**
 * The GBM backend interface version defined by this file.
 *
 * The GBM device interface version must be incremented whenever the structures
 * defined in this file are modified. To preserve ABI compatibility with
 * backends that support only older versions, modifications to this file must
 * consist only of appending new fields to the end of the structures defined in
 * it, defining new structures, or declaring new exported functions or global
 * variables.
 *
 * Note this version applies to ALL structures in this file, not just the core,
 * backend, and device structures which contain it explicitly. Buffer objects,
 * surfaces, and any other new structures introduced to this file are also part
 * of the backend ABI. The ABI version of an instance of any object in this file
 * is defined as the minimum of the version of the backend associated with the
 * object instance and the loader's core object version. Hence, any new objects
 * added to this file should contain either a reference to an existing object
 * defined here, or an explicit version field.
 *
 * A few examples of object versions:
 *
 * Backend ABI version: 0
 * Core ABI version: 3
 * ABI version of a device created by the backend: 0
 *
 * Backend ABI version: 2
 * Core ABI version: 1
 * ABI version of a surface created by a device from the backend: 1
 *
 * Backend ABI version: 4
 * Core ABI version: 4
 * ABI version of a buffer object created by a device from the backend: 4
 */
#define GBM_BACKEND_ABI_VERSION 1

/**
 * GBM device interface corresponding to GBM_BACKEND_ABI_VERSION = 0
 *
 * DO NOT MODIFY THIS STRUCT. Instead, introduce a gbm_bo_v1, increment
 * GBM_BACKEND_ABI_VERSION, and append gbm_bo_v1 to gbm_bo.
 */
struct gbm_device_v0 {
   const struct gbm_backend_desc *backend_desc;

   /**
    * The version of the GBM backend interface supported by this device and its
    * child objects. This may be less than the maximum version supported by the
    * GBM loader if the device was created by an older backend, or less than the
    * maximum version supported by the backend if the device was created by an
    * older loader. In other words, this will be:
    *
    *   MIN(backend GBM interface version, loader GBM interface version)
    *
    * It is the backend's responsibility to assign this field the value passed
    * in by the GBM loader to the backend's create_device function. The GBM
    * loader will pre-clamp the value based on the loader version and the
    * version reported by the backend in its gbm_backend_v0::backend_version
    * field. It is the loader's responsibility to respect this version when
    * directly accessing a device instance or any child objects instantiated by
    * a device instance.
    */
   uint32_t backend_version;

   int fd;
   const char *name;

   void (*destroy)(struct gbm_device *gbm);
   int (*is_format_supported)(struct gbm_device *gbm,
                              uint32_t format,
                              uint32_t usage);
   int (*get_format_modifier_plane_count)(struct gbm_device *device,
                                          uint32_t format,
                                          uint64_t modifier);

   /**
    * Since version 1, usage is properly populated when modifiers are
    * supplied. Version 0 always set usage to 0 in this case.
    */
   struct gbm_bo *(*bo_create)(struct gbm_device *gbm,
                               uint32_t width, uint32_t height,
                               uint32_t format,
                               uint32_t usage,
                               const uint64_t *modifiers,
                               const unsigned int count);
   struct gbm_bo *(*bo_import)(struct gbm_device *gbm, uint32_t type,
                               void *buffer, uint32_t usage);
   void *(*bo_map)(struct gbm_bo *bo,
                               uint32_t x, uint32_t y,
                               uint32_t width, uint32_t height,
                               uint32_t flags, uint32_t *stride,
                               void **map_data);
   void (*bo_unmap)(struct gbm_bo *bo, void *map_data);
   int (*bo_write)(struct gbm_bo *bo, const void *buf, size_t data);
   int (*bo_get_fd)(struct gbm_bo *bo);
   int (*bo_get_planes)(struct gbm_bo *bo);
   union gbm_bo_handle (*bo_get_handle)(struct gbm_bo *bo, int plane);
   int (*bo_get_plane_fd)(struct gbm_bo *bo, int plane);
   uint32_t (*bo_get_stride)(struct gbm_bo *bo, int plane);
   uint32_t (*bo_get_offset)(struct gbm_bo *bo, int plane);
   uint64_t (*bo_get_modifier)(struct gbm_bo *bo);
   void (*bo_destroy)(struct gbm_bo *bo);

   /**
    * Since version 1, flags are properly populated when modifiers are
    * supplied. Version 0 always set flags to 0 in this case.
    */
   struct gbm_surface *(*surface_create)(struct gbm_device *gbm,
                                         uint32_t width, uint32_t height,
                                         uint32_t format, uint32_t flags,
                                         const uint64_t *modifiers,
                                         const unsigned count);
   struct gbm_bo *(*surface_lock_front_buffer)(struct gbm_surface *surface);
   void (*surface_release_buffer)(struct gbm_surface *surface,
                                  struct gbm_bo *bo);
   int (*surface_has_free_buffers)(struct gbm_surface *surface);
   void (*surface_destroy)(struct gbm_surface *surface);
};

/**
 * The device used for the memory allocation.
 *
 * The members of this structure should be not accessed directly
 *
 * To modify this structure, introduce a new gbm_device_v<N> structure, add it
 * to the end of this structure, and increment GBM_BACKEND_ABI_VERSION.
 */
struct gbm_device {
   /* Hack to make a gbm_device detectable by its first element. */
   struct gbm_device *(*dummy)(int);
   struct gbm_device_v0 v0;
};

/**
 * GBM buffer object interface corresponding to GBM_BACKEND_ABI_VERSION = 0
 *
 * DO NOT MODIFY THIS STRUCT. Instead, introduce a gbm_bo_v1, increment
 * GBM_BACKEND_ABI_VERSION, and append gbm_bo_v1 to gbm_bo.
 */
struct gbm_bo_v0 {
   uint32_t width;
   uint32_t height;
   uint32_t stride;
   uint32_t format;
   union gbm_bo_handle  handle;
   void *user_data;
   void (*destroy_user_data)(struct gbm_bo *, void *);
};

/**
 * The allocated buffer object.
 *
 * The members in this structure should not be accessed directly.
 *
 * To modify this structure, introduce a new gbm_bo_v<N> structure, add it to
 * the end of this structure, and increment GBM_BACKEND_ABI_VERSION.
 */
struct gbm_bo {
   struct gbm_device *gbm;
   struct gbm_bo_v0 v0;
};

/**
 * GBM surface interface corresponding to GBM_BACKEND_ABI_VERSION = 0
 *
 * DO NOT MODIFY THIS STRUCT. Instead, introduce a gbm_surface_v1, increment
 * GBM_BACKEND_ABI_VERSION, and append gbm_surface_v1 to gbm_surface.
 */
struct gbm_surface_v0 {
   uint32_t width;
   uint32_t height;
   uint32_t format;
   uint32_t flags;
   struct {
      uint64_t *modifiers;
      unsigned count;
   };
};

/**
 * An allocated GBM surface.
 *
 * To modify this structure, introduce a new gbm_surface_v<N> structure, add it
 * to the end of this structure, and increment GBM_BACKEND_ABI_VERSION.
 */
struct gbm_surface {
   struct gbm_device *gbm;
   struct gbm_surface_v0 v0;
};

/**
 * GBM backend interfaces corresponding to GBM_BACKEND_ABI_VERSION = 0
 *
 * DO NOT MODIFY THIS STRUCT. Instead, introduce a gbm_backend_v1, increment
 * GBM_BACKEND_ABI_VERSION, append gbm_backend_v1 to gbm_backend.
 */
struct gbm_backend_v0 {
   /**
    * The version of the GBM backend interface supported by this backend. This
    * is set by the backend itself, and may be greater or less than the version
    * supported by the loader. It is the responsibility of the GBM loader to
    * respect this version when accessing fields in this structure.
    */
   uint32_t backend_version;

   const char *backend_name;
   struct gbm_device *(*create_device)(int fd, uint32_t gbm_backend_version);
};

/**
 * The interface exposed by an external GBM backend.
 *
 * To modify this structure, introduce a new gbm_backend_v<N> structure, add it
 * to the end of this structure, and increment GBM_BACKEND_ABI_VERSION.
 */
struct gbm_backend {
   struct gbm_backend_v0 v0;
};

/**
 * GBM interfaces exposed to GBM backends at GBM_BACKEND_ABI_VERSION >= 0
 *
 * DO NOT MODIFY THIS STRUCT. Instead, introduce a gbm_core_v1, increment
 * GBM_BACKEND_ABI_VERSION, and append gbm_core_v1 to gbm_backend.
 */
struct gbm_core_v0 {
   /**
    * The version of the GBM backend interface supported by the GBM loader. This
    * is set by the loader, and may be greater or less than the version
    * supported by a given backend. It is the responsibility of the backend to
    * respect this version when accessing fields in this structure and other
    * structures allocated or modified by the loader.
    */
   uint32_t core_version;

   uint32_t (*format_canonicalize)(uint32_t gbm_format);
};

/**
 * The interface exposed by the GBM core/loader code to GBM backends.
 *
 * To modify this structure, introduce a new gbm_core_v<N> structure, add it
 * to the end of this structure, and increment GBM_BACKEND_ABI_VERSION.
 */
struct gbm_core {
   struct gbm_core_v0 v0;
};

/**
 * The entrypoint an external GBM backend exports.
 *
 * Prior to creating any devices using the backend, GBM will look up and call
 * this function to request the backend's interface and convey the loader's
 * version and exported interface to the backend.
 *
 * DO NOT MODIFY THIS FUNCTION NAME OR PROTOTYPE. It must remain unchanged to
 * preserve backwards compatibility with existing GBM backends.
 */
#define GBM_GET_BACKEND_PROC gbmint_get_backend
#define _GBM_MKSTRX(s) _GBM_MKSTR(s)
#define _GBM_MKSTR(s) #s
#define GBM_GET_BACKEND_PROC_NAME _GBM_MKSTRX(GBM_GET_BACKEND_PROC)
typedef const struct gbm_backend *(*GBM_GET_BACKEND_PROC_PTR)(const struct gbm_core *gbm_core);

#endif
