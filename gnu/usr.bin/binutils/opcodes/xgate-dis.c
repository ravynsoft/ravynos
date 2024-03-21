/* xgate-dis.c -- Freescale XGATE disassembly
   Copyright (C) 2009-2023 Free Software Foundation, Inc.
   Written by Sean Keys (skeys@ipdatasys.com)

   This file is part of the GNU opcodes library.

   This library is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   It is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
   License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

#include "sysdep.h"
#include <assert.h>
#include "disassemble.h"
#include "opintl.h"
#include "libiberty.h"
#include "ansidecl.h"
#include "opcode/xgate.h"

#define XGATE_TWO_BYTES      0x02
#define XGATE_NINE_BITS      0x1FF
#define XGATE_TEN_BITS       0x3FF
#define XGATE_NINE_SIGNBIT   0x100
#define XGATE_TEN_SIGNBIT    0x200

/* Structures.  */
struct decodeInfo
{
  unsigned int operMask;
  unsigned int operMasksRegisterBits;
  struct xgate_opcode *opcodePTR;
};

/* Prototypes for local functions.  */
static int print_insn (bfd_vma, struct disassemble_info *);
static int read_memory (bfd_vma, bfd_byte*, int, struct disassemble_info *);
static int ripBits (unsigned int *, int,
		    struct xgate_opcode *, unsigned int);
static int macro_search (char *, char *);
static struct decodeInfo * find_match (unsigned int);

/* Statics.  */
static struct decodeInfo *decodeTable;
static int initialized;
static char previousOpName[10];
static unsigned int perviousBin;

/* Disassemble one instruction at address 'memaddr'.  Returns the number
   of bytes used by that instruction.  */

