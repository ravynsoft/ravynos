/*
 * Copyright 2018 Collabora Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHOR(S) AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "zink_screen.h"

#include "zink_kopper.h"
#include "zink_compiler.h"
#include "zink_context.h"
#include "zink_descriptors.h"
#include "zink_fence.h"
#include "vk_format.h"
#include "zink_format.h"
#include "zink_framebuffer.h"
#include "zink_program.h"
#include "zink_public.h"
#include "zink_query.h"
#include "zink_resource.h"
#include "zink_state.h"
#include "nir_to_spirv/nir_to_spirv.h" // for SPIRV_VERSION

#include "util/u_debug.h"
#include "util/u_dl.h"
#include "util/os_file.h"
#include "util/u_memory.h"
#include "util/u_screen.h"
#include "util/u_string.h"
#include "util/perf/u_trace.h"
#include "util/u_transfer_helper.h"
#include "util/hex.h"
#include "util/xmlconfig.h"

#include "util/u_cpu_detect.h"

#ifdef HAVE_LIBDRM
#include <xf86drm.h>
#include <fcntl.h>
#include <sys/stat.h>
#ifdef MAJOR_IN_MKDEV
#include <sys/mkdev.h>
#endif
#ifdef MAJOR_IN_SYSMACROS
#include <sys/sysmacros.h>
#endif
#endif

static int num_screens = 0;
bool zink_tracing = false;

#if DETECT_OS_WINDOWS
#include <io.h>
#define VK_LIBNAME "vulkan-1.dll"
#else
#include <unistd.h>
#if DETECT_OS_APPLE
#define VK_LIBNAME "libvulkan.1.dylib"
#elif DETECT_OS_ANDROID
#define VK_LIBNAME "libvulkan.so"
#else
#define VK_LIBNAME "libvulkan.so.1"
#endif
#endif

#if defined(__APPLE__)
// Source of MVK_VERSION
#include "MoltenVK/vk_mvk_moltenvk.h"
#endif

#ifdef HAVE_LIBDRM
#include "drm-uapi/dma-buf.h"
#include <xf86drm.h>
#endif

static const struct debug_named_value
zink_debug_options[] = {
   { "nir", ZINK_DEBUG_NIR, "Dump NIR during program compile" },
   { "spirv", ZINK_DEBUG_SPIRV, "Dump SPIR-V during program compile" },
   { "tgsi", ZINK_DEBUG_TGSI, "Dump TGSI during program compile" },
   { "validation", ZINK_DEBUG_VALIDATION, "Dump Validation layer output" },
   { "vvl", ZINK_DEBUG_VALIDATION, "Dump Validation layer output" },
   { "sync", ZINK_DEBUG_SYNC, "Force synchronization before draws/dispatches" },
   { "compact", ZINK_DEBUG_COMPACT, "Use only 4 descriptor sets" },
   { "noreorder", ZINK_DEBUG_NOREORDER, "Do not reorder command streams" },
   { "gpl", ZINK_DEBUG_GPL, "Force using Graphics Pipeline Library for all shaders" },
   { "shaderdb", ZINK_DEBUG_SHADERDB, "Do stuff to make shader-db work" },
   { "rp", ZINK_DEBUG_RP, "Enable renderpass tracking/optimizations" },
   { "norp", ZINK_DEBUG_NORP, "Disable renderpass tracking/optimizations" },
   { "map", ZINK_DEBUG_MAP, "Track amount of mapped VRAM" },
   { "flushsync", ZINK_DEBUG_FLUSHSYNC, "Force synchronous flushes/presents" },
   { "noshobj", ZINK_DEBUG_NOSHOBJ, "Disable EXT_shader_object" },
   { "optimal_keys", ZINK_DEBUG_OPTIMAL_KEYS, "Debug/use optimal_keys" },
   { "noopt", ZINK_DEBUG_NOOPT, "Disable async optimized pipeline compiles" },
   { "nobgc", ZINK_DEBUG_NOBGC, "Disable all async pipeline compiles" },
   { "dgc", ZINK_DEBUG_DGC, "Use DGC (driver testing only)" },
   { "mem", ZINK_DEBUG_MEM, "Debug memory allocations" },
   { "quiet", ZINK_DEBUG_QUIET, "Suppress warnings" },
   DEBUG_NAMED_VALUE_END
};

DEBUG_GET_ONCE_FLAGS_OPTION(zink_debug, "ZINK_DEBUG", zink_debug_options, 0)

uint32_t
zink_debug;


static const struct debug_named_value
zink_descriptor_options[] = {
   { "auto", ZINK_DESCRIPTOR_MODE_AUTO, "Automatically detect best mode" },
   { "lazy", ZINK_DESCRIPTOR_MODE_LAZY, "Don't cache, do least amount of updates" },
   { "db", ZINK_DESCRIPTOR_MODE_DB, "Use descriptor buffers" },
   DEBUG_NAMED_VALUE_END
};

DEBUG_GET_ONCE_FLAGS_OPTION(zink_descriptor_mode, "ZINK_DESCRIPTORS", zink_descriptor_options, ZINK_DESCRIPTOR_MODE_AUTO)

enum zink_descriptor_mode zink_descriptor_mode;

static const char *
zink_get_vendor(struct pipe_screen *pscreen)
{
   return "Mesa";
}

static const char *
zink_get_device_vendor(struct pipe_screen *pscreen)
{
   struct zink_screen *screen = zink_screen(pscreen);
   static char buf[1000];
   snprintf(buf, sizeof(buf), "Unknown (vendor-id: 0x%04x)", screen->info.props.vendorID);
   return buf;
}

static const char *
zink_get_name(struct pipe_screen *pscreen)
{
   struct zink_screen *screen = zink_screen(pscreen);
   const char *driver_name = vk_DriverId_to_str(screen->info.driver_props.driverID) + strlen("VK_DRIVER_ID_");
   static char buf[1000];
   snprintf(buf, sizeof(buf), "zink Vulkan %d.%d(%s (%s))",
            VK_VERSION_MAJOR(screen->info.device_version),
            VK_VERSION_MINOR(screen->info.device_version),
            screen->info.props.deviceName,
            strstr(vk_DriverId_to_str(screen->info.driver_props.driverID), "VK_DRIVER_ID_") ? driver_name : "Driver Unknown"
            );
   return buf;
}

static void
zink_get_driver_uuid(struct pipe_screen *pscreen, char *uuid)
{
   struct zink_screen *screen = zink_screen(pscreen);
   if (screen->vk_version >= VK_MAKE_VERSION(1,2,0)) {
      memcpy(uuid, screen->info.props11.driverUUID, VK_UUID_SIZE);
   } else {
      memcpy(uuid, screen->info.deviceid_props.driverUUID, VK_UUID_SIZE);
   }
}

static void
zink_get_device_uuid(struct pipe_screen *pscreen, char *uuid)
{
   struct zink_screen *screen = zink_screen(pscreen);
   if (screen->vk_version >= VK_MAKE_VERSION(1,2,0)) {
      memcpy(uuid, screen->info.props11.deviceUUID, VK_UUID_SIZE);
   } else {
      memcpy(uuid, screen->info.deviceid_props.deviceUUID, VK_UUID_SIZE);
   }
}

static void
zink_get_device_luid(struct pipe_screen *pscreen, char *luid)
{
   struct zink_screen *screen = zink_screen(pscreen);
   if (screen->info.have_vulkan12) {
      memcpy(luid, screen->info.props11.deviceLUID, VK_LUID_SIZE);
   } else {
      memcpy(luid, screen->info.deviceid_props.deviceLUID, VK_LUID_SIZE);
   }
}

static uint32_t
zink_get_device_node_mask(struct pipe_screen *pscreen)
{
   struct zink_screen *screen = zink_screen(pscreen);
   if (screen->info.have_vulkan12) {
      return screen->info.props11.deviceNodeMask;
   } else {
      return screen->info.deviceid_props.deviceNodeMask;
   }
}

static void
zink_set_max_shader_compiler_threads(struct pipe_screen *pscreen, unsigned max_threads)
{
   struct zink_screen *screen = zink_screen(pscreen);
   util_queue_adjust_num_threads(&screen->cache_get_thread, max_threads, false);
}

static bool
zink_is_parallel_shader_compilation_finished(struct pipe_screen *screen, void *shader, enum pipe_shader_type shader_type)
{
   if (shader_type == MESA_SHADER_COMPUTE) {
      struct zink_program *pg = shader;
      return !pg->can_precompile || util_queue_fence_is_signalled(&pg->cache_fence);
   }

   struct zink_shader *zs = shader;
   if (!util_queue_fence_is_signalled(&zs->precompile.fence))
      return false;
   bool finished = true;
   set_foreach(zs->programs, entry) {
      struct zink_gfx_program *prog = (void*)entry->key;
      finished &= util_queue_fence_is_signalled(&prog->base.cache_fence);
   }
   return finished;
}

static VkDeviceSize
get_video_mem(struct zink_screen *screen)
{
   VkDeviceSize size = 0;
   for (uint32_t i = 0; i < screen->info.mem_props.memoryHeapCount; ++i) {
      if (screen->info.mem_props.memoryHeaps[i].flags &
          VK_MEMORY_HEAP_DEVICE_LOCAL_BIT)
         size += screen->info.mem_props.memoryHeaps[i].size;
   }
   return size;
}

/**
 * Creates the disk cache used by mesa/st frontend for caching the GLSL -> NIR
 * path.
 *
 * The output that gets stored in the frontend's cache is the result of
 * zink_shader_finalize().  So, our sha1 cache key here needs to include
 * everything that would change the NIR we generate from a given set of GLSL
 * source, including our driver build, the Vulkan device and driver (which could
 * affect the pipe caps we show the frontend), and any debug flags that change
 * codegen.
 *
 * This disk cache also gets used by zink itself for storing its output from NIR
 * -> SPIRV translation.
 */
static bool
disk_cache_init(struct zink_screen *screen)
{
   if (zink_debug & ZINK_DEBUG_SHADERDB)
      return true;

#ifdef ENABLE_SHADER_CACHE
   struct mesa_sha1 ctx;
   _mesa_sha1_init(&ctx);

#ifdef HAVE_DL_ITERATE_PHDR
   /* Hash in the zink driver build. */
   const struct build_id_note *note =
       build_id_find_nhdr_for_addr(disk_cache_init);
   unsigned build_id_len = build_id_length(note);
   assert(note && build_id_len == 20); /* sha1 */
   _mesa_sha1_update(&ctx, build_id_data(note), build_id_len);
#endif

   /* Hash in the Vulkan pipeline cache UUID to identify the combination of
   *  vulkan device and driver (or any inserted layer that would invalidate our
   *  cached pipelines).
   *
   * "Although they have identical descriptions, VkPhysicalDeviceIDProperties
   *  ::deviceUUID may differ from
   *  VkPhysicalDeviceProperties2::pipelineCacheUUID. The former is intended to
   *  identify and correlate devices across API and driver boundaries, while the
   *  latter is used to identify a compatible device and driver combination to
   *  use when serializing and de-serializing pipeline state."
   */
   _mesa_sha1_update(&ctx, screen->info.props.pipelineCacheUUID, VK_UUID_SIZE);

   /* Hash in our debug flags that affect NIR generation as of finalize_nir */
   unsigned shader_debug_flags = zink_debug & ZINK_DEBUG_COMPACT;
   _mesa_sha1_update(&ctx, &shader_debug_flags, sizeof(shader_debug_flags));

   /* Some of the driconf options change shaders.  Let's just hash the whole
    * thing to not forget any (especially as options get added).
    */
   _mesa_sha1_update(&ctx, &screen->driconf, sizeof(screen->driconf));

   /* EXT_shader_object causes different descriptor layouts for separate shaders */
   _mesa_sha1_update(&ctx, &screen->info.have_EXT_shader_object, sizeof(screen->info.have_EXT_shader_object));

   /* Finish the sha1 and format it as text. */
   unsigned char sha1[20];
   _mesa_sha1_final(&ctx, sha1);

   char cache_id[20 * 2 + 1];
   mesa_bytes_to_hex(cache_id, sha1, 20);

   screen->disk_cache = disk_cache_create("zink", cache_id, 0);

   if (!screen->disk_cache)
      return true;

   if (!util_queue_init(&screen->cache_put_thread, "zcq", 8, 1, UTIL_QUEUE_INIT_RESIZE_IF_FULL, screen)) {
      mesa_loge("zink: Failed to create disk cache queue\n");

      disk_cache_destroy(screen->disk_cache);
      screen->disk_cache = NULL;

      return false;
   }
#endif

   return true;
}


static void
cache_put_job(void *data, void *gdata, int thread_index)
{
   struct zink_program *pg = data;
   struct zink_screen *screen = gdata;
   size_t size = 0;
   u_rwlock_rdlock(&pg->pipeline_cache_lock);
   VkResult result = VKSCR(GetPipelineCacheData)(screen->dev, pg->pipeline_cache, &size, NULL);
   if (result != VK_SUCCESS) {
      u_rwlock_rdunlock(&pg->pipeline_cache_lock);
      mesa_loge("ZINK: vkGetPipelineCacheData failed (%s)", vk_Result_to_str(result));
      return;
   }
   if (pg->pipeline_cache_size == size) {
      u_rwlock_rdunlock(&pg->pipeline_cache_lock);
      return;
   }
   void *pipeline_data = malloc(size);
   if (!pipeline_data) {
      u_rwlock_rdunlock(&pg->pipeline_cache_lock);
      return;
   }
   result = VKSCR(GetPipelineCacheData)(screen->dev, pg->pipeline_cache, &size, pipeline_data);
   u_rwlock_rdunlock(&pg->pipeline_cache_lock);
   if (result == VK_SUCCESS) {
      pg->pipeline_cache_size = size;

      cache_key key;
      disk_cache_compute_key(screen->disk_cache, pg->sha1, sizeof(pg->sha1), key);
      disk_cache_put_nocopy(screen->disk_cache, key, pipeline_data, size, NULL);
   } else {
      mesa_loge("ZINK: vkGetPipelineCacheData failed (%s)", vk_Result_to_str(result));
   }
}

void
zink_screen_update_pipeline_cache(struct zink_screen *screen, struct zink_program *pg, bool in_thread)
{
   if (!screen->disk_cache || !pg->pipeline_cache)
      return;

   if (in_thread)
      cache_put_job(pg, screen, 0);
   else if (util_queue_fence_is_signalled(&pg->cache_fence))
      util_queue_add_job(&screen->cache_put_thread, pg, &pg->cache_fence, cache_put_job, NULL, 0);
}

static void
cache_get_job(void *data, void *gdata, int thread_index)
{
   struct zink_program *pg = data;
   struct zink_screen *screen = gdata;

   VkPipelineCacheCreateInfo pcci;
   pcci.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
   pcci.pNext = NULL;
   pcci.flags = screen->info.have_EXT_pipeline_creation_cache_control ? VK_PIPELINE_CACHE_CREATE_EXTERNALLY_SYNCHRONIZED_BIT : 0;
   pcci.initialDataSize = 0;
   pcci.pInitialData = NULL;

   cache_key key;
   disk_cache_compute_key(screen->disk_cache, pg->sha1, sizeof(pg->sha1), key);
   pcci.pInitialData = disk_cache_get(screen->disk_cache, key, &pg->pipeline_cache_size);
   pcci.initialDataSize = pg->pipeline_cache_size;

   VkResult res = VKSCR(CreatePipelineCache)(screen->dev, &pcci, NULL, &pg->pipeline_cache);
   if (res != VK_SUCCESS) {
      mesa_loge("ZINK: vkCreatePipelineCache failed (%s)", vk_Result_to_str(res));
   }
   free((void*)pcci.pInitialData);
}

void
zink_screen_get_pipeline_cache(struct zink_screen *screen, struct zink_program *pg, bool in_thread)
{
   if (!screen->disk_cache)
      return;

   if (in_thread)
      cache_get_job(pg, screen, 0);
   else
      util_queue_add_job(&screen->cache_get_thread, pg, &pg->cache_fence, cache_get_job, NULL, 0);
}

static int
zink_get_compute_param(struct pipe_screen *pscreen, enum pipe_shader_ir ir_type,
                       enum pipe_compute_cap param, void *ret)
{
   struct zink_screen *screen = zink_screen(pscreen);
#define RET(x) do {                  \
   if (ret)                          \
      memcpy(ret, x, sizeof(x));     \
   return sizeof(x);                 \
} while (0)

   switch (param) {
   case PIPE_COMPUTE_CAP_ADDRESS_BITS:
      RET((uint32_t []){ 64 });

   case PIPE_COMPUTE_CAP_IR_TARGET:
      if (ret)
         strcpy(ret, "nir");
      return 4;

   case PIPE_COMPUTE_CAP_GRID_DIMENSION:
      RET((uint64_t []) { 3 });

   case PIPE_COMPUTE_CAP_MAX_GRID_SIZE:
      RET(((uint64_t []) { screen->info.props.limits.maxComputeWorkGroupCount[0],
                           screen->info.props.limits.maxComputeWorkGroupCount[1],
                           screen->info.props.limits.maxComputeWorkGroupCount[2] }));

   case PIPE_COMPUTE_CAP_MAX_BLOCK_SIZE:
      /* MaxComputeWorkGroupSize[0..2] */
      RET(((uint64_t []) {screen->info.props.limits.maxComputeWorkGroupSize[0],
                          screen->info.props.limits.maxComputeWorkGroupSize[1],
                          screen->info.props.limits.maxComputeWorkGroupSize[2]}));

   case PIPE_COMPUTE_CAP_MAX_THREADS_PER_BLOCK:
   case PIPE_COMPUTE_CAP_MAX_VARIABLE_THREADS_PER_BLOCK:
      RET((uint64_t []) { screen->info.props.limits.maxComputeWorkGroupInvocations });

   case PIPE_COMPUTE_CAP_MAX_LOCAL_SIZE:
      RET((uint64_t []) { screen->info.props.limits.maxComputeSharedMemorySize });

   case PIPE_COMPUTE_CAP_IMAGES_SUPPORTED:
      RET((uint32_t []) { 1 });

   case PIPE_COMPUTE_CAP_SUBGROUP_SIZES:
      RET((uint32_t []) { screen->info.props11.subgroupSize });

   case PIPE_COMPUTE_CAP_MAX_MEM_ALLOC_SIZE:
      RET((uint64_t []) { screen->clamp_video_mem });

   case PIPE_COMPUTE_CAP_MAX_GLOBAL_SIZE:
      RET((uint64_t []) { screen->total_video_mem });

   case PIPE_COMPUTE_CAP_MAX_COMPUTE_UNITS:
      // no way in vulkan to retrieve this information.
      RET((uint32_t []) { 1 });

   case PIPE_COMPUTE_CAP_MAX_SUBGROUPS:
   case PIPE_COMPUTE_CAP_MAX_CLOCK_FREQUENCY:
   case PIPE_COMPUTE_CAP_MAX_PRIVATE_SIZE:
   case PIPE_COMPUTE_CAP_MAX_INPUT_SIZE:
      // XXX: I think these are for Clover...
      return 0;

   default:
      unreachable("unknown compute param");
   }
}

static uint32_t
get_smallest_buffer_heap(struct zink_screen *screen)
{
   enum zink_heap heaps[] = {
      ZINK_HEAP_DEVICE_LOCAL,
      ZINK_HEAP_DEVICE_LOCAL_VISIBLE,
      ZINK_HEAP_HOST_VISIBLE_COHERENT,
      ZINK_HEAP_HOST_VISIBLE_COHERENT
   };
   unsigned size = UINT32_MAX;
   for (unsigned i = 0; i < ARRAY_SIZE(heaps); i++) {
      for (unsigned j = 0; j < screen->heap_count[i]; j++) {
         unsigned heap_idx = screen->info.mem_props.memoryTypes[screen->heap_map[i][j]].heapIndex;
         size = MIN2(screen->info.mem_props.memoryHeaps[heap_idx].size, size);
      }
   }
   return size;
}

