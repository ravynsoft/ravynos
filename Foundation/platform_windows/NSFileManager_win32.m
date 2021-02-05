/* Copyright (c) 2006-2007 Christopher J. W. Lloyd
                 2009 Markus Hitter <mah@jump-ing.de>

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#ifdef WINDOWS

#import <Foundation/NSFileManager_win32.h>
#import <Foundation/NSArray.h>
#import <Foundation/NSData.h>
#import <Foundation/NSDate.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSNumber.h>
#import <Foundation/NSPathUtilities.h>
#import <Foundation/NSThread.h>
#import <Foundation/NSString_cString.h>
#import <Foundation/NSRaise.h>
#import <Foundation/NSThread-Private.h>

#import <Foundation/NSPlatform_win32.h>
#import <Foundation/NSString_win32.h>

#include <sys/stat.h>
#include <windows.h>
#include <Sddl.h>
#include <shlobj.h>
#include <objbase.h>

#pragma mark -
#pragma mark Utility Methods

// Adapted from http://pastebin.com/bEkJVQx9
static int CopyDirectoryW(CONST WCHAR* cszFrom, CONST WCHAR* cszTo) {
	int success = CreateDirectoryW(cszTo, NULL) != 0;
	if (success) {
		WIN32_FIND_DATAW FindFileData;
		HANDLE hFindFile;
		
		WCHAR cszDirectoryFindPattern[1024] = {0};
		swprintf(cszDirectoryFindPattern, L"\\\\?\\%s\\*", cszFrom);
		
		if ((hFindFile = FindFirstFileW(cszDirectoryFindPattern, &FindFileData)) != INVALID_HANDLE_VALUE) {
			do {
				if (*FindFileData.cFileName == '.')
					continue;
				
				WCHAR cszFileOrDirectoryFrom[1024] = {0};
				WCHAR cszFileOrDirectoryTo[1024] = {0};
				swprintf(cszFileOrDirectoryFrom, L"%s\\%s", cszFrom, FindFileData.cFileName);
				swprintf(cszFileOrDirectoryTo, L"%s\\%s", cszTo, FindFileData.cFileName);
				
				if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
					success = CopyDirectoryW(cszFileOrDirectoryFrom, cszFileOrDirectoryTo);
				}
				else {
					success = CopyFileW(cszFileOrDirectoryFrom, cszFileOrDirectoryTo, YES);
				}
				if (success == 0) {
					break;
				}
			} while(FindNextFileW(hFindFile, &FindFileData));
			FindClose(hFindFile);
		}
	}
	return success;
}

static NSString *TranslatePath( NSString *path )
{
	NSInteger i,length=[path length],resultLength=0;
	unichar    buffer[length],result[length];
    
    [path getCharacters:buffer];
	
	for(i=0;i<length;i++){
		
		if(i==0){
			if(buffer[0]=='/' || buffer[0]=='\\') {
				// A leading "//" is actually legal for some network paths so don't skip them
				if (length < 2 || buffer[1] != buffer[0])
					continue;
			}
        }
		
		if(resultLength==1 && buffer[i]=='|'){
			result[resultLength++]=':';
			continue;
		}
		
		if(buffer[i]=='/')
			result[resultLength++]='\\';
		else
			result[resultLength++]=buffer[i];
    }
    
	return [NSString stringWithCharacters:result length:resultLength];
}

static NSString *DriveLetterInPath(NSString* path)
{
	NSArray* components = [path componentsSeparatedByString: @":"];
	if ([components count] > 0) {
		NSString* driveLetter = [NSString stringWithFormat: @"%@:\\", [components objectAtIndex: 0]];
		return driveLetter;
	}
	return nil;
}

static NSError *NSErrorForGetLastErrorCode(DWORD code)
{
	NSString *localizedDescription=@"NSErrorForGetLastError localizedDescription";
	unichar  *message;
	
	FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS,NULL,code,MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),(LPWSTR) &message,0, NULL );
	localizedDescription=NSStringFromNullTerminatedUnicode(message);
	
	LocalFree(message);
	
	return [NSError errorWithDomain:NSWin32ErrorDomain code:code userInfo:[NSDictionary dictionaryWithObject:localizedDescription forKey:NSLocalizedDescriptionKey]];
}

static NSError *NSErrorForGetLastError()
{
	return NSErrorForGetLastErrorCode(GetLastError());
}

static BOOL _NSCreateDirectory(NSString *path,NSError **errorp)
{
	if(CreateDirectoryW([path fileSystemRepresentationW],NULL)==0){
		DWORD error=GetLastError();
		
		if(error!=ERROR_ALREADY_EXISTS){
			if(errorp!=nil)
				*errorp=NSErrorForGetLastError();
			
			return NO;
		}
	}
	
	return YES;
}

#pragma mark -
@implementation NSFileManager(windows)

+allocWithZone:(NSZone *)zone {
   return NSAllocateObject([NSFileManager_win32 class],0,NULL);
}

@end

#pragma mark -
@implementation NSFileManager_win32

// we dont want to use fileExists... because it chases links 
-(BOOL)_isDirectory:(NSString *)path
{
	if(path == nil) {
		return NO;
	}
	DWORD attributes=GetFileAttributesW([path fileSystemRepresentationW]);
	
	if(attributes==0xFFFFFFFF)
		return NO;
	
	return (attributes&FILE_ATTRIBUTE_DIRECTORY)?YES:NO;
}

#pragma mark -
#pragma mark Moving an item

-(BOOL)moveItemAtPath:(NSString *)fromPath toPath:(NSString *)toPath error:(NSError **)error
{
	if(fromPath==nil || toPath==nil) {
		return NO;
	}
	
	return MoveFileW([fromPath fileSystemRepresentationW],[toPath fileSystemRepresentationW])?YES:NO;
}


-(BOOL)movePath:(NSString *)src toPath:(NSString *)dest handler:handler
{
    NSError *error = nil;
    if ([self moveItemAtPath:src toPath:dest error:&error] == NO && handler != nil) {
        //[self _errorHandler:handler src:src dest:dest operation:[error description]];
        return NO;
    }
    
    return YES;
}

#pragma mark -
#pragma mark Copying an item

-(BOOL)copyItemAtPath:(NSString *)fromPath toPath:(NSString *)toPath error:(NSError **)error
{
	NSDictionary *srcAttributes=[self attributesOfItemAtPath:fromPath error:error];
	
	if(srcAttributes==nil) {
		return NO;
	}
	
	if([[srcAttributes fileType] isEqual:NSFileTypeRegular] == NO
       && [[srcAttributes fileType] isEqual:NSFileTypeDirectory] == NO) {
		return NO;
	}
    
    if ([self fileExistsAtPath:toPath] == YES) {
        if(error!=NULL){
			NSDictionary *userInfo=[NSDictionary dictionaryWithObject:@"File exists" forKey:NSLocalizedDescriptionKey];
			
			*error=[NSError errorWithDomain:NSPOSIXErrorDomain code:17 userInfo:userInfo];
		}
		return NO;
    }
	
	BOOL isDirectory = NO;
	if ([self fileExistsAtPath:fromPath isDirectory:&isDirectory] && isDirectory) {
		if(!CopyDirectoryW([fromPath fileSystemRepresentationW],[toPath fileSystemRepresentationW])) {
			if(error!=NULL){
				NSString *msg = [NSString stringWithFormat:@"Copy error (%d)", GetLastError()];
				NSDictionary *userInfo=[NSDictionary dictionaryWithObject:msg forKey:NSLocalizedDescriptionKey];
				*error=[NSError errorWithDomain:NSPOSIXErrorDomain code:17 userInfo:userInfo];
			}
			NSLog(@"FILE Error %@ %@ %d", fromPath, toPath, GetLastError());
			return NO;
		}
	} else {
		if(!CopyFileW([fromPath fileSystemRepresentationW],[toPath fileSystemRepresentationW],YES)) {
			if(error!=NULL){
				NSString *msg = [NSString stringWithFormat:@"Copy error (%d)", GetLastError()];
				NSDictionary *userInfo=[NSDictionary dictionaryWithObject:msg forKey:NSLocalizedDescriptionKey];
				*error=[NSError errorWithDomain:NSPOSIXErrorDomain code:17 userInfo:userInfo];
			}
			NSLog(@"FILE Error %@ %@ %d", fromPath, toPath, GetLastError());
			return NO;
		}
	}
	return YES;
}

-(BOOL)copyPath:(NSString *)src toPath:(NSString *)dest handler:handler
{
    NSError *error = nil;
    if ([self copyItemAtPath:src toPath:dest error:&error] == NO && handler != nil) {
        //[self _errorHandler:handler src:src dest:dest operation:[error description]];
        return NO;
    }
    
    return YES;
}

#pragma mark -
#pragma mark Removing an item

-(BOOL)removeItemAtPath:(NSString *)path error:(NSError **)error
{
	if(path == nil) {
		return NO;
	}
	
	const unichar *fsrep=[path fileSystemRepresentationW];
	DWORD       attribute=GetFileAttributesW(fsrep);
	
	if([path isEqualToString:@"."] || [path isEqualToString:@".."]){
		[NSException raise:NSInvalidArgumentException format:@"-[%@ %s] path should not be . or ..",isa,sel_getName(_cmd)];
		return NO;
	}
	
	if(attribute==0xFFFFFFFF){
		if(error!=NULL)
			*error=NSErrorForGetLastError();
		return NO;
	}
	
	if(attribute&FILE_ATTRIBUTE_READONLY){
		attribute&=~FILE_ATTRIBUTE_READONLY;
		if(!SetFileAttributesW(fsrep,attribute)){
			if(error!=NULL)
				*error=NSErrorForGetLastError();
			return NO;
		}
	}
	
	if(![self _isDirectory:path]){
        if (_delegate != nil) {
            if(![_delegate fileManager:self shouldRemoveItemAtPath:path]){
                if(error!=NULL)
                    *error=nil; // FIXME; is there a Cocoa error for the delegate cancelling?
                return NO;
            }
        }
        
		if(!DeleteFileW(fsrep)){
			if(error!=NULL)
				*error=NSErrorForGetLastError();
			return NO;
		}
	}
	else {
		NSArray *contents=[self directoryContentsAtPath:path];
		NSInteger      i,count=[contents count];
		
        for(i=0;i<count;i++){
            NSString *fullPath=[path stringByAppendingPathComponent:[contents objectAtIndex:i]];
            if (_delegate != nil) {
                if(![_delegate fileManager:self shouldRemoveItemAtPath:fullPath]){
                    if(error!=NULL)
                        *error=nil; // FIXME; is there a Cocoa error for the delegate cancelling?
                    return NO;
                }
            }
            if ([self removeItemAtPath:fullPath error:error] == NO) {
                return NO;
            }
        } 
		
		if(!RemoveDirectoryW(fsrep)){
			if(error!=NULL)
				*error=NSErrorForGetLastError();
			return NO;
		}
	}
	
	return YES;
}

-(BOOL)removeFileAtPath:(NSString *)path handler:handler
{
    NSError *error = nil;
    if ([self removeItemAtPath:path error:&error] == NO && handler != nil) {
        //[self _errorHandler:handler src:src dest:dest operation:[error description]];
        return NO;
    }
    
    return YES;
}

#pragma mark -
#pragma mark Creating an item

-(BOOL)createDirectoryAtPath:(NSString *)path withIntermediateDirectories:(BOOL)intermediates attributes:(NSDictionary *)attributes error:(NSError **)error
{	
	if(intermediates){
		NSArray  *components=[path pathComponents];
		NSInteger i,count=[components count];
		NSString *check=@"";
		
		for(i=0;i<count-1;i++){
			check=[check stringByAppendingPathComponent:[components objectAtIndex:i]];
			// ignore errors on intermediates since we're not handling all possible error codes.
			_NSCreateDirectory(check,NULL);
		}
	}
	
	return _NSCreateDirectory(path,error);
}

-(BOOL)createFileAtPath:(NSString *)path contents:(NSData *)data attributes:(NSDictionary *)attributes
{
	return [[NSPlatform currentPlatform] writeContentsOfFile:path bytes:[data bytes] length:[data length] options:NSAtomicWrite error:NULL];
}

-(BOOL)createDirectoryAtPath:(NSString *)path attributes:(NSDictionary *)attributes
{
	return CreateDirectoryW([path fileSystemRepresentationW],NULL)?YES:NO;
}

#pragma mark -
#pragma mark Symbolic-Link Operations

-(NSString *)destinationOfSymbolicLinkAtPath:(NSString *)path error:(NSError **)error
{
// Code found at: http://www.catch22.net/tuts/tips2
    IShellLinkW * psl;
	
    SHFILEINFOW   info;
	
    IPersistFile *ppf;
	
	int nPathLen = [path length];
    WCHAR pszFilePath[1024];   
	WCHAR *pszShortcut = (WCHAR*)[path fileSystemRepresentationW];
	
    // assume failure
    if((SHGetFileInfoW(pszShortcut, 0, &info, sizeof(info), SHGFI_ATTRIBUTES) == 0)) {
	
//		NSLog(@"failed to get attributes for %S", pszShortcut);
		DWORD errNum=GetLastError();
		if(errNum != ERROR_ALREADY_EXISTS && error != nil) {
			*error = NSErrorForGetLastError();
		}
        return nil;
	}
	
	
    // not a shortcut?
	
    if(!(info.dwAttributes & SFGAO_LINK)) {
		
		// Docs say return nil on failure
		return nil;
    }	
	
    // obtain the IShellLink interface
	
    if(FAILED(CoCreateInstance(&CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, &IID_IShellLinkW, (LPVOID*)&psl))) {
	
		NSLog(@"IShellLink CoCreateInstance failed");
		DWORD errNum=GetLastError();

		if(errNum != ERROR_ALREADY_EXISTS && error != nil) {
			*error = NSErrorForGetLastError();
		}
        return nil;
	}
	
    if (SUCCEEDED(psl->lpVtbl->QueryInterface(psl, &IID_IPersistFile, (LPVOID*)&ppf))) {
		
        if (SUCCEEDED(ppf->lpVtbl->Load(ppf, pszShortcut, STGM_READ))) {
			
            // Resolve the link, this may post UI to find the link
            if (SUCCEEDED(psl->lpVtbl->Resolve(psl, 0, SLR_NO_UI))) {
                psl->lpVtbl->GetPath(psl, pszFilePath, 1024, NULL, 0);
				
                ppf->lpVtbl->Release(ppf);
				
                psl->lpVtbl->Release(psl);

				NSString* resolvedPath = [NSString stringWithFormat:@"%S", pszFilePath];
				// Mac-ify the path
				resolvedPath = [resolvedPath stringByReplacingOccurrencesOfString:@"\\" withString:@"/"];
				return resolvedPath;
            } else {
				NSLog(@"Unable to resolve link");
				DWORD errNum=GetLastError();
				
				if(errNum != ERROR_ALREADY_EXISTS && error != nil) {
					*error = NSErrorForGetLastError();
				}
			}
			
        } else {
			NSLog(@"IPersistFile->Load failed for: %S", pszShortcut);
			DWORD errNum=GetLastError();
			
			if(errNum != ERROR_ALREADY_EXISTS && error != nil) {
				*error = NSErrorForGetLastError();
			}
		}
		
        ppf->lpVtbl->Release(ppf);
		
    } else {
		NSLog(@"IShellLink->QueryInterface() for IPersistFile failed");
		DWORD errNum=GetLastError();
		
		if(errNum != ERROR_ALREADY_EXISTS && error != nil) {
			*error = NSErrorForGetLastError();
		}
	}
	
	
	
    psl->lpVtbl->Release(psl);
	
    return nil;
}

#pragma mark -
#pragma mark Discovering Directory Contents

-(NSArray *)contentsOfDirectoryAtPath:(NSString *)path error:(NSError **)error
{
   NSMutableArray   *result=[NSMutableArray array];
   WIN32_FIND_DATAW findData;
   HANDLE           handle;
   
   if(path == nil) {
    return nil;
   }
   
   handle=FindFirstFileW([[path stringByAppendingString:@"\\*.*"] fileSystemRepresentationW],&findData);

   if(handle==INVALID_HANDLE_VALUE)
    return nil;

   do{
    if(wcscmp(findData.cFileName,L".")!=0 && wcscmp(findData.cFileName,L"..")!=0)
     [result addObject:[NSString stringWithCharacters:findData.cFileName length:wcslen(findData.cFileName)]];
   }while(FindNextFileW(handle,&findData));

   FindClose(handle);

   return result;
}

-(NSArray *)directoryContentsAtPath:(NSString *)path
{
   NSMutableArray   *result=[NSMutableArray array];
   WIN32_FIND_DATAW findData;
   HANDLE           handle;
   
   if(path == nil) {
        return nil;
   }
    
   handle=FindFirstFileW([[path stringByAppendingString:@"\\*.*"] fileSystemRepresentationW],&findData);

   if(handle==INVALID_HANDLE_VALUE)
    return nil;

   do{
    if(wcscmp(findData.cFileName,L".")!=0 && wcscmp(findData.cFileName,L"..")!=0)
      [result addObject:[NSString stringWithCharacters:findData.cFileName length:wcslen(findData.cFileName)]];
   }while(FindNextFileW(handle,&findData));

   FindClose(handle);

   return result;
}

#pragma mark -
#pragma mark Getting and Setting Attributes

-(NSDictionary *)attributesOfFileSystemForPath:(NSString *)path error:(NSError **)errorp {
	DWORD serialNumber;
	
	if(path == nil) {
		return nil;
	}

	path = TranslatePath(path);

	if(![path hasSuffix:@"\\"]) {
		path=[path stringByAppendingString:@"\\"];
    }
	
	NSMutableDictionary* attrs = [NSMutableDictionary dictionaryWithCapacity: 3];
	
	const uint16_t *wPath = [path fileSystemRepresentationW];
	ULARGE_INTEGER freeBytesAvailable;
	ULARGE_INTEGER totalNumberOfBytes; // This seems to be the same as freeBytesAvailable
	ULARGE_INTEGER totalNumberOfFreeBytes;
	
	if (GetDiskFreeSpaceExW(wPath, &freeBytesAvailable, &totalNumberOfBytes, &totalNumberOfBytes) != 0) {
		[attrs setObject: [NSNumber numberWithUnsignedLongLong: freeBytesAvailable.QuadPart] forKey: NSFileSystemFreeSize];
		[attrs setObject: [NSNumber numberWithUnsignedLongLong: totalNumberOfFreeBytes.QuadPart] forKey: NSFileSystemSize];
	} else {
		NSError* error = NSErrorForGetLastError();
		NSLog(@"GetDiskFreeSpaceExW failed for path: '%@' with error: %@", path, error);
		if (errorp) {
			*errorp = error;
		}
		return nil;
	}

	
	NSString* driveLetter = DriveLetterInPath(path);
	if (driveLetter) {
		wPath = [driveLetter fileSystemRepresentationW];
	}
	// This seems to prefer Drive Letters to full paths
	if (GetVolumeInformationW(wPath, NULL , 0, &serialNumber, NULL, NULL, NULL, 0 ) != 0) {
		[attrs setObject:[NSNumber numberWithInt:serialNumber] forKey:NSFileSystemNumber];
	} else {
		NSError* error = NSErrorForGetLastError();
		NSLog(@"GetVolumeInformationW failed for path: '%@' with error: %@", path, error);
		if (errorp) {
			*errorp = error;
		}
		return nil;
	}
	
	return attrs;
}

-(NSDictionary *)attributesOfItemAtPath:(NSString *)path error:(NSError **)error {
	WIN32_FILE_ATTRIBUTE_DATA  fileData;
	
	if(path == nil) {
        return nil;
	}
	
	if (!GetFileAttributesExW( [path fileSystemRepresentationW],GetFileExInfoStandard,&fileData) ) {
		// TODO: set error
		return nil;
	}
	
    char		pSecurityDescriptor[128];
    DWORD		lengthNeeded;

	NSMutableDictionary *result = [NSMutableDictionary dictionary];
	NSDate *date = [NSDate dateWithTimeIntervalSinceReferenceDate:Win32TimeIntervalFromFileTime(fileData.ftLastWriteTime)];
	[result setObject:date forKey:NSFileModificationDate];
	
	NSString *fileType = NSFileTypeRegular;
	if (fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) fileType = NSFileTypeDirectory;
    
    if (GetFileSecurityW([path fileSystemRepresentationW], OWNER_SECURITY_INFORMATION, (SECURITY_DESCRIPTOR *)pSecurityDescriptor, 128, &lengthNeeded) != 0) {
        PSID 			sid = NULL;
        DWORD			lpbOwnerDefaulted;
        char 			lpName[128];
        DWORD			len = 128;
        SID_NAME_USE	nameUse;
        char			referencedDomainName[128];
        DWORD			domainLen = 128;
        
        if (GetSecurityDescriptorOwner((SECURITY_DESCRIPTOR *)pSecurityDescriptor, &sid, ((LPBOOL)(&lpbOwnerDefaulted))) != 0) {
            if (LookupAccountSid(NULL, sid, lpName, &len, referencedDomainName, &domainLen, &nameUse) != 0) {
                NSString    *owner = [[[NSString alloc] initWithCString:lpName] autorelease];
                
                if (referencedDomainName != NULL) {
                    [result setObject:[NSString stringWithFormat:@"%@\\%@", [[[NSString alloc] initWithCString:referencedDomainName] autorelease], owner] forKey:NSFileOwnerAccountName];
                }
                else {
                    [result setObject:owner forKey:NSFileOwnerAccountName];
                }
            } else {
                DWORD lastError = GetLastError();
                //1332 means that the sid is not resolvable (an old one/or on a network drive)
                if (lastError == 1332) {
                    /*LPWSTR  str = NULL;
                    if (ConvertSidToStringSidW(sid, &str) == TRUE) {
                        NSString    *owner = NSStringFromNullTerminatedUnicode(str);
                        LocalFree(str);
                        [result setObject:owner forKey:NSFileOwnerAccountName];
                    }
                    else {
                        if (error != nil) {
                            *error = NSErrorForGetLastErrorCode(lastError);
                        }
                        return nil;
                    }*/
                }
                else {
                    if (error != nil) {
                        *error = NSErrorForGetLastErrorCode(lastError);
                    }
                    return nil;
                }
            }
        }
