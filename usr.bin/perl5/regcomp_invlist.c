#ifdef PERL_EXT_RE_BUILD
#include "re_top.h"
#endif

#include "EXTERN.h"
#define PERL_IN_REGEX_ENGINE
#define PERL_IN_REGCOMP_ANY
#define PERL_IN_REGCOMP_INVLIST_C
#include "perl.h"

#ifdef PERL_IN_XSUB_RE
#  include "re_comp.h"
#else
#  include "regcomp.h"
#endif

#include "invlist_inline.h"
#include "unicode_constants.h"
#include "regcomp_internal.h"


void
Perl_populate_bitmap_from_invlist(pTHX_ SV * invlist, const UV offset, const U8 * bitmap, const Size_t len)
{
    PERL_ARGS_ASSERT_POPULATE_BITMAP_FROM_INVLIST;

    /* As the name says.  The zeroth bit corresponds to the code point given by
     * 'offset' */

    UV start, end;

    Zero(bitmap, len, U8);

    invlist_iterinit(invlist);
    while (invlist_iternext(invlist, &start, &end)) {
        assert(start >= offset);

        for (UV i = start; i <= end; i++) {
            UV adjusted = i - offset;

            BITMAP_BYTE(bitmap, adjusted) |= BITMAP_BIT(adjusted);
        }
    }
    invlist_iterfinish(invlist);
}

void
Perl_populate_invlist_from_bitmap(pTHX_ const U8 * bitmap, const Size_t bitmap_len, SV ** invlist, const UV offset)
{
    PERL_ARGS_ASSERT_POPULATE_INVLIST_FROM_BITMAP;

    /* As the name says.  The zeroth bit corresponds to the code point given by
     * 'offset' */

    Size_t i;

    for (i = 0; i < bitmap_len; i++) {
        if (BITMAP_TEST(bitmap, i)) {
            int start = i++;

            /* Save a little work by adding a range all at once instead of bit
             * by bit */
            while (i < bitmap_len && BITMAP_TEST(bitmap, i)) {
                i++;
            }

            *invlist = _add_range_to_invlist(*invlist,
                                             start + offset,
                                             i + offset - 1);
        }
    }
}

/* This section of code defines the inversion list object and its methods.  The
 * interfaces are highly subject to change, so as much as possible is static to
 * this file.  An inversion list is here implemented as a malloc'd C UV array
 * as an SVt_INVLIST scalar.
 *
 * An inversion list for Unicode is an array of code points, sorted by ordinal
 * number.  Each element gives the code point that begins a range that extends
 * up-to but not including the code point given by the next element.  The final
 * element gives the first code point of a range that extends to the platform's
 * infinity.  The even-numbered elements (invlist[0], invlist[2], invlist[4],
 * ...) give ranges whose code points are all in the inversion list.  We say
 * that those ranges are in the set.  The odd-numbered elements give ranges
 * whose code points are not in the inversion list, and hence not in the set.
 * Thus, element [0] is the first code point in the list.  Element [1]
 * is the first code point beyond that not in the list; and element [2] is the
 * first code point beyond that that is in the list.  In other words, the first
 * range is invlist[0]..(invlist[1]-1), and all code points in that range are
 * in the inversion list.  The second range is invlist[1]..(invlist[2]-1), and
 * all code points in that range are not in the inversion list.  The third
 * range invlist[2]..(invlist[3]-1) gives code points that are in the inversion
 * list, and so forth.  Thus every element whose index is divisible by two
 * gives the beginning of a range that is in the list, and every element whose
 * index is not divisible by two gives the beginning of a range not in the
 * list.  If the final element's index is divisible by two, the inversion list
 * extends to the platform's infinity; otherwise the highest code point in the
 * inversion list is the contents of that element minus 1.
 *
 * A range that contains just a single code point N will look like
 *  invlist[i]   == N
 *  invlist[i+1] == N+1
 *
 * If N is UV_MAX (the highest representable code point on the machine), N+1 is
 * impossible to represent, so element [i+1] is omitted.  The single element
 * inversion list
 *  invlist[0] == UV_MAX
 * contains just UV_MAX, but is interpreted as matching to infinity.
 *
 * Taking the complement (inverting) an inversion list is quite simple, if the
 * first element is 0, remove it; otherwise add a 0 element at the beginning.
 * This implementation reserves an element at the beginning of each inversion
 * list to always contain 0; there is an additional flag in the header which
 * indicates if the list begins at the 0, or is offset to begin at the next
 * element.  This means that the inversion list can be inverted without any
 * copying; just flip the flag.
 *
 * More about inversion lists can be found in "Unicode Demystified"
 * Chapter 13 by Richard Gillam, published by Addison-Wesley.
 *
 * The inversion list data structure is currently implemented as an SV pointing
 * to an array of UVs that the SV thinks are bytes.  This allows us to have an
 * array of UV whose memory management is automatically handled by the existing
 * facilities for SV's.
 *
 * Some of the methods should always be private to the implementation, and some
 * should eventually be made public */

/* The header definitions are in F<invlist_inline.h> */

#ifndef PERL_IN_XSUB_RE

PERL_STATIC_INLINE UV*
S__invlist_array_init(SV* const invlist, const bool will_have_0)
{
    /* Returns a pointer to the first element in the inversion list's array.
     * This is called upon initialization of an inversion list.  Where the
     * array begins depends on whether the list has the code point U+0000 in it
     * or not.  The other parameter tells it whether the code that follows this
     * call is about to put a 0 in the inversion list or not.  The first
     * element is either the element reserved for 0, if TRUE, or the element
     * after it, if FALSE */

    bool* offset = get_invlist_offset_addr(invlist);
    UV* zero_addr = (UV *) SvPVX(invlist);

    PERL_ARGS_ASSERT__INVLIST_ARRAY_INIT;

    /* Must be empty */
    assert(! _invlist_len(invlist));

    *zero_addr = 0;

    /* 1^1 = 0; 1^0 = 1 */
    *offset = 1 ^ will_have_0;
    return zero_addr + *offset;
}

