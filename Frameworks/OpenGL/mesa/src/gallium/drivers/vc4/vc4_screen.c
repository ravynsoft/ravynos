/*
 * Copyright © 2014 Broadcom
 * Copyright (C) 2012 Rob Clark <robclark@freedesktop.org>
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

#include "util/os_misc.h"
#include "pipe/p_defines.h"
#include "pipe/p_screen.h"
#include "pipe/p_state.h"

#include "util/u_debug.h"
#include "util/u_memory.h"
#include "util/format/u_format.h"
#include "util/u_hash_table.h"
#include "util/u_screen.h"
#include "util/u_transfer_helper.h"
#include "util/ralloc.h"

#include <xf86drm.h>
#include "drm-uapi/drm_fourcc.h"
#include "drm-uapi/vc4_drm.h"
#include "vc4_screen.h"
#include "vc4_context.h"
#include "vc4_resource.h"

static const struct debug_named_value vc4_debug_options[] = {
        { "cl",       VC4_DEBUG_CL,
          "Dump command list during creation" },
        { "surf",       VC4_DEBUG_SURFACE,
          "Dump surface layouts" },
        { "qpu",      VC4_DEBUG_QPU,
          "Dump generated QPU instructions" },
        { "qir",      VC4_DEBUG_QIR,
          "Dump QPU IR during program compile" },
        { "nir",      VC4_DEBUG_NIR,
          "Dump NIR during program compile" },
        { "tgsi",     VC4_DEBUG_TGSI,
          "Dump TGSI during program compile" },
        { "shaderdb", VC4_DEBUG_SHADERDB,
          "Dump program compile information for shader-db analysis" },
        { "perf",     VC4_DEBUG_PERF,
          "Print during performance-related events" },
        { "norast",   VC4_DEBUG_NORAST,
          "Skip actual hardware execution of commands" },
        { "always_flush", VC4_DEBUG_ALWAYS_FLUSH,
          "Flush after each draw call" },
        { "always_sync", VC4_DEBUG_ALWAYS_SYNC,
          "Wait for finish after each flush" },
#ifdef USE_VC4_SIMULATOR
        { "dump", VC4_DEBUG_DUMP,
          "Write a GPU command stream trace file" },
#endif
        DEBUG_NAMED_VALUE_END
};

DEBUG_GET_ONCE_FLAGS_OPTION(vc4_debug, "VC4_DEBUG", vc4_debug_options, 0)
uint32_t vc4_mesa_debug;

static const char *
vc4_screen_get_name(struct pipe_screen *pscreen)
{
        struct vc4_screen *screen = vc4_screen(pscreen);

        if (!screen->name) {
                screen->name = ralloc_asprintf(screen,
                                               "VC4 V3D %d.%d",
                                               screen->v3d_ver / 10,
                                               screen->v3d_ver % 10);
        }

        return screen->name;
}

static const char *
vc4_screen_get_vendor(struct pipe_screen *pscreen)
{
        return "Broadcom";
}

static void
vc4_screen_destroy(struct pipe_screen *pscreen)
{
        struct vc4_screen *screen = vc4_screen(pscreen);

        _mesa_hash_table_destroy(screen->bo_handles, NULL);
        vc4_bufmgr_destroy(pscreen);
        slab_destroy_parent(&screen->transfer_pool);
        if (screen->ro)
                screen->ro->destroy(screen->ro);

#ifdef USE_VC4_SIMULATOR
        vc4_simulator_destroy(screen);
#endif

        u_transfer_helper_destroy(pscreen->transfer_helper);

        close(screen->fd);
        ralloc_free(pscreen);
}

static bool
vc4_has_feature(struct vc4_screen *screen, uint32_t feature)
{
        struct drm_vc4_get_param p = {
                .param = feature,
        };
        int ret = vc4_ioctl(screen->fd, DRM_IOCTL_VC4_GET_PARAM, &p);

        if (ret != 0)
                return false;

        return p.value;
}

static int
vc4_screen_get_param(struct pipe_screen *pscreen, enum pipe_cap param)
{
        struct vc4_screen *screen = vc4_screen(pscreen);

        switch (param) {
                /* Supported features (boolean caps). */
        case PIPE_CAP_VERTEX_COLOR_UNCLAMPED:
        case PIPE_CAP_FRAGMENT_COLOR_CLAMPED:
        case PIPE_CAP_NPOT_TEXTURES:
        case PIPE_CAP_BLEND_EQUATION_SEPARATE:
        case PIPE_CAP_TEXTURE_MULTISAMPLE:
        case PIPE_CAP_TEXTURE_SWIZZLE:
        case PIPE_CAP_TEXTURE_BARRIER:
        case PIPE_CAP_TGSI_TEXCOORD:
                return 1;

        case PIPE_CAP_NATIVE_FENCE_FD:
                return screen->has_syncobj;

        case PIPE_CAP_TILE_RASTER_ORDER:
                return vc4_has_feature(screen,
                                       DRM_VC4_PARAM_SUPPORTS_FIXED_RCL_ORDER);

        case PIPE_CAP_FS_COORD_ORIGIN_UPPER_LEFT:
        case PIPE_CAP_FS_COORD_PIXEL_CENTER_HALF_INTEGER:
        case PIPE_CAP_FS_FACE_IS_INTEGER_SYSVAL:
                return 1;

        case PIPE_CAP_MIXED_FRAMEBUFFER_SIZES:
        case PIPE_CAP_MIXED_COLOR_DEPTH_BITS:
                return 1;

                /* Texturing. */
        case PIPE_CAP_MAX_TEXTURE_2D_SIZE:
                return 2048;
        case PIPE_CAP_MAX_TEXTURE_CUBE_LEVELS:
                return VC4_MAX_MIP_LEVELS;
        case PIPE_CAP_MAX_TEXTURE_3D_LEVELS:
                return 0;

        case PIPE_CAP_MAX_VARYINGS:
                return 8;

        case PIPE_CAP_VENDOR_ID:
                return 0x14E4;
        case PIPE_CAP_ACCELERATED:
                return 1;
        case PIPE_CAP_VIDEO_MEMORY: {
                uint64_t system_memory;

                if (!os_get_total_physical_memory(&system_memory))
                        return 0;

                return (int)(system_memory >> 20);
        }
        case PIPE_CAP_UMA:
                return 1;

        case PIPE_CAP_ALPHA_TEST:
        case PIPE_CAP_VERTEX_COLOR_CLAMPED:
        case PIPE_CAP_TWO_SIDED_COLOR:
        case PIPE_CAP_TEXRECT:
        case PIPE_CAP_IMAGE_STORE_FORMATTED:
                return 0;

        case PIPE_CAP_SUPPORTED_PRIM_MODES:
                return screen->prim_types;

        default:
                return u_pipe_screen_get_param_defaults(pscreen, param);
        }
}

