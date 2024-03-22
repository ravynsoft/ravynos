/**************************************************************************
 *
 * Copyright 2008 VMware, Inc.
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
 * IN NO EVENT SHALL VMWARE AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 **************************************************************************/


/**
 * @file
 * Trace dumping functions.
 *
 * For now we just use standard XML for dumping the trace calls, as this is
 * simple to write, parse, and visually inspect, but the actual representation
 * is abstracted out of this file, so that we can switch to a binary
 * representation if/when it becomes justified.
 *
 * @author Jose Fonseca <jfonseca@vmware.com>
 */

#include "util/detect.h"

#include <stdio.h>
#include <stdlib.h>

/* for access() */
#ifdef _WIN32
# include <io.h>
#endif

#include "util/compiler.h"
#include "util/u_thread.h"
#include "util/os_time.h"
#include "util/simple_mtx.h"
#include "util/u_debug.h"
#include "util/u_memory.h"
#include "util/u_string.h"
#include "util/u_math.h"
#include "util/format/u_format.h"
#include "compiler/nir/nir.h"

#include "tr_dump.h"
#include "tr_screen.h"
#include "tr_texture.h"


static bool close_stream = false;
static FILE *stream = NULL;
static simple_mtx_t call_mutex = SIMPLE_MTX_INITIALIZER;
static long unsigned call_no = 0;
static bool dumping = false;
static long nir_count = 0;

static bool trigger_active = true;
static char *trigger_filename = NULL;

void
trace_dump_trigger_active(bool active)
{
   trigger_active = active;
}

void
trace_dump_check_trigger(void)
{
   if (!trigger_filename)
      return;

   simple_mtx_lock(&call_mutex);
   if (trigger_active) {
      trigger_active = false;
   } else {
      if (!access(trigger_filename, 2 /* W_OK but compiles on Windows */)) {
         if (!unlink(trigger_filename)) {
            trigger_active = true;
         } else {
            fprintf(stderr, "error removing trigger file\n");
            trigger_active = false;
         }
      }
   }
   simple_mtx_unlock(&call_mutex);
}

bool
trace_dump_is_triggered(void)
{
   return trigger_active && !!trigger_filename;
}

static inline void
trace_dump_write(const char *buf, size_t size)
{
   if (stream && trigger_active) {
      fwrite(buf, size, 1, stream);
   }
}


static inline void
trace_dump_writes(const char *s)
{
   trace_dump_write(s, strlen(s));
}


static inline void
trace_dump_writef(const char *format, ...)
{
   static char buf[1024];
   unsigned len;
   va_list ap;
   va_start(ap, format);
   len = vsnprintf(buf, sizeof(buf), format, ap);
   va_end(ap);
   trace_dump_write(buf, len);
}


static inline void
trace_dump_escape(const char *str)
{
   const unsigned char *p = (const unsigned char *)str;
   unsigned char c;
   while((c = *p++) != 0) {
      if(c == '<')
         trace_dump_writes("&lt;");
      else if(c == '>')
         trace_dump_writes("&gt;");
      else if(c == '&')
         trace_dump_writes("&amp;");
      else if(c == '\'')
         trace_dump_writes("&apos;");
      else if(c == '\"')
         trace_dump_writes("&quot;");
      else if(c >= 0x20 && c <= 0x7e)
         trace_dump_writef("%c", c);
      else
         trace_dump_writef("&#%u;", c);
   }
}


static inline void
trace_dump_indent(unsigned level)
{
   unsigned i;
   for(i = 0; i < level; ++i)
      trace_dump_writes("\t");
}


static inline void
trace_dump_newline(void)
{
   trace_dump_writes("\n");
}


static inline void
trace_dump_tag_begin(const char *name)
{
   trace_dump_writes("<");
   trace_dump_writes(name);
   trace_dump_writes(">");
}

static inline void
trace_dump_tag_begin1(const char *name,
                      const char *attr1, const char *value1)
{
   trace_dump_writes("<");
   trace_dump_writes(name);
   trace_dump_writes(" ");
   trace_dump_writes(attr1);
   trace_dump_writes("='");
   trace_dump_escape(value1);
   trace_dump_writes("'>");
}


static inline void
trace_dump_tag_end(const char *name)
{
   trace_dump_writes("</");
   trace_dump_writes(name);
   trace_dump_writes(">");
}

void
trace_dump_trace_flush(void)
{
   if (stream) {
      fflush(stream);
   }
}

static void
trace_dump_trace_close(void)
{
   if (stream) {
      trigger_active = true;
      trace_dump_writes("</trace>\n");
      if (close_stream) {
         fclose(stream);
         close_stream = false;
         stream = NULL;
      }
      call_no = 0;
      free(trigger_filename);
   }
}


