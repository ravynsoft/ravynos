/*
 * Copyright 2011-2013 Maarten Lankhorst
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
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#include "nvc0/nvc0_context.h"
#include "nvc0/nvc0_screen.h"
#include "nouveau_vp3_video.h"

#include "vl/vl_decoder.h"
#include "vl/vl_types.h"

#include "util/u_video.h"

extern unsigned
nvc0_decoder_bsp_begin(struct nouveau_vp3_decoder *dec, unsigned comm_seq);

extern unsigned
nvc0_decoder_bsp_next(struct nouveau_vp3_decoder *dec,
                      unsigned comm_seq, unsigned num_buffers,
                      const void *const *data, const unsigned *num_bytes);

extern unsigned
nvc0_decoder_bsp_end(struct nouveau_vp3_decoder *dec, union pipe_desc desc,
                     struct nouveau_vp3_video_buffer *target,
                     unsigned comm_seq, unsigned *vp_caps, unsigned *is_ref,
                     struct nouveau_vp3_video_buffer *refs[16]);

extern void
nvc0_decoder_vp(struct nouveau_vp3_decoder *dec, union pipe_desc desc,
                struct nouveau_vp3_video_buffer *target, unsigned comm_seq,
                unsigned caps, unsigned is_ref,
                struct nouveau_vp3_video_buffer *refs[16]);

extern void
nvc0_decoder_ppp(struct nouveau_vp3_decoder *dec, union pipe_desc desc,
                 struct nouveau_vp3_video_buffer *target, unsigned comm_seq);
