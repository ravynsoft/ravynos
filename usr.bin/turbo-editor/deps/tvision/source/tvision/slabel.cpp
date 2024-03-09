/*------------------------------------------------------------*/
/* filename -       slabel.cpp                                */
/*                                                            */
/* Registeration object for the class TLabel                  */
/*------------------------------------------------------------*/
/*
 *      Turbo Vision - Version 2.0
 *
 *      Copyright (c) 1994 by Borland International
 *      All Rights Reserved.
 *
 */

#if !defined(NO_STREAMABLE)
#define Uses_TLabel
#define Uses_TStreamableClass
#include <tvision/tv.h>
__link( RStaticText )

TStreamableClass RLabel( TLabel::name,
                         TLabel::build,
                         __DELTA(TLabel)
                       );
#endif

