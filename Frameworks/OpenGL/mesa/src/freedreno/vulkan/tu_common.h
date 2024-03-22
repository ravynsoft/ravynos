/*
 * Copyright © 2016 Red Hat.
 * Copyright © 2016 Bas Nieuwenhuizen
 * SPDX-License-Identifier: MIT
 *
 * based in part on anv driver which is:
 * Copyright © 2015 Intel Corporation
 */

#ifndef TU_COMMON_H
#define TU_COMMON_H

#include <assert.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tuple>
#ifdef HAVE_VALGRIND
#include <memcheck.h>
#include <valgrind.h>
#define VG(x) x
#else
#define VG(x) ((void)0)
#endif

#define MESA_LOG_TAG "TU"

#include "c11/threads.h"
#include "util/rounding.h"
#include "util/bitscan.h"
#include "util/list.h"
#include "util/log.h"
#include "util/macros.h"
#include "util/perf/cpu_trace.h"
#include "util/sparse_array.h"
#include "util/u_atomic.h"
#include "util/u_dynarray.h"
#include "util/xmlconfig.h"
#include "util/perf/u_trace.h"
#include "vk_alloc.h"
#include "vk_debug_report.h"
#include "vk_device.h"
#include "vk_dispatch_table.h"
#include "vk_extensions.h"
#include "vk_instance.h"
#include "vk_log.h"
#include "vk_physical_device.h"
#include "vk_pipeline_cache.h"
#include "wsi_common.h"

#include "ir3/ir3_compiler.h"
#include "ir3/ir3_shader.h"

#include "adreno_common.xml.h"
#include "adreno_pm4.xml.h"
#include "a6xx.xml.h"
#include "fdl/freedreno_layout.h"
#include "common/freedreno_dev_info.h"
#include "common/freedreno_common.h"
#include "perfcntrs/freedreno_perfcntr.h"

#include <vulkan/vk_android_native_buffer.h>
#include <vulkan/vk_icd.h>
#include <vulkan/vulkan.h>

#include "tu_entrypoints.h"
#include "vulkan/runtime/vk_common_entrypoints.h"

#include "vk_format.h"
#include "vk_image.h"
#include "vk_command_buffer.h"
#include "vk_command_pool.h"
#include "vk_queue.h"
#include "vk_object.h"
#include "vk_sync.h"
#include "vk_drm_syncobj.h"
#include "vk_sync_timeline.h"

#define MAX_VBS 32
#define MAX_VERTEX_ATTRIBS 32
#define MAX_RTS 8
#define MAX_VSC_PIPES 32
#define MAX_VIEWPORTS 16
#define MAX_VIEWPORT_SIZE (1 << 14)
#define MAX_SCISSORS 16
#define MAX_DISCARD_RECTANGLES 4
#define MAX_PUSH_CONSTANTS_SIZE 256
#define MAX_PUSH_DESCRIPTORS 32
#define MAX_DYNAMIC_UNIFORM_BUFFERS 16
#define MAX_DYNAMIC_STORAGE_BUFFERS 8
#define MAX_DYNAMIC_BUFFERS_SIZE                                             \
   (MAX_DYNAMIC_UNIFORM_BUFFERS + 2 * MAX_DYNAMIC_STORAGE_BUFFERS) *         \
   A6XX_TEX_CONST_DWORDS

#define SAMPLE_LOCATION_MIN 0.f
#define SAMPLE_LOCATION_MAX 0.9375f

#define TU_MAX_DRM_DEVICES 8
#define MAX_VIEWS 16
#define MAX_BIND_POINTS 2 /* compute + graphics */
/* match the latest Qualcomm driver which is also a hw limit on later gens */
#define MAX_STORAGE_BUFFER_RANGE (1u << 27)
/* We use ldc for uniform buffer loads, just like the Qualcomm driver, so
 * expose the same maximum range.
 * TODO: The SIZE bitfield is 15 bits, and in 4-dword units, so the actual
 * range might be higher.
 */
#define MAX_UNIFORM_BUFFER_RANGE 0x10000

/* Use the minimum maximum to guarantee that it can always fit in the safe
 * const file size, even with maximum push constant usage and driver params.
 */
#define MAX_INLINE_UBO_RANGE 256
#define MAX_INLINE_UBOS 4

#define A6XX_TEX_CONST_DWORDS 16
#define A6XX_TEX_SAMP_DWORDS 4

/* We sample the fragment density map on the CPU, so technically the
 * minimum/maximum texel size is arbitrary. However sizes smaller than the
 * minimum tile width alignment of 32 are likely pointless, so we use that as
 * the minimum value. For the maximum just pick a value larger than anyone
 * would reasonably need.
 */
#define MIN_FDM_TEXEL_SIZE_LOG2 5
#define MIN_FDM_TEXEL_SIZE (1u << MIN_FDM_TEXEL_SIZE_LOG2)
#define MAX_FDM_TEXEL_SIZE_LOG2 10
#define MAX_FDM_TEXEL_SIZE (1u << MAX_FDM_TEXEL_SIZE_LOG2)

#define TU_FROM_HANDLE(__tu_type, __name, __handle)                          \
   VK_FROM_HANDLE(__tu_type, __name, __handle)

#define TU_GPU_GENS A6XX, A7XX
#define TU_GENX(FUNC_NAME)                                                   \
   template <chip... CHIPs> constexpr auto FUNC_NAME##instantiate()          \
   {                                                                         \
      return std::tuple_cat(std::make_tuple(FUNC_NAME<CHIPs>)...);           \
   }                                                                         \
   static constexpr auto FUNC_NAME##tmpl __attribute__((used)) =             \
      FUNC_NAME##instantiate<TU_GPU_GENS>();

#define TU_CALLX(device, thing)                                              \
   ({                                                                        \
      decltype(&thing<A6XX>) genX_thing;                                     \
      switch ((device)->physical_device->info->chip) {                       \
      case 6:                                                                \
         genX_thing = &thing<A6XX>;                                          \
         break;                                                              \
      case 7:                                                                \
         genX_thing = &thing<A7XX>;                                          \
         break;                                                              \
      default:                                                               \
         unreachable("Unknown hardware generation");                         \
      }                                                                      \
      genX_thing;                                                            \
   })

/* vk object types */
struct tu_buffer;
struct tu_buffer_view;
struct tu_cmd_buffer;
struct tu_cmd_pool;
struct tu_descriptor_pool;
struct tu_descriptor_set;
struct tu_descriptor_set_layout;
struct tu_descriptor_update_template;
struct tu_device;
struct tu_device_memory;
struct tu_event;
struct tu_framebuffer;
struct tu_image;
struct tu_image_view;
struct tu_instance;
struct tu_physical_device;
struct tu_pipeline_layout;
struct tu_query_pool;
struct tu_queue;
struct tu_render_pass;
struct tu_sampler;
struct tu_sampler_ycbcr_conversion;

struct breadcrumbs_context;
struct tu_bo;
struct tu_cs;
struct tu_cs_entry;
struct tu_suballoc_bo;
struct tu_suballocator;
struct tu_subpass;
struct tu_u_trace_submission_data;

#endif /* TU_COMMON_H */