static int
print_insn (bfd_vma memaddr, struct disassemble_info* info)
{
  int status;
  unsigned int raw_code;
  char *s = 0;
  long bytesRead = 0;
  int i = 0;
  struct xgate_opcode *opcodePTR = (struct xgate_opcode*) xgate_opcodes;
  struct decodeInfo *decodeTablePTR = 0;
  struct decodeInfo *decodePTR = 0;
  unsigned int operandRegisterBits = 0;
  signed int relAddr = 0;
  signed int operandOne = 0;
  signed int operandTwo = 0;
  bfd_byte buffer[4];
  bfd_vma absAddress;

  unsigned int operMaskReg = 0;
  /* Initialize our array of opcode masks and check them against our constant
     table.  */
  if (!initialized)
    {
      decodeTable = xmalloc (sizeof (struct decodeInfo) * xgate_num_opcodes);
      for (i = 0, decodeTablePTR = decodeTable; i < xgate_num_opcodes;
          i++, decodeTablePTR++, opcodePTR++)
        {
          unsigned int bin = 0;
          unsigned int mask = 0;
          for (s = opcodePTR->format; *s; s++)
            {
              bin <<= 1;
              mask <<= 1;
              operandRegisterBits <<= 1;
              bin |= (*s == '1');
              mask |= (*s == '0' || *s == '1');
              operandRegisterBits |= (*s == 'r');
            }
          /* Asserting will uncover inconsistencies in our table.  */
          assert ((s - opcodePTR->format) == 16 || (s - opcodePTR->format) == 32);
          assert (opcodePTR->bin_opcode == bin);

          decodeTablePTR->operMask = mask;
          decodeTablePTR->operMasksRegisterBits = operandRegisterBits;
          decodeTablePTR->opcodePTR = opcodePTR;
        }
      initialized = 1;
    }

  /* Read 16 bits.  */
  bytesRead += XGATE_TWO_BYTES;
  status = read_memory (memaddr, buffer, XGATE_TWO_BYTES, info);
  if (status == 0)
    {
      raw_code = buffer[0];
      raw_code <<= 8;
      raw_code += buffer[1];

      decodePTR = find_match (raw_code);
      if (decodePTR)
        {
          operMaskReg = decodePTR->operMasksRegisterBits;
          (*info->fprintf_func)(info->stream, "%s", decodePTR->opcodePTR->name);

          /* First we compare the shorthand format of the constraints. If we
	      still are unable to pinpoint the operands
	      we analyze the opcodes constraint string.  */
          if (!strcmp (decodePTR->opcodePTR->constraints, XGATE_OP_MON_R_C))
        	{
        	  (*info->fprintf_func)(info->stream, " R%x, CCR",
        		  (raw_code >> 8) & 0x7);
        	}
          else if (!strcmp (decodePTR->opcodePTR->constraints, XGATE_OP_MON_C_R))
            {
        	  (*info->fprintf_func)(info->stream, " CCR, R%x",
        	      (raw_code >> 8) & 0x7);
            }
          else if (!strcmp (decodePTR->opcodePTR->constraints, XGATE_OP_MON_R_P))
            {
        	  (*info->fprintf_func)(info->stream, " R%x, PC",
        	      (raw_code >> 8) & 0x7);
            }
          else if (!strcmp (decodePTR->opcodePTR->constraints, XGATE_OP_TRI))
            {
                  (*info->fprintf_func)(info->stream, " R%x, R%x, R%x",
                      (raw_code >> 8) & 0x7, (raw_code >> 5) & 0x7,
                      (raw_code >> 2) & 0x7);
            }
          else if (!strcmp (decodePTR->opcodePTR->constraints, XGATE_OP_IDR))
            {
                  if (raw_code & 0x01)
                    {
                      (*info->fprintf_func)(info->stream, " R%x, (R%x, R%x+)",
                          (raw_code >> 8) & 0x7, (raw_code >> 5) & 0x7,
                          (raw_code >> 2) & 0x7);
                    }
                   else if (raw_code & 0x02)
                          {
                            (*info->fprintf_func)(info->stream, " R%x, (R%x, -R%x)",
                                (raw_code >> 8) & 0x7, (raw_code >> 5) & 0x7,
                                (raw_code >> 2) & 0x7);
                          }
                   else
                     {
                       (*info->fprintf_func)(info->stream, " R%x, (R%x, R%x)",
                           (raw_code >> 8) & 0x7, (raw_code >> 5) & 0x7,
                           (raw_code >> 2) & 0x7);
                     }
            }
          else if (!strcmp (decodePTR->opcodePTR->constraints, XGATE_OP_DYA))
            {
        	  operandOne = ripBits (&operMaskReg, 3, decodePTR->opcodePTR, raw_code);
        	  operandTwo = ripBits (&operMaskReg, 3, decodePTR->opcodePTR, raw_code);
        	 ( *info->fprintf_func)(info->stream, " R%x, R%x", operandOne,
        	      operandTwo);
            }
          else if (!strcmp (decodePTR->opcodePTR->constraints, XGATE_OP_IDO5))
            {
        	  (*info->fprintf_func)(info->stream, " R%x, (R%x, #0x%x)",
        	      (raw_code >> 8) & 0x7, (raw_code >> 5) & 0x7, raw_code & 0x1f);
            }
          else if (!strcmp (decodePTR->opcodePTR->constraints, XGATE_OP_MON))
            {
        	  operandOne = ripBits (&operMaskReg, 3, decodePTR->opcodePTR,
        	     raw_code);
        	 (*info->fprintf_func)(info->stream, " R%x", operandOne);
            }
          else if (!strcmp (decodePTR->opcodePTR->constraints, XGATE_OP_REL9))
            {
              /* If address is negative handle it accordingly.  */
              if (raw_code & XGATE_NINE_SIGNBIT)
                {
                  relAddr = XGATE_NINE_BITS >> 1; /* Clip sign bit.  */
                  relAddr = ~relAddr; /* Make signed.  */
                  relAddr |= (raw_code & 0xFF) + 1; /* Apply our value.  */
                  relAddr *= 2; /* Multiply by two as per processor docs.  */
                }
              else
                {
                  relAddr = raw_code & 0xff;
                  relAddr = relAddr * 2 + 2;
                }
             (*info->fprintf_func)(info->stream, " *%d", relAddr);
             (*info->fprintf_func)(info->stream, "  Abs* 0x");
             (*info->print_address_func)(memaddr + relAddr, info);
           }
          else if (!strcmp (decodePTR->opcodePTR->constraints, XGATE_OP_REL10))
            {
              /* If address is negative handle it accordingly.  */
              if (raw_code & XGATE_TEN_SIGNBIT)
                {
                  relAddr = XGATE_TEN_BITS >> 1; /* Clip sign bit.  */
                  relAddr = ~relAddr; /* Make signed.  */
                  relAddr |= (raw_code & 0x1FF) + 1; /* Apply our value.  */
                  relAddr *= 2; /* Multiply by two as per processor docs.  */
                }
              else
                {
                  relAddr = raw_code & 0x1FF;
                  relAddr = relAddr * 2 + 2;
                }
              (*info->fprintf_func)(info->stream, " *%d", relAddr);
              (*info->fprintf_func)(info->stream, "  Abs* 0x");
              (*info->print_address_func)(memaddr + relAddr, info);
            }
          else if (!strcmp (decodePTR->opcodePTR->constraints, XGATE_OP_IMM4))
            {
              (*info->fprintf_func)(info->stream, " R%x, #0x%02x",
              (raw_code >> 8) & 0x7, (raw_code >> 4) & 0xF);
            }
          else if (!strcmp (decodePTR->opcodePTR->constraints, XGATE_OP_IMM8))
            {
              if (macro_search (decodePTR->opcodePTR->name, previousOpName) &&
                 previousOpName[0])
               {
                 absAddress = (0xFF & raw_code) << 8;
                 absAddress |= perviousBin & 0xFF;
                 (*info->fprintf_func)(info->stream, " R%x, #0x%02x Abs* 0x",
                     (raw_code >> 8) & 0x7, raw_code & 0xff);
                 (*info->print_address_func)(absAddress, info);
                 previousOpName[0] = 0;
               }
              else
               {
                 strcpy (previousOpName, decodePTR->opcodePTR->name);
                 (*info->fprintf_func)(info->stream, " R%x, #0x%02x",
                     (raw_code >> 8) & 0x7, raw_code & 0xff);
               }
            }
          else if (!strcmp (decodePTR->opcodePTR->constraints, XGATE_OP_IMM3))
            {
        	  (*info->fprintf_func)(info->stream, " #0x%x",
        	     (raw_code >> 8) & 0x7);
            }
          else if (!strcmp (decodePTR->opcodePTR->constraints, XGATE_OP_INH))
            {
            }
          else
            {
              (*info->fprintf_func)(info->stream, " unhandled mode %s",
				    decodePTR->opcodePTR->constraints);
            }
          perviousBin = raw_code;
        }
      else
        {
          (*info->fprintf_func)(info->stream,
				" unable to find opcode match #0%x", raw_code);
        }
    }
  return bytesRead;
}

