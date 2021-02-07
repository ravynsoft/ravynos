/* 
   GSServicesManager.m

   Copyright (C) 1998 Free Software Foundation, Inc.

   Author:  Richard Frith-Macdonald <richard@brainstorm.co.uk>
   Date: Novemeber 1998
  
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

#import <Foundation/NSArray.h>
#import <Foundation/NSSet.h>
#import <Foundation/NSException.h>
#import <Foundation/NSData.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSNotification.h>
#import <Foundation/NSRunLoop.h>
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSTimer.h>
#import <Foundation/NSProcessInfo.h>
#import <Foundation/NSFileManager.h>
#import <Foundation/NSConnection.h>
#import <Foundation/NSDistantObject.h>
#import <Foundation/NSMethodSignature.h>
#import <Foundation/NSPathUtilities.h>
#import <Foundation/NSUserDefaults.h>
#import <Foundation/NSSerialization.h>
#import <Foundation/NSPort.h>
#import <Foundation/NSPortNameServer.h>
#import <Foundation/NSTask.h>
#import <Foundation/NSObjCRuntime.h>
#import <Foundation/NSInvocation.h>

#import "AppKit/NSApplication.h"
#import "AppKit/NSPasteboard.h"
#import "AppKit/NSMenu.h"
#import "AppKit/NSPanel.h"
#import "AppKit/NSWindow.h"
#import "AppKit/NSWorkspace.h"
#import "AppKit/NSDocumentController.h"

#import "GNUstepGUI/GSServicesManager.h"
#import "GSGuiPrivate.h"

static GSServicesManager	*manager = nil;

/**
 *	The GSListener class is for talking to other applications.
 *	It is a proxy with some dangerous methods implemented in a
 *	harmless manner to reduce the chances of a malicious app
 *	messing with us.  This is responsible for forwarding service
 *      requests and other communications with the outside world.
 */
@interface      GSListener : NSProxy
+ (id) connectionBecameInvalid: (NSNotification*)notification;
+ (GSListener*) listener;
+ (id) servicesProvider;
+ (void) setServicesProvider: (id)anObject;
- (Class) class;
- (void) dealloc;
- (void) release;
- (id) retain;
- (void) activateIgnoringOtherApps: (BOOL)flag;
- (id) self;
@end

static NSConnection	*listenerConnection = nil;
static NSMutableArray	*listeners = nil;
static GSListener	*listener = nil;
static id		servicesProvider = nil;
static NSString		*providerName = nil;

/**
 * Unregisters the service provider registered on the named port.<br />
 * Applications should use [NSApplication-setServicesProvider:] with a nil
 * argument instead.
 */
void
NSUnregisterServicesProvider(NSString *name)
{
  if (listenerConnection != nil)
    {
      /*
       *        Ensure there is no previous listener and nothing else using
       *        the given port name.
       */
      [[NSPortNameServer systemDefaultPortNameServer] removePortForName: name];
      [[NSNotificationCenter defaultCenter]
	removeObserver: [GSListener class]
		  name: NSConnectionDidDieNotification
		object: listenerConnection];
      DESTROY(listenerConnection);
    }
  ASSIGN(servicesProvider, nil);
  ASSIGN(providerName, nil);
}

/**
 * Registers a services providing object using the specified port name.<br />
 * Applications should not need to use this, as they can use the
 * [NSApplication-setServicesProvider:] method instead.  The NSApplication
 * method will use the name of the application rather than an other port name.
 */
void
NSRegisterServicesProvider(id provider, NSString *name)
{
  NSPortNameServer	*ns;
  id			namedPort;

  if ([name length] == 0)
    {
      [NSException raise: NSInvalidArgumentException
		  format: @"NSRegisterServicesProvider() no name provided"];
    }
  if (provider == nil)
    {
      [NSException raise: NSInvalidArgumentException
		  format: @"NSRegisterServicesProvider() no provider"];
    }
  if (servicesProvider == provider && [providerName isEqual: name])
    {
      return;	// Already registered.
    }

  ns = [NSPortNameServer systemDefaultPortNameServer];
  namedPort = [ns portForName: name];
  if (namedPort && [listenerConnection receivePort] == namedPort)
    {
      [ns removePortForName: name];
      namedPort = nil;
    }
  if (namedPort != nil)
    {
      [NSException raise: NSInvalidArgumentException
		  format: @"NSRegisterServicesProvider() %@ already in use",
	name];
    }

  if (listenerConnection != nil)
    {
      [[NSNotificationCenter defaultCenter]
	removeObserver: [GSListener class]
		  name: NSConnectionDidDieNotification
		object: listenerConnection];
      DESTROY(listenerConnection);
    }

  listenerConnection = [[NSConnection alloc]
    initWithReceivePort: [NSPort port] sendPort: nil];
  [listenerConnection setRootObject: [GSListener listener]];
  if ([listenerConnection registerName: name] == NO)
    {
      DESTROY(listenerConnection);
    }

  if (listenerConnection != nil)
    {
      RETAIN(listenerConnection);
      [[NSNotificationCenter defaultCenter]
		addObserver: [GSListener class]
		   selector: @selector(connectionBecameInvalid:)
		       name: NSConnectionDidDieNotification
		     object: listenerConnection];
    }
  else
    {
      [NSException raise: NSGenericException
		  format: @"unable to register %@", name];
    }

  ASSIGN(servicesProvider, provider);
  ASSIGN(providerName, name);
}


@interface NSNotificationCenter (NSWorkspacePrivate)
- (void) _postLocal: (NSString*)name userInfo: (NSDictionary*)info;
@end

/**
 * The GSListener class exists as a proxy to forward messages to
 * service provider objects.  It implements very few methods and
 * those that it does implement are generally designed to defeat
 * any attack by a malicious program.
 */
@implementation GSListener

+ (id) connectionBecameInvalid: (NSNotification*)notification
{
  NSAssert(listenerConnection==[notification object],
	NSInternalInconsistencyException);

  [[NSNotificationCenter defaultCenter]
    removeObserver: self
	      name: NSConnectionDidDieNotification
	    object: listenerConnection];
  DESTROY(listenerConnection);
  return self;
}

