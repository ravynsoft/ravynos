/** <title>NSPrinter</title>

   <abstract>Class representing a printer's capabilities.</abstract>

   Copyright (C) 1996, 1997, 2004 Free Software Foundation, Inc.

   Authors: Simon Frankau <sgf@frankau.demon.co.uk>
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

#include "config.h"
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSArray.h>
#import <Foundation/NSDebug.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSString.h>
#import <Foundation/NSBundle.h>
#import <Foundation/NSCharacterSet.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSEnumerator.h>
#import <Foundation/NSException.h>
#import <Foundation/NSFileManager.h>
#import <Foundation/NSPathUtilities.h>
#import <Foundation/NSScanner.h>
#import <Foundation/NSSet.h>
#import <Foundation/NSString.h>
#import <Foundation/NSValue.h>
#import <Foundation/NSMapTable.h>
#import <Foundation/NSSet.h>
#import "AppKit/AppKitExceptions.h"
#import "AppKit/NSGraphics.h"
#import "AppKit/NSPrinter.h"
#import "GNUstepGUI/GSPrinting.h"

//
// Class variables:
//

//
// Class variables used during scanning:
//

// Character sets used in scanning.
static NSCharacterSet* newlineSet = nil;
static NSCharacterSet* keyEndSet = nil;
static NSCharacterSet* optKeyEndSet = nil;
static NSCharacterSet* valueEndSet = nil;

//Class variable to cache NSPrinters, without this they 
//are created (and PPDs are parsed) ALL the time
static NSMutableDictionary* printerCache;


//
// Private methods used for PPD Parsing
//
@interface NSPrinter (PPDParsingPrivate)

-(void) loadPPDAtPath: (NSString*) PPDstring
         symbolValues: (NSMutableDictionary*) ppdSymbolValues
         inclusionSet: (NSMutableSet*) includeSet;

-(void) addPPDKeyword: (NSString*) mainKeyword
          withScanner: (NSScanner*) PPDdata
          withPPDPath: (NSString*) ppdPath;
    
-(void) addPPDUIConstraint: (NSScanner*) constraint
               withPPDPath: (NSString*) ppdPath;

-(void) addPPDOrderDependency: (NSScanner*) dependency
                  withPPDPath: (NSString*) ppdPath;

-(id) addString: (NSString*) string
         forKey: (NSString*) key
        inTable: (NSString*) table;

-(void)       addValue: (NSString*) value
   andValueTranslation: (NSString*) valueTranslation
  andOptionTranslation: (NSString*) optionTranslation
                forKey: (NSString*) key;

-(NSString*) interpretQuotedValue: (NSString*) qString;

-(int) gethex: (unichar) character;

@end



@implementation NSPrinter

//
// Class methods
//
+(void) initialize
{
  if (self == [NSPrinter class])
    {
      // Initial version
      [self setVersion:1];
    }
  printerCache = RETAIN([NSMutableDictionary dictionary]);
}

/** Load the appropriate bundle for the Printer
    (eg: GSLPRPrinter, GSCUPSPrinter).
*/
+(id) allocWithZone: (NSZone*) zone
{
  Class principalClass;

  principalClass = [[GSPrinting printingBundle] principalClass];

  if (principalClass == nil)
    return nil;
	
  return [[principalClass printerClass] allocWithZone: zone];
}


//
// Finding an NSPrinter 
// 
+(NSPrinter*) printerWithName: (NSString*) name
{
  NSEnumerator *keyEnum;
  NSString *key;
  NSPrinter *printer;
  
  //First, the cache has to be managed.
  //Take into account any deleted printers.
  keyEnum = [[printerCache allKeys] objectEnumerator];
  while ((key = [keyEnum nextObject]))
    {
      NSEnumerator *namesEnum;
      NSString *validName;
      BOOL stillValid = NO;

      namesEnum = [[self printerNames] objectEnumerator];
      while ((validName = [namesEnum nextObject]))
        {
          if ([validName isEqualToString: key])
            {
              stillValid = YES;
              break;
            }
        }
      
      if (stillValid == NO)
        {
          [printerCache removeObjectForKey: key];
        }
    }

  printer = [printerCache objectForKey: name];

  if (printer)
    {
      return printer;
    }
  else
    {
      Class principalClass;

      principalClass = [[GSPrinting printingBundle] principalClass];

      if (principalClass == nil)
        return nil;
    
      printer =  [[principalClass printerClass] printerWithName: name];

      if (printer)
        {
          [printerCache setObject: printer
                           forKey: name];
        }
      return printer;
    }
}

