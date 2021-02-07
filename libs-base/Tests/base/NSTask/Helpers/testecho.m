#import	<Foundation/Foundation.h>

int
main(int argc, char **argv)
{
  NSAutoreleasePool	*arp = [NSAutoreleasePool new];
  NSArray		*args = [[NSProcessInfo processInfo] arguments];
  unsigned		c = [args count];
  int			i;

  for (i = 0; i < c; i++)
    {
      GSPrintf(stdout, @"%s%@", (i == 0 ? "" : " "), [args objectAtIndex: i]);
    }
  GSPrintf(stdout, @"\n");
  fflush(stdout);
  [arp release];
  return 0;
}

