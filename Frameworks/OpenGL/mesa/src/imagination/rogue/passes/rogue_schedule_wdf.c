/*
 * Copyright © 2022 Imagination Technologies Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "rogue.h"
#include "rogue_builder.h"
#include "util/macros.h"

#include <stdbool.h>

/**
 * \file rogue_schedule_wdf.c
 *
 * \brief Contains the rogue_schedule_wdf pass.
 */

static bool
rogue_insert_wdf(rogue_builder *b, rogue_drc_trxn *drc_trxn, unsigned num)
{
   assert(drc_trxn->acquire);

   if (drc_trxn->release)
      return false;

   b->cursor = rogue_cursor_after_instr(drc_trxn->acquire);
   drc_trxn->release = &rogue_WDF(b, rogue_ref_drc_trxn(num, drc_trxn))->instr;

   return true;
}

/* Schedules WDF instructions for DRC transactions. */
PUBLIC
bool rogue_schedule_wdf(rogue_shader *shader, bool latency_hiding)
{
   if (shader->is_grouped)
      return false;

   /* TODO: Add support for delayed scheduling (latency hiding). */
   if (latency_hiding)
      unreachable("Latency hiding is unimplemented.");

   bool progress = false;

   rogue_builder b;
   rogue_builder_init(&b, shader);

   rogue_foreach_drc_trxn (drc_trxn, shader, 0)
      progress |= rogue_insert_wdf(&b, drc_trxn, 0);

   rogue_foreach_drc_trxn (drc_trxn, shader, 1)
      progress |= rogue_insert_wdf(&b, drc_trxn, 1);

   return progress;
}
