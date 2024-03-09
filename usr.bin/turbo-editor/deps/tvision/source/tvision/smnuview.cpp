/*------------------------------------------------------------*/
/* filename -       smnuview.cpp                              */
/*                                                            */
/* Registeration object for the class TMenuView               */
/*------------------------------------------------------------*/
/*
 *      Turbo Vision - Version 2.0
 *
 *      Copyright (c) 1994 by Borland International
 *      All Rights Reserved.
 *
 */

#if !defined(NO_STREAMABLE)
#define Uses_TMenuView
#define Uses_TStreamableClass
#include <tvision/tv.h>
__link( RView )

TStreamableClass RMenuView( TMenuView::name,
                            TMenuView::build,
                            __DELTA(TMenuView)
                          );
#endif

