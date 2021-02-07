#import "ObjectTesting.h"
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSXMLParser.h>
int main()
{
  NSAutoreleasePool     *arp = [NSAutoreleasePool new];
  NSXMLParser           *parser;
  Class                 c = NSClassFromString(@"GSSloppyXMLParser");

  parser = [c new];
  test_alloc(@"GSSloppyXMLParser");
  test_NSObject(@"GSSloppyXMLParser", [NSArray arrayWithObject: parser]);

  parser = [NSXMLParser new];
  test_alloc(@"NSXMLParser");
  test_NSObject(@"NSXMLParser", [NSArray arrayWithObject: parser]);

  [arp release]; arp = nil;

/* Don't release the parser ... it appears that on OSX there is a bug in
 * NSXMLParser such that deallocation crashes if the -self method has been
 * called on it (which is something the NSObject test does).
  [parser release];
 */
  return 0;
}
