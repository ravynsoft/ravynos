/*
 * Copyright 2022 Advanced Micro Devices, Inc.
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
 */

#ifndef DRIVERS_VPELIB_INC_DIAG_REG_HELPER_H_
#define DRIVERS_VPELIB_INC_DIAG_REG_HELPER_H_

#pragma once

#include "reg_helper.h"

#ifdef __cplusplus
extern "C" {
#endif

/** CTX is defined in the .c files */
#define DIAG_PROGRAM_ENTRY()                                                                       \
    struct vpe_priv                 *vpe_priv      = CTX_BASE->vpe_priv;                           \
    struct CTX                      *CTX           = (struct CTX *)CTX_BASE;                       \
    struct config_writer            *config_writer = &vpe_priv->config_diag_writer;                \
    struct vpep_direct_config_packet packet        = {0}

#define DIAG_REG_SET(reg_name, val)                                                                \
    do {                                                                                           \
        packet.bits.INC                         = 0;                                               \
        packet.bits.VPEP_CONFIG_DATA_SIZE       = 0;                                               \
        packet.bits.VPEP_CONFIG_REGISTER_OFFSET = REG_OFFSET(reg_name);                            \
        REG_IS_WRITTEN(reg_name)                = true;                                            \
        packet.data[0] = REG_LAST_WRITTEN_VAL(reg_name) = (uint32_t)val;                           \
        config_writer_fill_direct_config_packet(config_writer, &packet);                           \
    } while (0)

#define DIAG_REG_CURRENT(reg_name)       REG_CURRENT(reg_name)
#define DIAG_REG_UPDATE(reg, field, val) REG_UPDATE(reg, field, val)

#ifdef __cplusplus
}
#endif

#endif /* DRIVERS_VPELIB_INC_REG_HELPER_H_ */
