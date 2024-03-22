/*
 * Copyright © 2015 Intel Corporation
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
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include <sys/mman.h>
#include <sys/syscall.h>

#include "util/anon_file.h"
#include "anv_private.h"

uint32_t
anv_gem_create(struct anv_device *device, uint64_t size)
{
   int fd = os_create_anonymous_file(size, "fake bo");
   if (fd == -1)
      return 0;

   assert(fd != 0);

   return fd;
}

void
anv_gem_close(struct anv_device *device, uint32_t gem_handle)
{
   close(gem_handle);
}

void*
anv_gem_mmap(struct anv_device *device, uint32_t gem_handle,
             uint64_t offset, uint64_t size, uint32_t flags)
{
   /* Ignore flags, as they're specific to I915_GEM_MMAP. */
   (void) flags;

   return mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED,
               gem_handle, offset);
}

/* This is just a wrapper around munmap, but it also notifies valgrind that
 * this map is no longer valid.  Pair this with anv_gem_mmap().
 */
void
anv_gem_munmap(struct anv_device *device, void *p, uint64_t size)
{
   munmap(p, size);
}

uint32_t
anv_gem_userptr(struct anv_device *device, void *mem, size_t size)
{
   int fd = os_create_anonymous_file(size, "fake bo");
   if (fd == -1)
      return 0;

   assert(fd != 0);

   return fd;
}

int
anv_gem_wait(struct anv_device *device, uint32_t gem_handle, int64_t *timeout_ns)
{
   return 0;
}

int
anv_gem_execbuffer(struct anv_device *device,
                   struct drm_i915_gem_execbuffer2 *execbuf)
{
   return 0;
}

int
anv_gem_set_tiling(struct anv_device *device,
                   uint32_t gem_handle, uint32_t stride, uint32_t tiling)
{
   return 0;
}

int
anv_gem_get_tiling(struct anv_device *device, uint32_t gem_handle)
{
   return 0;
}

int
anv_gem_set_caching(struct anv_device *device, uint32_t gem_handle,
                    uint32_t caching)
{
   return 0;
}

int
anv_gem_set_context_param(int fd, uint32_t context, uint32_t param, uint64_t value)
{
   unreachable("Unused");
}

bool
anv_gem_has_context_priority(int fd, int priority)
{
   unreachable("Unused");
}

int
anv_gem_context_get_reset_stats(int fd, int context,
                                uint32_t *active, uint32_t *pending)
{
   unreachable("Unused");
}

int
anv_gem_handle_to_fd(struct anv_device *device, uint32_t gem_handle)
{
   unreachable("Unused");
}

uint32_t
anv_gem_fd_to_handle(struct anv_device *device, int fd)
{
   unreachable("Unused");
}
