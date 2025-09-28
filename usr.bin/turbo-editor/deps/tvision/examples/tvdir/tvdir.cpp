/*---------------------------------------------------------*/
/*                                                         */
/*---------------------------------------------------------*/
/*
 *      Turbo Vision - Version 2.0
 *
 *      Copyright (c) 1994 by Borland International
 *      All Rights Reserved.
 *
 */

#define Uses_TKeys
#define Uses_TApplication
#define Uses_TEvent
#define Uses_TRect
#define Uses_TDialog
#define Uses_TStaticText
#define Uses_TButton
#define Uses_TMenuBar
#define Uses_TSubMenu
#define Uses_TMenuItem
#define Uses_TStatusLine
#define Uses_TStatusItem
#define Uses_TStatusDef
#define Uses_TDeskTop
#define Uses_TOutline
#define Uses_TScrollBar
#define Uses_TParamText
#define Uses_TScreen
#define Uses_TText
#include <tvision/tv.h>
#include <dos.h>
#include <string.h>
#include <stdio.h>
#include <strstrea.h>

const int cmDirTree       = 100;
const int cmAbout         = 101;
const int cmNewDirFocused = 102;

#ifdef __BORLANDC__
// This is merely for aesthetic purposes.
#define sep "\\"
#else
#define sep "/"
#endif

class QuickMessage: public TWindow
{
    TParamText *currentDir;
public:
    QuickMessage( const char *drive ):
        TWindowInit( TWindow::initFrame ),
        TWindow( TRect( 15,8,65,19 ), "Please Wait...", 0 ) {

    flags = 0; // no move, close, grow or zoom
    options |= ofCentered;
    palette = wpGrayWindow;
    char temp[64];
    ostrstream os( temp, sizeof( temp ) );
    os << "Scanning Drive '" << drive << "'\n" << ends;
    insert( new TStaticText( TRect( 2,2,48,3 ), temp ) );
    currentDir = new TParamText( TRect( 2,3,48,9 ) );
    insert( currentDir );
  }
  virtual void handleEvent( TEvent &event ) {
    TWindow::handleEvent( event );
  }
  void setCurrentDir( char *newDir ) {
    currentDir->setText( newDir );
    TScreen::flushScreen();
  }
};

class TDirOutline: public TOutline {
public:
  TDirOutline( const TRect &bounds, TScrollBar *hsb, TScrollBar *vsb, TNode *root ):
    TOutline( bounds, hsb, vsb, root ) {}
  virtual void focused( int i ) {
    foc=i;
    message( owner, evCommand, cmNewDirFocused, 0 );
  }
  static Boolean isParent( TOutlineViewer *, TNode *cur, int, int, long, ushort, void * );
  TNode *getParent( TNode *child ) {
    return firstThat( isParent, child );
  }
  void getCurrentPath( char *buffer, short bufferSize );
};

Boolean TDirOutline::isParent( TOutlineViewer *, TNode *cur, int, int, long, ushort, void *arg ) {
    TNode *parentSearch = (TNode*)arg;
    TNode *temp = cur->childList;
    while (temp!=0) {
      if (temp==parentSearch)
        return True;
      temp=temp->next;
    }
    return False;
}

void TDirOutline::getCurrentPath( char *buffer, short bufferSize ) {
    char temp1[128], temp2[128];
    TNode *current = getNode( foc );
    TNode *root = getRoot();

    temp1[0] = 0;
    temp1[sizeof(temp1) - 1] = 0;
    temp2[sizeof(temp2) - 1] = 0;
    buffer[bufferSize - 1] = 0;
    while (current!=root) {
      strncpy(temp2, temp1, sizeof(temp2) - 1);
      strncpy(temp1, current->text, sizeof(temp1) - 1);
      strncat(temp1, sep, sizeof(temp1) - 1);
      strncat(temp1, temp2, sizeof(temp1) - 1);
      current = getParent( current );
    }
    strncpy(buffer, root->text, bufferSize - 1);
    char last = buffer[strlen(buffer) - 1];
    if (last != '/' && last != '\\')
      strncat(buffer, sep, bufferSize - 1);
    strncat(buffer, temp1, bufferSize - 1);
}