+ (void) initialize
{
  static BOOL	beenHere = NO;

  if (beenHere == NO)
    {
      beenHere = YES;
      listeners = [NSMutableArray new];
    }
}

+ (GSListener*) listener
{
  if (listener == nil)
    {
      listener = (id)NSAllocateObject(self, 0, NSDefaultMallocZone());
      [listeners addObject: listener];
    }
  return listener;
}

/**
 * Needed to permit use of this class as a notification observer,
 * since the notification system caches method implementations for speed.
 */
+ (IMP) methodForSelector: (SEL)aSelector
{
  if (aSelector == 0)
    [NSException raise: NSInvalidArgumentException
                format: @"%@ null selector given", NSStringFromSelector(_cmd)];
  return class_getMethodImplementation(GSObjCClass(self), aSelector);
}

+ (id) servicesProvider
{
  return servicesProvider;
}

+ (void) setServicesProvider: (id)anObject
{
  if (servicesProvider != anObject)
    {
      NSString	*port = [[GSServicesManager manager] port];

      if (port == nil)
	{
	  port = [[NSProcessInfo processInfo] processName];
	}
      NSRegisterServicesProvider(anObject, port);
    }
}

- (id) autorelease
{
   return self;
}

- (Class) class
{
  return 0;
}

- (void) dealloc
{
  GSNOSUPERDEALLOC;
}

/**
 * Selectively forwards those messages which are thought to be safe,
 * and perform any special operations we need for workspace management
 * etc.<br />
 * The logic in this method <strong>must</strong> match that in 
 * methodSignatureForSelector:
 */
- (void) forwardInvocation: (NSInvocation*)anInvocation
{
  SEL		aSel = [anInvocation selector];
  NSString      *selName = NSStringFromSelector(aSel);
  id		target = nil;
  id		delegate;

  /*
   * We never permit remote processes to call private methods in this
   * application.
   */
  if ([selName hasPrefix: @"_"] == YES)
    {
      [NSException raise: NSGenericException
		  format: @"method name '%@' private in '%@'",
	selName, [manager port]];
    }

  if ([selName hasSuffix: @":userData:error:"] == YES)
    {
      /*
       * The selector matches the correct form for a services request,
       * so we send the message to the services provider.
       */
      if ([servicesProvider respondsToSelector: aSel] == YES)
	{
	  NSPasteboard	*pb;

	  /*
	   * Create a local NSPasteboard object for this pasteboard.
	   * If we try to use the remote NSPasteboard object, we get
	   * trouble when setting property lists since the remote
	   * NSPasteboard fails to serialize the local property
	   * list objects for sending to gpbs.
	   */
	  [anInvocation getArgument: (void*)&pb atIndex: 2];
	  pb = [NSPasteboard pasteboardWithName: [pb name]];
	  [anInvocation setArgument: (void*)&pb atIndex: 2];
	  [anInvocation invokeWithTarget: servicesProvider];
	  return;
	}
      [NSException raise: NSGenericException
		  format: @"service request '%@' not implemented in '%@'",
	selName, [manager port]];
    }

  delegate = [[NSApplication sharedApplication] delegate];

  /*
   * We assume that messages of the form 'application:...' are all
   * safe and do not need to be listed in GSPermittedMessages.
   * They can be handled either by the application delegate or by
   * the shared GSServicesManager instance.
   */
  if ([selName hasPrefix: @"application:"] == YES)
    {
      if ([delegate respondsToSelector: aSel] == YES)
	{
	  target = delegate;
	}
      else if ([manager respondsToSelector: aSel] == YES)
	{
	  target = manager;
	}
    }

  if (target == nil)
    {
      NSArray	*messages;
 
      /*
       * Unless the message was of a format assumed to be safe,
       * we must check it against the GSPermittedMessages array
       * to see if the app allows it to be sent from a remote
       * process.
       */
      messages = [[NSUserDefaults standardUserDefaults] arrayForKey:
	@"GSPermittedMessages"];
      if (messages != nil && [messages containsObject: selName] == NO)
	{
	  [NSException raise: NSGenericException
		      format: @"method '%@' not in GSPermittedMessages in '%@'",
	    selName, [manager port]];
	}
      if ([delegate respondsToSelector: aSel] == YES)
	{
	  target = delegate;
	}
      else if ([NSApp respondsToSelector: aSel] == YES)
	{
	  target = NSApp;
	}
    }

  if (target == nil)
    {
      [NSException raise: NSGenericException
		  format: @"method '%@' not implemented in '%@'",
	selName, [manager port]];
    }
  else
    {
      if ([selName isEqualToString: @"terminate:"])
	{
	  NSNotificationCenter	*c;

	  /*
	   * We handle the terminate: message as a special case, sending
	   * a power off notification before allowing it to be processed
	   * as normal.
	   */
	  c = [[NSWorkspace sharedWorkspace] notificationCenter];
	  [c _postLocal: NSWorkspaceWillPowerOffNotification userInfo: nil];
	}

      [anInvocation invokeWithTarget: target];
    }
}

/**
 * Return the appropriate method signature for aSelector, checking
 * to see if it's a standard service message or standard application
 * message.<br />
 * If the message is non-standard, it can be checked against a list
 * of messages specified by the GSPermittedMessages user default.<br />
 * The logic in this method <strong>must</strong> match that in 
 * forwardInvocation:
 */
- (NSMethodSignature*) methodSignatureForSelector: (SEL)aSelector
{
  NSMethodSignature	*sig = nil;
  NSString      	*selName = NSStringFromSelector(aSelector);
  id			delegate; 

  if ([selName hasSuffix: @":userData:error:"])
    {
      return [servicesProvider methodSignatureForSelector: aSelector];
    }

  delegate = [[NSApplication sharedApplication] delegate];
  if ([selName hasPrefix: @"application:"] == YES)
    {
      if ([delegate respondsToSelector: aSelector] == YES)
	{
	  sig = [delegate methodSignatureForSelector: aSelector];
	}
      else if ([manager respondsToSelector: aSelector] == YES)
	{
	  sig = [manager methodSignatureForSelector: aSelector];
	}
    }

  if (sig == nil)
    {
      NSArray	*messages;

      messages = [[NSUserDefaults standardUserDefaults] arrayForKey:
	@"GSPermittedMessages"];
      if (messages != nil && [messages containsObject: selName] == NO)
	{
	  return nil;
	}
      if ([delegate respondsToSelector: aSelector] == YES)
	{
	  sig = [delegate methodSignatureForSelector: aSelector];
	}
      else if ([NSApp respondsToSelector: aSelector] == YES)
	{
	  sig = [NSApp methodSignatureForSelector: aSelector];
	}
    }
  return sig;
}

