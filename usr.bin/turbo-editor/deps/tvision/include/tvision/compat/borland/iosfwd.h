#ifndef TVISION_IOSFWD_H
#define TVISION_IOSFWD_H

#ifdef __BORLANDC__

#include <_defs.h>

class _EXPCLASS ostream;
class _EXPCLASS streambuf;
typedef long streampos;
typedef long streamoff;

#else

#include <iosfwd>

using std::ostream;
using std::streambuf;
using std::streampos;
using std::streamoff;

#endif // __BORLANDC__

#endif // TVISION_IOSFWD_H
