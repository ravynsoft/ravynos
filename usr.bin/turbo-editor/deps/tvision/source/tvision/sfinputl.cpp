/*------------------------------------------------------------*/
/* filename -       sfinputl.cpp                              */
/*                                                            */
/* Registeration object for the class TFileInputLine          */
/*------------------------------------------------------------*/
/*
 *      Turbo Vision - Version 2.0
 *
 *      Copyright (c) 1994 by Borland International
 *      All Rights Reserved.
 *
 */

#if !defined(NO_STREAMABLE)
#define Uses_TFileInputLine
#define Uses_TStreamableClass
#include <tvision/tv.h>
__link( RInputLine )

TFileInputLine::TFileInputLine( StreamableInit ) noexcept :
                TInputLine( streamableInit)
{
}

TStreamable *TFileInputLine::build()
{
    return new TFileInputLine( streamableInit );
}

TStreamableClass RFileInputLine( TFileInputLine::name,
                                 TFileInputLine::build,
                                __DELTA(TFileInputLine)
                               );
#endif

