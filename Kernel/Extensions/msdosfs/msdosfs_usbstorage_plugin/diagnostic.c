/* Copyright © 2017 Apple Inc. All rights reserved.
 *
 *  diagnostic.c
 *  livefiles_msdos
 *
 *  Created by Or Haimovich on 31/1/18.
 *
 */

#ifdef DIAGNOSTIC
#include "diagnostic.h"
#include "Logger.h"
#include "Conv.h"

#define HASH_MASK (0x00000000FF)
#define CALCULATE_HASH(A) (A & HASH_MASK)

// -------------------------------------- Function Decleration -----------------------------------
static bool DIAGNOSTIC_AreNamesEquals(struct unistr255* psName1,struct unistr255* psName2);
static void DIAGNOSTIC_RemoveAllCacheElements(FileSystemRecord_s* psFSRecord, uint32_t uFileIDHash);
static void DIAGNOSTIC_RemoveCacheElement(FileSystemRecord_s* psFSRecord, FRHashElement_s* psFileHashElementToRemove);
static bool DIAGNOSTIC_IsRecordExist(FileSystemRecord_s* psFSRecord, uint64_t uParentStartCluster, struct unistr255* psName, FRHashElement_s** ppsReturnHashElement);
// -------------------------------------- Function Implementation -----------------------------------

static bool DIAGNOSTIC_AreNamesEquals(struct unistr255* psName1,struct unistr255* psName2)
{
    /* Convert to lower case */
    CONV_Unistr255ToLowerCase( psName1 );
    CONV_Unistr255ToLowerCase( psName2 );

    return ((psName1->length == psName2->length) && !memcmp(psName1->chars, psName2->chars, psName2->length*sizeof(psName2->chars[0])));
}

//Assumption: Under Lock
static bool DIAGNOSTIC_IsRecordExist(FileSystemRecord_s* psFSRecord, uint64_t uParentStartCluster, struct unistr255* psName, FRHashElement_s** ppsReturnHashElement)
{
    uint64_t uFileIDHash = CALCULATE_HASH(uParentStartCluster);
    FRHashElement_s* psFileHashElement = psFSRecord->sDiagnosticDB.psDiagnosticCache[uFileIDHash];
    bool bFound = false;

    while ( (!bFound) && (psFileHashElement != NULL))
    {
        if ((psFileHashElement->uParentStartCluster == uParentStartCluster)
            && DIAGNOSTIC_AreNamesEquals(&psFileHashElement->sName,psName))
        {
            bFound = true;
            if (ppsReturnHashElement!= NULL)
                *ppsReturnHashElement = psFileHashElement;
        }

        psFileHashElement = psFileHashElement->psNext;
    }

    return bFound;
}

int DIAGNOSTIC_InsertNewRecord(NodeRecord_s* psFileRecord, uint64_t uParentCluster, const char * pcName)
{
    int iError = 0;
    struct unistr255 sTempName;
    FileSystemRecord_s* psFSRecord = GET_FSRECORD(psFileRecord);

    /* Convert the search name to UTF-16 */
    iError = CONV_UTF8ToUnistr255((const uint8_t *)pcName, strlen(pcName), &sTempName, UTF_SFM_CONVERSIONS);
    if ( iError != 0 )
    {
        MSDOS_LOG(LEVEL_ERROR, "DIAGNOSTIC_InsertNewRecord fail to convert utf8 -> utf16 (Error =%d).\n",iError);
        goto exit;
    }

    if (psFileRecord != NULL)
    {
        psFileRecord->sRecordData.uParentCluster = uParentCluster;
        memcpy((void *) &psFileRecord->sRecordData.sName,&sTempName,sizeof(struct unistr255));
    }

    MultiReadSingleWrite_LockWrite(&psFSRecord->sDiagnosticDB.sCacheLock);
    if ( DIAGNOSTIC_IsRecordExist(psFSRecord, uParentCluster,&sTempName, NULL))
    {
        MSDOS_LOG(LEVEL_ERROR, "DIAGNOSTIC_InsertNewRecord: FileRecord with name %s in directory with uFileID %llu already exist.\n",pcName,uParentCluster);
        assert(0);
        iError = EEXIST;
        goto exit;
    }

    uint64_t uFileIDHash = CALCULATE_HASH(uParentCluster);
    FRHashElement_s* psNewFileHashElement = malloc(sizeof(FRHashElement_s));
    if (psNewFileHashElement == NULL)
    {
        MSDOS_LOG(LEVEL_ERROR, "DIAGNOSTIC_InsertNewRecord: Failed to allocate record.\n");
        iError = ENOMEM;
        goto exit;
    }

    //Initialize new Cache Element
    psNewFileHashElement->uParentStartCluster = uParentCluster;
    memcpy((void *) &psNewFileHashElement->sName,&sTempName,sizeof(struct unistr255));
    psNewFileHashElement->psNext =
    psNewFileHashElement->psPrev = NULL;

    //If this is the first one
    if (psFSRecord->sDiagnosticDB.psDiagnosticCache[uFileIDHash] == NULL)
    {
        psFSRecord->sDiagnosticDB.psDiagnosticCache[uFileIDHash] = psNewFileHashElement;
        goto exit;
    }

    FRHashElement_s* psFileHashElement = psFSRecord->sDiagnosticDB.psDiagnosticCache[uFileIDHash];
    //Find the last element in the hash
    while (psFileHashElement->psNext != NULL)
    {
        psFileHashElement = psFileHashElement->psNext;
    }

    psFileHashElement->psNext = psNewFileHashElement;
    psNewFileHashElement->psPrev = psFileHashElement;

exit:
    MultiReadSingleWrite_FreeWrite(&psFSRecord->sDiagnosticDB.sCacheLock);
    return iError;
}