//
// This now different than the OpenStep spec and instead
// follows the more useful implementation Apple choosed.  In
// OpenStep, this method would read a PPD and return a NSPrinter
// based upon values from that PPD, regardless if that printer
// was actually avaiable for use or not.  On the contrary, Apple's 
// implementation looks
// at all avaiable printers and returns one that has the same
// type.  The reason for this is because they use CUPS.  CUPS
// does not work by maintaining a repository of PPDs.  Instead, 
// the CUPS server trasnmits PPDs as they are needed, and only
// for actual real printers.  Since we cannot know how the backend 
// bundles will be handling their PPDs, or if they will even be using
// PPDs for that matter, (a Win32 printing backend, for example), 
// I've choosen to go with Apple's implementation.  Really, I see
// little use in creating a NSPrinter for a printer that is not
// available for use in the first place, I am open for commments
// on this, of course.
+(NSPrinter*) printerWithType: (NSString*) type
{
  NSEnumerator *printerNamesEnum;
  NSString *printerName;
  
  printerNamesEnum = [[self printerNames] objectEnumerator];
  
  while ((printerName = [printerNamesEnum nextObject]))
    {
      NSPrinter *printer;

      printer = [self printerWithName: printerName];

      if ([[printer type] isEqualToString: type])
        {
          return printer;
        }
    }
  return nil;
}


+(NSArray*) printerNames
{
  Class principalClass;

  principalClass = [[GSPrinting printingBundle] principalClass];

  if (principalClass == nil)
    return nil;
    
  return  [[principalClass printerClass] printerNames];
}


// See note at +(NSPrinter*) printerWithType:(NSString*) type
+(NSArray*) printerTypes
{
  NSMutableSet *printerTypes;
  NSEnumerator *printerNamesEnum;
  NSString *printerName;
  NSPrinter *printer;
  
  printerTypes = [NSMutableSet setWithCapacity:1];
  
  printerNamesEnum = [[self printerNames] objectEnumerator];
  
  while ((printerName = [printerNamesEnum nextObject]))
    {
      printer = [self printerWithName: printerName];
      
      [printerTypes addObject: [printer type]];
    }
    
  return [printerTypes allObjects];
}

//
// Instance methods
//


//
// Printer Attributes 
//
-(NSString*) host
{
  return _printerHost;
}

-(NSString*) name
{
  return _printerName;
}

-(NSString*) note
{
  return _printerNote;
}

-(NSString*) type
{
  return _printerType;
}

//
// Retrieving Specific Information 
//
-(BOOL) acceptsBinary
{
  // FIXME: I'm not sure if acceptsBinary is the same as BCP protocol?
  NSString *result;
  NSScanner *protocols;

  result = [self stringForKey: @"Protocols" 
                      inTable: @"PPD"];
  if (!result)
      return NO;

  protocols = [NSScanner scannerWithString: result];

  while (![protocols isAtEnd])
    {
      [protocols scanUpToCharactersFromSet: [NSCharacterSet whitespaceCharacterSet]
                                intoString: &result];

      if ([result isEqual:@"BCP"])
	  return YES;
    }

  return NO;    
}

-(NSRect) imageRectForPaper: (NSString*) paperName
{
  NSString *key;
 
  key = [NSString stringWithFormat: @"ImageableArea/%@", paperName];

  return [self rectForKey: key
                  inTable: @"PPD"];
}

-(NSSize) pageSizeForPaper: (NSString*) paperName
{
  NSString *key;

  key = [NSString stringWithFormat: @"PaperDimension/%@", paperName];

  return [self sizeForKey: key
                  inTable: @"PPD"];
}

-(BOOL) isColor
{
  return [self booleanForKey: @"ColorDevice" 
                     inTable: @"PPD"];
}

-(BOOL) isFontAvailable: (NSString*) fontName
{
  NSString *key;

  key = [NSString stringWithFormat: @"Font/%@", fontName];
  return [self isKey: key
             inTable: @"PPD"];
}

-(int) languageLevel
{
  return [self intForKey: @"LanguageLevel" 
                 inTable: @"PPD"];
}

-(BOOL) isOutputStackInReverseOrder
{
  // FIXME: Is this what is needed? I'm not sure how this is worked out.
  NSString *result;
  
  result = [self stringForKey: @"DefaultOutputOrder" 
                      inTable: @"PPD"];

  if (!result)
      return NO;
  
  if ([result caseInsensitiveCompare: @"REVERSE"] == NSOrderedSame)
    return YES;
  else
    return NO;
}

//
// Querying the NSPrinter Tables 
//
-(BOOL) booleanForKey: (NSString*) key
              inTable: (NSString*) table
{
  NSString *result;
  result = [self stringForKey: key 
                      inTable: table];

  if (!result)  //raise exception?
    return NO;
 
  if ([result caseInsensitiveCompare: @"TRUE"] == NSOrderedSame)
    return YES;
  else
    return NO;
}


