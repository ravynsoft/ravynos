/*
 * Copyright © 2017 Intel Corporation
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
#ifndef VK_UTIL_H
#define VK_UTIL_H

#include "util/bitscan.h"
#include "util/macros.h"
#include "compiler/shader_enums.h"
#include <stdlib.h>
#include <string.h>

#include "vk_struct_type_cast.h"

#ifdef __cplusplus
extern "C" {
#endif

/* common inlines and macros for vulkan drivers */

#include <vulkan/vulkan_core.h>

struct vk_pnext_iterator {
   VkBaseOutStructure *pos;
#ifndef NDEBUG
   VkBaseOutStructure *half_pos;
   unsigned idx;
#endif
   bool done;
};

static inline struct vk_pnext_iterator
vk_pnext_iterator_init(void *start)
{
   struct vk_pnext_iterator iter;

   iter.pos = (VkBaseOutStructure *)start;
#ifndef NDEBUG
   iter.half_pos = (VkBaseOutStructure *)start;
   iter.idx = 0;
#endif
   iter.done = false;

   return iter;
}

static inline struct vk_pnext_iterator
vk_pnext_iterator_init_const(const void *start)
{
   return vk_pnext_iterator_init((void *)start);
}

static inline VkBaseOutStructure *
vk_pnext_iterator_next(struct vk_pnext_iterator *iter)
{
   iter->pos = iter->pos->pNext;

#ifndef NDEBUG
   if (iter->idx++ & 1) {
      /** This the "tortoise and the hare" algorithm.  We increment
       * chaser->pNext every other time *iter gets incremented.  Because *iter
       * is incrementing twice as fast as chaser->pNext, the distance between
       * them in the list increases by one for each time we get here.  If we
       * have a loop, eventually, both iterators will be inside the loop and
       * this distance will be an integer multiple of the loop length, at
       * which point the two pointers will be equal.
       */
      iter->half_pos = iter->half_pos->pNext;
      if (iter->half_pos == iter->pos)
         assert(!"Vulkan input pNext chain has a loop!");
   }
#endif

   return iter->pos;
}

/* Because the outer loop only executes once, independently of what happens in
 * the inner loop, breaks and continues should work exactly the same as if
 * there were only one for loop.
 */
#define vk_foreach_struct(__e, __start) \
   for (struct vk_pnext_iterator __iter = vk_pnext_iterator_init(__start); \
        !__iter.done; __iter.done = true) \
      for (VkBaseOutStructure *__e = __iter.pos; \
           __e; __e = vk_pnext_iterator_next(&__iter))

#define vk_foreach_struct_const(__e, __start) \
   for (struct vk_pnext_iterator __iter = \
            vk_pnext_iterator_init_const(__start); \
        !__iter.done; __iter.done = true) \
      for (const VkBaseInStructure *__e = (VkBaseInStructure *)__iter.pos; \
           __e; __e = (VkBaseInStructure *)vk_pnext_iterator_next(&__iter))

static inline void
vk_copy_struct_guts(VkBaseOutStructure *dst, VkBaseInStructure *src, size_t struct_size)
{
   STATIC_ASSERT(sizeof(*dst) == sizeof(*src));
   memcpy(dst + 1, src + 1, struct_size - sizeof(VkBaseOutStructure));
}

/**
 * A wrapper for a Vulkan output array. A Vulkan output array is one that
 * follows the convention of the parameters to
 * vkGetPhysicalDeviceQueueFamilyProperties().
 *
 * Example Usage:
 *
 *    VkResult
 *    vkGetPhysicalDeviceQueueFamilyProperties(
 *       VkPhysicalDevice           physicalDevice,
 *       uint32_t*                  pQueueFamilyPropertyCount,
 *       VkQueueFamilyProperties*   pQueueFamilyProperties)
 *    {
 *       VK_OUTARRAY_MAKE_TYPED(VkQueueFamilyProperties, props,
 *                              pQueueFamilyProperties,
 *                              pQueueFamilyPropertyCount);
 *
 *       vk_outarray_append_typed(VkQueueFamilyProperties, &props, p) {
 *          p->queueFlags = ...;
 *          p->queueCount = ...;
 *       }
 *
 *       vk_outarray_append_typed(VkQueueFamilyProperties, &props, p) {
 *          p->queueFlags = ...;
 *          p->queueCount = ...;
 *       }
 *
 *       return vk_outarray_status(&props);
 *    }
 */
