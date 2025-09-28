/* nojobs.c - functions that make children, remember them, and handle their termination. */

/* This file works under BSD, System V, minix, and Posix systems.  It does
   not implement job control. */

/* Copyright (C) 1987-2022 Free Software Foundation, Inc.

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
*/

#include "config.h"

#include "bashtypes.h"
#include "filecntl.h"

#if defined (HAVE_UNISTD_H)
#  include <unistd.h>
#endif

#include <stdio.h>
#include <signal.h>
#include <errno.h>

#if defined (BUFFERED_INPUT)
#  include "input.h"
#endif

/* Need to include this up here for *_TTY_DRIVER definitions. */
#include "shtty.h"

#include "bashintl.h"

#include "shell.h"
#include "jobs.h"
#include "execute_cmd.h"
#include "trap.h"

#include "builtins/builtext.h"	/* for wait_builtin */
#include "builtins/common.h"

#define DEFAULT_CHILD_MAX 4096

#if defined (_POSIX_VERSION) || !defined (HAVE_KILLPG)
#  define killpg(pg, sig)		kill(-(pg),(sig))
#endif /* USG || _POSIX_VERSION */

#if !defined (HAVE_SIGINTERRUPT) && !defined (HAVE_POSIX_SIGNALS)
#  define siginterrupt(sig, code)
#endif /* !HAVE_SIGINTERRUPT && !HAVE_POSIX_SIGNALS */

#if defined (HAVE_WAITPID)
#  define WAITPID(pid, statusp, options) waitpid (pid, statusp, options)
#else
#  define WAITPID(pid, statusp, options) wait (statusp)
#endif /* !HAVE_WAITPID */

/* Return the fd from which we are actually getting input. */
#define input_tty() (shell_tty != -1) ? shell_tty : fileno (stderr)

#if !defined (errno)
extern int errno;
#endif /* !errno */

extern void set_original_signal PARAMS((int, SigHandler *));

volatile pid_t last_made_pid = NO_PID;
volatile pid_t last_asynchronous_pid = NO_PID;

static int queue_sigchld;		/* dummy declaration */
int waiting_for_child;

/* Call this when you start making children. */
int already_making_children = 0;

/* The controlling tty for this shell. */
int shell_tty = -1;

/* If this is non-zero, $LINES and $COLUMNS are reset after every process
   exits from get_tty_state(). */
int check_window_size = CHECKWINSIZE_DEFAULT;

/* We don't have job control. */
int job_control = 0;

int running_in_background = 0;	/* can't tell without job control */

/* STATUS and FLAGS are only valid if pid != NO_PID
   STATUS is only valid if (flags & PROC_RUNNING) == 0 */
struct proc_status {
  pid_t pid;
  int status;	/* Exit status of PID or 128 + fatal signal number */
  int flags;
};

/* Values for proc_status.flags */
#define PROC_RUNNING	0x01
#define PROC_NOTIFIED	0x02
#define PROC_ASYNC	0x04
#define PROC_SIGNALED	0x10

/* Return values from find_status_by_pid */
#define PROC_BAD	 -1
#define PROC_STILL_ALIVE -2

static struct proc_status *pid_list = (struct proc_status *)NULL;
static int pid_list_size;
static int wait_sigint_received;

static long child_max = -1L;

static void alloc_pid_list PARAMS((void));
static int find_proc_slot PARAMS((pid_t));
static int find_index_by_pid PARAMS((pid_t));
static int find_status_by_pid PARAMS((pid_t));
static int process_exit_status PARAMS((WAIT));
static int find_termsig_by_pid PARAMS((pid_t));
static int get_termsig PARAMS((WAIT));
static void set_pid_status PARAMS((pid_t, WAIT));
static void set_pid_flags PARAMS((pid_t, int));
static void unset_pid_flags PARAMS((pid_t, int));
static int get_pid_flags PARAMS((pid_t));
static void add_pid PARAMS((pid_t, int));
static void mark_dead_jobs_as_notified PARAMS((int));

static sighandler wait_sigint_handler PARAMS((int));
static char *j_strsignal PARAMS((int));

#if defined (HAVE_WAITPID)
static void reap_zombie_children PARAMS((void));
#endif

