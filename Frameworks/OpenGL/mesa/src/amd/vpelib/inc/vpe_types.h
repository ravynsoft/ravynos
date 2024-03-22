/* Copyright 2022 Advanced Micro Devices, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE COPYRIGHT HOLDER(S) OR AUTHOR(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * Authors: AMD
 *
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <limits.h>
#include "vpe_hw_types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct vpe;

#define MAX_NB_POLYPHASE_COEFFS                                                                    \
    (8 * 33) /* currently vpe supports up to 8 taps and 64 phases, only (32+1) phases needed*/

enum vpe_status {
    VPE_STATUS_OK = 1,
    VPE_STATUS_ERROR,
    VPE_STATUS_NO_MEMORY,

    // errors for not supported operations
    VPE_STATUS_NOT_SUPPORTED,
    VPE_STATUS_DCC_NOT_SUPPORTED,
    VPE_STATUS_SWIZZLE_NOT_SUPPORTED,
    VPE_STATUS_NUM_STREAM_NOT_SUPPORTED,
    VPE_STATUS_PIXEL_FORMAT_NOT_SUPPORTED,
    VPE_STATUS_COLOR_SPACE_VALUE_NOT_SUPPORTED,
    VPE_STATUS_SCALING_RATIO_NOT_SUPPORTED,
    VPE_STATUS_PITCH_ALIGNMENT_NOT_SUPPORTED,
    VPE_STATUS_ROTATION_NOT_SUPPORTED,
    VPE_STATUS_MIRROR_NOT_SUPPORTED,
    VPE_STATUS_ALPHA_BLENDING_NOT_SUPPORTED,
    VPE_STATUS_VIEWPORT_SIZE_NOT_SUPPORTED,
    VPE_STATUS_LUMA_KEYING_NOT_SUPPORTED,
    VPE_STATUS_PLANE_ADDR_NOT_SUPPORTED,
    VPE_STATUS_ADJUSTMENT_NOT_SUPPORTED,
    VPE_STATUS_CMD_OVERFLOW_ERROR,
    VPE_STATUS_SEGMENT_WIDTH_ERROR,
    VPE_STATUS_PARAM_CHECK_ERROR,
    VPE_STATUS_TONE_MAP_NOT_SUPPORTED,
    VPE_STATUS_BAD_TONE_MAP_PARAMS,
    VPE_STATUS_BAD_HDR_METADATA,
    VPE_STATUS_BUFFER_OVERFLOW,
    VPE_STATUS_BUFFER_UNDERRUN,
    VPE_STATUS_BG_COLOR_OUT_OF_RANGE,
    VPE_STATUS_REPEAT_ITEM,
    VPE_STATUS_PATCH_OVER_MAXSIZE,
    VPE_STATUS_INVALID_BUFFER_SIZE,
    VPE_STATUS_SCALER_NOT_SET
};

/** HW IP level */
enum vpe_ip_level {
    VPE_IP_LEVEL_UNKNOWN = (-1),
    VPE_IP_LEVEL_1_0,
};

/****************************************
 * Plane Caps
 ****************************************/
struct vpe_pixel_format_support {
    uint32_t argb_packed_32b : 1;
    uint32_t nv12            : 1;
    uint32_t fp16            : 1;
    uint32_t p010            : 1; /**< planar 4:2:0 10-bit */
    uint32_t p016            : 1; /**< planar 4:2:0 16-bit */
    uint32_t ayuv            : 1; /**< packed 4:4:4 */
    uint32_t yuy2            : 1; /**< packed 4:2:2 */
};

struct vpe_plane_caps {
    uint32_t per_pixel_alpha : 1;

    struct vpe_pixel_format_support input_pixel_format_support;
    struct vpe_pixel_format_support output_pixel_format_support;

    /* max upscaling factor x 1000
     * upscaling factors are always >= 1
     * e.g. 1080p -> 8K is 4.0 => 4000
     */
    uint32_t max_upscale_factor;

    /* max downscale factor x1000
     * downscale factors are always <= 1
     * e.g 8K -> 1080p is 0.25 => 250
     */
    uint32_t max_downscale_factor;

    uint32_t pitch_alignment; /**< alignment in bytes */
    uint32_t addr_alignment;  /**< alignment in bytes */
    uint32_t max_viewport_width;
};

