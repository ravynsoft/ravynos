/*
 * Copyright © 2022 Collabora, Ltd.
 * SPDX-License-Identifier: MIT
 */
#ifndef NVK_PRIVATE_H
#define NVK_PRIVATE_H 1

#include <assert.h>

#include "vk_log.h"
#include "vk_util.h"

#define NVK_MAX_SETS 8
#define NVK_MAX_PUSH_SIZE 128
#define NVK_MAX_DYNAMIC_BUFFERS 64
#define NVK_MAX_RTS 8
#define NVK_MIN_SSBO_ALIGNMENT 16
#define NVK_MIN_TEXEL_BUFFER_ALIGNMENT 16
#define NVK_MIN_UBO_ALIGNMENT 64
#define NVK_MAX_VIEWPORTS 16
#define NVK_MAX_DESCRIPTOR_SIZE 16
#define NVK_MAX_PUSH_DESCRIPTORS 32
#define NVK_MAX_DESCRIPTOR_SET_SIZE (1u << 30)
#define NVK_MAX_DESCRIPTORS (1 << 20)
#define NVK_PUSH_DESCRIPTOR_SET_SIZE \
   (NVK_MAX_PUSH_DESCRIPTORS * NVK_MAX_DESCRIPTOR_SIZE)
#define NVK_SSBO_BOUNDS_CHECK_ALIGNMENT 4
#define NVK_MAX_MULTIVIEW_VIEW_COUNT 32

#define NVK_SPARSE_ADDR_SPACE_SIZE (1ull << 39)
#define NVK_MAX_BUFFER_SIZE (1ull << 31)

/* Max size of a bound cbuf */
#define NVK_MAX_CBUF_SIZE (1u << 16)

struct nvk_addr_range {
   uint64_t addr;
   uint64_t range;
};

/**
 * Warn on ignored extension structs.
 *
 * The Vulkan spec requires us to ignore unsupported or unknown structs in
 * a pNext chain.  In debug mode, emitting warnings for ignored structs may
 * help us discover structs that we should not have ignored.
 *
 *
 * From the Vulkan 1.0.38 spec:
 *
 *    Any component of the implementation (the loader, any enabled layers,
 *    and drivers) must skip over, without processing (other than reading the
 *    sType and pNext members) any chained structures with sType values not
 *    defined by extensions supported by that component.
 */
#define nvk_debug_ignored_stype(sType)                                                             \
   mesa_logd("%s: ignored VkStructureType %u\n", __func__, (sType))

#endif
