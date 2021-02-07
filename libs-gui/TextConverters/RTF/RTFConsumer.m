/* attributedStringConsumer.m

   Copyright (C) 1999 Free Software Foundation, Inc.

   Author:  Stefan Böhringer (stefan.boehringer@uni-bochum.de)
   Date: Dec 1999
   Author: Fred Kiefer <FredKiefer@gmx.de>
   Date: June 2000

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
#import "RTFConsumer.h"
#import "RTFConsumerFunctions.h"
#import "RTFProducer.h"

/*  we have to satisfy the scanner with a stream reading function */
typedef struct {
  const char	*string;
  int		position;
  int		length;
} StringContext;

static void	
initStringContext (StringContext *ctxt, NSData *data)
{
  ctxt->string = [data bytes];
  ctxt->position = 0;
  ctxt->length = [data length];
}

static int	
readString (StringContext *ctxt)
{
  return ctxt->position < ctxt->length ? ctxt->string[ctxt->position++] : EOF;
}

// Hold the attributs of the current run
@interface RTFAttribute: NSObject <NSCopying>
{
@public
  BOOL changed;
  BOOL tabChanged;
  NSMutableParagraphStyle *paragraph;
  NSColor *fgColour;
  NSColor *bgColour;
  NSColor *ulColour;
  NSString *fontName;
  float fontSize;
  BOOL bold;
  BOOL italic;
  NSInteger underline;
  NSInteger strikethrough;
  int script;

  float real_fi, real_li;
}

- (NSFont*) currentFont;
- (NSNumber*) script;
- (NSNumber*) underline;
- (NSNumber*) strikethrough;
- (void) resetParagraphStyle;
- (void) resetFont;
- (void) addTab: (float)location  type: (NSTextTabType)type;

@end

@implementation RTFAttribute

- (id) init
{
  [self resetFont];
  [self resetParagraphStyle];

  return self;
}

- (void) dealloc
{
  RELEASE(paragraph);
  RELEASE(fontName);
  RELEASE(fgColour);
  RELEASE(bgColour);
  RELEASE(ulColour);
  [super dealloc];
}

- (id) copyWithZone: (NSZone*)zone
{
  RTFAttribute *new =  (RTFAttribute *)NSCopyObject (self, 0, zone);

  new->paragraph = [paragraph mutableCopyWithZone: zone];
  RETAIN(new->fontName);
  RETAIN(new->fgColour);
  RETAIN(new->bgColour);
  RETAIN(new->ulColour);

  return new;
}

- (NSFont*) currentFont
{
  NSFont *font;
  NSFontTraitMask traits = 0;
  int weight;

  if (bold)
    {
      weight = 9;
      traits |= NSBoldFontMask;
    }
  else
    {
      weight = 5;
      traits |= NSUnboldFontMask;
    }

  if (italic)
    {
      traits |= NSItalicFontMask;
    }
  else
    {
      traits |= NSUnitalicFontMask;
    }

  font = [[NSFontManager sharedFontManager] fontWithFamily: fontName
					    traits: traits
					    weight: weight
					    size: fontSize];
  if (font == nil)
    {
      /* Before giving up and using a default font, we try if this is
       * not the case of a font with a composite name, such as
       * 'Helvetica-Light'.  In that case, even if we don't have
       * exactly an 'Helvetica-Light' font family, we might have an
       * 'Helvetica' one.  */
      NSRange range = [fontName rangeOfString:@"-"];

      if (range.location != NSNotFound)
	{
	  NSString *fontFamily = [fontName substringToIndex: range.location];

	  font = [[NSFontManager sharedFontManager] fontWithFamily: fontFamily
						    traits: traits
						    weight: weight
						    size: fontSize];
	}
      
      if (font == nil)
	{
	  NSDebugMLLog(@"RTFParser", 
		       @"Could not find font %@ size %f traits %d weight %d", 
		       fontName, fontSize, traits, weight);

	  /* Last resort, default font.  :-(  */
	  font = [NSFont userFontOfSize: fontSize];
	}
    }
  
  return font;
}

- (NSNumber*) script
{
  return [NSNumber numberWithInt: script];
}

- (NSNumber*) underline
{
  if (underline != NSUnderlineStyleNone)
    return [NSNumber numberWithInteger: underline];
  else
    return nil;
}

- (NSNumber*) strikethrough
{
  if (strikethrough != NSUnderlineStyleNone)
    return [NSNumber numberWithInteger: strikethrough];
  else
    return nil;
}

- (void) resetParagraphStyle
{
  DESTROY(paragraph);
  paragraph = [[NSParagraphStyle defaultParagraphStyle] mutableCopy];
  real_fi = real_li = 0.0;

  tabChanged = NO;
  changed = YES;
}