- (BOOL) respondsToSelector: (SEL)aSelector
{
  if ([self methodSignatureForSelector: aSelector] != nil)
    {
      return YES;
    }
  return NO;
}

- (void) release
{
}

- (id) retain
{
  return self;
}

- (void) activateIgnoringOtherApps: (BOOL)flag
{
  [NSApp activateIgnoringOtherApps: flag];
}

- (id) self
{
  return self;
}
@end /* GSListener */



@implementation GSServicesManager

static NSString         *servicesName = @".GNUstepServices";
static NSString         *disabledName = @".GNUstepDisabled";

/**
 *      Create a new listener for this application.
 *      Uses NSRegisterServicesProvider() to register itsself as a service
 *      provider with the applications name so we can handle incoming
 *      service requests.
 */
+ (GSServicesManager*) newWithApplication: (NSApplication*)app
{
  NSString	*str = nil;
  NSArray       *paths;
  NSString      *path = nil;

  if (manager != nil)
    {
      if (manager->_application == nil)
	{
	  manager->_application = app;
	}
      return RETAIN(manager);
    }

  manager = [GSServicesManager alloc];

  paths = NSSearchPathForDirectoriesInDomains(NSLibraryDirectory,
					      NSUserDomainMask, YES);
  if ((paths != nil) && ([paths count] > 0))
    {
      str = [paths objectAtIndex: 0];
    }
  /*
   * If standard search paths are not set up, try a default location.
   */
  if (str == nil)
    {
      str = [[NSHomeDirectory() stringByAppendingPathComponent:
        @"GNUstep"] stringByAppendingPathComponent: @"Library"];
    }
  str = [str stringByAppendingPathComponent: @"Services"];
  path = [str stringByAppendingPathComponent: servicesName];
  manager->_servicesPath = [path copy];
  path = [str stringByAppendingPathComponent: disabledName];
  manager->_disabledPath = [path copy];
  /*
   *    Don't retain application object - that would create a cycle.
   */
  manager->_application = app;
  manager->_returnInfo = [[NSMutableSet alloc] initWithCapacity: 16];
  manager->_combinations = [[NSMutableDictionary alloc] initWithCapacity: 16];
  /*
   *	Check for changes to the services cache every thirty seconds.
   */
  manager->_timer =
	RETAIN([NSTimer scheduledTimerWithTimeInterval: 30.0
					 target: manager
				       selector: @selector(loadServices)
				       userInfo: nil
					repeats: YES]);

  [manager loadServices];
  return manager;
}

+ (GSServicesManager*) manager
{
  if (manager == nil)
    {
      [self newWithApplication: nil];
    }
  return manager;
}

- (BOOL) application: (NSApplication*)theApp
	    openFile: (NSString*)file
{
  id	del = [NSApp delegate];
  BOOL	result = NO;
  NSError *err = nil;

  if ([del respondsToSelector: _cmd])
    {
      result = [del application: theApp openFile: file];
    }
  else if ([[NSDocumentController sharedDocumentController]
             openDocumentWithContentsOfURL: [NSURL fileURLWithPath: file]
                                   display: YES
                                     error: &err] != nil)
    {
      [NSApp activateIgnoringOtherApps: YES];
      result = YES;
    }
  else
    {
      [NSApp presentError: err];
    }
  return result;
}

- (void) application: (NSApplication*)theApp
           openFiles: (NSArray*)files
{
  id    del = [NSApp delegate];
  
  if ([del respondsToSelector: _cmd])
    {
      [del application: theApp openFiles: files];
    }
  else
    {
      NSString *filePath;
      NSEnumerator *en = [files objectEnumerator];

      while ((filePath = (NSString *)[en nextObject]) != nil)
	{
	  [self application: theApp openFile: filePath];
	}
    }
}


- (BOOL) application: (NSApplication*)theApp
   openFileWithoutUI: (NSString*)file
{
  id	del = [NSApp delegate];
  BOOL	result = NO;
  NSError *err = nil;

  if ([del respondsToSelector: _cmd])
    {
      result = [del application: theApp openFileWithoutUI: file];
    }
  else if ([[NSDocumentController sharedDocumentController]
             openDocumentWithContentsOfURL: [NSURL fileURLWithPath: file]
                                   display: NO
                                     error: &err] != nil)
    {
      result = YES;
    }
  return result;
}

- (BOOL) application: (NSApplication*)theApp
	openTempFile: (NSString*)file
{
  BOOL	result = [self application: theApp openFile: file];

  [[NSFileManager defaultManager] removeFileAtPath: file handler: nil];

  return result;
}

- (BOOL) application: (NSApplication*)theApp
	     openURL: (NSURL*)aURL
{
  id	del = [NSApp delegate];
  BOOL	result = NO;
  NSError *err = nil;

  if ([del respondsToSelector: _cmd])
    {
      result = [del application: theApp openURL: aURL];
    }
  else if ([[NSDocumentController sharedDocumentController]
             openDocumentWithContentsOfURL: aURL
                                   display: YES
                                     error: &err] != nil)
    {
      [NSApp activateIgnoringOtherApps: YES];
      result = YES;
    }
  else
    {
      NSString	*s = [aURL absoluteString];

      result = [self application: theApp openFile: s];
    }
  return result;
}

- (BOOL) application: (NSApplication*)theApp
	   printFile: (NSString*)file
{
  id	del = [NSApp delegate];

  if ([del respondsToSelector: _cmd])
    return [del application: theApp printFile: file];
  return NO;
}