static float
vc4_screen_get_paramf(struct pipe_screen *pscreen, enum pipe_capf param)
{
        switch (param) {
        case PIPE_CAPF_MIN_LINE_WIDTH:
        case PIPE_CAPF_MIN_LINE_WIDTH_AA:
        case PIPE_CAPF_MIN_POINT_SIZE:
        case PIPE_CAPF_MIN_POINT_SIZE_AA:
           return 1;

        case PIPE_CAPF_POINT_SIZE_GRANULARITY:
        case PIPE_CAPF_LINE_WIDTH_GRANULARITY:
           return 0.1;

        case PIPE_CAPF_MAX_LINE_WIDTH:
        case PIPE_CAPF_MAX_LINE_WIDTH_AA:
                return 32;

        case PIPE_CAPF_MAX_POINT_SIZE:
        case PIPE_CAPF_MAX_POINT_SIZE_AA:
                return 512.0f;

        case PIPE_CAPF_MAX_TEXTURE_ANISOTROPY:
                return 0.0f;
        case PIPE_CAPF_MAX_TEXTURE_LOD_BIAS:
                return 0.0f;

        case PIPE_CAPF_MIN_CONSERVATIVE_RASTER_DILATE:
        case PIPE_CAPF_MAX_CONSERVATIVE_RASTER_DILATE:
        case PIPE_CAPF_CONSERVATIVE_RASTER_DILATE_GRANULARITY:
                return 0.0f;
        default:
                fprintf(stderr, "unknown paramf %d\n", param);
                return 0;
        }
}