-(NSDictionary*) deviceDescription
{
  NSMutableDictionary *result;

  result = [NSMutableDictionary dictionary];
  
  if ([self isKey: @"DefaultResolution" 
          inTable:@"PPD"])
    {
      int dpi = [self intForKey: @"DefaultResolution" 
                        inTable: @"PPD"];

      [result setObject: [NSNumber numberWithInt: dpi]
                forKey: NSDeviceResolution];
    }

  if ([self isKey: @"ColorDevice" 
          inTable: @"PPD"])
    {
      BOOL color = [self booleanForKey: @"ColorDevice" 
                      inTable: @"PPD"];

      // FIXME: Should NSDeviceWhiteColorSpace be NSDeviceBlackColorSpace?
      // FIXME #2: Are they calibrated?
      // Basically I'm not sure which color spaces should be used...
      if (color == YES)
        {
          [result setObject: NSDeviceCMYKColorSpace
                     forKey: NSDeviceColorSpaceName];
        }
      else
        {
          [result setObject: NSDeviceWhiteColorSpace
                     forKey: NSDeviceColorSpaceName];
        }
    }

  if ([self isKey: @"DefaultBitsPerPixel" 
          inTable: @"PPD"])
    {
      int bits = [self intForKey: @"DefaultBitsPerPixel" 
                         inTable: @"PPD"];

      [result setObject: [NSNumber numberWithInt: bits]
                 forKey: NSDeviceBitsPerSample];
    }

  if ([self isKey: @"DefaultPageSize"
          inTable: @"PPD"])
    {
      NSString* defaultPageSize = [self stringForKey: @"DefaultPageSize"
                                             inTable: @"PPD"];

      if (defaultPageSize)
        {
          NSSize paperSize = [self pageSizeForPaper: defaultPageSize];

          [result setObject: [NSValue valueWithSize:paperSize]
                     forKey: NSDeviceSize];
        }
    }

  [result setObject: [NSNumber numberWithBool:NO]
             forKey: NSDeviceIsScreen];

  [result setObject: [NSNumber numberWithBool:YES]
             forKey: NSDeviceIsPrinter];

  NSDebugMLLog(@"GSPrinting", @"Device Description: %@", [result description]);
  return result;
}


-(float) floatForKey: (NSString*) key
             inTable: (NSString*) table
{
  NSString *result;

  result = [self stringForKey: key 
                      inTable: table];

  if (!result)  //raise exception?
    return 0.0;

  return [result floatValue];
}


-(int) intForKey: (NSString*) key
         inTable: (NSString*) table
{
  NSString *result;

  result = [self stringForKey: key 
                      inTable: table];

  if (!result) //raise exception?
    return 0;

  return [result intValue];
}


-(NSRect) rectForKey: (NSString*) key
             inTable: (NSString*) table
{
  NSString *result;
  NSScanner *bits;
  double x1, y1, x2, y2;

  result = [self stringForKey: key 
                      inTable: table];

  if (!result)  //raise exception?
    return NSZeroRect;

  bits = [NSScanner scannerWithString: result];
  if ([bits scanDouble: &x1] && 
      [bits scanDouble: &y1] &&
      [bits scanDouble: &x2] &&
      [bits scanDouble: &y2])
    {
      return NSMakeRect(x1, y1, x2-x1, y2-y1);
    }
  return NSZeroRect;
}

-(NSSize) sizeForKey: (NSString*) key
             inTable: (NSString*) table
{
  NSString *result;
  NSScanner *bits;
  double x, y;

  result = [self stringForKey: key 
                      inTable: table];

  if (!result)  //raise exception?
    return NSZeroSize;

  bits = [NSScanner scannerWithString: result];
  if ([bits scanDouble: &x] && 
      [bits scanDouble: &y])
    {
      return NSMakeSize(x,y);
    }
  return NSZeroSize;
}


-(NSString*) stringForKey: (NSString*) key
                  inTable: (NSString*) table
{
  NSArray *results;

  results = [self stringListForKey: key
                           inTable: table];

  if (results == nil)
    return nil;

  return [results objectAtIndex: 0];
}

-(NSArray*) stringListForKey: (NSString*) key
                     inTable: (NSString*) table
{
  NSDictionary *tableObj;
  NSMutableArray *result;

  tableObj = [_tables objectForKey: table ];

  if (tableObj == nil) //raise exception?
    {
      return nil;
    }

  result = [tableObj objectForKey: key];
  if ([[result objectAtIndex:0] isEqual:@""])
    {
      NSMutableArray *origResult = result;
      result = [NSMutableArray array];
      [result addObjectsFromArray: origResult];
      [result removeObjectAtIndex: 0];
    }
  return result;
}


-(NSPrinterTableStatus) statusForTable: (NSString*) table
{
  NSDictionary *tableObj;

  // Select correct table
  tableObj = [_tables objectForKey: table];

  if (tableObj == nil)
    return NSPrinterTableNotFound;
  else if (![tableObj isKindOfClass: [NSDictionary class]])
    return NSPrinterTableError;
  else
    return NSPrinterTableOK;
}


-(BOOL) isKey: (NSString*) key
      inTable: (NSString*) table
{
  NSMutableDictionary *tableObj;

  // Select correct table
  tableObj = [_tables objectForKey: table];

  if (tableObj == nil) //raise exception?
    {
      return NO;
    }

  // And check it
  if ([tableObj objectForKey: key] == nil)
    return NO;
  else
    return YES;
}

//
// NSCoding protocol
//
- (void) encodeWithCoder: (NSCoder*)aCoder
{ 
  if ([aCoder allowsKeyedCoding])
    {
      // TODO: Determine keys for NSPrinter.
    }
  else
    {
      [aCoder encodeObject: _printerHost];
      [aCoder encodeObject: _printerName];
      [aCoder encodeObject: _printerNote];
      [aCoder encodeObject: _printerType];
      [aCoder encodeObject: _tables];
    }
}

