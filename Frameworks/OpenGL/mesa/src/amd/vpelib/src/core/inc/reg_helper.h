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

#ifndef DRIVERS_VPELIB_INC_REG_HELPER_H_
#define DRIVERS_VPELIB_INC_REG_HELPER_H_

#pragma once

#include <stdint.h>
#include "vpe_command.h"
#include "config_writer.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct reg_id_val {
    const uint32_t id;
    const uint32_t default_value;
    uint32_t       lastWritten_value;
    bool           isWritten;
} reg_id_val;

/** CTX is defined in the .c files */
#define PROGRAM_ENTRY()                                                                            \
    struct vpe_priv                 *vpe_priv      = CTX_BASE->vpe_priv;                           \
    struct CTX                      *CTX           = (struct CTX *)CTX_BASE;                       \
    struct config_writer            *config_writer = &vpe_priv->config_writer;                     \
    struct vpep_direct_config_packet packet        = {0}

// for use with reg_id_val struct that stores id, default and current val together
#define REG_OFFSET(reg_name)           CTX->regs->reg_name.id // Register offset in DWORD
#define REG_DEFAULT(reg_name)          CTX->regs->reg_name.default_value
#define REG_IS_WRITTEN(reg_name)       CTX->regs->reg_name.isWritten
#define REG_LAST_WRITTEN_VAL(reg_name) CTX->regs->reg_name.lastWritten_value
#define REG_CURRENT(reg_name)                                                                      \
    (REG_IS_WRITTEN(reg_name) ? REG_LAST_WRITTEN_VAL(reg_name) : REG_DEFAULT(reg_name))

#define REG_FIELD_VALUE(field, value) ((uint32_t)((value) << CTX->shift->field) & CTX->mask->field)
#define REG_FIELD_SHIFT(field)        CTX->shift->field
#define REG_FIELD_MASK(field)         CTX->mask->field
#define VPEC_FIELD_VALUE(field, data) ((uint32_t)((data) << field##__SHIFT) & field##_MASK)

/* macro to set register fields. */
#define REG_SET_DEFAULT(reg_name)                                                                  \
    do {                                                                                           \
        packet.bits.INC                         = 0;                                               \
        packet.bits.VPEP_CONFIG_DATA_SIZE       = 0;                                               \
        packet.bits.VPEP_CONFIG_REGISTER_OFFSET = REG_OFFSET(reg_name);                            \
        REG_IS_WRITTEN(reg_name)                = true;                                            \
        packet.data[0] = REG_LAST_WRITTEN_VAL(reg_name) = REG_DEFAULT(reg_name);                   \
        config_writer_fill_direct_config_packet(config_writer, &packet);                           \
    } while (0)

#define REG_SET(reg_name, init_val, field, val)                                                    \
    do {                                                                                           \
        packet.bits.INC                         = 0;                                               \
        packet.bits.VPEP_CONFIG_DATA_SIZE       = 0;                                               \
        packet.bits.VPEP_CONFIG_REGISTER_OFFSET = REG_OFFSET(reg_name);                            \
        REG_IS_WRITTEN(reg_name)                = true;                                            \
        packet.data[0]                          = REG_LAST_WRITTEN_VAL(reg_name) =                 \
            (((uint32_t)init_val & ~(REG_FIELD_MASK(field))) |                                     \
                REG_FIELD_VALUE(field, (uint32_t)val));                                            \
        config_writer_fill_direct_config_packet(config_writer, &packet);                           \
    } while (0)

#define REG_SET_2(reg_name, init_val, f1, v1, f2, v2)                                              \
    do {                                                                                           \
        packet.bits.INC                         = 0;                                               \
        packet.bits.VPEP_CONFIG_DATA_SIZE       = 0;                                               \
        packet.bits.VPEP_CONFIG_REGISTER_OFFSET = REG_OFFSET(reg_name);                            \
        REG_IS_WRITTEN(reg_name)                = true;                                            \
        packet.data[0]                          = REG_LAST_WRITTEN_VAL(reg_name) =                 \
            (((uint32_t)init_val & ~(REG_FIELD_MASK(f1)) & ~(REG_FIELD_MASK(f2))) |                \
                REG_FIELD_VALUE(f1, (uint32_t)v1) | REG_FIELD_VALUE(f2, (uint32_t)v2));            \
        config_writer_fill_direct_config_packet(config_writer, &packet);                           \
    } while (0)

