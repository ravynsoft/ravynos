/*------------------------------------------------------------*/
/* filename -       sfilelst.cpp                              */
/*                                                            */
/* Registeration object for the class TFileList               */
/*------------------------------------------------------------*/
/*
 *      Turbo Vision - Version 2.0
 *
 *      Copyright (c) 1994 by Borland International
 *      All Rights Reserved.
 *
 */

#if !defined(NO_STREAMABLE)
#define Uses_TFileList
#define Uses_TStreamableClass
#include <tvision/tv.h>
__link( RSortedListBox )            // In case the object below is moved
                                    //   to another file.

TStreamableClass RFileList( TFileList::name,
                            TFileList::build,
                            __DELTA(TFileList)
                          );

/* TSortedListBox registration. */
__link( RListBox )

TStreamableClass RSortedListBox( TSortedListBox::name,
                                 TSortedListBox::build,
                                 __DELTA( TSortedListBox )
                               );
#endif

