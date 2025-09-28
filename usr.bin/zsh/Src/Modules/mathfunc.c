/*
 * mathfunc.c - basic mathematical functions for use in math evaluations
 *
 * This file is part of zsh, the Z shell.
 *
 * Copyright (c) 1999 Peter Stephenson
 * All rights reserved.
 *
 * Permission is hereby granted, without written agreement and without
 * license or royalty fees, to use, copy, modify, and distribute this
 * software and to distribute modified versions of this software for any
 * purpose, provided that the above copyright notice and the following
 * two paragraphs appear in all copies of this software.
 *
 * In no event shall Peter Stephenson or the Zsh Development Group be liable
 * to any party for direct, indirect, special, incidental, or consequential
 * damages arising out of the use of this software and its documentation,
 * even if Peter Stephenson and the Zsh Development Group have been advised of
 * the possibility of such damage.
 *
 * Peter Stephenson and the Zsh Development Group specifically disclaim any
 * warranties, including, but not limited to, the implied warranties of
 * merchantability and fitness for a particular purpose.  The software
 * provided hereunder is on an "as is" basis, and Peter Stephenson and the
 * Zsh Development Group have no obligation to provide maintenance,
 * support, updates, enhancements, or modifications.
 *
 */

#include "mathfunc.mdh"
#include "mathfunc.pro"

#include <math.h>

enum {
MF_ABS,
MF_ACOS,
MF_ACOSH,
MF_ASIN,
MF_ASINH,
MF_ATAN,
MF_ATANH,
MF_CBRT,
MF_CEIL,
MF_COPYSIGN,
MF_COS,
MF_COSH,
MF_ERF,
MF_ERFC,
MF_EXP,
MF_EXPM1,
MF_FABS,
MF_FLOAT,
MF_FLOOR,
MF_FMOD,
MF_GAMMA,
MF_HYPOT,
MF_ILOGB,
MF_INT,
MF_J0,
MF_J1,
MF_JN,
MF_LDEXP,
MF_LGAMMA,
MF_LOG,
MF_LOG10,
MF_LOG1P,
MF_LOG2,
MF_LOGB,
MF_NEXTAFTER,
MF_RINT,
MF_SCALB,
#ifdef HAVE_SIGNGAM
MF_SIGNGAM,
#endif
MF_SIN,
MF_SINH,
MF_SQRT,
MF_TAN,
MF_TANH,
MF_Y0,
MF_Y1,
MF_YN
};

/* also functions taking a string argument */

enum {
MS_RAND48
};

/*
 * also to do, but differently argument or returned: abs (no type
 * conversion), atan2.
 */

/*
 * Flags for type of function: unlike the above, these must
 * be individually bit-testable.
 */

enum {
    TF_NOCONV = 1,		/* don't convert to float */
    TF_INT1   = 2,		/* first argument is integer */
    TF_INT2   = 4,		/* second argument is integer */
    TF_NOASS  = 8		/* don't assign result as double */
};

#define TFLAG(x) ((x) << 8)