struct __vk_outarray {
   /** May be null. */
   void *data;

   /**
    * Capacity, in number of elements. Capacity is unlimited (UINT32_MAX) if
    * data is null.
    */
   uint32_t cap;

   /**
    * Count of elements successfully written to the array. Every write is
    * considered successful if data is null.
    */
   uint32_t *filled_len;

   /**
    * Count of elements that would have been written to the array if its
    * capacity were sufficient. Vulkan functions often return VK_INCOMPLETE
    * when `*filled_len < wanted_len`.
    */
   uint32_t wanted_len;
};

static inline void
__vk_outarray_init(struct __vk_outarray *a,
                   void *data, uint32_t *restrict len)
{
   a->data = data;
   a->cap = *len;
   a->filled_len = len;
   *a->filled_len = 0;
   a->wanted_len = 0;

   if (a->data == NULL)
      a->cap = UINT32_MAX;
}

static inline VkResult
__vk_outarray_status(const struct __vk_outarray *a)
{
   if (*a->filled_len < a->wanted_len)
      return VK_INCOMPLETE;
   else
      return VK_SUCCESS;
}

static inline void *
__vk_outarray_next(struct __vk_outarray *a, size_t elem_size)
{
   void *p = NULL;

   a->wanted_len += 1;

   if (*a->filled_len >= a->cap)
      return NULL;

   if (a->data != NULL)
      p = (uint8_t *)a->data + (*a->filled_len) * elem_size;

   *a->filled_len += 1;

   return p;
}

#define vk_outarray(elem_t) \
   struct { \
      struct __vk_outarray base; \
      elem_t meta[]; \
   }

#define vk_outarray_typeof_elem(a) __typeof__((a)->meta[0])
#define vk_outarray_sizeof_elem(a) sizeof((a)->meta[0])

#define vk_outarray_init(a, data, len) \
   __vk_outarray_init(&(a)->base, (data), (len))

#define VK_OUTARRAY_MAKE_TYPED(type, name, data, len) \
   vk_outarray(type) name; \
   vk_outarray_init(&name, (data), (len))

#define vk_outarray_status(a) \
   __vk_outarray_status(&(a)->base)

#define vk_outarray_next(a) \
   vk_outarray_next_typed(vk_outarray_typeof_elem(a), a)
#define vk_outarray_next_typed(type, a) \
   ((type *) \
      __vk_outarray_next(&(a)->base, vk_outarray_sizeof_elem(a)))

/**
 * Append to a Vulkan output array.
 *
 * This is a block-based macro. For example:
 *
 *    vk_outarray_append_typed(T, &a, elem) {
 *       elem->foo = ...;
 *       elem->bar = ...;
 *    }
 *
 * The array `a` has type `vk_outarray(elem_t) *`. It is usually declared with
 * VK_OUTARRAY_MAKE_TYPED(). The variable `elem` is block-scoped and has type
 * `elem_t *`.
 *
 * The macro unconditionally increments the array's `wanted_len`. If the array
 * is not full, then the macro also increment its `filled_len` and then
 * executes the block. When the block is executed, `elem` is non-null and
 * points to the newly appended element.
 */
#define vk_outarray_append_typed(type, a, elem) \
   for (type *elem = vk_outarray_next_typed(type, a); \
        elem != NULL; elem = NULL)

static inline void *
__vk_find_struct(void *start, VkStructureType sType)
{
   vk_foreach_struct(s, start) {
      if (s->sType == sType)
         return s;
   }

   return NULL;
}

