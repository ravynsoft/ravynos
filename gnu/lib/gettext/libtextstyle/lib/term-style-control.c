/* Terminal control for outputting styled text to a terminal.
   Copyright (C) 2006-2008, 2017, 2019-2023 Free Software Foundation, Inc.
   Written by Bruno Haible <bruno@clisp.org>, 2019.

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
#include "term-style-control.h"

/* Set to 1 to get debugging output regarding signals.  */
#define DEBUG_SIGNALS 0

#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#if DEBUG_SIGNALS
# include <stdio.h>
#endif
#if HAVE_TCGETATTR
# include <termios.h>
# if !defined NOFLSH            /* QNX */
#  define NOFLSH 0
# endif
#endif
#if HAVE_TCGETATTR
# include <sys/stat.h>
#endif

#include "fatal-signal.h"
#include "sig-handler.h"
#include "full-write.h"
#include "same-inode.h"
#include "xalloc.h"

#define SIZEOF(a) (sizeof(a) / sizeof(a[0]))


/* ============================ EINTR handling ============================ */

/* EINTR handling for tcgetattr(), tcsetattr().
   These functions can return -1/EINTR even when we don't have any
   signal handlers set up, namely when we get interrupted via SIGSTOP.  */

#if HAVE_TCGETATTR

static inline int
nonintr_tcgetattr (int fd, struct termios *tcp)
{
  int retval;

  do
    retval = tcgetattr (fd, tcp);
  while (retval < 0 && errno == EINTR);

  return retval;
}

static inline int
nonintr_tcsetattr (int fd, int flush_mode, const struct termios *tcp)
{
  int retval;

  do
    retval = tcsetattr (fd, flush_mode, tcp);
  while (retval < 0 && errno == EINTR);

  return retval;
}

#endif


/* ========================== Logging primitives ========================== */

/* We need logging, especially for the signal handling, because
     - Debugging through gdb is hardly possible, because gdb produces output
       by itself and interferes with the process states.
     - strace is buggy when it comes to SIGTSTP handling:  By default, it
       sends the process a SIGSTOP signal instead of SIGTSTP.  It supports
       an option '-D -I4' to mitigate this, though.  Also, race conditions
       appear with different probability with and without strace.
   fprintf(stderr) is not possible within async-safe code, because fprintf()
   may invoke malloc().  */

#if DEBUG_SIGNALS

/* Log a simple message.  */
static _GL_ASYNC_SAFE void
log_message (const char *message)
{
  full_write (STDERR_FILENO, message, strlen (message));
}

#else

# define log_message(message)

#endif

#if HAVE_TCGETATTR || DEBUG_SIGNALS

/* Async-safe implementation of sprintf (str, "%d", n).  */
static _GL_ASYNC_SAFE void
sprintf_integer (char *str, int x)
{
  unsigned int y;
  char buf[20];
  char *p;
  size_t n;

  if (x < 0)
    {
      *str++ = '-';
      y = (unsigned int) (-1 - x) + 1;
    }
  else
    y = x;

  p = buf + sizeof (buf);
  do
    {
      *--p = '0' + (y % 10);
      y = y / 10;
    }
  while (y > 0);
  n = buf + sizeof (buf) - p;
  memcpy (str, p, n);
  str[n] = '\0';
}

#endif

#if HAVE_TCGETATTR

/* Async-safe conversion of errno value to string.  */
static _GL_ASYNC_SAFE void
simple_errno_string (char *str, int errnum)
{
  switch (errnum)
    {
    case EBADF:  strcpy (str, "EBADF"); break;
    case EINTR:  strcpy (str, "EINTR"); break;
    case EINVAL: strcpy (str, "EINVAL"); break;
    case EIO:    strcpy (str, "EIO"); break;
    case ENOTTY: strcpy (str, "ENOTTY"); break;
    default: sprintf_integer (str, errnum); break;
    }
}

#endif

#if DEBUG_SIGNALS

