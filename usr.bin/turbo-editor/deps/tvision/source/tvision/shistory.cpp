/*------------------------------------------------------------*/
/* filename -       shistory.cpp                              */
/*                                                            */
/* Registeration object for the class THistory                */
/*------------------------------------------------------------*/
/*
 *      Turbo Vision - Version 2.0
 *
 *      Copyright (c) 1994 by Borland International
 *      All Rights Reserved.
 *
 */

#if !defined(NO_STREAMABLE)
#define Uses_THistory
#define Uses_TStreamableClass
#include <tvision/tv.h>
__link( RView )
__link( RInputLine )

TStreamableClass RHistory( THistory::name,
                           THistory::build,
                           __DELTA(THistory)
                         );
#endif

