
#ifndef NVE4_COMPUTE_H
#define NVE4_COMPUTE_H

#include "nvc0/nve4_compute.xml.h"

struct nve4_mp_trap_info {
   u32 lock;
   u32 pc;
   u32 trapstat;
   u32 warperr;
   u32 tid[3];
   u32 ctaid[3];
   u32 pad028[2];
   u32 r[64];
   u32 flags;
   u32 pad134[3];
   u32 s[0x3000];
};

#endif /* NVE4_COMPUTE_H */
