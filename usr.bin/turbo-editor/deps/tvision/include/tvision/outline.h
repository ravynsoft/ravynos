/* ------------------------------------------------------------------------*/
/*                                                                         */
/*   OUTLINE.H                                                             */
/*                                                                         */
/*   defines the classes TOutline, and TOutlineViewer.                     */
/*                                                                         */
/* ------------------------------------------------------------------------*/
/*
 *      Turbo Vision - Version 2.0
 *
 *      Copyright (c) 1994 by Borland International
 *      All Rights Reserved.
 *
 */

#if defined( __BORLANDC__ )
#pragma option -Vo-
#endif
#if defined( __BCOPT__ ) && !defined (__FLAT__)
#pragma option -po-
#endif

#if defined( Uses_TOutlineViewer ) && !defined( __TOutlineViewer )
#define __TOutlineViewer

const int
  ovExpanded = 0x01,
  ovChildren = 0x02,
  ovLast     = 0x04;

const int
  cmOutlineItemSelected = 301;

class TNode
{
public:
    TNode(TStringView aText) noexcept;
    TNode(TStringView aText, TNode* aChildren, TNode* aNext, Boolean initialState = True) noexcept;
    virtual ~TNode();

    TNode* next;
    const char* text;
    TNode* childList;
    Boolean expanded;
};

inline TNode::TNode(TStringView aText) noexcept :
    next(0), text(newStr(aText)), childList(0), expanded(True)
{
}

inline TNode::TNode( TStringView aText, TNode* aChildren,
                     TNode* aNext, Boolean initialState ) noexcept :
    next(aNext), text(newStr(aText)),
    childList(aChildren), expanded(initialState)
{
}

inline TNode::~TNode() {
  delete [] (char *) text;
}

/* ------------------------------------------------------------------------*/
/*      class TOutlineViewer                                               */
/*                                                                         */
/*      Palette layout                                                     */
/*        1 = Normal color                                                 */
/*        2 = Focus color                                                  */
/*        3 = Select color                                                 */
/*        4 = Not expanded color                                           */
/* ------------------------------------------------------------------------*/

class _FAR TRect;
class _FAR TScrollBar;
class _FAR TOutlineViewer;
struct _FAR TEvent;

// Callback types for TOutlineViewer's traverse functions.

typedef Boolean (*TOutlineVisitor)( TOutlineViewer*, TNode*,
                                    int, int, long, ushort, void* );
typedef Boolean (*TOutlineVisitorNoArg)( TOutlineViewer*, TNode*,
                                         int, int, long, ushort );

class TOutlineViewer : public TScroller
{
public:
    TOutlineViewer(const TRect& bounds, TScrollBar* aHScrollBar,
        TScrollBar* aVScrollBar) noexcept;
    TOutlineViewer(StreamableInit s) noexcept;
    virtual void adjust(TNode* node, Boolean expand)=0;
    virtual void draw();
    virtual void focused(int i);
    virtual TNode* getNext(TNode* node)=0;
    virtual TNode* getChild(TNode* node, int i)=0;
    virtual char* getGraph(int level, long lines, ushort flags);
    virtual int getNumChildren(TNode* node)=0;
    virtual TNode* getNode(int i);
    virtual TPalette& getPalette() const;
    virtual TNode* getRoot()=0;
    virtual const char* getText(TNode* node)=0;
    virtual void handleEvent(TEvent& event);
    virtual Boolean hasChildren(TNode* node)=0;
    virtual Boolean isExpanded(TNode* node)=0;
    virtual Boolean isSelected(int i);
    virtual void selected(int i);
    virtual void setState(ushort aState, Boolean enable);

    void update() noexcept;
    void expandAll(TNode* node);

    TNode* firstThat(TOutlineVisitor test, void* arg) noexcept;
    TNode* firstThat(TOutlineVisitorNoArg test) noexcept;

    TNode* forEach(TOutlineVisitor action, void* arg) noexcept;
    TNode* forEach(TOutlineVisitorNoArg action) noexcept;

    char* createGraph(int level, long lines, ushort flags, int levWidth,
        int endWidth, const char* chars) noexcept;

    int foc;

protected:
    static void disposeNode(TNode* node) noexcept;
    virtual void write( opstream& );
    virtual void *read( ipstream& );

public:
    static TStreamable *build();
    static const char * const _NEAR name;

private:
    void adjustFocus(int newFocus) noexcept;
    TNode* iterate(TOutlineVisitor action, void *arg, Boolean checkResult) noexcept;
};

inline TOutlineViewer::TOutlineViewer( StreamableInit s) noexcept :
    TScroller(s)
{
}

#endif // Uses_TOutlineViewer

#if defined( Uses_TOutline ) && !defined( __TOutline )
#define __TOutline

/* ------------------------------------------------------------------------*/
/*      class TOutline                                                     */
/*                                                                         */
/*      Palette layout                                                     */
/*        1 = Normal color                                                 */
/*        2 = Focus color                                                  */
/*        3 = Select color                                                 */
/*        4 = Not expanded color                                           */
/* ------------------------------------------------------------------------*/

class _FAR TRect;
class _FAR TScrollBar;
struct _FAR TEvent;

class TOutline : public TOutlineViewer
{
public:
    TOutline(const TRect& bounds, TScrollBar* aHScrollBar, TScrollBar* aVScrollBar,
        TNode* aRoot) noexcept;
    ~TOutline();

    virtual void adjust(TNode* node, Boolean expand);
    virtual TNode* getRoot();
    virtual int getNumChildren(TNode* node);
    virtual TNode* getNext(TNode *node);
    virtual TNode* getChild(TNode* node, int i);
    virtual const char* getText(TNode* node);
    virtual Boolean isExpanded(TNode* node);
    virtual Boolean hasChildren(TNode* node);

    TNode* root;

protected:
    virtual void write( opstream& );
    virtual void* read( ipstream& );
    virtual void writeNode( TNode*, opstream& );
    virtual TNode* readNode( ipstream& );
    TOutline( StreamableInit ) noexcept;

public:
    static TStreamable* build();
    static const char* const _NEAR name;

private:
    virtual const char *streamableName() const
        { return name; }

};

inline TOutline::TOutline( StreamableInit s ) noexcept : TOutlineViewer( s )
{
}

inline ipstream& operator >> ( ipstream& is, TOutline& o )
    { return is >> (TStreamable&)o; }
inline ipstream& operator >> ( ipstream& is, TOutline*& o )
    { return is >> (void *&)o; }

inline opstream& operator << ( opstream& os, TOutline& o )
    { return os << (TStreamable&)o; }
inline opstream& operator << ( opstream& os, TOutline* o )
    { return os << (TStreamable*)o; }

#endif // Uses_TOutline

#if defined( __BORLANDC__ )
#pragma option -Vo.
#endif
#if defined( __BCOPT__ ) && !defined (__FLAT__)
#pragma option -po.
#endif