- (id) initWithCoder: (NSCoder*)aDecoder
{  
  if ([aDecoder allowsKeyedCoding])
    {
      // TODO: Determine keys for NSPrinter.
    }
  else
    {
      _printerHost = [aDecoder decodeObject];
      _printerName = [aDecoder decodeObject];
      _printerNote = [aDecoder decodeObject];
      _printerType = [aDecoder decodeObject];
      _tables = [aDecoder decodeObject];
    }
  return self;
}

@end



///
///Private implementation of routines that will be usefull
///for the printing backend bundles that subclass us.
///
@implementation NSPrinter (Private)

//
// Initialisation method used by backend bundles
//
-(id) initWithName: (NSString*) name
          withType: (NSString*) type
          withHost: (NSString*) host
          withNote: (NSString*) note
{
  self = [super init];
  
  // Initialise instance variables
  ASSIGN(_printerName, name);
  ASSIGN(_printerType, type);
  ASSIGN(_printerHost, host);
  ASSIGN(_printerNote, note);
  
  _tables = RETAIN([NSMutableDictionary dictionary]);

  return self;
}

//
// Deallocation of instance variables
//
-(void) dealloc
{
  RELEASE(_printerHost);
  RELEASE(_printerName);
  RELEASE(_printerNote);
  RELEASE(_printerType);
  RELEASE(_tables); 

  [super dealloc];
}

@end



@implementation NSPrinter (PPDParsing)

-(BOOL) parsePPDAtPath: (NSString*) ppdPath
{
  NSAutoreleasePool* subpool;
  NSMutableDictionary* ppdSymbolValues;
  NSEnumerator* objEnum;
  NSMutableArray* valArray;

  //make sure the class variables for scanning are created
  if (!newlineSet)
    {
      newlineSet = [NSCharacterSet characterSetWithCharactersInString: @"\n\r"];
      RETAIN(newlineSet);
    }

  if (!keyEndSet)
    {
      keyEndSet = [NSCharacterSet characterSetWithCharactersInString: @"\n\r\t: "];
      RETAIN(keyEndSet);
    }

  if (!optKeyEndSet)
    {
      optKeyEndSet = [NSCharacterSet characterSetWithCharactersInString: @"\n\r:/"];
      RETAIN(optKeyEndSet);
    }

  if (!valueEndSet)
    {
      valueEndSet = [NSCharacterSet characterSetWithCharactersInString: @"\n\r/"];
      RETAIN(valueEndSet);
    }



  [_tables setObject: [NSMutableDictionary dictionary]
              forKey: @"PPD"];
      
  [_tables setObject: [NSMutableDictionary dictionary]
              forKey: @"PPDOptionTranslation"];

  [_tables setObject: [NSMutableDictionary dictionary]
              forKey: @"PPDArgumentTranslation"];

  [_tables setObject: [NSMutableDictionary dictionary]
              forKey: @"PPDOrderDependency"];

  [_tables setObject: [NSMutableDictionary dictionary]
              forKey: @"PPDUIConstraints"];
  
  
  // Create a temporary autorelease pool, as many temporary objects are used
  subpool = [[NSAutoreleasePool alloc] init];


  // NB: There are some structure keywords (such as OpenUI/CloseUI) that may
  // be repeated, but as yet are not used. Since they are structure keywords,
  // they'll probably need special processing anyway, and so aren't
  // added to this list.

  // Create dictionary for temporary storage of symbol values
  ppdSymbolValues = [NSMutableDictionary dictionary];

  //The inclusion set keeps track of what PPD files have been *Include(d).
  //If one comes up twice recursion has occurred and we stop it.
  // And scan the PPD itself
  [self loadPPDAtPath: ppdPath
         symbolValues: ppdSymbolValues
         inclusionSet: [NSMutableSet setWithCapacity:10]];

  // Search the PPD dictionary for symbolvalues and substitute them.
  objEnum = [[_tables objectForKey: @"PPD"] objectEnumerator];
  while ((valArray = [objEnum nextObject]))
    {
      NSString *oldValue;
      NSString *newValue;
      int i, max;

      max = [valArray count];
      for (i=0 ; i < max ; i++)
        {
          oldValue = [valArray objectAtIndex: i];
          if ([oldValue isKindOfClass: [NSString class]] 
              && ![oldValue isEqual: @""] 
              &&  [[oldValue substringToIndex: 1] isEqual: @"^"])
              {
                newValue = [ppdSymbolValues
                             objectForKey: [oldValue substringFromIndex: 1]];

	            if (!newValue)
                    {
                      [NSException raise: NSPPDParseException
                       format: @"Unknown symbol value, ^%@ in PPD file %@.ppd",
                       oldValue, ppdPath];
                    }

                  [valArray replaceObjectAtIndex: i 
                                      withObject: newValue];
             }
        }
    }


  
  // Make sure all the required keys are present
  //Too many PPDs don't pass the test....
  /*
  objEnum = [[NSArray arrayWithObjects: @"NickName",
                  @"ModelName",
                  @"PCFileName",
                  @"Product",
                  @"PSVersion",
                  @"FileVersion",
                  @"FormatVersion",
                  @"LanguageEncoding",
                  @"LanguageVersion",
                  @"PageSize",
                  @"PageRegion",
                  @"ImageableArea",
                  @"PaperDimension",
                  @"PPD-Adobe",
                  nil] objectEnumerator];

  while ((checkVal = [objEnum nextObject]))
    {
      if (![self isKey: checkVal 
               inTable: @"PPD"])
        {
          [NSException raise:NSPPDParseException
           format:@"Required keyword *%@ not found in PPD file %@.ppd",
           checkVal, PPDPath];
        }
    }
  */

  // Release the local autoreleasePool
  [subpool drain];


//Sometimes it's good to see the tables...
/*
  NSDebugMLLog(@"GSPrinting", @"\n\nPPD: %@\n\n", 
               [[_tables objectForKey: @"PPD"] description]);

  NSDebugMLLog(@"GSPrinting", @"\n\nPPDOptionTranslation: %@\n\n", 
               [[_tables objectForKey: @"PPDOptionTranslation"] description]);

  NSDebugMLLog(@"GSPrinting", @"\n\nPPDArgumentTranslation: %@\n\n", 
               [[_tables objectForKey: @"PPDArgumentTranslation"] description]);

  NSDebugMLLog(@"GSPrinting", @"\n\nPPDOrderDependency: %@\n\n", 
               [[_tables objectForKey: @"PPDOrderDependency"] description]);

  NSDebugMLLog(@"GSPrinting", @"\n\nPPDUIConstraints: %@\n\n", 
               [[_tables objectForKey: @"PPDUIConstraints"] description]);
*/


  return YES;
}

