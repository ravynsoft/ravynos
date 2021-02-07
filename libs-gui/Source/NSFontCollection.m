/* Implementation of class NSFontCollection
   Copyright (C) 2019, 2020 Free Software Foundation, Inc.

   By: Gregory John Casamento
   Date: Tue Dec 10 11:51:33 EST 2019

   Author: Fred Kiefer <fredkiefer@gmx.de>
   Date:   March 2020

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

#import <Foundation/NSArray.h>
#import <Foundation/NSData.h>
#import <Foundation/NSDebug.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSError.h>
#import <Foundation/NSException.h>
#import <Foundation/NSFileManager.h>
#import <Foundation/NSKeyedArchiver.h>
#import <Foundation/NSLocale.h>
#import <Foundation/NSLock.h>
#import <Foundation/NSPathUtilities.h>
#import <Foundation/NSString.h>
#import <Foundation/NSValue.h>

#import <AppKit/NSFontCollection.h>
#import <AppKit/NSFontDescriptor.h>
#import <GNUstepGUI/GSFontInfo.h>

static NSMutableDictionary *_availableFontCollections = nil;
static NSLock *_fontCollectionLock = nil;

/*
 * Private functions...
 */
@interface NSFontCollection (Private)

+ (NSFontCollection *) _readFileAtPath: (NSString *)path;
+ (void) _loadAvailableFontCollections;
- (BOOL) _writeToFileAtPath: (NSString *)path;
- (BOOL) _writeToFile;
- (BOOL) _removeFile;
- (NSMutableDictionary *) _fontCollectionDictionary;
- (void) _setFontCollectionDictionary: (NSMutableDictionary *)dict;
- (void) _setQueryDescriptors: (NSArray *)queryDescriptors;
- (void) _setFullFileName: (NSString *)fn;
- (NSString *) _fullFileName;
- (void) _setName: (NSString *)n;
- (NSString *) _name;
@end

/*
 * Private functions...
 */
@implementation NSFontCollection (Private)

/**
 * Load all font collections....
 */
+ (void) _loadAvailableFontCollections
{
  [_fontCollectionLock lock];

  if (_availableFontCollections != nil)
    {
      // Nothing to do ... already loaded
      [_fontCollectionLock unlock];
    }
  else
    {
      NSString      *dir = nil;
      NSEnumerator  *e = nil;
      NSFileManager *fm = [NSFileManager defaultManager];

      // Create the global array of font collections...
      _availableFontCollections = [[NSMutableDictionary alloc] init];

      /*
       * Load font lists found in standard paths into the array
       */
      e = [NSSearchPathForDirectoriesInDomains(NSLibraryDirectory,
                                               NSAllDomainsMask, YES) objectEnumerator];

      while ((dir = (NSString *)[e nextObject]))
	{
	  BOOL flag;
          NSDirectoryEnumerator *de = nil;
          NSString *file = nil;

	  dir = [dir stringByAppendingPathComponent: @"FontCollections"];
	  if (![fm fileExistsAtPath: dir isDirectory: &flag] || !flag)
	    {
	      // Only process existing directories
	      continue;
	    }

	  de = [fm enumeratorAtPath: dir];
	  while ((file = [de nextObject]))
	    {
	      if ([[file pathExtension] isEqualToString: @"collection"])
		{
		  NSString *name = [file stringByDeletingPathExtension];
                  NSString *path = [dir stringByAppendingPathComponent: file];
                  NSFontCollection *newCollection = [self _readFileAtPath: path];

                  if (newCollection != nil && name != nil)
                    {
                      [newCollection _setFullFileName: path];
                      [newCollection _setName: name];
                      [_availableFontCollections setObject: newCollection
                                                    forKey: name];
                    }
                }
	    }
	}

      [_fontCollectionLock unlock];
    }
}

+ (NSFontCollection *) _readFileAtPath: (NSString *)path
{
  NSData *d = [NSData dataWithContentsOfFile: path];
  NSKeyedUnarchiver *u = [[NSKeyedUnarchiver alloc] initForReadingWithData: d];
  NSFontCollection *fc = [[NSFontCollection alloc] initWithCoder: u];

  RELEASE(u);

  return AUTORELEASE(fc);
}

/*
 * Writing and Removing Files
 */
- (BOOL) _writeToFileAtPath: (NSString *)path
{
  BOOL success = NO;
  NSMutableData *m = [[NSMutableData alloc] initWithCapacity: 1024];
  NSKeyedArchiver *a = [[NSKeyedArchiver alloc] initForWritingWithMutableData: m];

  [self encodeWithCoder: a];
  [a finishEncoding];
  RELEASE(a);

  // Write the file....
  NSDebugLLog(@"NSFontCollection", @"Writing to %@", path);
  success = [m writeToFile: path atomically: YES];
  RELEASE(m);
  return success;
}

