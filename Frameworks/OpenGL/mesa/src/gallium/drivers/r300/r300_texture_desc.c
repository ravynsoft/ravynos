/*
 * Copyright 2008 Corbin Simpson <MostAwesomeDude@gmail.com>
 * Copyright 2010 Marek Olšák <maraeo@gmail.com>
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
 * USE OR OTHER DEALINGS IN THE SOFTWARE. */

#include "r300_texture_desc.h"
#include "r300_context.h"

#include "util/format/u_format.h"
#include <inttypes.h>

/* Returns the number of pixels that the texture should be aligned to
 * in the given dimension. */
unsigned r300_get_pixel_alignment(enum pipe_format format,
                                  unsigned num_samples,
                                  enum radeon_bo_layout microtile,
                                  enum radeon_bo_layout macrotile,
                                  enum r300_dim dim, bool is_rs690)
{
    static const unsigned table[2][5][3][2] =
    {
        {
    /* Macro: linear    linear    linear
       Micro: linear    tiled  square-tiled */
            {{ 32, 1}, { 8,  4}, { 0,  0}}, /*   8 bits per pixel */
            {{ 16, 1}, { 8,  2}, { 4,  4}}, /*  16 bits per pixel */
            {{  8, 1}, { 4,  2}, { 0,  0}}, /*  32 bits per pixel */
            {{  4, 1}, { 2,  2}, { 0,  0}}, /*  64 bits per pixel */
            {{  2, 1}, { 0,  0}, { 0,  0}}  /* 128 bits per pixel */
        },
        {
    /* Macro: tiled     tiled     tiled
       Micro: linear    tiled  square-tiled */
            {{256, 8}, {64, 32}, { 0,  0}}, /*   8 bits per pixel */
            {{128, 8}, {64, 16}, {32, 32}}, /*  16 bits per pixel */
            {{ 64, 8}, {32, 16}, { 0,  0}}, /*  32 bits per pixel */
            {{ 32, 8}, {16, 16}, { 0,  0}}, /*  64 bits per pixel */
            {{ 16, 8}, { 0,  0}, { 0,  0}}  /* 128 bits per pixel */
        }
    };

    unsigned tile = 0;
    unsigned pixsize = util_format_get_blocksize(format);

    assert(macrotile <= RADEON_LAYOUT_TILED);
    assert(microtile <= RADEON_LAYOUT_SQUARETILED);
    assert(pixsize <= 16);
    assert(dim <= DIM_HEIGHT);

    tile = table[macrotile][util_logbase2(pixsize)][microtile][dim];
    if (macrotile == 0 && is_rs690 && dim == DIM_WIDTH) {
        int align;
        int h_tile;
        h_tile = table[macrotile][util_logbase2(pixsize)][microtile][DIM_HEIGHT];
        align = 64 / (pixsize * h_tile);
        if (tile < align)
            tile = align;
    }

    assert(tile);
    return tile;
}

/* Return true if macrotiling should be enabled on the miplevel. */
static bool r300_texture_macro_switch(struct r300_resource *tex,
                                      unsigned level,
                                      bool rv350_mode,
                                      enum r300_dim dim)
{
    unsigned tile, texdim;

    if (tex->b.nr_samples > 1) {
        return true;
    }

    tile = r300_get_pixel_alignment(tex->b.format, tex->b.nr_samples,
                                    tex->tex.microtile, RADEON_LAYOUT_TILED, dim, 0);
    if (dim == DIM_WIDTH) {
        texdim = u_minify(tex->tex.width0, level);
    } else {
        texdim = u_minify(tex->tex.height0, level);
    }

    /* See TX_FILTER1_n.MACRO_SWITCH. */
    if (rv350_mode) {
        return texdim >= tile;
    } else {
        return texdim > tile;
    }
}

/**
 * Return the stride, in bytes, of the texture image of the given texture
 * at the given level.
 */
