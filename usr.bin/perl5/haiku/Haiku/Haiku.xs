#define PERL_NO_GET_CONTEXT
#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include <stdarg.h>

#include <OS.h>

static void
haiku_do_debugger(const char* format,...)
{
    char buffer[1024];
    va_list args;
    va_start(args, format);
    my_vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    debugger(buffer);
}

static void
haiku_do_debug_printf(pTHX_ SV *sv,
    void (*printfFunc)(const char*,...))
{

    if (!sv)
	return;
    if (SvTYPE(sv) == SVt_IV && SvIOK(sv)) {
	assert(!SvGMAGICAL(sv));
	if (SvIsUV(sv))
	    (*printfFunc)("%"UVuf, (UV)SvUVX(sv));
	else
	    (*printfFunc)("%"IVdf, (IV)SvIVX(sv));
	return;
    }
    else {
	STRLEN len;
	/* Do this first to trigger any overloading.  */
	const char *tmps = SvPV_const(sv, len);
	U8 *tmpbuf = NULL;

	if (!SvUTF8(sv)) {
	    /* We don't modify the original scalar.  */
	    tmpbuf = bytes_to_utf8((const U8*) tmps, &len);
	    tmps = (char *) tmpbuf;
	}

	if (len)
	    (*printfFunc)("%.*s", (int)len, tmps);
	Safefree(tmpbuf);
    }
}

XS(haiku_debug_printf)
{
    dXSARGS;
    dORIGMARK;
    SV *sv;

    if (items < 1)
	Perl_croak(aTHX_ "usage: Haiku::debug_printf($format,...)");

    sv = newSV(0);

    if (SvTAINTED(MARK[1]))
	TAINT_PROPER("debug_printf");
    do_sprintf(sv, SP - MARK, MARK + 1);

    haiku_do_debug_printf(sv, &debug_printf);

    SvREFCNT_dec(sv);
    SP = ORIGMARK;
    PUSHs(&PL_sv_yes);
}

XS(haiku_ktrace_printf)
{
    dXSARGS;
    dORIGMARK;
    SV *sv;

    if (items < 1)
	Perl_croak(aTHX_ "usage: Haiku::debug_printf($format,...)");

    sv = newSV(0);

    if (SvTAINTED(MARK[1]))
	TAINT_PROPER("ktrace_printf");
    do_sprintf(sv, SP - MARK, MARK + 1);

    haiku_do_debug_printf(sv, &ktrace_printf);

    SvREFCNT_dec(sv);
    SP = ORIGMARK;
    PUSHs(&PL_sv_yes);
}

XS(haiku_debugger)
{
    dXSARGS;
    dORIGMARK;
    SV *sv;

    if (items < 1)
	Perl_croak(aTHX_ "usage: Haiku::debugger($format,...)");

    sv = newSV(0);

    if (SvTAINTED(MARK[1]))
	TAINT_PROPER("debugger");
    do_sprintf(sv, SP - MARK, MARK + 1);

    haiku_do_debug_printf(sv, &haiku_do_debugger);

    SvREFCNT_dec(sv);
    SP = ORIGMARK;
    PUSHs(&PL_sv_yes);
}

MODULE = Haiku            PACKAGE = Haiku

PROTOTYPES: DISABLE

BOOT:
{
    char *file = __FILE__;

    newXS("Haiku::debug_printf", haiku_debug_printf, file);
    newXS("Haiku::ktrace_printf", haiku_ktrace_printf, file);
    newXS("Haiku::debugger", haiku_debugger, file);
    XSRETURN_YES;
}
