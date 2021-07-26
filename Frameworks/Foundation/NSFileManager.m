/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Foundation/NSFileManager.h>
#import <Foundation/NSThread.h>
#import <Foundation/NSRaise.h>
#import <Foundation/NSNumber.h>
#import <Foundation/NSData.h>
#import <Foundation/NSArray.h>
#import <Foundation/NSConcreteDirectoryEnumerator.h>
#import <Foundation/NSBundle.h>
#import <Foundation/NSURL.h>
#import <Foundation/NSPathUtilities.h>

NSString * const NSFileType = @"NSFileType";
NSString * const NSFileTypeRegular = @"NSFileTypeRegular";
NSString * const NSFileTypeDirectory = @"NSFileTypeDirectory";
NSString * const NSFileTypeSymbolicLink = @"NSFileTypeSymbolicLink";
NSString * const NSFileTypeCharacterSpecial = @"NSFileTypeCharacterSpecial";
NSString * const NSFileTypeBlockSpecial = @"NSFileTypeBlockSpecial";
NSString * const NSFileTypeFIFO = @"NSFileTypeFIFO";
NSString * const NSFileTypeSocket = @"NSFileTypeSocket";
NSString * const NSFileTypeUnknown = @"NSFileTypeUnknown";

NSString * const NSFileSize = @"NSFileSize";
NSString * const NSFileCreationDate = @"NSFileCreationDate";
NSString * const NSFileModificationDate = @"NSFileModificationDate";
NSString * const NSFileOwnerAccountName = @"NSFileOwnerAccountName";
NSString * const NSFileGroupOwnerAccountName = @"NSFileGroupOwnerAccountName";

NSString * const NSFileReferenceCount = @"NSFileReferenceCount";
NSString * const NSFileIdentifier = @"NSFileIdentifier";
NSString * const NSFileDeviceIdentifier = @"NSFileDeviceIdentifier";
NSString * const NSFilePosixPermissions = @"NSFilePosixPermissions";
NSString * const NSFileHFSCreatorCode = @"NSFileHFSCreatorCode";
NSString * const NSFileHFSTypeCode = @"NSFileHFSTypeCode";

NSString * const NSFileSystemNumber=@"NSFileSystemNumber";
NSString * const NSFileSystemSize=@"NSFileSystemSize";
NSString * const NSFileSystemFreeSize=@"NSFileSystemFreeSize";

@implementation NSFileManager

+(NSFileManager *)defaultManager {
   return NSThreadSharedInstance(@"NSFileManager");
}

-delegate {
   NSUnimplementedMethod();
   return 0;
}
-(void)setDelegate:delegate {
   NSUnimplementedMethod();
}

-(NSDictionary *)attributesOfFileSystemForPath:(NSString *)path error:(NSError **)errorp {
   NSUnimplementedMethod();
   return 0;
}
-(NSDictionary *)attributesOfItemAtPath:(NSString *)path error:(NSError **)error {
   NSUnimplementedMethod();
   return 0;
}
-(BOOL)changeCurrentDirectoryPath:(NSString *)path {
   NSUnimplementedMethod();
   return 0;
}
-(NSArray *)componentsToDisplayForPath:(NSString *)path {
   NSUnimplementedMethod();
   return 0;
}
-(BOOL)contentsEqualAtPath:(NSString *)path1 andPath:(NSString *)path2 {
   NSUnimplementedMethod();
   return 0;
}
-(NSArray *)contentsOfDirectoryAtPath:(NSString *)path error:(NSError **)error {
   NSUnimplementedMethod();
   return 0;
}
-(BOOL)copyItemAtPath:(NSString *)fromPath toPath:(NSString *)toPath error:(NSError **)error {
   NSUnimplementedMethod();
   return 0;
}
-(NSString *)destinationOfSymbolicLinkAtPath:(NSString *)path error:(NSError **)error {
   NSInvalidAbstractInvocation();
   return 0;
}

-(NSString *)displayNameAtPath:(NSString *)path {
   NSBundle *bundle=[NSBundle bundleWithPath:path];
   NSString *name=nil;
   if(bundle) {
    NSDictionary *localizedInfo=[bundle localizedInfoDictionary];
    name=[localizedInfo objectForKey:@"CFBundleDisplayName"];
    if(!name)
     name=[localizedInfo objectForKey:@"CFBundleName"];
   }
   if(!name)
    name=[path lastPathComponent];
   return name;
}

