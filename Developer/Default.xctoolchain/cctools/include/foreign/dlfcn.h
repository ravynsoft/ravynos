#ifndef __CCTOOLS_PORT_DLFCN_H__
#define __CCTOOLS_PORT_DLFCN_H__
#include_next <dlfcn.h>
#include <stdio.h> /* stderr */
#ifdef __CYGWIN__
typedef struct dl_info {
    const char  *dli_fname;
    void        *dli_fbase;
    const char  *dli_sname;
    void        *dli_saddr;
} Dl_info;

static inline int dladdr(void *addr, Dl_info *info)
{
    fprintf(stderr, "dladdr() not implemented\n");
    return 0;
}
#endif /* __CYGWIN__ */
#endif /* __CCTOOLS_PORT_DLFCN_H__ */
