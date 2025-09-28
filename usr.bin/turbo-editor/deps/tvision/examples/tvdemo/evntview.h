#ifndef EVNTVIEW_H
#define EVNTVIEW_H

#define Uses_TWindow
#define Uses_TRect
#include <tvision/tv.h>

// TEventViewer: a TTerminal window displaying the attributes of TEvents
// received by the application.
//
// Inspired by TTYWindow from Daniel Ambrose.

class TTerminal;

class TEventViewer : public TWindow
{
    Boolean stopped;
    size_t eventCount;
    ushort bufSize;
    TTerminal *interior;
    TScrollBar *scrollBar;
    ostream *out;

    static const char * const titles[2];

    void init(ushort aBufSize);

    static void printEvent(ostream &out, const TEvent &ev);

public:

    TEventViewer(const TRect &bounds, ushort aBufSize) noexcept;
    TEventViewer(StreamableInit) :
        TWindowInit(0), TWindow(streamableInit) { }
    ~TEventViewer();

    virtual void handleEvent(TEvent &ev);
    virtual void shutDown();

    void toggle();
    void print(const TEvent &ev);

    static const char * const name;
    static TStreamable *build();

private:

    virtual const char *streamableName() const
        { return name; }

    virtual void write( opstream& );
    virtual void *read( ipstream& );
};

#endif
