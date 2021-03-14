/* Copyright (c) 2006-2007 Christopher J. W. Lloyd <cjwl@objc.net>

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <AppKit/NSPanel.h>
#import <AppKit/NSAlertPanel.h>
#import <AppKit/NSApplication.h>
#import <CoreGraphics/CGWindow.h>

@implementation NSPanel

+(BOOL)hasMainMenuForStyleMask:(NSUInteger)styleMask {
    return NO;
}

-initWithContentRect:(NSRect)contentRect styleMask:(unsigned int)styleMask backing:(unsigned)backing defer:(BOOL)defer {
   [super initWithContentRect:contentRect styleMask:styleMask backing:backing defer:defer];
	if ((styleMask & NSUtilityWindowMask) ||
		(styleMask & NSDocModalWindowMask)) {
		_level = NSFloatingWindowLevel; // We want these panels to be above normal windows - so they don't get lost!
	} else {
		_level = NSNormalWindowLevel;
	}
   _releaseWhenClosed=NO;
   return self;
}

-(BOOL)canBecomeMainWindow {
   return NO;
}

-(void)becomeMainWindow {
   if([self canBecomeMainWindow])
    [super becomeMainWindow];
}

-(BOOL)worksWhenModal {
   return _worksWhenModal;
}

-(BOOL)becomesKeyOnlyIfNeeded {
   return _becomesKeyOnlyIfNeeded;
}

-(BOOL)isFloatingPanel {
   return _isFloatingPanel;
}

-(void)setWorksWhenModal:(BOOL)flag {
   _worksWhenModal=flag;
}

-(void)setBecomesKeyOnlyIfNeeded:(BOOL)flag {
   _becomesKeyOnlyIfNeeded=flag;
}

-(void)setFloatingPanel:(BOOL)flag {
   _isFloatingPanel=flag;
}

@end

int NSRunAlertPanel(NSString *title, NSString *format, NSString *defaultButton, NSString *alternateButton, NSString *otherButton, ...) {
   va_list          arguments;
   NSString        *message;
   NSPanel         *panel;
   int              result;

   va_start(arguments,otherButton);

   message=[[[NSString alloc] initWithFormat:format arguments:arguments] autorelease];

   panel=[[NSAlertPanel alloc] initWithTitle:title message:message defaultButton:defaultButton alternateButton:alternateButton otherButton:otherButton sheet:NO];

   result=[NSApp runModalForWindow:panel];

   [panel release];

   return result;
}

int NSRunInformationalAlertPanel(NSString *title, NSString *msgFormat, NSString *defaultButton, NSString *alternateButton, NSString *otherButton, ...) {

    // FIXME: should have a different icon
	va_list          arguments;
	NSString        *message;
    
	va_start(arguments,otherButton);
	
	message=[[[NSString alloc] initWithFormat:msgFormat arguments:arguments] autorelease];
    
    return NSRunAlertPanel(title, msgFormat, defaultButton, alternateButton, otherButton);
}

int NSRunCriticalAlertPanel(NSString *title, NSString *msgFormat, NSString *defaultButton, NSString *alternateButton, NSString *otherButton, ...) {
    
    // FIXME: should have a different icon
    va_list          arguments;
    NSString        *message;

    va_start(arguments,otherButton);

    message=[[[NSString alloc] initWithFormat:msgFormat arguments:arguments] autorelease];

    return NSRunAlertPanel(title, msgFormat, defaultButton, alternateButton, otherButton);
}

void NSBeginAlertSheet(NSString *title,NSString *defaultButton,NSString *alternateButton,NSString *otherButton,NSWindow *window, id modalDelegate,SEL didEndSelector,SEL didDismissSelector,void *contextInfo,NSString *format,...) {
   va_list          arguments;
   NSString        *message;
   NSPanel         *panel;

   va_start(arguments,format);

   message=[[[NSString alloc] initWithFormat:format arguments:arguments] autorelease];

   panel=[[[NSAlertPanel alloc] initWithTitle:title message:message defaultButton:defaultButton alternateButton:alternateButton otherButton:otherButton sheet:YES] autorelease];

   [NSApp beginSheet:panel modalForWindow:window modalDelegate:modalDelegate didEndSelector:didEndSelector contextInfo:contextInfo];
}

void NSBeginCriticalAlertSheet(NSString *title,NSString *defaultButton,NSString *alternateButton,NSString *otherButton,NSWindow *window, id modalDelegate,SEL didEndSelector,SEL didDismissSelector,void *contextInfo,NSString *format,...)
{
	// FIXME: should probably have different icon or so
	
	va_list          arguments;
	NSString        *message;

	va_start(arguments,format);
	
	message=[[[NSString alloc] initWithFormat:format arguments:arguments] autorelease];
	
	NSBeginAlertSheet(title, defaultButton, alternateButton,otherButton,window, modalDelegate, didEndSelector,didDismissSelector,contextInfo,message);
}

void NSBeginInformationalAlertSheet(NSString *title,NSString *defaultButton,NSString *alternateButton,NSString *otherButton,NSWindow *window, id modalDelegate,SEL didEndSelector,SEL didDismissSelector,void *contextInfo,NSString *format,...)
{
	// FIXME: should probably have different icon or so
	
	va_list          arguments;
	NSString        *message;
	
	va_start(arguments,format);
	
	message=[[[NSString alloc] initWithFormat:format arguments:arguments] autorelease];
	
	NSBeginAlertSheet(title, defaultButton, alternateButton,otherButton,window, modalDelegate, didEndSelector,didDismissSelector,contextInfo,message);
}