/*************************
 * Color management caps
 *************************/
struct vpe_rom_curve_caps {
    uint32_t srgb     : 1;
    uint32_t bt2020   : 1;
    uint32_t gamma2_2 : 1;
    uint32_t pq       : 1;
    uint32_t hlg      : 1;
};

struct dpp_color_caps {
    uint32_t                  pre_csc    : 1;
    uint32_t                  luma_key   : 1;
    uint32_t                  dgam_ram   : 1;
    uint32_t                  post_csc   : 1; /**< before gamut remap */
    uint32_t                  gamma_corr : 1;
    uint32_t                  hw_3dlut   : 1;
    uint32_t                  ogam_ram   : 1;
    uint32_t                  ocsc       : 1;
    struct vpe_rom_curve_caps dgam_rom_caps;
};

struct mpc_color_caps {
    uint32_t gamut_remap         : 1;
    uint32_t ogam_ram            : 1;
    uint32_t ocsc                : 1;
    uint32_t shared_3d_lut       : 1; /**< can be in either dpp or mpc, but single instance */
    uint32_t global_alpha        : 1; /**< e.g. top plane 30 %. bottom 70 % */
    uint32_t top_bottom_blending : 1; /**< two-layer blending */
};

struct vpe_color_caps {
    struct dpp_color_caps dpp;
    struct mpc_color_caps mpc;
};

/**************************************************
 * VPE Capabilities.
 *
 * Those depend on the condition like input format
 * shall be queried by vpe_cap_funcs
 **************************************************/
struct vpe_caps {
    uint32_t max_downscale_ratio; /**< max downscaling ratio in hundred.
                                       ratio as src/dest x 100. e.g 600 */
    uint64_t lut_size;            /**< 3dlut size */

    uint32_t rotation_support       : 1;
    uint32_t h_mirror_support       : 1;
    uint32_t v_mirror_support       : 1;
    uint32_t is_apu                 : 1;
    uint32_t bg_color_check_support : 1;
    struct {
        int num_dpp;
        int num_opp;
        int num_mpc_3dlut;

        int num_queue; /**< num of hw queue */
    } resource_caps;

    struct vpe_color_caps color_caps;
    struct vpe_plane_caps plane_caps;
};

/***********************************
 * Conditional Capabilities
 ***********************************/
/** DCC CAP */
struct vpe_dcc_surface_param {
    struct vpe_size               surface_size;
    enum vpe_surface_pixel_format format;
    enum vpe_swizzle_mode_values  swizzle_mode;
    enum vpe_scan_direction       scan;
};

struct vpe_dcc_setting {
    unsigned int max_compressed_blk_size;
    unsigned int max_uncompressed_blk_size;
    bool         independent_64b_blks;

    struct {
        uint32_t dcc_256_64_64             : 1;
        uint32_t dcc_128_128_uncontrained  : 1;
        uint32_t dcc_256_128_128           : 1;
        uint32_t dcc_256_256_unconstrained : 1;
    } dcc_controls;
};

struct vpe_surface_dcc_cap {
    union {
        struct {
            struct vpe_dcc_setting rgb;
        } grph;

        struct {
            struct vpe_dcc_setting luma;
            struct vpe_dcc_setting chroma;
        } video;
    };

    bool capable;
    bool const_color_support;
};

/** Conditional Capability functions */
struct vpe_cap_funcs {
    /**
     * Get DCC support and setting according to the format,
     * scan direction and  swizzle mdoe.
     *
     * @param[in]      vpe           vpe instance
     * @param[in]      input         surface and scan properties
     * @param[in/out]  output        dcc capable result and related settings
     * @return true if supported
     */
    bool (*get_dcc_compression_cap)(const struct vpe *vpe,
        const struct vpe_dcc_surface_param *input, struct vpe_surface_dcc_cap *output);
};

/****************************************
 * VPE Init Param
 ****************************************/
/** Log function
 * @param[in] log_ctx  given in the struct vpe_init_params
 * @param[in] fmt      format string
 */
typedef void (*vpe_log_func_t)(void *log_ctx, const char *fmt, ...);

