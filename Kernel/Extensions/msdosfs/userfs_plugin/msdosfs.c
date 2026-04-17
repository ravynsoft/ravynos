/*
 UserFS plug-in for FAT (msdosfs)
 Copyright © 2013-2014 Apple Inc. All rights reserved.
 */

#import  <os/log.h>
#include <assert.h>
#include <errno.h>
#include <notify.h>
#include <notify_keys.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <spawn.h>
#include <time.h>
#include <xpc/xpc.h>
#include <sys/ioctl.h>
#include <sys/disk.h>
#include <sys/time.h>
#include <sys/uio.h>
#include <pthread.h>
#include <UserFS/UserFS_Plugin.h>
#include <UserFS/UserFS_XPC.h>
#include "bpb.h"
#include "bootsect.h"
#include "direntry.h"
#include "fat.h"
#include <err.h>
#include <sysexits.h>

os_log_t         msdosfs_log_default;      // Default log destination
                                           // mildly redundant as UserFS has only one, but best practice
os_log_type_t    msdosfs_log_Level       = OS_LOG_TYPE_DEBUG;        // Run-time default log level, can conceivably be raised by CL options
os_log_type_t    msdosfs_log_Info_Level  = OS_LOG_TYPE_INFO;   // Run-time Info log level, can conceivably be raised by CL options

#define MSDOSFS_LOG(LOG, ...)        os_log_with_type((LOG), msdosfs_log_Level, ##__VA_ARGS__)
#define MSDOSFS_LOG_INFO(LOG, ...)   os_log_with_type((LOG), msdosfs_log_Info_Level, ##__VA_ARGS__)
#define MSDOSFS_LOG_ERR(LOG, ...)    os_log_with_type((LOG), OS_LOG_TYPE_ERROR, ##__VA_ARGS__)

struct _userfs_volume_s {
    userfs_device_t device;
    
    uint32_t (*fatEntryOffset)(uint32_t cluster);
    uint32_t (*fatEntryGet)(uint32_t cluster, uint8_t **entry);
    uint32_t (*fatEntrySet)(uint32_t cluster, uint8_t **entry, uint32_t value);
    size_t dirBlockSize;            // Size in bytes of a single cached block of a directory
    uint32_t fatOffset;             // Offset, in bytes, from start of volume to start of active FAT
    uint32_t fatSize;               // Size, in bytes, of the FAT
    uint32_t fatMask;               // Mask to use for cluster numbers
    uint32_t bytesPerSector;
    uint32_t bytesPerCluster;
    uint32_t rootSector;            // Offset, in sectors, from start of volume to start of root directory
    uint32_t rootSize;              // Size, in sectors, of root directory
    uint64_t rootLength;            // Size, in bytes, of root directory
    uint32_t rootCluster;           // Cluster number of first cluster of root directory (FAT32 only)
    uint32_t clusterOffset;         // Offset, in sectors, from start of volume to start of first cluster
    uint32_t maxCluster;            // Largest valid cluster number (_clusterCount + 1)
    uint32_t fsInfoSector;          // FAT32 only; zero if no valid FSInfo sector
    uint32_t freeClusters;          // FAT32 only; only used if fsInfoSector != 0
    bool locked;
    bool dirty;
};

struct _userfs_stream_s {
    userfs_volume_t volume;
    const char *name;               // malloc'ed; must be freed when stream is freed
    uint64_t dirEntryOffset;
    int64_t  create_date;
    int64_t  mod_date;
    uint64_t length;
    uint32_t startingCluster;
    uint32_t cachedLogicalCluster;
    uint32_t cachedPhysicalCluster;
    uint32_t cachedClusterCount;
    uint32_t cachedNextCluster;
    uint64_t next_offset_to_cache;
    uint64_t last_offset_in_cache;
    int thread_count;
    bool directory;
    bool locked;
    bool fixedRoot;                 // True for FAT12 or FAT16 root directory
    bool close_lazily;
};

/*
 * The following are callbacks, into the executable that loaded us.
 * Having them be global function pointers lets us use them syntactically
 * like any other function, but without having to link explicitly at build
 * time, nor use dynamic lookup.  These pointers will be set up in
 * userfs_plugin_init().
 */
const char * (*device_name)(userfs_device_t device);
int (*device_fd)(userfs_device_t device);
pthread_mutex_t* (*device_mutex)(userfs_device_t device);
int (*cache_get_buffer)(userfs_device_t device, uint64_t offset, size_t length, userfs_buffer_t *buffer);
void (*cache_release_buffer)(userfs_device_t device, userfs_buffer_t buffer);     // TODO: Should this take a "dirty" flag?
int (*cache_flush_buffer)(userfs_device_t device, userfs_buffer_t buffer);
int (*cache_flush)(userfs_device_t device);
int (*cache_invalidate)(userfs_device_t device);
void * (*buffer_bytes)(userfs_buffer_t buffer);
off_t (*buffer_offset)(userfs_buffer_t buffer);
size_t (*buffer_size)(userfs_buffer_t buffer);
void (*buffer_mark_dirty)(userfs_buffer_t buffer);

bool (*cache_get_content_buffer)(userfs_device_t device, userfs_contentbuffer_t *buffer, uint64_t offset, bool empty);
void * (*content_buffer_bytes)(userfs_contentbuffer_t buffer);
void (*populate_content_buffer_cache)(userfs_device_t device, userfs_contentbuffer_t buffer);


struct unistr255 {
    uint16_t length;
    uint16_t chars[255];
};

enum {
    // Maximum device block size that we support
    MAX_BLOCK_SIZE = 4096U,
    
    // The I/O size for reading the head or tail of a misaligned read
    UNALIGNED_BLOCK_SIZE  = 4096U,
    UNALIGNED_BLOCK_MASK  = UNALIGNED_BLOCK_SIZE - 1,
    
    //
    // (Maximum) size of a FAT buffer.
    //
    // This is assumed to be a power of two.  It must be at least as large as
    // the sector size (which could be as large as 4096).  Since FAT12 entries
    // cross sector boundaries, it needs to be a multiple of two sectors.
    // The maximum size of a FAT12 FAT is 6KiB, so using 8KiB here guarantees
    // the entire FAT12 FAT will be read/written at one time, which simplifies
    // things.
    //
    FAT_BLOCK_SIZE = 8192U,
    FAT_BLOCK_MASK = FAT_BLOCK_SIZE -1,
    
    // Maximum amount of a directory to read at one time
    MAX_DIR_BLOCK_SIZE = 128 * 1024,

    // The maximum length of a FAT file name when converted to UTF-8
    // C-string.  It is a maximum of three bytes per UTF-16 code point,
    // plus one byte for the NUL terminator.
    FAT_MAX_FILENAME_UTF8 = WIN_MAXLEN * 3 + 1
};

static errno_t msdosfs_volume_open(userfs_device_t device, bool locked, userfs_volume_t *volume);
static bool msdosfs_volume_is_locked(userfs_volume_t volume);
static errno_t msdosfs_volume_flush(userfs_volume_t volume);
static errno_t msdosfs_volume_close(userfs_volume_t volume);

/* Files and directories */
static errno_t msdosfs_item_get_info(userfs_volume_t volume, const char *path, xpc_object_t info);
static errno_t msdosfs_dir_enumerate(userfs_volume_t volume, const char *path, __strong xpc_object_t *state, xpc_object_t children);
static errno_t msdosfs_item_delete(userfs_volume_t volume, const char *path);

/* Streams */
static errno_t msdosfs_stream_open(userfs_volume_t volume, const char *path, userfs_stream_t *stream);
static uint64_t msdosfs_stream_length(userfs_stream_t stream);
//static uint64_t stream_get_next_offset_to_cache(userfs_stream_t stream);
//static uint64_t stream_get_last_offset_in_cache(userfs_stream_t stream);
void (*stream_increment_threadCount)(userfs_stream_t stream);
static errno_t msdosfs_stream_read(userfs_stream_t stream, void *buffer, uint64_t offset, size_t length, bool readAheadOperation);
static errno_t msdosfs_stream_close(userfs_stream_t stream);
static const char * msdosfs_stream_name(userfs_stream_t stream);    // For debugging

static int dir_lookup(userfs_stream_t parent, const uint8_t *child_name, size_t child_name_length, userfs_stream_t *child);

static int stream_get_buffer(userfs_stream_t stream, uint64_t offset, size_t length, userfs_buffer_t *buffer);


/*
 * Convert from FAT's date, time, and hundredths of seconds to nanoseconds
 * since the POSIX epoch.
 *
 * Dates/times on disk are sometimes garbage.  All zeroes is a reasonably common
 * form of garbage, but we also have to cope with some of the sub-fields being
 * too large.  The way we handle garbage is somewhat arbitrary.
 *
 * The month and day-of-month fields are 1-based.  It seems reasonable to pin
 * them to 1 if they were zero (resulting in Jan 1, instead of the previous
 * December, or Nov 30th).  Pinning too-large months to 1 is arbitrary.
 *
 * Since hours, minutes and seconds are 0-based, all-zero garbage produces a
 * valid result.  What to do with too-large values?  mktime(3) handles out-of-
 * range values predictably, and there doesn't seem to be any advantage to
 * explictly handling them in some other way.
 */
static uint64_t timestamp_to_nanoseconds(uint16_t date, uint16_t time, uint8_t hundredths)
{
    time_t seconds;
    struct tm tm = {0};
    int64_t ns;
    int temp;
    
    tm.tm_year = (date >> 9) + 80;
    temp = (date >> 5) & 0x0F;              // month; should be 1..12
    if (temp < 1 || temp > 12)
        temp = 1;
    tm.tm_mon = temp - 1;
    temp = date & 0x1F;                     // day of month; should be 1..31
    if (temp == 0)
        temp = 1;
    tm.tm_mday = temp;
    tm.tm_hour = (time >> 11) & 0x1F;       // 0..23
    tm.tm_min = (time >> 5) & 0x3F;         // 0..59
    tm.tm_sec = (time & 0x1F) * 2 + hundredths/100;  // 0..59
    tm.tm_isdst = -1;   // Automatically determine daylight saving time
    seconds = mktime(&tm);
    ns = (int64_t)seconds * 1000000000 + (int64_t)(hundredths % 100) * 10000000;
    return ns;
}

/*
 * Case folding table for Latin-1.
 *
 * Maps 'A'..'Z' to 'a'..'z' and 0xc0..0xd6 to 0xe0..0xf6 and
 * 0xd8..0xde to 0xf8..0xfe.
 */
static uint8_t latin1_to_lower[256] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, /* 00-07 */
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, /* 08-0f */
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, /* 10-17 */
    0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, /* 18-1f */
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, /* 20-27 */
    0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, /* 28-2f */
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, /* 30-37 */
    0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f, /* 38-3f */
    0x40, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, /* 40-47 */
    0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f, /* 48-4f */
    0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, /* 50-57 */
    0x78, 0x79, 0x7a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f, /* 58-5f */
    0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, /* 60-67 */
    0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f, /* 68-6f */
    0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, /* 70-77 */
    0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f, /* 78-7f */
    0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, /* 80-87 */
    0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f, /* 88-8f */
    0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, /* 90-97 */
    0x98, 0x99, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f, /* 98-9f */
    0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, /* a0-a7 */
    0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf, /* a8-af */
    0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, /* b0-b7 */
    0xb8, 0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf, /* b8-bf */
    0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, /* c0-c7 */
    0xe8, 0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef, /* c8-cf */
    0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xd7, /* d0-d7 */
    0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xdf, /* d8-df */
    0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, /* e0-e7 */
    0xe8, 0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef, /* e8-ef */
    0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, /* f0-f7 */
    0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff, /* f8-ff */
};

/*
 * Map byte values 0x80..0x9F from Windows Code Page 1252 to Unicode.
 * See <http://en.wikipedia.org/wiki/Windows-1252>
 */
static uint16_t cp1252_to_unicode[32] = {
    0x20AC, 0x003f, 0x201A, 0x0192, 0x201E, 0x2026, 0x2020, 0x2021, /* 80-87 */
    0x02C6, 0x2030, 0x0160, 0x2039, 0x0152, 0x003f, 0x017D, 0x003f, /* 88-8F */
    0x003f, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014, /* 90-97 */
    0x02DC, 0x2122, 0x0161, 0x203A, 0x0153, 0x003f, 0x017E, 0x0178, /* 98-9F */
};


