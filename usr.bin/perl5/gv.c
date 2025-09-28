/*    gv.c
 *
 *    Copyright (C) 1991, 1992, 1993, 1994, 1995, 1996, 1997, 1998, 1999, 2000,
 *    2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009 by Larry Wall and others
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

/*
 *   'Mercy!' cried Gandalf.  'If the giving of information is to be the cure
 * of your inquisitiveness, I shall spend all the rest of my days in answering
 * you.  What more do you want to know?'
 *   'The names of all the stars, and of all living things, and the whole
 * history of Middle-earth and Over-heaven and of the Sundering Seas,'
 * laughed Pippin.
 *
 *     [p.599 of _The Lord of the Rings_, III/xi: "The Palantír"]
 */

/*
=head1 GV Handling and Stashes
A GV is a structure which corresponds to to a Perl typeglob, ie *foo.
It is a structure that holds a pointer to a scalar, an array, a hash etc,
corresponding to $foo, @foo, %foo.

GVs are usually found as values in stashes (symbol table hashes) where
Perl stores its global variables.

A B<stash> is a hash that contains all variables that are defined
within a package.  See L<perlguts/Stashes and Globs>

=for apidoc Ayh||GV

=cut
*/

#include "EXTERN.h"
#define PERL_IN_GV_C
#include "perl.h"
#include "overload.inc"
#include "keywords.h"
#include "feature.h"

static const char S_autoload[] = "AUTOLOAD";
#define S_autolen (sizeof("AUTOLOAD")-1)

/*
=for apidoc gv_add_by_type

Make sure there is a slot of type C<type> in the GV C<gv>.

=cut
*/

GV *
Perl_gv_add_by_type(pTHX_ GV *gv, svtype type)
{
    SV **where;

    if (
        !gv
     || (
            SvTYPE((const SV *)gv) != SVt_PVGV
         && SvTYPE((const SV *)gv) != SVt_PVLV
        )
    ) {
        const char *what;
        if (type == SVt_PVIO) {
            /*
             * if it walks like a dirhandle, then let's assume that
             * this is a dirhandle.
             */
            what = OP_IS_DIRHOP(PL_op->op_type) ?
                "dirhandle" : "filehandle";
        } else if (type == SVt_PVHV) {
            what = "hash";
        } else {
            what = type == SVt_PVAV ? "array" : "scalar";
        }
        Perl_croak(aTHX_ "Bad symbol for %s", what);
    }

    if (type == SVt_PVHV) {
        where = (SV **)&GvHV(gv);
    } else if (type == SVt_PVAV) {
        where = (SV **)&GvAV(gv);
    } else if (type == SVt_PVIO) {
        where = (SV **)&GvIOp(gv);
    } else {
        where = &GvSV(gv);
    }

    if (!*where)
    {
        *where = newSV_type(type);
        if (   type == SVt_PVAV
            && memEQs(GvNAME(gv), GvNAMELEN(gv), "ISA"))
        {
            sv_magic(*where, (SV *)gv, PERL_MAGIC_isa, NULL, 0);
        }
    }
    return gv;
}

/*
=for apidoc gv_fetchfile
=for apidoc_item gv_fetchfile_flags

These return the debugger glob for the file (compiled by Perl) whose name is
given by the C<name> parameter.

There are currently exactly two differences between these functions.

The C<name> parameter to C<gv_fetchfile> is a C string, meaning it is
C<NUL>-terminated; whereas the C<name> parameter to C<gv_fetchfile_flags> is a
Perl string, whose length (in bytes) is passed in via the C<namelen> parameter
This means the name may contain embedded C<NUL> characters.
C<namelen> doesn't exist in plain C<gv_fetchfile>).

The other difference is that C<gv_fetchfile_flags> has an extra C<flags>
parameter, which is currently completely ignored, but allows for possible
future extensions.

=cut
*/
GV *
Perl_gv_fetchfile(pTHX_ const char *name)
{
    PERL_ARGS_ASSERT_GV_FETCHFILE;
    return gv_fetchfile_flags(name, strlen(name), 0);
}

GV *
Perl_gv_fetchfile_flags(pTHX_ const char *const name, const STRLEN namelen,
                        const U32 flags)
{
    char smallbuf[128];
    char *tmpbuf;
    const STRLEN tmplen = namelen + 2;
    GV *gv;

    PERL_ARGS_ASSERT_GV_FETCHFILE_FLAGS;
    PERL_UNUSED_ARG(flags);

    if (!PL_defstash)
        return NULL;

    if (tmplen <= sizeof smallbuf)
        tmpbuf = smallbuf;
    else
        Newx(tmpbuf, tmplen, char);
    /* This is where the debugger's %{"::_<$filename"} hash is created */
    tmpbuf[0] = '_';
    tmpbuf[1] = '<';
    memcpy(tmpbuf + 2, name, namelen);
    GV **gvp = (GV**)hv_fetch(PL_defstash, tmpbuf, tmplen, (flags & GVF_NOADD) ? FALSE : TRUE);
    if (gvp) {
        gv = *gvp;
        if (!isGV(gv)) {
            gv_init(gv, PL_defstash, tmpbuf, tmplen, FALSE);
#ifdef PERL_DONT_CREATE_GVSV
            GvSV(gv) = newSVpvn(name, namelen);
#else
            sv_setpvn(GvSV(gv), name, namelen);
#endif
        }
        if (PERLDB_LINE_OR_SAVESRC && !GvAV(gv))
            hv_magic(GvHVn(gv), GvAVn(gv), PERL_MAGIC_dbfile);
    }
    else {
        gv = NULL;
    }
    if (tmpbuf != smallbuf)
        Safefree(tmpbuf);
    return gv;
}

/*
=for apidoc gv_const_sv

If C<gv> is a typeglob whose subroutine entry is a constant sub eligible for
inlining, or C<gv> is a placeholder reference that would be promoted to such
a typeglob, then returns the value returned by the sub.  Otherwise, returns
C<NULL>.

=cut
*/

SV *
Perl_gv_const_sv(pTHX_ GV *gv)
{
    PERL_ARGS_ASSERT_GV_CONST_SV;
    PERL_UNUSED_CONTEXT;

    if (SvTYPE(gv) == SVt_PVGV)
        return cv_const_sv(GvCVu(gv));
    return SvROK(gv) && SvTYPE(SvRV(gv)) != SVt_PVAV && SvTYPE(SvRV(gv)) != SVt_PVCV ? SvRV(gv) : NULL;
}

GP *
Perl_newGP(pTHX_ GV *const gv)
{
    GP *gp;
    U32 hash;
    const char *file;
    STRLEN len;

    PERL_ARGS_ASSERT_NEWGP;
    Newxz(gp, 1, GP);
    gp->gp_egv = gv; /* allow compiler to reuse gv after this */
#ifndef PERL_DONT_CREATE_GVSV
    gp->gp_sv = newSV_type(SVt_NULL);
#endif

    /* PL_curcop may be null here.  E.g.,
        INIT { bless {} and exit }
       frees INIT before looking up DESTROY (and creating *DESTROY)
    */
    if (PL_curcop) {
        char *tmp= CopFILE(PL_curcop);
        gp->gp_line = CopLINE(PL_curcop); /* 0 otherwise Newxz */

        if (tmp) {
            file = tmp;
            len = CopFILE_LEN(PL_curcop);
        }
        else goto no_file;
    }
    else {
        no_file:
        file = "";
        len = 0;
    }

    PERL_HASH(hash, file, len);
    gp->gp_file_hek = share_hek(file, len, hash);
    gp->gp_refcnt = 1;

    return gp;
}

/* Assign CvGV(cv) = gv, handling weak references.
 * See also S_anonymise_cv_maybe */

void
Perl_cvgv_set(pTHX_ CV* cv, GV* gv)
{
    GV * const oldgv = CvNAMED(cv) ? NULL : SvANY(cv)->xcv_gv_u.xcv_gv;
    HEK *hek;
    PERL_ARGS_ASSERT_CVGV_SET;

    if (oldgv == gv)
        return;

    if (oldgv) {
        if (CvCVGV_RC(cv)) {
            SvREFCNT_dec_NN(oldgv);
            CvCVGV_RC_off(cv);
        }
        else {
            sv_del_backref(MUTABLE_SV(oldgv), MUTABLE_SV(cv));
        }
    }
    else if ((hek = CvNAME_HEK(cv))) {
        unshare_hek(hek);
        CvLEXICAL_off(cv);
    }

    CvNAMED_off(cv);
    SvANY(cv)->xcv_gv_u.xcv_gv = gv;
    assert(!CvCVGV_RC(cv));

    if (!gv)
        return;

    if (isGV_with_GP(gv) && GvGP(gv) && (GvCV(gv) == cv || GvFORM(gv) == cv))
        Perl_sv_add_backref(aTHX_ MUTABLE_SV(gv), MUTABLE_SV(cv));
    else {
        CvCVGV_RC_on(cv);
        SvREFCNT_inc_simple_void_NN(gv);
    }
}

/* Convert CvSTASH + CvNAME_HEK into a GV.  Conceptually, all subs have a
   GV, but for efficiency that GV may not in fact exist.  This function,
   called by CvGV, reifies it. */

GV *
Perl_cvgv_from_hek(pTHX_ CV *cv)
{
    GV *gv;
    SV **svp;
    PERL_ARGS_ASSERT_CVGV_FROM_HEK;
    assert(SvTYPE(cv) == SVt_PVCV);
    if (!CvSTASH(cv)) return NULL;
    ASSUME(CvNAME_HEK(cv));
    svp = hv_fetchhek(CvSTASH(cv), CvNAME_HEK(cv), 0);
    gv = MUTABLE_GV(svp && *svp ? *svp : newSV_type(SVt_NULL));
    if (!isGV(gv))
        gv_init_pvn(gv, CvSTASH(cv), HEK_KEY(CvNAME_HEK(cv)),
                HEK_LEN(CvNAME_HEK(cv)),
                SVf_UTF8 * cBOOL(HEK_UTF8(CvNAME_HEK(cv))));
    if (!CvNAMED(cv)) { /* gv_init took care of it */
        assert (SvANY(cv)->xcv_gv_u.xcv_gv == gv);
        return gv;
    }
    unshare_hek(CvNAME_HEK(cv));
    CvNAMED_off(cv);
    SvANY(cv)->xcv_gv_u.xcv_gv = gv;
    if (svp && *svp) SvREFCNT_inc_simple_void_NN(gv);
    CvCVGV_RC_on(cv);
    return gv;
}

/* Assign CvSTASH(cv) = st, handling weak references. */

void
Perl_cvstash_set(pTHX_ CV *cv, HV *st)
{
    HV *oldst = CvSTASH(cv);
    PERL_ARGS_ASSERT_CVSTASH_SET;
    if (oldst == st)
        return;
    if (oldst)
        sv_del_backref(MUTABLE_SV(oldst), MUTABLE_SV(cv));
    SvANY(cv)->xcv_stash = st;
    if (st)
        Perl_sv_add_backref(aTHX_ MUTABLE_SV(st), MUTABLE_SV(cv));
}

/*
=for apidoc gv_init_pvn

Converts a scalar into a typeglob.  This is an incoercible typeglob;
assigning a reference to it will assign to one of its slots, instead of
overwriting it as happens with typeglobs created by C<SvSetSV>.  Converting
any scalar that is C<SvOK()> may produce unpredictable results and is reserved
for perl's internal use.

C<gv> is the scalar to be converted.

C<stash> is the parent stash/package, if any.

C<name> and C<len> give the name.  The name must be unqualified;
that is, it must not include the package name.  If C<gv> is a
stash element, it is the caller's responsibility to ensure that the name
passed to this function matches the name of the element.  If it does not
match, perl's internal bookkeeping will get out of sync.

C<flags> can be set to C<SVf_UTF8> if C<name> is a UTF-8 string, or
the return value of SvUTF8(sv).  It can also take the
C<GV_ADDMULTI> flag, which means to pretend that the GV has been
seen before (i.e., suppress "Used once" warnings).

=for apidoc Amnh||GV_ADDMULTI

=for apidoc gv_init

The old form of C<gv_init_pvn()>.  It does not work with UTF-8 strings, as it
has no flags parameter.  If the C<multi> parameter is set, the
C<GV_ADDMULTI> flag will be passed to C<gv_init_pvn()>.

=for apidoc gv_init_pv

Same as C<gv_init_pvn()>, but takes a nul-terminated string for the name
instead of separate char * and length parameters.

=for apidoc gv_init_sv

Same as C<gv_init_pvn()>, but takes an SV * for the name instead of separate
char * and length parameters.  C<flags> is currently unused.

=cut
*/

void
Perl_gv_init_sv(pTHX_ GV *gv, HV *stash, SV* namesv, U32 flags)
{
   char *namepv;
   STRLEN namelen;
   PERL_ARGS_ASSERT_GV_INIT_SV;
   namepv = SvPV(namesv, namelen);
   if (SvUTF8(namesv))
       flags |= SVf_UTF8;
   gv_init_pvn(gv, stash, namepv, namelen, flags);
}

void
Perl_gv_init_pv(pTHX_ GV *gv, HV *stash, const char *name, U32 flags)
{
   PERL_ARGS_ASSERT_GV_INIT_PV;
   gv_init_pvn(gv, stash, name, strlen(name), flags);
}

/* Packages in the symbol table are "stashes" - hashes where the keys are symbol
   names and the values are typeglobs. The value $foo::bar is actually found
   by looking up the typeglob *foo::{bar} and then reading its SCALAR slot.

   At least, that's what you see in Perl space if you use typeglob syntax.
   Usually it's also what's actually stored in the stash, but for some cases
   different values are stored (as a space optimisation) and converted to full
   typeglobs "on demand" - if a typeglob syntax is used to read a value. It's
   the job of this function, Perl_gv_init_pvn(), to undo any trickery and
   replace the SV stored in the stash with the regular PVGV structure that it is
   a shorthand for. This has to be done "in-place" by upgrading the actual SV
   that is already stored in the stash to a PVGV.

   As the public documentation above says:
       Converting any scalar that is C<SvOK()> may produce unpredictable
       results and is reserved for perl's internal use.

   Values that can be stored:

   * plain scalar - a subroutine declaration
     The scalar's string value is the subroutine prototype; the integer -1 is
     "no prototype". ie shorthand for sub foo ($$); or sub bar;
   * reference to a scalar - a constant. ie shorthand for sub PI() { 4; }
   * reference to a sub - a subroutine (avoids allocating a PVGV)

   The earliest optimisation was subroutine declarations, implemented in 1998
   by commit 8472ac73d6d80294:
      "Sub declaration cost reduced from ~500 to ~100 bytes"

   This space optimisation needs to be invisible to regular Perl code. For this
   code:

         sub foo ($$);
         *foo = [];

   When the first line is compiled, the optimisation is used, and $::{foo} is
   assigned the scalar '$$'. No PVGV or PVCV is created.

   When the second line encountered, the typeglob lookup on foo needs to
   "upgrade" the symbol table entry to a PVGV, and then create a PVCV in the
   {CODE} slot with the prototype $$ and no body. The typeglob is then available
   so that [] can be assigned to the {ARRAY} slot. For the code above the
   upgrade happens at compile time, the assignment at runtime.

   Analogous code unwinds the other optimisations.
*/
void
Perl_gv_init_pvn(pTHX_ GV *gv, HV *stash, const char *name, STRLEN len, U32 flags)
{
    const U32 old_type = SvTYPE(gv);
    const bool doproto = old_type > SVt_NULL;
    char * const proto = (doproto && SvPOK(gv))
        ? ((void)(SvIsCOW(gv) && (sv_force_normal((SV *)gv), 0)), SvPVX(gv))
        : NULL;
    const STRLEN protolen = proto ? SvCUR(gv) : 0;
    const U32 proto_utf8  = proto ? SvUTF8(gv) : 0;
    SV *const has_constant = doproto && SvROK(gv) ? SvRV(gv) : NULL;
    const U32 exported_constant = has_constant ? SvPCS_IMPORTED(gv) : 0;
    const bool really_sub =
        has_constant && SvTYPE(has_constant) == SVt_PVCV;
    COP * const old = PL_curcop;

    PERL_ARGS_ASSERT_GV_INIT_PVN;
    assert (!(proto && has_constant));

    if (has_constant) {
        /* The constant has to be a scalar, array or subroutine.  */
        switch (SvTYPE(has_constant)) {
        case SVt_PVHV:
        case SVt_PVFM:
        case SVt_PVIO:
            Perl_croak(aTHX_ "Cannot convert a reference to %s to typeglob",
                       sv_reftype(has_constant, 0));
            NOT_REACHED; /* NOTREACHED */
            break;

        default: NOOP;
        }
        SvRV_set(gv, NULL);
        SvROK_off(gv);
    }


    if (old_type < SVt_PVGV) {
        if (old_type >= SVt_PV)
            SvCUR_set(gv, 0);
        sv_upgrade(MUTABLE_SV(gv), SVt_PVGV);
    }
    if (SvLEN(gv)) {
        if (proto) {
            /* For this case, we are "stealing" the buffer from the SvPV and
               re-attaching to an SV below with the call to sv_usepvn_flags().
               Hence we don't free it. */
            SvPV_set(gv, NULL);
        }
        else {
            /* There is no valid prototype. (SvPOK() must be true for a valid
               prototype.) Hence we free the memory. */
            Safefree(SvPVX_mutable(gv));
        }
        SvLEN_set(gv, 0);
        SvPOK_off(gv);
    }
    SvIOK_off(gv);
    isGV_with_GP_on(gv);

    if (really_sub && !CvISXSUB(has_constant) && CvSTART(has_constant)
     && (  CvSTART(has_constant)->op_type == OP_NEXTSTATE
        || CvSTART(has_constant)->op_type == OP_DBSTATE))
        PL_curcop = (COP *)CvSTART(has_constant);
    GvGP_set(gv, Perl_newGP(aTHX_ gv));
    PL_curcop = old;
    GvSTASH(gv) = stash;
    if (stash)
        Perl_sv_add_backref(aTHX_ MUTABLE_SV(stash), MUTABLE_SV(gv));
    gv_name_set(gv, name, len, GV_ADD | ( flags & SVf_UTF8 ? SVf_UTF8 : 0 ));
    if (flags & GV_ADDMULTI || doproto)	/* doproto means it */
        GvMULTI_on(gv);			/* _was_ mentioned */
    if (really_sub) {
        /* Not actually a constant.  Just a regular sub.  */
        CV * const cv = (CV *)has_constant;
        GvCV_set(gv,cv);
        if (CvNAMED(cv) && CvSTASH(cv) == stash && (
               CvNAME_HEK(cv) == GvNAME_HEK(gv)
            || (  HEK_LEN(CvNAME_HEK(cv)) == HEK_LEN(GvNAME_HEK(gv))
               && HEK_FLAGS(CvNAME_HEK(cv)) != HEK_FLAGS(GvNAME_HEK(gv))
               && HEK_UTF8(CvNAME_HEK(cv)) == HEK_UTF8(GvNAME_HEK(gv))
               && memEQ(HEK_KEY(CvNAME_HEK(cv)), GvNAME(gv), GvNAMELEN(gv))
               )
           ))
            CvGV_set(cv,gv);
    }
    else if (doproto) {
        CV *cv;
        if (has_constant) {
            /* newCONSTSUB takes ownership of the reference from us.  */
            cv = newCONSTSUB_flags(stash, name, len, flags, has_constant);
            /* In case op.c:S_process_special_blocks stole it: */
            if (!GvCV(gv))
                GvCV_set(gv, (CV *)SvREFCNT_inc_simple_NN(cv));
            assert(GvCV(gv) == cv); /* newCONSTSUB should have set this */
            /* If this reference was a copy of another, then the subroutine
               must have been "imported", by a Perl space assignment to a GV
               from a reference to CV.  */
            if (exported_constant)
                GvIMPORTED_CV_on(gv);
            CvSTASH_set(cv, PL_curstash); /* XXX Why is this needed? */
        } else {
            cv = newSTUB(gv,1);
        }
        if (proto) {
            sv_usepvn_flags(MUTABLE_SV(cv), proto, protolen,
                            SV_HAS_TRAILING_NUL);
            if ( proto_utf8 ) SvUTF8_on(MUTABLE_SV(cv));
        }
    }
}

