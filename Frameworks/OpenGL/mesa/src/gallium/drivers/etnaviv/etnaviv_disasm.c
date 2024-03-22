/*
 * Copyright (c) 2016 Etnaviv Project
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sub license,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Authors:
 *    Christian Gmeiner <christian.gmeiner@gmail.com>
 */

#include <assert.h>
#include <isa/isa.h>

#include "etnaviv_disasm.h"

static void
pre_instr_cb(void *d, unsigned n, void *instr)
{
   uint32_t *dwords = (uint32_t *)instr;
   printf("%03d [%08x %08x %08x %08x] ", n, dwords[0], dwords[1], dwords[2], dwords[3]);
}

void
etna_disasm(uint32_t *dwords, int sizedwords, enum debug_t debug)
{
   assert((sizedwords % 2) == 0);

   struct isa_decode_options options = {
      .show_errors = true,
      .branch_labels = true,
   };

   if (debug & PRINT_RAW)
      options.pre_instr_cb = pre_instr_cb;

   isa_disasm(dwords, sizedwords * sizeof(uint32_t), stdout, &options);
}
