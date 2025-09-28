#ifndef BACKGRND_H
#define BACKGRND_H

#define Uses_TBackground
#define Uses_TProgram
#define Uses_TDeskTop
#define Uses_TDialog
#define Uses_TButton
#define Uses_TStaticText
#define Uses_TInputLine
#include <tvision/tv.h>

class TChBackground : public TDialog
{

public:

    TChBackground( TBackground * );
    virtual Boolean valid( ushort );

private:

    TBackground *background;
    TInputLine *input;

};

#endif
