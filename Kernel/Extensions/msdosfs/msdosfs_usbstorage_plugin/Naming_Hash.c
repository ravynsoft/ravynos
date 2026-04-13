//
//  Naming_Hash.c
//  usbstorage_plugin
//
//  Created by Or Haimovich on 12/02/2019.
//

#include "Naming_Hash.h"
#include <errno.h>
#include <assert.h>
#include "Conv.h"
#include "DirOPS_Handler.h"

#define MAX_HASH_KEY (HASH_TABLE_SIZE)

#define MAX_HASH_VALUES (4096)
#define LOW_BOUND_HASH_VALUES (4000)

// -------------------------------------------------------------------------------------
static uint32_t hash(struct unistr255* psName);
static void ht_remove_from_list(LF_HashTable_t* psHT, LF_TableEntry_t* psTEToRemove, LF_TableEntry_t* psPrevTE, uint32_t uKey);
static void ht_remove_rand_item(LF_HashTable_t* psHT, uint32_t uStartKey);
// -------------------------------------------------------------------------------------

/* creates hash for a hashtab */
static uint32_t hash(struct unistr255* psName)
{
    uint64_t hashval = 0;
    for (int i=0; i <  psName->length ; i++)
        hashval = psName->chars[i] + 31 * hashval;
    return (hashval % MAX_HASH_KEY);
}

static void ht_remove_from_list(LF_HashTable_t* psHT, LF_TableEntry_t* psTEToRemove, LF_TableEntry_t* psPrevTE, uint32_t uKey )
{
    //This is the first entry
    if (psPrevTE == NULL)
    {
        psHT->psEntries[uKey] = psHT->psEntries[uKey]->psNextEntry;
    }
    else
        psPrevTE->psNextEntry = psTEToRemove->psNextEntry;

    assert(psHT->uCounter != 0);
    psHT->uCounter--;

    free(psTEToRemove);
}

int ht_AllocateHashTable(struct LF_HashTable** ppTable)
{
    *ppTable = (struct LF_HashTable*) malloc(sizeof(struct LF_HashTable));
    if (*ppTable == NULL)
        return ENOMEM;

    memset(*ppTable, 0, sizeof(struct LF_HashTable));
    MultiReadSingleWrite_Init(&(*ppTable)->sHTLck);
    return 0;
}

// Should be locked from outside the function
LF_TableEntry_t* ht_LookupByName(LF_HashTable_t* psHT, struct unistr255* psName, uint32_t* puKey)
{
    uint32_t uKey = hash(psName);
    if (puKey != NULL)
        *puKey = uKey;

    return psHT->psEntries[uKey];
}

LF_TableEntry_t* ht_LookupByEntry(LF_HashTable_t* psHT, struct unistr255* psName, uint64_t uEntry, LF_TableEntry_t** ppsPrevTE, uint32_t* puKey)
{
    MultiReadSingleWrite_LockRead(&psHT->sHTLck);
    CONV_Unistr255ToLowerCase( psName );
    LF_TableEntry_t* psTE = ht_LookupByName(psHT, psName, puKey);
    LF_TableEntry_t* psPrevTE = NULL;

    /* step through linked list */
    for (; psTE != NULL; psTE = psTE->psNextEntry)
    {
        if (psTE->uEntryNum == uEntry)
        {
            if (ppsPrevTE != NULL)
                *ppsPrevTE = psPrevTE;

            MultiReadSingleWrite_FreeRead(&psHT->sHTLck);
            return psTE; /* found */
        }
        psPrevTE = psTE;
    }

    MultiReadSingleWrite_FreeRead(&psHT->sHTLck);
    return NULL; /* not found */
}

/* force inserts the key-val pair */
static void ht_remove_rand_item(LF_HashTable_t* psHT, uint32_t uStartKey)
{
    bool bRemoved = false;
    uint32_t uRandKey = uStartKey;
    while (!bRemoved)
    {
        if (psHT->psEntries[uRandKey] != NULL)
        {
            ht_remove_from_list(psHT, psHT->psEntries[uRandKey], NULL, uRandKey);
            bRemoved = true;
        }
        uRandKey = rand() % MAX_HASH_KEY;
    }
}

