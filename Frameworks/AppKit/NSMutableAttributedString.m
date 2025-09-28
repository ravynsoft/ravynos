/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

// Original - Christopher Lloyd <cjwl@objc.net>
#import <AppKit/NSMutableAttributedString.h>
#import <AppKit/NSFontManager.h>
#import <AppKit/NSFont.h>

@implementation NSMutableAttributedString(NSMutableAttributedString_AppKit)

// Looks for a substitute in the list able to display the given char
-(NSFont *)_bestFontForCharacter:(unichar)c userFont:(NSFont *)font usingFontNames:(NSArray *)fontnameList {
    // We are caching the fontName->charset info, to prevent the creation/conversion of plenty of fonts
    static NSMutableDictionary *sNameToCharacterSetCache = nil;
    if (sNameToCharacterSetCache == nil) {
        sNameToCharacterSetCache = [[NSMutableDictionary alloc] init];
    }
    
    NSFont *substitute = nil;
    for (NSString *name in fontnameList) {
        
        NSCharacterSet *fontCharSet = [sNameToCharacterSetCache objectForKey:name];
        if (fontCharSet == nil) {
            NSFont *fontCandidate = [[NSFontManager sharedFontManager] convertFont:font toFace:name];
            fontCharSet = [fontCandidate coveredCharacterSet];
            if (fontCharSet != nil) {
                // Cache the info
                [sNameToCharacterSetCache setObject:fontCharSet forKey:name];
            } else {
                // Cache some empty charset so we don't look for it again
                [sNameToCharacterSetCache setObject:[NSCharacterSet characterSetWithRange:NSMakeRange(0, 0)] forKey:name];
            }
        }
        if (fontCharSet) {
            if ([fontCharSet characterIsMember:c]) {
                substitute = [[NSFontManager sharedFontManager] convertFont:font toFace:name];
            }
        }
        if (substitute) {
            break;
        }
    }
    return substitute;
}

-(NSFont *)_bestFontForCharacter:(unichar)c userFont:(NSFont *)font {
    // We first try a limited list of good known candidates - if that fails, then we'll try all of
    // the other fonts
    NSArray *list = [NSFont preferredFontNames];
    NSFont *bestFont = [self _bestFontForCharacter:c userFont:font usingFontNames:list];
    if (bestFont == nil) {
        // We couldn't find any prefered font - try all of them
        static NSArray *sAllFonts = nil;
        if (sAllFonts == nil) {
            sAllFonts = [[[NSFontManager sharedFontManager] availableFonts] retain];
        }
        bestFont = [self _bestFontForCharacter:c userFont:font usingFontNames:sAllFonts];
    }
    
    return bestFont;
}

-(void)fixFontAttributeInRange:(NSRange)range {
    NSString *string =[self string];
    unsigned  location=range.location;
    unsigned  limit=NSMaxRange(range);
    
    // Check that all of the chars in the range are supported by the font attribute
    // Else, change the font attribute to one supporting these chars
    NSCharacterSet *blankChars = [NSCharacterSet whitespaceAndNewlineCharacterSet];
    NSCharacterSet *controlChars = [NSCharacterSet controlCharacterSet];
    while(location<limit){
        NSRange              effectiveRange;
        NSFont              *font=[self attribute:NSFontAttributeName atIndex:location effectiveRange:&effectiveRange];
        if (font == nil) {
            // We really need a font for doing font substitution
            // Use the default attribute value
            // Note that if no substitution is needed, then we'll keep the no font info
            font = NSFontAttributeInDictionary(nil);
        }
        if (font) {
            NSCharacterSet      *fontCharSet = nil;
            NSRange testRange = NSIntersectionRange(range, effectiveRange);
            unichar *chars = malloc(sizeof(unichar)*testRange.length);
            if (chars) {
                [string getCharacters:chars range:testRange];
                for (int i = 0; i < testRange.length; ++i) {
                    unichar charToTest = chars[i];
                    // No need to process blanks
                    if ([blankChars characterIsMember:charToTest] == NO && [controlChars characterIsMember:charToTest] == NO) {
                        if (fontCharSet == nil) {
                            fontCharSet = [font coveredCharacterSet];
                        }
                        if ([fontCharSet characterIsMember:charToTest] == NO) {
                            NSFont *substitute = [self _bestFontForCharacter:charToTest userFont:font];
                            if (substitute) {
                                [self addAttribute:NSFontAttributeName value:substitute range:NSMakeRange(i+testRange.location, 1)];
                            }
                        }
                    }
                }
                free(chars);
            }
        }
        location=NSMaxRange(effectiveRange);
    }
}

-(void)fixParagraphStyleAttributeInRange:(NSRange)range {
    // TODO :
    /**
     Fixes the paragraph style attributes in aRange, assigning the first paragraph style attribute value in each paragraph to all characters of the paragraph.
     This method extends the range as needed to cover the last paragraph partially contained. A paragraph is delimited by any of these characters, the longest possible sequence being preferred to any shorter:
     U+000D (\r or CR)
     U+000A (\n or LF)
     U+2028 (Unicode line separator)
     U+2029 (Unicode paragraph separator) \r\n, in that order (also known as CRLF)
     */
}

-(void)fixAttributesInRange:(NSRange)range {
    [self fixFontAttributeInRange: range];
    [self fixParagraphStyleAttributeInRange: range];
}

-(void)applyFontTraits:(NSFontTraitMask)traits range:(NSRange)range {
   unsigned  location=range.location;
   unsigned  limit=NSMaxRange(range);

   [self beginEditing];

   while(location<limit){
    NSRange              effectiveRange;
    NSMutableDictionary *attributes=[[[self attributesAtIndex:location effectiveRange:&effectiveRange] mutableCopy] autorelease];
    NSFont              *font=NSFontAttributeInDictionary(attributes);

    font=[[NSFontManager sharedFontManager] convertFont:font toHaveTrait:traits];
    [attributes setObject:font forKey:NSFontAttributeName];

    if(effectiveRange.location<location){
     effectiveRange.length=NSMaxRange(effectiveRange)-location;
     effectiveRange.location=location;
    }
    if(NSMaxRange(effectiveRange)>limit)
     effectiveRange.length=limit-effectiveRange.location;

    [self setAttributes:attributes range:effectiveRange];

    location=NSMaxRange(effectiveRange);
   }
   [self endEditing];
}

@end

