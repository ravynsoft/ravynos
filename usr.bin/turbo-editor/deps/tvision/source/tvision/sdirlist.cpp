/*------------------------------------------------------------*/
/* filename -       sdirlist.cpp                              */
/*                                                            */
/* Registeration object for the class TDirListBox             */
/*------------------------------------------------------------*/
/*
 *      Turbo Vision - Version 2.0
 *
 *      Copyright (c) 1994 by Borland International
 *      All Rights Reserved.
 *
 */

#if !defined(NO_STREAMABLE)
#define Uses_TDirListBox
#define Uses_TStreamableClass
#include <tvision/tv.h>
__link( RListBox )

TStreamableClass RDirListBox( TDirListBox::name,
                              TDirListBox::build,
                              __DELTA(TDirListBox)
                            );

#endif