static struct mathfunc mftab[] = {
  NUMMATHFUNC("abs", math_func, 1, 1, MF_ABS |
	      TFLAG(TF_NOCONV|TF_NOASS)),
  NUMMATHFUNC("acos", math_func, 1, 1, MF_ACOS),
  NUMMATHFUNC("acosh", math_func, 1, 1, MF_ACOSH),
  NUMMATHFUNC("asin", math_func, 1, 1, MF_ASIN),
  NUMMATHFUNC("asinh", math_func, 1, 1, MF_ASINH),
  NUMMATHFUNC("atan", math_func, 1, 2, MF_ATAN),
  NUMMATHFUNC("atanh", math_func, 1, 1, MF_ATANH),
  NUMMATHFUNC("cbrt", math_func, 1, 1, MF_CBRT),
  NUMMATHFUNC("ceil", math_func, 1, 1, MF_CEIL),
  NUMMATHFUNC("copysign", math_func, 2, 2, MF_COPYSIGN),
  NUMMATHFUNC("cos", math_func, 1, 1, MF_COS),
  NUMMATHFUNC("cosh", math_func, 1, 1, MF_COSH),
  NUMMATHFUNC("erf", math_func, 1, 1, MF_ERF),
  NUMMATHFUNC("erfc", math_func, 1, 1, MF_ERFC),
  NUMMATHFUNC("exp", math_func, 1, 1, MF_EXP),
  NUMMATHFUNC("expm1", math_func, 1, 1, MF_EXPM1),
  NUMMATHFUNC("fabs", math_func, 1, 1, MF_FABS),
  NUMMATHFUNC("float", math_func, 1, 1, MF_FLOAT),
  NUMMATHFUNC("floor", math_func, 1, 1, MF_FLOOR),
  NUMMATHFUNC("fmod", math_func, 2, 2, MF_FMOD),
  NUMMATHFUNC("gamma", math_func, 1, 1, MF_GAMMA),
  NUMMATHFUNC("hypot", math_func, 2, 2, MF_HYPOT),
  NUMMATHFUNC("ilogb", math_func, 1, 1, MF_ILOGB | TFLAG(TF_NOASS)),
  NUMMATHFUNC("int", math_func, 1, 1, MF_INT | TFLAG(TF_NOASS)),
  NUMMATHFUNC("j0", math_func, 1, 1, MF_J0),
  NUMMATHFUNC("j1", math_func, 1, 1, MF_J1),
  NUMMATHFUNC("jn", math_func, 2, 2, MF_JN | TFLAG(TF_INT1)),
  NUMMATHFUNC("ldexp", math_func, 2, 2, MF_LDEXP | TFLAG(TF_INT2)),
  NUMMATHFUNC("lgamma", math_func, 1, 1, MF_LGAMMA),
  NUMMATHFUNC("log", math_func, 1, 1, MF_LOG),
  NUMMATHFUNC("log10", math_func, 1, 1, MF_LOG10),
  NUMMATHFUNC("log1p", math_func, 1, 1, MF_LOG1P),
  NUMMATHFUNC("log2", math_func, 1, 1, MF_LOG2),
  NUMMATHFUNC("logb", math_func, 1, 1, MF_LOGB),
  NUMMATHFUNC("nextafter", math_func, 2, 2, MF_NEXTAFTER),
#ifdef HAVE_ERAND48
  STRMATHFUNC("rand48", math_string, MS_RAND48),
#endif
  NUMMATHFUNC("rint", math_func, 1, 1, MF_RINT),
  NUMMATHFUNC("scalb", math_func, 2, 2, MF_SCALB | TFLAG(TF_INT2)),
#ifdef HAVE_SIGNGAM
  NUMMATHFUNC("signgam", math_func, 0, 0, MF_SIGNGAM | TFLAG(TF_NOASS)),
#endif
  NUMMATHFUNC("sin", math_func, 1, 1, MF_SIN),
  NUMMATHFUNC("sinh", math_func, 1, 1, MF_SINH),
  NUMMATHFUNC("sqrt", math_func, 1, 1, MF_SQRT),
  NUMMATHFUNC("tan", math_func, 1, 1, MF_TAN),
  NUMMATHFUNC("tanh", math_func, 1, 1, MF_TANH),
  NUMMATHFUNC("y0", math_func, 1, 1, MF_Y0),
  NUMMATHFUNC("y1", math_func, 1, 1, MF_Y1),
  NUMMATHFUNC("yn", math_func, 2, 2, MF_YN | TFLAG(TF_INT1))
};