static unsigned r300_texture_get_stride(struct r300_screen *screen,
                                        struct r300_resource *tex,
                                        unsigned level)
{
    unsigned tile_width, width, stride;
    bool is_rs690 = (screen->caps.family == CHIP_RS600 ||
                     screen->caps.family == CHIP_RS690 ||
                     screen->caps.family == CHIP_RS740);

    if (tex->tex.stride_in_bytes_override)
        return tex->tex.stride_in_bytes_override;

    /* Check the level. */
    if (level > tex->b.last_level) {
        SCREEN_DBG(screen, DBG_TEX, "%s: level (%u) > last_level (%u)\n",
                   __func__, level, tex->b.last_level);
        return 0;
    }

    width = u_minify(tex->tex.width0, level);

    if (util_format_is_plain(tex->b.format)) {
        tile_width = r300_get_pixel_alignment(tex->b.format,
                                              tex->b.nr_samples,
                                              tex->tex.microtile,
                                              tex->tex.macrotile[level],
                                              DIM_WIDTH, is_rs690);
        width = align(width, tile_width);

        stride = util_format_get_stride(tex->b.format, width);
        /* The alignment to 32 bytes is sort of implied by the layout... */
        return stride;
    } else {
        return align(util_format_get_stride(tex->b.format, width), is_rs690 ? 64 : 32);
    }
}

static unsigned r300_texture_get_nblocksy(struct r300_resource *tex,
                                          unsigned level,
                                          bool *out_aligned_for_cbzb)
{
    unsigned height, tile_height;

    height = u_minify(tex->tex.height0, level);

    /* Mipmapped and 3D textures must have their height aligned to POT. */
    if ((tex->b.target != PIPE_TEXTURE_1D &&
         tex->b.target != PIPE_TEXTURE_2D &&
         tex->b.target != PIPE_TEXTURE_RECT) ||
        tex->b.last_level != 0) {
        height = util_next_power_of_two(height);
    }

    if (util_format_is_plain(tex->b.format)) {
        tile_height = r300_get_pixel_alignment(tex->b.format,
                                               tex->b.nr_samples,
                                               tex->tex.microtile,
                                               tex->tex.macrotile[level],
                                               DIM_HEIGHT, 0);
        height = align(height, tile_height);

        /* See if the CBZB clear can be used on the buffer,
         * taking the texture size into account. */
        if (out_aligned_for_cbzb) {
            if (tex->tex.macrotile[level]) {
                /* When clearing, the layer (width*height) is horizontally split
                 * into two, and the upper and lower halves are cleared by the CB
                 * and ZB units, respectively. Therefore, the number of macrotiles
                 * in the Y direction must be even. */

                /* Align the height so that there is an even number of macrotiles.
                 * Do so for 3 or more macrotiles in the Y direction. */
                if (level == 0 && tex->b.last_level == 0 &&
                    (tex->b.target == PIPE_TEXTURE_1D ||
                     tex->b.target == PIPE_TEXTURE_2D ||
                     tex->b.target == PIPE_TEXTURE_RECT) &&
                    height >= tile_height * 3) {
                    height = align(height, tile_height * 2);
                }

                *out_aligned_for_cbzb = height % (tile_height * 2) == 0;
            } else {
                *out_aligned_for_cbzb = false;
            }
        }
    }

    return util_format_get_nblocksy(tex->b.format, height);
}

/* Get a width in pixels from a stride in bytes. */
unsigned r300_stride_to_width(enum pipe_format format,
                              unsigned stride_in_bytes)
{
    return (stride_in_bytes / util_format_get_blocksize(format)) *
            util_format_get_blockwidth(format);
}

