/* Copyright © 2017 Apple Inc. All rights reserved.
 *
 *  usbstorage_tester.c
 *  usbstorage_plugin
 */

#include "FSOPS_Handler.h"
#include "Conv.h"
#include <fcntl.h>
#import  <os/log.h>
#include <time.h>
#include <sys/timeb.h>
#include <sys/time.h>
#include "FAT_Access_M.h"
#include <pthread.h>


#define TEST_1_THREAD_COUNT (2)
#define TEST_2_THREAD_COUNT (4)
#define TEST_CYCLE_COUNT (1)

#define CMP_TIMES(timspec1, timspec2)                    \
((timspec1.tv_sec == timspec2.tv_sec) \
&& (timspec1.tv_nsec == timspec2.tv_nsec))
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

__unused static void print_dir_entry_name( uint32_t uLen, char* pcName, char* pcSearchName, bool* pbFound )
{
    struct unistr255 sU255;
    struct unistr255 sU2552;
    memset( &sU255, 0, sizeof(struct unistr255));
    memset( &sU2552, 0, sizeof(struct unistr255));
    errno_t status = CONV_UTF8ToUnistr255( (uint8_t*)pcName, strlen(pcName), &sU255 , UTF_SFM_CONVERSIONS);
    status |= CONV_UTF8ToUnistr255( (uint8_t*)pcSearchName, strlen(pcSearchName), &sU2552, UTF_SFM_CONVERSIONS);

    if ( status != 0 ) { assert(0); }

    printf( "TESTER Entry Name = [%s]\n", pcName );

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

/* --------------------------------------------------------------------------------------------- */
static int GetFSAttr(UVFSFileNode Node);
static void* MultiThreadSingleFile(void *arg);
static void* MultiThreadMultiFiles(void *arg);
static int RemoveFile(UVFSFileNode ParentNode,char* FileNameToRemove);
static int SetAttrChangeSize(UVFSFileNode FileNode,uint64_t uNewSize);
static int RemoveFolder(UVFSFileNode ParentNode,char* DirNameToRemove);
static int GetAttrAndCompare(UVFSFileNode FileNode,UVFSFileAttributes* sInAttrs);
static int Lookup(UVFSFileNode ParentNode,UVFSFileNode *NewFileNode,char* FileName);
static int CreateNewFolder(UVFSFileNode ParentNode,UVFSFileNode* NewDirNode,char* NewDirName);
static int CreateLink(UVFSFileNode ParentNode,UVFSFileNode* NewLinkNode,char* LinkName,char* LinkContent);
static int CreateNewFile(UVFSFileNode ParentNode,UVFSFileNode* NewFileNode,char* NewFileName,uint64_t size);
static int Rename(UVFSFileNode fromDirNode, UVFSFileNode fromNode, const char *fromName, UVFSFileNode toDirNode, UVFSFileNode toNode, const char *toName);
static void read_directory_and_search_for_name( UVFSFileNode UVFSFileNode, char* pcSearchName, bool* pbFound);
static int ReadLinkToBuffer(UVFSFileNode LinkdNode,void* pvExternalBuffer, uint32_t uExternalBufferSize, size_t* actuallyRead);
static int ReadToBuffer(UVFSFileNode FileToReadNode,uint32_t uOffset, size_t* actuallyRead,void* pvExternalBuffer, uint32_t uExternalBufferSize);
static int WriteFromBuffer(UVFSFileNode FileToWriteNode,uint32_t uOffset, size_t* uActuallyWritten,void* pvExternalBuffer, uint32_t uExternalBufferSize);
static int CloseFile(UVFSFileNode UVFSFileNode);
/* --------------------------------------------------------------------------------------------- */

static int CloseFile(UVFSFileNode UVFSFileNode)
{

    int iErr = MSDOS_fsOps.fsops_sync(UVFSFileNode);

    if (iErr)
    {
        printf("Got Err: %d, while syncing\n",iErr);
        return iErr;
    }

    iErr = MSDOS_fsOps.fsops_reclaim(UVFSFileNode);
    if (iErr)
    {
        printf("Got Err: %d, while reclaiming\n",iErr);
    }

    return iErr;
}

static void read_directory_and_search_for_name( UVFSFileNode UVFSFileNode, char* pcSearchName, bool* pbFound )
{
    NodeRecord_s* psFolder = UVFSFileNode;

    uint32_t uBufferSize = 1000;
    uint8_t* puBuffer = malloc(uBufferSize*2);
    if ( puBuffer == NULL )
    {
        return;
    }
    memset(puBuffer, 0xff, uBufferSize*2);

    uint64_t uCookie = 0;
    uint64_t uVerifier = UVFS_DIRCOOKIE_VERIFIER_INITIAL;
    bool bConRead = true;

    uint32_t uDirsCounter   = 0;
    uint32_t uFilesCounter  = 0;
    uint32_t uLinksCounter  = 0;
    size_t   outLen = 0;

    int iReadDirERR = 0;
    do {

        uint32_t uBufCurOffset  = 0;

        memset(puBuffer, 0, uBufferSize);

        iReadDirERR = MSDOS_fsOps.fsops_readdir (psFolder, puBuffer, uBufferSize, uCookie, &outLen, &uVerifier);
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
                        printf("found dir: ID: %llu, in offset [%u] \n",psNewDirListEntry->de_fileid,uBufCurOffset);
                        uDirsCounter++;
                    }
                        break;
                    case UVFS_FA_TYPE_FILE:
                    {
                        printf("found file: ID: %llu, in offset [%u] \n",psNewDirListEntry->de_fileid,uBufCurOffset);
                        uFilesCounter++;
                    }
                        break;
                    case UVFS_FA_TYPE_SYMLINK:
                    {
                        printf("found link: ID: %llu, in offset [%u] \n",psNewDirListEntry->de_fileid,uBufCurOffset);
                        uLinksCounter++;
                    }
                        break;

                    default:
                        printf("Found Unkown file type %d, Exiting\n", psNewDirListEntry->de_filetype);
                        bEndOfDirectoryList = true;
                        bConRead = false;
                        break;
                }

                uBufCurOffset += UVFS_DIRENTRY_RECLEN(strlen(psNewDirListEntry->de_name));
            }
        }
    } while( bConRead && (iReadDirERR != UVFS_READDIR_EOF_REACHED) );

    //    printf("Amount of files %d \n", uFilesCounter);
    //    printf("Amount of dirs %d \n",  uDirsCounter);
    //    printf("Amount of links %d \n", uLinksCounter);

    if ( puBuffer ) free( puBuffer );
}

