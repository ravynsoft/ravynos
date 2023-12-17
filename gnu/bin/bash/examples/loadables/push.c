/*
 * push - anyone remember TOPS-20?
 *
 */

/*
   Copyright (C) 1999-2020 Free Software Foundation, Inc.

   This file is part of GNU Bash.
   Bash is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Bash is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Bash.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <config.h>
#include <stdio.h>
#include <errno.h>

#include "builtins.h"
#include "shell.h"
#include "jobs.h"
#include "bashgetopt.h"
#include "common.h"

#ifndef errno
extern int errno;
#endif

extern pid_t dollar_dollar_pid;
extern int last_command_exit_value;

int
push_builtin (list)
     WORD_LIST *list;
{
  pid_t pid;
  int xstatus, opt;

  xstatus = EXECUTION_SUCCESS;
  reset_internal_getopt ();
  while ((opt = internal_getopt (list, "")) != -1)
    {
      switch (opt)
	{
	CASE_HELPOPT;
	default:
	  builtin_usage ();
	  return (EX_USAGE);
	}
    }
  list = loptend;  

  pid = make_child (savestring ("push"), 0);
  if (pid == -1)
    {
      builtin_error ("cannot fork: %s", strerror (errno));
      return (EXECUTION_FAILURE);
    }
  else if (pid == 0)
    {
      /* Shell variable adjustments: $SHLVL, $$, $PPID, $! */
      adjust_shell_level (1);
      dollar_dollar_pid = getpid ();
      set_ppid ();

      /* Clean up job control stuff. */
      stop_making_children ();
      cleanup_the_pipeline ();
      delete_all_jobs (0);

      last_asynchronous_pid = NO_PID;

      /* Make sure the job control code has the right values for
	 the shell's process group and tty process group, and that
	 the signals are set correctly for job control. */
      initialize_job_control (0);
      initialize_job_signals ();

      /* And read commands until exit. */
      reader_loop ();
      exit_shell (last_command_exit_value);
    }
  else
    {
      stop_pipeline (0, (COMMAND *)NULL);
      xstatus = wait_for (pid, 0);
      return (xstatus);
    }   
}

char *push_doc[] = {
	"Create child shell.",
	"",
	"Create a child that is an exact duplicate of the running shell",
	"and wait for it to exit.  The $SHLVL, $!, $$, and $PPID variables",
	"are adjusted in the child.  The return value is the exit status",
	"of the child.",
	(char *)NULL
};

struct builtin push_struct = {
	"push",
	push_builtin,
	BUILTIN_ENABLED,
	push_doc,
	"push",
	0
};
