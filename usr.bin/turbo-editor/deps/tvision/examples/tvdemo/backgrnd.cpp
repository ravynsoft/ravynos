#define Uses_TText
#include <tvision/tv.h>

#include "backgrnd.h"

TChBackground::TChBackground(TBackground *b) :
    TWindowInit( &TChBackground::initFrame ),
    TDialog(TRect(0, 0, 29, 9), 0),
    background(b)
{
    // Ideally, we would show an ASCII table and let the user choose from there.
    TRect r = getExtent();
    r.move((TProgram::deskTop->size.x - r.b.x) / 2,
           (TProgram::deskTop->size.y - r.b.y) / 2);
    changeBounds(r);
    input = new TInputLine(TRect(4, 5, 7, 6), 1, 0, ilMaxChars);
    insert(input);
    insert(new TStaticText(TRect(2, 2, 27, 3), "Enter background pattern:"));
    insert(new TButton(TRect(16, 4, 26, 6), "~A~pply", cmOK, bfDefault));
    insert(new TButton(TRect(16, 6, 26, 8), "~C~lose", cmCancel, bfNormal));
    input->focus();
}

Boolean TChBackground::valid(ushort command)
{
    if (TDialog::valid(command))
    {
        if (background && command == cmOK)
        {
            char pattern = TText::toCodePage(input->data);
            if (pattern != '\0')
            {
                background->pattern = pattern;
                background->drawView();
            }
            return False; // Keep dialog open.
        }
        return True;
    }
    return False;
}
