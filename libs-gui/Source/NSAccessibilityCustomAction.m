/* Implementation of class NSAccessibilityCustomAction
   Copyright (C) 2020 Free Software Foundation, Inc.
   
   By: Gregory John Casamento
   Date: Mon 15 Jun 2020 03:18:47 AM EDT

   This file is part of the GNUstep Library.
   
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.
   
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.
   
   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free
   Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110 USA.
*/

#import "AppKit/NSAccessibilityCustomAction.h"

@implementation NSAccessibilityCustomAction

- (instancetype)initWithName: (NSString *)name
                     handler: (GSAccessibilityCustomActionHandler)handler
{
  return nil;
}

- (instancetype)initWithName: (NSString *)name
                      target: (id)target
                    selector: (SEL)selector
{
  return nil;
}

- (NSString *) name
{
  return nil;
}

- (void) setName: (NSString *)name
{
}
  
- (GSAccessibilityCustomActionHandler) handler
{
  return nil;
}

- (void) setHandler: (GSAccessibilityCustomActionHandler)handler
{
}

- (id) target
{
  return nil;
}

- (void) setTarget: (id)target
{
}

- (SEL) selector
{
  return NULL;
}

- (void) setSelector: (SEL)selector
{
}

@end

