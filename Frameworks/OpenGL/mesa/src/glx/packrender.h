#ifndef __GLX_packrender_h__
#define __GLX_packrender_h__

/*
 * SGI FREE SOFTWARE LICENSE B (Version 2.0, Sept. 18, 2008)
 * Copyright (C) 1991-2000 Silicon Graphics, Inc. All Rights Reserved.
 *
 * SPDX-License-Identifier: SGI-B-2.0
 */

#include "glxclient.h"

/*
** The macros in this header convert the client machine's native data types to
** wire protocol data types.  The header is part of the porting layer of the
** client library, and it is intended that hardware vendors will rewrite this
** header to suit their own machines.
*/

/*
** Pad a count of bytes to the nearest multiple of 4.  The X protocol
** transfers data in 4 byte quantities, so this macro is used to
** insure the right amount of data being sent.
*/
#define __GLX_PAD(a) (((a)+3) & ~3)

/*
** Network size parameters
*/
#define sz_double 8

/* Setup for all commands */
#define __GLX_DECLARE_VARIABLES()               \
   struct glx_context *gc;                            \
   GLubyte *pc, *pixelHeaderPC;                 \
   GLuint compsize, cmdlen

#define __GLX_LOAD_VARIABLES()     \
   gc = __glXGetCurrentContext();  \
   pc = gc->pc;                    \
   /* Muffle compilers */                  \
   cmdlen = 0;           (void)cmdlen;          \
   compsize = 0;         (void)compsize;        \
   pixelHeaderPC = NULL; (void)pixelHeaderPC

/*
** Variable sized command support macro.  This macro is used by calls
** that are potentially larger than __GLX_SMALL_RENDER_CMD_SIZE.
** Because of their size, they may not automatically fit in the buffer.
** If the buffer can't hold the command then it is flushed so that
** the command will fit in the next buffer.
*/
#define __GLX_BEGIN_VARIABLE(opcode,size)       \
   if (pc + (size) > gc->bufEnd) {              \
      pc = __glXFlushRenderBuffer(gc, pc);      \
   }                                            \
   __GLX_PUT_SHORT(0,size);                     \
   __GLX_PUT_SHORT(2,opcode)

#define __GLX_BEGIN_VARIABLE_LARGE(opcode,size) \
   pc = __glXFlushRenderBuffer(gc, pc);         \
   __GLX_PUT_LONG(0,size);                      \
   __GLX_PUT_LONG(4,opcode)

#define __GLX_BEGIN_VARIABLE_WITH_PIXEL(opcode,size)  \
   if (pc + (size) > gc->bufEnd) {                    \
      pc = __glXFlushRenderBuffer(gc, pc);            \
   }                                                  \
   __GLX_PUT_SHORT(0,size);                           \
   __GLX_PUT_SHORT(2,opcode);                         \
   pc += __GLX_RENDER_HDR_SIZE;                       \
   pixelHeaderPC = pc;                                \
   pc += __GLX_PIXEL_HDR_SIZE

#define __GLX_BEGIN_VARIABLE_LARGE_WITH_PIXEL(opcode,size)  \
   pc = __glXFlushRenderBuffer(gc, pc);                     \
   __GLX_PUT_LONG(0,size);                                  \
   __GLX_PUT_LONG(4,opcode);                                \
   pc += __GLX_RENDER_LARGE_HDR_SIZE;                       \
   pixelHeaderPC = pc;                                      \
   pc += __GLX_PIXEL_HDR_SIZE

#define __GLX_BEGIN_VARIABLE_WITH_PIXEL_3D(opcode,size)  \
   if (pc + (size) > gc->bufEnd) {                       \
      pc = __glXFlushRenderBuffer(gc, pc);               \
   }                                                     \
   __GLX_PUT_SHORT(0,size);                              \
   __GLX_PUT_SHORT(2,opcode);                            \
   pc += __GLX_RENDER_HDR_SIZE;                          \
   pixelHeaderPC = pc;                                   \
   pc += __GLX_PIXEL_3D_HDR_SIZE

