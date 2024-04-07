/*    mathoms.c
 *
 *    Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010,
 *    2011, 2012 by Larry Wall and others
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

/*
 *  Anything that Hobbits had no immediate use for, but were unwilling to
 *  throw away, they called a mathom.  Their dwellings were apt to become
 *  rather crowded with mathoms, and many of the presents that passed from
 *  hand to hand were of that sort.
 *
 *     [p.5 of _The Lord of the Rings_: "Prologue"]
 */



/*
 * This file contains mathoms, various binary artifacts from previous
 * versions of Perl which we cannot completely remove from the core
 * code. There are two reasons functions should be here:
 *
 * 1) A function has been replaced by a macro within a minor release,
 *    so XS modules compiled against an older release will expect to
 *    still be able to link against the function
 * 2) A function Perl_foo(...) with #define foo Perl_foo(aTHX_ ...)
 *    has been replaced by a macro, e.g. #define foo(...) foo_flags(...,0)
 *    but XS code may still explicitly use the long form, i.e.
 *    Perl_foo(aTHX_ ...)
 *
 * This file can't just be cleaned out periodically, because that would break
 * builds with -DPERL_NO_SHORT_NAMES
 *
 * NOTE: ALL FUNCTIONS IN THIS FILE should have an entry with the 'b' flag in
 * embed.fnc.
 *
 * To move a function to this file, simply cut and paste it here, and change
 * its embed.fnc entry to additionally have the 'b' flag.  If, for some reason
 * a function you'd like to be treated as mathoms can't be moved from its
 * current place, simply enclose it between
 *
 * #ifndef NO_MATHOMS
 *    ...
 * #endif
 *
 * and add the 'b' flag in embed.fnc.
 *
 * The compilation of this file can be suppressed; see INSTALL
 *
 * Some blurb for perlapi.pod:

 head1 Obsolete backwards compatibility functions

Some of these are also deprecated.  You can exclude these from
your compiled Perl by adding this option to Configure:
C<-Accflags='-DNO_MATHOMS'>

=cut

 */


#include "EXTERN.h"
#define PERL_IN_MATHOMS_C
#include "perl.h"

#ifdef NO_MATHOMS
/* ..." warning: ISO C forbids an empty source file"
   So make sure we have something in here by processing the headers anyway.
 */
#else

/* The functions in this file should be able to call other deprecated functions
 * without a compiler warning */
GCC_DIAG_IGNORE(-Wdeprecated-declarations)

/* ref() is now a macro using Perl_doref;
 * this version provided for binary compatibility only.
 */
OP *
Perl_ref(pTHX_ OP *o, I32 type)
{
    return doref(o, type, TRUE);
}

/*
=for apidoc_section $SV
=for apidoc sv_unref

Unsets the RV status of the SV, and decrements the reference count of
whatever was being referenced by the RV.  This can almost be thought of
as a reversal of C<newSVrv>.  This is C<sv_unref_flags> with the C<flag>
being zero.  See C<L</SvROK_off>>.

=cut
*/

void
Perl_sv_unref(pTHX_ SV *sv)
{
    PERL_ARGS_ASSERT_SV_UNREF;

    sv_unref_flags(sv, 0);
}

/*
=for apidoc_section $tainting
=for apidoc sv_taint

Taint an SV.  Use C<SvTAINTED_on> instead.

=cut
*/

void
Perl_sv_taint(pTHX_ SV *sv)
{
    PERL_ARGS_ASSERT_SV_TAINT;

    sv_magic((sv), NULL, PERL_MAGIC_taint, NULL, 0);
}

/* sv_2iv() is now a macro using Perl_sv_2iv_flags();
 * this function provided for binary compatibility only
 */

IV
Perl_sv_2iv(pTHX_ SV *sv)
{
    PERL_ARGS_ASSERT_SV_2IV;

    return sv_2iv_flags(sv, SV_GMAGIC);
}

/* sv_2uv() is now a macro using Perl_sv_2uv_flags();
 * this function provided for binary compatibility only
 */

UV
Perl_sv_2uv(pTHX_ SV *sv)
{
    PERL_ARGS_ASSERT_SV_2UV;

    return sv_2uv_flags(sv, SV_GMAGIC);
}

