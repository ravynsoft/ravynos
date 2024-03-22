/*
 * Copyright © 2021 Google, Inc.
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
 */

#include "u_perfetto.h"

#include <perfetto.h>

#include "c11/threads.h"
#include "util/macros.h"

/* perfetto requires string literals */
#define UTIL_PERFETTO_CATEGORY_DEFAULT_STR "mesa.default"

PERFETTO_DEFINE_CATEGORIES(
   perfetto::Category(UTIL_PERFETTO_CATEGORY_DEFAULT_STR)
      .SetDescription("Mesa default events"));

PERFETTO_TRACK_EVENT_STATIC_STORAGE();

int util_perfetto_tracing_state;

static void
util_perfetto_update_tracing_state(void)
{
   p_atomic_set(&util_perfetto_tracing_state,
                TRACE_EVENT_CATEGORY_ENABLED(UTIL_PERFETTO_CATEGORY_DEFAULT_STR));
}

void
util_perfetto_trace_begin(const char *name)
{
   TRACE_EVENT_BEGIN(
      UTIL_PERFETTO_CATEGORY_DEFAULT_STR, nullptr,
      [&](perfetto::EventContext ctx) { ctx.event()->set_name(name); });
}

void
util_perfetto_trace_end(void)
{
   TRACE_EVENT_END(UTIL_PERFETTO_CATEGORY_DEFAULT_STR);

   util_perfetto_update_tracing_state();
}

class UtilPerfettoObserver : public perfetto::TrackEventSessionObserver {
 public:
   UtilPerfettoObserver() { perfetto::TrackEvent::AddSessionObserver(this); }

   void OnStart(const perfetto::DataSourceBase::StartArgs &) override
   {
      util_perfetto_update_tracing_state();
   }

   /* XXX There is no PostStop callback.  We have to call
    * util_perfetto_update_tracing_state occasionally to poll.
    */
};

static void
util_perfetto_fini(void)
{
   perfetto::Tracing::Shutdown();
}

static void
util_perfetto_init_once(void)
{
   // Connects to the system tracing service
   perfetto::TracingInitArgs args;
   args.backends = perfetto::kSystemBackend;
   perfetto::Tracing::Initialize(args);

   static UtilPerfettoObserver observer;
   perfetto::TrackEvent::Register();

   atexit(&util_perfetto_fini);
}

static once_flag perfetto_once_flag = ONCE_FLAG_INIT;

void
util_perfetto_init(void)
{
   call_once(&perfetto_once_flag, util_perfetto_init_once);
}
