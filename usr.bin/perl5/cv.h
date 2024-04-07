/*    cv.h
 *
 *    Copyright (C) 1991, 1992, 1993, 1994, 1995, 1996, 1997, 1999, 2000, 2001,
 *    2002, 2003, 2004, 2005, 2006, 2007, 2008 by Larry Wall and others
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

/* This structure must match the beginning of XPVFM in sv.h  */

struct xpvcv {
    _XPV_HEAD;
    _XPVCV_COMMON;
};

/*
=for apidoc Ayh||CV

=for apidoc ADmnU||Nullcv
Null CV pointer.

(deprecated - use C<(CV *)NULL> instead)

=for apidoc Am|HV*|CvSTASH|CV* cv
Returns the stash of the CV.  A stash is the symbol table hash, containing
the package-scoped variables in the package where the subroutine was defined.
For more information, see L<perlguts>.

This also has a special use with XS AUTOLOAD subs.
See L<perlguts/Autoloading with XSUBs>.

=cut
*/

#ifndef PERL_CORE
#  define Nullcv Null(CV*)
#endif

#define CvSTASH(sv)	(MUTABLE_HV(((XPVCV*)MUTABLE_PTR(SvANY(sv)))->xcv_stash))
#define CvSTASH_set(cv,st) Perl_cvstash_set(aTHX_ cv, st)
#define CvSTART(sv)	((XPVCV*)MUTABLE_PTR(SvANY(sv)))->xcv_start_u.xcv_start
#define CvROOT(sv)	((XPVCV*)MUTABLE_PTR(SvANY(sv)))->xcv_root_u.xcv_root
#define CvXSUB(sv)	((XPVCV*)MUTABLE_PTR(SvANY(sv)))->xcv_root_u.xcv_xsub
#define CvXSUBANY(sv)	((XPVCV*)MUTABLE_PTR(SvANY(sv)))->xcv_start_u.xcv_xsubany
#define CvGV(sv)	Perl_CvGV(aTHX_ (CV *)(sv))
#define CvGV_set(cv,gv)	Perl_cvgv_set(aTHX_ cv, gv)
#define CvHASGV(cv)	cBOOL(SvANY(cv)->xcv_gv_u.xcv_gv)
#define CvFILE(sv)	((XPVCV*)MUTABLE_PTR(SvANY(sv)))->xcv_file
#ifdef USE_ITHREADS
#  define CvFILE_set_from_cop(sv, cop)	\
    (CvFILE(sv) = savepv(CopFILE(cop)), CvDYNFILE_on(sv))
#else
#  define CvFILE_set_from_cop(sv, cop)	\
    (CvFILE(sv) = CopFILE(cop), CvDYNFILE_off(sv))
#endif
#define CvFILEGV(sv)	(gv_fetchfile(CvFILE(sv)))
#define CvDEPTH(sv)	(*Perl_CvDEPTH((const CV *)sv))
/* For use when you only have a XPVCV*, not a real CV*.
   Must be assert protected as in Perl_CvDEPTH before use. */
#define CvDEPTHunsafe(sv) ((XPVCV*)MUTABLE_PTR(SvANY(sv)))->xcv_depth

/* these CvPADLIST/CvRESERVED asserts can be reverted one day, once stabilized */
#define CvPADLIST(sv)	  (*(assert_(!CvISXSUB((CV*)(sv))) \
        &(((XPVCV*)MUTABLE_PTR(SvANY(sv)))->xcv_padlist_u.xcv_padlist)))
/* CvPADLIST_set is not public API, it can be removed one day, once stabilized */
#ifdef DEBUGGING
#  define CvPADLIST_set(sv, padlist) Perl_set_padlist((CV*)sv, padlist)
#else
#  define CvPADLIST_set(sv, padlist) (CvPADLIST(sv) = (padlist))
#endif
#define CvHSCXT(sv)	  *(assert_(CvISXSUB((CV*)(sv))) \
        &(((XPVCV*)MUTABLE_PTR(SvANY(sv)))->xcv_padlist_u.xcv_hscxt))
#ifdef DEBUGGING
#  if PTRSIZE == 8
#    define PoisonPADLIST(sv) \
        (((XPVCV*)MUTABLE_PTR(SvANY(sv)))->xcv_padlist_u.xcv_padlist = (PADLIST *)UINT64_C(0xEFEFEFEFEFEFEFEF))
