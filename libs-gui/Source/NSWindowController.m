/** <title>NSWindowController</title>

   Copyright (C) 2000 Free Software Foundation, Inc.

   Author: Carl Lindberg <Carl.Lindberg@hbo.com>
   Date: 1999
   Author: Fred Kiefer <FredKiefer@gmx.de>
   Date: Aug 2003

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

#import <Foundation/NSArray.h>
#import <Foundation/NSBundle.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSEnumerator.h>
#import <Foundation/NSException.h>
#import <Foundation/NSNotification.h>
#import <Foundation/NSString.h>
#import <Foundation/NSMapTable.h>

#import "AppKit/NSNib.h"
#import "AppKit/NSNibLoading.h"
#import "AppKit/NSPanel.h"
#import "AppKit/NSWindowController.h"
#import "AppKit/NSStoryboardSegue.h"
#import "AppKit/NSStoryboard.h"
#import "AppKit/NSViewController.h"

#import "NSDocumentFrameworkPrivate.h"

@implementation NSWindowController

+ (void) initialize
{
  if (self == [NSWindowController class])
    {
      [self setVersion: 1];
    }
}

- (id) initWithWindowNibName: (NSString *)windowNibName
{
  return [self initWithWindowNibName: windowNibName owner: self];
}

- (id) initWithWindowNibName: (NSString *)windowNibName owner: (id)owner
{
  if (windowNibName == nil)
    {
      [NSException raise: NSInvalidArgumentException
		   format: @"attempt to init NSWindowController with nil windowNibName"];
    }

  if (owner == nil)
    {
      [NSException raise: NSInvalidArgumentException
		   format: @"attempt to init NSWindowController with nil owner"];
    }

  self = [self initWithWindow: nil];
  if (!self)
    return nil;

  ASSIGN(_window_nib_name, windowNibName);
  _owner = owner;
  return self;
}

- (id) initWithWindowNibPath: (NSString *)windowNibPath
                       owner: (id)owner
{
  if (windowNibPath == nil)
    {
      [NSException raise: NSInvalidArgumentException
		   format: @"attempt to init NSWindowController with nil windowNibPath"];
    }

  if (owner == nil)
    {
      [NSException raise: NSInvalidArgumentException
		   format: @"attempt to init NSWindowController with nil owner"];
    }

  self = [self initWithWindow: nil];
  if (!self)
    return nil;

  ASSIGN(_window_nib_path, windowNibPath);
  _owner = owner;
  return self;
}

- (id) initWithWindow: (NSWindow *)window
{
  self = [super init];
  if (!self)
    return nil;

  ASSIGN(_window_frame_autosave_name, @"");
  _wcFlags.should_cascade = YES;
  //_wcFlags.should_close_document = NO;
  _owner = self;

  [self setWindow: window];
  if (_window != nil)
    {
      [self _windowDidLoad];
      [self setDocument: nil];
    }

  return self;
}

- (id) init
{
  return [self initWithWindow: nil];
}

- (void) dealloc
{
  // Window Controllers are expect to release their own top-level objects
  [_top_level_objects makeObjectsPerformSelector: @selector(release)];
  [self setWindow: nil];
  RELEASE(_window_nib_name);
  RELEASE(_window_nib_path);
  RELEASE(_window_frame_autosave_name);
  RELEASE(_top_level_objects);
  RELEASE(_segueMap);
  [super dealloc];
}

- (NSString *) windowNibName
{
  if ((_window_nib_name == nil) && (_window_nib_path != nil))
  {
    return [[_window_nib_path lastPathComponent] 
	       stringByDeletingPathExtension];
  }

  return _window_nib_name;
}

- (NSString *)windowNibPath
{
  if ((_window_nib_name != nil) && (_window_nib_path == nil))
  {
    NSString *path;

    path = [[NSBundle bundleForClass: [_owner class]] 
	       pathForNibResource: _window_nib_name];
    if (path == nil)
      path = [[NSBundle mainBundle] 
		 pathForNibResource: _window_nib_name];

    return path;
  }

  return _window_nib_path;
}

- (id) owner
{
  return _owner;
}

/** Sets the document associated with this controller. A document
    automatically calls this method when adding a window controller to
    its list of window controllers. You should not call this method
    directly when using NSWindowController with an NSDocument
    or subclass. */
- (void) setDocument: (NSDocument *)document
{
  // As the document retains us, we only keep a week reference.
  _document = document;
  [self synchronizeWindowTitleWithDocumentName];

  if (_document == nil)
    {
      /* If you want the window to be deallocated when closed, you
	 need to observe the NSWindowWillCloseNotification (or
	 implement the window's delegate windowWillClose: method) and
	 autorelease the window controller in that method.  That will
	 then release the window when the window controller is
	 released. */
      [_window setReleasedWhenClosed: NO];
    }
  else
    {
      /* When a window owned by a document is closed, it is released
	 and the window controller is removed from the documents
	 list of controllers.
       */
      [_window setReleasedWhenClosed: YES];
    }
}

