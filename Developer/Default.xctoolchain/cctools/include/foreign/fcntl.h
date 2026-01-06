#include_next <fcntl.h>

#ifndef O_FSYNC
#define O_FSYNC O_SYNC /* Cygwin */
#endif

