#ifndef __GLX_packsingle_h__
#define __GLX_packsingle_h__

/*
 * SGI FREE SOFTWARE LICENSE B (Version 2.0, Sept. 18, 2008)
 * Copyright (C) 1991-2000 Silicon Graphics, Inc. All Rights Reserved.
 *
 * SPDX-License-Identifier: SGI-B-2.0
 */

#include "packrender.h"

/*
** The macros in this header convert wire protocol data types to the client
** machine's native data types.  The header is part of the porting layer of
** the client library, and it is intended that hardware vendors will rewrite
** this header to suit their own machines.
*/

/*
** Dummy define to make the GetReqExtra macro happy.  The value is not
** used, but instead the code in __GLX_SINGLE_BEGIN issues its own store
** to req->reqType with the proper code (our extension code).
*/
#define X_GLXSingle 0

/* Declare common variables used during a single command */
#define __GLX_SINGLE_DECLARE_VARIABLES()         \
   struct glx_context *gc = __glXGetCurrentContext();  \
   GLubyte *pc, *pixelHeaderPC;                  \
   GLuint compsize, cmdlen;                      \
   Display *dpy = gc->currentDpy;                \
   xGLXSingleReq *req

#define __GLX_SINGLE_LOAD_VARIABLES()           \
   pc = gc->pc;                                 \
   /* Muffle compilers */                       \
   pixelHeaderPC = NULL;  (void)pixelHeaderPC;     \
   compsize = 0;       (void)compsize;          \
   cmdlen = 0;         (void)cmdlen

/* Start a single command */
#define __GLX_SINGLE_BEGIN(opcode,bytes)        \
   if (dpy) {                                   \
   (void) __glXFlushRenderBuffer(gc, pc);       \
   LockDisplay(dpy);                            \
   GetReqExtra(GLXSingle,bytes,req);            \
   req->reqType = gc->majorOpcode;              \
   req->glxCode = opcode;                       \
   req->contextTag = gc->currentContextTag;     \
   pc = ((GLubyte *)(req) + sz_xGLXSingleReq)

/* End a single command */
#define __GLX_SINGLE_END()       \
   UnlockDisplay(dpy);           \
   SyncHandle();                 \
   }

/* Store data to sending for a single command */
#define __GLX_SINGLE_PUT_CHAR(offset,a)         \
   *((INT8 *) (pc + offset)) = a

#define __GLX_SINGLE_PUT_SHORT(offset,a)        \
   *((INT16 *) (pc + offset)) = a

#define __GLX_SINGLE_PUT_LONG(offset,a)         \
   *((INT32 *) (pc + offset)) = a

#define __GLX_SINGLE_PUT_FLOAT(offset,a)        \
   *((FLOAT32 *) (pc + offset)) = a

/* Read support macros */
#define __GLX_SINGLE_READ_XREPLY()                    \
   (void) _XReply(dpy, (xReply*) &reply, 0, False)

#define __GLX_SINGLE_GET_RETVAL(a,cast)         \
   a = (cast) reply.retval

#define __GLX_SINGLE_GET_SIZE(a)                \
   a = (GLint) reply.size

#define __GLX_SINGLE_GET_CHAR(p)                \
   memcpy((p), &reply.pad3, 1);

#define __GLX_SINGLE_GET_SHORT(p)               \
   memcpy((p), &reply.pad3, 2);

#define __GLX_SINGLE_GET_LONG(p)                \
   memcpy((p), &reply.pad3, 4);

#define __GLX_SINGLE_GET_FLOAT(p)               \
   memcpy((p), &reply.pad3, 4);

#define __GLX_SINGLE_GET_DOUBLE(p)              \
   memcpy((p), &reply.pad3, 8);

/* Get an array of typed data */
#define __GLX_SINGLE_GET_VOID_ARRAY(a,alen)     \
   {                                            \
      GLint slop = alen*__GLX_SIZE_INT8 & 3;    \
      _XRead(dpy,(char *)a,alen*__GLX_SIZE_INT8);  \
      if (slop) _XEatData(dpy,4-slop);             \
   }

#define __GLX_SINGLE_GET_CHAR_ARRAY(a,alen)     \
   {                                            \
      GLint slop = alen*__GLX_SIZE_INT8 & 3;    \
      _XRead(dpy,(char *)a,alen*__GLX_SIZE_INT8);  \
      if (slop) _XEatData(dpy,4-slop);             \
   }


#define __GLX_SINGLE_GET_SHORT_ARRAY(a,alen)    \
   {                                            \
      GLint slop = (alen*__GLX_SIZE_INT16) & 3;    \
      _XRead(dpy,(char *)a,alen*__GLX_SIZE_INT16); \
      if (slop) _XEatData(dpy,4-slop);             \
   }

#define __GLX_SINGLE_GET_LONG_ARRAY(a,alen)        \
   _XRead(dpy,(char *)a,alen*__GLX_SIZE_INT32);

#define __GLX_SINGLE_GET_FLOAT_ARRAY(a,alen)       \
   _XRead(dpy,(char *)a,alen*__GLX_SIZE_FLOAT32);

#define __GLX_SINGLE_GET_DOUBLE_ARRAY(a,alen)      \
   _XRead(dpy,(char *)a,alen*__GLX_SIZE_FLOAT64);

#endif /* !__GLX_packsingle_h__ */
