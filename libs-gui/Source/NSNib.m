/** <title>NSNib</title>

   <abstract>
   This class serves as a container for a nib file.  It's possible
   to load a nib file from a URL or from a bundle.   Using this
   class the nib file can now be "preloaded" and instantiated
   multiple times when/if needed.  Also, since it's possible to
   initialize this class using a NSURL it's possible to load
   nib files from remote locations.
   <br/>
   This class uses: NSNibOwner and NSNibTopLevelObjects to allow
   the caller to specify the owner of the nib during instantiation
   and receive an array containing the top level objects of the nib
   file.
   </abstract>

   Copyright (C) 2004 Free Software Foundation, Inc.

   Author: Gregory John Casamento <greg_casamento@yahoo.com>
   Date: 2004

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
#import <Foundation/NSArchiver.h>
#import <Foundation/NSBundle.h>
#import <Foundation/NSData.h>
#import <Foundation/NSDebug.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSException.h>
#import <Foundation/NSString.h>
#import <Foundation/NSURL.h>

#import "AppKit/NSNib.h"
#import "AppKit/NSNibLoading.h"
#import "GNUstepGUI/GSModelLoaderFactory.h"

@implementation NSNib

// private method to read in the data...
- (void) _readNibData: (NSString *)fileName
{
  NSDebugLog(@"Loading model `%@'...\n", fileName);
  NS_DURING
    {
      NSString *newFileName = [GSModelLoaderFactory supportedModelFileAtPath: fileName];
      ASSIGN(_loader, [GSModelLoaderFactory modelLoaderForFileType: [newFileName pathExtension]]);
      ASSIGN(_nibData, [_loader dataForFile: newFileName]);
      NSDebugLog(@"Loaded data from %@...", newFileName);
    }
  NS_HANDLER
    {
      NSLog(@"Exception occurred while loading model: %@", [localException reason]);
    }
  NS_ENDHANDLER
}

// Public methods...

/**
 * Load the NSNib object from the specified URL.  This location can be
 * any type of resource capable of being pointed to by the NSURL object.
 * A file in the local file system or a file on an ftp site.
 */
- (id) initWithContentsOfURL: (NSURL *)nibFileURL
{
  if ((self = [super init]) != nil)
    {
      // Currently we need this short cut for GModel files.
      // Remove this when the hack there is cleaned up.
      if ([nibFileURL isFileURL])
        {
          [self _readNibData: [nibFileURL path]];
        }
      else
        {
          NS_DURING
            {
              ASSIGN(_loader, [GSModelLoaderFactory modelLoaderForFileType:
                                                      [[nibFileURL path] pathExtension]]);
              // load the nib data into memory...
              ASSIGN(_nibData, [NSData dataWithContentsOfURL: nibFileURL]);
            }
          NS_HANDLER
            {
              NSLog(@"Exception occurred while loading model: %@", [localException reason]);
            }
          NS_ENDHANDLER
        }
    }
  return self;
}

/**
 * Load the nib indicated by <code>nibNamed</code>.  If the <code>bundle</code>
 * argument is <code>nil</code>, then the main bundle is used to resolve
 * the path, otherwise the bundle which is supplied will be used.
 */
- (instancetype) initWithNibNamed: (NSNibName)nibNamed
                           bundle: (NSBundle *)bundle
{
  if ((self = [super init]) != nil)
    {
      NSString *fileName = nil;

      // Keep the bundle for resource creation
      ASSIGN(_bundle, bundle);

      if (bundle == nil)
	{
	  bundle = [NSBundle mainBundle];
	}

      // initialize the bundle...
      fileName = [bundle pathForNibResource: nibNamed];
      if (fileName == nil)
        {
          DESTROY(self);
          return nil;
        }

      // load the nib data into memory...
      [self _readNibData: fileName];
    }
  return self;
}

- (instancetype) initWithNibData: (NSData *)nibData
                          bundle: (NSBundle *)bundle
{
  if ((self = [super init]) != nil)
    {
      ASSIGN(_bundle, bundle);
      ASSIGN(_nibData, nibData);
      // FIXME: Hardcode the most likely loader
      ASSIGN(_loader, [GSModelLoaderFactory modelLoaderForFileType: @"nib"]);
    }
  return self;
}

/**
 * This is a GNUstep specific method.  This method is used when the caller
 * wants the objects instantiated in the nib to be stored in the given
 * <code>zone</code>.
 */
- (BOOL) instantiateNibWithExternalNameTable: (NSDictionary *)externalNameTable
                                    withZone: (NSZone *)zone
{
  return [_loader loadModelData: _nibData
		  externalNameTable: externalNameTable
		  withZone: zone];
}

/**
 * This method instantiates the nib file.  The externalNameTable dictionary
 * accepts the NSNibOwner and NSNibTopLevelObjects entries described earlier.
 * It is recommended, for subclasses whose purpose is to change the behaviour
 * of nib loading, to override this method.
 */
- (BOOL) instantiateNibWithExternalNameTable: (NSDictionary *)externalNameTable
{
  return [self instantiateNibWithExternalNameTable: externalNameTable
	       withZone: NSDefaultMallocZone()];
}

/**
 * This method instantiates the nib file.  It utilizes the
 * instantiateNibWithExternalNameTable: method to, in a convenient way,
 * allow the user to specify both keys accepted by the
 * nib loading process.
 */
- (BOOL) instantiateNibWithOwner: (id)owner
                 topLevelObjects: (NSArray **)topLevelObjects
{
  NSMutableDictionary *externalNameTable = [NSMutableDictionary dictionary];

  // add the necessary things to the table...
  [externalNameTable setObject: owner forKey: NSNibOwner];

  if (topLevelObjects != 0)
    {
      *topLevelObjects = [NSMutableArray array];
      [externalNameTable setObject: *topLevelObjects forKey: NSNibTopLevelObjects];
    }

  return [self instantiateNibWithExternalNameTable: externalNameTable];
}

- (BOOL) instantiateWithOwner: (id)owner
              topLevelObjects: (NSArray **)topLevelObjects
{
  return [self instantiateNibWithOwner: owner topLevelObjects: topLevelObjects];
}

- (id) initWithCoder: (NSCoder *)coder
{
  if ((self = [super init]) != nil)
    {
      //
      // NOTE: This is okay, since the only encodings which will ever be built into
      //       the gui library are nib and gorm.  GModel only supports certain
      //       objects and is going to be deprecated in the future.  There just so
      //       happens to be a one to one correspondence here.
      //
      if ([coder allowsKeyedCoding])
	{
	  // TODO_NIB: Need to verify this key...
	  ASSIGN(_nibData, [coder decodeObjectForKey: @"NSData"]);
	  ASSIGN(_loader, [GSModelLoaderFactory modelLoaderForFileType: @"nib"]);
	}
      else
	{
	  // this is sort of a kludge...
	  [coder decodeValueOfObjCType: @encode(id)
		 at: &_nibData];
	  ASSIGN(_loader, [GSModelLoaderFactory modelLoaderForFileType: @"gorm"]);
	}
    }
  return self;
}

- (void) encodeWithCoder: (NSCoder *)coder
{
  if ([coder allowsKeyedCoding])
    {
      // TODO_NIB: Need to verify this key...
      [coder encodeObject: _nibData
	     forKey: @"NSData"];
    }
  else
    {
      [coder encodeValueOfObjCType: @encode(id)
	     at: &_nibData];
    }
}

- (void) dealloc
{
  RELEASE(_nibData);
  RELEASE(_loader);
  TEST_RELEASE(_bundle);
  [super dealloc];
}

@end
