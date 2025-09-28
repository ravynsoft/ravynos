/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

// Original - Christopher Lloyd <cjwl@objc.net>
#import <Foundation/NSNotificationQueue.h>
#import <Foundation/NSNotification.h>
#import <Foundation/NSArray.h>
#import <Foundation/NSThread.h>
#import <Foundation/NSRaise.h>
#import <Foundation/NSNotificationAndModes.h>
#import <Foundation/NSThread-Private.h>

@implementation NSNotificationQueue

-initWithNotificationCenter:(NSNotificationCenter *)center {
   _center=[center retain];
   _asapQueue=[[NSMutableArray allocWithZone:[self zone]] init];
   _idleQueue=[[NSMutableArray allocWithZone:[self zone]] init];
   return self;
}

-init {
   return [self initWithNotificationCenter:[NSNotificationCenter defaultCenter]];
}

+(NSNotificationQueue *)defaultQueue {
   return NSThreadSharedInstance(@"NSNotificationQueue");
}

-(void)asapProcessMode:(NSString *)mode {
   BOOL hasNotes=NO;

   do{
    NSInteger i,count=[_asapQueue count];

    hasNotes=NO;

    for(i=0;i<count && !hasNotes;i++){
     NSNotificationAndModes *qd=[_asapQueue objectAtIndex:i];
     NSArray               *modes=[qd modes];

     if(modes==nil || [modes containsObject:mode]){
      [qd retain];
      [_asapQueue removeObjectAtIndex:i];

      hasNotes=YES;
      [_center postNotification:[qd notification]];

      [qd release];
     }
    }

   }while(hasNotes);
}

-(BOOL)hasIdleNotificationsInMode:(NSString *)mode {
   NSInteger count=[_idleQueue count];

   while(--count>=0){
    NSNotificationAndModes *check=[_idleQueue objectAtIndex:count];
    NSArray                *modes=[check modes];

    if(modes==nil || [modes containsObject:mode])
     return YES;
   }

   return NO;
}

-(void)idleProcessMode:(NSString *)mode {
   NSMutableArray *idle=[NSMutableArray array];
   NSInteger             count=[_idleQueue count];

   while(--count>=0){
    NSNotificationAndModes *check=[_idleQueue objectAtIndex:count];
    NSArray                *modes=[check modes];

    if(modes==nil || [modes containsObject:mode]){
     [idle addObject:check];
     [_idleQueue removeObjectAtIndex:count];
    }
   }

   count=[idle count];
   while(--count>=0){
    NSNotificationAndModes *check=[idle objectAtIndex:count];

    [_center postNotification:[check notification]];
   }
}

-(void)coalesceNotification:(NSNotification *)note inQueue:(NSMutableArray *)queue coalesceMask:(NSUInteger)mask {
   if(mask!=NSNotificationNoCoalescing){
    NSInteger count=[queue count];

    while(--count>=0){
     NSNotification *check=[[queue objectAtIndex:count] notification];
     BOOL            matches=NO;

     if(mask&NSNotificationCoalescingOnName)
      if([[check name] isEqualToString:[note name]])
       matches=YES;

     if(mask&NSNotificationCoalescingOnSender)
      if([check object]==[note object])
       matches=YES;

     if(matches)
      [queue removeObjectAtIndex:count];
    }
   }
}

-(BOOL)canPlaceNotification:(NSNotification *)note
  inQueue:(NSArray *)queue coalesceMask:(NSUInteger)mask {
   if(mask!=NSNotificationNoCoalescing) {
    NSInteger i,count=[queue count];

    for(i=0;i<count;i++){
     NSNotification *check=[[queue objectAtIndex:i] notification];

     if(mask&NSNotificationCoalescingOnName)
      if([[check name] isEqualToString:[note name]])
       return NO;
     if(mask&NSNotificationCoalescingOnSender)
      if([check object]==[note object])
       return NO;
    }
   }

   return YES;
}

-(void)enqueueNotification:(NSNotification *)note
              postingStyle:(NSPostingStyle)style
              coalesceMask:(NSUInteger)mask
                  forModes:(NSArray *)modes {
/*
  Figure out what modes==nil means, does it mean the current mode?
    .. run in mode X ...
     post in nil modes
      .. run in mode Y ...

  Does it notify in Y or X? Current in Y, experience suggests it should be X.
 */
   if(style==NSPostNow)
    [_center postNotification:note];
   else {
    NSMutableArray *queue=nil;

    if(style==NSPostWhenIdle)
     queue=_idleQueue;
    else if(style==NSPostASAP)
     queue=_asapQueue;

    [self coalesceNotification:note inQueue:queue coalesceMask:mask];
    [queue addObject:[NSNotificationAndModes queuedWithNotification:note modes:modes]];
   }
}

-(void)enqueueNotification:(NSNotification *)note
              postingStyle:(NSPostingStyle)style {
   [self enqueueNotification:note postingStyle:style
     coalesceMask:NSNotificationCoalescingOnName|
                  NSNotificationCoalescingOnSender forModes:nil];
}

-(void)dequeueNotificationsMatching:(NSNotification *)note
                       coalesceMask:(NSUInteger)mask {
   NSUnimplementedMethod();

   if(mask==(NSNotificationCoalescingOnName|NSNotificationCoalescingOnSender)){

   }
   else if(mask==NSNotificationCoalescingOnName){

   }
   else if(mask==NSNotificationCoalescingOnSender){

   }
}

@end