static int
vc4_screen_get_shader_param(struct pipe_screen *pscreen,
                            enum pipe_shader_type shader,
                            enum pipe_shader_cap param)
{
        if (shader != PIPE_SHADER_VERTEX &&
            shader != PIPE_SHADER_FRAGMENT) {
                return 0;
        }

        /* this is probably not totally correct.. but it's a start: */
        switch (param) {
        case PIPE_SHADER_CAP_MAX_INSTRUCTIONS:
        case PIPE_SHADER_CAP_MAX_ALU_INSTRUCTIONS:
        case PIPE_SHADER_CAP_MAX_TEX_INSTRUCTIONS:
        case PIPE_SHADER_CAP_MAX_TEX_INDIRECTIONS:
                return 16384;

        case PIPE_SHADER_CAP_MAX_CONTROL_FLOW_DEPTH:
                return vc4_screen(pscreen)->has_control_flow;

        case PIPE_SHADER_CAP_MAX_INPUTS:
                return 8;
        case PIPE_SHADER_CAP_MAX_OUTPUTS:
                return shader == PIPE_SHADER_FRAGMENT ? 1 : 8;
        case PIPE_SHADER_CAP_MAX_TEMPS:
                return 256; /* GL_MAX_PROGRAM_TEMPORARIES_ARB */
        case PIPE_SHADER_CAP_MAX_CONST_BUFFER0_SIZE:
                return 16 * 1024 * sizeof(float);
        case PIPE_SHADER_CAP_MAX_CONST_BUFFERS:
                return 1;
        case PIPE_SHADER_CAP_CONT_SUPPORTED:
                return 0;
        case PIPE_SHADER_CAP_INDIRECT_INPUT_ADDR:
        case PIPE_SHADER_CAP_INDIRECT_OUTPUT_ADDR:
        case PIPE_SHADER_CAP_INDIRECT_TEMP_ADDR:
                return 0;
        case PIPE_SHADER_CAP_INDIRECT_CONST_ADDR:
                return 1;
        case PIPE_SHADER_CAP_SUBROUTINES:
                return 0;
        case PIPE_SHADER_CAP_TGSI_SQRT_SUPPORTED:
                return 0;
        case PIPE_SHADER_CAP_INTEGERS:
                return 1;
        case PIPE_SHADER_CAP_INT64_ATOMICS:
        case PIPE_SHADER_CAP_FP16:
        case PIPE_SHADER_CAP_FP16_DERIVATIVES:
        case PIPE_SHADER_CAP_FP16_CONST_BUFFERS:
        case PIPE_SHADER_CAP_INT16:
        case PIPE_SHADER_CAP_GLSL_16BIT_CONSTS:
        case PIPE_SHADER_CAP_TGSI_ANY_INOUT_DECL_RANGE:
                return 0;
        case PIPE_SHADER_CAP_MAX_TEXTURE_SAMPLERS:
        case PIPE_SHADER_CAP_MAX_SAMPLER_VIEWS:
                return VC4_MAX_TEXTURE_SAMPLERS;
        case PIPE_SHADER_CAP_SUPPORTED_IRS:
                return 1 << PIPE_SHADER_IR_NIR;
        case PIPE_SHADER_CAP_MAX_SHADER_BUFFERS:
        case PIPE_SHADER_CAP_MAX_SHADER_IMAGES:
        case PIPE_SHADER_CAP_MAX_HW_ATOMIC_COUNTERS:
        case PIPE_SHADER_CAP_MAX_HW_ATOMIC_COUNTER_BUFFERS:
                return 0;
        default:
                fprintf(stderr, "unknown shader param %d\n", param);
                return 0;
        }
        return 0;
}

