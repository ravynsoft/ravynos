#include "EXTERN.h"
#include "perl.h"

/* We have to be in a different .xs so that we can do this:  */

#undef XS_VERSION
#define XS_VERSION " "
#undef PERL_API_VERSION_STRING
#define PERL_API_VERSION_STRING "1.0.16"
#include "XSUB.h"

/* This can't be "MODULE = XS::APItest" as then we get duplicate bootstraps.  */
MODULE = XS::APItest::XSUB1	PACKAGE = XS::APItest::XSUB

PROTOTYPES: DISABLE

EXPORT_XSUB_SYMBOLS: ENABLE

void
XS_VERSION_empty(...)
    PPCODE:
        XS_VERSION_BOOTCHECK;
        XSRETURN_EMPTY;

void
XS_APIVERSION_invalid(...)
    PPCODE:
        XS_APIVERSION_BOOTCHECK;
        XSRETURN_EMPTY;
