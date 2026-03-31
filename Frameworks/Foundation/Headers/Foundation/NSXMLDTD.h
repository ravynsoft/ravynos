/* Copyright (c) 2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Foundation/NSXMLNode.h>

@class NSXMLDTDNode, NSURL, NSData, NSMutableArray;

@interface NSXMLDTD : NSXMLNode {
    NSString *_publicID;
    NSString *_systemID;
}

+ (NSXMLDTDNode *)predefinedEntityDeclarationForName:(NSString *)name;

- initWithData:(NSData *)data options:(NSUInteger)options error:(NSError **)error;
- initWithContentsOfURL:(NSURL *)url options:(NSUInteger)options error:(NSError **)error;

- (NSString *)publicID;
- (NSString *)systemID;

- (NSXMLDTDNode *)attributeDeclarationForName:(NSString *)attributeName elementName:(NSString *)elementName;
- (NSXMLDTDNode *)elementDeclarationForName:(NSString *)name;
- (NSXMLDTDNode *)entityDeclarationForName:(NSString *)name;
- (NSXMLDTDNode *)notationDeclarationForName:(NSString *)name;

- (void)setPublicID:(NSString *)publicID;
- (void)setSystemID:(NSString *)systemID;

- (void)setChildren:(NSArray *)children;

- (void)addChild:(NSXMLNode *)node;
- (void)insertChild:(NSXMLNode *)child atIndex:(NSUInteger)index;
- (void)insertChildren:(NSArray *)children atIndex:(NSUInteger)index;
- (void)removeChildAtIndex:(NSUInteger)index;
- (void)replaceChildAtIndex:(NSUInteger)index withNode:(NSXMLNode *)node;

@end
