/*
 * Copyright 2023 Alyssa Rosenzweig
 * SPDX-License-Identifier: MIT
 */

#include "agx_compiler.h"

/* Table describing the relationship between registers pressure and thread
 * count. Each entry describes a maximum number of registers and the associated
 * best-case thread count.
 *
 * Sorted in ascending order of maximum registers for easy lookup.
 */
static const struct agx_occupancy occupancies[] = {
   {104, 1024}, {112, 896}, {128, 832}, {136, 768}, {144, 704},
   {160, 640},  {184, 576}, {208, 512}, {232, 448}, {256, 384},
};

struct agx_occupancy
agx_occupancy_for_register_count(unsigned halfregs)
{
   for (unsigned i = 0; i < ARRAY_SIZE(occupancies); ++i) {
      unsigned max = occupancies[i].max_registers;
      assert((i == 0 || max > occupancies[i - 1].max_registers) && "ascending");

      if (halfregs <= max)
         return occupancies[i];
   }

   unreachable("Register count must be less than the maximum");
}
