#import <Foundation/NSObject.h>
#import <OpenGL/gl.h>
#import <OpenGL/glext.h>
#import <stdbool.h>
#import <windows.h>
#import <OpenGL/wglext.h>

HGLRC opengl_wglCreateContext(HDC dc);
BOOL opengl_wglDeleteContext(HGLRC hglrc);
HGLRC opengl_wglGetCurrentContext(void);
BOOL opengl_wglMakeCurrent(HDC dc, HGLRC hglrc);
BOOL opengl_wglMakeCurrent(HDC dc, HGLRC hglrc);
PROC opengl_wglGetProcAddress(LPCSTR name);
BOOL opengl_wglShareLists(HGLRC hglrc1, HGLRC hglrc2);
void opengl_glReadBuffer(GLenum mode);
void opengl_glGetIntegerv(GLenum pname, GLint *params);
void opengl_glDrawBuffer(GLenum mode);
void opengl_glReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels);

bool opengl_hasPixelBufferObject();
void opengl_glGenBuffers(GLsizei n, GLuint *buffers);
void opengl_glBindBuffer(GLenum target, GLuint buffer);
void opengl_glBufferData(GLenum target, GLsizeiptr size, const GLvoid *bytes, GLenum usage);
void opengl_glBufferSubData(GLenum target, GLsizeiptr offset, GLsizeiptr size, const GLvoid *bytes);
void opengl_glDeleteBuffers(GLsizei n, const GLuint *buffers);
void opengl_glGetBufferParameteriv(GLenum target, GLenum value, GLint *data);
GLvoid *opengl_glMapBuffer(GLenum target, GLenum access);
GLboolean opengl_glUnmapBuffer(GLenum target);

const char *opengl_wglGetExtensionsStringARB(HDC hdc);
const char *opengl_wglGetExtensionsStringEXT(HDC hdc);

HPBUFFERARB opengl_wglCreatePbufferARB(HDC hDC, int iPixelFormat, int iWidth, int iHeight, const int *piAttribList);

HDC opengl_wglGetPbufferDCARB(HPBUFFERARB hPbuffer);

int opengl_wglReleasePbufferDCARB(HPBUFFERARB hPbuffer, HDC hDC);

BOOL opengl_wglDestroyPbufferARB(HPBUFFERARB hPbuffer);

BOOL opengl_wglQueryPbufferARB(HPBUFFERARB hPbuffer, int iAttribute, int *piValue);

BOOL opengl_wglGetPixelFormatAttribivARB(HDC hdc, int iPixelFormat, int iLayerPlane, UINT nAttributes, const int *piAttributes, int *piValues);
BOOL opengl_wglGetPixelFormatAttribfvARB(HDC hdc, int iPixelFormat, int iLayerPlane, UINT nAttributes, const int *piAttributes, FLOAT *pfValues);

BOOL opengl_wglChoosePixelFormatARB(HDC hdc, const int *piAttribIList, const FLOAT *pfAttribFList, UINT nMaxFormats, int *piFormats, UINT *nNumFormats);

void opengl_glGenFramebuffersEXT(GLsizei, GLuint *);
void opengl_glDeleteFramebuffersEXT(GLsizei, const GLuint *);
void opengl_glBindFramebufferEXT(GLenum, GLuint);

void opengl_glGenRenderbuffersEXT(GLsizei, GLuint *);
void opengl_glDeleteRenderbuffersEXT(GLsizei, const GLuint *);
void opengl_glRenderbufferStorageEXT(GLenum, GLenum, GLsizei, GLsizei);
void opengl_glBindRenderbufferEXT(GLenum, GLuint);

void opengl_glFramebufferRenderbufferEXT(GLenum, GLenum, GLenum, GLuint);

GLenum opengl_glCheckFramebufferStatusEXT(GLenum target);

void opengl_glFramebufferTexture2DEXT(GLenum target, GLenum attachmentPoint, GLenum textureTarget, GLuint textureId, GLint level);

BOOL opengl_wglMakeContextCurrentARB(HDC hDrawDC, HDC hReadDC, HGLRC hglrc);

BOOL opengl_wglBindTexImageARB(HPBUFFERARB hPbuffer, int iBuffer);
BOOL opengl_wglReleaseTexImageARB(HPBUFFERARB hPbuffer, int iBuffer);
BOOL opengl_wglSetPbufferAttribARB(HPBUFFERARB hPbuffer, const int *piAttribList);

BOOL opengl_wglSwapIntervalEXT(int interval);

void opengl_glAddSwapHintRectWIN(GLint x, GLint y, GLsizei width, GLsizei height);