- (void) resetFont
{
  NSFont *font = [NSFont userFontOfSize:12];

  ASSIGN(fontName, [font familyName]);
  fontSize = 12.0;
  italic = NO;
  bold = NO;

  underline = NSUnderlineStyleNone;
  strikethrough = NSUnderlineStyleNone;
  script = 0;
  DESTROY(fgColour);
  DESTROY(bgColour);
  DESTROY(ulColour);

  changed = YES;
}

- (void) addTab: (float) location  type: (NSTextTabType) type
{
  NSTextTab *tab = [[NSTextTab alloc] initWithType: NSLeftTabStopType 
				      location: location];

  if (!tabChanged)
    {
      NSArray *a;
      a = [[NSArray alloc] initWithObjects: tab, nil];
      // remove all tab stops
      [paragraph setTabStops: a];
      DESTROY(a);
      tabChanged = YES;
    }
  else
    {
      [paragraph addTabStop: tab];
    }

  changed = YES;
  RELEASE(tab);
}

@end

static BOOL classInheritsFromNSMutableAttributedString (Class c)
{
  Class mutable = [NSMutableAttributedString class];
  
  while (c != Nil)
    {
      if (c == mutable)
	{
	  return YES;
	}
      c = [c superclass];
    }
  return NO;
}

@interface RTFConsumer (Private)

- (NSAttributedString*) parseRTF: (NSData *)rtfData 
	      documentAttributes: (NSDictionary **)dict
			   class: (Class)class;
- (NSDictionary*) documentAttributes;

- (RTFAttribute*) attr;
- (void) push;
- (void) pop;
- (void) appendString: (NSString*)string;
- (void) appendHelpLink: (NSString*)fileName marker: (NSString *)markerName;
- (void) appendHelpMarker: (NSString*)markerName;
- (void) appendField: (int)start
         instruction: (NSString*)instruction;
- (void) appendImage: (NSString*) string;
- (void) reset;
- (void) setEncoding: (NSStringEncoding)anEncoding;
@end

@implementation RTFConsumer

/* RTFConsumer is the principal class and thus implements this */
+ (Class) classForFormat: (NSString *)format producer: (BOOL)flag
{
  Class cClass = Nil;

  if (flag)
    {
      if (([format isEqual: NSRTFDTextDocumentType]) ||
          ([format isEqual: @"com.apple.rtfd"]) ||
          ([format isEqual: @"rtfd"]))
	{
	  cClass = [RTFDProducer class];
	}
      else if (([format isEqual: NSRTFTextDocumentType]) ||
               ([format isEqual: @"public.rtf"]) ||
               ([format isEqual: @"rtf"]))
	{
	  cClass = [RTFProducer class];
	}
    }
  else
    {
      if (([format isEqual: NSRTFDTextDocumentType]) ||
          ([format isEqual: @"com.apple.rtfd"]) ||
          ([format isEqual: @"rtfd"]))
	{
	  cClass = [RTFDConsumer class];
	}
      else if (([format isEqual: NSRTFTextDocumentType]) ||
               ([format isEqual: @"public.rtf"]) ||
               ([format isEqual: @"rtf"]))
	{
	  cClass = [RTFConsumer class];
	}
    }
  return cClass;
}

+ (NSAttributedString*) parseFile: (NSFileWrapper *)wrapper
                          options: (NSDictionary *)options
	       documentAttributes: (NSDictionary **)dict
                            error: (NSError **)error
			    class: (Class)class
{
  NSAttributedString *text = nil;

  if ([wrapper isRegularFile])
    {
      RTFConsumer *consumer = [RTFConsumer new];
      text = [consumer parseRTF: [wrapper regularFileContents]
		       documentAttributes: dict
		       class: class];
      RELEASE(consumer);
    }
  else if ([wrapper isDirectory])
    {
      NSDictionary *files = [wrapper fileWrappers];
      NSFileWrapper *contents;
      RTFDConsumer* consumer = [RTFDConsumer new];

      if ((contents = [files objectForKey: @"TXT.rtf"]) != nil)
	{
	  [consumer setFiles: files];
	  text = [consumer parseRTF: [contents regularFileContents]
			   documentAttributes: dict
			   class: class];
	}
      RELEASE(consumer);
    }
  

  return text;
}

+ (NSAttributedString*) parseData: (NSData *)rtfData 
                          options: (NSDictionary *)options
	       documentAttributes: (NSDictionary **)dict
                            error: (NSError **)error
			    class: (Class)class
{
  RTFConsumer *consumer = [RTFConsumer new];
  NSAttributedString *text;

  text = [consumer parseRTF: rtfData
		   documentAttributes: dict
		   class: class];
  RELEASE(consumer);

  return text;
}

- (id) init
{
  ignore = 0;  
  result = nil;
  encoding = NSISOLatin1StringEncoding;
  documentAttributes = nil;
  fonts = nil;
  attrs = nil;
  colours = nil;
  _class = Nil;

  return self;
}

- (void) dealloc
{
  RELEASE(fonts);
  RELEASE(attrs);
  RELEASE(colours);
  RELEASE(result);
  RELEASE(documentAttributes);
  [super dealloc];
}

