/* 
   NSPrintInfo.m

   Stores information used in printing

   Copyright (C) 1996,1997,2004 Free Software Foundation, Inc.

   Author:  Simon Frankau <sgf@frankau.demon.co.uk>
   Date: July 1997
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

#import <Foundation/NSBundle.h>
#import <Foundation/NSCoder.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSValue.h>
#import "AppKit/NSPrinter.h"
#import "AppKit/NSPrintInfo.h"
#import "GNUstepGUI/GSPrinting.h"
#import "GSGuiPrivate.h"

// Class variables:
static NSPrintInfo *sharedPrintInfo = nil;

/**
  <unit>
  <heading>Class Description</heading>
  <p>
  NSPrintInfo is a storage object that stores information that describes
  how a view is to printed and the destination information for printing.
  </p>
  </unit>
*/

@implementation NSPrintInfo

//
// Class methods
//
+ (void) initialize
{
  if (self == [NSPrintInfo class])
    {
      // Initial version
      [self setVersion: 1];
    }
}

/** Load the appropriate bundle for the PrintInfo
    (eg: GSLPRPrintInfo, GSCUPSPrintInfo).
*/
+ (id) allocWithZone: (NSZone*)zone
{
  Class principalClass;

  principalClass = [[GSPrinting printingBundle] principalClass];

  if (principalClass == nil)
    return nil;
	
  return [[principalClass printInfoClass] allocWithZone: zone];
}

//
// Managing the Shared NSPrintInfo Object 
//
+ (void) setSharedPrintInfo: (NSPrintInfo*)printInfo
{
  ASSIGN(sharedPrintInfo, printInfo);
}

+ (NSPrintInfo*) sharedPrintInfo
{
  if (!sharedPrintInfo)
   {    
     sharedPrintInfo = [[NSPrintInfo alloc] initWithDictionary: nil]; 
   }
  return sharedPrintInfo;
}

//
// Managing the Printing Rectangle 
//
+ (NSSize) sizeForPaperName: (NSString*)name
{
  return [[self defaultPrinter] pageSizeForPaper: name];
}

//
// Specifying the Printer 
//
+ (NSPrinter*) defaultPrinter
{
  Class principalClass;

  principalClass = [[GSPrinting printingBundle] principalClass];

  if (principalClass == nil)
    return nil;
	
  return [[principalClass printInfoClass] defaultPrinter];
}

+ (void) setDefaultPrinter: (NSPrinter*)printer
{
  Class principalClass;

  principalClass = [[GSPrinting printingBundle] principalClass];

  if (principalClass == nil)
    return;
	
  [[principalClass printInfoClass] setDefaultPrinter: printer];
}

//
// Instance methods
//
//
// Creating and Initializing an NSPrintInfo Instance 
//
- (id) initWithDictionary: (NSDictionary*)aDict
{
  NSPrinter *printer;
  NSString *pageSize;
  NSRect imageRect;
  NSSize paperSize;

  if (!(self = [super init]))
    {
      return self;
    }
  
  _info = [[NSMutableDictionary alloc] init];
      
  // put in the defaults
  [self setVerticalPagination: NSAutoPagination];
  [self setHorizontalPagination: NSClipPagination];
  [self setJobDisposition: NSPrintSpoolJob];
  [self setHorizontallyCentered: NO];
  [self setVerticallyCentered: NO];

  printer = [NSPrintInfo defaultPrinter];
  [self setPrinter: printer];

  /* Set up other defaults from the printer object */
  pageSize = [printer stringForKey: @"DefaultPageSize" 
                           inTable: @"PPD"];
                      
  /* FIXME: Need to check for AutoSelect and probably a million other things... */
  if (pageSize == nil)
    pageSize = @"A4";
  
  [self setPaperName: pageSize];
  
  /* Set default margins. */
  paperSize = [printer pageSizeForPaper: pageSize];
  imageRect = [printer imageRectForPaper: pageSize];
  [self setRightMargin: (paperSize.width - NSMaxX(imageRect))];
  [self setLeftMargin: imageRect.origin.y];
  [self setTopMargin: (paperSize.height - NSMaxY(imageRect))];
  [self setBottomMargin: imageRect.origin.x];
  [self setOrientation: NSPortraitOrientation];  
  
  if (aDict != nil)
    {
      [_info addEntriesFromDictionary: aDict];
  
      if ([[_info objectForKey: NSPrintPrinter] isKindOfClass: [NSString class]])
        {
          NSString *printerName;
      
          printerName = [_info objectForKey: NSPrintPrinter];
          printer = [NSPrinter printerWithName: printerName];
          
          [self setPrinter: printer];
        }
    }
  return self;
}

