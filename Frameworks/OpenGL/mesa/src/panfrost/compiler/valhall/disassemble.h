#ifndef __DISASM_H
#define __DISASM_H

#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BIT(b)         (1ull << (b))
#define MASK(count)    ((1ull << (count)) - 1)
#define SEXT(b, count) ((b ^ BIT(count - 1)) - BIT(count - 1))
#define UNUSED         __attribute__((unused))

#define VA_SRC_UNIFORM_TYPE 0x2
#define VA_SRC_IMM_TYPE     0x3

static inline void
va_print_dest(FILE *fp, uint8_t dest, bool can_mask)
{
   unsigned mask = (dest >> 6);
   unsigned value = (dest & 0x3F);
   fprintf(fp, "r%u", value);

   /* Should write at least one component */
   //	assert(mask != 0);
   //	assert(mask == 0x3 || can_mask);

   if (mask != 0x3)
      fprintf(fp, ".h%u", (mask == 1) ? 0 : 1);
}

void va_disasm_instr(FILE *fp, uint64_t instr);

static inline void
disassemble_valhall(FILE *fp, const uint64_t *code, unsigned size, bool verbose)
{
   assert((size & 7) == 0);

   /* Segment into 8-byte instructions */
   for (unsigned i = 0; i < (size / 8); ++i) {
      uint64_t instr = code[i];

      if (instr == 0) {
         fprintf(fp, "\n");
         return;
      }

      if (verbose) {
         /* Print byte pattern */
         for (unsigned j = 0; j < 8; ++j)
            fprintf(fp, "%02x ", (uint8_t)(instr >> (j * 8)));

         fprintf(fp, "   ");
      } else {
         /* Print whitespace */
         fprintf(fp, "   ");
      }

      va_disasm_instr(fp, instr);
      fprintf(fp, "\n");

      /* Detect branches */
      uint64_t opcode = (instr >> 48) & MASK(9);
      bool branchz = (opcode == 0x1F);
      bool branchzi = (opcode == 0x2F);

      /* Separate blocks visually by inserting whitespace after branches */
      if (branchz || branchzi)
         fprintf(fp, "\n");
   }

   fprintf(fp, "\n");
}

#endif