/* Async-safe conversion of signal number to name.  */
static _GL_ASYNC_SAFE void
simple_signal_string (char *str, int sig)
{
  switch (sig)
    {
    /* Fatal signals (see fatal-signal.c).  */
    #ifdef SIGINT
    case SIGINT:   strcpy (str, "SIGINT"); break;
    #endif
    #ifdef SIGTERM
    case SIGTERM:  strcpy (str, "SIGTERM"); break;
    #endif
    #ifdef SIGHUP
    case SIGHUP:   strcpy (str, "SIGHUP"); break;
    #endif
    #ifdef SIGPIPE
    case SIGPIPE:  strcpy (str, "SIGPIPE"); break;
    #endif
    #ifdef SIGXCPU
    case SIGXCPU:  strcpy (str, "SIGXCPU"); break;
    #endif
    #ifdef SIGXFSZ
    case SIGXFSZ:  strcpy (str, "SIGXFSZ"); break;
    #endif
    #ifdef SIGBREAK
    case SIGBREAK: strcpy (str, "SIGBREAK"); break;
    #endif
    /* Stopping signals.  */
    #ifdef SIGTSTP
    case SIGTSTP:  strcpy (str, "SIGTSTP"); break;
    #endif
    #ifdef SIGTTIN
    case SIGTTIN:  strcpy (str, "SIGTTIN"); break;
    #endif
    #ifdef SIGTTOU
    case SIGTTOU:  strcpy (str, "SIGTTOU"); break;
    #endif
    /* Continuing signals.  */
    #ifdef SIGCONT
    case SIGCONT:  strcpy (str, "SIGCONT"); break;
    #endif
    default: sprintf_integer (str, sig); break;
    }
}

/* Emit a message that a given signal handler is being run.  */
static _GL_ASYNC_SAFE void
log_signal_handler_called (int sig)
{
  char message[100];
  strcpy (message, "Signal handler for signal ");
  simple_signal_string (message + strlen (message), sig);
  strcat (message, " called.\n");
  log_message (message);
}

#else

static void
log_signal_handler_called (_GL_UNUSED int sig)
{
}

#endif


/* ============================ Signal handling ============================ */

