/*-------------------------------------------------------*/
/*                                                       */
/*   Turbo Vision Forms Demo                             */
/*                                                       */
/*   Listdlg.cpp: Support source file for TVFORMS demo   */
/*-------------------------------------------------------*/
/*
 *      Turbo Vision - Version 2.0
 *
 *      Copyright (c) 1994 by Borland International
 *      All Rights Reserved.
 *
 */

#define Uses_TRect
#define Uses_TSortedListBox
#define Uses_TDialog
#define Uses_MsgBox
#define Uses_TProgram
#define Uses_TScrollBar
#define Uses_TButton
#define Uses_TResourceFile
#define Uses_TEvent
#define Uses_TApplication
#define Uses_TKeys
#define Uses_TDeskTop
#define Uses_TLabel
#define Uses_fpstream
#define Uses_TStreamableClass
#include <tvision/tv.h>
__link( RResourceCollection )
__link( RDialog )
__link( RScrollBar )

#if !defined( __LISTDLG_H )
#include "listdlg.h"
#endif  // __LISTDLG_H

#if !defined( __FORMCMDS_H )
#include "formcmds.h"
#endif  // __FORMCMDS_H

#if !defined( __FORMS_H )
#include "forms.h"
#endif  // __FORMS_H

#if !defined( __DATACOLL_H )
#include "datacoll.h"
#endif  // __DATACOLL_H

#if !defined( __STDLIB_H )
#include <stdlib.h>
#endif  // __STDLIB_H

#if !defined( __STDIO_H )
#include <stdio.h>
#endif  // __STDIO_H

#if !defined( __STRING_H )
#include <string.h>
#endif  // __STRING_H

#if !defined( __DIR_H )
#include <dir.h>
#endif  // __DIR_H

Boolean fileExists( char *name )
{
    struct ffblk sr;
    int ccode;

    ccode = findfirst(name, &sr, 0);
    if (!ccode)
        return True;
    else
        return False;
}

// TListKeyBox

TListKeyBox::TListKeyBox(const TRect& bounds, ushort aNumCols,
                  TScrollBar *aScrollBar):
     TSortedListBox(bounds, aNumCols, aScrollBar)
{
}

void *TListKeyBox::getKey( const char *s )
{
    if (list()->keyType == stringKey)
        return (void *) s;
    else
        return 0;
}

void TListKeyBox::getText( char *dest, short item, short maxLen )
{
    switch (list()->keyType)
        {
        case stringKey:
            TSortedListBox::getText(dest, item, maxLen);
            break;
        case longIntKey:
            ltoa(*(int32_t *)list()->keyOf(list()->at(item)), dest, 10);
            break;
        }
}

// TListDialog

