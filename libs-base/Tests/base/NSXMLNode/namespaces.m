// Based on code with the following copyright:
/* Copyright (c) 2008 Google Inc.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/
#import "Testing.h"
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSArray.h>
#import <Foundation/NSXMLNode.h>
#import <Foundation/NSXMLDocument.h>
#import <Foundation/NSXMLElement.h>
#import "GNUstepBase/GSConfig.h"

NSString *kGDataNamespaceGData = @"http://schemas.google.com/g/2005";
NSString *kGDataNamespaceBatch = @"http://schemas.google.com/gdata/batch";
NSString *kGDataNamespaceAtom = @"http://www.w3.org/2005/Atom";
NSString *kGDataNamespaceOpenSearch = @"http://a9.com/-/spec/opensearchrss/1.0/";
NSString *kGDataNamespaceAtomPrefix = @"atom";

int main()
{
  START_SET("NSXMLNode namespace handling")
#if !GS_USE_LIBXML
    SKIP("library built without libxml2")
#else
  NSAutoreleasePool     *arp = [NSAutoreleasePool new];

  // create elements and attributes

  // create with URI and local name
  NSXMLNode *attr1 = [NSXMLNode attributeWithName: @"foo"
                                              URI: kGDataNamespaceGData
                                      stringValue: @"baz"];
  NSXMLElement *child1 = [NSXMLNode elementWithName: @"chyld"
                                                URI: kGDataNamespaceGData];
  [child1 setStringValue: @"fuzz"];

  // create with URI and prefix
  NSXMLNode *attr2 = [NSXMLNode attributeWithName: @"openSearch:foo"
                                      stringValue: @"baz2"];
  NSXMLElement *child2 = [NSXMLNode elementWithName: @"openSearch:chyld"];
  [child2 setStringValue: @"fuzz2"];

  // create with unqualified local name
  NSXMLElement *child3 = [NSXMLNode elementWithName: @"chyld"];
  [child3 setStringValue: @"fuzz3"];

  // create with a missing namespace URI that we'll never actually add,
  // so we can test searches based on our faked use of {uri}: as
  // a prefix for the local name
  NSXMLNode *attr4 = [NSXMLNode attributeWithName: @"zorgbot"
                                              URI: kGDataNamespaceBatch
                                      stringValue: @"gollyfum"];
  NSXMLElement *child4 = [NSXMLNode elementWithName: @"zorgtree"
                                                URI: kGDataNamespaceBatch];
  [child4 setStringValue: @"gollyfoo"];

  // create an element with a local namespace not defined in the parent level
  NSString *lonerURI = @"http://loner.ns";
  NSXMLElement *child5 = [NSXMLNode elementWithName: @"ln:loner"
                                                URI: lonerURI];
  NSXMLNode *ns5 = [NSXMLNode namespaceWithName: @"ln"
                                    stringValue: lonerURI];
  [child5 addNamespace: ns5];

  // add these to a parent element, along with namespaces
  NSXMLElement *parent = [NSXMLNode elementWithName: @"dad"];
  [parent setStringValue: @"buzz"];

  // atom is the default namespace
  NSXMLNode *nsAtom = [NSXMLNode namespaceWithName: @""
                                       stringValue: kGDataNamespaceAtom];
  [parent addNamespace: nsAtom];

  NSXMLNode *nsGD = [NSXMLNode namespaceWithName: @"gd"
                                     stringValue: kGDataNamespaceGData];
  [parent addNamespace: nsGD];

  NSXMLNode *nsOpenSearch = [NSXMLNode namespaceWithName: @"openSearch"
                                            stringValue: kGDataNamespaceOpenSearch];
  [parent addNamespace: nsOpenSearch];

  [parent addChild: child1];
  [parent addAttribute: attr1];
  [parent addChild: child2];
  [parent addAttribute: attr2];
  [parent addChild: child3];
  [parent addChild: child4];
  [parent addAttribute: attr4];
  [parent addChild: child5];

  // search for attr1 and child1 by qualified name, since they were
  // created by URI
  NSXMLNode *attr1Found = [parent attributeForName: @"gd:foo"];
  PASS_EQUAL([attr1Found stringValue], @"baz", "attribute gd:foo");

  NSArray *elements = [parent elementsForName: @"gd:chyld"];
  PASS((int)[elements count] == 1, "getting gd:chyld");
  if ([elements count] > 0)
    {
      NSXMLNode *child1Found = [elements objectAtIndex: 0];
      PASS_EQUAL([child1Found stringValue], @"fuzz", "child gd:chyld");
    }

  // search for attr2 and child2 by local name and URI, since they were
  // created by qualified names
  NSXMLNode *attr2Found = [parent attributeForLocalName: @"foo"
                                                    URI: kGDataNamespaceOpenSearch];
  PASS_EQUAL([attr2Found stringValue], @"baz2", "attribute openSearch:foo");

  NSArray *elements2 = [parent elementsForLocalName: @"chyld"
                                                URI: kGDataNamespaceOpenSearch];

  PASS((int)[elements2 count] == 1, "getting openSearch:chyld");
  if ([elements2 count] > 0)
    {
      NSXMLNode *child2Found = [elements2 objectAtIndex: 0];
      PASS_EQUAL([child2Found stringValue], @"fuzz2", "child openSearch:chyld");
    }

  // search for child3 by local name
  NSArray *elements3 = [parent elementsForName: @"chyld"];
  PASS((int)[elements3 count] == 1, "getting chyld");
  if ([elements3 count] > 0)
    {
      NSXMLNode *child3Found = [elements3 objectAtIndex: 0];
      PASS_EQUAL([child3Found stringValue], @"fuzz3", "child chyld");
    }

  // search for child3 by URI
  NSArray *elements3a = [parent elementsForLocalName: @"chyld"
                                                 URI: kGDataNamespaceAtom];
  PASS((int)[elements3a count] == 1, "getting chyld (URI");
  if ([elements3a count] > 0)
    {
      NSXMLNode *child3aFound = [elements3 objectAtIndex: 0];
      PASS_EQUAL([child3aFound stringValue], @"fuzz3", "child chyld");
    }

  // search for attr4 and child4 by local name and URI, since they were
  // created by URI and never bound to a prefix by a namespace declaration
  NSXMLNode *attr4Found = [parent attributeForLocalName: @"zorgbot"
                                                    URI: kGDataNamespaceBatch];
  PASS_EQUAL([attr4Found stringValue], @"gollyfum", "in test batch zorgbot");

  NSArray *elements4 = [parent elementsForLocalName: @"zorgtree"
                                                URI: kGDataNamespaceBatch];
  PASS((int)[elements4 count] == 1, "getting batch zorgtree");
  if ([elements4 count] > 0)
    {
      NSXMLNode *child4Found = [elements4 objectAtIndex: 0];
      PASS_EQUAL([child4Found stringValue], @"gollyfoo", "in test batch zorgtree");
    }

  // search for child 5 by local name and URI, since it has a locally-defined
  // namespace
  NSArray *elements5 = [parent elementsForLocalName: @"loner"
                                                URI: lonerURI];
  PASS((int)[elements5 count] == 1, "getting loner");

  // test output
  NSString *expectedXML = @"<dad xmlns=\"http://www.w3.org/2005/Atom\" "
    "xmlns:gd=\"http://schemas.google.com/g/2005\" "
    "xmlns:openSearch=\"http://a9.com/-/spec/opensearchrss/1.0/\" "
    "foo=\"baz\" openSearch:foo=\"baz2\" zorgbot=\"gollyfum\">"
    "buzz<chyld>fuzz</chyld><openSearch:chyld>fuzz2</openSearch:chyld>"
    "<chyld>fuzz3</chyld><zorgtree>gollyfoo</zorgtree>"
    "<ln:loner xmlns:ln=\"http://loner.ns\"></ln:loner></dad>";

  NSString *actualXML = [parent XMLString];
  testHopeful = YES;
  PASS_EQUAL(actualXML, expectedXML, "unexpected xml output");
  testHopeful = NO;

  [arp drain];
  arp = nil;
#endif
  END_SET("NSXMLNode namespace handling")
  return 0;
}
