/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <Foundation/NSObject.h>
#import <objc/message.h>

// how lame but this is all I need, sparc passes arguments in registers
id objc_msgSendv(id self, SEL selector, uint32_t arg_size, void *arg_frame) {
  uint32_t *argWords=arg_frame;
  IMP       method=objc_msg_lookup(self,selector);

  arg_size/=sizeof(int);
  
  switch(arg_size){
  case 0:
  case 1:
  case 2:
    return method(self,selector);
  case 3:
    return method(self,selector,argWords[2]);
  case 4:
    return method(self,selector,argWords[2],argWords[3]);
  case 5:
    return method(self,selector,argWords[2],argWords[3],argWords[4]);
  case 6:
    return method(self,selector,argWords[2],argWords[3],argWords[4],argWords[5]);
  case 7:
    return method(self,selector,argWords[2],argWords[3],argWords[4],argWords[5],argWords[6]);
  case 8:
    return method(self,selector,argWords[2],argWords[3],argWords[4],argWords[5],argWords[6],argWords[7]);
  case 9:
    return method(self,selector,argWords[2],argWords[3],argWords[4],argWords[5],argWords[6],argWords[7],argWords[8]);
  case 10:
    return method(self,selector,argWords[2],argWords[3],argWords[4],argWords[5],argWords[6],argWords[7],argWords[8],argWords[9]);
  }

  return nil;
}
