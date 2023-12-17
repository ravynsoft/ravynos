/* Compile a C# program.
   Copyright (C) 2003, 2009-2023 Free Software Foundation, Inc.
   Written by Bruno Haible <bruno@clisp.org>, 2003.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

#ifndef _CSHARPCOMP_H
#define _CSHARPCOMP_H

/* Compile a set of C# source files to bytecode.
   sources is an array of source file names, including resource files.
   libdirs is a list of directories to be searched for libraries.
   libraries is a list of libraries on which the program depends.
   output_file is the name of the output file; it should end in .exe or .dll.
   If verbose, the command to be executed will be printed.
   Return false if OK, true on error.  */
extern bool compile_csharp_class (const char * const *sources,
                                  unsigned int sources_count,
                                  const char * const *libdirs,
                                  unsigned int libdirs_count,
                                  const char * const *libraries,
                                  unsigned int libraries_count,
                                  const char *output_file,
                                  bool optimize, bool debug,
                                  bool verbose);

#endif /* _CSHARPCOMP_H */
