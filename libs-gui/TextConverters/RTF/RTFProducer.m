/*
   RTFProducer.m

   Writes out a NSAttributedString as RTF

   Copyright (C) 1999 Free Software Foundation, Inc.

   Author: Daniel Boehringer
   Date: November 1999
   Modifications: Fred Kiefer <FredKiefer@gmx.de>
   Date: June 2000
   Modifications: Axel Katerbau <axel@objectpark.org>
   Date: April 2003

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
#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>
#import <GNUstepGUI/GSHelpAttachment.h>
#import "RTFProducer.h"

// FIXME: Should be defined in a central place
#define PAPERSIZE @"PaperSize"
#define LEFTMARGIN @"LeftMargin"
#define RIGHTMARGIN @"RightMargin"
#define TOPMARGIN @"TopMargin"
#define BUTTOMMARGIN @"ButtomMargin"
#define HYPHENATIONFACTOR @"HyphenationFactor"
#define VIEWSIZE @"ViewSize"
#define VIEWZOOM @"ViewZoom"
#define VIEWMODE @"ViewMode"

#define	points2twips(a)	((int)((a) * 20.0))

@interface RTFDProducer (Private)

- (NSArray *)_attachments;
- (NSDictionary *)_attributesOfLastRun;
- (void)_setAttributesOfLastRun: (NSDictionary *)aDict;

- (NSString *)_runStringForString: (NSString *)substring
                       attributes: (NSDictionary *)attributes;

- (NSString *)_ASCIIfiedString: (NSString *)string;
- (NSString *)_headerString;
- (NSString *)_trailerString;
- (NSString *)_bodyString;
- (NSString *)_RTFDStringFromAttributedString: (NSAttributedString *)aText
                           documentAttributes: (NSDictionary *)dict
                               inlineGraphics: (BOOL)inlineGraphics;
@end

@implementation RTFDProducer

+ (NSFileWrapper *)produceFileFrom: (NSAttributedString *)aText
                documentAttributes: (NSDictionary *)dict
                             error: (NSError **)error
{
  RTFDProducer *producer;
  NSData *encodedText;
  NSFileWrapper *wrapper;

  producer = [[self alloc] init];

  encodedText = [[producer _RTFDStringFromAttributedString: aText
                                        documentAttributes: dict
                                            inlineGraphics: NO]
                   dataUsingEncoding: NSASCIIStringEncoding];

//  if ([aText containsAttachments])
  if (YES)
    {
      NSMutableDictionary *fileDict;
      NSFileWrapper *txt;
      NSEnumerator *enumerator;
      NSFileWrapper *fileWrapper;
    
      fileDict = [NSMutableDictionary dictionary];
      txt = [[NSFileWrapper alloc] initRegularFileWithContents: encodedText];

      [fileDict setObject: txt forKey: @"TXT.rtf"];
    
      RELEASE(txt);
    
      enumerator = [[producer _attachments] objectEnumerator];
      while ((fileWrapper = [enumerator nextObject]))
        {
          NSString *filename;
    
          filename = [fileWrapper filename] ? [fileWrapper filename]
                                            : [fileWrapper preferredFilename];
	  filename = [filename lastPathComponent];

          [fileDict setObject: fileWrapper forKey: filename];
        }

      wrapper = [[NSFileWrapper alloc] initDirectoryWithFileWrappers: fileDict];
    }
  else
    {
      wrapper = [[NSFileWrapper alloc]
                  initRegularFileWithContents: encodedText];
    }

  RELEASE(producer);
  return AUTORELEASE(wrapper);
}

+ (NSData *)produceDataFrom: (NSAttributedString *)aText
         documentAttributes: (NSDictionary *)dict
                      error: (NSError **)error
{
  return [[self produceFileFrom: aText
                documentAttributes: dict
                error: error] serializedRepresentation];
}

- (id)init
{
  /*
   * maintain a dictionary for the used colours
   * (for rtf-header generation)
   */
  colorDict = [[NSMutableDictionary alloc] init];
  /*
   * maintain a dictionary for the used fonts
   * (for rtf-header generation)
   */
  fontDict = [[NSMutableDictionary alloc] init];

  attachments = [[NSMutableArray alloc] init];

  ASSIGN(fgColor, [NSColor textColor]);
  ASSIGN(bgColor, [NSColor textBackgroundColor]);
  ASSIGN(ulColor, [NSColor textColor]);

  return self;
}