- (void) dealloc
{
  NSString          *appName;

  appName = [[NSProcessInfo processInfo] processName];
  [_timer invalidate];
  RELEASE(_timer);
  NSUnregisterServicesProvider(appName);
  RELEASE(_languages);
  RELEASE(_returnInfo);
  RELEASE(_combinations);
  RELEASE(_title2info);
  RELEASE(_menuTitles);
  RELEASE(_servicesMenu);
  RELEASE(_disabledPath);
  RELEASE(_servicesPath);
  RELEASE(_disabledStamp);
  RELEASE(_servicesStamp);
  RELEASE(_allDisabled);
  RELEASE(_allServices);
  [super dealloc];
}

- (void) doService: (NSMenuItem*)item
{
  NSString      *title = [self item2title: item];
  NSDictionary  *info = [_title2info objectForKey: title];
  NSArray       *sendTypes = [info objectForKey: @"NSSendTypes"];
  NSArray       *returnTypes = [info objectForKey: @"NSReturnTypes"];
  unsigned      i, j;
  unsigned      es = [sendTypes count];
  unsigned      er = [returnTypes count];
  NSResponder   *resp = [[_application keyWindow] firstResponder];
  id            obj = nil;

  for (i = 0; i <= es; i++)
    {
      NSString  *sendType;

      sendType = (i < es) ? [sendTypes objectAtIndex: i] : nil;

      for (j = 0; j <= er; j++)
        {
          NSString      *returnType;

          returnType = (j < er) ? [returnTypes objectAtIndex: j] : nil;

          obj = [resp validRequestorForSendType: sendType
                                     returnType: returnType];
          if (obj != nil)
            {
              NSPasteboard      *pb;

              pb = [NSPasteboard pasteboardWithUniqueName];
	      if (sendType
		  && [obj writeSelectionToPasteboard: pb
					       types: sendTypes] == NO)
		{
		  NSRunAlertPanel(nil,
			@"object failed to write to pasteboard",
			@"Continue", nil, nil);
		}
	      else if (NSPerformService(title, pb) == YES)
		{
		  if (returnType
		      && [obj readSelectionFromPasteboard: pb] == NO)
		    {
		      NSRunAlertPanel(nil,
			    @"object failed to read from pasteboard",
			    @"Continue", nil, nil);
		    }
		}
              return;
            }
        }
    }
}

/**
 * Return a dictionary of information about registered filter services.
 */
- (NSArray*) filters
{
  return [_allServices objectForKey: @"ByFilter"];
}

- (BOOL) hasRegisteredTypes: (NSDictionary*)service
{
  NSArray       *sendTypes = [service objectForKey: @"NSSendTypes"];
  NSArray       *returnTypes = [service objectForKey: @"NSReturnTypes"];
  NSString      *type;
  unsigned      i;

  /*
   *    We know that both sendTypes and returnTypes can't be nil since
   *    make_services has validated the service entry for us.
   */
  if (sendTypes == nil || [sendTypes count] == 0)
    {
      for (i = 0; i < [returnTypes count]; i++)
        {
          type = [returnTypes objectAtIndex: i];
          if ([_returnInfo member: type] != nil)
            {
              return YES;
            }
        }
    }
  else if (returnTypes == nil || [returnTypes count] == 0)
    {
      for (i = 0; i < [sendTypes count]; i++)
        {
          type = [sendTypes objectAtIndex: i];
          if ([_combinations objectForKey: type] != nil)
            {
              return YES;
            }
        }
    }
  else
    {
      for (i = 0; i < [sendTypes count]; i++)
        {
          NSSet *rset;

          type = [sendTypes objectAtIndex: i];
          rset = [_combinations objectForKey: type];
          if (rset != nil)
            {
              unsigned  j;

              for (j = 0; j < [returnTypes count]; j++)
                {
                  type = [returnTypes objectAtIndex: j];
                  if ([rset member: type] != nil)
                    {
                      return YES;
                    }
                }
            }
        }
    }
  return NO;
}

/**
 *      Use tag in menu item to identify slot in menu titles array that
 *      contains the full title of the service.
 *      Return nil if this is not one of our service menu items.
 */
- (NSString*) item2title: (id<NSMenuItem>)item
{
  unsigned      pos;

  if ([item target] != self)
    return nil;
  pos = [item tag];
  if (pos > [_menuTitles count])
    return nil;
  return [_menuTitles objectAtIndex: pos];
}

- (void) loadServices
{
  NSFileManager         *mgr = [NSFileManager defaultManager];
  BOOL			changed = NO;

  if ([mgr fileExistsAtPath: _disabledPath])
    {
      NSDictionary	*attr;
      NSDate		*mod;

      attr = [mgr fileAttributesAtPath: _disabledPath
			  traverseLink: YES];
      mod = [attr objectForKey: NSFileModificationDate];
      if (_disabledStamp == nil || [_disabledStamp laterDate: mod] == mod)
	{
	  NSData	*data;
	  id		plist = nil;

	  data = [NSData dataWithContentsOfFile: _disabledPath];
	  if (data)
	    {
	      plist = [NSDeserializer deserializePropertyListFromData: data
						    mutableContainers: NO];
	      if (plist)
		{
		  NSMutableSet	*s;
		  changed = YES;
		  s = (NSMutableSet*)[NSMutableSet setWithArray: plist];
		  ASSIGN(_allDisabled, s);
		}
	    }
	  /* Track most recent version of file loaded */
	  ASSIGN(_disabledStamp, mod);
	}
    }

  if ([mgr fileExistsAtPath: _servicesPath])
    {
      NSDictionary	*attr;
      NSDate		*mod;

      attr = [mgr fileAttributesAtPath: _servicesPath
			  traverseLink: YES];
      mod = [attr objectForKey: NSFileModificationDate];
      if (_servicesStamp == nil || [_servicesStamp laterDate: mod] == mod)
	{
	  NSData	*data;
	  id		plist = nil;

	  data = [NSData dataWithContentsOfFile: _servicesPath];
	  if (data)
	    {
	      plist = [NSDeserializer deserializePropertyListFromData: data
						    mutableContainers: YES];
	      if (plist)
		{
		  ASSIGN(_allServices, plist);
		  changed = YES;
		}
	    }
	  /* Track most recent version of file loaded */
	  ASSIGN(_servicesStamp, mod);
	}
    }
  if (changed)
    {
      /* If we have changed the enabled/disabled services,
       * or there have been services added/removed
       * then we must rebuild the services menu to add/remove
       * items as appropriate.
       */
      [self rebuildServicesMenu];
    }
}

