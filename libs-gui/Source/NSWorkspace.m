/** <title>NSWorkspace</title>

   <abstract>Workspace class</abstract>

   Copyright (C) 1996-2016 Free Software Foundation, Inc.

   Author: Scott Christley <scottc@net-community.com>
   Date: 1996
   Implementation by: Richard Frith-Macdonald <richard@brainstorm.co.uk>
   Date: 1998
   Implementation by: Fred Kiefer <FredKiefer@gmx.de>
   Date: 2001
   
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

#import "config.h"

#include <unistd.h>
#include <sys/types.h>

#if defined(HAVE_GETMNTINFO)
#include <sys/param.h>
#include <sys/mount.h>
#endif

#if defined(HAVE_GETMNTENT) && defined (MNT_MEMB)
  #if defined(HAVE_MNTENT_H)
  #include <mntent.h>
  #elif defined(HAVE_SYS_MNTENT_H)
  #include <sys/mntent.h>
  #else
  #undef HAVE_GETMNTENT
  #endif
#endif /* HAVE_GETMNTENT */

#if defined (HAVE_SYS_STATVFS_H)
#include <sys/statvfs.h>
#endif
#if defined (HAVE_SYS_VFS_H)
#include <sys/vfs.h>
  #ifdef __linux__
  #include <linux/magic.h>
  #endif
#endif

/* FIXME Solaris uses /etc/mnttab instead of /etc/mtab, but defines
 * MNTTAB to that path.
 * FIXME We won't get here on Solaris at all because it defines the
 * mntent struct in sys/mnttab.h instead of sys/mntent.h.
 */
# ifdef _PATH_MOUNTED
#  define MOUNTED_PATH _PATH_MOUNTED
# elif defined(MOUNTED)
#  define MOUNTED_PATH MOUNTED
# else
#  define MNTTAB "/etc/mtab"
#  warning "Mounted path file for you OS guessed to /etc/mtab";
# endif

#import <Foundation/NSBundle.h>
#import <Foundation/NSData.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSHost.h>
#import <Foundation/NSLock.h>
#import <Foundation/NSDistributedLock.h>
#import <Foundation/NSPathUtilities.h>
#import <Foundation/NSUserDefaults.h>
#import <Foundation/NSTask.h>
#import <GNUstepBase/NSTask+GNUstepBase.h>
#import <Foundation/NSException.h>
#import <Foundation/NSFileManager.h>
#import <Foundation/NSNotificationQueue.h>
#import <Foundation/NSDistributedNotificationCenter.h>
#import <Foundation/NSConnection.h>
#import <Foundation/NSDebug.h>
#import <Foundation/NSProcessInfo.h>
#import <Foundation/NSThread.h>
#import <Foundation/NSURL.h>
#import <Foundation/NSValue.h>
#import "AppKit/NSWorkspace.h"
#import "AppKit/NSApplication.h"
#import "AppKit/NSImage.h"
#import "AppKit/NSPasteboard.h"
#import "AppKit/NSView.h"
#import "AppKit/NSPanel.h"
#import "AppKit/NSWindow.h"
#import "AppKit/NSScreen.h"
#import "GNUstepGUI/GSServicesManager.h"
#import "GNUstepGUI/GSDisplayServer.h"
#import "GSGuiPrivate.h"

/* Informal protocol for method to ask an app to open a URL.
 */
@interface NSObject (OpenURL)
- (BOOL) application: (NSApplication*)a openURL: (NSURL*)u;
@end

/* Private method to check that a process exists.
 */
@interface NSProcessInfo (Private)
+ (BOOL)_exists: (int)pid;
@end

#define PosixExecutePermission	(0111)

static NSMutableDictionary *folderPathIconDict = nil;
static NSMutableDictionary *folderIconCache = nil;

static NSImage	*folderImage = nil;
static NSImage	*multipleFiles = nil;
static NSImage	*unknownApplication = nil;
static NSImage	*unknownTool = nil;

static NSLock   *mlock = nil;

static NSString	*GSWorkspaceNotification = @"GSWorkspaceNotification";
static NSString *GSWorkspacePreferencesChanged =
    @"GSWorkspacePreferencesChanged";

/*
 * Depending on the 'active' flag this returns either the currently
 * active application or an array containing all launched apps.<br />
 * The 'notification' argument is either nil (simply query on disk
 * database) or a notification containing information to be used to
 * update the database.
 */
static id GSLaunched(NSNotification *notification, BOOL active)
{
  static NSString		*path = nil;
  static NSDistributedLock	*lock = nil;
  NSDictionary			*info = [notification userInfo];
  NSString			*mode = [notification name];
  NSMutableDictionary		*file = nil;
  NSString			*name;
  NSDictionary			*apps = nil;
  BOOL				modified = NO;
  unsigned	                sleeps = 0;

  [mlock lock]; // start critical section
  if (path == nil)
    {
      path = [NSTemporaryDirectory()
	stringByAppendingPathComponent: @"GSLaunchedApplications"];
      RETAIN(path);
      lock = [[NSDistributedLock alloc] initWithPath:
	[path stringByAppendingPathExtension: @"lock"]];
    }
  if ([lock tryLock] == NO)
    {
      /*
       * If the lock is really old ... assume the app has died and break it.
       */
      if ([[lock lockDate] timeIntervalSinceNow] < -20.0)
        {
	  NS_DURING
	    {
	      [lock breakLock];
	    }
	  NS_HANDLER
	    {
              NSLog(@"Unable to break lock %@ ... %@", lock, localException);
	    }
	  NS_ENDHANDLER
        }
      /*
       * Retry locking several times if necessary before giving up.
       */
      for (sleeps = 0; sleeps < 10; sleeps++)
	{
	  if ([lock tryLock] == YES)
	    {
	      break;
	    }
	  sleeps++;
	  [NSThread sleepUntilDate: [NSDate dateWithTimeIntervalSinceNow: 0.1]];
	}
      if (sleeps >= 10)
        {
          [mlock unlock];
          NSLog(@"Unable to obtain lock %@", lock);
          return nil;
	}
    }

  if ([[NSFileManager defaultManager] isReadableFileAtPath: path] == YES)
    {
      file = [NSMutableDictionary dictionaryWithContentsOfFile: path];
    }
  if (file == nil)
    {
      file = [NSMutableDictionary dictionaryWithCapacity: 2];
    }
  apps = [file objectForKey: @"GSLaunched"];
  if (apps == nil)
    {
      apps = [NSDictionary new];
      [file setObject: apps forKey: @"GSLaunched"];
      RELEASE(apps);
    }

  if (info != nil
    && (name = [info objectForKey: @"NSApplicationName"]) != nil)
    {
      NSDictionary	*oldInfo = [apps objectForKey: name];

      if ([mode isEqualToString:
	NSApplicationDidResignActiveNotification] == YES
	|| [mode isEqualToString:
	  NSWorkspaceDidTerminateApplicationNotification] == YES)
	{
	  if ([name isEqual: [file objectForKey: @"GSActive"]] == YES)
	    {
	      [file removeObjectForKey: @"GSActive"];
	      modified = YES;
	    }
	}
      else if ([mode isEqualToString:
	NSApplicationDidBecomeActiveNotification] == YES)
	{
	  if ([name isEqual: [file objectForKey: @"GSActive"]] == NO)
	    {
	      [file setObject: name forKey: @"GSActive"];
	      modified = YES;
	    }
	}

      if ([mode isEqualToString:
	NSWorkspaceDidTerminateApplicationNotification] == YES)
	{
	  if (oldInfo != nil)
	    {
	      NSMutableDictionary	*m = [apps mutableCopy];

	      [m removeObjectForKey: name];
	      [file setObject: m forKey: @"GSLaunched"];
	      apps = m;
	      RELEASE(m);
	      modified = YES;
	    }
	}
      else if ([mode isEqualToString:
	NSApplicationDidResignActiveNotification] == NO)
	{
	  if ([info isEqual: oldInfo] == NO)
	    {
	      NSMutableDictionary	*m = [apps mutableCopy];

	      [m setObject: info forKey: name];
	      [file setObject: m forKey: @"GSLaunched"];
	      apps = m;
	      RELEASE(m);
	      modified = YES;
	    }
	}
    }

  if (modified == YES)
    {
      [file writeToFile: path atomically: YES];
    }

  NS_DURING
    {
      sleeps = 0;
      [lock unlock];
    }
  NS_HANDLER
    {
      for (sleeps = 0; sleeps < 10; sleeps++)
	{
	  NS_DURING
	    {
	      [lock unlock];
	      NSLog(@"Unlocked %@", lock);
	      break;
	    }
	  NS_HANDLER
	    {
	      sleeps++;
	      if (sleeps >= 10)
		{
		  NSLog(@"Unable to unlock %@", lock);
		  break;
		}      
	      [NSThread sleepForTimeInterval: 0.1];
	      continue;
	    }
	  NS_ENDHANDLER;
	}
    }
  NS_ENDHANDLER;
  [mlock unlock];  // end critical section
  
  if (active == YES)
    {
      NSString	*activeName = [file objectForKey: @"GSActive"];

      if (activeName == nil)
	{
	  return nil;
	}
      return [apps objectForKey: activeName];
    }
  else
    {
      return [[file objectForKey: @"GSLaunched"] allValues];
    }
}

@interface	_GSWorkspaceCenter: NSNotificationCenter
{
  NSNotificationCenter	*remote;
}
- (void) _handleRemoteNotification: (NSNotification*)aNotification;
- (void) _postLocal: (NSString*)name userInfo: (NSDictionary*)info;
@end

@implementation	_GSWorkspaceCenter

- (void) dealloc
{
  [remote removeObserver: self name: nil object: GSWorkspaceNotification];
  RELEASE(remote);
  [super dealloc];
}

- (id) init
{
  self = [super init];
  if (self != nil)
    {
      remote = RETAIN([NSDistributedNotificationCenter defaultCenter]);
      NS_DURING
	{
	  [remote addObserver: self
		     selector: @selector(_handleRemoteNotification:)
			 name: nil
		       object: GSWorkspaceNotification];
	}
      NS_HANDLER
	{
	  NSUserDefaults	*defs = [NSUserDefaults standardUserDefaults];

	  if ([defs boolForKey: @"GSLogWorkspaceTimeout"])
	    {
	      NSLog(@"NSWorkspace caught exception %@: %@", 
	        [localException name], [localException reason]);
	    }
	  else
	    {
	      [localException raise];
	    }
	}
      NS_ENDHANDLER
    }
  return self;
}

/*
 * Post notification remotely - since we are listening for distributed
 * notifications, we will observe the notification arriving from the
 * distributed notification center, and it will get sent out locally too.
 */
- (void) postNotification: (NSNotification*)aNotification
{
  NSNotification	*rem;
  NSString		*name = [aNotification name];
  NSDictionary		*info = [aNotification userInfo];

  if ([name isEqual: NSWorkspaceDidTerminateApplicationNotification] == YES
    || [name isEqual: NSWorkspaceDidLaunchApplicationNotification] == YES
    || [name isEqualToString: NSApplicationDidBecomeActiveNotification] == YES
    || [name isEqualToString: NSApplicationDidResignActiveNotification] == YES)
    {
      GSLaunched(aNotification, YES);
    }

  rem = [NSNotification notificationWithName: name
				      object: GSWorkspaceNotification
				    userInfo: info];
  NS_DURING
    {
      [remote postNotification: rem];
    }
  NS_HANDLER
    {
      NSUserDefaults	*defs = [NSUserDefaults standardUserDefaults];

      if ([defs boolForKey: @"GSLogWorkspaceTimeout"])
	{
	  NSLog(@"NSWorkspace caught exception %@: %@", 
	    [localException name], [localException reason]);
	}
      else
	{
	  [localException raise];
	}
    }
  NS_ENDHANDLER
}

- (void) postNotificationName: (NSString*)name 
		       object: (id)object
{
  [self postNotification: [NSNotification notificationWithName: name
							object: object]];
}

- (void) postNotificationName: (NSString*)name 
		       object: (id)object
		     userInfo: (NSDictionary*)info
{
  [self postNotification: [NSNotification notificationWithName: name
							object: object
						      userInfo: info]];
}

/*
 * Forward a notification from a remote application to observers in this
 * application.
 */
- (void) _handleRemoteNotification: (NSNotification*)aNotification
{
  [self _postLocal: [aNotification name]
	  userInfo: [aNotification userInfo]];
}

/*
 * Method allowing a notification to be posted locally.
 */
- (void) _postLocal: (NSString*)name userInfo: (NSDictionary*)info
{
  NSNotification	*aNotification;

  aNotification = [NSNotification notificationWithName: name
						object: self
					      userInfo: info];
  [super postNotification: aNotification];
}
@end


@interface NSWorkspace (Private)

