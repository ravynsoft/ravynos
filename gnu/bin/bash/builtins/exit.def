This file is exit.def, from which is created exit.c.
It implements the builtins "exit", and "logout" in Bash.

Copyright (C) 1987-2021 Free Software Foundation, Inc.

This file is part of GNU Bash, the Bourne Again SHell.

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

$PRODUCES exit.c

$BUILTIN exit
$FUNCTION exit_builtin
$SHORT_DOC exit [n]
Exit the shell.

Exits the shell with a status of N.  If N is omitted, the exit status
is that of the last command executed.
$END

#include <config.h>

#include "../bashtypes.h"
#include <stdio.h>

#if defined (HAVE_UNISTD_H)
#  include <unistd.h>
#endif

#include "../bashintl.h"

#include "../shell.h"
#include "../execute_cmd.h"
#include "../jobs.h"
#include "../trap.h"

#include "common.h"
#include "builtext.h"	/* for jobs_builtin */

extern int check_jobs_at_exit;

static int exit_or_logout PARAMS((WORD_LIST *));
static int sourced_logout;

int
exit_builtin (list)
     WORD_LIST *list;
{
  CHECK_HELPOPT (list);

  if (interactive)
    {
      fprintf (stderr, login_shell ? _("logout\n") : "exit\n");
      fflush (stderr);
    }

  return (exit_or_logout (list));
}

$BUILTIN logout
$FUNCTION logout_builtin
$SHORT_DOC logout [n]
Exit a login shell.

Exits a login shell with exit status N.  Returns an error if not executed
in a login shell.
$END

/* How to logout. */
int
logout_builtin (list)
     WORD_LIST *list;
{
  CHECK_HELPOPT (list);

  if (login_shell == 0 /* && interactive */)
    {
      builtin_error (_("not login shell: use `exit'"));
      return (EXECUTION_FAILURE);
    }
  else
    return (exit_or_logout (list));
}

static int
exit_or_logout (list)
     WORD_LIST *list;
{
  int exit_value;

#if defined (JOB_CONTROL)
  int exit_immediate_okay, stopmsg;

  exit_immediate_okay = (interactive  == 0 ||
			 last_shell_builtin == exit_builtin ||
			 last_shell_builtin == logout_builtin ||
			 last_shell_builtin == jobs_builtin);

  /* Check for stopped jobs if the user wants to. */
  if (exit_immediate_okay == 0)
    {
      register int i;
      for (i = stopmsg = 0; i < js.j_jobslots; i++)
	if (jobs[i] && STOPPED (i))
	  stopmsg = JSTOPPED;
	else if (check_jobs_at_exit && stopmsg == 0 && jobs[i] && RUNNING (i))
	  stopmsg = JRUNNING;

      if (stopmsg == JSTOPPED)
	fprintf (stderr, _("There are stopped jobs.\n"));
      else if (stopmsg == JRUNNING)
	fprintf (stderr, _("There are running jobs.\n"));

      if (stopmsg && check_jobs_at_exit)
        list_all_jobs (JLIST_STANDARD);

      if (stopmsg)
	{
	  /* This is NOT superfluous because EOF can get here without
	     going through the command parser.  Set both last and this
	     so that either `exit', `logout', or ^D will work to exit
	     immediately if nothing intervenes. */
	  this_shell_builtin = last_shell_builtin = exit_builtin;
	  return (EXECUTION_FAILURE);
	}
    }
#endif /* JOB_CONTROL */

  /* Get return value if present.  This means that you can type
     `logout 5' to a shell, and it returns 5. */

  /* If we're running the exit trap (running_trap == 1, since running_trap
     gets set to SIG+1), and we don't have a argument given to `exit'
     (list == 0), use the exit status we saved before running the trap
     commands (trap_saved_exit_value). */
  exit_value = (running_trap == 1 && list == 0) ? trap_saved_exit_value : get_exitstat (list);

  bash_logout ();

  last_command_exit_value = exit_value;

  /* Exit the program. */
  jump_to_top_level (EXITBLTIN);
  /*NOTREACHED*/
}

void
bash_logout ()
{
  /* Run our `~/.bash_logout' file if it exists, and this is a login shell. */
  if (login_shell && sourced_logout++ == 0 && subshell_environment == 0)
    {
      maybe_execute_file ("~/.bash_logout", 1);
#ifdef SYS_BASH_LOGOUT
      maybe_execute_file (SYS_BASH_LOGOUT, 1);
#endif
    }
}