@end


@implementation RTFDConsumer

- (id) init
{
  self = [super init];

  files = nil;

  return self;
}

- (void) reset
{
  [super reset];
  [documentAttributes setValue: NSRTFDTextDocumentType
			forKey: NSDocumentTypeDocumentAttribute];
}

- (void) dealloc
{
  RELEASE(files);
  [super dealloc];
}

- (void) setFiles: (NSDictionary*) theFiles
{
  ASSIGN (files, theFiles);
}

+ (NSAttributedString*) parseData: (NSData *)rtfData 
                          options: (NSDictionary *)options
	       documentAttributes: (NSDictionary **)dict
                            error: (NSError **)error
			    class: (Class)class
{
  NSAttributedString *str;
  NSFileWrapper *wrapper = [[NSFileWrapper alloc] 
			     initWithSerializedRepresentation: rtfData];
  
  str = [self parseFile: wrapper 
              options: options 
              documentAttributes: dict 
              error: error 
              class: class];
  RELEASE (wrapper);

  return str;
}

- (void) appendImage: (NSString*)string
{
  int  oldPosition = [result length];
  NSRange insertionRange = NSMakeRange(oldPosition,0);

  if (!ignore)
    {
      NSString* fileName = [string stringByTrimmingCharactersInSet:
                         [NSCharacterSet whitespaceAndNewlineCharacterSet]];
      NSFileWrapper* wrapper = [files objectForKey: fileName];
      
      if (wrapper != nil)
        {
          NSImage* image = [[NSImage alloc] initWithData: [wrapper regularFileContents]];
          NSTextAttachment* attachment;
          RTFAttribute* attr = [self attr];
          NSMutableDictionary* attributes = nil;
          NSMutableAttributedString* str = nil;

          if (image != nil)
            {
              [wrapper setIcon: image];
            }
          attachment = [[NSTextAttachment alloc] initWithFileWrapper: wrapper];
          if (attachment == nil)
            {
              NSLog(@"No attachment at %d", oldPosition);
              RELEASE(image);
              return;
            }
        
          attributes = [[NSMutableDictionary alloc]
			 initWithObjectsAndKeys:
			   [attr currentFont], NSFontAttributeName,
			   attr->paragraph, NSParagraphStyleAttributeName,
			   nil];
          
          str = (NSMutableAttributedString*) [NSMutableAttributedString 
                         attributedStringWithAttachment: attachment];

          [str addAttributes: attributes range: NSMakeRange (0, [str length])];
          
          [result replaceCharactersInRange: insertionRange withAttributedString: str];
          attr->changed = YES;
          RELEASE(attributes);
          RELEASE(attachment);
          RELEASE(image);
        }
    }
}

@end

@implementation RTFConsumer (Private)

- (NSDictionary*) documentAttributes
{
  RETAIN(documentAttributes);
  return AUTORELEASE(documentAttributes);
}

- (void) reset
{
  RTFAttribute *attr = [RTFAttribute new];

  ignore = 0;  
  DESTROY(result);
  
  if (classInheritsFromNSMutableAttributedString (_class))
    {
      result = [[_class alloc] init];
    }
  else
    {
      result = [[NSMutableAttributedString alloc] init];
    }
  ASSIGN(documentAttributes, [NSMutableDictionary dictionary]);
  [documentAttributes setValue: NSRTFTextDocumentType
			forKey: NSDocumentTypeDocumentAttribute];
  ASSIGN(fonts, [NSMutableDictionary dictionary]);
  ASSIGN(attrs, [NSMutableArray array]);
  ASSIGN(colours, [NSMutableArray array]);
  [attrs addObject: attr];
  RELEASE(attr);
}

- (void) setEncoding: (NSStringEncoding)anEncoding
{
  encoding = anEncoding;
}

- (RTFAttribute*) attr
{
  return [attrs lastObject];
}

- (void) push
{
  RTFAttribute *attr = [[attrs lastObject] copy];

  [attrs addObject: attr];
  RELEASE(attr);
}

- (void) pop
{
  [attrs removeLastObject];
  ((RTFAttribute*)[attrs lastObject])->changed = YES;
}

- (NSAttributedString*) parseRTF: (NSData *)rtfData 
	      documentAttributes: (NSDictionary **)dict
			   class: (Class)class
{
  CREATE_AUTORELEASE_POOL(pool);
  RTFscannerCtxt scanner;
  StringContext stringCtxt;

  // We read in the first few characters to find out which
  // encoding we have
  if ([rtfData length] < 10)
    {
      // Too short to be an RTF
      return nil;
    }

  // Reset this RFTConsumer, as it might already have been used!
  _class = class;
  [self reset];

  initStringContext(&stringCtxt, rtfData);
  lexInitContext(&scanner, &stringCtxt, (int (*)(void*))readString);
  [result beginEditing];
  NS_DURING
    GSRTFparse((void *)self, &scanner);
  NS_HANDLER
    NSLog(@"Problem during RTF Parsing: %@", 
	  [localException reason]);
  //[localException raise];
  NS_ENDHANDLER
  [result endEditing];