/* There are several situations which can cause garbled output on the terminal's
   screen:
   (1) When the program calls exit() after calling flush_to_current_style,
       the program would terminate and leave the terminal in a non-default
       state.
   (2) When the program is interrupted through a fatal signal, the terminal
       would be left in a non-default state.
   (3) When the program is stopped through a stopping signal, the terminal
       would be left (for temporary use by other programs) in a non-default
       state.
   (4) When a foreground process receives a SIGINT, the kernel(!) prints '^C'.
       On Linux, the place where this happens is
         linux-5.0/drivers/tty/n_tty.c:713..730
       within a call sequence
         n_tty_receive_signal_char (n_tty.c:1245..1246)
         -> commit_echoes (n_tty.c:792)
         -> __process_echoes (n_tty.c:713..730).
   (5) When a signal is sent, the output buffer is cleared.
       On Linux, this output buffer consists of the "echo buffer" in the tty
       and the "output buffer" in the driver.  The place where this happens is
         linux-5.0/drivers/tty/n_tty.c:1133..1140
       within a call
         isig (n_tty.c:1133..1140).

   How do we mitigate these problems?
   (1) We install an exit handler that restores the terminal to the default
       state.
   (2) If tty_control is TTYCTL_PARTIAL or TTYCTL_FULL:
       For some of the fatal signals (see gnulib's 'fatal-signal' module for
       the precise list), we install a handler that attempts to restore the
       terminal to the default state.  Since the terminal may be in the middle
       of outputting an escape sequence at this point, the first escape
       sequence emitted from this handler may have no effect and produce
       garbled characters instead.  Therefore the handler outputs the cleanup
       sequence twice.
       For the other fatal signals, we don't do anything.
   (3) If tty_control is TTYCTL_PARTIAL or TTYCTL_FULL:
       For some of the stopping signals (SIGTSTP, SIGTTIN, SIGTTOU), we install
       a handler that attempts to restore the terminal to the default state.
       For SIGCONT, we install a handler that does the opposite: it puts the
       terminal into the desired state again.
       For SIGSTOP, we cannot do anything.
   (4) If tty_control is TTYCTL_FULL:
       The kernel's action depends on L_ECHO(tty) and L_ISIG(tty), that is, on
       the local modes of the tty (see
       <https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/V1_chap11.html>
       section 11.2.5).  We don't want to change L_ISIG; hence we change L_ECHO.
       So, we disable the ECHO local flag of the tty; the equivalent command is
       'stty -echo'.
   (5) If tty_control is TTYCTL_FULL:
       The kernel's action depends on !L_NOFLSH(tty), that is, again on the
       local modes of the tty (see
       <https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/V1_chap11.html>
       section 11.2.5).  So, we enable the NOFLSH local flag of the tty; the
       equivalent command is 'stty noflsh'.
       For terminals with a baud rate < 9600 this is suboptimal.  For this case
       - where the traditional flushing behaviour makes sense - we would use a
       technique that involves tcdrain(), TIOCOUTQ, and usleep() when it is OK
       to disable NOFLSH.

   Regarding (4) and (5), there is a complication: Changing the local modes is
   done through tcsetattr().  However, when the process is put into the
   background, tcsetattr() does not operate the same way as when the process is
   running in the foreground.
   To test this kind of behaviour, use the 'color-filter' example like this:
     $ yes | ./filter '.*'
     <Ctrl-Z>
     $ bg 1
   We have three possible implementation options:
     * If we don't ignore the signal SIGTTOU:
       If the TOSTOP bit in the terminal's local mode is clear (command
       equivalent: 'stty -tostop') and the process is put into the background,
       normal output would continue (per POSIX
       <https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/V1_chap11.html>
       section 11.2.5) but tcsetattr() calls would cause it to stop due to
       a SIGTTOU signal (per POSIX
       <https://pubs.opengroup.org/onlinepubs/9699919799/functions/tcsetattr.html>).
       Thus, the program would behave differently with term-style-control than
       without.
     * If we ignore the signal SIGTTOU when the TOSTOP bit in the terminal's
       local mode is clear (i.e. when (tc.c_lflag & TOSTOP) == 0):
       The tcsetattr() calls do not stop the process, but they don't have the
       desired effect.
       On Linux, when I put the process into the background and then kill it with
       signal SIGINT, I can see that the last operation on the terminal settings
       (as shown by 'strace') is
         ioctl(1, TCSETSW, {B38400 opost isig icanon echo ...}) = 0
       and yet, once the process is terminated, the terminal settings contain
       '-echo', not 'echo'.
     * Don't call tcsetattr() if the process is not in the foreground.
       This approach produces reliable results.

   Blocking some signals while a non-default style is active is *not* useful:
     - It does not help against (1), since exit() is not a signal.
     - Signal handlers are the better approach against (2) and (3).
     - It does not help against (4) and (5), because the kernel's actions happen
       outside the process.  */
#define BLOCK_SIGNALS_DURING_NON_DEFAULT_STYLE_OUTPUT 0

/* File descriptor of the currently active 'struct term_style_controller' and
   'struct term_style_user_data'.  */
static int volatile term_fd = -1;

#if HAVE_TCGETATTR

/* Status of the process group of term_fd.  */
typedef enum
{
  PGRP_UNKNOWN = 0,     /* term_fd < 0.  Unknown status.  */
  PGRP_NO_TTY,          /* term_fd >= 0 but is not connected to a tty.  */
  PGRP_IN_FOREGROUND,   /* term_fd >= 0 is a tty.  This process is running in
                           the foreground.  */
  PGRP_IN_BACKGROUND    /* term_fd >= 0 is a tty.  This process is running in
                           the background.  */
} pgrp_status_t;
static pgrp_status_t volatile pgrp_status = PGRP_UNKNOWN;

