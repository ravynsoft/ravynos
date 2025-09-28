/*--------------------------------------------------------------*/
/* filename -           srescoll.cpp                            */
/*                                                              */
/* Registeration object for the class TResourceCollection       */
/*--------------------------------------------------------------*/
/*
 *      Turbo Vision - Version 2.0
 *
 *      Copyright (c) 1994 by Borland International
 *      All Rights Reserved.
 *
 */

#define Uses_TResourceCollection
#define Uses_TStreamableClass
#include <tvision/tv.h>

TStreamableClass RResourceCollection( TResourceCollection::name,
                                      TResourceCollection::build,
                                      __DELTA(TResourceCollection)
                                    );
