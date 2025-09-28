/*------------------------------------------------------------*/
/* filename -       sbkgrnd.cpp                               */
/*                                                            */
/* Registeration object for the class TBackground             */
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
#define Uses_TBackground
#include <tvision/tv.h>
__link( RView )

TStreamableClass RBackground( TBackground::name,
                              TBackground::build,
                              __DELTA(TBackground)
                            );
#endif