  RELEASE(pool);
  // document attributes
  if (dict)
    {
      *dict = [self documentAttributes];
    }

  if (classInheritsFromNSMutableAttributedString (_class))
    {
      RETAIN (result);
      AUTORELEASE (result);
      return result;
    }
  else
    {
      return AUTORELEASE ([[_class alloc] initWithAttributedString: result]);
    }
}

- (void) appendString: (NSString*)string
{
  int  oldPosition = [result length];
  int  textlen = [string length]; 
  NSRange insertionRange = NSMakeRange(oldPosition,0);
  NSMutableDictionary *attributes;

  if (!ignore && textlen)
    {
      RTFAttribute* attr = [self attr];
      [result replaceCharactersInRange: insertionRange 
	      withString: string];

      if (attr->changed)
        {
	  NSParagraphStyle *ps = [attr->paragraph copy];
	  attributes = [[NSMutableDictionary alloc]
			 initWithObjectsAndKeys:
			   [attr currentFont], NSFontAttributeName,
			   ps, NSParagraphStyleAttributeName,
			   nil];
	  DESTROY(ps);
	  if ([attr underline])
	    {
	      [attributes setObject: [attr underline]
			  forKey: NSUnderlineStyleAttributeName];
	    }
	  if ([attr strikethrough])
	    {
	      [attributes setObject: [attr strikethrough]
			  forKey: NSStrikethroughStyleAttributeName];
	    }
	  if (attr->script)
	    {
	      [attributes setObject: [attr script]
			  forKey: NSSuperscriptAttributeName];
	    }
	  if (attr->fgColour != nil)
	    {
	      [attributes setObject: attr->fgColour 
			  forKey: NSForegroundColorAttributeName];
	    }
	  if (attr->bgColour != nil)
	    {
	      [attributes setObject: attr->bgColour 
			  forKey: NSBackgroundColorAttributeName];
	    }
	  if (attr->ulColour != nil)
	    {
	      [attributes setObject: attr->ulColour 
			  forKey: NSUnderlineColorAttributeName];
	    }
  
	  [result setAttributes: attributes 
		  range: NSMakeRange(oldPosition, textlen)];
	  DESTROY(attributes);
	  attr->changed = NO;
	}
    }
}

- (void) appendHelpLink: (NSString*)fileName marker: (NSString*)markerName
{
  int  oldPosition = [result length];
  NSRange insertionRange = NSMakeRange(oldPosition,0);

  if (!ignore)
    {
      GSHelpLinkAttachment* attachment;
      RTFAttribute* attr = [self attr];
      NSMutableDictionary* attributes = nil;
      NSMutableAttributedString* str = nil;

      attachment =
	[[GSHelpLinkAttachment alloc]
	  initWithFileName: fileName
		markerName: markerName];
      if (attachment == nil)
	{
	  NSLog(@"No attachment at %d", oldPosition);
	  return;
	}
        
      attributes = [[NSMutableDictionary alloc]
		     initWithObjectsAndKeys:
		       [attr currentFont], NSFontAttributeName,
		       attr->paragraph, NSParagraphStyleAttributeName,
		       nil];
          
      str = (NSMutableAttributedString*) [NSMutableAttributedString 
                     attributedStringWithAttachment: attachment];

      [str addAttributes: attributes range: NSMakeRange (0, [str length])];
          
      [result replaceCharactersInRange: insertionRange withAttributedString: str];
      attr->changed = YES;
      RELEASE(attributes);
      RELEASE(attachment);
    }
}

- (void) appendHelpMarker: (NSString*)markerName
{
  int  oldPosition = [result length];
  NSRange insertionRange = NSMakeRange(oldPosition,0);

  if (!ignore)
    {
      GSHelpMarkerAttachment* attachment;
      RTFAttribute* attr = [self attr];
      NSMutableDictionary* attributes = nil;
      NSMutableAttributedString* str = nil;

      attachment =
	[[GSHelpMarkerAttachment alloc] initWithMarkerName: markerName];
      if (attachment == nil)
	{
	  NSLog(@"No attachment at %d", oldPosition);
	  return;
	}
        
      attributes = [[NSMutableDictionary alloc]
		     initWithObjectsAndKeys:
		       [attr currentFont], NSFontAttributeName,
		       attr->paragraph, NSParagraphStyleAttributeName,
		       nil];
          
      str = (NSMutableAttributedString*) [NSMutableAttributedString 
                     attributedStringWithAttachment: attachment];

      [str addAttributes: attributes range: NSMakeRange (0, [str length])];
          
      [result replaceCharactersInRange: insertionRange withAttributedString: str];
      attr->changed = YES;
      RELEASE(attributes);
      RELEASE(attachment);
    }
}

