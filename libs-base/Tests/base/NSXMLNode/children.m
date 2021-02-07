#import "Testing.h"
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSXMLNode.h>
#import <Foundation/NSXMLDocument.h>
#import <Foundation/NSXMLElement.h>
#import "GNUstepBase/GSConfig.h"

int main()
{
  START_SET("NSXMLNode - handling children")
#if !GS_USE_LIBXML
    SKIP("library built without libxml2")
#else
  NS_DURING
  {
    NSXMLElement *node = [[NSXMLElement alloc] initWithKind: NSXMLElementKind];
    NSXMLDocument *docA = nil;
    NSXMLDocument *docB = nil;
    NSXMLNode *attr;
    
    [node setName: @"name"];
    attr = [NSXMLNode attributeWithName: @"key" stringValue: @"value"];
    [node addAttribute: attr];
    PASS(node == [attr parent], "Attr parent is set to node");
    
    PASS_EXCEPTION([node addAttribute: attr], NSInternalInconsistencyException, "Cannot add attribute twice");

    [node release];
    PASS(nil == [attr parent], "Attr parent is set to nil");

    node = [[NSXMLElement alloc] initWithKind: NSXMLElementKind];
    [node setName: @"name"];
    [node addAttribute: attr];
    docA = [[NSXMLDocument alloc] initWithRootElement: node];
    PASS(docA == [node parent], "Parent is set to docA");

    // NSLog(@"Here...");
    [node detach];
    PASS((docB = [[NSXMLDocument alloc] initWithRootElement: node]), "Detached children can be reattached.");
    [docA release];

    // NSLog(@"Here... again");
    PASS(docB == [node parent], "Parent is set to docB");
 
    [docB release];
    PASS(nil == [node parent], "Parent is set to nil");
    docA = [[NSXMLDocument alloc] initWithRootElement: node];
    // NSLog(@"Yet again");
    PASS_EXCEPTION(docB = [[NSXMLDocument alloc] initWithRootElement: node], NSInternalInconsistencyException, "Reusing a child throws an exception");
    // NSLog(@"Last time");
    
    [node release];
    //[docA release];
   }
  NS_HANDLER
  {
    PASS (0 == 1, "NSXML child handling working."); // I don't think this is valid... commenting out for now.
  }
  NS_ENDHANDLER
#endif
  END_SET("NSXMLNode - handling children")
  return 0;
}
