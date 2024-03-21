/* objdump.h
   Copyright (C) 2011-2023 Free Software Foundation, Inc.

   This file is part of GNU Binutils.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

/* Non-zero if wide output has been enabled.  */
extern int wide_output;	

struct objdump_private_option
{
  /* Option name.  */
  const char *name;

  /* TRUE if the option is selected.  Automatically set and cleared by
     objdump.  */
  unsigned int selected;
};

struct objdump_private_desc
{
  /* Help displayed for --help.  */
  void (*help)(FILE *stream);

  /* Return TRUE if these options can be applied to ABFD.  */
  int (*filter)(bfd *abfd);

  /* Do the actual work: display whatever is requested according to the
     options whose SELECTED field is set.  */
  void (*dump)(bfd *abfd);

  /* List of options.  Terminated by a NULL name.  */
  struct objdump_private_option *options;
};

/* ELF32_AVR specific target.  */
extern const struct objdump_private_desc objdump_private_desc_elf32_avr;

/* XCOFF specific target.  */
extern const struct objdump_private_desc objdump_private_desc_xcoff;

/* PE specific target.  */
extern const struct objdump_private_desc objdump_private_desc_pe;

/* Mach-O specific target.  */
extern const struct objdump_private_desc objdump_private_desc_mach_o;
