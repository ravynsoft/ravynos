/*
 * Copyright © 2014-2017 Broadcom
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

#include <sys/sysinfo.h>

#include "common/v3d_device_info.h"
#include "common/v3d_limits.h"
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
#include "util/xmlconfig.h"

#include <xf86drm.h>
#include "v3d_screen.h"
#include "v3d_context.h"
#include "v3d_resource.h"
#include "compiler/v3d_compiler.h"
#include "drm-uapi/drm_fourcc.h"

static const char *
v3d_screen_get_name(struct pipe_screen *pscreen)
{
        struct v3d_screen *screen = v3d_screen(pscreen);

        if (!screen->name) {
                screen->name = ralloc_asprintf(screen,
                                               "V3D %d.%d.%d",
                                               screen->devinfo.ver / 10,
                                               screen->devinfo.ver % 10,
                                               screen->devinfo.rev);
        }

        return screen->name;
}

static const char *
v3d_screen_get_vendor(struct pipe_screen *pscreen)
{
        return "Broadcom";
}

static void
v3d_screen_destroy(struct pipe_screen *pscreen)
{
        struct v3d_screen *screen = v3d_screen(pscreen);

        _mesa_hash_table_destroy(screen->bo_handles, NULL);
        v3d_bufmgr_destroy(pscreen);
        slab_destroy_parent(&screen->transfer_pool);
        if (screen->ro)
                screen->ro->destroy(screen->ro);

        if (using_v3d_simulator)
                v3d_simulator_destroy(screen->sim_file);

        v3d_compiler_free(screen->compiler);

#ifdef ENABLE_SHADER_CACHE
        if (screen->disk_cache)
                disk_cache_destroy(screen->disk_cache);
#endif

        u_transfer_helper_destroy(pscreen->transfer_helper);

        close(screen->fd);
        ralloc_free(pscreen);
}

static bool
v3d_has_feature(struct v3d_screen *screen, enum drm_v3d_param feature)
{
        struct drm_v3d_get_param p = {
                .param = feature,
        };
        int ret = v3d_ioctl(screen->fd, DRM_IOCTL_V3D_GET_PARAM, &p);

        if (ret != 0)
                return false;

        return p.value;
}

static int
v3d_screen_get_param(struct pipe_screen *pscreen, enum pipe_cap param)
{
        struct v3d_screen *screen = v3d_screen(pscreen);

        switch (param) {
                /* Supported features (boolean caps). */
        case PIPE_CAP_VERTEX_COLOR_UNCLAMPED:
        case PIPE_CAP_NPOT_TEXTURES:
        case PIPE_CAP_BLEND_EQUATION_SEPARATE:
        case PIPE_CAP_TEXTURE_MULTISAMPLE:
        case PIPE_CAP_TEXTURE_SWIZZLE:
        case PIPE_CAP_VERTEX_ELEMENT_INSTANCE_DIVISOR:
        case PIPE_CAP_START_INSTANCE:
        case PIPE_CAP_VS_INSTANCEID:
        case PIPE_CAP_FRAGMENT_SHADER_TEXTURE_LOD:
        case PIPE_CAP_FRAGMENT_SHADER_DERIVATIVES:
        case PIPE_CAP_PRIMITIVE_RESTART_FIXED_INDEX:
        case PIPE_CAP_EMULATE_NONFIXED_PRIMITIVE_RESTART:
        case PIPE_CAP_PRIMITIVE_RESTART:
        case PIPE_CAP_OCCLUSION_QUERY:
        case PIPE_CAP_STREAM_OUTPUT_PAUSE_RESUME:
        case PIPE_CAP_DRAW_INDIRECT:
        case PIPE_CAP_MULTI_DRAW_INDIRECT:
        case PIPE_CAP_QUADS_FOLLOW_PROVOKING_VERTEX_CONVENTION:
        case PIPE_CAP_SIGNED_VERTEX_BUFFER_OFFSET:
        case PIPE_CAP_SHADER_CAN_READ_OUTPUTS:
        case PIPE_CAP_SHADER_PACK_HALF_FLOAT:
        case PIPE_CAP_TEXTURE_HALF_FLOAT_LINEAR:
        case PIPE_CAP_FRAMEBUFFER_NO_ATTACHMENT:
        case PIPE_CAP_FS_FACE_IS_INTEGER_SYSVAL:
        case PIPE_CAP_TGSI_TEXCOORD:
        case PIPE_CAP_TEXTURE_MIRROR_CLAMP_TO_EDGE:
        case PIPE_CAP_SAMPLER_VIEW_TARGET:
        case PIPE_CAP_ANISOTROPIC_FILTER:
        case PIPE_CAP_COPY_BETWEEN_COMPRESSED_AND_PLAIN_FORMATS:
        case PIPE_CAP_INDEP_BLEND_FUNC:
        case PIPE_CAP_CONDITIONAL_RENDER:
        case PIPE_CAP_CONDITIONAL_RENDER_INVERTED:
        case PIPE_CAP_CUBE_MAP_ARRAY:
        case PIPE_CAP_NIR_COMPACT_ARRAYS:
                return 1;

        case PIPE_CAP_POLYGON_OFFSET_CLAMP:
                return screen->devinfo.ver >= 42;


        case PIPE_CAP_TEXTURE_QUERY_LOD:
                return screen->devinfo.ver >= 42;
                break;

        case PIPE_CAP_PACKED_UNIFORMS:
                /* We can't enable this flag, because it results in load_ubo
                 * intrinsics across a 16b boundary, but v3d's TMU general
                 * memory accesses wrap on 16b boundaries.
                 */
                return 0;

        case PIPE_CAP_NIR_IMAGES_AS_DEREF:
                return 0;

        case PIPE_CAP_TEXTURE_TRANSFER_MODES:
                /* XXX perf: we don't want to emit these extra blits for
                 * glReadPixels(), since we still have to do an uncached read
                 * from the GPU of the result after waiting for the TFU blit
                 * to happen.  However, disabling this introduces instability
                 * in
                 * dEQP-GLES31.functional.image_load_store.early_fragment_tests.*
                 * and corruption in chromium's rendering.
                 */
                return PIPE_TEXTURE_TRANSFER_BLIT;

        case PIPE_CAP_COMPUTE:
                return screen->has_csd && screen->devinfo.ver >= 42;

        case PIPE_CAP_GENERATE_MIPMAP:
                return v3d_has_feature(screen, DRM_V3D_PARAM_SUPPORTS_TFU);

        case PIPE_CAP_INDEP_BLEND_ENABLE:
                return 1;

        case PIPE_CAP_CONSTANT_BUFFER_OFFSET_ALIGNMENT:
                return V3D_NON_COHERENT_ATOM_SIZE;

        case PIPE_CAP_MAX_TEXTURE_GATHER_COMPONENTS:
                return 4;

        case PIPE_CAP_SHADER_BUFFER_OFFSET_ALIGNMENT:
                if (screen->has_cache_flush)
                        return 4;
                else
                        return 0; /* Disables shader storage */

        case PIPE_CAP_GLSL_FEATURE_LEVEL:
                return 330;

        case PIPE_CAP_ESSL_FEATURE_LEVEL:
                return 310;

	case PIPE_CAP_GLSL_FEATURE_LEVEL_COMPATIBILITY:
		return 140;

        case PIPE_CAP_FS_COORD_ORIGIN_UPPER_LEFT:
                return 1;
        case PIPE_CAP_FS_COORD_ORIGIN_LOWER_LEFT:
                return 0;
        case PIPE_CAP_FS_COORD_PIXEL_CENTER_INTEGER:
                return 0;
        case PIPE_CAP_FS_COORD_PIXEL_CENTER_HALF_INTEGER:
                return 1;

        case PIPE_CAP_MIXED_FRAMEBUFFER_SIZES:
        case PIPE_CAP_MIXED_COLOR_DEPTH_BITS:
                return 1;

        case PIPE_CAP_MAX_STREAM_OUTPUT_BUFFERS:
                return 4;

        case PIPE_CAP_MAX_VARYINGS:
                return V3D_MAX_FS_INPUTS / 4;

                /* Texturing. */
        case PIPE_CAP_MAX_TEXTURE_2D_SIZE:
                if (screen->nonmsaa_texture_size_limit)
                        return 7680;
                else
                        return V3D_MAX_IMAGE_DIMENSION;
        case PIPE_CAP_MAX_TEXTURE_CUBE_LEVELS:
        case PIPE_CAP_MAX_TEXTURE_3D_LEVELS:
                return V3D_MAX_MIP_LEVELS;
        case PIPE_CAP_MAX_TEXTURE_ARRAY_LAYERS:
                return V3D_MAX_ARRAY_LAYERS;

        case PIPE_CAP_MAX_RENDER_TARGETS:
                return V3D_MAX_RENDER_TARGETS(screen->devinfo.ver);

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
        case PIPE_CAP_FLATSHADE:
        case PIPE_CAP_TWO_SIDED_COLOR:
        case PIPE_CAP_VERTEX_COLOR_CLAMPED:
        case PIPE_CAP_FRAGMENT_COLOR_CLAMPED:
        case PIPE_CAP_GL_CLAMP:
                return 0;

        /* Geometry shaders */
        case PIPE_CAP_MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS:
                /* Minimum required by GLES 3.2 */
                return 1024;
        case PIPE_CAP_MAX_GEOMETRY_OUTPUT_VERTICES:
                /* MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS / 4 */
                return 256;
        case PIPE_CAP_MAX_GS_INVOCATIONS:
                return 32;

        case PIPE_CAP_SUPPORTED_PRIM_MODES:
        case PIPE_CAP_SUPPORTED_PRIM_MODES_WITH_RESTART:
                return screen->prim_types;

        case PIPE_CAP_TEXTURE_BUFFER_OBJECTS:
                return true;

        case PIPE_CAP_TEXTURE_BUFFER_OFFSET_ALIGNMENT:
                return V3D_TMU_TEXEL_ALIGN;

        case PIPE_CAP_IMAGE_STORE_FORMATTED:
                return false;

        case PIPE_CAP_NATIVE_FENCE_FD:
                return true;

        default:
                return u_pipe_screen_get_param_defaults(pscreen, param);
        }
}

