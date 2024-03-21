#ifdef __BORLANDC__
#include <iostream.h>
#else

#ifndef TVISION_COMPAT_IOSTREAM_H
#define TVISION_COMPAT_IOSTREAM_H

#include <iostream>
#include <ios>
#include <streambuf>
#include <ostream>

using std::cerr;
using std::cin;
using std::cout;
using std::dec;
using std::endl;
using std::flush;
using std::hex;
using std::ios;
using std::ostream;
using std::streambuf;
using std::streamoff;
using std::streampos;

#endif // TVISION_COMPAT_IOSTREAM_H

#endif // __BORLANDC__