STATIC void
S_gv_init_svtype(pTHX_ GV *gv, const svtype sv_type)
{
    PERL_ARGS_ASSERT_GV_INIT_SVTYPE;

    switch (sv_type) {
    case SVt_PVIO:
        (void)GvIOn(gv);
        break;
    case SVt_PVAV:
        (void)GvAVn(gv);
        break;
    case SVt_PVHV:
        (void)GvHVn(gv);
        break;
#ifdef PERL_DONT_CREATE_GVSV
    case SVt_NULL:
    case SVt_PVCV:
    case SVt_PVFM:
    case SVt_PVGV:
        break;
    default:
        if(GvSVn(gv)) {
            /* Work round what appears to be a bug in Sun C++ 5.8 2005/10/13
               If we just cast GvSVn(gv) to void, it ignores evaluating it for
               its side effect */
        }
#endif
    }
}

static void core_xsub(pTHX_ CV* cv);

static GV *
S_maybe_add_coresub(pTHX_ HV * const stash, GV *gv,
                          const char * const name, const STRLEN len)
{
    const int code = keyword(name, len, 1);
    static const char file[] = __FILE__;
    CV *cv, *oldcompcv = NULL;
    int opnum = 0;
    bool ampable = TRUE; /* &{}-able */
    COP *oldcurcop = NULL;
    yy_parser *oldparser = NULL;
    I32 oldsavestack_ix = 0;

    assert(gv || stash);
    assert(name);

    if (!code) return NULL; /* Not a keyword */
    switch (code < 0 ? -code : code) {
     /* no support for \&CORE::infix;
        no support for funcs that do not parse like funcs */
    case KEY___DATA__: case KEY___END__ :
    case KEY_ADJUST  : case KEY_AUTOLOAD: case KEY_BEGIN : case KEY_CHECK :
    case KEY_DESTROY : case KEY_END     : case KEY_INIT  : case KEY_UNITCHECK:
    case KEY_and     : case KEY_catch  : case KEY_class  :
    case KEY_cmp     : case KEY_default: case KEY_defer :
    case KEY_do      : case KEY_dump   : case KEY_else  : case KEY_elsif  :
    case KEY_eq     : case KEY_eval  : case KEY_field  :
    case KEY_finally:
    case KEY_for     : case KEY_foreach: case KEY_format: case KEY_ge     :
    case KEY_given   : case KEY_goto   : case KEY_grep  : case KEY_gt     :
    case KEY_if      : case KEY_isa    : 
    case KEY_last   :
    case KEY_le      : case KEY_local  : case KEY_lt    : case KEY_m      :
    case KEY_map     : case KEY_method : case KEY_my    :
    case KEY_ne   : case KEY_next : case KEY_no: case KEY_or: case KEY_our:
    case KEY_package: case KEY_print: case KEY_printf:
    case KEY_q    : case KEY_qq   : case KEY_qr     : case KEY_qw    :
    case KEY_qx   : case KEY_redo : case KEY_require: case KEY_return:
    case KEY_s    : case KEY_say  : case KEY_sort   :
    case KEY_state: case KEY_sub  :
    case KEY_tr   : case KEY_try  :
    case KEY_unless:
    case KEY_until: case KEY_use  : case KEY_when     : case KEY_while :
    case KEY_x    : case KEY_xor  : case KEY_y        :
        return NULL;
    case KEY_chdir:
    case KEY_chomp: case KEY_chop: case KEY_defined: case KEY_delete:
    case KEY_eof  : case KEY_exec: case KEY_exists :
    case KEY_lstat:
    case KEY_split:
    case KEY_stat:
    case KEY_system:
    case KEY_truncate: case KEY_unlink:
        ampable = FALSE;
    }
    if (!gv) {
        gv = (GV *)newSV_type(SVt_NULL);
        gv_init(gv, stash, name, len, TRUE);
    }
    GvMULTI_on(gv);
    if (ampable) {
        ENTER;
        oldcurcop = PL_curcop;
        oldparser = PL_parser;
        lex_start(NULL, NULL, 0);
        oldcompcv = PL_compcv;
        PL_compcv = NULL; /* Prevent start_subparse from setting
                             CvOUTSIDE. */
        oldsavestack_ix = start_subparse(FALSE,0);
        cv = PL_compcv;
    }
    else {
        /* Avoid calling newXS, as it calls us, and things start to
           get hairy. */
        cv = MUTABLE_CV(newSV_type(SVt_PVCV));
        GvCV_set(gv,cv);
        GvCVGEN(gv) = 0;
        CvISXSUB_on(cv);
        CvXSUB(cv) = core_xsub;
        PoisonPADLIST(cv);
    }
    CvGV_set(cv, gv); /* This stops new ATTRSUB from setting CvFILE
                         from PL_curcop. */
    /* XSUBs can't be perl lang/perl5db.pl debugged
    if (PERLDB_LINE_OR_SAVESRC)
        (void)gv_fetchfile(file); */
    CvFILE(cv) = (char *)file;
    /* XXX This is inefficient, as doing things this order causes
           a prototype check in newATTRSUB.  But we have to do
           it this order as we need an op number before calling
           new ATTRSUB. */
    (void)core_prototype((SV *)cv, name, code, &opnum);
    if (stash)
        (void)hv_store(stash,name,len,(SV *)gv,0);
    if (ampable) {
#ifdef DEBUGGING
        CV *orig_cv = cv;
#endif
        CvLVALUE_on(cv);
        /* newATTRSUB will free the CV and return NULL if we're still
           compiling after a syntax error */
        if ((cv = newATTRSUB_x(
                   oldsavestack_ix, (OP *)gv,
                   NULL,NULL,
                   coresub_op(
                     opnum
                       ? newSVuv((UV)opnum)
                       : newSVpvn(name,len),
                     code, opnum
                   ),
                   TRUE
               )) != NULL) {
            assert(GvCV(gv) == orig_cv);
            if (opnum != OP_VEC && opnum != OP_SUBSTR && opnum != OP_POS
                && opnum != OP_UNDEF && opnum != OP_KEYS)
                CvLVALUE_off(cv); /* Now *that* was a neat trick. */
        }
        LEAVE;
        PL_parser = oldparser;
        PL_curcop = oldcurcop;
        PL_compcv = oldcompcv;
    }
    if (cv) {
        SV *opnumsv = newSViv(
            (opnum == OP_ENTEREVAL && len == 9 && memEQ(name, "evalbytes", 9)) ?
                (OP_ENTEREVAL | (1<<16))
            : opnum ? opnum : (((I32)name[2]) << 16));
        cv_set_call_checker_flags(cv, Perl_ck_entersub_args_core, opnumsv, 0);
        SvREFCNT_dec_NN(opnumsv);
    }

    return gv;
}

/*
=for apidoc      gv_fetchmeth
=for apidoc_item gv_fetchmeth_pv
=for apidoc_item gv_fetchmeth_pvn
=for apidoc_item gv_fetchmeth_sv

These each look for a glob with name C<name>, containing a defined subroutine,
returning the GV of that glob if found, or C<NULL> if not.

C<stash> is always searched (first), unless it is C<NULL>.

If C<stash> is NULL, or was searched but nothing was found in it, and the
C<GV_SUPER> bit is set in C<flags>, stashes accessible via C<@ISA> are searched
next.  Searching is conducted according to L<C<MRO> order|perlmroapi>.

Finally, if no matches were found so far, and the C<GV_NOUNIVERSAL> flag in
C<flags> is not set,  C<UNIVERSAL::> is searched.

The argument C<level> should be either 0 or -1.  If -1, the function will
return without any side effects or caching.  If 0, the function makes sure
there is a glob named C<name> in C<stash>, creating one if necessary.
The subroutine slot in the glob will be set to any subroutine found in the
C<stash> and C<SUPER::> search, hence caching any C<SUPER::> result.  Note that
subroutines found in C<UNIVERSAL::> are not cached.

The GV returned from these may be a method cache entry, which is not visible to
Perl code.  So when calling C<call_sv>, you should not use the GV directly;
instead, you should use the method's CV, which can be obtained from the GV with
the C<GvCV> macro.

The only other significant value for C<flags> is C<SVf_UTF8>, indicating that
C<name> is to be treated as being encoded in UTF-8.

Plain C<gv_fetchmeth> lacks a C<flags> parameter, hence always searches in
C<stash>, then C<UNIVERSAL::>, and C<name> is never UTF-8.  Otherwise it is
exactly like C<gv_fetchmeth_pvn>.

The other forms do have a C<flags> parameter, and differ only in how the glob
name is specified.

In C<gv_fetchmeth_pv>, C<name> is a C language NUL-terminated string.

In C<gv_fetchmeth_pvn>, C<name> points to the first byte of the name, and an
additional parameter, C<len>, specifies its length in bytes.  Hence, the name
may contain embedded-NUL characters.

In C<gv_fetchmeth_sv>, C<*name> is an SV, and the name is the PV extracted from
that, using L</C<SvPV>>.  If the SV is marked as being in UTF-8, the extracted
PV will also be.

=for apidoc Amnh||GV_SUPER

=cut
*/

GV *
Perl_gv_fetchmeth_sv(pTHX_ HV *stash, SV *namesv, I32 level, U32 flags)
{
    char *namepv;
    STRLEN namelen;
    PERL_ARGS_ASSERT_GV_FETCHMETH_SV;
    if (LIKELY(SvPOK_nog(namesv))) /* common case */
        return gv_fetchmeth_internal(stash, namesv, NULL, 0, level,
                                     flags | SvUTF8(namesv));
    namepv = SvPV(namesv, namelen);
    if (SvUTF8(namesv)) flags |= SVf_UTF8;
    return gv_fetchmeth_pvn(stash, namepv, namelen, level, flags);
}


GV *
Perl_gv_fetchmeth_pv(pTHX_ HV *stash, const char *name, I32 level, U32 flags)
{
    PERL_ARGS_ASSERT_GV_FETCHMETH_PV;
    return gv_fetchmeth_internal(stash, NULL, name, strlen(name), level, flags);
}

/* NOTE: No support for tied ISA */

PERL_STATIC_INLINE GV*
S_gv_fetchmeth_internal(pTHX_ HV* stash, SV* meth, const char* name, STRLEN len, I32 level, U32 flags)
{
    GV** gvp;
    HE* he;
    AV* linear_av;
    SV** linear_svp;
    SV* linear_sv;
    HV* cstash, *cachestash;
    GV* candidate = NULL;
    CV* cand_cv = NULL;
    GV* topgv = NULL;
    const char *hvname;
    STRLEN hvnamelen;
    I32 create = (level >= 0) ? HV_FETCH_LVALUE : 0;
    I32 items;
    U32 topgen_cmp;
    U32 is_utf8 = flags & SVf_UTF8;

    /* UNIVERSAL methods should be callable without a stash */
    if (!stash) {
        create = 0;  /* probably appropriate */
        if(!(stash = gv_stashpvs("UNIVERSAL", 0)))
            return 0;
    }

    assert(stash);

    hvname = HvNAME_get(stash);
    hvnamelen = HvNAMELEN_get(stash);
    if (!hvname)
      Perl_croak(aTHX_ "Can't use anonymous symbol table for method lookup");

    assert(hvname);
    assert(name || meth);

    DEBUG_o( Perl_deb(aTHX_ "Looking for %smethod %s in package %s\n",
                      flags & GV_SUPER ? "SUPER " : "",
                      name ? name : SvPV_nolen(meth), hvname) );

    topgen_cmp = HvMROMETA(stash)->cache_gen + PL_sub_generation;

    if (flags & GV_SUPER) {
        if (!HvAUX(stash)->xhv_mro_meta->super)
            HvAUX(stash)->xhv_mro_meta->super = newHV();
        cachestash = HvAUX(stash)->xhv_mro_meta->super;
    }
    else cachestash = stash;

    /* check locally for a real method or a cache entry */
    he = (HE*)hv_common(
        cachestash, meth, name, len, is_utf8 ? HVhek_UTF8 : 0, create, NULL, 0
    );
    if (he) gvp = (GV**)&HeVAL(he);
    else gvp = NULL;

    if(gvp) {
        topgv = *gvp;
      have_gv:
        assert(topgv);
        if (SvTYPE(topgv) != SVt_PVGV)
        {
            if (!name)
                name = SvPV_nomg(meth, len);
            gv_init_pvn(topgv, stash, name, len, GV_ADDMULTI|is_utf8);
        }
        if ((cand_cv = GvCV(topgv))) {
            /* If genuine method or valid cache entry, use it */
            if (!GvCVGEN(topgv) || GvCVGEN(topgv) == topgen_cmp) {
                return topgv;
            }
            else {
                /* stale cache entry, junk it and move on */
                SvREFCNT_dec_NN(cand_cv);
                GvCV_set(topgv, NULL);
                cand_cv = NULL;
                GvCVGEN(topgv) = 0;
            }
        }
        else if (GvCVGEN(topgv) == topgen_cmp) {
            /* cache indicates no such method definitively */
            return 0;
        }
        else if (stash == cachestash
              && len > 1 /* shortest is uc */
              && memEQs(hvname, HvNAMELEN_get(stash), "CORE")
              && S_maybe_add_coresub(aTHX_ NULL,topgv,name,len))
            goto have_gv;
    }

    linear_av = mro_get_linear_isa(stash); /* has ourselves at the top of the list */
    linear_svp = AvARRAY(linear_av) + 1; /* skip over self */
    items = AvFILLp(linear_av); /* no +1, to skip over self */
    while (items--) {
        linear_sv = *linear_svp++;
        assert(linear_sv);
        cstash = gv_stashsv(linear_sv, 0);

        if (!cstash) {
            if ( ckWARN(WARN_SYNTAX)) {
                if(     /* these are loaded from Perl_Gv_AMupdate() one way or another */
                           ( len    && name[0] == '(' )  /* overload.pm related, in particular "()" */
                        || ( memEQs( name, len, "DESTROY") )
                ) {
                     Perl_ck_warner(aTHX_ packWARN(WARN_SYNTAX),
                            "Can't locate package %" SVf " for @%" HEKf "::ISA",
                            SVfARG(linear_sv),
                            HEKfARG(HvNAME_HEK(stash)));

                } else if( memEQs( name, len, "AUTOLOAD") ) {
                    /* gobble this warning */
                } else {
                    Perl_ck_warner(aTHX_ packWARN(WARN_SYNTAX),
                        "While trying to resolve method call %.*s->%.*s()"
                        " can not locate package %" SVf_QUOTEDPREFIX " yet it is mentioned in @%.*s::ISA"
                        " (perhaps you forgot to load %" SVf_QUOTEDPREFIX "?)",
                         (int) hvnamelen, hvname,
                         (int) len, name,
                        SVfARG(linear_sv),
                         (int) hvnamelen, hvname,
                         SVfARG(linear_sv));
                }
            }
            continue;
        }

        assert(cstash);

        gvp = (GV**)hv_common(
            cstash, meth, name, len, is_utf8 ? HVhek_UTF8 : 0, HV_FETCH_JUST_SV, NULL, 0
        );
        if (!gvp) {
            if (len > 1 && HvNAMELEN_get(cstash) == 4) {
                const char *hvname = HvNAME(cstash); assert(hvname);
                if (strBEGINs(hvname, "CORE")
                 && (candidate =
                      S_maybe_add_coresub(aTHX_ cstash,NULL,name,len)
                    ))
                    goto have_candidate;
            }
            continue;
        }
        else candidate = *gvp;
       have_candidate:
        assert(candidate);
        if (SvTYPE(candidate) != SVt_PVGV)
            gv_init_pvn(candidate, cstash, name, len, GV_ADDMULTI|is_utf8);
        if (SvTYPE(candidate) == SVt_PVGV && (cand_cv = GvCV(candidate)) && !GvCVGEN(candidate)) {
            /*
             * Found real method, cache method in topgv if:
             *  1. topgv has no synonyms (else inheritance crosses wires)
             *  2. method isn't a stub (else AUTOLOAD fails spectacularly)
             */
            if (topgv && (GvREFCNT(topgv) == 1) && (CvROOT(cand_cv) || CvXSUB(cand_cv))) {
                  CV *old_cv = GvCV(topgv);
                  SvREFCNT_dec(old_cv);
                  SvREFCNT_inc_simple_void_NN(cand_cv);
                  GvCV_set(topgv, cand_cv);
                  GvCVGEN(topgv) = topgen_cmp;
            }
            return candidate;
        }
    }

    /* Check UNIVERSAL without caching */
    if((level == 0 || level == -1) && !(flags & GV_NOUNIVERSAL)) {
        candidate = gv_fetchmeth_internal(NULL, meth, name, len, 1,
                                          flags &~GV_SUPER);
        if(candidate) {
            cand_cv = GvCV(candidate);
            if (topgv && (GvREFCNT(topgv) == 1) && (CvROOT(cand_cv) || CvXSUB(cand_cv))) {
                  CV *old_cv = GvCV(topgv);
                  SvREFCNT_dec(old_cv);
                  SvREFCNT_inc_simple_void_NN(cand_cv);
                  GvCV_set(topgv, cand_cv);
                  GvCVGEN(topgv) = topgen_cmp;
            }
            return candidate;
        }
    }

    if (topgv && GvREFCNT(topgv) == 1 && !(flags & GV_NOUNIVERSAL)) {
        /* cache the fact that the method is not defined */
        GvCVGEN(topgv) = topgen_cmp;
    }

    return 0;
}

GV *
Perl_gv_fetchmeth_pvn(pTHX_ HV *stash, const char *name, STRLEN len, I32 level, U32 flags)
{
    PERL_ARGS_ASSERT_GV_FETCHMETH_PVN;
    return gv_fetchmeth_internal(stash, NULL, name, len, level, flags);
}

/*
=for apidoc gv_fetchmeth_autoload

This is the old form of L</gv_fetchmeth_pvn_autoload>, which has no flags
parameter.

=for apidoc gv_fetchmeth_sv_autoload

Exactly like L</gv_fetchmeth_pvn_autoload>, but takes the name string in the form
of an SV instead of a string/length pair.

=cut
*/

GV *
Perl_gv_fetchmeth_sv_autoload(pTHX_ HV *stash, SV *namesv, I32 level, U32 flags)
{
   char *namepv;
   STRLEN namelen;
   PERL_ARGS_ASSERT_GV_FETCHMETH_SV_AUTOLOAD;
   namepv = SvPV(namesv, namelen);
   if (SvUTF8(namesv))
       flags |= SVf_UTF8;
   return gv_fetchmeth_pvn_autoload(stash, namepv, namelen, level, flags);
}

/*
=for apidoc gv_fetchmeth_pv_autoload

Exactly like L</gv_fetchmeth_pvn_autoload>, but takes a nul-terminated string
instead of a string/length pair.

=cut
*/

GV *
Perl_gv_fetchmeth_pv_autoload(pTHX_ HV *stash, const char *name, I32 level, U32 flags)
{
    PERL_ARGS_ASSERT_GV_FETCHMETH_PV_AUTOLOAD;
    return gv_fetchmeth_pvn_autoload(stash, name, strlen(name), level, flags);
}

/*
=for apidoc gv_fetchmeth_pvn_autoload

Same as C<gv_fetchmeth_pvn()>, but looks for autoloaded subroutines too.
Returns a glob for the subroutine.

For an autoloaded subroutine without a GV, will create a GV even
if C<level < 0>.  For an autoloaded subroutine without a stub, C<GvCV()>
of the result may be zero.

Currently, the only significant value for C<flags> is C<SVf_UTF8>.

=cut
*/

