/*
 * Copyright 2009 Nicolai Haehnle <nhaehnle@gmail.com>
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

#include "r300_context.h"

#include "util/u_debug.h"

#include <stdio.h>

static const struct debug_named_value r300_debug_options[] = {
    { "info", DBG_INFO, "Print hardware info (printed by default on debug builds"},
    { "fp", DBG_FP, "Log fragment program compilation" },
    { "vp", DBG_VP, "Log vertex program compilation" },
    { "draw", DBG_DRAW, "Log draw calls" },
    { "swtcl", DBG_SWTCL, "Log SWTCL-specific info" },
    { "rsblock", DBG_RS_BLOCK, "Log rasterizer registers" },
    { "psc", DBG_PSC, "Log vertex stream registers" },
    { "tex", DBG_TEX, "Log basic info about textures" },
    { "texalloc", DBG_TEXALLOC, "Log texture mipmap tree info" },
    { "rs", DBG_RS, "Log rasterizer" },
    { "fb", DBG_FB, "Log framebuffer" },
    { "cbzb", DBG_CBZB, "Log fast color clear info" },
    { "hyperz", DBG_HYPERZ, "Log HyperZ info" },
    { "scissor", DBG_SCISSOR, "Log scissor info" },
    { "msaa", DBG_MSAA, "Log MSAA resources"},
    { "anisohq", DBG_ANISOHQ, "Use high quality anisotropic filtering" },
    { "notiling", DBG_NO_TILING, "Disable tiling" },
    { "noimmd", DBG_NO_IMMD, "Disable immediate mode" },
    { "noopt", DBG_NO_OPT, "Disable shader optimizations" },
    { "nocbzb", DBG_NO_CBZB, "Disable fast color clear" },
    { "nozmask", DBG_NO_ZMASK, "Disable zbuffer compression" },
    { "nohiz", DBG_NO_HIZ, "Disable hierarchical zbuffer" },
    { "nocmask", DBG_NO_CMASK, "Disable AA compression and fast AA clear" },
    { "notcl", DBG_NO_TCL, "Disable hardware accelerated Transform/Clip/Lighting" },

    /* must be last */
    DEBUG_NAMED_VALUE_END
};

void r300_init_debug(struct r300_screen * screen)
{
    screen->debug = debug_get_flags_option("RADEON_DEBUG", r300_debug_options, 0);
}

void r500_dump_rs_block(struct r300_rs_block *rs)
{
    unsigned count, ip, it_count, ic_count, i, j;
    unsigned tex_ptr;
    unsigned col_ptr, col_fmt;

    count = rs->inst_count & 0xf;
    count++;

    it_count = rs->count & 0x7f;
    ic_count = (rs->count >> 7) & 0xf;

    fprintf(stderr, "RS Block: %d texcoords (linear), %d colors (perspective)\n",
        it_count, ic_count);
    fprintf(stderr, "%d instructions\n", count);

    for (i = 0; i < count; i++) {
        if (rs->inst[i] & 0x10) {
            ip = rs->inst[i] & 0xf;
            fprintf(stderr, "texture: ip %d to psf %d\n",
                ip, (rs->inst[i] >> 5) & 0x7f);

            tex_ptr = rs->ip[ip] & 0xffffff;
            fprintf(stderr, "       : ");

            j = 3;
            do {
                if ((tex_ptr & 0x3f) == 63) {
                    fprintf(stderr, "1.0");
                } else if ((tex_ptr & 0x3f) == 62) {
                    fprintf(stderr, "0.0");
                } else {
                    fprintf(stderr, "[%d]", tex_ptr & 0x3f);
                }
            } while (j-- && fprintf(stderr, "/"));
            fprintf(stderr, "\n");
        }

        if (rs->inst[i] & 0x10000) {
            ip = (rs->inst[i] >> 12) & 0xf;
            fprintf(stderr, "color: ip %d to psf %d\n",
                ip, (rs->inst[i] >> 18) & 0x7f);

            col_ptr = (rs->ip[ip] >> 24) & 0x7;
            col_fmt = (rs->ip[ip] >> 27) & 0xf;
            fprintf(stderr, "     : offset %d ", col_ptr);

            switch (col_fmt) {
                case 0:
                    fprintf(stderr, "(R/G/B/A)");
                    break;
                case 1:
                    fprintf(stderr, "(R/G/B/0)");
                    break;
                case 2:
                    fprintf(stderr, "(R/G/B/1)");
                    break;
                case 4:
                    fprintf(stderr, "(0/0/0/A)");
                    break;
                case 5:
                    fprintf(stderr, "(0/0/0/0)");
                    break;
                case 6:
                    fprintf(stderr, "(0/0/0/1)");
                    break;
                case 8:
                    fprintf(stderr, "(1/1/1/A)");
                    break;
                case 9:
                    fprintf(stderr, "(1/1/1/0)");
                    break;
                case 10:
                    fprintf(stderr, "(1/1/1/1)");
                    break;
            }
            fprintf(stderr, "\n");
        }
    }
}
