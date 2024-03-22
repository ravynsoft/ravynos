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

#ifndef R300_SCREEN_H
#define R300_SCREEN_H

#include "r300_chipset.h"
#include "winsys/radeon_winsys.h"
#include "pipe/p_screen.h"
#include "util/disk_cache.h"
#include "util/slab.h"
#include "util/u_thread.h"
#include <stdio.h>

struct r300_screen {
    /* Parent class */
    struct pipe_screen screen;

    struct radeon_winsys *rws;

    /* Chipset info and capabilities. */
    struct radeon_info info;
    struct r300_capabilities caps;

    /** Combination of DBG_xxx flags */
    unsigned debug;

    struct disk_cache *disk_shader_cache;

    struct slab_parent_pool pool_transfers;

    /* The MSAA texture with CMASK access; */
    struct pipe_resource *cmask_resource;
    mtx_t cmask_mutex;
};


/* Convenience cast wrappers. */
static inline struct r300_screen* r300_screen(struct pipe_screen* screen) {
    return (struct r300_screen*)screen;
}

static inline struct radeon_winsys *
radeon_winsys(struct pipe_screen *screen) {
    return r300_screen(screen)->rws;
}

/* Debug functionality. */

/**
 * Debug flags to disable/enable certain groups of debugging outputs.
 *
 * \note These may be rather coarse, and the grouping may be impractical.
 * If you find, while debugging the driver, that a different grouping
 * of these flags would be beneficial, just feel free to change them
 * but make sure to update the documentation in r300_debug.c to reflect
 * those changes.
 */
/*@{*/

/* Logging. */
#define DBG_PSC         (1 << 0)
#define DBG_FP          (1 << 1)
#define DBG_VP          (1 << 2)
#define DBG_SWTCL       (1 << 3)
#define DBG_DRAW        (1 << 4)
#define DBG_TEX         (1 << 5)
#define DBG_TEXALLOC    (1 << 6)
#define DBG_RS          (1 << 7)
#define DBG_FB          (1 << 8)
#define DBG_RS_BLOCK    (1 << 9)
#define DBG_CBZB        (1 << 10)
#define DBG_HYPERZ      (1 << 11)
#define DBG_SCISSOR     (1 << 12)
#define DBG_INFO        (1 << 13)
#define DBG_MSAA        (1 << 14)
/* Features. */
#define DBG_ANISOHQ     (1 << 16)
#define DBG_NO_TILING   (1 << 17)
#define DBG_NO_IMMD     (1 << 18)
#define DBG_NO_OPT      (1 << 19)
#define DBG_NO_CBZB     (1 << 20)
#define DBG_NO_ZMASK    (1 << 21)
#define DBG_NO_HIZ      (1 << 22)
#define DBG_NO_CMASK    (1 << 23)
#define DBG_NO_TCL      (1 << 25)
/*@}*/
static inline bool SCREEN_DBG_ON(struct r300_screen * screen, unsigned flags)
{
    return (screen->debug & flags) ? true : false;
}

static inline void SCREEN_DBG(struct r300_screen * screen, unsigned flags,
                              const char * fmt, ...)
{
    if (SCREEN_DBG_ON(screen, flags)) {
        va_list va;
        va_start(va, fmt);
        vfprintf(stderr, fmt, va);
        va_end(va);
    }
}

void r300_init_debug(struct r300_screen* ctx);

void r300_init_screen_resource_functions(struct r300_screen *r300screen);

#endif /* R300_SCREEN_H */
