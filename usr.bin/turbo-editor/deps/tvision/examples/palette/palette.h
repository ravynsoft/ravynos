//
// PALETTE.H - Example view with own palette.
//
//  Copyright (C) Borland International, 1991.
//

#if !defined( _PALETTE_H )
#define _PALETTE_H

//
// class TTestView
//      View that simply displays some text on the screen in a
//      random color.
//
// Member functions:
//      TTestView - Constructor
//      ~TTestView - Destructor
//      draw - Display the text
//      getPalette - To have a color to display the text in.
//

class TTestView : public TView
{
public:
    TTestView( TRect& r );
    virtual ~TTestView() {}
    virtual void draw();
    virtual TPalette& getPalette() const;
private:
};

//
// TTestWindow - provides encapsulation for TTestView, as well as
// the ability to move the view around the screen and remove it
// from the desktop with little coding (since TWindow's
// automatically call cmClose)
//
// Member functions:
//      TTestWindow - constructor
//      getPalette  -
//      sizeLimits  - so we can have a smaller than regulation
//                    TWindow.
//

#define TEST_WIDTH   42
#define TEST_HEIGHT  11

class TTestWindow : public TWindow
{
public:
    TTestWindow();
    virtual ~TTestWindow() {}
    virtual TPalette& getPalette() const;
    virtual void sizeLimits( TPoint& min, TPoint& max )
    {
          min.x = max.x = TEST_WIDTH;
          min.y = max.y = TEST_HEIGHT;
    }
};

#endif  // _PALETTE_H
