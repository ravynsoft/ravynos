#import "ObjectTesting.h"
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSXMLDocument.h>
#import <Foundation/NSXMLElement.h>
#import "GNUstepBase/GSConfig.h"

int main()
{
  NSAutoreleasePool *arp = [NSAutoreleasePool new];
  START_SET("NSXMLDocument")
#if !GS_USE_LIBXML
    SKIP("library built without libxml2")
#else
  NSArray *nodes = nil;
  NSXMLDocument *node;
  NSXMLDocument *node2;
  NSXMLElement *elem;
  NSString *documentXML = 
    @"<?xml version=\"1.0\" encoding=\"utf-8\"?>" 
    @"<bookstore>"
    @"  <book category=\"COOKING\">"
    @"    <title lang=\"en\">Everyday Italian</title>"
    @"    <author>Giada De Laurentiis</author>"
    @"    <year>2005</year>"
    @"    <price>30.00</price>"
    @"  </book>"
    @"  <book category=\"CHILDREN\">"
    @"    <title lang=\"en\">Harry Potter</title>"
    @"    <author>J K. Rowling</author>"
    @"    <year>2005</year>"
    @"    <price>29.99</price>"
    @"  </book>"
    @"  <book category=\"WEB\">"
    @"    <title lang=\"en\">XQuery Kick Start</title>"
    @"    <author>James McGovern</author>"
    @"    <author>Per Bothner</author>"
    @"    <author>Kurt Cagle</author>"
    @"    <author>James Linn</author>"
    @"    <author>Vaidyanathan Nagarajan</author>"
    @"    <year>2003</year>"
    @"    <price>49.99</price>"
    @"  </book>"
    @"  <book category=\"WEB\">"
    @"    <title lang=\"en\">Learning XML</title>"
    @"    <author>Erik T. Ray</author>"
    @"    <year>2003</year>"
    @"    <price>39.95</price>"
    @"  </book>"
    @"</bookstore>";

  node = [NSXMLDocument alloc];
  PASS_EXCEPTION([node initWithData: nil options: 0 error: 0],
    NSInvalidArgumentException,
    "Cannot initialise an XML document with nil data");

  node = [NSXMLDocument alloc];
  PASS_EXCEPTION([node initWithData: (NSData*)@"bad" options: 0 error: 0],
    NSInvalidArgumentException,
    "Cannot initialise an XML document with bad data class");

  node = [[NSXMLDocument alloc] init];
  test_alloc(@"NSXMLDocument");
  test_NSObject(@"NSXMLDocument", [NSArray arrayWithObject: node]);

  elem = [[NSXMLElement alloc] initWithName: @"elem1"];
  [node addChild: elem];
  PASS_EQUAL([[node children] lastObject], elem, "can add elem to doc");
  [elem release];
  elem = [[NSXMLElement alloc] initWithName: @"root"];
  [node setRootElement: elem];
  PASS_EQUAL([[node children] lastObject], elem, "can set elem as root");
  PASS([[node children] count] == 1, "set root removes other children");
  
  PASS_RUNS([node setRootElement: nil], "setting a nil root is ignored");
  PASS_EQUAL([node rootElement], elem, "root element remains");

  node = [[NSXMLDocument alloc] initWithXMLString:documentXML
					  options:0
					    error:NULL];
  elem = [node rootElement];
  PASS(node != nil, "document was initialized from a string");
  PASS_EQUAL([node rootElement], elem, "root element is correct");
  PASS_EQUAL([elem name],@"bookstore", "root element is bookstore");

  nodes = [node nodesForXPath:@"/bookstore/book" error:NULL];
  PASS([nodes count] == 4,
	     "Xpath function returns the correct number of elements (4)");
  elem = [nodes objectAtIndex: 0];
  PASS_EQUAL([elem class],[NSXMLElement class],
	     "first node in Xpath result is an element");
  PASS([[elem name] isEqualToString: @"book"],
       "Got the correct elements from XPath query");

  node2 = [[NSXMLDocument alloc] initWithXMLString:documentXML
					   options:0
					     error:NULL];
  PASS([node isEqual: node2],
       "Equal documents are equivalent");
#endif

  END_SET("NSXMLDocument")
  [arp release];
  arp = nil;

  // [elem release];
  // [node release];
  return 0;
}