static inline bool
have_fp32_filter_linear(struct zink_screen *screen)
{
   const VkFormat fp32_formats[] = {
      VK_FORMAT_R32_SFLOAT,
      VK_FORMAT_R32G32_SFLOAT,
      VK_FORMAT_R32G32B32_SFLOAT,
      VK_FORMAT_R32G32B32A32_SFLOAT,
      VK_FORMAT_D32_SFLOAT,
   };
   for (int i = 0; i < ARRAY_SIZE(fp32_formats); ++i) {
      VkFormatProperties props;
      VKSCR(GetPhysicalDeviceFormatProperties)(screen->pdev,
                                               fp32_formats[i],
                                               &props);
      if (((props.linearTilingFeatures | props.optimalTilingFeatures) &
           (VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT |
            VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) ==
          VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT) {
         return false;
      }
   }
   return true;
}

static int
zink_get_param(struct pipe_screen *pscreen, enum pipe_cap param)
{
   struct zink_screen *screen = zink_screen(pscreen);

   switch (param) {
   case PIPE_CAP_NULL_TEXTURES:
      return screen->info.rb_image_feats.robustImageAccess;
   case PIPE_CAP_TEXRECT:
   case PIPE_CAP_MULTI_DRAW_INDIRECT_PARTIAL_STRIDE:
      return 0;
   case PIPE_CAP_ANISOTROPIC_FILTER:
      return screen->info.feats.features.samplerAnisotropy;
   case PIPE_CAP_EMULATE_NONFIXED_PRIMITIVE_RESTART:
      return 1;
   case PIPE_CAP_SUPPORTED_PRIM_MODES_WITH_RESTART: {
      uint32_t modes = BITFIELD_BIT(MESA_PRIM_LINE_STRIP) |
                       BITFIELD_BIT(MESA_PRIM_TRIANGLE_STRIP) |
                       BITFIELD_BIT(MESA_PRIM_LINE_STRIP_ADJACENCY) |
                       BITFIELD_BIT(MESA_PRIM_TRIANGLE_STRIP_ADJACENCY);
      if (screen->have_triangle_fans)
         modes |= BITFIELD_BIT(MESA_PRIM_TRIANGLE_FAN);
      if (screen->info.have_EXT_primitive_topology_list_restart) {
         modes |= BITFIELD_BIT(MESA_PRIM_POINTS) |
                  BITFIELD_BIT(MESA_PRIM_LINES) |
                  BITFIELD_BIT(MESA_PRIM_LINES_ADJACENCY) |
                  BITFIELD_BIT(MESA_PRIM_TRIANGLES) |
                  BITFIELD_BIT(MESA_PRIM_TRIANGLES_ADJACENCY);
         if (screen->info.list_restart_feats.primitiveTopologyPatchListRestart)
            modes |= BITFIELD_BIT(MESA_PRIM_PATCHES);
      }
      return modes;
   }
   case PIPE_CAP_SUPPORTED_PRIM_MODES: {
      uint32_t modes = BITFIELD_MASK(MESA_PRIM_COUNT);
      modes &= ~BITFIELD_BIT(MESA_PRIM_QUAD_STRIP);
      modes &= ~BITFIELD_BIT(MESA_PRIM_POLYGON);
      modes &= ~BITFIELD_BIT(MESA_PRIM_LINE_LOOP);
      if (!screen->have_triangle_fans)
         modes &= ~BITFIELD_BIT(MESA_PRIM_TRIANGLE_FAN);
      return modes;
   }

   case PIPE_CAP_FBFETCH:
      return 1;
   case PIPE_CAP_FBFETCH_COHERENT:
      return screen->info.have_EXT_rasterization_order_attachment_access;

   case PIPE_CAP_MEMOBJ:
      return screen->instance_info.have_KHR_external_memory_capabilities && (screen->info.have_KHR_external_memory_fd || screen->info.have_KHR_external_memory_win32);
   case PIPE_CAP_FENCE_SIGNAL:
      return screen->info.have_KHR_external_semaphore_fd || screen->info.have_KHR_external_semaphore_win32;
   case PIPE_CAP_NATIVE_FENCE_FD:
      return screen->instance_info.have_KHR_external_semaphore_capabilities && screen->info.have_KHR_external_semaphore_fd;
   case PIPE_CAP_RESOURCE_FROM_USER_MEMORY:
      return screen->info.have_EXT_external_memory_host;

   case PIPE_CAP_SURFACE_REINTERPRET_BLOCKS:
      return screen->info.have_vulkan11 || screen->info.have_KHR_maintenance2;

   case PIPE_CAP_VALIDATE_ALL_DIRTY_STATES:
   case PIPE_CAP_ALLOW_MAPPED_BUFFERS_DURING_EXECUTION:
   case PIPE_CAP_MAP_UNSYNCHRONIZED_THREAD_SAFE:
   case PIPE_CAP_SHAREABLE_SHADERS:
   case PIPE_CAP_DEVICE_RESET_STATUS_QUERY:
   case PIPE_CAP_QUERY_MEMORY_INFO:
   case PIPE_CAP_NPOT_TEXTURES:
   case PIPE_CAP_TGSI_TEXCOORD:
   case PIPE_CAP_DRAW_INDIRECT:
   case PIPE_CAP_TEXTURE_QUERY_LOD:
   case PIPE_CAP_GLSL_TESS_LEVELS_AS_INPUTS:
   case PIPE_CAP_COPY_BETWEEN_COMPRESSED_AND_PLAIN_FORMATS:
   case PIPE_CAP_FORCE_PERSAMPLE_INTERP:
   case PIPE_CAP_FRAMEBUFFER_NO_ATTACHMENT:
   case PIPE_CAP_SHADER_ARRAY_COMPONENTS:
   case PIPE_CAP_QUERY_BUFFER_OBJECT:
   case PIPE_CAP_CONDITIONAL_RENDER_INVERTED:
   case PIPE_CAP_CLIP_HALFZ:
   case PIPE_CAP_TEXTURE_QUERY_SAMPLES:
   case PIPE_CAP_TEXTURE_BARRIER:
   case PIPE_CAP_QUERY_SO_OVERFLOW:
   case PIPE_CAP_GL_SPIRV:
   case PIPE_CAP_CLEAR_SCISSORED:
   case PIPE_CAP_INVALIDATE_BUFFER:
   case PIPE_CAP_PREFER_REAL_BUFFER_IN_CONSTBUF0:
   case PIPE_CAP_PACKED_UNIFORMS:
   case PIPE_CAP_SHADER_PACK_HALF_FLOAT:
   case PIPE_CAP_CULL_DISTANCE_NOCOMBINE:
   case PIPE_CAP_SEAMLESS_CUBE_MAP_PER_TEXTURE:
   case PIPE_CAP_LOAD_CONSTBUF:
   case PIPE_CAP_MULTISAMPLE_Z_RESOLVE:
   case PIPE_CAP_ALLOW_GLTHREAD_BUFFER_SUBDATA_OPT:
      return 1;

   case PIPE_CAP_DRAW_VERTEX_STATE:
      return screen->info.have_EXT_vertex_input_dynamic_state;

   case PIPE_CAP_SURFACE_SAMPLE_COUNT:
      return screen->vk_version >= VK_MAKE_VERSION(1,2,0);

   case PIPE_CAP_SHADER_GROUP_VOTE:
      if (screen->info.have_vulkan11 &&
          (screen->info.subgroup.supportedOperations & VK_SUBGROUP_FEATURE_VOTE_BIT) &&
          (screen->info.subgroup.supportedStages & VK_SHADER_STAGE_COMPUTE_BIT))
         return true;
      if (screen->info.have_EXT_shader_subgroup_vote)
         return true;
      return false;
   case PIPE_CAP_QUADS_FOLLOW_PROVOKING_VERTEX_CONVENTION:
      return 1;

   case PIPE_CAP_TEXTURE_MIRROR_CLAMP_TO_EDGE:
      return screen->info.have_KHR_sampler_mirror_clamp_to_edge || (screen->info.have_vulkan12 && screen->info.feats12.samplerMirrorClampToEdge);

   case PIPE_CAP_POLYGON_OFFSET_UNITS_UNSCALED:
      return 1;

   case PIPE_CAP_POLYGON_OFFSET_CLAMP:
      return screen->info.feats.features.depthBiasClamp;

   case PIPE_CAP_QUERY_PIPELINE_STATISTICS_SINGLE:
      return screen->info.feats.features.pipelineStatisticsQuery;

   case PIPE_CAP_ROBUST_BUFFER_ACCESS_BEHAVIOR:
      return screen->info.feats.features.robustBufferAccess &&
             (screen->info.rb2_feats.robustImageAccess2 || screen->driver_workarounds.lower_robustImageAccess2);

   case PIPE_CAP_MULTI_DRAW_INDIRECT:
      return screen->info.feats.features.multiDrawIndirect;

   case PIPE_CAP_IMAGE_ATOMIC_FLOAT_ADD:
      return (screen->info.have_EXT_shader_atomic_float &&
              screen->info.atomic_float_feats.shaderSharedFloat32AtomicAdd &&
              screen->info.atomic_float_feats.shaderBufferFloat32AtomicAdd);
   case PIPE_CAP_SHADER_ATOMIC_INT64:
      return (screen->info.have_KHR_shader_atomic_int64 &&
              screen->info.atomic_int_feats.shaderSharedInt64Atomics &&
              screen->info.atomic_int_feats.shaderBufferInt64Atomics);

   case PIPE_CAP_MULTI_DRAW_INDIRECT_PARAMS:
      return screen->info.have_KHR_draw_indirect_count;

   case PIPE_CAP_START_INSTANCE:
   case PIPE_CAP_DRAW_PARAMETERS:
      return (screen->info.have_vulkan12 && screen->info.feats11.shaderDrawParameters) ||
              screen->info.have_KHR_shader_draw_parameters;

   case PIPE_CAP_VERTEX_ELEMENT_INSTANCE_DIVISOR:
      return screen->info.have_EXT_vertex_attribute_divisor;

   case PIPE_CAP_MAX_VERTEX_STREAMS:
      return screen->info.tf_props.maxTransformFeedbackStreams;

   case PIPE_CAP_COMPUTE_SHADER_DERIVATIVES:
      return screen->info.have_NV_compute_shader_derivatives;

   case PIPE_CAP_INT64:
   case PIPE_CAP_DOUBLES:
      return 1;

   case PIPE_CAP_MAX_DUAL_SOURCE_RENDER_TARGETS:
      if (!screen->info.feats.features.dualSrcBlend)
         return 0;
      return screen->info.props.limits.maxFragmentDualSrcAttachments;

   case PIPE_CAP_MAX_RENDER_TARGETS:
      return screen->info.props.limits.maxColorAttachments;

   case PIPE_CAP_OCCLUSION_QUERY:
      return screen->info.feats.features.occlusionQueryPrecise;

   case PIPE_CAP_PROGRAMMABLE_SAMPLE_LOCATIONS:
      return screen->info.have_EXT_sample_locations && screen->info.have_EXT_extended_dynamic_state;

   case PIPE_CAP_QUERY_TIME_ELAPSED:
      return screen->timestamp_valid_bits > 0;

   case PIPE_CAP_TEXTURE_MULTISAMPLE:
      return 1;

   case PIPE_CAP_FRAGMENT_SHADER_INTERLOCK:
      return screen->info.have_EXT_fragment_shader_interlock;

   case PIPE_CAP_SHADER_CLOCK:
      return screen->info.have_KHR_shader_clock;

   case PIPE_CAP_SHADER_BALLOT:
      if (screen->info.props11.subgroupSize > 64)
         return false;
      if (screen->info.have_vulkan11 &&
          screen->info.subgroup.supportedOperations & VK_SUBGROUP_FEATURE_BALLOT_BIT)
         return true;
      if (screen->info.have_EXT_shader_subgroup_ballot)
         return true;
      return false;

   case PIPE_CAP_DEMOTE_TO_HELPER_INVOCATION:
      return screen->spirv_version >= SPIRV_VERSION(1, 6) ||
             screen->info.have_EXT_shader_demote_to_helper_invocation;

   case PIPE_CAP_SAMPLE_SHADING:
      return screen->info.feats.features.sampleRateShading;

   case PIPE_CAP_TEXTURE_SWIZZLE:
      return 1;

   case PIPE_CAP_VERTEX_ATTRIB_ELEMENT_ALIGNED_ONLY:
      return 1;

   case PIPE_CAP_GL_CLAMP:
      return 0;

   case PIPE_CAP_PREFER_IMM_ARRAYS_AS_CONSTBUF:
      return 0; /* Assume that the vk driver is capable of moving imm arrays to some sort of constant storage on its own. */

   case PIPE_CAP_TEXTURE_BORDER_COLOR_QUIRK: {
      enum pipe_quirk_texture_border_color_swizzle quirk = PIPE_QUIRK_TEXTURE_BORDER_COLOR_SWIZZLE_ALPHA_NOT_W;
      if (!screen->info.border_color_feats.customBorderColorWithoutFormat)
         return quirk | PIPE_QUIRK_TEXTURE_BORDER_COLOR_SWIZZLE_FREEDRENO;
      /* assume that if drivers don't implement this extension they either:
       * - don't support custom border colors
       * - handle things correctly
       * - hate border color accuracy
       */
      if (screen->info.have_EXT_border_color_swizzle &&
          !screen->info.border_swizzle_feats.borderColorSwizzleFromImage)
         return quirk | PIPE_QUIRK_TEXTURE_BORDER_COLOR_SWIZZLE_NV50;
      return quirk;
   }

   case PIPE_CAP_MAX_TEXTURE_2D_SIZE:
      return MIN2(screen->info.props.limits.maxImageDimension1D,
                  screen->info.props.limits.maxImageDimension2D);
   case PIPE_CAP_MAX_TEXTURE_3D_LEVELS:
      return 1 + util_logbase2(screen->info.props.limits.maxImageDimension3D);
   case PIPE_CAP_MAX_TEXTURE_CUBE_LEVELS:
      return 1 + util_logbase2(screen->info.props.limits.maxImageDimensionCube);

   case PIPE_CAP_FRAGMENT_SHADER_TEXTURE_LOD:
   case PIPE_CAP_FRAGMENT_SHADER_DERIVATIVES:
      return 1;

   case PIPE_CAP_BLEND_EQUATION_SEPARATE:
   case PIPE_CAP_INDEP_BLEND_ENABLE:
   case PIPE_CAP_INDEP_BLEND_FUNC:
      return screen->info.feats.features.independentBlend;

   case PIPE_CAP_DITHERING:
      return 0;

   case PIPE_CAP_MAX_STREAM_OUTPUT_BUFFERS:
      return screen->info.have_EXT_transform_feedback ? screen->info.tf_props.maxTransformFeedbackBuffers : 0;
   case PIPE_CAP_STREAM_OUTPUT_PAUSE_RESUME:
   case PIPE_CAP_STREAM_OUTPUT_INTERLEAVE_BUFFERS:
      return screen->info.have_EXT_transform_feedback;

   case PIPE_CAP_MAX_TEXTURE_ARRAY_LAYERS:
      return screen->info.props.limits.maxImageArrayLayers;

   case PIPE_CAP_DEPTH_CLIP_DISABLE:
      return screen->info.have_EXT_depth_clip_enable;

   case PIPE_CAP_SHADER_STENCIL_EXPORT:
      return screen->info.have_EXT_shader_stencil_export;

   case PIPE_CAP_VS_INSTANCEID:
   case PIPE_CAP_SEAMLESS_CUBE_MAP:
      return 1;

   case PIPE_CAP_MIN_TEXEL_OFFSET:
      return screen->info.props.limits.minTexelOffset;
   case PIPE_CAP_MAX_TEXEL_OFFSET:
      return screen->info.props.limits.maxTexelOffset;

   case PIPE_CAP_VERTEX_COLOR_UNCLAMPED:
      return 1;

   case PIPE_CAP_CONDITIONAL_RENDER:
     return 1;

   case PIPE_CAP_GLSL_FEATURE_LEVEL_COMPATIBILITY:
   case PIPE_CAP_GLSL_FEATURE_LEVEL:
      return 460;

   case PIPE_CAP_COMPUTE:
      return 1;

   case PIPE_CAP_CONSTANT_BUFFER_OFFSET_ALIGNMENT:
      return screen->info.props.limits.minUniformBufferOffsetAlignment;

   case PIPE_CAP_QUERY_TIMESTAMP:
      return screen->timestamp_valid_bits > 0;

   case PIPE_CAP_QUERY_TIMESTAMP_BITS:
      return screen->timestamp_valid_bits;

   case PIPE_CAP_TIMER_RESOLUTION:
      return ceil(screen->info.props.limits.timestampPeriod);

   case PIPE_CAP_MIN_MAP_BUFFER_ALIGNMENT:
      return 1 << MIN_SLAB_ORDER;

   case PIPE_CAP_CUBE_MAP_ARRAY:
      return screen->info.feats.features.imageCubeArray;

   case PIPE_CAP_TEXTURE_BUFFER_OBJECTS:
   case PIPE_CAP_PRIMITIVE_RESTART:
      return 1;

   case PIPE_CAP_BINDLESS_TEXTURE:
      if (zink_descriptor_mode == ZINK_DESCRIPTOR_MODE_DB &&
          (screen->info.db_props.maxDescriptorBufferBindings < 2 || screen->info.db_props.maxSamplerDescriptorBufferBindings < 2))
         return 0;
      return screen->info.have_EXT_descriptor_indexing;

   case PIPE_CAP_TEXTURE_BUFFER_OFFSET_ALIGNMENT:
      return screen->info.props.limits.minTexelBufferOffsetAlignment;

   case PIPE_CAP_TEXTURE_TRANSFER_MODES: {
      enum pipe_texture_transfer_mode mode = PIPE_TEXTURE_TRANSFER_BLIT;
      if (!screen->is_cpu &&
          /* this needs substantial perf tuning */
          screen->info.driver_props.driverID != VK_DRIVER_ID_MESA_TURNIP &&
          screen->info.have_KHR_8bit_storage &&
          screen->info.have_KHR_16bit_storage &&
          screen->info.have_KHR_shader_float16_int8)
         mode |= PIPE_TEXTURE_TRANSFER_COMPUTE;
      return mode;
   }

   case PIPE_CAP_MAX_TEXEL_BUFFER_ELEMENTS_UINT:
      return MIN2(get_smallest_buffer_heap(screen),
                  screen->info.props.limits.maxTexelBufferElements);

   case PIPE_CAP_ENDIANNESS:
      return PIPE_ENDIAN_NATIVE; /* unsure */

   case PIPE_CAP_MAX_VIEWPORTS:
      return MIN2(screen->info.props.limits.maxViewports, PIPE_MAX_VIEWPORTS);

   case PIPE_CAP_IMAGE_LOAD_FORMATTED:
      return screen->info.feats.features.shaderStorageImageReadWithoutFormat;

   case PIPE_CAP_IMAGE_STORE_FORMATTED:
      return screen->info.feats.features.shaderStorageImageWriteWithoutFormat;

   case PIPE_CAP_MIXED_FRAMEBUFFER_SIZES:
      return 1;

   case PIPE_CAP_MAX_GEOMETRY_OUTPUT_VERTICES:
      return screen->info.props.limits.maxGeometryOutputVertices;
   case PIPE_CAP_MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS:
      return screen->info.props.limits.maxGeometryTotalOutputComponents;

   case PIPE_CAP_MAX_TEXTURE_GATHER_COMPONENTS:
      return 4;

   case PIPE_CAP_MIN_TEXTURE_GATHER_OFFSET:
      return screen->info.props.limits.minTexelGatherOffset;
   case PIPE_CAP_MAX_TEXTURE_GATHER_OFFSET:
      return screen->info.props.limits.maxTexelGatherOffset;

   case PIPE_CAP_SAMPLER_REDUCTION_MINMAX_ARB:
      return screen->info.feats12.samplerFilterMinmax || screen->info.have_EXT_sampler_filter_minmax;

   case PIPE_CAP_OPENCL_INTEGER_FUNCTIONS:
   case PIPE_CAP_INTEGER_MULTIPLY_32X16:
      return screen->info.have_INTEL_shader_integer_functions2;

   case PIPE_CAP_FS_FINE_DERIVATIVE:
      return 1;

   case PIPE_CAP_VENDOR_ID:
      return screen->info.props.vendorID;
   case PIPE_CAP_DEVICE_ID:
      return screen->info.props.deviceID;

   case PIPE_CAP_ACCELERATED:
      return !screen->is_cpu;
   case PIPE_CAP_VIDEO_MEMORY:
      return get_video_mem(screen) >> 20;
   case PIPE_CAP_UMA:
      return screen->info.props.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU;

   case PIPE_CAP_MAX_VERTEX_ATTRIB_STRIDE:
      return screen->info.props.limits.maxVertexInputBindingStride;

   case PIPE_CAP_SAMPLER_VIEW_TARGET:
      return 1;

   case PIPE_CAP_VS_LAYER_VIEWPORT:
   case PIPE_CAP_TES_LAYER_VIEWPORT:
      return screen->info.have_EXT_shader_viewport_index_layer ||
             (screen->spirv_version >= SPIRV_VERSION(1, 5) &&
              screen->info.feats12.shaderOutputLayer &&
              screen->info.feats12.shaderOutputViewportIndex);

   case PIPE_CAP_TEXTURE_FLOAT_LINEAR:
      return have_fp32_filter_linear(screen);

   case PIPE_CAP_TEXTURE_HALF_FLOAT_LINEAR:
      return 1;

   case PIPE_CAP_SHADER_BUFFER_OFFSET_ALIGNMENT:
      return screen->info.props.limits.minStorageBufferOffsetAlignment;

   case PIPE_CAP_PCI_GROUP:
   case PIPE_CAP_PCI_BUS:
   case PIPE_CAP_PCI_DEVICE:
   case PIPE_CAP_PCI_FUNCTION:
      return 0; /* TODO: figure these out */

   case PIPE_CAP_CULL_DISTANCE:
      return screen->info.feats.features.shaderCullDistance;

   case PIPE_CAP_SPARSE_BUFFER_PAGE_SIZE:
      return screen->info.feats.features.sparseResidencyBuffer ? ZINK_SPARSE_BUFFER_PAGE_SIZE : 0;

   /* Sparse texture */
   case PIPE_CAP_MAX_SPARSE_TEXTURE_SIZE:
      return screen->info.feats.features.sparseResidencyImage2D ?
         zink_get_param(pscreen, PIPE_CAP_MAX_TEXTURE_2D_SIZE) : 0;
   case PIPE_CAP_MAX_SPARSE_3D_TEXTURE_SIZE:
      return screen->info.feats.features.sparseResidencyImage3D ?
         (1 << (zink_get_param(pscreen, PIPE_CAP_MAX_TEXTURE_3D_LEVELS) - 1)) : 0;
   case PIPE_CAP_MAX_SPARSE_ARRAY_TEXTURE_LAYERS:
      return screen->info.feats.features.sparseResidencyImage2D ?
         zink_get_param(pscreen, PIPE_CAP_MAX_TEXTURE_ARRAY_LAYERS) : 0;
   case PIPE_CAP_SPARSE_TEXTURE_FULL_ARRAY_CUBE_MIPMAPS:
      return screen->info.feats.features.sparseResidencyImage2D ? 1 : 0;
   case PIPE_CAP_QUERY_SPARSE_TEXTURE_RESIDENCY:
      return screen->info.feats.features.sparseResidency2Samples &&
             screen->info.feats.features.shaderResourceResidency ? 1 : 0;
   case PIPE_CAP_CLAMP_SPARSE_TEXTURE_LOD:
      return screen->info.feats.features.shaderResourceMinLod &&
             screen->info.feats.features.sparseResidency2Samples &&
             screen->info.feats.features.shaderResourceResidency ? 1 : 0;

   case PIPE_CAP_VIEWPORT_SUBPIXEL_BITS:
      return screen->info.props.limits.viewportSubPixelBits;

   case PIPE_CAP_MAX_GS_INVOCATIONS:
      return screen->info.props.limits.maxGeometryShaderInvocations;

   case PIPE_CAP_MAX_COMBINED_SHADER_BUFFERS:
      /* gallium handles this automatically */
      return 0;

   case PIPE_CAP_MAX_SHADER_BUFFER_SIZE_UINT:
      /* 1<<27 is required by VK spec */
      assert(screen->info.props.limits.maxStorageBufferRange >= 1 << 27);
      /* clamp to VK spec minimum */
      return MIN2(get_smallest_buffer_heap(screen), screen->info.props.limits.maxStorageBufferRange);

   case PIPE_CAP_FS_COORD_ORIGIN_UPPER_LEFT:
   case PIPE_CAP_FS_COORD_PIXEL_CENTER_HALF_INTEGER:
      return 1;

   case PIPE_CAP_FS_COORD_ORIGIN_LOWER_LEFT:
   case PIPE_CAP_FS_COORD_PIXEL_CENTER_INTEGER:
      return 0;

   case PIPE_CAP_NIR_COMPACT_ARRAYS:
      return 1;

   case PIPE_CAP_FS_FACE_IS_INTEGER_SYSVAL:
   case PIPE_CAP_FS_POINT_IS_SYSVAL:
      return 1;

   case PIPE_CAP_VIEWPORT_TRANSFORM_LOWERED:
      return 1;

   case PIPE_CAP_FLATSHADE:
   case PIPE_CAP_ALPHA_TEST:
   case PIPE_CAP_CLIP_PLANES:
   case PIPE_CAP_POINT_SIZE_FIXED:
   case PIPE_CAP_TWO_SIDED_COLOR:
      return 0;

   case PIPE_CAP_MAX_SHADER_PATCH_VARYINGS:
      return screen->info.props.limits.maxTessellationControlPerPatchOutputComponents / 4;
   case PIPE_CAP_MAX_VARYINGS:
      /* need to reserve up to 60 of our varying components and 16 slots for streamout */
      return MIN2(screen->info.props.limits.maxVertexOutputComponents / 4 / 2, 16);

   case PIPE_CAP_DMABUF:
#if defined(HAVE_LIBDRM) && (DETECT_OS_LINUX || DETECT_OS_BSD)
      return screen->info.have_KHR_external_memory_fd &&
             screen->info.have_EXT_external_memory_dma_buf &&
             screen->info.have_EXT_queue_family_foreign
             ? DRM_PRIME_CAP_IMPORT | DRM_PRIME_CAP_EXPORT
             : 0;
#else
      return 0;
#endif

   case PIPE_CAP_DEPTH_BOUNDS_TEST:
      return screen->info.feats.features.depthBounds;

   case PIPE_CAP_POST_DEPTH_COVERAGE:
      return screen->info.have_EXT_post_depth_coverage;

   case PIPE_CAP_STRING_MARKER:
      return screen->instance_info.have_EXT_debug_utils;

   default:
      return u_pipe_screen_get_param_defaults(pscreen, param);
   }
}

static float
zink_get_paramf(struct pipe_screen *pscreen, enum pipe_capf param)
{
   struct zink_screen *screen = zink_screen(pscreen);

   switch (param) {
   case PIPE_CAPF_MIN_LINE_WIDTH:
   case PIPE_CAPF_MIN_LINE_WIDTH_AA:
      if (!screen->info.feats.features.wideLines)
         return 1.0f;
      return MAX2(screen->info.props.limits.lineWidthRange[0], 0.01);

   case PIPE_CAPF_MIN_POINT_SIZE:
   case PIPE_CAPF_MIN_POINT_SIZE_AA:
      if (!screen->info.feats.features.largePoints)
         return 1.0f;
      return MAX2(screen->info.props.limits.pointSizeRange[0], 0.01);


   case PIPE_CAPF_LINE_WIDTH_GRANULARITY:
      if (!screen->info.feats.features.wideLines)
         return 0.1f;
      return screen->info.props.limits.lineWidthGranularity;

   case PIPE_CAPF_POINT_SIZE_GRANULARITY:
      if (!screen->info.feats.features.largePoints)
         return 0.1f;
      return screen->info.props.limits.pointSizeGranularity;


   case PIPE_CAPF_MAX_LINE_WIDTH:
   case PIPE_CAPF_MAX_LINE_WIDTH_AA:
      if (!screen->info.feats.features.wideLines)
         return 1.0f;
      return screen->info.props.limits.lineWidthRange[1];

   case PIPE_CAPF_MAX_POINT_SIZE:
   case PIPE_CAPF_MAX_POINT_SIZE_AA:
      if (!screen->info.feats.features.largePoints)
         return 1.0f;
      return screen->info.props.limits.pointSizeRange[1];

   case PIPE_CAPF_MAX_TEXTURE_ANISOTROPY:
      if (!screen->info.feats.features.samplerAnisotropy)
         return 1.0f;
      return screen->info.props.limits.maxSamplerAnisotropy;

   case PIPE_CAPF_MAX_TEXTURE_LOD_BIAS:
      return screen->info.props.limits.maxSamplerLodBias;

   case PIPE_CAPF_MIN_CONSERVATIVE_RASTER_DILATE:
   case PIPE_CAPF_MAX_CONSERVATIVE_RASTER_DILATE:
   case PIPE_CAPF_CONSERVATIVE_RASTER_DILATE_GRANULARITY:
      return 0.0f; /* not implemented */
   }

   /* should only get here on unhandled cases */
   return 0.0f;
}

static int
zink_get_shader_param(struct pipe_screen *pscreen,
                       gl_shader_stage shader,
                       enum pipe_shader_cap param)
{
   struct zink_screen *screen = zink_screen(pscreen);

   switch (param) {
   case PIPE_SHADER_CAP_MAX_INSTRUCTIONS:
      switch (shader) {
      case MESA_SHADER_FRAGMENT:
      case MESA_SHADER_VERTEX:
         return INT_MAX;
      case MESA_SHADER_TESS_CTRL:
      case MESA_SHADER_TESS_EVAL:
         if (screen->info.feats.features.tessellationShader &&
             screen->info.have_KHR_maintenance2)
            return INT_MAX;
         break;

      case MESA_SHADER_GEOMETRY:
         if (screen->info.feats.features.geometryShader)
            return INT_MAX;
         break;

      case MESA_SHADER_COMPUTE:
         return INT_MAX;
      default:
         break;
      }
      return 0;
   case PIPE_SHADER_CAP_MAX_ALU_INSTRUCTIONS:
   case PIPE_SHADER_CAP_MAX_TEX_INSTRUCTIONS:
   case PIPE_SHADER_CAP_MAX_TEX_INDIRECTIONS:
   case PIPE_SHADER_CAP_MAX_CONTROL_FLOW_DEPTH:
      return INT_MAX;

   case PIPE_SHADER_CAP_MAX_INPUTS: {
      uint32_t max = 0;
      switch (shader) {
      case MESA_SHADER_VERTEX:
         max = MIN2(screen->info.props.limits.maxVertexInputAttributes, PIPE_MAX_ATTRIBS);
         break;
      case MESA_SHADER_TESS_CTRL:
         max = screen->info.props.limits.maxTessellationControlPerVertexInputComponents / 4;
         break;
      case MESA_SHADER_TESS_EVAL:
         max = screen->info.props.limits.maxTessellationEvaluationInputComponents / 4;
         break;
      case MESA_SHADER_GEOMETRY:
         max = screen->info.props.limits.maxGeometryInputComponents / 4;
         break;
      case MESA_SHADER_FRAGMENT:
         /* intel drivers report fewer components, but it's a value that's compatible
          * with what we need for GL, so we can still force a conformant value here
          */
         if (screen->info.driver_props.driverID == VK_DRIVER_ID_INTEL_OPEN_SOURCE_MESA ||
             screen->info.driver_props.driverID == VK_DRIVER_ID_INTEL_PROPRIETARY_WINDOWS ||
             (screen->info.driver_props.driverID == VK_DRIVER_ID_MESA_VENUS
              && screen->info.props.vendorID == 0x8086))
            return 32;
         max = screen->info.props.limits.maxFragmentInputComponents / 4;
         break;
      default:
         return 0; /* unsupported stage */
      }
      switch (shader) {
      case MESA_SHADER_VERTEX:
      case MESA_SHADER_TESS_EVAL:
      case MESA_SHADER_GEOMETRY:
         /* last vertex stage must support streamout, and this is capped in glsl compiler */
         return MIN2(max, MAX_VARYING);
      default: break;
      }
      return MIN2(max, 64); // prevent overflowing struct shader_info::inputs_read
   }

   case PIPE_SHADER_CAP_MAX_OUTPUTS: {
      uint32_t max = 0;
      switch (shader) {
      case MESA_SHADER_VERTEX:
         max = screen->info.props.limits.maxVertexOutputComponents / 4;
         break;
      case MESA_SHADER_TESS_CTRL:
         max = screen->info.props.limits.maxTessellationControlPerVertexOutputComponents / 4;
         break;
      case MESA_SHADER_TESS_EVAL:
         max = screen->info.props.limits.maxTessellationEvaluationOutputComponents / 4;
         break;
      case MESA_SHADER_GEOMETRY:
         max = screen->info.props.limits.maxGeometryOutputComponents / 4;
         break;
      case MESA_SHADER_FRAGMENT:
         max = screen->info.props.limits.maxColorAttachments;
         break;
      default:
         return 0; /* unsupported stage */
      }
      return MIN2(max, 64); // prevent overflowing struct shader_info::outputs_read/written
   }

   case PIPE_SHADER_CAP_MAX_CONST_BUFFER0_SIZE:
      /* At least 16384 is guaranteed by VK spec */
      assert(screen->info.props.limits.maxUniformBufferRange >= 16384);
      /* but Gallium can't handle values that are too big */
      return MIN3(get_smallest_buffer_heap(screen),
                  screen->info.props.limits.maxUniformBufferRange, BITFIELD_BIT(31));

   case PIPE_SHADER_CAP_MAX_CONST_BUFFERS:
      return  MIN2(screen->info.props.limits.maxPerStageDescriptorUniformBuffers,
                   PIPE_MAX_CONSTANT_BUFFERS);

   case PIPE_SHADER_CAP_MAX_TEMPS:
      return INT_MAX;

   case PIPE_SHADER_CAP_INTEGERS:
      return 1;

   case PIPE_SHADER_CAP_INDIRECT_CONST_ADDR:
   case PIPE_SHADER_CAP_INDIRECT_TEMP_ADDR:
   case PIPE_SHADER_CAP_INDIRECT_INPUT_ADDR:
   case PIPE_SHADER_CAP_INDIRECT_OUTPUT_ADDR:
      return 1;

   case PIPE_SHADER_CAP_SUBROUTINES:
   case PIPE_SHADER_CAP_INT64_ATOMICS:
   case PIPE_SHADER_CAP_GLSL_16BIT_CONSTS:
      return 0; /* not implemented */

   case PIPE_SHADER_CAP_FP16_CONST_BUFFERS:
      //enabling this breaks GTF-GL46.gtf21.GL2Tests.glGetUniform.glGetUniform
      //return screen->info.feats11.uniformAndStorageBuffer16BitAccess ||
             //(screen->info.have_KHR_16bit_storage && screen->info.storage_16bit_feats.uniformAndStorageBuffer16BitAccess);
      return 0;
   case PIPE_SHADER_CAP_FP16_DERIVATIVES:
      return 0; //spirv requires 32bit derivative srcs and dests
   case PIPE_SHADER_CAP_FP16:
      return screen->info.feats12.shaderFloat16 ||
             (screen->info.have_KHR_shader_float16_int8 &&
              screen->info.shader_float16_int8_feats.shaderFloat16);

   case PIPE_SHADER_CAP_INT16:
      return screen->info.feats.features.shaderInt16;

   case PIPE_SHADER_CAP_TGSI_SQRT_SUPPORTED:
      return 0; /* not implemented */

   case PIPE_SHADER_CAP_MAX_TEXTURE_SAMPLERS:
   case PIPE_SHADER_CAP_MAX_SAMPLER_VIEWS:
      return MIN2(MIN2(screen->info.props.limits.maxPerStageDescriptorSamplers,
                       screen->info.props.limits.maxPerStageDescriptorSampledImages),
                  PIPE_MAX_SAMPLERS);

   case PIPE_SHADER_CAP_TGSI_ANY_INOUT_DECL_RANGE:
      return 0; /* no idea */

   case PIPE_SHADER_CAP_MAX_SHADER_BUFFERS:
      switch (shader) {
      case MESA_SHADER_VERTEX:
      case MESA_SHADER_TESS_CTRL:
      case MESA_SHADER_TESS_EVAL:
      case MESA_SHADER_GEOMETRY:
         if (!screen->info.feats.features.vertexPipelineStoresAndAtomics)
            return 0;
         break;

      case MESA_SHADER_FRAGMENT:
         if (!screen->info.feats.features.fragmentStoresAndAtomics)
            return 0;
         break;

      default:
         break;
      }

      /* TODO: this limitation is dumb, and will need some fixes in mesa */
      return MIN2(screen->info.props.limits.maxPerStageDescriptorStorageBuffers, PIPE_MAX_SHADER_BUFFERS);

   case PIPE_SHADER_CAP_SUPPORTED_IRS:
      return (1 << PIPE_SHADER_IR_NIR) | (1 << PIPE_SHADER_IR_TGSI);

   case PIPE_SHADER_CAP_MAX_SHADER_IMAGES:
      if (screen->info.feats.features.shaderStorageImageExtendedFormats &&
          screen->info.feats.features.shaderStorageImageWriteWithoutFormat)
         return MIN2(screen->info.props.limits.maxPerStageDescriptorStorageImages,
                     ZINK_MAX_SHADER_IMAGES);
      return 0;

   case PIPE_SHADER_CAP_MAX_HW_ATOMIC_COUNTERS:
   case PIPE_SHADER_CAP_MAX_HW_ATOMIC_COUNTER_BUFFERS:
      return 0; /* not implemented */
   case PIPE_SHADER_CAP_CONT_SUPPORTED:
      return 1;
   }

   /* should only get here on unhandled cases */
   return 0;
}

static VkSampleCountFlagBits
vk_sample_count_flags(uint32_t sample_count)
{
   switch (sample_count) {
   case 1: return VK_SAMPLE_COUNT_1_BIT;
   case 2: return VK_SAMPLE_COUNT_2_BIT;
   case 4: return VK_SAMPLE_COUNT_4_BIT;
   case 8: return VK_SAMPLE_COUNT_8_BIT;
   case 16: return VK_SAMPLE_COUNT_16_BIT;
   case 32: return VK_SAMPLE_COUNT_32_BIT;
   case 64: return VK_SAMPLE_COUNT_64_BIT;
   default:
      return 0;
   }
}

static bool
zink_is_compute_copy_faster(struct pipe_screen *pscreen,
                            enum pipe_format src_format,
                            enum pipe_format dst_format,
                            unsigned width,
                            unsigned height,
                            unsigned depth,
                            bool cpu)
{
   if (cpu)
      /* very basic for now, probably even worse for some cases,
       * but fixes lots of others
       */
      return width * height * depth > 64 * 64;
   return false;
}

static bool
zink_is_format_supported(struct pipe_screen *pscreen,
                         enum pipe_format format,
                         enum pipe_texture_target target,
                         unsigned sample_count,
                         unsigned storage_sample_count,
                         unsigned bind)
{
   struct zink_screen *screen = zink_screen(pscreen);

   if (storage_sample_count && !screen->info.feats.features.shaderStorageImageMultisample && bind & PIPE_BIND_SHADER_IMAGE)
      return false;

   if (format == PIPE_FORMAT_NONE)
      return screen->info.props.limits.framebufferNoAttachmentsSampleCounts &
             vk_sample_count_flags(sample_count);

   if (bind & PIPE_BIND_INDEX_BUFFER) {
      if (format == PIPE_FORMAT_R8_UINT &&
          !screen->info.have_EXT_index_type_uint8)
         return false;
      if (format != PIPE_FORMAT_R8_UINT &&
          format != PIPE_FORMAT_R16_UINT &&
          format != PIPE_FORMAT_R32_UINT)
         return false;
   }

   /* always use superset to determine feature support */
   VkFormat vkformat = zink_get_format(screen, PIPE_FORMAT_A8_UNORM ? zink_format_get_emulated_alpha(format) : format);
   if (vkformat == VK_FORMAT_UNDEFINED)
      return false;

   if (sample_count >= 1) {
      VkSampleCountFlagBits sample_mask = vk_sample_count_flags(sample_count);
      if (!sample_mask)
         return false;
      const struct util_format_description *desc = util_format_description(format);
      if (util_format_is_depth_or_stencil(format)) {
         if (util_format_has_depth(desc)) {
            if (bind & PIPE_BIND_DEPTH_STENCIL &&
                (screen->info.props.limits.framebufferDepthSampleCounts & sample_mask) != sample_mask)
               return false;
            if (bind & PIPE_BIND_SAMPLER_VIEW &&
                (screen->info.props.limits.sampledImageDepthSampleCounts & sample_mask) != sample_mask)
               return false;
         }
         if (util_format_has_stencil(desc)) {
            if (bind & PIPE_BIND_DEPTH_STENCIL &&
                (screen->info.props.limits.framebufferStencilSampleCounts & sample_mask) != sample_mask)
               return false;
            if (bind & PIPE_BIND_SAMPLER_VIEW &&
                (screen->info.props.limits.sampledImageStencilSampleCounts & sample_mask) != sample_mask)
               return false;
         }
      } else if (util_format_is_pure_integer(format)) {
         if (bind & PIPE_BIND_RENDER_TARGET &&
             !(screen->info.props.limits.framebufferColorSampleCounts & sample_mask))
            return false;
         if (bind & PIPE_BIND_SAMPLER_VIEW &&
             !(screen->info.props.limits.sampledImageIntegerSampleCounts & sample_mask))
            return false;
      } else {
         if (bind & PIPE_BIND_RENDER_TARGET &&
             !(screen->info.props.limits.framebufferColorSampleCounts & sample_mask))
            return false;
         if (bind & PIPE_BIND_SAMPLER_VIEW &&
             !(screen->info.props.limits.sampledImageColorSampleCounts & sample_mask))
            return false;
      }
      if (bind & PIPE_BIND_SHADER_IMAGE) {
          if (!(screen->info.props.limits.storageImageSampleCounts & sample_mask))
             return false;
      }
   }

   struct zink_format_props props = screen->format_props[format];

   if (target == PIPE_BUFFER) {
      if (bind & PIPE_BIND_VERTEX_BUFFER) {
         if (!(props.bufferFeatures & VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT)) {
            enum pipe_format new_format = zink_decompose_vertex_format(format);
            if (!new_format)
               return false;
            if (!(screen->format_props[new_format].bufferFeatures & VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT))
               return false;
         }
      }

      if (bind & PIPE_BIND_SAMPLER_VIEW &&
         !(props.bufferFeatures & VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT))
            return false;

      if (bind & PIPE_BIND_SHADER_IMAGE &&
          !(props.bufferFeatures & VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_BIT))
         return false;
   } else {
      /* all other targets are texture-targets */
      if (bind & PIPE_BIND_RENDER_TARGET &&
          !(props.optimalTilingFeatures & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT))
         return false;

      if (bind & PIPE_BIND_BLENDABLE &&
         !(props.optimalTilingFeatures & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT))
        return false;

      if (bind & PIPE_BIND_SAMPLER_VIEW &&
         !(props.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT))
            return false;

      if (bind & PIPE_BIND_SAMPLER_REDUCTION_MINMAX &&
          !(props.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_MINMAX_BIT))
         return false;

      if ((bind & PIPE_BIND_SAMPLER_VIEW) || (bind & PIPE_BIND_RENDER_TARGET)) {
         /* if this is a 3-component texture, force gallium to give us 4 components by rejecting this one */
         const struct util_format_description *desc = util_format_description(format);
         if (desc->nr_channels == 3 &&
             (desc->block.bits == 24 || desc->block.bits == 48 || desc->block.bits == 96))
            return false;
      }

      if (bind & PIPE_BIND_DEPTH_STENCIL &&
          !(props.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT))
         return false;

      if (bind & PIPE_BIND_SHADER_IMAGE &&
          !(props.optimalTilingFeatures & VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT))
         return false;
   }

   return true;
}

static void
zink_destroy_screen(struct pipe_screen *pscreen)
{
   struct zink_screen *screen = zink_screen(pscreen);
   struct zink_batch_state *bs = screen->free_batch_states;
   while (bs) {
      struct zink_batch_state *bs_next = bs->next;
      zink_batch_state_destroy(screen, bs);
      bs = bs_next;
   }

#ifdef HAVE_RENDERDOC_APP_H
   if (screen->renderdoc_capture_all && p_atomic_dec_zero(&num_screens))
      screen->renderdoc_api->EndFrameCapture(RENDERDOC_DEVICEPOINTER_FROM_VKINSTANCE(screen->instance), NULL);
#endif

   hash_table_foreach(&screen->dts, entry)
      zink_kopper_deinit_displaytarget(screen, entry->data);

   if (screen->copy_context)
      screen->copy_context->base.destroy(&screen->copy_context->base);

   if (VK_NULL_HANDLE != screen->debugUtilsCallbackHandle) {
      VKSCR(DestroyDebugUtilsMessengerEXT)(screen->instance, screen->debugUtilsCallbackHandle, NULL);
   }

   util_vertex_state_cache_deinit(&screen->vertex_state_cache);

   if (screen->gfx_push_constant_layout)
      VKSCR(DestroyPipelineLayout)(screen->dev, screen->gfx_push_constant_layout, NULL);

   u_transfer_helper_destroy(pscreen->transfer_helper);
   if (util_queue_is_initialized(&screen->cache_get_thread)) {
      util_queue_finish(&screen->cache_get_thread);
      util_queue_destroy(&screen->cache_get_thread);
   }
#ifdef ENABLE_SHADER_CACHE
   if (screen->disk_cache && util_queue_is_initialized(&screen->cache_put_thread)) {
      util_queue_finish(&screen->cache_put_thread);
      disk_cache_wait_for_idle(screen->disk_cache);
      util_queue_destroy(&screen->cache_put_thread);
   }
#endif
   disk_cache_destroy(screen->disk_cache);

   /* we don't have an API to check if a set is already initialized */
   for (unsigned i = 0; i < ARRAY_SIZE(screen->pipeline_libs); i++)
      if (screen->pipeline_libs[i].table)
         _mesa_set_clear(&screen->pipeline_libs[i], NULL);

   zink_bo_deinit(screen);
   util_live_shader_cache_deinit(&screen->shaders);

   zink_descriptor_layouts_deinit(screen);

   if (screen->sem)
      VKSCR(DestroySemaphore)(screen->dev, screen->sem, NULL);

   if (screen->fence)
      VKSCR(DestroyFence)(screen->dev, screen->fence, NULL);

   if (util_queue_is_initialized(&screen->flush_queue))
      util_queue_destroy(&screen->flush_queue);

   while (util_dynarray_contains(&screen->semaphores, VkSemaphore))
      VKSCR(DestroySemaphore)(screen->dev, util_dynarray_pop(&screen->semaphores, VkSemaphore), NULL);
   while (util_dynarray_contains(&screen->fd_semaphores, VkSemaphore))
      VKSCR(DestroySemaphore)(screen->dev, util_dynarray_pop(&screen->fd_semaphores, VkSemaphore), NULL);
   if (screen->bindless_layout)
      VKSCR(DestroyDescriptorSetLayout)(screen->dev, screen->bindless_layout, NULL);

   if (screen->dev)
      VKSCR(DestroyDevice)(screen->dev, NULL);

   if (screen->instance)
      VKSCR(DestroyInstance)(screen->instance, NULL);

   util_idalloc_mt_fini(&screen->buffer_ids);

   if (screen->loader_lib)
      util_dl_close(screen->loader_lib);

   if (screen->drm_fd != -1)
      close(screen->drm_fd);

   slab_destroy_parent(&screen->transfer_pool);
   ralloc_free(screen);
   glsl_type_singleton_decref();
}

static int
zink_get_display_device(const struct zink_screen *screen, uint32_t pdev_count,
                        const VkPhysicalDevice *pdevs, int64_t dev_major,
                        int64_t dev_minor)
{
   VkPhysicalDeviceDrmPropertiesEXT drm_props = {
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DRM_PROPERTIES_EXT,
   };
   VkPhysicalDeviceProperties2 props = {
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2,
      .pNext = &drm_props,
   };

   for (uint32_t i = 0; i < pdev_count; ++i) {
      VKSCR(GetPhysicalDeviceProperties2)(pdevs[i], &props);
      if (drm_props.renderMajor == dev_major &&
          drm_props.renderMinor == dev_minor)
         return i;
   }

   return -1;
}

static int
zink_get_cpu_device_type(const struct zink_screen *screen, uint32_t pdev_count,
                         const VkPhysicalDevice *pdevs)
{
   VkPhysicalDeviceProperties props;

   for (uint32_t i = 0; i < pdev_count; ++i) {
      VKSCR(GetPhysicalDeviceProperties)(pdevs[i], &props);

      /* if user wants cpu, only give them cpu */
      if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_CPU)
         return i;
   }

   mesa_loge("ZINK: CPU device requested but none found!");

   return -1;
}

