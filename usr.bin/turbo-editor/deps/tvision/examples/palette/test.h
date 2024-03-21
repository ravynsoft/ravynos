//
// TEST.H - The class definition for TTestApp.
//
//
// class TTestApp
//      The application object, derived from the abstract class
//      TApplication
//
// Member functions:
//      TTestApp - Constructor
//      initMenuBar - custom menu
//      handleEvent - now handling menu events
//      aboutDlg - creates and shows about box
//

#if !defined( _TEST_H )
#define _TEST_H

class TTestApp : public TApplication
{
public:
    TTestApp();
    static TMenuBar *initMenuBar( TRect r );
    virtual void handleEvent( TEvent& event);
    virtual TPalette& getPalette() const;
private:
    void aboutDlg();
    void paletteView();
};

#endif  // _TEST_H
