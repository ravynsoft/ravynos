/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <AppKit/NSSecureTextFieldCell.h>
#import <AppKit/NSSecureTextView.h>
#import <AppKit/NSStringDrawer.h>

@implementation NSSecureTextFieldCell

-initTextCell:(NSString *)string {
   [super initTextCell:string];
   _echosBullets=YES;
   return self;
}

-initImageCell:(NSImage *)image {
   [super initImageCell:image];
   _echosBullets=YES;
   return self;
}

-initWithCoder:(NSCoder *)coder {
   [super initWithCoder:coder];
   _echosBullets=YES;
   return self;
}

-(BOOL)echosBullets {
   return _echosBullets;
}

-(void)setEchosBullets:(BOOL)yorn {
   _echosBullets=yorn;
}

-(NSText *)setUpFieldEditorAttributes:(NSText *)editor {
   editor=[[[NSSecureTextView alloc] init] autorelease];
   [editor setHorizontallyResizable:NO];
   [editor setVerticallyResizable:NO];
   [editor setFieldEditor:YES];
   [editor setAutoresizingMask:NSViewWidthSizable|NSViewHeightSizable];
   [(NSSecureTextView *)editor setEchosBullets:[self echosBullets]];

   return [super setUpFieldEditorAttributes:editor];
}

-(void)drawInteriorWithFrame:(NSRect)frame inView:(NSView *)control {
    NSRect titleRect=[self titleRectForBounds:frame];
    NSMutableAttributedString *astring=[[self attributedStringValue] mutableCopy];
    NSUInteger                 i,length=[astring length];
    unichar                   *buffer=NSZoneMalloc(NULL,length*sizeof(unichar));
    
    for(i=0;i<length;i++)
        buffer[i]=_echosBullets?0x2022:' '; // unicode bullet
    
    NSString *string=[[NSString alloc] initWithCharactersNoCopy:buffer length:length freeWhenDone:YES];
    [astring replaceCharactersInRange:NSMakeRange(0,length) withString:string];
    
    [astring _clipAndDrawInRect:titleRect];
    
    [astring release];
    [string release];
}

@end
