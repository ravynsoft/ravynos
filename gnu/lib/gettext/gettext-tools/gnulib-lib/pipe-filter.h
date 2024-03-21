/* Filtering of data through a subprocess.  -*- coding: utf-8 -*-
   Copyright (C) 2009-2023 Free Software Foundation, Inc.
   Written by Bruno Haible <haible@clisp.cons.org>, 2009,
   and Paolo Bonzini <bonzini@gnu.org>, 2009.

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

#ifndef _PIPE_FILTER_H
#define _PIPE_FILTER_H

/* This file uses _GL_ATTRIBUTE_DEALLOC.  */
#if !_GL_CONFIG_H_INCLUDED
 #error "Please include config.h first."
#endif

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif


/* Piping data through a subprocess in the naïve way - write data to the
   subprocess and read from the subprocess when you expect it to have
   produced results - is subject to two kinds of deadlocks:
   1) If you write more than PIPE_MAX bytes or, more generally, if you write
      more bytes than the subprocess can handle at once, the subprocess
      may write its data and wait on you to read it, but you are currently
      busy writing.
   2) When you don't know ahead of time how many bytes the subprocess
      will produce, the usual technique of calling read (fd, buf, BUFSIZ)
      with a fixed BUFSIZ will, on Linux 2.2.17 and on BSD systems, cause
      the read() call to block until *all* of the buffer has been filled.
      But the subprocess cannot produce more data until you gave it more
      input.  But you are currently busy reading from it.

   This header file declares four set of functions that pipes data through
   the subprocess, without risking these deadlocks.

   The side that writes data to the subprocess can be seen as a "generator",
   that is, as a subroutine that produces and writes a piece of data here and
   there, see <https://en.wikipedia.org/wiki/Generator_(computer_science)>.
   But often, it can be written in the form of an "iterator", that is, as a
   function that, each time it is invoked, produces and writes one more piece
   of data.

   Similarly, the side that reads data from the subprocess can be seen as
   a "generator", that is, as a subroutine that consumes a piece of data here
   and there.  Often, it can be written in the form of an "iterator", that
   is, as a function that, each time it is invoked, consumes one more piece
   of data.

   This header file declares four set of functions:

                       |   writer   |   reader   |
       ----------------+------------+------------+
       pipe_filter_ii  |  iterator  |  iterator  |
       pipe_filter_ig  |  iterator  |  generator |
       pipe_filter_gi  |  generator |  iterator  |
       pipe_filter_gg  |  generator |  generator |
       ----------------+------------+------------+

   The last one uses threads in order to implement two generators running at
   the same time.  (For the relation between generators, coroutines, and
   threads, see <https://en.wikipedia.org/wiki/Generator_(computer_science)>
   and <https://en.wikipedia.org/wiki/Coroutine>.)  It is therefore only
   portable to platforms with kernel-based POSIX threads.  */

/* These two functions together describe the side that writes data to the
   subprocess when it has the form of an iterator.
   - prepare_write (&num_bytes, p) must either return a pointer to data that
     is ready to be written and set num_bytes to the number of bytes ready to
     be written, or return NULL when no more bytes are to be written.
   - done_write (data_written, num_bytes_written) is called after
     num_bytes_written bytes were written.  It is guaranteed that
     num_bytes_written > 0.
   Here p is always the private_data argument passed to the main function.  */
typedef const void * (*prepare_write_fn) (size_t *num_bytes_p,
                                          void *private_data);
typedef void (*done_write_fn) (void *data_written, size_t num_bytes_written,
                               void *private_data);

/* These two functions together describe the side that reads data from the
   subprocess when it has the form of an iterator.
   - prepare_read (&num_bytes, p) must return a pointer to a buffer for data
     that can be read and set num_bytes to the size of that buffer
     (must be > 0).
   - done_read (data_read, num_bytes_read, p) is called after num_bytes_read
     bytes were read into the buffer.
   Here p is always the private_data argument passed to the main function.  */
typedef void * (*prepare_read_fn) (size_t *num_bytes_p,
                                   void *private_data);
typedef void (*done_read_fn) (void *data_read, size_t num_bytes_read,
                              void *private_data);


/* ============================ pipe_filter_ii ============================ */

