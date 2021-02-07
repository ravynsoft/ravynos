#ifndef	_INCLUDED_AGSHTML_H
#define	_INCLUDED_AGSHTML_H
/** 

   <title>AGSHtml ... a class to output html for a gsdoc file</title>
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
#import "AGSIndex.h"

@class	NSMutableString;
@class	NSString;

@interface AGSHtml : NSObject
{
  NSString		*project;
  AGSIndex		*localRefs;
  AGSIndex		*globalRefs;
  AGSIndex		*projectRefs;
  NSMutableString	*indent;
  NSString		*base;	// Not retained
  NSString		*unit;	// Not retained
  NSString		*category;	// Not retained
  NSString		*classname;	// Not retained
  NSString		*heading;	// Not retained
  NSString		*nextFile;	// Not retained
  NSString		*prevFile;	// Not retained
  NSString		*upFile;	// Not retained
  unsigned		chap;
  unsigned		sect;
  unsigned		ssect;
  unsigned		sssect;
  BOOL			isContentsDoc;
  BOOL			ivarsAtEnd;
}
- (void) decIndent;
- (void) incIndent;
- (NSString*) makeAnchor: (NSString*)r
		  ofType: (NSString*)t
		    name: (NSString*)n;
- (NSString*) makeLink: (NSString*)r
		ofType: (NSString*)t
		 isRef: (BOOL)f;
- (NSString*) makeLink: (NSString*)r
		ofType: (NSString*)t
		inUnit: (NSString*)u
		 isRef: (BOOL)f;
- (void) outputIndex: (NSString*)type
	       scope: (NSString*)scope
	       title: (NSString*)title
	       style: (NSString*)style
              target: (NSString*)target
                  to: (NSMutableString*)buf;
- (NSString*) outputDocument: (GSXMLNode*)node;
- (void) outputNode: (GSXMLNode*)node to: (NSMutableString*)buf;
- (void) outputNodeList: (GSXMLNode*)node to: (NSMutableString*)buf;
- (GSXMLNode*) outputBlock: (GSXMLNode*)node
			to: (NSMutableString*)buf
		    inPara: (BOOL)flag;
- (GSXMLNode*) outputList: (GSXMLNode*)node to: (NSMutableString*)buf;
- (GSXMLNode*) outputText: (GSXMLNode*)node to: (NSMutableString*)buf;
- (void) outputUnit: (GSXMLNode*)node to: (NSMutableString*)buf;
- (void) outputVersion: (NSDictionary*)prop to: (NSMutableString*)buf;
- (NSString*) protocolRef: (NSString*)t;
- (void) setGlobalRefs: (AGSIndex*)r;
- (void) setLocalRefs: (AGSIndex*)r;
- (void) setProjectRefs: (AGSIndex*)r;
- (void) setInstanceVariablesAtEnd: (BOOL)val;
- (NSString*) typeRef: (NSString*)t;
@end
#endif
