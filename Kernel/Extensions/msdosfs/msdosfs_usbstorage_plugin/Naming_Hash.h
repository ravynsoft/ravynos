//
//  Naming_Hash.h
//  usbstorage_plugin
//
//  Created by Or Haimovich on 12/02/2019.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "MR_SW_Lock.h"
#include "Common.h"

int ht_AllocateHashTable(struct LF_HashTable** ppTable);

bool ht_reached_low_bound(LF_HashTable_t* psHT);
bool ht_reached_max_bound(LF_HashTable_t* psHT);

/* inserts the key-val pair */
int ht_insert(LF_HashTable_t* psHT, struct unistr255* psName, uint64_t uEntryNum, bool bForceInsert);

/* remove the key-val pair */
int ht_remove(LF_HashTable_t* psHT, const char *pcUTF8Name, uint64_t uEntry);

/* lookup the key-val pair */
LF_TableEntry_t* ht_LookupByEntry(LF_HashTable_t* psHT, struct unistr255* psName, uint64_t uEntry, LF_TableEntry_t** ppsPrevTE, uint32_t* puKey);

/* lookup by name -> returns the begging of the
 * hash table entry with relevant hash value */
LF_TableEntry_t* ht_LookupByName(LF_HashTable_t* psHT, struct unistr255* psName, uint32_t* puKey);

/* frees all values in the hashtable */
void ht_free_all(LF_HashTable_t* psHT);

/* free the hashtable */
void ht_DeAllocateHashTable(LF_HashTable_t *psHT);