#if !defined (HAVE_SIGINTERRUPT) && defined (HAVE_POSIX_SIGNALS)
static int siginterrupt PARAMS((int, int));
#endif

static void restore_sigint_handler PARAMS((void));

/* Allocate new, or grow existing PID_LIST. */
static void
alloc_pid_list ()
{
  register int i;
  int old = pid_list_size;

  pid_list_size += 10;
  pid_list = (struct proc_status *)xrealloc (pid_list, pid_list_size * sizeof (struct proc_status));

  /* None of the newly allocated slots have process id's yet. */
  for (i = old; i < pid_list_size; i++)
    {
      pid_list[i].pid = NO_PID;
      pid_list[i].status = pid_list[i].flags = 0;
    }
}

/* Return the offset within the PID_LIST array of an empty slot.  This can
   create new slots if all of the existing slots are taken. */
static int
find_proc_slot (pid)
     pid_t pid;
{
  register int i;

  for (i = 0; i < pid_list_size; i++)
    if (pid_list[i].pid == NO_PID || pid_list[i].pid == pid)
      return (i);

  if (i == pid_list_size)
    alloc_pid_list ();

  return (i);
}

/* Return the offset within the PID_LIST array of a slot containing PID,
   or the value NO_PID if the pid wasn't found. */
static int
find_index_by_pid (pid)
     pid_t pid;
{
  register int i;

  for (i = 0; i < pid_list_size; i++)
    if (pid_list[i].pid == pid)
      return (i);

  return (NO_PID);
}

/* Return the status of PID as looked up in the PID_LIST array.  A
   return value of PROC_BAD indicates that PID wasn't found. */
static int
find_status_by_pid (pid)
     pid_t pid;
{
  int i;

  i = find_index_by_pid (pid);
  if (i == NO_PID)
    return (PROC_BAD);
  if (pid_list[i].flags & PROC_RUNNING)
    return (PROC_STILL_ALIVE);
  return (pid_list[i].status);
}

static int
process_exit_status (status)
     WAIT status;
{
  if (WIFSIGNALED (status))
    return (128 + WTERMSIG (status));
  else
    return (WEXITSTATUS (status));
}

/* Return the status of PID as looked up in the PID_LIST array.  A
   return value of PROC_BAD indicates that PID wasn't found. */
static int
find_termsig_by_pid (pid)
     pid_t pid;
{
  int i;

  i = find_index_by_pid (pid);
  if (i == NO_PID)
    return (0);
  if (pid_list[i].flags & PROC_RUNNING)
    return (0);
  return (get_termsig ((WAIT)pid_list[i].status));
}

/* Set LAST_COMMAND_EXIT_SIGNAL depending on STATUS.  If STATUS is -1, look
   up PID in the pid array and set LAST_COMMAND_EXIT_SIGNAL appropriately
   depending on its flags and exit status. */
static int
get_termsig (status)
     WAIT status;
{
  if (WIFSTOPPED (status) == 0 && WIFSIGNALED (status))
    return (WTERMSIG (status));
  else
    return (0);
}

/* Give PID the status value STATUS in the PID_LIST array. */
static void
set_pid_status (pid, status)
     pid_t pid;
     WAIT status;
{
  int slot;

#if defined (COPROCESS_SUPPORT)
  coproc_pidchk (pid, status);
#endif

#if defined (PROCESS_SUBSTITUTION)
  if ((slot = find_procsub_child (pid)) >= 0)
    set_procsub_status (slot, pid, WSTATUS (status));
    /* XXX - also saving in list below */
#endif

  slot = find_index_by_pid (pid);
  if (slot == NO_PID)
    return;

  pid_list[slot].status = process_exit_status (status);
  pid_list[slot].flags &= ~PROC_RUNNING;
  if (WIFSIGNALED (status))
    pid_list[slot].flags |= PROC_SIGNALED;
  /* If it's not a background process, mark it as notified so it gets
     cleaned up. */
  if ((pid_list[slot].flags & PROC_ASYNC) == 0)
    pid_list[slot].flags |= PROC_NOTIFIED;
}

/* Give PID the flags FLAGS in the PID_LIST array. */
static void
set_pid_flags (pid, flags)
     pid_t pid;
     int flags;
{
  int slot;

  slot = find_index_by_pid (pid);
  if (slot == NO_PID)
    return;

  pid_list[slot].flags |= flags;
}