/* Update pgrp_status, depending on term_fd.  */
static _GL_ASYNC_SAFE void
update_pgrp_status (void)
{
  int fd = term_fd;
  if (fd < 0)
    {
      pgrp_status = PGRP_UNKNOWN;
      log_message ("pgrp_status = PGRP_UNKNOWN\n");
    }
  else
    {
      pid_t p = tcgetpgrp (fd);
      if (p < 0)
        {
          pgrp_status = PGRP_NO_TTY;
          log_message ("pgrp_status = PGRP_NO_TTY\n");
        }
      else
        {
          /* getpgrp () changes when the process gets put into the background
             by a shell that implements job control.  */
          if (p == getpgrp ())
            {
              pgrp_status = PGRP_IN_FOREGROUND;
              log_message ("pgrp_status = PGRP_IN_FOREGROUND\n");
            }
          else
            {
              pgrp_status = PGRP_IN_BACKGROUND;
              log_message ("pgrp_status = PGRP_IN_BACKGROUND\n");
            }
        }
    }
}

#else

# define update_pgrp_status()

#endif

/* Controller and its user_data that contain information about how to do
   output.  */
static const struct term_style_controller * volatile active_controller;
static struct term_style_user_data * volatile active_user_data;

/* The 'struct term_style_control_data' embedded in active_user_data.
   Same as
     (active_controller != NULL
      ? active_controller->get_control_data (active_user_data)
      : NULL).  */
static struct term_style_control_data * volatile active_control_data;

/* The fd contained in active_control_data.
   Same as
     (active_controller != NULL
      ? active_control_data->fd
      : -1).  */
static int volatile active_fd = -1;

/* The exit handler.  */
static void
atexit_handler (void)
{
  /* Only do something while some output was started but not completed.  */
  if (active_controller != NULL)
    {
      active_controller->restore (active_user_data);
      deactivate_term_non_default_mode (active_controller, active_user_data);
      #if 0 /* not needed */
      deactivate_term_style_controller (active_controller, active_user_data);
      #endif
    }
}

#if HAVE_TCGETATTR

/* Return a failure message after tcsetattr() failed.  */
static _GL_ASYNC_SAFE void
tcsetattr_failed (char message[100], const char *caller)
{
  int errnum = errno;
  strcpy (message, caller);
  strcat (message, ": tcsetattr(fd=");
  sprintf_integer (message + strlen (message), active_fd);
  strcat (message, ") failed, errno=");
  simple_errno_string (message + strlen (message), errnum);
  strcat (message, "\n");
}

/* True when orig_lflag represents the original tc.c_lflag.  */
static bool volatile orig_lflag_set;
static tcflag_t volatile orig_lflag;

/* Modifies the tty's local mode, preparing for non-default terminal state.
   Used only when the active_control_data's tty_control is TTYCTL_FULL.  */
static _GL_ASYNC_SAFE void
clobber_local_mode (void)
{
  /* Here, active_fd == term_fd.  */
  if (pgrp_status == PGRP_IN_FOREGROUND)
    {
      struct termios tc;
      if (nonintr_tcgetattr (active_fd, &tc) >= 0)
        {
          if (!orig_lflag_set)
            orig_lflag = tc.c_lflag;
          /* Set orig_lflag_set to true before actually modifying the tty's
             local mode, because restore_local_mode does nothing if
             orig_lflag_set is false.  */
          orig_lflag_set = true;
          tc.c_lflag &= ~ECHO;
          tc.c_lflag |= NOFLSH;
          if (nonintr_tcsetattr (active_fd, TCSANOW, &tc) < 0)
            {
              /* Since tcsetattr failed, restore_local_mode does not need to
                 restore anything.  Set orig_lflag_set to false to indicate
                 this.  */
              orig_lflag_set = false;
              {
                char message[100];
                tcsetattr_failed (message,
                                  "term-style-control:clobber_local_mode");
                full_write (STDERR_FILENO, message, strlen (message));
              }
            }
        }
    }
}

