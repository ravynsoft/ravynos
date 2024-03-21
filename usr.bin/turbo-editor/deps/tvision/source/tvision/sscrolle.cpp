/*------------------------------------------------------------*/
/* filename -       sscrolle.cpp                              */
/*                                                            */
/* Registeration object for the class TScroller               */
/*------------------------------------------------------------*/
/*
 *      Turbo Vision - Version 2.0
 *
 *      Copyright (c) 1994 by Borland International
 *      All Rights Reserved.
 *
 */

#if !defined(NO_STREAMABLE)
#define Uses_TScroller
#define Uses_TStreamableClass
#include <tvision/tv.h>
__link( RView )
__link( RScrollBar )

TStreamableClass RScroller( TScroller::name,
                             TScroller::build,
                             __DELTA(TScroller)
                          );

#endif
