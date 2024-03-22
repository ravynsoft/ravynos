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

#include "vpe_assert.h"
#include "vpe_command.h"
#include "plane_desc_writer.h"
#include "reg_helper.h"

void plane_desc_writer_init(struct plane_desc_writer *writer, struct vpe_buf *buf, int32_t nps0,
    int32_t npd0, int32_t nps1, int32_t npd1, int32_t subop)
{
    uint32_t *cmd_space;
    uint64_t  size      = 4;
    writer->status      = VPE_STATUS_OK;
    writer->base_cpu_va = buf->cpu_va;
    writer->base_gpu_va = buf->gpu_va;
    writer->buf         = buf;
    writer->num_src     = 0;
    writer->num_dst     = 0;

    /* Buffer does not have enough space to write */
    if (buf->size < (int64_t)size) {
        writer->status = VPE_STATUS_BUFFER_OVERFLOW;
        return;
    }

    cmd_space    = (uint32_t *)(uintptr_t)writer->buf->cpu_va;
    *cmd_space++ = VPE_PLANE_CFG_CMD_HEADER(subop, nps0, npd0, nps1, npd1);

    writer->buf->cpu_va += size;
    writer->buf->gpu_va += size;
    writer->buf->size -= size;
}

/** fill the value to the embedded buffer. */
void plane_desc_writer_add_source(
    struct plane_desc_writer *writer, struct plane_desc_src *src, bool is_plane0)
{
    uint32_t *cmd_space, *cmd_start;
    uint32_t  num_wd = is_plane0 ? 6 : 5;
    uint64_t  size   = num_wd * sizeof(uint32_t);

    if (writer->status != VPE_STATUS_OK)
        return;

    /* Buffer does not have enough space to write */
    if (writer->buf->size < (int64_t)size) {
        writer->status = VPE_STATUS_BUFFER_OVERFLOW;
        return;
    }
    cmd_start = cmd_space = (uint32_t *)(uintptr_t)writer->buf->cpu_va;

    if (is_plane0) {
        *cmd_space++ = VPEC_FIELD_VALUE(VPE_PLANE_CFG_TMZ, src->tmz) |
                       VPEC_FIELD_VALUE(VPE_PLANE_CFG_SWIZZLE_MODE, src->swizzle) |
                       VPEC_FIELD_VALUE(VPE_PLANE_CFG_ROTATION, src->rotation);
        writer->num_src++;
    }

    VPE_ASSERT(!(src->base_addr_lo & 0xFF));

    *cmd_space++ = src->base_addr_lo;
    *cmd_space++ = src->base_addr_hi;

    *cmd_space++ =
        VPEC_FIELD_VALUE(VPE_PLANE_CFG_PITCH, src->pitch - 1); // 1-based number of element
    *cmd_space++ = VPEC_FIELD_VALUE(VPE_PLANE_CFG_VIEWPORT_X, src->viewport_x) |
                   VPEC_FIELD_VALUE(VPE_PLANE_CFG_VIEWPORT_Y, src->viewport_y);

    *cmd_space++ = VPEC_FIELD_VALUE(VPE_PLANE_CFG_VIEWPORT_WIDTH, src->viewport_w - 1) |
                   VPEC_FIELD_VALUE(VPE_PLANE_CFG_VIEWPORT_HEIGHT, src->viewport_h - 1) |
                   VPEC_FIELD_VALUE(VPE_PLANE_CFG_VIEWPORT_ELEMENT_SIZE, src->elem_size);

    writer->buf->cpu_va += size;
    writer->buf->gpu_va += size;
    writer->buf->size -= size;
}

/** fill the value to the embedded buffer. */
void plane_desc_writer_add_destination(
    struct plane_desc_writer *writer, struct plane_desc_dst *dst, bool is_plane0)
{
    uint32_t *cmd_space, *cmd_start;
    uint32_t  num_wd = is_plane0 ? 6 : 5;
    uint64_t  size   = num_wd * sizeof(uint32_t);

    if (writer->status != VPE_STATUS_OK)
        return;

    /* Buffer does not have enough space to write */
    if (writer->buf->size < (int64_t)size) {
        writer->status = VPE_STATUS_BUFFER_OVERFLOW;
        return;
    }

    cmd_start = cmd_space = (uint32_t *)(uintptr_t)writer->buf->cpu_va;

    if (is_plane0) {
        *cmd_space++ = VPEC_FIELD_VALUE(VPE_PLANE_CFG_TMZ, dst->tmz) |
                       VPEC_FIELD_VALUE(VPE_PLANE_CFG_SWIZZLE_MODE, dst->swizzle) |
                       VPEC_FIELD_VALUE(VPE_PLANE_CFG_MIRROR, dst->mirror);
        writer->num_dst++;
    }

    VPE_ASSERT(!(dst->base_addr_lo & 0xFF));

    *cmd_space++ = dst->base_addr_lo;
    *cmd_space++ = dst->base_addr_hi;

    *cmd_space++ =
        VPEC_FIELD_VALUE(VPE_PLANE_CFG_PITCH, dst->pitch - 1); // 1-based number of element
    *cmd_space++ = VPEC_FIELD_VALUE(VPE_PLANE_CFG_VIEWPORT_X, dst->viewport_x) |
                   VPEC_FIELD_VALUE(VPE_PLANE_CFG_VIEWPORT_Y, dst->viewport_y);

    *cmd_space++ = VPEC_FIELD_VALUE(VPE_PLANE_CFG_VIEWPORT_WIDTH, dst->viewport_w - 1) |
                   VPEC_FIELD_VALUE(VPE_PLANE_CFG_VIEWPORT_HEIGHT, dst->viewport_h - 1) |
                   VPEC_FIELD_VALUE(VPE_PLANE_CFG_VIEWPORT_ELEMENT_SIZE, dst->elem_size);

    writer->buf->cpu_va += size;
    writer->buf->gpu_va += size;
    writer->buf->size -= size;
}
