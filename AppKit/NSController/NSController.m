/* Copyright (c) 2007 Johannes Fortmann

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <AppKit/NSController.h>
#import <AppKit/NSRaise.h>
#import <AppKit/NSControllerMarker.h>

id NSNoSelectionMarker=@"NSNoSelectionMarker";
id NSMultipleValuesMarker=@"NSMultipleValuesMarker";
id NSNotApplicableMarker=@"NSNotApplicableMarker";

BOOL NSIsControllerMarker(id object)
{
	if(object==NSNoSelectionMarker ||
	   object==NSMultipleValuesMarker ||
	   object==NSNotApplicableMarker)
		return YES;
	return NO;
}

@implementation NSController

+(void)initialize {
   if(self == [NSController class]) {
      NSNoSelectionMarker = [[_NSControllerMarker alloc] initWithString:@"NSNoSelectionMarker"];
      NSMultipleValuesMarker = [[_NSControllerMarker alloc] initWithString:@"NSMultipleValuesMarker"];
      NSNotApplicableMarker = [[_NSControllerMarker alloc] initWithString:@"NSNotApplicableMarker"];
   }
}

-init {
  self = [super init];
  if (self)
    {
      _editors = [[NSMutableArray alloc] init];
    }
  return self;
}

-(void)dealloc {
  [_editors release];
  [super dealloc];
}

-initWithCoder:(NSCoder *)coder {
   NSUnimplementedMethod();
   return self;
}

-(void)encodeWithCoder:(NSCoder *)coder {
   NSUnimplementedMethod();
}

-(BOOL)commitEditing {
  if ([_editors count] == 0)
    return YES;
  
  NSEnumerator * e = [_editors objectEnumerator];
  id editor;
  while ((editor = [e nextObject]))
    if ([editor commitEditing] == NO)
      return NO;
  
  return YES;
}

-(void)discardEditing {
  NSEnumerator * e = [_editors objectEnumerator];
  id editor;
  while ((editor = [e nextObject]))
    [editor discardEditing];
}

-(BOOL)isEditing {
  return [_editors count] > 0;
}

-(void)objectDidBeginEditing:editor {
  [_editors addObject:editor];
}

-(void)objectDidEndEditing:editor {
  [_editors removeObject:editor];
}

@end