#define REG_SET_3(reg_name, init_val, f1, v1, f2, v2, f3, v3)                                      \
    do {                                                                                           \
        packet.bits.INC                         = 0;                                               \
        packet.bits.VPEP_CONFIG_DATA_SIZE       = 0;                                               \
        packet.bits.VPEP_CONFIG_REGISTER_OFFSET = REG_OFFSET(reg_name);                            \
        REG_IS_WRITTEN(reg_name)                = true;                                            \
        packet.data[0]                          = REG_LAST_WRITTEN_VAL(reg_name) =                 \
            (((uint32_t)init_val & ~(REG_FIELD_MASK(f1)) & ~(REG_FIELD_MASK(f2)) &                 \
                 ~(REG_FIELD_MASK(f3))) |                                                          \
                REG_FIELD_VALUE(f1, (uint32_t)v1) | REG_FIELD_VALUE(f2, (uint32_t)v2) |            \
                REG_FIELD_VALUE(f3, (uint32_t)v3));                                                \
        config_writer_fill_direct_config_packet(config_writer, &packet);                           \
    } while (0)

#define REG_SET_4(reg_name, init_val, f1, v1, f2, v2, f3, v3, f4, v4)                              \
    do {                                                                                           \
        packet.bits.INC                         = 0;                                               \
        packet.bits.VPEP_CONFIG_DATA_SIZE       = 0;                                               \
        packet.bits.VPEP_CONFIG_REGISTER_OFFSET = REG_OFFSET(reg_name);                            \
        REG_IS_WRITTEN(reg_name)                = true;                                            \
        packet.data[0]                          = REG_LAST_WRITTEN_VAL(reg_name) =                 \
            (((uint32_t)init_val & ~(REG_FIELD_MASK(f1)) & ~(REG_FIELD_MASK(f2)) &                 \
                 ~(REG_FIELD_MASK(f3)) & ~(REG_FIELD_MASK(f4))) |                                  \
                REG_FIELD_VALUE(f1, (uint32_t)v1) | REG_FIELD_VALUE(f2, (uint32_t)v2) |            \
                REG_FIELD_VALUE(f3, (uint32_t)v3) | REG_FIELD_VALUE(f4, (uint32_t)v4));            \
        config_writer_fill_direct_config_packet(config_writer, &packet);                           \
    } while (0)

#define REG_SET_5(reg_name, init_val, f1, v1, f2, v2, f3, v3, f4, v4, f5, v5)                      \
    do {                                                                                           \
        packet.bits.INC                         = 0;                                               \
        packet.bits.VPEP_CONFIG_DATA_SIZE       = 0;                                               \
        packet.bits.VPEP_CONFIG_REGISTER_OFFSET = REG_OFFSET(reg_name);                            \
        REG_IS_WRITTEN(reg_name)                = true;                                            \
        packet.data[0]                          = REG_LAST_WRITTEN_VAL(reg_name) =                 \
            (((uint32_t)init_val & ~(REG_FIELD_MASK(f1)) & ~(REG_FIELD_MASK(f2)) &                 \
                 ~(REG_FIELD_MASK(f3)) & ~(REG_FIELD_MASK(f4)) & ~(REG_FIELD_MASK(f5))) |          \
                REG_FIELD_VALUE(f1, (uint32_t)v1) | REG_FIELD_VALUE(f2, (uint32_t)v2) |            \
                REG_FIELD_VALUE(f3, (uint32_t)v3) | REG_FIELD_VALUE(f4, (uint32_t)v4) |            \
                REG_FIELD_VALUE(f5, (uint32_t)v5));                                                \
        config_writer_fill_direct_config_packet(config_writer, &packet);                           \
    } while (0)