static void
ReadDirAttr( UVFSFileNode UVFSFileNode)
{
    NodeRecord_s* psFolder = UVFSFileNode;

    uint32_t uBufferSize = 1000;
    uint8_t* puBuffer = malloc(uBufferSize);
    if ( puBuffer == NULL )
    {
        return;
    }

    uint64_t uCookie = 0;
    uint64_t uVerifier = UVFS_DIRCOOKIE_VERIFIER_INITIAL;
    bool bConRead = true;
    size_t   outLen = 0;
    int iReadDirERR = 0;
    do {

        uint32_t uBufCurOffset  = 0;

        memset(puBuffer, 0, uBufferSize);

        iReadDirERR = MSDOS_fsOps.fsops_readdirattr (psFolder, puBuffer, uBufferSize, uCookie, &outLen, &uVerifier);
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

                printf("FileName = [%s],  FileID = [%llu], FileSize = [%llu].\n", UVFS_DIRENTRYATTR_NAMEPTR(psNewDirListEntry), psNewDirListEntry->dea_attrs.fa_fileid, psNewDirListEntry->dea_attrs.fa_size);

                uBufCurOffset += UVFS_DIRENTRYATTR_RECLEN(psNewDirListEntry, strlen(UVFS_DIRENTRYATTR_NAMEPTR(psNewDirListEntry)));
            }
        }
    } while( bConRead && (iReadDirERR != UVFS_READDIR_EOF_REACHED) );

    if ( puBuffer )
        free( puBuffer );
}

