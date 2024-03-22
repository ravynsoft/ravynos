/*
 * Copyright © 2016 Red Hat.
 * Copyright © 2016 Bas Nieuwenhuizen
 *
 * based in part on anv driver which is:
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

#include "radv_debug.h"
#include "radv_private.h"

#include "util/driconf.h"

#include "vk_instance.h"
#include "vk_util.h"

static const struct debug_control radv_debug_options[] = {{"nofastclears", RADV_DEBUG_NO_FAST_CLEARS},
                                                          {"nodcc", RADV_DEBUG_NO_DCC},
                                                          {"shaders", RADV_DEBUG_DUMP_SHADERS},
                                                          {"nocache", RADV_DEBUG_NO_CACHE},
                                                          {"shaderstats", RADV_DEBUG_DUMP_SHADER_STATS},
                                                          {"nohiz", RADV_DEBUG_NO_HIZ},
                                                          {"nocompute", RADV_DEBUG_NO_COMPUTE_QUEUE},
                                                          {"allbos", RADV_DEBUG_ALL_BOS},
                                                          {"noibs", RADV_DEBUG_NO_IBS},
                                                          {"spirv", RADV_DEBUG_DUMP_SPIRV},
                                                          {"zerovram", RADV_DEBUG_ZERO_VRAM},
                                                          {"syncshaders", RADV_DEBUG_SYNC_SHADERS},
                                                          {"preoptir", RADV_DEBUG_PREOPTIR},
                                                          {"nodynamicbounds", RADV_DEBUG_NO_DYNAMIC_BOUNDS},
                                                          {"info", RADV_DEBUG_INFO},
                                                          {"startup", RADV_DEBUG_STARTUP},
                                                          {"checkir", RADV_DEBUG_CHECKIR},
                                                          {"nobinning", RADV_DEBUG_NOBINNING},
                                                          {"nongg", RADV_DEBUG_NO_NGG},
                                                          {"metashaders", RADV_DEBUG_DUMP_META_SHADERS},
                                                          {"nomemorycache", RADV_DEBUG_NO_MEMORY_CACHE},
                                                          {"discardtodemote", RADV_DEBUG_DISCARD_TO_DEMOTE},
                                                          {"llvm", RADV_DEBUG_LLVM},
                                                          {"forcecompress", RADV_DEBUG_FORCE_COMPRESS},
                                                          {"hang", RADV_DEBUG_HANG},
                                                          {"img", RADV_DEBUG_IMG},
                                                          {"noumr", RADV_DEBUG_NO_UMR},
                                                          {"invariantgeom", RADV_DEBUG_INVARIANT_GEOM},
                                                          {"splitfma", RADV_DEBUG_SPLIT_FMA},
                                                          {"nodisplaydcc", RADV_DEBUG_NO_DISPLAY_DCC},
                                                          {"notccompatcmask", RADV_DEBUG_NO_TC_COMPAT_CMASK},
                                                          {"novrsflatshading", RADV_DEBUG_NO_VRS_FLAT_SHADING},
                                                          {"noatocdithering", RADV_DEBUG_NO_ATOC_DITHERING},
                                                          {"nonggc", RADV_DEBUG_NO_NGGC},
                                                          {"prologs", RADV_DEBUG_DUMP_PROLOGS},
                                                          {"nodma", RADV_DEBUG_NO_DMA_BLIT},
                                                          {"epilogs", RADV_DEBUG_DUMP_EPILOGS},
                                                          {"nofmask", RADV_DEBUG_NO_FMASK},
                                                          {"shadowregs", RADV_DEBUG_SHADOW_REGS},
                                                          {"extra_md", RADV_DEBUG_EXTRA_MD},
                                                          {"nogpl", RADV_DEBUG_NO_GPL},
                                                          {"videoarraypath", RADV_DEBUG_VIDEO_ARRAY_PATH},
                                                          {"nort", RADV_DEBUG_NO_RT},
                                                          {"nomeshshader", RADV_DEBUG_NO_MESH_SHADER},
                                                          {NULL, 0}};

const char *
radv_get_debug_option_name(int id)
{
   assert(id < ARRAY_SIZE(radv_debug_options) - 1);
   return radv_debug_options[id].string;
}

static const struct debug_control radv_perftest_options[] = {{"localbos", RADV_PERFTEST_LOCAL_BOS},
                                                             {"dccmsaa", RADV_PERFTEST_DCC_MSAA},
                                                             {"bolist", RADV_PERFTEST_BO_LIST},
                                                             {"cswave32", RADV_PERFTEST_CS_WAVE_32},
                                                             {"pswave32", RADV_PERFTEST_PS_WAVE_32},
                                                             {"gewave32", RADV_PERFTEST_GE_WAVE_32},
                                                             {"nosam", RADV_PERFTEST_NO_SAM},
                                                             {"sam", RADV_PERFTEST_SAM},
                                                             {"nggc", RADV_PERFTEST_NGGC},
                                                             {"emulate_rt", RADV_PERFTEST_EMULATE_RT},
                                                             {"rtwave64", RADV_PERFTEST_RT_WAVE_64},
                                                             {"video_decode", RADV_PERFTEST_VIDEO_DECODE},
                                                             {"dmashaders", RADV_PERFTEST_DMA_SHADERS},
                                                             {"gsfastlaunch2", RADV_PERFTEST_GS_FAST_LAUNCH_2},
                                                             {"transfer_queue", RADV_PERFTEST_TRANSFER_QUEUE},
                                                             {NULL, 0}};

const char *
radv_get_perftest_option_name(int id)
{
   assert(id < ARRAY_SIZE(radv_perftest_options) - 1);
   return radv_perftest_options[id].string;
}

static const struct debug_control trace_options[] = {
   {"rgp", RADV_TRACE_MODE_RGP},
   {"rra", RADV_TRACE_MODE_RRA},
   {NULL, 0},
};

// clang-format off
static const driOptionDescription radv_dri_options[] = {
   DRI_CONF_SECTION_PERFORMANCE
      DRI_CONF_ADAPTIVE_SYNC(true)
      DRI_CONF_VK_X11_OVERRIDE_MIN_IMAGE_COUNT(0)
      DRI_CONF_VK_X11_STRICT_IMAGE_COUNT(false)
      DRI_CONF_VK_X11_ENSURE_MIN_IMAGE_COUNT(false)
      DRI_CONF_VK_KHR_PRESENT_WAIT(false)
      DRI_CONF_VK_XWAYLAND_WAIT_READY(true)
      DRI_CONF_RADV_REPORT_LLVM9_VERSION_STRING(false)
      DRI_CONF_RADV_ENABLE_MRT_OUTPUT_NAN_FIXUP(false)
      DRI_CONF_RADV_DISABLE_SHRINK_IMAGE_STORE(false)
      DRI_CONF_RADV_NO_DYNAMIC_BOUNDS(false)
      DRI_CONF_RADV_OVERRIDE_UNIFORM_OFFSET_ALIGNMENT(0)
      DRI_CONF_RADV_CLEAR_LDS(false)
   DRI_CONF_SECTION_END

   DRI_CONF_SECTION_DEBUG
      DRI_CONF_OVERRIDE_VRAM_SIZE()
      DRI_CONF_VK_WSI_FORCE_BGRA8_UNORM_FIRST(false)
      DRI_CONF_VK_WSI_FORCE_SWAPCHAIN_TO_CURRENT_EXTENT(false)
      DRI_CONF_VK_REQUIRE_ETC2(false)
      DRI_CONF_VK_REQUIRE_ASTC(false)
      DRI_CONF_RADV_ZERO_VRAM(false)
      DRI_CONF_RADV_LOWER_DISCARD_TO_DEMOTE(false)
      DRI_CONF_RADV_INVARIANT_GEOM(false)
      DRI_CONF_RADV_SPLIT_FMA(false)
      DRI_CONF_RADV_DISABLE_TC_COMPAT_HTILE_GENERAL(false)
      DRI_CONF_RADV_DISABLE_DCC(false)
      DRI_CONF_RADV_DISABLE_ANISO_SINGLE_LEVEL(false)
      DRI_CONF_RADV_DISABLE_TRUNC_COORD(false)
      DRI_CONF_RADV_DISABLE_SINKING_LOAD_INPUT_FS(false)
      DRI_CONF_RADV_DGC(false)
      DRI_CONF_RADV_FLUSH_BEFORE_QUERY_COPY(false)
      DRI_CONF_RADV_ENABLE_UNIFIED_HEAP_ON_APU(false)
      DRI_CONF_RADV_TEX_NON_UNIFORM(false)
      DRI_CONF_RADV_FLUSH_BEFORE_TIMESTAMP_WRITE(false)
      DRI_CONF_RADV_RT_WAVE64(false)
      DRI_CONF_RADV_LEGACY_SPARSE_BINDING(false)
      DRI_CONF_DUAL_COLOR_BLEND_BY_LOCATION(false)
      DRI_CONF_RADV_OVERRIDE_GRAPHICS_SHADER_VERSION(0)
      DRI_CONF_RADV_OVERRIDE_COMPUTE_SHADER_VERSION(0)
      DRI_CONF_RADV_OVERRIDE_RAY_TRACING_SHADER_VERSION(0)
      DRI_CONF_RADV_SSBO_NON_UNIFORM(false)
      DRI_CONF_RADV_FORCE_ACTIVE_ACCEL_STRUCT_LEAVES(false)
      DRI_CONF_RADV_APP_LAYER()
   DRI_CONF_SECTION_END
};
// clang-format on

static void
radv_init_dri_options(struct radv_instance *instance)
{
   driParseOptionInfo(&instance->drirc.available_options, radv_dri_options, ARRAY_SIZE(radv_dri_options));
   driParseConfigFiles(&instance->drirc.options, &instance->drirc.available_options, 0, "radv", NULL, NULL,
                       instance->vk.app_info.app_name, instance->vk.app_info.app_version,
                       instance->vk.app_info.engine_name, instance->vk.app_info.engine_version);

   instance->drirc.enable_mrt_output_nan_fixup =
      driQueryOptionb(&instance->drirc.options, "radv_enable_mrt_output_nan_fixup");

   instance->drirc.disable_shrink_image_store =
      driQueryOptionb(&instance->drirc.options, "radv_disable_shrink_image_store");

   instance->drirc.disable_tc_compat_htile_in_general =
      driQueryOptionb(&instance->drirc.options, "radv_disable_tc_compat_htile_general");

   if (driQueryOptionb(&instance->drirc.options, "radv_no_dynamic_bounds"))
      instance->debug_flags |= RADV_DEBUG_NO_DYNAMIC_BOUNDS;

   if (driQueryOptionb(&instance->drirc.options, "radv_lower_discard_to_demote"))
      instance->debug_flags |= RADV_DEBUG_DISCARD_TO_DEMOTE;

   if (driQueryOptionb(&instance->drirc.options, "radv_invariant_geom"))
      instance->debug_flags |= RADV_DEBUG_INVARIANT_GEOM;

   if (driQueryOptionb(&instance->drirc.options, "radv_split_fma"))
      instance->debug_flags |= RADV_DEBUG_SPLIT_FMA;

   if (driQueryOptionb(&instance->drirc.options, "radv_disable_dcc"))
      instance->debug_flags |= RADV_DEBUG_NO_DCC;

   instance->drirc.clear_lds = driQueryOptionb(&instance->drirc.options, "radv_clear_lds");

   instance->drirc.zero_vram = driQueryOptionb(&instance->drirc.options, "radv_zero_vram");

   instance->drirc.disable_aniso_single_level =
      driQueryOptionb(&instance->drirc.options, "radv_disable_aniso_single_level");

   instance->drirc.disable_trunc_coord = driQueryOptionb(&instance->drirc.options, "radv_disable_trunc_coord");

   instance->drirc.disable_sinking_load_input_fs =
      driQueryOptionb(&instance->drirc.options, "radv_disable_sinking_load_input_fs");

   instance->drirc.flush_before_query_copy = driQueryOptionb(&instance->drirc.options, "radv_flush_before_query_copy");

   instance->drirc.enable_unified_heap_on_apu =
      driQueryOptionb(&instance->drirc.options, "radv_enable_unified_heap_on_apu");

   instance->drirc.tex_non_uniform = driQueryOptionb(&instance->drirc.options, "radv_tex_non_uniform");

   instance->drirc.ssbo_non_uniform = driQueryOptionb(&instance->drirc.options, "radv_ssbo_non_uniform");

   instance->drirc.app_layer = driQueryOptionstr(&instance->drirc.options, "radv_app_layer");

   instance->drirc.flush_before_timestamp_write =
      driQueryOptionb(&instance->drirc.options, "radv_flush_before_timestamp_write");

   instance->drirc.force_rt_wave64 = driQueryOptionb(&instance->drirc.options, "radv_rt_wave64");

   instance->drirc.dual_color_blend_by_location =
      driQueryOptionb(&instance->drirc.options, "dual_color_blend_by_location");

   instance->drirc.legacy_sparse_binding = driQueryOptionb(&instance->drirc.options, "radv_legacy_sparse_binding");

   instance->drirc.override_graphics_shader_version =
      driQueryOptioni(&instance->drirc.options, "radv_override_graphics_shader_version");
   instance->drirc.override_compute_shader_version =
      driQueryOptioni(&instance->drirc.options, "radv_override_compute_shader_version");
   instance->drirc.override_ray_tracing_shader_version =
      driQueryOptioni(&instance->drirc.options, "radv_override_ray_tracing_shader_version");

   instance->drirc.enable_dgc = driQueryOptionb(&instance->drirc.options, "radv_dgc");

   instance->drirc.override_vram_size = driQueryOptioni(&instance->drirc.options, "override_vram_size");

   instance->drirc.enable_khr_present_wait = driQueryOptionb(&instance->drirc.options, "vk_khr_present_wait");

   instance->drirc.override_uniform_offset_alignment =
      driQueryOptioni(&instance->drirc.options, "radv_override_uniform_offset_alignment");

   instance->drirc.report_llvm9_version_string =
      driQueryOptionb(&instance->drirc.options, "radv_report_llvm9_version_string");

   instance->drirc.vk_require_etc2 = driQueryOptionb(&instance->drirc.options, "vk_require_etc2");
   instance->drirc.vk_require_astc = driQueryOptionb(&instance->drirc.options, "vk_require_astc");

   instance->drirc.force_active_accel_struct_leaves =
      driQueryOptionb(&instance->drirc.options, "radv_force_active_accel_struct_leaves");
}

static const struct vk_instance_extension_table radv_instance_extensions_supported = {
   .KHR_device_group_creation = true,
   .KHR_external_fence_capabilities = true,
   .KHR_external_memory_capabilities = true,
   .KHR_external_semaphore_capabilities = true,
   .KHR_get_physical_device_properties2 = true,
   .EXT_debug_report = true,
   .EXT_debug_utils = true,

#ifdef RADV_USE_WSI_PLATFORM
   .KHR_get_surface_capabilities2 = true,
   .KHR_surface = true,
   .KHR_surface_protected_capabilities = true,
   .EXT_surface_maintenance1 = true,
   .EXT_swapchain_colorspace = true,
#endif
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
   .KHR_wayland_surface = true,
#endif
#ifdef VK_USE_PLATFORM_XCB_KHR
   .KHR_xcb_surface = true,
#endif
#ifdef VK_USE_PLATFORM_XLIB_KHR
   .KHR_xlib_surface = true,
#endif
#ifdef VK_USE_PLATFORM_XLIB_XRANDR_EXT
   .EXT_acquire_xlib_display = true,
#endif
#ifdef VK_USE_PLATFORM_DISPLAY_KHR
   .KHR_display = true,
   .KHR_get_display_properties2 = true,
   .EXT_direct_mode_display = true,
   .EXT_display_surface_counter = true,
   .EXT_acquire_drm_display = true,
#endif
};

static void
radv_handle_legacy_sqtt_trigger(struct vk_instance *instance)
{
   char *trigger_file = secure_getenv("RADV_THREAD_TRACE_TRIGGER");
   if (trigger_file) {
      instance->trace_trigger_file = trigger_file;
      instance->trace_mode |= RADV_TRACE_MODE_RGP;
      fprintf(stderr, "WARNING: RADV_THREAD_TRACE_TRIGGER is deprecated, please use MESA_VK_TRACE_TRIGGER instead.\n");
   }
}

VKAPI_ATTR VkResult VKAPI_CALL
radv_CreateInstance(const VkInstanceCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator,
                    VkInstance *pInstance)
{
   struct radv_instance *instance;
   VkResult result;

   if (!pAllocator)
      pAllocator = vk_default_allocator();

   instance = vk_zalloc(pAllocator, sizeof(*instance), 8, VK_SYSTEM_ALLOCATION_SCOPE_INSTANCE);
   if (!instance)
      return vk_error(NULL, VK_ERROR_OUT_OF_HOST_MEMORY);

   struct vk_instance_dispatch_table dispatch_table;
   vk_instance_dispatch_table_from_entrypoints(&dispatch_table, &radv_instance_entrypoints, true);
   vk_instance_dispatch_table_from_entrypoints(&dispatch_table, &wsi_instance_entrypoints, false);

   result =
      vk_instance_init(&instance->vk, &radv_instance_extensions_supported, &dispatch_table, pCreateInfo, pAllocator);
   if (result != VK_SUCCESS) {
      vk_free(pAllocator, instance);
      return vk_error(NULL, result);
   }

   vk_instance_add_driver_trace_modes(&instance->vk, trace_options);
   radv_handle_legacy_sqtt_trigger(&instance->vk);

   instance->debug_flags = parse_debug_string(getenv("RADV_DEBUG"), radv_debug_options);
   instance->perftest_flags = parse_debug_string(getenv("RADV_PERFTEST"), radv_perftest_options);

   /* When RADV_FORCE_FAMILY is set, the driver creates a null
    * device that allows to test the compiler without having an
    * AMDGPU instance.
    */
   if (getenv("RADV_FORCE_FAMILY"))
      instance->vk.physical_devices.enumerate = create_null_physical_device;
   else
      instance->vk.physical_devices.try_create_for_drm = create_drm_physical_device;

   instance->vk.physical_devices.destroy = radv_physical_device_destroy;

   if (instance->debug_flags & RADV_DEBUG_STARTUP)
      fprintf(stderr, "radv: info: Created an instance.\n");

   VG(VALGRIND_CREATE_MEMPOOL(instance, 0, false));

   radv_init_dri_options(instance);

   *pInstance = radv_instance_to_handle(instance);

   return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL
