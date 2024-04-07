#define PERL_NO_GET_CONTEXT
#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"
#define U8 U8

#define OUR_DEFAULT_FB	"Encode::PERLQQ"
#define OUR_STOP_AT_PARTIAL "Encode::STOP_AT_PARTIAL"
#define OUR_LEAVE_SRC "Encode::LEAVE_SRC"

/* This will be set during BOOT */
static unsigned int encode_stop_at_partial = 0;
static unsigned int encode_leave_src = 0;

#if defined(USE_PERLIO)

/* Define an encoding "layer" in the perliol.h sense.

   The layer defined here "inherits" in an object-oriented sense from
   the "perlio" layer with its PerlIOBuf_* "methods".  The
   implementation is particularly efficient as until Encode settles
   down there is no point in tryint to tune it.

   The layer works by overloading the "fill" and "flush" methods.

   "fill" calls "SUPER::fill" in perl terms, then calls the encode OO
   perl API to convert the encoded data to UTF-8 form, then copies it
   back to the buffer. The "base class's" read methods then see the
   UTF-8 data.

   "flush" transforms the UTF-8 data deposited by the "base class's
   write method in the buffer back into the encoded form using the
   encode OO perl API, then copies data back into the buffer and calls
   "SUPER::flush.

   Note that "flush" is _also_ called for read mode - we still do the
   (back)-translate so that the base class's "flush" sees the
   correct number of encoded chars for positioning the seek
   pointer. (This double translation is the worst performance issue -
   particularly with all-perl encode engine.)

*/

#include "perliol.h"

typedef struct {
    PerlIOBuf base;		/* PerlIOBuf stuff */
    SV *bufsv;			/* buffer seen by layers above */
    SV *dataSV;			/* data we have read from layer below */
    SV *enc;			/* the encoding object */
    SV *chk;                    /* CHECK in Encode methods */
    int flags;			/* Flags currently just needs lines */
    int inEncodeCall;		/* trap recursive encode calls */
} PerlIOEncode;

#define NEEDS_LINES	1

static const MGVTBL PerlIOEncode_tag = { 0, 0, 0, 0, 0, 0, 0, 0 };

static SV *
PerlIOEncode_getarg(pTHX_ PerlIO * f, CLONE_PARAMS * param, int flags)
{
    PerlIOEncode *e = PerlIOSelf(f, PerlIOEncode);
    SV *sv;
    PERL_UNUSED_ARG(flags);
    /* During cloning, return an undef token object so that _pushed() knows
     * that it should not call methods and wait for _dup() to actually dup the
     * encoding object. */
    if (param) {
	sv = newSV(0);
	sv_magicext(sv, NULL, PERL_MAGIC_ext, &PerlIOEncode_tag, 0, 0);
	return sv;
    }
    sv = &PL_sv_undef;
    if (e->enc) {
	dSP;
	/* Not 100% sure stack swap is right thing to do during dup ... */
	PUSHSTACKi(PERLSI_MAGIC);
	ENTER;
	SAVETMPS;
	PUSHMARK(sp);
	XPUSHs(e->enc);
	PUTBACK;
	if (call_method("name", G_SCALAR) == 1) {
	    SPAGAIN;
	    sv = newSVsv(POPs);
	    PUTBACK;
	}
	FREETMPS;
	LEAVE;
	POPSTACK;
    }
    return sv;
}