- (void)dealloc
{
  RELEASE(text);
  RELEASE(fontDict);
  RELEASE(colorDict);
  RELEASE(docDict);
  RELEASE(attachments);

  RELEASE(fgColor);
  RELEASE(bgColor);
  RELEASE(ulColor);

  RELEASE(_attributesOfLastRun);

  [super dealloc];
}

@end

@implementation RTFProducer

+ (NSData *)produceDataFrom: (NSAttributedString *)aText
         documentAttributes: (NSDictionary *)dict
                      error: (NSError **)error
{
  RTFProducer *producer;
  NSData *data;

  producer = [[self alloc] init];
  data = [[producer _RTFDStringFromAttributedString: aText
                                 documentAttributes: dict
                                     inlineGraphics: YES]
            dataUsingEncoding: NSASCIIStringEncoding];

  RELEASE(producer);

  return data;
}

+ (NSFileWrapper *)produceFileFrom: (NSAttributedString *)aText
                documentAttributes: (NSDictionary *)dict
                             error: (NSError **)error
{
  return AUTORELEASE([[NSFileWrapper alloc]
            initRegularFileWithContents: [self produceDataFrom: aText
                                               documentAttributes: dict
                                               error: error]]);
}

@end

@implementation RTFDProducer (Private)

- (NSArray *)_attachments
{
  return attachments;
}

- (NSDictionary *)_attributesOfLastRun
{
  return _attributesOfLastRun;
}

- (void)_setAttributesOfLastRun: (NSDictionary *)aDict;
{
  ASSIGN(_attributesOfLastRun, aDict);
}

- (NSString *)fontTable
{
  if ([fontDict count])
    {
      NSMutableString *fontlistString;
      NSEnumerator *fontEnum;
      NSString *currFont;
      NSArray	*keyArray;

      fontlistString = (NSMutableString *)[NSMutableString string];
      keyArray = [fontDict allKeys];
      keyArray = [keyArray sortedArrayUsingSelector: @selector(compare:)];

      fontEnum = [keyArray objectEnumerator];
      while ((currFont = [fontEnum nextObject]))
        {
          NSString *fontFamily;

          // ##FIXME: If we ever have more fonts to map to families, we should
          // use a dictionary
          if ([currFont isEqualToString: @"Symbol"])
            {
              fontFamily = @"tech";
            }
          else if ([currFont isEqualToString: @"Helvetica"])
            {
              fontFamily = @"swiss";
            }
          else if ([currFont isEqualToString: @"Courier"])
            {
              fontFamily = @"modern";
            }
          else if ([currFont isEqualToString: @"Times"])
            {
              fontFamily = @"roman";
            }
          else
            {
              fontFamily = @"nil";
            }

          [fontlistString appendFormat: @"%@\\f%@ %@;",
              [fontDict objectForKey: currFont], fontFamily, currFont];
        }
      return [NSString stringWithFormat: @"{\\fonttbl%@}\n", fontlistString];
    }
  else
    {
      return @"";
    }
}

- (NSString *)colorTable
{
  if ([colorDict count])
    {
      NSMutableString *result;
      unsigned int count = [colorDict count];
      id list[count];
      NSEnumerator *keyEnum;
      id next;
      int i;

      keyEnum = [colorDict keyEnumerator];

      while ((next = [keyEnum nextObject]))
        {
          NSNumber *cn;

          cn = [colorDict objectForKey: next];
          list[[cn intValue] - 1] = next;
        }

      result = (NSMutableString *)[NSMutableString
                                   stringWithString: @"{\\colortbl;"];

      for (i = 0; i < count; i++)
        {
          NSColor *color;

          color = [list[i] colorUsingColorSpaceName: NSCalibratedRGBColorSpace];

          [result appendFormat: @"\\red%d\\green%d\\blue%d;",
              (short)([color redComponent] * 255),
              (short)([color greenComponent] * 255),
              (short)([color blueComponent] * 255)];
        }

      [result appendString: @"}\n"];
      return result;
    }
  else
    {
      return @"";
    }
}