STATIC void
S_invlist_replace_list_destroys_src(pTHX_ SV * dest, SV * src)
{
    /* Replaces the inversion list in 'dest' with the one from 'src'.  It
     * steals the list from 'src', so 'src' is made to have a NULL list.  This
     * is similar to what SvSetMagicSV() would do, if it were implemented on
     * inversion lists, though this routine avoids a copy */

    const UV src_len          = _invlist_len(src);
    const bool src_offset     = *get_invlist_offset_addr(src);
    const STRLEN src_byte_len = SvLEN(src);
    char * array              = SvPVX(src);

#ifndef NO_TAINT_SUPPORT
    const int oldtainted = TAINT_get;
#endif

    PERL_ARGS_ASSERT_INVLIST_REPLACE_LIST_DESTROYS_SRC;

    assert(is_invlist(src));
    assert(is_invlist(dest));
    assert(! invlist_is_iterating(src));
    assert(SvCUR(src) == 0 || SvCUR(src) < SvLEN(src));

    /* Make sure it ends in the right place with a NUL, as our inversion list
     * manipulations aren't careful to keep this true, but sv_usepvn_flags()
     * asserts it */
    array[src_byte_len - 1] = '\0';

    TAINT_NOT;      /* Otherwise it breaks */
    sv_usepvn_flags(dest,
                    (char *) array,
                    src_byte_len - 1,

                    /* This flag is documented to cause a copy to be avoided */
                    SV_HAS_TRAILING_NUL);
    TAINT_set(oldtainted);
    SvPV_set(src, 0);
    SvLEN_set(src, 0);
    SvCUR_set(src, 0);

    /* Finish up copying over the other fields in an inversion list */
    *get_invlist_offset_addr(dest) = src_offset;
    invlist_set_len(dest, src_len, src_offset);
    *get_invlist_previous_index_addr(dest) = 0;
    invlist_iterfinish(dest);
}

PERL_STATIC_INLINE IV*
S_get_invlist_previous_index_addr(SV* invlist)
{
    /* Return the address of the IV that is reserved to hold the cached index
     * */
    PERL_ARGS_ASSERT_GET_INVLIST_PREVIOUS_INDEX_ADDR;

    assert(is_invlist(invlist));

    return &(((XINVLIST*) SvANY(invlist))->prev_index);
}

PERL_STATIC_INLINE IV
S_invlist_previous_index(SV* const invlist)
{
    /* Returns cached index of previous search */

    PERL_ARGS_ASSERT_INVLIST_PREVIOUS_INDEX;

    return *get_invlist_previous_index_addr(invlist);
}

PERL_STATIC_INLINE void
S_invlist_set_previous_index(SV* const invlist, const IV index)
{
    /* Caches <index> for later retrieval */

    PERL_ARGS_ASSERT_INVLIST_SET_PREVIOUS_INDEX;

    assert(index == 0 || index < (int) _invlist_len(invlist));

    *get_invlist_previous_index_addr(invlist) = index;
}

PERL_STATIC_INLINE void
S_invlist_trim(SV* invlist)
{
    /* Free the not currently-being-used space in an inversion list */

    /* But don't free up the space needed for the 0 UV that is always at the
     * beginning of the list, nor the trailing NUL */
    const UV min_size = TO_INTERNAL_SIZE(1) + 1;

    PERL_ARGS_ASSERT_INVLIST_TRIM;

    assert(is_invlist(invlist));

    SvPV_renew(invlist, MAX(min_size, SvCUR(invlist) + 1));
}

PERL_STATIC_INLINE void
S_invlist_clear(pTHX_ SV* invlist)    /* Empty the inversion list */
{
    PERL_ARGS_ASSERT_INVLIST_CLEAR;

    assert(is_invlist(invlist));

    invlist_set_len(invlist, 0, 0);
    invlist_trim(invlist);
}

PERL_STATIC_INLINE UV
S_invlist_max(const SV* const invlist)
{
    /* Returns the maximum number of elements storable in the inversion list's
     * array, without having to realloc() */

    PERL_ARGS_ASSERT_INVLIST_MAX;

    assert(is_invlist(invlist));

    /* Assumes worst case, in which the 0 element is not counted in the
     * inversion list, so subtracts 1 for that */
    return SvLEN(invlist) == 0  /* This happens under _new_invlist_C_array */
           ? FROM_INTERNAL_SIZE(SvCUR(invlist)) - 1
           : FROM_INTERNAL_SIZE(SvLEN(invlist)) - 1;
}

STATIC void
S_initialize_invlist_guts(pTHX_ SV* invlist, const Size_t initial_size)
{
    PERL_ARGS_ASSERT_INITIALIZE_INVLIST_GUTS;

    /* First 1 is in case the zero element isn't in the list; second 1 is for
     * trailing NUL */
    SvGROW(invlist, TO_INTERNAL_SIZE(initial_size + 1) + 1);
    invlist_set_len(invlist, 0, 0);

    /* Force iterinit() to be used to get iteration to work */
    invlist_iterfinish(invlist);

    *get_invlist_previous_index_addr(invlist) = 0;
    SvPOK_on(invlist);  /* This allows B to extract the PV */
}

SV*
Perl__new_invlist(pTHX_ IV initial_size)
{

    /* Return a pointer to a newly constructed inversion list, with enough
     * space to store 'initial_size' elements.  If that number is negative, a
     * system default is used instead */

    SV* new_list;

    if (initial_size < 0) {
        initial_size = 10;
    }

    new_list = newSV_type(SVt_INVLIST);
    initialize_invlist_guts(new_list, initial_size);

    return new_list;
}

SV*
Perl__new_invlist_C_array(pTHX_ const UV* const list)
{
    /* Return a pointer to a newly constructed inversion list, initialized to
     * point to <list>, which has to be in the exact correct inversion list
     * form, including internal fields.  Thus this is a dangerous routine that
     * should not be used in the wrong hands.  The passed in 'list' contains
     * several header fields at the beginning that are not part of the
     * inversion list body proper */

    const STRLEN length = (STRLEN) list[0];
    const UV version_id =          list[1];
    const bool offset   =    cBOOL(list[2]);
#define HEADER_LENGTH 3
    /* If any of the above changes in any way, you must change HEADER_LENGTH
     * (if appropriate) and regenerate INVLIST_VERSION_ID by running
     *      perl -E 'say int(rand 2**31-1)'
     */
#define INVLIST_VERSION_ID 148565664 /* This is a combination of a version and
                                        data structure type, so that one being
                                        passed in can be validated to be an
                                        inversion list of the correct vintage.
                                       */

    SV* invlist = newSV_type(SVt_INVLIST);

    PERL_ARGS_ASSERT__NEW_INVLIST_C_ARRAY;

    if (version_id != INVLIST_VERSION_ID) {
        Perl_croak(aTHX_ "panic: Incorrect version for previously generated inversion list");
    }

    /* The generated array passed in includes header elements that aren't part
     * of the list proper, so start it just after them */
    SvPV_set(invlist, (char *) (list + HEADER_LENGTH));

    SvLEN_set(invlist, 0);  /* Means we own the contents, and the system
                               shouldn't touch it */

    *(get_invlist_offset_addr(invlist)) = offset;

    /* The 'length' passed to us is the physical number of elements in the
     * inversion list.  But if there is an offset the logical number is one
     * less than that */
    invlist_set_len(invlist, length  - offset, offset);

    invlist_set_previous_index(invlist, 0);

    /* Initialize the iteration pointer. */
    invlist_iterfinish(invlist);

    SvREADONLY_on(invlist);
    SvPOK_on(invlist);

    return invlist;
}

