/*
 * Copyright © 2023 Intel Corporation
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
#include "xe/iris_bufmgr.h"

#include "common/intel_gem.h"
#include "iris/iris_bufmgr.h"

#include "drm-uapi/xe_drm.h"

bool
iris_xe_init_global_vm(struct iris_bufmgr *bufmgr, uint32_t *vm_id)
{
   struct drm_xe_vm_create create = {
      .flags = DRM_XE_VM_CREATE_FLAG_SCRATCH_PAGE,
   };
   if (intel_ioctl(iris_bufmgr_get_fd(bufmgr), DRM_IOCTL_XE_VM_CREATE, &create))
      return false;

   *vm_id = create.vm_id;
   return true;
}

bool
iris_xe_destroy_global_vm(struct iris_bufmgr *bufmgr)
{
   struct drm_xe_vm_destroy destroy = {
      .vm_id = iris_bufmgr_get_global_vm_id(bufmgr),
   };
   return intel_ioctl(iris_bufmgr_get_fd(bufmgr), DRM_IOCTL_XE_VM_DESTROY,
                      &destroy) == 0;
}