/*
 * Run fsck_msdos to repair the device using the open file descriptor "fd".
 * Return zero if fsck_msdos was executed and exited with status zero (success);
 * otherwise return non-zero.
 */
static int fsck_msdos(int fd, const char *name)
{
    int result;
    int child_status;
    pid_t child, child_found;
    extern char **environ;
    posix_spawn_file_actions_t file_actions;
    posix_spawnattr_t spawn_attr;
    char path[24];      /* /dev/fd/<NUMBER> */
    const char * const argv[] = {"fsck_msdos", "-y", path, NULL};
    
    snprintf(path, sizeof(path), "/dev/fd/%d", fd);
    
    posix_spawn_file_actions_init(&file_actions);
    posix_spawn_file_actions_addinherit_np(&file_actions, 0);
    posix_spawn_file_actions_addinherit_np(&file_actions, 1);
    posix_spawn_file_actions_addinherit_np(&file_actions, 2);
    posix_spawn_file_actions_addinherit_np(&file_actions, fd);
    posix_spawnattr_init(&spawn_attr);
    posix_spawnattr_setflags(&spawn_attr, POSIX_SPAWN_CLOEXEC_DEFAULT);
    
    result = posix_spawn(&child,
                         "/System/Library/Filesystems/msdos.fs/Contents/Resources/fsck_msdos",
                         &file_actions,
                         &spawn_attr,
                         (char * const *)argv,
                         environ);
    if (result)
    {
        MSDOSFS_LOG_ERR(msdosfs_log_default, "%s: posix_spawn fsck_msdos: error=%d\n", name, result);
        return -1;
    }
    
    // Wait for child to finish
    do {
        child_found = waitpid(child, &child_status, 0);
    } while (child_found == -1 && errno == EINTR);
    
    if (child_found == -1)
    {
        MSDOSFS_LOG_ERR(msdosfs_log_default, "%s: waitpid fsck_msdos: errno=%d\n", name, errno);
        return -1;
    }
    if (WIFEXITED(child_status))
    {
        result = WEXITSTATUS(child_status);
        if (result)
        {
            MSDOSFS_LOG_ERR(msdosfs_log_default, "%s: fsck_msdos: exited with status %d\n", name, result);
        }
    }
    else
    {
        MSDOSFS_LOG_ERR(msdosfs_log_default, "%s: fsck_msdos: terminated by signal %d\n", name, WTERMSIG(child_status));
        result = -1;
    }
    
    return result;
}


static uint32_t fat32EntryOffset(uint32_t cluster)
{
    // FAT32 uses 32 bits per FAT entry, but the upper 4 bits are reserved
    return cluster * sizeof(uint32_t);
}

static uint32_t fat32EntryGet(uint32_t cluster, uint8_t **entry)
{
    uint32_t result = getuint32(*entry);
    *entry += sizeof(uint32_t);
    return result & 0x0FFFFFFFU;
}

static uint32_t fat32EntrySet(uint32_t cluster, uint8_t **entry, uint32_t value)
{
    uint32_t oldValue = getuint32(*entry);
    putuint32(*entry, (value & 0x0FFFFFFFU) | (oldValue & 0xF0000000U));
    *entry += sizeof(uint32_t);
    return oldValue & 0x0FFFFFFFU;
}

static uint32_t fat16EntryOffset(uint32_t cluster)
{
    return cluster * sizeof(uint16_t);
}

static uint32_t fat16EntryGet(uint32_t cluster, uint8_t **entry)
{
    uint32_t result = getuint16(*entry);
    *entry += sizeof(uint16_t);
    return result;
}

static uint32_t fat16EntrySet(uint32_t cluster, uint8_t **entry, uint32_t value)
{
    uint32_t oldValue = getuint16(*entry);
    putuint16(*entry, value);
    *entry += sizeof(uint16_t);
    return oldValue;
}

static uint32_t fat12EntryOffset(uint32_t cluster)
{
    /*
     * FAT12 tightly packs 12 bits per cluster; 3 bytes for every 2 clusters.
     *
     * I find it easiest to think in terms of a pair of FAT entries (cluster
     * numbers 2*N and 2*N+1) occupying 3 bytes (at offset 3*N).
     */
    return (cluster * 3) / 2;
}

static uint32_t fat12EntryGet(uint32_t cluster, uint8_t **entry)
{
    // *entry may not be 2-byte aligned, so fetch the value a byte at a time
    uint32_t result = (*entry)[0] | ((*entry)[1] << 8);
    
    /*
     * Extract the upper or lower 12 bits.  If the cluster number is even, we
     * want the low 12 bits.  If the cluster number is odd, we want the high
     * 12 bits.
     */
    if (cluster & 1)
    {
        result >>= 4;
        *entry += 2;
    }
    else
    {
        result &= 0x0FFF;
        *entry += 1;
    }
    
    return result;
}

static uint32_t fat12EntrySet(uint32_t cluster, uint8_t **entry, uint32_t value)
{
    uint32_t oldValue = (*entry)[0] | ((*entry)[1] << 8);
    uint32_t newValue;
    
    /*
     * Replace the upper or lower 12 bits.  If the cluster number is even, we
     * change the low 12 bits.  If the cluster number is odd, we change the high
     * 12 bits.
     */
    if (cluster & 1)
    {
        newValue = (oldValue & 0x000F) | (value << 4);
        (*entry)[0] = newValue;
        (*entry)[1] = newValue >> 8;
        oldValue >>= 4;
        *entry += 2;
    }
    else
    {
        newValue = (oldValue & 0xF000) | (value & 0x0FFF);
        (*entry)[0] = newValue;
        (*entry)[1] = newValue >> 8;
        oldValue &= 0x0FFF;
        *entry += 1;
    }
    
    return oldValue;
}

/* Pin the size of a FAT cache block to the end of the FAT */
static size_t fat_block_size(userfs_volume_t v, off_t blockOffset)
{
    size_t blockSize = FAT_BLOCK_SIZE;
    
    if (blockOffset + blockSize > v->fatSize)
        blockSize = (size_t)(v->fatSize - blockOffset);
    
    return blockSize;
}

/*
 * Determine how many clusters within a chain (eg., allocated to a file) are contiguous.
 * This tells you how big the "extent" at the given cluster is.
 *
 * Inputs:
 *  v           The volume containing the clusters
 *  cluster     The first cluster of the extent (the first of potentially many contiguous
 *              clusters in a chain).
 * Outputs:
 * *next_extent The next cluster in the chain (the first non-contiguous cluster).  This is
 *              the cluster number pointed to by the last cluster in the found extent.
 * *error       Errno value if an error was detected.  Unchanged on success.
 *
 * Function result:
 *  0           An error was detected.  The errno value is returned in *error.
 *  non-0       The number of contiguous clusters found.
 *
 */
static uint32_t contiguous_clusters_in_chain(userfs_volume_t v, uint32_t cluster, uint32_t *next_cluster, errno_t *error)
{
    assert(cluster >= CLUST_FIRST && cluster <= v->maxCluster);
    errno_t err = 0;
    uint32_t result = 1;
    uint8_t *entry;
    uint8_t *bufferEnd;
    userfs_buffer_t buffer = NULL;
    
    if (cluster < CLUST_FIRST || cluster > v->maxCluster)
    {
        MSDOSFS_LOG_ERR(msdosfs_log_default, "%s: invalid cluster number (%u) in contiguous_clusters_in_chain\n", device_name(v->device), cluster);
        *error = EIO;
        return 0;
    }
    
    //
    // Figure out which block of the FAT contains the entry for the given cluster.
    //
    // NOTE: FAT_BLOCK_SIZE is at least as large as the used portion of a FAT12 FAT.
    // So, for FAT12, we will end up reading the entire FAT at one time.
    off_t entryOffset = v->fatEntryOffset(cluster);
    off_t blockOffset = entryOffset & ~FAT_BLOCK_MASK;    // round down to start of block
    size_t blockSize = fat_block_size(v, blockOffset);
    
    //
    //  Fetch the block from the cache.  Get a pointer to the given cluster's entry.
    //
    err = cache_get_buffer(v->device, v->fatOffset+blockOffset, blockSize, &buffer);
    if (err)
    {
        MSDOSFS_LOG_ERR(msdosfs_log_default, "%s: unable to get FAT buffer for cluster=%lu (error %d)\n", device_name(v->device), (unsigned long)cluster, err);
        *error = EIO;
        result = 0;
        goto done;
    }
    assert(buffer_size(buffer) == blockSize);
    bufferEnd = (uint8_t*)buffer_bytes(buffer) + buffer_size(buffer);
    entry = (uint8_t *)buffer_bytes(buffer) + (entryOffset - blockOffset);
    
    //
    // Look for contiguous clusters
    //
    while ((*next_cluster = v->fatEntryGet(cluster, &entry)) == cluster+1)
    {
        result++;
        cluster++;
        if (entry == bufferEnd)
        {
            // We ran off the end of the current block, so fetch the next one.
            cache_release_buffer(v->device, buffer);
            blockOffset += blockSize;
            blockSize = fat_block_size(v, blockOffset);
            
            err = cache_get_buffer(v->device, v->fatOffset+blockOffset, blockSize, &buffer);
            if (err)
            {
                MSDOSFS_LOG_ERR(msdosfs_log_default, "%s: unable to get FAT buffer for cluster=%lu (error %d)\n", device_name(v->device), (unsigned long)cluster, err);
                *error = EIO;
                result = 0;
                goto done;
            }
            bufferEnd = (uint8_t*)buffer_bytes(buffer) + buffer_size(buffer);
            entry = (uint8_t*)buffer_bytes(buffer);
        }
    }
done:
    if (buffer != NULL)
        cache_release_buffer(v->device, buffer);
    return result;
}

static errno_t chain_length(userfs_volume_t v, uint32_t cluster, uint32_t *length)
{
    uint32_t numClusters = 0;
    uint32_t extentLength;
    errno_t err;
    
    while (cluster >= CLUST_FIRST && cluster <= v->maxCluster)
    {
        extentLength = contiguous_clusters_in_chain(v, cluster, &cluster, &err);
        if (extentLength == 0)
        {
            /* There was an error reading from the FAT */
            MSDOSFS_LOG_ERR(msdosfs_log_default, "%s: Error %d reading cluster chain at %u\n", device_name(v->device), err, cluster);
            return err;
        }
        numClusters += extentLength;
        if (numClusters >= v->maxCluster)
        {
            MSDOSFS_LOG_ERR(msdosfs_log_default, "%s: Cluster chain has a cycle.\n", device_name(v->device));
            return EIO;
        }
    }
    if (cluster < (CLUST_EOFS & v->fatMask))
    {
        /* There was a bad cluster number in the chain. */
        MSDOSFS_LOG_ERR(msdosfs_log_default, "%s: A cluster chain has an invalid cluster number (%u)\n", device_name(v->device), cluster);
        return EIO;
    }
    *length = numClusters;
    return 0;
}

