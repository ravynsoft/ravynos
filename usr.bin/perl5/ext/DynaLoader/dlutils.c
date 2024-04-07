/* dlutils.c - handy functions and definitions for dl_*.xs files
 *
 * Currently this file is simply #included into dl_*.xs/.c files.
 * It should really be split into a dlutils.h and dlutils.c
 *
 * Modified:
 * 29th Feburary 2000 - Alan Burlison: Added functionality to close dlopen'd
 *                      files when the interpreter exits
 */

#define PERL_EUPXS_ALWAYS_EXPORT
#ifndef START_MY_CXT /* Some IDEs try compiling this standalone. */
#   define PERL_EXT
#   include "EXTERN.h"
#   include "perl.h"
#   include "XSUB.h"
#endif

#ifndef XS_VERSION
#  define XS_VERSION "0"
#endif
#define MY_CXT_KEY "DynaLoader::_guts" XS_VERSION

/* disable version checking since DynaLoader can't be DynaLoaded */
#undef dXSBOOTARGSXSAPIVERCHK
#define dXSBOOTARGSXSAPIVERCHK dXSBOOTARGSNOVERCHK

typedef struct {
    SV*		x_dl_last_error;	/* pointer to allocated memory for
                                           last error message */
#if defined(PERL_IN_DL_HPUX_XS) || defined(PERL_IN_DL_DLOPEN_XS)
    int		x_dl_nonlazy;		/* flag for immediate rather than lazy
                                           linking (spots unresolved symbol) */
#endif
#ifdef DL_LOADONCEONLY
    HV *	x_dl_loaded_files;	/* only needed on a few systems */
#endif
#ifdef DL_CXT_EXTRA
    my_cxtx_t	x_dl_cxtx;		/* extra platform-specific data */
#endif
#ifdef DEBUGGING
    int		x_dl_debug;	/* value copied from $DynaLoader::dl_debug */
#endif
} my_cxt_t;

START_MY_CXT

#define dl_last_error	(SvPVX(MY_CXT.x_dl_last_error))
#if defined(PERL_IN_DL_HPUX_XS) || defined(PERL_IN_DL_DLOPEN_XS)
#define dl_nonlazy	(MY_CXT.x_dl_nonlazy)
#endif
#ifdef DL_LOADONCEONLY
#define dl_loaded_files	(MY_CXT.x_dl_loaded_files)
#endif
#ifdef DL_CXT_EXTRA
#define dl_cxtx		(MY_CXT.x_dl_cxtx)
#endif
#ifdef DEBUGGING
#define dl_debug	(MY_CXT.x_dl_debug)
#endif

#ifdef DEBUGGING
#define DLDEBUG(level,code) \
    STMT_START {					\
        dMY_CXT;					\
        if (dl_debug>=level) { code; }			\
    } STMT_END
#else
#define DLDEBUG(level,code)	NOOP
#endif

#ifdef DL_UNLOAD_ALL_AT_EXIT
/* Close all dlopen'd files */
static void
dl_unload_all_files(pTHX_ void *unused)
{
    CV *sub;
    PERL_UNUSED_ARG(unused);
    if ((sub = get_cvs("DynaLoader::dl_unload_file", 0)) != NULL) {
        AV *dl_librefs = get_av("DynaLoader::dl_librefs", 0);
        SV *dl_libref;
        while ((dl_libref = av_pop(dl_librefs)) != &PL_sv_undef) {
           dSP;
           ENTER;
           SAVETMPS;
           PUSHMARK(SP);
           XPUSHs(sv_2mortal(dl_libref));
           PUTBACK;
           call_sv((SV*)sub, G_DISCARD | G_NODEBUG);
           FREETMPS;
           LEAVE;
        }
    }
}
#endif

static void
dl_generic_private_init(pTHX)	/* called by dl_*.xs dl_private_init() */
{
#if defined(PERL_IN_DL_HPUX_XS) || defined(PERL_IN_DL_DLOPEN_XS)
    char *perl_dl_nonlazy;
    UV uv;
#endif
    MY_CXT_INIT;

    MY_CXT.x_dl_last_error = newSVpvs("");
#ifdef DL_LOADONCEONLY
    dl_loaded_files = NULL;
#endif
#ifdef DEBUGGING
    {
        SV *sv = get_sv("DynaLoader::dl_debug", 0);
        dl_debug = sv ? SvIV(sv) : 0;
    }
#endif

#if defined(PERL_IN_DL_HPUX_XS) || defined(PERL_IN_DL_DLOPEN_XS)
    if ( (perl_dl_nonlazy = PerlEnv_getenv("PERL_DL_NONLAZY")) != NULL
        && grok_atoUV(perl_dl_nonlazy, &uv, NULL)
        && uv <= INT_MAX
    ) {
        dl_nonlazy = (int)uv;
    } else
        dl_nonlazy = 0;
    if (dl_nonlazy)
        DLDEBUG(1,PerlIO_printf(Perl_debug_log, "DynaLoader bind mode is 'non-lazy'\n"));
#endif
#ifdef DL_LOADONCEONLY
    if (!dl_loaded_files)
        dl_loaded_files = newHV(); /* provide cache for dl_*.xs if needed */
#endif
#ifdef DL_UNLOAD_ALL_AT_EXIT
    call_atexit(&dl_unload_all_files, (void*)0);
#endif
}


#ifndef SYMBIAN
/* SaveError() takes printf style args and saves the result in dl_last_error */
static void
SaveError(pTHX_ const char* pat, ...)
{
    va_list args;
    SV *msv;
    const char *message;
    STRLEN len;

    /* This code is based on croak/warn, see mess() in util.c */

    va_start(args, pat);
    msv = vmess(pat, &args);
    va_end(args);

    message = SvPV(msv,len);
    len++;		/* include terminating null char */

    {
        dMY_CXT;
    /* Copy message into dl_last_error (including terminating null char) */
        sv_setpvn(MY_CXT.x_dl_last_error, message, len) ;
        DLDEBUG(2,PerlIO_printf(Perl_debug_log, "DynaLoader: stored error msg '%s'\n",dl_last_error));
    }
}
#endif

