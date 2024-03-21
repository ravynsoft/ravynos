/* Execute a Java program.
   Copyright (C) 2001-2002, 2009-2023 Free Software Foundation, Inc.
   Written by Bruno Haible <haible@clisp.cons.org>, 2001.

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

#ifndef _JAVAEXEC_H
#define _JAVAEXEC_H

typedef bool execute_fn (const char *progname,
                         const char *prog_path, const char * const *prog_argv,
                         void *private_data);

/* Execute a Java class.
   class_name is the Java class name to be executed.
   classpaths is a list of pathnames to be prepended to the CLASSPATH.
   use_minimal_classpath = true means to ignore the user's CLASSPATH and
   use a minimal one. This is likely to reduce possible problems if the
   user's CLASSPATH contains garbage or a classes.zip file of the wrong
   Java version.
   exe_dir is a directory that may contain a native executable for the class.
   args is a NULL terminated list of arguments to be passed to the program.
   If verbose, the command to be executed will be printed.
   Then the command is passed to the execute function together with the
   private_data argument.  This function returns false if OK, true on error.
   Return false if OK, true on error.
   If quiet, error messages will not be printed.  */
extern bool execute_java_class (const char *class_name,
                                const char * const *classpaths,
                                unsigned int classpaths_count,
                                bool use_minimal_classpath,
                                const char *exe_dir,
                                const char * const *args,
                                bool verbose, bool quiet,
                                execute_fn *executer, void *private_data);

#endif /* _JAVAEXEC_H */
