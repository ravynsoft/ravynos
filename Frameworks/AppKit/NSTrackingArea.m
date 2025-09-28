/* Copyright (c) 2006-2007 Christopher J. W. Lloyd
                 2009-2010 Markus Hitter <mah@jump-ing.de>

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

/*
 * This class is also used for ToolTips, legacy TrackingRects and legacy
 * CursorRects. All these do some special assumptions in NSView.m and
 * NSWindow.m and are not for user consumtion.
 *
 * On how to set up these special cases, see NSView.m.
 */

#import <AppKit/NSTrackingArea.h>
#import <AppKit/NSView.h>

@implementation NSTrackingArea

-(id)initWithRect:(NSRect)rect options:(NSTrackingAreaOptions)options owner:(id)owner userInfo:(NSDictionary *)userInfo {
 return [self _initWithRect:rect options:options owner:owner userData:userInfo retainUserData:YES isToolTip:NO isLegacy:NO];
}

-(id)_initWithRect:(NSRect)rect options:(NSTrackingAreaOptions)options owner:(id)owner userData:(void *)userData retainUserData:(BOOL)retainUserData isToolTip:(BOOL)isToolTip isLegacy:(BOOL)legacy {
   self=[super init];
   if(self!=nil){
    _rect=rect;
    _options=options;
    _owner=[owner retain];
    _userData=userData;
    _retainUserData=retainUserData;
    if(_retainUserData)
     _userData=[(id)userData retain];
     
    if(_options&NSTrackingAssumeInside)
     _mouseInside=YES;
    else
     _mouseInside=NO;
    _view=nil;
    _rectInWindow=NSZeroRect;
    _isToolTip=isToolTip;
    _legacy=legacy;
   }
   return self;
}

-(void)dealloc {
   [_owner release];
   if(_retainUserData)
    [(id)_userData release];
   [super dealloc];
}

-(NSRect)rect {
   return _rect;
}

-(NSTrackingAreaOptions)options {
   return _options;
}

-(id)owner {
   return _owner;
}

-(NSDictionary *)userInfo {
   return (id)_userData;
}

-(NSRect)_rectInWindow {
   return _rectInWindow;
}

-(void)_setRectInWindow:(NSRect)rectInWindow {
   _rectInWindow=rectInWindow;
}

-(void)_setRect:(NSRect)rect {
   _rect=rect;
}

-(NSView *)_view {
   return _view;
}

-(void)_setView:(NSView *)newView {
	// Don't retain or we could create a retain loop between the view array of tracking
	// areas and this tracking area
   _view=newView;
}

-(BOOL)_isToolTip {
   return _isToolTip;
}

-(BOOL)_isLegacy {
   return _legacy;
}

-(BOOL)_mouseInside {
   return _mouseInside;
}

-(void)_setMouseInside:(BOOL)inside {
   _mouseInside=inside;
}

-(NSString *)description {
   return [NSString stringWithFormat:@"<%@[0x%lx] rect:%@ options:%d owner:%@ userInfo:%p view:%@ rectInWindow:%@ mouseInside:%@ isToolTip:%@>", [self class], self, NSStringFromRect(_rect), _options, [_owner class], _userData, [_view class], NSStringFromRect(_rectInWindow), _mouseInside ? @"YES" : @"NO", _isToolTip ? @"YES" : @"NO"];
}

@end