/* Modifies the tty's local mode, once the terminal is back to the default state.
   Returns true if ECHO was turned off.
   Used only when the active_control_data's tty_control is TTYCTL_FULL.  */
static _GL_ASYNC_SAFE bool
restore_local_mode (void)
{
  /* Here, active_fd == term_fd.  */
  bool echo_was_off = false;
  /* Nothing to do if !orig_lflag_set.  */
  if (orig_lflag_set)
    {
      struct termios tc;
      if (nonintr_tcgetattr (active_fd, &tc) >= 0)
        {
          echo_was_off = (tc.c_lflag & ECHO) == 0;
          tc.c_lflag = orig_lflag;
          if (nonintr_tcsetattr (active_fd, TCSADRAIN, &tc) < 0)
            {
              char message[100];
              tcsetattr_failed (message,
                                "term-style-control:restore_local_mode");
              full_write (STDERR_FILENO, message, strlen (message));
            }
        }
      orig_lflag_set = false;
    }
  return echo_was_off;
}

#endif

#if defined SIGCONT

/* The list of signals whose default behaviour is to stop or continue the
   program.  */
static int const job_control_signals[] =
  {
    #ifdef SIGTSTP
    SIGTSTP,
    #endif
    #ifdef SIGTTIN
    SIGTTIN,
    #endif
    #ifdef SIGTTOU
    SIGTTOU,
    #endif
    #ifdef SIGCONT
    SIGCONT,
    #endif
    0
  };

# define num_job_control_signals (SIZEOF (job_control_signals) - 1)

#endif

/* The following signals are relevant because they output escape sequences to
   the terminal:
     - fatal signals,
     - stopping signals,
     - continuing signals (SIGCONT).  */

static sigset_t relevant_signal_set;
static bool relevant_signal_set_initialized = false;

static void
init_relevant_signal_set ()
{
  if (!relevant_signal_set_initialized)
    {
      int fatal_signals[64];
      size_t num_fatal_signals;
      size_t i;

      num_fatal_signals = get_fatal_signals (fatal_signals);

      sigemptyset (&relevant_signal_set);
      for (i = 0; i < num_fatal_signals; i++)
        sigaddset (&relevant_signal_set, fatal_signals[i]);
      #if defined SIGCONT
      for (i = 0; i < num_job_control_signals; i++)
        sigaddset (&relevant_signal_set, job_control_signals[i]);
      #endif

      relevant_signal_set_initialized = true;
    }
}

/* Temporarily delay the relevant signals.  */
static _GL_ASYNC_SAFE inline void
block_relevant_signals ()
{
  /* The caller must ensure that init_relevant_signal_set () was already
     called.  */
  if (!relevant_signal_set_initialized)
    abort ();

  sigprocmask (SIG_BLOCK, &relevant_signal_set, NULL);
}

/* Stop delaying the relevant signals.  */
static _GL_ASYNC_SAFE inline void
unblock_relevant_signals ()
{
  sigprocmask (SIG_UNBLOCK, &relevant_signal_set, NULL);
}

#if defined SIGCONT

/* Determines whether a signal is ignored.  */
static _GL_ASYNC_SAFE bool
is_ignored (int sig)
{
  struct sigaction action;

  return (sigaction (sig, NULL, &action) >= 0
          && get_handler (&action) == SIG_IGN);
}

#endif

#if HAVE_TCGETATTR

/* Write the same signal marker that the kernel would have printed if ECHO had
   been turned on.  See (4) above.
   This is a makeshift and is not perfect:
     - When stderr refers to a different target than active_control_data->fd,
       it is too hairy to write the signal marker.
     - In some cases, when the signal was generated right before and delivered
       right after a clobber_local_mode invocation, the result is that the
       marker appears twice, e.g. ^C^C.  This occurs only with a small
       probability.
     - In some cases, when the signal was generated right before and delivered
       right after a restore_local_mode invocation, the result is that the
       marker does not appear at all.  This occurs only with a small
       probability.
   To test this kind of behaviour, use the 'test-term-style-control-yes' example
   like this:
     $ ./test-term-style-control-yes
 */
