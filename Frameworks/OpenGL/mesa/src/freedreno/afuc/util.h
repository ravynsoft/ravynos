/*
 * Copyright © 2021 Google, Inc.
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
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef _UTIL_H_
#define _UTIL_H_

#include <stdbool.h>

/*
 * AFUC disasm / asm helpers
 */

unsigned afuc_control_reg(const char *name);
char * afuc_control_reg_name(unsigned id);

unsigned afuc_sqe_reg(const char *name);
char * afuc_sqe_reg_name(unsigned id);

unsigned afuc_pipe_reg(const char *name);
char * afuc_pipe_reg_name(unsigned id);
bool afuc_pipe_reg_is_void(unsigned id);

unsigned afuc_gpu_reg(const char *name);
char * afuc_gpu_reg_name(unsigned id);

unsigned afuc_gpr_reg(const char *name);

int afuc_pm4_id(const char *name);
const char * afuc_pm_id_name(unsigned id);

enum afuc_color {
   AFUC_ERR,
   AFUC_LBL,
};

void afuc_printc(enum afuc_color c, const char *fmt, ...);

int afuc_util_init(int gpuver, bool colors);

#endif /* _UTIL_H_ */