static IV
PerlIOEncode_pushed(pTHX_ PerlIO * f, const char *mode, SV * arg, PerlIO_funcs *tab)
{
    PerlIOEncode *e = PerlIOSelf(f, PerlIOEncode);
    dSP;
    IV  code = PerlIOBuf_pushed(aTHX_ f, mode, Nullsv,tab);
    SV *result = Nullsv;

    if (SvTYPE(arg) >= SVt_PVMG
		&& mg_findext(arg, PERL_MAGIC_ext, &PerlIOEncode_tag)) {
	e->enc = NULL;
	e->chk = NULL;
	e->inEncodeCall = 0;
	return code;
    }

    PUSHSTACKi(PERLSI_MAGIC);
    ENTER;
    SAVETMPS;

    PUSHMARK(sp);
    XPUSHs(arg);
    PUTBACK;
    if (call_pv("Encode::find_encoding", G_SCALAR) != 1) {
	/* should never happen */
	Perl_die(aTHX_ "Encode::find_encoding did not return a value");
	return -1;
    }
    SPAGAIN;
    result = POPs;
    PUTBACK;

    if (!SvROK(result) || !SvOBJECT(SvRV(result))) {
	e->enc = Nullsv;
        if (ckWARN_d(WARN_IO))
            Perl_warner(aTHX_ packWARN(WARN_IO), "Cannot find encoding \"%" SVf "\"",
                    arg);
	errno = EINVAL;
	code = -1;
    }
    else {

       /* $enc->renew */
	PUSHMARK(sp);
	XPUSHs(result);
	PUTBACK;
	if (call_method("renew",G_SCALAR|G_EVAL) != 1 || SvTRUE(ERRSV)) {
            if (ckWARN_d(WARN_IO))
                Perl_warner(aTHX_ packWARN(WARN_IO), "\"%" SVf "\" does not support renew method",
                        arg);
	}
	else {
	    SPAGAIN;
	    result = POPs;
	    PUTBACK;
	}
	e->enc = newSVsv(result);
	PUSHMARK(sp);
	XPUSHs(e->enc);
	PUTBACK;
	if (call_method("needs_lines",G_SCALAR|G_EVAL) != 1 || SvTRUE(ERRSV)) {
            if (ckWARN_d(WARN_IO))
                Perl_warner(aTHX_ packWARN(WARN_IO), "\"%" SVf "\" does not support needs_lines",
			arg);
	}
	else {
	    SPAGAIN;
	    result = POPs;
	    PUTBACK;
	    if (SvTRUE(result)) {
		e->flags |= NEEDS_LINES;
	    }
	}
	PerlIOBase(f)->flags |= PERLIO_F_UTF8;
    }

    e->chk = newSVsv(get_sv("PerlIO::encoding::fallback", 0));
    if (SvROK(e->chk))
        Perl_croak(aTHX_ "PerlIO::encoding::fallback must be an integer");
    SvUV_set(e->chk, ((SvUV(e->chk) & ~encode_leave_src) | encode_stop_at_partial));
    e->inEncodeCall = 0;

    FREETMPS;
    LEAVE;
    POPSTACK;
    return code;
}

static IV
PerlIOEncode_popped(pTHX_ PerlIO * f)
{
    PerlIOEncode *e = PerlIOSelf(f, PerlIOEncode);
    if (e->enc) {
	SvREFCNT_dec(e->enc);
	e->enc = Nullsv;
    }
    if (e->bufsv) {
	SvREFCNT_dec(e->bufsv);
	e->bufsv = Nullsv;
    }
    if (e->dataSV) {
	SvREFCNT_dec(e->dataSV);
	e->dataSV = Nullsv;
    }
    if (e->chk) {
	SvREFCNT_dec(e->chk);
	e->chk = Nullsv;
    }
    return 0;
}

static STDCHAR *
PerlIOEncode_get_base(pTHX_ PerlIO * f)
{
    PerlIOEncode *e = PerlIOSelf(f, PerlIOEncode);
    if (!e->base.bufsiz)
	e->base.bufsiz = 1024;
    if (!e->bufsv) {
	e->bufsv = newSV(e->base.bufsiz);
	SvPVCLEAR(e->bufsv);
    }
    e->base.buf = (STDCHAR *) SvPVX(e->bufsv);
    if (!e->base.ptr)
	e->base.ptr = e->base.buf;
    if (!e->base.end)
	e->base.end = e->base.buf;
    if (e->base.ptr < e->base.buf
	|| e->base.ptr > e->base.buf + SvLEN(e->bufsv)) {
	Perl_warn(aTHX_ " ptr %p(%p)%p", e->base.buf, e->base.ptr,
		  e->base.buf + SvLEN(e->bufsv));
	abort();
    }
    if (SvLEN(e->bufsv) < e->base.bufsiz) {
	SSize_t poff = e->base.ptr - e->base.buf;
	SSize_t eoff = e->base.end - e->base.buf;
	e->base.buf = (STDCHAR *) SvGROW(e->bufsv, e->base.bufsiz);
	e->base.ptr = e->base.buf + poff;
	e->base.end = e->base.buf + eoff;
    }
    if (e->base.ptr < e->base.buf
	|| e->base.ptr > e->base.buf + SvLEN(e->bufsv)) {
	Perl_warn(aTHX_ " ptr %p(%p)%p", e->base.buf, e->base.ptr,
		  e->base.buf + SvLEN(e->bufsv));
	abort();
    }
    return e->base.buf;
}

