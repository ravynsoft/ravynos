#import <Foundation/NSObject.h>
#import <Onyx2D/O2Geometry.h>
#import <OpenGL/OpenGL.h>

@class O2Surface, CGWindow;

@interface CGLPixelSurface : NSObject {
    int _width, _height;
    BOOL _isOpaque, _validBuffers;
    int _numberOfBuffers;
    int _rowsPerBuffer;
    GLuint *_bufferObjects;
    void **_readPixels;
    void **_staticPixels;

    O2Surface *_surface;
}

- initWithSize:(O2Size)size;

- (void)setFrameSize:(O2Size)value;
- (void)setOpaque:(BOOL)value;

- (void)readBuffer;

- (O2Surface *)validSurface;

@end
