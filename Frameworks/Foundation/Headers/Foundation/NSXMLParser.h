/* Copyright (c) 2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Foundation/NSObject.h>
#import <Foundation/NSString.h>
#import <Foundation/NSHashTable.h>

@class NSURL, NSData, NSError, NSXMLParser, NSDictionary, NSMutableArray, NSMutableDictionary;

@protocol NSXMLParserDelegate

- (void)parserDidStartDocument:(NSXMLParser *)parser;
- (void)parserDidEndDocument:(NSXMLParser *)parser;

- (void)parser:(NSXMLParser *)parser foundElementDeclarationWithName:(NSString *)elementName model:(NSString *)model;
- (void)parser:(NSXMLParser *)parser didStartElement:(NSString *)elementName namespaceURI:(NSString *)namespaceURI qualifiedName:(NSString *)qualifiedName attributes:(NSDictionary *)attributes;
- (void)parser:(NSXMLParser *)parser didEndElement:(NSString *)elementName namespaceURI:(NSString *)namespaceURI qualifiedName:(NSString *)qualifiedName;

- (void)parser:(NSXMLParser *)parser foundAttributeDeclarationWithName:(NSString *)attributeName forElement:(NSString *)elementName type:(NSString *)type defaultValue:(NSString *)defaultValue;

- (void)parser:(NSXMLParser *)parser didStartMappingPrefix:(NSString *)prefix toURI:(NSString *)uri;
- (void)parser:(NSXMLParser *)parser didEndMappingPrefix:(NSString *)prefix;

- (void)parser:(NSXMLParser *)parser foundCDATA:(NSData *)cdata;
- (void)parser:(NSXMLParser *)parser foundCharacters:(NSString *)string;
- (void)parser:(NSXMLParser *)parser foundComment:(NSString *)comment;
- (void)parser:(NSXMLParser *)parser foundIgnorableWhitespace:(NSString *)whitespace;

- (void)parser:(NSXMLParser *)parser foundExternalEntityDeclarationWithName:(NSString *)entityName publicID:(NSString *)publicID systemID:(NSString *)systemID;
- (void)parser:(NSXMLParser *)parser foundInternalEntityDeclarationWithName:(NSString *)entityName value:(NSString *)value;
- (void)parser:(NSXMLParser *)parser foundNotationDeclarationWithName:(NSString *)name publicID:(NSString *)publicID systemID:(NSString *)systemID;
- (void)parser:(NSXMLParser *)parser foundProcessingInstructionWithTarget:(NSString *)target data:(NSString *)data;
- (void)parser:(NSXMLParser *)parser foundUnparsedEntityDeclarationWithName:(NSString *)name publicID:(NSString *)publicID systemID:(NSString *)systemID notationName:(NSString *)notationName;
- (void)parser:(NSXMLParser *)parser parseErrorOccurred:(NSError *)parseError;
- (NSData *)parser:(NSXMLParser *)parser resolveExternalEntityName:(NSString *)entityName systemID:(NSString *)systemID;
- (void)parser:(NSXMLParser *)parser validationErrorOccurred:(NSError *)validationError;

@end

@interface NSXMLParser : NSObject {
    NSData *_data;
    id _delegate;
    BOOL _shouldProcessNamespaces;
    BOOL _shouldReportNamespacePrefixes;
    BOOL _shouldResolveExternalEntities;
    NSError *_parserError;
    NSString *_systemID;
    NSString *_publicID;
    NSInteger _columnNumber;
    NSInteger _lineNumber;

    // parsing state
    const uint8_t *_bytes;
    NSUInteger _length;
    NSRange _range;

    NSMutableDictionary *_entityRefContents;

    int _state;
    unichar _charRef;
    NSMutableArray *_elementNameStack;
    NSString *_currentAttributeName;
    NSMutableDictionary *_currentAttributes;
}

- (NSXMLParser *)initWithData:(NSData *)data;
- (NSXMLParser *)initWithContentsofURL:(NSURL *)url;

- (id)delegate;
- (BOOL)shouldProcessNamespaces;
- (BOOL)shouldReportNamespacePrefixes;
- (BOOL)shouldResolveExternalEntities;

- (void)setDelegate:delegate;
- (void)setShouldProcessNamespaces:(BOOL)flag;
- (void)setShouldReportNamespacePrefixes:(BOOL)flag;
- (void)setShouldResolveExternalEntities:(BOOL)flag;

- (BOOL)parse;
- (void)abortParsing;
- (NSError *)parserError;

- (NSString *)systemID;
- (NSString *)publicID;
- (NSInteger)columnNumber;
- (NSInteger)lineNumber;

@end