static float
v3d_screen_get_paramf(struct pipe_screen *pscreen, enum pipe_capf param)
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
                return V3D_MAX_LINE_WIDTH;

        case PIPE_CAPF_MAX_POINT_SIZE:
        case PIPE_CAPF_MAX_POINT_SIZE_AA:
                return V3D_MAX_POINT_SIZE;

        case PIPE_CAPF_MAX_TEXTURE_ANISOTROPY:
                return 16.0f;
        case PIPE_CAPF_MAX_TEXTURE_LOD_BIAS:
                return 16.0f;

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
v3d_screen_get_shader_param(struct pipe_screen *pscreen, enum pipe_shader_type shader,
                           enum pipe_shader_cap param)
{
        struct v3d_screen *screen = v3d_screen(pscreen);

        switch (shader) {
        case PIPE_SHADER_VERTEX:
        case PIPE_SHADER_FRAGMENT:
                break;
        case PIPE_SHADER_COMPUTE:
                if (!screen->has_csd)
                        return 0;
                break;
        case PIPE_SHADER_GEOMETRY:
                if (screen->devinfo.ver < 42)
                        return 0;
                break;
        default:
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
                return UINT_MAX;

        case PIPE_SHADER_CAP_MAX_INPUTS:
                switch (shader) {
                case PIPE_SHADER_VERTEX:
                        return V3D_MAX_VS_INPUTS / 4;
                case PIPE_SHADER_GEOMETRY:
                        return V3D_MAX_GS_INPUTS / 4;
                case PIPE_SHADER_FRAGMENT:
                        return V3D_MAX_FS_INPUTS / 4;
                default:
                        return 0;
                };
        case PIPE_SHADER_CAP_MAX_OUTPUTS:
                if (shader == PIPE_SHADER_FRAGMENT)
                        return 4;
                else
                        return V3D_MAX_FS_INPUTS / 4;
        case PIPE_SHADER_CAP_MAX_TEMPS:
                return 256; /* GL_MAX_PROGRAM_TEMPORARIES_ARB */
        case PIPE_SHADER_CAP_MAX_CONST_BUFFER0_SIZE:
                /* Note: Limited by the offset size in
                 * v3d_unit_data_create().
                 */
                return 16 * 1024 * sizeof(float);
        case PIPE_SHADER_CAP_MAX_CONST_BUFFERS:
                return 16;
        case PIPE_SHADER_CAP_CONT_SUPPORTED:
                return 0;
        case PIPE_SHADER_CAP_INDIRECT_INPUT_ADDR:
                /* We don't currently support this in the backend, but that is
                 * okay because our NIR compiler sets the option
                 * lower_all_io_to_temps, which will eliminate indirect
                 * indexing on all input/output variables by translating it to
                 * indirect indexing on temporary variables instead, which we
                 * will then lower to scratch. We prefer this over setting this
                 * to 0, which would cause if-ladder injection to eliminate
                 * indirect indexing on inputs.
                 */
                return 1;
        case PIPE_SHADER_CAP_INDIRECT_OUTPUT_ADDR:
                return 1;
        case PIPE_SHADER_CAP_INDIRECT_TEMP_ADDR:
                return 1;
        case PIPE_SHADER_CAP_INDIRECT_CONST_ADDR:
                return 1;
        case PIPE_SHADER_CAP_SUBROUTINES:
                return 0;
        case PIPE_SHADER_CAP_INTEGERS:
                return 1;
        case PIPE_SHADER_CAP_FP16:
        case PIPE_SHADER_CAP_FP16_DERIVATIVES:
        case PIPE_SHADER_CAP_FP16_CONST_BUFFERS:
        case PIPE_SHADER_CAP_INT16:
        case PIPE_SHADER_CAP_GLSL_16BIT_CONSTS:
        case PIPE_SHADER_CAP_TGSI_ANY_INOUT_DECL_RANGE:
        case PIPE_SHADER_CAP_TGSI_SQRT_SUPPORTED:
        case PIPE_SHADER_CAP_MAX_HW_ATOMIC_COUNTERS:
        case PIPE_SHADER_CAP_MAX_HW_ATOMIC_COUNTER_BUFFERS:
                return 0;
        case PIPE_SHADER_CAP_MAX_TEXTURE_SAMPLERS:
        case PIPE_SHADER_CAP_MAX_SAMPLER_VIEWS:
                return V3D_MAX_TEXTURE_SAMPLERS;

        case PIPE_SHADER_CAP_MAX_SHADER_BUFFERS:
                if (screen->has_cache_flush) {
                        if (shader == PIPE_SHADER_VERTEX ||
                            shader == PIPE_SHADER_GEOMETRY) {
                                return 0;
                        }
                        return PIPE_MAX_SHADER_BUFFERS;
                 } else {
                        return 0;
                 }

        case PIPE_SHADER_CAP_MAX_SHADER_IMAGES:
                if (screen->has_cache_flush) {
                        if (screen->devinfo.ver < 42)
                                return 0;
                        else
                                return PIPE_MAX_SHADER_IMAGES;
                } else {
                        return 0;
                }

        case PIPE_SHADER_CAP_SUPPORTED_IRS:
                return 1 << PIPE_SHADER_IR_NIR;
        default:
                fprintf(stderr, "unknown shader param %d\n", param);
                return 0;
        }
        return 0;
}