-(NSDictionary *)fileSystemAttributesAtPath:(NSString *)path
{
	return [self attributesOfFileSystemForPath: path error: NULL];
}

-(BOOL)isDeletableFileAtPath:(NSString *)path {
   NSUnimplementedMethod();
   return 0;
}

-(BOOL)linkItemAtPath:(NSString *)fromPath toPath:(NSString *)toPath error:(NSError **)error {
   NSUnimplementedMethod();
   return 0;
}
-(BOOL)linkPath:(NSString *)source toPath:(NSString *)destination handler:handler {
   NSUnimplementedMethod();
   return 0;
}
-(BOOL)moveItemAtPath:(NSString *)fromPath toPath:(NSString *)toPath error:(NSError **)error {
   NSUnimplementedMethod();
   return 0;
}
-(BOOL)removeItemAtPath:(NSString *)path error:(NSError **)error {
   NSUnimplementedMethod();
   return 0;
}

-(BOOL)removeItemAtURL:(NSURL *)url error:(NSError **)error {
    // Default to the AtPath implementation
    return [self removeItemAtPath:[url path] error:error];
}

-(BOOL)moveItemAtURL:(NSURL *)url toURL:(NSURL *)dstURL error:(NSError **)error {
    // Default to the AtPath implementation
    return [self moveItemAtPath:[url path] toPath:[dstURL path] error:error];
}

-(BOOL)linkItemAtURL:(NSURL *)url toURL:(NSURL *)dstURL error:(NSError **)error {
    // Default to the AtPath implementation
    return [self linkItemAtPath:[url path] toPath:[dstURL path] error:error];
}

-(BOOL)copyItemAtURL:(NSURL *)url toURL:(NSURL *)dstURL error:(NSError **)error {
    // Default to the AtPath implementation
    return [self copyItemAtPath:[url path] toPath:[dstURL path] error:error];
}

-(BOOL)setAttributes:(NSDictionary *)attributes ofItemAtPath:(NSString *)path error:(NSError **)error {
   NSUnimplementedMethod();
   return 0;
}

-(NSString *)stringWithFileSystemRepresentation:(const char *)string length:(NSUInteger)length {
   NSUnimplementedMethod();
   return 0;
}

-(void)_subpathsAtPath:(NSString *)path basePath:(NSString*)basePath
paths:(NSMutableArray*)paths
{
	NSArray* files = [self directoryContentsAtPath:path];

	int x; for (x = 0; x < [files count]; x++)
	{
		[paths addObject:[basePath stringByAppendingPathComponent:[files objectAtIndex:x]]];
	}

	for (x = 0; x < [files count]; x++)
	{
		BOOL isDir = NO;
		NSString* newPath = [path stringByAppendingPathComponent:[files objectAtIndex:x]];
		[self fileExistsAtPath:newPath isDirectory:&isDir];
		if (isDir)
			[self _subpathsAtPath:newPath basePath:[basePath
stringByAppendingPathComponent:[files objectAtIndex:x]] paths:paths];
	}
}

-(NSArray *)subpathsAtPath:(NSString *)path {
	NSMutableArray *result=[NSMutableArray array];

	[self _subpathsAtPath:path basePath:@"" paths:result];
	return result;
}

-(NSArray *)subpathsOfDirectoryAtPath:(NSString *)path error:(NSError **)error {
   NSUnimplementedMethod();
   return 0;
}

-(NSData *)contentsAtPath:(NSString *)path {
   return [NSData dataWithContentsOfFile:path];
}

-(BOOL)createFileAtPath:(NSString *)path contents:(NSData *)data
             attributes:(NSDictionary *)attributes {
   NSInvalidAbstractInvocation();
   return NO;
}

-(NSArray *)directoryContentsAtPath:(NSString *)path {
   NSInvalidAbstractInvocation();
   return nil;
}

-(NSDirectoryEnumerator *)enumeratorAtPath:(NSString *)path {
    return [[[NSConcreteDirectoryEnumerator alloc] initWithPath: path] autorelease];
}

-(BOOL)createDirectoryAtPath:(NSString *)path attributes:(NSDictionary *)attributes {
   NSInvalidAbstractInvocation();
   return NO;
}