- (NSString *)documentAttributes
{
  if (docDict)
    {
      NSMutableString *result;
      NSValue *val;
      NSNumber *num;

      result = (NSMutableString *)[NSMutableString string];

      if ((val = [docDict objectForKey: PAPERSIZE]))
        {
          NSSize size = [val sizeValue];
          [result appendFormat: @"\\paperw%d\\paperh%d",
              (short)points2twips(size.width),
              (short)points2twips(size.height)];
        }

      if ((num = [docDict objectForKey: LEFTMARGIN]))
        {
          [result appendFormat: @"\\margl%d",
              (short)points2twips([num floatValue])];
        }

      if ((num = [docDict objectForKey: RIGHTMARGIN]))
        {
          [result appendFormat: @"\\margr%d",
              (short)points2twips([num floatValue])];
        }

      if ((num = [docDict objectForKey: TOPMARGIN]))
        {
          [result appendFormat: @"\\margt%d",
              (short)points2twips([num floatValue])];
        }

      if ((num = [docDict objectForKey: BUTTOMMARGIN]))
        {
          [result appendFormat: @"\\margb%d",
              (short)points2twips([num floatValue])];
        }

      if ((val = [docDict objectForKey: VIEWSIZE]))
        {
          NSSize size = [val sizeValue];
          [result appendFormat: @"\\vieww%d\\viewh%d",
              (short)points2twips(size.width),
              (short)points2twips(size.height)];
        }

      if ((num = [docDict objectForKey: VIEWZOOM]))
        {
          float factor = [num floatValue];
          [result appendFormat: @"\\viewscale%d",
              (short)factor];
        }

      if ((num = [docDict objectForKey: VIEWMODE]))
        {
          int mode = [num intValue];
          [result appendFormat: @"\\viewkind%d",
              (short)mode];
        }

      if ((num = [docDict objectForKey: HYPHENATIONFACTOR]))
        {
          [result appendFormat: @"\\hyphauto1\\hyphfactor%d",
              (short)points2twips([num floatValue]) * 5];
        }

      return result;
    }
  else
    {
      return @"";
    }
}

- (NSString *)_headerString
/*" It is essential that before this method is called the method
-_bodyString is called! "*/
{
  NSMutableString *result;

  // As 'ugly' as it seems but had to add \cocoartf to let Apple's RTF parser
  // grok paragraph spacing \saN. Should be no problem with other RTF parsers
  // as this command will be ignored. So this is for compatibility with OS X.
  result = (NSMutableString *)[NSMutableString stringWithString: 
      @"{\\rtf1\\ansi\\ansicpg1252\\cocoartf102"];

  [result appendString: [self fontTable]];
  [result appendString: [self colorTable]];
  [result appendString: [self documentAttributes]];

  return result;
}

- (NSString *)_trailerString
{
  return @"}";
}

- (NSString *)fontToken: (NSString *)fontName
{
  NSString *fCount;

  fCount = [fontDict objectForKey: fontName];

  if (! fCount)
    {
      unsigned count = [fontDict count];

      fCount = [NSString stringWithFormat: @"\\f%d", (short)count];
      [fontDict setObject: fCount forKey: fontName];
    }

  return fCount;
}

- (int)numberForColor: (NSColor *)color
{
  unsigned int cn;
  NSNumber *num;

  num = [colorDict objectForKey: color];

  if (! num)
    {
      cn = [colorDict count] + 1;

      [colorDict setObject: [NSNumber numberWithInt: cn] forKey: color];
    }
  else
    {
      cn = [num intValue];
    }

  return cn;
}

