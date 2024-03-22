/*
 * Copyright © 2017 Google, Inc.
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

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "c11/threads.h"
#include "util/detect_os.h"
#include "util/log.h"
#include "util/ralloc.h"
#include "util/u_debug.h"

#if DETECT_OS_UNIX
#include <syslog.h>
#include "util/u_process.h"
#endif

#if DETECT_OS_ANDROID
#include <android/log.h>
#endif

#if DETECT_OS_WINDOWS
#include <windows.h>
#endif

enum mesa_log_control {
   MESA_LOG_CONTROL_NULL = 1 << 0,
   MESA_LOG_CONTROL_FILE = 1 << 1,
   MESA_LOG_CONTROL_SYSLOG = 1 << 2,
   MESA_LOG_CONTROL_ANDROID = 1 << 3,
   MESA_LOG_CONTROL_WINDBG = 1 << 4,
   MESA_LOG_CONTROL_LOGGER_MASK = 0xff,

   MESA_LOG_CONTROL_WAIT = 1 << 8,
};

static const struct debug_control mesa_log_control_options[] = {
   /* loggers */
   { "null", MESA_LOG_CONTROL_NULL },
   { "file", MESA_LOG_CONTROL_FILE },
   { "syslog", MESA_LOG_CONTROL_SYSLOG },
   { "android", MESA_LOG_CONTROL_ANDROID },
   { "windbg", MESA_LOG_CONTROL_WINDBG },
   /* flags */
   { "wait", MESA_LOG_CONTROL_WAIT },
   { NULL, 0 },
};

static uint32_t mesa_log_control;
static FILE *mesa_log_file;

static void
mesa_log_init_once(void)
{
   mesa_log_control = parse_debug_string(os_get_option("MESA_LOG"),
         mesa_log_control_options);

   if (!(mesa_log_control & MESA_LOG_CONTROL_LOGGER_MASK)) {
      /* pick the default loggers */
#if DETECT_OS_ANDROID
      mesa_log_control |= MESA_LOG_CONTROL_ANDROID;
#else
      mesa_log_control |= MESA_LOG_CONTROL_FILE;
#endif

#if DETECT_OS_WINDOWS
      /* stderr from windows applications without console is not usually
       * visible, so communicate with the debugger instead */
      mesa_log_control |= MESA_LOG_CONTROL_WINDBG;
#endif
   }

   mesa_log_file = stderr;

#if !DETECT_OS_WINDOWS
   if (__normal_user()) {
      const char *log_file = os_get_option("MESA_LOG_FILE");
      if (log_file) {
         FILE *fp = fopen(log_file, "w");
         if (fp) {
            mesa_log_file = fp;
            mesa_log_control |= MESA_LOG_CONTROL_FILE;
         }
      }
   }
#endif

#if DETECT_OS_UNIX
   if (mesa_log_control & MESA_LOG_CONTROL_SYSLOG)
      openlog(util_get_process_name(), LOG_NDELAY | LOG_PID, LOG_USER);
#endif
}

static void
mesa_log_init(void)
{
   static once_flag once = ONCE_FLAG_INIT;
   call_once(&once, mesa_log_init_once);
}

static inline const char *
level_to_str(enum mesa_log_level l)
{
   switch (l) {
   case MESA_LOG_ERROR: return "error";
   case MESA_LOG_WARN: return "warning";
   case MESA_LOG_INFO: return "info";
   case MESA_LOG_DEBUG: return "debug";
   }

   unreachable("bad mesa_log_level");
}

enum logger_vasnprintf_affix {
   LOGGER_VASNPRINTF_AFFIX_TAG = 1 << 0,
   LOGGER_VASNPRINTF_AFFIX_LEVEL = 1 << 1,
   LOGGER_VASNPRINTF_AFFIX_NEWLINE = 1 << 2,
};

/* Try vsnprintf first and fall back to vasprintf if buf is too small.  This
 * function handles all errors and never fails.
 */
