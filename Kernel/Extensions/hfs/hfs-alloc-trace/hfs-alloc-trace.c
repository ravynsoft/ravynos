//
//  hfs-alloc-trace.c
//  hfs-alloc-trace
//
//  Created by Chris Suter on 8/19/15.
//
//

#include <sys/sysctl.h>
#include <stdlib.h>
#include <err.h>
#include <stdio.h>
#include <stdbool.h>

#include "../core/hfs_alloc_trace.h"

int main(void)
{
    size_t sz = 128 * 1024;
    struct hfs_alloc_trace_info *info = malloc(sz);

    if (sysctlbyname("vfs.generic.hfs.alloc_trace_info", info, &sz,
                     NULL, 0)) {
        err(1, "sysctlbyname failed");
    }

    for (int i = 0; i < info->entry_count; ++i) {
        printf(" -- 0x%llx:%llu <%llu> --\n", info->entries[i].ptr,
               info->entries[i].sequence, info->entries[i].size);
        for (int j = 0; j < HFS_ALLOC_BACKTRACE_LEN; ++j)
            printf("0x%llx\n", info->entries[i].backtrace[j]);
    }

    if (info->more)
        printf("[skipped]\n");

    return 0;
}
