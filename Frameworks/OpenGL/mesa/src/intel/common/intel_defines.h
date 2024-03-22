/*
 * Copyright © 2018 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE COPYRIGHT OWNER(S) AND/OR ITS SUPPLIERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef INTEL_DEFINES_H
#define INTEL_DEFINES_H

#include "drm-uapi/i915_drm.h"

#ifdef __cplusplus
extern "C" {
#endif
/**
 * \file gen_defines.h
 *
 * Common defines we want to share between GL And Vulkan.
 */

#define INTEL_CONTEXT_LOW_PRIORITY ((I915_CONTEXT_MIN_USER_PRIORITY-1)/2)
#define INTEL_CONTEXT_MEDIUM_PRIORITY (I915_CONTEXT_DEFAULT_PRIORITY)
#define INTEL_CONTEXT_HIGH_PRIORITY ((I915_CONTEXT_MAX_USER_PRIORITY+1)/2)
/* We don't have a strict notion of RT (yet, and when we do it is likely
 * to be more complicated than a mere priority value!), but we can give
 * it the absolute most priority available to us. By convention, this
 * is higher than any other client, except for blocked interactive
 * clients.
 */
#define INTEL_CONTEXT_REALTIME_PRIORITY I915_CONTEXT_MAX_USER_PRIORITY

#ifdef __cplusplus
}
#endif

#endif /* INTEL_DEFINES_H */