STATIC void
S__append_range_to_invlist(pTHX_ SV* const invlist,
                                 const UV start, const UV end)
{
   /* Subject to change or removal.  Append the range from 'start' to 'end' at
    * the end of the inversion list.  The range must be above any existing
    * ones. */

    UV* array;
    UV max = invlist_max(invlist);
    UV len = _invlist_len(invlist);
    bool offset;

    PERL_ARGS_ASSERT__APPEND_RANGE_TO_INVLIST;

    if (len == 0) { /* Empty lists must be initialized */
        offset = start != 0;
        array = _invlist_array_init(invlist, ! offset);
    }
    else {
        /* Here, the existing list is non-empty. The current max entry in the
         * list is generally the first value not in the set, except when the
         * set extends to the end of permissible values, in which case it is
         * the first entry in that final set, and so this call is an attempt to
         * append out-of-order */

        UV final_element = len - 1;
        array = invlist_array(invlist);
        if (   array[final_element] > start
            || ELEMENT_RANGE_MATCHES_INVLIST(final_element))
        {
            Perl_croak(aTHX_ "panic: attempting to append to an inversion list, but wasn't at the end of the list, final=%" UVuf ", start=%" UVuf ", match=%c",
                     array[final_element], start,
                     ELEMENT_RANGE_MATCHES_INVLIST(final_element) ? 't' : 'f');
        }

        /* Here, it is a legal append.  If the new range begins 1 above the end
         * of the range below it, it is extending the range below it, so the
         * new first value not in the set is one greater than the newly
         * extended range.  */
        offset = *get_invlist_offset_addr(invlist);
        if (array[final_element] == start) {
            if (end != UV_MAX) {
                array[final_element] = end + 1;
            }
            else {
                /* But if the end is the maximum representable on the machine,
                 * assume that infinity was actually what was meant.  Just let
                 * the range that this would extend to have no end */
                invlist_set_len(invlist, len - 1, offset);
            }
            return;
        }
    }

    /* Here the new range doesn't extend any existing set.  Add it */

    len += 2;   /* Includes an element each for the start and end of range */

    /* If wll overflow the existing space, extend, which may cause the array to
     * be moved */
    if (max < len) {
        invlist_extend(invlist, len);

        /* Have to set len here to avoid assert failure in invlist_array() */
        invlist_set_len(invlist, len, offset);

        array = invlist_array(invlist);
    }
    else {
        invlist_set_len(invlist, len, offset);
    }

    /* The next item on the list starts the range, the one after that is
     * one past the new range.  */
    array[len - 2] = start;
    if (end != UV_MAX) {
        array[len - 1] = end + 1;
    }
    else {
        /* But if the end is the maximum representable on the machine, just let
         * the range have no end */
        invlist_set_len(invlist, len - 1, offset);
    }
}

SSize_t
Perl__invlist_search(SV* const invlist, const UV cp)
{
    /* Searches the inversion list for the entry that contains the input code
     * point <cp>.  If <cp> is not in the list, -1 is returned.  Otherwise, the
     * return value is the index into the list's array of the range that
     * contains <cp>, that is, 'i' such that
     *  array[i] <= cp < array[i+1]
     */

    IV low = 0;
    IV mid;
    IV high = _invlist_len(invlist);
    const IV highest_element = high - 1;
    const UV* array;

    PERL_ARGS_ASSERT__INVLIST_SEARCH;

    /* If list is empty, return failure. */
    if (UNLIKELY(high == 0)) {
        return -1;
    }

    /* (We can't get the array unless we know the list is non-empty) */
    array = invlist_array(invlist);

    mid = invlist_previous_index(invlist);
    assert(mid >=0);
    if (UNLIKELY(mid > highest_element)) {
        mid = highest_element;
    }

    /* <mid> contains the cache of the result of the previous call to this
     * function (0 the first time).  See if this call is for the same result,
     * or if it is for mid-1.  This is under the theory that calls to this
     * function will often be for related code points that are near each other.
     * And benchmarks show that caching gives better results.  We also test
     * here if the code point is within the bounds of the list.  These tests
     * replace others that would have had to be made anyway to make sure that
     * the array bounds were not exceeded, and these give us extra information
     * at the same time */
    if (cp >= array[mid]) {
        if (cp >= array[highest_element]) {
            return highest_element;
        }

        /* Here, array[mid] <= cp < array[highest_element].  This means that
         * the final element is not the answer, so can exclude it; it also
         * means that <mid> is not the final element, so can refer to 'mid + 1'
         * safely */
        if (cp < array[mid + 1]) {
            return mid;
        }
        high--;
        low = mid + 1;
    }
    else { /* cp < aray[mid] */
        if (cp < array[0]) { /* Fail if outside the array */
            return -1;
        }
        high = mid;
        if (cp >= array[mid - 1]) {
            goto found_entry;
        }
    }

    /* Binary search.  What we are looking for is <i> such that
     *  array[i] <= cp < array[i+1]
     * The loop below converges on the i+1.  Note that there may not be an
     * (i+1)th element in the array, and things work nonetheless */
    while (low < high) {
        mid = (low + high) / 2;
        assert(mid <= highest_element);
        if (array[mid] <= cp) { /* cp >= array[mid] */
            low = mid + 1;

            /* We could do this extra test to exit the loop early.
            if (cp < array[low]) {
                return mid;
            }
            */
        }
        else { /* cp < array[mid] */
            high = mid;
        }
    }

  found_entry:
    high--;
    invlist_set_previous_index(invlist, high);
    return high;
}

