/*
 * Copyright 2022 Google LLC
 * SPDX-License-Identifier: MIT
 */

#ifndef CPU_TRACE_H
#define CPU_TRACE_H

#include "u_perfetto.h"
#include "u_gpuvis.h"

#include "util/macros.h"

#if defined(HAVE_PERFETTO)

/* note that util_perfetto_is_tracing_enabled always returns false util
 * util_perfetto_init is called
 */
#define _MESA_TRACE_BEGIN(name)                                              \
   do {                                                                      \
      if (unlikely(util_perfetto_is_tracing_enabled()))                      \
         util_perfetto_trace_begin(name);                                    \
   } while (0)

#define _MESA_TRACE_END()                                                    \
   do {                                                                      \
      if (unlikely(util_perfetto_is_tracing_enabled()))                      \
         util_perfetto_trace_end();                                          \
   } while (0)

/* NOTE: for now disable atrace for C++ to workaround a ndk bug with ordering
 * between stdatomic.h and atomic.h.  See:
 *
 *   https://github.com/android/ndk/issues/1178
 */
#elif defined(ANDROID) && !defined(__cplusplus)

#include <cutils/trace.h>

#define _MESA_TRACE_BEGIN(name)                                              \
   atrace_begin(ATRACE_TAG_GRAPHICS, name)
#define _MESA_TRACE_END() atrace_end(ATRACE_TAG_GRAPHICS)

#else

#define _MESA_TRACE_BEGIN(name)
#define _MESA_TRACE_END()

#endif /* HAVE_PERFETTO */

#if defined(HAVE_GPUVIS)

#define _MESA_GPUVIS_TRACE_BEGIN(name) util_gpuvis_begin(name)
#define _MESA_GPUVIS_TRACE_END() util_gpuvis_end()

#else

#define _MESA_GPUVIS_TRACE_BEGIN(name)
#define _MESA_GPUVIS_TRACE_END()

#endif /* HAVE_GPUVIS */

#if __has_attribute(cleanup) && __has_attribute(unused)

#define _MESA_TRACE_SCOPE_VAR_CONCAT(name, suffix) name##suffix
#define _MESA_TRACE_SCOPE_VAR(suffix)                                        \
   _MESA_TRACE_SCOPE_VAR_CONCAT(_mesa_trace_scope_, suffix)

/* This must expand to a single non-scoped statement for
 *
 *    if (cond)
 *       _MESA_TRACE_SCOPE(...)
 *
 * to work.
 */
#define _MESA_TRACE_SCOPE(name)                                              \
   int _MESA_TRACE_SCOPE_VAR(__LINE__)                                       \
      __attribute__((cleanup(_mesa_trace_scope_end), unused)) =              \
         _mesa_trace_scope_begin(name)

static inline int
_mesa_trace_scope_begin(const char *name)
{
   _MESA_TRACE_BEGIN(name);
   _MESA_GPUVIS_TRACE_BEGIN(name);
   return 0;
}

static inline void
_mesa_trace_scope_end(UNUSED int *scope)
{
   _MESA_GPUVIS_TRACE_END();
   _MESA_TRACE_END();
}

#else

#define _MESA_TRACE_SCOPE(name)

#endif /* __has_attribute(cleanup) && __has_attribute(unused) */

#define MESA_TRACE_SCOPE(name) _MESA_TRACE_SCOPE(name)
#define MESA_TRACE_FUNC() _MESA_TRACE_SCOPE(__func__)

static inline void
util_cpu_trace_init()
{
   util_perfetto_init();
   util_gpuvis_init();
}

#endif /* CPU_TRACE_H */
