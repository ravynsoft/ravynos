/*------------------------------------------------------------*/
/* filename -       smenupop.cpp                              */
/*                                                            */
/* Registeration object for the class TMenuPopup              */
/*------------------------------------------------------------*/
/*
 *      Turbo Vision - Version 2.0
 *
 *      Copyright (c) 1994 by Borland International
 *      All Rights Reserved.
 *
 */

#if !defined(NO_STREAMABLE)
#define Uses_TMenuPopup
#define Uses_TStreamableClass
#include <tvision/tv.h>

TStreamableClass RMenuPopup( TMenuPopup::name,
                           TMenuPopup::build,
                           __DELTA(TMenuPopup)
                         );
#endif
