/* 
   SndfileSource.m

   Load and read sound data using libsndfile.

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
#include "GNUstepGUI/GSSoundSource.h"
#include <sndfile.h>

@interface SndfileSource : NSObject <GSSoundSource>
{
  NSData *_data;
  SNDFILE *_snd;
  SF_INFO _info;
  
  NSUInteger _curPos;
  NSTimeInterval _dur;
  int _encoding;
}

- (NSData *)data;
- (NSUInteger)currentPosition;
- (void)setCurrentPosition: (NSUInteger)curPos;
@end

/**********************************/
/* Sndfile virtual I/O functions. */
/**********************************/
static inline sf_count_t dataLength (void *user_data)
{
  SndfileSource *snd = (SndfileSource *)user_data;
  
  return (sf_count_t)[[snd data] length];
}

static inline sf_count_t dataSeek (sf_count_t offset, int whence,
                           void *user_data)
{
  SndfileSource *snd = (SndfileSource *)user_data;
  
  switch (whence)
    {
      case SEEK_SET:
        break;
      case SEEK_END:
        offset = (sf_count_t)[[snd data] length] + offset;
        break;
      case SEEK_CUR:
        offset = (sf_count_t)[snd currentPosition] + offset;
        break;
      default:
        return 0;
    }
  [snd setCurrentPosition: (NSUInteger)offset];
  return (sf_count_t)[snd currentPosition];
}

static inline sf_count_t dataRead (void *ptr, sf_count_t count,
                           void *user_data)
{
  NSUInteger newPos;
  SndfileSource *snd = (SndfileSource *)user_data;
  
  // Can't read more data that we have available...
  if (([snd currentPosition] + (NSUInteger)count) > [[snd data] length])
    {
      count = (sf_count_t)([[snd data] length] - [snd currentPosition]);
    }
  
  newPos = [snd currentPosition] + (NSUInteger)count;
  [[snd data] getBytes: ptr
                 range: NSMakeRange ([snd currentPosition], count)];
  [snd setCurrentPosition: newPos];
  
  return count;
}

static inline sf_count_t dataWrite (const void *ptr, sf_count_t count,
                           void *user_data)
{
  /* FIXME: No write support... do we even need it? */
  return 0;
}

static inline sf_count_t dataTell (void *user_data)
{
  SndfileSource *snd = (SndfileSource *)user_data;
  return (sf_count_t)[snd currentPosition];
}

// The libsndfile virtual I/O function structure
static SF_VIRTUAL_IO dataIO = { (sf_vio_get_filelen)dataLength,
                                (sf_vio_seek)dataSeek,
                                (sf_vio_read)dataRead,
                                (sf_vio_write)dataWrite,
                                (sf_vio_tell)dataTell };
/**********************************/

@implementation SndfileSource

+ (NSArray *)soundUnfilteredFileTypes
{
  return [NSArray arrayWithObjects: @"wav", @"au", @"snd", @"aif", @"aiff",
           @"aifc", @"paf", @"sf", @"voc", @"w64", @"mat", @"mat4", @"mat5",
           @"pcf", @"xi", @"caf", @"sd2", @"iff", @"flac", @"ogg", @"oga",
           nil];
}
+ (NSArray *)soundUnfilteredTypes
{
  /* FIXME: I'm not sure what the UTI for all the types above are. */
  return [NSArray arrayWithObjects: @"com.microsoft.waveform-audio",
           @"public.ulaw-audio", @"public.aiff-audio", @"public.aifc-audio",
           @"com.apple.coreaudio-format", @"com.digidesign.sd2-audio",
           /* FIXME: are these right? */
           @"org.xiph.flac-audio", @"org.xiph.vorbis-audio", nil];
}
+ (BOOL)canInitWithData: (NSData *)data
{
  return YES;
}

- (void)dealloc
{
  TEST_RELEASE (_data);
  sf_close (_snd);
  
  [super dealloc];
}

- (id)initWithData: (NSData *)data
{
  self = [super init];
  if (self == nil)
    {
      return nil;
    }
  
  _data = data;
  RETAIN(_data);
  
  _info.format = 0;
  _snd = sf_open_virtual (&dataIO, SFM_READ, &_info, self);
  if (_snd == NULL)
    {
      DESTROY(self);
      return nil;
    }
  
  // Setup immutable values...
  /* FIXME: support multiple types */
  _encoding = GSSoundFormatPCM16;
  _dur = (double)_info.frames / (double)_info.samplerate;
  
  return self;
}

- (NSUInteger)readBytes: (void *)bytes length: (NSUInteger)length
{
  return (NSUInteger) (sf_read_short (_snd, bytes, (length>>1))<<1);
}

- (NSTimeInterval)duration
{
  return _dur;
}

- (void)setCurrentTime: (NSTimeInterval)currentTime
{
  sf_count_t frames = (sf_count_t)((double)_info.samplerate * currentTime);
  sf_seek (_snd, frames, SEEK_SET);
}
- (NSTimeInterval)currentTime
{
  sf_count_t frames;
  frames = sf_seek (_snd, 0, SEEK_CUR);
  return (NSTimeInterval)((double)frames / (double)_info.samplerate);
}

- (int)encoding
{
  return _encoding;
}

- (NSUInteger)channelCount
{
  return (NSUInteger)_info.channels;
}

- (NSUInteger)sampleRate;
{
  return (NSUInteger)_info.samplerate;
}

- (NSByteOrder)byteOrder
{
  // Equivalent to sending native byte order...
  // Sndfile always reads as native format.
  return NS_UnknownByteOrder;
}

- (NSData *)data
{
  return _data;
}

- (NSUInteger)currentPosition
{
  return _curPos;
}

- (void)setCurrentPosition: (NSUInteger)curPos
{
  _curPos = curPos;
}

@end
