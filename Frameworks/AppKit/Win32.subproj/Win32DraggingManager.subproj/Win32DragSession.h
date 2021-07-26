/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <AppKit/NSDragging.h>

@class Win32Window;

@interface Win32DragSession : NSObject <NSDraggingInfo> {
    Win32Window *_window;
    id _receiver;
    NSDragOperation _sourceOperationMask;
    NSPoint _screenLocation;
    NSPoint _imageLocation;
    NSImage *_image;
    NSPasteboard *_pasteboard;
    id _source;
    int _sequenceNumber;
}

- initWithWindow:(Win32Window *)window pasteboard:(NSPasteboard *)pasteboard;

- (void)setSourceOperationMask:(unsigned)mask;
- (void)setScreenLocation:(NSPoint)point;

- (id)receiver;
- (void)setReceiver:(id)object;

- (BOOL)pasteboardTypesIntersectTypes:(NSArray *)types;

@end