static int
v3d_get_compute_param(struct pipe_screen *pscreen, enum pipe_shader_ir ir_type,
                      enum pipe_compute_cap param, void *ret)
{
        struct v3d_screen *screen = v3d_screen(pscreen);

        if (!screen->has_csd)
                return 0;

#define RET(x) do {                                     \
                if (ret)                                \
                        memcpy(ret, x, sizeof(x));      \
                return sizeof(x);                       \
        } while (0)

        switch (param) {
        case PIPE_COMPUTE_CAP_ADDRESS_BITS:
                RET((uint32_t []) { 32 });
                break;

        case PIPE_COMPUTE_CAP_IR_TARGET:
                sprintf(ret, "v3d");
                return strlen(ret);

        case PIPE_COMPUTE_CAP_GRID_DIMENSION:
                RET((uint64_t []) { 3 });

        case PIPE_COMPUTE_CAP_MAX_GRID_SIZE:
                /* GL_MAX_COMPUTE_SHADER_WORK_GROUP_COUNT: The CSD has a
                 * 16-bit field for the number of workgroups in each
                 * dimension.
                 */
                RET(((uint64_t []) { 65535, 65535, 65535 }));

        case PIPE_COMPUTE_CAP_MAX_BLOCK_SIZE:
                /* GL_MAX_COMPUTE_WORK_GROUP_SIZE */
                RET(((uint64_t []) { 256, 256, 256 }));

        case PIPE_COMPUTE_CAP_MAX_THREADS_PER_BLOCK:
        case PIPE_COMPUTE_CAP_MAX_VARIABLE_THREADS_PER_BLOCK:
                /* GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS: This is
                 * limited by WG_SIZE in the CSD.
                 */
                RET((uint64_t []) { 256 });

        case PIPE_COMPUTE_CAP_MAX_GLOBAL_SIZE:
                RET((uint64_t []) { 1024 * 1024 * 1024 });

        case PIPE_COMPUTE_CAP_MAX_LOCAL_SIZE:
                /* GL_MAX_COMPUTE_SHARED_MEMORY_SIZE */
                RET((uint64_t []) { 32768 });

        case PIPE_COMPUTE_CAP_MAX_PRIVATE_SIZE:
        case PIPE_COMPUTE_CAP_MAX_INPUT_SIZE:
                RET((uint64_t []) { 4096 });

        case PIPE_COMPUTE_CAP_MAX_MEM_ALLOC_SIZE: {
                struct sysinfo si;
                sysinfo(&si);
                RET((uint64_t []) { si.totalram });
        }

        case PIPE_COMPUTE_CAP_MAX_CLOCK_FREQUENCY:
                /* OpenCL only */
                RET((uint32_t []) { 0 });

        case PIPE_COMPUTE_CAP_MAX_COMPUTE_UNITS:
                RET((uint32_t []) { 1 });

        case PIPE_COMPUTE_CAP_IMAGES_SUPPORTED:
                RET((uint32_t []) { 1 });

        case PIPE_COMPUTE_CAP_SUBGROUP_SIZES:
                RET((uint32_t []) { 16 });

        case PIPE_COMPUTE_CAP_MAX_SUBGROUPS:
                RET((uint32_t []) { 0 });

        }

        return 0;
}

