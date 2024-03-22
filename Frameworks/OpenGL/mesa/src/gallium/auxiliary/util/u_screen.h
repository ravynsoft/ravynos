/*
 * Copyright © 2018 Broadcom
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

#include <stdint.h>

struct pipe_screen;
struct pipe_screen_config;
struct renderonly;
enum pipe_cap;

#ifdef __cplusplus
extern "C" {
#endif

int
u_pipe_screen_get_param_defaults(struct pipe_screen *pscreen,
                                 enum pipe_cap param);

uint64_t u_default_get_timestamp(struct pipe_screen *screen);

typedef struct pipe_screen * (*pipe_screen_create_function)
   (int fd, const struct pipe_screen_config *config, struct renderonly *ro);

struct pipe_screen *
u_pipe_screen_lookup_or_create(int fd,
                               const struct pipe_screen_config *config,
                               struct renderonly *ro,
                               pipe_screen_create_function screen_create);

#ifdef __cplusplus
};
#endif
