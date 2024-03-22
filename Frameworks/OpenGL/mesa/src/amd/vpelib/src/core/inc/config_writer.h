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

#if defined(LITTLEENDIAN_CPU)
#elif defined(BIGENDIAN_CPU)
#else
#error "BIGENDIAN_CPU or LITTLEENDIAN_CPU must be defined"
#endif

#ifdef __cplusplus
extern "C" {
#endif

enum config_type {
    CONFIG_TYPE_UNKNOWN,
    CONFIG_TYPE_DIRECT,
    CONFIG_TYPE_INDIRECT
};

typedef void (*config_callback_t)(
    void *ctx, uint64_t cfg_base_gpu, uint64_t cfg_base_cpu, int64_t size);

#define MAX_CONFIG_PACKET_DATA_SIZE_DWORD 0x01000

struct vpep_direct_config_packet {
    union {
        struct {
#if defined(LITTLEENDIAN_CPU)
            uint32_t INC                         : 1;
            uint32_t                             : 1;
            uint32_t VPEP_CONFIG_REGISTER_OFFSET : 18;
            uint32_t VPEP_CONFIG_DATA_SIZE       : 12;
#elif defined(BIGENDIAN_CPU)
            uint32_t VPEP_CONFIG_DATA_SIZE       : 12;
            uint32_t VPEP_CONFIG_REGISTER_OFFSET : 18;
            uint32_t                             : 1;
            uint32_t INC                         : 1;
#endif
        } bitfields, bits;
        uint32_t u32all;
    };
    uint32_t data[1];
};

/* config writer only help initialize the 1st DWORD,
 * and 'close' the config (i.e. finalize the size) once it is completed.
 * it doesn't help generate the content, which shall be prepared by the caller
 * and then call config_writer_fill()
 */
struct config_writer {
    struct vpe_buf *buf; /**< store the current buf pointer */

    /* store the base addr of the currnet config
     * i.e. config header
     * it is always constructed in emb_buf
     */
    uint64_t base_gpu_va;
    uint64_t base_cpu_va;

    enum config_type type;
    bool             completed;

    void             *callback_ctx;
    config_callback_t callback;
    enum vpe_status   status;
};

/** initialize the config writer.
 * Calls right before building any VPEP configs
 *
 * /param   writer      writer instance
 * /param   emb_buf     points to the current cmd_buf,
 *                      each config_writer_fill will update the address
 */
void config_writer_init(struct config_writer *writer, struct vpe_buf *emb_buf);

/** set the callback function (can be null) for notifying any config completion
 * In the callback, caller can:
 * 1. save the config for later reuse
 * 2. write it to vpe descriptor
 */
void config_writer_set_callback(
    struct config_writer *writer, void *callback_ctx, config_callback_t callback);

/** set the config type before config_writer_fill()
 * if the config_type has changed, it will finalize the current one,
 *  1) direct config
 *      VPEP_DIRECT_CONFIG_ARRAY_SIZE is finalized (in DW0) automatically.
 *  2) indirect config
 *      NUM_DST is finalized (in DW0) automatically.
 * and run callback (if set) to notify the completion.
 * A new config desc header DW0 will be generated.
 *
 * /param   writer      writer instance
 * /param   type        config type
 */
void config_writer_set_type(struct config_writer *writer, enum config_type type);

/** fill the value to the buffer.
 * If the dword exceeds the config packet size limit,
 * callback will be called and a new config desc is created.
 *
 * /param   writer      writer instance
 * /param   value       fill the DW to the config desc body
 */
void config_writer_fill(struct config_writer *writer, uint32_t value);

/** fill the header value to the buffer.
 * If the current size + number of dwords in the array
 * exceeds the config packet size limit,
 * callback will be called and a new config desc is created.
 *
 * /param   writer      writer instance
 * /param   packet      config packet with header filled properly
 */
void config_writer_fill_direct_config_packet_header(
    struct config_writer *writer, struct vpep_direct_config_packet *packet);

/** fill the header and data value to the buffer.
 * For single DATA element ONLY.
 * If the current size + number of dwords in the array
 * exceeds the config packet size limit,
 * callback will be called and a new config desc is created.
 *
 * /param   writer      writer instance
 * /param   packet      config packet with valid header and data
 */
void config_writer_fill_direct_config_packet(
    struct config_writer *writer, struct vpep_direct_config_packet *packet);

void config_writer_fill_indirect_data_array(
    struct config_writer *writer, const uint64_t data_gpuva, uint32_t size);

void config_writer_fill_indirect_destination(struct config_writer *writer,
    const uint32_t offset_index, const uint32_t start_index, const uint32_t offset_data);

/** explicitly complete the config */
void config_writer_complete(struct config_writer *writer);

#ifdef __cplusplus
}
#endif
