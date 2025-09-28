/* ------------------------------------------------------------------------*/
/*                                                                         */
/*   MSGBOX.H                                                              */
/*                                                                         */
/*   defines the functions messageBox(), messageBoxRect(),                 */
/*   inputBox(), and inputBoxRect()                                        */
/*                                                                         */
/* ------------------------------------------------------------------------*/
/*
 *      Turbo Vision - Version 2.0
 *
 *      Copyright (c) 1994 by Borland International
 *      All Rights Reserved.
 *
 */

#if defined( Uses_MsgBox ) && !defined( __MsgBox )
#define __MsgBox

#if defined( __BORLANDC__ )
#pragma option -Vo-
#endif
#if defined( __BCOPT__ ) && !defined (__FLAT__)
#pragma option -po-
#endif

class _FAR TRect;

ushort messageBox( TStringView msg, ushort aOptions ) noexcept;
ushort messageBox( unsigned aOptions, const char *msg, ... ) noexcept;

ushort messageBoxRect( const TRect &r, TStringView msg, ushort aOptions ) noexcept;
ushort messageBoxRect( const TRect &r, ushort aOptions, const char *msg, ... ) noexcept;

ushort inputBox( TStringView Title, TStringView aLabel, char *s, uchar limit ) noexcept;

ushort inputBoxRect( const TRect &bounds, TStringView title,
                     TStringView aLabel, char *s, uchar limit ) noexcept;

const int

//  Message box classes

    mfWarning      = 0x0000,       // Display a Warning box
    mfError        = 0x0001,       // Dispaly a Error box
    mfInformation  = 0x0002,       // Display an Information Box
    mfConfirmation = 0x0003,       // Display a Confirmation Box

// Message box button flags

    mfYesButton    = 0x0100,       // Put a Yes button into the dialog
    mfNoButton     = 0x0200,       // Put a No button into the dialog
    mfOKButton     = 0x0400,       // Put an OK button into the dialog
    mfCancelButton = 0x0800,       // Put a Cancel button into the dialog

    mfYesNoCancel  = mfYesButton | mfNoButton | mfCancelButton,
                                    // Standard Yes, No, Cancel dialog
    mfOKCancel     = mfOKButton | mfCancelButton;
                                    // Standard OK, Cancel dialog

class MsgBoxText
{

public:

    static const char * _NEAR yesText;
    static const char * _NEAR noText;
    static const char * _NEAR okText;
    static const char * _NEAR cancelText;
    static const char * _NEAR warningText;
    static const char * _NEAR errorText;
    static const char * _NEAR informationText;
    static const char * _NEAR confirmText;
};

#if defined( __BORLANDC__ )
#pragma option -Vo.
#endif
#if defined( __BCOPT__ ) && !defined (__FLAT__)
#pragma option -po.
#endif

#endif  // Uses_MsgBox
