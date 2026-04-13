//
//  hfs_alloc_trace.h
//  hfs
//
//  Created by Chris Suter on 8/19/15.
//
//

#ifndef hfs_alloc_trace_h
#define hfs_alloc_trace_h

#include <sys/types.h>
#include <stdbool.h>

enum {
	HFS_ALLOC_BACKTRACE_LEN = 4,
};

#pragma pack(push, 8)

struct hfs_alloc_trace_info {
	int entry_count;
	bool more;
	struct hfs_alloc_info_entry {
		uint64_t ptr;
		uint64_t sequence;
		uint64_t size;
		uint64_t backtrace[HFS_ALLOC_BACKTRACE_LEN];
	} entries[];
};

#pragma pack(pop)

#endif /* hfs_alloc_trace_h */
