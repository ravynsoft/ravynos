/*----------------------------------------------------------*/
/*                                                          */
/*   Copyright (c) 1991 by Borland International            */
/*                                                          */
/*   Turbo Vision TVEDIT header file                        */
/*                                                          */
/*----------------------------------------------------------*/
/*
 *      Turbo Vision - Version 2.0
 *
 *      Copyright (c) 1994 by Borland International
 *      All Rights Reserved.
 *
 */

#if !defined( __TVEDIT_H )
#define __TVEDIT_H

class TMenuBar;
class TStatusLine;
class TEditWindow;
class TDialog;

const int
  cmChangeDrct = 102;

class TEditorApp : public TApplication
{

public:

    TEditorApp( int argc, char **argv );

    virtual void handleEvent( TEvent& event );
    static TMenuBar *initMenuBar( TRect );
    static TStatusLine *initStatusLine( TRect );
    virtual void outOfMemory();

private:

    TEditWindow *openEditor( const char *fileName, Boolean visible );
    void fileOpen();
    void fileNew();
    void changeDir();
};

ushort execDialog( TDialog *d, void *data );
TDialog *createFindDialog();
TDialog *createReplaceDialog();
ushort doEditDialog( int dialog, ... );

#endif // __TVEDIT_H
