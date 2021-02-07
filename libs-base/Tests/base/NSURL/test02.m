#import <Foundation/Foundation.h>
#import "Testing.h"
#import "ObjectTesting.h"


int main()
{
#if     GNUSTEP
  NSAutoreleasePool   *arp = [NSAutoreleasePool new];
  unsigned		i;
  NSURL			*url;
  NSMutableString	*m;
  NSData                *data;
  NSString              *str;
  NSTask		*t;
  NSString		*helpers;
  NSString		*capture;
  NSMutableURLRequest   *request;
  NSHTTPURLResponse         *response = nil;
  NSError               *error = nil;
  NSFileManager         *fm;
  NSRange               r;
  NSString              *file = @"Capture.dat";

  fm = [NSFileManager defaultManager];
  helpers = [fm currentDirectoryPath];
  helpers = [helpers stringByAppendingPathComponent: @"Helpers"];
  helpers = [helpers stringByAppendingPathComponent: @"obj"];
  capture = [helpers stringByAppendingPathComponent: @"capture"];

  m = [NSMutableString stringWithCapacity: 2048];
  for (i = 0; i < 128; i++)
    {
      [m appendFormat: @"Hello %d\r\n", i];
    }

  t = [NSTask launchedTaskWithLaunchPath: capture
			       arguments: [NSArray arrayWithObjects:
						     nil]];
  if (t != nil)
    {
      // Pause to allow server subtask to set up.
      [NSThread sleepUntilDate: [NSDate dateWithTimeIntervalSinceNow: 0.5]];
      // remove the captured data from a possible previous run
      [fm removeItemAtPath: file error: NULL];
      // making a POST request
      url = [NSURL URLWithString: @"http://localhost:1234/"];
      request = [NSMutableURLRequest requestWithURL: url];
      data = [m dataUsingEncoding: NSUTF8StringEncoding];
      [request setHTTPBody: data];
      [request setHTTPMethod: @"POST"];

      // sending the request
      [NSURLConnection sendSynchronousRequest: request
			    returningResponse: &response
					error: &error];

      // analyzing the response
      PASS(response != nil && [response statusCode] == 204,
	"NSURLConnection synchronous load returns a response");

      data = [NSData dataWithContentsOfFile: @"Capture.dat"];
      str = [[NSString alloc] initWithData: data
				  encoding: NSUTF8StringEncoding];
      r = [str rangeOfString: m];
      PASS(r.location != NSNotFound,
	   "NSURLConnection capture test OK");

      // Wait for server termination
      [t terminate];
      [t waitUntilExit];
      DESTROY(str);
      response = nil;
      error = nil;
    }  

  // the same but with secure connection (HTTPS)
  t = [NSTask launchedTaskWithLaunchPath: capture
			       arguments: [NSArray arrayWithObjects:
						     @"-Secure", @"YES",
						     nil]];
  if (t != nil)
    {
      // Pause to allow server subtask to set up.
      [NSThread sleepUntilDate: [NSDate dateWithTimeIntervalSinceNow: 0.5]];
      // remove the captured data from a possible previous run
      [fm removeItemAtPath: file error: NULL];
      // making a POST request
      url = [NSURL URLWithString: @"https://localhost:1234/"];
      request = [NSMutableURLRequest requestWithURL: url];
      data = [m dataUsingEncoding: NSUTF8StringEncoding];
      [request setHTTPBody: data];
      [request setHTTPMethod: @"POST"];

      // sending the request
      [NSURLConnection sendSynchronousRequest: request
			    returningResponse: &response
					error: &error];

      // sending the request
      PASS(response != nil && [response statusCode] == 204,
	"NSURLConnection synchronous load returns a response");

      data = [NSData dataWithContentsOfFile: @"Capture.dat"];
      str = [[NSString alloc] initWithData: data
				  encoding: NSUTF8StringEncoding];      
      r = [str rangeOfString: m];
      PASS(r.location != NSNotFound,
	   "NSURLConnection capture test OK");

      // Wait for server termination
      [t terminate];
      [t waitUntilExit];
      DESTROY(str);
    }  

  [arp release]; arp = nil;
#endif
  return 0;
}
