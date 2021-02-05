/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#ifdef PLATFORM_IS_POSIX
#import <Foundation/NSFileManager_posix.h>
#import <Foundation/NSArray.h>
#import <Foundation/NSData.h>
#import <Foundation/NSDate.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSNumber.h>
#import <Foundation/NSPathUtilities.h>
#import <Foundation/NSThread.h>
#import <Foundation/NSRaise.h>
#import <Foundation/NSString_cString.h>
#import <Foundation/NSThread-Private.h>
#import <Foundation/NSRaiseException.h>

#import <Foundation/NSPlatform_posix.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <unistd.h>
#include <fcntl.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <dirent.h>
#include <errno.h>

@implementation NSFileManager(posix)

+allocWithZone:(NSZone *)zone {
   return NSAllocateObject([NSFileManager_posix class],0,NULL);
}

@end

@implementation NSFileManager_posix

-(BOOL)createFileAtPath:(NSString *)path contents:(NSData *)data
             attributes:(NSDictionary *)attributes {
    return [[NSPlatform currentPlatform] writeContentsOfFile:path bytes:[data bytes] length:[data length] options:NSAtomicWrite error:NULL];
}

-(NSArray *)directoryContentsAtPath:(NSString *)path {
    return [self contentsOfDirectoryAtPath:path error:NULL];
}

-(NSArray *)contentsOfDirectoryAtPath:(NSString *)path error:(NSError **)error
{
//TODO fill error
    NSMutableArray *result=nil;
    DIR *dirp = NULL;
    struct dirent *dire;

    if(path == nil) {
        return nil;
    }

    dirp = opendir([path fileSystemRepresentation]);
    
    if (dirp == NULL)
        return nil;

    result=[NSMutableArray array];

    while ((dire = readdir(dirp))){
	 if(strcmp(".",dire->d_name)==0)
	  continue;
	 if(strcmp("..",dire->d_name)==0)
	  continue;
     [result addObject:[NSString stringWithCString:dire->d_name]];
    }

    closedir(dirp);
    
    return result;
}


-(BOOL)createDirectoryAtPath:(NSString *)path attributes:(NSDictionary *)attributes {
    // you can set all these, but we don't respect 'em all yet
    //NSDate *date = [attributes objectForKey:NSFileModificationDate];
    //NSString *owner = [attributes objectForKey:NSFileOwnerAccountName];
    //NSString *group = [attributes objectForKey:NSFileGroupOwnerAccountName];
    int mode = [[attributes objectForKey:NSFilePosixPermissions] intValue];

    if (mode == 0)
        mode = FOUNDATION_DIR_MODE;

    return (mkdir([path fileSystemRepresentation], mode) == 0);
}

-(BOOL)fileExistsAtPath:(NSString *)path isDirectory:(BOOL *)isDirectory {
    struct stat buf;

    if(stat([path fileSystemRepresentation],&buf)<0)
        return NO;

    if(isDirectory!=NULL)
     *isDirectory=S_ISDIR(buf.st_mode);

    return YES;
}

// we dont want to use fileExists... because it chases links
-(BOOL)_isDirectory:(NSString *)path {
    struct stat buf;

    if(lstat([path fileSystemRepresentation],&buf)<0)
        return NO;

    if (buf.st_mode & S_IFDIR && !(buf.st_mode & S_IFLNK))
        return YES;

    return NO;
}

-(BOOL)_errorHandler:handler src:(NSString *)src dest:(NSString *)dest operation:(NSString *)op {
    if ([handler respondsToSelector:@selector(fileManager:shouldProceedAfterError:)]) {
        NSDictionary *errorInfo = [NSDictionary dictionaryWithObjectsAndKeys:
            src, @"Path",
            [NSString stringWithFormat:@"%@: %s", op, strerror(errno)], @"Error",
            dest, @"ToPath",
            nil];

        if ([handler fileManager:self shouldProceedAfterError:errorInfo])
            return YES;
    }

    return NO;
}

- (BOOL)removeItemAtPath:(NSString *)path error:(NSError **)error {
    if([path isEqualToString:@"."] || [path isEqualToString:@".."])
        NSRaiseException(NSInvalidArgumentException, self, _cmd, @"%@: invalid path", path);

    if(![self _isDirectory:path]){
        if(remove([path fileSystemRepresentation]) == -1) {
            if(error!=NULL)
				*error=nil; //TODO set error
            return NO;
        }
    }
    else{
        NSArray *contents=[self directoryContentsAtPath:path];
        NSInteger i,count=[contents count];
        
        for(i=0;i<count;i++){
            NSString *name = [contents objectAtIndex:i];
            NSString *fullPath;
            
            if([name isEqualToString:@"."] || [name isEqualToString:@".."])
                continue;
            
            fullPath=[path stringByAppendingPathComponent:name];
            if(![self removeItemAtPath:fullPath error:error]) {
                if(error!=NULL)
                    *error=nil; //TODO set error
                return NO;
            }
        }
        
        if(rmdir([path fileSystemRepresentation]) == -1) {
            if(error!=NULL)
				*error=nil; //TODO set error
            return NO;
        }
    }
    return YES;
}