TListDialog::TListDialog( char *rezName, char *title) :
    TWindowInit(&TListDialog::initFrame),
    TDialog(TRect( 2, 2, 32, 15 ), title),
    dataCollection(0),
    fileName(newStr(rezName)),
    isValid(False),
    modified(False)
{
    const short
        buttonCt      = 4,
        listX         = 2,
        listY         = 3,
        formWd        = 30,
        formHt        = 13,
        defaultListWd = 12,
        listHt        = buttonCt * 2,
        buttonWd      = 12,
        buttonY       = listY;
    TScrollBar *sb;
    short y;
    TForm *f;
    short listWd;
    short buttonX;

    // Read data off resource stream
    if (openDataFile(fileName, formDataFile, ios::in) == True)
        {
        // Get horizontal size of key field
        f = (TForm *)formDataFile->get("FormDialog");
        if (f == NULL)
            {
            messageBox("Error accessing file data.", mfError | mfOKButton);
            return;
            }

            // Base listbox width on key width. Grow entire dialog if required
        if (f->keyWidth > defaultListWd)
            {
            listWd = f->keyWidth;
            growTo((short)(formWd + listWd - defaultListWd), (short)formHt);
            }
        else
            listWd = defaultListWd;

        // Move to upper right corner of desktop
        TRect r (TProgram::deskTop->getExtent());   // Desktop coordinates
        moveTo((short)(r.b.x - size.x), 1);

        destroy(f);

        // Read data collection into memory
        dataCollection = (TDataCollection *)formDataFile->get("FormData");
        if (dataCollection != NULL)
            {
            // Loaded successfully: build ListDialog dialog

            // Scrollbar
            sb = new TScrollBar( TRect(listX + listWd, listY,
                     listX + listWd + 1, listY + listHt));
            insert(sb);

            // List box
            list = new TListKeyBox( TRect(listX, listY, listX + listWd,
                       listY + listHt), 1, sb);
            list->newList(dataCollection);
            insert(list);

            // Label
            insert(new TLabel ( TRect(listX, listY - 1,
                       listX + 10, listY), "~K~eys", list));

            // Buttons
            buttonX = listX + listWd + 2;
            y = buttonY;

            insert(new TButton (TRect(buttonX, y, buttonX + buttonWd,
                       y + 2), "~E~dit", cmFormEdit, bfDefault));

            y += 2;

            insert(new TButton (TRect(buttonX, y, buttonX + buttonWd,
                       y + 2), "~N~ew", cmFormNew, bfNormal));

            y += 2;
            insert(new TButton (TRect(buttonX, y, buttonX + buttonWd,
                       y + 2), "~D~elete", cmFormDel, bfNormal));

            y += 2;

            insert(new TButton (TRect(buttonX, y, buttonX + buttonWd,
                       y + 2), "~S~ave", cmListSave, bfNormal));

            selectNext(False);      // Select first field
            isValid = True;
            }
        }
}

TListDialog::~TListDialog(void)
{
    if (dataCollection)
        destroy(dataCollection);
    if (formDataFile != NULL)
        destroy(formDataFile);
    if (fileName != NULL)
        delete[] fileName;
}

void TListDialog::close(void)
{
    if (valid(cmClose))
        {
        // Stop desktop video update in case there are scores of attached forms 
        TProgram::deskTop->lock();
        message(TProgram::deskTop, evBroadcast, cmCloseForm, this);
        TProgram::deskTop->unlock();
        destroy(this);
        }
}

TForm *TListDialog::editingForm()
{
    // Return pointer to the form that is editing the current selection

    return (TForm *)message(TProgram::deskTop, evBroadcast, cmEditingForm,
                            dataCollection->at(list->focused));
}

void TListDialog::formOpen(Boolean newForm)
{
    TForm *f;

    if (!newForm)
        {
        // Empty collection?
        if (dataCollection->getCount() == 0)
            return;

        // If selection is being edited, then bring its form to top
        f = editingForm();
        if (f != NULL)
            {
            f->select();
            return;
            }
        }

    // Selection is not being edited: open new form from the resource file
    f = (TForm *) formDataFile->get("FormDialog");
    if (f == NULL)
        messageBox("Error opening form.", mfError | mfOKButton);
    else
        {
        f->listDialog = this;                // Form points back to List
        if (newForm)
            f->prevData = NULL;               // Adding new form
        else
            {
            // Edit data from collection
            f->prevData = dataCollection->at(list->focused);
            f->setData(f->prevData);
            }
        if (TApplication::application->validView(f) != NULL)
            {
            stackOnPrev(f);
            TProgram::deskTop->insert(f);      // Insert & select
            }
        }
}

void TListDialog::deleteSelection()
{
    TForm *f;

    // Empty collection?
    if (dataCollection->getCount() == 0)
        return;

    // Don't allow delete of data already being edited
    f = editingForm();
    if (f != NULL)
        {
        f->select();
        messageBox("Data is already being edited. Close form before deleting.",
                    mfWarning | mfOKButton);
        return;
        }

    // Confirm delete 
    if (messageBox("Are you sure you want to delete this item?",
                    mfWarning | mfYesNoCancel) == cmYes)
        {
        dataCollection->atFree(list->focused);
        list->setRange(dataCollection->getCount());
        list->drawView();
        modified = True;
        }
}

