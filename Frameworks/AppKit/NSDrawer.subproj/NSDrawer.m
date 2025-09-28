/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

// Original - David Young <daver@geeks.org>
#import <AppKit/AppKitExport.h>
#import <AppKit/NSDrawer.h>
#import <AppKit/NSWindow-Private.h>
#import <AppKit/NSScreen.h>
#import <AppKit/NSDrawerWindow.h>
#import <Foundation/NSKeyedArchiver.h>

NSString * const NSDrawerWillOpenNotification = @"NSDrawerWillOpenNotification";
NSString * const NSDrawerDidOpenNotification = @"NSDrawerDidOpenNotification";
NSString * const NSDrawerWillCloseNotification = @"NSDrawerWillCloseNotification";
NSString * const NSDrawerDidCloseNotification = @"NSDrawerDidCloseNotification";

@implementation NSDrawer

- (BOOL)acceptsFirstResponder {
    return YES;
}

- (NSResponder *)nextResponder {
    return [_drawerWindow firstResponder];
}

// - modify the leading/trailing offset portions of the drawer's geometry.
// - constrain the "expandable" portion of the drawer to the min/max content sizes.
+ (NSRect)drawerFrameWithContentSize:(NSSize)contentSize parentWindow:(NSWindow *)parentWindow leadingOffset:(float)leadingOffset trailingOffset:(float)trailingOffset edge:(NSRectEdge)edge state:(NSDrawerState)state {
	 NSRect parentFrame       = [parentWindow frame];
	 NSRect parentContentRect = [parentWindow contentRectForFrameRect:parentFrame];
	 NSRect drawerFrame       = [NSWindow frameRectForContentRect:NSMakeRect(0, 0, contentSize.width, contentSize.height) styleMask:NSDrawerWindowMask];
    
    if (edge == NSMinXEdge || edge == NSMaxXEdge) {
        drawerFrame.origin.x = parentFrame.origin.x - 12.0;
        drawerFrame.origin.y = parentContentRect.origin.y + trailingOffset - 12.0;
        drawerFrame.size.height = parentContentRect.size.height - (leadingOffset + trailingOffset) + 9.0;
    }
    else {
        drawerFrame.origin.x = parentContentRect.origin.x + leadingOffset - 12.0;
        drawerFrame.origin.y = parentContentRect.origin.y - 12.0;
        drawerFrame.size.width = parentContentRect.size.width - (leadingOffset + trailingOffset);
    }
    
    // Initially I was only computing the open-state frame. Code added to compute closed state...
    switch (edge) {
        case NSMinXEdge:
            if (state != NSDrawerClosedState)
                drawerFrame.origin.x -= drawerFrame.size.width;
            break;
            
        case NSMaxXEdge:
            drawerFrame.origin.x += parentFrame.size.width;
            if (state == NSDrawerClosedState)
                drawerFrame.origin.x -= drawerFrame.size.width;
			break;
            
        case NSMinYEdge:
            if (state != NSDrawerClosedState)
                drawerFrame.origin.y -= drawerFrame.size.height;
            break;
            
        case NSMaxYEdge:
            drawerFrame.origin.y += parentFrame.size.height;
            if (state == NSDrawerClosedState)
                drawerFrame.origin.y -= drawerFrame.size.height;
			break;
            
        default:
            break;
    }
	
    return drawerFrame;
}

