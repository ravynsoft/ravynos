/* 
   NSDragging.h

   Protocols for drag 'n' drop.

   Copyright (C) 1997 Free Software Foundation, Inc.

   Author:  Simon Frankau <sgf@frankau.demon.co.uk>
   Date: 1997
   
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

#ifndef _GNUstep_H_NSDragging
#define _GNUstep_H_NSDragging
#import <GNUstepBase/GSVersionMacros.h>

#import <Foundation/NSGeometry.h>

@class NSWindow;
@class NSPasteboard;
@class NSImage;
@class NSURL;
@class NSDraggingSession;

enum _NSDragOperation {
  NSDragOperationNone = 0,
  NSDragOperationCopy = 1,
  NSDragOperationLink = 2,
  NSDragOperationGeneric = 4,
  NSDragOperationPrivate = 8,
  NSDragOperationAll = 15,
  NSDragOperationMove = 16,
  NSDragOperationDelete = 32,
  NSDragOperationEvery = UINT_MAX  
};

typedef NSUInteger NSDragOperation;

#if OS_API_VERSION(MAC_OS_X_VERSION_10_7, GS_API_LATEST)
typedef NSInteger NSDraggingContext;
enum
{
  NSDraggingContextOutsideApplication = 0,
  NSDraggingContextWithinApplication
};
#endif

@protocol NSDraggingInfo

//
// Dragging-session Information
//
- (NSWindow *)draggingDestinationWindow;
- (NSPoint)draggingLocation;
- (NSPasteboard *)draggingPasteboard;
- (NSInteger)draggingSequenceNumber;
- (id)draggingSource;
- (NSDragOperation)draggingSourceOperationMask;

//
// Image Information
//
- (NSImage *)draggedImage;
- (NSPoint)draggedImageLocation;

//
// Sliding the Image
//
- (void)slideDraggedImageTo:(NSPoint)screenPoint;

#if OS_API_VERSION(MAC_OS_X_VERSION_10_2, GS_API_LATEST)
- (NSArray *)namesOfPromisedFilesDroppedAtDestination:(NSURL *)dropDestination;
#endif
@end

@interface NSObject (NSDraggingDestination)

//
// Before the Image is Released
//
- (NSDragOperation)draggingEntered:(id <NSDraggingInfo>)sender;
- (NSDragOperation)draggingUpdated:(id <NSDraggingInfo>)sender;
- (void)draggingExited:(id <NSDraggingInfo>)sender;

//
// After the Image is Released
//
- (BOOL)prepareForDragOperation:(id <NSDraggingInfo>)sender;
- (BOOL)performDragOperation:(id <NSDraggingInfo>)sender;
- (void)concludeDragOperation:(id <NSDraggingInfo>)sender;

#if OS_API_VERSION(GS_API_MACOSX, GS_API_LATEST)
- (void)draggingEnded: (id <NSDraggingInfo>)sender;
#endif
#if OS_API_VERSION(MAC_OS_X_VERSION_10_4, GS_API_LATEST)
- (BOOL)wantsPeriodicDraggingUpdates;
#endif
@end

#if OS_API_VERSION(MAC_OS_X_VERSION_10_7, GS_API_LATEST)
@protocol NSDraggingSource <NSObject>
- (NSDragOperation) draggingSession:(NSDraggingSession *)session
sourceOperationMaskForDraggingContext:(NSDraggingContext)context;

#if GS_PROTOCOLS_HAVE_OPTIONAL
@optional
#else
@end
@interface NSObject (NSDraggingSource107)
#endif
- (void) draggingSession: (NSDraggingSession *)session
        willBeginAtPoint: (NSPoint)screenPoint;
- (void) draggingSession: (NSDraggingSession *)session
            movedToPoint: (NSPoint)screenPoint;
- (void) draggingSession: (NSDraggingSession *)session
            endedAtPoint: (NSPoint)screenPoint
               operation: (NSDragOperation)operation;

- (BOOL) ignoreModifierKeysForDraggingSession:(NSDraggingSession *)session;
@end
#endif

@interface NSObject (NSDraggingSource)

//
// Querying the Source
//
- (NSDragOperation)draggingSourceOperationMaskForLocal:(BOOL)isLocal;
- (BOOL)ignoreModifierKeysWhileDragging;

//
// Informing the Source
//
- (void)draggedImage:(NSImage *)image
             beganAt:(NSPoint)screenPoint;
- (void)draggedImage: (NSImage*)image
             endedAt: (NSPoint)screenPoint
           deposited: (BOOL)didDeposit;

#if OS_API_VERSION(GS_API_MACOSX, GS_API_LATEST)
- (void)draggedImage: (NSImage*)image
             endedAt: (NSPoint)screenPoint
           operation: (NSDragOperation)operation;
- (void)draggedImage: (NSImage*)image
             movedTo: (NSPoint)screenPoint;
#endif 
#if OS_API_VERSION(MAC_OS_X_VERSION_10_2, GS_API_LATEST)
- (NSArray *)namesOfPromisedFilesDroppedAtDestination:(NSURL *)dropDestination;
#endif
@end

#endif // _GNUstep_H_NSDragging