void TListDialog::handleEvent(TEvent& event)
{
    if ( (event.what == evKeyDown) && (event.keyDown.keyCode == kbEsc) )
         {
         event.what = evCommand;
         event.message.command = cmClose;
         event.message.infoPtr = 0;
         }

    TDialog::handleEvent(event);

    switch (event.what)
        {
        case evCommand:
            switch (event.message.command)
                {
                case cmFormEdit:
                    formOpen(False);
                    break;
                case cmFormNew:
                    formOpen(True);
                    break;    
                case cmFormDel: 
                    deleteSelection();
                    break;   
                case cmListSave: 
                    if (modified)
                        saveList();
                     break;
                default: 
                    return;
                }
            clearEvent(event);
            break;
        case evKeyDown:
            switch (event.keyDown.keyCode)
                { 
                case kbIns: 
                    formOpen(True);
                    break;
                default: 
                    return;
                }
            clearEvent(event);
            break;
        case evBroadcast:
            switch (event.message.command)
                {  // Respond to broadcast from TSortedListBox 
                case cmListItemSelected: 
                    formOpen(False);
                    break;

                // Keep file from being edited simultaneously by 2 lists
                case cmEditingFile:
                    if (strcmp(fileName, (char *)event.message.infoPtr) == 0)
                        clearEvent(event);
                    break;

                // Respond to search for topmost list dialog
                case cmTopList:
                    clearEvent(event);
                    break;
                }
            break;
        }
}

Boolean TListDialog::openDataFile( char *name,
    TResourceFile *&dataFile, pstream::openmode mode )
{
    fpstream* s;

    s = new fpstream(name, mode);
    dataFile = new TResourceFile(s);
    if (!s->good())
        {
        destroy(dataFile);
        dataFile = NULL;
        return False;
        }
    else
        return True;
}

Boolean TListDialog::saveList()
{
    TResourceFile *newDataFile;
    TForm *form;
    char drive[MAXDRIVE];
    char d[MAXDIR];
    char n[MAXFILE];
    char e[MAXEXT];
    char bufStr[MAXPATH];
    short ccode;

    // Empty collection? Unedited?
    if ( (dataCollection->getCount() == 0) || (!modified) )
        {
        return True;
        }

    //saveList = False;
    // Read form definition out of original form file
    form = (TForm *)formDataFile->get("FormDialog");
    if (form == NULL)
        messageBox("Cannot find original file. Data not saved.",
                    mfError | mfOKButton);
    else
        {
        // Create new data file
        fnsplit(fileName, drive, d, n, e);
        fnmerge(bufStr, drive, d, n, ".$$$");
        if (openDataFile(bufStr, newDataFile, ios::out) == False)
            messageBox("Cannot create file. Data not saved.",
                        mfError | mfOKButton);
        else
            {
            // Create new from form and collection in memory
            newDataFile->put(form, "FormDialog");
            newDataFile->put(dataCollection, "FormData");
            newDataFile->flush();
            destroy(newDataFile);

            // Close original file, rename to .BAK
            destroy (formDataFile);
            formDataFile = NULL;
            fnmerge(bufStr, drive, d, n, ".bak");
            if (fileExists(bufStr))
                ::remove(bufStr);
            ccode = rename(fileName, bufStr);
            // Error trying to erase old .BAK or rename original to .BAK? 
            if (ccode)
                {
                messageBox("Cannot create .BAK file. Data not saved.",
                            mfError | mfOKButton);

                //Try to re-open original. New data will still be in memory
                if (openDataFile(fileName, formDataFile, ios::in) == False)
                    {
                    messageBox("Cannot re-open original file.",
                                mfError | mfOKButton);
                    destroy(this);        // Cannot proceed. Free data and close window }
                    }
                }
            else
                {
                // Rename temp file to original file and re-open 
                fnmerge(bufStr, drive, d, n, ".$$$");
                rename(bufStr, fileName);
                openDataFile(fileName, formDataFile, ios::in);

                modified = False;
                destroy(form);
                return True;
                }
            }
            destroy(form);
        }
    return False;
}

