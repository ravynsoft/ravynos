/* Copyright © 2017-2018 Apple Inc. All rights reserved.
 *
 *  livefiles_hfs_tester.c
 *  hfs
 *
 *  Created by Yakov Ben Zaken on 31/12/2017.
 */

#include <stdint.h>
#include <stdlib.h>
#include <mach/mach_time.h>
#include "livefiles_hfs_tester.h"
#include "lf_hfs_fsops_handler.h"
#include "lf_hfs_dirops_handler.h"
#include <UserFS/UserVFS.h>
#include <assert.h>
#include <sys/queue.h>
#include "lf_hfs_journal.h"
#include "lf_hfs_generic_buf.h"
#include "lf_hfs_vfsutils.h"
#include "lf_hfs_raw_read_write.h"

#define DEFAULT_SYNCER_PERIOD     100 // mS
#define MAX_UTF8_NAME_LENGTH (NAME_MAX*3+1)
#define MAX_MAC2SFM            (0x80)
#define TEST_CYCLE_COUNT (1)
#define CMP_TIMES(timspec1, timspec2)             \
    ((timspec1.tv_sec == timspec2.tv_sec)         \
    && (timspec1.tv_nsec == timspec2.tv_nsec))

#define ARR_LEN(arr) ((sizeof(arr))/(sizeof(arr[0])))

uint32_t guSyncerPeriod = DEFAULT_SYNCER_PERIOD;

typedef int (*test_hander_t)( UVFSFileNode RootNode );

#if HFS_CRASH_TEST
    typedef int (*TesterCrashAbortFunction_FP)(void *psTestData, CrashAbort_E eAbort, int iFD, UVFSFileNode psNode, pthread_t bSyncerThread);
#endif


typedef struct {
    
    char*           pcTestName;
    char*           pcDMGPath;
    test_hander_t   pfTestHandler;
    
    UVFSFileNode    psRootNode;
    pthread_t       sSyncerThread;
    bool            bSyncerOn;
    uint32_t        uSyncerCount;
    bool            bSparseImage;
    
    #if HFS_CRASH_TEST
        uint32_t                    uCrashAbortCnt;
        CrashAbort_E                eCrashID;
        TesterCrashAbortFunction_FP pAbortFunc;
        pthread_t                   sTestExeThread;
    #endif
} TestData_S;

typedef struct {
    int          iErr;
    int          iFD;
    UVFSFileNode psNode;
    pthread_t    pSyncerThread;
    #if  HFS_CRASH_TEST
        CrashAbort_E eCrashID;
    #endif
} TesterThreadReturnStatus_S;

#if  HFS_CRASH_TEST
typedef struct {
    uint32_t       uCrashCount;
    CrashAbort_E   eCrashID;
    int            iFD;
    UVFSFileNode   psNode;
    pthread_t      pSyncerThread;
} CrashReport_S;
#endif

#if HFS_CRASH_TEST
char *ppcCrashAbortDesc[CRASH_ABORT_LAST] = {
    [CRASH_ABORT_NONE]                         = "None",
    [CRASH_ABORT_MAKE_DIR]                     = "Make Dir",
    [CRASH_ABORT_JOURNAL_BEFORE_FINISH]        = "Journal, before transaction finish",
    [CRASH_ABORT_JOURNAL_AFTER_JOURNAL_DATA]   = "Journal, after journal data has been written",
    [CRASH_ABORT_JOURNAL_AFTER_JOURNAL_HEADER] = "Journal, after journal header has been written",
    [CRASH_ABORT_JOURNAL_IN_BLOCK_DATA]        = "Journal, while block data is being written",
    [CRASH_ABORT_JOURNAL_AFTER_BLOCK_DATA]     = "Journal, after block data has been written",
    [CRASH_ABORT_ON_UNMOUNT]                   = "Unmount",
};
uint32_t      guCrashAbortCnt = 0;
CrashReport_S gsCrashReport;
#endif

int giFD = 0;

// Multi-thread read-write test
#if 1 // Quick Regression
    #define MTRW_NUM_OF_THREADS             10
    #define MTRW_FILE_SIZE              5*1000
    #define MTRW_NUM_OF_FILES               10
    #define MTRW_NUM_OF_SYMLINKS            10
    #define MTRW_SYMLINK_SIZE         PATH_MAX
    #define MTRW_NUM_OF_OPERATIONS          10
#else // Longer Regression
    #define MTRW_NUM_OF_THREADS             30
    #define MTRW_FILE_SIZE              5*1000
    #define MTRW_NUM_OF_FILES               30
    #define MTRW_NUM_OF_SYMLINKS            30
    #define MTRW_SYMLINK_SIZE         PATH_MAX
    #define MTRW_NUM_OF_OPERATIONS          30
#endif

typedef struct {
    uint32_t     uThreadNum;
    UVFSFileNode psRootNode;
    uint32_t     uNumOfFiles;
    uint32_t     uNumOfSymLinks;
    uint32_t     uSymLinkSize;
    uint64_t     uFileSize;
    int32_t      iRetVal;
} RWThreadData_S;


static int   SetAttrChangeSize(UVFSFileNode FileNode,uint64_t uNewSize);
static int   SetAttrChangeMode(UVFSFileNode FileNode,uint32_t uNewMode);
static int   SetAttrChangeUidGid(UVFSFileNode FileNode, uint32_t uNewUid, uint32_t uNewGid);
static int   SetAttrChangeAtimeMtime(UVFSFileNode FileNode);
static int   GetAttrAndCompare(UVFSFileNode FileNode,UVFSFileAttributes* sInAttrs);
static int   HFSTest_RunTest(TestData_S *psTestData);
static void *ReadWriteThread(void *pvArgs);


struct unistr255 {
    uint16_t length;
    uint16_t chars[255];
};

u_char
l2u[256] = {
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
 * Macintosh Unicode (LSB) to Microsoft Services for Macintosh (SFM) Unicode
 */
static const uint16_t
mac2sfm[MAX_MAC2SFM] = {
    0x0,    0xf001, 0xf002, 0xf003, 0xf004, 0xf005, 0xf006, 0xf007,     /* 00-07 */
    0xf008, 0xf009, 0xf00a, 0xf00b, 0xf00c, 0xf00d, 0xf00e, 0xf00f,     /* 08-0f */
    0xf010, 0xf011, 0xf012, 0xf013, 0xf014, 0xf015, 0xf016, 0xf017,     /* 10-17 */
    0xf018, 0xf019, 0xf01a, 0xf01b, 0xf01c, 0xf01d, 0xf01e, 0xf01f,     /* 18-1f */
    0x20,   0x21,   0xf020, 0x23,   0x24,   0x25,   0x26,   0x27,       /* 20-27 */
    0x28,   0x29,   0xf021, 0x2b,   0x2c,   0x2d,   0x2e,   0x2f,       /* 28-2f */
    0x30,   0x31,   0x32,   0x33,   0x34,   0x35,   0x36,   0x37,       /* 30-37 */
    0x38,   0x39,   0xf022, 0x3b,   0xf023, 0x3d,   0xf024, 0xf025,     /* 38-3f */
    0x40,   0x41,   0x42,   0x43,   0x44,   0x45,   0x46,   0x47,       /* 40-47 */
    0x48,   0x49,   0x4a,   0x4b,   0x4c,   0x4d,   0x4e,   0x4f,       /* 48-4f */
    0x50,   0x51,   0x52,   0x53,   0x54,   0x55,   0x56,   0x57,       /* 50-57 */
    0x58,   0x59,   0x5a,   0x5b,   0xf026, 0x5d,   0x5e,   0x5f,       /* 58-5f */
    0x60,   0x61,   0x62,   0x63,   0x64,   0x65,   0x66,   0x67,       /* 60-67 */
    0x68,   0x69,   0x6a,   0x6b,   0x6c,   0x6d,   0x6e,   0x6f,       /* 68-6f */
    0x70,   0x71,   0x72,   0x73,   0x74,   0x75,   0x76,   0x77,       /* 70-77 */
    0x78,   0x79,   0x7a,   0x7b,   0xf027, 0x7d,   0x7e,   0x7f,       /* 78-7f */
};

static void
unistr255ToLowerCase( struct unistr255* psUnistr255 )
{
    for ( uint16_t uIdx=0; uIdx<psUnistr255->length; uIdx++ )
    {
        if ( psUnistr255->chars[uIdx] < 0x100 )
        {
            psUnistr255->chars[uIdx] = l2u[psUnistr255->chars[uIdx]];
        }
    }
}

void HFSTest_PrintCacheStats(void) {
    printf("Cache Statistics: buf_cache_size %u, max_buf_cache_size %u, buf_cache_cleanup %u, buf_cache_remove %u, max_gen_buf_uncached %u, gen_buf_uncached %u.\n",
           gCacheStat.buf_cache_size,
           gCacheStat.max_buf_cache_size,
           gCacheStat.buf_cache_cleanup,
           gCacheStat.buf_cache_remove,
           gCacheStat.max_gen_buf_uncached,
           gCacheStat.gen_buf_uncached);
}

__unused static long long int timestamp()
{
    /* Example of timestamp in second. */
    time_t timestamp_sec; /* timestamp in second */
    time(&timestamp_sec);  /* get current time; same as: timestamp_sec = time(NULL)  */

    /* Example of timestamp in microsecond. */
    struct timeval timer_usec;
    long long int timestamp_usec; /* timestamp in microsecond */
    if (!gettimeofday(&timer_usec, NULL)) {
        timestamp_usec = ((long long int) timer_usec.tv_sec) * 1000000ll +
        (long long int) timer_usec.tv_usec;
    }
    else {
        timestamp_usec = -1;
    }

    return timestamp_usec;
}

__unused static errno_t
CONV_UTF8ToUnistr255(const uint8_t *utf8, size_t utf8Length, struct unistr255 *unicode)
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

    //Only in "." and ".." we don't need to change the last char to speciel char.
    bool bNeedToChangeLastChar = true;
    if ( ((unicode->length == 1) && (unicode->chars[0] == '.')) ||
        ((unicode->length == 2) && (unicode->chars[0] == '.') && (unicode->chars[1] == '.')) )
    {
        bNeedToChangeLastChar = false;
    }

    for ( uint16_t uIdx=0; uIdx<unicode->length; uIdx++ )
    {
        //If the last char is "." or " " we need to change it.
        //We won't use the mac2sfm table, we will do it manually
        if ( bNeedToChangeLastChar && uIdx == unicode->length-1 )
        {
            if ( unicode->chars[uIdx] == ' ' )
            {
                unicode->chars[uIdx] = 0xf028;
                continue;
            }

            if ( unicode->chars[uIdx] == '.' )
            {
                unicode->chars[uIdx] = 0xf029;
                continue;
            }
        }


        if ( unicode->chars[uIdx] < MAX_MAC2SFM )
        {
            unicode->chars[uIdx] = mac2sfm[unicode->chars[uIdx]];
        }
    }

    return 0;
}

__unused static void print_dir_entry_name( uint32_t uLen, char* pcName, char* pcSearchName, bool* pbFound )
{
    struct unistr255 sU255;
    struct unistr255 sU2552;
    memset( &sU255, 0, sizeof(struct unistr255));
    memset( &sU2552, 0, sizeof(struct unistr255));
    errno_t status = CONV_UTF8ToUnistr255( (uint8_t*)pcName, strlen(pcName), &sU255 );
    status |= CONV_UTF8ToUnistr255( (uint8_t*)pcSearchName, strlen(pcSearchName), &sU2552 );

    if ( status != 0 ) { assert(0); }

    uLen = sU255.length;

    char pcNameToPrint[uLen+1];
    memset( pcNameToPrint, 0, sizeof(pcNameToPrint) );

    uint16_t* puName = sU255.chars;
    for ( uint32_t uIdx=0; uIdx<uLen; uIdx++ )
    {
        pcNameToPrint[uIdx] = *puName++ & 0xff;
    }

    //    printf( "TESTER Entry Name = [%s]\n", pcNameToPrint );

    if ( pcSearchName )
    {
        unistr255ToLowerCase(&sU255);
        unistr255ToLowerCase(&sU2552);

        if ( (sU255.length == sU2552.length) && !memcmp(sU255.chars, sU2552.chars, sU255.length*2) )
        {
            *pbFound = true;
        }
    }
}

static bool
HFSTest_CompareReadDir( void* psReadEntry, UVFSDirEntryAttr* psExpectedAttr, bool bIsWAttr)
{
    bool bIsEqual = true;
    if (bIsWAttr)
    {
        UVFSDirEntryAttr* psNewDirListEntry = (UVFSDirEntryAttr*) psReadEntry;
        if (  (strcmp(UVFS_DIRENTRYATTR_NAMEPTR(psExpectedAttr),UVFS_DIRENTRYATTR_NAMEPTR(psNewDirListEntry)))               || // Comapre Name
            (psNewDirListEntry->dea_attrs.fa_type != psExpectedAttr->dea_attrs.fa_type)                       || // Comapre Type
            (psNewDirListEntry->dea_attrs.fa_size != psExpectedAttr->dea_attrs.fa_size)                       || // Comapre Size
            (psNewDirListEntry->dea_attrs.fa_nlink != psExpectedAttr->dea_attrs.fa_nlink)                     || // Compare Nlink
            (psNewDirListEntry->dea_attrs.fa_mtime.tv_sec != psExpectedAttr->dea_attrs.fa_mtime.tv_sec)       || // Comapre MTime
            (psNewDirListEntry->dea_attrs.fa_ctime.tv_sec != psExpectedAttr->dea_attrs.fa_ctime.tv_sec)       || // Comapre Ctime
            (psNewDirListEntry->dea_attrs.fa_atime.tv_sec != psExpectedAttr->dea_attrs.fa_atime.tv_sec)       || // Comapre Atime
            (psNewDirListEntry->dea_attrs.fa_birthtime.tv_sec != psExpectedAttr->dea_attrs.fa_birthtime.tv_sec)       || // Comapre birthtime
            (psNewDirListEntry->dea_attrs.fa_mtime.tv_nsec != psExpectedAttr->dea_attrs.fa_mtime.tv_nsec)     || // Comapre MTime
            (psNewDirListEntry->dea_attrs.fa_ctime.tv_nsec != psExpectedAttr->dea_attrs.fa_ctime.tv_nsec)     || // Comapre Ctime
            (psNewDirListEntry->dea_attrs.fa_atime.tv_nsec != psExpectedAttr->dea_attrs.fa_atime.tv_nsec)     || // Comapre Atime
            (psNewDirListEntry->dea_attrs.fa_birthtime.tv_nsec != psExpectedAttr->dea_attrs.fa_birthtime.tv_nsec)     || // Comapre birthtime
            (psNewDirListEntry->dea_attrs.fa_allocsize != psExpectedAttr->dea_attrs.fa_allocsize)       )
        {
            printf("HFSTest_CompareReadDir: failed.\n");

            printf("HFSTest_CompareReadDir: expected- name [%s], type [%d], size [%llu], nlink [%u], allocsize [%llu].\n",UVFS_DIRENTRYATTR_NAMEPTR(psExpectedAttr), psExpectedAttr->dea_attrs.fa_type, psExpectedAttr->dea_attrs.fa_size, psExpectedAttr->dea_attrs.fa_nlink, psExpectedAttr->dea_attrs.fa_allocsize);
            printf("HFSTest_CompareReadDir: expected- mtime [%ld.%ld], ctime [%ld.%ld], atime [%ld.%ld], btime [%ld.%ld] .\n",psExpectedAttr->dea_attrs.fa_mtime.tv_sec,psExpectedAttr->dea_attrs.fa_mtime.tv_nsec,psExpectedAttr->dea_attrs.fa_ctime.tv_sec,psExpectedAttr->dea_attrs.fa_ctime.tv_nsec, psExpectedAttr->dea_attrs.fa_atime.tv_sec, psExpectedAttr->dea_attrs.fa_atime.tv_nsec, psExpectedAttr->dea_attrs.fa_birthtime.tv_sec, psExpectedAttr->dea_attrs.fa_birthtime.tv_nsec);

            printf("HFSTest_CompareReadDir: got     - name [%s], type [%d], size [%llu], nlink [%u], allocsize [%llu].\n",UVFS_DIRENTRYATTR_NAMEPTR(psNewDirListEntry), psNewDirListEntry->dea_attrs.fa_type, psNewDirListEntry->dea_attrs.fa_size, psNewDirListEntry->dea_attrs.fa_nlink, psNewDirListEntry->dea_attrs.fa_allocsize);
            printf("HFSTest_CompareReadDir: got     - mtime [%ld.%ld], ctime [%ld.%ld], atime [%ld.%ld], btime [%ld.%ld] .\n",psNewDirListEntry->dea_attrs.fa_mtime.tv_sec,psNewDirListEntry->dea_attrs.fa_mtime.tv_nsec,psNewDirListEntry->dea_attrs.fa_ctime.tv_sec,psNewDirListEntry->dea_attrs.fa_ctime.tv_nsec, psNewDirListEntry->dea_attrs.fa_atime.tv_sec,psNewDirListEntry->dea_attrs.fa_atime.tv_nsec, psNewDirListEntry->dea_attrs.fa_birthtime.tv_sec,psNewDirListEntry->dea_attrs.fa_birthtime.tv_nsec);

            bIsEqual = false;
        }
    }
    else
    {
        UVFSDirEntry* psNewDirListEntry = ( UVFSDirEntry*) psReadEntry;
        if ( (strcmp(UVFS_DIRENTRYATTR_NAMEPTR(psExpectedAttr),psNewDirListEntry->de_name)) ||  // Comapre Name
            (psNewDirListEntry->de_filetype != psExpectedAttr->dea_attrs.fa_type))              // Comapre Type
        {
            bIsEqual = false;
        }
    }



    return bIsEqual;
}

/* --------------------------------------------------------------------------------------------- */
static void SetExpectedAttr(char* pcName, uint32_t uType, UVFSDirEntryAttr* psAttr);
static int ReadDirAttr(UVFSFileNode psNode, UVFSDirEntryAttr* psReadDirTestsData, uint32_t uDirEntries);
static int RemoveFolder(UVFSFileNode ParentNode,char* DirNameToRemove);
static int CreateNewFolder(UVFSFileNode ParentNode,UVFSFileNode* NewDirNode,char* NewDirName);
static int CreateHardLink(UVFSFileNode FromNode, UVFSFileNode ToDirNode, char* NewHardLinkName);
static int RemoveFile(UVFSFileNode ParentNode,char* FileNameToRemove);
static int CreateNewFile(UVFSFileNode ParentNode,UVFSFileNode* NewFileNode,char* NewFileName,uint64_t size);
static int read_directory_and_search_for_name( UVFSFileNode psNode, char* pcSearchName, bool* pbFound, UVFSDirEntryAttr* psReadDirTestsData, uint32_t uDirEntries );
static int RenameFile(UVFSFileNode FromParentNode,UVFSFileNode FromNode,char* FromName, UVFSFileNode ToParentNode,UVFSFileNode ToNode,char* ToName);
/* --------------------------------------------------------------------------------------------- */

static int RemoveFolder(UVFSFileNode ParentNode,char* DirNameToRemove)
{
    int error =0;

    error = HFS_fsOps.fsops_rmdir(ParentNode, DirNameToRemove);;

    return error;
}

static int RemoveFile(UVFSFileNode ParentNode,char* FileNameToRemove)
{
    int error =0;

    error = HFS_fsOps.fsops_remove( ParentNode, FileNameToRemove, NULL);

    return error;
}

static int RenameFile(UVFSFileNode FromParentNode,UVFSFileNode FromNode,char* FromName, UVFSFileNode ToParentNode,UVFSFileNode ToNode,char* ToName)
{
    int error =0;

    error = HFS_fsOps.fsops_rename( FromParentNode, FromNode, FromName, ToParentNode, ToNode, ToName, 0);

    return error;
}

static int CreateNewFile(UVFSFileNode ParentNode,UVFSFileNode* NewFileNode,char* NewFileName,uint64_t size)
{
    int error =0;
    UVFSFileAttributes attrs = {0};

    attrs.fa_validmask = UVFS_FA_VALID_MODE | UVFS_FA_VALID_SIZE;
    attrs.fa_type = UVFS_FA_TYPE_FILE;
    attrs.fa_mode = UVFS_FA_MODE_OTH(UVFS_FA_MODE_RWX)|UVFS_FA_MODE_GRP(UVFS_FA_MODE_RWX)|UVFS_FA_MODE_USR(UVFS_FA_MODE_RWX);;
    attrs.fa_size = size;

    error = HFS_fsOps.fsops_create(ParentNode, NewFileName, &attrs, NewFileNode);
    
    return error;
}

static int CreateNewFolder(UVFSFileNode ParentNode,UVFSFileNode* NewDirNode,char* NewDirName)
{
    int error =0;

    UVFSFileAttributes attrs;
    memset(&attrs,0,sizeof(UVFSFileAttributes));
    attrs.fa_validmask = UVFS_FA_VALID_MODE;
    attrs.fa_type = UVFS_FA_TYPE_DIR;
    attrs.fa_mode = UVFS_FA_MODE_OTH(UVFS_FA_MODE_RWX)|UVFS_FA_MODE_GRP(UVFS_FA_MODE_RWX)|UVFS_FA_MODE_USR(UVFS_FA_MODE_RWX);
    error = HFS_fsOps.fsops_mkdir(ParentNode, NewDirName, &attrs, NewDirNode);

    return error;
}

static int CreateHardLink(UVFSFileNode FromNode, UVFSFileNode ToDirNode, char* NewHardLinkName)
{
    int error =0;

    UVFSFileAttributes sToDirAttrs;
    UVFSFileAttributes sFromNodeAttrs;
    memset(&sToDirAttrs,0,sizeof(UVFSFileAttributes));
    memset(&sFromNodeAttrs,0,sizeof(UVFSFileAttributes));

    error = HFS_fsOps.fsops_link(FromNode, ToDirNode, NewHardLinkName, &sFromNodeAttrs, &sToDirAttrs);

    return error;
}

static int read_directory_and_search_for_name( UVFSFileNode psNode, char* pcSearchName, bool* pbFound, UVFSDirEntryAttr* psReadDirTestsData, uint32_t uDirEntries )
{
    if (pbFound) *pbFound = false;
    
    uint32_t uBufferSize = 1000;
    uint8_t* puBuffer = malloc(uBufferSize*2);
    if ( puBuffer == NULL )
    {
        return ENOMEM;
    }
    memset(puBuffer, 0xff, uBufferSize*2);

    uint64_t uCookie = 0;
    uint64_t uVerifier = UVFS_DIRCOOKIE_VERIFIER_INITIAL;
    bool bConRead = true;

    uint32_t uDirsCounter   = 0;
    uint32_t uFilesCounter  = 0;
    uint32_t uLinksCounter  = 0;
    size_t   outLen = 0;
    uint32_t uDirIndex = 0;
    int iReadDirERR = 0;
    UVFSDirEntryAttr* psDirData = psReadDirTestsData;

    do {
        uint32_t uBufCurOffset  = 0;

        memset(puBuffer, 0, uBufferSize);

        iReadDirERR = HFS_fsOps.fsops_readdir (psNode, puBuffer, uBufferSize, uCookie, &outLen, &uVerifier);
        //        assert(0xffffffffffffffff == *(uint64_t*)(&puBuffer[100]));
        if ( (iReadDirERR != 0 && iReadDirERR != UVFS_READDIR_EOF_REACHED) || outLen==0)
        {
            bConRead = false;
        }
        else
        {
            //Go over all entries in the list and check if we got to the end of the directory
            bool bEndOfDirectoryList = false;

            while ( !bEndOfDirectoryList && iReadDirERR != UVFS_READDIR_EOF_REACHED )
            {
                UVFSDirEntry* psNewDirListEntry = (UVFSDirEntry*) &puBuffer[uBufCurOffset];
                uCookie = psNewDirListEntry->de_nextcookie;

                //We found all the files in the root directory
                if ( ( psNewDirListEntry->de_nextcookie == UVFS_DIRCOOKIE_EOF ) || ( psNewDirListEntry->de_reclen == 0 ) )
                {
                    bEndOfDirectoryList = true;
                }

                print_dir_entry_name( psNewDirListEntry->de_namelen, psNewDirListEntry->de_name, pcSearchName, pbFound );
                switch ( psNewDirListEntry->de_filetype )
                {
                    case UVFS_FA_TYPE_DIR:
                    {
                        //printf("found dir: ID: %llu, named [%s], in offset [%u]\n",psNewDirListEntry->de_fileid, psNewDirListEntry->de_name, uBufCurOffset);
                        uDirsCounter++;
                    }
                        break;
                    case UVFS_FA_TYPE_FILE:
                    {
                        //printf("found file: ID: %llu, named [%s], in offset [%u]\n",psNewDirListEntry->de_fileid, psNewDirListEntry->de_name, uBufCurOffset);
                        uFilesCounter++;
                    }
                        break;
                    case UVFS_FA_TYPE_SYMLINK:
                    {
                        //printf("found link: ID: %llu, named [%s], in offset [%u]\n",psNewDirListEntry->de_fileid, psNewDirListEntry->de_name, uBufCurOffset);
                        uLinksCounter++;
                    }
                        break;

                    default:
                        printf("Found Unkown file type %d, named [%s], Exiting\n", psNewDirListEntry->de_filetype, psNewDirListEntry->de_name);
                        bEndOfDirectoryList = true;
                        bConRead = false;
                        break;
                }

                if (psDirData != NULL)
                {
                    printf("Expected FileName = [%s], FileType = [%d].\n",  UVFS_DIRENTRYATTR_NAMEPTR(psDirData), psDirData->dea_attrs.fa_type);

                    //TBD - When getAttr will work need to change the compare to getAttr vs ReadDirAttr
                    if ( !HFSTest_CompareReadDir(psNewDirListEntry, psDirData, false) )
                    {
                        iReadDirERR = EINVAL;
                        bConRead = false;
                        break;
                    }
                    assert(uDirIndex<uDirEntries);
                    uDirIndex++;
                    psDirData = (UVFSDirEntryAttr*) ((void*) psDirData + sizeof(UVFSDirEntryAttr) + MAX_UTF8_NAME_LENGTH);
                }

                uBufCurOffset += UVFS_DIRENTRY_RECLEN(strlen(psNewDirListEntry->de_name));
            }
        }
    } while( bConRead && (iReadDirERR != UVFS_READDIR_EOF_REACHED) );

    if ( puBuffer ) free( puBuffer );

    if(iReadDirERR == UVFS_READDIR_EOF_REACHED)
        iReadDirERR = 0;

    if ( psDirData )
        assert(uDirIndex==uDirEntries);

    return iReadDirERR;
}

