/** <title>GSPrinting</title>

   <abstract>Loads the proper bundle for the printing backend and
   contains the class GSPrintingPrincipleClass, which is subclassed
   by printing backend bundles.</abstract>

   Copyright (C) 2004 Free Software Foundation, Inc.

   Author:  Chad Elliott Hardin <cehardin@mac.com>
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

#ifndef _GNUstep_H_GSPrinting
#define _GNUstep_H_GSPrinting

#import <Foundation/NSObject.h>

@class NSBundle;

@interface GSPrinting : NSObject
{
}

+(NSBundle*) printingBundle;

@end


@interface GSPrintingPrincipalClass : NSObject
{
}

+(Class) pageLayoutClass;

+(Class) printInfoClass;

+(Class) printOperationClass;

+(Class) printPanelClass;

+(Class) printerClass;

+(Class) gsPrintOperationClass;

@end


#endif // _GNUstep_H_GSPrinting