-(BOOL)removeFileAtPath:(NSString *)path handler:handler {
    NSError *error = nil;
    
    if ([handler respondsToSelector:@selector(fileManager:willProcessPath:)])
        [handler fileManager:self willProcessPath:path];

    if ([self removeItemAtPath:path error:&error] == NO && handler != nil) {
        [self _errorHandler:handler src:path dest:@"" operation:[error description]];
        return NO;
    }
    
    return YES;
}


-(BOOL)movePath:(NSString *)src toPath:(NSString *)dest handler:handler {
    NSError *error = nil;
    
    if ([handler respondsToSelector:@selector(fileManager:willProcessPath:)])
        [handler fileManager:self willProcessPath:src];

    if ([self moveItemAtPath:src toPath:dest error:&error] == NO && handler != nil) {
        [self _errorHandler:handler src:src dest:dest operation:[error description]];
        return NO;
    }
    
    return YES;
}

- (BOOL)moveItemAtPath:(NSString *)srcPath toPath:(NSString *)dstPath error:(NSError **)error
{
    
    /*
     It's not this easy...
     return rename([src fileSystemRepresentation],[dest fileSystemRepresentation])?NO:YES;
     */

    BOOL isDirectory;
    
//TODO fill error
    
    if ([self fileExistsAtPath:srcPath isDirectory:&isDirectory] == NO)
        return NO;
    if ([self fileExistsAtPath:dstPath isDirectory:&isDirectory] == YES)
        return NO;
    
    if ([self copyPath:srcPath toPath:dstPath handler:nil] == NO) {
        [self removeFileAtPath:dstPath handler:nil];
        return NO;
    }
    
    // not much we can do if this fails
    [self removeFileAtPath:srcPath handler:nil];
    
    return YES;
}

-(BOOL)copyPath:(NSString *)src toPath:(NSString *)dest handler:handler {
    NSError *error = nil;
    if ([self copyItemAtPath:src toPath:dest error:&error] == NO && handler != nil) {
        [self _errorHandler:handler src:src dest:dest operation:[error description]];
        return NO;
    }
    
    return YES;
}

-(BOOL)copyItemAtPath:(NSString *)fromPath toPath:(NSString *)toPath error:(NSError **)error
{
    BOOL isDirectory;
    
    if(![self fileExistsAtPath:fromPath isDirectory:&isDirectory]) {
        if (error != NULL) {
            //TODO set error
        }
        return NO;
    }    
    
    if (!isDirectory){
        int r, w;
        char buf[4096];
        size_t count;
        
        if ((w = open([toPath fileSystemRepresentation], O_WRONLY|O_CREAT, FOUNDATION_FILE_MODE)) == -1) {
            if (error != NULL) {
                //TODO set error
            }
            return NO;
        }
        if ((r = open([fromPath fileSystemRepresentation], O_RDONLY)) == -1) {
            if (error != NULL) {
                //TODO set error
            }
            close(w);
            return NO;

        }
        
        while ((count = read(r, &buf, sizeof(buf))) > 0) {
            if (count == -1) 
                break;
            
            if (write(w, &buf, count) != count) {
                count = -1;
                break;
            }
        }
        
        close(w);
        close(r);
        
        if (count == -1) {
            if (error != NULL) {
                //TODO set error
            }
            return NO;
        }
        else
            return YES;
    }
    else {
        NSArray *files;
        NSInteger      i,count;
        
        if (mkdir([toPath fileSystemRepresentation], FOUNDATION_DIR_MODE) != 0) {
            if (error != NULL) {
                //TODO set error
            }
            return NO;
        }
        
        //if (chdir([dest fileSystemRepresentation]) != 0)
        //    return [self _errorHandler:handler src:src dest:dest operation:@"copyPath: chdir(subdir)"];
        
        files = [self directoryContentsAtPath:fromPath];
        count = [files count];
        
        for(i=0;i<count;i++){
            NSString *name=[files objectAtIndex:i];
            NSString *subsrc, *subdst;
            
            if ([name isEqualToString:@"."] || [name isEqualToString:@".."])
                continue;
            
            subsrc=[fromPath stringByAppendingPathComponent:name];
            subdst=[toPath stringByAppendingPathComponent:name];
            
            if([self copyItemAtPath:subsrc toPath:subdst error:error] == NO) {
                if (error != NULL) {
                    //TODO set error
                }
                return NO;
            }
        }
        
        //if (chdir("..") != 0)
        //    return [self _errorHandler:handler src:src dest:dest operation:@"copyPath: chdir(..)"];
    }
    
    return YES;

}
-(NSString *)currentDirectoryPath {
    char  path[MAXPATHLEN+1];

    if (getcwd(path, sizeof(path)) != NULL)
        return [NSString stringWithCString:path];

    return nil;
}

