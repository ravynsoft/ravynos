/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <Foundation/NSObject.h>

@class NSURLRequest, NSURLResponse, NSURLAuthenticationChallenge, NSData, NSError, NSURLConnection, NSOutputStream;

@interface NSURLDownload : NSObject {
    NSURLRequest *_request;
    id _delegate;
    BOOL _deletesOnFailure;
    NSString *_path;
    BOOL _allowOverwrite;
    NSURLConnection *_connection;
    NSOutputStream *_fileStream;
}

+ (BOOL)canResumeDownloadDecodedWithEncodingMIMEType:(NSString *)mimeType;

- initWithRequest:(NSURLRequest *)requst delegate:delegate;
- initWithResumeData:(NSData *)data delegate:delegate path:(NSString *)path;

- (NSURLRequest *)request;
- (NSData *)resumeData;

- (BOOL)deletesFileUponFailure;

- (void)setDeletesFileUponFailure:(BOOL)flag;
- (void)setDestination:(NSString *)path allowOverwrite:(BOOL)allowOverwrite;

- (void)cancel;

@end

@interface NSObject (NSURLDownloadDelegate)
- (void)downloadDidBegin:(NSURLDownload *)download;
- (NSURLRequest *)download:(NSURLDownload *)download willSendRequest:(NSURLRequest *)request redirectResponse:(NSURLResponse *)redirect;
- (void)download:(NSURLDownload *)download didReceiveAuthenticationChallenge:(NSURLAuthenticationChallenge *)authChallenge;
- (void)download:(NSURLDownload *)download didCancelAuthenticationChallenge:(NSURLAuthenticationChallenge *)authChallenge;
- (void)download:(NSURLDownload *)download didReceiveResponse:(NSURLResponse *)response;
- (void)download:(NSURLDownload *)download didReceiveDataOfLength:(NSUInteger)length;

- (void)download:(NSURLDownload *)download didFailWithError:(NSError *)error;
- (void)downloadDidFinish:(NSURLDownload *)download;

- (void)download:(NSURLDownload *)download decideDestinationWithSuggestedFilename:(NSString *)suggested;
- (void)download:(NSURLDownload *)download didCreateDestination:(NSString *)destPath;
- (BOOL)download:(NSURLDownload *)download shouldDecodeSourceDataOfMIMEType:(NSString *)mimeType;

- (void)download:(NSURLDownload *)download willResumeWithResponse:(NSURLResponse *)response fromByte:(long long)position;

- (void)download:(NSURLDownload *)download didReceiveDataOfLength:(NSUInteger)length;

@end
