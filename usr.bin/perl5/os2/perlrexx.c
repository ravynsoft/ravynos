#define INCL_DOSPROCESS
#define INCL_DOSSEMAPHORES
#define INCL_DOSMODULEMGR
#define INCL_DOSMISC
#define INCL_DOSEXCEPTIONS
#define INCL_DOSERRORS
#define INCL_REXXSAA
#include <os2.h>

/*
 *      The Road goes ever on and on
 *          Down from the door where it began.
 *
 *     [Bilbo on p.35 of _The Lord of the Rings_, I/i: "A Long-Expected Party"]
 *     [Frodo on p.73 of _The Lord of the Rings_, I/iii: "Three Is Company"]
 */

#ifdef OEMVS
#ifdef MYMALLOC
/* sbrk is limited to first heap segement so make it big */
#pragma runopts(HEAP(8M,500K,ANYWHERE,KEEP,8K,4K) STACK(,,ANY,) ALL31(ON))
#else
#pragma runopts(HEAP(2M,500K,ANYWHERE,KEEP,8K,4K) STACK(,,ANY,) ALL31(ON))
#endif
#endif


#include "EXTERN.h"
#include "perl.h"

static void xs_init (pTHX);
static PerlInterpreter *my_perl;

ULONG PERLEXPORTALL(PCSZ name, LONG rargc, const RXSTRING *rargv, PCSZ queuename, PRXSTRING retstr);
ULONG PERLDROPALL(PCSZ name, LONG rargc, const RXSTRING *rargv, PCSZ queuename, PRXSTRING retstr);
ULONG PERLDROPALLEXIT(PCSZ name, LONG rargc, const RXSTRING *rargv, PCSZ queuename, PRXSTRING retstr);

/* Register any extra external extensions */

/* Do not delete this line--writemain depends on it */
EXTERN_C void boot_DynaLoader (pTHX_ CV* cv);

static void
xs_init(pTHX)
{
    char *file = __FILE__;
    dXSUB_SYS;
        newXS("DynaLoader::boot_DynaLoader", boot_DynaLoader, file);
}

int perlos2_is_inited;

static void
init_perlos2(void)
{
/*    static char *env[1] = {NULL};	*/

    Perl_OS2_init3(0, 0, 0);
}

static int
init_perl(int doparse)
{
    char *argv[3] = {"perl_in_REXX", "-e", ""};

    if (!perlos2_is_inited) {
        perlos2_is_inited = 1;
        init_perlos2();
    }
    if (my_perl)
        return 1;
    if (!PL_do_undump) {
        my_perl = perl_alloc();
        if (!my_perl)
            return 0;
        perl_construct(my_perl);
        PL_exit_flags |= PERL_EXIT_DESTRUCT_END;
        PL_perl_destruct_level = 1;
    }
    if (!doparse)
        return 1;
    return !perl_parse(my_perl, xs_init, 3, argv, (char **)NULL);
}

static char last_error[4096];

static int
seterr(char *format, ...)
{
        va_list va;
        char *s = last_error;

        va_start(va, format);
        if (s[0]) {
            s += strlen(s);
            if (s[-1] != '\n') {
                snprintf(s, sizeof(last_error) - (s - last_error), "\n");
                s += strlen(s);
            }
        }
        vsnprintf(s, sizeof(last_error) - (s - last_error), format, va);
        return 1;
}

/* The REXX-callable entrypoints ... */

ULONG PERL (PCSZ name, LONG rargc, const RXSTRING *rargv,
                    PCSZ queuename, PRXSTRING retstr)
{
    int exitstatus;
    char buf[256];
    char *argv[3] = {"perl_from_REXX", "-e", buf};
    ULONG ret;

    if (rargc != 1)
        return seterr("one argument expected, got %ld", rargc);
    if (rargv[0].strlength >= sizeof(buf))
        return seterr("length of the argument %ld exceeds the maximum %ld",
                      rargv[0].strlength, (long)sizeof(buf) - 1);

    if (!init_perl(0))
        return 1;

    memcpy(buf, rargv[0].strptr, rargv[0].strlength);
    buf[rargv[0].strlength] = 0;
    
    if (!perl_parse(my_perl, xs_init, 3, argv, (char **)NULL))
        perl_run(my_perl);

    exitstatus = perl_destruct(my_perl);
    perl_free(my_perl);
    my_perl = 0;

    if (exitstatus)
        ret = 1;
    else {
        ret = 0;
        sprintf(retstr->strptr, "%s", "ok");
        retstr->strlength = strlen (retstr->strptr);
    }
    PERL_SYS_TERM1(0);
    return ret;
}

ULONG PERLEXIT (PCSZ name, LONG rargc, const RXSTRING *rargv,
                    PCSZ queuename, PRXSTRING retstr)
{
    if (rargc != 0)
        return seterr("no arguments expected, got %ld", rargc);
    PERL_SYS_TERM1(0);
    return 0;
}

ULONG PERLTERM (PCSZ name, LONG rargc, const RXSTRING *rargv,
                    PCSZ queuename, PRXSTRING retstr)
{
    if (rargc != 0)
        return seterr("no arguments expected, got %ld", rargc);
    if (!my_perl)
        return seterr("no perl interpreter present");
    perl_destruct(my_perl);
    perl_free(my_perl);
    my_perl = 0;

    sprintf(retstr->strptr, "%s", "ok");
    retstr->strlength = strlen (retstr->strptr);
    return 0;
}


