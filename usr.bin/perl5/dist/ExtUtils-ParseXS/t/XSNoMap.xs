#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

typedef struct { int a; } S;

static void
xsnomap_unknown(S* p) {
}

MODULE = XSNoMap		PACKAGE = XSNoMap	PREFIX = xsnomap_

PROTOTYPES: DISABLE

void
xsnomap_unknown(S *arg)
