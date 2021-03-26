/* Copyright (c) 2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Onyx2D/O2Layer.h>
#import <Onyx2D/O2Exceptions.h>
#import <Onyx2D/O2BitmapContext.h>
#import <Foundation/NSDictionary.h>

@implementation O2Layer

O2Surface *O2LayerGetSurface(O2LayerRef self) {
   return [self->_context surface];
}

O2LayerRef O2LayerCreateWithContext(O2ContextRef context,O2Size size,NSDictionary *unused) {
   O2LayerRef self=NSAllocateObject([O2Layer class],0,NULL);

    if (self) {
       self->_context=[context createCompatibleContextWithSize:size unused:unused];
       self->_size=size;
       self->_unused=[unused copy];
       self->_surface=[context surface];
    }
   return self;
}

O2LayerRef O2LayerRetain(O2LayerRef self) {
   return (self!=NULL)?(O2LayerRef)CFRetain(self):NULL;
}

void O2LayerRelease(O2LayerRef self) {
   if(self!=NULL)
    CFRelease(self);
}

O2Size O2LayerGetSize(O2LayerRef self) {
   return self->_size;
}

O2ContextRef O2LayerGetContext(O2LayerRef self) {
   return self->_context;
}

- (void)dealloc
{
	[_context release];
	[_unused release];
	[super dealloc];
}
@end
