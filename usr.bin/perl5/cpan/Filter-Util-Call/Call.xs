/* 
 * Filename : Call.xs
 * 
 * Author   : Reini Urban
 * Date     : Di 16. Aug 7:59:10 CEST 2022
 * Version  : 1.64
 *
 *    Copyright (c) 1995-2011 Paul Marquess. All rights reserved.
 *    Copyright (c) 2011-2014, 2018 Reini Urban. All rights reserved.
 *       This program is free software; you can redistribute it and/or
 *              modify it under the same terms as Perl itself.
 *
 */

#define PERL_NO_GET_CONTEXT
#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"
#ifdef _NOT_CORE
#  include "ppport.h"
#endif

/* Internal defines */
#define PERL_MODULE(s)		IoBOTTOM_NAME(s)
#define PERL_OBJECT(s)		IoTOP_GV(s)
#define FILTER_ACTIVE(s)	IoLINES(s)
#define BUF_OFFSET(sv)  	IoPAGE_LEN(sv)
#define CODE_REF(sv)  		IoPAGE(sv)
#ifndef PERL_FILTER_EXISTS
#  define PERL_FILTER_EXISTS(i) (PL_rsfp_filters && (i) <= av_len(PL_rsfp_filters))
#endif

#define SET_LEN(sv,len) \
        do { SvPVX(sv)[len] = '\0'; SvCUR_set(sv, len); } while (0)


/* Global Data */

#define MY_CXT_KEY "Filter::Util::Call::_guts" XS_VERSION
 
typedef struct {
    int x_fdebug ;
    int x_current_idx ;
} my_cxt_t;
 
START_MY_CXT
 
#define fdebug          (MY_CXT.x_fdebug)
#define current_idx     (MY_CXT.x_current_idx)


static I32
filter_call(pTHX_ int idx, SV *buf_sv, int maxlen)
{
    dMY_CXT;
    SV   *my_sv = FILTER_DATA(idx);
    const char *nl = "\n";
    char *p;
    char *out_ptr;
    int n;

    if (fdebug)
	warn("**** In filter_call - maxlen = %d, out len buf = %" IVdf " idx = %d my_sv = %" IVdf " [%s]\n",
             maxlen, (IV)SvCUR(buf_sv), idx, (IV)SvCUR(my_sv), SvPVX(my_sv) ) ;

    while (1) {

	/* anything left from last time */

        if ((n = SvCUR(my_sv))) {
            assert(SvCUR(my_sv) < PERL_INT_MAX) ;

	    out_ptr = SvPVX(my_sv) + BUF_OFFSET(my_sv) ;

	    if (maxlen) { 
		/* want a block */ 
		if (fdebug)
		    warn("BLOCK(%d): size = %d, maxlen = %d\n", 
			idx, n, maxlen) ;

	        sv_catpvn(buf_sv, out_ptr, maxlen > n ? n : maxlen );
		if(n <= maxlen) {
		    BUF_OFFSET(my_sv) = 0 ;
	            SET_LEN(my_sv, 0) ;
		}
		else {
		    BUF_OFFSET(my_sv) += maxlen ;
	            SvCUR_set(my_sv, n - maxlen) ;
		}
	        return SvCUR(buf_sv);
	    }
	    else {
		/* want lines */
                if ((p = ninstr(out_ptr, out_ptr + n, nl, nl + 1))) {

	            sv_catpvn(buf_sv, out_ptr, p - out_ptr + 1);

	            n = n - (p - out_ptr + 1);
		    BUF_OFFSET(my_sv) += (p - out_ptr + 1);
	            SvCUR_set(my_sv, n) ;
	            if (fdebug)
		        warn("recycle %d - leaving %d, returning %" IVdf " [%s]",
                             idx, n, (IV)SvCUR(buf_sv), SvPVX(buf_sv)) ;

	            return SvCUR(buf_sv);
	        }
	        else /* no EOL, so append the complete buffer */
	            sv_catpvn(buf_sv, out_ptr, n) ;
	    }
	    
	}


	SET_LEN(my_sv, 0) ;
	BUF_OFFSET(my_sv) = 0 ;

	if (FILTER_ACTIVE(my_sv))
	{
    	    dSP ;
    	    int count ;

            if (fdebug)
		warn("gonna call %s::filter\n", PERL_MODULE(my_sv)) ;

    	    ENTER ;
    	    SAVETMPS;
	
	    SAVEINT(current_idx) ; 	/* save current idx */
	    current_idx = idx ;

	    SAVE_DEFSV ;	/* save $_ */
	    /* make $_ use our buffer */
	    DEFSV_set(newSVpv("", 0)) ; 

    	    PUSHMARK(sp) ;
	    if (CODE_REF(my_sv)) {
	    /* if (SvROK(PERL_OBJECT(my_sv)) && SvTYPE(SvRV(PERL_OBJECT(my_sv))) == SVt_PVCV) { */
    	        count = perl_call_sv((SV*)PERL_OBJECT(my_sv), G_SCALAR);
	    }
	    else {
                XPUSHs((SV*)PERL_OBJECT(my_sv)) ;  
    	        PUTBACK ;
    	        count = perl_call_method("filter", G_SCALAR);
	    }
    	    SPAGAIN ;

            if (count != 1)
	        croak("Filter::Util::Call - %s::filter returned %d values, 1 was expected \n", 
			PERL_MODULE(my_sv), count ) ;
    
	    n = (IV)POPi ;

	    if (fdebug)
	        warn("status = %d, length op buf = %" IVdf " [%s]\n",
		     n, (IV)SvCUR(DEFSV), SvPVX(DEFSV) ) ;
	    if (SvCUR(DEFSV))
	        sv_setpvn(my_sv, SvPVX(DEFSV), SvCUR(DEFSV)) ; 

    	    sv_2mortal(DEFSV);

    	    PUTBACK ;
    	    FREETMPS ;
    	    LEAVE ;
	}
	else
	    n = FILTER_READ(idx + 1, my_sv, maxlen) ;

 	if (n <= 0)
	{
	    /* Either EOF or an error */

	    if (fdebug) 
	        warn ("filter_read %d returned %d , returning %" IVdf "\n", idx, n,
		      (SvCUR(buf_sv)>0) ? (IV)SvCUR(buf_sv) : (IV)n);

	    /* PERL_MODULE(my_sv) ; */
	    /* PERL_OBJECT(my_sv) ; */
	    filter_del(filter_call); 

	    /* If error, return the code */
	    if (n < 0)
		return n ;

	    /* return what we have so far else signal eof */
	    return (SvCUR(buf_sv)>0) ? (int)SvCUR(buf_sv) : n;
	}

    }
}