#  elif PTRSIZE == 4
#    define PoisonPADLIST(sv) \
        (((XPVCV*)MUTABLE_PTR(SvANY(sv)))->xcv_padlist_u.xcv_padlist = (PADLIST *)0xEFEFEFEF)
#  else
#    error unknown pointer size
#  endif
#else
#  define PoisonPADLIST(sv) NOOP
#endif

#define CvOUTSIDE(sv)	  ((XPVCV*)MUTABLE_PTR(SvANY(sv)))->xcv_outside
#define CvOUTSIDE_SEQ(sv) ((XPVCV*)MUTABLE_PTR(SvANY(sv)))->xcv_outside_seq
#define CvFLAGS(sv)	  ((XPVCV*)MUTABLE_PTR(SvANY(sv)))->xcv_flags

/* These two are sometimes called on non-CVs */
#define CvPROTO(sv)                               \
        (                                          \
         SvPOK(sv)                                  \
          ? SvTYPE(sv) == SVt_PVCV && CvAUTOLOAD(sv) \
             ? SvEND(sv)+1 : SvPVX_const(sv)          \
          : NULL                                       \
        )
#define CvPROTOLEN(sv)	                          \
        (                                          \
         SvPOK(sv)                                  \
          ? SvTYPE(sv) == SVt_PVCV && CvAUTOLOAD(sv) \
             ? SvLEN(sv)-SvCUR(sv)-2                  \
             : SvCUR(sv)                               \
          : 0                                           \
        )

/* CV has the `:method` attribute. This used to be called CVf_METHOD but is
 * renamed to avoid collision with CVf_IsMETHOD */
#define CVf_NOWARN_AMBIGUOUS	0x0001

#define CVf_LVALUE	0x0002  /* CV return value can be used as lvalue */
#define CVf_CONST	0x0004  /* inlinable sub */
#define CVf_ISXSUB	0x0008	/* CV is an XSUB, not pure perl.  */

#define CVf_WEAKOUTSIDE	0x0010  /* CvOUTSIDE isn't ref counted */
#define CVf_CLONE	0x0020	/* anon CV uses external lexicals */
#define CVf_CLONED	0x0040	/* a clone of one of those */
#define CVf_ANON	0x0080	/* CV is not pointed to by a GV */
#define CVf_UNIQUE	0x0100	/* sub is only called once (eg PL_main_cv,
                                   require, eval). */
#define CVf_NODEBUG	0x0200	/* no DB::sub indirection for this CV
                                   (esp. useful for special XSUBs) */
#define CVf_CVGV_RC	0x0400	/* CvGV is reference counted */
#if defined(PERL_CORE) || defined(PERL_EXT)
# define CVf_SLABBED	0x0800	/* Holds refcount on op slab  */
#endif
#define CVf_DYNFILE	0x1000	/* The filename is malloced  */
#define CVf_AUTOLOAD	0x2000	/* SvPVX contains AUTOLOADed sub name  */
#define CVf_HASEVAL	0x4000	/* contains string eval  */
#define CVf_NAMED	0x8000  /* Has a name HEK */
#define CVf_LEXICAL	0x10000 /* Omit package from name */
#define CVf_ANONCONST	0x20000 /* :const - create anonconst op */
#define CVf_SIGNATURE   0x40000 /* CV uses a signature */
#define CVf_REFCOUNTED_ANYSV 0x80000 /* CvXSUBANY().any_sv is refcounted */
#define CVf_IsMETHOD    0x100000 /* CV is a (real) method of a real class. Not
                                   to be confused with what used to be called
                                   CVf_METHOD; now CVf_NOWARN_AMBIGUOUS */

/* This symbol for optimised communication between toke.c and op.c: */
#define CVf_BUILTIN_ATTRS	(CVf_NOWARN_AMBIGUOUS|CVf_LVALUE|CVf_ANONCONST)

#define CvCLONE(cv)		(CvFLAGS(cv) & CVf_CLONE)
#define CvCLONE_on(cv)		(CvFLAGS(cv) |= CVf_CLONE)
#define CvCLONE_off(cv)		(CvFLAGS(cv) &= ~CVf_CLONE)

