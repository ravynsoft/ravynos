/*
 * Copyright (c) 2017 Rob Clark <robdclark@gmail.com>
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

#ifndef _ASM_H_
#define _ASM_H_

#include <stdbool.h>
#include <stdint.h>
#include "afuc.h"

extern int gpuver;

struct asm_label {
   unsigned offset;
   const char *label;
};

struct afuc_instr *next_instr(afuc_opc opc);
void decl_label(const char *str);

static inline uint32_t
parse_reg(const char *str)
{
   char *retstr;
   long int ret;

   if (!strcmp(str, "$rem"))
      return REG_REM;
   else if (!strcmp(str, "$memdata"))
      return REG_MEMDATA;
   else if (!strcmp(str, "$addr"))
      return REG_ADDR;
   else if (!strcmp(str, "$regdata"))
      return REG_REGDATA;
   else if (!strcmp(str, "$usraddr"))
      return REG_USRADDR;
   else if (!strcmp(str, "$data"))
      return 0x1f;

   ret = strtol(str + 1, &retstr, 16);

   if (*retstr != '\0') {
      printf("invalid register: %s\n", str);
      exit(2);
   }

   return ret;
}

static inline uint32_t
parse_literal(const char *str)
{
   char *retstr;
   long int ret;

   ret = strtol(str + 1, &retstr, 16);

   if (*retstr != ']') {
      printf("invalid literal: %s\n", str);
      exit(2);
   }

   return ret;
}

static inline uint32_t
parse_bit(const char *str)
{
   return strtol(str + 1, NULL, 10);
}

unsigned parse_control_reg(const char *name);
unsigned parse_sqe_reg(const char *name);

/* string trailing ':' off label: */
static inline const char *
parse_label_decl(const char *str)
{
   char *s = strdup(str);
   s[strlen(s) - 1] = '\0';
   return s;
}

void yyset_in(FILE *_in_str);

#endif /* _ASM_H_ */