void
Perl__invlist_union_maybe_complement_2nd(pTHX_ SV* const a, SV* const b,
                                         const bool complement_b, SV** output)
{
    /* Take the union of two inversion lists and point '*output' to it.  On
     * input, '*output' MUST POINT TO NULL OR TO AN SV* INVERSION LIST (possibly
     * even 'a' or 'b').  If to an inversion list, the contents of the original
     * list will be replaced by the union.  The first list, 'a', may be
     * NULL, in which case a copy of the second list is placed in '*output'.
     * If 'complement_b' is TRUE, the union is taken of the complement
     * (inversion) of 'b' instead of b itself.
     *
     * The basis for this comes from "Unicode Demystified" Chapter 13 by
     * Richard Gillam, published by Addison-Wesley, and explained at some
     * length there.  The preface says to incorporate its examples into your
     * code at your own risk.
     *
     * The algorithm is like a merge sort. */

    const UV* array_a;    /* a's array */
    const UV* array_b;
    UV len_a;       /* length of a's array */
    UV len_b;

    SV* u;                      /* the resulting union */
    UV* array_u;
    UV len_u = 0;

    UV i_a = 0;             /* current index into a's array */
    UV i_b = 0;
    UV i_u = 0;

    /* running count, as explained in the algorithm source book; items are
     * stopped accumulating and are output when the count changes to/from 0.
     * The count is incremented when we start a range that's in an input's set,
     * and decremented when we start a range that's not in a set.  So this
     * variable can be 0, 1, or 2.  When it is 0 neither input is in their set,
     * and hence nothing goes into the union; 1, just one of the inputs is in
     * its set (and its current range gets added to the union); and 2 when both
     * inputs are in their sets.  */
    UV count = 0;

    PERL_ARGS_ASSERT__INVLIST_UNION_MAYBE_COMPLEMENT_2ND;
    assert(a != b);
    assert(*output == NULL || is_invlist(*output));

    len_b = _invlist_len(b);
    if (len_b == 0) {

        /* Here, 'b' is empty, hence it's complement is all possible code
         * points.  So if the union includes the complement of 'b', it includes
         * everything, and we need not even look at 'a'.  It's easiest to
         * create a new inversion list that matches everything.  */
        if (complement_b) {
            SV* everything = _add_range_to_invlist(NULL, 0, UV_MAX);

            if (*output == NULL) { /* If the output didn't exist, just point it
                                      at the new list */
                *output = everything;
            }
            else { /* Otherwise, replace its contents with the new list */
                invlist_replace_list_destroys_src(*output, everything);
                SvREFCNT_dec_NN(everything);
            }

            return;
        }

        /* Here, we don't want the complement of 'b', and since 'b' is empty,
         * the union will come entirely from 'a'.  If 'a' is NULL or empty, the
         * output will be empty */

        if (a == NULL || _invlist_len(a) == 0) {
            if (*output == NULL) {
                *output = _new_invlist(0);
            }
            else {
                invlist_clear(*output);
            }
            return;
        }

        /* Here, 'a' is not empty, but 'b' is, so 'a' entirely determines the
         * union.  We can just return a copy of 'a' if '*output' doesn't point
         * to an existing list */
        if (*output == NULL) {
            *output = invlist_clone(a, NULL);
            return;
        }

        /* If the output is to overwrite 'a', we have a no-op, as it's
         * already in 'a' */
        if (*output == a) {
            return;
        }

        /* Here, '*output' is to be overwritten by 'a' */
        u = invlist_clone(a, NULL);
        invlist_replace_list_destroys_src(*output, u);
        SvREFCNT_dec_NN(u);

        return;
    }

    /* Here 'b' is not empty.  See about 'a' */

    if (a == NULL || ((len_a = _invlist_len(a)) == 0)) {

        /* Here, 'a' is empty (and b is not).  That means the union will come
         * entirely from 'b'.  If '*output' is NULL, we can directly return a
         * clone of 'b'.  Otherwise, we replace the contents of '*output' with
         * the clone */

        SV ** dest = (*output == NULL) ? output : &u;
        *dest = invlist_clone(b, NULL);
        if (complement_b) {
            _invlist_invert(*dest);
        }

        if (dest == &u) {
            invlist_replace_list_destroys_src(*output, u);
            SvREFCNT_dec_NN(u);
        }

        return;
    }

    /* Here both lists exist and are non-empty */
    array_a = invlist_array(a);
    array_b = invlist_array(b);

    /* If are to take the union of 'a' with the complement of b, set it
     * up so are looking at b's complement. */
    if (complement_b) {

        /* To complement, we invert: if the first element is 0, remove it.  To
         * do this, we just pretend the array starts one later */
        if (array_b[0] == 0) {
            array_b++;
            len_b--;
        }
        else {

            /* But if the first element is not zero, we pretend the list starts
             * at the 0 that is always stored immediately before the array. */
            array_b--;
            len_b++;
        }
    }

    /* Size the union for the worst case: that the sets are completely
     * disjoint */
    u = _new_invlist(len_a + len_b);

    /* Will contain U+0000 if either component does */
    array_u = _invlist_array_init(u, (    len_a > 0 && array_a[0] == 0)
                                      || (len_b > 0 && array_b[0] == 0));

    /* Go through each input list item by item, stopping when have exhausted
     * one of them */
    while (i_a < len_a && i_b < len_b) {
        UV cp;      /* The element to potentially add to the union's array */
        bool cp_in_set;   /* is it in the input list's set or not */

        /* We need to take one or the other of the two inputs for the union.
         * Since we are merging two sorted lists, we take the smaller of the
         * next items.  In case of a tie, we take first the one that is in its
         * set.  If we first took the one not in its set, it would decrement
         * the count, possibly to 0 which would cause it to be output as ending
         * the range, and the next time through we would take the same number,
         * and output it again as beginning the next range.  By doing it the
         * opposite way, there is no possibility that the count will be
         * momentarily decremented to 0, and thus the two adjoining ranges will
         * be seamlessly merged.  (In a tie and both are in the set or both not
         * in the set, it doesn't matter which we take first.) */
        if (       array_a[i_a] < array_b[i_b]
            || (   array_a[i_a] == array_b[i_b]
                && ELEMENT_RANGE_MATCHES_INVLIST(i_a)))
        {
            cp_in_set = ELEMENT_RANGE_MATCHES_INVLIST(i_a);
            cp = array_a[i_a++];
        }
        else {
            cp_in_set = ELEMENT_RANGE_MATCHES_INVLIST(i_b);
            cp = array_b[i_b++];
        }

        /* Here, have chosen which of the two inputs to look at.  Only output
         * if the running count changes to/from 0, which marks the
         * beginning/end of a range that's in the set */
        if (cp_in_set) {
            if (count == 0) {
                array_u[i_u++] = cp;
            }
            count++;
        }
        else {
            count--;
            if (count == 0) {
                array_u[i_u++] = cp;
            }
        }
    }


    /* The loop above increments the index into exactly one of the input lists
     * each iteration, and ends when either index gets to its list end.  That
     * means the other index is lower than its end, and so something is
     * remaining in that one.  We decrement 'count', as explained below, if
     * that list is in its set.  (i_a and i_b each currently index the element
     * beyond the one we care about.) */
    if (   (i_a != len_a && PREV_RANGE_MATCHES_INVLIST(i_a))
        || (i_b != len_b && PREV_RANGE_MATCHES_INVLIST(i_b)))
    {
        count--;
    }

    /* Above we decremented 'count' if the list that had unexamined elements in
     * it was in its set.  This has made it so that 'count' being non-zero
     * means there isn't anything left to output; and 'count' equal to 0 means
     * that what is left to output is precisely that which is left in the
     * non-exhausted input list.
     *
     * To see why, note first that the exhausted input obviously has nothing
     * left to add to the union.  If it was in its set at its end, that means
     * the set extends from here to the platform's infinity, and hence so does
     * the union and the non-exhausted set is irrelevant.  The exhausted set
     * also contributed 1 to 'count'.  If 'count' was 2, it got decremented to
     * 1, but if it was 1, the non-exhausted set wasn't in its set, and so
     * 'count' remains at 1.  This is consistent with the decremented 'count'
     * != 0 meaning there's nothing left to add to the union.
     *
     * But if the exhausted input wasn't in its set, it contributed 0 to
     * 'count', and the rest of the union will be whatever the other input is.
     * If 'count' was 0, neither list was in its set, and 'count' remains 0;
     * otherwise it gets decremented to 0.  This is consistent with 'count'
     * == 0 meaning the remainder of the union is whatever is left in the
     * non-exhausted list. */
    if (count != 0) {
        len_u = i_u;
    }
    else {
        IV copy_count = len_a - i_a;
        if (copy_count > 0) {   /* The non-exhausted input is 'a' */
            Copy(array_a + i_a, array_u + i_u, copy_count, UV);
        }
        else { /* The non-exhausted input is b */
            copy_count = len_b - i_b;
            Copy(array_b + i_b, array_u + i_u, copy_count, UV);
        }
        len_u = i_u + copy_count;
    }

    /* Set the result to the final length, which can change the pointer to
     * array_u, so re-find it.  (Note that it is unlikely that this will
     * change, as we are shrinking the space, not enlarging it) */
    if (len_u != _invlist_len(u)) {
        invlist_set_len(u, len_u, *get_invlist_offset_addr(u));
        invlist_trim(u);
        array_u = invlist_array(u);
    }

    if (*output == NULL) {  /* Simply return the new inversion list */
        *output = u;
    }
    else {
        /* Otherwise, overwrite the inversion list that was in '*output'.  We
         * could instead free '*output', and then set it to 'u', but experience
         * has shown [perl #127392] that if the input is a mortal, we can get a
         * huge build-up of these during regex compilation before they get
         * freed. */
        invlist_replace_list_destroys_src(*output, u);
        SvREFCNT_dec_NN(u);
    }

    return;
}