static int
ReadDirAttr( UVFSFileNode psNode, UVFSDirEntryAttr * psReadDirTestsData, uint32_t uDirEntries)
{
    uint32_t uBufferSize = 1000;
    uint8_t* puBuffer = malloc(uBufferSize);
    if ( puBuffer == NULL )
    {
        return ENOMEM;
    }

    uint64_t uCookie = 0;
    uint64_t uVerifier = UVFS_DIRCOOKIE_VERIFIER_INITIAL;
    bool bConRead = true;
    size_t   outLen = 0;
    int iReadDirERR = 0;
    uint32_t uDirIndex = 0;
    UVFSDirEntryAttr* psReadDir = psReadDirTestsData;

    do {

        uint32_t uBufCurOffset  = 0;

        memset(puBuffer, 0, uBufferSize);

        iReadDirERR = HFS_fsOps.fsops_readdirattr (psNode, puBuffer, uBufferSize, uCookie, &outLen, &uVerifier);
        if ( (iReadDirERR != 0 && iReadDirERR != UVFS_READDIR_EOF_REACHED) || (outLen == 0) )
        {
            bConRead = false;
        }
        else
        {
            //Go over all entries in the list and check if we got to the end of the directory
            bool bEndOfDirectoryList = false;

            while ( !bEndOfDirectoryList && iReadDirERR != UVFS_READDIR_EOF_REACHED )
            {
                UVFSDirEntryAttr* psNewDirListEntry = (UVFSDirEntryAttr*) &puBuffer[uBufCurOffset];
                uCookie = psNewDirListEntry->dea_nextcookie;
                //We found all the files in the root directory
                if ( ( psNewDirListEntry->dea_nextcookie == UVFS_DIRCOOKIE_EOF ) || ( psNewDirListEntry->dea_nextrec == 0 ) )
                {
                    bEndOfDirectoryList = true;
                }

                printf("Found FileName = [%s],  FileID = [%llu], FileSize = [%llu].\n", UVFS_DIRENTRYATTR_NAMEPTR(psNewDirListEntry), psNewDirListEntry->dea_attrs.fa_fileid, psNewDirListEntry->dea_attrs.fa_size);

                if (psReadDir != NULL)
                {
                    printf("Expected FileName = [%s], FileType = [%d].\n",  UVFS_DIRENTRYATTR_NAMEPTR(psReadDir), psReadDir->dea_attrs.fa_type);

                    //TBD - When getAttr will work need to change the compare to getAttr vs ReadDirAttr
                    if ( !HFSTest_CompareReadDir((void*)psNewDirListEntry, psReadDir, true) )
                    {
                        iReadDirERR = EINVAL;
                        bConRead = false;
                        break;
                    }
                    assert( uDirIndex < uDirEntries );
                    uDirIndex++;
                    psReadDir = (UVFSDirEntryAttr*) ((void*) psReadDir + sizeof(UVFSDirEntryAttr) + MAX_UTF8_NAME_LENGTH);
                }

                uBufCurOffset += UVFS_DIRENTRYATTR_RECLEN(psNewDirListEntry, strlen(UVFS_DIRENTRYATTR_NAMEPTR(psNewDirListEntry)));
            }
        }
    } while( bConRead && (iReadDirERR != UVFS_READDIR_EOF_REACHED) );

    if ( puBuffer )
        free( puBuffer );

    if(iReadDirERR == UVFS_READDIR_EOF_REACHED)
        iReadDirERR = 0;

    if (psReadDir != NULL)
        assert( uDirIndex == uDirEntries );

    return iReadDirERR;
}

static int
GetAttrAndCompare(UVFSFileNode FileNode,UVFSFileAttributes* sInAttrs)
{
    int error =0;
    UVFSFileAttributes sOutAttrs;
    error = HFS_fsOps.fsops_getattr(FileNode, &sOutAttrs);
    if (error)
    {
        printf("Failed in get attr with err [%d]\n",error);
        return error;
    }
    if (sInAttrs->fa_validmask & UVFS_FA_VALID_SIZE)
        if (sOutAttrs.fa_size != sInAttrs->fa_size)
            goto fail;

    if (sInAttrs->fa_validmask & UVFS_FA_VALID_MODE)
        if (sOutAttrs.fa_mode != sInAttrs->fa_mode)
            goto fail;

    if (sInAttrs->fa_validmask & UVFS_FA_VALID_BSD_FLAGS)
        if (sOutAttrs.fa_bsd_flags != sInAttrs->fa_bsd_flags)
            goto fail;

    if (sInAttrs->fa_validmask & UVFS_FA_VALID_ATIME)
        if (CMP_TIMES(sOutAttrs.fa_atime,sInAttrs->fa_atime))
            goto fail;

    if (sInAttrs->fa_validmask & UVFS_FA_VALID_MTIME)
        if (CMP_TIMES(sOutAttrs.fa_atime,sInAttrs->fa_atime))
            goto fail;

    if (sInAttrs->fa_validmask & UVFS_FA_VALID_CTIME)
        if (CMP_TIMES(sOutAttrs.fa_ctime, sInAttrs->fa_ctime))
            goto fail;

    if (sInAttrs->fa_validmask & UVFS_FA_VALID_BIRTHTIME)
        if (CMP_TIMES(sOutAttrs.fa_birthtime, sInAttrs->fa_birthtime))
            goto fail;

    goto out;

fail:
    error = 1;
out:
    if (error) printf("Failed in compare attr\n");
    return error;
}

static int
SetAttrChangeSize(UVFSFileNode FileNode,uint64_t uNewSize)
{
    int error =0;
    UVFSFileAttributes sInAttrs;
    UVFSFileAttributes sOutAttrs;
    memset(&sInAttrs,0,sizeof(UVFSFileAttributes));
    sInAttrs.fa_validmask |= UVFS_FA_VALID_SIZE;
    sInAttrs.fa_size = uNewSize;

    error =  HFS_fsOps.fsops_setattr( FileNode, &sInAttrs , &sOutAttrs );

    error = GetAttrAndCompare(FileNode,&sInAttrs);

    return error;
}

static int
SetAttrChangeMode(UVFSFileNode FileNode,uint32_t uNewMode)
{
    int error =0;
    UVFSFileAttributes sInAttrs;
    UVFSFileAttributes sOutAttrs;
    memset(&sInAttrs,0,sizeof(UVFSFileAttributes));
    sInAttrs.fa_validmask |= UVFS_FA_VALID_MODE;
    sInAttrs.fa_mode = uNewMode;

    error =  HFS_fsOps.fsops_setattr( FileNode, &sInAttrs , &sOutAttrs );

    return error;
}

static int
SetAttrChangeUidGid(UVFSFileNode FileNode, uint32_t uNewUid, uint32_t uNewGid)
{
    int error =0;
    UVFSFileAttributes sInAttrs;
    UVFSFileAttributes sOutAttrs;
    memset(&sInAttrs,0,sizeof(UVFSFileAttributes));
    sInAttrs.fa_validmask |= UVFS_FA_VALID_UID;
    sInAttrs.fa_validmask |= UVFS_FA_VALID_GID;
    sInAttrs.fa_uid = uNewUid;
    sInAttrs.fa_gid = uNewGid;

    error =  HFS_fsOps.fsops_setattr( FileNode, &sInAttrs , &sOutAttrs );

    return error;
}

static int
SetAttrChangeAtimeMtime(UVFSFileNode FileNode)
{
    int error =0;
    UVFSFileAttributes sInAttrs;
    UVFSFileAttributes sOutAttrs;

    error = HFS_fsOps.fsops_getattr(FileNode, &sOutAttrs);
    if (error)
    {
        printf("Failed in get attr (1) with err [%d]\n",error);
        return error;
    }

    memset(&sInAttrs,0,sizeof(UVFSFileAttributes));
    sInAttrs.fa_validmask |= UVFS_FA_VALID_ATIME;
    sInAttrs.fa_validmask |= UVFS_FA_VALID_MTIME;
    sInAttrs.fa_atime.tv_sec = sOutAttrs.fa_atime.tv_sec + 90000000;
    sInAttrs.fa_mtime.tv_sec = sOutAttrs.fa_mtime.tv_sec + 90000000;

    error =  HFS_fsOps.fsops_setattr( FileNode, &sInAttrs , &sOutAttrs );

    if (error)
    {
        printf("Failed to set attr to change atime and mtime err [%d]", error);
        return error;
    }
    error = HFS_fsOps.fsops_getattr(FileNode, &sOutAttrs);
    if (error)
    {
        printf("Failed in get attr (2) with err [%d]\n",error);
        return error;
    }

    if ( (sOutAttrs.fa_atime.tv_sec != sInAttrs.fa_atime.tv_sec) || (sOutAttrs.fa_mtime.tv_sec != sInAttrs.fa_mtime.tv_sec) )
    {
        printf("Failed to update time!\n");
        error = 1;
    }
    return error;
}

/*******************************************/
/*******************************************/
/*******************************************/
// Predefined Tests START.
/*******************************************/
/*******************************************/
/*******************************************/

#define HFS_TEST_PREFIX        "RUN_HFS_TESTS"
#define HFS_RUN_FSCK           "RUN_FSCK"
#define HFS_DMGS_FOLDER        "/Volumes/SSD_Shared/FS_DMGs/"
#define TEMP_DMG               "/tmp/hfstester.dmg"
#define TEMP_DMG_SPARSE        "/tmp/hfstester.dmg.sparseimage"
#define TEMP_DMG_BKUP          "/tmp/hfstester_bkup.dmg"
#define TEMP_DMG_BKUP_SPARSE   "/tmp/hfstester_bkup.dmg.sparseimage"
#define TEMP_DEV_PATH          "/tmp/dev_path.txt"
#define TEMP_DEV_PATH2         "/tmp/dev_path2.txt"
#define TEMP_DEV_PATH3         "/tmp/dev_path3.txt"
#define CREATE_SPARSE_VOLUME   "CREATE_SPARSE_VOLUME"
#define CREATE_HFS_DMG         "CREATE_HFS_DMG"

#define MAX_CMN_LEN (1024*2)

typedef int (*test_hander_t)( UVFSFileNode RootNode );

char pcLastDevPathName[50] = {0};
char pcDevPath[50] = {0};
char pcDevNum[50]  = {0};
char gpcResultsFolder[256] = {0};

static int
HFSTest_PrepareEnv(TestData_S *psTestData )
{
    int iErr = 0;
    bool bMountedAlready = false;
    char* pcCmd = malloc(MAX_CMN_LEN);
    assert(pcCmd);

    // Remove old dmg if exist.
    strcpy(pcCmd, "rm -rf ");
    strcat(pcCmd, TEMP_DMG" "TEMP_DEV_PATH" "TEMP_DEV_PATH2" "TEMP_DMG_SPARSE);
    #if HFS_CRASH_TEST
        if (psTestData->eCrashID) {
            strcat(pcCmd, " "TEMP_DMG_BKUP" "TEMP_DMG_BKUP_SPARSE);
        }
    #endif
    printf("Execute %s:\n", pcCmd);
    system(pcCmd);

    if (!strcmp(CREATE_SPARSE_VOLUME, psTestData->pcDMGPath)) {
        // Create a spase volume
        psTestData->bSparseImage = true;
        strcpy(pcCmd, "hdiutil create -ov -size 20G -type SPARSE -layout NONE ");
        strcat(pcCmd, TEMP_DMG);
        printf("Execute %s:\n", pcCmd);
        iErr = system( pcCmd );
        if ( iErr != 0 )
        {
            exit(-1);
        }
        strcpy(pcCmd, "hdiutil attach -nomount ");
        strcat(pcCmd, TEMP_DMG_SPARSE" > "TEMP_DEV_PATH);
        printf("Execute %s:\n", pcCmd);
        iErr = system( pcCmd );
        if ( iErr != 0 )
        {
            exit(-1);
        }
        bMountedAlready = true;
        
        // Extract disk number (disk??)
        strcpy(pcCmd, "cat "TEMP_DEV_PATH" | sed 's/\\/dev\\/disk\\([0-9]*\\)/\\1/' | awk '{print $1}' > "TEMP_DEV_PATH2);
        printf("Execute %s:\n", pcCmd);
        iErr = system( pcCmd );
        if ( iErr != 0 )
        {
            exit(-1);
        }
        FILE *psCat = fopen(TEMP_DEV_PATH2, "r");
        fgets(pcLastDevPathName, sizeof(pcLastDevPathName), psCat);
        pclose(psCat);
        pcLastDevPathName[strlen(pcLastDevPathName)-1] = '\0';
        
        sprintf(pcCmd, "newfs_hfs -v SparsedVolume -J /dev/disk%s ", pcLastDevPathName);
        printf("Execute %s:\n", pcCmd);
        iErr = system( pcCmd );
        if ( iErr != 0 )
        {
            exit(-1);
        }
        strcpy(pcDevPath, "/dev/rdisk");
        strcat(pcDevPath, pcLastDevPathName);
        printf("%s\n", pcDevPath);
        pcDevNum[0] = '\0';

    } else if (!strcmp(CREATE_HFS_DMG, psTestData->pcDMGPath)) {
        // No dmg filename provided. Create one:
        strcpy(pcCmd, "hdiutil create -size 20G -fs HFS+ -volname TwentyGigDmg ");
        strcat(pcCmd, TEMP_DMG);
        printf("Execute %s:\n", pcCmd);
        iErr = system( pcCmd );
        if ( iErr != 0 )
        {
            exit(-1);
        }

    } else if (psTestData->pcDMGPath[0] == '\0') {
        // No dmg filename provided. Create one:
        strcpy(pcCmd, "hdiutil create -size 20G -fs HFS+J -volname TwentyGigJournalDmg ");
        strcat(pcCmd, TEMP_DMG);
        printf("Execute %s:\n", pcCmd);
        iErr = system( pcCmd );
        if ( iErr != 0 )
        {
            exit(-1);
        }
    } else {
        // use dmg from provided path
         psTestData->bSparseImage = (strstr(psTestData->pcDMGPath, ".sparseimage") != NULL);
        // Copy dmg to tmp folder
        strcpy(pcCmd, "cp ");
        strcat(pcCmd, psTestData->pcDMGPath);
        strcat(pcCmd, " ");
        if (psTestData->bSparseImage) {
            strcat(pcCmd, TEMP_DMG_SPARSE);
        } else {
            strcat(pcCmd, TEMP_DMG);
        }
        printf("Execute %s:\n", pcCmd);
        iErr = system( pcCmd );
        if ( iErr != 0 )
        {
            exit(-1);
        }
    }

    if (!bMountedAlready) {
        // Attach DMG.
        strcpy(pcCmd, "hdiutil attach -nomount ");
        if (psTestData->bSparseImage) {
            strcat(pcCmd, TEMP_DMG_SPARSE);
        } else {
            strcat(pcCmd, TEMP_DMG);
        }
        strcat(pcCmd," > ");
        strcat(pcCmd, TEMP_DEV_PATH);
        printf("Execute %s:\n", pcCmd);
        iErr = system( pcCmd );
        if ( iErr != 0 )
        {
            exit(-1);
        }

        // Do we have multiple partitions?
        strcpy(pcCmd, "cat "TEMP_DEV_PATH" | grep Apple_HFS > "TEMP_DEV_PATH2);
        printf("Execute %s:\n", pcCmd);
        int iSinglePartition = system( pcCmd );

        if (iSinglePartition) {
            // Extract disk number (disk??)
            strcpy(pcCmd, "cat "TEMP_DEV_PATH" | sed 's/\\/dev\\/disk\\([0-9]*\\)/\\1/' | awk '{print $1}' > "TEMP_DEV_PATH2);
            printf("Execute %s:\n", pcCmd);
            iErr = system( pcCmd );
            if ( iErr != 0 )
            {
                exit(-1);
            }
            FILE *psCat = fopen(TEMP_DEV_PATH2, "r");
            fgets(pcLastDevPathName, sizeof(pcLastDevPathName), psCat);
            pclose(psCat);
            pcLastDevPathName[strlen(pcLastDevPathName)-1] = '\0';
            
            // Generate the full path
            pcDevNum[0] = '\0';
            strcpy(pcDevPath, "/dev/rdisk");
            strcat(pcDevPath, pcLastDevPathName);
            printf("%s\n", pcDevPath);
            
        } else { // Multilpe partitions
            // Extract disk number (disk??)
            strcpy(pcCmd, "cat "TEMP_DEV_PATH" | grep Apple_HFS | sed 's/\\/dev\\/disk\\([0-9]*\\)s[0-9]*/\\1/' | awk '{print $1}' > "TEMP_DEV_PATH2);
            printf("Execute %s:\n", pcCmd);
            iErr = system( pcCmd );
            if ( iErr != 0 )
            {
                exit(-1);
            }
            FILE *psCat = fopen(TEMP_DEV_PATH2, "r");
            fgets(pcLastDevPathName, sizeof(pcLastDevPathName), psCat);
            pclose(psCat);
            pcLastDevPathName[strlen(pcLastDevPathName)-1] = '\0';

            // Extract s number (s??)
            strcpy(pcCmd, "cat "TEMP_DEV_PATH" | grep Apple_HFS | sed 's/\\/dev\\/disk[0-9]*s\\([0-9]*\\)/\\1/' | awk '{print $1}' > "TEMP_DEV_PATH3);
            printf("Execute %s:\n", pcCmd);
            iErr = system( pcCmd );
            if ( iErr != 0 )
            {
                exit(-1);
            }
            psCat = fopen(TEMP_DEV_PATH3, "r");
            fgets(pcDevNum, sizeof(pcDevNum), psCat);
            pclose(psCat);
            pcDevNum[strlen(pcDevNum)-1] = '\0';

            // Generate the full path
            strcpy(pcDevPath, "/dev/rdisk");
            strcat(pcDevPath, pcLastDevPathName);
            strcat(pcDevPath, "s");
            strcat(pcDevPath, pcDevNum);
            printf("%s\n", pcDevPath);
        }

    }

    // Open file.
    printf("pcDevPath is %s\n", pcDevPath);
    int iFD = open( pcDevPath, O_RDWR );
    if ( iFD < 0 )
    {
        printf("Failed to open %s\n", pcDevPath);
        exit(EBADF);
    }

    free(pcCmd);

    return iFD;
}

static void
HFSTest_DestroyEnv(__unused int iFD )
{
    int iErr = 0;
    char* pcCmd = malloc(MAX_CMN_LEN);
    assert(pcCmd);

    // Detach DMG.
    memset(pcCmd, 0, MAX_CMN_LEN);
    strcat(pcCmd, "hdiutil detach /dev/disk");
    strcat(pcCmd, pcLastDevPathName);
    printf("Execute %s:\n", pcCmd);
    iErr = system(pcCmd);
    if ( iErr != 0 )
    {
        exit(-1);
    }

    // Remove old dmg if exist.
    memset(pcCmd, 0, MAX_CMN_LEN);
    strcat(pcCmd, "rm -rf ");
    strcat(pcCmd, TEMP_DEV_PATH);
    strcat(pcCmd, " ");
    strcat(pcCmd, TEMP_DEV_PATH2);
    printf("Execute %s:\n", pcCmd);
    system(pcCmd);

    free(pcCmd);
}

#if HFS_CRASH_TEST
void HFSTest_ClearCrashAbortFunctionArray(void) {
    for(unsigned u=0; u<CRASH_ABORT_LAST; u++)
        gpsCrashAbortFunctionArray[u] = NULL;
}

typedef struct {
    char pcMountedDev[512];
    char pcMountedVol[512];
} MountedDrive_S;

int HFSTest_KextMount(char *pcDmgFilename, MountedDrive_S *psMntd, bool bMount) {
    int iErr = 0;
    
    char pcMountCmd[512] = {0};
    strcat( pcMountCmd, "hdiutil attach ");
    if (!bMount) {
        strcat( pcMountCmd, "-nomount ");
    }
    strcat( pcMountCmd, "-plist ");
    strcat( pcMountCmd, pcDmgFilename );
    strcat( pcMountCmd, " > /tmp/tmp.txt ");
    printf("Execute %s:\n", pcMountCmd);
    iErr = system(pcMountCmd);
    printf("*** %s returned %d\n", pcMountCmd,iErr);
    if (iErr)
        return(iErr);
    
    char pcCatCmd[512] = {0};
    strcat( pcCatCmd, "cat /tmp/tmp.txt | grep '\\/dev\\/disk[0-9]*' | tr -d '\\t' | sed 's/<string>//' | sed 's/<\\/string>//' > /tmp/dev.txt");
    printf("Execute %s:\n", pcCatCmd);
    iErr = system(pcCatCmd);
    printf("returned %d\n", iErr);
    if (iErr)
        return(iErr);
    
    psMntd->pcMountedDev[0] = '\0';
    FILE *psCat = fopen("/tmp/dev.txt", "r");
    fscanf(psCat, "%s", psMntd->pcMountedDev);
    fclose(psCat);
    printf("pcMountedDev is %s\n", psMntd->pcMountedDev);

    strcpy( pcCatCmd, "cat /tmp/tmp.txt | grep '\\/Volumes' | tr -d '\\t' | sed 's/<string>//' | sed 's/<\\/string>//' > /tmp/vol.txt");
    printf("Execute %s:\n", pcCatCmd);
    iErr = system(pcCatCmd);
    printf("returned %d\n", iErr);
    if (iErr)
        return(iErr);

    psCat = fopen("/tmp/vol.txt", "r");
    strcpy(psMntd->pcMountedVol, "\"");
    fscanf(psCat, "%[^\n]", &psMntd->pcMountedVol[1]);
    strcat(psMntd->pcMountedVol, "\"");
    fclose(psCat);
    printf("pcMountedVol is %s\n", psMntd->pcMountedVol);

    return(iErr);
}

int HFSTest_KextFindAll(MountedDrive_S *psMntd, char *pcSearchFile) {
    int iErr = 0;

    char pcFindCmd[512] = {0};
    sprintf(pcFindCmd, "find %s | tee /tmp/file-list.txt", psMntd->pcMountedVol);
    printf("Execute %s :\n", pcFindCmd);
    iErr = system(pcFindCmd);
    printf("returned %d\n", iErr);
    if (iErr)
        return(iErr);

    sprintf(pcFindCmd, "cat /tmp/file-list.txt | grep %s", pcSearchFile);
    printf("Execute %s :\n", pcFindCmd);
    iErr = system(pcFindCmd);
    printf("returned %d\n", iErr);
    if (iErr)
        return(iErr);

    return(iErr);
}

int HFSTest_KextCount(MountedDrive_S *psMntd, char *pcSearchFile, uint32_t *puCount) {
    int iErr = 0;
    
    char pcFindCmd[512] = {0};
    sprintf(pcFindCmd, "find %s | tee /tmp/file-list.txt", psMntd->pcMountedVol);
    printf("Execute %s :\n", pcFindCmd);
    iErr = system(pcFindCmd);
    printf("returned %d\n", iErr);
    if (iErr)
        return(iErr);
    
    sprintf(pcFindCmd, "cat /tmp/file-list.txt | grep %s | wc -l > /tmp/word-count.txt", pcSearchFile);
    printf("Execute %s :\n", pcFindCmd);
    iErr = system(pcFindCmd);
    printf("returned %d\n", iErr);

    FILE *psCat = fopen("/tmp/word-count.txt", "r");
    fscanf(psCat, "%u", puCount);
    fclose(psCat);

    return(iErr);
}

