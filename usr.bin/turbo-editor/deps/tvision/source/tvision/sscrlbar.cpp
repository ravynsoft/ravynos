/*------------------------------------------------------------*/
/* filename -       sscrlbar.cpp                              */
/*                                                            */
/* Registeration object for the class TScrollBar              */
/*------------------------------------------------------------*/
/*
 *      Turbo Vision - Version 2.0
 *
 *      Copyright (c) 1994 by Borland International
 *      All Rights Reserved.
 *
 */

#if !defined(NO_STREAMABLE)
#define Uses_TScrollBar
#define Uses_TStreamableClass
#include <tvision/tv.h>
__link( RView )

TStreamableClass RScrollBar( TScrollBar::name,
                              TScrollBar::build,
                              __DELTA(TScrollBar)
                           );
#endif

