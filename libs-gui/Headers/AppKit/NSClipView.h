/*
   NSClipView.h

   The class that contains the document view displayed by a NSScrollView.

   Copyright (C) 1996 Free Software Foundation, Inc.

   Author: Ovidiu Predescu <ovidiu@net-community.com>
   Date: July 1997
   
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

#ifndef _GNUstep_H_NSClipView
#define _GNUstep_H_NSClipView
#import <GNUstepBase/GSVersionMacros.h>

#import <AppKit/NSView.h>

@class NSNotification;
@class NSCursor;
@class NSColor;

@interface NSClipView : NSView
{
  NSView* _documentView;
  NSCursor* _cursor;
  NSColor* _backgroundColor;
  BOOL _drawsBackground;
  BOOL _copiesOnScroll;
  /* Cached */
  BOOL _isOpaque;
}

/* Setting the document view */
- (void)setDocumentView:(NSView*)aView;
- (id)documentView;

/* Scrolling */
- (void)scrollToPoint:(NSPoint)aPoint;
- (BOOL)autoscroll:(NSEvent*)theEvent;
- (NSPoint)constrainScrollPoint:(NSPoint)proposedNewOrigin;

/* Determining scrolling efficiency */
- (void)setCopiesOnScroll:(BOOL)flag;
- (BOOL)copiesOnScroll;

/* Getting the visible portion */
- (NSRect)documentRect;
- (NSRect)documentVisibleRect;

/* Setting the document cursor */
- (void)setDocumentCursor:(NSCursor*)aCursor;
- (NSCursor*)documentCursor;

/* Setting the background color */
- (void)setBackgroundColor:(NSColor*)aColor;
- (NSColor*)backgroundColor;

#if OS_API_VERSION(GS_API_MACOSX, GS_API_LATEST)
/* Setting the background drawing */
- (void)setDrawsBackground:(BOOL)flag;
- (BOOL)drawsBackground;
#endif

@end

#endif /* _GNUstep_H_NSClipView */