- (NSDictionary*) menuServices
{
  if (_allServices == nil)
    {
      [self loadServices];
    }
  return _title2info;
}

/**
 * Returns the 'port' of this application ... this is the name the
 * application is registered under so that other apps can connect to
 * it to use any services it provides.
 */
- (NSString*) port
{
  return _port;
}

/**
 * Makes the current set of usable services consistent with the
 * data types currently available.
 */
- (void) rebuildServices
{
  NSDictionary          *services;
  NSMutableArray        *newLang;
  NSMutableSet          *alreadyFound;
  NSMutableDictionary   *newServices;
  unsigned              pos;

  if (_allServices == nil)
    return;

  newLang = [[[[NSUserDefaults standardUserDefaults]
    stringArrayForKey: @"NSLanguages"] mutableCopy] autorelease];
  if (newLang == nil)
    {
      newLang = [NSMutableArray arrayWithCapacity: 1];
    }
  if ([newLang containsObject:  @"default"] == NO)
    {
      [newLang addObject: @"default"];
    }
  ASSIGN(_languages, newLang);

  services = [_allServices objectForKey: @"ByService"];

  newServices = [NSMutableDictionary dictionaryWithCapacity: 16];
  alreadyFound = [NSMutableSet setWithCapacity: 16];

  /*
   *    Build dictionary of services we can use.
   *    1. make sure we make dictionary keyed on preferred menu item language
   *    2. don't include entries for services already examined.
   *    3. don't include entries for menu items specifically disabled.
   *    4. don't include entries for which we have no registered types.
   */
  for (pos = 0; pos < [_languages count]; pos++)
    {
      NSDictionary      *byLanguage;

      byLanguage = [services objectForKey: [_languages objectAtIndex: pos]];
      if (byLanguage != nil)
        {
          NSEnumerator  *enumerator = [byLanguage keyEnumerator];
          NSString      *menuItem;

          while ((menuItem = [enumerator nextObject]) != nil)
            {
              NSDictionary      *service = [byLanguage objectForKey: menuItem];

              if ([alreadyFound member: service] != nil)
                continue;

              [alreadyFound addObject: service];

	      /* See if this service item is disabled. */
              if ([_allDisabled member: menuItem] != nil)
                continue;

              if ([self hasRegisteredTypes: service])
                [newServices setObject: service forKey: menuItem];
            }
        }
    }
  if ([newServices isEqual: _title2info] == NO)
    {
      NSArray   *titles;

      ASSIGN(_title2info, newServices);
      titles = [_title2info allKeys];
      titles = [titles sortedArrayUsingSelector: @selector(compare:)];
      ASSIGN(_menuTitles, titles);
      [self rebuildServicesMenu];
    }
}

/** Adds or removes items in the services menu in response to a change
 * in the services which are available to the app.
 */
- (void) rebuildServicesMenu
{
  if (_servicesMenu != nil)
    {
      NSMutableSet      *keyEquivalents;
      unsigned          pos;
      unsigned          loc0;
      unsigned          loc1 = 0;
      SEL               sel = @selector(doService:);
      NSMenu            *submenu = nil;

      [_servicesMenu setAutoenablesItems: NO];
      for (pos = [_servicesMenu numberOfItems]; pos > 0; pos--)
        {
          [_servicesMenu removeItemAtIndex: 0];
        }
      [_servicesMenu setAutoenablesItems: YES];

      keyEquivalents = [NSMutableSet setWithCapacity: 4];
      for (loc0 = pos = 0; pos < [_menuTitles count]; pos++)
        {
          NSString      *title = [_menuTitles objectAtIndex: pos];
          NSString      *equiv = @"";
          NSDictionary  *info;
          NSDictionary  *titles;
          NSDictionary  *equivs;
          NSRange       r;
          unsigned      lang;
          id<NSMenuItem>        item;

          if (NSShowsServicesMenuItem(title) == NO)
            {
              continue; // We don't want to show this one.
            }

          /*
           *    Find the key equivalent corresponding to this menu title
           *    in the service definition.
           */
          info = [_title2info objectForKey: title];
          titles = [info objectForKey: @"NSMenuItem"];
          equivs = [info objectForKey: @"NSKeyEquivalent"];
          for (lang = 0; lang < [_languages count]; lang++)
            {
              NSString  *language = [_languages objectAtIndex: lang];
              NSString  *t = [titles objectForKey: language];

              if ([t isEqual: title])
                {
                  equiv = [equivs objectForKey: language]; 
		  if (equiv == nil)
		    {
		      equiv = [equivs objectForKey: @"default"];
		    }		  
                }
            }

          /*
           *    Make a note that we are using the key equivalent, or
           *    set to nil if we have already used it in this menu.
           */
          if (equiv)
            {
              if ([keyEquivalents member: equiv] == nil)
                {
                  [keyEquivalents addObject: equiv];
                }
              else
                {
                  equiv = @"";
                }
            }

          r = [title rangeOfString: @"/"];
          if (r.length > 0)
            {
              NSString  *subtitle = [title substringFromIndex: r.location+1];
              NSString  *parentTitle = [title substringToIndex: r.location];
              NSMenu    *menu;

              item = [_servicesMenu itemWithTitle: parentTitle];
              if (item == nil)
                {
                  loc1 = 0;
                  item = [_servicesMenu insertItemWithTitle: parentTitle
                                                    action: 0
                                             keyEquivalent: @""
                                                   atIndex: loc0++];
                  menu = [[NSMenu alloc] initWithTitle: parentTitle];
                  [_servicesMenu setSubmenu: menu
                                   forItem: item];
                  RELEASE(menu);
                }
              else
                {
                  menu = (NSMenu*)[item submenu];
                }
              if (menu != submenu)
                {
                  [submenu sizeToFit];
                  submenu = menu;
                }
              item = [submenu insertItemWithTitle: subtitle
                                           action: sel
                                    keyEquivalent: equiv
                                          atIndex: loc1++];
              [item setTarget: self];
              [item setTag: pos];
            }
          else
            {
              item = [_servicesMenu insertItemWithTitle: title
                                                action: sel
                                         keyEquivalent: equiv
                                               atIndex: loc0++];
              [item setTarget: self];
              [item setTag: pos];
            }
        }
      [submenu update];
//      [submenu sizeToFit];
//      [_servicesMenu sizeToFit];
      [_servicesMenu update];
    }
}