static bool
v3d_screen_is_format_supported(struct pipe_screen *pscreen,
                               enum pipe_format format,
                               enum pipe_texture_target target,
                               unsigned sample_count,
                               unsigned storage_sample_count,
                               unsigned usage)
{
        struct v3d_screen *screen = v3d_screen(pscreen);

        if (MAX2(1, sample_count) != MAX2(1, storage_sample_count))
                return false;

        if (sample_count > 1 && sample_count != V3D_MAX_SAMPLES)
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
                case PIPE_FORMAT_R16G16B16A16_FLOAT:
                case PIPE_FORMAT_R16G16B16_UNORM:
                case PIPE_FORMAT_R16G16_UNORM:
                case PIPE_FORMAT_R16_UNORM:
                case PIPE_FORMAT_R16_FLOAT:
                case PIPE_FORMAT_R16G16B16A16_SNORM:
                case PIPE_FORMAT_R16G16B16_SNORM:
                case PIPE_FORMAT_R16G16_SNORM:
                case PIPE_FORMAT_R16G16_FLOAT:
                case PIPE_FORMAT_R16_SNORM:
                case PIPE_FORMAT_R16G16B16A16_USCALED:
                case PIPE_FORMAT_R16G16B16_USCALED:
                case PIPE_FORMAT_R16G16_USCALED:
                case PIPE_FORMAT_R16_USCALED:
                case PIPE_FORMAT_R16G16B16A16_SSCALED:
                case PIPE_FORMAT_R16G16B16_SSCALED:
                case PIPE_FORMAT_R16G16_SSCALED:
                case PIPE_FORMAT_R16_SSCALED:
                case PIPE_FORMAT_B8G8R8A8_UNORM:
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
                case PIPE_FORMAT_R10G10B10A2_UNORM:
                case PIPE_FORMAT_B10G10R10A2_UNORM:
                case PIPE_FORMAT_R10G10B10A2_SNORM:
                case PIPE_FORMAT_B10G10R10A2_SNORM:
                case PIPE_FORMAT_R10G10B10A2_USCALED:
                case PIPE_FORMAT_B10G10R10A2_USCALED:
                case PIPE_FORMAT_R10G10B10A2_SSCALED:
                case PIPE_FORMAT_B10G10R10A2_SSCALED:
                        break;
                default:
                        return false;
                }
        }

        /* FORMAT_NONE gets allowed for ARB_framebuffer_no_attachments's probe
         * of FRAMEBUFFER_MAX_SAMPLES
         */
        if ((usage & PIPE_BIND_RENDER_TARGET) &&
            format != PIPE_FORMAT_NONE &&
            !v3d_rt_format_supported(&screen->devinfo, format)) {
                return false;
        }

        if ((usage & PIPE_BIND_SAMPLER_VIEW) &&
            !v3d_tex_format_supported(&screen->devinfo, format)) {
                return false;
        }

        if ((usage & PIPE_BIND_DEPTH_STENCIL) &&
            !(format == PIPE_FORMAT_S8_UINT_Z24_UNORM ||
              format == PIPE_FORMAT_X8Z24_UNORM ||
              format == PIPE_FORMAT_Z16_UNORM ||
              format == PIPE_FORMAT_Z32_FLOAT ||
              format == PIPE_FORMAT_Z32_FLOAT_S8X24_UINT)) {
                return false;
        }

        if ((usage & PIPE_BIND_INDEX_BUFFER) &&
            !(format == PIPE_FORMAT_R8_UINT ||
              format == PIPE_FORMAT_R16_UINT ||
              format == PIPE_FORMAT_R32_UINT)) {
                return false;
        }

        if (usage & PIPE_BIND_SHADER_IMAGE) {
                switch (format) {
                /* FIXME: maybe we can implement a swizzle-on-writes to add
                 * support for BGRA-alike formats.
                 */
                case PIPE_FORMAT_A4B4G4R4_UNORM:
                case PIPE_FORMAT_A1B5G5R5_UNORM:
                case PIPE_FORMAT_B5G6R5_UNORM:
                case PIPE_FORMAT_B8G8R8A8_UNORM:
                case PIPE_FORMAT_X8Z24_UNORM:
                case PIPE_FORMAT_Z16_UNORM:
                        return false;
                default:
                        return true;
                }
        }

        return true;
}

