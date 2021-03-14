#import <OpenGL/OpenGL.h>
#import <Foundation/NSString.h>
#import <Foundation/NSRaise.h>
#import <stdbool.h>
#import <CoreGraphics/CoreGraphics.h>
#import <CoreGraphics/CGWindow.h>
#import <CoreGraphics/CGLPixelSurface.h>
#import <Onyx2D/O2Surface_DIBSection.h>
#import <Onyx2D/O2DeviceContext_gdi.h>
#import <pthread.h>
#import "Win32Window.h"
#import <windowsx.h>
#import "Win32EventInputSource.h"

/* There is essentially only two ways to implement a CGLContext on Windows.

   1) A double buffered off-screen window, using glReadPixels on the back buffer. You can't use a single buffered window because the front buffer is typically considered on-screen and will more likely fail the pixel ownership test. You can't use the front buffer in a double buffered window for the same reasons.
   
     Technique 1 works on a lot of systems, but not all. Some drivers will consider an off-screen window as failing the pixel ownership test and not produce any results (black). Some nVidia card/drivers exhibit this behavior.
     Technique 1 is also slow and there are no acceleration options, the pixels must be retrieved with glReadPixels
     
     
   2) Two pbuffers. Because pbuffers can't be resized you must use one pbuffer for the rendering context and another which is destroyed/created for the backing. The one small static pbuffer is used as the primary rendering context and participates in wglShareLists and preserves rendering context state during resizes. A second pbuffer is used as the backing HDC, this pbuffer is destroyed/created during a context resize (pbuffers can't be resized), no HGLRC is needed for this pbuffer (although we create one as a side effect of pbuffer management). This is all possible because wglMakeCurrent accepts both a HDC and HGLRC, provided the HDC and HGLRC have the same pixel format.
   
   Technique 2 is used if pbuffers are available, 1 is the fallback.
   
   You can't use FBO's because they are context state and you can't hide them from the application software. E.g. if the application switches to fbo 0 (default), and CGLContext is using 1 for the backing things will fail. Pbuffer doesn't have this problem, pbuffer is also older and more common on Windows at least. 
      
   You can't use child windows (this was the first implementation) because they have a number of problems, e.g. no transparency, they are not supported well in layered windows, you can't have multiple child windows with different pixels formats and they have a number of visual artifacts which are hard to control, e.g. flicker during resize.
   
   VMWare Fusion 3 (as of April 4,2011) does not support pbuffers so you can't test it there.
   Parallels Desktop (as of April 4,2011) does support pbuffers but the implementation appears slightly buggy. There is a workaround present (see WARNING in the pbuffer resize code).
    */
 
#import "opengl_dll.h"
HDC CGLGetWindowDC(CGLContextObj context);

struct _CGLPBufferObj {
   GLuint retainCount;
   GLsizei width;
   GLsizei height;
   GLenum target;
   GLenum internalFormat;
   GLint maxDetail;
   HPBUFFERARB pBuffer;
   HDC dc;
};

struct _CGLContextObj {
   GLuint           retainCount;
   CRITICAL_SECTION lock; // This must be a recursive lock.
   HWND             parent;
   HWND             window;
   int              x,y;
   int              w,h;
   GLint            opacity;
   GLint            forceChildWindow;
   GLint            hidden;
   GLint            inParent;
   CGLPixelSurface  *overlay;
   CGLPBufferObj    dynamicpBuffer;
   BOOL             needsViewport;
   HDC              windowDC;
   HGLRC            windowGLContext;
   int              parentWindowNumber;
};

struct _CGLPixelFormatObj {
   GLuint retainCount;
   CGLPixelFormatAttribute *attributes;
};

static BOOL usesChildWindow(CGLContextObj context);

// FIXME: there should be a lock around initialization of this
static DWORD cglThreadStorageIndex(){
   static DWORD tlsIndex=TLS_OUT_OF_INDEXES;

   if(tlsIndex==TLS_OUT_OF_INDEXES)
    tlsIndex=TlsAlloc();

   if(tlsIndex==TLS_OUT_OF_INDEXES)
    NSLog(@"TlsAlloc failed in CGLContext");

   return tlsIndex;
}

