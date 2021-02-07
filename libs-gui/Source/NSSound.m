/** <title>NSSound</title>

   <abstract>Load, manipulate and play sounds</abstract>

   Copyright (C) 2002, 2009 Free Software Foundation, Inc.
   
   Author: Enrico Sersale <enrico@imago.ro>,
             Stefan Bidigaray <stefanbidi@gmail.com>
   Date: Jul 2002, Jul 2009

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

#import <Foundation/Foundation.h>
#import "AppKit/NSPasteboard.h"
#import "AppKit/NSSound.h"

#import "GNUstepGUI/GSSoundSource.h"
#import "GNUstepGUI/GSSoundSink.h"

// Private NSConditionLock conditions used for streaming
enum
{
  SOUND_SHOULD_PLAY = 1,
  SOUND_SHOULD_PAUSE
};

#define BUFFER_SIZE 4096

/* Class variables and functions for class methods */
static NSMutableDictionary *nameDict = nil;
static NSDictionary *nsmapping = nil;
static NSArray *sourcePlugIns = nil;
static NSArray *sinkPlugIns = nil;

static inline void _loadNSSoundPlugIns (void)
{
  NSString      *path;
  NSArray       *paths;
  NSBundle      *bundle;
  NSEnumerator  *enumerator;
  NSMutableArray *all,
                 *_sourcePlugIns,
                 *_sinkPlugIns;
  Class plugInClass;

  /* Gather up the paths */
  paths = NSSearchPathForDirectoriesInDomains(NSLibraryDirectory,
            NSAllDomainsMask, YES);

  enumerator = [paths objectEnumerator];
  all = [NSMutableArray array];
  while ((path = [enumerator nextObject]) != nil)
   {
     bundle = [NSBundle bundleWithPath: path];
     paths = [bundle pathsForResourcesOfType: @"nssound"
                                 inDirectory: @"Bundles"];
     [all addObjectsFromArray: paths];
   }
  
  enumerator = [all objectEnumerator];
  _sourcePlugIns = [NSMutableArray array];
  _sinkPlugIns = [NSMutableArray array];
  while ((path = [enumerator nextObject]) != nil)
    {
      NSBundle *nssoundBundle = [NSBundle bundleWithPath: path];
      plugInClass = [nssoundBundle principalClass];
      if ([plugInClass conformsToProtocol: @protocol(GSSoundSource)])
        {
          [_sourcePlugIns addObject:plugInClass];
        }
      else if ([plugInClass conformsToProtocol: @protocol(GSSoundSink)])
        {
          [_sinkPlugIns addObject:plugInClass];
        }
      else
        {
          NSLog (@"Bundle %@ does not conform to GSSoundSource or GSSoundSink",
            path);
        }
    }
  
  sourcePlugIns = [[NSArray alloc] initWithArray: _sourcePlugIns];
  sinkPlugIns = [[NSArray alloc] initWithArray: _sinkPlugIns];
}



@implementation NSBundle (NSSoundAdditions)

- (NSString *) pathForSoundResource: (NSString *)name
{
  NSString *ext = [name pathExtension];
  NSString *path = nil;

  if ((ext == nil) || [ext isEqualToString:@""])
    {
      NSArray	*types = [NSSound soundUnfilteredFileTypes];
      unsigned	c = [types count];
      unsigned	i;

      for (i = 0; path == nil && i < c; i++)
	      {
	        ext = [types objectAtIndex: i];
	        path = [self pathForResource: name ofType: ext];
	      }
    }
  else
    {
      name = [name stringByDeletingPathExtension];
      path = [self pathForResource: name ofType: ext];
    }
  return path;
}

@end 

@interface NSSound (PrivateMethods)

- (void)_stream;
- (void)_finished: (NSNumber *)finishedPlaying;

@end

@implementation NSSound (PrivateMethods)