/* sv_2nv() is now a macro using Perl_sv_2nv_flags();
 * this function provided for binary compatibility only
 */

NV
Perl_sv_2nv(pTHX_ SV *sv)
{
    return sv_2nv_flags(sv, SV_GMAGIC);
}


/* sv_2pv() is now a macro using Perl_sv_2pv_flags();
 * this function provided for binary compatibility only
 */

char *
Perl_sv_2pv(pTHX_ SV *sv, STRLEN *lp)
{
    PERL_ARGS_ASSERT_SV_2PV;

    return sv_2pv_flags(sv, lp, SV_GMAGIC);
}

/*
=for apidoc_section $SV
=for apidoc sv_2pv_nolen

Like C<sv_2pv()>, but doesn't return the length too.  You should usually
use the macro wrapper C<SvPV_nolen(sv)> instead.

=cut
*/

char *
Perl_sv_2pv_nolen(pTHX_ SV *sv)
{
    PERL_ARGS_ASSERT_SV_2PV_NOLEN;
    return sv_2pv(sv, NULL);
}

/*
=for apidoc_section $SV
=for apidoc sv_2pvbyte_nolen

Return a pointer to the byte-encoded representation of the SV.
May cause the SV to be downgraded from UTF-8 as a side-effect.

Usually accessed via the C<SvPVbyte_nolen> macro.

=cut
*/

char *
Perl_sv_2pvbyte_nolen(pTHX_ SV *sv)
{
    PERL_ARGS_ASSERT_SV_2PVBYTE_NOLEN;

    return sv_2pvbyte(sv, NULL);
}

/*
=for apidoc_section $SV
=for apidoc sv_2pvutf8_nolen

Return a pointer to the UTF-8-encoded representation of the SV.
May cause the SV to be upgraded to UTF-8 as a side-effect.

Usually accessed via the C<SvPVutf8_nolen> macro.

=cut
*/

char *
Perl_sv_2pvutf8_nolen(pTHX_ SV *sv)
{
    PERL_ARGS_ASSERT_SV_2PVUTF8_NOLEN;

    return sv_2pvutf8(sv, NULL);
}

/*
=for apidoc_section $SV
=for apidoc sv_force_normal

Undo various types of fakery on an SV: if the PV is a shared string, make
a private copy; if we're a ref, stop refing; if we're a glob, downgrade to
an C<xpvmg>.  See also C<L</sv_force_normal_flags>>.

=cut
*/

void
Perl_sv_force_normal(pTHX_ SV *sv)
{
    PERL_ARGS_ASSERT_SV_FORCE_NORMAL;

    sv_force_normal_flags(sv, 0);
}

/* sv_setsv() is now a macro using Perl_sv_setsv_flags();
 * this function provided for binary compatibility only
 */

void
Perl_sv_setsv(pTHX_ SV *dsv, SV *ssv)
{
    PERL_ARGS_ASSERT_SV_SETSV;

    sv_setsv_flags(dsv, ssv, SV_GMAGIC);
}

/* sv_catpvn() is now a macro using Perl_sv_catpvn_flags();
 * this function provided for binary compatibility only
 */

void
Perl_sv_catpvn(pTHX_ SV *dsv, const char* sstr, STRLEN slen)
{
    PERL_ARGS_ASSERT_SV_CATPVN;

    sv_catpvn_flags(dsv, sstr, slen, SV_GMAGIC);
}

void
Perl_sv_catpvn_mg(pTHX_ SV *dsv, const char *sstr, STRLEN len)
{
    PERL_ARGS_ASSERT_SV_CATPVN_MG;

    sv_catpvn_flags(dsv,sstr,len,SV_GMAGIC|SV_SMAGIC);
}

/* sv_catsv() is now a macro using Perl_sv_catsv_flags();
 * this function provided for binary compatibility only
 */

void
Perl_sv_catsv(pTHX_ SV *dsv, SV *sstr)
{
    PERL_ARGS_ASSERT_SV_CATSV;

    sv_catsv_flags(dsv, sstr, SV_GMAGIC);
}

