/*    pad.c
 *
 *    Copyright (C) 2002, 2003, 2004, 2005, 2006, 2007, 2008
 *    by Larry Wall and others
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 */

/*
 *  'Anyway: there was this Mr. Frodo left an orphan and stranded, as you
 *   might say, among those queer Bucklanders, being brought up anyhow in
 *   Brandy Hall.  A regular warren, by all accounts.  Old Master Gorbadoc
 *   never had fewer than a couple of hundred relations in the place.
 *   Mr. Bilbo never did a kinder deed than when he brought the lad back
 *   to live among decent folk.'                           --the Gaffer
 *
 *     [p.23 of _The Lord of the Rings_, I/i: "A Long-Expected Party"]
 */

/*
=for apidoc_section $pad

=for apidoc Amx|PADLIST *|CvPADLIST|CV *cv

CV's can have CvPADLIST(cv) set to point to a PADLIST.  This is the CV's
scratchpad, which stores lexical variables and opcode temporary and
per-thread values.

For these purposes "formats" are a kind-of CV; eval""s are too (except they're
not callable at will and are always thrown away after the eval"" is done
executing).  Require'd files are simply evals without any outer lexical
scope.

XSUBs do not have a C<CvPADLIST>.  C<dXSTARG> fetches values from C<PL_curpad>,
but that is really the callers pad (a slot of which is allocated by
every entersub). Do not get or set C<CvPADLIST> if a CV is an XSUB (as
determined by C<CvISXSUB()>), C<CvPADLIST> slot is reused for a different
internal purpose in XSUBs.

The PADLIST has a C array where pads are stored.

The 0th entry of the PADLIST is a PADNAMELIST
which represents the "names" or rather
the "static type information" for lexicals.  The individual elements of a
PADNAMELIST are PADNAMEs.  Future
refactorings might stop the PADNAMELIST from being stored in the PADLIST's
array, so don't rely on it.  See L</PadlistNAMES>.

The CvDEPTH'th entry of a PADLIST is a PAD (an AV) which is the stack frame
at that depth of recursion into the CV.  The 0th slot of a frame AV is an
AV which is C<@_>.  Other entries are storage for variables and op targets.

Iterating over the PADNAMELIST iterates over all possible pad
items.  Pad slots for targets (C<SVs_PADTMP>)
and GVs end up having &PL_padname_undef "names", while slots for constants 
have C<&PL_padname_const> "names" (see C<L</pad_alloc>>).  That
C<&PL_padname_undef>
and C<&PL_padname_const> are used is an implementation detail subject to
change.  To test for them, use C<!PadnamePV(name)> and
S<C<PadnamePV(name) && !PadnameLEN(name)>>, respectively.

Only C<my>/C<our> variable slots get valid names.
The rest are op targets/GVs/constants which are statically allocated
or resolved at compile time.  These don't have names by which they
can be looked up from Perl code at run time through eval"" the way
C<my>/C<our> variables can be.  Since they can't be looked up by "name"
but only by their index allocated at compile time (which is usually
in C<< PL_op->op_targ >>), wasting a name SV for them doesn't make sense.

The pad names in the PADNAMELIST have their PV holding the name of
the variable.  The C<COP_SEQ_RANGE_LOW> and C<_HIGH> fields form a range
(low+1..high inclusive) of cop_seq numbers for which the name is
valid.  During compilation, these fields may hold the special value
PERL_PADSEQ_INTRO to indicate various stages:

 COP_SEQ_RANGE_LOW        _HIGH
 -----------------        -----
 PERL_PADSEQ_INTRO            0   variable not yet introduced:
                                  { my ($x
 valid-seq#   PERL_PADSEQ_INTRO   variable in scope:
                                  { my ($x);
 valid-seq#          valid-seq#   compilation of scope complete:
                                  { my ($x); .... }

When a lexical var hasn't yet been introduced, it already exists from the
perspective of duplicate declarations, but not for variable lookups, e.g.

    my ($x, $x); # '"my" variable $x masks earlier declaration'
    my $x = $x;  # equal to my $x = $::x;

For typed lexicals C<PadnameTYPE> points at the type stash.  For C<our>
lexicals, C<PadnameOURSTASH> points at the stash of the associated global (so
that duplicate C<our> declarations in the same package can be detected).
C<PadnameGEN> is sometimes used to store the generation number during
compilation.

If C<PadnameOUTER> is set on the pad name, then that slot in the frame AV
is a REFCNT'ed reference to a lexical from "outside".  Such entries
are sometimes referred to as 'fake'.  In this case, the name does not
use 'low' and 'high' to store a cop_seq range, since it is in scope
throughout.  Instead 'high' stores some flags containing info about
the real lexical (is it declared in an anon, and is it capable of being
instantiated multiple times?), and for fake ANONs, 'low' contains the index
within the parent's pad where the lexical's value is stored, to make
cloning quicker.

If the 'name' is C<&> the corresponding entry in the PAD
is a CV representing a possible closure.

Note that formats are treated as anon subs, and are cloned each time
write is called (if necessary).

The flag C<SVs_PADSTALE> is cleared on lexicals each time the C<my()> is executed,
and set on scope exit.  This allows the
C<"Variable $x is not available"> warning
to be generated in evals, such as 

    { my $x = 1; sub f { eval '$x'} } f();

For state vars, C<SVs_PADSTALE> is overloaded to mean 'not yet initialised',
but this internal state is stored in a separate pad entry.

=for apidoc Amnh||SVs_PADSTALE

=for apidoc AmnxU|PADNAMELIST *|PL_comppad_name

During compilation, this points to the array containing the names part
of the pad for the currently-compiling code.

=for apidoc AmnxU|PAD *|PL_comppad

During compilation, this points to the array containing the values
part of the pad for the currently-compiling code.  (At runtime a CV may
have many such value arrays; at compile time just one is constructed.)
At runtime, this points to the array containing the currently-relevant
values for the pad for the currently-executing code.

=for apidoc AmnxU|SV **|PL_curpad

Points directly to the body of the L</PL_comppad> array.
(I.e., this is C<PadARRAY(PL_comppad)>.)

=cut
*/


#include "EXTERN.h"
#define PERL_IN_PAD_C
#include "perl.h"
#include "keywords.h"

#define COP_SEQ_RANGE_LOW_set(sv,val)		\
  STMT_START { (sv)->xpadn_low = (val); } STMT_END
#define COP_SEQ_RANGE_HIGH_set(sv,val)		\
  STMT_START { (sv)->xpadn_high = (val); } STMT_END

#define PARENT_PAD_INDEX_set		COP_SEQ_RANGE_LOW_set
#define PARENT_FAKELEX_FLAGS_set	COP_SEQ_RANGE_HIGH_set

#ifdef DEBUGGING
void
Perl_set_padlist(CV * cv, PADLIST *padlist){
    PERL_ARGS_ASSERT_SET_PADLIST;
#  if PTRSIZE == 8
    assert((Size_t)padlist != UINT64_C(0xEFEFEFEFEFEFEFEF));
#  elif PTRSIZE == 4
    assert((Size_t)padlist != 0xEFEFEFEF);
#  else
#    error unknown pointer size
#  endif
    assert(!CvISXSUB(cv));
    ((XPVCV*)MUTABLE_PTR(SvANY(cv)))->xcv_padlist_u.xcv_padlist = padlist;
}
#endif

/*
=for apidoc pad_new

Create a new padlist, updating the global variables for the
currently-compiling padlist to point to the new padlist.  The following
flags can be OR'ed together:

    padnew_CLONE	this pad is for a cloned CV
    padnew_SAVE		save old globals on the save stack
    padnew_SAVESUB	also save extra stuff for start of sub

=cut
*/

PADLIST *
Perl_pad_new(pTHX_ int flags)
{
    PADLIST *padlist;
    PADNAMELIST *padname;
    PAD *pad;
    PAD **ary;

    ASSERT_CURPAD_LEGAL("pad_new");

    /* save existing state, ... */

    if (flags & padnew_SAVE) {
        SAVECOMPPAD();
        if (! (flags & padnew_CLONE)) {
            SAVESPTR(PL_comppad_name);
            SAVESTRLEN(PL_padix);
            SAVESTRLEN(PL_constpadix);
            SAVESTRLEN(PL_comppad_name_fill);
            SAVESTRLEN(PL_min_intro_pending);
            SAVESTRLEN(PL_max_intro_pending);
            SAVEBOOL(PL_cv_has_eval);
            if (flags & padnew_SAVESUB) {
                SAVEBOOL(PL_pad_reset_pending);
            }
        }
    }

    /* ... create new pad ... */

    Newxz(padlist, 1, PADLIST);
    pad		= newAV();
    Newxz(AvALLOC(pad), 4, SV *); /* Originally sized to
                                     match av_extend default */
    AvARRAY(pad) = AvALLOC(pad);
    AvMAX(pad) = 3;
    AvFILLp(pad) = 0; /* @_ or NULL, set below. */

    if (flags & padnew_CLONE) {
        AV * const a0 = newAV();			/* will be @_ */
        AvARRAY(pad)[0] = MUTABLE_SV(a0);
        AvREIFY_only(a0);

        PadnamelistREFCNT(padname = PL_comppad_name)++;
    }
    else {
        padlist->xpadl_id = PL_padlist_generation++;
        /* Set implicitly through use of Newxz above
            AvARRAY(pad)[0] = NULL;
        */
        padname = newPADNAMELIST(0);
        padnamelist_store(padname, 0, &PL_padname_undef);
    }

    /* Most subroutines never recurse, hence only need 2 entries in the padlist
       array - names, and depth=1.  The default for av_store() is to allocate
       0..3, and even an explicit call to av_extend() with <3 will be rounded
       up, so we inline the allocation of the array here.  */
    Newx(ary, 2, PAD *);
    PadlistMAX(padlist) = 1;
    PadlistARRAY(padlist) = ary;
    ary[0] = (PAD *)padname;
    ary[1] = pad;

    /* ... then update state variables */

    PL_comppad		= pad;
    PL_curpad		= AvARRAY(pad);

    if (! (flags & padnew_CLONE)) {
        PL_comppad_name	     = padname;
        PL_comppad_name_fill = 0;
        PL_min_intro_pending = 0;
        PL_padix	     = 0;
        PL_constpadix	     = 0;
        PL_cv_has_eval	     = 0;
    }

    DEBUG_X(PerlIO_printf(Perl_debug_log,
          "Pad 0x%" UVxf "[0x%" UVxf "] new:       compcv=0x%" UVxf
              " name=0x%" UVxf " flags=0x%" UVxf "\n",
          PTR2UV(PL_comppad), PTR2UV(PL_curpad), PTR2UV(PL_compcv),
              PTR2UV(padname), (UV)flags
        )
    );

    return (PADLIST*)padlist;
}


/*
=for apidoc_section $embedding

=for apidoc cv_undef

Clear out all the active components of a CV.  This can happen either
by an explicit C<undef &foo>, or by the reference count going to zero.
In the former case, we keep the C<CvOUTSIDE> pointer, so that any anonymous
children can still follow the full lexical scope chain.

=cut
*/

void
Perl_cv_undef(pTHX_ CV *cv)
{
    PERL_ARGS_ASSERT_CV_UNDEF;
    cv_undef_flags(cv, 0);
}

