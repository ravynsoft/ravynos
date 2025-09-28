#define Uses_TEvent
#define Uses_TKeys
#define Uses_TFrame
#define Uses_TTerminal
#define Uses_TScrollBar
#define Uses_TStreamableClass
#include <tvision/tv.h>
__link(RWindow)

#include <iostream.h>
#include <iomanip.h>
#include <strstrea.h>

#include "tvcmds.h"
#include "evntview.h"

const char * const TEventViewer::name = "TEventViewer";

TStreamable *TEventViewer::build()
{
    return new TEventViewer(streamableInit);
}

TStreamableClass REventViewer( TEventViewer::name,
                               TEventViewer::build,
                               __DELTA(TEventViewer)
                             );

void TEventViewer::write(opstream &os)
{
    // TTerminal does not override the TStreamable methods, so do not
    // store it in the stream.
    title = 0;
    remove(scrollBar);
    remove(interior);
    TWindow::write(os);
    title = titles[stopped];
    insert(scrollBar);
    insert(interior);

    os << bufSize;
}

void *TEventViewer::read(ipstream &is)
{
    TWindow::read(is);

    ushort aBufSize;
    is >> aBufSize;
    init(aBufSize);
    return this;
}

const char * const TEventViewer::titles[2] =
{
    "Event Viewer",
    "Event Viewer (Stopped)",
};

void TEventViewer::toggle()
{
    stopped = Boolean(!stopped);
    title = titles[stopped];
    if (frame)
        frame->drawView();
}

void TEventViewer::print(const TEvent &ev)
{
    if (ev.what != evNothing && !stopped && out)
    {
        lock();
        *out << "Received event #" << ++eventCount << '\n';
        printEvent(*out, ev);
        *out << flush;
        unlock();
    }
}

TEventViewer::TEventViewer(const TRect &bounds, ushort aBufSize) noexcept :
    TWindowInit(&initFrame),
    TWindow(bounds, 0, wnNoNumber)
{
    eventMask |= evBroadcast;
    init(aBufSize);
}

void TEventViewer::init(ushort aBufSize)
{
    stopped = False;
    eventCount = 0;
    bufSize = aBufSize;
    title = titles[stopped];
    scrollBar = standardScrollBar(sbVertical | sbHandleKeyboard);
    interior = new TTerminal( getExtent().grow(-1, -1),
                              0,
                              scrollBar,
                              bufSize );
    insert(interior);
    out = new ostream(interior);
}

void TEventViewer::shutDown()
{
    delete out;
    interior = 0;
    scrollBar = 0;
    out = 0;
    TWindow::shutDown();
}

TEventViewer::~TEventViewer()
{
    title = 0; // So that TWindow doesn't delete it.
}

void TEventViewer::handleEvent(TEvent &ev)
{
    TWindow::handleEvent(ev);
    if (ev.what == evBroadcast && ev.message.command == cmFndEventView)
        clearEvent(ev);
}

static void printConstants(ostream &out, ushort value, void (_FAR &doPrint)(ostream _FAR &, ushort))
{
    out << hex << setfill('0')
        << "0x" << setw(4) << value;
    char buf[256];
    ostrstream os(buf, sizeof(buf));
    doPrint(os, value);
    os << ends;
    if (buf[0] != '0')
        out << " (" << buf << ")";
    out << dec;
}

void TEventViewer::printEvent(ostream &out, const TEvent &ev)
{
    out << "TEvent {\n"
        << "  .what = ";
        printConstants(out, ev.what, printEventCode);
        out << ",\n";
    if (ev.what & evMouse)
    {
        out << "  .mouse = MouseEventType {\n"
            << "    .where = TPoint {\n"
            << "      .x = " << ev.mouse.where.x << "\n"
            << "      .y = " << ev.mouse.where.y << "\n"
            << "    },\n"
            << "    .eventFlags = ";
        printConstants(out, ev.mouse.eventFlags, printMouseEventFlags);
        out << ",\n"
            << "    .controlKeyState = ";
        printConstants(out, ev.mouse.controlKeyState, printControlKeyState);
        out << ",\n"
            << "    .buttons = ";
        printConstants(out, ev.mouse.buttons, printMouseButtonState);
        out << ",\n"
            << "    .wheel = ";
        printConstants(out, ev.mouse.wheel, printMouseWheelState);
        out << "\n"
            << "  }\n";
    }
    if (ev.what & evKeyboard)
    {
        char charCode = ev.keyDown.charScan.charCode;
        out << "  .keyDown = KeyDownEvent {\n"
            << "    .keyCode = ";
        printConstants(out, ev.keyDown.keyCode, printKeyCode);
        out << ",\n"
            << "    .charScan = CharScanType {\n"
            << "      .charCode = " << (int) (uchar) charCode;
        if (charCode)
            out << " ('" << charCode << "')";
        out << ",\n"
            << "      .scanCode = " << (int) (uchar) ev.keyDown.charScan.scanCode << "\n"
            << "    },\n"
            << "    .controlKeyState = ";
        printConstants(out, ev.keyDown.controlKeyState, printControlKeyState);
        out << ",\n"
            << hex
            << "    .text = {";
        Boolean first = True;
        for (int i = 0; i < ev.keyDown.textLength; ++i)
        {
            if (first)
                first = False;
            else
                out << ", ";
            out << "0x" << (int) (uchar) ev.keyDown.text[i];
        }
        out << "},\n"
            << dec
            << "    .textLength = " << (int) ev.keyDown.textLength << "\n"
            << "  }\n";
    }
    if (ev.what & evCommand)
        out << "  .message = MessageEvent {\n"
            << "    .command = " << ev.message.command << ",\n"
            << "    .infoPtr = " << ev.message.infoPtr << "\n"
            << "  }\n";
    out << "}\n";
}
