/* Implementation of class GSSpeechRecognitionEngine
   Copyright (C) 2019 Free Software Foundation, Inc.
   
   By: Gregory John Casamento
   Date: Fri Dec  6 04:55:59 EST 2019

   This file is part of the GNUstep Library.
   
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
   
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.
   
   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free
   Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110 USA.
*/

#import "GSSpeechRecognitionEngine.h"

/**
 * Dummy implementation of a speech engine.  Doesn't do anything.
 */
@implementation GSSpeechRecognitionEngine

+ (GSSpeechRecognitionEngine*) defaultSpeechEngine
{
  return AUTORELEASE([[self alloc] init]);
} 

- (void) recognize
{
}

- (void) start
{
}

- (void) stop
{
}

@end