void
Perl_cv_undef_flags(pTHX_ CV *cv, U32 flags)
{
    CV cvbody;/*CV body will never be realloced inside this func,
               so don't read it more than once, use fake CV so existing macros
               will work, the indirection and CV head struct optimized away*/
    SvANY(&cvbody) = SvANY(cv);

    PERL_ARGS_ASSERT_CV_UNDEF_FLAGS;

    DEBUG_X(PerlIO_printf(Perl_debug_log,
          "CV undef: cv=0x%" UVxf " comppad=0x%" UVxf "\n",
            PTR2UV(cv), PTR2UV(PL_comppad))
    );

    if (CvFILE(&cvbody)) {
        char * file = CvFILE(&cvbody);
        CvFILE(&cvbody) = NULL;
        if(CvDYNFILE(&cvbody))
            Safefree(file);
    }

    /* CvSLABBED_off(&cvbody); *//* turned off below */
    /* release the sub's body */
    if (!CvISXSUB(&cvbody)) {
        if(CvROOT(&cvbody)) {
            assert(SvTYPE(cv) == SVt_PVCV || SvTYPE(cv) == SVt_PVFM); /*unsafe is safe */
            if (CvDEPTHunsafe(&cvbody)) {
                assert(SvTYPE(cv) == SVt_PVCV);
                Perl_croak_nocontext("Can't undef active subroutine");
            }
            ENTER;

            PAD_SAVE_SETNULLPAD();

            if (CvSLABBED(&cvbody)) OpslabREFCNT_dec_padok(OpSLAB(CvROOT(&cvbody)));
            op_free(CvROOT(&cvbody));
            CvROOT(&cvbody) = NULL;
            CvSTART(&cvbody) = NULL;
            LEAVE;
        }
        else if (CvSLABBED(&cvbody)) {
            if( CvSTART(&cvbody)) {
                ENTER;
                PAD_SAVE_SETNULLPAD();

                /* discard any leaked ops */
                if (PL_parser)
                    parser_free_nexttoke_ops(PL_parser, (OPSLAB *)CvSTART(&cvbody));
                opslab_force_free((OPSLAB *)CvSTART(&cvbody));
                CvSTART(&cvbody) = NULL;

                LEAVE;
            }
#ifdef DEBUGGING
            else Perl_warn(aTHX_ "Slab leaked from cv %p", (void*)cv);
#endif
        }
    }
    else { /* don't bother checking if CvXSUB(cv) is true, less branching */
        CvXSUB(&cvbody) = NULL;
    }
    SvPOK_off(MUTABLE_SV(cv));		/* forget prototype */
    sv_unmagic((SV *)cv, PERL_MAGIC_checkcall);
    if (!(flags & CV_UNDEF_KEEP_NAME)) {
        if (CvNAMED(&cvbody)) {
            CvNAME_HEK_set(&cvbody, NULL);
            CvNAMED_off(&cvbody);
        }
        else CvGV_set(cv, NULL);
    }

    /* This statement and the subsequence if block was pad_undef().  */
    pad_peg("pad_undef");

    if (!CvISXSUB(&cvbody) && CvPADLIST(&cvbody)) {
        PADOFFSET ix;
        const PADLIST *padlist = CvPADLIST(&cvbody);

        /* Free the padlist associated with a CV.
           If parts of it happen to be current, we null the relevant PL_*pad*
           global vars so that we don't have any dangling references left.
           We also repoint the CvOUTSIDE of any about-to-be-orphaned inner
           subs to the outer of this cv.  */

        DEBUG_X(PerlIO_printf(Perl_debug_log,
                              "Pad undef: cv=0x%" UVxf " padlist=0x%" UVxf " comppad=0x%" UVxf "\n",
                              PTR2UV(cv), PTR2UV(padlist), PTR2UV(PL_comppad))
                );

        /* detach any '&' anon children in the pad; if afterwards they
         * are still live, fix up their CvOUTSIDEs to point to our outside,
         * bypassing us. */

        if (PL_phase != PERL_PHASE_DESTRUCT) { /* don't bother during global destruction */
            CV * const outercv = CvOUTSIDE(&cvbody);
            const U32 seq = CvOUTSIDE_SEQ(&cvbody);
            PADNAMELIST * const comppad_name = PadlistNAMES(padlist);
            PADNAME ** const namepad = PadnamelistARRAY(comppad_name);
            PAD * const comppad = PadlistARRAY(padlist)[1];
            SV ** const curpad = AvARRAY(comppad);
            for (ix = PadnamelistMAX(comppad_name); ix > 0; ix--) {
                PADNAME * const name = namepad[ix];
                if (name && PadnamePV(name) && *PadnamePV(name) == '&')
                    {
                        CV * const innercv = MUTABLE_CV(curpad[ix]);
                        U32 inner_rc;
                        assert(innercv);
                        assert(SvTYPE(innercv) != SVt_PVFM);
                        inner_rc = SvREFCNT(innercv);
                        assert(inner_rc);

                        if (SvREFCNT(comppad) < 2) { /* allow for /(?{ sub{} })/  */
                            curpad[ix] = NULL;
                            SvREFCNT_dec_NN(innercv);
                            inner_rc--;
                        }

                        /* in use, not just a prototype */
                        if (inner_rc && SvTYPE(innercv) == SVt_PVCV
                         && (CvOUTSIDE(innercv) == cv))
                        {
                            assert(CvWEAKOUTSIDE(innercv));
                            /* don't relink to grandfather if he's being freed */
                            if (outercv && SvREFCNT(outercv)) {
                                CvWEAKOUTSIDE_off(innercv);
                                CvOUTSIDE(innercv) = outercv;
                                CvOUTSIDE_SEQ(innercv) = seq;
                                SvREFCNT_inc_simple_void_NN(outercv);
                            }
                            else {
                                CvOUTSIDE(innercv) = NULL;
                            }
                        }
                    }
            }
        }

        ix = PadlistMAX(padlist);
        while (ix > 0) {
            PAD * const sv = PadlistARRAY(padlist)[ix--];
            if (sv) {
                if (sv == PL_comppad) {
                    PL_comppad = NULL;
                    PL_curpad = NULL;
                }
                SvREFCNT_dec_NN(sv);
            }
        }
        {
            PADNAMELIST * const names = PadlistNAMES(padlist);
            if (names == PL_comppad_name && PadnamelistREFCNT(names) == 1)
                PL_comppad_name = NULL;
            PadnamelistREFCNT_dec(names);
        }
        if (PadlistARRAY(padlist)) Safefree(PadlistARRAY(padlist));
        Safefree(padlist);
        CvPADLIST_set(&cvbody, NULL);
    }
    else if (CvISXSUB(&cvbody)) {
        if (CvREFCOUNTED_ANYSV(&cvbody))
            SvREFCNT_dec(CvXSUBANY(&cvbody).any_sv);
        CvHSCXT(&cvbody) = NULL;
    }
    /* else is (!CvISXSUB(&cvbody) && !CvPADLIST(&cvbody)) {do nothing;} */


    /* remove CvOUTSIDE unless this is an undef rather than a free */
    if (!SvREFCNT(cv)) {
        CV * outside = CvOUTSIDE(&cvbody);
        if(outside) {
            CvOUTSIDE(&cvbody) = NULL;
            if (!CvWEAKOUTSIDE(&cvbody))
                SvREFCNT_dec_NN(outside);
        }
    }
    if (CvCONST(&cvbody)) {
        SvREFCNT_dec(MUTABLE_SV(CvXSUBANY(&cvbody).any_ptr));
        /* CvCONST_off(cv); *//* turned off below */
    }
    /* delete all flags except WEAKOUTSIDE and CVGV_RC, which indicate the
     * ref status of CvOUTSIDE and CvGV, and ANON, NAMED and
     * LEXICAL, which are used to determine the sub's name.  */
    CvFLAGS(&cvbody) &= (CVf_WEAKOUTSIDE|CVf_CVGV_RC|CVf_ANON|CVf_LEXICAL
                   |CVf_NAMED);
}

/*
=for apidoc cv_forget_slab

When a CV has a reference count on its slab (C<CvSLABBED>), it is responsible
for making sure it is freed.  (Hence, no two CVs should ever have a
reference count on the same slab.)  The CV only needs to reference the slab
during compilation.  Once it is compiled and C<CvROOT> attached, it has
finished its job, so it can forget the slab.

=cut
*/

void
Perl_cv_forget_slab(pTHX_ CV *cv)
{
    bool slabbed;
    OPSLAB *slab = NULL;

    if (!cv)
        return;
    slabbed = cBOOL(CvSLABBED(cv));
    if (!slabbed) return;

    CvSLABBED_off(cv);

    if      (CvROOT(cv))  slab = OpSLAB(CvROOT(cv));
    else if (CvSTART(cv)) slab = (OPSLAB *)CvSTART(cv);
#ifdef DEBUGGING
    else if (slabbed)     Perl_warn(aTHX_ "Slab leaked from cv %p", (void*)cv);
#endif

    if (slab) {
#ifdef PERL_DEBUG_READONLY_OPS
        const size_t refcnt = slab->opslab_refcnt;
#endif
        OpslabREFCNT_dec(slab);
#ifdef PERL_DEBUG_READONLY_OPS
        if (refcnt > 1) Slab_to_ro(slab);
#endif
    }
}

/*
=for apidoc pad_alloc_name

Allocates a place in the currently-compiling
pad (via L<perlapi/pad_alloc>) and
then stores a name for that entry.  C<name> is adopted and
becomes the name entry; it must already contain the name
string.  C<typestash> and C<ourstash> and the C<padadd_STATE>
flag get added to C<name>.  None of the other
processing of L<perlapi/pad_add_name_pvn>
is done.  Returns the offset of the allocated pad slot.

=cut
*/

static PADOFFSET
S_pad_alloc_name(pTHX_ PADNAME *name, U32 flags, HV *typestash,
                       HV *ourstash)
{
    const PADOFFSET offset = pad_alloc(OP_PADSV, SVs_PADMY);

    PERL_ARGS_ASSERT_PAD_ALLOC_NAME;

    ASSERT_CURPAD_ACTIVE("pad_alloc_name");

    if (typestash) {
        PadnameFLAGS(name) |= PADNAMEf_TYPED;
        PadnameTYPE(name) =
            MUTABLE_HV(SvREFCNT_inc_simple_NN(MUTABLE_SV(typestash)));
    }
    if (ourstash) {
        PadnameFLAGS(name) |= PADNAMEf_OUR;
        PadnameOURSTASH_set(name, ourstash);
        SvREFCNT_inc_simple_void_NN(ourstash);
    }
    else if (flags & padadd_STATE) {
        PadnameFLAGS(name) |= PADNAMEf_STATE;
    }
    if (flags & padadd_FIELD) {
        assert(HvSTASH_IS_CLASS(PL_curstash));
        class_add_field(PL_curstash, name);
    }

    padnamelist_store(PL_comppad_name, offset, name);
    if (PadnameLEN(name) > 1)
        PadnamelistMAXNAMED(PL_comppad_name) = offset;
    return offset;
}

/*
=for apidoc pad_add_name_pvn

Allocates a place in the currently-compiling pad for a named lexical
variable.  Stores the name and other metadata in the name part of the
pad, and makes preparations to manage the variable's lexical scoping.
Returns the offset of the allocated pad slot.

C<namepv>/C<namelen> specify the variable's name, including leading sigil.
If C<typestash> is non-null, the name is for a typed lexical, and this
identifies the type.  If C<ourstash> is non-null, it's a lexical reference
to a package variable, and this identifies the package.  The following
flags can be OR'ed together:

 padadd_OUR          redundantly specifies if it's a package var
 padadd_STATE        variable will retain value persistently
 padadd_NO_DUP_CHECK skip check for lexical shadowing
 padadd_FIELD        specifies that the lexical is a field for a class

=cut
*/

PADOFFSET
Perl_pad_add_name_pvn(pTHX_ const char *namepv, STRLEN namelen,
                U32 flags, HV *typestash, HV *ourstash)
{
    PADOFFSET offset;
    PADNAME *name;

    PERL_ARGS_ASSERT_PAD_ADD_NAME_PVN;

    if (flags & ~(padadd_OUR|padadd_STATE|padadd_NO_DUP_CHECK|padadd_FIELD))
        Perl_croak(aTHX_ "panic: pad_add_name_pvn illegal flag bits 0x%" UVxf,
                   (UV)flags);

    name = newPADNAMEpvn(namepv, namelen);

    if ((flags & padadd_NO_DUP_CHECK) == 0) {
        ENTER;
        SAVEFREEPADNAME(name); /* in case of fatal warnings */
        /* check for duplicate declaration */
        pad_check_dup(name, flags & (padadd_OUR|padadd_FIELD), ourstash);
        PadnameREFCNT_inc(name);
        LEAVE;
    }

    offset = pad_alloc_name(name, flags, typestash, ourstash);

    /* not yet introduced */
    COP_SEQ_RANGE_LOW_set(name, PERL_PADSEQ_INTRO);
    COP_SEQ_RANGE_HIGH_set(name, 0);

    if (!PL_min_intro_pending)
        PL_min_intro_pending = offset;
    PL_max_intro_pending = offset;
    /* if it's not a simple scalar, replace with an AV or HV */
    assert(SvTYPE(PL_curpad[offset]) == SVt_NULL);
    assert(SvREFCNT(PL_curpad[offset]) == 1);
    if (namelen != 0 && *namepv == '@')
        sv_upgrade(PL_curpad[offset], SVt_PVAV);
    else if (namelen != 0 && *namepv == '%')
        sv_upgrade(PL_curpad[offset], SVt_PVHV);
    else if (namelen != 0 && *namepv == '&')
        sv_upgrade(PL_curpad[offset], SVt_PVCV);
    assert(SvPADMY(PL_curpad[offset]));
    DEBUG_Xv(PerlIO_printf(Perl_debug_log,
                           "Pad addname: %ld \"%s\" new lex=0x%" UVxf "\n",
                           (long)offset, PadnamePV(name),
                           PTR2UV(PL_curpad[offset])));

    return offset;
}

/*
=for apidoc pad_add_name_pv

Exactly like L</pad_add_name_pvn>, but takes a nul-terminated string
instead of a string/length pair.

=cut
*/

PADOFFSET
Perl_pad_add_name_pv(pTHX_ const char *name,
                     const U32 flags, HV *typestash, HV *ourstash)
{
    PERL_ARGS_ASSERT_PAD_ADD_NAME_PV;
    return pad_add_name_pvn(name, strlen(name), flags, typestash, ourstash);
}

/*
=for apidoc pad_add_name_sv

Exactly like L</pad_add_name_pvn>, but takes the name string in the form
of an SV instead of a string/length pair.

=cut
*/

PADOFFSET
Perl_pad_add_name_sv(pTHX_ SV *name, U32 flags, HV *typestash, HV *ourstash)
{
    char *namepv;
    STRLEN namelen;
    PERL_ARGS_ASSERT_PAD_ADD_NAME_SV;
    namepv = SvPVutf8(name, namelen);
    return pad_add_name_pvn(namepv, namelen, flags, typestash, ourstash);
}

/*
=for apidoc pad_alloc

Allocates a place in the currently-compiling pad,
returning the offset of the allocated pad slot.
No name is initially attached to the pad slot.
C<tmptype> is a set of flags indicating the kind of pad entry required,
which will be set in the value SV for the allocated pad entry:

    SVs_PADMY    named lexical variable ("my", "our", "state")
    SVs_PADTMP   unnamed temporary store
    SVf_READONLY constant shared between recursion levels

C<SVf_READONLY> has been supported here only since perl 5.20.  To work with
earlier versions as well, use C<SVf_READONLY|SVs_PADTMP>.  C<SVf_READONLY>
does not cause the SV in the pad slot to be marked read-only, but simply
tells C<pad_alloc> that it I<will> be made read-only (by the caller), or at
least should be treated as such.

C<optype> should be an opcode indicating the type of operation that the
pad entry is to support.  This doesn't affect operational semantics,
but is used for debugging.

=cut
*/

