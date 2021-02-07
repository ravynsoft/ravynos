/* 
   GSSoundSource.h

   Load and read sound data.

   Copyright (C) 2009 Free Software Foundation, Inc.

   Written by:  Stefan Bidigaray <stefanbidi@gmail.com>
   Date: Jun 2009
   
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

#ifndef _GNUstep_H_GSSoundSource
#define _GNUstep_H_GSSoundSource

#import <Foundation/NSByteOrder.h>
#import <Foundation/NSObject.h>

@class NSArray;

enum
{
  GSSoundFormatUnknown = 0x0000,
  GSSoundFormatPCMS8   = 0x0001,
  GSSoundFormatPCM16   = 0x0002,
  GSSoundFormatPCM24   = 0x0003,
  GSSoundFormatPCM32   = 0x0004,
  GSSoundFormatPCMU8   = 0x0005,
  GSSoundFormatFloat32 = 0x0006,
  GSSoundFormatFloat64 = 0x0007,
  
  GSSoundFormatULaw    = 0x0010,
  GSSoundFormatALaw    = 0x0011
};

@protocol GSSoundSource <NSObject>

/** Returns an array of the file types supported by the class.
 */
+ (NSArray *)soundUnfilteredFileTypes;
/** Returns an array of UTIs identifying the file types the class understands.
 */
+ (NSArray *)soundUnfilteredTypes;
/** Returns YES if the class can understand data and NO otherwise.
 */
+ (BOOL)canInitWithData: (NSData *)data;

/** <init />
 *  Initilizes the reciever for output.
 */
- (id)initWithData: (NSData *)data;
/** Reads data provided in -initWithData:.  Parameter bytes must be big enough
 *  to hold length bytes.
 */
- (NSUInteger)readBytes: (void *)bytes length: (NSUInteger)length;

/** Returns the duration, in seconds.  Equivalent to [NSSound-duration].
 */
- (NSTimeInterval)duration;
/** Called by [NSSound-setCurrentTime:].
 */
- (void)setCurrentTime: (NSTimeInterval)currentTime;
/** Called by [NSSound-currentTime].
 */
- (NSTimeInterval)currentTime;

/** Returns encoding of the audio data.
 */
- (int)encoding;
/** Returns the number of channels.
 */
- (NSUInteger)channelCount;
/** Returns the receiver's sample rate (ie 44100, 8000, etc).
 */
- (NSUInteger)sampleRate;
/** Returns the byte order of the audio data.
 */
- (NSByteOrder)byteOrder;

@end

#endif // _GNUstep_H_GSSoundSource
