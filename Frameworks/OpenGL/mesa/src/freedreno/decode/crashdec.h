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

#ifndef __CRASHDEC_H__
#define __CRASHDEC_H__

#include <assert.h>
#include <getopt.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "freedreno_pm4.h"

#include "ir3/instr-a3xx.h"
#include "buffers.h"
#include "cffdec.h"
#include "disasm.h"
#include "pager.h"
#include "rnnutil.h"
#include "util.h"

extern struct rnn *rnn_gmu;
extern struct rnn *rnn_control;
extern struct rnn *rnn_pipe;

extern bool verbose;

extern struct cffdec_options options;

static inline bool
is_a6xx(void)
{
   return options.info->chip == 6;
}

static inline bool
is_a5xx(void)
{
   return options.info->chip == 5;
}

static inline bool
is_64b(void)
{
   return options.info->chip >= 5;
}

static inline bool
is_gmu_legacy(void)
{
   switch (options.dev_id.gpu_id) {
   case 615:
   case 618:
   case 630:
      return true;
   default:
      return false;
   }
}

void dump_register(struct regacc *r);
void dump_cp_mem_pool(uint32_t *mempool);
void handle_prefetch(uint32_t *dwords, uint32_t sizedwords);

struct a6xx_hfi_state {
   uint64_t iova;
   void *buf;
   uint32_t size;
   int32_t history[2][8];
};
void dump_gmu_hfi(struct a6xx_hfi_state *hfi);

#endif /* __CRASHDEC_H__ */
