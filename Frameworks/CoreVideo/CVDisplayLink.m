#import <CoreVideo/CVDisplayLink.h>
#import <Foundation/NSRaise.h>

// FIXME: use only one timer for all the display links, this will reduce run loop overhead

@interface CVDisplayLink : NSObject {
    NSTimer *_timer;
    CVDisplayLinkOutputCallback _callback;
    void  *_userInfo;
}

@end

@implementation CVDisplayLink

-init {
    return self;
}

-(void)dealloc {
    [_timer invalidate];
    [_timer release];
    [super dealloc];
}

-(void)displayLinkTimer:(NSTimer *)timer {
    if(_callback!=NULL)
        _callback(self,NULL,NULL,0,NULL,_userInfo);
}

-(void)start {
    _timer=[[NSTimer scheduledTimerWithTimeInterval:1.0/60.0 target:self selector:@selector(displayLinkTimer:) userInfo:nil repeats:YES] retain];
}

-(void)stop {
    [_timer invalidate];
    [_timer release];
    _timer = nil;
}

CVReturn CVDisplayLinkCreateWithActiveCGDisplays(CVDisplayLinkRef *result) {
    *result=[[CVDisplayLink alloc] init];
    return kCVReturnSuccess;
}

CVReturn CVDisplayLinkSetOutputCallback(CVDisplayLinkRef self,CVDisplayLinkOutputCallback callback,void *userInfo) {
    self->_callback=callback;
    self->_userInfo=userInfo;
    return kCVReturnSuccess;
}

CVReturn CVDisplayLinkSetCurrentCGDisplayFromOpenGLContext(CVDisplayLinkRef self,CGLContextObj cglContext,CGLPixelFormatObj cglPixelFormat) {
    return kCVReturnSuccess;
}

CVReturn CVDisplayLinkStart(CVDisplayLinkRef self) {
    if (CVDisplayLinkIsRunning(self))
        return kCVReturnDisplayLinkAlreadyRunning;
    [self start];
    return kCVReturnSuccess;
}

CVReturn CVDisplayLinkStop(CVDisplayLinkRef self) {
    if (!CVDisplayLinkIsRunning(self))
        return kCVReturnDisplayLinkNotRunning;
    [self stop];
    return kCVReturnSuccess;
}

Boolean CVDisplayLinkIsRunning (CVDisplayLinkRef self) {
    return (self->_timer != nil);
}

CVDisplayLinkRef CVDisplayLinkRetain(CVDisplayLinkRef self) {
    return [self retain];
}

void CVDisplayLinkRelease(CVDisplayLinkRef self) {
    [self release];
}

@end