void
Perl_sv_catsv_mg(pTHX_ SV *dsv, SV *sstr)
{
    PERL_ARGS_ASSERT_SV_CATSV_MG;

    sv_catsv_flags(dsv,sstr,SV_GMAGIC|SV_SMAGIC);
}

/*
=for apidoc_section $SV
=for apidoc sv_pv

Use the C<SvPV_nolen> macro instead

=cut
*/

/* sv_pv() is now a macro using SvPV_nolen();
 * this function provided for binary compatibility only
 */

char *
Perl_sv_pv(pTHX_ SV *sv)
{
    PERL_ARGS_ASSERT_SV_PV;

    if (SvPOK(sv))
        return SvPVX(sv);

    return sv_2pv(sv, NULL);
}

/* sv_pvn_force() is now a macro using Perl_sv_pvn_force_flags();
 * this function provided for binary compatibility only
 */

char *
Perl_sv_pvn_force(pTHX_ SV *sv, STRLEN *lp)
{
    PERL_ARGS_ASSERT_SV_PVN_FORCE;

    return sv_pvn_force_flags(sv, lp, SV_GMAGIC);
}

/* sv_pvbyte () is now a macro using Perl_sv_2pv_flags();
 * this function provided for binary compatibility only
 */

char *
Perl_sv_pvbyte(pTHX_ SV *sv)
{
    PERL_ARGS_ASSERT_SV_PVBYTE;

    (void)sv_utf8_downgrade(sv, FALSE);
    return sv_pv(sv);
}

/*
=for apidoc_section $SV
=for apidoc sv_pvbyte

Use C<SvPVbyte_nolen> instead.

=cut
*/

/*
=for apidoc_section $SV
=for apidoc sv_pvutf8

Use the C<SvPVutf8_nolen> macro instead

=cut
*/


char *
Perl_sv_pvutf8(pTHX_ SV *sv)
{
    PERL_ARGS_ASSERT_SV_PVUTF8;

    sv_utf8_upgrade(sv);
    return sv_pv(sv);
}

/* sv_utf8_upgrade() is now a macro using sv_utf8_upgrade_flags();
 * this function provided for binary compatibility only
 */

STRLEN
Perl_sv_utf8_upgrade(pTHX_ SV *sv)
{
    PERL_ARGS_ASSERT_SV_UTF8_UPGRADE;

    return sv_utf8_upgrade_flags(sv, SV_GMAGIC);
}

#if defined(HUGE_VAL) || (defined(USE_LONG_DOUBLE) && defined(HUGE_VALL))
/*
 * This hack is to force load of "huge" support from libm.a
 * So it is in perl for (say) POSIX to use.
 * Needed for SunOS with Sun's 'acc' for example.
 */
NV
Perl_huge(void)
{
#  if defined(USE_LONG_DOUBLE) && defined(HUGE_VALL)
    return HUGE_VALL;
#  else
    return HUGE_VAL;
#  endif
}
#endif

void
Perl_gv_fullname3(pTHX_ SV *sv, const GV *gv, const char *prefix)
{
    PERL_ARGS_ASSERT_GV_FULLNAME3;

    gv_fullname4(sv, gv, prefix, TRUE);
}

void
Perl_gv_efullname3(pTHX_ SV *sv, const GV *gv, const char *prefix)
{
    PERL_ARGS_ASSERT_GV_EFULLNAME3;

    gv_efullname4(sv, gv, prefix, TRUE);
}

/*
=for apidoc_section $GV
=for apidoc gv_fetchmethod

See L</gv_fetchmethod_autoload>.

=cut
*/

GV *
Perl_gv_fetchmethod(pTHX_ HV *stash, const char *name)
{
    PERL_ARGS_ASSERT_GV_FETCHMETHOD;

    return gv_fetchmethod_autoload(stash, name, TRUE);
}

HE *
Perl_hv_iternext(pTHX_ HV *hv)
{
    PERL_ARGS_ASSERT_HV_ITERNEXT;

    return hv_iternext_flags(hv, 0);
}