/* Create a subprocess and pipe some data through it.
   Arguments:
   - progname is the program name used in error messages.
   - prog_path is the file name of the program to invoke.
   - prog_argv is a NULL terminated argument list, starting with prog_path as
     first element.
   - If null_stderr is true, the subprocess' stderr will be redirected to
     /dev/null, and the usual error message to stderr will be omitted.
     This is suitable when the subprocess does not fulfill an important task.
   - If exit_on_error is true, any error will cause the main process to exit
     with an error status.
   If the subprocess does not terminate correctly, exit if exit_on_error is
   true, otherwise return 127.
   Callback arguments are as described above.

   Data is alternately written to the subprocess, through the functions
   prepare_write and done_write, and read from the subprocess, through the
   functions prepare_read and done_read.

   Note that the prepare_write/done_write functions and the
   prepare_read/done_read functions may be called in different threads than
   the current thread (depending on the platform).  But they will not be
   called after the pipe_filter_ii_execute function has returned.

   Return 0 upon success, or (only if exit_on_error is false):
   - -1 with errno set upon failure,
   - the positive exit code of the subprocess if that failed.  */
extern int
       pipe_filter_ii_execute (const char *progname,
                               const char *prog_path,
                               const char * const *prog_argv,
                               bool null_stderr, bool exit_on_error,
                               prepare_write_fn prepare_write,
                               done_write_fn done_write,
                               prepare_read_fn prepare_read,
                               done_read_fn done_read,
                               void *private_data);


/* ============================ pipe_filter_ig ============================ */

struct pipe_filter_ig;


/* ============================ pipe_filter_gi ============================ */

struct pipe_filter_gi;

/* Finish reading the output via the prepare_read/done_read functions
   specified to pipe_filter_gi_create.

   Note that the prepare_read/done_read functions may be called in a
   different thread than the current thread (depending on the platform).
   However, they will always be called before pipe_filter_gi_close has
   returned.

   The write side of the pipe is closed as soon as pipe_filter_gi_close
   starts, while the read side will be closed just before it finishes.

   Return 0 upon success, or (only if exit_on_error is false):
   - -1 with errno set upon failure,
   - the positive exit code of the subprocess if that failed.  */
extern int
       pipe_filter_gi_close (struct pipe_filter_gi *filter);

/* Create a subprocess and pipe some data through it.
   Arguments:
   - progname is the program name used in error messages.
   - prog_path is the file name of the program to invoke.
   - prog_argv is a NULL terminated argument list, starting with
     prog_path as first element.
   - If null_stderr is true, the subprocess' stderr will be redirected
     to /dev/null, and the usual error message to stderr will be
     omitted.  This is suitable when the subprocess does not fulfill an
     important task.
   - If exit_on_error is true, any error will cause the main process to
     exit with an error status.
   If the subprocess does not start correctly, exit if exit_on_error is
   true, otherwise return NULL and set errno.

   The caller will write to the subprocess through pipe_filter_gi_write
   and finally call pipe_filter_gi_close.  During such calls, the
   prepare_read and done_read function may be called to process any data
   that the subprocess has written.

   Note that the prepare_read/done_read functions may be called in a
   different thread than the current thread (depending on the platform).
   But they will not be called after the pipe_filter_gi_close function has
   returned.

   Return the freshly created 'struct pipe_filter_gi'.  */
extern struct pipe_filter_gi *
       pipe_filter_gi_create (const char *progname,
                              const char *prog_path,
                              const char * const *prog_argv,
                              bool null_stderr, bool exit_on_error,
                              prepare_read_fn prepare_read,
                              done_read_fn done_read,
                              void *private_data)
  _GL_ATTRIBUTE_DEALLOC (pipe_filter_gi_close, 1);

/* Write size bytes starting at buf into the pipe and in the meanwhile
   possibly call the prepare_read and done_read functions specified to
   pipe_filter_gi_create.

   Note that the prepare_read/done_read functions may be called in a
   different thread than the current thread (depending on the platform).
   However, they will always be called before pipe_filter_gi_write has
   returned, or otherwise not sooner than the next call to
   pipe_filter_gi_write or pipe_filter_gi_close.

   Return only after all the entire buffer has been written to the pipe or
   the subprocess has exited.

   Return 0 upon success, or (only if exit_on_error is false):
   - -1 with errno set upon failure,
   - the positive exit code of the subprocess if that failed.  */
extern int
       pipe_filter_gi_write (struct pipe_filter_gi *filter,
                             const void *buf, size_t size);


/* ============================ pipe_filter_gg ============================ */


/* ======================================================================== */


#ifdef __cplusplus
}
#endif


#endif /* _PIPE_FILTER_H */
