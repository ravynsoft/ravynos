/*------------------------------------------------------------*/
/* filename -       nmeditor.cpp                              */
/*                                                            */
/* defines the streamable names for classes                   */
/*   TIndicator, TEditor, TMemo, TFileEditor, TEditWindow     */
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
#include <tvision/tv.h>

const char * const _NEAR TIndicator::name = "TIndicator";
const char * const _NEAR TEditor::name = "TEditor";
const char * const _NEAR TMemo::name = "TMemo";
const char * const _NEAR TFileEditor::name = "TFileEditor";
const char * const _NEAR TEditWindow::name = "TEditWindow";
#endif

