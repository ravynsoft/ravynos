/*------------------------------------------------------*/
/*                                                      */
/*   Turbo Vision Forms Demo                            */
/*                                                      */
/*------------------------------------------------------*/
/*
 *      Turbo Vision - Version 2.0
 *
 *      Copyright (c) 1994 by Borland International
 *      All Rights Reserved.
 *
 */

// Use GENFORMS.MAK to generate data files for TVFORMS demo
// (this file is used in GENFORM.CPP).

#if !defined( __GENPARTS_H )
#define __GENPARTS_H

#define Uses_TRect
#define Uses_TButton
#define Uses_TMemo
#define Uses_TLabel
#define Uses_TScrollBar
#include <tvision/tv.h>
__link( RScrollBar )
__link( RLabel )
__link( RMemo )
__link( RButton )

#if !defined( __FORMS_H )
#include "forms.h"
#endif  // __FORMS_H

#if !defined( __FORMCMDS_H )
#include "formcmds.h"
#endif  // __FORMCMDS_H

#if !defined( __DATACOLL_H )
#include "datacoll.h"
#endif  // __DATACOLL_H

#if !defined( __FIELDS_H )
#include "fields.h"
#endif  // __FIELDS_H

#if defined( __FLAT__ )
#define FORM_EXTENSION "f32"
#else
#define FORM_EXTENSION "f16"
#endif

const char rezFileName[] = "parts." FORM_EXTENSION;

const int
    partNumWidth   =   6,
    descrWidth     =  30,
    qtyWidth       =   6,
    descrLen       = 512;            // Length of text array

struct TDescrRec // See TMemoData.
{
    ushort textLen;
    char textData[descrLen];
};

struct TDataRec
{
    int32_t partNum;
    int32_t qty;
    TDescrRec descr;
};

const Boolean allowDuplicates = False;

const KeyTypes dataKeyType = longIntKey;

const int dataCount = 5;

TDataRec data[dataCount] =
{
    {1, 1036,  { 0, "Government standard issue\r"
                    "and certified by FAA for\r"
                    "international use."
               }},
    {2035, 33, { 0, "Warbling mini version with\r"
                    "modified mechanisms for\r"
                    "handling streamliners."
               }},
    {2034, 13, { 0, "Hybrid version." }},
    {2, -123,  { 0, "Catalytic version for\r"
                    "meeting stricter emission\r"
                    "standards in industrial areas."
               }},
    {45, 8567, { 0, "Prototype for new model." }},
};


void initDescrLengths()
{
    for ( int i = 0 ; i < dataCount ; i++ )
        data[i].descr.textLen = strlen(data[i].descr.textData);
}

TForm *makeForm()
{
    const int
        formX1 = 1,
        formY1 = 1,
        formWd = 36,
        formHt = 17,
        labelCol = 1,
        labelWid = 8,
        inputCol = 11,
        buttonWd = 12,
        descrHt = 6;

    TForm *f;
    int x, y;
    TView *c;
    TView *d;

    // Create a form
    TRect r(formX1, formY1, formX1 + formWd, formY1 + formHt);
    f =  new TForm(r, "Parts");

    // Create and insert controls into the form }
    f->keyWidth = partNumWidth + 2;
    y = 2;

    r = TRect(inputCol, y, inputCol + partNumWidth + 2, y + 1);
    c = new TNumInputLine(r, partNumWidth, 0, 9999);
    f->insert(c);

    r = TRect(labelCol, y, labelCol + labelWid, y + 1);
    f->insert( new TLabel(r, "~P~art", c));

    y += 2;
    r = TRect(inputCol, y, inputCol + qtyWidth + 2, y + 1);
    c = new TNumInputLine(r, qtyWidth, -99999, 99999);
    f->insert(c);

    r = TRect(labelCol, y, labelCol + qtyWidth, y + 1);
    f->insert( new TLabel(r, "~Q~ty", c));

    y += 3;
    r = TRect(labelCol + descrWidth + 1, y, labelCol + descrWidth + 2,
                  y + descrHt);
    c = new TScrollBar(r);
    f->insert(c);

    r = TRect(labelCol + 1, y, labelCol + descrWidth + 1, y + descrHt);
    d = new TMemo(r, 0, (TScrollBar *) c, 0, descrLen);
    f->insert(d);

    r = TRect(labelCol, y - 1, labelCol + strlen("Description") + 1, y);
    f->insert( new TLabel(r, "~D~escription", c));

    // Buttons 
    y += descrHt + 1;
    x = formWd - 2 * (buttonWd + 2);
    r = TRect(x, y, x + buttonWd, y + 2);
    f->insert( new TButton(r, "~S~ave", cmFormSave, bfDefault));

    x = formWd - 1 * (buttonWd + 2);
    r = TRect(x, y, x + buttonWd, y + 2);

    f->insert( new TButton(r, "Cancel", cmCancel, bfNormal));

    f->selectNext(False);      // Select first field 

    initDescrLengths();

    return f;
}

#endif  // __GENPARTS_H