/* FIXME: GetFileSecurityW(path, OWNER_SECURITY_INFORMATION, ...) retunrs for files on network
          shares on Windows 7 HP 32bit always 0, and because of this the present function would
          returns nil below. So files on network shares could not be accessed. See:
          https://github.com/cjwl/cocotron/issues/5
    } else {
        // TODO: set error
		return nil;
*/
    }
	
	[result setObject:fileType forKey:NSFileType];
	[result setObject:@"" forKey:NSFileGroupOwnerAccountName];
	[result setObject:[NSNumber numberWithUnsignedLong:0666] forKey:NSFilePosixPermissions];
	
	uint64_t sizeOfFile = fileData.nFileSizeLow;
	uint64_t sizeHigh = fileData.nFileSizeHigh;
	sizeOfFile |= sizeHigh << 32;
	
	[result setObject:[NSNumber numberWithUnsignedLongLong:sizeOfFile] forKey:NSFileSize];	
	
	return result;
}

-(BOOL)setAttributes:(NSDictionary *)attributes ofItemAtPath:(NSString *)path error:(NSError **)error
{
    if (error != NULL) {
        //TODO set error
    }
    
	return NO;
#if 0
	NSDate *date=[attributes objectForKey:NSFileModificationDate];
	
	if(date!=nil){
		time_t timep[2]={ time(NULL),[date timeIntervalSince1970] };
		if(utime((unichar *)[path fileSystemRepresentationW],timep)<0)
			return NO;
	}
    
	return YES;
#endif

}