GV *
Perl_gv_fetchmeth_pvn_autoload(pTHX_ HV *stash, const char *name, STRLEN len, I32 level, U32 flags)
{
    GV *gv = gv_fetchmeth_pvn(stash, name, len, level, flags);

    PERL_ARGS_ASSERT_GV_FETCHMETH_PVN_AUTOLOAD;

    if (!gv) {
        CV *cv;
        GV **gvp;

        if (!stash)
            return NULL;	/* UNIVERSAL::AUTOLOAD could cause trouble */
        if (len == S_autolen && memEQ(name, S_autoload, S_autolen))
            return NULL;
        if (!(gv = gv_fetchmeth_pvn(stash, S_autoload, S_autolen, FALSE, flags)))
            return NULL;
        cv = GvCV(gv);
        if (!(CvROOT(cv) || CvXSUB(cv)))
            return NULL;
        /* Have an autoload */
        if (level < 0)	/* Cannot do without a stub */
            gv_fetchmeth_pvn(stash, name, len, 0, flags);
        gvp = (GV**)hv_fetch(stash, name,
                        (flags & SVf_UTF8) ? -(I32)len : (I32)len, (level >= 0));
        if (!gvp)
            return NULL;
        return *gvp;
    }
    return gv;
}

/*
=for apidoc gv_fetchmethod_autoload

Returns the glob which contains the subroutine to call to invoke the method
on the C<stash>.  In fact in the presence of autoloading this may be the
glob for "AUTOLOAD".  In this case the corresponding variable C<$AUTOLOAD> is
already setup.

The third parameter of C<gv_fetchmethod_autoload> determines whether
AUTOLOAD lookup is performed if the given method is not present: non-zero
means yes, look for AUTOLOAD; zero means no, don't look for AUTOLOAD.
Calling C<gv_fetchmethod> is equivalent to calling C<gv_fetchmethod_autoload>
with a non-zero C<autoload> parameter.

These functions grant C<"SUPER"> token
as a prefix of the method name.  Note
that if you want to keep the returned glob for a long time, you need to
check for it being "AUTOLOAD", since at the later time the call may load a
different subroutine due to C<$AUTOLOAD> changing its value.  Use the glob
created as a side effect to do this.

These functions have the same side-effects as C<gv_fetchmeth> with
C<level==0>.  The warning against passing the GV returned by
C<gv_fetchmeth> to C<call_sv> applies equally to these functions.

=cut
*/

GV *
Perl_gv_fetchmethod_autoload(pTHX_ HV *stash, const char *name, I32 autoload)
{
    PERL_ARGS_ASSERT_GV_FETCHMETHOD_AUTOLOAD;

    return gv_fetchmethod_flags(stash, name, autoload ? GV_AUTOLOAD : 0);
}

GV *
Perl_gv_fetchmethod_sv_flags(pTHX_ HV *stash, SV *namesv, U32 flags)
{
    char *namepv;
    STRLEN namelen;
    PERL_ARGS_ASSERT_GV_FETCHMETHOD_SV_FLAGS;
    namepv = SvPV(namesv, namelen);
    if (SvUTF8(namesv))
       flags |= SVf_UTF8;
    return gv_fetchmethod_pvn_flags(stash, namepv, namelen, flags);
}

GV *
Perl_gv_fetchmethod_pv_flags(pTHX_ HV *stash, const char *name, U32 flags)
{
    PERL_ARGS_ASSERT_GV_FETCHMETHOD_PV_FLAGS;
    return gv_fetchmethod_pvn_flags(stash, name, strlen(name), flags);
}

GV *
Perl_gv_fetchmethod_pvn_flags(pTHX_ HV *stash, const char *name, const STRLEN len, U32 flags)
{
    const char * const origname = name;
    const char * const name_end = name + len;
    const char *last_separator = NULL;
    GV* gv;
    HV* ostash = stash;
    SV *const error_report = MUTABLE_SV(stash);
    const U32 autoload = flags & GV_AUTOLOAD;
    const U32 do_croak = flags & GV_CROAK;
    const U32 is_utf8  = flags & SVf_UTF8;

    PERL_ARGS_ASSERT_GV_FETCHMETHOD_PVN_FLAGS;

    if (SvTYPE(stash) < SVt_PVHV)
        stash = NULL;
    else {
        /* The only way stash can become NULL later on is if last_separator is set,
           which in turn means that there is no need for a SVt_PVHV case
           the error reporting code.  */
    }

    {
        /* check if the method name is fully qualified or
         * not, and separate the package name from the actual
         * method name.
         *
         * leaves last_separator pointing to the beginning of the
         * last package separator (either ' or ::) or 0
         * if none was found.
         *
         * leaves name pointing at the beginning of the
         * method name.
         */
        const char *name_cursor = name;
        const char * const name_em1 = name_end - 1; /* name_end minus 1 */
        for (name_cursor = name; name_cursor < name_end ; name_cursor++) {
            if (*name_cursor == '\'') {
                last_separator = name_cursor;
                name = name_cursor + 1;
            }
            else if (name_cursor < name_em1 && *name_cursor == ':' && name_cursor[1] == ':') {
                last_separator = name_cursor++;
                name = name_cursor + 1;
            }
        }
    }

    /* did we find a separator? */
    if (last_separator) {
        STRLEN sep_len= last_separator - origname;
        if ( memEQs(origname, sep_len, "SUPER")) {
            /* ->SUPER::method should really be looked up in original stash */
            stash = CopSTASH(PL_curcop);
            flags |= GV_SUPER;
            DEBUG_o( Perl_deb(aTHX_ "Treating %s as %s::%s\n",
                         origname, HvENAME_get(stash), name) );
        }
        else if ( sep_len >= 7 &&
                 strBEGINs(last_separator - 7, "::SUPER")) {
            /* don't autovivify if ->NoSuchStash::SUPER::method */
            stash = gv_stashpvn(origname, sep_len - 7, is_utf8);
            if (stash) flags |= GV_SUPER;
        }
        else {
            /* don't autovivify if ->NoSuchStash::method */
            stash = gv_stashpvn(origname, sep_len, is_utf8);
        }
        ostash = stash;
    }

    gv = gv_fetchmeth_pvn(stash, name, name_end - name, 0, flags);
    if (!gv) {
        /* This is the special case that exempts Foo->import and
           Foo->unimport from being an error even if there's no
          import/unimport subroutine */
        if (strEQ(name,"import") || strEQ(name,"unimport")) {
            gv = (GV*)sv_2mortal((SV*)newCONSTSUB_flags(NULL,
                                                NULL, 0, 0, NULL));
        } else if (autoload)
            gv = gv_autoload_pvn(
                ostash, name, name_end - name, GV_AUTOLOAD_ISMETHOD|flags
            );
        if (!gv && do_croak) {
            /* Right now this is exclusively for the benefit of S_method_common
               in pp_hot.c  */
            if (stash) {
                /* If we can't find an IO::File method, it might be a call on
                 * a filehandle. If IO:File has not been loaded, try to
                 * require it first instead of croaking */
                const char *stash_name = HvNAME_get(stash);
                if (stash_name && memEQs(stash_name, HvNAMELEN_get(stash), "IO::File")
                    && !Perl_hv_common(aTHX_ GvHVn(PL_incgv), NULL,
                                       STR_WITH_LEN("IO/File.pm"), 0,
                                       HV_FETCH_ISEXISTS, NULL, 0)
                ) {
                    require_pv("IO/File.pm");
                    gv = gv_fetchmeth_pvn(stash, name, name_end - name, 0, flags);
                    if (gv)
                        return gv;
                }
                Perl_croak(aTHX_
                           "Can't locate object method %" UTF8f_QUOTEDPREFIX ""
                           " via package %" HEKf_QUOTEDPREFIX,
                                    UTF8fARG(is_utf8, name_end - name, name),
                                    HEKfARG(HvNAME_HEK(stash)));
            }
            else {
                SV* packnamesv;

                if (last_separator) {
                    packnamesv = newSVpvn_flags(origname, last_separator - origname,
                                                    SVs_TEMP | is_utf8);
                } else {
                    packnamesv = error_report;
                }

                Perl_croak(aTHX_
                           "Can't locate object method %" UTF8f_QUOTEDPREFIX ""
                           " via package %" SVf_QUOTEDPREFIX ""
                           " (perhaps you forgot to load %" SVf_QUOTEDPREFIX "?)",
                           UTF8fARG(is_utf8, name_end - name, name),
                           SVfARG(packnamesv), SVfARG(packnamesv));
            }
        }
    }
    else if (autoload) {
        CV* const cv = GvCV(gv);
        if (!CvROOT(cv) && !CvXSUB(cv)) {
            GV* stubgv;
            GV* autogv;

            if (CvANON(cv) || CvLEXICAL(cv))
                stubgv = gv;
            else {
                stubgv = CvGV(cv);
                if (GvCV(stubgv) != cv)		/* orphaned import */
                    stubgv = gv;
            }
            autogv = gv_autoload_pvn(GvSTASH(stubgv),
                                  GvNAME(stubgv), GvNAMELEN(stubgv),
                                  GV_AUTOLOAD_ISMETHOD
                                   | (GvNAMEUTF8(stubgv) ? SVf_UTF8 : 0));
            if (autogv)
                gv = autogv;
        }
    }

    return gv;
}


/*
=for apidoc      gv_autoload_pv
=for apidoc_item gv_autoload_pvn
=for apidoc_item gv_autoload_sv

These each search for an C<AUTOLOAD> method, returning NULL if not found, or
else returning a pointer to its GV, while setting the package
L<C<$AUTOLOAD>|perlobj/AUTOLOAD> variable to C<name> (fully qualified).  Also,
if found and the GV's CV is an XSUB, the CV's PV will be set to C<name>, and
its stash will be set to the stash of the GV.

Searching is done in L<C<MRO> order|perlmroapi>, as specified in
L</C<gv_fetchmeth>>, beginning with C<stash> if it isn't NULL.

The forms differ only in how C<name> is specified.

In C<gv_autoload_pv>, C<namepv> is a C language NUL-terminated string.

In C<gv_autoload_pvn>, C<name> points to the first byte of the name, and an
additional parameter, C<len>, specifies its length in bytes.  Hence, C<*name>
may contain embedded-NUL characters.

In C<gv_autoload_sv>, C<*namesv> is an SV, and the name is the PV extracted
from that using L</C<SvPV>>.  If the SV is marked as being in UTF-8, the
extracted PV will also be.

=cut
*/

GV*
Perl_gv_autoload_sv(pTHX_ HV *stash, SV* namesv, U32 flags)
{
   char *namepv;
   STRLEN namelen;
   PERL_ARGS_ASSERT_GV_AUTOLOAD_SV;
   namepv = SvPV(namesv, namelen);
   if (SvUTF8(namesv))
       flags |= SVf_UTF8;
   return gv_autoload_pvn(stash, namepv, namelen, flags);
}

GV*
Perl_gv_autoload_pv(pTHX_ HV *stash, const char *namepv, U32 flags)
{
   PERL_ARGS_ASSERT_GV_AUTOLOAD_PV;
   return gv_autoload_pvn(stash, namepv, strlen(namepv), flags);
}

GV*
Perl_gv_autoload_pvn(pTHX_ HV *stash, const char *name, STRLEN len, U32 flags)
{
    GV* gv;
    CV* cv;
    HV* varstash;
    GV* vargv;
    SV* varsv;
    SV *packname = NULL;
    U32 is_utf8 = flags & SVf_UTF8 ? SVf_UTF8 : 0;

    PERL_ARGS_ASSERT_GV_AUTOLOAD_PVN;

    if (len == S_autolen && memEQ(name, S_autoload, S_autolen))
        return NULL;
    if (stash) {
        if (SvTYPE(stash) < SVt_PVHV) {
            STRLEN packname_len = 0;
            const char * const packname_ptr = SvPV_const(MUTABLE_SV(stash), packname_len);
            packname = newSVpvn_flags(packname_ptr, packname_len,
                                      SVs_TEMP | SvUTF8(stash));
            stash = NULL;
        }
        else
            packname = newSVhek_mortal(HvNAME_HEK(stash));
        if (flags & GV_SUPER) sv_catpvs(packname, "::SUPER");
    }
    if (!(gv = gv_fetchmeth_pvn(stash, S_autoload, S_autolen, FALSE,
                                is_utf8 | (flags & GV_SUPER))))
        return NULL;
    cv = GvCV(gv);

    if (!(CvROOT(cv) || CvXSUB(cv)))
        return NULL;

    /*
     * Inheriting AUTOLOAD for non-methods no longer works
     */
    if (
        !(flags & GV_AUTOLOAD_ISMETHOD)
     && (GvCVGEN(gv) || GvSTASH(gv) != stash)
    )
        Perl_croak(aTHX_ "Use of inherited AUTOLOAD for non-method %" SVf
                         "::%" UTF8f "() is no longer allowed",
                         SVfARG(packname),
                         UTF8fARG(is_utf8, len, name));

    if (CvISXSUB(cv)) {
        /* Instead of forcing the XSUB to do another lookup for $AUTOLOAD
         * and split that value on the last '::', pass along the same data
         * via the SvPVX field in the CV, and the stash in CvSTASH.
         *
         * Due to an unfortunate accident of history, the SvPVX field
         * serves two purposes.  It is also used for the subroutine's
         * prototype.  Since SvPVX has been documented as returning the sub
         * name for a long time, but not as returning the prototype, we have to
         * preserve the SvPVX AUTOLOAD behaviour and put the prototype
         * elsewhere.
         *
         * We put the prototype in the same allocated buffer, but after
         * the sub name.  The SvPOK flag indicates the presence of a proto-
         * type.  The CvAUTOLOAD flag indicates the presence of a sub name.
         * If both flags are on, then SvLEN is used to indicate the end of
         * the prototype (artificially lower than what is actually allo-
         * cated), at the risk of having to reallocate a few bytes unneces-
         * sarily--but that should happen very rarely, if ever.
         *
         * We use SvUTF8 for both prototypes and sub names, so if one is
         * UTF8, the other must be upgraded.
         */
        CvSTASH_set(cv, stash);
        if (SvPOK(cv)) { /* Ouch! */
            SV * const tmpsv = newSVpvn_flags(name, len, is_utf8);
            STRLEN ulen;
            const char *proto = CvPROTO(cv);
            assert(proto);
            if (SvUTF8(cv))
                sv_utf8_upgrade_flags_grow(tmpsv, 0, CvPROTOLEN(cv) + 2);
            ulen = SvCUR(tmpsv);
            SvCUR_set(tmpsv, SvCUR(tmpsv) + 1); /* include null in string */
            sv_catpvn_flags(
                tmpsv, proto, CvPROTOLEN(cv), SV_CATBYTES*!SvUTF8(cv)
            );
            SvTEMP_on(tmpsv); /* Allow theft */
            sv_setsv_nomg((SV *)cv, tmpsv);
            SvTEMP_off(tmpsv);
            SvREFCNT_dec_NN(tmpsv);
            SvLEN_set(cv, SvCUR(cv) + 1);
            SvCUR_set(cv, ulen);
        }
        else {
          sv_setpvn((SV *)cv, name, len);
          SvPOK_off(cv);
          if (is_utf8)
            SvUTF8_on(cv);
          else SvUTF8_off(cv);
        }
        CvAUTOLOAD_on(cv);
    }

    /*
     * Given &FOO::AUTOLOAD, set $FOO::AUTOLOAD to desired function name.
     * The subroutine's original name may not be "AUTOLOAD", so we don't
     * use that, but for lack of anything better we will use the sub's
     * original package to look up $AUTOLOAD.
     */
    varstash = CvNAMED(cv) ? CvSTASH(cv) : GvSTASH(CvGV(cv));
    vargv = *(GV**)hv_fetch(varstash, S_autoload, S_autolen, TRUE);
    ENTER;

    if (!isGV(vargv)) {
        gv_init_pvn(vargv, varstash, S_autoload, S_autolen, 0);
#ifdef PERL_DONT_CREATE_GVSV
        GvSV(vargv) = newSV_type(SVt_NULL);
#endif
    }
    LEAVE;
    varsv = GvSVn(vargv);
    SvTAINTED_off(varsv); /* previous $AUTOLOAD taint is obsolete */
    /* XXX: this process is not careful to avoid extra magic gets and sets; tied $AUTOLOAD will get noise */
    sv_setsv(varsv, packname);
    sv_catpvs(varsv, "::");
    /* Ensure SvSETMAGIC() is called if necessary. In particular, to clear
       tainting if $FOO::AUTOLOAD was previously tainted, but is not now.  */
    sv_catpvn_flags(
        varsv, name, len,
        SV_SMAGIC|(is_utf8 ? SV_CATUTF8 : SV_CATBYTES)
    );
    if (is_utf8)
        SvUTF8_on(varsv);
    return gv;
}


/* require_tie_mod() internal routine for requiring a module
 * that implements the logic of automatic ties like %! and %-
 * It loads the module and then calls the _tie_it subroutine
 * with the passed gv as an argument.
 *
 * The "gv" parameter should be the glob.
 * "varname" holds the 1-char name of the var, used for error messages.
 * "namesv" holds the module name. Its refcount will be decremented.
 * "flags": if flag & 1 then save the scalar before loading.
 * For the protection of $! to work (it is set by this routine)
 * the sv slot must already be magicalized.
 */
STATIC void
S_require_tie_mod(pTHX_ GV *gv, const char varname, const char * name,
                        STRLEN len, const U32 flags)
{
    const SV * const target = varname == '[' ? GvSV(gv) : (SV *)GvHV(gv);

    PERL_ARGS_ASSERT_REQUIRE_TIE_MOD;

    /* If it is not tied */
    if (!target || !SvRMAGICAL(target)
     || !mg_find(target,
                 varname == '[' ? PERL_MAGIC_tiedscalar : PERL_MAGIC_tied))
    {
      HV *stash;
      GV **gvp;
      dSP;

      PUSHSTACKi(PERLSI_MAGIC);
      ENTER;

#define GET_HV_FETCH_TIE_FUNC				 \
    (  (gvp = (GV **)hv_fetchs(stash, "_tie_it", 0))	  \
    && *gvp						   \
    && (  (isGV(*gvp) && GvCV(*gvp))			    \
       || (SvROK(*gvp) && SvTYPE(SvRV(*gvp)) == SVt_PVCV)  ) \
    )

      /* Load the module if it is not loaded.  */
      if (!(stash = gv_stashpvn(name, len, 0))
       || ! GET_HV_FETCH_TIE_FUNC)
      {
        SV * const module = newSVpvn(name, len);
        const char type = varname == '[' ? '$' : '%';
        if ( flags & 1 )
            save_scalar(gv);
        Perl_load_module(aTHX_ PERL_LOADMOD_NOIMPORT, module, NULL);
        assert(sp == PL_stack_sp);
        stash = gv_stashpvn(name, len, 0);
        if (!stash)
            Perl_croak(aTHX_ "panic: Can't use %c%c because %s is not available",
                    type, varname, name);
        else if (! GET_HV_FETCH_TIE_FUNC)
            Perl_croak(aTHX_ "panic: Can't use %c%c because %s does not define _tie_it",
                    type, varname, name);
      }
      /* Now call the tie function.  It should be in *gvp.  */
      assert(gvp); assert(*gvp);
      PUSHMARK(SP);
      XPUSHs((SV *)gv);
      PUTBACK;
      call_sv((SV *)*gvp, G_VOID|G_DISCARD);
      LEAVE;
      POPSTACK;
    }
}

/* add a require_tie_mod_s - the _s suffix is similar to pvs type suffixes,
 * IOW it means we do STR_WITH_LEN() ourselves and the user should pass in
 * a true string WITHOUT a len.
 */
#define require_tie_mod_s(gv, varname, name, flags) \
    S_require_tie_mod(aTHX_ gv, varname, STR_WITH_LEN(name), flags)

