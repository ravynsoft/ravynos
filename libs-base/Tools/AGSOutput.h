#ifndef	_INCLUDED_AGSOUTPUT_H
#define	_INCLUDED_AGSOUTPUT_H
/**

   <title>AGSOutput ... a class to output gsdoc source</title>
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

#include "Foundation/NSObject.h"

@class	NSArray;
@class	NSCharacterSet;
@class	NSDictionary;
@class	NSMutableArray;

@interface	AGSOutput : NSObject
{
  NSDictionary		*info;		// Not retained.
  NSCharacterSet	*identifier;	// Legit char in identifier
  NSCharacterSet	*identStart;	// Legit initial char of identifier
  NSCharacterSet	*spaces;	// All blank characters
  NSCharacterSet	*spacenl;	// Blanks excluding newline
  NSArray		*args;		// Not retained.
  NSMutableArray	*informalProtocols; // built up as discovered
  BOOL			verbose;
  BOOL			warn;
}

- (NSString*) checkComment: (NSString*)comment
		      unit: (NSString*)unit
		      info: (NSMutableDictionary*)d;
- (unsigned) fitWords: (NSArray*)a
		 from: (unsigned)start
		   to: (unsigned)end
	      maxSize: (unsigned)limit
	       output: (NSMutableString*)buf;
- (NSArray*) output: (NSMutableDictionary*)d;
- (void) outputDecl: (NSMutableDictionary*)d
	       kind: (NSString*)kind
		 to: (NSMutableString*)str;
- (void) outputFunction: (NSMutableDictionary*)d to: (NSMutableString*)str;
- (void) outputInstanceVariable: (NSMutableDictionary*)d
			     to: (NSMutableString*)str
			    for: (NSString*)unit;
- (void) outputMacro: (NSMutableDictionary*)d
		  to: (NSMutableString*)str;
- (void) outputMethod: (NSMutableDictionary*)d
		   to: (NSMutableString*)str
		  for: (NSString*)unit;
- (void) outputUnit: (NSMutableDictionary*)d to: (NSMutableString*)str;
- (unsigned) reformat: (NSString*)str
	   withIndent: (unsigned)ind
		   to: (NSMutableString*)buf;
- (NSArray*) split: (NSString*)str;
- (NSArray*) informalProtocols;
@end
#endif
