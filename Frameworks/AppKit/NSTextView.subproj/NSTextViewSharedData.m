/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import "NSTextViewSharedData.h"
#import <Foundation/NSString.h>
#import <Foundation/NSException.h>
#import <Foundation/NSDictionary.h>
#import <AppKit/NSColor.h>
#import <AppKit/NSParagraphStyle.h>
#import <Foundation/NSKeyedArchiver.h>

@implementation NSTextViewSharedData

-initWithCoder:(NSCoder *)coder {
   if([coder allowsKeyedCoding]){
    NSKeyedUnarchiver *keyed=(NSKeyedUnarchiver *)coder;
    
    _backgroundColor=[[keyed decodeObjectForKey:@"NSBackgroundColor"] retain];
    _defaultParagraphStyle=[[keyed decodeObjectForKey:@"NSDefaultParagraphStyle"] retain];
    _flags=[keyed decodeIntForKey:@"NSFlags"];
    _insertionColor=[[keyed decodeObjectForKey:@"NSInsertionColor"] retain];
    _linkAttributes=[[keyed decodeObjectForKey:@"NSLinkAttributes"] retain];
    _markedAttributes=[[keyed decodeObjectForKey:@"NSMarkedAttributes"] retain];
    _selectedAttributes=[[keyed decodeObjectForKey:@"NSSelectedAttributes"] retain];
   }
   else {
    [NSException raise:NSInvalidArgumentException format:@"-[%@ %s] is not implemented for coder %@",isa,sel_getName(_cmd),coder];
   }
   
   return self;
}

-(void)dealloc {
   [_backgroundColor release];
   [_defaultParagraphStyle release];
   [_insertionColor release];
   [_linkAttributes release];
   [_markedAttributes release];
   [_selectedAttributes release];
   [super dealloc];
}

-(NSColor *)backgroundColor {
   return _backgroundColor;
}

-(NSColor *)insertionColor {
   return _insertionColor;
}

-(NSParagraphStyle *)defaultParagraphStyle {
   return _defaultParagraphStyle;
}

-(BOOL)drawsBackground {
	return (_flags & (1 << 8)) ? YES : NO;
}

-(BOOL)isEditable {
   return (_flags&0x00000002)?YES:NO;
}

-(BOOL)isSelectable {
   return (_flags&0x00000001)?YES:NO;
}

-(BOOL)isRichText {
    return (_flags&0x00000004)?YES:NO;
}

-(BOOL)allowsUndo {
    return (_flags&0x00000400)?YES:NO;
}

@end