- (NSString *)paragraphStyle: (NSParagraphStyle *)paraStyle
{
  NSMutableString *result;
  int twips;

  result = (NSMutableString *)[NSMutableString stringWithString: @"\\pard"];

  if (! paraStyle)
    {
      return result;
    }

  // tabs
  {
    NSEnumerator *enumerator;
    NSTextTab *tab;

    enumerator = [[paraStyle tabStops] objectEnumerator];
    while ((tab = [enumerator nextObject]))
      {
        switch ([tab tabStopType])
          {
            case NSLeftTabStopType:
                // no tabkind emission needed
                break;
            case NSRightTabStopType:
                [result appendString: @"\\tqr"];
                break;
            case NSCenterTabStopType:
                [result appendString: @"\\tqc"];
                break;
            case NSDecimalTabStopType:
                [result appendString: @"\\tqdec"];
                break;
            default:
                NSLog(@"Unknown tab stop type.");
                break;
          }

        [result appendString: [NSString stringWithFormat: @"\\tx%d",
            (short)points2twips([tab location])]];
      }
  }

  switch ((int)[paraStyle baseWritingDirection])
    {
      case NSWritingDirectionLeftToRight:
        // default -> nothing to emit
        break;
      case NSWritingDirectionRightToLeft:
        [result appendString: @"\\rtlpar"];
        break;
      default:
        break;
    }
  
  // write first line indent and left indent
  twips = points2twips([paraStyle headIndent]);
  if (twips != 0)
    {
      [result appendFormat: @"\\li%d", (short)twips];
    }

  twips = points2twips([paraStyle firstLineHeadIndent]) - twips;
  if (twips != 0)
    {
      [result appendFormat: @"\\fi%d", (short)twips];
    }

  // right indent
  // this only works when doc attributes are given
  {
    NSNumber *rightMargin, *leftMargin;
    NSValue *paperSize;

    paperSize = [docDict objectForKey: PAPERSIZE];
    rightMargin = [docDict objectForKey: RIGHTMARGIN];
    leftMargin = [docDict objectForKey: LEFTMARGIN];

    if (paperSize && leftMargin && rightMargin)
      {
        int rightMarginTwips;
        int leftMarginTwips;
        int tailIndentTwips;
        int paperWidthTwips;

        rightMarginTwips = points2twips([rightMargin floatValue]);
        leftMarginTwips = points2twips([leftMargin floatValue]);
        tailIndentTwips = points2twips([paraStyle tailIndent]);
        paperWidthTwips = points2twips([paperSize sizeValue].width);

        [result appendFormat: @"\\ri%d",
            (short)(paperWidthTwips - rightMarginTwips
            - leftMarginTwips - tailIndentTwips)];
      }
  }
  
  twips = points2twips([paraStyle paragraphSpacing]);
  if (twips != 0)
    {
      [result appendFormat: @"\\sa%d", (short)twips];
    }

  twips = points2twips([paraStyle minimumLineHeight]);
  if (twips != 0)
    {
      [result appendFormat: @"\\sl%d", (short)twips];
    }

  twips = points2twips([paraStyle maximumLineHeight]);
  if (twips != 0)
    {
      [result appendFormat: @"\\sl-%d", (short)twips];
    }

  switch ([paraStyle alignment])
    {
      case NSRightTextAlignment:
          [result appendString: @"\\qr"];
          break;
      case NSCenterTextAlignment:
          [result appendString: @"\\qc"];
          break;
      case NSLeftTextAlignment:
          [result appendString: @"\\ql"];
          break;
      case NSJustifiedTextAlignment:
          [result appendString: @"\\qj"];
          break;
      default:
          [result appendString: @"\\ql"];
          break;
    }

  return result;
}

- (NSString *)font: (NSFont *)font
{
  NSString *fontName;
  NSFontTraitMask traits, traitsOfLastRun;
  NSMutableString *result;
  NSFont *fontOfLastRun;

  fontOfLastRun = [[self _attributesOfLastRun]
                         objectForKey: NSFontAttributeName];

  result = (NSMutableString *)[NSMutableString string];

  // name
  fontName = [font familyName];
  if ((! fontOfLastRun) || (! [fontName isEqualToString: 
      [fontOfLastRun familyName]]))
    {
      [result appendString: [self fontToken: fontName]];
    }

  // size
  if ((! fontOfLastRun) || ([font pointSize] != [fontOfLastRun pointSize]))
    {
      [result appendFormat: @"\\fs%d", (short)(int)([font pointSize] * 2)];
    }

  // traits
  traits = [[NSFontManager sharedFontManager] traitsOfFont: font];
  traitsOfLastRun = [[NSFontManager sharedFontManager] traitsOfFont: 
      fontOfLastRun];

  if ((traits & NSItalicFontMask) != (traitsOfLastRun & NSItalicFontMask))
    {
      if (traits & NSItalicFontMask)
        {
          [result appendString: @"\\i"];
        }
      else
        {
          [result appendString: @"\\i0"];
        }
    }

  if ((traits & NSBoldFontMask) != (traitsOfLastRun & NSBoldFontMask))
    {
      if (traits & NSBoldFontMask)
        {
          [result appendString: @"\\b"];
        }
      else
        {
          [result appendString: @"\\b0"];
        }
    }
  return result;
}