static errno_t chain_free(userfs_volume_t volume, uint32_t cluster, uint32_t *numFreed)
{
    off_t entryOffset;      // Offset in bytes from start of FAT to cluster's entry
    off_t blockOffset;      // Offset in bytes from start of FAT to start of block containing cluster's entry
    size_t blockSize;       // Size in bytes of FAT block containing cluster's entry
    userfs_buffer_t buffer = NULL;
    uint8_t *entry;
    errno_t error;
    
    *numFreed = 0;
    
    while (cluster >= CLUST_FIRST && cluster <= volume->maxCluster)
    {
        
        //
        // Figure out the location in the FAT of cluster's entry.
        //
        entryOffset = volume->fatEntryOffset(cluster);
        
        //
        // See if we need to fetch the FAT block containing cluster's entry.
        //
        if (buffer == NULL || entryOffset < blockOffset || entryOffset >= blockOffset + blockSize)
        {
            //
            // If we already had a cached block, release it now.
            //
            if (buffer)
            {
                cache_release_buffer(volume->device, buffer);
                buffer = NULL;
            }
            
            //
            //  Fetch the block from the cache.
            //
            blockOffset = entryOffset & ~FAT_BLOCK_MASK;    // round down to start of block
            blockSize = fat_block_size(volume, blockOffset);
            error = cache_get_buffer(volume->device, volume->fatOffset+blockOffset, blockSize, &buffer);
            if (error)
            {
                MSDOSFS_LOG_ERR(msdosfs_log_default, "%s: unable to get FAT buffer for cluster=%lu (error %d)\n", device_name(volume->device), (unsigned long)cluster, error);
                return EIO;
            }
            assert(buffer_size(buffer) == blockSize);
        }
        
        // Get a pointer to the given cluster's entry.
        assert(entryOffset >= blockOffset);
        assert(entryOffset < blockOffset + blockSize);
        entry = (uint8_t *)buffer_bytes(buffer) + (entryOffset - blockOffset);
        
        // Mark the cluster free and get the next cluster in the chain
        cluster = volume->fatEntrySet(cluster, &entry, CLUST_FREE);
        buffer_mark_dirty(buffer);
        ++(*numFreed);
    }
    
    // Release the cache buffer
    if (buffer)
    {
        cache_release_buffer(volume->device, buffer);
        buffer = NULL;
    }
    
    // Check if we encountered an out-of-range cluster number
    if (cluster < (CLUST_EOFS & volume->fatMask))
    {
        /* There was a bad cluster number in the chain. */
        MSDOSFS_LOG_ERR(msdosfs_log_default, "%s: A cluster chain has an invalid cluster number (%u)\n", device_name(volume->device), cluster);
        return EIO;
    }
    
    return 0;
}

static void volume_update_fsinfo(userfs_volume_t volume, uint32_t numFreed)
{
    if (volume->fsInfoSector == 0)
        return;
    
    if ((uint64_t) numFreed + volume->freeClusters >= volume->maxCluster)
    {
        /* Free cluster count would become too large, so ignore updates. */
        MSDOSFS_LOG_ERR(msdosfs_log_default, "%s: Free cluster count would exceed cluster count; ignoring.\n", device_name(volume->device));
        volume->fsInfoSector = 0;
        return;
    }
    volume->freeClusters += numFreed;
    
    userfs_buffer_t fsInfoBuffer = NULL;
    errno_t error = cache_get_buffer(volume->device, volume->fsInfoSector*volume->bytesPerSector, volume->bytesPerSector, &fsInfoBuffer);
    if (error)
    {
        MSDOSFS_LOG_ERR(msdosfs_log_default, "%s: Error %d reading FSInfo sector; ignoring.\n", device_name(volume->device), error);
        volume->fsInfoSector = 0;
        return;
    }
    
    struct fsinfo *fsInfo = buffer_bytes(fsInfoBuffer);
    putuint32(fsInfo->fsinfree, volume->freeClusters);
    
    buffer_mark_dirty(fsInfoBuffer);
    cache_release_buffer(volume->device, fsInfoBuffer);
}

static uint64_t volume_offset_for_cluster(userfs_volume_t volume, uint32_t cluster)
{
    return (((uint64_t)cluster - CLUST_FIRST) * volume->bytesPerCluster) + ((uint64_t)(volume->clusterOffset) * volume->bytesPerSector);
}

static errno_t volume_read(userfs_volume_t volume, void *buffer, size_t length, off_t offset)
{
    MSDOSFS_LOG(msdosfs_log_default, "vol_read    O=0x%016llx B=0x%08zx\n", offset, length);
    
    char *tempBuffer = NULL;
    ssize_t actual;
    errno_t error = 0;
    int fd = device_fd(volume->device);
    
    /*
     * If the starting offset was not block aligned, then we have
     * to read the corresponding block into a temporary buffer
     * and start copying from somewhere in the middle of that
     * buffer to the caller's buffer.  If the length is small
     * enough, we'll only copy from the middle of the temporary
     * buffer.  If the length is large enough, we'll copy through
     * the end of the temporary buffer.
     */
    size_t block_offset = offset & UNALIGNED_BLOCK_MASK;
    if (block_offset != 0)
    {
        size_t tail_length = UNALIGNED_BLOCK_SIZE - block_offset;
        if (tail_length > length)
            tail_length = length;
        
        tempBuffer = malloc(UNALIGNED_BLOCK_SIZE);
        assert(tempBuffer);
        
        /*
         * TODO: Handle case where we're reading at end of partition,
         * and partition isn't a multiple of UNALIGNED_BLOCK_SIZE.
         */
        actual = pread(fd, tempBuffer, UNALIGNED_BLOCK_SIZE, offset - block_offset);
        if (actual != UNALIGNED_BLOCK_SIZE)
        {
            error = (actual == -1) ? errno : EIO;
            goto done;
        }
        memcpy(buffer, &tempBuffer[offset & UNALIGNED_BLOCK_MASK], tail_length);
        offset += tail_length;
        length -= tail_length;
        buffer = (char *)buffer + tail_length;
    }
    
    /*
     * Handle the middle of the read, if it is big enough to read whole
     * blocks directly into the caller's buffer (bypassing our temporary
     * buffer).
     */
    if (length >= UNALIGNED_BLOCK_SIZE)
    {
        assert((offset & UNALIGNED_BLOCK_MASK) == 0);
        
        size_t middle_length = length & ~UNALIGNED_BLOCK_MASK;
        actual = pread(fd, buffer, middle_length, offset);
        if (actual != middle_length)
        {
            error = (actual == -1) ? errno : EIO;
            goto done;
        }
        offset += middle_length;
        length -= middle_length;
        buffer = (char *)buffer + middle_length;
    }
    
    /*
     * Handle the end of the read, if there is anything left.
     * Read the corresponding block into a temporary buffer
     * and copy bytes from the start of the temporary buffer
     * into the caller's buffer.
     */
    if (length)
    {
        assert((offset & UNALIGNED_BLOCK_MASK) == 0);
        assert(length < UNALIGNED_BLOCK_SIZE);
        
        if (tempBuffer == NULL)
            tempBuffer = malloc(UNALIGNED_BLOCK_SIZE);
        assert(tempBuffer);
        
        /*
         * TODO: Handle case where we're reading at end of partition,
         * and partition isn't a multiple of UNALIGNED_BLOCK_SIZE.
         */
        actual = pread(fd, tempBuffer, UNALIGNED_BLOCK_SIZE, offset);
        if (actual != UNALIGNED_BLOCK_SIZE)
        {
            error = (actual == -1) ? errno : EIO;
            goto done;
        }
        memcpy(buffer, tempBuffer, length);
    }
    
done:
    if (tempBuffer)
    {
        free(tempBuffer);
        tempBuffer = NULL;
    }
    return error;
}

static userfs_stream_t volume_create_stream(userfs_volume_t v, const char *name, uint32_t cluster, uint64_t length)
{
    userfs_stream_t stream = calloc(1, sizeof(*stream));
    assert(stream != NULL);
    
    stream->volume = v;
    stream->name = name ? strdup(name) : NULL;
    stream->length = length;
    stream->startingCluster = cluster;
    stream->cachedLogicalCluster = 0;
    stream->cachedPhysicalCluster = 0;
    stream->cachedClusterCount = 0;
    stream->cachedNextCluster = cluster;
    stream->next_offset_to_cache = 0;
    stream->last_offset_in_cache = 0;
    stream->thread_count = 0;
    stream->close_lazily = 0;
    return stream;
}

static userfs_stream_t volume_get_root(userfs_volume_t v, int *error)
{
    userfs_stream_t root = volume_create_stream(v, "", v->rootCluster, v->rootLength);
    root->directory = true;
    if (v->fatMask != FAT32_MASK)
        root->fixedRoot = true;
    root->create_date = timestamp_to_nanoseconds(0, 0, 0);
    root->mod_date = root->create_date;
    return root;
}

/*
 * Return a stream object for the file or directory given by a pathname.  The pathname uses a forward
 * slash ("/") to separate components.  Pathnames are relative to the root of the given volume.
 * There is no notion of "current" or "working" directory.  Leading, trailing and repeated
 * slashes are ignored.  An empty path, or a path containing all slashes means the volume's root
 * directory.  The special names "." and ".." are explicitly hidden and will never be found.
 *
 * Inputs:
 *  volume      The volume to search for the given path.
 *  path        The pathname, encoded in UTF-8.
 *
 * Outputs:
 *  result          A newly created stream object for the file or directory (if it was found).
 *  result_parent   (Optional) If the pointer is non-NULL, contains a newly created stream for
 *                  the found object's parent directory.  If the found object is the root
 *                  directory (which has no parent), the returned stream will be NULL.
 * Function result:
 *  Zero for success, or an errno value.  If an error is returned, result and result_parent will
 *  be unchanged.
 */
static errno_t volume_lookup(userfs_volume_t volume, const char *path, userfs_stream_t *result, userfs_stream_t *result_parent)
{
    errno_t err = 0;
    
    // NOTE: We ignore repeated separators, and leading and trailing separators.
    // NOTE: Partial paths are always relative to the root directory.
    // NOTE: As a result, a path of "" is equivalent to "/" (and "////").
    
    // Start with the volume's root directory
    userfs_stream_t item = volume_get_root(volume, &err);
    if (item == NULL)
        return err;
    userfs_stream_t parent = NULL;
    userfs_stream_t child = NULL;
    
    // Special case getting info for "/" (or any of its aliases)?
    
    size_t child_name_length;
    
    while (*path && err == 0)
    {
        // Skip over previous separator to find start of name
        while (*path == '/')
            ++path;
        if (*path == '\0')
            break;
        
        // Find child name
        child_name_length = strcspn(path, "/");
        
        // search current directory for child with the given name
        err = dir_lookup(item, (const uint8_t *)path, child_name_length, &child);
        if (err == 0)
        {
            if (parent)
                msdosfs_stream_close(parent);
            parent = item;
            item = child;
            child = NULL;
            path += child_name_length;
        }
    }
    
    if (err == 0 && item != NULL)
    {
        *result = item;
        item = NULL;
        if (result_parent)
        {
            *result_parent = parent;
            parent = NULL;
        }
    }
    
    if (item != NULL)
        msdosfs_stream_close(item);
    if (parent != NULL)
        msdosfs_stream_close(parent);
    
    return err;
}

static size_t unistr255_to_utf8(const struct unistr255 *utf16, const char utf8[FAT_MAX_FILENAME_UTF8])
{
    uint8_t *p = (uint8_t *)utf8;
    uint16_t ch;
    uint16_t i;
    
    for (i = 0; i < utf16->length; ++i)
    {
        ch = utf16->chars[i];
        
        if (ch < 0x0080)
        {
            *(p++) = ch;
        }
        else if (ch < 0x0800)
        {
            *(p++) = 0xC0 + (ch >> 6);
            *(p++) = 0x80 + (ch & 0x3F);
        }
        else
        {
            *(p++) = 0xE0 + (ch >> 12);
            *(p++) = 0x80 + ((ch >> 6) & 0x3F);
            *(p++) = 0x80 + (ch & 0x3F);
        }
    }
    *(p++) = 0;
    
    return p-(uint8_t *)utf8;
}

static void xpc_dictionary_set_unistr255(xpc_object_t dictionary, const char *key, const struct unistr255 *utf16)
{
    char utf8[FAT_MAX_FILENAME_UTF8];
    unistr255_to_utf8(utf16, utf8);
    xpc_dictionary_set_string(dictionary, key, utf8);
}