/** system memory zalloc, allocated memory initailized with 0
 *
 * @param[in] mem_ctx  given in the struct vpe_init_params
 * @param[in] size     number of bytes
 * @return             allocated memory
 */
typedef void *(*vpe_zalloc_func_t)(void *mem_ctx, size_t size);

/** system memory free
 * @param[in] mem_ctx  given in the struct vpe_init_params
 * @param[in] ptr      number of bytes
 */
typedef void (*vpe_free_func_t)(void *mem_ctx, void *ptr);

struct vpe_callback_funcs {
    void          *log_ctx; /**< optional. provided by the caller and pass back to callback */
    vpe_log_func_t log;

    void             *mem_ctx; /**< optional. provided by the caller and pass back to callback */
    vpe_zalloc_func_t zalloc;
    vpe_free_func_t   free;
};

struct vpe_mem_low_power_enable_options {
    // override flags
    struct {
        uint32_t dscl : 1;
        uint32_t cm   : 1;
        uint32_t mpc  : 1;
    } flags;

    struct {
        uint32_t dscl : 1;
        uint32_t cm   : 1;
        uint32_t mpc  : 1;
    } bits;
};

enum vpe_expansion_mode {
    VPE_EXPANSION_MODE_DYNAMIC,
    VPE_EXPANSION_MODE_ZERO
};

enum vpe_clamping_range {
    VPE_CLAMPING_FULL_RANGE = 0,      /* No Clamping */
    VPE_CLAMPING_LIMITED_RANGE_8BPC,  /* 8  bpc: Clamping 1  to FE */
    VPE_CLAMPING_LIMITED_RANGE_10BPC, /* 10 bpc: Clamping 4  to 3FB */
    VPE_CLAMPING_LIMITED_RANGE_12BPC, /* 12 bpc: Clamping 10 to FEF */
    /* Use programmable clampping value on FMT_CLAMP_COMPONENT_R/G/B. */
    VPE_CLAMPING_LIMITED_RANGE_PROGRAMMABLE
};

struct vpe_clamping_params {
    enum vpe_clamping_range clamping_range;
    uint32_t                r_clamp_component_upper;
    uint32_t                b_clamp_component_upper;
    uint32_t                g_clamp_component_upper;
    uint32_t                r_clamp_component_lower;
    uint32_t                b_clamp_component_lower;
    uint32_t                g_clamp_component_lower;
};

struct vpe_visual_confirm {
    union {
        struct {
            uint32_t input_format  : 1;
            uint32_t output_format : 1;
            uint32_t reserved      : 30;
        };
        uint32_t value;
    };
};

/** configurable params for debugging purpose */
struct vpe_debug_options {
    // override flags
    struct {
        uint32_t cm_in_bypass            : 1;
        uint32_t vpcnvc_bypass           : 1;
        uint32_t mpc_bypass              : 1;
        uint32_t identity_3dlut          : 1;
        uint32_t sce_3dlut               : 1;
        uint32_t disable_reuse_bit       : 1;
        uint32_t bg_color_fill_only      : 1;
        uint32_t assert_when_not_support : 1;
        uint32_t bypass_gamcor           : 1;
        uint32_t bypass_ogam             : 1;
        uint32_t force_tf_calculation    : 1;
        uint32_t bypass_dpp_gamut_remap  : 1;
        uint32_t bypass_post_csc         : 1;
        uint32_t clamping_setting        : 1;
        uint32_t expansion_mode          : 1;
        uint32_t bypass_per_pixel_alpha  : 1;
        uint32_t dpp_crc_ctrl            : 1;
        uint32_t opp_pipe_crc_ctrl       : 1;
        uint32_t mpc_crc_ctrl            : 1;
        uint32_t bg_bit_depth            : 1;
        uint32_t visual_confirm          : 1;
    } flags;