static char *
logger_vasnprintf(char *buf,
                  int size,
                  int affixes,
                  enum mesa_log_level level,
                  const char *tag,
                  const char *format,
                  va_list in_va)
{
   struct {
      char *cur;
      int rem;
      int total;
      bool invalid;
   } state = {
      .cur = buf,
      .rem = size,
   };

   va_list va;
   va_copy(va, in_va);

#define APPEND(state, func, ...)                                     \
   do {                                                              \
      int ret = func(state.cur, state.rem, __VA_ARGS__);             \
      if (ret < 0) {                                                 \
         state.invalid = true;                                       \
      }  else {                                                      \
         state.total += ret;                                         \
         if (ret >= state.rem)                                       \
            ret = state.rem;                                         \
         state.cur += ret;                                           \
         state.rem -= ret;                                           \
      }                                                              \
   } while (false)

   if (affixes & LOGGER_VASNPRINTF_AFFIX_TAG)
      APPEND(state, snprintf, "%s: ", tag);
   if (affixes & LOGGER_VASNPRINTF_AFFIX_LEVEL)
      APPEND(state, snprintf, "%s: ", level_to_str(level));

   APPEND(state, vsnprintf, format, va);

   if (affixes & LOGGER_VASNPRINTF_AFFIX_NEWLINE) {
      if (state.cur == buf || state.cur[-1] != '\n')
         APPEND(state, snprintf, "\n");
   }
#undef APPEND

   assert(size >= 64);
   if (state.invalid) {
      strncpy(buf, "invalid message format", size);
   } else if (state.total >= size) {
      /* print again into alloc to avoid truncation */
      void *alloc = malloc(state.total + 1);
      if (alloc) {
         buf = logger_vasnprintf(alloc, state.total + 1, affixes, level, tag,
               format, in_va);
         assert(buf == alloc);
      } else {
         /* pretty-truncate the message */
         strncpy(buf + size - 4, "...", 4);
      }
   }

   va_end(va);

   return buf;
}

static void
logger_file(enum mesa_log_level level,
            const char *tag,
            const char *format,
            va_list va)
{
   FILE *fp = mesa_log_file;
   char local_msg[1024];
   char *msg = logger_vasnprintf(local_msg, sizeof(local_msg),
         LOGGER_VASNPRINTF_AFFIX_TAG |
         LOGGER_VASNPRINTF_AFFIX_LEVEL |
         LOGGER_VASNPRINTF_AFFIX_NEWLINE,
         level, tag, format, va);

   fprintf(fp, "%s", msg);
   fflush(fp);

   if (msg != local_msg)
      free(msg);
}

#if DETECT_OS_UNIX

static inline int
level_to_syslog(enum mesa_log_level l)
{
   switch (l) {
   case MESA_LOG_ERROR: return LOG_ERR;
   case MESA_LOG_WARN: return LOG_WARNING;
   case MESA_LOG_INFO: return LOG_INFO;
   case MESA_LOG_DEBUG: return LOG_DEBUG;
   }

   unreachable("bad mesa_log_level");
}

static void
logger_syslog(enum mesa_log_level level,
              const char *tag,
              const char *format,
              va_list va)
{
   char local_msg[1024];
   char *msg = logger_vasnprintf(local_msg, sizeof(local_msg),
         LOGGER_VASNPRINTF_AFFIX_TAG, level, tag, format, va);

   syslog(level_to_syslog(level), "%s", msg);

   if (msg != local_msg)
      free(msg);
}

#endif /* DETECT_OS_UNIX */

#if DETECT_OS_ANDROID

static inline android_LogPriority
level_to_android(enum mesa_log_level l)
{
   switch (l) {
   case MESA_LOG_ERROR: return ANDROID_LOG_ERROR;
   case MESA_LOG_WARN: return ANDROID_LOG_WARN;
   case MESA_LOG_INFO: return ANDROID_LOG_INFO;
   case MESA_LOG_DEBUG: return ANDROID_LOG_DEBUG;
   }

   unreachable("bad mesa_log_level");
}

static void
logger_android(enum mesa_log_level level,
               const char *tag,
               const char *format,
               va_list va)
{
   /* Android can truncate/drop messages
    *
    *  - the internal buffer for vsnprintf has a fixed size (usually 1024)
    *  - the socket to logd is non-blocking
    *
    * and provides no way to detect.  Try our best.
    */
   char local_msg[1024];
   char *msg = logger_vasnprintf(local_msg, sizeof(local_msg), 0, level, tag,
         format, va);

   __android_log_write(level_to_android(level), tag, msg);

   if (msg != local_msg)
      free(msg);

   /* increase the chance of logd doing its part */
   if (mesa_log_control & MESA_LOG_CONTROL_WAIT)
      thrd_yield();
}

