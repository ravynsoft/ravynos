#import "opengl_dll.h"
#import <Foundation/NSString.h>
#import <Foundation/NSDebug.h>
#import <OpenGL/glext.h>

HGLRC opengl_wglCreateContext(HDC dc) {
   return wglCreateContext(dc);
   }
    
BOOL  opengl_wglDeleteContext(HGLRC hglrc) {
   return wglDeleteContext(hglrc);
   }
    
HGLRC opengl_wglGetCurrentContext(void) {
   return wglGetCurrentContext();
   }
    
BOOL  opengl_wglMakeCurrent(HDC dc,HGLRC hglrc) {
   return wglMakeCurrent(dc,hglrc);
   }

PROC  opengl_wglGetProcAddress(LPCSTR name){
   return wglGetProcAddress(name);
   }

BOOL opengl_wglShareLists(HGLRC hglrc1,HGLRC hglrc2) {
   return wglShareLists(hglrc1,hglrc2);
}

void  opengl_glReadBuffer(GLenum mode) {
   glReadBuffer(mode);
}

void opengl_glGetIntegerv(GLenum pname,GLint *params) {
   glGetIntegerv(pname,params);
}

void  opengl_glDrawBuffer(GLenum mode) {
   glDrawBuffer(mode);
}

void opengl_glReadPixels(GLint x,GLint y,GLsizei width,GLsizei height,GLenum format,GLenum type,GLvoid *pixels) {
   glReadPixels(x,y,width,height,format,type,pixels);
}

bool opengl_hasPixelBufferObject(){
   
   if(wglGetProcAddress("glGenBuffersARB")==NULL)
    return FALSE;
    
   if(wglGetProcAddress("glBindBufferARB")==NULL)
    return FALSE;
    
   if(wglGetProcAddress("glBufferDataARB")==NULL)
    return FALSE;
    
   if(wglGetProcAddress("glBufferSubDataARB")==NULL)
    return FALSE;
    
   if(wglGetProcAddress("glDeleteBuffersARB")==NULL)
    return FALSE;
    
   if(wglGetProcAddress("glGetBufferParameterivARB")==NULL)
    return FALSE;
    
   if(wglGetProcAddress("glMapBufferARB")==NULL)
    return FALSE;
    
   if(wglGetProcAddress("glUnmapBufferARB")==NULL)
    return FALSE;
    
   return TRUE;
}

void opengl_glGenBuffers(GLsizei n,GLuint *buffers) {
   PFNGLGENBUFFERSARBPROC function=(PFNGLGENBUFFERSARBPROC)wglGetProcAddress("glGenBuffersARB");
   function(n,buffers);
}

void opengl_glBindBuffer(GLenum target,GLuint buffer) {
   PFNGLBINDBUFFERARBPROC function=(PFNGLBINDBUFFERARBPROC)wglGetProcAddress("glBindBufferARB");
   function(target,buffer);
}

void opengl_glBufferData(GLenum target,GLsizeiptr size,const GLvoid *bytes,GLenum usage) {
   PFNGLBUFFERDATAARBPROC function=(PFNGLBUFFERDATAARBPROC)wglGetProcAddress("glBufferDataARB");
   function(target,size,bytes,usage);
}

void opengl_glBufferSubData(GLenum target,GLsizeiptr offset,GLsizeiptr size,const GLvoid *bytes) {
   PFNGLBUFFERSUBDATAARBPROC function=(PFNGLBUFFERSUBDATAARBPROC)wglGetProcAddress("glBufferSubDataARB");
   function(target,offset,size,bytes);
}

void opengl_glDeleteBuffers(GLsizei n,const GLuint *buffers) {
   PFNGLDELETEBUFFERSARBPROC function=(PFNGLDELETEBUFFERSARBPROC)wglGetProcAddress("glDeleteBuffersARB");
   function(n,buffers);
}

void opengl_glGetBufferParameteriv(GLenum target,GLenum value,GLint *data) {
   PFNGLGETBUFFERPARAMETERIVARBPROC function=(PFNGLGETBUFFERPARAMETERIVARBPROC)wglGetProcAddress("glGetBufferParameterivARB");
   function(target,value,data);
}

GLvoid *opengl_glMapBuffer(GLenum target,GLenum access) {
    PFNGLMAPBUFFERARBPROC function=(PFNGLMAPBUFFERARBPROC)wglGetProcAddress("glMapBufferARB");
   return function(target,access);
}

GLboolean opengl_glUnmapBuffer(GLenum target) {
   PFNGLUNMAPBUFFERARBPROC function=(PFNGLUNMAPBUFFERARBPROC)wglGetProcAddress("glUnmapBufferARB");
   return function(target);
}