- (NSString *)_removeAttributesString: (NSDictionary *)attributesToRemove
{
  NSMutableString *result;
  NSEnumerator *enumerator;
  NSString *attributeName;

  result = (NSMutableString *)[NSMutableString string];

  enumerator = [attributesToRemove keyEnumerator];
  while ((attributeName = [enumerator nextObject]))
    {
      if ([attributeName isEqualToString: NSParagraphStyleAttributeName])
        {
          [result appendString: @"\\pard\\ql"];
        }
      else if ([attributeName isEqualToString: NSForegroundColorAttributeName])
        {
          [result appendString: @"\\cf0"];
        }
      else if ([attributeName isEqualToString: NSBackgroundColorAttributeName])
        {
          [result appendString: @"\\cb0"];
        }
      else if ([attributeName isEqualToString: NSUnderlineStyleAttributeName])
        {
          [result appendString: @"\\ulnone"];
        }
      else if ([attributeName isEqualToString: NSSuperscriptAttributeName])
        {
          [result appendString: @"\\nosupersub"];
        }
      else if ([attributeName isEqualToString: NSBaselineOffsetAttributeName])
        {
          NSNumber *value = [[self _attributesOfLastRun] objectForKey: 
              NSBaselineOffsetAttributeName];
          int svalue = (int)[value floatValue];

          if (svalue >= 0)
            {
              [result appendString: @"\\up0"];
            }
          else if (svalue < 0)
            {
              [result appendString: @"\\dn0"];
            }
        }
      else if ([attributeName isEqualToString: NSLigatureAttributeName])
        {
          [result appendString: @"\\zwnj"];
        }
      else if ([attributeName isEqualToString: NSAttachmentAttributeName])
        {
        }
      else if ([attributeName isEqualToString: NSFontAttributeName])
        {
        }
      else if ([attributeName isEqualToString: NSLinkAttributeName])
	{
	  [result appendString: @"}}"];
	}
      else
        {
          NSLog(@"(removal) Missing handling of '%@' attributes.",
                attributeName);
          // ##TODO: attributes missing e.g. NSKernAttributeName
        }
    }

  return result;
}

- (NSString *)_stringWithRTFCharacters: (NSString *)string
{
  NSString *result;
  NSMutableData *resultData;
  unichar *buffer;
  int length, i;
  BOOL uc_flagged = NO;

  if (string == nil)
    {
      return nil;
    }
  
  length = [string length];
  buffer = NSZoneCalloc([self zone], length, sizeof(unichar));
  [string getCharacters: buffer];
  resultData = [[NSMutableData alloc]
                               initWithCapacity: (int)(length * 1.2)];

  for (i = 0; i < length; i++)
    {
      unichar c;

      c = buffer[i];
      if (c < 0x80)
        {
          // encoding found
          char ansiChar;

          ansiChar = (char)c;

          switch (ansiChar)
            {
              case '\\':
                  [resultData appendBytes: "\\\\" length: 2];
                  break;
              case '\n':
                  [resultData appendBytes: "\\par\n" length: 5];
                  break;
              case '\t':
                  [resultData appendBytes: "\\tab " length: 5];
                  break;
              case '{':
                  [resultData appendBytes: "\\{" length: 2];
                  break;
              case '}':
                  [resultData appendBytes: "\\}" length: 2];
                  break;
              case '`':
                  [resultData appendBytes: "\\lquote " length: 8];
                  break;
              case '\'':
                  [resultData appendBytes: "\\rquote " length: 8];
                  break;
              default:
                  [resultData appendBytes: &ansiChar length: 1];
                  break;                  
            }
        }
      else if (c < 0xFF)
        {
          char unicodeCommand[16];
          
          snprintf(unicodeCommand, 16, "\\'%X", (short)c);
          unicodeCommand[15] = '\0';

          [resultData appendBytes: unicodeCommand
                           length: strlen(unicodeCommand)];
	}
      else if (c == NSAttachmentCharacter)
        {
          [resultData appendBytes: "\\'AC}" length: 5];
        }
      else
        {
          // write unicode encoding
          char unicodeCommand[16];

          if (!uc_flagged)
            {
              // We don't supply an ANSI representation for Unicode characters
              [resultData appendBytes: "\\uc0 " length: 5];
              uc_flagged = YES;
            }

          snprintf(unicodeCommand, 16, "\\u%d ", (short)c);
          unicodeCommand[15] = '\0';

          [resultData appendBytes: unicodeCommand
                           length: strlen(unicodeCommand)];       
        }
    }

  NSZoneFree([self zone], buffer);
  result = AUTORELEASE([[NSString alloc] initWithData: resultData
					 encoding: NSASCIIStringEncoding]);
  RELEASE(resultData);

  return result;
}

- (NSString *)_ASCIIfiedString: (NSString *)string;
{
    // workaround:converting non-ASCII chars
    NSData *lossyConversion;

    lossyConversion = [string dataUsingEncoding: NSASCIIStringEncoding
                           allowLossyConversion: YES];

    return AUTORELEASE([[NSString alloc] initWithData: lossyConversion
					 encoding: NSASCIIStringEncoding]);
}