Boolean TListDialog::saveForm(TDialog *f)
{
    ccIndex i;
    void *p;

    // Validate data before updating collection
    if (!f->valid(cmFormSave))
      return False;

    // Extract data from form. Don't use safety pool.
    p = new char[dataCollection->itemSize];
    if (p == NULL)
        {
        TApplication::application->outOfMemory();
        return False;
        }

    memset(p, 0, dataCollection->itemSize);
    f->getData(p);
    // If no duplicates, make sure not attempting to add duplicate key
    if ( (!(dataCollection->duplicates) && dataCollection->search(dataCollection->keyOf(p), i)) )
        if ( (((TForm*)f)->prevData == NULL) || (((TForm *)f)->prevData != dataCollection->at(i)) )
            {
            delete[] (char *) p;
            messageBox("Duplicate keys are not allowed in this database. "
                       "Delete duplicate record before saving this form.",
                        mfError | mfOKButton);
            return False;
            }

    // Free previous data?
    if (((TForm *)f)->prevData != NULL)
        dataCollection->remove(((TForm*)f)->prevData);

    // TDataCollection.Insert may fail because it doesn't use
    //  the safety pool. Check status field after insert and cleanup
    //  if necessary.

    dataCollection->insert(p);
    if (dataCollection->status != 0)
        {
        delete[] (char *) p;
        TApplication::application->outOfMemory();
        return False;
        }

    // Success: store off original data pointer
    ((TForm *)f)->prevData = p;

    // Redraw list
    list->setRange(dataCollection->getCount());
    list->drawView();

    modified = True;
    return True;
}

void TListDialog::stackOnPrev(TDialog *f)
{
    TForm *topForm;

    // Stack on top topmost form or on top list if first form
    topForm = (TForm *)message(owner, evBroadcast, cmTopForm, this);
    if (topForm != NULL)
    // Stack on top previous topmost form
        f->moveTo(topForm->origin.x + 1, topForm->origin.y + 1);
    else
        {
        // Stack right or left of ListDialog
        if (origin.x > f->size.x)
            f->moveTo(0, origin.y);
        else
            f->moveTo(origin.x + size.x + 1, origin.y);
        }

    // Visible on desktop? Make sure at least half of form is visible
    TRect r = owner->getExtent();                      // Desktop coordinates
    if (f->origin.x + size.x / 2 > r.b.x) 
        f->moveTo(0, 1);
    if (f->origin.y + size.y / 2 > r.b.y)
        f->moveTo(f->origin.x, 1);
}

Boolean TListDialog::valid(ushort command)
{
    Boolean ok;
    ushort reply;

    ok = True;
    switch (command)
        {
        case cmValid:
            ok = isValid;
            if (!ok)
                messageBox(mfError | mfOKButton, "Error opening file (%s).", fileName);
            break;

        case cmQuit:
        case cmClose:

            // Any forms open that cannot close?
            if (message(TProgram::deskTop, evBroadcast, cmCanCloseForm, this) == NULL)
                ok = True;
            else
                ok = False;

            // Any data modified?
            if (ok && modified)
                {
                select();
                reply = messageBox("Database has been modified. Save? ",
                                    mfYesNoCancel);
                switch (reply)
                    {
                    case cmYes:
                        ok = saveList();
                        break;
                    case cmNo:
                        modified = False;       // abandon changes
                        break;
                    default:
                        ok = False;             // cancel close request
                        break;
                    }
                }
            break;
        }
    if (ok)
        return TDialog::valid(command);
    else
        return False;
}

