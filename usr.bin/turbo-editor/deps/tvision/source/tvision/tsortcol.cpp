/*------------------------------------------------------------*/
/* filename -       tsortcol.cpp                              */
/*                                                            */
/* function(s)                                                */
/*                  TNSSortedCollection member functions      */
/*------------------------------------------------------------*/
/*
 *      Turbo Vision - Version 2.0
 *
 *      Copyright (c) 1994 by Borland International
 *      All Rights Reserved.
 *
 */

#define Uses_TNSSortedCollection
#define Uses_opstream
#define Uses_ipstream
#define Uses_TSortedCollection
#include <tvision/tv.h>


ccIndex TNSSortedCollection::indexOf(void *item)
{
    ccIndex  i;

    if( search( keyOf(item), i ) == 0 )
        return ccNotFound;
    else
        {
        if( duplicates )
            {
            while( i < count && item != items[i] )
                i++;
            }
        if( i < count )
            return i;
        else
            return ccNotFound;
        }
}

ccIndex TNSSortedCollection::insert( void *item )
{
    ccIndex  i;
    if( search( keyOf(item), i ) == 0 || duplicates )   // order dependency!
        atInsert( i, item );                            // must do Search
                                                        // before calling
                                                        // AtInsert
    return i;
}

void *TNSSortedCollection::keyOf( void *item )
{
    return item;
}

Boolean TNSSortedCollection::search( void *key, ccIndex& index )
{
    ccIndex l = 0;
    ccIndex h = count - 1;
    Boolean res = False;
    while( l <= h )
        {
        ccIndex i = (l +  h) >> 1;
        ccIndex c = compare( keyOf( items[i] ), key );
        if( c < 0 )
            l = i + 1;
        else
            {
            h = i - 1;
            if( c == 0 )
                {
                res = True;
                if( !duplicates )
                    l = i;
                }
            }
        }
    index = l;
    return res;
}


void TSortedCollection::write( opstream& os )
{
    TCollection::write( os );
    os << (int)duplicates;
}

void *TSortedCollection::read( ipstream& is )
{
    TCollection::read( is );
    int temp;
    is >> temp;
    duplicates = Boolean(temp);
    return this;
}


TSortedCollection::TSortedCollection( StreamableInit ) noexcept :
    TCollection( streamableInit )
{
}

