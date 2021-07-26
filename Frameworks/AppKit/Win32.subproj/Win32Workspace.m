/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Onyx2D/O2Context_gdi.h>
#import <AppKit/Win32Workspace.h>
#import <Foundation/NSString_win32.h>
#import <AppKit/NSBitmapImageRep.h>
#import <AppKit/NSColor.h>
#import <AppKit/NSImage.h>
#import <windows.h>
#import <commctrl.h>
#import <shellapi.h>
#import <AppKit/NSRaise.h>

static NSString *DriveLetterInPath(NSString* path)
{
	NSArray* components = [path componentsSeparatedByString: @":"];
	if ([components count] > 0) {
		NSString* driveLetter = [NSString stringWithFormat: @"%@:\\", [components objectAtIndex: 0]];
		return driveLetter;
	}
	return nil;
}

@implementation NSWorkspace(windows)

+allocWithZone:(NSZone *)zone {
   return NSAllocateObject([Win32Workspace class],0,NULL);
}

@end

@implementation Win32Workspace

-(NSArray *)mountedLocalVolumePaths {
   NSMutableArray *result=[NSMutableArray array];
   DWORD           driveMask=GetLogicalDrives();
   unichar         drive='A';
   
   for(;driveMask!=0;driveMask>>=1,drive++){
    if(driveMask&0x1)
     [result addObject:[NSString stringWithFormat:@"%C:",drive]];
   }
   
   return result;
}

-(BOOL)getFileSystemInfoForPath:(NSString *)path isRemovable:(BOOL *)isRemovable isWritable:(BOOL *)isWritable isUnmountable:(BOOL *)isUnmountable description:(NSString **)description type:(NSString **)type
{
	*isWritable = YES;
	*isRemovable = NO;
	*isUnmountable = NO;
	*type = NSLocalizedStringFromTableInBundle(@"WinFS", nil, [NSBundle bundleForClass: [Win32Workspace class]], @"");
	
	NSString* driveLetter = DriveLetterInPath(path);
	
	UINT driveType = GetDriveTypeW([driveLetter fileSystemRepresentationW]);
	switch (driveType) {
		case DRIVE_UNKNOWN:
			*description = NSLocalizedStringFromTableInBundle(@"Unrecognized drive", nil, [NSBundle bundleForClass: [Win32Workspace class]], @"Error description for a disk drive");
			break;
		case DRIVE_NO_ROOT_DIR:
			*description = NSLocalizedStringFromTableInBundle(@"Corrupted drive", nil, [NSBundle bundleForClass: [Win32Workspace class]], @"Error description for a disk drive");
			break;
		case DRIVE_REMOVABLE:
			*description = NSLocalizedStringFromTableInBundle(@"Removable drive", nil, [NSBundle bundleForClass: [Win32Workspace class]], @"Describes a removable disk drive");
			*isRemovable = YES;
			break;
		case DRIVE_FIXED:
			*description = NSLocalizedStringFromTableInBundle(@"Fixed drive", nil, [NSBundle bundleForClass: [Win32Workspace class]], @"Describes a standard hard disk drive");
			break;
		case DRIVE_REMOTE:
			*description = NSLocalizedStringFromTableInBundle(@"Network drive", nil, [NSBundle bundleForClass: [Win32Workspace class]], @"Describes a disk drive accessable over a network");
			*isUnmountable = YES;
			break;
		case DRIVE_CDROM:
			*description = NSLocalizedStringFromTableInBundle(@"CD-ROM drive", nil, [NSBundle bundleForClass: [Win32Workspace class]], @"Describes a CD-ROM drive");
			*isUnmountable = YES;
			*isRemovable = YES;
			break;
		case DRIVE_RAMDISK:
			*description = NSLocalizedStringFromTableInBundle(@"RAM drive", nil, [NSBundle bundleForClass: [Win32Workspace class]], @"Describes a RAM drive");
			*isUnmountable = YES;
			break;
	}
	return driveType >= DRIVE_REMOVABLE;
}

