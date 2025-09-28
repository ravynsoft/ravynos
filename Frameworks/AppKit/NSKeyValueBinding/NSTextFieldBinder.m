/* Copyright (c) 2009 Andy Balholm
 
 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import "NSTextFieldBinder.h"

#import <Foundation/Foundation.h>
#import <AppKit/NSControl.h>
#import <AppKit/NSText.h>
#import "NSObject+BindingSupport.h"
#import <AppKit/NSWindow.h>
#import <AppKit/NSDocument.h>

@implementation _NSTextFieldBinder

- (void)dealloc
{
  [_originalValue release];
  [super dealloc];
}

- (void)startObservingChanges
{
   if(!_isObserving){
  [super startObservingChanges];
  
  NSNotificationCenter * nc = [NSNotificationCenter defaultCenter];
  if ([_source isKindOfClass:[NSControl class]])
    {
      [nc addObserver:self 
             selector:@selector(textDidEndEditing:) 
                 name:NSControlTextDidEndEditingNotification 
               object:_source];
      [nc addObserver:self 
             selector:@selector(textDidBeginEditing:) 
                 name:NSControlTextDidBeginEditingNotification 
               object:_source];
      if ([self continuouslyUpdatesValue])
        [nc addObserver:self 
               selector:@selector(textDidChange:) 
                   name:NSControlTextDidChangeNotification 
                 object:_source];
    }
}
}

- (void)stopObservingChanges
{
   if(_isObserving){
  [super stopObservingChanges];
  
   [[NSNotificationCenter defaultCenter] removeObserver:self name:nil object:_source];
}
}

- (void)textDidBeginEditing:(NSNotification *)note
{
  [self setOriginalValue:[_source valueForKeyPath:_bindingPath]];
  if ([_destination respondsToSelector:@selector(objectDidBeginEditing:)])
    [_destination objectDidBeginEditing:self];
  if ([_source respondsToSelector:@selector(window)])
    {
      NSDocument * document = [[[_source window] windowController] document];
      if (document)
        [document objectDidBeginEditing:self];
    }
}

- (void)textDidChange:(NSNotification *)note
{
  [self applyDisplayedValue];
}

- (void)textDidEndEditing:(NSNotification *)note
{
  [self applyDisplayedValue];
  if (_originalValue)
    {
      [self setOriginalValue:nil];
      if ([_destination respondsToSelector:@selector(objectDidEndEditing:)])
        [_destination objectDidEndEditing:self];
      if ([_source respondsToSelector:@selector(window)])
        {
          NSDocument * document = [[[_source window] windowController] document];
          if (document)
            [document objectDidEndEditing:self];
        }
    }
}

- (void)setOriginalValue:(id)value
{
  [value retain];
  [_originalValue release];
  _originalValue = value;
}

- (BOOL)commitEditing
{
  BOOL returnValue = [[_source window] makeFirstResponder:nil];
  NSUndoManager * undoManager = [[_source window] undoManager];
  [undoManager endUndoGrouping];
  [undoManager beginUndoGrouping];
  return returnValue;
}

- (void)discardEditing
{
  if (_originalValue)
    [_source setValue:_originalValue forKeyPath:_bindingPath];
  NSUndoManager * undoManager = [[_source window] undoManager];
  [undoManager endUndoGrouping];
  [undoManager beginUndoGrouping];
}

@end