int HFSTest_KextUnMount(MountedDrive_S *psMntd) {
    int iErr = 0;

    char pcUnMountCmd[512] = {0};
    strcat( pcUnMountCmd, "hdiutil detach ");
    strcat( pcUnMountCmd, psMntd->pcMountedDev );
    printf("Execute %s:\n", pcUnMountCmd);
    iErr = system(pcUnMountCmd);
    printf("returned %d\n", iErr);

    return(iErr);
}

int HFSTest_RunFsck(void) {
    int iErr = 0;
    char pcFsckCmd[512] = {0};
    
    strcat( pcFsckCmd, "/System/Library/Filesystems/hfs.fs/Contents/Resources/fsck_hfs -fd -D 0x22 /dev/disk");
    strcat( pcFsckCmd, pcLastDevPathName );
    
    if (pcDevNum[0] != '\0') {
        strcat( pcFsckCmd, "s" );
        strcat( pcFsckCmd, pcDevNum);
    }
    printf("Execute %s:\n", pcFsckCmd);

    iErr = system( pcFsckCmd );

    if (iErr) {
        printf( "*** Fsck CMD failed! (%d) \n", iErr);
    } else {
        printf( "*** Fsck CMD succeeded!\n");
    }
 
    return(iErr);
}

int HFSTest_RestartEnv(__unused int iFD) {
    // Restart FS for following tests
    HFS_fsOps.fsops_fini();

    int iErr = HFS_fsOps.fsops_init();
    printf("Init err [%d]\n", iErr);
    if (iErr) {
        printf("Can't re-init (%d).\n", iErr);
    }
    return(iErr);
}

int HFSTest_FailTestOnCrashAbort(__unused void *psTestData, CrashAbort_E eAbort, int iFD, UVFSFileNode psNode, __unused pthread_t bSyncerThread) {
    
    printf("**** HFSTest_FailTestOnCrashAbort: eAbort (%u) \"%s\", iFD %d, psNode %p ****\n", eAbort, ppcCrashAbortDesc[eAbort], iFD, psNode);
    
    if (eAbort != CRASH_ABORT_NONE) {
        panic("We should never get here!\n");
        return(-1);
    }
    
    close(iFD);
    // Seek & destroy
    HFSTest_DestroyEnv( iFD );
    
    return(0);
}

static int HFSTest_ConfirmTestFolderExists(UVFSFileNode RootNode ) {
    bool bFound;
    
    printf("HFSTest_ConfirmTestFolderExists:\n");
    bFound = false;
    char pcFolderName[256];
    for(unsigned u=0; u<5; u++) {
        sprintf(pcFolderName, "TestFolder_%u", u);
        read_directory_and_search_for_name( RootNode, pcFolderName, &bFound, NULL, 0 );
        if (!bFound) {
            printf("Error: Can not find replayed dir! (%s)\n", pcFolderName);
            return -1;
        } else {
            printf("dir %s found after journal replay.\n", pcFolderName);
        }
    }
    
    return 0;
}

static int HFSTest_ConfirmTestFolderDoesntExists(UVFSFileNode RootNode ) {
    bool bFound;
    
    printf("HFSTest_ConfirmTestFolderExists:\n");
    bFound = false;
    read_directory_and_search_for_name( RootNode, "TestFolder", &bFound, NULL, 0 );
    if (bFound) {
        printf("dir \"TestFolder\" found.\n");
        return -1;
    }

    printf("As expected, \"TestFolder\" was not found.\n");

    return 0;
}

int HFSTest_ValidateImageOnMac(__unused TestData_S *psTestData, char *pcDmgFilename, char *pcSearchItem, bool bFindNotFind) {
    int iErr = 0;
    MountedDrive_S sMntd;

    // Validate image with fsck + Kext mount
    // attach -nomount
    iErr = HFSTest_KextMount(pcDmgFilename, &sMntd, false);
    if (iErr) {
        printf("Can't HFSTest_KextMount(false).\n");
        goto exit;
    }

    // Run FSCK
    iErr = HFSTest_RunFsck();
    if (iErr) {
        printf("Can't HFSTest_RunFsck.\n");
        goto exit;
    }
    
    // detach
    iErr = HFSTest_KextUnMount(&sMntd);
    if (iErr) {
        printf("Can't HFSTest_KextUnMount.\n");
        goto exit;
    }

    // Validate that we can mount
    iErr = HFSTest_KextMount(pcDmgFilename, &sMntd, true);
    if (iErr) {
        printf("Can't HFSTest_KextMount(true).\n");
        goto exit;
    }
    
    if (pcSearchItem) {
        char pcSearchPath[512] = {0};
        strcpy(pcSearchPath, sMntd.pcMountedVol);
        strcat(pcSearchPath, pcSearchItem);
        printf("pcSearchPath is %s\n", pcSearchPath);

        iErr = HFSTest_KextFindAll(&sMntd, pcSearchPath);
        printf("grep returned %d.\n", iErr);
        if (bFindNotFind == false) {
            // Make sure string was not found on drive
            if (iErr == 256) {
                iErr = 0;
            } else {
                iErr = 1;
            }
        }
        if (iErr) {
            goto exit;
        }
        
        // Count SymLinks
        uint32_t uNumOfSymLinks = 0;
        strcpy(pcSearchPath, "TestSymLink_thread");
        iErr = HFSTest_KextCount(&sMntd, pcSearchPath, &uNumOfSymLinks);
        printf("*** found %u SymLinks\n", uNumOfSymLinks);
        
        // Count Files
        uint32_t uNumOfFiles = 0;
        strcpy(pcSearchPath, "file_Thread_");
        iErr = HFSTest_KextCount(&sMntd, pcSearchPath, &uNumOfFiles);
        printf("*** found %u files\n", uNumOfFiles);

    }
    
    iErr = HFSTest_KextUnMount(&sMntd);
    if (iErr) {
        goto exit;
    }
    
exit:
    return(iErr);
}

int HFSTest_SaveDMG(void *pvTestData, CrashAbort_E eAbort, int iFD, UVFSFileNode psNode, __unused pthread_t pSyncerThread) {
    int iErr = 0;
    TestData_S *psTestData = pvTestData;
    
    printf("**** HFSTest_SaveDMG: eAbort (%u) \"%s\", psNode %p ****\n", eAbort, ppcCrashAbortDesc[eAbort], psNode);
    
    close(iFD);
    // Seek & destroy
    HFSTest_DestroyEnv( iFD );
    
    // Create a snapshot of the dmg
    char pcDmgFilename[256];
    strcpy(pcDmgFilename, psTestData->bSparseImage?TEMP_DMG_SPARSE:TEMP_DMG);
    
    char pcCpCmd[512] = {0};
    strcat( pcCpCmd, "cp ");
    strcat( pcCpCmd, pcDmgFilename);
    if (psTestData->bSparseImage) {
        strcat( pcCpCmd, " "TEMP_DMG_BKUP_SPARSE);
    } else {
        strcat( pcCpCmd, " "TEMP_DMG_BKUP);
    }
    printf("Execute %s:\n", pcCpCmd);
    iErr = system(pcCpCmd);
    printf("returned %d\n", iErr);
    if (iErr) {
        goto exit;
    }
exit:
    return(iErr);
}

int HFSTest_CrashAbortAtRandom(void *pvTestData, CrashAbort_E eAbort, int iFD, UVFSFileNode psNode, __unused pthread_t pSyncerThread) {
    int iErr = 0;
    TestData_S *psTestData = pvTestData;
    
    printf("**** HFSTest_CrashAbortAtRandom: eAbort (%u) \"%s\", psNode %p ****\n", eAbort, ppcCrashAbortDesc[eAbort], psNode);
    
    close(iFD);
    // Seek & destroy
    HFSTest_DestroyEnv( iFD );
    
    // Create a snapshot of the crashed dmg
    char pcDmgFilename[256];
    strcpy(pcDmgFilename, psTestData->bSparseImage?TEMP_DMG_SPARSE:TEMP_DMG);
    
    char pcCpCmd[512] = {0};
    strcat( pcCpCmd, "cp ");
    strcat( pcCpCmd, pcDmgFilename);
    if (psTestData->bSparseImage) {
        strcat( pcCpCmd, " "TEMP_DMG_BKUP_SPARSE);
    } else {
        strcat( pcCpCmd, " "TEMP_DMG_BKUP);
    }
    printf("Execute %s:\n", pcCpCmd);
    iErr = system(pcCpCmd);
    printf("returned %d\n", iErr);
    if (iErr) {
        goto exit;
    }
    
    bool bFindNotFind = true;
    if (eAbort == CRASH_ABORT_JOURNAL_AFTER_JOURNAL_HEADER) {
        bFindNotFind = true;
    } else {
        bFindNotFind = false;
    }
    
    iErr = HFSTest_ValidateImageOnMac(psTestData, pcDmgFilename, "/TestFolder", bFindNotFind);
    if (iErr) {
        goto exit;
    }
    
exit:
    return(iErr);
}

int HFSTest_CrashAbortOnMkDir(void *pvTestData, CrashAbort_E eAbort, int iFD, UVFSFileNode psNode, __unused pthread_t pSyncerThread) {
    int iErr = 0;
    TestData_S *psTestData = pvTestData;
    
    printf("**** HFSTest_CrashAbortOnMkDir: eAbort (%u) \"%s\", psNode %p ****\n", eAbort, ppcCrashAbortDesc[eAbort], psNode);
    
    close(iFD);
    // Seek & destroy
    HFSTest_DestroyEnv( iFD );

    // Create a snapshot of the crashed dmg
    char pcDmgFilename[256];
    strcpy(pcDmgFilename, psTestData->bSparseImage?TEMP_DMG_SPARSE:TEMP_DMG);
    
    char pcCpCmd[512] = {0};
    strcat( pcCpCmd, "cp ");
    strcat( pcCpCmd, pcDmgFilename);
    if (psTestData->bSparseImage) {
        strcat( pcCpCmd, " "TEMP_DMG_BKUP_SPARSE);
    } else {
        strcat( pcCpCmd, " "TEMP_DMG_BKUP);
    }
    printf("Execute %s:\n", pcCpCmd);
    iErr = system(pcCpCmd);
    printf("returned %d\n", iErr);
    if (iErr) {
        goto exit;
    }
    
    bool bFindNotFind = true;
    if (eAbort == CRASH_ABORT_JOURNAL_AFTER_JOURNAL_HEADER) {
        printf("CRASH_ABORT_JOURNAL_AFTER_JOURNAL_HEADER, expecting to find!\n");
        bFindNotFind = true;
    } else {
        bFindNotFind = false;
    }

    iErr = HFSTest_ValidateImageOnMac(psTestData, pcDmgFilename, "/TestFolder", bFindNotFind);
    if (iErr) {
        goto exit;
    }

exit:
    printf("HFSTest_CrashAbortOnMkDir returns %d\n", iErr);
    return(iErr);
}

int HFSTest_CrashAbort(void *pvTestData, CrashAbort_E eAbort, int iFD, UVFSFileNode psNode, __unused pthread_t pSyncerThread) {
    int iErr = 0;
    TestData_S *psTestData = pvTestData;
    
    printf("**** HFSTest_CrashAbort: eAbort (%u) \"%s\", psNode %p ****\n", eAbort, ppcCrashAbortDesc[eAbort], psNode);

    close(iFD);
    // Seek & destroy
    HFSTest_DestroyEnv( iFD );
    
    // Create a snapshot of the crashed dmg
    char pcDmgFilename[256];
    strcpy(pcDmgFilename, psTestData->bSparseImage?TEMP_DMG_SPARSE:TEMP_DMG);
    
    char pcCpCmd[512] = {0};
    strcat( pcCpCmd, "cp ");
    strcat( pcCpCmd, pcDmgFilename);
    if (psTestData->bSparseImage) {
        strcat( pcCpCmd, " "TEMP_DMG_BKUP_SPARSE);
    } else {
        strcat( pcCpCmd, " "TEMP_DMG_BKUP);
    }
    printf("Execute %s:\n", pcCpCmd);
    iErr = system(pcCpCmd);
    printf("returned %d\n", iErr);
    if (iErr) {
        goto exit;
    }

    iErr = HFSTest_ValidateImageOnMac(psTestData, pcDmgFilename, NULL, false);
    if (iErr) {
        goto exit;
    }
    
exit:
    return(iErr);
}
#endif

static int
HFSTest_ScanID( UVFSFileNode RootNode )
{
    int iErr = 0;
    printf("HFSTest_ScanID\n");
    __block uint64_t uScanIDFileIDArray;
    __block char* pcScanIDPath = malloc(sizeof(char)* MAX_UTF8_NAME_LENGTH);
    __block char* pcTempPath;
    __block int iFoundRoot = 0;

    //Creating the following path
    // /TestFolder/TestFolder2/TestFolder3/TestFolder4/TestFolder5/file.txt
    UVFSFileNode TestFolder  = NULL;
    UVFSFileNode TestFolder2 = NULL;
    UVFSFileNode TestFolder3 = NULL;
    UVFSFileNode TestFolder4 = NULL;
    UVFSFileNode TestFolder5 = NULL;
    UVFSFileNode TestFile1   = NULL;

    iErr = CreateNewFolder( RootNode, &TestFolder, "TestFolder");
    printf("CreateNewFolder TestFolder err [%d]\n", iErr);
    if (iErr) goto exit;

    iErr = CreateNewFolder( TestFolder, &TestFolder2, "TestFolder2");
    printf("CreateNewFolder TestFolder2 err [%d]\n", iErr);
    if (iErr) goto exit;

    iErr = CreateNewFolder( TestFolder2, &TestFolder3, "TestFolder3");
    printf("CreateNewFolder TestFolder3 err [%d]\n", iErr);
    if (iErr) goto exit;

    iErr = CreateNewFolder( TestFolder3, &TestFolder4, "TestFolder4");
    printf("CreateNewFolder TestFolder4 err [%d]\n", iErr);
    if (iErr) goto exit;

    iErr = CreateNewFolder( TestFolder4, &TestFolder5, "TestFolder5");
    printf("CreateNewFolder TestFolder5 err [%d]\n", iErr);
    if (iErr) goto exit;

    //Create new file with size 0
    iErr = CreateNewFile(TestFolder5, &TestFile1, "file.txt",512);
    printf("Create file.txt in TestFolder5 err [%d]\n", iErr);
    if (iErr) goto exit;

    LIFileAttributes_t FileAttr;
    iErr = HFS_fsOps.fsops_getattr( TestFile1, &FileAttr);
    if (iErr) goto exit;

    HFS_fsOps.fsops_reclaim(TestFile1);
    
    memset(pcScanIDPath, 0, MAX_UTF8_NAME_LENGTH);
    uScanIDFileIDArray = FileAttr.fa_fileid;

    do
    {
        iErr = HFS_fsOps.fsops_scanids(RootNode, 0, &uScanIDFileIDArray, 1,
        ^(__unused unsigned int fileid_index, const UVFSFileAttributes *file_attrs, const char *file_name) {
            iFoundRoot = (file_attrs->fa_parentid == file_attrs->fa_fileid);
            uScanIDFileIDArray = file_attrs->fa_parentid;
            size_t uTmpPathSize = strlen(pcScanIDPath) + 1;
            pcTempPath = malloc(uTmpPathSize);
            strlcpy(pcTempPath, pcScanIDPath, uTmpPathSize);
            strlcpy(pcScanIDPath, file_name, MAX_UTF8_NAME_LENGTH);

            if (uTmpPathSize != 1) { //For the first time we don't want to set /
                strcat(pcScanIDPath,"/");
                strcat(pcScanIDPath,pcTempPath);
            }

            free(pcTempPath);
        });
        printf("HFS_fsOps.fsops_scanids err [%d]\n", iErr);
        if (iErr) goto exit;
    } while (!iFoundRoot);

    if (strcmp(pcScanIDPath,"/TestFolder/TestFolder2/TestFolder3/TestFolder4/TestFolder5/file.txt"))
    {
        iErr = EFAULT;
        printf("Found path to file [%s]\n", pcScanIDPath);
    }

    // *********************  Add Hard Links: ************************
    uint32_t uOriginalFileSize = 500000;
    UVFSFileNode psFile     = NULL;
    size_t iActuallyWrite   = 0;
    size_t iActuallyRead    = 0;
    void* pvOutBuf          = malloc(uOriginalFileSize);
    void* pvInBuf           = malloc(uOriginalFileSize);

    if (pvOutBuf == NULL || pvInBuf == NULL) {
        printf("ERROR: HFSTest_ScanID: can't malloc (%p, %p)\n", pvOutBuf,  pvInBuf);
        iErr = -1;
        goto exit;
    }

    uint64_t* puOutBuf  = pvOutBuf;
    uint64_t* puInBuf   = pvInBuf;

    // Create the original file with size 500,000 Bytes
    iErr = CreateNewFile( RootNode, &psFile, "original_file.txt", uOriginalFileSize);
    if (iErr) {
        printf("ERROR: CreateNewFile return %d\n", iErr);
        iErr = -1;
        goto exit;
    }

    // lets write 10,000 Bytes with 0xCD
    memset(pvOutBuf, 0, uOriginalFileSize);
    memset(pvInBuf,  0, uOriginalFileSize);

    memset(pvOutBuf, 0xCD, uOriginalFileSize);

    iErr = HFS_fsOps.fsops_write( psFile, 0, uOriginalFileSize, pvOutBuf, &iActuallyWrite );
    if (iErr) {
        printf("ERROR: fsops_write return %d\n", iErr);
        goto exit;
    }

    iErr = HFS_fsOps.fsops_read( psFile, 0, uOriginalFileSize, pvInBuf, &iActuallyRead );
    if (iErr) {
        printf("ERROR: fsops_read return %d\n", iErr);
        goto exit;
    }

    // Lets test it...
    for ( uint64_t uIdx=0; uIdx<(uOriginalFileSize/sizeof(uint64_t)); uIdx++ ) {
        if (puInBuf[uIdx] != puOutBuf[uIdx] ) {
            printf("ERROR: puInBuf[uIdx] != puOutBuf[uIdx]\n");
            iErr = -1;
            goto exit;
        }
    }

    UVFSFileNode psDirectory = NULL;
    iErr = CreateNewFolder(RootNode,&psDirectory,"dir");
    if (iErr) {
        printf("ERROR: CreateNewFolder return %d\n", iErr);
        goto exit;
    }
    iErr = CreateHardLink(psFile,RootNode,"first_link.txt");
    if (iErr) {
        printf("ERROR: CreateHardLink return %d\n", iErr);
        goto exit;
    }
    iErr = CreateHardLink(psFile,psDirectory,"second_link.txt");
    if (iErr) {
        printf("ERROR: CreateHardLink return %d\n", iErr);
        goto exit;
    }

    UVFSFileNode psFirstLink = NULL;
    iErr = HFS_fsOps.fsops_lookup( psDirectory, "second_link.txt", &psFirstLink );
    if (iErr) goto exit;

    iErr = HFS_fsOps.fsops_getattr( psFirstLink, &FileAttr);
    if (iErr) goto exit;

    uScanIDFileIDArray = FileAttr.fa_fileid;
    memset(pcScanIDPath, 0, MAX_UTF8_NAME_LENGTH);

    HFS_fsOps.fsops_reclaim( psFile );
    HFS_fsOps.fsops_reclaim( psFirstLink );
    HFS_fsOps.fsops_reclaim( psDirectory );

    // Now run SanID on hardlinks:
    do {
        iErr = HFS_fsOps.fsops_scanids(RootNode, 0, &uScanIDFileIDArray, 1,
        ^(__unused unsigned int fileid_index, const UVFSFileAttributes *file_attrs, const char *file_name) {
            iFoundRoot = (file_attrs->fa_parentid == file_attrs->fa_fileid);
            uScanIDFileIDArray = file_attrs->fa_parentid;
            size_t uTmpPathSize = strlen(pcScanIDPath) + 1;
            pcTempPath = malloc(uTmpPathSize);
            strlcpy(pcTempPath, pcScanIDPath, uTmpPathSize);
            strlcpy(pcScanIDPath, file_name, MAX_UTF8_NAME_LENGTH);

            if (uTmpPathSize != 1) { //For the first time we don't want to set /
                strcat(pcScanIDPath,"/");
                strcat(pcScanIDPath,pcTempPath);
            }

            free(pcTempPath);
        });
        printf("HFS_fsOps.fsops_scanids err [%d]\n", iErr);
        if (iErr) goto exit;
    } while (!iFoundRoot);

    if (strcmp(pcScanIDPath,"/original_file.txt"))
    {
        iErr = EFAULT;
        printf("Found path to file [%s]\n", pcScanIDPath);
    }

exit:
    free(pcScanIDPath);

    if (TestFile1) {
        RemoveFile(TestFolder5,"file.txt");
    }
    if (TestFolder5) {
        HFS_fsOps.fsops_reclaim(TestFolder5);
        RemoveFolder(TestFolder4,"TestFolder5");
    }
    if (TestFolder4) {
        HFS_fsOps.fsops_reclaim(TestFolder4);
        RemoveFolder(TestFolder3,"TestFolder4");
    }
    if (TestFolder3) {
        HFS_fsOps.fsops_reclaim(TestFolder3);
        RemoveFolder(TestFolder2,"TestFolder3");
    }
    if (TestFolder2) {
        HFS_fsOps.fsops_reclaim(TestFolder2);
        RemoveFolder(TestFolder,"TestFolder2");
    }
    if (TestFolder) {
        HFS_fsOps.fsops_reclaim(TestFolder);
        RemoveFolder(RootNode,"TestFolder");
    }

    return iErr;
}

static int
HFSTest_Create( UVFSFileNode RootNode )
{
    int iErr = 0;
    printf("HFSTest_Create\n");

    UVFSFileNode TestFolder = NULL;
    iErr = CreateNewFolder( RootNode, &TestFolder, "TestFolder");
    printf("CreateNewFolder err [%d]\n", iErr);
    if (iErr)
        return iErr;
    
    //Create new file with size 0
    UVFSFileNode TestFile1 = NULL;
    CreateNewFile(TestFolder, &TestFile1, "TestFile",0);
    printf("Create TestFile in TestFolder err [%d]\n", iErr);
    if (iErr)
        return iErr;

    //Create new file with size 512
    UVFSFileNode TestFile2 = NULL;
    CreateNewFile(TestFolder, &TestFile2, "TestFile2",512);
    printf("Create TestFile2 in TestFolder err [%d]\n", iErr);
    if (iErr)
        return iErr;

    uint32_t uEntrySize = sizeof(UVFSDirEntryAttr) + MAX_UTF8_NAME_LENGTH;
    UVFSDirEntryAttr *psReadDirTestsData = malloc(2*uEntrySize);
    if (psReadDirTestsData == NULL)
        return ENOMEM;

    UVFSDirEntryAttr *psCurrentReadDirTestsData = psReadDirTestsData;
    SetExpectedAttr("TestFile", UVFS_FA_TYPE_FILE, psCurrentReadDirTestsData);
    iErr = HFS_fsOps.fsops_getattr( TestFile1, &psCurrentReadDirTestsData->dea_attrs );
    psCurrentReadDirTestsData = (UVFSDirEntryAttr *) ((void*) psCurrentReadDirTestsData + uEntrySize);
    SetExpectedAttr("TestFile2", UVFS_FA_TYPE_FILE, psCurrentReadDirTestsData);
    iErr = HFS_fsOps.fsops_getattr( TestFile2, &psCurrentReadDirTestsData->dea_attrs );

//    {
//        {.pcTestName = "TestFile",  .uTyppe = UVFS_FA_TYPE_FILE, .uSize = 0, .uNlink = 1, .uAllocatedSize = 0},
//        {.pcTestName = "TestFile2", .uTyppe = UVFS_FA_TYPE_FILE, .uSize = 512, .uNlink = 1, .uAllocatedSize = 4096},
//    };

    iErr = ReadDirAttr(TestFolder, psReadDirTestsData, 2);
    free(psReadDirTestsData);
    printf("ReadDirAttr err [%d]\n", iErr);
    if (iErr)
        goto exit;

    // Remove File1
    iErr =  RemoveFile(TestFolder,"TestFile");
    printf("Remove File TestFile from TestFolder err [%d]\n", iErr);
    if (iErr)
        goto exit;

    // Remove File2
    iErr =  RemoveFile(TestFolder,"TestFile2");
    printf("Remove File TestFile2 from TestFolder err [%d]\n", iErr);
    if (iErr)
        goto exit;

    // Remove TestFolder
    iErr =  RemoveFolder(RootNode,"TestFolder");
    printf("Remove Folder TestFolder from Root err [%d]\n", iErr);
    if (iErr)
        goto exit;

exit:
    HFS_fsOps.fsops_reclaim(TestFolder);
    HFS_fsOps.fsops_reclaim(TestFile1);
    HFS_fsOps.fsops_reclaim(TestFile2);
    return iErr;
}

