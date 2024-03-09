/*------------------------------------------------------------*/
/* filename -       sstatusl.cpp                              */
/*                                                            */
/* Registeration object for the class TStatusLine             */
/*------------------------------------------------------------*/
/*
 *      Turbo Vision - Version 2.0
 *
 *      Copyright (c) 1994 by Borland International
 *      All Rights Reserved.
 *
 */

#if !defined(NO_STREAMABLE)
#define Uses_TStatusLine
#define Uses_TStreamableClass
#include <tvision/tv.h>
__link( RView )

TStreamableClass RStatusLine( TStatusLine::name,
                              TStatusLine::build,
                              __DELTA(TStatusLine)
                            );

#endif