TNode *getDirList( const char *path, QuickMessage *qm = 0 ) {
  TNode  *dirList = 0,
         *current = 0;
  char   searchPath[128] = {0};
  find_t searchRec;
  int    result;
  TNode  *temp;

  ostrstream os( searchPath, sizeof( searchPath )-1 );
  os << path << sep "*.*" << ends;
  result = _dos_findfirst( searchPath, 0xff, &searchRec );

  while (result==0) {
    if (searchRec.name[0]!='.') {
      if (searchRec.attrib & FA_DIREC) {
        os.seekp(0);
        os << path << *sep << searchRec.name << ends;
        // Strings may become equal when searchPath is full.
        if (strcmp(path, searchPath) == 0)
          break;
        qm->setCurrentDir(searchPath);
        temp = new TNode( searchRec.name, getDirList(searchPath,qm), 0, False );
        if (current) {
          current->next = temp;
          current=current->next;
        } else
          current = dirList = temp;
      }
    }
    result = _dos_findnext( &searchRec );
  }
  return dirList;
}

class TFilePane: public TScroller {
  char **files;
  short fileCount;

public:
  TFilePane( const TRect &bounds, TScrollBar *hsb, TScrollBar *vsb ):
    TScroller( bounds, hsb, vsb ) {
    fileCount=0;
    files=0;
  }
  ~TFilePane() {
    deleteFiles();
  }
  void newDir( const char *path );
  virtual void draw();
  void deleteFiles();
};

void TFilePane::draw() {
    TDrawBuffer dBuf;
    short i;
    for (i=0;i<size.y;i++) {
      dBuf.moveChar(0, ' ', getColor(0x0101), (short)size.x );
      if ((fileCount==0)&&(i==0))
        dBuf.moveStr( 2, "<no files>", getColor(0x0101) );
      if ((i+delta.y)<fileCount)
        dBuf.moveStr( 2, files[i+delta.y], getColor(0x0101), (short)size.x, (short)delta.x );
      writeLine( 0, i, (short)size.x, 1, dBuf );
    }
  }

static char *formatFileRow( char buf[128], const find_t &searchRec ) {
    sprintf(buf, "  %8ld %2d-%02d-%02d  %2d:%02d  %c%c%c%c",
      (long) searchRec.size,
      ((searchRec.wr_date & 0x01E0) >> 5),
      (searchRec.wr_date & 0x001F),
      ((searchRec.wr_date >> 9)+1980)%100,
      ((searchRec.wr_time & 0xF800) >> 11)%13,
      ((searchRec.wr_time & 0x07E0) >> 5),
      searchRec.attrib & FA_ARCH   ? 'a' : '\xFA',
      searchRec.attrib & FA_RDONLY ? 'r' : '\xFA',
      searchRec.attrib & FA_SYSTEM ? 's' : '\xFA',
      searchRec.attrib & FA_HIDDEN ? 'h' : '\xFA');
    size_t bufLen = strlen(buf) + 1;
    size_t nameLen, nameWidth;
    TText::scroll( searchRec.name, 18, False, nameLen, nameWidth );
    size_t namePad = 18 - nameWidth;
    char *row = new char[nameLen + namePad + bufLen];
    memcpy( row, searchRec.name, nameLen );
    memset( row + nameLen, ' ', namePad );
    memcpy( row + nameLen + namePad, buf, bufLen );
    return row;
}

void TFilePane::newDir( const char *path ) {
    char searchPath[128] = {0};
    find_t searchRec;
    int result;
    short i;

    deleteFiles();

    ostrstream os( searchPath, sizeof( searchPath )-1 );
    os << path << "*.*" << ends;
    result = _dos_findfirst( searchPath, 0xff, &searchRec );
    while (result==0) {
      if (!(searchRec.attrib & FA_DIREC))
        fileCount++;
      result=_dos_findnext( &searchRec );
    }
    files = new char *[fileCount];
    result = _dos_findfirst( searchPath, 0xff, &searchRec );
    i=0;
    while (result==0) {
      if (!(searchRec.attrib & FA_DIREC))
        files[i++] = formatFileRow( searchPath, searchRec );
      result=_dos_findnext( &searchRec );
    }
    if (fileCount==0)
      setLimit( 1, 1 );
    else
      setLimit( strwidth(files[0]) + 2, fileCount );
    drawView();
}

void TFilePane::deleteFiles() {
    short i;
    for (i=0;i<fileCount;i++)
      delete[] files[i];
    delete[] files;
    fileCount=0;
}

