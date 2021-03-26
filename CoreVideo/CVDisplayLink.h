#import <Foundation/Foundation.h>
#import <CoreVideo/CVBase.h>
#import <CoreVideo/CVReturn.h>
#import <OpenGL/OpenGL.h>

@class CVDisplayLink;

typedef CVDisplayLink *CVDisplayLinkRef;

typedef CVReturn (*CVDisplayLinkOutputCallback)(CVDisplayLinkRef, const CVTimeStamp *, const CVTimeStamp *, CVOptionFlags, CVOptionFlags *, void *);

COREVIDEO_EXPORT CVReturn CVDisplayLinkCreateWithActiveCGDisplays(CVDisplayLinkRef *result);
COREVIDEO_EXPORT CVReturn CVDisplayLinkSetOutputCallback(CVDisplayLinkRef self, CVDisplayLinkOutputCallback callback, void *userInfo);
COREVIDEO_EXPORT CVReturn CVDisplayLinkSetCurrentCGDisplayFromOpenGLContext(CVDisplayLinkRef self, CGLContextObj cglContext, CGLPixelFormatObj cglPixelFormat);

COREVIDEO_EXPORT CVReturn CVDisplayLinkStart(CVDisplayLinkRef self);
COREVIDEO_EXPORT CVReturn CVDisplayLinkStop(CVDisplayLinkRef self);
COREVIDEO_EXPORT Boolean CVDisplayLinkIsRunning(CVDisplayLinkRef self);

COREVIDEO_EXPORT CVDisplayLinkRef CVDisplayLinkRetain(CVDisplayLinkRef self);
COREVIDEO_EXPORT void CVDisplayLinkRelease(CVDisplayLinkRef self);
