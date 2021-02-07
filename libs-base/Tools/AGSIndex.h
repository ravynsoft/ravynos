#ifndef	_INCLUDED_AGSINDEX_H
#define	_INCLUDED_AGSINDEX_H
/** 

   <title>AGSIndex ... a class to create references for a gsdoc file</title>
   Copyright (C) 2001 Free Software Foundation, Inc.

   Written by:  Richard Frith-Macdonald <richard@brainstorm.co.uk>
   Created: October 2001

   This file is part of the GNUstep Project

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either
   version 3 of the License, or (at your option) any later version.

   You should have received a copy of the GNU General Public
   License along with this program; see the file COPYINGv3.
   If not, write to the Free Software Foundation,
   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

   */

#import "Foundation/NSObject.h"
#import "GNUstepBase/GSXML.h"

@class	NSDictionary;
@class	NSMutableArray;
@class	NSMutableDictionary;
@class	NSString;

@interface AGSIndex : NSObject
{
  NSMutableDictionary	*refs;
  NSString		*base;		// Not retained
  NSString		*unit;		// Not retained
  NSString		*classname;	// Not retained
  NSString		*category;	// Not retained
  unsigned		chap;
  unsigned		sect;
  unsigned		ssect;
  unsigned		sssect;
}
- (NSString*) globalRef: (NSString*)ref type: (NSString*)type;
- (void) makeRefs: (GSXMLNode*)node;
- (void) mergeRefs: (NSDictionary*)more override: (BOOL)flag;
- (void) addInformalProtocols: (NSArray *)protocolNames;
- (NSArray*) methodsInUnit: (NSString*)aUnit;
- (NSMutableDictionary*) refs;
- (void) setDirectory: (NSString*)path;
- (void) setGlobalRef: (NSString*)ref type: (NSString*)type;
- (void) setOutputs: (NSArray*)a forHeader: (NSString*)h;
- (void) setRelationship: (NSString*)r from: (NSString*)from to: (NSString*)to;
- (void) setSources: (NSArray*)a forHeader: (NSString*)h;
- (void) setUnitRef: (NSString*)ref type: (NSString*)type;
- (NSMutableArray*) outputsForHeader: (NSString*)h;
- (NSMutableArray*) sourcesForHeader: (NSString*)h;
- (NSDictionary*) unitRef: (NSString*)ref type: (NSString*)type;
- (NSString*) unitRef: (NSString*)ref type: (NSString*)type unit: (NSString**)u;
@end
#endif
