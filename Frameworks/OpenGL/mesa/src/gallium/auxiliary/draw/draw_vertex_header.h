/**************************************************************************
 *
 * Copyright 2007 VMware, Inc.
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
 * IN NO EVENT SHALL VMWARE AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 **************************************************************************/

#ifndef DRAW_VERTEX_HEADER_H
#define DRAW_VERTEX_HEADER_H

/** Sum of frustum planes and user-defined planes */
#define DRAW_TOTAL_CLIP_PLANES (6 + PIPE_MAX_CLIP_PLANES)

/**
 * Basic vertex info.  Used to represent vertices after VS (through GS, TESS,
 * etc.) to vbuf output.
 */
struct vertex_header {
   unsigned clipmask:DRAW_TOTAL_CLIP_PLANES;
   unsigned edgeflag:1;
   unsigned pad:1;
   unsigned vertex_id:16;

   float clip_pos[4];
   float data[][4]; // the vertex attributes
};

#endif
