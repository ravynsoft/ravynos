/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <Foundation/NSURLCache.h>
#import <Foundation/NSProcessInfo.h>
#import <Foundation/NSCachedURLResponse.h>
#import <Foundation/NSURLRequest.h>
#import <Foundation/NSString.h>
#import <Foundation/NSPathUtilities.h>
#import <Foundation/NSArray.h>
#import <Foundation/NSData.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSHTTPURLResponse.h>
#import <Foundation/NSRaise.h>
#import <Foundation/NSFileManager.h>
#import <Foundation/NSMutableSet.h>
#import <Foundation/NSSortDescriptor.h>
#import <Foundation/NSURL.h>
#import <CoreFoundation/CFUUID.h>

@interface NSHTTPURLResponse(private)
-initWithURL:(NSURL *)url statusCode:(NSInteger)statusCode headers:(NSDictionary *)headers;
@end

@implementation NSURLCache

static NSURLCache *shared=nil;

+(NSURLCache *)sharedURLCache {
   if(shared==nil){
    NSString *path=[NSSearchPathForDirectoriesInDomains(NSCachesDirectory,NSUserDomainMask,YES) lastObject];
    
    path=[path stringByAppendingPathComponent:@"NSURLCache"];
    
    [[NSFileManager defaultManager] createDirectoryAtPath:path withIntermediateDirectories:YES attributes:nil error:NULL];
        
    shared=[[NSURLCache alloc] initWithMemoryCapacity:1024*512 diskCapacity:1024*1024*20 diskPath:path];
}

   return shared;
}

+(void)setSharedURLCache:(NSURLCache *)cache {
   cache=[cache retain];
   [shared release];
   shared=cache;
}

-initWithMemoryCapacity:(NSUInteger)memoryCapacity diskCapacity:(NSUInteger)diskCapacity diskPath:(NSString *)diskPath {
   _path=[diskPath copy];
   _memoryCapacity=memoryCapacity;
   _diskCapacity=diskCapacity;
   _memoryCache=[[NSMutableDictionary alloc] init];
   _memoryAccess=[[NSMutableArray alloc] init];
   
   _diskCache=[[NSMutableDictionary alloc] initWithContentsOfFile:[_path stringByAppendingPathComponent:@"cache"]];

   if(_diskCache==nil)
    _diskCache=[[NSMutableDictionary alloc] init];

   return self;
}

-(void)dealloc {
   [_path release];
   [_memoryCache release];
   [_memoryAccess release];
   [_diskCache release];
   [super dealloc];
}

-(NSUInteger)memoryCapacity {
   return _memoryCapacity;
}

-(NSUInteger)diskCapacity {
   return _diskCapacity;
}

-(NSUInteger)currentDiskUsage {
   return _currentDiskUsage;
}

-(NSUInteger)currentMemoryUsage {
   return _currentMemoryUsage;
}

-(NSCachedURLResponse *)cachedResponseForRequest:(NSURLRequest *)request {
   NSCachedURLResponse *result=[_memoryCache objectForKey:[request URL]];
      
   if(result==nil){    
    NSMutableDictionary *properties=[_diskCache objectForKey:[[request URL] absoluteString]];
        
    if(properties!=nil){
     NSString     *dataPath=[[_path stringByAppendingPathComponent:[properties objectForKey:@"file"]] stringByAppendingPathExtension:@"data"];
     NSData       *data=[NSData dataWithContentsOfFile:dataPath];
     NSString     *headerPath=[[_path stringByAppendingPathComponent:[properties objectForKey:@"file"]] stringByAppendingPathExtension:@"headers"];
     NSDictionary *headers=[NSDictionary dictionaryWithContentsOfFile:headerPath];
     
     if(data!=nil && headers!=nil){
      NSHTTPURLResponse *response=[[[NSHTTPURLResponse alloc] initWithURL:[request URL] statusCode:200 headers:headers] autorelease];
     
      result=[[[NSCachedURLResponse alloc] initWithResponse:response data:data userInfo:nil storagePolicy:NSURLCacheStorageNotAllowed] autorelease];

      properties=[[properties mutableCopy] autorelease];
      [properties setObject:[NSString stringWithFormat:@"%f",[[NSDate date] timeIntervalSinceReferenceDate]] forKey:@"lastAccess"];
      
      [_diskCache setObject:properties forKey:[[request URL] absoluteString]];

      if(![_diskCache writeToFile:[_path stringByAppendingPathComponent:@"cache"] atomically:YES]){
}
     }
    }
   }

   return result;
}

-(void)setMemoryCapacity:(NSUInteger)memoryCapacity {
   _memoryCapacity=memoryCapacity;
}

-(void)setDiskCapacity:(NSUInteger)diskCapacity {
   _diskCapacity=diskCapacity;
}