- (NSString *)_addAttributesString: (NSDictionary *)attributesToAdd
{
  NSMutableString *result;
  NSEnumerator *enumerator;
  NSString *attributeName;

  result = (NSMutableString *)[NSMutableString string];

  enumerator = [attributesToAdd keyEnumerator];
  while ((attributeName = [enumerator nextObject]))
    {
      if ([attributeName isEqualToString: NSParagraphStyleAttributeName])
        {
          [result appendString: [self paragraphStyle:
              [attributesToAdd objectForKey: NSParagraphStyleAttributeName]]];
        }
      else if ([attributeName isEqualToString: NSFontAttributeName])
        {
          [result appendString: [self font:
              [attributesToAdd objectForKey: NSFontAttributeName]]];
        }
      else if ([attributeName isEqualToString: NSForegroundColorAttributeName])
        {
          NSColor *color = [attributesToAdd objectForKey: 
              NSForegroundColorAttributeName];
          if (! [color isEqual: fgColor])
            {
              [result appendFormat: @"\\cf%d",
                  (short)[self numberForColor: color]];
            }
        }
      else if ([attributeName isEqualToString: NSBackgroundColorAttributeName])
        {
          NSColor *color = [attributesToAdd objectForKey: 
              NSBackgroundColorAttributeName];

          if (! [color isEqual: bgColor])
            {
              [result appendFormat: @"\\cb%d",
                  (short)[self numberForColor: color]];
            }
        }
      else if ([attributeName isEqualToString: NSUnderlineColorAttributeName])
        {
          NSColor *color = [attributesToAdd objectForKey: 
              NSUnderlineColorAttributeName];

          [result appendFormat: @"\\ulc%d",
              (short)[self numberForColor: color]];
        }
      else if ([attributeName isEqualToString: NSUnderlineStyleAttributeName])
        {
	  NSInteger styleMask = [[attributesToAdd objectForKey:
              NSUnderlineStyleAttributeName] integerValue];

	  if ((styleMask & NSUnderlineByWordMask) == NSUnderlineByWordMask)
	    {
	      [result appendString: @"\\ulw"];
	    }

          if (styleMask == NSUnderlineStyleNone)
            {
              [result appendString: @"\\ulnone"];
            }
	  else if ((styleMask & NSUnderlineStyleDouble) == NSUnderlineStyleDouble)
	    {
	      [result appendString: @"\\uldb"];
	    }
	  else if ((styleMask & NSUnderlineStyleThick) == NSUnderlineStyleThick)
	    {
	      if ((styleMask & NSUnderlinePatternDot) == NSUnderlinePatternDot)
		{
		  [result appendString: @"\\ulthd"];
		}
	      else if ((styleMask & NSUnderlinePatternDash) == NSUnderlinePatternDash)
		{
		  [result appendString: @"\\ulthdash"];
		}
	      else if ((styleMask & NSUnderlinePatternDashDot) == NSUnderlinePatternDashDot)
		{
		  [result appendString: @"\\ulthdashd"];
		}
	      else if ((styleMask & NSUnderlinePatternDashDotDot) == NSUnderlinePatternDashDotDot)
		{
		  [result appendString: @"\\ulthdashdd"];
		}
	      else // Assume NSUnderlinePatternSolid
		{
		  [result appendString: @"\\ulth"];
		}
	    }
	  else // Assume NSUnderlineStyleSingle
	    {
	      if ((styleMask & NSUnderlinePatternDot) == NSUnderlinePatternDot)
		{
		  [result appendString: @"\\uld"];
		}
	      else if ((styleMask & NSUnderlinePatternDash) == NSUnderlinePatternDash)
		{
		  [result appendString: @"\\uldash"];
		}
	      else if ((styleMask & NSUnderlinePatternDashDot) == NSUnderlinePatternDashDot)
		{
		  [result appendString: @"\\uldashd"];
		}
	      else if ((styleMask & NSUnderlinePatternDashDotDot) == NSUnderlinePatternDashDotDot)
		{
		  [result appendString: @"\\uldashdd"];
		}
	      else // Assume NSUnderlinePatternSolid
		{
		  [result appendString: @"\\ul"];
		}
	    }
        }
      else if ([attributeName isEqualToString: NSSuperscriptAttributeName])
        {
          NSNumber *value = [attributesToAdd objectForKey: 
              NSSuperscriptAttributeName];
          int svalue = [value intValue];

          if (svalue > 0)
            {
              [result appendString: @"\\super"];
              if (svalue > 1)
                {
                  [result appendFormat: @"%d", (short)svalue];
                }
            }
          else if (svalue < 0)
            {
              [result appendString: @"\\sub"];
              if (svalue < -1)
                {
                  [result appendFormat: @"%d", (short)-svalue];
                }
            }
        }
      else if ([attributeName isEqualToString: NSBaselineOffsetAttributeName])
        {
          NSNumber *value = [attributesToAdd objectForKey: 
              NSBaselineOffsetAttributeName];
          int svalue = (int)([value floatValue] * 2);

          if (svalue > 0)
            {
              [result appendFormat: @"\\up%d", (short)svalue];
            }
          else if (svalue < 0)
            {
              [result appendFormat: @"\\dn%d", (short)-svalue];
            }
        }
      else if ([attributeName isEqualToString: NSLigatureAttributeName])
        {
          [result appendString: @"\\zwj"];
        }
      else if ([attributeName isEqualToString: NSAttachmentAttributeName])
        {
          NSTextAttachment *attachment;

          if ((attachment = [attributesToAdd objectForKey: 
              NSAttachmentAttributeName]))
            {
              NSFileWrapper *attachmentFileWrapper;
              NSString *attachmentFilename;
              NSSize cellSize;

	      if ([attachment isKindOfClass: [GSHelpLinkAttachment class]])
		{
		  GSHelpLinkAttachment *link =
		    (GSHelpLinkAttachment *)attachment;

		  [result appendString: @"{{\\NeXTHelpLink"];
		  [result appendString: @" \\markername "];
		  [result appendString: @";\\linkFilename "];
		  [result appendString: [link fileName]];
		  [result appendString: @";\\linkMarkername "];
		  [result appendString: [link markerName]];
		  [result appendString: @";}"];
		}
	      else if ([attachment
			 isKindOfClass: [GSHelpMarkerAttachment class]])
		{
		  GSHelpMarkerAttachment *marker =
		    (GSHelpMarkerAttachment *)attachment;

		  [result appendString: @"{{\\NeXTHelpMarker"];
		  [result appendString: @" \\markername "];
		  [result appendString: [marker markerName]];
		  [result appendString: @";}"];
		}
	      else
		{
		  attachmentFileWrapper = [attachment fileWrapper];
		  attachmentFilename = [attachmentFileWrapper filename];
		  if (! attachmentFilename)
		    {
		      attachmentFilename =
			[attachmentFileWrapper preferredFilename];

		      if (! attachmentFilename)
			{
			  // If we do not have a proper filename, we set it
			  // here, incrementing the number of unnamed attachment
			  // so we do not overwrite existing ones.

			  // FIXME: setting the filename extension to tiff
			  // is not that great, but as we anyway append
			  // \NeXTGraphic just after it makes sense... (without
			  // the extension the file is not loaded) 

			  attachmentFilename =
			    [NSString stringWithFormat:
					@"__unnamed_file_%d.tiff",
					unnamedAttachmentCounter++];
			  [attachmentFileWrapper
			    setPreferredFilename: attachmentFilename];
			}
		    }

		  /*
		  if ([attachmentFilename respondsToSelector: 
		         @selector(fileSystemRepresentation)])
		    {
		      const char *fileSystemRepresentation;

		      fileSystemRepresentation =
			[attachmentFilename fileSystemRepresentation];

		      attachmentFilename =
			[self _encodedFilenameRepresentation:
				fileSystemRepresentation];
		    }
		  else
		    */
		    {
		      attachmentFilename =
			[self _ASCIIfiedString: attachmentFilename];
		    }

		  cellSize = [[attachment attachmentCell] cellSize];

		  [result appendString: @"{{\\NeXTGraphic "];
		  [result appendString: [attachmentFilename lastPathComponent]];
		  [result appendFormat: @" \\width%d \\height%d}",
			  (short)points2twips(cellSize.width),
			  (short)points2twips(cellSize.height)];

		  [attachmentFileWrapper setFilename: attachmentFilename];
		  [attachmentFileWrapper
		    setPreferredFilename: attachmentFilename];
		  if (attachmentFileWrapper)
		    [attachments addObject: attachmentFileWrapper];
		}
            }
        }
      else if ([attributeName isEqualToString: NSLinkAttributeName])
	{
	  id dest = [attributesToAdd objectForKey:
              NSLinkAttributeName];

	  NSString *destString = @"";

	  if ([dest isKindOfClass: [NSURL class]])
	    {
	      destString = [dest absoluteString];
	    }
	  else if ([dest isKindOfClass: [NSString class]])
	    {
	      destString = [[NSURL URLWithString: dest] absoluteString];
	    }

	  // NOTE: RTF control characters (backslash, curly braces)
	  // will be escaped by -absoluteString, so the result is safe to 
	  // concatenate into the RTF stream

	  [result appendString: 
		    [NSString stringWithFormat: @"{\\field{\\*\\fldinst HYPERLINK \"%@\"}{\\fldrslt ", destString]];
	}
      else
        {
          NSLog(@"(addition) Missing handling of '%@' attributes.",
                attributeName);
          // ##TODO: attributes missing e.g. NSKernAttributeName
        }
    }
  return result;
}

