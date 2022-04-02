#import <QuartzCore/CAWindowOpenGLContext.h>
#import <OpenGL/OpenGL.h>
#import <Onyx2D/O2Surface.h>

@implementation CAWindowOpenGLContext

-initWithCGLContext:(CGLContextObj)cglContext {
    _cglContext=CGLRetainContext(cglContext);
    glGenBuffers(1, &vbo);
    glGenVertexArrays(1, &vao);

    GLfloat vertices[] = {
    // positions     colors     texture
     -1, -1, 0,    1, 1, 1, 0,   0, 1,
      1, -1, 0,    1, 1, 1, 0,   1, 1,
     -1,  1, 0,    1, 1, 1, 0,   0, 0,
      1,  1, 0,    1, 1, 1, 0,   1, 0,
    };
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    return self;
}

-(void)dealloc {
   CGLReleaseContext(_cglContext);
   glDeleteVertexArrays(1, &vao);
   glDeleteBuffers(1, &vbo);
   [super dealloc];
}

-(void)prepareViewportWidth:(int)width height:(int)height {
// prepare
   CGLError error;

   if((error=CGLSetCurrentContext(_cglContext))!=kCGLNoError)
    NSLog(@"CGLSetCurrentContext failed with %d in %s %d",error,__FILE__,__LINE__);

   glEnable(GL_DEPTH_TEST);
   //glShadeModel(GL_SMOOTH);

// reshape
   glViewport(0,0,width,height);
   //glMatrixMode(GL_PATH_PROJECTION_NV);                      
   glLoadIdentity();
   glOrtho (0, width, 0, height, -1, 1);
}

-(void)renderSurface:(O2Surface *)surface {
    size_t width=O2ImageGetWidth(surface);
    size_t height=O2ImageGetHeight(surface);

// prepare
    glEnable(GL_DEPTH_TEST);
    CGLUseShaders(_cglContext);

// reshape
    glViewport(0, 0, width, height);
    //glMatrixMode(GL_PATH_PROJECTION_NV);                      
    glLoadIdentity();
    glOrtho(0, width, 0, height, -1, 1);

// render
    glMatrixMode(GL_PATH_MODELVIEW_NV);                                     
    glLoadIdentity();

    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
   
    //glEnable(GL_TEXTURE_2D);

    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, [surface pixelBytes]);

    //glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glPushMatrix();
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 9*sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 9*sizeof(float), (void*)(7*sizeof(float)));
    glEnableVertexAttribArray(2);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glPopMatrix();

    glFlush();
}

@end
