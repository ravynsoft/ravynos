/* Compile a C# program.
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
#include "csharpcomp.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "execute.h"
#include "spawn-pipe.h"
#include "wait-process.h"
#include "sh-quote.h"
#include "safe-read.h"
#include "xmalloca.h"
#include "error.h"
#include "gettext.h"

#define _(str) gettext (str)


/* Survey of C# compilers.

   Program    from

   mcs        mono
   csc        sscli

   We try the CIL interpreters in the following order:
     1. "mcs", because it is a free system but doesn't integrate so well
        with Unix. (Command line options start with / instead of -. Errors go
        to stdout instead of stderr. Source references are printed as
        "file(lineno)" instead of "file:lineno:".)
     2. "csc", although it is not free, because it is a kind of "reference
        implementation" of C#.
   But the order can be changed through the --enable-csharp configuration
   option.
 */

static int
compile_csharp_using_mono (const char * const *sources,
                           unsigned int sources_count,
                           const char * const *libdirs,
                           unsigned int libdirs_count,
                           const char * const *libraries,
                           unsigned int libraries_count,
                           const char *output_file, bool output_is_library,
                           bool optimize, bool debug,
                           bool verbose)
{
  static bool mcs_tested;
  static bool mcs_present;

  if (!mcs_tested)
    {
      /* Test for presence of mcs:
         "mcs --version >/dev/null 2>/dev/null"
         and (to exclude an unrelated 'mcs' program on QNX 6)
         "mcs --version 2>/dev/null | grep Mono >/dev/null"  */
      const char *argv[3];
      pid_t child;
      int fd[1];
      int exitstatus;

      argv[0] = "mcs";
      argv[1] = "--version";
      argv[2] = NULL;
      child = create_pipe_in ("mcs", "mcs", argv, NULL,
                              DEV_NULL, true, true, false, fd);
      mcs_present = false;
      if (child != -1)
        {
          /* Read the subprocess output, and test whether it contains the
             string "Mono".  */
          char c[4];
          size_t count = 0;

          while (safe_read (fd[0], &c[count], 1) > 0)
            {
              count++;
              if (count == 4)
                {
                  if (memcmp (c, "Mono", 4) == 0)
                    mcs_present = true;
                  c[0] = c[1]; c[1] = c[2]; c[2] = c[3];
                  count--;
                }
            }

          close (fd[0]);

          /* Remove zombie process from process list, and retrieve exit
             status.  */
          exitstatus =
            wait_subprocess (child, "mcs", false, true, true, false, NULL);
          if (exitstatus != 0)
            mcs_present = false;
        }
      mcs_tested = true;
    }

  if (mcs_present)
    {
      unsigned int argc;
      const char **argv;
      const char **argp;
      pid_t child;
      int fd[1];
      FILE *fp;
      char *line[2];
      size_t linesize[2];
      size_t linelen[2];
      unsigned int l;
      int exitstatus;
      unsigned int i;

      argc =
        1 + (output_is_library ? 1 : 0) + 1 + libdirs_count + libraries_count
        + (debug ? 1 : 0) + sources_count;
      argv = (const char **) xmalloca ((argc + 1) * sizeof (const char *));

      argp = argv;
      *argp++ = "mcs";
      if (output_is_library)
        *argp++ = "-target:library";
      {
        char *option = (char *) xmalloca (5 + strlen (output_file) + 1);
        memcpy (option, "-out:", 5);
        strcpy (option + 5, output_file);
        *argp++ = option;
      }
      for (i = 0; i < libdirs_count; i++)
        {
          char *option = (char *) xmalloca (5 + strlen (libdirs[i]) + 1);
          memcpy (option, "-lib:", 5);
          strcpy (option + 5, libdirs[i]);
          *argp++ = option;
        }
      for (i = 0; i < libraries_count; i++)
        {
          char *option = (char *) xmalloca (11 + strlen (libraries[i]) + 4 + 1);
          memcpy (option, "-reference:", 11);
          memcpy (option + 11, libraries[i], strlen (libraries[i]));
          strcpy (option + 11 + strlen (libraries[i]), ".dll");
          *argp++ = option;
        }
      if (debug)
        *argp++ = "-debug";
      for (i = 0; i < sources_count; i++)
        {
          const char *source_file = sources[i];
          if (strlen (source_file) >= 10
              && memcmp (source_file + strlen (source_file) - 10, ".resources",
                         10) == 0)
            {
              char *option = (char *) xmalloca (10 + strlen (source_file) + 1);

              memcpy (option, "-resource:", 10);
              strcpy (option + 10, source_file);
              *argp++ = option;
            }
          else
            *argp++ = source_file;
        }
      *argp = NULL;
      /* Ensure argv length was correctly calculated.  */
      if (argp - argv != argc)
        abort ();

      if (verbose)
        {
          char *command = shell_quote_argv (argv);
          printf ("%s\n", command);
          free (command);
        }

      child = create_pipe_in ("mcs", "mcs", argv, NULL,
                              NULL, false, true, true, fd);

      /* Read the subprocess output, copying it to stderr.  Drop the last
         line if it starts with "Compilation succeeded".  */
      fp = fdopen (fd[0], "r");
      if (fp == NULL)
        error (EXIT_FAILURE, errno, _("fdopen() failed"));
      line[0] = NULL; linesize[0] = 0;
      line[1] = NULL; linesize[1] = 0;
      l = 0;
      for (;;)
        {
          linelen[l] = getline (&line[l], &linesize[l], fp);
          if (linelen[l] == (size_t)(-1))
            break;
          l = (l + 1) % 2;
          if (line[l] != NULL)
            fwrite (line[l], 1, linelen[l], stderr);
        }
      l = (l + 1) % 2;
      if (line[l] != NULL
          && !(linelen[l] >= 21
               && memcmp (line[l], "Compilation succeeded", 21) == 0))
        fwrite (line[l], 1, linelen[l], stderr);
      if (line[0] != NULL)
        free (line[0]);
      if (line[1] != NULL)
        free (line[1]);
      fclose (fp);

      /* Remove zombie process from process list, and retrieve exit status.  */
      exitstatus =
        wait_subprocess (child, "mcs", false, false, true, true, NULL);

      for (i = 1 + (output_is_library ? 1 : 0);
           i < 1 + (output_is_library ? 1 : 0)
               + 1 + libdirs_count + libraries_count;
           i++)
        freea ((char *) argv[i]);
      for (i = 0; i < sources_count; i++)
        if (argv[argc - sources_count + i] != sources[i])
          freea ((char *) argv[argc - sources_count + i]);
      freea (argv);

      return (exitstatus != 0);
    }
  else
    return -1;
}

