/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Foundation/Foundation.h>
#import <AppKit/AppKitExport.h>

@class NSImage, NSView;

APPKIT_EXPORT NSString *const NSWorkspaceWillPowerOffNotification;

APPKIT_EXPORT NSString *const NSWorkspaceRecycleOperation;

@interface NSWorkspace : NSObject {
    NSNotificationCenter *_notificationCenter;
}

+ (NSWorkspace *)sharedWorkspace;

- (NSNotificationCenter *)notificationCenter;

- (NSImage *)iconForFile:(NSString *)path;
- (NSImage *)iconForFiles:(NSArray *)array;
- (NSImage *)iconForFileType:(NSString *)type;
- (NSString *)localizedDescriptionForType:(NSString *)type;
- (BOOL)filenameExtension:(NSString *)extension isValidForType:(NSString *)type;
- (NSString *)preferredFilenameExtensionForType:(NSString *)type;
- (BOOL)type:(NSString *)type conformsToType:(NSString *)conformsToType;
- (NSString *)typeOfFile:(NSString *)path error:(NSError **)error;

- (BOOL)openFile:(NSString *)path;
- (BOOL)openFile:(NSString *)path withApplication:(NSString *)application;
- (BOOL)openTempFile:(NSString *)path;
- (BOOL)openFile:(NSString *)path fromImage:(NSImage *)image at:(NSPoint)point inView:(NSView *)view;
- (BOOL)openFile:(NSString *)path withApplication:(NSString *)application andDeactivate:(BOOL)deactivate;
- (BOOL)openURL:(NSURL *)url;

- (BOOL)selectFile:(NSString *)path inFileViewerRootedAtPath:(NSString *)rootedAtPath;
- (void)slideImage:(NSImage *)image from:(NSPoint)from to:(NSPoint)to;
- (BOOL)performFileOperation:(NSString *)operation source:(NSString *)source destination:(NSString *)destination files:(NSArray *)files tag:(int *)tag;

- (BOOL)getFileSystemInfoForPath:(NSString *)path isRemovable:(BOOL *)isRemovable isWritable:(BOOL *)isWritable isUnmountable:(BOOL *)isUnmountable description:(NSString **)description type:(NSString **)type;

- (BOOL)getInfoForFile:(NSString *)path application:(NSString **)application type:(NSString **)type;

- (void)checkForRemovableMedia;
- (NSArray *)mountNewRemovableMedia;
- (NSArray *)mountedRemovableMedia;
- (NSArray *)mountedLocalVolumePaths;

- (BOOL)unmountAndEjectDeviceAtPath:(NSString *)path;

- (BOOL)fileSystemChanged;
- (BOOL)userDefaultsChanged;

- (void)noteFileSystemChanged;
- (void)noteFileSystemChanged:(NSString *)path;
- (void)noteUserDefaultsChanged;

- (BOOL)isFilePackageAtPath:(NSString *)path;
- (NSString *)absolutePathForAppBundleWithIdentifier:(NSString *)identifier;
- (NSString *)pathForApplication:(NSString *)application;
- (NSArray *)launchedApplications;
- (BOOL)launchApplication:(NSString *)application;
- (BOOL)launchApplication:(NSString *)application showIcon:(BOOL)showIcon autolaunch:(BOOL)autolaunch;

- (void)findApplications;
- (NSDictionary *)activeApplication;
- (void)hideOtherApplications;

- (int)extendPowerOffBy:(int)milliseconds;

@end

@interface NSWorkspace (CocotronAdditions)

// Cocoa should provide a simple method for this - but it doesn't. You have to build it with Launch Services
// But so many files are typically hidden from users in GUI browsers that this just makes life better for everyone...
- (BOOL)isFileHiddenAtPath:(NSString *)path;

@end
