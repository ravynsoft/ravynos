/*
 * Copyright 2011 Joakim Sindholt <opensource@zhasha.com>
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

#ifndef _NINE_DEFINES_H_
#define _NINE_DEFINES_H_

#include "pipe/p_defines.h"


#define NINE_RESOURCE_FLAG_LOCKABLE (PIPE_RESOURCE_FLAG_FRONTEND_PRIV << 1)
#define NINE_RESOURCE_FLAG_DUMMY    (PIPE_RESOURCE_FLAG_FRONTEND_PRIV << 2)

/* vertexdeclaration9.c */
uint16_t nine_d3d9_to_nine_declusage(unsigned usage, unsigned index);

#define NINE_DECLUSAGE_POSITION         0
#define NINE_DECLUSAGE_BLENDWEIGHT      1
#define NINE_DECLUSAGE_BLENDINDICES     2
#define NINE_DECLUSAGE_NORMAL           3
#define NINE_DECLUSAGE_TEXCOORD         4
#define NINE_DECLUSAGE_TANGENT          5
#define NINE_DECLUSAGE_BINORMAL         6
#define NINE_DECLUSAGE_COLOR            7
#define NINE_DECLUSAGE_POSITIONT        8

#define NINE_DECLUSAGE_PSIZE            9
#define NINE_DECLUSAGE_TESSFACTOR       10
#define NINE_DECLUSAGE_DEPTH            11
#define NINE_DECLUSAGE_FOG              12
#define NINE_DECLUSAGE_SAMPLE           13
#define NINE_DECLUSAGE_NONE             14
#define NINE_DECLUSAGE_COUNT            (NINE_DECLUSAGE_NONE + 1)

#define NINE_DECLUSAGE_i(declusage, n) NINE_DECLUSAGE_##declusage + n * NINE_DECLUSAGE_COUNT

#define NINED3DCLEAR_DEPTHSTENCIL   (D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL)

#define NINE_BIND_BACKBUFFER_FLAGS    (PIPE_BIND_RENDER_TARGET |\
                                       PIPE_BIND_SAMPLER_VIEW)

#define NINE_BIND_PRESENTBUFFER_FLAGS (PIPE_BIND_RENDER_TARGET |\
                                       PIPE_BIND_DISPLAY_TARGET |\
                                       PIPE_BIND_SCANOUT |\
                                       PIPE_BIND_SHARED)

#endif /* _NINE_DEFINES_H_ */
