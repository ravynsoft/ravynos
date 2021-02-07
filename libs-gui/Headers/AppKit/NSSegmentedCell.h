/* NSSegmentedCell.h
 *
 * Copyright (C) 2007 Free Software Foundation, Inc.
 *
 * Author:	Gregory John Casamento <greg_casamento@yahoo.com>
 * Date:	2007
 * 
 * This file is part of GNUstep.
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110 
 * USA.
 */

#ifndef _GNUstep_H_NSSegmentedCell
#define _GNUstep_H_NSSegmentedCell

#import <AppKit/NSActionCell.h>
#import <AppKit/NSSegmentedControl.h>

// tracking types...
typedef enum {
  NSSegmentSwitchTrackingSelectOne = 0,
  NSSegmentSwitchTrackingSelectAny = 1,
  NSSegmentSwitchTrackingMomentary = 2 
} NSSegmentSwitchTracking;

// forward declarations
@class NSMutableArray;
@class NSImage;
@class NSString;
@class NSMenu;
@class NSView;

@interface NSSegmentedCell : NSActionCell
{
  @private
  NSInteger _selected_segment;
  int _key_selection;
  NSMutableArray *_items;
  struct {
    unsigned int _tracking_mode:3;
    unsigned int _trimmed_labels:1;
    unsigned int _drawing:1;
    unsigned int unused1:2;
    unsigned int _recalcToolTips:1;
    unsigned int unused2:3;
    unsigned int _style:8;
    unsigned int unused3:13;
  } _segmentCellFlags;
}

// Specifying number of segments...
- (void) setSegmentCount: (NSInteger) count;
- (NSInteger) segmentCount; 

// Specifying selected segment...
- (void) setSelectedSegment: (NSInteger) segment;
- (void) setSelected: (BOOL)flag forSegment: (NSInteger)segment;
- (NSInteger) selectedSegment;
- (void) selectSegmentWithTag: (NSInteger)tag;
- (void) makeNextSegmentKey;
- (void) makePreviousSegmentKey;

// Specify tracking mode...
- (void) setTrackingMode: (NSSegmentSwitchTracking)mode;
- (NSSegmentSwitchTracking) trackingMode;

// Working with individual segments...
- (void) setWidth: (CGFloat)width forSegment: (NSInteger)segment;
- (CGFloat) widthForSegment: (NSInteger)segment;
- (void) setImage: (NSImage *)image forSegment: (NSInteger)segment;
- (NSImage *) imageForSegment: (NSInteger)segment;
- (void) setLabel: (NSString *)label forSegment: (NSInteger)segment;
- (NSString *) labelForSegment: (NSInteger)segment;
- (BOOL) isSelectedForSegment: (NSInteger)segment;
- (void) setEnabled: (BOOL)flag forSegment: (NSInteger)segment;
- (BOOL) isEnabledForSegment: (NSInteger)segment;
- (void) setMenu: (NSMenu *)menu forSegment: (NSInteger)segment;
- (NSMenu *) menuForSegment: (NSInteger)segment;
- (void) setToolTip: (NSString *) toolTip forSegment: (NSInteger)segment;
- (NSString *) toolTipForSegment: (NSInteger)segment;
- (void) setTag: (NSInteger)tag forSegment: (NSInteger)segment;
- (NSInteger) tagForSegment: (NSInteger)segment;

// Drawing custom content
- (void) drawSegment: (NSInteger)segment 
             inFrame: (NSRect)frame 
            withView: (NSView *)view;

// Setting the style of the segments
#if OS_API_VERSION(GS_API_MACOSX, GS_API_LATEST)
- (void)setSegmentStyle:(NSSegmentStyle)style;
- (NSSegmentStyle)segmentStyle;
#endif

@end
#endif