-(NSString *)nextFileForDiskCache {
   CFUUIDRef uuid=CFUUIDCreate(NULL);
   NSString *result=[(NSString *)CFUUIDCreateString(NULL,uuid) autorelease];
   
   CFRelease(uuid);

   return result;
}

-(void)reduceDiskCacheForSize:(NSUInteger)chunk {
// just remove the excess entries from the cache, the removal of invalid files will purge them
   NSMutableArray *bySize=[[[_diskCache allValues] mutableCopy] autorelease];
   NSSortDescriptor *sort=[NSSortDescriptor sortDescriptorWithKey:@"lastAccess" ascending:NO];
   
   [bySize sortUsingDescriptors:[NSArray arrayWithObject:sort]];
   
   NSInteger i,count=[bySize count];
   NSInteger total=0;
   
   for(i=0;i<count;i++){
    NSDictionary *check=[bySize objectAtIndex:i];
    NSInteger     size=[[check objectForKey:@"size"] integerValue];
    
    if(total+size+chunk>_diskCapacity)
     break;
    
    total+=size;
   }
      
   for(;i<count;i++){
    NSDictionary *check=[bySize objectAtIndex:i];
    
    [_diskCache removeObjectForKey:[check objectForKey:@"url"]];
   }
   
   NSMutableSet *allValidFiles=[[[NSMutableSet alloc] init] autorelease];

   [allValidFiles addObject:@"cache"];
   
   for(NSString *key in _diskCache){
    NSDictionary *properties=[_diskCache objectForKey:key];
    
    [allValidFiles addObject:[[properties objectForKey:@"file"] stringByAppendingPathExtension:@"headers"]];
    [allValidFiles addObject:[[properties objectForKey:@"file"] stringByAppendingPathExtension:@"data"]];
   }
   
   NSArray *allFiles=[[NSFileManager defaultManager] contentsOfDirectoryAtPath:_path error:NULL];
   
   for(NSString *check in allFiles){
   
    if(![allValidFiles containsObject:check]){
     NSString *fullPath=[_path stringByAppendingPathComponent:check];
     
     [[NSFileManager defaultManager] removeItemAtPath:fullPath error:NULL];
    }
   }
   
}

-(void)storeDiskCachedResponse:(NSCachedURLResponse *)cachedResponse forRequest:(NSURLRequest *)request {
   NSMutableDictionary  *properties=[NSMutableDictionary dictionary];
   NSURLResponse *response=[cachedResponse response];
   NSData        *data=[cachedResponse data];
   
   [self reduceDiskCacheForSize:[data length]];
   
   [properties setObject:[NSString stringWithFormat:@"%d",[data length]] forKey:@"size"];
   NSString *file=[self nextFileForDiskCache];
   [properties setObject:file forKey:@"file"];
   [properties setObject:[NSString stringWithFormat:@"%f",[[NSDate date] timeIntervalSinceReferenceDate]] forKey:@"lastAccess"];
   // duplicative but makes cache purging simpler
   [properties setObject:[[request URL] absoluteString] forKey:@"url"];
      
   NSString *headersPath=[[_path stringByAppendingPathComponent:file] stringByAppendingPathExtension:@"headers"];
   NSString *dataPath=[[_path stringByAppendingPathComponent:file] stringByAppendingPathExtension:@"data"];
   
   if(![[(NSHTTPURLResponse *)response allHeaderFields] writeToFile:headersPath atomically:YES]){
    return;
   }
    
   if(![data writeToFile:dataPath atomically:YES]){
    return;
   }
  
   [_diskCache setObject:properties forKey:[[request URL] absoluteString]];
   
   if(![_diskCache writeToFile:[_path stringByAppendingPathComponent:@"cache"] atomically:YES]){
   }
}

-(void)storeCachedResponse:(NSCachedURLResponse *)response forRequest:(NSURLRequest *)request {
   switch([response storagePolicy]){
   
    case NSURLCacheStorageAllowed:
      [self storeDiskCachedResponse:response forRequest:request];
      break;
     
    case NSURLCacheStorageAllowedInMemoryOnly:
     [_memoryCache setObject:response forKey:[request URL]];
     break;
     
    case NSURLCacheStorageNotAllowed:
     break;
}

}

-(void)removeAllCachedResponses {
   NSUnimplementedMethod();
}

-(void)removeCachedResponseForRequest:(NSURLRequest *)request {
   [_memoryCache removeObjectForKey:[request URL]];
   
   [_diskCache removeObjectForKey:[[request URL] absoluteString]];
   
   if(![_diskCache writeToFile:[_path stringByAppendingPathComponent:@"cache"] atomically:YES]){
}
}

@end
