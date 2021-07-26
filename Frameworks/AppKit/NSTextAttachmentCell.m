/* Copyright (c) 2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <AppKit/NSTextAttachmentCell.h>
#import <AppKit/NSRaise.h>

@implementation NSTextAttachmentCell

-(NSTextAttachment *)attachment {
   return _attachment;
}

-(void)setAttachment:(NSTextAttachment *)attachment {
   _attachment=attachment;
}

-(NSSize)cellSize {
   NSUnimplementedMethod();
   return NSMakeSize(0,0);
}

-(NSPoint)cellBaselineOffset {
   NSUnimplementedMethod();
   return NSMakePoint(0,0);
}

-(NSRect)cellFrameForTextContainer:(NSTextContainer *)textContainer proposedLineFragment:(NSRect)proposedRect glyphPosition:(NSPoint)glyphPoint characterIndex:(unsigned)characterIndex {
   NSUnimplementedMethod();
   return NSMakeRect(0,0,0,0);
}

-(BOOL)wantsToTrackMouse {
   return YES;
}

-(BOOL)wantsToTrackMouseForEvent:(NSEvent *)event inRect:(NSRect)rect ofView:(NSView *)view atCharacterIndex:(unsigned)characterIndex {
   return [self wantsToTrackMouse];
}

-(BOOL)trackMouse:(NSEvent *)event inRect:(NSRect)rect ofView:(NSView *)view atCharacterIndex:(unsigned)characterIndex untilMouseUp:(BOOL)untilMouseUp {
   NSUnimplementedMethod();
   return NO;
}

-(BOOL)trackMouse:(NSEvent *)event inRect:(NSRect)rect ofView:(NSView *)view untilMouseUp:(BOOL)untilMouseUp {
   NSUnimplementedMethod();
   return NO;
}

-(void)highlight:(BOOL)highlight withFrame:(NSRect)frame inView:(NSView *)view {
   NSUnimplementedMethod();
}

-(void)drawWithFrame:(NSRect)frame inView:(NSView *)view characterIndex:(unsigned)characterIndex layoutManager:(NSLayoutManager *)layoutManager {
   NSUnimplementedMethod();
}

-(void)drawWithFrame:(NSRect)frame inView:(NSView *)view characterIndex:(unsigned)characterIndex {
   NSUnimplementedMethod();
}

-(void)drawWithFrame:(NSRect)frame inView:(NSView *)view {
   NSUnimplementedMethod();
}

@end
