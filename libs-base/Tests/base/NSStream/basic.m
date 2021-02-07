#import "ObjectTesting.h"
#import <Foundation/Foundation.h>
#import <Foundation/NSStream.h>

int main()
{
  NSAutoreleasePool   *arp = [NSAutoreleasePool new];
  NSInputStream *t1;
  NSOutputStream *t2;
  NSArray *a;
  NSHost *host = [NSHost hostWithName:@"localhost"];

  [NSStream getStreamsToHost:host port:80 inputStream:&t1 outputStream:&t2];

  a = [NSArray arrayWithObjects:t1, t2, nil]; 
  test_NSObject(@"NSStream", a); 

  [arp release]; arp = nil;
  return 0;
}