- (id) document
{
  return _document;
}

- (void) setDocumentEdited: (BOOL)flag
{
  if ([self isWindowLoaded])
    [[self window] setDocumentEdited: flag];
}

- (void) setWindowFrameAutosaveName:(NSString *)name
{
  ASSIGN(_window_frame_autosave_name, name);
  
  if ([self isWindowLoaded])
    {
      [[self window] setFrameAutosaveName: name ? (id)name : (id)@""];
    }
}

- (NSString *) windowFrameAutosaveName
{
  return _window_frame_autosave_name;
}

- (void) setShouldCloseDocument: (BOOL)flag
{
  _wcFlags.should_close_document = flag;
}

- (BOOL) shouldCloseDocument
{
  return _wcFlags.should_close_document;
}

- (void) setShouldCascadeWindows: (BOOL)flag
{
  _wcFlags.should_cascade = flag;
}

- (BOOL) shouldCascadeWindows
{
  return _wcFlags.should_cascade;
}

- (void) close
{
  [_window close];
}

- (void) _windowWillClose: (NSNotification *)notification
{
  if ([notification object] == _window)
    {
      /* We only need to do something if the window is set to be
	 released when closed (which should only happen if _document
	 != nil).  In this case, we release everything; otherwise,
	 well the window is closed but nothing is released so there's
	 nothing to do here. */
      if ([_window isReleasedWhenClosed])
	{
	  RETAIN(self);
	  
	  /*
	   * If the window is set to isReleasedWhenClosed, it will release
	   * itself, so we have to retain it once more.
	   * 
	   * Apple's implementation doesn't seem to deal with this case, and
	   * crashes if isReleaseWhenClosed is set.
	   */
	  RETAIN(_window);
	  [self setWindow: nil];

	  [_document _removeWindowController: self];
	  AUTORELEASE(self);
	}
    }
}

- (NSWindow *) window
{
  if (_window == nil && ![self isWindowLoaded])
    {
      // Do all the notifications.  Yes, the docs say this should
      // be implemented here instead of in -loadWindow itself.

      // Note: The docs say that windowController{Will,Did}LoadNib: are sent
      // to the window controller's document, but Apple's implementation
      // really sends them to the owner of the nib. Since this behavior is
      // more useful, in particular when non-document classes use a window
      // controller, we implement it here too.
      [self windowWillLoad];
      if (_owner != self &&
	  [_owner respondsToSelector: @selector(windowControllerWillLoadNib:)])
	{
	  [_owner windowControllerWillLoadNib: self];
	}

      [self loadWindow];
      if ([self isWindowLoaded]) 
      {
        [self _windowDidLoad];
	if (_owner != self &&
	    [_owner respondsToSelector: @selector(windowControllerDidLoadNib:)])
	{
	  [_owner windowControllerDidLoadNib: self];
	}
      }
    }

  return _window;
}

/** Sets the window that this controller manages to aWindow. The old
   window is released. */
- (void) setWindow: (NSWindow *)aWindow
{
  NSNotificationCenter *nc;

  if (_window == aWindow)
    {
      return;
    }

  nc = [NSNotificationCenter defaultCenter];

  if (_window != nil)
    {
      NSResponder *responder;

      [nc removeObserver: self
          name: NSWindowWillCloseNotification
          object: _window];
      // Remove self from the responder chain
      responder = _window;
      while (responder && [responder nextResponder] != self)
        {
          responder = [responder nextResponder];
        }
      [responder setNextResponder: [self nextResponder]];
      [_window setWindowController: nil];

      // Remove the delegate as well if set to the owner in the NIB file
      if ([_window delegate] == _owner)
        {
          [_window setDelegate: nil];
        }
    }

  ASSIGN(_window, aWindow);

  if (_window != nil)
    {
      [_window setWindowController: self];
      // Put self into the responder chain
      [self setNextResponder: [_window nextResponder]];
      [_window setNextResponder: self];
      [nc addObserver: self
          selector: @selector(_windowWillClose:)
          name: NSWindowWillCloseNotification
          object: _window];

      /* For information on the following, see the description in 
         -setDocument: */
      if (_document == nil)
        {
          [_window setReleasedWhenClosed: NO];
        }
      else
        {
          [_window setReleasedWhenClosed: YES];
          [_window setDocumentEdited: [_document isDocumentEdited]];
        }

      /* Make sure window sizes itself right */
      if ([_window_frame_autosave_name length] > 0)
        {
          [_window setFrameAutosaveName: _window_frame_autosave_name];
        }
    }
}

/** Orders the receiver's window front, also making it the key window
    if appropriate. */
