/*
 * Copyright 2009 Marek Olšák <maraeo@gmail.com>
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

#ifndef R300_SHADER_SEMANTICS_H
#define R300_SHADER_SEMANTICS_H

#define ATTR_UNUSED             (-1)
#define ATTR_COLOR_COUNT        2
#define ATTR_GENERIC_COUNT      32
#define ATTR_TEXCOORD_COUNT     8

/* This structure contains information about what attributes are written by VS
 * or read by FS. (but not both) It's much easier to work with than
 * tgsi_shader_info.
 *
 * The variables contain indices to tgsi_shader_info semantics and those
 * indices are nothing else than input/output register numbers. */
struct r300_shader_semantics {
    int pos;
    int psize;
    int color[ATTR_COLOR_COUNT];
    int bcolor[ATTR_COLOR_COUNT];
    int face;
    int texcoord[ATTR_TEXCOORD_COUNT];
    int generic[ATTR_GENERIC_COUNT];
    int fog;
    int wpos;
    int pcoord;

    int num_texcoord;
    int num_generic;
};

static inline void r300_shader_semantics_reset(
    struct r300_shader_semantics* info)
{
    int i;

    info->pos = ATTR_UNUSED;
    info->psize = ATTR_UNUSED;
    info->face = ATTR_UNUSED;
    info->fog = ATTR_UNUSED;
    info->wpos = ATTR_UNUSED;
    info->pcoord = ATTR_UNUSED;

    for (i = 0; i < ATTR_COLOR_COUNT; i++) {
        info->color[i] = ATTR_UNUSED;
        info->bcolor[i] = ATTR_UNUSED;
    }

    for (i = 0; i < ATTR_TEXCOORD_COUNT; i++) {
        info->texcoord[i] = ATTR_UNUSED;
    }

    for (i = 0; i < ATTR_GENERIC_COUNT; i++) {
        info->generic[i] = ATTR_UNUSED;
    }

    info->num_texcoord = 0;
    info->num_generic = 0;
}

#endif
