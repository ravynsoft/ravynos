/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <AppKit/NSGlyphGenerator.h>
#import <AppKit/NSAttributedString.h>

@implementation NSGlyphGenerator

+sharedGlyphGenerator {
   NSGlyphGenerator *shared=nil;
   
   if(shared==nil)
    shared=[self new];
   
   return shared;
}

-(void)generateGlyphsForGlyphStorage:(id <NSGlyphStorage>)glyphStorage desiredNumberOfCharacters:(unsigned int)numberOfCharacters glyphIndex:(unsigned *)glyphIndexp characterIndex:(unsigned *)characterIndexp {
   NSAttributedString *text=[glyphStorage attributedString];
   NSString           *string=[text string];
   unsigned            length=[string length];
   unsigned            characterIndex=*characterIndexp;
   unsigned            glyphIndex=*glyphIndexp;
   NSRange             effectiveRange=NSMakeRange(0,0);
   NSDictionary       *attributes=nil;
   NSFont             *font=nil;
   int                 i;
   
   for(i=0;i<numberOfCharacters && characterIndex<length;){
    unsigned chunkSize=MIN(1024,numberOfCharacters-i);
    unichar  characterChunk[chunkSize];
    NSGlyph  glyphChunk[chunkSize];
    
    if(!NSLocationInRange(characterIndex,effectiveRange)){
     attributes=[text attributesAtIndex:characterIndex effectiveRange:&effectiveRange];
     font=NSFontAttributeInDictionary(attributes);
    }
    if(chunkSize>(NSMaxRange(effectiveRange)-characterIndex))
     chunkSize=NSMaxRange(effectiveRange)-characterIndex;
    
    [string getCharacters:characterChunk range:NSMakeRange(characterIndex,chunkSize)];
    [font getGlyphs:glyphChunk forCharacters:characterChunk length:chunkSize];
    
    [glyphStorage insertGlyphs:glyphChunk length:chunkSize forStartingGlyphAtIndex:glyphIndex characterIndex:characterIndex];
    
    characterIndex+=chunkSize;
    glyphIndex+=chunkSize;
    i+=chunkSize;
   }
   *glyphIndexp=glyphIndex;
   *characterIndexp=characterIndex;
}

@end
