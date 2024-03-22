/* -*- mode: C; c-file-style: "k&r"; tab-width 4; indent-tabs-mode: t; -*- */

/*
 * Copyright (C) 2014 Rob Clark <robclark@freedesktop.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Authors:
 *    Rob Clark <robclark@freedesktop.org>
 */

#include <archive.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <archive_entry.h>

#include "io.h"

struct io {
   struct archive *a;
   struct archive_entry *entry;
   unsigned offset;
};

static void
io_error(struct io *io)
{
   fprintf(stderr, "%s\n", archive_error_string(io->a));
   io_close(io);
}

static struct io *
io_new(void)
{
   struct io *io = calloc(1, sizeof(*io));
   int ret;

   if (!io)
      return NULL;

   io->a = archive_read_new();
   ret = archive_read_support_filter_gzip(io->a);
   if (ret != ARCHIVE_OK) {
      io_error(io);
      return NULL;
   }

   ret = archive_read_support_filter_none(io->a);
   if (ret != ARCHIVE_OK) {
      io_error(io);
      return NULL;
   }

   ret = archive_read_support_format_all(io->a);
   if (ret != ARCHIVE_OK) {
      io_error(io);
      return NULL;
   }

   ret = archive_read_support_format_raw(io->a);
   if (ret != ARCHIVE_OK) {
      io_error(io);
      return NULL;
   }

   return io;
}

struct io *
io_open(const char *filename)
{
   struct io *io = io_new();
   int ret;

   if (!io)
      return NULL;

   ret = archive_read_open_filename(io->a, filename, 10240);
   if (ret != ARCHIVE_OK) {
      io_error(io);
      return NULL;
   }

   ret = archive_read_next_header(io->a, &io->entry);
   if (ret != ARCHIVE_OK) {
      io_error(io);
      return NULL;
   }

   return io;
}

struct io *
io_openfd(int fd)
{
   struct io *io = io_new();
   int ret;

   if (!io)
      return NULL;

   ret = archive_read_open_fd(io->a, fd, 10240);
   if (ret != ARCHIVE_OK) {
      io_error(io);
      return NULL;
   }

   ret = archive_read_next_header(io->a, &io->entry);
   if (ret != ARCHIVE_OK) {
      io_error(io);
      return NULL;
   }

   return io;
}

void
io_close(struct io *io)
{
   archive_read_free(io->a);
   free(io);
}

unsigned
io_offset(struct io *io)
{
   return io->offset;
}

#include <assert.h>
int
io_readn(struct io *io, void *buf, int nbytes)
{
   char *ptr = buf;
   int ret = 0;
   while (nbytes > 0) {
      int n = archive_read_data(io->a, ptr, nbytes);
      if (n < 0) {
         fprintf(stderr, "%s\n", archive_error_string(io->a));
         return n;
      }
      if (n == 0)
         break;
      ptr += n;
      nbytes -= n;
      ret += n;
      io->offset += n;
   }
   return ret;
}
