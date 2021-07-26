/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <AppKit/NSAlertPanel.h>
#import <AppKit/NSStringDrawer.h>
#import <AppKit/NSImage.h>
#import <AppKit/NSApplication.h>
#import <AppKit/NSButton.h>
#import <AppKit/NSTextField.h>
#import <AppKit/NSImageView.h>

@implementation NSAlertPanel

#define BOTTOM_MARGIN 16
#define TOP_MARGIN 16
#define LEFT_MARGIN 16
#define RIGHT_MARGIN 16
#define BUTTONMESSAGE_GAP 20
#define INTERBUTTON_GAP 6
#define IMAGEMESSAGE_GAP 12
#define BUTTON_MARGIN 8

-initWithTitle:(NSString *)title message:(NSString *)message defaultButton:(NSString *)defaultTitle alternateButton:(NSString *)alternateTitle otherButton:(NSString *)otherTitle sheet:(BOOL)sheet {
   NSImage        *image=[NSImage imageNamed:@"NSAlertPanelExclamation"];
   NSSize          imageSize=[image size];
   NSSize          size={640,480};
   NSStringDrawer *drawer=[NSStringDrawer sharedStringDrawer];
   NSSize          messageSize=[drawer sizeOfString:message withAttributes:nil inSize:size];
   NSSize          defaultSize,alternateSize,otherSize;
   NSSize          imageMessageSize=NSZeroSize;
   float           buttonWidth,buttonHeight,buttonTotalWidth,buttonTotalHeight;
   NSSize          panelSize=NSZeroSize;
   NSRect          contentRect,viewFrame;
   NSImageView    *imageView;

   if(defaultTitle==nil)
    defaultTitle= NSLocalizedStringFromTableInBundle(@"OK", nil, [NSBundle bundleForClass: [NSAlertPanel class]], @"");

   defaultSize=[drawer sizeOfString:defaultTitle withAttributes:nil inSize:size];
   buttonWidth=defaultSize.width;
   buttonHeight=defaultSize.height;

   if(alternateTitle==nil)
    alternateSize=NSZeroSize;
   else {
    alternateSize=[drawer sizeOfString:alternateTitle withAttributes:nil inSize:size];
    buttonWidth=MAX(alternateSize.width,buttonWidth);
    buttonHeight=MAX(alternateSize.height,buttonHeight);
   }

   if(otherTitle==nil)
    otherSize=NSZeroSize;
   else {
    otherSize=[drawer sizeOfString:otherTitle withAttributes:nil inSize:size];
    buttonWidth=MAX(otherSize.width,buttonWidth);
    buttonHeight=MAX(otherSize.height,buttonHeight);
   }

   buttonWidth+=BUTTON_MARGIN*2; 
   buttonWidth=MAX(66,buttonWidth);

   buttonTotalWidth=LEFT_MARGIN;
   buttonTotalWidth+=buttonWidth;
   if(alternateTitle!=nil){
    buttonTotalWidth+=INTERBUTTON_GAP;
    buttonTotalWidth+=buttonWidth;
   }
   if(otherTitle!=nil){
    buttonTotalWidth+=INTERBUTTON_GAP;
    buttonTotalWidth+=buttonWidth;
   }
   buttonTotalWidth+=RIGHT_MARGIN;

   if(buttonHeight<24)
    buttonHeight=24;
   if(buttonHeight>24)
    buttonHeight+=12;

   buttonTotalHeight=BOTTOM_MARGIN+buttonHeight+BUTTONMESSAGE_GAP;

   imageMessageSize.width+=LEFT_MARGIN;
   imageMessageSize.width+=imageSize.width;
   imageMessageSize.width+=IMAGEMESSAGE_GAP;
   imageMessageSize.width+=messageSize.width;
   imageMessageSize.width+=RIGHT_MARGIN;
   imageMessageSize.height=MAX(imageSize.height,messageSize.height);
   imageMessageSize.height+=TOP_MARGIN;

   panelSize.width=MAX(imageMessageSize.width,buttonTotalWidth);
   panelSize.height=imageMessageSize.height+buttonTotalHeight;

   contentRect.origin=NSZeroPoint;
   contentRect.size=panelSize;

   self=[self initWithContentRect:contentRect styleMask:sheet?NSDocModalWindowMask:NSTitledWindowMask|NSClosableWindowMask backing:NSBackingStoreBuffered defer:NO];

   if([title length]==0)
    title= NSLocalizedStringFromTableInBundle(@"Alert", nil, [NSBundle bundleForClass: [NSAlertPanel class]], @"Alert panel title");

   [self setTitle:title];
	[self setLevel: NSModalPanelWindowLevel];
	
   viewFrame.origin.y=BOTTOM_MARGIN;
   viewFrame.origin.x=LEFT_MARGIN+floor((panelSize.width-buttonTotalWidth)/2);
   viewFrame.size.width=buttonWidth;
   viewFrame.size.height=buttonHeight;
   _defaultButton=[[[NSButton alloc] initWithFrame:viewFrame] autorelease];
   [_defaultButton setTitle:defaultTitle];
   [_defaultButton setTarget:self];
   [_defaultButton setAction:@selector(defaultButton:)];
   [[self contentView] addSubview:_defaultButton];
   [self setInitialFirstResponder:_defaultButton];
   [self setDefaultButtonCell:[_defaultButton cell]];

   if(alternateTitle!=nil){
    viewFrame.origin.x+=viewFrame.size.width;
    viewFrame.origin.x+=INTERBUTTON_GAP;
    viewFrame.size.width=buttonWidth;
    viewFrame.size.height=buttonHeight;
    _alternateButton=[[[NSButton alloc] initWithFrame:viewFrame] autorelease];
    [_alternateButton setTitle:alternateTitle];
    [_alternateButton setTarget:self];
    [_alternateButton setAction:@selector(alternateButton:)];
    [[self contentView] addSubview:_alternateButton];
    [_defaultButton setNextKeyView:_alternateButton];
   }

   if(otherTitle!=nil){
    viewFrame.origin.x+=viewFrame.size.width;
    viewFrame.origin.x+=INTERBUTTON_GAP;
    viewFrame.size.width=buttonWidth;
    viewFrame.size.height=buttonHeight;
    _otherButton=[[[NSButton alloc] initWithFrame:viewFrame] autorelease];
    [_otherButton setTitle:otherTitle];
    [_otherButton setTarget:self];
    [_otherButton setAction:@selector(otherButton:)];
    [[self contentView] addSubview:_otherButton];
    [_alternateButton setNextKeyView:_otherButton];
    [_otherButton setNextKeyView:_defaultButton];
   }

   viewFrame.origin.x=LEFT_MARGIN;
   viewFrame.origin.y+=viewFrame.size.height;
   viewFrame.origin.y+=BUTTONMESSAGE_GAP;
   if(messageSize.height>imageSize.height)
    viewFrame.origin.y+=messageSize.height-imageSize.height;
   viewFrame.size=imageSize;
   imageView=[[[NSImageView alloc] initWithFrame:viewFrame] autorelease];
   [imageView setImage:image];
   [[self contentView] addSubview:imageView];

   viewFrame.origin.x+=imageSize.width;
   viewFrame.origin.x+=IMAGEMESSAGE_GAP;
   viewFrame.origin.y-=messageSize.height-imageSize.height;
   viewFrame.size=messageSize;
	viewFrame.size.width += 6; // TextField's like to shrink the title area a bit - so make sure it doesn't make it too small for the message!
   _messageText=[[[NSTextField alloc] initWithFrame:viewFrame] autorelease];
   [_messageText setStringValue:message];
   [_messageText setSelectable:YES];
   [_messageText setBordered:NO];
   [_messageText setEditable:NO];
   [[self contentView] addSubview:_messageText];

   return self;
}

-(void)defaultButton:sender {
   if([self isSheet])
    [NSApp endSheet:self returnCode:NSAlertDefaultReturn];
   else
    [NSApp stopModalWithCode:NSAlertDefaultReturn];

   [self close];
}

-(void)alternateButton:sender {
   if([self isSheet])
    [NSApp endSheet:self returnCode:NSAlertAlternateReturn];
   else
    [NSApp stopModalWithCode:NSAlertAlternateReturn];

   [self close];
}

-(void)otherButton:sender {
   if([self isSheet])
    [NSApp endSheet:self returnCode:NSAlertOtherReturn];
   else
    [NSApp stopModalWithCode:NSAlertOtherReturn];

   [self close];
}

@end
