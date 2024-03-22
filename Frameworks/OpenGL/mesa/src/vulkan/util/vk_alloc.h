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
#ifndef VK_ALLOC_H
#define VK_ALLOC_H

/* common allocation inlines for vulkan drivers */

#include <stdio.h>
#include <string.h>
#include <vulkan/vulkan_core.h>

#include "util/u_math.h"
#include "util/macros.h"
#include "util/u_printf.h"

#ifdef __cplusplus
extern "C" {
#endif

const VkAllocationCallbacks *
vk_default_allocator(void);

static inline void *
vk_alloc(const VkAllocationCallbacks *alloc,
         size_t size, size_t align,
         VkSystemAllocationScope scope)
{
   return alloc->pfnAllocation(alloc->pUserData, size, align, scope);
}

static inline void *
vk_zalloc(const VkAllocationCallbacks *alloc,
          size_t size, size_t align,
          VkSystemAllocationScope scope)
{
   void *mem = vk_alloc(alloc, size, align, scope);
   if (mem == NULL)
      return NULL;

   memset(mem, 0, size);

   return mem;
}

static inline void *
vk_realloc(const VkAllocationCallbacks *alloc,
           void *ptr, size_t size, size_t align,
           VkSystemAllocationScope scope)
{
   return alloc->pfnReallocation(alloc->pUserData, ptr, size, align, scope);
}

static inline void
vk_free(const VkAllocationCallbacks *alloc, void *data)
{
   if (data == NULL)
      return;

   alloc->pfnFree(alloc->pUserData, data);
}

static inline char *
vk_strdup(const VkAllocationCallbacks *alloc, const char *s,
          VkSystemAllocationScope scope)
{
   if (s == NULL)
      return NULL;

   size_t size = strlen(s) + 1;
   char *copy = (char *)vk_alloc(alloc, size, 1, scope);
   if (copy == NULL)
      return NULL;

   memcpy(copy, s, size);

   return copy;
}

static inline char *
vk_vasprintf(const VkAllocationCallbacks *alloc,
             VkSystemAllocationScope scope,
             const char *fmt, va_list args)
{
   size_t size = u_printf_length(fmt, args) + 1;
   char *ptr = (char *)vk_alloc(alloc, size, 1, scope);
   if (ptr != NULL)
      vsnprintf(ptr, size, fmt, args);

   return ptr;
}

PRINTFLIKE(3, 4) static inline char *
vk_asprintf(const VkAllocationCallbacks *alloc,
            VkSystemAllocationScope scope,
            const char *fmt, ...)
{
   va_list args;
   va_start(args, fmt);
   char *ptr = vk_vasprintf(alloc, scope, fmt, args);
   va_end(args);

   return ptr;
}

static inline void *
vk_alloc2(const VkAllocationCallbacks *parent_alloc,
          const VkAllocationCallbacks *alloc,
          size_t size, size_t align,
          VkSystemAllocationScope scope)
{
   if (alloc)
      return vk_alloc(alloc, size, align, scope);
   else
      return vk_alloc(parent_alloc, size, align, scope);
}

static inline void *
vk_zalloc2(const VkAllocationCallbacks *parent_alloc,
           const VkAllocationCallbacks *alloc,
           size_t size, size_t align,
           VkSystemAllocationScope scope)
{
   void *mem = vk_alloc2(parent_alloc, alloc, size, align, scope);
   if (mem == NULL)
      return NULL;

   memset(mem, 0, size);

   return mem;
}

static inline void
vk_free2(const VkAllocationCallbacks *parent_alloc,
         const VkAllocationCallbacks *alloc,
         void *data)
{
   if (alloc)
      vk_free(alloc, data);
   else
      vk_free(parent_alloc, data);
}

/* A multi-pointer allocator
 *
 * When copying data structures from the user (such as a render pass), it's
 * common to need to allocate data for a bunch of different things.  Instead
 * of doing several allocations and having to handle all of the error checking
 * that entails, it can be easier to do a single allocation.  This struct
 * helps facilitate that.  The intended usage looks like this:
 *
 *    VK_MULTIALLOC(ma)
 *    vk_multialloc_add(&ma, &main_ptr, 1);
 *    vk_multialloc_add(&ma, &substruct1, substruct1Count);
 *    vk_multialloc_add(&ma, &substruct2, substruct2Count);
 *
 *    if (!vk_multialloc_alloc(&ma, pAllocator, VK_ALLOCATION_SCOPE_FOO))
 *       return vk_error(VK_ERROR_OUT_OF_HOST_MEORY);
 */
struct vk_multialloc {
    size_t size;
    size_t align;

    uint32_t ptr_count;
    void **ptrs[12];
};

#define VK_MULTIALLOC(_name) \
   struct vk_multialloc _name = { .align = 1 }

static ALWAYS_INLINE void
vk_multialloc_add_size_align(struct vk_multialloc *ma,
                             void **ptr, size_t size, size_t align)
{
   assert(util_is_power_of_two_nonzero(align));
   if (size == 0) {
      *ptr = NULL;
      return;
   }

   size_t offset = ALIGN_POT(ma->size, align);
   ma->size = offset + size;
   ma->align = MAX2(ma->align, align);

   /* Store the offset in the pointer. */
   *ptr = (void *)(uintptr_t)offset;

   assert(ma->ptr_count < ARRAY_SIZE(ma->ptrs));
   ma->ptrs[ma->ptr_count++] = ptr;
}

#define vk_multialloc_add_size(_ma, _ptr, _type, _size) \
   do { \
      _type **_tmp = (_ptr); \
      (void)_tmp; \
      vk_multialloc_add_size_align((_ma), (void **)(_ptr), \
                                   (_size), alignof(_type)); \
   } while(0)

#define vk_multialloc_add(_ma, _ptr, _type, _count) \
   vk_multialloc_add_size(_ma, _ptr, _type, (_count) * sizeof(**(_ptr)));

#define VK_MULTIALLOC_DECL_SIZE(_ma, _type, _name, _size) \
   _type *_name; \
   vk_multialloc_add_size(_ma, &_name, _type, _size);

#define VK_MULTIALLOC_DECL(_ma, _type, _name, _count) \
   VK_MULTIALLOC_DECL_SIZE(_ma, _type, _name, (_count) * sizeof(_type));

static ALWAYS_INLINE void *
vk_multialloc_alloc(struct vk_multialloc *ma,
                    const VkAllocationCallbacks *alloc,
                    VkSystemAllocationScope scope)
{
   void *ptr = vk_alloc(alloc, ma->size, ma->align, scope);
   if (!ptr)
      return NULL;

   /* Fill out each of the pointers with their final value.
    *
    *   for (uint32_t i = 0; i < ma->ptr_count; i++)
    *      *ma->ptrs[i] = ptr + (uintptr_t)*ma->ptrs[i];
    *
    * Unfortunately, even though ma->ptr_count is basically guaranteed to be a
    * constant, GCC is incapable of figuring this out and unrolling the loop
    * so we have to give it a little help.
    */
   STATIC_ASSERT(ARRAY_SIZE(ma->ptrs) == 12);
#define _VK_MULTIALLOC_UPDATE_POINTER(_i) \
   if ((_i) < ma->ptr_count) \
      *ma->ptrs[_i] = (char *)ptr + (uintptr_t)*ma->ptrs[_i]
   _VK_MULTIALLOC_UPDATE_POINTER(0);
   _VK_MULTIALLOC_UPDATE_POINTER(1);
   _VK_MULTIALLOC_UPDATE_POINTER(2);
   _VK_MULTIALLOC_UPDATE_POINTER(3);
   _VK_MULTIALLOC_UPDATE_POINTER(4);
   _VK_MULTIALLOC_UPDATE_POINTER(5);
   _VK_MULTIALLOC_UPDATE_POINTER(6);
   _VK_MULTIALLOC_UPDATE_POINTER(7);
   _VK_MULTIALLOC_UPDATE_POINTER(8);
   _VK_MULTIALLOC_UPDATE_POINTER(9);
   _VK_MULTIALLOC_UPDATE_POINTER(10);
   _VK_MULTIALLOC_UPDATE_POINTER(11);
#undef _VK_MULTIALLOC_UPDATE_POINTER

   return ptr;
}

static ALWAYS_INLINE void *
vk_multialloc_alloc2(struct vk_multialloc *ma,
                     const VkAllocationCallbacks *parent_alloc,
                     const VkAllocationCallbacks *alloc,
                     VkSystemAllocationScope scope)
{
   return vk_multialloc_alloc(ma, alloc ? alloc : parent_alloc, scope);
}

static ALWAYS_INLINE void *
vk_multialloc_zalloc(struct vk_multialloc *ma,
                     const VkAllocationCallbacks *alloc,
                     VkSystemAllocationScope scope)
{
   void *ptr = vk_multialloc_alloc(ma, alloc, scope);

   if (ptr == NULL)
      return NULL;

   memset(ptr, 0, ma->size);

   return ptr;
}

static ALWAYS_INLINE void *
vk_multialloc_zalloc2(struct vk_multialloc *ma,
                      const VkAllocationCallbacks *parent_alloc,
                      const VkAllocationCallbacks *alloc,
                      VkSystemAllocationScope scope)
{
   return vk_multialloc_zalloc(ma, alloc ? alloc : parent_alloc, scope);
}

#ifdef __cplusplus
}
#endif

#endif
