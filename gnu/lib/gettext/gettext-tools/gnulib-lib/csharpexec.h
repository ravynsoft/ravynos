/* Execute a C# program.
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

#ifndef _CSHARPEXEC_H
#define _CSHARPEXEC_H

typedef bool execute_fn (const char *progname,
                         const char *prog_path, const char * const *prog_argv,
                         void *private_data);

/* Execute a C# program.
   assembly_path is the assembly's pathname (= program name with .exe).
   libdirs is a list of directories to be searched for libraries.
   args is a NULL terminated list of arguments to be passed to the program.
   If verbose, the command to be executed will be printed.
   Then the command is passed to the execute function together with the
   private_data argument.  This function returns false if OK, true on error.
   Return false if OK, true on error.
   If quiet, error messages will not be printed.  */
extern bool execute_csharp_program (const char *assembly_path,
                                    const char * const *libdirs,
                                    unsigned int libdirs_count,
                                    const char * const *args,
                                    bool verbose, bool quiet,
                                    execute_fn *executer, void *private_data);

#endif /* _CSHARPEXEC_H */