//Assumption: Under Lock
static void DIAGNOSTIC_RemoveCacheElement(FileSystemRecord_s* psFSRecord, FRHashElement_s* psFileHashElementToRemove)
{
    //If this is the first one
    if (psFileHashElementToRemove->psPrev == NULL)
    {
        uint64_t uFileIDHash = CALCULATE_HASH(psFileHashElementToRemove->uParentStartCluster);
        psFSRecord->sDiagnosticDB.psDiagnosticCache[uFileIDHash] = psFileHashElementToRemove->psNext;
    }

    if (psFileHashElementToRemove->psPrev)
    {
        psFileHashElementToRemove->psPrev->psNext = psFileHashElementToRemove->psNext;
    }
    if (psFileHashElementToRemove->psNext)
    {
        psFileHashElementToRemove->psNext->psPrev = psFileHashElementToRemove->psPrev;
    }

    free(psFileHashElementToRemove);
}

int DIAGNOSTIC_RemoveRecord(FileSystemRecord_s* psFSRecord, uint64_t uParentCluster, struct unistr255* psName)
{
    int iError = 0;
    FRHashElement_s* psFileHashElementToRemove;

    MultiReadSingleWrite_LockWrite(& psFSRecord->sDiagnosticDB.sCacheLock);

    if ( !DIAGNOSTIC_IsRecordExist(psFSRecord, uParentCluster , psName, &psFileHashElementToRemove))
    {
        MSDOS_LOG(LEVEL_ERROR, "DIAGNOSTIC_RemoveRecord: Failed to find FileRecord.\n");
        assert(0);
        iError = ENOENT;
        goto exit;
    }

    DIAGNOSTIC_RemoveCacheElement(psFSRecord, psFileHashElementToRemove);

exit:
    MultiReadSingleWrite_FreeWrite(& psFSRecord->sDiagnosticDB.sCacheLock);
    return iError;
}

static void DIAGNOSTIC_RemoveAllCacheElements(FileSystemRecord_s* psFSRecord, uint32_t uFileIDHash)
{
    MultiReadSingleWrite_LockWrite(&psFSRecord->sDiagnosticDB.sCacheLock);

    FRHashElement_s* psFileHashElementToRemove = psFSRecord->sDiagnosticDB.psDiagnosticCache[uFileIDHash];
    FRHashElement_s* psNextFileHashElementToRemove = NULL;
    while (psFileHashElementToRemove != NULL)
    {
        psNextFileHashElementToRemove = psFileHashElementToRemove->psNext;
        DIAGNOSTIC_RemoveCacheElement(psFSRecord, psFileHashElementToRemove);
        psFileHashElementToRemove = psNextFileHashElementToRemove;
    }

    MultiReadSingleWrite_FreeWrite(& psFSRecord->sDiagnosticDB.sCacheLock);
}

/*
 * General routine to allocate a hash table.
 */
void DIAGNOSTIC_CacheInit(FileSystemRecord_s* psFSRecord)
{
    for (int i=0; i < DIAGNOSTIC_CACHE_SIZE; ++i)
    {
        psFSRecord->sDiagnosticDB.psDiagnosticCache[i] = NULL;
    }
    MultiReadSingleWrite_Init(&psFSRecord->sDiagnosticDB.sCacheLock);
}

/*
 * General routine to validate that all records were reclaimed.
 */
int DIAGNOSTIC_CacheDeInit(FileSystemRecord_s* psFSRecord)
{
    int iError = 0;
    for (int i=0; i < DIAGNOSTIC_CACHE_SIZE; ++i)
    {
        if (psFSRecord->sDiagnosticDB.psDiagnosticCache[i] != NULL)
        {
            //Need to free all items and return an error
            iError = ENOTEMPTY;
            DIAGNOSTIC_RemoveAllCacheElements(psFSRecord,i);
        }
    }

    MultiReadSingleWrite_DeInit(&psFSRecord->sDiagnosticDB.sCacheLock);

    return iError;
}
#endif //DIAGNOSTIC
