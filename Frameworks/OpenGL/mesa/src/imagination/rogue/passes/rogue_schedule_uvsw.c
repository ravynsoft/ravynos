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
 * \file rogue_schedule_uvsw.c
 *
 * \brief Contains the rogue_schedule_uvsw pass.
 */

/* TODO: See if maybe this can/should be done in rogue_lower_END instead? */

/* Schedules UVSW task control instructions. */
PUBLIC
bool rogue_schedule_uvsw(rogue_shader *shader, bool latency_hiding)
{
   if (shader->is_grouped)
      return false;

   /* TODO: Support for other shader types that write to the unified vertex
    * store. */
   if (shader->stage != MESA_SHADER_VERTEX)
      return false;

   /* TODO: Add support for delayed scheduling (latency hiding). */
   if (latency_hiding)
      unreachable("Latency hiding is unimplemented.");

   rogue_builder b;
   rogue_builder_init(&b, shader);

   /* Check whether there are uvsw.writes. */
   bool uvsw_write_present = false;
   rogue_foreach_instr_in_shader (instr, shader) {
      if (instr->type != ROGUE_INSTR_TYPE_BACKEND)
         continue;

      rogue_backend_instr *backend = rogue_instr_as_backend(instr);
      if (backend->op != ROGUE_BACKEND_OP_UVSW_WRITE)
         continue;

      uvsw_write_present = true;
      break;
   }

   if (!uvsw_write_present)
      return false;

   /* Insert emit/end task before the final instruction (nop.end). */
   rogue_block *final_block =
      list_last_entry(&shader->blocks, rogue_block, link);
   rogue_instr *final_instr =
      list_last_entry(&final_block->instrs, rogue_instr, link);

   if (!rogue_instr_is_nop_end(final_instr))
      unreachable("UVSW emit/end task need to be the final instruction.");

   b.cursor = rogue_cursor_before_instr(final_instr);

   /* TODO: If instruction before nop.end is uvsw.write then
    * UVSW_WRITETHENEMITTHENENDTASK. */

   rogue_UVSW_EMIT(&b);
   rogue_UVSW_ENDTASK(&b);

   /* TODO: replace nop.end and add end flag to final uvsw instruction instead.
    */

   return true;
}
