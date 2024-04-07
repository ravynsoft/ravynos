#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#define INCL_BASE
#define INCL_REXXSAA
#include <os2emx.h>

static RXSTRING * strs;
static int	  nstrs;
static char *	  trace;

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

typedef ULONG (*fptr_UL_20)(ULONG, ULONG, ULONG, ULONG, ULONG, ULONG, ULONG, ULONG, ULONG, ULONG, ULONG, ULONG, ULONG, ULONG, ULONG, ULONG, ULONG, ULONG, ULONG, ULONG);
typedef __attribute__((regparm(3))) ULONG (*fptr_UL_20_rp3)(ULONG, ULONG, ULONG, ULONG, ULONG, ULONG, ULONG, ULONG, ULONG, ULONG, ULONG, ULONG, ULONG, ULONG, ULONG, ULONG, ULONG, ULONG, ULONG, ULONG);

static inline unsigned long
call20_p(unsigned long fp, char* str)
{
  ULONG *argv = (ULONG*)str;
  fptr_UL_20 f = (fptr_UL_20)fp;

  return f(argv[0], argv[1], argv[2], argv[3], argv[4], argv[5], argv[6], argv[7], argv[8], argv[9], argv[10], argv[11], argv[12], argv[13], argv[14], argv[15], argv[16], argv[17], argv[18], argv[19]);
}

static inline unsigned long
call20(unsigned long fp, unsigned long arg0, unsigned long arg1, unsigned long arg2, unsigned long arg3, unsigned long arg4, unsigned long arg5, unsigned long arg6, unsigned long arg7, unsigned long arg8, unsigned long arg9, unsigned long arg10, unsigned long arg11, unsigned long arg12, unsigned long arg13, unsigned long arg14, unsigned long arg15, unsigned long arg16, unsigned long arg17, unsigned long arg18, unsigned long arg19)
{
  fptr_UL_20 f = (fptr_UL_20)fp;

  return f(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12, arg13, arg14, arg15, arg16, arg17, arg18, arg19);
}

static inline unsigned long
call20_rp3_p(unsigned long fp, char* str)
{
  ULONG *argv = (ULONG*)str;
  fptr_UL_20_rp3 f = (fptr_UL_20_rp3)fp;

  return f(argv[0], argv[1], argv[2], argv[3], argv[4], argv[5], argv[6], argv[7], argv[8], argv[9], argv[10], argv[11], argv[12], argv[13], argv[14], argv[15], argv[16], argv[17], argv[18], argv[19]);
}

static inline unsigned long
call20_rp3(unsigned long fp, unsigned long arg0, unsigned long arg1, unsigned long arg2, unsigned long arg3, unsigned long arg4, unsigned long arg5, unsigned long arg6, unsigned long arg7, unsigned long arg8, unsigned long arg9, unsigned long arg10, unsigned long arg11, unsigned long arg12, unsigned long arg13, unsigned long arg14, unsigned long arg15, unsigned long arg16, unsigned long arg17, unsigned long arg18, unsigned long arg19)
{
  fptr_UL_20_rp3 f = (fptr_UL_20_rp3)fp;

  return f(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12, arg13, arg14, arg15, arg16, arg17, arg18, arg19);
}

static inline void
call20_Dos(char *msg, unsigned long fp, unsigned long arg0, unsigned long arg1, unsigned long arg2, unsigned long arg3, unsigned long arg4, unsigned long arg5, unsigned long arg6, unsigned long arg7, unsigned long arg8, unsigned long arg9, unsigned long arg10, unsigned long arg11, unsigned long arg12, unsigned long arg13, unsigned long arg14, unsigned long arg15, unsigned long arg16, unsigned long arg17, unsigned long arg18, unsigned long arg19)
{
  fptr_UL_20 f = (fptr_UL_20)fp;
  ULONG rc;

  if (CheckOSError(f(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12, arg13, arg14, arg15, arg16, arg17, arg18, arg19)))
    croak_with_os2error(msg);
}

