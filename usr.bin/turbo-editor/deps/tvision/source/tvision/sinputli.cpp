/*------------------------------------------------------------*/
/* filename -       sinputli.cpp                              */
/*                                                            */
/* Registeration object for the class TInputLine              */
/*------------------------------------------------------------*/
/*
 *      Turbo Vision - Version 2.0
 *
 *      Copyright (c) 1994 by Borland International
 *      All Rights Reserved.
 *
 */

#if !defined(NO_STREAMABLE)
#define Uses_TInputLine
#define Uses_TStreamableClass
#include <tvision/tv.h>
__link( RView )

TStreamableClass RInputLine( TInputLine::name,
                             TInputLine::build,
                             __DELTA(TInputLine)
                           );
#endif

