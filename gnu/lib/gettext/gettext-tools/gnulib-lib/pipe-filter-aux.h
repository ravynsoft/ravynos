/* Auxiliary code for filtering of data through a subprocess.
   Copyright (C) 2001-2003, 2008-2023 Free Software Foundation, Inc.
   Written by Bruno Haible <haible@clisp.cons.org>, 2009.

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

/* This file uses _GL_INLINE_HEADER_BEGIN, _GL_INLINE.  */
#if !_GL_CONFIG_H_INCLUDED
 #error "Please include config.h first."
#endif

_GL_INLINE_HEADER_BEGIN
#ifndef PIPE_FILTER_AUX_INLINE
# define PIPE_FILTER_AUX_INLINE _GL_INLINE
#endif

#if defined _WIN32 && ! defined __CYGWIN__
/* In the pipe-filter-* modules we want to use the write() function that is
   not overridden to emulate SIGPIPE behaviour, because we don't want force
   the caller to do
     signal (SIGPIPE, SIG_DFL);
   To reproduce the problem, use a gnulib testdir for the modules
   'pipe-filter-gi', 'write', 'sigpipe'.  */
# undef write
# define write _write
#endif

#ifndef SSIZE_MAX
# define SSIZE_MAX ((ssize_t) (SIZE_MAX / 2))
#endif
#ifdef _AIX
/* On AIX, despite having select() and despite having put the file descriptor
   in non-blocking mode, it can happen that select() reports that fd[1] is
   writable but writing a large amount of data to fd[1] then fails with errno
   EAGAIN.  Seen with test-pipe-filter-gi1 on AIX 7.2, with data sizes of
   29 KB.  So, limit the size of data passed to the write() call to 4 KB.  */
# undef SSIZE_MAX
# define SSIZE_MAX 4096
#endif

/* We use a child process, and communicate through a bidirectional pipe.
   To avoid deadlocks, let the child process decide when it wants to read
   or to write, and let the parent behave accordingly.  The parent uses
   select() to know whether it must write or read.  On platforms without
   select(), we use non-blocking I/O.  (This means the parent is busy
   looping while waiting for the child.  Not good.  But hardly any platform
   lacks select() nowadays.)  */

/* On BeOS and OS/2 kLIBC select() works only on sockets, not on normal file
   descriptors.  */
#if defined __BEOS__ || defined __KLIBC__
# undef HAVE_SELECT
#endif

#ifdef EINTR

/* EINTR handling for close(), read(), write(), select().
   These functions can return -1/EINTR even though we don't have any
   signal handlers set up, namely when we get interrupted via SIGSTOP.  */

PIPE_FILTER_AUX_INLINE int
nonintr_close (int fd)
{
  int retval;

  do
    retval = close (fd);
  while (retval < 0 && errno == EINTR);

  return retval;
}
#undef close /* avoid warning related to gnulib module unistd */
#define close nonintr_close

PIPE_FILTER_AUX_INLINE ssize_t
nonintr_read (int fd, void *buf, size_t count)
{
  ssize_t retval;

  do
    retval = read (fd, buf, count);
  while (retval < 0 && errno == EINTR);

  return retval;
}
#undef read /* avoid warning related to gnulib module unistd */
#define read nonintr_read

PIPE_FILTER_AUX_INLINE ssize_t
nonintr_write (int fd, const void *buf, size_t count)
{
  ssize_t retval;

  do
    retval = write (fd, buf, count);
  while (retval < 0 && errno == EINTR);

  return retval;
}
#undef write /* avoid warning on VMS */
#define write nonintr_write

#endif

/* Non-blocking I/O.  */
#if HAVE_SELECT
# define IS_EAGAIN(errcode) 0
#else
# ifdef EWOULDBLOCK
#  define IS_EAGAIN(errcode) ((errcode) == EAGAIN || (errcode) == EWOULDBLOCK)
# else
#  define IS_EAGAIN(errcode) ((errcode) == EAGAIN)
# endif
#endif

_GL_INLINE_HEADER_END
