/** <title>GSPrintOperation</title>

   <abstract>Controls generation of print jobs.  Backends determine the 
             file format and how to send the data to the printer.</abstract>

   Copyright (C) 1996,2004 Free Software Foundation, Inc.

   Author:  Scott Christley <scottc@net-community.com>
   Date: 1996
   Author: Fred Kiefer <FredKiefer@gmx.de>
   Date: November 2000
   Updated to new specification
   Author: Adam Fedor <fedor@gnu.org>
   Date: Oct 2001
   Modified for Printing Backend Support
   Author: Chad Hardin <cehardin@mac.com>
   Date: June 2004

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

#ifndef _GNUstep_H_GSPrintOperation
#define _GNUstep_H_GSPrintOperation

#import "AppKit/NSPrintOperation.h"


@class NSView;
@class NSPrintInfo;


@interface GSPrintOperation: NSPrintOperation
{
}

- (id) initWithView:(NSView *)aView	
		      printInfo:(NSPrintInfo *)aPrintInfo;

- (BOOL) _deliverSpooledResult;
@end


#endif // _GNUstep_H_GSPrintOperation