PADOFFSET
Perl_pad_alloc(pTHX_ I32 optype, U32 tmptype)
{
    SV *sv;
    PADOFFSET retval;

    PERL_UNUSED_ARG(optype);
    ASSERT_CURPAD_ACTIVE("pad_alloc");

    if (AvARRAY(PL_comppad) != PL_curpad)
        Perl_croak(aTHX_ "panic: pad_alloc, %p!=%p",
                   AvARRAY(PL_comppad), PL_curpad);
    if (PL_pad_reset_pending)
        pad_reset();
    if (tmptype == SVs_PADMY) { /* Not & because this ‘flag’ is 0.  */
        /* For a my, simply push a null SV onto the end of PL_comppad. */
        sv = *av_store_simple(PL_comppad, AvFILLp(PL_comppad) + 1, newSV_type(SVt_NULL));
        retval = (PADOFFSET)AvFILLp(PL_comppad);
    }
    else {
        /* For a tmp, scan the pad from PL_padix upwards
         * for a slot which has no name and no active value.
         * For a constant, likewise, but use PL_constpadix.
         */
        PADNAME * const * const names = PadnamelistARRAY(PL_comppad_name);
        const SSize_t names_fill = PadnamelistMAX(PL_comppad_name);
        const bool konst = cBOOL(tmptype & SVf_READONLY);
        retval = konst ? PL_constpadix : PL_padix;
        for (;;) {
            /*
             * Entries that close over unavailable variables
             * in outer subs contain values not marked PADMY.
             * Thus we must skip, not just pad values that are
             * marked as current pad values, but also those with names.
             * If pad_reset is enabled, ‘current’ means different
             * things depending on whether we are allocating a con-
             * stant or a target.  For a target, things marked PADTMP
             * can be reused; not so for constants.
             */
            PADNAME *pn;
            if (++retval <= names_fill &&
                   (pn = names[retval]) && PadnamePV(pn))
                continue;
            sv = *av_fetch_simple(PL_comppad, retval, TRUE);
            if (!(SvFLAGS(sv) &
#ifdef USE_PAD_RESET
                    (konst ? SVs_PADTMP : 0)
#else
                    SVs_PADTMP
#endif
                 ))
                break;
        }
        if (konst) {
            padnamelist_store(PL_comppad_name, retval, &PL_padname_const);
            tmptype &= ~SVf_READONLY;
            tmptype |= SVs_PADTMP;
        }
        *(konst ? &PL_constpadix : &PL_padix) = retval;
    }
    SvFLAGS(sv) |= tmptype;
    PL_curpad = AvARRAY(PL_comppad);

    DEBUG_X(PerlIO_printf(Perl_debug_log,
          "Pad 0x%" UVxf "[0x%" UVxf "] alloc:   %ld for %s\n",
          PTR2UV(PL_comppad), PTR2UV(PL_curpad), (long) retval,
          PL_op_name[optype]));
#ifdef DEBUG_LEAKING_SCALARS
    sv->sv_debug_optype = optype;
    sv->sv_debug_inpad = 1;
#endif
    return retval;
}

/*
=for apidoc pad_add_anon

Allocates a place in the currently-compiling pad (via L</pad_alloc>)
for an anonymous function that is lexically scoped inside the
currently-compiling function.
The function C<func> is linked into the pad, and its C<CvOUTSIDE> link
to the outer scope is weakened to avoid a reference loop.

One reference count is stolen, so you may need to do C<SvREFCNT_inc(func)>.

C<optype> should be an opcode indicating the type of operation that the
pad entry is to support.  This doesn't affect operational semantics,
but is used for debugging.

=cut
*/

PADOFFSET
Perl_pad_add_anon(pTHX_ CV* func, I32 optype)
{
    PADOFFSET ix;
    PADNAME * const name = newPADNAMEpvn("&", 1);

    PERL_ARGS_ASSERT_PAD_ADD_ANON;
    assert (SvTYPE(func) == SVt_PVCV);

    pad_peg("add_anon");
    /* These two aren't used; just make sure they're not equal to
     * PERL_PADSEQ_INTRO.  They should be 0 by default.  */
    assert(COP_SEQ_RANGE_LOW (name) != PERL_PADSEQ_INTRO);
    assert(COP_SEQ_RANGE_HIGH(name) != PERL_PADSEQ_INTRO);
    ix = pad_alloc(optype, SVs_PADMY);
    padnamelist_store(PL_comppad_name, ix, name);
    av_store(PL_comppad, ix, (SV*)func);

    /* to avoid ref loops, we never have parent + child referencing each
     * other simultaneously */
    if (CvOUTSIDE(func)) {
        assert(!CvWEAKOUTSIDE(func));
        CvWEAKOUTSIDE_on(func);
        SvREFCNT_dec_NN(CvOUTSIDE(func));
    }
    return ix;
}

void
Perl_pad_add_weakref(pTHX_ CV* func)
{
    const PADOFFSET ix = pad_alloc(OP_NULL, SVs_PADMY);
    PADNAME * const name = newPADNAMEpvn("&", 1);
    SV * const rv = newRV_inc((SV *)func);

    PERL_ARGS_ASSERT_PAD_ADD_WEAKREF;

    /* These two aren't used; just make sure they're not equal to
     * PERL_PADSEQ_INTRO.  They should be 0 by default.  */
    assert(COP_SEQ_RANGE_LOW (name) != PERL_PADSEQ_INTRO);
    assert(COP_SEQ_RANGE_HIGH(name) != PERL_PADSEQ_INTRO);
    padnamelist_store(PL_comppad_name, ix, name);
    sv_rvweaken(rv);
    av_store(PL_comppad, ix, rv);
}

/*
=for apidoc pad_check_dup

Check for duplicate declarations: report any of:

     * a 'my' in the current scope with the same name;
     * an 'our' (anywhere in the pad) with the same name and the
       same stash as 'ourstash'

C<is_our> indicates that the name to check is an C<"our"> declaration.

=cut
*/

STATIC void
S_pad_check_dup(pTHX_ PADNAME *name, U32 flags, const HV *ourstash)
{
    PADNAME	**svp;
    PADOFFSET	top, off;
    const U32	is_our = flags & padadd_OUR;
    bool        is_field = flags & padadd_FIELD;

    PERL_ARGS_ASSERT_PAD_CHECK_DUP;

    ASSERT_CURPAD_ACTIVE("pad_check_dup");

    assert((flags & ~(padadd_OUR|padadd_FIELD)) == 0);

    if (PadnamelistMAX(PL_comppad_name) < 0 || !ckWARN(WARN_SHADOW))
        return; /* nothing to check */

    svp = PadnamelistARRAY(PL_comppad_name);
    top = PadnamelistMAX(PL_comppad_name);
    /* check the current scope */
    for (off = top; off > PL_comppad_name_floor; off--) {
        PADNAME * const pn = svp[off];
        if (pn
            && PadnameLEN(pn) == PadnameLEN(name)
            && !PadnameOUTER(pn)
            && (   COP_SEQ_RANGE_LOW(pn)  == PERL_PADSEQ_INTRO
                || COP_SEQ_RANGE_HIGH(pn) == PERL_PADSEQ_INTRO)
            && memEQ(PadnamePV(pn), PadnamePV(name), PadnameLEN(name)))
        {
            if (is_our && (PadnameIsOUR(pn)))
                break; /* "our" masking "our" */
            if (is_field && PadnameIsFIELD(pn) &&
                    PadnameFIELDINFO(pn)->fieldstash != PL_curstash)
                break; /* field of a different class */
            /* diag_listed_as: "%s" variable %s masks earlier declaration in same %s */
            Perl_warner(aTHX_ packWARN(WARN_SHADOW),
                "\"%s\" %s %" PNf " masks earlier declaration in same %s",
                (   is_our                         ? "our"   :
                    PL_parser->in_my == KEY_my     ? "my"    :
                    PL_parser->in_my == KEY_sigvar ? "my"    :
                    PL_parser->in_my == KEY_field  ? "field" :
                                                     "state" ),
                *PadnamePV(pn) == '&' ? "subroutine" : "variable",
                PNfARG(pn),
                (COP_SEQ_RANGE_HIGH(pn) == PERL_PADSEQ_INTRO
                    ? "scope" : "statement"));
            --off;
            break;
        }
    }
    /* check the rest of the pad */
    if (is_our) {
        while (off > 0) {
            PADNAME * const pn = svp[off];
            if (pn
                && PadnameLEN(pn) == PadnameLEN(name)
                && !PadnameOUTER(pn)
                && (   COP_SEQ_RANGE_LOW(pn)  == PERL_PADSEQ_INTRO
                    || COP_SEQ_RANGE_HIGH(pn) == PERL_PADSEQ_INTRO)
                && PadnameOURSTASH(pn) == ourstash
                && memEQ(PadnamePV(pn), PadnamePV(name), PadnameLEN(name)))
            {
                Perl_warner(aTHX_ packWARN(WARN_SHADOW),
                    "\"our\" variable %" PNf " redeclared", PNfARG(pn));
                if (off <= PL_comppad_name_floor)
                    Perl_warner(aTHX_ packWARN(WARN_SHADOW),
                        "\t(Did you mean \"local\" instead of \"our\"?)\n");
                break;
            }
            --off;
        }
    }
}


/*
=for apidoc pad_findmy_pvn

Given the name of a lexical variable, find its position in the
currently-compiling pad.
C<namepv>/C<namelen> specify the variable's name, including leading sigil.
C<flags> is reserved and must be zero.
If it is not in the current pad but appears in the pad of any lexically
enclosing scope, then a pseudo-entry for it is added in the current pad.
Returns the offset in the current pad,
or C<NOT_IN_PAD> if no such lexical is in scope.

=cut
*/

PADOFFSET
Perl_pad_findmy_pvn(pTHX_ const char *namepv, STRLEN namelen, U32 flags)
{
    PADNAME *out_pn;
    int out_flags;
    PADOFFSET offset;
    const PADNAMELIST *namelist;
    PADNAME **name_p;

    PERL_ARGS_ASSERT_PAD_FINDMY_PVN;

    pad_peg("pad_findmy_pvn");

    if (flags)
        Perl_croak(aTHX_ "panic: pad_findmy_pvn illegal flag bits 0x%" UVxf,
                   (UV)flags);

    /* compilation errors can zero PL_compcv */
    if (!PL_compcv)
        return NOT_IN_PAD;

    offset = pad_findlex(namepv, namelen, flags,
                PL_compcv, PL_cop_seqmax, 1, NULL, &out_pn, &out_flags);
    if (offset != NOT_IN_PAD)
        return offset;

    /* Skip the ‘our’ hack for subroutines, as the warning does not apply.
     */
    if (*namepv == '&') return NOT_IN_PAD;

    /* look for an our that's being introduced; this allows
     *    our $foo = 0 unless defined $foo;
     * to not give a warning. (Yes, this is a hack) */

    namelist = PadlistNAMES(CvPADLIST(PL_compcv));
    name_p = PadnamelistARRAY(namelist);
    for (offset = PadnamelistMAXNAMED(namelist); offset > 0; offset--) {
        const PADNAME * const name = name_p[offset];
        if (name && PadnameLEN(name) == namelen
            && !PadnameOUTER(name)
            && (PadnameIsOUR(name))
            && (  PadnamePV(name) == namepv
               || memEQ(PadnamePV(name), namepv, namelen)  )
            && COP_SEQ_RANGE_LOW(name) == PERL_PADSEQ_INTRO
        )
            return offset;
    }
    return NOT_IN_PAD;
}

/*
=for apidoc pad_findmy_pv

Exactly like L</pad_findmy_pvn>, but takes a nul-terminated string
instead of a string/length pair.

=cut
*/

PADOFFSET
Perl_pad_findmy_pv(pTHX_ const char *name, U32 flags)
{
    PERL_ARGS_ASSERT_PAD_FINDMY_PV;
    return pad_findmy_pvn(name, strlen(name), flags);
}

/*
=for apidoc pad_findmy_sv

Exactly like L</pad_findmy_pvn>, but takes the name string in the form
of an SV instead of a string/length pair.

=cut
*/

PADOFFSET
Perl_pad_findmy_sv(pTHX_ SV *name, U32 flags)
{
    char *namepv;
    STRLEN namelen;
    PERL_ARGS_ASSERT_PAD_FINDMY_SV;
    namepv = SvPVutf8(name, namelen);
    return pad_findmy_pvn(namepv, namelen, flags);
}

/*
=for apidoc find_rundefsv

Returns the global variable C<$_>.

=cut
*/

SV *
Perl_find_rundefsv(pTHX)
{
    return DEFSV;
}

/*
=for apidoc pad_findlex

Find a named lexical anywhere in a chain of nested pads.  Add fake entries
in the inner pads if it's found in an outer one.

Returns the offset in the bottom pad of the lex or the fake lex.
C<cv> is the CV in which to start the search, and seq is the current C<cop_seq>
to match against.  If C<warn> is true, print appropriate warnings.  The C<out_>*
vars return values, and so are pointers to where the returned values
should be stored.  C<out_capture>, if non-null, requests that the innermost
instance of the lexical is captured; C<out_name> is set to the innermost
matched pad name or fake pad name; C<out_flags> returns the flags normally
associated with the C<PARENT_FAKELEX_FLAGS> field of a fake pad name.

Note that C<pad_findlex()> is recursive; it recurses up the chain of CVs,
then comes back down, adding fake entries
as it goes.  It has to be this way
because fake names in anon prototypes have to store in C<xpadn_low> the
index into the parent pad.

=cut
*/

/* the CV has finished being compiled. This is not a sufficient test for
 * all CVs (eg XSUBs), but suffices for the CVs found in a lexical chain */
#define CvCOMPILED(cv)	CvROOT(cv)

/* the CV does late binding of its lexicals */
#define CvLATE(cv) (CvANON(cv) || CvCLONE(cv) || SvTYPE(cv) == SVt_PVFM)

static void
S_unavailable(pTHX_ PADNAME *name)
{
    /* diag_listed_as: Variable "%s" is not available */
    Perl_ck_warner(aTHX_ packWARN(WARN_CLOSURE),
                        "%s \"%" PNf "\" is not available",
                         *PadnamePV(name) == '&'
                                         ? "Subroutine"
                                         : "Variable",
                         PNfARG(name));
}