- (void)_stream
{
  NSUInteger bytesRead;
  BOOL success = NO;
  void *buffer;
  
  // Exit with success = NO if device could not be open.
  if ([_sink open])
    {
      // Allocate space for buffer and start writing.
      buffer = NSZoneMalloc(NSDefaultMallocZone(), BUFFER_SIZE);
      do
        {
          do
            {
              // If not SOUND_SHOULD_PLAY block thread
              [_readLock lockWhenCondition: SOUND_SHOULD_PLAY];
              if (_shouldStop)
                {
                  [_readLock unlock];
                  break;
                }
              bytesRead = [_source readBytes: buffer
                                     length: BUFFER_SIZE];
              [_readLock unlock];
              [_playbackLock lock];
              success = [_sink playBytes: buffer length: bytesRead];
              [_playbackLock unlock];
            } while ((!_shouldStop) && (bytesRead > 0) && success);
          
          [_source setCurrentTime: 0.0];
        } while (_shouldLoop == YES && _shouldStop == NO);
      
      [_sink close];
      NSZoneFree (NSDefaultMallocZone(), buffer);
    }
  
  RETAIN(self);
  [self performSelectorOnMainThread: @selector(_finished:)
                         withObject: [NSNumber numberWithBool: success]
                      waitUntilDone: YES];
  RELEASE(self);
}

- (void)_finished: (NSNumber *)finishedPlaying
{
  DESTROY(_readLock);
  DESTROY(_playbackLock);
  
  /* FIXME: should I call -sound:didFinishPlaying: when -stop was sent? */
  if ([_delegate respondsToSelector: @selector(sound:didFinishPlaying:)])
    {
      [_delegate sound: self didFinishPlaying: [finishedPlaying boolValue]];
    }
}

@end

@implementation	NSSound

+ (void) initialize
{
  if (self == [NSSound class])
    {
      NSString *path = [NSBundle pathForLibraryResource: @"nsmapping"
                                                 ofType: @"strings"
                                            inDirectory: @"Sounds"];
      [self setVersion: 2];

      nameDict = [[NSMutableDictionary alloc] initWithCapacity: 10];
      
      if (path)
        {
          nsmapping = RETAIN([[NSString stringWithContentsOfFile: path]
                  propertyListFromStringsFileFormat]);
        }
      
      /* FIXME: Not sure if this is the best way... */
      _loadNSSoundPlugIns ();
    }
}

- (void) dealloc
{
  // Make sure sound is stopped before deallocating.
  [self stop];
  
  RELEASE (_data);
  if (self == [nameDict objectForKey: _name])
    {
      [nameDict removeObjectForKey: _name];
    }
  RELEASE (_name);
  RELEASE (_playbackDeviceIdentifier);
  RELEASE (_channelMapping);
  RELEASE (_source);
  RELEASE (_sink);
  
  [super dealloc];
}

//
// Creating an NSSound 
//
- (id) initWithContentsOfFile: (NSString *)path byReference:(BOOL)byRef
{
  NSData *fileData;
  
  // Problem here: should every NSSound instance have a _name set?
  // The Apple docs are a bit confusing here.  For now, the only way
  // _name will be set is if -setName: is called, or if the sound already
  // exists in on of the Sounds/ directories.
  _onlyReference = byRef;

  
  fileData = [NSData dataWithContentsOfMappedFile: path];
  if (!fileData)
    {
      NSLog (@"Could not get sound data from: %@", path);
      DESTROY(self);
      return nil;
    }
  
  return [self initWithData: fileData];
}

- (id) initWithContentsOfURL: (NSURL *)url byReference:(BOOL)byRef
{
  _onlyReference = byRef;	
  return [self initWithData: [NSData dataWithContentsOfURL: url]];
}