void
Perl__invlist_intersection_maybe_complement_2nd(pTHX_ SV* const a, SV* const b,
                                               const bool complement_b, SV** i)
{
    /* Take the intersection of two inversion lists and point '*i' to it.  On
     * input, '*i' MUST POINT TO NULL OR TO AN SV* INVERSION LIST (possibly
     * even 'a' or 'b').  If to an inversion list, the contents of the original
     * list will be replaced by the intersection.  The first list, 'a', may be
     * NULL, in which case '*i' will be an empty list.  If 'complement_b' is
     * TRUE, the result will be the intersection of 'a' and the complement (or
     * inversion) of 'b' instead of 'b' directly.
     *
     * The basis for this comes from "Unicode Demystified" Chapter 13 by
     * Richard Gillam, published by Addison-Wesley, and explained at some
     * length there.  The preface says to incorporate its examples into your
     * code at your own risk.  In fact, it had bugs
     *
     * The algorithm is like a merge sort, and is essentially the same as the
     * union above
     */

    const UV* array_a;          /* a's array */
    const UV* array_b;
    UV len_a;   /* length of a's array */
    UV len_b;

    SV* r;                   /* the resulting intersection */
    UV* array_r;
    UV len_r = 0;

    UV i_a = 0;             /* current index into a's array */
    UV i_b = 0;
    UV i_r = 0;

    /* running count of how many of the two inputs are postitioned at ranges
     * that are in their sets.  As explained in the algorithm source book,
     * items are stopped accumulating and are output when the count changes
     * to/from 2.  The count is incremented when we start a range that's in an
     * input's set, and decremented when we start a range that's not in a set.
     * Only when it is 2 are we in the intersection. */
    UV count = 0;

    PERL_ARGS_ASSERT__INVLIST_INTERSECTION_MAYBE_COMPLEMENT_2ND;
    assert(a != b);
    assert(*i == NULL || is_invlist(*i));

    /* Special case if either one is empty */
    len_a = (a == NULL) ? 0 : _invlist_len(a);
    if ((len_a == 0) || ((len_b = _invlist_len(b)) == 0)) {
        if (len_a != 0 && complement_b) {

            /* Here, 'a' is not empty, therefore from the enclosing 'if', 'b'
             * must be empty.  Here, also we are using 'b's complement, which
             * hence must be every possible code point.  Thus the intersection
             * is simply 'a'. */

            if (*i == a) {  /* No-op */
                return;
            }

            if (*i == NULL) {
                *i = invlist_clone(a, NULL);
                return;
            }

            r = invlist_clone(a, NULL);
            invlist_replace_list_destroys_src(*i, r);
            SvREFCNT_dec_NN(r);
            return;
        }

        /* Here, 'a' or 'b' is empty and not using the complement of 'b'.  The
         * intersection must be empty */
        if (*i == NULL) {
            *i = _new_invlist(0);
            return;
        }

        invlist_clear(*i);
        return;
    }

    /* Here both lists exist and are non-empty */
    array_a = invlist_array(a);
    array_b = invlist_array(b);

    /* If are to take the intersection of 'a' with the complement of b, set it
     * up so are looking at b's complement. */
    if (complement_b) {

        /* To complement, we invert: if the first element is 0, remove it.  To
         * do this, we just pretend the array starts one later */
        if (array_b[0] == 0) {
            array_b++;
            len_b--;
        }
        else {

            /* But if the first element is not zero, we pretend the list starts
             * at the 0 that is always stored immediately before the array. */
            array_b--;
            len_b++;
        }
    }

    /* Size the intersection for the worst case: that the intersection ends up
     * fragmenting everything to be completely disjoint */
    r= _new_invlist(len_a + len_b);

    /* Will contain U+0000 iff both components do */
    array_r = _invlist_array_init(r,    len_a > 0 && array_a[0] == 0
                                     && len_b > 0 && array_b[0] == 0);

    /* Go through each list item by item, stopping when have exhausted one of
     * them */
    while (i_a < len_a && i_b < len_b) {
        UV cp;      /* The element to potentially add to the intersection's
                       array */
        bool cp_in_set; /* Is it in the input list's set or not */

        /* We need to take one or the other of the two inputs for the
         * intersection.  Since we are merging two sorted lists, we take the
         * smaller of the next items.  In case of a tie, we take first the one
         * that is not in its set (a difference from the union algorithm).  If
         * we first took the one in its set, it would increment the count,
         * possibly to 2 which would cause it to be output as starting a range
         * in the intersection, and the next time through we would take that
         * same number, and output it again as ending the set.  By doing the
         * opposite of this, there is no possibility that the count will be
         * momentarily incremented to 2.  (In a tie and both are in the set or
         * both not in the set, it doesn't matter which we take first.) */
        if (       array_a[i_a] < array_b[i_b]
            || (   array_a[i_a] == array_b[i_b]
                && ! ELEMENT_RANGE_MATCHES_INVLIST(i_a)))
        {
            cp_in_set = ELEMENT_RANGE_MATCHES_INVLIST(i_a);
            cp = array_a[i_a++];
        }
        else {
            cp_in_set = ELEMENT_RANGE_MATCHES_INVLIST(i_b);
            cp= array_b[i_b++];
        }

        /* Here, have chosen which of the two inputs to look at.  Only output
         * if the running count changes to/from 2, which marks the
         * beginning/end of a range that's in the intersection */
        if (cp_in_set) {
            count++;
            if (count == 2) {
                array_r[i_r++] = cp;
            }
        }
        else {
            if (count == 2) {
                array_r[i_r++] = cp;
            }
            count--;
        }

    }

    /* The loop above increments the index into exactly one of the input lists
     * each iteration, and ends when either index gets to its list end.  That
     * means the other index is lower than its end, and so something is
     * remaining in that one.  We increment 'count', as explained below, if the
     * exhausted list was in its set.  (i_a and i_b each currently index the
     * element beyond the one we care about.) */
    if (   (i_a == len_a && PREV_RANGE_MATCHES_INVLIST(i_a))
        || (i_b == len_b && PREV_RANGE_MATCHES_INVLIST(i_b)))
    {
        count++;
    }

    /* Above we incremented 'count' if the exhausted list was in its set.  This
     * has made it so that 'count' being below 2 means there is nothing left to
     * output; otheriwse what's left to add to the intersection is precisely
     * that which is left in the non-exhausted input list.
     *
     * To see why, note first that the exhausted input obviously has nothing
     * left to affect the intersection.  If it was in its set at its end, that
     * means the set extends from here to the platform's infinity, and hence
     * anything in the non-exhausted's list will be in the intersection, and
     * anything not in it won't be.  Hence, the rest of the intersection is
     * precisely what's in the non-exhausted list  The exhausted set also
     * contributed 1 to 'count', meaning 'count' was at least 1.  Incrementing
     * it means 'count' is now at least 2.  This is consistent with the
     * incremented 'count' being >= 2 means to add the non-exhausted list to
     * the intersection.
     *
     * But if the exhausted input wasn't in its set, it contributed 0 to
     * 'count', and the intersection can't include anything further; the
     * non-exhausted set is irrelevant.  'count' was at most 1, and doesn't get
     * incremented.  This is consistent with 'count' being < 2 meaning nothing
     * further to add to the intersection. */
    if (count < 2) { /* Nothing left to put in the intersection. */
        len_r = i_r;
    }
    else { /* copy the non-exhausted list, unchanged. */
        IV copy_count = len_a - i_a;
        if (copy_count > 0) {   /* a is the one with stuff left */
            Copy(array_a + i_a, array_r + i_r, copy_count, UV);
        }
        else {  /* b is the one with stuff left */
            copy_count = len_b - i_b;
            Copy(array_b + i_b, array_r + i_r, copy_count, UV);
        }
        len_r = i_r + copy_count;
    }

    /* Set the result to the final length, which can change the pointer to
     * array_r, so re-find it.  (Note that it is unlikely that this will
     * change, as we are shrinking the space, not enlarging it) */
    if (len_r != _invlist_len(r)) {
        invlist_set_len(r, len_r, *get_invlist_offset_addr(r));
        invlist_trim(r);
        array_r = invlist_array(r);
    }

    if (*i == NULL) { /* Simply return the calculated intersection */
        *i = r;
    }
    else { /* Otherwise, replace the existing inversion list in '*i'.  We could
              instead free '*i', and then set it to 'r', but experience has
              shown [perl #127392] that if the input is a mortal, we can get a
              huge build-up of these during regex compilation before they get
              freed. */
        if (len_r) {
            invlist_replace_list_destroys_src(*i, r);
        }
        else {
            invlist_clear(*i);
        }
        SvREFCNT_dec_NN(r);
    }

    return;
}

