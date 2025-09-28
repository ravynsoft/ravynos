/*-------------------------------------------------------*/
/*                                                       */
/*   Turbo Vision Forms Demo                             */
/*                                                       */
/*   Listdlg.h: Header file for Listdlg.cpp              */
/*              (Support header file for TVFORMS Demo)   */
/*-------------------------------------------------------*/
/*
 *      Turbo Vision - Version 2.0
 *
 *      Copyright (c) 1994 by Borland International
 *      All Rights Reserved.
 *
 */

#if !defined( __LISTDLG_H )
#define __LISTDLG_H

#define Uses_TSortedListBox
#define Uses_TRect
#define Uses_TScrollBar
#define Uses_TDialog
#define Uses_TResourceFile
#define Uses_pstream
#include <tvision/tv.h>

#if !defined( __DATACOLL_H )
#include "datacoll.h"
#endif  // __DATACOLL_H

#if !defined( __FORMS_H )
#include "forms.h"
#endif  // __FORMS_H


typedef char * (*ExpandFunc)(char *);

class TListKeyBox :  public TSortedListBox
{

public:

    TListKeyBox( const TRect&, ushort, TScrollBar *);

    virtual void *getKey( const char * );
    virtual void getText( char *, short, short );
    void newList( TDataCollection * );
    TDataCollection *list();
};

inline void TListKeyBox::newList( TDataCollection * aList )
{
    TSortedListBox::newList(aList);
}

inline TDataCollection *TListKeyBox::list()
{
    return (TDataCollection *)TSortedListBox::list();
}

class TListDialog :  public TDialog
{

public:

    TListDialog( char *, char * );
    ~TListDialog(void);

    virtual void close();
    void deleteSelection();
    TForm *editingForm();
    void formOpen( Boolean );
    virtual void handleEvent( TEvent& );
    Boolean openDataFile( char *, TResourceFile *&, pstream::openmode );
    Boolean saveList();
    Boolean saveForm(TDialog *);
    void stackOnPrev(TDialog *);
    virtual Boolean valid( ushort );


    TDataCollection *dataCollection;
    char *fileName;
    TResourceFile *formDataFile;
    Boolean isValid;
    TListKeyBox *list;
    Boolean modified;
};

#endif  // __LISTDLG_H
