#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

void
xstest_something (char * some_thing)
{
	some_thing = some_thing;
}

void
xstest_something2 (char * some_thing)
{
	some_thing = some_thing;
}


MODULE = XSTest         PACKAGE = XSTest	PREFIX = xstest_

PROTOTYPES: DISABLE

int
is_even(input)
	int     input
    CODE:
	RETVAL = (input % 2 == 0);
    OUTPUT:
	RETVAL

void
xstest_something (myclass, some_thing)
	char * some_thing
    C_ARGS:
	some_thing

void
xstest_something2 (some_thing)
	char * some_thing

void
xstest_something3 (myclass, some_thing)
	SV   * myclass
	char * some_thing
    PREINIT:
    	int i = 0;
    PPCODE:
    	/* it's up to us clear these warnings */
	myclass = myclass;
	some_thing = some_thing;
	i = i;
	XSRETURN_UNDEF;
	
int
consts (myclass)
	SV * myclass
    ALIAS:
	const_one = 1
	const_two = 2
	const_three = 3
    CODE:
    	/* it's up to us clear these warnings */
    	myclass = myclass;
	ix = ix;
    	RETVAL = 1;
    OUTPUT:
	RETVAL

bool
T_BOOL(in)
        bool in
    CODE:
        RETVAL = in;
    OUTPUT: RETVAL

bool
T_BOOL_2(in)
        bool in
    CODE:
	PERL_UNUSED_VAR(RETVAL);
    OUTPUT: in

void
T_BOOL_OUT( out, in )
        bool out
        bool in
    CODE:
        out = in;
    OUTPUT: out
