/*---------------------------------------------------------*/
/*                                                         */
/*   Turbo Vision Puzzle Demo                              */
/*                                                         */
/*---------------------------------------------------------*/
/*
 *      Turbo Vision - Version 2.0
 *
 *      Copyright (c) 1994 by Borland International
 *      All Rights Reserved.
 *
 */

#define Uses_TRect
#define Uses_TEvent
#define Uses_TKeys
#define Uses_TDrawBuffer
#define Uses_TStreamableClass
#define Uses_TStreamable
#define Uses_TView
#define Uses_TWindow
#define Uses_ipstream
#define Uses_opstream
#include <tvision/tv.h>
__link( RView )
__link( RWindow )

#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <strstrea.h>
#include <iomanip.h>
#include <time.h>

#include "puzzle.h"


#define cpPuzzlePalette "\x06\x07"


//
// TPuzzleView functions & static variables
//

const char * const TPuzzleView::name = "TPuzzleView";


void TPuzzleView::write( opstream& os )
{
    TView::write( os );
    os.writeBytes(board, sizeof(board));
    os << moves << solved;
}


void *TPuzzleView::read( ipstream& is )
{
    TView::read( is );
    is.readBytes(board, sizeof(board));
    is >> moves >> solved;
    return this;
}


TStreamable *TPuzzleView::build()
{
    return new TPuzzleView( streamableInit );
}


TStreamableClass RPuzzleView( TPuzzleView::name,
                              TPuzzleView::build,
                              __DELTA(TPuzzleView)
                            );


static char boardStart[16] =
    { 'A', 'B', 'C', 'D',
      'E', 'F', 'G', 'H',
      'I', 'J', 'K', 'L',
      'M', 'N', 'O', ' '
    };

static char map[15] =
    { 0, 1, 0, 1,
      1, 0, 1, 0,
      0, 1, 0, 1,
      1, 0, 1
    };


TPuzzleView::TPuzzleView(TRect& r) : TView(r)
{
    /* Initialize random number generator so that rand(), later used in
     * scramble(), works properly. */
    srand((uint)time(0));

    options |= ofSelectable;
    memset( board, ' ', sizeof(board) );

    for(int i = 0; i <= 3; i++)
        for(int j = 0; j <= 3; j++)
            board[i][j] = boardStart[i*4+j];

    scramble();
}


void TPuzzleView::draw()
{
    char tmp[8];
    TColorAttr color[2], colorBack;
    TDrawBuffer buf;

    color[0] = color[1] = colorBack = getColor(1);
    if (!solved)
        color[1] = getColor(2);

    for(short i = 0; i <= 3; i++)
        {
        buf.moveChar(0, ' ', colorBack, 18);
        if(i == 1)
            buf.moveStr(13, "Move", colorBack);
        if(i == 2)
            buf.moveStr(14, itoa(moves, tmp, 10), colorBack);
        for(short j = 0; j <= 3; j++)
            {
            strcpy(tmp, "   ");
            tmp[1] = board[i][j];
            if(board[i][j] == ' ')
                buf.moveStr( short(j*3), tmp, color[0]);
            else
                buf.moveStr( short(j*3), tmp, color[uchar(map[uchar(board[i][j]-'A')])]);
            }
        writeLine(0, i, 18, 1, buf);
        }
}


TPalette& TPuzzleView::getPalette() const
{
    static TPalette palette( cpPuzzlePalette, sizeof(cpPuzzlePalette)-1 );
    return palette;
}


void TPuzzleView::handleEvent(TEvent& event)
{
    TView::handleEvent(event);

    if (solved && (event.what & (evKeyboard | evMouse) ) )
        {
        scramble();
        clearEvent(event);
        }

    if(event.what == evMouseDown)
        {
        moveTile(event.mouse.where);
        clearEvent(event);
        winCheck();
        }
    else if(event.what == evKeyDown)
        {
        moveKey(event.keyDown.keyCode);
        clearEvent(event);
        winCheck();
        }
}

void TPuzzleView::moveKey(int key)
{
    int i;
    for(i = 0; i <= 15; i++)
        if(board[i/4][i%4] == ' ')
            break;

    int x = i % 4;
    int y = i / 4;

    switch(key)
        {
        case kbDown:
            if (y > 0)
                {
                board[y][x] = board[y-1][x];
                board[y-1][x] = ' ';
                if(moves < 1000)
                    moves++;
                }
            break;

        case kbUp:
            if (y < 3)
                {
                board[y][x] = board[y+1][x];
                board[y+1][x] = ' ';
                if(moves < 1000)
                    moves++;
                }
            break;

        case kbRight:
            if (x > 0)
                {
                board[y][x] = board[y][x-1];
                board[y][x-1] = ' ';
                if(moves < 1000)
                    moves++;
                }
            break;

        case kbLeft:
            if (x < 3)
                {
                board[y][x] = board[y][x+1];
                board[y][x+1] = ' ';
                if(moves < 1000)
                    moves++;
                }
            break;
        }
    drawView();
}

void TPuzzleView::moveTile(TPoint p)
{
    p = makeLocal(p);

    int i;
    for(i = 0; i <= 15; i++)
        if(board[i/4][i%4] == ' ')
            break;
    int x = p.x / 3;
    int y = p.y;

    switch( (y*4 + x - i) )
        {
        case -4:                            //  Piece moves down
            moveKey(kbDown);
            break;

        case -1:                            //  Piece moves right
            moveKey(kbRight);
            break;

        case 1:                             //  Piece moves left
            moveKey(kbLeft);
            break;

        case 4:                             //  Piece moves up
            moveKey(kbUp);
            break;

        }
    drawView();
}

void TPuzzleView::scramble()
{
    moves = 0;
    solved = 0;
    do
        {
        switch( (rand() >> 4) % 4)
            {
            case 0:
                moveKey(kbUp);
                break;

            case 1:
                moveKey(kbDown);
                break;

            case 2:
                moveKey(kbRight);
                break;

            case 3:
                moveKey(kbLeft);
                break;
            }
        } while (moves++ <= 500);

    moves = 0;
    drawView();
}


static const char *solution = "ABCDEFGHIJKLMNO ";

void TPuzzleView::winCheck()
{
    int i;
    for(i = 0; i <= 15; i++)
        if(board[i/4][i%4] != solution[i])
            break;

    if(i == 16)
        solved = 1;
    drawView();
}


//
// TPuzzleWindow functions
//

const char * const TPuzzleWindow::name = "TPuzzleWindow";


void TPuzzleWindow::write( opstream& os )
{
    TWindow::write( os );
}


void *TPuzzleWindow::read( ipstream& is )
{
    TWindow::read( is );
    return this;
}


TStreamable *TPuzzleWindow::build()
{
    return new TPuzzleWindow( streamableInit );
}


TStreamableClass RPuzzleWindow( TPuzzleWindow::name,
                                TPuzzleWindow::build,
                                __DELTA(TPuzzleWindow)
                              );


TPuzzleWindow::TPuzzleWindow() :
    TWindowInit( &TPuzzleWindow::initFrame ),
    TWindow( TRect(1, 1, 21, 7), "Puzzle", wnNoNumber )
{
    flags &= ~(wfZoom | wfGrow);
    growMode = 0;

    TRect r = getExtent();
    r.grow(-1, -1);
    insert( new TPuzzleView(r) );
}