static LRESULT CALLBACK windowProcedure(HWND handle,UINT message,WPARAM wParam,LPARAM lParam){

    if(message==WM_PAINT){    
        Win32Window *parentWindow=GetProp(handle,"Win32Window");

#if 1
        ValidateRect(handle, NULL);

        return [parentWindow WM_APP1_wParam:wParam lParam:lParam];
#else        
        PAINTSTRUCT paintStruct;
        RECT        updateRECT;
        GetWindowRect(handle,&updateRECT);
        InvalidateRect(handle,&updateRECT,FALSE);
        if(GetUpdateRect(handle,&updateRECT,NO)){
            BeginPaint(handle,&paintStruct);
            [parentWindow WM_APP1_wParam:wParam lParam:lParam];
            EndPaint(handle,&paintStruct);
        }
#endif

        return 0;
    }

    if(message==WM_SETCURSOR){
        Win32Window *parentWindow=GetProp(handle,"Win32Window");

        return [parentWindow WM_SETCURSOR_wParam:wParam lParam:lParam];
    }
    
    if(message==WM_ACTIVATE)
        return 0;
        
   return DefWindowProc(handle,message,wParam,lParam);
}

/* This is a dedicated thread for creating and dispatching messages to OpenGL context windows. 

   Each HWND in Win32 is associated with the thread it is created on. Messages for that window are
   sent to the associated threads input queue. So, for example if you send a MoveWindow to a window,
   all the underlying messages related to the MoveWindow are sent to the input queue on the associated thread.
   
   The problem is that the application developer can not be expected to process the messages on the creating
   thread. In a single threaded AppKit program under typical condition that is OK because you are processing messages,
   but in a multi-threaded environment where the creating thread is not handling messages this can be a problem.

   So, we create all OpenGL windows on a dedicated thread which handles the messages.

 */
CRITICAL_SECTION requestCriticalSection;

static void initializeRequest(){
   InitializeCriticalSection(&requestCriticalSection);
}

void CGLInitializeIfNeeded(){
   static pthread_once_t once=PTHREAD_ONCE_INIT;
   
   pthread_once(&once,initializeRequest);
   
   EnterCriticalSection(&requestCriticalSection);
   
   static bool registerWindowClass=FALSE;
   
   if(!registerWindowClass){
    static WNDCLASSEX windowClass;
    
    windowClass.cbSize=sizeof(WNDCLASSEX);
    windowClass.style=CS_HREDRAW|CS_VREDRAW|CS_OWNDC|CS_DBLCLKS;
    windowClass.lpfnWndProc=windowProcedure;
    windowClass.cbClsExtra=0;
    windowClass.cbWndExtra=0;
    windowClass.hInstance=NULL;
    windowClass.hIcon=NULL;
    windowClass.hCursor=LoadCursor(NULL,IDC_ARROW);
    windowClass.hbrBackground=NULL;
    windowClass.lpszMenuName=NULL;
    windowClass.lpszClassName="CGLWindow";
    windowClass.hIconSm=NULL;
    
    if(RegisterClassEx(&windowClass)==0)
     NSLog(@"RegisterClass failed %s %d",__FILE__,__LINE__);
     
    registerWindowClass=TRUE;
   }
   LeaveCriticalSection(&requestCriticalSection);
}

CGL_EXPORT CGLContextObj CGLGetCurrentContext(void) {
   CGLContextObj result=TlsGetValue(cglThreadStorageIndex());
   
   return result;
}

static int reportGLErrorIfNeeded(const char *function,int line){
  // return GL_NO_ERROR;
   
   GLenum error=glGetError();

   if(error!=GL_NO_ERROR)
     NSLog(@"%s %d error=%d/%x",function,line,error,error);
   
   return error;
}

static void _CGLCreateDynamicPbufferBacking(CGLContextObj context);
static void _CGLDestroyDynamicPbufferBacking(CGLContextObj context);
 CGLError _CGLSetCurrentContextFromThreadLocal(int value);