// This function tests the system behavior when deleting a very large defragmented file,
// which will cause the creation of a very large journal transaction and lots of BT buffers
static int HFSTest_DeleteAHugeDefragmentedFile_wJournal(UVFSFileNode RootNode) {
    #define DHF_NUM_OF_FILES_TO_CREATE 1000
    #define DHF_HUGE_FILE_SIZE        (15000000000ULL) // 15GBytes
    #define DHF_SMALL_FILENAME         "SmallFile"
    #define DHF_HUGE_FILENAME          "HugeFile"

    int iErr = 0;
    
    // Create many small files
    char pcName[100] = {0};
    UVFSFileNode psNode;
    for ( int i=0; i<DHF_NUM_OF_FILES_TO_CREATE; i++ ) {
        sprintf(pcName, "%s_%u.txt", DHF_SMALL_FILENAME, i);
        printf("Creating file %s\n", pcName);
        if ( (iErr = CreateNewFile(RootNode, &psNode, pcName, 100)) != 0 ) {
            printf("Failed to create file [%s]\n", pcName);
            goto exit;
        }
        HFS_fsOps.fsops_reclaim(psNode);
    }
    
    // delete every other file
    for ( int i=0; i<DHF_NUM_OF_FILES_TO_CREATE; i+=2 ) {
        sprintf(pcName, "%s_%u.txt", DHF_SMALL_FILENAME, i);
        printf("Removing file %s\n", pcName);
        if ( (iErr = RemoveFile(RootNode, pcName)) != 0 ) {
            printf("Failed to remove file [%s]", pcName);
            goto exit;
        }
    }
    
    // Create a huge file
    sprintf(pcName, "%s.txt", DHF_HUGE_FILENAME);
    printf("Creating huge file %s\n", pcName);
    if ( (iErr = CreateNewFile(RootNode, &psNode, pcName, DHF_HUGE_FILE_SIZE)) != 0 ) {
        printf("Failed to create file [%s]\n", pcName);
        goto exit;
    }
    HFS_fsOps.fsops_reclaim(psNode);
    
    // Delete the huge defragmented file
    sprintf(pcName, "%s.txt", DHF_HUGE_FILENAME);;
    printf("Removing file %s\n", pcName);
    if ( (iErr = RemoveFile(RootNode, pcName)) != 0 ) {
        printf("Failed to remove file [%s]", pcName);
        goto exit;
    }

exit:
    return iErr;
}


static void *ReadWriteThread(void *pvArgs) {
    int iErr = 0;
    
    RWThreadData_S *psThrdData = pvArgs;
    UVFSFileNode   psRootNode  = psThrdData->psRootNode;
    char pcName[100] = {0};
    char *pcFilenameFormat = "file_Thread_%u_OpCnt_%u_FileNum_%u_Len_%u.txt";

    for(uint32_t uOpCnt=0; uOpCnt<MTRW_NUM_OF_OPERATIONS; uOpCnt++) {

        // Create files
        for(uint32_t uNumOfFiles=0; uNumOfFiles<psThrdData->uNumOfFiles; uNumOfFiles++) {

            UVFSFileNode psNode;
            uint64_t uLen = psThrdData->uFileSize * uNumOfFiles;
            sprintf(pcName, pcFilenameFormat, psThrdData->uThreadNum, uOpCnt, uNumOfFiles, uLen);
            printf("Creating file %s\n", pcName);
            iErr = CreateNewFile(psRootNode, &psNode, pcName, uLen);
            if (iErr) {
                printf("Failed creating file %s with iErr %d.\n", pcName,iErr);
                goto exit;
            }
            HFS_fsOps.fsops_reclaim(psNode);
        }

        // Create SymLinks
        UVFSFileNode *pOutNode = malloc(sizeof(UVFSFileNode) * psThrdData->uNumOfSymLinks);
        uint32_t uSymLinkMode = UVFS_FA_MODE_USR(UVFS_FA_MODE_RWX) | UVFS_FA_MODE_GRP(UVFS_FA_MODE_R) | UVFS_FA_MODE_OTH(UVFS_FA_MODE_R | UVFS_FA_MODE_X);
        for(uint32_t uNumOfSymLinks=0; uNumOfSymLinks<psThrdData->uNumOfSymLinks; uNumOfSymLinks++) {
            
            // Create Symlink
            char pcSymLinkName[256] = {0};
            sprintf(pcSymLinkName, "TestSymLink_thread_%u_op_%u_symlink_%u", psThrdData->uThreadNum, uOpCnt, uNumOfSymLinks);
            uint32_t uSymLinkSize = psThrdData->uSymLinkSize;
            char *pcSymLinkContent = malloc(uSymLinkSize);
            memset(pcSymLinkContent, 0, uSymLinkSize);
            uint32_t uStampLen = sprintf(pcSymLinkContent, "/just/to/check/that/symlink/works/properly/thread/%u/op/%u/symlink_%u",psThrdData->uThreadNum, uOpCnt, uNumOfSymLinks);
            assert(uSymLinkSize >= uStampLen);
            for(uint32_t uStampCount=1; uSymLinkSize>(uStampCount+1)*uStampLen+1; uStampCount++) {
                memcpy(pcSymLinkContent + uStampCount*uStampLen, pcSymLinkContent, uStampLen);
            }
            assert(strlen(pcSymLinkContent) + 1 <=  uSymLinkSize);
            
            UVFSFileAttributes sAttr = {0};
            sAttr.fa_validmask       = UVFS_FA_VALID_MODE;
            sAttr.fa_type            = UVFS_FA_TYPE_SYMLINK;
            sAttr.fa_mode            = uSymLinkMode;
            
            pOutNode[uNumOfSymLinks] = NULL;
            printf("Creating Symlink %s\n", pcSymLinkName);
            iErr = HFS_fsOps.fsops_symlink(psRootNode, pcSymLinkName, pcSymLinkContent, &sAttr, &pOutNode[uNumOfSymLinks] );
            if ( iErr != 0 ) {
                printf( "fsops_symlink failed with eror code : %d\n", iErr );
                goto exit;
            }
            free(pcSymLinkContent);
        }
        
        // Verify Symlink content
        for(uint32_t uNumOfSymLinks=0; uNumOfSymLinks<psThrdData->uNumOfSymLinks; uNumOfSymLinks++) {
            
            uint32_t uSymLinkSize = psThrdData->uSymLinkSize;
            char pcSymLinkName[256] = {0};
            sprintf(pcSymLinkName, "TestSymLink_thread_%u_op_%u_symlink_%u", psThrdData->uThreadNum, uOpCnt, uNumOfSymLinks);
            char *pcSymLinkReadContent = malloc(uSymLinkSize);
            memset(pcSymLinkReadContent, 0, uSymLinkSize);
            size_t iActuallyRead;
            
            UVFSFileAttributes sOutAttr = {0};
            
            printf("Reading Symlink %s\n", pcSymLinkName);
            iErr = HFS_fsOps.fsops_readlink( pOutNode[uNumOfSymLinks], pcSymLinkReadContent, uSymLinkSize, &iActuallyRead, &sOutAttr );
            
            if ( iErr != 0 ) {
                printf( "fsops_readlink failed with eror code : %d\n", iErr );
                goto exit;
            }
            
            char *pcSymLinkContent = malloc(uSymLinkSize);
            memset(pcSymLinkContent, 0, uSymLinkSize);
            uint32_t uStampLen = sprintf(pcSymLinkContent, "/just/to/check/that/symlink/works/properly/thread/%u/op/%u/symlink_%u",psThrdData->uThreadNum, uOpCnt, uNumOfSymLinks);
            assert(uSymLinkSize >= uStampLen);
            for(uint32_t uStampCount=1; uSymLinkSize>(uStampCount+1)*uStampLen+1; uStampCount++) {
                memcpy(pcSymLinkContent + uStampCount*uStampLen, pcSymLinkContent, uStampLen);
            }

            if ( memcmp( pcSymLinkContent, pcSymLinkReadContent, uSymLinkSize) != 0 ) {
                printf( "Read bad symlink content\n" );
                iErr = -1;
                goto exit;
            }
            
            if ( sOutAttr.fa_mode != uSymLinkMode) {
                printf( "Mode mismatch [%d != %d]\n", sOutAttr.fa_mode, uSymLinkMode);
                iErr = -1;
                goto exit;
            }
            
            if ( sOutAttr.fa_type != UVFS_FA_TYPE_SYMLINK ) {
                printf( "Type mismatch\n" );
                iErr = -1;
                goto exit;
            }
            
            HFS_fsOps.fsops_reclaim( pOutNode[uNumOfSymLinks] );
            free(pcSymLinkContent);
            free(pcSymLinkReadContent);
        }
        free(pOutNode);
        
        // Remove files
        for(uint32_t uNumOfFiles=0; uNumOfFiles<psThrdData->uNumOfFiles; uNumOfFiles++) {
            uint64_t uLen = psThrdData->uFileSize * uNumOfFiles;
            sprintf(pcName, pcFilenameFormat, psThrdData->uThreadNum, uOpCnt, uNumOfFiles, uLen);
            printf("Removing file %s\n", pcName);
            iErr = RemoveFile(psRootNode, pcName);
            if (iErr) {
                printf("Failed deleting file %s with iErr %d.\n", pcName, iErr);
                goto exit;
            }
        }

        // Remove SymLinks
        for(uint32_t uNumOfSymLinks=0; uNumOfSymLinks<psThrdData->uNumOfSymLinks; uNumOfSymLinks++) {
            
            char pcSymLinkName[256] = {0};
            sprintf(pcSymLinkName, "TestSymLink_thread_%u_op_%u_symlink_%u", psThrdData->uThreadNum, uOpCnt, uNumOfSymLinks);
            
            printf("Deleting Symlink %s\n", pcSymLinkName);
            iErr = HFS_fsOps.fsops_remove(psRootNode, pcSymLinkName, NULL);
            if ( iErr != 0 ) {
                printf( "Failed to remove symlink %d\n", iErr );
                goto exit;
            }
        }
    }
exit:
    psThrdData->iRetVal = iErr;
    return psThrdData;
}

#if HFS_CRASH_TEST
static int MultiThreadedRW_wJournal_RandomCrash(UVFSFileNode psRootNode) {
    int iErr = 0;

    pthread_attr_t sAttr;
    pthread_attr_setdetachstate(&sAttr, PTHREAD_CREATE_JOINABLE);
    pthread_attr_init(&sAttr);
    pthread_t psExecThread[MTRW_NUM_OF_THREADS];
    RWThreadData_S pcThreadData[MTRW_NUM_OF_THREADS] = {{0}};
    for(uint32_t u = 0; u < MTRW_NUM_OF_THREADS; u++) {
        pcThreadData[u].uThreadNum     = u;
        pcThreadData[u].psRootNode     = psRootNode;
        pcThreadData[u].uNumOfFiles    = MTRW_NUM_OF_FILES*(u+1);
        pcThreadData[u].uFileSize      = MTRW_FILE_SIZE;
        pcThreadData[u].uNumOfSymLinks = MTRW_NUM_OF_SYMLINKS*(u+1);
        pcThreadData[u].uSymLinkSize   = MTRW_SYMLINK_SIZE;
        
        iErr = pthread_create(&psExecThread[u], &sAttr, ReadWriteThread, &pcThreadData[u]);
        if (iErr) {
            printf("can't pthread_create\n");
            goto exit;
        }
    }
    pthread_attr_destroy(&sAttr);
    
    time_t sTime;
    srand((unsigned) time(&sTime));
    uint32_t uRandTime = 0;
    if (guSyncerPeriod) {
        uRandTime = rand() % (guSyncerPeriod * 150);
    } else {
        uRandTime = rand() % (15000);
    }
    printf("******* uRandTime is %u mS ******\n", uRandTime);
    
    uint32_t uAbortTime = uRandTime; // mSec
    usleep(uAbortTime * 1000);
    close(giFD);
    printf("******* now: close(giFD) **************\n");

    for(uint32_t u = 0; u < MTRW_NUM_OF_THREADS; u++) {
        iErr = pthread_join(psExecThread[u], NULL);
        if (iErr) {
            printf("can't pthread_join\n");
            goto exit;
        }
        iErr = pcThreadData[u].iRetVal;
        if (iErr) {
            printf("Thread %u return error %d\n", u, iErr);
        }
    }
    
exit:
    return iErr;

}
#endif

static int HFSTest_MultiThreadedRW_wJournal(UVFSFileNode psRootNode) {
    
    int iErr = 0;

    pthread_attr_t sAttr;
    pthread_attr_setdetachstate(&sAttr, PTHREAD_CREATE_JOINABLE);
    pthread_attr_init(&sAttr);
    pthread_t psExecThread[MTRW_NUM_OF_THREADS];
    RWThreadData_S pcThreadData[MTRW_NUM_OF_THREADS] = {{0}};
    for(uint32_t u = 0; u < MTRW_NUM_OF_THREADS; u++) {
        pcThreadData[u].uThreadNum     = u;
        pcThreadData[u].psRootNode     = psRootNode;
        pcThreadData[u].uNumOfFiles    = MTRW_NUM_OF_FILES*(u+1);
        pcThreadData[u].uFileSize      = MTRW_FILE_SIZE;
        pcThreadData[u].uNumOfSymLinks = MTRW_NUM_OF_SYMLINKS;
        pcThreadData[u].uSymLinkSize   = MTRW_SYMLINK_SIZE/3;

        iErr = pthread_create(&psExecThread[u], &sAttr, ReadWriteThread, &pcThreadData[u]);
        if (iErr) {
            printf("can't pthread_create\n");
            goto exit;
        }
    }
    pthread_attr_destroy(&sAttr);

    for(uint32_t u = 0; u < MTRW_NUM_OF_THREADS; u++) {
        iErr = pthread_join(psExecThread[u], NULL);
        if (iErr) {
            printf("can't pthread_join\n");
            goto exit;
        }
        iErr = pcThreadData[u].iRetVal;
        if (iErr) {
            printf("Thread %u return error %d\n", u, iErr);
        }
    }

exit:
    return iErr;
}

static int
HFSTest_Create1000Files( UVFSFileNode RootNode )
{
    #define CREATE_NUM_OF_FILES    (1000)
    #define FILE_NAME       "Iamjustasimplefile_%d"
    
    int iErr = 0;

    char pcName[100] = {0};
    UVFSFileNode psNode;
    for ( int i=0; i<CREATE_NUM_OF_FILES; i++ )
    {
        sprintf(pcName, FILE_NAME, i);
        printf("Creating file %s\n", pcName);
        if ( (iErr = CreateNewFile(RootNode, &psNode, pcName, 0)) != 0 )
        {
            printf("Failed to create file [%s]\n", pcName);
            goto exit;
        }
        HFS_fsOps.fsops_reclaim(psNode);
    }

    for ( int i=0; i<CREATE_NUM_OF_FILES; i++ )
    {
        sprintf(pcName, FILE_NAME, i);
        printf("Removing file %s\n", pcName);
        if ( (iErr = RemoveFile(RootNode, pcName)) != 0 )
        {
            printf("Failed to remove file [%s]", pcName);
            goto exit;
        }
    }

exit:
    return iErr;
}

static int
HFSTest_Rename( UVFSFileNode RootNode )
{
    int iErr = 0;

    printf("HFSTest_Rename\n");

    UVFSFileNode TestFolder = NULL;
    iErr = CreateNewFolder( RootNode, &TestFolder, "TestFolder");
    printf("CreateNewFolder err [%d]\n", iErr);
    if (iErr)
        return iErr;

    //Create new file with size 0
    UVFSFileNode TestFile1 = NULL;
    CreateNewFile(TestFolder, &TestFile1, "TestFile",0);
    printf("Create TestFile in TestFolder err [%d]\n", iErr);
    if (iErr)
        return iErr;

    //Create new file with size 512
    UVFSFileNode TestFile2 = NULL;
    CreateNewFile(TestFolder, &TestFile2, "TestFile2",512);
    printf("Create TestFile2 in TestFolder err [%d]\n", iErr);
    if (iErr)
        return iErr;


    uint32_t uEntrySize = sizeof(UVFSDirEntryAttr) + MAX_UTF8_NAME_LENGTH;
    UVFSDirEntryAttr *psReadDirTestsData = malloc(2*uEntrySize);
    if (psReadDirTestsData == NULL)
        return ENOMEM;

    UVFSDirEntryAttr *psCurrentReadDirTestsData = psReadDirTestsData;
    SetExpectedAttr("TestFile", UVFS_FA_TYPE_FILE, psCurrentReadDirTestsData);
    iErr = HFS_fsOps.fsops_getattr( TestFile1, &psCurrentReadDirTestsData->dea_attrs );
    psCurrentReadDirTestsData = (UVFSDirEntryAttr *) ((void*) psCurrentReadDirTestsData + uEntrySize);
    SetExpectedAttr("TestFile2", UVFS_FA_TYPE_FILE, psCurrentReadDirTestsData);
    iErr = HFS_fsOps.fsops_getattr( TestFile2, &psCurrentReadDirTestsData->dea_attrs );

    iErr = ReadDirAttr(TestFolder, psReadDirTestsData, 2);
    free(psReadDirTestsData);
    printf("ReadDirAttr err [%d]\n", iErr);
    if (iErr)
        goto exit;


    iErr = RenameFile(TestFolder, TestFile1, "TestFile", TestFolder, NULL, "TestFile3");
    printf("Rename TestFile to TestFile3 in same dir err [%d]\n", iErr);
    if (iErr)
        goto exit;

    iErr = RenameFile(TestFolder, TestFile2, "TestFile2", RootNode, NULL, "TestFile4");
    printf("Rename TestFile2 to TestFile4 in diff Dir err [%d]\n", iErr);
    if (iErr)
        goto exit;

    iErr = RenameFile(RootNode, TestFolder, "TestFolder", RootNode, NULL, "TestFolder5");
    printf("Rename Dir TestFolder to TestFolder5 err [%d]\n", iErr);
    if (iErr)
        goto exit;

    iErr = RenameFile(TestFolder, TestFile1, "TestFile3", RootNode, TestFile2, "TestFile4");
    printf("Rename TestFile3 to TestFile4 in diff Dir err [%d]\n", iErr);
    if (iErr)
        goto exit;


    // Remove File2
    iErr =  RemoveFile(RootNode,"TestFile4");
    printf("Remove File TestFile2 from TestFolder err [%d]\n", iErr);
    if (iErr)
        goto exit;

    // Remove TestFolder
    iErr =  RemoveFolder(RootNode,"TestFolder5");
    printf("Remove Folder TestFolder from Root err [%d]\n", iErr);
    if (iErr)
        goto exit;

exit:
    HFS_fsOps.fsops_reclaim(TestFolder);
    HFS_fsOps.fsops_reclaim(TestFile1);
    HFS_fsOps.fsops_reclaim(TestFile2);
    return iErr;
}

static int ScanDir(UVFSFileNode UVFSFolderNode, char** contain_str_array, char** end_with_str_array, struct timespec mTime)
{
    int err = 0;

    scandir_matching_request_t sMatchingCriteria = {0};
    UVFSFileAttributes smr_attribute_filter = {0};
    scandir_matching_reply_t sMatchingResult = {0};
    void* pvAttrBuf = malloc(sizeof(UVFSDirEntryAttr) + MAX_UTF8_NAME_LENGTH*sizeof(char));
    sMatchingResult.smr_entry = pvAttrBuf;

    sMatchingCriteria.smr_filename_contains = contain_str_array;
    sMatchingCriteria.smr_filename_ends_with = end_with_str_array;
    sMatchingCriteria.smr_attribute_filter = &smr_attribute_filter;

    if (mTime.tv_nsec != 0 || mTime.tv_sec != 0 )
    {
        sMatchingCriteria.smr_attribute_filter->fa_validmask |= UVFS_FA_VALID_MTIME;
        sMatchingCriteria.smr_attribute_filter->fa_mtime = mTime;
    }

    bool bConRead = true;

    do
    {
        err = HFS_fsOps.fsops_scandir (UVFSFolderNode, &sMatchingCriteria, &sMatchingResult);
        if ( err != 0 || ( sMatchingResult.smr_entry->dea_nextcookie == UVFS_DIRCOOKIE_EOF && sMatchingResult.smr_result_type == 0 ) )
        {
            bConRead = false;
        }
        else
        {
            if ( sMatchingResult.smr_entry->dea_nextcookie == UVFS_DIRCOOKIE_EOF )
            {
                bConRead = false;
            }
            printf("SearchDir Returned with status %d, FileName = [%s], M-Time sec:[%ld] nsec:[%ld].\n", sMatchingResult.smr_result_type, UVFS_DIRENTRYATTR_NAMEPTR(sMatchingResult.smr_entry),sMatchingResult.smr_entry->dea_attrs.fa_mtime.tv_sec,sMatchingResult.smr_entry->dea_attrs.fa_mtime.tv_nsec);

            sMatchingCriteria.smr_start_cookie = sMatchingResult.smr_entry->dea_nextcookie;
            sMatchingCriteria.smr_verifier = sMatchingResult.smr_verifier;
        }

    }while (bConRead);

    free(pvAttrBuf);

    return err;
}

static int HFSTest_Corrupted2ndDiskImage(__unused UVFSFileNode RootNode )
{
    int iErr = 0;
    printf("HFSTest_Corrupted2ndDiskImage:\n");
    
    UVFSFileNode TestFolder1 = NULL;
    iErr = CreateNewFolder( RootNode, &TestFolder1, "StamFolder");
    printf("CreateNewFolder err [%d]\n", iErr);
    if (iErr) goto exit;
    
    HFS_fsOps.fsops_reclaim(TestFolder1);

exit:
    return iErr;
}

static int HFSTest_ScanDir(UVFSFileNode RootNode )
{
    int iErr = 0;
    UVFSFileNode TestFolder1 = NULL;
    UVFSFileNode TestFolder2 = NULL;
    UVFSFileNode TestFile1 = NULL;

    iErr = CreateNewFolder( RootNode, &TestFolder1, "D2");
    printf("CreateNewFolder err [%d]\n", iErr);
    if (iErr) goto exit;

    iErr = CreateNewFolder( RootNode, &TestFolder2, "ÖÖ");
    printf("CreateNewFolder err [%d]\n", iErr);
    if (iErr) goto exit;

    //Create new file with size 0
    iErr = CreateNewFile(RootNode, &TestFile1, "F🤪2",0);
    printf("Create TestFile in TestFolder err [%d]\n", iErr);
    if (iErr) goto exit;

    UVFSFileAttributes sOutAttrs;
    iErr = HFS_fsOps.fsops_getattr(TestFile1, &sOutAttrs);
    printf("fsops_getattr F🤪2 err [%d]\n",iErr);
    if (iErr) goto exit;

    struct timespec mTime = {0};
    mTime.tv_nsec = sOutAttrs.fa_mtime.tv_nsec;
    mTime.tv_sec = sOutAttrs.fa_mtime.tv_sec;

    char* name_contains_array[5] = {0};
    char* name_end_with_array[5] = {0};
    char Smile[5] = "🤪";
    char ContainLetter[2] = "d";
    char EndsWithLetter[2] = "2";
    char SpecialChar[3] = "ö";

    name_contains_array[0] = (char*) Smile;
    name_contains_array[1] = (char*) ContainLetter;
    name_contains_array[2] = (char*) SpecialChar;
    name_contains_array[3] = NULL;

    name_end_with_array[0] = (char*) EndsWithLetter;
    name_end_with_array[1] = (char*) SpecialChar;
    name_end_with_array[2] = NULL;

    iErr =  ScanDir(RootNode, (char**) &name_contains_array, (char**) &name_end_with_array, mTime);
    printf("ScanDir err [%d]\n",iErr);

    
    HFS_fsOps.fsops_reclaim(TestFolder1);
    HFS_fsOps.fsops_reclaim(TestFolder2);
    HFS_fsOps.fsops_reclaim(TestFile1);

    // Remove File
    iErr =  RemoveFile(RootNode,"F🤪2");
    printf("Remove File F🤪2 from TestFolder err [%d]\n", iErr);
    if (iErr) goto exit;
    // Remove Folders
    iErr =  RemoveFolder(RootNode,"D2");
    printf("Remove Folder D1 from Root err [%d]\n", iErr);
    if (iErr) goto exit;

    iErr =  RemoveFolder(RootNode,"ÖÖ");
    printf("Remove Folder ÖÖ from Root err [%d]\n", iErr);
    if (iErr) goto exit;
exit:
    return iErr;
}

