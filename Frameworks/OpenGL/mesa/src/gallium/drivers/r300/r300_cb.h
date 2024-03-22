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

/**
 * This file contains macros for building command buffers in memory.
 *
 * Use NEW_CB for buffers with a varying size and it will also allocate
 * the buffer.
 * Use BEGIN_CB for arrays with a static size.
 *
 * Example:
 *
 *     uint32_t cb[3];
 *     CB_LOCALS;
 *
 *     BEGIN_CB(cb, 3);
 *     OUT_CB_REG_SEQ(R500_RB3D_CONSTANT_COLOR_AR, 2);
 *     OUT_CB(blend_color_red_alpha);
 *     OUT_CB(blend_color_green_blue);
 *     END_CB;
 *
 * And later:
 *
 *     CS_LOCALS;
 *     WRITE_CS_TABLE(cb, 3);
 *
 * Or using a little slower variant:
 *
 *     CS_LOCALS;
 *     BEGIN_CS(cb, 3);
 *     OUT_CS_TABLE(cb, 3);
 *     END_CS;
 */

#ifndef R300_CB_H
#define R300_CB_H

#include "r300_reg.h"

/* Yes, I know macros are ugly. However, they are much prettier than the code
 * that they neatly hide away, and don't have the cost of function setup, so
 * we're going to use them. */

/**
 * Command buffer setup.
 */

#ifdef DEBUG

#define CB_LOCALS \
    int cs_count = 0; \
    uint32_t *cs_ptr = NULL; \
    (void) cs_count; (void) cs_ptr

#define BEGIN_CB(ptr, size) do { \
    assert(sizeof(*(ptr)) == sizeof(uint32_t)); \
    cs_count = (size); \
    cs_ptr = (ptr); \
} while (0)

#define NEW_CB(ptr, size) \
    do { \
    assert(sizeof(*(ptr)) == sizeof(uint32_t)); \
    cs_count = (size); \
    cs_ptr = (ptr) = malloc((size) * sizeof(uint32_t)); \
} while (0)

#define END_CB do { \
    if (cs_count != 0) \
        debug_printf("r300: Warning: cs_count off by %d at (%s, %s:%i)\n", \
                     cs_count, __func__, __FILE__, __LINE__); \
} while (0)

#define CB_USED_DW(x) cs_count -= x

#else

#define CB_LOCALS \
    uint32_t *cs_ptr = NULL; (void) cs_ptr

#define NEW_CB(ptr, size) \
    cs_ptr = (ptr) = malloc((size) * sizeof(uint32_t))

#define BEGIN_CB(ptr, size) cs_ptr = (ptr)
#define END_CB
#define CB_USED_DW(x)

#endif


/**
 * Storing pure DWORDs.
 */

#define OUT_CB(value) do { \
    *cs_ptr = (value); \
    cs_ptr++; \
    CB_USED_DW(1); \
} while (0)

#define OUT_CB_TABLE(values, count) do { \
    memcpy(cs_ptr, values, count * sizeof(uint32_t)); \
    cs_ptr += count; \
    CB_USED_DW(count); \
} while (0)

#define OUT_CB_32F(value) \
    OUT_CB(fui(value));

#define OUT_CB_REG(register, value) do { \
    assert(register); \
    OUT_CB(CP_PACKET0(register, 0)); \
    OUT_CB(value); \
} while (0)

/* Note: This expects count to be the number of registers,
 * not the actual packet0 count! */
#define OUT_CB_REG_SEQ(register, count) do { \
    assert(register); \
    OUT_CB(CP_PACKET0(register, (count) - 1)); \
} while (0)

#define OUT_CB_ONE_REG(register, count) do { \
    assert(register); \
    OUT_CB(CP_PACKET0(register, (count) - 1) | RADEON_ONE_REG_WR); \
} while (0)

#define OUT_CB_PKT3(op, count) \
    OUT_CB(CP_PACKET3(op, count))

#endif /* R300_CB_H */
