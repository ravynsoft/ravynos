/* Copyright (c) 2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Foundation/NSXMLNode.h>
#import <Foundation/NSXMLNodeOptions.h>

@class NSXMLDTD, NSURL, NSData, NSMutableArray;

enum {
    NSXMLDocumentXMLKind = 0,
    NSXMLDocumentXHTMLKind = 1,
    NSXMLDocumentHTMLKind = 2,
    NSXMLDocumentTextKind = 3,
};

typedef NSUInteger NSXMLDocumentContentKind;

@interface NSXMLDocument : NSXMLNode {
    NSXMLDocumentContentKind _contentKind;
    NSString *_version;
    NSString *_characterEncoding;
    NSString *_mimeType;
    BOOL _isStandalone;

    NSXMLElement *_rootElement;
    NSXMLDTD *_dtd;
    NSString *_uri;

    // parsing state, should be moved out
    NSMutableArray *_elementStack;
}

+ (Class)replacementClassForClass:(Class)aClass;

- initWithRootElement:(NSXMLElement *)element;
- initWithXMLString:(NSString *)string options:(NSUInteger)options error:(NSError **)error;
- initWithData:(NSData *)data options:(NSUInteger)options error:(NSError **)error;
- initWithContentsOfURL:(NSURL *)url options:(NSUInteger)options error:(NSError **)error;

- (NSXMLDocumentContentKind)documentContentKind;
- (NSString *)version;
- (NSString *)characterEncoding;
- (NSString *)MIMEType;
- (BOOL)isStandalone;
- (NSXMLElement *)rootElement;
- (NSXMLDTD *)DTD;
- (NSString *)URI;

- (void)setDocumentContentKind:(NSXMLDocumentContentKind)kind;
- (void)setCharacterEncoding:(NSString *)encoding;
- (void)setVersion:(NSString *)version;
- (void)setMIMEType:(NSString *)mimeType;
- (void)setStandalone:(BOOL)flag;
- (void)setRootElement:(NSXMLNode *)element;
- (void)setDTD:(NSXMLDTD *)dtd;
- (void)setURI:(NSString *)uri;

- (void)setChildren:(NSArray *)children;
- (void)addChild:(NSXMLNode *)child;
- (void)insertChild:(NSXMLNode *)child atIndex:(NSUInteger)index;
- (void)insertChildren:(NSArray *)children atIndex:(NSUInteger)index;
- (void)removeChildAtIndex:(NSUInteger)index;
- (void)replaceChildAtIndex:(NSUInteger)index withNode:(NSXMLNode *)node;

- (BOOL)validateAndReturnError:(NSError **)error;

- (NSData *)XMLData;
- (NSData *)XMLDataWithOptions:(NSUInteger)options;

- objectByApplyingXSLT:(NSData *)xslt arguments:(NSDictionary *)arguments error:(NSError *)error;
- objectByApplyingXSLTAtURL:(NSURL *)url arguments:(NSDictionary *)arguments error:(NSError *)error;
- objectByApplyingXSLTString:(NSString *)string arguments:(NSDictionary *)arguments error:(NSError *)error;

@end