#define CvCLONED(cv)		(CvFLAGS(cv) & CVf_CLONED)
#define CvCLONED_on(cv)		(CvFLAGS(cv) |= CVf_CLONED)
#define CvCLONED_off(cv)	(CvFLAGS(cv) &= ~CVf_CLONED)

#define CvANON(cv)		(CvFLAGS(cv) & CVf_ANON)
#define CvANON_on(cv)		(CvFLAGS(cv) |= CVf_ANON)
#define CvANON_off(cv)		(CvFLAGS(cv) &= ~CVf_ANON)

/* CvEVAL or CvSPECIAL */
#define CvUNIQUE(cv)		(CvFLAGS(cv) & CVf_UNIQUE)
#define CvUNIQUE_on(cv)		(CvFLAGS(cv) |= CVf_UNIQUE)
#define CvUNIQUE_off(cv)	(CvFLAGS(cv) &= ~CVf_UNIQUE)

#define CvNODEBUG(cv)		(CvFLAGS(cv) & CVf_NODEBUG)
#define CvNODEBUG_on(cv)	(CvFLAGS(cv) |= CVf_NODEBUG)
#define CvNODEBUG_off(cv)	(CvFLAGS(cv) &= ~CVf_NODEBUG)

#define CvNOWARN_AMBIGUOUS(cv)		(CvFLAGS(cv) & CVf_NOWARN_AMBIGUOUS)
#define CvNOWARN_AMBIGUOUS_on(cv)	(CvFLAGS(cv) |= CVf_NOWARN_AMBIGUOUS)
#define CvNOWARN_AMBIGUOUS_off(cv)	(CvFLAGS(cv) &= ~CVf_NOWARN_AMBIGUOUS)

#define CvLVALUE(cv)		(CvFLAGS(cv) & CVf_LVALUE)
#define CvLVALUE_on(cv)		(CvFLAGS(cv) |= CVf_LVALUE)
#define CvLVALUE_off(cv)	(CvFLAGS(cv) &= ~CVf_LVALUE)

/* eval or PL_main_cv */
#define CvEVAL(cv)		(CvUNIQUE(cv) && !SvFAKE(cv))
#define CvEVAL_on(cv)		(CvUNIQUE_on(cv),SvFAKE_off(cv))
#define CvEVAL_off(cv)		CvUNIQUE_off(cv)

/* BEGIN|CHECK|INIT|UNITCHECK|END */
#define CvSPECIAL(cv)		(CvUNIQUE(cv) && SvFAKE(cv))
#define CvSPECIAL_on(cv)	(CvUNIQUE_on(cv),SvFAKE_on(cv))
#define CvSPECIAL_off(cv)	(CvUNIQUE_off(cv),SvFAKE_off(cv))

#define CvCONST(cv)		(CvFLAGS(cv) & CVf_CONST)
#define CvCONST_on(cv)		(CvFLAGS(cv) |= CVf_CONST)
#define CvCONST_off(cv)		(CvFLAGS(cv) &= ~CVf_CONST)

#define CvWEAKOUTSIDE(cv)	(CvFLAGS(cv) & CVf_WEAKOUTSIDE)
#define CvWEAKOUTSIDE_on(cv)	(CvFLAGS(cv) |= CVf_WEAKOUTSIDE)
#define CvWEAKOUTSIDE_off(cv)	(CvFLAGS(cv) &= ~CVf_WEAKOUTSIDE)

#define CvISXSUB(cv)		(CvFLAGS(cv) & CVf_ISXSUB)
#define CvISXSUB_on(cv)		(CvFLAGS(cv) |= CVf_ISXSUB)
#define CvISXSUB_off(cv)	(CvFLAGS(cv) &= ~CVf_ISXSUB)

#define CvCVGV_RC(cv)		(CvFLAGS(cv) & CVf_CVGV_RC)
#define CvCVGV_RC_on(cv)	(CvFLAGS(cv) |= CVf_CVGV_RC)
#define CvCVGV_RC_off(cv)	(CvFLAGS(cv) &= ~CVf_CVGV_RC)

#ifdef PERL_CORE
# define CvSLABBED(cv)		(CvFLAGS(cv) & CVf_SLABBED)
# define CvSLABBED_on(cv)	(CvFLAGS(cv) |= CVf_SLABBED)
# define CvSLABBED_off(cv)	(CvFLAGS(cv) &= ~CVf_SLABBED)
#endif

