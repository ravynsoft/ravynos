/*
 * Copyright 2019 Google LLC
 * SPDX-License-Identifier: MIT
 *
 * based in part on anv and radv which are:
 * Copyright © 2015 Intel Corporation
 * Copyright © 2016 Red Hat.
 * Copyright © 2016 Bas Nieuwenhuizen
 */

#ifndef VN_PIPELINE_H
#define VN_PIPELINE_H

#include "vn_common.h"

struct vn_shader_module {
   struct vn_object_base base;
};
VK_DEFINE_NONDISP_HANDLE_CASTS(vn_shader_module,
                               base.base,
                               VkShaderModule,
                               VK_OBJECT_TYPE_SHADER_MODULE)

struct vn_pipeline_layout {
   struct vn_object_base base;

   struct vn_descriptor_set_layout *push_descriptor_set_layout;
   bool has_push_constant_ranges;
   struct vn_refcount refcount;
};
VK_DEFINE_NONDISP_HANDLE_CASTS(vn_pipeline_layout,
                               base.base,
                               VkPipelineLayout,
                               VK_OBJECT_TYPE_PIPELINE_LAYOUT)

struct vn_pipeline_cache {
   struct vn_object_base base;
};
VK_DEFINE_NONDISP_HANDLE_CASTS(vn_pipeline_cache,
                               base.base,
                               VkPipelineCache,
                               VK_OBJECT_TYPE_PIPELINE_CACHE)

enum vn_pipeline_type {
   VN_PIPELINE_TYPE_GRAPHICS,
   VN_PIPELINE_TYPE_COMPUTE,
};

struct vn_pipeline {
   struct vn_object_base base;
   enum vn_pipeline_type type;

   /**
    * The VkPipelineLayout provided directly (without linking) at pipeline
    * creation. Null if none was provided.
    *
    * We track the pipeline layout here to extend its and its children's
    * lifetime, NOT because this is the actual layout used by the pipeline.
    *
    * WARNING. This may not be the actual layout used by the pipeline. The
    * Vulkan 1.3.254 spec says:
    *
    *    The final effective pipeline layout is effectively the union of the
    *    linked pipeline layouts. When binding descriptor sets for this
    *    pipeline, the pipeline layout used must be compatible with this
    *    union.
    */
   struct vn_pipeline_layout *layout;
};
VK_DEFINE_NONDISP_HANDLE_CASTS(vn_pipeline,
                               base.base,
                               VkPipeline,
                               VK_OBJECT_TYPE_PIPELINE)

#endif /* VN_PIPELINE_H */