static int HFSTest_RootFillUp( UVFSFileNode RootNode ) {
    #define ROOT_FILL_UP_NUM_OF_FOLDERS 512
    #define ROOT_FILL_UP_NUM_OF_SYMLINKS 512
    UVFSFileNode pTestFolder[ROOT_FILL_UP_NUM_OF_FOLDERS] = {NULL};

    int      iErr = 0;
    unsigned u    = 0;
    bool    bFound;

   printf("HFSTest_RootFillUp\n");

   // Create folders
   for(u=0; u<ROOT_FILL_UP_NUM_OF_FOLDERS; u++) {

      char pcFolderName[256] = {0};
      sprintf(pcFolderName, "TestFolder_%d", u);

      printf("Creating folder %s.\n", pcFolderName);
      iErr = CreateNewFolder( RootNode, &pTestFolder[u], pcFolderName);
      //printf("iErr [%d]\n", iErr);
       if (iErr) {
          printf("Error: Creating folder %s. iErr [%d]\n", pcFolderName, iErr);
         return iErr;
       }
   }

   // Validate folders exist
   for(u=0; u<ROOT_FILL_UP_NUM_OF_FOLDERS; u++) {
      
      char pcFolderName[256] = {0};
      sprintf(pcFolderName, "TestFolder_%d", u);

      bFound = false;
      read_directory_and_search_for_name( RootNode, pcFolderName, &bFound, NULL, 0);
      if (!bFound) {
        printf("Error: %s wasn't found in Root.\n", pcFolderName);
        return -1;
      } else {
        printf("%s was found in Root.\n", pcFolderName);
      }
   }

   // Remove folders
   for(u=0; u<ROOT_FILL_UP_NUM_OF_FOLDERS; u++) {
      
      char pcFolderName[256] = {0};
      sprintf(pcFolderName, "TestFolder_%d", u);

      HFS_fsOps.fsops_reclaim(pTestFolder[u]);
      iErr =  RemoveFolder(RootNode, pcFolderName);
      printf("Remove Folder %s from Root err [%d]\n", pcFolderName, iErr);
      if (iErr) {
        return iErr;
      }
   }

   // Validate folders have been removed
   for(u=0; u<ROOT_FILL_UP_NUM_OF_FOLDERS; u++) {
      
      char pcFolderName[256] = {0};
      sprintf(pcFolderName, "TestFolder_%d", u);

      bFound = false;
      read_directory_and_search_for_name( RootNode, pcFolderName, &bFound, NULL, 0 );
      if (bFound) {
         printf("Found deleted dir! (%s)", pcFolderName);
         return -1;
      } else {
         printf(" Folder %s has been removed successfully.\n", pcFolderName);
      }
   }


    // Create Symlink
    UVFSFileNode pOutNode[ROOT_FILL_UP_NUM_OF_SYMLINKS] = {NULL};
    uint32_t uSymLinkMode = UVFS_FA_MODE_USR(UVFS_FA_MODE_RWX) | UVFS_FA_MODE_GRP(UVFS_FA_MODE_R) | UVFS_FA_MODE_OTH(UVFS_FA_MODE_R | UVFS_FA_MODE_X);
    for(u=0; u<ROOT_FILL_UP_NUM_OF_SYMLINKS; u++) {
        
        char pcSymLinkName[256] = {0};
        sprintf(pcSymLinkName,   "TestSymLink_%d", u);
        char pcSymLinkContent[256] = {0};
        sprintf(pcSymLinkContent, "/just/for/check/that/symlink/work/properly_%d", u);
        UVFSFileAttributes sAttr = {0};
        sAttr.fa_validmask       = UVFS_FA_VALID_MODE;
        sAttr.fa_type            = UVFS_FA_TYPE_SYMLINK;
        sAttr.fa_mode            = uSymLinkMode;

        iErr = HFS_fsOps.fsops_symlink( RootNode, pcSymLinkName, pcSymLinkContent, &sAttr, &pOutNode[u] );
        if ( iErr != 0 ) {
            printf( "fsops_symlink failed with eror code : %d\n", iErr );
            return(iErr);
        }

        //HFS_fsOps.fsops_reclaim( pOutNode[u] );
    }


    // Verify Symlink content
    for(u=0; u<ROOT_FILL_UP_NUM_OF_SYMLINKS; u++) {
        
        char pcSymLinkName[256] = {0};
        sprintf(pcSymLinkName,   "TestSymLink_%d", u);
        char pcSymLinkReadContent[256] = {0};
        size_t iActuallyRead;

        //UVFSFileNode SymLinkNode = NULL;
        //iErr = HFS_fsOps.fsops_lookup( RootNode, pcSymLinkName, &SymLinkNode );
        //printf("Symlink (%s) Lookup err [%d]\n", pcSymLinkName, iErr);
        //if ( iErr )
        //    return iErr;

        UVFSFileAttributes sOutAttr = {0};
        
        iErr = HFS_fsOps.fsops_readlink( pOutNode[u], pcSymLinkReadContent, sizeof(pcSymLinkReadContent), &iActuallyRead, &sOutAttr );
        //iErr = HFS_fsOps.fsops_readlink( SymLinkNode, pcSymLinkReadContent, sizeof(pcSymLinkReadContent), &iActuallyRead, &sOutAttr );
        if ( iErr != 0 ) {
            printf( "fsops_readlink failed with eror code : %d\n", iErr );
            return(iErr);
        }

        char pcSymLinkContent[256] = {0};
        sprintf(pcSymLinkContent, "/just/for/check/that/symlink/work/properly_%d", u);

        if ( strcmp( pcSymLinkContent, pcSymLinkReadContent) != 0 ) {
            printf( "Read bad symlink content\n" );
            iErr = -1;
            return(iErr);
        }
        
        if ( sOutAttr.fa_mode != uSymLinkMode) {
            printf( "Mode mismatch [%d != %d]\n", sOutAttr.fa_mode, uSymLinkMode);
            iErr = -1;
            return(iErr);
        }
        
        if ( sOutAttr.fa_type != UVFS_FA_TYPE_SYMLINK ) {
            printf( "Type mismatch\n" );
            iErr = -1;
            return(iErr);
        }
        
        HFS_fsOps.fsops_reclaim( pOutNode[u] );
        //HFS_fsOps.fsops_reclaim( SymLinkNode );
    }
        
    // Remove SymLinks.
    for(u=0; u<ROOT_FILL_UP_NUM_OF_SYMLINKS; u++) {
        
        char pcSymLinkName[256] = {0};
        sprintf(pcSymLinkName,   "TestSymLink_%d", u);
        
        iErr = HFS_fsOps.fsops_remove( RootNode, pcSymLinkName, NULL);
        if ( iErr != 0 ) {
            printf( "Failed to remove symlink %d\n", iErr );
            return(iErr);
        }
        
        bFound = false;
        read_directory_and_search_for_name( RootNode, pcSymLinkName, &bFound, NULL, 0 );
        if ( bFound ) {
            printf( "Failed to remove symlink\n");
            iErr = -1;
            return(iErr);
        }
    }
        
    return iErr;
}

static int ReadUnMountBit(UVFSFileNode psRootNode, uint32_t *puUnMountBit) {
    
    int iErr = 0;
    char pcVolumeHeader[512];
    uint64_t uActuallyRead = 0;
    iErr = raw_readwrite_read_mount(psRootNode, 2, 512, pcVolumeHeader, 512, &uActuallyRead, NULL);
    if (iErr) {
        printf("Failed to read volume header\n");
        goto exit;
    }

    uint32_t uVolHdrAttributesBigEndian = *(uint32_t*)(pcVolumeHeader+4);
    uint32_t uVolHdrAttributes = OSSwapBigToHostInt32(uVolHdrAttributesBigEndian);
    printf("uVolHdrAttributes 0x%x\n", uVolHdrAttributes);

    uint32_t uUnMountBit = (uVolHdrAttributes & 0x00000100)?1:0;
    printf("uUnMountBit %u\n", uUnMountBit);

    *puUnMountBit = uUnMountBit;

exit:
    return(iErr);
}

static int HFSTest_ValidateUnmount_wJournal( UVFSFileNode psRootNode ) {
    int iErr = 0;
    
    printf("HFSTest_ValidateUnmount_wJournal\n");
    
    UVFSFileNode psTestFolder = NULL;
    iErr = CreateNewFolder( psRootNode, &psTestFolder, "TestFolder");
    printf("CreateNewFolder err [%d]\n", iErr);
    if (iErr)
        return iErr;
    
    HFS_fsOps.fsops_reclaim( psTestFolder );
    
    // Read volume header & validate unmount is set (journaled data is still in cache)
    uint32_t uUnMountBit;
    if (ReadUnMountBit(psRootNode, &uUnMountBit)) {
        goto exit;
    }
    
    if (!uUnMountBit) {
        printf("uUnMountBit is cleared though it should be set\n");
        iErr = -1;
        goto exit;
    }
    
    // Call Sync
    HFS_fsOps.fsops_sync(psRootNode);
    
    // Read volume header & validate unmount is set
    if (ReadUnMountBit(psRootNode, &uUnMountBit)) {
        goto exit;
    }
    if (!uUnMountBit) {
        printf("uUnMountBit is cleared though it should be set\n");
        iErr = -1;
        goto exit;
    }
    
exit:
    return iErr;
}

static int HFSTest_ValidateUnmount( UVFSFileNode psRootNode ) {
    int iErr = 0;
    uint32_t uUnMountBit;
    
    printf("HFSTest_ValidateUnmount\n");
    
    UVFSFileNode psTestFolder = NULL;
    iErr = CreateNewFolder( psRootNode, &psTestFolder, "TestFolder_1");
    printf("CreateNewFolder err [%d]\n", iErr);
    if (iErr)
        return iErr;
    
    HFS_fsOps.fsops_reclaim( psTestFolder );
    
    // Read volume header & validate unmount is cleared
    if (ReadUnMountBit(psRootNode, &uUnMountBit)) {
        goto exit;
    }
    
    if (uUnMountBit) {
        printf("uUnMountBit is set though it should be cleared\n");
        iErr = -1;
        goto exit;
    }
    
    // Call Sync
    HFS_fsOps.fsops_sync(psRootNode);

    // Read volume header & validate unmount is set
    if (ReadUnMountBit(psRootNode, &uUnMountBit)) {
        goto exit;
    }
    if (!uUnMountBit) {
        printf("uUnMountBit is cleared though it should be set\n");
        iErr = -1;
        goto exit;
    }
    
    iErr = CreateNewFolder( psRootNode, &psTestFolder, "TestFolder_2");
    printf("CreateNewFolder err [%d]\n", iErr);
    if (iErr)
        return iErr;
    
    HFS_fsOps.fsops_reclaim( psTestFolder );
    
    // Read volume header & validate unmount is cleared
    if (ReadUnMountBit(psRootNode, &uUnMountBit)) {
        goto exit;
    }
    
    if (uUnMountBit) {
        printf("uUnMountBit is set though it should be cleared\n");
        iErr = -1;
        goto exit;
    }
    
    // Call Sync
    HFS_fsOps.fsops_sync(psRootNode);
    
    // Read volume header & validate unmount is set
    if (ReadUnMountBit(psRootNode, &uUnMountBit)) {
        goto exit;
    }
    if (!uUnMountBit) {
        printf("uUnMountBit is cleared though it should be set\n");
        iErr = -1;
        goto exit;
    }
    
exit:
    return iErr;
}

static int HFSTest_OneSync( UVFSFileNode RootNode ) {
    int iErr = 0;
    char pcFolderName[256];
    
    printf("HFSTest_OneSync\n");
    
    for(unsigned u=0; u<5; u++) {
        UVFSFileNode TestFolder = NULL;
        sprintf(pcFolderName, "TestFolder_%u", u);

        iErr = CreateNewFolder( RootNode, &TestFolder, pcFolderName);
        printf("CreateNewFolder err [%d]\n", iErr);
        if (iErr)
            return iErr;
        HFS_fsOps.fsops_reclaim( TestFolder );
    }
    
    for(unsigned u=0; u<5; u++) {
        bool bFound = false;
        sprintf(pcFolderName, "TestFolder_%u", u);
        read_directory_and_search_for_name( RootNode, pcFolderName, &bFound, NULL, 0);
        if (!bFound)
        {
            printf("Error: %s wasn't found in Root.\n", pcFolderName);
            return -1;
        }
        else
        {
            printf("%s found in Root!\n", pcFolderName);
        }
    }

    printf("Calling Sync\n");
    HFS_fsOps.fsops_sync(RootNode);
    
    return iErr;
}

static int
HFSTest_MakeDir( UVFSFileNode RootNode )
{
    int iErr = 0;

    printf("HFSTest_MakeDir\n");

    UVFSFileNode TestFolder = NULL;
    iErr = CreateNewFolder( RootNode, &TestFolder, "TestFolder");
    printf("CreateNewFolder err [%d]\n", iErr);
    if (iErr)
        return iErr;

    bool bFound = false;
    read_directory_and_search_for_name( RootNode, "TestFolder", &bFound, NULL, 0);
    if (!bFound)
    {
        printf("Error: TestFolder wasn't found in Root.\n");
        return -1;
    }
    else
    {
        printf("TestFolder found in Root!\n");
    }

    // Remove D1
    iErr =  RemoveFolder(RootNode,"TestFolder");
    printf("Remove Folder D1 from Root err [%d]\n", iErr);
    if (iErr)
        return iErr;

    bFound = false;
    read_directory_and_search_for_name( RootNode, "TestFolder", &bFound, NULL, 0 );
    if (bFound)
    {
        printf("Found deleted dir!");
        return -1;
    }

    HFS_fsOps.fsops_reclaim( TestFolder );

    return iErr;
}

__unused static int
HFSTest_MakeDirAndKeep( UVFSFileNode psRootNode )
{
    int iErr = 0;
    char pcFolderName[256];
    
    printf("HFSTest_MakeDirAndKeep\n");
    
    for(unsigned u=0; u<100; u++) {
        UVFSFileNode TestFolder = NULL;
        sprintf(pcFolderName, "TestFolder_%u", u);
        iErr = CreateNewFolder( psRootNode, &TestFolder, pcFolderName);
        printf("CreateNewFolder err [%d]\n", iErr);
        if (iErr)
            return iErr;
        usleep(guSyncerPeriod * 10); // Allow the syncer to run (at 1/100th rate)
        
        HFS_fsOps.fsops_reclaim( TestFolder );
    }
    
    for(unsigned u=0; u<100; u++) {
        bool bFound = false;
        sprintf(pcFolderName, "TestFolder_%u", u);
        read_directory_and_search_for_name( psRootNode, pcFolderName, &bFound, NULL, 0);
        if (!bFound)
        {
            printf("Error: %s wasn't found in Root.\n", pcFolderName);
            return -1;
        }
        else
        {
            printf("%s found in Root!\n", pcFolderName);
        }
    }
    
    return iErr;
}

static int
HFSTest_ReadDefragmentFile( UVFSFileNode RootNode )
{

    int iErr = 0;

    printf("HFSTest_ReadDefragmentFile\n");


    UVFSFileNode DeFragmentFile = NULL;
    iErr = HFS_fsOps.fsops_lookup( RootNode, "defragment1.bin", &DeFragmentFile );
    printf("Lookup err [%d]\n", iErr);
    if ( iErr )
        return iErr;

    UVFSFileAttributes sOutAttr;
    iErr = HFS_fsOps.fsops_getattr( DeFragmentFile, &sOutAttr );
    printf("GetAttr err [%d]\n", iErr);
    if ( iErr )
        return iErr;

#define FILE_SIZE   (1638400)
    if ( sOutAttr.fa_allocsize != FILE_SIZE || sOutAttr.fa_size != FILE_SIZE)
    {
        printf("Wrong size [%llu\n] [%llu\n]\n", sOutAttr.fa_size, sOutAttr.fa_allocsize);
        return -1;
    }

    void* pvReadBuf = malloc(FILE_SIZE);
    memset(pvReadBuf, 0, FILE_SIZE);

    size_t iActuallyRead;
    iErr = HFS_fsOps.fsops_read( DeFragmentFile, 0, FILE_SIZE, pvReadBuf, &iActuallyRead );

    HFS_fsOps.fsops_reclaim( DeFragmentFile );
    
    if ( iErr != 0 )
    {
        printf("HFS_fsOps.fsops_read return status %d\n", iErr);
        free(pvReadBuf);
        return iErr;
    }

    if ( iActuallyRead != FILE_SIZE )
    {
        printf("iActuallyRead != FILE_SIZE %lu\n", iActuallyRead);
        free(pvReadBuf);
        return -1;
    }

    uint32_t uDetectedNum   = 0;
    char pcDetectedNum[17]  = {0};
    for ( uint32_t uIdx=0; uIdx<FILE_SIZE/16; uIdx++ )
    {
        memcpy(pcDetectedNum, pvReadBuf + uIdx*16, 16);
        sscanf(pcDetectedNum, "%u", &uDetectedNum);

        if ( uIdx+1 != uDetectedNum )
        {
            printf("Read failed. Expected [%u], detected [%u]\n", uIdx, uDetectedNum);
            free(pvReadBuf);
            return -1;
        }
    }

    free(pvReadBuf);
    return iErr;
}

static int
HFSTest_RemoveDir( UVFSFileNode RootNode )
{
    int iErr = 0;

    UVFSFileNode MainDir = NULL;
    iErr = HFS_fsOps.fsops_lookup( RootNode, "MainDir", &MainDir );
    if ( iErr != 0 )
    {
        printf("Failed to lookup for main dir\n");
        return iErr;
    }

    // Try to delete non empty directoy.
    iErr = HFS_fsOps.fsops_rmdir( RootNode, "MainDir" );
    if ( iErr != ENOTEMPTY )
    {
        printf( "Return status is [%d], expected [%d]\n", iErr, ENOTEMPTY );
        return -1;
    }

    // Delete empty dirs.. Dir[1..10];
    char pcDirName[10];
    for ( int iDirIdx=1; iDirIdx<11; iDirIdx++ )
    {
        memset( pcDirName, 0, sizeof(pcDirName) );
        sprintf( pcDirName, "Dir%d", iDirIdx);

        // Try to delete empty directoy.
        iErr = HFS_fsOps.fsops_rmdir( MainDir, pcDirName );
        printf( "remove dir ended with err [%d]\n", iErr );
        if ( iErr != 0 )
        {
            return iErr;
        }
    }

    // Reclaim main dir.
    HFS_fsOps.fsops_reclaim( MainDir );

    // Now, try to delete empty main directoy.
    iErr = HFS_fsOps.fsops_rmdir( RootNode, "MainDir" );
    if ( iErr != 0 )
    {
        printf( "Failed to remove main dir [%d]\n", iErr );
        return iErr;
    }

    // Make sure main directory deleted.
    iErr = HFS_fsOps.fsops_lookup( RootNode, "MainDir", &MainDir );
    if ( iErr == 0 )
    {
        printf("Main dir still exist.\n");
        return -1;
    }

    // Try to remove unexisting directory
    iErr = HFS_fsOps.fsops_rmdir( RootNode, "MainDir" );
    if ( iErr != ENOENT )
    {
        printf( "Expected [%d], detected [%d]\n", ENOENT, iErr );
        return -1;
    }

    return 0;
}

static int
HFSTest_Remove( UVFSFileNode RootNode )
{
    int iErr = 0;

#define NUM_OF_FILES    (2)

    char* ppcFilesNames[NUM_OF_FILES] = { "SpecialFileName+-)(*&^%$#@\\!\\}\\{~~<>||??\\.txt", "file1.txt" };

    for ( uint8_t uIdx=0; uIdx<NUM_OF_FILES; uIdx++ )
    {
        bool bFound = false;
        read_directory_and_search_for_name( RootNode, ppcFilesNames[uIdx], &bFound, NULL, 0);
        if ( !bFound )
        {
            printf( "Failed to found [%s] status [%d]\n", ppcFilesNames[uIdx], iErr );
            return -1;
        }

        iErr = HFS_fsOps.fsops_remove( RootNode, ppcFilesNames[uIdx], NULL);
        if ( iErr != 0 )
        {
            printf( "Failed to remove [%s] status [%d]\n", ppcFilesNames[uIdx], iErr );
            return iErr;
        }

        bFound = false;
        read_directory_and_search_for_name( RootNode, ppcFilesNames[uIdx], &bFound, NULL, 0);
        if ( bFound )
        {
            printf( "Found [%s] status [%d]\n", ppcFilesNames[uIdx], iErr );
            return -1;
        }

        iErr = HFS_fsOps.fsops_remove( RootNode, ppcFilesNames[uIdx], NULL);
        if ( iErr != ENOENT )
        {
            printf( "Removed deleted file [%s] status [%d]\n", ppcFilesNames[uIdx], iErr );
            return -1;
        }
    }

    return 0;
}

static void SetExpectedAttr(char* pcName, uint32_t uType, UVFSDirEntryAttr* psAttr)
{
    psAttr->dea_nameoff = UVFS_DIRENTRYATTR_NAMEOFF;
    memcpy (UVFS_DIRENTRYATTR_NAMEPTR(psAttr),pcName, strlen(pcName) + 1);
    psAttr->dea_attrs.fa_type = uType;
}

static int
HFSTest_ReadDir( UVFSFileNode RootNode )
{
    int iErr = 0;

    uint32_t uEntrySize = sizeof(UVFSDirEntryAttr) + MAX_UTF8_NAME_LENGTH;
    UVFSDirEntryAttr *psReadDirTestsData = malloc(6*uEntrySize);
    if (psReadDirTestsData == NULL)
        return ENOMEM;

    UVFSDirEntryAttr *psCurrentReadDirTestsData = psReadDirTestsData;
    SetExpectedAttr(".", UVFS_FA_TYPE_DIR, psCurrentReadDirTestsData);
    psCurrentReadDirTestsData = (UVFSDirEntryAttr *) ((void*) psCurrentReadDirTestsData + uEntrySize);
    SetExpectedAttr("..", UVFS_FA_TYPE_DIR, psCurrentReadDirTestsData);
    psCurrentReadDirTestsData = (UVFSDirEntryAttr *) ((void*) psCurrentReadDirTestsData + uEntrySize);
    SetExpectedAttr(".DS_Store", UVFS_FA_TYPE_FILE, psCurrentReadDirTestsData);
    psCurrentReadDirTestsData = (UVFSDirEntryAttr *) ((void*) psCurrentReadDirTestsData + uEntrySize);
    SetExpectedAttr("D1", UVFS_FA_TYPE_DIR, psCurrentReadDirTestsData);
    psCurrentReadDirTestsData = (UVFSDirEntryAttr *) ((void*) psCurrentReadDirTestsData + uEntrySize);
    SetExpectedAttr("F1", UVFS_FA_TYPE_FILE, psCurrentReadDirTestsData);
    psCurrentReadDirTestsData = (UVFSDirEntryAttr *) ((void*) psCurrentReadDirTestsData + uEntrySize);
    SetExpectedAttr("L1", UVFS_FA_TYPE_SYMLINK, psCurrentReadDirTestsData);

//
//        {.pcTestName = ".",         .uTyppe = UVFS_FA_TYPE_DIR},
//        {.pcTestName = "..",        .uTyppe = UVFS_FA_TYPE_DIR},
//        {.pcTestName = ".DS_Store", .uTyppe = UVFS_FA_TYPE_FILE},
//        {.pcTestName = "D1",        .uTyppe = UVFS_FA_TYPE_DIR},
//        {.pcTestName = "F1",        .uTyppe = UVFS_FA_TYPE_FILE},
//        {.pcTestName = "L1",        .uTyppe = UVFS_FA_TYPE_SYMLINK},
//    };

    bool bFound;
    UVFSFileNode MainDir = NULL;
    iErr = HFS_fsOps.fsops_lookup( RootNode, "D1", &MainDir );
    printf("Lookup err [%d]\n", iErr);
    if ( iErr )
        return iErr;

    iErr = read_directory_and_search_for_name( MainDir, "D1", &bFound, psReadDirTestsData, 6);
    free(psReadDirTestsData);
    // Reclaim main dir.
    HFS_fsOps.fsops_reclaim(MainDir);

    return iErr;
}

