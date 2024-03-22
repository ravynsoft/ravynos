/*
 * Copyright (C) 2019 Connor Abbott <cwabbott0@gmail.com>
 * Copyright (C) 2019 Lyude Paul <thatslyude@gmail.com>
 * Copyright (C) 2019 Ryan Houdek <Sonicadvance1@gmail.com>
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

#ifndef __BI_DISASM_H
#define __BI_DISASM_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include "../bifrost.h"

void disassemble_bifrost(FILE *fp, uint8_t *code, size_t size, bool verbose);

void bi_disasm_fma(FILE *fp, unsigned bits, struct bifrost_regs *srcs,
                   struct bifrost_regs *next_regs, unsigned staging_register,
                   unsigned branch_offset, struct bi_constants *consts,
                   bool first);

void bi_disasm_add(FILE *fp, unsigned bits, struct bifrost_regs *srcs,
                   struct bifrost_regs *next_regs, unsigned staging_register,
                   unsigned branch_offset, struct bi_constants *consts,
                   bool first);

void bi_disasm_dest_fma(FILE *fp, struct bifrost_regs *next_regs, bool first);
void bi_disasm_dest_add(FILE *fp, struct bifrost_regs *next_regs, bool first);

void dump_src(FILE *fp, unsigned src, struct bifrost_regs srcs,
              unsigned branch_offset, struct bi_constants *consts, bool isFMA);

#endif
