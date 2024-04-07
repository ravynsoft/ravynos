#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#define INCL_BASE
#define INCL_REXXSAA
#include <os2emx.h>

#if 0
#define INCL_REXXSAA
#pragma pack(1)
#define _Packed
#include <rexxsaa.h>
#pragma pack()
#endif

extern ULONG _emx_exception (	EXCEPTIONREPORTRECORD *,
				EXCEPTIONREGISTRATIONRECORD *,
                                CONTEXTRECORD *,
                                void *);

static RXSTRING * strs;
static int	  nstrs;
static SHVBLOCK * vars;
static int	  nvars;
static char *	  trace;

/*
static RXSTRING   rxcommand    = {  9, "RXCOMMAND" };
static RXSTRING   rxsubroutine = { 12, "RXSUBROUTINE" };
static RXSTRING   rxfunction   = { 11, "RXFUNCTION" };
*/

static ULONG PERLCALL(PCSZ name, ULONG argc, PRXSTRING argv, PCSZ queue, PRXSTRING ret);
static ULONG PERLCALLcv(PCSZ name, SV *cv, ULONG argc, PRXSTRING argv, PCSZ queue, PRXSTRING ret);
static ULONG PERLSTART(PCSZ name, ULONG argc, PRXSTRING argv, PCSZ queue, PRXSTRING ret);
static RexxSubcomHandler SubCommandPerlEval;

#if 1
 #define Set	RXSHV_SET
 #define Fetch	RXSHV_FETCH
 #define Drop	RXSHV_DROPV
#else
 #define Set	RXSHV_SYSET
 #define Fetch	RXSHV_SYFET
 #define Drop	RXSHV_SYDRO
#endif

static long incompartment;	/* May be used to unload the REXX */

static LONG    APIENTRY (*pRexxStart) (LONG, PRXSTRING, PSZ, PRXSTRING, 
				    PSZ, LONG, PRXSYSEXIT, PSHORT, PRXSTRING);
static APIRET  APIENTRY (*pRexxRegisterFunctionExe) (PSZ,
						  RexxFunctionHandler *);
static APIRET  APIENTRY (*pRexxRegisterSubcomExe)  (PCSZ pszEnvName, PFN pfnEntryPoint,
    PUCHAR pUserArea);
static APIRET  APIENTRY (*pRexxDeregisterFunction) (PSZ);

static ULONG (*pRexxVariablePool) (PSHVBLOCK pRequest);

static SV* exec_cv;

/* Create a REXX compartment,
   register `n' callbacks `handlers' with the REXX names `handlerNames',
   evaluate the REXX expression `cmd'.
 */