/*
=for apidoc gv_stashpv

Returns a pointer to the stash for a specified package.  Uses C<strlen> to
determine the length of C<name>, then calls C<gv_stashpvn()>.

=cut
*/

HV*
Perl_gv_stashpv(pTHX_ const char *name, I32 create)
{
    PERL_ARGS_ASSERT_GV_STASHPV;
    return gv_stashpvn(name, strlen(name), create);
}

/*
=for apidoc gv_stashpvn

Returns a pointer to the stash for a specified package.  The C<namelen>
parameter indicates the length of the C<name>, in bytes.  C<flags> is passed
to C<gv_fetchpvn_flags()>, so if set to C<GV_ADD> then the package will be
created if it does not already exist.  If the package does not exist and
C<flags> is 0 (or any other setting that does not create packages) then C<NULL>
is returned.

Flags may be one of:

 GV_ADD           Create and initialize the package if doesn't
                  already exist
 GV_NOADD_NOINIT  Don't create the package,
 GV_ADDMG         GV_ADD iff the GV is magical
 GV_NOINIT        GV_ADD, but don't initialize
 GV_NOEXPAND      Don't expand SvOK() entries to PVGV
 SVf_UTF8         The name is in UTF-8

The most important of which are probably C<GV_ADD> and C<SVf_UTF8>.

Note, use of C<gv_stashsv> instead of C<gv_stashpvn> where possible is strongly
recommended for performance reasons.

=for apidoc Amnh||GV_ADD
=for apidoc Amnh||GV_NOADD_NOINIT
=for apidoc Amnh||GV_NOINIT
=for apidoc Amnh||GV_NOEXPAND
=for apidoc Amnh||GV_ADDMG
=for apidoc Amnh||SVf_UTF8

=cut
*/

/*
gv_stashpvn_internal

Perform the internal bits of gv_stashsvpvn_cached. You could think of this
as being one half of the logic. Not to be called except from gv_stashsvpvn_cached().

*/

PERL_STATIC_INLINE HV*
S_gv_stashpvn_internal(pTHX_ const char *name, U32 namelen, I32 flags)
{
    char smallbuf[128];
    char *tmpbuf;
    HV *stash;
    GV *tmpgv;
    U32 tmplen = namelen + 2;

    PERL_ARGS_ASSERT_GV_STASHPVN_INTERNAL;

    if (tmplen <= sizeof smallbuf)
        tmpbuf = smallbuf;
    else
        Newx(tmpbuf, tmplen, char);
    Copy(name, tmpbuf, namelen, char);
    tmpbuf[namelen]   = ':';
    tmpbuf[namelen+1] = ':';
    tmpgv = gv_fetchpvn_flags(tmpbuf, tmplen, flags, SVt_PVHV);
    if (tmpbuf != smallbuf)
        Safefree(tmpbuf);
    if (!tmpgv || !isGV_with_GP(tmpgv))
        return NULL;
    stash = GvHV(tmpgv);
    if (!(flags & ~GV_NOADD_MASK) && !stash) return NULL;
    assert(stash);
    if (!HvHasNAME(stash)) {
        hv_name_set(stash, name, namelen, flags & SVf_UTF8 ? SVf_UTF8 : 0 );

        /* FIXME: This is a repeat of logic in gv_fetchpvn_flags */
        /* If the containing stash has multiple effective
           names, see that this one gets them, too. */
        if (HvAUX(GvSTASH(tmpgv))->xhv_name_count)
            mro_package_moved(stash, NULL, tmpgv, 1);
    }
    return stash;
}

/*
=for apidoc gv_stashsvpvn_cached

Returns a pointer to the stash for a specified package, possibly
cached.  Implements both L<perlapi/C<gv_stashpvn>> and
L<perlapi/C<gv_stashsv>>.

Requires one of either C<namesv> or C<namepv> to be non-null.

If the flag C<GV_CACHE_ONLY> is set, return the stash only if found in the
cache; see L<perlapi/C<gv_stashpvn>> for details on the other C<flags>.

Note it is strongly preferred for C<namesv> to be non-null, for performance
reasons.

=for apidoc Emnh||GV_CACHE_ONLY

=cut
*/

#define PERL_ARGS_ASSERT_GV_STASHSVPVN_CACHED \
    assert(namesv || name)

HV*
Perl_gv_stashsvpvn_cached(pTHX_ SV *namesv, const char *name, U32 namelen, I32 flags)
{
    HV* stash;
    HE* he;

    PERL_ARGS_ASSERT_GV_STASHSVPVN_CACHED;

    he = (HE *)hv_common(
        PL_stashcache, namesv, name, namelen,
        (flags & SVf_UTF8) ? HVhek_UTF8 : 0, 0, NULL, 0
    );

    if (he) {
        SV *sv = HeVAL(he);
        HV *hv;
        assert(SvIOK(sv));
        hv = INT2PTR(HV*, SvIVX(sv));
        assert(SvTYPE(hv) == SVt_PVHV);
        return hv;
    }
    else if (flags & GV_CACHE_ONLY) return NULL;

    if (namesv) {
        if (SvOK(namesv)) { /* prevent double uninit warning */
            STRLEN len;
            name = SvPV_const(namesv, len);
            namelen = len;
            flags |= SvUTF8(namesv);
        } else {
            name = ""; namelen = 0;
        }
    }
    stash = gv_stashpvn_internal(name, namelen, flags);

    if (stash && namelen) {
        SV* const ref = newSViv(PTR2IV(stash));
        (void)hv_store(PL_stashcache, name,
            (flags & SVf_UTF8) ? -(I32)namelen : (I32)namelen, ref, 0);
    }

    return stash;
}

HV*
Perl_gv_stashpvn(pTHX_ const char *name, U32 namelen, I32 flags)
{
    PERL_ARGS_ASSERT_GV_STASHPVN;
    return gv_stashsvpvn_cached(NULL, name, namelen, flags);
}

/*
=for apidoc gv_stashsv

Returns a pointer to the stash for a specified package.  See
C<L</gv_stashpvn>>.

Note this interface is strongly preferred over C<gv_stashpvn> for performance
reasons.

=cut
*/

HV*
Perl_gv_stashsv(pTHX_ SV *sv, I32 flags)
{
    PERL_ARGS_ASSERT_GV_STASHSV;
    return gv_stashsvpvn_cached(sv, NULL, 0, flags);
}
GV *
Perl_gv_fetchpv(pTHX_ const char *nambeg, I32 flags, const svtype sv_type) {
    PERL_ARGS_ASSERT_GV_FETCHPV;
    return gv_fetchpvn_flags(nambeg, strlen(nambeg), flags, sv_type);
}

GV *
Perl_gv_fetchsv(pTHX_ SV *name, I32 flags, const svtype sv_type) {
    STRLEN len;
    const char * const nambeg =
       SvPV_flags_const(name, len, flags & GV_NO_SVGMAGIC ? 0 : SV_GMAGIC);
    PERL_ARGS_ASSERT_GV_FETCHSV;
    return gv_fetchpvn_flags(nambeg, len, flags | SvUTF8(name), sv_type);
}

PERL_STATIC_INLINE void
S_gv_magicalize_isa(pTHX_ GV *gv)
{
    AV* av;

    PERL_ARGS_ASSERT_GV_MAGICALIZE_ISA;

    av = GvAVn(gv);
    GvMULTI_on(gv);
    sv_magic(MUTABLE_SV(av), MUTABLE_SV(gv), PERL_MAGIC_isa,
             NULL, 0);

    if(HvSTASH_IS_CLASS(GvSTASH(gv))) {
        /* Don't permit modification of @ISA outside of the class management
         * code. This is temporarily undone by class.c when fiddling with the
         * array, so it knows it can be done safely.
         */
        SvREADONLY_on((SV *)av);
    }
}

/* This function grabs name and tries to split a stash and glob
 * from its contents. TODO better description, comments
 *
 * If the function returns TRUE and 'name == name_end', then
 * 'gv' can be directly returned to the caller of gv_fetchpvn_flags
 */
PERL_STATIC_INLINE bool
S_parse_gv_stash_name(pTHX_ HV **stash, GV **gv, const char **name,
               STRLEN *len, const char *nambeg, STRLEN full_len,
               const U32 is_utf8, const I32 add)
{
    char *tmpfullbuf = NULL; /* only malloc one big chunk of memory when the smallbuff is not large enough */
    const char *name_cursor;
    const char *const name_end = nambeg + full_len;
    const char *const name_em1 = name_end - 1;
    char smallbuf[64]; /* small buffer to avoid a malloc when possible */

    PERL_ARGS_ASSERT_PARSE_GV_STASH_NAME;

    if (   full_len > 2
        && **name == '*'
        && isIDFIRST_lazy_if_safe(*name + 1, name_end, is_utf8))
    {
        /* accidental stringify on a GV? */
        (*name)++;
    }

    for (name_cursor = *name; name_cursor < name_end; name_cursor++) {
        if (name_cursor < name_em1 &&
            ((*name_cursor == ':' && name_cursor[1] == ':')
           || *name_cursor == '\''))
        {
            if (!*stash)
                *stash = PL_defstash;
            if (!*stash || !SvREFCNT(*stash)) /* symbol table under destruction */
                goto notok;

            *len = name_cursor - *name;
            if (name_cursor > nambeg) { /* Skip for initial :: or ' */
                const char *key;
                GV**gvp;
                if (*name_cursor == ':') {
                    key = *name;
                    *len += 2;
                }
                else { /* using ' for package separator */
                    /* use our pre-allocated buffer when possible to save a malloc */
                    char *tmpbuf;
                    if ( *len+2 <= sizeof smallbuf)
                        tmpbuf = smallbuf;
                    else {
                        /* only malloc once if needed */
                        if (tmpfullbuf == NULL) /* only malloc&free once, a little more than needed */
                            Newx(tmpfullbuf, full_len+2, char);
                        tmpbuf = tmpfullbuf;
                    }
                    Copy(*name, tmpbuf, *len, char);
                    tmpbuf[(*len)++] = ':';
                    tmpbuf[(*len)++] = ':';
                    key = tmpbuf;
                }
                gvp = (GV**)hv_fetch(*stash, key, is_utf8 ? -((I32)*len) : (I32)*len, add);
                *gv = gvp ? *gvp : NULL;
                if (!*gv || *gv == (const GV *)&PL_sv_undef) {
                    goto notok;
                }
                /* here we know that *gv && *gv != &PL_sv_undef */
                if (SvTYPE(*gv) != SVt_PVGV)
                    gv_init_pvn(*gv, *stash, key, *len, (add & GV_ADDMULTI)|is_utf8);
                else
                    GvMULTI_on(*gv);

                if (!(*stash = GvHV(*gv))) {
                    *stash = GvHV(*gv) = newHV();
                    if (!HvHasNAME(*stash)) {
                        if (GvSTASH(*gv) == PL_defstash && *len == 6
                            && strBEGINs(*name, "CORE"))
                            hv_name_sets(*stash, "CORE", 0);
                        else
                            hv_name_set(
                                *stash, nambeg, name_cursor-nambeg, is_utf8
                            );
                    /* If the containing stash has multiple effective
                    names, see that this one gets them, too. */
                    if (HvAUX(GvSTASH(*gv))->xhv_name_count)
                        mro_package_moved(*stash, NULL, *gv, 1);
                    }
                }
                else if (!HvHasNAME(*stash))
                    hv_name_set(*stash, nambeg, name_cursor - nambeg, is_utf8);
            }

            if (*name_cursor == ':')
                name_cursor++;
            *name = name_cursor+1;
            if (*name == name_end) {
                if (!*gv) {
                    *gv = MUTABLE_GV(*hv_fetchs(PL_defstash, "main::", TRUE));
                    if (SvTYPE(*gv) != SVt_PVGV) {
                        gv_init_pvn(*gv, PL_defstash, "main::", 6,
                                    GV_ADDMULTI);
                        GvHV(*gv) =
                            MUTABLE_HV(SvREFCNT_inc_simple(PL_defstash));
                    }
                }
                goto ok;
            }
        }
    }
    *len = name_cursor - *name;
  ok:
    Safefree(tmpfullbuf); /* free our tmpfullbuf if it was used */
    return TRUE;
  notok:
    Safefree(tmpfullbuf); /* free our tmpfullbuf if it was used */
    return FALSE;
}


/* Checks if an unqualified name is in the main stash */
PERL_STATIC_INLINE bool
S_gv_is_in_main(pTHX_ const char *name, STRLEN len, const U32 is_utf8)
{
    PERL_ARGS_ASSERT_GV_IS_IN_MAIN;

    /* If it's an alphanumeric variable */
    if ( len && isIDFIRST_lazy_if_safe(name, name + len, is_utf8) ) {
        /* Some "normal" variables are always in main::,
         * like INC or STDOUT.
         */
        switch (len) {
            case 1:
            if (*name == '_')
                return TRUE;
            break;
            case 3:
            if ((name[0] == 'I' && name[1] == 'N' && name[2] == 'C')
                || (name[0] == 'E' && name[1] == 'N' && name[2] == 'V')
                || (name[0] == 'S' && name[1] == 'I' && name[2] == 'G'))
                return TRUE;
            break;
            case 4:
            if (name[0] == 'A' && name[1] == 'R' && name[2] == 'G'
                && name[3] == 'V')
                return TRUE;
            break;
            case 5:
            if (name[0] == 'S' && name[1] == 'T' && name[2] == 'D'
                && name[3] == 'I' && name[4] == 'N')
                return TRUE;
            break;
            case 6:
            if ((name[0] == 'S' && name[1] == 'T' && name[2] == 'D')
                &&((name[3] == 'O' && name[4] == 'U' && name[5] == 'T')
                    ||(name[3] == 'E' && name[4] == 'R' && name[5] == 'R')))
                return TRUE;
            break;
            case 7:
            if (name[0] == 'A' && name[1] == 'R' && name[2] == 'G'
                && name[3] == 'V' && name[4] == 'O' && name[5] == 'U'
                && name[6] == 'T')
                return TRUE;
            break;
        }
    }
    /* *{""}, or a special variable like $@ */
    else
        return TRUE;

    return FALSE;
}


/* This function is called if parse_gv_stash_name() failed to
 * find a stash, or if GV_NOTQUAL or an empty name was passed
 * to gv_fetchpvn_flags.
 *
 * It returns FALSE if the default stash can't be found nor created,
 * which might happen during global destruction.
 */
PERL_STATIC_INLINE bool
S_find_default_stash(pTHX_ HV **stash, const char *name, STRLEN len,
               const U32 is_utf8, const I32 add,
               const svtype sv_type)
{
    PERL_ARGS_ASSERT_FIND_DEFAULT_STASH;

    /* No stash in name, so see how we can default */

    if ( gv_is_in_main(name, len, is_utf8) ) {
        *stash = PL_defstash;
    }
    else {
        if (IN_PERL_COMPILETIME) {
            *stash = PL_curstash;
            if (add && (PL_hints & HINT_STRICT_VARS) &&
                sv_type != SVt_PVCV &&
                sv_type != SVt_PVGV &&
                sv_type != SVt_PVFM &&
                sv_type != SVt_PVIO &&
                !(len == 1 && sv_type == SVt_PV &&
                (*name == 'a' || *name == 'b')) )
            {
                GV**gvp = (GV**)hv_fetch(*stash,name,is_utf8 ? -(I32)len : (I32)len,0);
                if (!gvp || *gvp == (const GV *)&PL_sv_undef ||
                    SvTYPE(*gvp) != SVt_PVGV)
                {
                    *stash = NULL;
                }
                else if ((sv_type == SVt_PV   && !GvIMPORTED_SV(*gvp)) ||
                         (sv_type == SVt_PVAV && !GvIMPORTED_AV(*gvp)) ||
                         (sv_type == SVt_PVHV && !GvIMPORTED_HV(*gvp)) )
                {
                    /* diag_listed_as: Variable "%s" is not imported%s */
                    Perl_ck_warner_d(
                        aTHX_ packWARN(WARN_MISC),
                        "Variable \"%c%" UTF8f "\" is not imported",
                        sv_type == SVt_PVAV ? '@' :
                        sv_type == SVt_PVHV ? '%' : '$',
                        UTF8fARG(is_utf8, len, name));
                    if (GvCVu(*gvp))
                        Perl_ck_warner_d(
                            aTHX_ packWARN(WARN_MISC),
                            "\t(Did you mean &%" UTF8f " instead?)\n",
                            UTF8fARG(is_utf8, len, name)
                        );
                    *stash = NULL;
                }
            }
        }
        else {
            /* Use the current op's stash */
            *stash = CopSTASH(PL_curcop);
        }
    }

    if (!*stash) {
        if (add && !PL_in_clean_all) {
            GV *gv;
            qerror(Perl_mess(aTHX_
                 "Global symbol \"%s%" UTF8f
                 "\" requires explicit package name (did you forget to "
                 "declare \"my %s%" UTF8f "\"?)",
                 (sv_type == SVt_PV ? "$"
                  : sv_type == SVt_PVAV ? "@"
                  : sv_type == SVt_PVHV ? "%"
                  : ""), UTF8fARG(is_utf8, len, name),
                 (sv_type == SVt_PV ? "$"
                  : sv_type == SVt_PVAV ? "@"
                  : sv_type == SVt_PVHV ? "%"
                  : ""), UTF8fARG(is_utf8, len, name)));
            /* To maintain the output of errors after the strict exception
             * above, and to keep compat with older releases, rather than
             * placing the variables in the pad, we place
             * them in the <none>:: stash.
             */
            gv = gv_fetchpvs("<none>::", GV_ADDMULTI, SVt_PVHV);
            if (!gv) {
                /* symbol table under destruction */
                return FALSE;
            }
            *stash = GvHV(gv);
        }
        else
            return FALSE;
    }

    if (!SvREFCNT(*stash))   /* symbol table under destruction */
        return FALSE;

    return TRUE;
}

/* gv_magicalize only turns on the SVf_READONLY flag, not SVf_PROTECT.  So
   redefine SvREADONLY_on for that purpose.  We don’t use it later on in
   this file.  */
#undef SvREADONLY_on
#define SvREADONLY_on(sv) (SvFLAGS(sv) |= SVf_READONLY)

/* gv_magicalize() is called by gv_fetchpvn_flags when creating
 * a new GV.
 * Note that it does not insert the GV into the stash prior to
 * magicalization, which some variables require need in order
 * to work (like %+, %-, %!), so callers must take care of
 * that.
 *
 * It returns true if the gv did turn out to be magical one; i.e.,
 * if gv_magicalize actually did something.
 */