-(NSDictionary *)fileAttributesAtPath:(NSString *)path traverseLink:(BOOL)traverse
{
	return [self attributesOfItemAtPath: path error: 0];
}

#pragma mark -
#pragma mark Determining Access To Files


-(BOOL)fileExistsAtPath:(NSString *)path isDirectory:(BOOL *)isDirectory {
   if(path == nil) {
    return NO;
   }

    DWORD attributes=GetFileAttributesW([self fileSystemRepresentationWithPathW:path]);

   if(attributes==0xFFFFFFFF)
    return NO;

   if(isDirectory!=NULL)
    *isDirectory=(attributes&FILE_ATTRIBUTE_DIRECTORY)?YES:NO;

   return YES;
}


-(BOOL)isReadableFileAtPath:(NSString *)path {
    if(path == nil) {
     return NO;
    }
    DWORD attributes=GetFileAttributesW([path fileSystemRepresentationW]);

   if(attributes==-1)
    return NO;

   return YES;
}

-(BOOL)isWritableFileAtPath:(NSString *)path {
   if(path == nil) {
    return NO;
   }
   DWORD attributes=GetFileAttributesW([path fileSystemRepresentationW]);

   if(attributes==-1)
    return NO;

   if(attributes&FILE_ATTRIBUTE_READONLY)
    return NO;

   return YES;
}