static void
choose_pdev(struct zink_screen *screen, int64_t dev_major, int64_t dev_minor)
{
   bool cpu = debug_get_bool_option("LIBGL_ALWAYS_SOFTWARE", false) ||
              debug_get_bool_option("D3D_ALWAYS_SOFTWARE", false);

   if (cpu || (dev_major > 0 && dev_major < 255)) {
      uint32_t pdev_count;
      int idx;
      VkPhysicalDevice *pdevs;
      VkResult result = VKSCR(EnumeratePhysicalDevices)(screen->instance, &pdev_count, NULL);
      if (result != VK_SUCCESS) {
         mesa_loge("ZINK: vkEnumeratePhysicalDevices failed (%s)", vk_Result_to_str(result));
         return;
      }

      assert(pdev_count > 0);

      pdevs = malloc(sizeof(*pdevs) * pdev_count);
      if (!pdevs) {
         mesa_loge("ZINK: failed to allocate pdevs!");
         return;
      }
      result = VKSCR(EnumeratePhysicalDevices)(screen->instance, &pdev_count, pdevs);
      assert(result == VK_SUCCESS);
      assert(pdev_count > 0);

      if (cpu)
         idx = zink_get_cpu_device_type(screen, pdev_count, pdevs);
      else
         idx = zink_get_display_device(screen, pdev_count, pdevs, dev_major,
                                       dev_minor);

      if (idx != -1)
         /* valid cpu device */
         screen->pdev = pdevs[idx];

      free(pdevs);

      if (idx == -1)
         return;

   } else {
      VkPhysicalDevice pdev;
      unsigned pdev_count = 1;
      VkResult result = VKSCR(EnumeratePhysicalDevices)(screen->instance, &pdev_count, &pdev);
      if (result != VK_SUCCESS && result != VK_INCOMPLETE) {
         mesa_loge("ZINK: vkEnumeratePhysicalDevices failed (%s)", vk_Result_to_str(result));
         return;
      }
      screen->pdev = pdev;
   }
   VKSCR(GetPhysicalDeviceProperties)(screen->pdev, &screen->info.props);

   /* allow software rendering only if forced by the user */
   if (!cpu && screen->info.props.deviceType == VK_PHYSICAL_DEVICE_TYPE_CPU) {
      screen->pdev = VK_NULL_HANDLE;
      return;
   }

   screen->info.device_version = screen->info.props.apiVersion;

   /* runtime version is the lesser of the instance version and device version */
   screen->vk_version = MIN2(screen->info.device_version, screen->instance_info.loader_version);

   /* calculate SPIR-V version based on VK version */
   if (screen->vk_version >= VK_MAKE_VERSION(1, 3, 0))
      screen->spirv_version = SPIRV_VERSION(1, 6);
   else if (screen->vk_version >= VK_MAKE_VERSION(1, 2, 0))
      screen->spirv_version = SPIRV_VERSION(1, 5);
   else if (screen->vk_version >= VK_MAKE_VERSION(1, 1, 0))
      screen->spirv_version = SPIRV_VERSION(1, 3);
   else
      screen->spirv_version = SPIRV_VERSION(1, 0);
}

