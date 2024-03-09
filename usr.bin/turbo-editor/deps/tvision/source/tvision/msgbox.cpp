/*------------------------------------------------------------*/
/* filename -       msgbox.cpp                                */
/*                                                            */
/* function(s)                                                */
/*                  messageBox related functions              */
/*------------------------------------------------------------*/
/*
 *      Turbo Vision - Version 2.0
 *
 *      Copyright (c) 1994 by Borland International
 *      All Rights Reserved.
 *
 */


#define Uses_MsgBox
#define Uses_TObject
#define Uses_TDialog
#define Uses_TStaticText
#define Uses_TRect
#define Uses_TButton
#define Uses_TProgram
#define Uses_TInputLine
#define Uses_TDeskTop
#define Uses_TLabel
#include <tvision/tv.h>

#if !defined( __STRING_H )
#include <string.h>
#endif  // __STRING_H

static const char *buttonName[] =
{
    MsgBoxText::yesText,
    MsgBoxText::noText,
    MsgBoxText::okText,
    MsgBoxText::cancelText
};

static ushort commands[] =
{
    cmYes,
    cmNo,
    cmOK,
    cmCancel
};

static const char *Titles[] =
{
    MsgBoxText::warningText,
    MsgBoxText::errorText,
    MsgBoxText::informationText,
    MsgBoxText::confirmText
};

ushort messageBoxRect( const TRect &r, TStringView msg, ushort aOptions ) noexcept
{
    TDialog *dialog;
    short i, x, buttonCount;
    TView* buttonList[5];
    ushort ccode;

    dialog = new TDialog( r, Titles[aOptions & 0x3] );

    dialog->insert(
        new TStaticText(TRect(3, 2, dialog->size.x - 2, dialog->size.y - 3),
                        msg) );

    for( i = 0, x = -2, buttonCount = 0; i < 4; i++ )
        {
        if( (aOptions & (0x0100 << i)) != 0)
            {
            buttonList[buttonCount] =
                new TButton( TRect(0, 0, 10, 2), buttonName[i], commands[i], bfNormal );
            x += buttonList[buttonCount++]->size.x + 2;
            }
        }

    x = (dialog->size.x - x) / 2;

    for( i = 0; i < buttonCount; i++ )
        {
        dialog->insert(buttonList[i]);
        buttonList[i]->moveTo(x, dialog->size.y - 3);
        x += buttonList[i]->size.x + 2;
        }

    dialog->selectNext(False);

    ccode = TProgram::application->execView(dialog);

    TObject::destroy( dialog );

    return ccode;
}

ushort messageBoxRect( const TRect &r,
                       ushort aOptions,
                       const char *fmt,
                       ... ) noexcept
{
    va_list argptr;

    va_start( argptr, fmt );
    char *msg = vfmtStr( fmt, argptr );
    va_end( argptr );

    ushort ret = messageBoxRect( r, msg, aOptions );

    delete[] msg;
    return ret;
}

static TRect makeRect(TStringView text)
{
    TRect r( 0, 0, 40, 9 );

    int width = strwidth(text);
    if (width > (r.b.x - 7)*(r.b.y - 6))
        r.b.y = width/(r.b.x - 7) + 6 + 1;

    r.move((TProgram::deskTop->size.x - r.b.x) / 2,
           (TProgram::deskTop->size.y - r.b.y) / 2);
    return r;
}

ushort messageBox( TStringView msg, ushort aOptions ) noexcept
{
    return messageBoxRect( makeRect(msg), msg, aOptions );
}

ushort messageBox( unsigned aOptions, const char *fmt, ... ) noexcept
{
    va_list argptr;

    va_start( argptr, fmt );
    char *msg = vfmtStr( fmt, argptr );
    va_end( argptr );

    ushort ret = messageBoxRect( makeRect(msg), msg, aOptions );

    delete[] msg;
    return ret;
}

ushort inputBox( TStringView Title, TStringView aLabel, char *s, uchar limit ) noexcept
{
    TRect r(0, 0, 60, 8);
    r.move((TProgram::deskTop->size.x - r.b.x) / 2,
           (TProgram::deskTop->size.y - r.b.y) / 2);
    return inputBoxRect(r, Title, aLabel, s, limit);
}

ushort inputBoxRect( const TRect &bounds,
                     TStringView Title,
                     TStringView aLabel,
                     char *s,
                     uchar limit ) noexcept
{
    TDialog *dialog;
    TView* control;
    TRect r;
    ushort c;

    dialog = new TDialog(bounds, Title);

    r = TRect( 4 + aLabel.size(), 2, dialog->size.x - 3, 3 );
    control = new TInputLine( r, limit );
    dialog->insert( control );

    r = TRect(2, 2, 3 + aLabel.size(), 3);
    dialog->insert( new TLabel( r, aLabel, control ) );

    r = TRect( dialog->size.x - 24, dialog->size.y - 4,
               dialog->size.x - 14, dialog->size.y - 2);
    dialog->insert( new TButton(r, MsgBoxText::okText, cmOK, bfDefault));

    r.a.x += 12;
    r.b.x += 12;
    dialog->insert( new TButton(r, MsgBoxText::cancelText, cmCancel, bfNormal));

    r.a.x += 12;
    r.b.x += 12;
    dialog->selectNext(False);
    dialog->setData(s);
    c = TProgram::application->execView(dialog);
    if( c != cmCancel )
        dialog->getData(s);
    TObject::destroy( dialog );
    return c;
}