static void r300_setup_miptree(struct r300_screen *screen,
                               struct r300_resource *tex,
                               bool align_for_cbzb)
{
    struct pipe_resource *base = &tex->b;
    unsigned stride, size, layer_size, nblocksy, i;
    bool rv350_mode = screen->caps.family >= CHIP_R350;
    bool aligned_for_cbzb;

    tex->tex.size_in_bytes = 0;

    SCREEN_DBG(screen, DBG_TEXALLOC,
        "r300: Making miptree for texture, format %s\n",
        util_format_short_name(base->format));

    for (i = 0; i <= base->last_level; i++) {
        /* Let's see if this miplevel can be macrotiled. */
        tex->tex.macrotile[i] =
            (tex->tex.macrotile[0] == RADEON_LAYOUT_TILED &&
             r300_texture_macro_switch(tex, i, rv350_mode, DIM_WIDTH) &&
             r300_texture_macro_switch(tex, i, rv350_mode, DIM_HEIGHT)) ?
             RADEON_LAYOUT_TILED : RADEON_LAYOUT_LINEAR;

        stride = r300_texture_get_stride(screen, tex, i);

        /* Compute the number of blocks in Y, see if the CBZB clear can be
         * used on the texture. */
        aligned_for_cbzb = false;
        if (align_for_cbzb && tex->tex.cbzb_allowed[i])
            nblocksy = r300_texture_get_nblocksy(tex, i, &aligned_for_cbzb);
        else
            nblocksy = r300_texture_get_nblocksy(tex, i, NULL);

        layer_size = stride * nblocksy;

        if (base->nr_samples > 1) {
            layer_size *= base->nr_samples;
        }

        if (base->target == PIPE_TEXTURE_CUBE)
            size = layer_size * 6;
        else
            size = layer_size * u_minify(tex->tex.depth0, i);

        tex->tex.offset_in_bytes[i] = tex->tex.size_in_bytes;
        tex->tex.size_in_bytes = tex->tex.offset_in_bytes[i] + size;
        tex->tex.layer_size_in_bytes[i] = layer_size;
        tex->tex.stride_in_bytes[i] = stride;
        tex->tex.cbzb_allowed[i] = tex->tex.cbzb_allowed[i] && aligned_for_cbzb;

        SCREEN_DBG(screen, DBG_TEXALLOC, "r300: Texture miptree: Level %d "
                "(%dx%dx%d px, pitch %d bytes) %d bytes total, macrotiled %s\n",
                i, u_minify(tex->tex.width0, i), u_minify(tex->tex.height0, i),
                u_minify(tex->tex.depth0, i), stride, tex->tex.size_in_bytes,
                tex->tex.macrotile[i] ? "TRUE" : "FALSE");
    }
}

static void r300_setup_flags(struct r300_resource *tex)
{
    tex->tex.uses_stride_addressing =
        !util_is_power_of_two_or_zero(tex->b.width0) ||
        (tex->tex.stride_in_bytes_override &&
         r300_stride_to_width(tex->b.format,
                         tex->tex.stride_in_bytes_override) != tex->b.width0);

    tex->tex.is_npot =
        tex->tex.uses_stride_addressing ||
        !util_is_power_of_two_or_zero(tex->b.height0) ||
        !util_is_power_of_two_or_zero(tex->b.depth0);
}

static void r300_setup_cbzb_flags(struct r300_screen *rscreen,
                                  struct r300_resource *tex)
{
    unsigned i, bpp;
    bool first_level_valid;

    bpp = util_format_get_blocksizebits(tex->b.format);

    /* 1) The texture must be point-sampled,
     * 2) The depth must be 16 or 32 bits.
     * 3) If the midpoint ZB offset is not aligned to 2048, it returns garbage
     *    with certain texture sizes. Macrotiling ensures the alignment. */
    first_level_valid = tex->b.nr_samples <= 1 &&
                       (bpp == 16 || bpp == 32) &&
                       tex->tex.macrotile[0];

    if (SCREEN_DBG_ON(rscreen, DBG_NO_CBZB))
        first_level_valid = false;

    for (i = 0; i <= tex->b.last_level; i++)
        tex->tex.cbzb_allowed[i] = first_level_valid && tex->tex.macrotile[i];
}

static unsigned r300_pixels_to_dwords(unsigned stride,
                                      unsigned height,
                                      unsigned xblock, unsigned yblock)
{
    return (util_align_npot(stride, xblock) * align(height, yblock)) / (xblock * yblock);
}

static void r300_setup_hyperz_properties(struct r300_screen *screen,
                                         struct r300_resource *tex)
{
    /* The tile size of 1 DWORD in ZMASK RAM is:
     *
     * GPU    Pipes    4x4 mode   8x8 mode
     * ------------------------------------------
     * R580   4P/1Z    32x32      64x64
     * RV570  3P/1Z    48x16      96x32
     * RV530  1P/2Z    32x16      64x32
     *        1P/1Z    16x16      32x32
     */
    static unsigned zmask_blocks_x_per_dw[4] = {4, 8, 12, 8};
    static unsigned zmask_blocks_y_per_dw[4] = {4, 4,  4, 8};