#define __GLX_BEGIN_VARIABLE_LARGE_WITH_PIXEL_3D(opcode,size)  \
   pc = __glXFlushRenderBuffer(gc, pc);                        \
   __GLX_PUT_LONG(0,size);                                     \
   __GLX_PUT_LONG(4,opcode);                                   \
   pc += __GLX_RENDER_LARGE_HDR_SIZE;                          \
   pixelHeaderPC = pc;                                         \
   pc += __GLX_PIXEL_3D_HDR_SIZE

/*
** Fixed size command support macro.  This macro is used by calls that
** are never larger than __GLX_SMALL_RENDER_CMD_SIZE.  Because they
** always fit in the buffer, and because the buffer promises to
** maintain enough room for them, we don't need to check for space
** before doing the storage work.
*/
#define __GLX_BEGIN(opcode,size) \
   __GLX_PUT_SHORT(0,size);      \
   __GLX_PUT_SHORT(2,opcode)

/*
** Finish a rendering command by advancing the pc.  If the pc is now past
** the limit pointer then there is no longer room for a
** __GLX_SMALL_RENDER_CMD_SIZE sized command, which will break the
** assumptions present in the __GLX_BEGIN macro.  In this case the
** rendering buffer is flushed out into the X protocol stream (which may
** or may not do I/O).
*/
#define __GLX_END(size)           \
   pc += size;                       \
   if (pc > gc->limit) {                  \
      (void) __glXFlushRenderBuffer(gc, pc);    \
   } else {                                     \
      gc->pc = pc;                              \
   }

/* Array copy macros */
#define __GLX_MEM_COPY(dest,src,bytes)          \
   if (src && dest)                             \
      memcpy(dest, src, bytes)

/* Single item copy macros */
#define __GLX_PUT_CHAR(offset,a)                \
   do {                                         \
      int8_t __tmp = (a);                       \
      memcpy((pc + (offset)), &__tmp, 1);       \
   } while (0)

#define __GLX_PUT_SHORT(offset,a)               \
   do {                                         \
      int16_t __tmp = (a);                      \
      memcpy((pc + (offset)), &__tmp, 2);       \
   } while (0)

#define __GLX_PUT_LONG(offset,a)                \
   do {                                         \
      int32_t __tmp = (a);                      \
      memcpy((pc + (offset)), &__tmp, 4);       \
   } while (0)

#define __GLX_PUT_FLOAT(offset,a)               \
   do {                                         \
      float __tmp = (a);                        \
      memcpy((pc + (offset)), &__tmp, 4);       \
   } while (0)

#define __GLX_PUT_DOUBLE(offset,a)              \
   do {                                         \
      double __tmp = (a);                       \
      memcpy((pc + (offset)), &__tmp, 8);       \
   } while (0)

#define __GLX_PUT_CHAR_ARRAY(offset,a,alen)                 \
   __GLX_MEM_COPY(pc + offset, a, alen * __GLX_SIZE_INT8)

#define __GLX_PUT_SHORT_ARRAY(offset,a,alen)                \
   __GLX_MEM_COPY(pc + offset, a, alen * __GLX_SIZE_INT16)

#define __GLX_PUT_LONG_ARRAY(offset,a,alen)                 \
   __GLX_MEM_COPY(pc + offset, a, alen * __GLX_SIZE_INT32)

#define __GLX_PUT_FLOAT_ARRAY(offset,a,alen)                   \
   __GLX_MEM_COPY(pc + offset, a, alen * __GLX_SIZE_FLOAT32)

#define __GLX_PUT_DOUBLE_ARRAY(offset,a,alen)                  \
   __GLX_MEM_COPY(pc + offset, a, alen * __GLX_SIZE_FLOAT64)


#endif /* !__GLX_packrender_h__ */
