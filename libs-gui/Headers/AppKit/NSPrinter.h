/* 
   NSPrinter.h

   Class representing a printer's capabilities.

   Copyright (C) 1996, 1997, 2004 Free Software Foundation, Inc.

   Authors:  Simon Frankau <sgf@frankau.demon.co.uk>
   Date: June 1997
   Modified for Printing Backend Support
   Author: Chad Hardin <cehardin@mac.com>
   Date: July 2004
   
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

#ifndef _GNUstep_H_NSPrinter
#define _GNUstep_H_NSPrinter

#import <Foundation/NSObject.h>
#import <Foundation/NSGeometry.h>

@class NSString;
@class NSArray;
@class NSDictionary;
@class NSMutableDictionary;

typedef enum _NSPrinterTableStatus {
  NSPrinterTableOK,
  NSPrinterTableNotFound,
  NSPrinterTableError
} NSPrinterTableStatus;

@interface NSPrinter : NSObject <NSCoding>
{
  NSString *_printerHost;
  NSString *_printerName;
  NSString *_printerNote;
  NSString *_printerType;
  
  //The way openstep handled NSPrinter was odd, it had a concept of "real" 
  //printers and
  //not real printers.  Printers that were not real were simply parsed PPDs, 
  //whether the printer was actually available or not was irrevelatn.  This is
  //is where the all the -types and +types methods come from.  Apple's
  //adoption of CUPS forced them to change this behavior, now, their types 
  //methods only search for the types of printers that are actually avaiable, 
  //it has nothing to do with the PPDs that happen to be avaiable on the file
  //system.  I think this is also the behavior we should adopt.  Not simply 
  //because it is what Apple does but because it facilitates a generic printing
  //backend system, especially one that will likely be using CUPS itself.
  //This is why the following ivar is removed and the behavior of this class
  //and the GSLPR Bundle has been changed.  Doing this has also allowed me
  //to put a lot of generic code in ths class that would have otherwise had
  //to have been handles by each bundle.  See +printerWithType: in NSPrinter.m
  //BOOL _isRealPrinter;

  NSMutableDictionary *_tables; //maps the tables, which are 
                                //NSMutableDictionaries, to their names 
                               //(such as PPD, PPDOptionTranslation, etc)
}

//
// Finding an NSPrinter 
//
+ (NSPrinter*) printerWithName: (NSString*) name;

+ (NSPrinter*) printerWithType: (NSString*) type;

+ (NSArray*) printerNames;

+ (NSArray*) printerTypes;

//
// Printer Attributes 
//
- (NSString*) host;

- (NSString*) name;

- (NSString*) note;

- (NSString*) type;

//
// Retrieving Specific Information 
//
- (BOOL) acceptsBinary;

- (NSRect) imageRectForPaper: (NSString*) paperName;

- (NSSize) pageSizeForPaper: (NSString*) paperName;

- (BOOL) isColor;

- (BOOL) isFontAvailable: (NSString*) fontName;

- (int) languageLevel;

- (BOOL) isOutputStackInReverseOrder;


//
// Querying the NSPrinter Tables 
//
- (BOOL) booleanForKey: (NSString*) key
               inTable: (NSString*) table;

- (NSDictionary*) deviceDescription;

- (float) floatForKey: (NSString*) key
              inTable: (NSString*) table;

- (int) intForKey: (NSString*) key
          inTable: (NSString*) table;

- (NSRect) rectForKey: (NSString*) key
              inTable: (NSString*) table;

- (NSSize) sizeForKey: (NSString*) key
              inTable: (NSString*) table;

- (NSString*) stringForKey: (NSString*) key
                   inTable: (NSString*) table;

- (NSArray*) stringListForKey: (NSString*) key
                      inTable: (NSString*) table;

- (NSPrinterTableStatus) statusForTable: (NSString*) table;

- (BOOL) isKey: (NSString*) key
       inTable: (NSString*) table;

//
// NSCoding protocol
//
- (void) encodeWithCoder: (NSCoder*) aCoder;

- (id)initWithCoder: (NSCoder*) aDecoder;

@end


//
// Private methods that will be usefull for the
// printing backend bundles that subclass NSPrinter
//
@interface NSPrinter (Private)
//
// Initialisation method used by backend bundles
//
-(id) initWithName: (NSString*) name
          withType: (NSString*) type
          withHost: (NSString*) host
          withNote: (NSString*) note;

@end 



//
// Have the NSPrinter parse a PPD and put it into its 
// tables.  Used by the printing backend bundles.
//
@interface NSPrinter (PPDParsing)

-(BOOL) parsePPDAtPath: (NSString*) ppdPath;

@end

#endif // _GNUstep_H_NSPrinter