static IV
PerlIOEncode_fill(pTHX_ PerlIO * f)
{
    PerlIOEncode *e = PerlIOSelf(f, PerlIOEncode);
    dSP;
    IV code = 0;
    PerlIO *n;
    SSize_t avail;

    if (PerlIO_flush(f) != 0)
	return -1;
    n  = PerlIONext(f);
    if (!PerlIO_fast_gets(n)) {
	/* Things get too messy if we don't have a buffer layer
	   push a :perlio to do the job */
	char mode[8];
	n  = PerlIO_push(aTHX_ n, &PerlIO_perlio, PerlIO_modestr(f,mode), Nullsv);
	if (!n) {
	    Perl_die(aTHX_ "panic: cannot push :perlio for %p",f);
	}
    }
    PUSHSTACKi(PERLSI_MAGIC);
    ENTER;
    SAVETMPS;
  retry:
    avail = PerlIO_get_cnt(n);
    if (avail <= 0) {
	avail = PerlIO_fill(n);
	if (avail == 0) {
	    avail = PerlIO_get_cnt(n);
	}
	else {
	    if (!PerlIO_error(n) && PerlIO_eof(n))
		avail = 0;
	}
    }
    if (avail > 0 || (e->flags & NEEDS_LINES)) {
	STDCHAR *ptr = PerlIO_get_ptr(n);
	SSize_t use  = (avail >= 0) ? avail : 0;
	SV *uni;
	char *s = NULL;
	STRLEN len = 0;
	e->base.ptr = e->base.end = (STDCHAR *) NULL;
	(void) PerlIOEncode_get_base(aTHX_ f);
	if (!e->dataSV)
	    e->dataSV = newSV(0);
	if (SvTYPE(e->dataSV) < SVt_PV) {
	    sv_upgrade(e->dataSV,SVt_PV);
	}
	if (e->flags & NEEDS_LINES) {
	    /* Encoding needs whole lines (e.g. iso-2022-*)
	       search back from end of available data for
	       and line marker
	     */
	    STDCHAR *nl = ptr+use-1;
	    while (nl >= ptr) {
		if (*nl == '\n') {
		    break;
		}
		nl--;
	    }
	    if (nl >= ptr && *nl == '\n') {
		/* found a line - take up to and including that */
		use = (nl+1)-ptr;
	    }
	    else if (avail > 0) {
		/* No line, but not EOF - append avail to the pending data */
		sv_catpvn(e->dataSV, (char*)ptr, use);
		PerlIO_set_ptrcnt(n, ptr+use, 0);
		goto retry;
	    }
	    else if (!SvCUR(e->dataSV)) {
		goto end_of_file;
	    }
	}
	if (!SvCUR(e->dataSV))
	    SvPVCLEAR(e->dataSV);
	if (use + SvCUR(e->dataSV) > e->base.bufsiz) {
	    if (e->flags & NEEDS_LINES) {
		/* Have to grow buffer */
		e->base.bufsiz = use + SvCUR(e->dataSV);
		PerlIOEncode_get_base(aTHX_ f);
	    }
	    else {
		use = e->base.bufsiz - SvCUR(e->dataSV);
	    }
	}
	sv_catpvn(e->dataSV,(char*)ptr,use);
	SvUTF8_off(e->dataSV);
	PUSHMARK(sp);
	XPUSHs(e->enc);
	XPUSHs(e->dataSV);
	XPUSHs(e->chk);
	PUTBACK;
	if (call_method("decode", G_SCALAR) != 1) {
	    Perl_die(aTHX_ "panic: decode did not return a value");
	}
	SPAGAIN;
	uni = POPs;
	PUTBACK;
	/* No cows allowed. */
	if (SvTHINKFIRST(e->dataSV)) SvPV_force_nolen(e->dataSV);
	/* Now get translated string (forced to UTF-8) and use as buffer */
	if (SvPOK(uni)) {
	    s = SvPVutf8(uni, len);
#ifdef PARANOID_ENCODE_CHECKS
	    if (len && !is_utf8_string((U8*)s,len)) {
		Perl_warn(aTHX_ "panic: decode did not return UTF-8 '%.*s'",(int) len,s);
	    }
#endif
	}
	if (len > 0) {
	    /* Got _something */
	    /* if decode gave us back dataSV then data may vanish when
	       we do ptrcnt adjust - so take our copy now.
	       (The copy is a pain - need a put-it-here option for decode.)
	     */
	    sv_setpvn(e->bufsv,s,len);
	    e->base.ptr = e->base.buf = (STDCHAR*)SvPVX(e->bufsv);
	    e->base.end = e->base.ptr + SvCUR(e->bufsv);
	    PerlIOBase(f)->flags |= PERLIO_F_RDBUF;
	    SvUTF8_on(e->bufsv);

	    /* Adjust ptr/cnt not taking anything which
	       did not translate - not clear this is a win */
	    /* compute amount we took */
	    if (!SvPOKp(e->dataSV)) (void)SvPV_force_nolen(e->dataSV);
	    use -= SvCUR(e->dataSV);
	    PerlIO_set_ptrcnt(n, ptr+use, (avail-use));
	    /* and as we did not take it, it isn't pending */
	    SvCUR_set(e->dataSV,0);
	} else {
	    /* Got nothing - assume partial character so we need some more */
	    /* Make sure e->dataSV is a normal SV before re-filling as
	       buffer alias will change under us
	     */
	    s = SvPV(e->dataSV,len);
	    sv_setpvn(e->dataSV,s,len);
	    PerlIO_set_ptrcnt(n, ptr+use, (avail-use));
	    goto retry;
	}
    }
    else {
    end_of_file:
	code = -1;
	if (avail == 0)
	    PerlIOBase(f)->flags |= PERLIO_F_EOF;
	else
	{
	    PerlIOBase(f)->flags |= PERLIO_F_ERROR;
	    Perl_PerlIO_save_errno(aTHX_ f);
	}
    }
    FREETMPS;
    LEAVE;
    POPSTACK;
    return code;
}