    /* In HIZ RAM, one dword is always 8x8 pixels (each byte is 4x4 pixels),
     * but the blocks have very weird ordering.
     *
     * With 2 pipes and an image of size 8xY, where Y >= 1,
     * clearing 4 dwords clears blocks like this:
     *
     *    01012323
     *
     * where numbers correspond to dword indices. The blocks are interleaved
     * in the X direction, so the alignment must be 4x1 blocks (32x8 pixels).
     *
     * With 4 pipes and an image of size 8xY, where Y >= 4,
     * clearing 8 dwords clears blocks like this:
     *    01012323
     *    45456767
     *    01012323
     *    45456767
     * where numbers correspond to dword indices. The blocks are interleaved
     * in both directions, so the alignment must be 4x4 blocks (32x32 pixels)
     */
    static unsigned hiz_align_x[4] = {8, 32, 48, 32};
    static unsigned hiz_align_y[4] = {8, 8, 8, 32};

    if (util_format_is_depth_or_stencil(tex->b.format) &&
        util_format_get_blocksizebits(tex->b.format) == 32 &&
        tex->tex.microtile) {
        unsigned i, pipes;

        if (screen->caps.family == CHIP_RV530) {
            pipes = screen->info.r300_num_z_pipes;
        } else {
            pipes = screen->info.r300_num_gb_pipes;
        }

        for (i = 0; i <= tex->b.last_level; i++) {
            unsigned zcomp_numdw, zcompsize, hiz_numdw, stride, height;

            stride = r300_stride_to_width(tex->b.format,
                                          tex->tex.stride_in_bytes[i]);
            stride = align(stride, 16);
            height = u_minify(tex->b.height0, i);

            /* The 8x8 compression mode needs macrotiling. */
            zcompsize = screen->caps.z_compress == R300_ZCOMP_8X8 &&
                       tex->tex.macrotile[i] &&
                       tex->b.nr_samples <= 1 ? 8 : 4;

            /* Get the ZMASK buffer size in dwords. */
            zcomp_numdw = r300_pixels_to_dwords(stride, height,
                                zmask_blocks_x_per_dw[pipes-1] * zcompsize,
                                zmask_blocks_y_per_dw[pipes-1] * zcompsize);

            /* Check whether we have enough ZMASK memory. */
            if (util_format_get_blocksizebits(tex->b.format) == 32 &&
                zcomp_numdw <= screen->caps.zmask_ram * pipes) {
                tex->tex.zmask_dwords[i] = zcomp_numdw;
                tex->tex.zcomp8x8[i] = zcompsize == 8;

                tex->tex.zmask_stride_in_pixels[i] =
                    util_align_npot(stride, zmask_blocks_x_per_dw[pipes-1] * zcompsize);
            } else {
                tex->tex.zmask_dwords[i] = 0;
                tex->tex.zcomp8x8[i] = false;
                tex->tex.zmask_stride_in_pixels[i] = 0;
            }

            /* Now setup HIZ. */
            stride = util_align_npot(stride, hiz_align_x[pipes-1]);
            height = align(height, hiz_align_y[pipes-1]);

            /* Get the HIZ buffer size in dwords. */
            hiz_numdw = (stride * height) / (8*8 * pipes);

            /* Check whether we have enough HIZ memory. */
            if (hiz_numdw <= screen->caps.hiz_ram * pipes) {
                tex->tex.hiz_dwords[i] = hiz_numdw;
                tex->tex.hiz_stride_in_pixels[i] = stride;
            } else {
                tex->tex.hiz_dwords[i] = 0;
                tex->tex.hiz_stride_in_pixels[i] = 0;
            }
        }
    }
}

static void r300_setup_cmask_properties(struct r300_screen *screen,
                                        struct r300_resource *tex)
{
    static unsigned cmask_align_x[4] = {16, 32, 48, 32};
    static unsigned cmask_align_y[4] = {16, 16, 16, 32};
    unsigned pipes, stride, cmask_num_dw, cmask_max_size;

    if (!screen->caps.has_cmask) {
        return;
    }

    /* We need an AA colorbuffer, no mipmaps. */
    if (tex->b.nr_samples <= 1 ||
        tex->b.last_level > 0 ||
        util_format_is_depth_or_stencil(tex->b.format)) {
        return;
    }

    /* FP16 AA needs R500 and a fairly new DRM. */
    if ((tex->b.format == PIPE_FORMAT_R16G16B16A16_FLOAT ||
         tex->b.format == PIPE_FORMAT_R16G16B16X16_FLOAT) &&
        !screen->caps.is_r500) {
        return;
    }

