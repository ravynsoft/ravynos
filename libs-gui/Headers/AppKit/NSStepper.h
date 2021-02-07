/*
   NSStepper.h

   The stepper class

   Copyright (C) 2001 Free Software Foundation, Inc.

   Author:  Pierre-Yves Rivaille <pyrivail@ens-lyon.fr>
   Date: August 2001
   
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

#ifndef _GNUstep_H_NSStepper
#define _GNUstep_H_NSStepper

#import <AppKit/NSControl.h>

@interface NSStepper : NSControl
{
  // Attributes
}
- (double)maxValue;
- (void)setMaxValue: (double)maxValue;
- (double)minValue;
- (void)setMinValue: (double)minValue;
- (double)increment;
- (void)setIncrement: (double)increment;

- (BOOL)autorepeat;
- (void)setAutorepeat: (BOOL)autorepeat;
- (BOOL)valueWraps;
- (void)setValueWraps: (BOOL)valueWraps;

@end

#endif // _GNUstep_H_NSStepper
