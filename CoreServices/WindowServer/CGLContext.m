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

#import <OpenGL/OpenGL.h>
#import <Foundation/NSString.h>
#import <Foundation/NSRaise.h>

#import <GL/gl.h>
#import <GL/glx.h>
#import <GL/glext.h>
#import <pthread.h>
#import <sys/stat.h>

#import "WLWindow.h"

/* FIXME: This class doesn't do anything for WLWindows yet. It's for
 *        supporting EGL eventually
 */

struct _CGLContextObj {
   GLuint           retainCount;
   pthread_mutex_t lock;
   int          x,y,w,h;
   GLint        opacity;
   unsigned long window; // address of WLWindow
   int          windowNumber;
};

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
      
   //if(context==NULL)
    //eglMakeCurrent(egl_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT); 
   //else    
    //eglMakeCurrent(egl_display, context->egl_surface, context->egl_surface, context->egl_context);
   
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

   result=addAttribute(result,&resultCapacity,&resultCount,GLX_RGBA);

   for(i=0;pixelFormat->attributes[i]!=0;i++){
    CGLPixelFormatAttribute attribute=pixelFormat->attributes[i];

    if(attributeHasArgument(pixelFormat->attributes[i]))
     i++;

    switch(attribute){
    
     case kCGLPFAColorSize:
      result=addAttribute(result,&resultCapacity,&resultCount,GLX_RED_SIZE);
      result=addAttribute(result,&resultCapacity,&resultCount,pixelFormat->attributes[i]/3);
      result=addAttribute(result,&resultCapacity,&resultCount,GLX_GREEN_SIZE);
      result=addAttribute(result,&resultCapacity,&resultCount,pixelFormat->attributes[i]/3);
      result=addAttribute(result,&resultCapacity,&resultCount,GLX_BLUE_SIZE);
      result=addAttribute(result,&resultCapacity,&resultCount,pixelFormat->attributes[i]/3);
      break;
      
     case kCGLPFAAlphaSize:
      result=addAttribute(result,&resultCapacity,&resultCount,GLX_ALPHA_SIZE);
      result=addAttribute(result,&resultCapacity,&resultCount,pixelFormat->attributes[i]);
      break;
      
     case kCGLPFAAccumSize:
      result=addAttribute(result,&resultCapacity,&resultCount,GLX_ACCUM_RED_SIZE);
      result=addAttribute(result,&resultCapacity,&resultCount,pixelFormat->attributes[i]/4);
      result=addAttribute(result,&resultCapacity,&resultCount,GLX_ACCUM_GREEN_SIZE);
      result=addAttribute(result,&resultCapacity,&resultCount,pixelFormat->attributes[i]/4);
      result=addAttribute(result,&resultCapacity,&resultCount,GLX_ACCUM_BLUE_SIZE);
      result=addAttribute(result,&resultCapacity,&resultCount,pixelFormat->attributes[i]/4);
      result=addAttribute(result,&resultCapacity,&resultCount,GLX_ACCUM_ALPHA_SIZE);
      result=addAttribute(result,&resultCapacity,&resultCount,pixelFormat->attributes[i]/4);
      break;
      
     case kCGLPFADepthSize:
      result=addAttribute(result,&resultCapacity,&resultCount,GLX_DEPTH_SIZE);
      result=addAttribute(result,&resultCapacity,&resultCount,pixelFormat->attributes[i]);
      break;
      
     case kCGLPFAStencilSize:
      result=addAttribute(result,&resultCapacity,&resultCount,GLX_STENCIL_SIZE);
      result=addAttribute(result,&resultCapacity,&resultCount,pixelFormat->attributes[i]);
      break;
      
     case kCGLPFAAuxBuffers:
      result=addAttribute(result,&resultCapacity,&resultCount,GLX_AUX_BUFFERS);
      result=addAttribute(result,&resultCapacity,&resultCount,pixelFormat->attributes[i]);
      break;
    }
    
   }
   result=addAttribute(result,&resultCapacity,&resultCount,None);
   
   return result;
}

CGLError CGLCreateContextForWindow(CGLPixelFormatObj pixelFormat,CGLContextObj share,CGLContextObj *resultp,unsigned long window) {
   CGLContextObj context=malloc(sizeof(struct _CGLContextObj));

    WLWindow *glw = (__bridge WLWindow *)((void *)window);
    NSRect frame = [glw frame];

    context->retainCount=1;
    pthread_mutex_init(&(context->lock),NULL);
    //context->display=display;
    //context->visualInfo=visualInfo;
    context->window = window;
    context->x=0;
    context->y=0;
    context->w=frame.size.width;
    context->h=frame.size.height;
    context->opacity=1;
    context->windowNumber=0;
   
    *resultp=context;
   
    return kCGLNoError;
}

CGLError CGLCreateContext(CGLPixelFormatObj pixelFormat,CGLContextObj share,CGLContextObj *resultp) {
   GLint      *attribList=attributesFromPixelFormat(pixelFormat);
//    GLint       attribList[] = {GLX_RGBA, GLX_DOUBLEBUFFER, GLX_RED_SIZE, 4, GLX_GREEN_SIZE, 4,
//                                GLX_BLUE_SIZE, 4, GLX_DEPTH_SIZE, 4, None};
      
   //window = XCreateWindow(display,parent, 0, 0, 1,1, 0, visualInfo->depth, InputOutput, visualInfo->visual, CWBorderPixel | CWColormap | CWEventMask, &xattr);
      
   //XSetWindowBackgroundPixmap(display, window, None);
   //[X11Window removeDecorationForWindow:window onDisplay:display];
   
   //XMapWindow(display, window);

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
    
   //if(context->window)
      //XDestroyWindow(context->display, context->window);

    pthread_mutex_destroy(&(context->lock));
    //glXDestroyContext(context->display, context->glc);
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
//   glXSwapBuffers(context->display,context->window);
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
