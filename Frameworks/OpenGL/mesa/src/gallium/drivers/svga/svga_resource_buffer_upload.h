/**********************************************************
 * Copyright 2008-2009 VMware, Inc.  All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 **********************************************************/

#ifndef SVGA_BUFFER_UPLOAD_H
#define SVGA_BUFFER_UPLOAD_H


void
svga_buffer_upload_flush(struct svga_context *svga,
                         struct svga_buffer *sbuf);

void
svga_buffer_add_range(struct svga_buffer *sbuf,
                      unsigned start,
                      unsigned end);

enum pipe_error
svga_buffer_create_hw_storage(struct svga_screen *ss,
                              struct svga_buffer *sbuf,
                              unsigned bind_flags);

void
svga_buffer_destroy_hw_storage(struct svga_screen *ss,
			       struct svga_buffer *sbuf);

enum pipe_error
svga_buffer_create_host_surface(struct svga_screen *ss,
                                struct svga_buffer *sbuf,
                                unsigned bind_flags);

enum pipe_error
svga_buffer_recreate_host_surface(struct svga_context *svga,
                                  struct svga_buffer *sbuf,
                                  unsigned bind_flags);

struct svga_buffer_surface *
svga_buffer_add_host_surface(struct svga_buffer *sbuf,
                             struct svga_winsys_surface *handle,
                             struct svga_host_surface_cache_key *key,
                             unsigned bind_flags);

void
svga_buffer_bind_host_surface(struct svga_context *svga,
                             struct svga_buffer *sbuf,
                             struct svga_buffer_surface *bufsurf);

enum pipe_error
svga_buffer_validate_host_surface(struct svga_context *svga,
                                  struct svga_buffer *sbuf,
                                  unsigned bind_flags);

void
svga_buffer_destroy_host_surface(struct svga_screen *ss,
                                 struct svga_buffer *sbuf);




#endif /* SVGA_BUFFER_H */
