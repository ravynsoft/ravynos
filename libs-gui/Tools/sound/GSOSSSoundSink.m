/* 
   AudioOutputSink.m

   Sink audio data to the Open Sound System

   Copyright (C) 2009 Free Software Foundation, Inc.

   Written by:  David Chisnall <theraven@sucs.org>
   Date: Jun 2009
   
   This file is part of the GNUstep GUI Library.

*/ 

#import <Foundation/Foundation.h>
#import <GNUstepGUI/GSSoundSource.h>
#import <GNUstepGUI/GSSoundSink.h>

#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/soundcard.h>


#if SOUND_VERSION >= 0x040000 
#define OSS_V4
#endif


@interface GSOSSSoundSink : NSObject <GSSoundSink>
{
	NSString *devicePath;
	int dev;
	int channels;
	int format;
	int rate;
}
@end

const static NSString *DefaultDevice = @"/dev/dsp";

@implementation GSOSSSoundSink
+ (BOOL)canInitWithPlaybackDevice: (NSString *)playbackDevice
{
	if (NULL == playbackDevice)
	{
		playbackDevice = DefaultDevice;
	}
	const char *device = [playbackDevice UTF8String];
	BOOL success = NO;
	int d = open(device, O_WRONLY, 0);
	if (-1 != d)
	{
		int ver;
		/* Check that this is an OSS device by trying an OSS ioctl on it */
		success = (-1 != ioctl(d, OSS_GETVERSION, &ver));
		close(d);
	}
	return success;
}
- (BOOL)configureDevice
{
	/* Close the device if it's open already. */
	[self close];

	/* Open the device */
	if (-1 == (dev = open([devicePath UTF8String], O_WRONLY, 0)))
	{
		return NO;
	}

	/* Set the number of channels */
	if (-1 == ioctl(dev, SNDCTL_DSP_CHANNELS, &channels))
	{
		[self close];
		return NO;
	}

	/* Set the sample format. */
	if (-1 == ioctl(dev, SNDCTL_DSP_SETFMT, &format))
	{
		[self close];
		return NO;
	}

	if (-1 == ioctl(dev, SNDCTL_DSP_SPEED, &rate))
	{
		[self close];
		return NO;
	}

	return YES;
}


- (id)initWithEncoding: (int)encoding
              channels: (NSUInteger)channelCount
            sampleRate: (NSUInteger)sampleRate
             byteOrder: (NSByteOrder)byteOrder
{
	if (nil == (self = [super init])) { return nil; }

	channels = channelCount;
	rate = sampleRate;

	switch (encoding)
	{
		case GSSoundFormatPCMS8:
			format = AFMT_S8;
			break;

		case GSSoundFormatPCM16:
			switch (byteOrder)
			{
				case NS_LittleEndian:
					format = AFMT_S16_LE;
					break;
				case NS_BigEndian:
					format = AFMT_S16_BE;
					break;
				default:
					format = AFMT_S16_NE;
			}
			break;

		case GSSoundFormatPCM24:
			switch (byteOrder)
			{
				case NS_LittleEndian:
					format = AFMT_S24_LE;
					break;
				case NS_BigEndian:
					format = AFMT_S24_BE;
					break;
				default:
					format = AFMT_S24_NE;
			}
			break;

		case GSSoundFormatPCM32:
			switch (byteOrder)
			{
				case NS_LittleEndian:
					format = AFMT_S32_LE;
					break;
				case NS_BigEndian:
					format = AFMT_S32_BE;
					break;
				default:
					format = AFMT_S32_NE;
			}
			break;

		case GSSoundFormatFloat32: 
/* Some OSS implementations (e.g. FreeBSD) don't support AFMT_FLOAT)
 * Fall through to unsupported formats if this is one of them.
 */
#ifdef AFMT_FLOAT 
			format = AFMT_FLOAT;
			break;
#endif

		/* Does this even exist? */
		case GSSoundFormatFloat64:
		default:
			[self release];
			return nil;
	}
  
	/* Try to initialise this device */
	if (![self configureDevice])
	{
		[self release];
		return nil;
	}
  
  return self;
}

- (BOOL)open
{
  return [self configureDevice];
}

- (void)close
{
	if (-1 != dev)
	{
		close(dev);
		dev = -1;
	}
}

- (BOOL)playBytes: (void*)bytes length: (NSUInteger)length
{
	do 
	{
		int written = write(dev, bytes, (size_t)length);
		if (-1 == written)
		{
			return NO;
		}
		length -= written;
		bytes += written;
	} while (length > 0);
	return YES;
}

- (void)setVolume: (float)volume
{
#ifdef OSS_V4
	char channelVolue = volume * 255;
	/* OSS uses one byte for left and one byte for right volume */
	int vol = (channelVolue << 8) + channelVolue;
	ioctl(dev, SNDCTL_DSP_SETPLAYVOL, &vol);
#endif
}

- (float)volume
{
#ifdef OSS_V4
	int vol;
	if (-1 == ioctl(dev, SNDCTL_DSP_SETPLAYVOL, &vol))
	{
		return 0;
	}
	/* Mask off the low 8 bits and scale back */
	return ((float)(vol & 255) ) / 255;
#else
	return 1;
#endif
}

- (void)setPlaybackDeviceIdentifier: (NSString*)playbackDeviceIdentifier
{
	ASSIGN(devicePath, playbackDeviceIdentifier);
	[self configureDevice];
}

- (NSString*)playbackDeviceIdentifier
{
	return devicePath;
}
/* Not implemented */
- (void)setChannelMapping: (NSArray*)channelMapping { return; }
- (NSArray*)channelMapping { return nil; }

@end