-(BOOL)isExecutableFileAtPath:(NSString *)path {
   if(path == nil) {
     return NO;
   }
   DWORD attributes=GetFileAttributesW([path fileSystemRepresentationW]);

   if(attributes==-1)
    return NO;

   if(attributes&(FILE_ATTRIBUTE_DIRECTORY))
    return NO;

   return [[[path pathExtension] uppercaseString] isEqualToString:@"EXE"];
}

-(NSString *)displayNameAtPath:(NSString *)path {
	NSString *result = [super displayNameAtPath:path];
	if ([result isEqualToString:[path lastPathComponent]]) {
		// Check if Win32 can find a better name
		const unichar *pathCString=[path fileSystemRepresentationW];
		SHFILEINFOW fileInfo;
		if(SHGetFileInfoW(pathCString, FILE_ATTRIBUTE_NORMAL, &fileInfo, sizeof(SHFILEINFOW), SHGFI_DISPLAYNAME)) {
			result = [NSString stringWithFormat:@"%S", fileInfo.szDisplayName];
		}
	}
	return result;
}

#pragma mark -
#pragma mark Getting Representations of File Paths

-(NSString *)stringWithFileSystemRepresentation:(const char *)string length:(NSUInteger)length {
	return [NSString stringWithCString:string length:length];
}

-(const unichar*)fileSystemRepresentationWithPathW:(NSString *)path {
    path = TranslatePath( path );
	
    return (const unichar *)[path cStringUsingEncoding:NSUnicodeStringEncoding];
}

-(const char*)fileSystemRepresentationWithPath:(NSString *)path {
    path = TranslatePath( path );
    return [path cString];
}

#pragma mark -
#pragma mark Managing the Current Directory

-(BOOL)changeCurrentDirectoryPath:(NSString *)path {
   if(path == nil) { 
    return NO;
   }
   if (SetCurrentDirectoryW([self fileSystemRepresentationWithPathW:path]))
    return YES;
   Win32Assert("SetCurrentDirectory");

   return NO;
}

-(NSString *)currentDirectoryPath {
	unichar  path[MAX_PATH+1];
	DWORD length;
	
	length=GetCurrentDirectoryW(MAX_PATH+1,path);
	Win32Assert("GetCurrentDirectory");
	
	return [NSString stringWithCharacters:path length:length];
}

@end
#endif