STATIC PADOFFSET
S_pad_findlex(pTHX_ const char *namepv, STRLEN namelen, U32 flags, const CV* cv, U32 seq,
        int warn, SV** out_capture, PADNAME** out_name, int *out_flags)
{
    PADOFFSET offset, new_offset;
    SV *new_capture;
    SV **new_capturep;
    const PADLIST * const padlist = CvPADLIST(cv);
    const bool staleok = cBOOL(flags & padadd_STALEOK);
    const bool fieldok = cBOOL(flags & padfind_FIELD_OK);

    PERL_ARGS_ASSERT_PAD_FINDLEX;

    flags &= ~(padadd_STALEOK|padfind_FIELD_OK); /* one-shot flags */
    if (flags)
        Perl_croak(aTHX_ "panic: pad_findlex illegal flag bits 0x%" UVxf,
                   (UV)flags);

    *out_flags = 0;

    DEBUG_Xv(PerlIO_printf(Perl_debug_log,
        "Pad findlex cv=0x%" UVxf " searching \"%.*s\" seq=%d%s\n",
                           PTR2UV(cv), (int)namelen, namepv, (int)seq,
        out_capture ? " capturing" : "" ));

    /* first, search this pad */

    if (padlist) { /* not an undef CV */
        PADOFFSET fake_offset = 0;
        const PADNAMELIST * const names = PadlistNAMES(padlist);
        PADNAME * const * const name_p = PadnamelistARRAY(names);

        for (offset = PadnamelistMAXNAMED(names); offset > 0; offset--) {
            const PADNAME * const name = name_p[offset];
            if (name && PadnameLEN(name) == namelen
                     && (  PadnamePV(name) == namepv
                        || memEQ(PadnamePV(name), namepv, namelen)  ))
            {
                if (PadnameOUTER(name)) {
                    fake_offset = offset; /* in case we don't find a real one */
                    continue;
                }
                if (PadnameIN_SCOPE(name, seq))
                    break;
            }
        }

        if (offset > 0 || fake_offset > 0 ) { /* a match! */
            if (offset > 0) { /* not fake */
                fake_offset = 0;
                *out_name = name_p[offset]; /* return the name */

                if (PadnameIsFIELD(*out_name) && !fieldok)
                    croak("Field %" SVf " is not accessible outside a method",
                            SVfARG(PadnameSV(*out_name)));

                /* set PAD_FAKELEX_MULTI if this lex can have multiple
                 * instances. For now, we just test !CvUNIQUE(cv), but
                 * ideally, we should detect my's declared within loops
                 * etc - this would allow a wider range of 'not stayed
                 * shared' warnings. We also treated already-compiled
                 * lexes as not multi as viewed from evals. */

                *out_flags = CvANON(cv) ?
                        PAD_FAKELEX_ANON :
                            (!CvUNIQUE(cv) && ! CvCOMPILED(cv))
                                ? PAD_FAKELEX_MULTI : 0;

                DEBUG_Xv(PerlIO_printf(Perl_debug_log,
                    "Pad findlex cv=0x%" UVxf " matched: offset=%ld (%lu,%lu)\n",
                    PTR2UV(cv), (long)offset,
                    (unsigned long)COP_SEQ_RANGE_LOW(*out_name),
                    (unsigned long)COP_SEQ_RANGE_HIGH(*out_name)));
            }
            else { /* fake match */
                offset = fake_offset;
                *out_name = name_p[offset]; /* return the name */
                *out_flags = PARENT_FAKELEX_FLAGS(*out_name);
                DEBUG_Xv(PerlIO_printf(Perl_debug_log,
                    "Pad findlex cv=0x%" UVxf " matched: offset=%ld flags=0x%lx index=%lu\n",
                    PTR2UV(cv), (long)offset, (unsigned long)*out_flags,
                    (unsigned long) PARENT_PAD_INDEX(*out_name) 
                ));
            }

            /* return the lex? */

            if (out_capture) {

                /* our ? */
                if (PadnameIsOUR(*out_name)) {
                    *out_capture = NULL;
                    return offset;
                }

                /* trying to capture from an anon prototype? */
                if (CvCOMPILED(cv)
                        ? CvANON(cv) && CvCLONE(cv) && !CvCLONED(cv)
                        : *out_flags & PAD_FAKELEX_ANON)
                {
                    if (warn)
                        S_unavailable(aTHX_
                                      *out_name);

                    *out_capture = NULL;
                }

                /* real value */
                else {
                    int newwarn = warn;
                    if (!CvCOMPILED(cv) && (*out_flags & PAD_FAKELEX_MULTI)
                         && !PadnameIsSTATE(name_p[offset])
                         && warn && ckWARN(WARN_CLOSURE)) {
                        newwarn = 0;
                        /* diag_listed_as: Variable "%s" will not stay
                                           shared */
                        Perl_warner(aTHX_ packWARN(WARN_CLOSURE),
                            "%s \"%" UTF8f "\" will not stay shared",
                             *namepv == '&' ? "Subroutine" : "Variable",
                             UTF8fARG(1, namelen, namepv));
                    }

                    if (fake_offset && CvANON(cv)
                            && CvCLONE(cv) &&!CvCLONED(cv))
                    {
                        PADNAME *n;
                        /* not yet caught - look further up */
                        DEBUG_Xv(PerlIO_printf(Perl_debug_log,
                            "Pad findlex cv=0x%" UVxf " chasing lex in outer pad\n",
                            PTR2UV(cv)));
                        n = *out_name;
                        (void) pad_findlex(namepv, namelen, flags, CvOUTSIDE(cv),
                            CvOUTSIDE_SEQ(cv),
                            newwarn, out_capture, out_name, out_flags);
                        *out_name = n;
                        return offset;
                    }

                    *out_capture = AvARRAY(PadlistARRAY(padlist)[
                                    CvDEPTH(cv) ? CvDEPTH(cv) : 1])[offset];
                    DEBUG_Xv(PerlIO_printf(Perl_debug_log,
                        "Pad findlex cv=0x%" UVxf " found lex=0x%" UVxf "\n",
                        PTR2UV(cv), PTR2UV(*out_capture)));

                    if (SvPADSTALE(*out_capture)
                        && (!CvDEPTH(cv) || !staleok)
                        && !PadnameIsSTATE(name_p[offset]))
                    {
                        S_unavailable(aTHX_
                                      name_p[offset]);
                        *out_capture = NULL;
                    }
                }
                if (!*out_capture) {
                    if (namelen != 0 && *namepv == '@')
                        *out_capture = newSV_type_mortal(SVt_PVAV);
                    else if (namelen != 0 && *namepv == '%')
                        *out_capture = newSV_type_mortal(SVt_PVHV);
                    else if (namelen != 0 && *namepv == '&')
                        *out_capture = newSV_type_mortal(SVt_PVCV);
                    else
                        *out_capture = newSV_type_mortal(SVt_NULL);
                }
            }

            return offset;
        }
    }

    /* it's not in this pad - try above */

    if (!CvOUTSIDE(cv))
        return NOT_IN_PAD;

    /* out_capture non-null means caller wants us to capture lex; in
     * addition we capture ourselves unless it's an ANON/format */
    new_capturep = out_capture ? out_capture :
                CvLATE(cv) ? NULL : &new_capture;

    U32 recurse_flags = flags;
    if(new_capturep == &new_capture)
        recurse_flags |= padadd_STALEOK;
    if(CvIsMETHOD(cv))
        recurse_flags |= padfind_FIELD_OK;

    offset = pad_findlex(namepv, namelen, recurse_flags,
                CvOUTSIDE(cv), CvOUTSIDE_SEQ(cv), 1,
                new_capturep, out_name, out_flags);
    if (offset == NOT_IN_PAD)
        return NOT_IN_PAD;

    if (PadnameIsFIELD(*out_name)) {
        HV *fieldstash = PadnameFIELDINFO(*out_name)->fieldstash;

        /* fields are only visible to the class that declared them */
        if(fieldstash != PL_curstash)
            croak("Field %" SVf " of %" HvNAMEf_QUOTEDPREFIX " is not accessible in a method of %" HvNAMEf_QUOTEDPREFIX,
                SVfARG(PadnameSV(*out_name)), HvNAMEfARG(fieldstash), HvNAMEfARG(PL_curstash));
    }

    /* found in an outer CV. Add appropriate fake entry to this pad */

    /* don't add new fake entries (via eval) to CVs that we have already
     * finished compiling, or to undef CVs */
    if (CvCOMPILED(cv) || !padlist)
        return 0; /* this dummy (and invalid) value isnt used by the caller */

    {
        PADNAME *new_name = newPADNAMEouter(*out_name);
        PADNAMELIST * const ocomppad_name = PL_comppad_name;
        PAD * const ocomppad = PL_comppad;
        PL_comppad_name = PadlistNAMES(padlist);
        PL_comppad = PadlistARRAY(padlist)[1];
        PL_curpad = AvARRAY(PL_comppad);

        new_offset
            = pad_alloc_name(new_name,
                              PadnameIsSTATE(*out_name) ? padadd_STATE : 0,
                              PadnameTYPE(*out_name),
                              PadnameOURSTASH(*out_name)
                              );

        DEBUG_Xv(PerlIO_printf(Perl_debug_log,
                               "Pad addname: %ld \"%.*s\" FAKE\n",
                               (long)new_offset,
                               (int) PadnameLEN(new_name),
                               PadnamePV(new_name)));
        PARENT_FAKELEX_FLAGS_set(new_name, *out_flags);

        PARENT_PAD_INDEX_set(new_name, 0);
        if (PadnameIsOUR(new_name)) {
            NOOP;   /* do nothing */
        }
        else if (CvLATE(cv)) {
            /* delayed creation - just note the offset within parent pad */
            PARENT_PAD_INDEX_set(new_name, offset);
            CvCLONE_on(cv);
        }
        else {
            /* immediate creation - capture outer value right now */
            av_store(PL_comppad, new_offset, SvREFCNT_inc(*new_capturep));
            /* But also note the offset, as newMYSUB needs it */
            PARENT_PAD_INDEX_set(new_name, offset);
            DEBUG_Xv(PerlIO_printf(Perl_debug_log,
                "Pad findlex cv=0x%" UVxf " saved captured sv 0x%" UVxf " at offset %ld\n",
                PTR2UV(cv), PTR2UV(*new_capturep), (long)new_offset));
        }
        *out_name = new_name;
        *out_flags = PARENT_FAKELEX_FLAGS(new_name);

        PL_comppad_name = ocomppad_name;
        PL_comppad = ocomppad;
        PL_curpad = ocomppad ? AvARRAY(ocomppad) : NULL;
    }
    return new_offset;
}

#ifdef DEBUGGING

/*
=for apidoc pad_sv

Get the value at offset C<po> in the current (compiling or executing) pad.
Use macro C<PAD_SV> instead of calling this function directly.

=cut
*/

SV *
Perl_pad_sv(pTHX_ PADOFFSET po)
{
    ASSERT_CURPAD_ACTIVE("pad_sv");

    if (!po)
        Perl_croak(aTHX_ "panic: pad_sv po");
    DEBUG_X(PerlIO_printf(Perl_debug_log,
        "Pad 0x%" UVxf "[0x%" UVxf "] sv:      %ld sv=0x%" UVxf "\n",
        PTR2UV(PL_comppad), PTR2UV(PL_curpad), (long)po, PTR2UV(PL_curpad[po]))
    );
    return PL_curpad[po];
}

/*
=for apidoc pad_setsv

Set the value at offset C<po> in the current (compiling or executing) pad.
Use the macro C<PAD_SETSV()> rather than calling this function directly.

=cut
*/

void
Perl_pad_setsv(pTHX_ PADOFFSET po, SV* sv)
{
    PERL_ARGS_ASSERT_PAD_SETSV;

    ASSERT_CURPAD_ACTIVE("pad_setsv");

    DEBUG_X(PerlIO_printf(Perl_debug_log,
        "Pad 0x%" UVxf "[0x%" UVxf "] setsv:   %ld sv=0x%" UVxf "\n",
        PTR2UV(PL_comppad), PTR2UV(PL_curpad), (long)po, PTR2UV(sv))
    );
    PL_curpad[po] = sv;
}

#endif /* DEBUGGING */

/*
=for apidoc pad_block_start

Update the pad compilation state variables on entry to a new block.

=cut
*/

void
Perl_pad_block_start(pTHX_ int full)
{
    ASSERT_CURPAD_ACTIVE("pad_block_start");
    SAVESTRLEN(PL_comppad_name_floor);
    PL_comppad_name_floor = PadnamelistMAX(PL_comppad_name);
    if (full)
        PL_comppad_name_fill = PL_comppad_name_floor;
    if (PL_comppad_name_floor < 0)
        PL_comppad_name_floor = 0;
    SAVESTRLEN(PL_min_intro_pending);
    SAVESTRLEN(PL_max_intro_pending);
    PL_min_intro_pending = 0;
    SAVESTRLEN(PL_comppad_name_fill);
    SAVESTRLEN(PL_padix_floor);
    /* PL_padix_floor is what PL_padix is reset to at the start of each
       statement, by pad_reset().  We set it when entering a new scope
       to keep things like this working:
            print "$foo$bar", do { this(); that() . "foo" };
       We must not let "$foo$bar" and the later concatenation share the
       same target.  */
    PL_padix_floor = PL_padix;
    PL_pad_reset_pending = FALSE;
}

/*
=for apidoc intro_my

"Introduce" C<my> variables to visible status.  This is called during parsing
at the end of each statement to make lexical variables visible to subsequent
statements.

=cut
*/

U32
Perl_intro_my(pTHX)
{
    PADNAME **svp;
    PADOFFSET i;
    U32 seq;

    ASSERT_CURPAD_ACTIVE("intro_my");
    if (PL_compiling.cop_seq) {
        seq = PL_compiling.cop_seq;
        PL_compiling.cop_seq = 0;
    }
    else
        seq = PL_cop_seqmax;
    if (! PL_min_intro_pending)
        return seq;

    svp = PadnamelistARRAY(PL_comppad_name);
    for (i = PL_min_intro_pending; i <= PL_max_intro_pending; i++) {
        PADNAME * const sv = svp[i];

        if (sv && PadnameLEN(sv) && !PadnameOUTER(sv)
            && COP_SEQ_RANGE_LOW(sv) == PERL_PADSEQ_INTRO)
        {
            COP_SEQ_RANGE_HIGH_set(sv, PERL_PADSEQ_INTRO); /* Don't know scope end yet. */
            COP_SEQ_RANGE_LOW_set(sv, PL_cop_seqmax);
            DEBUG_Xv(PerlIO_printf(Perl_debug_log,
                "Pad intromy: %ld \"%s\", (%lu,%lu)\n",
                (long)i, PadnamePV(sv),
                (unsigned long)COP_SEQ_RANGE_LOW(sv),
                (unsigned long)COP_SEQ_RANGE_HIGH(sv))
            );
        }
    }
    COP_SEQMAX_INC;
    PL_min_intro_pending = 0;
    PL_comppad_name_fill = PL_max_intro_pending; /* Needn't search higher */
    DEBUG_Xv(PerlIO_printf(Perl_debug_log,
                "Pad intromy: seq -> %ld\n", (long)(PL_cop_seqmax)));

    return seq;
}

