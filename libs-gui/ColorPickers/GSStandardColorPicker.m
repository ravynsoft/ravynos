/* GSStandardColorPicker.m

   Copyright (C) 2001 Free Software Foundation, Inc.

   Author:  Fred Kiefer <FredKiefer@gmx.de>
   Date: Febuary 2001
   Author: Alexander Malmberg <alexander@malmberg.org>
   Date: May 2002

   This file is part of GNUstep.
   
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

#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>
#import <GNUstepGUI/GSVbox.h>
#import <GNUstepGUI/GSHbox.h>
#import "GSRGBColorPicker.h"
#import "GSHSBColorPicker.h"
#import "GSCMYKColorPicker.h"
#import "GSGrayColorPicker.h"
#import "GSStandardColorPicker.h"

@implementation GSStandardColorPicker

- (void) dealloc
{
  RELEASE(pickers);
  RELEASE(pickerBox);
  RELEASE(pickerMatrix);
  RELEASE(baseView);
  [super dealloc]; 
}

- (id)initWithPickerMask:(int)aMask
              colorPanel:(NSColorPanel *)colorPanel
{
  if (aMask & (NSColorPanelRGBModeMask | NSColorPanelHSBModeMask | 
               NSColorPanelCMYKModeMask | NSColorPanelGrayModeMask))
  {
    NSColorPicker *picker;

    pickers = [[NSMutableArray alloc] init];
    picker = [[GSGrayColorPicker alloc] initWithPickerMask: aMask
                                        colorPanel: colorPanel];
    if (picker != nil)
      {
        [pickers addObject: picker];
        RELEASE(picker);
      }
    picker = [[GSRGBColorPicker alloc] initWithPickerMask: aMask
                                       colorPanel: colorPanel];
    if (picker != nil)
      {
        [pickers addObject: picker];
        RELEASE(picker);
      }
    picker = [[GSCMYKColorPicker alloc] initWithPickerMask: aMask
                                        colorPanel: colorPanel];
    if (picker != nil)
      {
        [pickers addObject: picker];
        RELEASE(picker);
      }
    picker = [[GSHSBColorPicker alloc] initWithPickerMask: aMask
                                       colorPanel: colorPanel];
    if (picker != nil)
      {
        [pickers addObject: picker];
        RELEASE(picker);
      }

    currentPicker = [pickers lastObject];
    return [super initWithPickerMask: aMask
                  colorPanel: colorPanel];
  }
  RELEASE(self);
  return nil;
}

- (int)currentMode
{
  return [currentPicker currentMode];
}

- (void)setMode:(int)mode
{
  int i, count;

  if (mode == [self currentMode])
    return;

  count = [pickers count];
  for (i = 0; i < count; i++)
    {
      if ([[pickers objectAtIndex: i] supportsMode: mode])
        {
          [pickerMatrix selectCellWithTag: i];
          [self _showNewPicker: pickerMatrix];
          [currentPicker setMode: mode];
          break;
        }
    }
}

- (BOOL)supportsMode:(int)mode
{
  return ((mode == NSGrayModeColorPanel) ||
          (mode == NSRGBModeColorPanel)  ||
          (mode == NSCMYKModeColorPanel) ||
          (mode == NSHSBModeColorPanel));
}

- (void)insertNewButtonImage:(NSImage *)newImage
                          in:(NSButtonCell *)newButtonCell
{
  // Store the image button cell
  imageCell = newButtonCell;
  [super insertNewButtonImage: newImage
         in: newButtonCell];
}

- (NSView *)provideNewView:(BOOL)initialRequest
{
  if (initialRequest)
    {
      [self loadViews];
    }
  return baseView;
}

- (NSImage *)provideNewButtonImage
{
  return [currentPicker provideNewButtonImage];
}

- (void)setColor:(NSColor *)color
{
  [currentPicker setColor: color];
}

- (void) loadViews
{
  NSEnumerator *enumerator;
  id<NSColorPickingCustom, NSColorPickingDefault> picker;
  NSButtonCell *cell;
  NSMutableArray *cells = [NSMutableArray new];
  int i, count;

  // Initaliase all the sub pickers
  enumerator = [pickers objectEnumerator];

  while ((picker = [enumerator nextObject]) != nil)
    [picker provideNewView: YES];

  baseView = [[GSTable alloc] initWithNumberOfRows: 3 numberOfColumns: 1];
  [baseView setAutoresizingMask: NSViewWidthSizable | NSViewHeightSizable];
  [baseView setYResizingEnabled: NO forRow: 1];
  [baseView setYResizingEnabled: NO forRow: 2];

  // Prototype cell for the matrix
  cell = [[NSButtonCell alloc] initImageCell: nil];
  [cell setButtonType: NSOnOffButton];
  [cell setBordered: YES];

  count = [pickers count];
  pickerMatrix = [[NSMatrix alloc] initWithFrame: NSMakeRect(0,0,0,0)
                                   mode: NSRadioModeMatrix
                                   prototype: cell
                                   numberOfRows: 0
                                   numberOfColumns: count];
  RELEASE(cell);
  [pickerMatrix setAutoresizingMask: (NSViewWidthSizable | NSViewHeightSizable)];
  [pickerMatrix setIntercellSpacing: NSMakeSize(1, 0)];
  [pickerMatrix setAutosizesCells: YES];

  for (i = 0; i < count; i++)
    {
      cell = [[pickerMatrix prototype] copy];
      [cell setTag: i];
      picker = [pickers objectAtIndex: i];
      [picker insertNewButtonImage: [picker provideNewButtonImage] in: cell];
      [cells addObject: cell];
      RELEASE(cell);
    }

  [pickerMatrix addRowWithCells: cells];
  RELEASE(cells);
  [pickerMatrix setCellSize: NSMakeSize(1, 36)];
  [pickerMatrix setTarget: self];
  [pickerMatrix setAction: @selector(_showNewPicker:)];

  pickerBox = [[NSBox alloc] init];
  [pickerBox setAutoresizingMask: NSViewWidthSizable | NSViewHeightSizable];
  [baseView putView: pickerBox
        atRow: 0
        column: 0
        withMargins: 0];
  [pickerBox setTitlePosition: NSNoTitle];
  [pickerBox setBorderType: NSNoBorder];
  [pickerBox setContentView: [currentPicker provideNewView: NO]];

  [baseView putView: pickerMatrix
        atRow: 1
        column: 0
        withMargins: 0];

  {
    NSBox *b = [[NSBox alloc] initWithFrame: NSMakeRect(0,0,0,2)];
    [b setAutoresizingMask: NSViewWidthSizable];
    [b setTitlePosition: NSNoTitle];
    [b setBorderType: NSGrooveBorder];
    [baseView putView: b
        atRow: 2
        column: 0
        withMinXMargin: 0
        maxXMargin: 0
        minYMargin: 4
        maxYMargin: 0];
    DESTROY(b);
  }
}

- (void) _showNewPicker: (id) sender
{
  currentPicker = [pickers objectAtIndex: [sender selectedColumn]];
  [currentPicker setColor: [_colorPanel color]];
  [pickerBox setContentView: [currentPicker provideNewView: NO]];

  // Show the new image
  [imageCell setImage: [[sender selectedCell] image]];
}

@end



@implementation GSStandardCSColorPicker

- (void) dealloc
{
  int i;
  for (i = 0; i < numFields; i++)
    {
      RELEASE(sliders[i]);
      RELEASE(fields[i]);
    }
  RELEASE(baseView);
  [super dealloc];
}

- (int)currentMode
{
  return currentMode;
}

- (BOOL)supportsMode:(int)mode
{
  return currentMode == NSRGBModeColorPanel;
}

- (NSView *)provideNewView:(BOOL)initialRequest
{
  if (initialRequest)
    {
      [self loadViews];
    }
  return baseView;
}

- (void)setColor:(NSColor *)color
{
  [self subclassResponsibility: _cmd];
}

- (void) loadViews
{
  int i;

  baseView = [[GSTable alloc] initWithNumberOfRows: numFields + 2 numberOfColumns: 2];
  /* keep a single resizable but empty row at the bottom so it sucks up any
  extra space in the box that contains us */
  for (i = 1; i < numFields + 2; i++)
    [baseView setYResizingEnabled: NO forRow: i];
  [baseView setXResizingEnabled: YES forColumn: 0];
  [baseView setXResizingEnabled: NO forColumn: 1];
  [baseView setAutoresizingMask: NSViewWidthSizable | NSViewMinYMargin];

  for (i = 0; i < numFields; i++)
    {
      NSSlider *s;
      NSCell *c;

      s = sliders[i] = [[NSSlider alloc] initWithFrame: NSMakeRect(0, 0, 0, 16)];
      c = [[GSColorSliderCell alloc] init];
      [s setCell: c];
      RELEASE(c);
      [s setContinuous: YES];
      [s setMinValue: 0.0];
      [s setTitle: names[i]];
      [s setTarget: self];
      [s setAction: @selector(sliderChanged:)];

      [[s cell] setBezeled: YES];
      [s setAutoresizingMask: NSViewWidthSizable];

      [baseView putView: s
        atRow: numFields - i
        column: 0
        withMinXMargin: 0
        maxXMargin: 0
        minYMargin: 0
        maxYMargin: i?6:2];
    }

  if (maxValue)
    {
      NSTextField *tv;
      GSHbox *hb=[[GSHbox alloc] init];

      tv=[[NSTextField alloc] init];
      [tv setStringValue: @"0"];
      [tv setEditable: 0];
      [tv setFont: [NSFont userFontOfSize: 10.0]];
      [tv setTextColor: [NSColor darkGrayColor]];
      [tv setDrawsBackground: NO];
      [tv setBordered: NO];
      [tv setSelectable: NO];
      [tv setBezeled: NO];
      [tv sizeToFit];
      [tv setAutoresizingMask: NSViewMaxXMargin];
      [hb addView: tv];
      DESTROY(tv);

      tv=[[NSTextField alloc] init];
      [tv setIntValue: maxValue];
      [tv setEditable: 0];
      [tv setFont: [NSFont userFontOfSize: 10.0]];
      [tv setTextColor: [NSColor darkGrayColor]];
      [tv setDrawsBackground: NO];
      [tv setBordered: NO];
      [tv setSelectable: NO];
      [tv setBezeled: NO];
      [tv sizeToFit];
      [tv setAutoresizingMask: NSViewMinXMargin];
      [hb addView: tv];
      DESTROY(tv);

      [hb setAutoresizingMask: NSViewWidthSizable];
      [baseView putView: hb
        atRow: numFields + 1
        column: 0
        withMargins: 0];
      DESTROY(hb);
    }
  else
    {
      NSTextField *tv;
      NSView *v;
      NSRect frame;

      tv=[[NSTextField alloc] init];
      [tv setStringValue: @"0"];
      [tv setEditable: 0];
      [tv setFont: [NSFont userFontOfSize: 10.0]];
      [tv setTextColor: [NSColor darkGrayColor]];
      [tv setDrawsBackground: NO];
      [tv setBordered: NO];
      [tv setSelectable: NO];
      [tv setBezeled: NO];
      [tv sizeToFit];
      frame=[tv frame];
      DESTROY(tv);
      v = [[NSView alloc] initWithFrame: frame];
      [baseView putView: v
        atRow: numFields + 1
        column: 0
        withMargins: 0];
      DESTROY(v);
    }

  for (i = 0; i < numFields; i++)
    {
      NSTextField *f;
      f = fields[i] = [[NSTextField alloc] init];
      [f setStringValue: @"255"]; /* just to get a good size */
      [f setFont: [NSFont userFontOfSize: 10.0]];
      [f sizeToFit];
      [f setFrameSize: NSMakeSize([f frame].size.width * 1.5, [f frame].size.height)];
      [f setDelegate: self];
      [baseView putView: f
        atRow: numFields - i
        column: 1
        withMinXMargin: 3
        maxXMargin: 0
        minYMargin: 0
        maxYMargin: 0];
    }
}

- (void) sliderChanged: (id) sender
{
  int i;

  if (updating)
    return;
  updating = YES;

  for (i = 0; i < numFields; i++)
    {
      values[i] = [sliders[i] floatValue];
      [fields[i] setIntValue: (int)values[i]];
    }

  [self _setColorFromValues];

  updating = NO;
}

-(void) controlTextDidChange: (NSNotification *)n
{
  int i;

  if (updating)
    return;
  updating = YES;

  for (i = 0; i < numFields; i++)
    {
      values[i] = [fields[i] floatValue];
      [sliders[i] setIntValue: (int)values[i]];
    }

  [self _setColorFromValues];

  updating = NO;
}

-(void) _valuesChanged
{
  int i;

  for (i = 0; i < numFields; i++)
    {
      [fields[i] setIntValue: (int)values[i]];
      [sliders[i] setIntValue: (int)values[i]];
    }
}

-(void) _setColorFromValues
{
  [self subclassResponsibility: _cmd];
}

@end