@end





@implementation NSPrinter (PPDParsingPrivate)


-(void) loadPPDAtPath: (NSString*) ppdPath
         symbolValues: (NSMutableDictionary*) ppdSymbolValues
         inclusionSet: (NSMutableSet*) inclusionSet
{
  NSString* ppdString;
  NSScanner* ppdData;
  NSString* keyword;

  
  //See if this ppd has been processed before
  if ([inclusionSet member: ppdPath])
    {
      //this ppd has been done already!
      [NSException raise: NSPPDIncludeStackOverflowException
                         format: @"Recursive *Includes! PPD *Include stack: %@",
                         [[inclusionSet allObjects] description] ];
    }
  
  [inclusionSet addObject: ppdPath];

  ppdString = [NSString stringWithContentsOfFile: ppdPath];
  if (nil == ppdString)
    {
      // The file isn't readable
      [NSException raise: NSPPDParseException
                  format: @"PPD file '%@' isn't readable", ppdPath];
    }

  // Set up the scanner - Appending a newline means that it should be
  // able to process the last line correctly
  ppdData = [NSScanner scannerWithString:
			 [ppdString stringByAppendingString: @"\n"]];
       
  [ppdData setCharactersToBeSkipped: [NSCharacterSet whitespaceCharacterSet]];
  
  // Main processing starts here...
  while (YES)  //Only check for the end after accounting for whitespace
    {
      // Get to the start of a new keyword, skipping blank lines
      [ppdData scanCharactersFromSet: 
               [NSCharacterSet whitespaceAndNewlineCharacterSet]
                          intoString: NULL];
    
      //this could be the end...
      if ([ppdData isAtEnd])
        break;
        
      // All new entries should starts '*'
      if (![ppdData scanString: @"*" 
                    intoString: NULL])
        {
          [NSException raise: NSPPDParseException
                      format: @"Line not starting with * in PPD file %@", 
                      ppdPath];
        }

      // Skip lines starting '*%', '*End', '*SymbolLength', or '*SymbolEnd'
      if ([ppdData scanString: @"%" 
                      intoString: NULL]
          || [ppdData scanString: @"End" //if we get this there is problem, yes?
                      intoString: NULL]
          || [ppdData scanString: @"SymbolLength" 
                      intoString: NULL]
          || [ppdData scanString: @"SymbolEnd" //if we get this there is problem, yes?
                      intoString: NULL])
        {
          [ppdData scanUpToCharactersFromSet: newlineSet 
                                  intoString: NULL];
          continue;
        }
        
      // Read main keyword, up to a colon, space or newline
      [ppdData scanUpToCharactersFromSet: keyEndSet 
                              intoString: &keyword];
      
      // Loop if there is no value section, these keywords are ignored
      if ([ppdData scanCharactersFromSet: newlineSet 
                              intoString: NULL])
        {
          continue;
        }
        
      // Add the line to the relevant table
      if ([keyword isEqual: @"OrderDependency"])
        {
          [self addPPDOrderDependency: ppdData
                          withPPDPath: ppdPath];
        }
      else if ([keyword isEqual: @"UIConstraints"])
        {
          [self addPPDUIConstraint: ppdData
                       withPPDPath: ppdPath];
        }
      else if ([keyword isEqual: @"Include"])
        {
          NSFileManager *fileManager;
          NSString *fileName = nil;
          NSString *path = nil;

          fileManager = [NSFileManager defaultManager];
          
          [ppdData scanString: @":" 
                   intoString: NULL];
                  
          // Find the filename between two "s"
          [ppdData scanString: @"\""             /*"*/
                   intoString: NULL];
                  
          [ppdData scanUpToString: @"\""         /*"*/ 
                       intoString: &fileName];
                      
          [ppdData scanString: @"\""            /*"*/
                   intoString: NULL];

          //the fileName could be an absolute path or just a filename.
          if ([fileManager fileExistsAtPath: fileName])
            {
              //it was absolute, we are done
              path = fileName;
            }
          //it was not absolute.  Check to see if it exists in the 
          //directory of this ppd
          else if ([fileManager fileExistsAtPath: 
                     [[ppdPath stringByDeletingLastPathComponent] 
                       stringByAppendingPathComponent: fileName] ])
            {
              path = [[ppdPath stringByDeletingLastPathComponent] 
                      stringByAppendingPathComponent: fileName];
            }
          else  //could not find the *Include fileName
            {
              [NSException raise: NSPPDIncludeNotFoundException
                         format: @"Could not find *Included PPD file %@", path];
            }
        
          [self loadPPDAtPath: path
                 symbolValues: ppdSymbolValues 
                 inclusionSet: inclusionSet];
        }
      else if ([keyword isEqual: @"SymbolValue"])
        {
          NSString *symbolName;
          NSString *symbolVal;

          if (![ppdData scanString: @"^" 
                        intoString: NULL])
            {
              [NSException raise: NSPPDParseException
               format:@"Badly formatted *SymbolValue in PPD file %@",
               ppdPath];
            }	    

          [ppdData scanUpToString: @":" 
                       intoString: &symbolName];

                      
          [ppdData scanString: @":" 
                   intoString: NULL];
                  
          [ppdData scanString: @"\""              /*"*/
                   intoString: NULL];
                 
          [ppdData scanUpToString: @"\""          /*"*/
                       intoString: &symbolVal];
                 
          if (!symbolVal)
            symbolVal = @"";
      
          [ppdData scanString: @"\""             /*"*/
                   intoString: NULL];
             
          [ppdSymbolValues setObject: symbolVal 
                              forKey: symbolName];                         
        }
      else
        {
          [self addPPDKeyword: keyword 
                  withScanner: ppdData
                  withPPDPath: ppdPath];
        }


      // Skip any other data that don't conform with the specification.
      [ppdData scanUpToCharactersFromSet: newlineSet
			      intoString: NULL];
    }
}


