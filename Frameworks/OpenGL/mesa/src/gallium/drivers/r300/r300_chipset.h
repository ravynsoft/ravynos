/*
 * Copyright 2008 Corbin Simpson <MostAwesomeDude@gmail.com>
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

#ifndef R300_CHIPSET_H
#define R300_CHIPSET_H

#include "util/compiler.h"

/* these are sizes in dwords */
#define R300_HIZ_LIMIT 10240
#define RV530_HIZ_LIMIT 15360

/* rv3xx have only one pipe */
#define PIPE_ZMASK_SIZE 4096
#define RV3xx_ZMASK_SIZE 5120

/* The size of a compressed tile. Each compressed tile takes 2 bits
 * in the ZMASK RAM, so there is always 16 tiles per one dword. */
enum r300_zmask_compression {
   R300_ZCOMP_4X4 = 4,
   R300_ZCOMP_8X8 = 8
};

/* Structure containing all the possible information about a specific Radeon
 * in the R3xx, R4xx, and R5xx families. */
struct r300_capabilities {
    /* Chipset family */
    int family;
    /* The number of vertex floating-point units */
    unsigned num_vert_fpus;
    /* The number of texture units. */
    unsigned num_tex_units;
    /* Whether or not TCL is physically present */
    bool has_tcl;
    /* Some chipsets do not have HiZ RAM - other have varying amounts. */
    int hiz_ram;
    /* Some chipsets have zmask ram per pipe some don't. */
    int zmask_ram;
    /* CMASK is for MSAA colorbuffer compression and fast clear. */
    bool has_cmask;
    /* Compression mode for ZMASK. */
    enum r300_zmask_compression z_compress;
    /* Whether or not this is RV350 or newer, including all r400 and r500
     * chipsets. The differences compared to the oldest r300 chips are:
     * - Blend LTE/GTE thresholds
     * - Better MACRO_SWITCH in texture tiling
     * - Half float vertex
     * - More HyperZ optimizations */
    bool is_rv350;
    /* Whether or not this is R400. The differences compared their rv350
     * cousins are:
     * - Extended fragment shader registers
     * - 3DC texture compression (RGTC2) */
    bool is_r400;
    /* Whether or not this is an RV515 or newer; R500s have many differences
     * that require extra consideration, compared to their rv350 cousins:
     * - Extra bit of width and height on texture sizes
     * - Blend color is split across two registers
     * - Universal Shader (US) block used for fragment shaders
     * - FP16 blending and multisampling
     * - Full RGTC texture compression
     * - 24-bit depth textures
     * - Stencil back-face reference value
     * - Ability to render up to 2^24 - 1 vertices with signed index offset */
    bool is_r500;
    /* Whether or not the second pixel pipe is accessed with the high bit */
    bool high_second_pipe;
    /* DXTC texture swizzling. */
    bool dxtc_swizzle;
    /* Whether R500_US_FORMAT0_0 exists (R520-only and depends on DRM). */
    bool has_us_format;
};

void r300_parse_chipset(uint32_t pci_id, struct r300_capabilities* caps);

#endif /* R300_CHIPSET_H */