static SV*
exec_in_REXX_with(pTHX_ char *cmd, int c, char **handlerNames, RexxFunctionHandler **handlers)
{
    RXSTRING args[1];
    RXSTRING inst[2];
    RXSTRING result;
    USHORT   retcode;
    LONG rc;
    SV *res;
    char *subs = 0;
    int n = c, have_nl = 0;
    char *ocmd = cmd, *s, *t;

    incompartment++;

    if (c)
	Newxz(subs, c, char);
    while (n--) {
	rc = pRexxRegisterFunctionExe(handlerNames[n], handlers[n]);
	if (rc == RXFUNC_DEFINED)
	    subs[n] = 1;
    }

    s = cmd;
    while (*s) {
	if (*s == '\n') {		/* Is not preceded by \r! */
	    Newx(cmd, 2*strlen(cmd)+1, char);
	    s = ocmd;
	    t = cmd;
	    while (*s) {
		if (*s == '\n')
		    *t++ = '\r';
		*t++ = *s++;
	    }
	    *t = 0;
	    break;
	} else if (*s == '\r')
	    s++;
	s++;
    }
    MAKERXSTRING(args[0], NULL, 0);
    MAKERXSTRING(inst[0], cmd,  strlen(cmd));
    MAKERXSTRING(inst[1], NULL, 0);
    MAKERXSTRING(result,  NULL, 0);
    rc = pRexxStart(0, args,		/* No arguments */
		    "REXX_in_Perl",	/* Returned on REXX' PARSE SOURCE,
					   and the "macrospace function name" */
		    inst,		/* inst[0] - the code to execute,
					   inst[1] will contain tokens. */
		    "Perl",		/* Pass string-cmds to this callback */
		    RXSUBROUTINE,	/* Many arguments, maybe result */
		    NULL,		/* No callbacks/exits to register */
		    &retcode, &result);

    incompartment--;
    n = c;
    while (n--)
	if (!subs[n])
	    pRexxDeregisterFunction(handlerNames[n]);
    if (c)
	Safefree(subs);
    if (cmd != ocmd)
	Safefree(cmd);
#if 0					/* Do we want to restore these? */
    DosFreeModule(hRexxAPI);
    DosFreeModule(hRexx);
#endif

    if (RXSTRPTR(inst[1]))		/* Free the tokenized version */
	DosFreeMem(RXSTRPTR(inst[1]));
    if (!RXNULLSTRING(result)) {
	res = newSVpv(RXSTRPTR(result), RXSTRLEN(result));
	DosFreeMem(RXSTRPTR(result));
    } else {
	res = newSV(0);
    }
    if (rc || SvTRUE(GvSV(PL_errgv))) {
	if (SvTRUE(GvSV(PL_errgv))) {
	    STRLEN n_a;
	    Perl_croak(aTHX_ "Error inside perl function called from REXX compartment:\n%s", SvPV(GvSV(PL_errgv), n_a)) ;
	}
	Perl_croak(aTHX_ "REXX compartment returned non-zero status %li", rc);
    }

    return res;
}

/* Call the Perl function given by name, or if name=0, by cv,
   with the given arguments.  Return the stringified result to REXX. */
static ULONG
PERLCALLcv(PCSZ name, SV *cv, ULONG argc, PRXSTRING argv, PCSZ queue, PRXSTRING ret)
{
    dTHX;
    EXCEPTIONREGISTRATIONRECORD xreg = { NULL, _emx_exception };
    int i, rc;
    unsigned long len;
    char *str;
    SV *res;
    dSP;

    DosSetExceptionHandler(&xreg);

    ENTER;
    SAVETMPS;
    PUSHMARK(SP);

#if 0
    if (!my_perl) {
	DosUnsetExceptionHandler(&xreg);
	return 1;
    }
#endif 

    for (i = 0; i < argc; ++i)
	XPUSHs(sv_2mortal(newSVpvn(argv[i].strptr, argv[i].strlength)));
    PUTBACK;
    if (name)
	rc = perl_call_pv(name, G_SCALAR | G_EVAL);
    else if (cv)
	rc = perl_call_sv(cv, G_SCALAR | G_EVAL);
    else
	rc = -1;

    SPAGAIN;

    if (rc == 1)			/* must be! */
	res = POPs;
    if (rc == 1 && SvOK(res)) { 
	str = SvPVx(res, len);
	if (len <= 256			/* Default buffer is 256-char long */
	    || !CheckOSError(DosAllocMem((PPVOID)&ret->strptr, len,
					PAG_READ|PAG_WRITE|PAG_COMMIT))) {
	    memcpy(ret->strptr, str, len);
	    ret->strlength = len;
	} else
	    rc = 0;
    } else
	rc = 0;

    PUTBACK ;
    FREETMPS ;
    LEAVE ;

    DosUnsetExceptionHandler(&xreg);
    return rc == 1 ? 0 : 1;			/* 0 means SUCCESS */
}

static ULONG
PERLSTART(PCSZ name, ULONG argc, PRXSTRING argv, PCSZ queue, PRXSTRING ret)
{
    SV *cv = exec_cv;

    exec_cv = NULL;
    return PERLCALLcv(NULL, cv, argc, argv, queue, ret);
}