-(void) addPPDKeyword: (NSString*) mainKeyword
          withScanner: (NSScanner*) ppdData
          withPPDPath: (NSString*) ppdPath
{ 
  NSArray *repKeys;
  NSString* optionKeyword = nil;
  NSString* optionTranslation = nil;
  NSString* value = nil;
  NSString* valueTranslation = nil;

  // Array of Repeated Keywords (Appendix B of the PostScript Printer
  // Description File Format Specification).
  repKeys = [NSArray arrayWithObjects:@"Emulators",
		     @"Extensions",
		     @"FaxSupport",
		   //@"Include", (handled separately)
		     @"Message",
		     @"PrinterError",
		     @"Product",
		     @"Protocols",
		     @"PSVersion",
		     @"Source",
		     @"Status",
		   //@"UIConstraints", (handled separately)
  // Even though this is not mentioned in the list of repeated keywords, 
  // it's often repeated anyway, so I'm putting it here.
		     @"InkName",
		     nil];


  // Scan off any optionKeyword
  [ppdData scanUpToCharactersFromSet: optKeyEndSet 
                          intoString: &optionKeyword];

  if ([ppdData scanCharactersFromSet: newlineSet 
                          intoString: NULL])
    {
      [NSException raise: NSPPDParseException
       format: @"Keyword has optional keyword but no value in PPD file %@",
       ppdPath];
    }

  if ([ppdData scanString: @"/" 
               intoString: NULL])
    {
      // Option keyword translation exists - scan it
      [ppdData scanUpToString: @":" 
                   intoString: &optionTranslation];
    }

  [ppdData scanString: @":" 
           intoString: NULL];

  // Read the value part
  // Values starting with a " are read until the second ", ignoring \n etc.
  
  if ([ppdData scanString: @"\""             /*"*/
               intoString: NULL])
    {
      [ppdData scanUpToString: @"\""         /*"*/
                   intoString: &value];
                   
      [ppdData scanString: @"\""             /*"*/
               intoString: NULL];
               
      // It is a QuotedValue if it's in quotes, and there is no option
      // key, or the main key is a *JCL keyword
      if (!optionKeyword || [[mainKeyword substringToIndex:3]
                               isEqualToString: @"JCL"])
        {
          value = [self interpretQuotedValue: value];
        }
    }
  else
    {
      // Otherwise, scan up to the end of line or '/'
      [ppdData scanUpToCharactersFromSet: valueEndSet 
                              intoString: &value];
    }

  if (!value)
    {
      value = @"";
    }

  // If there is a value translation, scan it
  if ([ppdData scanString: @"/" 
               intoString: NULL])
    {
      [ppdData scanUpToCharactersFromSet: newlineSet
                              intoString: &valueTranslation];
    }

  // The translations also have to have any hex substrings interpreted
  optionTranslation = [self interpretQuotedValue: optionTranslation];
  valueTranslation = [self interpretQuotedValue: valueTranslation];

  // The keyword (or keyword/option pair, if there's a option), should only
  // only have one value, unless it's one of the optionless keywords which
  // allow multiple instances.
  // If a keyword is read twice, 'first instance is correct', according to
  // the standard.
  // Finally, add the strings to the tables
  if (optionKeyword)
    {
      NSString *mainAndOptionKeyword;

      mainAndOptionKeyword=[mainKeyword stringByAppendingFormat: @"/%@",
                            optionKeyword];
               
      if ([self isKey: mainAndOptionKeyword 
              inTable: @"PPD"])
        {
          return;
        }
        
      [self             addValue: value
             andValueTranslation: valueTranslation
            andOptionTranslation: optionTranslation
                          forKey: mainAndOptionKeyword];
           
      // Deal with the oddities of stringForKey:inTable:
      // If this method is used to find a keyword with options, using
      // just the keyword it should return an empty string
      // stringListForKey:inTable:, however, should return the list of
      // option keywords.
      // This is done by making the first item in the array an empty
      // string, which will be skipped by stringListForKey:, if necessary
      if (![[_tables objectForKey: @"PPD"] objectForKey: mainKeyword])
        {
          [self addString: @"" 
                   forKey: mainKeyword 
                  inTable: @"PPD"];
                  
          [self addString: @"" 
                   forKey: mainKeyword 
                  inTable: @"PPDOptionTranslation"];
                  
          [self addString: @"" 
                   forKey: mainKeyword 
                  inTable: @"PPDArgumentTranslation"];
                  
        }
        
      [self            addValue: optionKeyword
            andValueTranslation: optionKeyword
           andOptionTranslation: optionKeyword
                         forKey: mainKeyword];
    }
  else
    {
      if ([self isKey: mainKeyword 
              inTable: @"PPD"] && 
         ![repKeys containsObject: mainKeyword])
        {
          return;
        }
        
      [self            addValue: value
            andValueTranslation: valueTranslation
           andOptionTranslation: optionTranslation
                         forKey: mainKeyword];
    }
}


