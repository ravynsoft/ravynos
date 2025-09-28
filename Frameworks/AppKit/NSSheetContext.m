/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <AppKit/NSSheetContext.h>
#import <AppKit/NSWindow.h>

@implementation NSSheetContext

-initWithSheet:(NSWindow *)sheet modalDelegate:modalDelegate didEndSelector:(SEL)didEndSelector contextInfo:(void *)contextInfo frame:(NSRect)frame {
   if(sheet==nil)
    [NSException raise:NSInvalidArgumentException format:@"sheet==nil"];
    
   _sheet=[sheet retain];
   _modalDelegate=modalDelegate;
   _didEndSelector=didEndSelector;
   _contextInfo=contextInfo;
   _frame=frame;
   return self;
}

-(void)dealloc {
	[_session release];
   [_sheet release];
   [super dealloc];
}

+(NSSheetContext *)sheetContextWithSheet:(NSWindow *)sheet modalDelegate:modalDelegate didEndSelector:(SEL)didEndSelector contextInfo:(void *)contextInfo frame:(NSRect)frame {
    return [[[self alloc] initWithSheet:sheet modalDelegate:modalDelegate didEndSelector:didEndSelector contextInfo:contextInfo frame:frame] autorelease];
}

-(NSWindow *)sheet {
   return _sheet;
}

-modalDelegate {
   return _modalDelegate;
}

-(SEL)didEndSelector {
   return _didEndSelector;
}

-(void *)contextInfo {
   return _contextInfo;
}

-(NSRect)frame {
   return _frame;
}

- (void)setModalSession:(NSModalSession)session
{
	[session retain];
	[_session release];
	_session = session;
}

- (NSModalSession)modalSession
{
	return _session;
}

@end
