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

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <assert.h>
#include <dlfcn.h>
#include <xf86drm.h>

#include "loader.h"
#include "backend.h"

#define ARRAY_SIZE(a) (sizeof(a)/sizeof((a)[0]))
#define VER_MIN(a, b) ((a) < (b) ? (a) : (b))

#if defined(HAVE_DRI) || defined(HAVE_DRI2) || defined(HAVE_DRI3)
extern const struct gbm_backend gbm_dri_backend;
#endif

struct gbm_backend_desc {
   const char *name;
   const struct gbm_backend *backend;
   void *lib;
};

static const struct gbm_backend_desc builtin_backends[] = {
#if defined(HAVE_DRI) || defined(HAVE_DRI2) || defined(HAVE_DRI3)
   { "dri", &gbm_dri_backend },
#endif
};

#define BACKEND_LIB_SUFFIX "_gbm"
static const char *backend_search_path_vars[] = {
   "GBM_BACKENDS_PATH",
   NULL
};

static void
free_backend_desc(const struct gbm_backend_desc *backend_desc)
{
   assert(backend_desc->lib);

   dlclose(backend_desc->lib);
   free((void *)backend_desc->name);
   free((void *)backend_desc);
}

static struct gbm_backend_desc *
create_backend_desc(const char *name,
                    const struct gbm_backend *backend,
                    void *lib)
{
   struct gbm_backend_desc *new_desc = calloc(1, sizeof(*new_desc));

   if (!new_desc)
      return NULL;

   new_desc->name = strdup(name);

   if (!new_desc->name) {
      free(new_desc);
      return NULL;
   }

   new_desc->backend = backend;
   new_desc->lib = lib;

   return new_desc;
}

static struct gbm_device *
backend_create_device(const struct gbm_backend_desc *bd, int fd)
{
   const uint32_t abi_ver = VER_MIN(GBM_BACKEND_ABI_VERSION,
                                    bd->backend->v0.backend_version);
   struct gbm_device *dev = bd->backend->v0.create_device(fd, abi_ver);

   if (dev) {
      if (abi_ver != dev->v0.backend_version) {
         _gbm_device_destroy(dev);
         return NULL;
      }
      dev->v0.backend_desc = bd;
   }

   return dev;
}

static struct gbm_device *
load_backend(void *lib, int fd, const char *name)
{
   struct gbm_device *dev = NULL;
   struct gbm_backend_desc *backend_desc;
   const struct gbm_backend *gbm_backend;
   GBM_GET_BACKEND_PROC_PTR get_backend;

   get_backend = dlsym(lib, GBM_GET_BACKEND_PROC_NAME);

   if (!get_backend)
      goto fail;

   gbm_backend = get_backend(&gbm_core);
   backend_desc = create_backend_desc(name, gbm_backend, lib);

   if (!backend_desc)
      goto fail;

   dev = backend_create_device(backend_desc, fd);

   if (!dev)
      free_backend_desc(backend_desc);

   return dev;

fail:
   dlclose(lib);
   return NULL;
}

static struct gbm_device *
find_backend(const char *name, int fd)
{
   struct gbm_device *dev = NULL;
   const struct gbm_backend_desc *bd;
   void *lib;
   unsigned i;

   for (i = 0; i < ARRAY_SIZE(builtin_backends); ++i) {
      bd = &builtin_backends[i];

      if (name && strcmp(bd->name, name))
         continue;

      dev = backend_create_device(bd, fd);

      if (dev)
         break;
   }

   if (name && !dev) {
      lib = loader_open_driver_lib(name, BACKEND_LIB_SUFFIX,
                                   backend_search_path_vars,
                                   DEFAULT_BACKENDS_PATH,
                                   true);

      if (lib)
         dev = load_backend(lib, fd, name);
   }

   return dev;
}

static struct gbm_device *
override_backend(int fd)
{
   struct gbm_device *dev = NULL;
   const char *b;

   b = getenv("GBM_BACKEND");
   if (b)
      dev = find_backend(b, fd);

   return dev;
}

static struct gbm_device *
backend_from_driver_name(int fd)
{
   struct gbm_device *dev = NULL;
   drmVersionPtr v = drmGetVersion(fd);
   void *lib;

   if (!v)
      return NULL;

   lib = loader_open_driver_lib(v->name, BACKEND_LIB_SUFFIX,
                                backend_search_path_vars,
                                DEFAULT_BACKENDS_PATH,
                                false);

   if (lib)
      dev = load_backend(lib, fd, v->name);

   drmFreeVersion(v);

   return dev;
}

struct gbm_device *
_gbm_create_device(int fd)
{
   struct gbm_device *dev;

   dev = override_backend(fd);

   if (!dev)
      dev = backend_from_driver_name(fd);

   if (!dev)
      dev = find_backend(NULL, fd);

   return dev;
}

void
_gbm_device_destroy(struct gbm_device *gbm)
{
   const struct gbm_backend_desc *backend_desc = gbm->v0.backend_desc;
   gbm->v0.destroy(gbm);

   if (backend_desc && backend_desc->lib)
      free_backend_desc(backend_desc);
}
