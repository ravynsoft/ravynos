#import "ObjectTesting.h"
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSXMLDocument.h>
#import <Foundation/NSXMLElement.h>
#import "GNUstepBase/GSConfig.h"

int main()
{
  NSAutoreleasePool *arp = [NSAutoreleasePool new];
  START_SET("NSXMLDocument CDATA")
#if !GS_USE_LIBXML
    SKIP("library built without libxml2")
#else
  NSString *docString = @"<root><node><![CDATA[How to read this text ?]]></node></root>";
  NSData *data = [docString dataUsingEncoding: NSUTF8StringEncoding];
  NSError *outError = nil;
  NSXMLDocument *document = [[[NSXMLDocument alloc]
                               initWithData: data
                                    options: (NSXMLNodePreserveCDATA | NSXMLNodePreserveWhitespace)
                                      error: &outError] autorelease];
  NSXMLElement *rootElement = [document rootElement];
  NSXMLNode *childNode = [rootElement childAtIndex: 0];
  NSString *cData = [childNode stringValue];
  PASS_EQUAL(cData, @"How to read this text ?", "CDATA element is correct");
#endif

  END_SET("NSXMLDocument CDATA")
  [arp release];
  arp = nil;

  return 0;
}
