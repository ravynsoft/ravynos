/*
 * Copyright 2021 Google LLC
 * SPDX-License-Identifier: MIT
 */

#include "vk_alloc.h"

#include <stdlib.h>

#ifndef _MSC_VER
#include <stddef.h>
#define MAX_ALIGN alignof(max_align_t)
#else
/* long double might be 128-bit, but our callers do not need that anyway(?) */
#include <stdint.h>
#define MAX_ALIGN alignof(uint64_t)
#endif

static VKAPI_ATTR void * VKAPI_CALL
vk_default_alloc(void *pUserData,
                 size_t size,
                 size_t alignment,
                 VkSystemAllocationScope allocationScope)
{
   assert(MAX_ALIGN % alignment == 0);
   return malloc(size);
}

static VKAPI_ATTR void * VKAPI_CALL
vk_default_realloc(void *pUserData,
                   void *pOriginal,
                   size_t size,
                   size_t alignment,
                   VkSystemAllocationScope allocationScope)
{
   assert(MAX_ALIGN % alignment == 0);
   return realloc(pOriginal, size);
}

static VKAPI_ATTR void VKAPI_CALL
vk_default_free(void *pUserData, void *pMemory)
{
   free(pMemory);
}

const VkAllocationCallbacks *
vk_default_allocator(void)
{
   static const VkAllocationCallbacks allocator = {
      .pfnAllocation = vk_default_alloc,
      .pfnReallocation = vk_default_realloc,
      .pfnFree = vk_default_free,
   };
   return &allocator;
}