// compute the proper edge on which to open based upon the dimensions of the parent window, its screen, the drawer and its preferred edge. choose the preferred edge if possible, then the opposite edge if possible, then the preferred edge if neither is possible.
+ (NSRectEdge)visibleEdgeWithPreferredEdge:(NSRectEdge)preferredEdge parentWindow:(NSWindow *)parentWindow drawerWindow:(NSWindow *)drawerWindow {
    NSRect screenRect = [[parentWindow screen] visibleFrame];
    NSRect parentRect = [parentWindow frame];
    NSRect drawerRect = [drawerWindow frame];
    
    switch (preferredEdge) {
        case NSMinXEdge:
            if (parentRect.origin.x - screenRect.origin.x < drawerRect.size.width)
                if (NSMaxX(screenRect) - NSMaxX(parentRect) > drawerRect.size.width)
                    return NSMaxXEdge;
            
            return NSMinXEdge;
            
        case NSMaxXEdge:
            if (NSMaxX(screenRect) - NSMaxX(parentRect) < drawerRect.size.width)
                if (parentRect.origin.x - screenRect.origin.x > drawerRect.size.width)
                    return NSMinXEdge;
            
            return NSMaxXEdge;
            
        case NSMinYEdge:
            if (parentRect.origin.y - screenRect.origin.y < drawerRect.size.height)
                if (NSMaxY(screenRect) - NSMaxY(parentRect) > drawerRect.size.height)
                    return NSMaxYEdge;
            
            return NSMinYEdge;
            
        case NSMaxYEdge:
            if (NSMaxY(screenRect) - NSMaxY(parentRect) < drawerRect.size.height)
                if (parentRect.origin.y - screenRect.origin.y > drawerRect.size.height)
                    return NSMinYEdge;
            
            return NSMinXEdge;
            
        default:
            return NSMaxXEdge;
    }
}

- (id)initWithCoder:(NSCoder *)coder
{
   self = [super initWithCoder:coder];

   if (self && [coder allowsKeyedCoding])
   {
      NSKeyedUnarchiver *keyed = (NSKeyedUnarchiver *)coder;

      if([keyed containsValueForKey:@"NSContentSize"])
         _contentSize = [keyed decodeSizeForKey:@"NSContentSize"];
      else
         _contentSize = NSZeroSize;

      _state = 0;
      _edge  = 0;
      if([keyed containsValueForKey:@"NSPreferredEdge"])
         [self setPreferredEdge:[keyed decodeIntForKey:@"NSPreferredEdge"]];
      else
         _preferredEdge = 0;

      if([keyed containsValueForKey:@"NSLeadingOffset"])
         [self setLeadingOffset:[keyed decodeFloatForKey:@"NSLeadingOffset"]];
      else
         _leadingOffset = 0;
 
      if([keyed containsValueForKey:@"NSTrailingOffset"])
         [self setTrailingOffset:[keyed decodeFloatForKey:@"NSTrailingOffset"]];
      else
         _trailingOffset = 0;

      _drawerWindow = [[[NSDrawerWindow alloc] initWithContentRect:NSMakeRect(0, 0, _contentSize.width, _contentSize.height) styleMask:NSDrawerWindowMask backing:NSBackingStoreBuffered defer:NO] retain];
      [_drawerWindow setDrawer:self];
      [self setContentSize:_contentSize];

      _parentWindow = _nextParentWindow = nil;
      if([keyed containsValueForKey:@"NSParentWindow"])
         [self setParentWindow:[keyed decodeObjectForKey:@"NSParentWindow"]];

      if([keyed containsValueForKey:@"NSDelegate"])
         [self setDelegate:[keyed decodeObjectForKey:@"NSDelegate"]];
      else
         _delegate = nil;

      if([keyed containsValueForKey:@"NSMinContentSize"])
      {
         [self setMinContentSize:[keyed decodeSizeForKey:@"NSMinContentSize"]];
         _minContentSize.width += 12.0;
         _minContentSize.height += 12.0;
      }
      else
         _minContentSize = NSZeroSize;

      if([keyed containsValueForKey:@"NSMaxContentSize"])
      {
         [self setMaxContentSize:[keyed decodeSizeForKey:@"NSMaxContentSize"]];
         _maxContentSize.width += 12.0;
         _maxContentSize.height += 12.0;
      }
      else
         _maxContentSize = NSZeroSize;
   }

   return self;
}