static _GL_ASYNC_SAFE void
show_signal_marker (int sig)
{
  /* Write to stderr, not to active_control_data->fd, because
     active_control_data->fd is often logged or used with 'less -R'.  */
  if (active_controller != NULL && active_control_data->same_as_stderr)
    switch (sig)
      {
      /* The kernel's action when the user presses the INTR key.  */
      case SIGINT:
        full_write (STDERR_FILENO, "^C", 2); break;
      /* The kernel's action when the user presses the SUSP key.  */
      case SIGTSTP:
        full_write (STDERR_FILENO, "^Z", 2); break;
      /* The kernel's action when the user presses the QUIT key.  */
      case SIGQUIT:
        full_write (STDERR_FILENO, "^\\", 2); break;
      default: break;
      }
}

#endif

/* The main code of the signal handler for fatal signals and stopping signals.
   It is reentrant.  */
static _GL_ASYNC_SAFE void
fatal_or_stopping_signal_handler (int sig)
{
  #if HAVE_TCGETATTR
  bool echo_was_off = false;
  #endif
  /* Only do something while some output was interrupted.  */
  if (active_controller != NULL
      && active_control_data->tty_control != TTYCTL_NONE)
    {
      unsigned int i;

      /* Block the relevant signals.  This is needed, because the output
         of escape sequences below (usually through tputs invocations) is
         not reentrant.  */
      block_relevant_signals ();

      /* Restore the terminal to the default state.  */
      for (i = 0; i < 2; i++)
        active_controller->async_restore (active_user_data);
      #if HAVE_TCGETATTR
      if (active_control_data->tty_control == TTYCTL_FULL)
        {
          /* Restore the local mode, once the escape sequences output above
             have reached their destination.  */
          echo_was_off = restore_local_mode ();
        }
      #endif

      /* Unblock the relevant signals.  */
      unblock_relevant_signals ();
    }

  #if HAVE_TCGETATTR
  if (echo_was_off)
    show_signal_marker (sig);
  #endif
}

/* The signal handler for fatal signals.
   It is reentrant.  */
static _GL_ASYNC_SAFE void
fatal_signal_handler (int sig)
{
  log_signal_handler_called (sig);
  fatal_or_stopping_signal_handler (sig);
}

#if defined SIGCONT

/* The signal handler for stopping signals.
   It is reentrant.  */
static _GL_ASYNC_SAFE void
stopping_signal_handler (int sig)
{
  int saved_errno = errno;

  log_signal_handler_called (sig);
  fatal_or_stopping_signal_handler (sig);

  /* Now execute the signal's default action.
     We reinstall the handler later, during the SIGCONT handler.  */
  {
    struct sigaction action;
    action.sa_handler = SIG_DFL;
    action.sa_flags = SA_NODEFER;
    sigemptyset (&action.sa_mask);
    sigaction (sig, &action, NULL);
  }
  errno = saved_errno;
  raise (sig);
}

/* The signal handler for SIGCONT.
   It is reentrant.  */
