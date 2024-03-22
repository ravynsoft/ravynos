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

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#include "emu.h"
#include "util.h"

/*
 * Emulation for draw-state (ie. CP_SET_DRAW_STATE) related control registers:
 */

EMU_CONTROL_REG(DRAW_STATE_SET_HDR);
EMU_CONTROL_REG(DRAW_STATE_SEL);
EMU_CONTROL_REG(DRAW_STATE_ACTIVE_BITMASK);
EMU_CONTROL_REG(DRAW_STATE_HDR);
EMU_CONTROL_REG(DRAW_STATE_BASE);
EMU_CONTROL_REG(SDS_BASE);
EMU_CONTROL_REG(SDS_DWORDS);

uint32_t
emu_get_draw_state_reg(struct emu *emu, unsigned n)
{
   // TODO maybe we don't need to do anything here
   return emu->control_regs.val[n];
}

void
emu_set_draw_state_base(struct emu *emu, unsigned n, uint32_t val)
{
   struct emu_draw_state *ds = &emu->draw_state;

   unsigned cur_idx = (emu_get_reg32(emu, &DRAW_STATE_SET_HDR) >> 24) & 0x1f;
   ds->state[cur_idx].base_lohi[n] = val;
}

void
emu_set_draw_state_reg(struct emu *emu, unsigned n, uint32_t val)
{
   struct emu_draw_state *ds = &emu->draw_state;
   unsigned cur_idx = emu_get_reg32(emu, &DRAW_STATE_SEL);

   if (n == emu_reg_offset(&DRAW_STATE_SET_HDR)) {
      cur_idx = (val >> 24) & 0x1f;
      ds->state[cur_idx].count = val & 0xffff;
      ds->state[cur_idx].mode_mask = (val >> 20) & 0x7;

      unsigned active_mask = emu_get_reg32(emu, &DRAW_STATE_ACTIVE_BITMASK);
      active_mask |= (1 << cur_idx);

      emu_set_reg32(emu, &DRAW_STATE_ACTIVE_BITMASK, active_mask);
   } else if (n == emu_reg_offset(&DRAW_STATE_SEL)) {
      emu_set_reg32(emu, &DRAW_STATE_HDR, ds->state[val].hdr);
      emu_set_reg64(emu, &DRAW_STATE_BASE, ds->state[val].base);

      /* It seems that SDS_BASE/SDS_DWORDS is also per draw-state group,
       * and that when a new state-group is selected, SQE compares to
       * the previous values to new DRAW_STATE_BASE & count to detect
       * that new state has been appended to existing draw-state group:
       */
      unsigned prev_idx = ds->prev_draw_state_sel;
      ds->state[prev_idx].sds_base = emu_get_reg64(emu, &SDS_BASE);
      ds->state[prev_idx].sds_dwords = emu_get_reg32(emu, &SDS_DWORDS);

      emu_set_reg64(emu, &SDS_BASE, ds->state[val].sds_base);
      emu_set_reg32(emu, &SDS_DWORDS, ds->state[val].sds_dwords);

      ds->prev_draw_state_sel = val;
   }
}