ULONG PERLINIT (PCSZ name, LONG rargc, const RXSTRING *rargv,
                    PCSZ queuename, PRXSTRING retstr)
{
    if (rargc != 0)
        return seterr("no argument expected, got %ld", rargc);
    if (!init_perl(1))
        return 1;

    sprintf(retstr->strptr, "%s", "ok");
    retstr->strlength = strlen (retstr->strptr);
    return 0;
}

ULONG
PERLLASTERROR (PCSZ name, LONG rargc, const RXSTRING *rargv, PCSZ queuename, PRXSTRING retstr)
{
    int len = strlen(last_error);

    if (len <= 256			/* Default buffer is 256-char long */
        || !DosAllocMem((PPVOID)&retstr->strptr, len,
                        PAG_READ|PAG_WRITE|PAG_COMMIT)) {
            memcpy(retstr->strptr, last_error, len);
            retstr->strlength = len;
    } else {
        strcpy(retstr->strptr, "[Not enough memory to copy the errortext]");
        retstr->strlength = strlen(retstr->strptr);
    }
    return 0;
}

ULONG
PERLEVAL (PCSZ name, LONG rargc, const RXSTRING *rargv, PCSZ queuename, PRXSTRING retstr)
{
    SV *res, *in;
    STRLEN len, n_a;
    char *str;

    last_error[0] = 0;
    if (rargc != 1)
        return seterr("one argument expected, got %ld", rargc);

    if (!init_perl(1))
        return seterr("error initializing perl");

  {
    dSP;
    int ret;

    ENTER;
    SAVETMPS;

    PUSHMARK(SP);
    in = sv_2mortal(newSVpvn(rargv[0].strptr, rargv[0].strlength));
    eval_sv(in, G_SCALAR);
    SPAGAIN;
    res = POPs;
    PUTBACK;

    ret = 0;
    if (SvTRUE(ERRSV))
        ret = seterr(SvPV(ERRSV, n_a));
    if (!SvOK(res))
        ret = seterr("undefined value returned by Perl-in-REXX");
    str = SvPV(res, len);
    if (len <= 256			/* Default buffer is 256-char long */
        || !DosAllocMem((PPVOID)&retstr->strptr, len,
                        PAG_READ|PAG_WRITE|PAG_COMMIT)) {
            memcpy(retstr->strptr, str, len);
            retstr->strlength = len;
    } else
        ret = seterr("Not enough memory for the return string of Perl-in-REXX");

    FREETMPS;
    LEAVE;

    return ret;
  }
}

ULONG
PERLEVALSUBCOMMAND(
  const RXSTRING    *command,          /* command to issue           */
  PUSHORT      flags,                  /* error/failure flags        */
  PRXSTRING    retstr )                /* return code                */
{
    ULONG rc = PERLEVAL(NULL, 1, command, NULL, retstr);

    if (rc)
        *flags = RXSUBCOM_ERROR;         /* raise error condition    */

    return 0;                            /* finished                   */
}

#define ArrLength(a) (sizeof(a)/sizeof(*(a)))

static const struct {
  char *name;
  RexxFunctionHandler *f;
} funcs[] = {
             {"PERL",			(RexxFunctionHandler *)&PERL},
             {"PERLTERM",		(RexxFunctionHandler *)&PERLTERM},
             {"PERLINIT",		(RexxFunctionHandler *)&PERLINIT},
             {"PERLEXIT",		(RexxFunctionHandler *)&PERLEXIT},
             {"PERLEVAL",		(RexxFunctionHandler *)&PERLEVAL},
             {"PERLLASTERROR",		(RexxFunctionHandler *)&PERLLASTERROR},
             {"PERLDROPALL",		(RexxFunctionHandler *)&PERLDROPALL},
             {"PERLDROPALLEXIT",	(RexxFunctionHandler *)&PERLDROPALLEXIT},
             /* Should be the last entry */
             {"PERLEXPORTALL",		(RexxFunctionHandler *)&PERLEXPORTALL}
          };

ULONG
PERLEXPORTALL(PCSZ name, LONG rargc, const RXSTRING *rargv, PCSZ queuename, PRXSTRING retstr)
{
   int i = -1;

   while (++i < ArrLength(funcs) - 1)
        RexxRegisterFunctionExe(funcs[i].name, funcs[i].f);
   RexxRegisterSubcomExe("EVALPERL", (PFN)&PERLEVALSUBCOMMAND, NULL);
   retstr->strlength = 0;
   return 0;
}

ULONG
PERLDROPALL(PCSZ name, LONG rargc, const RXSTRING *rargv, PCSZ queuename, PRXSTRING retstr)
{
   int i = -1;

   while (++i < ArrLength(funcs))
        RexxDeregisterFunction(funcs[i].name);
   RexxDeregisterSubcom("EVALPERL", NULL /* Not a DLL version */);
   retstr->strlength = 0;
   return 0;
}

ULONG
PERLDROPALLEXIT(PCSZ name, LONG rargc, const RXSTRING *rargv, PCSZ queuename, PRXSTRING retstr)
{
   int i = -1;

   while (++i < ArrLength(funcs))
        RexxDeregisterFunction(funcs[i].name);
   RexxDeregisterSubcom("EVALPERL", NULL /* Not a DLL version */);
   PERL_SYS_TERM1(0);
   retstr->strlength = 0;
   return 0;
}
