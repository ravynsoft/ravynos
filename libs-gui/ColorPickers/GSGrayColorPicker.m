/* GSGrayColorPicker.m

   Copyright (C) 2001 Free Software Foundation, Inc.

   Author:  Fred Kiefer <FredKiefer@gmx.de>
   Date: January 2001
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

#include "GSGrayColorPicker.h"

#include <GNUstepGUI/GSHbox.h>

@implementation GSGrayColorPicker

- (id)initWithPickerMask:(int)aMask
	      colorPanel:(NSColorPanel *)colorPanel
{
  if (aMask & NSColorPanelGrayModeMask)
    {
      NSBundle *b;

      self = [super initWithPickerMask: aMask
		colorPanel: colorPanel];
      if (!self)
	return nil;

      numFields = 1;
      currentMode = NSColorPanelGrayModeMask;
      maxValue = 100;

      b = [NSBundle bundleForClass: [self class]];
      r_names[0] = NSLocalizedStringFromTableInBundle(@"White",@"StandardPicker",b,@"");
      names = r_names;

      sliders = r_sliders;
      fields = r_fields;
      values = r_values;
      return self;
    }
  RELEASE(self);
  return nil;
}

- (void)setColor:(NSColor *)color
{
  CGFloat white, alpha;
  NSColor *c;

  if (updating)
    return;
  updating = YES;

  c = [color colorUsingColorSpaceName: NSCalibratedWhiteColorSpace];
  [c getWhite: &white alpha: &alpha];

  values[0] = white * 100;
  [self _valuesChanged];

  updating = NO;
}

-(void) _setColorFromValues
{
  float white = values[0] / 100;
  float alpha = [_colorPanel alpha];
  NSColor *c = [NSColor colorWithCalibratedWhite: white
			alpha: alpha];
  [_colorPanel setColor: c];
}


- (void) loadViews
{
  int i;
  GSHbox *hb;
  NSColorWell *well;

  [super loadViews];
  [sliders[0] setMaxValue: 100];
  [(GSColorSliderCell *)[sliders[0] cell] _setColorSliderCellMode: 0];

  hb = [[GSHbox alloc] init];
  [hb setAutoresizingMask: NSViewWidthSizable | NSViewMinYMargin];

  for (i = 0; i < 7; i++)
    {
      well = [[NSColorWell alloc] initWithFrame: NSMakeRect(0, 0, 20, 20)];
      [well setColor: [NSColor colorWithCalibratedWhite: (i / 6.0) alpha: 1.0]];
      [well setBordered: NO];
      //[well setTarget: self];
      //[well setAction: @selector(takeColor:)];
      [hb addView: well enablingXResizing: NO];
      RELEASE(well);
    }

  [baseView putView: hb
	atRow: 0
	column: 0
	withXMargins: 0
	yMargins: 4];
  DESTROY(hb);
}


- (void) takeColor: (id) sender
{
  [self setColor: [sender color]];
}

@end