// Icon handling
- (NSImage*) _extIconForApp: (NSString*)appName info: (NSDictionary*)extInfo;
- (NSImage*) unknownFiletypeImage;
- (NSImage*) _saveImageFor: (NSString*)iconPath;
- (NSString*) thumbnailForFile: (NSString *)file;
- (NSImage*) _iconForExtension: (NSString*)ext;
- (BOOL) _extension: (NSString*)ext
               role: (NSString*)role
	        app: (NSString**)app;
- (BOOL) _scheme: (NSString*)scheme
	    role: (NSString*)role
	     app: (NSString**)app;
- (void) _workspacePreferencesChanged: (NSNotification *)aNotification;

// application communication
- (BOOL) _launchApplication: (NSString*)appName
		  arguments: (NSArray*)args;
- (id) _connectApplication: (NSString*)appName;
- (id) _workspaceApplication;

@end


/**
 * <p>The NSWorkspace class gathers together a large number of capabilities
 * needed for workspace management.
 * </p>
 * <p>The make_services tool examines all applications (anything with a
 * .app, .debug, or .profile suffix) in the system, local, and user Apps
 * directories, and caches information about the services each app
 * provides (extracted from the Info-gnustep.plist file in each application).
 * </p>
 * <p>In addition to the cache of services information, it builds a cache of
 * information about all known applications (including information about file
 * types they handle).
 * </p>
 * <p>NSWorkspace reads the cache and uses it to determine which application
 * to use to open a document and which icon to use to represent that document.
 * </p>
 * <p>The NSWorkspace API has been extended to provide methods for
 * finding/setting the preferred icon/application for a particular file
 * type. NSWorkspace will use the 'best' icon/application available.
 * </p>
 * <p>To determine the executable to launch, if there was an
 * Info-gnustep.plist/Info.plist in the app wrapper and it had an
 * NSExecutable field - it uses that name. Otherwise, it tries to use
 * the name of the app - eg. foo.app/foo <br />
 * The executable is launched by NSTask, which handles the addition
 * of machine/os/library path components as necessary.
 * </p>
 * <p>To determine the icon for a file, it uses the value from the
 * cache of icons for the file extension, or use an 'unknown' icon.
 * </p>
 * <p>To determine the icon for a folder, if the folder has a '.app',
 * '.debug' or '.profile' extension - the Info-gnustep.plist file
 * is examined for an 'NSIcon' value and NSWorkspace tries to use that.
 * If there is no value specified - it tries 'foo.app/foo.png'
 * or 'foo.app/foo.tiff' or 'foo.app/.dir.png' or 'foo.app/.dir.tiff'
 * </p>
 * <p>If the folder was not an application wrapper, it just tries
 * the .dir.png and .dir.tiff file.
 * </p>
 * <p>If no icon was available, it uses a default folder icon or a
 * special icon for the root directory.
 * </p>
 * <p>The information about what file types an app can handle needs
 * to be stored in Info-gnustep.plist in an array keyed on the name
 * <em>NSTypes</em>, within which each value is a dictionary.<br />
 * </p>
 * <p>In the NSTypes fields, NSWorkspace uses NSIcon (the icon to use
 * for the type) NSUnixExtensions (a list of file extensions
 * corresponding to the type) and NSRole (what the app can do with
 * documents of this type <em>Editor</em>, <em>Viewer</em>,
 * or <em>None</em>). In the AppList cache, make_services
 * generates a dictionary, keyed by file extension, whose values are
 * the dictionaries containing the NSTypes dictionaries of each
 * of the apps that handle the extension.  The NSWorkspace class
 * makes use of this cache at runtime.
 * </p>
 * <p>If the Info-gnustep.plist of an application says that it
 * can open files with a particular extension, then when NSWorkspace
 * is asked to open such a file it will attempt to send an
 * -application:openFile: message to the application (which must be
 * handled by the applications delegate).  If the application is not
 * running, NSWorkspace will instead attempt to launch the application
 * passing the filename to open after a '-GSFilePath' flag
 * in the command line arguments.  For a GNUstep application, the
 * application will recognize this and invoke the -application:openFile:
 * method passing it the file name.
 * </p>
 * <p>This command line argument mechanism provides a way for non-gnustep
 * applications to be used to open files simply by provideing a wrapper
 * for them containing the appropriate Info-gnustep.plist.<br />
 * For instance - you could set up xv.app to contain a shellscript 'xv'
 * that would start the real xv binary passing it a file to open if the
 * '-GSFilePath' argument was given. The Info-gnustep.plist file could look
 * like this:
 * </p>
 * <example>
 * 
 * {
 *   NSExecutable = "xv";
 *   NSIcon = "xv.png";
 *   NSTypes = (
 *     {
 *       NSIcon = "tiff.tiff";
 *       NSUnixExtensions = (tiff, tif);
 *     },
 *     {
 *       NSIcon = "xbm.tiff";
 *       NSUnixExtensions = (xbm);
 *     }
 *);
 * }
 * </example>
 */
@implementation	NSWorkspace

static NSWorkspace		*sharedWorkspace = nil;

static NSString			*appListPath = nil;
static NSDictionary		*applications = nil;

static NSString			*extPrefPath = nil;
static NSDictionary		*extPreferences = nil;

static NSString			*urlPrefPath = nil;
static NSDictionary		*urlPreferences = nil;

/*
 * Class methods
 */
+ (void) initialize
{
  if (self == [NSWorkspace class])
    {
      static BOOL	beenHere = NO;
      NSFileManager	*mgr = [NSFileManager defaultManager];
      NSString		*service;
      NSData		*data;
      NSDictionary	*dict;

      [self setVersion: 1];

      [gnustep_global_lock lock];
      if (beenHere == YES)
	{
	  [gnustep_global_lock unlock];
	  return;
	}

      beenHere = YES;
      mlock = [NSLock new];

      NS_DURING
	{
	  service = [[NSSearchPathForDirectoriesInDomains(NSLibraryDirectory, 
	    NSUserDomainMask, YES) objectAtIndex: 0]
	    stringByAppendingPathComponent: @"Services"];
	  
	  /*
	   *	Load file extension preferences.
	   */
	  extPrefPath = [service
	    stringByAppendingPathComponent: @".GNUstepExtPrefs"];
	  RETAIN(extPrefPath);
	  if ([mgr isReadableFileAtPath: extPrefPath] == YES)
	    {
	      data = [NSData dataWithContentsOfFile: extPrefPath];
	      if (data)
		{
		  dict = [NSDeserializer deserializePropertyListFromData: data
					 mutableContainers: NO];
		  extPreferences = RETAIN(dict);
		}
	    }
	  
	  /*
	   *	Load URL scheme preferences.
	   */
	  urlPrefPath = [service
	    stringByAppendingPathComponent: @".GNUstepURLPrefs"];
	  RETAIN(urlPrefPath);
	  if ([mgr isReadableFileAtPath: urlPrefPath] == YES)
	    {
	      data = [NSData dataWithContentsOfFile: urlPrefPath];
	      if (data)
		{
		  dict = [NSDeserializer deserializePropertyListFromData: data
					 mutableContainers: NO];
		  urlPreferences = RETAIN(dict);
		}
	    }
	  
	  /*
	   *	Load cached application information.
	   */
	  appListPath = [service
	    stringByAppendingPathComponent: @".GNUstepAppList"];
	  RETAIN(appListPath);
	  if ([mgr isReadableFileAtPath: appListPath] == YES)
	    {
	      data = [NSData dataWithContentsOfFile: appListPath];
	      if (data)
		{
		  dict = [NSDeserializer deserializePropertyListFromData: data
					 mutableContainers: NO];
		  applications = RETAIN(dict);
		}
	    }
	}
      NS_HANDLER
	{
	  [gnustep_global_lock unlock];
	  [localException raise];
	}
      NS_ENDHANDLER

      [gnustep_global_lock unlock];
    }
}

+ (id) allocWithZone: (NSZone*)zone
{
  [NSException raise: NSInvalidArgumentException
	      format: @"You may not allocate a workspace directly"];
  return nil;
}

/*
 * Creating a Workspace
 */
+ (NSWorkspace*) sharedWorkspace
{
  if (sharedWorkspace == nil)
    {
      [gnustep_global_lock lock];
      if (sharedWorkspace == nil)
	{
	  sharedWorkspace =
		(NSWorkspace*)NSAllocateObject(self, 0, NSDefaultMallocZone());
	  [sharedWorkspace init];
	}
      [gnustep_global_lock unlock];
    }
  return sharedWorkspace;
}

/*
 * Instance methods
 */
- (void) dealloc
{
  [NSException raise: NSInvalidArgumentException
	      format: @"Attempt to call dealloc for shared worksapace"];
  GSNOSUPERDEALLOC;
}

- (id) init
{
  NSArray *documentDir;
  NSArray *libraryDirs;
  NSArray *sysAppDir;
  NSArray *appDirs;
  NSArray *downloadDir;
  NSArray *desktopDir;
  NSArray *imgDir;
  NSArray *musicDir;
  NSArray *videoDir;
  NSString *sysDir;
  NSUInteger i;

  if (sharedWorkspace != self)
    {
      RELEASE(self);
      return RETAIN(sharedWorkspace);
    }

  [[NSNotificationCenter defaultCenter]
    addObserver: self
    selector: @selector(noteUserDefaultsChanged)
    name: NSUserDefaultsDidChangeNotification
    object: nil];

  /* There's currently no way of knowing if things have changed due to
   * apps being installed etc ... so we actually poll regularly.
   */
  [[NSNotificationCenter defaultCenter]
    addObserver: self
    selector: @selector(_workspacePreferencesChanged:)
    name: @"GSHousekeeping"
    object: nil];

  _workspaceCenter = [_GSWorkspaceCenter new];
  _iconMap = [NSMutableDictionary new];
  _launched = [NSMutableDictionary new];
  if (applications == nil)
    {
      [self findApplications];
    }
  [_workspaceCenter
    addObserver: self
    selector: @selector(_workspacePreferencesChanged:)
    name: GSWorkspacePreferencesChanged
    object: nil];

  /* icon association and caching */
  folderPathIconDict = [[NSMutableDictionary alloc] initWithCapacity:5];

  documentDir = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory,
    NSUserDomainMask, YES);
  downloadDir = NSSearchPathForDirectoriesInDomains(NSDownloadsDirectory,
    NSUserDomainMask, YES);
  desktopDir = NSSearchPathForDirectoriesInDomains(NSDesktopDirectory,
    NSUserDomainMask, YES);
  libraryDirs = NSSearchPathForDirectoriesInDomains(NSLibraryDirectory,
    NSAllDomainsMask, YES);
  sysAppDir = NSSearchPathForDirectoriesInDomains(NSApplicationDirectory,
    NSSystemDomainMask, YES);
  appDirs = NSSearchPathForDirectoriesInDomains(NSApplicationDirectory,
    NSAllDomainsMask, YES);
  imgDir = NSSearchPathForDirectoriesInDomains(NSPicturesDirectory,
    NSUserDomainMask, YES);
  musicDir = NSSearchPathForDirectoriesInDomains(NSMusicDirectory,
    NSUserDomainMask, YES);
  videoDir = NSSearchPathForDirectoriesInDomains(NSMoviesDirectory,
    NSUserDomainMask, YES);

  /* we try to guess a System directory and check if looks like one */
  sysDir = nil;
  if ([sysAppDir count] > 0)
    {
      sysDir = [[sysAppDir objectAtIndex: 0] stringByDeletingLastPathComponent];
      if (![[sysDir lastPathComponent] isEqualToString: @"System"])
	sysDir = nil;
    }

  if (sysDir != nil)
    [folderPathIconDict setObject: @"GSFolder" forKey: sysDir];

  [folderPathIconDict setObject: @"HomeDirectory"
    forKey: NSHomeDirectory()];

  /* it would be nice to use different root icons... */
  [folderPathIconDict setObject: @"Root_PC" forKey: NSOpenStepRootDirectory()];

  for (i = 0; i < [libraryDirs count]; i++)
    {
      [folderPathIconDict setObject: @"LibraryFolder"
	forKey: [libraryDirs objectAtIndex: i]];
    }
  for (i = 0; i < [appDirs count]; i++)
    {
      [folderPathIconDict setObject: @"ApplicationFolder"
	forKey: [appDirs objectAtIndex: i]];
    }
  for (i = 0; i < [documentDir count]; i++)
    {
      [folderPathIconDict setObject: @"DocsFolder"
	forKey: [documentDir objectAtIndex: i]];
    }
  for (i = 0; i < [downloadDir count]; i++)
    {
      [folderPathIconDict setObject: @"DownloadFolder"
	forKey: [downloadDir objectAtIndex: i]];
    }
  for (i = 0; i < [desktopDir count]; i++)
    {
      [folderPathIconDict setObject: @"Desktop"
	forKey: [desktopDir objectAtIndex: i]];
    }
  for (i = 0; i < [imgDir count]; i++)
    {
      [folderPathIconDict setObject: @"ImageFolder"
	forKey: [imgDir objectAtIndex: i]];
    }
  for (i = 0; i < [musicDir count]; i++)
    {
      [folderPathIconDict setObject: @"MusicFolder"
	forKey: [musicDir objectAtIndex: i]];
    }
  for (i = 0; i < [videoDir count]; i++)
    {
      [folderPathIconDict setObject: @"VideoFolder"
	forKey: [videoDir objectAtIndex: i]];
    }
  folderIconCache = [[NSMutableDictionary alloc] init];

  return self;
}