static _GL_ASYNC_SAFE void
continuing_signal_handler (int sigcont)
{
  int saved_errno = errno;

  log_signal_handler_called (sigcont);
  update_pgrp_status ();
  /* Only do something while some output was interrupted.  */
  if (active_controller != NULL
      && active_control_data->tty_control != TTYCTL_NONE)
    {
      /* Reinstall the signals handlers removed in stopping_signal_handler.  */
      {
        unsigned int i;

        for (i = 0; i < num_job_control_signals; i++)
          {
            int sig = job_control_signals[i];

            if (sig != SIGCONT && !is_ignored (sig))
              {
                struct sigaction action;
                action.sa_handler = &stopping_signal_handler;
                /* If we get a stopping or continuing signal while executing
                   stopping_signal_handler or continuing_signal_handler, enter
                   it recursively, since it is reentrant.
                   Hence no SA_RESETHAND.  */
                action.sa_flags = SA_NODEFER;
                sigemptyset (&action.sa_mask);
                sigaction (sig, &action, NULL);
              }
          }
      }

      /* Block the relevant signals.  This is needed, because the output of
         escape sequences done inside the async_set_attributes_from_default
         call below is not reentrant.  */
      block_relevant_signals ();

      #if HAVE_TCGETATTR
      if (active_control_data->tty_control == TTYCTL_FULL)
        {
          /* Modify the local mode.  */
          clobber_local_mode ();
        }
      #endif
      /* Set the terminal attributes.  */
      active_controller->async_set_attributes_from_default (active_user_data);

      /* Unblock the relevant signals.  */
      unblock_relevant_signals ();
    }

  errno = saved_errno;
}

/* Ensure the signal handlers are installed.
   Once they are installed, we leave them installed.  It's not worth
   installing and uninstalling them each time we switch the terminal to a
   non-default state and back; instead we set active_controller to tell the
   signal handler whether it has something to do or not.  */

static void
ensure_continuing_signal_handler (void)
{
  static bool signal_handler_installed = false;

  if (!signal_handler_installed)
    {
      int sig = SIGCONT;
      struct sigaction action;
      action.sa_handler = &continuing_signal_handler;
      /* If we get a stopping or continuing signal while executing
         continuing_signal_handler, enter it recursively, since it is
         reentrant.  Hence no SA_RESETHAND.  */
      action.sa_flags = SA_NODEFER;
      sigemptyset (&action.sa_mask);
      sigaction (sig, &action, NULL);

      signal_handler_installed = true;
    }
}

#endif

static void
ensure_other_signal_handlers (void)
{
  static bool signal_handlers_installed = false;

  if (!signal_handlers_installed)
    {
      /* Install the handlers for the fatal signals.  */
      if (at_fatal_signal (fatal_signal_handler) < 0)
        xalloc_die ();

      #if defined SIGCONT

      /* Install the handlers for the stopping and continuing signals.  */
      {
        unsigned int i;

        for (i = 0; i < num_job_control_signals; i++)
          {
            int sig = job_control_signals[i];

            if (sig == SIGCONT)
              /* Already handled in ensure_continuing_signal_handler.  */
              ;
            else if (!is_ignored (sig))
              {
                struct sigaction action;
                action.sa_handler = &stopping_signal_handler;
                /* If we get a stopping or continuing signal while executing
                   stopping_signal_handler, enter it recursively, since it is
                   reentrant.  Hence no SA_RESETHAND.  */
                action.sa_flags = SA_NODEFER;
                sigemptyset (&action.sa_mask);
                sigaction (sig, &action, NULL);
              }
            #if DEBUG_SIGNALS
            else
              {
                fprintf (stderr, "Signal %d is ignored. Not installing a handler!\n",
                         sig);
                fflush (stderr);
              }
            #endif
          }
      }

      #endif

      signal_handlers_installed = true;
    }
}


/* ============================== Public API ============================== */

void
activate_term_non_default_mode (const struct term_style_controller *controller,
                                struct term_style_user_data *user_data)
{
  struct term_style_control_data *control_data =
    controller->get_control_data (user_data);

