/* 
   AudioOutputSink.m

   Sink audio data to libao.

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

#include <Foundation/Foundation.h>
#include <GNUstepGUI/GSSoundSource.h>
#include <GNUstepGUI/GSSoundSink.h>
#include <ao/ao.h>

@interface AudioOutputSink : NSObject <GSSoundSink>
{
  ao_device *_dev;
  int _driver;
  ao_sample_format _format;
}
@end

@implementation AudioOutputSink

+ (void) initialize
{
  /* FIXME: According to the docs, this needs a corresponding ao_shutdown(). */
  ao_initialize ();
}

+ (BOOL)canInitWithPlaybackDevice: (NSString *)playbackDevice
{
  // This is currently the only sink in NSSound, just say
  // YES to everything.
  /* FIXME: What is OS X's identifier for the main sound? */
  return (playbackDevice == nil ? YES : NO);
}

- (void)dealloc
{
  [super dealloc];
}

- (id)initWithEncoding: (int)encoding
              channels: (NSUInteger)channelCount
            sampleRate: (NSUInteger)sampleRate
             byteOrder: (NSByteOrder)byteOrder
{
  self = [super init];
  if (self == nil)
    {
      return nil;
    }
  
  _format.channels = (int)channelCount;
  _format.rate = (int)sampleRate;
  
  switch (encoding)
    {
      case GSSoundFormatPCMS8:
        _format.bits = 8;
        break;
      
      case GSSoundFormatPCM16:
        _format.bits = 16;
        break;
      
      case GSSoundFormatPCM24:
        _format.bits = 24;
        break;
      
      case GSSoundFormatPCM32:
        _format.bits = 32;
        break;
      
      case GSSoundFormatFloat32: // Float and double not supported by libao.
      case GSSoundFormatFloat64:
      default:
        DESTROY(self);
        return nil;
    }
  
	if (byteOrder == NS_LittleEndian)
	  {
	    _format.byte_format = AO_FMT_LITTLE;
    }
  else if (byteOrder == NS_BigEndian)
    {
      _format.byte_format = AO_FMT_BIG;
    }
  else
    {
      _format.byte_format = AO_FMT_NATIVE;
    }

  return self;
}

- (BOOL)open
{
  _driver = ao_default_driver_id();
  
  _dev = ao_open_live(_driver, &_format, NULL);
  return ((_dev == NULL) ? NO : YES);
}

- (void)close
{
  ao_close(_dev);
}

- (BOOL)playBytes: (void *)bytes length: (NSUInteger)length
{
  int ret = ao_play(_dev, bytes, (uint_32)length);
  return (ret == 0 ? NO : YES);
}

/* Functionality not supported by libao */
- (void)setVolume: (float)volume
{
  return;
}

- (float)volume
{
  return 1.0;
}

- (void)setPlaybackDeviceIdentifier: (NSString *)playbackDeviceIdentifier
{
  return;
}

- (NSString *)playbackDeviceIdentifier
{
  return nil;
}

- (void)setChannelMapping: (NSArray *)channelMapping
{
  return;
}

- (NSArray *)channelMapping
{
  return nil;
}

@end