/*
 * Opening Files
 */
- (BOOL) _openUnknown: (NSString*)fullPath
{
  NSString *tool = [[NSUserDefaults standardUserDefaults] objectForKey: @"GSUnknownFileTool"];
  NSString *launchPath;

  if ((tool == nil) || (launchPath = [NSTask launchPathForTool: tool]) == nil)
    {
#ifdef __MINGW32__
      // Maybe we should rather use "Explorer.exe /e, " as the tool name
      unichar *buffer = (unichar *)calloc(1, ([fullPath length] + 1) * sizeof(unichar));
      [fullPath getCharacters: buffer range: NSMakeRange(0, [fullPath length])];
      buffer[[fullPath length]] = 0;
      BOOL success = ((int)ShellExecuteW(GetDesktopWindow(), L"open", buffer, NULL, 
                                    NULL, SW_SHOWNORMAL) > 32);
      free(buffer);
      return success;
#else
      // Fall back to xdg-open
      launchPath = [NSTask launchPathForTool: @"xdg-open"];
#endif
    }

  if (launchPath)
    {
      NSTask * task = [NSTask launchedTaskWithLaunchPath: launchPath
                                               arguments: [NSArray arrayWithObject: fullPath]];
      if (task != nil)
        {
          [task waitUntilExit];
          if ([task terminationStatus] == 0)
            return YES;
        }
    }

  return NO;
}

- (BOOL) openFile: (NSString*)fullPath
{
  return [self openFile: fullPath withApplication: nil];
}

- (BOOL) openFile: (NSString*)fullPath
        fromImage: (NSImage*)anImage
	       at: (NSPoint)point
	   inView: (NSView*)aView
{
  NSWindow *win = [aView window];
  NSPoint screenLoc = [win convertBaseToScreen:
			[aView convertPoint: point toView: nil]];
  NSSize screenSize = [[win screen] frame].size;
  NSPoint screenCenter = NSMakePoint(screenSize.width / 2, 
				     screenSize.height / 2);

  [self slideImage: anImage from: screenLoc to: screenCenter];
  return [self openFile: fullPath];
}

- (BOOL) openFile: (NSString*)fullPath
  withApplication: (NSString*)appName
{
  return [self openFile: fullPath withApplication: appName andDeactivate: YES];
}

- (BOOL) openFile: (NSString*)fullPath
  withApplication: (NSString*)appName
    andDeactivate: (BOOL)flag
{
  id app;

  NS_DURING
    {
      if ((app = [self _workspaceApplication]) != nil)
	{
	  BOOL	result;

	  result = [app openFile: fullPath
		 withApplication: appName
		   andDeactivate: flag];
	  NS_VALRETURN(result);
	}
    }
  NS_HANDLER
    // workspace manager problem ... fall through to default code
  NS_ENDHANDLER

  if (appName == nil)
    {
      NSString *ext = [fullPath pathExtension];
  
      if ([self _extension: ext role: nil app: &appName] == NO)
        {
          if ([self _openUnknown: fullPath])
            {
              return YES;
            }
          else
            {
              NSWarnLog(@"No known applications for file extension '%@'", ext);
              return NO;
            }
	}
    }

  app = [self _connectApplication: appName];
  if (app == nil)
    {
      NSArray *args;

      args = [NSArray arrayWithObjects: @"-GSFilePath", fullPath, nil];
      return [self _launchApplication: appName arguments: args];
    }
  else
    {
      NS_DURING
	{
	  if (flag == NO)
	    {
	      [app application: NSApp openFileWithoutUI: fullPath];
	    }
	  else
	    {
	      [app application: NSApp openFile: fullPath];
	    }
	}
      NS_HANDLER
	{
	  NSWarnLog(@"Failed to contact '%@' to open file", appName);
	  return NO;
	}
      NS_ENDHANDLER
    }
  if (flag)
    {
      [NSApp deactivate];
    }
  return YES;
}

- (BOOL) openTempFile: (NSString*)fullPath
{
  id app;
  NSString *appName;
  NSString *ext;

  NS_DURING
    {
      if ((app = [self _workspaceApplication]) != nil)
	{
	  BOOL	result;

	  result = [app openTempFile: fullPath];
	  NS_VALRETURN(result);
	}
    }
  NS_HANDLER
    // workspace manager problem ... fall through to default code
  NS_ENDHANDLER

  ext = [fullPath pathExtension];
  if ([self _extension: ext role: nil app: &appName] == NO)
    {
      if ([self _openUnknown: fullPath])
        {
          return YES;
        }
      else
        {
          NSWarnLog(@"No known applications for file extension '%@'", ext);
          return NO;
        }
    }

  app = [self _connectApplication: appName];
  if (app == nil)
    {
      NSArray *args;

      args = [NSArray arrayWithObjects: @"-GSTempPath", fullPath, nil];
      return [self _launchApplication: appName arguments: args];
    }
  else
    {
      NS_DURING
	{
	  [app application: NSApp openTempFile: fullPath];
	}
      NS_HANDLER
	{
	  NSWarnLog(@"Failed to contact '%@' to open temp file", appName);
	  return NO;
	}
      NS_ENDHANDLER
    }

  [NSApp deactivate];

  return YES;
}

- (BOOL) openURL: (NSURL*)url
{
  if ([url isFileURL])
    {
      return [self openFile: [url path]];
    }
  else
    {
      NSString		*appName;
      NSPasteboard      *pb;

      appName = [self getBestAppInRole: nil forScheme: [url scheme]];
      if (appName != nil)
	{
	  id app;

	  /* Now try to get the application to open the URL.
	   */
	  app = GSContactApplication(appName, nil, nil);
	  if (app != nil)
	    {
	      NS_DURING
		{
	          [app application: NSApp openURL: url];
		}
	      NS_HANDLER
		{
		  NSWarnLog(@"Failed to contact '%@' to open file", appName);
		  return NO;
		}
	      NS_ENDHANDLER
	      [NSApp deactivate];
	      return YES;
	    }
	}
      /* No application found to open the URL.
       * Try any OpenURL service available.
       */
      pb = [NSPasteboard pasteboardWithUniqueName];
      [pb declareTypes: [NSArray arrayWithObject: NSURLPboardType]
                         owner: nil];
     [url writeToPasteboard: pb];
     if (NSPerformService(@"OpenURL", pb))
       {
         return YES;
       }
     else
       {
         return [self _openUnknown: [url absoluteString]];
       }
    }
}

/*
 * Manipulating Files	
 */
- (BOOL) performFileOperation: (NSString*)operation
		       source: (NSString*)source
		  destination: (NSString*)destination
		        files: (NSArray*)files
			  tag: (NSInteger*)tag
{
  id app;

  NS_DURING
    {
      if ((app = [self _workspaceApplication]) != nil)
	{
	  BOOL	result;

	  result = [app performFileOperation: operation
				      source: source
				 destination: destination
				       files: files
					 tag: tag];
	  NS_VALRETURN(result);
	}
    }
  NS_HANDLER
    // workspace manager problem ... fall through to default code
  NS_ENDHANDLER

  return NO;
}

- (BOOL) selectFile: (NSString*)fullPath
inFileViewerRootedAtPath: (NSString*)rootFullpath
{
  id app;

  NS_DURING
    {
      if ((app = [self _workspaceApplication]) != nil)
	{
	  BOOL	result;

	  result = [app selectFile: fullPath
	  inFileViewerRootedAtPath: rootFullpath];
	  NS_VALRETURN(result);
	}
    }
  NS_HANDLER
    // workspace manager problem ... fall through to default code
  NS_ENDHANDLER

  return NO;
}

/**
 * Given an application name, return the full path for that application.<br />
 * This method looks for the application in standard locations, and if not
 * found there, according to MacOS-X documentation, returns nil.<br />
 * If the supplied application name is an absolute path, returns that path
 * irrespective of whether such an application exists or not.  This is
 * <em>not</em> the docmented debavior in the MacOS-X documentation, but is
 * the MacOS-X implemented behavior.<br />
 * If the appName has an extension, it is used, otherwise in GNUstep
 * the standard app, debug, and profile extensions * are tried.<br />
 */
- (NSString*) fullPathForApplication: (NSString*)appName
{
  NSString	*base;
  NSString	*path;
  NSString	*ext;

  if ([appName length] == 0)
    {
      return nil;
    }
  if ([[appName lastPathComponent] isEqual: appName] == NO)
    {
      if ([appName isAbsolutePath] == YES)
	{
	  return appName;		// MacOS-X implementation behavior.
	}
      /*
       * Relative path ... get standarized absolute path
       */
      path = [[NSFileManager defaultManager] currentDirectoryPath];
      appName = [path stringByAppendingPathComponent: appName];
      appName = [appName stringByStandardizingPath];
    }
  base = [appName stringByDeletingLastPathComponent];
  appName = [appName lastPathComponent];
  ext = [appName pathExtension];
  if ([ext length] == 0) // no extension, let's find one
    {
      path = [appName stringByAppendingPathExtension: @"app"];
      path = [applications objectForKey: path];
      if (path == nil)
	{
	  path = [appName stringByAppendingPathExtension: @"debug"];
	  path = [applications objectForKey: path];
	}
      if (path == nil)
	{
	  path = [appName stringByAppendingPathExtension: @"profile"];
	  path = [applications objectForKey: path];
	}
    }
  else
    {
      path = [applications objectForKey: appName];
    }

  /*
   * If the original name included a path, check that the located name
   * matches it.  If it doesn't we return nil as MacOS-X does.
   */
  if ([base length] > 0
    && [base isEqual: [path stringByDeletingLastPathComponent]] == NO)
    {
      path = nil;
    }
  return path;
}