PERL_STATIC_INLINE bool
S_gv_magicalize(pTHX_ GV *gv, HV *stash, const char *name, STRLEN len,
                      const svtype sv_type)
{
    SSize_t paren;

    PERL_ARGS_ASSERT_GV_MAGICALIZE;

    if (stash != PL_defstash) { /* not the main stash */
        /* We only have to check for a few names here: a, b, EXPORT, ISA
           and VERSION. All the others apply only to the main stash or to
           CORE (which is checked right after this). */
        if (len) {
            switch (*name) {
            case 'E':
                if (
                    len >= 6 && name[1] == 'X' &&
                    (memEQs(name, len, "EXPORT")
                    ||memEQs(name, len, "EXPORT_OK")
                    ||memEQs(name, len, "EXPORT_FAIL")
                    ||memEQs(name, len, "EXPORT_TAGS"))
                )
                    GvMULTI_on(gv);
                break;
            case 'I':
                if (memEQs(name, len, "ISA"))
                    gv_magicalize_isa(gv);
                break;
            case 'V':
                if (memEQs(name, len, "VERSION"))
                    GvMULTI_on(gv);
                break;
            case 'a':
                if (stash == PL_debstash && memEQs(name, len, "args")) {
                    GvMULTI_on(gv_AVadd(gv));
                    break;
                }
                /* FALLTHROUGH */
            case 'b':
                if (len == 1 && sv_type == SVt_PV)
                    GvMULTI_on(gv);
                /* FALLTHROUGH */
            default:
                goto try_core;
            }
            goto ret;
        }
      try_core:
        if (len > 1 /* shortest is uc */ && HvNAMELEN_get(stash) == 4) {
          /* Avoid null warning: */
          const char * const stashname = HvNAME(stash); assert(stashname);
          if (strBEGINs(stashname, "CORE"))
            S_maybe_add_coresub(aTHX_ 0, gv, name, len);
        }
    }
    else if (len > 1) {
#ifndef EBCDIC
        if (*name > 'V' ) {
            NOOP;
            /* Nothing else to do.
               The compiler will probably turn the switch statement into a
               branch table. Make sure we avoid even that small overhead for
               the common case of lower case variable names.  (On EBCDIC
               platforms, we can't just do:
                 if (NATIVE_TO_ASCII(*name) > NATIVE_TO_ASCII('V') ) {
               because cases like '\027' in the switch statement below are
               C1 (non-ASCII) controls on those platforms, so the remapping
               would make them larger than 'V')
             */
        } else
#endif
        {
            switch (*name) {
            case 'A':
                if (memEQs(name, len, "ARGV")) {
                    IoFLAGS(GvIOn(gv)) |= IOf_ARGV|IOf_START;
                }
                else if (memEQs(name, len, "ARGVOUT")) {
                    GvMULTI_on(gv);
                }
                break;
            case 'E':
                if (
                    len >= 6 && name[1] == 'X' &&
                    (memEQs(name, len, "EXPORT")
                    ||memEQs(name, len, "EXPORT_OK")
                    ||memEQs(name, len, "EXPORT_FAIL")
                    ||memEQs(name, len, "EXPORT_TAGS"))
                )
                    GvMULTI_on(gv);
                break;
            case 'I':
                if (memEQs(name, len, "ISA")) {
                    gv_magicalize_isa(gv);
                }
                break;
            case 'S':
                if (memEQs(name, len, "SIG")) {
                    HV *hv;
                    I32 i;
                    if (!PL_psig_name) {
                        Newxz(PL_psig_name, 2 * SIG_SIZE, SV*);
                        Newxz(PL_psig_pend, SIG_SIZE, int);
                        PL_psig_ptr = PL_psig_name + SIG_SIZE;
                    } else {
                        /* I think that the only way to get here is to re-use an
                           embedded perl interpreter, where the previous
                           use didn't clean up fully because
                           PL_perl_destruct_level was 0. I'm not sure that we
                           "support" that, in that I suspect in that scenario
                           there are sufficient other garbage values left in the
                           interpreter structure that something else will crash
                           before we get here. I suspect that this is one of
                           those "doctor, it hurts when I do this" bugs.  */
                        Zero(PL_psig_name, 2 * SIG_SIZE, SV*);
                        Zero(PL_psig_pend, SIG_SIZE, int);
                    }
                    GvMULTI_on(gv);
                    hv = GvHVn(gv);
                    hv_magic(hv, NULL, PERL_MAGIC_sig);
                    for (i = 1; i < SIG_SIZE; i++) {
                        SV * const * const init = hv_fetch(hv, PL_sig_name[i], strlen(PL_sig_name[i]), 1);
                        if (init)
                            sv_setsv(*init, &PL_sv_undef);
                    }
                }
                break;
            case 'V':
                if (memEQs(name, len, "VERSION"))
                    GvMULTI_on(gv);
                break;
            case '\003':        /* $^CHILD_ERROR_NATIVE */
                if (memEQs(name, len, "\003HILD_ERROR_NATIVE"))
                    goto magicalize;
                                /* @{^CAPTURE} %{^CAPTURE} */
                if (memEQs(name, len, "\003APTURE")) {
                    AV* const av = GvAVn(gv);
                    const Size_t n = *name;

                    sv_magic(MUTABLE_SV(av), (SV*)n, PERL_MAGIC_regdata, NULL, 0);
                    SvREADONLY_on(av);

                    require_tie_mod_s(gv, '+', "Tie::Hash::NamedCapture",0);

                } else          /* %{^CAPTURE_ALL} */
                if (memEQs(name, len, "\003APTURE_ALL")) {
                    require_tie_mod_s(gv, '-', "Tie::Hash::NamedCapture",0);
                }
                break;
            case '\005':        /* ${^ENCODING} */
                if (memEQs(name, len, "\005NCODING"))
                    goto magicalize;
                break;
            case '\007':        /* ${^GLOBAL_PHASE} */
                if (memEQs(name, len, "\007LOBAL_PHASE"))
                    goto ro_magicalize;
                break;
            case '\010':        /* %{^HOOK} */
                if (memEQs(name, len, "\010OOK")) {
                    GvMULTI_on(gv);
                    HV *hv = GvHVn(gv);
                    hv_magic(hv, NULL, PERL_MAGIC_hook);
                }
                break;
            case '\014':
                if ( memEQs(name, len, "\014AST_FH") ||               /* ${^LAST_FH} */
                     memEQs(name, len, "\014AST_SUCCESSFUL_PATTERN")) /* ${^LAST_SUCCESSFUL_PATTERN} */
                    goto ro_magicalize;
                break;
            case '\015':        /* ${^MATCH} */
                if (memEQs(name, len, "\015ATCH")) {
                    paren = RX_BUFF_IDX_CARET_FULLMATCH;
                    goto storeparen;
                }
                break;
            case '\017':        /* ${^OPEN} */
                if (memEQs(name, len, "\017PEN"))
                    goto magicalize;
                break;
            case '\020':        /* ${^PREMATCH}  ${^POSTMATCH} */
                if (memEQs(name, len, "\020REMATCH")) {
                    paren = RX_BUFF_IDX_CARET_PREMATCH;
                    goto storeparen;
                }
                if (memEQs(name, len, "\020OSTMATCH")) {
                    paren = RX_BUFF_IDX_CARET_POSTMATCH;
                    goto storeparen;
                }
                break;
            case '\023':
                if (memEQs(name, len, "\023AFE_LOCALES"))
                    goto ro_magicalize;
                break;
            case '\024':	/* ${^TAINT} */
                if (memEQs(name, len, "\024AINT"))
                    goto ro_magicalize;
                break;
            case '\025':	/* ${^UNICODE}, ${^UTF8LOCALE} */
                if (memEQs(name, len, "\025NICODE"))
                    goto ro_magicalize;
                if (memEQs(name, len, "\025TF8LOCALE"))
                    goto ro_magicalize;
                if (memEQs(name, len, "\025TF8CACHE"))
                    goto magicalize;
                break;
            case '\027':	/* $^WARNING_BITS */
                if (memEQs(name, len, "\027ARNING_BITS"))
                    goto magicalize;
#ifdef WIN32
                else if (memEQs(name, len, "\027IN32_SLOPPY_STAT"))
                    goto magicalize;
#endif
                break;
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
            {
                /* Ensures that we have an all-digit variable, ${"1foo"} fails
                   this test  */
                UV uv;
                if (!grok_atoUV(name, &uv, NULL) || uv > I32_MAX)
                    goto ret;
                /* XXX why are we using a SSize_t? */
                paren = (SSize_t)(I32)uv;
                goto storeparen;
            }
            }
        }
    } else {
        /* Names of length 1.  (Or 0. But name is NUL terminated, so that will
           be case '\0' in this switch statement (ie a default case)  */
        switch (*name) {
        case '&':		/* $& */
            paren = RX_BUFF_IDX_FULLMATCH;
            goto sawampersand;
        case '`':		/* $` */
            paren = RX_BUFF_IDX_PREMATCH;
            goto sawampersand;
        case '\'':		/* $' */
            paren = RX_BUFF_IDX_POSTMATCH;
        sawampersand:
#ifdef PERL_SAWAMPERSAND
            if (!(
                sv_type == SVt_PVAV ||
                sv_type == SVt_PVHV ||
                sv_type == SVt_PVCV ||
                sv_type == SVt_PVFM ||
                sv_type == SVt_PVIO
                )) { PL_sawampersand |=
                        (*name == '`')
                            ? SAWAMPERSAND_LEFT
                            : (*name == '&')
                                ? SAWAMPERSAND_MIDDLE
                                : SAWAMPERSAND_RIGHT;
                }
#endif
            goto storeparen;
        case '1':               /* $1 */
        case '2':               /* $2 */
        case '3':               /* $3 */
        case '4':               /* $4 */
        case '5':               /* $5 */
        case '6':               /* $6 */
        case '7':               /* $7 */
        case '8':               /* $8 */
        case '9':               /* $9 */
            paren = *name - '0';

        storeparen:
            /* Flag the capture variables with a NULL mg_ptr
               Use mg_len for the array index to lookup.  */
            sv_magic(GvSVn(gv), MUTABLE_SV(gv), PERL_MAGIC_sv, NULL, paren);
            break;

        case ':':		/* $: */
            sv_setpv(GvSVn(gv),PL_chopset);
            goto magicalize;

        case '?':		/* $? */
#ifdef COMPLEX_STATUS
            SvUPGRADE(GvSVn(gv), SVt_PVLV);
#endif
            goto magicalize;

        case '!':		/* $! */
            GvMULTI_on(gv);
            /* If %! has been used, automatically load Errno.pm. */

            sv_magic(GvSVn(gv), MUTABLE_SV(gv), PERL_MAGIC_sv, name, len);

            /* magicalization must be done before require_tie_mod_s is called */
            if (sv_type == SVt_PVHV || sv_type == SVt_PVGV)
                require_tie_mod_s(gv, '!', "Errno", 1);

            break;
        case '-':		/* $-, %-, @- */
        case '+':		/* $+, %+, @+ */
            GvMULTI_on(gv); /* no used once warnings here */
            {   /* $- $+ */
                sv_magic(GvSVn(gv), MUTABLE_SV(gv), PERL_MAGIC_sv, name, len);
                if (*name == '+')
                    SvREADONLY_on(GvSVn(gv));
            }
            {   /* %- %+ */
                if (sv_type == SVt_PVHV || sv_type == SVt_PVGV)
                    require_tie_mod_s(gv, *name, "Tie::Hash::NamedCapture",0);
            }
            {   /* @- @+ */
                AV* const av = GvAVn(gv);
                const Size_t n = *name;

                sv_magic(MUTABLE_SV(av), (SV*)n, PERL_MAGIC_regdata, NULL, 0);
                SvREADONLY_on(av);
            }
            break;
        case '*':		/* $* */
        case '#':		/* $# */
        if (sv_type == SVt_PV)
            /* diag_listed_as: $* is no longer supported as of Perl 5.30 */
            Perl_croak(aTHX_ "$%c is no longer supported as of Perl 5.30", *name);
        break;
        case '\010':	/* $^H */
            {
                HV *const hv = GvHVn(gv);
                hv_magic(hv, NULL, PERL_MAGIC_hints);
            }
            goto magicalize;
        case '\023':	/* $^S */
        ro_magicalize:
            SvREADONLY_on(GvSVn(gv));
            /* FALLTHROUGH */
        case '0':		/* $0 */
        case '^':		/* $^ */
        case '~':		/* $~ */
        case '=':		/* $= */
        case '%':		/* $% */
        case '.':		/* $. */
        case '(':		/* $( */
        case ')':		/* $) */
        case '<':		/* $< */
        case '>':		/* $> */
        case '\\':		/* $\ */
        case '/':		/* $/ */
        case '|':		/* $| */
        case '$':		/* $$ */
        case '[':		/* $[ */
        case '\001':	/* $^A */
        case '\003':	/* $^C */
        case '\004':	/* $^D */
        case '\005':	/* $^E */
        case '\006':	/* $^F */
        case '\011':	/* $^I, NOT \t in EBCDIC */
        case '\016':	/* $^N */
        case '\017':	/* $^O */
        case '\020':	/* $^P */
        case '\024':	/* $^T */
        case '\027':	/* $^W */
        magicalize:
            sv_magic(GvSVn(gv), MUTABLE_SV(gv), PERL_MAGIC_sv, name, len);
            break;

        case '\014':	/* $^L */
            sv_setpvs(GvSVn(gv),"\f");
            break;
        case ';':		/* $; */
            sv_setpvs(GvSVn(gv),"\034");
            break;
        case ']':		/* $] */
        {
            SV * const sv = GvSV(gv);
            if (!sv_derived_from(PL_patchlevel, "version"))
                upg_version(PL_patchlevel, TRUE);
            GvSV(gv) = vnumify(PL_patchlevel);
            SvREADONLY_on(GvSV(gv));
            SvREFCNT_dec(sv);
        }
        break;
        case '\026':	/* $^V */
        {
            SV * const sv = GvSV(gv);
            GvSV(gv) = new_version(PL_patchlevel);
            SvREADONLY_on(GvSV(gv));
            SvREFCNT_dec(sv);
        }
        break;
        case 'a':
        case 'b':
            if (sv_type == SVt_PV)
                GvMULTI_on(gv);
        }
    }

   ret:
    /* Return true if we actually did something.  */
    return GvAV(gv) || GvHV(gv) || GvIO(gv) || GvCV(gv)
        || ( GvSV(gv) && (
                           SvOK(GvSV(gv)) || SvMAGICAL(GvSV(gv))
                         )
           );
}

/* If we do ever start using this later on in the file, we need to make
   sure we don’t accidentally use the wrong definition.  */
#undef SvREADONLY_on

/* This function is called when the stash already holds the GV of the magic
 * variable we're looking for, but we need to check that it has the correct
 * kind of magic.  For example, if someone first uses $! and then %!, the
 * latter would end up here, and we add the Errno tie to the HASH slot of
 * the *! glob.
 */
PERL_STATIC_INLINE void
S_maybe_multimagic_gv(pTHX_ GV *gv, const char *name, const svtype sv_type)
{
    PERL_ARGS_ASSERT_MAYBE_MULTIMAGIC_GV;

    if (sv_type == SVt_PVHV || sv_type == SVt_PVGV) {
        if (*name == '!')
            require_tie_mod_s(gv, '!', "Errno", 1);
        else if (*name == '-' || *name == '+')
            require_tie_mod_s(gv, *name, "Tie::Hash::NamedCapture", 0);
    } else if (sv_type == SVt_PV) {
        if (*name == '*' || *name == '#') {
            /* diag_listed_as: $* is no longer supported as of Perl 5.30 */
            Perl_croak(aTHX_ "$%c is no longer supported as of Perl 5.30", *name);
        }
    }
    if (sv_type==SVt_PV || sv_type==SVt_PVGV) {
      switch (*name) {
#ifdef PERL_SAWAMPERSAND
      case '`':
          PL_sawampersand |= SAWAMPERSAND_LEFT;
          (void)GvSVn(gv);
          break;
      case '&':
          PL_sawampersand |= SAWAMPERSAND_MIDDLE;
          (void)GvSVn(gv);
          break;
      case '\'':
          PL_sawampersand |= SAWAMPERSAND_RIGHT;
          (void)GvSVn(gv);
          break;
#endif
      }
    }
}

/*
=for apidoc gv_fetchpv
=for apidoc_item |GV *|gv_fetchpvn|const char * nambeg|STRLEN full_len|I32 flags|const svtype sv_type
=for apidoc_item ||gv_fetchpvn_flags
=for apidoc_item |GV *|gv_fetchpvs|"name"|I32 flags|const svtype sv_type
=for apidoc_item ||gv_fetchsv
=for apidoc_item |GV *|gv_fetchsv_nomg|SV *name|I32 flags|const svtype sv_type

These all return the GV of type C<sv_type> whose name is given by the inputs,
or NULL if no GV of that name and type could be found.  See L<perlguts/Stashes
and Globs>.

The only differences are how the input name is specified, and if 'get' magic is
normally used in getting that name.

Don't be fooled by the fact that only one form has C<flags> in its name.  They
all have a C<flags> parameter in fact, and all the flag bits have the same
meanings for all

If any of the flags C<GV_ADD>, C<GV_ADDMG>, C<GV_ADDWARN>, C<GV_ADDMULTI>, or
C<GV_NOINIT> is set, a GV is created if none already exists for the input name
and type.  However, C<GV_ADDMG> will only do the creation for magical GV's.
For all of these flags except C<GV_NOINIT>, C<L</gv_init_pvn>> is called after
the addition.  C<GV_ADDWARN> is used when the caller expects that adding won't
be necessary because the symbol should already exist; but if not, add it
anyway, with a warning that it was unexpectedly absent.  The C<GV_ADDMULTI>
flag means to pretend that the GV has been seen before (I<i.e.>, suppress "Used
once" warnings).

The flag C<GV_NOADD_NOINIT> causes C<L</gv_init_pvn>> not be to called if the
GV existed but isn't PVGV.

If the C<SVf_UTF8> bit is set, the name is treated as being encoded in UTF-8;
otherwise the name won't be considered to be UTF-8 in the C<pv>-named forms,
and the UTF-8ness of the underlying SVs will be used in the C<sv> forms.

If the flag C<GV_NOTQUAL> is set, the caller warrants that the input name is a
plain symbol name, not qualified with a package, otherwise the name is checked
for being a qualified one.

In C<gv_fetchpv>, C<nambeg> is a C string, NUL-terminated with no intermediate
NULs.

In C<gv_fetchpvs>, C<name> is a literal C string, hence is enclosed in
double quotes.

C<gv_fetchpvn> and C<gv_fetchpvn_flags> are identical.  In these, <nambeg> is
a Perl string whose byte length is given by C<full_len>, and may contain
embedded NULs.

In C<gv_fetchsv> and C<gv_fetchsv_nomg>, the name is extracted from the PV of
the input C<name> SV.  The only difference between these two forms is that
'get' magic is normally done on C<name> in C<gv_fetchsv>, and always skipped
with C<gv_fetchsv_nomg>.  Including C<GV_NO_SVGMAGIC> in the C<flags> parameter
to C<gv_fetchsv> makes it behave identically to C<gv_fetchsv_nomg>.

=for apidoc Amnh||GV_ADD
=for apidoc Amnh||GV_ADDMG
=for apidoc Amnh||GV_ADDMULTI
=for apidoc Amnh||GV_ADDWARN
=for apidoc Amnh||GV_NOINIT
=for apidoc Amnh||GV_NOADD_NOINIT
=for apidoc Amnh||GV_NOTQUAL
=for apidoc Amnh||GV_NO_SVGMAGIC
=for apidoc Amnh||SVf_UTF8

=cut
*/

