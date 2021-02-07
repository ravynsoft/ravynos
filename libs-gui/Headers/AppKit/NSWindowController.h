/* 
   NSWindowController.h

   The document controller class

   Copyright (C) 1999 Free Software Foundation, Inc.

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

#ifndef _GNUstep_H_NSWindowController
#define _GNUstep_H_NSWindowController

#import <AppKit/NSNibDeclarations.h>
#import <AppKit/NSResponder.h>
#import <AppKit/NSSeguePerforming.h>

@class NSString;
@class NSArray;
@class NSWindow;
@class NSDocument;
@class NSMapTable;
@class NSStoryboard;

@interface NSWindowController : NSResponder <NSCoding, NSSeguePerforming>
{
  @private
    NSWindow            *_window;
    NSString            *_window_nib_name;
    NSString            *_window_nib_path;
    NSString            *_window_frame_autosave_name;
    NSDocument          *_document;
    NSArray             *_top_level_objects;
    id                  _owner;
    NSMapTable          *_segueMap;
    NSStoryboard        *_storyboard; // a weak reference to the origin storyboard
    struct ___wcFlags 
    {
      unsigned int should_close_document:1;
      unsigned int should_cascade:1;
      unsigned int nib_is_loaded:1;
      unsigned int RESERVED:29;
    } _wcFlags;
    void                *_reserved1;
    void                *_reserved2;
}

- (id) initWithWindowNibName: (NSString *)windowNibName;  // self is the owner
- (id) initWithWindowNibName: (NSString *)windowNibName  owner: (id)owner;
- (id) initWithWindow: (NSWindow *)window;
- (id) initWithWindowNibPath: (NSString *)windowNibPath
		       owner: (id)owner;

- (void) loadWindow;
- (IBAction) showWindow: (id)sender;
- (BOOL) isWindowLoaded;
- (NSWindow *) window;
- (void) setWindow: (NSWindow *)aWindow;
- (void) windowDidLoad;
- (void) windowWillLoad;

- (void) setDocument: (NSDocument *)document;
- (id) document;
- (void) setDocumentEdited: (BOOL)flag;

- (void) close;
- (BOOL) shouldCloseDocument;
- (void) setShouldCloseDocument: (BOOL)flag;

- (id) owner;
- (NSString *) windowNibName;
- (NSString *) windowNibPath;

- (BOOL) shouldCascadeWindows;
- (void) setShouldCascadeWindows: (BOOL)flag;
- (void) setWindowFrameAutosaveName: (NSString *)name;
- (NSString *) windowFrameAutosaveName;
- (NSString *) windowTitleForDocumentDisplayName: (NSString *)displayName;
- (void) synchronizeWindowTitleWithDocumentName;
@end

#endif /* _GNUstep_H_NSWindowController */
