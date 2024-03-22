/*
 * Copyright © 2019 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#ifndef CROCUS_MONITOR_H
#define CROCUS_MONITOR_H

#include "pipe/p_screen.h"

int crocus_get_monitor_info(struct pipe_screen *pscreen, unsigned index,
                            struct pipe_driver_query_info *info);
int crocus_get_monitor_group_info(struct pipe_screen *pscreen,
                                  unsigned index,
                                  struct pipe_driver_query_group_info *info);

struct crocus_context;
struct crocus_screen;

struct crocus_monitor_object *
crocus_create_monitor_object(struct crocus_context *ice,
                             unsigned num_queries,
                             unsigned *query_types);

struct pipe_query;
void crocus_destroy_monitor_object(struct pipe_context *ctx,
                                   struct crocus_monitor_object *monitor);

bool
crocus_begin_monitor(struct pipe_context *ctx,
                     struct crocus_monitor_object *monitor);
bool
crocus_end_monitor(struct pipe_context *ctx,
                   struct crocus_monitor_object *monitor);

bool
crocus_get_monitor_result(struct pipe_context *ctx,
                          struct crocus_monitor_object *monitor,
                          bool wait,
                          union pipe_numeric_type_union *result);

#endif
