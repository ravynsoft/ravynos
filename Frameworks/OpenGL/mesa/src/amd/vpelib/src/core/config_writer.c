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
#include "config_writer.h"
#include "reg_helper.h"
#include "common.h"

// in bytes
#define MAX_DIRECT_CONFIG_SIZE   (4 * 0x10000)
#define MAX_INDIRECT_CONFIG_SIZE ((4 + 16 * 3) * sizeof(uint32_t))

void config_writer_init(struct config_writer *writer, struct vpe_buf *buf)
{
    writer->base_cpu_va  = buf->cpu_va;
    writer->base_gpu_va  = buf->gpu_va;
    writer->buf          = buf;
    writer->type         = CONFIG_TYPE_UNKNOWN;
    writer->callback_ctx = NULL;
    writer->callback     = NULL;
    writer->completed    = false;
    writer->status       = VPE_STATUS_OK;
}

void config_writer_set_callback(
    struct config_writer *writer, void *callback_ctx, config_callback_t callback)
{
    writer->callback_ctx = callback_ctx;
    writer->callback     = callback;
}

static inline void config_writer_new(struct config_writer *writer)
{
    if (writer->status != VPE_STATUS_OK)
        return;

    /* Buffer does not have enough space to write */
    if (writer->buf->size < (int64_t)sizeof(uint32_t)) {
        writer->status = VPE_STATUS_BUFFER_OVERFLOW;
        return;
    }
    // new base
    writer->base_cpu_va = writer->buf->cpu_va;
    writer->base_gpu_va = writer->buf->gpu_va;

    // new header. don't need to fill it yet until completion
    writer->buf->cpu_va += sizeof(uint32_t);
    writer->buf->gpu_va += sizeof(uint32_t);
    writer->buf->size -= sizeof(uint32_t);
    writer->completed = false;
}

void config_writer_set_type(struct config_writer *writer, enum config_type type)
{
    VPE_ASSERT(type != CONFIG_TYPE_UNKNOWN);

    if (writer->status != VPE_STATUS_OK)
        return;

    if (writer->type != type) {
        if (writer->type == CONFIG_TYPE_UNKNOWN) {
            // new header. don't need to fill it yet until completion
            config_writer_new(writer);
        } else {
            // a new config type, close the previous one
            config_writer_complete(writer);

            config_writer_new(writer);
        }
        writer->type = type;
    }
}

void config_writer_fill(struct config_writer *writer, uint32_t value)
{
    uint32_t *cmd_space;
    uint64_t  size = writer->buf->cpu_va - writer->base_cpu_va;

    VPE_ASSERT(writer->type != CONFIG_TYPE_UNKNOWN);

    if (writer->status != VPE_STATUS_OK)
        return;

    // check overflow, open a new one if it is
    if (writer->type == CONFIG_TYPE_DIRECT) {
        if (size >= MAX_DIRECT_CONFIG_SIZE) {
            config_writer_complete(writer);
            config_writer_new(writer);
        } else if (writer->completed) {
            config_writer_new(writer);
        }
    } else {
        if (size >= MAX_INDIRECT_CONFIG_SIZE) {
            config_writer_complete(writer);
            config_writer_new(writer);
        } else if (writer->completed) {
            config_writer_new(writer);
        }
    }

    /* Buffer does not have enough space to write */
    if (writer->buf->size < (int64_t)sizeof(uint32_t)) {
        writer->status = VPE_STATUS_BUFFER_OVERFLOW;
        return;
    }

    cmd_space    = (uint32_t *)(uintptr_t)writer->buf->cpu_va;
    *cmd_space++ = value;
    writer->buf->cpu_va += sizeof(uint32_t);
    writer->buf->gpu_va += sizeof(uint32_t);
    writer->buf->size -= sizeof(uint32_t);
}

void config_writer_fill_direct_config_packet_header(
    struct config_writer *writer, struct vpep_direct_config_packet *packet)
{
    uint32_t *cmd_space;
    uint64_t  size = writer->buf->cpu_va - writer->base_cpu_va;

    VPE_ASSERT(writer->type == CONFIG_TYPE_DIRECT);

    if (writer->status != VPE_STATUS_OK)
        return;

    // first + 1 for header, DATA_SIZE + 1 for real data size
    // for estimate overflow, this function only write packet header
    if (size + (1 + packet->bits.VPEP_CONFIG_DATA_SIZE + 1) * sizeof(uint32_t) >=
        MAX_DIRECT_CONFIG_SIZE) {
        config_writer_complete(writer);
        config_writer_new(writer);
    } else if (writer->completed) {
        config_writer_new(writer);
    }

    /* Buffer does not have enough space to write */
    if (writer->buf->size < (int64_t)sizeof(uint32_t)) {
        writer->status = VPE_STATUS_BUFFER_OVERFLOW;
        return;
    }

