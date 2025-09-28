/*------------------------------------------------------------*/
/* filename -       sdesktop.cpp                              */
/*                                                            */
/* Registeration object for the class TDeskTop                */
/*------------------------------------------------------------*/
/*
 *      Turbo Vision - Version 2.0
 *
 *      Copyright (c) 1994 by Borland International
 *      All Rights Reserved.
 *
 */

#if !defined(NO_STREAMABLE)
#define Uses_TDeskTop
#define Uses_TStreamableClass
#include <tvision/tv.h>
__link( RBackground )

TStreamableClass RDeskTop( TDeskTop::name,
                           TDeskTop::build,
                           __DELTA(TDeskTop)
                         );
#endif


