/* 
   NSSound.h

   Load, manipulate and play sounds

   Copyright (C) 2002, 2009 Free Software Foundation, Inc.

   Written by:  Enrico Sersale <enrico@imago.ro>,
                  Stefan Bidigaray <stefanbidi@gmail.com>
   Date: Jul 2002, Jun 2009
   
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

#ifndef _GNUstep_H_NSSound
#define _GNUstep_H_NSSound

#import <Foundation/NSObject.h>
#import <Foundation/NSBundle.h>

#import "GNUstepGUI/GSSoundSource.h"
#import "GNUstepGUI/GSSoundSink.h"

@class NSArray;
@class NSData;
@class NSMutableData;
@class NSPasteboard;
@class NSString;
@class NSURL;
@class NSThread;
@class NSConditionLock;
@class NSLock;

/** Function used to retrieve all available playback devices.
 *  <p>This function is the only way to retrieve possible playback
 *  device identifiers understood by
 *  [NSSound -setPlaybackDeviceIdentifier:].</p>
 */
NSArray *PlaybackDeviceIdentifiers (void);

@interface NSSound : NSObject <NSCoding, NSCopying>
{		
  NSString *_name;
  NSData   *_data;
  NSString *_playbackDeviceIdentifier; // Currently unused
  NSArray  *_channelMapping; // Currently unused
  BOOL     _onlyReference;
  id       _delegate;
  
  id<GSSoundSource> _source;
  id<GSSoundSink>   _sink;
  NSConditionLock   *_readLock;
  NSLock            * _playbackLock;
  BOOL _shouldStop;
  BOOL _shouldLoop;
}

//
// Creating an NSSound 
//
/** <init/>
 *  Initalizes the receiver object with the contents of file located at path.
 *  If byRef is set to YES only the name of the NSSound is encoded with
 *  -encodeWithCoder:; if set to NO the data is encoded.
 *  <p>See Also:</p>
 *  <list>
 *    <item>-initWithContentsOfURL:byReference:</item>
 *    <item>-initWithData:</item>
 *  </list>
 */
- (id)initWithContentsOfFile:(NSString *)path byReference:(BOOL)byRef;
/** <init/>
 *  Initializes the receiver object with the contents of the data located in
 *  url.  If byRef is set to YES only the name of the NSSound is encoded with
 *  -encodeWithCoder:;  if set to NO the data is encoded.
 *  <p>See Also:</p>
 *  <list>
 *    <item>-initWithContentsOfFile:byReference:</item>
 *    <item>-initWithData:</item>
 *  </list>
 */
- (id)initWithContentsOfURL:(NSURL *)url byReference:(BOOL)byRef;
/** <init/>
 *  Initializes the receiver object with the contents of data with a
 *  valid magic number.
 *  <p>See Also:</p>
 *  <list>
 *    <item>-initWithContentsOfFile:byReference:</item>
 *    <item>-initWithContentsOfURL:byReference:</item>
 *  </list>
 */
- (id)initWithData:(NSData *)data;
- (id)initWithPasteboard:(NSPasteboard *)pasteboard;

//
// Playing
//
/** Pauses audio playback.
 *  <p>Returns NO if receiver could not be paused or is already paused, 
 *  and YES if receiver was successfully paused.</p>
 */
- (BOOL)pause;
/** Start audio playback.  Playback is done asynchronously.
 *  <p>Returns NO if receiver is already playing or if an error occurred, and
 *  YES if receiver was started successfully.</p>
 */
- (BOOL)play;
/** Resume audio playback after a -pause.
 *  <p>Returns NO if receiver is already playing or if an error occurred, and
 *  YES if receiver was successfully restarted/resumed.</p>
 */
- (BOOL)resume;
/** Stop audio playback.
 *  <p>Return YES if receiver was successfully stopped.</p>
 *  <p>This method will close the playback device.</p>
 */
- (BOOL)stop;
/* Returns YES if receiver is playing and NO otherwise.
 */
- (BOOL)isPlaying;

//
// Working with pasteboards 
//
+ (BOOL)canInitWithPasteboard:(NSPasteboard *)pasteboard;
+ (NSArray *)soundUnfilteredPasteboardTypes;
- (void)writeToPasteboard:(NSPasteboard *)pasteboard;

//
// Working with delegates 
//
/** Returns the receiver's delegate
 */
- (id)delegate;
/** Sets the receiver's delegate
 */
- (void)setDelegate:(id)aDelegate;

//
// Sound Information
//
+ (id)soundNamed:(NSString *)name;
/** Provides an array of file types that NSSound can understand.  The returning
 *  array may be directly passed to [NSOpenPanel -runModalForTypes:].
 *  <p>Built with libsndfile:</p>
 *    wav, aiff, aifc, aif, au, snd, mat, mat4, mat5, paf, sf, voc, w64,
 *    xi, caf, sd2, flac, ogg, oga
 *  <p>Built without libsndfile:</p>
 *    wav, aiff, aifc, aif, au, snd
 */
+ (NSArray *)soundUnfilteredFileTypes;
/** Returns the name of the receiver.  Use -setName: to set the name.
 */
- (NSString *)name;
/** Sets the receiver's name.  Method -name will return aName.
 */
- (BOOL)setName:(NSString *)aName;

#if OS_API_VERSION(MAC_OS_X_VERSION_10_5, GS_API_LATEST)
+ (NSArray *)soundUnfilteredTypes;
/** Returns the length, in seconds, of the receiver.
 */
- (NSTimeInterval)duration;
/** Returns the volume of the receiver.
 *  Volume will be between 0.0 and 1.0.
 */
- (float)volume;
/** Sets the volume of the receiver.
 *  Volume must be between 0.0 and 1.0.
 */
- (void)setVolume: (float)volume;
/** Returns the current position of the audio playback.
 */
- (NSTimeInterval)currentTime;
/** Sets the current time of the audio playback.
 */
- (void)setCurrentTime: (NSTimeInterval)currentTime;
/** Returns the current loop property of the receiver.
 *  YES indicates this NSSound will restart once it reaches the end,
 *  otherwise NO.
 */
- (BOOL)loops;
/** Sets the loop property of the receiver.
 *  YES indicates this NSSound will restart once it reaches the end,
 *  otherwise NO.
 */
- (void)setLoops: (BOOL)loops;

- (NSString *)playbackDeviceIdentifier;
- (void)setPlaybackDeviceIdentifier: (NSString *)playbackDeviceIdentifier;
- (NSArray *)channelMapping;
- (void)setChannelMapping: (NSArray *)channelMapping;
#endif

@end

//
// Methods Implemented by the Delegate 
//
@interface NSObject (NSSoundDelegate)

/** Method called when sound has finished playing.  Currently this method
 *  is not called.
 */
- (void)sound:(NSSound *)sound didFinishPlaying:(BOOL)aBool;

@end


@interface NSBundle (NSSoundAdditions)

- (NSString *)pathForSoundResource:(NSString *)name;

@end

#endif // _GNUstep_H_NSSound

