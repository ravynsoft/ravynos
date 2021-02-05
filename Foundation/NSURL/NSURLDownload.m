/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <Foundation/NSURLDownload.h>
#import <Foundation/NSURLRequest.h>
#import <Foundation/NSURLConnection.h>
#import <Foundation/NSRaise.h>
#import <Foundation/NSRunLoop.h>
#import <Foundation/NSFileManager.h>
#import <Foundation/NSPathUtilities.h>
#import <Foundation/NSOutputStream.h>
#import <Foundation/NSData.h>

@implementation NSURLDownload

+(BOOL)canResumeDownloadDecodedWithEncodingMIMEType:(NSString *)mimeType {
   return YES;
}

-initWithRequest:(NSURLRequest *)request delegate:delegate {
   _request=[request copy];
   _delegate=delegate;
   _connection=[[NSURLConnection alloc] initWithRequest:_request delegate:self startImmediately:YES];
   return self;
}

-initWithResumeData:(NSData *)data delegate:delegate path:(NSString *)path {
   NSUnimplementedMethod();
   return NO;
}

-(NSURLRequest *)request {
   return _request;
}

-(NSData *)resumeData {
   NSUnimplementedMethod();
   return NO;
}

-(BOOL)deletesFileUponFailure {
   return _deletesOnFailure;
}

-(void)setDeletesFileUponFailure:(BOOL)flag {
   _deletesOnFailure=flag;
}

-(void)setDestination:(NSString *)path allowOverwrite:(BOOL)allowOverwrite {
   _path=[path copy];
   _allowOverwrite=allowOverwrite;
}

-(void)cancel {
   [_connection cancel];
}

-(void)connection:(NSURLConnection *)connection didFailWithError:(NSError *)error {
   if([_delegate respondsToSelector:@selector(download:didFailWithError:)])
    [_delegate download:self didFailWithError:error];
}

-(void)connection:(NSURLConnection *)connection didReceiveAuthenticationChallenge:(NSURLAuthenticationChallenge *)challenge {
   if([_delegate respondsToSelector:@selector(download:didReceiveAuthenticationChallenge:)])
    [_delegate download:self didReceiveAuthenticationChallenge:challenge];
}

-(void)connection:(NSURLConnection *)connection didCancelAuthenticationChallenge:(NSURLAuthenticationChallenge *)challenge {
   if([_delegate respondsToSelector:@selector(download:didCancelAuthenticationChallenge:)])
    [_delegate download:self didCancelAuthenticationChallenge:challenge];
}

-(void)_createFileStreamIfNeeded {
   if(_fileStream!=nil)
    return;

   NSString *check=_path;
   
   if(!_allowOverwrite){
    if([[NSFileManager defaultManager] fileExistsAtPath:check]){
     NSString *tryThis;
     NSInteger i;
     
     for(i=0;;i++){
      tryThis=[check stringByDeletingPathExtension];
      tryThis=[tryThis stringByAppendingFormat:@"-%d",i];
      tryThis=[tryThis stringByAppendingPathExtension:[check pathExtension]];
      
      if(![[NSFileManager defaultManager] fileExistsAtPath:tryThis]){
       check=tryThis;
       break;
      }
     }
    }
   }

   _fileStream=[[NSOutputStream alloc] initToFileAtPath:check append:NO];
   
   if([_delegate respondsToSelector:@selector(download:didCreateDestination:)])
    [_delegate download:self didCreateDestination:check];
}

-(void)connection:(NSURLConnection *)connection didReceiveData:(NSData *)data {
   [self _createFileStreamIfNeeded];
   [_fileStream write:[data bytes] maxLength:[data length]];
   
   if([_delegate respondsToSelector:@selector(download:didReceiveDataOfLength:)])
    [_delegate download:self didReceiveDataOfLength:[data length]];
}

-(void)connection:(NSURLConnection *)connection didReceiveResponse:(NSURLResponse *)response {
   if([_delegate respondsToSelector:@selector(download:didReceiveResponse:)])
    [_delegate download:self didReceiveResponse:response];
}

-(NSCachedURLResponse *)connection:(NSURLConnection *)connection willCacheResponse:(NSCachedURLResponse *)response {
   return nil;
}

-(NSURLRequest *)connection:(NSURLConnection *)connection willSendRequest:(NSURLRequest *)request redirectResponse:(NSURLResponse *)response {
   if([_delegate respondsToSelector:@selector(download:willSendRequest:redirectResponse:)])
    return [_delegate download:self willSendRequest:request redirectResponse:response];
   
   return request;
}

-(void)connectionDidFinishLoading:(NSURLConnection *)connection {
   if([_delegate respondsToSelector:@selector(downloadDidFinish:)])
    [_delegate downloadDidFinish:self];
}


@end