static int
compile_csharp_using_sscli (const char * const *sources,
                            unsigned int sources_count,
                            const char * const *libdirs,
                            unsigned int libdirs_count,
                            const char * const *libraries,
                            unsigned int libraries_count,
                            const char *output_file, bool output_is_library,
                            bool optimize, bool debug,
                            bool verbose)
{
  static bool csc_tested;
  static bool csc_present;

  if (!csc_tested)
    {
      /* Test for presence of csc:
         "csc -help >/dev/null 2>/dev/null \
          && ! { csc -help 2>/dev/null | grep -i chicken > /dev/null; }"  */
      const char *argv[3];
      pid_t child;
      int fd[1];
      int exitstatus;

      argv[0] = "csc";
      argv[1] = "-help";
      argv[2] = NULL;
      child = create_pipe_in ("csc", "csc", argv, NULL,
                              DEV_NULL, true, true, false, fd);
      csc_present = false;
      if (child != -1)
        {
          /* Read the subprocess output, and test whether it contains the
             string "chicken".  */
          char c[7];
          size_t count = 0;

          csc_present = true;
          while (safe_read (fd[0], &c[count], 1) > 0)
            {
              if (c[count] >= 'A' && c[count] <= 'Z')
                c[count] += 'a' - 'A';
              count++;
              if (count == 7)
                {
                  if (memcmp (c, "chicken", 7) == 0)
                    csc_present = false;
                  c[0] = c[1]; c[1] = c[2]; c[2] = c[3];
                  c[3] = c[4]; c[4] = c[5]; c[5] = c[6];
                  count--;
                }
            }

          close (fd[0]);

          /* Remove zombie process from process list, and retrieve exit
             status.  */
          exitstatus =
            wait_subprocess (child, "csc", false, true, true, false, NULL);
          if (exitstatus != 0)
            csc_present = false;
        }
      csc_tested = true;
    }

  if (csc_present)
    {
      unsigned int argc;
      const char **argv;
      const char **argp;
      int exitstatus;
      unsigned int i;

      argc =
        1 + 1 + 1 + libdirs_count + libraries_count
        + (optimize ? 1 : 0) + (debug ? 1 : 0) + sources_count;
      argv = (const char **) xmalloca ((argc + 1) * sizeof (const char *));

      argp = argv;
      *argp++ = "csc";
      *argp++ = (output_is_library ? "-target:library" : "-target:exe");
      {
        char *option = (char *) xmalloca (5 + strlen (output_file) + 1);
        memcpy (option, "-out:", 5);
        strcpy (option + 5, output_file);
        *argp++ = option;
      }
      for (i = 0; i < libdirs_count; i++)
        {
          char *option = (char *) xmalloca (5 + strlen (libdirs[i]) + 1);
          memcpy (option, "-lib:", 5);
          strcpy (option + 5, libdirs[i]);
          *argp++ = option;
        }
      for (i = 0; i < libraries_count; i++)
        {
          char *option = (char *) xmalloca (11 + strlen (libraries[i]) + 4 + 1);
          memcpy (option, "-reference:", 11);
          memcpy (option + 11, libraries[i], strlen (libraries[i]));
          strcpy (option + 11 + strlen (libraries[i]), ".dll");
          *argp++ = option;
        }
      if (optimize)
        *argp++ = "-optimize+";
      if (debug)
        *argp++ = "-debug+";
      for (i = 0; i < sources_count; i++)
        {
          const char *source_file = sources[i];
          if (strlen (source_file) >= 10
              && memcmp (source_file + strlen (source_file) - 10, ".resources",
                         10) == 0)
            {
              char *option = (char *) xmalloca (10 + strlen (source_file) + 1);

              memcpy (option, "-resource:", 10);
              strcpy (option + 10, source_file);
              *argp++ = option;
            }
          else
            *argp++ = source_file;
        }
      *argp = NULL;
      /* Ensure argv length was correctly calculated.  */
      if (argp - argv != argc)
        abort ();

      if (verbose)
        {
          char *command = shell_quote_argv (argv);
          printf ("%s\n", command);
          free (command);
        }

      exitstatus = execute ("csc", "csc", argv, NULL,
                            false, false, false, false,
                            true, true, NULL);

      for (i = 2; i < 3 + libdirs_count + libraries_count; i++)
        freea ((char *) argv[i]);
      for (i = 0; i < sources_count; i++)
        if (argv[argc - sources_count + i] != sources[i])
          freea ((char *) argv[argc - sources_count + i]);
      freea (argv);

      return (exitstatus != 0);
    }
  else
    return -1;
}

