/* Copyright (c) 2006-2007 Christopher J. W. Lloyd
                 2009 Markus Hitter <mah@jump-ing.de>

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <AppKit/NSAlert.h>
#import <AppKit/NSImage.h>
#import <AppKit/NSRaise.h>
#import <Foundation/NSDictionary.h>
#import <AppKit/NSStringDrawer.h>
#import <AppKit/NSButton.h>
#import <AppKit/NSScreen.h>
#import <AppKit/NSApplication.h>
#import <AppKit/NSPanel.h>
#import <AppKit/NSWindow-Private.h>
#import <AppKit/NSImageView.h>
#import <AppKit/NSTextField.h>
#import <AppKit/NSFont.h>
#import <AppKit/NSAttributedString.h>

@implementation NSAlert

/* 
 NSWarningAlertStyle - app icon
 NSInformationalAlertStyle - app icon
 NSCriticalAlertStyle - large yellow /!\ triangle w/ small app icon
 */
 
-init {
   _style=NSWarningAlertStyle;
   _icon=[[NSImage imageNamed:@"NSAlertPanelExclamation"] retain];
	_messageText=[NSLocalizedStringFromTableInBundle(@"Alert", nil, [NSBundle bundleForClass: [NSAlert class]], @"Default message text for NSAlert") copy];
   _informativeText=@"";
   _accessoryView=nil;
   _showsHelp=NO;
   _showsSuppressionButton=NO;
   _helpAnchor=nil;
   _buttons=[NSMutableArray new];   
   _window=[[NSPanel alloc] initWithContentRect:NSMakeRect(0,0,10,10) styleMask:NSTitledWindowMask backing:NSBackingStoreBuffered defer:NO];
   _suppressionButton=[[NSButton alloc] init];
 //  [_suppressionButton setButtonType:NSSwitchButton];
   [_suppressionButton setTitle:NSLocalizedStringFromTableInBundle(@"Do not show this message again", nil, [NSBundle bundleForClass: [NSAlert class]], @"Default NSAlert supression button title")];
   _needsLayout=YES;
   return self;
}

-(void)dealloc {
   [_icon release];
   [_messageText release];
   [_informativeText release];
   [_accessoryView release];
   [_helpAnchor release];
   [_buttons release];
   [_suppressionButton release];
   [_window release];
   [super dealloc];
}

+(NSAlert *)alertWithError:(NSError *)error {
   NSArray  *titles=[error localizedRecoveryOptions];
   NSString *defaultTitle=([titles count]>0)?[titles objectAtIndex:0]:nil;
   NSString *alternateTitle=([titles count]>1)?[titles objectAtIndex:1]:nil;
   NSString *otherTitle=([titles count]>2)?[titles objectAtIndex:2]:nil;
   
   NSAlert *result=[[[self alloc] init] autorelease];
   
   [result setMessageText:[error localizedDescription]];
   [result setInformativeText:[error localizedRecoverySuggestion]];
   int i,count=[titles count];
   for(i=0;i<count;i++)
    [result addButtonWithTitle:[titles objectAtIndex:i]];
    
   return result;    
}

+(NSAlert *)alertWithMessageText:(NSString *)messageText defaultButton:(NSString *)defaultTitle alternateButton:(NSString *)alternateTitle otherButton:(NSString *)otherTitle informativeTextWithFormat:(NSString *)format,... {
   va_list          arguments;
   NSString        *informativeText;

   va_start(arguments,format);

   informativeText=[[[NSString alloc] initWithFormat:format arguments:arguments] autorelease];
   
   NSAlert *result=[[[self alloc] init] autorelease];
   
   [result setMessageText:messageText];
   [result setInformativeText:informativeText];
   if(defaultTitle==nil)
    defaultTitle=NSLocalizedStringFromTableInBundle(@"OK", nil, [NSBundle bundleForClass: [NSAlert class]], @"Default button title for NSAlert");
   [result addButtonWithTitle:defaultTitle];
   if(alternateTitle!=nil)
    [result addButtonWithTitle:alternateTitle];
   if(otherTitle!=nil)
    [result addButtonWithTitle:otherTitle];
    
   return result;
}


-delegate {
   return _delegate;
}

-(NSAlertStyle)alertStyle {
   return _style;
}

-(NSImage *)icon {
   return _icon;
}

-(NSString *)messageText {
   return _messageText;
}

-(NSString *)informativeText {
   return _informativeText;
}

-(NSView *)accessoryView {
   return _accessoryView;
}

-(BOOL)showsHelp {
   return _showsHelp;
}