static int __used
HFSTest_ReadDirAttr( UVFSFileNode RootNode )
{
    int iErr = 0;

//    struct ReadDirTestData_s  psReadDirTestsData[] = {
//        {.pcTestName = ".DS_Store",  .uTyppe = UVFS_FA_TYPE_FILE,    .uSize = 6148, .uNlink = 1, .uAllocatedSize = 8192},
//        {.pcTestName = "D1",         .uTyppe = UVFS_FA_TYPE_DIR,     .uSize = 0,    .uNlink = 2, .uAllocatedSize = 0},
//        {.pcTestName = "F1",         .uTyppe = UVFS_FA_TYPE_FILE,    .uSize = 4,    .uNlink = 1, .uAllocatedSize = 4096},
//        {.pcTestName = "L1",         .uTyppe = UVFS_FA_TYPE_SYMLINK, .uSize = 23,   .uNlink = 1, .uAllocatedSize = 4096},
//    };

    UVFSFileNode MainDir = NULL;
    iErr = HFS_fsOps.fsops_lookup( RootNode, "D1", &MainDir );
    printf("Lookup err [%d]\n", iErr);
    if ( iErr )
        return iErr;

    uint32_t uEntrySize = sizeof(UVFSDirEntryAttr) + MAX_UTF8_NAME_LENGTH;
    UVFSDirEntryAttr *psReadDirTestsData = malloc(4*uEntrySize);
    if (psReadDirTestsData == NULL)
        return ENOMEM;

    UVFSFileNode psVnode = NULL;
    UVFSFileNode psVnode1 = NULL;
    UVFSFileNode psVnode2 = NULL;
    UVFSFileNode psVnode3 = NULL;
    UVFSDirEntryAttr *psCurrentReadDirTestsData = psReadDirTestsData;
    SetExpectedAttr(".DS_Store", UVFS_FA_TYPE_FILE, psCurrentReadDirTestsData);
    iErr = HFS_fsOps.fsops_lookup( MainDir, ".DS_Store", &psVnode );
    iErr = HFS_fsOps.fsops_getattr( psVnode, &psCurrentReadDirTestsData->dea_attrs );
    psCurrentReadDirTestsData = (UVFSDirEntryAttr *) ((void*) psCurrentReadDirTestsData + uEntrySize);
    SetExpectedAttr("D1", UVFS_FA_TYPE_DIR, psCurrentReadDirTestsData);
    iErr = HFS_fsOps.fsops_lookup( MainDir, "D1", &psVnode1 );
    iErr = HFS_fsOps.fsops_getattr( psVnode1, &psCurrentReadDirTestsData->dea_attrs );
    psCurrentReadDirTestsData = (UVFSDirEntryAttr *) ((void*) psCurrentReadDirTestsData + uEntrySize);
    SetExpectedAttr("F1", UVFS_FA_TYPE_FILE, psCurrentReadDirTestsData);
    iErr = HFS_fsOps.fsops_lookup( MainDir, "F1", &psVnode2 );
    iErr = HFS_fsOps.fsops_getattr( psVnode2, &psCurrentReadDirTestsData->dea_attrs );
    psCurrentReadDirTestsData = (UVFSDirEntryAttr *) ((void*) psCurrentReadDirTestsData + uEntrySize);
    SetExpectedAttr("L1", UVFS_FA_TYPE_SYMLINK, psCurrentReadDirTestsData);
    iErr = HFS_fsOps.fsops_lookup( MainDir, "L1", &psVnode3 );
    iErr = HFS_fsOps.fsops_getattr( psVnode3, &psCurrentReadDirTestsData->dea_attrs );

    iErr = ReadDirAttr(MainDir, psReadDirTestsData, 4);
    free(psReadDirTestsData);
    // Reclaim main dir.
    HFS_fsOps.fsops_reclaim(MainDir);
    HFS_fsOps.fsops_reclaim(psVnode);
    HFS_fsOps.fsops_reclaim(psVnode1);
    HFS_fsOps.fsops_reclaim(psVnode2);
    HFS_fsOps.fsops_reclaim(psVnode3);

    return iErr;
}

static int
HFSTest_ReadSymlink( UVFSFileNode RootNode )
{
    void* pvBuf                 = malloc(200);
    assert( pvBuf != NULL );
    memset( pvBuf, 0, 200 );
    char* pcSymLinkContent      = "/just/for/check/that/symlink/work/properly";
    char* pcSymlinkFileName     = "symlinkfile";
    UVFSFileNode outNode        = NULL;

    int iErr = HFS_fsOps.fsops_lookup(RootNode, pcSymlinkFileName, &outNode);
    if (iErr)
        printf("Dir read failed, D2 wasn't found in Root");

    // Verify Symlink content
    size_t iActuallyRead;
    UVFSFileAttributes sOutAttr = {0};
    iErr = HFS_fsOps.fsops_readlink( outNode, pvBuf, 200, &iActuallyRead, &sOutAttr );
    if ( iErr != 0 )
    {
        printf( "fsops_readlink failed with eror code : %d\n", iErr );
        goto exit;
    }

    if ( strcmp( pvBuf, pcSymLinkContent) != 0 )
    {
        printf( "Read bad symlink content\n" );
        iErr = -1;
        goto exit;
    }

    HFS_fsOps.fsops_reclaim( outNode );

exit:
    if (pvBuf)
        free(pvBuf);

    return iErr;
}

static int
HFSTest_Symlink( UVFSFileNode RootNode )
{

#define SYMLINK_MODE                                      \
( UVFS_FA_MODE_USR(UVFS_FA_MODE_RWX)  |                   \
  UVFS_FA_MODE_GRP(UVFS_FA_MODE_R)    |                   \
  UVFS_FA_MODE_OTH(UVFS_FA_MODE_R     | UVFS_FA_MODE_X) )

    void* pvBuf                 = malloc(200);
    assert( pvBuf != NULL );
    memset( pvBuf, 0xff, 200 );
    char* pcSymLinkContent      = "/just/for/check/that/symlink/work/properly";
    char* pcSymlinkFileName     = "symlinkfile";
    UVFSFileAttributes sAttr    = {0};
    sAttr.fa_validmask          = UVFS_FA_VALID_MODE;
    sAttr.fa_type               = UVFS_FA_TYPE_SYMLINK;
    sAttr.fa_mode               = SYMLINK_MODE;
    UVFSFileNode outNode        = NULL;

    // Create Symlink.
    int iErr = HFS_fsOps.fsops_symlink( RootNode, pcSymlinkFileName, pcSymLinkContent, &sAttr, &outNode );
    if ( iErr != 0 )
    {
        printf( "fsops_symlink failed with eror code : %d\n", iErr );
        goto exit;
    }

    // Enable once vnode functionality will be merged.

     // Verify Symlink content
     size_t iActuallyRead;
     UVFSFileAttributes sOutAttr = {0};
     iErr = HFS_fsOps.fsops_readlink( outNode, pvBuf, 200, &iActuallyRead, &sOutAttr );
     if ( iErr != 0 )
     {
         printf( "fsops_readlink failed with eror code : %d\n", iErr );
         goto exit;
     }

     if ( strcmp( pvBuf, pcSymLinkContent) != 0 )
     {
         printf( "Read bad symlink content\n" );
         iErr = -1;
         goto exit;
     }

     if ( sOutAttr.fa_mode != SYMLINK_MODE)
     {
         printf( "Mode mismatch [%d != %d]\n", sOutAttr.fa_mode, SYMLINK_MODE);
         iErr = -1;
         goto exit;
     }

     if ( sOutAttr.fa_type != UVFS_FA_TYPE_SYMLINK )
     {
         printf( "Type mismatch\n" );
         iErr = -1;
         goto exit;
     }


    HFS_fsOps.fsops_reclaim( outNode );

    // Remove link.
    iErr = HFS_fsOps.fsops_remove( RootNode, pcSymlinkFileName, NULL);
    if ( iErr != 0 )
    {
        printf( "Failed to remove symlink %d\n", iErr );
        goto exit;
    }

    bool bFound = false;
    read_directory_and_search_for_name( RootNode, pcSymlinkFileName, &bFound, NULL, 0 );
    if ( bFound )
    {
        printf( "Failed to remove symlink\n");
        iErr = -1;
        goto exit;
    }

exit:
    if (pvBuf)
        free(pvBuf);

    return iErr;
}

static int HFSTest_SymlinkOnFile( UVFSFileNode pRootNode ) {
    // This test creates a file and a folder on root.
    // It then tries to create a SymLink inside the folder and expects pass,
    // and creates a SymLink inside a file and expects a failure.
    printf("HFSTest_SymlinkOnFile\n");

    char    *pcFolderName      = "NewFolder";
    char    *pcFileName        = "NewFile.txt";
    uint32_t uFileLen          = 985;
    char    *pcSymlinkFilename = "SymLinkFile";
    char    *pcSymLinkContent  = "/SymlinkContent";
    int      iErr              = 0;
    UVFSFileNode pFolderNode          = NULL;
    UVFSFileNode pFileNode            = NULL;
    UVFSFileNode pSymLinkOnRootNode   = NULL;
    UVFSFileNode pSymLinkOnFolderNode = NULL;
    UVFSFileNode pSymLinkOnFileNode   = NULL;

    iErr = CreateNewFolder( pRootNode, &pFolderNode, pcFolderName);
    printf("CreateNewFolder err [%d]\n", iErr);
    if (iErr) {
        printf("Error: CreateNewFolder failed.\n");
        return iErr;
    }
    
    //Create new file with size 512
    CreateNewFile(pRootNode, &pFileNode, pcFileName, uFileLen);
    printf("Create %s Len %u err [%d]\n", pcFileName, uFileLen, iErr);
    if (iErr) {
        printf("Error: CreateNewFile failed.\n");
        return iErr;
    }

    bool bFound = false;
    read_directory_and_search_for_name( pRootNode, pcFolderName, &bFound, NULL, 0);
    if (!bFound) {
        printf("Error: %s wasn't found in Root.\n", pcFolderName);
        return -1;
    } else {
        printf("%s found in Root!\n", pcFolderName);
    }
    
    read_directory_and_search_for_name( pRootNode, pcFileName, &bFound, NULL, 0);
    if (!bFound) {
        printf("Error: %s wasn't found in Root.\n", pcFileName);
        return -1;
    } else {
        printf("%s found in Root!\n", pcFileName);
    }

    UVFSFileAttributes sAttr        = {0};
    sAttr.fa_validmask              = UVFS_FA_VALID_MODE;
    sAttr.fa_type                   = UVFS_FA_TYPE_SYMLINK;
    sAttr.fa_mode                   = SYMLINK_MODE;

    // Create Symlink on root
    iErr = HFS_fsOps.fsops_symlink( pRootNode, pcSymlinkFilename, pcSymLinkContent, &sAttr, &pSymLinkOnRootNode );
    if ( iErr != 0 ) {
        printf( "fsops_symlink failed to create %s with eror code : %d\n", pcSymlinkFilename, iErr );
        return(iErr);
    }
    
    // Create Symlink on folder
    iErr = HFS_fsOps.fsops_symlink( pFolderNode, pcSymlinkFilename, pcSymLinkContent, &sAttr, &pSymLinkOnFolderNode );
    if ( iErr != 0 ) {
        printf( "fsops_symlink failed to create %s inside %s with eror code : %d\n", pcSymlinkFilename, pcFolderName, iErr );
        return(iErr);
    }

    // Create Symlink on file
    iErr = HFS_fsOps.fsops_symlink( pFileNode, pcSymlinkFilename, pcSymLinkContent, &sAttr, &pSymLinkOnFileNode );
    if ( iErr == 0 ) {
        printf( "fsops_symlink error: did not fail to create %s inside %s with eror code : %d\n", pcSymlinkFilename, pcFileName, iErr );
        return(-1);
    }

    // cleanup
    assert(pSymLinkOnFileNode == NULL);
    HFS_fsOps.fsops_reclaim( pFileNode );
    HFS_fsOps.fsops_reclaim( pFolderNode );
    HFS_fsOps.fsops_reclaim( pSymLinkOnFolderNode );
    HFS_fsOps.fsops_reclaim( pSymLinkOnRootNode );

    return 0;
}

static int
HFSTest_SetAttr( UVFSFileNode RootNode )
{
    int iErr = 0;

    UVFSFileNode Dir1 = NULL;
    iErr = HFS_fsOps.fsops_lookup(RootNode, "D2", &Dir1);
    if (iErr)
        printf("Dir read failed, D2 wasn't found in Root");
    UVFSFileNode File1 = NULL;
    iErr = HFS_fsOps.fsops_lookup(Dir1, "a.txt", &File1);

    if (iErr)
    {
        printf("File not found!\n");
        return -1;
    }

    // Change file size
    // Set Attr, make F1 larger
    iErr = SetAttrChangeSize(File1,12*1024);
    printf("SetAttrChangeSize to 12K err [%d]\n",iErr);
    if (iErr)
    {
        return iErr;
    }

    iErr = SetAttrChangeSize(File1,4*1024);
    printf("SetAttrChangeSize to 4 err [%d]\n",iErr);
    if (iErr)
    {
        return iErr;
    }

    iErr = SetAttrChangeSize(File1,0*1024);
    printf("SetAttrChangeSize to 0 err [%d]\n",iErr);
    if (iErr)
    {
        return iErr;
    }

    iErr = SetAttrChangeSize(File1,8*1024*1024);
    printf("SetAttrChangeSize to 120MB err [%d]\n",iErr);
    if (iErr)
    {
        return iErr;
    }

    iErr = SetAttrChangeMode(File1, UVFS_FA_MODE_GRP(UVFS_FA_MODE_RWX) | UVFS_FA_MODE_USR(UVFS_FA_MODE_RWX));
    printf("Changed file mode to RO err[ %d]\n",iErr);
    if (iErr)
    {
        return iErr;
    }

    iErr = SetAttrChangeUidGid(File1, 222, 555);

    printf("Changed Uid and Gid err [%d]\n", iErr);
    if (iErr)
    {
        return iErr;
    }

    iErr = SetAttrChangeAtimeMtime(File1);

    printf("Changed Atime and Mtime err [%d]\n", iErr);
    if (iErr)
    {
        return iErr;
    }

    HFS_fsOps.fsops_reclaim(File1);

    HFS_fsOps.fsops_reclaim(Dir1);

    iErr = HFS_fsOps.fsops_lookup(RootNode, "D2", &Dir1);
    if (iErr)
        printf("Dir read failed, D2 wasn't found in Root");
    iErr = HFS_fsOps.fsops_lookup(Dir1, "a.txt", &File1);
    if (iErr)
    {
        printf("File not found! (2)\n");
        return -1;
    }

    iErr = SetAttrChangeAtimeMtime(File1);

    printf("Changed Atime and Mtime (2) err [%d]\n", iErr);
    if (iErr)
    {
        return iErr;
    }

    HFS_fsOps.fsops_reclaim(File1);

    HFS_fsOps.fsops_reclaim(Dir1);

    return iErr;
}

static char* gpcFSAttrs [] = {
    UVFS_FSATTR_PC_LINK_MAX,
    UVFS_FSATTR_PC_NAME_MAX,
    UVFS_FSATTR_PC_NO_TRUNC,
    UVFS_FSATTR_PC_FILESIZEBITS,
    UVFS_FSATTR_PC_XATTR_SIZE_BITS,
    UVFS_FSATTR_BLOCKSIZE,
    UVFS_FSATTR_IOSIZE,
    UVFS_FSATTR_TOTALBLOCKS,
    UVFS_FSATTR_BLOCKSFREE,
    UVFS_FSATTR_BLOCKSAVAIL,
    UVFS_FSATTR_BLOCKSUSED,
    UVFS_FSATTR_CNAME,
    UVFS_FSATTR_FSTYPENAME,
    UVFS_FSATTR_FSSUBTYPE,
    UVFS_FSATTR_VOLNAME,
    UVFS_FSATTR_VOLUUID,
    UVFS_FSATTR_CAPS_FORMAT,
    UVFS_FSATTR_CAPS_INTERFACES,
    UVFS_FSATTR_LAST_MTIME,
    UVFS_FSATTR_MOUNT_TIME
};

static int
HFSTest_GetFSAttr( UVFSFileNode RootNode )
{
    int iErr        = 0;
    size_t uLen     = 512;
    size_t uRetLen  = 0;
    UVFSFSAttributeValue* psAttrVal = (UVFSFSAttributeValue*)malloc(uLen);
    assert( psAttrVal );

    for ( uint32_t uIdx=0; uIdx<ARR_LEN(gpcFSAttrs); uIdx++ )
    {
        memset( psAttrVal, 0, uLen );

        iErr = HFS_fsOps.fsops_getfsattr( RootNode, gpcFSAttrs[uIdx], psAttrVal, uLen, &uRetLen );
        if ( iErr != 0 )
        {
            printf( "fsops_getfsattr attr = %s return with error code [%d]\n", gpcFSAttrs[uIdx], iErr );
            goto exit;
        }

        printf( "FSAttr = [%s] Value = [", gpcFSAttrs[uIdx]);
        if ( UVFS_FSATTR_IS_BOOL( gpcFSAttrs[uIdx] ) )
        {
            printf( psAttrVal->fsa_bool? "true" : "false" );
        }
        else if ( UVFS_FSATTR_IS_NUMBER( gpcFSAttrs[uIdx] ) )
        {
            printf( "%llu", psAttrVal->fsa_number );
        }
        else if ( UVFS_FSATTR_IS_OPAQUE( gpcFSAttrs[uIdx] ) )
        {
            printf("0x");
            for ( uint32_t uOp=0; uOp<uRetLen; uOp++ )
            {
                printf( "%x", psAttrVal->fsa_opaque[uOp] );
            }
        }
        else if ( UVFS_FSATTR_IS_STRING( gpcFSAttrs[uIdx] ) )
        {
            printf( "%s", psAttrVal->fsa_string );
        }
        else
        {
            assert(0);
        }
        printf("].\n");
    }

exit:
    free(psAttrVal);
    return (iErr);
}

static int
HFSTest_WriteRead( UVFSFileNode RootNode )
{
#define FILENAME    "NewFileForTest"
#define MAXFILESIZE (1024*1024*1024)

    int iErr                = 0;
    UVFSFileNode psFile     = NULL;
    size_t iActuallyWrite   = 0;
    size_t iActuallyRead   = 0;
    void* pvOutBuf          = malloc(MAXFILESIZE);
    void* pvInBuf           = malloc(MAXFILESIZE);
    assert( pvOutBuf != NULL && pvInBuf != NULL );
    uint64_t* puOutBuf  = pvOutBuf;
    uint64_t* puInBuf   = pvInBuf;

    // Create new file with size 50,000 Bytes
    assert( CreateNewFile( RootNode, &psFile, FILENAME, 50000 ) == 0 );

    // lets write 10,000 Bytes with 0xCD
    memset(pvOutBuf, 0, MAXFILESIZE);
    memset(pvInBuf, 0, MAXFILESIZE);

    memset(pvOutBuf, 0xCD, 10000);

    assert( HFS_fsOps.fsops_write( psFile, 0, 10000, pvOutBuf, &iActuallyWrite ) == 0 );
    assert( HFS_fsOps.fsops_read( psFile, 0, 10000, pvInBuf, &iActuallyRead ) == 0 );

    // Lets test it...
    for ( uint64_t uIdx=0; uIdx<(MAXFILESIZE/sizeof(uint64_t)); uIdx++ )
    {
        assert( puInBuf[uIdx] == puOutBuf[uIdx] );
    }

    // Lets extend the file to 100,000 Bytes...
    memset(pvOutBuf+10000, 0xED, 90000);
    assert( HFS_fsOps.fsops_write( psFile, 10000, 90000, pvOutBuf+10000, &iActuallyWrite ) == 0 );
    assert( HFS_fsOps.fsops_read( psFile, 0, 100000, pvInBuf, &iActuallyRead ) == 0 );

    // Lets test it...
    for ( uint64_t uIdx=0; uIdx<(MAXFILESIZE/sizeof(uint64_t)); uIdx++ )
    {
        assert( puInBuf[uIdx] == puOutBuf[uIdx] );
    }

    memset(pvOutBuf, 0, MAXFILESIZE);
    memset(pvInBuf, 0, MAXFILESIZE);
    assert( SetAttrChangeSize(psFile, 10000) == 0 );
    memset(pvOutBuf, 0xCD, 10000);
    memset(pvOutBuf+20000, 0xBB, 10000);

    assert( HFS_fsOps.fsops_write( psFile, 20000, 10000, pvOutBuf+20000, &iActuallyWrite ) == 0 );
    assert( HFS_fsOps.fsops_read( psFile, 0, 30000, pvInBuf, &iActuallyRead ) == 0 );

    // Lets test it...
    for ( uint64_t uIdx=0; uIdx<(MAXFILESIZE/sizeof(uint64_t)); uIdx++ )
    {
        assert( puInBuf[uIdx] == puOutBuf[uIdx] );
    }

    HFS_fsOps.fsops_reclaim( psFile );

    goto exit;

exit:
    return iErr;
}

static int
HFSTest_RandomIO( UVFSFileNode RootNode )
{
#define MAX_IO_SIZE (1024*1024)
#define MAX_IO_OFFSET (80*MAX_IO_SIZE)
#define TEST_RUN_TIME_SEC (30)

    int iErr = 0;
    UVFSFileNode psFile;
    static mach_timebase_info_data_t sTimebaseInfo;
    mach_timebase_info(&sTimebaseInfo);

    void* pvWriteBuf    = malloc(MAX_IO_SIZE);
    void* pvReadBuf     = malloc(MAX_IO_SIZE);

    int* puBuf = pvWriteBuf;
    for ( uint64_t uIdx=0; uIdx<(MAX_IO_SIZE/sizeof(int)); uIdx++ )
    {
        puBuf[uIdx] = rand();
    }

    iErr = CreateNewFile( RootNode, &psFile, "SimpleFile", 0 );
    assert(iErr == 0);

    uint64_t start = mach_absolute_time();
    uint64_t elapsedSec = 0;
    // while( elapsedSec < TEST_RUN_TIME_SEC )
    for(uint32_t uWriteReadCnt=1000; uWriteReadCnt; uWriteReadCnt--)
    {
        uint64_t uNextIOSize   = rand() % MAX_IO_SIZE;
        uint64_t uNextIOOffset = rand() % MAX_IO_OFFSET;

        printf("uNextIOSize = %llu, uNextIOOffset = %llu\n", uNextIOSize, uNextIOOffset);

        size_t iActuallyWrite;
        size_t iActuallyRead;
        
        iErr = HFS_fsOps.fsops_write( psFile, uNextIOOffset, uNextIOSize, pvWriteBuf, &iActuallyWrite );
        assert(iErr==0);
        iErr = HFS_fsOps.fsops_read( psFile, uNextIOOffset, uNextIOSize, pvReadBuf, &iActuallyRead );
        assert(iErr==0);

        uint8_t* puRead = pvReadBuf;
        uint8_t* puWrite = pvWriteBuf;
        for ( uint64_t uIdx=0; uIdx<uNextIOSize; uIdx++ )
        {
            assert( puRead[uIdx] == puWrite[uIdx] );
        }

        uint64_t end = mach_absolute_time();
        uint64_t elapsed = end - start;
        uint64_t elapsedNano = elapsed * sTimebaseInfo.numer / sTimebaseInfo.denom;
        elapsedSec = elapsedNano / 1000 / 1000 / 1000;
    }

    free(pvReadBuf);
    free(pvWriteBuf);

    HFS_fsOps.fsops_reclaim(psFile);

    return 0;
}