static inline unsigned long
call20_Win(char *msg, unsigned long fp, unsigned long arg0, unsigned long arg1, unsigned long arg2, unsigned long arg3, unsigned long arg4, unsigned long arg5, unsigned long arg6, unsigned long arg7, unsigned long arg8, unsigned long arg9, unsigned long arg10, unsigned long arg11, unsigned long arg12, unsigned long arg13, unsigned long arg14, unsigned long arg15, unsigned long arg16, unsigned long arg17, unsigned long arg18, unsigned long arg19)
{
  fptr_UL_20 f = (fptr_UL_20)fp;

  if (CheckWinError(f(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12, arg13, arg14, arg15, arg16, arg17, arg18, arg19)))
    croak_with_os2error(msg);
}

static inline unsigned long
call20_Win_0OK(char *msg, unsigned long fp, unsigned long arg0, unsigned long arg1, unsigned long arg2, unsigned long arg3, unsigned long arg4, unsigned long arg5, unsigned long arg6, unsigned long arg7, unsigned long arg8, unsigned long arg9, unsigned long arg10, unsigned long arg11, unsigned long arg12, unsigned long arg13, unsigned long arg14, unsigned long arg15, unsigned long arg16, unsigned long arg17, unsigned long arg18, unsigned long arg19)
{
  fptr_UL_20 f = (fptr_UL_20)fp;

  ResetWinError();
  return SaveCroakWinError(f(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12, arg13, arg14, arg15, arg16, arg17, arg18, arg19),
			   1 /* Die on error */, /* No prefix */, msg);
}

static inline unsigned long
call20_Win_0OK_survive(unsigned long fp, unsigned long arg0, unsigned long arg1, unsigned long arg2, unsigned long arg3, unsigned long arg4, unsigned long arg5, unsigned long arg6, unsigned long arg7, unsigned long arg8, unsigned long arg9, unsigned long arg10, unsigned long arg11, unsigned long arg12, unsigned long arg13, unsigned long arg14, unsigned long arg15, unsigned long arg16, unsigned long arg17, unsigned long arg18, unsigned long arg19)
{
  fptr_UL_20 f = (fptr_UL_20)fp;

  ResetWinError();
  return SaveCroakWinError(f(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12, arg13, arg14, arg15, arg16, arg17, arg18, arg19),
			   0 /* No die on error */, /* No prefix */, "N/A");
}

MODULE = OS2::DLL		PACKAGE = OS2::DLL

BOOT:
    needstrs(8);
    trace = getenv("PERL_REXX_DEBUG");

unsigned long
call20_p(unsigned long fp, char* argv)

unsigned long
call20(unsigned long fp, unsigned long arg0 = 0, unsigned long arg1 = 0, unsigned long arg2 = 0, unsigned long arg3 = 0, unsigned long arg4 = 0, unsigned long arg5 = 0, unsigned long arg6 = 0, unsigned long arg7 = 0, unsigned long arg8 = 0, unsigned long arg9 = 0, unsigned long arg10 = 0, unsigned long arg11 = 0, unsigned long arg12 = 0, unsigned long arg13 = 0, unsigned long arg14 = 0, unsigned long arg15 = 0, unsigned long arg16 = 0, unsigned long arg17 = 0, unsigned long arg18 = 0, unsigned long arg19 = 0)

void
call20_Dos(char* msg, unsigned long fp, unsigned long arg0 = 0, unsigned long arg1 = 0, unsigned long arg2 = 0, unsigned long arg3 = 0, unsigned long arg4 = 0, unsigned long arg5 = 0, unsigned long arg6 = 0, unsigned long arg7 = 0, unsigned long arg8 = 0, unsigned long arg9 = 0, unsigned long arg10 = 0, unsigned long arg11 = 0, unsigned long arg12 = 0, unsigned long arg13 = 0, unsigned long arg14 = 0, unsigned long arg15 = 0, unsigned long arg16 = 0, unsigned long arg17 = 0, unsigned long arg18 = 0, unsigned long arg19 = 0)