- (id)initWithContentSize:(NSSize)contentSize preferredEdge:(NSRectEdge)edge {
    _drawerWindow = [[NSDrawerWindow alloc] initWithContentRect:NSMakeRect(0, 0, contentSize.width, contentSize.height) styleMask:NSDrawerWindowMask backing:NSBackingStoreBuffered defer:NO];
    [_drawerWindow setDrawer:self];
    [self setContentSize:contentSize];
    [self setPreferredEdge:edge];
    _parentWindow = _nextParentWindow = nil;

    return self;
}

- (void)dealloc {
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    [self setDelegate:nil];
    [_drawerWindow release];
    [super dealloc];
}

- (id)delegate {
    return _delegate;
}

- (NSWindow *)parentWindow {
    return _parentWindow;
}

- (NSView *)contentView {
    return [_drawerWindow contentView];
}

- (NSSize)contentSize {
    return [[self contentView] frame].size;
}

- (NSSize)minContentSize {
    return _minContentSize;
}

- (NSSize)maxContentSize {
    return _maxContentSize;
}

- (float)leadingOffset {
    return _leadingOffset;
}

- (float)trailingOffset {
    return _trailingOffset;
}

- (NSRectEdge)preferredEdge {
    return _preferredEdge;
}

- (int)state {
    return _state;
}

- (NSRectEdge)edge {
    return _edge;
}

- (void)setDelegate:delegate {
    struct {
        NSString *name;
        SEL       selector;
    } notes[]={
        { NSDrawerDidCloseNotification,@selector(drawerDidClose:) },
        { NSDrawerDidOpenNotification ,@selector(drawerDidOpen:) },
        { NSDrawerWillCloseNotification,@selector(drawerWillClose:) },
        { NSDrawerWillOpenNotification,@selector(drawerWillOpen:) },
        { nil, NULL }
    };

    int i;
    
    if(_delegate!=nil)
        for (i = 0; notes[i].name != nil; i++)
            [[NSNotificationCenter defaultCenter] removeObserver:_delegate name:notes[i].name object:self];
    
    _delegate = delegate;
    
    for (i = 0; notes[i].name != nil; i++)
		if ([_delegate respondsToSelector:notes[i].selector])
			[[NSNotificationCenter defaultCenter] addObserver:_delegate selector:notes[i].selector name:notes[i].name object:self];
}

- (void)setNextParentWindow {
    if (_nextParentWindow == (NSWindow *)-1)
        _nextParentWindow = nil;
        
    [_parentWindow _detachDrawer:self];
    [_parentWindow release];
    
    if ((_parentWindow = [_nextParentWindow retain]))
        [_parentWindow _attachDrawer:self];

    _nextParentWindow = nil;
}

- (void)setParentWindow:(NSWindow *)window {
    if (window != _parentWindow)
    {
       _nextParentWindow = window;
       
       if (_state == NSDrawerClosedState)
           [self setNextParentWindow];
                                               // otherwise postpone action until drawer is closed
       else if (_nextParentWindow == nil)      // for the postponed action case make 
           _nextParentWindow = (NSWindow *)-1; // _nextParentWindow != nil so that setNextParentWindow
    }                                          // eventually becomes called later
}

- (void)setContentView:(NSView *)view {
    [_drawerWindow setContentView:view];
}

- (void)setContentSize:(NSSize)size {
   [[self contentView] setFrameSize:size];
   _contentSize = size;
}

- (void)setMinContentSize:(NSSize)size {
    _minContentSize = size;
}

- (void)setMaxContentSize:(NSSize)size {
    _maxContentSize = size;
}

- (void)setPreferredEdge:(NSRectEdge)edge {
    _preferredEdge = edge;
}

- (void)setLeadingOffset:(float)offset {
    _leadingOffset = offset;
}

- (void)setTrailingOffset:(float)offset {
    _trailingOffset = offset;
}