- (NSString *)_runStringForString: (NSString *)string
                       attributes: (NSDictionary *)attributes
{
  NSMutableString *result;
  NSMutableDictionary *attributesToAdd, *attributesToRemove;
  NSEnumerator *enumerator;
  NSString *attributeName;

  result = (NSMutableString *)[NSMutableString stringWithCapacity:
                                            [string length] + 15];
  attributesToAdd = [[NSMutableDictionary alloc] init];
  attributesToRemove = [[self _attributesOfLastRun] mutableCopy];

  // calculation of deltas
  enumerator = [attributes keyEnumerator];
  while ((attributeName = [enumerator nextObject]))
    {
      id attributeValue;
      id attributeValueOfLastRun;

      attributeValue = [attributes objectForKey: attributeName];
      attributeValueOfLastRun =
          [attributesToRemove objectForKey: attributeName];

      if (attributeValueOfLastRun)
        {
          if ([attributeValueOfLastRun isEqual: attributeValue])
            {
              [attributesToRemove removeObjectForKey: attributeName];
            }
          else
            {
              [attributesToAdd setObject: attributeValue forKey: attributeName];
            }
        }
      else
        {
          [attributesToAdd setObject: attributeValue forKey: attributeName];
        }
    }

  [result appendString: [self _removeAttributesString: attributesToRemove]];
  [result appendString: [self _addAttributesString: attributesToAdd]];
  RELEASE(attributesToRemove);
  RELEASE(attributesToAdd);

  if ([result length])
    {
      unichar c = [result characterAtIndex: [result length] - 1];

      if ((c != '}') && (c != ' '))
        {
          // ensure delimiter
          [result appendString: @" "];
        }
    }

  [result appendString: [self _stringWithRTFCharacters: string]];

  return result;
}

