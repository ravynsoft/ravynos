/*
 * Copyright 2021 Alyssa Rosenzweig
 * SPDX-License-Identifier: MIT
 */

#ifndef U_HEXDUMP_H
#define U_HEXDUMP_H

#include <stdio.h>
#include <stdbool.h>

static inline void
u_hexdump(FILE *fp, const uint8_t *hex, size_t cnt, bool with_strings)
{
   for (unsigned i = 0; i < cnt; ++i) {
      if ((i & 0xF) == 0)
         fprintf(fp, "%06X  ", i);

      uint8_t v = hex[i];

      if (v == 0 && (i & 0xF) == 0) {
         /* Check if we're starting an aligned run of zeroes */
         unsigned zero_count = 0;

         for (unsigned j = i; j < cnt; ++j) {
            if (hex[j] == 0)
               zero_count++;
            else
               break;
         }

         if (zero_count >= 32) {
            fprintf(fp, "*\n");
            i += (zero_count & ~0xF) - 1;
            continue;
         }
      }

      fprintf(fp, "%02X ", hex[i]);
      if ((i & 0xF) == 0xF && with_strings) {
         fprintf(fp, " | ");
         for (unsigned j = i & ~0xF; j <= i; ++j) {
            uint8_t c = hex[j];
            fputc((c < 32 || c > 128) ? '.' : c, fp);
         }
      }

      if ((i & 0xF) == 0xF)
         fprintf(fp, "\n");
   }

   fprintf(fp, "\n");
}

#endif