/*
=for apidoc pad_leavemy

Cleanup at end of scope during compilation: set the max seq number for
lexicals in this scope and warn of any lexicals that never got introduced.

=cut
*/

OP *
Perl_pad_leavemy(pTHX)
{
    PADOFFSET off;
    OP *o = NULL;
    PADNAME * const * const svp = PadnamelistARRAY(PL_comppad_name);

    PL_pad_reset_pending = FALSE;

    ASSERT_CURPAD_ACTIVE("pad_leavemy");
    if (PL_min_intro_pending && PL_comppad_name_fill < PL_min_intro_pending) {
        for (off = PL_max_intro_pending; off >= PL_min_intro_pending; off--) {
            const PADNAME * const name = svp[off];
            if (name && PadnameLEN(name) && !PadnameOUTER(name))
                Perl_ck_warner_d(aTHX_ packWARN(WARN_INTERNAL),
                                      "%" PNf " never introduced",
                                       PNfARG(name));
        }
    }
    /* "Deintroduce" my variables that are leaving with this scope. */
    for (off = PadnamelistMAX(PL_comppad_name);
         off > PL_comppad_name_fill; off--) {
        PADNAME * const sv = svp[off];
        if (sv && PadnameLEN(sv) && !PadnameOUTER(sv)
            && COP_SEQ_RANGE_HIGH(sv) == PERL_PADSEQ_INTRO)
        {
            COP_SEQ_RANGE_HIGH_set(sv, PL_cop_seqmax);
            DEBUG_Xv(PerlIO_printf(Perl_debug_log,
                "Pad leavemy: %ld \"%s\", (%lu,%lu)\n",
                (long)off, PadnamePV(sv),
                (unsigned long)COP_SEQ_RANGE_LOW(sv),
                (unsigned long)COP_SEQ_RANGE_HIGH(sv))
            );
            if (!PadnameIsSTATE(sv) && !PadnameIsOUR(sv)
             && *PadnamePV(sv) == '&' && PadnameLEN(sv) > 1) {
                OP *kid = newOP(OP_INTROCV, 0);
                kid->op_targ = off;
                o = op_prepend_elem(OP_LINESEQ, kid, o);
            }
        }
    }
    COP_SEQMAX_INC;
    DEBUG_Xv(PerlIO_printf(Perl_debug_log,
            "Pad leavemy: seq = %ld\n", (long)PL_cop_seqmax));
    return o;
}

/*
=for apidoc pad_swipe

Abandon the tmp in the current pad at offset C<po> and replace with a
new one.

=cut
*/

void
Perl_pad_swipe(pTHX_ PADOFFSET po, bool refadjust)
{
    ASSERT_CURPAD_LEGAL("pad_swipe");
    if (!PL_curpad)
        return;
    if (AvARRAY(PL_comppad) != PL_curpad)
        Perl_croak(aTHX_ "panic: pad_swipe curpad, %p!=%p",
                   AvARRAY(PL_comppad), PL_curpad);
    if (!po || ((SSize_t)po) > AvFILLp(PL_comppad))
        Perl_croak(aTHX_ "panic: pad_swipe po=%ld, fill=%ld",
                   (long)po, (long)AvFILLp(PL_comppad));

    DEBUG_X(PerlIO_printf(Perl_debug_log,
                "Pad 0x%" UVxf "[0x%" UVxf "] swipe:   %ld\n",
                PTR2UV(PL_comppad), PTR2UV(PL_curpad), (long)po));

    if (refadjust)
        SvREFCNT_dec(PL_curpad[po]);


    /* if pad tmps aren't shared between ops, then there's no need to
     * create a new tmp when an existing op is freed */
#ifdef USE_PAD_RESET
    PL_curpad[po] = newSV_type(SVt_NULL);
    SvPADTMP_on(PL_curpad[po]);
#else
    PL_curpad[po] = NULL;
#endif
    if (PadnamelistMAX(PL_comppad_name) != -1
     && (PADOFFSET)PadnamelistMAX(PL_comppad_name) >= po) {
        if (PadnamelistARRAY(PL_comppad_name)[po]) {
            assert(!PadnameLEN(PadnamelistARRAY(PL_comppad_name)[po]));
        }
        PadnamelistARRAY(PL_comppad_name)[po] = &PL_padname_undef;
    }
    /* Use PL_constpadix here, not PL_padix.  The latter may have been
       reset by pad_reset.  We don’t want pad_alloc to have to scan the
       whole pad when allocating a constant. */
    if (po < PL_constpadix)
        PL_constpadix = po - 1;
}

/*
=for apidoc pad_reset

Mark all the current temporaries for reuse

=cut
*/

/* pad_reset() causes pad temp TARGs (operator targets) to be shared
 * between OPs from different statements.  During compilation, at the start
 * of each statement pad_reset resets PL_padix back to its previous value.
 * When allocating a target, pad_alloc begins its scan through the pad at
 * PL_padix+1.  */
static void
S_pad_reset(pTHX)
{
#ifdef USE_PAD_RESET
    if (AvARRAY(PL_comppad) != PL_curpad)
        Perl_croak(aTHX_ "panic: pad_reset curpad, %p!=%p",
                   AvARRAY(PL_comppad), PL_curpad);

    DEBUG_X(PerlIO_printf(Perl_debug_log,
            "Pad 0x%" UVxf "[0x%" UVxf "] reset:     padix %ld -> %ld",
            PTR2UV(PL_comppad), PTR2UV(PL_curpad),
                (long)PL_padix, (long)PL_padix_floor
            )
    );

    if (!TAINTING_get) {	/* Can't mix tainted and non-tainted temporaries. */
        PL_padix = PL_padix_floor;
    }
#endif
    PL_pad_reset_pending = FALSE;
}

/*
=for apidoc pad_tidy

Tidy up a pad at the end of compilation of the code to which it belongs.
Jobs performed here are: remove most stuff from the pads of anonsub
prototypes; give it a C<@_>; mark temporaries as such.  C<type> indicates
the kind of subroutine:

    padtidy_SUB        ordinary subroutine
    padtidy_SUBCLONE   prototype for lexical closure
    padtidy_FORMAT     format

=cut
*/

void
Perl_pad_tidy(pTHX_ padtidy_type type)
{

    ASSERT_CURPAD_ACTIVE("pad_tidy");

    /* If this CV has had any 'eval-capable' ops planted in it:
     * i.e. it contains any of:
     *
     *     * eval '...',
     *     * //ee,
     *     * use re 'eval'; /$var/
     *     * /(?{..})/),
     *
     * Then any anon prototypes in the chain of CVs should be marked as
     * cloneable, so that for example the eval's CV in
     *
     *    sub { eval '$x' }
     *
     * gets the right CvOUTSIDE.  If running with -d, *any* sub may
     * potentially have an eval executed within it.
     */

    if (PL_cv_has_eval || PL_perldb) {
        const CV *cv;
        for (cv = PL_compcv ;cv; cv = CvOUTSIDE(cv)) {
            if (cv != PL_compcv && CvCOMPILED(cv))
                break; /* no need to mark already-compiled code */
            if (CvANON(cv)) {
                DEBUG_Xv(PerlIO_printf(Perl_debug_log,
                    "Pad clone on cv=0x%" UVxf "\n", PTR2UV(cv)));
                CvCLONE_on(cv);
            }
            CvHASEVAL_on(cv);
        }
    }

    /* extend namepad to match curpad */
    if (PadnamelistMAX(PL_comppad_name) < AvFILLp(PL_comppad))
        padnamelist_store(PL_comppad_name, AvFILLp(PL_comppad), NULL);

    if (type == padtidy_SUBCLONE) {
        PADNAME ** const namep = PadnamelistARRAY(PL_comppad_name);
        PADOFFSET ix;

        for (ix = AvFILLp(PL_comppad); ix > 0; ix--) {
            PADNAME *namesv;
            if (!namep[ix]) namep[ix] = &PL_padname_undef;

            /*
             * The only things that a clonable function needs in its
             * pad are anonymous subs, constants and GVs.
             * The rest are created anew during cloning.
             */
            if (!PL_curpad[ix] || SvIMMORTAL(PL_curpad[ix]))
                continue;
            namesv = namep[ix];
            if (!(PadnamePV(namesv) &&
                   (!PadnameLEN(namesv) || *PadnamePV(namesv) == '&')))
            {
                SvREFCNT_dec(PL_curpad[ix]);
                PL_curpad[ix] = NULL;
            }
        }
    }
    else if (type == padtidy_SUB) {
        AV * const av = newAV();			/* Will be @_ */
        av_store(PL_comppad, 0, MUTABLE_SV(av));
        AvREIFY_only(av);
    }

    if (type == padtidy_SUB || type == padtidy_FORMAT) {
        PADNAME ** const namep = PadnamelistARRAY(PL_comppad_name);
        PADOFFSET ix;
        for (ix = AvFILLp(PL_comppad); ix > 0; ix--) {
            if (!namep[ix]) namep[ix] = &PL_padname_undef;
            if (!PL_curpad[ix] || SvIMMORTAL(PL_curpad[ix]))
                continue;
            if (SvPADMY(PL_curpad[ix]) && !PadnameOUTER(namep[ix])) {
                /* This is a work around for how the current implementation of
                   ?{ } blocks in regexps interacts with lexicals.

                   One of our lexicals.
                   Can't do this on all lexicals, otherwise sub baz() won't
                   compile in

                   my $foo;

                   sub bar { ++$foo; }

                   sub baz { ++$foo; }

                   because completion of compiling &bar calling pad_tidy()
                   would cause (top level) $foo to be marked as stale, and
                   "no longer available".  */
                SvPADSTALE_on(PL_curpad[ix]);
            }
        }
    }
    PL_curpad = AvARRAY(PL_comppad);
}

/*
=for apidoc pad_free

Free the SV at offset po in the current pad.

=cut
*/

void
Perl_pad_free(pTHX_ PADOFFSET po)
{
#ifndef USE_PAD_RESET
    SV *sv;
#endif
    ASSERT_CURPAD_LEGAL("pad_free");
    if (!PL_curpad)
        return;
    if (AvARRAY(PL_comppad) != PL_curpad)
        Perl_croak(aTHX_ "panic: pad_free curpad, %p!=%p",
                   AvARRAY(PL_comppad), PL_curpad);
    if (!po)
        Perl_croak(aTHX_ "panic: pad_free po");

    DEBUG_X(PerlIO_printf(Perl_debug_log,
            "Pad 0x%" UVxf "[0x%" UVxf "] free:    %ld\n",
            PTR2UV(PL_comppad), PTR2UV(PL_curpad), (long)po)
    );

#ifndef USE_PAD_RESET
    sv = PL_curpad[po];
    if (sv && sv != &PL_sv_undef && !SvPADMY(sv))
        SvFLAGS(sv) &= ~SVs_PADTMP;

    if (po < PL_padix)
        PL_padix = po - 1;
#endif
}

/*
=for apidoc do_dump_pad

Dump the contents of a padlist

=cut
*/

void
Perl_do_dump_pad(pTHX_ I32 level, PerlIO *file, PADLIST *padlist, int full)
{
    const PADNAMELIST *pad_name;
    const AV *pad;
    PADNAME **pname;
    SV **ppad;
    PADOFFSET ix;

    PERL_ARGS_ASSERT_DO_DUMP_PAD;

    if (!padlist) {
        return;
    }
    pad_name = PadlistNAMES(padlist);
    pad = PadlistARRAY(padlist)[1];
    pname = PadnamelistARRAY(pad_name);
    ppad = AvARRAY(pad);
    Perl_dump_indent(aTHX_ level, file,
            "PADNAME = 0x%" UVxf "(0x%" UVxf ") PAD = 0x%" UVxf "(0x%" UVxf ")\n",
            PTR2UV(pad_name), PTR2UV(pname), PTR2UV(pad), PTR2UV(ppad)
    );

    for (ix = 1; ix <= PadnamelistMAX(pad_name); ix++) {
        const PADNAME *namesv = pname[ix];
        if (namesv && !PadnameLEN(namesv)) {
            namesv = NULL;
        }
        if (namesv) {
            if (PadnameOUTER(namesv))
                Perl_dump_indent(aTHX_ level+1, file,
                    "%2d. 0x%" UVxf "<%lu> FAKE \"%s\" flags=0x%lx index=%lu\n",
                    (int) ix,
                    PTR2UV(ppad[ix]),
                    (unsigned long) (ppad[ix] ? SvREFCNT(ppad[ix]) : 0),
                    PadnamePV(namesv),
                    (unsigned long)PARENT_FAKELEX_FLAGS(namesv),
                    (unsigned long)PARENT_PAD_INDEX(namesv)

                );
            else
                Perl_dump_indent(aTHX_ level+1, file,
                    "%2d. 0x%" UVxf "<%lu> (%lu,%lu) \"%s\"\n",
                    (int) ix,
                    PTR2UV(ppad[ix]),
                    (unsigned long) (ppad[ix] ? SvREFCNT(ppad[ix]) : 0),
                    (unsigned long)COP_SEQ_RANGE_LOW(namesv),
                    (unsigned long)COP_SEQ_RANGE_HIGH(namesv),
                    PadnamePV(namesv)
                );
        }
        else if (full) {
            Perl_dump_indent(aTHX_ level+1, file,
                "%2d. 0x%" UVxf "<%lu>\n",
                (int) ix,
                PTR2UV(ppad[ix]),
                (unsigned long) (ppad[ix] ? SvREFCNT(ppad[ix]) : 0)
            );
        }
    }
}