static void
update_queue_props(struct zink_screen *screen)
{
   uint32_t num_queues;
   VKSCR(GetPhysicalDeviceQueueFamilyProperties)(screen->pdev, &num_queues, NULL);
   assert(num_queues > 0);

   VkQueueFamilyProperties *props = malloc(sizeof(*props) * num_queues);
   if (!props) {
      mesa_loge("ZINK: failed to allocate props!");
      return;
   }
      
   VKSCR(GetPhysicalDeviceQueueFamilyProperties)(screen->pdev, &num_queues, props);

   bool found_gfx = false;
   uint32_t sparse_only = UINT32_MAX;
   screen->sparse_queue = UINT32_MAX;
   for (uint32_t i = 0; i < num_queues; i++) {
      if (props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
         if (found_gfx)
            continue;
         screen->sparse_queue = screen->gfx_queue = i;
         screen->max_queues = props[i].queueCount;
         screen->timestamp_valid_bits = props[i].timestampValidBits;
         found_gfx = true;
      } else if (props[i].queueFlags & VK_QUEUE_SPARSE_BINDING_BIT)
         sparse_only = i;
   }
   if (sparse_only != UINT32_MAX)
      screen->sparse_queue = sparse_only;
   free(props);
}

static void
init_queue(struct zink_screen *screen)
{
   simple_mtx_init(&screen->queue_lock, mtx_plain);
   VKSCR(GetDeviceQueue)(screen->dev, screen->gfx_queue, 0, &screen->queue);
   if (screen->sparse_queue != screen->gfx_queue)
      VKSCR(GetDeviceQueue)(screen->dev, screen->sparse_queue, 0, &screen->queue_sparse);
   else
      screen->queue_sparse = screen->queue;
}

static void
zink_flush_frontbuffer(struct pipe_screen *pscreen,
                       struct pipe_context *pctx,
                       struct pipe_resource *pres,
                       unsigned level, unsigned layer,
                       void *winsys_drawable_handle,
                       struct pipe_box *sub_box)
{
   struct zink_screen *screen = zink_screen(pscreen);
   struct zink_resource *res = zink_resource(pres);
   struct zink_context *ctx = zink_context(pctx);

   /* if the surface is no longer a swapchain, this is a no-op */
   if (!zink_is_swapchain(res))
      return;

   ctx = zink_tc_context_unwrap(pctx, screen->threaded);

   if (!zink_kopper_acquired(res->obj->dt, res->obj->dt_idx)) {
      /* swapbuffers to an undefined surface: acquire and present garbage */
      zink_kopper_acquire(ctx, res, UINT64_MAX);
      ctx->needs_present = res;
      /* set batch usage to submit acquire semaphore */
      zink_batch_resource_usage_set(&ctx->batch, res, true, false);
      /* ensure the resource is set up to present garbage */
      ctx->base.flush_resource(&ctx->base, pres);
   }

   /* handle any outstanding acquire submits (not just from above) */
   if (ctx->batch.swapchain || ctx->needs_present) {
      ctx->batch.has_work = true;
      pctx->flush(pctx, NULL, PIPE_FLUSH_END_OF_FRAME);
      if (ctx->last_fence && screen->threaded_submit) {
         struct zink_batch_state *bs = zink_batch_state(ctx->last_fence);
         util_queue_fence_wait(&bs->flush_completed);
      }
   }

   /* always verify that this was acquired */
   assert(zink_kopper_acquired(res->obj->dt, res->obj->dt_idx));
   zink_kopper_present_queue(screen, res);
}

bool
zink_is_depth_format_supported(struct zink_screen *screen, VkFormat format)
{
   VkFormatProperties props;
   VKSCR(GetPhysicalDeviceFormatProperties)(screen->pdev, format, &props);
   return (props.linearTilingFeatures | props.optimalTilingFeatures) &
          VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;
}

static enum pipe_format
emulate_x8(enum pipe_format format)
{
   /* convert missing Xn variants to An */
   switch (format) {
   case PIPE_FORMAT_B8G8R8X8_UNORM:
      return PIPE_FORMAT_B8G8R8A8_UNORM;

   case PIPE_FORMAT_B8G8R8X8_SRGB:
      return PIPE_FORMAT_B8G8R8A8_SRGB;
   case PIPE_FORMAT_R8G8B8X8_SRGB:
      return PIPE_FORMAT_R8G8B8A8_SRGB;

   case PIPE_FORMAT_R8G8B8X8_SINT:
      return PIPE_FORMAT_R8G8B8A8_SINT;
   case PIPE_FORMAT_R8G8B8X8_SNORM:
      return PIPE_FORMAT_R8G8B8A8_SNORM;
   case PIPE_FORMAT_R8G8B8X8_UNORM:
      return PIPE_FORMAT_R8G8B8A8_UNORM;

   case PIPE_FORMAT_R16G16B16X16_FLOAT:
      return PIPE_FORMAT_R16G16B16A16_FLOAT;
   case PIPE_FORMAT_R16G16B16X16_SINT:
      return PIPE_FORMAT_R16G16B16A16_SINT;
   case PIPE_FORMAT_R16G16B16X16_SNORM:
      return PIPE_FORMAT_R16G16B16A16_SNORM;
   case PIPE_FORMAT_R16G16B16X16_UNORM:
      return PIPE_FORMAT_R16G16B16A16_UNORM;

   case PIPE_FORMAT_R32G32B32X32_FLOAT:
      return PIPE_FORMAT_R32G32B32A32_FLOAT;
   case PIPE_FORMAT_R32G32B32X32_SINT:
      return PIPE_FORMAT_R32G32B32A32_SINT;

   default:
      return format;
   }
}

VkFormat
zink_get_format(struct zink_screen *screen, enum pipe_format format)
{
   if (format == PIPE_FORMAT_A8_UNORM && !screen->driver_workarounds.missing_a8_unorm)
      return VK_FORMAT_A8_UNORM_KHR;
   else if (!screen->driver_workarounds.broken_l4a4 || format != PIPE_FORMAT_L4A4_UNORM)
      format = zink_format_get_emulated_alpha(format);

   VkFormat ret = vk_format_from_pipe_format(emulate_x8(format));

   if (format == PIPE_FORMAT_X32_S8X24_UINT &&
       screen->have_D32_SFLOAT_S8_UINT)
      return VK_FORMAT_D32_SFLOAT_S8_UINT;

   if (format == PIPE_FORMAT_X24S8_UINT)
      /* valid when using aspects to extract stencil,
       * fails format test because it's emulated */
      ret = VK_FORMAT_D24_UNORM_S8_UINT;

   if (ret == VK_FORMAT_X8_D24_UNORM_PACK32 &&
       !screen->have_X8_D24_UNORM_PACK32) {
      assert(zink_is_depth_format_supported(screen, VK_FORMAT_D32_SFLOAT));
      return VK_FORMAT_D32_SFLOAT;
   }

   if (ret == VK_FORMAT_D24_UNORM_S8_UINT &&
       !screen->have_D24_UNORM_S8_UINT) {
      assert(screen->have_D32_SFLOAT_S8_UINT);
      return VK_FORMAT_D32_SFLOAT_S8_UINT;
   }

   if ((ret == VK_FORMAT_A4B4G4R4_UNORM_PACK16 &&
        !screen->info.format_4444_feats.formatA4B4G4R4) ||
       (ret == VK_FORMAT_A4R4G4B4_UNORM_PACK16 &&
        !screen->info.format_4444_feats.formatA4R4G4B4))
      return VK_FORMAT_UNDEFINED;

   if (format == PIPE_FORMAT_R4A4_UNORM)
      return VK_FORMAT_R4G4_UNORM_PACK8;

   return ret;
}

void
zink_convert_color(const struct zink_screen *screen, enum pipe_format format,
                   union pipe_color_union *dst,
                   const union pipe_color_union *src)
{
   const struct util_format_description *desc = util_format_description(format);
   union pipe_color_union tmp = *src;

   for (unsigned i = 0; i < 4; i++)
      zink_format_clamp_channel_color(desc, &tmp, src, i);

   if (zink_format_is_emulated_alpha(format) &&
       /* Don't swizzle colors if the driver supports real A8_UNORM */
       (format != PIPE_FORMAT_A8_UNORM ||
         screen->driver_workarounds.missing_a8_unorm)) {
      if (util_format_is_alpha(format)) {
         tmp.ui[0] = tmp.ui[3];
         tmp.ui[1] = 0;
         tmp.ui[2] = 0;
         tmp.ui[3] = 0;
      } else if (util_format_is_luminance(format)) {
         tmp.ui[1] = 0;
         tmp.ui[2] = 0;
         tmp.f[3] = 1.0;
      } else if (util_format_is_luminance_alpha(format)) {
         tmp.ui[1] = tmp.ui[3];
         tmp.ui[2] = 0;
         tmp.f[3] = 1.0;
      } else /* zink_format_is_red_alpha */ {
         tmp.ui[1] = tmp.ui[3];
         tmp.ui[2] = 0;
         tmp.ui[3] = 0;
      }
   }

   memcpy(dst, &tmp, sizeof(union pipe_color_union));
}

