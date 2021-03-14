/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

// Original - David Young <daver@geeks.org>
#import <AppKit/NSNibHelpConnector.h>
#import <AppKit/NSView.h>

@implementation NSNibHelpConnector
- (void)dealloc
{
	[_file release];
	[_marker release];
	[super dealloc];
}

-file {
    return _file;
}

-marker {
    return _marker;
}

-(void)setFile:(NSString *)file {
   file=[file copy];
   [_file release];
   _file=file;
}

-(void)setMarker:(NSString *)marker {
   marker=[marker copy];
   [_marker release];
   _marker=marker;
}

-(void)establishConnection  {
   if ([[self file] isEqualToString:@"NSToolTipHelpKey"])
        [_destination setToolTip:[self marker]];
}

-initWithCoder:(NSCoder *)coder {
	if ((self = [super initWithCoder:coder])) {
		if ([coder allowsKeyedCoding]) {
			if ([coder containsValueForKey: @"NSFile"]) {
				_file = [[coder decodeObjectForKey: @"NSFile"] retain];
			}
			if ([coder containsValueForKey: @"NSMarker"]) {
				_marker = [[coder decodeObjectForKey: @"NSMarker"] retain];
			}
		}
	}
	return self;
}

@end
