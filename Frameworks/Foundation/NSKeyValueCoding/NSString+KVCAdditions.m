/* Copyright (c) 2007 Johannes Fortmann

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import "NSString+KVCAdditions.h"
#include <ctype.h>

@implementation NSString (KVCPrivateAdditions)

-(BOOL)_KVC_isSetterForSelectorNameStartingWith:(NSString *)start endingWith:(NSString *)end;
{
	if([self hasPrefix:start] && [self hasSuffix:end])
	{
		NSString* keyName=[self substringWithRange:NSMakeRange([start length], [self length]-[end length]-[start length])];
      if(![keyName length])
         return NO;

		return YES;
	}
	return NO;
}

// KVO and KVC can not share the path splitting code:
// Strangely, KVO will observe a key when specified capitalized
// Even stranger, KVC will treat such a key or path as undefined 
NSString *_NSKVOSplitKeyPath(NSString *path,NSString **restOfPath){
   NSInteger dot,length=[path length];
   unichar   buffer[length];
   
   [path getCharacters:buffer];
   
   for(dot=0;dot<length;dot++)
    if(buffer[dot]=='.')
     break;
   
   if(dot<length)
    *restOfPath=[NSString stringWithCharacters:buffer+dot+1 length:length-(dot+1)];
   else
    *restOfPath=nil;   

// Disabling this code - see the function comment for why it could be needed
// It doesn't seem to be the case anymore in recent OS X versions and it's killing
// the observing of properties that really have an uppercase char (which is quite
// common if you try to observe NSDefaults properties)
#if 0
// we must always lowercase
   buffer[0]=tolower(buffer[0]);
#endif
    
   return [NSString stringWithCharacters:buffer length:dot];
}

-(void)_KVC_partBeforeDot:(NSString**)before afterDot:(NSString**)after;
{
	NSRange range=[self rangeOfString:@"."];
	if(range.location!=NSNotFound)
	{
		*before=[self substringToIndex:range.location];
		*after=[self substringFromIndex:range.location+1];
	}
	else
	{
		*before=self;
		*after=nil;
	}
}

@end