- (void)open {
    [self openOnEdge:[[self class] visibleEdgeWithPreferredEdge:_preferredEdge parentWindow:_parentWindow drawerWindow:_drawerWindow]];
}

- (void)_resetWindowOrdering:sender {
    [_drawerWindow orderWindow:NSWindowAbove relativeTo:[_parentWindow windowNumber]];
    [_drawerWindow orderWindow:NSWindowBelow relativeTo:[_parentWindow windowNumber]];
}

- (void)openOnEdge:(NSRectEdge)edge {
    NSRect start, frame;

    if ([self state] != NSDrawerClosedState)
        return;
    
    [_parentWindow makeFirstResponder:self];

    // if we've moved to a different edge, recompute...
    if (_edge != edge)
        [self setContentSize:[self drawerWindow:_drawerWindow constrainSize:[self contentSize] edge:edge]];
    
    _edge = edge;
    
	[[NSNotificationCenter defaultCenter] postNotificationName:NSDrawerWillOpenNotification object:self];

    frame = [[self class] drawerFrameWithContentSize:[self contentSize] parentWindow:[self parentWindow] leadingOffset:_leadingOffset trailingOffset:_trailingOffset edge:_edge state:NSDrawerOpenState];
    [self setContentSize:frame.size];

    // OK. setup and slide the drawer out
    start = [[self class] drawerFrameWithContentSize:[self contentSize] parentWindow:[self parentWindow] leadingOffset:_leadingOffset trailingOffset:_trailingOffset edge:_edge state:NSDrawerClosedState];
    
    frame.size = start.size = [self drawerWindow:_drawerWindow constrainSize:frame.size edge:_edge];
//    frame.size = [self drawerWindow:_drawerWindow constrainSize:frame.size];
//    start.size = [self drawerWindow:_drawerWindow constrainSize:start.size];
    [_drawerWindow setFrame:start display:YES animate:NO];
    
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(drawerDidOpen:) name:NSWindowDidAnimateNotification object:_drawerWindow];
    _state = NSDrawerOpeningState;
    [self _resetWindowOrdering:nil];
    [_drawerWindow setFrame:frame display:YES animate:YES];
}

// a bit easier time of it as the much of the setup is already done.
// FIXME: in general either make sure contentSize stays in sync, or do this from the open frame...
- (void)close {
    NSRect frame;
    
    if ([self state] != NSDrawerOpenState)
        return;

    [_parentWindow endEditingFor:nil];
    [_parentWindow makeFirstResponder:_parentWindow];

    frame = [[self class] drawerFrameWithContentSize:[self contentSize] parentWindow:[self parentWindow] leadingOffset:_leadingOffset trailingOffset:_trailingOffset edge:_edge state:NSDrawerClosedState];
    frame.size   = [self drawerWindow:_drawerWindow constrainSize:frame.size edge:_edge];

	[[NSNotificationCenter defaultCenter] postNotificationName:NSDrawerWillCloseNotification object:self];

    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(drawerDidClose:) name:NSWindowDidAnimateNotification object:_drawerWindow];
    _state = NSDrawerClosingState;
    
    [_drawerWindow setFrame:frame display:YES animate:YES];
}

- (void)open:sender {
    if ([_delegate respondsToSelector:@selector(drawerShouldOpen:)])
        if ([_delegate drawerShouldOpen:self] == NO)
            return;
    
    [self open];
}

- (void)close:sender {
    if ([_delegate respondsToSelector:@selector(drawerShouldClose:)])
        if ([_delegate drawerShouldClose:self] == NO)
            return;

    [self close];
}

// FIXME: add support for transient states
- (void)toggle:sender {
    switch ([self state]) {
        case NSDrawerOpenState:     [self close:self];   break;
        case NSDrawerClosedState:   [self open:self];    break;
            
        case NSDrawerOpeningState:
        case NSDrawerClosingState:
        default:
            break;
    }
}

