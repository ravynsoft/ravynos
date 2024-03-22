/*
 * Copyright © 2022 Intel Corporation
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

#pragma once

#include <stdint.h>

#include "intel/dev/intel_kmd.h"

enum intel_engine_class {
   INTEL_ENGINE_CLASS_RENDER = 0,
   INTEL_ENGINE_CLASS_COPY,
   INTEL_ENGINE_CLASS_VIDEO,
   INTEL_ENGINE_CLASS_VIDEO_ENHANCE,
   INTEL_ENGINE_CLASS_COMPUTE,
   INTEL_ENGINE_CLASS_INVALID
};

struct intel_engine_class_instance {
   enum intel_engine_class engine_class;
   uint16_t engine_instance;
   uint16_t gt_id;
};

struct intel_query_engine_info {
   uint32_t num_engines;
   struct intel_engine_class_instance engines[];
};

struct intel_query_engine_info *
intel_engine_get_info(int fd, enum intel_kmd_type type);
int intel_engines_count(const struct intel_query_engine_info *info,
                        enum intel_engine_class engine_class);
const char *intel_engines_class_to_string(enum intel_engine_class engine_class);