- (void) appendField: (int)start
         instruction: (NSString*)instruction
{
  if (!ignore)
    {
      int  oldPosition = start;
      int  textlen = [result length] - start;
      NSRange insertionRange = NSMakeRange(oldPosition, textlen);

      if ([instruction hasPrefix: @"HYPERLINK "])
        {
          NSDictionary *attributes;
          NSString *link = [instruction substringFromIndex: 10];

          if ([link characterAtIndex: 0] == (unichar)'\"') 
            {
              link = [link substringWithRange: NSMakeRange(1, [link length] - 2)];
            }

          attributes = [[NSDictionary alloc] 
                                       initWithObjectsAndKeys:
                           link, NSLinkAttributeName, 
                                     [NSNumber numberWithInt : 1], NSUnderlineStyleAttributeName,
                         [NSColor blueColor], NSForegroundColorAttributeName, 
                         nil];
          [result addAttributes: attributes
                         range: insertionRange];
          DESTROY(attributes);
        }
    }
}

- (void) appendImage: (NSString*)string
{
  // Do nothing for RTF
}

@end

#undef IGNORE
#define FONTS	((RTFConsumer *)ctxt)->fonts
#define COLOURS	((RTFConsumer *)ctxt)->colours
#define RESULT	((RTFConsumer *)ctxt)->result
#define IGNORE	((RTFConsumer *)ctxt)->ignore
#define TEXTPOSITION [RESULT length]
#define DOCUMENTATTRIBUTES ((RTFConsumer*)ctxt)->documentAttributes
#define ENCODING ((RTFConsumer *)ctxt)->encoding

#define FILES ((RTFDConsumer*)ctxt)->files

#define CTXT [((RTFConsumer *)ctxt) attr]
#define CHANGED CTXT->changed
#define PARAGRAPH CTXT->paragraph
#define FONTNAME CTXT->fontName
#define SCRIPT CTXT->script
#define ITALIC CTXT->italic
#define BOLD CTXT->bold
#define UNDERLINE CTXT->underline
#define STRIKETHROUGH CTXT->strikethrough
#define FGCOLOUR CTXT->fgColour
#define BGCOLOUR CTXT->bgColour
#define ULCOLOUR CTXT->ulColour

#define PAPERSIZE @"PaperSize"
#define LEFTMARGIN @"LeftMargin"
#define RIGHTMARGIN @"RightMargin"
#define TOPMARGIN @"TopMargin"
#define BUTTOMMARGIN @"ButtomMargin"

/*
  we must implement from the rtfConsumerFunctions.h file (Supporting files)
  this includes the yacc error handling and output
*/

/* handle errors (this is the yacc error mech)	*/
void GSRTFerror (void *ctxt, void *lctxt, const char *msg)
{
/*  [NSException raise:NSInvalidArgumentException
	       format:@"Syntax error in RTF: %s", msg];*/
  NSDebugLLog(@"RTFParser",@"Syntax error in RTF: %s", msg);
}

void GSRTFgenericRTFcommand (void *ctxt, RTFcmd cmd)
{
  NSDebugLLog(@"RTFParser", @"encountered rtf cmd:%s", cmd.name);
  if (!cmd.isEmpty) 
    NSDebugLLog(@"RTFParser", @" argument is %d\n", cmd.parameter);
}

//Start: we're doing some initialization
void GSRTFstart (void *ctxt)
{
  NSDebugLLog(@"RTFParser", @"Start RTF parsing");
}

// Finished to parse one piece of RTF.
void GSRTFstop (void *ctxt)
{
  //<!> close all open bolds et al.
  NSDebugLLog(@"RTFParser", @"End RTF parsing");
}

int GSRTFgetPosition(void *ctxt)
{
  return [((RTFConsumer *)ctxt)->result length];
}

void GSRTFopenBlock (void *ctxt, BOOL ignore)
{
  if (!IGNORE)
    {
      [(RTFConsumer *)ctxt push];
    }
  // Switch off any output for ignored block statements
  if (ignore)
    {
      IGNORE++;
    }
}

void GSRTFcloseBlock (void *ctxt, BOOL ignore)
{
  if (ignore)
    {
      IGNORE--;
    }
  if (!IGNORE)
    {
      [(RTFConsumer *)ctxt pop];
    }
}

void GSRTFmangleText (void *ctxt, const char *text)
{
  NSData *data = [[NSData alloc] initWithBytes: (void*)text 
				 length: strlen(text)];
  NSString *str = [[NSString alloc] initWithData: data
				    encoding: ENCODING];

  [(RTFConsumer *)ctxt appendString: str];
  DESTROY(str);
  DESTROY(data);
}

void GSRTFunicode (void *ctxt, int uchar)
{
  // Don't add the attachment character, this gets handled separatly
  if (uchar != (int)NSAttachmentCharacter)
    {
      unichar chars = uchar;
      NSString *str = [[NSString alloc] initWithCharacters: &chars 
                                                    length: 1];
      [(RTFConsumer *)ctxt appendString: str];
      DESTROY(str);
    }
}

