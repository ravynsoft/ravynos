/*------------------------------------------------------------*/
/* filename -       seditors.cpp                              */
/*                                                            */
/* Registeration objects for the following classes:           */
/*                      TIndicator                            */
/*                      TEditor                               */
/*                      TView                                 */
/*                      TMemo                                 */
/*                      TFileEditor                           */
/*                      TEditWindow                           */
/*------------------------------------------------------------*/
/*
 *      Turbo Vision - Version 2.0
 *
 *      Copyright (c) 1994 by Borland International
 *      All Rights Reserved.
 *
 */

#if !defined(NO_STREAMABLE)
#define Uses_TIndicator
#define Uses_TEditor
#define Uses_TMemo
#define Uses_TFileEditor
#define Uses_TEditWindow
#define Uses_TStreamableClass
#include <tvision/tv.h>
__link( RView )
__link( RWindow )
__link( RScrollBar )

TStreamableClass RIndicator( TIndicator::name,
                             TIndicator::build,
                             __DELTA(TIndicator)
                           );

TStreamableClass REditor( TEditor::name,
                          TEditor::build,
                          __DELTA(TEditor)
                        );

TStreamableClass RMemo( TMemo::name,
                        TMemo::build,
                        __DELTA(TMemo)
                      );

TStreamableClass RFileEditor( TFileEditor::name,
                              TFileEditor::build,
                              __DELTA(TFileEditor)
                            );

TStreamableClass REditWindow( TEditWindow::name,
                              TEditWindow::build,
                              __DELTA(TEditWindow)
                            );
#endif

