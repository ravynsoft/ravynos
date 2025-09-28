/*    util.c
 *
 *    Copyright (C) 1993, 1994, 1995, 1996, 1997, 1998, 1999, 2000, 2001,
 *    2002, 2003, 2004, 2005, 2006, 2007, 2008 by Larry Wall and others
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

/*
 * 'Very useful, no doubt, that was to Saruman; yet it seems that he was
 *  not content.'                                    --Gandalf to Pippin
 *
 *     [p.598 of _The Lord of the Rings_, III/xi: "The Palantír"]
 */

/* This file contains assorted utility routines.
 * Which is a polite way of saying any stuff that people couldn't think of
 * a better place for. Amongst other things, it includes the warning and
 * dieing stuff, plus wrappers for malloc code.
 */

#include "EXTERN.h"
#define PERL_IN_UTIL_C
#include "perl.h"
#include "reentr.h"

#if defined(USE_PERLIO)
#include "perliol.h" /* For PerlIOUnix_refcnt */
#endif

#ifndef PERL_MICRO
#include <signal.h>
#ifndef SIG_ERR
# define SIG_ERR ((Sighandler_t) -1)
#endif
#endif

#include <math.h>
#include <stdlib.h>

#ifdef __Lynx__
/* Missing protos on LynxOS */
int putenv(char *);
#endif

#ifdef __amigaos__
# include "amigaos4/amigaio.h"
#endif

#ifdef HAS_SELECT
# ifdef I_SYS_SELECT
#  include <sys/select.h>
# endif
#endif

#ifdef USE_C_BACKTRACE
#  ifdef I_BFD
#    define USE_BFD
#    ifdef PERL_DARWIN
#      undef USE_BFD /* BFD is useless in OS X. */
#    endif
#    ifdef USE_BFD
#      include <bfd.h>
#    endif
#  endif
#  ifdef I_DLFCN
#    include <dlfcn.h>
#  endif
#  ifdef I_EXECINFO
#    include <execinfo.h>
#  endif
#endif

#ifdef PERL_DEBUG_READONLY_COW
# include <sys/mman.h>
#endif

#define FLUSH

/* NOTE:  Do not call the next three routines directly.  Use the macros
 * in handy.h, so that we can easily redefine everything to do tracking of
 * allocated hunks back to the original New to track down any memory leaks.
 * XXX This advice seems to be widely ignored :-(   --AD  August 1996.
 */

#if defined (DEBUGGING) || defined(PERL_IMPLICIT_SYS) || defined (PERL_TRACK_MEMPOOL)
#  define ALWAYS_NEED_THX
#endif

#if defined(PERL_TRACK_MEMPOOL) && defined(PERL_DEBUG_READONLY_COW)
static void
S_maybe_protect_rw(pTHX_ struct perl_memory_debug_header *header)
{
    if (header->readonly
     && mprotect(header, header->size, PROT_READ|PROT_WRITE))
        Perl_warn(aTHX_ "mprotect for COW string %p %lu failed with %d",
                         header, header->size, errno);
}

static void
S_maybe_protect_ro(pTHX_ struct perl_memory_debug_header *header)
{
    if (header->readonly
     && mprotect(header, header->size, PROT_READ))
        Perl_warn(aTHX_ "mprotect RW for COW string %p %lu failed with %d",
                         header, header->size, errno);
}
# define maybe_protect_rw(foo) S_maybe_protect_rw(aTHX_ foo)
# define maybe_protect_ro(foo) S_maybe_protect_ro(aTHX_ foo)
#else
# define maybe_protect_rw(foo) NOOP
# define maybe_protect_ro(foo) NOOP
#endif

#if defined(PERL_TRACK_MEMPOOL) || defined(PERL_DEBUG_READONLY_COW)
 /* Use memory_debug_header */
# define USE_MDH
# if (defined(PERL_POISON) && defined(PERL_TRACK_MEMPOOL)) \
   || defined(PERL_DEBUG_READONLY_COW)
#  define MDH_HAS_SIZE
# endif
#endif

/*
=for apidoc_section $memory
=for apidoc safesysmalloc
Paranoid version of system's malloc()

=cut
*/

Malloc_t
Perl_safesysmalloc(MEM_SIZE size)
{
#ifdef ALWAYS_NEED_THX
    dTHX;
#endif
    Malloc_t ptr;
    dSAVEDERRNO;

#ifdef USE_MDH
    if (size + PERL_MEMORY_DEBUG_HEADER_SIZE < size)
        goto out_of_memory;
    size += PERL_MEMORY_DEBUG_HEADER_SIZE;
#endif
#ifdef DEBUGGING
    if ((SSize_t)size < 0)
        Perl_croak_nocontext("panic: malloc, size=%" UVuf, (UV) size);
#endif
    if (!size) size = 1;	/* malloc(0) is NASTY on our system */
    SAVE_ERRNO;
#ifdef PERL_DEBUG_READONLY_COW
    if ((ptr = mmap(0, size, PROT_READ|PROT_WRITE,
                    MAP_ANON|MAP_PRIVATE, -1, 0)) == MAP_FAILED) {
        perror("mmap failed");
        abort();
    }
#else
    ptr = (Malloc_t)PerlMem_malloc(size);
#endif
    PERL_ALLOC_CHECK(ptr);
    if (ptr != NULL) {
#ifdef USE_MDH
        struct perl_memory_debug_header *const header
            = (struct perl_memory_debug_header *)ptr;
#endif

#ifdef PERL_POISON
        PoisonNew(((char *)ptr), size, char);
#endif

#ifdef PERL_TRACK_MEMPOOL
        header->interpreter = aTHX;
        /* Link us into the list.  */
        header->prev = &PL_memory_debug_header;
        header->next = PL_memory_debug_header.next;
        PL_memory_debug_header.next = header;
        maybe_protect_rw(header->next);
        header->next->prev = header;
        maybe_protect_ro(header->next);
#  ifdef PERL_DEBUG_READONLY_COW
        header->readonly = 0;
#  endif
#endif
#ifdef MDH_HAS_SIZE
        header->size = size;
#endif
        ptr = (Malloc_t)((char*)ptr+PERL_MEMORY_DEBUG_HEADER_SIZE);
        DEBUG_m(PerlIO_printf(Perl_debug_log, "0x%" UVxf ": (%05ld) malloc %ld bytes\n",PTR2UV(ptr),(long)PL_an++,(long)size));

        /* malloc() can modify errno() even on success, but since someone
           writing perl code doesn't have any control over when perl calls
           malloc() we need to hide that.
        */
        RESTORE_ERRNO;
    }
    else {
#ifdef USE_MDH
      out_of_memory:
#endif
        {
#ifndef ALWAYS_NEED_THX
            dTHX;
#endif
            if (PL_nomemok)
                ptr =  NULL;
            else
                croak_no_mem();
        }
    }
    return ptr;
}

/*
=for apidoc safesysrealloc
Paranoid version of system's realloc()

=cut
*/

Malloc_t
Perl_safesysrealloc(Malloc_t where,MEM_SIZE size)
{
#ifdef ALWAYS_NEED_THX
    dTHX;
#endif
    Malloc_t ptr;
#ifdef PERL_DEBUG_READONLY_COW
    const MEM_SIZE oldsize = where
        ? ((struct perl_memory_debug_header *)((char *)where - PERL_MEMORY_DEBUG_HEADER_SIZE))->size
        : 0;
#endif

    if (!size) {
        safesysfree(where);
        ptr = NULL;
    }
    else if (!where) {
        ptr = safesysmalloc(size);
    }
    else {
        dSAVE_ERRNO;
        PERL_DEB(UV was_where = PTR2UV(where)); /* used in diags below */
#ifdef USE_MDH
        where = (Malloc_t)((char*)where-PERL_MEMORY_DEBUG_HEADER_SIZE);
        if (size + PERL_MEMORY_DEBUG_HEADER_SIZE < size)
            goto out_of_memory;
        size += PERL_MEMORY_DEBUG_HEADER_SIZE;
        {
            struct perl_memory_debug_header *const header
                = (struct perl_memory_debug_header *)where;

# ifdef PERL_TRACK_MEMPOOL
            if (header->interpreter != aTHX) {
                Perl_croak_nocontext("panic: realloc from wrong pool, %p!=%p",
                                     header->interpreter, aTHX);
            }
            assert(header->next->prev == header);
            assert(header->prev->next == header);
#  ifdef PERL_POISON
            if (header->size > size) {
                const MEM_SIZE freed_up = header->size - size;
                char *start_of_freed = ((char *)where) + size;
                PoisonFree(start_of_freed, freed_up, char);
            }
#  endif
# endif
# ifdef MDH_HAS_SIZE
            header->size = size;
# endif
        }
#endif
#ifdef DEBUGGING
        if ((SSize_t)size < 0)
            Perl_croak_nocontext("panic: realloc, size=%" UVuf, (UV)size);
#endif
#ifdef PERL_DEBUG_READONLY_COW
        if ((ptr = mmap(0, size, PROT_READ|PROT_WRITE,
                        MAP_ANON|MAP_PRIVATE, -1, 0)) == MAP_FAILED) {
            perror("mmap failed");
            abort();
        }
        Copy(where,ptr,oldsize < size ? oldsize : size,char);
        if (munmap(where, oldsize)) {
            perror("munmap failed");
            abort();
        }
#else
        ptr = (Malloc_t)PerlMem_realloc(where,size);
#endif
        PERL_ALLOC_CHECK(ptr);

    /* MUST do this fixup first, before doing ANYTHING else, as anything else
       might allocate memory/free/move memory, and until we do the fixup, it
       may well be chasing (and writing to) free memory.  */
        if (ptr != NULL) {
#ifdef PERL_TRACK_MEMPOOL
            struct perl_memory_debug_header *const header
                = (struct perl_memory_debug_header *)ptr;

#  ifdef PERL_POISON
            if (header->size < size) {
                const MEM_SIZE fresh = size - header->size;
                char *start_of_fresh = ((char *)ptr) + size;
                PoisonNew(start_of_fresh, fresh, char);
            }
#  endif

            maybe_protect_rw(header->next);
            header->next->prev = header;
            maybe_protect_ro(header->next);
            maybe_protect_rw(header->prev);
            header->prev->next = header;
            maybe_protect_ro(header->prev);
#endif
            ptr = (Malloc_t)((char*)ptr+PERL_MEMORY_DEBUG_HEADER_SIZE);

            /* realloc() can modify errno() even on success, but since someone
               writing perl code doesn't have any control over when perl calls
               realloc() we need to hide that.
            */
            RESTORE_ERRNO;
        }

    /* In particular, must do that fixup above before logging anything via
     *printf(), as it can reallocate memory, which can cause SEGVs.  */

        DEBUG_m(PerlIO_printf(Perl_debug_log, "0x%" UVxf ": (%05ld) rfree\n",was_where,(long)PL_an++));
        DEBUG_m(PerlIO_printf(Perl_debug_log, "0x%" UVxf ": (%05ld) realloc %ld bytes\n",PTR2UV(ptr),(long)PL_an++,(long)size));

        if (ptr == NULL) {
#ifdef USE_MDH
          out_of_memory:
#endif
            {
#ifndef ALWAYS_NEED_THX
                dTHX;
#endif
                if (PL_nomemok)
                    ptr = NULL;
                else
                    croak_no_mem();
            }
        }
    }
    return ptr;
}

/*
=for apidoc safesysfree
Safe version of system's free()

=cut
*/

Free_t
Perl_safesysfree(Malloc_t where)
{
#ifdef ALWAYS_NEED_THX
    dTHX;
#endif
    DEBUG_m( PerlIO_printf(Perl_debug_log, "0x%" UVxf ": (%05ld) free\n",PTR2UV(where),(long)PL_an++));
    if (where) {
#ifdef USE_MDH
        Malloc_t where_intrn = (Malloc_t)((char*)where-PERL_MEMORY_DEBUG_HEADER_SIZE);
        {
            struct perl_memory_debug_header *const header
                = (struct perl_memory_debug_header *)where_intrn;

# ifdef MDH_HAS_SIZE
            const MEM_SIZE size = header->size;
# endif
# ifdef PERL_TRACK_MEMPOOL
            if (header->interpreter != aTHX) {
                Perl_croak_nocontext("panic: free from wrong pool, %p!=%p",
                                     header->interpreter, aTHX);
            }
            if (!header->prev) {
                Perl_croak_nocontext("panic: duplicate free");
            }
            if (!(header->next))
                Perl_croak_nocontext("panic: bad free, header->next==NULL");
            if (header->next->prev != header || header->prev->next != header) {
                Perl_croak_nocontext("panic: bad free, ->next->prev=%p, "
                                     "header=%p, ->prev->next=%p",
                                     header->next->prev, header,
                                     header->prev->next);
            }
            /* Unlink us from the chain.  */
            maybe_protect_rw(header->next);
            header->next->prev = header->prev;
            maybe_protect_ro(header->next);
            maybe_protect_rw(header->prev);
            header->prev->next = header->next;
            maybe_protect_ro(header->prev);
            maybe_protect_rw(header);
#  ifdef PERL_POISON
            PoisonNew(where_intrn, size, char);
#  endif
            /* Trigger the duplicate free warning.  */
            header->next = NULL;
# endif
# ifdef PERL_DEBUG_READONLY_COW
            if (munmap(where_intrn, size)) {
                perror("munmap failed");
                abort();
            }	
# endif
        }
#else
        Malloc_t where_intrn = where;
#endif /* USE_MDH */
#ifndef PERL_DEBUG_READONLY_COW
        PerlMem_free(where_intrn);
#endif
    }
}

/*
=for apidoc safesyscalloc
Safe version of system's calloc()

=cut
*/

Malloc_t
Perl_safesyscalloc(MEM_SIZE count, MEM_SIZE size)
{
#ifdef ALWAYS_NEED_THX
    dTHX;
#endif
    Malloc_t ptr;
#if defined(USE_MDH) || defined(DEBUGGING)
    MEM_SIZE total_size = 0;
#endif

    /* Even though calloc() for zero bytes is strange, be robust. */
    if (size && (count <= MEM_SIZE_MAX / size)) {
#if defined(USE_MDH) || defined(DEBUGGING)
        total_size = size * count;
#endif
    }
    else
        croak_memory_wrap();
#ifdef USE_MDH
    if (PERL_MEMORY_DEBUG_HEADER_SIZE <= MEM_SIZE_MAX - (MEM_SIZE)total_size)
        total_size += PERL_MEMORY_DEBUG_HEADER_SIZE;
    else
        croak_memory_wrap();
#endif
#ifdef DEBUGGING
    if ((SSize_t)size < 0 || (SSize_t)count < 0)
        Perl_croak_nocontext("panic: calloc, size=%" UVuf ", count=%" UVuf,
                             (UV)size, (UV)count);
#endif
#ifdef PERL_DEBUG_READONLY_COW
    if ((ptr = mmap(0, total_size ? total_size : 1, PROT_READ|PROT_WRITE,
                    MAP_ANON|MAP_PRIVATE, -1, 0)) == MAP_FAILED) {
        perror("mmap failed");
        abort();
    }
#elif defined(PERL_TRACK_MEMPOOL)
    /* Have to use malloc() because we've added some space for our tracking
       header.  */
    /* malloc(0) is non-portable. */
    ptr = (Malloc_t)PerlMem_malloc(total_size ? total_size : 1);
#else
    /* Use calloc() because it might save a memset() if the memory is fresh
       and clean from the OS.  */
    if (count && size)
        ptr = (Malloc_t)PerlMem_calloc(count, size);
    else /* calloc(0) is non-portable. */
        ptr = (Malloc_t)PerlMem_calloc(count ? count : 1, size ? size : 1);
#endif
    PERL_ALLOC_CHECK(ptr);
    DEBUG_m(PerlIO_printf(Perl_debug_log, "0x%" UVxf ": (%05ld) calloc %zu x %zu = %zu bytes\n",PTR2UV(ptr),(long)PL_an++, count, size, total_size));
    if (ptr != NULL) {
#ifdef USE_MDH
        {
            struct perl_memory_debug_header *const header
                = (struct perl_memory_debug_header *)ptr;

#  ifndef PERL_DEBUG_READONLY_COW
            memset((void*)ptr, 0, total_size);
#  endif
#  ifdef PERL_TRACK_MEMPOOL
            header->interpreter = aTHX;
            /* Link us into the list.  */
            header->prev = &PL_memory_debug_header;
            header->next = PL_memory_debug_header.next;
            PL_memory_debug_header.next = header;
            maybe_protect_rw(header->next);
            header->next->prev = header;
            maybe_protect_ro(header->next);
#    ifdef PERL_DEBUG_READONLY_COW
            header->readonly = 0;
#    endif
#  endif
#  ifdef MDH_HAS_SIZE
            header->size = total_size;
#  endif
            ptr = (Malloc_t)((char*)ptr+PERL_MEMORY_DEBUG_HEADER_SIZE);
        }
#endif
        return ptr;
    }
    else {
#ifndef ALWAYS_NEED_THX
        dTHX;
#endif
        if (PL_nomemok)
            return NULL;
        croak_no_mem();
    }
}

/* These must be defined when not using Perl's malloc for binary
 * compatibility */

#ifndef MYMALLOC

Malloc_t Perl_malloc (MEM_SIZE nbytes)
{
#ifdef PERL_IMPLICIT_SYS
    dTHX;
#endif
    return (Malloc_t)PerlMem_malloc(nbytes);
}

Malloc_t Perl_calloc (MEM_SIZE elements, MEM_SIZE size)
{
#ifdef PERL_IMPLICIT_SYS
    dTHX;
#endif
    return (Malloc_t)PerlMem_calloc(elements, size);
}

Malloc_t Perl_realloc (Malloc_t where, MEM_SIZE nbytes)
{
#ifdef PERL_IMPLICIT_SYS
    dTHX;
#endif
    return (Malloc_t)PerlMem_realloc(where, nbytes);
}

Free_t   Perl_mfree (Malloc_t where)
{
#ifdef PERL_IMPLICIT_SYS
    dTHX;
#endif
    PerlMem_free(where);
}

#endif

/* This is the value stored in *retlen in the two delimcpy routines below when
 * there wasn't enough room in the destination to store everything it was asked
 * to.  The value is deliberately very large so that hopefully if code uses it
 * unquestioningly to access memory, it will likely segfault.  And it is small
 * enough that if the caller does some arithmetic on it before accessing, it
 * won't overflow into a small legal number. */
#define DELIMCPY_OUT_OF_BOUNDS_RET  I32_MAX

/*
=for apidoc_section $string
=for apidoc delimcpy_no_escape

Copy a source buffer to a destination buffer, stopping at (but not including)
the first occurrence in the source of the delimiter byte, C<delim>.  The source
is the bytes between S<C<from> and C<from_end> - 1>.  Similarly, the dest is
C<to> up to C<to_end>.

The number of bytes copied is written to C<*retlen>.

Returns the position of C<delim> in the C<from> buffer, but if there is no
such occurrence before C<from_end>, then C<from_end> is returned, and the entire
buffer S<C<from> .. C<from_end> - 1> is copied.

If there is room in the destination available after the copy, an extra
terminating safety C<NUL> byte is appended (not included in the returned
length).

The error case is if the destination buffer is not large enough to accommodate
everything that should be copied.  In this situation, a value larger than
S<C<to_end> - C<to>> is written to C<*retlen>, and as much of the source as
fits will be written to the destination.  Not having room for the safety C<NUL>
is not considered an error.

=cut
*/
char *
Perl_delimcpy_no_escape(char *to, const char *to_end,
                        const char *from, const char *from_end,
                        const int delim, I32 *retlen)
{
    const char * delim_pos;
    Ptrdiff_t from_len = from_end - from;
    Ptrdiff_t to_len = to_end - to;
    SSize_t copy_len;

    PERL_ARGS_ASSERT_DELIMCPY_NO_ESCAPE;

    assert(from_len >= 0);
    assert(to_len >= 0);

    /* Look for the first delimiter in the source */
    delim_pos = (const char *) memchr(from, delim, from_len);

    /* Copy up to where the delimiter was found, or the entire buffer if not
     * found */
    copy_len = (delim_pos) ? delim_pos - from : from_len;

    /* If not enough room, copy as much as can fit, and set error return */
    if (copy_len > to_len) {
        Copy(from, to, to_len, char);
        *retlen = DELIMCPY_OUT_OF_BOUNDS_RET;
    }
    else {
        Copy(from, to, copy_len, char);

        /* If there is extra space available, add a trailing NUL */
        if (copy_len < to_len) {
            to[copy_len] = '\0';
        }

        *retlen = copy_len;
    }

    return (char *) from + copy_len;
}

/*
=for apidoc delimcpy

Copy a source buffer to a destination buffer, stopping at (but not including)
the first occurrence in the source of an unescaped (defined below) delimiter
byte, C<delim>.  The source is the bytes between S<C<from> and C<from_end> -
1>.  Similarly, the dest is C<to> up to C<to_end>.

The number of bytes copied is written to C<*retlen>.

Returns the position of the first uncopied C<delim> in the C<from> buffer, but
if there is no such occurrence before C<from_end>, then C<from_end> is returned,
and the entire buffer S<C<from> .. C<from_end> - 1> is copied.

If there is room in the destination available after the copy, an extra
terminating safety C<NUL> byte is appended (not included in the returned
length).

The error case is if the destination buffer is not large enough to accommodate
everything that should be copied.  In this situation, a value larger than
S<C<to_end> - C<to>> is written to C<*retlen>, and as much of the source as
fits will be written to the destination.  Not having room for the safety C<NUL>
is not considered an error.

In the following examples, let C<x> be the delimiter, and C<0> represent a C<NUL>
byte (B<NOT> the digit C<0>).  Then we would have

  Source     Destination
 abcxdef        abc0

provided the destination buffer is at least 4 bytes long.

An escaped delimiter is one which is immediately preceded by a single
backslash.  Escaped delimiters are copied, and the copy continues past the
delimiter; the backslash is not copied:

  Source       Destination
 abc\xdef       abcxdef0

(provided the destination buffer is at least 8 bytes long).

It's actually somewhat more complicated than that. A sequence of any odd number
of backslashes escapes the following delimiter, and the copy continues with
exactly one of the backslashes stripped.

     Source         Destination
     abc\xdef          abcxdef0
   abc\\\xdef        abc\\xdef0
 abc\\\\\xdef      abc\\\\xdef0

(as always, if the destination is large enough)

An even number of preceding backslashes does not escape the delimiter, so that
the copy stops just before it, and includes all the backslashes (no stripping;
zero is considered even):

      Source         Destination
      abcxdef          abc0
    abc\\xdef          abc\\0
  abc\\\\xdef          abc\\\\0

=cut
*/

char *
Perl_delimcpy(char *to, const char *to_end,
              const char *from, const char *from_end,
              const int delim, I32 *retlen)
{
    const char * const orig_to = to;
    Ptrdiff_t copy_len = 0;
    bool stopped_early = FALSE;     /* Ran out of room to copy to */

    PERL_ARGS_ASSERT_DELIMCPY;
    assert(from_end >= from);
    assert(to_end >= to);

    /* Don't use the loop for the trivial case of the first character being the
     * delimiter; otherwise would have to worry inside the loop about backing
     * up before the start of 'from' */
    if (LIKELY(from_end > from && *from != delim)) {
        while ((copy_len = from_end - from) > 0) {
            const char * backslash_pos;
            const char * delim_pos;

            /* Look for the next delimiter in the remaining portion of the
             * source. A loop invariant is that we already know that the copy
             * should include *from; this comes from the conditional before the
             * loop, and how we set things up at the end of each iteration */
            delim_pos = (const char *) memchr(from + 1, delim, copy_len - 1);

            /* If didn't find it, done looking; set up so copies all of the
             * source */
            if (! delim_pos) {
                copy_len = from_end - from;
                break;
            }

            /* Look for a backslash immediately before the delimiter */
            backslash_pos = delim_pos - 1;

            /* If the delimiter is not escaped, this ends the copy */
            if (*backslash_pos != '\\') {
                copy_len = delim_pos - from;
                break;
            }

            /* Here there is a backslash just before the delimiter, but it
             * could be the final backslash in a sequence of them.  Backup to
             * find the first one in it. */
            do {
                backslash_pos--;
            }
            while (backslash_pos >= from && *backslash_pos == '\\');

            /* If the number of backslashes is even, they just escape one
             * another, leaving the delimiter unescaped, and stopping the copy.
             * */
            if (! ((delim_pos - (backslash_pos + 1)) & 1)) {
                copy_len = delim_pos - from;  /* even, copy up to delimiter */
                break;
            }

            /* Here is odd, so the delimiter is escaped.  We will try to copy
             * all but the final backslash in the sequence */
            copy_len = delim_pos - 1 - from;

            /* Do the copy, but not beyond the end of the destination */
            if (copy_len >= to_end - to) {
                Copy(from, to, to_end - to, char);
                stopped_early = TRUE;
                to = (char *) to_end;
            }
            else {
                Copy(from, to, copy_len, char);
                to += copy_len;
            }

            /* Set up so next iteration will include the delimiter */
            from = delim_pos;
        }
    }

    /* Here, have found the final segment to copy.  Copy that, but not beyond
     * the size of the destination.  If not enough room, copy as much as can
     * fit, and set error return */
    if (stopped_early || copy_len > to_end - to) {
        Copy(from, to, to_end - to, char);
        *retlen = DELIMCPY_OUT_OF_BOUNDS_RET;
    }
    else {
        Copy(from, to, copy_len, char);

        to += copy_len;

        /* If there is extra space available, add a trailing NUL */
        if (to < to_end) {
            *to = '\0';
        }

        *retlen = to - orig_to;
    }

    return (char *) from + copy_len;
}

/*
=for apidoc ninstr

Find the first (leftmost) occurrence of a sequence of bytes within another
sequence.  This is the Perl version of C<strstr()>, extended to handle
arbitrary sequences, potentially containing embedded C<NUL> characters (C<NUL>
is what the initial C<n> in the function name stands for; some systems have an
equivalent, C<memmem()>, but with a somewhat different API).

Another way of thinking about this function is finding a needle in a haystack.
C<big> points to the first byte in the haystack.  C<big_end> points to one byte
beyond the final byte in the haystack.  C<little> points to the first byte in
the needle.  C<little_end> points to one byte beyond the final byte in the
needle.  All the parameters must be non-C<NULL>.

The function returns C<NULL> if there is no occurrence of C<little> within
C<big>.  If C<little> is the empty string, C<big> is returned.

Because this function operates at the byte level, and because of the inherent
characteristics of UTF-8 (or UTF-EBCDIC), it will work properly if both the
needle and the haystack are strings with the same UTF-8ness, but not if the
UTF-8ness differs.

=cut

*/