/* Unset FLAGS for PID in the pid list */
static void
unset_pid_flags (pid, flags)
     pid_t pid;
     int flags;
{
  int slot;

  slot = find_index_by_pid (pid);
  if (slot == NO_PID)
    return;

  pid_list[slot].flags &= ~flags;
}

/* Return the flags corresponding to PID in the PID_LIST array. */
static int
get_pid_flags (pid)
     pid_t pid;
{
  int slot;

  slot = find_index_by_pid (pid);
  if (slot == NO_PID)
    return 0;

  return (pid_list[slot].flags);
}

static void
add_pid (pid, async)
     pid_t pid;
     int async;
{
  int slot;

  slot = find_proc_slot (pid);

  pid_list[slot].pid = pid;
  pid_list[slot].status = -1;
  pid_list[slot].flags = PROC_RUNNING;
  if (async)
    pid_list[slot].flags |= PROC_ASYNC;
}

static void
mark_dead_jobs_as_notified (force)
     int force;
{
  register int i, ndead;

  /* first, count the number of non-running async jobs if FORCE == 0 */
  for (i = ndead = 0; force == 0 && i < pid_list_size; i++)
    {
      if (pid_list[i].pid == NO_PID)
	continue;
      if (((pid_list[i].flags & PROC_RUNNING) == 0) &&
	   (pid_list[i].flags & PROC_ASYNC))
	ndead++;
    }

  if (child_max < 0)
    child_max = getmaxchild ();
  if (child_max < 0)
    child_max = DEFAULT_CHILD_MAX;

  if (force == 0 && ndead <= child_max)
    return;

  /* If FORCE == 0, we just mark as many non-running async jobs as notified
     to bring us under the CHILD_MAX limit. */
  for (i = 0; i < pid_list_size; i++)
    {
      if (pid_list[i].pid == NO_PID)
	continue;
      if (((pid_list[i].flags & PROC_RUNNING) == 0) &&
	   pid_list[i].pid != last_asynchronous_pid)
	{
	  pid_list[i].flags |= PROC_NOTIFIED;
	  if (force == 0 && (pid_list[i].flags & PROC_ASYNC) && --ndead <= child_max)
	    break;
	}
    }
}

/* Remove all dead, notified jobs from the pid_list. */
int
cleanup_dead_jobs ()
{
  register int i;

#if defined (HAVE_WAITPID)
  reap_zombie_children ();
#endif

  for (i = 0; i < pid_list_size; i++)
    {
      if (pid_list[i].pid != NO_PID &&
	    (pid_list[i].flags & PROC_RUNNING) == 0 &&
	    (pid_list[i].flags & PROC_NOTIFIED))
	pid_list[i].pid = NO_PID;
    }

#if defined (COPROCESS_SUPPORT)
  coproc_reap ();
#endif

  return 0;
}

void
reap_dead_jobs ()
{
  mark_dead_jobs_as_notified (0);
  cleanup_dead_jobs ();
}

/* Initialize the job control mechanism, and set up the tty stuff. */
int
initialize_job_control (force)
     int force;
{
  shell_tty = fileno (stderr);

  if (interactive)
    get_tty_state ();
  return 0;
}

/* Setup this shell to handle C-C, etc. */
void
initialize_job_signals ()
{
  set_signal_handler (SIGINT, sigint_sighandler);

  /* If this is a login shell we don't wish to be disturbed by
     stop signals. */
  if (login_shell)
    ignore_tty_job_signals ();
}

#if defined (HAVE_WAITPID)
/* Collect the status of all zombie children so that their system
   resources can be deallocated. */
static void
reap_zombie_children ()
{
#  if defined (WNOHANG)
  pid_t pid;
  WAIT status;

  CHECK_TERMSIG;
  CHECK_WAIT_INTR;
  while ((pid = waitpid (-1, (int *)&status, WNOHANG)) > 0)
    set_pid_status (pid, status);
#  endif /* WNOHANG */
  CHECK_TERMSIG;
  CHECK_WAIT_INTR;
}
#endif /* WAITPID */

