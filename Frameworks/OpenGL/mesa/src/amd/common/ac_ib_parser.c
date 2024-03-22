/*
 * Copyright © 2023 Advanced Micro Devices, Inc.
 *
 * SPDX-License-Identifier: MIT
 */

#include "ac_debug.h"
#include <string.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
   if (argc < 3) {
      fprintf(stderr, "Usage: [LLVM processor] [IB filenames]\n");
      return 1;
   }

   const char *gpu = argv[1];
   enum amd_gfx_level gfx_level;
   enum radeon_family family = CHIP_UNKNOWN;

   for (unsigned i = 0; i < CHIP_LAST; i++) {
      if (!strcmp(gpu, ac_get_llvm_processor_name(i))) {
         family = i;
         gfx_level = ac_get_gfx_level(i);
         break;
      }
   }

   if (family == CHIP_UNKNOWN) {
      fprintf(stderr, "Unknown LLVM processor.\n");
      return 1;
   }

   for (unsigned i = 2; i < argc; i++) {
      const char *filename = argv[i];

      FILE *f = fopen(filename, "r");
      if (!f) {
         fprintf(stderr, "Can't open IB: %s\n", filename);
         continue;
      }

      fseek(f, 0, SEEK_END);
      int size = ftell(f);
      fseek(f, 0, SEEK_SET);

      uint32_t *ib = (uint32_t*)malloc(size);

      if (fread(ib, size, 1, f) != 1) {
         fprintf(stderr, "Can't read IB: %s\n", filename);
         fclose(f);
         free(ib);
         continue;
      }
      fclose(f);

      ac_parse_ib(stdout, ib, size / 4, NULL, 0, filename, gfx_level, family, AMD_IP_GFX, NULL, NULL);
      free(ib);
   }

   return 0;
}