static const nir_shader_compiler_options v3d_nir_options = {
        .lower_uadd_sat = true,
        .lower_usub_sat = true,
        .lower_iadd_sat = true,
        .lower_all_io_to_temps = true,
        .lower_extract_byte = true,
        .lower_extract_word = true,
        .lower_insert_byte = true,
        .lower_insert_word = true,
        .lower_bitfield_insert = true,
        .lower_bitfield_extract = true,
        .lower_bitfield_reverse = true,
        .lower_bit_count = true,
        .lower_cs_local_id_to_index = true,
        .lower_ffract = true,
        .lower_fmod = true,
        .lower_pack_unorm_2x16 = true,
        .lower_pack_snorm_2x16 = true,
        .lower_pack_unorm_4x8 = true,
        .lower_pack_snorm_4x8 = true,
        .lower_unpack_unorm_4x8 = true,
        .lower_unpack_snorm_4x8 = true,
        .lower_pack_half_2x16 = true,
        .lower_unpack_half_2x16 = true,
        .lower_pack_32_2x16 = true,
        .lower_pack_32_2x16_split = true,
        .lower_unpack_32_2x16_split = true,
        .lower_fdiv = true,
        .lower_find_lsb = true,
        .lower_ffma16 = true,
        .lower_ffma32 = true,
        .lower_ffma64 = true,
        .lower_flrp32 = true,
        .lower_fpow = true,
        .lower_fsat = true,
        .lower_fsqrt = true,
        .lower_ifind_msb = true,
        .lower_isign = true,
        .lower_ldexp = true,
        .lower_mul_high = true,
        .lower_wpos_pntc = true,
        .lower_to_scalar = true,
        .lower_int64_options = nir_lower_imul_2x32_64,
        .lower_fquantize2f16 = true,
        .has_fsub = true,
        .has_isub = true,
        .divergence_analysis_options =
                nir_divergence_multiple_workgroup_per_compute_subgroup,
        /* This will enable loop unrolling in the state tracker so we won't
         * be able to selectively disable it in backend if it leads to
         * lower thread counts or TMU spills. Choose a conservative maximum to
         * limit register pressure impact.
         */
        .max_unroll_iterations = 16,
        .force_indirect_unrolling_sampler = true,
};