static ULONG
PERLCALL(PCSZ name, ULONG argc, PRXSTRING argv, PCSZ queue, PRXSTRING ret)
{
  return PERLCALLcv(name, NULL, argc, argv, queue, ret);
}

RexxFunctionHandler* PF = &PERLSTART;
char* PF_name = "StartPerl";

#define REXX_eval_with(cmd,name,cv)	\
	( exec_cv = cv, exec_in_REXX_with(aTHX_ (cmd),1, &(name), &PF))
#define REXX_call(cv) REXX_eval_with("return StartPerl()\r\n", PF_name, (cv))
#define REXX_eval(cmd) ( exec_in_REXX_with(aTHX_ (cmd), 0, NULL, NULL))

static ULONG
SubCommandPerlEval(
  PRXSTRING    command,                /* command to issue           */
  PUSHORT      flags,                  /* error/failure flags        */
  PRXSTRING    retstr )                /* return code                */
{
    dSP;
    STRLEN len;
    int ret;
    char *str = 0;
    SV *in, *res;

    ENTER;
    SAVETMPS;

    PUSHMARK(SP);
    in = sv_2mortal(newSVpvn(command->strptr, command->strlength));
    eval_sv(in, G_SCALAR);
    SPAGAIN;
    res = POPs;
    PUTBACK;

    ret = 0;
    if (SvTRUE(ERRSV)) {
	*flags = RXSUBCOM_ERROR;         /* raise error condition    */
	str = SvPV(ERRSV, len);
    } else if (!SvOK(res)) {
	*flags = RXSUBCOM_ERROR;         /* raise error condition    */
	str = "undefined value returned by Perl-in-REXX";
        len = strlen(str);
    } else
	str = SvPV(res, len);
    if (len <= 256			/* Default buffer is 256-char long */
	|| !DosAllocMem((PPVOID)&retstr->strptr, len,
			PAG_READ|PAG_WRITE|PAG_COMMIT)) {
	    memcpy(retstr->strptr, str, len);
	    retstr->strlength = len;
    } else {
	*flags = RXSUBCOM_ERROR;         /* raise error condition    */
	strcpy(retstr->strptr, "Not enough memory for the return string of Perl-in-REXX");
	retstr->strlength = strlen(retstr->strptr);
    }

    FREETMPS;
    LEAVE;

    return 0;                            /* finished                   */
}

static void
needstrs(int n)
{
    if (n > nstrs) {
	if (strs)
	    free(strs);
	nstrs = 2 * n;
	strs = malloc(nstrs * sizeof(RXSTRING));
    }
}

static void
needvars(int n)
{
    if (n > nvars) {
	if (vars)
	    free(vars);
	nvars = 2 * n;
	vars = malloc(nvars * sizeof(SHVBLOCK));
    }
}

static void
initialize(void)
{
    ULONG rc;
    *(PFN *)&pRexxStart = loadByOrdinal(ORD_RexxStart, 1);
    *(PFN *)&pRexxRegisterFunctionExe
	= loadByOrdinal(ORD_RexxRegisterFunctionExe, 1);
    *(PFN *)&pRexxDeregisterFunction
	= loadByOrdinal(ORD_RexxDeregisterFunction, 1);
    *(PFN *)&pRexxVariablePool = loadByOrdinal(ORD_RexxVariablePool, 1);
    *(PFN *)&pRexxRegisterSubcomExe
	= loadByOrdinal(ORD_RexxRegisterSubcomExe, 1);
    needstrs(8);
    needvars(8);
    trace = getenv("PERL_REXX_DEBUG");
     
    rc = pRexxRegisterSubcomExe("PERLEVAL", (PFN)&SubCommandPerlEval, NULL);
}

static int
constant(char *name, int arg)
{
    errno = EINVAL;
    return 0;
}


MODULE = OS2::REXX		PACKAGE = OS2::REXX

