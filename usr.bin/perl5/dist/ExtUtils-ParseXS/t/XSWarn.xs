#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

MODULE = XSWarn		PACKAGE = XSWarn	PREFIX = xswarn_

PROTOTYPES: DISABLE

void
xswarn_nonargs()
# see perl #112776
    SV *sv = sv_2mortal(newSV());
  CODE:
    ;