static errno_t utf8_to_unistr255(const uint8_t *utf8, size_t utf8Length, struct unistr255 *unicode)
{
    size_t i;
    uint32_t ch;
    
    unicode->length = 0;
    for (i = 0; i < utf8Length; ++i)
    {
        ch = utf8[i];
        if ((ch & 0x80) == 0)
        {
            /* Plain ASCII */
        }
        else if ((ch & 0xE0) == 0xC0)
        {
            /* Two-byte sequence */
            if (utf8Length - i >= 2 && (utf8[i+1] & 0xC0) == 0x80)
            {
                ch = ((ch << 6) + utf8[++i]) - 0x3080;
            }
            else
            {
                /* Ill-formed UTF-8 */
                return EILSEQ;
            }
        }
        else if  ((ch & 0xF0) == 0xE0)
        {
            /* Three-byte sequence */
            if (utf8Length - i >= 3 && (utf8[i+1] & 0xC0) == 0x80 && (utf8[i+2] & 0xC0) == 0x80)
            {
                ch <<= 6;
                ch += utf8[++i];
                ch <<= 6;
                ch += utf8[++i];
                ch -= 0xE2080;
            }
            else
            {
                /* Ill-formed UTF-8 */
                return EILSEQ;
            }
        }
        else if ((ch & 0xF8) == 0xF0)
        {
            /* Four-byte sequence; requires surrogate pair for UTF-16 */
            if (utf8Length - i >= 4 && (utf8[i+1] & 0xC0) == 0x80 && (utf8[i+2] & 0xC0) == 0x80 && (utf8[i+3] & 0xC0) == 0x80)
            {
                ch <<= 6;
                ch += utf8[++i];
                ch <<= 6;
                ch += utf8[++i];
                ch <<= 6;
                ch += utf8[++i];
                ch -= 0x3C82080;
            }
            else
            {
                /* Ill-formed UTF-8 */
                return EILSEQ;
            }
        }
        
        if (ch > 0xFFFF)
        {
            /* Requires surrogate pairs for UTF-16 */
            if (unicode->length < 254)
            {
                ch -= 0x00010000;
                unicode->chars[unicode->length++] = 0xD800 | (ch >> 10);
                unicode->chars[unicode->length++] = 0xDC00 | (ch & 0x003F);
            }
            else
            {
                return ENAMETOOLONG;
            }
        }
        else
        {
            if (unicode->length < 255)
            {
                unicode->chars[unicode->length++] = ch;
            }
            else
            {
                /* No room to store character */
                return ENAMETOOLONG;
            }
        }
    }
    
    return 0;
}

static void direntry_get_short_name(const struct dosdirentry *entry, struct unistr255 *name)
{
    int i;
    unsigned ch;
    
    /*
     * Convert the base name portion to UTF-8
     */
    name->length = 0;
    for (i = 0; i < sizeof(entry->deName); ++i)
    {
        ch = entry->deName[i];
        
        /*
         * If first char of the filename is SLOT_E5 (0x05), then
         * the real first char of the filename should be 0xe5.
         * But, they couldn't just have a 0xe5 mean 0xe5 because
         * that is used to mean a freed directory slot.
         */
        if (i == 0 && ch == SLOT_E5)
            ch = 0xe5;
        
        /*
         * If the base name was supposed to be lower case,
         * then convert it.
         */
        if (entry->deLowerCase & LCASE_BASE)
            ch = latin1_to_lower[ch];   /* Map to lower case equivalent */
        
        /*
         * Convert Latin-1 to Unicode
         */
        if (ch >= 0x80 && ch <= 0x9F)
            ch = cp1252_to_unicode[ch - 0x80];
        
        name->chars[name->length++] = ch;
    }
    
    /*
     * Get rid of trailing space padding in the base name.
     */
    while (name->length && name->chars[name->length-1] == ' ')
    {
        name->length--;
    }
    
    /*
     * Convert the extension portion (with dot) to UTF-8
     */
    name->chars[name->length++] = '.';
    for (i = 0; i < sizeof(entry->deExtension); ++i)
    {
        ch = entry->deExtension[i];
        
        /*
         * If the extension was supposed to be lower case,
         * then convert it.
         */
        if (entry->deLowerCase & LCASE_EXT)
            ch = latin1_to_lower[ch];   /* Map to lower case equivalent */
        
        /*
         * Convert Latin-1 to Unicode
         */
        if (ch >= 0x80 && ch <= 0x9F)
            ch = cp1252_to_unicode[ch - 0x80];
        
        name->chars[name->length++] = ch;
    }
    
    /*
     * Get rid of trailing space padding in the extension (and the dot
     * if there was no extension).
     */
    for (i=0; i<sizeof(entry->deExtension) && name->chars[name->length-1] == ' '; ++i)
    {
        name->length--;
    }
    if (i==sizeof(entry->deExtension))  /* Was the extension entirely spaces? */
        name->length--;                 /* Yes, so remove the dot, too. */
}

/*
 * Compute the checksum of a DOS filename for Win95 use
 */
static u_int8_t msdosfsNameChecksum(u_int8_t *name)
{
    int i;
    u_int8_t s;
    
    for (s = 0, i = 0; i < SHORT_NAME_LEN; ++i, s += *name++)
        s = (s << 7)|(s >> 1);
    return s;
}

/*
 * dir_entry_enumerate
 *
 * Iterate over the 32-byte entries ("slots") in a directory, calling the
 * callback for each one.  This function does not interpret the content of
 * a directory entry; it just passes them one at a time to its callback.
 */
static errno_t dir_entry_enumerate(userfs_stream_t dir, uint64_t dirOffset, int (^callback)(uint64_t offset, struct dosdirentry *entry, bool *dirty))
{
    errno_t error = 0;
    size_t blockOffset;
    size_t dirBlockSize = dir->volume->dirBlockSize;
    userfs_buffer_t dirBlock = NULL;
    struct dosdirentry *entry;
    bool dirty = false;
    
    assert(dir->directory);
    assert((dirOffset % sizeof(struct dosdirentry)) == 0);
    
    blockOffset = dirOffset % dirBlockSize;
    while (dirOffset < dir->length)
    {
        /*
         * Don't try to read past the end of the directory.
         * This is primarily for FAT12 and FAT16 root directories, which may
         * not be a multiple of the volume's cluster size.
         */
        if (dirOffset + dirBlockSize > dir->length)
            dirBlockSize = (size_t)(dir->length - dirOffset + blockOffset);
        
        /* Get the next directory block from the cache. */
        error = stream_get_buffer(dir, dirOffset-blockOffset, dirBlockSize, &dirBlock);
        if (error)
        {
            break;
        }
        
        /* Loop over entries in the current block. */
        /* blockOffset is already set */
        for (entry = (struct dosdirentry *)((char*)buffer_bytes(dirBlock) + blockOffset);
             blockOffset < dirBlockSize;
             blockOffset += sizeof(*entry), ++entry, dirOffset += sizeof(*entry))
        {
            error = callback(dirOffset, entry, &dirty);
            if (error)
                goto done;
        }
        
        if (dirty)
            buffer_mark_dirty(dirBlock);
        cache_release_buffer(dir->volume->device, dirBlock);
        dirBlock = NULL;
        dirty = false;
        blockOffset = 0;
    }
done:
    if (dirBlock)
    {
        if (dirty)
            buffer_mark_dirty(dirBlock);
        cache_release_buffer(dir->volume->device, dirBlock);
    }
    return error;
}

/*
 * The UTF-16 code points in a long name directory entry are scattered across
 * three areas within the directory entry.  This array contains the byte offsets
 * of each UTF-16 code point, making it easier to access them in a single loop.
 */
static const uint8_t msdosfs_long_name_offset[13] = {
    1, 3, 5, 7, 9, 14, 16, 18, 20, 22, 24, 28, 30
};

/*
 * dir_enumerate
 *
 * Iterates through the directory "dir", starting at byte offset "dirOffset" from
 * the start of the directory.  Calls the callback ("callback") for each file or
 * directory entry, excluding ".", ".." and volume labels.
 *
 * The callback receives the byte offset of the item's first directory entry in
 * the "offset" parameter.  For items with a long (Unicode) name, this is the
 * offset of the first long name entry; otherwise it is the offset of the short
 * name entry.  The callback receives a pointer to the short name entry via the
 * "entry" parameter.  The callback receives the item's name via the "name"
 * parameter.  If the item has a long (Unicode) name, then "name" contains that
 * long name; otherwise, it contains the short (8.3-style) name.
 *
 * This function returns any non-zero value it receives from its callback block,
 * or zero to indicate that it iterated to the end of the directory.
 */
static errno_t dir_enumerate(userfs_stream_t dir, uint64_t dirOffset, int (^callback)(uint64_t offset, struct dosdirentry *entry, const struct unistr255 *name))
{
    errno_t error = 0;
    __block uint64_t initialOffset;     // Offset of this item's first directory entry
    __block int longNameChecksum;       // -1 if not processing long name entries
    __block int longNameIndex;      // Last long name ordinal, or 0
    __block struct unistr255 name;
    __block bool endOfDirectory = false;
    
    longNameChecksum = -1;      // No long name entries seen so far
    longNameIndex = 0;
    error = dir_entry_enumerate(dir, dirOffset, ^int(uint64_t entryOffset, struct dosdirentry *entry, bool *dirty) {
        /* Have we hit the end of the directory? */
        if (entry->deName[0] == SLOT_EMPTY)
        {
            /*
             * We need to set the endOfDirectory flag to tell that we returned
             * the USERFS_END_ENUMERATION as a result of hitting the end of
             * the directory, and that it didn't wasn't returned by our callback
             * block (because it decided to stop iteration early).
             */
            endOfDirectory = true;
            return USERFS_END_ENUMERATION;
        }
        
        /* Did we find a deleted entry? */
        if (entry->deName[0] == SLOT_DELETED)
        {
            longNameChecksum = -1;  // Stop collecting long name entries
            return 0;
        }
        
        /* Skip over "." and ".." */
        if (!memcmp(".          ", entry->deName, SHORT_NAME_LEN) ||
            !memcmp("..         ", entry->deName, SHORT_NAME_LEN))
        {
            longNameChecksum = -1;  // Stop collecting long name entries
            return 0;
        }
        
        /* If it's a long name entry, gather the characters */
        if ((entry->deAttributes & ATTR_WIN95_MASK) == ATTR_WIN95)
        {
            struct winentry *longEntry = (struct winentry*)entry;
            unsigned unicodeIndex;
            int i;
            
            if (longEntry->weCnt & WIN_LAST)
            {
                initialOffset = entryOffset;
                longNameChecksum = longEntry->weChksum;
                longNameIndex = longEntry->weCnt & WIN_CNT;
                
                // Guess at the long name's length.  If we see a NULL
                // terminator, we'll shrink the length.
                name.length = longNameIndex * WIN_CHARS;
            }
            else if (longNameChecksum != longEntry->weChksum ||
                     --longNameIndex != (longEntry->weCnt & WIN_CNT))
            {
                longNameChecksum = -1;  // Not a valid long name.
                return 0;
            }
            
            // Compute offset of this long entry within the whole Unicode name
            unicodeIndex = (longNameIndex - 1) * WIN_CHARS;
            
            // Copy the UTF-16 code points from the long name
            for (i = 0; i < WIN_CHARS; ++i)
            {
                uint8_t *cp = ((uint8_t*)entry) + msdosfs_long_name_offset[i];
                uint16_t ch = cp[0] | (cp[1] << 8);
                
                /* Did we find the NUL terminator for the name? */
                if (ch == 0)
                {
                    /* It must be in the last long name entry */
                    if (longEntry->weCnt & WIN_LAST)
                    {
                        name.length = unicodeIndex + i;
                    }
                    else
                    {
                        // Can't have NUL char in middle of long name!
                        longNameChecksum = -1;
                    }
                    break;
                }
                else
                {
                    if (unicodeIndex + i > WIN_MAXLEN)
                    {
                        /* Name too long; ignore it */
                        longNameChecksum = -1;
                        break;
                    }
                    
                    /* Store the current code point. */
                    name.chars[unicodeIndex + i] = ch;
                }
            }
            
            return 0;
        }
        
        /* Skip over volume name entries */
        if (entry->deAttributes & ATTR_VOLUME)
        {
            longNameChecksum = -1;  // Stop collecting long name entries
            return 0;
        }
        
        /*
         * If we get here, we must have found a short name entry.
         */
        if (longNameChecksum != msdosfsNameChecksum(entry->deName) ||
            longNameIndex != 1)
        {
            /* Long name didn't match short name, so ignore long name */
            initialOffset = entryOffset;
            direntry_get_short_name(entry, &name);
        }
        
        longNameChecksum = -1;  // Don't associate long name with future children
        return callback(initialOffset, entry, &name);
    });
    
    if (endOfDirectory)
        error = 0;
    return error;
}

static errno_t dir_delete_entries(userfs_stream_t dir, uint64_t dirOffset)
{
    errno_t error = dir_entry_enumerate(dir, dirOffset, ^int(uint64_t offset, struct dosdirentry *entry, bool *dirty) {
        assert(entry->deName[0] != SLOT_EMPTY && entry->deName[0] != SLOT_DELETED);
        entry->deName[0] = SLOT_DELETED;
        *dirty = true;
        if ((entry->deAttributes & ATTR_WIN95_MASK) == ATTR_WIN95)
            return 0;
        else
            return USERFS_END_ENUMERATION;
    });
    if (error == USERFS_END_ENUMERATION)
        error = 0;
    return error;
}

