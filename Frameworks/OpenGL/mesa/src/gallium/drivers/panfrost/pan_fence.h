/*
 * Copyright (c) 2022 Amazon.com, Inc. or its affiliates.
 * Copyright 2018-2019 Alyssa Rosenzweig
 * Copyright 2018-2019 Collabora, Ltd.
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
 */

#include "pipe/p_state.h"

struct panfrost_context;

struct pipe_fence_handle {
   struct pipe_reference reference;
   uint32_t syncobj;
   bool signaled;
};

void panfrost_fence_reference(struct pipe_screen *pscreen,
                              struct pipe_fence_handle **ptr,
                              struct pipe_fence_handle *fence);

bool panfrost_fence_finish(struct pipe_screen *pscreen,
                           struct pipe_context *ctx,
                           struct pipe_fence_handle *fence, uint64_t timeout);

int panfrost_fence_get_fd(struct pipe_screen *screen,
                          struct pipe_fence_handle *f);

struct pipe_fence_handle *panfrost_fence_from_fd(struct panfrost_context *ctx,
                                                 int fd,
                                                 enum pipe_fd_type type);

struct pipe_fence_handle *panfrost_fence_create(struct panfrost_context *ctx);