static void adjustFrameInParent(CGLContextObj context,Win32Window *parentWindow,GLint *x,GLint *y,GLint *w,GLint *h){
   if(parentWindow!=nil){
    CGFloat top,left,bottom,right;

    CGNativeBorderFrameWidthsForStyle([parentWindow styleMask],&top,&left,&bottom,&right);

    *y=[parentWindow frame].size.height-(*y+*h);

    *y-=top;
    *x-=left;
   }
}
static int resizeBackingIfNeeded(CGLContextObj context){
    opengl_wglMakeCurrent(context->windowDC,context->windowGLContext);

   if(context->w<0)
    context->w=0;
   if(context->h<0)
    context->h=0;
       
      /* If we're using a Pbuffer we don't want the window large because it consumes resources */

    if(usesChildWindow(context) || (context->dynamicpBuffer==NULL)){
        GLint x=context->x;
        GLint y=context->y;
        GLint w=context->w;
        GLint h=context->h;
        
        adjustFrameInParent(context,[CGWindow windowWithWindowNumber:context->parentWindowNumber],&x,&y,&w,&h);
        
        MoveWindow(context->window,x,y,w,h,NO);
    }
    else {
// Window context must be current for pBuffer functions to work. Since this is now only called during a SetCurrent, no need to wglMakeCurrent

/* WARNING:
 *   We retain the old dynamic pbuffer here, create a new one, then release the old one. There is a bug (mis-feature?) in the Parallels driver
 *   at least which will cause invalid pbuffers to be returned if you immediately free then alloc a new one. If you save the old one and free
 *   it AFTER the new one is allocated it all works.
 *   I am not aware of this being a problem in other drivers.
 *
 *   This can be problematic with big pbuffers as the old one would be around while allocating a new one. Could possibly special case for just
 *   Parallels as that appears to be the only place it happens.
 */
        CGLPBufferObj releaseThis = CGLRetainPBuffer(context->dynamicpBuffer);
    
        _CGLDestroyDynamicPbufferBacking(context);
        _CGLCreateDynamicPbufferBacking(context);
   
        CGLReleasePBuffer(releaseThis);
    }   
    
    [context->overlay setFrameSize:O2SizeMake(context->w,context->h)];
   
    return 1;
}

 CGLError _CGLSetCurrentContextFromThreadLocal(int value){
    CGLContextObj context=TlsGetValue(cglThreadStorageIndex());
   
    if(context==NULL)
        opengl_wglMakeCurrent(NULL,NULL);
    else {
        if(usesChildWindow(context) || context->dynamicpBuffer==NULL) {
            opengl_wglMakeCurrent(context->windowDC,context->windowGLContext);
            reportGLErrorIfNeeded(__PRETTY_FUNCTION__,__LINE__);
        }
        else {
            int lost;
#if 0
            opengl_wglQueryPbufferARB(context->dynamicpBuffer->pBuffer, WGL_PBUFFER_LOST_ARB, &lost);
     
            if(lost)
                NSLog(@"lost dynamic pBuffer %d",value);
#endif

            opengl_wglMakeCurrent(context->dynamicpBuffer->dc,context->windowGLContext);
            if(context->needsViewport){
                glViewport(0,0,context->w,context->h);
                context->needsViewport=NO;
            }
            reportGLErrorIfNeeded(__PRETTY_FUNCTION__,__LINE__);
        }
    }
   
    return kCGLNoError;
}

