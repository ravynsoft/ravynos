/*
 * This is needed to work-around a clash of __block usage
 * between Clang and older glibc. For example on CentOS 7.x
 * See https://www.sourceware.org/bugzilla/show_bug.cgi?id=11157
 */
#ifdef __block
#       undef __block
#       include_next <unistd.h>
#       define __block __attribute__((__blocks__(byref)))
#else
#       include_next <unistd.h>
#endif

#ifndef L_SET
#define L_SET SEEK_SET /* Cygwin */
#endif
