/*------------------------------------------------------------*/
/* filename -       sparamte.cpp                              */
/*                                                            */
/* Registeration object for the class TParamText              */
/*------------------------------------------------------------*/
/*
 *      Turbo Vision - Version 2.0
 *
 *      Copyright (c) 1994 by Borland International
 *      All Rights Reserved.
 *
 */

#if !defined(NO_STREAMABLE)
#define Uses_TParamText
#define Uses_TStreamableClass
#include <tvision/tv.h>
__link( RView )
__link( RStaticText )

TStreamableClass RParamText( TParamText::name,
                              TParamText::build,
                              __DELTA(TParamText)
                            );
#endif

