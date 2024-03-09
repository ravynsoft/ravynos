/*------------------------------------------------------------*/
/* filename -       sstrcoll.cpp                              */
/*                                                            */
/* Registeration object for the class TStringCollection       */
/*------------------------------------------------------------*/
/*
 *      Turbo Vision - Version 2.0
 *
 *      Copyright (c) 1994 by Borland International
 *      All Rights Reserved.
 *
 */

#define Uses_TStringCollection
#define Uses_TStreamableClass
#include <tvision/tv.h>

TStreamableClass RStringCollection( TStringCollection::name,
                                    TStringCollection::build,
                                    __DELTA(TStringCollection)
                                  );

