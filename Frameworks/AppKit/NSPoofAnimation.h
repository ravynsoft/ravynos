//
//  NSPoofAnimation.h
//  Poof
//
//  Created by Airy ANDRE on 24/07/13.
//  Copyright (c) 2013 plasq. All rights reserved.
//

#import <AppKit/AppKit.h>

@interface NSPoofAnimation : NSObject

+ (void)poofAtLocation:(NSPoint)location size:(NSSize)size animationDelegate:(id)animationDelegate didEndSelector:(SEL)didEndSelector contextInfo:(void *)contextInfo;

@end