- (BOOL) _writeToFile
{
  NSFileManager *fm = [NSFileManager defaultManager];
  BOOL           success = NO;
  NSString      *path = [self _fullFileName];

  /*
   * We need to initialize before saving, to avoid the new file being
   * counted as a different collection thus making it appear twice
   */
  [NSFontCollection _loadAvailableFontCollections];

  if (path == nil)
    {
      // Find library....
      NSArray *paths = NSSearchPathForDirectoriesInDomains(NSLibraryDirectory,
                                                           NSUserDomainMask, YES);

      if ([paths count] == 0)
        {
          NSLog(@"Failed to find Library directory for user");
          return NO;
        }

      path = [[paths objectAtIndex: 0]
               stringByAppendingPathComponent: @"FontCollections"];
      if ([fm fileExistsAtPath: path] == NO)
        {
          if ([fm createDirectoryAtPath: path
                  withIntermediateDirectories: YES
                             attributes: nil
                                  error: NULL])
            {
              NSDebugLLog(@"NSFontCollection", @"Created standard directory %@", path);
            }
          else
            {
              NSLog(@"Failed attempt to create directory %@", path);
              return NO;
            }
        }

      NSDebugLLog(@"NSFontCollection", @"Font collection name = %@", [self _name]);
      path = [path stringByAppendingPathComponent:
                     [[self _name] stringByAppendingPathExtension: @"collection"]];

      [self _setFullFileName: path];
    }

  // Create the archive...
  success = [self _writeToFileAtPath: path];
  if (success)
    {
      [_fontCollectionLock lock];
      if ([[_availableFontCollections allValues] containsObject: self] == NO)
        {
          [_availableFontCollections setObject: self forKey: [self _name]];
        }
      [_fontCollectionLock unlock];
    }

  return success;
}

- (BOOL) _removeFile
{
  NSFileManager *fm = [NSFileManager defaultManager];
  BOOL           isDir;
  NSString      *path = [self _fullFileName];
  BOOL           result = NO;

  /*
   * We need to initialize before saving, to avoid the new file being
   * counted as a different collection thus making it appear twice
   */
  [NSFontCollection _loadAvailableFontCollections];

  if (path == nil)
    {
      // the standard path for saving font collections
      NSArray *paths = NSSearchPathForDirectoriesInDomains(NSLibraryDirectory,
                                                           NSUserDomainMask, YES);
      if ([paths count] == 0)
	{
	  NSLog(@"Failed to find Library directory for user");
	  return NO;
	}
      path = [[[[paths objectAtIndex: 0]
                 stringByAppendingPathComponent: @"FontCollections"]
                stringByAppendingPathComponent: [self _name]]
               stringByAppendingPathExtension: @"collection"];
    }

  if (![fm fileExistsAtPath: path isDirectory: &isDir] || isDir)
    {
      NSLog(@"Failed to find font collection file to delete");
      return NO;
    }
  else
    {
      // Remove the file
      result = [[NSFileManager defaultManager] removeFileAtPath: path
                                                        handler: nil];

      // Remove the collection from the global list of font collections
      [_fontCollectionLock lock];
      [_availableFontCollections removeObjectForKey: [self _name]];
      [_fontCollectionLock unlock];

      // Reset file name
      [self _setFullFileName: nil];
    }

  return result;
}

- (NSMutableDictionary *) _fontCollectionDictionary
{
  return _fontCollectionDictionary;
}

- (void) _setFontCollectionDictionary: (NSMutableDictionary *)dict
{
  ASSIGNCOPY(_fontCollectionDictionary, dict);
}

- (void) _setQueryDescriptors: (NSArray *)queryDescriptors
{
  return [_fontCollectionDictionary setObject: queryDescriptors
                                       forKey: @"NSFontCollectionFontDescriptors"];
}

- (NSMutableDictionary *) _fontCollectionAttributes
{
  NSMutableDictionary *attrs = [_fontCollectionDictionary objectForKey:
                                                            @"NSFontCollectionAttributes"];

  if (attrs == nil)
    {
      attrs = [NSMutableDictionary dictionary];
      [_fontCollectionDictionary setObject: attrs
                                    forKey: @"NSFontCollectionAttributes"];
    }
  return attrs;
}

- (void) _setName: (NSString *)n
{
  [[self _fontCollectionAttributes] setObject: n
                                       forKey: @"NSFontCollectionName"];
}

- (NSString *) _name
{
  return [[self _fontCollectionAttributes] objectForKey: @"NSFontCollectionName"];
}

