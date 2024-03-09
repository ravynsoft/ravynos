/*------------------------------------------------------------*/
/* filename -       sradiobu.cpp                              */
/*                                                            */
/* Registeration object for the class TRadioButtons           */
/*------------------------------------------------------------*/
/*
 *      Turbo Vision - Version 2.0
 *
 *      Copyright (c) 1994 by Borland International
 *      All Rights Reserved.
 *
 */

#if !defined(NO_STREAMABLE)
#define Uses_TRadioButtons
#define Uses_TStreamableClass
#include <tvision/tv.h>

__link( RCluster )

TStreamableClass RRadioButtons( TRadioButtons::name,
                                TRadioButtons::build,
                                __DELTA(TRadioButtons)
                              );
#endif

