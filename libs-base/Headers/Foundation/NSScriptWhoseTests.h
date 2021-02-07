/*
   Global include file for the GNUstep Base Library.

   Copyright (C) 1997 Free Software Foundation, Inc.

   Date: Sep 2012
   
   This file is part of the GNUstep Base Library.

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

#ifndef __NSScriptWhoseTests_h_GNUSTEP_BASE_INCLUDE
#define __NSScriptWhoseTests_h_GNUSTEP_BASE_INCLUDE

#import <Foundation/NSObject.h>

@interface NSObject (NSComparisonMethods)
- (BOOL) doesContain: (id) object;
- (BOOL) isCaseInsensitiveLike: (id) object;
- (BOOL) isEqualTo: (id) object;
- (BOOL) isGreaterThan: (id) object;
- (BOOL) isGreaterThanOrEqualTo: (id) object;
- (BOOL) isLessThan: (id) object;
- (BOOL) isLessThanOrEqualTo: (id) object;
- (BOOL) isLike: (NSString *)object;
- (BOOL) isNotEqualTo: (id) object;
@end

#endif /* __NSScriptWhoseTests_h_GNUSTEP_BASE_INCLUDE */
