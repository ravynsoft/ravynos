#include <Foundation/Foundation.h>
#include "Testing.h"
#include "ObjectTesting.h"

/* This test collection examines the responses when a variety of HTTP
* status codes are returned by the server. Relies on the
* StatusServer helper tool.
*
* Graham J Lee < leeg@thaesofereode.info >
*/

int main(int argc, char **argv)
{
  NSAutoreleasePool   *arp = [NSAutoreleasePool new] ;
  
  NSString *helpers;
  NSString *statusServer;
  NSURL *url;
  NSURLHandle *handle;
  NSTask *t;
  Class cls;
  NSData *resp;
  NSData *rxd;
  
  url = [NSURL URLWithString: @"http://localhost:1234/200"];
  cls = [NSURLHandle URLHandleClassForURL: url];
  resp = [NSData dataWithBytes: "Hello\r\n" length: 7];
  
  helpers = [[NSFileManager defaultManager] currentDirectoryPath];
  helpers = [helpers stringByAppendingPathComponent: @"Helpers"];
  helpers = [helpers stringByAppendingPathComponent: @"obj"];
  statusServer = [helpers stringByAppendingPathComponent: @"StatusServer"];
  
  t = [NSTask launchedTaskWithLaunchPath: statusServer arguments: nil];
  
  if (t != nil)
    {
      // pause, so that the server is set up
      [NSThread sleepUntilDate: [NSDate dateWithTimeIntervalSinceNow: 0.5]];
      // try some different requests
      handle = [[[cls alloc] initWithURL: url cached: NO] autorelease];
      rxd = [handle loadInForeground];
      PASS([rxd isEqual: resp],
           "Got the correct data from a 200 - status load") ;
      PASS([handle status] == NSURLHandleLoadSucceeded,
           "200 - status: Handle load succeeded") ;
      
      url = [NSURL URLWithString: @"http://localhost:1234/401"];
      handle = [[[cls alloc] initWithURL: url cached: NO] autorelease];
      rxd = [handle loadInForeground];
      PASS([handle status] == NSURLHandleNotLoaded,
           "401 - status: Handle load not loaded (unanswered auth challenge)");

      url = [NSURL URLWithString: @"http://localhost:1234/404"];
      handle = [[[cls alloc] initWithURL: url cached: NO] autorelease];
      rxd = [handle loadInForeground];
      PASS([handle status] == NSURLHandleNotLoaded,
	   "404 - status: Handle load not loaded (resource not found)");
      [t terminate];
      [t waitUntilExit];
    }
  
  [arp release]; arp = nil ;
  
  return 0;
}

