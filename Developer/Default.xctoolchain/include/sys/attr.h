#ifdef __APPLE__
#include_next <sys/attr.h>
#else
#ifndef _SYS_ATTR_H_
#define _SYS_ATTR_H_

#include <sys/types.h>
#include <stdint.h>

typedef u_int32_t attrgroup_t;
struct attrlist {
  u_short bitmapcount;                    /* number of attr. bit sets in list (should be 5) */
  u_int16_t reserved;                             /* (to maintain 4-byte alignment) */
  attrgroup_t commonattr;                 /* common attribute group */
  attrgroup_t volattr;                    /* Volume attribute group */
  attrgroup_t dirattr;                    /* directory attribute group */
  attrgroup_t fileattr;                   /* file attribute group */
  attrgroup_t forkattr;                   /* fork attribute group */
};
#define ATTR_BIT_MAP_COUNT 5

#define ATTR_CMN_FNDRINFO 1
#define ATTR_FILE_DATALENGTH 2
#define ATTR_FILE_RSRCLENGTH 4

int    getattrlist(const char*,void*,void*,size_t,unsigned int);

#endif

#endif /* __APPLE__ */
