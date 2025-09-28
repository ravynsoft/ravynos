/*------------------------------------------------------------*/
/* filename -       sfinfpne.cpp                              */
/*                                                            */
/* Registeration object for the class TFileInfoPane           */
/*------------------------------------------------------------*/
/*
 *      Turbo Vision - Version 2.0
 *
 *      Copyright (c) 1994 by Borland International
 *      All Rights Reserved.
 *
 */

#if !defined(NO_STREAMABLE)
#define Uses_TFileInfoPane
#define Uses_TStreamableClass
#include <tvision/tv.h>
__link( RView )

TStreamableClass RFileInfoPane( TFileInfoPane::name,
                                TFileInfoPane::build,
                                __DELTA(TFileInfoPane)
                              );
#endif