- (BOOL) getFileSystemInfoForPath: (NSString*)fullPath
		      isRemovable: (BOOL*)removableFlag
		       isWritable: (BOOL*)writableFlag
		    isUnmountable: (BOOL*)unmountableFlag
		      description: (NSString **)description
			     type: (NSString **)fileSystemType
{
  NSArray *removables;
  NSString  *fsName;

  /* since we might not be able to get information about removable volumes
     we use the information from the preferences which can be set in SystemPreferences
   */
  removables = [[[NSUserDefaults standardUserDefaults] persistentDomainForName: NSGlobalDomain] objectForKey: @"GSRemovableMediaPaths"];

  *removableFlag = NO;
  if ([removables containsObject: fullPath])
    *removableFlag = YES;

  fsName = nil;
#if defined(HAVE_GETMNTENT) && defined (MNT_MEMB)
  // if this is called from mountedLocalVolumePaths, the getmntent is searched again for each item
  FILE		*fptr = setmntent(MOUNTED_PATH, "r");
  struct mntent	*me;

  while ((me = getmntent(fptr)) != 0)
  {
    if (strcmp(me->MNT_MEMB, [fullPath fileSystemRepresentation]) == 0)
      {
	fsName = [NSString stringWithCString:me->MNT_FSNAME];
      }
  }
  endmntent(fptr);
#endif /* HAVE_GETMINTENT */
  if (fsName && [fsName hasPrefix:@"/dev"])
    {
      NSString *devName;
      NSString *devInfoPath;
      BOOL r;
      NSString *removableString;

      r = NO;
      devName = [fsName lastPathComponent];
      // This is a very crude way of removing the partition number
      if ([devName length] > 3)
	devName = [devName substringToIndex: 3];

      devInfoPath = [@"/sys/block" stringByAppendingPathComponent:devName];
      devInfoPath = [devInfoPath stringByAppendingPathComponent:@"removable"];

      removableString = [[NSString alloc] initWithContentsOfFile:devInfoPath];

      if ([removableString hasPrefix:@"1"])
	r = YES;
      [removableString release];

      // we go in OR against the informatoin derived from declared removables
      // so we enrich, but don't mark removables as not
      *removableFlag |= r;
    }

  
#if defined (HAVE_SYS_STATVFS_H) || defined (HAVE_SYS_VFS_H)
  /* We use statvfs() if available to get information but statfs()
     will provide more information on different systems, but in a non
     standard way. Thus e.g. on Linux two calls are needed.
     The NetBSD statvfs is a statfs in disguise, i.e., it provides all
     information available in      the 4.4BSD statfs call. 
     Other BSDs and      Linuxes have statvfs as well, but this returns less
     information than      the 4.4BSD statfs call.
     Note that the POSIX statvfs is not really helpful for us here. The
     only information that could be extracted from the data returned by
     that syscall is the ST_RDONLY flag. There is no owner field nor a
     typename.
     The statvfs call on Solaris returns a structure that includes a
     non-standard f_basetype field, which provides the name of the
     underlying file system type.
  */
  BOOL isRootFS;
  BOOL hasOwnership;

#if defined(HAVE_STATVFS)
  #define USING_STATVFS 1
  struct statvfs m;
  if (statvfs([fullPath fileSystemRepresentation], &m))
    return NO;
#elif defined (HAVE_STATFS)
  #define USING_STATFS 1
  struct statfs m;
  if (statfs([fullPath fileSystemRepresentation], &m))
    return NO;  
#endif

  *writableFlag = 1;
#if  defined(HAVE_STRUCT_STATVFS_F_FLAG)
  *writableFlag = (m.f_flag & ST_RDONLY) == 0;
#elif defined(HAVE_STRUCT_STATFS_F_FLAGS)
  *writableFlag = (m.f_flags & ST_RDONLY) == 0;
#endif


  isRootFS = NO;
#if defined(ST_ROOTFS)
  isRootFS = (m.f_flag & ST_ROOTFS);
#elif defined (MNT_ROOTFS)
  isRootFS = (m.f_flag & MNT_ROOTFS);
#endif

  hasOwnership = NO;
#if (defined(USING_STATFS) && defined(HAVE_STRUCT_STATFS_F_OWNER)) || (defined(USING_STATVFS) &&  defined(HAVE_STRUCT_STATVFS_F_OWNER))
  uid_t uid = geteuid();
  if (uid == 0 || uid == m.f_owner)
    hasOwnership = YES;
#elif (defined(USING_STATVFS) && !defined(USING_STATFS) && defined (HAVE_STATFS) && defined(HAVE_STRUCT_STATFS_F_OWNER))
  uid_t uid = geteuid();
  // FreeBSD only?
  struct statfs m2;
  statfs([fullPath fileSystemRepresentation], &m2);
  if (uid == 0 || uid == m2.f_owner)
    hasOwnership = YES;
#endif
  
  *unmountableFlag = !isRootFS && hasOwnership;
  
  *description = @"filesystem"; // FIXME

  *fileSystemType = nil;
#if defined (__linux__)
  struct statfs m2;

  statfs([fullPath fileSystemRepresentation], &m2);
  if (m2.f_type == EXT2_SUPER_MAGIC)
    *fileSystemType = @"EXT2";
  else if (m2.f_type == EXT3_SUPER_MAGIC)
    *fileSystemType = @"EXT3";
  else if (m2.f_type == EXT4_SUPER_MAGIC)
    *fileSystemType = @"EXT4";
  else if (m2.f_type == ISOFS_SUPER_MAGIC)
    *fileSystemType = @"ISO9660";
#ifdef JFS_SUPER_MAGIC
  else if (m2.f_type == JFS_SUPER_MAGIC)
    *fileSystemType = @"JFS";
#endif
  else if (m2.f_type == MSDOS_SUPER_MAGIC)
    *fileSystemType = @"MSDOS";
  else if (m2.f_type == NFS_SUPER_MAGIC)
    *fileSystemType = @"NFS";
  else
    *fileSystemType = @"Other";
#elif defined(__sun__)
  *fileSystemType =
    [[NSString alloc] initWithCString: m.f_basetype encoding: [NSString defaultCStringEncoding]];
#elif !defined(__GNU__)
  // FIXME we disable this for HURD, but we need to check for struct member in configure
  //  *fileSystemType = [[NSString alloc] initWithCString: m.f_fstypename encoding: [NSString defaultCStringEncoding]];
#endif

#else /* no statfs() nor statvfs() */
  NSLog(@"getFileSystemInfoForPath not supported on your OS");
#endif
  
  return YES;
}

/**
 * This method gets information about the file at fullPath and
 * returns YES on success, NO if the named file could not be
 * found.<br />
 * On success, the name of the preferred application for opening
 * the file is returned in *appName, or nil if there is no known
 * application to open it.<br />
 * The returned value in *type describes the file using one of
 * the following constants.
 * <deflist>
 *   <term>NSPlainFileType</term>
 *   <desc>
 *     A plain file or a directory that some application
 *     claims to be able to open like a file.
 *   </desc>
 *   <term>NSDirectoryFileType</term>
 *   <desc>An untyped directory</desc>
 *   <term>NSApplicationFileType</term>
 *   <desc>A GNUstep application</desc>
 *   <term>NSFilesystemFileType</term>
 *   <desc>A file system mount point</desc>
 *   <term>NSShellCommandFileType</term>
 *   <desc>Executable shell command</desc>
 * </deflist>
 */
- (BOOL) getInfoForFile: (NSString*)fullPath
	    application: (NSString **)appName
		   type: (NSString **)type
{
  NSFileManager	*fm = [NSFileManager defaultManager];
  NSDictionary	*attributes;
  NSString	*fileType;
  NSString	*extension = [fullPath pathExtension];

  attributes = [fm fileAttributesAtPath: fullPath traverseLink: YES];

  if (attributes != nil)
    {
      *appName = [self getBestAppInRole: nil forExtension: extension];
      fileType = [attributes fileType];
      if ([fileType isEqualToString: NSFileTypeRegular])
	{
	  if ([attributes filePosixPermissions] & PosixExecutePermission)
	    {
	      *type = NSShellCommandFileType;
	    }
	  else
	    {
	      *type = NSPlainFileType;
	    }
	}
      else if ([fileType isEqualToString: NSFileTypeDirectory])
	{
	  if ([extension isEqualToString: @"app"]
	    || [extension isEqualToString: @"debug"]
	    || [extension isEqualToString: @"profile"])
	    {
	      *type = NSApplicationFileType;
	    }
	  else if ([extension isEqualToString: @"bundle"])
	    {
	      *type = NSPlainFileType;
	    }
	  else if (*appName != nil && [extension length] > 0)
	    {
	      *type = NSPlainFileType;
	    }
	  /*
	   * The idea here is that if the parent directory's
	   * fileSystemNumber differs, this must be a filesystem
	   * mount point.
	   */
	  else if ([[fm fileAttributesAtPath:
	    [fullPath stringByDeletingLastPathComponent]
	    traverseLink: YES] fileSystemNumber]
	    != [attributes fileSystemNumber])
	    {
	      *type = NSFilesystemFileType;
	    }
	  else
	    {
	      *type = NSDirectoryFileType;
	    }
	}
      else
	{
	  /*
	   * This catches sockets, character special, block special,
	   * and unknown file types
	   */
	  *type = NSPlainFileType;
	}
      return YES;
    }
  else
    {
      *appName = nil;
      return NO;
    }
}

- (NSImage*) iconForFile: (NSString*)fullPath
{
  NSImage	*image = nil;
  NSString	*pathExtension = [[fullPath pathExtension] lowercaseString];
  NSFileManager	*mgr = [NSFileManager defaultManager];
  NSDictionary	*attributes;
  NSString	*fileType;

  /*
    If we have a symobolic link, get not only the original path attributes,
    but also the original path, to resolve the correct icon.
    mac resolves the original icon
  */
  fullPath = [fullPath stringByResolvingSymlinksInPath];

  /* now we get the target attributes of the traversed link */
  attributes = [mgr fileAttributesAtPath: fullPath traverseLink: NO];
  fileType = [attributes fileType];

  if ([fileType isEqual: NSFileTypeDirectory] == YES)
    {
      NSString *iconPath = nil;
      
      if ([pathExtension isEqualToString: @"app"]
	|| [pathExtension isEqualToString: @"debug"]
	|| [pathExtension isEqualToString: @"profile"])
	{
	  image = [self appIconForApp: fullPath];
	  
	  if (image == nil)
	    {
	      /*
               * Just use the appropriate icon for the path extension
               */
              return [self _iconForExtension: pathExtension];
	    }
	}

      /*
       * If we have no iconPath, try 'dir/.dir.png' as a
       * possible locations for the directory icon.
       */
      if (iconPath == nil)
	{
	  iconPath = [fullPath stringByAppendingPathComponent: @".dir.png"];
	  if ([mgr isReadableFileAtPath: iconPath] == NO)
	    {
	      iconPath
		= [fullPath stringByAppendingPathComponent: @".dir.tiff"];
	      if ([mgr isReadableFileAtPath: iconPath] == NO)
		{
		  iconPath = nil;
		}
	    }
	}

      if (iconPath != nil)
	{
	  image = [self _saveImageFor: iconPath];
	}

      if (image == nil)
	{
	  image = [self _iconForExtension: pathExtension];
	  if (image == nil || image == [self unknownFiletypeImage])
	    {
	      NSString *iconName;

	      iconName = [folderPathIconDict objectForKey: fullPath];
	      if (iconName != nil)
		{
		  NSImage *iconImage;

		  iconImage = [folderIconCache objectForKey: iconName];
		  if (iconImage == nil)
		    {
		      iconImage = [NSImage _standardImageWithName: iconName];
                      if (!iconImage)
                        {
                          /* no specific image found in theme, fall-back to folder */
                          NSLog(@"no image found for %@", iconName);
                          iconImage = [NSImage _standardImageWithName: @"Folder"];
                        }
                      /* the dictionary retains the image */
                      [folderIconCache setObject: iconImage forKey: iconName];
		    }
		  image = iconImage;
		}
	      else
		{
		  if (folderImage == nil)
		    {
		      folderImage = RETAIN([NSImage _standardImageWithName:
						      @"Folder"]);
		    }
		  image = folderImage;
		}
	    }

	}
    }
  else
    {
      NSDebugLog(@"pathExtension is '%@'", pathExtension);

      if ([[NSUserDefaults standardUserDefaults] boolForKey: 
	      @"GSUseFreedesktopThumbnails"])
        {
	  /* This image will be 128x128 pixels as oposed to the 48x48 
	     of other GNUstep icons or the 32x32 of the specification */  
	  image = [self _saveImageFor: [self thumbnailForFile: fullPath]];
	  if (image != nil)
	    {
	      return image;
	    }
	}

      image = [self _iconForExtension: pathExtension];
      if (image == nil || image == [self unknownFiletypeImage])
	{
	  NSFileManager	*mgr;

	  mgr = [NSFileManager defaultManager];
	  if ([mgr isExecutableFileAtPath: fullPath] == YES)
	    {
	      NSDictionary	*attributes;
	      NSString		*fileType;

	      attributes = [mgr fileAttributesAtPath: fullPath
					traverseLink: YES];
	      fileType = [attributes objectForKey: NSFileType];
	      if ([fileType isEqual: NSFileTypeRegular] == YES)
		{
		  if (unknownTool == nil)
		    {
		      unknownTool = RETAIN([NSImage _standardImageWithName:
			@"UnknownTool"]);
		    }
		  image = unknownTool;
		}
	    }
	}
    }

  if (image == nil)
    {
      image = [self unknownFiletypeImage];
    }

  return image;
}

- (NSImage*) iconForFiles: (NSArray*)pathArray
{
  if ([pathArray count] == 1)
    {
      return [self iconForFile: [pathArray objectAtIndex: 0]];
    }
  if (multipleFiles == nil)
    {
      // FIXME: Icon does not exist
      multipleFiles = [NSImage imageNamed: @"FileIcon_multi"];
    }

  return multipleFiles;
}

- (NSImage*) iconForFileType: (NSString*)fileType
{
  return [self _iconForExtension: fileType];
}

- (BOOL) isFilePackageAtPath: (NSString*)fullPath
{
  NSFileManager	*mgr = [NSFileManager defaultManager];
  NSDictionary	*attributes;
  NSString	*fileType, *extension;

  attributes = [mgr fileAttributesAtPath: fullPath traverseLink: YES];
  fileType = [attributes objectForKey: NSFileType];
  if ([fileType isEqual: NSFileTypeDirectory] == YES)
    {
      /*
       * We return YES here exactly when getInfoForFile:application:type:
       * considers the directory an application or a plain file
       */
      extension = [fullPath pathExtension];
      if ([extension isEqualToString: @"app"]
	|| [extension isEqualToString: @"debug"]
	|| [extension isEqualToString: @"profile"]
        || [extension isEqualToString: @"bundle"])
        {
	  return YES;
	}
      else if ([extension length] > 0
        && [self getBestAppInRole: nil forExtension: extension] != nil)
        {
	  return YES;
        }
    }
  return NO;
}

- (BOOL) setIcon: (NSImage *)image
         forFile: (NSString *)fullPath
         options: (NSWorkspaceIconCreationOptions)options
{
  // FIXME
  return NO;
}

/**
 * Tracking Changes to the File System
 */
- (BOOL) fileSystemChanged
{
  BOOL flag = _fileSystemChanged;

  _fileSystemChanged = NO;
  return flag;
}

