/* ------------------------------------------------------------------------*/
/*                                                                         */
/*   HARDWARE.H                                                            */
/*                                                                         */
/*   defines the class THardwareInfo                                       */
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

#if defined( Uses_THardwareInfo ) && !defined( __THardwareInfo )
#define __THardwareInfo

#if defined( __FLAT__ )

#if !defined( __WINDOWS_H )
#include <tvision/compat/windows/windows.h>
#endif

#else

#if !defined( MAKELONG )
#define MAKELONG(h,l) \
    ((long)(((unsigned)(l)) | (((long)((unsigned)(h))) << 16)))
#endif

#endif

struct TEvent;
struct MouseEventType;

class THardwareInfo
{

public:

    THardwareInfo() noexcept;
    ~THardwareInfo();

    static uint32_t getTickCount() noexcept;

#if defined( __FLAT__ )

    enum ConsoleType { cnInput = 0, cnOutput = 1, cnStartup = 2 };
    enum PlatformType { plDPMI32 = 1, plWinNT = 2, plOS2 = 4 };

    static PlatformType getPlatform() noexcept;

// Caret functions.

    static void setCaretSize( ushort size ) noexcept;
    static ushort getCaretSize() noexcept;
    static void setCaretPosition( ushort x, ushort y ) noexcept;
    static BOOL isCaretVisible() noexcept;

// Screen functions.

    static ushort getScreenRows() noexcept;
    static ushort getScreenCols() noexcept;
    static ushort getScreenMode() noexcept;
    static void setScreenMode( ushort mode ) noexcept;
    static void clearScreen( ushort w, ushort h ) noexcept;
    static void flushScreen() noexcept;
    static void screenWrite( ushort x, ushort y, TScreenCell *buf, DWORD len ) noexcept;
    static TScreenCell *allocateScreenBuffer() noexcept;
    static void freeScreenBuffer( TScreenCell *buffer ) noexcept;
    static void setUpConsole() noexcept;
    static void restoreConsole() noexcept;

// Mouse functions.

    static DWORD getButtonCount() noexcept;
    static void cursorOn() noexcept;
    static void cursorOff() noexcept;

// Event functions.

    static BOOL getMouseEvent( MouseEventType& event ) noexcept;
    static BOOL getKeyEvent( TEvent& event ) noexcept;
    static void clearPendingEvent() noexcept;
    static void waitForEvent( int timeoutMs ) noexcept;
    static void interruptEventWait() noexcept;
    static BOOL setClipboardText( TStringView text ) noexcept;
    static BOOL requestClipboardText( void (&accept)( TStringView ) ) noexcept;

// System functions.

    static BOOL setCtrlBrkHandler( BOOL install ) noexcept;
    static BOOL setCritErrorHandler( BOOL install ) noexcept;

    static const ushort NormalCvt[89];
    static const ushort ShiftCvt[89];
    static const ushort CtrlCvt[89];
    static const ushort AltCvt[89];

private:

    static BOOL __stdcall ctrlBreakHandler( DWORD dwCtrlType ) noexcept;

    static BOOL insertState;
    static PlatformType platform;
    static HANDLE consoleHandle[3];
    static DWORD consoleMode;
    static DWORD pendingEvent;
    static INPUT_RECORD irBuffer;
    static CONSOLE_CURSOR_INFO crInfo;
    static CONSOLE_SCREEN_BUFFER_INFO sbInfo;

#ifndef __BORLANDC__
    enum { eventQSize = 24 };
    static TEvent eventQ[eventQSize];
    static size_t eventCount;
    static BOOL getPendingEvent(TEvent &event, Boolean mouse) noexcept;
    static void readEvents() noexcept;
#endif

#else

    static ushort *getColorAddr( ushort offset = 0 );
    static ushort *getMonoAddr( ushort offset = 0 );
    static uchar getShiftState();
    static uchar getBiosScreenRows();
    static uchar getBiosVideoInfo();
    static void setBiosVideoInfo( uchar info );
    static ushort getBiosEquipmentFlag();
    static ushort huge getBiosEquipmentFlag(int);   // Non-inline version.
    static void setBiosEquipmentFlag( ushort flag );
    static Boolean getDPMIFlag();

private:

    static ushort huge getBiosSelector();   // For SYSINT.ASM.

    static Boolean dpmiFlag;
    static ushort colorSel;
    static ushort monoSel;
    static ushort biosSel;

#endif

};

#if defined( __FLAT__ )

inline THardwareInfo::PlatformType THardwareInfo::getPlatform() noexcept
{
    return platform;
}