int
print_insn_xgate (bfd_vma memaddr, struct disassemble_info* info)
{
  return print_insn (memaddr, info);
}

static int
read_memory (bfd_vma memaddr, bfd_byte* buffer, int size,
    struct disassemble_info* info)
{
  int status;
  status = (*info->read_memory_func) (memaddr, buffer, size, info);
  if (status != 0)
    {
      (*info->memory_error_func) (status, memaddr, info);
      return -1;
    }
  return 0;
}

static int
ripBits (unsigned int *operandBitsRemaining,
	 int numBitsRequested,
	 struct xgate_opcode *opcodePTR,
	 unsigned int memory)
{
  unsigned int currentBit;
  unsigned int operand = 0;
  int numBitsFound;

  for (numBitsFound = 0, currentBit = 1u << ((opcodePTR->size * 8) - 1);
       numBitsFound < numBitsRequested && currentBit != 0;
       currentBit >>= 1)
    {
      if (currentBit & *operandBitsRemaining)
	{
	  *operandBitsRemaining &= ~(currentBit); /* Consume the current bit.  */
	  operand <<= 1; /* Make room for our next bit.  */
	  numBitsFound++;
	  operand |= (currentBit & memory) > 0;
	}
    }
  return operand;
}

static int
macro_search (char *currentName, char *lastName)
{
  int i;
  int length = 0;
  char *where;

  for (i = 0; i < xgate_num_opcodes; i++)
    {
      where = strstr (xgate_opcodes[i].constraints, lastName);

      if (where)
        {
          length = strlen (where);
        }
      if (length)
        {
          where = strstr (xgate_opcodes[i].constraints, currentName);
          if (where)
            {
              length = strlen (where);
              return 1;
            }
        }
    }
  return 0;
}

static struct decodeInfo *
find_match (unsigned int raw_code)
{
  struct decodeInfo *decodeTablePTR = 0;
  int i;

  for (i = 0, decodeTablePTR = decodeTable; i < xgate_num_opcodes;
      i++, decodeTablePTR++)
    {
      if ((raw_code & decodeTablePTR->operMask)
          == decodeTablePTR->opcodePTR->bin_opcode)
        {
          /* Make sure we didn't run into a macro or alias.  */
          if (decodeTablePTR->opcodePTR->cycles_min != 0)
            {
              return decodeTablePTR;
              break;
            }
          else
	    continue;
        }
    }
  return 0;
}
