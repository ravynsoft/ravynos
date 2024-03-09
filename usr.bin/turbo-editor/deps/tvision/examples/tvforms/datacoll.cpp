/*-------------------------------------------------------*/
/*                                                       */
/*   Turbo Vision Forms Demo                             */
/*                                                       */
/*   Datacoll.cpp: Support source file for TVFORMS demo  */
/*-------------------------------------------------------*/
/*
 *      Turbo Vision - Version 2.0
 *
 *      Copyright (c) 1994 by Borland International
 *      All Rights Reserved.
 *
 */

#define Uses_TStreamableClass
#define Uses_TStringCollection
#define Uses_ipstream
#define Uses_opstream
#include <tvision/tv.h>
__link( RStringCollection )

#if !defined( __DATACOLL_H )
#include "datacoll.h"
#endif  // __DATACOLL_H

#if !defined( __STRING_H )
#include <string.h>
#endif  // __STRING_H

const char * const TDataCollection::name = "TDataCollection";

void TDataCollection::write( opstream& os )
{
    os << itemSize;
    TStringCollection::write(os);
    int temp = int(keyType);
    os << temp;
}

void *TDataCollection::read( ipstream& is )
{
    is >> itemSize;
    TStringCollection::read( is );
    int temp;
    is >> temp;
    keyType = KeyTypes(temp);
    status = 0;
    return this;
}

TStreamable *TDataCollection::build()
{
    return new TDataCollection( streamableInit );
}

void TDataCollection::writeItem( void *obj, opstream& os )
{
    os.writeBytes( obj, itemSize );
}

void *TDataCollection::readItem( ipstream& is )
{
    void *obj;

    obj = new char[itemSize];
    is.readBytes(obj, itemSize);
    return obj;
}

TStreamableClass RDataCollection( TDataCollection::name,
                                  TDataCollection::build,
                                  __DELTA(TDataCollection)
                                );

TDataCollection::TDataCollection( short aLimit, short aDelta,
                                  int anItemSize,
                                  KeyTypes aKeyType) :
    TStringCollection( aLimit, aDelta ),
    itemSize( anItemSize ),
    keyType( aKeyType )
{
}

int TDataCollection::compare( void *key1, void *key2 )
{

    if (keyType == stringKey)
        return stricmp((char*)key1, (char*) key2);
    else
        {
        if (!key1 || !key2 || *(int32_t *)key1 < *(int32_t *)key2)
            return -1;
        else if (*(int32_t *)key1 == *(int32_t *)key2)
            return 0;
        else
            return 1;
        }
}

void TDataCollection::error( int code )
// Save error status instead of giving a runtime error
{
    status = code;
}

void TDataCollection::freeItem( void *item )
{
    if (item != NULL)
        delete[] (char *) item;
}

void TDataCollection::setLimit( int aLimit )
{
    void **aItems;

    if (aLimit < count)
        aLimit = count;
    if (aLimit > maxCollectionSize)
        aLimit = maxCollectionSize;
    if (aLimit != limit)
        {
        if (aLimit == 0)
            aItems = NULL;
        else
            {
            // Restrict collection: don't allow it to eat into safety pool.
            // Requires careful checking for success at point of insertion.
            aItems = new void *[aLimit];
            if ( (count != 0) && (items != 0) )
                memcpy(aItems, items, count * sizeof(void *));
            }
        if (limit != 0)
            delete[] items;
        items = aItems;
        limit = aLimit;
        }
}
