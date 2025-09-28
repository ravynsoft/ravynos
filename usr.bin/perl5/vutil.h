/* This file is part of the "version" CPAN distribution.  Please avoid
   editing it in the perl core. */

/* The MUTABLE_*() macros cast pointers to the types shown, in such a way
 * (compiler permitting) that casting away const-ness will give a warning;
 * e.g.:
 *
 * const SV *sv = ...;
 * AV *av1 = (AV*)sv;        <== BAD:  the const has been silently cast away
 * AV *av2 = MUTABLE_AV(sv); <== GOOD: it may warn
 */

#if PERL_VERSION_LT(5,15,4)
#  define ISA_VERSION_OBJ(v) (sv_isobject(v) && sv_derived_from(v,"version"))
#else
#  define ISA_VERSION_OBJ(v) (sv_isobject(v) && sv_derived_from_pvn(v,"version",7,0))
#endif

#if PERL_VERSION_GE(5,9,0) && !defined(PERL_CORE)

#  define VUTIL_REPLACE_CORE 1

static const char * Perl_scan_version2(pTHX_ const char *s, SV *rv, bool qv);
static SV * Perl_new_version2(pTHX_ SV *ver);
static SV * Perl_upg_version2(pTHX_ SV *sv, bool qv);
static SV * Perl_vstringify2(pTHX_ SV *vs);
static SV * Perl_vverify2(pTHX_ SV *vs);
static SV * Perl_vnumify2(pTHX_ SV *vs);
static SV * Perl_vnormal2(pTHX_ SV *vs);
static SV * Perl_vstringify2(pTHX_ SV *vs);
static int Perl_vcmp2(pTHX_ SV *lsv, SV *rsv);
static const char * Perl_prescan_version2(pTHX_ const char *s, bool strict, const char** errstr, bool *sqv, int *ssaw_decimal, int *swidth, bool *salpha);

#  define SCAN_VERSION(a,b,c)	Perl_scan_version2(aTHX_ a,b,c)
#  define NEW_VERSION(a)	Perl_new_version2(aTHX_ a)
#  define UPG_VERSION(a,b)	Perl_upg_version2(aTHX_ a, b)
#  define VSTRINGIFY(a)		Perl_vstringify2(aTHX_ a)
#  define VVERIFY(a)		Perl_vverify2(aTHX_ a)
#  define VNUMIFY(a)		Perl_vnumify2(aTHX_ a)
#  define VNORMAL(a)		Perl_vnormal2(aTHX_ a)
#  define VCMP(a,b)		Perl_vcmp2(aTHX_ a,b)
#  define PRESCAN_VERSION(a,b,c,d,e,f,g)	Perl_prescan_version2(aTHX_ a,b,c,d,e,f,g)
#  undef is_LAX_VERSION
#  define is_LAX_VERSION(a,b) \
	(a != Perl_prescan_version2(aTHX_ a, FALSE, b, NULL, NULL, NULL, NULL))
#  undef is_STRICT_VERSION
#  define is_STRICT_VERSION(a,b) \
	(a != Perl_prescan_version2(aTHX_ a, TRUE, b, NULL, NULL, NULL, NULL))

#else

const char * Perl_scan_version(pTHX_ const char *s, SV *rv, bool qv);
SV * Perl_new_version(pTHX_ SV *ver);
SV * Perl_upg_version(pTHX_ SV *sv, bool qv);
SV * Perl_vverify(pTHX_ SV *vs);
SV * Perl_vnumify(pTHX_ SV *vs);
SV * Perl_vnormal(pTHX_ SV *vs);
SV * Perl_vstringify(pTHX_ SV *vs);
int Perl_vcmp(pTHX_ SV *lsv, SV *rsv);
const char * Perl_prescan_version(pTHX_ const char *s, bool strict, const char** errstr, bool *sqv, int *ssaw_decimal, int *swidth, bool *salpha);

#  define SCAN_VERSION(a,b,c)	Perl_scan_version(aTHX_ a,b,c)
#  define NEW_VERSION(a)	Perl_new_version(aTHX_ a)
#  define UPG_VERSION(a,b)	Perl_upg_version(aTHX_ a, b)
#  define VSTRINGIFY(a)		Perl_vstringify(aTHX_ a)
#  define VVERIFY(a)		Perl_vverify(aTHX_ a)
#  define VNUMIFY(a)		Perl_vnumify(aTHX_ a)
#  define VNORMAL(a)		Perl_vnormal(aTHX_ a)
#  define VCMP(a,b)		Perl_vcmp(aTHX_ a,b)

#  define PRESCAN_VERSION(a,b,c,d,e,f,g)	Perl_prescan_version(aTHX_ a,b,c,d,e,f,g)
#  ifndef is_LAX_VERSION
#    define is_LAX_VERSION(a,b) \
	(a != Perl_prescan_version(aTHX_ a, FALSE, b, NULL, NULL, NULL, NULL))
#  endif
#  ifndef is_STRICT_VERSION
#    define is_STRICT_VERSION(a,b) \
	(a != Perl_prescan_version(aTHX_ a, TRUE, b, NULL, NULL, NULL, NULL))
#  endif

#endif

#if PERL_VERSION_LT(5,11,4)
#  define BADVERSION(a,b,c) \
	if (b) { \
	    *b = c; \
	} \
	return a;

#  define PERL_ARGS_ASSERT_PRESCAN_VERSION	\
	assert(s); assert(sqv); assert(ssaw_decimal);\
	assert(swidth); assert(salpha);

#  define PERL_ARGS_ASSERT_SCAN_VERSION	\
	assert(s); assert(rv)
#  define PERL_ARGS_ASSERT_NEW_VERSION	\
	assert(ver)
#  define PERL_ARGS_ASSERT_UPG_VERSION	\
	assert(ver)
#  define PERL_ARGS_ASSERT_VVERIFY	\
	assert(vs)
#  define PERL_ARGS_ASSERT_VNUMIFY	\
	assert(vs)
#  define PERL_ARGS_ASSERT_VNORMAL	\
	assert(vs)
#  define PERL_ARGS_ASSERT_VSTRINGIFY	\
	assert(vs)
#  define PERL_ARGS_ASSERT_VCMP	\
	assert(lhv); assert(rhv)
#  define PERL_ARGS_ASSERT_CK_WARNER      \
	assert(pat)
#endif

/* ex: set ro: */
