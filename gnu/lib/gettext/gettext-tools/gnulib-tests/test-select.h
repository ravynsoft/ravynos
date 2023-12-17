/* Test of select() substitute.
   Copyright (C) 2008-2023 Free Software Foundation, Inc.

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

/* Written by Paolo Bonzini, 2008.  */

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <errno.h>

#include "macros.h"

#if defined _WIN32 && ! defined __CYGWIN__
# define WINDOWS_NATIVE
#endif

#ifdef HAVE_SYS_WAIT_H
# include <sys/wait.h>
#endif

/* Tell GCC not to warn about the specific edge cases tested here.  */
#if __GNUC__ >= 13
# pragma GCC diagnostic ignored "-Wanalyzer-fd-use-without-check"
#endif

typedef int (*select_fn) (int, fd_set *, fd_set *, fd_set *, struct timeval *);


/* Minimal testing infrastructure.  */

static int failures;

static void
failed (const char *reason)
{
  if (++failures > 1)
    printf ("  ");
  printf ("failed (%s)\n", reason);
}

static int
test (void (*fn) (select_fn), select_fn my_select, const char *msg)
{
  failures = 0;
  printf ("%s... ", msg);
  fflush (stdout);
  fn (my_select);

  if (!failures)
    printf ("passed\n");

  return failures;
}


/* Funny socket code.  */

static int
open_server_socket (void)
{
  int s, x;
  struct sockaddr_in ia;

  s = socket (AF_INET, SOCK_STREAM, 0);

  x = 1;
  setsockopt (s, SOL_SOCKET, SO_REUSEPORT, &x, sizeof (x));

  memset (&ia, 0, sizeof (ia));
  ia.sin_family = AF_INET;
  inet_pton (AF_INET, "127.0.0.1", &ia.sin_addr);
  ia.sin_port = htons (TEST_PORT);
  if (bind (s, (struct sockaddr *) &ia, sizeof (ia)) < 0)
    {
      perror ("bind");
      exit (77);
    }

  if (listen (s, 1) < 0)
    {
      perror ("listen");
      exit (77);
    }

  return s;
}

static int
connect_to_socket (bool blocking)
{
  int s;
  struct sockaddr_in ia;

  s = socket (AF_INET, SOCK_STREAM, 0);

  memset (&ia, 0, sizeof (ia));
  ia.sin_family = AF_INET;
  inet_pton (AF_INET, "127.0.0.1", &ia.sin_addr);
  ia.sin_port = htons (TEST_PORT);

  if (!blocking)
    {
#ifdef WINDOWS_NATIVE
      unsigned long iMode = 1;
      ioctl (s, FIONBIO, (char *) &iMode);

#elif defined F_GETFL
      int oldflags = fcntl (s, F_GETFL, NULL);

      if (!(oldflags & O_NONBLOCK))
        fcntl (s, F_SETFL, oldflags | O_NONBLOCK);
#endif
    }

  if (connect (s, (struct sockaddr *) &ia, sizeof (ia)) < 0
      && (blocking || errno != EINPROGRESS))
    {
      perror ("connect");
      exit (77);
    }

  return s;
}


/* A slightly more convenient interface to select(2).
   Waits until a specific event occurs on a file descriptor FD.
   EV is a bit mask of events to look for:
     SEL_IN - input can be polled without blocking,
     SEL_OUT - output can be provided without blocking,
     SEL_EXC - an exception occurred,
   A maximum wait time is specified by TIMEOUT.
   *TIMEOUT = { 0, 0 } means to return immediately,
   TIMEOUT = NULL means to wait indefinitely.  */

enum { SEL_IN = 1, SEL_OUT = 2, SEL_EXC = 4 };

static int
do_select (int fd, int ev, struct timeval *timeout, select_fn my_select)
{
  fd_set rfds, wfds, xfds;
  int r, rev;

  FD_ZERO (&rfds);
  FD_ZERO (&wfds);
  FD_ZERO (&xfds);
  if (ev & SEL_IN)
    FD_SET (fd, &rfds);
  if (ev & SEL_OUT)
    FD_SET (fd, &wfds);
  if (ev & SEL_EXC)
    FD_SET (fd, &xfds);
  r = my_select (fd + 1, &rfds, &wfds, &xfds, timeout);
  if (r < 0)
    return r;

  rev = 0;
  if (FD_ISSET (fd, &rfds))
    rev |= SEL_IN;
  if (FD_ISSET (fd, &wfds))
    rev |= SEL_OUT;
  if (FD_ISSET (fd, &xfds))
    rev |= SEL_EXC;
  if (rev && r == 0)
    failed ("select returned 0");
  if (rev & ~ev)
    failed ("select returned unrequested events");

  return rev;
}

