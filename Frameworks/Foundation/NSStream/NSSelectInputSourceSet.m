/* Copyright (c) 2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Foundation/NSSelectInputSourceSet.h>
#import <Foundation/NSSelectInputSource.h>
#import <Foundation/NSSelectSet.h>
#import <Foundation/NSMutableSet.h>
#import <Foundation/NSNotificationCenter.h>
#import <Foundation/NSArray.h>
#import <Foundation/NSEnumerator.h>
#import <Foundation/NSThread.h>
#import <Foundation/NSString.h>

@implementation NSSelectInputSourceSet

-init {
   [super init];
   _outputSet=nil;
   [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(selectSetOutputNotification:) name:NSSelectSetOutputNotification object:nil];
   return self;
}

-(void)dealloc {
   [[NSNotificationCenter defaultCenter] removeObserver:self];
   [_outputSet release];
   [super dealloc];
}

-(BOOL)recognizesInputSource:(NSInputSource *)source {
   return [source isKindOfClass:[NSSelectInputSource class]];
}

/* The old logic was to remove all output when starting a different mode to prevent
   stale triggers. However, when switching back and forth between modes the output
   from the previous run was cleared when switching modes, causing the sockets to
   never fire since they are a two times through event (one to fire the handle, one to fire the socket).
   
   At this point, sockets always check their status with a select before firing, so staleness
   shouldnt be a problem anymore.
   
   Perfect solution is to probably make sockets a one time fire event.
   
 */
-(void)startingInMode:(NSString *)mode {   
}

-(BOOL)fireSingleImmediateInputInMode:(NSString *)mode {
   NSSet   *validInputSources=[self validInputSources];
   NSArray   *sources=[validInputSources allObjects];
   NSInteger      i,count=[sources count];

   for(i=0;i<count;i++){   
    NSSelectInputSource *check=[sources objectAtIndex:i];
     NSSocket *socket=[check socket];
    NSUInteger event=0,remove;
    
     if([_outputSet containsObjectForRead:socket])
      event|=NSSelectReadEvent;
     if([_outputSet containsObjectForWrite:socket])
      event|=NSSelectWriteEvent;
     if([_outputSet containsObjectForException:socket])
      event|=NSSelectExceptEvent;
     
    if((remove=[check processImmediateEvents:event])){
     if(remove&NSSelectReadEvent)
      [_outputSet removeObjectForRead:socket];
     if(remove&NSSelectWriteEvent)
      [_outputSet removeObjectForWrite:socket];
     if(remove&NSSelectExceptEvent)
      [_outputSet removeObjectForException:socket];
      return YES;
    }
   }
   
   return NO;
}

-(NSSelectSet *)inputSelectSet {
   NSSelectSet         *result=[[[NSSelectSet alloc] init] autorelease];
   NSEnumerator        *state=[[self validInputSources] objectEnumerator];
   NSSelectInputSource *check;
   
   while((check=[state nextObject])!=nil){
    NSSocket *socket=[check socket];
    NSUInteger  mask=[check selectEventMask];
    
    if(mask&NSSelectReadEvent)
     [result addObjectForRead:socket];
    if(mask&NSSelectWriteEvent)
     [result addObjectForWrite:socket];
    if(mask&NSSelectExceptEvent)
     [result addObjectForException:socket];
   }
   
   return result;
}

-(void)selectSetOutputNotification:(NSNotification *)note {
   NSSelectSet *outputSet=[note object];
 
   [_outputSet autorelease];
   _outputSet=[outputSet copy];
}

-(void)waitInBackgroundInMode:(NSString *)mode {
   NSSelectSet *selectSet=[self inputSelectSet];
   
   [_outputSet autorelease];
   _outputSet=nil;
   
   [selectSet waitInBackgroundInMode:mode];
}

-(BOOL)waitForInputInMode:(NSString *)mode beforeDate:(NSDate *)date {
   NSSelectSet *selectSet=[self inputSelectSet];
   NSError     *error;
      
   [_outputSet autorelease];
   _outputSet=nil;
   
   if([selectSet isEmpty]){
    [NSThread sleepUntilDate:date];
    return NO;
   }
   else if((error=[selectSet waitForSelectWithOutputSet:&_outputSet beforeDate:date])!=nil)
    return NO;
   else {
    [_outputSet retain];
   
    return [self fireSingleImmediateInputInMode:mode];
   }
}

@end