    // valid only if the corresponding flag is set
    uint32_t cm_in_bypass            : 1;
    uint32_t vpcnvc_bypass           : 1;
    uint32_t mpc_bypass              : 1;
    uint32_t identity_3dlut          : 1;
    uint32_t sce_3dlut               : 1;
    uint32_t disable_reuse_bit       : 1;
    uint32_t bg_color_fill_only      : 1;
    uint32_t assert_when_not_support : 1;
    uint32_t bypass_gamcor           : 1;
    uint32_t bypass_ogam             : 1;
    uint32_t force_tf_calculation    : 1;
    uint32_t bypass_dpp_gamut_remap  : 1;
    uint32_t bypass_post_csc         : 1;
    uint32_t clamping_setting        : 1;
    uint32_t bypass_per_pixel_alpha  : 1;
    uint32_t dpp_crc_ctrl            : 1;
    uint32_t opp_pipe_crc_ctrl       : 1;
    uint32_t mpc_crc_ctrl            : 1;
    uint32_t bg_bit_depth;

    struct vpe_mem_low_power_enable_options enable_mem_low_power;
    enum vpe_expansion_mode                 expansion_mode;
    struct vpe_clamping_params              clamping_params;
    struct vpe_visual_confirm               visual_confirm_params;
};

struct vpe_init_data {
    /** vpe ip info */
    uint8_t ver_major;
    uint8_t ver_minor;
    uint8_t ver_rev;

    /** function callbacks */
    struct vpe_callback_funcs funcs;

    /** debug options */
    struct vpe_debug_options debug;
};

/** VPE instance created through vpelib entry function vpe_create() */
struct vpe {
    uint32_t          version;       /**< API version */
    enum vpe_ip_level level;         /**< HW IP level */

    struct vpe_caps      *caps;      /**< general static chip caps */
    struct vpe_cap_funcs *cap_funcs; /**< conditional caps */
};

/*****************************************************
 * Structures for build VPE command
 *****************************************************/
enum vpe_pixel_encoding {
    VPE_PIXEL_ENCODING_YCbCr,
    VPE_PIXEL_ENCODING_RGB,
    VPE_PIXEL_ENCODING_COUNT
};

enum vpe_color_range {
    VPE_COLOR_RANGE_FULL,
    VPE_COLOR_RANGE_STUDIO,
    VPE_COLOR_RANGE_COUNT
};

enum vpe_chroma_cositing {
    VPE_CHROMA_COSITING_NONE,
    VPE_CHROMA_COSITING_LEFT,
    VPE_CHROMA_COSITING_TOPLEFT,
    VPE_CHROMA_COSITING_COUNT
};

enum vpe_color_primaries {
    VPE_PRIMARIES_BT601,
    VPE_PRIMARIES_BT709,
    VPE_PRIMARIES_BT2020,
    VPE_PRIMARIES_JFIF,
    VPE_PRIMARIES_COUNT
};

enum vpe_transfer_function {
    VPE_TF_G22,
    VPE_TF_G24,
    VPE_TF_G10,
    VPE_TF_PQ,
    VPE_TF_PQ_NORMALIZED,
    VPE_TF_HLG,
    VPE_TF_COUNT
};

enum vpe_alpha_mode {
    VPE_ALPHA_OPAQUE,
    VPE_ALPHA_BGCOLOR
};

struct vpe_color_space {
    enum vpe_pixel_encoding    encoding;
    enum vpe_color_range       range;
    enum vpe_transfer_function tf;
    enum vpe_chroma_cositing   cositing;
    enum vpe_color_primaries   primaries;
};

/* component values are in the range: 0 - 1.0f */
struct vpe_color_rgba {
    float r;
    float g;
    float b;
    float a;
};

struct vpe_color_ycbcra {
    float y;
    float cb;
    float cr;
    float a;
};

struct vpe_color {
    bool is_ycbcr;
    union {
        struct vpe_color_rgba   rgba;
        struct vpe_color_ycbcra ycbcra;
    };
};

/**
 * Adjustment     Min      Max    default   step
 * Brightness  -100.0f,  100.0f,   0.0f,    0.1f
 * Contrast       0.0f,    2.0f,    1.0f,   0.01f
 * Hue         -180.0f,  180.0f,   0.0f,    1.0f
 * Saturation     0.0f,    3.0f,   1.0f,    0.01f
 *
 */
struct vpe_color_adjust {
    float brightness;
    float contrast;
    float hue;
    float saturation;
};

struct vpe_surface_info {

    /** surface addressing info */
    struct vpe_plane_address     address;
    enum vpe_swizzle_mode_values swizzle;

    /** surface properties */
    struct vpe_plane_size         plane_size; /**< pitch */
    struct vpe_plane_dcc_param    dcc;
    enum vpe_surface_pixel_format format;

