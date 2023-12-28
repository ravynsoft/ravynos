#include <features.h>
#if defined(__UCLIBC__)
LIBC=uclibc
#elif defined(__dietlibc__)
LIBC=dietlibc
#elif defined(__GLIBC__)
LIBC=gnu
#else
#include <stdarg.h>
/* First heuristic to detect musl libc.  */
#ifdef __DEFINED_va_list
LIBC=musl
#else
LIBC=gnu
#endif
#endif
