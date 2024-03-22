/*
 * Copyright © 2016 Intel Corporation
 * Copyright © 2017 Broadcom
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

#ifndef V3D_DECODER_H
#define V3D_DECODER_H

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#include "broadcom/common/v3d_device_info.h"

struct v3d_spec;
struct v3d_group;
struct v3d_field;
struct clif_dump;

struct v3d_group *v3d_spec_find_struct(struct v3d_spec *spec, const char *name);
struct v3d_spec *v3d_spec_load(const struct v3d_device_info *devinfo);
struct v3d_group *v3d_spec_find_instruction(struct v3d_spec *spec, const uint8_t *p);
struct v3d_group *v3d_spec_find_register(struct v3d_spec *spec, uint32_t offset);
struct v3d_group *v3d_spec_find_register_by_name(struct v3d_spec *spec, const char *name);
int v3d_group_get_length(struct v3d_group *group);
const char *v3d_group_get_name(struct v3d_group *group);
uint8_t v3d_group_get_opcode(struct v3d_group *group);
struct v3d_enum *v3d_spec_find_enum(struct v3d_spec *spec, const char *name);

struct v3d_field_iterator {
        struct v3d_group *group;
        char name[128];
        char value[128];
        struct v3d_group *struct_desc;
        const uint8_t *p;
        int offset; /**< current field starts at &p[offset] */

        int field_iter;
        int group_iter;

        struct v3d_field *field;
};

struct v3d_group {
        struct v3d_spec *spec;
        char *name;

        struct v3d_field **fields;
        uint32_t nfields;
        uint32_t fields_size;

        uint32_t group_offset, group_count;
        uint32_t group_size;
        bool variable;

        struct v3d_group *parent;
        struct v3d_group *next;

        uint8_t opcode;

        /* Register specific */
        uint32_t register_offset;
};

struct v3d_value {
        char *name;
        uint64_t value;
};

struct v3d_enum {
        char *name;
        int nvalues;
        struct v3d_value **values;
};

struct v3d_type {
        enum {
                V3D_TYPE_UNKNOWN,
                V3D_TYPE_INT,
                V3D_TYPE_UINT,
                V3D_TYPE_BOOL,
                V3D_TYPE_FLOAT,
                V3D_TYPE_F187,
                V3D_TYPE_ADDRESS,
                V3D_TYPE_OFFSET,
                V3D_TYPE_STRUCT,
                V3D_TYPE_UFIXED,
                V3D_TYPE_SFIXED,
                V3D_TYPE_MBO,
                V3D_TYPE_ENUM
        } kind;

        /* Struct definition for V3D_TYPE_STRUCT */
        union {
                struct v3d_group *v3d_struct;
                struct v3d_enum *v3d_enum;
                struct {
                        /* Integer and fractional sizes for V3D_TYPE_UFIXED and
                         * V3D_TYPE_SFIXED
                         */
                        int i, f;
                };
        };
};

struct v3d_field {
        char *name;
        int start, end;
        struct v3d_type type;
        bool minus_one;
        bool has_default;
        uint32_t default_value;

        struct v3d_enum inline_enum;
};

void v3d_field_iterator_init(struct v3d_field_iterator *iter,
                             struct v3d_group *group,
                             const uint8_t *p);

bool v3d_field_iterator_next(struct clif_dump *clif,
                             struct v3d_field_iterator *iter);

void v3d_print_group(struct clif_dump *clif,
                     struct v3d_group *group,
                     uint64_t offset, const uint8_t *p);

#endif /* V3D_DECODER_H */