static IV
PerlIOEncode_flush(pTHX_ PerlIO * f)
{
    PerlIOEncode *e = PerlIOSelf(f, PerlIOEncode);
    IV code = 0;

    if (e->bufsv) {
	dSP;
	SV *str;
	char *s;
	STRLEN len;
	SSize_t count = 0;
	if ((PerlIOBase(f)->flags & PERLIO_F_WRBUF) && (e->base.ptr > e->base.buf)) {
	    if (e->inEncodeCall) return 0;
	    /* Write case - encode the buffer and write() to layer below */
	    PUSHSTACKi(PERLSI_MAGIC);
	    ENTER;
	    SAVETMPS;
	    PUSHMARK(sp);
	    XPUSHs(e->enc);
	    SvCUR_set(e->bufsv, e->base.ptr - e->base.buf);
	    SvUTF8_on(e->bufsv);
	    XPUSHs(e->bufsv);
	    XPUSHs(e->chk);
	    PUTBACK;
	    e->inEncodeCall = 1;
	    if (call_method("encode", G_SCALAR) != 1) {
		e->inEncodeCall = 0;
		Perl_die(aTHX_ "panic: encode did not return a value");
	    }
	    e->inEncodeCall = 0;
	    SPAGAIN;
	    str = POPs;
	    PUTBACK;
	    s = SvPV(str, len);
	    count = PerlIO_write(PerlIONext(f),s,len);
	    if ((STRLEN)count != len) {
		code = -1;
	    }
	    FREETMPS;
	    LEAVE;
	    POPSTACK;
	    if (PerlIO_flush(PerlIONext(f)) != 0) {
		code = -1;
	    }
	    if (!SvPOKp(e->bufsv) || SvTHINKFIRST(e->bufsv))
		(void)SvPV_force_nolen(e->bufsv);
	    if ((STDCHAR *)SvPVX(e->bufsv) != e->base.buf) {
		e->base.ptr = (STDCHAR *)SvEND(e->bufsv);
		e->base.end = (STDCHAR *)SvPVX(e->bufsv) + (e->base.end-e->base.buf);
		e->base.buf = (STDCHAR *)SvPVX(e->bufsv);
	    }
	    (void)PerlIOEncode_get_base(aTHX_ f);
	    if (SvCUR(e->bufsv)) {
		/* Did not all translate */
		e->base.ptr = e->base.buf+SvCUR(e->bufsv);
		return code;
	    }
	}
	else if ((PerlIOBase(f)->flags & PERLIO_F_RDBUF)) {
	    /* read case */
	    /* if we have any untranslated stuff then unread that first */
	    /* FIXME - unread is fragile is there a better way ? */
	    if (e->dataSV && SvCUR(e->dataSV)) {
		s = SvPV(e->dataSV, len);
		count = PerlIO_unread(PerlIONext(f),s,len);
		if ((STRLEN)count != len) {
		    code = -1;
		}
		SvCUR_set(e->dataSV,0);
	    }
	    /* See if there is anything left in the buffer */
	    if (e->base.ptr < e->base.end) {
		if (e->inEncodeCall) return 0;
		/* Bother - have unread data.
		   re-encode and unread() to layer below
		 */
		PUSHSTACKi(PERLSI_MAGIC);
		ENTER;
		SAVETMPS;
		str = sv_newmortal();
		sv_upgrade(str, SVt_PV);
		SvPV_set(str, (char*)e->base.ptr);
		SvLEN_set(str, 0);
		SvCUR_set(str, e->base.end - e->base.ptr);
		SvPOK_only(str);
		SvUTF8_on(str);
		PUSHMARK(sp);
		XPUSHs(e->enc);
		XPUSHs(str);
		XPUSHs(e->chk);
		PUTBACK;
		e->inEncodeCall = 1;
		if (call_method("encode", G_SCALAR) != 1) {
		    e->inEncodeCall = 0;
		    Perl_die(aTHX_ "panic: encode did not return a value");
		}
		e->inEncodeCall = 0;
		SPAGAIN;
		str = POPs;
		PUTBACK;
		s = SvPV(str, len);
		count = PerlIO_unread(PerlIONext(f),s,len);
		if ((STRLEN)count != len) {
		    code = -1;
		}
		FREETMPS;
		LEAVE;
		POPSTACK;
	    }
	}
	e->base.ptr = e->base.end = e->base.buf;
	PerlIOBase(f)->flags &= ~(PERLIO_F_RDBUF | PERLIO_F_WRBUF);
    }
    return code;
}

