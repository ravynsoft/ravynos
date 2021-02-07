#import "ObjectTesting.h"
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSXMLDocument.h>
#import <Foundation/NSXMLElement.h>
#import "GNUstepBase/GSConfig.h"

/*
int main()
{
  NSAutoreleasePool *arp = [NSAutoreleasePool new];
  NSXMLElement *elem1 = [[NSXMLElement alloc] initWithXMLString: @"<num>6</num>" error: NULL];
  NSXMLElement *elem2 = [[NSXMLElement alloc] initWithXMLString: @"<num>7</num>" error: NULL];
  NSXMLElement *copy1 = [elem1 copy];
  NSXMLElement *copy2 = [elem2 copy];

  [copy1 setStringValue: @"7"];
  PASS_EQUAL(copy1, copy2, "equal after setStringValue:");

  [arp drain];
  arp = nil;

  return 0;
}
*/

int main()
{
  NSAutoreleasePool *arp = [NSAutoreleasePool new];
  START_SET("NSXMLElement transfer")
#if !GS_USE_LIBXML
    SKIP("library built without libxml2")
#else
  NSXMLDocument *doc;
  NSXMLElement *elem;
  NSXMLNode *child;
  NSString *simpleXML = @"<num>6</num>";

  doc = [[NSXMLDocument alloc] initWithXMLString: simpleXML
					  options: 0
					    error: NULL];
  PASS(doc != nil, "document was initialized from a string");

  // detach the root element from its document
  elem = [doc rootElement];
  //  PASS_EQUAL([elem XMLString], simpleXML, "root element is correct");
  [elem detach];

  // now, simply accessing the text node child of the element leads to a CRASH
  // when releasing the document
  child = [elem childAtIndex: 0];
  
  [doc release];
#endif

  END_SET("NSXMLElement transfer")
  [arp release];
  arp = nil;

  return 0;
}