-(BOOL)showsSuppressionButton {
   return _showsSuppressionButton;
}

-(NSString *)helpAnchor {
   return _helpAnchor;
}

-(NSArray *)buttons {
   return _buttons;
}

-(NSButton *)suppressionButton {
   return _suppressionButton;
}

-window {
   return _window;
}

-(void)setDelegate:delegate {
   _delegate=delegate;
}

-(void)setAlertStyle:(NSAlertStyle)style {
   _style=style;
   _needsLayout=YES;
}

-(void)setIcon:(NSImage *)icon {
   icon=[icon copy];
   [_icon release];
   _icon=icon;
   _needsLayout=YES;
}

-(void)setMessageText:(NSString *)string {
   string=[string copy];
   [_messageText release];
   _messageText=string;
   _needsLayout=YES;
}

-(void)setInformativeText:(NSString *)string {
   string=[string copy];
   [_informativeText release];
   _informativeText=string;
   _needsLayout=YES;
}

-(void)setAccessoryView:(NSView *)value {
   value=[value retain];
   
   [_accessoryView removeFromSuperview];
   [_accessoryView release];
   _accessoryView=value;
   
   // We must add it as a subview here such that a makeFirstResponder: immediately after
   // works properly by setting up the field editor
   [[_window contentView] addSubview:_accessoryView];

   _needsLayout=YES;
}

-(void)setShowsHelp:(BOOL)flag {
   _showsHelp=flag;
   _needsLayout=YES;
}

-(void)setShowsSuppressionButton:(BOOL)value {
   _showsSuppressionButton=value;
   _needsLayout=YES;
}

-(void)setHelpAnchor:(NSString *)anchor {
   anchor=[anchor copy];
   [_helpAnchor release];
   _helpAnchor=anchor;
   _needsLayout=YES;
}

-(NSButton *)addButtonWithTitle:(NSString *)title {
   NSButton *result=[[NSButton alloc] init];
   [result setTitle:title];
   [result setTarget:self];
   [result setAction:@selector(_alertButton:)];
   [result setTag:NSAlertFirstButtonReturn+[_buttons count]];
   [_buttons addObject:result];
   _needsLayout=YES;
   return result;
}

