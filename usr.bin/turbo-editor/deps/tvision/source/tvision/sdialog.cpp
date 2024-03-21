/*------------------------------------------------------------*/
/* filename -       sdialog.cpp                               */
/*                                                            */
/* Registeration object for the class TDialog                 */
/*------------------------------------------------------------*/
/*
 *      Turbo Vision - Version 2.0
 *
 *      Copyright (c) 1994 by Borland International
 *      All Rights Reserved.
 *
 */

#if !defined(NO_STREAMABLE)
#define Uses_TDialog
#define Uses_TStreamableClass
#include <tvision/tv.h>

__link( RWindow )

TStreamableClass RDialog( TDialog::name,
                          TDialog::build,
                          __DELTA(TDialog)
                        );
#endif