void
Perl_hv_magic(pTHX_ HV *hv, GV *gv, int how)
{
    PERL_ARGS_ASSERT_HV_MAGIC;

    sv_magic(MUTABLE_SV(hv), MUTABLE_SV(gv), how, NULL, 0);
}

bool
Perl_do_open(pTHX_ GV *gv, const char *name, I32 len, int as_raw,
             int rawmode, int rawperm, PerlIO *supplied_fp)
{
    PERL_ARGS_ASSERT_DO_OPEN;

    return do_openn(gv, name, len, as_raw, rawmode, rawperm,
                    supplied_fp, (SV **) NULL, 0);
}

#ifndef OS2
bool
Perl_do_aexec(pTHX_ SV *really, SV **mark, SV **sp)
{
    PERL_ARGS_ASSERT_DO_AEXEC;

    return do_aexec5(really, mark, sp, 0, 0);
}
#endif

bool
Perl_is_utf8_string_loc(const U8 *s, const STRLEN len, const U8 **ep)
{
    PERL_ARGS_ASSERT_IS_UTF8_STRING_LOC;

    return is_utf8_string_loclen(s, len, ep, 0);
}

/*
=for apidoc_section $SV
=for apidoc sv_nolocking

Dummy routine which "locks" an SV when there is no locking module present.
Exists to avoid test for a C<NULL> function pointer and because it could
potentially warn under some level of strict-ness.

"Superseded" by C<sv_nosharing()>.

=cut
*/

void
Perl_sv_nolocking(pTHX_ SV *sv)
{
    PERL_UNUSED_CONTEXT;
    PERL_UNUSED_ARG(sv);
}


/*
=for apidoc_section $SV
=for apidoc sv_nounlocking

Dummy routine which "unlocks" an SV when there is no locking module present.
Exists to avoid test for a C<NULL> function pointer and because it could
potentially warn under some level of strict-ness.

"Superseded" by C<sv_nosharing()>.

=cut

PERL_UNLOCK_HOOK in intrpvar.h is the macro that refers to this, and guarantees
that mathoms gets loaded.

*/

void
Perl_sv_nounlocking(pTHX_ SV *sv)
{
    PERL_UNUSED_CONTEXT;
    PERL_UNUSED_ARG(sv);
}

void
Perl_sv_usepvn_mg(pTHX_ SV *sv, char *ptr, STRLEN len)
{
    PERL_ARGS_ASSERT_SV_USEPVN_MG;

    sv_usepvn_flags(sv,ptr,len, SV_SMAGIC);
}


void
Perl_sv_usepvn(pTHX_ SV *sv, char *ptr, STRLEN len)
{
    PERL_ARGS_ASSERT_SV_USEPVN;

    sv_usepvn_flags(sv,ptr,len, 0);
}

HE *
Perl_hv_store_ent(pTHX_ HV *hv, SV *keysv, SV *val, U32 hash)
{
  return (HE *)hv_common(hv, keysv, NULL, 0, 0, HV_FETCH_ISSTORE, val, hash);
}

bool
Perl_hv_exists_ent(pTHX_ HV *hv, SV *keysv, U32 hash)
{
    PERL_ARGS_ASSERT_HV_EXISTS_ENT;

    return cBOOL(hv_common(hv, keysv, NULL, 0, 0, HV_FETCH_ISEXISTS, 0, hash));
}

HE *
Perl_hv_fetch_ent(pTHX_ HV *hv, SV *keysv, I32 lval, U32 hash)
{
    PERL_ARGS_ASSERT_HV_FETCH_ENT;

    return (HE *)hv_common(hv, keysv, NULL, 0, 0, 
                     (lval ? HV_FETCH_LVALUE : 0), NULL, hash);
}

SV *
Perl_hv_delete_ent(pTHX_ HV *hv, SV *keysv, I32 flags, U32 hash)
{
    PERL_ARGS_ASSERT_HV_DELETE_ENT;

    return MUTABLE_SV(hv_common(hv, keysv, NULL, 0, 0, flags | HV_DELETE, NULL,
                                hash));
}