GV *
Perl_gv_fetchpvn_flags(pTHX_ const char *nambeg, STRLEN full_len, I32 flags,
                       const svtype sv_type)
{
    const char *name = nambeg;
    GV *gv = NULL;
    GV**gvp;
    STRLEN len;
    HV *stash = NULL;
    const I32 no_init = flags & (GV_NOADD_NOINIT | GV_NOINIT);
    const I32 no_expand = flags & GV_NOEXPAND;
    const I32 add = flags & ~GV_NOADD_MASK;
    const U32 is_utf8 = flags & SVf_UTF8;
    bool addmg = cBOOL(flags & GV_ADDMG);
    const char *const name_end = nambeg + full_len;
    U32 faking_it;

    PERL_ARGS_ASSERT_GV_FETCHPVN_FLAGS;

     /* If we have GV_NOTQUAL, the caller promised that
      * there is no stash, so we can skip the check.
      * Similarly if full_len is 0, since then we're
      * dealing with something like *{""} or ""->foo()
      */
    if ((flags & GV_NOTQUAL) || !full_len) {
        len = full_len;
    }
    else if (parse_gv_stash_name(&stash, &gv, &name, &len, nambeg, full_len, is_utf8, add)) {
        if (name == name_end) return gv;
    }
    else {
        return NULL;
    }

    if (!stash && !find_default_stash(&stash, name, len, is_utf8, add, sv_type)) {
        return NULL;
    }

    /* By this point we should have a stash and a name */
    gvp = (GV**)hv_fetch(stash,name,is_utf8 ? -(I32)len : (I32)len,add);
    if (!gvp || *gvp == (const GV *)&PL_sv_undef) {
        if (addmg) gv = (GV *)newSV_type(SVt_NULL);     /* tentatively */
        else return NULL;
    }
    else gv = *gvp, addmg = 0;
    /* From this point on, addmg means gv has not been inserted in the
       symtab yet. */

    if (SvTYPE(gv) == SVt_PVGV) {
        /* The GV already exists, so return it, but check if we need to do
         * anything else with it before that.
         */
        if (add) {
            /* This is the heuristic that handles if a variable triggers the
             * 'used only once' warning.  If there's already a GV in the stash
             * with this name, then we assume that the variable has been used
             * before and turn its MULTI flag on.
             * It's a heuristic because it can easily be "tricked", like with
             * BEGIN { $a = 1; $::{foo} = *a }; () = $foo
             * not warning about $main::foo being used just once
             */
            GvMULTI_on(gv);
            gv_init_svtype(gv, sv_type);
            /* You reach this path once the typeglob has already been created,
               either by the same or a different sigil.  If this path didn't
               exist, then (say) referencing $! first, and %! second would
               mean that %! was not handled correctly.  */
            if (len == 1 && stash == PL_defstash) {
                maybe_multimagic_gv(gv, name, sv_type);
            }
            else if (sv_type == SVt_PVAV
                  && memEQs(name, len, "ISA")
                  && (!GvAV(gv) || !SvSMAGICAL(GvAV(gv))))
                gv_magicalize_isa(gv);
        }
        return gv;
    } else if (no_init) {
        assert(!addmg);
        return gv;
    }
    /* If GV_NOEXPAND is true and what we got off the stash is a ref,
     * don't expand it to a glob. This is an optimization so that things
     * copying constants over, like Exporter, don't have to be rewritten
     * to take into account that you can store more than just globs in
     * stashes.
     */
    else if (no_expand && SvROK(gv)) {
        assert(!addmg);
        return gv;
    }

    /* Adding a new symbol.
       Unless of course there was already something non-GV here, in which case
       we want to behave as if there was always a GV here, containing some sort
       of subroutine.
       Otherwise we run the risk of creating things like GvIO, which can cause
       subtle bugs. eg the one that tripped up SQL::Translator  */

    faking_it = SvOK(gv);

    if (add & GV_ADDWARN)
        Perl_ck_warner_d(aTHX_ packWARN(WARN_INTERNAL),
                "Had to create %" UTF8f " unexpectedly",
                 UTF8fARG(is_utf8, name_end-nambeg, nambeg));
    gv_init_pvn(gv, stash, name, len, (add & GV_ADDMULTI)|is_utf8);

    if (   full_len != 0
        && isIDFIRST_lazy_if_safe(name, name + full_len, is_utf8)
        && !ckWARN(WARN_ONCE) )
    {
        GvMULTI_on(gv) ;
    }

    /* set up magic where warranted */
    if ( gv_magicalize(gv, stash, name, len, sv_type) ) {
        /* See 23496c6 */
        if (addmg) {
                /* gv_magicalize magicalised this gv, so we want it
                 * stored in the symtab.
                 * Effectively the caller is asking, ‘Does this gv exist?’
                 * And we respond, ‘Er, *now* it does!’
                 */
                (void)hv_store(stash,name,len,(SV *)gv,0);
        }
    }
    else if (addmg) {
                /* The temporary GV created above */
                SvREFCNT_dec_NN(gv);
                gv = NULL;
    }

    if (gv) gv_init_svtype(gv, faking_it ? SVt_PVCV : sv_type);
    return gv;
}

/*
=for apidoc      gv_efullname3
=for apidoc_item gv_efullname4
=for apidoc_item gv_fullname3
=for apidoc_item gv_fullname4

Place the full package name of C<gv> into C<sv>.  The C<gv_e*> forms return
instead the effective package name (see L</HvENAME>).

If C<prefix> is non-NULL, it is considered to be a C language NUL-terminated
string, and the stored name will be prefaced with it.

The other difference between the functions is that the C<*4> forms have an
extra parameter, C<keepmain>.  If C<true> an initial C<main::> in the name is
kept; if C<false> it is stripped.  With the C<*3> forms, it is always kept.

=cut
*/

void
Perl_gv_fullname4(pTHX_ SV *sv, const GV *gv, const char *prefix, bool keepmain)
{
    const char *name;
    const HV * const hv = GvSTASH(gv);

    PERL_ARGS_ASSERT_GV_FULLNAME4;

    sv_setpv(sv, prefix ? prefix : "");

    if (hv && (name = HvNAME(hv))) {
      const STRLEN len = HvNAMELEN(hv);
      if (keepmain || ! memBEGINs(name, len, "main")) {
        sv_catpvn_flags(sv,name,len,HvNAMEUTF8(hv)?SV_CATUTF8:SV_CATBYTES);
        sv_catpvs(sv,"::");
      }
    }
    else sv_catpvs(sv,"__ANON__::");
    sv_catsv(sv,newSVhek_mortal(GvNAME_HEK(gv)));
}

void
Perl_gv_efullname4(pTHX_ SV *sv, const GV *gv, const char *prefix, bool keepmain)
{
    const GV * const egv = GvEGVx(gv);

    PERL_ARGS_ASSERT_GV_EFULLNAME4;

    gv_fullname4(sv, egv ? egv : gv, prefix, keepmain);
}


/* recursively scan a stash and any nested stashes looking for entries
 * that need the "only used once" warning raised
 */

void
Perl_gv_check(pTHX_ HV *stash)
{
    I32 i;

    PERL_ARGS_ASSERT_GV_CHECK;

    if (!HvHasAUX(stash))
        return;

    assert(HvARRAY(stash));

    /* mark stash is being scanned, to avoid recursing */
    HvAUX(stash)->xhv_aux_flags |= HvAUXf_SCAN_STASH;
    for (i = 0; i <= (I32) HvMAX(stash); i++) {
        const HE *entry;
        for (entry = HvARRAY(stash)[i]; entry; entry = HeNEXT(entry)) {
            GV *gv;
            HV *hv;
            STRLEN keylen = HeKLEN(entry);
            const char * const key = HeKEY(entry);

            if (keylen >= 2 && key[keylen-2] == ':'  && key[keylen-1] == ':' &&
                (gv = MUTABLE_GV(HeVAL(entry))) && isGV(gv) && (hv = GvHV(gv)))
            {
                if (hv != PL_defstash && hv != stash
                    && !(HvHasAUX(hv)
                        && (HvAUX(hv)->xhv_aux_flags & HvAUXf_SCAN_STASH))
                )
                     gv_check(hv);              /* nested package */
            }
            else if (   HeKLEN(entry) != 0
                     && *HeKEY(entry) != '_'
                     && isIDFIRST_lazy_if_safe(HeKEY(entry),
                                               HeKEY(entry) + HeKLEN(entry),
                                               HeUTF8(entry)) )
            {
                const char *file;
                gv = MUTABLE_GV(HeVAL(entry));
                if (SvTYPE(gv) != SVt_PVGV || GvMULTI(gv))
                    continue;
                file = GvFILE(gv);
                assert(PL_curcop == &PL_compiling);
                CopLINE_set(PL_curcop, GvLINE(gv));
#ifdef USE_ITHREADS
                SAVECOPFILE_FREE(PL_curcop);
                CopFILE_set(PL_curcop, (char *)file);	/* set for warning */
#else
                CopFILEGV(PL_curcop)
                    = gv_fetchfile_flags(file, HEK_LEN(GvFILE_HEK(gv)), 0);
#endif
                Perl_warner(aTHX_ packWARN(WARN_ONCE),
                        "Name \"%" HEKf "::%" HEKf
                        "\" used only once: possible typo",
                            HEKfARG(HvNAME_HEK(stash)),
                            HEKfARG(GvNAME_HEK(gv)));
            }
        }
    }
    HvAUX(stash)->xhv_aux_flags &= ~HvAUXf_SCAN_STASH;
}

/*
=for apidoc      newGVgen
=for apidoc_item newGVgen_flags

Create a new, guaranteed to be unique, GV in the package given by the
NUL-terminated C language string C<pack>, and return a pointer to it.

For C<newGVgen> or if C<flags> in C<newGVgen_flags> is 0, C<pack> is to be
considered to be encoded in Latin-1.  The only other legal C<flags> value is
C<SVf_UTF8>, which indicates C<pack> is to be considered to be encoded in
UTF-8.

=cut
*/

GV *
Perl_newGVgen_flags(pTHX_ const char *pack, U32 flags)
{
    PERL_ARGS_ASSERT_NEWGVGEN_FLAGS;
    assert(!(flags & ~SVf_UTF8));

    return gv_fetchpv(Perl_form(aTHX_ "%" UTF8f "::_GEN_%ld",
                                UTF8fARG(flags, strlen(pack), pack),
                                (long)PL_gensym++),
                      GV_ADD, SVt_PVGV);
}

/* hopefully this is only called on local symbol table entries */

GP*
Perl_gp_ref(pTHX_ GP *gp)
{
    if (!gp)
        return NULL;
    gp->gp_refcnt++;
    if (gp->gp_cv) {
        if (gp->gp_cvgen) {
            /* If the GP they asked for a reference to contains
               a method cache entry, clear it first, so that we
               don't infect them with our cached entry */
            SvREFCNT_dec_NN(gp->gp_cv);
            gp->gp_cv = NULL;
            gp->gp_cvgen = 0;
        }
    }
    return gp;
}

void
Perl_gp_free(pTHX_ GV *gv)
{
    GP* gp;
    int attempts = 100;
    bool in_global_destruction = PL_phase == PERL_PHASE_DESTRUCT;

    if (!gv || !isGV_with_GP(gv) || !(gp = GvGP(gv)))
        return;
    if (gp->gp_refcnt == 0) {
        Perl_ck_warner_d(aTHX_ packWARN(WARN_INTERNAL),
                         "Attempt to free unreferenced glob pointers"
                         pTHX__FORMAT pTHX__VALUE);
        return;
    }
    if (gp->gp_refcnt > 1) {
       borrowed:
        if (gp->gp_egv == gv)
            gp->gp_egv = 0;
        gp->gp_refcnt--;
        GvGP_set(gv, NULL);
        return;
    }

    while (1) {
      /* Copy and null out all the glob slots, so destructors do not see
         freed SVs. */
      HEK * const file_hek = gp->gp_file_hek;
      SV  * sv             = gp->gp_sv;
      AV  * av             = gp->gp_av;
      HV  * hv             = gp->gp_hv;
      IO  * io             = gp->gp_io;
      CV  * cv             = gp->gp_cv;
      CV  * form           = gp->gp_form;

      int need = 0;

      gp->gp_file_hek = NULL;
      gp->gp_sv       = NULL;
      gp->gp_av       = NULL;
      gp->gp_hv       = NULL;
      gp->gp_io       = NULL;
      gp->gp_cv       = NULL;
      gp->gp_form     = NULL;

      if (file_hek)
        unshare_hek(file_hek);

      /* Storing the SV on the temps stack (instead of freeing it immediately)
         is an admitted bodge that attempt to compensate for the lack of
         reference counting on the stack. The motivation is that typeglob syntax
         is extremely short hence programs such as '$a += (*a = 2)' are often
         found randomly by researchers running fuzzers. Previously these
         programs would trigger errors, that the researchers would
         (legitimately) report, and then we would spend time figuring out that
         the cause was "stack not reference counted" and so not a dangerous
         security hole. This consumed a lot of researcher time, our time, and
         prevents "interesting" security holes being uncovered.

         Typeglob assignment is rarely used in performance critical production
         code, so we aren't causing much slowdown by doing extra work here.

         In turn, the need to check for SvOBJECT (and references to objects) is
         because we have regression tests that rely on timely destruction that
         happens *within this while loop* to demonstrate behaviour, and
         potentially there is also *working* code in the wild that relies on
         such behaviour.

         And we need to avoid doing this in global destruction else we can end
         up with "Attempt to free temp prematurely ... Unbalanced string table
         refcount".

         Hence the whole thing is a heuristic intended to mitigate against
         simple problems likely found by fuzzers but never written by humans,
         whilst leaving working code unchanged. */
      if (sv) {
          SV *referent;
          if (SvREFCNT(sv) > 1 || SvOBJECT(sv) || UNLIKELY(in_global_destruction)) {
              SvREFCNT_dec_NN(sv);
              sv = NULL;
          } else if (SvROK(sv) && (referent = SvRV(sv))
                     && (SvREFCNT(referent) > 1 || SvOBJECT(referent))) {
              SvREFCNT_dec_NN(sv);
              sv = NULL;
          } else {
              ++need;
          }
      }
      if (av) {
          if (SvREFCNT(av) > 1 || SvOBJECT(av) || UNLIKELY(in_global_destruction)) {
              SvREFCNT_dec_NN(av);
              av = NULL;
          } else {
              ++need;
          }
      }
      /* FIXME - another reference loop GV -> symtab -> GV ?
         Somehow gp->gp_hv can end up pointing at freed garbage.  */
      if (hv && SvTYPE(hv) == SVt_PVHV) {
        const HEK *hvname_hek = HvNAME_HEK(hv);
        if (PL_stashcache && hvname_hek) {
           DEBUG_o(Perl_deb(aTHX_
                          "gp_free clearing PL_stashcache for '%" HEKf "'\n",
                           HEKfARG(hvname_hek)));
           (void)hv_deletehek(PL_stashcache, hvname_hek, G_DISCARD);
        }
        if (SvREFCNT(hv) > 1 || SvOBJECT(hv) || UNLIKELY(in_global_destruction)) {
          SvREFCNT_dec_NN(hv);
          hv = NULL;
        } else {
          ++need;
        }
      }
      if (io && SvREFCNT(io) == 1 && IoIFP(io)
             && (IoTYPE(io) == IoTYPE_WRONLY ||
                 IoTYPE(io) == IoTYPE_RDWR   ||
                 IoTYPE(io) == IoTYPE_APPEND)
             && ckWARN_d(WARN_IO)
             && IoIFP(io) != PerlIO_stdin()
             && IoIFP(io) != PerlIO_stdout()
             && IoIFP(io) != PerlIO_stderr()
             && !(IoFLAGS(io) & IOf_FAKE_DIRP))
        io_close(io, gv, FALSE, TRUE);
      if (io) {
          if (SvREFCNT(io) > 1 || SvOBJECT(io) || UNLIKELY(in_global_destruction)) {
              SvREFCNT_dec_NN(io);
              io = NULL;
          } else {
              ++need;
          }
      }
      if (cv) {
          if (SvREFCNT(cv) > 1 || SvOBJECT(cv) || UNLIKELY(in_global_destruction)) {
              SvREFCNT_dec_NN(cv);
              cv = NULL;
          } else {
              ++need;
          }
      }
      if (form) {
          if (SvREFCNT(form) > 1 || SvOBJECT(form) || UNLIKELY(in_global_destruction)) {
              SvREFCNT_dec_NN(form);
              form = NULL;
          } else {
              ++need;
          }
      }

      if (need) {
          /* We don't strictly need to defer all this to the end, but it's
             easiest to do so. The subtle problems we have are
             1) any of the actions triggered by the various SvREFCNT_dec()s in
                any of the intermediate blocks can cause more items to be added
                to the temps stack. So we can't "cache" its state locally
             2) We'd have to re-check the "extend by 1?" for each time.
                Whereas if we don't NULL out the values that we want to put onto
                the save stack until here, we can do it in one go, with one
                one size check. */

          SSize_t max_ix = PL_tmps_ix + need;

          if (max_ix >= PL_tmps_max) {
              tmps_grow_p(max_ix);
          }

          if (sv) {
              PL_tmps_stack[++PL_tmps_ix] = sv;
          }
          if (av) {
              PL_tmps_stack[++PL_tmps_ix] = (SV *) av;
          }
          if (hv) {
              PL_tmps_stack[++PL_tmps_ix] = (SV *) hv;
          }
          if (io) {
              PL_tmps_stack[++PL_tmps_ix] = (SV *) io;
          }
          if (cv) {
              PL_tmps_stack[++PL_tmps_ix] = (SV *) cv;
          }
          if (form) {
              PL_tmps_stack[++PL_tmps_ix] = (SV *) form;
          }
      }

      /* Possibly reallocated by a destructor */
      gp = GvGP(gv);

      if (!gp->gp_file_hek
       && !gp->gp_sv
       && !gp->gp_av
       && !gp->gp_hv
       && !gp->gp_io
       && !gp->gp_cv
       && !gp->gp_form) break;

      if (--attempts == 0) {
        Perl_die(aTHX_
          "panic: gp_free failed to free glob pointer - "
          "something is repeatedly re-creating entries"
        );
      }
    }

    /* Possibly incremented by a destructor doing glob assignment */
    if (gp->gp_refcnt > 1) goto borrowed;
    Safefree(gp);
    GvGP_set(gv, NULL);
}

int
Perl_magic_freeovrld(pTHX_ SV *sv, MAGIC *mg)
{
    AMT * const amtp = (AMT*)mg->mg_ptr;
    PERL_UNUSED_ARG(sv);

    PERL_ARGS_ASSERT_MAGIC_FREEOVRLD;

    if (amtp && AMT_AMAGIC(amtp)) {
        int i;
        for (i = 1; i < NofAMmeth; i++) {
            CV * const cv = amtp->table[i];
            if (cv) {
                SvREFCNT_dec_NN(MUTABLE_SV(cv));
                amtp->table[i] = NULL;
            }
        }
    }
 return 0;
}

/*
=for apidoc Gv_AMupdate

Recalculates overload magic in the package given by C<stash>.

Returns:

=over

=item 1 on success and there is some overload

=item 0 if there is no overload

=item -1 if some error occurred and it couldn't croak (because C<destructing>
is true).

=back

=cut
*/