char *
Perl_ninstr(const char *big, const char *bigend, const char *little, const char *lend)
{
    PERL_ARGS_ASSERT_NINSTR;

#ifdef HAS_MEMMEM
    return ninstr(big, bigend, little, lend);
#else

    if (little >= lend) {
        return (char*) big;
    }
    else {
        const U8 first = *little;
        Size_t lsize;

        /* No match can start closer to the end of the haystack than the length
         * of the needle. */
        bigend -= lend - little;
        little++;       /* Look for 'first', then the remainder is in here */
        lsize = lend - little;

        while (big <= bigend) {
            big = (char *) memchr((U8 *) big, first, bigend - big + 1);
            if (big == NULL || big > bigend) {
                return NULL;
            }

            if (memEQ(big + 1, little, lsize)) {
                return (char*) big;
            }
            big++;
        }
    }

    return NULL;

#endif

}

/*
=for apidoc rninstr

Like C<L</ninstr>>, but instead finds the final (rightmost) occurrence of a
sequence of bytes within another sequence, returning C<NULL> if there is no
such occurrence.

=cut

*/

char *
Perl_rninstr(const char *big, const char *bigend, const char *little, const char *lend)
{
    const Ptrdiff_t little_len = lend - little;
    const Ptrdiff_t big_len = bigend - big;

    PERL_ARGS_ASSERT_RNINSTR;

    /* A non-existent needle trivially matches the rightmost possible position
     * in the haystack */
    if (UNLIKELY(little_len <= 0)) {
        return (char*)bigend;
    }

    /* If the needle is larger than the haystack, the needle can't possibly fit
     * inside the haystack. */
    if (UNLIKELY(little_len > big_len)) {
        return NULL;
    }

    /* Special case length 1 needles.  It's trivial if we have memrchr();
     * and otherwise we just do a per-byte search backwards.
     *
     * XXX When we don't have memrchr, we could use something like
     * S_find_next_masked( or S_find_span_end() to do per-word searches */
    if (little_len == 1) {
        const char final = *little;

#ifdef HAS_MEMRCHR

        return (char *) memrchr(big, final, big_len);
#else
        const char * cur = bigend - 1;

        do {
            if (*cur == final) {
                return (char *) cur;
            }
        } while (--cur >= big);

        return NULL;
#endif

    }
    else {  /* Below, the needle is longer than a single byte */

        /* We search backwards in the haystack for the final character of the
         * needle.  Each time one is found, we see if the characters just
         * before it in the haystack match the rest of the needle. */
        const char final = *(lend - 1);

        /* What matches consists of 'little_len'-1 characters, then the final
         * one */
        const Size_t prefix_len = little_len - 1;

        /* If the final character in the needle is any closer than this to the
         * left edge, there wouldn't be enough room for all of it to fit in the
         * haystack */
        const char * const left_fence = big + prefix_len;

        /* Start at the right edge */
        char * cur = (char *) bigend;

        /* memrchr() makes the search easy (and fast); otherwise, look
         * backwards byte-by-byte. */
        do {

#ifdef HAS_MEMRCHR

            cur = (char *) memrchr(left_fence, final, cur - left_fence);
            if (cur == NULL) {
                return NULL;
            }
#else
            do {
                cur--;
                if (cur < left_fence) {
                    return NULL;
                }
            }
            while (*cur != final);
#endif

            /* Here, we know that *cur is 'final'; see if the preceding bytes
             * of the needle also match the corresponding haystack bytes */
            if memEQ(cur - prefix_len, little, prefix_len) {
                return cur - prefix_len;
            }
        } while (cur > left_fence);

        return NULL;
    }
}

/* As a space optimization, we do not compile tables for strings of length
   0 and 1, and for strings of length 2 unless FBMcf_TAIL.  These are
   special-cased in fbm_instr().

   If FBMcf_TAIL, the table is created as if the string has a trailing \n. */

/*

=for apidoc fbm_compile

Analyzes the string in order to make fast searches on it using C<fbm_instr()>
-- the Boyer-Moore algorithm.

=cut
*/

void
Perl_fbm_compile(pTHX_ SV *sv, U32 flags)
{
    const U8 *s;
    STRLEN i;
    STRLEN len;
    MAGIC *mg;

    PERL_ARGS_ASSERT_FBM_COMPILE;

    if (isGV_with_GP(sv) || SvROK(sv))
        return;

    if (SvVALID(sv))
        return;

    if (flags & FBMcf_TAIL) {
        MAGIC * const mg = SvUTF8(sv) && SvMAGICAL(sv) ? mg_find(sv, PERL_MAGIC_utf8) : NULL;
        sv_catpvs(sv, "\n");		/* Taken into account in fbm_instr() */
        if (mg && mg->mg_len >= 0)
            mg->mg_len++;
    }
    if (!SvPOK(sv) || SvNIOKp(sv))
        s = (U8*)SvPV_force_mutable(sv, len);
    else s = (U8 *)SvPV_mutable(sv, len);
    if (len == 0)		/* TAIL might be on a zero-length string. */
        return;
    SvUPGRADE(sv, SVt_PVMG);
    SvIOK_off(sv);
    SvNOK_off(sv);

    /* add PERL_MAGIC_bm magic holding the FBM lookup table */

    assert(!mg_find(sv, PERL_MAGIC_bm));
    mg = sv_magicext(sv, NULL, PERL_MAGIC_bm, &PL_vtbl_bm, NULL, 0);
    assert(mg);

    if (len > 2) {
        /* Shorter strings are special-cased in Perl_fbm_instr(), and don't use
           the BM table.  */
        const U8 mlen = (len>255) ? 255 : (U8)len;
        const unsigned char *const sb = s + len - mlen; /* first char (maybe) */
        U8 *table;

        Newx(table, 256, U8);
        memset((void*)table, mlen, 256);
        mg->mg_ptr = (char *)table;
        mg->mg_len = 256;

        s += len - 1; /* last char */
        i = 0;
        while (s >= sb) {
            if (table[*s] == mlen)
                table[*s] = (U8)i;
            s--, i++;
        }
    }

    BmUSEFUL(sv) = 100;			/* Initial value */
    ((XPVNV*)SvANY(sv))->xnv_u.xnv_bm_tail = cBOOL(flags & FBMcf_TAIL);
}


/*
=for apidoc fbm_instr

Returns the location of the SV in the string delimited by C<big> and
C<bigend> (C<bigend>) is the char following the last char).
It returns C<NULL> if the string can't be found.  The C<sv>
does not have to be C<fbm_compiled>, but the search will not be as fast
then.

=cut

If SvTAIL(littlestr) is true, a fake "\n" was appended to the string
during FBM compilation due to FBMcf_TAIL in flags. It indicates that
the littlestr must be anchored to the end of bigstr (or to any \n if
FBMrf_MULTILINE).

E.g. The regex compiler would compile /abc/ to a littlestr of "abc",
while /abc$/ compiles to "abc\n" with SvTAIL() true.

A littlestr of "abc", !SvTAIL matches as /abc/;
a littlestr of "ab\n", SvTAIL matches as:
   without FBMrf_MULTILINE: /ab\n?\z/
   with    FBMrf_MULTILINE: /ab\n/ || /ab\z/;

(According to Ilya from 1999; I don't know if this is still true, DAPM 2015):
  "If SvTAIL is actually due to \Z or \z, this gives false positives
  if multiline".
*/


char *
Perl_fbm_instr(pTHX_ unsigned char *big, unsigned char *bigend, SV *littlestr, U32 flags)
{
    unsigned char *s;
    STRLEN l;
    const unsigned char *little = (const unsigned char *)SvPV_const(littlestr,l);
    STRLEN littlelen = l;
    const I32 multiline = flags & FBMrf_MULTILINE;
    bool valid = SvVALID(littlestr);
    bool tail = valid ? cBOOL(SvTAIL(littlestr)) : FALSE;

    PERL_ARGS_ASSERT_FBM_INSTR;

    assert(bigend >= big);

    if ((STRLEN)(bigend - big) < littlelen) {
        if (     tail
             && ((STRLEN)(bigend - big) == littlelen - 1)
             && (littlelen == 1
                 || (*big == *little &&
                     memEQ((char *)big, (char *)little, littlelen - 1))))
            return (char*)big;
        return NULL;
    }

    switch (littlelen) { /* Special cases for 0, 1 and 2  */
    case 0:
        return (char*)big;		/* Cannot be SvTAIL! */

    case 1:
            if (tail && !multiline) /* Anchor only! */
                /* [-1] is safe because we know that bigend != big.  */
                return (char *) (bigend - (bigend[-1] == '\n'));

            s = (unsigned char *)memchr((void*)big, *little, bigend-big);
            if (s)
                return (char *)s;
            if (tail)
                return (char *) bigend;
            return NULL;

    case 2:
        if (tail && !multiline) {
            /* a littlestr with SvTAIL must be of the form "X\n" (where X
             * is a single char). It is anchored, and can only match
             * "....X\n"  or  "....X" */
            if (bigend[-2] == *little && bigend[-1] == '\n')
                return (char*)bigend - 2;
            if (bigend[-1] == *little)
                return (char*)bigend - 1;
            return NULL;
        }

        {
            /* memchr() is likely to be very fast, possibly using whatever
             * hardware support is available, such as checking a whole
             * cache line in one instruction.
             * So for a 2 char pattern, calling memchr() is likely to be
             * faster than running FBM, or rolling our own. The previous
             * version of this code was roll-your-own which typically
             * only needed to read every 2nd char, which was good back in
             * the day, but no longer.
             */
            unsigned char c1 = little[0];
            unsigned char c2 = little[1];

            /* *** for all this case, bigend points to the last char,
             * not the trailing \0: this makes the conditions slightly
             * simpler */
            bigend--;
            s = big;
            if (c1 != c2) {
                while (s < bigend) {
                    /* do a quick test for c1 before calling memchr();
                     * this avoids the expensive fn call overhead when
                     * there are lots of c1's */
                    if (LIKELY(*s != c1)) {
                        s++;
                        s = (unsigned char *)memchr((void*)s, c1, bigend - s);
                        if (!s)
                            break;
                    }
                    if (s[1] == c2)
                        return (char*)s;

                    /* failed; try searching for c2 this time; that way
                     * we don't go pathologically slow when the string
                     * consists mostly of c1's or vice versa.
                     */
                    s += 2;
                    if (s > bigend)
                        break;
                    s = (unsigned char *)memchr((void*)s, c2, bigend - s + 1);
                    if (!s)
                        break;
                    if (s[-1] == c1)
                        return (char*)s - 1;
                }
            }
            else {
                /* c1, c2 the same */
                while (s < bigend) {
                    if (s[0] == c1) {
                      got_1char:
                        if (s[1] == c1)
                            return (char*)s;
                        s += 2;
                    }
                    else {
                        s++;
                        s = (unsigned char *)memchr((void*)s, c1, bigend - s);
                        if (!s || s >= bigend)
                            break;
                        goto got_1char;
                    }
                }
            }

            /* failed to find 2 chars; try anchored match at end without
             * the \n */
            if (tail && bigend[0] == little[0])
                return (char *)bigend;
            return NULL;
        }

    default:
        break; /* Only lengths 0 1 and 2 have special-case code.  */
    }

    if (tail && !multiline) {	/* tail anchored? */
        s = bigend - littlelen;
        if (s >= big && bigend[-1] == '\n' && *s == *little
            /* Automatically of length > 2 */
            && memEQ((char*)s + 1, (char*)little + 1, littlelen - 2))
        {
            return (char*)s;		/* how sweet it is */
        }
        if (s[1] == *little
            && memEQ((char*)s + 2, (char*)little + 1, littlelen - 2))
        {
            return (char*)s + 1;	/* how sweet it is */
        }
        return NULL;
    }

    if (!valid) {
        /* not compiled; use Perl_ninstr() instead */
        char * const b = ninstr((char*)big,(char*)bigend,
                         (char*)little, (char*)little + littlelen);

        assert(!tail); /* valid => FBM; tail only set on SvVALID SVs */
        return b;
    }

    /* Do actual FBM.  */
    if (littlelen > (STRLEN)(bigend - big))
        return NULL;

    {
        const MAGIC *const mg = mg_find(littlestr, PERL_MAGIC_bm);
        const unsigned char *oldlittle;

        assert(mg);

        --littlelen;			/* Last char found by table lookup */

        s = big + littlelen;
        little += littlelen;		/* last char */
        oldlittle = little;
        if (s < bigend) {
            const unsigned char * const table = (const unsigned char *) mg->mg_ptr;
            const unsigned char lastc = *little;
            I32 tmp;

          top2:
            if ((tmp = table[*s])) {
                /* *s != lastc; earliest position it could match now is
                 * tmp slots further on */
                if ((s += tmp) >= bigend)
                    goto check_end;
                if (LIKELY(*s != lastc)) {
                    s++;
                    s = (unsigned char *)memchr((void*)s, lastc, bigend - s);
                    if (!s) {
                        s = bigend;
                        goto check_end;
                    }
                    goto top2;
                }
            }


            /* hand-rolled strncmp(): less expensive than calling the
             * real function (maybe???) */
            {
                unsigned char * const olds = s;

                tmp = littlelen;

                while (tmp--) {
                    if (*--s == *--little)
                        continue;
                    s = olds + 1;	/* here we pay the price for failure */
                    little = oldlittle;
                    if (s < bigend)	/* fake up continue to outer loop */
                        goto top2;
                    goto check_end;
                }
                return (char *)s;
            }
        }
      check_end:
        if ( s == bigend
             && tail
             && memEQ((char *)(bigend - littlelen),
                      (char *)(oldlittle - littlelen), littlelen) )
            return (char*)bigend - littlelen;
        return NULL;
    }
}

const char *
Perl_cntrl_to_mnemonic(const U8 c)
{
    /* Returns the mnemonic string that represents character 'c', if one
     * exists; NULL otherwise.  The only ones that exist for the purposes of
     * this routine are a few control characters */

    switch (c) {
        case '\a':       return "\\a";
        case '\b':       return "\\b";
        case ESC_NATIVE: return "\\e";
        case '\f':       return "\\f";
        case '\n':       return "\\n";
        case '\r':       return "\\r";
        case '\t':       return "\\t";
    }

    return NULL;
}

/*
=for apidoc savesharedpv

A version of C<savepv()> which allocates the duplicate string in memory
which is shared between threads.

=cut
*/
char *
Perl_savesharedpv(pTHX_ const char *pv)
{
    char *newaddr;
    STRLEN pvlen;

    PERL_UNUSED_CONTEXT;

    if (!pv)
        return NULL;

    pvlen = strlen(pv)+1;
    newaddr = (char*)PerlMemShared_malloc(pvlen);
    if (!newaddr) {
        croak_no_mem();
    }
    return (char*)memcpy(newaddr, pv, pvlen);
}

/*
=for apidoc savesharedpvn

A version of C<savepvn()> which allocates the duplicate string in memory
which is shared between threads.  (With the specific difference that a C<NULL>
pointer is not acceptable)

=cut
*/
char *
Perl_savesharedpvn(pTHX_ const char *const pv, const STRLEN len)
{
    char *const newaddr = (char*)PerlMemShared_malloc(len + 1);

    PERL_UNUSED_CONTEXT;
    /* PERL_ARGS_ASSERT_SAVESHAREDPVN; */

    if (!newaddr) {
        croak_no_mem();
    }
    newaddr[len] = '\0';
    return (char*)memcpy(newaddr, pv, len);
}

/* the SV for Perl_form() and mess() is not kept in an arena */

STATIC SV *
S_mess_alloc(pTHX)
{
    SV *sv;
    XPVMG *any;

    if (PL_phase != PERL_PHASE_DESTRUCT)
        return newSVpvs_flags("", SVs_TEMP);

    if (PL_mess_sv)
        return PL_mess_sv;

    /* Create as PVMG now, to avoid any upgrading later */
    Newx(sv, 1, SV);
    Newxz(any, 1, XPVMG);
    SvFLAGS(sv) = SVt_PVMG;
    SvANY(sv) = (void*)any;
    SvPV_set(sv, NULL);
    SvREFCNT(sv) = 1 << 30; /* practically infinite */
    PL_mess_sv = sv;
    return sv;
}

#if defined(MULTIPLICITY)
char *
Perl_form_nocontext(const char* pat, ...)
{
    dTHX;
    char *retval;
    va_list args;
    PERL_ARGS_ASSERT_FORM_NOCONTEXT;
    va_start(args, pat);
    retval = vform(pat, &args);
    va_end(args);
    return retval;
}
#endif /* MULTIPLICITY */

/*
=for apidoc_section $display
=for apidoc form
=for apidoc_item form_nocontext

These take a sprintf-style format pattern and conventional
(non-SV) arguments and return the formatted string.

    (char *) Perl_form(pTHX_ const char* pat, ...)

can be used any place a string (char *) is required:

    char * s = Perl_form("%d.%d",major,minor);

They use a single (per-thread) private buffer so if you want to format several
strings you must explicitly copy the earlier strings away (and free the copies
when you are done).

The two forms differ only in that C<form_nocontext> does not take a thread
context (C<aTHX>) parameter, so is used in situations where the caller doesn't
already have the thread context.

=for apidoc vform
Like C<L</form>> but but the arguments are an encapsulated argument list.

=cut
*/

char *
Perl_form(pTHX_ const char* pat, ...)
{
    char *retval;
    va_list args;
    PERL_ARGS_ASSERT_FORM;
    va_start(args, pat);
    retval = vform(pat, &args);
    va_end(args);
    return retval;
}

char *
Perl_vform(pTHX_ const char *pat, va_list *args)
{
    SV * const sv = mess_alloc();
    PERL_ARGS_ASSERT_VFORM;
    sv_vsetpvfn(sv, pat, strlen(pat), args, NULL, 0, NULL);
    return SvPVX(sv);
}

/*
=for apidoc mess
=for apidoc_item mess_nocontext

These take a sprintf-style format pattern and argument list, which are used to
generate a string message.  If the message does not end with a newline, then it
will be extended with some indication of the current location in the code, as
described for C<L</mess_sv>>.

Normally, the resulting message is returned in a new mortal SV.
But during global destruction a single SV may be shared between uses of
this function.

The two forms differ only in that C<mess_nocontext> does not take a thread
context (C<aTHX>) parameter, so is used in situations where the caller doesn't
already have the thread context.

=cut
*/

#if defined(MULTIPLICITY)
SV *
Perl_mess_nocontext(const char *pat, ...)
{
    dTHX;
    SV *retval;
    va_list args;
    PERL_ARGS_ASSERT_MESS_NOCONTEXT;
    va_start(args, pat);
    retval = vmess(pat, &args);
    va_end(args);
    return retval;
}
#endif /* MULTIPLICITY */

SV *
Perl_mess(pTHX_ const char *pat, ...)
{
    SV *retval;
    va_list args;
    PERL_ARGS_ASSERT_MESS;
    va_start(args, pat);
    retval = vmess(pat, &args);
    va_end(args);
    return retval;
}

const COP*
Perl_closest_cop(pTHX_ const COP *cop, const OP *o, const OP *curop,
                       bool opnext)
{
    /* Look for curop starting from o.  cop is the last COP we've seen. */
    /* opnext means that curop is actually the ->op_next of the op we are
       seeking. */

    PERL_ARGS_ASSERT_CLOSEST_COP;

    if (!o || !curop || (
        opnext ? o->op_next == curop && o->op_type != OP_SCOPE : o == curop
    ))
        return cop;

    if (o->op_flags & OPf_KIDS) {
        const OP *kid;
        for (kid = cUNOPo->op_first; kid; kid = OpSIBLING(kid)) {
            const COP *new_cop;

            /* If the OP_NEXTSTATE has been optimised away we can still use it
             * the get the file and line number. */

            if (kid->op_type == OP_NULL && kid->op_targ == OP_NEXTSTATE)
                cop = (const COP *)kid;

            /* Keep searching, and return when we've found something. */

            new_cop = closest_cop(cop, kid, curop, opnext);
            if (new_cop)
                return new_cop;
        }
    }

    /* Nothing found. */

    return NULL;
}

/*
=for apidoc mess_sv

Expands a message, intended for the user, to include an indication of
the current location in the code, if the message does not already appear
to be complete.

C<basemsg> is the initial message or object.  If it is a reference, it
will be used as-is and will be the result of this function.  Otherwise it
is used as a string, and if it already ends with a newline, it is taken
to be complete, and the result of this function will be the same string.
If the message does not end with a newline, then a segment such as C<at
foo.pl line 37> will be appended, and possibly other clauses indicating
the current state of execution.  The resulting message will end with a
dot and a newline.

Normally, the resulting message is returned in a new mortal SV.
During global destruction a single SV may be shared between uses of this
function.  If C<consume> is true, then the function is permitted (but not
required) to modify and return C<basemsg> instead of allocating a new SV.

=cut
*/

SV *
Perl_mess_sv(pTHX_ SV *basemsg, bool consume)
{
    SV *sv;

#if defined(USE_C_BACKTRACE) && defined(USE_C_BACKTRACE_ON_ERROR)
    {
        char *ws;
        UV wi;
        /* The PERL_C_BACKTRACE_ON_WARN must be an integer of one or more. */
        if ((ws = PerlEnv_getenv("PERL_C_BACKTRACE_ON_ERROR"))
            && grok_atoUV(ws, &wi, NULL)
            && wi <= PERL_INT_MAX
        ) {
            Perl_dump_c_backtrace(aTHX_ Perl_debug_log, (int)wi, 1);
        }
    }
#endif

    PERL_ARGS_ASSERT_MESS_SV;

    if (SvROK(basemsg)) {
        if (consume) {
            sv = basemsg;
        }
        else {
            sv = mess_alloc();
            sv_setsv(sv, basemsg);
        }
        return sv;
    }

    if (SvPOK(basemsg) && consume) {
        sv = basemsg;
    }
    else {
        sv = mess_alloc();
        sv_copypv(sv, basemsg);
    }

    if (!SvCUR(sv) || *(SvEND(sv) - 1) != '\n') {
        /*
         * Try and find the file and line for PL_op.  This will usually be
         * PL_curcop, but it might be a cop that has been optimised away.  We
         * can try to find such a cop by searching through the optree starting
         * from the sibling of PL_curcop.
         */

        if (PL_curcop) {
            const COP *cop =
                closest_cop(PL_curcop, OpSIBLING(PL_curcop), PL_op, FALSE);
            if (!cop)
                cop = PL_curcop;

            if (CopLINE(cop))
                Perl_sv_catpvf(aTHX_ sv, " at %s line %" LINE_Tf,
                                OutCopFILE(cop), CopLINE(cop));
        }

        /* Seems that GvIO() can be untrustworthy during global destruction. */
        if (GvIO(PL_last_in_gv) && (SvTYPE(GvIOp(PL_last_in_gv)) == SVt_PVIO)
                && IoLINES(GvIOp(PL_last_in_gv)))
        {
            STRLEN l;
            const bool line_mode = (RsSIMPLE(PL_rs) &&
                                   *SvPV_const(PL_rs,l) == '\n' && l == 1);
            Perl_sv_catpvf(aTHX_ sv, ", <%" SVf "> %s %" IVdf,
                           SVfARG(PL_last_in_gv == PL_argvgv
                                 ? &PL_sv_no
                                 : newSVhek_mortal(GvNAME_HEK(PL_last_in_gv))),
                           line_mode ? "line" : "chunk",
                           (IV)IoLINES(GvIOp(PL_last_in_gv)));
        }
        if (PL_phase == PERL_PHASE_DESTRUCT)
            sv_catpvs(sv, " during global destruction");
        sv_catpvs(sv, ".\n");
    }
    return sv;
}

/*
=for apidoc vmess

C<pat> and C<args> are a sprintf-style format pattern and encapsulated
argument list, respectively.  These are used to generate a string message.  If
the
message does not end with a newline, then it will be extended with
some indication of the current location in the code, as described for
L</mess_sv>.

Normally, the resulting message is returned in a new mortal SV.
During global destruction a single SV may be shared between uses of
this function.

=cut
*/

SV *
Perl_vmess(pTHX_ const char *pat, va_list *args)
{
    SV * const sv = mess_alloc();

    PERL_ARGS_ASSERT_VMESS;

    sv_vsetpvfn(sv, pat, strlen(pat), args, NULL, 0, NULL);
    return mess_sv(sv, 1);
}

void
Perl_write_to_stderr(pTHX_ SV* msv)
{
    IO *io;
    MAGIC *mg;

    PERL_ARGS_ASSERT_WRITE_TO_STDERR;

    if (PL_stderrgv && SvREFCNT(PL_stderrgv) 
        && (io = GvIO(PL_stderrgv))
        && (mg = SvTIED_mg((const SV *)io, PERL_MAGIC_tiedscalar))) 
        Perl_magic_methcall(aTHX_ MUTABLE_SV(io), mg, SV_CONST(PRINT),
                            G_SCALAR | G_DISCARD | G_WRITING_TO_STDERR, 1, msv);
    else {
        PerlIO * const serr = Perl_error_log;

        do_print(msv, serr);
        (void)PerlIO_flush(serr);
    }
}

/*
=for apidoc_section $warning
*/

/* Common code used in dieing and warning */

STATIC SV *
S_with_queued_errors(pTHX_ SV *ex)
{
    PERL_ARGS_ASSERT_WITH_QUEUED_ERRORS;
    if (PL_errors && SvCUR(PL_errors) && !SvROK(ex)) {
        sv_catsv(PL_errors, ex);
        ex = sv_mortalcopy(PL_errors);
        SvCUR_set(PL_errors, 0);
    }
    return ex;
}

bool
Perl_invoke_exception_hook(pTHX_ SV *ex, bool warn)
{
    HV *stash;
    GV *gv;
    CV *cv;
    SV **const hook = warn ? &PL_warnhook : &PL_diehook;
    /* sv_2cv might call Perl_croak() or Perl_warner() */
    SV * const oldhook = *hook;

    if (!oldhook || oldhook == PERL_WARNHOOK_FATAL)
        return FALSE;

    ENTER;
    SAVESPTR(*hook);
    *hook = NULL;
    cv = sv_2cv(oldhook, &stash, &gv, 0);
    LEAVE;
    if (cv && !CvDEPTH(cv) && (CvROOT(cv) || CvXSUB(cv))) {
        dSP;
        SV *exarg;

        ENTER;
        save_re_context();
        if (warn) {
            SAVESPTR(*hook);
            *hook = NULL;
        }
        exarg = newSVsv(ex);
        SvREADONLY_on(exarg);
        SAVEFREESV(exarg);

        PUSHSTACKi(warn ? PERLSI_WARNHOOK : PERLSI_DIEHOOK);
        PUSHMARK(SP);
        XPUSHs(exarg);
        PUTBACK;
        call_sv(MUTABLE_SV(cv), G_DISCARD);
        POPSTACK;
        LEAVE;
        return TRUE;
    }
    return FALSE;
}

/*
=for apidoc die_sv

This behaves the same as L</croak_sv>, except for the return type.
It should be used only where the C<OP *> return type is required.
The function never actually returns.

=cut
*/

/* silence __declspec(noreturn) warnings */
MSVC_DIAG_IGNORE(4646 4645)
OP *
Perl_die_sv(pTHX_ SV *baseex)
{
    PERL_ARGS_ASSERT_DIE_SV;
    croak_sv(baseex);
    /* NOTREACHED */
    NORETURN_FUNCTION_END;
}
MSVC_DIAG_RESTORE