SV**
Perl_hv_store_flags(pTHX_ HV *hv, const char *key, I32 klen, SV *val, U32 hash,
                    int flags)
{
    return (SV**) hv_common(hv, NULL, key, klen, flags,
                            (HV_FETCH_ISSTORE|HV_FETCH_JUST_SV), val, hash);
}

SV**
Perl_hv_store(pTHX_ HV *hv, const char *key, I32 klen_i32, SV *val, U32 hash)
{
    STRLEN klen;
    int flags;

    if (klen_i32 < 0) {
        klen = -klen_i32;
        flags = HVhek_UTF8;
    } else {
        klen = klen_i32;
        flags = 0;
    }
    return (SV **) hv_common(hv, NULL, key, klen, flags,
                             (HV_FETCH_ISSTORE|HV_FETCH_JUST_SV), val, hash);
}

bool
Perl_hv_exists(pTHX_ HV *hv, const char *key, I32 klen_i32)
{
    STRLEN klen;
    int flags;

    PERL_ARGS_ASSERT_HV_EXISTS;

    if (klen_i32 < 0) {
        klen = -klen_i32;
        flags = HVhek_UTF8;
    } else {
        klen = klen_i32;
        flags = 0;
    }
    return cBOOL(hv_common(hv, NULL, key, klen, flags, HV_FETCH_ISEXISTS, 0, 0));
}

SV**
Perl_hv_fetch(pTHX_ HV *hv, const char *key, I32 klen_i32, I32 lval)
{
    STRLEN klen;
    int flags;

    PERL_ARGS_ASSERT_HV_FETCH;

    if (klen_i32 < 0) {
        klen = -klen_i32;
        flags = HVhek_UTF8;
    } else {
        klen = klen_i32;
        flags = 0;
    }
    return (SV **) hv_common(hv, NULL, key, klen, flags,
                             lval ? (HV_FETCH_JUST_SV | HV_FETCH_LVALUE)
                             : HV_FETCH_JUST_SV, NULL, 0);
}

SV *
Perl_hv_delete(pTHX_ HV *hv, const char *key, I32 klen_i32, I32 flags)
{
    STRLEN klen;
    int k_flags;

    PERL_ARGS_ASSERT_HV_DELETE;

    if (klen_i32 < 0) {
        klen = -klen_i32;
        k_flags = HVhek_UTF8;
    } else {
        klen = klen_i32;
        k_flags = 0;
    }
    return MUTABLE_SV(hv_common(hv, NULL, key, klen, k_flags, flags | HV_DELETE,
                                NULL, 0));
}

AV *
Perl_newAV(pTHX)
{
    return MUTABLE_AV(newSV_type(SVt_PVAV));
    /* sv_upgrade does AvREAL_only():
    AvALLOC(av) = 0;
    AvARRAY(av) = NULL;
    AvMAX(av) = AvFILLp(av) = -1; */
}

HV *
Perl_newHV(pTHX)
{
    HV * const hv = MUTABLE_HV(newSV_type(SVt_PVHV));
    assert(!SvOK(hv));

    return hv;
}

void
Perl_sv_insert(pTHX_ SV *const bigstr, const STRLEN offset, const STRLEN len, 
              const char *const little, const STRLEN littlelen)
{
    PERL_ARGS_ASSERT_SV_INSERT;
    sv_insert_flags(bigstr, offset, len, little, littlelen, SV_GMAGIC);
}

void
Perl_save_freesv(pTHX_ SV *sv)
{
    save_freesv(sv);
}

void
Perl_save_mortalizesv(pTHX_ SV *sv)
{
    PERL_ARGS_ASSERT_SAVE_MORTALIZESV;

    save_mortalizesv(sv);
}

void
Perl_save_freeop(pTHX_ OP *o)
{
    save_freeop(o);
}

void
Perl_save_freepv(pTHX_ char *pv)
{
    save_freepv(pv);
}

void
Perl_save_op(pTHX)
{
    save_op();
}

#ifdef PERL_DONT_CREATE_GVSV
GV *
Perl_gv_SVadd(pTHX_ GV *gv)
{
    return gv_SVadd(gv);
}
#endif

GV *
Perl_gv_AVadd(pTHX_ GV *gv)
{
    return gv_AVadd(gv);
}

