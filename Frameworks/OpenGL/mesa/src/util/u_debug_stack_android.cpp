/*
 * Copyright (C) 2018 Stefan Schake <stschake@gmail.com>
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
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include "u_debug_stack.h"

#if WITH_LIBBACKTRACE

#include <backtrace/Backtrace.h>

#include "util/simple_mtx.h"
#include "util/u_debug.h"
#include "util/hash_table.h"
#include "util/u_thread.h"

static hash_table *symbol_table;
static simple_mtx_t table_mutex = SIMPLE_MTX_INITIALIZER;

static const char *
intern_symbol(const char *symbol)
{
   if (!symbol_table)
      symbol_table = _mesa_hash_table_create(NULL, NULL, _mesa_key_string_equal);

   uint32_t hash = _mesa_hash_string(symbol);
   hash_entry *entry =
      _mesa_hash_table_search_pre_hashed(symbol_table, hash, symbol);
   if (!entry)
      entry = _mesa_hash_table_insert_pre_hashed(symbol_table, hash, symbol, strdup(symbol));

   return (const char *) entry->data;
}

void
debug_backtrace_capture(debug_stack_frame *backtrace,
                        unsigned start_frame,
                        unsigned nr_frames)
{
   Backtrace *bt;

   if (!nr_frames)
      return;

   bt = Backtrace::Create(BACKTRACE_CURRENT_PROCESS,
                          BACKTRACE_CURRENT_THREAD);
   if (bt == NULL) {
      for (unsigned i = 0; i < nr_frames; i++)
         backtrace[i].procname = NULL;
      return;
   }

   /* Add one to exclude this call. Unwind already ignores itself. */
   bt->Unwind(start_frame + 1);

   simple_mtx_lock(&table_mutex);

   for (unsigned i = 0; i < nr_frames; i++) {
      const backtrace_frame_data_t* frame = bt->GetFrame(i);
      if (frame) {
         backtrace[i].procname = intern_symbol(frame->func_name.c_str());
         backtrace[i].start_ip = frame->pc;
         backtrace[i].off = frame->func_offset;
         backtrace[i].map = intern_symbol(frame->map.Name().c_str());
         backtrace[i].map_off = frame->rel_pc;
      } else {
         backtrace[i].procname = NULL;
      }
   }

   simple_mtx_unlock(&table_mutex);

   delete bt;
}

void
debug_backtrace_dump(const debug_stack_frame *backtrace,
                     unsigned nr_frames)
{
   for (unsigned i = 0; i < nr_frames; i++) {
      if (backtrace[i].procname)
         debug_printf(
            "%s(+0x%x)\t%012" PRIx64 ": %s+0x%x\n",
            backtrace[i].map,
            backtrace[i].map_off,
            backtrace[i].start_ip,
            backtrace[i].procname,
            backtrace[i].off);
   }
}

void
debug_backtrace_print(FILE *f,
                      const debug_stack_frame *backtrace,
                      unsigned nr_frames)
{
   for (unsigned i = 0; i < nr_frames; i++) {
      if (backtrace[i].procname)
         fprintf(f,
                 "%s(+0x%x)\t%012" PRIx64 ": %s+0x%x\n",
                 backtrace[i].map,
                 backtrace[i].map_off,
                 backtrace[i].start_ip,
                 backtrace[i].procname,
                 backtrace[i].off);
   }
}

#else

void
debug_backtrace_capture(debug_stack_frame *backtrace,
                        unsigned start_frame,
                        unsigned nr_frames)
{
}

void
debug_backtrace_dump(const debug_stack_frame *backtrace,
                     unsigned nr_frames)
{
}

void
debug_backtrace_print(FILE *f,
                      const debug_stack_frame *backtrace,
                      unsigned nr_frames)
{
}

#endif // WITH_LIBBACKTRACE