/* Copyright (c) 2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Foundation/NSObject.h>
#import <Foundation/NSXMLNodeOptions.h>

@class NSArray, NSError, NSDictionary, NSXMLElement, NSXMLNode, NSXMLDocument, NSMutableArray;

typedef enum {
    NSXMLInvalidKind,
    NSXMLDocumentKind,
    NSXMLElementKind,
    NSXMLAttributeKind,
    NSXMLNamespaceKind,
    NSXMLProcessingInstructionKind,
    NSXMLCommentKind,
    NSXMLTextKind,
    NSXMLDTDKind,
    NSXMLEntityDeclarationKind,
    NSXMLAttributeDeclarationKind,
    NSXMLElementDeclarationKind,
    NSXMLNotationDeclarationKind
} NSXMLNodeKind;

enum {
    NSXMLNodeOptionsNone,
    NSXMLNodeIsCDATA,
    NSXMLNodeExpandEmptyElement,
    NSXMLNodeCompactEmptyElement,
    NSXMLNodeUseSingleQuotes,
    NSXMLNodeUseDoubleQuotes,
    NSXMLNodePrettyPrint,
    NSXMLNodePreserveNamespaceOrder,
    NSXMLNodePreserveAttributeOrder,
    NSXMLNodePreserveEntities,
    NSXMLNodePreserveCharacterReferences,
    NSXMLNodePreservePrefixes,
    NSXMLNodePreserveCDATA,
    NSXMLNodePreserveWhitespace,
    NSXMLNodePreserveEmptyElements,
    NSXMLNodePreserveQuotes,
    NSXMLNodePreserveDTD,
    NSXMLNodePreserveAll,
};

@interface NSXMLNode : NSObject <NSCopying> {
    NSXMLNode *_parent;
    NSMutableArray *_children;
    NSUInteger _index;
    NSXMLNodeKind _kind;
    NSUInteger _options;
    NSString *_name;
    id _value;
}

+ document;
+ documentWithRootElement:(NSXMLElement *)element;

+ elementWithName:(NSString *)name;
+ elementWithName:(NSString *)name children:(NSArray *)children attributes:(NSArray *)attributes;
+ elementWithName:(NSString *)name stringValue:(NSString *)string;

+ attributeWithName:(NSString *)name stringValue:(NSString *)string;

+ commentWithStringValue:(NSString *)string;
+ textWithStringValue:(NSString *)string;
+ processingInstructionWithName:(NSString *)name stringValue:(NSString *)string;

+ DTDNodeWithXMLString:(NSString *)string;
+ namespaceWithName:(NSString *)name stringValue:(NSString *)string;
+ (NSXMLNode *)predefinedNamespaceForPrefix:(NSString *)prefix;

+ (NSString *)prefixForName:(NSString *)name;
+ (NSString *)localNameForName:(NSString *)name;

- initWithKind:(NSXMLNodeKind)kind;
- initWithKind:(NSXMLNodeKind)kind options:(NSUInteger)options;

- (NSUInteger)index;
- (NSXMLNodeKind)kind;
- (NSUInteger)level;
- (NSString *)localName;
- (NSString *)name;
- (NSXMLNode *)nextNode;
- (NSXMLNode *)nextSibling;
- (NSString *)stringValue;
- (NSString *)URI;
- objectValue;
- (NSXMLNode *)parent;
- (NSString *)prefix;
- (NSXMLNode *)previousNode;
- (NSXMLNode *)previousSibling;
- (NSXMLDocument *)rootDocument;

- (NSUInteger)childCount;
- (NSArray *)children;
- (NSXMLNode *)childAtIndex:(NSUInteger)index;

- (void)setName:(NSString *)name;
- (void)setObjectValue:object;
- (void)setStringValue:(NSString *)string;
- (void)setStringValue:(NSString *)string resolvingEntities:(BOOL)resolveEntities;

- (void)detach;
- (NSArray *)nodesForXPath:(NSString *)xpath error:(NSError **)error;
- (NSArray *)objectsForXQuery:(NSString *)xquery constants:(NSDictionary *)constants error:(NSError **)error;
- (NSArray *)objectsForXQuery:(NSString *)xquery error:(NSError **)error;
- (NSString *)XMLString;
- (NSString *)XMLStringWithOptions:(NSUInteger)options;
- (NSString *)XPath;
- (NSString *)canonicalXMLStringPreservingComments:(BOOL)comments;

@end