#define CvDYNFILE(cv)		(CvFLAGS(cv) & CVf_DYNFILE)
#define CvDYNFILE_on(cv)	(CvFLAGS(cv) |= CVf_DYNFILE)
#define CvDYNFILE_off(cv)	(CvFLAGS(cv) &= ~CVf_DYNFILE)

#define CvAUTOLOAD(cv)		(CvFLAGS(cv) & CVf_AUTOLOAD)
#define CvAUTOLOAD_on(cv)	(CvFLAGS(cv) |= CVf_AUTOLOAD)
#define CvAUTOLOAD_off(cv)	(CvFLAGS(cv) &= ~CVf_AUTOLOAD)

#define CvHASEVAL(cv)		(CvFLAGS(cv) & CVf_HASEVAL)
#define CvHASEVAL_on(cv)	(CvFLAGS(cv) |= CVf_HASEVAL)
#define CvHASEVAL_off(cv)	(CvFLAGS(cv) &= ~CVf_HASEVAL)

#define CvNAMED(cv)		(CvFLAGS(cv) & CVf_NAMED)
#define CvNAMED_on(cv)		(CvFLAGS(cv) |= CVf_NAMED)
#define CvNAMED_off(cv)		(CvFLAGS(cv) &= ~CVf_NAMED)

#define CvLEXICAL(cv)		(CvFLAGS(cv) & CVf_LEXICAL)
#define CvLEXICAL_on(cv)	(CvFLAGS(cv) |= CVf_LEXICAL)
#define CvLEXICAL_off(cv)	(CvFLAGS(cv) &= ~CVf_LEXICAL)

#define CvANONCONST(cv)		(CvFLAGS(cv) & CVf_ANONCONST)
#define CvANONCONST_on(cv)	(CvFLAGS(cv) |= CVf_ANONCONST)
#define CvANONCONST_off(cv)	(CvFLAGS(cv) &= ~CVf_ANONCONST)

#define CvSIGNATURE(cv)		(CvFLAGS(cv) & CVf_SIGNATURE)
#define CvSIGNATURE_on(cv)	(CvFLAGS(cv) |= CVf_SIGNATURE)
#define CvSIGNATURE_off(cv)	(CvFLAGS(cv) &= ~CVf_SIGNATURE)

/*

=for apidoc m|bool|CvREFCOUNTED_ANYSV|CV *cv

If true, indicates that the C<CvXSUBANY(cv).any_sv> member contains an SV
pointer whose reference count should be decremented when the CV itself is
freed.  In addition, C<cv_clone()> will increment the reference count, and
C<sv_dup()> will duplicate the entire pointed-to SV if this flag is set.

Any CV that wraps an XSUB has an C<ANY> union that the XSUB function is free
to use for its own purposes.  It may be the case that the code wishes to store
an SV in the C<any_sv> member of this union.  By setting this flag, this SV
reference will be properly reclaimed or duplicated when the CV itself is.

=for apidoc m|void|CvREFCOUNTED_ANYSV_on|CV *cv

Helper macro to turn on the C<CvREFCOUNTED_ANYSV> flag.

=for apidoc m|void|CvREFCOUNTED_ANYSV_off|CV *cv

Helper macro to turn off the C<CvREFCOUNTED_ANYSV> flag.

=cut
*/

#define CvREFCOUNTED_ANYSV(cv)          (CvFLAGS(cv) & CVf_REFCOUNTED_ANYSV)
#define CvREFCOUNTED_ANYSV_on(cv)       (CvFLAGS(cv) |= CVf_REFCOUNTED_ANYSV)
#define CvREFCOUNTED_ANYSV_off(cv)      (CvFLAGS(cv) &= ~CVf_REFCOUNTED_ANYSV)

#define CvIsMETHOD(cv)		(CvFLAGS(cv) & CVf_IsMETHOD)
#define CvIsMETHOD_on(cv)	(CvFLAGS(cv) |= CVf_IsMETHOD)
#define CvIsMETHOD_off(cv)	(CvFLAGS(cv) &= ~CVf_IsMETHOD)

/* Back-compat */
#ifndef PERL_CORE
#  define CVf_METHOD            CVf_NOWARN_AMBIGUOUS
#  define CvMETHOD(cv)          CvNOWARN_AMBIGUOUS(cv)
#  define CvMETHOD_on(cv)       CvNOWARN_AMBIGUOUS_on(cv)
#  define CvMETHOD_off(cv)      CvNOWARN_AMBIGUOUS_off(cv)
#endif

