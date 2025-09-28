/*------------------------------------------------------------*/
/* filename -       slistbox.cpp                              */
/*                                                            */
/* Registeration object for the class TListBox                */
/*------------------------------------------------------------*/
/*
 *      Turbo Vision - Version 2.0
 *
 *      Copyright (c) 1994 by Borland International
 *      All Rights Reserved.
 *
 */

#if !defined(NO_STREAMABLE)
#define Uses_TListBox
#define Uses_TStreamableClass
#include <tvision/tv.h>
__link( RListViewer )

TStreamableClass RListBox( TListBox::name,
                           TListBox::build,
                           __DELTA(TListBox)
                         );
#endif

