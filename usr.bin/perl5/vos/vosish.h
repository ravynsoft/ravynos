#ifdef __GNUC__
#include "../unixish.h"
#else
#include "unixish.h"
#endif

/* VOS does not support SA_SIGINFO, so undefine the macro.  This
   is a work-around for posix-1302.  */
#undef SA_SIGINFO

/* Specify a prototype for truncate() since we are supplying one. */
extern int truncate (const char *path, off_t len);