static const void *
v3d_screen_get_compiler_options(struct pipe_screen *pscreen,
                                enum pipe_shader_ir ir, enum pipe_shader_type shader)
{
        return &v3d_nir_options;
}

static const uint64_t v3d_available_modifiers[] = {
   DRM_FORMAT_MOD_BROADCOM_UIF,
   DRM_FORMAT_MOD_LINEAR,
   DRM_FORMAT_MOD_BROADCOM_SAND128,
};

static void
v3d_screen_query_dmabuf_modifiers(struct pipe_screen *pscreen,
                                  enum pipe_format format, int max,
                                  uint64_t *modifiers,
                                  unsigned int *external_only,
                                  int *count)
{
        int i;
        int num_modifiers = ARRAY_SIZE(v3d_available_modifiers);

        switch (format) {
        case PIPE_FORMAT_P030:
                /* Expose SAND128, but not LINEAR or UIF */
                *count = 1;
                if (modifiers && max > 0) {
                        modifiers[0] = DRM_FORMAT_MOD_BROADCOM_SAND128;
                        if (external_only)
                                external_only[0] = true;
                }
                return;

        case PIPE_FORMAT_NV12:
                /* Expose UIF, LINEAR and SAND128 */
                break;
        
        case PIPE_FORMAT_R8_UNORM:
        case PIPE_FORMAT_R8G8_UNORM:
        case PIPE_FORMAT_R16_UNORM:
        case PIPE_FORMAT_R16G16_UNORM:
                /* Expose UIF, LINEAR and SAND128 */
		if (!modifiers) break;
                *count = MIN2(max, num_modifiers);
                for (i = 0; i < *count; i++) {
                        modifiers[i] = v3d_available_modifiers[i];
                        if (external_only)
                                external_only[i] = modifiers[i] == DRM_FORMAT_MOD_BROADCOM_SAND128;
                }
                return;

        default:
                /* Expose UIF and LINEAR, but not SAND128 */
                num_modifiers--;
        }

        if (!modifiers) {
                *count = num_modifiers;
                return;
        }

        *count = MIN2(max, num_modifiers);
        for (i = 0; i < *count; i++) {
                modifiers[i] = v3d_available_modifiers[i];
                if (external_only)
                        external_only[i] = util_format_is_yuv(format);
        }
}

