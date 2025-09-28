/* Disassembler code for Renesas RX.
   Copyright (C) 2008-2023 Free Software Foundation, Inc.
   Contributed by Red Hat.
   Written by DJ Delorie.

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
#include <stdio.h>

#include "bfd.h"
#include "dis-asm.h"
#include "opcode/rx.h"
#include "libiberty.h"
#include "opintl.h"

#include <setjmp.h>

typedef struct
{
  bfd_vma pc;
  disassemble_info * dis;
} RX_Data;

struct private
{
  OPCODES_SIGJMP_BUF bailout;
};

static int
rx_get_byte (void * vdata)
{
  bfd_byte buf[1];
  RX_Data *rx_data = (RX_Data *) vdata;
  int status;

  status = rx_data->dis->read_memory_func (rx_data->pc,
					   buf,
					   1,
					   rx_data->dis);
  if (status != 0)
    {
      struct private *priv = (struct private *) rx_data->dis->private_data;

      rx_data->dis->memory_error_func (status, rx_data->pc,
				       rx_data->dis);
       OPCODES_SIGLONGJMP (priv->bailout, 1);
    }

  rx_data->pc ++;
  return buf[0];
}

static char const * size_names[RX_MAX_SIZE] =
{
  "", ".b", ".ub", ".b", ".w", ".uw", ".w", ".a", ".l", "", "<error>"
};

static char const * opsize_names[RX_MAX_SIZE] =
{
  "", ".b", ".b", ".b", ".w", ".w", ".w", ".a", ".l", ".d", "<error>"
};

static char const * register_names[] =
{
  /* General registers.  */
  "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
  "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
  /* Control registers.  */
  "psw", "pc", "usp", "fpsw", NULL, NULL, NULL, NULL,
  "bpsw", "bpc", "isp", "fintv", "intb", "extb", NULL, NULL,
  "a0", "a1", NULL, NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
};

static char const * condition_names[] =
{
  /* Condition codes.  */
  "eq", "ne", "c", "nc", "gtu", "leu", "pz", "n",
  "ge", "lt", "gt", "le", "o", "no", "<invalid>", "<invalid>"
};

static const char * flag_names[] =
{
  "c", "z", "s", "o", "", "", "", "",
  "", "", "", "", "", "", "", "",
  "i", "u", "", "", "", "", "", "",
  "", "", "", "", "", "", "", ""
};

static const char * double_register_names[] =
{
  "dr0", "dr1", "dr2", "dr3", "dr4", "dr5", "dr6", "dr7",
  "dr8", "dr9", "dr10", "dr11", "dr12", "dr13", "dr14", "dr15"
};

static const char * double_register_high_names[] =
{
  "drh0", "drh1", "drh2", "drh3", "drh4", "drh5", "drh6", "drh7",
  "drh8", "drh9", "drh10", "drh11", "drh12", "drh13", "drh14", "drh15"
};

static const char * double_register_low_names[] =
{
  "drl0", "drl1", "drl2", "drl3", "drl4", "drl5", "drl6", "drl7",
  "drl8", "drl9", "drl10", "drl11", "drl12", "drl13", "drl14", "drl15"
};

static const char * double_control_register_names[] =
{
  "dpsw", "dcmr", "decnt", "depc"
};

static const char * double_condition_names[] =
{
  "", "un", "eq", "", "lt", "", "le"
};

static inline const char *
get_register_name (unsigned int reg)
{
  if (reg < ARRAY_SIZE (register_names))
    return register_names[reg];
  return _("<invalid register number>");
}

static inline const char *
get_condition_name (unsigned int cond)
{
  if (cond < ARRAY_SIZE (condition_names))
    return condition_names[cond];
  return _("<invalid condition code>");
}

static inline const char *
get_flag_name (unsigned int flag)
{
  if (flag < ARRAY_SIZE (flag_names))
    return flag_names[flag];
  return _("<invalid flag>");
}

static inline const char *
get_double_register_name (unsigned int reg)
{
  if (reg < ARRAY_SIZE (double_register_names))
    return double_register_names[reg];
  return _("<invalid register number>");
}

static inline const char *
get_double_register_high_name (unsigned int reg)
{
  if (reg < ARRAY_SIZE (double_register_high_names))
    return double_register_high_names[reg];
  return _("<invalid register number>");
}

static inline const char *
get_double_register_low_name (unsigned int reg)
{
  if (reg < ARRAY_SIZE (double_register_low_names))
    return double_register_low_names[reg];
  return _("<invalid register number>");
}

static inline const char *
get_double_control_register_name (unsigned int reg)
{
  if (reg < ARRAY_SIZE (double_control_register_names))
    return double_control_register_names[reg];
  return _("<invalid register number>");
}

static inline const char *
get_double_condition_name (unsigned int cond)
{
  if (cond < ARRAY_SIZE (double_condition_names))
    return double_condition_names[cond];
  return _("<invalid condition code>");
}

static inline const char *
get_opsize_name (unsigned int opsize)
{
  if (opsize < ARRAY_SIZE (opsize_names))
    return opsize_names[opsize];
  return _("<invalid opsize>");
}

static inline const char *
get_size_name (unsigned int size)
{
  if (size < ARRAY_SIZE (size_names))
    return size_names[size];
  return _("<invalid size>");
}


