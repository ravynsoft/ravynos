
#ifndef _NINE_PDATA_H_
#define _NINE_PDATA_H_

#include "util/hash_table.h"

struct pheader
{
    bool unknown;
    GUID guid;
    DWORD size;
};

static bool
ht_guid_compare( const void *a,
                 const void *b )
{
    return GUID_equal(a, b);
}

static uint32_t
ht_guid_hash( const void *key )
{
    unsigned i, hash = 0;
    const unsigned char *str = key;

    for (i = 0; i < sizeof(GUID); i++) {
        hash = (unsigned)(str[i]) + (hash << 6) + (hash << 16) - hash;
    }

    return hash;
}

static void
ht_guid_delete( struct hash_entry *entry )
{
    struct pheader *header = entry->data;
    void *header_data = (void *)header + sizeof(*header);

    if (header->unknown) { IUnknown_Release(*(IUnknown **)header_data); }
    FREE(header);
}

#endif /* _NINE_PDATA_H_ */