static bool
check_have_device_time(struct zink_screen *screen)
{
   uint32_t num_domains = 0;
   VkTimeDomainEXT domains[8]; //current max is 4
   VkResult result = VKSCR(GetPhysicalDeviceCalibrateableTimeDomainsEXT)(screen->pdev, &num_domains, NULL);
   if (result != VK_SUCCESS) {
      mesa_loge("ZINK: vkGetPhysicalDeviceCalibrateableTimeDomainsEXT failed (%s)", vk_Result_to_str(result));
   }
   assert(num_domains > 0);
   assert(num_domains < ARRAY_SIZE(domains));

   result = VKSCR(GetPhysicalDeviceCalibrateableTimeDomainsEXT)(screen->pdev, &num_domains, domains);
   if (result != VK_SUCCESS) {
      mesa_loge("ZINK: vkGetPhysicalDeviceCalibrateableTimeDomainsEXT failed (%s)", vk_Result_to_str(result));
   }

   /* VK_TIME_DOMAIN_DEVICE_EXT is used for the ctx->get_timestamp hook and is the only one we really need */
   for (unsigned i = 0; i < num_domains; i++) {
      if (domains[i] == VK_TIME_DOMAIN_DEVICE_EXT) {
         return true;
      }
   }

   return false;
}

static void
zink_error(const char *msg)
{
}

static void
zink_warn(const char *msg)
{
}

static void
zink_info(const char *msg)
{
}

static void
zink_msg(const char *msg)
{
}

static VKAPI_ATTR VkBool32 VKAPI_CALL
zink_debug_util_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT           messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT                  messageType,
    const VkDebugUtilsMessengerCallbackDataEXT      *pCallbackData,
    void                                            *pUserData)
{
   // Pick message prefix and color to use.
   // Only MacOS and Linux have been tested for color support
   if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
      zink_error(pCallbackData->pMessage);
   } else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
      zink_warn(pCallbackData->pMessage);
   } else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
      zink_info(pCallbackData->pMessage);
   } else
      zink_msg(pCallbackData->pMessage);

   return VK_FALSE;
}

static bool
create_debug(struct zink_screen *screen)
{
   VkDebugUtilsMessengerCreateInfoEXT vkDebugUtilsMessengerCreateInfoEXT = {
       VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
       NULL,
       0,  // flags
       VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
       VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
       VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
       VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
       VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
       VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
       VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
       zink_debug_util_callback,
       NULL
   };

   VkDebugUtilsMessengerEXT vkDebugUtilsCallbackEXT = VK_NULL_HANDLE;

   VkResult result = VKSCR(CreateDebugUtilsMessengerEXT)(
           screen->instance,
           &vkDebugUtilsMessengerCreateInfoEXT,
           NULL,
           &vkDebugUtilsCallbackEXT);
   if (result != VK_SUCCESS) {
      mesa_loge("ZINK: vkCreateDebugUtilsMessengerEXT failed (%s)", vk_Result_to_str(result));
   }

   screen->debugUtilsCallbackHandle = vkDebugUtilsCallbackEXT;

   return true;
}

static bool
zink_internal_setup_moltenvk(struct zink_screen *screen)
{
#if defined(MVK_VERSION)
   if (!screen->instance_info.have_MVK_moltenvk)
      return true;

   GET_PROC_ADDR_INSTANCE_LOCAL(screen, screen->instance, GetMoltenVKConfigurationMVK);
   GET_PROC_ADDR_INSTANCE_LOCAL(screen, screen->instance, SetMoltenVKConfigurationMVK);
   GET_PROC_ADDR_INSTANCE_LOCAL(screen, screen->instance, GetVersionStringsMVK);

   if (vk_GetVersionStringsMVK) {
      char molten_version[64] = {0};
      char vulkan_version[64] = {0};

      vk_GetVersionStringsMVK(molten_version, sizeof(molten_version) - 1, vulkan_version, sizeof(vulkan_version) - 1);

      printf("zink: MoltenVK %s Vulkan %s \n", molten_version, vulkan_version);
   }

   if (vk_GetMoltenVKConfigurationMVK && vk_SetMoltenVKConfigurationMVK) {
      MVKConfiguration molten_config = {0};
      size_t molten_config_size = sizeof(molten_config);

      VkResult res = vk_GetMoltenVKConfigurationMVK(screen->instance, &molten_config, &molten_config_size);
      if (res == VK_SUCCESS || res == VK_INCOMPLETE) {
         // Needed to allow MoltenVK to accept VkImageView swizzles.
         // Encountered when using VK_FORMAT_R8G8_UNORM
         molten_config.fullImageViewSwizzle = VK_TRUE;
         vk_SetMoltenVKConfigurationMVK(screen->instance, &molten_config, &molten_config_size);
      }
   }
#endif // MVK_VERSION

   return true;
}

static void
check_vertex_formats(struct zink_screen *screen)
{
   /* from vbuf */
   enum pipe_format format_list[] = {
      /* not supported by vk
      PIPE_FORMAT_R32_FIXED,
      PIPE_FORMAT_R32G32_FIXED,
      PIPE_FORMAT_R32G32B32_FIXED,
      PIPE_FORMAT_R32G32B32A32_FIXED,
      */
      PIPE_FORMAT_R16_FLOAT,
      PIPE_FORMAT_R16G16_FLOAT,
      PIPE_FORMAT_R16G16B16_FLOAT,
      PIPE_FORMAT_R16G16B16A16_FLOAT,
      /* not supported by vk
      PIPE_FORMAT_R64_FLOAT,
      PIPE_FORMAT_R64G64_FLOAT,
      PIPE_FORMAT_R64G64B64_FLOAT,
      PIPE_FORMAT_R64G64B64A64_FLOAT,
      PIPE_FORMAT_R32_UNORM,
      PIPE_FORMAT_R32G32_UNORM,
      PIPE_FORMAT_R32G32B32_UNORM,
      PIPE_FORMAT_R32G32B32A32_UNORM,
      PIPE_FORMAT_R32_SNORM,
      PIPE_FORMAT_R32G32_SNORM,
      PIPE_FORMAT_R32G32B32_SNORM,
      PIPE_FORMAT_R32G32B32A32_SNORM,
      PIPE_FORMAT_R32_USCALED,
      PIPE_FORMAT_R32G32_USCALED,
      PIPE_FORMAT_R32G32B32_USCALED,
      PIPE_FORMAT_R32G32B32A32_USCALED,
      PIPE_FORMAT_R32_SSCALED,
      PIPE_FORMAT_R32G32_SSCALED,
      PIPE_FORMAT_R32G32B32_SSCALED,
      PIPE_FORMAT_R32G32B32A32_SSCALED,
      */
      PIPE_FORMAT_R16_UNORM,
      PIPE_FORMAT_R16G16_UNORM,
      PIPE_FORMAT_R16G16B16_UNORM,
      PIPE_FORMAT_R16G16B16A16_UNORM,
      PIPE_FORMAT_R16_SNORM,
      PIPE_FORMAT_R16G16_SNORM,
      PIPE_FORMAT_R16G16B16_SNORM,
      PIPE_FORMAT_R16G16B16_SINT,
      PIPE_FORMAT_R16G16B16_UINT,
      PIPE_FORMAT_R16G16B16A16_SNORM,
      PIPE_FORMAT_R16_USCALED,
      PIPE_FORMAT_R16G16_USCALED,
      PIPE_FORMAT_R16G16B16_USCALED,
      PIPE_FORMAT_R16G16B16A16_USCALED,
      PIPE_FORMAT_R16_SSCALED,
      PIPE_FORMAT_R16G16_SSCALED,
      PIPE_FORMAT_R16G16B16_SSCALED,
      PIPE_FORMAT_R16G16B16A16_SSCALED,
      PIPE_FORMAT_R8_UNORM,
      PIPE_FORMAT_R8G8_UNORM,
      PIPE_FORMAT_R8G8B8_UNORM,
      PIPE_FORMAT_R8G8B8A8_UNORM,
      PIPE_FORMAT_R8_SNORM,
      PIPE_FORMAT_R8G8_SNORM,
      PIPE_FORMAT_R8G8B8_SNORM,
      PIPE_FORMAT_R8G8B8A8_SNORM,
      PIPE_FORMAT_R8_USCALED,
      PIPE_FORMAT_R8G8_USCALED,
      PIPE_FORMAT_R8G8B8_USCALED,
      PIPE_FORMAT_R8G8B8A8_USCALED,
      PIPE_FORMAT_R8_SSCALED,
      PIPE_FORMAT_R8G8_SSCALED,
      PIPE_FORMAT_R8G8B8_SSCALED,
      PIPE_FORMAT_R8G8B8A8_SSCALED,
   };
   for (unsigned i = 0; i < ARRAY_SIZE(format_list); i++) {
      if (zink_is_format_supported(&screen->base, format_list[i], PIPE_BUFFER, 0, 0, PIPE_BIND_VERTEX_BUFFER))
         continue;
      if (util_format_get_nr_components(format_list[i]) == 1)
         continue;
      enum pipe_format decomposed = zink_decompose_vertex_format(format_list[i]);
      if (zink_is_format_supported(&screen->base, decomposed, PIPE_BUFFER, 0, 0, PIPE_BIND_VERTEX_BUFFER)) {
         screen->need_decompose_attrs = true;
         mesa_logw("zink: this application would be much faster if %s supported vertex format %s", screen->info.props.deviceName, util_format_name(format_list[i]));
      }
   }
}

static void
populate_format_props(struct zink_screen *screen)
{
   for (unsigned i = 0; i < PIPE_FORMAT_COUNT; i++) {
      VkFormat format;
retry:
      format = zink_get_format(screen, i);
      if (!format)
         continue;
      if (VKSCR(GetPhysicalDeviceFormatProperties2)) {
         VkFormatProperties2 props = {0};
         props.sType = VK_STRUCTURE_TYPE_FORMAT_PROPERTIES_2;

         VkDrmFormatModifierPropertiesListEXT mod_props;
         VkDrmFormatModifierPropertiesEXT mods[128];
         if (screen->info.have_EXT_image_drm_format_modifier) {
            mod_props.sType = VK_STRUCTURE_TYPE_DRM_FORMAT_MODIFIER_PROPERTIES_LIST_EXT;
            mod_props.pNext = NULL;
            mod_props.drmFormatModifierCount = ARRAY_SIZE(mods);
            mod_props.pDrmFormatModifierProperties = mods;
            props.pNext = &mod_props;
         }
         VkFormatProperties3 props3 = {0};
         props3.sType = VK_STRUCTURE_TYPE_FORMAT_PROPERTIES_3;
         props3.pNext = props.pNext;
         props.pNext = &props3;
         VKSCR(GetPhysicalDeviceFormatProperties2)(screen->pdev, format, &props);
         screen->format_props[i].linearTilingFeatures = props3.linearTilingFeatures;
         screen->format_props[i].optimalTilingFeatures = props3.optimalTilingFeatures;
         screen->format_props[i].bufferFeatures = props3.bufferFeatures;
         if (props3.linearTilingFeatures & VK_FORMAT_FEATURE_2_LINEAR_COLOR_ATTACHMENT_BIT_NV)
            screen->format_props[i].linearTilingFeatures |= VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT;
         if (screen->info.have_EXT_image_drm_format_modifier && mod_props.drmFormatModifierCount) {
            screen->modifier_props[i].drmFormatModifierCount = mod_props.drmFormatModifierCount;
            screen->modifier_props[i].pDrmFormatModifierProperties = ralloc_array(screen, VkDrmFormatModifierPropertiesEXT, mod_props.drmFormatModifierCount);
            if (mod_props.pDrmFormatModifierProperties) {
               for (unsigned j = 0; j < mod_props.drmFormatModifierCount; j++)
                  screen->modifier_props[i].pDrmFormatModifierProperties[j] = mod_props.pDrmFormatModifierProperties[j];
            }
         }
      } else {
         VkFormatProperties props = {0};
         VKSCR(GetPhysicalDeviceFormatProperties)(screen->pdev, format, &props);
         screen->format_props[i].linearTilingFeatures = props.linearTilingFeatures;
         screen->format_props[i].optimalTilingFeatures = props.optimalTilingFeatures;
         screen->format_props[i].bufferFeatures = props.bufferFeatures;
      }
      if (i == PIPE_FORMAT_A8_UNORM && !screen->driver_workarounds.missing_a8_unorm) {
         if (!screen->format_props[i].linearTilingFeatures &&
             !screen->format_props[i].optimalTilingFeatures &&
             !screen->format_props[i].bufferFeatures) {
            screen->driver_workarounds.missing_a8_unorm = true;
            goto retry;
         }
      }
      if (zink_format_is_emulated_alpha(i)) {
         VkFormatFeatureFlags blocked = VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT | VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT;
         screen->format_props[i].linearTilingFeatures &= ~blocked;
         screen->format_props[i].optimalTilingFeatures &= ~blocked;
         screen->format_props[i].bufferFeatures = 0;
      }
   }
   check_vertex_formats(screen);
   VkImageFormatProperties image_props;
   VkResult ret = VKSCR(GetPhysicalDeviceImageFormatProperties)(screen->pdev, VK_FORMAT_D32_SFLOAT,
                                                                VK_IMAGE_TYPE_1D,
                                                                VK_IMAGE_TILING_OPTIMAL,
                                                                VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                                                                0, &image_props);
   if (ret != VK_SUCCESS && ret != VK_ERROR_FORMAT_NOT_SUPPORTED) {
      mesa_loge("ZINK: vkGetPhysicalDeviceImageFormatProperties failed (%s)", vk_Result_to_str(ret));
   }
   screen->need_2D_zs = ret != VK_SUCCESS;

   if (screen->info.feats.features.sparseResidencyImage2D)
      screen->need_2D_sparse = !screen->base.get_sparse_texture_virtual_page_size(&screen->base, PIPE_TEXTURE_1D, false, PIPE_FORMAT_R32_FLOAT, 0, 16, NULL, NULL, NULL);
}

static void
setup_renderdoc(struct zink_screen *screen)
{
#ifdef HAVE_RENDERDOC_APP_H
   const char *capture_id = debug_get_option("ZINK_RENDERDOC", NULL);
   if (!capture_id)
      return;
   void *renderdoc = dlopen("librenderdoc.so", RTLD_NOW | RTLD_NOLOAD);
   /* not loaded */
   if (!renderdoc)
      return;

   pRENDERDOC_GetAPI get_api = dlsym(renderdoc, "RENDERDOC_GetAPI");
   if (!get_api)
      return;

   /* need synchronous dispatch for renderdoc coherency */
   screen->threaded_submit = false;
   get_api(eRENDERDOC_API_Version_1_0_0, (void*)&screen->renderdoc_api);
   screen->renderdoc_api->SetActiveWindow(RENDERDOC_DEVICEPOINTER_FROM_VKINSTANCE(screen->instance), NULL);

   int count = sscanf(capture_id, "%u:%u", &screen->renderdoc_capture_start, &screen->renderdoc_capture_end);
   if (count != 2) {
      count = sscanf(capture_id, "%u", &screen->renderdoc_capture_start);
      if (!count) {
         if (!strcmp(capture_id, "all")) {
            screen->renderdoc_capture_all = true;
         } else {
            printf("`ZINK_RENDERDOC` usage: ZINK_RENDERDOC=all|frame_no[:end_frame_no]\n");
            abort();
         }
      }
      screen->renderdoc_capture_end = screen->renderdoc_capture_start;
   }
   p_atomic_set(&screen->renderdoc_frame, 1);
#endif
}

bool
zink_screen_init_semaphore(struct zink_screen *screen)
{
   VkSemaphoreCreateInfo sci = {0};
   VkSemaphoreTypeCreateInfo tci = {0};
   sci.pNext = &tci;
   sci.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
   tci.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
   tci.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;

   return VKSCR(CreateSemaphore)(screen->dev, &sci, NULL, &screen->sem) == VK_SUCCESS;
}

VkSemaphore
zink_create_exportable_semaphore(struct zink_screen *screen)
{
   VkExportSemaphoreCreateInfo eci = {
      VK_STRUCTURE_TYPE_EXPORT_SEMAPHORE_CREATE_INFO,
      NULL,
      VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_SYNC_FD_BIT
   };
   VkSemaphoreCreateInfo sci = {
      VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
      &eci,
      0
   };

   VkSemaphore sem = VK_NULL_HANDLE;
   if (util_dynarray_contains(&screen->fd_semaphores, VkSemaphore)) {
      simple_mtx_lock(&screen->semaphores_lock);
      if (util_dynarray_contains(&screen->fd_semaphores, VkSemaphore))
         sem = util_dynarray_pop(&screen->fd_semaphores, VkSemaphore);
      simple_mtx_unlock(&screen->semaphores_lock);
   }
   if (sem)
      return sem;
   VkResult ret = VKSCR(CreateSemaphore)(screen->dev, &sci, NULL, &sem);
   return ret == VK_SUCCESS ? sem : VK_NULL_HANDLE;
}

VkSemaphore
zink_screen_export_dmabuf_semaphore(struct zink_screen *screen, struct zink_resource *res)
{
   VkSemaphore sem = VK_NULL_HANDLE;
#if defined(HAVE_LIBDRM) && (DETECT_OS_LINUX || DETECT_OS_BSD)
   struct dma_buf_export_sync_file export = {
      .flags = DMA_BUF_SYNC_RW,
      .fd = -1,
   };

   int fd = -1;
   if (res->obj->is_aux) {
      fd = os_dupfd_cloexec(res->obj->handle);
   } else {
      VkMemoryGetFdInfoKHR fd_info = {0};
      fd_info.sType = VK_STRUCTURE_TYPE_MEMORY_GET_FD_INFO_KHR;
      fd_info.memory = zink_bo_get_mem(res->obj->bo);
      fd_info.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_DMA_BUF_BIT_EXT;
      VKSCR(GetMemoryFdKHR)(screen->dev, &fd_info, &fd);
   }

   if (unlikely(fd < 0)) {
      mesa_loge("MESA: Unable to get a valid memory fd");
      return VK_NULL_HANDLE;
   }

   int ret = drmIoctl(fd, DMA_BUF_IOCTL_EXPORT_SYNC_FILE, &export);
   if (ret) {
      if (errno == ENOTTY || errno == EBADF || errno == ENOSYS) {
         assert(!"how did this fail?");
         return VK_NULL_HANDLE;
      } else {
         mesa_loge("MESA: failed to import sync file '%s'", strerror(errno));
         return VK_NULL_HANDLE;
      }
   }

   sem = zink_create_exportable_semaphore(screen);

   const VkImportSemaphoreFdInfoKHR sdi = {
      .sType = VK_STRUCTURE_TYPE_IMPORT_SEMAPHORE_FD_INFO_KHR,
      .semaphore = sem,
      .flags = VK_SEMAPHORE_IMPORT_TEMPORARY_BIT,
      .handleType = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_SYNC_FD_BIT,
      .fd = export.fd,
   };
   bool success = VKSCR(ImportSemaphoreFdKHR)(screen->dev, &sdi) == VK_SUCCESS;
   close(fd);
   if (!success) {
      VKSCR(DestroySemaphore)(screen->dev, sem, NULL);
      return VK_NULL_HANDLE;
   }
#endif
   return sem;
}

