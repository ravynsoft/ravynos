#import "ObjectTesting.h"
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSXMLElement.h>
#import "GNUstepBase/GSConfig.h"

int main()
{
  NSAutoreleasePool     *arp = [NSAutoreleasePool new];
  START_SET("NSXMLElement")
#if !GS_USE_LIBXML
    SKIP("library built without libxml2")
#else
  NSXMLElement          *node;
  NSXMLElement          *other;
  NSXMLElement          *xml;
  NSXMLNode *attr;
  NSArray *instances;

  node = [[NSXMLElement alloc] initWithName: @"node"];
  xml = [[NSXMLElement alloc] initWithXMLString: @"<element attr=\"value\"></element>"
                                          error: NULL];
  other = [NSXMLNode elementWithName: @"other" children: nil attributes: nil];
  instances = [NSArray arrayWithObjects: node, other, xml, nil];
  test_alloc(@"NSXMLElement");
  test_NSObject(@"NSXMLElement", instances);
  test_NSCopying(@"NSXMLElement", @"NSXMLElement", instances, NO, YES);

  other = [[NSXMLElement alloc] initWithName: @"other"];
  PASS(NO == [other isEqual: node], "differently named elements are not equal");

  [other setName: @"node"];
  PASS_EQUAL([other name], @"node", "setting name of element works");
  PASS([other isEqual: node], "elements with same name are equal");

  [other release];

  PASS(NSXMLElementKind == [node kind], "element node kind is correct");
  PASS(0 == [node level], "element node level is zero");
  PASS_EQUAL([node URI], nil, "element node URI is nil");
  PASS_EQUAL([node objectValue], @"", "element node object value is empty");
  PASS_EQUAL([node stringValue], @"", "element node string value is empty");
  PASS_EQUAL([node children], nil, "element node children is nil");

  [node setURI: @"URI"];
  PASS_EQUAL([node URI], @"URI",
    "setting URI on element node works");
  [node setObjectValue: @"anObject"];
  PASS_EQUAL([node objectValue], @"anObject",
    "setting object value on element node works");
  [node setObjectValue: nil];
  PASS_EQUAL([node objectValue], @"",
    "setting nil object value on element node gives empty string");
  [node setStringValue: @"aString"];
  PASS_EQUAL([node stringValue], @"aString",
    "setting string value on element node works");
  [node setStringValue: nil];
  PASS_EQUAL([node stringValue], @"",
    "setting nil string value on element node gives empty string");

  [node release];

  // Equality tests.
  node = [[NSXMLNode alloc] initWithKind: NSXMLElementKind];
  other = [[NSXMLNode alloc] initWithKind: NSXMLElementKind];
  [other setName: @"test"];
  [node setName: @"test"];
  PASS([node isEqual: other], 
       "Elements with the same name are equal");
  
  attr = [NSXMLNode attributeWithName: @"key"
			  stringValue: @"value"];
  [node addAttribute:attr];
  PASS(![node isEqual: other],
       "Elements with different attributes are NOT equal");

  attr = [NSXMLNode attributeWithName: @"key"
			  stringValue: @"value"];
  [other addAttribute:attr];
  PASS([node isEqual: other], 
       "Elements with the same attributes are equal");

  [other setStringValue: @"value"];
  PASS(![node isEqual: other],
       "Elements with different values are NOT equal");

  [node setStringValue: @"value"];
  PASS([node isEqual: other],
       "Elements with same values are equal");

  [node release];
  [other release];
#endif
  END_SET("NSXMLElement")
  [arp release];
  arp = nil;

  return 0;
}
