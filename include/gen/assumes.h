#ifndef __OSX_ASSUMES_H__
#define __OSX_ASSUMES_H__

/* The interfaces in this file have been replaced by those in os/assumes.h.
 * Use the os_*() variants instead of these. The posix_assumes_*() macros have
 * moved to os/assumes.h.
 */
#include <os/assumes.h>

__BEGIN_DECLS

#define osx_fastpath(x) os_fastpath(x)
#define osx_slowpath(x) os_slowpath(x)
#define osx_constant(x) os_constant(x)
#define osx_hardware_trap() os_hardware_trap()
#define __OSX_COMPILETIME_ASSERT__(e) __OS_COMPILETIME_ASSERT__((e))

typedef os_redirect_t osx_redirect_t;
typedef os_log_callout_t osx_log_callout_t;

#define osx_set_crash_message(arg) os_set_crash_message(arg)

#define osx_assumes(e) os_assumes((e))
#define osx_assumes_zero(e) os_assumes_zero((e))

#define osx_assert(e) os_assert((e))
#define osx_assert_zero(e) os_assert_zero((e))

#define osx_assumes_ctx(f, ctx, e) os_assumes_ctx((f), (ctx), (e))
#define osx_assumes_zero_ctx(f, ctx, e) os_assumes_zero_ctx((f), (ctx), (e))

#define osx_assert_ctx(f, ctx, e) os_assert_ctx((f), (ctx), (e))
#define osx_assert_zero_ctx(f, ctx, e) os_assert_zero_ctx((f), (ctx), (e))

__OSX_AVAILABLE_STARTING(__MAC_10_7, __IPHONE_4_3)
extern void
_osx_assumes_log(uint64_t code);

__OSX_AVAILABLE_STARTING(__MAC_10_8, __IPHONE_6_0)
extern char *
_osx_assert_log(uint64_t code);

__OSX_AVAILABLE_STARTING(__MAC_10_8, __IPHONE_6_0)
extern void
_osx_assumes_log_ctx(osx_log_callout_t callout, void *ctx, uint64_t code);

__OSX_AVAILABLE_STARTING(__MAC_10_8, __IPHONE_6_0)
extern char *
_osx_assert_log_ctx(osx_log_callout_t callout, void *ctx, uint64_t code);

__OSX_AVAILABLE_STARTING(__MAC_10_8, __IPHONE_6_0)
extern void
_osx_avoid_tail_call(void);

__END_DECLS

#endif /* __OSX_ASSUMES_H__ */