bool
zink_screen_import_dmabuf_semaphore(struct zink_screen *screen, struct zink_resource *res, VkSemaphore sem)
{
#if defined(HAVE_LIBDRM) && (DETECT_OS_LINUX || DETECT_OS_BSD)
   const VkSemaphoreGetFdInfoKHR get_fd_info = {
      .sType = VK_STRUCTURE_TYPE_SEMAPHORE_GET_FD_INFO_KHR,
      .semaphore = sem,
      .handleType = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_SYNC_FD_BIT,
   };
   int sync_file_fd = -1;
   VkResult result = VKSCR(GetSemaphoreFdKHR)(screen->dev, &get_fd_info, &sync_file_fd);
   if (result != VK_SUCCESS) {
      return false;
   }

   bool ret = false;
   int fd;
   if (res->obj->is_aux) {
      fd = os_dupfd_cloexec(res->obj->handle);
   } else {
      VkMemoryGetFdInfoKHR fd_info = {0};
      fd_info.sType = VK_STRUCTURE_TYPE_MEMORY_GET_FD_INFO_KHR;
      fd_info.memory = zink_bo_get_mem(res->obj->bo);
      fd_info.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_DMA_BUF_BIT_EXT;
      if (VKSCR(GetMemoryFdKHR)(screen->dev, &fd_info, &fd) != VK_SUCCESS)
         fd = -1;
   }
   if (fd != -1) {
      struct dma_buf_import_sync_file import = {
         .flags = DMA_BUF_SYNC_RW,
         .fd = sync_file_fd,
      };
      int ret = drmIoctl(fd, DMA_BUF_IOCTL_IMPORT_SYNC_FILE, &import);
      if (ret) {
         if (errno == ENOTTY || errno == EBADF || errno == ENOSYS) {
            assert(!"how did this fail?");
         } else {
            ret = true;
         }
      }
      close(fd);
   }
   close(sync_file_fd);
   return ret;
#else
   return true;
#endif
}

bool
zink_screen_timeline_wait(struct zink_screen *screen, uint64_t batch_id, uint64_t timeout)
{
   VkSemaphoreWaitInfo wi = {0};

   if (zink_screen_check_last_finished(screen, batch_id))
      return true;

   wi.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO;
   wi.semaphoreCount = 1;
   wi.pSemaphores = &screen->sem;
   wi.pValues = &batch_id;
   bool success = false;
   if (screen->device_lost)
      return true;
   VkResult ret = VKSCR(WaitSemaphores)(screen->dev, &wi, timeout);
   success = zink_screen_handle_vkresult(screen, ret);

   if (success)
      zink_screen_update_last_finished(screen, batch_id);

   return success;
}

static uint32_t
zink_get_loader_version(struct zink_screen *screen)
{

   uint32_t loader_version = VK_API_VERSION_1_0;

   // Get the Loader version
   GET_PROC_ADDR_INSTANCE_LOCAL(screen, NULL, EnumerateInstanceVersion);
   if (vk_EnumerateInstanceVersion) {
      uint32_t loader_version_temp = VK_API_VERSION_1_0;
      VkResult result = (*vk_EnumerateInstanceVersion)(&loader_version_temp);
      if (VK_SUCCESS == result) {
         loader_version = loader_version_temp;
      } else {
         mesa_loge("ZINK: vkEnumerateInstanceVersion failed (%s)", vk_Result_to_str(result));
      }
   }

   return loader_version;
}

static void
zink_query_memory_info(struct pipe_screen *pscreen, struct pipe_memory_info *info)
{
   struct zink_screen *screen = zink_screen(pscreen);
   memset(info, 0, sizeof(struct pipe_memory_info));
   if (screen->info.have_EXT_memory_budget && VKSCR(GetPhysicalDeviceMemoryProperties2)) {
      VkPhysicalDeviceMemoryProperties2 mem = {0};
      mem.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2;

      VkPhysicalDeviceMemoryBudgetPropertiesEXT budget = {0};
      budget.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_BUDGET_PROPERTIES_EXT;
      mem.pNext = &budget;
      VKSCR(GetPhysicalDeviceMemoryProperties2)(screen->pdev, &mem);

      for (unsigned i = 0; i < mem.memoryProperties.memoryHeapCount; i++) {
         if (mem.memoryProperties.memoryHeaps[i].flags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) {
            /* VRAM */
            info->total_device_memory += mem.memoryProperties.memoryHeaps[i].size / 1024;
            info->avail_device_memory += (mem.memoryProperties.memoryHeaps[i].size - budget.heapUsage[i]) / 1024;
         } else {
            /* GART */
            info->total_staging_memory += mem.memoryProperties.memoryHeaps[i].size / 1024;
            info->avail_staging_memory += (mem.memoryProperties.memoryHeaps[i].size - budget.heapUsage[i]) / 1024;
         }
      }
      /* evictions not yet supported in vulkan */
   } else {
      for (unsigned i = 0; i < screen->info.mem_props.memoryHeapCount; i++) {
         if (screen->info.mem_props.memoryHeaps[i].flags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) {
            /* VRAM */
            info->total_device_memory += screen->info.mem_props.memoryHeaps[i].size / 1024;
            /* free real estate! */
            info->avail_device_memory += info->total_device_memory;
         } else {
            /* GART */
            info->total_staging_memory += screen->info.mem_props.memoryHeaps[i].size / 1024;
            /* free real estate! */
            info->avail_staging_memory += info->total_staging_memory;
         }
      }
   }
}

static void
zink_query_dmabuf_modifiers(struct pipe_screen *pscreen, enum pipe_format format, int max, uint64_t *modifiers, unsigned int *external_only, int *count)
{
   struct zink_screen *screen = zink_screen(pscreen);
   *count = screen->modifier_props[format].drmFormatModifierCount;
   for (int i = 0; i < MIN2(max, *count); i++) {
      if (external_only)
         external_only[i] = 0;

      modifiers[i] = screen->modifier_props[format].pDrmFormatModifierProperties[i].drmFormatModifier;
   }
}

static bool
zink_is_dmabuf_modifier_supported(struct pipe_screen *pscreen, uint64_t modifier, enum pipe_format format, bool *external_only)
{
   struct zink_screen *screen = zink_screen(pscreen);
   for (unsigned i = 0; i < screen->modifier_props[format].drmFormatModifierCount; i++)
      if (screen->modifier_props[format].pDrmFormatModifierProperties[i].drmFormatModifier == modifier)
         return true;
   return false;
}

static unsigned
zink_get_dmabuf_modifier_planes(struct pipe_screen *pscreen, uint64_t modifier, enum pipe_format format)
{
   struct zink_screen *screen = zink_screen(pscreen);
   for (unsigned i = 0; i < screen->modifier_props[format].drmFormatModifierCount; i++)
      if (screen->modifier_props[format].pDrmFormatModifierProperties[i].drmFormatModifier == modifier)
         return screen->modifier_props[format].pDrmFormatModifierProperties[i].drmFormatModifierPlaneCount;
   return util_format_get_num_planes(format);
}

static int
zink_get_sparse_texture_virtual_page_size(struct pipe_screen *pscreen,
                                          enum pipe_texture_target target,
                                          bool multi_sample,
                                          enum pipe_format pformat,
                                          unsigned offset, unsigned size,
                                          int *x, int *y, int *z)
{
   struct zink_screen *screen = zink_screen(pscreen);
   static const int page_size_2d[][3] = {
      { 256, 256, 1 }, /* 8bpp   */
      { 256, 128, 1 }, /* 16bpp  */
      { 128, 128, 1 }, /* 32bpp  */
      { 128, 64,  1 }, /* 64bpp  */
      { 64,  64,  1 }, /* 128bpp */
   };
   static const int page_size_3d[][3] = {
      { 64,  32,  32 }, /* 8bpp   */
      { 32,  32,  32 }, /* 16bpp  */
      { 32,  32,  16 }, /* 32bpp  */
      { 32,  16,  16 }, /* 64bpp  */
      { 16,  16,  16 }, /* 128bpp */
   };
   /* Only support one type of page size. */
   if (offset != 0)
      return 0;

   /* reject multisample if 2x isn't supported; assume none are */
   if (multi_sample && !screen->info.feats.features.sparseResidency2Samples)
      return 0;

   VkFormat format = zink_get_format(screen, pformat);
   bool is_zs = util_format_is_depth_or_stencil(pformat);
   VkImageType type;
   switch (target) {
   case PIPE_TEXTURE_1D:
   case PIPE_TEXTURE_1D_ARRAY:
      type = (screen->need_2D_sparse || (screen->need_2D_zs && is_zs)) ? VK_IMAGE_TYPE_2D : VK_IMAGE_TYPE_1D;
      break;

   case PIPE_TEXTURE_2D:
   case PIPE_TEXTURE_CUBE:
   case PIPE_TEXTURE_RECT:
   case PIPE_TEXTURE_2D_ARRAY:
   case PIPE_TEXTURE_CUBE_ARRAY:
      type = VK_IMAGE_TYPE_2D;
      break;

   case PIPE_TEXTURE_3D:
      type = VK_IMAGE_TYPE_3D;
      break;

   case PIPE_BUFFER:
      goto hack_it_up;

   default:
      return 0;
   }
   VkImageUsageFlags flags = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                             VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
   flags |= is_zs ? VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT : VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
   VkSparseImageFormatProperties props[4]; //planar?
   unsigned prop_count = ARRAY_SIZE(props);
   VKSCR(GetPhysicalDeviceSparseImageFormatProperties)(screen->pdev, format, type,
                                                       multi_sample ? VK_SAMPLE_COUNT_2_BIT : VK_SAMPLE_COUNT_1_BIT,
                                                       flags,
                                                       VK_IMAGE_TILING_OPTIMAL,
                                                       &prop_count, props);
   if (!prop_count) {
      if (pformat == PIPE_FORMAT_R9G9B9E5_FLOAT) {
         screen->faked_e5sparse = true;
         goto hack_it_up;
      }
      return 0;
   }

   if (size) {
      if (x)
         *x = props[0].imageGranularity.width;
      if (y)
         *y = props[0].imageGranularity.height;
      if (z)
         *z = props[0].imageGranularity.depth;
   }

   return 1;
hack_it_up:
   {
      const int (*page_sizes)[3] = target == PIPE_TEXTURE_3D ? page_size_3d : page_size_2d;
      int blk_size = util_format_get_blocksize(pformat);

      if (size) {
         unsigned index = util_logbase2(blk_size);
         if (x) *x = page_sizes[index][0];
         if (y) *y = page_sizes[index][1];
         if (z) *z = page_sizes[index][2];
      }
   }
   return 1;
}