#define REG_SET_6(reg_name, init_val, f1, v1, f2, v2, f3, v3, f4, v4, f5, v5, f6, v6)              \
    do {                                                                                           \
        packet.bits.INC                         = 0;                                               \
        packet.bits.VPEP_CONFIG_DATA_SIZE       = 0;                                               \
        packet.bits.VPEP_CONFIG_REGISTER_OFFSET = REG_OFFSET(reg_name);                            \
        REG_IS_WRITTEN(reg_name)                = true;                                            \
        packet.data[0]                          = REG_LAST_WRITTEN_VAL(reg_name) =                 \
            (((uint32_t)init_val & ~(REG_FIELD_MASK(f1)) & ~(REG_FIELD_MASK(f2)) &                 \
                 ~(REG_FIELD_MASK(f3)) & ~(REG_FIELD_MASK(f4)) & ~(REG_FIELD_MASK(f5)) &           \
                 ~(REG_FIELD_MASK(f6))) |                                                          \
                REG_FIELD_VALUE(f1, (uint32_t)v1) | REG_FIELD_VALUE(f2, (uint32_t)v2) |            \
                REG_FIELD_VALUE(f3, (uint32_t)v3) | REG_FIELD_VALUE(f4, (uint32_t)v4) |            \
                REG_FIELD_VALUE(f5, (uint32_t)v5) | REG_FIELD_VALUE(f6, (uint32_t)v6));            \
        config_writer_fill_direct_config_packet(config_writer, &packet);                           \
    } while (0)

#define REG_SET_7(reg_name, init_val, f1, v1, f2, v2, f3, v3, f4, v4, f5, v5, f6, v6, f7, v7)      \
    do {                                                                                           \
        packet.bits.INC                         = 0;                                               \
        packet.bits.VPEP_CONFIG_DATA_SIZE       = 0;                                               \
        packet.bits.VPEP_CONFIG_REGISTER_OFFSET = REG_OFFSET(reg_name);                            \
        REG_IS_WRITTEN(reg_name)                = true;                                            \
        packet.data[0]                          = REG_LAST_WRITTEN_VAL(reg_name) =                 \
            (((uint32_t)init_val & ~(REG_FIELD_MASK(f1)) & ~(REG_FIELD_MASK(f2)) &                 \
                 ~(REG_FIELD_MASK(f3)) & ~(REG_FIELD_MASK(f4)) & ~(REG_FIELD_MASK(f5)) &           \
                 ~(REG_FIELD_MASK(f6)) & ~(REG_FIELD_MASK(f7))) |                                  \
                REG_FIELD_VALUE(f1, (uint32_t)v1) | REG_FIELD_VALUE(f2, (uint32_t)v2) |            \
                REG_FIELD_VALUE(f3, (uint32_t)v3) | REG_FIELD_VALUE(f4, (uint32_t)v4) |            \
                REG_FIELD_VALUE(f5, (uint32_t)v5) | REG_FIELD_VALUE(f6, (uint32_t)v6) |            \
                REG_FIELD_VALUE(f7, (uint32_t)v7));                                                \
        config_writer_fill_direct_config_packet(config_writer, &packet);                           \
    } while (0)

#define REG_SET_8(                                                                                 \
    reg_name, init_val, f1, v1, f2, v2, f3, v3, f4, v4, f5, v5, f6, v6, f7, v7, f8, v8)            \
    do {                                                                                           \
        packet.bits.INC                         = 0;                                               \
        packet.bits.VPEP_CONFIG_DATA_SIZE       = 0;                                               \
        packet.bits.VPEP_CONFIG_REGISTER_OFFSET = REG_OFFSET(reg_name);                            \
        REG_IS_WRITTEN(reg_name)                = true;                                            \
        packet.data[0]                          = REG_LAST_WRITTEN_VAL(reg_name) =                 \
            (((uint32_t)init_val & ~(REG_FIELD_MASK(f1)) & ~(REG_FIELD_MASK(f2)) &                 \
                 ~(REG_FIELD_MASK(f3)) & ~(REG_FIELD_MASK(f4)) & ~(REG_FIELD_MASK(f5)) &           \
                 ~(REG_FIELD_MASK(f6)) & ~(REG_FIELD_MASK(f7)) & ~(REG_FIELD_MASK(f8))) |          \
                REG_FIELD_VALUE(f1, (uint32_t)v1) | REG_FIELD_VALUE(f2, (uint32_t)v2) |            \
                REG_FIELD_VALUE(f3, (uint32_t)v3) | REG_FIELD_VALUE(f4, (uint32_t)v4) |            \
                REG_FIELD_VALUE(f5, (uint32_t)v5) | REG_FIELD_VALUE(f6, (uint32_t)v6) |            \
                REG_FIELD_VALUE(f7, (uint32_t)v7) | REG_FIELD_VALUE(f8, (uint32_t)v8));            \
        config_writer_fill_direct_config_packet(config_writer, &packet);                           \
    } while (0)