bool
compile_csharp_class (const char * const *sources,
                      unsigned int sources_count,
                      const char * const *libdirs,
                      unsigned int libdirs_count,
                      const char * const *libraries,
                      unsigned int libraries_count,
                      const char *output_file,
                      bool optimize, bool debug,
                      bool verbose)
{
  bool output_is_library =
    (strlen (output_file) >= 4
     && memcmp (output_file + strlen (output_file) - 4, ".dll", 4) == 0);
  int result;

  /* First try the C# implementation specified through --enable-csharp.  */
#if CSHARP_CHOICE_MONO
  result = compile_csharp_using_mono (sources, sources_count,
                                      libdirs, libdirs_count,
                                      libraries, libraries_count,
                                      output_file, output_is_library,
                                      optimize, debug, verbose);
  if (result >= 0)
    return (bool) result;
#endif

  /* Then try the remaining C# implementations in our standard order.  */
#if !CSHARP_CHOICE_MONO
  result = compile_csharp_using_mono (sources, sources_count,
                                      libdirs, libdirs_count,
                                      libraries, libraries_count,
                                      output_file, output_is_library,
                                      optimize, debug, verbose);
  if (result >= 0)
    return (bool) result;
#endif

  result = compile_csharp_using_sscli (sources, sources_count,
                                       libdirs, libdirs_count,
                                       libraries, libraries_count,
                                       output_file, output_is_library,
                                       optimize, debug, verbose);
  if (result >= 0)
    return (bool) result;

  error (0, 0, _("C# compiler not found, try installing mono"));
  return true;
}
