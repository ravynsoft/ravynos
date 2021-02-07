/* 
   NSCustomImageRep.h

   Custom image representation.

   Copyright (C) 1996 Free Software Foundation, Inc.

   Written by:  Adam Fedor <fedor@colorado.edu>
   Date: Feb 1996
   
   This file is part of the GNUstep GUI Library.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
   
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; see the file COPYING.LIB.
   If not, write to the Free Software Foundation,
   51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/ 

#ifndef _GNUstep_H_NSCustomImageRep
#define _GNUstep_H_NSCustomImageRep
#import <GNUstepBase/GSVersionMacros.h>

#import <AppKit/NSImageRep.h>

@interface NSCustomImageRep : NSImageRep
{
  // Attributes
  id  _delegate;
  SEL _selector;
}

- (id)initWithDrawSelector:(SEL)aSelector
		  delegate:(id)anObject;

//
// Identifying the Object 
//
- (id)delegate;
- (SEL)drawSelector;

@end

#endif // _GNUstep_H_NSCustomImageRep
