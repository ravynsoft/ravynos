/* If we're compiling with watcom, we want to silence domain errors */
#if defined(__QNX__) && defined(__WATCOMC__)
#include <math.h>

/* Return default value and print no error message */
int matherr( struct exception *err )
  {
        return 1;
  }

#endif