-(NSString *)pathContentOfSymbolicLinkAtPath:(NSString *)path {
    char linkbuf[MAXPATHLEN+1];
    size_t length;

    length = readlink([path fileSystemRepresentation], linkbuf, MAXPATHLEN);
    if (length ==-1)
        return nil;

    linkbuf[length] = 0;
    return [NSString stringWithCString:linkbuf];
}


-(NSDictionary *)fileAttributesAtPath:(NSString *)path traverseLink:(BOOL)traverse {
    NSMutableDictionary *result=[NSMutableDictionary dictionary];
    struct stat statBuf;
    struct passwd *pwd;
    struct group *grp;

    if (lstat([path fileSystemRepresentation], &statBuf) != 0)
        return nil;

    // (Not in POSIX.1-1996.)
    if (S_ISLNK(statBuf.st_mode) && traverse) {
        NSString *linkContents = [self pathContentOfSymbolicLinkAtPath:path];
        return [self fileAttributesAtPath:linkContents traverseLink:traverse];
    }

    [result setObject:[NSNumber numberWithUnsignedLong:statBuf.st_size]
               forKey:NSFileSize];
    [result setObject:[NSDate dateWithTimeIntervalSince1970:statBuf.st_mtime]
               forKey:NSFileModificationDate];

    // User/group names don't always exist for the IDs in the filesystem.
    // If we don't check for NULLs, we'll segfault.
    pwd = getpwuid(statBuf.st_uid);
    if (pwd != NULL)
        [result setObject:[NSString stringWithCString:pwd->pw_name]
                   forKey:NSFileOwnerAccountName];

    grp = getgrgid(statBuf.st_gid);
    if (grp != NULL)
        [result setObject:[NSString stringWithCString:grp->gr_name]
                   forKey:NSFileGroupOwnerAccountName];

    [result setObject:[NSNumber numberWithUnsignedLong:statBuf.st_nlink]
               forKey:NSFileReferenceCount];
    [result setObject:[NSNumber numberWithUnsignedLong:statBuf.st_ino]
               forKey:NSFileIdentifier];
    [result setObject:[NSNumber numberWithUnsignedLong:statBuf.st_dev]
               forKey:NSFileDeviceIdentifier];
    [result setObject:[NSNumber numberWithUnsignedLong:statBuf.st_mode]
               forKey:NSFilePosixPermissions];

    // ugh.. skip this if we can
    if (!S_ISREG(statBuf.st_mode)) {
        if (S_ISDIR(statBuf.st_mode))
            [result setObject:NSFileTypeDirectory forKey:NSFileType];
        else if (S_ISCHR(statBuf.st_mode))
            [result setObject:NSFileTypeCharacterSpecial forKey:NSFileType];
        else if (S_ISBLK(statBuf.st_mode))
            [result setObject:NSFileTypeBlockSpecial forKey:NSFileType];
        else if (S_ISFIFO(statBuf.st_mode))
            [result setObject:NSFileTypeFIFO forKey:NSFileType];
        else if (S_ISLNK(statBuf.st_mode))
            [result setObject:NSFileTypeSymbolicLink forKey:NSFileType];
        else if (S_ISSOCK(statBuf.st_mode))
            [result setObject:NSFileTypeSocket forKey:NSFileType];
    }
    else
        [result setObject:NSFileTypeRegular forKey:NSFileType];

    return result;
}

-(BOOL)isReadableFileAtPath:(NSString *)path {
    return access([path fileSystemRepresentation], R_OK) ? NO : YES;
}

-(BOOL)isWritableFileAtPath:(NSString *)path {
    return access([path fileSystemRepresentation], W_OK) ? NO : YES;
}

-(BOOL)isExecutableFileAtPath:(NSString *)path {
    return access([path fileSystemRepresentation], X_OK) ? NO : YES;
}

-(BOOL)createSymbolicLinkAtPath:(NSString *)path pathContent:(NSString *)otherPath {
    return (symlink([otherPath fileSystemRepresentation], [path fileSystemRepresentation]) == 0);
}

-(BOOL)setAttributes:(NSDictionary *)attributes ofItemAtPath:(NSString *)path error:(NSError **)error
{
    if (error != NULL) {
        //TODO set error
    }
    
    return NO;
}

-(const char *)fileSystemRepresentationWithPath:(NSString *)path {
    return [path cStringUsingEncoding:NSUTF8StringEncoding];
}

-(NSString *)destinationOfSymbolicLinkAtPath:(NSString *)path error:(NSError **)error {
    char destination[MAXPATHLEN+1];
    ssize_t bytes;

    bytes = readlink([path fileSystemRepresentation], destination, MAXPATHLEN);

    if (bytes == -1) {
        //TODO fill error
        return nil;
    }

    destination[bytes] = 0;

    return [NSString stringWithCString:destination encoding:NSUTF8StringEncoding];
}

@end
#endif