static int
HFSTest_HardLink( UVFSFileNode RootNode )
{
    int iErr                = 0;

    // Validate files exist on media
    UVFSFileNode psOriginalFile = NULL;
    iErr = HFS_fsOps.fsops_lookup( RootNode, "original_file.txt", &psOriginalFile );
    printf("Lookup for original file err [%d]\n", iErr);
    if ( iErr )
        return iErr;

    UVFSFileNode psFirstLink = NULL;
    iErr = HFS_fsOps.fsops_lookup( RootNode, "first_link.txt", &psFirstLink );
    printf("Lookup for original file err [%d]\n", iErr);
    if ( iErr )
        return iErr;

    UVFSFileNode psDirectory = NULL;
    iErr = HFS_fsOps.fsops_lookup( RootNode, "dir", &psDirectory );
    printf("Lookup for original file err [%d]\n", iErr);
    if ( iErr )
        return iErr;

    UVFSFileNode psSecondLink = NULL;
    iErr = HFS_fsOps.fsops_lookup( psDirectory, "second_link.txt", &psSecondLink );
    printf("Lookup for original file err [%d]\n", iErr);
    if ( iErr )
        return iErr;


    UVFSFileAttributes sOutAttrs;
    iErr = HFS_fsOps.fsops_getattr(psOriginalFile, &sOutAttrs);
    printf("GetAttr for original file err [%d]\n", iErr);
    if (iErr)
        return iErr;

    if (sOutAttrs.fa_nlink != 3)
    {
        printf("nlink of original file should be 3, got [%d]\n", sOutAttrs.fa_nlink);
        return 1;
    }

    void* pvOriginalFileBuf          = malloc(sOutAttrs.fa_size);
    void* pvFirstLinkeBuf            = malloc(sOutAttrs.fa_size);
    void* pvSecondLinkeBuf           = malloc(sOutAttrs.fa_size);
    uint64_t* puOriginalFileBuf  = pvOriginalFileBuf;
    uint64_t* puFirstLinkeBuf    = pvFirstLinkeBuf;
    uint64_t* puSecondLinkeBuf   = pvSecondLinkeBuf;

    size_t iActuallyRead   = 0;
    assert( HFS_fsOps.fsops_read( psOriginalFile, 0, sOutAttrs.fa_size, pvOriginalFileBuf, &iActuallyRead ) == 0 );
    assert( HFS_fsOps.fsops_read( psFirstLink,    0, sOutAttrs.fa_size, pvFirstLinkeBuf, &iActuallyRead ) == 0 );
    assert( HFS_fsOps.fsops_read( psSecondLink,   0, sOutAttrs.fa_size, pvSecondLinkeBuf, &iActuallyRead ) == 0 );

    // Lets test if all links has the same content it...
    for ( uint64_t uIdx=0; uIdx<(sOutAttrs.fa_size/sizeof(uint64_t)); uIdx++ )
    {
        assert( puOriginalFileBuf[uIdx] == puFirstLinkeBuf[uIdx] );
        assert( puOriginalFileBuf[uIdx] == puSecondLinkeBuf[uIdx] );
    }

    // Save content of the original file and fill up 0x1000 0xAAs
    void* pvNewContentBuf  = malloc(sOutAttrs.fa_size + 1000);
    uint64_t* puNewContentBuf   = pvNewContentBuf;

    memcpy(pvNewContentBuf, pvOriginalFileBuf ,sOutAttrs.fa_size);
    memset(pvNewContentBuf + sOutAttrs.fa_size, 0xAA,1000);
    
    assert( HFS_fsOps.fsops_write( psOriginalFile, 0, sOutAttrs.fa_size + 1000, pvNewContentBuf, &iActuallyRead ) == 0 );
    free(pvOriginalFileBuf);
    free(pvFirstLinkeBuf);
    free(pvSecondLinkeBuf);

    pvOriginalFileBuf  = malloc(sOutAttrs.fa_size + 1000);
    pvFirstLinkeBuf    = malloc(sOutAttrs.fa_size + 1000);
    pvSecondLinkeBuf   = malloc(sOutAttrs.fa_size + 1000);
    puOriginalFileBuf  = pvOriginalFileBuf;
    puFirstLinkeBuf    = pvFirstLinkeBuf;
    puSecondLinkeBuf   = pvSecondLinkeBuf;

    // Make sure the file and its hardlinks have the new content
    assert( HFS_fsOps.fsops_read( psOriginalFile, 0, sOutAttrs.fa_size + 1000, pvOriginalFileBuf, &iActuallyRead ) == 0 );
    assert( HFS_fsOps.fsops_read( psFirstLink,    0, sOutAttrs.fa_size + 1000, pvFirstLinkeBuf, &iActuallyRead ) == 0 );
    assert( HFS_fsOps.fsops_read( psSecondLink,   0, sOutAttrs.fa_size + 1000, pvSecondLinkeBuf, &iActuallyRead ) == 0 );
    for ( uint64_t uIdx=0; uIdx<((sOutAttrs.fa_size + 1000)/sizeof(uint64_t)); uIdx++ )
    {
        assert( puOriginalFileBuf[uIdx] == puNewContentBuf[uIdx] );
        assert( puOriginalFileBuf[uIdx] == puFirstLinkeBuf[uIdx] );
        assert( puOriginalFileBuf[uIdx] == puSecondLinkeBuf[uIdx] );
    }

    iErr = HFS_fsOps.fsops_remove( RootNode, "original_file.txt", NULL);
    printf( "Remove original file err [%d]\n", iErr );
    if ( iErr != 0 )
        return iErr;

    iErr = HFS_fsOps.fsops_getattr(psFirstLink, &sOutAttrs);
    printf("GetAttr for first link err [%d]\n", iErr);
    if (iErr)
        return iErr;

    if (sOutAttrs.fa_nlink != 2)
    {
        printf("nlink of first link should be 2, got [%d]\n", sOutAttrs.fa_nlink);
        return 1;
    }

    RenameFile(psDirectory, psSecondLink,"second_link.txt", RootNode, psFirstLink,"first_link.txt" );

    iErr = HFS_fsOps.fsops_getattr(psSecondLink, &sOutAttrs);
    printf("GetAttr for second link err [%d]\n", iErr);
    if (iErr)
        return iErr;
    

    if (sOutAttrs.fa_nlink != 2)
    {
        printf("nlink of first link should be 2, got [%d]\n", sOutAttrs.fa_nlink);
        return 1;
    }

    iErr = HFS_fsOps.fsops_remove( RootNode, "first_link.txt", NULL);
    printf( "Remove first link err [%d]\n", iErr );
    if ( iErr != 0 )
        return iErr;

    free(pvOriginalFileBuf);
    free(pvFirstLinkeBuf);
    free(pvSecondLinkeBuf);
    free(pvNewContentBuf);
    HFS_fsOps.fsops_reclaim(psOriginalFile);
    HFS_fsOps.fsops_reclaim(psFirstLink);
    HFS_fsOps.fsops_reclaim(psSecondLink);
    HFS_fsOps.fsops_reclaim(psDirectory);

    return iErr;
}

static int
HFSTest_CreateHardLink( UVFSFileNode RootNode )
{
    uint32_t uOriginalFileSize = 500000;

    int iErr                = 0;
    UVFSFileNode psFile     = NULL;
    size_t iActuallyWrite   = 0;
    size_t iActuallyRead   = 0;
    void* pvOutBuf          = malloc(uOriginalFileSize);
    void* pvInBuf           = malloc(uOriginalFileSize);
    assert( pvOutBuf != NULL && pvInBuf != NULL );
    uint64_t* puOutBuf  = pvOutBuf;
    uint64_t* puInBuf   = pvInBuf;

    // Create the original file with size 500,000 Bytes
    iErr = CreateNewFile( RootNode, &psFile, "original_file.txt", uOriginalFileSize);
    assert( iErr == 0 );

    // lets write 10,000 Bytes with 0xCD
    memset(pvOutBuf, 0, uOriginalFileSize);
    memset(pvInBuf, 0, uOriginalFileSize);

    memset(pvOutBuf, 0xCD, uOriginalFileSize);

    assert( HFS_fsOps.fsops_write( psFile, 0, uOriginalFileSize, pvOutBuf, &iActuallyWrite ) == 0 );
    assert( HFS_fsOps.fsops_read( psFile, 0, uOriginalFileSize, pvInBuf, &iActuallyRead ) == 0 );

    // Lets test it...
    for ( uint64_t uIdx=0; uIdx<(uOriginalFileSize/sizeof(uint64_t)); uIdx++ )
    {
        assert( puInBuf[uIdx] == puOutBuf[uIdx] );
    }

    UVFSFileNode psDirectory = NULL;
    assert (CreateNewFolder(RootNode,&psDirectory,"dir") == 0);
    assert (CreateHardLink(psFile,RootNode,"first_link.txt") == 0);
    assert (CreateHardLink(psFile,psDirectory,"second_link.txt") == 0);

    HFS_fsOps.fsops_reclaim( psFile );
    HFS_fsOps.fsops_reclaim( psDirectory );

    assert (HFSTest_HardLink( RootNode ) == 0);

    goto exit;

exit:
    return iErr;
}

static int
HFSTest_RenameToHardlink( UVFSFileNode RootNode )
{
    UVFSFileNode psFile0            = NULL;
    UVFSFileNode psFile1            = NULL;
    UVFSFileNode psLink             = NULL;
    UVFSFileAttributes sOutAttrs    = {0};

    assert ( CreateNewFile( RootNode, &psFile0, "simple_file.txt", 0 ) == 0 );
    assert ( CreateNewFile( RootNode, &psFile1, "original_file.txt", 0 ) == 0 );
    assert ( CreateHardLink( psFile1, RootNode, "first_link.txt" ) == 0 );
    assert ( CreateHardLink( psFile1, RootNode, "second_link.txt" ) == 0 );
    assert ( HFS_fsOps.fsops_lookup( RootNode, "first_link.txt", &psLink ) == 0 );
    assert( RenameFile(RootNode, psFile0, "simple_file.txt", RootNode, psLink, "first_link.txt") == 0 );

    assert( HFS_fsOps.fsops_getattr(psFile1, &sOutAttrs) == 0 );

    assert( HFS_fsOps.fsops_reclaim(psFile0) == 0 );
    for ( uint32_t uIdx=0; uIdx<sOutAttrs.fa_nlink; uIdx++ )
        assert( HFS_fsOps.fsops_reclaim(psFile1) == 0 );

    return 0;
}

static int
HFSTest_JustMount( __unused UVFSFileNode RootNode )
{
    return 0;
}

static int HFSTest_OpenJournal( __unused UVFSFileNode RootNode ) {
    
    printf("HFSTest_OpenJournal:\n");
    
    return 0;
}

static int HFSTest_WriteToJournal(UVFSFileNode RootNode ) {
    int iErr = 0;
    
    printf("HFSTest_WriteToJournal:\n");
    
    printf("Create a new folder:\n");
    UVFSFileNode TestFolder = NULL;
    iErr = CreateNewFolder( RootNode, &TestFolder, "TestFolder");
    printf("CreateNewFolder err [%d]\n", iErr);
    if (iErr) {
        return iErr;
    }

    printf("Create new file with size 0:\n");
    UVFSFileNode TestFile1 = NULL;
    CreateNewFile(TestFolder, &TestFile1, "TestFile.txt",0);
    printf("Create TestFile in TestFolder err [%d]\n", iErr);
    if (iErr) {
        return iErr;
    }
    
    printf("Create new file with size 512:\n");
    UVFSFileNode TestFile2 = NULL;
    CreateNewFile(TestFolder, &TestFile2, "TestFile2.txt",512);
    printf("Create TestFile2 in TestFolder err [%d]\n", iErr);
    if (iErr) {
        return iErr;
    }

    uint32_t uEntrySize = sizeof(UVFSDirEntryAttr) + MAX_UTF8_NAME_LENGTH;
    UVFSDirEntryAttr *psReadDirTestsData = malloc(2*uEntrySize);
    if (psReadDirTestsData == NULL)
        return ENOMEM;

    UVFSDirEntryAttr *psCurrentReadDirTestsData = psReadDirTestsData;
    SetExpectedAttr("TestFile.txt", UVFS_FA_TYPE_FILE, psCurrentReadDirTestsData);
    iErr = HFS_fsOps.fsops_getattr( TestFile1, &psCurrentReadDirTestsData->dea_attrs );
    psCurrentReadDirTestsData = (UVFSDirEntryAttr *) ((void*) psCurrentReadDirTestsData + uEntrySize);
    SetExpectedAttr("TestFile2.txt", UVFS_FA_TYPE_FILE, psCurrentReadDirTestsData);
    iErr = HFS_fsOps.fsops_getattr( TestFile2, &psCurrentReadDirTestsData->dea_attrs );

    printf("Read DIR attr:\n");
    iErr = ReadDirAttr(TestFolder, psReadDirTestsData, 2);
    free(psReadDirTestsData);
    printf("ReadDirAttr err [%d]\n", iErr);
    if (iErr) {
        goto exit;
    }
    
    printf("Remove File1:\n");
    iErr =  RemoveFile(TestFolder,"TestFile.txt");
    printf("Remove File TestFile from TestFolder err [%d]\n", iErr);
    if (iErr) {
        goto exit;
    }
    
    printf("Remove File2:\n");
    iErr =  RemoveFile(TestFolder,"TestFile2.txt");
    printf("Remove File TestFile2 from TestFolder err [%d]\n", iErr);
    if (iErr) {
        goto exit;
    }
    
    printf("Remove TestFolder:\n");
    iErr =  RemoveFolder(RootNode,"TestFolder");
    printf("Remove Folder TestFolder from Root err [%d]\n", iErr);
    if (iErr) {
        goto exit;
    }
    
exit:
    HFS_fsOps.fsops_reclaim(TestFolder);
    HFS_fsOps.fsops_reclaim(TestFile1);
    HFS_fsOps.fsops_reclaim(TestFile2);
    return iErr;
}

static int __used
HFSTest_SetXattr( UVFSFileNode RootNode )
{
    const char * pcAttr = "com.apple.test.set";
    const char * pcAttr2 = "com.apple.test.set2";
    char pcData[] = "This is attribute data";
    char pcData2[] = "This is attribute data 2";
    int iErr = 0;

    UVFSFileNode TestFile = NULL;
    iErr = HFS_fsOps.fsops_lookup( RootNode, "kl_set.test", &TestFile );
    if ( iErr )
    {
        printf("Lookup err [%d]\n", iErr);
        return iErr;
    }

    // Add Attribute - create
    iErr = HFS_fsOps.fsops_setxattr(TestFile, pcAttr, pcData, strlen(pcData)+1, UVFSXattrHowCreate);
    if ( iErr )
    {
        printf("SetAttr err [%d]\n", iErr);
        goto out;
    }

    // Add Attribute - create with failure
    iErr = HFS_fsOps.fsops_setxattr(TestFile, pcAttr, pcData2, strlen(pcData2)+1, UVFSXattrHowCreate);
    if ( iErr != EEXIST)
    {
        printf("SetAttr err [%d]\n", iErr);
        goto out;
    }

    // Add Attribute - set
    iErr = HFS_fsOps.fsops_setxattr(TestFile, pcAttr, pcData2, strlen(pcData2)+1, UVFSXattrHowSet);
    if ( iErr )
    {
        printf("SetAttr err [%d]\n", iErr);
        goto out;
    }

    // Add Attribute - replace
    iErr = HFS_fsOps.fsops_setxattr(TestFile, pcAttr, pcData, strlen(pcData)+1, UVFSXattrHowReplace);
    if ( iErr )
    {
        printf("SetAttr err [%d]\n", iErr);
        goto out;
    }

    // Add Attribute - replace with failure
    iErr = HFS_fsOps.fsops_setxattr(TestFile, pcAttr2, pcData, strlen(pcData)+1, UVFSXattrHowReplace);
    if ( iErr != ENOATTR )
    {
        printf("SetAttr err [%d]\n", iErr);
        goto out;
    }

    // Add Attribute - remove
    iErr = HFS_fsOps.fsops_setxattr(TestFile, pcAttr, pcData, strlen(pcData)+1, UVFSXattrHowRemove);
    if ( iErr )
    {
        printf("SetAttr err [%d]\n", iErr);
        goto out;
    }

    // Add Attribute - remove with failure
    iErr = HFS_fsOps.fsops_setxattr(TestFile, pcAttr, pcData, strlen(pcData)+1, UVFSXattrHowRemove);
    if ( iErr != ENOATTR )
    {
        printf("SetAttr err [%d]\n", iErr);
        goto out;
    }

    // Get Attribute - check nothing left
    size_t actual_size = INT32_MAX;
    iErr = HFS_fsOps.fsops_listxattr(TestFile, NULL, 0, &actual_size);
    if ( iErr || (actual_size != 0))
    {
        printf("ListAttr err [%d]\n", iErr);
        goto out;
    }

    // Add Attribute - create extended attribute and check content
    const char * pcAttr3 = "com.apple.test.set3";
    uint8_t *pBuffer = NULL;
    uint8_t *pBufferRet = NULL;

    // Test more than one sector and ending outside boundary.
#define ATTR_EXT_SIZE   (5000)

    pBuffer = malloc(ATTR_EXT_SIZE);
    pBufferRet = malloc(ATTR_EXT_SIZE);

    if ( pBuffer == NULL || pBufferRet == NULL)
    {
        iErr = ENOMEM;
        goto out;
    }

    for (int i = 0; i < ATTR_EXT_SIZE; ++i)
    {
        pBuffer[i] = i % 256;
        pBufferRet[i] = 0xff;
    }

    iErr = HFS_fsOps.fsops_setxattr(TestFile, pcAttr3, pBuffer, ATTR_EXT_SIZE, UVFSXattrHowCreate);
    if ( iErr )
    {
        printf("SetAttr err [%d]\n", iErr);
        goto out_mem;
    }

    iErr = HFS_fsOps.fsops_getxattr(TestFile, pcAttr3, pBufferRet, ATTR_EXT_SIZE, &actual_size);
    if ( iErr )
    {
        printf("GetAttr err [%d]\n", iErr);
        goto out_mem;
    }

    // check data
    assert(actual_size == ATTR_EXT_SIZE);

    for (int i = 0; i < ATTR_EXT_SIZE; ++i)
    {
        assert(pBuffer[i] == pBufferRet[i]);
    }

    iErr = HFS_fsOps.fsops_setxattr(TestFile, pcAttr3, pBuffer, ATTR_EXT_SIZE, UVFSXattrHowRemove);
    if ( iErr )
    {
        printf("SetAttr err [%d]\n", iErr);
        goto out_mem;
    }

out_mem:
    free(pBuffer);
    free(pBufferRet);

out:
    // Reclaim test file
    HFS_fsOps.fsops_reclaim(TestFile);

    return iErr;
}


static int __used
HFSTest_ListXattr( UVFSFileNode RootNode )
{
    int iErr = 0;

    UVFSFileNode TestFile = NULL;
    iErr = HFS_fsOps.fsops_lookup( RootNode, "kl.test", &TestFile );
    if ( iErr )
    {
        printf("Lookup err [%d]\n", iErr);
        return iErr;
    }

    // Get required size
    size_t actual_size = 0;
    iErr = HFS_fsOps.fsops_listxattr(TestFile, NULL, 0, &actual_size);
    if ( iErr )
    {
        printf("ListAttr err [%d]\n", iErr);
        goto out;
    }

    // Get Attributes
    size_t size = actual_size;
    char *pcBuffer = malloc(size);
    if ( pcBuffer == NULL )
    {
        iErr = ENOMEM;
        goto out;
    }

    actual_size = 0;
    iErr = HFS_fsOps.fsops_listxattr(TestFile, pcBuffer, size, &actual_size);
    if ( iErr )
    {
        printf("ListAttr err [%d]\n", iErr);
        goto mem_err;
    }

    // Just check it
    assert(actual_size == size);

    // Print Attributes Names
    size_t attr_size = 0;
    char *pcAttribues = pcBuffer;

    while (attr_size < size)
    {
        // Get required size
        actual_size = 0;
        iErr = HFS_fsOps.fsops_getxattr(TestFile, pcAttribues, NULL, 0, &actual_size);
        if ( iErr )
        {
            printf("GetAttr size err [%d]\n", iErr);
            goto mem_err;
        }

        // Get Attributes
        size_t bufsize = actual_size+1;
        char *pcAttrBuffer = malloc(bufsize);
        if ( pcAttrBuffer == NULL )
        {
            iErr = ENOMEM;
            goto mem_err;
        }
        bzero(pcAttrBuffer, bufsize);

        HFS_fsOps.fsops_getxattr(TestFile, pcAttribues, pcAttrBuffer, bufsize, &actual_size);
        if ( iErr )
        {
            printf("GetAttr err [%d]\n", iErr);
            free(pcAttrBuffer);
            goto mem_err;
        }

        printf("Found attribute '%s' : %s\n", pcAttribues, pcAttrBuffer);

        free(pcAttrBuffer);

        size_t curr_attr_size = strlen(pcAttribues) + 1;

        attr_size += curr_attr_size;
        pcAttribues += curr_attr_size;
    }

mem_err:
    free(pcBuffer);

out:
    // Reclaim test file
    HFS_fsOps.fsops_reclaim(TestFile);

    return iErr;
}

/*
 *  Tests List Struct.
 */

#if HFS_CRASH_TEST
    #define ADD_TEST_WITH_CRASH_ABORT(testName, dmgPath, testHandler, CrashAbortType, CrashAbortCallback, CrashAbortCount)    \
    { .pcTestName = testName, .pcDMGPath = dmgPath, .pfTestHandler = testHandler,                                             \
      .eCrashID = CrashAbortType, .pAbortFunc = CrashAbortCallback,                                                           \
      .uCrashAbortCnt = CrashAbortCount                                                                                       \
    }
#endif

#define ADD_TEST(testName, dmgPath, testHandler)                              \
{ .pcTestName = testName, .pcDMGPath = dmgPath, .pfTestHandler = testHandler, \
}

#define ADD_TEST_NO_SYNC(testName, dmgPath, testHandler)                      \
{ .pcTestName = testName, .pcDMGPath = dmgPath, .pfTestHandler = testHandler, \
}