BOOT:
	initialize();

int
constant(name,arg)
	char *		name
	int		arg

int
_set(name,value,...)
	char *		name
	char *		value
 CODE:
   {
       int   i;
       int   n = (items + 1) / 2;
       ULONG rc;
       needvars(n);
       if (trace)
	   fprintf(stderr, "REXXCALL::_set");
       for (i = 0; i < n; ++i) {
	   SHVBLOCK * var = &vars[i];
	   STRLEN     namelen;
	   STRLEN     valuelen;
	   name = SvPV(ST(2*i+0),namelen);
	   if (2*i+1 < items) {
	       value = SvPV(ST(2*i+1),valuelen);
	   }
	   else {
	       value = "";
	       valuelen = 0;
	   }
	   var->shvcode = RXSHV_SET;
	   var->shvnext = &vars[i+1];
	   var->shvnamelen = namelen;
	   var->shvvaluelen = valuelen;
	   MAKERXSTRING(var->shvname, name, namelen);
	   MAKERXSTRING(var->shvvalue, value, valuelen);
	   if (trace)
	       fprintf(stderr, " %.*s='%.*s'",
		       (int)var->shvname.strlength, var->shvname.strptr,
		       (int)var->shvvalue.strlength, var->shvvalue.strptr);
       }
       if (trace)
	   fprintf(stderr, "\n");
       vars[n-1].shvnext = NULL;
       rc = pRexxVariablePool(vars);
       if (trace)
	   fprintf(stderr, "  rc=%#lX\n", rc);
       RETVAL = (rc & ~RXSHV_NEWV) ? FALSE : TRUE;
   }
 OUTPUT:
    RETVAL

void
_fetch(name, ...)
	char *		name
 PPCODE:
   {
       int   i;
       ULONG rc;
       EXTEND(SP, items);
       needvars(items);
       if (trace)
	   fprintf(stderr, "REXXCALL::_fetch");
       for (i = 0; i < items; ++i) {
	   SHVBLOCK * var = &vars[i];
	   STRLEN     namelen;
	   name = SvPV(ST(i),namelen);
	   var->shvcode = RXSHV_FETCH;
	   var->shvnext = &vars[i+1];
	   var->shvnamelen = namelen;
	   var->shvvaluelen = 0;
	   MAKERXSTRING(var->shvname, name, namelen);
	   MAKERXSTRING(var->shvvalue, NULL, 0);
	   if (trace)
	       fprintf(stderr, " '%s'", name);
       }
       if (trace)
	   fprintf(stderr, "\n");
       vars[items-1].shvnext = NULL;
       rc = pRexxVariablePool(vars);
       if (!(rc & ~RXSHV_NEWV)) {
	   for (i = 0; i < items; ++i) {
	       int namelen;
	       SHVBLOCK * var = &vars[i];
	       /* returned lengths appear to be swapped */
	       /* but beware of "future bug fixes" */
	       namelen = var->shvvalue.strlength; /* should be */
	       if (var->shvvaluelen < var->shvvalue.strlength)
		   namelen = var->shvvaluelen; /* is */
	       if (trace)
		   fprintf(stderr, "  %.*s='%.*s'\n",
			   (int)var->shvname.strlength, var->shvname.strptr,
			   namelen, var->shvvalue.strptr);
	       if (var->shvret & RXSHV_NEWV || !var->shvvalue.strptr)
		   PUSHs(&PL_sv_undef);
	       else
		   PUSHs(sv_2mortal(newSVpv(var->shvvalue.strptr,
					    namelen)));
	   }
       } else {
	   if (trace)
	       fprintf(stderr, "  rc=%#lX\n", rc);
       }
   }