//
// In the directory "parent", search for a child matching the given name.  If found, create and return that child.
//
// Inputs:
//      parent              The parent directory to search
//      child_name          The UTF-8 name of the child.
//      child_name_length   The length of child_name, in bytes.
//
// Outputs:
//      child               The found child (if any) is returned here.  On error, this
//                          will be NULL.
//
// Result:  0 if the child was found
//          ENOENT if there was no child with the given name
//          (other) errno value
//
static errno_t dir_lookup(userfs_stream_t parent, const uint8_t *child_name, size_t child_name_length, userfs_stream_t *child)
{
    *child = NULL;
    
    // Make sure parent is actually a directory
    if (!parent->directory)
        return ENOTDIR;
    
    struct unistr255 searchName;
    int err;
    
    /* Convert the search name to UTF-16 */
    err = utf8_to_unistr255(child_name, child_name_length, &searchName);
    if (err)
        return err;
    
    /* Iterate through the directory looking for a short name match */
    err = dir_enumerate(parent, 0, ^int(uint64_t offset, struct dosdirentry *entry, const struct unistr255 *name) {
        /* Compare names here. */
        if (searchName.length != name->length)
            return 0;
        for (int i = 0; i < searchName.length; ++i)
        {
            uint16_t ch1, ch2;
            ch1 = searchName.chars[i];
            if (ch1 < 0x100)
                ch1 = latin1_to_lower[ch1];
            ch2 = name->chars[i];
            if (ch2 < 0x100)
                ch2 = latin1_to_lower[ch2];
            if (ch1 != ch2)
                return 0;
        }
        
        /* Found it! */
        char utf8[FAT_MAX_FILENAME_UTF8];
        unistr255_to_utf8(name, utf8);
        userfs_volume_t volume = parent->volume;
        uint32_t cluster = getuint16(entry->deStartCluster);
        if (volume->fatMask == FAT32_MASK)
            cluster |= getuint16(entry->deHighClust) << 16;
        *child = volume_create_stream(volume, utf8, cluster, getuint32(entry->deFileSize));
        (*child)->dirEntryOffset = offset;
        (*child)->create_date = timestamp_to_nanoseconds(getuint16(entry->deCDate), getuint16(entry->deCTime), entry->deCHundredth);
        (*child)->mod_date = timestamp_to_nanoseconds(getuint16(entry->deMDate), getuint16(entry->deMTime), 0);
        if (entry->deAttributes & ATTR_DIRECTORY)
        {
            (*child)->directory = true;
            
            /* Calculate the length of the directory */
            uint32_t numClusters = 0;
            int err2 = chain_length(volume, cluster, &numClusters);
            if (err2)
            {
                msdosfs_stream_close(*child);
                *child = NULL;
                return err2;
            }
            (*child)->length = (uint64_t)numClusters * volume->bytesPerCluster;
        }
        if (entry->deAttributes & ATTR_READONLY)
            (*child)->locked = true;
        return USERFS_END_ENUMERATION;
    });
    
    if (err == USERFS_END_ENUMERATION)
        err = 0;
    else if (err == 0)
        err = ENOENT;
    
    return err;
    
}

static errno_t stream_get_physical_extent(userfs_stream_t stream, size_t logical_length, off_t logical_offset, size_t *physical_length, off_t *physical_offset)
{
    errno_t error = 0;
    
    assert(logical_length != 0);
    
    uint32_t logical_cluster = (uint32_t)(logical_offset / stream->volume->bytesPerCluster);
    
    /*
     * If the desired offset is earlier than the cached extent, reset the cached
     * extent to point to the start of the file.
     */
    if (logical_cluster < stream->cachedLogicalCluster)
    {
        stream->cachedLogicalCluster = 0;
        stream->cachedClusterCount = 0;
        stream->cachedNextCluster = stream->startingCluster;
    }
    
    /*
     * Search forward through the FAT chain, one extent at a time.
     */
    while (stream->cachedNextCluster >= CLUST_FIRST &&
           stream->cachedNextCluster <= stream->volume->maxCluster &&
           logical_cluster >= (stream->cachedLogicalCluster + stream->cachedClusterCount))
    {
        stream->cachedLogicalCluster += stream->cachedClusterCount;
        stream->cachedPhysicalCluster = stream->cachedNextCluster;
        stream->cachedClusterCount = contiguous_clusters_in_chain(stream->volume, stream->cachedPhysicalCluster, &stream->cachedNextCluster, &error);
        // TODO: What if the above failed?  Check for error?
    }
    
    /* Sanity check the current extent. */
    if (stream->cachedPhysicalCluster < CLUST_FIRST ||
        stream->cachedPhysicalCluster > stream->volume->maxCluster ||
        stream->cachedClusterCount > stream->volume->maxCluster - stream->cachedPhysicalCluster)
    {
        MSDOSFS_LOG_ERR(msdosfs_log_default, "%s: Invalid extent (start=%u, count=%u, max=%u)\n", device_name(stream->volume->device), stream->cachedPhysicalCluster, stream->cachedClusterCount, stream->volume->maxCluster);
        stream->cachedClusterCount = 0;
        error = EIO;
    }
    
    if (stream->cachedClusterCount == 0)
    {
        /* Got an error reading the FAT.  error is set. */
        assert(error != 0);
        stream->cachedLogicalCluster = 0;
        stream->cachedClusterCount = 0;
        stream->cachedNextCluster = stream->startingCluster;
        return error;
    }
    
    if (logical_cluster >= stream->cachedLogicalCluster && logical_cluster < (stream->cachedLogicalCluster + stream->cachedClusterCount))
    {
        /* The given offset is in the currently cached extent, so we can just return it. */
        uint64_t cachedOffset = (uint64_t)stream->cachedLogicalCluster * stream->volume->bytesPerCluster;
        uint64_t cachedLength = (uint64_t)stream->cachedClusterCount * stream->volume->bytesPerCluster;
        uint64_t cachedEnd = cachedOffset + cachedLength;
        
        /* Don't go beyond end of currently cached extent. */
        if (logical_length > cachedEnd - logical_offset)
            logical_length = (size_t)(cachedEnd - logical_offset);
        
        *physical_offset = volume_offset_for_cluster(stream->volume, stream->cachedPhysicalCluster) + logical_offset - cachedOffset;
        *physical_length = logical_length;
        return 0;
    }
    
    /* If we get here, there wasn't enough cluster chain to map the given offset. */
    if (stream->cachedNextCluster >= (CLUST_EOFS & stream->volume->fatMask))
    {
        MSDOSFS_LOG_ERR(msdosfs_log_default, "%s: Cluster chain is too short\n", device_name(stream->volume->device));
    }
    else
    {
        MSDOSFS_LOG_ERR(msdosfs_log_default, "%s: Invalid cluster number in chain (%u)\n", device_name(stream->volume->device), stream->cachedNextCluster);
    }
    
    return EIO;
}

static errno_t stream_get_buffer(userfs_stream_t stream, uint64_t offset, size_t length, userfs_buffer_t *buffer)
{
    userfs_volume_t volume = stream->volume;
    size_t physical_length;
    off_t physical_offset;
    
    /*
     * stream->fixedRoot is true if and only if the stream is the root directory
     * of a FAT12 or FAT16 volume.  In this case, the root directory occupies a
     * fixed range of sectors preceding the volume's clusters.
     */
    if (stream->fixedRoot)
    {
        assert((offset % volume->bytesPerSector) == 0);
        assert((length % volume->bytesPerSector) == 0);
        assert(offset < ((uint64_t)volume->rootSize * volume->bytesPerSector));
        assert(offset + length <= ((uint64_t)volume->rootSize * volume->bytesPerSector));
        physical_offset = (off_t)volume->rootSector * volume->bytesPerSector + offset;
        return cache_get_buffer(volume->device, physical_offset, length, buffer);
    }
    
    /*
     * Compute the item's physical size (its size, rounded up to a multiple of the cluster size).
     * We use that for the assertions below so that we can get a buffer that extents beyond
     * the file's logical size, but not beyond the physical space allocated to it.
     */
    uint64_t roundedLength = roundup(stream->length, volume->bytesPerCluster);
    
    if (offset > roundedLength)
    {
        MSDOSFS_LOG_ERR(msdosfs_log_default, "%s: offset > self.length (%llu > %llu)\n", __PRETTY_FUNCTION__, offset, stream->length);
        return EINVAL;
    }
    if (length > (roundedLength - offset))
    {
        MSDOSFS_LOG_ERR(msdosfs_log_default, "%s: offset + length > self.length (%llu + %zu > %llu)\n", __PRETTY_FUNCTION__, offset, length, stream->length);
        return EINVAL;
    }
    if (length == 0)
    {
        MSDOSFS_LOG_ERR(msdosfs_log_default, "%s: length == 0\n", __PRETTY_FUNCTION__);
        return EINVAL;
    }
    
    errno_t error = stream_get_physical_extent(stream, length, offset, &physical_length, &physical_offset);
    if (error)
    {
        return error;
    }
    if (physical_length != length)
    {
        MSDOSFS_LOG_ERR(msdosfs_log_default, "%s: physical_length != length (%zu != %zu)\n", __PRETTY_FUNCTION__, physical_length, length);
        return EINVAL;
    }
    
    return cache_get_buffer(volume->device, physical_offset, physical_length, buffer);
}

static errno_t stream_get_read_data(userfs_stream_t stream, uint64_t offset, size_t length, void *buffer)
{
    errno_t error = 0;
    size_t physical_length = 0;
    off_t physical_offset = 0;
    off_t cache_offset = 0;
    size_t block_offset = 0;
    bool found_cache_data = false;
    void* temp_buf = NULL;
    userfs_contentbuffer_t buf = NULL;
    size_t tail_length = 0;

    pthread_mutex_t* readahead_mutex = device_mutex(stream->volume->device);

    assert(readahead_mutex != NULL);

    while (length > 0)
    {
        pthread_mutex_lock(readahead_mutex);

        error = stream_get_physical_extent(stream, length, offset, &physical_length, &physical_offset);
        if (error)
        {
            goto done;
        }

        block_offset = physical_offset & UNALIGNED_BLOCK_MASK;
        cache_offset = physical_offset - block_offset;
        tail_length = UNALIGNED_BLOCK_SIZE - block_offset;

        if (tail_length > length)
            tail_length = length;

        /* Check if the data is in cache. */
        found_cache_data = cache_get_content_buffer(stream->volume->device, &buf, cache_offset, false);

        if (found_cache_data)
        {
            /* Its a cache hit, copy the data from the cache buffer. */
            if(buf == NULL)
            {
                MSDOSFS_LOG_ERR(msdosfs_log_default, "%s: buffer is null",__PRETTY_FUNCTION__);
                error = EIO;
                goto done;
            }

            temp_buf = content_buffer_bytes(buf);

            if (temp_buf == NULL)
            {
                MSDOSFS_LOG_ERR(msdosfs_log_default, "%s: temp_buf is null",__PRETTY_FUNCTION__);
                error = EIO;
                goto done;
            }

            memcpy(buffer,&temp_buf[block_offset],tail_length);

            offset += tail_length;
            length -= tail_length;
            buffer = (char *)buffer + tail_length;

            pthread_mutex_unlock(readahead_mutex);
        }
        else
        {
            /* Its a cache miss, perform the actual io. */
            MSDOSFS_LOG_ERR(msdosfs_log_default, "cache-miss: offset <%llu> length %zu",offset, length);
			pthread_mutex_unlock(readahead_mutex);

            error = volume_read(stream->volume, buffer, physical_length, physical_offset);
            if (error)
            {
                return error;
            }

            offset += physical_length;
            length -= physical_length;
            buffer = (char *)buffer + physical_length;
        }
    }

done:
    if(error)
    {
        pthread_mutex_unlock(readahead_mutex);
    }
    return error;

}

