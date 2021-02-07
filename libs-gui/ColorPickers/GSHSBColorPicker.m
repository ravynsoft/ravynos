/* GSHSBColorPicker.m

   Copyright (C) 2000 Free Software Foundation, Inc.

   Author:  Fred Kiefer <FredKiefer@gmx.de>
   Date: December 2000
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

#include "GSHSBColorPicker.h"

@implementation GSHSBColorPicker

- (id)initWithPickerMask:(int)aMask
              colorPanel:(NSColorPanel *)colorPanel
{
  if (aMask & NSColorPanelHSBModeMask)
    {
      NSBundle *b;

      self = [super initWithPickerMask: aMask
                colorPanel: colorPanel];
      if (!self)
        return nil;

      numFields = 3;
      currentMode = NSColorPanelHSBModeMask;

      b = [NSBundle bundleForClass: [self class]];
      r_names[0] = NSLocalizedStringFromTableInBundle(@"Hue",@"StandardPicker",b,@"");
      r_names[1] = NSLocalizedStringFromTableInBundle(@"Saturation",@"StandardPicker",b,@"");
      r_names[2] = NSLocalizedStringFromTableInBundle(@"Brightness",@"StandardPicker",b,@"");
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
  CGFloat hue, saturation, brightness, alpha;
  NSColor *c;

  if (updating)
    return;
  updating = YES;

  c = [color colorUsingColorSpaceName: NSCalibratedRGBColorSpace];
  [c getHue: &hue saturation: &saturation brightness: &brightness alpha: &alpha];

  values[0] = hue * 360;
  values[1] = saturation * 100;
  values[2] = brightness * 100;
  [self _valuesChanged];

  [(GSColorSliderCell *)[sliders[0] cell]
    _setColorSliderCellValues: hue : saturation : brightness];
  [(GSColorSliderCell *)[sliders[1] cell]
    _setColorSliderCellValues: hue : saturation : brightness];
  [(GSColorSliderCell *)[sliders[2] cell]
    _setColorSliderCellValues: hue : saturation : brightness];
  [sliders[0] setNeedsDisplay: YES];
  [sliders[1] setNeedsDisplay: YES];
  [sliders[2] setNeedsDisplay: YES];

  updating = NO;
}

-(void) _setColorFromValues
{
  float hue = values[0] / 360;
  float saturation = values[1] / 100;
  float brightness  = values[2] / 100;
  float alpha = [_colorPanel alpha];
  NSColor *c = [NSColor colorWithCalibratedHue: hue
                        saturation: saturation
                        brightness: brightness
                        alpha: alpha];
  [_colorPanel setColor: c];

  [(GSColorSliderCell *)[sliders[0] cell]
    _setColorSliderCellValues: hue : saturation : brightness];
  [(GSColorSliderCell *)[sliders[1] cell]
    _setColorSliderCellValues: hue : saturation : brightness];
  [(GSColorSliderCell *)[sliders[2] cell]
    _setColorSliderCellValues: hue : saturation : brightness];
  [sliders[0] setNeedsDisplay: YES];
  [sliders[1] setNeedsDisplay: YES];
  [sliders[2] setNeedsDisplay: YES];
}


- (void) loadViews
{
  [super loadViews];
  [sliders[0] setMaxValue: 360];
  [sliders[1] setMaxValue: 100];
  [sliders[2] setMaxValue: 100];
  [(GSColorSliderCell *)[sliders[0] cell] _setColorSliderCellMode: 8];
  [(GSColorSliderCell *)[sliders[1] cell] _setColorSliderCellMode: 9];
  [(GSColorSliderCell *)[sliders[2] cell] _setColorSliderCellMode: 10];
}

@end