- (id) initWithData: (NSData *)data
{
  NSEnumerator *enumerator;
  Class sourceClass,
        sinkClass;
    
  _data = data;
  RETAIN(_data);
  
  // Search for an GSSoundSource bundle that can play this data.
  enumerator = [sourcePlugIns objectEnumerator];
  while ((sourceClass = [enumerator nextObject]) != nil)
    {
      if ([sourceClass canInitWithData: _data])
        {
          _source = [[sourceClass alloc] initWithData: _data];
          if (_source == nil)
            {
              NSLog (@"Could not read sound data!");
              DESTROY(self);
              return nil;
            }
          break;
        }
    }
  
  enumerator = [sinkPlugIns objectEnumerator];
  /* FIXME: Grab the first available sink/device for now.  In the future
       look for what is set in the GSSoundDeviceBundle default first. */
  while ((sinkClass = [enumerator nextObject]) != nil)
    {
      if ([sinkClass canInitWithPlaybackDevice: nil])
        {
          _sink = [[sinkClass alloc] initWithEncoding: [_source encoding]
                                             channels: [_source channelCount]
                                           sampleRate: [_source sampleRate]
                                            byteOrder: [_source byteOrder]];
          if (_sink == nil)
            {
              NSLog (@"Could not open sound sink!");
              DESTROY(self);
              return nil;
            }
          break;
        }
    }
  
  /* FIXME: There has to be a better way to do this check??? */
  if (sourceClass == nil || sinkClass == nil)
    {
      NSLog (@"Could not find suitable sound plug-in");
      DESTROY(self);
      return nil;
    }
  
  return self;
}

- (id) initWithPasteboard: (NSPasteboard *)pasteboard
{
  if ([object_getClass(self) canInitWithPasteboard: pasteboard] == YES)
    {
      /* FIXME: Should this be @"NSGeneralPboardType" or @"NSSoundPboardType"?
           Apple also defines "NSString *NSSoundPboardType". */
      NSData *d = [pasteboard dataForType: @"NSGeneralPboardType"];	
      return [self initWithData: d];	
    }
  return nil;
}

//
// Playing and Information
//
- (BOOL) pause 
{
  // Do nothing if sound is already paused.
  if ([_readLock condition] == SOUND_SHOULD_PAUSE)
    {
      return NO;
    }
  
  if ([_readLock tryLock] == NO)
    {
      return NO;
    }
  [_readLock unlockWithCondition: SOUND_SHOULD_PAUSE];
  return YES;
}

- (BOOL) play
{
  // If the locks exists this instance is already playing
  if (_readLock != nil && _playbackLock != nil)
    {
      return NO;
    }
  
  _readLock = [[NSConditionLock alloc] initWithCondition: SOUND_SHOULD_PAUSE];
  _playbackLock = [[NSLock alloc] init];

  if ([_readLock tryLock] != YES)
    {
      return NO;
    }
  _shouldStop = NO;
  [NSThread detachNewThreadSelector: @selector(_stream)
                           toTarget: self
                         withObject: nil];
  [_readLock unlockWithCondition: SOUND_SHOULD_PLAY];
  
  return YES;
}

- (BOOL) resume
{
  // Do nothing if sound is already playing.
  if ([_readLock condition] == SOUND_SHOULD_PLAY)
    {
      return NO;
    }
  
  if ([_readLock tryLock] == NO)
    {
      return NO;
    }
  [_readLock unlockWithCondition: SOUND_SHOULD_PLAY];
  return YES;
}

- (BOOL) stop
{
  if (_readLock == nil)
    {
      return NO;
    }
  
  if ([_readLock tryLock] != YES)
    {
      return NO;
    }
  _shouldStop = YES;
  // Set to SOUND_SHOULD_PLAY so that thread isn't blocked.
  [_readLock unlockWithCondition: SOUND_SHOULD_PLAY];
  
  return YES;
}

- (BOOL) isPlaying
{
  if (_readLock == nil)
    {
      return NO;
    }
  if ([_readLock condition] == SOUND_SHOULD_PLAY)
    {
      return YES;
    }
  return NO;
}

- (float) volume
{
  return [_sink volume];
}

- (void) setVolume: (float) volume
{
  [_playbackLock lock];
  [_sink setVolume: volume];
  [_playbackLock unlock];
}

- (NSTimeInterval) currentTime
{
  return [_source currentTime];
}

- (void) setCurrentTime: (NSTimeInterval) currentTime
{
  [_readLock lock];
  [_source setCurrentTime: currentTime];
  [_readLock unlock];
}

- (BOOL) loops
{
  return _shouldLoop;
}

- (void) setLoops: (BOOL) loops
{
  _shouldLoop = loops;
}