GV *
Perl_gv_HVadd(pTHX_ GV *gv)
{
    return gv_HVadd(gv);
}

GV *
Perl_gv_IOadd(pTHX_ GV *gv)
{
    return gv_IOadd(gv);
}

IO *
Perl_newIO(pTHX)
{
    return MUTABLE_IO(newSV_type(SVt_PVIO));
}

I32
Perl_my_stat(pTHX)
{
    return my_stat_flags(SV_GMAGIC);
}

I32
Perl_my_lstat(pTHX)
{
    return my_lstat_flags(SV_GMAGIC);
}

I32
Perl_sv_eq(pTHX_ SV *sv1, SV *sv2)
{
    return sv_eq_flags(sv1, sv2, SV_GMAGIC);
}

#ifdef USE_LOCALE_COLLATE
char *
Perl_sv_collxfrm(pTHX_ SV *const sv, STRLEN *const nxp)
{
    PERL_ARGS_ASSERT_SV_COLLXFRM;
    return sv_collxfrm_flags(sv, nxp, SV_GMAGIC);
}

#endif

bool
Perl_sv_2bool(pTHX_ SV *const sv)
{
    PERL_ARGS_ASSERT_SV_2BOOL;
    return sv_2bool_flags(sv, SV_GMAGIC);
}

CV *
Perl_newSUB(pTHX_ I32 floor, OP *o, OP *proto, OP *block)
{
    return newATTRSUB(floor, o, proto, NULL, block);
}

SV *
Perl_sv_mortalcopy(pTHX_ SV *const oldsv)
{
    return Perl_sv_mortalcopy_flags(aTHX_ oldsv, SV_GMAGIC);
}

void
Perl_sv_copypv(pTHX_ SV *const dsv, SV *const ssv)
{
    PERL_ARGS_ASSERT_SV_COPYPV;

    sv_copypv_flags(dsv, ssv, SV_GMAGIC);
}

/*
=for apidoc_section $unicode
=for apidoc is_utf8_char_buf

This is identical to the macro L<perlapi/isUTF8_CHAR>.

=cut */

STRLEN
Perl_is_utf8_char_buf(const U8 *buf, const U8* buf_end)
{

    PERL_ARGS_ASSERT_IS_UTF8_CHAR_BUF;

    return isUTF8_CHAR(buf, buf_end);
}

/*
=for apidoc_section $unicode
=for apidoc utf8_to_uvuni

Returns the Unicode code point of the first character in the string C<s>
which is assumed to be in UTF-8 encoding; C<retlen> will be set to the
length, in bytes, of that character.

Some, but not all, UTF-8 malformations are detected, and in fact, some
malformed input could cause reading beyond the end of the input buffer, which
is one reason why this function is deprecated.  The other is that only in
extremely limited circumstances should the Unicode versus native code point be
of any interest to you.

If C<s> points to one of the detected malformations, and UTF8 warnings are
enabled, zero is returned and C<*retlen> is set (if C<retlen> doesn't point to
NULL) to -1.  If those warnings are off, the computed value if well-defined (or
the Unicode REPLACEMENT CHARACTER, if not) is silently returned, and C<*retlen>
is set (if C<retlen> isn't NULL) so that (S<C<s> + C<*retlen>>) is the
next possible position in C<s> that could begin a non-malformed character.
See L<perlapi/utf8n_to_uvchr> for details on when the REPLACEMENT CHARACTER is returned.

=cut
*/

UV
Perl_utf8_to_uvuni(pTHX_ const U8 *s, STRLEN *retlen)
{
    PERL_UNUSED_CONTEXT;
    PERL_ARGS_ASSERT_UTF8_TO_UVUNI;

    return NATIVE_TO_UNI(valid_utf8_to_uvchr(s, retlen));
}

/* return ptr to little string in big string, NULL if not found */
/* The original version of this routine was donated by Corey Satten. */

char *
Perl_instr(const char *big, const char *little)
{
    PERL_ARGS_ASSERT_INSTR;

    return instr(big, little);
}

SV *
Perl_newSVsv(pTHX_ SV *const old)
{
    return newSVsv(old);
}

