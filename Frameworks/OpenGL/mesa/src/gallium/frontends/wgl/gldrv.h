/*
 * Copyright © Microsoft Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#ifndef _GLDRV_
#define _GLDRV_

#include <specstrings.h>

// Number of entries expected for various versions of OpenGL
#define OPENGL_VERSION_100_ENTRIES      306
#define OPENGL_VERSION_110_ENTRIES      336

typedef struct _GLDISPATCHTABLE {
    void      (APIENTRY *glNewList                )( GLuint list, GLenum mode );
    void      (APIENTRY *glEndList                )( void );
    void      (APIENTRY *glCallList               )( GLuint list );
    void      (APIENTRY *glCallLists              )( GLsizei n, GLenum type, const GLvoid *lists );
    void      (APIENTRY *glDeleteLists            )( GLuint list, GLsizei range );
    GLuint    (APIENTRY *glGenLists               )( GLsizei range );
    void      (APIENTRY *glListBase               )( GLuint base );
    void      (APIENTRY *glBegin                  )( GLenum mode );
    void      (APIENTRY *glBitmap                 )( GLsizei width, GLsizei height, GLfloat xorig, GLfloat yorig, GLfloat xmove, GLfloat ymove, const GLubyte *bitmap );
    void      (APIENTRY *glColor3b                )( GLbyte red, GLbyte green, GLbyte blue );
    void      (APIENTRY *glColor3bv               )( const GLbyte *v );
    void      (APIENTRY *glColor3d                )( GLdouble red, GLdouble green, GLdouble blue );
    void      (APIENTRY *glColor3dv               )( const GLdouble *v );
    void      (APIENTRY *glColor3f                )( GLfloat red, GLfloat green, GLfloat blue );
    void      (APIENTRY *glColor3fv               )( const GLfloat *v );
    void      (APIENTRY *glColor3i                )( GLint red, GLint green, GLint blue );
    void      (APIENTRY *glColor3iv               )( const GLint *v );
    void      (APIENTRY *glColor3s                )( GLshort red, GLshort green, GLshort blue );
    void      (APIENTRY *glColor3sv               )( const GLshort *v );
    void      (APIENTRY *glColor3ub               )( GLubyte red, GLubyte green, GLubyte blue );
    void      (APIENTRY *glColor3ubv              )( const GLubyte *v );
    void      (APIENTRY *glColor3ui               )( GLuint red, GLuint green, GLuint blue );
    void      (APIENTRY *glColor3uiv              )( const GLuint *v );
    void      (APIENTRY *glColor3us               )( GLushort red, GLushort green, GLushort blue );
    void      (APIENTRY *glColor3usv              )( const GLushort *v );
    void      (APIENTRY *glColor4b                )( GLbyte red, GLbyte green, GLbyte blue, GLbyte alpha );
    void      (APIENTRY *glColor4bv               )( const GLbyte *v );
    void      (APIENTRY *glColor4d                )( GLdouble red, GLdouble green, GLdouble blue, GLdouble alpha );
    void      (APIENTRY *glColor4dv               )( const GLdouble *v );
    void      (APIENTRY *glColor4f                )( GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha );
    void      (APIENTRY *glColor4fv               )( const GLfloat *v );
    void      (APIENTRY *glColor4i                )( GLint red, GLint green, GLint blue, GLint alpha );
    void      (APIENTRY *glColor4iv               )( const GLint *v );
    void      (APIENTRY *glColor4s                )( GLshort red, GLshort green, GLshort blue, GLshort alpha );
    void      (APIENTRY *glColor4sv               )( const GLshort *v );
    void      (APIENTRY *glColor4ub               )( GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha );
    void      (APIENTRY *glColor4ubv              )( const GLubyte *v );
    void      (APIENTRY *glColor4ui               )( GLuint red, GLuint green, GLuint blue, GLuint alpha );
    void      (APIENTRY *glColor4uiv              )( const GLuint *v );
    void      (APIENTRY *glColor4us               )( GLushort red, GLushort green, GLushort blue, GLushort alpha );
    void      (APIENTRY *glColor4usv              )( const GLushort *v );
    void      (APIENTRY *glEdgeFlag               )( GLboolean flag );
    void      (APIENTRY *glEdgeFlagv              )( const GLboolean *flag );
    void      (APIENTRY *glEnd                    )( void );
    void      (APIENTRY *glIndexd                 )( GLdouble c );
    void      (APIENTRY *glIndexdv                )( const GLdouble *c );
    void      (APIENTRY *glIndexf                 )( GLfloat c );
    void      (APIENTRY *glIndexfv                )( const GLfloat *c );
    void      (APIENTRY *glIndexi                 )( GLint c );
    void      (APIENTRY *glIndexiv                )( const GLint *c );
    void      (APIENTRY *glIndexs                 )( GLshort c );
    void      (APIENTRY *glIndexsv                )( const GLshort *c );
    void      (APIENTRY *glNormal3b               )( GLbyte nx, GLbyte ny, GLbyte nz );
    void      (APIENTRY *glNormal3bv              )( const GLbyte *v );
    void      (APIENTRY *glNormal3d               )( GLdouble nx, GLdouble ny, GLdouble nz );
    void      (APIENTRY *glNormal3dv              )( const GLdouble *v );
    void      (APIENTRY *glNormal3f               )( GLfloat nx, GLfloat ny, GLfloat nz );
    void      (APIENTRY *glNormal3fv              )( const GLfloat *v );
    void      (APIENTRY *glNormal3i               )( GLint nx, GLint ny, GLint nz );
    void      (APIENTRY *glNormal3iv              )( const GLint *v );
    void      (APIENTRY *glNormal3s               )( GLshort nx, GLshort ny, GLshort nz );
    void      (APIENTRY *glNormal3sv              )( const GLshort *v );
    void      (APIENTRY *glRasterPos2d            )( GLdouble x, GLdouble y );
    void      (APIENTRY *glRasterPos2dv           )( const GLdouble *v );
    void      (APIENTRY *glRasterPos2f            )( GLfloat x, GLfloat y );
    void      (APIENTRY *glRasterPos2fv           )( const GLfloat *v );
    void      (APIENTRY *glRasterPos2i            )( GLint x, GLint y );
    void      (APIENTRY *glRasterPos2iv           )( const GLint *v );
    void      (APIENTRY *glRasterPos2s            )( GLshort x, GLshort y );
    void      (APIENTRY *glRasterPos2sv           )( const GLshort *v );
    void      (APIENTRY *glRasterPos3d            )( GLdouble x, GLdouble y, GLdouble z );
    void      (APIENTRY *glRasterPos3dv           )( const GLdouble *v );
    void      (APIENTRY *glRasterPos3f            )( GLfloat x, GLfloat y, GLfloat z );
    void      (APIENTRY *glRasterPos3fv           )( const GLfloat *v );
    void      (APIENTRY *glRasterPos3i            )( GLint x, GLint y, GLint z );
    void      (APIENTRY *glRasterPos3iv           )( const GLint *v );
    void      (APIENTRY *glRasterPos3s            )( GLshort x, GLshort y, GLshort z );
    void      (APIENTRY *glRasterPos3sv           )( const GLshort *v );
    void      (APIENTRY *glRasterPos4d            )( GLdouble x, GLdouble y, GLdouble z, GLdouble w );
    void      (APIENTRY *glRasterPos4dv           )( const GLdouble *v );
    void      (APIENTRY *glRasterPos4f            )( GLfloat x, GLfloat y, GLfloat z, GLfloat w );
    void      (APIENTRY *glRasterPos4fv           )( const GLfloat *v );
    void      (APIENTRY *glRasterPos4i            )( GLint x, GLint y, GLint z, GLint w );
    void      (APIENTRY *glRasterPos4iv           )( const GLint *v );
    void      (APIENTRY *glRasterPos4s            )( GLshort x, GLshort y, GLshort z, GLshort w );
    void      (APIENTRY *glRasterPos4sv           )( const GLshort *v );
    void      (APIENTRY *glRectd                  )( GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2 );
    void      (APIENTRY *glRectdv                 )( const GLdouble *v1, const GLdouble *v2 );
    void      (APIENTRY *glRectf                  )( GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2 );
    void      (APIENTRY *glRectfv                 )( const GLfloat *v1, const GLfloat *v2 );
    void      (APIENTRY *glRecti                  )( GLint x1, GLint y1, GLint x2, GLint y2 );
    void      (APIENTRY *glRectiv                 )( const GLint *v1, const GLint *v2 );
    void      (APIENTRY *glRects                  )( GLshort x1, GLshort y1, GLshort x2, GLshort y2 );
    void      (APIENTRY *glRectsv                 )( const GLshort *v1, const GLshort *v2 );
    void      (APIENTRY *glTexCoord1d             )( GLdouble s );
    void      (APIENTRY *glTexCoord1dv            )( const GLdouble *v );
    void      (APIENTRY *glTexCoord1f             )( GLfloat s );
    void      (APIENTRY *glTexCoord1fv            )( const GLfloat *v );
    void      (APIENTRY *glTexCoord1i             )( GLint s );
    void      (APIENTRY *glTexCoord1iv            )( const GLint *v );
    void      (APIENTRY *glTexCoord1s             )( GLshort s );
    void      (APIENTRY *glTexCoord1sv            )( const GLshort *v );
    void      (APIENTRY *glTexCoord2d             )( GLdouble s, GLdouble t );
    void      (APIENTRY *glTexCoord2dv            )( const GLdouble *v );
    void      (APIENTRY *glTexCoord2f             )( GLfloat s, GLfloat t );
    void      (APIENTRY *glTexCoord2fv            )( const GLfloat *v );
    void      (APIENTRY *glTexCoord2i             )( GLint s, GLint t );
    void      (APIENTRY *glTexCoord2iv            )( const GLint *v );
    void      (APIENTRY *glTexCoord2s             )( GLshort s, GLshort t );
    void      (APIENTRY *glTexCoord2sv            )( const GLshort *v );
    void      (APIENTRY *glTexCoord3d             )( GLdouble s, GLdouble t, GLdouble r );
    void      (APIENTRY *glTexCoord3dv            )( const GLdouble *v );
    void      (APIENTRY *glTexCoord3f             )( GLfloat s, GLfloat t, GLfloat r );
    void      (APIENTRY *glTexCoord3fv            )( const GLfloat *v );
    void      (APIENTRY *glTexCoord3i             )( GLint s, GLint t, GLint r );
    void      (APIENTRY *glTexCoord3iv            )( const GLint *v );
    void      (APIENTRY *glTexCoord3s             )( GLshort s, GLshort t, GLshort r );
    void      (APIENTRY *glTexCoord3sv            )( const GLshort *v );
    void      (APIENTRY *glTexCoord4d             )( GLdouble s, GLdouble t, GLdouble r, GLdouble q );
    void      (APIENTRY *glTexCoord4dv            )( const GLdouble *v );
    void      (APIENTRY *glTexCoord4f             )( GLfloat s, GLfloat t, GLfloat r, GLfloat q );
    void      (APIENTRY *glTexCoord4fv            )( const GLfloat *v );
    void      (APIENTRY *glTexCoord4i             )( GLint s, GLint t, GLint r, GLint q );
    void      (APIENTRY *glTexCoord4iv            )( const GLint *v );
    void      (APIENTRY *glTexCoord4s             )( GLshort s, GLshort t, GLshort r, GLshort q );
    void      (APIENTRY *glTexCoord4sv            )( const GLshort *v );
    void      (APIENTRY *glVertex2d               )( GLdouble x, GLdouble y );
    void      (APIENTRY *glVertex2dv              )( const GLdouble *v );
    void      (APIENTRY *glVertex2f               )( GLfloat x, GLfloat y );
    void      (APIENTRY *glVertex2fv              )( const GLfloat *v );
    void      (APIENTRY *glVertex2i               )( GLint x, GLint y );
    void      (APIENTRY *glVertex2iv              )( const GLint *v );
    void      (APIENTRY *glVertex2s               )( GLshort x, GLshort y );
    void      (APIENTRY *glVertex2sv              )( const GLshort *v );
    void      (APIENTRY *glVertex3d               )( GLdouble x, GLdouble y, GLdouble z );
    void      (APIENTRY *glVertex3dv              )( const GLdouble *v );
    void      (APIENTRY *glVertex3f               )( GLfloat x, GLfloat y, GLfloat z );
    void      (APIENTRY *glVertex3fv              )( const GLfloat *v );
    void      (APIENTRY *glVertex3i               )( GLint x, GLint y, GLint z );
    void      (APIENTRY *glVertex3iv              )( const GLint *v );
    void      (APIENTRY *glVertex3s               )( GLshort x, GLshort y, GLshort z );
    void      (APIENTRY *glVertex3sv              )( const GLshort *v );
    void      (APIENTRY *glVertex4d               )( GLdouble x, GLdouble y, GLdouble z, GLdouble w );
    void      (APIENTRY *glVertex4dv              )( const GLdouble *v );
    void      (APIENTRY *glVertex4f               )( GLfloat x, GLfloat y, GLfloat z, GLfloat w );
    void      (APIENTRY *glVertex4fv              )( const GLfloat *v );
    void      (APIENTRY *glVertex4i               )( GLint x, GLint y, GLint z, GLint w );
    void      (APIENTRY *glVertex4iv              )( const GLint *v );
    void      (APIENTRY *glVertex4s               )( GLshort x, GLshort y, GLshort z, GLshort w );
    void      (APIENTRY *glVertex4sv              )( const GLshort *v );
    void      (APIENTRY *glClipPlane              )( GLenum plane, const GLdouble *equation );
    void      (APIENTRY *glColorMaterial          )( GLenum face, GLenum mode );
    void      (APIENTRY *glCullFace               )( GLenum mode );
    void      (APIENTRY *glFogf                   )( GLenum pname, GLfloat param );
    void      (APIENTRY *glFogfv                  )( GLenum pname, const GLfloat *params );
    void      (APIENTRY *glFogi                   )( GLenum pname, GLint param );
    void      (APIENTRY *glFogiv                  )( GLenum pname, const GLint *params );
    void      (APIENTRY *glFrontFace              )( GLenum mode );
    void      (APIENTRY *glHint                   )( GLenum target, GLenum mode );
    void      (APIENTRY *glLightf                 )( GLenum light, GLenum pname, GLfloat param );
    void      (APIENTRY *glLightfv                )( GLenum light, GLenum pname, const GLfloat *params );
    void      (APIENTRY *glLighti                 )( GLenum light, GLenum pname, GLint param );
    void      (APIENTRY *glLightiv                )( GLenum light, GLenum pname, const GLint *params );
    void      (APIENTRY *glLightModelf            )( GLenum pname, GLfloat param );
    void      (APIENTRY *glLightModelfv           )( GLenum pname, const GLfloat *params );
    void      (APIENTRY *glLightModeli            )( GLenum pname, GLint param );
    void      (APIENTRY *glLightModeliv           )( GLenum pname, const GLint *params );
    void      (APIENTRY *glLineStipple            )( GLint factor, GLushort pattern );
    void      (APIENTRY *glLineWidth              )( GLfloat width );
    void      (APIENTRY *glMaterialf              )( GLenum face, GLenum pname, GLfloat param );
    void      (APIENTRY *glMaterialfv             )( GLenum face, GLenum pname, const GLfloat *params );
    void      (APIENTRY *glMateriali              )( GLenum face, GLenum pname, GLint param );
    void      (APIENTRY *glMaterialiv             )( GLenum face, GLenum pname, const GLint *params );
    void      (APIENTRY *glPointSize              )( GLfloat size );
    void      (APIENTRY *glPolygonMode            )( GLenum face, GLenum mode );
    void      (APIENTRY *glPolygonStipple         )( const GLubyte *mask );
    void      (APIENTRY *glScissor                )( GLint x, GLint y, GLsizei width, GLsizei height );
    void      (APIENTRY *glShadeModel             )( GLenum mode );
    void      (APIENTRY *glTexParameterf          )( GLenum target, GLenum pname, GLfloat param );
    void      (APIENTRY *glTexParameterfv         )( GLenum target, GLenum pname, const GLfloat *params );
    void      (APIENTRY *glTexParameteri          )( GLenum target, GLenum pname, GLint param );
    void      (APIENTRY *glTexParameteriv         )( GLenum target, GLenum pname, const GLint *params );
    void      (APIENTRY *glTexImage1D             )( GLenum target, GLint level, GLint components, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid *pixels );
    void      (APIENTRY *glTexImage2D             )( GLenum target, GLint level, GLint components, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels );
    void      (APIENTRY *glTexEnvf                )( GLenum target, GLenum pname, GLfloat param );
    void      (APIENTRY *glTexEnvfv               )( GLenum target, GLenum pname, const GLfloat *params );
    void      (APIENTRY *glTexEnvi                )( GLenum target, GLenum pname, GLint param );
    void      (APIENTRY *glTexEnviv               )( GLenum target, GLenum pname, const GLint *params );
    void      (APIENTRY *glTexGend                )( GLenum coord, GLenum pname, GLdouble param );
    void      (APIENTRY *glTexGendv               )( GLenum coord, GLenum pname, const GLdouble *params );
    void      (APIENTRY *glTexGenf                )( GLenum coord, GLenum pname, GLfloat param );
    void      (APIENTRY *glTexGenfv               )( GLenum coord, GLenum pname, const GLfloat *params );
    void      (APIENTRY *glTexGeni                )( GLenum coord, GLenum pname, GLint param );
    void      (APIENTRY *glTexGeniv               )( GLenum coord, GLenum pname, const GLint *params );
    void      (APIENTRY *glFeedbackBuffer         )( GLsizei size, GLenum type, GLfloat *buffer );
    void      (APIENTRY *glSelectBuffer           )( GLsizei size, GLuint *buffer );
    GLint     (APIENTRY *glRenderMode             )( GLenum mode );
    void      (APIENTRY *glInitNames              )( void );
    void      (APIENTRY *glLoadName               )( GLuint name );
    void      (APIENTRY *glPassThrough            )( GLfloat token );
    void      (APIENTRY *glPopName                )( void );
    void      (APIENTRY *glPushName               )( GLuint name );
    void      (APIENTRY *glDrawBuffer             )( GLenum mode );
    void      (APIENTRY *glClear                  )( GLbitfield mask );
    void      (APIENTRY *glClearAccum             )( GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha );
    void      (APIENTRY *glClearIndex             )( GLfloat c );
    void      (APIENTRY *glClearColor             )( GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha );
    void      (APIENTRY *glClearStencil           )( GLint s );
    void      (APIENTRY *glClearDepth             )( GLclampd depth );
    void      (APIENTRY *glStencilMask            )( GLuint mask );
    void      (APIENTRY *glColorMask              )( GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha );
    void      (APIENTRY *glDepthMask              )( GLboolean flag );
    void      (APIENTRY *glIndexMask              )( GLuint mask );
    void      (APIENTRY *glAccum                  )( GLenum op, GLfloat value );
    void      (APIENTRY *glDisable                )( GLenum cap );
    void      (APIENTRY *glEnable                 )( GLenum cap );
    void      (APIENTRY *glFinish                 )( void );
    void      (APIENTRY *glFlush                  )( void );
    void      (APIENTRY *glPopAttrib              )( void );
    void      (APIENTRY *glPushAttrib             )( GLbitfield mask );
    void      (APIENTRY *glMap1d                  )( GLenum target, GLdouble u1, GLdouble u2, GLint stride, GLint order, const GLdouble *points );
    void      (APIENTRY *glMap1f                  )( GLenum target, GLfloat u1, GLfloat u2, GLint stride, GLint order, const GLfloat *points );
    void      (APIENTRY *glMap2d                  )( GLenum target, GLdouble u1, GLdouble u2, GLint ustride, GLint uorder, GLdouble v1, GLdouble v2, GLint vstride, GLint vorder, const GLdouble *points );
    void      (APIENTRY *glMap2f                  )( GLenum target, GLfloat u1, GLfloat u2, GLint ustride, GLint uorder, GLfloat v1, GLfloat v2, GLint vstride, GLint vorder, const GLfloat *points );
    void      (APIENTRY *glMapGrid1d              )( GLint un, GLdouble u1, GLdouble u2 );
    void      (APIENTRY *glMapGrid1f              )( GLint un, GLfloat u1, GLfloat u2 );
    void      (APIENTRY *glMapGrid2d              )( GLint un, GLdouble u1, GLdouble u2, GLint vn, GLdouble v1, GLdouble v2 );
    void      (APIENTRY *glMapGrid2f              )( GLint un, GLfloat u1, GLfloat u2, GLint vn, GLfloat v1, GLfloat v2 );
    void      (APIENTRY *glEvalCoord1d            )( GLdouble u );
    void      (APIENTRY *glEvalCoord1dv           )( const GLdouble *u );
    void      (APIENTRY *glEvalCoord1f            )( GLfloat u );
    void      (APIENTRY *glEvalCoord1fv           )( const GLfloat *u );
    void      (APIENTRY *glEvalCoord2d            )( GLdouble u, GLdouble v );
    void      (APIENTRY *glEvalCoord2dv           )( const GLdouble *u );
    void      (APIENTRY *glEvalCoord2f            )( GLfloat u, GLfloat v );
    void      (APIENTRY *glEvalCoord2fv           )( const GLfloat *u );
    void      (APIENTRY *glEvalMesh1              )( GLenum mode, GLint i1, GLint i2 );
    void      (APIENTRY *glEvalPoint1             )( GLint i );
    void      (APIENTRY *glEvalMesh2              )( GLenum mode, GLint i1, GLint i2, GLint j1, GLint j2 );
    void      (APIENTRY *glEvalPoint2             )( GLint i, GLint j );
    void      (APIENTRY *glAlphaFunc              )( GLenum func, GLclampf ref );
    void      (APIENTRY *glBlendFunc              )( GLenum sfactor, GLenum dfactor );
    void      (APIENTRY *glLogicOp                )( GLenum opcode );
    void      (APIENTRY *glStencilFunc            )( GLenum func, GLint ref, GLuint mask );
    void      (APIENTRY *glStencilOp              )( GLenum fail, GLenum zfail, GLenum zpass );
    void      (APIENTRY *glDepthFunc              )( GLenum func );
    void      (APIENTRY *glPixelZoom              )( GLfloat xfactor, GLfloat yfactor );
    void      (APIENTRY *glPixelTransferf         )( GLenum pname, GLfloat param );
    void      (APIENTRY *glPixelTransferi         )( GLenum pname, GLint param );
    void      (APIENTRY *glPixelStoref            )( GLenum pname, GLfloat param );
    void      (APIENTRY *glPixelStorei            )( GLenum pname, GLint param );
    void      (APIENTRY *glPixelMapfv             )( GLenum map, GLint mapsize, const GLfloat *values );
    void      (APIENTRY *glPixelMapuiv            )( GLenum map, GLint mapsize, const GLuint *values );
    void      (APIENTRY *glPixelMapusv            )( GLenum map, GLint mapsize, const GLushort *values );
    void      (APIENTRY *glReadBuffer             )( GLenum mode );
    void      (APIENTRY *glCopyPixels             )( GLint x, GLint y, GLsizei width, GLsizei height, GLenum type );
    void      (APIENTRY *glReadPixels             )( GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels );
    void      (APIENTRY *glDrawPixels             )( GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels );
    void      (APIENTRY *glGetBooleanv            )( GLenum pname, GLboolean *params );
    void      (APIENTRY *glGetClipPlane           )( GLenum plane, GLdouble *equation );
    void      (APIENTRY *glGetDoublev             )( GLenum pname, GLdouble *params );
    GLenum    (APIENTRY *glGetError               )( void );
    void      (APIENTRY *glGetFloatv              )( GLenum pname, GLfloat *params );
    void      (APIENTRY *glGetIntegerv            )( GLenum pname, GLint *params );
    void      (APIENTRY *glGetLightfv             )( GLenum light, GLenum pname, GLfloat *params );
    void      (APIENTRY *glGetLightiv             )( GLenum light, GLenum pname, GLint *params );
    void      (APIENTRY *glGetMapdv               )( GLenum target, GLenum query, GLdouble *v );
    void      (APIENTRY *glGetMapfv               )( GLenum target, GLenum query, GLfloat *v );
    void      (APIENTRY *glGetMapiv               )( GLenum target, GLenum query, GLint *v );
    void      (APIENTRY *glGetMaterialfv          )( GLenum face, GLenum pname, GLfloat *params );
    void      (APIENTRY *glGetMaterialiv          )( GLenum face, GLenum pname, GLint *params );
    void      (APIENTRY *glGetPixelMapfv          )( GLenum map, GLfloat *values );
    void      (APIENTRY *glGetPixelMapuiv         )( GLenum map, GLuint *values );
    void      (APIENTRY *glGetPixelMapusv         )( GLenum map, GLushort *values );
    void      (APIENTRY *glGetPolygonStipple      )( GLubyte *mask );
    const GLubyte * (APIENTRY *glGetString        )( GLenum name );
    void      (APIENTRY *glGetTexEnvfv            )( GLenum target, GLenum pname, GLfloat *params );
    void      (APIENTRY *glGetTexEnviv            )( GLenum target, GLenum pname, GLint *params );
    void      (APIENTRY *glGetTexGendv            )( GLenum coord, GLenum pname, GLdouble *params );
    void      (APIENTRY *glGetTexGenfv            )( GLenum coord, GLenum pname, GLfloat *params );
    void      (APIENTRY *glGetTexGeniv            )( GLenum coord, GLenum pname, GLint *params );
    void      (APIENTRY *glGetTexImage            )( GLenum target, GLint level, GLenum format, GLenum type, GLvoid *pixels );
    void      (APIENTRY *glGetTexParameterfv      )( GLenum target, GLenum pname, GLfloat *params );
    void      (APIENTRY *glGetTexParameteriv      )( GLenum target, GLenum pname, GLint *params );
    void      (APIENTRY *glGetTexLevelParameterfv )( GLenum target, GLint level, GLenum pname, GLfloat *params );
    void      (APIENTRY *glGetTexLevelParameteriv )( GLenum target, GLint level, GLenum pname, GLint *params );
    GLboolean (APIENTRY *glIsEnabled              )( GLenum cap );
    GLboolean (APIENTRY *glIsList                 )( GLuint list );
    void      (APIENTRY *glDepthRange             )( GLclampd zNear, GLclampd zFar );
    void      (APIENTRY *glFrustum                )( GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar );
    void      (APIENTRY *glLoadIdentity           )( void );
    void      (APIENTRY *glLoadMatrixf            )( const GLfloat *m );
    void      (APIENTRY *glLoadMatrixd            )( const GLdouble *m );
    void      (APIENTRY *glMatrixMode             )( GLenum mode );
    void      (APIENTRY *glMultMatrixf            )( const GLfloat *m );
    void      (APIENTRY *glMultMatrixd            )( const GLdouble *m );
    void      (APIENTRY *glOrtho                  )( GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar );
    void      (APIENTRY *glPopMatrix              )( void );
    void      (APIENTRY *glPushMatrix             )( void );
    void      (APIENTRY *glRotated                )( GLdouble angle, GLdouble x, GLdouble y, GLdouble z );
    void      (APIENTRY *glRotatef                )( GLfloat angle, GLfloat x, GLfloat y, GLfloat z );
    void      (APIENTRY *glScaled                 )( GLdouble x, GLdouble y, GLdouble z );
    void      (APIENTRY *glScalef                 )( GLfloat x, GLfloat y, GLfloat z );
    void      (APIENTRY *glTranslated             )( GLdouble x, GLdouble y, GLdouble z );
    void      (APIENTRY *glTranslatef             )( GLfloat x, GLfloat y, GLfloat z );
    void      (APIENTRY *glViewport               )( GLint x, GLint y, GLsizei width, GLsizei height );
    // OpenGL version 1.0 entries end here

    // OpenGL version 1.1 entries begin here
    void      (APIENTRY *glArrayElement           )(GLint i);
    void      (APIENTRY *glBindTexture            )(GLenum target, GLuint texture);
    void      (APIENTRY *glColorPointer           )(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
    void      (APIENTRY *glDisableClientState     )(GLenum array);
    void      (APIENTRY *glDrawArrays             )(GLenum mode, GLint first, GLsizei count);
    void      (APIENTRY *glDrawElements           )(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices);
    void      (APIENTRY *glEdgeFlagPointer        )(GLsizei stride, const GLvoid *pointer);
    void      (APIENTRY *glEnableClientState      )(GLenum array);
    void      (APIENTRY *glIndexPointer           )(GLenum type, GLsizei stride, const GLvoid *pointer);
    void      (APIENTRY *glIndexub                )(GLubyte c);
    void      (APIENTRY *glIndexubv               )(const GLubyte *c);
    void      (APIENTRY *glInterleavedArrays      )(GLenum format, GLsizei stride, const GLvoid *pointer);
    void      (APIENTRY *glNormalPointer          )(GLenum type, GLsizei stride, const GLvoid *pointer);
    void      (APIENTRY *glPolygonOffset          )(GLfloat factor, GLfloat units);
    void      (APIENTRY *glTexCoordPointer        )(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
    void      (APIENTRY *glVertexPointer          )(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
    GLboolean (APIENTRY *glAreTexturesResident    )(GLsizei n, const GLuint *textures, GLboolean *residences);
    void      (APIENTRY *glCopyTexImage1D         )(GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y, GLsizei width, GLint border);
    void      (APIENTRY *glCopyTexImage2D         )(GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
    void      (APIENTRY *glCopyTexSubImage1D      )(GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width);
    void      (APIENTRY *glCopyTexSubImage2D      )(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
    void      (APIENTRY *glDeleteTextures         )(GLsizei n, const GLuint *textures);
    void      (APIENTRY *glGenTextures            )(GLsizei n, GLuint *textures);
    void      (APIENTRY *glGetPointerv            )(GLenum pname, GLvoid* *params);
    GLboolean (APIENTRY *glIsTexture              )(GLuint texture);
    void      (APIENTRY *glPrioritizeTextures     )(GLsizei n, const GLuint *textures, const GLclampf *priorities);
    void      (APIENTRY *glTexSubImage1D          )(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid *pixels);
    void      (APIENTRY *glTexSubImage2D          )(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels);
    void      (APIENTRY *glPopClientAttrib        )(void);
    void      (APIENTRY *glPushClientAttrib       )(GLbitfield mask);
} GLDISPATCHTABLE, *PGLDISPATCHTABLE;

// OpenGL Client/Driver Procedure Table.

typedef struct _GLCLTPROCTABLE {
    _In_range_(0,OPENGL_VERSION_110_ENTRIES) int             cEntries;           // Number of function entries in table
    GLDISPATCHTABLE glDispatchTable;    // OpenGL function dispatch table
} GLCLTPROCTABLE, *PGLCLTPROCTABLE;

// Driver GLRC handle.

typedef ULONG DHGLRC;

// SetProcTable function prototype for DrvSetContext.

typedef VOID (APIENTRY *PFN_SETPROCTABLE)(PGLCLTPROCTABLE);

// DrvSetCallbackProcs prototypes
typedef VOID   (APIENTRY *PFN_SETCURRENTVALUE)(VOID *pv);
typedef VOID  *(APIENTRY *PFN_GETCURRENTVALUE)(VOID);
typedef DHGLRC (APIENTRY *PFN_GETDHGLRC)(HGLRC hrc);
#if WINVER >= 0x600
typedef struct _PRESENTBUFFERSCB {
    IN UINT nVersion;
    IN UINT syncType;      // See PRESCB_SYNCTYPE_NONE and PRESCB_SYNCTYPE_VSYNC
    IN LUID luidAdapter;
    IN LPVOID pPrivData;
    IN RECT updateRect;    // Update rectangle in the coordinate system of the window, whose HDC is passed to PFN_PRESENTBUFFERS
} PRESENTBUFFERSCB, *LPPRESENTBUFFERSCB;
typedef BOOL (APIENTRY *PFN_PRESENTBUFFERS)(HDC hdc, LPPRESENTBUFFERSCB pprsbcbData);
#endif
#if WINVER >= 0xA00
typedef VOID   (APIENTRY *PFN_GETADAPTERLUID)(HDC hdc, OUT LUID* pLuid);
typedef struct _CHECKFULLSCREENSUPPORTCB {
    IN UINT nVersion;
    IN LUID luidAdapter;
    IN UINT hDevice;
    IN HMONITOR hMonitor;
    OUT UINT VidPnSourceId;
} CHECKFULLSCREENSUPPORTCB, *LPCHECKFULLSCREENSUPPORTCB;
typedef BOOL   (APIENTRY *PFN_CHECKFULLSCREENSUPPORT)(HDC hdc, LPCHECKFULLSCREENSUPPORTCB pArgs);
typedef struct _PRESENTTOREDIRECTIONSURFACECB {
    IN UINT nVersion;
    IN UINT hContext;
    IN UINT hSource;
    IN UINT hDestination;
    IN HANDLE hSharedHandle;
    IN UINT64 updateId;
    IN RECT updateRect;
    IN UINT broadcastContextCount;
    IN UINT broadcastContext[64];
    IN UINT broadcastSrcAllocation[64];
    IN UINT broadcastDstAllocation[64];
    IN UINT cbPrivateDriverDataSize;
    IN VOID *pPrivateDriverData;
} PRESENTTOREDIRECTIONSURFACECB, *LPPRESENTTOREDIRECTIONSURFACECB;
typedef BOOL   (APIENTRY *PFN_PRESENTTOREDIRECTIONSURFACE)(HDC hdc, LPPRESENTTOREDIRECTIONSURFACECB pArgs);
typedef struct _SUBMITPRESENTTOREDIRECTIONSURFACECB {
    IN UINT nVersion;
    IN HANDLE hSharedHandle;
    IN UINT64 updateId;
    IN RECT updateRect;
    IN UINT broadcastHwQueueCount;

    _Field_size_(broadcastHwQueueCount)
    IN UINT *broadcastHwQueue;

    _Field_size_(broadcastHwQueueCount)
    IN UINT *broadcastSrcAllocation;

    _Field_size_opt_(broadcastHwQueueCount)
    IN UINT *broadcastDstAllocation;

    IN UINT cbPrivateDriverDataSize;
    IN VOID *pPrivateDriverData;
} SUBMITPRESENTTOREDIRECTIONSURFACECB, *LPSUBMITPRESENTTOREDIRECTIONSURFACECB;
typedef BOOL   (APIENTRY *PFN_SUBMITPRESENTTOREDIRECTIONSURFACE)(HDC hdc, LPSUBMITPRESENTTOREDIRECTIONSURFACECB pArgs);
#endif

// Note: Structure not referenced directly, simply present to document the expected order/count of
// callbacks received through DrvSetCallbackProcs.
struct WGLCALLBACKS
{
    PFN_SETCURRENTVALUE pfnSetCurrentValue;
    PFN_GETCURRENTVALUE pfnGetCurrentValue;
    PFN_GETDHGLRC pfnGetDhglrc;
    PROC pfnUnused;
#if WINVER >= 0x600
    PFN_PRESENTBUFFERS pfnPresentBuffers;
#endif
#if WINVER >= 0xA00
    PFN_GETADAPTERLUID pfnGetAdapterLuid;
    PFN_CHECKFULLSCREENSUPPORT pfnCheckFullscreenSupport;
    PFN_PRESENTTOREDIRECTIONSURFACE pfnPresentToRedirectionSurface;
    PFN_SUBMITPRESENTTOREDIRECTIONSURFACE pfnSubmitPresentToRedirectionSurface;
#endif
};

// Driver context function prototypes.

BOOL            APIENTRY DrvCopyContext(DHGLRC, DHGLRC, UINT);
DHGLRC          APIENTRY DrvCreateContext(HDC);
DHGLRC          APIENTRY DrvCreateLayerContext(HDC, int);
BOOL            APIENTRY DrvDeleteContext(DHGLRC);
PGLCLTPROCTABLE APIENTRY DrvSetContext(HDC,DHGLRC,PFN_SETPROCTABLE);
BOOL            APIENTRY DrvReleaseContext(DHGLRC);
BOOL            APIENTRY DrvValidateVersion(ULONG);
BOOL		APIENTRY DrvShareLists(DHGLRC, DHGLRC);
PROC            APIENTRY DrvGetProcAddress(LPCSTR);
VOID            APIENTRY DrvSetCallbackProcs(INT, PROC *); // See WGLCALLBACKS for expected order/count per OS.
BOOL            APIENTRY DrvDescribeLayerPlane(HDC, INT, INT, UINT,
                                               LPLAYERPLANEDESCRIPTOR);
INT             APIENTRY DrvSetLayerPaletteEntries(HDC, INT, INT, INT,
                                                   CONST COLORREF *);
INT             APIENTRY DrvGetLayerPaletteEntries(HDC, INT, INT, INT,
                                                   COLORREF *);
BOOL            APIENTRY DrvRealizeLayerPalette(HDC, INT, BOOL);
BOOL            APIENTRY DrvSwapLayerBuffers(HDC, UINT);

#if WINVER >= 0x500

typedef struct IDirectDrawSurface *LPDIRECTDRAWSURFACE;
typedef struct _DDSURFACEDESC *LPDDSURFACEDESC;

DHGLRC          APIENTRY DrvCreateDirectDrawContext(HDC, LPDIRECTDRAWSURFACE,
                                                    int);
int             APIENTRY DrvEnumTextureFormats(int, LPDDSURFACEDESC);
BOOL            APIENTRY DrvBindDirectDrawTexture(LPDIRECTDRAWSURFACE);
DWORD           APIENTRY DrvSwapMultipleBuffers(UINT cBuffers,
                                                CONST WGLSWAP *pgswap);

LONG            APIENTRY DrvDescribePixelFormat(HDC, INT, ULONG, PIXELFORMATDESCRIPTOR*);
BOOL            APIENTRY DrvSetPixelFormat(HDC, LONG);
BOOL            APIENTRY DrvSwapBuffers(HDC);

#endif // WINVER >= 0x500

#if WINVER >= 0x600
typedef struct _PRESENTBUFFERS {
    IN HANDLE hSurface;
    IN LUID luidAdapter;
    IN ULONGLONG ullPresentToken;
    IN LPVOID pPrivData;
} PRESENTBUFFERS, *LPPRESENTBUFFERS;
typedef BOOL (APIENTRY *PFN_PRESENTBUFFERS)(HDC hdc, LPPRESENTBUFFERSCB pprsbcbData);

#define PRESCB_SYNCTYPE_NONE 0
#define PRESCB_SYNCTYPE_VSYNC  1

BOOL            APIENTRY DrvPresentBuffers(HDC hdc, LPPRESENTBUFFERS pprsbData);

#endif

// Input structure for OPENGL_CMD ExtEscape.

typedef struct _WNDOBJ WNDOBJ;
typedef struct _XLATEOBJ XLATEOBJ;

typedef struct _OPENGLCMD
{
    ULONG    ulSubEsc;
    FLONG    fl;
    WNDOBJ   *pwo;
    XLATEOBJ *pxo;
} OPENGLCMD, *POPENGLCMD;

#if WINVER >= 0x500

#define OPENGLCMD_MAXMULTI WGL_SWAPMULTIPLE_MAX

typedef struct _OPENGLCMDMULTI
{
    ULONG ulSubEsc;
    FLONG fl;
    ULONG cMulti;
    XLATEOBJ *pxo;
} OPENGLCMDMULTI, *POPENGLCMDMULTI;

#endif // WINVER >= 0x500

// Flags for OPENGL_CMD ExtEscape.

#define OGLCMD_NEEDWNDOBJ       0x01
#define OGLCMD_NEEDXLATEOBJ     0x02

#if WINVER >= 0x500
#define OGLCMD_MULTIWNDOBJ      0x04
#endif // WINVER >= 0x500

// OPENGL_GETINFO ExtEscape sub-escape numbers.  They are defined by Microsoft.

#define OPENGL_GETINFO_DRVNAME  0

// Input structure for OPENGL_GETINFO ExtEscape.

typedef struct _OPENGLGETINFO
{
    ULONG   ulSubEsc;
} OPENGLGETINFO, *POPENGLGETINFO;

// Input structure for OPENGL_GETINFO_DRVNAME ExtEscape.

typedef struct _GLDRVNAME
{
    OPENGLGETINFO   oglget;
} GLDRVNAME, *PGLDRVNAME;

// Output structure for OPENGL_GETINFO_DRVNAME ExtEscape.

typedef struct _GLDRVNAMERET
{
    ULONG   ulVersion;              // must be 1 for this version
    ULONG   ulDriverVersion;        // driver specific version number
    WCHAR   awch[MAX_PATH+1];
} GLDRVNAMERET, *PGLDRVNAMERET;

#endif /* _GLDRV_ */
