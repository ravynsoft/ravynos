#include "EXTERN.h"
#include "perl.h"

/* We have to be in a different .xs so that we can do this:  */

#undef XS_VERSION
#include "XSUB.h"

/* This can't be "MODULE = XS::APItest" as then we get duplicate bootstraps.  */
MODULE = XS::APItest::XSUB	PACKAGE = XS::APItest::XSUB

PROTOTYPES: DISABLE

EXPORT_XSUB_SYMBOLS: ENABLE

void
XS_VERSION_undef(...)
    PPCODE:
        XS_VERSION_BOOTCHECK;
        XSRETURN_EMPTY;