-(BOOL)openURL:(NSURL *)url {
   return ((int)ShellExecuteW(GetDesktopWindow(),L"open",(const unichar *)[[url absoluteString] cStringUsingEncoding:NSUnicodeStringEncoding],NULL,NULL,SW_SHOWNORMAL)<=32)?NO:YES;
}

-(BOOL)openFile:(NSString *)path {
   NSString *extension=[path pathExtension];
   
   if([extension isEqualToString:@"app"]){
    NSString *name=[[path lastPathComponent] stringByDeletingPathExtension];
    
    path=[path stringByAppendingPathComponent:@"Contents"];
    path=[path stringByAppendingPathComponent:@"Windows"];
    path=[path stringByAppendingPathComponent:name];
    path=[path stringByAppendingPathExtension:@"exe"];
   }
   
   return ((int)ShellExecuteW(GetDesktopWindow(),L"open",[path fileSystemRepresentationW],NULL,NULL,SW_SHOWNORMAL)<=32)?NO:YES;
}

static BOOL openFileWithHelpViewer(const char *helpFilePath)
{
   char buf[1024];
   snprintf(buf, sizeof(buf), "hh.exe %s", helpFilePath);
   return ((int)WinExec(buf, SW_SHOWNORMAL)<=32)?NO:YES;
}

-(BOOL)openFile:(NSString *)path withApplication:(NSString *)appName {
#if 1
   if(!strcmp([appName UTF8String], "Help Viewer"))
   {
		return openFileWithHelpViewer([path fileSystemRepresentation]);
   }
   else
   {
    NSBundle *bundle=[NSBundle bundleForClass:isa];
    NSString *bundlePath=[bundle bundlePath];
    NSString *app=[[[[[[[bundlePath stringByDeletingLastPathComponent] stringByDeletingLastPathComponent] stringByAppendingPathComponent:@"Applications"] stringByAppendingPathComponent:appName] stringByAppendingPathExtension:@"app"] stringByAppendingPathComponent:appName] stringByAppendingPathExtension:@"exe"];
    NSMutableData *args=[NSMutableData data];

    [args appendData:NSTaskArgumentDataFromStringW(@"-NSOpen")];
    [args appendBytes:L" " length:2];
    [args appendData:NSTaskArgumentDataFromStringW(path)];
    [args appendBytes:L"\0" length:2];

    return ((int)ShellExecuteW(GetDesktopWindow(),L"open",[app fileSystemRepresentationW],[args bytes],NULL,SW_SHOWNORMAL)<=32)?NO:YES;
   }
#else
   return ((int)ShellExecuteW(GetDesktopWindow(),L"open",[path fileSystemRepresentationW],NULL,NULL,SW_SHOWNORMAL)<=32)?NO:YES;
#endif
}

-(BOOL)openTempFile:(NSString *)path {
   return ((int)ShellExecuteW(GetDesktopWindow(),L"open",[path fileSystemRepresentationW],NULL,NULL,SW_SHOWNORMAL)<=32)?NO:YES;
}

-(BOOL)selectFile:(NSString *)path inFileViewerRootedAtPath:(NSString *)rootFullpath {
	BOOL isDir = NO;
	
	if ([[NSFileManager defaultManager] fileExistsAtPath:path isDirectory:&isDir])
	{
		NSString  *win32Path = NSStringFromNullTerminatedUnicode([path fileSystemRepresentationW]);

		NSMutableData *args=[NSMutableData data];
		[args appendBytes:L"/select," length:16];
		[args appendData: NSTaskArgumentDataFromStringW(win32Path)];
		[args appendBytes:L"\0" length:2];

		return ((int)ShellExecuteW(GetDesktopWindow(),L"open",L"explorer",[args bytes],NULL,SW_SHOWNORMAL)<=32)?NO:YES;
	}
	return NO;
}

-(int)extendPowerOffBy:(int)seconds {
   NSUnimplementedMethod ();
   return 0;
}

-(void)slideImage:(NSImage *)image from:(NSPoint)fromPoint to:(NSPoint)toPoint {
   NSUnimplementedMethod();
}