static bool
vc4_screen_is_format_supported(struct pipe_screen *pscreen,
                               enum pipe_format format,
                               enum pipe_texture_target target,
                               unsigned sample_count,
                               unsigned storage_sample_count,
                               unsigned usage)
{
        struct vc4_screen *screen = vc4_screen(pscreen);

        if (MAX2(1, sample_count) != MAX2(1, storage_sample_count))
                return false;

        if (sample_count > 1 && sample_count != VC4_MAX_SAMPLES)
                return false;

        if (target >= PIPE_MAX_TEXTURE_TYPES) {
                return false;
        }

        if (usage & PIPE_BIND_VERTEX_BUFFER) {
                switch (format) {
                case PIPE_FORMAT_R32G32B32A32_FLOAT:
                case PIPE_FORMAT_R32G32B32_FLOAT:
                case PIPE_FORMAT_R32G32_FLOAT:
                case PIPE_FORMAT_R32_FLOAT:
                case PIPE_FORMAT_R32G32B32A32_SNORM:
                case PIPE_FORMAT_R32G32B32_SNORM:
                case PIPE_FORMAT_R32G32_SNORM:
                case PIPE_FORMAT_R32_SNORM:
                case PIPE_FORMAT_R32G32B32A32_SSCALED:
                case PIPE_FORMAT_R32G32B32_SSCALED:
                case PIPE_FORMAT_R32G32_SSCALED:
                case PIPE_FORMAT_R32_SSCALED:
                case PIPE_FORMAT_R16G16B16A16_UNORM:
                case PIPE_FORMAT_R16G16B16_UNORM:
                case PIPE_FORMAT_R16G16_UNORM:
                case PIPE_FORMAT_R16_UNORM:
                case PIPE_FORMAT_R16G16B16A16_SNORM:
                case PIPE_FORMAT_R16G16B16_SNORM:
                case PIPE_FORMAT_R16G16_SNORM:
                case PIPE_FORMAT_R16_SNORM:
                case PIPE_FORMAT_R16G16B16A16_USCALED:
                case PIPE_FORMAT_R16G16B16_USCALED:
                case PIPE_FORMAT_R16G16_USCALED:
                case PIPE_FORMAT_R16_USCALED:
                case PIPE_FORMAT_R16G16B16A16_SSCALED:
                case PIPE_FORMAT_R16G16B16_SSCALED:
                case PIPE_FORMAT_R16G16_SSCALED:
                case PIPE_FORMAT_R16_SSCALED:
                case PIPE_FORMAT_R8G8B8A8_UNORM:
                case PIPE_FORMAT_R8G8B8_UNORM:
                case PIPE_FORMAT_R8G8_UNORM:
                case PIPE_FORMAT_R8_UNORM:
                case PIPE_FORMAT_R8G8B8A8_SNORM:
                case PIPE_FORMAT_R8G8B8_SNORM:
                case PIPE_FORMAT_R8G8_SNORM:
                case PIPE_FORMAT_R8_SNORM:
                case PIPE_FORMAT_R8G8B8A8_USCALED:
                case PIPE_FORMAT_R8G8B8_USCALED:
                case PIPE_FORMAT_R8G8_USCALED:
                case PIPE_FORMAT_R8_USCALED:
                case PIPE_FORMAT_R8G8B8A8_SSCALED:
                case PIPE_FORMAT_R8G8B8_SSCALED:
                case PIPE_FORMAT_R8G8_SSCALED:
                case PIPE_FORMAT_R8_SSCALED:
                        break;
                default:
                        return false;
                }
        }

        if ((usage & PIPE_BIND_RENDER_TARGET) &&
            !vc4_rt_format_supported(format)) {
                return false;
        }

        if ((usage & PIPE_BIND_SAMPLER_VIEW) &&
            (!vc4_tex_format_supported(format) ||
             (format == PIPE_FORMAT_ETC1_RGB8 && !screen->has_etc1))) {
                return false;
        }

        if ((usage & PIPE_BIND_DEPTH_STENCIL) &&
            format != PIPE_FORMAT_S8_UINT_Z24_UNORM &&
            format != PIPE_FORMAT_X8Z24_UNORM) {
                return false;
        }

        if ((usage & PIPE_BIND_INDEX_BUFFER) &&
            format != PIPE_FORMAT_R8_UINT &&
            format != PIPE_FORMAT_R16_UINT) {
                return false;
        }

        return true;
}