#if !defined (HAVE_SIGINTERRUPT) && defined (HAVE_POSIX_SIGNALS)

#if !defined (SA_RESTART)
#  define SA_RESTART 0
#endif

static int
siginterrupt (sig, flag)
     int sig, flag;
{
  struct sigaction act;

  sigaction (sig, (struct sigaction *)NULL, &act);

  if (flag)
    act.sa_flags &= ~SA_RESTART;
  else
    act.sa_flags |= SA_RESTART;

  return (sigaction (sig, &act, (struct sigaction *)NULL));
}
#endif /* !HAVE_SIGINTERRUPT && HAVE_POSIX_SIGNALS */

/* Fork, handling errors.  Returns the pid of the newly made child, or 0.
   COMMAND is just for remembering the name of the command; we don't do
   anything else with it.  ASYNC_P says what to do with the tty.  If
   non-zero, then don't give it away. */
pid_t
make_child (command, flags)
     char *command;
     int flags;
{
  pid_t pid;
  int async_p, forksleep;
  sigset_t set, oset;

  /* Discard saved memory. */
  if (command)
    free (command);

  async_p = (flags & FORK_ASYNC);
  start_pipeline ();

#if defined (BUFFERED_INPUT)
  /* If default_buffered_input is active, we are reading a script.  If
     the command is asynchronous, we have already duplicated /dev/null
     as fd 0, but have not changed the buffered stream corresponding to
     the old fd 0.  We don't want to sync the stream in this case. */
  if (default_buffered_input != -1 && (!async_p || default_buffered_input > 0))
    sync_buffered_stream (default_buffered_input);
#endif /* BUFFERED_INPUT */

  /* Block SIGTERM here and unblock in child after fork resets the
     set of pending signals */
  if (interactive_shell)
    {
      sigemptyset (&set);
      sigaddset (&set, SIGTERM);
      sigemptyset (&oset);
      sigprocmask (SIG_BLOCK, &set, &oset);
      set_signal_handler (SIGTERM, SIG_DFL);
    }

  /* Create the child, handle severe errors.  Retry on EAGAIN. */
  forksleep = 1;
  while ((pid = fork ()) < 0 && errno == EAGAIN && forksleep < FORKSLEEP_MAX)
    {
      sys_error ("fork: retry");

#if defined (HAVE_WAITPID)
      /* Posix systems with a non-blocking waitpid () system call available
	 get another chance after zombies are reaped. */
      reap_zombie_children ();
      if (forksleep > 1 && sleep (forksleep) != 0)
        break;
#else
      if (sleep (forksleep) != 0)
	break;
#endif /* HAVE_WAITPID */
      forksleep <<= 1;
    }

  if (pid != 0)
    if (interactive_shell)
      {
	set_signal_handler (SIGTERM, SIG_IGN);
	sigprocmask (SIG_SETMASK, &oset, (sigset_t *)NULL);
      }

  if (pid < 0)
    {
      sys_error ("fork");
      last_command_exit_value = EX_NOEXEC;
      throw_to_top_level ();
    }

  if (pid == 0)
    {
#if defined (BUFFERED_INPUT)
      unset_bash_input (0);
#endif /* BUFFERED_INPUT */

      CLRINTERRUPT;	/* XXX - children have their own interrupt state */

      /* Restore top-level signal mask. */
      restore_sigmask ();

#if 0
      /* Ignore INT and QUIT in asynchronous children. */
      if (async_p)
	last_asynchronous_pid = getpid ();
#endif

      subshell_environment |= SUBSHELL_IGNTRAP;

      default_tty_job_signals ();
    }
  else
    {
      /* In the parent. */

      last_made_pid = pid;

      if (async_p)
	last_asynchronous_pid = pid;

      add_pid (pid, async_p);
    }
  return (pid);
}

void
ignore_tty_job_signals ()
{
#if defined (SIGTSTP)
  set_signal_handler (SIGTSTP, SIG_IGN);
  set_signal_handler (SIGTTIN, SIG_IGN);
  set_signal_handler (SIGTTOU, SIG_IGN);
#endif
}

