/* -*- mesa-c++  -*-
 *
 * Copyright (c) 2018-2019 Collabora LTD
 *
 * Author: Gert Wollny <gert.wollny@collabora.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHOR(S) AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef SFN_STDERR_STREAMLOG_H
#define SFN_STDERR_STREAMLOG_H

#include "compiler/nir/nir.h"

#include <fstream>
#include <ostream>
#include <streambuf>

namespace r600 {
/* Implement some logging for shader-from-nir

*/

class stderr_streambuf : public std::streambuf {
public:
   stderr_streambuf();

protected:
   int sync();
   int overflow(int c);
   std::streamsize xsputn(const char *s, std::streamsize n);
};

class SfnLog {
public:
   enum LogFlag {
      instr = 1 << 0,
      r600ir = 1 << 1,
      cc = 1 << 2,
      err = 1 << 3,
      shader_info = 1 << 4,
      test_shader = 1 << 5,
      reg = 1 << 6,
      io = 1 << 7,
      assembly = 1 << 8,
      flow = 1 << 9,
      merge = 1 << 10,
      tex = 1 << 11,
      trans = 1 << 12,
      schedule = 1 << 13,
      opt = 1 << 14,
      all = (1 << 15) - 1,
      nomerge = 1 << 16,
      steps = 1 << 17,
      noopt = 1 << 18,
      warn = 1 << 20,
   };

   SfnLog();

   /** a special handling to set the output level "inline"
       \param l the level of the following messages
     */
   SfnLog& operator<<(LogFlag const l);

   /* general output routine; output is only given, if the log flags and the
    * currently active log mask overlap
      \returns a reference to this object
   */
   template <class T> SfnLog& operator<<(const T& text)
   {
      if (m_active_log_flags & m_log_mask)
         m_output << text;

      return *this;
   }

   /* A funny construct to enable std::endl to work on this stream
      idea of Dave Brondsema:
      http://gcc.gnu.org/bugzilla/show_bug.cgi?id=8567
    */
   SfnLog& operator<<(std::ostream& (*f)(std::ostream&));

   SfnLog& operator<<(nir_shader& sh);

   SfnLog& operator<<(nir_instr& instr);

   int has_debug_flag(uint64_t flag) { return (m_log_mask & flag) == flag; }

private:
   uint64_t m_active_log_flags;
   uint64_t m_log_mask;
   stderr_streambuf m_buf;
   std::ostream m_output;
};

class SfnTrace {
public:
   SfnTrace(SfnLog::LogFlag flag, const char *msg);
   ~SfnTrace();

private:
   SfnLog::LogFlag m_flag;
   const char *m_msg;
   static int m_indention;
};

#ifndef NDEBUG
#define SFN_TRACE_FUNC(LEVEL, MSG) SfnTrace __trace(LEVEL, MSG)
#else
#define SFN_TRACE_FUNC(LEVEL, MSG)
#endif

extern SfnLog sfn_log;

} // namespace r600
#endif // SFN_STDERR_STREAMBUF_H
