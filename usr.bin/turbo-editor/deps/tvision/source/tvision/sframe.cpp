/*------------------------------------------------------------*/
/* filename -       sframe.cpp                                */
/*                                                            */
/* Registeration object for the class TFrame                  */
/*------------------------------------------------------------*/
/*
 *      Turbo Vision - Version 2.0
 *
 *      Copyright (c) 1994 by Borland International
 *      All Rights Reserved.
 *
 */

#if !defined(NO_STREAMABLE)
#define Uses_TFrame
#define Uses_TStreamableClass
#include <tvision/tv.h>

TStreamableClass RFrame( TFrame::name,
                         TFrame::build,
                         __DELTA(TFrame)
                       );
#endif

