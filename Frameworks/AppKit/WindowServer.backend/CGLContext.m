/*
 * Copyright (C) 2022 Zoe Knox <zoe@pixin.net>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#import <Foundation/NSString.h>
#import <Foundation/NSRaise.h>

#import <OpenGL/OpenGL.h>
#import <wayland-egl.h>
#import <pthread.h>
#import <sys/stat.h>
#import <sys/mman.h>
#import <fcntl.h>

#import "WLWindow.h"

PFNEGLGETPLATFORMDISPLAYEXTPROC eglGetPlatformDisplayEXT;
PFNEGLCREATEPLATFORMWINDOWSURFACEEXTPROC eglCreatePlatformWindowSurfaceEXT;

struct _CGLContextObj {
    GLuint retainCount;
    pthread_mutex_t lock;
    int x,y,w,h;
    GLint opacity;
    unsigned long window; // address of WLWindow
    unsigned long windowNumber; // same

    EGLNativeDisplayType display;
    EGLDisplay egl_display;
    EGLContext egl_context;
    EGLNativeWindowType egl_window;
    EGLSurface egl_surface;
    EGLConfig egl_config;

    GLint program;
};

static GLint loadShader(NSString *path, GLenum type)
{
    GLint result;
    GLint shader = glCreateShader(type);
    if(!shader)
        return 0;

    struct stat st;
    int fd = open([path UTF8String], O_RDONLY);
    if(fd < 0)
        return 0;
    fstat(fd, &st);
    const char *src = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);

    glShaderSource(shader, 1, &src, NULL);
    glCompileShader(shader);
    glGetShaderiv(shader, GL_COMPILE_STATUS, &result);
    if(!result) {
        GLint infolen = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infolen);
        if(infolen) {
            char *log = malloc(infolen * sizeof(char));
            glGetShaderInfoLog(shader, infolen, NULL, log);
            NSLog(@"ERROR: shader compile failed: %s", log);
            free(log);
        }
        glDeleteShader(shader);
        shader = 0;
    }
    munmap(src, st.st_size);
    close(fd);
    return shader;
}

struct _CGLPixelFormatObj {
   GLuint retainCount;
   CGLPixelFormatAttribute *attributes;
};

static pthread_key_t cglContextKey;

static void make_key(){
   pthread_key_create(&cglContextKey,NULL);
}

static pthread_key_t cglThreadKey(){
   static pthread_once_t key_once=PTHREAD_ONCE_INIT;
   
   pthread_once(&key_once,make_key);

   return cglContextKey;
}

CGLContextObj CGLGetCurrentContext(void) {
   CGLContextObj result=pthread_getspecific(cglThreadKey());
   
   return result;
}

CGLError CGLSetCurrentContext(CGLContextObj context) {
    pthread_setspecific(cglThreadKey(), context);
      
    if(context==NULL)
       eglMakeCurrent(EGL_NO_DISPLAY, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT); 
    else    
       eglMakeCurrent(context->egl_display, context->egl_surface, context->egl_surface, context->egl_context);
   
    return kCGLNoError;
}

static inline bool attributeHasArgument(CGLPixelFormatAttribute attribute){
   switch(attribute){
    case kCGLPFAAuxBuffers:
    case kCGLPFAColorSize:
    case kCGLPFAAlphaSize:
    case kCGLPFADepthSize:
    case kCGLPFAStencilSize:
    case kCGLPFAAccumSize:
    case kCGLPFARendererID:
    case kCGLPFADisplayMask:
     return TRUE;
     
    default:
     return FALSE;
   }
}

static GLint *addAttribute(GLint *attribList,int *capacity,int *count,GLint value){

   if(*count>=*capacity){
    *capacity*=2;
    attribList=realloc(attribList,*capacity*sizeof(GLint));
   }
   
   attribList[(*count)++]=value;
   
   return attribList;
}

static GLint *attributesFromPixelFormat(CGLPixelFormatObj pixelFormat){
   int    resultCapacity=8,resultCount=0;
   GLint *result=malloc(resultCapacity*sizeof(GLint));
   int  i,virtualScreen=0;

   result=addAttribute(result,&resultCapacity,&resultCount,GL_RGBA);

   for(i=0;pixelFormat->attributes[i]!=0;i++){
    CGLPixelFormatAttribute attribute=pixelFormat->attributes[i];

    if(attributeHasArgument(pixelFormat->attributes[i]))
     i++;

    switch(attribute){
    
     case kCGLPFAColorSize:
      result=addAttribute(result,&resultCapacity,&resultCount,EGL_RED_SIZE);
      result=addAttribute(result,&resultCapacity,&resultCount,pixelFormat->attributes[i]/3);
      result=addAttribute(result,&resultCapacity,&resultCount,EGL_GREEN_SIZE);
      result=addAttribute(result,&resultCapacity,&resultCount,pixelFormat->attributes[i]/3);
      result=addAttribute(result,&resultCapacity,&resultCount,EGL_BLUE_SIZE);
      result=addAttribute(result,&resultCapacity,&resultCount,pixelFormat->attributes[i]/3);
      break;
      
     case kCGLPFAAlphaSize:
      result=addAttribute(result,&resultCapacity,&resultCount,EGL_ALPHA_SIZE);
      result=addAttribute(result,&resultCapacity,&resultCount,pixelFormat->attributes[i]);
      break;
      
     case kCGLPFAAccumSize:
      result=addAttribute(result,&resultCapacity,&resultCount,EGL_RED_SIZE);
      result=addAttribute(result,&resultCapacity,&resultCount,pixelFormat->attributes[i]/4);
      result=addAttribute(result,&resultCapacity,&resultCount,EGL_GREEN_SIZE);
      result=addAttribute(result,&resultCapacity,&resultCount,pixelFormat->attributes[i]/4);
      result=addAttribute(result,&resultCapacity,&resultCount,EGL_BLUE_SIZE);
      result=addAttribute(result,&resultCapacity,&resultCount,pixelFormat->attributes[i]/4);
      result=addAttribute(result,&resultCapacity,&resultCount,EGL_ALPHA_SIZE);
      result=addAttribute(result,&resultCapacity,&resultCount,pixelFormat->attributes[i]/4);
      break;
      
     case kCGLPFADepthSize:
      result=addAttribute(result,&resultCapacity,&resultCount,EGL_DEPTH_SIZE);
      result=addAttribute(result,&resultCapacity,&resultCount,pixelFormat->attributes[i]);
      break;
      
     case kCGLPFAStencilSize:
      result=addAttribute(result,&resultCapacity,&resultCount,EGL_STENCIL_SIZE);
      result=addAttribute(result,&resultCapacity,&resultCount,pixelFormat->attributes[i]);
      break;
      
     case kCGLPFAAuxBuffers:
      //result=addAttribute(result,&resultCapacity,&resultCount,EGL_AUX_BUFFERS);
      //result=addAttribute(result,&resultCapacity,&resultCount,pixelFormat->attributes[i]);
      NSLog(@"ERROR: unsupported pixel format kCGLPFAAuxBuffers");
      break;
    }
    
   }
   result=addAttribute(result,&resultCapacity,&resultCount,0L);
   
   return result;
}

CGLError CGLCreateContextForWindow(CGLPixelFormatObj pixelFormat,CGLContextObj share,CGLContextObj *resultp,unsigned long window) {
   CGLContextObj context=malloc(sizeof(struct _CGLContextObj));

    WLWindow *w = (__bridge WLWindow *)((void *)window);
    NSRect frame = [w frame];

    EGLint attrs[] = {
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_RED_SIZE, 1,
        EGL_GREEN_SIZE, 1,
        EGL_BLUE_SIZE, 1,
        EGL_ALPHA_SIZE, 1,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGL_NONE
    };

    context->retainCount=1;
    pthread_mutex_init(&(context->lock),NULL);
    context->display=[(WLDisplay *)[NSDisplay currentDisplay] display];
    context->window = window;
    context->x=0;
    context->y=0;
    context->w=frame.size.width;
    context->h=frame.size.height;
    context->opacity=1;
    context->windowNumber=0;

    eglGetPlatformDisplayEXT = eglGetProcAddress("eglGetPlatformDisplayEXT");
    if(eglGetPlatformDisplayEXT  == NULL)
        context->egl_display = eglGetDisplay(context->display);
    else
        context->egl_display = eglGetPlatformDisplayEXT(
                EGL_PLATFORM_WAYLAND_EXT, context->display, NULL);

    eglCreatePlatformWindowSurfaceEXT = eglGetProcAddress("eglCreatePlatformWindowSurfaceEXT");

    if(context->egl_display == EGL_NO_DISPLAY) {
        NSLog(@"failed to create EGL display");
        return kCGLBadDisplay;
    }
    
    EGLint major, minor;
    if(!eglInitialize(context->egl_display, &major, &minor)) {
        NSLog(@"error initializing EGL display");
        return kCGLBadDisplay;
    }

    EGLint matched = 0;
    if (!eglChooseConfig(context->egl_display, attrs, &context->egl_config, 1, &matched)) {
        NSLog(@"ERROR: eglChooseConfig failed");
        return kCGLBadMatch;
    }
    if (matched == 0) {
        NSLog(@"Failed to match an EGL config");
        return kCGLBadMatch;
    }

    EGLint ctxattrs[] = {
        EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE
    };

    context->egl_context = eglCreateContext(context->egl_display, context->egl_config,
        EGL_NO_CONTEXT, ctxattrs);
    if (context->egl_context == EGL_NO_CONTEXT) {
        NSLog(@"Failed to create EGL context");
        return kCGLBadMatch;
    }

    if(window) {
        NSRect frame = CGOutsetRectForNativeWindowBorder([w frame], [w styleMask]);
        context->egl_window = wl_egl_window_create([w wl_surface], frame.size.width,
            frame.size.height);

        context->egl_surface = eglCreatePlatformWindowSurfaceEXT(context->egl_display,
            context->egl_config, (EGLNativeWindowType)context->egl_window, NULL);
        if(context->egl_surface == EGL_BAD_PARAMETER) {
            NSLog(@"ERROR: bad EGL parameter");
            return kCGLBadMatch;
        }
        if(context->egl_surface == EGL_NO_DISPLAY) {
            NSLog(@"ERROR: no suitable display");
            return kCGLBadDisplay;
        }

        eglMakeCurrent(context->egl_display, context->egl_surface,
            context->egl_surface, context->egl_context);

        // Load vertex and fragment shaders. GLES 2.0 won't render without them
        NSString *path = [[NSBundle bundleForClass:[WLWindow class]]
            pathForResource:@"vertShader" ofType:@"glsl" inDirectory:@"WindowServer.backend"];
        GLint vShader = loadShader(path, GL_VERTEX_SHADER);
        path = [[NSBundle bundleForClass:[WLWindow  class]]
            pathForResource:@"fragShader" ofType:@"glsl" inDirectory:@"WindowServer.backend"];
        GLint fShader = loadShader(path, GL_FRAGMENT_SHADER);
        context->program = glCreateProgram();
        if(context->program == 0) {
            NSLog(@"failed to create shader program");
            return kCGLBadDisplay;
        }
        glAttachShader(context->program, vShader);
        glAttachShader(context->program, fShader);
        glLinkProgram(context->program);
    
        GLint result;
        glGetProgramiv(context->program, GL_LINK_STATUS, &result);
        if(!result) {
            GLint infolen = 0;
            glGetProgramiv(context->program, GL_INFO_LOG_LENGTH, &infolen);
            if(infolen) {
                char *log = malloc(infolen * sizeof(char));
                glGetProgramInfoLog(context->program, infolen, NULL, log);
                NSLog(@"ERROR: shader link failed: %s", log);
                free(log);
            }
            glDeleteProgram(context->program);
            return kCGLBadDisplay;
        }

        wl_surface_commit([w wl_surface]);
        wl_display_roundtrip(context->display);
    }
   
    *resultp=context;
    return kCGLNoError;
}

CGLError CGLCreateContext(CGLPixelFormatObj pixelFormat,CGLContextObj share,CGLContextObj *resultp) {
   GLint      *attribList=attributesFromPixelFormat(pixelFormat);
   return CGLCreateContextForWindow(pixelFormat,share,resultp,0);
}

CGLContextObj CGLRetainContext(CGLContextObj context) {
   if(context==NULL)
    return NULL;

   context->retainCount++;
   return context;
}

void CGLReleaseContext(CGLContextObj context) {
   if(context==NULL)
    return;
    
   context->retainCount--;
   
   if(context->retainCount==0){
    if(CGLGetCurrentContext()==context)
     CGLSetCurrentContext(NULL);
    
    if(context->egl_surface)
       eglDestroySurface(context->egl_display, context->egl_surface);
    if(context->egl_window)
        wl_egl_window_destroy(context->egl_window);

    pthread_mutex_destroy(&(context->lock));
    eglDestroyContext(context->egl_display, context->egl_context);
    free(context);
   }
   
}

GLuint CGLGetContextRetainCount(CGLContextObj context) {
   if(context==NULL)
    return 0;

   return context->retainCount;
}

CGLError CGLDestroyContext(CGLContextObj context) {
   CGLReleaseContext(context);

   return kCGLNoError;
}

CGLError CGLLockContext(CGLContextObj context) {
   pthread_mutex_lock(&(context->lock));
   return kCGLNoError;
}

CGLError CGLUnlockContext(CGLContextObj context) {
   pthread_mutex_unlock(&(context->lock));
   return kCGLNoError;
}

static bool usesChildWindow(CGLContextObj context){
   return (context->opacity!=0)?TRUE:FALSE;
}

static void adjustFrameInParent(CGLContextObj context,WLWindow *parentWindow,GLint *x,GLint *y,GLint *w,GLint *h){
   fprintf(stderr, "adjustFrameInParent %p\n", parentWindow);
   if(parentWindow!=nil){
    CGFloat top,left,bottom,right;

    CGNativeBorderFrameWidthsForStyle([parentWindow styleMask],&top,&left,&bottom,&right);

    *y=[parentWindow frame].size.height-(*y+*h);

    *y-=top;
    *x-=left;
   }
}

static void adjustInParentForSurfaceOpacity(CGLContextObj context){
   GLint x=context->x;
   GLint y=context->y;
   GLint w=context->w;
   GLint h=context->h;

   if(usesChildWindow(context)){
    //X11Window *parentWindow=[X11Window windowWithWindowNumber:context->windowNumber];
    //Window     parentHandle=[parentWindow windowHandle];
   
    //adjustFrameInParent(context,parentWindow,&x,&y,&w,&h);

    //XReparentWindow(context->display, parentHandle, context->window, x, y);
   }
   
   //XMoveResizeWindow(context->display,context->window, x, y, w, h);
   fprintf(stderr,"adjustInParent %d,%d %dx%d\n",x,y,w,h);
}

CGLError CGLSetParameter(CGLContextObj context,CGLContextParameter parameter,const GLint *value) {
   switch(parameter){
    
    case kCGLCPSurfaceFrame:;
     context->x=value[0];
     context->y=value[1];
     context->w=value[2];
     context->h=value[3];
     adjustInParentForSurfaceOpacity(context);     
     break;
    
    case kCGLCPSurfaceOpacity:
     context->opacity=*value;
     adjustInParentForSurfaceOpacity(context);
     break;
    
    case kCGLCPWindowNumber:
     context->windowNumber=*value;
     adjustInParentForSurfaceOpacity(context);
     break;
     
    default:
     NSLog(@"CGLSetParameter unimplemented for parameter %i",parameter);
     break;
   }
  
   return kCGLNoError;
}

CGLError CGLGetParameter(CGLContextObj context,CGLContextParameter parameter,GLint *value) { 
   switch(parameter){
   
    case kCGLCPSurfaceOpacity:
     *value=context->opacity;
     break;
    
    default:
     break;
   }
   
   return kCGLNoError;
}

CGLError CGLFlushDrawable(CGLContextObj context) {
    if(!context)
        return kCGLNoError;
    eglSwapInterval(context->egl_display, 0); // don't block
    eglSwapBuffers(context->egl_display, context->egl_surface);
    return kCGLNoError;
}

static int attributesCount(const CGLPixelFormatAttribute *attributes){
   int result;
   
   for(result=0;attributes[result]!=0;result++)
    if(attributeHasArgument(attributes[result]))
     result++;
   
   return result;
}

CGLError CGLChoosePixelFormat(const CGLPixelFormatAttribute *attributes,CGLPixelFormatObj *pixelFormatp,GLint *numberOfScreensp) {
   CGLPixelFormatObj result=malloc(sizeof(struct _CGLPixelFormatObj));
   int               i,count=attributesCount(attributes);
     
   result->retainCount=1;
   result->attributes=malloc(sizeof(CGLPixelFormatAttribute)*count);
   for(i=0;i<count;i++)
    result->attributes[i]=attributes[i];
   
   *pixelFormatp=result;
   *numberOfScreensp=1;
   
   return kCGLNoError;
}

CGLPixelFormatObj CGLRetainPixelFormat(CGLPixelFormatObj pixelFormat) {
   if(pixelFormat==NULL)
    return NULL;
    
   pixelFormat->retainCount++;
   return pixelFormat;
}

void CGLReleasePixelFormat(CGLPixelFormatObj pixelFormat) {
   if(pixelFormat==NULL)
    return;
    
   pixelFormat->retainCount--;
   
   if(pixelFormat->retainCount==0){
    free(pixelFormat->attributes);
    free(pixelFormat);
   }
}

CGLError CGLDestroyPixelFormat(CGLPixelFormatObj pixelFormat) {
   CGLReleasePixelFormat(pixelFormat);
   return kCGLNoError;
}

GLuint CGLGetPixelFormatRetainCount(CGLPixelFormatObj pixelFormat) {
   return pixelFormat->retainCount;
}

CGLError CGLDescribePixelFormat(CGLPixelFormatObj pixelFormat,GLint screenNumber,CGLPixelFormatAttribute attribute,GLint *valuesp) {
   int i;
   
   for(i=0;pixelFormat->attributes[i]!=0;i++){
    bool hasArgument=attributeHasArgument(pixelFormat->attributes[i]);
    
    if(pixelFormat->attributes[i]==attribute){
     if(hasArgument)
      *valuesp=pixelFormat->attributes[i+1];
     else
      *valuesp=1;
     
     return kCGLNoError;
    }
    
    if(hasArgument)
     i++;
   }
   *valuesp=0;
     return kCGLNoError;
}

void CGLBufferData(GLenum target,GLsizeiptr size,const GLvoid *data,GLenum usage) {
    glBufferData(target,size,data,usage);
}

void CGLGenBuffers(GLsizei n,GLuint *buffers) {
    glGenBuffers(n,buffers);
}

void CGLDeleteBuffers(GLsizei n,const GLuint *buffers) {
    glDeleteBuffers(n,buffers);
}

void CGLBindBuffer(GLenum target,GLuint buffer) {
    glBindBuffer(target,buffer);
}

void *CGLMapBuffer(GLenum target,GLenum access) {
    return glMapBuffer(target,access);
}

CGL_EXPORT GLboolean CGLUnmapBuffer(GLenum target) {
    return glUnmapBuffer(target);
}

void CGLBufferSubData(GLenum target,GLintptr offset,GLsizeiptr size,const GLvoid *data) {
    glBufferSubData(target,offset,size,data);
}

void CGLSurfaceResize(CGLContextObj context, int width, int height) {
    wl_egl_window_resize(context->egl_window, width, height, 0, 0);
}

void CGLUseShaders(CGLContextObj context) {
    glUseProgram(context->program);
}