- (void) _setFullFileName: (NSString *)fn
{
  [[self _fontCollectionAttributes] setObject: fn
                                forKey: @"NSFontCollectionFileName"];
}

- (NSString *) _fullFileName
{
  return [[self _fontCollectionAttributes] objectForKey: @"NSFontCollectionFileName"];
}

@end

/*
 * NSFontCollection
 */
@implementation NSFontCollection

+ (void) initialize
{
  if (self == [NSFontCollection class])
    {
      [self _loadAvailableFontCollections];
    }
}

// Initializers...
- (instancetype) initWithDescriptors: (NSArray *)queryDescriptors
{
  self = [super init];
  if (self != nil)
    {
      _fontCollectionDictionary = [[NSMutableDictionary alloc] initWithCapacity: 10];
      [self _setQueryDescriptors: queryDescriptors];
    }
  return self;
}

- (instancetype) init
{
  return [self initWithDescriptors: [NSArray array]];
}

- (void) dealloc
{
  RELEASE(_fontCollectionDictionary);
  [super dealloc];
}

+ (NSFontCollection *) fontCollectionWithDescriptors: (NSArray *)queryDescriptors
{
  NSFontCollection *fc = [[NSFontCollection alloc] initWithDescriptors: queryDescriptors];
  return AUTORELEASE(fc);
}

+ (NSFontCollection *) fontCollectionWithAllAvailableDescriptors
{
  NSFontCollection *fc = [_availableFontCollections objectForKey: NSFontCollectionAllFonts];
  if (fc == nil)
    {
      NSDictionary *fa = [NSDictionary dictionary];
      NSFontDescriptor *fd = [NSFontDescriptor fontDescriptorWithFontAttributes: fa];
      fc =  [self fontCollectionWithDescriptors: [NSArray arrayWithObject: fd]];
      if (fc != nil)
        {
          [fc _setName: NSFontCollectionAllFonts];
          [fc _writeToFile];
        }
    }
  return fc;
}

+ (NSFontCollection *) fontCollectionWithLocale: (NSLocale *)locale
{
  NSDictionary *fa = [NSDictionary dictionaryWithObject: [locale languageCode]
                                                 forKey: @"NSCTFontDesignLanguagesAttribute"];
  NSFontDescriptor *fd = [NSFontDescriptor fontDescriptorWithFontAttributes: fa];
  return [self fontCollectionWithDescriptors: [NSArray arrayWithObject: fd]];
}

+ (BOOL) showFontCollection: (NSFontCollection *)collection
                   withName: (NSFontCollectionName)name
                 visibility: (NSFontCollectionVisibility)visibility
                      error: (NSError **)error
{
  [collection _setName: name];
  return [collection _writeToFile];
}

+ (BOOL) hideFontCollectionWithName: (NSFontCollectionName)name
                         visibility: (NSFontCollectionVisibility)visibility
                              error: (NSError **)error
{
  NSFontCollection *collection = [_availableFontCollections objectForKey: name];

  return [collection _removeFile];
}

+ (BOOL) renameFontCollectionWithName: (NSFontCollectionName)aname
                           visibility: (NSFontCollectionVisibility)visibility
                               toName: (NSFontCollectionName)name
                                error: (NSError **)error
{
  NSFontCollection *collection = [_availableFontCollections objectForKey: aname];
  BOOL rv = [collection _removeFile];

  if (rv == YES)
    {
      [collection _setName: name];
      [collection _writeToFile];
    }

  return rv;
}

+ (NSArray *) allFontCollectionNames
{
  return [_availableFontCollections allKeys];
}

+ (NSFontCollection *) fontCollectionWithName: (NSFontCollectionName)name
{
  NSFontCollection *fc = [_availableFontCollections objectForKey: name];
  if (fc == nil)
    {
      fc = [[NSFontCollection alloc] init];
      [fc _setName: name];
      AUTORELEASE(fc);
    }
  return fc;
}

+ (NSFontCollection *) fontCollectionWithName: (NSFontCollectionName)name
                                   visibility: (NSFontCollectionVisibility)visibility
{
  return [self fontCollectionWithName: name];
}

// Descriptors
- (NSArray *) queryDescriptors
{
  return [_fontCollectionDictionary objectForKey: @"NSFontCollectionFontDescriptors"];
}

- (NSArray *) exclusionDescriptors
{
  return [_fontCollectionDictionary objectForKey: @"NSFontExclusionDescriptorAttributes"];
}

- (NSArray *) matchingDescriptors
{
  return [self matchingDescriptorsWithOptions: nil];
}

- (NSArray *) matchingDescriptorsWithOptions: (NSDictionary *)options
{
  return [self matchingDescriptorsForFamily: nil options: options];
}

