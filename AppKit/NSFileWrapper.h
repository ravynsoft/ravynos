/* Copyright (c) 2007 Dirk Theisen

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Foundation/Foundation.h>

@class NSImage;

@interface NSFileWrapper : NSObject {
  @protected
    NSString *_path;
    NSMutableDictionary *_fileAttributes;
    NSString *_filename;
    NSString *_preferredFilename;
    NSImage *_icon;
}

- (id)initDirectoryWithFileWrappers:(NSDictionary *)docs;
// Designated initializer. Inits new directory type instances.

- (id)initRegularFileWithContents:(NSData *)data;
// Designated initializer. Inits new regular file type instances.

- (id)initSymbolicLinkWithDestination:(NSString *)path;
// Designated initializer. Inits new symbolic link type instances.

- (id)initWithPath:(NSString *)path;
// Designated initializer.

- (NSDictionary *)fileWrappers;
- (NSString *)addFileWrapper:(NSFileWrapper *)wrapper;

- (NSString *)filename;
- (void)setFilename:(NSString *)filename;

- (NSString *)preferredFilename;
- (void)setPreferredFilename:(NSString *)filename;

- (void)setFileAttributes:(NSDictionary *)attributes;
- (NSDictionary *)fileAttributes;

- (BOOL)isRegularFile;
- (BOOL)isDirectory;
- (BOOL)isSymbolicLink;

- (NSData *)regularFileContents;
- (NSString *)symbolicLinkDestination;

- (NSImage *)icon;
- (void)setIcon:(NSImage *)anImage;

// Write instace to disk at path. if directory type, this method is recursive
// if flag is YES, the wrapper will be updated with the name used in writing the file
- (BOOL)writeToFile:(NSString *)path
         atomically:(BOOL)atomicFlag
    updateFilenames:(BOOL)updateFilenamesFlag;

- (BOOL)needsToBeUpdatedFromPath:(NSString *)path;
- (BOOL)updateFromPath:(NSString *)path;

@end