SV*
Perl__add_range_to_invlist(pTHX_ SV* invlist, UV start, UV end)
{
    /* Add the range from 'start' to 'end' inclusive to the inversion list's
     * set.  A pointer to the inversion list is returned.  This may actually be
     * a new list, in which case the passed in one has been destroyed.  The
     * passed-in inversion list can be NULL, in which case a new one is created
     * with just the one range in it.  The new list is not necessarily
     * NUL-terminated.  Space is not freed if the inversion list shrinks as a
     * result of this function.  The gain would not be large, and in many
     * cases, this is called multiple times on a single inversion list, so
     * anything freed may almost immediately be needed again.
     *
     * This used to mostly call the 'union' routine, but that is much more
     * heavyweight than really needed for a single range addition */

    UV* array;              /* The array implementing the inversion list */
    UV len;                 /* How many elements in 'array' */
    SSize_t i_s;            /* index into the invlist array where 'start'
                               should go */
    SSize_t i_e = 0;        /* And the index where 'end' should go */
    UV cur_highest;         /* The highest code point in the inversion list
                               upon entry to this function */

    /* This range becomes the whole inversion list if none already existed */
    if (invlist == NULL) {
        invlist = _new_invlist(2);
        _append_range_to_invlist(invlist, start, end);
        return invlist;
    }

    /* Likewise, if the inversion list is currently empty */
    len = _invlist_len(invlist);
    if (len == 0) {
        _append_range_to_invlist(invlist, start, end);
        return invlist;
    }

    /* Starting here, we have to know the internals of the list */
    array = invlist_array(invlist);

    /* If the new range ends higher than the current highest ... */
    cur_highest = invlist_highest(invlist);
    if (end > cur_highest) {

        /* If the whole range is higher, we can just append it */
        if (start > cur_highest) {
            _append_range_to_invlist(invlist, start, end);
            return invlist;
        }

        /* Otherwise, add the portion that is higher ... */
        _append_range_to_invlist(invlist, cur_highest + 1, end);

        /* ... and continue on below to handle the rest.  As a result of the
         * above append, we know that the index of the end of the range is the
         * final even numbered one of the array.  Recall that the final element
         * always starts a range that extends to infinity.  If that range is in
         * the set (meaning the set goes from here to infinity), it will be an
         * even index, but if it isn't in the set, it's odd, and the final
         * range in the set is one less, which is even. */
        if (end == UV_MAX) {
            i_e = len;
        }
        else {
            i_e = len - 2;
        }
    }

    /* We have dealt with appending, now see about prepending.  If the new
     * range starts lower than the current lowest ... */
    if (start < array[0]) {

        /* Adding something which has 0 in it is somewhat tricky, and uncommon.
         * Let the union code handle it, rather than having to know the
         * trickiness in two code places.  */
        if (UNLIKELY(start == 0)) {
            SV* range_invlist;

            range_invlist = _new_invlist(2);
            _append_range_to_invlist(range_invlist, start, end);

            _invlist_union(invlist, range_invlist, &invlist);

            SvREFCNT_dec_NN(range_invlist);

            return invlist;
        }

        /* If the whole new range comes before the first entry, and doesn't
         * extend it, we have to insert it as an additional range */
        if (end < array[0] - 1) {
            i_s = i_e = -1;
            goto splice_in_new_range;
        }

        /* Here the new range adjoins the existing first range, extending it
         * downwards. */
        array[0] = start;

        /* And continue on below to handle the rest.  We know that the index of
         * the beginning of the range is the first one of the array */
        i_s = 0;
    }
    else { /* Not prepending any part of the new range to the existing list.
            * Find where in the list it should go.  This finds i_s, such that:
            *     invlist[i_s] <= start < array[i_s+1]
            */
        i_s = _invlist_search(invlist, start);
    }

    /* At this point, any extending before the beginning of the inversion list
     * and/or after the end has been done.  This has made it so that, in the
     * code below, each endpoint of the new range is either in a range that is
     * in the set, or is in a gap between two ranges that are.  This means we
     * don't have to worry about exceeding the array bounds.
     *
     * Find where in the list the new range ends (but we can skip this if we
     * have already determined what it is, or if it will be the same as i_s,
     * which we already have computed) */
    if (i_e == 0) {
        i_e = (start == end)
              ? i_s
              : _invlist_search(invlist, end);
    }

    /* Here generally invlist[i_e] <= end < array[i_e+1].  But if invlist[i_e]
     * is a range that goes to infinity there is no element at invlist[i_e+1],
     * so only the first relation holds. */

    if ( ! ELEMENT_RANGE_MATCHES_INVLIST(i_s)) {

        /* Here, the ranges on either side of the beginning of the new range
         * are in the set, and this range starts in the gap between them.
         *
         * The new range extends the range above it downwards if the new range
         * ends at or above that range's start */
        const bool extends_the_range_above = (   end == UV_MAX
                                              || end + 1 >= array[i_s+1]);

        /* The new range extends the range below it upwards if it begins just
         * after where that range ends */
        if (start == array[i_s]) {

            /* If the new range fills the entire gap between the other ranges,
             * they will get merged together.  Other ranges may also get
             * merged, depending on how many of them the new range spans.  In
             * the general case, we do the merge later, just once, after we
             * figure out how many to merge.  But in the case where the new
             * range exactly spans just this one gap (possibly extending into
             * the one above), we do the merge here, and an early exit.  This
             * is done here to avoid having to special case later. */
            if (i_e - i_s <= 1) {

                /* If i_e - i_s == 1, it means that the new range terminates
                 * within the range above, and hence 'extends_the_range_above'
                 * must be true.  (If the range above it extends to infinity,
                 * 'i_s+2' will be above the array's limit, but 'len-i_s-2'
                 * will be 0, so no harm done.) */
                if (extends_the_range_above) {
                    Move(array + i_s + 2, array + i_s, len - i_s - 2, UV);
                    invlist_set_len(invlist,
                                    len - 2,
                                    *(get_invlist_offset_addr(invlist)));
                    return invlist;
                }

                /* Here, i_e must == i_s.  We keep them in sync, as they apply
                 * to the same range, and below we are about to decrement i_s
                 * */
                i_e--;
            }

            /* Here, the new range is adjacent to the one below.  (It may also
             * span beyond the range above, but that will get resolved later.)
             * Extend the range below to include this one. */
            array[i_s] = (end == UV_MAX) ? UV_MAX : end + 1;
            i_s--;
            start = array[i_s];
        }
        else if (extends_the_range_above) {

            /* Here the new range only extends the range above it, but not the
             * one below.  It merges with the one above.  Again, we keep i_e
             * and i_s in sync if they point to the same range */
            if (i_e == i_s) {
                i_e++;
            }
            i_s++;
            array[i_s] = start;
        }
    }

    /* Here, we've dealt with the new range start extending any adjoining
     * existing ranges.
     *
     * If the new range extends to infinity, it is now the final one,
     * regardless of what was there before */
    if (UNLIKELY(end == UV_MAX)) {
        invlist_set_len(invlist, i_s + 1, *(get_invlist_offset_addr(invlist)));
        return invlist;
    }

    /* If i_e started as == i_s, it has also been dealt with,
     * and been updated to the new i_s, which will fail the following if */
    if (! ELEMENT_RANGE_MATCHES_INVLIST(i_e)) {

        /* Here, the ranges on either side of the end of the new range are in
         * the set, and this range ends in the gap between them.
         *
         * If this range is adjacent to (hence extends) the range above it, it
         * becomes part of that range; likewise if it extends the range below,
         * it becomes part of that range */
        if (end + 1 == array[i_e+1]) {
            i_e++;
            array[i_e] = start;
        }
        else if (start <= array[i_e]) {
            array[i_e] = end + 1;
            i_e--;
        }
    }

    if (i_s == i_e) {

        /* If the range fits entirely in an existing range (as possibly already
         * extended above), it doesn't add anything new */
        if (ELEMENT_RANGE_MATCHES_INVLIST(i_s)) {
            return invlist;
        }

        /* Here, no part of the range is in the list.  Must add it.  It will
         * occupy 2 more slots */
      splice_in_new_range:

        invlist_extend(invlist, len + 2);
        array = invlist_array(invlist);
        /* Move the rest of the array down two slots. Don't include any
         * trailing NUL */
        Move(array + i_e + 1, array + i_e + 3, len - i_e - 1, UV);

        /* Do the actual splice */
        array[i_e+1] = start;
        array[i_e+2] = end + 1;
        invlist_set_len(invlist, len + 2, *(get_invlist_offset_addr(invlist)));
        return invlist;
    }

    /* Here the new range crossed the boundaries of a pre-existing range.  The
     * code above has adjusted things so that both ends are in ranges that are
     * in the set.  This means everything in between must also be in the set.
     * Just squash things together */
    Move(array + i_e + 1, array + i_s + 1, len - i_e - 1, UV);
    invlist_set_len(invlist,
                    len - i_e + i_s,
                    *(get_invlist_offset_addr(invlist)));

    return invlist;
}