static const uint64_t *vc4_get_modifiers(struct pipe_screen *pscreen, int *num)
{
        struct vc4_screen *screen = vc4_screen(pscreen);
        static const uint64_t all_modifiers[] = {
                DRM_FORMAT_MOD_BROADCOM_VC4_T_TILED,
                DRM_FORMAT_MOD_LINEAR,
        };
        int m;

        /* We support both modifiers (tiled and linear) for all sampler
         * formats, but if we don't have the DRM_VC4_GET_TILING ioctl
         * we shouldn't advertise the tiled formats.
         */
        if (screen->has_tiling_ioctl) {
                m = 0;
                *num = 2;
        } else{
                m = 1;
                *num = 1;
        }

        return &all_modifiers[m];
}

static void
vc4_screen_query_dmabuf_modifiers(struct pipe_screen *pscreen,
                                  enum pipe_format format, int max,
                                  uint64_t *modifiers,
                                  unsigned int *external_only,
                                  int *count)
{
        const uint64_t *available_modifiers;
        int i;
        bool tex_will_lower;
        int num_modifiers;

        available_modifiers = vc4_get_modifiers(pscreen, &num_modifiers);

        if (!modifiers) {
                *count = num_modifiers;
                return;
        }

        *count = MIN2(max, num_modifiers);
        tex_will_lower = !vc4_tex_format_supported(format);
        for (i = 0; i < *count; i++) {
                modifiers[i] = available_modifiers[i];
                if (external_only)
                        external_only[i] = tex_will_lower;
       }
}

static bool
vc4_screen_is_dmabuf_modifier_supported(struct pipe_screen *pscreen,
                                        uint64_t modifier,
                                        enum pipe_format format,
                                        bool *external_only)
{
        const uint64_t *available_modifiers;
        int i, num_modifiers;

        available_modifiers = vc4_get_modifiers(pscreen, &num_modifiers);

        for (i = 0; i < num_modifiers; i++) {
                if (modifier == available_modifiers[i]) {
                        if (external_only)
                                *external_only = !vc4_tex_format_supported(format);

                        return true;
                }
        }

        return false;
}

static bool
vc4_get_chip_info(struct vc4_screen *screen)
{
        struct drm_vc4_get_param ident0 = {
                .param = DRM_VC4_PARAM_V3D_IDENT0,
        };
        struct drm_vc4_get_param ident1 = {
                .param = DRM_VC4_PARAM_V3D_IDENT1,
        };
        int ret;

        ret = vc4_ioctl(screen->fd, DRM_IOCTL_VC4_GET_PARAM, &ident0);
        if (ret != 0) {
                if (errno == EINVAL) {
                        /* Backwards compatibility with 2835 kernels which
                         * only do V3D 2.1.
                         */
                        screen->v3d_ver = 21;
                        return true;
                } else {
                        fprintf(stderr, "Couldn't get V3D IDENT0: %s\n",
                                strerror(errno));
                        return false;
                }
        }
        ret = vc4_ioctl(screen->fd, DRM_IOCTL_VC4_GET_PARAM, &ident1);
        if (ret != 0) {
                fprintf(stderr, "Couldn't get V3D IDENT1: %s\n",
                        strerror(errno));
                return false;
        }

        uint32_t major = (ident0.value >> 24) & 0xff;
        uint32_t minor = (ident1.value >> 0) & 0xf;
        screen->v3d_ver = major * 10 + minor;

        if (screen->v3d_ver != 21 && screen->v3d_ver != 26) {
                fprintf(stderr,
                        "V3D %d.%d not supported by this version of Mesa.\n",
                        screen->v3d_ver / 10,
                        screen->v3d_ver % 10);
                return false;
        }

        return true;
}

