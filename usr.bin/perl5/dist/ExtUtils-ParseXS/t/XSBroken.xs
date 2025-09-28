#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

typedef IV MyType3;

MODULE = XSBroken         PACKAGE = XSBroken

PROTOTYPES: ENABLE


TYPEMAP: <<'END'
MyType3	T_BAAR

OUTPUT
T_BAAR
	sv_setiv($arg, (IV)$var);
END

MyType3
typemaptest3(foo)
    MyType3 foo
  CODE:
    RETVAL = foo;
  OUTPUT:
    RETVAL
