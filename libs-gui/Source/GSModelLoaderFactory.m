/** <title>GSModelLoaderFactory</title>

   <abstract>Model loader framework</abstract>

   Copyright (C) 1997, 1999 Free Software Foundation, Inc.

   Author:  Gregory John Casamento <greg_casamento@yahoo.com>
   Date: 2005
   
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

#import "config.h"
#import <Foundation/NSArray.h>
#import <Foundation/NSData.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSEnumerator.h>
#import <Foundation/NSException.h>
#import <Foundation/NSFileManager.h>
#import <Foundation/NSString.h>

#import "GNUstepGUI/GSModelLoaderFactory.h"

@implementation GSModelLoader
+ (NSString *) type
{
  return nil;
}

+ (float) priority
{
  return 0.0;
}

- (BOOL) loadModelData: (NSData *)data
     externalNameTable: (NSDictionary *)context
              withZone: (NSZone *)zone
{
  [NSException raise: NSInternalInconsistencyException
	       format: @"Abstract model loader."];
  return NO;
}

- (BOOL) loadModelFile: (NSString *)fileName
     externalNameTable: (NSDictionary *)context
              withZone: (NSZone *)zone
{
  NSData *data = [self dataForFile: fileName];

  if (data != nil)
    {
      BOOL loaded = [self loadModelData: data
                      externalNameTable: context
                               withZone: zone];
      if (!loaded)
        NSLog(@"Could not load Nib file: %@", fileName);
      return loaded;
    }
  else
    {
      return NO;
    }
}

- (NSData *) dataForFile: (NSString *)fileName
{
  return [NSData dataWithContentsOfFile: fileName];
}

+ (NSComparisonResult) _comparePriority: (Class)loader
{
  NSComparisonResult result = NSOrderedSame;

  if ([self priority] < [loader priority])
    {
      result = NSOrderedAscending;
    }
  if ([self priority] > [loader priority])
    {
      result = NSOrderedDescending;
    }

  return result;
} 
@end

static NSMutableDictionary *_modelMap = nil;

@implementation GSModelLoaderFactory
+ (void) initialize
{
  NSArray *classes = GSObjCAllSubclassesOfClass([GSModelLoader class]);
  NSEnumerator *en = [classes objectEnumerator];
  Class cls = nil;
  
  while ((cls = [en nextObject]) != nil)
    {
      [self registerModelLoaderClass: cls];
    }
}

+ (void) registerModelLoaderClass: (Class)aClass
{
  if (_modelMap == nil)
    {
      _modelMap = [[NSMutableDictionary alloc] initWithCapacity: 5];
    }

  [_modelMap setObject: aClass forKey: (NSString *)[aClass type]];
}

+ (Class)classForType: (NSString *)type
{
  return [_modelMap objectForKey: type];
}

+ (NSArray *) supportedTypes
{
  NSArray *objectArray = [_modelMap allValues];
  NSArray *sortedArray = [objectArray sortedArrayUsingSelector: 
					    @selector(_comparePriority:)];
  NSEnumerator *oen = [sortedArray objectEnumerator];
  Class cls = nil;
  NSMutableArray *types = [[NSMutableArray alloc] init];

  while ((cls = [oen nextObject]) != nil)
    {
      [types addObject: [cls type]];
    }

  return AUTORELEASE(types);
}

+ (NSString *) supportedModelFileAtPath: (NSString *)modelPath
{
  NSString *result = nil;
  NSFileManager	*mgr = [NSFileManager defaultManager];
  NSString *ext = [modelPath pathExtension];

  if ([ext isEqual: @""])
    {
      NSArray *objectArray = [_modelMap allValues];
      NSArray *sortedArray = [objectArray sortedArrayUsingSelector: 
					    @selector(_comparePriority:)];
      NSEnumerator *oen = [sortedArray objectEnumerator];
      Class cls = nil;

      while ((cls = [oen nextObject]) != nil && result == nil)
	{
	  NSString *path = [modelPath stringByAppendingPathExtension: 
					(NSString *)[cls type]];
	  if ([mgr isReadableFileAtPath: path])
	    {
	      result = path;
	    }
	}
    }
  else
    {
      if ([_modelMap objectForKey: ext] != nil)
	{
	  if ([mgr isReadableFileAtPath: modelPath])
	    {
	      result = modelPath;
	    }
	} 
    }
  
  return result;
}

+ (GSModelLoader *) modelLoaderForFileType: (NSString *)type
{
  Class aClass = [GSModelLoaderFactory classForType: type];
  GSModelLoader *loader = nil;

  if (aClass != nil)
    {
      loader = AUTORELEASE([[aClass alloc] init]);
    }
  else
    {
      [NSException raise: NSInternalInconsistencyException
		   format: @"Unable to find model loader class."];
    }

  return loader;
}

+ (GSModelLoader *) modelLoaderForFileName: (NSString *)modelPath
{
  NSString *path = [GSModelLoaderFactory supportedModelFileAtPath: modelPath];
  GSModelLoader *result = nil;

  if (path != nil)
    {
      NSString *ext = [path pathExtension];
      result = [self modelLoaderForFileType: ext];
    }
  
  return result;  
}
@end