-(void)layout {
#define BOTTOM_MARGIN 16
#define TOP_MARGIN 16
#define LEFT_MARGIN 16
#define RIGHT_MARGIN 16

// A NSTextField adds a small margin around the text.
#define TEXTFIELD_MARGIN 2.

#define INTERTEXT_GAP 8.
#define BUTTON_MARGIN 8
#define INTERBUTTON_GAP 6
#define OTHER_GAP 20
#define ICON_MAIN_GAP 20
#define MAIN_BUTTON_GAP 20

   NSSize screenSize=[[NSScreen mainScreen] visibleFrame].size;
   NSSize textSize=NSZeroSize;
   NSStringDrawer *drawer=[NSStringDrawer sharedStringDrawer];
   NSSize iconSize=(_icon!=nil)?[_icon size]:NSZeroSize;
   NSDictionary *messageAttributes=[NSDictionary dictionaryWithObjectsAndKeys:[NSFont boldSystemFontOfSize:0],NSFontAttributeName,nil];
   NSSize messageSize=NSZeroSize;
   NSDictionary *informativeAttributes=[NSDictionary dictionaryWithObjectsAndKeys:[NSFont systemFontOfSize:0],NSFontAttributeName,nil];
   NSSize informativeSize=NSZeroSize;
   float messageInformativeGap=0.;
   NSDictionary *suppressionAttributes=[NSDictionary dictionaryWithObjectsAndKeys:[NSFont systemFontOfSize:0],NSFontAttributeName,nil];
   NSSize supressionSize=NSZeroSize;
   float informativeSuppressionGap=0.;
   NSSize accessorySize=(_accessoryView!=nil)?[_accessoryView frame].size:NSZeroSize;
   float suppressionAccessoryGap=0.;
   NSSize mainSize=NSZeroSize;
   NSSize panelSize=NSZeroSize;
   int i,count=[_buttons count];
   NSSize okCancelButtonSize=NSMakeSize(40,24);
   NSSize otherButtonSize=okCancelButtonSize;
   NSSize allButtonsSize=NSZeroSize;

   // Size the buttons.
   for(i=0;i<count && i<2;i++){
    NSButton *check=[_buttons objectAtIndex:i];
    NSAttributedString *title=[check attributedTitle];
    NSSize size=[drawer sizeOfAttributedString:title inSize:NSZeroSize];

    okCancelButtonSize.width=MAX(size.width+BUTTON_MARGIN*2,okCancelButtonSize.width);
    okCancelButtonSize.height=MAX(size.height,okCancelButtonSize.height);
   }

   for(i=0;i<count && i<2;i++)
    [[_buttons objectAtIndex:i] setFrameSize:okCancelButtonSize];

   for(i=2;i<count;i++){
    NSButton *check=[_buttons objectAtIndex:i];
    NSAttributedString *title=[check attributedTitle];
    NSSize size=[drawer sizeOfAttributedString:title inSize:NSZeroSize];

    otherButtonSize.width=MAX(size.width+BUTTON_MARGIN*2,otherButtonSize.width);
    otherButtonSize.height=MAX(size.height,otherButtonSize.height);
   }

   for(i=2;i<count;i++)
    [[_buttons objectAtIndex:i] setFrameSize:otherButtonSize];

   for(i=0;i<count;i++){
    NSButton *button=[_buttons objectAtIndex:i];

    allButtonsSize.width+=[button frame].size.width;
    allButtonsSize.height=MAX(allButtonsSize.height,[button frame].size.height);

    allButtonsSize.width+=INTERBUTTON_GAP;
    if(i==1)
     allButtonsSize.width+=OTHER_GAP;
   }

   // Size the main reasonable and at least wide enough to make all buttons fit.
   mainSize.width=MAX(screenSize.width/3.,allButtonsSize.width-iconSize.width-ICON_MAIN_GAP);
   mainSize.width=MAX(mainSize.width,accessorySize.width);

   // Size the texts.
   textSize=NSMakeSize(mainSize.width-TEXTFIELD_MARGIN*2,NSStringDrawerLargeDimension);

   if(_messageText!=nil){
    messageSize=[drawer sizeOfString:_messageText withAttributes:messageAttributes inSize:textSize];
    messageSize.width+=TEXTFIELD_MARGIN*2;
    messageSize.height+=TEXTFIELD_MARGIN*2;
   }

   if(_informativeText!=nil){
    informativeSize=[drawer sizeOfString:_informativeText withAttributes:informativeAttributes inSize:textSize];
    informativeSize.width+=TEXTFIELD_MARGIN*2;
    informativeSize.height+=TEXTFIELD_MARGIN*2;
   }

   if(_messageText!=nil && _informativeText!=nil)
    messageInformativeGap=INTERTEXT_GAP;

   if(_showsSuppressionButton)
    supressionSize=[drawer sizeOfString:[_suppressionButton title] withAttributes:suppressionAttributes inSize:textSize];

   if(_informativeText!=nil && _showsSuppressionButton)
    informativeSuppressionGap=INTERTEXT_GAP;

   if(_showsSuppressionButton && _accessoryView!=nil)
    suppressionAccessoryGap=INTERTEXT_GAP;

   // Now we can size the heights as well.
   mainSize.height=messageSize.height+informativeSize.height+messageInformativeGap+supressionSize.height+informativeSuppressionGap+accessorySize.height+suppressionAccessoryGap;

   panelSize.width=LEFT_MARGIN+mainSize.width+iconSize.width+ICON_MAIN_GAP+RIGHT_MARGIN;
   panelSize.height=TOP_MARGIN+MAX(mainSize.height,iconSize.height)+MAIN_BUTTON_GAP+allButtonsSize.height+BOTTOM_MARGIN;

  if(_icon!=nil){
   NSRect       frame;
   NSImageView *imageView;
   
   frame.origin.x=LEFT_MARGIN;
   frame.origin.y=panelSize.height-TOP_MARGIN-iconSize.height;
   frame.size=iconSize;
   imageView=[[[NSImageView alloc] initWithFrame:frame] autorelease];
   [imageView setImage:_icon];
   [[_window contentView] addSubview:imageView];
  }
  
  if(_messageText!=nil){
   NSRect       frame;
   NSTextField *textField;
   
   frame.origin.x=LEFT_MARGIN+iconSize.width+ICON_MAIN_GAP;
   frame.origin.y=panelSize.height-TOP_MARGIN-messageSize.height;
   frame.size=messageSize;
   textField=[[[NSTextField alloc] initWithFrame:frame] autorelease];
   [textField setAttributedStringValue:[[[NSAttributedString alloc] initWithString:_messageText attributes:messageAttributes] autorelease]];
   [textField setEditable:NO];
   [textField setSelectable:YES];
   [textField setBordered:NO];
   [[_window contentView] addSubview:textField];
  }
  
  if(_informativeText!=nil){
   NSRect       frame;
   NSTextField *textField;
   
   frame.origin.x=LEFT_MARGIN+iconSize.width+ICON_MAIN_GAP;
   frame.origin.y=panelSize.height-TOP_MARGIN-messageSize.height-messageInformativeGap-informativeSize.height;
   frame.size=informativeSize;
   textField=[[[NSTextField alloc] initWithFrame:frame] autorelease];
   [textField setStringValue:[[[NSAttributedString alloc] initWithString:_informativeText attributes:informativeAttributes] autorelease]];
   [textField setEditable:NO];
   [textField setSelectable:YES];
   [textField setBordered:NO];
   [[_window contentView] addSubview:textField];
  }

  if(_showsSuppressionButton){
   NSRect frame;
   
   frame.origin.x=LEFT_MARGIN+iconSize.width+ICON_MAIN_GAP;
   frame.origin.y=panelSize.height-TOP_MARGIN-messageSize.height-messageInformativeGap-informativeSize.height-informativeSuppressionGap-frame.size.height;
   frame.size=supressionSize;
   [_suppressionButton setFrame:frame];
   [[_window contentView] addSubview:_suppressionButton];
  }

  if(_accessoryView!=nil){
   NSRect frame=[_accessoryView frame];
   
   frame.origin.x=LEFT_MARGIN+iconSize.width+ICON_MAIN_GAP;
   frame.origin.y=panelSize.height-TOP_MARGIN-messageSize.height-messageInformativeGap-informativeSize.height-informativeSuppressionGap-supressionSize.height-suppressionAccessoryGap-frame.size.height;
   [_accessoryView setFrame:frame];
  }

  NSPoint origin={panelSize.width-RIGHT_MARGIN,BOTTOM_MARGIN};
  
  for(i=0;i<count;i++){
   NSButton *button=[_buttons objectAtIndex:i];
   NSSize    bSize=[button frame].size;
   
   origin.x-=bSize.width;
   [button setFrameOrigin:origin];
   origin.x-=INTERBUTTON_GAP;
   if(i==1)
    origin.x-=OTHER_GAP;
   [[_window contentView] addSubview:button];
  }

  NSRect contentRect={{0,0},panelSize};

  NSRect frame=[_window frameRectForContentRect:contentRect];

	// This stops the platform window from trying to impose a different size on us
	// Which for some reason it wants to do.
	[_window setMinSize: frame.size]; 
	[_window setMaxSize: frame.size];
	
  [_window setFrame:frame display:NO];
  _needsLayout=NO;
}

