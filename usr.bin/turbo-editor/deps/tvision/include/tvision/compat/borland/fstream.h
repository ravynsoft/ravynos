#ifdef __BORLANDC__
#include <fstream.h>
#else

#ifndef TVISION_COMPAT_FSTREAM_H
#define TVISION_COMPAT_FSTREAM_H

#include "iostream.h"
#include <fstream>

using std::filebuf;
using std::fstream;
using std::ifstream;
using std::ofstream;

#endif // TVISION_COMPAT_FSTREAM_H

#endif // __BORLANDC__
