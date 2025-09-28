#ifdef _MSC_VER
#include <crtdbg.h>
#endif

#include "EXTERN.h"
#include "perl.h"

#ifdef __GNUC__

/* Mingw32 defaults to globing command line 
 * This is inconsistent with other Win32 ports and 
 * seems to cause trouble with passing -DXSVERSION=\"1.6\" 
 * So we turn it off like this, but only when compiling
 * perlmain.c: perlmainst.c is linked into the same executable
 * as win32.c, which also does this, so we mustn't do it twice
 * otherwise we get a multiple definition error.
 */
#ifndef PERLDLL
int _CRT_glob = 0;
#endif

#endif

int
main(int argc, char **argv, char **env)
{
#ifdef _MSC_VER
    /* Arrange for _CrtDumpMemoryLeaks() to be called automatically at program
     * termination when built with CFG = DebugFull. */
    int currentFlag = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
    currentFlag |= _CRTDBG_LEAK_CHECK_DF;
    _CrtSetDbgFlag(currentFlag);

    /* Change this -1 to the allocation number of any reported memory leaks to
     * break on the allocation call that was leaked. */
    _CrtSetBreakAlloc(-1L);
#endif

    return RunPerl(argc, argv, env);
}