/* Flags for newXS_flags  */
#define XS_DYNAMIC_FILENAME	0x01	/* The filename isn't static  */

PERL_STATIC_INLINE HEK *
CvNAME_HEK(CV *sv)
{
    return CvNAMED(sv)
        ? ((XPVCV*)MUTABLE_PTR(SvANY(sv)))->xcv_gv_u.xcv_hek
        : 0;
}

/* helper for the common pattern:
   CvNAMED(sv) ? CvNAME_HEK((CV *)sv) : GvNAME_HEK(CvGV(sv))
*/
#define CvGvNAME_HEK(sv) ( \
        CvNAMED((CV*)sv) ? \
            ((XPVCV*)MUTABLE_PTR(SvANY((SV*)sv)))->xcv_gv_u.xcv_hek\
            : GvNAME_HEK(CvGV( (SV*) sv)) \
        )

/* This lowers the reference count of the previous value, but does *not*
   increment the reference count of the new value. */
#define CvNAME_HEK_set(cv, hek) ( \
        CvNAME_HEK((CV *)(cv))						 \
            ? unshare_hek(SvANY((CV *)(cv))->xcv_gv_u.xcv_hek)	  \
            : (void)0,						   \
        ((XPVCV*)MUTABLE_PTR(SvANY(cv)))->xcv_gv_u.xcv_hek = (hek), \
        CvNAMED_on(cv)						     \
    )

/*

=for apidoc m|bool|CvWEAKOUTSIDE|CV *cv

Each CV has a pointer, C<CvOUTSIDE()>, to its lexically enclosing
CV (if any).  Because pointers to anonymous sub prototypes are
stored in C<&> pad slots, it is a possible to get a circular reference,
with the parent pointing to the child and vice-versa.  To avoid the
ensuing memory leak, we do not increment the reference count of the CV
pointed to by C<CvOUTSIDE> in the I<one specific instance> that the parent
has a C<&> pad slot pointing back to us.  In this case, we set the
C<CvWEAKOUTSIDE> flag in the child.  This allows us to determine under what
circumstances we should decrement the refcount of the parent when freeing
the child.

There is a further complication with non-closure anonymous subs (i.e. those
that do not refer to any lexicals outside that sub).  In this case, the
anonymous prototype is shared rather than being cloned.  This has the
consequence that the parent may be freed while there are still active
children, I<e.g.>,

    BEGIN { $a = sub { eval '$x' } }

In this case, the BEGIN is freed immediately after execution since there
are no active references to it: the anon sub prototype has
C<CvWEAKOUTSIDE> set since it's not a closure, and $a points to the same
CV, so it doesn't contribute to BEGIN's refcount either.  When $a is
executed, the C<eval '$x'> causes the chain of C<CvOUTSIDE>s to be followed,
and the freed BEGIN is accessed.

To avoid this, whenever a CV and its associated pad is freed, any
C<&> entries in the pad are explicitly removed from the pad, and if the
refcount of the pointed-to anon sub is still positive, then that
child's C<CvOUTSIDE> is set to point to its grandparent.  This will only
occur in the single specific case of a non-closure anon prototype
having one or more active references (such as C<$a> above).

One other thing to consider is that a CV may be merely undefined
rather than freed, eg C<undef &foo>.  In this case, its refcount may
not have reached zero, but we still delete its pad and its C<CvROOT> etc.
Since various children may still have their C<CvOUTSIDE> pointing at this
undefined CV, we keep its own C<CvOUTSIDE> for the time being, so that
the chain of lexical scopes is unbroken.  For example, the following
should print 123:

    my $x = 123;
    sub tmp { sub { eval '$x' } }
    my $a = tmp();
    undef &tmp;
    print  $a->();

=cut
*/

typedef OP *(*Perl_call_checker)(pTHX_ OP *, GV *, SV *);

#define CALL_CHECKER_REQUIRE_GV	MGf_REQUIRE_GV

#define CV_NAME_NOTQUAL		1

#ifdef PERL_CORE
# define CV_UNDEF_KEEP_NAME	1
#endif

/*
 * ex: set ts=8 sts=4 sw=4 et:
 */
