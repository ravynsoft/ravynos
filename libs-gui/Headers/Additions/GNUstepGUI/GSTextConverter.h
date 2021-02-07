/*                                                    -*-objc-*-
   GSTextConverter.h

   Define two protocols for text converter that will either read an external
   format from a file or data object into an attributed string or write out
   an attributed string in a format into a file or data object.

   Copyright (C) 2001 Free Software Foundation, Inc.

   Author: Fred Kiefer <FredKiefer@gmx.de>
   Date: August 2001

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

#ifndef _GNUstep_H_GSTextConverter
#define _GNUstep_H_GSTextConverter

#import <Foundation/NSObject.h>

@class NSAttributedString;
@class NSData;
@class NSDictionary;
@class NSError;
@class NSString;

@protocol GSTextConverter
+ (Class) classForFormat: (NSString*)format 
		producer: (BOOL)producer;
@end

@protocol GSTextProducer
+ (NSData*) produceDataFrom: (NSAttributedString*) aText
	 documentAttributes: (NSDictionary*)dict
                      error: (NSError **)error;
@end

/* 
 * The 'class' argument must be NSAttributedString (or a subclass);
 * the results of parsing will be saved into a newly created object of
 * that class, which is then returned.
 */
@protocol GSTextConsumer
+ (NSAttributedString*) parseData: (NSData *)aData 
                          options: (NSDictionary *)options
	       documentAttributes: (NSDictionary **)dict
                            error: (NSError **)error
			    class: (Class)class;
@end

#endif // _GNUstep_H_GSTextConverter