static int
vc4_screen_get_fd(struct pipe_screen *pscreen)
{
        struct vc4_screen *screen = vc4_screen(pscreen);

        return screen->fd;
}

struct pipe_screen *
vc4_screen_create(int fd, const struct pipe_screen_config *config,
                  struct renderonly *ro)
{
        struct vc4_screen *screen = rzalloc(NULL, struct vc4_screen);
        uint64_t syncobj_cap = 0;
        struct pipe_screen *pscreen;
        int err;

        pscreen = &screen->base;

        pscreen->destroy = vc4_screen_destroy;
        pscreen->get_screen_fd = vc4_screen_get_fd;
        pscreen->get_param = vc4_screen_get_param;
        pscreen->get_paramf = vc4_screen_get_paramf;
        pscreen->get_shader_param = vc4_screen_get_shader_param;
        pscreen->context_create = vc4_context_create;
        pscreen->is_format_supported = vc4_screen_is_format_supported;

        screen->fd = fd;
        screen->ro = ro;

        list_inithead(&screen->bo_cache.time_list);
        (void) mtx_init(&screen->bo_handles_mutex, mtx_plain);
        screen->bo_handles = util_hash_table_create_ptr_keys();

        screen->has_control_flow =
                vc4_has_feature(screen, DRM_VC4_PARAM_SUPPORTS_BRANCHES);
        screen->has_etc1 =
                vc4_has_feature(screen, DRM_VC4_PARAM_SUPPORTS_ETC1);
        screen->has_threaded_fs =
                vc4_has_feature(screen, DRM_VC4_PARAM_SUPPORTS_THREADED_FS);
        screen->has_madvise =
                vc4_has_feature(screen, DRM_VC4_PARAM_SUPPORTS_MADVISE);
        screen->has_perfmon_ioctl =
                vc4_has_feature(screen, DRM_VC4_PARAM_SUPPORTS_PERFMON);

        err = drmGetCap(fd, DRM_CAP_SYNCOBJ, &syncobj_cap);
        if (err == 0 && syncobj_cap)
                screen->has_syncobj = true;

        if (!vc4_get_chip_info(screen))
                goto fail;

        slab_create_parent(&screen->transfer_pool, sizeof(struct vc4_transfer), 16);

        vc4_fence_screen_init(screen);

        vc4_mesa_debug = debug_get_option_vc4_debug();

#ifdef USE_VC4_SIMULATOR
        vc4_simulator_init(screen);
#endif

        vc4_resource_screen_init(pscreen);

        pscreen->get_name = vc4_screen_get_name;
        pscreen->get_vendor = vc4_screen_get_vendor;
        pscreen->get_device_vendor = vc4_screen_get_vendor;
        pscreen->get_compiler_options = vc4_screen_get_compiler_options;
        pscreen->query_dmabuf_modifiers = vc4_screen_query_dmabuf_modifiers;
        pscreen->is_dmabuf_modifier_supported = vc4_screen_is_dmabuf_modifier_supported;

        if (screen->has_perfmon_ioctl) {
                pscreen->get_driver_query_group_info = vc4_get_driver_query_group_info;
                pscreen->get_driver_query_info = vc4_get_driver_query_info;
        }

        /* Generate the bitmask of supported draw primitives. */
        screen->prim_types = BITFIELD_BIT(MESA_PRIM_POINTS) |
                             BITFIELD_BIT(MESA_PRIM_LINES) |
                             BITFIELD_BIT(MESA_PRIM_LINE_LOOP) |
                             BITFIELD_BIT(MESA_PRIM_LINE_STRIP) |
                             BITFIELD_BIT(MESA_PRIM_TRIANGLES) |
                             BITFIELD_BIT(MESA_PRIM_TRIANGLE_STRIP) |
                             BITFIELD_BIT(MESA_PRIM_TRIANGLE_FAN);


        return pscreen;

fail:
        close(fd);
        ralloc_free(pscreen);
        return NULL;
}