void
default_tty_job_signals ()
{
#if defined (SIGTSTP)
  if (signal_is_trapped (SIGTSTP) == 0 && signal_is_hard_ignored (SIGTSTP))
    set_signal_handler (SIGTSTP, SIG_IGN);
  else
    set_signal_handler (SIGTSTP, SIG_DFL);
  if (signal_is_trapped (SIGTTIN) == 0 && signal_is_hard_ignored (SIGTTIN))
    set_signal_handler (SIGTTIN, SIG_IGN);
  else
    set_signal_handler (SIGTTIN, SIG_DFL);
  if (signal_is_trapped (SIGTTOU) == 0 && signal_is_hard_ignored (SIGTTOU))
    set_signal_handler (SIGTTOU, SIG_IGN);
  else
    set_signal_handler (SIGTTOU, SIG_DFL);
#endif
}

/* Called once in a parent process. */
void
get_original_tty_job_signals ()
{
  static int fetched = 0;

  if (fetched == 0)
    {
#if defined (SIGTSTP)
      if (interactive_shell)
	{
	  set_original_signal (SIGTSTP, SIG_DFL);
	  set_original_signal (SIGTTIN, SIG_DFL);
	  set_original_signal (SIGTTOU, SIG_DFL);
	}
      else
	{
	  get_original_signal (SIGTSTP);
	  get_original_signal (SIGTTIN);
	  get_original_signal (SIGTTOU);
	}
#endif
      fetched = 1;
    }
}

/* Wait for a single pid (PID) and return its exit status.  Called by
   the wait builtin. */
int
wait_for_single_pid (pid, flags)
     pid_t pid;
     int flags;
{
  pid_t got_pid;
  WAIT status;
  int pstatus;

  pstatus = find_status_by_pid (pid);

  if (pstatus == PROC_BAD)
    {
      internal_error (_("wait: pid %ld is not a child of this shell"), (long)pid);
      return (257);
    }

  if (pstatus != PROC_STILL_ALIVE)
    {
      if (pstatus > 128)
	last_command_exit_signal = find_termsig_by_pid (pid);
      return (pstatus);
    }

  siginterrupt (SIGINT, 1);
  while ((got_pid = WAITPID (pid, &status, 0)) != pid)
    {
      CHECK_TERMSIG;
      CHECK_WAIT_INTR;
      if (got_pid < 0)
	{
	  if (errno != EINTR && errno != ECHILD)
	    {
	      siginterrupt (SIGINT, 0);
	      sys_error ("wait");
	    }
	  break;
	}
      else if (got_pid > 0)
	set_pid_status (got_pid, status);
    }

  if (got_pid > 0)
    {
      set_pid_status (got_pid, status);
      set_pid_flags (got_pid, PROC_NOTIFIED);
    }

  siginterrupt (SIGINT, 0);
  QUIT;
  CHECK_WAIT_INTR;

  return (got_pid > 0 ? process_exit_status (status) : -1);
}

/* Wait for all of the shell's children to exit.  Called by the `wait'
   builtin. */
int
wait_for_background_pids (ps)
     struct procstat *ps;
{
  pid_t got_pid;
  WAIT status;
  int njobs;

  /* If we aren't using job control, we let the kernel take care of the
     bookkeeping for us.  wait () will return -1 and set errno to ECHILD
     when there are no more unwaited-for child processes on both
     4.2 BSD-based and System V-based systems. */

  njobs = 0;
  siginterrupt (SIGINT, 1);

  /* Wait for ECHILD */
  waiting_for_child = 1;
  while ((got_pid = WAITPID (-1, &status, 0)) != -1)
    {
      waiting_for_child = 0;
      njobs++;
      set_pid_status (got_pid, status);
      if (ps)
	{
	  ps->pid = got_pid;
	  ps->status = process_exit_status (status);
	}
      waiting_for_child = 1;
      CHECK_WAIT_INTR;
    }
  waiting_for_child = 0;

  if (errno != EINTR && errno != ECHILD)
    {
      siginterrupt (SIGINT, 0);
      sys_error("wait");
    }

  siginterrupt (SIGINT, 0);
  QUIT;
  CHECK_WAIT_INTR;

  mark_dead_jobs_as_notified (1);
  cleanup_dead_jobs ();

  return njobs;
}

void
wait_sigint_cleanup ()
{
}

