/* 
   NSWorkspace.h

   Interface for workspace.

   Copyright (C) 1996-2016 Free Software Foundation, Inc.

   Author:  Scott Christley <scottc@net-community.com>
   Date: 1996
   
   This file is part of the GNUstep GUI Library.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; see the file COPYING.LIB.
   If not, see <http://www.gnu.org/licenses/> or write to the 
   Free Software Foundation, 51 Franklin Street, Fifth Floor, 
   Boston, MA 02110-1301, USA.
*/ 

#ifndef _GNUstep_H_NSWorkspace
#define _GNUstep_H_NSWorkspace

#import <Foundation/NSObject.h>
#import <Foundation/NSGeometry.h>
#import <AppKit/AppKitDefines.h>

#if OS_API_VERSION(MAC_OS_X_VERSION_10_3, GS_API_LATEST)
#import <Foundation/NSAppleEventDescriptor.h>
#endif

@class NSString;
@class NSNumber;
@class NSArray;
@class NSMutableArray;
@class NSMutableDictionary;
@class NSNotificationCenter;
@class NSImage;
@class NSView;
@class NSURL;


#if OS_API_VERSION(MAC_OS_X_VERSION_10_3, GS_API_LATEST)
enum {
  NSWorkspaceLaunchAndPrint                  = 0x2,
  NSWorkspaceLaunchInhibitingBackgroundOnly  = 0x80,
  NSWorkspaceLaunchWithoutAddingToRecents    = 0x100,
  NSWorkspaceLaunchWithoutActivation         = 0x200,
  NSWorkspaceLaunchAsync                     = 0x10000,
  NSWorkspaceLaunchAllowingClassicStartup    = 0x20000,
  NSWorkspaceLaunchPreferringClassic         = 0x40000,
  NSWorkspaceLaunchNewInstance               = 0x80000,
  NSWorkspaceLaunchAndHide                   = 0x100000,
  NSWorkspaceLaunchAndHideOthers             = 0x200000,
  NSWorkspaceLaunchDefault                   = NSWorkspaceLaunchAsync | NSWorkspaceLaunchAllowingClassicStartup
};

typedef NSUInteger NSWorkspaceLaunchOptions;
#endif

#if OS_API_VERSION(MAC_OS_X_VERSION_10_4, GS_API_LATEST)
enum {
  NSExcludeQuickDrawElementsIconCreationOption = 1 << 1,
  NSExclude10_4ElementsIconCreationOption = 1 << 2
};

typedef NSUInteger NSWorkspaceIconCreationOptions;
#endif

@interface NSWorkspace : NSObject
{
  NSMutableDictionary	*_iconMap;
  NSMutableDictionary	*_launched;
  NSNotificationCenter	*_workspaceCenter;
  BOOL			_fileSystemChanged;
  BOOL			_userDefaultsChanged;
}

//
// Creating a Workspace
//
+ (NSWorkspace*) sharedWorkspace;

//
// Opening Files
//
- (BOOL) openFile: (NSString*)fullPath;
- (BOOL) openFile: (NSString*)fullPath
	fromImage: (NSImage*)anImage
	       at: (NSPoint)point
	   inView: (NSView*)aView;
- (BOOL) openFile: (NSString*)fullPath
  withApplication: (NSString*)appName;
- (BOOL) openFile: (NSString*)fullPath
  withApplication: (NSString*)appName
    andDeactivate: (BOOL)flag;
- (BOOL) openTempFile: (NSString*)fullPath;
#if OS_API_VERSION(GS_API_MACOSX, GS_API_LATEST)
- (BOOL) openURL: (NSURL*)url;
#endif

//
// Manipulating Files	
//
- (BOOL) performFileOperation: (NSString*)operation
		       source: (NSString*)source
		  destination: (NSString*)destination
			files: (NSArray*)files
			  tag: (NSInteger*)tag;
- (BOOL) selectFile: (NSString*)fullPath
  inFileViewerRootedAtPath: (NSString*)rootFullpath;

//
// Requesting Information about Files
//
- (NSString*) fullPathForApplication: (NSString*)appName;
- (BOOL) getFileSystemInfoForPath: (NSString*)fullPath
		      isRemovable: (BOOL*)removableFlag
		       isWritable: (BOOL*)writableFlag
		    isUnmountable: (BOOL*)unmountableFlag
		      description: (NSString**)description
			     type: (NSString**)fileSystemType;
