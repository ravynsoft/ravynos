/* This code is compiled twice, once with -DPERL_CORE defined, once without */

#include "EXTERN.h"
#include "perl.h"

#ifdef PERL_CORE
#  define SUFFIX core
#else
#  define SUFFIX notcore
#endif

bool
CAT2(sv_setsv_cow_hashkey_, SUFFIX) () {
    dTHX;
    SV *source = newSVpvn_share("pie", 3, 0);
    SV *destination = newSV(0);
    bool result;

    if(!SvIsCOW(source)) {
	SvREFCNT_dec(source);
	Perl_croak(aTHX_ "Creating a shared hash key scalar failed when "
	       STRINGIFY(SUFFIX) " got flags %" UVxf, (UV)SvFLAGS(source));
    }

    sv_setsv(destination, source);

    result = cBOOL(SvIsCOW(destination));

    SvREFCNT_dec(source);
    SvREFCNT_dec(destination);

    return result;
}

/*
 * Local variables:
 * mode: c
 * End:
 *
 * ex: set ts=8 sts=4 sw=4 et:
 */