static NSImageRep *imageRepForIcon(HICON icon) {
	// Create a bitmap context, and draw the icon into its DC
	NSImageRep *imageRep = nil;
	if (icon != NULL) {
		ICONINFO iconInfo;
		GetIconInfo(icon, &iconInfo);
		BITMAP bmp;
		GetObject(iconInfo.hbmColor, sizeof(BITMAP), (void *) &bmp);
		int w = bmp.bmWidth;
		int h = bmp.bmHeight;
		DeleteObject(iconInfo.hbmMask);
		DeleteObject(iconInfo.hbmColor);

		if (w > 0 && h > 0) {
			CGColorSpaceRef colorspace = CGColorSpaceCreateDeviceRGB();
			CGContextRef ctx = CGBitmapContextCreate(NULL, w, h, 8, 4*w, colorspace, kCGBitmapByteOrder32Little|kCGImageAlphaPremultipliedFirst);
			CGColorSpaceRelease(colorspace);
			// Contexts created on the Win32 platform are supposed to have an "dc" method
			HDC dc = [(id)ctx dc];
			if (dc) {
				DrawIcon(dc, 0, 0, icon);
				CGImageRef image = CGBitmapContextCreateImage(ctx);
				if (image) {
					imageRep = [[[NSBitmapImageRep alloc] initWithCGImage: image] autorelease];
					CGImageRelease(image);
				}
			}
			CGContextRelease(ctx);
		}
	}
	return imageRep;
}

static NSImageRep *imageRepForImageListAndIndex(HIMAGELIST imageListH, int index) {
	// Create a bitmap context, and draw the image into its DC
	NSImageRep *imageRep = nil;
	if (imageListH != NULL) {
		int w, h;
		if (ImageList_GetIconSize(imageListH, &w, &h)) {
			CGColorSpaceRef colorspace = CGColorSpaceCreateDeviceRGB();
			CGContextRef ctx = CGBitmapContextCreate(NULL, w, h, 8, 4*w, colorspace, kCGBitmapByteOrder32Little|kCGImageAlphaPremultipliedFirst);
			CGColorSpaceRelease(colorspace);
			// Contexts created on the Win32 platform are supposed to have an "dc" method
			HDC dc = [(id)ctx dc];
			if (dc) {
				ImageList_Draw(imageListH, index, dc, 0, 0, ILD_TRANSPARENT);
				CGImageRef image = CGBitmapContextCreateImage(ctx);
				if (image) {
					imageRep = [[[NSBitmapImageRep alloc] initWithCGImage: image] autorelease];
					CGImageRelease(image);
				}
			}
			CGContextRelease(ctx);
        }
    }
	return imageRep;
}