- (IBAction) showWindow: (id)sender
{
  NSWindow *window = [self window];

  if ([window isKindOfClass: [NSPanel class]] 
      && [(NSPanel*)window becomesKeyOnlyIfNeeded])
    {
      [window orderFront: sender];
    }
  else
    {
      [window makeKeyAndOrderFront: sender];
    }
}

- (NSString *) windowTitleForDocumentDisplayName: (NSString *)displayName
{
  return displayName;
}

- (void) synchronizeWindowTitleWithDocumentName
{
  if ((_document != nil) && [self isWindowLoaded])
    {
      NSString *filename = [_document fileName];
      NSString *displayName = [_document displayName];
      NSString *title = [self windowTitleForDocumentDisplayName: displayName];

      /* If they just want to display the filename, use the fancy method */
      /* NB For compatibility with Mac OS X, a document's display name is equal
         to its last path component, so we check for that here too */
      if (filename != nil &&
	  ([title isEqualToString: filename] ||
	   [title isEqualToString: [filename lastPathComponent]]))
        {
          [_window setTitleWithRepresentedFilename: filename];
        }
      else
        {
          if (filename) 
	    [_window setRepresentedFilename: filename];
	  [_window setTitle: title];
        }
    }
}

/** Returns YES if the receiver's window has loaded. */
- (BOOL) isWindowLoaded
{
  return _wcFlags.nib_is_loaded;
}

/** Subclasses can override this method to perform any customisation
    needed after the receiver has loaded its window. */
- (void) windowDidLoad
{
}

/** Subclasses can override this method to perform any customisation
    needed before the receiver loads its window. */
- (void) windowWillLoad
{
}

- (void) _windowDidLoad
{
  _wcFlags.nib_is_loaded = YES;

  [self synchronizeWindowTitleWithDocumentName];

  if ([self shouldCascadeWindows])
    {
      static NSPoint nextWindowLocation = { 0.0, 0.0 };
      /*
       * cascadeTopLeftFromPoint will "wrap" the point back to the
       * top left if the normal cascading will cause the window to go
       * off the screen. In Apple's implementation, this wraps to the
       * extreme top of the screen, and offset only a small amount
       * from the left.
       */
       nextWindowLocation 
	     = [_window cascadeTopLeftFromPoint: nextWindowLocation];
    }

  [self windowDidLoad];
}

/** Loads the receiver's window. You can override this method if the
    way that the window is loaded is not appropriate. You should not
    normally need to call this method directly; it will be called when
    the window controller needs to access the window.
 */
- (void) loadWindow
{
  NSDictionary *table;

  if ([self isWindowLoaded]) 
    {
      return;
    }

  table = [NSDictionary dictionaryWithObject: _owner forKey: NSNibOwner];
  if ([NSBundle loadNibFile: [self windowNibPath]
		externalNameTable: table
		withZone: [_owner zone]])
    {
      _wcFlags.nib_is_loaded = YES;
	  
      if (_window == nil  &&  _document != nil  &&  _owner == _document)
        {
          [self setWindow: [_document _transferWindowOwnership]];
        }
      else
        {
          // The window was already retained by the NIB loading.
          RELEASE(_window);
        }
    }
  else
    {
      if (_window_nib_name != nil)
        {
	  NSLog (@"%@: could not load nib named %@.nib", 
		 [self class], _window_nib_name);
	}
    }
}

- (id) initWithCoder: (NSCoder *)coder
{
  if ([coder allowsKeyedCoding] 
      || [coder versionForClassName: @"NSWindowController"] >= 1)
    {
      self = [super initWithCoder: coder];
      if (!self)
        return nil;

      _segueMap = nil;
      ASSIGN(_window_frame_autosave_name, @"");
      _wcFlags.should_cascade = YES;
      //_wcFlags.should_close_document = NO;

      return self;
    }
  else
    {
      /* backward compatibility: old NSWindowController instances are not
         subclasses of NSResponder, but of NSObject */
      return [self init];
    }
}

- (void) encodeWithCoder: (NSCoder *)coder
{
  // What are we supposed to encode?  Window nib name?  Or should these
  // be empty, just to conform to NSCoding, so we do an -init on
  // unarchival.  ?

  [super encodeWithCoder: coder];
}

// NSSeguePerforming methods...
- (void)performSegueWithIdentifier: (NSStoryboardSegueIdentifier)identifier 
                            sender: (id)sender
{
  NSStoryboardSegue *segue = [_segueMap objectForKey: identifier];
  [self prepareForSegue: segue
                 sender: sender];  
  [segue perform];
}

- (void)prepareForSegue: (NSStoryboardSegue *)segue 
                 sender: (id)sender
{
  // do nothing in base class method...
}

- (BOOL)shouldPerformSegueWithIdentifier: (NSStoryboardSegueIdentifier)identifier 
                                  sender: (id)sender
{
  return YES;
}

@end