/*
=for apidoc      die
=for apidoc_item die_nocontext

These behave the same as L</croak>, except for the return type.
They should be used only where the C<OP *> return type is required.
They never actually return.

The two forms differ only in that C<die_nocontext> does not take a thread
context (C<aTHX>) parameter, so is used in situations where the caller doesn't
already have the thread context.

=cut
*/

#if defined(MULTIPLICITY)

/* silence __declspec(noreturn) warnings */
MSVC_DIAG_IGNORE(4646 4645)
OP *
Perl_die_nocontext(const char* pat, ...)
{
    dTHX;
    va_list args;
    va_start(args, pat);
    vcroak(pat, &args);
    NOT_REACHED; /* NOTREACHED */
    va_end(args);
    NORETURN_FUNCTION_END;
}
MSVC_DIAG_RESTORE

#endif /* MULTIPLICITY */

/* silence __declspec(noreturn) warnings */
MSVC_DIAG_IGNORE(4646 4645)
OP *
Perl_die(pTHX_ const char* pat, ...)
{
    va_list args;
    va_start(args, pat);
    vcroak(pat, &args);
    NOT_REACHED; /* NOTREACHED */
    va_end(args);
    NORETURN_FUNCTION_END;
}
MSVC_DIAG_RESTORE

/*
=for apidoc croak_sv

This is an XS interface to Perl's C<die> function.

C<baseex> is the error message or object.  If it is a reference, it
will be used as-is.  Otherwise it is used as a string, and if it does
not end with a newline then it will be extended with some indication of
the current location in the code, as described for L</mess_sv>.

The error message or object will be used as an exception, by default
returning control to the nearest enclosing C<eval>, but subject to
modification by a C<$SIG{__DIE__}> handler.  In any case, the C<croak_sv>
function never returns normally.

To die with a simple string message, the L</croak> function may be
more convenient.

=cut
*/

void
Perl_croak_sv(pTHX_ SV *baseex)
{
    SV *ex = with_queued_errors(mess_sv(baseex, 0));
    PERL_ARGS_ASSERT_CROAK_SV;
    invoke_exception_hook(ex, FALSE);
    die_unwind(ex);
}

/*
=for apidoc vcroak

This is an XS interface to Perl's C<die> function.

C<pat> and C<args> are a sprintf-style format pattern and encapsulated
argument list.  These are used to generate a string message.  If the
message does not end with a newline, then it will be extended with
some indication of the current location in the code, as described for
L</mess_sv>.

The error message will be used as an exception, by default
returning control to the nearest enclosing C<eval>, but subject to
modification by a C<$SIG{__DIE__}> handler.  In any case, the C<croak>
function never returns normally.

For historical reasons, if C<pat> is null then the contents of C<ERRSV>
(C<$@>) will be used as an error message or object instead of building an
error message from arguments.  If you want to throw a non-string object,
or build an error message in an SV yourself, it is preferable to use
the L</croak_sv> function, which does not involve clobbering C<ERRSV>.

=cut
*/

void
Perl_vcroak(pTHX_ const char* pat, va_list *args)
{
    SV *ex = with_queued_errors(pat ? vmess(pat, args) : mess_sv(ERRSV, 0));
    invoke_exception_hook(ex, FALSE);
    die_unwind(ex);
}

/*
=for apidoc croak
=for apidoc_item croak_nocontext

These are XS interfaces to Perl's C<die> function.

They take a sprintf-style format pattern and argument list, which are used to
generate a string message.  If the message does not end with a newline, then it
will be extended with some indication of the current location in the code, as
described for C<L</mess_sv>>.

The error message will be used as an exception, by default
returning control to the nearest enclosing C<eval>, but subject to
modification by a C<$SIG{__DIE__}> handler.  In any case, these croak
functions never return normally.

For historical reasons, if C<pat> is null then the contents of C<ERRSV>
(C<$@>) will be used as an error message or object instead of building an
error message from arguments.  If you want to throw a non-string object,
or build an error message in an SV yourself, it is preferable to use
the C<L</croak_sv>> function, which does not involve clobbering C<ERRSV>.

The two forms differ only in that C<croak_nocontext> does not take a thread
context (C<aTHX>) parameter.  It is usually preferred as it takes up fewer
bytes of code than plain C<Perl_croak>, and time is rarely a critical resource
when you are about to throw an exception.

=cut
*/

#if defined(MULTIPLICITY)
void
Perl_croak_nocontext(const char *pat, ...)
{
    dTHX;
    va_list args;
    va_start(args, pat);
    vcroak(pat, &args);
    NOT_REACHED; /* NOTREACHED */
    va_end(args);
}
#endif /* MULTIPLICITY */

void
Perl_croak(pTHX_ const char *pat, ...)
{
    va_list args;
    va_start(args, pat);
    vcroak(pat, &args);
    NOT_REACHED; /* NOTREACHED */
    va_end(args);
}

/*
=for apidoc croak_no_modify

This encapsulates a common reason for dying, generating terser object code than
using the generic C<Perl_croak>.  It is exactly equivalent to
C<Perl_croak(aTHX_ "%s", PL_no_modify)> (which expands to something like
"Modification of a read-only value attempted").

Less code used on exception code paths reduces CPU cache pressure.

=cut
*/

void
Perl_croak_no_modify(void)
{
    Perl_croak_nocontext( "%s", PL_no_modify);
}

/* does not return, used in util.c perlio.c and win32.c
   This is typically called when malloc returns NULL.
*/
void
Perl_croak_no_mem(void)
{
    dTHX;

    int fd = PerlIO_fileno(Perl_error_log);
    if (fd < 0)
        SETERRNO(EBADF,RMS_IFI);
    else {
        /* Can't use PerlIO to write as it allocates memory */
        PERL_UNUSED_RESULT(PerlLIO_write(fd, PL_no_mem, sizeof(PL_no_mem)-1));
    }
    my_exit(1);
}

/* does not return, used only in POPSTACK */
void
Perl_croak_popstack(void)
{
    dTHX;
    PerlIO_printf(Perl_error_log, "panic: POPSTACK\n");
    my_exit(1);
}

/*
=for apidoc warn_sv

This is an XS interface to Perl's C<warn> function.

C<baseex> is the error message or object.  If it is a reference, it
will be used as-is.  Otherwise it is used as a string, and if it does
not end with a newline then it will be extended with some indication of
the current location in the code, as described for L</mess_sv>.

The error message or object will by default be written to standard error,
but this is subject to modification by a C<$SIG{__WARN__}> handler.

To warn with a simple string message, the L</warn> function may be
more convenient.

=cut
*/

void
Perl_warn_sv(pTHX_ SV *baseex)
{
    SV *ex = mess_sv(baseex, 0);
    PERL_ARGS_ASSERT_WARN_SV;
    if (!invoke_exception_hook(ex, TRUE))
        write_to_stderr(ex);
}

/*
=for apidoc vwarn

This is an XS interface to Perl's C<warn> function.

This is like C<L</warn>>, but C<args> are an encapsulated
argument list.

Unlike with L</vcroak>, C<pat> is not permitted to be null.

=cut
*/

void
Perl_vwarn(pTHX_ const char* pat, va_list *args)
{
    SV *ex = vmess(pat, args);
    PERL_ARGS_ASSERT_VWARN;
    if (!invoke_exception_hook(ex, TRUE))
        write_to_stderr(ex);
}

/*
=for apidoc warn
=for apidoc_item warn_nocontext

These are XS interfaces to Perl's C<warn> function.

They take a sprintf-style format pattern and argument list, which  are used to
generate a string message.  If the message does not end with a newline, then it
will be extended with some indication of the current location in the code, as
described for C<L</mess_sv>>.

The error message or object will by default be written to standard error,
but this is subject to modification by a C<$SIG{__WARN__}> handler.

Unlike with C<L</croak>>, C<pat> is not permitted to be null.

The two forms differ only in that C<warn_nocontext> does not take a thread
context (C<aTHX>) parameter, so is used in situations where the caller doesn't
already have the thread context.

=cut
*/

#if defined(MULTIPLICITY)
void
Perl_warn_nocontext(const char *pat, ...)
{
    dTHX;
    va_list args;
    PERL_ARGS_ASSERT_WARN_NOCONTEXT;
    va_start(args, pat);
    vwarn(pat, &args);
    va_end(args);
}
#endif /* MULTIPLICITY */

void
Perl_warn(pTHX_ const char *pat, ...)
{
    va_list args;
    PERL_ARGS_ASSERT_WARN;
    va_start(args, pat);
    vwarn(pat, &args);
    va_end(args);
}

/*
=for apidoc warner
=for apidoc_item warner_nocontext

These output a warning of the specified category (or categories) given by
C<err>, using the sprintf-style format pattern C<pat>, and argument list.

C<err> must be one of the C<L</packWARN>>, C<packWARN2>, C<packWARN3>,
C<packWARN4> macros populated with the appropriate number of warning
categories.  If any of the warning categories they specify is fatal, a fatal
exception is thrown.

In any event a message is generated by the pattern and arguments.  If the
message does not end with a newline, then it will be extended with some
indication of the current location in the code, as described for L</mess_sv>.

The error message or object will by default be written to standard error,
but this is subject to modification by a C<$SIG{__WARN__}> handler.

C<pat> is not permitted to be null.

The two forms differ only in that C<warner_nocontext> does not take a thread
context (C<aTHX>) parameter, so is used in situations where the caller doesn't
already have the thread context.

These functions differ from the similarly named C<L</warn>> functions, in that
the latter are for XS code to unconditionally display a warning, whereas these
are for code that may be compiling a perl program, and does extra checking to
see if the warning should be fatal.

=for apidoc ck_warner
=for apidoc_item ck_warner_d
If none of the warning categories given by C<err> are enabled, do nothing;
otherwise call C<L</warner>>  or C<L</warner_nocontext>> with the passed-in
parameters;.

C<err> must be one of the C<L</packWARN>>, C<packWARN2>, C<packWARN3>,
C<packWARN4> macros populated with the appropriate number of warning
categories.

The two forms differ only in that C<ck_warner_d> should be used if warnings for
any of the categories are by default enabled.

=for apidoc vwarner
This is like C<L</warner>>, but C<args> are an encapsulated argument list.

=cut
*/

#if defined(MULTIPLICITY)
void
Perl_warner_nocontext(U32 err, const char *pat, ...)
{
    dTHX; 
    va_list args;
    PERL_ARGS_ASSERT_WARNER_NOCONTEXT;
    va_start(args, pat);
    vwarner(err, pat, &args);
    va_end(args);
}
#endif /* MULTIPLICITY */

void
Perl_ck_warner_d(pTHX_ U32 err, const char* pat, ...)
{
    PERL_ARGS_ASSERT_CK_WARNER_D;

    if (Perl_ckwarn_d(aTHX_ err)) {
        va_list args;
        va_start(args, pat);
        vwarner(err, pat, &args);
        va_end(args);
    }
}

void
Perl_ck_warner(pTHX_ U32 err, const char* pat, ...)
{
    PERL_ARGS_ASSERT_CK_WARNER;

    if (Perl_ckwarn(aTHX_ err)) {
        va_list args;
        va_start(args, pat);
        vwarner(err, pat, &args);
        va_end(args);
    }
}

void
Perl_warner(pTHX_ U32  err, const char* pat,...)
{
    va_list args;
    PERL_ARGS_ASSERT_WARNER;
    va_start(args, pat);
    vwarner(err, pat, &args);
    va_end(args);
}

void
Perl_vwarner(pTHX_ U32  err, const char* pat, va_list* args)
{
    PERL_ARGS_ASSERT_VWARNER;
    if (
        (PL_warnhook == PERL_WARNHOOK_FATAL || ckDEAD(err)) &&
        !(PL_in_eval & EVAL_KEEPERR)
    ) {
        SV * const msv = vmess(pat, args);

        if (PL_parser && PL_parser->error_count) {
            qerror(msv);
        }
        else {
            invoke_exception_hook(msv, FALSE);
            die_unwind(msv);
        }
    }
    else {
        Perl_vwarn(aTHX_ pat, args);
    }
}

/* implements the ckWARN? macros */

bool
Perl_ckwarn(pTHX_ U32 w)
{
    /* If lexical warnings have not been set, use $^W.  */
    if (isLEXWARN_off)
        return PL_dowarn & G_WARN_ON;

    return ckwarn_common(w);
}

/* implements the ckWARN?_d macro */

bool
Perl_ckwarn_d(pTHX_ U32 w)
{
    /* If lexical warnings have not been set then default classes warn.  */
    if (isLEXWARN_off)
        return TRUE;

    return ckwarn_common(w);
}

static bool
S_ckwarn_common(pTHX_ U32 w)
{
    if (PL_curcop->cop_warnings == pWARN_ALL)
        return TRUE;

    if (PL_curcop->cop_warnings == pWARN_NONE)
        return FALSE;

    /* Check the assumption that at least the first slot is non-zero.  */
    assert(unpackWARN1(w));

    /* Check the assumption that it is valid to stop as soon as a zero slot is
       seen.  */
    if (!unpackWARN2(w)) {
        assert(!unpackWARN3(w));
        assert(!unpackWARN4(w));
    } else if (!unpackWARN3(w)) {
        assert(!unpackWARN4(w));
    }
        
    /* Right, dealt with all the special cases, which are implemented as non-
       pointers, so there is a pointer to a real warnings mask.  */
    do {
        if (isWARN_on(PL_curcop->cop_warnings, unpackWARN1(w)))
            return TRUE;
    } while (w >>= WARNshift);

    return FALSE;
}

char *
Perl_new_warnings_bitfield(pTHX_ char *buffer, const char *const bits,
                           STRLEN size) {
    const MEM_SIZE len_wanted = (size > WARNsize ? size : WARNsize);
    PERL_UNUSED_CONTEXT;
    PERL_ARGS_ASSERT_NEW_WARNINGS_BITFIELD;

    /* pass in null as the source string as we will do the
     * copy ourselves. */
    buffer = rcpv_new(NULL, len_wanted, RCPVf_NO_COPY);
    Copy(bits, buffer, size, char);
    if (size < WARNsize)
        Zero(buffer + size, WARNsize - size, char);
    return buffer;
}

/* since we've already done strlen() for both nam and val
 * we can use that info to make things faster than
 * sprintf(s, "%s=%s", nam, val)
 */
#define my_setenv_format(s, nam, nlen, val, vlen) \
   Copy(nam, s, nlen, char); \
   *(s+nlen) = '='; \
   Copy(val, s+(nlen+1), vlen, char); \
   *(s+(nlen+1+vlen)) = '\0'



#if defined(USE_ENVIRON_ARRAY) || defined(WIN32)
/* NB: VMS' my_setenv() is in vms.c */

/* small wrapper for use by Perl_my_setenv that mallocs, or reallocs if
 * 'current' is non-null, with up to three sizes that are added together.
 * It handles integer overflow.
 */
#  ifndef HAS_SETENV
static char *
S_env_alloc(void *current, Size_t l1, Size_t l2, Size_t l3, Size_t size)
{
    void *p;
    Size_t sl, l = l1 + l2;

    if (l < l2)
        goto panic;
    l += l3;
    if (l < l3)
        goto panic;
    sl = l * size;
    if (sl < l)
        goto panic;

    p = current
            ? safesysrealloc(current, sl)
            : safesysmalloc(sl);
    if (p)
        return (char*)p;

  panic:
    croak_memory_wrap();
}
#  endif

/*
=for apidoc_section $utility
=for apidoc my_setenv

A wrapper for the C library L<setenv(3)>.  Don't use the latter, as the perl
version has desirable safeguards

=cut
*/

void
Perl_my_setenv(pTHX_ const char *nam, const char *val)
{
#  if defined(USE_ITHREADS) && !defined(WIN32)
    /* only parent thread can modify process environment, so no need to use a
     * mutex */
    if (PL_curinterp != aTHX)
        return;
#  endif

#  if defined(HAS_SETENV) && defined(HAS_UNSETENV)
        if (val == NULL) {
            unsetenv(nam);
        } else {
            setenv(nam, val, 1);
        }

#  elif defined(HAS_UNSETENV)

        if (val == NULL) {
            if (environ) /* old glibc can crash with null environ */
                unsetenv(nam);
        } else {
            const Size_t nlen = strlen(nam);
            const Size_t vlen = strlen(val);
            char * const new_env = S_env_alloc(NULL, nlen, vlen, 2, 1);
            my_setenv_format(new_env, nam, nlen, val, vlen);
            putenv(new_env);
        }

#  else /* ! HAS_UNSETENV */

        const Size_t nlen = strlen(nam);
        if (!val) {
           val = "";
        }
        Size_t vlen = strlen(val);
        char *new_env = S_env_alloc(NULL, nlen, vlen, 2, 1);
        /* all that work just for this */
        my_setenv_format(new_env, nam, nlen, val, vlen);
#    ifndef WIN32
        putenv(new_env);
#    else
        PerlEnv_putenv(new_env);
        safesysfree(new_env);
#    endif

#  endif /* HAS_SETENV */
}

#endif /* USE_ENVIRON_ARRAY || WIN32 */

#ifdef UNLINK_ALL_VERSIONS
I32
Perl_unlnk(pTHX_ const char *f)	/* unlink all versions of a file */
{
    I32 retries = 0;

    PERL_ARGS_ASSERT_UNLNK;

    while (PerlLIO_unlink(f) >= 0)
        retries++;
    return retries ? 0 : -1;
}
#endif

#if defined(OEMVS)
  #if (__CHARSET_LIB == 1)
  static int chgfdccsid(int fd, unsigned short ccsid) 
  {
    attrib_t attr;
    memset(&attr, 0, sizeof(attr));
    attr.att_filetagchg = 1;
    attr.att_filetag.ft_ccsid = ccsid;
    if (ccsid != FT_BINARY) {
      attr.att_filetag.ft_txtflag = 1;
    }
    return __fchattr(fd, &attr, sizeof(attr));
  }
  #endif
#endif

/*
=for apidoc my_popen_list

Implementing function on some systems for PerlProc_popen_list()

=cut
*/

PerlIO *
Perl_my_popen_list(pTHX_ const char *mode, int n, SV **args)
{
#if (!defined(DOSISH) || defined(HAS_FORK)) && !defined(OS2) && !defined(VMS) && !defined(__LIBCATAMOUNT__) && !defined(__amigaos4__)
    int p[2];
    I32 This, that;
    Pid_t pid;
    SV *sv;
    I32 did_pipes = 0;
    int pp[2];

    PERL_ARGS_ASSERT_MY_POPEN_LIST;

    PERL_FLUSHALL_FOR_CHILD;
    This = (*mode == 'w');
    that = !This;
    if (TAINTING_get) {
        taint_env();
        taint_proper("Insecure %s%s", "EXEC");
    }
    if (PerlProc_pipe_cloexec(p) < 0)
        return NULL;
    /* Try for another pipe pair for error return */
    if (PerlProc_pipe_cloexec(pp) >= 0)
        did_pipes = 1;
    while ((pid = PerlProc_fork()) < 0) {
        if (errno != EAGAIN) {
            PerlLIO_close(p[This]);
            PerlLIO_close(p[that]);
            if (did_pipes) {
                PerlLIO_close(pp[0]);
                PerlLIO_close(pp[1]);
            }
            return NULL;
        }
        Perl_ck_warner(aTHX_ packWARN(WARN_PIPE), "Can't fork, trying again in 5 seconds");
        sleep(5);
    }
    if (pid == 0) {
        /* Child */
#undef THIS
#undef THAT
#define THIS that
#define THAT This
        /* Close parent's end of error status pipe (if any) */
        if (did_pipes)
            PerlLIO_close(pp[0]);
#if defined(OEMVS)
  #if (__CHARSET_LIB == 1)
        chgfdccsid(p[THIS], 819);
        chgfdccsid(p[THAT], 819);
  #endif
#endif
        /* Now dup our end of _the_ pipe to right position */
        if (p[THIS] != (*mode == 'r')) {
            PerlLIO_dup2(p[THIS], *mode == 'r');
            PerlLIO_close(p[THIS]);
            if (p[THAT] != (*mode == 'r'))	/* if dup2() didn't close it */
                PerlLIO_close(p[THAT]);	/* close parent's end of _the_ pipe */
        }
        else {
            setfd_cloexec_or_inhexec_by_sysfdness(p[THIS]);
            PerlLIO_close(p[THAT]);	/* close parent's end of _the_ pipe */
        }
#if !defined(HAS_FCNTL) || !defined(F_SETFD)
        /* No automatic close - do it by hand */
#  ifndef NOFILE
#  define NOFILE 20
#  endif
        {
            int fd;

            for (fd = PL_maxsysfd + 1; fd < NOFILE; fd++) {
                if (fd != pp[1])
                    PerlLIO_close(fd);
            }
        }
#endif
        do_aexec5(NULL, args-1, args-1+n, pp[1], did_pipes);
        PerlProc__exit(1);
#undef THIS
#undef THAT
    }
    /* Parent */
    if (did_pipes)
        PerlLIO_close(pp[1]);
    /* Keep the lower of the two fd numbers */
    if (p[that] < p[This]) {
        PerlLIO_dup2_cloexec(p[This], p[that]);
        PerlLIO_close(p[This]);
        p[This] = p[that];
    }
    else
        PerlLIO_close(p[that]);		/* close child's end of pipe */

    sv = *av_fetch(PL_fdpid,p[This],TRUE);
    SvUPGRADE(sv,SVt_IV);
    SvIV_set(sv, pid);
    PL_forkprocess = pid;
    /* If we managed to get status pipe check for exec fail */
    if (did_pipes && pid > 0) {
        int errkid;
        unsigned read_total = 0;

        while (read_total < sizeof(int)) {
            const SSize_t n1 = PerlLIO_read(pp[0],
                              (void*)(((char*)&errkid)+read_total),
                              (sizeof(int)) - read_total);
            if (n1 <= 0)
                break;
            read_total += n1;
        }
        PerlLIO_close(pp[0]);
        did_pipes = 0;
        if (read_total) {			/* Error */
            int pid2, status;
            PerlLIO_close(p[This]);
            if (read_total != sizeof(int))
                Perl_croak(aTHX_ "panic: kid popen errno read, n=%u", read_total);
            do {
                pid2 = wait4pid(pid, &status, 0);
            } while (pid2 == -1 && errno == EINTR);
            errno = errkid;		/* Propagate errno from kid */
            return NULL;
        }
    }
    if (did_pipes)
         PerlLIO_close(pp[0]);
#if defined(OEMVS)
  #if (__CHARSET_LIB == 1)
    PerlIO* io = PerlIO_fdopen(p[This], mode);
    if (io) {
      chgfdccsid(p[This], 819);
    }
    return io;
  #else
    return PerlIO_fdopen(p[This], mode);
  #endif
#else
    return PerlIO_fdopen(p[This], mode);
#endif

#else
#  if defined(OS2)	/* Same, without fork()ing and all extra overhead... */
    return my_syspopen4(aTHX_ NULL, mode, n, args);
#  elif defined(WIN32)
    return win32_popenlist(mode, n, args);
#  else
    Perl_croak(aTHX_ "List form of piped open not implemented");
    return (PerlIO *) NULL;
#  endif
#endif
}

    /* VMS' my_popen() is in VMS.c, same with OS/2 and AmigaOS 4. */
#if (!defined(DOSISH) || defined(HAS_FORK)) && !defined(VMS) && !defined(__LIBCATAMOUNT__) && !defined(__amigaos4__)

/*
=for apidoc_section $io
=for apidoc my_popen

A wrapper for the C library L<popen(3)>.  Don't use the latter, as the Perl
version knows things that interact with the rest of the perl interpreter.

=cut
*/

PerlIO *
Perl_my_popen(pTHX_ const char *cmd, const char *mode)
{
    int p[2];
    I32 This, that;
    Pid_t pid;
    SV *sv;
    const I32 doexec = !(*cmd == '-' && cmd[1] == '\0');
    I32 did_pipes = 0;
    int pp[2];

    PERL_ARGS_ASSERT_MY_POPEN;

    PERL_FLUSHALL_FOR_CHILD;
#ifdef OS2
    if (doexec) {
        return my_syspopen(aTHX_ cmd,mode);
    }
#endif
    This = (*mode == 'w');
    that = !This;
    if (doexec && TAINTING_get) {
        taint_env();
        taint_proper("Insecure %s%s", "EXEC");
    }
    if (PerlProc_pipe_cloexec(p) < 0)
        return NULL;
    if (doexec && PerlProc_pipe_cloexec(pp) >= 0)
        did_pipes = 1;
    while ((pid = PerlProc_fork()) < 0) {
        if (errno != EAGAIN) {
            PerlLIO_close(p[This]);
            PerlLIO_close(p[that]);
            if (did_pipes) {
                PerlLIO_close(pp[0]);
                PerlLIO_close(pp[1]);
            }
            if (!doexec)
                Perl_croak(aTHX_ "Can't fork: %s", Strerror(errno));
            return NULL;
        }
        Perl_ck_warner(aTHX_ packWARN(WARN_PIPE), "Can't fork, trying again in 5 seconds");
        sleep(5);
    }
    if (pid == 0) {

#undef THIS
#undef THAT
#define THIS that
#define THAT This
        if (did_pipes)
            PerlLIO_close(pp[0]);
#if defined(OEMVS)
  #if (__CHARSET_LIB == 1)
        chgfdccsid(p[THIS], 819);
        chgfdccsid(p[THAT], 819);
  #endif
#endif
        if (p[THIS] != (*mode == 'r')) {
            PerlLIO_dup2(p[THIS], *mode == 'r');
            PerlLIO_close(p[THIS]);
            if (p[THAT] != (*mode == 'r'))	/* if dup2() didn't close it */
                PerlLIO_close(p[THAT]);
        }
        else {
            setfd_cloexec_or_inhexec_by_sysfdness(p[THIS]);
            PerlLIO_close(p[THAT]);
        }
#ifndef OS2
        if (doexec) {
#if !defined(HAS_FCNTL) || !defined(F_SETFD)
#ifndef NOFILE
#define NOFILE 20
#endif
            {
                int fd;

                for (fd = PL_maxsysfd + 1; fd < NOFILE; fd++)
                    if (fd != pp[1])
                        PerlLIO_close(fd);
            }
#endif
            /* may or may not use the shell */
            do_exec3(cmd, pp[1], did_pipes);
            PerlProc__exit(1);
        }
#endif	/* defined OS2 */

#ifdef PERLIO_USING_CRLF
   /* Since we circumvent IO layers when we manipulate low-level
      filedescriptors directly, need to manually switch to the
      default, binary, low-level mode; see PerlIOBuf_open(). */
   PerlLIO_setmode((*mode == 'r'), O_BINARY);
#endif 
        PL_forkprocess = 0;
#ifdef PERL_USES_PL_PIDSTATUS
        hv_clear(PL_pidstatus);	/* we have no children */
#endif
        return NULL;
#undef THIS
#undef THAT
    }
    if (did_pipes)
        PerlLIO_close(pp[1]);
    if (p[that] < p[This]) {
        PerlLIO_dup2_cloexec(p[This], p[that]);
        PerlLIO_close(p[This]);
        p[This] = p[that];
    }
    else
        PerlLIO_close(p[that]);

    sv = *av_fetch(PL_fdpid,p[This],TRUE);
    SvUPGRADE(sv,SVt_IV);
    SvIV_set(sv, pid);
    PL_forkprocess = pid;
    if (did_pipes && pid > 0) {
        int errkid;
        unsigned n = 0;

        while (n < sizeof(int)) {
            const SSize_t n1 = PerlLIO_read(pp[0],
                              (void*)(((char*)&errkid)+n),
                              (sizeof(int)) - n);
            if (n1 <= 0)
                break;
            n += n1;
        }
        PerlLIO_close(pp[0]);
        did_pipes = 0;
        if (n) {			/* Error */
            int pid2, status;
            PerlLIO_close(p[This]);
            if (n != sizeof(int))
                Perl_croak(aTHX_ "panic: kid popen errno read, n=%u", n);
            do {
                pid2 = wait4pid(pid, &status, 0);
            } while (pid2 == -1 && errno == EINTR);
            errno = errkid;		/* Propagate errno from kid */
            return NULL;
        }
    }
    if (did_pipes)
         PerlLIO_close(pp[0]);
#if defined(OEMVS)
  #if (__CHARSET_LIB == 1)
    PerlIO* io = PerlIO_fdopen(p[This],	mode);
    if (io) {
      chgfdccsid(p[This], 819);
    }
    return io;
  #else
    return PerlIO_fdopen(p[This], mode);
  #endif
#else
    return PerlIO_fdopen(p[This], mode);
#endif
}
#elif defined(__LIBCATAMOUNT__)
PerlIO *
Perl_my_popen(pTHX_ const char *cmd, const char *mode)
{
    return NULL;
}