- (void) dealloc
{
  RELEASE(_info);
  [super dealloc];
}

- (id) copyWithZone: (NSZone*)z
{
  NSPrintInfo *new = (NSPrintInfo *)NSCopyObject(self, 0, z);

  new->_info = [_info mutableCopyWithZone: z];

  return new;
}

//
// Managing the Printing Rectangle 
//
- (CGFloat) bottomMargin
{
  return [(NSNumber *)[_info objectForKey: NSPrintBottomMargin] doubleValue];
}

- (CGFloat) leftMargin
{
  return [(NSNumber *)[_info objectForKey: NSPrintLeftMargin] doubleValue];
}

- (NSPrintingOrientation) orientation
{
  return [(NSNumber *)[_info objectForKey: NSPrintOrientation] intValue];
}

- (NSString*) paperName
{
  return [_info objectForKey: NSPrintPaperName];
}

- (NSSize) paperSize
{
  /* Don't simplify this. Some OSs can't handle returning a NULL value into
     a struct.  */
  NSValue *val = [_info objectForKey: NSPrintPaperSize];

  if (val == nil)
    return NSMakeSize(0,0);
  return [val sizeValue];
}

- (CGFloat) rightMargin
{
  return [(NSNumber *)[_info objectForKey: NSPrintRightMargin] doubleValue];
}

- (void) setBottomMargin: (CGFloat)value
{
  [_info setObject: [NSNumber numberWithDouble: value]
            forKey: NSPrintBottomMargin];
}

- (void) setLeftMargin: (CGFloat)value
{
  [_info setObject: [NSNumber numberWithDouble: value]
            forKey: NSPrintLeftMargin];
}

- (void) setOrientation: (NSPrintingOrientation)mode
{
  NSSize size;

  [_info setObject: [NSNumber numberWithInt: mode]
            forKey: NSPrintOrientation];
		
  /* Set the paper size accordingly */
  size = [self paperSize];
  if ((mode == NSPortraitOrientation && size.width > size.height)
      || (mode == NSLandscapeOrientation && size.width < size.height))
    {
      CGFloat tmp = size.width;
      size.width = size.height;
      size.height = tmp;
      [_info setObject: [NSValue valueWithSize: size] 
                forKey: NSPrintPaperSize];
    }
}

- (void) setPaperName: (NSString*)name
{
  [_info setObject: name
         forKey: NSPrintPaperName];
  // FIXME: Should this change the orientation?
  [_info setObject: [NSValue valueWithSize: 
                                 [NSPrintInfo sizeForPaperName: name]]
         forKey: NSPrintPaperSize];
}

- (void) setPaperSize: (NSSize)size
{
  NSPrintingOrientation orient;
  [_info setObject: [NSValue valueWithSize: size]
            forKey: NSPrintPaperSize];
		
  // Set orientation accordingly
  if (size.width <= size.height)
    orient = NSPortraitOrientation;
  else
    orient = NSLandscapeOrientation;
  [_info setObject: [NSNumber numberWithInt: orient]
         forKey: NSPrintOrientation];
}

- (void) setRightMargin: (CGFloat)value
{
  [_info setObject:[NSNumber numberWithDouble:value]
            forKey:NSPrintRightMargin];
}

- (void) setTopMargin: (CGFloat)value
{
  [_info setObject:[NSNumber numberWithDouble:value]
            forKey:NSPrintTopMargin];
}

- (CGFloat) topMargin
{
  return [(NSNumber *)[_info objectForKey:NSPrintTopMargin] doubleValue];
}

