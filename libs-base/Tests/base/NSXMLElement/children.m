#import "ObjectTesting.h"
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSXMLDocument.h>
#import <Foundation/NSXMLElement.h>
#import "GNUstepBase/GSConfig.h"

int main()
{
  NSAutoreleasePool     *arp = [NSAutoreleasePool new];
  START_SET("NSXMLElement children")
#if !GS_USE_LIBXML
    SKIP("library built without libxml2")
#else
  NSXMLElement          *root1;
  NSXMLElement          *root2;
  NSXMLElement          *child1;
  NSXMLElement          *child2;

  root1 = [[NSXMLElement alloc] initWithName: @"root1"];
  root2 = [[NSXMLElement alloc] initWithName: @"root2"];

  child1 = [[NSXMLElement alloc] initWithName: @"child1"];
  child2 = [[NSXMLElement alloc] initWithName: @"child2"];

/* In OSX (snow leopard) an attempt to add a child beyond the legal range
 * actally causes the data structures to be corrupted so that subsequent
 * operations go horribly wrong ... so we can['t run these tests.
 *
  PASS_EXCEPTION([root1 insertChild: child1 atIndex: 1],
    NSRangeException, "may not add a child at a bad index");
  PASS(0 == [[root1 children] count], "parent has no child after failed insert");
  PASS(nil == [child1 parent], "child has no parent after failed insert");
  PASS_RUNS([child1 detach], "May detach");
*/

  PASS_RUNS([root1 insertChild: child2 atIndex: 0],
    "may add a child at index 0");
  PASS(1 == [[root1 children] count], "parent has a child after insertion");
  PASS_EQUAL([child2 parent], root1, "child has correct parent");
  PASS_RUNS([root1 removeChildAtIndex: 0],
   "removing child works");
  PASS_EQUAL([root1 children], nil, "children is nil after removal");
  PASS_EQUAL([child2 parent], nil, "child has no parent");
  PASS_RUNS([root1 insertChild: child2 atIndex: 0],
    "may reinsert a child at index 0");

  PASS_RUNS([root1 insertChild: child1 atIndex: 0],
    "may add a child at index 0");
  PASS(2 == [[root1 children] count], "parent has a child after insertion");

  PASS_EXCEPTION([root2 insertChild: child1 atIndex: 0],
    NSInternalInconsistencyException, 
    "cannot add a child if it has a parent");
  
  {
    NSXMLNode   *c;

    c = [[[NSXMLNode alloc] initWithKind: NSXMLElementKind] autorelease];
    PASS_RUNS([root1 insertChild: c atIndex: 0],
      "may add NSXMLElementKind child");

    c = [[[NSXMLNode alloc] initWithKind:
      NSXMLProcessingInstructionKind] autorelease];
    PASS_RUNS([root1 insertChild: c atIndex: 0],
      "may add NSXMLProcessingInstructionKind child");

    c = [[[NSXMLNode alloc] initWithKind: NSXMLTextKind] autorelease];
    PASS_RUNS([root1 insertChild: c atIndex: 0],
      "may add NSXMLTextKind child");

    c = [[[NSXMLNode alloc] initWithKind: NSXMLCommentKind] autorelease];
    PASS_RUNS([root1 insertChild: c atIndex: 0],
      "may add NSXMLCommentKind child");

    /* Removed based on test run on Cocoa.
    c = [[[NSXMLNode alloc] initWithKind:
      NSXMLAttributeDeclarationKind] autorelease];
    PASS_RUNS([root1 insertChild: c atIndex: 0],
      "may add NSXMLAttributeDeclarationKind child");
    */

    c = [[[NSXMLNode alloc] initWithKind:
      NSXMLEntityDeclarationKind] autorelease];
    PASS_EXCEPTION([root1 insertChild: c atIndex: 0], nil,
      "may not add NSXMLEntityDeclarationKind child");

    c = [[[NSXMLNode alloc] initWithKind:
      NSXMLElementDeclarationKind] autorelease];
    PASS_EXCEPTION([root1 insertChild: c atIndex: 0], nil,
      "may not add NSXMLElementDeclarationKind child");

    c = [[[NSXMLNode alloc] initWithKind:
      NSXMLNotationDeclarationKind] autorelease];
    PASS_EXCEPTION([root1 insertChild: c atIndex: 0], nil,
      "may not add NSXMLNotationDeclarationKind child");

    c = [[[NSXMLNode alloc] initWithKind: NSXMLInvalidKind] autorelease];
    PASS_EXCEPTION([root1 insertChild: c atIndex: 0],
      nil, "may not add NSXMLInvalidKind child");

    c = [[[NSXMLNode alloc] initWithKind: NSXMLDocumentKind] autorelease];
    PASS_EXCEPTION([root1 insertChild: c atIndex: 0],
      nil, "may not add NSXMLDocumentKind child");

    c = [[[NSXMLNode alloc] initWithKind: NSXMLDTDKind] autorelease];
    PASS_EXCEPTION([root1 insertChild: c atIndex: 0],
      nil, "may not add NSXMLDTDKind child");

    c = [[[NSXMLNode alloc] initWithKind: NSXMLNamespaceKind] autorelease];
    PASS_EXCEPTION([root1 insertChild: c atIndex: 0],
      nil, "may not add NSXMLNamespaceKind child");

    c = [[[NSXMLNode alloc] initWithKind: NSXMLAttributeKind] autorelease];
    PASS_EXCEPTION([root1 insertChild: c atIndex: 0],
      nil, "may not add NSXMLAttributeKind child");
  }

  PASS(1 == [child1 level], "child element node level is one");

  PASS_RUNS([root1 setChildren: nil],
    "may set a nil array of children");
  PASS(0 == [[root1 children] count], "setting nil children works");

  PASS_EXCEPTION([root1 removeChildAtIndex: 100], NSRangeException,
   "removing child from invalid index raises");


  [root1 release];
  [root2 release];
  [child1 release];
  [child2 release];
#endif
  END_SET("NSXMLElement children")
  [arp release];
  arp = nil;

  return 0;
}
