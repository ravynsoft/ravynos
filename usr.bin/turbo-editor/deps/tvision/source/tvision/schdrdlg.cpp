/*------------------------------------------------------------*/
/* filename -       schdrdlg.cpp                              */
/*                                                            */
/* Registeration object for the class TChDirDialog            */
/*------------------------------------------------------------*/
/*
 *      Turbo Vision - Version 2.0
 *
 *      Copyright (c) 1994 by Borland International
 *      All Rights Reserved.
 *
 */

#if !defined(NO_STREAMABLE)
#define Uses_TChDirDialog
#define Uses_TStreamableClass
#include <tvision/tv.h>
__link( RDialog )
__link( RButton )
__link( RDirListBox )
__link( RInputLine )
__link( RHistory )
__link( RLabel )
__link( RScrollBar )

TStreamableClass RChDirDialog( TChDirDialog::name,
                               TChDirDialog::build,
                               __DELTA(TChDirDialog)
                             );
#endif
