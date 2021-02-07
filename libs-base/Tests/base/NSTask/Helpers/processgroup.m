#include	<Foundation/Foundation.h>
#if	!defined(__MINGW32__)
#include	<sys/file.h>
#include        <sys/fcntl.h>
#include        <unistd.h>
#endif

/* Test that the process group has been changed (not the same as that of our
 * parent) and that we have been detached from any controlling terminal.
 */
int
main(int argc, char **argv)
{
  int	i = 0;
  NSAutoreleasePool   *arp = [NSAutoreleasePool new];

#if	!defined(__MINGW32__)
/*
  printf("argc %d\n", argc);
  for (i = 0; i < argc; i++)
    printf("argv[%d] %s\n", i, argv[i]);
  printf("getpgrp %d\n", getpgrp());
  printf("getsid %d\n", getsid(0));
  printf("result of open of /dev/tty is %d\n", open("/dev/tty", O_WRONLY));
*/
  if (atoi(argv[1]) == getpgrp())
    i = 1;                                      /* pgrp not set properly */
  else if (open("/dev/tty", O_WRONLY) >= 0)
    i = 2;                                      /* not detached from tty */
  else
    i = 0;                                      /* OK */
#endif  /* __MINGW32__ */

  [arp release];
  return i;
}

