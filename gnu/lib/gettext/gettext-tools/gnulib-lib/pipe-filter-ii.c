/* Filtering of data through a subprocess.
   Copyright (C) 2001-2003, 2008-2023 Free Software Foundation, Inc.
   Written by Bruno Haible <bruno@clisp.org>, 2009.

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

#include "pipe-filter.h"

#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#if defined _WIN32 && ! defined __CYGWIN__
# include <windows.h>
# include <process.h> /* _beginthreadex, _endthreadex */
#elif defined __KLIBC__
# define INCL_DOS
# include <os2.h>

/* Simple implementation of Win32 APIs */

# define WINAPI

typedef struct _HANDLE
{
  TID tid;
  HEV hevDone;
  unsigned int WINAPI (*start) (void *);
  void *arg;
} *HANDLE;

typedef ULONG DWORD;

static void
start_wrapper (void *arg)
{
  HANDLE h = (HANDLE) arg;

  h->start (h->arg);

  DosPostEventSem (h->hevDone);
  _endthread ();
}

static HANDLE
_beginthreadex (void *s, unsigned n, unsigned int WINAPI (*start) (void *),
                void *arg, unsigned fl, unsigned *th)
{
  HANDLE h;

  h = malloc (sizeof (*h));
  if (!h)
    return NULL;

  if (DosCreateEventSem (NULL, &h->hevDone, 0, FALSE))
    goto exit_free;

  h->start = start;
  h->arg = arg;

  h->tid = _beginthread (start_wrapper, NULL, n, (void *) h);
  if (h->tid == -1)
    goto exit_close_event_sem;

  return h;

 exit_close_event_sem:
  DosCloseEventSem (h->hevDone);

 exit_free:
  free (h);

  return NULL;
}

static BOOL
CloseHandle (HANDLE h)
{
  DosCloseEventSem (h->hevDone);
  free (h);
}

# define _endthreadex(x) return (x)
# define TerminateThread(h, e) DosKillThread (h->tid)

# define GetLastError()  (-1)

# ifndef ERROR_NO_DATA
#  define ERROR_NO_DATA 232
# endif

# define INFINITE SEM_INDEFINITE_WAIT
# define WAIT_OBJECT_0  0

static DWORD
WaitForSingleObject (HANDLE h, DWORD ms)
{
  return DosWaitEventSem (h->hevDone, ms) == 0 ? WAIT_OBJECT_0 : (DWORD) -1;
}

static DWORD
WaitForMultipleObjects (DWORD nCount, const HANDLE *pHandles, BOOL bWaitAll,
                        DWORD ms)
{
  HMUX hmux;
  PSEMRECORD psr;
  ULONG ulUser;
  ULONG rc = (ULONG) -1;
  DWORD i;

  psr = malloc (sizeof (*psr) * nCount);
  if (!psr)
    goto exit_return;

  for (i = 0; i < nCount; ++i)
    {
      psr[i].hsemCur = (HSEM) pHandles[i]->hevDone;
      psr[i].ulUser  = WAIT_OBJECT_0 + i;
    }

  if (DosCreateMuxWaitSem (NULL, &hmux, nCount, psr,
                           bWaitAll ? DCMW_WAIT_ALL : DCMW_WAIT_ANY))
    goto exit_free;

  rc = DosWaitMuxWaitSem (hmux, ms, &ulUser);
  DosCloseMuxWaitSem (hmux);

 exit_free:
  free (psr);

 exit_return:
  if (rc)
    return (DWORD) -1;

  return ulUser;
}
#else
# include <signal.h>
# include <sys/select.h>
#endif

#include "error.h"
#include "spawn-pipe.h"
#include "wait-process.h"
#include "gettext.h"

#define _(str) gettext (str)

#include "pipe-filter-aux.h"

#if (defined _WIN32 && ! defined __CYGWIN__) || defined __KLIBC__

struct locals
{
  /* Arguments passed to pipe_filter_ii_execute.  */
  prepare_write_fn prepare_write;
  done_write_fn done_write;
  prepare_read_fn prepare_read;
  done_read_fn done_read;

  /* Management of the subprocess.  */
  void *private_data;
  int fd[2];

  /* Status of the writer part.  */
  volatile bool writer_terminated;
  volatile int writer_errno;
  /* Status of the reader part.  */
  volatile bool reader_terminated;
  volatile int reader_errno;
};