/**
 *      Set up connection to listen for incoming service requests.
 */
- (void) registerAsServiceProvider
{
  NSString      *appName;
  BOOL          registered;

  appName = [[NSProcessInfo processInfo] processName];
  NS_DURING
    {
      NSRegisterServicesProvider(self, appName);
      registered = YES;
    }
  NS_HANDLER
    {
      registered = NO;
    }
  NS_ENDHANDLER

  if (registered == NO)
    {
      NSUserDefaults	*defs = [NSUserDefaults standardUserDefaults];
      if ([defs boolForKey: @"NSUseRunningCopy"] == YES)
	{
	  id	app;

	  /*
	   * Try to activate the other app and terminate self.
	   */
	  app = [NSConnection rootProxyForConnectionWithRegisteredName: appName
								  host: @""];
	  NS_DURING
	    {
	      [app activateIgnoringOtherApps: YES];
	    }
	  NS_HANDLER
	    {
	      /* maybe it terminated. */
	    }
	  NS_ENDHANDLER
	  registered = NO;
        }
      else
	{
	  unsigned	count = 0;

	  /*
	   * Try to rename self as a copy.
	   */
	  while (registered == NO && ++count < 100)
	    {
	      NSString	*tmp;

	      tmp = [appName stringByAppendingFormat: @"Copy%d", count];
	      NS_DURING
		{
		  NSRegisterServicesProvider(self, tmp);
		  registered = YES;
		  appName = tmp;
		}
	      NS_HANDLER
		{
		  registered = NO;
		}
	      NS_ENDHANDLER
	    }
	  if (registered == NO)
	    {
	      int	result;

	      /*
	       * Something is seriously wrong - we can't talk to the
	       * nameserver, so all interaction with the workspace manager
	       * and/or other applications will fail.
	       * Give the user a chance to keep on going anyway.
	       */
	      result = NSRunAlertPanel(appName,
		@"Unable to register application with ANY name",
		@"Abort", @"Continue", nil);
	      if (result == NSAlertDefaultReturn)
		{
		  registered = YES;
		}
	    }
        }

      if (registered == NO)
	{
	  [[NSApplication sharedApplication] terminate: self];
	}
    }
  ASSIGN(_port, appName);
}

/**
 * Register send and return types that an object can handle - we keep
 * a note of all the possible combinations -
 * 'returnInfo' is a set of all the return types that can be handled
 * without a send.
 * 'combinations' is a dictionary of all send types, with the associated
 * values being sets of possible return types.
 */
- (void) registerSendTypes: (NSArray *)sendTypes
               returnTypes: (NSArray *)returnTypes
{
  BOOL          didChange = NO;
  unsigned      i;

  for (i = 0; i < [sendTypes count]; i++)
    {
      NSString          *sendType = [sendTypes objectAtIndex: i];
      NSMutableSet      *returnSet = [_combinations objectForKey: sendType];

      if (returnSet == nil)
        {
          returnSet = [NSMutableSet setWithCapacity: [returnTypes count]];
          [_combinations setObject: returnSet forKey: sendType];
          [returnSet addObjectsFromArray: returnTypes];
          didChange = YES;
        }
      else
        {
          unsigned      count = [returnSet count];

          [returnSet addObjectsFromArray: returnTypes];
          if ([returnSet count] != count)
            {
              didChange = YES;
            }
        }
    }

  i = [_returnInfo count];
  [_returnInfo addObjectsFromArray: returnTypes];
  if ([_returnInfo count] != i)
    {
      didChange = YES;
    }

  if (didChange)
    {
      /* Types have changed, so we need to enable/disable items in the
       * services menu depending on what types they support.
       */
      [self rebuildServices];
    }
}

- (NSMenu*) servicesMenu
{
  return _servicesMenu;
}

- (id) servicesProvider
{
  return [GSListener servicesProvider];
}

- (void) setServicesMenu: (NSMenu*)aMenu
{
  ASSIGN(_servicesMenu, aMenu);
  [self rebuildServicesMenu];
}

- (void) setServicesProvider: (id)anObject
{
  [GSListener setServicesProvider: anObject];
}

- (int) setShowsServicesMenuItem: (NSString*)item to: (BOOL)enable
{
  NSData	*d;

  [self loadServices];
  if (_allDisabled == nil)
    {
      _allDisabled = [[NSMutableSet alloc] initWithCapacity: 1];
    }
  if (enable)
    {
      [_allDisabled removeObject: item];
    }
  else
    {
      [_allDisabled addObject: item];
    }
  d = [NSSerializer serializePropertyList: [_allDisabled allObjects]];
  if ([d writeToFile: _disabledPath atomically: YES] == YES)
    {
      return 0;
    }
  return -1;
}

- (BOOL) showsServicesMenuItem: (NSString*)item
{
  [self loadServices];
  if ([_allDisabled member: item] == nil)
    return YES;
  return NO;
}