#ifdef DEBUGGING

/*
=for apidoc cv_dump

dump the contents of a CV

=cut
*/

STATIC void
S_cv_dump(pTHX_ const CV *cv, const char *title)
{
    const CV * const outside = CvOUTSIDE(cv);

    PERL_ARGS_ASSERT_CV_DUMP;

    PerlIO_printf(Perl_debug_log,
                  "  %s: CV=0x%" UVxf " (%s), OUTSIDE=0x%" UVxf " (%s)\n",
                  title,
                  PTR2UV(cv),
                  (CvANON(cv) ? "ANON"
                   : (SvTYPE(cv) == SVt_PVFM) ? "FORMAT"
                   : (cv == PL_main_cv) ? "MAIN"
                   : CvUNIQUE(cv) ? "UNIQUE"
                   : CvGV(cv) ? GvNAME(CvGV(cv)) : "UNDEFINED"),
                  PTR2UV(outside),
                  (!outside ? "null"
                   : CvANON(outside) ? "ANON"
                   : (outside == PL_main_cv) ? "MAIN"
                   : CvUNIQUE(outside) ? "UNIQUE"
                   : CvGV(outside) ? GvNAME(CvGV(outside)) : "UNDEFINED"));

    if (!CvISXSUB(cv)) {
        /* SVPADLIST(cv) will fail an assert if CvISXSUB(cv) is true,
         * and if the assert is removed this code will SEGV. XSUBs don't
         * have padlists I believe - Yves */
        PADLIST* const padlist = CvPADLIST(cv);
        PerlIO_printf(Perl_debug_log,
                    "    PADLIST = 0x%" UVxf "\n", PTR2UV(padlist));
        do_dump_pad(1, Perl_debug_log, padlist, 1);
    }
}

#endif /* DEBUGGING */

/*
=for apidoc cv_clone

Clone a CV, making a lexical closure.  C<proto> supplies the prototype
of the function: its code, pad structure, and other attributes.
The prototype is combined with a capture of outer lexicals to which the
code refers, which are taken from the currently-executing instance of
the immediately surrounding code.

=cut
*/

static CV *S_cv_clone(pTHX_ CV *proto, CV *cv, CV *outside, HV *cloned);

static CV *
S_cv_clone_pad(pTHX_ CV *proto, CV *cv, CV *outside, HV *cloned,
                     bool newcv)
{
    PADOFFSET ix;
    PADLIST* const protopadlist = CvPADLIST(proto);
    PADNAMELIST *const protopad_name = PadlistNAMES(protopadlist);
    const PAD *const protopad = PadlistARRAY(protopadlist)[1];
    PADNAME** const pname = PadnamelistARRAY(protopad_name);
    SV** const ppad = AvARRAY(protopad);
    const PADOFFSET fname = PadnamelistMAX(protopad_name);
    const PADOFFSET fpad = AvFILLp(protopad);
    SV** outpad;
    long depth;
    U32 subclones = 0;
    bool trouble = FALSE;

    assert(!CvUNIQUE(proto));

    /* Anonymous subs have a weak CvOUTSIDE pointer, so its value is not
     * reliable.  The currently-running sub is always the one we need to
     * close over.
     * For my subs, the currently-running sub may not be the one we want.
     * We have to check whether it is a clone of CvOUTSIDE.
     * Note that in general for formats, CvOUTSIDE != find_runcv.
     * Since formats may be nested inside closures, CvOUTSIDE may point
     * to a prototype; we instead want the cloned parent who called us.
     */

    if (!outside) {
      if (CvWEAKOUTSIDE(proto))
        outside = find_runcv(NULL);
      else {
        outside = CvOUTSIDE(proto);
        if ((CvCLONE(outside) && ! CvCLONED(outside))
            || !CvPADLIST(outside)
            || CvPADLIST(outside)->xpadl_id != protopadlist->xpadl_outid) {
            outside = find_runcv_where(
                FIND_RUNCV_padid_eq, PTR2IV(protopadlist->xpadl_outid), NULL
            );
            /* outside could be null */
        }
      }
    }
    depth = outside ? CvDEPTH(outside) : 0;
    if (!depth)
        depth = 1;

    ENTER;
    SAVESPTR(PL_compcv);
    PL_compcv = cv;
    if (newcv) SAVEFREESV(cv); /* in case of fatal warnings */

    if (CvHASEVAL(cv))
        CvOUTSIDE(cv)	= MUTABLE_CV(SvREFCNT_inc_simple(outside));

    SAVESPTR(PL_comppad_name);
    PL_comppad_name = protopad_name;
    CvPADLIST_set(cv, pad_new(padnew_CLONE|padnew_SAVE));
    CvPADLIST(cv)->xpadl_id = protopadlist->xpadl_id;

    av_fill(PL_comppad, fpad);

    PL_curpad = AvARRAY(PL_comppad);

    outpad = outside && CvPADLIST(outside)
        ? AvARRAY(PadlistARRAY(CvPADLIST(outside))[depth])
        : NULL;
    if (outpad) CvPADLIST(cv)->xpadl_outid = CvPADLIST(outside)->xpadl_id;

    for (ix = fpad; ix > 0; ix--) {
        PADNAME* const namesv = (ix <= fname) ? pname[ix] : NULL;
        SV *sv = NULL;
        if (namesv && PadnameLEN(namesv)) { /* lexical */
          if (PadnameIsOUR(namesv)) { /* or maybe not so lexical */
                NOOP;
          }
          else {
            if (PadnameOUTER(namesv)) {   /* lexical from outside? */
                /* formats may have an inactive, or even undefined, parent;
                   but state vars are always available. */
                if (!outpad || !(sv = outpad[PARENT_PAD_INDEX(namesv)])
                 || (  SvPADSTALE(sv) && !PadnameIsSTATE(namesv)
                    && (!outside || !CvDEPTH(outside)))  ) {
                    S_unavailable(aTHX_ namesv);
                    sv = NULL;
                }
                else 
                    SvREFCNT_inc_simple_void_NN(sv);
            }
            if (!sv) {
                const char sigil = PadnamePV(namesv)[0];
                if (sigil == '&')
                    /* If there are state subs, we need to clone them, too.
                       But they may need to close over variables we have
                       not cloned yet.  So we will have to do a second
                       pass.  Furthermore, there may be state subs clos-
                       ing over other state subs’ entries, so we have
                       to put a stub here and then clone into it on the
                       second pass. */
                    if (PadnameIsSTATE(namesv) && !CvCLONED(ppad[ix])) {
                        assert(SvTYPE(ppad[ix]) == SVt_PVCV);
                        subclones ++;
                        if (CvOUTSIDE(ppad[ix]) != proto)
                             trouble = TRUE;
                        sv = newSV_type(SVt_PVCV);
                        CvLEXICAL_on(sv);
                    }
                    else if (PadnameLEN(namesv)>1 && !PadnameIsOUR(namesv))
                    {
                        /* my sub */
                        /* Just provide a stub, but name it.  It will be
                           upgraded to the real thing on scope entry. */
                        U32 hash;
                        PERL_HASH(hash, PadnamePV(namesv)+1,
                                  PadnameLEN(namesv) - 1);
                        sv = newSV_type(SVt_PVCV);
                        CvNAME_HEK_set(
                            sv,
                            share_hek(PadnamePV(namesv)+1,
                                      1 - PadnameLEN(namesv),
                                      hash)
                        );
                        CvLEXICAL_on(sv);
                    }
                    else sv = SvREFCNT_inc(ppad[ix]);
                else if (sigil == '@')
                    sv = MUTABLE_SV(newAV());
                else if (sigil == '%')
                    sv = MUTABLE_SV(newHV());
                else
                    sv = newSV_type(SVt_NULL);
                /* reset the 'assign only once' flag on each state var */
                if (sigil != '&' && PadnameIsSTATE(namesv))
                    SvPADSTALE_on(sv);
            }
          }
        }
        else if (namesv && PadnamePV(namesv)) {
            sv = SvREFCNT_inc_NN(ppad[ix]);
        }
        else {
            sv = newSV_type(SVt_NULL);
            SvPADTMP_on(sv);
        }
        PL_curpad[ix] = sv;
    }

    if (subclones)
    {
        if (trouble || cloned) {
            /* Uh-oh, we have trouble!  At least one of the state subs here
               has its CvOUTSIDE pointer pointing somewhere unexpected.  It
               could be pointing to another state protosub that we are
               about to clone.  So we have to track which sub clones come
               from which protosubs.  If the CvOUTSIDE pointer for a parti-
               cular sub points to something we have not cloned yet, we
               delay cloning it.  We must loop through the pad entries,
               until we get a full pass with no cloning.  If any uncloned
               subs remain (probably nested inside anonymous or ‘my’ subs),
               then they get cloned in a final pass.
             */
            bool cloned_in_this_pass;
            if (!cloned)
                cloned = (HV *)newSV_type_mortal(SVt_PVHV);
            do {
                cloned_in_this_pass = FALSE;
                for (ix = fpad; ix > 0; ix--) {
                    PADNAME * const name =
                        (ix <= fname) ? pname[ix] : NULL;
                    if (name && name != &PL_padname_undef
                     && !PadnameOUTER(name) && PadnamePV(name)[0] == '&'
                     && PadnameIsSTATE(name) && !CvCLONED(PL_curpad[ix]))
                    {
                        CV * const protokey = CvOUTSIDE(ppad[ix]);
                        CV ** const cvp = protokey == proto
                            ? &cv
                            : (CV **)hv_fetch(cloned, (char *)&protokey,
                                              sizeof(CV *), 0);
                        if (cvp && *cvp) {
                            S_cv_clone(aTHX_ (CV *)ppad[ix],
                                             (CV *)PL_curpad[ix],
                                             *cvp, cloned);
                            (void)hv_store(cloned, (char *)&ppad[ix],
                                     sizeof(CV *),
                                     SvREFCNT_inc_simple_NN(PL_curpad[ix]),
                                     0);
                            subclones--;
                            cloned_in_this_pass = TRUE;
                        }
                    }
                }
            } while (cloned_in_this_pass);
            if (subclones)
                for (ix = fpad; ix > 0; ix--) {
                    PADNAME * const name =
                        (ix <= fname) ? pname[ix] : NULL;
                    if (name && name != &PL_padname_undef
                     && !PadnameOUTER(name) && PadnamePV(name)[0] == '&'
                     && PadnameIsSTATE(name) && !CvCLONED(PL_curpad[ix]))
                        S_cv_clone(aTHX_ (CV *)ppad[ix],
                                         (CV *)PL_curpad[ix],
                                         CvOUTSIDE(ppad[ix]), cloned);
                }
        }
        else for (ix = fpad; ix > 0; ix--) {
            PADNAME * const name = (ix <= fname) ? pname[ix] : NULL;
            if (name && name != &PL_padname_undef && !PadnameOUTER(name)
             && PadnamePV(name)[0] == '&' && PadnameIsSTATE(name))
                S_cv_clone(aTHX_ (CV *)ppad[ix], (CV *)PL_curpad[ix], cv,
                                 NULL);
        }
    }

    if (newcv) SvREFCNT_inc_simple_void_NN(cv);
    LEAVE;

    if (CvCONST(cv)) {
        /* Constant sub () { $x } closing over $x:
         * The prototype was marked as a candidate for const-ization,
         * so try to grab the current const value, and if successful,
         * turn into a const sub:
         */
        SV* const_sv;
        OP *o = CvSTART(cv);
        assert(newcv);
        for (; o; o = o->op_next)
            if (o->op_type == OP_PADSV)
                break;
        ASSUME(o->op_type == OP_PADSV);
        const_sv = PAD_BASE_SV(CvPADLIST(cv), o->op_targ);
        /* the candidate should have 1 ref from this pad and 1 ref
         * from the parent */
        if (const_sv && SvREFCNT(const_sv) == 2) {
            const bool was_method = cBOOL(CvNOWARN_AMBIGUOUS(cv));
            if (outside) {
                PADNAME * const pn =
                    PadlistNAMESARRAY(CvPADLIST(outside))
                        [PARENT_PAD_INDEX(PadlistNAMESARRAY(
                            CvPADLIST(cv))[o->op_targ])];
                assert(PadnameOUTER(PadlistNAMESARRAY(CvPADLIST(cv))
                                        [o->op_targ]));
                if (PadnameLVALUE(pn)) {
                    /* We have a lexical that is potentially modifiable
                       elsewhere, so making a constant will break clo-
                       sure behaviour.  If this is a ‘simple lexical
                       op tree’, i.e., sub(){$x}, emit a deprecation
                       warning, but continue to exhibit the old behav-
                       iour of making it a constant based on the ref-
                       count of the candidate variable.

                       A simple lexical op tree looks like this:

                         leavesub
                           lineseq
                             nextstate
                             padsv
                     */
                    if (OpSIBLING(
                         cUNOPx(cUNOPx(CvROOT(cv))->op_first)->op_first
                        ) == o
                     && !OpSIBLING(o))
                    {
                        Perl_croak(aTHX_
                            "Constants from lexical variables potentially modified "
                            "elsewhere are no longer permitted");
                    }
                    else
                        goto constoff;
                }
            }
            SvREFCNT_inc_simple_void_NN(const_sv);
            /* If the lexical is not used elsewhere, it is safe to turn on
               SvPADTMP, since it is only when it is used in lvalue con-
               text that the difference is observable.  */
            SvREADONLY_on(const_sv);
            SvPADTMP_on(const_sv);
            SvREFCNT_dec_NN(cv);
            cv = newCONSTSUB(CvSTASH(proto), NULL, const_sv);
            if (was_method)
                CvNOWARN_AMBIGUOUS_on(cv);
        }
        else {
          constoff:
            CvCONST_off(cv);
        }
    }

    return cv;
}

