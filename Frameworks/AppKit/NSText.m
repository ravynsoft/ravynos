/* Copyright (c) 2006-2007 Christopher J. W. Lloyd <cjwl@objc.net>

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <AppKit/NSText.h>
#import <AppKit/NSRaise.h>

NSString * const NSTextDidBeginEditingNotification=@"NSTextDidBeginEditingNotification";
NSString * const NSTextDidEndEditingNotification=@"NSTextDidEndEditingNotification";
NSString * const NSTextDidChangeNotification=@"NSTextDidChangeNotification";

@implementation NSText

-delegate {
   NSInvalidAbstractInvocation();
   return nil;
}

-(NSString *)string {
   NSInvalidAbstractInvocation();
   return nil;
}

-(NSData *)RTFFromRange:(NSRange)range {
   NSInvalidAbstractInvocation();
   return nil;
}

-(NSData *)RTFDFromRange:(NSRange)range {
   NSInvalidAbstractInvocation();
   return nil;
}

-(BOOL)isEditable {
   NSInvalidAbstractInvocation();
   return NO;
}

-(BOOL)isSelectable {
   NSInvalidAbstractInvocation();
   return NO;
}

-(BOOL)isRichText {
   NSInvalidAbstractInvocation();
   return NO;
}

-(BOOL)isFieldEditor {
   NSInvalidAbstractInvocation();
   return NO;
}

-(BOOL)usesFontPanel {
   NSInvalidAbstractInvocation();
   return NO;
}

-(BOOL)importsGraphics {
   NSInvalidAbstractInvocation();
   return NO;
}

-(NSFont *)font {
   NSInvalidAbstractInvocation();
   return nil;
}

-(NSTextAlignment)alignment {
   NSInvalidAbstractInvocation();
   return 0;
}

-(NSColor *)textColor {
   NSInvalidAbstractInvocation();
   return nil;
}

-(BOOL)drawsBackground {
   NSInvalidAbstractInvocation();
   return NO;
}

-(NSColor *)backgroundColor {
   NSInvalidAbstractInvocation();
   return nil;
}

-(BOOL)isHorizontallyResizable {
   NSInvalidAbstractInvocation();
   return NO;
}

-(BOOL)isVerticallyResizable {
   NSInvalidAbstractInvocation();
   return NO;
}

-(NSSize)maxSize {
   NSInvalidAbstractInvocation();
   return NSMakeSize(0,0);
}

-(NSSize)minSize {
   NSInvalidAbstractInvocation();
   return NSMakeSize(0,0);
}
-(void)setMinSize:(NSSize)size {
   NSInvalidAbstractInvocation();
}




-(NSRange)selectedRange {
   NSInvalidAbstractInvocation();
   return NSMakeRange(0,0);
}

-(void)setDelegate:delegate {
   NSInvalidAbstractInvocation();
}

-(void)setString:(NSString *)string {
   [self replaceCharactersInRange:NSMakeRange(0,[[self string] length]) withString:string];
}

-(void)replaceCharactersInRange:(NSRange)range withString:(NSString *)string {
   NSInvalidAbstractInvocation();
}

-(BOOL)readRTFDFromFile:(NSString *)path {
   NSInvalidAbstractInvocation();
   return NO;
}

-(void)replaceCharactersInRange:(NSRange)range withRTF:(NSData *)rtf {
   NSInvalidAbstractInvocation();
}

-(void)replaceCharactersInRange:(NSRange)range withRTFD:(NSData *)rtfd {
   NSInvalidAbstractInvocation();
}

-(void)setEditable:(BOOL)flag {
   NSInvalidAbstractInvocation();
}

-(void)setSelectable:(BOOL)flag {
   NSInvalidAbstractInvocation();
}

-(void)setRichText:(BOOL)flag {
   NSInvalidAbstractInvocation();
}

-(void)setFieldEditor:(BOOL)flag {
   NSInvalidAbstractInvocation();
}

-(void)setUsesFontPanel:(BOOL)value {
   NSInvalidAbstractInvocation();
}

-(void)setImportsGraphics:(BOOL)value {
   NSInvalidAbstractInvocation();
}

-(void)setFont:(NSFont *)font {
   NSInvalidAbstractInvocation();
}

-(void)setFont:(NSFont *)font range:(NSRange)range {
   NSInvalidAbstractInvocation();
}

-(void)setAlignment:(NSTextAlignment)alignment {
   NSInvalidAbstractInvocation();
}

-(void)setTextColor:(NSColor *)color {
   NSInvalidAbstractInvocation();
}

-(void)setTextColor:(NSColor *)color range:(NSRange)range {
   NSInvalidAbstractInvocation();
}

-(void)setDrawsBackground:(BOOL)flag {
   NSInvalidAbstractInvocation();
}

-(void)setBackgroundColor:(NSColor *)color {
   NSInvalidAbstractInvocation();
}

-(void)setHorizontallyResizable:(BOOL)flag {
   NSInvalidAbstractInvocation();
}

-(void)setVerticallyResizable:(BOOL)flag {
   NSInvalidAbstractInvocation();
}

-(void)setMaxSize:(NSSize)size {
   NSInvalidAbstractInvocation();
}

-(void)setSelectedRange:(NSRange)range {
   NSInvalidAbstractInvocation();
}

-(void)sizeToFit {
   NSInvalidAbstractInvocation();
}

-(void)scrollRangeToVisible:(NSRange)range {
   NSInvalidAbstractInvocation();
}

-(void)changeFont:sender {
    NSInvalidAbstractInvocation();
}

- (void)alignCenter:sender {
   NSInvalidAbstractInvocation();
}

- (void)alignLeft:sender {
   NSInvalidAbstractInvocation();
}

- (void)alignRight:sender {
   NSInvalidAbstractInvocation();
}

- (void)underline:sender {
   NSInvalidAbstractInvocation();
}

-(void)selectAll:sender {
   [self setSelectedRange:NSMakeRange(0,[[self string] length])];
   [self setNeedsDisplay:YES];
}

-(void)delete:sender {
   NSRange selection=[self selectedRange];

   [self replaceCharactersInRange:[self selectedRange] withString:@""];
   [self setSelectedRange:NSMakeRange(selection.location,0)];
   
   [self setNeedsDisplay:YES];
}

-(void)toggleRuler:sender {
    NSInvalidAbstractInvocation();
}

-(void)copyRuler:sender {
    NSInvalidAbstractInvocation();
}

-(void)pasteRuler:sender {
    NSInvalidAbstractInvocation();
}

-(void)changeSpelling:sender {
    NSInvalidAbstractInvocation();
}

-(void)ignoreSpelling:sender {
    NSInvalidAbstractInvocation();
}

-(void)showGuessPanel:sender {
    NSInvalidAbstractInvocation();
}

-(void)checkSpelling:sender {
    NSInvalidAbstractInvocation();
}

@end

