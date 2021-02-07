/* 
   NSDataLinkManager.h

   Manager of a NSDataLink

   Copyright (C) 1996 Free Software Foundation, Inc.

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

#ifndef _GNUstep_H_NSDataLinkManager
#define _GNUstep_H_NSDataLinkManager
#import <GNUstepBase/GSVersionMacros.h>

#import <Foundation/NSObject.h>

@class NSString;
@class NSEnumerator;
@class NSMutableArray;
@class NSDataLink;
@class NSSelection;
@class NSPasteboard;
@class NSWindow;

@interface NSDataLinkManager : NSObject <NSCoding>
{
  // Attributes
  id                  delegate;
  NSString            *filename;
  NSMutableArray      *sourceLinks;
  NSMutableArray      *destinationLinks;

  struct __dlmFlags {
    unsigned areLinkOutlinesVisible:1;
    unsigned delegateVerifiesLinks:1;
    unsigned interactsWithUser:1;
    unsigned isEdited:1;
  } _flags;

}

//
// Initializing and Freeing a Link Manager
//
- (id)initWithDelegate:(id)anObject;
- (id)initWithDelegate:(id)anObject
	      fromFile:(NSString *)path;

//
// Adding and Removing Links
//
- (BOOL)addLink:(NSDataLink *)link
	     at:(NSSelection *)selection;
- (BOOL)addLinkAsMarker:(NSDataLink *)link
		     at:(NSSelection *)selection;
- (NSDataLink *)addLinkPreviouslyAt:(NSSelection *)oldSelection
		     fromPasteboard:(NSPasteboard *)pasteboard
				 at:(NSSelection *)selection;
- (void)breakAllLinks;
- (void)writeLinksToPasteboard:(NSPasteboard *)pasteboard;

//
// Informing the Link Manager of Document Status
//
- (void)noteDocumentClosed;
- (void)noteDocumentEdited;
- (void)noteDocumentReverted;
- (void)noteDocumentSaved;
- (void)noteDocumentSavedAs:(NSString *)path;
- (void)noteDocumentSavedTo:(NSString *)path;

//
// Getting and Setting Information about the Link Manager
//
- (id)delegate;
- (BOOL)delegateVerifiesLinks;
- (NSString *)filename;
- (BOOL)interactsWithUser;
- (BOOL)isEdited;
- (void)setDelegateVerifiesLinks:(BOOL)flag;
- (void)setInteractsWithUser:(BOOL)flag;

//
// Getting and Setting Information about the Manager's Links
//
- (BOOL)areLinkOutlinesVisible;
- (NSEnumerator *)destinationLinkEnumerator;
- (NSDataLink *)destinationLinkWithSelection:(NSSelection *)destSel;
- (void)setLinkOutlinesVisible:(BOOL)flag;
- (NSEnumerator *)sourceLinkEnumerator;
@end


//
// Methods Implemented by the Delegate
//
@interface NSObject (NSDataLinkManagerDelegate)
// data link management methods.
- (void)dataLinkManager:(NSDataLinkManager *)sender 
	   didBreakLink:(NSDataLink *)link;
- (BOOL)dataLinkManager:(NSDataLinkManager *)sender 
  isUpdateNeededForLink:(NSDataLink *)link;
- (void)dataLinkManager:(NSDataLinkManager *)sender 
      startTrackingLink:(NSDataLink *)link;
- (void)dataLinkManager:(NSDataLinkManager *)sender 
       stopTrackingLink:(NSDataLink *)link;
- (void)dataLinkManagerCloseDocument:(NSDataLinkManager *)sender;
- (void)dataLinkManagerDidEditLinks:(NSDataLinkManager *)sender;
- (void)dataLinkManagerRedrawLinkOutlines:(NSDataLinkManager *)sender;
- (BOOL)dataLinkManagerTracksLinksIndividually:(NSDataLinkManager *)sender;

// selection management methods.
- (BOOL)copyToPasteboard:(NSPasteboard *)pasteboard 
		      at:(NSSelection *)selection
	cheapCopyAllowed:(BOOL)flag;
- (BOOL)importFile:(NSString *)filename
		at:(NSSelection *)selection;
- (BOOL)pasteFromPasteboard:(NSPasteboard *)pasteboard 
			 at:(NSSelection *)selection;
- (BOOL)showSelection:(NSSelection *)selection;
- (NSWindow *)windowForSelection:(NSSelection *)selection;
@end


//
// Draw a Distinctive Outline around Linked Data
//
void NSFrameLinkRect(NSRect aRect, BOOL isDestination);
float NSLinkFrameThickness(void);

#endif // _GNUstep_H_NSDataLinkManager
