
#ifndef __NOUVEAU_DEBUG_H__
#define __NOUVEAU_DEBUG_H__

#include <stdio.h>

#include "util/u_debug.h"

#define NOUVEAU_DEBUG_MISC       0x0001
#define NOUVEAU_DEBUG_SHADER     0x0100
#define NOUVEAU_DEBUG_PROG_IR    0x0200
#define NOUVEAU_DEBUG_PROG_RA    0x0400
#define NOUVEAU_DEBUG_PROG_CFLOW 0x0800
#define NOUVEAU_DEBUG_PROG_ALL   0x1f00

#define NOUVEAU_DEBUG 0

#define NOUVEAU_ERR(fmt, args...)                                 \
   fprintf(stderr, "%s:%d - " fmt, __func__, __LINE__, ##args)

#define NOUVEAU_DBG(ch, args...)           \
   if ((NOUVEAU_DEBUG) & (NOUVEAU_DEBUG_##ch))        \
      debug_printf(args)

#endif /* __NOUVEAU_DEBUG_H__ */
