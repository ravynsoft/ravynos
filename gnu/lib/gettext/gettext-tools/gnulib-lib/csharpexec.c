/* Execute a C# program.
   Copyright (C) 2003-2023 Free Software Foundation, Inc.
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

#include <config.h>
#include <alloca.h>

/* Specification.  */
#include "csharpexec.h"

#include <stdio.h>
#include <stdlib.h>

#include "execute.h"
#include "sh-quote.h"
#include "xmalloca.h"
#include "error.h"
#include "gettext.h"

/* Handling of MONO_PATH is just like Java CLASSPATH.  */
#define CLASSPATHVAR "MONO_PATH"
#define new_classpath new_monopath
#define set_classpath set_monopath
#define reset_classpath reset_monopath
#include "classpath.h"
#include "classpath.c"
#undef reset_classpath
#undef set_classpath
#undef new_classpath
#undef CLASSPATHVAR

/* Handling of clix' PATH variable is just like Java CLASSPATH.  */
#if defined _WIN32 || defined __CYGWIN__
  /* Native Windows, Cygwin */
  #define CLASSPATHVAR "PATH"
#elif defined __APPLE__ && defined __MACH__
  /* Mac OS X */
  #define CLASSPATHVAR "DYLD_LIBRARY_PATH"
#else
  /* Normal Unix */
  #define CLASSPATHVAR "LD_LIBRARY_PATH"
#endif
#define new_classpath new_clixpath
#define set_classpath set_clixpath
#define reset_classpath reset_clixpath
#include "classpath.h"
#include "classpath.c"
#undef reset_classpath
#undef set_classpath
#undef new_classpath
#undef CLASSPATHVAR

#define _(str) gettext (str)


/* Survey of CIL interpreters.

   Program    from

   mono       mono
   clix       sscli

   With Mono, the MONO_PATH is a colon separated list of pathnames. (On
   Windows: semicolon separated list of pathnames.)

   We try the CIL interpreters in the following order:
     1. "mono", because it is a partially free system but doesn't integrate
        well with Unix.
     2. "clix", although it is not free, because it is a kind of "reference
        implementation" of C#.
   But the order can be changed through the --enable-csharp configuration
   option.
 */

static int
execute_csharp_using_mono (const char *assembly_path,
                           const char * const *libdirs,
                           unsigned int libdirs_count,
                           const char * const *args, unsigned int nargs,
                           bool verbose, bool quiet,
                           execute_fn *executer, void *private_data)
{
  static bool mono_tested;
  static bool mono_present;

  if (!mono_tested)
    {
      /* Test for presence of mono:
         "mono --version >/dev/null 2>/dev/null"  */
      const char *argv[3];
      int exitstatus;

      argv[0] = "mono";
      argv[1] = "--version";
      argv[2] = NULL;
      exitstatus = execute ("mono", "mono", argv, NULL,
                            false, false, true, true,
                            true, false, NULL);
      mono_present = (exitstatus == 0);
      mono_tested = true;
    }

  if (mono_present)
    {
      char *old_monopath;
      const char **argv =
        (const char **) xmalloca ((2 + nargs + 1) * sizeof (const char *));
      unsigned int i;
      bool err;

      /* Set MONO_PATH.  */
      old_monopath = set_monopath (libdirs, libdirs_count, false, verbose);

      argv[0] = "mono";
      argv[1] = assembly_path;
      for (i = 0; i <= nargs; i++)
        argv[2 + i] = args[i];

      if (verbose)
        {
          char *command = shell_quote_argv (argv);
          printf ("%s\n", command);
          free (command);
        }

      err = executer ("mono", "mono", argv, private_data);

      /* Reset MONO_PATH.  */
      reset_monopath (old_monopath);

      freea (argv);

      return err;
    }
  else
    return -1;
}

static int
execute_csharp_using_sscli (const char *assembly_path,
                            const char * const *libdirs,
                            unsigned int libdirs_count,
                            const char * const *args, unsigned int nargs,
                            bool verbose, bool quiet,
                            execute_fn *executer, void *private_data)
{
  static bool clix_tested;
  static bool clix_present;

  if (!clix_tested)
    {
      /* Test for presence of clix:
         "clix >/dev/null 2>/dev/null ; test $? = 1"  */
      const char *argv[2];
      int exitstatus;

      argv[0] = "clix";
      argv[1] = NULL;
      exitstatus = execute ("clix", "clix", argv, NULL,
                            false, false, true, true,
                            true, false, NULL);
      clix_present = (exitstatus == 0 || exitstatus == 1);
      clix_tested = true;
    }

  if (clix_present)
    {
      char *old_clixpath;
      const char **argv =
        (const char **) xmalloca ((2 + nargs + 1) * sizeof (const char *));
      unsigned int i;
      bool err;

      /* Set clix' PATH variable.  */
      old_clixpath = set_clixpath (libdirs, libdirs_count, false, verbose);

      argv[0] = "clix";
      argv[1] = assembly_path;
      for (i = 0; i <= nargs; i++)
        argv[2 + i] = args[i];

      if (verbose)
        {
          char *command = shell_quote_argv (argv);
          printf ("%s\n", command);
          free (command);
        }

      err = executer ("clix", "clix", argv, private_data);

      /* Reset clix' PATH variable.  */
      reset_clixpath (old_clixpath);

      freea (argv);

      return err;
    }
  else
    return -1;
}

bool
execute_csharp_program (const char *assembly_path,
                        const char * const *libdirs,
                        unsigned int libdirs_count,
                        const char * const *args,
                        bool verbose, bool quiet,
                        execute_fn *executer, void *private_data)
{
  unsigned int nargs;
  int result;

  /* Count args.  */
  {
    const char * const *arg;

    for (nargs = 0, arg = args; *arg != NULL; nargs++, arg++)
     ;
  }

  /* First try the C# implementation specified through --enable-csharp.  */
#if CSHARP_CHOICE_MONO
  result = execute_csharp_using_mono (assembly_path, libdirs, libdirs_count,
                                      args, nargs, verbose, quiet,
                                      executer, private_data);
  if (result >= 0)
    return (bool) result;
#endif

  /* Then try the remaining C# implementations in our standard order.  */
#if !CSHARP_CHOICE_MONO
  result = execute_csharp_using_mono (assembly_path, libdirs, libdirs_count,
                                      args, nargs, verbose, quiet,
                                      executer, private_data);
  if (result >= 0)
    return (bool) result;
#endif

  result = execute_csharp_using_sscli (assembly_path, libdirs, libdirs_count,
                                       args, nargs, verbose, quiet,
                                       executer, private_data);
  if (result >= 0)
    return (bool) result;

  if (!quiet)
    error (0, 0, _("C# virtual machine not found, try installing mono"));
  return true;
}