#define REG_SET_9(                                                                                 \
    reg_name, init_val, f1, v1, f2, v2, f3, v3, f4, v4, f5, v5, f6, v6, f7, v7, f8, v8, f9, v9)    \
    do {                                                                                           \
        packet.bits.INC                         = 0;                                               \
        packet.bits.VPEP_CONFIG_DATA_SIZE       = 0;                                               \
        packet.bits.VPEP_CONFIG_REGISTER_OFFSET = REG_OFFSET(reg_name);                            \
        REG_IS_WRITTEN(reg_name)                = true;                                            \
        packet.data[0]                          = REG_LAST_WRITTEN_VAL(reg_name) =                 \
            (((uint32_t)init_val & ~(REG_FIELD_MASK(f1)) & ~(REG_FIELD_MASK(f2)) &                 \
                 ~(REG_FIELD_MASK(f3)) & ~(REG_FIELD_MASK(f4)) & ~(REG_FIELD_MASK(f5)) &           \
                 ~(REG_FIELD_MASK(f6)) & ~(REG_FIELD_MASK(f7)) & ~(REG_FIELD_MASK(f8)) &           \
                 ~(REG_FIELD_MASK(f9))) |                                                          \
                REG_FIELD_VALUE(f1, (uint32_t)v1) | REG_FIELD_VALUE(f2, (uint32_t)v2) |            \
                REG_FIELD_VALUE(f3, (uint32_t)v3) | REG_FIELD_VALUE(f4, (uint32_t)v4) |            \
                REG_FIELD_VALUE(f5, (uint32_t)v5) | REG_FIELD_VALUE(f6, (uint32_t)v6) |            \
                REG_FIELD_VALUE(f7, (uint32_t)v7) | REG_FIELD_VALUE(f8, (uint32_t)v8) |            \
                REG_FIELD_VALUE(f9, (uint32_t)v9));                                                \
        config_writer_fill_direct_config_packet(config_writer, &packet);                           \
    } while (0)

#define REG_SET_10(reg_name, init_val, f1, v1, f2, v2, f3, v3, f4, v4, f5, v5, f6, v6, f7, v7, f8, \
    v8, f9, v9, f10, v10)                                                                          \
    do {                                                                                           \
        packet.bits.INC                         = 0;                                               \
        packet.bits.VPEP_CONFIG_DATA_SIZE       = 0;                                               \
        packet.bits.VPEP_CONFIG_REGISTER_OFFSET = REG_OFFSET(reg_name);                            \
        REG_IS_WRITTEN(reg_name)                = true;                                            \
        packet.data[0]                          = REG_LAST_WRITTEN_VAL(reg_name) =                 \
            (((uint32_t)init_val & ~(REG_FIELD_MASK(f1)) & ~(REG_FIELD_MASK(f2)) &                 \
                 ~(REG_FIELD_MASK(f3)) & ~(REG_FIELD_MASK(f4)) & ~(REG_FIELD_MASK(f5)) &           \
                 ~(REG_FIELD_MASK(f6)) & ~(REG_FIELD_MASK(f7)) & ~(REG_FIELD_MASK(f8)) &           \
                 ~(REG_FIELD_MASK(f9)) & ~(REG_FIELD_MASK(f10))) |                                 \
                REG_FIELD_VALUE(f1, (uint32_t)v1) | REG_FIELD_VALUE(f2, (uint32_t)v2) |            \
                REG_FIELD_VALUE(f3, (uint32_t)v3) | REG_FIELD_VALUE(f4, (uint32_t)v4) |            \
                REG_FIELD_VALUE(f5, (uint32_t)v5) | REG_FIELD_VALUE(f6, (uint32_t)v6) |            \
                REG_FIELD_VALUE(f7, (uint32_t)v7) | REG_FIELD_VALUE(f8, (uint32_t)v8) |            \
                REG_FIELD_VALUE(f9, (uint32_t)v9) | REG_FIELD_VALUE(f10, (uint32_t)v10));          \
        config_writer_fill_direct_config_packet(config_writer, &packet);                           \
    } while (0)

