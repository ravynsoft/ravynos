/*------------------------------------------------------------*/
/* filename -       scheckbo.cpp                              */
/*                                                            */
/* Registeration object for the class TCheckBoxes             */
/*------------------------------------------------------------*/
/*
 *      Turbo Vision - Version 2.0
 *
 *      Copyright (c) 1994 by Borland International
 *      All Rights Reserved.
 *
 */

#if !defined(NO_STREAMABLE)
#define Uses_TCheckBoxes
#define Uses_TStreamableClass
#include <tvision/tv.h>

__link( RCluster )

TStreamableClass RCheckBoxes( TCheckBoxes::name,
                              TCheckBoxes::build,
                              __DELTA(TCheckBoxes)
                            );
#endif

