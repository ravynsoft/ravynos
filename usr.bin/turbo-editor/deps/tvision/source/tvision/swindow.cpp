/*------------------------------------------------------------*/
/* filename -       swindow.cpp                               */
/*                                                            */
/* Registeration object for the class TWindow                 */
/*------------------------------------------------------------*/
/*
 *      Turbo Vision - Version 2.0
 *
 *      Copyright (c) 1994 by Borland International
 *      All Rights Reserved.
 *
 */

#if !defined(NO_STREAMABLE)
#define Uses_TWindow
#define Uses_TStreamableClass
#include <tvision/tv.h>
__link( RGroup )
__link( RFrame )

TStreamableClass RWindow( TWindow::name,
                          TWindow::build,
                          __DELTA(TWindow)
                        );

#endif