static void
trace_dump_call_time(int64_t time)
{
   if (stream) {
      trace_dump_indent(2);
      trace_dump_tag_begin("time");
      trace_dump_int(time);
      trace_dump_tag_end("time");
      trace_dump_newline();
   }
}


bool
trace_dump_trace_begin(void)
{
   const char *filename;

   filename = debug_get_option("GALLIUM_TRACE", NULL);
   if (!filename)
      return false;

   nir_count = debug_get_num_option("GALLIUM_TRACE_NIR", 32);

   if (!stream) {

      if (strcmp(filename, "stderr") == 0) {
         close_stream = false;
         stream = stderr;
      }
      else if (strcmp(filename, "stdout") == 0) {
         close_stream = false;
         stream = stdout;
      }
      else {
         close_stream = true;
         stream = fopen(filename, "wt");
         if (!stream)
            return false;
      }

      trace_dump_writes("<?xml version='1.0' encoding='UTF-8'?>\n");
      trace_dump_writes("<?xml-stylesheet type='text/xsl' href='trace.xsl'?>\n");
      trace_dump_writes("<trace version='0.1'>\n");

      /* Many applications don't exit cleanly, others may create and destroy a
       * screen multiple times, so we only write </trace> tag and close at exit
       * time.
       */
      atexit(trace_dump_trace_close);

      const char *trigger = debug_get_option("GALLIUM_TRACE_TRIGGER", NULL);
      if (trigger && __normal_user()) {
         trigger_filename = strdup(trigger);
         trigger_active = false;
      } else
         trigger_active = true;
   }

   return true;
}

bool trace_dump_trace_enabled(void)
{
   return stream ? true : false;
}

/*
 * Call lock
 */

void trace_dump_call_lock(void)
{
   simple_mtx_lock(&call_mutex);
}

void trace_dump_call_unlock(void)
{
   simple_mtx_unlock(&call_mutex);
}

/*
 * Dumping control
 */

void trace_dumping_start_locked(void)
{
   dumping = true;
}

void trace_dumping_stop_locked(void)
{
   dumping = false;
}

bool trace_dumping_enabled_locked(void)
{
   return dumping;
}

void trace_dumping_start(void)
{
   simple_mtx_lock(&call_mutex);
   trace_dumping_start_locked();
   simple_mtx_unlock(&call_mutex);
}

void trace_dumping_stop(void)
{
   simple_mtx_lock(&call_mutex);
   trace_dumping_stop_locked();
   simple_mtx_unlock(&call_mutex);
}

bool trace_dumping_enabled(void)
{
   bool ret;
   simple_mtx_lock(&call_mutex);
   ret = trace_dumping_enabled_locked();
   simple_mtx_unlock(&call_mutex);
   return ret;
}

/*
 * Dump functions
 */

static int64_t call_start_time = 0;

void trace_dump_call_begin_locked(const char *klass, const char *method)
{
   if (!dumping)
      return;

   ++call_no;
   trace_dump_indent(1);
   trace_dump_writes("<call no=\'");
   trace_dump_writef("%lu", call_no);
   trace_dump_writes("\' class=\'");
   trace_dump_escape(klass);
   trace_dump_writes("\' method=\'");
   trace_dump_escape(method);
   trace_dump_writes("\'>");
   trace_dump_newline();

   call_start_time = os_time_get();
}

void trace_dump_call_end_locked(void)
{
   int64_t call_end_time;

   if (!dumping)
      return;

   call_end_time = os_time_get();

   trace_dump_call_time(call_end_time - call_start_time);
   trace_dump_indent(1);
   trace_dump_tag_end("call");
   trace_dump_newline();
   fflush(stream);
}

void trace_dump_call_begin(const char *klass, const char *method)
{
   simple_mtx_lock(&call_mutex);
   trace_dump_call_begin_locked(klass, method);
}

void trace_dump_call_end(void)
{
   trace_dump_call_end_locked();
   simple_mtx_unlock(&call_mutex);
}

void trace_dump_arg_begin(const char *name)
{
   if (!dumping)
      return;

   trace_dump_indent(2);
   trace_dump_tag_begin1("arg", "name", name);
}

void trace_dump_arg_end(void)
{
   if (!dumping)
      return;

   trace_dump_tag_end("arg");
   trace_dump_newline();
}

void trace_dump_ret_begin(void)
{
   if (!dumping)
      return;

   trace_dump_indent(2);
   trace_dump_tag_begin("ret");
}

void trace_dump_ret_end(void)
{
   if (!dumping)
      return;

   trace_dump_tag_end("ret");
   trace_dump_newline();
}

void trace_dump_bool(bool value)
{
   if (!dumping)
      return;

   trace_dump_writef("<bool>%c</bool>", value ? '1' : '0');
}

