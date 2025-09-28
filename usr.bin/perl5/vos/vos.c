/* Beginning of modification history */
/* Written 02-01-02 by Nick Ing-Simmons (nick@ing-simmons.net) */
/* Modified 02-03-27 by Paul Green (Paul.Green@stratus.com) to
     add socketpair() dummy. */
/* Modified 02-04-24 by Paul Green (Paul.Green@stratus.com) to
     have pow(0,0) return 1, avoiding c-1471. */
/* Modified 06-09-25 by Paul Green (Paul.Green@stratus.com) to
     add syslog entries. */
/* Modified 08-02-04 by Paul Green (Paul.Green@stratus.com) to
     open the syslog file in the working dir. */
/* Modified 11-10-17 by Paul Green to remove the dummy copies
     of socketpair() and the syslog functions. */
/* End of modification history */

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

/* VOS doesn't supply a truncate function, so we build one up
   from the available POSIX functions.  */

int
truncate(const char *path, off_t len)
{
 int fd = open(path,O_WRONLY);
 int code = -1;
 if (fd >= 0) {
   code = ftruncate(fd,len);
   close(fd); 
 }
 return code;
}

/* Supply a private version of the power function that returns 1
   for x**0.  This avoids c-1471.  Abigail's Japh tests depend
   on this fix.  We leave all the other cases to the VOS C
   runtime.  */

double s_crt_pow(double *x, double *y);

double pow(x,y)
double x, y;
{
     if (y == 0e0)                 /* c-1471 */
     {
          errno = EDOM;
          return (1e0);
     }

     return(s_crt_pow(&x,&y));
}