MODULE = Filter::Util::Call		PACKAGE = Filter::Util::Call

REQUIRE:	1.924
PROTOTYPES:	ENABLE

#define IDX		current_idx

int
filter_read(size=0)
	int	size 
	CODE:
	{
    	    dMY_CXT;
	    SV * buffer = DEFSV ;

	    RETVAL = FILTER_READ(IDX + 1, buffer, size) ;
	}
	OUTPUT:
	    RETVAL




void
real_import(object, perlmodule, coderef)
    SV *	object
    char *	perlmodule 
    IV		coderef
    PPCODE:
    {
        SV * sv = newSV(1) ;

        (void)SvPOK_only(sv) ;
        filter_add(filter_call, sv) ;

	PERL_MODULE(sv) = savepv(perlmodule) ;
	PERL_OBJECT(sv) = (GV*) newSVsv(object) ;
	FILTER_ACTIVE(sv) = TRUE ;
        BUF_OFFSET(sv) = 0 ;
	CODE_REF(sv)   = coderef ;

        SvCUR_set(sv, 0) ;

    }

void
filter_del()
    CODE:
        dMY_CXT;
	if (PERL_FILTER_EXISTS(IDX) && FILTER_DATA(IDX) && FILTER_ACTIVE(FILTER_DATA(IDX)))
	    FILTER_ACTIVE(FILTER_DATA(IDX)) = FALSE ;



void
unimport(package="$Package", ...)
    const char *package
    PPCODE:
    PERL_UNUSED_VAR(package);
    filter_del(filter_call);


BOOT:
  {
    MY_CXT_INIT;
#ifdef FDEBUG
    fdebug = 1;
#else
    fdebug = 0;
#endif
    /* temporary hack to control debugging in toke.c */
    if (fdebug)
        filter_add(NULL, (fdebug) ? (SV*)"1" : (SV*)"0");  
  }