static errno_t stream_get_read_ahead_data(userfs_stream_t stream, uint64_t offset, size_t length)
{
    errno_t error = 0;
	off_t block_offset = 0;
    off_t physical_offset = 0;
    off_t cache_offset = 0;
	off_t tail_length = 0;
    size_t physical_length = 0;
    size_t extent_size = 0;
    struct iovec *iov = NULL;
    userfs_contentbuffer_t *buffer_list = NULL;
    int iter = 0;
    int i = 0;
    int num_blocks = 0;
    uint64_t read_physical_offset = 0;
    userfs_contentbuffer_t buf = NULL;
    void* temp = NULL;

    int fd = device_fd(stream->volume->device);

    pthread_mutex_t* readahead_mutex = device_mutex(stream->volume->device);

    assert(readahead_mutex != NULL);

    pthread_mutex_lock(readahead_mutex);

    /* If a close has been issued by the client on the stream
     * do not bother to read-ahead instead close the stream
     * when the thread count is zero.
     */
    if(stream->close_lazily)
    {
        error = EIO;
        MSDOSFS_LOG(msdosfs_log_default, "close lazily, thread count is %d",stream->thread_count);
        if(!stream->thread_count)
        {
            pthread_mutex_unlock(readahead_mutex);
            msdosfs_stream_close(stream);
            return error;
        }
        goto done;
    }

	/* Length passed in by the daemon is cache block aligned. */
	assert((length % (CONTENT_BUFFER_SIZE)) == 0);

	/* Loop over the extents. */
    while (length > 0)
    {
		error = stream_get_physical_extent(stream, length, offset, &physical_length, &physical_offset);

        if (error)
        {
            goto done;
        }

        num_blocks = ((int)physical_length/(CONTENT_BUFFER_SIZE))+1;

        iov = (struct iovec *) malloc(num_blocks * (sizeof (struct iovec)));
        buffer_list = (userfs_contentbuffer_t *) malloc(num_blocks * (sizeof (userfs_contentbuffer_t)));

        extent_size = physical_length;
        iter = 0;
        i = 0;

        while ((extent_size > 0) && (length > 0))
        {
            /* Round off the physical offset so that i/os are always cache block aligned */
			block_offset = physical_offset & (CONTENT_BUFFER_SIZE - 1);
            cache_offset = physical_offset - block_offset;
			tail_length = CONTENT_BUFFER_SIZE - block_offset;

            if (iter == 0)
            {
                /* Save the starting offset to pass it to lseek. */
                read_physical_offset = cache_offset;
            }

			if (!cache_get_content_buffer(stream->volume->device, &buf, cache_offset, true))
			{
				MSDOSFS_LOG_ERR(msdosfs_log_default, "%s: could not get buffer from buffer pool",__PRETTY_FUNCTION__);
				error = EIO;
				goto done;
			}

			/* Set up the iov for readv. */
            assert (iter < num_blocks);
			assert (buf != NULL);
			buffer_list[iter] = buf;
			temp = content_buffer_bytes(buf);
            assert(temp != NULL);
            iov[iter].iov_base = temp;
            iov[iter++].iov_len = CONTENT_BUFFER_SIZE;

            offset += MIN(tail_length, extent_size);
            physical_offset += tail_length;
			length -= CONTENT_BUFFER_SIZE;
			extent_size = (extent_size > tail_length ? (size_t) (extent_size - tail_length) : 0);

        }

		lseek(fd, read_physical_offset, SEEK_SET);
        if ((readv(fd, iov, iter)) == -1)
        {
            MSDOSFS_LOG_ERR(msdosfs_log_default, "%s: readv failed with errno: %d", __FUNCTION__, errno);
        }
        else
        {
            /* readv successfully completed, populate the content buffer cache. */
            while(i < iter)
            {
                populate_content_buffer_cache(stream->volume->device, buffer_list[i]);
                i++;
            }
        }

        free(iov);
        free(buffer_list);
        iov = NULL;
        buffer_list = NULL;

    }

done:

    if (offset > stream->last_offset_in_cache)
    {
        stream->last_offset_in_cache = offset;
    }

    if(iov != NULL)
        free(iov);
    if(buffer_list != NULL)
        free(buffer_list);

    stream->thread_count--;
    pthread_mutex_unlock(readahead_mutex);

    return error;
}