- (NSTimeInterval) duration
{
  return [_source duration];
}

//
// Working with pasteboards 
//
+ (BOOL) canInitWithPasteboard: (NSPasteboard *)pasteboard
{
  NSArray *pbTypes = [pasteboard types];
  NSArray *myTypes = [NSSound soundUnfilteredPasteboardTypes];

  return ([pbTypes firstObjectCommonWithArray: myTypes] != nil);
}

+ (NSArray *) soundUnfilteredPasteboardTypes
{
  return [NSArray arrayWithObjects: @"NSGeneralPboardType", nil];
}

- (void) writeToPasteboard: (NSPasteboard *)pasteboard
{
  NSData *d = [NSArchiver archivedDataWithRootObject: self];

  if (d != nil) {
    [pasteboard declareTypes: [NSSound soundUnfilteredPasteboardTypes] 
		owner: nil];
    [pasteboard setData: d forType: @"NSGeneralPboardType"];
  }
}

//
// Working with delegates 
//
- (id) delegate
{
  return _delegate;
}

- (void) setDelegate: (id)aDelegate
{
  _delegate = aDelegate;
}

//
// Naming Sounds 
//
+ (id) soundNamed: (NSString*)name
{
  NSString	*realName = [nsmapping objectForKey: name];
  NSSound	*sound;

  if (realName)
    {
      name = realName;
    }
	
  sound = (NSSound *)[nameDict objectForKey: name];
 
  if (sound == nil)
    {
      NSString	*extension;
      NSString	*path = nil;
      NSBundle	*main_bundle;
      NSArray	*array;
      NSString	*the_name = name;

      // FIXME: This should use [NSBundle pathForSoundResource], but this will 
      // only allow soundUnfilteredFileTypes.
      /* If there is no sound with that name, search in the main bundle */
			
      main_bundle = [NSBundle mainBundle];
      extension = [name pathExtension];
		
      if (extension != nil && [extension length] == 0)
	{
	  extension = nil;
	}

      /* Check if extension is one of the sound types */
      array = [NSSound soundUnfilteredFileTypes];
	
      if ([array indexOfObject: extension] != NSNotFound)
	{
	  /* Extension is one of the sound types
	     So remove from the name */
	  the_name = [name stringByDeletingPathExtension];
	} 
      else 
	{
	  /* Otherwise extension is not an sound type
	     So leave it alone */
	  the_name = name;
	  extension = nil;
	}

      /* First search locally */
      if (extension)
	{
	  path = [main_bundle pathForResource: the_name ofType: extension];
	} 
      else 
	{
	  id o, e;

	  e = [array objectEnumerator];
	  while ((o = [e nextObject]))
	    {
	      path = [main_bundle pathForResource: the_name ofType: o];
	      if (path != nil && [path length] != 0)
		{
		  break;
		}
	    }
	}

      /* If not found then search in system */
      if (!path)
	{
	  if (extension)
	    {
	      path = [NSBundle pathForLibraryResource: the_name
				               ofType: extension
				          inDirectory: @"Sounds"];
	    } 
	  else 
	    {
	      id o, e;

	      e = [array objectEnumerator];
	      while ((o = [e nextObject])) {
	      path = [NSBundle pathForLibraryResource: the_name
				               ofType: o
				          inDirectory: @"Sounds"];
		if (path != nil && [path length] != 0)
		  {
		    break;
		  }
	      }
	    }
	}

      if ([path length] != 0)
	{
	  sound = [[self allocWithZone: NSDefaultMallocZone()]
		    initWithContentsOfFile: path byReference: NO];

	  if (sound != nil)
	    {
	      [sound setName: name];
	      RELEASE(sound);	
	      sound->_onlyReference = YES;
	    }

	  return sound;
	}
    }  
	
  return sound;
}

+ (NSArray *) soundUnfilteredFileTypes
{
  Class sourceClass;
  NSMutableArray *array;
  NSEnumerator *enumerator;
  
  array = [NSMutableArray arrayWithCapacity: 10];
  enumerator = [sourcePlugIns objectEnumerator];
  while ((sourceClass = [enumerator nextObject]) != nil)
    {
      [array addObjectsFromArray: [sourceClass soundUnfilteredFileTypes]];
    }
  
  return array;
}

