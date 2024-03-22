/*
 * Copyright © 2023 Igalia S.L.
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>

#include "util/os_file.h"

#include "isa.h"

static void
pre_instr_cb(void *d, unsigned n, void *instr)
{
   uint32_t *dwords = (uint32_t *)instr;
   printf("%03d [%08x %08x %08x %08x] ", n, dwords[0], dwords[1], dwords[2], dwords[3]);
}

int
main(int argc, char *argv[])
{
   size_t sz;
   void *raw = os_read_file(argv[1], &sz);

   isa_disasm(raw, sz, stdout,
              &(struct isa_decode_options){
                 .show_errors = true,
                 .branch_labels = true,
                 .pre_instr_cb = pre_instr_cb,
              });

   return 0;
}
