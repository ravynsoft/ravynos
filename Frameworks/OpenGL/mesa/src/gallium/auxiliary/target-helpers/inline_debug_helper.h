
#ifndef INLINE_DEBUG_HELPER_H
#define INLINE_DEBUG_HELPER_H

#include "util/compiler.h"
#include "util/u_debug.h"
#include "util/u_tests.h"


/* Helper function to wrap a screen with
 * one or more debug drivers.
 */

#include "driver_ddebug/dd_public.h"
#include "driver_trace/tr_public.h"
#include "driver_noop/noop_public.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * TODO: Audit the following *screen_create() - all of
 * them should return the original screen on failuire.
 */
static inline struct pipe_screen *
debug_screen_wrap(struct pipe_screen *screen)
{
   screen = ddebug_screen_create(screen);
   screen = trace_screen_create(screen);
   screen = noop_screen_create(screen);

   if (debug_get_bool_option("GALLIUM_TESTS", false))
      util_run_tests(screen);

   return screen;
}

#ifdef __cplusplus
}
#endif

#endif // INLINE_DEBUG_HELPER_H
