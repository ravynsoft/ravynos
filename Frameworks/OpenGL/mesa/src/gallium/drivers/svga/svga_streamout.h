/**********************************************************
 * Copyright 2014 VMware, Inc.  All rights reserved.
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

#ifndef SVGA_STREAMOUT_H
#define SVGA_STREAMOUT_H

struct svga_shader;

struct svga_stream_output {
   struct pipe_stream_output_info info;
   unsigned pos_out_index;                  // position output index
   unsigned id;
   unsigned streammask;                     // bitmask to specify which streams are enabled
   unsigned buffer_stream;
   struct svga_winsys_buffer *declBuf;
};

struct svga_stream_output *
svga_create_stream_output(struct svga_context *svga,
                          struct svga_shader *shader,
                          const struct pipe_stream_output_info *info);

enum pipe_error
svga_set_stream_output(struct svga_context *svga,
                       struct svga_stream_output *streamout);

void
svga_delete_stream_output(struct svga_context *svga,
                          struct svga_stream_output *streamout);

enum pipe_error
svga_rebind_stream_output_targets(struct svga_context *svga);

void
svga_create_stream_output_queries(struct svga_context *svga);

void
svga_destroy_stream_output_queries(struct svga_context *svga);

void
svga_begin_stream_output_queries(struct svga_context *svga, unsigned mask);

void
svga_end_stream_output_queries(struct svga_context *svga, unsigned mask);

unsigned
svga_get_primcount_from_stream_output(struct svga_context *svga,
                                      unsigned stream);

#endif /* SVGA_STREAMOUT_H */