/**/
static mnumber
math_func(UNUSED(char *name), int argc, mnumber *argv, int id)
{
  mnumber ret;
  double argd = 0, argd2 = 0, retd = 0;
  int argi = 0;

  if (argc && !(id & TFLAG(TF_NOCONV))) {
      if (id & TFLAG(TF_INT1))
	  argi = (argv->type == MN_FLOAT) ? (zlong)argv->u.d : argv->u.l;
      else
	  argd = (argv->type == MN_INTEGER) ? (double)argv->u.l : argv->u.d;
      if (argc > 1) {
	  if (id & TFLAG(TF_INT2))
	      argi = (argv[1].type == MN_FLOAT) ? (zlong)argv[1].u.d :
	      argv[1].u.l;
	  else
	      argd2 = (argv[1].type == MN_INTEGER) ? (double)argv[1].u.l :
	      argv[1].u.d;
      }
  }

  ret.type = MN_FLOAT;
  ret.u.d = 0;

  if (errflag)
    return ret;

  switch (id & 0xff) {
  case MF_ABS:
      ret.type = argv->type;
      if (argv->type == MN_INTEGER)
	  ret.u.l = (argv->u.l < 0) ? - argv->u.l : argv->u.l;
      else
	  ret.u.d = fabs(argv->u.d);
      break;

  case MF_ACOS:
      retd = acos(argd);
      break;

  case MF_ACOSH:
      retd = acosh(argd);
      break;

  case MF_ASIN:
      retd = asin(argd);
      break;

  case MF_ASINH:
      retd = asinh(argd);
      break;

  case MF_ATAN:
      if (argc == 2)
	  retd = atan2(argd, argd2);
      else
	  retd = atan(argd);
      break;

  case MF_ATANH:
      retd = atanh(argd);
      break;

  case MF_CBRT:
      retd = cbrt(argd);
      break;

  case MF_CEIL:
      retd = ceil(argd);
      break;

  case MF_COPYSIGN:
      retd = copysign(argd, argd2);
      break;

  case MF_COS:
      retd = cos(argd);
      break;

  case MF_COSH:
      retd = cosh(argd);
      break;

  case MF_ERF:
      retd = erf(argd);
      break;

  case MF_ERFC:
      retd = erfc(argd);
      break;

  case MF_EXP:
      retd = exp(argd);
      break;

  case MF_EXPM1:
      retd = expm1(argd);
      break;

  case MF_FABS:
      retd = fabs(argd);
      break;

  case MF_FLOAT:
      retd = argd;
      break;

  case MF_FLOOR:
      retd = floor(argd);
      break;

  case MF_FMOD:
      retd = fmod(argd, argd2);
      break;

  case MF_GAMMA:
#ifdef HAVE_TGAMMA
      retd = tgamma(argd);
#else
#ifdef HAVE_SIGNGAM
      retd = lgamma(argd);
      retd = signgam*exp(retd);
#else /*XXX assume gamma(x) returns Gamma(x), not log(|Gamma(x)|) */
      retd = gamma(argd);
#endif
#endif
      break;

  case MF_HYPOT:
      retd = hypot(argd, argd2);
      break;

  case MF_ILOGB:
      ret.type = MN_INTEGER;
      ret.u.l = ilogb(argd); 
      break;

  case MF_INT:
      ret.type = MN_INTEGER;
      ret.u.l = (zlong)argd;
      break;

  case MF_J0:
      retd = j0(argd);
      break;

  case MF_J1:
      retd = j1(argd);
      break;

  case MF_JN:
      retd = jn(argi, argd2);
      break;

  case MF_LDEXP:
      retd = ldexp(argd, argi);
      break;

  case MF_LGAMMA:
      retd = lgamma(argd);
      break;

  case MF_LOG:
      retd = log(argd);
      break;

  case MF_LOG10:
      retd = log10(argd);
      break;

  case MF_LOG1P:
      retd = log1p(argd);
      break;

  case MF_LOG2:
#ifdef HAVE_LOG2
      retd = log2(argd);
#else
      retd = log(argd) / log(2);
#endif
      break;

  case MF_LOGB:
      retd = logb(argd); 
      break;

  case MF_NEXTAFTER:
      retd = nextafter(argd, argd2);
      break;

  case MF_RINT:
      retd = rint(argd);
      break;

  case MF_SCALB:
#ifdef HAVE_SCALBN
      retd = scalbn(argd, argi);
#else
      retd = scalb(argd, argi);
#endif
      break;

#ifdef HAVE_SIGNGAM
  case MF_SIGNGAM:
      ret.type = MN_INTEGER;
      ret.u.l = signgam;
      break;
#endif

  case MF_SIN:
      retd = sin(argd);
      break;

  case MF_SINH:
      retd = sinh(argd);
      break;

  case MF_SQRT:
      retd = sqrt(argd);
      break;

  case MF_TAN:
      retd = tan(argd);
      break;

  case MF_TANH:
      retd = tanh(argd);
      break;

  case MF_Y0:
      retd = y0(argd);
      break;

  case MF_Y1:
      retd = y1(argd);
      break;

  case MF_YN:
      retd = yn(argi, argd2);
      break;

#ifdef DEBUG
  default:
      fprintf(stderr, "BUG: mathfunc type not handled: %d", id);
      break;
#endif
  }

  if (!(id & TFLAG(TF_NOASS)))
      ret.u.d = retd;

  return ret;
}

