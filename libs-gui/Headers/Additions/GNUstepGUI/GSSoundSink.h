/* 
   GSSoundSink.h

   Sink audio data.

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

#ifndef _GNUstep_H_GSSoundSink
#define _GNUstep_H_GSSoundSink

#import <Foundation/NSByteOrder.h>
#import <Foundation/NSObject.h>

@protocol GSSoundSink <NSObject>

/** Returns YES if class has the ability of playing audio data
 *  through a playback device playbackDevice, NO otherwise.
 */
+ (BOOL)canInitWithPlaybackDevice: (NSString *)playbackDevice;

/** <init/>
 *  Initializes the receiver for output using the defined parameters.
 *  <p><b>WARNING:</b> This method does not open the device, see -open.</p>
 */
- (id)initWithEncoding: (int)encoding
              channels: (NSUInteger)channelCount
            sampleRate: (NSUInteger)sampleRate
             byteOrder: (NSByteOrder)byteOrder;
/** Opens the device for output, called by [NSSound-play].
 */
- (BOOL)open;
/** Closes the device, called by [NSSound-stop].
 */
- (void)close;
/** Plays the data in bytes to the device.  Data <i>must</i> be in 
 *  the same format as specified in
 *  -initWithEncoding:channels:sampleRate:byteOrder:.
 */
- (BOOL)playBytes: (void *)bytes length: (NSUInteger)length;

/** Called by [NSSound-setVolume:], and corresponds to it.  Parameter volume
 *  is between the values 0.0 and 1.0.
 */
- (void)setVolume: (float)volume;
/** Called by [NSSound-volume].
 */
- (float)volume;
/** Called by [NSSound-setPlaybackDeviceIdentifier:].
 */
- (void)setPlaybackDeviceIdentifier: (NSString *)playbackDeviceIdentifier;
/** Called by [NSSound-playbackDeviceIdentifier].
 */
- (NSString *)playbackDeviceIdentifier;
/** Called by [NSSound-setChannelMapping:].
 */
- (void)setChannelMapping: (NSArray *)channelMapping;
/** Called by [NSSound-channelMapping].
 */
- (NSArray *)channelMapping;

@end

#endif // _GNUstep_H_GSSoundSink

