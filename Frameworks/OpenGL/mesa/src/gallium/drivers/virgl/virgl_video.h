/*
 * Copyright 2022 Kylin Software Co., Ltd.
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
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/**
 * @file
 * Virgl video driver interface.
 *
 * This file defines two objects:
 *   virgl_video_buffer: Buffer for storing raw YUV formatted data.
 *   virgl_video_codec : Represents an encoder or decoder.
 *
 * @author Feng Jiang <jiangfeng@kylinos.cn>
 */

#ifndef VIRGL_VIDEO_H
#define VIRGL_VIDEO_H


#include "virgl_context.h"
#include "vl/vl_video_buffer.h"
#include "pipe/p_video_codec.h"
#include "virtio-gpu/virgl_video_hw.h"

#define VIRGL_VIDEO_CODEC_BUF_NUM    10

struct virgl_video_codec {
    struct pipe_video_codec base;       /* must be first */

    uint32_t handle;
    struct virgl_context *vctx;
    union virgl_picture_desc desc;

    uint32_t bs_size;                   /* size of data in bs_buffer */
    uint32_t cur_buffer;                /* index of current bs/desc buffer */
    struct pipe_resource *bs_buffers[VIRGL_VIDEO_CODEC_BUF_NUM];
    struct pipe_resource *desc_buffers[VIRGL_VIDEO_CODEC_BUF_NUM];
    struct pipe_resource *feed_buffers[VIRGL_VIDEO_CODEC_BUF_NUM];
};

struct virgl_video_buffer {
    uint32_t handle;
    enum pipe_format buffer_format;
    unsigned width;
    unsigned height;
    struct virgl_context *vctx;
    struct pipe_video_buffer *buf;
    unsigned num_planes;
    struct pipe_sampler_view **plane_views;
};

static inline struct virgl_video_codec *
virgl_video_codec(struct pipe_video_codec *codec)
{
    return (struct virgl_video_codec *)codec;
}

static inline struct virgl_video_buffer *
virgl_video_buffer(struct pipe_video_buffer *buffer)
{
    return buffer ? vl_video_buffer_get_associated_data(buffer, NULL) : NULL;
}

struct pipe_video_codec *
virgl_video_create_codec(struct pipe_context *ctx,
                         const struct pipe_video_codec *templ);

struct pipe_video_buffer *
virgl_video_create_buffer(struct pipe_context *ctx,
                          const struct pipe_video_buffer *tmpl);

#endif /* VIRGL_VIDEO_H */