radv_DestroyInstance(VkInstance _instance, const VkAllocationCallbacks *pAllocator)
{
   RADV_FROM_HANDLE(radv_instance, instance, _instance);

   if (!instance)
      return;

   VG(VALGRIND_DESTROY_MEMPOOL(instance));

   driDestroyOptionCache(&instance->drirc.options);
   driDestroyOptionInfo(&instance->drirc.available_options);

   vk_instance_finish(&instance->vk);
   vk_free(&instance->vk.alloc, instance);
}

VKAPI_ATTR VkResult VKAPI_CALL
radv_EnumerateInstanceExtensionProperties(const char *pLayerName, uint32_t *pPropertyCount,
                                          VkExtensionProperties *pProperties)
{
   if (pLayerName)
      return vk_error(NULL, VK_ERROR_LAYER_NOT_PRESENT);

   return vk_enumerate_instance_extension_properties(&radv_instance_extensions_supported, pPropertyCount, pProperties);
}

VKAPI_ATTR VkResult VKAPI_CALL
radv_EnumerateInstanceVersion(uint32_t *pApiVersion)
{
   *pApiVersion = RADV_API_VERSION;
   return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL
radv_EnumerateInstanceLayerProperties(uint32_t *pPropertyCount, VkLayerProperties *pProperties)
{
   if (pProperties == NULL) {
      *pPropertyCount = 0;
      return VK_SUCCESS;
   }

   /* None supported at this time */
   return vk_error(NULL, VK_ERROR_LAYER_NOT_PRESENT);
}

VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL
radv_GetInstanceProcAddr(VkInstance _instance, const char *pName)
{
   RADV_FROM_HANDLE(vk_instance, instance, _instance);
   return vk_instance_get_proc_addr(instance, &radv_instance_entrypoints, pName);
}

/* Windows will use a dll definition file to avoid build errors. */
#ifdef _WIN32
#undef PUBLIC
#define PUBLIC
#endif

/* The loader wants us to expose a second GetInstanceProcAddr function
 * to work around certain LD_PRELOAD issues seen in apps.
 */
PUBLIC
VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL
vk_icdGetInstanceProcAddr(VkInstance instance, const char *pName)
{
   return radv_GetInstanceProcAddr(instance, pName);
}