    cmd_space    = (uint32_t *)(uintptr_t)writer->buf->cpu_va;
    *cmd_space++ = packet->u32all;
    writer->buf->cpu_va += sizeof(uint32_t);
    writer->buf->gpu_va += sizeof(uint32_t);
    writer->buf->size -= sizeof(uint32_t);
}

void config_writer_fill_direct_config_packet(
    struct config_writer *writer, struct vpep_direct_config_packet *packet)
{
    uint32_t *cmd_space;
    uint64_t  size = writer->buf->cpu_va - writer->base_cpu_va;

    VPE_ASSERT(writer->type == CONFIG_TYPE_DIRECT);
    VPE_ASSERT(packet->bits.VPEP_CONFIG_DATA_SIZE == 0);
    if (writer->status != VPE_STATUS_OK)
        return;

    // first + 1 for header, DATA_SIZE + 1 for real data size
    // this function writes both header and the data
    if (size + 1 + (packet->bits.VPEP_CONFIG_DATA_SIZE + 1) * sizeof(uint32_t) >=
        MAX_DIRECT_CONFIG_SIZE) {
        config_writer_complete(writer);
        config_writer_new(writer);
    } else if (writer->completed) {
        config_writer_new(writer);
    }

    if (writer->buf->size < (int64_t)(2 * sizeof(uint32_t))) {
        writer->status = VPE_STATUS_BUFFER_OVERFLOW;
        return;
    }

    cmd_space    = (uint32_t *)(uintptr_t)writer->buf->cpu_va;
    *cmd_space++ = packet->u32all; // Write header
    writer->buf->cpu_va += sizeof(uint32_t);
    writer->buf->gpu_va += sizeof(uint32_t);
    writer->buf->size -= sizeof(uint32_t);
    *cmd_space++ = packet->data[0]; // Write data
    writer->buf->cpu_va += sizeof(uint32_t);
    writer->buf->gpu_va += sizeof(uint32_t);
    writer->buf->size -= sizeof(uint32_t);
}

void config_writer_fill_indirect_data_array(
    struct config_writer *writer, const uint64_t data_gpuva, uint32_t size)
{
    VPE_ASSERT(writer->type == CONFIG_TYPE_INDIRECT);
    VPE_ASSERT(size > 0);

    // the DATA_ARRAY_SIZE is 1-based, hence -1 from actual size
    config_writer_fill(writer, VPEC_FIELD_VALUE(VPE_IND_CFG_DATA_ARRAY_SIZE, size - 1));
    config_writer_fill(writer, ADDR_LO(data_gpuva));
    config_writer_fill(writer, ADDR_HI(data_gpuva));
}

void config_writer_fill_indirect_destination(struct config_writer *writer,
    const uint32_t offset_index, const uint32_t start_index, const uint32_t offset_data)
{
    VPE_ASSERT(writer->type == CONFIG_TYPE_INDIRECT);
    config_writer_fill(writer, VPEC_FIELD_VALUE(VPE_IND_CFG_PKT_REGISTER_OFFSET, offset_index));
    config_writer_fill(writer, start_index);
    config_writer_fill(writer, VPEC_FIELD_VALUE(VPE_IND_CFG_PKT_REGISTER_OFFSET, offset_data));
}

void config_writer_complete(struct config_writer *writer)
{
    uint32_t *cmd_space = (uint32_t *)(uintptr_t)writer->base_cpu_va;
    uint32_t  size      = (uint32_t)(writer->buf->cpu_va - writer->base_cpu_va);

    if (writer->status != VPE_STATUS_OK)
        return;

    VPE_ASSERT(writer->type != CONFIG_TYPE_UNKNOWN);
    VPE_ASSERT(writer->buf->cpu_va != writer->base_cpu_va);

    if (writer->type == CONFIG_TYPE_DIRECT) {
        // -4 for exclude header
        // VPEP_DIRECT_CONFIG_ARRAY_SIZE is 1-based, hence need -1
        *cmd_space = VPE_DIR_CFG_CMD_HEADER(((size - 4) / sizeof(uint32_t) - 1));
    } else {
        // -4 DW for header, data array size, data array lo and data array hi
        // /3 DW for each destination reg
        // NUM_DST is 1-based, hence need -1
        uint32_t num_dst = (uint32_t)((size - (4 * sizeof(uint32_t))) / (3 * sizeof(uint32_t)) - 1);
        *cmd_space       = VPE_IND_CFG_CMD_HEADER(num_dst);
    }

    writer->completed = true;

    if (writer->callback) {
        writer->callback(writer->callback_ctx, writer->base_gpu_va, writer->base_cpu_va, size);
    }
}
