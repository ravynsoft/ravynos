#ifdef __BORLANDC__
#include <iomanip.h>
#else

#ifndef TVISION_COMPAT_IOMANIP_H
#define TVISION_COMPAT_IOMANIP_H

#include <iomanip>

using std::resetiosflags;
using std::setiosflags;
using std::setbase;
using std::setfill;
using std::setprecision;
using std::setw;

#endif // TVISION_COMPAT_IOMANIP_H

#endif // __BORLANDC__
