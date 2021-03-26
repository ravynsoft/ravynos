/* Copyright (c) 2006-2007 Christopher J. W. Lloyd <cjwl@objc.net>

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <AppKit/NSWorkspace.h>
#import <AppKit/NSRaise.h>

NSString * const NSWorkspaceWillPowerOffNotification=@"NSWorkspaceWillPowerOffNotification";

NSString * const NSWorkspaceRecycleOperation=@"NSWorkspaceRecycleOperation";

@implementation NSWorkspace

+(NSWorkspace *)sharedWorkspace {
   return NSThreadSharedInstance(@"NSWorkspace");
}

-init {
   _notificationCenter=[[NSNotificationCenter alloc] init];
   return self;
}

-(NSNotificationCenter *)notificationCenter {
   return _notificationCenter;
}

-(NSImage *)iconForFile:(NSString *)path {
   NSInvalidAbstractInvocation();
   return 0;
}

-(NSImage *)iconForFiles:(NSArray *)array {
   NSInvalidAbstractInvocation();
   return 0;
}

-(NSImage *)iconForFileType:(NSString *)type {
   NSInvalidAbstractInvocation();
   return 0;
}

-(NSString *)localizedDescriptionForType:(NSString *)type {
   NSInvalidAbstractInvocation();
   return 0;
}

-(BOOL)filenameExtension:(NSString *)extension isValidForType:(NSString *)type {
   NSInvalidAbstractInvocation();
   return 0;
}

-(NSString *)preferredFilenameExtensionForType:(NSString *)type {
   NSInvalidAbstractInvocation();
   return 0;
}

-(BOOL)type:(NSString *)type conformsToType:(NSString *)conformsToType {
   NSInvalidAbstractInvocation();
   return 0;
}

-(NSString *)typeOfFile:(NSString *)path error:(NSError **)error {
   NSInvalidAbstractInvocation();
   return 0;
}

-(BOOL)openFile:(NSString *)path {
   NSInvalidAbstractInvocation();
   return 0;
}

-(BOOL)openFile:(NSString *)path withApplication:(NSString *)application {
   NSInvalidAbstractInvocation();
   return 0;
}

-(BOOL)openTempFile:(NSString *)path {
   NSInvalidAbstractInvocation();
   return 0;
}

-(BOOL)openFile:(NSString *)path fromImage:(NSImage *)image at:(NSPoint)point inView:(NSView *)view {
   NSInvalidAbstractInvocation();
   return 0;
}

-(BOOL)openFile:(NSString *)path withApplication:(NSString *)application andDeactivate:(BOOL)deactivate {
   NSInvalidAbstractInvocation();
   return 0;
}

-(BOOL)openURL:(NSURL *)url {
   NSInvalidAbstractInvocation();
   return 0;
}

-(BOOL)selectFile:(NSString *)path inFileViewerRootedAtPath:(NSString *)rootedAtPath {
   NSInvalidAbstractInvocation();
   return 0;
}

-(void)slideImage:(NSImage *)image from:(NSPoint)from to:(NSPoint)to {
   NSInvalidAbstractInvocation();
}

-(BOOL)performFileOperation:(NSString *)operation source:(NSString *)source destination:(NSString *)destination files:(NSArray *)files tag:(int *)tag {
   NSInvalidAbstractInvocation();
   return 0;
}

-(BOOL)getFileSystemInfoForPath:(NSString *)path isRemovable:(BOOL *)isRemovable isWritable:(BOOL *)isWritable isUnmountable:(BOOL *)isUnmountable description:(NSString **)description type:(NSString **)type {
   NSInvalidAbstractInvocation();
   return 0;
}

-(BOOL)getInfoForFile:(NSString *)path application:(NSString **)application type:(NSString **)type {
   NSInvalidAbstractInvocation();
   return 0;
}

-(void)checkForRemovableMedia {
   NSInvalidAbstractInvocation();
}

-(NSArray *)mountNewRemovableMedia {
   NSInvalidAbstractInvocation();
   return 0;
}

-(NSArray *)mountedRemovableMedia {
   NSInvalidAbstractInvocation();
   return 0;
}

-(NSArray *)mountedLocalVolumePaths {
   NSInvalidAbstractInvocation();
   return 0;
}

-(BOOL)unmountAndEjectDeviceAtPath:(NSString *)path {
   NSInvalidAbstractInvocation();
   return 0;
}

-(BOOL)fileSystemChanged {
   NSInvalidAbstractInvocation();
   return 0;
}

-(BOOL)userDefaultsChanged {
   NSInvalidAbstractInvocation();
   return 0;
}

-(void)noteFileSystemChanged {
   NSInvalidAbstractInvocation();
}

-(void)noteFileSystemChanged:(NSString *)path {
   NSInvalidAbstractInvocation();
}

-(void)noteUserDefaultsChanged {
   NSInvalidAbstractInvocation();
}

-(BOOL)isFilePackageAtPath:(NSString *)path {
   NSInvalidAbstractInvocation();
   return 0;
}

-(NSString *)absolutePathForAppBundleWithIdentifier:(NSString *)identifier {
   NSInvalidAbstractInvocation();
   return 0;
}

-(NSString *)pathForApplication:(NSString *)application {
   NSInvalidAbstractInvocation();
   return 0;
}

-(NSArray *)launchedApplications {
   NSInvalidAbstractInvocation();
   return 0;
}

-(BOOL)launchApplication:(NSString *)application {
   NSInvalidAbstractInvocation();
   return 0;
}

-(BOOL)launchApplication:(NSString *)application showIcon:(BOOL)showIcon autolaunch:(BOOL)autolaunch {
   NSInvalidAbstractInvocation();
   return 0;
}

-(void)findApplications {
   NSInvalidAbstractInvocation();
}

-(NSDictionary *)activeApplication {
   NSInvalidAbstractInvocation();
   return 0;
}

-(void)hideOtherApplications {
   NSInvalidAbstractInvocation();
}

-(int)extendPowerOffBy:(int)milliseconds {
   NSInvalidAbstractInvocation();
   return 0;
}

@end

@implementation NSWorkspace (CocotronAdditions)

- (BOOL)isFileHiddenAtPath:(NSString*)path
{
	return NO;
}

@end