/* inserts the key-val pair */
int ht_insert(LF_HashTable_t* psHT, struct unistr255* psName, uint64_t uEntryNum, bool bForceInsert)
{
    int err = 0;
    uint32_t uKey;
    bool bNeedToEvictRandom = false;
    //check if we can insert more values
    if (ht_reached_max_bound(psHT))
    {
        if (!bForceInsert)
        {
            psHT->bIncomplete = 1;
            return 0;
        }
        else
        {
            bNeedToEvictRandom = true;
        }
    }

    LF_TableEntry_t* psTE = ht_LookupByEntry( psHT, psName, uEntryNum, NULL, &uKey);
    if (psTE != NULL)
    {
        return EEXIST;
    }

    MultiReadSingleWrite_LockWrite(&psHT->sHTLck);

    //If we are in force insert and there is no space
    //evict a random item
    if (bNeedToEvictRandom)
    {
        ht_remove_rand_item(psHT, uKey);
    }

    LF_TableEntry_t* psPrevTE = NULL;
    LF_TableEntry_t* psNewTE = (LF_TableEntry_t*) malloc (sizeof(LF_TableEntry_t));
    if (psNewTE == NULL)
    {
        err= ENOMEM;
        goto exit;
    }

    psNewTE->psNextEntry = NULL;
    psNewTE->uEntryNum = uEntryNum;
    psTE = psHT->psEntries[uKey];
    /* look for the correct place */
    /* if this is the first one */
    if (psTE == NULL)
    {
        psHT->psEntries[uKey] = psNewTE;
        goto exit;
    }

    for (; psTE != NULL; psTE = psTE->psNextEntry)
    {
        if (psTE->uEntryNum > uEntryNum)
        {
            psNewTE->psNextEntry = psTE;
            (psPrevTE == NULL) ? (psHT->psEntries[uKey] = psNewTE) : (psPrevTE->psNextEntry = psNewTE);

            goto exit;
        }

        psPrevTE = psTE;
    }

    // We got to the last entry
    if (psTE == NULL && psPrevTE != NULL)
    {
        psPrevTE->psNextEntry = psNewTE;
    }

exit:
    if (!err) psHT->uCounter++;
    MultiReadSingleWrite_FreeWrite(&psHT->sHTLck);
    return err;
}

bool ht_reached_low_bound(LF_HashTable_t* psHT)
{
    return (psHT->uCounter <= LOW_BOUND_HASH_VALUES);
}

bool ht_reached_max_bound(LF_HashTable_t* psHT)
{
    return !(psHT->uCounter < MAX_HASH_VALUES);
}

int ht_remove(LF_HashTable_t* psHT, const char *pcUTF8Name, uint64_t uEntry)
{
    LF_TableEntry_t* psPrevTE;
    uint32_t uKey;
    struct unistr255* psName = (struct unistr255*) malloc(sizeof(struct unistr255));
    if ( psName == NULL )
    {
        return ENOMEM;
    }
    memset( psName, 0, sizeof(struct unistr255));

    // Convert the search name to UTF-16
    uint32_t uFlags = (DIROPS_IsDotOrDotDotName(pcUTF8Name))? 0 : UTF_SFM_CONVERSIONS;
    int iError = CONV_UTF8ToUnistr255((const uint8_t *)pcUTF8Name, strlen(pcUTF8Name), psName, uFlags);
    if ( iError != 0 )
    {
        goto exit;
    }

    LF_TableEntry_t* psTE = ht_LookupByEntry( psHT, psName, uEntry, &psPrevTE, &uKey);
    if (psTE == NULL)
    {
        return (psHT->bIncomplete)? 0 : ENOENT;
    }

    MultiReadSingleWrite_LockWrite(&psHT->sHTLck);

    ht_remove_from_list(psHT, psTE, psPrevTE, uKey);

    MultiReadSingleWrite_FreeWrite(&psHT->sHTLck);

exit:
    if (psName != NULL)
        free(psName);

    return iError;
}

/* recursively frees table entriy chains, starting with last one added */
void ht_free_all(LF_HashTable_t *psHT)
{
    MultiReadSingleWrite_LockWrite(&psHT->sHTLck);

    LF_TableEntry_t* psTE = NULL;
    LF_TableEntry_t* psNextTE = NULL;

    for (uint64_t uKey = 0; uKey < MAX_HASH_KEY; uKey++)
    {
        if (psHT->psEntries[uKey] != NULL)
        {
            for (psTE = psHT->psEntries[uKey]; psTE != NULL; psTE = psNextTE)
            {
                psNextTE = psTE->psNextEntry;
                free(psTE);
            }
            psHT->psEntries[uKey] = NULL;
        }
    }

    //Free the table as well
    MultiReadSingleWrite_FreeWrite(&psHT->sHTLck);
}

void ht_DeAllocateHashTable(LF_HashTable_t *psHT)
{
    ht_free_all(psHT);
    MultiReadSingleWrite_DeInit(&psHT->sHTLck);
    free(psHT);
}