SV*
Perl__setup_canned_invlist(pTHX_ const STRLEN size, const UV element0,
                                 UV** other_elements_ptr)
{
    /* Create and return an inversion list whose contents are to be populated
     * by the caller.  The caller gives the number of elements (in 'size') and
     * the very first element ('element0').  This function will set
     * '*other_elements_ptr' to an array of UVs, where the remaining elements
     * are to be placed.
     *
     * Obviously there is some trust involved that the caller will properly
     * fill in the other elements of the array.
     *
     * (The first element needs to be passed in, as the underlying code does
     * things differently depending on whether it is zero or non-zero) */

    SV* invlist = _new_invlist(size);
    bool offset;

    PERL_ARGS_ASSERT__SETUP_CANNED_INVLIST;

    invlist = add_cp_to_invlist(invlist, element0);
    offset = *get_invlist_offset_addr(invlist);

    invlist_set_len(invlist, size, offset);
    *other_elements_ptr = invlist_array(invlist) + 1;
    return invlist;
}

#endif

#ifndef PERL_IN_XSUB_RE
void
Perl__invlist_invert(pTHX_ SV* const invlist)
{
    /* Complement the input inversion list.  This adds a 0 if the list didn't
     * have a zero; removes it otherwise.  As described above, the data
     * structure is set up so that this is very efficient */

    PERL_ARGS_ASSERT__INVLIST_INVERT;

    assert(! invlist_is_iterating(invlist));

    /* The inverse of matching nothing is matching everything */
    if (_invlist_len(invlist) == 0) {
        _append_range_to_invlist(invlist, 0, UV_MAX);
        return;
    }

    *get_invlist_offset_addr(invlist) = ! *get_invlist_offset_addr(invlist);
}