/* Make OLD_SIGINT_HANDLER the SIGINT signal handler. */
#define INVALID_SIGNAL_HANDLER (SigHandler *)wait_for_background_pids
static SigHandler *old_sigint_handler = INVALID_SIGNAL_HANDLER;

static void
restore_sigint_handler ()
{
  if (old_sigint_handler != INVALID_SIGNAL_HANDLER)
    {
      set_signal_handler (SIGINT, old_sigint_handler);
      old_sigint_handler = INVALID_SIGNAL_HANDLER;
    }
}

/* Handle SIGINT while we are waiting for children in a script to exit.
   All interrupts are effectively ignored by the shell, but allowed to
   kill a running job. */
static sighandler
wait_sigint_handler (sig)
     int sig;
{
  SigHandler *sigint_handler;

  /* If we got a SIGINT while in `wait', and SIGINT is trapped, do
     what POSIX.2 says (see builtins/wait.def for more info). */
  if (this_shell_builtin && this_shell_builtin == wait_builtin &&
      signal_is_trapped (SIGINT) &&
      ((sigint_handler = trap_to_sighandler (SIGINT)) == trap_handler))
    {
      last_command_exit_value = 128+SIGINT;
      restore_sigint_handler ();
      trap_handler (SIGINT);	/* set pending_traps[SIGINT] */
      wait_signal_received = SIGINT;
      SIGRETURN (0);
    }

  wait_sigint_received = 1;

  SIGRETURN (0);
}

static char *
j_strsignal (s)
     int s;
{
  static char retcode_name_buffer[64] = { '\0' };
  char *x;

  x = strsignal (s);
  if (x == 0)
    {
      x = retcode_name_buffer;
      sprintf (x, "Signal %d", s);
    }
  return x;
}

/* Wait for pid (one of our children) to terminate.  This is called only
   by the execution code in execute_cmd.c. */
int
wait_for (pid, flags)
     pid_t pid;
     int flags;
{
  int return_val, pstatus;
  pid_t got_pid;
  WAIT status;

  pstatus = find_status_by_pid (pid);

  if (pstatus == PROC_BAD)
    return (0);

  if (pstatus != PROC_STILL_ALIVE)
    {
      if (pstatus > 128)
	last_command_exit_signal = find_termsig_by_pid (pid);
      return (pstatus);
    }

  /* If we are running a script, ignore SIGINT while we're waiting for
     a child to exit.  The loop below does some of this, but not all. */
  wait_sigint_received = 0;
  if (interactive_shell == 0)
    old_sigint_handler = set_signal_handler (SIGINT, wait_sigint_handler);

  waiting_for_child = 1;  
  CHECK_WAIT_INTR;
  while ((got_pid = WAITPID (-1, &status, 0)) != pid) /* XXX was pid now -1 */
    {
      waiting_for_child = 0;
      CHECK_TERMSIG;
      CHECK_WAIT_INTR;
      if (got_pid < 0 && errno == ECHILD)
	{
#if !defined (_POSIX_VERSION)
	  status.w_termsig = status.w_retcode = 0;
#else
	  status = 0;
#endif /* _POSIX_VERSION */
	  break;
	}
      else if (got_pid < 0 && errno != EINTR)
	programming_error ("wait_for(%ld): %s", (long)pid, strerror(errno));
      else if (got_pid > 0)
	set_pid_status (got_pid, status);
      waiting_for_child = 1;
    }
  waiting_for_child = 0;

  if (got_pid > 0)
    set_pid_status (got_pid, status);

#if defined (HAVE_WAITPID)
  if (got_pid >= 0)
    reap_zombie_children ();
#endif /* HAVE_WAITPID */

  CHECK_TERMSIG;
  CHECK_WAIT_INTR;

  if (interactive_shell == 0)
    {
      SigHandler *temp_handler;

      temp_handler = old_sigint_handler;
      restore_sigint_handler ();

      /* If the job exited because of SIGINT, make sure the shell acts as if
	 it had received one also. */
      if (WIFSIGNALED (status) && (WTERMSIG (status) == SIGINT))
	{

	  if (maybe_call_trap_handler (SIGINT) == 0)
	    {
	      if (temp_handler == SIG_DFL)
		termsig_handler (SIGINT);
	      else if (temp_handler != INVALID_SIGNAL_HANDLER && temp_handler != SIG_IGN)
		(*temp_handler) (SIGINT);
	    }
	}
    }

  /* Default return value. */
  /* ``a full 8 bits of status is returned'' */
  return_val = process_exit_status (status);
  last_command_exit_signal = get_termsig (status);

#if defined (DONT_REPORT_SIGPIPE) && defined (DONT_REPORT_SIGTERM)
#  define REPORTSIG(x) ((x) != SIGINT && (x) != SIGPIPE && (x) != SIGTERM)
#elif !defined (DONT_REPORT_SIGPIPE) && !defined (DONT_REPORT_SIGTERM)
#  define REPORTSIG(x) ((x) != SIGINT)
#elif defined (DONT_REPORT_SIGPIPE)
#  define REPORTSIG(x) ((x) != SIGINT && (x) != SIGPIPE)
#else
#  define REPORTSIG(x) ((x) != SIGINT && (x) != SIGTERM)
#endif

  if ((WIFSTOPPED (status) == 0) && WIFSIGNALED (status) && REPORTSIG(WTERMSIG (status)))
    {
      fprintf (stderr, "%s", j_strsignal (WTERMSIG (status)));
      if (WIFCORED (status))
	fprintf (stderr, _(" (core dumped)"));
      fprintf (stderr, "\n");
    }

  if (interactive_shell && subshell_environment == 0)
    {
      if (WIFSIGNALED (status) || WIFSTOPPED (status))
	set_tty_state ();
      else
	get_tty_state ();
    }
  else if (interactive_shell == 0 && subshell_environment == 0 && check_window_size)
    get_new_window_size (0, (int *)0, (int *)0);

  return (return_val);
}