int
print_insn_rx (bfd_vma addr, disassemble_info * dis)
{
  int rv;
  RX_Data rx_data;
  RX_Opcode_Decoded opcode;
  const char * s;
  struct private priv;

  dis->private_data = &priv;
  rx_data.pc = addr;
  rx_data.dis = dis;

  if (OPCODES_SIGSETJMP (priv.bailout) != 0)
    {
      /* Error return.  */
      return -1;
    }

  rv = rx_decode_opcode (addr, &opcode, rx_get_byte, &rx_data);

  dis->bytes_per_line = 10;

#define PR (dis->fprintf_func)
#define PS (dis->stream)
#define PC(c) PR (PS, "%c", c)

  /* Detect illegal instructions.  */
  if (opcode.op[0].size == RX_Bad_Size
      || register_names [opcode.op[0].reg] == NULL
      || register_names [opcode.op[1].reg] == NULL
      || register_names [opcode.op[2].reg] == NULL)
    {
      bfd_byte buf[10];
      int i;

      PR (PS, ".byte ");
      rx_data.dis->read_memory_func (rx_data.pc - rv, buf, rv, rx_data.dis);
      
      for (i = 0 ; i < rv; i++)
	PR (PS, "0x%02x ", buf[i]);
      return rv;
    }
      
  for (s = opcode.syntax; *s; s++)
    {
      if (*s != '%')
	{
	  PC (*s);
	}
      else
	{
	  RX_Opcode_Operand * oper;
	  int do_size = 0;
	  int do_hex = 0;
	  int do_addr = 0;

	  s ++;

	  if (*s == 'S')
	    {
	      do_size = 1;
	      s++;
	    }
	  if (*s == 'x')
	    {
	      do_hex = 1;
	      s++;
	    }
	  if (*s == 'a')
	    {
	      do_addr = 1;
	      s++;
	    }

	  switch (*s)
	    {
	    case '%':
	      PC ('%');
	      break;

	    case 's':
	      PR (PS, "%s", get_opsize_name (opcode.size));
	      break;

	    case 'b':
	      s ++;
	      if (*s == 'f')
		{
		  int imm = opcode.op[2].addend;
		  int slsb, dlsb, width;

		  dlsb = (imm >> 5) & 0x1f;
		  slsb = (imm & 0x1f);
		  slsb = (slsb >= 0x10?(slsb ^ 0x1f) + 1:slsb);
		  slsb = dlsb - slsb;
		  slsb = (slsb < 0?-slsb:slsb);
		  width = ((imm >> 10) & 0x1f) - dlsb;
		  PR (PS, "#%d, #%d, #%d, %s, %s",
		      slsb, dlsb, width,
		      get_register_name (opcode.op[1].reg),
		      get_register_name (opcode.op[0].reg));
		}
	      break;
	    case '0':
	    case '1':
	    case '2':
	      oper = opcode.op + (*s - '0');
	      if (do_size)
		{
		  if (oper->type == RX_Operand_Indirect || oper->type == RX_Operand_Zero_Indirect)
		    PR (PS, "%s", get_size_name (oper->size));
		}
	      else
		switch (oper->type)
		  {
		  case RX_Operand_Immediate:
		    if (do_addr)
		      dis->print_address_func (oper->addend, dis);
		    else if (do_hex
			     || oper->addend > 999
			     || oper->addend < -999)
		      PR (PS, "%#x", oper->addend);
		    else
		      PR (PS, "%d", oper->addend);
		    break;
		  case RX_Operand_Register:
		  case RX_Operand_TwoReg:
		    PR (PS, "%s", get_register_name (oper->reg));
		    break;
		  case RX_Operand_Indirect:
		    PR (PS, "%d[%s]", oper->addend, get_register_name (oper->reg));
		    break;
		  case RX_Operand_Zero_Indirect:
		    PR (PS, "[%s]", get_register_name (oper->reg));
		    break;
		  case RX_Operand_Postinc:
		    PR (PS, "[%s+]", get_register_name (oper->reg));
		    break;
		  case RX_Operand_Predec:
		    PR (PS, "[-%s]", get_register_name (oper->reg));
		    break;
		  case RX_Operand_Condition:
		    PR (PS, "%s", get_condition_name (oper->reg));
		    break;
		  case RX_Operand_Flag:
		    PR (PS, "%s", get_flag_name (oper->reg));
		    break;
		  case RX_Operand_DoubleReg:
		    PR (PS, "%s", get_double_register_name (oper->reg));
		    break;
		  case RX_Operand_DoubleRegH:
		    PR (PS, "%s", get_double_register_high_name (oper->reg));
		    break;
		  case RX_Operand_DoubleRegL:
		    PR (PS, "%s", get_double_register_low_name (oper->reg));
		    break;
		  case RX_Operand_DoubleCReg:
		    PR (PS, "%s", get_double_control_register_name (oper->reg));
		    break;
		  case RX_Operand_DoubleCond:
		    PR (PS, "%s", get_double_condition_name (oper->reg));
		    break;
		  default:
		    PR (PS, "[???]");
		    break;
		  }
	    }
	}
    }

  return rv;
}
