/*
   GSVbox.h

   The GSVbox class (a GNU extension)

   Copyright (C) 1999 Free Software Foundation, Inc.

   Author:  Nicola Pero <n.pero@mi.flashnet.it>
   Date: 1999

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

//
// See GSHbox.h for comments. 
// This file is generated from GSHbox.h by removing comments 
// and substituting all occurrences of "hbox" with "vbox", 
// "XResizing" with "YResizing", "MinXMargin" with "MinYMargin".
//


#ifndef _GNUstep_H_GSVbox
#define _GNUstep_H_GSVbox

#import "GSTable.h"

/** 
  <unit>
  <heading>GSVbox</heading>
 
  <p> A GSVbox is completely analogue to a GSHbox.  It implements the
  same methods; the only different is that it packs views from bottom
  to top (from top to bottom if flipped).  </p>
  </unit>
*/
@interface GSVbox: GSTable
{
  BOOL _haveViews;
  float _defaultMinYMargin;
}

//
// Initizialing.  
//
-(id) init;

//
//  Adding a View. 
//
/** See the documentation for GSHbox */
-(void) addView: (NSView *)aView;

/** See the documentation for GSHbox */
-(void) addView: (NSView *)aView
enablingYResizing: (BOOL)aFlag;

/** See the documentation for GSHbox */
-(void) addView: (NSView *)aView
 withMinYMargin: (float)aMargin;

/** See the documentation for GSHbox */
-(void) addView: (NSView *)aView
enablingYResizing: (BOOL)aFlag
 withMinYMargin: (float)aMargin;

//
// Adding a Separator. 
//
/** See the documentation for GSHbox */
-(void) addSeparator;
/** See the documentation for GSHbox */
-(void) addSeparatorWithMinYMargin: (float)aMargin;

//
//  Setting Margins.  
//

/** See the documentation for GSHbox */
-(void) setDefaultMinYMargin: (float)aMargin;

//
// Getting Number of Views
//
/** Return the number of views in the GSVbox (separators included).  */
-(int) numberOfViews;
@end

#endif /* _GNUstep_H_GSVbox */
