/*------------------------------------------------------------*/
/* filename -       smenubar.cpp                              */
/*                                                            */
/* Registeration object for the class TMenuBar                */
/*------------------------------------------------------------*/
/*
 *      Turbo Vision - Version 2.0
 *
 *      Copyright (c) 1994 by Borland International
 *      All Rights Reserved.
 *
 */

#if !defined(NO_STREAMABLE)
#define Uses_TMenuBar
#define Uses_TStreamableClass
#include <tvision/tv.h>

TStreamableClass RMenuBar( TMenuBar::name,
                           TMenuBar::build,
                           __DELTA(TMenuBar)
                         );
#endif

