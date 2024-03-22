/*
 * Copyright (C) 2018-2019 Lima Project
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
 */

#ifndef H_LIMA_UTIL
#define H_LIMA_UTIL

#include <stdint.h>
#include <stdbool.h>

#define LIMA_PAGE_SIZE 4096

struct lima_dump;

struct lima_dump *lima_dump_create(void);
struct lima_dump *lima_dump_next(struct lima_dump *dump);
void lima_dump_free(struct lima_dump *dump);

void lima_dump_shader(struct lima_dump *dump, void *data, int size, bool is_frag);
void lima_dump_vs_command_stream_print(struct lima_dump *dump, void *data,
                                       int size, uint32_t start);
void lima_dump_plbu_command_stream_print(struct lima_dump *dump, void *data,
                                         int size, uint32_t start);
void lima_dump_rsw_command_stream_print(struct lima_dump *dump, void *data,
                                        int size, uint32_t start);
void lima_dump_texture_descriptor(struct lima_dump *dump, void *data,
                                  int size, uint32_t start, uint32_t offset);

void _lima_dump_command_stream_print(struct lima_dump *dump, void *data,
                                     int size, bool is_float, const char *fmt, ...);
#define lima_dump_command_stream_print(dump, ...) \
   do { \
      if (dump) \
         _lima_dump_command_stream_print(dump, __VA_ARGS__); \
   } while (0)

struct pipe_scissor_state;

void lima_damage_rect_union(struct pipe_scissor_state *rect,
                            unsigned minx, unsigned maxx,
                            unsigned miny, unsigned maxy);
#endif