static IV
PerlIOEncode_close(pTHX_ PerlIO * f)
{
    PerlIOEncode *e = PerlIOSelf(f, PerlIOEncode);
    IV code;
    if (PerlIOBase(f)->flags & PERLIO_F_RDBUF) {
	/* Discard partial character */
	if (e->dataSV) {
	    SvCUR_set(e->dataSV,0);
	}
	/* Don't back decode and unread any pending data */
	e->base.ptr = e->base.end = e->base.buf;
    }
    code = PerlIOBase_close(aTHX_ f);
    if (e->bufsv) {
	/* This should only fire for write case */
	if (e->base.buf && e->base.ptr > e->base.buf) {
	    Perl_croak(aTHX_ "Close with partial character");
	}
	SvREFCNT_dec(e->bufsv);
	e->bufsv = Nullsv;
    }
    e->base.buf = NULL;
    e->base.ptr = NULL;
    e->base.end = NULL;
    PerlIOBase(f)->flags &= ~(PERLIO_F_RDBUF | PERLIO_F_WRBUF);
    return code;
}

static Off_t
PerlIOEncode_tell(pTHX_ PerlIO * f)
{
    PerlIOBuf *b = PerlIOSelf(f, PerlIOBuf);
    /* Unfortunately the only way to get a position is to (re-)translate,
       the UTF8 we have in buffer and then ask layer below
     */
    PerlIO_flush(f);
    if (b->buf && b->ptr > b->buf) {
	Perl_croak(aTHX_ "Cannot tell at partial character");
    }
    return PerlIO_tell(PerlIONext(f));
}

