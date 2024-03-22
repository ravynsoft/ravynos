#ifndef __NVHW_QMD_H__
#define __NVHW_QMD_H__
#include <stdio.h>
#include <stdint.h>
#include "util/u_debug.h"
#include "drf.h"

#define NVQMD_ENUM_1(X,drf,v0)                                                 \
   [drf##_##v0] = #v0
#define NVQMD_ENUM_2(X,drf,v0,v1)                                              \
   [drf##_##v0] = #v0,                                                         \
   [drf##_##v1] = #v1
#define NVQMD_ENUM_3(X,drf,v0,v1,v2)                                           \
   [drf##_##v0] = #v0,                                                         \
   [drf##_##v1] = #v1,                                                         \
   [drf##_##v2] = #v2
#define NVQMD_ENUM_8(X,drf,v0,v1,v2,v3,v4,v5,v6,v7)                            \
   [drf##_##v0] = #v0,                                                         \
   [drf##_##v1] = #v1,                                                         \
   [drf##_##v2] = #v2,                                                         \
   [drf##_##v3] = #v3,                                                         \
   [drf##_##v4] = #v4,                                                         \
   [drf##_##v5] = #v5,                                                         \
   [drf##_##v6] = #v6,                                                         \
   [drf##_##v7] = #v7

#define NVQMD_ENUM_(X,_1,_2,_3,_4,_5,_6,_7,_8,_9,IMPL,...) IMPL
#define NVQMD_ENUM(A...) NVQMD_ENUM_(X, ##A, NVQMD_ENUM_8, NVQMD_ENUM_7,       \
                                             NVQMD_ENUM_6, NVQMD_ENUM_5,       \
                                             NVQMD_ENUM_4, NVQMD_ENUM_3,       \
                                             NVQMD_ENUM_2, NVQMD_ENUM_1)(X, ##A)

#define NVQMD_VAL_N(X,d,r,p,f,o) do {                                          \
   uint32_t val = NVVAL_MW_GET_X((p), d##_##r##_##f);                          \
   debug_printf("   %-36s: "o"\n", #f, val);                                   \
} while(0)
#define NVQMD_VAL_I(X,d,r,p,f,i,o) do {                                        \
   uint32_t val = NVVAL_MW_GET_X((p), d##_##r##_##f(i));                       \
   char name[80];                                                              \
   snprintf(name, sizeof(name), "%s(%d)", #f, i);                              \
   debug_printf("   %-36s: "o"\n", name, val);                                 \
} while(0)
#define NVQMD_VAL_(X,_1,_2,_3,_4,_5,_6,IMPL,...) IMPL
#define NVQMD_VAL(A...) NVQMD_VAL_(X, ##A, NVQMD_VAL_I, NVQMD_VAL_N)(X, ##A)

#define NVQMD_DEF(d,r,p,f,e...) do {                                           \
   static const char *ev[] = { NVQMD_ENUM(d##_##r##_##f,##e) };                \
   uint32_t val = NVVAL_MW_GET((p), d, r, f);                                  \
   if (val < ARRAY_SIZE(ev) && ev[val])                                        \
      debug_printf("   %-36s: %s\n", #f, ev[val]);                             \
   else                                                                        \
      debug_printf("   %-36s: UNKNOWN 0x%x\n", #f, val);                       \
} while(0)
#define NVQMD_IDX(d,r,p,f,i,e...) do {                                         \
   static const char *ev[] = { NVQMD_ENUM(d##_##r##_##f,##e) };                \
   char name[80];                                                              \
   snprintf(name, sizeof(name), "%s(%d)", #f, i);                              \
   uint32_t val = NVVAL_MW_GET((p), d, r, f, i);                               \
   if (val < ARRAY_SIZE(ev) && ev[val])                                        \
      debug_printf("   %-36s: %s\n", name, ev[val]);                           \
   else                                                                        \
      debug_printf("   %-36s: UNKNOWN 0x%x\n", name, val);                     \
} while(0)

void NVA0C0QmdDump_V00_06(uint32_t *);
void NVC0C0QmdDump_V02_01(uint32_t *);
void NVC3C0QmdDump_V02_02(uint32_t *);
#endif