  if (!control_data->non_default_active)
    {
      if (control_data->tty_control != TTYCTL_NONE)
        ensure_other_signal_handlers ();

      #if BLOCK_SIGNALS_DURING_NON_DEFAULT_STYLE_OUTPUT
      /* Block fatal signals, so that a SIGINT or similar doesn't interrupt
         us without the possibility of restoring the terminal's state.
         Likewise for SIGTSTP etc.  */
      block_relevant_signals ();
      #endif

      /* Enable the exit handler for restoring the terminal's state,
         and make the signal handlers effective.  */
      if (active_controller != NULL)
        {
          /* We can't support two active controllers with non-default
             attributes at the same time.  */
          abort ();
        }
      /* The uses of 'volatile' (and ISO C 99 section 5.1.2.3.(5)) ensure that
         we set active_controller to a non-NULL value only after the memory
         locations active_user_data, active_control_data, active_fd have been
         filled.  */
      active_fd = control_data->fd;
      active_control_data = control_data;
      active_user_data = user_data;
      active_controller = controller;

      #if HAVE_TCGETATTR
      /* Now that the signal handlers are effective, modify the tty.  */
      if (active_control_data->tty_control == TTYCTL_FULL)
        {
          /* Modify the local mode.  */
          clobber_local_mode ();
        }
      #endif

      control_data->non_default_active = true;
    }
}

void
deactivate_term_non_default_mode (const struct term_style_controller *controller,
                                  struct term_style_user_data *user_data)
{
  struct term_style_control_data *control_data =
    controller->get_control_data (user_data);

  if (control_data->non_default_active)
    {
      #if HAVE_TCGETATTR
      /* Before we make the signal handlers ineffective, modify the tty.  */
      if (active_control_data->tty_control == TTYCTL_FULL)
        {
          /* Restore the local mode, once the tputs calls from out_attr_change
             have reached their destination.  */
          restore_local_mode ();
        }
      #endif

      /* Disable the exit handler, and make the signal handlers ineffective.  */
      /* The uses of 'volatile' (and ISO C 99 section 5.1.2.3.(5)) ensure that
         we reset active_user_data, active_control_data, active_fd only after
         the memory location active_controller has been cleared.  */
      active_controller = NULL;
      active_user_data = NULL;
      active_control_data = NULL;
      active_fd = -1;

      #if BLOCK_SIGNALS_DURING_NON_DEFAULT_STYLE_OUTPUT
      /* Unblock the relevant signals.  */
      unblock_relevant_signals ();
      #endif

      control_data->non_default_active = false;
    }
}

void
activate_term_style_controller (const struct term_style_controller *controller,
                                struct term_style_user_data *user_data,
                                int fd, ttyctl_t tty_control)
{
  struct term_style_control_data *control_data =
    controller->get_control_data (user_data);

  control_data->fd = fd;

  /* Prepare tty control.  */
  if (tty_control == TTYCTL_AUTO)
    tty_control = TTYCTL_FULL;
  control_data->tty_control = tty_control;
  if (control_data->tty_control != TTYCTL_NONE)
    init_relevant_signal_set ();
  #if HAVE_TCGETATTR
  if (control_data->tty_control == TTYCTL_FULL)
    {
      struct stat statbuf1;
      struct stat statbuf2;
      if (fd == STDERR_FILENO
          || (fstat (fd, &statbuf1) >= 0
              && fstat (STDERR_FILENO, &statbuf2) >= 0
              && psame_inode (&statbuf1, &statbuf2)))
        control_data->same_as_stderr = true;
      else
        control_data->same_as_stderr = false;
    }
  else
    /* This value is actually not used.  */
    control_data->same_as_stderr = false;
  #endif

  control_data->non_default_active = false;

  /* Start keeping track of the process group status.  */
  term_fd = fd;
  #if defined SIGCONT
  ensure_continuing_signal_handler ();
  #endif
  update_pgrp_status ();

  /* Register an exit handler.  */
  {
    static bool registered = false;
    if (!registered)
      {
        atexit (atexit_handler);
        registered = true;
      }
  }
}

void
deactivate_term_style_controller (const struct term_style_controller *controller,
                                  struct term_style_user_data *user_data)
{
  struct term_style_control_data *control_data =
    controller->get_control_data (user_data);

  /* Verify that the non-default attributes mode is turned off.  */
  if (control_data->non_default_active)
    abort ();

  term_fd = -1;
  update_pgrp_status ();
}