- (NSRect) imageablePageBounds
{
  NSRect pageBounds;
  NSPrinter *printer;

  printer = [self printer];
  if (printer)
    {
      NSPrintingOrientation mode;

      mode = [self orientation];
      pageBounds = [printer imageRectForPaper: [self paperName]];
      if ((NSPortraitOrientation == mode
           && pageBounds.size.width > pageBounds.size.height)
          || (NSLandscapeOrientation == mode
              && pageBounds.size.width < pageBounds.size.height))
        {
          CGFloat tmp;

          tmp = pageBounds.origin.x;
          pageBounds.origin.x = pageBounds.origin.y;
          pageBounds.origin.y = tmp;
          tmp = pageBounds.size.width;
          pageBounds.size.width = pageBounds.size.height;
          pageBounds.size.height = tmp;
        }
    }
  else
    {
      pageBounds.origin = NSMakePoint(0, 0);
      pageBounds.size = [self paperSize];
    }
     
  return pageBounds;
}

- (NSString*) localizedPaperName
{
  return NSLocalizedString([self paperName], @"paper name");
}

//
// Pagination 
//
- (NSPrintingPaginationMode) horizontalPagination
{
  return [(NSNumber*)[_info objectForKey: NSPrintHorizontalPagination] intValue];
}

- (void) setHorizontalPagination: (NSPrintingPaginationMode)mode
{
  [_info setObject: [NSNumber numberWithInt:mode]
            forKey: NSPrintHorizontalPagination];
}

- (void)setVerticalPagination: (NSPrintingPaginationMode)mode
{
  [_info setObject: [NSNumber numberWithInt:mode]
            forKey: NSPrintVerticalPagination];
}

- (NSPrintingPaginationMode) verticalPagination
{
  return [(NSNumber *)[_info objectForKey: NSPrintVerticalPagination] intValue];
}

//
// Positioning the Image on the Page 
//
- (BOOL) isHorizontallyCentered
{
  return [(NSNumber *)[_info objectForKey: NSPrintHorizontallyCentered] boolValue];
}

- (BOOL) isVerticallyCentered
{
  return [(NSNumber *)[_info objectForKey: NSPrintVerticallyCentered] boolValue];
}

- (void) setHorizontallyCentered: (BOOL)flag
{
  [_info setObject: [NSNumber numberWithBool: flag]
            forKey: NSPrintHorizontallyCentered];
}

- (void) setVerticallyCentered: (BOOL)flag
{
  [_info setObject: [NSNumber numberWithBool: flag]
            forKey: NSPrintVerticallyCentered];
}

//
// Specifying the Printer 
//
- (NSPrinter*) printer
{
  return [_info objectForKey: NSPrintPrinter];
}

- (void) setPrinter: (NSPrinter*)aPrinter
{
  if (aPrinter)
    {
      [_info setObject: aPrinter 
             forKey: NSPrintPrinter];
      // FIXME: Remove features not supported by the new printer
    }
  else
    {
      [_info removeObjectForKey: NSPrintPrinter];
      // FIXME: Should we reset the default printer?
    }
}

//
// Controlling Printing
//
- (NSString*) jobDisposition
{
  return [_info objectForKey: NSPrintJobDisposition];
}

- (void) setJobDisposition: (NSString*)disposition
{
  [_info setObject: disposition 
            forKey: NSPrintJobDisposition];
}

- (void) setUpPrintOperationDefaultValues
{
  // [self subclassResponsibility: _cmd];
}

//
// Accessing the NSPrintInfo Object's Dictionary 
//
- (NSMutableDictionary*) dictionary
{
  return _info;
}

//
// NSCoding protocol
//
- (void) encodeWithCoder: (NSCoder*)aCoder
{
  NSMutableDictionary *dict;
  NSString *printerName;

  dict = [_info mutableCopy];
  printerName = [[self printer] name];

  if (printerName)
    {
      [dict setObject: printerName
            forKey: NSPrintPrinter];
    }
  else
    {
      [dict removeObjectForKey: NSPrintPrinter];
    }
           
  if ([aCoder allowsKeyedCoding])
    {
      [aCoder encodeObject: dict forKey: @"NSAttributes"];
    }
  else
    {
      [aCoder encodePropertyList: dict];
    }
  RELEASE(dict);
}

- (id) initWithCoder: (NSCoder*)aDecoder
{
  NSMutableDictionary *dict;
 
  if ([aDecoder allowsKeyedCoding])
    {
      dict = [aDecoder decodeObjectForKey: @"NSAttributes"];
    }
  else
    {
      dict = [aDecoder decodePropertyList];
    }
  return [self initWithDictionary: dict];
}

@end
