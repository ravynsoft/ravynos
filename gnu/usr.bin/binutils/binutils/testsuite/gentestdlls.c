/* Copyright (C) 2019-2023 Free Software Foundation, Inc.

   This file is part of GNU Binutils.

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
   Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston, MA
   02110-1301, USA.  */


/* This file generates a number of DLL (PE/COFF binaries traditionally
   used on Windows) that we can then utilize in various tests to
   ensure objdump can parse these file correctly.

   See:
   https://www.ecma-international.org/publications/files/ECMA-ST/ECMA-335.pdf  */

#include <memory.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define INCORRECT_USAGE 2
#define IO_ERROR 3

static void
write_dos_header_and_stub (FILE* file)
{
  /* See ECMA-335 II.25.2.1.
     Instead of lfanew, lets just hardcode the offset of the next byte
     after this header (0x80).  */
  char buffer[128] =
    {
     0x4d, 0x5a, 0x90, 0x00, 0x03, 0x00, 0x00, 0x00,
     0x04, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00,
     0xb8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, /* Last 4 bytes are precomputed lfanew.  */
     0x0e, 0x1f, 0xba, 0x0e, 0x00, 0xb4, 0x09, 0xcd,
     0x21, 0xb8, 0x01, 0x4c, 0xcd, 0x21, 0x54, 0x68,
     0x69, 0x73, 0x20, 0x70, 0x72, 0x6f, 0x67, 0x72,
     0x61, 0x6d, 0x20, 0x63, 0x61, 0x6e, 0x6e, 0x6f,
     0x74, 0x20, 0x62, 0x65, 0x20, 0x72, 0x75, 0x6e,
     0x20, 0x69, 0x6e, 0x20, 0x44, 0x4f, 0x53, 0x20,
     0x6d, 0x6f, 0x64, 0x65, 0x2e, 0x0d, 0x0d, 0x0a,
     0x24, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };

  fwrite (buffer, 1, 128, file);
}

static void
write_pe_signature (FILE* file)
{
  char buffer[4];

  buffer[0] = 'P';
  buffer[1] = 'E';
  buffer[2] = 0;
  buffer[3] = 0;
  fwrite (buffer, 1, 4, file);
}

static void
write_coff_header (FILE* file, uint16_t machine)
{
  char buffer[128];

  memset (buffer, 0, sizeof (buffer));

  /* Machine.  ECMA-335 says this must be 0x14c but that's not true anymore.  */
  buffer[0] = machine & 0xff;
  buffer[1] = machine >> 0x8;
  fwrite (buffer, 2, 1, file);
  memset (buffer, 0, sizeof (buffer));
  /* NumberOfSections = 0.  */
  fwrite (buffer, 2, 1, file);
  /* TimeDateStamp = 0.  */
  fwrite (buffer, 4, 1, file);
  /* PointerToSymbolTable = 0.  */
  fwrite (buffer, 4, 1, file);
  /* NumberOfSymbols = 0.  */
  fwrite (buffer, 4, 1, file);
  /* OptionalHeaderSize = 0.  */
  fwrite (buffer, 2, 1, file);
  /* Characteristics = 0x2000.  */
  buffer[0] = 0x00;
  buffer[1] = 0x20;
  fwrite (buffer, 2, 1, file);
  memset (buffer, 0 , sizeof (buffer));
}

static void
write_simple_dll (const char* name, uint16_t machine)
{
  FILE* file = fopen (name, "w");

  if (file == NULL)
    {
      fprintf (stderr, "error: unable to open file for writing\n");
      exit (IO_ERROR);
    }

  write_dos_header_and_stub (file);
  write_pe_signature (file);
  write_coff_header (file, machine);
  fclose (file);
  file = NULL;
  printf ("wrote %s\n", name);
}

int
main (int argc, char** argv)
{
  char* program_name = argv[0];
  char* output_directory = argv[1];
  int i;

  if (argc < 3)
    {
      fprintf (stderr, "usage: %s output-directory format [format ...] \n\n", program_name);
      fprintf (stderr, "format is an objdump-style format string, like pei-i386\n");
      exit (INCORRECT_USAGE);
    }

  if (chdir (output_directory) != 0)
    {
      fprintf (stderr, "error: unable to change directory to %s\n", output_directory);
      exit (INCORRECT_USAGE);
    }

  /* We generate a simple PEI format files, and then .NET Core on
     Linux-style PEI files for a number of architectures.  As opposed
     to the more common PEI files that contain bytecode (CIL/MSIL), many
     .NET Core DLLs are pre-compiled for specific architectures and
     platforms.  See https://github.com/jbevain/cecil/issues/337 for an
     example of this value being used in practice.  */

  for (i = 2; i < argc; i++)
    {
      char* wanted_format = argv[i];

      if (strcmp ("pei-i386", wanted_format) == 0)
        {
          write_simple_dll ("simple-pei-i386.dll", 0x14c);

          write_simple_dll ("linux-pei-i386.dll", 0x14c ^ 0x7b79 /* i386 + Linux */);
        }
      else if (strcmp ("pei-x86-64", wanted_format) == 0)
        {
          write_simple_dll ("simple-pei-x86-64.dll", 0x8664);

          write_simple_dll ("linux-pei-x86-64.dll", 0x8664 ^ 0x7b79 /* x86-64 + Linux */);
        }
      else
        {
          fprintf (stderr, "error: can't handle format %s\n", wanted_format);
          exit (INCORRECT_USAGE);
        }
    }

  return 0;
}