/**/
static mnumber
math_string(UNUSED(char *name), char *arg, int id)
{
    mnumber ret = zero_mnumber;
    char *send;
    /*
     * Post-process the string argument, which is just passed verbatim.
     * Not clear if any other functions that use math_string() will
     * want this, but assume so for now.
     */
    while (iblank(*arg))
	arg++;
    send = arg + strlen(arg);
    while (send > arg && iblank(send[-1]))
	send--;
    *send = '\0';

    switch (id)
    {
#ifdef HAVE_ERAND48
    case MS_RAND48:
	{
	    static unsigned short seedbuf[3];
	    static int seedbuf_init;
	    unsigned short tmp_seedbuf[3], *seedbufptr;
	    int do_init = 1;

	    if (*arg) {
		/* Seed is contained in parameter named by arg */
		char *seedstr;
		seedbufptr = tmp_seedbuf;
		if ((seedstr = getsparam(arg)) && strlen(seedstr) >= 12) {
		    int i, j;
		    do_init = 0;
		    /*
		     * Decode three sets of four hex digits corresponding
		     * to each unsigned short.
		     */
		    for (i = 0; i < 3 && !do_init; i++) {
			unsigned short *seedptr = seedbufptr + i;
			*seedptr = 0;
			for (j = 0; j < 4; j++) {
			    if (idigit(*seedstr))
				*seedptr += *seedstr - '0';
			    else if (tolower(*seedstr) >= 'a' &&
				     tolower(*seedstr) <= 'f')
				*seedptr += tolower(*seedstr) - 'a' + 10;
			    else {
				do_init = 1;
				break;
			    }
			    seedstr++;
			    if (j < 3)
				*seedptr *= 16;
			}
		    }
		}
		else if (errflag)
		    break;
	    }
	    else
	    {
		/* Use default seed: must be initialised. */
		seedbufptr = seedbuf;
		if (!seedbuf_init)
		    seedbuf_init = 1;
		else
		    do_init = 1;
	    }
	    if (do_init) {
		seedbufptr[0] = (unsigned short)rand();
		seedbufptr[1] = (unsigned short)rand();
		seedbufptr[2] = (unsigned short)rand();
		/*
		 * Some implementations of rand48() need initialization.
		 * This is likely to be harmless elsewhere, since
		 * according to the documentation erand48() normally
		 * doesn't look at the seed set in this way.
		 */
		(void)seed48(seedbufptr);
	    }
	    ret.type = MN_FLOAT;
	    ret.u.d = erand48(seedbufptr);

	    if (*arg)
	    {
		char outbuf[13];
		sprintf(outbuf, "%04x%04x%04x", (int)seedbufptr[0],
			(int)seedbufptr[1], (int)seedbufptr[2]);
		setsparam(arg, ztrdup(outbuf));
	    }
	}
	break;
#endif
    }

    return ret;
}


static struct features module_features = {
    NULL, 0,
    NULL, 0,
    mftab, sizeof(mftab)/sizeof(*mftab),
    NULL, 0,
    0
};

/**/
int
setup_(UNUSED(Module m))
{
    return 0;
}

/**/
int
features_(Module m, char ***features)
{
    *features = featuresarray(m, &module_features);
    return 0;
}

/**/
int
enables_(Module m, int **enables)
{
    return handlefeatures(m, &module_features, enables);
}

/**/
int
boot_(UNUSED(Module m))
{
    return 0;
}

/**/
int
cleanup_(Module m)
{
    return setfeatureenables(m, &module_features, NULL);
}

/**/
int
finish_(UNUSED(Module m))
{
    return 0;
}