void
_next(stem)
	char *	stem
 PPCODE:
   {
       SHVBLOCK sv;
       BYTE     name[4096];
       ULONG    rc;
       int      len = strlen(stem), namelen, valuelen;
       if (trace)
	   fprintf(stderr, "REXXCALL::_next stem='%s'\n", stem);
       sv.shvcode = RXSHV_NEXTV;
       sv.shvnext = NULL;
       MAKERXSTRING(sv.shvvalue, NULL, 0);
       do {
	   sv.shvnamelen = sizeof name;
	   sv.shvvaluelen = 0;
	   MAKERXSTRING(sv.shvname, name, sizeof name);
	   if (sv.shvvalue.strptr) {
	       DosFreeMem(sv.shvvalue.strptr);
	       MAKERXSTRING(sv.shvvalue, NULL, 0);
	   }
	   rc = pRexxVariablePool(&sv);
       } while (!rc && memcmp(stem, sv.shvname.strptr, len) != 0);
       if (!rc) {
	   EXTEND(SP, 2);
	   /* returned lengths appear to be swapped */
	   /* but beware of "future bug fixes" */
	   namelen = sv.shvname.strlength; /* should be */
	   if (sv.shvnamelen < sv.shvname.strlength)
	       namelen = sv.shvnamelen; /* is */
	   valuelen = sv.shvvalue.strlength; /* should be */
	   if (sv.shvvaluelen < sv.shvvalue.strlength)
	       valuelen = sv.shvvaluelen; /* is */
	   if (trace)
	       fprintf(stderr, "  %.*s='%.*s'\n",
		       namelen, sv.shvname.strptr,
		       valuelen, sv.shvvalue.strptr);
	   PUSHs(sv_2mortal(newSVpv(sv.shvname.strptr+len, namelen-len)));
	   if (sv.shvvalue.strptr) {
	       PUSHs(sv_2mortal(newSVpv(sv.shvvalue.strptr, valuelen)));
				DosFreeMem(sv.shvvalue.strptr);
	   } else	
	       PUSHs(&PL_sv_undef);
       } else if (rc != RXSHV_LVAR) {
	   die("Error %i when in _next", rc);
       } else {
	   if (trace)
	       fprintf(stderr, "  rc=%#lX\n", rc);
       }
   }

int
_drop(name,...)
	char *		name
 CODE:
   {
       int i;
       needvars(items);
       for (i = 0; i < items; ++i) {
	   SHVBLOCK * var = &vars[i];
	   STRLEN     namelen;
	   name = SvPV(ST(i),namelen);
	   var->shvcode = RXSHV_DROPV;
	   var->shvnext = &vars[i+1];
	   var->shvnamelen = namelen;
	   var->shvvaluelen = 0;
	   MAKERXSTRING(var->shvname, name, var->shvnamelen);
	   MAKERXSTRING(var->shvvalue, NULL, 0);
       }
       vars[items-1].shvnext = NULL;
       RETVAL = (pRexxVariablePool(vars) & ~RXSHV_NEWV) ? FALSE : TRUE;
   }
 OUTPUT:
    RETVAL

int
_register(name)
	char *	name
 CODE:
    RETVAL = pRexxRegisterFunctionExe(name, PERLCALL);
 OUTPUT:
    RETVAL

SV*
REXX_call(cv)
	SV *cv
  PROTOTYPE: &

SV*
REXX_eval(cmd)
	char *cmd

SV*
REXX_eval_with(cmd,name,cv)
	char *cmd
	char *name
	SV *cv

#ifdef THIS_IS_NOT_FINISHED

SV*
_REXX_eval_with(cmd,...)
	char *cmd
 CODE:
   {
	int n = (items - 1)/2;
	char **names;
	SV **cvs;

	if ((items % 2) == 0)
	    Perl_croak(aTHX_ "Name/values should come in pairs in REXX_eval_with()");
	Newx(names, n, char*);
	Newx(cvs, n, SV*);
	/* XXX Unfinished... */
	RETVAL = NULL;
	Safefree(names);
	Safefree(cvs);
   }
 OUTPUT:
    RETVAL

#endif
