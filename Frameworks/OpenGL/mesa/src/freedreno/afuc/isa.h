/*
 * Copyright © 2020 Google, Inc.
 * Copyright © 2023 Valve Corporation
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

#ifndef _ISA_H_
#define _ISA_H_

#include <stdlib.h>

#include "compiler/isaspec/isaspec.h"
#include "afuc.h"

static inline struct afuc_instr *__instruction_create(afuc_opc opc)
{
   struct afuc_instr *instr = calloc(1, sizeof(struct afuc_instr));

   switch (opc) {
#define ALU(name) \
   case OPC_##name##I: \
      instr->opc = OPC_##name; \
      instr->has_immed = true; \
      break;
   ALU(ADD)
   ALU(ADDHI)
   ALU(SUB)
   ALU(SUBHI)
   ALU(AND)
   ALU(OR)
   ALU(XOR)
   ALU(NOT)
   ALU(SHL)
   ALU(USHR)
   ALU(ISHR)
   ALU(ROT)
   ALU(MUL8)
   ALU(MIN)
   ALU(MAX)
   ALU(CMP)
#undef ALU

   default:
      instr->opc = opc;
   }

   return instr;
}

#endif /* _ISA_H_ */