+ (NSArray *) soundUnfilteredTypes
{
  Class sourceClass;
  NSMutableArray *array;
  NSEnumerator *enumerator;
  
  array = [NSMutableArray arrayWithCapacity: 10];
  enumerator = [sourcePlugIns objectEnumerator];
  while ((sourceClass = [enumerator nextObject]) != nil)
    {
      [array addObjectsFromArray: [sourceClass soundUnfilteredTypes]];
    }
  
  return array;
}

- (NSString *) name
{
  return _name;
}

- (BOOL) setName: (NSString *)aName
{
  if (!aName || [nameDict objectForKey: aName])
    {
      return NO;
    }
	
  if ((_name != nil) && self == [nameDict objectForKey: _name])
    {
      [nameDict removeObjectForKey: _name];
    }
  
  ASSIGN(_name, aName);
  
  [nameDict setObject: self forKey: _name];
  
  return YES;
}

- (NSString *) playbackDeviceIdentifier
{
  return [_sink playbackDeviceIdentifier];
}

- (void) setPlaybackDeviceIdentifier: (NSString *)playbackDeviceIdentifier
{
  if ([[_sink class] canInitWithPlaybackDevice: playbackDeviceIdentifier])
    {
      [_playbackLock lock];
      [_sink setPlaybackDeviceIdentifier: playbackDeviceIdentifier];
      [_playbackLock unlock];
    }
}

- (NSArray *) channelMapping
{
  return [_sink channelMapping];
}

- (void) setChannelMapping: (NSArray *)channelMapping
{
  [_playbackLock lock];
  [_sink setChannelMapping: channelMapping];
  [_playbackLock unlock];
}

//
// NSCoding 
//
- (void) encodeWithCoder: (NSCoder *)coder
{
  if ([coder allowsKeyedCoding])
    {
      // TODO_NIB: Determine keys for NSSound.
    }
  else
    {
      [coder encodeValueOfObjCType: @encode(BOOL) at: &_onlyReference];
      [coder encodeObject: _name];
      
      if (_onlyReference == YES)
      	{
	        return;
      	}
      [coder encodeConditionalObject: _delegate];
      [coder encodeObject: _data];
      [coder encodeObject: _playbackDeviceIdentifier];
      [coder encodeObject: _channelMapping];
    }
}

- (id) initWithCoder: (NSCoder*)decoder
{	
  if ([decoder allowsKeyedCoding])
    {
      // TODO_NIB: Determine keys for NSSound.
    }
  else
    {
      [decoder decodeValueOfObjCType: @encode(BOOL) at: &_onlyReference];
      
      if (_onlyReference == YES)
        {
          NSString *theName = [decoder decodeObject];
          RELEASE (self);
          self = RETAIN ([NSSound soundNamed: theName]);
          [self setName: theName];	
        } 
      else 
        {
          _name = RETAIN ([decoder decodeObject]);
          [self setDelegate: [decoder decodeObject]];
          _data = RETAIN([decoder decodeObject]);
          _playbackDeviceIdentifier = RETAIN([decoder decodeObject]);
          _channelMapping = RETAIN([decoder decodeObject]);
        }
    
    /* FIXME: Need to prepare the object for playback before going further. */
    }
  return self;
}

//
// NSCopying 
//
- (id) copyWithZone: (NSZone *)zone
{
  NSSound *newSound = (NSSound *)NSCopyObject(self, 0, zone);

  /* FIXME: Is all this correct?  And is this all that needs to be copied? */
  newSound->_name = [_name copyWithZone: zone];
  newSound->_data = [_data copyWithZone: zone];
  newSound->_playbackDeviceIdentifier = [_playbackDeviceIdentifier
                                          copyWithZone: zone];
  newSound->_channelMapping = [_channelMapping copyWithZone: zone];
	
  /* FIXME: Need to prepare the object for playback before going further. */
  return newSound;
}

@end
