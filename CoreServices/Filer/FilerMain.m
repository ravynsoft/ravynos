/*
 * FilerMain.m - ravynOS Desktop File Manager (Filer.app)
 *
 * Provides desktop file browsing, bundle execution via LaunchServices,
 * and drag-and-drop .app bundle installation into /Applications.
 *
 * Copyright (C) 2026 Jack Davenport. All rights reserved.
 * SPDX-License-Identifier: BSD-2-Clause
 */

#import <Cocoa/Cocoa.h>
#import <CoreServices/CoreServices.h>

@interface FilerAppDelegate : NSObject <NSApplicationDelegate, NSWindowDelegate>
@property (strong) NSWindow *window;
@property (strong) NSString *currentPath;
@end

@implementation FilerAppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
    self.currentPath = @"/Applications";

    NSRect frame = NSMakeRect(200, 200, 800, 500);
    NSUInteger style = NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable | NSWindowStyleMaskResizable;
    self.window = [[NSWindow alloc] initWithContentRect:frame styleMask:style backing:NSBackingStoreBuffered defer:NO];
    [self.window setTitle:@"Filer - /Applications"];
    [self.window setDelegate:self];
    [self.window makeKeyAndOrderFront:nil];

    // Register drag and drop for application bundles and files
    [self.window registerForDraggedTypes:@[NSPasteboardTypeFileURL, NSPasteboardTypeString]];
}

- (BOOL)installBundleFromPath:(NSString *)sourcePath toDirectory:(NSString *)destDir {
    NSFileManager *fm = [NSFileManager defaultManager];
    NSString *fileName = [sourcePath lastPathComponent];
    NSString *targetPath = [destDir stringByAppendingPathComponent:fileName];

    NSError *error = nil;
    if ([fm fileExistsAtPath:targetPath]) {
        [fm removeItemAtPath:targetPath error:nil];
    }

    BOOL success = [fm copyItemAtPath:sourcePath toPath:targetPath error:&error];
    if (success) {
        NSLog(@"Filer: Installed bundle %@ to %@", fileName, targetPath);
    } else {
        NSLog(@"Filer: Error installing bundle %@: %@", fileName, [error localizedDescription]);
    }
    return success;
}

- (void)openItemAtPath:(NSString *)itemPath {
    BOOL isDir = NO;
    if ([[NSFileManager defaultManager] fileExistsAtPath:itemPath isDirectory:&isDir]) {
        if ([itemPath hasSuffix:@".app"] || [itemPath hasSuffix:@".APP"]) {
            CFURLRef url = (__bridge CFURLRef)[NSURL fileURLWithPath:itemPath];
            LSOpenCFURLRef(url, NULL);
        }
    }
}

- (NSDragOperation)draggingEntered:(id<NSDraggingInfo>)sender {
    NSPasteboard *pboard = [sender draggingPasteboard];
    if ([[pboard types] containsObject:NSPasteboardTypeFileURL]) {
        return NSDragOperationCopy;
    }
    return NSDragOperationNone;
}

- (BOOL)performDragOperation:(id<NSDraggingInfo>)sender {
    NSPasteboard *pboard = [sender draggingPasteboard];
    if ([[pboard types] containsObject:NSPasteboardTypeFileURL]) {
        NSArray *urls = [pboard readObjectsForClasses:@[[NSURL class]] options:nil];
        for (NSURL *url in urls) {
            [self installBundleFromPath:[url path] toDirectory:self.currentPath];
        }
        return YES;
    }
    return NO;
}

@end

int main(int argc, const char *argv[]) {
    @autoreleasepool {
        NSApplication *app = [NSApplication sharedApplication];
        FilerAppDelegate *delegate = [FilerAppDelegate new];
        [app setDelegate:delegate];
        [app run];
    }
    return 0;
}
