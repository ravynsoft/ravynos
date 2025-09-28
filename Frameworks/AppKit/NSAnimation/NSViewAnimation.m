/* Copyright (c) 2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <AppKit/NSViewAnimation.h>
#import <AppKit/NSView.h>
#import <AppKit/NSWindow.h>
#import <Foundation/NSString.h>
#import <Foundation/NSArray.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSValue.h>

NSString * const NSViewAnimationStartFrameKey=@"NSViewAnimationStartFrameKey";
NSString * const NSViewAnimationEndFrameKey=@"NSViewAnimationEndFrameKey";
NSString * const NSViewAnimationTargetKey=@"NSViewAnimationTargetKey";
NSString * const NSViewAnimationEffectKey=@"NSViewAnimationEffectKey";

NSString * const NSViewAnimationFadeInEffect=@"NSViewAnimationFadeInEffect";
NSString * const NSViewAnimationFadeOutEffect=@"NSViewAnimationFadeOutEffect";

@implementation NSViewAnimation

-initWithViewAnimations:(NSArray *)animations {
// default is 0.5
   [super initWithDuration:0.5 animationCurve:NSAnimationEaseInOut];
   _animations=[animations retain];
   return self;
}

-(void)dealloc {
   [_animations release];
   [super dealloc];
}

-copyWithZone:(NSZone *)zone {
   NSViewAnimation *result=[super copyWithZone:zone];
   
   result->_animations=[_animations copy];

   return result;
}

-(NSArray *)viewAnimations {
   return _animations;
}

-(void)setViewAnimations:(NSArray *)animations {
   animations=[animations retain];
   [_animations release];
   _animations=animations;
}

-(void)setCurrentProgress:(NSAnimationProgress)progress {
   [super setCurrentProgress:progress];
// For now some of the animations dont actually animate, they just perform the last frame
   BOOL isFinalFrame=(progress>=1.0)?YES:NO;
   
   NSInteger i,count=[_animations count];
   for(i=0;i<count;i++){
    NSDictionary *info=[_animations objectAtIndex:i];
    id            target=[info objectForKey:NSViewAnimationTargetKey];
    NSValue      *startFrame=[info objectForKey:NSViewAnimationStartFrameKey];
    NSValue      *endFrame=[info objectForKey:NSViewAnimationEndFrameKey];
    NSString     *effect=[info objectForKey:NSViewAnimationEffectKey];
    
    if(isFinalFrame && startFrame!=nil && endFrame!=nil){
     NSRect startRect=[startFrame rectValue];
     NSRect endRect=[endFrame rectValue];
     NSRect currentRect;
     
     currentRect.origin.x=startRect.origin.x+(endRect.origin.x-startRect.origin.x)*progress;
     currentRect.origin.y=startRect.origin.y+(endRect.origin.y-startRect.origin.y)*progress;
     currentRect.size.width=startRect.size.width+(endRect.size.width-startRect.size.width)*progress;
     currentRect.size.height=startRect.size.height+(endRect.size.height-startRect.size.height)*progress;
    
     if([target isKindOfClass:[NSView class]])
      [target setFrame:currentRect];
     else if([target isKindOfClass:[NSWindow class]]){
      [target setFrame:currentRect display:YES];
     }
    }
    
    if(effect!=nil){
     if([effect isEqualToString:NSViewAnimationFadeInEffect]){
     
      if([target isKindOfClass:[NSView class]]){
       if(progress>0)
        [target setHidden:NO];
      }
      else if([target isKindOfClass:[NSWindow class]]){
       [target setAlphaValue:progress];
       if(progress>0)
        [target orderFront:nil];
      }
     }
     if([effect isEqualToString:NSViewAnimationFadeOutEffect]){
     
      if([target isKindOfClass:[NSView class]]){
       if(progress>0)
        [target setHidden:YES];
      }
      else if([target isKindOfClass:[NSWindow class]]){
       [target setAlphaValue:1.0-progress];
       
       if(isFinalFrame)
        [target orderOut:nil];
      }
     }
    }
    if([target isKindOfClass:[NSView class]])
     [[target superview] setNeedsDisplay:YES];
   }
}


@end
