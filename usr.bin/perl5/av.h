/*    av.h
 *
 *    Copyright (C) 1991, 1992, 1993, 1995, 1996, 1997, 1998, 1999, 2000,
 *    2001, 2002, 2005, 2006, 2007, 2008, by Larry Wall and others
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

struct xpvav {
    HV*		xmg_stash;	/* class package */
    union _xmgu	xmg_u;
    SSize_t	xav_fill;       /* Index of last element present */
    SSize_t	xav_max;        /* max index for which array has space */
    SV**	xav_alloc;	/* pointer to beginning of C array of SVs */
};

/* SV*	xav_arylen; */

/* SVpav_REAL is set for all AVs whose xav_array contents are refcounted
 * and initialized such that any element can be retrieved as a SV*.
 * Such AVs may be referred to as "real" AVs. Examples include regular
 * perl arrays, tiedarrays (since v5.16), and padlist AVs.
 *
 * Some things do not set SVpav_REAL, to indicate that they are cheating
 * (for efficiency) by not refcounting the AV's contents or ensuring that
 * all elements are safe for arbitrary access. This type of AV may be
 * referred to as "fake" AVs. Examples include "@_" (unless tied), the
 * scratchpad list, and the backrefs list on an object or stash.
 *
 * SVpav_REIFY is only meaningful on such "fake" AVs (i.e. where SVpav_REAL
 * is not set).  It indicates that the fake AV is capable of becoming
 * real if the array needs to be modified in some way.  Functions that
 * modify fake AVs check both flags to call av_reify() as appropriate.
 *
 * av_reify() transforms a fake AV into a real one through two actions.
 * Allocated but unpopulated elements are initialized to make them safe for
 * arbitrary retrieval and the reference counts of populated elements are
 * incremented.
 *
 * Note that the Perl stack has neither flag set. (Thus,
 * items that go on the stack are never refcounted.)
 *
 * These internal details are subject to change any time.  AV
 * manipulations external to perl should not care about any of this.
 * GSAR 1999-09-10
 */

/*
=for apidoc ADmnU||Nullav
Null AV pointer.

(deprecated - use C<(AV *)NULL> instead)

=for apidoc Am|SSize_t|AvFILL|AV* av
Same as C<L</av_top_index>> or C<L</av_tindex>>.

=for apidoc Cm|SSize_t|AvFILLp|AV* av

If the array C<av> is empty, this returns -1; otherwise it returns the maximum
value of the indices of all the array elements which are currently defined in
C<av>.  It does not handle magic, hence the C<p> private indication in its name.

=for apidoc Am|SV**|AvARRAY|AV* av
Returns a pointer to the AV's internal SV* array.

This is useful for doing pointer arithmetic on the array.
If all you need is to look up an array element, then prefer C<av_fetch>.

=cut
*/

#ifndef PERL_CORE
#  define Nullav Null(AV*)
#endif

#define AvARRAY(av)	((av)->sv_u.svu_array)
#define AvALLOC(av)	((XPVAV*)  SvANY(av))->xav_alloc
#define AvMAX(av)	((XPVAV*)  SvANY(av))->xav_max
#define AvFILLp(av)	((XPVAV*)  SvANY(av))->xav_fill
#define AvARYLEN(av)	(*Perl_av_arylen_p(aTHX_ MUTABLE_AV(av)))

#define AvREAL(av)	(SvFLAGS(av) & SVpav_REAL)
#define AvREAL_on(av)	(SvFLAGS(av) |= SVpav_REAL)
#define AvREAL_off(av)	(SvFLAGS(av) &= ~SVpav_REAL)
#define AvREAL_only(av)	(AvREIFY_off(av), SvFLAGS(av) |= SVpav_REAL)
#define AvREIFY(av)	(SvFLAGS(av) & SVpav_REIFY)
#define AvREIFY_on(av)	(SvFLAGS(av) |= SVpav_REIFY)
#define AvREIFY_off(av)	(SvFLAGS(av) &= ~SVpav_REIFY)
#define AvREIFY_only(av)	(AvREAL_off(av), SvFLAGS(av) |= SVpav_REIFY)


#define AvREALISH(av)	(SvFLAGS(av) & (SVpav_REAL|SVpav_REIFY))
                                          
#define AvFILL(av)	((SvRMAGICAL((const SV *) (av))) \
                         ? mg_size(MUTABLE_SV(av)) : AvFILLp(av))
#define av_top_index(av) AvFILL(av)
#define av_tindex(av)    av_top_index(av)

/* Note that it doesn't make sense to do this:
 *      SvGETMAGIC(av); IV x = av_tindex_nomg(av);
 */
#   define av_top_index_skip_len_mg(av)                                     \
                            (__ASSERT_(SvTYPE(av) == SVt_PVAV) AvFILLp(av))
#   define av_tindex_skip_len_mg(av)  av_top_index_skip_len_mg(av)

#define NEGATIVE_INDICES_VAR "NEGATIVE_INDICES"

/*

Note that there are both real and fake AVs; see the beginning of this file and
'av.c'

=for apidoc newAV
=for apidoc_item newAV_alloc_x
=for apidoc_item newAV_alloc_xz

These all create a new AV, setting the reference count to 1.  If you also know
the initial elements of the array with, see L</C<av_make>>.

As background, an array consists of three things:

=over

=item 1.

A data structure containing information about the array as a whole, such as its
size and reference count.

=item 2.

A C language array of pointers to the individual elements.  These are treated
as pointers to SVs, so all must be castable to SV*.

=item 3.

The individual elements themselves.  These could be, for instance, SVs and/or
AVs and/or HVs, etc.

=back

An empty array need only have the first data structure, and all these functions
create that.  They differ in what else they do, as follows:

=over

=item C<newAV> form

=for comment
'form' above and below is because otherwise have two =items with the same name,
can't link to them.

This does nothing beyond creating the whole-array data structure.
The Perl equivalent is approximately S<C<my @array;>>

This is useful when the minimum size of the array could be zero (perhaps there
are likely code paths that will entirely skip using it).

If the array does get used, the pointers data structure will need to be
allocated at that time.  This will end up being done by L</av_extend>>,
either explicitly:

    av_extend(av, len);

or implicitly when the first element is stored:

    (void)av_store(av, 0, sv);

Unused array elements are typically initialized by C<av_extend>.

=item C<newAV_alloc_x> form

This effectively does a C<newAV> followed by also allocating (uninitialized)
space for the pointers array.  This is used when you know ahead of time the
likely minimum size of the array.  It is more efficient to do this than doing a
plain C<newAV> followed by an C<av_extend>.

Of course the array can be extended later should it become necessary.

C<size> must be at least 1.

=item C<newAV_alloc_xz> form

This is C<newAV_alloc_x>, but initializes each pointer in it to NULL.  This
gives added safety to guard against them being read before being set.

C<size> must be at least 1.

=back

The following examples all result in an array that can fit four elements
(indexes 0 .. 3):

    AV *av = newAV();
    av_extend(av, 3);

    AV *av = newAV_alloc_x(4);

    AV *av = newAV_alloc_xz(4);

In contrast, the following examples allocate an array that is only guaranteed
to fit one element without extending:

    AV *av = newAV_alloc_x(1);
    AV *av = newAV_alloc_xz(1);

=cut

*/

#define newAV()	MUTABLE_AV(newSV_type(SVt_PVAV))
#define newAV_alloc_x(size)  av_new_alloc(size,0)
#define newAV_alloc_xz(size) av_new_alloc(size,1)

/*
 * ex: set ts=8 sts=4 sw=4 et:
 */