static errno_t msdosfs_volume_open(userfs_device_t device, bool locked, userfs_volume_t *volume)
{
    userfs_volume_t v;
    userfs_buffer_t bootSector = NULL;
    errno_t err = 0;
    uint32_t bytesPerSector;
    bool tryRepair = true;
    
    v = calloc(1, sizeof(struct _userfs_volume_s));
    if (v == NULL)
        return ENOMEM;
    *volume = v;
    
    v->device = device;
    v->locked = locked;
    
    /* Sanity check the device's logical block size. */
    if (ioctl(device_fd(device), DKIOCGETBLOCKSIZE, &bytesPerSector) < 0)
    {
        err = errno;
        MSDOSFS_LOG_ERR(msdosfs_log_default, "%s: ioctl(DKIOCGETBLOCKSIZE) failed, %s\n", device_name(device), strerror(err));
        goto fail;
    }
    if (bytesPerSector > MAX_BLOCK_SIZE)
    {
        err = EIO;
        MSDOSFS_LOG_ERR(msdosfs_log_default, "%s: block size is too big (%lu)\n", device_name(device), (unsigned long) bytesPerSector);
        goto fail;
    }
    
    /*
     * Read in the boot sector.
     */
read_boot_sector:
    err = cache_get_buffer(device, 0, bytesPerSector, &bootSector);
    if (err)
    {
        assert(bootSector == NULL);
        goto fail;
    }
    union bootsector *boot = buffer_bytes(bootSector);
    struct byte_bpb50 *b50 = (struct byte_bpb50 *)boot->bs50.bsBPB;
    struct byte_bpb710 *b710 = (struct byte_bpb710 *)boot->bs710.bsBPB;
    
    /*
     * The first three bytes are an Intel x86 jump instruction.  Windows only
     * checks the first byte, so that's what we'll do, too.
     */
    if (boot->bs50.bsJump[0] != 0xE9 &&
        boot->bs50.bsJump[0] != 0xEB)
    {
        err = EINVAL;
        MSDOSFS_LOG_ERR(msdosfs_log_default, "%s: invalid jump signature (0x%02X)\n", device_name(device), boot->bs50.bsJump[0]);
        goto fail;
    }
    
    /* Check the trailing "boot signature" */
    if (boot->bs50.bsBootSectSig0 != BOOTSIG0 ||
        boot->bs50.bsBootSectSig1 != BOOTSIG1)
    {
        err = EINVAL;
        MSDOSFS_LOG_ERR(msdosfs_log_default, "%s: invalid boot signature (0x%02X 0x%02X)\n", device_name(device), boot->bs50.bsBootSectSig0, boot->bs50.bsBootSectSig1);
    }
    
    /* Compute several useful quantities from the boot sector. */
    v->bytesPerSector = getuint16(b50->bpbBytesPerSec);
    unsigned sectorsPerCluster = b50->bpbSecPerClust;
    v->bytesPerCluster = v->bytesPerSector * sectorsPerCluster;
    uint32_t reservedSectors = getuint16(b50->bpbResSectors);
    uint32_t rootEntryCount = getuint16(b50->bpbRootDirEnts);
    uint32_t totalSectors = getuint16(b50->bpbSectors);
    if (totalSectors == 0)
        totalSectors = getuint32(b50->bpbHugeSectors);
    uint32_t fatSectors = getuint16(b50->bpbFATsecs);
    if (fatSectors == 0)
        fatSectors = getuint32(b710->bpbBigFATsecs);
    
    /*
     * Check a few values (could do some more):
     * - logical sector size: == device's current sector size
     * - sectors per cluster: power of 2, >= 1
     * - number of sectors:   >= 1
     * - number of FAT sectors > 0 (too large values handled later)
     */
    if (v->bytesPerSector != bytesPerSector)
    {
        err = EINVAL;
        MSDOSFS_LOG_ERR(msdosfs_log_default, "%s: logical sector size (%u) != physical sector size (%u)\n", device_name(device), v->bytesPerSector, bytesPerSector);
        goto fail;
    }
    if (sectorsPerCluster == 0 || (sectorsPerCluster & (sectorsPerCluster - 1)))
    {
        err = EINVAL;
        MSDOSFS_LOG_ERR(msdosfs_log_default, "%s: invalid sectors per cluster (%u)\n", device_name(device), sectorsPerCluster);
        goto fail;
    }
    if (totalSectors == 0)
    {
        err = EINVAL;
        MSDOSFS_LOG_ERR(msdosfs_log_default, "%s: invalid total sectors (%u)\n", device_name(device), totalSectors);
        goto fail;
    }
    if (fatSectors == 0)
    {
        err = EINVAL;
        MSDOSFS_LOG_ERR(msdosfs_log_default, "%s: invalid sectors per FAT (%u)\n", device_name(device), fatSectors);
        goto fail;
    }
    
    /*
     * For now, assume a FAT12 or FAT16 volume with a dedicated root directory.
     * We need these values before we can figure out how many clusters (and
     * thus whether the volume is FAT32 or not).  Start by calculating sectors.
     */
    v->rootSector = reservedSectors + (b50->bpbFATs * fatSectors);
    v->rootSize = (rootEntryCount * sizeof(struct dosdirentry) + bytesPerSector - 1) / bytesPerSector;
    v->clusterOffset = v->rootSector + v->rootSize;
    
    if (fatSectors > totalSectors ||
        v->rootSector < fatSectors ||           /* Catch numeric overflow! */
        v->clusterOffset + sectorsPerCluster > totalSectors)
    {
        /* We think there isn't room for even a single cluster. */
        err = EINVAL;
        MSDOSFS_LOG_ERR(msdosfs_log_default, "%s: invalid configuration; no room for clusters\n", device_name(device));
        goto fail;
    }
    
    /*
     * Usable clusters are numbered starting at 2, so the maximum usable cluster
     * is (number of clusters) + 1.  Convert the pm_firstcluster to device blocks.
     */
    v->maxCluster = (totalSectors - v->clusterOffset) / sectorsPerCluster + 1;
    
    /*
     * Figure out the FAT type based on the number of clusters.
     */
    if (v->maxCluster < (CLUST_RSRVD & FAT12_MASK))
    {
        v->fatMask = FAT12_MASK;
        v->fatEntryOffset = fat12EntryOffset;
        v->fatEntryGet = fat12EntryGet;
        v->fatEntrySet = fat12EntrySet;
    }
    else if (v->maxCluster < (CLUST_RSRVD & FAT16_MASK))
    {
        v->fatMask = FAT16_MASK;
        v->fatEntryOffset = fat16EntryOffset;
        v->fatEntryGet = fat16EntryGet;
        v->fatEntrySet = fat16EntrySet;
    }
    else if (v->maxCluster < (CLUST_RSRVD & FAT32_MASK))
    {
        v->fatMask = FAT32_MASK;
        v->fatEntryOffset = fat32EntryOffset;
        v->fatEntryGet = fat32EntryGet;
        v->fatEntrySet = fat32EntrySet;
    }
    else
    {
        err = EINVAL;
        MSDOSFS_LOG_ERR(msdosfs_log_default, "%s: number of clusters (0x%x) is too large\n", device_name(device), v->maxCluster + 1);
        goto fail;
    }
    
    // TODO: Check b710->bpbExtFlags to see if FAT mirroring is on, and which
    // copy of the FAT is currently active.
    v->fatOffset = reservedSectors * bytesPerSector;
    v->fatSize = fatSectors * bytesPerSector;
    
    /*
     * If the volume is dirty, run fsck_msdos to repair it.
     *
     * FAT12 is always assumed to be dirty.  For FAT16 and FAT32, we examine the
     * FAT entry for cluster #1.
     */
    if (tryRepair)
    {
        bool dirty = true;
        if (v->fatMask == FAT12_MASK)
        {
            dirty = true;
        }
        else
        {
            /* Read in the FAT entry for cluster #1.  Get the "clean shut down" bit. */
            userfs_buffer_t fatBuffer = NULL;
            uint8_t *entry;
            off_t entryOffset = v->fatEntryOffset(1);
            size_t blockSize = fat_block_size(v, 0);
            
            /* Read the block from through the cache */
            err = cache_get_buffer(v->device, v->fatOffset, blockSize, &fatBuffer);
            if (err)
            {
                MSDOSFS_LOG_ERR(msdosfs_log_default, "%s: Unable to read FAT[1] (error %d)\n", device_name(device), err);
                goto fail;
            }
            assert(buffer_size(fatBuffer) == blockSize);
            entry = (uint8_t *)buffer_bytes(fatBuffer) + entryOffset;
            
            /* The flag in FAT[1] is set if the volume is clean, so invert for dirty */
            uint32_t fatFlags = v->fatEntryGet(1, &entry);
            dirty = !(fatFlags & (v->fatMask == FAT16_MASK ? 0x00008000 : 0x08000000));
            
            cache_release_buffer(v->device, fatBuffer);
        }
        
        if (dirty)
        {
            tryRepair = false;
            if (v->locked)
            {
                MSDOSFS_LOG_ERR(msdosfs_log_default, "%s: Volume is %s dirty and locked.  Will use read-only.\n", device_name(device), v->fatMask == FAT12_MASK ? "assumed" : "marked");
            }
            else
            {
                MSDOSFS_LOG_INFO(msdosfs_log_default, "%s: Volume is %s dirty.  Attempting to repair.\n", device_name(device), v->fatMask == FAT12_MASK ? "assumed" : "marked");
                
                cache_release_buffer(device, bootSector);
                bootSector = NULL;
                cache_invalidate(device);
                
                int repair_result = fsck_msdos(device_fd(device), device_name(device));
                if (repair_result)
                {
                    MSDOSFS_LOG_ERR(msdosfs_log_default, "%s: Could not be repaired (status %d).  Will use read-only.\n", device_name(device), repair_result);
                    v->locked = true;
                }
                else
                {
                    MSDOSFS_LOG_INFO(msdosfs_log_default, "%s: Volume repaired.\n", device_name(device));
                }
                
                goto read_boot_sector;
            }
        }
    }
    
    /*
     * Sanity check some differences between FAT32 and FAT12/16.
     * Also set up FAT32's root directory and FSInfo sector.
     */
    if (v->fatMask == FAT32_MASK)
    {
        v->fsInfoSector = getuint16(b710->bpbFSInfo);
        v->rootCluster = getuint32(b710->bpbRootClust);
        if (v->rootCluster < CLUST_FIRST || v->rootCluster > v->maxCluster)
        {
            err = EINVAL;
            MSDOSFS_LOG_ERR(msdosfs_log_default, "%s: FAT32 root starting cluster (%u) out of range (%u..%u)\n",
                    device_name(device), v->rootCluster, CLUST_FIRST, v->maxCluster);
            goto fail;
        }
        if (rootEntryCount)
        {
            err = EINVAL;
            MSDOSFS_LOG_ERR(msdosfs_log_default, "%s: FAT32 has non-zero root directory entry count\n", device_name(device));
            goto fail;
        }
        if (getuint16(b710->bpbFSVers) != 0)
        {
            err = EINVAL;
            MSDOSFS_LOG_ERR(msdosfs_log_default, "%s: FAT32 has non-zero version\n", device_name(device));
            goto fail;
        }
        if (getuint16(b50->bpbSectors) != 0)
        {
            err = EINVAL;
            MSDOSFS_LOG_ERR(msdosfs_log_default, "%s: FAT32 has 16-bit total sectors\n", device_name(device));
            goto fail;
        }
        if (getuint16(b50->bpbFATsecs) != 0)
        {
            err = EINVAL;
            MSDOSFS_LOG_ERR(msdosfs_log_default, "%s: FAT32 has 16-bit FAT sectors\n", device_name(device));
            goto fail;
        }
    }
    else
    {
        if (rootEntryCount == 0)
        {
            err = EINVAL;
            MSDOSFS_LOG_ERR(msdosfs_log_default, "%s: FAT12/16 has zero-length root directory\n", device_name(device));
            goto fail;
        }
        if (totalSectors < 0x10000 && getuint16(b50->bpbSectors) == 0)
        {
            MSDOSFS_LOG_ERR(msdosfs_log_default, "%s: FAT12/16 total sectors (%u) fit in 16 bits, but stored in 32 bits\n", device_name(device), totalSectors);
        }
        if (getuint16(b50->bpbFATsecs) == 0)
        {
            MSDOSFS_LOG_ERR(msdosfs_log_default, "%s: FAT12/16 has 32-bit FAT sectors\n", device_name(device));
        }
    }
    
    /*
     * Compute number of clusters this FAT could hold based on its total size.
     * Pin the maximum cluster number to the size of the FAT.
     *
     * NOTE: We have to do this AFTER determining the FAT type.  If we did this
     * before, we could end up deducing a different FAT type than what's actually
     * on disk, and that would be very bad.
     */
    uint32_t clusters = fatSectors * v->bytesPerSector;     /* Size of FAT in bytes */
    if (v->fatMask == FAT32_MASK)
        clusters /= 4;                                      /* FAT32: 4 bytes per FAT entry */
    else if (v->fatMask == FAT16_MASK)
        clusters /= 2;                                      /* FAT16: 2 bytes per FAT entry */
    else
        clusters = clusters * 2 / 3;                        /* FAT12: 3 bytes for every two FAT entries */
    if (v->maxCluster >= clusters) {
        MSDOSFS_LOG_ERR(msdosfs_log_default, "%s: Number of clusters (%d) exceeds FAT capacity (%d)\n",
                device_name(device), v->maxCluster + 1, clusters);
        v->maxCluster = clusters - 1;
    }
    
    /*
     * Pin the maximum cluster number based on the number of sectors the device
     * is reporting (in case that is smaller than the total sectors field(s)
     * in the boot sector).
     */
    uint64_t blockCount;
    err = ioctl(device_fd(device), DKIOCGETBLOCKCOUNT, &blockCount);
    if (err == 0 && blockCount < totalSectors)
    {
        if (v->clusterOffset + sectorsPerCluster > blockCount)
        {
            err = EINVAL;
            MSDOSFS_LOG_ERR(msdosfs_log_default, "%s: device sector count (%llu) too small; no room for clusters\n", device_name(device), blockCount);
            goto fail;
        }
        
        uint32_t maxcluster = (uint32_t)((blockCount - v->clusterOffset) / sectorsPerCluster + 1);
        if (maxcluster < v->maxCluster)
        {
            MSDOSFS_LOG_ERR(msdosfs_log_default, "%s: device sector count (%llu) is less than volume sector count (%u); limiting maximum cluster to %u (was %u)\n",
                    device_name(device), blockCount, totalSectors, maxcluster, v->maxCluster);
            v->maxCluster = maxcluster;
        }
        else
        {
            MSDOSFS_LOG_ERR(msdosfs_log_default, "msdosfs_mount: device sector count (%llu) is less than volume sector count (%u)\n",
                    blockCount, totalSectors);
        }
    }
    
    /*
     * Compute the size (in bytes) of the volume root directory.
     */
    if (v->fatMask == FAT32_MASK)
    {
        uint32_t numClusters = 0;
        err = chain_length(v, v->rootCluster, &numClusters);
        if (err) goto fail;
        v->rootLength = (uint64_t)numClusters * v->bytesPerCluster;
    }
    else
    {
        v->rootLength = v->rootSize * v->bytesPerSector;
    }
    
    /*
     * Calculate the amount to read from a directory at one time.  Default to
     * one cluster, but pin to no more than 128KiB.
     */
    v->dirBlockSize = v->bytesPerCluster;
    if (v->dirBlockSize > MAX_DIR_BLOCK_SIZE)
        v->dirBlockSize = MAX_DIR_BLOCK_SIZE;
    
    /*
     * If this is a FAT32 volume, try to read in the FSInfo sector and fetch
     * the free cluster count.
     */
    if (v->fsInfoSector)
    {
        userfs_buffer_t fsInfoBuffer = NULL;
        err = cache_get_buffer(device, v->fsInfoSector*v->bytesPerSector, v->bytesPerSector, &fsInfoBuffer);
        if (err)
        {
            MSDOSFS_LOG_ERR(msdosfs_log_default, "%s: Error %d trying to read FSInfo sector; ignoring.\n", device_name(device), err);
            v->fsInfoSector = 0;
            err = 0;
        }
        else
        {
            struct fsinfo *fsInfo = buffer_bytes(fsInfoBuffer);
            if (!bcmp(fsInfo->fsisig1, "RRaA", 4) &&
                !bcmp(fsInfo->fsisig2, "rrAa", 4) &&
                !bcmp(fsInfo->fsisig3, "\0\0\125\252", 4))
            {
                v->freeClusters = getuint32(fsInfo->fsinfree);
                
                /*
                 * Sanity check the free cluster count.  If it is bigger than the
                 * total number of clusters (maxCluster - 1), then ignore it.
                 */
                if (v->freeClusters > v->maxCluster-1)
                    v->fsInfoSector = 0;
            }
            else
            {
                MSDOSFS_LOG_ERR(msdosfs_log_default, "%s: FSInfo sector has invalid signature(s); ignoring.\n", device_name(device));
                v->fsInfoSector = 0;
            }
        }
        if (fsInfoBuffer)
            cache_release_buffer(device, fsInfoBuffer);
    }
    
    // TODO: Create stream(s) for the FAT (active & mirror)?
    
    // TODO: Scan the root directory looking for a Windows hibernation image.
    //       If found, make the volume read-only.
    
    if (bootSector)
        cache_release_buffer(device, bootSector);
    
    return 0;
    
fail:
    assert(err != 0);
    if (bootSector)
        cache_release_buffer(device, bootSector);
    if (v)
    {
        free(v);
        *volume = NULL;
    }
    return err;
}

static bool msdosfs_volume_is_locked(userfs_volume_t volume)
{
    return volume->locked;
}

static errno_t msdosfs_volume_mark_dirty(userfs_volume_t volume, bool dirty)
{
    /*
     * Fast path: not changing state (marking a dirty volume dirty, or a clean
     * volume clean).  Just return.
     */
    if (dirty == volume->dirty)
        return 0;
    
    /* FAT12 does not support a "clean" bit, so don't do anything */
    if (volume->fatMask == FAT12_MASK)
        return 0;
    
    errno_t error = 0;
    userfs_buffer_t buffer = NULL;
    uint8_t *entry;
    off_t entryOffset;
    size_t blockSize;
    uint32_t mask;
    uint32_t flags;
    
    /* Figure out the FAT block containing FAT[1] */
    entryOffset = volume->fatEntryOffset(1);
    blockSize = fat_block_size(volume, 0);
    
    /* Figure out which bit in the FAT entry needs to change */
    if (volume->fatMask == FAT32_MASK)
        mask = 0x08000000;      // FAT32 uses bit #27
    else
        mask = 0x8000;          // FAT16 uses bit #15
    
    /* Fetch the cache block containing FAT[1] */
    error = cache_get_buffer(volume->device, volume->fatOffset, blockSize, &buffer);
    if (error)
    {
        MSDOSFS_LOG_ERR(msdosfs_log_default, "%s: msdosfs_volume_mark_dirty: unable to get FAT buffer for cluster=1 (error %d)\n", device_name(volume->device), error);
        goto done;
    }
    
    /* Update the "clean" bit */
    entry = (uint8_t *)buffer_bytes(buffer) + entryOffset;
    flags = volume->fatEntryGet(1, &entry);
    if (dirty)
        flags &= ~mask;
    else
        flags |= mask;
    entry = (uint8_t *)buffer_bytes(buffer) + entryOffset;
    volume->fatEntrySet(1, &entry, flags);
    
    /* Write the entry back to the FAT immediately */
    buffer_mark_dirty(buffer);
    error = cache_flush_buffer(volume->device, buffer);
    cache_release_buffer(volume->device, buffer);
    buffer = NULL;
    if (error)
    {
        MSDOSFS_LOG_ERR(msdosfs_log_default, "%s: msdosfs_volume_mark_dirty: error %d writing FAT\n", device_name(volume->device), error);
        goto done;
    }
    if (fsync(device_fd(volume->device)))
    {
        error = errno;
        MSDOSFS_LOG_ERR(msdosfs_log_default, "%s: msdosfs_volume_mark_dirty: error %d from fsync\n", device_name(volume->device), error);
        goto done;
    }
    if (ioctl(device_fd(volume->device), DKIOCSYNCHRONIZECACHE) == -1)
    {
        error = errno;
        MSDOSFS_LOG_ERR(msdosfs_log_default, "%s: msdosfs_volume_mark_dirty: error %d from DKIOCSYNCHRONIZECACHE\n", device_name(volume->device), error);
        goto done;
    }
    
done:
    if (!error)
        volume->dirty = dirty;
    return error;
}

