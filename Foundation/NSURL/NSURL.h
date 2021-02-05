/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <Foundation/NSObject.h>

@class NSURLHandle, NSNumber, NSData, NSArray;

FOUNDATION_EXPORT NSString *const NSURLFileScheme;

@interface NSURL : NSObject <NSCopying, NSCoding> {
    NSURL *_baseURL;
    NSString *_string;
    NSString *_scheme;
    NSString *_host;
    NSNumber *_port;
    NSString *_user;
    NSString *_password;
    NSString *_path;
    NSString *_parameter;
    NSString *_query;
    NSString *_fragment;
}

- initWithScheme:(NSString *)scheme host:(NSString *)host path:(NSString *)path;
- initFileURLWithPath:(NSString *)path;
- initWithString:(NSString *)string;
- initWithString:(NSString *)string relativeToURL:(NSURL *)parent;

+ fileURLWithPath:(NSString *)path;
+ URLWithString:(NSString *)string;
+ URLWithString:(NSString *)string relativeToURL:(NSURL *)parent;

- (NSString *)absoluteString;
- (NSString *)parameterString;
- propertyForKey:(NSString *)key;

- (NSString *)scheme;
- (NSString *)host;
- (NSString *)user;
- (NSString *)password;
- (NSString *)fragment;
- (NSString *)path;
- (NSNumber *)port;
- (NSString *)query;
- (NSString *)relativePath;
- (NSString *)relativeString;
- (NSString *)resourceSpecifier;

- (BOOL)isFileURL;

- (NSURL *)standardizedURL;
- (NSURL *)absoluteURL;
- (NSURL *)baseURL;

- (NSURL *)URLByAppendingPathComponent:(NSString *)pathComponent;
- (NSURL *)URLByAppendingPathExtension:(NSString *)pathExtension;
- (NSURL *)URLByDeletingLastPathComponent;
- (NSURL *)URLByDeletingPathExtension;
- (NSString *)lastPathComponent;
- (NSString *)pathExtension;

- (BOOL)setProperty:property forKey:(NSString *)key;

- (BOOL)setResourceData:(NSData *)data;

- (NSData *)resourceDataUsingCache:(BOOL)useCache;
- (NSURLHandle *)URLHandleUsingCache:(BOOL)useCache;
- (void)loadResourceDataNotifyingClient:client usingCache:(BOOL)useCache;

@end

@interface NSURL (NSURLPathUtilities)

+ (NSURL *)fileURLWithPathComponents:(NSArray *)components;
- (NSArray *)pathComponents;
- (NSURL *)URLByAppendingPathComponent:(NSString *)pathComponent isDirectory:(BOOL)isDirectory;
- (NSURL *)URLByAppendingPathExtension:(NSString *)pathExtension;
- (NSURL *)URLByDeletingPathExtension;

- (NSURL *)URLByStandardizingPath;
- (NSURL *)URLByResolvingSymlinksInPath;

@end
