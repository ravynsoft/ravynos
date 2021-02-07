/*
   NSScrollView.h

   A view that allows you to scroll a document view that's too big to display
   entirely on a window.

   Copyright (C) 1996-2015 Free Software Foundation, Inc.

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

#ifndef _GNUstep_H_NSScrollView
#define _GNUstep_H_NSScrollView
#import <GNUstepBase/GSVersionMacros.h>

#import <AppKit/NSView.h>

enum
{
  NSScrollElasticityAutomatic = 0,
  NSScrollElasticityNone      = 1,
  NSScrollElasticityAllowed   = 2
};
typedef NSInteger NSScrollElasticity;


@class NSClipView;
@class NSRulerView;
@class NSColor;
@class NSCursor;
@class NSScroller;

@interface NSScrollView : NSView
{
  NSClipView *_contentView;
  NSScroller *_horizScroller;
  NSScroller *_vertScroller;
  NSRulerView *_horizRuler;
  NSRulerView *_vertRuler;
  NSClipView *_headerClipView;
  NSView *_cornerView;
  float _hLineScroll;
  float _hPageScroll;
  float _vLineScroll;
  float _vPageScroll;
  NSBorderType _borderType;
  BOOL _hasHorizScroller;
  BOOL _hasVertScroller;
  BOOL _hasHorizRuler;
  BOOL _hasVertRuler;
  BOOL _scrollsDynamically;
  BOOL _rulersVisible;
  BOOL _knobMoved;
  BOOL _hasHeaderView;
  BOOL _hasCornerView;
  BOOL _autohidesScrollers;
  NSScrollElasticity _horizScrollElasticity;
  NSScrollElasticity _vertScrollElasticity;
}

/* Calculating layout */
+ (NSSize)contentSizeForFrameSize:(NSSize)frameSize
  hasHorizontalScroller:(BOOL)hFlag
  hasVerticalScroller:(BOOL)vFlag
  borderType:(NSBorderType)borderType;
+ (NSSize)frameSizeForContentSize:(NSSize)contentSize
  hasHorizontalScroller:(BOOL)hFlag
  hasVerticalScroller:(BOOL)vFlag
  borderType:(NSBorderType)borderType;

/* Determining component sizes */
- (NSSize)contentSize;
- (NSRect)documentVisibleRect;

/* Managing graphic attributes */
- (void)setBackgroundColor:(NSColor*)aColor;
- (NSColor*)backgroundColor;
- (void)setBorderType:(NSBorderType)borderType;
- (NSBorderType)borderType;

/* Managing the scrolled views */
- (void)setContentView:(NSClipView*)aView;
- (NSClipView*)contentView;
- (void)setDocumentView:(NSView*)aView;
- (id)documentView;
- (void)setDocumentCursor:(NSCursor*)aCursor;
- (NSCursor*)documentCursor;

/* Managing scrollers */
- (void)setHorizontalScroller:(NSScroller*)aScroller;
- (NSScroller*)horizontalScroller;
- (void)setHasHorizontalScroller:(BOOL)flag;
- (BOOL)hasHorizontalScroller;
- (void)setVerticalScroller:(NSScroller*)aScroller;
- (NSScroller*)verticalScroller;
- (void)setHasVerticalScroller:(BOOL)flag;
- (BOOL)hasVerticalScroller;

/* Managing rulers */
+ (void)setRulerViewClass:(Class)aClass;
+ (Class)rulerViewClass;
- (void)setHasHorizontalRuler:(BOOL)flag;
- (BOOL)hasHorizontalRuler;
- (void)setHorizontalRulerView:(NSRulerView*)aRulerView;
- (NSRulerView*)horizontalRulerView;
- (void)setHasVerticalRuler:(BOOL)flag;
- (BOOL)hasVerticalRuler;
- (void)setVerticalRulerView:(NSRulerView*)aRulerView;
- (NSRulerView*)verticalRulerView;
- (void)setRulersVisible:(BOOL)flag;
- (BOOL)rulersVisible;

/* Setting scrolling behavior */
- (void)setLineScroll:(CGFloat)aFloat;
- (CGFloat)lineScroll;
- (void)setPageScroll:(CGFloat)aFloat;
- (CGFloat)pageScroll;
- (void)setScrollsDynamically:(BOOL)flag;
- (BOOL)scrollsDynamically;
#if OS_API_VERSION(GS_API_MACOSX, GS_API_LATEST)
- (CGFloat) horizontalLineScroll;
- (CGFloat) horizontalPageScroll;
- (CGFloat) verticalLineScroll;
- (CGFloat) verticalPageScroll;
- (void) setHorizontalLineScroll: (CGFloat)aFloat;
- (void) setHorizontalPageScroll: (CGFloat)aFloat;
- (void) setVerticalLineScroll: (CGFloat)aFloat;
- (void) setVerticalPageScroll: (CGFloat)aFloat;
/* Setting the background drawing */
- (void)setDrawsBackground:(BOOL)flag;
- (BOOL)drawsBackground;
#endif
#if OS_API_VERSION(MAC_OS_X_VERSION_10_3, GS_API_LATEST)
- (BOOL)autohidesScrollers;
- (void)setAutohidesScrollers:(BOOL)flag;
#endif
#if OS_API_VERSION(MAC_OS_X_VERSION_10_7, GS_API_LATEST)
- (NSScrollElasticity)horizontalScrollElasticity;
- (void)setHorizontalScrollElasticity:(NSScrollElasticity)value;
- (NSScrollElasticity)verticalScrollElasticity;
- (void)setVerticalScrollElasticity:(NSScrollElasticity)value;
#endif
#if OS_API_VERSION(MAC_OS_X_VERSION_10_8, GS_API_LATEST)
- (BOOL)allowsMagnification;
- (void)setAllowsMagnification:(BOOL)m;
#endif

#if OS_API_VERSION(MAC_OS_X_VERSION_10_10, GS_API_LATEST)
#if GS_HAS_DECLARED_PROPERTIES
@property BOOL automaticallyAdjustsContentInsets;
@property NSEdgeInsets contentInsets;
@property NSEdgeInsets scrollerInsets;
#else
- (BOOL)automaticallyAdjustsContentInsets;
- (void)setAutomaticallyAdjustsContentInsets: (BOOL)adjusts;

- (NSEdgeInsets)contentInsets;
- (void)setContentInsets:(NSEdgeInsets)edgeInsets;

- (NSEdgeInsets)scrollerInsets;
- (void)setScrollerInsets:(NSEdgeInsets)insets;
#endif
#endif

/* Updating display after scrolling */
- (void)reflectScrolledClipView:(NSClipView*)aClipView;

/* Arranging components */
- (void)tile;

@end

#endif /* _GNUstep_H_NSScrollView */