- (void) noteFileSystemChanged
{
  _fileSystemChanged = YES;
}

- (void) noteFileSystemChanged: (NSString*)path
{
  _fileSystemChanged = YES;
}

/**
 * Updates Registered Services, File Types, and other information about any
 * applications installed in the standard locations.
 */
- (void) findApplications
{
  static NSString	*path = nil;
  NSTask		*task;

  /*
   * Try to locate and run an executable copy of 'make_services'
   */
  if (path == nil)
    {
      path = [[NSTask launchPathForTool: @"make_services"] retain];
    }
  task = [NSTask launchedTaskWithLaunchPath: path
				  arguments: nil];
  if (task != nil)
    {
      [task waitUntilExit];
    }
  [self _workspacePreferencesChanged:
     [NSNotification notificationWithName: GSWorkspacePreferencesChanged
				   object: self]];
}

/**
 * Instructs all the other running applications to hide themselves.
 * <em>not yet implemented</em>
 */
- (void) hideOtherApplications
{
  // FIXME
}

/**
 * Calls -launchApplication:showIcon:autolaunch: with arguments set to
 * show the icon but not set it up as an autolaunch.
 */ 
- (BOOL) launchApplication: (NSString*)appName
{
  return [self launchApplication: appName
			showIcon: YES
		      autolaunch: NO];
}

/**
 * <p>Launches the specified application (unless it is already running).<br />
 * If the autolaunch flag is yes, sets the autolaunch user default for the
 * newly launched application, so that applications which understand the
 * concept of being autolaunched at system startup time can modify their
 * behavior appropriately.
 * </p>
 * <p>Sends an NSWorkspaceWillLaunchApplicationNotification before it
 * actually attempts to launch the application (this is not sent if the
 * application is already running).
 * </p>
 * <p>The application sends an NSWorkspaceDidlLaunchApplicationNotification
 * on completion of launching.  This is not sent if the application is already
 * running, or if it fails to complete its startup.
 * </p>
 * <p>Returns NO if the application cannot be launched (eg. it does not exist
 * or the binary is not executable).
 * </p>
 * <p>Returns YES if the application was already running or of it was launched
 * (this does not necessarily mean that the application succeeded in starting
 * up fully).
 * </p>
 * <p>Once an application has fully started up, you should be able to connect
 * to it using [NSConnection+rootProxyForConnectionWithRegisteredName:host:]
 * passing the application name (normally the filesystem name excluding path
 * and file extension) and an empty host name.  This will let you communicate
 * with the the [NSApplication-delegate] of the launched application, and you
 * can generally use this as a test of whether an application is running
 * correctly.
 * </p>
 */
- (BOOL) launchApplication: (NSString*)appName
		  showIcon: (BOOL)showIcon
	        autolaunch: (BOOL)autolaunch
{
  id 	app;

  NS_DURING
    {
      if ((app = [self _workspaceApplication]) != nil)
	{
	  BOOL	result;

	  result = [app launchApplication: appName
	   			 showIcon: showIcon
			       autolaunch: autolaunch];
	  NS_VALRETURN(result);
	}
    }
  NS_HANDLER
    // workspace manager problem ... fall through to default code
  NS_ENDHANDLER

  app = [self _connectApplication: appName];
  if (app == nil)
    {
      NSArray	*args = nil;

      if (autolaunch == YES)
	{
	  args = [NSArray arrayWithObjects: @"-autolaunch", @"YES", nil];
	}
      return [self _launchApplication: appName arguments: args];
    }
  else
    {
      [app activateIgnoringOtherApps:YES];
    }

  return YES;
}

- (NSString *) absolutePathForAppBundleWithIdentifier: (NSString *)bundleIdentifier
{
  // TODO: full implementation
  return [self fullPathForApplication: bundleIdentifier];
}

- (BOOL) launchAppWithBundleIdentifier: (NSString *)bundleIdentifier
			       options: (NSWorkspaceLaunchOptions)options 
	additionalEventParamDescriptor: (NSAppleEventDescriptor *)descriptor 
		      launchIdentifier: (NSNumber **)identifier 
{
  // TODO: full implementation
  return [self launchApplication: bundleIdentifier
			showIcon: YES
		      autolaunch: NO];
}

- (BOOL) openURLs: (NSArray *)urls
withAppBundleIdentifier: (NSString *)bundleIdentifier
          options: (NSWorkspaceLaunchOptions)options
additionalEventParamDescriptor: (NSAppleEventDescriptor *)descriptor
launchIdentifiers: (NSArray **)identifiers
{
  // FIXME
  return NO;
}

/**
 * Returns a description of the currently active application, containing
 * the name (NSApplicationName), path (NSApplicationPath) and process
 * identifier (NSApplicationProcessIdentifier).<br />
 * Returns nil if there is no known active application.
 */
- (NSDictionary*) activeApplication
{
  id	app;

  NS_DURING
    {
      if ((app = [self _workspaceApplication]) != nil)
	{
	  NSDictionary	*result;

	  result = [app activeApplication];
	  NS_VALRETURN(result);
	}
    }
  NS_HANDLER
    // workspace manager problem ... fall through to default code
  NS_ENDHANDLER

  return GSLaunched(nil, YES);
}

/**
 * Returns an array listing all the applications known to have been
 * launched.  Each entry in the array is a dictionary providing
 * the name, path and process identfier of an application.
 */
- (NSArray*) launchedApplications
{
  NSArray       *apps = nil;

  NS_DURING
    {
      id	app;

      if ((app = [self _workspaceApplication]) != nil)
	{
	  apps = [app launchedApplications];
	}
    }
  NS_HANDLER
    {
      // workspace manager problem ... fall through to default code
    }
  NS_ENDHANDLER

  if (apps == nil)
    {
      NSMutableArray    *m;
      unsigned          count;

      apps = GSLaunched(nil, NO);
      apps = m = AUTORELEASE([apps mutableCopy]);
      if ((count = [apps count]) > 0)
        {
          if ([NSProcessInfo respondsToSelector: @selector(_exists:)] == YES)
            {
              /* Check and remove apps whose pid no longer exists
               */
              while (count-- > 0)
                {
                  int       pid;
                  NSString  *name;

                  name = [[apps objectAtIndex: count]
                    objectForKey: @"NSApplicationName"];
                  pid = [[[apps objectAtIndex: count]
                    objectForKey: @"NSApplicationProcessIdentifier"] intValue];
                  if (pid > 0 && [name length] > 0)
                    {
                      if ([NSProcessInfo _exists: pid] == NO)
                        {
                          GSLaunched([NSNotification notificationWithName:
                            NSWorkspaceDidTerminateApplicationNotification
                            object: self
                            userInfo: [NSDictionary dictionaryWithObject: name
                              forKey: @"NSApplicationName"]], NO);
                          [m removeObjectAtIndex: count];
                        }
                    }
                }
            }
        }
    }
  return apps;
}

/*
 * Unmounting a Device and eject if possible
 */
- (BOOL) unmountAndEjectDeviceAtPath: (NSString*)path
{
  NSUInteger    systype = [[NSProcessInfo processInfo] operatingSystem];
  NSDictionary	*userinfo;
  NSTask	*task;

  /* let's check if it is a local volume we may unmount */
  if (![[self mountedLocalVolumePaths] containsObject:path])
    {
      NSLog(@"unmountAndEjectDeviceAtPath: Path %@ not mounted", path);
      return NO;
    }

  userinfo = [NSDictionary dictionaryWithObject: path
					 forKey: @"NSDevicePath"];
  [_workspaceCenter postNotificationName: NSWorkspaceWillUnmountNotification
				  object: self
				userInfo: userinfo];
  task = [NSTask launchedTaskWithLaunchPath: @"umount"
				  arguments: [NSArray arrayWithObject: path]];
      
  if (task)
    {
      [task waitUntilExit];
      if ([task terminationStatus] != 0)
	{
	  return NO;
	} 
    }
  else
    {
      return NO;
    }

  [[self notificationCenter] postNotificationName: NSWorkspaceDidUnmountNotification
					   object: self
					 userInfo: userinfo];

  /* this is system specific and we try our best
     and the failure of eject doesn't mean unmount failed */
  task = nil;
  if (systype == NSGNULinuxOperatingSystem)
    {
      task = [NSTask launchedTaskWithLaunchPath: @"eject"
				      arguments: [NSArray arrayWithObject: path]];
    }
  else if (systype == NSBSDOperatingSystem || systype == NSSolarisOperatingSystem)
    {
      NSString *mountDir;

      // Note: it would be better to check the device, not the mount point
      mountDir = [path lastPathComponent];
      if ([mountDir rangeOfString:@"cd"].location != NSNotFound ||
	  [mountDir rangeOfString:@"dvd"].location != NSNotFound)
	{
	  task = [NSTask launchedTaskWithLaunchPath: @"eject"
					  arguments: [NSArray arrayWithObject: @"cdrom"]];
	}
      else if ([mountDir rangeOfString:@"fd"].location != NSNotFound ||
	  [mountDir rangeOfString:@"floppy"].location != NSNotFound)
	{
	  task = [NSTask launchedTaskWithLaunchPath: @"eject"
					  arguments: [NSArray arrayWithObject: @"floppy"]];
	}
    }
  else
    {
      NSLog(@"Don't know how to eject");
    }
  if (task != nil)
    {
      [task waitUntilExit];
      if ([task terminationStatus] != 0)
	{
	  NSLog(@"eject failed");
	}
    }

  return YES;
}

/*
 * Tracking Status Changes for Devices
 */
- (void) checkForRemovableMedia
{
  // FIXME
}

- (NSArray*) mountNewRemovableMedia
{
  NSArray *removables;
  NSArray *mountedMedia = [self mountedRemovableMedia]; 
  NSMutableArray *willMountMedia = [NSMutableArray array];
  NSMutableArray *newlyMountedMedia = [NSMutableArray array];
  NSUInteger i;

  /* we use the system preferences to know which ones to mount */
  removables = [[[NSUserDefaults standardUserDefaults] persistentDomainForName: NSGlobalDomain] objectForKey: @"GSRemovableMediaPaths"];

  for (i = 0; i < [removables count]; i++)
    {
      NSString *removable = [removables objectAtIndex: i];
    
      if ([mountedMedia containsObject: removable] == NO)
        {
          [willMountMedia addObject: removable];
        }
    }  

  for (i = 0; i < [willMountMedia count]; i++)
    {
      NSString *media = [willMountMedia objectAtIndex: i];
      NSTask *task = [NSTask launchedTaskWithLaunchPath: @"mount"
                                              arguments: [NSArray arrayWithObject: media]];
      
      if (task)
        {
          [task waitUntilExit];
      
          if ([task terminationStatus] == 0)
            {
              NSDictionary *userinfo = [NSDictionary dictionaryWithObject: media 
                                                                   forKey: @"NSDevicePath"];

              [[self notificationCenter] postNotificationName: NSWorkspaceDidMountNotification
                                                       object: self
                                                     userInfo: userinfo];
              
              [newlyMountedMedia addObject: media];
            }
        }
    }

  return newlyMountedMedia;
}

- (NSArray*) mountedRemovableMedia
{
  NSArray		*volumes;
  NSMutableArray	*names;
  NSUInteger		count;
  NSUInteger		i;

  volumes = [self mountedLocalVolumePaths];
  count = [volumes count];
  names = [NSMutableArray arrayWithCapacity: count];
  for (i = 0; i < count; i++)
    {
      BOOL	removableFlag;
      BOOL	writableFlag;
      BOOL	unmountableFlag;
      NSString	*description;
      NSString	*fileSystemType;
      NSString	*name = [volumes objectAtIndex: i];

      if ([self getFileSystemInfoForPath: name
			     isRemovable: &removableFlag
			      isWritable: &writableFlag
			   isUnmountable: &unmountableFlag
			     description: &description
				    type: &fileSystemType] && removableFlag)
        {
	  [names addObject: name];
	}
    }
  NSDebugLog(@"mountedRemovableMedia returning names: %@", names);
  return names;
}

