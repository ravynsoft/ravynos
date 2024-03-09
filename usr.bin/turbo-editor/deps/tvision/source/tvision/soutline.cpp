/*------------------------------------------------------------*/
/* filename -       soutline.cpp                              */
/*                                                            */
/* Registeration object for the class TOutline                */
/*------------------------------------------------------------*/
/*
 *      Turbo Vision - Version 2.0
 *
 *      Copyright (c) 1994 by Borland International
 *      All Rights Reserved.
 *
 */

#if !defined(NO_STREAMABLE)
#define Uses_TStreamableClass
#define Uses_TOutline
#include <tvision/tv.h>
__link( RScroller )

TStreamableClass ROutline( TOutline::name,
                              TOutline::build,
                              __DELTA(TOutline)
                            );
#endif

