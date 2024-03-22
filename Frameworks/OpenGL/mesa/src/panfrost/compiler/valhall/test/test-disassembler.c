/*
 * Copyright (C) 2021 Collabora, Ltd.
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

#include <inttypes.h>
#include <stdio.h>
#include "disassemble.h"

static inline uint8_t
parse_nibble(const char c)
{
   return (c >= 'a') ? 10 + (c - 'a') : (c - '0');
}

/* Given a little endian 8 byte hexdump, parse out the 64-bit value */
static uint64_t
parse_hex(const char *in)
{
   uint64_t v = 0;

   for (unsigned i = 0; i < 8; ++i) {
      uint8_t byte = (parse_nibble(in[0]) << 4) | parse_nibble(in[1]);
      v |= ((uint64_t)byte) << (8 * i);

      /* Skip the space after the byte */
      in += 3;
   }

   return v;
}

int
main(int argc, const char **argv)
{
   if (argc < 2) {
      fprintf(stderr, "Expected case list\n");
      return 1;
   }

   FILE *fp = fopen(argv[1], "r");

   if (fp == NULL) {
      fprintf(stderr, "Could not open the case list");
      return 1;
   }

   char line[128];
   unsigned nr_fail = 0, nr_pass = 0;

   while (fgets(line, sizeof(line), fp) != NULL) {
      char *output = NULL;
      size_t sz = 0;
      size_t len = strlen(line);

      /* Skip empty lines */
      if (len <= 1)
         continue;

      /* Check for buffer overflow */
      if (len < 28) {
         fprintf(stderr, "Invalid reference %s\n", line);
         nr_fail++;
      }

      uint64_t bin = parse_hex(line);
      FILE *outputp = open_memstream(&output, &sz);
      va_disasm_instr(outputp, bin);
      fprintf(outputp, "\n");
      fclose(outputp);

      /* Skip hexdump: 8 bytes * (2 nibbles + 1 space) + 3 spaces */
      const char *reference = line + 27;
      bool fail = strcmp(reference, output);

      if (fail) {
         /* Extra spaces after Got to align with Expected */
         printf("Got      %sExpected %s\n", output, reference);
         nr_fail++;
      } else {
         nr_pass++;
      }

      free(output);
   }

   printf("Passed %u/%u tests.\n", nr_pass, nr_pass + nr_fail);
   fclose(fp);

   return nr_fail ? 1 : 0;
}