- (BOOL) getInfoForFile: (NSString*)fullPath
	    application: (NSString**)appName
		   type: (NSString**)type;
- (NSImage*) iconForFile: (NSString*)fullPath;
- (NSImage*) iconForFiles: (NSArray*)pathArray;
- (NSImage*) iconForFileType: (NSString*)fileType;
#if OS_API_VERSION(GS_API_MACOSX, GS_API_LATEST)
- (BOOL) isFilePackageAtPath: (NSString*)fullPath;
#endif
#if OS_API_VERSION(MAC_OS_X_VERSION_10_3, GS_API_LATEST)
- (BOOL) setIcon: (NSImage *)image
         forFile: (NSString *)fullPath
         options: (NSWorkspaceIconCreationOptions)options;
#endif

//
// Tracking Changes to the File System
//
- (BOOL) fileSystemChanged;
- (void) noteFileSystemChanged;
#if OS_API_VERSION(GS_API_MACOSX, GS_API_LATEST)
- (void) noteFileSystemChanged: (NSString*)path;
#endif

//
// Updating Registered Services and File Types
//
- (void) findApplications;

//
// Launching and Manipulating Applications	
//
- (void) hideOtherApplications;
- (BOOL) launchApplication: (NSString*)appName;
- (BOOL) launchApplication: (NSString*)appName
		  showIcon: (BOOL)showIcon
		autolaunch: (BOOL)autolaunch;

#if OS_API_VERSION(MAC_OS_X_VERSION_10_3, GS_API_LATEST)
- (NSString *) absolutePathForAppBundleWithIdentifier: (NSString *)bundleIdentifier;

- (BOOL) launchAppWithBundleIdentifier: (NSString *)bundleIdentifier
			       options: (NSWorkspaceLaunchOptions)options 
	additionalEventParamDescriptor: (NSAppleEventDescriptor *)descriptor 
		      launchIdentifier: (NSNumber **)identifier;
- (BOOL) openURLs: (NSArray *)urls
withAppBundleIdentifier: (NSString *)bundleIdentifier
          options: (NSWorkspaceLaunchOptions)options
additionalEventParamDescriptor: (NSAppleEventDescriptor *)descriptor
launchIdentifiers: (NSArray **)identifiers;
#endif

#if OS_API_VERSION(MAC_OS_X_VERSION_10_5, GS_API_LATEST)
- (BOOL) filenameExtension: (NSString *)filenameExtension 
            isValidForType: (NSString*)typeName;
- (NSString *) localizedDescriptionForType: (NSString *)typeName;
- (NSString *) preferredFilenameExtensionForType: (NSString *)typeName;
- (BOOL) type: (NSString *)firstTypeName 
conformsToType: (NSString *)secondTypeName;
- (NSString *) typeOfFile: (NSString *)absoluteFilePath 
                    error: (NSError **)outError;
#endif

#if OS_API_VERSION(MAC_OS_X_VERSION_10_2, GS_API_LATEST)
- (NSDictionary*) activeApplication;
- (NSArray*) launchedApplications;
#endif

//
// Unmounting a Device	
//
- (BOOL) unmountAndEjectDeviceAtPath: (NSString*)path;

//
// Tracking Status Changes for Devices
//
- (void) checkForRemovableMedia;
- (NSArray*) mountNewRemovableMedia;
- (NSArray*) mountedRemovableMedia;
#if OS_API_VERSION(GS_API_MACOSX, GS_API_LATEST)
- (NSArray*) mountedLocalVolumePaths;
#endif

//
// Notification Center
//
- (NSNotificationCenter*) notificationCenter;

//
// Tracking Changes to the User Defaults Database
//
- (void) noteUserDefaultsChanged;
- (BOOL) userDefaultsChanged;

//
// Animating an Image	
//
- (void) slideImage: (NSImage*)image
	       from: (NSPoint)fromPoint
		 to: (NSPoint)toPoint;

//
// Requesting Additional Time before Power Off or Logout
//
- (int) extendPowerOffBy: (int)requested;

@end

#if OS_API_VERSION(GS_API_NONE, GS_API_NONE)

@class NSBundle;

