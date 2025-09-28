/*------------------------------------------------------------*/
/* filename -       nmclrsel.cpp                              */
/*                                                            */
/* defines the streamable names for classes                   */
/*   TColorSelector, TMonoSelector, TColorDisplay,            */
/*   TColorGroupList, TColorItemList, TColorDialog            */
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
#include <tvision/tv.h>

const char * const _NEAR TColorSelector::name = "TColorSelector";
const char * const _NEAR TMonoSelector::name = "TMonoSelector";
const char * const _NEAR TColorDisplay::name = "TColorDisplay";
const char * const _NEAR TColorGroupList::name = "TColorGroupList";
const char * const _NEAR TColorItemList::name = "TColorItemList";
const char * const _NEAR TColorDialog::name = "TColorDialog";
#endif