static unsigned int WINAPI
writer_thread_func (void *thread_arg)
{
  struct locals *l = (struct locals *) thread_arg;

  for (;;)
    {
      size_t bufsize;
      const void *buf = l->prepare_write (&bufsize, l->private_data);
      if (buf != NULL)
        {
          ssize_t nwritten =
            write (l->fd[1], buf, bufsize > SSIZE_MAX ? SSIZE_MAX : bufsize);
          if (nwritten < 0)
            {
              /* Don't assume that the gnulib modules 'write' and 'sigpipe' are
                 used.  */
              if (GetLastError () == ERROR_NO_DATA)
                errno = EPIPE;
              l->writer_errno = errno;
              break;
            }
          else if (nwritten > 0)
            l->done_write ((void *) buf, nwritten, l->private_data);
        }
      else
        break;
    }

  l->writer_terminated = true;
  _endthreadex (0); /* calls ExitThread (0) */
  abort ();
}

static unsigned int WINAPI
reader_thread_func (void *thread_arg)
{
  struct locals *l = (struct locals *) thread_arg;

  for (;;)
    {
      size_t bufsize;
      void *buf = l->prepare_read (&bufsize, l->private_data);
      if (!(buf != NULL && bufsize > 0))
        /* prepare_read returned wrong values.  */
        abort ();
      {
        ssize_t nread =
          read (l->fd[0], buf, bufsize > SSIZE_MAX ? SSIZE_MAX : bufsize);
        if (nread < 0)
          {
            l->reader_errno = errno;
            break;
          }
        else if (nread > 0)
          l->done_read (buf, nread, l->private_data);
        else /* nread == 0 */
          break;
      }
    }

  l->reader_terminated = true;
  _endthreadex (0); /* calls ExitThread (0) */
  abort ();
}

#endif