- (void)parentWindowDidActivate:(NSWindow *)window {
    if (_state == NSDrawerOpenState)
        [self _resetWindowOrdering:nil];
}

- (void)parentWindowDidDeactivate:(NSWindow *)window {
    if (_state == NSDrawerOpenState && ![_parentWindow isMiniaturized])
        [self _resetWindowOrdering:nil];
}

- (void)parentWindowDidChangeFrame:(NSWindow *)window {
    if (_state == NSDrawerOpenState) {
        NSRect frame = [[self class] drawerFrameWithContentSize:[self contentSize] parentWindow:[self parentWindow] leadingOffset:_leadingOffset trailingOffset:_trailingOffset edge:_edge state:_state];

        if (_edge == NSMinXEdge || _edge == NSMaxXEdge) {
            if (frame.size.width > _maxContentSize.width && _maxContentSize.width > 0)
                frame.size.width = _maxContentSize.width;
        }
        else {
            if (frame.size.height > _maxContentSize.height && _maxContentSize.height > 0)
                frame.size.height = _maxContentSize.height;
        }
        
        [_drawerWindow setFrame:frame display:YES];
    }
}

- (void)parentWindowDidExitMove:(NSWindow *)window {
    if (_state == NSDrawerOpenState)
        [self _resetWindowOrdering:nil];
}

- (void)parentWindowDidMiniaturize:(NSWindow *)window {
    if (_state == NSDrawerOpenState)
        [_drawerWindow orderOut:nil];
}

- (void)parentWindowDidDeminiaturize:(NSWindow *)window {
}

- (void)parentWindowDidClose:(NSWindow *)window {
    if (_state == NSDrawerOpenState)
       [_drawerWindow orderOut:nil];
}

- (void)drawerWindowDidActivate:(NSDrawerWindow *)window {
    if (_state == NSDrawerOpenState  && ![_parentWindow isMiniaturized])
       [self _resetWindowOrdering:nil];
}

- (NSSize)drawerWindow:(NSDrawerWindow *)window constrainSize:(NSSize)size edge:(NSRectEdge)edge {
    if (edge == NSMinXEdge || edge == NSMaxXEdge) {
        if (size.width > _maxContentSize.width && _maxContentSize.width > 0)
            size.width = _maxContentSize.width;
        
        size.height = [self contentSize].height;
    }
    else {
        if (size.height > _maxContentSize.height && _maxContentSize.height > 0)
            size.height = _maxContentSize.height;
        
        size.width = [self contentSize].width;
    }
    
    [self setContentSize:size];
    
    return size;
}

// close the drawer when size < minContentSize. reset _contentSize for the next go
- (void)drawerWindowDidResize:(NSDrawerWindow *)window {
    NSSize size = [window frame].size;
    
    [self _resetWindowOrdering:nil];

    if (_edge == NSMinXEdge || _edge == NSMaxXEdge) {
        if (size.width < _minContentSize.width) {
            [self close];
            _contentSize.width = _minContentSize.width;
            
        }
    }
    else {
        if (size.height < _minContentSize.height) {
            [self close];
            _contentSize.height = _minContentSize.height;
        }
    }

    [self setContentSize:_contentSize];
}

// ?
- (void)drawerDidOpen:(NSNotification *)note {
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    _state = NSDrawerOpenState;
	[[NSNotificationCenter defaultCenter] postNotificationName:NSDrawerDidOpenNotification object:self];
}

- (void)drawerDidClose:(NSNotification *)nilObject {
	[[NSNotificationCenter defaultCenter] removeObserver:self];
	[_drawerWindow orderOut:nil];        
	_state = NSDrawerClosedState;
	[[NSNotificationCenter defaultCenter] postNotificationName:NSDrawerDidCloseNotification object:self];
	
	if (_nextParentWindow != nil)
		[self setNextParentWindow];    
}

@end

