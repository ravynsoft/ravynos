/*------------------------------------------------------------*/
/* filename -       sview.cpp                                 */
/*                                                            */
/* Registeration object for the class TView                   */
/*------------------------------------------------------------*/
/*
 *      Turbo Vision - Version 2.0
 *
 *      Copyright (c) 1994 by Borland International
 *      All Rights Reserved.
 *
 */

#if !defined(NO_STREAMABLE)
#define Uses_TView
#define Uses_TStreamableClass
#include <tvision/tv.h>

TStreamableClass RView( TView::name,
                        TView::build,
                        __DELTA(TView)
                      );

#endif