#endif /* !DOSISH */

/* this is called in parent before the fork() */
void
Perl_atfork_lock(void)
#if defined(USE_ITHREADS)
#  ifdef USE_PERLIO
  PERL_TSA_ACQUIRE(PL_perlio_mutex)
#  endif
#  ifdef MYMALLOC
  PERL_TSA_ACQUIRE(PL_malloc_mutex)
#  endif
  PERL_TSA_ACQUIRE(PL_op_mutex)
#endif
{
#if defined(USE_ITHREADS)
    /* locks must be held in locking order (if any) */
#  ifdef USE_PERLIO
    MUTEX_LOCK(&PL_perlio_mutex);
#  endif
#  ifdef MYMALLOC
    MUTEX_LOCK(&PL_malloc_mutex);
#  endif
    OP_REFCNT_LOCK;
#endif
}

/* this is called in both parent and child after the fork() */
void
Perl_atfork_unlock(void)
#if defined(USE_ITHREADS)
#  ifdef USE_PERLIO
  PERL_TSA_RELEASE(PL_perlio_mutex)
#  endif
#  ifdef MYMALLOC
  PERL_TSA_RELEASE(PL_malloc_mutex)
#  endif
  PERL_TSA_RELEASE(PL_op_mutex)
#endif
{
#if defined(USE_ITHREADS)
    /* locks must be released in same order as in atfork_lock() */
#  ifdef USE_PERLIO
    MUTEX_UNLOCK(&PL_perlio_mutex);
#  endif
#  ifdef MYMALLOC
    MUTEX_UNLOCK(&PL_malloc_mutex);
#  endif
    OP_REFCNT_UNLOCK;
#endif
}

/*
=for apidoc_section $concurrency
=for apidoc my_fork

This is for the use of C<PerlProc_fork> as a wrapper for the C library
L<fork(2)> on some platforms to hide some platform quirks.  It should not be
used except through C<PerlProc_fork>.

=cut
*/


Pid_t
Perl_my_fork(void)
{
#if defined(HAS_FORK)
    Pid_t pid;
#if defined(USE_ITHREADS) && !defined(HAS_PTHREAD_ATFORK)
    atfork_lock();
    pid = fork();
    atfork_unlock();
#else
    /* atfork_lock() and atfork_unlock() are installed as pthread_atfork()
     * handlers elsewhere in the code */
    pid = fork();
#endif
    return pid;
#elif defined(__amigaos4__)
    return amigaos_fork();
#else
    /* this "canna happen" since nothing should be calling here if !HAS_FORK */
    Perl_croak_nocontext("fork() not available");
    return 0;
#endif /* HAS_FORK */
}

#ifndef HAS_DUP2
int
dup2(int oldfd, int newfd)
{
#if defined(HAS_FCNTL) && defined(F_DUPFD)
    if (oldfd == newfd)
        return oldfd;
    PerlLIO_close(newfd);
    return fcntl(oldfd, F_DUPFD, newfd);
#else
#define DUP2_MAX_FDS 256
    int fdtmp[DUP2_MAX_FDS];
    I32 fdx = 0;
    int fd;

    if (oldfd == newfd)
        return oldfd;
    PerlLIO_close(newfd);
    /* good enough for low fd's... */
    while ((fd = PerlLIO_dup(oldfd)) != newfd && fd >= 0) {
        if (fdx >= DUP2_MAX_FDS) {
            PerlLIO_close(fd);
            fd = -1;
            break;
        }
        fdtmp[fdx++] = fd;
    }
    while (fdx > 0)
        PerlLIO_close(fdtmp[--fdx]);
    return fd;
#endif
}
#endif

#ifndef PERL_MICRO
#ifdef HAS_SIGACTION

/*
=for apidoc_section $signals
=for apidoc rsignal

A wrapper for the C library functions L<sigaction(2)> or L<signal(2)>.
Use this instead of those libc functions, as the Perl version gives the
safest available implementation, and knows things that interact with the
rest of the perl interpreter.

=cut
*/

Sighandler_t
Perl_rsignal(pTHX_ int signo, Sighandler_t handler)
{
    struct sigaction act, oact;

#ifdef USE_ITHREADS
    /* only "parent" interpreter can diddle signals */
    if (PL_curinterp != aTHX)
        return (Sighandler_t) SIG_ERR;
#endif

    act.sa_handler = handler;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
#ifdef SA_RESTART
    if (PL_signals & PERL_SIGNALS_UNSAFE_FLAG)
        act.sa_flags |= SA_RESTART;	/* SVR4, 4.3+BSD */
#endif
#if defined(SA_NOCLDWAIT) && !defined(BSDish) /* See [perl #18849] */
    if (signo == SIGCHLD && handler == (Sighandler_t) SIG_IGN)
        act.sa_flags |= SA_NOCLDWAIT;
#endif
    if (sigaction(signo, &act, &oact) == -1)
        return (Sighandler_t) SIG_ERR;
    else
        return (Sighandler_t) oact.sa_handler;
}

/*
=for apidoc_section $signals
=for apidoc rsignal_state

Returns a the current signal handler for signal C<signo>.
See L</C<rsignal>>.

=cut
*/

Sighandler_t
Perl_rsignal_state(pTHX_ int signo)
{
    struct sigaction oact;
    PERL_UNUSED_CONTEXT;

    if (sigaction(signo, (struct sigaction *)NULL, &oact) == -1)
        return (Sighandler_t) SIG_ERR;
    else
        return (Sighandler_t) oact.sa_handler;
}

int
Perl_rsignal_save(pTHX_ int signo, Sighandler_t handler, Sigsave_t *save)
{
    struct sigaction act;

    PERL_ARGS_ASSERT_RSIGNAL_SAVE;

#ifdef USE_ITHREADS
    /* only "parent" interpreter can diddle signals */
    if (PL_curinterp != aTHX)
        return -1;
#endif

    act.sa_handler = handler;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
#ifdef SA_RESTART
    if (PL_signals & PERL_SIGNALS_UNSAFE_FLAG)
        act.sa_flags |= SA_RESTART;	/* SVR4, 4.3+BSD */
#endif
#if defined(SA_NOCLDWAIT) && !defined(BSDish) /* See [perl #18849] */
    if (signo == SIGCHLD && handler == (Sighandler_t) SIG_IGN)
        act.sa_flags |= SA_NOCLDWAIT;
#endif
    return sigaction(signo, &act, save);
}

int
Perl_rsignal_restore(pTHX_ int signo, Sigsave_t *save)
{
    PERL_UNUSED_CONTEXT;
#ifdef USE_ITHREADS
    /* only "parent" interpreter can diddle signals */
    if (PL_curinterp != aTHX)
        return -1;
#endif

    return sigaction(signo, save, (struct sigaction *)NULL);
}

#else /* !HAS_SIGACTION */

Sighandler_t
Perl_rsignal(pTHX_ int signo, Sighandler_t handler)
{
#if defined(USE_ITHREADS) && !defined(WIN32)
    /* only "parent" interpreter can diddle signals */
    if (PL_curinterp != aTHX)
        return (Sighandler_t) SIG_ERR;
#endif

    return PerlProc_signal(signo, handler);
}

static Signal_t
sig_trap(int signo)
{
    PL_sig_trapped++;
}

Sighandler_t
Perl_rsignal_state(pTHX_ int signo)
{
    Sighandler_t oldsig;

#if defined(USE_ITHREADS) && !defined(WIN32)
    /* only "parent" interpreter can diddle signals */
    if (PL_curinterp != aTHX)
        return (Sighandler_t) SIG_ERR;
#endif

    PL_sig_trapped = 0;
    oldsig = PerlProc_signal(signo, sig_trap);
    PerlProc_signal(signo, oldsig);
    if (PL_sig_trapped)
        PerlProc_kill(PerlProc_getpid(), signo);
    return oldsig;
}

int
Perl_rsignal_save(pTHX_ int signo, Sighandler_t handler, Sigsave_t *save)
{
#if defined(USE_ITHREADS) && !defined(WIN32)
    /* only "parent" interpreter can diddle signals */
    if (PL_curinterp != aTHX)
        return -1;
#endif
    *save = PerlProc_signal(signo, handler);
    return (*save == (Sighandler_t) SIG_ERR) ? -1 : 0;
}

int
Perl_rsignal_restore(pTHX_ int signo, Sigsave_t *save)
{
#if defined(USE_ITHREADS) && !defined(WIN32)
    /* only "parent" interpreter can diddle signals */
    if (PL_curinterp != aTHX)
        return -1;
#endif
    return (PerlProc_signal(signo, *save) == (Sighandler_t) SIG_ERR) ? -1 : 0;
}

#endif /* !HAS_SIGACTION */
#endif /* !PERL_MICRO */

    /* VMS' my_pclose() is in VMS.c */

/*
=for apidoc_section $io
=for apidoc my_pclose

A wrapper for the C library L<pclose(3)>.  Don't use the latter, as the Perl
version knows things that interact with the rest of the perl interpreter.

=cut
*/

#if (!defined(DOSISH) || defined(HAS_FORK)) && !defined(VMS) && !defined(__LIBCATAMOUNT__) && !defined(__amigaos4__)
I32
Perl_my_pclose(pTHX_ PerlIO *ptr)
{
    int status;
    SV **svp;
    Pid_t pid;
    Pid_t pid2 = 0;
    bool close_failed;
    dSAVEDERRNO;
    const int fd = PerlIO_fileno(ptr);
    bool should_wait;

    svp = av_fetch(PL_fdpid, fd, FALSE);
    if (svp) {
        pid = (SvTYPE(*svp) == SVt_IV) ? SvIVX(*svp) : -1;
        SvREFCNT_dec(*svp);
        *svp = NULL;
    } else {
        pid = -1;
    }

#if defined(USE_PERLIO)
    /* Find out whether the refcount is low enough for us to wait for the
       child proc without blocking. */
    should_wait = PerlIOUnix_refcnt(fd) == 1 && pid > 0;
#else
    should_wait = pid > 0;
#endif

#ifdef OS2
    if (pid == -2) {                    /* Opened by popen. */
        return my_syspclose(ptr);
    }
#endif
    close_failed = (PerlIO_close(ptr) == EOF);
    SAVE_ERRNO;
    if (should_wait) do {
        pid2 = wait4pid(pid, &status, 0);
    } while (pid2 == -1 && errno == EINTR);
    if (close_failed) {
        RESTORE_ERRNO;
        return -1;
    }
    return(
      should_wait
       ? pid2 < 0 ? pid2 : status == 0 ? 0 : (errno = 0, status)
       : 0
    );
}
#elif defined(__LIBCATAMOUNT__)
I32
Perl_my_pclose(pTHX_ PerlIO *ptr)
{
    return -1;
}
#endif /* !DOSISH */

#if  (!defined(DOSISH) || defined(OS2) || defined(WIN32)) && !defined(__LIBCATAMOUNT__)
I32
Perl_wait4pid(pTHX_ Pid_t pid, int *statusp, int flags)
{
    I32 result = 0;
    PERL_ARGS_ASSERT_WAIT4PID;
#ifdef PERL_USES_PL_PIDSTATUS
    if (!pid) {
        /* PERL_USES_PL_PIDSTATUS is only defined when neither
           waitpid() nor wait4() is available, or on OS/2, which
           doesn't appear to support waiting for a progress group
           member, so we can only treat a 0 pid as an unknown child.
        */
        errno = ECHILD;
        return -1;
    }
    {
        if (pid > 0) {
            /* The keys in PL_pidstatus are now the raw 4 (or 8) bytes of the
               pid, rather than a string form.  */
            SV * const * const svp = hv_fetch(PL_pidstatus,(const char*) &pid,sizeof(Pid_t),FALSE);
            if (svp && *svp != &PL_sv_undef) {
                *statusp = SvIVX(*svp);
                (void)hv_delete(PL_pidstatus,(const char*) &pid,sizeof(Pid_t),
                                G_DISCARD);
                return pid;
            }
        }
        else {
            HE *entry;

            hv_iterinit(PL_pidstatus);
            if ((entry = hv_iternext(PL_pidstatus))) {
                SV * const sv = hv_iterval(PL_pidstatus,entry);
                I32 len;
                const char * const spid = hv_iterkey(entry,&len);

                assert (len == sizeof(Pid_t));
                memcpy((char *)&pid, spid, len);
                *statusp = SvIVX(sv);
                /* The hash iterator is currently on this entry, so simply
                   calling hv_delete would trigger the lazy delete, which on
                   aggregate does more work, because next call to hv_iterinit()
                   would spot the flag, and have to call the delete routine,
                   while in the meantime any new entries can't re-use that
                   memory.  */
                hv_iterinit(PL_pidstatus);
                (void)hv_delete(PL_pidstatus,spid,len,G_DISCARD);
                return pid;
            }
        }
    }
#endif
#ifdef HAS_WAITPID
#  ifdef HAS_WAITPID_RUNTIME
    if (!HAS_WAITPID_RUNTIME)
        goto hard_way;
#  endif
    result = PerlProc_waitpid(pid,statusp,flags);
    goto finish;
#endif
#if !defined(HAS_WAITPID) && defined(HAS_WAIT4)
    result = wait4(pid,statusp,flags,NULL);
    goto finish;
#endif
#ifdef PERL_USES_PL_PIDSTATUS
#if defined(HAS_WAITPID) && defined(HAS_WAITPID_RUNTIME)
  hard_way:
#endif
    {
        if (flags)
            Perl_croak(aTHX_ "Can't do waitpid with flags");
        else {
            while ((result = PerlProc_wait(statusp)) != pid && pid > 0 && result >= 0)
                pidgone(result,*statusp);
            if (result < 0)
                *statusp = -1;
        }
    }
#endif
#if defined(HAS_WAITPID) || defined(HAS_WAIT4)
  finish:
#endif
    if (result < 0 && errno == EINTR) {
        PERL_ASYNC_CHECK();
        errno = EINTR; /* reset in case a signal handler changed $! */
    }
    return result;
}
#endif /* !DOSISH || OS2 || WIN32 */

#ifdef PERL_USES_PL_PIDSTATUS
void
S_pidgone(pTHX_ Pid_t pid, int status)
{
    SV *sv;

    sv = *hv_fetch(PL_pidstatus,(const char*)&pid,sizeof(Pid_t),TRUE);
    SvUPGRADE(sv,SVt_IV);
    SvIV_set(sv, status);
    return;
}
#endif

#if defined(OS2)
int pclose();
#ifdef HAS_FORK
int					/* Cannot prototype with I32
                                           in os2ish.h. */
my_syspclose(PerlIO *ptr)
#else
I32
Perl_my_pclose(pTHX_ PerlIO *ptr)
#endif
{
    /* Needs work for PerlIO ! */
    FILE * const f = PerlIO_findFILE(ptr);
    const I32 result = pclose(f);
    PerlIO_releaseFILE(ptr,f);
    return result;
}
#endif

/*
=for apidoc repeatcpy

Make C<count> copies of the C<len> bytes beginning at C<from>, placing them
into memory beginning at C<to>, which must be big enough to accommodate them
all.

=cut
*/

#define PERL_REPEATCPY_LINEAR 4
void
Perl_repeatcpy(char *to, const char *from, I32 len, IV count)
{
    PERL_ARGS_ASSERT_REPEATCPY;

    assert(len >= 0);

    if (count < 0)
        croak_memory_wrap();

    if (len == 1)
        memset(to, *from, count);
    else if (count) {
        char *p = to;
        IV items, linear, half;

        linear = count < PERL_REPEATCPY_LINEAR ? count : PERL_REPEATCPY_LINEAR;
        for (items = 0; items < linear; ++items) {
            const char *q = from;
            IV todo;
            for (todo = len; todo > 0; todo--)
                *p++ = *q++;
        }

        half = count / 2;
        while (items <= half) {
            IV size = items * len;
            memcpy(p, to, size);
            p     += size;
            items *= 2;
        }

        if (count > items)
            memcpy(p, to, (count - items) * len);
    }
}

#ifndef HAS_RENAME
I32
Perl_same_dirent(pTHX_ const char *a, const char *b)
{
    char *fa = strrchr(a,'/');
    char *fb = strrchr(b,'/');
    Stat_t tmpstatbuf1;
    Stat_t tmpstatbuf2;
    SV * const tmpsv = sv_newmortal();

    PERL_ARGS_ASSERT_SAME_DIRENT;

    if (fa)
        fa++;
    else
        fa = a;
    if (fb)
        fb++;
    else
        fb = b;
    if (strNE(a,b))
        return FALSE;
    if (fa == a)
        sv_setpvs(tmpsv, ".");
    else
        sv_setpvn(tmpsv, a, fa - a);
    if (PerlLIO_stat(SvPVX_const(tmpsv), &tmpstatbuf1) < 0)
        return FALSE;
    if (fb == b)
        sv_setpvs(tmpsv, ".");
    else
        sv_setpvn(tmpsv, b, fb - b);
    if (PerlLIO_stat(SvPVX_const(tmpsv), &tmpstatbuf2) < 0)
        return FALSE;
    return tmpstatbuf1.st_dev == tmpstatbuf2.st_dev &&
           tmpstatbuf1.st_ino == tmpstatbuf2.st_ino;
}
#endif /* !HAS_RENAME */

