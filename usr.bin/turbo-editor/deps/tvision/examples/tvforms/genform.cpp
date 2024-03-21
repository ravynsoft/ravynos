/*------------------------------------------------------------------*/
/*                                                                  */
/*   Turbo Vision Forms Demo                                        */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/*    This program uses GENPHONE.H and GENPARTS.H to generate forms */
/*  data files which are used by the TVFORMS demo program. Use      */
/*  GENFORMS.MAK to create data files for TVFORMS demo.             */
/*                                                                  */
/*------------------------------------------------------------------*/
/*
 *      Turbo Vision - Version 2.0
 *
 *      Copyright (c) 1994 by Borland International
 *      All Rights Reserved.
 *
 */

#define Uses_fpstream
#define Uses_TResourceFile
#define Uses_TScreen
#define Uses_TStreamableClass
#include <tvision/tv.h>
__link ( RResourceCollection )

#if defined( PHONENUM )
#include "genphone.h"
#elif defined( PARTS )
#include "genparts.h"
#else
#error Specify PHONENUM or PARTS as a conditional define, compile and then run.
#endif

#if !defined( __FORMS_H )
#include "forms.h"
#endif  // __FORMS_H

#if !defined( __STDLIB_H )
#include <stdlib.h>
#endif  // __STDLIB_H

int main(void)
{
    TSortedCollection *collection;
    int i;
    TForm *f;
    void *p;
    fpstream *s;
    TResourceFile* r;

    TScreen::clearOnSuspend=False;

    cout <<"Creating  " << rezFileName << "\n";

    // Construct stream and resource
    s = new fpstream (rezFileName, ios::out|ios::binary);
    r = new TResourceFile(s);

    // Form
    f = makeForm();
    r->put(f, "FormDialog");

    // Data
    collection = new TDataCollection((dataCount + 10), 5, sizeof(TDataRec),
                          dataKeyType);
    collection->duplicates = allowDuplicates;
    for(i = 0; i < dataCount; ++i)
        {
        p = new TDataRec;
        memset(p, 0 , sizeof(TDataRec));   // keep padding bytes initialized
        f->setData((void *)&data[i]);      // move into object
        f->getData(p);                     // move onto heap
        collection->insert(p);             // insert in sorted order
        }
    r->put(collection, "FormData");

    // Done
    TObject::destroy(f);
    TObject::destroy((TCollection *)collection);
    TObject::destroy(r);
    return 0;
}