static int
do_select_nowait (int fd, int ev, select_fn my_select)
{
  struct timeval tv0;
  tv0.tv_sec = 0;
  tv0.tv_usec = 0;
  return do_select (fd, ev, &tv0, my_select);
}

static int
do_select_wait (int fd, int ev, select_fn my_select)
{
  return do_select (fd, ev, NULL, my_select);
}


/* Test select(2) for TTYs.  */

#ifdef INTERACTIVE
static void
test_tty (select_fn my_select)
{
  if (do_select_nowait (0, SEL_IN, my_select) != 0)
    failed ("can read");
  if (do_select_nowait (0, SEL_OUT, my_select) == 0)
    failed ("cannot write");

  if (do_select_wait (0, SEL_IN, my_select) == 0)
    failed ("return with infinite timeout");

  getchar ();
  if (do_select_nowait (0, SEL_IN, my_select) != 0)
    failed ("can read after getc");
}
#endif


static int
do_select_bad_nfd_nowait (int nfd, select_fn my_select)
{
  struct timeval tv0;
  tv0.tv_sec = 0;
  tv0.tv_usec = 0;
  errno = 0;
  return my_select (nfd, NULL, NULL, NULL, &tv0);
}

static void
test_bad_nfd (select_fn my_select)
{
  if (do_select_bad_nfd_nowait (-1, my_select) != -1 || errno != EINVAL)
    failed ("invalid errno after negative nfds");
  /* Can't test FD_SETSIZE + 1 for EINVAL, since some systems allow
     dynamically larger set size by redefining FD_SETSIZE anywhere up
     to the actual maximum fd.  */
#if 0
  if (do_select_bad_nfd_nowait (FD_SETSIZE + 1, my_select) != -1
      || errno != EINVAL)
    failed ("invalid errno after bogus nfds");
#endif
}

/* Test select(2) on invalid file descriptors.  */

static int
do_select_bad_fd (int fd, int ev, struct timeval *timeout, select_fn my_select)
{
  fd_set rfds, wfds, xfds;

  FD_ZERO (&rfds);
  FD_ZERO (&wfds);
  FD_ZERO (&xfds);
  if (ev & SEL_IN)
    FD_SET (fd, &rfds);
  if (ev & SEL_OUT)
    FD_SET (fd, &wfds);
  if (ev & SEL_EXC)
    FD_SET (fd, &xfds);
  errno = 0;
  return my_select (fd + 1, &rfds, &wfds, &xfds, timeout);
  /* In this case, when fd is invalid, on some platforms, the bit for fd
     is left alone in the fd_set, whereas on other platforms it is cleared.
     So, don't check the bit for fd here.  */
}

static int
do_select_bad_fd_nowait (int fd, int ev, select_fn my_select)
{
  struct timeval tv0;
  tv0.tv_sec = 0;
  tv0.tv_usec = 0;
  return do_select_bad_fd (fd, ev, &tv0, my_select);
}

static void
test_bad_fd (select_fn my_select)
{
  /* This tests fails on OSF/1 and native Windows, even with fd = 16.  */
#if !(defined __osf__ || defined WINDOWS_NATIVE)
  int fd;

  /* On Linux, Mac OS X, *BSD, values of fd like 99 or 399 are discarded
     by the kernel early and therefore do *not* lead to EBADF, as required
     by POSIX.  */
# if defined __linux__ || (defined __APPLE__ && defined __MACH__) || (defined __FreeBSD__ || defined __DragonFly__) || defined __OpenBSD__ || defined __NetBSD__
  fd = 14;
# else
  fd = 99;
# endif
  /* Even on the best POSIX compliant platforms, values of fd >= FD_SETSIZE
     require an nfds argument that is > FD_SETSIZE and thus may lead to EINVAL,
     not EBADF.  */
  if (fd >= FD_SETSIZE)
    fd = FD_SETSIZE - 1;
  close (fd);

  if (do_select_bad_fd_nowait (fd, SEL_IN, my_select) == 0 || errno != EBADF)
    failed ("invalid fd among rfds");
  if (do_select_bad_fd_nowait (fd, SEL_OUT, my_select) == 0 || errno != EBADF)
    failed ("invalid fd among wfds");
  if (do_select_bad_fd_nowait (fd, SEL_EXC, my_select) == 0 || errno != EBADF)
    failed ("invalid fd among xfds");
#endif
}


/* Test select(2) for unconnected nonblocking sockets.  */

