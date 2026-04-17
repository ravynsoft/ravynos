/*
cc tools/vtstat.c -o /tmp/vtstat -framework IOKit -framework CoreFoundation -g -Wall
*/

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <IOKit/IOKitLib.h>


typedef uint32_t ppnum_t;
#define arrayCount(x)	(sizeof(x) / sizeof(x[0]))

struct vtd_space_stats
{
    ppnum_t vsize;
    ppnum_t tables;
    ppnum_t bused;
    ppnum_t rused;
    ppnum_t largest_paging;
    ppnum_t largest_32b;
    ppnum_t inserts;
    ppnum_t max_inval[2];
    ppnum_t breakups;
    ppnum_t merges;
    ppnum_t allocs[64];
	ppnum_t bcounts[20];
};
typedef struct vtd_space_stats vtd_space_stats_t;

int main(int argc, char * argv[])
{
    io_service_t		vtd;
    CFDataRef			statsData;
    vtd_space_stats_t *	stats;
    uint32_t			idx;
    uint64_t            totalAllocs;

    vtd = IOServiceGetMatchingService(kIOMasterPortDefault, IOServiceMatching("AppleVTD"));
    assert(vtd);
	statsData = IORegistryEntryCreateCFProperty(vtd, CFSTR("stats"),
								kCFAllocatorDefault, kNilOptions);
    assert(statsData);

	stats = (vtd_space_stats_t *) CFDataGetBytePtr(statsData);

	printf("vsize          0x%x\n", stats->vsize);
	printf("tables         0x%x\n", stats->tables);
	printf("bused          0x%x\n", stats->bused);
	printf("rused          0x%x\n", stats->rused);
	printf("largest_paging 0x%x\n", stats->largest_paging);
	printf("largest_32b    0x%x\n", stats->largest_32b);
	printf("max_binval     0x%x\n", stats->max_inval[0]);
	printf("max_rinval     0x%x\n", stats->max_inval[1]);
	printf("inserts        0x%x\n", stats->inserts);

	for (totalAllocs = 0, idx = 0; idx < arrayCount(stats->allocs); idx++)
	{
		printf("allocs[%2d]    0x%x\n", idx, stats->allocs[idx]);
		totalAllocs += stats->allocs[idx];
	}

	printf("breakups       0x%x(%.3f)\n", stats->breakups, stats->breakups * 100.0 / totalAllocs);
	printf("merges         0x%x(%.3f)\n", stats->breakups, stats->breakups * 100.0 / totalAllocs);

	for (idx = 0; idx < arrayCount(stats->bcounts); idx++)	printf("bcounts[%2d]    0x%x\n", idx, stats->bcounts[idx]);

	exit(0);
}