char*
Perl_find_script(pTHX_ const char *scriptname, bool dosearch,
                 const char *const *const search_ext, I32 flags)
{
    const char *xfound = NULL;
    char *xfailed = NULL;
    char tmpbuf[MAXPATHLEN];
    char *s;
    I32 len = 0;
    int retval;
    char *bufend;
#if defined(DOSISH) && !defined(OS2)
#  define SEARCH_EXTS ".bat", ".cmd", NULL
#  define MAX_EXT_LEN 4
#endif
#ifdef OS2
#  define SEARCH_EXTS ".cmd", ".btm", ".bat", ".pl", NULL
#  define MAX_EXT_LEN 4
#endif
#ifdef VMS
#  define SEARCH_EXTS ".pl", ".com", NULL
#  define MAX_EXT_LEN 4
#endif
    /* additional extensions to try in each dir if scriptname not found */
#ifdef SEARCH_EXTS
    static const char *const exts[] = { SEARCH_EXTS };
    const char *const *const ext = search_ext ? search_ext : exts;
    int extidx = 0, i = 0;
    const char *curext = NULL;
#else
    PERL_UNUSED_ARG(search_ext);
#  define MAX_EXT_LEN 0
#endif

    PERL_ARGS_ASSERT_FIND_SCRIPT;

    /*
     * If dosearch is true and if scriptname does not contain path
     * delimiters, search the PATH for scriptname.
     *
     * If SEARCH_EXTS is also defined, will look for each
     * scriptname{SEARCH_EXTS} whenever scriptname is not found
     * while searching the PATH.
     *
     * Assuming SEARCH_EXTS is C<".foo",".bar",NULL>, PATH search
     * proceeds as follows:
     *   If DOSISH or VMSISH:
     *     + look for ./scriptname{,.foo,.bar}
     *     + search the PATH for scriptname{,.foo,.bar}
     *
     *   If !DOSISH:
     *     + look *only* in the PATH for scriptname{,.foo,.bar} (note
     *       this will not look in '.' if it's not in the PATH)
     */
    tmpbuf[0] = '\0';

#ifdef VMS
#  ifdef ALWAYS_DEFTYPES
    len = strlen(scriptname);
    if (!(len == 1 && *scriptname == '-') && scriptname[len-1] != ':') {
        int idx = 0, deftypes = 1;
        bool seen_dot = 1;

        const int hasdir = !dosearch || (strpbrk(scriptname,":[</") != NULL);
#  else
    if (dosearch) {
        int idx = 0, deftypes = 1;
        bool seen_dot = 1;

        const int hasdir = (strpbrk(scriptname,":[</") != NULL);
#  endif
        /* The first time through, just add SEARCH_EXTS to whatever we
         * already have, so we can check for default file types. */
        while (deftypes ||
               (!hasdir && my_trnlnm("DCL$PATH",tmpbuf,idx++)) )
        {
            Stat_t statbuf;
            if (deftypes) {
                deftypes = 0;
                *tmpbuf = '\0';
            }
            if ((strlen(tmpbuf) + strlen(scriptname)
                 + MAX_EXT_LEN) >= sizeof tmpbuf)
                continue;	/* don't search dir with too-long name */
            my_strlcat(tmpbuf, scriptname, sizeof(tmpbuf));
#else  /* !VMS */

#ifdef DOSISH
    if (strEQ(scriptname, "-"))
        dosearch = 0;
    if (dosearch) {		/* Look in '.' first. */
        const char *cur = scriptname;
#ifdef SEARCH_EXTS
        if ((curext = strrchr(scriptname,'.')))	/* possible current ext */
            while (ext[i])
                if (strEQ(ext[i++],curext)) {
                    extidx = -1;		/* already has an ext */
                    break;
                }
        do {
#endif
            DEBUG_p(PerlIO_printf(Perl_debug_log,
                                  "Looking for %s\n",cur));
            {
                Stat_t statbuf;
                if (PerlLIO_stat(cur,&statbuf) >= 0
                    && !S_ISDIR(statbuf.st_mode)) {
                    dosearch = 0;
                    scriptname = cur;
#ifdef SEARCH_EXTS
                    break;
#endif
                }
            }
#ifdef SEARCH_EXTS
            if (cur == scriptname) {
                len = strlen(scriptname);
                if (len+MAX_EXT_LEN+1 >= sizeof(tmpbuf))
                    break;
                my_strlcpy(tmpbuf, scriptname, sizeof(tmpbuf));
                cur = tmpbuf;
            }
        } while (extidx >= 0 && ext[extidx]	/* try an extension? */
                 && my_strlcpy(tmpbuf+len, ext[extidx++], sizeof(tmpbuf) - len));
#endif
    }
#endif

    if (dosearch && !strchr(scriptname, '/')
#ifdef DOSISH
                 && !strchr(scriptname, '\\')
#endif
                 && (s = PerlEnv_getenv("PATH")))
    {
        bool seen_dot = 0;

        bufend = s + strlen(s);
        while (s < bufend) {
            Stat_t statbuf;
#  ifdef DOSISH
            for (len = 0; *s
                    && *s != ';'; len++, s++) {
                if (len < sizeof tmpbuf)
                    tmpbuf[len] = *s;
            }
            if (len < sizeof tmpbuf)
                tmpbuf[len] = '\0';
#  else
            s = delimcpy_no_escape(tmpbuf, tmpbuf + sizeof tmpbuf, s, bufend,
                                   ':', &len);
#  endif
            if (s < bufend)
                s++;
            if (len + 1 + strlen(scriptname) + MAX_EXT_LEN >= sizeof tmpbuf)
                continue;	/* don't search dir with too-long name */
            if (len
#  ifdef DOSISH
                && tmpbuf[len - 1] != '/'
                && tmpbuf[len - 1] != '\\'
#  endif
               )
                tmpbuf[len++] = '/';
            if (len == 2 && tmpbuf[0] == '.')
                seen_dot = 1;
            (void)my_strlcpy(tmpbuf + len, scriptname, sizeof(tmpbuf) - len);
#endif  /* !VMS */

#ifdef SEARCH_EXTS
            len = strlen(tmpbuf);
            if (extidx > 0)	/* reset after previous loop */
                extidx = 0;
            do {
#endif
                DEBUG_p(PerlIO_printf(Perl_debug_log, "Looking for %s\n",tmpbuf));
                retval = PerlLIO_stat(tmpbuf,&statbuf);
                if (S_ISDIR(statbuf.st_mode)) {
                    retval = -1;
                }
#ifdef SEARCH_EXTS
            } while (  retval < 0		/* not there */
                    && extidx>=0 && ext[extidx]	/* try an extension? */
                    && my_strlcpy(tmpbuf+len, ext[extidx++], sizeof(tmpbuf) - len)
                );
#endif
            if (retval < 0)
                continue;
            if (S_ISREG(statbuf.st_mode)
                && cando(S_IRUSR,TRUE,&statbuf)
#if !defined(DOSISH)
                && cando(S_IXUSR,TRUE,&statbuf)
#endif
                )
            {
                xfound = tmpbuf;		/* bingo! */
                break;
            }
            if (!xfailed)
                xfailed = savepv(tmpbuf);
        }
#ifndef DOSISH
        {
            Stat_t statbuf;
            if (!xfound && !seen_dot && !xfailed &&
                (PerlLIO_stat(scriptname,&statbuf) < 0
                 || S_ISDIR(statbuf.st_mode)))
#endif
                seen_dot = 1;			/* Disable message. */
#ifndef DOSISH
        }
#endif
        if (!xfound) {
            if (flags & 1) {			/* do or die? */
                /* diag_listed_as: Can't execute %s */
                Perl_croak(aTHX_ "Can't %s %s%s%s",
                      (xfailed ? "execute" : "find"),
                      (xfailed ? xfailed : scriptname),
                      (xfailed ? "" : " on PATH"),
                      (xfailed || seen_dot) ? "" : ", '.' not in PATH");
            }
            scriptname = NULL;
        }
        Safefree(xfailed);
        scriptname = xfound;
    }
    return (scriptname ? savepv(scriptname) : NULL);
}

#ifndef PERL_GET_CONTEXT_DEFINED

/*
=for apidoc_section $embedding
=for apidoc set_context

Implements L<perlapi/C<PERL_SET_CONTEXT>>, which you should use instead.

=cut
*/

void
Perl_set_context(void *t)
{
    PERL_ARGS_ASSERT_SET_CONTEXT;
#if defined(USE_ITHREADS)
#  ifdef PERL_USE_THREAD_LOCAL
    PL_current_context = t;
#  endif
#  ifdef I_MACH_CTHREADS
    cthread_set_data(cthread_self(), t);
#  else
    /* We set thread-specific value always, as C++ code has to read it with
     * pthreads, because the declaration syntax for thread local storage for C11
     * is incompatible with C++, meaning that we can't expose the thread local
     * variable to C++ code. */
    {
        const int error = pthread_setspecific(PL_thr_key, t);
        if (error)
            Perl_croak_nocontext("panic: pthread_setspecific, error=%d", error);
    }
#  endif

    PERL_SET_NON_tTHX_CONTEXT(t);

#else
    PERL_UNUSED_ARG(t);
#endif
}

#endif /* !PERL_GET_CONTEXT_DEFINED */

/*
=for apidoc get_op_names

Return a pointer to the array of all the names of the various OPs
Given an opcode from the enum in F<opcodes.h>, C<PL_op_name[opcode]> returns a
pointer to a C language string giving its name.

=cut
*/

char **
Perl_get_op_names(pTHX)
{
    PERL_UNUSED_CONTEXT;
    return (char **)PL_op_name;
}

/*
=for apidoc get_op_descs

Return a pointer to the array of all the descriptions of the various OPs
Given an opcode from the enum in F<opcodes.h>, C<PL_op_desc[opcode]> returns a
pointer to a C language string giving its description.

=cut
*/

char **
Perl_get_op_descs(pTHX)
{
    PERL_UNUSED_CONTEXT;
    return (char **)PL_op_desc;
}

const char *
Perl_get_no_modify(pTHX)
{
    PERL_UNUSED_CONTEXT;
    return PL_no_modify;
}

U32 *
Perl_get_opargs(pTHX)
{
    PERL_UNUSED_CONTEXT;
    return (U32 *)PL_opargs;
}

PPADDR_t*
Perl_get_ppaddr(pTHX)
{
    PERL_UNUSED_CONTEXT;
    return (PPADDR_t*)PL_ppaddr;
}

#ifndef HAS_GETENV_LEN
char *
Perl_getenv_len(pTHX_ const char *env_elem, unsigned long *len)
{
    char * const env_trans = PerlEnv_getenv(env_elem);
    PERL_UNUSED_CONTEXT;
    PERL_ARGS_ASSERT_GETENV_LEN;
    if (env_trans)
        *len = strlen(env_trans);
    return env_trans;
}
#endif

/*
=for apidoc_section $io
=for apidoc my_fflush_all

Implements C<PERL_FLUSHALL_FOR_CHILD> on some platforms.

=cut
 */

I32
Perl_my_fflush_all(pTHX)
{
#if defined(USE_PERLIO) || defined(FFLUSH_NULL)
    return PerlIO_flush(NULL);
#else
# if defined(HAS__FWALK)
    extern int fflush(FILE *);
    /* undocumented, unprototyped, but very useful BSDism */
    extern void _fwalk(int (*)(FILE *));
    _fwalk(&fflush);
    return 0;
# else
#  if defined(FFLUSH_ALL) && defined(HAS_STDIO_STREAM_ARRAY)
    long open_max = -1;
#   ifdef PERL_FFLUSH_ALL_FOPEN_MAX
    open_max = PERL_FFLUSH_ALL_FOPEN_MAX;
#   elif defined(HAS_SYSCONF) && defined(_SC_OPEN_MAX)
    open_max = sysconf(_SC_OPEN_MAX);
#   elif defined(FOPEN_MAX)
    open_max = FOPEN_MAX;
#   elif defined(OPEN_MAX)
    open_max = OPEN_MAX;
#   elif defined(_NFILE)
    open_max = _NFILE;
#   endif
    if (open_max > 0) {
      long i;
      for (i = 0; i < open_max; i++)
            if (STDIO_STREAM_ARRAY[i]._file >= 0 &&
                STDIO_STREAM_ARRAY[i]._file < open_max &&
                STDIO_STREAM_ARRAY[i]._flag)
                PerlIO_flush(&STDIO_STREAM_ARRAY[i]);
      return 0;
    }
#  endif
    SETERRNO(EBADF,RMS_IFI);
    return EOF;
# endif
#endif
}

void
Perl_report_wrongway_fh(pTHX_ const GV *gv, const char have)
{
    if (ckWARN(WARN_IO)) {
        HEK * const name
           = gv && (isGV_with_GP(gv))
                ? GvENAME_HEK((gv))
                : NULL;
        const char * const direction = have == '>' ? "out" : "in";

        if (name && HEK_LEN(name))
            Perl_warner(aTHX_ packWARN(WARN_IO),
                        "Filehandle %" HEKf " opened only for %sput",
                        HEKfARG(name), direction);
        else
            Perl_warner(aTHX_ packWARN(WARN_IO),
                        "Filehandle opened only for %sput", direction);
    }
}

void
Perl_report_evil_fh(pTHX_ const GV *gv)
{
    const IO *io = gv ? GvIO(gv) : NULL;
    const PERL_BITFIELD16 op = PL_op->op_type;
    const char *vile;
    I32 warn_type;

    if (io && IoTYPE(io) == IoTYPE_CLOSED) {
        vile = "closed";
        warn_type = WARN_CLOSED;
    }
    else {
        vile = "unopened";
        warn_type = WARN_UNOPENED;
    }

    if (ckWARN(warn_type)) {
        SV * const name
            = gv && isGV_with_GP(gv) && GvENAMELEN(gv) ?
                                     newSVhek_mortal(GvENAME_HEK(gv)) : NULL;
        const char * const pars =
            (const char *)(OP_IS_FILETEST(op) ? "" : "()");
        const char * const func =
            (const char *)
            (op == OP_READLINE || op == OP_RCATLINE
                                 ? "readline"  :	/* "<HANDLE>" not nice */
             op == OP_LEAVEWRITE ? "write" :		/* "write exit" not nice */
             PL_op_desc[op]);
        const char * const type =
            (const char *)
            (OP_IS_SOCKET(op) || (io && IoTYPE(io) == IoTYPE_SOCKET)
             ? "socket" : "filehandle");
        const bool have_name = name && SvCUR(name);
        Perl_warner(aTHX_ packWARN(warn_type),
                   "%s%s on %s %s%s%" SVf, func, pars, vile, type,
                    have_name ? " " : "",
                    SVfARG(have_name ? name : &PL_sv_no));
        if (io && IoDIRP(io) && !(IoFLAGS(io) & IOf_FAKE_DIRP))
                Perl_warner(
                            aTHX_ packWARN(warn_type),
                        "\t(Are you trying to call %s%s on dirhandle%s%" SVf "?)\n",
                        func, pars, have_name ? " " : "",
                        SVfARG(have_name ? name : &PL_sv_no)
                            );
    }
}

/* To workaround core dumps from the uninitialised tm_zone we get the
 * system to give us a reasonable struct to copy.  This fix means that
 * strftime uses the tm_zone and tm_gmtoff values returned by
 * localtime(time()). That should give the desired result most of the
 * time. But probably not always!
 *
 * This does not address tzname aspects of NETaa14816.
 *
 */

#ifdef __GLIBC__
# ifndef STRUCT_TM_HASZONE
#    define STRUCT_TM_HASZONE
# endif
#endif

#ifdef STRUCT_TM_HASZONE /* Backward compat */
# ifndef HAS_TM_TM_ZONE
#    define HAS_TM_TM_ZONE
# endif
#endif

void
Perl_init_tm(pTHX_ struct tm *ptm)	/* see mktime, strftime and asctime */
{
#ifdef HAS_TM_TM_ZONE
    Time_t now;
    const struct tm* my_tm;
    PERL_UNUSED_CONTEXT;
    PERL_ARGS_ASSERT_INIT_TM;
    (void)time(&now);

    LOCALTIME_LOCK;
    my_tm = localtime(&now);
    if (my_tm)
        Copy(my_tm, ptm, 1, struct tm);
    LOCALTIME_UNLOCK;
#else
    PERL_UNUSED_CONTEXT;
    PERL_ARGS_ASSERT_INIT_TM;
    PERL_UNUSED_ARG(ptm);
#endif
}

/*
=for apidoc_section $time
=for apidoc mini_mktime
normalise S<C<struct tm>> values without the localtime() semantics (and
overhead) of mktime().

=cut
 */
void
Perl_mini_mktime(struct tm *ptm)
{
    int yearday;
    int secs;
    int month, mday, year, jday;
    int odd_cent, odd_year;

    PERL_ARGS_ASSERT_MINI_MKTIME;

#define DAYS_PER_YEAR   365
#define DAYS_PER_QYEAR  (4*DAYS_PER_YEAR+1)
#define DAYS_PER_CENT   (25*DAYS_PER_QYEAR-1)
#define DAYS_PER_QCENT  (4*DAYS_PER_CENT+1)
#define SECS_PER_HOUR   (60*60)
#define SECS_PER_DAY    (24*SECS_PER_HOUR)
/* parentheses deliberately absent on these two, otherwise they don't work */
#define MONTH_TO_DAYS   153/5
#define DAYS_TO_MONTH   5/153
/* offset to bias by March (month 4) 1st between month/mday & year finding */
#define YEAR_ADJUST     (4*MONTH_TO_DAYS+1)
/* as used here, the algorithm leaves Sunday as day 1 unless we adjust it */
#define WEEKDAY_BIAS    6       /* (1+6)%7 makes Sunday 0 again */

/*
 * Year/day algorithm notes:
 *
 * With a suitable offset for numeric value of the month, one can find
 * an offset into the year by considering months to have 30.6 (153/5) days,
 * using integer arithmetic (i.e., with truncation).  To avoid too much
 * messing about with leap days, we consider January and February to be
 * the 13th and 14th month of the previous year.  After that transformation,
 * we need the month index we use to be high by 1 from 'normal human' usage,
 * so the month index values we use run from 4 through 15.
 *
 * Given that, and the rules for the Gregorian calendar (leap years are those
 * divisible by 4 unless also divisible by 100, when they must be divisible
 * by 400 instead), we can simply calculate the number of days since some
 * arbitrary 'beginning of time' by futzing with the (adjusted) year number,
 * the days we derive from our month index, and adding in the day of the
 * month.  The value used here is not adjusted for the actual origin which
 * it normally would use (1 January A.D. 1), since we're not exposing it.
 * We're only building the value so we can turn around and get the
 * normalised values for the year, month, day-of-month, and day-of-year.
 *
 * For going backward, we need to bias the value we're using so that we find
 * the right year value.  (Basically, we don't want the contribution of
 * March 1st to the number to apply while deriving the year).  Having done
 * that, we 'count up' the contribution to the year number by accounting for
 * full quadracenturies (400-year periods) with their extra leap days, plus
 * the contribution from full centuries (to avoid counting in the lost leap
 * days), plus the contribution from full quad-years (to count in the normal
 * leap days), plus the leftover contribution from any non-leap years.
 * At this point, if we were working with an actual leap day, we'll have 0
 * days left over.  This is also true for March 1st, however.  So, we have
 * to special-case that result, and (earlier) keep track of the 'odd'
 * century and year contributions.  If we got 4 extra centuries in a qcent,
 * or 4 extra years in a qyear, then it's a leap day and we call it 29 Feb.
 * Otherwise, we add back in the earlier bias we removed (the 123 from
 * figuring in March 1st), find the month index (integer division by 30.6),
 * and the remainder is the day-of-month.  We then have to convert back to
 * 'real' months (including fixing January and February from being 14/15 in
 * the previous year to being in the proper year).  After that, to get
 * tm_yday, we work with the normalised year and get a new yearday value for
 * January 1st, which we subtract from the yearday value we had earlier,
 * representing the date we've re-built.  This is done from January 1
 * because tm_yday is 0-origin.
 *
 * Since POSIX time routines are only guaranteed to work for times since the
 * UNIX epoch (00:00:00 1 Jan 1970 UTC), the fact that this algorithm
 * applies Gregorian calendar rules even to dates before the 16th century
 * doesn't bother me.  Besides, you'd need cultural context for a given
 * date to know whether it was Julian or Gregorian calendar, and that's
 * outside the scope for this routine.  Since we convert back based on the
 * same rules we used to build the yearday, you'll only get strange results
 * for input which needed normalising, or for the 'odd' century years which
 * were leap years in the Julian calendar but not in the Gregorian one.
 * I can live with that.
 *
 * This algorithm also fails to handle years before A.D. 1 gracefully, but
 * that's still outside the scope for POSIX time manipulation, so I don't
 * care.
 *
 * - lwall
 */

    year = 1900 + ptm->tm_year;
    month = ptm->tm_mon;
    mday = ptm->tm_mday;
    jday = 0;
    if (month >= 2)
        month+=2;
    else
        month+=14, year--;
    yearday = DAYS_PER_YEAR * year + year/4 - year/100 + year/400;
    yearday += month*MONTH_TO_DAYS + mday + jday;
    /*
     * Note that we don't know when leap-seconds were or will be,
     * so we have to trust the user if we get something which looks
     * like a sensible leap-second.  Wild values for seconds will
     * be rationalised, however.
     */
    if ((unsigned) ptm->tm_sec <= 60) {
        secs = 0;
    }
    else {
        secs = ptm->tm_sec;
        ptm->tm_sec = 0;
    }
    secs += 60 * ptm->tm_min;
    secs += SECS_PER_HOUR * ptm->tm_hour;
    if (secs < 0) {
        if (secs-(secs/SECS_PER_DAY*SECS_PER_DAY) < 0) {
            /* got negative remainder, but need positive time */
            /* back off an extra day to compensate */
            yearday += (secs/SECS_PER_DAY)-1;
            secs -= SECS_PER_DAY * (secs/SECS_PER_DAY - 1);
        }
        else {
            yearday += (secs/SECS_PER_DAY);
            secs -= SECS_PER_DAY * (secs/SECS_PER_DAY);
        }
    }
    else if (secs >= SECS_PER_DAY) {
        yearday += (secs/SECS_PER_DAY);
        secs %= SECS_PER_DAY;
    }
    ptm->tm_hour = secs/SECS_PER_HOUR;
    secs %= SECS_PER_HOUR;
    ptm->tm_min = secs/60;
    secs %= 60;
    ptm->tm_sec += secs;
    /* done with time of day effects */
    /*
     * The algorithm for yearday has (so far) left it high by 428.
     * To avoid mistaking a legitimate Feb 29 as Mar 1, we need to
     * bias it by 123 while trying to figure out what year it
     * really represents.  Even with this tweak, the reverse
     * translation fails for years before A.D. 0001.
     * It would still fail for Feb 29, but we catch that one below.
     */
    jday = yearday;	/* save for later fixup vis-a-vis Jan 1 */
    yearday -= YEAR_ADJUST;
    year = (yearday / DAYS_PER_QCENT) * 400;
    yearday %= DAYS_PER_QCENT;
    odd_cent = yearday / DAYS_PER_CENT;
    year += odd_cent * 100;
    yearday %= DAYS_PER_CENT;
    year += (yearday / DAYS_PER_QYEAR) * 4;
    yearday %= DAYS_PER_QYEAR;
    odd_year = yearday / DAYS_PER_YEAR;
    year += odd_year;
    yearday %= DAYS_PER_YEAR;
    if (!yearday && (odd_cent==4 || odd_year==4)) { /* catch Feb 29 */
        month = 1;
        yearday = 29;
    }
    else {
        yearday += YEAR_ADJUST;	/* recover March 1st crock */
        month = yearday*DAYS_TO_MONTH;
        yearday -= month*MONTH_TO_DAYS;
        /* recover other leap-year adjustment */
        if (month > 13) {
            month-=14;
            year++;
        }
        else {
            month-=2;
        }
    }
    ptm->tm_year = year - 1900;
    if (yearday) {
      ptm->tm_mday = yearday;
      ptm->tm_mon = month;
    }
    else {
      ptm->tm_mday = 31;
      ptm->tm_mon = month - 1;
    }
    /* re-build yearday based on Jan 1 to get tm_yday */
    year--;
    yearday = year*DAYS_PER_YEAR + year/4 - year/100 + year/400;
    yearday += 14*MONTH_TO_DAYS + 1;
    ptm->tm_yday = jday - yearday;
    ptm->tm_wday = (jday + WEEKDAY_BIAS) % 7;
}

#define SV_CWD_RETURN_UNDEF \
    sv_set_undef(sv); \
    return FALSE

#define SV_CWD_ISDOT(dp) \
    (dp->d_name[0] == '.' && (dp->d_name[1] == '\0' || \
        (dp->d_name[1] == '.' && dp->d_name[2] == '\0')))

/*
=for apidoc_section $utility

=for apidoc getcwd_sv

Fill C<sv> with current working directory

=cut
*/

/* Originally written in Perl by John Bazik; rewritten in C by Ben Sugars.
 * rewritten again by dougm, optimized for use with xs TARG, and to prefer
 * getcwd(3) if available
 * Comments from the original:
 *     This is a faster version of getcwd.  It's also more dangerous
 *     because you might chdir out of a directory that you can't chdir
 *     back into. */

int
Perl_getcwd_sv(pTHX_ SV *sv)
{
#ifndef PERL_MICRO
    SvTAINTED_on(sv);

    PERL_ARGS_ASSERT_GETCWD_SV;

#ifdef HAS_GETCWD
    {
        char buf[MAXPATHLEN];

        /* Some getcwd()s automatically allocate a buffer of the given
         * size from the heap if they are given a NULL buffer pointer.
         * The problem is that this behaviour is not portable. */
        if (getcwd(buf, sizeof(buf) - 1)) {
            sv_setpv(sv, buf);
            return TRUE;
        }
        else {
            SV_CWD_RETURN_UNDEF;
        }
    }

#else

    Stat_t statbuf;
    int orig_cdev, orig_cino, cdev, cino, odev, oino, tdev, tino;
    int pathlen=0;
    Direntry_t *dp;

    SvUPGRADE(sv, SVt_PV);

    if (PerlLIO_lstat(".", &statbuf) < 0) {
        SV_CWD_RETURN_UNDEF;
    }

    orig_cdev = statbuf.st_dev;
    orig_cino = statbuf.st_ino;
    cdev = orig_cdev;
    cino = orig_cino;

    for (;;) {
        DIR *dir;
        int namelen;
        odev = cdev;
        oino = cino;

        if (PerlDir_chdir("..") < 0) {
            SV_CWD_RETURN_UNDEF;
        }
        if (PerlLIO_stat(".", &statbuf) < 0) {
            SV_CWD_RETURN_UNDEF;
        }

        cdev = statbuf.st_dev;
        cino = statbuf.st_ino;

        if (odev == cdev && oino == cino) {
            break;
        }
        if (!(dir = PerlDir_open("."))) {
            SV_CWD_RETURN_UNDEF;
        }

        while ((dp = PerlDir_read(dir)) != NULL) {
#ifdef DIRNAMLEN
            namelen = dp->d_namlen;
#else
            namelen = strlen(dp->d_name);
#endif
            /* skip . and .. */
            if (SV_CWD_ISDOT(dp)) {
                continue;
            }

            if (PerlLIO_lstat(dp->d_name, &statbuf) < 0) {
                SV_CWD_RETURN_UNDEF;
            }

            tdev = statbuf.st_dev;
            tino = statbuf.st_ino;
            if (tino == oino && tdev == odev) {
                break;
            }
        }

        if (!dp) {
            SV_CWD_RETURN_UNDEF;
        }

        if (pathlen + namelen + 1 >= MAXPATHLEN) {
            SV_CWD_RETURN_UNDEF;
        }

        SvGROW(sv, pathlen + namelen + 1);

        if (pathlen) {
            /* shift down */
            Move(SvPVX_const(sv), SvPVX(sv) + namelen + 1, pathlen, char);
        }

        /* prepend current directory to the front */
        *SvPVX(sv) = '/';
        Move(dp->d_name, SvPVX(sv)+1, namelen, char);
        pathlen += (namelen + 1);

#ifdef VOID_CLOSEDIR
        PerlDir_close(dir);
#else
        if (PerlDir_close(dir) < 0) {
            SV_CWD_RETURN_UNDEF;
        }
#endif
    }

    if (pathlen) {
        SvCUR_set(sv, pathlen);
        *SvEND(sv) = '\0';
        SvPOK_only(sv);

        if (PerlDir_chdir(SvPVX_const(sv)) < 0) {
            SV_CWD_RETURN_UNDEF;
        }
    }
    if (PerlLIO_stat(".", &statbuf) < 0) {
        SV_CWD_RETURN_UNDEF;
    }

    cdev = statbuf.st_dev;
    cino = statbuf.st_ino;

    if (cdev != orig_cdev || cino != orig_cino) {
        Perl_croak(aTHX_ "Unstable directory path, "
                   "current directory changed unexpectedly");
    }

    return TRUE;
#endif

#else
    return FALSE;
#endif
}

#include "vutil.c"

#if !defined(HAS_SOCKETPAIR) && defined(HAS_SOCKET) && defined(AF_INET) && defined(PF_INET) && defined(SOCK_DGRAM) && defined(HAS_SELECT)
#   define EMULATE_SOCKETPAIR_UDP
#endif

#ifdef EMULATE_SOCKETPAIR_UDP
static int
S_socketpair_udp (int fd[2]) {
    dTHX;
    /* Fake a datagram socketpair using UDP to localhost.  */
    int sockets[2] = {-1, -1};
    struct sockaddr_in addresses[2];
    int i;
    Sock_size_t size = sizeof(struct sockaddr_in);
    unsigned short port;
    int got;

    memset(&addresses, 0, sizeof(addresses));
    i = 1;
    do {
        sockets[i] = PerlSock_socket(AF_INET, SOCK_DGRAM, PF_INET);
        if (sockets[i] == -1)
            goto tidy_up_and_fail;

        addresses[i].sin_family = AF_INET;
        addresses[i].sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        addresses[i].sin_port = 0;	/* kernel chooses port.  */
        if (PerlSock_bind(sockets[i], (struct sockaddr *) &addresses[i],
                sizeof(struct sockaddr_in)) == -1)
            goto tidy_up_and_fail;
    } while (i--);

    /* Now have 2 UDP sockets. Find out which port each is connected to, and
       for each connect the other socket to it.  */
    i = 1;
    do {
        if (PerlSock_getsockname(sockets[i], (struct sockaddr *) &addresses[i],
                &size) == -1)
            goto tidy_up_and_fail;
        if (size != sizeof(struct sockaddr_in))
            goto abort_tidy_up_and_fail;
        /* !1 is 0, !0 is 1 */
        if (PerlSock_connect(sockets[!i], (struct sockaddr *) &addresses[i],
                sizeof(struct sockaddr_in)) == -1)
            goto tidy_up_and_fail;
    } while (i--);

    /* Now we have 2 sockets connected to each other. I don't trust some other
       process not to have already sent a packet to us (by random) so send
       a packet from each to the other.  */
    i = 1;
    do {
        /* I'm going to send my own port number.  As a short.
           (Who knows if someone somewhere has sin_port as a bitfield and needs
           this routine. (I'm assuming crays have socketpair)) */
        port = addresses[i].sin_port;
        got = PerlLIO_write(sockets[i], &port, sizeof(port));
        if (got != sizeof(port)) {
            if (got == -1)
                goto tidy_up_and_fail;
            goto abort_tidy_up_and_fail;
        }
    } while (i--);

    /* Packets sent. I don't trust them to have arrived though.
       (As I understand it Solaris TCP stack is multithreaded. Non-blocking
       connect to localhost will use a second kernel thread. In 2.6 the
       first thread running the connect() returns before the second completes,
       so EINPROGRESS> In 2.7 the improved stack is faster and connect()
       returns 0. Poor programs have tripped up. One poor program's authors'
       had a 50-1 reverse stock split. Not sure how connected these were.)
       So I don't trust someone not to have an unpredictable UDP stack.
    */

    {
        struct timeval waitfor = {0, 100000}; /* You have 0.1 seconds */
        int max = sockets[1] > sockets[0] ? sockets[1] : sockets[0];
        fd_set rset;

        FD_ZERO(&rset);
        FD_SET((unsigned int)sockets[0], &rset);
        FD_SET((unsigned int)sockets[1], &rset);

        got = PerlSock_select(max + 1, &rset, NULL, NULL, &waitfor);
        if (got != 2 || !FD_ISSET(sockets[0], &rset)
                || !FD_ISSET(sockets[1], &rset)) {
            /* I hope this is portable and appropriate.  */
            if (got == -1)
                goto tidy_up_and_fail;
            goto abort_tidy_up_and_fail;
        }
    }

    /* And the paranoia department even now doesn't trust it to have arrive
       (hence MSG_DONTWAIT). Or that what arrives was sent by us.  */
    {
        struct sockaddr_in readfrom;
        unsigned short buffer[2];

        i = 1;
        do {
#ifdef MSG_DONTWAIT
            got = PerlSock_recvfrom(sockets[i], (char *) &buffer,
                    sizeof(buffer), MSG_DONTWAIT,
                    (struct sockaddr *) &readfrom, &size);
#else
            got = PerlSock_recvfrom(sockets[i], (char *) &buffer,
                    sizeof(buffer), 0,
                    (struct sockaddr *) &readfrom, &size);
#endif

            if (got == -1)
                goto tidy_up_and_fail;
            if (got != sizeof(port)
                    || size != sizeof(struct sockaddr_in)
                    /* Check other socket sent us its port.  */
                    || buffer[0] != (unsigned short) addresses[!i].sin_port
                    /* Check kernel says we got the datagram from that socket */
                    || readfrom.sin_family != addresses[!i].sin_family
                    || readfrom.sin_addr.s_addr != addresses[!i].sin_addr.s_addr
                    || readfrom.sin_port != addresses[!i].sin_port)
                goto abort_tidy_up_and_fail;
        } while (i--);
    }
    /* My caller (my_socketpair) has validated that this is non-NULL  */
    fd[0] = sockets[0];
    fd[1] = sockets[1];
    /* I hereby declare this connection open.  May God bless all who cross
       her.  */
    return 0;

  abort_tidy_up_and_fail:
    errno = ECONNABORTED;
  tidy_up_and_fail:
    {
        dSAVE_ERRNO;
        if (sockets[0] != -1)
            PerlLIO_close(sockets[0]);
        if (sockets[1] != -1)
            PerlLIO_close(sockets[1]);
        RESTORE_ERRNO;
        return -1;
    }
}
#endif /*  EMULATE_SOCKETPAIR_UDP */

#if !defined(HAS_SOCKETPAIR) && defined(HAS_SOCKET) && defined(AF_INET) && defined(PF_INET)

/*
=for apidoc my_socketpair

Emulates L<socketpair(2)> on systems that don't have it, but which do have
enough functionality for the emulation.

=cut
*/

int
Perl_my_socketpair (int family, int type, int protocol, int fd[2]) {
    /* Stevens says that family must be AF_LOCAL, protocol 0.
       I'm going to enforce that, then ignore it, and use TCP (or UDP).  */
    dTHXa(NULL);
    int listener = -1;
    int connector = -1;
    int acceptor = -1;
    struct sockaddr_in listen_addr;
    struct sockaddr_in connect_addr;
    Sock_size_t size;

    if (protocol
#ifdef AF_UNIX
        || family != AF_UNIX
#endif
    ) {
        errno = EAFNOSUPPORT;
        return -1;
    }
    if (!fd) {
        errno = EINVAL;
        return -1;
    }

#ifdef SOCK_CLOEXEC
    type &= ~SOCK_CLOEXEC;
#endif

#ifdef EMULATE_SOCKETPAIR_UDP
    if (type == SOCK_DGRAM)
        return S_socketpair_udp(fd);
#endif

    aTHXa(PERL_GET_THX);
    listener = PerlSock_socket(AF_INET, type, 0);
    if (listener == -1)
        return -1;
    memset(&listen_addr, 0, sizeof(listen_addr));
    listen_addr.sin_family = AF_INET;
    listen_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    listen_addr.sin_port = 0;	/* kernel chooses port.  */
    if (PerlSock_bind(listener, (struct sockaddr *) &listen_addr,
            sizeof(listen_addr)) == -1)
        goto tidy_up_and_fail;
    if (PerlSock_listen(listener, 1) == -1)
        goto tidy_up_and_fail;

    connector = PerlSock_socket(AF_INET, type, 0);
    if (connector == -1)
        goto tidy_up_and_fail;
    /* We want to find out the port number to connect to.  */
    size = sizeof(connect_addr);
    if (PerlSock_getsockname(listener, (struct sockaddr *) &connect_addr,
            &size) == -1)
        goto tidy_up_and_fail;
    if (size != sizeof(connect_addr))
        goto abort_tidy_up_and_fail;
    if (PerlSock_connect(connector, (struct sockaddr *) &connect_addr,
            sizeof(connect_addr)) == -1)
        goto tidy_up_and_fail;

    size = sizeof(listen_addr);
    acceptor = PerlSock_accept(listener, (struct sockaddr *) &listen_addr,
            &size);
    if (acceptor == -1)
        goto tidy_up_and_fail;
    if (size != sizeof(listen_addr))
        goto abort_tidy_up_and_fail;
    PerlLIO_close(listener);
    /* Now check we are talking to ourself by matching port and host on the
       two sockets.  */
    if (PerlSock_getsockname(connector, (struct sockaddr *) &connect_addr,
            &size) == -1)
        goto tidy_up_and_fail;
    if (size != sizeof(connect_addr)
            || listen_addr.sin_family != connect_addr.sin_family
            || listen_addr.sin_addr.s_addr != connect_addr.sin_addr.s_addr
            || listen_addr.sin_port != connect_addr.sin_port) {
        goto abort_tidy_up_and_fail;
    }
    fd[0] = connector;
    fd[1] = acceptor;
    return 0;

  abort_tidy_up_and_fail:
#ifdef ECONNABORTED
  errno = ECONNABORTED;	/* This would be the standard thing to do. */
#elif defined(ECONNREFUSED)
  errno = ECONNREFUSED;	/* some OSes might not have ECONNABORTED. */
#else
  errno = ETIMEDOUT;	/* Desperation time. */
#endif
  tidy_up_and_fail:
    {
        dSAVE_ERRNO;
        if (listener != -1)
            PerlLIO_close(listener);
        if (connector != -1)
            PerlLIO_close(connector);
        if (acceptor != -1)
            PerlLIO_close(acceptor);
        RESTORE_ERRNO;
        return -1;
    }
}
#else
/* In any case have a stub so that there's code corresponding
 * to the my_socketpair in embed.fnc. */
int
Perl_my_socketpair (int family, int type, int protocol, int fd[2]) {
#ifdef HAS_SOCKETPAIR
    return socketpair(family, type, protocol, fd);
#else
    return -1;
#endif
}
#endif

/*

=for apidoc sv_nosharing

Dummy routine which "shares" an SV when there is no sharing module present.
Or "locks" it.  Or "unlocks" it.  In other
words, ignores its single SV argument.
Exists to avoid test for a C<NULL> function pointer and because it could
potentially warn under some level of strict-ness.

=cut
*/

void
Perl_sv_nosharing(pTHX_ SV *sv)
{
    PERL_UNUSED_CONTEXT;
    PERL_UNUSED_ARG(sv);
}

/*

=for apidoc sv_destroyable

Dummy routine which reports that object can be destroyed when there is no
sharing module present.  It ignores its single SV argument, and returns
'true'.  Exists to avoid test for a C<NULL> function pointer and because it
could potentially warn under some level of strict-ness.

=cut
*/

bool
Perl_sv_destroyable(pTHX_ SV *sv)
{
    PERL_UNUSED_CONTEXT;
    PERL_UNUSED_ARG(sv);
    return TRUE;
}

U32
Perl_parse_unicode_opts(pTHX_ const char **popt)
{
  const char *p = *popt;
  U32 opt = 0;

  PERL_ARGS_ASSERT_PARSE_UNICODE_OPTS;

  if (*p) {
       if (isDIGIT(*p)) {
            const char* endptr = p + strlen(p);
            UV uv;
            if (grok_atoUV(p, &uv, &endptr) && uv <= U32_MAX) {
                opt = (U32)uv;
                p = endptr;
                if (p && *p && *p != '\n' && *p != '\r') {
                    if (isSPACE(*p))
                        goto the_end_of_the_opts_parser;
                    else
                        Perl_croak(aTHX_ "Unknown Unicode option letter '%c'", *p);
                }
            }
            else {
                Perl_croak(aTHX_ "Invalid number '%s' for -C option.\n", p);
            }
        }
        else {
            for (; *p; p++) {
                 switch (*p) {
                 case PERL_UNICODE_STDIN:
                      opt |= PERL_UNICODE_STDIN_FLAG;	break;
                 case PERL_UNICODE_STDOUT:
                      opt |= PERL_UNICODE_STDOUT_FLAG;	break;
                 case PERL_UNICODE_STDERR:
                      opt |= PERL_UNICODE_STDERR_FLAG;	break;
                 case PERL_UNICODE_STD:
                      opt |= PERL_UNICODE_STD_FLAG;    	break;
                 case PERL_UNICODE_IN:
                      opt |= PERL_UNICODE_IN_FLAG;	break;
                 case PERL_UNICODE_OUT:
                      opt |= PERL_UNICODE_OUT_FLAG;	break;
                 case PERL_UNICODE_INOUT:
                      opt |= PERL_UNICODE_INOUT_FLAG;	break;
                 case PERL_UNICODE_LOCALE:
                      opt |= PERL_UNICODE_LOCALE_FLAG;	break;
                 case PERL_UNICODE_ARGV:
                      opt |= PERL_UNICODE_ARGV_FLAG;	break;
                 case PERL_UNICODE_UTF8CACHEASSERT:
                      opt |= PERL_UNICODE_UTF8CACHEASSERT_FLAG; break;
                 default:
                      if (*p != '\n' && *p != '\r') {
                        if(isSPACE(*p)) goto the_end_of_the_opts_parser;
                        else
                          Perl_croak(aTHX_
                                     "Unknown Unicode option letter '%c'", *p);
                      }
                 }
            }
       }
  }
  else
       opt = PERL_UNICODE_DEFAULT_FLAGS;

  the_end_of_the_opts_parser:

  if (opt & ~PERL_UNICODE_ALL_FLAGS)
       Perl_croak(aTHX_ "Unknown Unicode option value %" UVuf,
                  (UV) (opt & ~PERL_UNICODE_ALL_FLAGS));

  *popt = p;

  return opt;
}

#ifdef VMS
#  include <starlet.h>
#endif

/* hash a pointer and return a U32
 *
 * this code was derived from Sereal, which was derived from autobox.
 */

PERL_STATIC_INLINE U32 S_ptr_hash(PTRV u) {
#if PTRSIZE == 8
    /*
     * This is one of Thomas Wang's hash functions for 64-bit integers from:
     * http://www.concentric.net/~Ttwang/tech/inthash.htm
     */
    u = (~u) + (u << 18);
    u = u ^ (u >> 31);
    u = u * 21;
    u = u ^ (u >> 11);
    u = u + (u << 6);
    u = u ^ (u >> 22);
#else
    /*
     * This is one of Bob Jenkins' hash functions for 32-bit integers
     * from: http://burtleburtle.net/bob/hash/integer.html
     */
    u = (u + 0x7ed55d16) + (u << 12);
    u = (u ^ 0xc761c23c) ^ (u >> 19);
    u = (u + 0x165667b1) + (u << 5);
    u = (u + 0xd3a2646c) ^ (u << 9);
    u = (u + 0xfd7046c5) + (u << 3);
    u = (u ^ 0xb55a4f09) ^ (u >> 16);
#endif
    return (U32)u;
}


U32
Perl_seed(pTHX)
{
    /*
     * This is really just a quick hack which grabs various garbage
     * values.  It really should be a real hash algorithm which
     * spreads the effect of every input bit onto every output bit,
     * if someone who knows about such things would bother to write it.
     * Might be a good idea to add that function to CORE as well.
     * No numbers below come from careful analysis or anything here,
     * except they are primes and SEED_C1 > 1E6 to get a full-width
     * value from (tv_sec * SEED_C1 + tv_usec).  The multipliers should
     * probably be bigger too.
     */
#if RANDBITS > 16
#  define SEED_C1	1000003
#define   SEED_C4	73819
#else
#  define SEED_C1	25747
#define   SEED_C4	20639
#endif
#define   SEED_C2	3
#define   SEED_C3	269
#define   SEED_C5	26107

#ifndef PERL_NO_DEV_RANDOM
    int fd;
#endif
    U32 u;
#ifdef HAS_GETTIMEOFDAY
    struct timeval when;
#else
    Time_t when;
#endif

/* This test is an escape hatch, this symbol isn't set by Configure. */
#ifndef PERL_NO_DEV_RANDOM
#ifndef PERL_RANDOM_DEVICE
   /* /dev/random isn't used by default because reads from it will block
    * if there isn't enough entropy available.  You can compile with
    * PERL_RANDOM_DEVICE to it if you'd prefer Perl to block until there
    * is enough real entropy to fill the seed. */
#  ifdef __amigaos4__
#    define PERL_RANDOM_DEVICE "RANDOM:SIZE=4"
#  else
#    define PERL_RANDOM_DEVICE "/dev/urandom"
#  endif
#endif
    fd = PerlLIO_open_cloexec(PERL_RANDOM_DEVICE, 0);
    if (fd != -1) {
        if (PerlLIO_read(fd, (void*)&u, sizeof u) != sizeof u)
            u = 0;
        PerlLIO_close(fd);
        if (u)
            return u;
    }
#endif

#ifdef HAS_GETTIMEOFDAY
    PerlProc_gettimeofday(&when,NULL);
    u = (U32)SEED_C1 * when.tv_sec + (U32)SEED_C2 * when.tv_usec;
#else
    (void)time(&when);
    u = (U32)SEED_C1 * when;
#endif
    u += SEED_C3 * (U32)PerlProc_getpid();
    u += SEED_C4 * (U32)PTR2UV(PL_stack_sp);
#ifndef PLAN9           /* XXX Plan9 assembler chokes on this; fix needed  */
    UV ptruv = PTR2UV(&when);
    u += SEED_C5 * ptr_hash(ptruv);
#endif
    return u;
}

void
Perl_get_hash_seed(pTHX_ unsigned char * const seed_buffer)
{
#ifndef NO_PERL_HASH_ENV
    const char *env_pv;
#endif
    unsigned long i;

    PERL_ARGS_ASSERT_GET_HASH_SEED;

    Zero(seed_buffer, PERL_HASH_SEED_BYTES, U8);
    Zero((U8*)PL_hash_state_w, PERL_HASH_STATE_BYTES, U8);

#ifndef NO_PERL_HASH_ENV
    env_pv= PerlEnv_getenv("PERL_HASH_SEED");

    if ( env_pv )
    {
        if (DEBUG_h_TEST)
            PerlIO_printf(Perl_debug_log,"Got PERL_HASH_SEED=<%s>\n", env_pv);
        /* ignore leading spaces */
        while (isSPACE(*env_pv))
            env_pv++;
#    ifdef USE_PERL_PERTURB_KEYS
        /* if they set it to "0" we disable key traversal randomization completely */
        if (strEQ(env_pv,"0")) {
            PL_hash_rand_bits_enabled= 0;
        } else {
            /* otherwise switch to deterministic mode */
            PL_hash_rand_bits_enabled= 2;
        }
#    endif
        /* ignore a leading 0x... if it is there */
        if (env_pv[0] == '0' && env_pv[1] == 'x')
            env_pv += 2;

        for( i = 0; isXDIGIT(*env_pv) && i < PERL_HASH_SEED_BYTES; i++ ) {
            seed_buffer[i] = READ_XDIGIT(env_pv) << 4;
            if ( isXDIGIT(*env_pv)) {
                seed_buffer[i] |= READ_XDIGIT(env_pv);
            }
        }
        while (isSPACE(*env_pv))
            env_pv++;

        if (*env_pv && !isXDIGIT(*env_pv)) {
            Perl_warn(aTHX_ "perl: warning: Non hex character in '$ENV{PERL_HASH_SEED}', seed only partially set\n");
        }
        /* should we check for unparsed crap? */
        /* should we warn about unused hex? */
        /* should we warn about insufficient hex? */
    }
    else
#endif /* NO_PERL_HASH_ENV */
    {
        for( i = 0; i < PERL_HASH_SEED_BYTES; i++ ) {
            seed_buffer[i] = (unsigned char)(Perl_internal_drand48() * (U8_MAX+1));
        }
    }
#ifdef USE_PERL_PERTURB_KEYS
#  ifndef NO_PERL_HASH_ENV
    env_pv= PerlEnv_getenv("PERL_PERTURB_KEYS");
    if (env_pv) {
        if (DEBUG_h_TEST)
            PerlIO_printf(Perl_debug_log,
                "Got PERL_PERTURB_KEYS=<%s>\n", env_pv);
        if (strEQ(env_pv,"0") || strEQ(env_pv,"NO")) {
            PL_hash_rand_bits_enabled= 0;
        } else if (strEQ(env_pv,"1") || strEQ(env_pv,"RANDOM")) {
            PL_hash_rand_bits_enabled= 1;
        } else if (strEQ(env_pv,"2") || strEQ(env_pv,"DETERMINISTIC")) {
            PL_hash_rand_bits_enabled= 2;
        } else {
            Perl_warn(aTHX_ "perl: warning: strange setting in '$ENV{PERL_PERTURB_KEYS}': '%s'\n", env_pv);
        }
    }
#  endif
    {   /* initialize PL_hash_rand_bits from the hash seed.
         * This value is highly volatile, it is updated every
         * hash insert, and is used as part of hash bucket chain
         * randomization and hash iterator randomization. */
        if (PL_hash_rand_bits_enabled == 1) {
            /* random mode initialize from seed() like we would our RNG() */
            PL_hash_rand_bits= seed();
        }
        else {
            /* Use a constant */
            PL_hash_rand_bits= 0xbe49d17f; /* I just picked a number */
            /* and then mix in the leading bytes of the hash seed */
            for( i = 0; i < sizeof(UV) ; i++ ) {
                PL_hash_rand_bits ^= seed_buffer[i % PERL_HASH_SEED_BYTES];
                PL_hash_rand_bits = ROTL_UV(PL_hash_rand_bits,8);
            }
        }
        if (!PL_hash_rand_bits) {
            /* we use an XORSHIFT RNG to munge PL_hash_rand_bits,
             * which means it cannot be 0 or it will stay 0 for the
             * lifetime of the process, so if by some insane chance we
             * ended up with a 0 after the above initialization
             * then set it to this. This really should not happen, or
             * very very very rarely.
             */
            PL_hash_rand_bits = 0x8110ba9d; /* a randomly chosen prime */
        }
    }
#endif
}

void
Perl_debug_hash_seed(pTHX_ bool via_debug_h)
{
    PERL_ARGS_ASSERT_DEBUG_HASH_SEED;
#if (defined(USE_HASH_SEED) || defined(USE_HASH_SEED_DEBUG)) && !defined(NO_PERL_HASH_SEED_DEBUG)
    {
        const char * const s = PerlEnv_getenv("PERL_HASH_SEED_DEBUG");
        bool via_env = cBOOL(s && strNE(s, "0") && strNE(s,""));

        if ( via_env != via_debug_h ) {
            const unsigned char *seed= PERL_HASH_SEED;
            const unsigned char *seed_end= PERL_HASH_SEED + PERL_HASH_SEED_BYTES;
            PerlIO_printf(Perl_debug_log, "HASH_FUNCTION = %s HASH_SEED = 0x", PERL_HASH_FUNC);
            while (seed < seed_end) {
                PerlIO_printf(Perl_debug_log, "%02x", *seed++);
            }
#ifdef PERL_HASH_RANDOMIZE_KEYS
            PerlIO_printf(Perl_debug_log, " PERTURB_KEYS = %d (%s)",
                    PL_HASH_RAND_BITS_ENABLED,
                    PL_HASH_RAND_BITS_ENABLED == 0 ? "NO" :
                    PL_HASH_RAND_BITS_ENABLED == 1 ? "RANDOM"
                                                   : "DETERMINISTIC");
            if (DEBUG_h_TEST)
                PerlIO_printf(Perl_debug_log,
                        " RAND_BITS=0x%" UVxf, PL_hash_rand_bits);
#endif
            PerlIO_printf(Perl_debug_log, "\n");
        }
    }
#endif /* #if (defined(USE_HASH_SEED) ... */
}




#ifdef PERL_MEM_LOG

/* -DPERL_MEM_LOG: the Perl_mem_log_..() is compiled, including
 * the default implementation, unless -DPERL_MEM_LOG_NOIMPL is also
 * given, and you supply your own implementation.
 *
 * The default implementation reads a single env var, PERL_MEM_LOG,
 * expecting one or more of the following:
 *
 *    \d+ - fd		fd to write to		: must be 1st (grok_atoUV)
 *    'm' - memlog	was PERL_MEM_LOG=1
 *    's' - svlog	was PERL_SV_LOG=1
 *    't' - timestamp	was PERL_MEM_LOG_TIMESTAMP=1
 *
 * This makes the logger controllable enough that it can reasonably be
 * added to the system perl.
 */

/* -DPERL_MEM_LOG_SPRINTF_BUF_SIZE=X: size of a (stack-allocated) buffer
 * the Perl_mem_log_...() will use (either via sprintf or snprintf).
 */
#define PERL_MEM_LOG_SPRINTF_BUF_SIZE 256

/* -DPERL_MEM_LOG_FD=N: the file descriptor the Perl_mem_log_...()
 * writes to.  In the default logger, this is settable at runtime.
 */
#ifndef PERL_MEM_LOG_FD
#  define PERL_MEM_LOG_FD 2 /* If STDERR is too boring for you. */
#endif

#ifndef PERL_MEM_LOG_NOIMPL

# ifdef DEBUG_LEAKING_SCALARS
#   define SV_LOG_SERIAL_FMT	    " [%lu]"
#   define _SV_LOG_SERIAL_ARG(sv)   , (unsigned long) (sv)->sv_debug_serial
# else
#   define SV_LOG_SERIAL_FMT
#   define _SV_LOG_SERIAL_ARG(sv)
# endif

static void
S_mem_log_common(enum mem_log_type mlt, const UV n, 
                 const UV typesize, const char *type_name, const SV *sv,
                 Malloc_t oldalloc, Malloc_t newalloc,
                 const char *filename, const int linenumber,
                 const char *funcname)
{
    const char *pmlenv;
    dTHX;

    PERL_ARGS_ASSERT_MEM_LOG_COMMON;

    PL_mem_log[0] |= 0x2;   /* Flag that the call is from this code */
    pmlenv = PerlEnv_getenv("PERL_MEM_LOG");
    PL_mem_log[0] &= ~0x2;
    if (!pmlenv)
        return;
    if (mlt < MLT_NEW_SV ? strchr(pmlenv,'m') : strchr(pmlenv,'s'))
    {
        /* We can't use SVs or PerlIO for obvious reasons,
         * so we'll use stdio and low-level IO instead. */
        char buf[PERL_MEM_LOG_SPRINTF_BUF_SIZE];

#   ifdef HAS_GETTIMEOFDAY
#     define MEM_LOG_TIME_FMT	"%10d.%06d: "
#     define MEM_LOG_TIME_ARG	(int)tv.tv_sec, (int)tv.tv_usec
        struct timeval tv;
        PerlProc_gettimeofday(&tv, 0);
#   else
#     define MEM_LOG_TIME_FMT	"%10d: "
#     define MEM_LOG_TIME_ARG	(int)when
        Time_t when;
        (void)time(&when);
#   endif
        /* If there are other OS specific ways of hires time than
         * gettimeofday() (see dist/Time-HiRes), the easiest way is
         * probably that they would be used to fill in the struct
         * timeval. */
        {
            STRLEN len;
            const char* endptr = pmlenv + strlen(pmlenv);
            int fd;
            UV uv;
            if (grok_atoUV(pmlenv, &uv, &endptr) /* Ignore endptr. */
                && uv && uv <= PERL_INT_MAX
            ) {
                fd = (int)uv;
            } else {
                fd = PERL_MEM_LOG_FD;
            }

            if (strchr(pmlenv, 't')) {
                len = my_snprintf(buf, sizeof(buf),
                                MEM_LOG_TIME_FMT, MEM_LOG_TIME_ARG);
                PERL_UNUSED_RESULT(PerlLIO_write(fd, buf, len));
            }
            switch (mlt) {
            case MLT_ALLOC:
                len = my_snprintf(buf, sizeof(buf),
                        "alloc: %s:%d:%s: %" IVdf " %" UVuf
                        " %s = %" IVdf ": %" UVxf "\n",
                        filename, linenumber, funcname, n, typesize,
                        type_name, n * typesize, PTR2UV(newalloc));
                break;
            case MLT_REALLOC:
                len = my_snprintf(buf, sizeof(buf),
                        "realloc: %s:%d:%s: %" IVdf " %" UVuf
                        " %s = %" IVdf ": %" UVxf " -> %" UVxf "\n",
                        filename, linenumber, funcname, n, typesize,
                        type_name, n * typesize, PTR2UV(oldalloc),
                        PTR2UV(newalloc));
                break;
            case MLT_FREE:
                len = my_snprintf(buf, sizeof(buf),
                        "free: %s:%d:%s: %" UVxf "\n",
                        filename, linenumber, funcname,
                        PTR2UV(oldalloc));
                break;
            case MLT_NEW_SV:
            case MLT_DEL_SV:
                len = my_snprintf(buf, sizeof(buf),
                        "%s_SV: %s:%d:%s: %" UVxf SV_LOG_SERIAL_FMT "\n",
                        mlt == MLT_NEW_SV ? "new" : "del",
                        filename, linenumber, funcname,
                        PTR2UV(sv) _SV_LOG_SERIAL_ARG(sv));
                break;
            default:
                len = 0;
            }
            PERL_UNUSED_RESULT(PerlLIO_write(fd, buf, len));
#ifdef USE_C_BACKTRACE
            if(strchr(pmlenv,'c') && (mlt == MLT_NEW_SV)) {
                len = my_snprintf(buf, sizeof(buf),
                        "  caller %s at %s line %" LINE_Tf "\n",
                        /* CopSTASHPV can crash early on startup; use CopFILE to check */
                        CopFILE(PL_curcop) ? CopSTASHPV(PL_curcop) : "<unknown>",
                        CopFILE(PL_curcop), CopLINE(PL_curcop));
                PERL_UNUSED_RESULT(PerlLIO_write(fd, buf, len));

                Perl_c_backtrace *bt = Perl_get_c_backtrace(aTHX_ 3, 3);
                Perl_c_backtrace_frame *frame;
                UV i;
                for (i = 0, frame = bt->frame_info;
                        i < bt->header.frame_count;
                        i++, frame++) {
                    len = my_snprintf(buf, sizeof(buf),
                            "  frame[%" UVuf "]: %p %s at %s +0x%lx\n",
                            i,
                            frame->addr,
                            frame->symbol_name_size && frame->symbol_name_offset ? (char *)bt + frame->symbol_name_offset : "-",
                            frame->object_name_size && frame->object_name_offset ? (char *)bt + frame->object_name_offset : "?",
                            (char *)frame->addr - (char *)frame->object_base_addr);
                    PERL_UNUSED_RESULT(PerlLIO_write(fd, buf, len));
                }
                Perl_free_c_backtrace(bt);
            }
#endif /* USE_C_BACKTRACE */
        }
    }
}
#endif /* !PERL_MEM_LOG_NOIMPL */

#ifndef PERL_MEM_LOG_NOIMPL
# define \
    mem_log_common_if(alty, num, tysz, tynm, sv, oal, nal, flnm, ln, fnnm) \
    mem_log_common   (alty, num, tysz, tynm, sv, oal, nal, flnm, ln, fnnm)
#else
/* this is suboptimal, but bug compatible.  User is providing their
   own implementation, but is getting these functions anyway, and they
   do nothing. But _NOIMPL users should be able to cope or fix */
# define \
    mem_log_common_if(alty, num, tysz, tynm, u, oal, nal, flnm, ln, fnnm) \
    /* mem_log_common_if_PERL_MEM_LOG_NOIMPL */
#endif

Malloc_t
Perl_mem_log_alloc(const UV n, const UV typesize, const char *type_name,
                   Malloc_t newalloc, 
                   const char *filename, const int linenumber,
                   const char *funcname)
{
    PERL_ARGS_ASSERT_MEM_LOG_ALLOC;

    mem_log_common_if(MLT_ALLOC, n, typesize, type_name,
                      NULL, NULL, newalloc,
                      filename, linenumber, funcname);
    return newalloc;
}

Malloc_t
Perl_mem_log_realloc(const UV n, const UV typesize, const char *type_name,
                     Malloc_t oldalloc, Malloc_t newalloc, 
                     const char *filename, const int linenumber, 
                     const char *funcname)
{
    PERL_ARGS_ASSERT_MEM_LOG_REALLOC;

    mem_log_common_if(MLT_REALLOC, n, typesize, type_name,
                      NULL, oldalloc, newalloc, 
                      filename, linenumber, funcname);
    return newalloc;
}

Malloc_t
Perl_mem_log_free(Malloc_t oldalloc, 
                  const char *filename, const int linenumber, 
                  const char *funcname)
{
    PERL_ARGS_ASSERT_MEM_LOG_FREE;

    mem_log_common_if(MLT_FREE, 0, 0, "", NULL, oldalloc, NULL, 
                      filename, linenumber, funcname);
    return oldalloc;
}

void
Perl_mem_log_new_sv(const SV *sv, 
                    const char *filename, const int linenumber,
                    const char *funcname)
{
    PERL_ARGS_ASSERT_MEM_LOG_NEW_SV;

    mem_log_common_if(MLT_NEW_SV, 0, 0, "", sv, NULL, NULL,
                      filename, linenumber, funcname);
}

void
Perl_mem_log_del_sv(const SV *sv,
                    const char *filename, const int linenumber, 
                    const char *funcname)
{
    PERL_ARGS_ASSERT_MEM_LOG_DEL_SV;

    mem_log_common_if(MLT_DEL_SV, 0, 0, "", sv, NULL, NULL, 
                      filename, linenumber, funcname);
}

#endif /* PERL_MEM_LOG */

/*
=for apidoc_section $string
=for apidoc quadmath_format_valid

C<quadmath_snprintf()> is very strict about its C<format> string and will
fail, returning -1, if the format is invalid.  It accepts exactly
one format spec.

C<quadmath_format_valid()> checks that the intended single spec looks
sane: begins with C<%>, has only one C<%>, ends with C<[efgaEFGA]>,
and has C<Q> before it.  This is not a full "printf syntax check",
just the basics.

Returns true if it is valid, false if not.

See also L</quadmath_format_needed>.

=cut
*/
#ifdef USE_QUADMATH
bool
Perl_quadmath_format_valid(const char* format)
{
    STRLEN len;

    PERL_ARGS_ASSERT_QUADMATH_FORMAT_VALID;

    if (format[0] != '%' || strchr(format + 1, '%'))
        return FALSE;
    len = strlen(format);
    /* minimum length three: %Qg */
    if (len < 3 || memCHRs("efgaEFGA", format[len - 1]) == NULL)
        return FALSE;
    if (format[len - 2] != 'Q')
        return FALSE;
    return TRUE;
}
#endif

/*
=for apidoc quadmath_format_needed

C<quadmath_format_needed()> returns true if the C<format> string seems to
contain at least one non-Q-prefixed C<%[efgaEFGA]> format specifier,
or returns false otherwise.

The format specifier detection is not complete printf-syntax detection,
but it should catch most common cases.

If true is returned, those arguments B<should> in theory be processed
with C<quadmath_snprintf()>, but in case there is more than one such
format specifier (see L</quadmath_format_valid>), and if there is
anything else beyond that one (even just a single byte), they
B<cannot> be processed because C<quadmath_snprintf()> is very strict,
accepting only one format spec, and nothing else.
In this case, the code should probably fail.

=cut
*/
#ifdef USE_QUADMATH
bool
Perl_quadmath_format_needed(const char* format)
{
  const char *p = format;
  const char *q;

  PERL_ARGS_ASSERT_QUADMATH_FORMAT_NEEDED;

  while ((q = strchr(p, '%'))) {
    q++;
    if (*q == '+') /* plus */
      q++;
    if (*q == '#') /* alt */
      q++;
    if (*q == '*') /* width */
      q++;
    else {
      if (isDIGIT(*q)) {
        while (isDIGIT(*q)) q++;
      }
    }
    if (*q == '.' && (q[1] == '*' || isDIGIT(q[1]))) { /* prec */
      q++;
      if (*q == '*')
        q++;
      else
        while (isDIGIT(*q)) q++;
    }
    if (memCHRs("efgaEFGA", *q)) /* Would have needed 'Q' in front. */
      return TRUE;
    p = q + 1;
  }
  return FALSE;
}
#endif

/*
=for apidoc my_snprintf

The C library C<snprintf> functionality, if available and
standards-compliant (uses C<vsnprintf>, actually).  However, if the
C<vsnprintf> is not available, will unfortunately use the unsafe
C<vsprintf> which can overrun the buffer (there is an overrun check,
but that may be too late).  Consider using C<sv_vcatpvf> instead, or
getting C<vsnprintf>.

=cut
*/

int
Perl_my_snprintf(char *buffer, const Size_t len, const char *format, ...)
{
    int retval = -1;
    va_list ap;
    dTHX;

    PERL_ARGS_ASSERT_MY_SNPRINTF;
#ifndef HAS_VSNPRINTF
    PERL_UNUSED_VAR(len);
#endif
    va_start(ap, format);
#ifdef USE_QUADMATH
    {
        bool quadmath_valid = FALSE;

        if (quadmath_format_valid(format)) {
            /* If the format looked promising, use it as quadmath. */
            WITH_LC_NUMERIC_SET_TO_NEEDED(
                retval = quadmath_snprintf(buffer, len, format, va_arg(ap, NV));
            );
            if (retval == -1) {
                Perl_croak_nocontext("panic: quadmath_snprintf failed, format \"%s\"", format);
            }
            quadmath_valid = TRUE;
        }
        /* quadmath_format_single() will return false for example for
         * "foo = %g", or simply "%g".  We could handle the %g by
         * using quadmath for the NV args.  More complex cases of
         * course exist: "foo = %g, bar = %g", or "foo=%Qg" (otherwise
         * quadmath-valid but has stuff in front).
         *
         * Handling the "Q-less" cases right would require walking
         * through the va_list and rewriting the format, calling
         * quadmath for the NVs, building a new va_list, and then
         * letting vsnprintf/vsprintf to take care of the other
         * arguments.  This may be doable.
         *
         * We do not attempt that now.  But for paranoia, we here try
         * to detect some common (but not all) cases where the
         * "Q-less" %[efgaEFGA] formats are present, and die if
         * detected.  This doesn't fix the problem, but it stops the
         * vsnprintf/vsprintf pulling doubles off the va_list when
         * __float128 NVs should be pulled off instead.
         *
         * If quadmath_format_needed() returns false, we are reasonably
         * certain that we can call vnsprintf() or vsprintf() safely. */
        if (!quadmath_valid && quadmath_format_needed(format))
          Perl_croak_nocontext("panic: quadmath_snprintf failed, format \"%s\"", format);

    }
#endif
    if (retval == -1) {

#ifdef HAS_VSNPRINTF
        WITH_LC_NUMERIC_SET_TO_NEEDED(
            retval = vsnprintf(buffer, len, format, ap);
        );
#else
        WITH_LC_NUMERIC_SET_TO_NEEDED(
            retval = vsprintf(buffer, format, ap);
        );
#endif

    }

    va_end(ap);
    /* vsprintf() shows failure with < 0 */
    if (retval < 0
#ifdef HAS_VSNPRINTF
    /* vsnprintf() shows failure with >= len */
        ||
        (len > 0 && (Size_t)retval >= len)
#endif
    )
        Perl_croak_nocontext("panic: my_snprintf buffer overflow");
    return retval;
}

/*
=for apidoc my_vsnprintf

The C library C<vsnprintf> if available and standards-compliant.
However, if the C<vsnprintf> is not available, will unfortunately
use the unsafe C<vsprintf> which can overrun the buffer (there is an
overrun check, but that may be too late).  Consider using
C<sv_vcatpvf> instead, or getting C<vsnprintf>.

=cut
*/

int
Perl_my_vsnprintf(char *buffer, const Size_t len, const char *format, va_list ap)
{
#ifdef USE_QUADMATH
    PERL_UNUSED_ARG(buffer);
    PERL_UNUSED_ARG(len);
    PERL_UNUSED_ARG(format);
    /* the cast is to avoid gcc -Wsizeof-array-argument complaining */
    PERL_UNUSED_ARG((void*)ap);
    Perl_croak_nocontext("panic: my_vsnprintf not available with quadmath");
    return 0;
#else
    int retval;
    dTHX;

#  ifdef NEED_VA_COPY
    va_list apc;

    PERL_ARGS_ASSERT_MY_VSNPRINTF;
    Perl_va_copy(ap, apc);
#    ifdef HAS_VSNPRINTF

    WITH_LC_NUMERIC_SET_TO_NEEDED(
        retval = vsnprintf(buffer, len, format, apc);
    );
#    else
    PERL_UNUSED_ARG(len);
    WITH_LC_NUMERIC_SET_TO_NEEDED(
        retval = vsprintf(buffer, format, apc);
    );
#    endif

    va_end(apc);
#  else
#    ifdef HAS_VSNPRINTF
    WITH_LC_NUMERIC_SET_TO_NEEDED(
        retval = vsnprintf(buffer, len, format, ap);
    );
#    else
    PERL_UNUSED_ARG(len);
    WITH_LC_NUMERIC_SET_TO_NEEDED(
        retval = vsprintf(buffer, format, ap);
    );
#    endif
#  endif /* #ifdef NEED_VA_COPY */

    /* vsprintf() shows failure with < 0 */
    if (retval < 0
#  ifdef HAS_VSNPRINTF
    /* vsnprintf() shows failure with >= len */
        ||
        (len > 0 && (Size_t)retval >= len)
#  endif
    )
        Perl_croak_nocontext("panic: my_vsnprintf buffer overflow");

    return retval;
#endif
}

void
Perl_my_clearenv(pTHX)
{
#if ! defined(PERL_MICRO)
#  if defined(PERL_IMPLICIT_SYS) || defined(WIN32)
    PerlEnv_clearenv();
#  else /* ! (PERL_IMPLICIT_SYS || WIN32) */
#    if defined(USE_ENVIRON_ARRAY)
#      if defined(USE_ITHREADS)
    /* only the parent thread can clobber the process environment, so no need
     * to use a mutex */
    if (PL_curinterp != aTHX)
        return;
#      endif /* USE_ITHREADS */
#      if defined(HAS_CLEARENV)
    clearenv();
#      elif defined(HAS_UNSETENV)
    int bsiz = 80; /* Most envvar names will be shorter than this. */
    char *buf = (char*)safesysmalloc(bsiz);
    while (*environ != NULL) {
        char *e = strchr(*environ, '=');
        int l = e ? e - *environ : (int)strlen(*environ);
        if (bsiz < l + 1) {
            safesysfree(buf);
            bsiz = l + 1; /* + 1 for the \0. */
            buf = (char*)safesysmalloc(bsiz);
        }
        memcpy(buf, *environ, l);
        buf[l] = '\0';
        unsetenv(buf);
    }
    safesysfree(buf);
#      else /* ! HAS_CLEARENV && ! HAS_UNSETENV */
    /* Just null environ and accept the leakage. */
    *environ = NULL;
#      endif /* HAS_CLEARENV || HAS_UNSETENV */
#    endif /* USE_ENVIRON_ARRAY */
#  endif /* PERL_IMPLICIT_SYS || WIN32 */
#endif /* PERL_MICRO */
}

#ifdef MULTIPLICITY

/*
=for apidoc my_cxt_init

Implements the L<perlxs/C<MY_CXT_INIT>> macro, which you should use instead.

The first time a module is loaded, the global C<PL_my_cxt_index> is incremented,
and that value is assigned to that module's static C<my_cxt_index> (whose
address is passed as an arg).  Then, for each interpreter this function is
called for, it makes sure a C<void*> slot is available to hang the static data
off, by allocating or extending the interpreter's C<PL_my_cxt_list> array

=cut
*/

void *
Perl_my_cxt_init(pTHX_ int *indexp, size_t size)
{
    void *p;
    int index;

    PERL_ARGS_ASSERT_MY_CXT_INIT;

    index = *indexp;
    /* do initial check without locking.
     * -1:    not allocated or another thread currently allocating
     *  other: already allocated by another thread
     */
    if (index == -1) {
        MUTEX_LOCK(&PL_my_ctx_mutex);
        /*now a stricter check with locking */
        index = *indexp;
        if (index == -1)
            /* this module hasn't been allocated an index yet */
            *indexp = PL_my_cxt_index++;
        index = *indexp;
        MUTEX_UNLOCK(&PL_my_ctx_mutex);
    }

    /* make sure the array is big enough */
    if (PL_my_cxt_size <= index) {
        if (PL_my_cxt_size) {
            IV new_size = PL_my_cxt_size;
            while (new_size <= index)
                new_size *= 2;
            Renew(PL_my_cxt_list, new_size, void *);
            PL_my_cxt_size = new_size;
        }
        else {
            PL_my_cxt_size = 16;
            Newx(PL_my_cxt_list, PL_my_cxt_size, void *);
        }
    }
    /* newSV() allocates one more than needed */
    p = (void*)SvPVX(newSV(size-1));
    PL_my_cxt_list[index] = p;
    Zero(p, size, char);
    return p;
}

#endif /* MULTIPLICITY */


/* Perl_xs_handshake():
   implement the various XS_*_BOOTCHECK macros, which are added to .c
   files by ExtUtils::ParseXS, to check that the perl the module was built
   with is binary compatible with the running perl.

   usage:
       Perl_xs_handshake(U32 key, void * v_my_perl, const char * file,
            [U32 items, U32 ax], [char * api_version], [char * xs_version])

   The meaning of the varargs is determined the U32 key arg (which is not
   a format string). The fields of key are assembled by using HS_KEY().

   Under PERL_IMPLICIT_CONTEX, the v_my_perl arg is of type
   "PerlInterpreter *" and represents the callers context; otherwise it is
   of type "CV *", and is the boot xsub's CV.

   v_my_perl will catch where a threaded future perl526.dll calling IO.dll
   for example, and IO.dll was linked with threaded perl524.dll, and both
   perl526.dll and perl524.dll are in %PATH and the Win32 DLL loader
   successfully can load IO.dll into the process but simultaneously it
   loaded an interpreter of a different version into the process, and XS
   code will naturally pass SV*s created by perl524.dll for perl526.dll to
   use through perl526.dll's my_perl->Istack_base.

   v_my_perl cannot be the first arg, since then 'key' will be out of
   place in a threaded vs non-threaded mixup; and analyzing the key
   number's bitfields won't reveal the problem, since it will be a valid
   key (unthreaded perl) on interp side, but croak will report the XS mod's
   key as gibberish (it is really a my_perl ptr) (threaded XS mod); or if
   it's a threaded perl and an unthreaded XS module, threaded perl will
   look at an uninit C stack or an uninit register to get 'key'
   (remember that it assumes that the 1st arg is the interp cxt).

   'file' is the source filename of the caller.
*/

I32
Perl_xs_handshake(const U32 key, void * v_my_perl, const char * file, ...)
{
    va_list args;
    U32 items, ax;
    void * got;
    void * need;
    const char *stage = "first";
#ifdef MULTIPLICITY
    dTHX;
    tTHX xs_interp;
#else
    CV* cv;
    SV *** xs_spp;
#endif
    PERL_ARGS_ASSERT_XS_HANDSHAKE;
    va_start(args, file);

    got = INT2PTR(void*, (UV)(key & HSm_KEY_MATCH));
    need = (void *)(HS_KEY(FALSE, FALSE, "", "") & HSm_KEY_MATCH);
    if (UNLIKELY(got != need))
        goto bad_handshake;
/* try to catch where a 2nd threaded perl interp DLL is loaded into a process
   by a XS DLL compiled against the wrong interl DLL b/c of bad @INC, and the
   2nd threaded perl interp DLL never initialized its TLS/PERL_SYS_INIT3 so
   dTHX call from 2nd interp DLL can't return the my_perl that pp_entersub
   passed to the XS DLL */
#ifdef MULTIPLICITY
    xs_interp = (tTHX)v_my_perl;
    got = xs_interp;
    need = my_perl;
#else
/* try to catch where an unthreaded perl interp DLL (for ex. perl522.dll) is
   loaded into a process by a XS DLL built by an unthreaded perl522.dll perl,
   but the DynaLoder/Perl that started the process and loaded the XS DLL is
   unthreaded perl524.dll, since unthreadeds don't pass my_perl (a unique *)
   through pp_entersub, use a unique value (which is a pointer to PL_stack_sp's
   location in the unthreaded perl binary) stored in CV * to figure out if this
   Perl_xs_handshake was called by the same pp_entersub */
    cv = (CV*)v_my_perl;
    xs_spp = (SV***)CvHSCXT(cv);
    got = xs_spp;
    need = &PL_stack_sp;
#endif
    stage = "second";
    if(UNLIKELY(got != need)) {
        bad_handshake:/* recycle branch and string from above */
        if(got != (void *)HSf_NOCHK)
            noperl_die("%s: loadable library and perl binaries are mismatched"
                       " (got %s handshake key %p, needed %p)\n",
                       file, stage, got, need);
    }

    if(key & HSf_SETXSUBFN) {     /* this might be called from a module bootstrap */
        SAVEPPTR(PL_xsubfilename);/* which was require'd from a XSUB BEGIN */
        PL_xsubfilename = file;   /* so the old name must be restored for
                                     additional XSUBs to register themselves */
        /* XSUBs can't be perl lang/perl5db.pl debugged
        if (PERLDB_LINE_OR_SAVESRC)
            (void)gv_fetchfile(file); */
    }

    if(key & HSf_POPMARK) {
        ax = POPMARK;
        {   SV **mark = PL_stack_base + ax++;
            {   dSP;
                items = (I32)(SP - MARK);
            }
        }
    } else {
        items = va_arg(args, U32);
        ax = va_arg(args, U32);
    }
    {
        U32 apiverlen;
        assert(HS_GETAPIVERLEN(key) <= UCHAR_MAX);
        if((apiverlen = HS_GETAPIVERLEN(key))) {
            char * api_p = va_arg(args, char*);
            if(apiverlen != sizeof("v" PERL_API_VERSION_STRING)-1
                || memNE(api_p, "v" PERL_API_VERSION_STRING,
                         sizeof("v" PERL_API_VERSION_STRING)-1))
                Perl_croak_nocontext("Perl API version %s of %" SVf " does not match %s",
                                    api_p, SVfARG(PL_stack_base[ax + 0]),
                                    "v" PERL_API_VERSION_STRING);
        }
    }
    {
        U32 xsverlen = HS_GETXSVERLEN(key);
        assert(xsverlen <= UCHAR_MAX && xsverlen <= HS_APIVERLEN_MAX);
        if(xsverlen)
            S_xs_version_bootcheck(aTHX_
                items, ax, va_arg(args, char*), xsverlen);
    }
    va_end(args);
    return ax;
}


STATIC void
S_xs_version_bootcheck(pTHX_ U32 items, U32 ax, const char *xs_p,
                          STRLEN xs_len)
{
    SV *sv;
    const char *vn = NULL;
    SV *const module = PL_stack_base[ax];

    PERL_ARGS_ASSERT_XS_VERSION_BOOTCHECK;

    if (items >= 2)	 /* version supplied as bootstrap arg */
        sv = PL_stack_base[ax + 1];
    else {
        /* XXX GV_ADDWARN */
        vn = "XS_VERSION";
        sv = get_sv(Perl_form(aTHX_ "%" SVf "::%s", SVfARG(module), vn), 0);
        if (!sv || !SvOK(sv)) {
            vn = "VERSION";
            sv = get_sv(Perl_form(aTHX_ "%" SVf "::%s", SVfARG(module), vn), 0);
        }
    }
    if (sv) {
        SV *xssv = Perl_newSVpvn_flags(aTHX_ xs_p, xs_len, SVs_TEMP);
        SV *pmsv = sv_isobject(sv) && sv_derived_from(sv, "version")
            ? sv : sv_2mortal(new_version(sv));
        xssv = upg_version(xssv, 0);
        if ( vcmp(pmsv,xssv) ) {
            SV *string = vstringify(xssv);
            SV *xpt = Perl_newSVpvf(aTHX_ "%" SVf " object version %" SVf
                                    " does not match ", SVfARG(module), SVfARG(string));

            SvREFCNT_dec(string);
            string = vstringify(pmsv);

            if (vn) {
                Perl_sv_catpvf(aTHX_ xpt, "$%" SVf "::%s %" SVf, SVfARG(module), vn,
                               SVfARG(string));
            } else {
                Perl_sv_catpvf(aTHX_ xpt, "bootstrap parameter %" SVf, SVfARG(string));
            }
            SvREFCNT_dec(string);

            Perl_sv_2mortal(aTHX_ xpt);
            Perl_croak_sv(aTHX_ xpt);
        }
    }
}

PERL_STATIC_INLINE bool
S_gv_has_usable_name(pTHX_ GV *gv)
{
    GV **gvp;
    return GvSTASH(gv)
        && HvHasENAME(GvSTASH(gv))
        && (gvp = (GV **)hv_fetchhek(
                        GvSTASH(gv), GvNAME_HEK(gv), 0
           ))
        && *gvp == gv;
}

void
Perl_get_db_sub(pTHX_ SV **svp, CV *cv)
{
    SV * const dbsv = GvSVn(PL_DBsub);
    const bool save_taint = TAINT_get;

    /* When we are called from pp_goto (svp is null),
     * we do not care about using dbsv to call CV;
     * it's for informational purposes only.
     */

    PERL_ARGS_ASSERT_GET_DB_SUB;

    TAINT_set(FALSE);
    save_item(dbsv);
    if (!PERLDB_SUB_NN) {
        GV *gv = CvGV(cv);

        if (!svp && !CvLEXICAL(cv)) {
            gv_efullname3(dbsv, gv, NULL);
        }
        else if ( (CvFLAGS(cv) & (CVf_ANON | CVf_CLONED)) || CvLEXICAL(cv)
             || strEQ(GvNAME(gv), "END")
             || ( /* Could be imported, and old sub redefined. */
                 (GvCV(gv) != cv || !S_gv_has_usable_name(aTHX_ gv))
                 &&
                 !( (SvTYPE(*svp) == SVt_PVGV)
                    && (GvCV((const GV *)*svp) == cv)
                    /* Use GV from the stack as a fallback. */
                    && S_gv_has_usable_name(aTHX_ gv = (GV *)*svp) 
                  )
                )
        ) {
            /* GV is potentially non-unique, or contain different CV. */
            SV * const tmp = newRV(MUTABLE_SV(cv));
            sv_setsv(dbsv, tmp);
            SvREFCNT_dec(tmp);
        }
        else {
            sv_sethek(dbsv, HvENAME_HEK(GvSTASH(gv)));
            sv_catpvs(dbsv, "::");
            sv_cathek(dbsv, GvNAME_HEK(gv));
        }
    }
    else {
        const int type = SvTYPE(dbsv);
        if (type < SVt_PVIV && type != SVt_IV)
            sv_upgrade(dbsv, SVt_PVIV);
        (void)SvIOK_on(dbsv);
        SvIV_set(dbsv, PTR2IV(cv));	/* Do it the quickest way  */
    }
    SvSETMAGIC(dbsv);
    TAINT_IF(save_taint);
#ifdef NO_TAINT_SUPPORT
    PERL_UNUSED_VAR(save_taint);
#endif
}

/*
=for apidoc_section $io
=for apidoc my_dirfd

The C library C<L<dirfd(3)>> if available, or a Perl implementation of it, or die
if not easily emulatable.

=cut
*/

int
Perl_my_dirfd(DIR * dir) {

    /* Most dirfd implementations have problems when passed NULL. */
    if(!dir)
        return -1;
#ifdef HAS_DIRFD
    return dirfd(dir);
#elif defined(HAS_DIR_DD_FD)
    return dir->dd_fd;
#else
    Perl_croak_nocontext(PL_no_func, "dirfd");
    NOT_REACHED; /* NOTREACHED */
    return 0;
#endif 
}

#if !defined(HAS_MKOSTEMP) || !defined(HAS_MKSTEMP)

#define TEMP_FILE_CH "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvxyz0123456789"
#define TEMP_FILE_CH_COUNT (sizeof(TEMP_FILE_CH)-1)

static int
S_my_mkostemp(char *templte, int flags) {
    dTHX;
    STRLEN len = strlen(templte);
    int fd;
    int attempts = 0;
#ifdef VMS
    int delete_on_close = flags & O_VMS_DELETEONCLOSE;

    flags &= ~O_VMS_DELETEONCLOSE;
#endif

    if (len < 6 ||
        templte[len-1] != 'X' || templte[len-2] != 'X' || templte[len-3] != 'X' ||
        templte[len-4] != 'X' || templte[len-5] != 'X' || templte[len-6] != 'X') {
        SETERRNO(EINVAL, LIB_INVARG);
        return -1;
    }

    do {
        int i;
        for (i = 1; i <= 6; ++i) {
            templte[len-i] = TEMP_FILE_CH[(int)(Perl_internal_drand48() * TEMP_FILE_CH_COUNT)];
        }
#ifdef VMS
        if (delete_on_close) {
            fd = open(templte, O_RDWR | O_CREAT | O_EXCL | flags, 0600, "fop=dlt");
        }
        else
#endif
        {
            fd = PerlLIO_open3(templte, O_RDWR | O_CREAT | O_EXCL | flags, 0600);
        }
    } while (fd == -1 && errno == EEXIST && ++attempts <= 100);

    return fd;
}

#endif

#ifndef HAS_MKOSTEMP

/*
=for apidoc my_mkostemp

The C library C<L<mkostemp(3)>> if available, or a Perl implementation of it.

=cut
*/

int
Perl_my_mkostemp(char *templte, int flags)
{
    PERL_ARGS_ASSERT_MY_MKOSTEMP;
    return S_my_mkostemp(templte, flags);
}
#endif

#ifndef HAS_MKSTEMP

/*
=for apidoc my_mkstemp

The C library C<L<mkstemp(3)>> if available, or a Perl implementation of it.

=cut
*/

int
Perl_my_mkstemp(char *templte)
{
    PERL_ARGS_ASSERT_MY_MKSTEMP;
    return S_my_mkostemp(templte, 0);
}
#endif

REGEXP *
Perl_get_re_arg(pTHX_ SV *sv) {

    if (sv) {
        if (SvMAGICAL(sv))
            mg_get(sv);
        if (SvROK(sv))
            sv = MUTABLE_SV(SvRV(sv));
        if (SvTYPE(sv) == SVt_REGEXP)
            return (REGEXP*) sv;
    }
 
    return NULL;
}

/*
 * This code is derived from drand48() implementation from FreeBSD,
 * found in lib/libc/gen/_rand48.c.
 *
 * The U64 implementation is original, based on the POSIX
 * specification for drand48().
 */

/*
* Copyright (c) 1993 Martin Birgmeier
* All rights reserved.
*
* You may redistribute unmodified or modified versions of this source
* code provided that the above copyright notice and this and the
* following conditions are retained.
*
* This software is provided ``as is'', and comes with no warranties
* of any kind. I shall in no event be liable for anything that happens
* to anyone/anything when using this software.
*/

#define FREEBSD_DRAND48_SEED_0   (0x330e)

#ifdef PERL_DRAND48_QUAD

#define DRAND48_MULT UINT64_C(0x5deece66d)
#define DRAND48_ADD  0xb
#define DRAND48_MASK UINT64_C(0xffffffffffff)

#else

#define FREEBSD_DRAND48_SEED_1   (0xabcd)
#define FREEBSD_DRAND48_SEED_2   (0x1234)
#define FREEBSD_DRAND48_MULT_0   (0xe66d)
#define FREEBSD_DRAND48_MULT_1   (0xdeec)
#define FREEBSD_DRAND48_MULT_2   (0x0005)
#define FREEBSD_DRAND48_ADD      (0x000b)

const unsigned short _rand48_mult[3] = {
                FREEBSD_DRAND48_MULT_0,
                FREEBSD_DRAND48_MULT_1,
                FREEBSD_DRAND48_MULT_2
};
const unsigned short _rand48_add = FREEBSD_DRAND48_ADD;

#endif

void
Perl_drand48_init_r(perl_drand48_t *random_state, U32 seed)
{
    PERL_ARGS_ASSERT_DRAND48_INIT_R;

#ifdef PERL_DRAND48_QUAD
    *random_state = FREEBSD_DRAND48_SEED_0 + ((U64)seed << 16);
#else
    random_state->seed[0] = FREEBSD_DRAND48_SEED_0;
    random_state->seed[1] = (U16) seed;
    random_state->seed[2] = (U16) (seed >> 16);
#endif
}

double
Perl_drand48_r(perl_drand48_t *random_state)
{
    PERL_ARGS_ASSERT_DRAND48_R;

#ifdef PERL_DRAND48_QUAD
    *random_state = (*random_state * DRAND48_MULT + DRAND48_ADD)
        & DRAND48_MASK;

    return ldexp((double)*random_state, -48);
#else
    {
    U32 accu;
    U16 temp[2];

    accu = (U32) _rand48_mult[0] * (U32) random_state->seed[0]
         + (U32) _rand48_add;
    temp[0] = (U16) accu;        /* lower 16 bits */
    accu >>= sizeof(U16) * 8;
    accu += (U32) _rand48_mult[0] * (U32) random_state->seed[1]
          + (U32) _rand48_mult[1] * (U32) random_state->seed[0];
    temp[1] = (U16) accu;        /* middle 16 bits */
    accu >>= sizeof(U16) * 8;
    accu += _rand48_mult[0] * random_state->seed[2]
          + _rand48_mult[1] * random_state->seed[1]
          + _rand48_mult[2] * random_state->seed[0];
    random_state->seed[0] = temp[0];
    random_state->seed[1] = temp[1];
    random_state->seed[2] = (U16) accu;

    return ldexp((double) random_state->seed[0], -48) +
           ldexp((double) random_state->seed[1], -32) +
           ldexp((double) random_state->seed[2], -16);
    }
#endif
}

#ifdef USE_C_BACKTRACE

/* Possibly move all this USE_C_BACKTRACE code into a new file. */

#ifdef USE_BFD

typedef struct {
    /* abfd is the BFD handle. */
    bfd* abfd;
    /* bfd_syms is the BFD symbol table. */
    asymbol** bfd_syms;
    /* bfd_text is handle to the ".text" section of the object file. */
    asection* bfd_text;
    /* Since opening the executable and scanning its symbols is quite
     * heavy operation, we remember the filename we used the last time,
     * and do the opening and scanning only if the filename changes.
     * This removes most (but not all) open+scan cycles. */
    const char* fname_prev;
} bfd_context;

/* Given a dl_info, update the BFD context if necessary. */
static void bfd_update(bfd_context* ctx, Dl_info* dl_info)
{
    /* BFD open and scan only if the filename changed. */
    if (ctx->fname_prev == NULL ||
        strNE(dl_info->dli_fname, ctx->fname_prev)) {
        if (ctx->abfd) {
            bfd_close(ctx->abfd);
        }
        ctx->abfd = bfd_openr(dl_info->dli_fname, 0);
        if (ctx->abfd) {
            if (bfd_check_format(ctx->abfd, bfd_object)) {
                IV symbol_size = bfd_get_symtab_upper_bound(ctx->abfd);
                if (symbol_size > 0) {
                    Safefree(ctx->bfd_syms);
                    Newx(ctx->bfd_syms, symbol_size, asymbol*);
                    ctx->bfd_text =
                        bfd_get_section_by_name(ctx->abfd, ".text");
                }
                else
                    ctx->abfd = NULL;
            }
            else
                ctx->abfd = NULL;
        }
        ctx->fname_prev = dl_info->dli_fname;
    }
}

/* Given a raw frame, try to symbolize it and store
 * symbol information (source file, line number) away. */
static void bfd_symbolize(bfd_context* ctx,
                          void* raw_frame,
                          char** symbol_name,
                          STRLEN* symbol_name_size,
                          char** source_name,
                          STRLEN* source_name_size,
                          STRLEN* source_line)
{
    *symbol_name = NULL;
    *symbol_name_size = 0;
    if (ctx->abfd) {
        IV offset = PTR2IV(raw_frame) - PTR2IV(ctx->bfd_text->vma);
        if (offset > 0 &&
            bfd_canonicalize_symtab(ctx->abfd, ctx->bfd_syms) > 0) {
            const char *file;
            const char *func;
            unsigned int line = 0;
            if (bfd_find_nearest_line(ctx->abfd, ctx->bfd_text,
                                      ctx->bfd_syms, offset,
                                      &file, &func, &line) &&
                file && func && line > 0) {
                /* Size and copy the source file, use only
                 * the basename of the source file.
                 *
                 * NOTE: the basenames are fine for the
                 * Perl source files, but may not always
                 * be the best idea for XS files. */
                const char *p, *b = NULL;
                /* Look for the last slash. */
                for (p = file; *p; p++) {
                    if (*p == '/')
                        b = p + 1;
                }
                if (b == NULL || *b == 0) {
                    b = file;
                }
                *source_name_size = p - b + 1;
                Newx(*source_name, *source_name_size + 1, char);
                Copy(b, *source_name, *source_name_size + 1, char);

                *symbol_name_size = strlen(func);
                Newx(*symbol_name, *symbol_name_size + 1, char);
                Copy(func, *symbol_name, *symbol_name_size + 1, char);

                *source_line = line;
            }
        }
    }
}

#endif /* #ifdef USE_BFD */

#ifdef PERL_DARWIN

/* OS X has no public API for for 'symbolicating' (Apple official term)
 * stack addresses to {function_name, source_file, line_number}.
 * Good news: there is command line utility atos(1) which does that.
 * Bad news 1: it's a command line utility.
 * Bad news 2: one needs to have the Developer Tools installed.
 * Bad news 3: in newer releases it needs to be run as 'xcrun atos'.
 *
 * To recap: we need to open a pipe for reading for a utility which
 * might not exist, or exists in different locations, and then parse
 * the output.  And since this is all for a low-level API, we cannot
 * use high-level stuff.  Thanks, Apple. */

typedef struct {
    /* tool is set to the absolute pathname of the tool to use:
     * xcrun or atos. */
    const char* tool;
    /* format is set to a printf format string used for building
     * the external command to run. */
    const char* format;
    /* unavail is set if e.g. xcrun cannot be found, or something
     * else happens that makes getting the backtrace dubious.  Note,
     * however, that the context isn't persistent, the next call to
     * get_c_backtrace() will start from scratch. */
    bool unavail;
    /* fname is the current object file name. */
    const char* fname;
    /* object_base_addr is the base address of the shared object. */
    void* object_base_addr;
} atos_context;

/* Given |dl_info|, updates the context.  If the context has been
 * marked unavailable, return immediately.  If not but the tool has
 * not been set, set it to either "xcrun atos" or "atos" (also set the
 * format to use for creating commands for piping), or if neither is
 * unavailable (one needs the Developer Tools installed), mark the context
 * an unavailable.  Finally, update the filename (object name),
 * and its base address. */

static void atos_update(atos_context* ctx,
                        Dl_info* dl_info)
{
    if (ctx->unavail)
        return;
    if (ctx->tool == NULL) {
        const char* tools[] = {
            "/usr/bin/xcrun",
            "/usr/bin/atos"
        };
        const char* formats[] = {
            "/usr/bin/xcrun atos -o '%s' -l %08x %08x 2>&1",
            "/usr/bin/atos -d -o '%s' -l %08x %08x 2>&1"
        };
        struct stat st;
        UV i;
        for (i = 0; i < C_ARRAY_LENGTH(tools); i++) {
            if (stat(tools[i], &st) == 0 && S_ISREG(st.st_mode)) {
                ctx->tool = tools[i];
                ctx->format = formats[i];
                break;
            }
        }
        if (ctx->tool == NULL) {
            ctx->unavail = TRUE;
            return;
        }
    }
    if (ctx->fname == NULL ||
        strNE(dl_info->dli_fname, ctx->fname)) {
        ctx->fname = dl_info->dli_fname;
        ctx->object_base_addr = dl_info->dli_fbase;
    }
}

/* Given an output buffer end |p| and its |start|, matches
 * for the atos output, extracting the source code location
 * and returning non-NULL if possible, returning NULL otherwise. */
static const char* atos_parse(const char* p,
                              const char* start,
                              STRLEN* source_name_size,
                              STRLEN* source_line) {
    /* atos() output is something like:
     * perl_parse (in miniperl) (perl.c:2314)\n\n".
     * We cannot use Perl regular expressions, because we need to
     * stay low-level.  Therefore here we have a rolled-out version
     * of a state machine which matches _backwards_from_the_end_ and
     * if there's a success, returns the starts of the filename,
     * also setting the filename size and the source line number.
     * The matched regular expression is roughly "\(.*:\d+\)\s*$" */
    const char* source_number_start;
    const char* source_name_end;
    const char* source_line_end = start;
    const char* close_paren;
    UV uv;

    /* Skip trailing whitespace. */
    while (p > start && isSPACE(*p)) p--;
    /* Now we should be at the close paren. */
    if (p == start || *p != ')')
        return NULL;
    close_paren = p;
    p--;
    /* Now we should be in the line number. */
    if (p == start || !isDIGIT(*p))
        return NULL;
    /* Skip over the digits. */
    while (p > start && isDIGIT(*p))
        p--;
    /* Now we should be at the colon. */
    if (p == start || *p != ':')
        return NULL;
    source_number_start = p + 1;
    source_name_end = p; /* Just beyond the end. */
    p--;
    /* Look for the open paren. */
    while (p > start && *p != '(')
        p--;
    if (p == start)
        return NULL;
    p++;
    *source_name_size = source_name_end - p;
    if (grok_atoUV(source_number_start, &uv,  &source_line_end)
        && source_line_end == close_paren
        && uv <= PERL_INT_MAX
    ) {
        *source_line = (STRLEN)uv;
        return p;
    }
    return NULL;
}

/* Given a raw frame, read a pipe from the symbolicator (that's the
 * technical term) atos, reads the result, and parses the source code
 * location.  We must stay low-level, so we use snprintf(), pipe(),
 * and fread(), and then also parse the output ourselves. */
static void atos_symbolize(atos_context* ctx,
                           void* raw_frame,
                           char** source_name,
                           STRLEN* source_name_size,
                           STRLEN* source_line)
{
    char cmd[1024];
    const char* p;
    Size_t cnt;

    if (ctx->unavail)
        return;
    /* Simple security measure: if there's any funny business with
     * the object name (used as "-o '%s'" ), leave since at least
     * partially the user controls it. */
    for (p = ctx->fname; *p; p++) {
        if (*p == '\'' || isCNTRL(*p)) {
            ctx->unavail = TRUE;
            return;
        }
    }

    dTHX;
    WITH_LC_NUMERIC_SET_TO_NEEDED(
        cnt = snprintf(cmd, sizeof(cmd), ctx->format,
                       ctx->fname, ctx->object_base_addr, raw_frame);
    );

    if (cnt < sizeof(cmd)) {
        /* Undo nostdio.h #defines that disable stdio.
         * This is somewhat naughty, but is used elsewhere
         * in the core, and affects only OS X. */
#undef FILE
#undef popen
#undef fread
#undef pclose
        FILE* fp = popen(cmd, "r");
        /* At the moment we open a new pipe for each stack frame.
         * This is naturally somewhat slow, but hopefully generating
         * stack traces is never going to in a performance critical path.
         *
         * We could play tricks with atos by batching the stack
         * addresses to be resolved: atos can either take multiple
         * addresses from the command line, or read addresses from
         * a file (though the mess of creating temporary files would
         * probably negate much of any possible speedup).
         *
         * Normally there are only two objects present in the backtrace:
         * perl itself, and the libdyld.dylib.  (Note that the object
         * filenames contain the full pathname, so perl may not always
         * be in the same place.)  Whenever the object in the
         * backtrace changes, the base address also changes.
         *
         * The problem with batching the addresses, though, would be
         * matching the results with the addresses: the parsing of
         * the results is already painful enough with a single address. */
        if (fp) {
            char out[1024];
            UV cnt = fread(out, 1, sizeof(out), fp);
            if (cnt < sizeof(out)) {
                const char* p = atos_parse(out + cnt - 1, out,
                                           source_name_size,
                                           source_line);
                if (p) {
                    Newx(*source_name,
                         *source_name_size, char);
                    Copy(p, *source_name,
                         *source_name_size,  char);
                }
            }
            pclose(fp);
        }
    }
}

#endif /* #ifdef PERL_DARWIN */

/*
=for apidoc_section $debugging
=for apidoc get_c_backtrace

Collects the backtrace (aka "stacktrace") into a single linear
malloced buffer, which the caller B<must> C<Perl_free_c_backtrace()>.

Scans the frames back by S<C<depth + skip>>, then drops the C<skip> innermost,
returning at most C<depth> frames.

=cut
*/

Perl_c_backtrace*
Perl_get_c_backtrace(pTHX_ int depth, int skip)
{
    /* Note that here we must stay as low-level as possible: Newx(),
     * Copy(), Safefree(); since we may be called from anywhere,
     * so we should avoid higher level constructs like SVs or AVs.
     *
     * Since we are using safesysmalloc() via Newx(), don't try
     * getting backtrace() there, unless you like deep recursion. */

    /* Currently only implemented with backtrace() and dladdr(),
     * for other platforms NULL is returned. */

#if defined(HAS_BACKTRACE) && defined(HAS_DLADDR)
    /* backtrace() is available via <execinfo.h> in glibc and in most
     * modern BSDs; dladdr() is available via <dlfcn.h>. */

    /* We try fetching this many frames total, but then discard
     * the |skip| first ones.  For the remaining ones we will try
     * retrieving more information with dladdr(). */
    int try_depth = skip +  depth;

    /* The addresses (program counters) returned by backtrace(). */
    void** raw_frames;

    /* Retrieved with dladdr() from the addresses returned by backtrace(). */
    Dl_info* dl_infos;

    /* Sizes _including_ the terminating \0 of the object name
     * and symbol name strings. */
    STRLEN* object_name_sizes;
    STRLEN* symbol_name_sizes;

#ifdef USE_BFD
    /* The symbol names comes either from dli_sname,
     * or if using BFD, they can come from BFD. */
    char** symbol_names;
#endif

    /* The source code location information.  Dug out with e.g. BFD. */
    char** source_names;
    STRLEN* source_name_sizes;
    STRLEN* source_lines;

    Perl_c_backtrace* bt = NULL;  /* This is what will be returned. */
    int got_depth; /* How many frames were returned from backtrace(). */
    UV frame_count = 0; /* How many frames we return. */
    UV total_bytes = 0; /* The size of the whole returned backtrace. */

#ifdef USE_BFD
    bfd_context bfd_ctx;
#endif
#ifdef PERL_DARWIN
    atos_context atos_ctx;
#endif

    /* Here are probably possibilities for optimizing.  We could for
     * example have a struct that contains most of these and then
     * allocate |try_depth| of them, saving a bunch of malloc calls.
     * Note, however, that |frames| could not be part of that struct
     * because backtrace() will want an array of just them.  Also be
     * careful about the name strings. */
    Newx(raw_frames, try_depth, void*);
    Newx(dl_infos, try_depth, Dl_info);
    Newx(object_name_sizes, try_depth, STRLEN);
    Newx(symbol_name_sizes, try_depth, STRLEN);
    Newx(source_names, try_depth, char*);
    Newx(source_name_sizes, try_depth, STRLEN);
    Newx(source_lines, try_depth, STRLEN);
#ifdef USE_BFD
    Newx(symbol_names, try_depth, char*);
#endif

    /* Get the raw frames. */
    got_depth = (int)backtrace(raw_frames, try_depth);

    /* We use dladdr() instead of backtrace_symbols() because we want
     * the full details instead of opaque strings.  This is useful for
     * two reasons: () the details are needed for further symbolic
     * digging, for example in OS X (2) by having the details we fully
     * control the output, which in turn is useful when more platforms
     * are added: we can keep out output "portable". */

    /* We want a single linear allocation, which can then be freed
     * with a single swoop.  We will do the usual trick of first
     * walking over the structure and seeing how much we need to
     * allocate, then allocating, and then walking over the structure
     * the second time and populating it. */

    /* First we must compute the total size of the buffer. */
    total_bytes = sizeof(Perl_c_backtrace_header);
    if (got_depth > skip) {
        int i;
#ifdef USE_BFD
        bfd_init(); /* Is this safe to call multiple times? */
        Zero(&bfd_ctx, 1, bfd_context);
#endif
#ifdef PERL_DARWIN
        Zero(&atos_ctx, 1, atos_context);
#endif
        for (i = skip; i < try_depth; i++) {
            Dl_info* dl_info = &dl_infos[i];

            object_name_sizes[i] = 0;
            source_names[i] = NULL;
            source_name_sizes[i] = 0;
            source_lines[i] = 0;

            /* Yes, zero from dladdr() is failure. */
            if (dladdr(raw_frames[i], dl_info)) {
                total_bytes += sizeof(Perl_c_backtrace_frame);

                object_name_sizes[i] =
                    dl_info->dli_fname ? strlen(dl_info->dli_fname) : 0;
                symbol_name_sizes[i] =
                    dl_info->dli_sname ? strlen(dl_info->dli_sname) : 0;
#ifdef USE_BFD
                bfd_update(&bfd_ctx, dl_info);
                bfd_symbolize(&bfd_ctx, raw_frames[i],
                              &symbol_names[i],
                              &symbol_name_sizes[i],
                              &source_names[i],
                              &source_name_sizes[i],
                              &source_lines[i]);
#endif
#if PERL_DARWIN
                atos_update(&atos_ctx, dl_info);
                atos_symbolize(&atos_ctx,
                               raw_frames[i],
                               &source_names[i],
                               &source_name_sizes[i],
                               &source_lines[i]);
#endif

                /* Plus ones for the terminating \0. */
                total_bytes += object_name_sizes[i] + 1;
                total_bytes += symbol_name_sizes[i] + 1;
                total_bytes += source_name_sizes[i] + 1;

                frame_count++;
            } else {
                break;
            }
        }
#ifdef USE_BFD
        Safefree(bfd_ctx.bfd_syms);
#endif
    }

    /* Now we can allocate and populate the result buffer. */
    Newxc(bt, total_bytes, char, Perl_c_backtrace);
    Zero(bt, total_bytes, char);
    bt->header.frame_count = frame_count;
    bt->header.total_bytes = total_bytes;
    if (frame_count > 0) {
        Perl_c_backtrace_frame* frame = bt->frame_info;
        char* name_base = (char *)(frame + frame_count);
        char* name_curr = name_base; /* Outputting the name strings here. */
        UV i;
        for (i = skip; i < skip + frame_count; i++) {
            Dl_info* dl_info = &dl_infos[i];

            frame->addr = raw_frames[i];
            frame->object_base_addr = dl_info->dli_fbase;
            frame->symbol_addr = dl_info->dli_saddr;

            /* Copies a string, including the \0, and advances the name_curr.
             * Also copies the start and the size to the frame. */
#define PERL_C_BACKTRACE_STRCPY(frame, doffset, src, dsize, size) \
            if (size && src) \
                Copy(src, name_curr, size, char); \
            frame->doffset = name_curr - (char*)bt; \
            frame->dsize = size; \
            name_curr += size; \
            *name_curr++ = 0;

            PERL_C_BACKTRACE_STRCPY(frame, object_name_offset,
                                    dl_info->dli_fname,
                                    object_name_size, object_name_sizes[i]);

#ifdef USE_BFD
            PERL_C_BACKTRACE_STRCPY(frame, symbol_name_offset,
                                    symbol_names[i],
                                    symbol_name_size, symbol_name_sizes[i]);
            Safefree(symbol_names[i]);
#else
            PERL_C_BACKTRACE_STRCPY(frame, symbol_name_offset,
                                    dl_info->dli_sname,
                                    symbol_name_size, symbol_name_sizes[i]);
#endif

            PERL_C_BACKTRACE_STRCPY(frame, source_name_offset,
                                    source_names[i],
                                    source_name_size, source_name_sizes[i]);
            Safefree(source_names[i]);

#undef PERL_C_BACKTRACE_STRCPY

            frame->source_line_number = source_lines[i];

            frame++;
        }
        assert(total_bytes ==
               (UV)(sizeof(Perl_c_backtrace_header) +
                    frame_count * sizeof(Perl_c_backtrace_frame) +
                    name_curr - name_base));
    }
#ifdef USE_BFD
    Safefree(symbol_names);
    if (bfd_ctx.abfd) {
        bfd_close(bfd_ctx.abfd);
    }
#endif
    Safefree(source_lines);
    Safefree(source_name_sizes);
    Safefree(source_names);
    Safefree(symbol_name_sizes);
    Safefree(object_name_sizes);
    /* Assuming the strings returned by dladdr() are pointers
     * to read-only static memory (the object file), so that
     * they do not need freeing (and cannot be). */
    Safefree(dl_infos);
    Safefree(raw_frames);
    return bt;
#else
    PERL_UNUSED_ARG(depth);
    PERL_UNUSED_ARG(skip);
    return NULL;
#endif
}

/*
=for apidoc free_c_backtrace

Deallocates a backtrace received from get_c_backtrace.

=cut
*/

/*
=for apidoc get_c_backtrace_dump

Returns a SV containing a dump of C<depth> frames of the call stack, skipping
the C<skip> innermost ones.  C<depth> of 20 is usually enough.

The appended output looks like:

 ...
 1   10e004812:0082   Perl_croak   util.c:1716    /usr/bin/perl
 2   10df8d6d2:1d72   perl_parse   perl.c:3975    /usr/bin/perl
 ...

The fields are tab-separated.  The first column is the depth (zero
being the innermost non-skipped frame).  In the hex:offset, the hex is
where the program counter was in C<S_parse_body>, and the :offset (might
be missing) tells how much inside the C<S_parse_body> the program counter was.

The C<util.c:1716> is the source code file and line number.

The F</usr/bin/perl> is obvious (hopefully).

Unknowns are C<"-">.  Unknowns can happen unfortunately quite easily:
if the platform doesn't support retrieving the information;
if the binary is missing the debug information;
if the optimizer has transformed the code by for example inlining.

=cut
*/

SV*
Perl_get_c_backtrace_dump(pTHX_ int depth, int skip)
{
    Perl_c_backtrace* bt;

    bt = get_c_backtrace(depth, skip + 1 /* Hide ourselves. */);
    if (bt) {
        Perl_c_backtrace_frame* frame;
        SV* dsv = newSVpvs("");
        UV i;
        for (i = 0, frame = bt->frame_info;
             i < bt->header.frame_count; i++, frame++) {
            Perl_sv_catpvf(aTHX_ dsv, "%d", (int)i);
            Perl_sv_catpvf(aTHX_ dsv, "\t%p", frame->addr ? frame->addr : "-");
            /* Symbol (function) names might disappear without debug info.
             *
             * The source code location might disappear in case of the
             * optimizer inlining or otherwise rearranging the code. */
            if (frame->symbol_addr) {
                Perl_sv_catpvf(aTHX_ dsv, ":%04x",
                               (int)
                               ((char*)frame->addr - (char*)frame->symbol_addr));
            }
            Perl_sv_catpvf(aTHX_ dsv, "\t%s",
                           frame->symbol_name_size &&
                           frame->symbol_name_offset ?
                           (char*)bt + frame->symbol_name_offset : "-");
            if (frame->source_name_size &&
                frame->source_name_offset &&
                frame->source_line_number) {
                Perl_sv_catpvf(aTHX_ dsv, "\t%s:%" UVuf,
                               (char*)bt + frame->source_name_offset,
                               (UV)frame->source_line_number);
            } else {
                Perl_sv_catpvf(aTHX_ dsv, "\t-");
            }
            Perl_sv_catpvf(aTHX_ dsv, "\t%s",
                           frame->object_name_size &&
                           frame->object_name_offset ?
                           (char*)bt + frame->object_name_offset : "-");
            /* The frame->object_base_addr is not output,
             * but it is used for symbolizing/symbolicating. */
            sv_catpvs(dsv, "\n");
        }

        Perl_free_c_backtrace(bt);

        return dsv;
    }

    return NULL;
}

/*
=for apidoc dump_c_backtrace

Dumps the C backtrace to the given C<fp>.

Returns true if a backtrace could be retrieved, false if not.

=cut
*/

bool
Perl_dump_c_backtrace(pTHX_ PerlIO* fp, int depth, int skip)
{
    SV* sv;

    PERL_ARGS_ASSERT_DUMP_C_BACKTRACE;

    sv = Perl_get_c_backtrace_dump(aTHX_ depth, skip);
    if (sv) {
        sv_2mortal(sv);
        PerlIO_printf(fp, "%s", SvPV_nolen(sv));
        return TRUE;
    }
    return FALSE;
}

#endif /* #ifdef USE_C_BACKTRACE */

#if defined(USE_ITHREADS) && defined(I_PTHREAD)

/* pthread_mutex_t and perl_mutex are typedef equivalent
 * so casting the pointers is fine. */

int perl_tsa_mutex_lock(perl_mutex* mutex)
{
    return pthread_mutex_lock((pthread_mutex_t *) mutex);
}

int perl_tsa_mutex_unlock(perl_mutex* mutex)
{
    return pthread_mutex_unlock((pthread_mutex_t *) mutex);
}

int perl_tsa_mutex_destroy(perl_mutex* mutex)
{
    return pthread_mutex_destroy((pthread_mutex_t *) mutex);
}

#endif

#ifdef USE_DTRACE

/* log a sub call or return */

void
Perl_dtrace_probe_call(pTHX_ CV *cv, bool is_call)
{
    const char *func;
    const char *file;
    const char *stash;
    const COP  *start;
    line_t      line;

    PERL_ARGS_ASSERT_DTRACE_PROBE_CALL;

    if (CvNAMED(cv)) {
        HEK *hek = CvNAME_HEK(cv);
        func = HEK_KEY(hek);
    }
    else {
        GV  *gv = CvGV(cv);
        func = GvENAME(gv);
    }
    start = (const COP *)CvSTART(cv);
    file  = CopFILE(start);
    line  = CopLINE(start);
    stash = CopSTASHPV(start);

    if (is_call) {
        PERL_SUB_ENTRY(func, file, line, stash);
    }
    else {
        PERL_SUB_RETURN(func, file, line, stash);
    }
}


/* log a require file loading/loaded  */

void
Perl_dtrace_probe_load(pTHX_ const char *name, bool is_loading)
{
    PERL_ARGS_ASSERT_DTRACE_PROBE_LOAD;

    if (is_loading) {
        PERL_LOADING_FILE(name);
    }
    else {
        PERL_LOADED_FILE(name);
    }
}


/* log an op execution */

void
Perl_dtrace_probe_op(pTHX_ const OP *op)
{
    PERL_ARGS_ASSERT_DTRACE_PROBE_OP;

    PERL_OP_ENTRY(OP_NAME(op));
}


/* log a compile/run phase change */

void
Perl_dtrace_probe_phase(pTHX_ enum perl_phase phase)
{
    const char *ph_old = PL_phase_names[PL_phase];
    const char *ph_new = PL_phase_names[phase];

    PERL_PHASE_CHANGE(ph_new, ph_old);
}

#endif

/*
 * ex: set ts=8 sts=4 sw=4 et:
 */