- (NSArray*) mountedLocalVolumePaths
{
  NSMutableArray	*names;
  NSArray	        *reservedMountNames;

  // get reserved names....
  reservedMountNames = [[NSUserDefaults standardUserDefaults] objectForKey: @"GSReservedMountNames"];
  if (reservedMountNames == nil)
    {
      reservedMountNames = [NSArray arrayWithObjects:
				      @"proc",@"devpts",
				    @"shm",@"usbdevfs",
				    @"devtmpfs", @"devpts",@"sysfs",
				    @"tmpfs",@"procbususb",
				    @"udev", @"pstore",
				    @"cgroup", nil];
      [[NSUserDefaults standardUserDefaults] setObject: reservedMountNames 
						forKey: @"GSReservedMountNames"];
    }

#if	defined(__MINGW32__)
  NSFileManager		*mgr = [NSFileManager defaultManager];
  unsigned		max = BUFSIZ;
  unichar		buf[max];
  unichar		*base = buf;
  unichar		*ptr;
  unichar		*end;
  unsigned		len;

  names = [NSMutableArray arrayWithCapacity: 8];
  len = GetLogicalDriveStringsW(max-1, base);
  while (len >= max)
    {
      base = NSZoneMalloc(NSDefaultMallocZone(), (len+1) * sizeof(unichar));
      max = len;
      len = GetLogicalDriveStringsW(max-1, base);
    }
  for (ptr = base; *ptr != 0; ptr = end + 1)
    {
      NSString	*path;

      end = ptr;
      while (*end != 0)
	{
	  end++;
	}
      len = (end - ptr);
      path = [mgr stringWithFileSystemRepresentation: ptr length: len];
      [names addObject: path];
    }
  if (base != buf)
    {
      NSZoneFree(NSDefaultMallocZone(), base);
    }

#elif defined (HAVE_GETMNTINFO)
  NSFileManager	*mgr = [NSFileManager defaultManager];
  unsigned int	i, n;
#if defined(HAVE_STATVFS) && defined (__NetBSD__)
  struct statvfs *m;
#else
  struct statfs	*m;
#endif
  
  n = getmntinfo(&m, MNT_NOWAIT);
  names = [NSMutableArray arrayWithCapacity: n];
  for (i = 0; i < n; i++)
    {
      /* NB For now assume that all local volumes are mounted from a device
         with an entry /dev and this is not the case for any pseudo
	 filesystems.
      */
      if (strncmp(m[i].f_mntfromname, "/dev/", 5) == 0)
        {
	  [names addObject:
		   [mgr stringWithFileSystemRepresentation: m[i].f_mntonname
			length: strlen(m[i].f_mntonname)]];
        }
    }
#elif	defined(HAVE_GETMNTENT) && defined (MNT_MEMB)

  NSFileManager	*mgr = [NSFileManager defaultManager];
  FILE		*fptr = setmntent(MOUNTED_PATH, "r");
  struct mntent	*m;

  names = [NSMutableArray arrayWithCapacity: 8];
  while ((m = getmntent(fptr)) != 0)
    {
      NSString	*path;
      NSString  *type;

      path = [mgr stringWithFileSystemRepresentation: m->MNT_MEMB
					      length: strlen(m->MNT_MEMB)];
      type = [NSString stringWithCString:m->mnt_type];
      if ([reservedMountNames containsObject: type] == NO)
	{
	  [names addObject: path];
	}
    }
  endmntent(fptr);
#else
  /* we resort in parsing mtab manually and removing then reserved mount names
     defined in preferences GSReservedMountNames (SystemPreferences) */
  NSString	*mtabPath;
  NSString	*mtab;
  NSArray	*mounts;
  unsigned int	i;

  // get mount table...
  mtabPath = [[NSUserDefaults standardUserDefaults] objectForKey:@"GSMtabPath"];
  if (mtabPath == nil)
    {
      mtabPath = @"/etc/mtab";
    }

  mtab = [NSString stringWithContentsOfFile: mtabPath];
  mounts = [mtab componentsSeparatedByString: @"\n"];

  names = [NSMutableArray arrayWithCapacity: [mounts count]];
  for (i = 0; i < [mounts count]; i++)
    {
      NSString  *mount = [mounts objectAtIndex: i];
    
      if ([mount length]) 
        {
          NSArray  *parts = [mount componentsSeparatedByString: @" "];
          
          if ([parts count] >= 2) 
            {          
              NSString	*type = [parts objectAtIndex: 2];
              if ([reservedMountNames containsObject: type] == NO)
              {
	         [names addObject: [parts objectAtIndex: 1]];
	      }
           }
        }
    }
#endif
  NSDebugLog(@"mountedLocalVolumePaths returning names: %@", names);
  return names;
}

/**
 * Returns the workspace notification center
 */
- (NSNotificationCenter*) notificationCenter
{
  return _workspaceCenter;
}

/**
 * Simply makes a note that the user defaults database has changed.
 */
- (void) noteUserDefaultsChanged
{
  _userDefaultsChanged = YES;
}

/**
 * Returns a flag to say if the defaults database has changed since
 * the last time this method was called.
 */
- (BOOL) userDefaultsChanged
{
  BOOL	hasChanged = _userDefaultsChanged;

  _userDefaultsChanged = NO;
  return hasChanged;
}

/**
 * Animating an Image- slides it from one point on the screen to another.
 */
- (void) slideImage: (NSImage*)image
	       from: (NSPoint)fromPoint
		 to: (NSPoint)toPoint
{
  [GSCurrentServer() slideImage: image from: fromPoint to: toPoint];
}

/*
 * Requesting Additional Time before Power Off or Logout<br />
 * Returns the amount of time actually granted (which may be less than
 * requested).<br />
 * Times are measured in milliseconds.
 */
- (int) extendPowerOffBy: (int)requested
{
  id	app;

  NS_DURING
    {
      if ((app = [self _workspaceApplication]) != nil)
	{
	  int	result;

	  result = [app extendPowerOffBy: requested];
	  NS_VALRETURN(result);
	}
    }
  NS_HANDLER
    // workspace manager problem ... fall through to default code
  NS_ENDHANDLER

  return 0;
}

- (BOOL) filenameExtension: (NSString *)filenameExtension 
            isValidForType: (NSString*)typeName
{
  // FIXME
  return [filenameExtension isEqualToString: typeName];
}

- (NSString *) localizedDescriptionForType: (NSString *)typeName
{
  // FIXME
  return typeName;
}

- (NSString *) preferredFilenameExtensionForType: (NSString *)typeName
{
  // FIXME
  return typeName;
}

- (BOOL) type: (NSString *)firstTypeName conformsToType: (NSString *)secondTypeName
{
  // FIXME
  return [firstTypeName isEqualToString: secondTypeName];
}

- (NSString *) typeOfFile: (NSString *)absoluteFilePath error: (NSError **)outError
{
  // FIXME
  return [absoluteFilePath pathExtension];
}

@end

@implementation	NSWorkspace (GNUstep)

/**
 * Returns the 'best' application to open a file with the specified extension
 * using the given role.  If the role is nil then apps which can edit are
 * preferred but viewers are also acceptable.  Uses a user preferred app
 * or picks any good match.
 */
- (NSString*) getBestAppInRole: (NSString*)role
		  forExtension: (NSString*)ext
{
  NSString	*appName = nil;

  if ([self _extension: ext role: role app: &appName] == NO)
    {
      appName = nil;
    }
  return appName;
}

/**
 * Returns the path set for the icon matching the image by
 * -setBestIcon:forExtension:
 */
- (NSString*) getBestIconForExtension: (NSString*)ext
{
  NSString	*iconPath = nil;

  if (extPreferences != nil)
    {
      NSDictionary	*inf;

      inf = [extPreferences objectForKey: [ext lowercaseString]];
      if (inf != nil)
	{
	  iconPath = [inf objectForKey: @"Icon"];
	}
    }
  return iconPath;
}

/**
 * Gets the applications cache (generated by the make_services tool)
 * and looks up the special entry that contains a dictionary of all
 * file extensions recognised by GNUstep applications.  Then finds
 * the dictionary of applications that can handle our file and
 * returns it.
 */
- (NSDictionary*) infoForExtension: (NSString*)ext
{
  NSDictionary  *map;

  ext = [ext lowercaseString];
  map = [applications objectForKey: @"GSExtensionsMap"];
  return [map objectForKey: ext];
}

/**
 * Returns the application bundle for the named application. Accepts
 * either a full path to an app or just the name. The extension (.app,
 * .debug, .profile) is optional, but if provided it will be used.<br />
 * Returns nil if the specified app does not exist as requested.
 */
- (NSBundle*) bundleForApp: (NSString*)appName
{
  if ([appName length] == 0)
    {
      return nil;
    }
  if ([[appName lastPathComponent] isEqual: appName]) // it's a name
    {
      appName = [self fullPathForApplication: appName];
    }
  else
    {
      NSFileManager	*fm;
      NSString		*ext;
      BOOL		flag;

      fm = [NSFileManager defaultManager];
      ext = [appName pathExtension];
      if ([ext length] == 0) // no extension, let's find one
	{
	  NSString	*path;

	  path = [appName stringByAppendingPathExtension: @"app"];
	  if ([fm fileExistsAtPath: path isDirectory: &flag] == NO
	    || flag == NO)
	    {
	      path = [appName stringByAppendingPathExtension: @"debug"];
	      if ([fm fileExistsAtPath: path isDirectory: &flag] == NO
		|| flag == NO)
		{
		  path = [appName stringByAppendingPathExtension: @"profile"];
		}
	    }
	  appName = path;
	}
      if ([fm fileExistsAtPath: appName isDirectory: &flag] == NO
	|| flag == NO)
	{
	  appName = nil;
	}
    }
  if (appName == nil)
    {
      return nil;
    }
  return [NSBundle bundleWithPath: appName];
}

/**
 * Returns the application icon for the given app.
 * Or null if none defined or appName is not a valid application name.
 */
- (NSImage*) appIconForApp: (NSString*)appName
{
  NSBundle *bundle;
  NSImage *image = nil;
  NSFileManager *mgr = [NSFileManager defaultManager];
  NSString *iconPath = nil;
  NSString *fullPath;
  
  fullPath = [self fullPathForApplication: appName];
  bundle = [self bundleForApp: fullPath];
  if (bundle == nil)
    {
      return nil;
    }
  
  iconPath = [[bundle infoDictionary] objectForKey: @"NSIcon"];
  if (iconPath == nil)
    {
      /*
       * Try the CFBundleIconFile property.
       */
      iconPath = [[bundle infoDictionary] objectForKey: @"CFBundleIconFile"];
    }

  if (iconPath && [iconPath isAbsolutePath] == NO)
    {
      NSString *file = iconPath;

      iconPath = [bundle pathForImageResource: file];

      /*
       * If there is no icon in the Resources of the app, try
       * looking directly in the app wrapper.
       */
      if (iconPath == nil)
        {
          iconPath = [fullPath stringByAppendingPathComponent: file];
          if ([mgr isReadableFileAtPath: iconPath] == NO)
            {
              iconPath = nil;
            }
        }
    }
    
  /*
   * If there is no icon specified in the Info.plist for app
   * try 'wrapper/app.png'
   */
  if (iconPath == nil)
    {      
      NSString *str;

      str = [fullPath lastPathComponent];
      str = [str stringByDeletingPathExtension];
      iconPath = [fullPath stringByAppendingPathComponent: str];
      iconPath = [iconPath stringByAppendingPathExtension: @"png"];
      if ([mgr isReadableFileAtPath: iconPath] == NO)
        {
	  iconPath = [iconPath stringByAppendingPathExtension: @"tiff"];
	  if ([mgr isReadableFileAtPath: iconPath] == NO)
	    {
	      iconPath = [iconPath stringByAppendingPathExtension: @"icns"];
	      if ([mgr isReadableFileAtPath: iconPath] == NO)
		{		  
		  iconPath = nil;
		}
	    }
        }
    }

  if (iconPath != nil)
    {
      image = [self _saveImageFor: iconPath];
    }
  
  return image;
}

/**
 * Requires the path to an application wrapper as an argument, and returns
 * the full path to the executable.
 */
- (NSString*) locateApplicationBinary: (NSString*)appName
{
  NSString	*path;
  NSString	*file;
  NSBundle	*bundle = [self bundleForApp: appName];

  if (bundle == nil)
    {
      return nil;
    }
  path = [bundle bundlePath];
  file = [[bundle infoDictionary] objectForKey: @"NSExecutable"];

  if (file == nil)
    {
      /*
       * If there is no executable specified in the info property-list, then
       * we expect the executable to reside within the app wrapper and to
       * have the same name as the app wrapper but without the extension.
       */
      file = [path lastPathComponent];
      file = [file stringByDeletingPathExtension];
      path = [path stringByAppendingPathComponent: file];
    }
  else
    {
      /*
       * If there is an executable specified in the info property-list, then
       * it can be either an absolute path, or a path relative to the app
       * wrapper, so we make sure we end up with an absolute path to return.
       */
      if ([file isAbsolutePath] == YES)
	{
	  path = file;
	}
      else
	{
	  path = [path stringByAppendingPathComponent: file];
	}
    }

  return path;
}

/**
 * Sets up a user preference  for which app should be used to open files
 * of the specified extension.
 */