@interface	NSWorkspace (GNUstep)
- (NSString*) getBestAppInRole: (NSString*)role
		  forExtension: (NSString*)ext;
- (NSString*) getBestIconForExtension: (NSString*)ext;
- (NSDictionary*) infoForExtension: (NSString*)ext;
- (NSBundle*) bundleForApp:(NSString*)appName;
- (NSImage*) appIconForApp:(NSString*)appName;
- (NSString*) locateApplicationBinary: (NSString*)appName;
- (void) setBestApp: (NSString*)appName
	     inRole: (NSString*)role
       forExtension: (NSString*)ext;
- (void) setBestIcon: (NSString*)iconPath forExtension: (NSString*)ext;
- (NSDictionary*) infoForScheme: (NSString*)scheme;
- (NSString*) getBestAppInRole: (NSString*)role
		     forScheme: (NSString*)scheme;
- (void) setBestApp: (NSString*)appName
	     inRole: (NSString*)role
	  forScheme: (NSString*)scheme;
@end
#endif

/* Notifications */

/**
 * This notification is sent by applications when they launch,
 * the notification userInfo dictionary contains the following -
 * <deflist>
 * <term>NSApplicationName</term>
 * <desc>The name of the launched application.
 * A string.
 * </desc>
 * <term>NSApplicationPath</term>
 * <desc>The full path to the launched application.
 * A string.
 * </desc>
 * <term>NSApplicationProcessIdentifier</term>
 * <desc>The process identifier (pid) of the launched application.
 * </desc>
 * <term>NSApplicationProcessSerialNumberHigh</term>
 * <desc>MacOS-X specific ... not present in GNUstep.
 * </desc>
 * <term>NSApplicationProcessSerialNumberLow</term>
 * <desc>MacOS-X specific ... not present in GNUstep.
 * </desc>
 * </deflist>
 */
APPKIT_EXPORT NSString *NSWorkspaceDidLaunchApplicationNotification;

APPKIT_EXPORT NSString *NSWorkspaceDidMountNotification;
APPKIT_EXPORT NSString *NSWorkspaceDidPerformFileOperationNotification;
APPKIT_EXPORT NSString *NSWorkspaceDidTerminateApplicationNotification;
APPKIT_EXPORT NSString *NSWorkspaceDidUnmountNotification;
APPKIT_EXPORT NSString *NSWorkspaceWillLaunchApplicationNotification;
APPKIT_EXPORT NSString *NSWorkspaceWillPowerOffNotification;
APPKIT_EXPORT NSString *NSWorkspaceWillUnmountNotification;
#if OS_API_VERSION(MAC_OS_X_VERSION_10_3, GS_API_LATEST)
APPKIT_EXPORT NSString *NSWorkspaceDidWakeNotification;
APPKIT_EXPORT NSString *NSWorkspaceSessionDidBecomeActiveNotification;
APPKIT_EXPORT NSString *NSWorkspaceSessionDidResignActiveNotification;
APPKIT_EXPORT NSString *NSWorkspaceWillSleepNotification;
#endif

//
// Workspace File Type Globals 
//
APPKIT_EXPORT NSString *NSPlainFileType;
APPKIT_EXPORT NSString *NSDirectoryFileType;
APPKIT_EXPORT NSString *NSApplicationFileType;
APPKIT_EXPORT NSString *NSFilesystemFileType;
APPKIT_EXPORT NSString *NSShellCommandFileType;

//
// Workspace File Operation Globals 
//
APPKIT_EXPORT NSString *NSWorkspaceCompressOperation;
APPKIT_EXPORT NSString *NSWorkspaceCopyOperation;
APPKIT_EXPORT NSString *NSWorkspaceDecompressOperation;
APPKIT_EXPORT NSString *NSWorkspaceDecryptOperation;
APPKIT_EXPORT NSString *NSWorkspaceDestroyOperation;
APPKIT_EXPORT NSString *NSWorkspaceDuplicateOperation;
APPKIT_EXPORT NSString *NSWorkspaceEncryptOperation;
APPKIT_EXPORT NSString *NSWorkspaceLinkOperation;
APPKIT_EXPORT NSString *NSWorkspaceMoveOperation;
APPKIT_EXPORT NSString *NSWorkspaceRecycleOperation;

#endif // _GNUstep_H_NSWorkspace