- (BOOL) validateMenuItem: (id<NSMenuItem>)item
{
  NSString      *title = [self item2title: item];
  NSDictionary  *info = [_title2info objectForKey: title];
  NSArray       *sendTypes = [info objectForKey: @"NSSendTypes"];
  NSArray       *returnTypes = [info objectForKey: @"NSReturnTypes"];
  unsigned      i, j;
  unsigned      es = [sendTypes count];
  unsigned      er = [returnTypes count];
  NSResponder	*resp = [[_application keyWindow] firstResponder];

  /*
   *    If the menu item is not in our map, it must be the item containing
   *    a sub-menu - so we see if any item in the submenu is valid.
   */
  if (title == nil)
    {
      NSMenu    *sub;

      if (![item isKindOfClass: [NSMenuItem class]])
        return NO;

      sub = [item submenu];

      if (sub && [sub isKindOfClass: [NSMenu class]])
        {
          NSArray       *a = [sub itemArray];

          for (i = 0; i < [a count]; i++)
            {
              if ([self validateMenuItem: [a objectAtIndex: i]] == YES)
                {
                  return YES;
                }
            }
        }
      return NO;
    }

  /*
   *    The item corresponds to one of our services - so we check to see if
   *    there is anything that can deal with it.
   */
  if (es == 0)
    {
      if (er == 0)
	{
	  if ([resp validRequestorForSendType: nil
				   returnType: nil] != nil)
	    return YES;
	}
      else
	{
	  for (j = 0; j < er; j++)
	    {
	      NSString      *returnType;

	      returnType = [returnTypes objectAtIndex: j];
	      if ([resp validRequestorForSendType: nil
				       returnType: returnType] != nil)
		return YES;
	    }
	}
    }
  else
    {
      for (i = 0; i < es; i++)
	{
	  NSString  *sendType;

	  sendType = [sendTypes objectAtIndex: i];

	  if (er == 0)
	    {
	      if ([resp validRequestorForSendType: sendType
				       returnType: nil] != nil)
		return YES;
	    }
	  else
	    {
	      for (j = 0; j < er; j++)
		{
		  NSString      *returnType;

		  returnType = [returnTypes objectAtIndex: j];
		  if ([resp validRequestorForSendType: sendType
					   returnType: returnType] != nil)
		    return YES;
		}
	    }
	}
    }
  return NO;
}

- (void) updateServicesMenu
{
  if (_servicesMenu && [[_application mainMenu] autoenablesItems])
    {
      NSArray   	*a;
      unsigned  	i;

      a = [_servicesMenu itemArray];

      for (i = 0; i < [a count]; i++)
        {
          NSMenuItem    *item = [a objectAtIndex: i];
	  BOOL		wasEnabled = [item isEnabled];
	  BOOL		shouldBeEnabled;
	  NSString      *title = [self item2title: item];

	  /*
	   *	If there is no title mapping, this item must be a
	   *	submenu - so we check the submenu items.
	   *
	   *	We always enable the submenu item itself. We do this
	   *	to prevent confusion (if the user is trying to use
	   *	a disabled item, it's clearer to show that item disabled
	   *	than to hide it in a disabled submenu), and to encourage
	   *	the user to explore the interface (it makes it possible
	   *	to browse the service list at any time).
	   */
	  if (title == nil && [[item submenu] isKindOfClass: [NSMenu class]])
	    {
	      NSArray		*sub = [[item submenu] itemArray];
	      unsigned		j;

	      shouldBeEnabled = YES;
	      for (j = 0; j < [sub count]; j++)
		{
		  NSMenuItem	*subitem = [sub objectAtIndex: j];
		  BOOL		subWasEnabled = [subitem isEnabled];
		  BOOL		subShouldBeEnabled = NO;

		  if ([self validateMenuItem: subitem] == YES)
		    {
		      subShouldBeEnabled = YES;
		    }
		  if (subWasEnabled != subShouldBeEnabled)
		    {
		      [subitem setEnabled: subShouldBeEnabled];
		    }
		}
	    }
	  else
	    {
	      shouldBeEnabled = [self validateMenuItem: item];
	    }

          if (wasEnabled != shouldBeEnabled)
            {
              [item setEnabled: shouldBeEnabled];
            }
        }
    }
}

@end /* GSServicesManager */


/**
 * <p>Establishes an NSConnection to the application listening at port
 * (by convention usually the application name), launching appName 
 * if necessary.  Returns the proxy to the remote application, or nil
 * on failure.
 * </p>
 * <p>The value of port specifies the name of the distributed objects
 * service to which the connection is to be made.  If this is nil
 * it will be inferred from appName ... by convention, applications
 * use their own name (minus any path or extension) for this.
 * </p>
 * <p>If appName is nil or cannot be launched, this attempts to locate any
 * application in a standard location whose name matches port and launch
 * that application.
 * </p>
 * <p>The value of expire provides a timeout in case the application cannot
 * be contacted promptly.  If it is omitted, a thirty second timeout is
 * used.
 * </p>
 */
id
GSContactApplication(NSString *appName, NSString *port, NSDate *expire)
{
  id	app;

  if (port == nil)
    {
      port = [[appName lastPathComponent] stringByDeletingPathExtension];
    }
  if (expire == nil)
    {
      expire = [NSDate dateWithTimeIntervalSinceNow: 30.0];
    }
  if (providerName != nil && [port isEqual: providerName] == YES)
    {
      app = [GSListener listener];	// Contect our own listener.
    }
  else
    {
      NS_DURING
	{
	  app = [NSConnection rootProxyForConnectionWithRegisteredName: port  
								  host: @""];
	}
      NS_HANDLER
	{
	  return nil;                /* Fatal error in DO    */
	}
      NS_ENDHANDLER
    }
  if (app == nil)
    {
      if (appName == nil
	|| [[NSWorkspace sharedWorkspace] launchApplication: appName] == NO)
	{
          if (port == nil
	    || [[NSWorkspace sharedWorkspace] launchApplication: port] == NO)
	    {
	      return nil;		/* Unable to launch.	*/
	    }
	}

      NS_DURING
	{
	  app = [NSConnection
			rootProxyForConnectionWithRegisteredName: port  
							    host: @""];
	  while (app == nil && [expire timeIntervalSinceNow] > 0.1)
	    {
	      NSRunLoop	*loop = [NSRunLoop currentRunLoop];
	      NSDate	*next;

	      [NSTimer scheduledTimerWithTimeInterval: 0.1
					   invocation: nil
					      repeats: NO];
	      next = [NSDate dateWithTimeIntervalSinceNow: 0.2];
	      [loop runUntilDate: next];
	      app = [NSConnection
			    rootProxyForConnectionWithRegisteredName: port  
								host: @""];
	    }
	}
      NS_HANDLER
	{
	  return nil;
	}
      NS_ENDHANDLER
    }

  return app;
}

