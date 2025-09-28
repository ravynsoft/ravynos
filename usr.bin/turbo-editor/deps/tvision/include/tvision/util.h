/* ------------------------------------------------------------------------*/
/*                                                                         */
/*   UTIL.H                                                                */
/*                                                                         */
/*   defines various utility functions used throughout Turbo Vision        */
/*                                                                         */
/* ------------------------------------------------------------------------*/
/*
 *      Turbo Vision - Version 2.0
 *
 *      Copyright (c) 1994 by Borland International
 *      All Rights Reserved.
 *
 */

#if !defined( __UTIL_H )
#define __UTIL_H

#include <stdarg.h>

inline constexpr int min( int a, int b )
{
    return a < b ? a : b;
}

inline constexpr int max( int a, int b )
{
    return a > b ? a : b;
}

#if !defined( __MINMAX_DEFINED ) // Also defined in Borland C++'s stdlib.h.
#define __MINMAX_DEFINED
template <class T>
inline constexpr const T& min( const T& a, const T& b )
{
    return a < b ? a : b;
}

template <class T>
inline constexpr const T& max( const T& a, const T& b )
{
    return a > b ? a : b;
}
#endif  // __MINMAX_DEFINED

void fexpand( char *rpath ) noexcept;
void fexpand( char *rpath, const char *relativeTo ) noexcept;

char hotKey( const char *s ) noexcept;
ushort ctrlToArrow( ushort ) noexcept;
char getAltChar( ushort keyCode ) noexcept;
ushort getAltCode( char ch ) noexcept;
char getCtrlChar(ushort) noexcept;
ushort getCtrlCode(uchar) noexcept;

ushort historyCount( uchar id ) noexcept;
const char *historyStr( uchar id, int index ) noexcept;
void historyAdd( uchar id, TStringView ) noexcept;

int cstrlen( TStringView ) noexcept;
int strwidth( TStringView ) noexcept;

class _FAR TView;
void *message( TView *receiver, ushort what, ushort command, void *infoPtr );

class _FAR TPoint;
class _FAR TGroup;
class _FAR TMenu;
class _FAR TMenuItem;
ushort popupMenu(TPoint where, TMenuItem &aMenu, TGroup * = 0);

Boolean lowMemory() noexcept;

char *newStr( TStringView ) noexcept;
char *fmtStr( const char _FAR *format, ... ) noexcept;
char *vfmtStr( const char _FAR *format, va_list args ) noexcept;

Boolean driveValid( char drive ) noexcept;
Boolean isDir( const char *str ) noexcept;
Boolean pathValid( const char *path ) noexcept;
Boolean validFileName( const char *fileName ) noexcept;
void getCurDir( char *dir, char drive=-1 ) noexcept;
Boolean getHomeDir( char *drive, char *dir ) noexcept;
Boolean isWild( const char *f ) noexcept;

size_t strnzcpy( char *dest, TStringView src, size_t n ) noexcept;
size_t strnzcat( char *dest, TStringView src, size_t n ) noexcept;

void printKeyCode(ostream _FAR &, ushort keyCode);
void printControlKeyState(ostream _FAR &, ushort controlKeyState);
void printEventCode(ostream _FAR &, ushort eventCode);
void printMouseButtonState(ostream _FAR &, ushort buttonState);
void printMouseWheelState(ostream _FAR &, ushort wheelState);
void printMouseEventFlags(ostream _FAR &, ushort eventFlags);

#if defined( __BORLANDC__ )

int snprintf( char _FAR *buffer, size_t size, const char _FAR *format, ... );
int vsnprintf( char _FAR *buffer, size_t size, const char _FAR *format,
               void _FAR *arglist );

#elif !defined( _WIN32 )

int stricmp( const char *s1, const char *s2 ) noexcept;
int strnicmp( const char *s1, const char *s2, size_t maxlen ) noexcept;
char *strupr(char *s) noexcept;
char *itoa( int value, char *buffer, int radix ) noexcept;
char *ltoa( long value, char *buffer, int radix ) noexcept;
char *ultoa( ulong value, char *buffer, int radix ) noexcept;

#endif // __BORLANDC__

#endif  // __UTIL_H
