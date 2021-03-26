/* Copyright (c) 2007 Christopher J. W. Lloyd, 2008 Johannes Fortmann

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import "NSSegmentItem.h"
#import <Foundation/NSString.h>
#import <AppKit/NSMenu.h>
#import <AppKit/NSImage.h>

@implementation NSSegmentItem

- (id)init {
	if ((self = [super init])) {
		_isEnabled = YES;
	}
	return self;
}

- (id)initWithCoder:(NSCoder *)decoder {
   _label=[[decoder decodeObjectForKey:@"NSSegmentItemLabel"] retain];
   _image=[[decoder decodeObjectForKey:@"NSSegmentItemImage"] retain];
   _isEnabled=![decoder decodeBoolForKey:@"NSSegmentItemDisabled"];
   _imageScaling=[decoder decodeIntForKey:@"NSSegmentItemImageScaling"];
   _isSelected=[decoder decodeBoolForKey:@"NSSegmentItemSelected"];
   _tag=[decoder decodeIntForKey:@"NSSegmentItemTag"];
   _width=[decoder decodeFloatForKey:@"NSSegmentItemWidth"];
   return self;
}

-(id)description {
   return [NSString stringWithFormat:@"%@ - %@ (%f) [%d]", [super description], _label, _width, _tag];
}

-(void)dealloc {
   [_image release];
   [_label release];
   [_menu release];
   [_toolTip release];
   [super dealloc];
}

-(int)tag {
   return _tag;
}

-(NSImage *)image {
   return _image;
}

-(NSImageScaling)imageScaling {
   return _imageScaling;
}

-(BOOL)isEnabled {
   return _isEnabled;
}

-(BOOL)isSelected {
   return _isSelected;
}

-(NSString *)label {
   return _label;
}

-(NSMenu *)menu {
   return _menu;
}

-(NSString *)toolTip {
   return _toolTip;
}

-(CGFloat)width {
   return _width;
}

-(void)setTag:(int)tag {
   _tag=tag;
}

-(void)setImage:(NSImage *)image {
   image=[image retain];
   [_image release];
   _image=image;
}

-(void)setEnabled:(BOOL)flag {
   _isEnabled=flag;
}

-(void)setSelected:(BOOL)flag {
   _isSelected=flag;
}

-(void)setLabel:(NSString *)label {
   label=[label copy];
   [_label release];
   _label=label;
}

-(void)setMenu:(NSMenu *)menu {
   menu=[menu retain];
   [_menu release];
   _menu=menu;
}

-(void)setToolTip:(NSString *)toolTip {
   toolTip=[toolTip copy];
   [_toolTip release];
   _toolTip=toolTip;
}

-(void)setWidth:(CGFloat)width {
   _width=width;
}

-(void)setImageScaling:(NSImageScaling)value {
   _imageScaling=value;
}

@end