TestData_S  gsTestsData[] = {
#if 1 // Enable non-journal tests
    ADD_TEST( "HFSTest_JustMount",               "/Volumes/SSD_Shared/FS_DMGs/HFSEmpty.dmg",         &HFSTest_JustMount ),
    ADD_TEST( "HFSTest_ReadDefragmentFile",      "/Volumes/SSD_Shared/FS_DMGs/HFSDeFragment.dmg",    &HFSTest_ReadDefragmentFile ),
    ADD_TEST( "HFSTest_RemoveDir",               "/Volumes/SSD_Shared/FS_DMGs/HFSRemoveDir.dmg",     &HFSTest_RemoveDir ),
    ADD_TEST( "HFSTest_Remove",                  "/Volumes/SSD_Shared/FS_DMGs/HFSRemove.dmg",        &HFSTest_Remove ),
    ADD_TEST( "HFSTest_ReadDir",                 "/Volumes/SSD_Shared/FS_DMGs/HFSReadDir.dmg",       &HFSTest_ReadDir ),
    ADD_TEST( "HFSTest_ReadDirAttr",             "/Volumes/SSD_Shared/FS_DMGs/HFSReadDir.dmg",       &HFSTest_ReadDirAttr ),
    ADD_TEST( "HFSTest_MakeDir",                 "/Volumes/SSD_Shared/FS_DMGs/HFSEmpty.dmg",         &HFSTest_MakeDir ),
    ADD_TEST( "HFSTest_SetAttr",                 "/Volumes/SSD_Shared/FS_DMGs/HFSSetAttr.dmg",       &HFSTest_SetAttr ),
    ADD_TEST( "HFSTest_ReadSymLink",             "/Volumes/SSD_Shared/FS_DMGs/HFSReadSymLink.dmg",   &HFSTest_ReadSymlink ),
    ADD_TEST( "HFSTest_GetFSAttr",               "/Volumes/SSD_Shared/FS_DMGs/HFSEmpty.dmg",         &HFSTest_GetFSAttr ),
    ADD_TEST( "HFSTest_Create",                  "/Volumes/SSD_Shared/FS_DMGs/HFSEmpty.dmg",         &HFSTest_Create ),
    ADD_TEST( "HFSTest_Symlink",                 "/Volumes/SSD_Shared/FS_DMGs/HFSEmpty.dmg",         &HFSTest_Symlink ),
    ADD_TEST( "HFSTest_SymlinkOnFile",           "/Volumes/SSD_Shared/FS_DMGs/HFSEmpty.dmg",         &HFSTest_SymlinkOnFile ),
    ADD_TEST( "HFSTest_Rename",                  "/Volumes/SSD_Shared/FS_DMGs/HFSEmpty.dmg",         &HFSTest_Rename ),
    ADD_TEST( "HFSTest_WriteRead",               "/Volumes/SSD_Shared/FS_DMGs/HFSEmpty.dmg",         &HFSTest_WriteRead ),
    ADD_TEST( "HFSTest_RandomIO",                "/Volumes/SSD_Shared/FS_DMGs/HFS100MB.dmg",         &HFSTest_RandomIO ),
    ADD_TEST( "HFSTest_Create1000Files",         "/Volumes/SSD_Shared/FS_DMGs/HFSEmpty.dmg",         &HFSTest_Create1000Files ),
    ADD_TEST( "HFSTest_HardLink",                "/Volumes/SSD_Shared/FS_DMGs/HFSHardLink.dmg",      &HFSTest_HardLink ),
    ADD_TEST( "HFSTest_CreateHardLink",          "/Volumes/SSD_Shared/FS_DMGs/HFSEmpty.dmg",         &HFSTest_CreateHardLink ),
    ADD_TEST( "HFSTest_RenameToHardlink",        "/Volumes/SSD_Shared/FS_DMGs/HFSEmpty.dmg",         &HFSTest_RenameToHardlink ),
    ADD_TEST( "HFSTest_SetXattr",                "/Volumes/SSD_Shared/FS_DMGs/HFSXattr.dmg",         &HFSTest_SetXattr ),
    ADD_TEST( "HFSTest_ListXattr",               "/Volumes/SSD_Shared/FS_DMGs/HFSXattr.dmg",         &HFSTest_ListXattr ),
    ADD_TEST( "HFSTest_RootFillUp",              "/Volumes/SSD_Shared/FS_DMGs/HFSEmpty.dmg",         &HFSTest_RootFillUp ),
    ADD_TEST( "HFSTest_ScanDir",                 "/Volumes/SSD_Shared/FS_DMGs/HFSEmpty.dmg",         &HFSTest_ScanDir ),
    ADD_TEST( "HFSTest_MultiThreadedRW",         CREATE_HFS_DMG,                                     &HFSTest_MultiThreadedRW_wJournal ),
    ADD_TEST_NO_SYNC( "HFSTest_ValidateUnmount", "/Volumes/SSD_Shared/FS_DMGs/HFSEmpty.dmg",         &HFSTest_ValidateUnmount ),
    ADD_TEST( "HFSTest_ScanID",                  "/Volumes/SSD_Shared/FS_DMGs/HFSEmpty.dmg",         &HFSTest_ScanID ),
#endif
#if 1 // Enbale journal-tests
    ADD_TEST( "HFSTest_OpenJournal",                 "/Volumes/SSD_Shared/FS_DMGs/HFSJ-Empty.dmg",           &HFSTest_OpenJournal ),
    ADD_TEST( "HFSTest_WriteToJournal",              "/Volumes/SSD_Shared/FS_DMGs/HFSJ-Empty.dmg",           &HFSTest_WriteToJournal ),
    ADD_TEST( "HFSTest_JustMount_wJournal",          "/Volumes/SSD_Shared/FS_DMGs/HFSJ-Empty.dmg",           &HFSTest_JustMount ),
    ADD_TEST( "HFSTest_ReadDefragmentFile_wJournal", "/Volumes/SSD_Shared/FS_DMGs/HFSJ-DeFragment.dmg",      &HFSTest_ReadDefragmentFile ),
    ADD_TEST( "HFSTest_RemoveDir_wJournal",          "/Volumes/SSD_Shared/FS_DMGs/HFSJ-RemoveDir.dmg",       &HFSTest_RemoveDir ),
    ADD_TEST( "HFSTest_Remove_wJournal",             "/Volumes/SSD_Shared/FS_DMGs/HFSJ-Remove.dmg",          &HFSTest_Remove ),
    ADD_TEST( "HFSTest_ReadDir_wJournal",            "/Volumes/SSD_Shared/FS_DMGs/HFSJ-ReadDir.dmg",         &HFSTest_ReadDir ),
    ADD_TEST( "HFSTest_ReadDirAttr_wJournal",        "/Volumes/SSD_Shared/FS_DMGs/HFSJ-ReadDir.dmg",         &HFSTest_ReadDirAttr ),
    ADD_TEST( "HFSTest_MakeDir_wJournal",            "/Volumes/SSD_Shared/FS_DMGs/HFSJ-Empty.dmg",           &HFSTest_MakeDir ),
    ADD_TEST( "HFSTest_SetAttr_wJournal",            "/Volumes/SSD_Shared/FS_DMGs/HFSJ-SetAttr.dmg",         &HFSTest_SetAttr ),
    ADD_TEST( "HFSTest_ReadSymLink_wJournal",        "/Volumes/SSD_Shared/FS_DMGs/HFSJ-ReadSymLink.dmg",     &HFSTest_ReadSymlink ),
    ADD_TEST( "HFSTest_GetFSAttr_wJournal",          "/Volumes/SSD_Shared/FS_DMGs/HFSJ-Empty.dmg",           &HFSTest_GetFSAttr ),
    ADD_TEST( "HFSTest_Create_wJournal",             "/Volumes/SSD_Shared/FS_DMGs/HFSJ-Empty.dmg",           &HFSTest_Create ),
    ADD_TEST( "HFSTest_Symlink_wJournal",            "/Volumes/SSD_Shared/FS_DMGs/HFSJ-Empty.dmg",           &HFSTest_Symlink ),
    ADD_TEST( "HFSTest_SymlinkOnFile",               "/Volumes/SSD_Shared/FS_DMGs/HFSEmpty.dmg",             &HFSTest_SymlinkOnFile ),
    ADD_TEST( "HFSTest_Rename_wJournal",             "/Volumes/SSD_Shared/FS_DMGs/HFSJ-Empty.dmg",           &HFSTest_Rename ),
    ADD_TEST( "HFSTest_WriteRead_wJournal",          "/Volumes/SSD_Shared/FS_DMGs/HFSJ-Empty.dmg",           &HFSTest_WriteRead ),
    ADD_TEST( "HFSTest_RandomIO_wJournal",           "/Volumes/SSD_Shared/FS_DMGs/HFSJ-144MB.dmg",           &HFSTest_RandomIO ),
    ADD_TEST( "HFSTest_Create1000Files_wJournal",    "/Volumes/SSD_Shared/FS_DMGs/HFSJ-EmptyLarge.dmg",      &HFSTest_Create1000Files ),
    ADD_TEST( "HFSTest_HardLink_wJournal",           "/Volumes/SSD_Shared/FS_DMGs/HFSJ-HardLink.dmg",        &HFSTest_HardLink ),
    ADD_TEST( "HFSTest_CreateHardLink_wJournal",     "/Volumes/SSD_Shared/FS_DMGs/HFSJ-EmptyLarge.dmg",      &HFSTest_CreateHardLink ),
    ADD_TEST( "HFSTest_RootFillUp_wJournal",         "/Volumes/SSD_Shared/FS_DMGs/HFSJ-EmptyLarge.dmg",      &HFSTest_RootFillUp ),
    ADD_TEST( "HFSTest_MultiThreadedRW_wJournal",                "",                                         &HFSTest_MultiThreadedRW_wJournal ),
    ADD_TEST( "HFSTest_DeleteAHugeDefragmentedFile_wJournal",    "",                                         &HFSTest_DeleteAHugeDefragmentedFile_wJournal ),
    ADD_TEST( "HFSTest_CreateJournal_Sparse",                CREATE_SPARSE_VOLUME,                           &HFSTest_OpenJournal ),
    ADD_TEST( "HFSTest_MakeDirAndKeep_Sparse",               CREATE_SPARSE_VOLUME,                           &HFSTest_MakeDirAndKeep ),
    ADD_TEST( "HFSTest_CreateAndWriteToJournal_Sparse",      CREATE_SPARSE_VOLUME,                           &HFSTest_WriteToJournal ),
    ADD_TEST( "HFSTest_MultiThreadedRW_wJournal_Sparse",     CREATE_SPARSE_VOLUME,                           &HFSTest_MultiThreadedRW_wJournal ),
    ADD_TEST( "HFSTest_ScanDir",                          "/Volumes/SSD_Shared/FS_DMGs/HFSJ-Empty.dmg",      &HFSTest_ScanDir ),
    ADD_TEST_NO_SYNC( "HFSTest_ValidateUnmount_wJournal", "/Volumes/SSD_Shared/FS_DMGs/HFSJ-Empty.dmg",      &HFSTest_ValidateUnmount_wJournal ),
    ADD_TEST( "HFSTest_Corrupted2ndDiskImage",            "/Volumes/SSD_Shared/FS_DMGs/corrupted_80M.dmg.sparseimage",
                                                                                                             &HFSTest_Corrupted2ndDiskImage ),
    ADD_TEST( "HFSTest_ScanID",                       "/Volumes/SSD_Shared/FS_DMGs/HFSJ-Empty.dmg",          &HFSTest_ScanID ),

#endif
#if HFS_CRASH_TEST
    // The following 2 tests checks mount after unmount, no-journal
    ADD_TEST_WITH_CRASH_ABORT( "HFSTest_OneSync", "/Volumes/SSD_Shared/FS_DMGs/HFSEmpty.dmg",
                              &HFSTest_OneSync, CRASH_ABORT_RANDOM, HFSTest_SaveDMG, 0 ),
    ADD_TEST( "HFSTest_ConfirmTestFolderExists", TEMP_DMG_BKUP, &HFSTest_ConfirmTestFolderExists ),

    // The following 2 tests checks mount after unmount with journal
    ADD_TEST_WITH_CRASH_ABORT( "HFSTest_OneSync_wJournal", "/Volumes/SSD_Shared/FS_DMGs/HFSJ-Empty.dmg",
                              &HFSTest_OneSync, CRASH_ABORT_RANDOM, HFSTest_SaveDMG, 0 ),
    ADD_TEST( "HFSTest_ConfirmTestFolderExists", TEMP_DMG_BKUP, &HFSTest_ConfirmTestFolderExists ),

    ADD_TEST_WITH_CRASH_ABORT("HFSTest_OpenJournal_wCrashOnMakeDir", "/Volumes/SSD_Shared/FS_DMGs/HFSJ-Empty.dmg",
                              &HFSTest_OpenJournal, CRASH_ABORT_MAKE_DIR, HFSTest_FailTestOnCrashAbort, 0),

    ADD_TEST_WITH_CRASH_ABORT("HFSTest_OpenJournal_wCrashAfterBlockData", "/Volumes/SSD_Shared/FS_DMGs/HFSJ-Empty.dmg",
                              &HFSTest_OpenJournal, CRASH_ABORT_JOURNAL_AFTER_BLOCK_DATA, HFSTest_CrashAbort, 0),

    ADD_TEST_WITH_CRASH_ABORT("HFSTest_OpenJournal_wCrashAfterJournalData", "/Volumes/SSD_Shared/FS_DMGs/HFSJ-Empty.dmg",
                              &HFSTest_OpenJournal, CRASH_ABORT_JOURNAL_AFTER_JOURNAL_DATA, HFSTest_CrashAbort, 0),

    ADD_TEST_WITH_CRASH_ABORT("HFSTest_OpenJournal_wCrashAfterJournalHeader", "/Volumes/SSD_Shared/FS_DMGs/HFSJ-Empty.dmg",
                              &HFSTest_OpenJournal, CRASH_ABORT_JOURNAL_AFTER_JOURNAL_HEADER, HFSTest_CrashAbort, 0),

    ADD_TEST_WITH_CRASH_ABORT("MultiThreadedRW_wJournal_RandomCrash", CREATE_SPARSE_VOLUME,
                              &MultiThreadedRW_wJournal_RandomCrash, CRASH_ABORT_RANDOM, HFSTest_CrashAbortAtRandom, 0),

    // The following 2 tests check journal replay, make sure the drive is mountable, and the created fonder DOES exist
    ADD_TEST_WITH_CRASH_ABORT("HFSTest_MakeDirAndKeep_wCrashAfterJournalHeader", "/Volumes/SSD_Shared/FS_DMGs/HFSJ-Empty.dmg",
                              &HFSTest_MakeDirAndKeep, CRASH_ABORT_JOURNAL_AFTER_JOURNAL_HEADER, HFSTest_CrashAbortOnMkDir, 0),
    ADD_TEST( "HFSTest_ConfirmTestFolderExists", TEMP_DMG_BKUP, &HFSTest_ConfirmTestFolderExists ),

    // The following 2 tests check journal replay, make sure the drive is mountable, and the created fonder does NOT exist
    ADD_TEST_WITH_CRASH_ABORT("HFSTest_MakeDirAndKeep_wCrashAfterJournalData", "/Volumes/SSD_Shared/FS_DMGs/HFSJ-Empty.dmg",
                              &HFSTest_MakeDirAndKeep, CRASH_ABORT_JOURNAL_AFTER_JOURNAL_DATA, HFSTest_CrashAbortOnMkDir, 0),
    ADD_TEST( "HFSTest_ConfirmTestFolderDoesntExists", TEMP_DMG_BKUP, &HFSTest_ConfirmTestFolderDoesntExists ),

    // The following 2 tests check journal replay, make sure the drive is mountable, and the created fonder DOES exist
    ADD_TEST_WITH_CRASH_ABORT("HFSTest_MakeDirAndKeep_wCrashAfterJournalHeader_Sparse", CREATE_SPARSE_VOLUME,
                              &HFSTest_MakeDirAndKeep, CRASH_ABORT_JOURNAL_AFTER_JOURNAL_HEADER, HFSTest_CrashAbortOnMkDir, 1),
    ADD_TEST( "HFSTest_ConfirmTestFolderExists", TEMP_DMG_BKUP_SPARSE, &HFSTest_ConfirmTestFolderExists ),
#endif

};

void *SyncerThread(void *pvArgs) {
    
    int iErr = 0;
    TestData_S *psTestData = pvArgs;
    
    printf("Syncer Thread runs every %u mS\n", guSyncerPeriod);
    
    while(psTestData->bSyncerOn) {
        usleep(guSyncerPeriod * 1000);
        iErr = HFS_fsOps.fsops_sync(psTestData->psRootNode);
        psTestData->uSyncerCount++;
        if (iErr) {
            printf("fsops_sync returned %d\n", iErr);
            break;
        }
    }
    
    TesterThreadReturnStatus_S *psReturnStatus = malloc(sizeof(TesterThreadReturnStatus_S));
    assert(psReturnStatus);
    memset(psReturnStatus, 0, sizeof(*psReturnStatus));
    
    printf("Syncer returns %d\n", iErr);
    
    psReturnStatus->iErr = iErr;
    
    return((void*)psReturnStatus);
}

static int KickOffSyncerThread(TestData_S *psTestData) {

    int iErr = 0;
    
    if (guSyncerPeriod == 0) {
        goto exit;
    }
    psTestData->bSyncerOn = true;
    
    pthread_attr_t sAttr;
    pthread_attr_init(&sAttr);
    pthread_attr_setdetachstate(&sAttr, PTHREAD_CREATE_JOINABLE);
    
    iErr = pthread_create(&psTestData->sSyncerThread, &sAttr, SyncerThread, psTestData);
    
    pthread_attr_destroy(&sAttr);

exit:
    return(iErr);
}

static int ShutdownSyncerThread(TestData_S *psTestData) {
    
    int iErr = 0;
    TesterThreadReturnStatus_S *psReturnStatus = NULL;

    if (guSyncerPeriod == 0) {
        goto exit;
    }
    
    psTestData->bSyncerOn = false;
    iErr = pthread_join(psTestData->sSyncerThread, (void*)&psReturnStatus);
    if (iErr) {
        printf("Error waiting for Syncer thread! %d\n", iErr);
        goto exit;
    }
    
    printf("Syncer Thead ran %u times (iErr %d)\n", psTestData->uSyncerCount, iErr);

    assert(psReturnStatus);

    iErr = psReturnStatus->iErr;
    if (iErr) {
        printf("Syncer thread returned iErr = %d\n", iErr);
        goto exit;
    }

exit:
    if (psReturnStatus) {
        free(psReturnStatus);
    }
        
    return(iErr);
}

static int HFSTest_RunTest(TestData_S *psTestData) {
    UVFSScanVolsRequest sScanVolsReq = {0};
    UVFSScanVolsReply sScanVolsReply = {0};
    int iErr = 0;
    int iFD = HFSTest_PrepareEnv( psTestData );
    giFD = iFD;

    iErr = HFS_fsOps.fsops_taste( iFD );
    printf("Taste err [%d]\n",iErr);
    if ( iErr ) {
        close(iFD);
        HFSTest_DestroyEnv( iFD );
        return(iErr);
    }

    iErr = HFS_fsOps.fsops_scanvols( iFD, &sScanVolsReq, &sScanVolsReply );
    printf("ScanVols err [%d]\n", iErr);
    if ( iErr )
    {
        close(iFD);
        HFSTest_DestroyEnv( iFD );
        return(iErr);
    }

    UVFSFileNode RootNode = NULL;
    iErr = HFS_fsOps.fsops_mount( iFD, sScanVolsReply.sr_volid, 0, NULL, &RootNode );
    printf("Mount err [%d]\n", iErr);
    if ( iErr )
    {
        close(iFD);
        HFSTest_DestroyEnv( iFD );
        return(iErr);
    }
    
    psTestData->psRootNode = RootNode;
    iErr = KickOffSyncerThread(psTestData);
    if ( iErr ) {
        close(iFD);
        HFSTest_DestroyEnv( iFD );
        return(iErr);
    }

    // Execute the test
    iErr = psTestData->pfTestHandler( RootNode );
    printf("Test [%s] finish with error code [%d]\n", psTestData->pcTestName, iErr);
    #if HFS_CRASH_TEST
    if (psTestData->eCrashID == CRASH_ABORT_NONE)
    #endif
        if ( iErr ) {
            close(iFD);
            HFSTest_DestroyEnv( iFD );
            return(iErr);
        }

    iErr = ShutdownSyncerThread(psTestData);
    if ( iErr ) {
        close(iFD);
        HFSTest_DestroyEnv( iFD );
        return(iErr);
    }
    
    iErr = HFS_fsOps.fsops_unmount(RootNode, UVFSUnmountHintNone);
    printf("UnMount err [%d]\n", iErr);
    if ( iErr ) {
        close(iFD);
        HFSTest_DestroyEnv( iFD );
        return(iErr);
    }
    
    HFSTest_PrintCacheStats();
    
    #if HFS_CRASH_TEST
        if (psTestData->eCrashID != CRASH_ABORT_NONE) {
            
            // Execute post crash analysis
            iErr = psTestData->pAbortFunc(psTestData,
                                          gsCrashReport.eCrashID,
                                          iFD,
                                          gsCrashReport.psNode,
                                          gsCrashReport.pSyncerThread);
            
            printf("Analysis [%s] finished with error code [%d]\n", psTestData->pcTestName, iErr);
            
            if (iErr) {
                HFSTest_DestroyEnv( iFD );
                return(iErr);
            }
        } else
    #endif
        {
            // close file
            close(iFD);
            
            //assert(gCacheStat.buf_cache_size == 0);
            
            // Run fsck
            char pcFsckCmd[512] = {0};
            strcat( pcFsckCmd, "/System/Library/Filesystems/hfs.fs/Contents/Resources/fsck_hfs -fd /dev/disk");

            strcat( pcFsckCmd, pcLastDevPathName );
            if (pcDevNum[0] != '\0') {
                strcat( pcFsckCmd, "s" );
                strcat( pcFsckCmd, pcDevNum );
            }
            printf("Execute %s\n", pcFsckCmd);
            iErr = system( pcFsckCmd );
            if ( iErr )
            {
                printf( "*** Fsck CMD failed! (%d)\n", iErr);
                HFSTest_DestroyEnv( iFD );
                return(iErr);
            } else {
                printf( "*** Fsck CMD succeeded!\n");
            }

            HFSTest_DestroyEnv( iFD );
        }
    return(iErr);
}

#if HFS_CRASH_TEST
int HFSTest_CrashAbort_Hanlder(CrashAbort_E eAbort, int iFD, UVFSFileNode psNode, pthread_t pSyncerThread) {
    printf("HFSTest_CrashAbort_Hanlder (%u):\n", guCrashAbortCnt);
    if (guCrashAbortCnt) {
        guCrashAbortCnt--;
        return(0);
    }

    close(iFD); // prevent additional writes to media
    if (pSyncerThread == pthread_self()) {
        printf("Crash Abort on Syncer Thread!\n");
    }
    
    gsCrashReport.uCrashCount++;
    gsCrashReport.eCrashID      = eAbort;
    gsCrashReport.iFD           = iFD;
    gsCrashReport.psNode        = psNode;
    gsCrashReport.pSyncerThread = pSyncerThread;
    
    return(0);
}
#endif


int hfs_tester_run_fsck(void)
{
    // Journaled
    int iErr = HFS_fsOps.fsops_init();
    printf("Init err [%d]\n",iErr);
    if (iErr)
        exit(-1);
    
    TestData_S sTestData = {
        .pcTestName = "hfs_tester_run_fsck",
        .pcDMGPath  = "/Volumes/SSD_Shared/FS_DMGs/HFSJ-Empty.dmg",
    };

    int iFD = HFSTest_PrepareEnv(&sTestData);
    
    iErr = HFS_fsOps.fsops_taste( iFD );
    printf("Taste err [%d]\n",iErr);
    if ( iErr ) {
        close(iFD);
        HFSTest_DestroyEnv( iFD );
        return(iErr);
    }
    
    iErr = HFS_fsOps.fsops_check(iFD, 0, NULL, QUICK_CHECK);
    printf("Check err [%d]\n",iErr);
    if (iErr)
        exit(-1);
    
    close(iFD);
    HFSTest_DestroyEnv( iFD );

    // Non-Journaled
    iErr = HFS_fsOps.fsops_init();
    printf("Init err [%d]\n",iErr);
    if (iErr)
        exit(-1);
    
    sTestData.pcDMGPath  = "/Volumes/SSD_Shared/FS_DMGs/HFSEmpty.dmg";
    
    iFD = HFSTest_PrepareEnv(&sTestData);
    
    iErr = HFS_fsOps.fsops_taste( iFD );
    printf("Taste err [%d]\n",iErr);
    if ( iErr ) {
        close(iFD);
        HFSTest_DestroyEnv( iFD );
        return(iErr);
    }
    
    iErr = HFS_fsOps.fsops_check(iFD, 0, NULL, QUICK_CHECK);
    printf("Check err [%d]\n",iErr);
    if (iErr)
        exit(-1);
    
    close(iFD);
    HFSTest_DestroyEnv( iFD );
    return iErr;
}

int hfs_tester_run(uint32_t uFirstTest, uint32_t uLastTest)
{
    uint32_t uTestRan = 0;
    int iErr = HFS_fsOps.fsops_init();
    printf("Init err [%d]\n",iErr);
    if (iErr)
        exit(-1);
    
    uint32_t uAvailTests = sizeof(gsTestsData)/sizeof(gsTestsData[0]);
    if ((!uLastTest) ||
        (uLastTest > uAvailTests)) {
        
        uLastTest = uAvailTests;
    }
    
    for ( unsigned uTestNum=uFirstTest; uTestNum < uLastTest ; uTestNum++ )
    {
        printf("******************************************************************************************\n");
        printf("**** about to run test [%s] [%u] \n", gsTestsData[uTestNum].pcTestName, uTestNum);
        printf("******************************************************************************************\n");
        
        #if HFS_CRASH_TEST
            HFSTest_ClearCrashAbortFunctionArray();
        
            if (gsTestsData[uTestNum].eCrashID) {
                // Inject Crach condition
                CrashAbort_E eCrashID = gsTestsData[uTestNum].eCrashID;
                printf( "Adding Crash-Abort Function at (%u), %s.\n", eCrashID, ppcCrashAbortDesc[eCrashID] );
                guCrashAbortCnt = gsTestsData[uTestNum].uCrashAbortCnt;
                gpsCrashAbortFunctionArray[eCrashID] = HFSTest_CrashAbort_Hanlder;
                memset(&gsCrashReport, 0, sizeof(gsCrashReport));
            }
        #endif
                
        iErr = HFSTest_RunTest(&gsTestsData[uTestNum]);

        if (iErr) {
            exit(-1);
        }
        uTestRan++;
    }

    HFS_fsOps.fsops_fini();
    
    printf("*** Run %u out of %u tests successfully\n", uTestRan, uAvailTests);

    return 0;
}

/*******************************************/
/*******************************************/
/*******************************************/
// Predefined Tests END.
/*******************************************/
/*******************************************/
/*******************************************/
int main( int argc, const char * argv[] ) {
    uint32_t uFirstTest = 0;
    uint32_t uLastTest = 0;

    time_t sTimeStamp = time(NULL);
    char pcTimeStamp[256] =  {0};
    strcpy(pcTimeStamp, ctime(&sTimeStamp));
    pcTimeStamp[strlen(pcTimeStamp)-1] = '\0'; // remove \n
    sprintf(gpcResultsFolder, "\"/tmp/%s\"", pcTimeStamp);
    printf("*** gpcResultsFolder is %s\n", gpcResultsFolder);


    if ((argc < 2) || (argc > 5))
    {
        printf("Usage : livefiles_hfs_tester < dev-path / RUN_HFS_TESTS > [First Test] [Last Test] [Syncer Period (mS)]\n");
        exit(1);
    }
    
    printf( "livefiles_hfs_tester %s (%u)\n", argv[1], uFirstTest );

    if (argc >= 3) {
        sscanf(argv[2], "%u", &uFirstTest);
    }
    
    if (argc >= 4) {
        sscanf(argv[3], "%u", &uLastTest);
    }
    
    if (argc >= 5) {
        sscanf(argv[4], "%u", &guSyncerPeriod);
    }
    
    if ( strncmp(argv[1], HFS_TEST_PREFIX, strlen(HFS_TEST_PREFIX)) == 0 )
    {
        int err = hfs_tester_run(uFirstTest, uLastTest);
        printf("*** hfs_tester_run return status : %d ***\n", err);
        if (err >= 256) err = -1; // exit code overflow
        exit(err);
        
    } else if  ( strncmp(argv[1], HFS_RUN_FSCK, strlen(HFS_RUN_FSCK)) == 0 )
    {
        int err = hfs_tester_run_fsck();
        printf("*** hfs_tester_run_fsck return status : %d ***\n", err);
        if (err >= 256) err = -1; // exit code overflow
        exit(err);
    }

    UVFSScanVolsRequest sScanVolsReq = {0};
    UVFSScanVolsReply sScanVolsReply = {0};
    int err = 0;
    int fd = open( argv[1], O_RDWR );
    int uCycleCounter = 0;

    UVFSFileNode RootNode = NULL;
    if(fd < 0)
    {
        printf("Failed to open [%s] errno %d\n", argv[1], errno);
        return EBADF;
    }

    do
    {

        err = HFS_fsOps.fsops_init();
        printf("Init err [%d]\n",err);
        if (err) break;

        err = HFS_fsOps.fsops_taste(fd);
        printf("Taste err [%d]\n",err);
        if (err) break;

        err = HFS_fsOps.fsops_scanvols(fd, &sScanVolsReq, &sScanVolsReply);
        printf("ScanVols err [%d]\n",err);
        if (err) break;

        err = HFS_fsOps.fsops_mount(fd, sScanVolsReply.sr_volid, 0, NULL, &RootNode);
        printf("Mount err [%d]\n",err);
        if (err) break;

        ReadDirAttr(RootNode,NULL, 0);

        UVFSFileNode D1_Node = NULL;
        err = CreateNewFolder(RootNode,&D1_Node,"D1");
        printf("CreateNewFolder err [%d]\n",err);
        if (err) break;

        bool bFound = false;
        read_directory_and_search_for_name( RootNode, "D1", &bFound, NULL, 0);
        if (!bFound)
        {
            printf("Dir read failed, D1 wasn't found in Root");
            break;
        }

        HFS_fsOps.fsops_reclaim(D1_Node);
        
        // Remove D1
        err =  RemoveFolder(RootNode,"D1");
        printf("Remove Folder D1 from Root err [%d]\n",err);
        if (err) break;
        
        uCycleCounter++;
    }while(uCycleCounter < TEST_CYCLE_COUNT);

    err = HFS_fsOps.fsops_unmount(RootNode, UVFSUnmountHintNone);
    printf("UnMount err [%d]\n",err);

    return err;
}


