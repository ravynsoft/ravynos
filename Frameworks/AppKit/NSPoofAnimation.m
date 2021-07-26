//
//  NSPoofAnimation.m
//
//  Created by Airy ANDRE on 24/07/13.
//  Copyright (c) 2013 plasq. All rights reserved.
//

#import "NSPoofAnimation.h"

static const float kAnimationDuration = .3;

// The animation is defined by the NSPoof.png image, which is made from individual square frame, stacked either
// horizontally or vertically
@interface NSPoofView : NSView {
    NSInteger _poofIndex;
    id _animationDelegate;
    SEL _didEndSelector;
    void *_contextInfo;
}

- (void)setAnimationDelegate:(id)animationDelegate;
- (void)setDidEndSelector:(SEL)didEndSelector;
- (void)setContextInfo:(void *)contextInfo;
@end

@implementation NSPoofView
+ (NSImage *)poofImages
{
    NSImage *image = [NSImage imageNamed:@"NSPoof"];
    return image;
}

+ (NSSize)poofImageSize
{
    NSSize poofImageSize = [[self poofImages] size];
    poofImageSize.width = poofImageSize.height = MIN(poofImageSize.width, poofImageSize.height);
    return poofImageSize;
}

+ (NSInteger)numberOfPoofImages
{
    NSSize poofImageSize = [[self poofImages] size];
    return MAX(poofImageSize.width, poofImageSize.height) / MIN(poofImageSize.width, poofImageSize.height);
}

- (BOOL)isOpaque
{
    return NO;
}

- (void)setAnimationDelegate:(id)animationDelegate
{
    _animationDelegate = animationDelegate;
}

- (void)setDidEndSelector:(SEL)didEndSelector
{
    _didEndSelector = didEndSelector;
}

- (void)setContextInfo:(void *)contextInfo
{
    _contextInfo = contextInfo;
}

- (void)displayPoofImageAtIndex:(NSInteger)index
{
    NSImage *poofImages = [[self class] poofImages];
    NSSize poofImageSize = [[self class] poofImageSize];

    NSRect r = NSMakeRect(0, 0, poofImageSize.width, poofImageSize.height);
    if (poofImages.size.width > poofImages.size.height) {
        r.origin.x += poofImageSize.width * index;
    } else {
        r.origin.y += poofImageSize.height * index;
    }
    [poofImages drawInRect:self.bounds fromRect:r operation:NSCompositeCopy fraction:1];
}

- (void)drawRect:(NSRect)rect
{
    [self displayPoofImageAtIndex: _poofIndex - 1];
}

- (void)poof:(NSTimer *)timer
{
    _poofIndex++;
    if (_poofIndex > [[self class] numberOfPoofImages]) {
        [timer invalidate];
        [self.window close];
        [_animationDelegate performSelector:_didEndSelector withObject:_contextInfo afterDelay:0.];
    }
    [self setNeedsDisplay:YES];
}


@end

@implementation NSPoofAnimation
+ (void)poofAtLocation:(NSPoint)location size:(NSSize)size animationDelegate:(id)animationDelegate didEndSelector:(SEL)didEndSelector contextInfo:(void *)contextInfo
{
    if (size.width == 0 || size.height == 0) {
        size.width = size.height = 48;
    }
    location.x -= size.width*.5;
    location.y -= size.height*.5;
    NSRect r = {
        .origin = location,
        .size = size
    };
    
    // Create a transparent window with a poof view - it will be closed when the animation is done
    NSWindow *window = [[NSWindow alloc] initWithContentRect:r styleMask:NSBorderlessWindowMask backing:NSBackingStoreBuffered defer:NO];
    [window setHasShadow:NO];
    
    NSPoofView *poofView = [[[NSPoofView alloc] initWithFrame:window.frame] autorelease];
    [poofView setAnimationDelegate:animationDelegate];
    [poofView setDidEndSelector:didEndSelector];
    [poofView setContextInfo:contextInfo];
    
    [window setContentView: poofView];
    [window setLevel: kCGScreenSaverWindowLevel];
    
    [window setReleasedWhenClosed:YES];
    
    [window setOpaque: NO];
    [window setBackgroundColor:[NSColor clearColor]];
    [window orderFront:nil];
    
    NSTimer *timer = [NSTimer timerWithTimeInterval:kAnimationDuration/([NSPoofView numberOfPoofImages] - 1) target:poofView selector:@selector(poof:) userInfo:nil repeats:YES] ;
    [[NSRunLoop currentRunLoop] addTimer:timer forMode:NSRunLoopCommonModes];
    [[NSRunLoop currentRunLoop] addTimer:timer forMode:NSDefaultRunLoopMode];
}
@end
