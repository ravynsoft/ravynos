/*
 * Copyright © 2020 Intel Corporation
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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */
#ifndef VK_DEFERRED_OPERATION_H
#define VK_DEFERRED_OPERATION_H

#include "vk_object.h"

#include "c11/threads.h"
#include "util/list.h"

#ifdef __cplusplus
extern "C" {
#endif

struct vk_deferred_operation {
   struct vk_object_base base;
};

VK_DEFINE_NONDISP_HANDLE_CASTS(vk_deferred_operation, base,
                               VkDeferredOperationKHR,
                               VK_OBJECT_TYPE_DEFERRED_OPERATION_KHR)

#ifdef __cplusplus
}
#endif

#endif /* VK_DEFERRED_OPERATION_H */