#define REG_UPDATE(reg, field, val)       REG_SET(reg, REG_CURRENT(reg), field, val)
#define REG_UPDATE_2(reg, f1, v1, f2, v2) REG_SET_2(reg, REG_CURRENT(reg), f1, v1, f2, v2)
#define REG_UPDATE_3(reg, f1, v1, f2, v2, f3, v3)                                                  \
    REG_SET_3(reg, REG_CURRENT(reg), f1, v1, f2, v2, f3, v3)
#define REG_UPDATE_4(reg, f1, v1, f2, v2, f3, v3, f4, v4)                                          \
    REG_SET_4(reg, REG_CURRENT(reg), f1, v1, f2, v2, f3, v3, f4, v4)
#define REG_UPDATE_5(reg, f1, v1, f2, v2, f3, v3, f4, v4, f5, v5)                                  \
    REG_SET_5(reg, REG_CURRENT(reg), f1, v1, f2, v2, f3, v3, f4, v4, f5, v5)

#define REG_UPDATE_6(reg, f1, v1, f2, v2, f3, v3, f4, v4, f5, v5, f6, v6)                          \
    REG_SET_6(reg, REG_CURRENT(reg), f1, v1, f2, v2, f3, v3, f4, v4, f5, v5, f6, v6)

#define REG_UPDATE_7(reg, f1, v1, f2, v2, f3, v3, f4, v4, f5, v5, f6, v6, f7, v7)                  \
    REG_SET_7(reg, REG_CURRENT(reg), f1, v1, f2, v2, f3, v3, f4, v4, f5, v5, f6, v6, f7, v7)

#define REG_UPDATE_8(reg, f1, v1, f2, v2, f3, v3, f4, v4, f5, v5, f6, v6, f7, v7, f8, v8)          \
    REG_SET_8(reg, REG_CURRENT(reg), f1, v1, f2, v2, f3, v3, f4, v4, f5, v5, f6, v6, f7, v7, f8, v8)

#define REG_UPDATE_9(reg, f1, v1, f2, v2, f3, v3, f4, v4, f5, v5, f6, v6, f7, v7, f8, v8, f9, v9)  \
    REG_SET_9(reg, REG_CURRENT(reg), f1, v1, f2, v2, f3, v3, f4, v4, f5, v5, f6, v6, f7, v7, f8,   \
        v8, f9, v9)

#define REG_UPDATE_10(                                                                             \
    reg, f1, v1, f2, v2, f3, v3, f4, v4, f5, v5, f6, v6, f7, v7, f8, v8, f9, v9, f10, v10)         \
    REG_SET_10(reg, REG_CURRENT(reg), f1, v1, f2, v2, f3, v3, f4, v4, f5, v5, f6, v6, f7, v7, f8,  \
        v8, f9, v9, f10, v10)

#define REG_SET_DEFAULT(reg_name)                                                                  \
    do {                                                                                           \
        packet.bits.INC                         = 0;                                               \
        packet.bits.VPEP_CONFIG_DATA_SIZE       = 0;                                               \
        packet.bits.VPEP_CONFIG_REGISTER_OFFSET = REG_OFFSET(reg_name);                            \
        REG_IS_WRITTEN(reg_name)                = true;                                            \
        packet.data[0] = REG_LAST_WRITTEN_VAL(reg_name) = REG_DEFAULT(reg_name);                   \
        config_writer_fill_direct_config_packet(config_writer, &packet);                           \
    } while (0)

#ifdef __cplusplus
}
#endif

#endif /* DRIVERS_VPELIB_INC_REG_HELPER_H_ */