#ifdef __BORLANDC__
// Caret functions.

inline ushort THardwareInfo::getCaretSize() noexcept
{
    return crInfo.dwSize;
}

inline BOOL THardwareInfo::isCaretVisible() noexcept
{
    return crInfo.bVisible;
}


// Screen functions.

inline ushort THardwareInfo::getScreenRows() noexcept
{
    return sbInfo.dwSize.Y;
}

inline ushort THardwareInfo::getScreenCols() noexcept
{
    return sbInfo.dwSize.X;
}

#pragma option -w-inl

inline void THardwareInfo::clearScreen( ushort w, ushort h ) noexcept
{
    COORD coord = { 0, 0 };
    DWORD read;

    FillConsoleOutputAttribute( consoleHandle[cnOutput], 0x07, w*h, coord, &read );
    FillConsoleOutputCharacterA( consoleHandle[cnOutput], ' ', w*h, coord, &read );
}

#pragma option -w+inl

inline void THardwareInfo::flushScreen() noexcept
{
}

inline TScreenCell *THardwareInfo::allocateScreenBuffer() noexcept
{
    GetConsoleScreenBufferInfo( consoleHandle[cnOutput], &sbInfo );
    short x = sbInfo.dwSize.X, y = sbInfo.dwSize.Y;

    if( x < 80 )        // Make sure we allocate at least enough for
        x = 80;         //   a 80x50 screen.
    if( y < 50 )
        y = 50;

    return (TScreenCell *) VirtualAlloc( 0, x * y * 4, MEM_COMMIT, PAGE_READWRITE );
}

inline void THardwareInfo::freeScreenBuffer( TScreenCell *buffer ) noexcept
{
    VirtualFree( buffer, 0, MEM_RELEASE );
}


// Mouse functions.

inline DWORD THardwareInfo::getButtonCount()
{
    DWORD num;
    GetNumberOfConsoleMouseButtons(&num);
    return num;
}

inline void THardwareInfo::cursorOn()
{
    SetConsoleMode( consoleHandle[cnInput], consoleMode | ENABLE_MOUSE_INPUT );
}

inline void THardwareInfo::cursorOff()
{
    SetConsoleMode( consoleHandle[cnInput], consoleMode & ~ENABLE_MOUSE_INPUT );
}
#endif // __BORLANDC__


// Event functions.

inline void THardwareInfo::clearPendingEvent() noexcept
{
    pendingEvent = 0;
}


// System functions.

inline BOOL THardwareInfo::setCtrlBrkHandler( BOOL install ) noexcept
{
#ifdef _WIN32
    return SetConsoleCtrlHandler( &THardwareInfo::ctrlBreakHandler, install );
#else
    (void) install;
    return TRUE;
#endif
}

inline BOOL THardwareInfo::setCritErrorHandler( BOOL install ) noexcept
{
    (void) install;
    return TRUE;        // Handled by NT or DPMI32..
}


#else

inline ushort *THardwareInfo::getColorAddr( ushort offset )
    { return (ushort *) MAKELONG( colorSel, offset ); }

inline ushort *THardwareInfo::getMonoAddr( ushort offset )
    { return (ushort *) MAKELONG( monoSel, offset ); }

inline uint32_t THardwareInfo::getTickCount()
    { return *(uint32_t *) MAKELONG( biosSel, 0x6C ); }

inline uchar THardwareInfo::getShiftState()
    { return *(uchar *) MAKELONG( biosSel, 0x17 ); }


inline uchar THardwareInfo::getBiosScreenRows()
    { return *(uchar *) MAKELONG( biosSel, 0x84 ); }

inline uchar THardwareInfo::getBiosVideoInfo()
    { return *(uchar *) MAKELONG( biosSel, 0x87 ); }

inline void THardwareInfo::setBiosVideoInfo( uchar info )
    { *(uchar *) MAKELONG( biosSel, 0x87 ) = info; }

inline ushort THardwareInfo::getBiosEquipmentFlag()
    { return *(ushort *) MAKELONG( biosSel, 0x10 ); }

inline void THardwareInfo::setBiosEquipmentFlag( ushort flag )
    { *(ushort *) MAKELONG( biosSel, 0x10 ) = flag; }

inline Boolean THardwareInfo::getDPMIFlag()
    { return dpmiFlag; }

#endif

#endif  // __THardwareInfo

#if defined( __BORLANDC__ )
#pragma option -Vo.
#endif
#if defined( __BCOPT__ ) && !defined (__FLAT__)
#pragma option -po.
#endif
