#ifndef COMPAT_KQUEUE1_H
#define COMPAT_KQUEUE1_H

int compat_kqueue1(int);

#ifdef COMPAT_ENABLE_KQUEUE1
#define kqueue1 compat_kqueue1
#endif

#endif