static bool
v3d_screen_is_dmabuf_modifier_supported(struct pipe_screen *pscreen,
                                        uint64_t modifier,
                                        enum pipe_format format,
                                        bool *external_only)
{
        int i;
        if (fourcc_mod_broadcom_mod(modifier) == DRM_FORMAT_MOD_BROADCOM_SAND128) {
                switch(format) {
                case PIPE_FORMAT_NV12:
                case PIPE_FORMAT_P030:
                case PIPE_FORMAT_R8_UNORM:
                case PIPE_FORMAT_R8G8_UNORM:
                case PIPE_FORMAT_R16_UNORM:
                case PIPE_FORMAT_R16G16_UNORM:
                        if (external_only)
                                *external_only = true;
                        return true;
                default:
                        return false;
                }
        } else if (format == PIPE_FORMAT_P030) {
                /* For PIPE_FORMAT_P030 we don't expose LINEAR or UIF. */
                return false;
        }

        /* We don't want to generally allow DRM_FORMAT_MOD_BROADCOM_SAND128
         * modifier, that is the last v3d_available_modifiers. We only accept
         * it in the case of having a PIPE_FORMAT_NV12 or PIPE_FORMAT_P030.
         */
        assert(v3d_available_modifiers[ARRAY_SIZE(v3d_available_modifiers) - 1] ==
               DRM_FORMAT_MOD_BROADCOM_SAND128);
        for (i = 0; i < ARRAY_SIZE(v3d_available_modifiers) - 1; i++) {
                if (v3d_available_modifiers[i] == modifier) {
                        if (external_only)
                                *external_only = util_format_is_yuv(format);

                        return true;
                }
        }

        return false;
}

static enum pipe_format
v3d_screen_get_compatible_tlb_format(struct pipe_screen *screen,
                                     enum pipe_format format)
{
        switch (format) {
        case PIPE_FORMAT_R16G16_UNORM:
                return PIPE_FORMAT_R16G16_UINT;
        default:
                return format;
        }
}

static struct disk_cache *
v3d_screen_get_disk_shader_cache(struct pipe_screen *pscreen)
{
        struct v3d_screen *screen = v3d_screen(pscreen);

        return screen->disk_cache;
}

static int
v3d_screen_get_fd(struct pipe_screen *pscreen)
{
        struct v3d_screen *screen = v3d_screen(pscreen);

        return screen->fd;
}

struct pipe_screen *
v3d_screen_create(int fd, const struct pipe_screen_config *config,
                  struct renderonly *ro)
{
        struct v3d_screen *screen = rzalloc(NULL, struct v3d_screen);
        struct pipe_screen *pscreen;

        pscreen = &screen->base;

