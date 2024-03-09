/*------------------------------------------------------------*/
/* filename -       smulchkb.cpp                              */
/*                                                            */
/* Registeration object for the class TMultiCheckBoxes        */
/*------------------------------------------------------------*/
/*
 *      Turbo Vision - Version 2.0
 *
 *      Copyright (c) 1994 by Borland International
 *      All Rights Reserved.
 *
 */

#if !defined(NO_STREAMABLE)
#define Uses_TMultiCheckBoxes
#define Uses_TStreamableClass
#include <tvision/tv.h>

TStreamableClass RMultiCheckBoxes( TMultiCheckBoxes::name,
                                   TMultiCheckBoxes::build,
                                   __DELTA(TMultiCheckBoxes)
                                 );
#endif