const char *opengl_wglGetExtensionsStringARB(HDC hdc) {
   APIENTRY typeof(opengl_wglGetExtensionsStringARB) *function=(typeof(function))wglGetProcAddress("wglGetExtensionsStringARB");

   if(function==NULL){
    if(NSDebugEnabled)
     NSLog(@"wglGetProcAddress(wglGetExtensionsStringARB) failed");
    return NULL;
   }
   
   return function(hdc);
}

const char *opengl_wglGetExtensionsStringEXT(HDC hdc) {
   APIENTRY typeof(opengl_wglGetExtensionsStringEXT) *function=(typeof(function))wglGetProcAddress("wglGetExtensionsStringEXT");

   if(function==NULL){
    if(NSDebugEnabled)
     NSLog(@"wglGetProcAddress(wglGetExtensionsStringEXT) failed");
    return NULL;
   }
   
   return function(hdc);
}

HPBUFFERARB opengl_wglCreatePbufferARB(HDC hDC,int iPixelFormat,int iWidth,int iHeight,const int *piAttribList) {
   APIENTRY typeof(opengl_wglCreatePbufferARB) *function=(typeof(function))wglGetProcAddress("wglCreatePbufferARB");

   if(function==NULL){
    if(NSDebugEnabled)
     NSLog(@"wglGetProcAddress(wglCreatePbufferARB) failed");
    return NULL;
   }
   
   return function(hDC,iPixelFormat,iWidth,iHeight,piAttribList);
}

HDC  opengl_wglGetPbufferDCARB(HPBUFFERARB hPbuffer) {
   APIENTRY typeof(opengl_wglGetPbufferDCARB) *function=(typeof(function))wglGetProcAddress("wglGetPbufferDCARB");

   if(function==NULL){
    if(NSDebugEnabled)
     NSLog(@"wglGetProcAddress(wglGetPbufferDCARB) failed");
    return NULL;
   }
   
   return function(hPbuffer);
}

int  opengl_wglReleasePbufferDCARB(HPBUFFERARB hPbuffer,HDC hDC) {
   APIENTRY typeof(opengl_wglReleasePbufferDCARB) *function=(typeof(function))wglGetProcAddress("wglReleasePbufferDCARB");

   if(function==NULL){
    if(NSDebugEnabled)
     NSLog(@"wglGetProcAddress(wglReleasePbufferDCARB) failed");
    return 0;
   }
   
   return function(hPbuffer,hDC);
}

BOOL opengl_wglDestroyPbufferARB(HPBUFFERARB hPbuffer) {
   APIENTRY typeof(opengl_wglDestroyPbufferARB) *function=(typeof(function))wglGetProcAddress("wglDestroyPbufferARB");

   if(function==NULL){
    if(NSDebugEnabled)
     NSLog(@"wglGetProcAddress(wglDestroyPbufferARB) failed");
    return NO;
   }
   
   return function(hPbuffer);
}

BOOL opengl_wglQueryPbufferARB(HPBUFFERARB hPbuffer,int iAttribute,int *piValue) {
   APIENTRY typeof(opengl_wglQueryPbufferARB) *function=(typeof(function))wglGetProcAddress("wglQueryPbufferARB");

   if(function==NULL){
    if(NSDebugEnabled)
     NSLog(@"wglGetProcAddress(wglQueryPbufferARB) failed");
    return NO;
   }
   
   return function(hPbuffer,iAttribute,piValue);
}

BOOL opengl_wglGetPixelFormatAttribivARB(HDC hdc,int iPixelFormat,int iLayerPlane,UINT nAttributes,const int *piAttributes,int *piValues) {
   APIENTRY typeof(opengl_wglGetPixelFormatAttribivARB) *function=(typeof(function))wglGetProcAddress("wglGetPixelFormatAttribivARB");

   if(function==NULL){
    if(NSDebugEnabled)
     NSLog(@"wglGetProcAddress(wglGetPixelFormatAttribivARB) failed");
    return NO;
   }
   
   return function(hdc,iPixelFormat,iLayerPlane,nAttributes,piAttributes,piValues);
}

BOOL opengl_wglGetPixelFormatAttribfvARB(HDC hdc,int iPixelFormat,int iLayerPlane,UINT nAttributes,const int *piAttributes,FLOAT *pfValues) {
   APIENTRY typeof(opengl_wglGetPixelFormatAttribfvARB) *function=(typeof(function))wglGetProcAddress("wglGetPixelFormatAttribfvARB");

   if(function==NULL){
    if(NSDebugEnabled)
     NSLog(@"wglGetProcAddress(wglGetPixelFormatAttribfvARB) failed");
    return NO;
   }
   
   return function(hdc,iPixelFormat,iLayerPlane,nAttributes,piAttributes,pfValues);
}