static VkDevice
zink_create_logical_device(struct zink_screen *screen)
{
   VkDevice dev = VK_NULL_HANDLE;

   VkDeviceQueueCreateInfo qci[2] = {0};
   uint32_t queues[3] = {
      screen->gfx_queue,
      screen->sparse_queue,
   };
   float dummy = 0.0f;
   for (unsigned i = 0; i < ARRAY_SIZE(qci); i++) {
      qci[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
      qci[i].queueFamilyIndex = queues[i];
      qci[i].queueCount = 1;
      qci[i].pQueuePriorities = &dummy;
   }

   unsigned num_queues = 1;
   if (screen->sparse_queue != screen->gfx_queue)
      num_queues++;

   VkDeviceCreateInfo dci = {0};
   dci.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
   dci.queueCreateInfoCount = num_queues;
   dci.pQueueCreateInfos = qci;
   /* extensions don't have bool members in pEnabledFeatures.
    * this requires us to pass the whole VkPhysicalDeviceFeatures2 struct
    */
   if (screen->info.feats.sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2) {
      dci.pNext = &screen->info.feats;
   } else {
      dci.pEnabledFeatures = &screen->info.feats.features;
   }

   dci.ppEnabledExtensionNames = screen->info.extensions;
   dci.enabledExtensionCount = screen->info.num_extensions;

   VkResult result = VKSCR(CreateDevice)(screen->pdev, &dci, NULL, &dev);
   if (result != VK_SUCCESS)
      mesa_loge("ZINK: vkCreateDevice failed (%s)", vk_Result_to_str(result));
   
   return dev;
}

static void
check_base_requirements(struct zink_screen *screen)
{
   if (zink_debug & ZINK_DEBUG_QUIET)
      return;
   if (screen->info.driver_props.driverID == VK_DRIVER_ID_MESA_V3DV) {
      /* v3dv doesn't support straddling i/o, but zink doesn't do that so this is effectively supported:
       * don't spam errors in this case
       */
      screen->info.feats12.scalarBlockLayout = true;
      screen->info.have_EXT_scalar_block_layout = true;
   }
   if (!screen->info.feats.features.logicOp ||
       !screen->info.feats.features.fillModeNonSolid ||
       !screen->info.feats.features.shaderClipDistance ||
       !(screen->info.feats12.scalarBlockLayout ||
         screen->info.have_EXT_scalar_block_layout) ||
       !screen->info.have_KHR_maintenance1 ||
       !screen->info.have_EXT_custom_border_color ||
       !screen->info.have_EXT_line_rasterization) {
      fprintf(stderr, "WARNING: Some incorrect rendering "
              "might occur because the selected Vulkan device (%s) doesn't support "
              "base Zink requirements: ", screen->info.props.deviceName);
#define CHECK_OR_PRINT(X) \
      if (!screen->info.X) \
         fprintf(stderr, "%s ", #X)
      CHECK_OR_PRINT(feats.features.logicOp);
      CHECK_OR_PRINT(feats.features.fillModeNonSolid);
      CHECK_OR_PRINT(feats.features.shaderClipDistance);
      if (!screen->info.feats12.scalarBlockLayout && !screen->info.have_EXT_scalar_block_layout)
         fprintf(stderr, "scalarBlockLayout OR EXT_scalar_block_layout ");
      CHECK_OR_PRINT(have_KHR_maintenance1);
      CHECK_OR_PRINT(have_EXT_custom_border_color);
      CHECK_OR_PRINT(have_EXT_line_rasterization);
      fprintf(stderr, "\n");
   }
   if (screen->info.driver_props.driverID == VK_DRIVER_ID_MESA_V3DV) {
      screen->info.feats12.scalarBlockLayout = false;
      screen->info.have_EXT_scalar_block_layout = false;
   }
}

static void
zink_get_sample_pixel_grid(struct pipe_screen *pscreen, unsigned sample_count,
                           unsigned *width, unsigned *height)
{
   struct zink_screen *screen = zink_screen(pscreen);
   unsigned idx = util_logbase2_ceil(MAX2(sample_count, 1));
   assert(idx < ARRAY_SIZE(screen->maxSampleLocationGridSize));
   *width = screen->maxSampleLocationGridSize[idx].width;
   *height = screen->maxSampleLocationGridSize[idx].height;
}

static void
init_driver_workarounds(struct zink_screen *screen)
{
   /* enable implicit sync for all non-mesa drivers */
   screen->driver_workarounds.implicit_sync = true;
   switch (screen->info.driver_props.driverID) {
   case VK_DRIVER_ID_MESA_RADV:
   case VK_DRIVER_ID_INTEL_OPEN_SOURCE_MESA:
   case VK_DRIVER_ID_MESA_LLVMPIPE:
   case VK_DRIVER_ID_MESA_TURNIP:
   case VK_DRIVER_ID_MESA_V3DV:
   case VK_DRIVER_ID_MESA_PANVK:
   case VK_DRIVER_ID_MESA_VENUS:
      screen->driver_workarounds.implicit_sync = false;
      break;
   default:
      break;
   }
   /* TODO: maybe compile multiple variants for different set counts for compact mode? */
   if (screen->info.props.limits.maxBoundDescriptorSets < ZINK_DESCRIPTOR_ALL_TYPES ||
       zink_debug & (ZINK_DEBUG_COMPACT | ZINK_DEBUG_NOSHOBJ))
      screen->info.have_EXT_shader_object = false;
   /* EDS2 is only used with EDS1 */
   if (!screen->info.have_EXT_extended_dynamic_state) {
      screen->info.have_EXT_extended_dynamic_state2 = false;
      /* CWE usage needs EDS1 */
      screen->info.have_EXT_color_write_enable = false;
   }
   if (screen->info.driver_props.driverID == VK_DRIVER_ID_AMD_PROPRIETARY)
      /* this completely breaks xfb somehow */
      screen->info.have_EXT_extended_dynamic_state2 = false;
   /* EDS3 is only used with EDS2 */
   if (!screen->info.have_EXT_extended_dynamic_state2)
      screen->info.have_EXT_extended_dynamic_state3 = false;
   /* EXT_vertex_input_dynamic_state is only used with EDS2 and above */
   if (!screen->info.have_EXT_extended_dynamic_state2)
      screen->info.have_EXT_vertex_input_dynamic_state = false;
   if (screen->info.line_rast_feats.stippledRectangularLines &&
       screen->info.line_rast_feats.stippledBresenhamLines &&
       screen->info.line_rast_feats.stippledSmoothLines &&
       !screen->info.dynamic_state3_feats.extendedDynamicState3LineStippleEnable)
      screen->info.have_EXT_extended_dynamic_state3 = false;
   if (!screen->info.dynamic_state3_feats.extendedDynamicState3PolygonMode ||
       !screen->info.dynamic_state3_feats.extendedDynamicState3DepthClampEnable ||
       !screen->info.dynamic_state3_feats.extendedDynamicState3DepthClipNegativeOneToOne ||
       !screen->info.dynamic_state3_feats.extendedDynamicState3DepthClipEnable ||
       !screen->info.dynamic_state3_feats.extendedDynamicState3ProvokingVertexMode ||
       !screen->info.dynamic_state3_feats.extendedDynamicState3LineRasterizationMode)
      screen->info.have_EXT_extended_dynamic_state3 = false;
   else if (screen->info.dynamic_state3_feats.extendedDynamicState3SampleMask &&
            screen->info.dynamic_state3_feats.extendedDynamicState3AlphaToCoverageEnable &&
            (!screen->info.feats.features.alphaToOne || screen->info.dynamic_state3_feats.extendedDynamicState3AlphaToOneEnable) &&
            screen->info.dynamic_state3_feats.extendedDynamicState3ColorBlendEnable &&
            screen->info.dynamic_state3_feats.extendedDynamicState3RasterizationSamples &&
            screen->info.dynamic_state3_feats.extendedDynamicState3ColorWriteMask &&
            screen->info.dynamic_state3_feats.extendedDynamicState3ColorBlendEquation &&
            screen->info.dynamic_state3_feats.extendedDynamicState3LogicOpEnable &&
            screen->info.dynamic_state2_feats.extendedDynamicState2LogicOp)
      screen->have_full_ds3 = true;
   if (screen->info.have_EXT_graphics_pipeline_library)
      screen->info.have_EXT_graphics_pipeline_library = screen->info.have_EXT_extended_dynamic_state &&
                                                        screen->info.have_EXT_extended_dynamic_state2 &&
                                                        ((zink_debug & ZINK_DEBUG_GPL) ||
                                                         screen->info.dynamic_state2_feats.extendedDynamicState2PatchControlPoints) &&
                                                        screen->info.have_EXT_extended_dynamic_state3 &&
                                                        screen->info.have_KHR_dynamic_rendering &&
                                                        screen->info.have_EXT_non_seamless_cube_map &&
                                                        (!(zink_debug & ZINK_DEBUG_GPL) ||
                                                         screen->info.gpl_props.graphicsPipelineLibraryFastLinking ||
                                                         screen->is_cpu);
   screen->driver_workarounds.broken_l4a4 = screen->info.driver_props.driverID == VK_DRIVER_ID_NVIDIA_PROPRIETARY;
   if (screen->info.driver_props.driverID == VK_DRIVER_ID_MESA_TURNIP) {
      /* performance */
      screen->info.border_color_feats.customBorderColorWithoutFormat = VK_FALSE;
   }
   if (!screen->info.have_KHR_maintenance5)
      screen->driver_workarounds.missing_a8_unorm = true;

   if ((!screen->info.have_EXT_line_rasterization ||
        !screen->info.line_rast_feats.stippledBresenhamLines) &&
       screen->info.feats.features.geometryShader &&
       screen->info.feats.features.sampleRateShading) {
      /* we're using stippledBresenhamLines as a proxy for all of these, to
       * avoid accidentally changing behavior on VK-drivers where we don't
       * want to add emulation.
       */
      screen->driver_workarounds.no_linestipple = true;
   }

   if (screen->info.driver_props.driverID ==
       VK_DRIVER_ID_IMAGINATION_PROPRIETARY) {
      assert(screen->info.feats.features.geometryShader);
      screen->driver_workarounds.no_linesmooth = true;
   }

   /* This is a workarround for the lack of
    * gl_PointSize + glPolygonMode(..., GL_LINE), in the imagination
    * proprietary driver.
    */
   switch (screen->info.driver_props.driverID) {
   case VK_DRIVER_ID_IMAGINATION_PROPRIETARY:
      screen->driver_workarounds.no_hw_gl_point = true;
      break;
   default:
      screen->driver_workarounds.no_hw_gl_point = false;
      break;
   }

   if (screen->info.driver_props.driverID == VK_DRIVER_ID_AMD_OPEN_SOURCE || 
       screen->info.driver_props.driverID == VK_DRIVER_ID_AMD_PROPRIETARY || 
       screen->info.driver_props.driverID == VK_DRIVER_ID_NVIDIA_PROPRIETARY || 
       screen->info.driver_props.driverID == VK_DRIVER_ID_MESA_RADV)
      screen->driver_workarounds.z24_unscaled_bias = 1<<23;
   else
      screen->driver_workarounds.z24_unscaled_bias = 1<<24;
   if (screen->info.driver_props.driverID == VK_DRIVER_ID_NVIDIA_PROPRIETARY)
      screen->driver_workarounds.z16_unscaled_bias = 1<<15;
   else
      screen->driver_workarounds.z16_unscaled_bias = 1<<16;
   /* these drivers don't use VK_PIPELINE_CREATE_COLOR_ATTACHMENT_FEEDBACK_LOOP_BIT_EXT, so it can always be set */
   switch (screen->info.driver_props.driverID) {
   case VK_DRIVER_ID_MESA_RADV:
   case VK_DRIVER_ID_INTEL_OPEN_SOURCE_MESA:
   case VK_DRIVER_ID_MESA_LLVMPIPE:
   case VK_DRIVER_ID_MESA_VENUS:
   case VK_DRIVER_ID_NVIDIA_PROPRIETARY:
   case VK_DRIVER_ID_INTEL_PROPRIETARY_WINDOWS:
   case VK_DRIVER_ID_IMAGINATION_PROPRIETARY:
      screen->driver_workarounds.always_feedback_loop = screen->info.have_EXT_attachment_feedback_loop_layout;
      break;
   default:
      break;
   }
   /* these drivers don't use VK_PIPELINE_CREATE_DEPTH_STENCIL_ATTACHMENT_FEEDBACK_LOOP_BIT_EXT, so it can always be set */
   switch (screen->info.driver_props.driverID) {
   case VK_DRIVER_ID_MESA_LLVMPIPE:
   case VK_DRIVER_ID_MESA_VENUS:
   case VK_DRIVER_ID_NVIDIA_PROPRIETARY:
   case VK_DRIVER_ID_IMAGINATION_PROPRIETARY:
      screen->driver_workarounds.always_feedback_loop_zs = screen->info.have_EXT_attachment_feedback_loop_layout;
      break;
   default:
      break;
   }
   /* use same mechanics if dynamic state is supported */
   screen->driver_workarounds.always_feedback_loop |= screen->info.have_EXT_attachment_feedback_loop_dynamic_state;
   screen->driver_workarounds.always_feedback_loop_zs |= screen->info.have_EXT_attachment_feedback_loop_dynamic_state;

   /* these drivers cannot handle OOB gl_Layer values, and therefore need clamping in shader.
    * TODO: Vulkan extension that details whether vulkan driver can handle OOB layer values
    */
   switch (screen->info.driver_props.driverID) {
   case VK_DRIVER_ID_IMAGINATION_PROPRIETARY:
      screen->driver_workarounds.needs_sanitised_layer = true;
      break;
   default:
      screen->driver_workarounds.needs_sanitised_layer = false;
      break;
   }
   /* these drivers will produce undefined results when using swizzle 1 with combined z/s textures
    * TODO: use a future device property when available
    */
   switch (screen->info.driver_props.driverID) {
   case VK_DRIVER_ID_IMAGINATION_PROPRIETARY:
   case VK_DRIVER_ID_IMAGINATION_OPEN_SOURCE_MESA:
      screen->driver_workarounds.needs_zs_shader_swizzle = true;
      break;
   default:
      screen->driver_workarounds.needs_zs_shader_swizzle = false;
      break;
   }

   /* When robust contexts are advertised but robustImageAccess2 is not available */
   screen->driver_workarounds.lower_robustImageAccess2 =
      !screen->info.rb2_feats.robustImageAccess2 &&
      screen->info.feats.features.robustBufferAccess &&
      screen->info.rb_image_feats.robustImageAccess;

   /* once more testing has been done, use the #if 0 block */
   unsigned illegal = ZINK_DEBUG_RP | ZINK_DEBUG_NORP;
   if ((zink_debug & illegal) == illegal) {
      mesa_loge("Cannot specify ZINK_DEBUG=rp and ZINK_DEBUG=norp");
      abort();
   }

   /* these drivers benefit from renderpass optimization */
   switch (screen->info.driver_props.driverID) {
   case VK_DRIVER_ID_MESA_LLVMPIPE:
   case VK_DRIVER_ID_MESA_TURNIP:
   case VK_DRIVER_ID_MESA_PANVK:
   case VK_DRIVER_ID_MESA_V3DV:
   case VK_DRIVER_ID_IMAGINATION_PROPRIETARY:
   case VK_DRIVER_ID_QUALCOMM_PROPRIETARY:
   case VK_DRIVER_ID_BROADCOM_PROPRIETARY:
   case VK_DRIVER_ID_ARM_PROPRIETARY:
      screen->driver_workarounds.track_renderpasses = true; //screen->info.primgen_feats.primitivesGeneratedQueryWithRasterizerDiscard
      break;
   default:
      break;
   }
   if (zink_debug & ZINK_DEBUG_RP)
      screen->driver_workarounds.track_renderpasses = true;
   else if (zink_debug & ZINK_DEBUG_NORP)
      screen->driver_workarounds.track_renderpasses = false;

   /* these drivers can't optimize non-overlapping copy ops */
   switch (screen->info.driver_props.driverID) {
   case VK_DRIVER_ID_MESA_TURNIP:
   case VK_DRIVER_ID_QUALCOMM_PROPRIETARY:
      screen->driver_workarounds.broken_cache_semantics = true;
      break;
   default:
      break;
   }

   /* these drivers can successfully do INVALID <-> LINEAR dri3 modifier swap */
   switch (screen->info.driver_props.driverID) {
   case VK_DRIVER_ID_MESA_TURNIP:
   case VK_DRIVER_ID_MESA_VENUS:
      screen->driver_workarounds.can_do_invalid_linear_modifier = true;
      break;
   default:
      break;
   }

   /* these drivers have no difference between unoptimized and optimized shader compilation */
   switch (screen->info.driver_props.driverID) {
   case VK_DRIVER_ID_MESA_LLVMPIPE:
      screen->driver_workarounds.disable_optimized_compile = true;
      break;
   default:
      if (zink_debug & ZINK_DEBUG_NOOPT)
         screen->driver_workarounds.disable_optimized_compile = true;
      break;
   }

   switch (screen->info.driver_props.driverID) {
   case VK_DRIVER_ID_MESA_RADV:
   case VK_DRIVER_ID_AMD_OPEN_SOURCE:
   case VK_DRIVER_ID_AMD_PROPRIETARY:
      /* this has bad perf on AMD */
      screen->info.have_KHR_push_descriptor = false;
      break;
   default:
      break;
   }

   if (!screen->resizable_bar)
      screen->info.have_EXT_host_image_copy = false;
}

static void
fixup_driver_props(struct zink_screen *screen)
{
   VkPhysicalDeviceProperties2 props = {
      VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2
   };
   if (screen->info.have_EXT_host_image_copy) {
      /* fill in layouts */
      screen->info.hic_props.pNext = props.pNext;
      props.pNext = &screen->info.hic_props;
      screen->info.hic_props.pCopySrcLayouts = ralloc_array(screen, VkImageLayout, screen->info.hic_props.copySrcLayoutCount);
      screen->info.hic_props.pCopyDstLayouts = ralloc_array(screen, VkImageLayout, screen->info.hic_props.copyDstLayoutCount);
   }
   if (props.pNext)
      screen->vk.GetPhysicalDeviceProperties2(screen->pdev, &props);

   if (screen->info.have_EXT_host_image_copy) {
      for (unsigned i = 0; i < screen->info.hic_props.copyDstLayoutCount; i++) {
         if (screen->info.hic_props.pCopyDstLayouts[i] == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
            screen->can_hic_shader_read = true;
            break;
         }
      }
   }
}

static void
init_optimal_keys(struct zink_screen *screen)
{
   /* assume that anyone who knows enough to enable optimal_keys on turnip doesn't care about missing line stipple */
   if (zink_debug & ZINK_DEBUG_OPTIMAL_KEYS && screen->info.driver_props.driverID == VK_DRIVER_ID_MESA_TURNIP)
      zink_debug |= ZINK_DEBUG_QUIET;
   screen->optimal_keys = !screen->need_decompose_attrs &&
                          screen->info.have_EXT_non_seamless_cube_map &&
                          screen->info.have_EXT_provoking_vertex &&
                          !screen->driconf.inline_uniforms &&
                          !screen->driver_workarounds.no_linestipple &&
                          !screen->driver_workarounds.no_linesmooth &&
                          !screen->driver_workarounds.no_hw_gl_point &&
                          !screen->driver_workarounds.lower_robustImageAccess2 &&
                          !screen->driconf.emulate_point_smooth &&
                          !screen->driver_workarounds.needs_zs_shader_swizzle;
   if (!screen->optimal_keys && zink_debug & ZINK_DEBUG_OPTIMAL_KEYS && !(zink_debug & ZINK_DEBUG_QUIET)) {
      fprintf(stderr, "The following criteria are preventing optimal_keys enablement:\n");
      if (screen->need_decompose_attrs)
         fprintf(stderr, "missing vertex attribute formats\n");
      if (screen->driconf.inline_uniforms)
         fprintf(stderr, "uniform inlining must be disabled (set ZINK_INLINE_UNIFORMS=0 in your env)\n");
      if (screen->driconf.emulate_point_smooth)
         fprintf(stderr, "smooth point emulation is enabled\n");
      if (screen->driver_workarounds.needs_zs_shader_swizzle)
         fprintf(stderr, "Z/S shader swizzle workaround is enabled\n");
      CHECK_OR_PRINT(have_EXT_line_rasterization);
      CHECK_OR_PRINT(line_rast_feats.stippledBresenhamLines);
      CHECK_OR_PRINT(feats.features.geometryShader);
      CHECK_OR_PRINT(feats.features.sampleRateShading);
      CHECK_OR_PRINT(have_EXT_non_seamless_cube_map);
      CHECK_OR_PRINT(have_EXT_provoking_vertex);
      if (screen->driver_workarounds.no_linesmooth)
         fprintf(stderr, "driver does not support smooth lines\n");
      if (screen->driver_workarounds.no_hw_gl_point)
         fprintf(stderr, "driver does not support hardware GL_POINT\n");
      CHECK_OR_PRINT(rb2_feats.robustImageAccess2);
      CHECK_OR_PRINT(feats.features.robustBufferAccess);
      CHECK_OR_PRINT(rb_image_feats.robustImageAccess);
      printf("\n");
      mesa_logw("zink: force-enabling optimal_keys despite missing features. Good luck!");
   }
   if (zink_debug & ZINK_DEBUG_OPTIMAL_KEYS)
      screen->optimal_keys = true;
   if (!screen->optimal_keys)
      screen->info.have_EXT_graphics_pipeline_library = false;

   if (!screen->optimal_keys ||
       !screen->info.have_KHR_maintenance5 ||
      /* EXT_shader_object needs either dynamic feedback loop or per-app enablement */
       (!screen->driconf.zink_shader_object_enable && !screen->info.have_EXT_attachment_feedback_loop_dynamic_state))
      screen->info.have_EXT_shader_object = false;
   if (screen->info.have_EXT_shader_object)
      screen->have_full_ds3 = true;
   if (zink_debug & ZINK_DEBUG_DGC) {
      if (!screen->optimal_keys) {
         mesa_loge("zink: can't DGC without optimal_keys!");
         zink_debug &= ~ZINK_DEBUG_DGC;
      } else {
         screen->info.have_EXT_multi_draw = false;
         screen->info.have_EXT_shader_object = false;
         screen->info.have_EXT_graphics_pipeline_library = false;
         screen->info.have_EXT_vertex_input_dynamic_state = false;
      }
   }
}

static struct disk_cache *
zink_get_disk_shader_cache(struct pipe_screen *_screen)
{
   struct zink_screen *screen = zink_screen(_screen);

   return screen->disk_cache;
}

VkSemaphore
zink_create_semaphore(struct zink_screen *screen)
{
   VkSemaphoreCreateInfo sci = {
      VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
      NULL,
      0
   };
   VkSemaphore sem = VK_NULL_HANDLE;
   if (util_dynarray_contains(&screen->semaphores, VkSemaphore)) {
      simple_mtx_lock(&screen->semaphores_lock);
      if (util_dynarray_contains(&screen->semaphores, VkSemaphore))
         sem = util_dynarray_pop(&screen->semaphores, VkSemaphore);
      simple_mtx_unlock(&screen->semaphores_lock);
   }
   if (sem)
      return sem;
   VkResult ret = VKSCR(CreateSemaphore)(screen->dev, &sci, NULL, &sem);
   return ret == VK_SUCCESS ? sem : VK_NULL_HANDLE;
}

void
zink_screen_lock_context(struct zink_screen *screen)
{
   simple_mtx_lock(&screen->copy_context_lock);
   if (!screen->copy_context)
      screen->copy_context = zink_context(screen->base.context_create(&screen->base, NULL, ZINK_CONTEXT_COPY_ONLY));
   if (!screen->copy_context) {
      mesa_loge("zink: failed to create copy context");
      /* realistically there's nothing that can be done here */
   }
}

void
zink_screen_unlock_context(struct zink_screen *screen)
{
   simple_mtx_unlock(&screen->copy_context_lock);
}

static bool
init_layouts(struct zink_screen *screen)
{
   if (screen->info.have_EXT_descriptor_indexing) {
      VkDescriptorSetLayoutBinding bindings[4];
      const unsigned num_bindings = 4;
      VkDescriptorSetLayoutCreateInfo dcslci = {0};
      dcslci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
      dcslci.pNext = NULL;
      VkDescriptorSetLayoutBindingFlagsCreateInfo fci = {0};
      VkDescriptorBindingFlags flags[4];
      dcslci.pNext = &fci;
      if (zink_descriptor_mode == ZINK_DESCRIPTOR_MODE_DB)
         dcslci.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT;
      else
         dcslci.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
      fci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
      fci.bindingCount = num_bindings;
      fci.pBindingFlags = flags;
      for (unsigned i = 0; i < num_bindings; i++) {
         flags[i] = VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT | VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT;
         if (zink_descriptor_mode != ZINK_DESCRIPTOR_MODE_DB)
            flags[i] |= VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;
      }
      /* there is exactly 1 bindless descriptor set per context, and it has 4 bindings, 1 for each descriptor type */
      for (unsigned i = 0; i < num_bindings; i++) {
         bindings[i].binding = i;
         bindings[i].descriptorType = zink_descriptor_type_from_bindless_index(i);
         bindings[i].descriptorCount = ZINK_MAX_BINDLESS_HANDLES;
         bindings[i].stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS | VK_SHADER_STAGE_COMPUTE_BIT;
         bindings[i].pImmutableSamplers = NULL;
      }

      dcslci.bindingCount = num_bindings;
      dcslci.pBindings = bindings;
      VkResult result = VKSCR(CreateDescriptorSetLayout)(screen->dev, &dcslci, 0, &screen->bindless_layout);
      if (result != VK_SUCCESS) {
         mesa_loge("ZINK: vkCreateDescriptorSetLayout failed (%s)", vk_Result_to_str(result));
         return false;
      }
   }

   screen->gfx_push_constant_layout = zink_pipeline_layout_create(screen, NULL, 0, false, 0);
   return !!screen->gfx_push_constant_layout;
}

static int
zink_screen_get_fd(struct pipe_screen *pscreen)
{
   struct zink_screen *screen = zink_screen(pscreen);

   return screen->drm_fd;
}

static struct zink_screen *
zink_internal_create_screen(const struct pipe_screen_config *config, int64_t dev_major, int64_t dev_minor)
{
   if (getenv("ZINK_USE_LAVAPIPE")) {
      mesa_loge("ZINK_USE_LAVAPIPE is obsolete. Use LIBGL_ALWAYS_SOFTWARE\n");
      return NULL;
   }

   struct zink_screen *screen = rzalloc(NULL, struct zink_screen);
   if (!screen) {
      mesa_loge("ZINK: failed to allocate screen");
      return NULL;
   }

   screen->drm_fd = -1;

   glsl_type_singleton_init_or_ref();
   zink_debug = debug_get_option_zink_debug();
   if (zink_descriptor_mode == ZINK_DESCRIPTOR_MODE_AUTO)
      zink_descriptor_mode = debug_get_option_zink_descriptor_mode();

   screen->threaded = util_get_cpu_caps()->nr_cpus > 1 && debug_get_bool_option("GALLIUM_THREAD", util_get_cpu_caps()->nr_cpus > 1);
   if (zink_debug & ZINK_DEBUG_FLUSHSYNC)
      screen->threaded_submit = false;
   else
      screen->threaded_submit = screen->threaded;
   screen->abort_on_hang = debug_get_bool_option("ZINK_HANG_ABORT", false);


   u_trace_state_init();

   screen->loader_lib = util_dl_open(VK_LIBNAME);
   if (!screen->loader_lib) {
      mesa_loge("ZINK: failed to load "VK_LIBNAME);
      goto fail;
   }

   screen->vk_GetInstanceProcAddr = (PFN_vkGetInstanceProcAddr)util_dl_get_proc_address(screen->loader_lib, "vkGetInstanceProcAddr");
   screen->vk_GetDeviceProcAddr = (PFN_vkGetDeviceProcAddr)util_dl_get_proc_address(screen->loader_lib, "vkGetDeviceProcAddr");
   if (!screen->vk_GetInstanceProcAddr ||
       !screen->vk_GetDeviceProcAddr) {
      mesa_loge("ZINK: failed to get proc address");
      goto fail;
   }

   screen->instance_info.loader_version = zink_get_loader_version(screen);
   if (config) {
      driParseConfigFiles(config->options, config->options_info, 0, "zink",
                          NULL, NULL, NULL, 0, NULL, 0);
      screen->driconf.dual_color_blend_by_location = driQueryOptionb(config->options, "dual_color_blend_by_location");
      screen->driconf.glsl_correct_derivatives_after_discard = driQueryOptionb(config->options, "glsl_correct_derivatives_after_discard");
      //screen->driconf.inline_uniforms = driQueryOptionb(config->options, "radeonsi_inline_uniforms");
      screen->driconf.emulate_point_smooth = driQueryOptionb(config->options, "zink_emulate_point_smooth");
      screen->driconf.zink_shader_object_enable = driQueryOptionb(config->options, "zink_shader_object_enable");
   }

   if (!zink_create_instance(screen, dev_major > 0 && dev_major < 255))
      goto fail;

   if (zink_debug & ZINK_DEBUG_VALIDATION) {
      if (!screen->instance_info.have_layer_KHRONOS_validation &&
          !screen->instance_info.have_layer_LUNARG_standard_validation) {
         mesa_loge("Failed to load validation layer");
         goto fail;
      }
   }

   vk_instance_uncompacted_dispatch_table_load(&screen->vk.instance,
                                                screen->vk_GetInstanceProcAddr,
                                                screen->instance);
   vk_physical_device_uncompacted_dispatch_table_load(&screen->vk.physical_device,
                                                      screen->vk_GetInstanceProcAddr,
                                                      screen->instance);

   zink_verify_instance_extensions(screen);

   if (screen->instance_info.have_EXT_debug_utils &&
      (zink_debug & ZINK_DEBUG_VALIDATION) && !create_debug(screen))
      debug_printf("ZINK: failed to setup debug utils\n");

   choose_pdev(screen, dev_major, dev_minor);
   if (screen->pdev == VK_NULL_HANDLE) {
      mesa_loge("ZINK: failed to choose pdev");
      goto fail;
   }
   screen->is_cpu = screen->info.props.deviceType == VK_PHYSICAL_DEVICE_TYPE_CPU;

   update_queue_props(screen);

   screen->have_X8_D24_UNORM_PACK32 = zink_is_depth_format_supported(screen,
                                              VK_FORMAT_X8_D24_UNORM_PACK32);
   screen->have_D24_UNORM_S8_UINT = zink_is_depth_format_supported(screen,
                                              VK_FORMAT_D24_UNORM_S8_UINT);
   screen->have_D32_SFLOAT_S8_UINT = zink_is_depth_format_supported(screen,
                                              VK_FORMAT_D32_SFLOAT_S8_UINT);

   if (!zink_get_physical_device_info(screen)) {
      debug_printf("ZINK: failed to detect features\n");
      goto fail;
   }

   memset(&screen->heap_map, UINT8_MAX, sizeof(screen->heap_map));
   for (enum zink_heap i = 0; i < ZINK_HEAP_MAX; i++) {
      for (unsigned j = 0; j < screen->info.mem_props.memoryTypeCount; j++) {
         VkMemoryPropertyFlags domains = vk_domain_from_heap(i);
         if ((screen->info.mem_props.memoryTypes[j].propertyFlags & domains) == domains) {
            screen->heap_map[i][screen->heap_count[i]++] = j;
         }
      }
   }
   /* iterate again to check for missing heaps */
   for (enum zink_heap i = 0; i < ZINK_HEAP_MAX; i++) {
      /* not found: use compatible heap */
      if (screen->heap_map[i][0] == UINT8_MAX) {
         /* only cached mem has a failure case for now */
         assert(i == ZINK_HEAP_HOST_VISIBLE_COHERENT_CACHED || i == ZINK_HEAP_DEVICE_LOCAL_LAZY ||
                i == ZINK_HEAP_DEVICE_LOCAL_VISIBLE);
         if (i == ZINK_HEAP_HOST_VISIBLE_COHERENT_CACHED) {
            memcpy(screen->heap_map[i], screen->heap_map[ZINK_HEAP_HOST_VISIBLE_COHERENT], screen->heap_count[ZINK_HEAP_HOST_VISIBLE_COHERENT]);
            screen->heap_count[i] = screen->heap_count[ZINK_HEAP_HOST_VISIBLE_COHERENT];
         } else {
            memcpy(screen->heap_map[i], screen->heap_map[ZINK_HEAP_DEVICE_LOCAL], screen->heap_count[ZINK_HEAP_DEVICE_LOCAL]);
            screen->heap_count[i] = screen->heap_count[ZINK_HEAP_DEVICE_LOCAL];
         }
      }
   }
   {
      uint64_t biggest_vis_vram = 0;
      for (unsigned i = 0; i < screen->heap_count[ZINK_HEAP_DEVICE_LOCAL_VISIBLE]; i++)
         biggest_vis_vram = MAX2(biggest_vis_vram, screen->info.mem_props.memoryHeaps[screen->info.mem_props.memoryTypes[screen->heap_map[ZINK_HEAP_DEVICE_LOCAL_VISIBLE][i]].heapIndex].size);
      uint64_t biggest_vram = 0;
      for (unsigned i = 0; i < screen->heap_count[ZINK_HEAP_DEVICE_LOCAL]; i++)
         biggest_vram = MAX2(biggest_vram, screen->info.mem_props.memoryHeaps[screen->info.mem_props.memoryTypes[screen->heap_map[ZINK_HEAP_DEVICE_LOCAL][i]].heapIndex].size);
      /* determine if vis vram is roughly equal to total vram */
      if (biggest_vis_vram > biggest_vram * 0.9)
         screen->resizable_bar = true;
   }

   setup_renderdoc(screen);
   if (screen->threaded_submit && !util_queue_init(&screen->flush_queue, "zfq", 8, 1, UTIL_QUEUE_INIT_RESIZE_IF_FULL, screen)) {
      mesa_loge("zink: Failed to create flush queue.\n");
      goto fail;
   }

   zink_internal_setup_moltenvk(screen);
   if (!screen->info.have_KHR_timeline_semaphore && !screen->info.feats12.timelineSemaphore) {
      mesa_loge("zink: KHR_timeline_semaphore is required");
      goto fail;
   }
   if (zink_debug & ZINK_DEBUG_DGC) {
      if (!screen->info.have_NV_device_generated_commands) {
         mesa_loge("zink: can't use DGC without NV_device_generated_commands");
         goto fail;
      }
   }

   if (zink_debug & ZINK_DEBUG_MEM) {
      simple_mtx_init(&screen->debug_mem_lock, mtx_plain);
      screen->debug_mem_sizes = _mesa_hash_table_create(screen, _mesa_hash_string, _mesa_key_string_equal);
   }

   fixup_driver_props(screen);

   init_driver_workarounds(screen);

   screen->dev = zink_create_logical_device(screen);
   if (!screen->dev)
      goto fail;

   vk_device_uncompacted_dispatch_table_load(&screen->vk.device,
                                             screen->vk_GetDeviceProcAddr,
                                             screen->dev);

   init_queue(screen);

   zink_verify_device_extensions(screen);

   /* descriptor set indexing is determined by 'compact' descriptor mode:
    * by default, 6 sets are used to provide more granular updating
    * in compact mode, a maximum of 4 sets are used, with like-types combined
    */
   if ((zink_debug & ZINK_DEBUG_COMPACT) ||
       screen->info.props.limits.maxBoundDescriptorSets < ZINK_MAX_DESCRIPTOR_SETS) {
      screen->desc_set_id[ZINK_DESCRIPTOR_TYPE_UNIFORMS] = 0;
      screen->desc_set_id[ZINK_DESCRIPTOR_TYPE_UBO] = 1;
      screen->desc_set_id[ZINK_DESCRIPTOR_TYPE_SSBO] = 1;
      screen->desc_set_id[ZINK_DESCRIPTOR_TYPE_SAMPLER_VIEW] = 2;
      screen->desc_set_id[ZINK_DESCRIPTOR_TYPE_IMAGE] = 2;
      screen->desc_set_id[ZINK_DESCRIPTOR_BINDLESS] = 3;
      screen->compact_descriptors = true;
   } else {
      screen->desc_set_id[ZINK_DESCRIPTOR_TYPE_UNIFORMS] = 0;
      screen->desc_set_id[ZINK_DESCRIPTOR_TYPE_UBO] = 1;
      screen->desc_set_id[ZINK_DESCRIPTOR_TYPE_SAMPLER_VIEW] = 2;
      screen->desc_set_id[ZINK_DESCRIPTOR_TYPE_SSBO] = 3;
      screen->desc_set_id[ZINK_DESCRIPTOR_TYPE_IMAGE] = 4;
      screen->desc_set_id[ZINK_DESCRIPTOR_BINDLESS] = 5;
   }

   if (screen->info.have_EXT_calibrated_timestamps && !check_have_device_time(screen))
      goto fail;

   screen->have_triangle_fans = true;
#if defined(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME)
   if (screen->info.have_KHR_portability_subset) {
      screen->have_triangle_fans = (VK_TRUE == screen->info.portability_subset_feats.triangleFans);
   }
#endif // VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME

   check_base_requirements(screen);
   util_live_shader_cache_init(&screen->shaders, zink_create_gfx_shader_state, zink_delete_shader_state);

   screen->base.get_name = zink_get_name;
   if (screen->instance_info.have_KHR_external_memory_capabilities) {
      screen->base.get_device_uuid = zink_get_device_uuid;
      screen->base.get_driver_uuid = zink_get_driver_uuid;
   }
   if (screen->info.have_KHR_external_memory_win32) {
      screen->base.get_device_luid = zink_get_device_luid;
      screen->base.get_device_node_mask = zink_get_device_node_mask;
   }
   screen->base.set_max_shader_compiler_threads = zink_set_max_shader_compiler_threads;
   screen->base.is_parallel_shader_compilation_finished = zink_is_parallel_shader_compilation_finished;
   screen->base.get_vendor = zink_get_vendor;
   screen->base.get_device_vendor = zink_get_device_vendor;
   screen->base.get_compute_param = zink_get_compute_param;
   screen->base.get_timestamp = zink_get_timestamp;
   screen->base.query_memory_info = zink_query_memory_info;
   screen->base.get_param = zink_get_param;
   screen->base.get_paramf = zink_get_paramf;
   screen->base.get_shader_param = zink_get_shader_param;
   screen->base.get_compiler_options = zink_get_compiler_options;
   screen->base.get_sample_pixel_grid = zink_get_sample_pixel_grid;
   screen->base.is_compute_copy_faster = zink_is_compute_copy_faster;
   screen->base.is_format_supported = zink_is_format_supported;
   screen->base.driver_thread_add_job = zink_driver_thread_add_job;
   if (screen->info.have_EXT_image_drm_format_modifier && screen->info.have_EXT_external_memory_dma_buf) {
      screen->base.query_dmabuf_modifiers = zink_query_dmabuf_modifiers;
      screen->base.is_dmabuf_modifier_supported = zink_is_dmabuf_modifier_supported;
      screen->base.get_dmabuf_modifier_planes = zink_get_dmabuf_modifier_planes;
   }
#if defined(_WIN32)
   if (screen->info.have_KHR_external_memory_win32)
      screen->base.create_fence_win32 = zink_create_fence_win32;
#endif
   screen->base.context_create = zink_context_create;
   screen->base.flush_frontbuffer = zink_flush_frontbuffer;
   screen->base.destroy = zink_destroy_screen;
   screen->base.finalize_nir = zink_shader_finalize;
   screen->base.get_disk_shader_cache = zink_get_disk_shader_cache;
   screen->base.get_sparse_texture_virtual_page_size = zink_get_sparse_texture_virtual_page_size;

   if (screen->info.have_EXT_sample_locations) {
      VkMultisamplePropertiesEXT prop;
      prop.sType = VK_STRUCTURE_TYPE_MULTISAMPLE_PROPERTIES_EXT;
      prop.pNext = NULL;
      for (unsigned i = 0; i < ARRAY_SIZE(screen->maxSampleLocationGridSize); i++) {
         if (screen->info.sample_locations_props.sampleLocationSampleCounts & (1 << i)) {
            VKSCR(GetPhysicalDeviceMultisamplePropertiesEXT)(screen->pdev, 1 << i, &prop);
            screen->maxSampleLocationGridSize[i] = prop.maxSampleLocationGridSize;
         }
      }
   }

   if (!zink_screen_resource_init(&screen->base))
      goto fail;
   if (!zink_bo_init(screen)) {
      mesa_loge("ZINK: failed to initialize suballocator");
      goto fail;
   }
   zink_screen_fence_init(&screen->base);

   zink_screen_init_compiler(screen);
   if (!disk_cache_init(screen)) {
      mesa_loge("ZINK: failed to initialize disk cache");
      goto fail;
   }
   if (!util_queue_init(&screen->cache_get_thread, "zcfq", 8, 4,
                        UTIL_QUEUE_INIT_RESIZE_IF_FULL, screen))
      goto fail;
   populate_format_props(screen);

   slab_create_parent(&screen->transfer_pool, sizeof(struct zink_transfer), 16);

   screen->driconf.inline_uniforms = debug_get_bool_option("ZINK_INLINE_UNIFORMS", screen->is_cpu) && !(zink_debug & ZINK_DEBUG_DGC);

   screen->total_video_mem = get_video_mem(screen);
   screen->clamp_video_mem = screen->total_video_mem * 0.8;
   if (!os_get_total_physical_memory(&screen->total_mem)) {
      mesa_loge("ZINK: failed to get total physical memory");
      goto fail;
   }

   if (!zink_screen_init_semaphore(screen)) {
      mesa_loge("zink: failed to create timeline semaphore");
      goto fail;
   }

   bool can_db = true;
   {
      if (!screen->info.have_EXT_descriptor_buffer) {
         if (zink_descriptor_mode == ZINK_DESCRIPTOR_MODE_DB) {
            mesa_loge("Cannot use db descriptor mode without EXT_descriptor_buffer");
            goto fail;
         }
         can_db = false;
      }
      if (!screen->resizable_bar) {
         if (zink_descriptor_mode == ZINK_DESCRIPTOR_MODE_DB) {
            mesa_loge("Cannot use db descriptor mode without resizable bar");
            goto fail;
         }
         can_db = false;
      }
      if (!screen->info.have_EXT_non_seamless_cube_map) {
         if (zink_descriptor_mode == ZINK_DESCRIPTOR_MODE_DB) {
            mesa_loge("Cannot use db descriptor mode without EXT_non_seamless_cube_map");
            goto fail;
         }
         can_db = false;
      }
      if (!screen->info.rb2_feats.nullDescriptor) {
         if (zink_descriptor_mode == ZINK_DESCRIPTOR_MODE_DB) {
            mesa_loge("Cannot use db descriptor mode without robustness2.nullDescriptor");
            goto fail;
         }
         can_db = false;
      }
      if (ZINK_FBFETCH_DESCRIPTOR_SIZE < screen->info.db_props.inputAttachmentDescriptorSize) {
         if (zink_descriptor_mode == ZINK_DESCRIPTOR_MODE_DB) {
            mesa_loge("Cannot use db descriptor mode with inputAttachmentDescriptorSize(%u) > %u", (unsigned)screen->info.db_props.inputAttachmentDescriptorSize, ZINK_FBFETCH_DESCRIPTOR_SIZE);
            goto fail;
         }
         mesa_logw("zink: bug detected: inputAttachmentDescriptorSize(%u) > %u", (unsigned)screen->info.db_props.inputAttachmentDescriptorSize, ZINK_FBFETCH_DESCRIPTOR_SIZE);
         can_db = false;
      }
      if (screen->info.db_props.maxDescriptorBufferBindings < 2 || screen->info.db_props.maxSamplerDescriptorBufferBindings < 2) {
         if (zink_descriptor_mode == ZINK_DESCRIPTOR_MODE_DB) {
            /* allow for testing, but disable bindless */
            mesa_logw("Cannot use bindless and db descriptor mode with (maxDescriptorBufferBindings||maxSamplerDescriptorBufferBindings) < 2");
         } else {
            can_db = false;
         }
      }
   }
   if (zink_descriptor_mode == ZINK_DESCRIPTOR_MODE_AUTO) {
      /* descriptor buffer is not performant with virt yet */
      if (screen->info.driver_props.driverID == VK_DRIVER_ID_MESA_VENUS)
         zink_descriptor_mode = ZINK_DESCRIPTOR_MODE_LAZY;
      else
         zink_descriptor_mode = can_db ? ZINK_DESCRIPTOR_MODE_DB : ZINK_DESCRIPTOR_MODE_LAZY;
   }
   if (zink_descriptor_mode == ZINK_DESCRIPTOR_MODE_DB) {
      const uint32_t sampler_size = MAX2(screen->info.db_props.combinedImageSamplerDescriptorSize, screen->info.db_props.robustUniformTexelBufferDescriptorSize);
      const uint32_t image_size = MAX2(screen->info.db_props.storageImageDescriptorSize, screen->info.db_props.robustStorageTexelBufferDescriptorSize);
      if (screen->compact_descriptors) {
         screen->db_size[ZINK_DESCRIPTOR_TYPE_UBO] = screen->info.db_props.robustUniformBufferDescriptorSize +
                                                     screen->info.db_props.robustStorageBufferDescriptorSize;
         screen->db_size[ZINK_DESCRIPTOR_TYPE_SAMPLER_VIEW] = sampler_size + image_size;
      } else {
         screen->db_size[ZINK_DESCRIPTOR_TYPE_UBO] = screen->info.db_props.robustUniformBufferDescriptorSize;
         screen->db_size[ZINK_DESCRIPTOR_TYPE_SAMPLER_VIEW] = sampler_size;
         screen->db_size[ZINK_DESCRIPTOR_TYPE_SSBO] = screen->info.db_props.robustStorageBufferDescriptorSize;
         screen->db_size[ZINK_DESCRIPTOR_TYPE_IMAGE] = image_size;
      }
      screen->db_size[ZINK_DESCRIPTOR_TYPE_UNIFORMS] = screen->info.db_props.robustUniformBufferDescriptorSize;
      screen->info.have_KHR_push_descriptor = false;
      screen->base_descriptor_size = MAX4(screen->db_size[0], screen->db_size[1], screen->db_size[2], screen->db_size[3]);
   }

   simple_mtx_init(&screen->free_batch_states_lock, mtx_plain);
   simple_mtx_init(&screen->dt_lock, mtx_plain);

   util_idalloc_mt_init_tc(&screen->buffer_ids);

   simple_mtx_init(&screen->semaphores_lock, mtx_plain);
   util_dynarray_init(&screen->semaphores, screen);
   util_dynarray_init(&screen->fd_semaphores, screen);

   util_vertex_state_cache_init(&screen->vertex_state_cache,
                                zink_create_vertex_state, zink_vertex_state_destroy);
   screen->base.create_vertex_state = zink_cache_create_vertex_state;
   screen->base.vertex_state_destroy = zink_cache_vertex_state_destroy;

   zink_synchronization_init(screen);

   zink_init_screen_pipeline_libs(screen);

   if (!init_layouts(screen)) {
      mesa_loge("ZINK: failed to initialize layouts");
      goto fail;
   }

   if (!zink_descriptor_layouts_init(screen)) {
      mesa_loge("ZINK: failed to initialize descriptor layouts");
      goto fail;
   }

   simple_mtx_init(&screen->copy_context_lock, mtx_plain);

   init_optimal_keys(screen);

   screen->screen_id = p_atomic_inc_return(&num_screens);
   zink_tracing = screen->instance_info.have_EXT_debug_utils &&
                  (u_trace_is_enabled(U_TRACE_TYPE_PERFETTO) || u_trace_is_enabled(U_TRACE_TYPE_MARKERS));

   screen->frame_marker_emitted = zink_screen_debug_marker_begin(screen, "frame");

   return screen;

fail:
   zink_destroy_screen(&screen->base);
   return NULL;
}

struct pipe_screen *
zink_create_screen(struct sw_winsys *winsys, const struct pipe_screen_config *config)
{
   struct zink_screen *ret = zink_internal_create_screen(config, -1, -1);
   if (ret) {
      ret->drm_fd = -1;
   }

   return &ret->base;
}

static inline int
zink_render_rdev(int fd, int64_t *dev_major, int64_t *dev_minor)
{
   int ret = 0;
   *dev_major = *dev_minor = -1;
#ifdef HAVE_LIBDRM
   struct stat stx;
   drmDevicePtr dev;

   if (fd == -1)
      return 0;

   if (drmGetDevice2(fd, 0, &dev))
      return -1;

   if(!(dev->available_nodes & (1 << DRM_NODE_RENDER))) {
      ret = -1;
      goto free_device;
   }

   if(stat(dev->nodes[DRM_NODE_RENDER], &stx)) {
      ret = -1;
      goto free_device;
   }

   *dev_major = major(stx.st_rdev);
   *dev_minor = minor(stx.st_rdev);

free_device:
   drmFreeDevice(&dev);
#endif //HAVE_LIBDRM

   return ret;
}

struct pipe_screen *
zink_drm_create_screen(int fd, const struct pipe_screen_config *config)
{
   int64_t dev_major, dev_minor;
   struct zink_screen *ret;

   if (zink_render_rdev(fd, &dev_major, &dev_minor))
      return NULL;

   ret = zink_internal_create_screen(config, dev_major, dev_minor);

   if (ret)
      ret->drm_fd = os_dupfd_cloexec(fd);
   if (ret && !ret->info.have_KHR_external_memory_fd) {
      debug_printf("ZINK: KHR_external_memory_fd required!\n");
      zink_destroy_screen(&ret->base);
      return NULL;
   }

   return &ret->base;
}

void zink_stub_function_not_loaded()
{
   /* this will be used by the zink_verify_*_extensions() functions on a
    * release build
    */
   mesa_loge("ZINK: a Vulkan function was called without being loaded");
   abort();
}

bool
zink_screen_debug_marker_begin(struct zink_screen *screen, const char *fmt, ...)
{
   if (!zink_tracing)
      return false;

   char *name;
   va_list va;
   va_start(va, fmt);
   int ret = vasprintf(&name, fmt, va);
   va_end(va);

   if (ret == -1)
      return false;

   VkDebugUtilsLabelEXT info = { 0 };
   info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
   info.pLabelName = name;

   VKSCR(QueueBeginDebugUtilsLabelEXT)(screen->queue, &info);

   free(name);
   return true;
}

void
zink_screen_debug_marker_end(struct zink_screen *screen, bool emitted)
{
   if (emitted)
      VKSCR(QueueEndDebugUtilsLabelEXT)(screen->queue);
}
