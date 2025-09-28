/*------------------------------------------------------------*/
/* filename -       sclrsel.cpp                               */
/*                                                            */
/* Registeration objects for the following classes:           */
/*                      TColorSelector                        */
/*                      TMonoSelector                         */
/*                      TColorDisplay                         */
/*                      TColorGroupList                       */
/*                      TColorItemList                        */
/*                      TColorDialog                          */
/*------------------------------------------------------------*/
/*
 *      Turbo Vision - Version 2.0
 *
 *      Copyright (c) 1994 by Borland International
 *      All Rights Reserved.
 *
 */

#if !defined(NO_STREAMABLE)
#define Uses_TColorSelector
#define Uses_TMonoSelector
#define Uses_TColorDisplay
#define Uses_TColorGroupList
#define Uses_TColorItemList
#define Uses_TColorDialog
#define Uses_TStreamableClass
#include <tvision/tv.h>
__link( RCluster )
__link( RView )
__link( RLabel )
__link( RButton )
__link( RListViewer )
__link( RDialog )

TStreamableClass RColorSelector( TColorSelector::name,
                                 TColorSelector::build,
                                 __DELTA(TColorSelector)
                               );

TStreamableClass RMonoSelector( TMonoSelector::name,
                                TMonoSelector::build,
                                __DELTA(TMonoSelector)
                              );

TStreamableClass RColorDisplay( TColorDisplay::name,
                                TColorDisplay::build,
                                __DELTA(TColorDisplay)
                              );

TStreamableClass RColorGroupList( TColorGroupList::name,
                                  TColorGroupList::build,
                                  __DELTA(TColorGroupList)
                                );


TStreamableClass RColorItemList( TColorItemList::name,
                                 TColorItemList::build,
                                 __DELTA(TColorItemList)
                               );

TStreamableClass RColorDialog( TColorDialog::name,
                               TColorDialog::build,
                               __DELTA(TColorDialog)
                             );

#endif
