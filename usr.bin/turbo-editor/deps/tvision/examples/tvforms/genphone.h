/*-------------------------------------------------------*/
/*                                                       */
/*   Turbo Vision Forms Demo                             */
/*                                                       */
/*-------------------------------------------------------*/
/*
 *      Turbo Vision - Version 2.0
 *
 *      Copyright (c) 1994 by Borland International
 *      All Rights Reserved.
 *
 */

// Use GENFORMS.MAK to generate data files for TVFORMS demo
// (this file is used in GENFORM.CPP).

#if !defined( __GENPHONE_H )
#define __GENPHONE_H

#define Uses_TButton
#define Uses_TRect
#define Uses_TCheckBoxes
#define Uses_TInputLine
#define Uses_TLabel
#define Uses_TMenuBar
#define Uses_TSItem
#define Uses_TRadioButtons
#include <tvision/tv.h>
__link( RButton )
__link( RCheckBoxes )
__link( RInputLine )
__link( RLabel )
__link( RMenuBar )
__link( RRadioButtons )
__link( RFrame )

#if !defined( __FORMS_H )
#include "forms.h"
#endif  // _FORMS_H

#if !defined( __FORMCMDS_H )
#include "formcmds.h"
#endif  // _FORMCMDS_H

#if !defined( __DATACOLL_H )
#include "datacoll.h"
#endif  // _DATACOLL_H

#if !defined( __FIELDS_H )
#include "fields.h"
#endif  // _FIELDS_H

#if !defined( __STRING_H )
#include <string.h>
#endif  // _STRING_H

#if defined( __FLAT__ )
#define FORM_EXTENSION "f32"
#else
#define FORM_EXTENSION "f16"
#endif

const char rezFileName[] = "phonenum." FORM_EXTENSION;

const int
    nameWidth = 25,
    companyWidth = 23,
    remarksWidth = 22,
    phoneWidth = 20;

struct TDataRec
{
    char name[nameWidth];
    char company[companyWidth];
    char remarks[remarksWidth];
    char phone[phoneWidth];
    ushort acquaintType;
    ushort gender;
};

const Boolean allowDuplicates = True;

const KeyTypes dataKeyType = stringKey;

const int
    dataCount = 4,
    male      = 0,
    female    = 1,
    business  = 0x1,
    personal  = 0x2;

const TDataRec data[dataCount] =
{
    {"Helton, Andrew", "Asterisk International", "Purch. Mgr.", "(415) 868-3964",
        business | personal, male},
    {"White, Natalie", "Exclamation, Inc.", "VP sales", "(408) 242-2030",
        business, female},
    {"Stern, Peter", "", "Decent violinist", "(111) 222-5555",
        personal, male},
    {"Whitcom, Hana O.", "Nate's girlfriend", "Birthday: Jan 8, 1990", "(408) 426-1234",
        personal, female},
};


TForm *makeForm()
{
    const int
        formX1 = 5,
        formY1 = 3,
        formWd = 41,
        formHt = 17,
        labelCol = 1,
        labelWid = 8,
        inputCol = 11,
        buttonWd = 12;

    TForm *f;
    int x,y;
    TView *control;

    // Create a form
    TRect r(formX1, formY1, formX1 + formWd, formY1 + formHt);
    f = new TForm(r, "Phone Numbers");

    // Create and insert controls into the form
    y = 2;
    f->keyWidth = nameWidth + 2;
    r = TRect(inputCol, y, inputCol + nameWidth + 2, y + 1);
    control = new TKeyInputLine(r, nameWidth);
    f->insert(control);
    r = TRect(labelCol, y, labelCol + labelWid, y + 1);

    f->insert(new TLabel(r, "~N~ame", control));

    y += 2;
    r = TRect(inputCol, y, inputCol + companyWidth + 2, y + 1);
    control = new TInputLine(r, companyWidth);
    f->insert(control);

    r = TRect(labelCol, y, labelCol + labelWid, y + 1);
    f->insert(new TLabel(r, "~C~ompany", control));

    y += 2;
    r = TRect(inputCol, y, inputCol + remarksWidth + 2, y + 1);
    control = new TInputLine(r, remarksWidth);
    f->insert(control);

    r = TRect(labelCol, y, labelCol + labelWid, y + 1);

    f->insert(new TLabel(r, "~R~emarks", control));

    y += 2;
    r = TRect(inputCol, y, inputCol + phoneWidth + 2, y + 1);
    control = new TInputLine(r, phoneWidth);
    f->insert(control);

    r = TRect(labelCol, y, labelCol + labelWid, y + 1);
    f->insert(new TLabel(r, "~P~hone", control));

    // Checkboxes
    x = inputCol;
    y += 3;
    r = TRect(inputCol, y , inputCol + strlen("Business") + 6, y + 2);
    control = new TCheckBoxes(r,
                      new TSItem("Business",
                      new TSItem("Personal",
                      NULL)));
    f->insert(control);
    r = TRect(x, y - 1, x + labelWid, y);
    f->insert(new TLabel(r, "~T~ype", control));

    // Radio buttons 
    x += 15;
    r = TRect(x, y, x + strlen("Female") + 6, y + 2);

    control = new TRadioButtons(r,
                   new TSItem("Male",
                   new TSItem("Female", NULL)));
    f->insert(control);
    r = TRect(x, y - 1, x + labelWid, y);
    f->insert(new TLabel(r, "~G~ender", control));

    // Buttons 
    y += 3;
    x = formWd - 2 * (buttonWd + 2);
    r = TRect(x, y, x + buttonWd, y + 2);
    f->insert(new TButton(r, "~S~ave", cmFormSave, bfDefault));

    x = formWd - 1 * (buttonWd + 2);
    r = TRect(x, y, x + buttonWd, y + 2);
    f->insert(new TButton(r, "Cancel", cmCancel, bfNormal));
    f->selectNext(False);      // Select first field 

    return f;
}

#endif  // __GENPHONE_H
