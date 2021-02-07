#import <Foundation/Foundation.h>
#import "Testing.h"
#import "ObjectTesting.h"

/* this test collection examines the behaviour of the
 * NSURLHandleClient protocol.
 * Graham J Lee <leeg@thaesofereode.info>
 */

typedef enum _URLHandleClientStatus {
  URLHandleClientNormal = 0,
  URLHandleClientDataDidBecomeAvailable,
  URLHandleClientDidFailLoadingWithReason,
  URLHandleClientDidBeginLoading,
  URLHandleClientDidCancelLoading,
  URLHandleClientDidFinishLoading } URLHandleClientStatus;

@interface TestObject : NSObject <NSURLHandleClient>
{
  @protected
  URLHandleClientStatus _status;
  NSData *_receivedData;
}
- (int) runTest;

- (URLHandleClientStatus) status;
- (void) setStatus: (URLHandleClientStatus)newStatus;

- (void) URLHandle: (NSURLHandle *)sender
resourceDataDidBecomeAvailable: (NSData *)newBytes;
- (void) URLHandle: (NSURLHandle *)sender
resourceDidFailLoadingWithReason: (NSString *)reason;
- (void) URLHandleResourceDidBeginLoading: (NSURLHandle *)sender;
- (void) URLHandleResourceDidCancelLoading: (NSURLHandle *)sender;
- (void) URLHandleResourceDidFinishLoading: (NSURLHandle *)sender;
@end

@implementation TestObject

- (id) init
{
  if ((self = [super init]))
    {
      _status = URLHandleClientNormal;
      _receivedData = nil;
    }
  return self;
}

- (void) dealloc
{
  if (_receivedData)
    {
      [_receivedData release];
    }
  [super dealloc];
}

- (URLHandleClientStatus) status { return _status; }
- (void) setStatus: (URLHandleClientStatus)newStatus { _status = newStatus; }

- (void) URLHandle: (NSURLHandle *)sender
resourceDataDidBecomeAvailable: (NSData *)newBytes
{
  [self setStatus: URLHandleClientDataDidBecomeAvailable];
  if (_receivedData)
    {
      [_receivedData release];
    }
  _receivedData = newBytes;
}

- (void) URLHandle: (NSURLHandle *)sender
resourceDidFailLoadingWithReason: (NSString *)reason
{
  [self setStatus: URLHandleClientDidFailLoadingWithReason];
  NSLog(@"Load failed: further tests may fail.  Reason: %@", reason);
}

- (void) URLHandleResourceDidBeginLoading: (NSURLHandle *)sender
{
  [self setStatus: URLHandleClientDidBeginLoading];
}

- (void) URLHandleResourceDidCancelLoading: (NSURLHandle *)sender
{
  [self setStatus: URLHandleClientDidCancelLoading];
}

- (void) URLHandleResourceDidFinishLoading: (NSURLHandle *)sender
{
  [self setStatus: URLHandleClientDidFinishLoading];
}

- (int)runTest
{
  id handle;
  NSURL *url;
  Class cls;

  url = [NSURL URLWithString: @"http://www.gnustep.org/"];
  cls = [NSURLHandle URLHandleClassForURL: url];
  handle = [[cls alloc] initWithURL: url cached: NO];

  [handle addClient: self];
  [self setStatus: URLHandleClientNormal];

  [handle beginLoadInBackground];
  [handle cancelLoadInBackground];
  PASS([self status] == URLHandleClientDidCancelLoading,
    "URLHandleResourceDidCancelLoading called");
  [handle release];

  handle = [[cls alloc] initWithURL: url cached: NO];
  [handle addClient: self];
  /* Don't get client messages in the foreground, so load in
   * background and wait a bit
   */
  [handle loadInBackground];
  PASS([self status] == URLHandleClientDidBeginLoading,
    "URLHandleResourceDidBeginLoading called");

  [handle release];
  return 0;
}

@end

int main(int argc, char **argv)
{
  int status;

  NSAutoreleasePool   *arp = [NSAutoreleasePool new];
  status = [[[[TestObject alloc] init] autorelease] runTest];
  [arp release]; arp = nil;

  return status;
}