- (void) setBestApp: (NSString*)appName
	     inRole: (NSString*)role
       forExtension: (NSString*)ext
{
  NSMutableDictionary	*map;
  NSMutableDictionary	*inf;
  NSData		*data;

  ext = [ext lowercaseString];
  if (extPreferences != nil)
    map = [extPreferences mutableCopy];
  else
    map = [NSMutableDictionary new];

  inf = [[map objectForKey: ext] mutableCopy];
  if (inf == nil)
    {
      inf = [NSMutableDictionary new];
    }
  if (appName == nil)
    {
      if (role == nil)
	{
	  NSString	*iconPath = [inf objectForKey: @"Icon"];

	  RETAIN(iconPath);
	  [inf removeAllObjects];
	  if (iconPath)
	    {
	      [inf setObject: iconPath forKey: @"Icon"];
	      RELEASE(iconPath);
	    }
	}
      else
	{
	  [inf removeObjectForKey: role];
	}
    }
  else
    {
      [inf setObject: appName forKey: (role ? (id)role : (id)@"Editor")];
    }
  [map setObject: inf forKey: ext];
  RELEASE(inf);
  RELEASE(extPreferences);
  extPreferences = map;
  data = [NSSerializer serializePropertyList: extPreferences];
  if ([data writeToFile: extPrefPath atomically: YES])
    {
      [_workspaceCenter postNotificationName: GSWorkspacePreferencesChanged
				      object: self];
    }
  else
    {
      NSLog(@"Update %@ of failed", extPrefPath);
    }
}

/**
 * Sets up a user preference for which icon should be used to
 * represent the specified file extension.
 */
- (void) setBestIcon: (NSString*)iconPath forExtension: (NSString*)ext
{
  NSMutableDictionary	*map;
  NSMutableDictionary	*inf;
  NSData		*data;

  ext = [ext lowercaseString];
  if (extPreferences != nil)
    map = [extPreferences mutableCopy];
  else
    map = [NSMutableDictionary new];

  inf = [[map objectForKey: ext] mutableCopy];
  if (inf == nil)
    inf = [NSMutableDictionary new];
  if (iconPath)
    [inf setObject: iconPath forKey: @"Icon"];
  else
    [inf removeObjectForKey: @"Icon"];
  [map setObject: inf forKey: ext];
  RELEASE(inf);
  RELEASE(extPreferences);
  extPreferences = map;
  data = [NSSerializer serializePropertyList: extPreferences];
  if ([data writeToFile: extPrefPath atomically: YES])
    {
      [_workspaceCenter postNotificationName: GSWorkspacePreferencesChanged
				      object: self];
    }
  else
    {
      NSLog(@"Update %@ of failed", extPrefPath);
    }
}

/**
 * Gets the applications cache (generated by the make_services tool)
 * and looks up the special entry that contains a dictionary of all
 * URL schemes recognised by GNUstep applications.  Then finds the
 * dictionary of applications that can handle our scheme and returns
 * it.
 */
- (NSDictionary*) infoForScheme: (NSString*)scheme
{
  NSDictionary  *map;

  scheme = [scheme lowercaseString];
  map = [applications objectForKey: @"GSSchemesMap"];
  return [map objectForKey: scheme];
}

/**
 * Returns the 'best' application to open a file with the specified URL
 * scheme using the given role.  If the role is nil then apps which can
 * edit are preferred but viewers are also acceptable.  Uses a user preferred
 * app or picks any good match.
 */
- (NSString*) getBestAppInRole: (NSString*)role
		     forScheme: (NSString*)scheme
{
  NSString	*appName = nil;

  if ([self _scheme: scheme role: role app: &appName] == NO)
    {
      appName = nil;
    }
  return appName;
}

/**
 * Sets up a user preference for which app should be used to open files
 * of the specified URL scheme
 */
- (void) setBestApp: (NSString*)appName
	     inRole: (NSString*)role
	  forScheme: (NSString*)scheme
{
  NSMutableDictionary	*map;
  NSMutableDictionary	*inf;
  NSData		*data;

  scheme = [scheme lowercaseString];
  if (urlPreferences != nil)
    map = [urlPreferences mutableCopy];
  else
    map = [NSMutableDictionary new];

  inf = [[map objectForKey: scheme] mutableCopy];
  if (inf == nil)
    {
      inf = [NSMutableDictionary new];
    }
  if (appName == nil)
    {
      if (role == nil)
	{
	  NSString	*iconPath = [inf objectForKey: @"Icon"];

	  RETAIN(iconPath);
	  [inf removeAllObjects];
	  if (iconPath)
	    {
	      [inf setObject: iconPath forKey: @"Icon"];
	      RELEASE(iconPath);
	    }
	}
      else
	{
	  [inf removeObjectForKey: role];
	}
    }
  else
    {
      [inf setObject: appName forKey: (role ? (id)role : (id)@"Editor")];
    }
  [map setObject: inf forKey: scheme];
  RELEASE(inf);
  RELEASE(urlPreferences);
  urlPreferences = map;
  data = [NSSerializer serializePropertyList: urlPreferences];
  if ([data writeToFile: urlPrefPath atomically: YES])
    {
      [_workspaceCenter postNotificationName: GSWorkspacePreferencesChanged
				      object: self];
    }
  else
    {
      NSLog(@"Update %@ of failed", urlPrefPath);
    }
}

@end

@implementation NSWorkspace (Private)

- (NSImage*) _extIconForApp: (NSString*)appName info: (NSDictionary*)extInfo
{
  NSDictionary	*typeInfo = [extInfo objectForKey: appName];
  NSString	*file = [typeInfo objectForKey: @"NSIcon"];
  
  /*
   * If the NSIcon entry isn't there and the CFBundle entries are,
   * get the first icon in the list if it's an array, or assign
   * the icon to file if it's a string.
   *
   * FIXME: CFBundleTypeExtensions/IconFile can be arrays which assign
   * multiple types to icons.  This needs to be handled eventually.
   */
  if (file == nil)
    {
      id icon = [typeInfo objectForKey: @"CFBundleTypeIconFile"];
      if ([icon isKindOfClass: [NSArray class]])
	{
	  if ([icon count])
	    {
	      file = [icon objectAtIndex: 0];
	    }
	}
      else
	{
	  file = icon;
	}
    }

  if (file && [file length] != 0)
    {
      if ([file isAbsolutePath] == NO)
	{
	  NSString *iconPath;
	  NSBundle *bundle;

	  bundle = [self bundleForApp: appName];
	  iconPath = [bundle pathForImageResource: file];
	  /*
	   * If the icon is not in the Resources of the app, try looking
	   * directly in the app wrapper.
	   */
	  if (iconPath == nil)
	    {
	      iconPath = [[bundle bundlePath]
		stringByAppendingPathComponent: file];
	    }
	  file = iconPath;
	}
      if ([[NSFileManager defaultManager] isReadableFileAtPath: file] == YES)
	{
	  return [self _saveImageFor: file];
	}
    }
  return nil;
}

/** Returns the default icon to display for a file */
- (NSImage*) unknownFiletypeImage
{
  static NSImage *image = nil;

  if (image == nil)
    {
      image = RETAIN([NSImage _standardImageWithName: @"Unknown"]);
    }

  return image;
}

/** Try to create the image in an exception handling context */
- (NSImage*) _saveImageFor: (NSString*)iconPath
{
  NSImage *tmp = nil;

  NS_DURING
    {
      tmp = [[NSImage alloc] initWithContentsOfFile: iconPath];
      if (tmp != nil)
        {
	  AUTORELEASE(tmp);
	}
    }
  NS_HANDLER
    {
      NSLog(@"BAD TIFF FILE '%@'", iconPath);
    }
  NS_ENDHANDLER

  return tmp;
}

/** Returns the freedesktop thumbnail file name for a given file name */
- (NSString*) thumbnailForFile: (NSString *)file
{
  NSString *absolute;
  NSString *digest;
  NSString *thumbnail;

  absolute = [[NSURL fileURLWithPath: [file stringByStandardizingPath]] 
		 absoluteString];
  /* This compensates for a feature we have in NSURL, that is there to have 
   * MacOSX compatibility.
   */
  if ([absolute hasPrefix:  @"file://localhost/"])
    {
      absolute = [@"file:///" stringByAppendingString: 
		       [absolute substringWithRange: 
				     NSMakeRange(17, [absolute length] - 17)]];
    }

  // FIXME: Not sure which encoding to use here. 
  digest = [[[[absolute dataUsingEncoding: NSASCIIStringEncoding]
		 md5Digest] hexadecimalRepresentation] lowercaseString];
  thumbnail = [@"~/.thumbnails/normal" stringByAppendingPathComponent: 
		    [digest stringByAppendingPathExtension: @"png"]];

  return [thumbnail stringByStandardizingPath];
}

- (NSImage*) _iconForExtension: (NSString*)ext
{
  NSImage	*icon = nil;

  if (ext == nil || [ext isEqualToString: @""])
    {
      return nil;
    }
  /*
   * extensions are case-insensitive - convert to lowercase.
   */
  ext = [ext lowercaseString];
  if ((icon = [_iconMap objectForKey: ext]) == nil)
    {
      NSDictionary	*prefs;
      NSDictionary	*extInfo;
      NSString		*iconPath;

      /*
       * If there is a user-specified preference for an image -
       * try to use that one.
       */
      prefs = [extPreferences objectForKey: ext];
      iconPath = [prefs objectForKey: @"Icon"];
      if (iconPath)
	{
	  icon = [self _saveImageFor: iconPath];
	}

      if (icon == nil && (extInfo = [self infoForExtension: ext]) != nil)
	{
	  NSString	*appName;

	  /*
	   * If there are any application preferences given, try to use the
	   * icon for this file that is used by the preferred app.
	   */
	  if (prefs)
	    {
	      if ((appName = [prefs objectForKey: @"Editor"]) != nil)
		{
		  icon = [self _extIconForApp: appName info: extInfo];
		}
	      if (icon == nil
		&& (appName = [prefs objectForKey: @"Viewer"]) != nil)
		{
		  icon = [self _extIconForApp: appName info: extInfo];
		}
	    }

	  if (icon == nil)
	    {
	      NSEnumerator	*enumerator;

	      /*
	       * Still no icon - try all the apps that handle this file
	       * extension.
	       */
	      enumerator = [extInfo keyEnumerator];
	      while (icon == nil && (appName = [enumerator nextObject]) != nil)
		{
		  icon = [self _extIconForApp: appName info: extInfo];
		}
	    }
	}

      /*
       * Nothing found at all - use the unknowntype icon.
       */
      if (icon == nil)
	{
	  if ([ext isEqualToString: @"app"] == YES
	    || [ext isEqualToString: @"debug"] == YES
	    || [ext isEqualToString: @"profile"] == YES)
	    {
	      if (unknownApplication == nil)
		{
		  unknownApplication = RETAIN([NSImage _standardImageWithName:
		    @"UnknownApplication"]);
		}
	      icon = unknownApplication;
	    }
	  else
	    {
	      icon = [self unknownFiletypeImage];
	    }
	}

      /*
       * Set the icon in the cache for next time.
       */
      if (icon != nil)
	{
	  [_iconMap setObject: icon forKey: ext];
	}
    }
  return icon;
}

- (BOOL) _extension: (NSString*)ext
               role: (NSString*)role
	        app: (NSString**)app
{
  NSEnumerator	*enumerator;
  NSString      *appName = nil;
  NSDictionary	*apps = [self infoForExtension: ext];
  NSDictionary	*prefs;
  NSDictionary	*info;

  ext = [ext lowercaseString];

  /*
   *	Look for the name of the preferred app in this role.
   *	A 'nil' roll is a wildcard - find the preferred Editor or Viewer.
   */
  prefs = [extPreferences objectForKey: ext];
  if (role == nil || [role isEqualToString: @"Editor"])
    {
      appName = [prefs objectForKey: @"Editor"];
      if (appName != nil)
	{
	  info = [apps objectForKey: appName];
	  if (info != nil)
	    {
	      if (app != 0)
		{
		  *app = appName;
		}
	      return YES;
	    }
	  else if ([self locateApplicationBinary: appName] != nil)
	    {
	      /*
	       * Return the preferred application even though it doesn't
	       * say it opens this type of file ... preferences overrule.
	       */
	      if (app != 0)
		{
		  *app = appName;
		}
	      return YES;
	    }
	}
    }
  if (role == nil || [role isEqualToString: @"Viewer"])
    {
      appName = [prefs objectForKey: @"Viewer"];
      if (appName != nil)
	{
	  info = [apps objectForKey: appName];
	  if (info != nil)
	    {
	      if (app != 0)
		{
		  *app = appName;
		}
	      return YES;
	    }
	  else if ([self locateApplicationBinary: appName] != nil)
	    {
	      /*
	       * Return the preferred application even though it doesn't
	       * say it opens this type of file ... preferences overrule.
	       */
	      if (app != 0)
		{
		  *app = appName;
		}
	      return YES;
	    }
	}
    }

  /*
   * Go through the dictionary of apps that know about this file type and
   * determine the best application to open the file by examining the
   * type information for each app.
   * The 'NSRole' field specifies what the app can do with the file - if it
   * is missing, we assume an 'Editor' role.
   */
  if (apps == nil || [apps count] == 0)
    {
      return NO;
    }
  enumerator = [apps keyEnumerator];

  if (role == nil)
    {
      BOOL	found = NO;

      /*
       * If the requested role is 'nil', we can accept an app that is either
       * an Editor (preferred) or a Viewer, or unknown.
       */
      while ((appName = [enumerator nextObject]) != nil)
	{
	  NSString	*str;

	  info = [apps objectForKey: appName];
	  str = [info objectForKey: @"NSRole"];
	  /* NB. If str is nil or an empty string, there is no role set,
	   * and we treat this as an Editor since the role is unrestricted.
	   */
	  if ([str length] == 0 || [str isEqualToString: @"Editor"])
	    {
	      if (app != 0)
		{
		  *app = appName;
		}
	      return YES;
	    }
	  if ([str isEqualToString: @"Viewer"])
	    {
	      if (app != 0)
		{
		  *app = appName;
		}
	      found = YES;
	    }
	}
      return found;
    }
  else
    {
      while ((appName = [enumerator nextObject]) != nil)
	{
	  NSString	*str;

	  info = [apps objectForKey: appName];
	  str = [info objectForKey: @"NSRole"];
	  if ((str == nil && [role isEqualToString: @"Editor"])
	    || [str isEqualToString: role])
	    {
	      if (app != 0)
		{
		  *app = appName;
		}
	      return YES;
	    }
	}
      return NO;
    }
}

