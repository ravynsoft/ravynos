/* Copyright 2022 Advanced Micro Devices, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE COPYRIGHT HOLDER(S) OR AUTHOR(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * Authors: AMD
 *
 */

#pragma once

#include "vpe_types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct vpe_desc_writer {
    struct vpe_buf *buf; /**< store the current buf pointer */

    /* store the base addr of the currnet config
     * i.e. config header
     * it is always constructed in emb_buf
     */
    uint64_t base_gpu_va;
    uint64_t base_cpu_va;

    uint32_t        num_config_desc;
    bool            plane_desc_added;
    enum vpe_status status;
};

/** initialize the vpe descriptor writer.
 * Calls right before building any vpe descriptor
 *
 * /param   writer      writer instance
 * /param   buf         points to the current buf,
 *                      each config_writer_fill will update the address
 * /param   cd          count down of slice in a frame
 */
void vpe_desc_writer_init(struct vpe_desc_writer *writer, struct vpe_buf *buf, int cd);

/** fill the value to the command buffer. */
void vpe_desc_writer_add_plane_desc(
    struct vpe_desc_writer *writer, uint64_t plane_desc_addr, bool tmz);

/** fill the value to the command buffer. */
void vpe_desc_writer_add_config_desc(
    struct vpe_desc_writer *writer, uint64_t config_desc_addr, bool reuse, bool tmz);

void vpe_desc_writer_complete(struct vpe_desc_writer *writer);

#ifdef __cplusplus
}
#endif
