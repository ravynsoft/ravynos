/**
   <title>NSNib.h</title>

   <abstract>
   This class serves as a container for a nib file.  It's possible 
   to load a nib file from a URL or from a bundle.   Using this 
   class the nib file can now be "preloaded" and instantiated 
   multiple times when/if needed.  Also, since it's possible to 
   initialize this class using a NSURL it's possible to load 
   nib files from remote locations. 
   <b/>
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

#ifndef _GNUstep_H_NSNib
#define _GNUstep_H_NSNib
#import <GNUstepBase/GSVersionMacros.h>

#import <Foundation/NSObject.h>
#import <Foundation/NSZone.h>
#import <AppKit/AppKitDefines.h>

@class NSData;
@class NSDictionary;
@class NSString;
@class NSBundle;
@class NSURL;
@class NSArray;
@class NSMutableArray;

#if OS_API_VERSION(MAC_OS_X_VERSION_10_3, GS_API_LATEST)
APPKIT_EXPORT NSString *NSNibTopLevelObjects;
APPKIT_EXPORT NSString *NSNibOwner;
#endif

typedef NSString *NSNibName;

@interface NSNib : NSObject <NSCoding>
{
  NSData *_nibData;
  id _loader;
  NSBundle *_bundle;
}

// reading the data...
- (id)initWithContentsOfURL: (NSURL *)nibFileURL;
- (instancetype)initWithNibNamed: (NSNibName)nibNamed bundle: (NSBundle *)bundle;

#if OS_API_VERSION(MAC_OS_X_VERSION_10_8, GS_API_LATEST)
- (instancetype)initWithNibData: (NSData *)nibData 
                         bundle: (NSBundle *)bundle;
#endif

// instantiating the nib.
- (BOOL)instantiateNibWithExternalNameTable: (NSDictionary *)externalNameTable;
- (BOOL)instantiateNibWithOwner: (id)owner topLevelObjects: (NSArray **)topLevelObjects;

#if OS_API_VERSION(GS_API_NONE, GS_API_NONE)
- (BOOL)instantiateNibWithExternalNameTable: (NSDictionary *)externalNameTable withZone: (NSZone *)zone;
#endif
#if OS_API_VERSION(MAC_OS_X_VERSION_10_8, GS_API_LATEST)
- (BOOL)instantiateWithOwner: (id)owner 
             topLevelObjects: (NSArray **)topLevelObjects;
#endif
@end

#endif /* _GNUstep_H_NSNib */
