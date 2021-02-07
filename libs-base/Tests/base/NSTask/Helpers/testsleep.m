#import	<Foundation/Foundation.h>

int
main(int argc, char **argv)
{
  NSAutoreleasePool	*arp = [NSAutoreleasePool new];

  GSPrintf(stdout, @"Child starting\n");
  fflush(stdout);
  [NSThread sleepForTimeInterval: 10.0];
  GSPrintf(stdout, @"Child exiting\n");
  fflush(stdout);
  [arp release];
  return 0;
}