-(BOOL)createDirectoryAtPath:(NSString *)path withIntermediateDirectories:(BOOL)intermediates attributes:(NSDictionary *)attributes error:(NSError **)error {
	BOOL result = YES;
	BOOL isDirectory = NO;
	if (intermediates) {
		NSArray *components = [path pathComponents];
		int i, count = [components count];
		for (i=1; i<=count && result; i++) {
			NSString *makePath = [NSString pathWithComponents:[components subarrayWithRange:NSMakeRange(0,i)]];
			if (![self fileExistsAtPath:makePath isDirectory:&isDirectory]) {
				result = [self createDirectoryAtPath:makePath attributes:attributes];
			}
		}
	}
	else {
		result = [self createDirectoryAtPath:path attributes:attributes];
	}
	// TODO: fill error
	return result;
}

-(BOOL)createDirectoryAtURL:(NSURL *)url withIntermediateDirectories:(BOOL)intermediates attributes:(NSDictionary *)attributes error:(NSError **)error
{
    return [self createDirectoryAtPath:[url path] withIntermediateDirectories:intermediates attributes:attributes error:error];
}

-(BOOL)createSymbolicLinkAtPath:(NSString *)path pathContent:(NSString *)destination {
   NSInvalidAbstractInvocation();
   return NO;
}

-(BOOL)createSymbolicLinkAtPath:(NSString *)path withDestinationPath:(NSString *)destPath error:(NSError **)error {
   NSUnimplementedMethod();
   return 0;
}

-(NSString *)pathContentOfSymbolicLinkAtPath:(NSString *)path {
   NSInvalidAbstractInvocation();
   return NO;
}

-(BOOL)fileExistsAtPath:(NSString *)path {
   BOOL foo;
   return [self fileExistsAtPath:path isDirectory:&foo];
}

-(BOOL)fileExistsAtPath:(NSString *)path isDirectory:(BOOL *)isDirectory {
   NSInvalidAbstractInvocation();
   return NO;
}

-(BOOL)removeFileAtPath:(NSString *)path handler:handler {
   NSInvalidAbstractInvocation();
   return NO;
}

-(BOOL)movePath:(NSString *)src toPath:(NSString *)dest handler:handler {
   NSInvalidAbstractInvocation();
   return NO;
}

-(BOOL)copyPath:(NSString *)src toPath:(NSString *)dest handler:handler {
   NSInvalidAbstractInvocation();
   return NO;
}

-(NSString *)currentDirectoryPath {
   NSInvalidAbstractInvocation();
   return nil;
}

-(NSDictionary *)fileAttributesAtPath:(NSString *)path traverseLink:(BOOL)traverse {
   NSInvalidAbstractInvocation();
   return nil;
}

-(BOOL)isReadableFileAtPath:(NSString *)path {
   NSInvalidAbstractInvocation();
   return NO;
}

-(BOOL)isWritableFileAtPath:(NSString *)path {
   NSInvalidAbstractInvocation();
   return NO;
}

-(BOOL)isExecutableFileAtPath:(NSString *)path {
   NSInvalidAbstractInvocation();
   return NO;
}

-(BOOL)changeFileAttributes:(NSDictionary *)attributes atPath:(NSString *)path {
    return [self setAttributes:(NSDictionary *)attributes ofItemAtPath:(NSString *)path error:NULL];
}

-(const char *)fileSystemRepresentationWithPath:(NSString *)path {
   NSInvalidAbstractInvocation();
   return NULL;
}

-(const uint16_t *)fileSystemRepresentationWithPathW:(NSString *)path {
   NSInvalidAbstractInvocation();
   return NULL;
}


@end

@implementation NSDictionary(NSFileAttributes)

-(NSDate *)fileModificationDate {
   return [self objectForKey:NSFileModificationDate];
}

-(NSUInteger)filePosixPermissions {
   return [[self objectForKey:NSFilePosixPermissions] unsignedIntegerValue];
}

-(NSString *)fileOwnerAccountName {
   return [self objectForKey:NSFileOwnerAccountName];
}

-(NSString *)fileGroupOwnerAccountName {
   return [self objectForKey:NSFileGroupOwnerAccountName];
}

-(NSString *)fileType {
   return [self objectForKey:NSFileType];
}

-(uint64_t)fileSize {
   return [[self objectForKey:NSFileSize] unsignedLongLongValue];
}

@end
