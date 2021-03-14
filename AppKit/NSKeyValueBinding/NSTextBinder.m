/* Copyright (c) 2009 Andy Balholm
 
 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import "NSTextBinder.h"

#import <Foundation/Foundation.h>
#import <AppKit/NSControl.h>
#import <AppKit/NSText.h>
#import <AppKit/NSView.h>
#import <AppKit/NSWindow.h>

@implementation _NSTextBinder

- (void)startObservingChanges
{
  [super startObservingChanges];
  
  NSNotificationCenter * nc = [NSNotificationCenter defaultCenter];
  if ([_source isKindOfClass:[NSText class]])
    {
      [nc addObserver:self 
             selector:@selector(textDidEndEditing:) 
                 name:NSTextDidEndEditingNotification 
               object:_source];
      if ([self continuouslyUpdatesValue])
        [nc addObserver:self 
               selector:@selector(textDidChange:) 
                   name:NSTextDidChangeNotification 
                 object:_source];
    }
  else if ([_source isKindOfClass:[NSControl class]])
    {
      [nc addObserver:self 
             selector:@selector(textDidEndEditing:) 
                 name:NSControlTextDidEndEditingNotification 
               object:_source];
      if ([self continuouslyUpdatesValue])
        [nc addObserver:self 
               selector:@selector(textDidChange:) 
                   name:NSControlTextDidChangeNotification 
                 object:_source];
    }
  
  if ([_source isKindOfClass:[NSView class]])
    {
      NSWindow * window = [_source window];
      if (window)
        [nc addObserver:self 
               selector:@selector(windowWillClose:)
                   name:NSWindowWillCloseNotification 
                 object:window];
    }
}

- (void)stopObservingChanges
{
  [super stopObservingChanges];
  
  [[NSNotificationCenter defaultCenter] removeObserver:self];
}

- (void)textDidChange:(NSNotification *)note
{
  [self applyDisplayedValue];
}

- (void)textDidEndEditing:(NSNotification *)note
{
  [self applyDisplayedValue];
}

- (void)windowWillClose:(NSNotification *)note
{
  [self applyDisplayedValue];
}

@end
