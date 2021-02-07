#import "ObjectTesting.h"
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSXMLNode.h>
#import <Foundation/NSValue.h>
#import "GNUstepBase/GSConfig.h"

int main()
{
  NSAutoreleasePool *arp = [NSAutoreleasePool new];
  START_SET("NSXMLNode")
#if !GS_USE_LIBXML
    SKIP("library built without libxml2")
#else
  NSXMLNode *node;
  NSXMLNode *other;
  NSXMLNode *text;
  NSXMLNode *pi;
  NSXMLNode *comment;
  NSXMLNode *ns;
  NSXMLNode *attr;
  NSNumber *number;
  NSArray *instances;

  test_alloc(@"NSXMLNode");

  node = [[NSXMLNode alloc] initWithKind: NSXMLInvalidKind];
  other = [[NSXMLNode alloc] initWithKind: NSXMLElementKind];
  // We need to set the name, otherwise isEqual: wont work.
  [other setName: @"test"];
  text = [NSXMLNode textWithStringValue: @"Text node"];
  pi = [NSXMLNode processingInstructionWithName: @"PI name"
                                    stringValue: @"PI string"];
  ns = [NSXMLNode namespaceWithName: @"name space name"
                        stringValue: @"name space string"];
  comment = [NSXMLNode commentWithStringValue: @"Comment node"];
  attr = [NSXMLNode attributeWithName: @"key"
			  stringValue: @"value"];
  instances = [NSArray arrayWithObjects: node, other, text, pi, 
                       ns, comment, attr, nil];
  test_NSObject(@"NSXMLNode", instances);
  test_NSCopying(@"NSXMLNode", @"NSXMLNode", instances, NO, YES);

  PASS(NO == [other isEqual: node], "different node kinds are not equal");
  [other release];

  other = [[NSXMLNode alloc] initWithKind: NSXMLInvalidKind];
  PASS([other isEqual: node], "invalid nodes are equal");

  // Tests on invalid node
  PASS(NSXMLInvalidKind == [node kind], "invalid node kind is correct");
  PASS(0 == [node level], "invalid node level is zero");
  PASS_EQUAL([node name], nil, "invalid node name is nil");
  PASS_EQUAL([node URI], nil, "invalid node URI is nil");
  PASS_EQUAL([node objectValue], nil, "invalid node object value is nil");
  PASS_EQUAL([node stringValue], @"", "invalid node string value is empty");
  PASS_EQUAL([node children], nil, "invalid node children is nil");

  [node setName: @"name"];
  PASS_EQUAL([node name], nil,
    "setting name on invalid node gives a nil name");
  [node setURI: @"URI"];
  PASS_EQUAL([node URI], nil,
    "setting URI on invalid node gives a nil URI");
  [node setObjectValue: @"anObject"];
  PASS_EQUAL([node objectValue], @"anObject",
    "setting object value on invalid node works");
  [node setObjectValue: nil];
  PASS([node childCount] == 0, "No child after setting object value");
  // Per documentation on NSXMLNode setObjectValue/objectValue, 
  // On 10.6 this returns nil not @""
  PASS_EQUAL([node objectValue], nil,
    "setting nil object value on invalid node works");
  PASS_EQUAL([node stringValue], @"",
    "setting nil object value on invalid node gives empty string");
    
  number = [NSNumber numberWithInt: 12];
  [node setObjectValue: number];
  PASS_EQUAL([node objectValue], number,
    "setting object value on invalid node works");
  testHopeful = YES;
  PASS_EQUAL([node stringValue], @"1,2E1",
    "setting object value on invalid node sets string value");
  testHopeful = NO;
  [node setObjectValue: nil];
  
  [node setStringValue: @"aString"];
  PASS_EQUAL([node stringValue], @"aString",
    "setting string value on invalid node works");
  PASS_EQUAL([node objectValue], @"aString",
    "setting string value on invalid node sets object value");
   [node setStringValue: nil];
  PASS_EQUAL([node stringValue], @"",
    "setting nil string value on invalid node gives empty string");
  PASS_EQUAL([node objectValue], nil,
    "setting nil string value on invalid node sets object value to nil");

  [node release];
  [other release];

  // Tests on attribute node
  attr = [NSXMLNode attributeWithName: @"key"
			  stringValue: @"value"];
  PASS(NSXMLAttributeKind == [attr kind], "attr node kind is correct");
  PASS(0 == [attr level], "attr node level is zero");
  PASS_EQUAL([attr name], @"key", "name on attr node works");
  PASS_EQUAL([attr URI], nil, "attr node URI is nil");
  PASS_EQUAL([attr objectValue], @"value", "attr node object value works");
  PASS_EQUAL([attr stringValue], @"value", "string value on attr node works");
  // In libxml2 the value is on a child node, but we don't report that
  PASS_EQUAL([attr children], nil, "attr node children is nil");
  PASS([attr childCount] == 0, "No child on attr node");

  [attr setName: @"name"];
  PASS_EQUAL([attr name], @"name",
    "setting name on attr node works");
  [attr setStringValue: @"aString"];
  PASS_EQUAL([attr stringValue], @"aString",
    "setting string value on attr node works");
#endif

  END_SET("NSXMLNode")
  [arp release];
  arp = nil;

  return 0;
}
