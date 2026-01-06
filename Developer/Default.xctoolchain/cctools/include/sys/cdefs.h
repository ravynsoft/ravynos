#if defined(__GLIBC__) || defined(__APPLE__) || defined(__FreeBSD__) || \
    defined(__OpenBSD__) || defined(__NetBSD__) || defined(__ANDROID__)

#include_next <sys/cdefs.h>

#else

#ifdef __cplusplus
#define __BEGIN_DECLS extern "C" {
#define __END_DECLS }
#else
#define __BEGIN_DECLS
#define __END_DECLS
#endif

#define __P(x) x

#endif /* __GLIBC__ || __APPLE__ */

#ifdef __GLIBC__

/*
 * Workaround for a GLIBC bug.
 * https://sourceware.org/bugzilla/show_bug.cgi?id=14952
 */

#ifndef __extern_inline
# define __extern_inline \
  extern __inline __attribute__ ((__gnu_inline__))
#endif

#ifndef __extern_always_inline
# define __extern_always_inline \
  extern __always_inline __attribute__ ((__gnu_inline__))
#endif

#endif /* __GLIBC__ */