- (NSString *)_bodyString
{
  NSString *string;
  NSMutableString *result;
  unsigned length;
  NSRange effectiveRange;

  string = [text string];
  result = (NSMutableString *)[NSMutableString string];
  length = [string length];
  effectiveRange = NSMakeRange(0, 0);

  while (effectiveRange.location < length)
    {
      NSDictionary *attributes;
      CREATE_AUTORELEASE_POOL(pool);

      attributes = [text attributesAtIndex: effectiveRange.location
                     longestEffectiveRange: &effectiveRange
                                   inRange: NSMakeRange(effectiveRange.location,
                                                       length
                                                    - effectiveRange.location)];

      [result appendString: [self _runStringForString:
                           [string substringWithRange: effectiveRange]
                                           attributes: attributes]];

      effectiveRange = NSMakeRange(NSMaxRange(effectiveRange), 0);

      [self _setAttributesOfLastRun: attributes];

      [pool drain];
    }

  [self _setAttributesOfLastRun: nil]; // cleanup, should be unneccessary

  return result;
}

- (NSString *)_RTFDStringFromAttributedString: (NSAttributedString *)aText
                           documentAttributes: (NSDictionary *)dict
                               inlineGraphics: (BOOL)inlineGraphics
{
  NSMutableString *output;
  NSString *headerString;
  NSString *trailerString;
  NSString *bodyString;

  ASSIGN(text, aText);
  ASSIGN(docDict, dict);

  output = (NSMutableString *)[NSMutableString string];
  _inlineGraphics = inlineGraphics;
  /*
   * do not change order! (esp. body has to be generated first; builds context)
   */
  bodyString = [self _bodyString];
  trailerString = [self _trailerString];
  headerString = [self _headerString];

  [output appendString: headerString];
  [output appendString: bodyString];
  [output appendString: trailerString];

  return output;
}

@end
