/*
 * Copyright (C) 2019 Rob Clark <robclark@freedesktop.org>
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
 *
 * Authors:
 *    Rob Clark <robclark@freedesktop.org>
 */

#include <stddef.h>

#include "freedreno_perfcntr.h"

extern const struct fd_perfcntr_group a2xx_perfcntr_groups[];
extern const unsigned a2xx_num_perfcntr_groups;

extern const struct fd_perfcntr_group a5xx_perfcntr_groups[];
extern const unsigned a5xx_num_perfcntr_groups;

extern const struct fd_perfcntr_group a6xx_perfcntr_groups[];
extern const unsigned a6xx_num_perfcntr_groups;

const struct fd_perfcntr_group *
fd_perfcntrs(const struct fd_dev_id *id, unsigned *count)
{
   switch (fd_dev_gen(id)) {
   case 2:
      *count = a2xx_num_perfcntr_groups;
      return a2xx_perfcntr_groups;
   case 5:
      *count = a5xx_num_perfcntr_groups;
      return a5xx_perfcntr_groups;
   case 6:
      *count = a6xx_num_perfcntr_groups;
      return a6xx_perfcntr_groups;
   default:
      *count = 0;
      return NULL;
   }
}
