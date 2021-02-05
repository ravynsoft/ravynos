/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <Foundation/NSInputSourceSet.h>
#import <Foundation/NSInputSource.h>
#import <Foundation/NSRaise.h>
#import <Foundation/NSMutableSet.h>
#import <Foundation/NSEnumerator.h>
#import <Foundation/NSDate.h>
#import <Foundation/NSArray.h>

@implementation NSInputSourceSet

-init {
   _inputSources=[NSMutableSet new];
   return self;
}

-(void)dealloc {
   [_inputSources release];
   [super dealloc];
}

-(NSUInteger)count {
   return [_inputSources count];
}

-(BOOL)recognizesInputSource:(NSInputSource *)source {
   return NO;
}

-(void)addInputSource:(NSInputSource *)source {
   [_inputSources addObject:source];
}

-(void)removeInputSource:(NSInputSource *)source {
   [_inputSources removeObject:source];
}

-(void)startingInMode:(NSString *)mode {
  // do nothing
}

-(NSSet *)validInputSources {
   NSEnumerator   *state=[_inputSources objectEnumerator];
   NSMutableArray *invalid=nil;
   NSInputSource  *check;

   while((check=[state nextObject])!=nil){
    if(![check isValid]){
     if(invalid==nil)
      invalid=[NSMutableArray array];
      
     [invalid addObject:check];
    }
   }

   while((check=[invalid lastObject])!=nil){
    [_inputSources removeObject:check];
    [invalid removeLastObject];
   }
   
   return _inputSources;
}

-(BOOL)fireSingleImmediateInputInMode:(NSString *)mode {
   NSEnumerator   *state=[[self validInputSources] objectEnumerator];
   NSInputSource  *check;

   while((check=[state nextObject])!=nil){
    if([check processInputImmediately])
     return YES;
   }

   return NO;
}

-(void)waitInBackgroundInMode:(NSString *)mode {
   NSInvalidAbstractInvocation();
}

-(BOOL)waitForInputInMode:(NSString *)mode beforeDate:(NSDate *)date {
   NSInvalidAbstractInvocation();
   return NO;
}

@end
