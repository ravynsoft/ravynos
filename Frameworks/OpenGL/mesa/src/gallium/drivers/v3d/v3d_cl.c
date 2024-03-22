/*
 * Copyright © 2014-2017 Broadcom
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

#include "util/u_math.h"
#include "util/ralloc.h"
#include "v3d_context.h"
/* We don't expect that the packets we use in this file change across across
 * hw versions, so we just explicitly set the V3D_VERSION and include
 * v3dx_pack here
 */
#define V3D_VERSION 42
#include "broadcom/common/v3d_macros.h"
#include "broadcom/cle/v3dx_pack.h"

void
v3d_init_cl(struct v3d_job *job, struct v3d_cl *cl)
{
        cl->base = NULL;
        cl->next = cl->base;
        cl->size = 0;
        cl->job = job;
}

uint32_t
v3d_cl_ensure_space(struct v3d_cl *cl, uint32_t space, uint32_t alignment)
{
        uint32_t offset = align(cl_offset(cl), alignment);

        if (offset + space <= cl->size) {
                cl->next = cl->base + offset;
                return offset;
        }

        v3d_bo_unreference(&cl->bo);
        cl->bo = v3d_bo_alloc(cl->job->v3d->screen, align(space, 4096), "CL");
        cl->base = v3d_bo_map(cl->bo);
        cl->size = cl->bo->size;
        cl->next = cl->base;

        return 0;
}

void
v3d_cl_ensure_space_with_branch(struct v3d_cl *cl, uint32_t space)
{
        if (cl_offset(cl) + space + cl_packet_length(BRANCH) <= cl->size)
                return;

        struct v3d_bo *new_bo = v3d_bo_alloc(cl->job->v3d->screen, space, "CL");
        assert(space <= new_bo->size);

        /* Chain to the new BO from the old one. */
        if (cl->bo) {
                cl_emit(cl, BRANCH, branch) {
                        branch.address = cl_address(new_bo, 0);
                }
                v3d_bo_unreference(&cl->bo);
        } else {
                /* Root the first RCL/BCL BO in the job. */
                v3d_job_add_bo(cl->job, new_bo);
        }

        cl->bo = new_bo;
        cl->base = v3d_bo_map(cl->bo);
        cl->size = cl->bo->size;
        cl->next = cl->base;
}

void
v3d_destroy_cl(struct v3d_cl *cl)
{
        v3d_bo_unreference(&cl->bo);
}
