#include <stddef.h>
#include <stdbool.h>

#include "test_support.h"

static bool mainCalled           = false;
static bool libCalled            = false;
static bool libCalledBeforeMain  = false;

void mainTerminated()
{
    mainCalled = true;
}

void libDynamicTerminated()
{
    libCalled = true;
    if ( !mainCalled )
        libCalledBeforeMain = true;
}


static __attribute__((destructor))
void myTerm()
{
    if ( !mainCalled )
        FAIL("main's terminator not called");
    else if ( !libCalled )
        FAIL("libDynamic's terminator not called");
    else if ( !libCalledBeforeMain )
        FAIL("libDynamic's terminator called out of order");
    else
        PASS("Success");
}