void GSRTFregisterFont (void *ctxt, const char *fontName, 
			RTFfontFamily family, int fontNumber)
{
  NSString		*fontNameString;
  NSNumber		*fontId = [NSNumber numberWithInt: fontNumber];
  
  if (!fontName || !*fontName)
    {	
      [NSException raise: NSInvalidArgumentException 
		   format: @"Error in RTF (font omitted?), position:%lu",
		   (unsigned long) TEXTPOSITION];
    }
  // exclude trailing ';' from fontName
  if (';' == fontName[strlen(fontName)-1])
    {
      fontNameString = [NSString stringWithCString: fontName 
				 length: strlen(fontName)-1];
    }
  else 
    {
      fontNameString = [NSString stringWithCString: fontName 
				 length: strlen(fontName)];
    }
  [FONTS setObject: fontNameString forKey: fontId];
}

void GSRTFfontNumber (void *ctxt, int fontNumber)
{
  NSNumber *fontId = [NSNumber numberWithInt: fontNumber];
  NSString *fontName = [FONTS objectForKey: fontId];

  if (fontName == nil)
    {
      /* we're about to set an unknown font */
      [NSException raise: NSInvalidArgumentException 
		   format: @"Error in RTF (referring to undefined font \\f%d), position:%lu",
		   fontNumber,
		   (unsigned long) TEXTPOSITION];
    } 
  else 
    {
      if (![fontName isEqual: FONTNAME])
        {
	    ASSIGN(FONTNAME, fontName);
	    CHANGED = YES;
	}
    }
}

//	<N> fontSize is in halfpoints according to spec
void GSRTFfontSize (void *ctxt, int fontSize)
{
  float size = halfpoints2points(fontSize);
  
  if (size != CTXT->fontSize)
    {
      CTXT->fontSize = size;
      CHANGED = YES;
    }
}

void GSRTFpaperWidth (void *ctxt, int width)
{
  float fwidth = twips2points(width);
  NSMutableDictionary *dict = DOCUMENTATTRIBUTES;
  NSValue *val = [dict objectForKey: PAPERSIZE];
  NSSize size;

  if (val == nil)
    {
      size = NSMakeSize(fwidth, 792);
    }
  else
    {
      size = [val sizeValue];
      size.width = fwidth;
    }
  [dict setObject: [NSValue valueWithSize: size]  forKey: PAPERSIZE];
}

void GSRTFpaperHeight (void *ctxt, int height)
{
  float fheight = twips2points(height);
  NSMutableDictionary *dict = DOCUMENTATTRIBUTES;
  NSValue *val = [dict objectForKey: PAPERSIZE];
  NSSize size;

  if (val == nil)
    {
      size = NSMakeSize(612, fheight);
    }
  else
    {
      size = [val sizeValue];
      size.height = fheight;
    }
  [dict setObject: [NSValue valueWithSize: size]  forKey: PAPERSIZE];
}

void GSRTFmarginLeft (void *ctxt, int margin)
{
  float fmargin = twips2points(margin);
  NSMutableDictionary *dict = DOCUMENTATTRIBUTES;

  [dict setObject: [NSNumber numberWithFloat: fmargin]  forKey: LEFTMARGIN];
}

void GSRTFmarginRight (void *ctxt, int margin)
{
  float fmargin = twips2points(margin);
  NSMutableDictionary *dict = DOCUMENTATTRIBUTES;

  [dict setObject: [NSNumber numberWithFloat: fmargin]  forKey: RIGHTMARGIN];
}

void GSRTFmarginTop (void *ctxt, int margin)
{
  float fmargin = twips2points(margin);
  NSMutableDictionary *dict = DOCUMENTATTRIBUTES;

  [dict setObject: [NSNumber numberWithFloat: fmargin]  forKey: TOPMARGIN];
}

void GSRTFmarginButtom (void *ctxt, int margin)
{
  float fmargin = twips2points(margin);
  NSMutableDictionary *dict = DOCUMENTATTRIBUTES;

  [dict setObject: [NSNumber numberWithFloat: fmargin]  forKey: BUTTOMMARGIN];
}

void GSRTFfirstLineIndent (void *ctxt, int indent)
{
  NSMutableParagraphStyle *para = PARAGRAPH;
  float findent = twips2points(indent);

  CTXT->real_fi = findent;

  findent = CTXT->real_li + CTXT->real_fi;

  // for attributed strings only positiv indent is allowed
  if ((findent >= 0.0) && ([para firstLineHeadIndent] != findent))
    {
      [para setFirstLineHeadIndent: findent];
      CHANGED = YES;
    }
}