static errno_t msdosfs_volume_flush(userfs_volume_t volume)
{
    errno_t err;
    
    err = cache_flush(volume->device);
    if (err == 0)
    {
        /* Mark the volume clean (force write) */
        err = msdosfs_volume_mark_dirty(volume, false);
    }
    return err;
}

static errno_t msdosfs_volume_close(userfs_volume_t volume)
{
    // Free any member pointers
    free(volume);
    return 0;
}

static errno_t msdosfs_item_get_info(userfs_volume_t volume, const char *path, xpc_object_t info)
{
    errno_t err;
    userfs_stream_t item = NULL;
    
    err = volume_lookup(volume, path, &item, NULL);
    if (err == 0)
    {
        xpc_dictionary_set_bool(info, USERFSD_PARM_FOLDER, item->directory);
        xpc_dictionary_set_bool(info, USERFSD_PARM_LOCKED, item->locked);
        xpc_dictionary_set_uint64(info, USERFSD_PARM_LENGTH, item->length);
        xpc_dictionary_set_date(info, USERFSD_PARM_CREATE_DATE, item->create_date);
        xpc_dictionary_set_date(info, USERFSD_PARM_MOD_DATE, item->mod_date);
        xpc_dictionary_set_string(info, USERFSD_PARM_NAME, item->name);
        msdosfs_stream_close(item);
    }
    
    return err;
}

/*
 * Return information about the contents of a given directory.
 *
 * Inputs:
 *  volume      The volume containing the parent directory.
 *  path        Pathname to the parent directory.  (This is relative to the volume root.)
 *
 * Input/Output:
 *  *state      Indicates where within the parent directory to resume enumeration.
 *              Use NULL to start enumerating with the first item in the directory; otherwise,
 *              use an object returned from a prior call to this function (with the same
 *              volume and path).
 *
 *              On return, this will contain NULL if there was an error, or if the end of
 *              the directory was reached.  Otherwise, it will be an XPC object that can
 *              be passed in to resume enumeration.
 *
 *  children    An existing XPC array (created with xpc_array_create) that will contain the
 *              returned items.  The returned items are appended to the array.
 *
 * For each item in the directory, a dictionary will be appended to the "children" array.
 * The dictionary contains the same key/value pairs as returned by the item_get_info callback.
 *
 * This routine limits the number of items returned in a single call.  This is done to limit
 * latency, and to limit the amount of memory that needs to be copied between processes
 * for a single XPC message.  Empirically, 10 items (or a little fewer) was enough to
 * achieve good performance; increasing the value did not significantly increase performance.
 * Having a relatively small number of items per message aided debugging, and probably
 * helps limit client memory use.
 *
 * Note: in case of error, some items may have been appended to "children".
 * TODO: in case of error, should we remove all items from "children"?
 */
static errno_t msdosfs_dir_enumerate(userfs_volume_t volume, const char *path, __strong xpc_object_t *state, xpc_object_t children)
{
    errno_t err;
    userfs_stream_t dir = NULL;
    uint64_t offset = 0;
    enum { MAX_CHILDREN_PER_CALL = 10 };
    
    // Grab any incoming state.  Initialize outgoing state to NULL in case of error.
    if (*state)
    {
        offset = xpc_uint64_get_value(*state);
        xpc_release(*state);
        *state = NULL;
    }
    
    err = volume_lookup(volume, path, &dir, NULL);
    if (err) return err;
    
    err = dir_enumerate(dir, offset, ^int(uint64_t offset, struct dosdirentry *entry, const struct unistr255 *name) {
        int err2 = 0;
        
        /*
         * Note: we make this check at the top of the loop, not the bottom, to
         * optimize the case where the directory has exactly MAX_CHILDREN_PER_CALL
         * items left.  Checking at the top of the loop means that we will return
         * the end-of-directory indication (*state == NULL) along with the children.
         * If we checked at the bottom of the loop, we'd return state, and the
         * caller would make one more call that returns no children.
         */
        if (xpc_array_get_count(children) >= MAX_CHILDREN_PER_CALL)
        {
            *state = xpc_uint64_create(offset);
            return USERFS_END_ENUMERATION;
        }
        
        xpc_object_t info = xpc_dictionary_create(NULL, NULL, 0);
        
        xpc_dictionary_set_bool(info, USERFSD_PARM_FOLDER, (entry->deAttributes & ATTR_DIRECTORY) != 0);
        xpc_dictionary_set_bool(info, USERFSD_PARM_LOCKED, (entry->deAttributes & ATTR_READONLY) != 0);
        xpc_dictionary_set_uint64(info, USERFSD_PARM_LENGTH, getuint32(entry->deFileSize));
        xpc_dictionary_set_date(info, USERFSD_PARM_CREATE_DATE, timestamp_to_nanoseconds(getuint16(entry->deCDate), getuint16(entry->deCTime), entry->deCHundredth));
        xpc_dictionary_set_date(info, USERFSD_PARM_MOD_DATE, timestamp_to_nanoseconds(getuint16(entry->deMDate), getuint16(entry->deMTime), 0));
        xpc_dictionary_set_unistr255(info, USERFSD_PARM_NAME, name);
        
        if (entry->deAttributes & ATTR_DIRECTORY)
        {
            /* Calculate the length of the directory */
            uint32_t numClusters = 0;
            uint32_t cluster = getuint16(entry->deStartCluster);
            if (volume->fatMask == FAT32_MASK)
                cluster |= getuint16(entry->deHighClust) << 16;
            int err2 = chain_length(volume, cluster, &numClusters);
            if (err2) goto dir_error;
            xpc_dictionary_set_uint64(info, USERFSD_PARM_LENGTH, (uint64_t)numClusters * volume->bytesPerCluster);
        }
        
        xpc_array_append_value(children, info);
        
    dir_error:
        xpc_release(info);
        
        return err2;
    });
    
    if (err == USERFS_END_ENUMERATION)
        err = 0;
    
    msdosfs_stream_close(dir);
    return err;
}

static errno_t msdosfs_item_delete(userfs_volume_t volume, const char *path)
{
    if (volume->locked)
        return EROFS;
    
    userfs_stream_t child = NULL;
    userfs_stream_t parent = NULL;
    errno_t error = volume_lookup(volume, path, &child, &parent);
    if (error)
        return error;
    if (child->directory)
    {
        error = EISDIR;
        goto fail;
    }
    if (child->locked)
    {
        error = EPERM;
        goto fail;
    }
    
    // Mark the volume dirty
    error = msdosfs_volume_mark_dirty(volume, true);
    if (error)
        goto fail;
    
    // Delete the directory entries for the child
    error = dir_delete_entries(parent, child->dirEntryOffset);
    if (error)
        goto fail;
    
    // Delete the clusters allocated to the child
    if (child->startingCluster)
    {
        uint32_t numFreed;
        error = chain_free(volume, child->startingCluster, &numFreed);
        if (error == 0)
            volume_update_fsinfo(volume, numFreed);
    }
    
    // TODO: Update parent dir's mod time
    
fail:
    msdosfs_stream_close(child);
    if (parent)
        msdosfs_stream_close(parent);
    return error;
}

static errno_t msdosfs_stream_open(userfs_volume_t volume, const char *path, userfs_stream_t *stream)
{
    errno_t err;
    
    err = volume_lookup(volume, path, stream, NULL);
    if (err == 0 && (*stream)->directory)
    {
        msdosfs_stream_close(*stream);
        *stream = NULL;
        err = EISDIR;
    }
    
    return err;
}

static uint64_t msdosfs_stream_length(userfs_stream_t stream)
{
    return stream->length;
}

static uint64_t msdosfs_stream_get_next_offset_to_cache(userfs_stream_t stream)
{
    return stream->next_offset_to_cache;
}

static uint64_t msdosfs_stream_get_last_offset_in_cache(userfs_stream_t stream)
{
    return stream->last_offset_in_cache;
}

static void msdosfs_stream_increment_threadCount(userfs_stream_t stream)
{
    stream->thread_count++;
}

static errno_t msdosfs_stream_read(userfs_stream_t stream, void *buffer, uint64_t offset, size_t length, bool readAheadOperation)
{
    errno_t error = 0;

    if (length <= 0)
        return 0;

    if (offset > stream->length)
    {
        MSDOSFS_LOG_ERR(msdosfs_log_default, "%s: %s: Attempt to read offset beyond EOF (offset=%llu, EOF=%llu)\n", device_name(stream->volume->device), stream->name, offset, stream->length);
        return EINVAL;
    }

    if (length > (stream->length - offset))
    {
        MSDOSFS_LOG_ERR(msdosfs_log_default, "%s: %s: Attempt to read beyond EOF (offset=%llu, length=%lu, EOF=%llu)\n", device_name(stream->volume->device), stream->name, offset, (unsigned long)length, stream->length);
        return EINVAL;
    }

    /* If it is not a read-ahead operation save the offset. */
    if(!readAheadOperation)
    {
        stream->next_offset_to_cache = offset + length;
        error = stream_get_read_data(stream, offset, length, buffer);
        if(error)
        {
            return error;
        }
    }
    else
    {
        error = stream_get_read_ahead_data(stream, offset, length);
        if(error)
        {
            return error;
        }
    }

    return error;
}

static errno_t msdosfs_stream_close(userfs_stream_t stream)
{
    pthread_mutex_t* readahead_mutex = device_mutex(stream->volume->device);

    assert(readahead_mutex != NULL);

    pthread_mutex_lock(readahead_mutex);
    if(!stream->thread_count)
    {
        if (stream->name)
            free((void*)stream->name);
        free(stream);
        pthread_mutex_unlock(readahead_mutex);
    }
    else
    {
        stream->close_lazily = true;
        pthread_mutex_unlock(readahead_mutex);
    }
    return 0;
}

static const char * msdosfs_stream_name(userfs_stream_t stream)    // For debugging
{
    return stream->name;
}


__attribute__((visibility("default")))
void userfs_plugin_init(struct userfs_plugin_operations *ops, const struct userfs_callbacks *callbacks)
{
    if ((msdosfs_log_default = os_log_create("com.apple.filesystems.msdosfs", "plugin")) == NULL) {
        err(EX_CANTCREAT, "Creating default log");
    }
    
    /* Return the function pointers to our plug-in operations. */
    ops->volume_open      = msdosfs_volume_open;
    ops->volume_is_locked = msdosfs_volume_is_locked;
    ops->volume_flush     = msdosfs_volume_flush;
    ops->volume_close     = msdosfs_volume_close;
    ops->item_get_info    = msdosfs_item_get_info;
    ops->dir_enumerate    = msdosfs_dir_enumerate;
    ops->item_delete      = msdosfs_item_delete;
    ops->stream_open      = msdosfs_stream_open;
    ops->stream_length    = msdosfs_stream_length;
    ops->stream_get_next_offset_to_cache = msdosfs_stream_get_next_offset_to_cache;
    ops->stream_get_last_offset_in_cache = msdosfs_stream_get_last_offset_in_cache;
    ops->stream_increment_threadCount = msdosfs_stream_increment_threadCount;
    ops->stream_read      = msdosfs_stream_read;
    ops->stream_close     = msdosfs_stream_close;
    ops->stream_name      = msdosfs_stream_name;
    
    /* Set up the callback function pointers. */
    device_name           = callbacks->device_name;
    device_fd             = callbacks->device_fd;
    device_mutex          = callbacks->device_mutex;
    cache_get_buffer      = callbacks->cache_get_buffer;
    cache_release_buffer  = callbacks->cache_release_buffer;
    cache_flush_buffer    = callbacks->cache_flush_buffer;
    cache_flush           = callbacks->cache_flush;
    cache_invalidate      = callbacks->cache_invalidate;
    buffer_bytes          = callbacks->buffer_bytes;
    buffer_offset         = callbacks->buffer_offset;
    buffer_size           = callbacks->buffer_size;
    buffer_mark_dirty     = callbacks->buffer_mark_dirty;

	/* Content caching */
    cache_get_content_buffer  = callbacks->cache_get_content_buffer;
    content_buffer_bytes   = callbacks->content_buffer_bytes;
    populate_content_buffer_cache = callbacks->populate_content_buffer_cache;
}
