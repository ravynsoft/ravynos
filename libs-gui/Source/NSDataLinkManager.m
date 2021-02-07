/** <title>NSDataLinkManager</title>

   Copyright (C) 1996, 2005 Free Software Foundation, Inc.

   Author: Gregory John Casamento <greg_casamento@yahoo.com>
   Date: 2005
   Author: Scott Christley <scottc@net-community.com>
   Date: 1996
   
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

#include "config.h"
#import <Foundation/NSDictionary.h>
#import <Foundation/NSEnumerator.h>
#import <Foundation/NSArray.h>
#import <Foundation/NSArchiver.h>
#import "AppKit/NSDataLinkManager.h"
#import "AppKit/NSDataLink.h"
#import "AppKit/NSPasteboard.h"

@interface NSDataLink (Private)
- (void) setLastUpdateTime: (NSDate *)date;
- (void) setSourceFilename: (NSString *)src;
- (void) setDestinationFilename: (NSString *)dst;
- (void) setSourceManager: (id)src;
- (void) setDestinationManager: (id)dst;
- (void) setSourceSelection: (id)src;
- (void) setDestinationSelection: (id)dst;
@end

@implementation NSDataLink (Private)
- (void) setLastUpdateTime: (NSDate *)date
{
  ASSIGN(lastUpdateTime, date);
}

- (void) setSourceFilename: (NSString *)src
{
  ASSIGN(sourceFilename,src);
}

- (void) setDestinationFilename: (NSString *)dst
{
  ASSIGN(destinationFilename, dst);
}

- (void) setSourceManager: (id)src
{
  ASSIGN(sourceManager,src);
}

- (void) setDestinationManager: (id)dst
{
  ASSIGN(destinationManager,dst);
}

- (void) setSourceSelection: (id)src
{
  ASSIGN(sourceSelection,src);
}

- (void) setDestinationSelection: (id)dst
{
  ASSIGN(destinationSelection,dst);
}

- (void) setIsMarker: (BOOL)flag
{
  _flags.isMarker = flag;
}
@end


@implementation NSDataLinkManager

//
// Class methods
//
+ (void)initialize
{
  if (self == [NSDataLinkManager class])
    {
      // Initial version
      [self setVersion: 0];
    }
}

//
// Instance methods
//
//
// Initializing and Freeing a Link Manager
//
- (id)initWithDelegate:(id)anObject
{
  self = [super init];

  if (self != nil)
    {
      ASSIGN(delegate,anObject);
      filename = nil;
      _flags.delegateVerifiesLinks = NO;
      _flags.interactsWithUser = NO;
      _flags.isEdited = NO;
      _flags.areLinkOutlinesVisible = NO;
    }

  return self;
}

- (id)initWithDelegate:(id)anObject
	      fromFile:(NSString *)path
{
  self = [super init];

  if (self != nil)
    {
      ASSIGN(delegate,anObject);
      ASSIGN(filename,path);
      _flags.delegateVerifiesLinks = NO;
      _flags.interactsWithUser = NO;
      _flags.isEdited = NO;
      _flags.areLinkOutlinesVisible = NO;
    }

  return self;
}

//
// Adding and Removing Links
//
- (BOOL)addLink:(NSDataLink *)link
	     at:(NSSelection *)selection
{
  BOOL result = NO;

  [link setDestinationSelection: selection];
  [link setDestinationManager: self];

  if ([destinationLinks containsObject: link] == NO)
    {
      [destinationLinks addObject: link];
      result = YES;
    }

  return result;
}

- (BOOL)addLinkAsMarker:(NSDataLink *)link
		     at:(NSSelection *)selection
{
  [link setIsMarker: YES];
  return [self addLink: link at: selection];
}

- (NSDataLink *)addLinkPreviouslyAt:(NSSelection *)oldSelection
		     fromPasteboard:(NSPasteboard *)pasteboard
                                 at:(NSSelection *)selection
{
  NSData *data = [pasteboard dataForType: NSDataLinkPboardType];
  NSArray *links = [NSUnarchiver unarchiveObjectWithData: data];
  NSEnumerator *en = [links objectEnumerator];
  NSDataLink *link = nil;

  while ((link = [en nextObject]) != nil)
    {
	if ([link destinationSelection] == oldSelection)
	{	    
	}
    }

  return nil;
}

- (void)breakAllLinks
{
  NSArray *allLinks = [sourceLinks arrayByAddingObjectsFromArray: destinationLinks];
  NSEnumerator *en = [allLinks objectEnumerator];
  id obj = nil;

  while ((obj = [en nextObject]) != nil)
    {
      [obj break];
    }
}

- (void)writeLinksToPasteboard:(NSPasteboard *)pasteboard
{
  NSArray *allLinks = [sourceLinks arrayByAddingObjectsFromArray: destinationLinks];
  NSEnumerator *en = [allLinks objectEnumerator];
  id obj = nil;

  while ((obj = [en nextObject]) != nil)
    {
      [obj writeToPasteboard: pasteboard];
    }
}

//
// Informing the Link Manager of Document Status
//
- (void)noteDocumentClosed
{
    if ([delegate respondsToSelector: @selector(dataLinkManagerCloseDocument:)])
    {
	[delegate dataLinkManagerCloseDocument: self];
    }
}

- (void)noteDocumentEdited
{
    if ([delegate respondsToSelector: @selector(dataLinkManagerDidEditLinks:)])
    {
	[delegate dataLinkManagerDidEditLinks: self];
    }
}

- (void)noteDocumentReverted
{
    if ([delegate respondsToSelector: @selector(dataLinkManagerDidEditLinks:)])
    {
	[delegate dataLinkManagerDidEditLinks: self];
    }
}

- (void)noteDocumentSaved
{
    // implemented by subclass
}

- (void)noteDocumentSavedAs:(NSString *)path
{
    // implemented by subclass
}

- (void)noteDocumentSavedTo:(NSString *)path
{
    // implemented by subclass
}

//
// Getting and Setting Information about the Link Manager
//
- (id)delegate
{
  return delegate;
}

- (BOOL)delegateVerifiesLinks
{
  return _flags.delegateVerifiesLinks;
}

- (NSString *)filename
{
  return filename;
}

- (BOOL)interactsWithUser
{
  return _flags.interactsWithUser;
}

- (BOOL)isEdited
{
  return _flags.isEdited;
}

- (void)setDelegateVerifiesLinks:(BOOL)flag
{
  _flags.delegateVerifiesLinks = flag;
}

- (void)setInteractsWithUser:(BOOL)flag
{
  _flags.interactsWithUser = flag;
}

//
// Getting and Setting Information about the Manager's Links
//
- (BOOL)areLinkOutlinesVisible
{
  return _flags.areLinkOutlinesVisible;
}

- (NSEnumerator *)destinationLinkEnumerator
{
  return [destinationLinks objectEnumerator];
}

- (NSDataLink *)destinationLinkWithSelection:(NSSelection *)destSel
{
  NSEnumerator *en = [self destinationLinkEnumerator];
  id obj = nil;

  while ((obj = [en nextObject]) != nil)
    {
      if ([obj destinationSelection] == destSel)
	{
	  break;
	}
    }

  return obj;
}

- (void)setLinkOutlinesVisible:(BOOL)flag
{
  _flags.areLinkOutlinesVisible = flag;
}

- (NSEnumerator *)sourceLinkEnumerator
{
  return [sourceLinks objectEnumerator];
}

//
// NSCoding protocol
//
- (void) encodeWithCoder: (NSCoder*)aCoder
{
  BOOL flag = NO;

  if ([aCoder allowsKeyedCoding])
    {
      [aCoder encodeObject: filename forKey: @"GSFilename"];
      [aCoder encodeObject: sourceLinks forKey: @"GSSourceLinks"];
      [aCoder encodeObject: destinationLinks forKey: @"GSDestinationLinks"];
      
      flag = _flags.areLinkOutlinesVisible;
      [aCoder encodeBool: flag forKey: @"GSAreLinkOutlinesVisible"];
      flag = _flags.delegateVerifiesLinks;
      [aCoder encodeBool: flag forKey: @"GSDelegateVerifiesLinks"];
      flag = _flags.interactsWithUser;
      [aCoder encodeBool: flag forKey: @"GSInteractsWithUser"];
      flag = _flags.isEdited;
      [aCoder encodeBool: flag forKey: @"GSIsEdited"];
    }
  else
    {
      [aCoder encodeValueOfObjCType: @encode(id)  at: &filename];
      [aCoder encodeValueOfObjCType: @encode(id)  at: &sourceLinks];
      [aCoder encodeValueOfObjCType: @encode(id)  at: &destinationLinks];
      
      flag = _flags.areLinkOutlinesVisible;
      [aCoder encodeValueOfObjCType: @encode(BOOL)  at: &flag];
      flag = _flags.delegateVerifiesLinks;
      [aCoder encodeValueOfObjCType: @encode(BOOL)  at: &flag];
      flag = _flags.interactsWithUser;
      [aCoder encodeValueOfObjCType: @encode(BOOL)  at: &flag];
      flag = _flags.isEdited;
      [aCoder encodeValueOfObjCType: @encode(BOOL)  at: &flag];
    }
}

- (id) initWithCoder: (NSCoder*)aCoder
{
  if ([aCoder allowsKeyedCoding])
    {
      BOOL flag = NO;
      id obj;

      obj = [aCoder decodeObjectForKey: @"GSFilename"];
      ASSIGN(filename,obj);
      obj = [aCoder decodeObjectForKey: @"GSSourceLinks"];
      ASSIGN(sourceLinks,obj);
      obj = [aCoder decodeObjectForKey: @"GSDestinationLinks"];
      ASSIGN(destinationLinks,obj);
      
      flag = [aCoder decodeBoolForKey: @"GSAreLinkOutlinesVisible"]; 
      _flags.areLinkOutlinesVisible = flag;
      flag = [aCoder decodeBoolForKey: @"GSDelegateVerifiesLinks"];
      _flags.delegateVerifiesLinks = flag;
      flag = [aCoder decodeBoolForKey: @"GSInteractsWithUser"];
      _flags.interactsWithUser = flag;
      flag = [aCoder decodeBoolForKey: @"GSIsEdited"];
      _flags.isEdited = flag;
    }
  else
    {
      int version = [aCoder versionForClassName: @"NSDataLinkManager"];
      if (version == 0)
	{
	  BOOL flag = NO;
	  
	  [aCoder decodeValueOfObjCType: @encode(id)  at: &filename];
	  [aCoder decodeValueOfObjCType: @encode(id)  at: &sourceLinks];
	  [aCoder decodeValueOfObjCType: @encode(id)  at: &destinationLinks];
	  
	  [aCoder decodeValueOfObjCType: @encode(BOOL)  at: &flag];
	  _flags.areLinkOutlinesVisible = flag;
	  [aCoder decodeValueOfObjCType: @encode(BOOL)  at: &flag];
	  _flags.delegateVerifiesLinks = flag;
	  [aCoder decodeValueOfObjCType: @encode(BOOL)  at: &flag];
	  _flags.interactsWithUser = flag;
	  [aCoder decodeValueOfObjCType: @encode(BOOL)  at: &flag];
	  _flags.isEdited = flag;
	}
      else
	return nil;
    }
  return self;
}

@end
