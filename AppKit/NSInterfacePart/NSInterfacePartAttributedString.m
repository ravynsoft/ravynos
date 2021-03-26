/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <AppKit/NSInterfacePartAttributedString.h>
#import <AppKit/NSAttributedString.h>
#import <AppKit/NSStringDrawing.h>
#import <AppKit/NSColor.h>
#import <AppKit/NSFont.h>

@implementation NSInterfacePartAttributedString

-initWithCharacter:(unichar)character fontName:(NSString *)fontName pointSize:(float)pointSize color:(NSColor *)color {
   NSString     *string=[NSString stringWithCharacters:&character length:1];
   NSFont       *font=[NSFont fontWithName:fontName size:pointSize];
   NSDictionary *attributes=[NSDictionary dictionaryWithObjectsAndKeys:
     font,NSFontAttributeName,
     color,NSForegroundColorAttributeName,
     nil];

   _attributedString=[[NSAttributedString alloc] initWithString:string attributes:attributes];

   return self;
}

+(NSColor *)textColor {
   return [NSColor controlTextColor];
}

-initWithMarlettCharacter:(unichar)character {
   return [self initWithCharacter:character fontName:@"Marlett" pointSize:10 color:[isa textColor]];
}

-(void)dealloc {
   [_attributedString release];
   [super dealloc];
}

-(NSSize)size {
   return [_attributedString size];
}

-(void)drawAtPoint:(NSPoint)point {
   [_attributedString drawAtPoint:point];
}

@end
