#include <stdio.h>
#include <stdbool.h>

#include "test_support.h"
#include "base.h"

static bool         wasProblem        = false;
static const char*	coal1Where        = NULL;
static int*			coal1Addr         = NULL;
static int			checkInCountCoal1 = 0;

void baseVerifyCoal1(const char* where, int* addr)
{
    LOG("baseVerifyCoal1(%s, %p)", where, addr);
    ++checkInCountCoal1;
    if ( coal1Where == NULL ) {
        coal1Where = where;
        coal1Addr = addr;
    }
    else {
        if ( addr != coal1Addr ) {
            LOG("coal1 resolved to different locations.  %p in %s and %p in %s",
                coal1Addr, coal1Where, addr, where);
            wasProblem = true;
        }
    }
}


static const char*	coal2Where        = NULL;
static int*			coal2Addr         = NULL;
static int			checkInCountCoal2 = 0;

void baseVerifyCoal2(const char* where, int* addr)
{
    LOG("baseVerifyCoal2(%s, %p)", where, addr);
    ++checkInCountCoal2;
    if ( coal2Where == NULL ) {
        coal2Where = where;
        coal2Addr = addr;
    }
    else {
        if ( addr != coal2Addr ) {
            LOG("coal2 resolved to different locations.  %p in %s and %p in %s",
                coal2Addr, coal2Where, addr, where);
            wasProblem = true;
        }
    }
}



void baseCheck()
{
    if ( wasProblem )
        FAIL("was problem");
    else if ( checkInCountCoal1 != 4 )
        FAIL("checkInCountCoal1 != 4");
    else if ( checkInCountCoal2 != 4 )
        FAIL("checkInCountCoal2 != 2");
    else
        PASS("Success");
}