SV*
Perl_invlist_clone(pTHX_ SV* const invlist, SV* new_invlist)
{
    /* Return a new inversion list that is a copy of the input one, which is
     * unchanged.  The new list will not be mortal even if the old one was. */

    const STRLEN nominal_length = _invlist_len(invlist);
    const STRLEN physical_length = SvCUR(invlist);
    const bool offset = *(get_invlist_offset_addr(invlist));

    PERL_ARGS_ASSERT_INVLIST_CLONE;

    if (new_invlist == NULL) {
        new_invlist = _new_invlist(nominal_length);
    }
    else {
        sv_upgrade(new_invlist, SVt_INVLIST);
        initialize_invlist_guts(new_invlist, nominal_length);
    }

    *(get_invlist_offset_addr(new_invlist)) = offset;
    invlist_set_len(new_invlist, nominal_length, offset);
    Copy(SvPVX(invlist), SvPVX(new_invlist), physical_length, char);

    return new_invlist;
}

#endif


#ifndef PERL_IN_XSUB_RE
void
Perl__invlist_dump(pTHX_ PerlIO *file, I32 level,
                         const char * const indent, SV* const invlist)
{
    /* Designed to be called only by do_sv_dump().  Dumps out the ranges of the
     * inversion list 'invlist' to 'file' at 'level'  Each line is prefixed by
     * the string 'indent'.  The output looks like this:
         [0] 0x000A .. 0x000D
         [2] 0x0085
         [4] 0x2028 .. 0x2029
         [6] 0x3104 .. INFTY
     * This means that the first range of code points matched by the list are
     * 0xA through 0xD; the second range contains only the single code point
     * 0x85, etc.  An inversion list is an array of UVs.  Two array elements
     * are used to define each range (except if the final range extends to
     * infinity, only a single element is needed).  The array index of the
     * first element for the corresponding range is given in brackets. */

    UV start, end;
    STRLEN count = 0;

    PERL_ARGS_ASSERT__INVLIST_DUMP;

    if (invlist_is_iterating(invlist)) {
        Perl_dump_indent(aTHX_ level, file,
             "%sCan't dump inversion list because is in middle of iterating\n",
             indent);
        return;
    }

    invlist_iterinit(invlist);
    while (invlist_iternext(invlist, &start, &end)) {
        if (end == UV_MAX) {
            Perl_dump_indent(aTHX_ level, file,
                                       "%s[%" UVuf "] 0x%04" UVXf " .. INFTY\n",
                                   indent, (UV)count, start);
        }
        else if (end != start) {
            Perl_dump_indent(aTHX_ level, file,
                                    "%s[%" UVuf "] 0x%04" UVXf " .. 0x%04" UVXf "\n",
                                indent, (UV)count, start,         end);
        }
        else {
            Perl_dump_indent(aTHX_ level, file, "%s[%" UVuf "] 0x%04" UVXf "\n",
                                            indent, (UV)count, start);
        }
        count += 2;
    }
}

#endif

#if defined(PERL_ARGS_ASSERT__INVLISTEQ) && !defined(PERL_IN_XSUB_RE)
bool
Perl__invlistEQ(pTHX_ SV* const a, SV* const b, const bool complement_b)
{
    /* Return a boolean as to if the two passed in inversion lists are
     * identical.  The final argument, if TRUE, says to take the complement of
     * the second inversion list before doing the comparison */

    const UV len_a = _invlist_len(a);
    UV len_b = _invlist_len(b);

    const UV* array_a = NULL;
    const UV* array_b = NULL;

    PERL_ARGS_ASSERT__INVLISTEQ;

    /* This code avoids accessing the arrays unless it knows the length is
     * non-zero */

    if (len_a == 0) {
        if (len_b == 0) {
            return ! complement_b;
        }
    }
    else {
        array_a = invlist_array(a);
    }

    if (len_b != 0) {
        array_b = invlist_array(b);
    }

    /* If are to compare 'a' with the complement of b, set it
     * up so are looking at b's complement. */
    if (complement_b) {

        /* The complement of nothing is everything, so <a> would have to have
         * just one element, starting at zero (ending at infinity) */
        if (len_b == 0) {
            return (len_a == 1 && array_a[0] == 0);
        }
        if (array_b[0] == 0) {

            /* Otherwise, to complement, we invert.  Here, the first element is
             * 0, just remove it.  To do this, we just pretend the array starts
             * one later */

            array_b++;
            len_b--;
        }
        else {

            /* But if the first element is not zero, we pretend the list starts
             * at the 0 that is always stored immediately before the array. */
            array_b--;
            len_b++;
        }
    }

    return    len_a == len_b
           && memEQ(array_a, array_b, len_a * sizeof(array_a[0]));

}
#endif

#undef HEADER_LENGTH
#undef TO_INTERNAL_SIZE
#undef FROM_INTERNAL_SIZE
#undef INVLIST_VERSION_ID

/* End of inversion list object */