static PerlIO *
PerlIOEncode_dup(pTHX_ PerlIO * f, PerlIO * o,
		 CLONE_PARAMS * params, int flags)
{
    if ((f = PerlIOBase_dup(aTHX_ f, o, params, flags))) {
	PerlIOEncode *fe = PerlIOSelf(f, PerlIOEncode);
	PerlIOEncode *oe = PerlIOSelf(o, PerlIOEncode);
	if (oe->enc) {
	    fe->enc = PerlIO_sv_dup(aTHX_ oe->enc, params);
	}
	if (oe->chk) {
	    fe->chk = PerlIO_sv_dup(aTHX_ oe->chk, params);
	}
    }
    return f;
}

static SSize_t
PerlIOEncode_write(pTHX_ PerlIO *f, const void *vbuf, Size_t count)
{
    PerlIOEncode *e = PerlIOSelf(f, PerlIOEncode);
    if (e->flags & NEEDS_LINES) {
	SSize_t done = 0;
	const char *ptr = (const char *) vbuf;
	const char *end = ptr+count;
	while (ptr < end) {
	    const char *nl = ptr;
	    while (nl < end && *nl++ != '\n') /* empty body */;
	    done = PerlIOBuf_write(aTHX_ f, ptr, nl-ptr);
	    if (done != nl-ptr) {
		if (done > 0) {
		    ptr += done;
		}
		break;
	    }
	    ptr += done;
	    if (ptr[-1] == '\n') {
		if (PerlIOEncode_flush(aTHX_ f) != 0) {
		    break;
		}
	    }
	}
	return (SSize_t) (ptr - (const char *) vbuf);
    }
    else {
	return PerlIOBuf_write(aTHX_ f, vbuf, count);
    }
}

static PERLIO_FUNCS_DECL(PerlIO_encode) = {
    sizeof(PerlIO_funcs),
    "encoding",
    sizeof(PerlIOEncode),
    PERLIO_K_BUFFERED|PERLIO_K_DESTRUCT,
    PerlIOEncode_pushed,
    PerlIOEncode_popped,
    PerlIOBuf_open,
    NULL, /* binmode - always pop */
    PerlIOEncode_getarg,
    PerlIOBase_fileno,
    PerlIOEncode_dup,
    PerlIOBuf_read,
    PerlIOBuf_unread,
    PerlIOEncode_write,
    PerlIOBuf_seek,
    PerlIOEncode_tell,
    PerlIOEncode_close,
    PerlIOEncode_flush,
    PerlIOEncode_fill,
    PerlIOBase_eof,
    PerlIOBase_error,
    PerlIOBase_clearerr,
    PerlIOBase_setlinebuf,
    PerlIOEncode_get_base,
    PerlIOBuf_bufsiz,
    PerlIOBuf_get_ptr,
    PerlIOBuf_get_cnt,
    PerlIOBuf_set_ptrcnt,
};
#endif				/* encode layer */

MODULE = PerlIO::encoding PACKAGE = PerlIO::encoding

PROTOTYPES: ENABLE

BOOT:
{
    /*
     * we now "use Encode ()" here instead of
     * PerlIO/encoding.pm.  This avoids SEGV when ":encoding()"
     * is invoked without prior "use Encode". -- dankogai
     */
    PUSHSTACKi(PERLSI_MAGIC);
    if (!get_cvs(OUR_STOP_AT_PARTIAL, 0)) {
	/* The SV is magically freed by load_module */
	load_module(PERL_LOADMOD_NOIMPORT, newSVpvs("Encode"), Nullsv, Nullsv);
	assert(sp == PL_stack_sp);
    }

    PUSHMARK(sp);
    PUTBACK;
    if (call_pv(OUR_STOP_AT_PARTIAL, G_SCALAR) != 1) {
	    /* should never happen */
	    Perl_die(aTHX_ "%s did not return a value", OUR_STOP_AT_PARTIAL);
    }
    SPAGAIN;
    encode_stop_at_partial = POPu;

    PUSHMARK(sp);
    PUTBACK;
    if (call_pv(OUR_LEAVE_SRC, G_SCALAR) != 1) {
	    /* should never happen */
	    Perl_die(aTHX_ "%s did not return a value", OUR_LEAVE_SRC);
    }
    SPAGAIN;
    encode_leave_src = POPu;

    PUTBACK;
#ifdef PERLIO_LAYERS
    PerlIO_define_layer(aTHX_ PERLIO_FUNCS_CAST(&PerlIO_encode));
#endif
    POPSTACK;
}