/* Send PID SIGNAL.  Returns -1 on failure, 0 on success.  If GROUP is non-zero,
   or PID is less than -1, then kill the process group associated with PID. */
int
kill_pid (pid, signal, group)
     pid_t pid;
     int signal, group;
{
  int result;

  if (pid < -1)
    {
      pid = -pid;
      group = 1;
    }
  result = group ? killpg (pid, signal) : kill (pid, signal);
  return (result);
}

static TTYSTRUCT shell_tty_info;
static int got_tty_state;

/* Fill the contents of shell_tty_info with the current tty info. */
int
get_tty_state ()
{
  int tty;

  tty = input_tty ();
  if (tty != -1)
    {
      ttgetattr (tty, &shell_tty_info);
      got_tty_state = 1;
      if (check_window_size)
	get_new_window_size (0, (int *)0, (int *)0);
    }
  return 0;
}

/* Make the current tty use the state in shell_tty_info. */
int
set_tty_state ()
{
  int tty;

  tty = input_tty ();
  if (tty != -1)
    {
      if (got_tty_state == 0)
	return 0;
      ttsetattr (tty, &shell_tty_info);
    }
  return 0;
}

/* Give the terminal to PGRP.  */
int
give_terminal_to (pgrp, force)
     pid_t pgrp;
     int force;
{
  return 0;
}

/* Stop a pipeline. */
int
stop_pipeline (async, ignore)
     int async;
     COMMAND *ignore;
{
  already_making_children = 0;
  return 0;
}

void
start_pipeline ()
{
  already_making_children = 1;
}

void
stop_making_children ()
{
  already_making_children = 0;
}

/* The name is kind of a misnomer, but it's what the job control code uses. */
void
without_job_control ()
{
  stop_making_children ();
  last_made_pid = NO_PID;	/* XXX */
}

int
get_job_by_pid (pid, block, ignore)
     pid_t pid;
     int block;
     PROCESS **ignore;
{
  int i;

  i = find_index_by_pid (pid);
  return ((i == NO_PID) ? PROC_BAD : i);
}

/* Print descriptive information about the job with leader pid PID. */
void
describe_pid (pid)
     pid_t pid;
{
  fprintf (stderr, "%ld\n", (long) pid);
}

int
freeze_jobs_list ()
{
  return 0;
}

void
unfreeze_jobs_list ()
{
}

void
set_jobs_list_frozen (s)
     int s;
{
}

int
count_all_jobs ()
{
  return 0;
}
