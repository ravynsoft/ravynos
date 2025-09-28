#define PERL_NO_GET_CONTEXT

#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#define MY_CXT_KEY "File::Glob::_guts" XS_VERSION

typedef struct {
    HV *		x_DG_ENTRIES;
    Perl_ophook_t	x_DG_OLD_OPHOOK;
} my_cxt_t;

START_MY_CXT

static void
glob_ophook(pTHX_ OP *o)
{
  if (PL_dirty) return;
  {
    dMY_CXT;
    if (!MY_CXT.x_DG_ENTRIES)
	MY_CXT.x_DG_ENTRIES = get_hv("File::DosGlob::entries", 0);
    if (MY_CXT.x_DG_ENTRIES)
	(void)hv_delete(MY_CXT.x_DG_ENTRIES, (char *)&o, sizeof(OP *),G_DISCARD);
    if (MY_CXT.x_DG_OLD_OPHOOK) MY_CXT.x_DG_OLD_OPHOOK(aTHX_ o);
  }
}

MODULE = File::DosGlob		PACKAGE = File::DosGlob

PROTOTYPES: DISABLE

BOOT:
{
    MY_CXT_INIT;
    {
	dMY_CXT;
	MY_CXT.x_DG_ENTRIES = NULL;
	MY_CXT.x_DG_OLD_OPHOOK = PL_opfreehook;
	PL_opfreehook = glob_ophook;
    }
}

SV *
_callsite(...)
    CODE:
        PERL_UNUSED_VAR(items);
	RETVAL = newSVpvn(
		   (char *)&cxstack[cxstack_ix].blk_sub.retop, sizeof(OP *)
		 );
    OUTPUT:
	RETVAL