int
Perl_Gv_AMupdate(pTHX_ HV *stash, bool destructing)
{
  MAGIC* const mg = mg_find((const SV *)stash, PERL_MAGIC_overload_table);
  AMT amt;
  const struct mro_meta* stash_meta = HvMROMETA(stash);
  U32 newgen;

  PERL_ARGS_ASSERT_GV_AMUPDATE;

  newgen = PL_sub_generation + stash_meta->pkg_gen + stash_meta->cache_gen;
  if (mg) {
      const AMT * const amtp = (AMT*)mg->mg_ptr;
      if (amtp->was_ok_sub == newgen) {
          return AMT_AMAGIC(amtp) ? 1 : 0;
      }
      sv_unmagic(MUTABLE_SV(stash), PERL_MAGIC_overload_table);
  }

  DEBUG_o( Perl_deb(aTHX_ "Recalculating overload magic in package %s\n",HvNAME_get(stash)) );

  Zero(&amt,1,AMT);
  amt.was_ok_sub = newgen;
  amt.fallback = AMGfallNO;
  amt.flags = 0;

  {
    int filled = 0;
    int i;
    bool deref_seen = 0;


    /* Work with "fallback" key, which we assume to be first in PL_AMG_names */

    /* Try to find via inheritance. */
    GV *gv = gv_fetchmeth_pvn(stash, PL_AMG_names[0], 2, -1, 0);
    SV * const sv = gv ? GvSV(gv) : NULL;
    CV* cv;

    if (!gv)
    {
      if (!gv_fetchmeth_pvn(stash, "((", 2, -1, 0))
        goto no_table;
    }
#ifdef PERL_DONT_CREATE_GVSV
    else if (!sv) {
        NOOP;   /* Equivalent to !SvTRUE and !SvOK  */
    }
#endif
    else if (SvTRUE(sv))
        /* don't need to set overloading here because fallback => 1
         * is the default setting for classes without overloading */
        amt.fallback=AMGfallYES;
    else if (SvOK(sv)) {
        amt.fallback=AMGfallNEVER;
        filled = 1;
    }
    else {
        filled = 1;
    }

    assert(HvHasAUX(stash));
    /* initially assume the worst */
    HvAUX(stash)->xhv_aux_flags &= ~HvAUXf_NO_DEREF;

    for (i = 1; i < NofAMmeth; i++) {
        const char * const cooky = PL_AMG_names[i];
        /* Human-readable form, for debugging: */
        const char * const cp = AMG_id2name(i);
        const STRLEN l = PL_AMG_namelens[i];

        DEBUG_o( Perl_deb(aTHX_ "Checking overloading of \"%s\" in package \"%.256s\"\n",
                     cp, HvNAME_get(stash)) );
        /* don't fill the cache while looking up!
           Creation of inheritance stubs in intermediate packages may
           conflict with the logic of runtime method substitution.
           Indeed, for inheritance A -> B -> C, if C overloads "+0",
           then we could have created stubs for "(+0" in A and C too.
           But if B overloads "bool", we may want to use it for
           numifying instead of C's "+0". */
        gv = Perl_gv_fetchmeth_pvn(aTHX_ stash, cooky, l, -1, 0);
        cv = 0;
        if (gv && (cv = GvCV(gv)) && CvHASGV(cv)) {
            const HEK * const gvhek = CvGvNAME_HEK(cv);
            const HEK * const stashek =
                HvNAME_HEK(CvNAMED(cv) ? CvSTASH(cv) : GvSTASH(CvGV(cv)));
            if (memEQs(HEK_KEY(gvhek), HEK_LEN(gvhek), "nil")
             && stashek
             && memEQs(HEK_KEY(stashek), HEK_LEN(stashek), "overload")) {
                /* This is a hack to support autoloading..., while
                   knowing *which* methods were declared as overloaded. */
                /* GvSV contains the name of the method. */
                GV *ngv = NULL;
                SV *gvsv = GvSV(gv);

                DEBUG_o( Perl_deb(aTHX_ "Resolving method \"%" SVf256\
                        "\" for overloaded \"%s\" in package \"%.256s\"\n",
                             (void*)GvSV(gv), cp, HvNAME(stash)) );
                if (!gvsv || !SvPOK(gvsv)
                    || !(ngv = gv_fetchmethod_sv_flags(stash, gvsv, 0)))
                {
                    /* Can be an import stub (created by "can"). */
                    if (destructing) {
                        return -1;
                    }
                    else {
                        const SV * const name = (gvsv && SvPOK(gvsv))
                                                    ? gvsv
                                                    : newSVpvs_flags("???", SVs_TEMP);
                        /* diag_listed_as: Can't resolve method "%s" overloading "%s" in package "%s" */
                        Perl_croak(aTHX_ "%s method \"%" SVf256
                                    "\" overloading \"%s\" "\
                                    "in package \"%" HEKf256 "\"",
                                   (GvCVGEN(gv) ? "Stub found while resolving"
                                    : "Can't resolve"),
                                   SVfARG(name), cp,
                                   HEKfARG(
                                        HvNAME_HEK(stash)
                                   ));
                    }
                }
                cv = GvCV(gv = ngv);
            }
            DEBUG_o( Perl_deb(aTHX_ "Overloading \"%s\" in package \"%.256s\" via \"%.256s::%.256s\"\n",
                         cp, HvNAME_get(stash), HvNAME_get(GvSTASH(CvGV(cv))),
                         GvNAME(CvGV(cv))) );
            filled = 1;
        } else if (gv) {		/* Autoloaded... */
            cv = MUTABLE_CV(gv);
            filled = 1;
        }
        amt.table[i]=MUTABLE_CV(SvREFCNT_inc_simple(cv));

        if (gv) {
            switch (i) {
            case to_sv_amg:
            case to_av_amg:
            case to_hv_amg:
            case to_gv_amg:
            case to_cv_amg:
            case nomethod_amg:
                deref_seen = 1;
                break;
            }
        }
    }
    if (!deref_seen)
        /* none of @{} etc overloaded; we can do $obj->[N] quicker.
         * NB - aux var invalid here, HvARRAY() could have been
         * reallocated since it was assigned to */
        HvAUX(stash)->xhv_aux_flags |= HvAUXf_NO_DEREF;

    if (filled) {
      AMT_AMAGIC_on(&amt);
      sv_magic(MUTABLE_SV(stash), 0, PERL_MAGIC_overload_table,
                                                (char*)&amt, sizeof(AMT));
      return TRUE;
    }
  }
  /* Here we have no table: */
 no_table:
  AMT_AMAGIC_off(&amt);
  sv_magic(MUTABLE_SV(stash), 0, PERL_MAGIC_overload_table,
                                                (char*)&amt, sizeof(AMTS));
  return 0;
}

/*
=for apidoc gv_handler

Implements C<StashHANDLER>, which you should use instead

=cut
*/

CV*
Perl_gv_handler(pTHX_ HV *stash, I32 id)
{
    MAGIC *mg;
    AMT *amtp;
    U32 newgen;
    struct mro_meta* stash_meta;

    if (!stash || !HvHasNAME(stash))
        return NULL;

    stash_meta = HvMROMETA(stash);
    newgen = PL_sub_generation + stash_meta->pkg_gen + stash_meta->cache_gen;

    mg = mg_find((const SV *)stash, PERL_MAGIC_overload_table);
    if (!mg) {
      do_update:
        if (Gv_AMupdate(stash, 0) == -1)
            return NULL;
        mg = mg_find((const SV *)stash, PERL_MAGIC_overload_table);
    }
    assert(mg);
    amtp = (AMT*)mg->mg_ptr;
    if ( amtp->was_ok_sub != newgen )
        goto do_update;
    if (AMT_AMAGIC(amtp)) {
        CV * const ret = amtp->table[id];
        if (ret && isGV(ret)) {		/* Autoloading stab */
            /* Passing it through may have resulted in a warning
               "Inherited AUTOLOAD for a non-method deprecated", since
               our caller is going through a function call, not a method call.
               So return the CV for AUTOLOAD, setting $AUTOLOAD. */
            GV * const gv = gv_fetchmethod(stash, PL_AMG_names[id]);

            if (gv && GvCV(gv))
                return GvCV(gv);
        }
        return ret;
    }

    return NULL;
}


/* Implement tryAMAGICun_MG macro.
   Do get magic, then see if the stack arg is overloaded and if so call it.
   Flags:
        AMGf_numeric apply sv_2num to the stack arg.
*/

bool
Perl_try_amagic_un(pTHX_ int method, int flags) {
    dSP;
    SV* tmpsv;
    SV* const arg = TOPs;

    SvGETMAGIC(arg);

    if (SvAMAGIC(arg) && (tmpsv = amagic_call(arg, &PL_sv_undef, method,
                                              AMGf_noright | AMGf_unary
                                            | (flags & AMGf_numarg))))
    {
        /* where the op is of the form:
         *    $lex = $x op $y (where the assign is optimised away)
         * then assign the returned value to targ and return that;
         * otherwise return the value directly
         */
        if (   (PL_opargs[PL_op->op_type] & OA_TARGLEX)
            && (PL_op->op_private & OPpTARGET_MY))
        {
            dTARGET;
            sv_setsv(TARG, tmpsv);
            SETTARG;
        }
        else
            SETs(tmpsv);

        PUTBACK;
        return TRUE;
    }

    if ((flags & AMGf_numeric) && SvROK(arg))
        *sp = sv_2num(arg);
    return FALSE;
}


/*
=for apidoc amagic_applies

Check C<sv> to see if the overloaded (active magic) operation C<method>
applies to it. If the sv is not SvROK or it is not an object then returns
false, otherwise checks if the object is blessed into a class supporting
overloaded operations, and returns true if a call to amagic_call() with
this SV and the given method would trigger an amagic operation, including
via the overload fallback rules or via nomethod. Thus a call like:

    amagic_applies(sv, string_amg, AMG_unary)

would return true for an object with overloading set up in any of the
following ways:

    use overload q("") => sub { ... };
    use overload q(0+) => sub { ... }, fallback => 1;

and could be used to tell if a given object would stringify to something
other than the normal default ref stringification.

Note that the fact that this function returns TRUE does not mean you
can succesfully perform the operation with amagic_call(), for instance
any overloaded method might throw a fatal exception,  however if this
function returns FALSE you can be confident that it will NOT perform
the given overload operation.

C<method> is an integer enum, one of the values found in F<overload.h>,
for instance C<string_amg>.

C<flags> should be set to AMG_unary for unary operations.

=cut
*/
bool
Perl_amagic_applies(pTHX_ SV *sv, int method, int flags)
{
    PERL_ARGS_ASSERT_AMAGIC_APPLIES;
    PERL_UNUSED_VAR(flags);

    assert(method >= 0 && method < NofAMmeth);

    if (!SvAMAGIC(sv))
        return FALSE;

    HV *stash = SvSTASH(SvRV(sv));
    if (!Gv_AMG(stash))
        return FALSE;

    MAGIC *mg = mg_find((const SV *)stash, PERL_MAGIC_overload_table);
    if (!mg)
        return FALSE;

    CV **cvp = NULL;
    AMT *amtp = NULL;
    if (AMT_AMAGIC((AMT *)mg->mg_ptr)) {
        amtp = (AMT *)mg->mg_ptr;
        cvp = amtp->table;
    }
    if (!cvp)
        return FALSE;

    if (cvp[method])
        return TRUE;

    /* Note this logic should be kept in sync with amagic_call() */
    if (amtp->fallback > AMGfallNEVER && flags & AMGf_unary) {
         CV *cv;       /* This makes it easier to kee ... */
         int off,off1; /* ... in sync with amagic_call() */

      /* look for substituted methods */
      /* In all the covered cases we should be called with assign==0. */
         switch (method) {
         case inc_amg:
           if ((cv = cvp[off=add_ass_amg]) || ((cv = cvp[off = add_amg])))
               return TRUE;
           break;
         case dec_amg:
           if((cv = cvp[off = subtr_ass_amg]) || ((cv = cvp[off = subtr_amg])))
               return TRUE;
           break;
         case bool__amg:
           if ((cv = cvp[off=numer_amg]) || (cv = cvp[off=string_amg]))
               return TRUE;
           break;
         case numer_amg:
           if((cv = cvp[off=string_amg]) || (cv = cvp[off=bool__amg]))
               return TRUE;
           break;
         case string_amg:
           if((cv = cvp[off=numer_amg]) || (cv = cvp[off=bool__amg]))
               return TRUE;
           break;
         case not_amg:
           if((cv = cvp[off=bool__amg])
                  || (cv = cvp[off=numer_amg])
                  || (cv = cvp[off=string_amg]))
               return TRUE;
           break;
         case abs_amg:
           if((cvp[off1=lt_amg] || cvp[off1=ncmp_amg])
               && ((cv = cvp[off=neg_amg]) || (cv = cvp[off=subtr_amg])))
               return TRUE;
           break;
         case neg_amg:
           if ((cv = cvp[off=subtr_amg]))
               return TRUE;
           break;
         }
    } else if (((cvp && amtp->fallback > AMGfallNEVER))
               && !(flags & AMGf_unary)) {
                                /* We look for substitution for
                                 * comparison operations and
                                 * concatenation */
      if (method==concat_amg || method==concat_ass_amg
          || method==repeat_amg || method==repeat_ass_amg) {
        return FALSE;            /* Delegate operation to string conversion */
      }
      switch (method) {
         case lt_amg:
         case le_amg:
         case gt_amg:
         case ge_amg:
         case eq_amg:
         case ne_amg:
             if (cvp[ncmp_amg])
                 return TRUE;
             break;
         case slt_amg:
         case sle_amg:
         case sgt_amg:
         case sge_amg:
         case seq_amg:
         case sne_amg:
             if (cvp[scmp_amg])
                 return TRUE;
             break;
      }
    }

    if (cvp[nomethod_amg])
        return TRUE;

    return FALSE;
}


/* Implement tryAMAGICbin_MG macro.
   Do get magic, then see if the two stack args are overloaded and if so
   call it.
   Flags:
        AMGf_assign  op may be called as mutator (eg +=)
        AMGf_numeric apply sv_2num to the stack arg.
*/

bool
Perl_try_amagic_bin(pTHX_ int method, int flags) {
    dSP;
    SV* const left = TOPm1s;
    SV* const right = TOPs;

    SvGETMAGIC(left);
    if (left != right)
        SvGETMAGIC(right);

    if (SvAMAGIC(left) || SvAMAGIC(right)) {
        SV * tmpsv;
        /* STACKED implies mutator variant, e.g. $x += 1 */
        bool mutator = (flags & AMGf_assign) && (PL_op->op_flags & OPf_STACKED);

        tmpsv = amagic_call(left, right, method,
                    (mutator ? AMGf_assign: 0)
                  | (flags & AMGf_numarg));
        if (tmpsv) {
            (void)POPs;
            /* where the op is one of the two forms:
             *    $x op= $y
             *    $lex = $x op $y (where the assign is optimised away)
             * then assign the returned value to targ and return that;
             * otherwise return the value directly
             */
            if (   mutator
                || (   (PL_opargs[PL_op->op_type] & OA_TARGLEX)
                    && (PL_op->op_private & OPpTARGET_MY)))
            {
                dTARG;
                TARG = mutator ? *SP : PAD_SV(PL_op->op_targ);
                sv_setsv(TARG, tmpsv);
                SETTARG;
            }
            else
                SETs(tmpsv);

            PUTBACK;
            return TRUE;
        }
    }

    if(left==right && SvGMAGICAL(left)) {
        SV * const left = sv_newmortal();
        *(sp-1) = left;
        /* Print the uninitialized warning now, so it includes the vari-
           able name. */
        if (!SvOK(right)) {
            if (ckWARN(WARN_UNINITIALIZED)) report_uninit(right);
            sv_setbool(left, FALSE);
        }
        else sv_setsv_flags(left, right, 0);
        SvGETMAGIC(right);
    }
    if (flags & AMGf_numeric) {
        if (SvROK(TOPm1s))
            *(sp-1) = sv_2num(TOPm1s);
        if (SvROK(right))
            *sp     = sv_2num(right);
    }
    return FALSE;
}

/*
=for apidoc amagic_deref_call

Perform C<method> overloading dereferencing on C<ref>, returning the
dereferenced result.  C<method> must be one of the dereference operations given
in F<overload.h>.

If overloading is inactive on C<ref>, returns C<ref> itself.

=cut
*/

SV *
Perl_amagic_deref_call(pTHX_ SV *ref, int method) {
    SV *tmpsv = NULL;
    HV *stash;

    PERL_ARGS_ASSERT_AMAGIC_DEREF_CALL;

    if (!SvAMAGIC(ref))
        return ref;
    /* return quickly if none of the deref ops are overloaded */
    stash = SvSTASH(SvRV(ref));
    assert(HvHasAUX(stash));
    if (HvAUX(stash)->xhv_aux_flags & HvAUXf_NO_DEREF)
        return ref;

    while ((tmpsv = amagic_call(ref, &PL_sv_undef, method,
                                AMGf_noright | AMGf_unary))) {
        if (!SvROK(tmpsv))
            Perl_croak(aTHX_ "Overloaded dereference did not return a reference");
        if (tmpsv == ref || SvRV(tmpsv) == SvRV(ref)) {
            /* Bail out if it returns us the same reference.  */
            return tmpsv;
        }
        ref = tmpsv;
        if (!SvAMAGIC(ref))
            break;
    }
    return tmpsv ? tmpsv : ref;
}

bool
Perl_amagic_is_enabled(pTHX_ int method)
{
      SV *lex_mask = cop_hints_fetch_pvs(PL_curcop, "overloading", 0);

      assert(PL_curcop->cop_hints & HINT_NO_AMAGIC);

      if ( !lex_mask || !SvOK(lex_mask) )
          /* overloading lexically disabled */
          return FALSE;
      else if ( lex_mask && SvPOK(lex_mask) ) {
          /* we have an entry in the hints hash, check if method has been
           * masked by overloading.pm */
          STRLEN len;
          const int offset = method / 8;
          const int bit    = method % 8;
          char *pv = SvPV(lex_mask, len);

          /* Bit set, so this overloading operator is disabled */
          if ( (STRLEN)offset < len && pv[offset] & ( 1 << bit ) )
              return FALSE;
      }
      return TRUE;
}

/*
=for apidoc amagic_call

Perform the overloaded (active magic) operation given by C<method>.
C<method> is one of the values found in F<overload.h>.

C<flags> affects how the operation is performed, as follows:

=over

=item C<AMGf_noleft>

C<left> is not to be used in this operation.

=item C<AMGf_noright>

C<right> is not to be used in this operation.

=item C<AMGf_unary>

The operation is done only on just one operand.

=item C<AMGf_assign>

The operation changes one of the operands, e.g., $x += 1

=back

=cut
*/

