/**************************************************************************
 *
 * Copyright 2010 Luca Barbieri
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE COPYRIGHT OWNER(S) AND/OR ITS SUPPLIERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 **************************************************************************/

#if defined(DEBUG)

/**
 * If the GALLIUM_REFCNT_LOG env var is defined as a filename, gallium
 * reference counting will be logged to the file.
 *
 * See http://www-archive.mozilla.org/performance/refcnt-balancer.html
 * for what to do with the output on Linux, use tools/addr2line.sh to
 * postprocess it before anything else.
 */

#include <stdio.h>

#include "util/simple_mtx.h"
#include "util/u_debug.h"
#include "util/u_debug_refcnt.h"
#include "util/u_debug_stack.h"
#include "util/u_debug_symbol.h"
#include "util/u_string.h"
#include "util/u_hash_table.h"
#include "util/u_thread.h"

int debug_refcnt_state;

static FILE *stream;

/* TODO: maybe move this serial machinery to a stand-alone module and
 * expose it?
 */

static simple_mtx_t serials_mutex = SIMPLE_MTX_INITIALIZER;
static struct hash_table *serials_hash;
static unsigned serials_last;


/**
 * Return a small integer serial number for the given pointer.
 */
static bool
debug_serial(void *p, unsigned *pserial)
{
   unsigned serial;
   bool found = true;

   simple_mtx_lock(&serials_mutex);
   if (!serials_hash)
      serials_hash = util_hash_table_create_ptr_keys();

   serial = (unsigned) (uintptr_t) util_hash_table_get(serials_hash, p);
   if (!serial) {
      /* time to stop logging... (you'll have a 100 GB logfile at least at
       * this point)  TODO: avoid this
       */
      serial = ++serials_last;
      if (!serial) {
         debug_error("More than 2^32 objects detected, aborting.\n");
         os_abort();
      }

      _mesa_hash_table_insert(serials_hash, p, (void *) (uintptr_t) serial);
      found = false;
   }
   simple_mtx_unlock(&serials_mutex);

   *pserial = serial;

   return found;
}


/**
 * Free the serial number for the given pointer.
 */
static void
debug_serial_delete(void *p)
{
   simple_mtx_lock(&serials_mutex);
   _mesa_hash_table_remove_key(serials_hash, p);
   simple_mtx_unlock(&serials_mutex);
}


#if DETECT_OS_WINDOWS
#define STACK_LEN 60
#else
#define STACK_LEN 64
#endif

/**
 * Log a reference count change to the log file (if enabled).
 * This is called via the pipe_reference() and debug_reference() functions,
 * basically whenever a reference count is initialized or changed.
 *
 * \param p  the refcount being changed (the value is not changed here)
 * \param get_desc  a function which will be called to print an object's
 *                  name/pointer into a string buffer during logging
 * \param change  the reference count change which must be +/-1 or 0 when
 *                creating the object and initializing the refcount.
 */
void
debug_reference_slowpath(const struct pipe_reference *p,
                         debug_reference_descriptor get_desc, int change)
{
   assert(change >= -1);
   assert(change <= 1);

   if (debug_refcnt_state < 0)
      return;

   if (!debug_refcnt_state) {
      const char *filename = debug_get_option("GALLIUM_REFCNT_LOG", NULL);
      if (filename && filename[0])
         stream = fopen(filename, "wt");

      if (stream)
         debug_refcnt_state = 1;
      else
         debug_refcnt_state = -1;
   }

   if (debug_refcnt_state > 0) {
      struct debug_stack_frame frames[STACK_LEN];
      char buf[1024];
      unsigned i;
      unsigned refcnt = p->count;
      unsigned serial;
      bool existing = debug_serial((void *) p, &serial);

      debug_backtrace_capture(frames, 1, STACK_LEN);

      get_desc(buf, p);

      if (!existing) {
         fprintf(stream, "<%s> %p %u Create\n", buf, (void *) p, serial);
         debug_backtrace_print(stream, frames, STACK_LEN);

         /* this is here to provide a gradual change even if we don't see
          * the initialization
          */
         if (refcnt <= 10) {
            for (i = 1; i <= refcnt - change; ++i) {
               fprintf(stream, "<%s> %p %u AddRef %u\n", buf, (void *) p,
                  serial, i);
               debug_backtrace_print(stream, frames, STACK_LEN);
            }
         }
      }

      if (change) {
         fprintf(stream, "<%s> %p %u %s %u\n", buf, (void *) p, serial,
                 change > 0 ? "AddRef" : "Release", refcnt);
         debug_backtrace_print(stream, frames, STACK_LEN);
      }

      if (!refcnt) {
         debug_serial_delete((void *) p);
         fprintf(stream, "<%s> %p %u Destroy\n", buf, (void *) p, serial);
         debug_backtrace_print(stream, frames, STACK_LEN);
      }

      fflush(stream);
   }
}

#endif /* DEBUG */
