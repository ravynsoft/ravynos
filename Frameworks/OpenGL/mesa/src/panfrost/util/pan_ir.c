/*
 * Copyright (C) 2020 Collabora Ltd.
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
 * Authors (Collabora):
 *      Alyssa Rosenzweig <alyssa.rosenzweig@collabora.com>
 */

#include "pan_ir.h"
#include "util/macros.h"

/* Converts a per-component mask to a byte mask */

uint16_t
pan_to_bytemask(unsigned bytes, unsigned mask)
{
   switch (bytes) {
   case 0:
      assert(mask == 0);
      return 0;

   case 8:
      return mask;

   case 16: {
      unsigned space =
         (mask & 0x1) | ((mask & 0x2) << (2 - 1)) | ((mask & 0x4) << (4 - 2)) |
         ((mask & 0x8) << (6 - 3)) | ((mask & 0x10) << (8 - 4)) |
         ((mask & 0x20) << (10 - 5)) | ((mask & 0x40) << (12 - 6)) |
         ((mask & 0x80) << (14 - 7));

      return space | (space << 1);
   }

   case 32: {
      unsigned space = (mask & 0x1) | ((mask & 0x2) << (4 - 1)) |
                       ((mask & 0x4) << (8 - 2)) | ((mask & 0x8) << (12 - 3));

      return space | (space << 1) | (space << 2) | (space << 3);
   }

   case 64: {
      unsigned A = (mask & 0x1) ? 0xFF : 0x00;
      unsigned B = (mask & 0x2) ? 0xFF : 0x00;
      return A | (B << 8);
   }

   default:
      unreachable("Invalid register mode");
   }
}

void
pan_block_add_successor(pan_block *block, pan_block *successor)
{
   assert(block);
   assert(successor);

   /* Cull impossible edges */
   if (block->unconditional_jumps)
      return;

   for (unsigned i = 0; i < ARRAY_SIZE(block->successors); ++i) {
      if (block->successors[i]) {
         if (block->successors[i] == successor)
            return;
         else
            continue;
      }

      block->successors[i] = successor;
      _mesa_set_add(successor->predecessors, block);
      return;
   }

   unreachable("Too many successors");
}

/* Prints a NIR ALU type in Bifrost-style ".f32" ".i8" etc */

void
pan_print_alu_type(nir_alu_type t, FILE *fp)
{
   unsigned size = nir_alu_type_get_type_size(t);
   nir_alu_type base = nir_alu_type_get_base_type(t);

   switch (base) {
   case nir_type_int:
      fprintf(fp, ".i");
      break;
   case nir_type_uint:
      fprintf(fp, ".u");
      break;
   case nir_type_bool:
      fprintf(fp, ".b");
      break;
   case nir_type_float:
      fprintf(fp, ".f");
      break;
   default:
      fprintf(fp, ".unknown");
      break;
   }

   fprintf(fp, "%u", size);
}

/* Could optimize with a better data structure if anyone cares, TODO: profile */

unsigned
pan_lookup_pushed_ubo(struct panfrost_ubo_push *push, unsigned ubo,
                      unsigned offs)
{
   struct panfrost_ubo_word word = {.ubo = ubo, .offset = offs};

   for (unsigned i = 0; i < push->count; ++i) {
      if (memcmp(push->words + i, &word, sizeof(word)) == 0)
         return i;
   }

   unreachable("UBO not pushed");
}
