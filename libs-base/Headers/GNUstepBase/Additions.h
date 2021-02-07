/*
   Global include file for the GNUstep Base Additions Library.

   Copyright (C) 2010 Free Software Foundation, Inc.

   Written by:  Richard Frith-Macdonald <rfm@gnu.org>
   Date: Feb 2010
   
   This file is part of the GNUstep Base Library.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
   
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.
   
   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free
   Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02111 USA.
   */ 

#ifndef __Additions_h_GNUSTEP_BASE_INCLUDE
#define __Additions_h_GNUSTEP_BASE_INCLUDE

#import	<GNUstepBase/GSVersionMacros.h>
#import	<GNUstepBase/GNUstep.h>


#if !(defined(NeXT_RUNTIME) || defined(Apple_RUNTIME))
#import	<GNUstepBase/GSBlocks.h>
#endif
#import	<GNUstepBase/GSFunctions.h>
#import	<GNUstepBase/GSLocale.h>
#import	<GNUstepBase/GSLock.h>
#import	<GNUstepBase/GSMime.h>
#import	<GNUstepBase/GSXML.h>
#import	<GNUstepBase/Unicode.h>

#import	<GNUstepBase/NSArray+GNUstepBase.h>
#import	<GNUstepBase/NSAttributedString+GNUstepBase.h>
#import	<GNUstepBase/NSBundle+GNUstepBase.h>
#import	<GNUstepBase/NSCalendarDate+GNUstepBase.h>
#import	<GNUstepBase/NSData+GNUstepBase.h>
#import	<GNUstepBase/NSDebug+GNUstepBase.h>
#import	<GNUstepBase/NSFileHandle+GNUstepBase.h>
#import	<GNUstepBase/NSLock+GNUstepBase.h>
#import	<GNUstepBase/NSMutableString+GNUstepBase.h>
#import	<GNUstepBase/NSNetServices+GNUstepBase.h>
#import	<GNUstepBase/NSNumber+GNUstepBase.h>
#import	<GNUstepBase/NSObject+GNUstepBase.h>
#import	<GNUstepBase/NSProcessInfo+GNUstepBase.h>
#import	<GNUstepBase/NSStream+GNUstepBase.h>
#import	<GNUstepBase/NSString+GNUstepBase.h>
#import	<GNUstepBase/NSTask+GNUstepBase.h>
#import	<GNUstepBase/NSThread+GNUstepBase.h>
#import	<GNUstepBase/NSURL+GNUstepBase.h>

#endif /* __Additions_h_GNUSTEP_BASE_INCLUDE */