bool
Perl_sv_utf8_downgrade(pTHX_ SV *const sv, const bool fail_ok)
{
    PERL_ARGS_ASSERT_SV_UTF8_DOWNGRADE;

    return sv_utf8_downgrade(sv, fail_ok);
}

char *
Perl_sv_2pvutf8(pTHX_ SV *sv, STRLEN *const lp)
{
    PERL_ARGS_ASSERT_SV_2PVUTF8;

    return sv_2pvutf8(sv, lp);
}

char *
Perl_sv_2pvbyte(pTHX_ SV *sv, STRLEN *const lp)
{
    PERL_ARGS_ASSERT_SV_2PVBYTE;

    return sv_2pvbyte(sv, lp);
}

U8 *
Perl_uvuni_to_utf8(pTHX_ U8 *d, UV uv)
{
    PERL_ARGS_ASSERT_UVUNI_TO_UTF8;

    return uvoffuni_to_utf8_flags(d, uv, 0);
}

/*
=for apidoc_section $unicode
=for apidoc utf8n_to_uvuni

Instead use L<perlapi/utf8_to_uvchr_buf>, or rarely, L<perlapi/utf8n_to_uvchr>.

This function was useful for code that wanted to handle both EBCDIC and
ASCII platforms with Unicode properties, but starting in Perl v5.20, the
distinctions between the platforms have mostly been made invisible to most
code, so this function is quite unlikely to be what you want.  If you do need
this precise functionality, use instead
C<L<NATIVE_TO_UNI(utf8_to_uvchr_buf(...))|perlapi/utf8_to_uvchr_buf>>
or C<L<NATIVE_TO_UNI(utf8n_to_uvchr(...))|perlapi/utf8n_to_uvchr>>.

=cut
*/

UV
Perl_utf8n_to_uvuni(pTHX_ const U8 *s, STRLEN curlen, STRLEN *retlen, U32 flags)
{
    PERL_ARGS_ASSERT_UTF8N_TO_UVUNI;

    return NATIVE_TO_UNI(utf8n_to_uvchr(s, curlen, retlen, flags));
}

/*
=for apidoc_section $unicode
=for apidoc utf8_to_uvchr

Returns the native code point of the first character in the string C<s>
which is assumed to be in UTF-8 encoding; C<retlen> will be set to the
length, in bytes, of that character.

Some, but not all, UTF-8 malformations are detected, and in fact, some
malformed input could cause reading beyond the end of the input buffer, which
is why this function is deprecated.  Use L</utf8_to_uvchr_buf> instead.

If C<s> points to one of the detected malformations, and UTF8 warnings are
enabled, zero is returned and C<*retlen> is set (if C<retlen> isn't
C<NULL>) to -1.  If those warnings are off, the computed value if well-defined (or
the Unicode REPLACEMENT CHARACTER, if not) is silently returned, and C<*retlen>
is set (if C<retlen> isn't NULL) so that (S<C<s> + C<*retlen>>) is the
next possible position in C<s> that could begin a non-malformed character.
See L</utf8n_to_uvchr> for details on when the REPLACEMENT CHARACTER is returned.

=cut
*/

UV
Perl_utf8_to_uvchr(pTHX_ const U8 *s, STRLEN *retlen)
{
    PERL_ARGS_ASSERT_UTF8_TO_UVCHR;

    /* This function is unsafe if malformed UTF-8 input is given it, which is
     * why the function is deprecated.  If the first byte of the input
     * indicates that there are more bytes remaining in the sequence that forms
     * the character than there are in the input buffer, it can read past the
     * end.  But we can make it safe if the input string happens to be
     * NUL-terminated, as many strings in Perl are, by refusing to read past a
     * NUL, which is what UTF8_CHK_SKIP() does.  A NUL indicates the start of
     * the next character anyway.  If the input isn't NUL-terminated, the
     * function remains unsafe, as it always has been. */

    return utf8_to_uvchr_buf(s, s + UTF8_CHK_SKIP(s), retlen);
}

GCC_DIAG_RESTORE

#endif /* NO_MATHOMS */

/*
 * ex: set ts=8 sts=4 sw=4 et:
 */
