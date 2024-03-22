/*
 * Copyright 2021 Advanced Micro Devices, Inc.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef AC_MSGPACK_H
#define AC_MSGPACK_H

struct ac_msgpack {
   uint8_t *mem;
   uint32_t mem_size;
   uint32_t offset;
};

void ac_msgpack_init(struct ac_msgpack *msgpack);
void ac_msgpack_destroy(struct ac_msgpack *msgpack);
int ac_msgpack_resize_if_required(struct ac_msgpack *msgpack,
                                  uint32_t data_size);
void ac_msgpack_add_fixmap_op(struct ac_msgpack *msgpack, uint32_t n);
void ac_msgpack_add_fixarray_op(struct ac_msgpack *msgpack, uint32_t n);
void ac_msgpack_add_fixstr(struct ac_msgpack *msgpack, const char *str);
void ac_msgpack_add_uint(struct ac_msgpack *msgpack, uint64_t val);
void ac_msgpack_add_int(struct ac_msgpack *msgpack, int64_t val);

#endif