CGL_EXPORT CGLError CGLSetCurrentContext(CGLContextObj context) {
   TlsSetValue(cglThreadStorageIndex(),context);
   return _CGLSetCurrentContextFromThreadLocal(0);
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

static void pfdFromPixelFormat(PIXELFORMATDESCRIPTOR *pfd,CGLPixelFormatObj pixelFormat){
   int  i,virtualScreen=0;
   
   memset(pfd,0,sizeof(PIXELFORMATDESCRIPTOR));
   
   pfd->nSize=sizeof(PIXELFORMATDESCRIPTOR);
   pfd->nVersion=1;
   
   /* It has to be double buffered regardless of what the application asks for, because we're reading from it, all the pixels must be
      valid. A single buffer context is problematic in that the driver may not render obscured pixels, all of them since it is off-screen.
      That isnt a problem with pbuffers but this is the fallback. */
#ifndef PFD_SUPPORT_COMPOSITION
    #define PFD_SUPPORT_COMPOSITION 0x00008000
#endif

   pfd->dwFlags=PFD_SUPPORT_OPENGL|PFD_DRAW_TO_WINDOW|PFD_DOUBLEBUFFER;
   pfd->iLayerType=PFD_MAIN_PLANE;
   pfd->iPixelType=PFD_TYPE_RGBA;
   pfd->cColorBits=24;
   pfd->cRedBits=8;
   pfd->cGreenBits=8;
   pfd->cBlueBits=8;
   pfd->cAlphaBits=8;
   pfd->cDepthBits=24;
return;
//ignored for now
   for(i=0;pixelFormat->attributes[i]!=0;i++){
    CGLPixelFormatAttribute attribute=pixelFormat->attributes[i];

    if(attributeHasArgument(pixelFormat->attributes[i]))
     i++;

    switch(attribute){
    
     case kCGLPFAColorSize:
      pfd->cColorBits=pixelFormat->attributes[i];
      break;
      
     case kCGLPFAAlphaSize:
      pfd->cAlphaBits=pixelFormat->attributes[i];
      break;
      
     case kCGLPFAAccumSize:
      pfd->cAccumBits=pixelFormat->attributes[i];
      break;
      
     case kCGLPFADepthSize:
      pfd->cDepthBits=pixelFormat->attributes[i];
      break;
      
     case kCGLPFAStencilSize:
      pfd->cStencilBits=pixelFormat->attributes[i];
      break;
      
     case kCGLPFAAuxBuffers:
      pfd->cAuxBuffers=pixelFormat->attributes[i];
      break;
    }
    
   }
}

static BOOL contextHasPbufferExtension(CGLContextObj context){
    const char *extensions=opengl_wglGetExtensionsStringARB(context->windowDC);
            
  // NSLog(@"extensions=%s",extensions);
        
   reportGLErrorIfNeeded(__PRETTY_FUNCTION__,__LINE__);
    
    if(extensions!=NULL) {
        if(strstr(extensions,"WGL_ARB_pbuffer")!=NULL){
            if(strstr(extensions,"WGL_ARB_pixel_format")!=NULL)
                return YES;
        }

        if(strstr(extensions,"WGL_EXT_pbuffer")!=NULL){
            if(strstr(extensions,"WGL_EXT_pixel_format")!=NULL)
                return YES;
        }
    }

    extensions=opengl_wglGetExtensionsStringEXT(context->windowDC);

    if(extensions!=NULL) {
        if(strstr(extensions,"WGL_ARB_pbuffer")!=NULL){
            if(strstr(extensions,"WGL_ARB_pixel_format")!=NULL)
                return YES;
        }

        if(strstr(extensions,"WGL_EXT_pbuffer")!=NULL){
            if(strstr(extensions,"WGL_EXT_pixel_format")!=NULL)
                return YES;
        }
    }

    return NO;
}


void _CGLCreateDynamicPbufferBacking(CGLContextObj context){
   
    if(!contextHasPbufferExtension(context))
        return;

    if(context->dynamicpBuffer==NULL){
        if(CGLCreatePBuffer(context->w,context->h,WGL_TEXTURE_2D_ARB,WGL_TEXTURE_RGBA_ARB,0,&(context->dynamicpBuffer))!=kCGLNoError)
            return;
            
        if(CGLSetPBuffer(context,context->dynamicpBuffer,0,0,0)!=kCGLNoError){
            NSLog(@"CGLSetPBuffer failed for dynamic pBuffer");
            CGLReleasePBuffer(context->dynamicpBuffer);
            context->dynamicpBuffer=NULL;
            return;
        }
    }

    context->needsViewport=YES;
}



void _CGLCreateBufferBackingIfPossible(CGLContextObj context){   
// Window context must be current for pBuffer functions to work.  

   opengl_wglMakeCurrent(context->windowDC,context->windowGLContext);
   reportGLErrorIfNeeded(__PRETTY_FUNCTION__,__LINE__);

   const char *extensions=glGetString(GL_EXTENSIONS);
   
  // NSLog(@"extensions=%s",extensions);
  // NSLog(@"vendor=%s",glGetString(GL_VENDOR));
   
   _CGLCreateDynamicPbufferBacking(context);
   _CGLSetCurrentContextFromThreadLocal(0);
}

static void _CGLDestroyDynamicPbufferBacking(CGLContextObj context){
// Window context must be current for pBuffer functions to work.  

    opengl_wglMakeCurrent(context->windowDC,context->windowGLContext);
    reportGLErrorIfNeeded(__PRETTY_FUNCTION__,__LINE__);

    CGLReleasePBuffer(context->dynamicpBuffer);
           
    context->dynamicpBuffer=NULL;
}


CGL_EXPORT CGLError CGLCreateContext(CGLPixelFormatObj pixelFormat,CGLContextObj share,CGLContextObj *resultp) {
   CGLContextObj         context=NSZoneCalloc(NULL,1,sizeof(struct _CGLContextObj));
   PIXELFORMATDESCRIPTOR pfd;
   int                   pfIndex;
   
   CGLInitializeIfNeeded();

   context->retainCount=1;
   
   pfdFromPixelFormat(&pfd,pixelFormat);

   InitializeCriticalSection(&(context->lock));
   
   context->w=64;
   context->h=64;

    context->forceChildWindow = TRUE;

    context->parent=CreateWindowEx(WS_EX_TOOLWINDOW|WS_EX_NOACTIVATE,"CGLWindow","",WS_POPUP|WS_CLIPCHILDREN|WS_CLIPSIBLINGS,0,0,10,10,NULL,NULL,GetModuleHandle(NULL),NULL);
    context->window=CreateWindowEx(WS_EX_TOOLWINDOW|WS_EX_NOACTIVATE,"CGLWindow","",WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS,0,0,context->w,context->h,context->parent,NULL,GetModuleHandle(NULL),NULL);
   
   context->windowDC=GetDC(context->window);

   pfIndex=ChoosePixelFormat(context->windowDC,&pfd); 

   if(!SetPixelFormat(context->windowDC,pfIndex,&pfd))
    NSLog(@"SetPixelFormat failed");

   context->windowGLContext=opengl_wglCreateContext(context->windowDC);
   
/* Creating a CGL context does NOT set it as current, if you need the context to be current,
   save it and restore it. Some applications may create a context while using another, so
   we don't want to improperly switch to another context during creating */
      
   _CGLCreateBufferBackingIfPossible(context);
   
   if(share!=NULL){
    HGLRC shareGL=share->windowGLContext;
    HGLRC otherGL=context->windowGLContext;

    if(!opengl_wglShareLists(shareGL,otherGL))
     NSLog(@"opengl_wglShareLists failed");
   }
   
   context->opacity=1;

   context->overlay=[[CGLPixelSurface alloc] initWithSize:O2SizeMake(context->w,context->h)];
   [context->overlay setOpaque:YES];
   
   *resultp=context;
   
   return kCGLNoError;
}

CGL_EXPORT CGLContextObj CGLRetainContext(CGLContextObj context) {
//if(NSDebugEnabled) NSCLog("%s %d %s %p",__FILE__,__LINE__,__PRETTY_FUNCTION__,context);
   if(context==NULL)
    return NULL;
    
   context->retainCount++;
      
   return context;
}

CGL_EXPORT void CGLReleaseContext(CGLContextObj context) {
//if(NSDebugEnabled) NSCLog("%s %d %s %p",__FILE__,__LINE__,__PRETTY_FUNCTION__,context);
   if(context==NULL)
    return;
    
   context->retainCount--;
   
   if(context->retainCount==0){

    [[CGWindow windowWithWindowNumber:context->parentWindowNumber] removeCGLContext:context]; 

    if(CGLGetCurrentContext()==context)
     CGLSetCurrentContext(NULL);
        
    _CGLDestroyDynamicPbufferBacking(context);

    ReleaseDC(context->window,context->windowDC);
    DestroyWindow(context->window);
    DestroyWindow(context->parent);

    [context->overlay release];

    DeleteCriticalSection(&(context->lock));
    opengl_wglDeleteContext(context->windowGLContext);
    
    
    NSZoneFree(NULL,context);
   }
}

CGL_EXPORT GLuint CGLGetContextRetainCount(CGLContextObj context) {
//if(NSDebugEnabled) NSCLog("%s %d %s %p",__FILE__,__LINE__,__PRETTY_FUNCTION__,context);
   if(context==NULL)
    return 0;

   return context->retainCount;
}

CGL_EXPORT CGLError CGLDestroyContext(CGLContextObj context) {
//if(NSDebugEnabled) NSCLog("%s %d %s %p",__FILE__,__LINE__,__PRETTY_FUNCTION__,context);
   CGLReleaseContext(context);

   return kCGLNoError;
}

CGL_EXPORT CGLError CGLLockContext(CGLContextObj context) {
   EnterCriticalSection(&(context->lock));
   return kCGLNoError;
}

CGL_EXPORT CGLError CGLUnlockContext(CGLContextObj context) {
   LeaveCriticalSection(&(context->lock));
   return kCGLNoError;
}

static BOOL usesChildWindow(CGLContextObj context){        
    Win32Window *parentWindow=[CGWindow windowWithWindowNumber:context->parentWindowNumber];
    
    if(parentWindow==nil)
        return NO;
    
    if([parentWindow isLayeredWindow])
        return NO;

    return context->forceChildWindow;
}

static BOOL shouldPutChildInParent(CGLContextObj context) {
    if(!usesChildWindow(context))
        return NO;

    if(context->hidden)
        return NO;
    
    return YES;
}

static void reflectChildWindowState(CGLContextObj context,GLint force) {
    if(shouldPutChildInParent(context)){
        if(!context->inParent || force){
            context->inParent=TRUE;
            
            Win32Window *parentWindow=[CGWindow windowWithWindowNumber:context->parentWindowNumber];
        
            SetParent(context->window,[parentWindow windowHandle]);
            SetProp(context->window,"parent",[parentWindow windowHandle]);
            SetProp(context->window,"Win32Window",parentWindow);
            ShowWindow(context->window,SW_SHOWNOACTIVATE);
        }
    }
    else {
        if(context->inParent || force){
            context->inParent=FALSE;
            
            ShowWindow(context->window,SW_HIDE);
            SetParent(context->window,context->parent);
            SetProp(context->window,"parent",NULL);
            SetProp(context->window,"Win32Window",NULL);
        }
    }
}

CGL_EXPORT CGLError CGLSetParameter(CGLContextObj context,CGLContextParameter parameter,const GLint *value) {

   switch(parameter){
    case kCGLCPSwapInterval:;
     CGLSetCurrentContext(context);
     
     opengl_wglSwapIntervalEXT(*value);
     break;
    
    case kCGLCPSurfaceOpacity:
     context->opacity=*value;
     [context->overlay setOpaque:context->opacity?YES:NO];
     break;
    
    case kCGLCPSurfaceBackingSize:;
     BOOL sizeChanged=(context->w!=value[0] || context->h!=value[1])?YES:NO;
     
     if(sizeChanged){
      context->w=value[0];
      context->h=value[1];
      
      /* We immediately resize so that the resize happens on the thread does the resize, otherwise MoveWindow can deadlock. */ 
      resizeBackingIfNeeded(context);
     }
     break;
     
        case kCGLCPSurfaceIsChildWindow:
            if(context->forceChildWindow!=value[0]){
                context->forceChildWindow=value[0];
                reflectChildWindowState(context,YES);
            }
            break;
            
        case kCGLCPSurfaceBackingOrigin:;
            BOOL originChanged=(context->x!=value[0] || context->y!=value[1])?YES:NO;
     
            if(originChanged){
                context->x=value[0];
                context->y=value[1];
                /* We immediately resize so that the resize happens on the thread does the resize, otherwise MoveWindow can deadlock. */ 
                resizeBackingIfNeeded(context);
            }
            break;

        case kCGLCPSurfaceWindowNumber:{
                GLint didChange=(context->parentWindowNumber!=value[0]);
            
                if(didChange){
                    [[CGWindow windowWithWindowNumber:context->parentWindowNumber] removeCGLContext:context]; 
                    context->parentWindowNumber=value[0];
                    [[CGWindow windowWithWindowNumber:context->parentWindowNumber] addCGLContext:context];
                }
                reflectChildWindowState(context,didChange);
            }
            break;

        case kCGLCPSurfaceHidden:
            context->hidden=value[0];
            reflectChildWindowState(context,FALSE);
            break;
            
        default:
            NSUnimplementedFunction();
            break;
   }

   return kCGLNoError;
}

CGL_EXPORT CGLError CGLGetParameter(CGLContextObj context,CGLContextParameter parameter,GLint *value) { 
//if(NSDebugEnabled) NSCLog("%s %d %p CGLGetParameter parameter=%d",__FILE__,__LINE__,context,parameter);
   switch(parameter){
   
    case kCGLCPSurfaceOpacity:
     *value=context->opacity;
     break;
         
    case kCGLCPSurfaceWindowNumber:
     *value=context->parentWindowNumber;
     break;
     
    case kCGLCPOverlayPointer:
     *value=(int)(context->overlay);
     break;
     
        case kCGLCPSurfaceBackingSize:;
        value[0]=context->w;
        value[1]=context->h;
            break;
            
    case kCGLCPSurfaceIsChildWindow:
     *value=context->forceChildWindow;
     break;
     
    case kCGLCPSurfaceBackingOrigin:
        value[0]=context->x;
        value[1]=context->y;
        break;
        
    default:
     break;
   }
   
   return kCGLNoError;
}

CGL_EXPORT CGLError CGLCopyPixelsFromSurface(O2Surface_DIBSection *srcSurface,CGLContextObj destination) {
    
    O2Surface_DIBSection *dstSurface=(O2Surface_DIBSection *)[destination->overlay validSurface];
    BLENDFUNCTION blend;
    
    blend.BlendOp=AC_SRC_OVER;
    blend.BlendFlags=0;
    blend.SourceConstantAlpha=255;
    blend.AlphaFormat=0;
    
    int width=O2ImageGetWidth(srcSurface);
    int height=O2ImageGetHeight(srcSurface);
    
    O2SurfaceLock(dstSurface);
        O2SurfaceLock(srcSurface);
            AlphaBlend([[dstSurface deviceContext] dc],0,0,width,height,[[srcSurface deviceContext] dc],0,0,width,height,blend);
        O2SurfaceUnlock(srcSurface);
    O2SurfaceUnlock(dstSurface);

    return kCGLNoError;
}

HDC CGLGetWindowDC(CGLContextObj context){
    return context->windowDC;
}

HDC CGLGetDC(CGLContextObj context){
    if(context->dynamicpBuffer!=NULL)
        return context->dynamicpBuffer->dc;
        
    return context->windowDC;
}

CGLPBufferObj CGLGetRetainedPBuffer(CGLContextObj context){
    return CGLRetainPBuffer(context->dynamicpBuffer);
}

O2Surface *CGLGetSurface(CGLContextObj context){    
    CGLPixelSurface *overlay=context->overlay;
        
    return (O2Surface_DIBSection *)[overlay validSurface];
}

CGL_EXPORT CGLError CGLCopyPixels(CGLContextObj source,CGLContextObj destination) {
    CGLPixelSurface *overlay=source->overlay;
        
    O2Surface_DIBSection *dstSurface=(O2Surface_DIBSection *)[destination->overlay validSurface];
    O2Surface_DIBSection *srcSurface=(O2Surface_DIBSection *)[overlay validSurface];
    
    if(srcSurface==nil)
        return kCGLNoError;

    BLENDFUNCTION blend;
    
    blend.BlendOp=AC_SRC_OVER;
    blend.BlendFlags=0;
    blend.SourceConstantAlpha=255;
    blend.AlphaFormat=source->opacity?0:AC_SRC_ALPHA;

    int y=destination->h-(source->y+source->h);
     
    O2SurfaceLock(dstSurface);
        O2SurfaceLock(srcSurface);
            AlphaBlend([[dstSurface deviceContext] dc],source->x,y,source->w,source->h,[[srcSurface deviceContext] dc],0,0,source->w,source->h,blend);
        O2SurfaceUnlock(srcSurface);
    O2SurfaceUnlock(dstSurface);

    return kCGLNoError;
}

CGLError CGLFlushDrawable(CGLContextObj context) {
    if(usesChildWindow(context)){
        if(!SwapBuffers(context->windowDC))
            NSLog(@"SwapBuffers failed, error = %d", GetLastError());
    }
    else {
        Win32Window *parentWindow=[CGWindow windowWithWindowNumber:context->parentWindowNumber];
        
        [parentWindow flushCGLContext:context];
    }

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

CGL_EXPORT CGLError CGLDescribePixelFormat(CGLPixelFormatObj pixelFormat,GLint screenNumber,CGLPixelFormatAttribute attribute,GLint *valuesp) {
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

CGLError CGLCreatePBuffer(GLsizei width,GLsizei height,GLenum target,GLenum internalFormat,GLint maxDetail,CGLPBufferObj *pbufferp) {
   CGLPBufferObj pbuffer=calloc(1,sizeof(struct _CGLPBufferObj));
   pbuffer->retainCount=1;
   pbuffer->width=width;
   pbuffer->height=height;
   pbuffer->target=target;
   pbuffer->internalFormat=internalFormat;
   pbuffer->maxDetail=maxDetail;
   *pbufferp=pbuffer;
   return kCGLNoError;
}

CGLError CGLDescribePBuffer(CGLPBufferObj pbuffer,GLsizei *width,GLsizei *height,GLenum *target,GLenum *internalFormat,GLint *mipmap) {
   *width=pbuffer->width;
   *height=pbuffer->height;
   *target=pbuffer->target;
   *internalFormat=pbuffer->internalFormat;
   *mipmap=pbuffer->maxDetail;
   return kCGLNoError;
}

HPBUFFERARB CGLGetPBUFFER(CGLPBufferObj pbuffer){
    return pbuffer->pBuffer;
}
CGLPBufferObj CGLRetainPBuffer(CGLPBufferObj pbuffer) {
   if(pbuffer==NULL)
    return NULL;
    
   pbuffer->retainCount++;
   return pbuffer;
}

void CGLReleasePBuffer(CGLPBufferObj pbuffer) {
   if(pbuffer==NULL)
    return;
    
   pbuffer->retainCount--;
   
   if(pbuffer->retainCount==0){
   
    if(pbuffer->dc!=NULL){
        opengl_wglReleasePbufferDCARB(pbuffer->pBuffer,pbuffer->dc);
        reportGLErrorIfNeeded(__PRETTY_FUNCTION__,__LINE__);
    }
   
    if(pbuffer->pBuffer!=NULL){
        opengl_wglDestroyPbufferARB(pbuffer->pBuffer);
        reportGLErrorIfNeeded(__PRETTY_FUNCTION__,__LINE__);
    }
    
    free(pbuffer);
   }
}

GLuint CGLGetPBufferRetainCount(CGLPBufferObj pbuffer) {
   return pbuffer->retainCount;
}

CGLError CGLDestroyPBuffer(CGLPBufferObj pbuffer) {
   CGLReleasePBuffer(pbuffer);
   return kCGLNoError;
}

CGLError CGLGetPBuffer(CGLContextObj context,CGLPBufferObj *pbuffer,GLenum *face,GLint *level,GLint *screen) {
   return kCGLNoError;
}

CGLError CGLSetPBuffer(CGLContextObj context,CGLPBufferObj pbuffer,GLenum face,GLint level,GLint screen) {
   int piFormats[1];
   
   /** XXX: this must match window context setup. */
   int piAttribIList[]={
        WGL_SUPPORT_OPENGL_ARB,GL_TRUE,
        WGL_DRAW_TO_PBUFFER_ARB,GL_TRUE,
        WGL_COLOR_BITS_ARB,24,
        WGL_ALPHA_BITS_ARB,8,
        WGL_DEPTH_BITS_ARB,24,
        WGL_PIXEL_TYPE_ARB,WGL_TYPE_RGBA_ARB,
        WGL_BIND_TO_TEXTURE_RGBA_ARB, GL_TRUE,
        WGL_ACCELERATION_ARB,WGL_FULL_ACCELERATION_ARB,
        WGL_DOUBLE_BUFFER_ARB,GL_TRUE,
        0
   }; 
        			
   UINT nNumFormats=0;	
    
   if(opengl_wglChoosePixelFormatARB(CGLGetWindowDC(context),piAttribIList,NULL,1,piFormats,&nNumFormats)==FALSE){
    NSLog(@"wglChoosePixelFormatARB returned %i", GetLastError());
    return kCGLBadAttribute;			
   }
    
   if(nNumFormats==0){
    NSLog(@"opengl_wglChoosePixelFormatARB return nNumFormats==0");
    return kCGLBadAttribute;
   }

    const int attributes[]= {
        WGL_TEXTURE_FORMAT_ARB,
        WGL_TEXTURE_RGBA_ARB, // p-buffer will have RBA texture format
        WGL_TEXTURE_TARGET_ARB,
        WGL_TEXTURE_2D_ARB,
        0
    }; // Of texture target will be GL_TEXTURE_2D

    pbuffer->pBuffer=opengl_wglCreatePbufferARB(CGLGetWindowDC(context), piFormats[0], pbuffer->width, pbuffer->height, attributes);
    reportGLErrorIfNeeded(__PRETTY_FUNCTION__,__LINE__);
    pbuffer->dc=opengl_wglGetPbufferDCARB(pbuffer->pBuffer);
    reportGLErrorIfNeeded(__PRETTY_FUNCTION__,__LINE__);

   int width=0;
   int height=0;
   opengl_wglQueryPbufferARB(pbuffer->pBuffer, WGL_PBUFFER_WIDTH_ARB, &width);
   reportGLErrorIfNeeded(__PRETTY_FUNCTION__,__LINE__);
   
   opengl_wglQueryPbufferARB(pbuffer->pBuffer, WGL_PBUFFER_HEIGHT_ARB, &height);
   reportGLErrorIfNeeded(__PRETTY_FUNCTION__,__LINE__);
    
   if(width!=pbuffer->width)
    NSLog(@"opengl_wglQueryPbufferARB width!=pbuffer->w");
    
   if(height!=pbuffer->height)
    NSLog(@"opengl_wglQueryPbufferARB height!=pbuffer->h");

    return kCGLNoError;
}

CGLError CGLTexImagePBuffer(CGLContextObj context,CGLPBufferObj pbuffer,GLenum sourceBuffer) {
    opengl_wglBindTexImageARB(pbuffer->pBuffer,sourceBuffer);
    reportGLErrorIfNeeded(__PRETTY_FUNCTION__,__LINE__);
    return kCGLNoError;
}