static int ScanDir(UVFSFileNode UVFSFolderNode, char** contains_str, char** end_with_str, struct timespec mTime)
{
    int err = 0;

    scandir_matching_request_t sMatchingCriteria;
    UVFSFileAttributes smr_attribute_filter;
    scandir_matching_reply_t sMatchingResult;
    void* pvAttrBuf = malloc(sizeof(UVFSDirEntryAttr) + FAT_MAX_FILENAME_UTF8*sizeof(char));

    memset(&sMatchingCriteria, 0, sizeof(sMatchingCriteria));
    memset(&smr_attribute_filter, 0, sizeof(smr_attribute_filter));
    memset(&sMatchingResult, 0, sizeof(sMatchingResult));
    sMatchingResult.smr_entry = pvAttrBuf;

    sMatchingCriteria.smr_filename_contains = (contains_str == NULL) ? NULL : contains_str;
    sMatchingCriteria.smr_filename_ends_with = (end_with_str == NULL) ? NULL : end_with_str;
    sMatchingCriteria.smr_attribute_filter = &smr_attribute_filter;

    if (mTime.tv_nsec != 0 || mTime.tv_sec != 0 )
    {
        sMatchingCriteria.smr_attribute_filter->fa_validmask |= UVFS_FA_VALID_MTIME;
        sMatchingCriteria.smr_attribute_filter->fa_mtime = mTime;
    }

    bool bConRead = true;

    do
    {
        err = MSDOS_fsOps.fsops_scandir (UVFSFolderNode, &sMatchingCriteria, &sMatchingResult);
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
    UVFS_FSATTR_CAPS_INTERFACES
};

#define ARR_LEN(arr) ((sizeof(arr))/(sizeof(arr[0])))

static int
GetFSAttr( UVFSFileNode Node )
{
    int iErr        = 0;
    size_t uLen     = 512;
    size_t uRetLen  = 0;
    UVFSFSAttributeValue* psAttrVal = (UVFSFSAttributeValue*)malloc(uLen);
    assert( psAttrVal );

    for ( uint32_t uIdx=0; uIdx<ARR_LEN(gpcFSAttrs); uIdx++ )
    {
        memset( psAttrVal, 0, uLen );

        iErr = MSDOS_fsOps.fsops_getfsattr( Node, gpcFSAttrs[uIdx], psAttrVal, uLen, &uRetLen );
        if ( iErr != 0 )
        {
            printf( "fsops_getfsattr attr = %s return with error code [%d]\n", gpcFSAttrs[uIdx], iErr );
            continue;
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

    free(psAttrVal);
    return (iErr);
}

static int Rename(UVFSFileNode fromDirNode, UVFSFileNode fromNode, const char *fromName, UVFSFileNode toDirNode, UVFSFileNode toNode, const char *toName)
{
    int error =0;
    error = MSDOS_fsOps.fsops_rename(fromDirNode, fromNode, fromName, toDirNode, toNode, toName, 0);
    return error;
}

static int CreateNewFolder(UVFSFileNode ParentNode,UVFSFileNode* NewDirNode,char* NewDirName)
{
    int error =0;

    UVFSFileAttributes attrs;
    memset(&attrs,0,sizeof(UVFSFileAttributes));
    attrs.fa_type = UVFS_FA_TYPE_DIR;

    error = MSDOS_fsOps.fsops_mkdir(ParentNode, NewDirName, &attrs, NewDirNode);

    return error;
}

static int RemoveFolder(UVFSFileNode ParentNode,char* DirNameToRemove)
{
    int error =0;

    error = MSDOS_fsOps.fsops_rmdir(ParentNode, DirNameToRemove);;

    return error;
}

static int CreateNewFile(UVFSFileNode ParentNode,UVFSFileNode* NewFileNode,char* NewFileName,uint64_t size)
{
    int error =0;
    UVFSFileAttributes attrs;

    memset(&attrs, 0, sizeof(attrs));

    attrs.fa_validmask = UVFS_FA_VALID_MODE | UVFS_FA_VALID_SIZE;

    attrs.fa_mode = UVFS_FA_MODE_RWX;
    attrs.fa_size = size;

    error = MSDOS_fsOps.fsops_create(ParentNode, NewFileName, &attrs, NewFileNode);

    return error;
}

static int RemoveFile(UVFSFileNode ParentNode,char* FileNameToRemove)
{
    int error =0;

    error = MSDOS_fsOps.fsops_remove(ParentNode, FileNameToRemove, NULL);

    return error;
}

static int Lookup(UVFSFileNode ParentNode,UVFSFileNode* NewFileNode,char* FileName)
{
    int error =0;
    error = MSDOS_fsOps.fsops_lookup(ParentNode, FileName, NewFileNode);
    return error;
}

static int GetAttrAndCompare(UVFSFileNode FileNode,UVFSFileAttributes* sInAttrs)
{
    int error =0;
    UVFSFileAttributes sOutAttrs;
    error = MSDOS_fsOps.fsops_getattr(FileNode, &sOutAttrs);
    if (error)
    {
        printf("Failed in get attr with err [%d]\n",error);
        return error;
    }
    if (sInAttrs->fa_validmask & UVFS_FA_VALID_SIZE)
        if (sOutAttrs.fa_size != sInAttrs->fa_size)
            error = 1;

    if (sInAttrs->fa_validmask & UVFS_FA_VALID_MODE)
        if (sOutAttrs.fa_mode != sInAttrs->fa_mode)
            error = 1;

    if (sInAttrs->fa_validmask & UVFS_FA_VALID_ATIME)
        if (CMP_TIMES(sOutAttrs.fa_atime,sInAttrs->fa_atime))
            error = 1;

    // Since there isn't an actual ctime in msdos
    // Check that mtime and ctime have the same value
    if (sInAttrs->fa_validmask & UVFS_FA_VALID_MTIME || sInAttrs->fa_validmask & UVFS_FA_VALID_CTIME )
        if (CMP_TIMES(sOutAttrs.fa_mtime,sInAttrs->fa_mtime) || CMP_TIMES(sOutAttrs.fa_ctime,sInAttrs->fa_mtime))
            error = 1;

    if (sInAttrs->fa_validmask & UVFS_FA_VALID_BIRTHTIME)
        if (CMP_TIMES(sOutAttrs.fa_birthtime,sInAttrs->fa_birthtime))
            error = 1;

    if (error) printf("Failed in compare attr\n");
    return error;
}

static int SetAttrChangeSize(UVFSFileNode FileNode,uint64_t uNewSize)
{
    int error =0;
    UVFSFileAttributes sInAttrs;
    UVFSFileAttributes sOutAttrs;
    memset(&sInAttrs,0,sizeof(UVFSFileAttributes));
    sInAttrs.fa_validmask |= UVFS_FA_VALID_SIZE;
    sInAttrs.fa_size = uNewSize;

    error =  MSDOS_fsOps.fsops_setattr( FileNode, &sInAttrs , &sOutAttrs );
    if (error)
        return error;

    error = GetAttrAndCompare(FileNode,&sInAttrs);

    return error;
}

static int ReadToBuffer(UVFSFileNode FileToReadNode,uint32_t uOffset, size_t* actuallyRead,void* pvExternalBuffer, uint32_t uExternalBufferSize)
{
    int error =0;

    size_t ExternalBufferOffset = 0;
    uint32_t uBufferSize =1*1024*1024;
    void* pvFileBuffer = malloc(uBufferSize);
    long long int lliTotalTime = 0;

    if (pvFileBuffer != NULL)
    {
        do
        {
            uint32_t uBytesToRead = 1+ rand() % uBufferSize;

            long long int lliReadStartTime = timestamp();

            error = MSDOS_fsOps.fsops_read(FileToReadNode,uOffset,uBytesToRead,pvFileBuffer,actuallyRead);

            long long int lliReadEndTime = timestamp();
            lliTotalTime += lliReadEndTime - lliReadStartTime;

            uOffset += *actuallyRead;

            //If need to write into external buffer and there is still space
            if (pvExternalBuffer != NULL && ExternalBufferOffset < uExternalBufferSize)
            {
                size_t uSizeToCopy = *actuallyRead;
                if (ExternalBufferOffset + *actuallyRead > uExternalBufferSize)
                {
                    uSizeToCopy = uExternalBufferSize - ExternalBufferOffset;
                }
                memcpy(pvExternalBuffer + ExternalBufferOffset,pvFileBuffer,uSizeToCopy);
                ExternalBufferOffset += uSizeToCopy;
            }

        }while (!error && *actuallyRead!= 0);
        free(pvFileBuffer);
    }
    else
    {
        printf("Failed to allocate read buffer\n");
        error = ENOMEM;
    }

    printf("Read total_time %lld [usec], total size: %u [Bytes]\n", lliTotalTime,uOffset);

    return error;
}

static int ReadLinkToBuffer(UVFSFileNode LinkdNode,void* pvExternalBuffer, uint32_t uExternalBufferSize,size_t* actuallyRead)
{
    int error =0;

    uint32_t uBufferSize = 4*1024;
    void* pvLinkBuffer = malloc(uBufferSize);

    UVFSFileAttributes sOutAttrs;

    if (pvLinkBuffer != NULL)
    {
        error = MSDOS_fsOps.fsops_readlink(LinkdNode,pvLinkBuffer,uBufferSize,actuallyRead,&sOutAttrs);

        //If need to write into external buffer and there is still space
        if (pvExternalBuffer != NULL)
        {
            size_t uSizeToCopy = *actuallyRead;
            if ( *actuallyRead > uExternalBufferSize)
            {
                uSizeToCopy = uExternalBufferSize;
            }
            memcpy(pvExternalBuffer,pvLinkBuffer,uSizeToCopy);
        }

        free(pvLinkBuffer);
    }
    else
    {
        printf("Failed to allocate link buffer\n");
        error = ENOMEM;
    }

    return error;
}

static int CreateLink(UVFSFileNode ParentNode,UVFSFileNode* NewLinkNode,char* LinkName,char* LinkContent)
{
    int error =0;
    UVFSFileAttributes attrs;

    memset(&attrs, 0, sizeof(attrs));

    attrs.fa_validmask = UVFS_FA_VALID_MODE;
    attrs.fa_mode = UVFS_FA_MODE_RWX;

    error = MSDOS_fsOps.fsops_symlink(ParentNode, LinkName, LinkContent, &attrs, NewLinkNode);

    return error;
}

static int WriteFromBuffer(UVFSFileNode FileToWriteNode,uint32_t uOffset, size_t* uActuallyWritten,void* pvExternalBuffer, uint32_t uExternalBufferSize)
{
    int iError =0;
    uint32_t uExternalBufferOffset = 0;
    long long int lliTotalTime = 0;
    uint32_t uBufferSize = 1*1024*1024;
    void* pvFileBuffer = malloc(uBufferSize);

    if ( (pvFileBuffer != NULL) && (pvExternalBuffer!= NULL) )
    {
        do
        {
            //Check the size of current write length
            uint32_t BytesToWrite = 1 + rand() % uBufferSize;
            if ((uExternalBufferSize - uExternalBufferOffset)< uBufferSize)
            {
                BytesToWrite = uExternalBufferSize - uExternalBufferOffset;
            }

            //Create buffer
            memcpy(pvFileBuffer,pvExternalBuffer + uExternalBufferOffset,BytesToWrite);

            long long int lliWriteStartTime = timestamp();

            iError = MSDOS_fsOps.fsops_write(FileToWriteNode,uOffset,BytesToWrite,pvFileBuffer, uActuallyWritten);

            long long int lliWriteEndTime = timestamp();
            lliTotalTime += lliWriteEndTime - lliWriteStartTime;

            if (*uActuallyWritten < BytesToWrite)
            {
                iError = ENOSPC;
                printf("Failed to write all data into device, Error [%d], offset [%d]\n",ENOSPC,uOffset);
                break;
            }

            //Set Offsets
            uOffset += *uActuallyWritten;
            uExternalBufferOffset += *uActuallyWritten;

        }while ( (!iError) && (*uActuallyWritten!= 0) && (uExternalBufferOffset < uExternalBufferSize) );
        free(pvFileBuffer);
    }
    else
    {
        printf("Failed to allocate read buffer\n");
        iError = ENOMEM;
    }

    printf("Write total_time %lld [usec], total size: %u [Bytes]\n", lliTotalTime,uOffset);

    return iError;
}

typedef struct
{
    uint32_t uThreadId;
    uint32_t uTotalNumberOfThreads;
    uint32_t uFileSize;
    uint32_t uBufferSize;

    union {
        UVFSFileNode* ppvFileToWriteNode;
        UVFSFileNode* ppvParentFolder;
    }ThreadInputNode;

} ThreadInput_s;

static void*
MultiThreadSingleFile(void *arg)
{

    int iError = 0;
    ThreadInput_s* psThreadInput = (ThreadInput_s*) arg;

    uint32_t uChunkSize = psThreadInput->uFileSize / psThreadInput->uTotalNumberOfThreads;
    uint32_t uStartOffset = psThreadInput->uThreadId*uChunkSize;

    void* pvBuf = malloc(uChunkSize);
    if (pvBuf == NULL) return (void*)ENOMEM;

    void* pvBufRead = malloc(uChunkSize);
    if (pvBufRead == NULL) return (void*)ENOMEM;

    for ( int64_t uIdx=0; uIdx < (uChunkSize/sizeof(int64_t)); uIdx++)
    {
        ((int64_t*)pvBuf)[uIdx] = uStartOffset + uIdx;
        ((int64_t*)pvBufRead)[uIdx] = 0;
    }

    size_t uActuallyWritten;
    iError = WriteFromBuffer(*psThreadInput->ThreadInputNode.ppvFileToWriteNode,uStartOffset, &uActuallyWritten,pvBuf, uChunkSize);
    if (iError) goto exit;

    size_t uActuallyRead;
    iError =  ReadToBuffer(*psThreadInput->ThreadInputNode.ppvFileToWriteNode,uStartOffset, &uActuallyRead,pvBufRead, uChunkSize);
    if (iError) goto exit;

    //Compare read to write
    for ( uint32_t uIdx=0; uIdx<uChunkSize/16; uIdx++)
    {
        if ( ((int64_t*)pvBuf)[uIdx] != ((int64_t*)pvBufRead)[uIdx] )
        {
            printf("Failed in compare read to write. Index [%d]\n",uIdx);
            iError = 1;
            break;
        }
    }

exit:
    free(pvBuf);
    free(pvBufRead);

    return (void*) (size_t) iError;
}

static void*
MultiThreadMultiFiles(void *arg)
{
    int iError = 0;
    ThreadInput_s* psThreadInput = (ThreadInput_s*) arg;

    char pcName[10] = {0};
    sprintf(pcName, "file%d", psThreadInput->uThreadId);
    UVFSFileNode pvFileNode= NULL;
    iError = CreateNewFile(*psThreadInput->ThreadInputNode.ppvParentFolder,&pvFileNode,pcName,0);
    if (iError) return (void *) (size_t) iError;;

    uint32_t uChunkSize = psThreadInput->uBufferSize;
    uint32_t uStartOffset = 0;

    void* pvBuf = malloc(uChunkSize);
    if (pvBuf == NULL) return (void*)ENOMEM;

    void* pvBufRead = malloc(uChunkSize);
    if (pvBufRead == NULL) return (void*)ENOMEM;

    for ( int64_t uIdx=0; uIdx < (uChunkSize/sizeof(int64_t)); uIdx++)
    {
        ((int64_t*)pvBuf)[uIdx] = uStartOffset + uIdx;
        ((int64_t*)pvBufRead)[uIdx] = 0;
    }

    size_t uActuallyWritten;
    iError = WriteFromBuffer(pvFileNode,uStartOffset, &uActuallyWritten,pvBuf, uChunkSize);
    if (iError) goto exit;

    size_t uActuallyRead;
    iError =  ReadToBuffer(pvFileNode,uStartOffset, &uActuallyRead,pvBufRead, uChunkSize);
    if (iError) goto exit;

    for ( uint32_t uIdx=0; uIdx<uChunkSize/16; uIdx++)
    {
        if ( ((int64_t*)pvBuf)[uIdx] != ((int64_t*)pvBufRead)[uIdx] )
        {
            printf("Failed in comare read to write. Index [%d]\n",uIdx);
            iError = 1;
            break;
        }
    }
    if (iError) goto exit;

    iError = CloseFile(pvFileNode);
    if (iError) goto exit;

    // Remove
    iError =  RemoveFile(*psThreadInput->ThreadInputNode.ppvParentFolder, pcName);

exit:
    free(pvBuf);
    free(pvBufRead);
    return (void *) (size_t) iError;
}

static void
TestNlink( UVFSFileNode RootNode )
{
    UVFSFileAttributes sOutAttrs;
    UVFSFileNode psParentNode;
    UVFSFileNode psChildNode;

    assert( CreateNewFolder(RootNode, &psParentNode, "TestNlinkParentDirectory") == 0 );
    assert( CreateNewFile(psParentNode, &psChildNode, "TestNlinkFile0", 0) == 0 );
    assert( CloseFile(psChildNode) == 0 );
    assert( CreateNewFile(psParentNode, &psChildNode, "TestNlinkFile1", 0) == 0 );
    assert( CloseFile(psChildNode) == 0 );
    assert( CreateNewFile(psParentNode, &psChildNode, "TestNlinkFile2", 0) == 0 );
    assert( CloseFile(psChildNode) == 0 );
    assert( CreateLink(psParentNode, &psChildNode, "TestNlinkLink0", "/I/am/just/a/link") == 0 );
    assert( CloseFile(psChildNode) == 0 );
    assert( CreateLink(psParentNode, &psChildNode, "TestNlinkLink1", "/I/am/just/a/link") == 0 );
    assert( CloseFile(psChildNode) == 0 );
    assert( CreateLink(psParentNode, &psChildNode, "TestNlinkLink2", "/I/am/just/a/link") == 0 );
    assert( CloseFile(psChildNode) == 0 );
    assert( CreateNewFolder(psParentNode, &psChildNode, "TestNlinkDir0") == 0 );
    assert( CloseFile(psChildNode) == 0 );
    assert( CreateNewFolder(psParentNode, &psChildNode, "TestNlinkDir1") == 0 );
    assert( CloseFile(psChildNode) == 0 );
    assert( CreateNewFolder(psParentNode, &psChildNode, "TestNlinkDir2") == 0 );
    assert( CloseFile(psChildNode) == 0 );

    assert( MSDOS_fsOps.fsops_getattr(psParentNode, &sOutAttrs) == 0 );
#ifdef MSDOS_NLINK_IS_CHILD_COUNT
    assert( sOutAttrs.fa_nlink == 9+2 );
#else
    assert( sOutAttrs.fa_nlink == 1 );
#endif

    assert( RemoveFile(psParentNode, "TestNlinkFile2") == 0 );
    assert( RemoveFile(psParentNode, "TestNlinkLink2") == 0 );
    assert( RemoveFolder(psParentNode, "TestNlinkDir2") == 0 );

    assert( MSDOS_fsOps.fsops_getattr(psParentNode, &sOutAttrs) == 0 );
#ifdef MSDOS_NLINK_IS_CHILD_COUNT
    assert( sOutAttrs.fa_nlink == 6+2 );
#else
    assert( sOutAttrs.fa_nlink == 1 );
#endif

    assert( RemoveFile(psParentNode, "TestNlinkFile0") == 0 );
    assert( RemoveFile(psParentNode, "TestNlinkLink0") == 0 );
    assert( RemoveFolder(psParentNode, "TestNlinkDir0") == 0 );
    assert( RemoveFile(psParentNode, "TestNlinkFile1") == 0 );
    assert( RemoveFile(psParentNode, "TestNlinkLink1") == 0 );
    assert( RemoveFolder(psParentNode, "TestNlinkDir1") == 0 );

    assert( MSDOS_fsOps.fsops_getattr(psParentNode, &sOutAttrs) == 0 );
#ifdef MSDOS_NLINK_IS_CHILD_COUNT
    assert( sOutAttrs.fa_nlink == 2 );
#else
    assert( sOutAttrs.fa_nlink == 1 );
#endif

    assert( RemoveFolder(RootNode, "TestNlinkParentDirectory") == 0 );
    assert( CloseFile(psParentNode) == 0 );
}

pthread_t psTest1_Threads[TEST_1_THREAD_COUNT] = {0};
int piTest1_Results[TEST_1_THREAD_COUNT] = {0};
ThreadInput_s sTest1_ThreadInput[TEST_1_THREAD_COUNT];

pthread_t psTest2_Threads[TEST_2_THREAD_COUNT] = {0};
int piTest2_Results[TEST_2_THREAD_COUNT] = {0};
ThreadInput_s sTest2_ThreadInput[TEST_2_THREAD_COUNT];

/* --------------------------------------------------------------------------------------------- */

/* --------------------------------------------------------------------------------------------- */
int main( int argc, const char * argv[] )
{
    /*Need to create a script that will:
     1) create an image - hdiutil create -size 1M -fs MS-DOS -volname test2 -type SPARSE usbstorage2
     2) attach nomount  - hdiutil attach -nomount usbStorage_tests_image.dmg
     3) find the name   - Need to call to the latest /dev/rdisk.
     */

    if ( argc != 2 )
    {
        printf("Usage : livefiles_msdos_tester < dev-path >\n");
        exit(EINVAL);
    }

    UVFSScanVolsRequest sScanVolsReq = {0};
    UVFSScanVolsReply sScanVolsReply = {0};
    int err = 0;
    int fd = open( argv[1] , O_RDWR );
    int uCycleCounter = 0;
    UVFSFileNode RootNode = NULL;
    if(fd < 0)
    {
        printf("Failed to open [%s]\n", argv[1]);
        return EBADF;
    }

    do
    {
        err = MSDOS_fsOps.fsops_init();
        printf("Init err [%d]\n",err);
        if (err) break;

        err = MSDOS_fsOps.fsops_taste(fd);
        printf("Taste err [%d]\n",err);
        if (err) break;

        err = MSDOS_fsOps.fsops_scanvols(fd, &sScanVolsReq, &sScanVolsReply);
        printf("ScanVols err [%d]\n",err);
        if (err) break;

        err = MSDOS_fsOps.fsops_mount(fd, sScanVolsReply.sr_volid, 0, NULL, &RootNode);
        printf("Mount err [%d]\n",err);
        if (err) break;

        ReadDirAttr(RootNode);

        //Check nlink.
        TestNlink(RootNode);

        // --------------------------Create & mkdir----------------------------------
        // Create Dir:  D1 in Root
        UVFSFileNode D1_Node = NULL;
        err = CreateNewFolder(RootNode,&D1_Node,"D1");
        printf("CreateNewFolder err [%d]\n",err);
        if (err) break;

        UVFSFileNode D11_Node = NULL;
        err = CreateNewFolder(RootNode,&D11_Node,(char*)"ÖÖ");
        printf("CreateNewFolder ÖÖ err [%d]\n",err);
        if (err) break;

        // Create File Validation-MirrorAckColoradoBulldog (size 0) in D1
        UVFSFileNode F12_Node= NULL;
        err = CreateNewFile(D1_Node,&F12_Node,(char*)"Validation-MirrorAckColoradoBulldog",0);
        printf("CreateNewFile Validation-MirrorAckColoradoBulldog err [%d]\n",err);
        if (err) break;
        
        // Create File validation-mirrorackcoloradobulldog (size 0) in D1 - should fail
        UVFSFileNode F13_Node= NULL;
        err = CreateNewFile(D1_Node,&F13_Node,(char*)"validation-mirrorackcoloradobulldog",0);
        printf("CreateNewFile validation-mirrorackcoloradobulldog err [%d]\n",err);
        if (err != EEXIST) break;
        
        // Reclaim Validation-MirrorAckColoradoBulldog
        err = CloseFile(F12_Node);
        printf("Reclaim Validation-MirrorAckColoradoBulldog err [%d]\n",err);
        if (err) break;
        
        //Remove validation-mirrorackcoloradobulldog - should remove Validation-MirrorAckColoradoBulldog
        err = RemoveFile(D1_Node,(char*)"validation-mirrorackcoloradobulldog");
        printf("RemoveFile validation-mirrorackcoloradobulldog from Root err [%d]\n",err);
        if (err) break;
        
        // Reclaim and Lookup of D1
        err = CloseFile(D1_Node);
        printf("Reclaim D1 err [%d]\n",err);
        if (err) break;

        err = CloseFile(D11_Node);
        printf("Reclaim ÖÖ err [%d]\n",err);
        if (err) break;

        err = Lookup(RootNode,&D1_Node,"D1");
        printf("Lookup err [%d]\n",err);
        if (err) break;

        // Create File F1 (size 0) in D1
        UVFSFileNode F1_Node= NULL;
        err = CreateNewFile(D1_Node,&F1_Node,"F1",0);
        printf("CreateNewFile err [%d]\n",err);
        if (err) break;

        err = GetFSAttr(F1_Node);
        printf("GetFSAttr err [%d]\n",err);
        if (err) break;

        // Set Attr, make F1 larger
        err = SetAttrChangeSize(F1_Node,10*1024);
        printf("SetAttrChangeSize to 10K err [%d]\n",err);
        if (err) break;

        // Set Attr, make F1 smaller
        err = SetAttrChangeSize(F1_Node,0*1024);
        printf("SetAttrChangeSize to 0 err [%d]\n",err);
        if (err) break;

        // -----------------------------Write/Read File-----------------------------------
        // Test1: 2 threads writing and reading to/from the same file

        for ( uint64_t uIdx=0; uIdx<TEST_1_THREAD_COUNT; uIdx++ )
        {
            memset(&sTest1_ThreadInput[uIdx],0,sizeof(ThreadInput_s));
            sTest1_ThreadInput[uIdx].uBufferSize = 1024*1024;
            sTest1_ThreadInput[uIdx].uFileSize = 100*1024*1024;
            sTest1_ThreadInput[uIdx].uTotalNumberOfThreads = TEST_1_THREAD_COUNT;
            sTest1_ThreadInput[uIdx].uThreadId = (uint32_t) uIdx;
            sTest1_ThreadInput[uIdx].ThreadInputNode.ppvFileToWriteNode = &F1_Node;

            pthread_create( &psTest1_Threads[uIdx], NULL, (void*)MultiThreadSingleFile, (void*)&sTest1_ThreadInput[uIdx]);
        }

        for ( uint32_t uIdx=0; uIdx<TEST_1_THREAD_COUNT; uIdx++ )
        {
            pthread_join( psTest1_Threads[uIdx], (void*) &piTest1_Results[uIdx] );

            if (piTest1_Results[uIdx])
            {
                err = piTest1_Results[uIdx];
                printf("Thread [%d] Failed in Test 1 with error [%d]\n",uIdx,err);
            }
        }

        printf("Test 1 finished with error [%d]\n",err);
        if (err) break;

        // Test2: 4 threads writing and reading to/from different files

        for ( uint64_t uIdx=0; uIdx<TEST_2_THREAD_COUNT; uIdx++ )
        {
            memset(&sTest2_ThreadInput[uIdx],0,sizeof(ThreadInput_s));
            sTest2_ThreadInput[uIdx].uBufferSize = 1024*1024;
            sTest2_ThreadInput[uIdx].uFileSize = 10*1024*1024;
            sTest2_ThreadInput[uIdx].uTotalNumberOfThreads = TEST_2_THREAD_COUNT;
            sTest2_ThreadInput[uIdx].uThreadId = (uint32_t) uIdx;
            sTest2_ThreadInput[uIdx].ThreadInputNode.ppvParentFolder = &D1_Node;

            pthread_create( &psTest2_Threads[uIdx], NULL, (void*)MultiThreadMultiFiles, (void*)&sTest2_ThreadInput[uIdx]);
        }

        for ( uint32_t uIdx=0; uIdx<TEST_2_THREAD_COUNT; uIdx++ )
        {
            pthread_join( psTest2_Threads[uIdx], (void*) &piTest2_Results[uIdx] );
            if (piTest2_Results[uIdx])
            {
                err = piTest2_Results[uIdx];
                printf("Thread [%d] Failed in Test 2 with error [%d]\n",uIdx,err);
            }
        }

        printf("Test 2 finished with error [%d]\n",err);
        if (err) break;

        // --------------------------------Read Dir-----------------------------------
        ReadDirAttr(D1_Node);

        bool bFound = false;
        read_directory_and_search_for_name( D1_Node, "F1", &bFound );

        if (!bFound)
        {
            printf("Dir read failed, F1 wasn't found in D1");
            break;
        }

        err = CloseFile(F1_Node);
        printf("Reclaim F1 err [%d]\n",err);
        if (err) break;

        // Rename F1 to F2 and move from D1 to Root
        err =  Rename(D1_Node, NULL, "F1", RootNode, NULL, "F2");
        printf("Rename F1 to F2 and move from D1 to Root err [%d]\n",err);
        if (err) break;

        err = CloseFile(D1_Node);
        printf("Reclaim D1 err [%d]\n",err);
        if (err) break;

        // Rename D1 to D2
        err =  Rename(RootNode, NULL, "D1", RootNode, NULL, "D2");
        printf("Rename D1 to D2 err [%d]\n",err);
        if (err) break;


        // ---------------------------------- Link ----------------------------------------

        UVFSFileNode L1_Node= NULL;
        char pcLinkContent[255] = "/dev/look/for/A/Link/In/Filder/?/Found/One";

        err =  CreateLink(RootNode ,&L1_Node,"L1",pcLinkContent);
        printf("CreateLink L1 err [%d]\n",err);
        if (err) break;

        void* pvExternalBufferForLink = malloc(4*1024);
        if (pvExternalBufferForLink == NULL)
        {
            err = ENOMEM;
            printf("Failed to allocate buffer to ReadLinkToBuffer L1 \n");
            break;
        }
        size_t actuallyRead = 0;
        err =  ReadLinkToBuffer(L1_Node,pvExternalBufferForLink, 4*1024, &actuallyRead);
        printf("ReadLinkToBuffer L1 err [%d]\n",err);
        if (err) break;

        if (memcmp(pcLinkContent,pvExternalBufferForLink,actuallyRead))
        {
            printf("Failed memcmp between symlink to readlink content\n");
            err = 1;
            break;
        }

        if (pvExternalBufferForLink!= NULL)
            free(pvExternalBufferForLink);

        // Rename L1 to L🤪2
        err =  Rename(RootNode, NULL, "L1", RootNode, NULL, "L🤪2");
        printf("Rename L1 to L🤪2 err [%d]\n",err);
        if (err) break;

        UVFSFileAttributes sOutAttrs;
        err = MSDOS_fsOps.fsops_getattr(L1_Node, &sOutAttrs);
        printf("fsops_getattr L2 err [%d]\n",err);
        if (err) break;

        struct timespec mTime = {0};
        mTime.tv_nsec = sOutAttrs.fa_mtime.tv_nsec;
        mTime.tv_sec = sOutAttrs.fa_mtime.tv_sec;

        err = CloseFile(L1_Node);
        printf("Reclaim L1 err [%d]\n",err);
        if (err) break;

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

        err =  ScanDir(RootNode, (char**) &name_contains_array, (char**) &name_end_with_array, mTime);
        printf("ScanDir of root err [%d]\n",err);
        if (err) break;

        // --------------------------------------------------------------------------------
        // Remove all files that left
        // --------------------------------------------------------------------------------

        // Remove ÖÖ
        err =  RemoveFolder(RootNode,(char*)"ÖÖ");
        printf("Remove Folder ÖÖ from Root err [%d]\n",err);
        if (err) break;

        // Remove D2
        err =  RemoveFolder(RootNode,(char*)"D2");
        printf("Remove Folder D2 from Root err [%d]\n",err);

        // Remove L🤪2
        err = RemoveFile(RootNode,"L🤪2");
        printf("Remove Link L🤪2 err [%d]\n",err);
        if (err) break;

        // Remove F2
        err = RemoveFile(RootNode,(char*)"F2");
        printf("RemoveFile F2 from Root err [%d]\n",err);
        if (err) break;

        // --------------------------------------------------------------------------------

        err = GetFSAttr(RootNode);
        printf("GetFSAttr err [%d]\n",err);
        if (err) break;

        err = MSDOS_fsOps.fsops_sync(RootNode);
        printf("Sync err [%d]\n",err);
        if (err) break;

        err = MSDOS_fsOps.fsops_unmount(RootNode, UVFSUnmountHintNone);
        printf("Unmount err [%d]\n",err);
        if (err) break;

        MSDOS_fsOps.fsops_fini();

        uCycleCounter++;
    }while(uCycleCounter < TEST_CYCLE_COUNT);

    return err;
}
