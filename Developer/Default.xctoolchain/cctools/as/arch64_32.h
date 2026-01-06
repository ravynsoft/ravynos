#ifndef _STUFF_TARGET_ARCH_H_
#define _STUFF_TARGET_ARCH_H_
#include <stdint.h>

#ifdef ARCH64	/* 64-bit architecutres */

typedef struct mach_header_64 mach_header_t;
#define MH_MAGIC_VALUE MH_MAGIC_64
#define swap_mach_header_t swap_mach_header_64
typedef struct segment_command_64 segment_command_t;
#define	LC_SEGMENT_VALUE LC_SEGMENT_64
#define swap_segment_command_t swap_segment_command_64
typedef struct section_64 section_t;
#define swap_section_t swap_section_64
typedef struct nlist_64 nlist_t;
#define swap_nlist_t swap_nlist_64

typedef int64_t signed_target_addr_t;
#define TA_DFMT "%llu"

#else		/* 32-bit architecutres */

typedef struct mach_header mach_header_t;
#define MH_MAGIC_VALUE MH_MAGIC
#define swap_mach_header_t swap_mach_header
typedef struct segment_command segment_command_t;
#define	LC_SEGMENT_VALUE LC_SEGMENT
#define swap_segment_command_t swap_segment_command
typedef struct section section_t;
#define swap_section_t swap_section
typedef struct nlist nlist_t;
#define swap_nlist_t swap_nlist

typedef int32_t signed_target_addr_t;
#define TA_DFMT "%u"

#endif 

#endif /* _STUFF_TARGET_ARCH_H_ */
