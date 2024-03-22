/*
 * Copyright © 2022 Collabora Ltd.
 * SPDX-License-Identifier: MIT
 */
#include "mme_tu104.h"

#include "mme_tu104_isa.h"
#include "isa.h"

#include <stdlib.h>

static void
disasm_instr_cb(void *d, unsigned n, void *instr)
{
   uint32_t *dwords = (uint32_t *)instr;
   fprintf(d, "%3d[%08x_%08x_%08x] ", n, dwords[2], dwords[1], dwords[0]);
}

void
mme_tu104_dump(FILE *fp, uint32_t *encoded, size_t encoded_size)
{
   assert(encoded_size % 12 == 0);

   uint32_t *swapped = malloc(encoded_size);
   for (uint32_t i = 0; i < (encoded_size / 12); i++) {
      swapped[i * 3 + 0] = encoded[i * 3 + 2];
      swapped[i * 3 + 1] = encoded[i * 3 + 1];
      swapped[i * 3 + 2] = encoded[i * 3 + 0];
   }

   const struct isa_decode_options opts = {
      .show_errors = true,
      .branch_labels = true,
      .cbdata = fp,
      .pre_instr_cb = disasm_instr_cb,
   };
   isa_disasm(swapped, encoded_size, fp, &opts);

   free(swapped);
}
