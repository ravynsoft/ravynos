/* Interface for GSFileHandle for GNUStep
   Copyright (C) 1997-2002 Free Software Foundation, Inc.

   Written by:  Richard Frith-Macdonald <richard@brainstorm.co.uk>
   Date: 1997

   This file is part of the GNUstep Base Library.

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

#ifndef __GSFileHandle_h_GNUSTEP_BASE_INCLUDE
#define __GSFileHandle_h_GNUSTEP_BASE_INCLUDE

#import "Foundation/NSFileHandle.h"
#import "Foundation/NSArray.h"
#import "Foundation/NSDictionary.h"
#import "Foundation/NSRunLoop.h"


#if	USE_ZLIB
#include <zlib.h>
#endif

#ifdef __ANDROID__
#include <android/asset_manager_jni.h>
#endif

struct sockaddr_in;

/**
 * DO NOT USE ... this header is here only for the SSL file handle support
 * and is not intended to be used by anyone else ... it is subject to
 * change or removal without warning.
 */
@interface GSFileHandle : NSFileHandle <RunLoopEvents>
{
#if	GS_EXPOSE(GSFileHandle)
  int			descriptor;
  BOOL			closeOnDealloc;
  BOOL			isStandardFile;
  BOOL			isNullDevice;
  BOOL			isSocket;
  BOOL			isNonBlocking;
  BOOL			wasNonBlocking;
  BOOL			acceptOK;
  BOOL			connectOK;
  BOOL			readOK;
  BOOL			writeOK;
  NSMutableDictionary	*readInfo;
  int			readMax;
  NSMutableArray	*writeInfo;
  int			writePos;
  NSString		*address;
  NSString		*service;
  NSString		*protocol;
#if	USE_ZLIB
  gzFile		gzDescriptor;
#endif
#if	defined(_WIN32)
  WSAEVENT  		event;
#endif
#ifdef __ANDROID__
  AAsset		*asset;
#endif
#endif
}

- (id) initAsClientAtAddress: (NSString*)address
		     service: (NSString*)service
		    protocol: (NSString*)protocol;
- (id) initAsClientInBackgroundAtAddress: (NSString*)address
				 service: (NSString*)service
				protocol: (NSString*)protocol
				forModes: (NSArray*)modes;
- (id) initAsServerAtAddress: (NSString*)address
		     service: (NSString*)service
		    protocol: (NSString*)protocol;
- (id) initForReadingAtPath: (NSString*)path;
- (id) initForWritingAtPath: (NSString*)path;
- (id) initForUpdatingAtPath: (NSString*)path;
- (id) initWithStandardError;
- (id) initWithStandardInput;
- (id) initWithStandardOutput;
- (id) initWithNullDevice;

- (void) checkAccept;
- (void) checkConnect;
- (void) checkRead;
- (void) checkWrite;

- (void) ignoreReadDescriptor;
- (void) ignoreWriteDescriptor;
- (void) setNonBlocking: (BOOL)flag;
- (void) postReadNotification;
- (void) postWriteNotification;
- (NSInteger) read: (void*)buf length: (NSUInteger)len;
- (void) receivedEvent: (void*)data
		  type: (RunLoopEventType)type
	         extra: (void*)extra
	       forMode: (NSString*)mode;

- (void) setAddr: (struct sockaddr *)sin;

- (BOOL) useCompression;
- (void) watchReadDescriptorForModes: (NSArray*)modes;
- (void) watchWriteDescriptor;
- (NSInteger) write: (const void*)buf length: (NSUInteger)len;

@end

#endif /* __GSFileHandle_h_GNUSTEP_BASE_INCLUDE */