- (NSArray *) matchingDescriptorsForFamily: (NSString *)family
{
  return [self matchingDescriptorsForFamily: family options: nil];
}

- (NSArray *) matchingDescriptorsForFamily: (NSString *)family options: (NSDictionary *)options
{
  GSFontEnumerator *fen = [GSFontEnumerator sharedEnumerator];

  return [fen matchingDescriptorsForFamily: family
                                   options: options
                                 inclusion: [self queryDescriptors]
                                 exculsion: [self exclusionDescriptors]];
}

- (instancetype) copyWithZone: (NSZone *)zone
{
  NSFontCollection *fc = [[NSFontCollection allocWithZone: zone] init];
  [fc _setFontCollectionDictionary: _fontCollectionDictionary];
  return fc;
}

- (instancetype) mutableCopyWithZone: (NSZone *)zone
{
  NSMutableFontCollection *fc = [[NSMutableFontCollection allocWithZone: zone] init];

  [fc _setFontCollectionDictionary: [_fontCollectionDictionary mutableCopyWithZone: zone]];

  return fc;
}

- (void) encodeWithCoder: (NSCoder *)coder
{
  if ([coder allowsKeyedCoding])
    {
      [coder encodeObject: _fontCollectionDictionary
                   forKey: @"NSFontCollectionDictionary"];
    }
  else
    {
      [coder encodeObject: _fontCollectionDictionary];
    }
}

- (instancetype) initWithCoder: (NSCoder *)coder
{
  self = [super init];
  if (self != nil)
    {
      if ([coder allowsKeyedCoding])
        {
          ASSIGN(_fontCollectionDictionary,
                 [coder decodeObjectForKey: @"NSFontCollectionDictionary"]);
        }
    }
  else
    {
      [coder decodeValueOfObjCType: @encode(id) at: &_fontCollectionDictionary];
    }
  return self;
}

@end


@implementation NSMutableFontCollection

+ (NSMutableFontCollection *) fontCollectionWithDescriptors: (NSArray *)queryDescriptors
{
  return [[NSFontCollection fontCollectionWithDescriptors: queryDescriptors] mutableCopy];
}

+ (NSMutableFontCollection *) fontCollectionWithAllAvailableDescriptors
{
  return [[NSFontCollection fontCollectionWithAllAvailableDescriptors] mutableCopy];
}

+ (NSMutableFontCollection *) fontCollectionWithLocale: (NSLocale *)locale
{
  return [[NSFontCollection fontCollectionWithLocale: locale] mutableCopy];
}

+ (NSMutableFontCollection *) fontCollectionWithName: (NSFontCollectionName)name
{
  return [[NSFontCollection fontCollectionWithName: name] mutableCopy];
}

+ (NSMutableFontCollection *) fontCollectionWithName: (NSFontCollectionName)name
                                          visibility: (NSFontCollectionVisibility)visibility
{
  return [[NSFontCollection fontCollectionWithName: name visibility: visibility] mutableCopy];
}

- (NSArray *) queryDescriptors
{
  return [super queryDescriptors];
}

- (void) setQueryDescriptors: (NSArray *)queryDescriptors
{
  [super _setQueryDescriptors: [queryDescriptors mutableCopy]];
}

- (NSArray *) exclusionDescriptors
{
  return [_fontCollectionDictionary objectForKey: @"NSFontExclusionDescriptorAttributes"];
}

- (void) setExclusionDescriptors: (NSArray *)exclusionDescriptors
{
  [_fontCollectionDictionary setObject: [exclusionDescriptors mutableCopy]
                                forKey: @"NSFontExclusionDescriptorAttributes"];

}

- (void) addQueryForDescriptors: (NSArray *)descriptors
{
  NSMutableArray *arr = [[self queryDescriptors] mutableCopy];
  NSMutableArray *ed = [[self exclusionDescriptors] mutableCopy];

  [ed removeObjectsInArray: descriptors];
  [arr addObjectsFromArray: descriptors];
  [self setQueryDescriptors: arr];
  [self setExclusionDescriptors: ed];
  RELEASE(arr);
  RELEASE(ed);
}

- (void) removeQueryForDescriptors: (NSArray *)descriptors
{
  NSMutableArray *arr = [[self queryDescriptors] mutableCopy];
  NSMutableArray *ed = [[self exclusionDescriptors] mutableCopy];

  [ed addObjectsFromArray: descriptors];
  [arr removeObjectsInArray: descriptors];
  [self setQueryDescriptors: arr];
  [self setExclusionDescriptors: ed];
  RELEASE(arr);
  RELEASE(ed);
}

@end