SV*
Perl_amagic_call(pTHX_ SV *left, SV *right, int method, int flags)
{
  MAGIC *mg;
  CV *cv=NULL;
  CV **cvp=NULL, **ocvp=NULL;
  AMT *amtp=NULL, *oamtp=NULL;
  int off = 0, off1, lr = 0, notfound = 0;
  int postpr = 0, force_cpy = 0;
  int assign = AMGf_assign & flags;
  const int assignshift = assign ? 1 : 0;
  int use_default_op = 0;
  int force_scalar = 0;
#ifdef DEBUGGING
  int fl=0;
#endif
  HV* stash=NULL;

  PERL_ARGS_ASSERT_AMAGIC_CALL;

  if ( PL_curcop->cop_hints & HINT_NO_AMAGIC ) {
      if (!amagic_is_enabled(method)) return NULL;
  }

  if (!(AMGf_noleft & flags) && SvAMAGIC(left)
      && (stash = SvSTASH(SvRV(left))) && Gv_AMG(stash)
      && (mg = mg_find((const SV *)stash, PERL_MAGIC_overload_table))
      && (ocvp = cvp = (AMT_AMAGIC((AMT*)mg->mg_ptr)
                        ? (oamtp = amtp = (AMT*)mg->mg_ptr)->table
                        : NULL))
      && ((cv = cvp[off=method+assignshift])
          || (assign && amtp->fallback > AMGfallNEVER && /* fallback to
                                                          * usual method */
                  (
#ifdef DEBUGGING
                   fl = 1,
#endif
                   cv = cvp[off=method]))))
  {
    lr = -1;			/* Call method for left argument */
  } else {
    /* Note this logic should be kept in sync with amagic_applies() */
    if (cvp && amtp->fallback > AMGfallNEVER && flags & AMGf_unary) {
      int logic;

      /* look for substituted methods */
      /* In all the covered cases we should be called with assign==0. */
         switch (method) {
         case inc_amg:
           force_cpy = 1;
           if ((cv = cvp[off=add_ass_amg])
               || ((cv = cvp[off = add_amg])
                   && (force_cpy = 0, (postpr = 1)))) {
             right = &PL_sv_yes; lr = -1; assign = 1;
           }
           break;
         case dec_amg:
           force_cpy = 1;
           if ((cv = cvp[off = subtr_ass_amg])
               || ((cv = cvp[off = subtr_amg])
                   && (force_cpy = 0, (postpr=1)))) {
             right = &PL_sv_yes; lr = -1; assign = 1;
           }
           break;
         case bool__amg:
           (void)((cv = cvp[off=numer_amg]) || (cv = cvp[off=string_amg]));
           break;
         case numer_amg:
           (void)((cv = cvp[off=string_amg]) || (cv = cvp[off=bool__amg]));
           break;
         case string_amg:
           (void)((cv = cvp[off=numer_amg]) || (cv = cvp[off=bool__amg]));
           break;
         case not_amg:
           (void)((cv = cvp[off=bool__amg])
                  || (cv = cvp[off=numer_amg])
                  || (cv = cvp[off=string_amg]));
           if (cv)
               postpr = 1;
           break;
         case copy_amg:
           {
             /*
                  * SV* ref causes confusion with the interpreter variable of
                  * the same name
                  */
             SV* const tmpRef=SvRV(left);
             if (!SvROK(tmpRef) && SvTYPE(tmpRef) <= SVt_PVMG) {
                /*
                 * Just to be extra cautious.  Maybe in some
                 * additional cases sv_setsv is safe, too.
                 */
                SV* const newref = newSVsv(tmpRef);
                SvOBJECT_on(newref);
                /* No need to do SvAMAGIC_on here, as SvAMAGIC macros
                   delegate to the stash. */
                SvSTASH_set(newref, MUTABLE_HV(SvREFCNT_inc(SvSTASH(tmpRef))));
                return newref;
             }
           }
           break;
         case abs_amg:
           if ((cvp[off1=lt_amg] || cvp[off1=ncmp_amg])
               && ((cv = cvp[off=neg_amg]) || (cv = cvp[off=subtr_amg]))) {
             SV* const nullsv=&PL_sv_zero;
             if (off1==lt_amg) {
               SV* const lessp = amagic_call(left,nullsv,
                                       lt_amg,AMGf_noright);
               logic = SvTRUE_NN(lessp);
             } else {
               SV* const lessp = amagic_call(left,nullsv,
                                       ncmp_amg,AMGf_noright);
               logic = (SvNV(lessp) < 0);
             }
             if (logic) {
               if (off==subtr_amg) {
                 right = left;
                 left = nullsv;
                 lr = 1;
               }
             } else {
               return left;
             }
           }
           break;
         case neg_amg:
           if ((cv = cvp[off=subtr_amg])) {
             right = left;
             left = &PL_sv_zero;
             lr = 1;
           }
           break;
         case int_amg:
         case iter_amg:			/* XXXX Eventually should do to_gv. */
         case ftest_amg:		/* XXXX Eventually should do to_gv. */
         case regexp_amg:
             /* FAIL safe */
             return NULL;	/* Delegate operation to standard mechanisms. */

         case to_sv_amg:
         case to_av_amg:
         case to_hv_amg:
         case to_gv_amg:
         case to_cv_amg:
             /* FAIL safe */
             return left;	/* Delegate operation to standard mechanisms. */

         default:
           goto not_found;
         }
         if (!cv) goto not_found;
    } else if (!(AMGf_noright & flags) && SvAMAGIC(right)
               && (stash = SvSTASH(SvRV(right))) && Gv_AMG(stash)
               && (mg = mg_find((const SV *)stash, PERL_MAGIC_overload_table))
               && (cvp = (AMT_AMAGIC((AMT*)mg->mg_ptr)
                          ? (amtp = (AMT*)mg->mg_ptr)->table
                          : NULL))
               && (cv = cvp[off=method])) { /* Method for right
                                             * argument found */
      lr=1;
    } else if (((cvp && amtp->fallback > AMGfallNEVER)
                || (ocvp && oamtp->fallback > AMGfallNEVER))
               && !(flags & AMGf_unary)) {
                                /* We look for substitution for
                                 * comparison operations and
                                 * concatenation */
      if (method==concat_amg || method==concat_ass_amg
          || method==repeat_amg || method==repeat_ass_amg) {
        return NULL;		/* Delegate operation to string conversion */
      }
      off = -1;
      switch (method) {
         case lt_amg:
         case le_amg:
         case gt_amg:
         case ge_amg:
         case eq_amg:
         case ne_amg:
             off = ncmp_amg;
             break;
         case slt_amg:
         case sle_amg:
         case sgt_amg:
         case sge_amg:
         case seq_amg:
         case sne_amg:
             off = scmp_amg;
             break;
         }
      if (off != -1) {
          if (ocvp && (oamtp->fallback > AMGfallNEVER)) {
              cv = ocvp[off];
              lr = -1;
          }
          if (!cv && (cvp && amtp->fallback > AMGfallNEVER)) {
              cv = cvp[off];
              lr = 1;
          }
      }
      if (cv)
          postpr = 1;
      else
          goto not_found;
    } else {
    not_found:			/* No method found, either report or croak */
      switch (method) {
         case to_sv_amg:
         case to_av_amg:
         case to_hv_amg:
         case to_gv_amg:
         case to_cv_amg:
             /* FAIL safe */
             return left;	/* Delegate operation to standard mechanisms. */
      }
      if (ocvp && (cv=ocvp[nomethod_amg])) { /* Call report method */
        notfound = 1; lr = -1;
      } else if (cvp && (cv=cvp[nomethod_amg])) {
        notfound = 1; lr = 1;
      } else if ((use_default_op =
                  (!ocvp || oamtp->fallback >= AMGfallYES)
                  && (!cvp || amtp->fallback >= AMGfallYES))
                 && !DEBUG_o_TEST) {
        /* Skip generating the "no method found" message.  */
        return NULL;
      } else {
        SV *msg;
        if (off==-1) off=method;
        msg = sv_2mortal(Perl_newSVpvf(aTHX_
                      "Operation \"%s\": no method found,%sargument %s%" SVf "%s%" SVf,
                      AMG_id2name(method + assignshift),
                      (flags & AMGf_unary ? " " : "\n\tleft "),
                      SvAMAGIC(left)?
                        "in overloaded package ":
                        "has no overloaded magic",
                      SvAMAGIC(left)?
                        SVfARG(newSVhek_mortal(HvNAME_HEK(SvSTASH(SvRV(left))))):
                        SVfARG(&PL_sv_no),
                      SvAMAGIC(right)?
                        ",\n\tright argument in overloaded package ":
                        (flags & AMGf_unary
                         ? ""
                         : ",\n\tright argument has no overloaded magic"),
                      SvAMAGIC(right)?
                        SVfARG(newSVhek_mortal(HvNAME_HEK(SvSTASH(SvRV(right))))):
                        SVfARG(&PL_sv_no)));
        if (use_default_op) {
          DEBUG_o( Perl_deb(aTHX_ "%" SVf, SVfARG(msg)) );
        } else {
          Perl_croak(aTHX_ "%" SVf, SVfARG(msg));
        }
        return NULL;
      }
      force_cpy = force_cpy || assign;
    }
  }

  switch (method) {
    /* in these cases, we're calling '+' or '-' as a fallback for a ++ or --
     * operation. we need this to return a value, so that it can be assigned
     * later on, in the postpr block (case inc_amg/dec_amg), even if the
     * increment or decrement was itself called in void context */
    case inc_amg:
      if (off == add_amg)
        force_scalar = 1;
      break;
    case dec_amg:
      if (off == subtr_amg)
        force_scalar = 1;
      break;
    /* in these cases, we're calling an assignment variant of an operator
     * (+= rather than +, for instance). regardless of whether it's a
     * fallback or not, it always has to return a value, which will be
     * assigned to the proper variable later */
    case add_amg:
    case subtr_amg:
    case mult_amg:
    case div_amg:
    case modulo_amg:
    case pow_amg:
    case lshift_amg:
    case rshift_amg:
    case repeat_amg:
    case concat_amg:
    case band_amg:
    case bor_amg:
    case bxor_amg:
    case sband_amg:
    case sbor_amg:
    case sbxor_amg:
      if (assign)
        force_scalar = 1;
      break;
    /* the copy constructor always needs to return a value */
    case copy_amg:
      force_scalar = 1;
      break;
    /* because of the way these are implemented (they don't perform the
     * dereferencing themselves, they return a reference that perl then
     * dereferences later), they always have to be in scalar context */
    case to_sv_amg:
    case to_av_amg:
    case to_hv_amg:
    case to_gv_amg:
    case to_cv_amg:
      force_scalar = 1;
      break;
    /* these don't have an op of their own; they're triggered by their parent
     * op, so the context there isn't meaningful ('$a and foo()' in void
     * context still needs to pass scalar context on to $a's bool overload) */
    case bool__amg:
    case numer_amg:
    case string_amg:
      force_scalar = 1;
      break;
  }

#ifdef DEBUGGING
  if (!notfound) {
    DEBUG_o(Perl_deb(aTHX_
                     "Overloaded operator \"%s\"%s%s%s:\n\tmethod%s found%s in package %" SVf "%s\n",
                     AMG_id2name(off),
                     method+assignshift==off? "" :
                     " (initially \"",
                     method+assignshift==off? "" :
                     AMG_id2name(method+assignshift),
                     method+assignshift==off? "" : "\")",
                     flags & AMGf_unary? "" :
                     lr==1 ? " for right argument": " for left argument",
                     flags & AMGf_unary? " for argument" : "",
                     stash ? SVfARG(newSVhek_mortal(HvNAME_HEK(stash))) : SVfARG(newSVpvs_flags("null", SVs_TEMP)),
                     fl? ",\n\tassignment variant used": "") );
  }
#endif
    /* Since we use shallow copy during assignment, we need
     * to duplicate the contents, probably calling user-supplied
     * version of copy operator
     */
    /* We need to copy in following cases:
     * a) Assignment form was called.
     * 		assignshift==1,  assign==T, method + 1 == off
     * b) Increment or decrement, called directly.
     * 		assignshift==0,  assign==0, method + 0 == off
     * c) Increment or decrement, translated to assignment add/subtr.
     * 		assignshift==0,  assign==T,
     *		force_cpy == T
     * d) Increment or decrement, translated to nomethod.
     * 		assignshift==0,  assign==0,
     *		force_cpy == T
     * e) Assignment form translated to nomethod.
     * 		assignshift==1,  assign==T, method + 1 != off
     *		force_cpy == T
     */
    /*	off is method, method+assignshift, or a result of opcode substitution.
     *	In the latter case assignshift==0, so only notfound case is important.
     */
  if ( (lr == -1) && ( ( (method + assignshift == off)
        && (assign || (method == inc_amg) || (method == dec_amg)))
      || force_cpy) )
  {
      /* newSVsv does not behave as advertised, so we copy missing
       * information by hand */
      SV *tmpRef = SvRV(left);
      SV *rv_copy;
      if (SvREFCNT(tmpRef) > 1 && (rv_copy = AMG_CALLunary(left,copy_amg))) {
          SvRV_set(left, rv_copy);
          SvSETMAGIC(left);
          SvREFCNT_dec_NN(tmpRef);
      }
  }

  {
    dSP;
    UNOP myop;
    SV* res;
    const bool oldcatch = CATCH_GET;
    I32 oldmark, nret;
                /* for multiconcat, we may call overload several times,
                 * with the context of individual concats being scalar,
                 * regardless of the overall context of the multiconcat op
                 */
    U8 gimme = (force_scalar || PL_op->op_type == OP_MULTICONCAT)
                    ? G_SCALAR : GIMME_V;

    CATCH_SET(TRUE);
    Zero(&myop, 1, UNOP);
    myop.op_flags = OPf_STACKED;
    myop.op_ppaddr = PL_ppaddr[OP_ENTERSUB];
    myop.op_type = OP_ENTERSUB;


    switch (gimme) {
        case G_VOID:
            myop.op_flags |= OPf_WANT_VOID;
            break;
        case G_LIST:
            if (flags & AMGf_want_list) {
                myop.op_flags |= OPf_WANT_LIST;
                break;
            }
            /* FALLTHROUGH */
        default:
            myop.op_flags |= OPf_WANT_SCALAR;
            break;
    }

    PUSHSTACKi(PERLSI_OVERLOAD);
    ENTER;
    SAVEOP();
    PL_op = (OP *) &myop;
    if (PERLDB_SUB && PL_curstash != PL_debstash)
        PL_op->op_private |= OPpENTERSUB_DB;
    Perl_pp_pushmark(aTHX);

    EXTEND(SP, notfound + 5);
    PUSHs(lr>0? right: left);
    PUSHs(lr>0? left: right);
    PUSHs( lr > 0 ? &PL_sv_yes : ( assign ? &PL_sv_undef : &PL_sv_no ));
    if (notfound) {
      PUSHs(newSVpvn_flags(AMG_id2name(method + assignshift),
                           AMG_id2namelen(method + assignshift), SVs_TEMP));
    }
    else if (flags & AMGf_numarg)
      PUSHs(&PL_sv_undef);
    if (flags & AMGf_numarg)
      PUSHs(&PL_sv_yes);
    PUSHs(MUTABLE_SV(cv));
    PUTBACK;
    oldmark = TOPMARK;
    CALLRUNOPS(aTHX);
    LEAVE;
    SPAGAIN;
    nret = SP - (PL_stack_base + oldmark);

    switch (gimme) {
        case G_VOID:
            /* returning NULL has another meaning, and we check the context
             * at the call site too, so this can be differentiated from the
             * scalar case */
            res = &PL_sv_undef;
            SP = PL_stack_base + oldmark;
            break;
        case G_LIST:
            if (flags & AMGf_want_list) {
                res = newSV_type_mortal(SVt_PVAV);
                av_extend((AV *)res, nret);
                while (nret--)
                    av_store((AV *)res, nret, POPs);
                break;
            }
            /* FALLTHROUGH */
        default:
            res = POPs;
            break;
    }

    PUTBACK;
    POPSTACK;
    CATCH_SET(oldcatch);

    if (postpr) {
      int ans;
      switch (method) {
      case le_amg:
      case sle_amg:
        ans=SvIV(res)<=0; break;
      case lt_amg:
      case slt_amg:
        ans=SvIV(res)<0; break;
      case ge_amg:
      case sge_amg:
        ans=SvIV(res)>=0; break;
      case gt_amg:
      case sgt_amg:
        ans=SvIV(res)>0; break;
      case eq_amg:
      case seq_amg:
        ans=SvIV(res)==0; break;
      case ne_amg:
      case sne_amg:
        ans=SvIV(res)!=0; break;
      case inc_amg:
      case dec_amg:
        SvSetSV(left,res); return left;
      case not_amg:
        ans=!SvTRUE_NN(res); break;
      default:
        ans=0; break;
      }
      return boolSV(ans);
    } else if (method==copy_amg) {
      if (!SvROK(res)) {
        Perl_croak(aTHX_ "Copy method did not return a reference");
      }
      return SvREFCNT_inc(SvRV(res));
    } else {
      return res;
    }
  }
}

/*
=for apidoc gv_name_set

Set the name for GV C<gv> to C<name> which is C<len> bytes long.  Thus it may
contain embedded NUL characters.

If C<flags> contains C<SVf_UTF8>, the name is treated as being encoded in
UTF-8; otherwise not.

=cut
*/

void
Perl_gv_name_set(pTHX_ GV *gv, const char *name, U32 len, U32 flags)
{
    U32 hash;

    PERL_ARGS_ASSERT_GV_NAME_SET;

    if (len > I32_MAX)
        Perl_croak(aTHX_ "panic: gv name too long (%" UVuf ")", (UV) len);

    if (!(flags & GV_ADD) && GvNAME_HEK(gv)) {
        unshare_hek(GvNAME_HEK(gv));
    }

    PERL_HASH(hash, name, len);
    GvNAME_HEK(gv) = share_hek(name, (flags & SVf_UTF8 ? -(I32)len : (I32)len), hash);
}

/*
=for apidoc gv_try_downgrade

If the typeglob C<gv> can be expressed more succinctly, by having
something other than a real GV in its place in the stash, replace it
with the optimised form.  Basic requirements for this are that C<gv>
is a real typeglob, is sufficiently ordinary, and is only referenced
from its package.  This function is meant to be used when a GV has been
looked up in part to see what was there, causing upgrading, but based
on what was found it turns out that the real GV isn't required after all.

If C<gv> is a completely empty typeglob, it is deleted from the stash.

If C<gv> is a typeglob containing only a sufficiently-ordinary constant
sub, the typeglob is replaced with a scalar-reference placeholder that
more compactly represents the same thing.

=cut
*/

void
Perl_gv_try_downgrade(pTHX_ GV *gv)
{
    HV *stash;
    CV *cv;
    HEK *namehek;
    SV **gvp;
    PERL_ARGS_ASSERT_GV_TRY_DOWNGRADE;

    /* XXX Why and where does this leave dangling pointers during global
       destruction? */
    if (PL_phase == PERL_PHASE_DESTRUCT) return;

    if (!(SvREFCNT(gv) == 1 && SvTYPE(gv) == SVt_PVGV && !SvFAKE(gv) &&
            !SvOBJECT(gv) && !SvREADONLY(gv) &&
            isGV_with_GP(gv) && GvGP(gv) &&
            !GvINTRO(gv) && GvREFCNT(gv) == 1 &&
            !GvSV(gv) && !GvAV(gv) && !GvHV(gv) && !GvIOp(gv) && !GvFORM(gv) &&
            GvEGVx(gv) == gv && (stash = GvSTASH(gv))))
        return;
    if (gv == PL_statgv || gv == PL_last_in_gv || gv == PL_stderrgv)
        return;
    if (SvMAGICAL(gv)) {
        MAGIC *mg;
        /* only backref magic is allowed */
        if (SvGMAGICAL(gv) || SvSMAGICAL(gv))
            return;
        for (mg = SvMAGIC(gv); mg; mg = mg->mg_moremagic) {
            if (mg->mg_type != PERL_MAGIC_backref)
                return;
        }
    }
    cv = GvCV(gv);
    if (!cv) {
        HEK *gvnhek = GvNAME_HEK(gv);
        (void)hv_deletehek(stash, gvnhek, G_DISCARD);
    } else if (GvMULTI(gv) && cv && SvREFCNT(cv) == 1 &&
            !SvOBJECT(cv) && !SvMAGICAL(cv) && !SvREADONLY(cv) &&
            CvSTASH(cv) == stash && !CvNAMED(cv) && CvGV(cv) == gv &&
            CvCONST(cv) && !CvNOWARN_AMBIGUOUS(cv) && !CvLVALUE(cv) && !CvUNIQUE(cv) &&
            !CvNODEBUG(cv) && !CvCLONE(cv) && !CvCLONED(cv) && !CvANON(cv) &&
            (namehek = GvNAME_HEK(gv)) &&
            (gvp = hv_fetchhek(stash, namehek, 0)) &&
            *gvp == (SV*)gv) {
        SV *value = SvREFCNT_inc(CvXSUBANY(cv).any_ptr);
        const bool imported = cBOOL(GvIMPORTED_CV(gv));
        SvREFCNT(gv) = 0;
        sv_clear((SV*)gv);
        SvREFCNT(gv) = 1;
        SvFLAGS(gv) = SVt_IV|SVf_ROK|SVprv_PCS_IMPORTED * imported;

        /* See also: 'SET_SVANY_FOR_BODYLESS_IV' in sv.c */
        SvANY(gv) = (XPVGV*)((char*)&(gv->sv_u.svu_iv) -
                                STRUCT_OFFSET(XPVIV, xiv_iv));
        SvRV_set(gv, value);
    }
}

GV *
Perl_gv_override(pTHX_ const char * const name, const STRLEN len)
{
    GV *gv = gv_fetchpvn(name, len, GV_NOTQUAL, SVt_PVCV);
    GV * const *gvp;
    PERL_ARGS_ASSERT_GV_OVERRIDE;
    if (gv && GvCVu(gv) && GvIMPORTED_CV(gv)) return gv;
    gvp = (GV**)hv_fetch(PL_globalstash, name, len, FALSE);
    gv = gvp ? *gvp : NULL;
    if (gv && !isGV(gv)) {
        if (!SvPCS_IMPORTED(gv)) return NULL;
        gv_init(gv, PL_globalstash, name, len, 0);
        return gv;
    }
    return gv && GvCVu(gv) && GvIMPORTED_CV(gv) ? gv : NULL;
}

#include "XSUB.h"

static void
core_xsub(pTHX_ CV* cv)
{
    Perl_croak(aTHX_
       "&CORE::%s cannot be called directly", GvNAME(CvGV(cv))
    );
}

/*
 * ex: set ts=8 sts=4 sw=4 et:
 */
