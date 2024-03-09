#ifdef __BORLANDC__
#include <strstrea.h>
#else

#ifndef TVISION_COMPAT_STRSTREA_H
#define TVISION_COMPAT_STRSTREA_H

#include <strstream>
#include <ostream>

using std::ends;
using std::istrstream;
using std::ostrstream;
using std::strstream;
using std::strstreambuf;

#endif // TVISION_COMPAT_STRSTREA_H

#endif // __BORLANDC__