unsigned long
call20_Win(char *msg, unsigned long fp, unsigned long arg0 = 0, unsigned long arg1 = 0, unsigned long arg2 = 0, unsigned long arg3 = 0, unsigned long arg4 = 0, unsigned long arg5 = 0, unsigned long arg6 = 0, unsigned long arg7 = 0, unsigned long arg8 = 0, unsigned long arg9 = 0, unsigned long arg10 = 0, unsigned long arg11 = 0, unsigned long arg12 = 0, unsigned long arg13 = 0, unsigned long arg14 = 0, unsigned long arg15 = 0, unsigned long arg16 = 0, unsigned long arg17 = 0, unsigned long arg18 = 0, unsigned long arg19 = 0)

unsigned long
call20_Win_0OK(char *msg, unsigned long fp, unsigned long arg0 = 0, unsigned long arg1 = 0, unsigned long arg2 = 0, unsigned long arg3 = 0, unsigned long arg4 = 0, unsigned long arg5 = 0, unsigned long arg6 = 0, unsigned long arg7 = 0, unsigned long arg8 = 0, unsigned long arg9 = 0, unsigned long arg10 = 0, unsigned long arg11 = 0, unsigned long arg12 = 0, unsigned long arg13 = 0, unsigned long arg14 = 0, unsigned long arg15 = 0, unsigned long arg16 = 0, unsigned long arg17 = 0, unsigned long arg18 = 0, unsigned long arg19 = 0)

unsigned long
call20_Win_0OK_survive(unsigned long fp, unsigned long arg0 = 0, unsigned long arg1 = 0, unsigned long arg2 = 0, unsigned long arg3 = 0, unsigned long arg4 = 0, unsigned long arg5 = 0, unsigned long arg6 = 0, unsigned long arg7 = 0, unsigned long arg8 = 0, unsigned long arg9 = 0, unsigned long arg10 = 0, unsigned long arg11 = 0, unsigned long arg12 = 0, unsigned long arg13 = 0, unsigned long arg14 = 0, unsigned long arg15 = 0, unsigned long arg16 = 0, unsigned long arg17 = 0, unsigned long arg18 = 0, unsigned long arg19 = 0)

unsigned long
call20_rp3_p(unsigned long fp, char* argv)

unsigned long
call20_rp3(unsigned long fp, unsigned long arg0 = 0, unsigned long arg1 = 0, unsigned long arg2 = 0, unsigned long arg3 = 0, unsigned long arg4 = 0, unsigned long arg5 = 0, unsigned long arg6 = 0, unsigned long arg7 = 0, unsigned long arg8 = 0, unsigned long arg9 = 0, unsigned long arg10 = 0, unsigned long arg11 = 0, unsigned long arg12 = 0, unsigned long arg13 = 0, unsigned long arg14 = 0, unsigned long arg15 = 0, unsigned long arg16 = 0, unsigned long arg17 = 0, unsigned long arg18 = 0, unsigned long arg19 = 0)

SV *
_call(name, address, queue="SESSION", ...)
	char *		name
	void *		address
	char *		queue
 CODE:
   {
       ULONG	rc;
       int	argc, i;
       RXSTRING	result;
       UCHAR	resbuf[256];
       RexxFunctionHandler *fcn = address;
       argc = items-3;
       needstrs(argc);
       if (trace)
	   fprintf(stderr, "REXXCALL::_call name: '%s' args:", name);
       for (i = 0; i < argc; ++i) {
	   STRLEN len;
	   char *ptr = SvPV(ST(3+i), len);
	   MAKERXSTRING(strs[i], ptr, len);
	   if (trace)
	       fprintf(stderr, " '%.*s'", len, ptr);
       }
       if (!*queue)
	   queue = "SESSION";
       if (trace)
	   fprintf(stderr, "\n");
       MAKERXSTRING(result, resbuf, sizeof resbuf);
       rc = fcn(name, argc, strs, queue, &result);
       if (trace)
	   fprintf(stderr, "  rc=%X, result='%.*s'\n", rc,
		   result.strlength, result.strptr);
       ST(0) = sv_newmortal();
       if (rc == 0) {
	   if (result.strptr)
	       sv_setpvn(ST(0), result.strptr, result.strlength);
	   else
	       SvPVCLEAR(ST(0));
       }
       if (result.strptr && result.strptr != resbuf)
	   DosFreeMem(result.strptr);
   }

