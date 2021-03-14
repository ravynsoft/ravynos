/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

// Original - Christopher Lloyd <cjwl@objc.net>
#import <AppKit/NSTextAttachment.h>
#import <AppKit/NSImage.h>
#import <AppKit/NSFileWrapper.h>

@implementation NSTextAttachment

-initWithFileWrapper:(NSFileWrapper *)fileWrapper {
   _fileWrapper=[fileWrapper retain];
   _cell=[[NSTextAttachmentCell alloc] init];
   return self;
}

-(void)dealloc {
   [_fileWrapper release];
   [_cell release];
   [super dealloc];
}

-(NSFileWrapper *)fileWrapper {
   return _fileWrapper;
}

-(id <NSTextAttachmentCell>)attachmentCell {
   return _cell;
}

-(void)setFileWrapper:(NSFileWrapper *)fileWrapper {
   fileWrapper=[fileWrapper retain];
   [_fileWrapper release];
   _fileWrapper=fileWrapper;
}

-(void)setAttachmentCell:(id <NSTextAttachmentCell>)cell {
   cell=[cell retain];
   [_cell release];
   _cell=cell;
}

@end