        pscreen->destroy = v3d_screen_destroy;
        pscreen->get_screen_fd = v3d_screen_get_fd;
        pscreen->get_param = v3d_screen_get_param;
        pscreen->get_paramf = v3d_screen_get_paramf;
        pscreen->get_shader_param = v3d_screen_get_shader_param;
        pscreen->get_compute_param = v3d_get_compute_param;
        pscreen->context_create = v3d_context_create;
        pscreen->is_format_supported = v3d_screen_is_format_supported;
        pscreen->get_canonical_format = v3d_screen_get_compatible_tlb_format;

        screen->fd = fd;
        screen->ro = ro;

        list_inithead(&screen->bo_cache.time_list);
        (void)mtx_init(&screen->bo_handles_mutex, mtx_plain);
        screen->bo_handles = util_hash_table_create_ptr_keys();

#if defined(USE_V3D_SIMULATOR)
        screen->sim_file = v3d_simulator_init(screen->fd);
#endif

        if (!v3d_get_device_info(screen->fd, &screen->devinfo, &v3d_ioctl))
                goto fail;

        driParseConfigFiles(config->options, config->options_info, 0, "v3d",
                            NULL, NULL, NULL, 0, NULL, 0);

        /* We have to driCheckOption for the simulator mode to not assertion
         * fail on not having our XML config.
         */
        const char *nonmsaa_name = "v3d_nonmsaa_texture_size_limit";
        screen->nonmsaa_texture_size_limit =
                driCheckOption(config->options, nonmsaa_name, DRI_BOOL) &&
                driQueryOptionb(config->options, nonmsaa_name);

        slab_create_parent(&screen->transfer_pool, sizeof(struct v3d_transfer), 16);

        screen->has_csd = v3d_has_feature(screen, DRM_V3D_PARAM_SUPPORTS_CSD);
        screen->has_cache_flush =
                v3d_has_feature(screen, DRM_V3D_PARAM_SUPPORTS_CACHE_FLUSH);
        screen->has_perfmon = v3d_has_feature(screen, DRM_V3D_PARAM_SUPPORTS_PERFMON);

        v3d_fence_screen_init(screen);

        v3d_process_debug_variable();

        v3d_resource_screen_init(pscreen);

        screen->compiler = v3d_compiler_init(&screen->devinfo, 0);

#ifdef ENABLE_SHADER_CACHE
        v3d_disk_cache_init(screen);
#endif

        pscreen->get_name = v3d_screen_get_name;
        pscreen->get_vendor = v3d_screen_get_vendor;
        pscreen->get_device_vendor = v3d_screen_get_vendor;
        pscreen->get_compiler_options = v3d_screen_get_compiler_options;
        pscreen->get_disk_shader_cache = v3d_screen_get_disk_shader_cache;
        pscreen->query_dmabuf_modifiers = v3d_screen_query_dmabuf_modifiers;
        pscreen->is_dmabuf_modifier_supported =
                v3d_screen_is_dmabuf_modifier_supported;

        if (screen->has_perfmon) {
                pscreen->get_driver_query_group_info = v3d_get_driver_query_group_info;
                pscreen->get_driver_query_info = v3d_get_driver_query_info;
        }

        /* Generate the bitmask of supported draw primitives. */
        screen->prim_types = BITFIELD_BIT(MESA_PRIM_POINTS) |
                             BITFIELD_BIT(MESA_PRIM_LINES) |
                             BITFIELD_BIT(MESA_PRIM_LINE_LOOP) |
                             BITFIELD_BIT(MESA_PRIM_LINE_STRIP) |
                             BITFIELD_BIT(MESA_PRIM_TRIANGLES) |
                             BITFIELD_BIT(MESA_PRIM_TRIANGLE_STRIP) |
                             BITFIELD_BIT(MESA_PRIM_TRIANGLE_FAN) |
                             BITFIELD_BIT(MESA_PRIM_LINES_ADJACENCY) |
                             BITFIELD_BIT(MESA_PRIM_LINE_STRIP_ADJACENCY) |
                             BITFIELD_BIT(MESA_PRIM_TRIANGLES_ADJACENCY) |
                             BITFIELD_BIT(MESA_PRIM_TRIANGLE_STRIP_ADJACENCY);

        return pscreen;

fail:
        close(fd);
        ralloc_free(pscreen);
        return NULL;
}
