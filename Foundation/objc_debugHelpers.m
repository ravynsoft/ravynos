/* Copyright (c) 2008 Johannes Fortmann
 
 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <signal.h>
#import <objc/objc.h>
#import <objc/runtime.h>
#import <string.h>

#import "objc_class.h"

static id objc_lookUpMetaClass(const char *name) {
   Class c=objc_lookUpClass(name);
   if(c)
      return c->isa;
   return nil;
}

BOOL _objc_checkObject(volatile id object)
{
   // assume no objects below a certain address 
   if(object<(id)0x2000)
      return NO;
   // objects begin at even addresses
   if((long)object%4!=0)
      return NO;
   volatile Class isa=object->isa;
   
   if(isa<(Class)0x2000)
      return NO;
   if((long)isa%4!=0)
      return NO;
   
   // check if name is all-ascii. We can't assume that name points to a nul-terminated string,
   // so only copy the first 256 characters.
   char *saneName=__builtin_alloca(256);
   strncpy(saneName, isa->name, 256);
   saneName[255]='\0';
   char* cur;
   for(cur=saneName; *cur!='\0'; cur++) {
      if(((uint8_t)*cur<=32 || (uint8_t)*cur>128))
      {
         return NO;
      }
   }
   // name is ok; lookup class and compare with what it should be
   Class accordingToName=Nil;
   
   if(isa->info&CLASS_INFO_META)
      accordingToName=objc_lookUpMetaClass(saneName);
   else
      accordingToName=objc_lookUpClass(saneName);
   
   if(isa==accordingToName)
      return YES;
   
   return NO;
}