void trace_dump_int(int64_t value)
{
   if (!dumping)
      return;

   trace_dump_writef("<int>%" PRIi64 "</int>", value);
}

void trace_dump_uint(uint64_t value)
{
   if (!dumping)
      return;

   trace_dump_writef("<uint>%" PRIu64 "</uint>", value);
}

void trace_dump_float(double value)
{
   if (!dumping)
      return;

   trace_dump_writef("<float>%g</float>", value);
}

void trace_dump_bytes(const void *data,
                      size_t size)
{
   static const char hex_table[16] = "0123456789ABCDEF";
   const uint8_t *p = data;
   size_t i;

   if (!dumping)
      return;

   trace_dump_writes("<bytes>");
   for(i = 0; i < size; ++i) {
      uint8_t byte = *p++;
      char hex[2];
      hex[0] = hex_table[byte >> 4];
      hex[1] = hex_table[byte & 0xf];
      trace_dump_write(hex, 2);
   }
   trace_dump_writes("</bytes>");
}

void trace_dump_box_bytes(const void *data,
                          struct pipe_resource *resource,
			  const struct pipe_box *box,
			  unsigned stride,
			  uint64_t slice_stride)
{
   enum pipe_format format = resource->format;
   uint64_t size;

   assert(box->height > 0);
   assert(box->depth > 0);

   size = util_format_get_nblocksx(format, box->width ) *
          (uint64_t)util_format_get_blocksize(format) +
          (util_format_get_nblocksy(format, box->height) - 1) *
          (uint64_t)stride + (box->depth - 1) * slice_stride;

   /*
    * Only dump buffer transfers to avoid huge files.
    * TODO: Make this run-time configurable
    */
   if (resource->target != PIPE_BUFFER) {
      size = 0;
   }

   assert(size <= SIZE_MAX);
   trace_dump_bytes(data, size);
}

void trace_dump_string(const char *str)
{
   if (!dumping)
      return;

   trace_dump_writes("<string>");
   trace_dump_escape(str);
   trace_dump_writes("</string>");
}

void trace_dump_enum(const char *value)
{
   if (!dumping)
      return;

   trace_dump_writes("<enum>");
   trace_dump_escape(value);
   trace_dump_writes("</enum>");
}

void trace_dump_array_begin(void)
{
   if (!dumping)
      return;

   trace_dump_writes("<array>");
}

void trace_dump_array_end(void)
{
   if (!dumping)
      return;

   trace_dump_writes("</array>");
}

void trace_dump_elem_begin(void)
{
   if (!dumping)
      return;

   trace_dump_writes("<elem>");
}

void trace_dump_elem_end(void)
{
   if (!dumping)
      return;

   trace_dump_writes("</elem>");
}

void trace_dump_struct_begin(const char *name)
{
   if (!dumping)
      return;

   trace_dump_writef("<struct name='%s'>", name);
}

void trace_dump_struct_end(void)
{
   if (!dumping)
      return;

   trace_dump_writes("</struct>");
}

void trace_dump_member_begin(const char *name)
{
   if (!dumping)
      return;

   trace_dump_writef("<member name='%s'>", name);
}

void trace_dump_member_end(void)
{
   if (!dumping)
      return;

   trace_dump_writes("</member>");
}

void trace_dump_null(void)
{
   if (!dumping)
      return;

   trace_dump_writes("<null/>");
}

void trace_dump_ptr(const void *value)
{
   if (!dumping)
      return;

   if(value)
      trace_dump_writef("<ptr>0x%08lx</ptr>", (unsigned long)(uintptr_t)value);
   else
      trace_dump_null();
}

void trace_dump_surface_ptr(struct pipe_surface *_surface)
{
   if (!dumping)
      return;

   if (_surface) {
      struct trace_surface *tr_surf = trace_surface(_surface);
      trace_dump_ptr(tr_surf->surface);
   } else {
      trace_dump_null();
   }
}

void trace_dump_transfer_ptr(struct pipe_transfer *_transfer)
{
   if (!dumping)
      return;

   if (_transfer) {
      struct trace_transfer *tr_tran = trace_transfer(_transfer);
      trace_dump_ptr(tr_tran->transfer);
   } else {
      trace_dump_null();
   }
}

void trace_dump_nir(void *nir)
{
   if (!dumping)
      return;

   if (--nir_count < 0) {
      fputs("<string>...</string>", stream);
      return;
   }

   // NIR doesn't have a print to string function.  Use CDATA and hope for the
   // best.
   if (stream) {
      fputs("<string><![CDATA[", stream);
      nir_print_shader(nir, stream);
      fputs("]]></string>", stream);
   }
}
