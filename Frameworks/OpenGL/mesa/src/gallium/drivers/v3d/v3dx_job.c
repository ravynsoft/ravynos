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

/** @file v3dx_job.c
 *
 * V3D version-specific functions for submitting V3D render jobs to the
 * kernel.
 */

#include "v3d_context.h"
#include "broadcom/cle/v3dx_pack.h"

void v3dX(bcl_epilogue)(struct v3d_context *v3d, struct v3d_job *job)
{
                v3d_cl_ensure_space_with_branch(&job->bcl,
                                                cl_packet_length(PRIMITIVE_COUNTS_FEEDBACK) +
                                                cl_packet_length(TRANSFORM_FEEDBACK_SPECS) +
                                                cl_packet_length(FLUSH));

                if (job->tf_enabled || job->needs_primitives_generated) {
                        /* Write primitive counts to memory. */
                        assert(v3d->prim_counts);
                        struct v3d_resource *rsc =
                                v3d_resource(v3d->prim_counts);
                        cl_emit(&job->bcl, PRIMITIVE_COUNTS_FEEDBACK, counter) {
                                counter.address =
                                        cl_address(rsc->bo,
                                                   v3d->prim_counts_offset);
                                counter.read_write_64byte = false;
                                counter.op = 0;
                        }
                }

                /* Disable TF at the end of the CL, so that the TF block
                 * cleans up and finishes before it gets reset by the next
                 * frame's tile binning mode cfg packet. (SWVC5-718).
                 */
                if (job->tf_enabled) {
                        cl_emit(&job->bcl, TRANSFORM_FEEDBACK_SPECS, tfe) {
                                tfe.enable = false;
                        };
                }

                /* We just FLUSH here to tell the HW to cap the bin CLs with a
                 * return.  Any remaining state changes won't be flushed to
                 * the bins first -- you would need FLUSH_ALL for that, but
                 * the HW for hasn't been validated
                 */
                cl_emit(&job->bcl, FLUSH, flush);
}
