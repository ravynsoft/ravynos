#import <Foundation/Foundation.h>
#import "Testing.h"
#import "ObjectTesting.h"


int main()
{
#if     GNUSTEP
  NSAutoreleasePool   *arp = [NSAutoreleasePool new];
  unsigned		i;
  NSURL			*url;
  NSData		*data;
  NSData		*resp;
  NSString		*str;
  NSMutableString	*m;
  NSTask		*t;
  NSString		*helpers;
  NSString		*keepalive;
  
  helpers = [[NSFileManager defaultManager] currentDirectoryPath];
  helpers = [helpers stringByAppendingPathComponent: @"Helpers"];
  helpers = [helpers stringByAppendingPathComponent: @"obj"];
  keepalive = [helpers stringByAppendingPathComponent: @"keepalive"];

  url = [NSURL URLWithString: @"http://localhost:4322/"];

  m = [NSMutableString stringWithCapacity: 2048];
  for (i = 0; i < 128; i++)
    {
      [m appendFormat: @"Hello %d\r\n", i];
    }
  resp = [m dataUsingEncoding: NSASCIIStringEncoding];
  [resp writeToFile: @"KAResponse.dat" atomically: YES];

  t = [NSTask launchedTaskWithLaunchPath: keepalive
    arguments: [NSArray arrayWithObjects:
    @"-FileName", @"KAResponse.dat",
    @"-CloseFreq", @"3",
    @"-Count", @"10",
    nil]];
  if (t != nil)
    {
      // Pause to allow server subtask to set up.
      [NSThread sleepUntilDate: [NSDate dateWithTimeIntervalSinceNow: 0.5]];
      
      for (i = 0; i < 10; i++)
	{
	  /*we just get the response every time.  It should be the
	   *same every time, even though the headers change and
	   *sometimes the connection gets dropped
	   */
	  char buf[BUFSIZ];
	  data = [url resourceDataUsingCache: NO];
	  str = [url propertyForKey: NSHTTPPropertyStatusCodeKey];
	  sprintf(buf, "keep-alive test %d OK",i);
	  PASS([data isEqual:resp], "%s", buf)
	}
      [t terminate];
      [t waitUntilExit];
    }  

  [arp release]; arp = nil;
#endif
  return 0;
}
