#import "ObjectTesting.h"
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSXMLDocument.h>
#import <Foundation/NSXMLElement.h>
#import "GNUstepBase/GSConfig.h"

int main()
{
  NSAutoreleasePool     *arp = [NSAutoreleasePool new];
  START_SET("NSXMLElement attributes")
#if !GS_USE_LIBXML
    SKIP("library built without libxml2")
#else
  NSXMLElement          *root1;
  NSXMLElement          *root2;
  NSXMLNode          *attr1;
  NSXMLNode          *attr2;
  NSXMLNode          *attrSameNameAsAttr1;

  root1 = [[NSXMLElement alloc] initWithName: @"root1"];
  root2 = [[NSXMLElement alloc] initWithName: @"root2"];

  attr1 = [NSXMLNode attributeWithName: @"attr1" stringValue: @"foo"];
  attr2 = [NSXMLNode attributeWithName: @"attr2" stringValue: @"foo"];
  attrSameNameAsAttr1 = [NSXMLNode attributeWithName: @"attr1" stringValue: @"foo"];


  PASS_RUNS([root1 addAttribute: attr1],
    "may add attributes");
  [root1 addAttribute: attr2];
  PASS_EQUAL([root1 attributeForName: @"attr1"], attr1,
    "element returns attribute by name");
  PASS_RUNS([root1 removeAttributeForName: @"attr2"],
    "removing attributes by name works");
  PASS_EQUAL([root1 attributeForName: @"attr2"], nil,
    "attribute is nil after removal");

  [root1 addAttribute: attrSameNameAsAttr1];
  PASS_EQUAL([root1 attributeForName: @"attr1"], attr1,
    "may not overwrite pre-existing attributes");

  PASS_EXCEPTION([root2 addAttribute: attr1],
    NSInternalInconsistencyException,
    "cannot add attributes to multiple parents");



  [root1 release];
  [root2 release];
#endif
  END_SET("NSXMLElement attributes")
  [arp release];
  arp = nil;

  return 0;
}