-(void) addPPDUIConstraint: (NSScanner*) constraint
               withPPDPath: (NSString*) ppdPath
{
  NSString* mainKey1 = nil;
  NSString* optionKey1 = nil;
  NSString* mainKey2 = nil;
  NSString* optionKey2 = nil;

  // UIConstraint should have no option keyword
  if (![constraint scanString: @":" 
                   intoString: NULL])
    {
      [NSException raise:NSPPDParseException
       format:@"UIConstraints has option keyword in PPD File %@",
       ppdPath];
    }
    
  // Skip the '*'
  [constraint scanString: @"*" 
              intoString: NULL];
              
  // Scan the bits. Stuff not starting with * must be an optionKeyword
  [constraint scanUpToCharactersFromSet: [NSCharacterSet whitespaceCharacterSet]
                             intoString: &mainKey1];

  if (![constraint scanString: @"*" 
                   intoString: NULL])
    {
      [constraint scanUpToCharactersFromSet: [NSCharacterSet whitespaceCharacterSet]
		                     intoString: &optionKey1];
                                 
      [constraint scanString: @"*" 
                  intoString: NULL];
                  
    }
    
  [constraint scanUpToCharactersFromSet: 
              [NSCharacterSet whitespaceAndNewlineCharacterSet]
                             intoString: &mainKey2];
                            
  if (![constraint scanCharactersFromSet: newlineSet 
                              intoString: NULL])
    {
      [constraint scanUpToCharactersFromSet: 
                  [NSCharacterSet whitespaceAndNewlineCharacterSet]
                                 intoString: &optionKey2];
    }
  else
    {
      optionKey2 = @"";
    }

  // Add to table
  if (optionKey1)
    mainKey1 = [mainKey1 stringByAppendingFormat: @"/%@", optionKey1];
    
  [self addString: mainKey2
           forKey: mainKey1
          inTable: @"PPDUIConstraints"];
          
  [self addString: optionKey2
           forKey: mainKey1
          inTable: @"PPDUIConstraints"];
          
}



-(void) addPPDOrderDependency: (NSScanner*) dependency
                  withPPDPath: (NSString*) ppdPath
{
  NSString *realValue = nil;
  NSString *section = nil;
  NSString *keyword = nil;
  NSString *optionKeyword = nil;

  // Order dependency should have no option keyword
  if (![dependency scanString: @":" 
                   intoString: NULL])
    {
      [NSException raise: NSPPDParseException
       format:@"OrderDependency has option keyword in PPD file %@",
       ppdPath];
    }

  [dependency scanUpToCharactersFromSet: [NSCharacterSet whitespaceCharacterSet]
                             intoString: &realValue];
                             
  [dependency scanUpToCharactersFromSet: [NSCharacterSet whitespaceCharacterSet]
                             intoString: &section];
                             
  [dependency scanString: @"*" 
              intoString: NULL];
              
  [dependency scanUpToCharactersFromSet: 
              [NSCharacterSet whitespaceAndNewlineCharacterSet]
                             intoString: &keyword];
                            
  if (![dependency scanCharactersFromSet: newlineSet 
                              intoString: NULL])
    {
      // Optional keyword exists
      [dependency scanUpToCharactersFromSet: 
                  [NSCharacterSet whitespaceAndNewlineCharacterSet]
                                 intoString: &optionKeyword];
    }

  // Go to next line of PPD file
  [dependency scanCharactersFromSet: newlineSet 
                         intoString: NULL];
                         
  // Add to table
  if (optionKeyword)
    keyword = [keyword stringByAppendingFormat: @"/%@", optionKeyword];
    
  [self addString: realValue 
           forKey: keyword 
          inTable: @"PPDOrderDependency"];
          
  [self addString: section 
           forKey: keyword 
          inTable: @"PPDOrderDependency"];
          
}