static void
test_connect_first (select_fn my_select)
{
  int s = open_server_socket ();
  struct sockaddr_in ia;
  socklen_t addrlen;

  int c1, c2;

  if (do_select_nowait (s, SEL_IN | SEL_EXC, my_select) != 0)
    failed ("can read, socket not connected");

  c1 = connect_to_socket (false);

  if (do_select_wait (s, SEL_IN | SEL_EXC, my_select) != SEL_IN)
    failed ("expecting readability on passive socket");
  if (do_select_nowait (s, SEL_IN | SEL_EXC, my_select) != SEL_IN)
    failed ("expecting readability on passive socket");

  addrlen = sizeof (ia);
  c2 = accept (s, (struct sockaddr *) &ia, &addrlen);
  ASSERT (close (s) == 0);
  ASSERT (close (c1) == 0);
  ASSERT (close (c2) == 0);
}


/* Test select(2) for unconnected blocking sockets.  */

static void
test_accept_first (select_fn my_select)
{
#ifndef WINDOWS_NATIVE
  int s = open_server_socket ();
  struct sockaddr_in ia;
  socklen_t addrlen;
  char buf[3];
  int c, pid;

  pid = fork ();
  if (pid < 0)
    return;

  if (pid == 0)
    {
      addrlen = sizeof (ia);
      c = accept (s, (struct sockaddr *) &ia, &addrlen);
      ASSERT (close (s) == 0);
      ASSERT (write (c, "foo", 3) == 3);
      ASSERT (read (c, buf, 3) == 3);
      shutdown (c, SHUT_RD);
      ASSERT (close (c) == 0);
      exit (0);
    }
  else
    {
      ASSERT (close (s) == 0);
      c = connect_to_socket (true);
      if (do_select_nowait (c, SEL_OUT, my_select) != SEL_OUT)
        failed ("cannot write after blocking connect");
      ASSERT (write (c, "foo", 3) == 3);
      wait (&pid);
      if (do_select_wait (c, SEL_IN, my_select) != SEL_IN)
        failed ("cannot read data left in the socket by closed process");
      ASSERT (read (c, buf, 3) == 3);
      ASSERT (write (c, "foo", 3) == 3);
      (void) close (c); /* may fail with errno = ECONNRESET */
    }
#endif
}


/* Common code for pipes and connected sockets.  */

static void
test_pair (int rd, int wd, select_fn my_select)
{
  char buf[3];
  if (do_select_wait (wd, SEL_IN | SEL_OUT | SEL_EXC, my_select) != SEL_OUT)
    failed ("expecting writability before writing");
  if (do_select_nowait (wd, SEL_IN | SEL_OUT | SEL_EXC, my_select) != SEL_OUT)
    failed ("expecting writability before writing");

  ASSERT (write (wd, "foo", 3) == 3);
  if (do_select_wait (rd, SEL_IN, my_select) != SEL_IN)
    failed ("expecting readability after writing");
  if (do_select_nowait (rd, SEL_IN, my_select) != SEL_IN)
    failed ("expecting readability after writing");

  ASSERT (read (rd, buf, 3) == 3);
}


/* Test select(2) on connected sockets.  */

static void
test_socket_pair (select_fn my_select)
{
  struct sockaddr_in ia;

  socklen_t addrlen = sizeof (ia);
  int s = open_server_socket ();
  int c1 = connect_to_socket (false);
  int c2 = accept (s, (struct sockaddr *) &ia, &addrlen);

  ASSERT (close (s) == 0);

  test_pair (c1, c2, my_select);
  ASSERT (close (c1) == 0);
  ASSERT (write (c2, "foo", 3) == 3);
  (void) close (c2); /* may fail with errno = ECONNRESET */
}


/* Test select(2) on pipes.  */

static void
test_pipe (select_fn my_select)
{
  int fd[2];

  ASSERT (pipe (fd) == 0);
  test_pair (fd[0], fd[1], my_select);
  ASSERT (close (fd[0]) == 0);
  ASSERT (close (fd[1]) == 0);
}


/* Do them all.  */

static int
test_function (select_fn my_select)
{
  int result = 0;

#ifdef INTERACTIVE
  printf ("Please press Enter\n");
  test (test_tty, "TTY", my_select);
#endif

  result += test (test_bad_nfd, my_select, "Invalid nfd test");
  result += test (test_bad_fd, my_select, "Invalid fd test");
  result += test (test_connect_first, my_select, "Unconnected socket test");
  result += test (test_socket_pair, my_select, "Connected sockets test");
  result += test (test_accept_first, my_select, "General socket test with fork");
  result += test (test_pipe, my_select, "Pipe test");

  return result;
}