-(void)layoutIfNeeded {
// This isn't an optimization per se, it is to prevent relayout after a manual layout
   if(_needsLayout){
    if([_buttons count]==0){
     [self addButtonWithTitle:NSLocalizedStringFromTableInBundle(@"OK", nil, [NSBundle bundleForClass: [NSAlert class]], @"Default button title for NSAlert")];
    }
    
    [self layout];
   }
}

-(void)sheetDidEnd:(NSWindow *)sheet returnCode:(int)returnCode contextInfo:(void *)contextInfo {
   typedef void (*alertDidEnd)(id,SEL,NSAlert *,int,void *);
   if (_sheetDidEnd) {
    alertDidEnd endFunction=(alertDidEnd)[_sheetDelegate methodForSelector:_sheetDidEnd];

    endFunction(_sheetDelegate,_sheetDidEnd,self,returnCode,contextInfo);
   }
   [self release];
}

-(void)beginSheetModalForWindow:(NSWindow *)window modalDelegate:delegate didEndSelector:(SEL)selector contextInfo:(void *)info {
   [_window setStyleMask:NSDocModalWindowMask];
   [self layoutIfNeeded];
   [_window setDefaultButtonCell:[[_buttons objectAtIndex:0] cell]];
   _sheetDelegate=delegate;
   _sheetDidEnd=selector;

   [self retain];
   [NSApp beginSheet:_window modalForWindow:window modalDelegate:self didEndSelector:@selector(sheetDidEnd:returnCode:contextInfo:) contextInfo:info];
}

-(NSInteger)runModal {
	[_window setLevel: NSModalPanelWindowLevel];
   [_window setStyleMask:NSTitledWindowMask];
   [self layoutIfNeeded];
   [_window setDefaultButtonCell:[[_buttons objectAtIndex:0] cell]];
   return [NSApp runModalForWindow:_window];
}

-(void)_alertButton:sender {
   if([_window isSheet])
    [NSApp endSheet:_window returnCode:[sender tag]];
   else
    [NSApp stopModalWithCode:[sender tag]];

    [_window close];
   }

@end
