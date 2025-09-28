/* Copyright (c) 2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Foundation/NSXMLNode.h>

@class NSXMLNode, NSArray, NSError, NSDictionary, NSMutableDictionary, NSMutableArray;

@interface NSXMLElement : NSXMLNode <NSCopying> {
    NSMutableDictionary *_attributes;
    NSMutableDictionary *_namespaces;
}

- initWithName:(NSString *)name;
- initWithName:(NSString *)name stringValue:(NSString *)string;
- initWithName:(NSString *)name URI:(NSString *)uri;
- initWithXMLString:(NSString *)xml error:(NSError **)error;

- (NSArray *)attributes;
- (NSXMLNode *)attributeForLocalName:(NSString *)name URI:(NSString *)uri;
- (NSXMLNode *)attributeForName:(NSString *)name;
- (NSArray *)elementsForLocalName:(NSString *)localName URI:(NSString *)uri;
- (NSArray *)elementsForName:(NSString *)name;
- (NSArray *)namespaces;
- (NSXMLNode *)namespaceForPrefix:(NSString *)prefix;

- (void)setAttributes:(NSArray *)attributes;
- (void)setAttributesAsDictionary:(NSDictionary *)attributes;
- (void)setChildren:(NSArray *)children;
- (void)setNamespaces:(NSArray *)namespaces;

- (void)addChild:(NSXMLNode *)child;
- (void)insertChild:(NSXMLNode *)child atIndex:(NSUInteger)index;
- (void)insertChildren:(NSArray *)children atIndex:(NSUInteger)index;
- (void)removeChildAtIndex:(NSUInteger)index;
- (void)replaceChildAtIndex:(NSUInteger)index withNode:(NSXMLNode *)node;

- (void)addAttribute:(NSXMLNode *)attribute;
- (void)removeAttributeForName:(NSString *)name;

- (void)addNamespace:(NSXMLNode *)aNamespace;
- (void)removeNamespaceForPrefix:(NSString *)prefix;

- (void)resolveNamespaceForName:(NSString *)name;
- (void)resolvePrefixForNamespaceURI:(NSString *)uri;

- (void)normalizeAdjacentTextNodesPreservingCDATA:(BOOL)preserve;

@end