static CV *
S_cv_clone(pTHX_ CV *proto, CV *cv, CV *outside, HV *cloned)
{
    const bool newcv = !cv;

    assert(!CvUNIQUE(proto));

    if (!cv) cv = MUTABLE_CV(newSV_type(SvTYPE(proto)));
    CvFLAGS(cv) = CvFLAGS(proto) & ~(CVf_CLONE|CVf_WEAKOUTSIDE|CVf_CVGV_RC
                                    |CVf_SLABBED);
    CvCLONED_on(cv);

    CvFILE(cv)		= CvDYNFILE(proto) ? savepv(CvFILE(proto))
                                           : CvFILE(proto);
    if (CvNAMED(proto))
         CvNAME_HEK_set(cv, share_hek_hek(CvNAME_HEK(proto)));
    else CvGV_set(cv,CvGV(proto));
    CvSTASH_set(cv, CvSTASH(proto));

    /* It is unlikely that proto is an xsub, but it could happen; e.g. if a
     * module has performed a lexical sub import trick on an xsub. This
     * happens with builtin::import, for example
     */
    if (UNLIKELY(CvISXSUB(proto))) {
        CvXSUB(cv)    = CvXSUB(proto);
        CvXSUBANY(cv) = CvXSUBANY(proto);
        if (CvREFCOUNTED_ANYSV(cv))
            SvREFCNT_inc(CvXSUBANY(cv).any_sv);
    }
    else {
        OP_REFCNT_LOCK;
        CvROOT(cv) = OpREFCNT_inc(CvROOT(proto));
        OP_REFCNT_UNLOCK;
        CvSTART(cv) = CvSTART(proto);
        CvOUTSIDE_SEQ(cv) = CvOUTSIDE_SEQ(proto);
    }

    if (SvPOK(proto)) {
        sv_setpvn(MUTABLE_SV(cv), SvPVX_const(proto), SvCUR(proto));
        if (SvUTF8(proto))
           SvUTF8_on(MUTABLE_SV(cv));
    }
    if (SvMAGIC(proto))
        mg_copy((SV *)proto, (SV *)cv, 0, 0);

    if (!CvISXSUB(proto) && CvPADLIST(proto))
        cv = S_cv_clone_pad(aTHX_ proto, cv, outside, cloned, newcv);

    DEBUG_Xv(
        PerlIO_printf(Perl_debug_log, "\nPad CV clone\n");
        if (CvOUTSIDE(cv)) cv_dump(CvOUTSIDE(cv), "Outside");
        cv_dump(proto,	 "Proto");
        cv_dump(cv,	 "To");
    );

    return cv;
}

CV *
Perl_cv_clone(pTHX_ CV *proto)
{
    PERL_ARGS_ASSERT_CV_CLONE;

    if (!CvPADLIST(proto)) Perl_croak(aTHX_ "panic: no pad in cv_clone");
    return S_cv_clone(aTHX_ proto, NULL, NULL, NULL);
}

/* Called only by pp_clonecv */
CV *
Perl_cv_clone_into(pTHX_ CV *proto, CV *target)
{
    PERL_ARGS_ASSERT_CV_CLONE_INTO;
    cv_undef(target);
    return S_cv_clone(aTHX_ proto, target, NULL, NULL);
}

/*
=for apidoc cv_name

Returns an SV containing the name of the CV, mainly for use in error
reporting.  The CV may actually be a GV instead, in which case the returned
SV holds the GV's name.  Anything other than a GV or CV is treated as a
string already holding the sub name, but this could change in the future.

An SV may be passed as a second argument.  If so, the name will be assigned
to it and it will be returned.  Otherwise the returned SV will be a new
mortal.

If C<flags> has the C<CV_NAME_NOTQUAL> bit set, then the package name will not be
included.  If the first argument is neither a CV nor a GV, this flag is
ignored (subject to change).

=for apidoc Amnh||CV_NAME_NOTQUAL

=cut
*/

SV *
Perl_cv_name(pTHX_ CV *cv, SV *sv, U32 flags)
{
    PERL_ARGS_ASSERT_CV_NAME;
    if (!isGV_with_GP(cv) && SvTYPE(cv) != SVt_PVCV) {
        if (sv) sv_setsv(sv,(SV *)cv);
        return sv ? (sv) : (SV *)cv;
    }
    {
        SV * const retsv = sv ? (sv) : sv_newmortal();
        if (SvTYPE(cv) == SVt_PVCV) {
            if (CvNAMED(cv)) {
                if (CvLEXICAL(cv) || flags & CV_NAME_NOTQUAL)
                    sv_sethek(retsv, CvNAME_HEK(cv));
                else {
                    if (CvSTASH(cv) && HvNAME_HEK(CvSTASH(cv)))
                        sv_sethek(retsv, HvNAME_HEK(CvSTASH(cv)));
                    else
                        sv_setpvs(retsv, "__ANON__");
                    sv_catpvs(retsv, "::");
                    sv_cathek(retsv, CvNAME_HEK(cv));
                }
            }
            else if (CvLEXICAL(cv) || flags & CV_NAME_NOTQUAL)
                sv_sethek(retsv, GvNAME_HEK(GvEGV(CvGV(cv))));
            else gv_efullname3(retsv, CvGV(cv), NULL);
        }
        else if (flags & CV_NAME_NOTQUAL) sv_sethek(retsv, GvNAME_HEK(cv));
        else gv_efullname3(retsv,(GV *)cv,NULL);
        return retsv;
    }
}

/*
=for apidoc pad_fixup_inner_anons

For any anon CVs in the pad, change C<CvOUTSIDE> of that CV from
C<old_cv> to C<new_cv> if necessary.  Needed when a newly-compiled CV has to be
moved to a pre-existing CV struct.

=cut
*/

void
Perl_pad_fixup_inner_anons(pTHX_ PADLIST *padlist, CV *old_cv, CV *new_cv)
{
    PADOFFSET ix;
    PADNAMELIST * const comppad_name = PadlistNAMES(padlist);
    AV * const comppad = PadlistARRAY(padlist)[1];
    PADNAME ** const namepad = PadnamelistARRAY(comppad_name);
    SV ** const curpad = AvARRAY(comppad);

    PERL_ARGS_ASSERT_PAD_FIXUP_INNER_ANONS;
    PERL_UNUSED_ARG(old_cv);

    for (ix = PadnamelistMAX(comppad_name); ix > 0; ix--) {
        const PADNAME *name = namepad[ix];
        if (name && name != &PL_padname_undef && !PadnameIsOUR(name)
            && *PadnamePV(name) == '&')
        {
          CV *innercv = MUTABLE_CV(curpad[ix]);
          if (UNLIKELY(PadnameOUTER(name))) {
            CV *cv = new_cv;
            PADNAME **names = namepad;
            PADOFFSET i = ix;
            while (PadnameOUTER(name)) {
                assert(SvTYPE(cv) == SVt_PVCV);
                cv = CvOUTSIDE(cv);
                names = PadlistNAMESARRAY(CvPADLIST(cv));
                i = PARENT_PAD_INDEX(name);
                name = names[i];
            }
            innercv = (CV *)PadARRAY(PadlistARRAY(CvPADLIST(cv))[1])[i];
          }
          if (SvTYPE(innercv) == SVt_PVCV) {
            /* XXX 0afba48f added code here to check for a proto CV
                   attached to the pad entry by magic.  But shortly there-
                   after 81df9f6f95 moved the magic to the pad name.  The
                   code here was never updated, so it wasn’t doing anything
                   and got deleted when PADNAME became a distinct type.  Is
                   there any bug as a result?  */
            if (CvOUTSIDE(innercv) == old_cv) {
                if (!CvWEAKOUTSIDE(innercv)) {
                    SvREFCNT_dec(old_cv);
                    SvREFCNT_inc_simple_void_NN(new_cv);
                }
                CvOUTSIDE(innercv) = new_cv;
            }
          }
          else { /* format reference */
            SV * const rv = curpad[ix];
            CV *innercv;
            if (!SvOK(rv)) continue;
            assert(SvROK(rv));
            assert(SvWEAKREF(rv));
            innercv = (CV *)SvRV(rv);
            assert(!CvWEAKOUTSIDE(innercv));
            assert(CvOUTSIDE(innercv) == old_cv);
            SvREFCNT_dec(CvOUTSIDE(innercv));
            CvOUTSIDE(innercv) = (CV *)SvREFCNT_inc_simple_NN(new_cv);
          }
        }
    }
}

/*
=for apidoc pad_push

Push a new pad frame onto the padlist, unless there's already a pad at
this depth, in which case don't bother creating a new one.  Then give
the new pad an C<@_> in slot zero.

=cut
*/

void
Perl_pad_push(pTHX_ PADLIST *padlist, int depth)
{
    PERL_ARGS_ASSERT_PAD_PUSH;

    if (depth > PadlistMAX(padlist) || !PadlistARRAY(padlist)[depth]) {
        PAD** const svp = PadlistARRAY(padlist);
        AV* const newpad = newAV();
        SV** const oldpad = AvARRAY(svp[depth-1]);
        PADOFFSET ix = AvFILLp((const AV *)svp[1]);
        const PADOFFSET names_fill = PadnamelistMAX((PADNAMELIST *)svp[0]);
        PADNAME ** const names = PadnamelistARRAY((PADNAMELIST *)svp[0]);
        AV *av;

        Newxz( AvALLOC(newpad), ix + 1, SV *);
        AvARRAY(newpad) = AvALLOC(newpad);
        AvMAX(newpad) = AvFILLp(newpad) = ix;

        for ( ;ix > 0; ix--) {
            SV *sv;
            if (names_fill >= ix && PadnameLEN(names[ix])) {
                const char sigil = PadnamePV(names[ix])[0];
                if (PadnameOUTER(names[ix])
                        || PadnameIsSTATE(names[ix])
                        || sigil == '&')
                {
                    /* outer lexical or anon code */
                    sv = SvREFCNT_inc(oldpad[ix]);
                }
                else {		/* our own lexical */
                    if (sigil == '@')
                        sv = MUTABLE_SV(newAV());
                    else if (sigil == '%')
                        sv = MUTABLE_SV(newHV());
                    else
                        sv = newSV_type(SVt_NULL);
                }
            }
            else if (PadnamePV(names[ix])) {
                sv = SvREFCNT_inc_NN(oldpad[ix]);
            }
            else {
                /* save temporaries on recursion? */
                sv = newSV_type(SVt_NULL);
                SvPADTMP_on(sv);
            }
            AvARRAY(newpad)[ix] = sv;
        }
        av = newAV();
        AvARRAY(newpad)[0] = MUTABLE_SV(av);
        AvREIFY_only(av);

        padlist_store(padlist, depth, newpad);
    }
}

#if defined(USE_ITHREADS)

/*
=for apidoc padlist_dup

Duplicates a pad.

=cut
*/

PADLIST *
Perl_padlist_dup(pTHX_ PADLIST *srcpad, CLONE_PARAMS *param)
{
    PADLIST *dstpad;
    bool cloneall;
    PADOFFSET max;

    PERL_ARGS_ASSERT_PADLIST_DUP;

    cloneall = cBOOL(param->flags & CLONEf_COPY_STACKS);
    assert (SvREFCNT(PadlistARRAY(srcpad)[1]) == 1);

    max = cloneall ? PadlistMAX(srcpad) : 1;

    Newx(dstpad, 1, PADLIST);
    ptr_table_store(PL_ptr_table, srcpad, dstpad);
    PadlistMAX(dstpad) = max;
    Newx(PadlistARRAY(dstpad), max + 1, PAD *);

    PadlistARRAY(dstpad)[0] = (PAD *)padnamelist_dup_inc(PadlistNAMES(srcpad), param);
    if (cloneall) {
        PADOFFSET depth;
        for (depth = 1; depth <= max; ++depth)
            PadlistARRAY(dstpad)[depth] =
                av_dup_inc(PadlistARRAY(srcpad)[depth], param);
    } else {
        /* CvDEPTH() on our subroutine will be set to 0, so there's no need
           to build anything other than the first level of pads.  */
        PADOFFSET ix = AvFILLp(PadlistARRAY(srcpad)[1]);
        AV *pad1;
        const PADOFFSET names_fill = PadnamelistMAX(PadlistNAMES(srcpad));
        const PAD *const srcpad1 = PadlistARRAY(srcpad)[1];
        SV **oldpad = AvARRAY(srcpad1);
        PADNAME ** const names = PadnamelistARRAY(PadlistNAMES(dstpad));
        SV **pad1a;
        AV *args;

        pad1 = newAV();

        av_extend(pad1, ix);
        PadlistARRAY(dstpad)[1] = pad1;
        pad1a = AvARRAY(pad1);

        if (ix > -1) {
            AvFILLp(pad1) = ix;

            for ( ;ix > 0; ix--) {
                if (!oldpad[ix]) {
                    pad1a[ix] = NULL;
                } else if (names_fill >= ix && names[ix] &&
                           PadnameLEN(names[ix])) {
                    const char sigil = PadnamePV(names[ix])[0];
                    if (PadnameOUTER(names[ix])
                        || PadnameIsSTATE(names[ix])
                        || sigil == '&')
                        {
                            /* outer lexical or anon code */
                            pad1a[ix] = sv_dup_inc(oldpad[ix], param);
                        }
                    else {		/* our own lexical */
                        if(SvPADSTALE(oldpad[ix]) && SvREFCNT(oldpad[ix]) > 1) {
                            /* This is a work around for how the current
                               implementation of ?{ } blocks in regexps
                               interacts with lexicals.  */
                            pad1a[ix] = sv_dup_inc(oldpad[ix], param);
                        } else {
                            SV *sv; 
                            
                            if (sigil == '@')
                                sv = MUTABLE_SV(newAV());
                            else if (sigil == '%')
                                sv = MUTABLE_SV(newHV());
                            else
                                sv = newSV_type(SVt_NULL);
                            pad1a[ix] = sv;
                        }
                    }
                }
                else if ((  names_fill >= ix && names[ix]
                         && PadnamePV(names[ix])  )) {
                    pad1a[ix] = sv_dup_inc(oldpad[ix], param);
                }
                else {
                    /* save temporaries on recursion? */
                    SV * const sv = newSV_type(SVt_NULL);
                    pad1a[ix] = sv;

                    /* SvREFCNT(oldpad[ix]) != 1 for some code in threads.xs
                       FIXTHAT before merging this branch.
                       (And I know how to) */
                    if (SvPADTMP(oldpad[ix]))
                        SvPADTMP_on(sv);
                }
            }

            if (oldpad[0]) {
                args = newAV();			/* Will be @_ */
                AvREIFY_only(args);
                pad1a[0] = (SV *)args;
            }
        }
    }

    return dstpad;
}