BOOL opengl_wglChoosePixelFormatARB(HDC hdc,const int *piAttribIList,const FLOAT *pfAttribFList,UINT nMaxFormats,int *piFormats,UINT *nNumFormats) {
   APIENTRY typeof(opengl_wglChoosePixelFormatARB) *function=(typeof(function))wglGetProcAddress("wglChoosePixelFormatARB");

   if(function==NULL){
    if(NSDebugEnabled)
     NSLog(@"wglGetProcAddress(wglChoosePixelFormatARB) failed");
    return NO;
   }
   
   return function(hdc,piAttribIList,pfAttribFList,nMaxFormats,piFormats,nNumFormats);
}

void opengl_glGenFramebuffersEXT(GLsizei count,GLuint *results) {
   APIENTRY typeof(opengl_glGenFramebuffersEXT) *function=(typeof(function))wglGetProcAddress("glGenFramebuffersEXT");

   if(function==NULL){
    if(NSDebugEnabled)
     NSLog(@"wglGetProcAddress(glGenFramebuffersEXT) failed");
    return;
   }
   
   return function(count,results);
}

void opengl_glDeleteFramebuffersEXT (GLsizei count, const GLuint *idents) {
   APIENTRY typeof(opengl_glDeleteFramebuffersEXT) *function=(typeof(function))wglGetProcAddress("glDeleteFramebuffersEXT");

   if(function==NULL){
    if(NSDebugEnabled)
     NSLog(@"wglGetProcAddress(glDeleteFramebuffersEXT) failed");
    return;
   }
   
   return function(count,idents);
}


void opengl_glBindFramebufferEXT (GLenum target, GLuint ident) {
   APIENTRY typeof(opengl_glBindFramebufferEXT) *function=(typeof(function))wglGetProcAddress("glBindFramebufferEXT");

   if(function==NULL){
    if(NSDebugEnabled)
     NSLog(@"wglGetProcAddress(glBindFramebufferEXT) failed");
    return;
   }
   
   return function(target,ident);
}

void opengl_glGenRenderbuffersEXT (GLsizei count, GLuint *results) {
   APIENTRY typeof(opengl_glGenRenderbuffersEXT) *function=(typeof(function))wglGetProcAddress("glGenRenderbuffersEXT");

   if(function==NULL){
    if(NSDebugEnabled)
     NSLog(@"wglGetProcAddress(glGenRenderbuffersEXT) failed");
    return;
   }
   
   return function(count,results);
}

void opengl_glDeleteRenderbuffersEXT (GLsizei count, const GLuint *idents) {
   APIENTRY typeof(opengl_glDeleteRenderbuffersEXT) *function=(typeof(function))wglGetProcAddress("glDeleteRenderbuffersEXT");

   if(function==NULL){
    if(NSDebugEnabled)
     NSLog(@"wglGetProcAddress(glDeleteRenderbuffersEXT) failed");
    return;
   }
   
   return function(count,idents);
}

void opengl_glRenderbufferStorageEXT (GLenum target, GLenum internalFormat, GLsizei width, GLsizei height) {
   APIENTRY typeof(opengl_glRenderbufferStorageEXT) *function=(typeof(function))wglGetProcAddress("glRenderbufferStorageEXT");

   if(function==NULL){
    if(NSDebugEnabled)
     NSLog(@"wglGetProcAddress(glRenderbufferStorageEXT) failed");
    return;
   }
   
   return function(target,internalFormat,width,height);
}

void opengl_glBindRenderbufferEXT (GLenum target, GLuint ident) {
   APIENTRY typeof(opengl_glBindRenderbufferEXT) *function=(typeof(function))wglGetProcAddress("glBindRenderbufferEXT");

   if(function==NULL){
    if(NSDebugEnabled)
     NSLog(@"wglGetProcAddress(glBindRenderbufferEXT) failed");
    return;
   }
   
   function(target,ident);
}

void opengl_glFramebufferRenderbufferEXT (GLenum target, GLenum attachmentPoint, GLenum renderbufferTarget, GLuint renderbufferId) {
   APIENTRY typeof(opengl_glFramebufferRenderbufferEXT ) *function=(typeof(function))wglGetProcAddress("glFramebufferRenderbufferEXT");

   if(function==NULL){
    if(NSDebugEnabled)
     NSLog(@"wglGetProcAddress(glFramebufferRenderbufferEXT ) failed");
    return;
   }
   
   function(target,attachmentPoint,renderbufferTarget,renderbufferId);
}

