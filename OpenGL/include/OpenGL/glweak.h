#import <OpenGL/CGLTypes.h>
#import <OpenGL/gl.h>
#import <OpenGL/glext.h>

CGL_EXPORT void CGLBufferData(GLenum target, GLsizeiptr size, const GLvoid *data, GLenum usage);
CGL_EXPORT void CGLGenBuffers(GLsizei n, GLuint *buffers);
CGL_EXPORT void CGLDeleteBuffers(GLsizei n, const GLuint *buffers);
CGL_EXPORT void CGLBindBuffer(GLenum target, GLuint buffer);
CGL_EXPORT void CGLBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid *data);
CGL_EXPORT void *CGLMapBuffer(GLenum target, GLenum access);
CGL_EXPORT GLboolean CGLUnmapBuffer(GLenum target);

#define glBufferData(_1, _2, _3, _4) CGLBufferData(_1, _2, _3, _4)
#define glGenBuffers(_1, _2) CGLGenBuffers(_1, _2)
#define glBindBuffer(_1, _2) CGLBindBuffer(_1, _2)
#define glBufferSubData(_1, _2, _3, _4) CGLBufferSubData(_1, _2, _3, _4)