int
pipe_filter_ii_execute (const char *progname,
                        const char *prog_path, const char * const *prog_argv,
                        bool null_stderr, bool exit_on_error,
                        prepare_write_fn prepare_write,
                        done_write_fn done_write,
                        prepare_read_fn prepare_read,
                        done_read_fn done_read,
                        void *private_data)
{
  pid_t child;
  int fd[2];
#if !((defined _WIN32 && ! defined __CYGWIN__) || defined __KLIBC__)
  struct sigaction orig_sigpipe_action;
#endif

  /* Open a bidirectional pipe to a subprocess.  */
  child = create_pipe_bidi (progname, prog_path, prog_argv,
                            NULL, null_stderr, true, exit_on_error,
                            fd);
  if (child == -1)
    return -1;

#if (defined _WIN32 && ! defined __CYGWIN__) || defined __KLIBC__
  /* Native Windows API.  */
  /* Pipes have a non-blocking mode, see function SetNamedPipeHandleState and
     the article "Named Pipe Type, Read, and Wait Modes", but Microsoft's
     documentation discourages its use.  So don't use it.
     Asynchronous I/O is also not suitable because it notifies the caller only
     about completion of the I/O request, not about intermediate progress.
     So do the writing and the reading in separate threads.  */
  {
    struct locals l;
    HANDLE handles[2];
    #define writer_thread_handle handles[0]
    #define reader_thread_handle handles[1]
    bool writer_cleaned_up;
    bool reader_cleaned_up;

    l.prepare_write = prepare_write;
    l.done_write = done_write;
    l.prepare_read = prepare_read;
    l.done_read = done_read;
    l.private_data = private_data;
    l.fd[0] = fd[0];
    l.fd[1] = fd[1];
    l.writer_terminated = false;
    l.writer_errno = 0;
    l.reader_terminated = false;
    l.reader_errno = 0;

    writer_thread_handle =
      (HANDLE) _beginthreadex (NULL, 100000, writer_thread_func, &l, 0, NULL);
    reader_thread_handle =
      (HANDLE) _beginthreadex (NULL, 100000, reader_thread_func, &l, 0, NULL);
    if (writer_thread_handle == NULL || reader_thread_handle == NULL)
      {
        if (exit_on_error)
          error (EXIT_FAILURE, 0, _("creation of threads failed"));
        if (reader_thread_handle != NULL)
          CloseHandle (reader_thread_handle);
        if (writer_thread_handle != NULL)
          CloseHandle (writer_thread_handle);
        goto fail;
      }
    writer_cleaned_up = false;
    reader_cleaned_up = false;
    for (;;)
      {
        DWORD ret;

        /* Here !(writer_cleaned_up && reader_cleaned_up).  */
        if (writer_cleaned_up)
          ret = WaitForSingleObject (reader_thread_handle, INFINITE);
        else if (reader_cleaned_up)
          ret = WaitForSingleObject (writer_thread_handle, INFINITE);
        else
          ret = WaitForMultipleObjects (2, handles, FALSE, INFINITE);
        if (!(ret == WAIT_OBJECT_0 + 0 || ret == WAIT_OBJECT_0 + 1))
          abort ();

        if (l.writer_terminated)
          {
            /* The writer thread has just terminated.  */
            l.writer_terminated = false;
            CloseHandle (writer_thread_handle);
            if (l.writer_errno)
              {
                if (exit_on_error)
                  error (EXIT_FAILURE, l.writer_errno,
                         _("write to %s subprocess failed"), progname);
                if (!reader_cleaned_up)
                  {
                    TerminateThread (reader_thread_handle, 1);
                    CloseHandle (reader_thread_handle);
                  }
                goto fail;
              }
            /* Tell the child there is nothing more the parent will send.  */
            close (fd[1]);
            writer_cleaned_up = true;
          }
        if (l.reader_terminated)
          {
            /* The reader thread has just terminated.  */
            l.reader_terminated = false;
            CloseHandle (reader_thread_handle);
            if (l.reader_errno)
              {
                if (exit_on_error)
                  error (EXIT_FAILURE, l.reader_errno,
                         _("read from %s subprocess failed"), progname);
                if (!writer_cleaned_up)
                  {
                    TerminateThread (writer_thread_handle, 1);
                    CloseHandle (writer_thread_handle);
                  }
                goto fail;
              }
            reader_cleaned_up = true;
          }
        if (writer_cleaned_up && reader_cleaned_up)
          break;
      }
  }
#else
  /* When we write to the child process and it has just terminated,
     we don't want to die from a SIGPIPE signal.  So set the SIGPIPE
     handler to SIG_IGN, and handle EPIPE error codes in write().  */
  {
    struct sigaction sigpipe_action;

    sigpipe_action.sa_handler = SIG_IGN;
    sigpipe_action.sa_flags = 0;
    sigemptyset (&sigpipe_action.sa_mask);
    if (sigaction (SIGPIPE, &sigpipe_action, &orig_sigpipe_action) < 0)
      abort ();
  }

  {
# if HAVE_SELECT
    fd_set readfds;  /* All bits except fd[0] are always cleared.  */
    fd_set writefds; /* All bits except fd[1] are always cleared.  */
# endif
    bool done_writing;

    /* Enable non-blocking I/O.  This permits the read() and write() calls
       to return -1/EAGAIN without blocking; this is important for polling
       if HAVE_SELECT is not defined.  It also permits the read() and write()
       calls to return after partial reads/writes; this is important if
       HAVE_SELECT is defined, because select() only says that some data
       can be read or written, not how many.  Without non-blocking I/O,
       Linux 2.2.17 and BSD systems prefer to block instead of returning
       with partial results.  */
    {
      int fcntl_flags;

      if ((fcntl_flags = fcntl (fd[1], F_GETFL, 0)) < 0
          || fcntl (fd[1], F_SETFL, fcntl_flags | O_NONBLOCK) == -1
          || (fcntl_flags = fcntl (fd[0], F_GETFL, 0)) < 0
          || fcntl (fd[0], F_SETFL, fcntl_flags | O_NONBLOCK) == -1)
        {
          if (exit_on_error)
            error (EXIT_FAILURE, errno,
                   _("cannot set up nonblocking I/O to %s subprocess"),
                   progname);
          goto fail;
        }
    }

# if HAVE_SELECT
    FD_ZERO (&readfds);
    FD_ZERO (&writefds);
# endif
    done_writing = false;
    for (;;)
      {
# if HAVE_SELECT
        int n, retval;

        FD_SET (fd[0], &readfds);
        n = fd[0] + 1;
        if (!done_writing)
          {
            FD_SET (fd[1], &writefds);
            if (n <= fd[1])
              n = fd[1] + 1;
          }

        /* Do EINTR handling here instead of in pipe-filter-aux.h,
           because select() cannot be referred to from an inline
           function on AIX 7.1.  */
        do
          retval = select (n, &readfds, (!done_writing ? &writefds : NULL),
                           NULL, NULL);
        while (retval < 0 && errno == EINTR);
        n = retval;

        if (n < 0)
          {
            if (exit_on_error)
              error (EXIT_FAILURE, errno,
                     _("communication with %s subprocess failed"), progname);
            goto fail;
          }
        if (!done_writing && FD_ISSET (fd[1], &writefds))
          goto try_write;
        if (FD_ISSET (fd[0], &readfds))
          goto try_read;
        /* How could select() return if none of the two descriptors is ready?  */
        abort ();
# endif

        /* Attempt to write.  */
# if HAVE_SELECT
      try_write:
# endif
        if (!done_writing)
          {
            size_t bufsize;
            const void *buf = prepare_write (&bufsize, private_data);
            if (buf != NULL)
              {
                /* Writing to a pipe in non-blocking mode is tricky: The
                   write() call may fail with EAGAIN, simply because sufficient
                   space is not available in the pipe. See POSIX:2008
                   <https://pubs.opengroup.org/onlinepubs/9699919799/functions/write.html>.
                   This happens actually on AIX and IRIX, when bufsize >= 8192
                   (even though PIPE_BUF and pathconf ("/", _PC_PIPE_BUF) are
                   both 32768).  */
                size_t attempt_to_write =
                  (bufsize > SSIZE_MAX ? SSIZE_MAX : bufsize);
                for (;;)
                  {
                    ssize_t nwritten = write (fd[1], buf, attempt_to_write);
                    if (nwritten < 0)
                      {
                        if (errno == EAGAIN)
                          {
                            attempt_to_write = attempt_to_write / 2;
                            if (attempt_to_write == 0)
                              break;
                          }
                        else if (!IS_EAGAIN (errno))
                          {
                            if (exit_on_error)
                              error (EXIT_FAILURE, errno,
                                     _("write to %s subprocess failed"),
                                     progname);
                            goto fail;
                          }
                      }
                    else
                      {
                        if (nwritten > 0)
                          done_write ((void *) buf, nwritten, private_data);
                        break;
                      }
                  }
              }
            else
              {
                /* Tell the child there is nothing more the parent will send.  */
                close (fd[1]);
                done_writing = true;
              }
          }
# if HAVE_SELECT
        continue;
# endif

        /* Attempt to read.  */
# if HAVE_SELECT
      try_read:
# endif
        {
          size_t bufsize;
          void *buf = prepare_read (&bufsize, private_data);
          if (!(buf != NULL && bufsize > 0))
            /* prepare_read returned wrong values.  */
            abort ();
          {
            ssize_t nread =
              read (fd[0], buf, bufsize > SSIZE_MAX ? SSIZE_MAX : bufsize);
            if (nread < 0)
              {
                if (!IS_EAGAIN (errno))
                  {
                    if (exit_on_error)
                      error (EXIT_FAILURE, errno,
                             _("read from %s subprocess failed"), progname);
                    goto fail;
                  }
              }
            else if (nread > 0)
              done_read (buf, nread, private_data);
            else /* nread == 0 */
              {
                if (done_writing)
                  break;
              }
          }
        }
# if HAVE_SELECT
        continue;
# endif
      }
  }

  /* Restore SIGPIPE signal handler.  */
  if (sigaction (SIGPIPE, &orig_sigpipe_action, NULL) < 0)
    abort ();
#endif

  close (fd[0]);

  /* Remove zombie process from process list.  */
  {
    int exitstatus =
      wait_subprocess (child, progname, false, null_stderr,
                       true, exit_on_error, NULL);
    if (exitstatus != 0 && exit_on_error)
      error (EXIT_FAILURE, 0, _("%s subprocess terminated with exit code %d"),
             progname, exitstatus);
    return exitstatus;
  }

 fail:
  {
    int saved_errno = errno;
    close (fd[1]);
#if !((defined _WIN32 && ! defined __CYGWIN__) || defined __KLIBC__)
    if (sigaction (SIGPIPE, &orig_sigpipe_action, NULL) < 0)
      abort ();
#endif
    close (fd[0]);
    wait_subprocess (child, progname, true, true, true, false, NULL);
    errno = saved_errno;
    return -1;
  }
}
