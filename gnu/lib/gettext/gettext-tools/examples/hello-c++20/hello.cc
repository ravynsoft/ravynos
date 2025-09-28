// Example for use of GNU gettext.
// This file is in the public domain.

// Source code of the ISO C++ 20 program.


#include <format>
#include <iostream>
using namespace std;

// Get setlocale() declaration.
#include <locale.h>

// Get getpid() declaration.
#if defined _WIN32 && !defined __CYGWIN__
/* native Windows API */
# include <process.h>
# define getpid _getpid
#else
/* POSIX API */
# include <unistd.h>
#endif

// Get gettext(), textdomain(), bindtextdomain() declaration.
#include "gettext.h"
// Define shortcut for gettext().
#define _(string) gettext (string)

int
main ()
{
  setlocale (LC_ALL, "");
  textdomain ("hello-c++20");
  bindtextdomain ("hello-c++20", LOCALEDIR);

  cout << _("Hello, world!") << endl;
  cout << vformat (_("This program is running as process number {:d}."),
                   make_format_args (getpid ()))
       << endl;
}
