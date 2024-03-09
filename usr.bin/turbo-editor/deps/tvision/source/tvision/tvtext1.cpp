/*------------------------------------------------------------*/
/* filename -       tvtext1.cpp                               */
/*                                                            */
/*------------------------------------------------------------*/
/*
 *      Turbo Vision - Version 2.0
 *
 *      Copyright (c) 1994 by Borland International
 *      All Rights Reserved.
 *
 */

#define Uses_TScreen
#define Uses_TRadioButtons
#define Uses_TMenuBox
#define Uses_TFrame
#define Uses_TIndicator
#define Uses_THistory
#define Uses_TColorSelector
#define Uses_TMonoSelector
#define Uses_TColorDialog
#define Uses_TInputLine
#define Uses_TStatusLine
#define Uses_TCheckBoxes
#define Uses_TScrollBar
#define Uses_TButton
#define Uses_TDirListBox
#define Uses_TFileEditor
#include <tvision/tv.h>

#include <dos.h>

static unsigned getCodePage() noexcept
{
#if defined( __BORLANDC__ )
#if !defined(__FLAT__)
    //  get version number, in the form of a normal number
    unsigned ver = (_version >> 8) | (_version << 8);
    if( ver < 0x30C )
        return 437; // United States code page, for all versions before 3.3

#if defined( __FLAT__ )
    Regs r; r.rDS.w.wl = r.rES.w.wl = -1;
#endif

    _AX = 0x6601;   // get code page
    _genInt( 0x21 );

#endif
    return _BX;
#else
    return 437;
#endif
}

void TDisplay::updateIntlChars() noexcept
{
    if(getCodePage() != 437 )
        TFrame::frameChars[30] = '\xCD';
}

extern const uchar specialChars[] =
{
    175, 174, 26, 27, ' ', ' '
};

const char * _NEAR TRadioButtons::button = " ( ) ";

const char * _NEAR TMenuBox::frameChars = " \332\304\277  \300\304\331  \263 \263  \303\304\264 ";

const char _NEAR TFrame::initFrame[19] =
  "\x06\x0A\x0C\x05\x00\x05\x03\x0A\x09\x16\x1A\x1C\x15\x00\x15\x13\x1A\x19";

char _NEAR TFrame::frameChars[33] =
    "   \xC0 \xB3\xDA\xC3 \xD9\xC4\xC1\xBF\xB4\xC2\xC5   \xC8 \xBA\xC9\xC7 \xBC\xCD\xCF\xBB\xB6\xD1 "; // for UnitedStates code page

const char * _NEAR TFrame::closeIcon = "[~\xFE~]";
const char * _NEAR TFrame::zoomIcon = "[~\x18~]";
const char * _NEAR TFrame::unZoomIcon = "[~\x12~]";
const char * _NEAR TFrame::dragIcon = "~\xC4\xD9~";
const char * _NEAR TFrame::dragLeftIcon = "~\xC0\xC4~";

const char _NEAR TIndicator::dragFrame = '\xCD';
const char _NEAR TIndicator::normalFrame = '\xC4';

const char * _NEAR THistory::icon = "\xDE~\x19~\xDD";

const char _NEAR TColorSelector::icon = '\xDB';

const char * _NEAR TMonoSelector::button = " ( ) ";
const char * _NEAR TMonoSelector::normal = "Normal";
const char * _NEAR TMonoSelector::highlight = "Highlight";
const char * _NEAR TMonoSelector::underline = "Underline";
const char * _NEAR TMonoSelector::inverse = "Inverse";

const char * _NEAR TColorDialog::colors = "Colors";
const char * _NEAR TColorDialog::groupText = "~G~roup";
const char * _NEAR TColorDialog::itemText = "~I~tem";
const char * _NEAR TColorDialog::forText = "~F~oreground";
const char * _NEAR TColorDialog::bakText = "~B~ackground";
const char * _NEAR TColorDialog::textText = "Text ";
const char * _NEAR TColorDialog::colorText = "Color";
const char * _NEAR TColorDialog::okText = "O~K~";
const char * _NEAR TColorDialog::cancelText = "Cancel";

const char _NEAR TInputLine::rightArrow = '\x10';
const char _NEAR TInputLine::leftArrow = '\x11';

const char * _NEAR TStatusLine::hintSeparator = "\xB3 ";

const char * _NEAR TCheckBoxes::button = " [ ] ";

TScrollChars _NEAR TScrollBar::vChars = {'\x1E', '\x1F', '\xB1', '\xFE', '\xB2'};
TScrollChars _NEAR TScrollBar::hChars = {'\x11', '\x10', '\xB1', '\xFE', '\xB2'};

const char * _NEAR TButton::shadows = "\xDC\xDB\xDF";
const char * _NEAR TButton::markers = "[]";

const char * _NEAR TDirListBox::pathDir   = "\xC0\xC4\xC2";
const char * _NEAR TDirListBox::firstDir  =   "\xC0\xC2\xC4";
const char * _NEAR TDirListBox::middleDir =   " \xC3\xC4";
const char * _NEAR TDirListBox::lastDir   =   " \xC0\xC4";
const char * _NEAR TDirListBox::drives = "Drives";
const char * _NEAR TDirListBox::graphics = "\xC0\xC3\xC4";

const char * _NEAR TFileEditor::backupExt = ".bak";

