/*------------------------------------------------------------*/
/* filename -       editstat.cpp                              */
/*                                                            */
/* defines the static members of class TEditor                */
/*------------------------------------------------------------*/
/*
 *      Turbo Vision - Version 2.0
 *
 *      Copyright (c) 1994 by Borland International
 *      All Rights Reserved.
 *
 */

#define Uses_TEditor
#define Uses_TEditorDialog
#include <tvision/tv.h>

ushort defEditorDialog( int, ... )
{
    return cmCancel;
}

TEditorDialog _NEAR TEditor::editorDialog = defEditorDialog;
ushort _NEAR TEditor::editorFlags = efBackupFiles | efPromptOnReplace;
char _NEAR TEditor::findStr[maxFindStrLen] = "";
char _NEAR TEditor::replaceStr[maxReplaceStrLen] = "";
TEditor * _NEAR TEditor::clipboard = 0;