void GSRTFleftIndent (void *ctxt, int indent)
{
  NSMutableParagraphStyle *para = PARAGRAPH;
  float findent = twips2points(indent);

  CTXT->real_li = findent;

  // for attributed strings only positiv indent is allowed
  if ((findent >= 0.0) && ([para headIndent] != findent))
    {
      [para setHeadIndent: findent];
      CHANGED = YES;
    }

  findent = CTXT->real_li + CTXT->real_fi;
  if ((findent >= 0.0) && ([para firstLineHeadIndent] != findent))
    {
      [para setFirstLineHeadIndent: findent];
      CHANGED = YES;
    }
}

void GSRTFrightIndent (void *ctxt, int indent)
{
  NSMutableParagraphStyle *para = PARAGRAPH;
  float findent = twips2points(indent);

  // for attributed strings only positiv indent is allowed
  if ((findent >= 0.0) && ([para tailIndent] != findent))
    {
      [para setTailIndent: -findent];
      CHANGED = YES;
    }
}

void GSRTFtabstop (void *ctxt, int location)
{
  float flocation = twips2points(location);

  if (flocation >= 0.0)
    {
      [CTXT addTab: flocation type: NSLeftTabStopType];
    }
}

void GSRTFalignCenter (void *ctxt)
{
  NSMutableParagraphStyle *para = PARAGRAPH;

  if ([para alignment] != NSCenterTextAlignment)
    {
      [para setAlignment: NSCenterTextAlignment];
      CHANGED = YES;
    }
}

void GSRTFalignJustified (void *ctxt)
{
  NSMutableParagraphStyle *para = PARAGRAPH;

  if ([para alignment] != NSJustifiedTextAlignment)
    {
      [para setAlignment: NSJustifiedTextAlignment];
      CHANGED = YES;
    }
}

void GSRTFalignLeft (void *ctxt)
{
  NSMutableParagraphStyle *para = PARAGRAPH;

  if ([para alignment] != NSLeftTextAlignment)
    {
      [para setAlignment: NSLeftTextAlignment];
      CHANGED = YES;
    }
}

void GSRTFalignRight (void *ctxt)
{
  NSMutableParagraphStyle *para = PARAGRAPH;

  if ([para alignment] != NSRightTextAlignment)
    {
      [para setAlignment: NSRightTextAlignment];
      CHANGED = YES;
    }
}

void GSRTFspaceAbove (void *ctxt, int space)
{
  NSMutableParagraphStyle *para = PARAGRAPH;
  float fspace = twips2points(space);

  if (fspace >= 0.0)
    {
      [para setParagraphSpacing: fspace];
      CHANGED = YES;
    }
}

void GSRTFlineSpace (void *ctxt, int space)
{
  NSMutableParagraphStyle *para = PARAGRAPH;
  float fspace = twips2points(space);

  if (space == 1000)
    {
      [para setMinimumLineHeight: 0.0];
      [para setMaximumLineHeight: 0.0];
      CHANGED = YES;
    }
  else if (fspace < 0.0)
    {
      [para setMaximumLineHeight: -fspace];
      CHANGED = YES;
    }
  else
    {
      [para setMinimumLineHeight: fspace];
      CHANGED = YES;
    }
}

void GSRTFdefaultParagraph (void *ctxt)
{
  [CTXT resetParagraphStyle];
}

void GSRTFstyle (void *ctxt, int style)
{
}

void GSRTFdefaultCharacterStyle (void *ctxt)
{
  [CTXT resetFont];
}

void GSRTFaddColor (void *ctxt, int red, int green, int blue)
{
  NSColor *colour = [NSColor colorWithCalibratedRed: red/255.0 
			     green: green/255.0 
			     blue: blue/255.0 
			     alpha: 1.0];

  [COLOURS addObject: colour];
}

void GSRTFaddDefaultColor (void *ctxt)
{
  [COLOURS addObject: [NSColor textColor]];
}

void GSRTFcolorbg (void *ctxt, int color)
{
  if ([COLOURS count] <= (unsigned int)color)
    {
      ASSIGN (BGCOLOUR, [NSColor whiteColor]);
    }
  else
    {
      ASSIGN (BGCOLOUR, [COLOURS objectAtIndex: color]);
    }
  CHANGED = YES;
}

void GSRTFcolorfg (void *ctxt, int color)
{
  if ([COLOURS count] <= (unsigned int)color)
    {
      ASSIGN (FGCOLOUR, [NSColor blackColor]);
    }
  else
    {
      ASSIGN (FGCOLOUR, [COLOURS objectAtIndex: color]);
    }
  CHANGED = YES;
}

void GSRTFunderlinecolor (void *ctxt, int color)
{
  if ([COLOURS count] <= (unsigned int)color)
    {
      ASSIGN (ULCOLOUR, [NSColor blackColor]);
    }
  else
    {
      ASSIGN (ULCOLOUR, [COLOURS objectAtIndex: color]);
    }
  CHANGED = YES;
}

void GSRTFsubscript (void *ctxt, int script)
{
  script = (int) (-halfpoints2points(script) / 3.0);

  if (script != SCRIPT)
    {
      SCRIPT = script;
      CHANGED = YES;
    }    
}