GLenum opengl_glCheckFramebufferStatusEXT(GLenum target) {
   APIENTRY typeof(opengl_glCheckFramebufferStatusEXT ) *function=(typeof(function))wglGetProcAddress("glCheckFramebufferStatusEXT");

   if(function==NULL){
    if(NSDebugEnabled)
     NSLog(@"wglGetProcAddress(glCheckFramebufferStatusEXT ) failed");
    return GL_FRAMEBUFFER_UNSUPPORTED_EXT;
   }
   
   return function(target);
}

void opengl_glFramebufferTexture2DEXT(GLenum target, GLenum attachmentPoint,GLenum textureTarget,GLuint textureId,GLint level) {
   APIENTRY typeof(opengl_glFramebufferTexture2DEXT ) *function=(typeof(function))wglGetProcAddress("glFramebufferTexture2DEXT");

   if(function==NULL){
    if(NSDebugEnabled)
     NSLog(@"wglGetProcAddress(glFramebufferTexture2DEXT ) failed");
    return ;
   }
   
   function(target,attachmentPoint,textureTarget,textureId,level);
}

HGLRC opengl_wglCreateContextAttribsARB(HDC hDC, HGLRC hshareContext,const int *attribList) {
   APIENTRY typeof(opengl_wglCreateContextAttribsARB ) *function=(typeof(function))wglGetProcAddress("wglCreateContextAttribsARB");

   if(function==NULL){
 //   if(NSDebugEnabled)
     NSLog(@"wglGetProcAddress(wglCreateContextAttribsARB ) failed");
    return NO;
   }
   
   return function(hDC,hshareContext,attribList);
}


BOOL opengl_wglMakeContextCurrentARB(HDC hDrawDC,HDC hReadDC,HGLRC hglrc) {
   APIENTRY typeof(opengl_wglMakeContextCurrentARB ) *function=(typeof(function))wglGetProcAddress("wglMakeContextCurrentARB");

   if(function==NULL){
 //   if(NSDebugEnabled)
     NSLog(@"wglGetProcAddress(wglMakeContextCurrentARB ) failed");
    return NO;
   }
   
   return function(hDrawDC,hReadDC,hglrc);
}

BOOL opengl_wglBindTexImageARB(HPBUFFERARB hPbuffer,int iBuffer) {
   APIENTRY typeof(opengl_wglBindTexImageARB) *function=(typeof(function))wglGetProcAddress("wglBindTexImageARB");

   if(function==NULL){
 //   if(NSDebugEnabled)
     NSLog(@"wglGetProcAddress(wglBindTexImageARB) failed");
    return NO;
   }
   
   return function(hPbuffer,iBuffer);
}

BOOL opengl_wglReleaseTexImageARB(HPBUFFERARB hPbuffer,int iBuffer) {
   APIENTRY typeof(opengl_wglReleaseTexImageARB) *function=(typeof(function))wglGetProcAddress("wglReleaseTexImageARB");

   if(function==NULL){
 //   if(NSDebugEnabled)
     NSLog(@"wglGetProcAddress(wglReleaseTexImageARB) failed");
    return NO;
   }
   
   return function(hPbuffer,iBuffer);
}

BOOL opengl_wglSetPbufferAttribARB(HPBUFFERARB hPbuffer,const int *piAttribList) {
   APIENTRY typeof(opengl_wglSetPbufferAttribARB) *function=(typeof(function))wglGetProcAddress("wglSetPbufferAttribARB");

   if(function==NULL){
 //   if(NSDebugEnabled)
     NSLog(@"wglGetProcAddress(wglSetPbufferAttribARB) failed");
    return NO;
   }
   
   return function(hPbuffer,piAttribList);
}

BOOL opengl_wglSwapIntervalEXT (int interval) {
   APIENTRY typeof(opengl_wglSwapIntervalEXT) *function=(typeof(function))wglGetProcAddress("wglSwapIntervalEXT");


   if(function==NULL){
    NSLog(@"wglGetProcAddress(wglSwapIntervalEXT) failed");
    return NO;
   }
   
    return function(interval);
}

void opengl_glAddSwapHintRectWIN(GLint x,GLint y,GLsizei width,GLsizei height) {
   WINAPI typeof(opengl_glAddSwapHintRectWIN) *function=(typeof(function))wglGetProcAddress("glAddSwapHintRectWIN");


   if(function==NULL){
    NSLog(@"wglGetProcAddress(glAddSwapHintRectWIN) failed");
    return ;
   }
   
    return function(x,y,width,height);
}

