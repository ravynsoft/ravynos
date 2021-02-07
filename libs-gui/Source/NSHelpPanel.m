/** <title>NSHelpPanel</title>

   Copyright (C) 1996 Free Software Foundation, Inc.

   Author: Scott Christley <scottc@net-community.com>
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

#include "config.h"
#import "AppKit/NSHelpPanel.h"
#import "AppKit/NSHelpManager.h"
#import "AppKit/NSScrollView.h"
#import "AppKit/NSTextView.h"
#import "AppKit/NSTextContainer.h"
#import "GSGuiPrivate.h" 


@implementation NSApplication (NSHelpPanel)

- (void) orderFrontHelpPanel: sender
{
  // This is implemented in NSHelpManager.m
  [self showHelp: sender];
}

@end

@implementation NSHelpPanel

static NSString	*_helpDirectory = nil;
static NSString	*_helpFile = nil;
static NSHelpPanel	*_sharedPanel = nil;

//
// Class methods
//
+ (void) initialize
{
  if (self == [NSHelpPanel class])
    {
      // Initial version
      [self setVersion: 1];
    }
}

//
// Accessing the Help Panel
//
+ (NSHelpPanel*) sharedHelpPanel
{
  if (_sharedPanel == nil)
    {
      return [self new];
    }
  return _sharedPanel;
}

+ (NSHelpPanel *) sharedHelpPanelWithDirectory: (NSString *)helpDirectory
{
  [self setHelpDirectory: helpDirectory];
  return [self sharedHelpPanel];
}

//
// Managing the Contents
//
+ (void) setHelpDirectory: (NSString *)helpDirectory
{
  ASSIGN(_helpDirectory, helpDirectory);
}

//
// Attaching Help to Objects 
//
+ (void) attachHelpFile: (NSString *)filename
	     markerName: (NSString *)markerName
		     to: (id)anObject
{
  if ([filename isAbsolutePath] == NO)
    {
      filename = [[[NSHelpPanel sharedHelpPanel] helpDirectory]
        stringByAppendingPathComponent: filename];
    }
  [[NSHelpManager sharedHelpManager] setContextHelp: (id)filename
					  forObject: anObject];
}

+ (void) detachHelpFrom: (id)anObject
{
  [[NSHelpManager sharedHelpManager] removeContextHelpForObject: anObject];
}

//
// Instance methods
//
//
// Managing the Contents
//
- (void) addSupplement: (NSString *)helpDirectory
	        inPath: (NSString *)supplementPath
{
}

- (NSString *) helpDirectory
{
  return _helpDirectory;
}

- (NSString *) helpFile
{
  return _helpFile;
}

- (id) initWithContentRect: (NSRect)contentRect
		 styleMask: (NSUInteger)aStyle
		   backing: (NSBackingStoreType)bufferingType
		     defer: (BOOL)flag
{
  if (_sharedPanel == nil)
    {
      NSScrollView	*s;
      NSTextView	*v;
      NSRect		r;

      /* We have a standard start size.
       */
      contentRect = NSMakeRect(100,100,400,500);
      self = [super initWithContentRect: contentRect
	styleMask: NSTitledWindowMask|NSClosableWindowMask|NSResizableWindowMask
	backing: NSBackingStoreBuffered
	defer: NO];
      if (nil == self)
        return nil;

      [self setReleasedWhenClosed: NO];
      [self setTitle: _(@"Help")];
      s = [[NSScrollView alloc] initWithFrame: contentRect];
      [s setHasHorizontalScroller: YES];
      [s setHasVerticalScroller: YES];
      [s setAutoresizingMask: (NSViewWidthSizable | NSViewHeightSizable)];
      [self setContentView: s];
      RELEASE(s);

      r = [[s documentView] frame];
      v = [[NSTextView alloc] initWithFrame: r];
      [v setHorizontallyResizable: YES];
      [v setVerticallyResizable: YES];
      [v setEditable: NO];
      [v setRichText: YES];
      [v setMinSize: NSMakeSize (0, 0)];
      [v setMaxSize: NSMakeSize (1E7, 1E7)];
      [v setAutoresizingMask: NSViewHeightSizable | NSViewWidthSizable];
      [[v textContainer] setContainerSize:
	NSMakeSize (MAX(r.size.width, 0.0), 1e7)];
      [[v textContainer] setWidthTracksTextView: YES];

      [s setDocumentView: v];
      RELEASE(v);

      _sharedPanel = self;
    }
  else
    {
      RELEASE(self);
      RETAIN(_sharedPanel);
    }
  return _sharedPanel;
}

//
// Showing Help 
//
- (void) showFile: (NSString *)filename
	 atMarker: (NSString *)markerName
{
  if ([filename isAbsolutePath] == NO)
    {
      filename = [[[NSHelpPanel sharedHelpPanel] helpDirectory]
        stringByAppendingPathComponent: filename];
    }
  [[NSHelpManager sharedHelpManager] setContextHelp: (id)filename
                                          forObject: self];
  [self showHelpAttachedTo: self];
}

- (BOOL) showHelpAttachedTo: (id)anObject
{
  return [[NSHelpManager sharedHelpManager]
    showContextHelpForObject: anObject locationHint: NSZeroPoint];
}

//
// Printing 
//
- (void) print: (id)sender
{
}

@end