void GSRTFsuperscript (void *ctxt, int script)
{
  script = (int) (halfpoints2points(script) / 3.0);

  if (script != SCRIPT)
    {
      SCRIPT = script;
      CHANGED = YES;
    }    
}

void GSRTFitalic (void *ctxt, BOOL state)
{
  if (state != ITALIC)
    {
      ITALIC = state;
      CHANGED = YES;
    }
}

void GSRTFbold (void *ctxt, BOOL state)
{
  if (state != BOLD)
    {
      BOLD = state;
      CHANGED = YES;
    }
}

void GSRTFunderline (void *ctxt, BOOL state, NSInteger style)
{
  if (state == NO)
    {
      style = NSUnderlineStyleNone;
    }

  if (UNDERLINE != style)
    {
      UNDERLINE = style;
      CHANGED = YES;
    }
}

void GSRTFstrikethrough (void *ctxt, NSInteger style)
{
  if (STRIKETHROUGH != style)
    {
      STRIKETHROUGH = style;
      CHANGED = YES;
    }
}

void GSRTFstrikethroughDouble (void *ctxt)
{
  const NSInteger style = NSUnderlineStyleDouble | NSUnderlinePatternSolid;

  if (STRIKETHROUGH != style)
    {
      STRIKETHROUGH = style;
      CHANGED = YES;
    }
}

void GSRTFparagraph (void *ctxt)
{
  GSRTFmangleText(ctxt, "\n");
  CTXT->tabChanged = NO;
}

void GSRTFNeXTGraphic (void *ctxt, const char *fileName, int width, int height)
{
  [(RTFDConsumer *)ctxt appendImage: [NSString stringWithCString: fileName]];
}

void GSRTFNeXTHelpLink (void *ctxt, int num, const char *markername,
			const char *linkFilename, const char *linkMarkername)
{
  NSRange range;
  NSString *fileName = [NSString stringWithCString: linkFilename];
  NSString *markerName = [NSString stringWithCString: linkMarkername];

  range = [fileName rangeOfString: @";"];
  if (range.location != NSNotFound)
    fileName = [fileName substringToIndex:range.location];

  range = [markerName rangeOfString: @";"];
  if (range.location == 0)
    markerName = nil;
  else if (range.location != NSNotFound)
    markerName = [markerName substringToIndex:range.location];

  [(RTFDConsumer *)ctxt appendHelpLink: fileName marker: markerName];
}

void GSRTFNeXTHelpMarker (void *ctxt, int num, const char *markername)
{
  NSRange range;
  NSString *markerName = [NSString stringWithCString: markername];

  range = [markerName rangeOfString: @";"];
  if (range.location != NSNotFound)
    markerName = [markerName substringToIndex:range.location];
  [(RTFDConsumer *)ctxt appendHelpMarker: markerName];
}

void GSRTFaddField (void *ctxt, int start, const char *inst)
{
  NSString *fieldInstruction;

  // Ignore leading blanks
  while (inst[0] == ' ')
    {
      inst++;
    }
  fieldInstruction = [[NSString alloc] initWithCString: inst
                                              encoding: ENCODING];
      
  [(RTFDConsumer *)ctxt appendField: start instruction: fieldInstruction];
  DESTROY(fieldInstruction);
}

void GSRTFencoding(void *ctxt, int encoding)
{
  switch (encoding)
    {
    // ansi
    case 1:
      [(RTFDConsumer *)ctxt setEncoding: NSISOLatin1StringEncoding];
      break;
    // mac
    case 2:
      [(RTFDConsumer *)ctxt setEncoding: NSMacOSRomanStringEncoding];
      break;
    // pc
    case 3:
      // FIXME: Code page 437 kCFStringEncodingDOSLatinUS
      [(RTFDConsumer *)ctxt setEncoding: NSISOLatin1StringEncoding];
      break;
    // pca
    case 4:
      // FIXME: Code page 850 kCFStringEncodingDOSLatin1
      [(RTFDConsumer *)ctxt setEncoding: NSISOLatin1StringEncoding];
      break;
    case 1250:
      [(RTFDConsumer *)ctxt setEncoding: NSWindowsCP1250StringEncoding];
      break;
    case 1251:
      [(RTFDConsumer *)ctxt setEncoding: NSWindowsCP1251StringEncoding];
      break;
    case 1252:
      [(RTFDConsumer *)ctxt setEncoding: NSWindowsCP1252StringEncoding];
      break;
    case 1253:
      [(RTFDConsumer *)ctxt setEncoding: NSWindowsCP1253StringEncoding];
      break;
    case 1254:
      [(RTFDConsumer *)ctxt setEncoding: NSWindowsCP1254StringEncoding];
      break;
    case 10000:
      [(RTFDConsumer *)ctxt setEncoding: NSMacOSRomanStringEncoding];
      break;
    default:
      NSLog(@"Setting unknown encoding %d", encoding);
      break;
    }
}