    if (SCREEN_DBG_ON(screen, DBG_NO_CMASK)) {
        return;
    }

    /* CMASK is part of raster pipes. The number of Z pipes doesn't matter. */
    pipes = screen->info.r300_num_gb_pipes;

    /* The single-pipe cards have 5120 dwords of CMASK RAM,
     * the other cards have 4096 dwords of CMASK RAM per pipe. */
    cmask_max_size = pipes == 1 ? 5120 : pipes * 4096;

    stride = r300_stride_to_width(tex->b.format,
                                  tex->tex.stride_in_bytes[0]);
    stride = align(stride, 16);

    /* Get the CMASK size in dwords. */
    cmask_num_dw = r300_pixels_to_dwords(stride, tex->b.height0,
                                         cmask_align_x[pipes-1],
                                         cmask_align_y[pipes-1]);

    /* Check the CMASK size against the CMASK memory limit. */
    if (cmask_num_dw <= cmask_max_size) {
        tex->tex.cmask_dwords = cmask_num_dw;
        tex->tex.cmask_stride_in_pixels =
            util_align_npot(stride, cmask_align_x[pipes-1]);
    }
}

static void r300_setup_tiling(struct r300_screen *screen,
                              struct r300_resource *tex)
{
    enum pipe_format format = tex->b.format;
    bool rv350_mode = screen->caps.family >= CHIP_R350;
    bool is_zb = util_format_is_depth_or_stencil(format);
    bool dbg_no_tiling = SCREEN_DBG_ON(screen, DBG_NO_TILING);
    bool force_microtiling =
        (tex->b.flags & R300_RESOURCE_FORCE_MICROTILING) != 0;

    if (tex->b.nr_samples > 1) {
        tex->tex.microtile = RADEON_LAYOUT_TILED;
        tex->tex.macrotile[0] = RADEON_LAYOUT_TILED;
        return;
    }

    tex->tex.microtile = RADEON_LAYOUT_LINEAR;
    tex->tex.macrotile[0] = RADEON_LAYOUT_LINEAR;

    if (tex->b.usage == PIPE_USAGE_STAGING) {
       return;
    }

    if (!util_format_is_plain(format)) {
        return;
    }

    /* If height == 1, disable microtiling except for zbuffer. */
    if (!force_microtiling && !is_zb &&
        (tex->b.height0 == 1 || dbg_no_tiling)) {
        return;
    }

    /* Set microtiling. */
    switch (util_format_get_blocksize(format)) {
        case 1:
        case 4:
        case 8:
            tex->tex.microtile = RADEON_LAYOUT_TILED;
            break;

        case 2:
            tex->tex.microtile = RADEON_LAYOUT_SQUARETILED;
            break;
    }

    if (dbg_no_tiling) {
        return;
    }

    /* Set macrotiling. */
    if (r300_texture_macro_switch(tex, 0, rv350_mode, DIM_WIDTH) &&
        r300_texture_macro_switch(tex, 0, rv350_mode, DIM_HEIGHT)) {
        tex->tex.macrotile[0] = RADEON_LAYOUT_TILED;
    }
}

static void r300_tex_print_info(struct r300_resource *tex,
                                const char *func)
{
    fprintf(stderr,
            "r300: %s: Macro: %s, Micro: %s, Pitch: %i, Dim: %ix%ix%i, "
            "LastLevel: %i, Size: %i, Format: %s, Samples: %i\n",
            func,
            tex->tex.macrotile[0] ? "YES" : " NO",
            tex->tex.microtile ? "YES" : " NO",
            r300_stride_to_width(tex->b.format, tex->tex.stride_in_bytes[0]),
            tex->b.width0, tex->b.height0, tex->b.depth0,
            tex->b.last_level, tex->tex.size_in_bytes,
            util_format_short_name(tex->b.format),
            tex->b.nr_samples);
}

