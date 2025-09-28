/*------------------------------------------------------------*/
/* filename -       smenubox.cpp                              */
/*                                                            */
/* Registeration object for the class TMenuBox                */
/*------------------------------------------------------------*/
/*
 *      Turbo Vision - Version 2.0
 *
 *      Copyright (c) 1994 by Borland International
 *      All Rights Reserved.
 *
 */

#if !defined(NO_STREAMABLE)
#define Uses_TMenuBox
#define Uses_TStreamableClass
#include <tvision/tv.h>

TStreamableClass RMenuBox( TMenuBox::name,
                           TMenuBox::build,
                           __DELTA(TMenuBox)
                         );
#endif