-(NSImage *)iconForFile:(NSString *)path {
    // This code seems very sensitive to some memory corruption happening (stack trashing it seems)
    // Just moving some of the local var or some code  order seems to trigger that either in Debug or
    // Release mode.
    // So maybe there must be something wrong with the use of these API
    
	// Some pointers to some needed SHELL32 functions
	 static HRESULT  (*SHGetImageListPtr)(
								  int iImageList,
								  REFIID riid,
								  HIMAGELIST *imageList
								  ) = NULL;
	 static HRESULT  (*FileIconInitPtr)(BOOL restoreCache) = NULL;
			
	if (FileIconInitPtr == NULL) {
        HANDLE library = LoadLibrary("SHELL32");
        FileIconInitPtr = (void*)GetProcAddress(library,(char *)660); // 660 is the magic number for it
        if (FileIconInitPtr) {
            // Call the init function the first time we get called - MS says it should be done before using
            // SHGetImageList
            FileIconInitPtr(YES);
        }
        if (SHGetImageListPtr == NULL) {
            SHGetImageListPtr = (void*)GetProcAddress(library,"SHGetImageList");
            if (SHGetImageListPtr == NULL) {
                SHGetImageListPtr = (void*)GetProcAddress(library,(char *)727);
            }
        }
	}

	const unichar *pathCString=[path fileSystemRepresentationW];
	SHFILEINFOW fileInfo = { 0 };
	
	NSImage *icon=[[[NSImage alloc] init] autorelease];
	if (SHGetImageListPtr) {
		if(SHGetFileInfoW(pathCString, 0, &fileInfo, sizeof(SHFILEINFOW), SHGFI_SYSICONINDEX)) {
            HIMAGELIST imageList;

#ifndef SHIL_SMALL
#define SHIL_SMALL          0x1
#endif
#ifndef SHIL_LARGE
#define SHIL_LARGE          0x0
#endif
#ifndef SHIL_EXTRALARGE
#define SHIL_EXTRALARGE		0x2
#endif
#ifndef SHIL_JUMBO
#define SHIL_JUMBO			0x4
#endif

            OSVERSIONINFOEX osVersion = { 0 };
            osVersion.dwOSVersionInfoSize=sizeof(osVersion);
            GetVersionEx((OSVERSIONINFO *)&osVersion);
            BOOL isRunningVistaOrBetter = osVersion.dwMajorVersion >= 6;

            int imageIndex = fileInfo.iIcon;

			static const IID IID_IImageList = {0x46EB5926L,0x582E,0x4017, {0x9F,0xDF,0xE8,0x99,0x8D,0xAA,0x09,0x50} };
			if (isRunningVistaOrBetter) {
				if (SHGetImageListPtr(SHIL_JUMBO, &IID_IImageList, &imageList) == S_OK) {
					NSImageRep* rep = imageRepForImageListAndIndex(imageList, imageIndex);
					if (rep) {
                        [icon addRepresentation: rep];
                   }
				}
			}
			if (SHGetImageListPtr(SHIL_EXTRALARGE, &IID_IImageList, &imageList) == S_OK) {
				NSImageRep* rep = imageRepForImageListAndIndex(imageList, imageIndex);
                if (rep) {
                    [icon addRepresentation: rep];
                }
			}
            if (SHGetImageListPtr(SHIL_LARGE, &IID_IImageList, &imageList) == S_OK) {
				NSImageRep* rep = imageRepForImageListAndIndex(imageList, imageIndex);
                if (rep) {
                    [icon addRepresentation: rep];
                }
			}
            if (SHGetImageListPtr(SHIL_SMALL, &IID_IImageList, &imageList) == S_OK) {
				NSImageRep* rep = imageRepForImageListAndIndex(imageList, imageIndex);
                if (rep) {
                    [icon addRepresentation: rep];
                }
			}

		}
	}
	if([[icon representations] count]==0) {
        // Do it the old way
        
        // Try to add the regular size (32 and 16) icons
        if(SHGetFileInfoW(pathCString, 0, &fileInfo, sizeof(SHFILEINFOW), SHGFI_ICON|SHGFI_LARGEICON)) {
            if (fileInfo.hIcon) {
                NSImageRep* rep = imageRepForIcon(fileInfo.hIcon);
                if (rep) {
                    [icon addRepresentation: rep];
                }
                DeleteObject(fileInfo.hIcon);
            }
        }

        if(SHGetFileInfoW(pathCString, 0, &fileInfo, sizeof(SHFILEINFOW), SHGFI_ICON|SHGFI_SMALLICON)) {
            if (fileInfo.hIcon) {
                NSImageRep* rep = imageRepForIcon(fileInfo.hIcon);
                if (rep) {
                    [icon addRepresentation: rep];
                }
                DeleteObject(fileInfo.hIcon);
            }
        }
    }

    if([[icon representations] count]==0) {
		NSLog(@"unable to load icon for file: %@", path);
		return nil;
	}
	return icon;
}

- (BOOL)isFileHiddenAtPath:(NSString*)path
{
	static NSArray* hiddenSuffixes = nil;
	if (hiddenSuffixes == nil) {
		hiddenSuffixes = [[NSArray arrayWithObjects: @".ini", @".INI" @".dat", @".DAT" @".log", @".LOG" @".sys", @".SYS", @".bat", @".BAT", @".db", @".DB", @"NetHood", @"PrintHood", nil] retain];
	}
	for (id suffix in hiddenSuffixes) {
		if ([path hasSuffix: suffix]) {
			return YES;
		}
	}
	return NO;
}

@end