static NSDictionary *
serviceFromAnyLocalizedTitle(NSString *title)
{
  NSDictionary	*allServices;
  NSEnumerator	*e1;
  NSDictionary	*service;

  allServices = [manager menuServices];
  if (allServices == nil)
    {
      return nil;
    }

  if ([allServices objectForKey: title] != nil)
    {
      return [allServices objectForKey: title];
     } 
  e1 = [allServices objectEnumerator];
  while ((service = [e1 nextObject]) != nil)
    {
      NSDictionary	*menuItems;
      NSString		*itemName;
      NSEnumerator	*e2;

      menuItems = [service objectForKey: @"NSMenuItem"];
      if (menuItems == nil)
	{
	  continue;
	}
      e2 = [menuItems objectEnumerator];
      while ((itemName = [e2 nextObject]) != nil)
	{
	  if ([itemName isEqualToString: title] == YES)
	    {
	      return service;
	    }
	}
    }

  return nil;
}

/**
 * <p>Given the name of a serviceItem, and some data in a pasteboard
 * this function sends the data to the service provider (launching
 * another application if necessary) and retrieves the result of
 * the service in the pastebaord.
 * </p>
 * Returns YES on success, NO otherwise.
 */
BOOL
NSPerformService(NSString *serviceItem, NSPasteboard *pboard)
{
  NSDictionary		*service;
  NSString		*port;
  NSString		*timeout;
  double		seconds;
  NSDate		*finishBy;
  NSString		*appPath;
  id			provider;
  NSString		*message;
  NSString		*selName;
  NSString		*userData;
  NSString		*error = nil;

  service = serviceFromAnyLocalizedTitle(serviceItem);
  if (service == nil)
    {
      NSRunAlertPanel(nil,
	@"No service matching '%@'",
	@"Continue", nil, nil,
	serviceItem);
      return NO;			/* No matching service.	*/
    }

  port = [service objectForKey: @"NSPortName"];
  timeout = [service objectForKey: @"NSTimeout"];
  if (timeout && [timeout floatValue] > 100)
    {
      seconds = [timeout floatValue] / 1000.0;
    }
  else
    {
      seconds = 30.0;
    }
  finishBy = [NSDate dateWithTimeIntervalSinceNow: seconds];
  appPath = [service objectForKey: @"ServicePath"];
  userData = [service objectForKey: @"NSUserData"];
  message = [service objectForKey: @"NSMessage"];
  selName = [message stringByAppendingString: @":userData:error:"];

  /*
   * Locate the service provider ... this will be a proxy to the remote
   * object, or a local object (if we provide the service ourself)
   */
  provider = GSContactApplication(appPath, port, finishBy);
  if (provider == nil)
    {
      NSRunAlertPanel(nil,
	@"Failed to contact service provider for '%@'",
	@"Continue", nil, nil,
	serviceItem);
      return NO;
    }

  /*
   * If the service provider is a remote object, we can set timeouts on
   * the NSConnection so we don't hang waiting for it to reply.
   */
  /*
  This check for a remote object is ugly. When GSListener is reworked,
  this should be improved.

  For now, we can't use -isProxy since GSListener is a proxy, and we can't
  use -isKindOfClass: since it gets forwarded. Fortunately, -class isn't
  forwarded, so that's what we use.

  (Note, though, that we can't even use
  [provider class] == [GSListener class] since [GSListener -class] returns
  NULL instead of the real class.)
  */
  if ([provider class] == [NSDistantObject class])
    {
      NSConnection	*connection;

      connection = [(NSDistantObject*)provider connectionForProxy];
      seconds = [finishBy timeIntervalSinceNow];
      [connection setRequestTimeout: seconds];
      [connection setReplyTimeout: seconds];
    }

  /*
   * At last, we ask for the service to be performed.
   * We create an untyped selector matching the message name we have,
   * Using that, we get a method signature from the provider, and
   * take the type information from that to make a fully typed
   * selector, with which we can create and use an invocation.
   */
  NS_DURING
    {
      SEL		sel = NSSelectorFromString(selName); 
      NSMethodSignature	*sig = [provider methodSignatureForSelector: sel];

      if (sig != nil)
	{
	  NSInvocation	*inv;
	  NSString	**errPtr = &error;

	  inv = [NSInvocation invocationWithMethodSignature: sig];
	  [inv setTarget: provider];
	  [inv setSelector: sel];
	  [inv setArgument: (void*)&pboard atIndex: 2];
	  [inv setArgument: (void*)&userData atIndex: 3];
	  [inv setArgument: (void*)&errPtr atIndex: 4];
	  [inv invoke];
	}
    }
  NS_HANDLER
    {
      error = [NSString stringWithFormat: @"%@", [localException reason]];
    }
  NS_ENDHANDLER

  if (error != nil)
    {
      NSRunAlertPanel(nil,
	@"Failed to contact service provider for '%@': %@",
	@"Continue", nil, nil,
	serviceItem, error);
      return NO;
    }

  return YES;
}

/**
 * <p>Controls whether the item name should be included in the services menu.
 * </p>
 * <p>If enabled is YES then the services menu for each application will
 * include the named item, if enabled is NO then the service will not be
 * shown in application services menus.
 * </p>
 * <p>Returns 0 if the setting is successfuly changed. Non-zero otherwise.
 * </p>
 */
int
NSSetShowsServicesMenuItem(NSString *name, BOOL enabled)
{
  return [[GSServicesManager manager] setShowsServicesMenuItem: name
							    to: enabled];
}

/**
 * Returns a flag indicating whether the named service is supposed to be
 * displayed in application services menus.
 */
BOOL
NSShowsServicesMenuItem(NSString *name)
{
  return [[GSServicesManager manager] showsServicesMenuItem: name];
}

/**
 * A services providing application may use this to update the list of
 * services it provides.<br />
 * In order to update the services advertised, the application must
 * create a <em>.service</em> bundle and place it in
 * <code>~/Library/Services</code> before invoking this function.
 */
void
NSUpdateDynamicServices(void)
{
  /* Get the workspace manager to make sure that cached service info is
   * up to date.
   */
  [[NSWorkspace sharedWorkspace] findApplications];

  /* Reload service information from disk cache.
   */
  [[GSServicesManager manager] loadServices];
}