//
// Adds the various values to the relevant tables, for the given key
//
-(void)           addValue: (NSString*) value
       andValueTranslation: (NSString*) valueTranslation
      andOptionTranslation: (NSString*) optionTranslation
                    forKey: (NSString*) key
{
  [self addString: value 
           forKey: key 
          inTable: @"PPD"];

  if (valueTranslation)
    {
      [self addString: valueTranslation 
               forKey: key
              inTable: @"PPDArgumentTranslation"];
    }

  if (optionTranslation)
    {
      [self addString: optionTranslation 
               forKey: key
              inTable: @"PPDOptionTranslation"];
    }
}

  

//
// Adds the string to the array of strings.
// Or creates the array if it does not exist and adds the string
//
-(id) addString: (NSString*) string
         forKey: (NSString*) key
        inTable: (NSString*) table
{
  NSMutableDictionary *tableObj;
  NSMutableArray *array;

  tableObj = [_tables objectForKey: table];

  if (tableObj == nil)
      NSDebugMLLog(@"GSPrinting", @"Could not find table %@!", table);

  array = (NSMutableArray*)[tableObj objectForKey:key];

  if (array == nil) //it does not exist, create it
    {
      array = [NSMutableArray array];
      [tableObj setObject: array
                   forKey: key];
    }

  [array addObject: string];

  return self;
}



// Function to convert hexadecimal substrings
-(NSString*) interpretQuotedValue: (NSString*) qString
{
  NSScanner *scanner;
  NSCharacterSet *emptySet;
  NSString *value = nil;
  NSString *part;
  int stringLength;
  int location;
  NSRange range;

  if (!qString)
    {
      return nil;
    }

  // Don't bother unless there's something to convert
  range = [qString rangeOfString: @"<"];
  if (!range.length)
    return qString;

  scanner = [NSScanner scannerWithString: qString];
  emptySet = [NSCharacterSet characterSetWithCharactersInString: @""];
  [scanner setCharactersToBeSkipped: emptySet];  

  if (![scanner scanUpToString: @"<" 
                    intoString: &value])
    {
      value = [NSString string];
    }

  stringLength = [qString length];

  while (![scanner isAtEnd]) 
    {
      [scanner scanString: @"<" 
               intoString: NULL];

      // "<<" is a valid part of a PS string
      if ([scanner scanString: @"<" 
		   intoString: NULL])
        {
	  value = [value stringByAppendingString: @"<<"];	    
	}
      else 
        {
	  [scanner scanCharactersFromSet: [NSCharacterSet whitespaceAndNewlineCharacterSet]
		   intoString: NULL];
	  
	  while (![scanner scanString: @">" 
			   intoString: NULL])
	    {
	      location = [scanner scanLocation];
	      if (location+2 > stringLength)
	        {
		  [NSException raise: NSPPDParseException
			       format: @"Badly formatted hexadecimal substring '%@' in \
                                  PPD printer file.", qString];
		  // NOT REACHED
		}
	      value = [value stringByAppendingFormat: @"%c",
			     16 * [self gethex: [qString characterAtIndex: location]]
			     + [self gethex: [qString characterAtIndex: location+1]]];
	      
	      [scanner setScanLocation: location+2];
	      
	      [scanner scanCharactersFromSet: [NSCharacterSet whitespaceAndNewlineCharacterSet]
		       intoString: NULL];
	    }
	}
      
      if ([scanner scanUpToString:@"<" intoString:&part])
        {
	  value = [value stringByAppendingString: part];
	}
    }

  return value;
}

// Convert a character to a value between 0 and 15
-(int) gethex: (unichar) character
{
  switch (character)
    {
      case '0': return 0;
      case '1': return 1;
      case '2': return 2;
      case '3': return 3;
      case '4': return 4;
      case '5': return 5;
      case '6': return 6;
      case '7': return 7;
      case '8': return 8;
      case '9': return 9;
      case 'A': return 10;
      case 'B': return 11;
      case 'C': return 12;
      case 'D': return 13;
      case 'E': return 14;
      case 'F': return 15;
      case 'a': return 10;
      case 'b': return 11;
      case 'c': return 12;
      case 'd': return 13;
      case 'e': return 14;
      case 'f': return 15;
    }      
  [NSException 
      raise: NSPPDParseException 
      format: @"Badly formatted hexadeximal character '%d' in PPD printer file.", 
      character];

  return 0; /* Quiet compiler warnings */
}

@end