class TDirWindow: public TWindow {
  char *drive;
  TNode *dirTree;
  TDirOutline *ol;
  TFilePane   *fp;
  TScrollBar *hsb, *vsb;
public:
  TDirWindow( const char *driveInit ):
    TWindowInit( TWindow::initFrame ),
    TWindow( TRect( 1,1,76,21 ), driveInit, 0 ) {

    drive = newStr( driveInit );

    vsb = new TScrollBar( TRect( 74,1,75,15 ) );
    hsb = new TScrollBar( TRect( 22,15,73,16 ) );

    fp = new TFilePane( TRect( 21,1,74,15 ), hsb, vsb );
    fp->options |= ofFramed;
    fp->growMode = gfGrowHiY | gfGrowHiX | gfFixed;

    insert( hsb );
    insert( vsb );
    insert( fp );

    vsb = new TScrollBar( TRect( 20,1,21,19 ) );
    hsb = new TScrollBar( TRect( 2,19,19,20 ) );

    QuickMessage *qm = new QuickMessage( drive );
    TProgram::deskTop->insert( qm );

    dirTree = new TNode( drive, getDirList( drive, qm ), 0, True );

    TProgram::deskTop->remove( qm );
    destroy(qm);

    ol = new TDirOutline( TRect( 1,1,20,19 ), hsb, vsb, dirTree );
    ol->options |= ofFramed;
    ol->growMode = gfGrowHiY | gfFixed;
    vsb->growMode = gfGrowHiY;
    hsb->growMode = gfGrowHiY | gfGrowLoY;

    insert( hsb );
    insert( vsb );
    insert( ol );

    char path[128];
    ol->getCurrentPath( path, 128 );
    fp->newDir( path );

  }
  ~TDirWindow() {
    delete[] drive;
  }
  virtual void handleEvent( TEvent &event ) {
    char buffer[128];
    if ((event.what == evCommand) &&
        (event.message.command == cmNewDirFocused )) {
       ol->getCurrentPath(buffer,128);
       fp->newDir(buffer);
       delete[] (char *) title;
       title = newStr(buffer);
       clearEvent(event);
       ((TView *)frame)->drawView();
    }
    TWindow::handleEvent( event );
  }
  virtual void sizeLimits( TPoint &min, TPoint &max ) {
    min.x = 40;
    min.y = 10;
    max = owner->size;
  }
};

class TDirApp : public TApplication
{
   char *drive;

public:

    TDirApp( const char *driveInit );
    ~TDirApp();

    virtual void handleEvent( TEvent& event );
    static TMenuBar *initMenuBar( TRect );
    static TStatusLine *initStatusLine( TRect );
    void aboutBox( void );

};

TDirApp::TDirApp( const char *driveInit ) :
    TProgInit( &TDirApp::initStatusLine,
               &TDirApp::initMenuBar,
               &TDirApp::initDeskTop
             )
{
   drive = newStr(driveInit);
   insertWindow( new TDirWindow( driveInit ) );
}

TDirApp::~TDirApp()
{
    delete[] drive;
}

void TDirApp::handleEvent( TEvent& event )
{
    TApplication::handleEvent( event );
    if( event.what == evCommand )
        {
        switch( event.message.command )
            {
            case cmAbout:
                    aboutBox();
                clearEvent( event );
                break;
            case cmDirTree:
                insertWindow( new TDirWindow( drive ) );
                clearEvent( event );
                break;
            default:
                break;
            }
        }
}

TMenuBar *TDirApp::initMenuBar( TRect r )
{

    r.b.y = r.a.y+1;

    return new TMenuBar( r,
      *new TSubMenu( "~\xF0~", kbAltSpace ) +
        *new TMenuItem( "~A~bout...", cmAbout, kbAltA ) +
      *new TSubMenu( "~F~ile", kbAltF ) +
        *new TMenuItem( "~N~ew Window...", cmDirTree, kbAltN ) +
         newLine() +
        *new TMenuItem( "E~x~it", cmQuit, cmQuit, hcNoContext, "Alt-X" )
        );

}

TStatusLine *TDirApp::initStatusLine( TRect r )
{
    r.a.y = r.b.y-1;
    return new TStatusLine( r,
        *new TStatusDef( 0, 0xFFFF ) +
            *new TStatusItem( "~Alt-X~ Exit", kbAltX, cmQuit ) +
            *new TStatusItem( 0, kbF10, cmMenu )
            );
}

void TDirApp::aboutBox( void ) {
    TDialog *aboutBox = new TDialog(TRect(0, 0, 39, 11), "About");

    aboutBox->insert(
      new TStaticText(TRect(9, 2, 30, 7),
        "\003Outline Viewer Demo\n\n"       // These strings will be
        "\003Copyright (c) 1994\n\n"       // The \003 centers the line.
        "\003Borland International"
        )
      );

    aboutBox->insert(
      new TButton(TRect(14, 8, 25, 10), " OK", cmOK, bfDefault)
      );

    aboutBox->options |= ofCentered;

    executeDialog(aboutBox);

}

int main( int argc, char *argv[] )
{
    TDirApp *dirApp = new TDirApp( argc == 2 ? argv[1] : ".");
    dirApp->run();
    TObject::destroy(dirApp);
    return 0;
}
