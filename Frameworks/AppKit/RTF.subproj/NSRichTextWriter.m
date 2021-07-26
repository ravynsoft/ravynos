/* Copyright (c) 2006-2007 Christopher J. W. Lloyd
 
 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

// Original - Christopher Lloyd <cjwl@objc.net>
#import <AppKit/NSRichTextWriter.h>
#import <AppKit/NSColor.h>
#import <AppKit/NSFont.h>
#import <AppKit/NSFontManager.h>
#import <AppKit/NSAttributedString.h>
#ifdef WIN32
#import <malloc.h>
#endif

@implementation NSRichTextWriter

-initWithAttributedString:(NSAttributedString *)attributedString range:(NSRange)range {
    _attributedString=[attributedString retain];
    _string=[[_attributedString string] retain];
    _range=range;
    _data=[NSMutableData new];
    return self;
}

-(void)dealloc {
    [_attributedString release];
    [_string release];
    [_data release];
    [super dealloc];
}

+(NSData *)dataWithAttributedString:(NSAttributedString *)attributedString range:(NSRange)range {
    NSRichTextWriter *writer=[[self alloc] initWithAttributedString:attributedString range:range];
    NSData           *result=[[[writer generateData] retain] autorelease];
    
    [writer release];
    
    return result;
}

-(void)appendCString:(const char *)cString {
    [_data appendBytes:cString length:strlen(cString)];
}

-(void)appendStringFromRange:(NSRange)range {
    unichar        buffer[range.length];
    unsigned char *ansi;
    int            i,ansiLength=0;
    
    [_string getCharacters:buffer range:range];
    for(i=0;i<range.length;i++){
        if(buffer[i]=='\n' || buffer[i]=='\\')
            ansiLength+=2;
        else if(buffer[i]>127)
            ansiLength+=9; // unicode is encoded as '\uXXXXXX?'
    }
    
    ansi=alloca(sizeof(unsigned char)*ansiLength);
    ansiLength=0;
    for(i=0;i<range.length;i++){
        unichar code=buffer[i];
        
        if(code=='\n'){
            ansi[ansiLength++]='\\';
            ansi[ansiLength++]='\n';
        }
        else if(code=='\\'){
            ansi[ansiLength++]='\\';
            ansi[ansiLength++]='\\';
        }
        else if(code>127){
            // "= \u<code>?" where "?" is a substitution char for readers not supporting
            
            int len = sprintf((char*)ansi+ansiLength, "\\u%d?", code);
            ansiLength += len;
        }
        else {
            ansi[ansiLength++]=code;
        }
    }
    
    [_data appendBytes:ansi length:ansiLength];
}

-(void)writeRichText {
    [self appendCString:"{\\rtf1\\ansi "];
    
    // Pass 1 : create the font & color tables
    NSMutableArray *colors = [NSMutableArray array];
    NSMutableArray *fonts = [NSMutableArray array];
    
    NSUInteger  location=_range.location;
    NSUInteger  limit=NSMaxRange(_range);
    while(location<limit){
        NSRange         effectiveRange;
        NSDictionary   *attributes=[_attributedString attributesAtIndex:location effectiveRange:&effectiveRange];
        NSFont         *font=[attributes objectForKey:NSFontAttributeName];
        NSColor        *fontColor = [attributes objectForKey:NSForegroundColorAttributeName];
        NSColor        *backgroundColor = [attributes objectForKey:NSBackgroundColorAttributeName];
        NSColor        *underlineColor = [attributes objectForKey:NSUnderlineColorAttributeName];
        
        fontColor = [fontColor colorUsingColorSpaceName:NSCalibratedRGBColorSpace];
        backgroundColor = [backgroundColor colorUsingColorSpaceName:NSCalibratedRGBColorSpace];
        underlineColor = [underlineColor colorUsingColorSpaceName:NSCalibratedRGBColorSpace];
        
        // Put there the font name without the italic/bold.. attributes
        NSFont *plainFont = [[NSFontManager sharedFontManager] convertFont:font toNotHaveTrait:0xFFFFFF];
        NSString *fontName = [plainFont fontName];
        if (fontName && [fonts containsObject:fontName] == NO) {
            [fonts addObject:fontName];
        }
        if (fontColor && [colors containsObject:fontColor] == NO) {
            [colors addObject:fontColor];
        }
        if (backgroundColor && [colors containsObject:backgroundColor] == NO) {
            [colors addObject:backgroundColor];
            NSColor *whiteColor = [NSColor colorWithCalibratedRed:1. green:1. blue:1. alpha:1.];
            if ([colors containsObject:whiteColor] == NO) {
                [colors addObject:whiteColor];
            }
        }
        if (underlineColor && [colors containsObject:underlineColor] == NO) {
            [colors addObject:underlineColor];
        }
        
        if(effectiveRange.location<location){
            effectiveRange.length=NSMaxRange(effectiveRange)-location;
            effectiveRange.location=location;
        }
        if(NSMaxRange(effectiveRange)>limit)
            effectiveRange.length=limit-effectiveRange.location;
        
        location=NSMaxRange(effectiveRange);
    }
    
    // Output the tables
    if (colors.count) {
        [self appendCString:"{\\colortbl "];
        for (NSColor *color in colors) {
            NSString *str = [NSString stringWithFormat:@"\\red%ld\\green%ld\\blue%ld;", lroundf(color.redComponent*255), lroundf(color.greenComponent*255), lroundf(color.blueComponent*255)];
            [self appendCString:[str UTF8String]];
        }
        [self appendCString:"}"];
    }
    if (fonts.count) {
        [self appendCString:"{\\fonttbl "];
        int i = 0;
        for (NSString *fontName in fonts) {
            NSString *str = [NSString stringWithFormat:@"\\f%d\\fnil\\fcharset0 %@;", i++, fontName];
            [self appendCString:[str UTF8String]];
        }
        [self appendCString:"}"];
    }
    NSUInteger currentFontIdx = NSNotFound;
    BOOL isBold = NO;
    BOOL isItalic = NO;
    NSUInteger currentColorIdx = NSNotFound;
    NSUInteger currentBackgroundColorIdx = NSNotFound;
    NSUInteger currentUnderlineColorIdx = NSNotFound;
    NSInteger currentUnderlineStyle = NSUnderlineStyleNone;
    
    // Pass 2 : output the text & attributes
    location=_range.location;
    limit=NSMaxRange(_range);
    while(location<limit){
        NSRange         effectiveRange;
        NSDictionary   *attributes=[_attributedString attributesAtIndex:location effectiveRange:&effectiveRange];
        NSFont         *font=[attributes objectForKey:NSFontAttributeName];
        NSFont *plainFont = [[NSFontManager sharedFontManager] convertFont:font toNotHaveTrait:0xFFFFFF];
        NSFontTraitMask traits=[[NSFontManager sharedFontManager] traitsOfFont:font];
        NSColor *color = [attributes objectForKey:NSForegroundColorAttributeName];
        NSColor *backgroundColor = [attributes objectForKey:NSBackgroundColorAttributeName];
        NSColor *underlineColor = [attributes objectForKey:NSUnderlineColorAttributeName];
        NSInteger underlineStyle = [[attributes objectForKey:NSUnderlineStyleAttributeName] integerValue];
        
        if(effectiveRange.location<location){
            effectiveRange.length=NSMaxRange(effectiveRange)-location;
            effectiveRange.location=location;
        }
        if(NSMaxRange(effectiveRange)>limit)
            effectiveRange.length=limit-effectiveRange.location;
        
        if((traits&NSBoldFontMask) && (isBold == NO))
            [self appendCString:"\\b "];
        
        if(((traits&NSBoldFontMask) == 0) && (isBold == YES))
            [self appendCString:"\\b0 "];
        isBold = (traits&NSBoldFontMask) != 0;
        
        if((traits&NSItalicFontMask) && isItalic == NO)
            [self appendCString:"\\i "];
        
        if(((traits&NSItalicFontMask) == 0) && (isItalic == YES))
            [self appendCString:"\\i0 "];
        isItalic = (traits&NSItalicFontMask) != 0;
        
        if (underlineStyle != currentUnderlineStyle) {
            switch (underlineStyle) {
                case NSUnderlineStyleNone:
                    [self appendCString:"\\ulnone "];
                    break;
                case NSUnderlineStyleSingle:
                    [self appendCString:"\\ul "];
                    break;
                case NSUnderlineStyleThick:
                    [self appendCString:"\\ulth "];
                    break;
                case NSUnderlineStyleDouble:
                    [self appendCString:"\\uldb "];
                    break;
                default:
                    break;
            }
            currentUnderlineStyle = underlineStyle;
        }
        
        if (color) {
            color = [color colorUsingColorSpaceName:NSCalibratedRGBColorSpace];
            NSUInteger idx = [colors indexOfObject:color];
            if (idx != NSNotFound && idx != currentColorIdx) {
                NSString *colorNum = [NSString stringWithFormat:@"\\cf%ld ", (unsigned long)idx];
                [self appendCString:[colorNum UTF8String]];
                currentColorIdx = idx;
            }
        }
        if (underlineColor) {
            underlineColor = [underlineColor colorUsingColorSpaceName:NSCalibratedRGBColorSpace];
            NSUInteger idx = [colors indexOfObject:underlineColor];
            if (idx != NSNotFound && idx != currentUnderlineColorIdx) {
                NSString *colorNum = [NSString stringWithFormat:@"\\ulc%ld ", (unsigned long)idx];
                [self appendCString:[colorNum UTF8String]];
                currentUnderlineColorIdx = idx;
            }
        }
        if (backgroundColor == nil && currentBackgroundColorIdx != NSNotFound) {
            backgroundColor = [NSColor whiteColor];
        }
        if (backgroundColor) {
            backgroundColor = [backgroundColor colorUsingColorSpaceName:NSCalibratedRGBColorSpace];
            NSUInteger idx = [colors indexOfObject:backgroundColor];
            if (idx != NSNotFound && idx != currentBackgroundColorIdx) {
                NSString *colorNum = [NSString stringWithFormat:@"\\cb%ld ", (unsigned long)idx];
                [self appendCString:[colorNum UTF8String]];
                currentBackgroundColorIdx = idx;
            }
        }
        
        if (font) {
            NSUInteger idx = [fonts indexOfObject:[plainFont fontName]];
            if (idx != NSNotFound && idx != currentFontIdx) {
                NSString *fontNum = [NSString stringWithFormat:@"\\f%ld ", (unsigned long)idx];
                [self appendCString:[fontNum UTF8String]];
                currentFontIdx = idx;
            }
            NSString *fontSize = [NSString stringWithFormat:@"\\fs%d ", (int)([font pointSize]*2.)];
            [self appendCString:[fontSize UTF8String]];
        }
        
        [self appendStringFromRange:effectiveRange];
        
        location=NSMaxRange(effectiveRange);
    }
    [self appendCString:"}"];
}

-(NSData *)generateData {
    [self writeRichText];
    return _data;
}

@end