#define vk_find_struct(__start, __sType)                                       \
  (VK_STRUCTURE_TYPE_##__sType##_cast *)__vk_find_struct(                      \
      (__start), VK_STRUCTURE_TYPE_##__sType)

#define vk_find_struct_const(__start, __sType)                                 \
  (const VK_STRUCTURE_TYPE_##__sType##_cast *)__vk_find_struct(                \
      (void *)(__start), VK_STRUCTURE_TYPE_##__sType)

static inline void
__vk_append_struct(void *start, void *element)
{
   vk_foreach_struct(s, start) {
      if (s->pNext)
         continue;

      s->pNext = (struct VkBaseOutStructure *) element;
      break;
   }
}

uint32_t vk_get_driver_version(void);

uint32_t vk_get_version_override(void);

void vk_warn_non_conformant_implementation(const char *driver_name);

struct vk_pipeline_cache_header {
   uint32_t header_size;
   uint32_t header_version;
   uint32_t vendor_id;
   uint32_t device_id;
   uint8_t  uuid[VK_UUID_SIZE];
};

#define VK_EXT_OFFSET (1000000000UL)
#define VK_ENUM_EXTENSION(__enum) \
   ((__enum) >= VK_EXT_OFFSET ? ((((__enum) - VK_EXT_OFFSET) / 1000UL) + 1) : 0)
#define VK_ENUM_OFFSET(__enum) \
   ((__enum) >= VK_EXT_OFFSET ? ((__enum) % 1000) : (__enum))

#define typed_memcpy(dest, src, count) do { \
   STATIC_ASSERT(sizeof(*(src)) == sizeof(*(dest))); \
   memcpy((dest), (src), (count) * sizeof(*(src))); \
} while (0)

void
vk_compiler_cache_init(void);

void
vk_compiler_cache_finish(void);

static inline gl_shader_stage
vk_to_mesa_shader_stage(VkShaderStageFlagBits vk_stage)
{
   assert(util_bitcount((uint32_t) vk_stage) == 1);
   return (gl_shader_stage) (ffs((uint32_t) vk_stage) - 1);
}

static inline VkShaderStageFlagBits
mesa_to_vk_shader_stage(gl_shader_stage mesa_stage)
{
   return (VkShaderStageFlagBits) (1 << ((uint32_t) mesa_stage));
}

/* iterate over a sequence of indexed multidraws for VK_EXT_multi_draw extension */
/* 'i' must be explicitly declared */
#define vk_foreach_multi_draw_indexed(_draw, _i, _pDrawInfo, _num_draws, _stride) \
   for (const VkMultiDrawIndexedInfoEXT *_draw = (const VkMultiDrawIndexedInfoEXT*)(_pDrawInfo); \
        (_i) < (_num_draws); \
        (_i)++, (_draw) = (const VkMultiDrawIndexedInfoEXT*)((const uint8_t*)(_draw) + (_stride)))

/* iterate over a sequence of multidraws for VK_EXT_multi_draw extension */
/* 'i' must be explicitly declared */
#define vk_foreach_multi_draw(_draw, _i, _pDrawInfo, _num_draws, _stride) \
   for (const VkMultiDrawInfoEXT *_draw = (const VkMultiDrawInfoEXT*)(_pDrawInfo); \
        (_i) < (_num_draws); \
        (_i)++, (_draw) = (const VkMultiDrawInfoEXT*)((const uint8_t*)(_draw) + (_stride)))


struct nir_spirv_specialization;

struct nir_spirv_specialization*
vk_spec_info_to_nir_spirv(const VkSpecializationInfo *spec_info,
                          uint32_t *out_num_spec_entries);

#define STACK_ARRAY_SIZE 8

#ifdef __cplusplus
#define STACK_ARRAY_ZERO_INIT {}
#else
#define STACK_ARRAY_ZERO_INIT {0}
#endif

#define STACK_ARRAY(type, name, size) \
   type _stack_##name[STACK_ARRAY_SIZE] = STACK_ARRAY_ZERO_INIT; \
   type *const name = \
     ((size) <= STACK_ARRAY_SIZE ? _stack_##name : (type *)malloc((size) * sizeof(type)))

#define STACK_ARRAY_FINISH(name) \
   if (name != _stack_##name) free(name)

static inline uint8_t
vk_index_type_to_bytes(enum VkIndexType type)
{
   switch (type) {
   case VK_INDEX_TYPE_NONE_KHR:  return 0;
   case VK_INDEX_TYPE_UINT8_EXT: return 1;
   case VK_INDEX_TYPE_UINT16:    return 2;
   case VK_INDEX_TYPE_UINT32:    return 4;
   default:                      unreachable("Invalid index type");
   }
}

#ifdef __cplusplus
}
#endif

#endif /* VK_UTIL_H */