    struct vpe_color_space cs;
};

struct vpe_blend_info {
    bool  blending;             /**< enable blending */
    bool  pre_multiplied_alpha; /**< is the pixel value pre-multiplied with alpha */
    bool  global_alpha;         /**< enable global alpha */
    float global_alpha_value;   /**< global alpha value, should be 0.0~1.0 */
};

struct vpe_scaling_info {

    struct vpe_rect         src_rect;
    struct vpe_rect         dst_rect;
    struct vpe_scaling_taps taps;
};

struct vpe_scaling_filter_coeffs {

    struct vpe_scaling_taps taps;
    unsigned int            nb_phases;
    uint16_t horiz_polyphase_coeffs[MAX_NB_POLYPHASE_COEFFS]; /*max nb of taps is 4, max nb of
                                                                 phases 33 = (32+1)*/
    uint16_t vert_polyphase_coeffs[MAX_NB_POLYPHASE_COEFFS]; /*max nb of taps is 4, max nb of phases
                                                                33 = (32+1)*/
};

struct vpe_hdr_metadata {
    uint16_t redX;
    uint16_t redY;
    uint16_t greenX;
    uint16_t greenY;
    uint16_t blueX;
    uint16_t blueY;
    uint16_t whiteX;
    uint16_t whiteY;

    uint32_t min_mastering; // luminance in 1/10000 nits
    uint32_t max_mastering; // luminance in nits
    uint32_t max_content;
    uint32_t avg_content;
};

struct vpe_tonemap_params {
    enum vpe_transfer_function shaper_tf;
    enum vpe_transfer_function lut_out_tf;
    enum vpe_color_primaries   lut_in_gamut;
    enum vpe_color_primaries   lut_out_gamut;
    uint16_t                   lut_dim;
    uint16_t                  *lut_data;

    bool update_3dlut;
    bool enable_3dlut;
};

struct vpe_stream {
    struct vpe_surface_info          surface_info;
    struct vpe_scaling_info          scaling_info;
    struct vpe_blend_info            blend_info;
    struct vpe_color_adjust          color_adj;
    struct vpe_tonemap_params        tm_params;
    struct vpe_hdr_metadata          hdr_metadata;
    struct vpe_scaling_filter_coeffs polyphase_scaling_coeffs;
    enum vpe_rotation_angle          rotation;
    bool                             horizontal_mirror;
    bool                             vertical_mirror;
    bool                             use_external_scaling_coeffs;
    bool                             enable_luma_key;
    float                            lower_luma_bound;
    float                            upper_luma_bound;

    struct {
        uint32_t hdr_metadata : 1;
        uint32_t reserved     : 31;
    } flags;
};

struct vpe_build_param {
    /** source */
    uint32_t           num_streams;
    struct vpe_stream *streams;

    /** destination */
    struct vpe_surface_info dst_surface;
    struct vpe_rect target_rect; /**< rectangle in target surface to be blt'd. Ranges out of rect
                                    won't be touched */
    struct vpe_color        bg_color;
    enum vpe_alpha_mode     alpha_mode;
    struct vpe_hdr_metadata hdr_metadata;

    // data flags
    struct {
        uint32_t hdr_metadata : 1;
        uint32_t reserved     : 31;
    } flags;

};

/** reported through vpe_check_support()
 * Once the operation is supported,
 * it returns the required memory for storing
 * 1. command buffer
 * 2. embedded buffer
 *    - Pointed by the command buffer content.
 *    - Shall be free'ed together with command buffer once
 *      command is finished.
 */
struct vpe_bufs_req {
    uint64_t cmd_buf_size; /**< total command buffer size for all vpe commands */
    uint64_t emb_buf_size; /**< total size for storing all embedded data */
};

struct vpe_buf {
    uint64_t gpu_va; /**< GPU start address of the buffer */
    uint64_t cpu_va;
    int64_t  size;
    bool     tmz; /**< allocated from tmz */
};

struct vpe_build_bufs {
    struct vpe_buf cmd_buf; /**< Command buffer. gpu_va is optional */
    struct vpe_buf emb_buf; /**< Embedded buffer */
};

#ifdef __cplusplus
}
#endif