#endif /* USE_ITHREADS */

PAD **
Perl_padlist_store(pTHX_ PADLIST *padlist, I32 key, PAD *val)
{
    PAD **ary;
    SSize_t const oldmax = PadlistMAX(padlist);

    PERL_ARGS_ASSERT_PADLIST_STORE;

    assert(key >= 0);

    if (key > PadlistMAX(padlist)) {
        av_extend_guts(NULL,key,&PadlistMAX(padlist),
                       (SV ***)&PadlistARRAY(padlist),
                       (SV ***)&PadlistARRAY(padlist));
        Zero(PadlistARRAY(padlist)+oldmax+1, PadlistMAX(padlist)-oldmax,
             PAD *);
    }
    ary = PadlistARRAY(padlist);
    SvREFCNT_dec(ary[key]);
    ary[key] = val;
    return &ary[key];
}

/*
=for apidoc newPADNAMELIST

Creates a new pad name list.  C<max> is the highest index for which space
is allocated.

=cut
*/

PADNAMELIST *
Perl_newPADNAMELIST(size_t max)
{
    PADNAMELIST *pnl;
    Newx(pnl, 1, PADNAMELIST);
    Newxz(PadnamelistARRAY(pnl), max+1, PADNAME *);
    PadnamelistMAX(pnl) = -1;
    PadnamelistREFCNT(pnl) = 1;
    PadnamelistMAXNAMED(pnl) = 0;
    pnl->xpadnl_max = max;
    return pnl;
}

/*
=for apidoc padnamelist_store

Stores the pad name (which may be null) at the given index, freeing any
existing pad name in that slot.

=cut
*/

PADNAME **
Perl_padnamelist_store(pTHX_ PADNAMELIST *pnl, SSize_t key, PADNAME *val)
{
    PADNAME **ary;

    PERL_ARGS_ASSERT_PADNAMELIST_STORE;

    assert(key >= 0);

    if (key > pnl->xpadnl_max)
        av_extend_guts(NULL,key,&pnl->xpadnl_max,
                       (SV ***)&PadnamelistARRAY(pnl),
                       (SV ***)&PadnamelistARRAY(pnl));
    if (PadnamelistMAX(pnl) < key) {
        Zero(PadnamelistARRAY(pnl)+PadnamelistMAX(pnl)+1,
             key-PadnamelistMAX(pnl), PADNAME *);
        PadnamelistMAX(pnl) = key;
    }
    ary = PadnamelistARRAY(pnl);
    if (ary[key])
        PadnameREFCNT_dec(ary[key]);
    ary[key] = val;
    return &ary[key];
}

/*
=for apidoc padnamelist_fetch

Fetches the pad name from the given index.

=cut
*/

PADNAME *
Perl_padnamelist_fetch(PADNAMELIST *pnl, SSize_t key)
{
    PERL_ARGS_ASSERT_PADNAMELIST_FETCH;
    ASSUME(key >= 0);

    return key > PadnamelistMAX(pnl) ? NULL : PadnamelistARRAY(pnl)[key];
}

void
Perl_padnamelist_free(pTHX_ PADNAMELIST *pnl)
{
    PERL_ARGS_ASSERT_PADNAMELIST_FREE;
    if (!--PadnamelistREFCNT(pnl)) {
        while(PadnamelistMAX(pnl) >= 0)
        {
            PADNAME * const pn =
                PadnamelistARRAY(pnl)[PadnamelistMAX(pnl)--];
            if (pn)
                PadnameREFCNT_dec(pn);
        }
        Safefree(PadnamelistARRAY(pnl));
        Safefree(pnl);
    }
}

#if defined(USE_ITHREADS)

/*
=for apidoc padnamelist_dup

Duplicates a pad name list.

=cut
*/

PADNAMELIST *
Perl_padnamelist_dup(pTHX_ PADNAMELIST *srcpad, CLONE_PARAMS *param)
{
    PADNAMELIST *dstpad;
    SSize_t max = PadnamelistMAX(srcpad);

    PERL_ARGS_ASSERT_PADNAMELIST_DUP;

    /* look for it in the table first */
    dstpad = (PADNAMELIST *)ptr_table_fetch(PL_ptr_table, srcpad);
    if (dstpad)
        return dstpad;

    dstpad = newPADNAMELIST(max);
    PadnamelistREFCNT(dstpad) = 0; /* The caller will increment it.  */
    PadnamelistMAXNAMED(dstpad) = PadnamelistMAXNAMED(srcpad);
    PadnamelistMAX(dstpad) = max;

    ptr_table_store(PL_ptr_table, srcpad, dstpad);
    for (; max >= 0; max--)
      if (PadnamelistARRAY(srcpad)[max]) {
        PadnamelistARRAY(dstpad)[max] =
            padname_dup_inc(PadnamelistARRAY(srcpad)[max], param);
      }

    return dstpad;
}

#endif /* USE_ITHREADS */

/*
=for apidoc newPADNAMEpvn

Constructs and returns a new pad name.  C<s> must be a UTF-8 string.  Do not
use this for pad names that point to outer lexicals.  See
C<L</newPADNAMEouter>>.

=cut
*/

PADNAME *
Perl_newPADNAMEpvn(const char *s, STRLEN len)
{
    struct padname_with_str *alloc;
    char *alloc2; /* for Newxz */
    PADNAME *pn;
    PERL_ARGS_ASSERT_NEWPADNAMEPVN;
    Newxz(alloc2,
          STRUCT_OFFSET(struct padname_with_str, xpadn_str[0]) + len + 1,
          char);
    alloc = (struct padname_with_str *)alloc2;
    pn = (PADNAME *)alloc;
    PadnameREFCNT(pn) = 1;
    PadnamePV(pn) = alloc->xpadn_str;
    Copy(s, PadnamePV(pn), len, char);
    *(PadnamePV(pn) + len) = '\0';
    PadnameLEN(pn) = len;
    return pn;
}

/*
=for apidoc newPADNAMEouter

Constructs and returns a new pad name.  Only use this function for names
that refer to outer lexicals.  (See also L</newPADNAMEpvn>.)  C<outer> is
the outer pad name that this one mirrors.  The returned pad name has the
C<PADNAMEf_OUTER> flag already set.

=for apidoc Amnh||PADNAMEf_OUTER

=cut
*/

PADNAME *
Perl_newPADNAMEouter(PADNAME *outer)
{
    PADNAME *pn;
    PERL_ARGS_ASSERT_NEWPADNAMEOUTER;
    Newxz(pn, 1, PADNAME);
    PadnameREFCNT(pn) = 1;
    PadnamePV(pn) = PadnamePV(outer);
    /* Not PadnameREFCNT(outer), because ‘outer’ may itself close over
       another entry.  The original pad name owns the buffer.  */
    PadnameREFCNT_inc(PADNAME_FROM_PV(PadnamePV(outer)));
    PadnameFLAGS(pn) = PADNAMEf_OUTER;
    if(PadnameIsFIELD(outer)) {
        PadnameFIELDINFO(pn) = PadnameFIELDINFO(outer);
        PadnameFIELDINFO(pn)->refcount++;
        PadnameFLAGS(pn) |= PADNAMEf_FIELD;
    }
    PadnameLEN(pn) = PadnameLEN(outer);
    return pn;
}

void
Perl_padname_free(pTHX_ PADNAME *pn)
{
    PERL_ARGS_ASSERT_PADNAME_FREE;
    if (!--PadnameREFCNT(pn)) {
        if (UNLIKELY(pn == &PL_padname_undef || pn == &PL_padname_const)) {
            PadnameREFCNT(pn) = SvREFCNT_IMMORTAL;
            return;
        }
        SvREFCNT_dec(PadnameTYPE(pn)); /* Takes care of protocv, too.  */
        SvREFCNT_dec(PadnameOURSTASH(pn));
        if (PadnameOUTER(pn))
            PadnameREFCNT_dec(PADNAME_FROM_PV(PadnamePV(pn)));
        if (PadnameIsFIELD(pn)) {
            struct padname_fieldinfo *info = PadnameFIELDINFO(pn);
            if(!--info->refcount) {
                SvREFCNT_dec(info->fieldstash);
                /* todo: something about defop */
                SvREFCNT_dec(info->paramname);

                Safefree(info);
            }
        }
        Safefree(pn);
    }
}

#if defined(USE_ITHREADS)

/*
=for apidoc padname_dup

Duplicates a pad name.

=cut
*/

PADNAME *
Perl_padname_dup(pTHX_ PADNAME *src, CLONE_PARAMS *param)
{
    PADNAME *dst;

    PERL_ARGS_ASSERT_PADNAME_DUP;

    /* look for it in the table first */
    dst = (PADNAME *)ptr_table_fetch(PL_ptr_table, src);
    if (dst)
        return dst;

    if (!PadnamePV(src)) {
        dst = &PL_padname_undef;
        ptr_table_store(PL_ptr_table, src, dst);
        return dst;
    }

    dst = PadnameOUTER(src)
     ? newPADNAMEouter(padname_dup(PADNAME_FROM_PV(PadnamePV(src)), param))
     : newPADNAMEpvn(PadnamePV(src), PadnameLEN(src));
    ptr_table_store(PL_ptr_table, src, dst);
    PadnameLEN(dst) = PadnameLEN(src);
    PadnameFLAGS(dst) = PadnameFLAGS(src);
    PadnameREFCNT(dst) = 0; /* The caller will increment it.  */
    PadnameTYPE   (dst) = (HV *)sv_dup_inc((SV *)PadnameTYPE(src), param);
    PadnameOURSTASH(dst) = (HV *)sv_dup_inc((SV *)PadnameOURSTASH(src),
                                            param);
    if(PadnameIsFIELD(src) && !PadnameOUTER(src)) {
        struct padname_fieldinfo *sinfo = PadnameFIELDINFO(src);
        struct padname_fieldinfo *dinfo;
        Newxz(dinfo, 1, struct padname_fieldinfo);

        dinfo->refcount   = 1;
        dinfo->fieldix    = sinfo->fieldix;
        dinfo->fieldstash = hv_dup_inc(sinfo->fieldstash, param);
        dinfo->paramname  = sv_dup_inc(sinfo->paramname, param);

        PadnameFIELDINFO(dst) = dinfo;
    }
    dst->xpadn_low  = src->xpadn_low;
    dst->xpadn_high = src->xpadn_high;
    dst->xpadn_gen  = src->xpadn_gen;
    return dst;
}

#endif /* USE_ITHREADS */

/*
=for apidoc_section $lexer
=for apidoc suspend_compcv

Implements part of the concept of a "suspended compilation CV", which can be
used to pause the parser and compiler during parsing a CV in order to come
back to it later on.

This function saves the current state of the subroutine under compilation
(C<PL_compcv>) into the supplied buffer.  This should be used initially to
create the state in the buffer, as the final thing before a C<LEAVE> within a
block.

    ENTER;
    start_subparse(0);
    ...

    suspend_compcv(&buffer);
    LEAVE;

Once suspended, the C<resume_compcv> or C<resume_compcv_and_save> function can
later be used to continue the parsing from the point this stopped.

=cut
*/

void
Perl_suspend_compcv(pTHX_ struct suspended_compcv *buffer)
{
    PERL_ARGS_ASSERT_SUSPEND_COMPCV;

    buffer->compcv = PL_compcv;

    buffer->padix             = PL_padix;
    buffer->constpadix        = PL_constpadix;

    buffer->comppad_name_fill = PL_comppad_name_fill;
    buffer->min_intro_pending = PL_min_intro_pending;
    buffer->max_intro_pending = PL_max_intro_pending;

    buffer->cv_has_eval       = PL_cv_has_eval;
    buffer->pad_reset_pending = PL_pad_reset_pending;
}

/*
=for apidoc resume_compcv_final

Resumes the parser state previously saved using the C<suspend_compcv> function
for a final time before being compiled into a full CV.  This should be used
within an C<ENTER>/C<LEAVE> scoped pair.

=for apidoc resume_compcv_and_save

Resumes a buffer previously suspended by the C<suspend_compcv> function, in a
way that will be re-suspended at the end of the scope so it can be used again
later.  This should be used within an C<ENTER>/C<LEAVE> scoped pair.

=cut
*/

void
Perl_resume_compcv(pTHX_ struct suspended_compcv *buffer, bool save)
{
    PERL_ARGS_ASSERT_RESUME_COMPCV;

    SAVESPTR(PL_compcv);
    PL_compcv = buffer->compcv;
    PAD_SET_CUR(CvPADLIST(PL_compcv), 1);

    SAVESPTR(PL_comppad_name);
    PL_comppad_name = PadlistNAMES(CvPADLIST(PL_compcv));

    SAVESTRLEN(PL_padix);             PL_padix             = buffer->padix;
    SAVESTRLEN(PL_constpadix);        PL_constpadix        = buffer->constpadix;
    SAVESTRLEN(PL_comppad_name_fill); PL_comppad_name_fill = buffer->comppad_name_fill;
    SAVESTRLEN(PL_min_intro_pending); PL_min_intro_pending = buffer->min_intro_pending;
    SAVESTRLEN(PL_max_intro_pending); PL_max_intro_pending = buffer->max_intro_pending;

    SAVEBOOL(PL_cv_has_eval);       PL_cv_has_eval       = buffer->cv_has_eval;
    SAVEBOOL(PL_pad_reset_pending); PL_pad_reset_pending = buffer->pad_reset_pending;

    if(save)
        SAVEDESTRUCTOR_X(&Perl_suspend_compcv, buffer);
}

/*
 * ex: set ts=8 sts=4 sw=4 et:
 */
