/* Determine the Java version supported by javaexec.
   Copyright (C) 2006-2023 Free Software Foundation, Inc.
   Written by Bruno Haible <bruno@clisp.org>, 2006.

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

/* Specification.  */
#include "javaversion.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#if ENABLE_RELOCATABLE
# include "relocatable.h"
#else
# define relocate(pathname) (pathname)
# define relocate2(pathname,allocatedp) (*(allocatedp) = NULL, (pathname))
#endif

#include "javaexec.h"
#include "spawn-pipe.h"
#include "wait-process.h"
#include "error.h"
#include "gettext.h"

#define _(str) gettext (str)

/* Get PKGDATADIR.  */
#include "configmake.h"


struct locals
{
  /* OUT */
  char *line;
};

static bool
execute_and_read_line (const char *progname,
                       const char *prog_path, const char * const *prog_argv,
                       void *private_data)
{
  struct locals *l = (struct locals *) private_data;
  pid_t child;
  int fd[1];
  FILE *fp;
  char *line;
  size_t linesize;
  size_t linelen;
  int exitstatus;

  /* Open a pipe to the JVM.  */
  child = create_pipe_in (progname, prog_path, prog_argv, NULL,
                          DEV_NULL, false, true, false, fd);

  if (child == -1)
    return false;

  /* Retrieve its result.  */
  fp = fdopen (fd[0], "r");
  if (fp == NULL)
    {
      error (0, errno, _("fdopen() failed"));
      return false;
    }

  line = NULL; linesize = 0;
  linelen = getline (&line, &linesize, fp);
  if (linelen == (size_t)(-1))
    {
      error (0, 0, _("%s subprocess I/O error"), progname);
      return false;
    }
  if (linelen > 0 && line[linelen - 1] == '\n')
    line[linelen - 1] = '\0';

  fclose (fp);

  /* Remove zombie process from process list, and retrieve exit status.  */
  exitstatus =
    wait_subprocess (child, progname, true, false, true, false, NULL);
  if (exitstatus != 0)
    {
      free (line);
      return false;
    }

  l->line = line;
  return false;
}

char *
javaexec_version (void)
{
  const char *class_name = "javaversion";
  char *malloc_pkgdatadir;
  const char *pkgdatadir = relocate2 (PKGDATADIR, &malloc_pkgdatadir);
  const char *args[1];
  struct locals locals;

  args[0] = NULL;
  locals.line = NULL;
  execute_java_class (class_name, &pkgdatadir, 1, true, NULL, args,
                      false, false, execute_and_read_line, &locals);

  free (malloc_pkgdatadir);
  return locals.line;
}
