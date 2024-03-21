//
// PALETTE.CPP - Example module for palette system.
//

#define Uses_TView
#define Uses_TWindow
#define Uses_TPalette
#define Uses_TDrawBuffer
#include <tv.h>
#include "palette.h"                    // Class definitions for
                                        // this module
#include <stdio.h>                      // For sprintf()

//
//  TTestView constructor
//

#define cpTestView "\x9\xA\xB\xC\xD\xE" // SIX colors available
                                        // in this view.

TTestView::TTestView( TRect& r ) : TView( r )
{
}

void TTestView::draw()
{
    TDrawBuffer buf;
    char textAttr, text[128];
    for(int i = 1; i <= 6; i++)         // Loop through palette
                                        // (6 entries).
    {
        textAttr = getColor( i );       // Obtain attribute for
                                        // given index.
        sprintf(text, " This line uses index %02X, color is %02X ", i, textAttr);
        buf.moveStr(0, text, textAttr);      // Write to buffer.
        writeLine(0, i-1, size.x, i, buf);   // Write buffer to
                                             // view.
    }
//
// The last line of this view will not use the palettes at all,
// but rather will print in Purple on Black, always.
//
    buf.moveStr(0, "   This line bypasses the palettes!    ", 5);
    writeLine(0, 6, size.x, 7, buf);
}

//
// getPalette: Create and return a palette with the given values.
//

TPalette& TTestView::getPalette() const
{
    static TPalette palette( cpTestView, sizeof(cpTestView)-1 );
    return palette;
}

//
// TTestWindow
//

#define cpTestWindow "\x40\x41\x42\x43\x44\x45"
// SIX new colors!
#define cpBlueWindow "\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F"
// original palettes
#define cpCyanWindow "\x10\x11\x12\x13\x14\x15\x16\x17"
#define cpGrayWindow "\x18\x19\x1A\x1B\x1C\x1D\x1E\x1F"

TTestWindow::TTestWindow() :
    TWindowInit( initFrame ),
    TWindow( TRect(0, 0, TEST_WIDTH, TEST_HEIGHT), 0, wnNoNumber )
{
    TRect r = getExtent();
    r.grow(-2, -2);
    insert( new TTestView(r) );
    options |= ofCentered;
    flags = wfMove | wfClose;
}

//
// getPalette: Like the system palette, windows employ more than
// one possible set of colors: blue, cyan, and gray.  Only one
// new set of colors has been defined though, so we simply
// concatenate that set to each of the others by applying the
// compiler feature that concatenates adjacent literal strings.
// This is why we use #defines instead of a  'const char *'.
//

TPalette& TTestWindow::getPalette() const
{
    static TPalette blue( cpBlueWindow cpTestWindow,
                          sizeof( cpBlueWindow cpTestWindow )-1
                        );
    static TPalette cyan( cpCyanWindow cpTestWindow,
                          sizeof( cpCyanWindow cpTestWindow )-1
                        );
    static TPalette gray( cpGrayWindow cpTestWindow,
                          sizeof( cpGrayWindow cpTestWindow )-1
                        );
    static TPalette *palettes[] = { &blue, &cyan, &gray };
    return *(palettes[palette]);    // 'palette' is a member
                                    // variable that
                                    // represents the palette
                                    // being used
                                    // currently.
}
