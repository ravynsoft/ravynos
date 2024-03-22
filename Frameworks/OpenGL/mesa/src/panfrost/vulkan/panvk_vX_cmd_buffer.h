/*
 * Copyright (C) 2021 Collabora Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#ifndef PANVK_PRIVATE_H
#error "Must be included from panvk_private.h"
#endif

#ifndef PAN_ARCH
#error "no arch"
#endif

#include "compiler/shader_enums.h"
#include <vulkan/vulkan.h>

extern const struct vk_command_buffer_ops panvk_per_arch(cmd_buffer_ops);

void panvk_per_arch(cmd_close_batch)(struct panvk_cmd_buffer *cmdbuf);

void panvk_per_arch(cmd_get_tiler_context)(struct panvk_cmd_buffer *cmdbuf,
                                           unsigned width, unsigned height);

void panvk_per_arch(cmd_alloc_fb_desc)(struct panvk_cmd_buffer *cmdbuf);

void panvk_per_arch(cmd_alloc_tls_desc)(struct panvk_cmd_buffer *cmdbuf,
                                        bool gfx);

void panvk_per_arch(cmd_prepare_tiler_context)(struct panvk_cmd_buffer *cmdbuf);
