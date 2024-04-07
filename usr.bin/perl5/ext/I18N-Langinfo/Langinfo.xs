#define PERL_NO_GET_CONTEXT
#define PERL_EXT
#define PERL_EXT_LANGINFO

#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#ifdef I_LANGINFO
#   define __USE_GNU 1 /* Enables YESSTR, otherwise only __YESSTR. */
#   include <langinfo.h>
#else
#   include <perl_langinfo.h>
#endif

#include "const-c.inc"

MODULE = I18N::Langinfo	PACKAGE = I18N::Langinfo

PROTOTYPES: ENABLE

INCLUDE: const-xs.inc

SV*
langinfo(code)
	int	code
  PREINIT:
        const char * value;
        utf8ness_t   is_utf8;
  PROTOTYPE: _
  CODE:
#ifdef HAS_NL_LANGINFO
	if (code < 0) {
	    SETERRNO(EINVAL, LIB_INVARG);
	    RETVAL = &PL_sv_undef;
	} else
#endif
        {
            value = Perl_langinfo8(code, &is_utf8);
            RETVAL = newSVpvn_utf8(value, strlen(value), is_utf8 == UTF8NESS_YES);
        }

  OUTPUT:
        RETVAL