- (BOOL) _scheme: (NSString*)scheme
	    role: (NSString*)role
	     app: (NSString**)app
{
  NSEnumerator	*enumerator;
  NSString      *appName = nil;
  NSDictionary	*apps = [self infoForScheme: scheme];
  NSDictionary	*prefs;
  NSDictionary	*info;

  scheme = [scheme lowercaseString];

  /*
   *	Look for the name of the preferred app in this role.
   *	A 'nil' roll is a wildcard - find the preferred Editor or Viewer.
   */
  prefs = [urlPreferences objectForKey: scheme];
  if (role == nil || [role isEqualToString: @"Editor"])
    {
      appName = [prefs objectForKey: @"Editor"];
      if (appName != nil)
	{
	  info = [apps objectForKey: appName];
	  if (info != nil)
	    {
	      if (app != 0)
		{
		  *app = appName;
		}
	      return YES;
	    }
	  else if ([self locateApplicationBinary: appName] != nil)
	    {
	      /*
	       * Return the preferred application even though it doesn't
	       * say it opens this type of file ... preferences overrule.
	       */
	      if (app != 0)
		{
		  *app = appName;
		}
	      return YES;
	    }
	}
    }
  if (role == nil || [role isEqualToString: @"Viewer"])
    {
      appName = [prefs objectForKey: @"Viewer"];
      if (appName != nil)
	{
	  info = [apps objectForKey: appName];
	  if (info != nil)
	    {
	      if (app != 0)
		{
		  *app = appName;
		}
	      return YES;
	    }
	  else if ([self locateApplicationBinary: appName] != nil)
	    {
	      /*
	       * Return the preferred application even though it doesn't
	       * say it opens this type of file ... preferences overrule.
	       */
	      if (app != 0)
		{
		  *app = appName;
		}
	      return YES;
	    }
	}
    }

  /*
   * Go through the dictionary of apps that know about this file type and
   * determine the best application to open the file by examining the
   * type information for each app.
   * The 'NSRole' field specifies what the app can do with the file - if it
   * is missing, we assume an 'Editor' role.
   */
  if (apps == nil || [apps count] == 0)
    {
      return NO;
    }
  enumerator = [apps keyEnumerator];

  if (role == nil)
    {
      BOOL	found = NO;

      /*
       * If the requested role is 'nil', we can accept an app that is either
       * an Editor (preferred) or a Viewer, or unknown.
       */
      while ((appName = [enumerator nextObject]) != nil)
	{
	  NSString	*str;

	  info = [apps objectForKey: appName];
	  str = [info objectForKey: @"NSRole"];
	  /* NB. If str is nil or an empty string, there is no role set,
	   * and we treat this as an Editor since the role is unrestricted.
	   */
	  if ([str length] == 0 || [str isEqualToString: @"Editor"])
	    {
	      if (app != 0)
		{
		  *app = appName;
		}
	      return YES;
	    }
	  if ([str isEqualToString: @"Viewer"])
	    {
	      if (app != 0)
		{
		  *app = appName;
		}
	      found = YES;
	    }
	}
      return found;
    }
  else
    {
      while ((appName = [enumerator nextObject]) != nil)
	{
	  NSString	*str;

	  info = [apps objectForKey: appName];
	  str = [info objectForKey: @"NSRole"];
	  if ((str == nil && [role isEqualToString: @"Editor"])
	    || [str isEqualToString: role])
	    {
	      if (app != 0)
		{
		  *app = appName;
		}
	      return YES;
	    }
	}
      return NO;
    }
}

- (void) _workspacePreferencesChanged: (NSNotification *)aNotification
{
  /* FIXME reload only those preferences that really were changed
   * TODO  add a user info to aNotification, which includes a bitmask
   *       denoting the updated preference files.
   */
  NSFileManager		*mgr = [NSFileManager defaultManager];
  NSData		*data;
  NSDictionary		*dict;

  if ([mgr isReadableFileAtPath: extPrefPath] == YES)
    {
      data = [NSData dataWithContentsOfFile: extPrefPath];
      if (data)
	{
	  dict = [NSDeserializer deserializePropertyListFromData: data
					       mutableContainers: NO];
	  ASSIGN(extPreferences, dict);
	}
    }

  if ([mgr isReadableFileAtPath: urlPrefPath] == YES)
    {
      data = [NSData dataWithContentsOfFile: urlPrefPath];
      if (data)
	{
	  dict = [NSDeserializer deserializePropertyListFromData: data
					       mutableContainers: NO];
	  ASSIGN(urlPreferences, dict);
	}
    }

  if ([mgr isReadableFileAtPath: appListPath] == YES)
    {
      data = [NSData dataWithContentsOfFile: appListPath];
      if (data)
	{
	  dict = [NSDeserializer deserializePropertyListFromData: data
					       mutableContainers: NO];
	  ASSIGN(applications, dict);
	}
    }
  /*
   *	Invalidate the cache of icons for file extensions.
   */
  [_iconMap removeAllObjects];
}


/**
 * Launch an application locally (ie without reference to the workspace
 * manager application).  We should only call this method when we want
 * the application launched by this process, either because what we are
 * launching IS the workspace manager, or because we have tried to get
 * the workspace manager to do the job and been unable to do so.
 */
- (BOOL) _launchApplication: (NSString*)appName
		  arguments: (NSArray*)args
{
  NSTask	*task;
  NSString	*path;
  NSDictionary	*userinfo;
  NSString	*host;

  path = [self locateApplicationBinary: appName];
  if (path == nil)
    {
      return NO;
    }

  /*
   * Try to ensure that apps we launch display in this workspace
   * ie they have the same -NSHost specification.
   */
  host = [[NSUserDefaults standardUserDefaults] stringForKey: @"NSHost"];
  if (host != nil)
    {
      NSHost	*h;

      h = [NSHost hostWithName: host];
      if ([h isEqual: [NSHost currentHost]] == NO)
	{
	  if ([args containsObject: @"-NSHost"] == NO)
	    {
	      NSMutableArray	*a;

	      if (args == nil)
		{
		  a = [NSMutableArray arrayWithCapacity: 2];
		}
	      else
		{
		  a = AUTORELEASE([args mutableCopy]);
		}
	      [a insertObject: @"-NSHost" atIndex: 0];
	      [a insertObject: host atIndex: 1];
	      args = a;
	    }
	}
    }
  /*
   * App being launched, send
   * NSWorkspaceWillLaunchApplicationNotification
   */
  userinfo = [NSDictionary dictionaryWithObjectsAndKeys:
    [[appName lastPathComponent] stringByDeletingPathExtension], 
			   @"NSApplicationName",
    appName, @"NSApplicationPath",
    nil];
  [_workspaceCenter
    postNotificationName: NSWorkspaceWillLaunchApplicationNotification
    object: self
    userInfo: userinfo];

  task = [NSTask launchedTaskWithLaunchPath: path arguments: args];
  if (task == nil)
    {
      return NO;
    }
  /*
   * The NSWorkspaceDidLaunchApplicationNotification will be
   * sent by the started application itself.
   */
  [_launched setObject: task forKey: appName];
  return YES;
}

- (id) _connectApplication: (NSString*)appName
{
  NSTimeInterval        replyTimeout = 0.0;
  NSTimeInterval        requestTimeout = 0.0;
  NSString	*host;
  NSString	*port;
  NSDate	*when = nil;
  NSConnection  *conn = nil;
  id		app = nil;

  while (app == nil)
    {
      host = [[NSUserDefaults standardUserDefaults] stringForKey: @"NSHost"];
      if (host == nil)
	{
	  host = @"";
	}
      else
	{
	  NSHost	*h;

	  h = [NSHost hostWithName: host];
	  if ([h isEqual: [NSHost currentHost]] == YES)
	    {
	      host = @"";
	    }
	}
      port = [[appName lastPathComponent] stringByDeletingPathExtension];
      /*
       *	Try to contact a running application.
       */
      NS_DURING
	{
          conn = [NSConnection connectionWithRegisteredName: port host: host];
          requestTimeout = [conn requestTimeout];
          [conn setRequestTimeout: 5.0];
          replyTimeout = [conn replyTimeout];
          [conn setReplyTimeout: 5.0];
	  app = [conn rootProxy];
	}
      NS_HANDLER
	{
	  /* Fatal error in DO	*/
          conn = nil;
	  app = nil;
	}
      NS_ENDHANDLER

      if (app == nil)
	{
	  NSTask	*task = [_launched objectForKey: appName];
	  NSDate	*limit;

	  if (task == nil || [task isRunning] == NO)
	    {
	      if (task != nil)	// Not running
		{
		  [_launched removeObjectForKey: appName];
		}
	      break;		// Need to launch the app
	    }

	  if (when == nil)
	    {
	      when = [[NSDate alloc] init];
	    }
	  else if ([when timeIntervalSinceNow] < -5.0)
	    {
	      int		result;

	      DESTROY(when);
              result = NSRunAlertPanel(appName,
                @"Application seems to have hung",
                @"Continue", @"Terminate", @"Wait");

	      if (result == NSAlertDefaultReturn)
		{
		  break;		// Finished without app
		}
	      else if (result == NSAlertOtherReturn)
		{
		  // Continue to wait for app startup.
		}
	      else
		{
		  [task terminate];
		  [_launched removeObjectForKey: appName];
		  break;		// Terminate hung app
		}
	    }

	  // Give it another 0.5 of a second to start up.
	  limit = [[NSDate alloc] initWithTimeIntervalSinceNow: 0.5];
	  [[NSRunLoop currentRunLoop] runUntilDate: limit];
	  RELEASE(limit);
	}
    }
  if (conn != nil)
    {
      /* Use original timeouts
       */
      [conn setRequestTimeout: requestTimeout];
      [conn setReplyTimeout: replyTimeout];
    }
  TEST_RELEASE(when);
  return app;
}

- (id) _workspaceApplication
{
  static NSUserDefaults		*defs = nil;
  static GSServicesManager	*smgr = nil;
  NSString			*appName;
  NSString			*myName;
  id				app;

  if (defs == nil)
    {
      defs = RETAIN([NSUserDefaults standardUserDefaults]);
    }
  if (smgr == nil)
    {
      smgr = RETAIN([GSServicesManager manager]);
    }
  /* What Workspace application? */
  appName = [defs stringForKey: @"GSWorkspaceApplication"];
  if (appName == nil)
    {
      appName = @"GWorkspace";
    }
  /*
   * If this app is the workspace app, there is no sense contacting
   * it as it would cause recursion ... so we return nil.
   */
  myName = [smgr port];
  if ([appName isEqual: myName] == YES)
    {
      return nil;
    }

  app = [self _connectApplication: appName];
  if (app == nil)
    {
      NSString	*host;

      host = [[NSUserDefaults standardUserDefaults] stringForKey: @"NSHost"];
      if (host == nil)
	{
	  host = @"";
	}
      else
	{
	  NSHost	*h;

	  h = [NSHost hostWithName: host];
	  if ([h isEqual: [NSHost currentHost]] == YES)
	    {
	      host = @"";
	    }
	}
      /**
       * We can only launch a workspace app if we are displaying to the
       * local host (since if we are displaying on another host we want
       * to to talk to the workspace app on that host too).
       */
      if ([host isEqual: @""] == YES)
	{
	  if ([self _launchApplication: appName
			     arguments: nil] == YES)
	    {
	      app = [self _connectApplication: appName];
	    }
	}
    }

  return app;
}

@end