void r300_texture_desc_init(struct r300_screen *rscreen,
                            struct r300_resource *tex,
                            const struct pipe_resource *base)
{
    tex->b.target = base->target;
    tex->b.format = base->format;
    tex->b.width0 = base->width0;
    tex->b.height0 = base->height0;
    tex->b.depth0 = base->depth0;
    tex->b.array_size = base->array_size;
    tex->b.last_level = base->last_level;
    tex->b.nr_samples = base->nr_samples;
    tex->tex.width0 = base->width0;
    tex->tex.height0 = base->height0;
    tex->tex.depth0 = base->depth0;

    /* There is a CB memory addressing hardware bug that limits the width
     * of the MSAA buffer in some cases in R520. In order to get around it,
     * the following code lowers the sample count depending on the format and
     * the width.
     *
     * The only catch is that all MSAA colorbuffers and a zbuffer which are
     * supposed to be used together should always be bound together. Only
     * then the correct minimum sample count of all bound buffers is used
     * for rendering. */
    if (rscreen->caps.is_r500) {
        /* FP16 6x MSAA buffers are limited to a width of 1360 pixels. */
        if ((tex->b.format == PIPE_FORMAT_R16G16B16A16_FLOAT ||
             tex->b.format == PIPE_FORMAT_R16G16B16X16_FLOAT) &&
            tex->b.nr_samples == 6 && tex->b.width0 > 1360) {
            tex->b.nr_samples = 4;
        }

        /* FP16 4x MSAA buffers are limited to a width of 2048 pixels. */
        if ((tex->b.format == PIPE_FORMAT_R16G16B16A16_FLOAT ||
             tex->b.format == PIPE_FORMAT_R16G16B16X16_FLOAT) &&
            tex->b.nr_samples == 4 && tex->b.width0 > 2048) {
            tex->b.nr_samples = 2;
        }
    }

    /* 32-bit 6x MSAA buffers are limited to a width of 2720 pixels.
     * This applies to all R300-R500 cards. */
    if (util_format_get_blocksizebits(tex->b.format) == 32 &&
        !util_format_is_depth_or_stencil(tex->b.format) &&
        tex->b.nr_samples == 6 && tex->b.width0 > 2720) {
        tex->b.nr_samples = 4;
    }

    r300_setup_flags(tex);

    /* Align a 3D NPOT texture to POT. */
    if (base->target == PIPE_TEXTURE_3D && tex->tex.is_npot) {
        tex->tex.width0 = util_next_power_of_two(tex->tex.width0);
        tex->tex.height0 = util_next_power_of_two(tex->tex.height0);
        tex->tex.depth0 = util_next_power_of_two(tex->tex.depth0);
    }

    /* Setup tiling. */
    if (tex->tex.microtile == RADEON_LAYOUT_UNKNOWN) {
        r300_setup_tiling(rscreen, tex);
    }

    r300_setup_cbzb_flags(rscreen, tex);

    /* Setup the miptree description. */
    r300_setup_miptree(rscreen, tex, true);
    /* If the required buffer size is larger than the given max size,
     * try again without the alignment for the CBZB clear. */
    if (tex->buf && tex->tex.size_in_bytes > tex->buf->size) {
        r300_setup_miptree(rscreen, tex, false);

        /* Make sure the buffer we got is large enough. */
        if (tex->tex.size_in_bytes > tex->buf->size) {
            fprintf(stderr,
                "r300: I got a pre-allocated buffer to use it as a texture "
                "storage, but the buffer is too small. I'll use the buffer "
                "anyway, because I can't crash here, but it's dangerous. "
                "This can be a DDX bug. Got: %"PRIu64"B, Need: %uB, Info:\n",
                tex->buf->size, tex->tex.size_in_bytes);
            r300_tex_print_info(tex, "texture_desc_init");
            /* Oops, what now. Apps will break if we fail this,
             * so just pretend everything's okay. */
        }
    }

    r300_setup_hyperz_properties(rscreen, tex);
    r300_setup_cmask_properties(rscreen, tex);

    if (SCREEN_DBG_ON(rscreen, DBG_TEX))
        r300_tex_print_info(tex, "texture_desc_init");
}

unsigned r300_texture_get_offset(struct r300_resource *tex,
                                 unsigned level, unsigned layer)
{
    unsigned offset = tex->tex.offset_in_bytes[level];

    switch (tex->b.target) {
        case PIPE_TEXTURE_3D:
        case PIPE_TEXTURE_CUBE:
            return offset + layer * tex->tex.layer_size_in_bytes[level];

        default:
            assert(layer == 0);
            return offset;
    }
}
