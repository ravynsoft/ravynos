/* RISC-V spec version controlling support.
   Copyright (C) 2019-2023 Free Software Foundation, Inc.

   This file is part of BFD, the Binary File Descriptor library.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

enum riscv_spec_class
{
  /* ISA spec.  */
  ISA_SPEC_CLASS_NONE = 0,
  ISA_SPEC_CLASS_2P2,
  ISA_SPEC_CLASS_20190608,
  ISA_SPEC_CLASS_20191213,
  ISA_SPEC_CLASS_DRAFT,

  /* Privileged spec.  */
  PRIV_SPEC_CLASS_NONE,
  PRIV_SPEC_CLASS_1P9P1,
  PRIV_SPEC_CLASS_1P10,
  PRIV_SPEC_CLASS_1P11,
  PRIV_SPEC_CLASS_1P12,
  PRIV_SPEC_CLASS_DRAFT,
};

struct riscv_spec
{
  const char *name;
  enum riscv_spec_class spec_class;
};

extern const struct riscv_spec riscv_isa_specs[];
extern const struct riscv_spec riscv_priv_specs[];

#define RISCV_GET_SPEC_CLASS(UTYPE, LTYPE, NAME, CLASS)			\
  do									\
    {									\
      if (NAME == NULL)							\
	break;								\
									\
      int i_spec = UTYPE##_SPEC_CLASS_NONE + 1;				\
      for (; i_spec < UTYPE##_SPEC_CLASS_DRAFT; i_spec++)		\
	{								\
	  int j_spec = i_spec - UTYPE##_SPEC_CLASS_NONE -1;		\
	  if (riscv_##LTYPE##_specs[j_spec].name			\
	      && strcmp (riscv_##LTYPE##_specs[j_spec].name, NAME) == 0)\
	  {								\
	    CLASS = riscv_##LTYPE##_specs[j_spec].spec_class;		\
	    break;							\
	  }								\
	}								\
    }									\
  while (0)

#define RISCV_GET_SPEC_NAME(UTYPE, LTYPE, NAME, CLASS)			\
  (NAME) = riscv_##LTYPE##_specs[(CLASS) - UTYPE##_SPEC_CLASS_NONE - 1].name

#define RISCV_GET_ISA_SPEC_CLASS(NAME, CLASS)	\
  RISCV_GET_SPEC_CLASS(ISA, isa, NAME, CLASS)
#define RISCV_GET_PRIV_SPEC_CLASS(NAME, CLASS)	\
  RISCV_GET_SPEC_CLASS(PRIV, priv, NAME, CLASS)
#define RISCV_GET_PRIV_SPEC_NAME(NAME, CLASS)	\
  RISCV_GET_SPEC_NAME(PRIV, priv, NAME, CLASS)

extern void
riscv_get_priv_spec_class_from_numbers (unsigned int,
					unsigned int,
					unsigned int,
					enum riscv_spec_class *);

extern bool
riscv_elf_is_mapping_symbols (const char *);