#endif /* DETECT_OS_ANDROID */

#if DETECT_OS_WINDOWS

static void
logger_windbg(enum mesa_log_level level,
              const char *tag,
              const char *format,
              va_list va)
{
   char local_msg[1024];
   char *msg = logger_vasnprintf(local_msg, sizeof(local_msg),
         LOGGER_VASNPRINTF_AFFIX_TAG |
         LOGGER_VASNPRINTF_AFFIX_LEVEL |
         LOGGER_VASNPRINTF_AFFIX_NEWLINE,
         level, tag, format, va);

   OutputDebugStringA(msg);

   if (msg != local_msg)
      free(msg);
}

#endif /* DETECT_OS_WINDOWS */

/* This is for use with debug functions that take a FILE, such as
 * nir_print_shader, although switching to nir_log_shader* is preferred.
 */
FILE *
mesa_log_get_file(void)
{
   mesa_log_init();
   return mesa_log_file;
}

void
mesa_log(enum mesa_log_level level, const char *tag, const char *format, ...)
{
   va_list va;

   va_start(va, format);
   mesa_log_v(level, tag, format, va);
   va_end(va);
}

void
mesa_log_v(enum mesa_log_level level, const char *tag, const char *format,
            va_list va)
{
   static const struct {
      enum mesa_log_control bit;
      void (*log)(enum mesa_log_level level,
                  const char *tag,
                  const char *format,
                  va_list va);
   } loggers[] = {
      { MESA_LOG_CONTROL_FILE, logger_file },
#if DETECT_OS_UNIX
      { MESA_LOG_CONTROL_SYSLOG, logger_syslog },
#endif
#if DETECT_OS_ANDROID
      { MESA_LOG_CONTROL_ANDROID, logger_android },
#endif
#if DETECT_OS_WINDOWS
      { MESA_LOG_CONTROL_WINDBG, logger_windbg },
#endif
   };

   mesa_log_init();

   for (uint32_t i = 0; i < ARRAY_SIZE(loggers); i++) {
      if (mesa_log_control & loggers[i].bit) {
         va_list copy;
         va_copy(copy, va);
         loggers[i].log(level, tag, format, copy);
         va_end(copy);
      }
   }
}

struct log_stream *
_mesa_log_stream_create(enum mesa_log_level level, const char *tag)
{
   struct log_stream *stream = ralloc(NULL, struct log_stream);
   stream->level = level;
   stream->tag = tag;
   stream->msg = ralloc_strdup(stream, "");
   stream->pos = 0;
   return stream;
}

void
mesa_log_stream_destroy(struct log_stream *stream)
{
   /* If you left trailing stuff in the log stream, flush it out as a line. */
   if (stream->pos != 0)
      mesa_log(stream->level, stream->tag, "%s", stream->msg);

   ralloc_free(stream);
}

static void
mesa_log_stream_flush(struct log_stream *stream, size_t scan_offset)
{
   char *end;
   char *next = stream->msg;
   while ((end = strchr(stream->msg + scan_offset, '\n'))) {
      *end = 0;
      mesa_log(stream->level, stream->tag, "%s", next);
      next = end + 1;
      scan_offset = next - stream->msg;
   }
   if (next != stream->msg) {
      /* Clear out the lines we printed and move any trailing chars to the start. */
      size_t remaining = stream->msg + stream->pos - next;
      memmove(stream->msg, next, remaining);
      stream->pos = remaining;
   }
}

void mesa_log_stream_printf(struct log_stream *stream, const char *format, ...)
{
   size_t old_pos = stream->pos;

   va_list va;
   va_start(va, format);
   ralloc_vasprintf_rewrite_tail(&stream->msg, &stream->pos, format, va);
   va_end(va);

   mesa_log_stream_flush(stream, old_pos);
}

void
_mesa_log_multiline(enum mesa_log_level level, const char *tag, const char *lines)
{
   struct log_stream tmp = {
      .level = level,
      .tag = tag,
      .msg = strdup(lines),
      .pos = strlen(lines),
   };
   mesa_log_stream_flush(&tmp, 0);
   free(tmp.msg);
}
