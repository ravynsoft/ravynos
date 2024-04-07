/*    perl.h
 *
 *    Copyright (C) 1993, 1994, 1995, 1996, 1997, 1998, 1999, 2000, 2001
 *    2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009 by Larry Wall and others
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

#ifndef H_PERL
#define H_PERL 1

#if defined(__HP_cc) || defined(__HP_aCC)
/* The HPUX compiler for Itanium is very picky and warns about
 * things that gcc doesn't and that we would prefer it does not.
 * So on that platform silence certain warnings unlaterally. */

/* silence "relational operator ">" always evaluates to 'false'"
 * warnings. We get a LOT of these from the memwrap checks. */
#pragma diag_suppress 4276

/* silence "may cause misaligned access" warnings from our "OO in C"
 * type logic. we do this a lot and if it was broken we would fail tests
 * all over the place */
#pragma diag_suppress 4232

#endif /* end HPUX warning disablement */

#ifdef PERL_FOR_X2P
/*
 * This file is being used for x2p stuff.
 * Above symbol is defined via -D in 'x2p/Makefile.SH'
 * Decouple x2p stuff from some of perls more extreme eccentricities.
 */
#undef MULTIPLICITY
#undef USE_STDIO
#define USE_STDIO
#endif /* PERL_FOR_X2P */

#ifdef PERL_MICRO
#   include "uconfig.h"
#else
#   include "config.h"
#endif

/*
=for apidoc_section $debugging
=for apidoc CmnW ||comma_aDEPTH
Some functions when compiled under DEBUGGING take an extra final argument named
C<depth>, indicating the C stack depth.  This argument is omitted otherwise.
This macro expands to either S<C<, depth>> under DEBUGGING, or to nothing at
all when not under DEBUGGING, reducing the number of C<#ifdef>'s in the code.

The program is responsible for maintaining the correct value for C<depth>.

=for apidoc CyW ||comma_pDEPTH
This is used in the prototype declarations for functions that take a L</C<comma_aDEPTH>>
final parameter, much like L<C<pTHX_>|perlguts/Background and MULTIPLICITY>
is used in functions that take a thread context initial parameter.

=for apidoc CmnW ||debug_aDEPTH
Same as L</C<comma_aDEPTH>> but with no leading argument. Intended for functions with
no normal arguments, and used by L</C<comma_aDEPTH>> itself.

=for apidoc CmnW ||debug_pDEPTH
Same as L</C<comma_pDEPTH>> but with no leading argument. Intended for functions with
no normal arguments, and used by L</C<comma_pDEPTH>> itself.

=cut
 */

#ifdef DEBUGGING
#  define debug_pDEPTH U32 depth
#  define comma_pDEPTH ,debug_pDEPTH
#  define debug_aDEPTH depth
#  define comma_aDEPTH ,debug_aDEPTH
#else
#  define debug_aDEPTH
#  define comma_aDEPTH
#  define debug_pDEPTH
#  define comma_pDEPTH
#endif

/* NOTE 1: that with gcc -std=c89 the __STDC_VERSION__ is *not* defined
 * because the __STDC_VERSION__ became a thing only with C90.  Therefore,
 * with gcc, HAS_C99 will never become true as long as we use -std=c89.

 * NOTE 2: headers lie.  Do not expect that if HAS_C99 gets to be true,
 * all the C99 features are there and are correct. */
#if (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L) || \
    defined(_STDC_C99) || defined(__c99)
#  define HAS_C99 1
#endif

/* See L<perlguts/"The Perl API"> for detailed notes on
 * MULTIPLICITY and PERL_IMPLICIT_SYS */

/* XXX NOTE that from here --> to <-- the same logic is
 * repeated in makedef.pl, so be certain to update
 * both places when editing. */

#ifdef USE_ITHREADS
#  if !defined(MULTIPLICITY)
#    define MULTIPLICITY
#  endif
#endif

/* PERL_IMPLICIT_CONTEXT is a legacy synonym for MULTIPLICITY */
#if defined(MULTIPLICITY)               \
 && ! defined(PERL_CORE)                \
 && ! defined(PERL_IMPLICIT_CONTEXT)
#  define PERL_IMPLICIT_CONTEXT
#endif
#if defined(PERL_IMPLICIT_CONTEXT) && !defined(MULTIPLICITY)
#  define MULTIPLICITY
#endif
#if defined(PERL_CORE) && defined(PERL_IMPLICIT_CONTEXT)
#  pragma message("PERL_IMPLICIT_CONTEXT was removed from core perl. It does not do anything. Undeffing it for compilation")
#  undef PERL_IMPLICIT_CONTEXT
#endif

/* undef WIN32 when building on Cygwin (for libwin32) - gph */
#ifdef __CYGWIN__
#   undef WIN32
#   undef _WIN32
#endif

/* Use the reentrant APIs like localtime_r and getpwent_r */
/* Win32 has naturally threadsafe libraries, no need to use any _r variants.
 * XXX KEEP makedef.pl copy of this code in sync */
#if defined(USE_ITHREADS) && !defined(USE_REENTRANT_API) && !defined(WIN32)
#   define USE_REENTRANT_API
#endif

/* <--- here ends the logic shared by perl.h and makedef.pl */

/*
=for apidoc_section $directives
=for apidoc AmnUu|void|EXTERN_C
When not compiling using C++, expands to nothing.
Otherwise is used in a declaration of a function to indicate the function
should have external C linkage.  This is required for things to work for just
about all functions with external linkage compiled into perl.
Often, you can use C<L</START_EXTERN_C>> ... C<L</END_EXTERN_C>> blocks
surrounding all your code that you need to have this linkage.

Example usage:

 EXTERN_C int flock(int fd, int op);

=for apidoc Amnu||START_EXTERN_C
When not compiling using C++, expands to nothing.
Otherwise begins a section of code in which every function will effectively
have C<L</EXTERN_C>> applied to it, that is to have external C linkage.  The
section is ended by a C<L</END_EXTERN_C>>.

=for apidoc Amnu||END_EXTERN_C
When not compiling using C++, expands to nothing.
Otherwise ends a section of code already begun by a C<L</START_EXTERN_C>>.

=cut
*/

#undef START_EXTERN_C
#undef END_EXTERN_C
#undef EXTERN_C
#ifdef __cplusplus
#  define EXTERN_C extern "C"
#  define START_EXTERN_C EXTERN_C {
#  define END_EXTERN_C }
#else
#  define START_EXTERN_C
#  define END_EXTERN_C
#  define EXTERN_C extern
#endif

/* Fallback definitions in case we don't have definitions from config.h.
   This should only matter for systems that don't use Configure and
   haven't been modified to define PERL_STATIC_INLINE yet.
*/
#if !defined(PERL_STATIC_INLINE)
#  ifdef HAS_STATIC_INLINE
#    define PERL_STATIC_INLINE static inline
#  else
#    define PERL_STATIC_INLINE static
#  endif
#endif

/*
=for apidoc_section $concurrency
=for apidoc AmU|void|dTHXa|PerlInterpreter * a
On threaded perls, set C<pTHX> to C<a>; on unthreaded perls, do nothing

=for apidoc AmU|void|dTHXoa|PerlInterpreter * a
Now a synonym for C<L</dTHXa>>.

=cut
*/

#ifdef MULTIPLICITY
#  define tTHX	PerlInterpreter*
#  define pTHX  tTHX my_perl PERL_UNUSED_DECL
#  define aTHX	my_perl
#  define aTHXa(a) aTHX = (tTHX)a
#  define dTHXa(a)	pTHX = (tTHX)a
#  define dTHX		pTHX = PERL_GET_THX
#  define pTHX_		pTHX,
#  define aTHX_		aTHX,
#  define pTHX_1	2
#  define pTHX_2	3
#  define pTHX_3	4
#  define pTHX_4	5
#  define pTHX_5	6
#  define pTHX_6	7
#  define pTHX_7	8
#  define pTHX_8	9
#  define pTHX_9	10
#  define pTHX_12	13
#  if defined(DEBUGGING) && !defined(PERL_TRACK_MEMPOOL)
#    define PERL_TRACK_MEMPOOL
#  endif
#else
#  undef PERL_TRACK_MEMPOOL
#endif

#ifdef DEBUGGING
#  define dTHX_DEBUGGING dTHX
#else
#  define dTHX_DEBUGGING dNOOP
#endif

#define STATIC static

#ifndef PERL_CORE
/* Do not use these macros. They were part of PERL_OBJECT, which was an
 * implementation of multiplicity using C++ objects. They have been left
 * here solely for the sake of XS code which has incorrectly
 * cargo-culted them.
 *
 * The only one Devel::PPPort handles is this; list it as deprecated

=for apidoc_section $concurrency
=for apidoc AmD|void|CPERLscope|void x
Now a no-op.

=cut
 */
#  define CPERLscope(x) x
#  define CPERLarg void
#  define CPERLarg_
#  define _CPERLarg
#  define PERL_OBJECT_THIS
#  define _PERL_OBJECT_THIS
#  define PERL_OBJECT_THIS_
#  define CALL_FPTR(fptr) (*fptr)
#  define MEMBER_TO_FPTR(name) name
#endif /* !PERL_CORE */

#define CALLRUNOPS  PL_runops

#define CALLREGCOMP(sv, flags) Perl_pregcomp(aTHX_ (sv),(flags))

#define CALLREGCOMP_ENG(prog, sv, flags) (prog)->comp(aTHX_ sv, flags)
#define CALLREGEXEC(prog,stringarg,strend,strbeg,minend,sv,data,flags) \
    RX_ENGINE(prog)->exec(aTHX_ (prog),(stringarg),(strend), \
        (strbeg),(minend),(sv),(data),(flags))
#define CALLREG_INTUIT_START(prog,sv,strbeg,strpos,strend,flags,data) \
    RX_ENGINE(prog)->intuit(aTHX_ (prog), (sv), (strbeg), (strpos), \
        (strend),(flags),(data))
#define CALLREG_INTUIT_STRING(prog) \
    RX_ENGINE(prog)->checkstr(aTHX_ (prog))

#define CALLREGFREE(prog) \
    Perl_pregfree(aTHX_ (prog))

#define CALLREGFREE_PVT(prog) \
    if(prog && RX_ENGINE(prog)) RX_ENGINE(prog)->rxfree(aTHX_ (prog))

#define CALLREG_NUMBUF_FETCH(rx,paren,usesv)                                \
    RX_ENGINE(rx)->numbered_buff_FETCH(aTHX_ (rx),(paren),(usesv))

#define CALLREG_NUMBUF_STORE(rx,paren,value) \
    RX_ENGINE(rx)->numbered_buff_STORE(aTHX_ (rx),(paren),(value))

#define CALLREG_NUMBUF_LENGTH(rx,sv,paren)                              \
    RX_ENGINE(rx)->numbered_buff_LENGTH(aTHX_ (rx),(sv),(paren))

#define CALLREG_NAMED_BUFF_FETCH(rx, key, flags) \
    RX_ENGINE(rx)->named_buff(aTHX_ (rx), (key), NULL, ((flags) | RXapif_FETCH))

#define CALLREG_NAMED_BUFF_STORE(rx, key, value, flags) \
    RX_ENGINE(rx)->named_buff(aTHX_ (rx), (key), (value), ((flags) | RXapif_STORE))

#define CALLREG_NAMED_BUFF_DELETE(rx, key, flags) \
    RX_ENGINE(rx)->named_buff(aTHX_ (rx),(key), NULL, ((flags) | RXapif_DELETE))

#define CALLREG_NAMED_BUFF_CLEAR(rx, flags) \
    RX_ENGINE(rx)->named_buff(aTHX_ (rx), NULL, NULL, ((flags) | RXapif_CLEAR))

#define CALLREG_NAMED_BUFF_EXISTS(rx, key, flags) \
    RX_ENGINE(rx)->named_buff(aTHX_ (rx), (key), NULL, ((flags) | RXapif_EXISTS))

#define CALLREG_NAMED_BUFF_FIRSTKEY(rx, flags) \
    RX_ENGINE(rx)->named_buff_iter(aTHX_ (rx), NULL, ((flags) | RXapif_FIRSTKEY))

#define CALLREG_NAMED_BUFF_NEXTKEY(rx, lastkey, flags) \
    RX_ENGINE(rx)->named_buff_iter(aTHX_ (rx), (lastkey), ((flags) | RXapif_NEXTKEY))

#define CALLREG_NAMED_BUFF_SCALAR(rx, flags) \
    RX_ENGINE(rx)->named_buff(aTHX_ (rx), NULL, NULL, ((flags) | RXapif_SCALAR))

#define CALLREG_NAMED_BUFF_COUNT(rx) \
    RX_ENGINE(rx)->named_buff(aTHX_ (rx), NULL, NULL, RXapif_REGNAMES_COUNT)

#define CALLREG_NAMED_BUFF_ALL(rx, flags) \
    RX_ENGINE(rx)->named_buff(aTHX_ (rx), NULL, NULL, flags)

#define CALLREG_PACKAGE(rx) \
    RX_ENGINE(rx)->qr_package(aTHX_ (rx))

#if defined(USE_ITHREADS)
#  define CALLREGDUPE(prog,param) \
    Perl_re_dup(aTHX_ (prog),(param))

#  define CALLREGDUPE_PVT(prog,param) \
    (prog ? RX_ENGINE(prog)->dupe(aTHX_ (prog),(param)) \
          : (REGEXP *)NULL)
#endif

/* some compilers impersonate gcc */
#if defined(__GNUC__) && !defined(__clang__) && !defined(__INTEL_COMPILER)
#  define PERL_IS_GCC 1
#endif

#define PERL_GCC_VERSION_GE(major,minor,patch)                              \
    (((100000 * __GNUC__) + (1000 * __GNUC_MINOR__) + __GNUC_PATCHLEVEL__)  \
        >= ((100000 * (major)) + (1000 * (minor)) + (patch)))
#define PERL_GCC_VERSION_GT(major,minor,patch)                              \
    (((100000 * __GNUC__) + (1000 * __GNUC_MINOR__) + __GNUC_PATCHLEVEL__)  \
        > ((100000 * (major)) + (1000 * (minor)) + (patch)))
#define PERL_GCC_VERSION_LE(major,minor,patch)                              \
    (((100000 * __GNUC__) + (1000 * __GNUC_MINOR__) + __GNUC_PATCHLEVEL__)  \
        <= ((100000 * (major)) + (1000 * (minor)) + (patch)))
#define PERL_GCC_VERSION_LT(major,minor,patch)                              \
    (((100000 * __GNUC__) + (1000 * __GNUC_MINOR__) + __GNUC_PATCHLEVEL__)  \
        < ((100000 * (major)) + (1000 * (minor)) + (patch)))

/* In case Configure was not used (we are using a "canned config"
 * such as Win32, or a cross-compilation setup, for example) try going
 * by the gcc major and minor versions.  One useful URL is
 * http://www.ohse.de/uwe/articles/gcc-attributes.html,
 * but contrary to this information warn_unused_result seems
 * not to be in gcc 3.3.5, at least. --jhi
 * Also, when building extensions with an installed perl, this allows
 * the user to upgrade gcc and get the right attributes, rather than
 * relying on the list generated at Configure time.  --AD
 * Set these up now otherwise we get confused when some of the <*thread.h>
 * includes below indirectly pull in <perlio.h> (which needs to know if we
 * have HASATTRIBUTE_FORMAT).
 */

#ifndef PERL_MICRO
#  if defined __GNUC__ && !defined(__INTEL_COMPILER)
#    if PERL_GCC_VERSION_GE(3,1,0)
#      define HASATTRIBUTE_DEPRECATED
#    endif
#    if PERL_GCC_VERSION_GE(3,0,0)  /* XXX Verify this version */
#      define HASATTRIBUTE_FORMAT
#      if defined __MINGW32__
#        define PRINTF_FORMAT_NULL_OK
#      endif
#    endif
#    if PERL_GCC_VERSION_GE(3,0,0)
#      define HASATTRIBUTE_MALLOC
#    endif
#    if PERL_GCC_VERSION_GE(3,3,0)
#      define HASATTRIBUTE_NONNULL
#    endif
#    if PERL_GCC_VERSION_GE(2,5,0)
#      define HASATTRIBUTE_NORETURN
#    endif
#    if PERL_GCC_VERSION_GE(3,0,0)
#      define HASATTRIBUTE_PURE
#    endif
#    if PERL_GCC_VERSION_GE(3,4,0)
#      define HASATTRIBUTE_UNUSED
#    endif
#    if __GNUC__ == 3 && __GNUC_MINOR__ == 3 && !defined(__cplusplus)
#      define HASATTRIBUTE_UNUSED /* gcc-3.3, but not g++-3.3. */
#    endif
#    if PERL_GCC_VERSION_GE(3,4,0)
#      define HASATTRIBUTE_WARN_UNUSED_RESULT
#    endif
     /* always_inline is buggy in gcc <= 4.6 and causes compilation errors */
#    if PERL_GCC_VERSION_GE(4,7,0)
#      define HASATTRIBUTE_ALWAYS_INLINE
#    endif
#    if PERL_GCC_VERSION_GE(3,3,0)
#      define HASATTRIBUTE_VISIBILITY
#    endif
#  endif
#endif /* #ifndef PERL_MICRO */

#ifdef HASATTRIBUTE_DEPRECATED
#  define __attribute__deprecated__         __attribute__((deprecated))
#endif
#ifdef HASATTRIBUTE_FORMAT
#  define __attribute__format__(x,y,z)      __attribute__((format(x,y,z)))
#endif
#ifdef HASATTRIBUTE_MALLOC
#  define __attribute__malloc__             __attribute__((__malloc__))
#endif
#ifdef HASATTRIBUTE_NONNULL
#  define __attribute__nonnull__(a)         __attribute__((nonnull(a)))
#endif
#ifdef HASATTRIBUTE_NORETURN
#  define __attribute__noreturn__           __attribute__((noreturn))
#endif
#ifdef HASATTRIBUTE_PURE
#  define __attribute__pure__               __attribute__((pure))
#endif
#ifdef HASATTRIBUTE_UNUSED
#  define __attribute__unused__             __attribute__((unused))
#endif
#ifdef HASATTRIBUTE_WARN_UNUSED_RESULT
#  define __attribute__warn_unused_result__ __attribute__((warn_unused_result))
#endif
#ifdef HASATTRIBUTE_ALWAYS_INLINE
/* always_inline is buggy in gcc <= 4.6 and causes compilation errors */
#  if !defined(PERL_IS_GCC) || PERL_GCC_VERSION_GE(4,7,0)
#    define __attribute__always_inline__      __attribute__((always_inline))
#  endif
#endif
#if defined(HASATTRIBUTE_VISIBILITY) && !defined(_WIN32) && !defined(__CYGWIN__)
/* On Windows instead of this, we use __declspec(dllexport) and a .def file
 * Cygwin works by exporting every global symbol, see the definition of ldflags
 * near the end of hints/cygwin.sh and the visibility attribute doesn't appear
 * to control that.
 */
#  define __attribute__visibility__(x) __attribute__((visibility(x)))
#endif

/* If we haven't defined the attributes yet, define them to blank. */
#ifndef __attribute__deprecated__
#  define __attribute__deprecated__
#endif
#ifndef __attribute__format__
#  define __attribute__format__(x,y,z)
#endif
#ifndef __attribute__malloc__
#  define __attribute__malloc__
#endif
#ifndef __attribute__nonnull__
#  define __attribute__nonnull__(a)
#endif
#ifndef __attribute__noreturn__
#  define __attribute__noreturn__
#endif
#ifndef __attribute__pure__
#  define __attribute__pure__
#endif
#ifndef __attribute__unused__
#  define __attribute__unused__
#endif
#ifndef __attribute__warn_unused_result__
#  define __attribute__warn_unused_result__
#endif
#ifndef __attribute__always_inline__
#  define __attribute__always_inline__
#endif
#ifndef __attribute__visibility__
#  define __attribute__visibility__(x)
#endif

/* Some OS warn on NULL format to printf */
#ifdef PRINTF_FORMAT_NULL_OK
#  define __attribute__format__null_ok__(x,y,z)  __attribute__format__(x,y,z)
#else
#  define __attribute__format__null_ok__(x,y,z)
#endif

/*
 * Because of backward compatibility reasons the PERL_UNUSED_DECL
 * cannot be changed from postfix to PERL_UNUSED_DECL(x).  Sigh.
 *
 * Note that there are C compilers such as MetroWerks CodeWarrior
 * which do not have an "inlined" way (like the gcc __attribute__) of
 * marking unused variables (they need e.g. a #pragma) and therefore
 * cpp macros like PERL_UNUSED_DECL cannot work for this purpose, even
 * if it were PERL_UNUSED_DECL(x), which it cannot be (see above).
*/

/*
=for apidoc_section $directives
=for apidoc AmnU||PERL_UNUSED_DECL
Tells the compiler that the parameter in the function prototype just before it
is not necessarily expected to be used in the function.  Not that many
compilers understand this, so this should only be used in cases where
C<L</PERL_UNUSED_ARG>> can't conveniently be used.

Example usage:

=over

 Signal_t
 Perl_perly_sighandler(int sig, Siginfo_t *sip PERL_UNUSED_DECL,
                       void *uap PERL_UNUSED_DECL, bool safe)

=back

=cut
*/

#ifndef PERL_UNUSED_DECL
#  define PERL_UNUSED_DECL __attribute__unused__
#endif

/* gcc -Wall:
 * for silencing unused variables that are actually used most of the time,
 * but we cannot quite get rid of, such as "ax" in PPCODE+noargs xsubs,
 * or variables/arguments that are used only in certain configurations.
 */
/*
=for apidoc Am;||PERL_UNUSED_ARG|void x
This is used to suppress compiler warnings that a parameter to a function is
not used.  This situation can arise, for example, when a parameter is needed
under some configuration conditions, but not others, so that C preprocessor
conditional compilation causes it be used just sometimes.

=for apidoc Amn;||PERL_UNUSED_CONTEXT
This is used to suppress compiler warnings that the thread context parameter to
a function is not used.  This situation can arise, for example, when a
C preprocessor conditional compilation causes it be used just some times.

=for apidoc Am;||PERL_UNUSED_VAR|void x
This is used to suppress compiler warnings that the variable I<x> is not used.
This situation can arise, for example, when a C preprocessor conditional
compilation causes it be used just some times.

=cut
*/
#ifndef PERL_UNUSED_ARG
#  define PERL_UNUSED_ARG(x) ((void)sizeof(x))
#endif
#ifndef PERL_UNUSED_VAR
#  define PERL_UNUSED_VAR(x) ((void)sizeof(x))
#endif

#if defined(USE_ITHREADS)
#  define PERL_UNUSED_CONTEXT PERL_UNUSED_ARG(my_perl)
#else
#  define PERL_UNUSED_CONTEXT
#endif

/* gcc (-ansi) -pedantic doesn't allow gcc statement expressions,
 * g++ allows them but seems to have problems with them
 * (insane errors ensue).
 * g++ does not give insane errors now (RMB 2008-01-30, gcc 4.2.2).
 */
#if defined(PERL_GCC_PEDANTIC) || \
    (defined(__GNUC__) && defined(__cplusplus) && \
        (PERL_GCC_VERSION_LT(4,2,0)))
#  ifndef PERL_GCC_BRACE_GROUPS_FORBIDDEN
#    define PERL_GCC_BRACE_GROUPS_FORBIDDEN
#  endif
#endif

/*

=for apidoc Am||PERL_UNUSED_RESULT|void x

This macro indicates to discard the return value of the function call inside
it, I<e.g.>,

 PERL_UNUSED_RESULT(foo(a, b))

The main reason for this is that the combination of C<gcc -Wunused-result>
(part of C<-Wall>) and the C<__attribute__((warn_unused_result))> cannot
be silenced with casting to C<void>.  This causes trouble when the system
header files use the attribute.

Use C<PERL_UNUSED_RESULT> sparingly, though, since usually the warning
is there for a good reason: you might lose success/failure information,
or leak resources, or changes in resources.

But sometimes you just want to ignore the return value, I<e.g.>, on
codepaths soon ending up in abort, or in "best effort" attempts,
or in situations where there is no good way to handle failures.

Sometimes C<PERL_UNUSED_RESULT> might not be the most natural way:
another possibility is that you can capture the return value
and use C<L</PERL_UNUSED_VAR>> on that.

=cut

The __typeof__() is used instead of typeof() since typeof() is not
available under strict ISO C, and because of compilers masquerading
as gcc (clang and icc), we want exactly the gcc extension
__typeof__ and nothing else.

*/
#ifndef PERL_UNUSED_RESULT
#  if defined(__GNUC__) && defined(HASATTRIBUTE_WARN_UNUSED_RESULT)
#    define PERL_UNUSED_RESULT(v) STMT_START { __typeof__(v) z = (v); (void)sizeof(z); } STMT_END
#  else
#    define PERL_UNUSED_RESULT(v) ((void)(v))
#  endif
#endif

/* on gcc (and clang), specify that a warning should be temporarily
 * ignored; e.g.
 *
 *    GCC_DIAG_IGNORE_DECL(-Wmultichar);
 *    char b = 'ab';
 *    GCC_DIAG_RESTORE_DECL;
 *
 * based on http://dbp-consulting.com/tutorials/SuppressingGCCWarnings.html
 *
 * Note that "pragma GCC diagnostic push/pop" was added in GCC 4.6, Mar 2011;
 * clang only pretends to be GCC 4.2, but still supports push/pop.
 *
 * Note on usage: all macros must be used at a place where a declaration
 * or statement can occur, i.e., not in the middle of an expression.
 * *_DIAG_IGNORE() and *_DIAG_RESTORE can be used in any such place, but
 * must be used without a following semicolon.  *_DIAG_IGNORE_DECL() and
 * *_DIAG_RESTORE_DECL must be used with a following semicolon, and behave
 * syntactically as declarations (like dNOOP).  *_DIAG_IGNORE_STMT()
 * and *_DIAG_RESTORE_STMT must be used with a following semicolon,
 * and behave syntactically as statements (like NOOP).
 *
 */

#if defined(__clang__) || defined(__clang) || PERL_GCC_VERSION_GE(4,6,0)
#  define GCC_DIAG_PRAGMA(x) _Pragma (#x)
/* clang has "clang diagnostic" pragmas, but also understands gcc. */
#  define GCC_DIAG_IGNORE(x) _Pragma("GCC diagnostic push") \
                             GCC_DIAG_PRAGMA(GCC diagnostic ignored #x)
#  define GCC_DIAG_RESTORE   _Pragma("GCC diagnostic pop")
#else
#  define GCC_DIAG_IGNORE(w)
#  define GCC_DIAG_RESTORE
#endif
#define GCC_DIAG_IGNORE_DECL(x) GCC_DIAG_IGNORE(x) dNOOP
#define GCC_DIAG_RESTORE_DECL GCC_DIAG_RESTORE dNOOP
#define GCC_DIAG_IGNORE_STMT(x) GCC_DIAG_IGNORE(x) NOOP
#define GCC_DIAG_RESTORE_STMT GCC_DIAG_RESTORE NOOP
/* for clang specific pragmas */
#if defined(__clang__) || defined(__clang)
#  define CLANG_DIAG_PRAGMA(x) _Pragma (#x)
#  define CLANG_DIAG_IGNORE(x) _Pragma("clang diagnostic push") \
                               CLANG_DIAG_PRAGMA(clang diagnostic ignored #x)
#  define CLANG_DIAG_RESTORE   _Pragma("clang diagnostic pop")
#else
#  define CLANG_DIAG_IGNORE(w)
#  define CLANG_DIAG_RESTORE
#endif
#define CLANG_DIAG_IGNORE_DECL(x) CLANG_DIAG_IGNORE(x) dNOOP
#define CLANG_DIAG_RESTORE_DECL CLANG_DIAG_RESTORE dNOOP
#define CLANG_DIAG_IGNORE_STMT(x) CLANG_DIAG_IGNORE(x) NOOP
#define CLANG_DIAG_RESTORE_STMT CLANG_DIAG_RESTORE NOOP

#if defined(_MSC_VER)
#  define MSVC_DIAG_IGNORE(x) __pragma(warning(push)) \
                              __pragma(warning(disable : x))
#  define MSVC_DIAG_RESTORE   __pragma(warning(pop))
#else
#  define MSVC_DIAG_IGNORE(x)
#  define MSVC_DIAG_RESTORE
#endif
#define MSVC_DIAG_IGNORE_DECL(x) MSVC_DIAG_IGNORE(x) dNOOP
#define MSVC_DIAG_RESTORE_DECL MSVC_DIAG_RESTORE dNOOP
#define MSVC_DIAG_IGNORE_STMT(x) MSVC_DIAG_IGNORE(x) NOOP
#define MSVC_DIAG_RESTORE_STMT MSVC_DIAG_RESTORE NOOP

/*
=for apidoc Amn;||NOOP
Do nothing; typically used as a placeholder to replace something that used to
do something.

=for apidoc Amn;||dNOOP
Declare nothing; typically used as a placeholder to replace something that used
to declare something.  Works on compilers that require declarations before any
code.

=cut
*/
#define NOOP ((void)0)
#define dNOOP struct Perl___notused_struct

#ifndef pTHX
/* Don't bother defining tTHX ; using it outside
 * code guarded by MULTIPLICITY is an error.
 */
#  define pTHX		void
#  define pTHX_
#  define aTHX
#  define aTHX_
#  define aTHXa(a)      NOOP
#  define dTHXa(a)	dNOOP
#  define dTHX		dNOOP
#  define pTHX_1	1
#  define pTHX_2	2
#  define pTHX_3	3
#  define pTHX_4	4
#  define pTHX_5	5
#  define pTHX_6	6
#  define pTHX_7	7
#  define pTHX_8	8
#  define pTHX_9	9
#  define pTHX_12	12
#endif

/*
=for apidoc_section $concurrency
=for apidoc AmnU||dVAR
This is now a synonym for dNOOP: declare nothing

=for apidoc_section $XS
=for apidoc Amn;||dMY_CXT_SV
Now a placeholder that declares nothing

=cut
*/

#ifndef PERL_CORE
    /* Backwards compatibility macro for XS code. It used to be part of the
     * PERL_GLOBAL_STRUCT(_PRIVATE) feature, which no longer exists */
#  define dVAR		dNOOP

    /* these are only defined for compatibility; should not be used internally.
     * */
#  define dMY_CXT_SV    dNOOP
#  ifndef pTHXo
#    define pTHXo		pTHX
#    define pTHXo_	pTHX_
#    define aTHXo		aTHX
#    define aTHXo_	aTHX_
#    define dTHXo		dTHX
#    define dTHXoa(x)	dTHXa(x)
#  endif
#endif

#ifndef pTHXx
#  define pTHXx		PerlInterpreter *my_perl
#  define pTHXx_	pTHXx,
#  define aTHXx		my_perl
#  define aTHXx_	aTHXx,
#  define dTHXx		dTHX
#endif

/* Under PERL_IMPLICIT_SYS (used in Windows for fork emulation)
 * PerlIO_foo() expands to PL_StdIO->pFOO(PL_StdIO, ...).
 * dTHXs is therefore needed for all functions using PerlIO_foo(). */
#ifdef PERL_IMPLICIT_SYS
#    define dTHXs		dTHX
#else
#    define dTHXs		dNOOP
#endif

#if defined(__GNUC__) && !defined(PERL_GCC_BRACE_GROUPS_FORBIDDEN) && !defined(__cplusplus)
#  ifndef PERL_USE_GCC_BRACE_GROUPS
#    define PERL_USE_GCC_BRACE_GROUPS
#  endif
#endif

/*
=for apidoc_section $directives
=for apidoc AmnUu|void|STMT_END
=for apidoc_item |    |STMT_START

These allow a series of statements in a macro to be used as a single statement,
as in

 if (x) STMT_START { ... } STMT_END else ...

Note that you can't return a value out of this construct and cannot use it as
an operand to the comma operator.  These limit its utility.  

But, a value could be returned by constructing the API so that a pointer is
passed and the macro dereferences this to set the return.  If the value can be
any of various types, depending on context, you can handle that situation in
some situations by adding the type of the return as an extra accompanying
parameter:

 #define foo(param, type)  STMT_START {
                              type * param; *param = do_calc; ...
                           } STMT_END

This could be awkward, so consider instead using a C language C<static inline>
function.

If you do use this construct, it is easy to forget that it is a macro and not a
function, and hence fall into traps that might not show up until someone
someday writes code which contains names that clash with the ones you chose
here, or calls it with a parameter which is an expression with side effects,
the consequences of which you didn't think about.  See L<perlhacktips/Writing
safer macros> for how to avoid these.

=for apidoc_section $genconfig
=for apidoc Amn#||PERL_USE_GCC_BRACE_GROUPS

This C pre-processor value, if defined, indicates that it is permissible to use
the GCC brace groups extension.  However, use of this extension is DISCOURAGED.
Use a C<static inline> function instead.

The extension, of the form

 ({ statement ... })

turns the block consisting of I<statement ...> into an expression with a
value, unlike plain C language blocks.  This can present optimization
possibilities, B<BUT>, unless you know for sure that this will never be
compiled without this extension being available and not forbidden, you need to
specify an alternative.  Thus two code paths have to be maintained, which can
get out-of-sync.  All these issues are solved by using a C<static inline>
function instead.

Perl can be configured to not use this feature by passing the parameter
C<-Accflags=-DPERL_GCC_BRACE_GROUPS_FORBIDDEN> to F<Configure>.

=for apidoc Amnh#||PERL_GCC_BRACE_GROUPS_FORBIDDEN

Example usage:

=over

 #ifdef PERL_USE_GCC_BRACE_GROUPS
   ...
 #else
   ...
 #endif

=back

=cut

 Trying to select a version that gives no warnings...
*/
#if !(defined(STMT_START) && defined(STMT_END))
#   define STMT_START	do
#   define STMT_END	while (0)
#endif

#ifndef BYTEORDER  /* Should never happen -- byteorder is in config.h */
#   define BYTEORDER 0x1234
#endif

/*
=for apidoc_section $genconfig
=for apidoc Amn#||ASCIIish

A preprocessor symbol that is defined iff the system is an ASCII platform; this
symbol would not be defined on C<L</EBCDIC>> platforms.

=cut
*/
#if 'A' == 65 && 'I' == 73 && 'J' == 74 && 'Z' == 90
#  define ASCIIish
#else
#  undef  ASCIIish
#endif

/*
 * The following contortions are brought to you on behalf of all the
 * standards, semi-standards, de facto standards, not-so-de-facto standards
 * of the world, as well as all the other botches anyone ever thought of.
 * The basic theory is that if we work hard enough here, the rest of the
 * code can be a lot prettier.  Well, so much for theory.  Sorry, Henry...
 */

/* define this once if either system, instead of cluttering up the src */
#if defined(WIN32)
#define DOSISH 1
#endif

/* These exist only for back-compat with XS modules. */
#ifndef PERL_CORE
#define VOL volatile
#define CAN_PROTOTYPE
#define _(args) args
#define I_LIMITS
#define I_STDARG
#define STANDARD_C
#endif

/* By compiling a perl with -DNO_TAINT_SUPPORT or -DSILENT_NO_TAINT_SUPPORT,
 * you get a perl without taint support, but doubtlessly with a lesser
 * degree of support. Do not do so unless you know exactly what it means
 * technically, have a good reason to do so, and know exactly how the
 * perl will be used. perls with -DSILENT_NO_TAINT_SUPPORT are considered
 * a potential security risk due to flat out ignoring the security-relevant
 * taint flags. This being said, a perl without taint support compiled in
 * has marginal run-time performance benefits.
 * SILENT_NO_TAINT_SUPPORT implies NO_TAINT_SUPPORT.
 * SILENT_NO_TAINT_SUPPORT is the same as NO_TAINT_SUPPORT except it
 * silently ignores -t/-T instead of throwing an exception.
 *
 * DANGER! Using NO_TAINT_SUPPORT or SILENT_NO_TAINT_SUPPORT
 *         voids your nonexistent warranty!
 */
#if defined(SILENT_NO_TAINT_SUPPORT) && !defined(NO_TAINT_SUPPORT)
#  define NO_TAINT_SUPPORT 1
#endif

/* NO_TAINT_SUPPORT can be set to transform virtually all taint-related
 * operations into no-ops for a very modest speed-up. Enable only if you
 * know what you're doing: tests and CPAN modules' tests are bound to fail.
 */
#ifdef NO_TAINT_SUPPORT
#   define TAINT		NOOP
#   define TAINT_NOT		NOOP
#   define TAINT_IF(c)		NOOP
#   define TAINT_ENV()		NOOP
#   define TAINT_PROPER(s)	NOOP
#   define TAINT_set(s)		NOOP
#   define TAINT_get		0
#   define TAINTING_get		0
#   define TAINTING_set(s)	NOOP
#   define TAINT_WARN_get       0
#   define TAINT_WARN_set(s)    NOOP
#else

/*
=for apidoc_section $tainting
=for apidoc Cm|void|TAINT

If we aren't in taint checking mode, do nothing;
otherwise indicate to L</C<TAINT_set>> and L</C<TAINT_PROPER>> that some
unspecified element is tainted.

=for apidoc Cm|void|TAINT_NOT

Remove any taintedness previously set by, I<e.g.>, C<TAINT>.

=for apidoc Cm|void|TAINT_IF|bool c

If C<c> evaluates to true, call L</C<TAINT>> to indicate that something is
tainted; otherwise do nothing.

=for apidoc Cmn|void|TAINT_ENV

Looks at several components of L<C<%ENV>|perlvar/%ENV> for taintedness, and
calls L</C<taint_proper>> if any are tainted.  The components it searches are
things like C<$PATH>.

=for apidoc Cm|void|TAINT_PROPER|const char * s

If no element is tainted, do nothing;
otherwise output a message (containing C<s>) that indicates there is a
tainting violation.  If such violations are fatal, it croaks.

=for apidoc Cm|void|TAINT_set|bool s

If C<s> is true, L</C<TAINT_get>> returns true;
If C<s> is false, L</C<TAINT_get>> returns false;

=for apidoc Cm|bool|TAINT_get

Returns a boolean as to whether some element is tainted or not.

=for apidoc Cm|bool|TAINTING_get

Returns a boolean as to whether taint checking is enabled or not.

=for apidoc Cm|void|TAINTING_set|bool s

Turn taint checking mode off/on

=for apidoc Cm|bool|TAINT_WARN_get

Returns false if tainting violations are fatal;
Returns true if they're just warnings

=for apidoc Cm|void|TAINT_WARN_set|bool s

C<s> being true indicates L</C<TAINT_WARN_get>> should return that tainting
violations are just warnings

C<s> being false indicates L</C<TAINT_WARN_get>> should return that tainting
violations are fatal.

=cut
*/
    /* Set to tainted if we are running under tainting mode */
#   define TAINT		(PL_tainted = PL_tainting)

#   define TAINT_NOT	(PL_tainted = FALSE)        /* Untaint */
#   define TAINT_IF(c)	if (UNLIKELY(c)) { TAINT; } /* Conditionally taint */
#   define TAINT_ENV()	if (UNLIKELY(PL_tainting)) { taint_env(); }
                                /* croak or warn if tainting */
#   define TAINT_PROPER(s)	if (UNLIKELY(PL_tainting)) {                \
                                    taint_proper(NULL, s);                  \
                                }
#   define TAINT_set(s)		(PL_tainted = cBOOL(s))
#   define TAINT_get		(cBOOL(UNLIKELY(PL_tainted)))    /* Is something tainted? */
#   define TAINTING_get		(cBOOL(UNLIKELY(PL_tainting)))
#   define TAINTING_set(s)	(PL_tainting = cBOOL(s))
#   define TAINT_WARN_get       (PL_taint_warn)
#   define TAINT_WARN_set(s)    (PL_taint_warn = cBOOL(s))
#endif

/* flags used internally only within pp_subst and pp_substcont */
#ifdef PERL_CORE
#  define SUBST_TAINT_STR      1	/* string tainted */
#  define SUBST_TAINT_PAT      2	/* pattern tainted */
#  define SUBST_TAINT_REPL     4	/* replacement tainted */
#  define SUBST_TAINT_RETAINT  8	/* use re'taint' in scope */
#  define SUBST_TAINT_BOOLRET 16	/* return is boolean (don't taint) */
#endif

/* XXX All process group stuff is handled in pp_sys.c.  Should these
   defines move there?  If so, I could simplify this a lot. --AD  9/96.
*/
/* Process group stuff changed from traditional BSD to POSIX.
   perlfunc.pod documents the traditional BSD-style syntax, so we'll
   try to preserve that, if possible.
*/
#ifdef HAS_SETPGID
#  define BSD_SETPGRP(pid, pgrp)	setpgid((pid), (pgrp))
#elif defined(HAS_SETPGRP) && defined(USE_BSD_SETPGRP)
#  define BSD_SETPGRP(pid, pgrp)	setpgrp((pid), (pgrp))
#elif defined(HAS_SETPGRP2)
#  define BSD_SETPGRP(pid, pgrp)	setpgrp2((pid), (pgrp))
#endif
#if defined(BSD_SETPGRP) && !defined(HAS_SETPGRP)
#  define HAS_SETPGRP  /* Well, effectively it does . . . */
#endif

/* getpgid isn't POSIX, but at least Solaris and Linux have it, and it makes
    our life easier :-) so we'll try it.
*/
#ifdef HAS_GETPGID
#  define BSD_GETPGRP(pid)		getpgid((pid))
#elif defined(HAS_GETPGRP) && defined(USE_BSD_GETPGRP)
#  define BSD_GETPGRP(pid)		getpgrp((pid))
#elif defined(HAS_GETPGRP2)
#  define BSD_GETPGRP(pid)		getpgrp2((pid))
#endif
#if defined(BSD_GETPGRP) && !defined(HAS_GETPGRP)
#  define HAS_GETPGRP  /* Well, effectively it does . . . */
#endif

/* These are not exact synonyms, since setpgrp() and getpgrp() may
   have different behaviors, but perl.h used to define USE_BSDPGRP
   (prior to 5.003_05) so some extension might depend on it.
*/
#if defined(USE_BSD_SETPGRP) || defined(USE_BSD_GETPGRP)
#  ifndef USE_BSDPGRP
#    define USE_BSDPGRP
#  endif
#endif

/* This define exists only for compatibility. It used to mean "my_setenv and
 * friends should use setenv/putenv, instead of manipulating environ directly",
 * which is now always the case. It's still defined to prevent XS modules from
 * using the no longer existing PL_use_safe_putenv variable.
 */
#define PERL_USE_SAFE_PUTENV

/* HP-UX 10.X CMA (Common Multithreaded Architecture) insists that
   pthread.h must be included before all other header files.
*/
#if defined(USE_ITHREADS) && defined(PTHREAD_H_FIRST) && defined(I_PTHREAD)
#  include <pthread.h>
#endif

#include <sys/types.h>

#  ifdef I_WCHAR
#    include <wchar.h>
#  endif

# include <stdarg.h>

#ifdef I_STDINT
# include <stdint.h>
#endif

#include <ctype.h>
#include <float.h>
#include <limits.h>

#ifdef METHOD 	/* Defined by OSF/1 v3.0 by ctype.h */
#undef METHOD
#endif

#ifdef PERL_MICRO
#   define NO_LOCALE
#endif

#ifdef I_LOCALE
#   include <locale.h>
#endif

#ifdef NEED_XLOCALE_H
#   include <xlocale.h>
#endif

#include "perl_langinfo.h"    /* Needed for _NL_LOCALE_NAME */

/* =========================================================================
 * The defines from here to the following ===== line are unfortunately
 * duplicated in makedef.pl, and changes here MUST also be made there */

/* If not forbidden, we enable locale handling if either 1) the POSIX 2008
 * functions are available, or 2) just the setlocale() function.  This logic is
 * repeated in t/loc_tools.pl and makedef.pl;  The three should be kept in
 * sync. */
#if   ! defined(NO_LOCALE)

#  if ! defined(NO_POSIX_2008_LOCALE)           \
   &&   defined(HAS_NEWLOCALE)                  \
   &&   defined(HAS_USELOCALE)                  \
   &&   defined(HAS_DUPLOCALE)                  \
   &&   defined(HAS_FREELOCALE)                 \
   &&   defined(LC_ALL_MASK)

    /* For simplicity, the code is written to assume that any platform advanced
     * enough to have the Posix 2008 locale functions has LC_ALL.  The final
     * test above makes sure that assumption is valid */

#    define HAS_POSIX_2008_LOCALE
#    define USE_LOCALE
#  elif defined(HAS_SETLOCALE)
#    define USE_LOCALE
#  endif
#endif

#ifdef USE_LOCALE
#   define HAS_SKIP_LOCALE_INIT /* Solely for XS code to test for this
                                   #define */
#   if !defined(NO_LOCALE_COLLATE) && defined(LC_COLLATE) \
       && defined(HAS_STRXFRM)
#	define USE_LOCALE_COLLATE
#   endif
#   if !defined(NO_LOCALE_CTYPE) && defined(LC_CTYPE)
#	define USE_LOCALE_CTYPE
#   endif
#   if !defined(NO_LOCALE_NUMERIC) && defined(LC_NUMERIC)
#	define USE_LOCALE_NUMERIC
#   endif
#   if !defined(NO_LOCALE_MESSAGES) && defined(LC_MESSAGES)
#	define USE_LOCALE_MESSAGES
#   endif
#   if !defined(NO_LOCALE_MONETARY) && defined(LC_MONETARY)
#	define USE_LOCALE_MONETARY
#   endif
#   if !defined(NO_LOCALE_TIME) && defined(LC_TIME)
#	define USE_LOCALE_TIME
#   endif
#   if !defined(NO_LOCALE_ADDRESS) && defined(LC_ADDRESS)
#	define USE_LOCALE_ADDRESS
#   endif
#   if !defined(NO_LOCALE_IDENTIFICATION) && defined(LC_IDENTIFICATION)
#	define USE_LOCALE_IDENTIFICATION
#   endif
#   if !defined(NO_LOCALE_MEASUREMENT) && defined(LC_MEASUREMENT)
#	define USE_LOCALE_MEASUREMENT
#   endif
#   if !defined(NO_LOCALE_PAPER) && defined(LC_PAPER)
#	define USE_LOCALE_PAPER
#   endif
#   if !defined(NO_LOCALE_TELEPHONE) && defined(LC_TELEPHONE)
#	define USE_LOCALE_TELEPHONE
#   endif
#   if !defined(NO_LOCALE_NAME) && defined(LC_NAME)
#	define USE_LOCALE_NAME
#   endif
#   if !defined(NO_LOCALE_SYNTAX) && defined(LC_SYNTAX)
#	define USE_LOCALE_SYNTAX
#   endif
#   if !defined(NO_LOCALE_TOD) && defined(LC_TOD)
#	define USE_LOCALE_TOD
#   endif

/* Now create LC_foo_INDEX_ #defines for just those categories on this system */
#  ifdef USE_LOCALE_CTYPE
#    define LC_CTYPE_INDEX_             0
#    define PERL_DUMMY_CTYPE_           LC_CTYPE_INDEX_
#  else
#    define PERL_DUMMY_CTYPE_           -1
#  endif
#  ifdef USE_LOCALE_NUMERIC
#    define LC_NUMERIC_INDEX_           PERL_DUMMY_CTYPE_ + 1
#    define PERL_DUMMY_NUMERIC_         LC_NUMERIC_INDEX_
#  else
#    define PERL_DUMMY_NUMERIC_         PERL_DUMMY_CTYPE_
#  endif
#  ifdef USE_LOCALE_COLLATE
#    define LC_COLLATE_INDEX_           PERL_DUMMY_NUMERIC_ + 1
#    define PERL_DUMMY_COLLATE_         LC_COLLATE_INDEX_
#  else
#    define PERL_DUMMY_COLLATE_         PERL_DUMMY_NUMERIC_
#  endif
#  ifdef USE_LOCALE_TIME
#    define LC_TIME_INDEX_              PERL_DUMMY_COLLATE_ + 1
#    define PERL_DUMMY_TIME_            LC_TIME_INDEX_
#  else
#    define PERL_DUMMY_TIME_            PERL_DUMMY_COLLATE_
#  endif
#  ifdef USE_LOCALE_MESSAGES
#    define LC_MESSAGES_INDEX_          PERL_DUMMY_TIME_ + 1
#    define PERL_DUMMY_MESSAGES_        LC_MESSAGES_INDEX_
#  else
#    define PERL_DUMMY_MESSAGES_        PERL_DUMMY_TIME_
#  endif
#  ifdef USE_LOCALE_MONETARY
#    define LC_MONETARY_INDEX_          PERL_DUMMY_MESSAGES_ + 1
#    define PERL_DUMMY_MONETARY_        LC_MONETARY_INDEX_
#  else
#    define PERL_DUMMY_MONETARY_        PERL_DUMMY_MESSAGES_
#  endif
#  ifdef USE_LOCALE_ADDRESS
#    define LC_ADDRESS_INDEX_           PERL_DUMMY_MONETARY_ + 1
#    define PERL_DUMMY_ADDRESS_         LC_ADDRESS_INDEX_
#  else
#    define PERL_DUMMY_ADDRESS_         PERL_DUMMY_MONETARY_
#  endif
#  ifdef USE_LOCALE_IDENTIFICATION
#    define LC_IDENTIFICATION_INDEX_    PERL_DUMMY_ADDRESS_ + 1
#    define PERL_DUMMY_IDENTIFICATION_  LC_IDENTIFICATION_INDEX_
#  else
#    define PERL_DUMMY_IDENTIFICATION_  PERL_DUMMY_ADDRESS_
#  endif
#  ifdef USE_LOCALE_MEASUREMENT
#    define LC_MEASUREMENT_INDEX_       PERL_DUMMY_IDENTIFICATION_ + 1
#    define PERL_DUMMY_MEASUREMENT_     LC_MEASUREMENT_INDEX_
#  else
#    define PERL_DUMMY_MEASUREMENT_    PERL_DUMMY_IDENTIFICATION_
#  endif
#  ifdef USE_LOCALE_PAPER
#    define LC_PAPER_INDEX_             PERL_DUMMY_MEASUREMENT_ + 1
#    define PERL_DUMMY_PAPER_           LC_PAPER_INDEX_
#  else
#    define PERL_DUMMY_PAPER_           PERL_DUMMY_MEASUREMENT_
#  endif
#  ifdef USE_LOCALE_TELEPHONE
#    define LC_TELEPHONE_INDEX_         PERL_DUMMY_PAPER_ + 1
#    define PERL_DUMMY_TELEPHONE_       LC_TELEPHONE_INDEX_
#  else
#    define PERL_DUMMY_TELEPHONE_       PERL_DUMMY_PAPER_
#  endif
#  ifdef USE_LOCALE_NAME
#    define LC_NAME_INDEX_              PERL_DUMMY_TELEPHONE_ + 1
#    define PERL_DUMMY_NAME_            LC_NAME_INDEX_
#  else
#    define PERL_DUMMY_NAME_            PERL_DUMMY_TELEPHONE_
#  endif
#  ifdef USE_LOCALE_SYNTAX
#    define LC_SYNTAX_INDEX_            PERL_DUMMY_NAME + 1
#    define PERL_DUMMY_SYNTAX_          LC_SYNTAX_INDEX_
#  else
#    define PERL_DUMMY_SYNTAX_          PERL_DUMMY_NAME_
#  endif
#  ifdef USE_LOCALE_TOD
#    define LC_TOD_INDEX_               PERL_DUMMY_SYNTAX_ + 1
#    define PERL_DUMMY_TOD_             LC_TOD_INDEX_
#  else
#    define PERL_DUMMY_TOD_             PERL_DUMMY_SYNTAX_
#  endif
#  ifdef LC_ALL
#    define LC_ALL_INDEX_               PERL_DUMMY_TOD_ + 1
#  endif


#  if defined(USE_ITHREADS) && ! defined(NO_LOCALE_THREADS)
#    define USE_LOCALE_THREADS
#  endif

   /* Use POSIX 2008 locales if available, and no alternative exists
    * ('setlocale()' is the alternative); or is threaded and not forbidden to
    * use them */
#  if defined(HAS_POSIX_2008_LOCALE) && (  ! defined(HAS_SETLOCALE)            \
                                         || (     defined(USE_LOCALE_THREADS)  \
                                             && ! defined(NO_POSIX_2008_LOCALE)))
#    define USE_POSIX_2008_LOCALE
#  endif

   /* On threaded builds, use thread-safe locales if they are available and not
    * forbidden.  Availability is when we are using POSIX 2008 locales, or
    * Windows for quite a few releases now. */
#  if defined(USE_LOCALE_THREADS) && ! defined(NO_THREAD_SAFE_LOCALE)
#    if defined(USE_POSIX_2008_LOCALE) || (defined(WIN32) && defined(_MSC_VER))
#      define USE_THREAD_SAFE_LOCALE
#    endif
#  endif

#  include "perl_langinfo.h"    /* Needed for _NL_LOCALE_NAME */

#  ifdef USE_POSIX_2008_LOCALE
#    if  defined(HAS_QUERYLOCALE)                                           \
              /* Use querylocale if has it, or has the glibc internal       \
               * undocumented equivalent. */                                \
     || (     defined(_NL_LOCALE_NAME)                                      \
              /* And asked for */                                           \
         &&   defined(USE_NL_LOCALE_NAME)                                   \
              /* nl_langinfo_l almost certainly will exist on systems that  \
               * have _NL_LOCALE_NAME, so there is nothing lost by          \
               * requiring it instead of also allowing plain nl_langinfo(). \
               * And experience indicates that its glibc implementation is  \
               * thread-safe, eliminating code complications */             \
         &&   defined(HAS_NL_LANGINFO_L)                                    \
               /* On systems that accept any locale name, the real          \
                * underlying locale is often returned by this internal      \
                * item, so we can't use it */                               \
         && ! defined(SETLOCALE_ACCEPTS_ANY_LOCALE_NAME))
#      define USE_QUERYLOCALE
#    endif
#  endif

   /* POSIX 2008 has no means of finding out the current locale without a
    * querylocale; so must keep track of it ourselves */
#  if (defined(USE_POSIX_2008_LOCALE) && ! defined(USE_QUERYLOCALE))
#    define USE_PL_CURLOCALES
#    define USE_PL_CUR_LC_ALL
#  endif

#  if defined(WIN32) && defined(USE_THREAD_SAFE_LOCALE)

   /* We need to be able to map the current value of what the tTHX context
    * thinks LC_ALL is so as to inform the Windows libc when switching
    * contexts. */
#    define USE_PL_CUR_LC_ALL

   /* Microsoft documentation reads in the change log for VS 2015: "The
    * localeconv function declared in locale.h now works correctly when
    * per-thread locale is enabled. In previous versions of the library, this
    * function would return the lconv data for the global locale, not the
    * thread's locale." */
#    if _MSC_VER < 1900
#      define TS_W32_BROKEN_LOCALECONV
#    endif
#  endif

   /* POSIX 2008 and Windows with thread-safe locales keep locale information
    * in libc data.  Therefore we must inform their libc's when the context
    * switches */
#  if defined(MULTIPLICITY) && (   defined(USE_POSIX_2008_LOCALE)           \
                                || (   defined(WIN32)                       \
                                    && defined(USE_THREAD_SAFE_LOCALE)))
#    define USE_PERL_SWITCH_LOCALE_CONTEXT
#  endif
#endif

/* end of makedef.pl logic duplication
 * ========================================================================= */

#ifdef PERL_CORE

/* Both typedefs are used in locale.c only, but defined here so that embed.fnc
 * can generate the proper prototypes. */

typedef enum {
    DONT_RECALC_LC_ALL,
    YES_RECALC_LC_ALL,

    /* Used in tight loops through all sub-categories, where LC_ALL won't be
     * fully known until all subcategories are handled. */
    RECALCULATE_LC_ALL_ON_FINAL_INTERATION
} recalc_lc_all_t;


typedef enum {  /* Is the locale UTF8? */
    LOCALE_NOT_UTF8,
    LOCALE_IS_UTF8,
    LOCALE_UTF8NESS_UNKNOWN
} locale_utf8ness_t;

typedef struct {
    const char *name;
    size_t offset;
} lconv_offset_t;


#endif

#include <setjmp.h>

#ifdef I_SYS_PARAM
#   ifdef PARAM_NEEDS_TYPES
#	include <sys/types.h>
#   endif
#   include <sys/param.h>
#endif

/* On BSD-derived systems, <sys/param.h> defines BSD to a year-month
   value something like 199306.  This may be useful if no more-specific
   feature test is available.
*/
#if defined(BSD)
#   ifndef BSDish
#       define BSDish
#   endif
#endif

/* Use all the "standard" definitions */
#include <stdlib.h>

/* If this causes problems, set i_unistd=undef in the hint file.  */
#ifdef I_UNISTD
#    if defined(__amigaos4__)
#        ifdef I_NETINET_IN
#            include <netinet/in.h>
#        endif
#   endif
#   include <unistd.h>
#   if defined(__amigaos4__)
/* Under AmigaOS 4 newlib.library provides an environ.  However using
 * it doesn't give us enough control over inheritance of variables by
 * subshells etc. so replace with custom version based on abc-shell
 * code. */
extern char **myenviron;
#       undef environ
#       define environ myenviron
#   endif
#endif

/* for WCOREDUMP */
#ifdef I_SYS_WAIT
#   include <sys/wait.h>
#endif

#if defined(HAS_SYSCALL) && !defined(HAS_SYSCALL_PROTO)
EXTERN_C int syscall(int, ...);
#endif

#if defined(HAS_USLEEP) && !defined(HAS_USLEEP_PROTO)
EXTERN_C int usleep(unsigned int);
#endif

/* Macros for correct constant construction.  These are in C99 <stdint.h>
 * (so they will not be available in strict C89 mode), but they are nice, so
 * let's define them if necessary. */

/*
=for apidoc_section $integer
=for apidoc    Am|I16|INT16_C|number
=for apidoc_item |I32|INT32_C|number
=for apidoc_item |I64|INT64_C|number

Returns a token the C compiler recognizes for the constant C<number> of the
corresponding integer type on the machine.

If the machine does not have a 64-bit type, C<INT64_C> is undefined.
Use C<L</INTMAX_C>> to get the largest type available on the platform.

=for apidoc    Am|U16|UINT16_C|number
=for apidoc_item |U32|UINT32_C|number
=for apidoc_item |U64|UINT64_C|number

Returns a token the C compiler recognizes for the constant C<number> of the
corresponding unsigned integer type on the machine.

If the machine does not have a 64-bit type, C<UINT64_C> is undefined.
Use C<L</UINTMAX_C>> to get the largest type available on the platform.


=cut
*/
#ifndef UINT16_C
#  if INTSIZE >= 2
#    define UINT16_C(x) ((U16_TYPE)x##U)
#  else
#    define UINT16_C(x) ((U16_TYPE)x##UL)
#  endif
#endif

#ifndef UINT32_C
#  if INTSIZE >= 4
#    define UINT32_C(x) ((U32_TYPE)x##U)
#  else
#    define UINT32_C(x) ((U32_TYPE)x##UL)
#  endif
#endif

#ifdef I_STDINT
    typedef intmax_t  PERL_INTMAX_T;
    typedef uintmax_t PERL_UINTMAX_T;
#endif

/* N.B.  We use QUADKIND here instead of HAS_QUAD here, because that doesn't
 * actually mean what it has always been documented to mean (see RT #119753)
 * and is explicitly turned off outside of core with dire warnings about
 * removing the undef. */

#if defined(QUADKIND)
#  undef PeRl_INT64_C
#  undef PeRl_UINT64_C
/* Prefer the native integer types (int and long) over long long
 * (which is not C89) and Win32-specific __int64. */
#  if QUADKIND == QUAD_IS_INT && INTSIZE == 8
#    define PeRl_INT64_C(c)	(c)
#    define PeRl_UINT64_C(c)	CAT2(c,U)
#  endif
#  if QUADKIND == QUAD_IS_LONG && LONGSIZE == 8
#    define PeRl_INT64_C(c)	CAT2(c,L)
#    define PeRl_UINT64_C(c)	CAT2(c,UL)
#  endif
#  if QUADKIND == QUAD_IS_LONG_LONG && defined(HAS_LONG_LONG)
#    define PeRl_INT64_C(c)	CAT2(c,LL)
#    define PeRl_UINT64_C(c)	CAT2(c,ULL)
#  endif
#  if QUADKIND == QUAD_IS___INT64
#    define PeRl_INT64_C(c)	CAT2(c,I64)
#    define PeRl_UINT64_C(c)	CAT2(c,UI64)
#  endif
#  ifndef PeRl_INT64_C
#    define PeRl_INT64_C(c)	((I64)(c)) /* last resort */
#    define PeRl_UINT64_C(c)	((U64TYPE)(c))
#  endif
/* In OS X the INT64_C/UINT64_C are defined with LL/ULL, which will
 * not fly with C89-pedantic gcc, so let's undefine them first so that
 * we can redefine them with our native integer preferring versions. */
#  if defined(PERL_DARWIN) && defined(PERL_GCC_PEDANTIC)
#    undef INT64_C
#    undef UINT64_C
#  endif
#  ifndef INT64_C
#    define INT64_C(c) PeRl_INT64_C(c)
#  endif
#  ifndef UINT64_C
#    define UINT64_C(c) PeRl_UINT64_C(c)
#  endif

/*
=for apidoc_section $integer
=for apidoc Am||INTMAX_C|number
Returns a token the C compiler recognizes for the constant C<number> of the
widest integer type on the machine.  For example, if the machine has C<long
long>s, C<INTMAX_C(-1)> would yield

 -1LL

See also, for example, C<L</INT32_C>>.

Use L</IV> to declare variables of the maximum usable size on this platform.

=for apidoc Am||UINTMAX_C|number
Returns a token the C compiler recognizes for the constant C<number> of the
widest unsigned integer type on the machine.  For example, if the machine has
C<long>s, C<UINTMAX_C(1)> would yield

 1UL

See also, for example, C<L</UINT32_C>>.

Use L</UV> to declare variables of the maximum usable size on this platform.

=cut
*/

#  ifndef I_STDINT
    typedef I64TYPE PERL_INTMAX_T;
    typedef U64TYPE PERL_UINTMAX_T;
#  endif
#  ifndef INTMAX_C
#    define INTMAX_C(c) INT64_C(c)
#  endif
#  ifndef UINTMAX_C
#    define UINTMAX_C(c) UINT64_C(c)
#  endif

#else  /* below QUADKIND is undefined */

/* Perl doesn't work on 16 bit systems, so must be 32 bit */
#  ifndef I_STDINT
    typedef I32TYPE PERL_INTMAX_T;
    typedef U32TYPE PERL_UINTMAX_T;
#  endif
#  ifndef INTMAX_C
#    define INTMAX_C(c) INT32_C(c)
#  endif
#  ifndef UINTMAX_C
#    define UINTMAX_C(c) UINT32_C(c)
#  endif

#endif  /* no QUADKIND */

#ifdef PERL_CORE

/* byte-swapping functions for big-/little-endian conversion */
# define _swab_16_(x) ((U16)( \
         (((U16)(x) & UINT16_C(0x00ff)) << 8) | \
         (((U16)(x) & UINT16_C(0xff00)) >> 8) ))

# define _swab_32_(x) ((U32)( \
         (((U32)(x) & UINT32_C(0x000000ff)) << 24) | \
         (((U32)(x) & UINT32_C(0x0000ff00)) <<  8) | \
         (((U32)(x) & UINT32_C(0x00ff0000)) >>  8) | \
         (((U32)(x) & UINT32_C(0xff000000)) >> 24) ))

# ifdef HAS_QUAD
#  define _swab_64_(x) ((U64)( \
          (((U64)(x) & UINT64_C(0x00000000000000ff)) << 56) | \
          (((U64)(x) & UINT64_C(0x000000000000ff00)) << 40) | \
          (((U64)(x) & UINT64_C(0x0000000000ff0000)) << 24) | \
          (((U64)(x) & UINT64_C(0x00000000ff000000)) <<  8) | \
          (((U64)(x) & UINT64_C(0x000000ff00000000)) >>  8) | \
          (((U64)(x) & UINT64_C(0x0000ff0000000000)) >> 24) | \
          (((U64)(x) & UINT64_C(0x00ff000000000000)) >> 40) | \
          (((U64)(x) & UINT64_C(0xff00000000000000)) >> 56) ))
# endif

/* Maximum level of recursion */
#ifndef PERL_SUB_DEPTH_WARN
#define PERL_SUB_DEPTH_WARN 100
#endif

#endif /* PERL_CORE */

/* Maximum number of args that may be passed to an OP_MULTICONCAT op.
 * It determines the size of local arrays in S_maybe_multiconcat() and
 * pp_multiconcat().
 */
#define PERL_MULTICONCAT_MAXARG 64

/* The indexes of fields of a multiconcat aux struct.
 * The fixed fields are followed by nargs+1 const segment lengths,
 * and if utf8 and non-utf8 differ, a second nargs+1 set for utf8.
 */

#define PERL_MULTICONCAT_IX_NARGS     0 /* number of arguments */
#define PERL_MULTICONCAT_IX_PLAIN_PV  1 /* non-utf8 constant string */
#define PERL_MULTICONCAT_IX_PLAIN_LEN 2 /* non-utf8 constant string length */
#define PERL_MULTICONCAT_IX_UTF8_PV   3 /* utf8 constant string */
#define PERL_MULTICONCAT_IX_UTF8_LEN  4 /* utf8 constant string length */
#define PERL_MULTICONCAT_IX_LENGTHS   5 /* first of nargs+1 const segment lens */
#define PERL_MULTICONCAT_HEADER_SIZE 5 /* The number of fields of a
                                           multiconcat header */

/* We no longer default to creating a new SV for GvSV.
   Do this before embed.  */
#ifndef PERL_CREATE_GVSV
#  ifndef PERL_DONT_CREATE_GVSV
#    define PERL_DONT_CREATE_GVSV
#  endif
#endif

#if !defined(HAS_WAITPID) && !defined(HAS_WAIT4) || defined(HAS_WAITPID_RUNTIME)
#define PERL_USES_PL_PIDSTATUS
#endif

#if !defined(OS2) && !defined(WIN32)
#define PERL_DEFAULT_DO_EXEC3_IMPLEMENTATION
#endif

#define MEM_SIZE Size_t

/* av_extend and analogues enforce a minimum number of array elements.
 * This has been 4 elements (so a minimum key size of 3) for a long
 * time, but the rationale behind this seems to have been lost to the
 * mists of time. */
#ifndef PERL_ARRAY_NEW_MIN_KEY
#define PERL_ARRAY_NEW_MIN_KEY 3
#endif

/* Functions like Perl_sv_grow mandate a minimum string size.
 * This was 10 bytes for a long time, the rationale for which seems lost
 * to the mists of time. However, this does not correlate to what modern
 * malloc implementations will actually return, in particular the fact
 * that chunks are almost certainly some multiple of pointer size. The
 * default has therefore been revised to a more useful approximation.
 * Notes: The following is specifically conservative for 64 bit, since
 * most dlmalloc derivatives seem to serve a 3xPTRSIZE minimum chunk,
 * so the below perhaps should be:
 *     ((PTRSIZE == 4) ? 12 : 24)
 * Configure probes for malloc_good_size, malloc_actual_size etc.
 * could be revised to record the actual minimum chunk size, to which
 * PERL_STRLEN_NEW_MIN could then be set.
 */
#ifndef PERL_STRLEN_NEW_MIN
#define PERL_STRLEN_NEW_MIN ((PTRSIZE == 4) ? 12 : 16)
#endif

/* Round all values passed to malloc up, by default to a multiple of
   sizeof(size_t)
*/
#ifndef PERL_STRLEN_ROUNDUP_QUANTUM
#define PERL_STRLEN_ROUNDUP_QUANTUM Size_t_size
#endif

/* sv_grow() will expand strings by at least a certain percentage of
   the previously *used* length to avoid excessive calls to realloc().
   The default is 25% of the current length.
*/
#ifndef PERL_STRLEN_EXPAND_SHIFT
#  define PERL_STRLEN_EXPAND_SHIFT 2
#endif

/* This use of offsetof() requires /Zc:offsetof- for VS2017 (and presumably
 * onwards) when building Socket.xs, but we can just use a different definition
 * for STRUCT_OFFSET instead. */
#if defined(WIN32) && defined(_MSC_VER) && _MSC_VER >= 1910
#  define STRUCT_OFFSET(s,m)  (Size_t)(&(((s *)0)->m))
#else
#  include <stddef.h>
#  define STRUCT_OFFSET(s,m)  offsetof(s,m)
#endif

/* ptrdiff_t is C11, so undef it under pedantic builds.  (Actually it is
 * in C89, but apparently there are platforms where it doesn't exist.  See
 * thread beginning at http://nntp.perl.org/group/perl.perl5.porters/251541.)
 * */
#ifdef PERL_GCC_PEDANTIC
#   undef HAS_PTRDIFF_T
#endif

#ifdef HAS_PTRDIFF_T
#  define Ptrdiff_t ptrdiff_t
#else
#  define Ptrdiff_t SSize_t
#endif

#  include <string.h>

/* This comes after <stdlib.h> so we don't try to change the standard
 * library prototypes; we'll use our own in proto.h instead. */

#ifdef MYMALLOC
#  ifdef PERL_POLLUTE_MALLOC
#   ifndef PERL_EXTMALLOC_DEF
#    define Perl_malloc		malloc
#    define Perl_calloc		calloc
#    define Perl_realloc	realloc
#    define Perl_mfree		free
#   endif
#  else
#    define EMBEDMYMALLOC	/* for compatibility */
#  endif

#  define safemalloc  Perl_malloc
#  define safecalloc  Perl_calloc
#  define saferealloc Perl_realloc
#  define safefree    Perl_mfree
#  define CHECK_MALLOC_TOO_LATE_FOR_(code)	STMT_START {		\
        if (!TAINTING_get && MallocCfg_ptr[MallocCfg_cfg_env_read])	\
                code;							\
    } STMT_END
#  define CHECK_MALLOC_TOO_LATE_FOR(ch)				\
        CHECK_MALLOC_TOO_LATE_FOR_(MALLOC_TOO_LATE_FOR(ch))
#  define panic_write2(s)		write(2, s, strlen(s))
#  define CHECK_MALLOC_TAINT(newval)				\
        CHECK_MALLOC_TOO_LATE_FOR_(				\
                if (newval) {					\
                  PERL_UNUSED_RESULT(panic_write2("panic: tainting with $ENV{PERL_MALLOC_OPT}\n"));\
                  exit(1); })
#  define MALLOC_CHECK_TAINT(argc,argv,env)                     \
    STMT_START {                                                \
        if (doing_taint(argc,argv,env)) {                       \
            MallocCfg_ptr[MallocCfg_skip_cfg_env] = 1;          \
        }                                                       \
    } STMT_END;
#else  /* MYMALLOC */
#  define safemalloc  safesysmalloc
#  define safecalloc  safesyscalloc
#  define saferealloc safesysrealloc
#  define safefree    safesysfree
#  define CHECK_MALLOC_TOO_LATE_FOR(ch)		((void)0)
#  define CHECK_MALLOC_TAINT(newval)		((void)0)
#  define MALLOC_CHECK_TAINT(argc,argv,env)
#endif /* MYMALLOC */

/* diag_listed_as: "-T" is on the #! line, it must also be used on the command line */
#define TOO_LATE_FOR_(ch,what)	Perl_croak(aTHX_ "\"-%c\" is on the #! line, it must also be used on the command line%s", (char)(ch), what)
#define TOO_LATE_FOR(ch)	TOO_LATE_FOR_(ch, "")
#define MALLOC_TOO_LATE_FOR(ch)	TOO_LATE_FOR_(ch, " with $ENV{PERL_MALLOC_OPT}")
#define MALLOC_CHECK_TAINT2(argc,argv)	MALLOC_CHECK_TAINT(argc,argv,NULL)

/*
=for apidoc Am|void|memzero|void * d|Size_t l
Set the C<l> bytes starting at C<*d> to all zeroes.

=cut
*/
#ifndef memzero
#   define memzero(d,l) memset(d,0,l)
#endif

#ifdef I_NETINET_IN
#   include <netinet/in.h>
#endif

#ifdef I_ARPA_INET
#   include <arpa/inet.h>
#endif

#ifdef I_SYS_STAT
#   include <sys/stat.h>
#endif

/* Microsoft VC's sys/stat.h defines all S_Ixxx macros except S_IFIFO.
   This definition should ideally go into win32/win32.h, but S_IFIFO is
   used later here in perl.h before win32/win32.h is being included. */
#if !defined(S_IFIFO) && defined(_S_IFIFO)
#   define S_IFIFO _S_IFIFO
#endif

/* The stat macros for Unisoft System V/88 (and derivatives
   like UTekV) are broken, sometimes giving false positives.  Undefine
   them here and let the code below set them to proper values.

   The ghs macro stands for GreenHills Software C-1.8.5 which
   is the C compiler for sysV88 and the various derivatives.
   This header file bug is corrected in gcc-2.5.8 and later versions.
   --Kaveh Ghazi (ghazi@noc.rutgers.edu) 10/3/94.  */

#if defined(m88k) && defined(ghs)
#   undef S_ISDIR
#   undef S_ISCHR
#   undef S_ISBLK
#   undef S_ISREG
#   undef S_ISFIFO
#   undef S_ISLNK
#endif

#include <time.h>

#ifdef I_SYS_TIME
#   ifdef I_SYS_TIME_KERNEL
#	define KERNEL
#   endif
#   include <sys/time.h>
#   ifdef I_SYS_TIME_KERNEL
#	undef KERNEL
#   endif
#endif

#if defined(HAS_TIMES) && defined(I_SYS_TIMES)
#    include <sys/times.h>
#endif

#include <errno.h>

#if defined(WIN32) && defined(PERL_IMPLICIT_SYS)
#  define WIN32SCK_IS_STDSCK		/* don't pull in custom wsock layer */
#endif

#if defined(HAS_SOCKET) && !defined(WIN32) /* WIN32 handles sockets via win32.h */
# include <sys/socket.h>
# if defined(USE_SOCKS) && defined(I_SOCKS)
#   if !defined(INCLUDE_PROTOTYPES)
#       define INCLUDE_PROTOTYPES /* for <socks.h> */
#       define PERL_SOCKS_NEED_PROTOTYPES
#   endif
#   include <socks.h>
#   ifdef PERL_SOCKS_NEED_PROTOTYPES /* keep cpp space clean */
#       undef INCLUDE_PROTOTYPES
#       undef PERL_SOCKS_NEED_PROTOTYPES
#   endif
# endif
# ifdef I_NETDB
#  include <netdb.h>
# endif
# ifndef ENOTSOCK
#  ifdef I_NET_ERRNO
#   include <net/errno.h>
#  endif
# endif
#endif

/* sockatmark() is so new (2001) that many places might have it hidden
 * behind some -D_BLAH_BLAH_SOURCE guard.  The __THROW magic is required
 * e.g. in Gentoo, see http://bugs.gentoo.org/show_bug.cgi?id=12605 */
#if defined(HAS_SOCKATMARK) && !defined(HAS_SOCKATMARK_PROTO)
# if defined(__THROW) && defined(__GLIBC__)
int sockatmark(int) __THROW;
# else
int sockatmark(int);
# endif
#endif

#if defined(__osf__) && defined(__cplusplus) && !defined(_XOPEN_SOURCE_EXTENDED) /* Tru64 "cxx" (C++), see hints/dec_osf.sh for why the _XOPEN_SOURCE_EXTENDED cannot be defined. */
EXTERN_C int fchdir(int);
EXTERN_C int flock(int, int);
EXTERN_C int fseeko(FILE *, off_t, int);
EXTERN_C off_t ftello(FILE *);
#endif

#if defined(__SUNPRO_CC) /* SUNWspro CC (C++) */
EXTERN_C char *crypt(const char *, const char *);
#endif

#if defined(__cplusplus) && defined(__CYGWIN__)
EXTERN_C char *crypt(const char *, const char *);
#endif

/*
=for apidoc_section $errno

=for apidoc m|void|SETERRNO|int errcode|int vmserrcode

Set C<errno>, and on VMS set C<vaxc$errno>.

=for apidoc mn|void|dSAVEDERRNO

Declare variables needed to save C<errno> and any operating system
specific error number.

=for apidoc mn|void|dSAVE_ERRNO

Declare variables needed to save C<errno> and any operating system
specific error number, and save them for optional later restoration
by C<RESTORE_ERRNO>.

=for apidoc mn|void|SAVE_ERRNO

Save C<errno> and any operating system specific error number for
optional later restoration by C<RESTORE_ERRNO>.  Requires
C<dSAVEDERRNO> or C<dSAVE_ERRNO> in scope.

=for apidoc mn|void|RESTORE_ERRNO

Restore C<errno> and any operating system specific error number that
was saved by C<dSAVE_ERRNO> or C<RESTORE_ERRNO>.

=cut
*/

#ifdef SETERRNO
# undef SETERRNO  /* SOCKS might have defined this */
#endif

#ifdef VMS
#   define SETERRNO(errcode,vmserrcode) \
        STMT_START {			\
            set_errno(errcode);		\
            set_vaxc_errno(vmserrcode);	\
        } STMT_END
#   define dSAVEDERRNO    int saved_errno; unsigned saved_vms_errno
#   define dSAVE_ERRNO    int saved_errno = errno; unsigned saved_vms_errno = vaxc$errno
#   define SAVE_ERRNO     ( saved_errno = errno, saved_vms_errno = vaxc$errno )
#   define RESTORE_ERRNO  SETERRNO(saved_errno, saved_vms_errno)

#   define LIB_INVARG 		LIB$_INVARG
#   define RMS_DIR    		RMS$_DIR
#   define RMS_FAC    		RMS$_FAC
#   define RMS_FEX    		RMS$_FEX
#   define RMS_FNF    		RMS$_FNF
#   define RMS_IFI    		RMS$_IFI
#   define RMS_ISI    		RMS$_ISI
#   define RMS_PRV    		RMS$_PRV
#   define SS_ACCVIO      	SS$_ACCVIO
#   define SS_DEVOFFLINE	SS$_DEVOFFLINE
#   define SS_IVCHAN  		SS$_IVCHAN
#   define SS_NORMAL  		SS$_NORMAL
#   define SS_NOPRIV  		SS$_NOPRIV
#   define SS_BUFFEROVF		SS$_BUFFEROVF
#else
#   define LIB_INVARG 		0
#   define RMS_DIR    		0
#   define RMS_FAC    		0
#   define RMS_FEX    		0
#   define RMS_FNF    		0
#   define RMS_IFI    		0
#   define RMS_ISI    		0
#   define RMS_PRV    		0
#   define SS_ACCVIO      	0
#   define SS_DEVOFFLINE	0
#   define SS_IVCHAN  		0
#   define SS_NORMAL  		0
#   define SS_NOPRIV  		0
#   define SS_BUFFEROVF		0
#endif

#ifdef WIN32
#   define dSAVEDERRNO  int saved_errno; DWORD saved_win32_errno
#   define dSAVE_ERRNO  int saved_errno = errno; DWORD saved_win32_errno = GetLastError()
#   define SAVE_ERRNO   ( saved_errno = errno, saved_win32_errno = GetLastError() )
#   define RESTORE_ERRNO ( errno = saved_errno, SetLastError(saved_win32_errno) )
#endif

#ifdef OS2
#   define dSAVEDERRNO  int saved_errno; unsigned long saved_os2_errno
#   define dSAVE_ERRNO  int saved_errno = errno; unsigned long saved_os2_errno = Perl_rc
#   define SAVE_ERRNO   ( saved_errno = errno, saved_os2_errno = Perl_rc )
#   define RESTORE_ERRNO ( errno = saved_errno, Perl_rc = saved_os2_errno )
#endif

#ifndef SETERRNO
#   define SETERRNO(errcode,vmserrcode) (errno = (errcode))
#endif

#ifndef dSAVEDERRNO
#   define dSAVEDERRNO    int saved_errno
#   define dSAVE_ERRNO    int saved_errno = errno
#   define SAVE_ERRNO     (saved_errno = errno)
#   define RESTORE_ERRNO  (errno = saved_errno)
#endif

/*
=for apidoc_section $warning

=for apidoc Amn|SV *|ERRSV

Returns the SV for C<$@>, creating it if needed.

=for apidoc Am|void|CLEAR_ERRSV

Clear the contents of C<$@>, setting it to the empty string.

This replaces any read-only SV with a fresh SV and removes any magic.

=for apidoc Am|void|SANE_ERRSV

Clean up ERRSV so we can safely set it.

This replaces any read-only SV with a fresh writable copy and removes
any magic.

=cut
*/

#define ERRSV GvSVn(PL_errgv)

/* contains inlined gv_add_by_type */
#define CLEAR_ERRSV() STMT_START {					\
    SV ** const svp = &GvSV(PL_errgv);					\
    if (!*svp) {							\
        *svp = newSVpvs("");                                            \
    } else if (SvREADONLY(*svp)) {					\
        SvREFCNT_dec_NN(*svp);						\
        *svp = newSVpvs("");						\
    } else {								\
        SV *const errsv = *svp;						\
        SvPVCLEAR(errsv);                                               \
        SvPOK_only(errsv);						\
        if (SvMAGICAL(errsv)) {						\
            mg_free(errsv);						\
        }								\
    }									\
    } STMT_END

/* contains inlined gv_add_by_type */
#define SANE_ERRSV() STMT_START {					\
    SV ** const svp = &GvSV(PL_errgv);					\
    if (!*svp) {							\
        *svp = newSVpvs("");                                            \
    } else if (SvREADONLY(*svp)) {					\
        SV *dupsv = newSVsv(*svp);					\
        SvREFCNT_dec_NN(*svp);						\
        *svp = dupsv;							\
    } else {								\
        SV *const errsv = *svp;						\
        if (SvMAGICAL(errsv)) {						\
            mg_free(errsv);						\
        }								\
    }									\
    } STMT_END


#ifdef PERL_CORE
# define DEFSV (0 + GvSVn(PL_defgv))
# define DEFSV_set(sv) \
    (SvREFCNT_dec(GvSV(PL_defgv)), GvSV(PL_defgv) = SvREFCNT_inc(sv))
# define SAVE_DEFSV                \
    (                               \
        save_gp(PL_defgv, 0),        \
        GvINTRO_off(PL_defgv),        \
        SAVEGENERICSV(GvSV(PL_defgv)), \
        GvSV(PL_defgv) = NULL           \
    )
#else
# define DEFSV GvSVn(PL_defgv)
# define DEFSV_set(sv) (GvSV(PL_defgv) = (sv))
# define SAVE_DEFSV SAVESPTR(GvSV(PL_defgv))
#endif

/*
=for apidoc_section $SV
=for apidoc Amn|SV *|DEFSV
Returns the SV associated with C<$_>

=for apidoc Am|void|DEFSV_set|SV * sv
Associate C<sv> with C<$_>

=for apidoc Amn|void|SAVE_DEFSV
Localize C<$_>.  See L<perlguts/Localizing changes>.

=cut
*/

#ifndef errno
        extern int errno;     /* ANSI allows errno to be an lvalue expr.
                               * For example in multithreaded environments
                               * something like this might happen:
                               * extern int *_errno(void);
                               * #define errno (*_errno()) */
#endif

#define UNKNOWN_ERRNO_MSG "(unknown)"

#ifdef VMS
#define Strerror(e) strerror((e), vaxc$errno)
#else
#define Strerror(e) strerror(e)
#endif

#ifdef I_SYS_IOCTL
#   ifndef _IOCTL_
#	include <sys/ioctl.h>
#   endif
#endif

#if defined(mc300) || defined(mc500) || defined(mc700) || defined(mc6000)
#   ifdef HAS_SOCKETPAIR
#	undef HAS_SOCKETPAIR
#   endif
#   ifdef I_NDBM
#	undef I_NDBM
#   endif
#endif

#ifndef HAS_SOCKETPAIR
#   ifdef HAS_SOCKET
#	define socketpair Perl_my_socketpair
#   endif
#endif

#if INTSIZE == 2
#   define htoni htons
#   define ntohi ntohs
#else
#   define htoni htonl
#   define ntohi ntohl
#endif

/* Configure already sets Direntry_t */
#if defined(I_DIRENT)
#  include <dirent.h>
#elif defined(I_SYS_NDIR)
#  include <sys/ndir.h>
#elif defined(I_SYS_DIR)
#  include <sys/dir.h>
#endif

/*
 * The following gobbledygook brought to you on behalf of __STDC__.
 * (I could just use #ifndef __STDC__, but this is more bulletproof
 * in the face of half-implementations.)
 */

#if defined(I_SYSMODE)
#include <sys/mode.h>
#endif

#ifndef S_IFMT
#   ifdef _S_IFMT
#	define S_IFMT _S_IFMT
#   else
#	define S_IFMT 0170000
#   endif
#endif

#ifndef S_ISDIR
#   define S_ISDIR(m) ((m & S_IFMT) == S_IFDIR)
#endif

#ifndef S_ISCHR
#   define S_ISCHR(m) ((m & S_IFMT) == S_IFCHR)
#endif

#ifndef S_ISBLK
#   ifdef S_IFBLK
#	define S_ISBLK(m) ((m & S_IFMT) == S_IFBLK)
#   else
#	define S_ISBLK(m) (0)
#   endif
#endif

#ifndef S_ISREG
#   define S_ISREG(m) ((m & S_IFMT) == S_IFREG)
#endif

#ifndef S_ISFIFO
#   ifdef S_IFIFO
#	define S_ISFIFO(m) ((m & S_IFMT) == S_IFIFO)
#   else
#	define S_ISFIFO(m) (0)
#   endif
#endif

#ifndef S_ISLNK
#  ifdef _S_ISLNK
#    define S_ISLNK(m) _S_ISLNK(m)
#  elif defined(_S_IFLNK)
#    define S_ISLNK(m) ((m & S_IFMT) == _S_IFLNK)
#  elif defined(S_IFLNK)
#    define S_ISLNK(m) ((m & S_IFMT) == S_IFLNK)
#  else
#    define S_ISLNK(m) (0)
#  endif
#endif

#ifndef S_ISSOCK
#  ifdef _S_ISSOCK
#    define S_ISSOCK(m) _S_ISSOCK(m)
#  elif defined(_S_IFSOCK)
#    define S_ISSOCK(m) ((m & S_IFMT) == _S_IFSOCK)
#  elif defined(S_IFSOCK)
#    define S_ISSOCK(m) ((m & S_IFMT) == S_IFSOCK)
#  else
#    define S_ISSOCK(m) (0)
#  endif
#endif

#ifndef S_IRUSR
#   ifdef S_IREAD
#	define S_IRUSR S_IREAD
#	define S_IWUSR S_IWRITE
#	define S_IXUSR S_IEXEC
#   else
#	define S_IRUSR 0400
#	define S_IWUSR 0200
#	define S_IXUSR 0100
#   endif
#endif

#ifndef S_IRGRP
#   ifdef S_IRUSR
#       define S_IRGRP (S_IRUSR>>3)
#       define S_IWGRP (S_IWUSR>>3)
#       define S_IXGRP (S_IXUSR>>3)
#   else
#       define S_IRGRP 0040
#       define S_IWGRP 0020
#       define S_IXGRP 0010
#   endif
#endif

#ifndef S_IROTH
#   ifdef S_IRUSR
#       define S_IROTH (S_IRUSR>>6)
#       define S_IWOTH (S_IWUSR>>6)
#       define S_IXOTH (S_IXUSR>>6)
#   else
#       define S_IROTH 0040
#       define S_IWOTH 0020
#       define S_IXOTH 0010
#   endif
#endif

#ifndef S_ISUID
#   define S_ISUID 04000
#endif

#ifndef S_ISGID
#   define S_ISGID 02000
#endif

#ifndef S_IRWXU
#   define S_IRWXU (S_IRUSR|S_IWUSR|S_IXUSR)
#endif

#ifndef S_IRWXG
#   define S_IRWXG (S_IRGRP|S_IWGRP|S_IXGRP)
#endif

#ifndef S_IRWXO
#   define S_IRWXO (S_IROTH|S_IWOTH|S_IXOTH)
#endif

/* Haiku R1 seems to define S_IREAD and S_IWRITE in <posix/fcntl.h>
 * which would get included through <sys/file.h >, but that is 3000
 * lines in the future.  --jhi */

#if !defined(S_IREAD) && !defined(__HAIKU__)
#   define S_IREAD S_IRUSR
#endif

#if !defined(S_IWRITE) && !defined(__HAIKU__)
#   define S_IWRITE S_IWUSR
#endif

#ifndef S_IEXEC
#   define S_IEXEC S_IXUSR
#endif

#if defined(cray) || defined(gould) || defined(i860) || defined(pyr)
#   define SLOPPYDIVIDE
#endif

#ifdef UV
#undef UV
#endif

/* This used to be conditionally defined based on whether we had a sprintf()
 * that correctly returns the string length (as required by C89), but we no
 * longer need that. XS modules can (and do) use this name, so it must remain
 * a part of the API that's visible to modules.

=for apidoc_section $string
=for apidoc ATmD|int|my_sprintf|NN char *buffer|NN const char *pat|...

Do NOT use this due to the possibility of overflowing C<buffer>.  Instead use
my_snprintf()

=cut
*/
#define my_sprintf sprintf

/*
 * If we have v?snprintf() and the C99 variadic macros, we can just
 * use just the v?snprintf().  It is nice to try to trap the buffer
 * overflow, however, so if we are DEBUGGING, and we cannot use the
 * gcc statement expressions, then use the function wrappers which try
 * to trap the overflow.  If we can use the gcc statement expressions,
 * we can try that even with the version that uses the C99 variadic
 * macros.
 */

/* Note that we do not check against snprintf()/vsnprintf() returning
 * negative values because that is non-standard behaviour and we use
 * snprintf/vsnprintf only iff HAS_VSNPRINTF has been defined, and
 * that should be true only if the snprintf()/vsnprintf() are true
 * to the standard. */

#define PERL_SNPRINTF_CHECK(len, max, api) STMT_START { if ((max) > 0 && (Size_t)len > (max)) Perl_croak_nocontext("panic: %s buffer overflow", STRINGIFY(api)); } STMT_END

#if defined(USE_LOCALE_NUMERIC) || defined(USE_QUADMATH)
#  define my_snprintf Perl_my_snprintf
#  define PERL_MY_SNPRINTF_GUARDED
#elif defined(HAS_SNPRINTF) && defined(HAS_C99_VARIADIC_MACROS) && !(defined(DEBUGGING) && !defined(PERL_USE_GCC_BRACE_GROUPS)) && !defined(PERL_GCC_PEDANTIC)
#  ifdef PERL_USE_GCC_BRACE_GROUPS
#      define my_snprintf(buffer, max, ...) ({ int len = snprintf(buffer, max, __VA_ARGS__); PERL_SNPRINTF_CHECK(len, max, snprintf); len; })
#      define PERL_MY_SNPRINTF_GUARDED
#  else
#    define my_snprintf(buffer, max, ...) snprintf(buffer, max, __VA_ARGS__)
#  endif
#else
#  define my_snprintf  Perl_my_snprintf
#  define PERL_MY_SNPRINTF_GUARDED
#endif

/* There is no quadmath_vsnprintf, and therefore my_vsnprintf()
 * dies if called under USE_QUADMATH. */
#if !  defined(USE_LOCALE_NUMERIC)                                  \
 &&    defined(HAS_VSNPRINTF)                                       \
 &&    defined(HAS_C99_VARIADIC_MACROS)                             \
 && ! (defined(DEBUGGING) && ! defined(PERL_USE_GCC_BRACE_GROUPS))  \
 && !  defined(PERL_GCC_PEDANTIC)
#  ifdef PERL_USE_GCC_BRACE_GROUPS
#      define my_vsnprintf(buffer, max, ...)                        \
            ({ int len = vsnprintf(buffer, max, __VA_ARGS__);       \
             PERL_SNPRINTF_CHECK(len, max, vsnprintf);              \
             len; })
#      define PERL_MY_VSNPRINTF_GUARDED
#  else
#    define my_vsnprintf(buffer, max, ...) vsnprintf(buffer, max, __VA_ARGS__)
#  endif
#else
#  define my_vsnprintf Perl_my_vsnprintf
#  define PERL_MY_VSNPRINTF_GUARDED
#endif

/* You will definitely need to use the PERL_MY_SNPRINTF_POST_GUARD()
 * or PERL_MY_VSNPRINTF_POST_GUARD() if you otherwise decide to ignore
 * the result of my_snprintf() or my_vsnprintf().  (No, you should not
 * completely ignore it: otherwise you cannot know whether your output
 * was too long.)
 *
 * int len = my_sprintf(buf, max, ...);
 * PERL_MY_SNPRINTF_POST_GUARD(len, max);
 *
 * The trick is that in certain platforms [a] the my_sprintf() already
 * contains the sanity check, while in certain platforms [b] it needs
 * to be done as a separate step.  The POST_GUARD is that step-- in [a]
 * platforms the POST_GUARD actually does nothing since the check has
 * already been done.  Watch out for the max being the same in both calls.
 *
 * If you actually use the snprintf/vsnprintf return value already,
 * you assumedly are checking its validity somehow.  But you can
 * insert the POST_GUARD() also in that case. */

#ifndef PERL_MY_SNPRINTF_GUARDED
#  define PERL_MY_SNPRINTF_POST_GUARD(len, max) PERL_SNPRINTF_CHECK(len, max, snprintf)
#else
#  define PERL_MY_SNPRINTF_POST_GUARD(len, max) PERL_UNUSED_VAR(len)
#endif

#ifndef  PERL_MY_VSNPRINTF_GUARDED
#  define PERL_MY_VSNPRINTF_POST_GUARD(len, max) PERL_SNPRINTF_CHECK(len, max, vsnprintf)
#else
#  define PERL_MY_VSNPRINTF_POST_GUARD(len, max) PERL_UNUSED_VAR(len)
#endif

#ifdef HAS_STRLCAT
#  define my_strlcat    strlcat
#endif

#if defined(PERL_CORE) || defined(PERL_EXT)
#  ifdef HAS_MEMRCHR
#    define my_memrchr	memrchr
#  else
#    define my_memrchr	S_my_memrchr
#  endif
#endif

#ifdef HAS_STRLCPY
#  define my_strlcpy	strlcpy
#endif

#ifdef HAS_STRNLEN
#  define my_strnlen	strnlen
#endif

/*
    The IV type is supposed to be long enough to hold any integral
    value or a pointer.
    --Andy Dougherty	August 1996
*/

typedef IVTYPE IV;
typedef UVTYPE UV;

#if defined(USE_64_BIT_INT) && defined(HAS_QUAD)
#  if QUADKIND == QUAD_IS_INT64_T && defined(INT64_MAX)
#    define IV_MAX ((IV)INT64_MAX)
#    define IV_MIN ((IV)INT64_MIN)
#    define UV_MAX ((UV)UINT64_MAX)
#    ifndef UINT64_MIN
#      define UINT64_MIN 0
#    endif
#    define UV_MIN ((UV)UINT64_MIN)
#  else
#    define IV_MAX PERL_QUAD_MAX
#    define IV_MIN PERL_QUAD_MIN
#    define UV_MAX PERL_UQUAD_MAX
#    define UV_MIN PERL_UQUAD_MIN
#  endif
#  define IV_IS_QUAD
#  define UV_IS_QUAD
#else
#  if defined(INT32_MAX) && IVSIZE == 4
#    define IV_MAX ((IV)INT32_MAX)
#    define IV_MIN ((IV)INT32_MIN)
#    ifndef UINT32_MAX_BROKEN /* e.g. HP-UX with gcc messes this up */
#        define UV_MAX ((UV)UINT32_MAX)
#    else
#        define UV_MAX ((UV)4294967295U)
#    endif
#    ifndef UINT32_MIN
#      define UINT32_MIN 0
#    endif
#    define UV_MIN ((UV)UINT32_MIN)
#  else
#    define IV_MAX PERL_LONG_MAX
#    define IV_MIN PERL_LONG_MIN
#    define UV_MAX PERL_ULONG_MAX
#    define UV_MIN PERL_ULONG_MIN
#  endif
#  if IVSIZE == 8
#    define IV_IS_QUAD
#    define UV_IS_QUAD
#    ifndef HAS_QUAD
#      define HAS_QUAD
#    endif
#  else
#    undef IV_IS_QUAD
#    undef UV_IS_QUAD
#if !defined(PERL_CORE)
/* We think that removing this decade-old undef this will cause too much
   breakage on CPAN for too little gain. (See RT #119753)
   However, we do need HAS_QUAD in the core for use by the drand48 code. */
#    undef HAS_QUAD
#endif
#  endif
#endif

#define Size_t_MAX (~(Size_t)0)
#define SSize_t_MAX (SSize_t)(~(Size_t)0 >> 1)

#define IV_DIG (BIT_DIGITS(IVSIZE * 8))
#define UV_DIG (BIT_DIGITS(UVSIZE * 8))

#ifndef NO_PERL_PRESERVE_IVUV
#define PERL_PRESERVE_IVUV	/* We like our integers to stay integers. */
#endif

/*
 *  The macros INT2PTR and NUM2PTR are (despite their names)
 *  bi-directional: they will convert int/float to or from pointers.
 *  However the conversion to int/float are named explicitly:
 *  PTR2IV, PTR2UV, PTR2NV.
 *
 *  For int conversions we do not need two casts if pointers are
 *  the same size as IV and UV.   Otherwise we need an explicit
 *  cast (PTRV) to avoid compiler warnings.
 *
 *  These are mentioned in perlguts
 */
#if (IVSIZE == PTRSIZE) && (UVSIZE == PTRSIZE)
#  define PTRV			UV
#  define INT2PTR(any,d)	(any)(d)
#elif PTRSIZE == LONGSIZE
#  define PTRV			unsigned long
#  define PTR2ul(p)		(unsigned long)(p)
#else
#  define PTRV			unsigned
#endif

#ifndef INT2PTR
#  define INT2PTR(any,d)	(any)(PTRV)(d)
#endif

#ifndef PTR2ul
#  define PTR2ul(p)	INT2PTR(unsigned long,p)
#endif

/*
=for apidoc_section $casting
=for apidoc Cyh|type|NUM2PTR|type|int value
You probably want to be using L<C</INT2PTR>> instead.

=cut
*/

#define NUM2PTR(any,d)	(any)(PTRV)(d)
#define PTR2IV(p)	INT2PTR(IV,p)
#define PTR2UV(p)	INT2PTR(UV,p)
#define PTR2NV(p)	NUM2PTR(NV,p)
#define PTR2nat(p)	(PTRV)(p)	/* pointer to integer of PTRSIZE */

/* According to strict ANSI C89 one cannot freely cast between
 * data pointers and function (code) pointers.  There are at least
 * two ways around this.  One (used below) is to do two casts,
 * first the other pointer to an (unsigned) integer, and then
 * the integer to the other pointer.  The other way would be
 * to use unions to "overlay" the pointers.  For an example of
 * the latter technique, see union dirpu in struct xpvio in sv.h.
 * The only feasible use is probably temporarily storing
 * function pointers in a data pointer (such as a void pointer). */

#define DPTR2FPTR(t,p) ((t)PTR2nat(p))	/* data pointer to function pointer */
#define FPTR2DPTR(t,p) ((t)PTR2nat(p))	/* function pointer to data pointer */

#ifdef USE_LONG_DOUBLE
#  if LONG_DOUBLESIZE == DOUBLESIZE
#    define LONG_DOUBLE_EQUALS_DOUBLE
#    undef USE_LONG_DOUBLE /* Ouch! */
#  endif
#endif

/* The following is all to get LDBL_DIG, in order to pick a nice
   default value for printing floating point numbers in Gconvert.
   (see config.h)
*/
#ifndef HAS_LDBL_DIG
#  if LONG_DOUBLESIZE == 10
#    define LDBL_DIG 18 /* assume IEEE */
#  elif LONG_DOUBLESIZE == 12
#    define LDBL_DIG 18 /* gcc? */
#  elif LONG_DOUBLESIZE == 16
#    define LDBL_DIG 33 /* assume IEEE */
#  elif LONG_DOUBLESIZE == DOUBLESIZE
#    define LDBL_DIG DBL_DIG /* bummer */
#  endif
#endif

/* On MS Windows,with 64-bit mingw-w64 compilers, we
   need to attend to a __float128 alignment issue if
   USE_QUADMATH is defined. Otherwise we simply:
   typedef NVTYPE NV
   32-bit mingw.org compilers might also require
   aligned(32) - at least that's what I found with my
   Math::Foat128 module. But this is as yet untested
   here, so no allowance is being made for mingw.org
   compilers at this stage. -- sisyphus January 2021
*/
#if (defined(USE_LONG_DOUBLE) || defined(USE_QUADMATH)) && defined(__MINGW64__)
   /* 64-bit build, mingw-w64 compiler only */
   typedef NVTYPE NV __attribute__ ((aligned(8)));
#else
   typedef NVTYPE NV;
#endif

#ifdef I_IEEEFP
#   include <ieeefp.h>
#endif

#if defined(__DECC) && defined(__osf__)
/* Also Tru64 cc has broken NaN comparisons. */
#  define NAN_COMPARE_BROKEN
#endif
#if defined(__sgi)
#  define NAN_COMPARE_BROKEN
#endif

#ifdef USE_LONG_DOUBLE
#   ifdef I_SUNMATH
#       include <sunmath.h>
#   endif
#   if defined(LDBL_DIG)
#       define NV_DIG LDBL_DIG
#       ifdef LDBL_MANT_DIG
#           define NV_MANT_DIG LDBL_MANT_DIG
#       endif
#       ifdef LDBL_MIN
#           define NV_MIN LDBL_MIN
#       endif
#       ifdef LDBL_MAX
#           define NV_MAX LDBL_MAX
#       endif
#       ifdef LDBL_MIN_EXP
#           define NV_MIN_EXP LDBL_MIN_EXP
#       endif
#       ifdef LDBL_MAX_EXP
#           define NV_MAX_EXP LDBL_MAX_EXP
#       endif
#       ifdef LDBL_MIN_10_EXP
#           define NV_MIN_10_EXP LDBL_MIN_10_EXP
#       endif
#       ifdef LDBL_MAX_10_EXP
#           define NV_MAX_10_EXP LDBL_MAX_10_EXP
#       endif
#       ifdef LDBL_EPSILON
#           define NV_EPSILON LDBL_EPSILON
#       endif
#       ifdef LDBL_MAX
#           define NV_MAX LDBL_MAX
/* Having LDBL_MAX doesn't necessarily mean that we have LDBL_MIN... -Allen */
#       elif defined(HUGE_VALL)
#           define NV_MAX HUGE_VALL
#       endif
#   endif
#   if defined(HAS_SQRTL)
#       define Perl_acos acosl
#       define Perl_asin asinl
#       define Perl_atan atanl
#       define Perl_atan2 atan2l
#       define Perl_ceil ceill
#       define Perl_cos cosl
#       define Perl_cosh coshl
#       define Perl_exp expl
#       define Perl_fabs fabsl
#       define Perl_floor floorl
#       define Perl_fmod fmodl
#       define Perl_log logl
#       define Perl_log10 log10l
#       define Perl_pow powl
#       define Perl_sin sinl
#       define Perl_sinh sinhl
#       define Perl_sqrt sqrtl
#       define Perl_tan tanl
#       define Perl_tanh tanhl
#   endif
/* e.g. libsunmath doesn't have modfl and frexpl as of mid-March 2000 */
#   ifndef Perl_modf
#       ifdef HAS_MODFL
#           define Perl_modf(x,y) modfl(x,y)
/* eg glibc 2.2 series seems to provide modfl on ppc and arm, but has no
   prototype in <math.h> */
#           ifndef HAS_MODFL_PROTO
EXTERN_C long double modfl(long double, long double *);
#	    endif
#       elif (defined(HAS_TRUNCL) || defined(HAS_AINTL)) && defined(HAS_COPYSIGNL)
        extern long double Perl_my_modfl(long double x, long double *ip);
#           define Perl_modf(x,y) Perl_my_modfl(x,y)
#       endif
#   endif
#   ifndef Perl_frexp
#       ifdef HAS_FREXPL
#           define Perl_frexp(x,y) frexpl(x,y)
#       elif defined(HAS_ILOGBL) && defined(HAS_SCALBNL)
extern long double Perl_my_frexpl(long double x, int *e);
#           define Perl_frexp(x,y) Perl_my_frexpl(x,y)
#       endif
#   endif
#   ifndef Perl_ldexp
#       ifdef HAS_LDEXPL
#           define Perl_ldexp(x, y) ldexpl(x,y)
#       elif defined(HAS_SCALBNL) && FLT_RADIX == 2
#           define Perl_ldexp(x,y) scalbnl(x,y)
#       endif
#   endif
#   ifndef Perl_isnan
#       if defined(HAS_ISNANL) && !(defined(isnan) && defined(HAS_C99))
#           define Perl_isnan(x) isnanl(x)
#       elif defined(__sgi) && defined(__c99)  /* XXX Configure test needed */
#           define Perl_isnan(x) isnan(x)
#       endif
#   endif
#   ifndef Perl_isinf
#       if defined(HAS_ISINFL) && !(defined(isinf) && defined(HAS_C99))
#           define Perl_isinf(x) isinfl(x)
#       elif defined(__sgi) && defined(__c99)  /* XXX Configure test needed */
#           define Perl_isinf(x) isinf(x)
#       elif defined(LDBL_MAX) && !defined(NAN_COMPARE_BROKEN)
#           define Perl_isinf(x) ((x) > LDBL_MAX || (x) < -LDBL_MAX)
#       endif
#   endif
#   ifndef Perl_isfinite
#       define Perl_isfinite(x) Perl_isfinitel(x)
#   endif
#elif defined(USE_QUADMATH) && defined(I_QUADMATH)
#   include <quadmath.h>
#   define NV_DIG FLT128_DIG
#   define NV_MANT_DIG FLT128_MANT_DIG
#   define NV_MIN FLT128_MIN
#   define NV_MAX FLT128_MAX
#   define NV_MIN_EXP FLT128_MIN_EXP
#   define NV_MAX_EXP FLT128_MAX_EXP
#   define NV_EPSILON FLT128_EPSILON
#   define NV_MIN_10_EXP FLT128_MIN_10_EXP
#   define NV_MAX_10_EXP FLT128_MAX_10_EXP
#   define Perl_acos acosq
#   define Perl_asin asinq
#   define Perl_atan atanq
#   define Perl_atan2 atan2q
#   define Perl_ceil ceilq
#   define Perl_cos cosq
#   define Perl_cosh coshq
#   define Perl_exp expq
#   define Perl_fabs fabsq
#   define Perl_floor floorq
#   define Perl_fmod fmodq
#   define Perl_log logq
#   define Perl_log10 log10q
#   define Perl_signbit signbitq
#   define Perl_pow powq
#   define Perl_sin sinq
#   define Perl_sinh sinhq
#   define Perl_sqrt sqrtq
#   define Perl_tan tanq
#   define Perl_tanh tanhq
#   define Perl_modf(x,y) modfq(x,y)
#   define Perl_frexp(x,y) frexpq(x,y)
#   define Perl_ldexp(x, y) ldexpq(x,y)
#   define Perl_isinf(x) isinfq(x)
#   define Perl_isnan(x) isnanq(x)
#   define Perl_isfinite(x) (!(isnanq(x) || isinfq(x)))
#   define Perl_fp_class(x) ((x) == 0.0Q ? 0 : isinfq(x) ? 3 : isnanq(x) ? 4 : PERL_ABS(x) < FLT128_MIN ? 2 : 1)
#   define Perl_fp_class_inf(x)    (Perl_fp_class(x) == 3)
#   define Perl_fp_class_nan(x)    (Perl_fp_class(x) == 4)
#   define Perl_fp_class_norm(x)   (Perl_fp_class(x) == 1)
#   define Perl_fp_class_denorm(x) (Perl_fp_class(x) == 2)
#   define Perl_fp_class_zero(x)   (Perl_fp_class(x) == 0)
#else
#   define NV_DIG DBL_DIG
#   define NV_MANT_DIG DBL_MANT_DIG
#   define NV_MIN DBL_MIN
#   define NV_MAX DBL_MAX
#   define NV_MIN_EXP DBL_MIN_EXP
#   define NV_MAX_EXP DBL_MAX_EXP
#   define NV_MIN_10_EXP DBL_MIN_10_EXP
#   define NV_MAX_10_EXP DBL_MAX_10_EXP
#   define NV_EPSILON DBL_EPSILON
#   define NV_MAX DBL_MAX
#   define NV_MIN DBL_MIN

/* These math interfaces are C89. */
#   define Perl_acos acos
#   define Perl_asin asin
#   define Perl_atan atan
#   define Perl_atan2 atan2
#   define Perl_ceil ceil
#   define Perl_cos cos
#   define Perl_cosh cosh
#   define Perl_exp exp
#   define Perl_fabs fabs
#   define Perl_floor floor
#   define Perl_fmod fmod
#   define Perl_log log
#   define Perl_log10 log10
#   define Perl_pow pow
#   define Perl_sin sin
#   define Perl_sinh sinh
#   define Perl_sqrt sqrt
#   define Perl_tan tan
#   define Perl_tanh tanh

#   define Perl_modf(x,y) modf(x,y)
#   define Perl_frexp(x,y) frexp(x,y)
#   define Perl_ldexp(x,y) ldexp(x,y)

#   ifndef Perl_isnan
#       ifdef HAS_ISNAN
#           define Perl_isnan(x) isnan(x)
#       endif
#   endif
#   ifndef Perl_isinf
#       if defined(HAS_ISINF)
#           define Perl_isinf(x) isinf(x)
#       elif defined(DBL_MAX) && !defined(NAN_COMPARE_BROKEN)
#           define Perl_isinf(x) ((x) > DBL_MAX || (x) < -DBL_MAX)
#       endif
#   endif
#   ifndef Perl_isfinite
#     ifdef HAS_ISFINITE
#       define Perl_isfinite(x) isfinite(x)
#     elif defined(HAS_FINITE)
#       define Perl_isfinite(x) finite(x)
#     endif
#   endif
#endif

/* fpclassify(): C99.  It is supposed to be a macro that switches on
* the sizeof() of its argument, so there's no need for e.g. fpclassifyl().*/
#if !defined(Perl_fp_class) && defined(HAS_FPCLASSIFY)
#    include <math.h>
#    if defined(FP_INFINITE) && defined(FP_NAN)
#        define Perl_fp_class(x)	fpclassify(x)
#        define Perl_fp_class_inf(x)	(Perl_fp_class(x)==FP_INFINITE)
#        define Perl_fp_class_nan(x)	(Perl_fp_class(x)==FP_NAN)
#        define Perl_fp_class_norm(x)	(Perl_fp_class(x)==FP_NORMAL)
#        define Perl_fp_class_denorm(x)	(Perl_fp_class(x)==FP_SUBNORMAL)
#        define Perl_fp_class_zero(x)	(Perl_fp_class(x)==FP_ZERO)
#    elif defined(FP_PLUS_INF) && defined(FP_QNAN)
/* Some versions of HP-UX (10.20) have (only) fpclassify() but which is
 * actually not the C99 fpclassify, with its own set of return defines. */
#        define Perl_fp_class(x)	fpclassify(x)
#        define Perl_fp_class_pinf(x)	(Perl_fp_class(x)==FP_PLUS_INF)
#        define Perl_fp_class_ninf(x)	(Perl_fp_class(x)==FP_MINUS_INF)
#        define Perl_fp_class_snan(x)	(Perl_fp_class(x)==FP_SNAN)
#        define Perl_fp_class_qnan(x)	(Perl_fp_class(x)==FP_QNAN)
#        define Perl_fp_class_pnorm(x)	(Perl_fp_class(x)==FP_PLUS_NORM)
#        define Perl_fp_class_nnorm(x)	(Perl_fp_class(x)==FP_MINUS_NORM)
#        define Perl_fp_class_pdenorm(x)	(Perl_fp_class(x)==FP_PLUS_DENORM)
#        define Perl_fp_class_ndenorm(x)	(Perl_fp_class(x)==FP_MINUS_DENORM)
#        define Perl_fp_class_pzero(x)	(Perl_fp_class(x)==FP_PLUS_ZERO)
#        define Perl_fp_class_nzero(x)	(Perl_fp_class(x)==FP_MINUS_ZERO)
#    else
#        undef Perl_fp_class /* Unknown set of defines */
#    endif
#endif

/* fp_classify(): Legacy: VMS, maybe Unicos? The values, however,
 * are identical to the C99 fpclassify(). */
#if !defined(Perl_fp_class) && defined(HAS_FP_CLASSIFY)
#    include <math.h>
#    ifdef __VMS
     /* FP_INFINITE and others are here rather than in math.h as C99 stipulates */
#        include <fp.h>
     /* oh, and the isnormal macro has a typo in it! */
#    undef isnormal
#    define isnormal(x) Perl_fp_class_norm(x)
#    endif
#    if defined(FP_INFINITE) && defined(FP_NAN)
#        define Perl_fp_class(x)	fp_classify(x)
#        define Perl_fp_class_inf(x)	(Perl_fp_class(x)==FP_INFINITE)
#        define Perl_fp_class_nan(x)	(Perl_fp_class(x)==FP_NAN)
#        define Perl_fp_class_norm(x)	(Perl_fp_class(x)==FP_NORMAL)
#        define Perl_fp_class_denorm(x)	(Perl_fp_class(x)==FP_SUBNORMAL)
#        define Perl_fp_class_zero(x)	(Perl_fp_class(x)==FP_ZERO)
#    else
#        undef Perl_fp_class /* Unknown set of defines */
#    endif
#endif

/* Feel free to check with me for the SGI manpages, SGI testing,
 * etcetera, if you want to try getting this to work with IRIX.
 *
 * - Allen <allens@cpan.org> */

/* fpclass(): SysV, at least Solaris and some versions of IRIX. */
#if !defined(Perl_fp_class) && (defined(HAS_FPCLASS)||defined(HAS_FPCLASSL))
/* Solaris and IRIX have fpclass/fpclassl, but they are using
 * an enum typedef, not cpp symbols, and Configure doesn't detect that.
 * Define some symbols also as cpp symbols so we can detect them. */
#    if defined(__sun) || defined(__sgi) /* XXX Configure test instead */
#     define FP_PINF FP_PINF
#     define FP_QNAN FP_QNAN
#    endif
#    include <math.h>
#    ifdef I_IEEEFP
#        include <ieeefp.h>
#    endif
#    ifdef I_FP
#        include <fp.h>
#    endif
#    if defined(USE_LONG_DOUBLE) && defined(HAS_FPCLASSL)
#        define Perl_fp_class(x)	fpclassl(x)
#    else
#        define Perl_fp_class(x)	fpclass(x)
#    endif
#    if defined(FP_CLASS_PINF) && defined(FP_CLASS_SNAN)
#        define Perl_fp_class_snan(x)	(Perl_fp_class(x)==FP_CLASS_SNAN)
#        define Perl_fp_class_qnan(x)	(Perl_fp_class(x)==FP_CLASS_QNAN)
#        define Perl_fp_class_ninf(x)	(Perl_fp_class(x)==FP_CLASS_NINF)
#        define Perl_fp_class_pinf(x)	(Perl_fp_class(x)==FP_CLASS_PINF)
#        define Perl_fp_class_nnorm(x)	(Perl_fp_class(x)==FP_CLASS_NNORM)
#        define Perl_fp_class_pnorm(x)	(Perl_fp_class(x)==FP_CLASS_PNORM)
#        define Perl_fp_class_ndenorm(x)	(Perl_fp_class(x)==FP_CLASS_NDENORM)
#        define Perl_fp_class_pdenorm(x)	(Perl_fp_class(x)==FP_CLASS_PDENORM)
#        define Perl_fp_class_nzero(x)	(Perl_fp_class(x)==FP_CLASS_NZERO)
#        define Perl_fp_class_pzero(x)	(Perl_fp_class(x)==FP_CLASS_PZERO)
#    elif defined(FP_PINF) && defined(FP_QNAN)
#        define Perl_fp_class_snan(x)	(Perl_fp_class(x)==FP_SNAN)
#        define Perl_fp_class_qnan(x)	(Perl_fp_class(x)==FP_QNAN)
#        define Perl_fp_class_ninf(x)	(Perl_fp_class(x)==FP_NINF)
#        define Perl_fp_class_pinf(x)	(Perl_fp_class(x)==FP_PINF)
#        define Perl_fp_class_nnorm(x)	(Perl_fp_class(x)==FP_NNORM)
#        define Perl_fp_class_pnorm(x)	(Perl_fp_class(x)==FP_PNORM)
#        define Perl_fp_class_ndenorm(x)	(Perl_fp_class(x)==FP_NDENORM)
#        define Perl_fp_class_pdenorm(x)	(Perl_fp_class(x)==FP_PDENORM)
#        define Perl_fp_class_nzero(x)	(Perl_fp_class(x)==FP_NZERO)
#        define Perl_fp_class_pzero(x)	(Perl_fp_class(x)==FP_PZERO)
#    else
#        undef Perl_fp_class /* Unknown set of defines */
#    endif
#endif

/* fp_class(): Legacy: at least Tru64, some versions of IRIX. */
#if !defined(Perl_fp_class) && (defined(HAS_FP_CLASS)||defined(HAS_FP_CLASSL))
#    include <math.h>
#    if !defined(FP_SNAN) && defined(I_FP_CLASS)
#        include <fp_class.h>
#    endif
#    if defined(FP_POS_INF) && defined(FP_QNAN)
#        ifdef __sgi /* XXX Configure test instead */
#            ifdef USE_LONG_DOUBLE
#                define Perl_fp_class(x)	fp_class_l(x)
#            else
#                define Perl_fp_class(x)	fp_class_d(x)
#            endif
#        else
#            if defined(USE_LONG_DOUBLE) && defined(HAS_FP_CLASSL)
#                define Perl_fp_class(x)	fp_classl(x)
#            else
#                define Perl_fp_class(x)	fp_class(x)
#            endif
#        endif
#        if defined(FP_POS_INF) && defined(FP_QNAN)
#            define Perl_fp_class_snan(x)	(Perl_fp_class(x)==FP_SNAN)
#            define Perl_fp_class_qnan(x)	(Perl_fp_class(x)==FP_QNAN)
#            define Perl_fp_class_ninf(x)	(Perl_fp_class(x)==FP_NEG_INF)
#            define Perl_fp_class_pinf(x)	(Perl_fp_class(x)==FP_POS_INF)
#            define Perl_fp_class_nnorm(x)	(Perl_fp_class(x)==FP_NEG_NORM)
#            define Perl_fp_class_pnorm(x)	(Perl_fp_class(x)==FP_POS_NORM)
#            define Perl_fp_class_ndenorm(x)	(Perl_fp_class(x)==FP_NEG_DENORM)
#            define Perl_fp_class_pdenorm(x)	(Perl_fp_class(x)==FP_POS_DENORM)
#            define Perl_fp_class_nzero(x)	(Perl_fp_class(x)==FP_NEG_ZERO)
#            define Perl_fp_class_pzero(x)	(Perl_fp_class(x)==FP_POS_ZERO)
#        else
#            undef Perl_fp_class /* Unknown set of defines */
#        endif
#    endif
#endif

/* class(), _class(): Legacy: AIX. */
#if !defined(Perl_fp_class) && defined(HAS_CLASS)
#    include <math.h>
#    if defined(FP_PLUS_NORM) && defined(FP_PLUS_INF)
#        ifndef _cplusplus
#            define Perl_fp_class(x)	class(x)
#        else
#            define Perl_fp_class(x)	_class(x)
#        endif
#        if defined(FP_PLUS_INF) && defined(FP_NANQ)
#            define Perl_fp_class_snan(x)	(Perl_fp_class(x)==FP_NANS)
#            define Perl_fp_class_qnan(x)	(Perl_fp_class(x)==FP_NANQ)
#            define Perl_fp_class_ninf(x)	(Perl_fp_class(x)==FP_MINUS_INF)
#            define Perl_fp_class_pinf(x)	(Perl_fp_class(x)==FP_PLUS_INF)
#            define Perl_fp_class_nnorm(x)	(Perl_fp_class(x)==FP_MINUS_NORM)
#            define Perl_fp_class_pnorm(x)	(Perl_fp_class(x)==FP_PLUS_NORM)
#            define Perl_fp_class_ndenorm(x)	(Perl_fp_class(x)==FP_MINUS_DENORM)
#            define Perl_fp_class_pdenorm(x)	(Perl_fp_class(x)==FP_PLUS_DENORM)
#            define Perl_fp_class_nzero(x)	(Perl_fp_class(x)==FP_MINUS_ZERO)
#            define Perl_fp_class_pzero(x)	(Perl_fp_class(x)==FP_PLUS_ZERO)
#        else
#            undef Perl_fp_class /* Unknown set of defines */
#        endif
#    endif
#endif

/* Win32: _fpclass(), _isnan(), _finite(). */
#ifdef _MSC_VER
#  ifndef Perl_isnan
#    define Perl_isnan(x) _isnan(x)
#  endif
#  ifndef Perl_isfinite
#    define Perl_isfinite(x) _finite(x)
#  endif
#  ifndef Perl_fp_class_snan
/* No simple way to #define Perl_fp_class because _fpclass()
 * returns a set of bits. */
#    define Perl_fp_class_snan(x) (_fpclass(x) & _FPCLASS_SNAN)
#    define Perl_fp_class_qnan(x) (_fpclass(x) & _FPCLASS_QNAN)
#    define Perl_fp_class_nan(x) (_fpclass(x) & (_FPCLASS_SNAN|_FPCLASS_QNAN))
#    define Perl_fp_class_ninf(x) (_fpclass(x) & _FPCLASS_NINF)
#    define Perl_fp_class_pinf(x) (_fpclass(x) & _FPCLASS_PINF)
#    define Perl_fp_class_inf(x) (_fpclass(x) & (_FPCLASS_NINF|_FPCLASS_PINF))
#    define Perl_fp_class_nnorm(x) (_fpclass(x) & _FPCLASS_NN)
#    define Perl_fp_class_pnorm(x) (_fpclass(x) & _FPCLASS_PN)
#    define Perl_fp_class_norm(x) (_fpclass(x) & (_FPCLASS_NN|_FPCLASS_PN))
#    define Perl_fp_class_ndenorm(x) (_fpclass(x) & _FPCLASS_ND)
#    define Perl_fp_class_pdenorm(x) (_fpclass(x) & _FPCLASS_PD)
#    define Perl_fp_class_denorm(x) (_fpclass(x) & (_FPCLASS_ND|_FPCLASS_PD))
#    define Perl_fp_class_nzero(x) (_fpclass(x) & _FPCLASS_NZ)
#    define Perl_fp_class_pzero(x) (_fpclass(x) & _FPCLASS_PZ)
#    define Perl_fp_class_zero(x) (_fpclass(x) & (_FPCLASS_NZ|_FPCLASS_PZ))
#  endif
#endif

#if !defined(Perl_fp_class_inf) && \
  defined(Perl_fp_class_pinf) && defined(Perl_fp_class_ninf)
#  define Perl_fp_class_inf(x) \
    (Perl_fp_class_pinf(x) || Perl_fp_class_ninf(x))
#endif

#if !defined(Perl_fp_class_nan) && \
  defined(Perl_fp_class_snan) && defined(Perl_fp_class_qnan)
#  define Perl_fp_class_nan(x) \
    (Perl_fp_class_snan(x) || Perl_fp_class_qnan(x))
#endif

#if !defined(Perl_fp_class_zero) && \
  defined(Perl_fp_class_pzero) && defined(Perl_fp_class_nzero)
#  define Perl_fp_class_zero(x) \
    (Perl_fp_class_pzero(x) || Perl_fp_class_nzero(x))
#endif

#if !defined(Perl_fp_class_norm) && \
  defined(Perl_fp_class_pnorm) && defined(Perl_fp_class_nnorm)
#  define Perl_fp_class_norm(x) \
    (Perl_fp_class_pnorm(x) || Perl_fp_class_nnorm(x))
#endif

#if !defined(Perl_fp_class_denorm) && \
  defined(Perl_fp_class_pdenorm) && defined(Perl_fp_class_ndenorm)
#  define Perl_fp_class_denorm(x) \
    (Perl_fp_class_pdenorm(x) || Perl_fp_class_ndenorm(x))
#endif

#ifndef Perl_isnan
#   ifdef Perl_fp_class_nan
#       define Perl_isnan(x) Perl_fp_class_nan(x)
#   elif defined(HAS_UNORDERED)
#       define Perl_isnan(x) unordered((x), 0.0)
#   else
#       define Perl_isnan(x) ((x)!=(x))
#   endif
#endif

#ifndef Perl_isinf
#   ifdef Perl_fp_class_inf
#       define Perl_isinf(x) Perl_fp_class_inf(x)
#   endif
#endif

#ifndef Perl_isfinite
#   if defined(HAS_ISFINITE) && !defined(isfinite)
#     define Perl_isfinite(x) isfinite((double)(x))
#   elif defined(HAS_FINITE)
#       define Perl_isfinite(x) finite((double)(x))
#   elif defined(Perl_fp_class_finite)
#     define Perl_isfinite(x) Perl_fp_class_finite(x)
#   else
/* For the infinities the multiplication returns nan,
 * for the nan the multiplication also returns nan,
 * for everything else (that is, finite) zero should be returned. */
#     define Perl_isfinite(x) (((x) * 0) == 0)
#   endif
#endif

#ifndef Perl_isinf
#   if defined(Perl_isfinite) && defined(Perl_isnan)
#       define Perl_isinf(x) (!(Perl_isfinite(x)||Perl_isnan(x)))
#   endif
#endif

/* We need Perl_isfinitel (ends with ell) (if available) even when
 * not USE_LONG_DOUBLE because the printf code (sv_catpvfn_flags)
 * needs that. */
#if defined(HAS_LONG_DOUBLE) && !defined(Perl_isfinitel)
/* If isfinite() is a macro and looks like we have C99,
 * we assume it's the type-aware C99 isfinite(). */
#    if defined(HAS_ISFINITE) && defined(isfinite) && defined(HAS_C99)
#        define Perl_isfinitel(x) isfinite(x)
#    elif defined(HAS_ISFINITEL)
#        define Perl_isfinitel(x) isfinitel(x)
#    elif defined(HAS_FINITEL)
#        define Perl_isfinitel(x) finitel(x)
#    elif defined(HAS_ISINFL) && defined(HAS_ISNANL)
#        define Perl_isfinitel(x) (!(isinfl(x)||isnanl(x)))
#    else
#        define Perl_isfinitel(x) ((x) * 0 == 0)  /* See Perl_isfinite. */
#    endif
#endif

/* The default is to use Perl's own atof() implementation (in numeric.c).
 * This knows about if 'use locale' is in effect or not, and handles the radix
 * character accordingly.  On some platforms (e.g. UNICOS) it is however best
 * to use the native implementation of atof, as long as you accept that the
 * current underlying locale will affect the radix character.  Perl's version
 * uses a dot for a radix, execpt within the lexical scope of a Perl C<use
 * locale> statement.
 *
 * You can experiment with using your native one by -DUSE_PERL_ATOF=0.
 * Some good tests to try out with either setting are t/base/num.t,
 * t/op/numconvert.t, and t/op/pack.t. Note that if using long doubles
 * you may need to be using a different function than atof! */

#ifndef USE_PERL_ATOF
#   ifndef _UNICOS
#       define USE_PERL_ATOF
#   endif
#else
#   if USE_PERL_ATOF == 0
#       undef USE_PERL_ATOF
#   endif
#endif

#ifdef USE_PERL_ATOF
#   define Perl_atof(s) Perl_my_atof(aTHX_ s)
#   define Perl_atof2(s, n) Perl_my_atof3(aTHX_ (s), &(n), 0)
#else
#   define Perl_atof(s) (NV)atof(s)
#   define Perl_atof2(s, n) ((n) = atof(s))
#endif
#define my_atof2(a,b) my_atof3(a,b,0)

/*
=for apidoc AmTR|NV|Atof|NN const char * const s

This is a synonym for L</C<my_atof>>.

=cut

*/

#define Atof				my_atof

/*
=for apidoc_section $numeric
=for apidoc   AmT|NV|Perl_acos|NV x
=for apidoc_item |NV|Perl_asin|NV x
=for apidoc_item |NV|Perl_atan|NV x
=for apidoc_item |NV|Perl_atan2|NV x|NV y
=for apidoc_item |NV|Perl_ceil|NV x
=for apidoc_item |NV|Perl_cos|NV x
=for apidoc_item |NV|Perl_cosh|NV x
=for apidoc_item |NV|Perl_exp|NV x
=for apidoc_item |NV|Perl_floor|NV x
=for apidoc_item |NV|Perl_fmod|NV x|NV y
=for apidoc_item |NV|Perl_frexp|NV x|int *exp
=for apidoc_item |IV|Perl_isfinite|NV x
=for apidoc_item |IV|Perl_isinf|NV x
=for apidoc_item |IV|Perl_isnan|NV x
=for apidoc_item |NV|Perl_ldexp|NV x|int exp
=for apidoc_item |NV|Perl_log|NV x
=for apidoc_item |NV|Perl_log10|NV x
=for apidoc_item |NV|Perl_modf|NV x|NV *iptr
=for apidoc_item |NV|Perl_pow|NV x|NV y
=for apidoc_item |NV|Perl_sin|NV x
=for apidoc_item |NV|Perl_sinh|NV x
=for apidoc_item |NV|Perl_sqrt|NV x
=for apidoc_item |NV|Perl_tan|NV x
=for apidoc_item |NV|Perl_tanh|NV x

These perform the corresponding mathematical operation on the operand(s), using
the libc function designed for the task that has just enough precision for an
NV on this platform.  If no such function with sufficient precision exists,
the highest precision one available is used.

=cut

*/

/*
 * CHAR_MIN and CHAR_MAX are not included here, as the (char) type may be
 * ambiguous. It may be equivalent to (signed char) or (unsigned char)
 * depending on local options. Until Configure detects this (or at least
 * detects whether the "signed" keyword is available) the CHAR ranges
 * will not be included. UCHAR functions normally.
 *                                                           - kja
 */

#define PERL_UCHAR_MIN ((unsigned char)0)
#define PERL_UCHAR_MAX ((unsigned char)UCHAR_MAX)

#define PERL_USHORT_MIN ((unsigned short)0)
#define PERL_USHORT_MAX ((unsigned short)USHRT_MAX)

#define PERL_SHORT_MAX ((short)SHRT_MAX)
#define PERL_SHORT_MIN ((short)SHRT_MIN)

#define PERL_UINT_MAX ((unsigned int)UINT_MAX)
#define PERL_UINT_MIN ((unsigned int)0)

#define PERL_INT_MAX ((int)INT_MAX)
#define PERL_INT_MIN ((int)INT_MIN)

#define PERL_ULONG_MAX ((unsigned long)ULONG_MAX)
#define PERL_ULONG_MIN ((unsigned long)0L)

#define PERL_LONG_MAX ((long)LONG_MAX)
#define PERL_LONG_MIN ((long)LONG_MIN)

#ifdef UV_IS_QUAD
#    define PERL_UQUAD_MAX	(~(UV)0)
#    define PERL_UQUAD_MIN	((UV)0)
#    define PERL_QUAD_MAX 	((IV) (PERL_UQUAD_MAX >> 1))
#    define PERL_QUAD_MIN 	(-PERL_QUAD_MAX - ((3 & -1) == 3))
#endif

/*
=for apidoc_section $integer

=for apidoc Amn |int|PERL_INT_MAX
=for apidoc_item |int|PERL_INT_MIN
=for apidoc_item |long|PERL_LONG_MAX
=for apidoc_item |long|PERL_LONG_MIN
=for apidoc_item |IV|PERL_QUAD_MAX
=for apidoc_item |IV|PERL_QUAD_MIN
=for apidoc_item |short|PERL_SHORT_MAX
=for apidoc_item |short|PERL_SHORT_MIN
=for apidoc_item |U8|PERL_UCHAR_MAX
=for apidoc_item |U8|PERL_UCHAR_MIN
=for apidoc_item |unsigned int|PERL_UINT_MAX
=for apidoc_item |unsigned int|PERL_UINT_MIN
=for apidoc_item |unsigned long|PERL_ULONG_MAX
=for apidoc_item |unsigned long|PERL_ULONG_MIN
=for apidoc_item |UV|PERL_UQUAD_MAX
=for apidoc_item |UV|PERL_UQUAD_MIN
=for apidoc_item |unsigned short|PERL_USHORT_MAX
=for apidoc_item |unsigned short|PERL_USHORT_MIN

These give the largest and smallest number representable in the current
platform in variables of the corresponding types.

For signed types, the smallest representable number is the most negative
number, the one furthest away from zero.

For C99 and later compilers, these correspond to things like C<INT_MAX>, which
are available to the C code.  But these constants, furnished by Perl,
allow code compiled on earlier compilers to portably have access to the same
constants.

=cut

*/

typedef MEM_SIZE STRLEN;

typedef struct op OP;
typedef struct cop COP;
typedef struct unop UNOP;
typedef struct unop_aux UNOP_AUX;
typedef struct binop BINOP;
typedef struct listop LISTOP;
typedef struct logop LOGOP;
typedef struct pmop PMOP;
typedef struct svop SVOP;
typedef struct padop PADOP;
typedef struct pvop PVOP;
typedef struct loop LOOP;
typedef struct methop METHOP;

#ifdef PERL_CORE
typedef struct opslab OPSLAB;
typedef struct opslot OPSLOT;
#endif

typedef struct block_hooks BHK;
typedef struct custom_op XOP;

typedef struct interpreter PerlInterpreter;

/* SGI's <sys/sema.h> has struct sv */
#if defined(__sgi)
#   define STRUCT_SV perl_sv
#else
#   define STRUCT_SV sv
#endif
typedef struct STRUCT_SV SV;
typedef struct av AV;
typedef struct hv HV;
typedef struct cv CV;
typedef struct p5rx REGEXP;
typedef struct gp GP;
typedef struct gv GV;
typedef struct io IO;
typedef struct context PERL_CONTEXT;
typedef struct block BLOCK;
typedef struct invlist INVLIST;

typedef struct magic MAGIC;
typedef struct xpv XPV;
typedef struct xpviv XPVIV;
typedef struct xpvuv XPVUV;
typedef struct xpvnv XPVNV;
typedef struct xpvmg XPVMG;
typedef struct xpvlv XPVLV;
typedef struct xpvinvlist XINVLIST;
typedef struct xpvav XPVAV;
typedef struct xpvhv XPVHV;
typedef struct xpvgv XPVGV;
typedef struct xpvcv XPVCV;
typedef struct xpvbm XPVBM;
typedef struct xpvfm XPVFM;
typedef struct xpvio XPVIO;
typedef struct xobject XPVOBJ;
typedef struct mgvtbl MGVTBL;
typedef union any ANY;
typedef struct ptr_tbl_ent PTR_TBL_ENT_t;
typedef struct ptr_tbl PTR_TBL_t;
typedef struct clone_params CLONE_PARAMS;

/* a pad is currently just an AV; but that might change,
 * so hide the type.  */
typedef struct padlist PADLIST;
typedef AV PAD;
typedef struct padnamelist PADNAMELIST;
typedef struct padname PADNAME;

/* always enable PERL_OP_PARENT  */
#if !defined(PERL_OP_PARENT)
#  define PERL_OP_PARENT
#endif

/* enable PERL_COPY_ON_WRITE by default */
#if !defined(PERL_COPY_ON_WRITE) && !defined(PERL_NO_COW)
#  define PERL_COPY_ON_WRITE
#endif

#ifdef PERL_COPY_ON_WRITE
#  define PERL_ANY_COW
#else
# define PERL_SAWAMPERSAND
#endif

#if defined(PERL_DEBUG_READONLY_OPS) && !defined(USE_ITHREADS)
# error PERL_DEBUG_READONLY_OPS only works with ithreads
#endif

#include "handy.h"
#include "charclass_invlists.h"

#if defined(USE_LARGE_FILES) && !defined(NO_64_BIT_RAWIO)
#   if LSEEKSIZE == 8 && !defined(USE_64_BIT_RAWIO)
#       define USE_64_BIT_RAWIO	/* implicit */
#   endif
#endif

/* Notice the use of HAS_FSEEKO: now we are obligated to always use
 * fseeko/ftello if possible.  Don't go #defining ftell to ftello yourself,
 * however, because operating systems like to do that themself. */
#ifndef FSEEKSIZE
#   ifdef HAS_FSEEKO
#       define FSEEKSIZE LSEEKSIZE
#   else
#       define FSEEKSIZE LONGSIZE
#   endif
#endif

#if defined(USE_LARGE_FILES) && !defined(NO_64_BIT_STDIO)
#   if FSEEKSIZE == 8 && !defined(USE_64_BIT_STDIO)
#       define USE_64_BIT_STDIO /* implicit */
#   endif
#endif

#ifdef USE_64_BIT_RAWIO
#   ifdef HAS_OFF64_T
#       undef Off_t
#       define Off_t off64_t
#       undef LSEEKSIZE
#       define LSEEKSIZE 8
#   endif
/* Most 64-bit environments have defines like _LARGEFILE_SOURCE that
 * will trigger defines like the ones below.  Some 64-bit environments,
 * however, do not.  Therefore we have to explicitly mix and match. */
#   if defined(USE_OPEN64)
#       define open open64
#   endif
#   if defined(USE_LSEEK64)
#       define lseek lseek64
#   else
#       if defined(USE_LLSEEK)
#           define lseek llseek
#       endif
#   endif
#   if defined(USE_STAT64)
#       define stat stat64
#   endif
#   if defined(USE_FSTAT64)
#       define fstat fstat64
#   endif
#   if defined(USE_LSTAT64)
#       define lstat lstat64
#   endif
#   if defined(USE_FLOCK64)
#       define flock flock64
#   endif
#   if defined(USE_LOCKF64)
#       define lockf lockf64
#   endif
#   if defined(USE_FCNTL64)
#       define fcntl fcntl64
#   endif
#   if defined(USE_TRUNCATE64)
#       define truncate truncate64
#   endif
#   if defined(USE_FTRUNCATE64)
#       define ftruncate ftruncate64
#   endif
#endif

#ifdef USE_64_BIT_STDIO
#   ifdef HAS_FPOS64_T
#       undef Fpos_t
#       define Fpos_t fpos64_t
#   endif
/* Most 64-bit environments have defines like _LARGEFILE_SOURCE that
 * will trigger defines like the ones below.  Some 64-bit environments,
 * however, do not. */
#   if defined(USE_FOPEN64)
#       define fopen fopen64
#   endif
#   if defined(USE_FSEEK64)
#       define fseek fseek64 /* don't do fseeko here, see perlio.c */
#   endif
#   if defined(USE_FTELL64)
#       define ftell ftell64 /* don't do ftello here, see perlio.c */
#   endif
#   if defined(USE_FSETPOS64)
#       define fsetpos fsetpos64
#   endif
#   if defined(USE_FGETPOS64)
#       define fgetpos fgetpos64
#   endif
#   if defined(USE_TMPFILE64)
#       define tmpfile tmpfile64
#   endif
#   if defined(USE_FREOPEN64)
#       define freopen freopen64
#   endif
#endif

#if defined(OS2)
#  include "iperlsys.h"
#endif

#ifdef DOSISH
#   if defined(OS2)
#       include "os2ish.h"
#   else
#       include "dosish.h"
#   endif
#elif defined(VMS)
#   include "vmsish.h"
#elif defined(PLAN9)
#   include "./plan9/plan9ish.h"
#elif defined(__VOS__)
#   ifdef __GNUC__
#     include "./vos/vosish.h"
#   else
#     include "vos/vosish.h"
#   endif
#elif defined(__HAIKU__)
#   include "haiku/haikuish.h"
#else
#   include "unixish.h"
#endif

#ifdef __amigaos4__
#    include "amigaos.h"
#    undef FD_CLOEXEC /* a lie in AmigaOS */
#endif

/* NSIG logic from Configure --> */
#ifndef NSIG
#  ifdef _NSIG
#    define NSIG (_NSIG)
#  elif defined(SIGMAX)
#    define NSIG (SIGMAX+1)
#  elif defined(SIG_MAX)
#    define NSIG (SIG_MAX+1)
#  elif defined(_SIG_MAX)
#    define NSIG (_SIG_MAX+1)
#  elif defined(MAXSIG)
#    define NSIG (MAXSIG+1)
#  elif defined(MAX_SIG)
#    define NSIG (MAX_SIG+1)
#  elif defined(SIGARRAYSIZE)
#    define NSIG SIGARRAYSIZE /* Assume ary[SIGARRAYSIZE] */
#  elif defined(_sys_nsig)
#    define NSIG (_sys_nsig) /* Solaris 2.5 */
#  else
     /* Default to some arbitrary number that's big enough to get most
      * of the common signals.  */
#    define NSIG 50
#  endif
#endif
/* <-- NSIG logic from Configure */

#ifndef NO_ENVIRON_ARRAY
#  define USE_ENVIRON_ARRAY
#endif

#if defined(HAS_SIGACTION) && defined(SA_SIGINFO)
    /* having sigaction(2) means that the OS supports both 1-arg and 3-arg
     * signal handlers. But the perl core itself only fully supports 1-arg
     * handlers, so don't enable for now.
     * NB: POSIX::sigaction() supports both.
     *
     * # define PERL_USE_3ARG_SIGHANDLER
     */
#endif

/* Siginfo_t:
 * This is an alias for the OS's siginfo_t, except that where the OS
 * doesn't support it, declare a dummy version instead. This allows us to
 * have signal handler functions which always have a Siginfo_t parameter
 * regardless of platform, (and which will just be passed a NULL value
 * where the OS doesn't support HAS_SIGACTION).
 */

#if defined(HAS_SIGACTION) && defined(SA_SIGINFO)
    typedef siginfo_t Siginfo_t;
#else
#ifdef si_signo /* minix */
#undef si_signo
#endif
    typedef struct {
        int si_signo;
    } Siginfo_t;
#endif


/*
 * initialise to avoid floating-point exceptions from overflow, etc
 */
#ifndef PERL_FPU_INIT
#  ifdef HAS_FPSETMASK
#    if HAS_FLOATINGPOINT_H
#      include <floatingpoint.h>
#    endif
/* Some operating systems have this as a macro, which in turn expands to a comma
   expression, and the last sub-expression is something that gets calculated,
   and then they have the gall to warn that a value computed is not used. Hence
   cast to void.  */
#    define PERL_FPU_INIT (void)fpsetmask(0)
#  elif defined(SIGFPE) && defined(SIG_IGN) && !defined(PERL_MICRO)
#    define PERL_FPU_INIT       PL_sigfpe_saved = (Sighandler_t) signal(SIGFPE, SIG_IGN)
#    define PERL_FPU_PRE_EXEC   { Sigsave_t xfpe; rsignal_save(SIGFPE, PL_sigfpe_saved, &xfpe);
#    define PERL_FPU_POST_EXEC    rsignal_restore(SIGFPE, &xfpe); }
#  else
#    define PERL_FPU_INIT
#  endif
#endif
#ifndef PERL_FPU_PRE_EXEC
#  define PERL_FPU_PRE_EXEC   {
#  define PERL_FPU_POST_EXEC  }
#endif

/* In Tru64 the cc -ieee enables the IEEE math but disables traps.
 * We need to reenable the "invalid" trap because otherwise generation
 * of NaN values leaves the IEEE fp flags in bad state, leaving any further
 * fp ops behaving strangely (Inf + 1 resulting in zero, for example). */
#ifdef __osf__
#  include <machine/fpu.h>
#  define PERL_SYS_FPU_INIT \
     STMT_START { \
         ieee_set_fp_control(IEEE_TRAP_ENABLE_INV); \
         signal(SIGFPE, SIG_IGN); \
     } STMT_END
#endif
/* In IRIX the default for Flush to Zero bit is true,
 * which means that results going below the minimum of normal
 * floating points go to zero, instead of going denormal/subnormal.
 * This is unlike almost any other system running Perl, so let's clear it.
 * [perl #123767] IRIX64 blead (ddce084a) opbasic/arith.t failure, originally
 * [perl #120426] small numbers shouldn't round to zero if they have extra floating digits
 *
 * XXX The flush-to-zero behaviour should be a Configure scan.
 * To change the behaviour usually requires some system-specific
 * incantation, though, like the below. */
#ifdef __sgi
#  include <sys/fpu.h>
#  define PERL_SYS_FPU_INIT \
     STMT_START { \
         union fpc_csr csr; \
         csr.fc_word = get_fpc_csr(); \
         csr.fc_struct.flush = 0; \
         set_fpc_csr(csr.fc_word); \
     } STMT_END
#endif

#ifndef PERL_SYS_FPU_INIT
#  define PERL_SYS_FPU_INIT NOOP
#endif

#ifndef PERL_SYS_INIT3_BODY
#  define PERL_SYS_INIT3_BODY(argvp,argcp,envp) PERL_SYS_INIT_BODY(argvp,argcp)
#endif

/*
=for apidoc_section $embedding

=for apidoc   Am|void|PERL_SYS_INIT |int *argc|char*** argv
=for apidoc_item|    |PERL_SYS_INIT3|int *argc|char*** argv|char*** env

These provide system-specific tune up of the C runtime environment necessary to
run Perl interpreters.  Only one should be used, and it should be called only
once, before creating any Perl interpreters.

They differ in that C<PERL_SYS_INIT3> also initializes C<env>.

=for apidoc Am|void|PERL_SYS_TERM|
Provides system-specific clean up of the C runtime environment after
running Perl interpreters.  This should be called only once, after
freeing any remaining Perl interpreters.

=cut
 */

#define PERL_SYS_INIT(argc, argv)	Perl_sys_init(argc, argv)
#define PERL_SYS_INIT3(argc, argv, env)	Perl_sys_init3(argc, argv, env)
#define PERL_SYS_TERM()			Perl_sys_term()

#ifndef PERL_WRITE_MSG_TO_CONSOLE
#  define PERL_WRITE_MSG_TO_CONSOLE(io, msg, len) PerlIO_write(io, msg, len)
#endif

#ifndef MAXPATHLEN
#  ifdef PATH_MAX
#    ifdef _POSIX_PATH_MAX
#       if PATH_MAX > _POSIX_PATH_MAX
/* POSIX 1990 (and pre) was ambiguous about whether PATH_MAX
 * included the null byte or not.  Later amendments of POSIX,
 * XPG4, the Austin Group, and the Single UNIX Specification
 * all explicitly include the null byte in the PATH_MAX.
 * Ditto for _POSIX_PATH_MAX. */
#         define MAXPATHLEN PATH_MAX
#       else
#         define MAXPATHLEN _POSIX_PATH_MAX
#       endif
#    else
#      define MAXPATHLEN (PATH_MAX+1)
#    endif
#  else
#    define MAXPATHLEN 1024	/* Err on the large side. */
#  endif
#endif

/* clang Thread Safety Analysis/Annotations/Attributes
 * http://clang.llvm.org/docs/ThreadSafetyAnalysis.html
 *
 * Available since clang 3.6-ish (appeared in 3.4, but shaky still in 3.5).
 * Apple XCode hijacks __clang_major__ and __clang_minor__
 * (6.1 means really clang 3.6), so needs extra hijinks
 * (could probably also test the contents of __apple_build_version__).
 */
#if defined(USE_ITHREADS) && defined(I_PTHREAD) && \
    defined(__clang__) && \
    !defined(SWIG) && \
  ((!defined(__apple_build_version__) &&               \
    ((__clang_major__ == 3 && __clang_minor__ >= 6) || \
     (__clang_major__ >= 4))) || \
   (defined(__apple_build_version__) &&                \
    ((__clang_major__ == 6 && __clang_minor__ >= 1) || \
     (__clang_major__ >= 7))))
#  define PERL_TSA__(x)   __attribute__((x))
#  define PERL_TSA_ACTIVE
#else
#  define PERL_TSA__(x)   /* No TSA, make TSA attributes no-ops. */
#  undef PERL_TSA_ACTIVE
#endif

/* PERL_TSA_CAPABILITY() is used to annotate typedefs.
 * typedef old_type PERL_TSA_CAPABILITY("mutex") new_type;
 */
#define PERL_TSA_CAPABILITY(x) \
    PERL_TSA__(capability(x))

/* In the below examples the mutex must be lexically visible, usually
 * either as global variables, or as function arguments. */

/* PERL_TSA_GUARDED_BY() is used to annotate global variables.
 *
 * Foo foo PERL_TSA_GUARDED_BY(mutex);
 */
#define PERL_TSA_GUARDED_BY(x) \
    PERL_TSA__(guarded_by(x))

/* PERL_TSA_PT_GUARDED_BY() is used to annotate global pointers.
 * The data _behind_ the pointer is guarded.
 *
 * Foo* ptr PERL_TSA_PT_GUARDED_BY(mutex);
 */
#define PERL_TSA_PT_GUARDED_BY(x) \
    PERL_TSA__(pt_guarded_by(x))

/* PERL_TSA_REQUIRES() is used to annotate functions.
 * The caller MUST hold the resource when calling the function.
 *
 * void Foo() PERL_TSA_REQUIRES(mutex);
 */
#define PERL_TSA_REQUIRES(x) \
    PERL_TSA__(requires_capability(x))

/* PERL_TSA_EXCLUDES() is used to annotate functions.
 * The caller MUST NOT hold resource when calling the function.
 *
 * EXCLUDES should be used when the function first acquires
 * the resource and then releases it.  Use to avoid deadlock.
 *
 * void Foo() PERL_TSA_EXCLUDES(mutex);
 */
#define PERL_TSA_EXCLUDES(x) \
    PERL_TSA__(locks_excluded(x))

/* PERL_TSA_ACQUIRE() is used to annotate functions.
 * The caller MUST NOT hold the resource when calling the function,
 * and the function will acquire the resource.
 *
 * void Foo() PERL_TSA_ACQUIRE(mutex);
 */
#define PERL_TSA_ACQUIRE(x) \
    PERL_TSA__(acquire_capability(x))

/* PERL_TSA_RELEASE() is used to annotate functions.
 * The caller MUST hold the resource when calling the function,
 * and the function will release the resource.
 *
 * void Foo() PERL_TSA_RELEASE(mutex);
 */
#define PERL_TSA_RELEASE(x) \
    PERL_TSA__(release_capability(x))

/* PERL_TSA_NO_TSA is used to annotate functions.
 * Used when being intentionally unsafe, or when the code is too
 * complicated for the analysis.  Use sparingly.
 *
 * void Foo() PERL_TSA_NO_TSA;
 */
#define PERL_TSA_NO_TSA \
    PERL_TSA__(no_thread_safety_analysis)

/* There are more annotations/attributes available, see the clang
 * documentation for details. */

#if defined(USE_ITHREADS)
#  if   defined(WIN32)
#    include <win32thread.h>
#  elif defined(OS2)
#    include "os2thread.h"
#  elif defined(I_MACH_CTHREADS)
#    include <mach/cthreads.h>
typedef cthread_t	perl_os_thread;
typedef mutex_t		perl_mutex;
typedef condition_t	perl_cond;
typedef void *		perl_key;
#  elif defined(I_PTHREAD) /* Posix threads */
#    include <pthread.h>
typedef pthread_t	perl_os_thread;
typedef pthread_mutex_t PERL_TSA_CAPABILITY("mutex") perl_mutex;
typedef pthread_cond_t	perl_cond;
typedef pthread_key_t	perl_key;
#  endif

/* Many readers; single writer */
typedef struct {
    perl_mutex lock;
    perl_cond  wakeup;
    SSize_t    readers_count;
} perl_RnW1_mutex_t;


#endif /* USE_ITHREADS */

#ifdef PERL_TSA_ACTIVE
/* Since most pthread mutex interfaces have not been annotated, we
 * need to have these wrappers. The NO_TSA annotation is quite ugly
 * but it cannot be avoided in plain C, unlike in C++, where one could
 * e.g. use ACQUIRE() with no arg on a mutex lock method.
 *
 * The bodies of these wrappers are in util.c
 *
 * TODO: however, some platforms are starting to get these clang
 * thread safety annotations for pthreads, for example FreeBSD.
 * Do we need a way to a bypass these wrappers? */
EXTERN_C int perl_tsa_mutex_lock(perl_mutex* mutex)
  PERL_TSA_ACQUIRE(*mutex)
  PERL_TSA_NO_TSA;
EXTERN_C int perl_tsa_mutex_unlock(perl_mutex* mutex)
  PERL_TSA_RELEASE(*mutex)
  PERL_TSA_NO_TSA;
#endif

#if defined(WIN32)
#  include "win32.h"
#endif

#define STATUS_UNIX	PL_statusvalue
#ifdef VMS
#   define STATUS_NATIVE	PL_statusvalue_vms
/*
 * vaxc$errno is only guaranteed to be valid if errno == EVMSERR, otherwise
 * its contents can not be trusted.  Unfortunately, Perl seems to check
 * it on exit, so it when PL_statusvalue_vms is updated, vaxc$errno should
 * be updated also.
 */
#  include <stsdef.h>
#  include <ssdef.h>
/* Presume this because if VMS changes it, it will require a new
 * set of APIs for waiting on children for binary compatibility.
 */
#  define child_offset_bits (8)
#  ifndef C_FAC_POSIX
#  define C_FAC_POSIX 0x35A000
#  endif

/*  STATUS_EXIT - validates and returns a NATIVE exit status code for the
 * platform from the existing UNIX or Native status values.
 */

#   define STATUS_EXIT \
        (((I32)PL_statusvalue_vms == -1 ? SS$_ABORT : PL_statusvalue_vms) | \
           (VMSISH_HUSHED ? STS$M_INHIB_MSG : 0))


/* STATUS_NATIVE_CHILD_SET - Calculate UNIX status that matches the child
 * exit code and shifts the UNIX value over the correct number of bits to
 * be a child status.  Usually the number of bits is 8, but that could be
 * platform dependent.  The NATIVE status code is presumed to have either
 * from a child process.
 */

/* This is complicated.  The child processes return a true native VMS
   status which must be saved.  But there is an assumption in Perl that
   the UNIX child status has some relationship to errno values, so
   Perl tries to translate it to text in some of the tests.
   In order to get the string translation correct, for the error, errno
   must be EVMSERR, but that generates a different text message
   than what the test programs are expecting.  So an errno value must
   be derived from the native status value when an error occurs.
   That will hide the true native status message.  With this version of
   perl, the true native child status can always be retrieved so that
   is not a problem.  But in this case, Pl_statusvalue and errno may
   have different values in them.
 */

#   define STATUS_NATIVE_CHILD_SET(n) \
        STMT_START {							\
            I32 evalue = (I32)n;					\
            if (evalue == EVMSERR) {					\
              PL_statusvalue_vms = vaxc$errno;				\
              PL_statusvalue = evalue;					\
            } else {							\
              PL_statusvalue_vms = evalue;				\
              if (evalue == -1) {					\
                PL_statusvalue = -1;					\
                PL_statusvalue_vms = SS$_ABORT; /* Should not happen */ \
              } else							\
                PL_statusvalue = Perl_vms_status_to_unix(evalue, 1);	\
              set_vaxc_errno(evalue);					\
              if ((PL_statusvalue_vms & C_FAC_POSIX) == C_FAC_POSIX)	\
                  set_errno(EVMSERR);					\
              else set_errno(Perl_vms_status_to_unix(evalue, 0));	\
              PL_statusvalue = PL_statusvalue << child_offset_bits;	\
            }								\
        } STMT_END

#   ifdef VMSISH_STATUS
#	define STATUS_CURRENT	(VMSISH_STATUS ? STATUS_NATIVE : STATUS_UNIX)
#   else
#	define STATUS_CURRENT	STATUS_UNIX
#   endif

  /* STATUS_UNIX_SET - takes a UNIX/POSIX errno value and attempts to update
   * the NATIVE status to an equivalent value.  Can not be used to translate
   * exit code values as exit code values are not guaranteed to have any
   * relationship at all to errno values.
   * This is used when Perl is forcing errno to have a specific value.
   */
#   define STATUS_UNIX_SET(n)				\
        STMT_START {					\
            I32 evalue = (I32)n;			\
            PL_statusvalue = evalue;			\
            if (PL_statusvalue != -1) {			\
                if (PL_statusvalue != EVMSERR) {	\
                  PL_statusvalue &= 0xFFFF;		\
                  if (MY_POSIX_EXIT)			\
                    PL_statusvalue_vms=PL_statusvalue ? SS$_ABORT : SS$_NORMAL;\
                  else PL_statusvalue_vms = Perl_unix_status_to_vms(evalue); \
                }					\
                else {					\
                  PL_statusvalue_vms = vaxc$errno;	\
                }					\
            }						\
            else PL_statusvalue_vms = SS$_ABORT;	\
            set_vaxc_errno(PL_statusvalue_vms);		\
        } STMT_END

  /* STATUS_UNIX_EXIT_SET - Takes a UNIX/POSIX exit code and sets
   * the NATIVE error status based on it.
   *
   * When in the default mode to comply with the Perl VMS documentation,
   * 0 is a success and any other code sets the NATIVE status to a failure
   * code of SS$_ABORT.
   *
   * In the new POSIX EXIT mode, native status will be set so that the
   * actual exit code will can be retrieved by the calling program or
   * shell.
   *
   * If the exit code is not clearly a UNIX parent or child exit status,
   * it will be passed through as a VMS status.
   */

#   define STATUS_UNIX_EXIT_SET(n)			\
        STMT_START {					\
            I32 evalue = (I32)n;			\
            PL_statusvalue = evalue;			\
            if (MY_POSIX_EXIT) { \
              if (evalue <= 0xFF00) {		\
                  if (evalue > 0xFF)			\
                    evalue = ((U8) (evalue >> child_offset_bits)); \
                  PL_statusvalue_vms =		\
                    (C_FAC_POSIX | (evalue << 3 ) |	\
                    ((evalue == 1) ? (STS$K_ERROR | STS$M_INHIB_MSG) : 1)); \
              } else /* forgive them Perl, for they have sinned */ \
                PL_statusvalue_vms = evalue; \
            } else { \
              if (evalue == 0)			\
                PL_statusvalue_vms = SS$_NORMAL;	\
              else if (evalue <= 0xFF00) \
                PL_statusvalue_vms = SS$_ABORT; \
              else { /* forgive them Perl, for they have sinned */ \
                  if (evalue != EVMSERR) PL_statusvalue_vms = evalue; \
                  else PL_statusvalue_vms = vaxc$errno;	\
                  /* And obviously used a VMS status value instead of UNIX */ \
                  PL_statusvalue = EVMSERR;		\
              } \
              set_vaxc_errno(PL_statusvalue_vms);	\
            }						\
        } STMT_END


  /* STATUS_EXIT_SET - Takes a NATIVE/UNIX/POSIX exit code
   * and sets the NATIVE error status based on it.  This special case
   * is needed to maintain compatibility with past VMS behavior.
   *
   * In the default mode on VMS, this number is passed through as
   * both the NATIVE and UNIX status.  Which makes it different
   * that the STATUS_UNIX_EXIT_SET.
   *
   * In the new POSIX EXIT mode, native status will be set so that the
   * actual exit code will can be retrieved by the calling program or
   * shell.
   *
   * A POSIX exit code is from 0 to 255.  If the exit code is higher
   * than this, it needs to be assumed that it is a VMS exit code and
   * passed through.
   */

#   define STATUS_EXIT_SET(n)				\
        STMT_START {					\
            I32 evalue = (I32)n;			\
            PL_statusvalue = evalue;			\
            if (MY_POSIX_EXIT)				\
                if (evalue > 255) PL_statusvalue_vms = evalue; else {	\
                  PL_statusvalue_vms = \
                    (C_FAC_POSIX | (evalue << 3 ) |	\
                     ((evalue == 1) ? (STS$K_ERROR | STS$M_INHIB_MSG) : 1));} \
            else					\
                PL_statusvalue_vms = evalue ? evalue : SS$_NORMAL; \
            set_vaxc_errno(PL_statusvalue_vms);		\
        } STMT_END


 /* This macro forces a success status */
#   define STATUS_ALL_SUCCESS	\
        (PL_statusvalue = 0, PL_statusvalue_vms = SS$_NORMAL)

 /* This macro forces a failure status */
#   define STATUS_ALL_FAILURE	(PL_statusvalue = 1, \
     vaxc$errno = PL_statusvalue_vms = MY_POSIX_EXIT ? \
        (C_FAC_POSIX | (1 << 3) | STS$K_ERROR | STS$M_INHIB_MSG) : SS$_ABORT)

#elif defined(__amigaos4__)
 /* A somewhat experimental attempt to simulate posix return code values */
#   define STATUS_NATIVE	PL_statusvalue_posix
#   define STATUS_NATIVE_CHILD_SET(n)                      \
        STMT_START {                                       \
            PL_statusvalue_posix = (n);                    \
            if (PL_statusvalue_posix < 0) {                \
                PL_statusvalue = -1;                       \
            }                                              \
            else {                                         \
                PL_statusvalue = n << 8;                   \
            }                                              \
        } STMT_END
#   define STATUS_UNIX_SET(n)		\
        STMT_START {			\
            PL_statusvalue = (n);		\
            if (PL_statusvalue != -1)	\
                PL_statusvalue &= 0xFFFF;	\
        } STMT_END
#   define STATUS_UNIX_EXIT_SET(n) STATUS_UNIX_SET(n)
#   define STATUS_EXIT_SET(n) STATUS_UNIX_SET(n)
#   define STATUS_CURRENT STATUS_UNIX
#   define STATUS_EXIT STATUS_UNIX
#   define STATUS_ALL_SUCCESS	(PL_statusvalue = 0, PL_statusvalue_posix = 0)
#   define STATUS_ALL_FAILURE	(PL_statusvalue = 1, PL_statusvalue_posix = 1)

#else
#   define STATUS_NATIVE	PL_statusvalue_posix
#   if defined(WCOREDUMP)
#       define STATUS_NATIVE_CHILD_SET(n)                  \
            STMT_START {                                   \
                PL_statusvalue_posix = (n);                \
                if (PL_statusvalue_posix == -1)            \
                    PL_statusvalue = -1;                   \
                else {                                     \
                    PL_statusvalue =                       \
                        (WIFEXITED(PL_statusvalue_posix) ? (WEXITSTATUS(PL_statusvalue_posix) << 8) : 0) |  \
                        (WIFSIGNALED(PL_statusvalue_posix) ? (WTERMSIG(PL_statusvalue_posix) & 0x7F) : 0) | \
                        (WIFSIGNALED(PL_statusvalue_posix) && WCOREDUMP(PL_statusvalue_posix) ? 0x80 : 0);  \
                }                                          \
            } STMT_END
#   elif defined(WIFEXITED)
#       define STATUS_NATIVE_CHILD_SET(n)                  \
            STMT_START {                                   \
                PL_statusvalue_posix = (n);                \
                if (PL_statusvalue_posix == -1)            \
                    PL_statusvalue = -1;                   \
                else {                                     \
                    PL_statusvalue =                       \
                        (WIFEXITED(PL_statusvalue_posix) ? (WEXITSTATUS(PL_statusvalue_posix) << 8) : 0) |  \
                        (WIFSIGNALED(PL_statusvalue_posix) ? (WTERMSIG(PL_statusvalue_posix) & 0x7F) : 0);  \
                }                                          \
            } STMT_END
#   else
#       define STATUS_NATIVE_CHILD_SET(n)                  \
            STMT_START {                                   \
                PL_statusvalue_posix = (n);                \
                if (PL_statusvalue_posix == -1)            \
                    PL_statusvalue = -1;                   \
                else {                                     \
                    PL_statusvalue =                       \
                        PL_statusvalue_posix & 0xFFFF;     \
                }                                          \
            } STMT_END
#   endif
#   define STATUS_UNIX_SET(n)		\
        STMT_START {			\
            PL_statusvalue = (n);		\
            if (PL_statusvalue != -1)	\
                PL_statusvalue &= 0xFFFF;	\
        } STMT_END
#   define STATUS_UNIX_EXIT_SET(n) STATUS_UNIX_SET(n)
#   define STATUS_EXIT_SET(n) STATUS_UNIX_SET(n)
#   define STATUS_CURRENT STATUS_UNIX
#   define STATUS_EXIT STATUS_UNIX
#   define STATUS_ALL_SUCCESS	(PL_statusvalue = 0, PL_statusvalue_posix = 0)
#   define STATUS_ALL_FAILURE	(PL_statusvalue = 1, PL_statusvalue_posix = 1)
#endif

/* flags in PL_exit_flags for nature of exit() */
#define PERL_EXIT_EXPECTED	0x01
#define PERL_EXIT_DESTRUCT_END  0x02  /* Run END in perl_destruct */
#define PERL_EXIT_WARN		0x04  /* Warn if Perl_my_exit() or Perl_my_failure_exit() called */
#define PERL_EXIT_ABORT		0x08  /* Call abort() if Perl_my_exit() or Perl_my_failure_exit() called */

#ifndef PERL_CORE
/* format to use for version numbers in file/directory names */
/* XXX move to Configure? */
/* This was only ever used for the current version, and that can be done at
   compile time, as PERL_FS_VERSION, so should we just delete it?  */
#  ifndef PERL_FS_VER_FMT
#    define PERL_FS_VER_FMT	"%d.%d.%d"
#  endif
#endif

#ifndef PERL_FS_VERSION
#  define PERL_FS_VERSION	PERL_VERSION_STRING
#endif

/*

=for apidoc_section $io
=for apidoc Amn|void|PERL_FLUSHALL_FOR_CHILD

This defines a way to flush all output buffers.  This may be a
performance issue, so we allow people to disable it.  Also, if
we are using stdio, there are broken implementations of fflush(NULL)
out there, Solaris being the most prominent.

=cut
 */

#ifndef PERL_FLUSHALL_FOR_CHILD
# if defined(USE_PERLIO) || defined(FFLUSH_NULL)
#  define PERL_FLUSHALL_FOR_CHILD	PerlIO_flush((PerlIO*)NULL)
# elif defined(FFLUSH_ALL)
#  define PERL_FLUSHALL_FOR_CHILD	my_fflush_all()
# else
#  define PERL_FLUSHALL_FOR_CHILD	NOOP
# endif
#endif

#ifndef PERL_WAIT_FOR_CHILDREN
#  define PERL_WAIT_FOR_CHILDREN	NOOP
#endif

/* the traditional thread-unsafe notion of "current interpreter". */
#ifndef PERL_SET_INTERP
#  define PERL_SET_INTERP(i)                                            \
            STMT_START { PL_curinterp = (PerlInterpreter*)(i);          \
                         PERL_SET_NON_tTHX_CONTEXT(i);                  \
            } STMT_END
#endif

#ifndef PERL_GET_INTERP
#  define PERL_GET_INTERP		(PL_curinterp)
#endif

#if defined(MULTIPLICITY) && !defined(PERL_GET_THX)
#  define PERL_GET_THX		((PerlInterpreter *)PERL_GET_CONTEXT)
#  define PERL_SET_THX(t)		PERL_SET_CONTEXT(t)
#endif

/*
    This replaces the previous %_ "hack" by the "%p" hacks.
    All that is required is that the perl source does not
    use "%-p" or "%-<number>p" or "%<number>p" formats.
    These formats will still work in perl code.
    See comments in sv.c for further details.

    Robin Barker 2005-07-14

    No longer use %1p for VDf = %vd.  RMB 2007-10-19
*/

#ifndef SVf_
#  define SVf_(n) "-" STRINGIFY(n) "p"
#endif

#ifndef SVf
#  define SVf "-p"
#endif

#ifndef SVf32
#  define SVf32 SVf_(32)
#endif

#ifndef SVf256
#  define SVf256 SVf_(256)
#endif

#define SVfARG(p) ((void*)(p))

/* Render an SV as a quoted and escaped string suitable for an error message.
 * Only shows the first PERL_QUOTEDPREFIX_LEN characters, and adds ellipses if the
 * string is too long.
 */
#ifndef PERL_QUOTEDPREFIX_LEN
# define PERL_QUOTEDPREFIX_LEN 256
#endif
#ifndef SVf_QUOTEDPREFIX
#  define SVf_QUOTEDPREFIX "5p"
#endif

/* like %s but runs through the quoted prefix logic */
#ifndef PVf_QUOTEDPREFIX
#  define PVf_QUOTEDPREFIX "1p"
#endif

#ifndef HEKf
#  define HEKf "2p"
#endif

#ifndef HEKf_QUOTEDPREFIX
#  define HEKf_QUOTEDPREFIX "7p"
#endif

/* Not ideal, but we cannot easily include a number in an already-numeric
 * format sequence. */
#ifndef HEKf256
#  define HEKf256 "3p"
#endif

#ifndef HEKf256_QUOTEDPREFIX
#  define HEKf256_QUOTEDPREFIX "8p"
#endif

#define HEKfARG(p) ((void*)(p))

/* Documented in perlguts
 *
 * %4p and %9p are custom formats for handling UTF8 parameters.
 * They only occur when prefixed by specific other formats.
 */
#ifndef UTF8f
#  define UTF8f "d%" UVuf "%4p"
#endif
#ifndef UTF8f_QUOTEDPREFIX
#  define UTF8f_QUOTEDPREFIX "d%" UVuf "%9p"
#endif
#define UTF8fARG(u,l,p) (int)cBOOL(u), (UV)(l), (void*)(p)

#define PNf UTF8f
#define PNfARG(pn) (int)1, (UV)PadnameLEN(pn), (void *)PadnamePV(pn)

#define HvNAMEf "6p"
#define HvNAMEf_QUOTEDPREFIX "10p"

#define HvNAMEfARG(hv) ((void*)(hv))

#ifdef PERL_CORE
/* not used; but needed for backward compatibility with XS code? - RMB
=for apidoc_section $io_formats
=for apidoc AmnD|const char *|UVf

Obsolete form of C<UVuf>, which you should convert to instead use

=cut
*/
#  undef UVf
#elif !defined(UVf)
#  define UVf UVuf
#endif

#if !defined(DEBUGGING) && !defined(NDEBUG)
#  define NDEBUG 1
#endif
#include <assert.h>

/* For functions that are marked as __attribute__noreturn__, it's not
   appropriate to call return.  In either case, include the lint directive.
 */
#ifdef HASATTRIBUTE_NORETURN
#  define NORETURN_FUNCTION_END NOT_REACHED;
#else
#  define NORETURN_FUNCTION_END NOT_REACHED; return 0
#endif

#ifdef HAS_BUILTIN_EXPECT
#  define EXPECT(expr,val)                  __builtin_expect(expr,val)
#else
#  define EXPECT(expr,val)                  (expr)
#endif

/*
=for apidoc_section $directives

=for apidoc Am||LIKELY|bool expr

Returns the input unchanged, but at the same time it gives a branch prediction
hint to the compiler that this condition is likely to be true.

=for apidoc Am||UNLIKELY|bool expr

Returns the input unchanged, but at the same time it gives a branch prediction
hint to the compiler that this condition is likely to be false.

=cut
*/
#define LIKELY(cond)                        EXPECT(cBOOL(cond),TRUE)
#define UNLIKELY(cond)                      EXPECT(cBOOL(cond),FALSE)

#ifdef HAS_BUILTIN_CHOOSE_EXPR
/* placeholder */
#endif

/* STATIC_ASSERT_DECL/STATIC_ASSERT_STMT are like assert(), but for compile
   time invariants. That is, their argument must be a constant expression that
   can be verified by the compiler. This expression can contain anything that's
   known to the compiler, e.g. #define constants, enums, or sizeof (...). If
   the expression evaluates to 0, compilation fails.
   Because they generate no runtime code (i.e.  their use is "free"), they're
   always active, even under non-DEBUGGING builds.
   STATIC_ASSERT_DECL expands to a declaration and is suitable for use at
   file scope (outside of any function).
   STATIC_ASSERT_STMT expands to a statement and is suitable for use inside a
   function.
*/
#if (! defined(__IBMC__) || __IBMC__ >= 1210)                               \
 && ((   defined(static_assert) && (   defined(_ISOC11_SOURCE)              \
                                    || (__STDC_VERSION__ - 0) >= 201101L))  \
     || (defined(__cplusplus) && __cplusplus >= 201103L))
/* XXX static_assert is a macro defined in <assert.h> in C11 or a compiler
   builtin in C++11.  But IBM XL C V11 does not support _Static_assert, no
   matter what <assert.h> says.
*/
#  define STATIC_ASSERT_DECL(COND) static_assert(COND, #COND)
#else
/* We use a bit-field instead of an array because gcc accepts
   'typedef char x[n]' where n is not a compile-time constant.
   We want to enforce constantness.
*/
#  define STATIC_ASSERT_2(COND, SUFFIX) \
    typedef struct { \
        unsigned int _static_assertion_failed_##SUFFIX : (COND) ? 1 : -1; \
    } _static_assertion_failed_##SUFFIX PERL_UNUSED_DECL
#  define STATIC_ASSERT_1(COND, SUFFIX) STATIC_ASSERT_2(COND, SUFFIX)
#  define STATIC_ASSERT_DECL(COND)    STATIC_ASSERT_1(COND, __LINE__)
#endif
/* We need this wrapper even in C11 because 'case X: static_assert(...);' is an
   error (static_assert is a declaration, and only statements can have labels).
*/
#define STATIC_ASSERT_STMT(COND)      STMT_START { STATIC_ASSERT_DECL(COND); } STMT_END

#ifndef __has_builtin
#  define __has_builtin(x) 0 /* not a clang style compiler */
#endif

/*
=for apidoc Am||ASSUME|bool expr
C<ASSUME> is like C<assert()>, but it has a benefit in a release build. It is a
hint to a compiler about a statement of fact in a function call free
expression, which allows the compiler to generate better machine code.  In a
debug build, C<ASSUME(x)> is a synonym for C<assert(x)>. C<ASSUME(0)> means the
control path is unreachable. In a for loop, C<ASSUME> can be used to hint that
a loop will run at least X times. C<ASSUME> is based off MSVC's C<__assume>
intrinsic function, see its documents for more details.

=cut
*/

#if __has_builtin(__builtin_unreachable)
#    define HAS_BUILTIN_UNREACHABLE
#elif PERL_GCC_VERSION_GE(4,5,0)
#    define HAS_BUILTIN_UNREACHABLE
#endif

#ifdef DEBUGGING
#  define ASSUME(x) assert(x)
#elif __has_builtin(__builtin_assume)
#  if defined(__clang__) || defined(__clang)
#    define ASSUME(x)  CLANG_DIAG_IGNORE(-Wassume)      \
                       __builtin_assume (x)             \
                       CLANG_DIAG_RESTORE
#  else
#    define ASSUME(x)  __builtin_assume(x)
#  endif
#elif defined(_MSC_VER)
#  define ASSUME(x) __assume(x)
#elif defined(__ARMCC_VERSION) /* untested */
#  define ASSUME(x) __promise(x)
#elif defined(HAS_BUILTIN_UNREACHABLE)
    /* Compilers can take the hint from something being unreachable */
#    define ASSUME(x) ((x) ? (void) 0 : __builtin_unreachable())
#else
    /* Not DEBUGGING, so assert() is a no-op, but a random compiler might
     * define assert() to its own special optimization token so pass it through
     * to C lib as a last resort */
#  define ASSUME(x) assert(x)
#endif

#ifdef HAS_BUILTIN_UNREACHABLE
#  define NOT_REACHED                                                       \
        STMT_START {                                                        \
            ASSUME(!"UNREACHABLE"); __builtin_unreachable();                \
        } STMT_END
#  undef HAS_BUILTIN_UNREACHABLE /* Don't leak out this internal symbol */
#elif ! defined(__GNUC__) && (defined(__sun) || defined(__hpux))
    /* These just complain that NOT_REACHED isn't reached */
#  define NOT_REACHED
#else
#  define NOT_REACHED  ASSUME(!"UNREACHABLE")
#endif

/* Some unistd.h's give a prototype for pause() even though
   HAS_PAUSE ends up undefined.  This causes the #define
   below to be rejected by the compiler.  Sigh.
*/
#ifdef HAS_PAUSE
#define Pause	pause
#else
#define Pause() sleep((32767<<16)+32767)
#endif

#ifndef IOCPARM_LEN
#   ifdef IOCPARM_MASK
        /* on BSDish systems we're safe */
#	define IOCPARM_LEN(x)  (((x) >> 16) & IOCPARM_MASK)
#   elif defined(_IOC_SIZE) && defined(__GLIBC__)
        /* on Linux systems we're safe; except when we're not [perl #38223] */
#	define IOCPARM_LEN(x) (_IOC_SIZE(x) < 256 ? 256 : _IOC_SIZE(x))
#   else
        /* otherwise guess at what's safe */
#	define IOCPARM_LEN(x)	256
#   endif
#endif

#if defined(__CYGWIN__)
/* USEMYBINMODE
 *   This symbol, if defined, indicates that the program should
 *   use the routine my_binmode(FILE *fp, char iotype, int mode) to insure
 *   that a file is in "binary" mode -- that is, that no translation
 *   of bytes occurs on read or write operations.
 */
#  define USEMYBINMODE /**/
#  include <io.h> /* for setmode() prototype */
#  define my_binmode(fp, iotype, mode) \
            cBOOL(PerlLIO_setmode(fileno(fp), mode) != -1)
#endif

#ifdef __CYGWIN__
void init_os_extras(void);
#endif

#ifdef UNION_ANY_DEFINITION
UNION_ANY_DEFINITION;
#else
union any {
    void*       any_ptr;
    SV*         any_sv;
    SV**        any_svp;
    GV*         any_gv;
    AV*         any_av;
    HV*         any_hv;
    OP*         any_op;
    char*       any_pv;
    char**      any_pvp;
    I32         any_i32;
    U32         any_u32;
    IV          any_iv;
    UV          any_uv;
    long        any_long;
    bool        any_bool;
    Size_t      any_size;
    SSize_t     any_ssize;
    STRLEN      any_strlen;
    void        (*any_dptr) (void*);
    void        (*any_dxptr) (pTHX_ void*);
};
#endif

typedef I32 (*filter_t) (pTHX_ int, SV *, int);

#define FILTER_READ(idx, sv, len)  filter_read(idx, sv, len)
#define FILTER_DATA(idx) \
            (PL_parser ? AvARRAY(PL_parser->rsfp_filters)[idx] : NULL)
#define FILTER_ISREADER(idx) \
            (PL_parser && PL_parser->rsfp_filters \
                && idx >= AvFILLp(PL_parser->rsfp_filters))
#define PERL_FILTER_EXISTS(i) \
            (PL_parser && PL_parser->rsfp_filters \
                && (Size_t) (i) < av_count(PL_parser->rsfp_filters))

#if defined(_AIX) && !defined(_AIX43)
#if defined(USE_REENTRANT) || defined(_REENTRANT) || defined(_THREAD_SAFE)
/* We cannot include <crypt.h> to get the struct crypt_data
 * because of setkey prototype problems when threading */
typedef        struct crypt_data {     /* straight from /usr/include/crypt.h */
    /* From OSF, Not needed in AIX
       char C[28], D[28];
    */
    char E[48];
    char KS[16][48];
    char block[66];
    char iobuf[16];
} CRYPTD;
#endif /* threading */
#endif /* AIX */

#ifndef PERL_CALLCONV
#  ifdef __cplusplus
#    define PERL_CALLCONV  EXTERN_C
#  else
#    define PERL_CALLCONV
#  endif
#endif
#ifndef PERL_CALLCONV_NO_RET
#    define PERL_CALLCONV_NO_RET PERL_CALLCONV
#endif

/* PERL_STATIC_NO_RET is supposed to be equivalent to STATIC on builds that
   dont have a noreturn as a declaration specifier
*/
#ifndef PERL_STATIC_NO_RET
#  define PERL_STATIC_NO_RET STATIC
#endif

/* PERL_STATIC_INLINE_NO_RET is supposed to be equivalent to PERL_STATIC_INLINE
 * on builds that dont have a noreturn as a declaration specifier
*/
#ifndef PERL_STATIC_INLINE_NO_RET
#  define PERL_STATIC_INLINE_NO_RET PERL_STATIC_INLINE
#endif

#ifndef PERL_STATIC_FORCE_INLINE
#  define PERL_STATIC_FORCE_INLINE PERL_STATIC_INLINE
#endif

#ifndef PERL_STATIC_FORCE_INLINE_NO_RET
#  define PERL_STATIC_FORCE_INLINE_NO_RET PERL_STATIC_INLINE
#endif

#if !defined(OS2)
#  include "iperlsys.h"
#endif

#ifdef __LIBCATAMOUNT__
#undef HAS_PASSWD  /* unixish.h but not unixish enough. */
#undef HAS_GROUP
#define FAKE_BIT_BUCKET
#endif

/* [perl #22371] Algorithmic Complexity Attack on Perl 5.6.1, 5.8.0.
 * Note that the USE_HASH_SEED and similar defines are *NOT* defined by
 * Configure, despite their names being similar to other defines like
 * USE_ITHREADS.  Configure in fact knows nothing about the randomised
 * hashes.  Therefore to enable/disable the hash randomisation defines
 * use the Configure -Accflags=... instead. */
#if !defined(NO_HASH_SEED) && !defined(USE_HASH_SEED)
#  define USE_HASH_SEED
#endif

#include "perly.h"


/* macros to define bit-fields in structs. */
#ifndef PERL_BITFIELD8
#  ifdef HAS_NON_INT_BITFIELDS
#  define PERL_BITFIELD8 U8
#  else
#    define PERL_BITFIELD8 unsigned
#  endif
#endif
#ifndef PERL_BITFIELD16
#  ifdef HAS_NON_INT_BITFIELDS
#  define PERL_BITFIELD16 U16
#  else
#    define PERL_BITFIELD16 unsigned
#  endif
#endif
#ifndef PERL_BITFIELD32
#  ifdef HAS_NON_INT_BITFIELDS
#  define PERL_BITFIELD32 U32
#  else
#    define PERL_BITFIELD32 unsigned
#  endif
#endif

#include "sv.h"
#include "regexp.h"
#include "util.h"
#include "form.h"
#include "gv.h"
#include "pad.h"
#include "cv.h"
#include "opnames.h"
#include "op.h"
#include "hv.h"
#include "cop.h"
#include "av.h"
#include "mg.h"
#include "scope.h"
#include "warnings.h"
#include "utf8.h"

/* these would be in doio.h if there was such a file */
#define my_stat()  my_stat_flags(SV_GMAGIC)
#define my_lstat() my_lstat_flags(SV_GMAGIC)

/* defined in sv.c, but also used in [ach]v.c */
#undef _XPV_HEAD
#undef _XPVMG_HEAD
#undef _XPVCV_COMMON

#include "parser.h"

typedef struct magic_state MGS;	/* struct magic_state defined in mg.c */

#if defined(PERL_IN_REGEX_ENGINE) || defined(PERL_EXT_RE_BUILD)

/* These have to be predeclared, as they are used in proto.h which is #included
 * before their definitions in regcomp.h. */

struct scan_data_t;
typedef struct regnode_charclass regnode_charclass;

/* A hopefully less confusing name.  The sub-classes are all Posix classes only
 * used under /l matching */
typedef struct regnode_charclass_posixl regnode_charclass_class;
typedef struct regnode_charclass_posixl regnode_charclass_posixl;

typedef struct regnode_ssc regnode_ssc;
typedef struct RExC_state_t RExC_state_t;
struct _reg_trie_data;
typedef struct scan_data_t scan_data_t;

#endif

struct ptr_tbl_ent {
    struct ptr_tbl_ent*		next;
    const void*			oldval;
    void*			newval;
};

struct ptr_tbl {
    struct ptr_tbl_ent**	tbl_ary;
    UV				tbl_max;
    UV				tbl_items;
    struct ptr_tbl_arena	*tbl_arena;
    struct ptr_tbl_ent		*tbl_arena_next;
    struct ptr_tbl_ent		*tbl_arena_end;
};

#if defined(htonl) && !defined(HAS_HTONL)
#define HAS_HTONL
#endif
#if defined(htons) && !defined(HAS_HTONS)
#define HAS_HTONS
#endif
#if defined(ntohl) && !defined(HAS_NTOHL)
#define HAS_NTOHL
#endif
#if defined(ntohs) && !defined(HAS_NTOHS)
#define HAS_NTOHS
#endif
#ifndef HAS_HTONL
#define HAS_HTONS
#define HAS_HTONL
#define HAS_NTOHS
#define HAS_NTOHL
#  if (BYTEORDER & 0xffff) == 0x4321
/* Big endian system, so ntohl, ntohs, htonl and htons do not need to
   re-order their values. However, to behave identically to the alternative
   implementations, they should truncate to the correct size.  */
#    define ntohl(x)    ((x)&0xFFFFFFFF)
#    define htonl(x)    ntohl(x)
#    define ntohs(x)    ((x)&0xFFFF)
#    define htons(x)    ntohs(x)
#  elif BYTEORDER == 0x1234 || BYTEORDER == 0x12345678

/* Note that we can't straight out declare our own htonl and htons because
   the Win32 build process forcibly undefines HAS_HTONL etc for its miniperl,
   to avoid the overhead of initialising the socket subsystem, but the headers
   that *declare* the various functions are still seen. If we declare our own
   htonl etc they will clash with the declarations in the Win32 headers.  */

PERL_STATIC_INLINE U32
my_swap32(const U32 x) {
    return ((x & 0xFF) << 24) | ((x >> 24) & 0xFF)
        | ((x & 0x0000FF00) << 8) | ((x & 0x00FF0000) >> 8);
}

PERL_STATIC_INLINE U16
my_swap16(const U16 x) {
    return ((x & 0xFF) << 8) | ((x >> 8) & 0xFF);
}

#    define htonl(x)    my_swap32(x)
#    define ntohl(x)    my_swap32(x)
#    define ntohs(x)    my_swap16(x)
#    define htons(x)    my_swap16(x)
#  else
#    error "Unsupported byteorder"
/* The C pre-processor doesn't let us return the value of BYTEORDER as part of
   the error message. Please check the value of the macro BYTEORDER, as defined
   in config.h. The values of BYTEORDER we expect are

            big endian  little endian
   32 bit       0x4321  0x1234
   64 bit   0x87654321  0x12345678

   If you have a system with a different byte order, please see
   pod/perlhack.pod for how to submit a patch to add supporting code.
*/
#  endif
#endif

/*
 * Little-endian byte order functions - 'v' for 'VAX', or 'reVerse'.
 * -DWS
 */
#if BYTEORDER == 0x1234 || BYTEORDER == 0x12345678
/* Little endian system, so vtohl, vtohs, htovl and htovs do not need to
   re-order their values. However, to behave identically to the alternative
   implementations, they should truncate to the correct size.  */
#  define vtohl(x)      ((x)&0xFFFFFFFF)
#  define vtohs(x)      ((x)&0xFFFF)
#  define htovl(x)      vtohl(x)
#  define htovs(x)      vtohs(x)
#elif BYTEORDER == 0x4321 || BYTEORDER == 0x87654321
#  define vtohl(x)	((((x)&0xFF)<<24)	\
                        +(((x)>>24)&0xFF)	\
                        +(((x)&0x0000FF00)<<8)	\
                        +(((x)&0x00FF0000)>>8)	)
#  define vtohs(x)	((((x)&0xFF)<<8) + (((x)>>8)&0xFF))
#  define htovl(x)	vtohl(x)
#  define htovs(x)	vtohs(x)
#else
#  error "Unsupported byteorder"
/* If you have need for current perl on PDP-11 or similar, and can help test
   that blead keeps working on a mixed-endian system, then see
   pod/perlhack.pod for how to submit patches to things working again.  */
#endif

/* *MAX Plus 1. A floating point value.
   Hopefully expressed in a way that dodgy floating point can't mess up.
   >> 2 rather than 1, so that value is safely less than I32_MAX after 1
   is added to it
   May find that some broken compiler will want the value cast to I32.
   [after the shift, as signed >> may not be as secure as unsigned >>]
*/
#define I32_MAX_P1 (2.0 * (1 + (((U32)I32_MAX) >> 1)))
#define U32_MAX_P1 (4.0 * (1 + ((U32_MAX) >> 2)))
/* For compilers that can't correctly cast NVs over 0x7FFFFFFF (or
   0x7FFFFFFFFFFFFFFF) to an unsigned integer. In the future, sizeof(UV)
   may be greater than sizeof(IV), so don't assume that half max UV is max IV.
*/
#define U32_MAX_P1_HALF (2.0 * (1 + ((U32_MAX) >> 2)))

#define UV_MAX_P1 (4.0 * (1 + ((UV_MAX) >> 2)))
#define IV_MAX_P1 (2.0 * (1 + (((UV)IV_MAX) >> 1)))
#define UV_MAX_P1_HALF (2.0 * (1 + ((UV_MAX) >> 2)))

/* This may look like unnecessary jumping through hoops, but converting
   out of range floating point values to integers *is* undefined behaviour,
   and it is starting to bite.

=for apidoc_section $casting
=for apidoc Am|I32|I_32|NV what
Cast an NV to I32 while avoiding undefined C behavior

=for apidoc Am|U32|U_32|NV what
Cast an NV to U32 while avoiding undefined C behavior

=for apidoc Am|IV|I_V|NV what
Cast an NV to IV while avoiding undefined C behavior

=for apidoc Am|UV|U_V|NV what
Cast an NV to UV while avoiding undefined C behavior

=cut
*/
#ifndef CAST_INLINE
#define I_32(what) (cast_i32((NV)(what)))
#define U_32(what) (cast_ulong((NV)(what)))
#define I_V(what) (cast_iv((NV)(what)))
#define U_V(what) (cast_uv((NV)(what)))
#else
#define I_32(n) ((n) < I32_MAX_P1 ? ((n) < I32_MIN ? I32_MIN : (I32) (n)) \
                  : ((n) < U32_MAX_P1 ? (I32)(U32) (n) \
                     : ((n) > 0 ? (I32) U32_MAX : 0 /* NaN */)))
#define U_32(n) ((n) < 0.0 ? ((n) < I32_MIN ? (UV) I32_MIN : (U32)(I32) (n)) \
                  : ((n) < U32_MAX_P1 ? (U32) (n) \
                     : ((n) > 0 ? U32_MAX : 0 /* NaN */)))
#define I_V(n) (LIKELY((n) < IV_MAX_P1) ? (UNLIKELY((n) < IV_MIN) ? IV_MIN : (IV) (n)) \
                  : (LIKELY((n) < UV_MAX_P1) ? (IV)(UV) (n) \
                     : ((n) > 0 ? (IV)UV_MAX : 0 /* NaN */)))
#define U_V(n) ((n) < 0.0 ? (UNLIKELY((n) < IV_MIN) ? (UV) IV_MIN : (UV)(IV) (n)) \
                  : (LIKELY((n) < UV_MAX_P1) ? (UV) (n) \
                     : ((n) > 0 ? UV_MAX : 0 /* NaN */)))
#endif

#define U_S(what) ((U16)U_32(what))
#define U_I(what) ((unsigned int)U_32(what))
#define U_L(what) U_32(what)

/*
=for apidoc_section $integer
=for apidoc Amn|IV|IV_MAX
The largest signed integer that fits in an IV on this platform.

=for apidoc Amn|IV|IV_MIN
The negative signed integer furthest away from 0 that fits in an IV on this
platform.

=for apidoc Amn|UV|UV_MAX
The largest unsigned integer that fits in a UV on this platform.

=for apidoc Amn|UV|UV_MIN
The smallest unsigned integer that fits in a UV on this platform.  It should
equal zero.

=cut
*/

#ifdef HAS_SIGNBIT
#  ifndef Perl_signbit
#    define Perl_signbit signbit
#  endif
#endif

/* These do not care about the fractional part, only about the range. */
#define NV_WITHIN_IV(nv) (I_V(nv) >= IV_MIN && I_V(nv) <= IV_MAX)
#define NV_WITHIN_UV(nv) ((nv)>=0.0 && U_V(nv) >= UV_MIN && U_V(nv) <= UV_MAX)

/* Used with UV/IV arguments: */
                                        /* XXXX: need to speed it up */
#define CLUMP_2UV(iv)	((iv) < 0 ? 0 : (UV)(iv))
#define CLUMP_2IV(uv)	((uv) > (UV)IV_MAX ? IV_MAX : (IV)(uv))

#ifndef MAXSYSFD
#   define MAXSYSFD 2
#endif

#ifndef __cplusplus
#if !defined(WIN32)
Uid_t getuid (void);
Uid_t geteuid (void);
Gid_t getgid (void);
Gid_t getegid (void);
#endif
#endif

#ifndef Perl_debug_log
#  define Perl_debug_log	PerlIO_stderr()
#endif

#ifndef Perl_error_log
#  define Perl_error_log	(PL_stderrgv			\
                                 && isGV(PL_stderrgv)		\
                                 && GvIOp(PL_stderrgv)          \
                                 && IoOFP(GvIOp(PL_stderrgv))	\
                                 ? IoOFP(GvIOp(PL_stderrgv))	\
                                 : PerlIO_stderr())
#endif


#define DEBUG_p_FLAG		0x00000001 /*      1 */
#define DEBUG_s_FLAG		0x00000002 /*      2 */
#define DEBUG_l_FLAG		0x00000004 /*      4 */
#define DEBUG_t_FLAG		0x00000008 /*      8 */
#define DEBUG_o_FLAG		0x00000010 /*     16 */
#define DEBUG_c_FLAG		0x00000020 /*     32 */
#define DEBUG_P_FLAG		0x00000040 /*     64 */
#define DEBUG_m_FLAG		0x00000080 /*    128 */
#define DEBUG_f_FLAG		0x00000100 /*    256 */
#define DEBUG_r_FLAG		0x00000200 /*    512 */
#define DEBUG_x_FLAG		0x00000400 /*   1024 */
#define DEBUG_u_FLAG		0x00000800 /*   2048 */
/* U is reserved for Unofficial, exploratory hacking */
#define DEBUG_U_FLAG		0x00001000 /*   4096 */
#define DEBUG_h_FLAG            0x00002000 /*   8192 */
#define DEBUG_X_FLAG		0x00004000 /*  16384 */
#define DEBUG_D_FLAG		0x00008000 /*  32768 */
#define DEBUG_S_FLAG		0x00010000 /*  65536 */
#define DEBUG_T_FLAG		0x00020000 /* 131072 */
#define DEBUG_R_FLAG		0x00040000 /* 262144 */
#define DEBUG_J_FLAG		0x00080000 /* 524288 */
#define DEBUG_v_FLAG		0x00100000 /*1048576 */
#define DEBUG_C_FLAG		0x00200000 /*2097152 */
#define DEBUG_A_FLAG		0x00400000 /*4194304 */
#define DEBUG_q_FLAG		0x00800000 /*8388608 */
#define DEBUG_M_FLAG		0x01000000 /*16777216*/
#define DEBUG_B_FLAG		0x02000000 /*33554432*/
#define DEBUG_L_FLAG		0x04000000 /*67108864*/
#define DEBUG_i_FLAG		0x08000000 /*134217728*/
#define DEBUG_y_FLAG		0x10000000 /*268435456*/
#define DEBUG_MASK		0x1FFFEFFF /* mask of all the standard flags */

#define DEBUG_DB_RECURSE_FLAG	0x40000000
#define DEBUG_TOP_FLAG		0x80000000 /* -D was given --> PL_debug |= FLAG */

/* Both flags have to be set */
#  define DEBUG_BOTH_FLAGS_TEST_(flag1, flag2)              \
            UNLIKELY((PL_debug & ((flag1)|(flag2)))         \
                              == ((flag1)|(flag2)))

#  define DEBUG_p_TEST_ UNLIKELY(PL_debug & DEBUG_p_FLAG)
#  define DEBUG_s_TEST_ UNLIKELY(PL_debug & DEBUG_s_FLAG)
#  define DEBUG_l_TEST_ UNLIKELY(PL_debug & DEBUG_l_FLAG)
#  define DEBUG_t_TEST_ UNLIKELY(PL_debug & DEBUG_t_FLAG)
#  define DEBUG_o_TEST_ UNLIKELY(PL_debug & DEBUG_o_FLAG)
#  define DEBUG_c_TEST_ UNLIKELY(PL_debug & DEBUG_c_FLAG)
#  define DEBUG_P_TEST_ UNLIKELY(PL_debug & DEBUG_P_FLAG)
#  define DEBUG_m_TEST_ UNLIKELY(PL_debug & DEBUG_m_FLAG)
#  define DEBUG_f_TEST_ UNLIKELY(PL_debug & DEBUG_f_FLAG)
#  define DEBUG_r_TEST_ UNLIKELY(PL_debug & DEBUG_r_FLAG)
#  define DEBUG_x_TEST_ UNLIKELY(PL_debug & DEBUG_x_FLAG)
#  define DEBUG_u_TEST_ UNLIKELY(PL_debug & DEBUG_u_FLAG)
#  define DEBUG_U_TEST_ UNLIKELY(PL_debug & DEBUG_U_FLAG)
#  define DEBUG_h_TEST_ UNLIKELY(PL_debug & DEBUG_h_FLAG)
#  define DEBUG_X_TEST_ UNLIKELY(PL_debug & DEBUG_X_FLAG)
#  define DEBUG_D_TEST_ UNLIKELY(PL_debug & DEBUG_D_FLAG)
#  define DEBUG_S_TEST_ UNLIKELY(PL_debug & DEBUG_S_FLAG)
#  define DEBUG_T_TEST_ UNLIKELY(PL_debug & DEBUG_T_FLAG)
#  define DEBUG_R_TEST_ UNLIKELY(PL_debug & DEBUG_R_FLAG)
#  define DEBUG_J_TEST_ UNLIKELY(PL_debug & DEBUG_J_FLAG)
#  define DEBUG_v_TEST_ UNLIKELY(PL_debug & DEBUG_v_FLAG)
#  define DEBUG_C_TEST_ UNLIKELY(PL_debug & DEBUG_C_FLAG)
#  define DEBUG_A_TEST_ UNLIKELY(PL_debug & DEBUG_A_FLAG)
#  define DEBUG_q_TEST_ UNLIKELY(PL_debug & DEBUG_q_FLAG)
#  define DEBUG_M_TEST_ UNLIKELY(PL_debug & DEBUG_M_FLAG)
#  define DEBUG_B_TEST_ UNLIKELY(PL_debug & DEBUG_B_FLAG)

/* Locale initialization comes earlier than PL_debug gets set,
 * DEBUG_LOCALE_INITIALIZATION_, if defined, will be set early enough */
#  ifndef DEBUG_LOCALE_INITIALIZATION_
#    define DEBUG_LOCALE_INITIALIZATION_ 0
#  endif
#  define DEBUG_L_TEST_                                                 \
        (   UNLIKELY(DEBUG_LOCALE_INITIALIZATION_)                      \
         || UNLIKELY(PL_debug & DEBUG_L_FLAG))
#  define DEBUG_Lv_TEST_                                                \
        (   UNLIKELY(DEBUG_LOCALE_INITIALIZATION_)                      \
         || UNLIKELY(DEBUG_BOTH_FLAGS_TEST_(DEBUG_L_FLAG, DEBUG_v_FLAG)))
#  define DEBUG_i_TEST_ UNLIKELY(PL_debug & DEBUG_i_FLAG)
#  define DEBUG_y_TEST_ UNLIKELY(PL_debug & DEBUG_y_FLAG)
#  define DEBUG_Xv_TEST_ DEBUG_BOTH_FLAGS_TEST_(DEBUG_X_FLAG, DEBUG_v_FLAG)
#  define DEBUG_Uv_TEST_ DEBUG_BOTH_FLAGS_TEST_(DEBUG_U_FLAG, DEBUG_v_FLAG)
#  define DEBUG_Pv_TEST_ DEBUG_BOTH_FLAGS_TEST_(DEBUG_P_FLAG, DEBUG_v_FLAG)
#  define DEBUG_yv_TEST_ DEBUG_BOTH_FLAGS_TEST_(DEBUG_y_FLAG, DEBUG_v_FLAG)

#ifdef DEBUGGING

#  define DEBUG_p_TEST DEBUG_p_TEST_
#  define DEBUG_s_TEST DEBUG_s_TEST_
#  define DEBUG_l_TEST DEBUG_l_TEST_
#  define DEBUG_t_TEST DEBUG_t_TEST_
#  define DEBUG_o_TEST DEBUG_o_TEST_
#  define DEBUG_c_TEST DEBUG_c_TEST_
#  define DEBUG_P_TEST DEBUG_P_TEST_
#  define DEBUG_m_TEST DEBUG_m_TEST_
#  define DEBUG_f_TEST DEBUG_f_TEST_
#  define DEBUG_r_TEST DEBUG_r_TEST_
#  define DEBUG_x_TEST DEBUG_x_TEST_
#  define DEBUG_u_TEST DEBUG_u_TEST_
#  define DEBUG_U_TEST DEBUG_U_TEST_
#  define DEBUG_h_TEST DEBUG_h_TEST_
#  define DEBUG_X_TEST DEBUG_X_TEST_
#  define DEBUG_D_TEST DEBUG_D_TEST_
#  define DEBUG_S_TEST DEBUG_S_TEST_
#  define DEBUG_T_TEST DEBUG_T_TEST_
#  define DEBUG_R_TEST DEBUG_R_TEST_
#  define DEBUG_J_TEST DEBUG_J_TEST_
#  define DEBUG_v_TEST DEBUG_v_TEST_
#  define DEBUG_C_TEST DEBUG_C_TEST_
#  define DEBUG_A_TEST DEBUG_A_TEST_
#  define DEBUG_q_TEST DEBUG_q_TEST_
#  define DEBUG_M_TEST DEBUG_M_TEST_
#  define DEBUG_B_TEST DEBUG_B_TEST_
#  define DEBUG_L_TEST DEBUG_L_TEST_
#  define DEBUG_i_TEST DEBUG_i_TEST_
#  define DEBUG_y_TEST DEBUG_y_TEST_
#  define DEBUG_Xv_TEST DEBUG_Xv_TEST_
#  define DEBUG_Uv_TEST DEBUG_Uv_TEST_
#  define DEBUG_Pv_TEST DEBUG_Pv_TEST_
#  define DEBUG_Lv_TEST DEBUG_Lv_TEST_
#  define DEBUG_yv_TEST DEBUG_yv_TEST_

#  define PERL_DEB(a)                  a
#  define PERL_DEB2(a,b)               a
#  define PERL_DEBUG(a) if (PL_debug)  a
#  define DEBUG_p(a) if (DEBUG_p_TEST) a
#  define DEBUG_s(a) if (DEBUG_s_TEST) a
#  define DEBUG_l(a) if (DEBUG_l_TEST) a
#  define DEBUG_t(a) if (DEBUG_t_TEST) a
#  define DEBUG_o(a) if (DEBUG_o_TEST) a
#  define DEBUG_c(a) if (DEBUG_c_TEST) a
#  define DEBUG_P(a) if (DEBUG_P_TEST) a

     /* Temporarily turn off memory debugging in case the a
      * does memory allocation, either directly or indirectly. */
#  define DEBUG_m(a)  \
    STMT_START {					                \
        if (PERL_GET_INTERP) {                                          \
                                dTHX;                                   \
                                if (DEBUG_m_TEST) {                     \
                                    PL_debug &= ~DEBUG_m_FLAG;          \
                                    a;                                  \
                                    PL_debug |= DEBUG_m_FLAG;           \
                                }                                       \
                              }                                         \
    } STMT_END

/* These allow you to customize your debugging output  for specialized,
 * generally temporary ad-hoc purposes.  For example, if you need 'errno'
 * preserved, you can add definitions to these macros (either in this file for
 * the whole program, or before the #include "perl.h" in a particular .c file
 * you're trying to debug) and recompile:
 *
 * #define DEBUG_PRE_STMTS   dSAVE_ERRNO;
 * #define DEBUG_POST_STMTS  RESTORE_ERRNO;
 *
 * Other potential things include displaying timestamps, location information,
 * which thread, etc.  Heres an example with both errno and location info:
 *
 * #define DEBUG_PRE_STMTS   dSAVE_ERRNO;  \
 *              PerlIO_printf(Perl_debug_log, "%s:%d: ", __FILE__, __LINE__);
 * #define DEBUG_POST  RESTORE_ERRNO;
 *
 * All DEBUG statements in the compiled scope will be have these extra
 * statements compiled in; they will be executed only for the DEBUG statements
 * whose flags are turned on.
 */
#ifndef DEBUG_PRE_STMTS
#  define DEBUG_PRE_STMTS
#endif
#ifndef DEBUG_POST_STMTS
#  define DEBUG_POST_STMTS
#endif

#  define DEBUG__(t, a)                                                 \
        STMT_START {                                                    \
            if (t) {                                                    \
                DEBUG_PRE_STMTS a; DEBUG_POST_STMTS                     \
            }                                                           \
        } STMT_END

#  define DEBUG_f(a) DEBUG__(DEBUG_f_TEST, a)

/* For re_comp.c, re_exec.c, assume -Dr has been specified */
#  ifdef PERL_EXT_RE_BUILD
#    define DEBUG_r(a) STMT_START {                                     \
                            DEBUG_PRE_STMTS a; DEBUG_POST_STMTS         \
                       } STMT_END;
#  else
#    define DEBUG_r(a) DEBUG__(DEBUG_r_TEST, a)
#  endif /* PERL_EXT_RE_BUILD */

#  define DEBUG_x(a) DEBUG__(DEBUG_x_TEST, a)
#  define DEBUG_u(a) DEBUG__(DEBUG_u_TEST, a)
#  define DEBUG_U(a) DEBUG__(DEBUG_U_TEST, a)
#  define DEBUG_X(a) DEBUG__(DEBUG_X_TEST, a)
#  define DEBUG_D(a) DEBUG__(DEBUG_D_TEST, a)
#  define DEBUG_Xv(a) DEBUG__(DEBUG_Xv_TEST, a)
#  define DEBUG_Uv(a) DEBUG__(DEBUG_Uv_TEST, a)
#  define DEBUG_Pv(a) DEBUG__(DEBUG_Pv_TEST, a)
#  define DEBUG_Lv(a) DEBUG__(DEBUG_Lv_TEST, a)
#  define DEBUG_yv(a) DEBUG__(DEBUG_yv_TEST, a)

#  define DEBUG_S(a) DEBUG__(DEBUG_S_TEST, a)
#  define DEBUG_T(a) DEBUG__(DEBUG_T_TEST, a)
#  define DEBUG_R(a) DEBUG__(DEBUG_R_TEST, a)
#  define DEBUG_v(a) DEBUG__(DEBUG_v_TEST, a)
#  define DEBUG_C(a) DEBUG__(DEBUG_C_TEST, a)
#  define DEBUG_A(a) DEBUG__(DEBUG_A_TEST, a)
#  define DEBUG_q(a) DEBUG__(DEBUG_q_TEST, a)
#  define DEBUG_M(a) DEBUG__(DEBUG_M_TEST, a)
#  define DEBUG_B(a) DEBUG__(DEBUG_B_TEST, a)
#  define DEBUG_L(a) DEBUG__(DEBUG_L_TEST, a)
#  define DEBUG_i(a) DEBUG__(DEBUG_i_TEST, a)
#  define DEBUG_y(a) DEBUG__(DEBUG_y_TEST, a)

#else /* ! DEBUGGING below */

#  define DEBUG_p_TEST (0)
#  define DEBUG_s_TEST (0)
#  define DEBUG_l_TEST (0)
#  define DEBUG_t_TEST (0)
#  define DEBUG_o_TEST (0)
#  define DEBUG_c_TEST (0)
#  define DEBUG_P_TEST (0)
#  define DEBUG_m_TEST (0)
#  define DEBUG_f_TEST (0)
#  define DEBUG_r_TEST (0)
#  define DEBUG_x_TEST (0)
#  define DEBUG_u_TEST (0)
#  define DEBUG_U_TEST (0)
#  define DEBUG_h_TEST (0)
#  define DEBUG_X_TEST (0)
#  define DEBUG_D_TEST (0)
#  define DEBUG_S_TEST (0)
#  define DEBUG_T_TEST (0)
#  define DEBUG_R_TEST (0)
#  define DEBUG_J_TEST (0)
#  define DEBUG_v_TEST (0)
#  define DEBUG_C_TEST (0)
#  define DEBUG_A_TEST (0)
#  define DEBUG_q_TEST (0)
#  define DEBUG_M_TEST (0)
#  define DEBUG_B_TEST (0)
#  define DEBUG_L_TEST (0)
#  define DEBUG_i_TEST (0)
#  define DEBUG_y_TEST (0)
#  define DEBUG_Xv_TEST (0)
#  define DEBUG_Uv_TEST (0)
#  define DEBUG_Pv_TEST (0)
#  define DEBUG_Lv_TEST (0)
#  define DEBUG_yv_TEST (0)

#  define PERL_DEB(a)
#  define PERL_DEB2(a,b)               b
#  define PERL_DEBUG(a)
#  define DEBUG_p(a)
#  define DEBUG_s(a)
#  define DEBUG_l(a)
#  define DEBUG_t(a)
#  define DEBUG_o(a)
#  define DEBUG_c(a)
#  define DEBUG_P(a)
#  define DEBUG_m(a)
#  define DEBUG_f(a)
#  define DEBUG_r(a)
#  define DEBUG_x(a)
#  define DEBUG_u(a)
#  define DEBUG_U(a)
#  define DEBUG_X(a)
#  define DEBUG_D(a)
#  define DEBUG_S(a)
#  define DEBUG_T(a)
#  define DEBUG_R(a)
#  define DEBUG_v(a)
#  define DEBUG_C(a)
#  define DEBUG_A(a)
#  define DEBUG_q(a)
#  define DEBUG_M(a)
#  define DEBUG_B(a)
#  define DEBUG_L(a)
#  define DEBUG_i(a)
#  define DEBUG_y(a)
#  define DEBUG_Xv(a)
#  define DEBUG_Uv(a)
#  define DEBUG_Pv(a)
#  define DEBUG_Lv(a)
#  define DEBUG_yv(a)
#endif /* DEBUGGING */


#define DEBUG_SCOPE(where) \
    DEBUG_l( \
    Perl_deb(aTHX_ "%s scope %ld (savestack=%ld) at %s:%d\n",	\
                    where, (long)PL_scopestack_ix, (long)PL_savestack_ix, \
                    __FILE__, __LINE__));

/* Keep the old croak based assert for those who want it, and as a fallback if
   the platform is so heretically non-ANSI that it can't assert.  */

#define Perl_assert(what)	PERL_DEB2( 				\
        ((what) ? ((void) 0) :						\
            (Perl_croak_nocontext("Assertion %s failed: file \"" __FILE__ \
                        "\", line %d", STRINGIFY(what), __LINE__),	\
             (void) 0)), ((void)0))

/* assert() gets defined if DEBUGGING.
 * If no DEBUGGING, the <assert.h> has not been included. */
#ifndef assert
#  define assert(what)	Perl_assert(what)
#endif
#ifdef DEBUGGING
#  define assert_(what)	assert(what),
#else
#  define assert_(what)
#endif

struct ufuncs {
    I32 (*uf_val)(pTHX_ IV, SV*);
    I32 (*uf_set)(pTHX_ IV, SV*);
    IV uf_index;
};

/* In pre-5.7-Perls the PERL_MAGIC_uvar magic didn't get the thread context.
 * XS code wanting to be backward compatible can do something
 * like the following:

#ifndef PERL_MG_UFUNC
#define PERL_MG_UFUNC(name,ix,sv) I32 name(IV ix, SV *sv)
#endif

static PERL_MG_UFUNC(foo_get, index, val)
{
    sv_setsv(val, ...);
    return TRUE;
}

-- Doug MacEachern

*/

#ifndef PERL_MG_UFUNC
#define PERL_MG_UFUNC(name,ix,sv) I32 name(pTHX_ IV ix, SV *sv)
#endif

#include <math.h>
#ifdef __VMS
     /* isfinite and others are here rather than in math.h as C99 stipulates */
#    include <fp.h>
#endif

#ifndef __cplusplus
#  if !defined(WIN32) && !defined(VMS)
#ifndef crypt
char *crypt (const char*, const char*);
#endif
#  endif /* !WIN32 */
#  ifndef WIN32
#    ifndef getlogin
char *getlogin (void);
#    endif
#  endif /* !WIN32 */
#endif /* !__cplusplus */

/* Fixme on VMS.  This needs to be a run-time, not build time options */
/* Also rename() is affected by this */
#ifdef UNLINK_ALL_VERSIONS /* Currently only makes sense for VMS */
#define UNLINK unlnk
I32 unlnk (pTHX_ const char*);
#else
#define UNLINK PerlLIO_unlink
#endif

/* some versions of glibc are missing the setresuid() proto */
#if defined(HAS_SETRESUID) && !defined(HAS_SETRESUID_PROTO)
int setresuid(uid_t ruid, uid_t euid, uid_t suid);
#endif
/* some versions of glibc are missing the setresgid() proto */
#if defined(HAS_SETRESGID) && !defined(HAS_SETRESGID_PROTO)
int setresgid(gid_t rgid, gid_t egid, gid_t sgid);
#endif

#ifndef HAS_SETREUID
#  ifdef HAS_SETRESUID
#    define setreuid(r,e) setresuid(r,e,(Uid_t)-1)
#    define HAS_SETREUID
#  endif
#endif
#ifndef HAS_SETREGID
#  ifdef HAS_SETRESGID
#    define setregid(r,e) setresgid(r,e,(Gid_t)-1)
#    define HAS_SETREGID
#  endif
#endif

/* Sighandler_t defined in iperlsys.h */

#ifdef HAS_SIGACTION
typedef struct sigaction Sigsave_t;
#else
typedef Sighandler_t Sigsave_t;
#endif

#define SCAN_DEF 0
#define SCAN_TR 1
#define SCAN_REPL 2

#ifdef DEBUGGING
# ifndef register
#  define register
# endif
# define RUNOPS_DEFAULT Perl_runops_debug
#else
# define RUNOPS_DEFAULT Perl_runops_standard
#endif

#if defined(USE_PERLIO)
EXTERN_C void PerlIO_teardown(void);
# ifdef USE_ITHREADS
#  define PERLIO_INIT MUTEX_INIT(&PL_perlio_mutex)
#  define PERLIO_TERM 				\
        STMT_START {				\
                PerlIO_teardown();		\
                MUTEX_DESTROY(&PL_perlio_mutex);\
        } STMT_END
# else
#  define PERLIO_INIT
#  define PERLIO_TERM	PerlIO_teardown()
# endif
#else
#  define PERLIO_INIT
#  define PERLIO_TERM
#endif

#ifdef MYMALLOC
#  ifdef MUTEX_INIT_CALLS_MALLOC
#    define MALLOC_INIT					\
        STMT_START {					\
                PL_malloc_mutex = NULL;			\
                MUTEX_INIT(&PL_malloc_mutex);		\
        } STMT_END
#    define MALLOC_TERM					\
        STMT_START {					\
                perl_mutex tmp = PL_malloc_mutex;	\
                PL_malloc_mutex = NULL;			\
                MUTEX_DESTROY(&tmp);			\
        } STMT_END
#  else
#    define MALLOC_INIT MUTEX_INIT(&PL_malloc_mutex)
#    define MALLOC_TERM MUTEX_DESTROY(&PL_malloc_mutex)
#  endif
#else
#  define MALLOC_INIT
#  define MALLOC_TERM
#endif

#if defined(MULTIPLICITY)

struct perl_memory_debug_header;
struct perl_memory_debug_header {
  tTHX	interpreter;
#  if defined(PERL_POISON) || defined(PERL_DEBUG_READONLY_COW)
  MEM_SIZE size;
#  endif
  struct perl_memory_debug_header *prev;
  struct perl_memory_debug_header *next;
#  ifdef PERL_DEBUG_READONLY_COW
  bool readonly;
#  endif
};

#elif defined(PERL_DEBUG_READONLY_COW)

struct perl_memory_debug_header;
struct perl_memory_debug_header {
  MEM_SIZE size;
};

#endif

#if defined (PERL_TRACK_MEMPOOL) || defined (PERL_DEBUG_READONLY_COW)

#  define PERL_MEMORY_DEBUG_HEADER_SIZE \
        (sizeof(struct perl_memory_debug_header) + \
        (MEM_ALIGNBYTES - sizeof(struct perl_memory_debug_header) \
         %MEM_ALIGNBYTES) % MEM_ALIGNBYTES)

#else
#  define PERL_MEMORY_DEBUG_HEADER_SIZE	0
#endif

#ifdef PERL_TRACK_MEMPOOL
# ifdef PERL_DEBUG_READONLY_COW
#  define INIT_TRACK_MEMPOOL(header, interp)			\
        STMT_START {						\
                (header).interpreter = (interp);		\
                (header).prev = (header).next = &(header);	\
                (header).readonly = 0;				\
        } STMT_END
# else
#  define INIT_TRACK_MEMPOOL(header, interp)			\
        STMT_START {						\
                (header).interpreter = (interp);		\
                (header).prev = (header).next = &(header);	\
        } STMT_END
# endif
# else
#  define INIT_TRACK_MEMPOOL(header, interp)
#endif

#ifdef I_MALLOCMALLOC
/* Needed for malloc_size(), malloc_good_size() on some systems */
#  include <malloc/malloc.h>
#endif

#ifdef MYMALLOC
#  define Perl_safesysmalloc_size(where)	Perl_malloced_size(where)
#else
#  if defined(HAS_MALLOC_SIZE) && !defined(PERL_DEBUG_READONLY_COW)
#    ifdef PERL_TRACK_MEMPOOL
#	define Perl_safesysmalloc_size(where)			\
            (malloc_size(((char *)(where)) - PERL_MEMORY_DEBUG_HEADER_SIZE) - PERL_MEMORY_DEBUG_HEADER_SIZE)
#    else
#	define Perl_safesysmalloc_size(where) malloc_size(where)
#    endif
#  endif
#  ifdef HAS_MALLOC_GOOD_SIZE
#    ifdef PERL_TRACK_MEMPOOL
#	define Perl_malloc_good_size(how_much)			\
            (malloc_good_size((how_much) + PERL_MEMORY_DEBUG_HEADER_SIZE) - PERL_MEMORY_DEBUG_HEADER_SIZE)
#    else
#	define Perl_malloc_good_size(how_much) malloc_good_size(how_much)
#    endif
#  else
/* Having this as the identity operation makes some code simpler.  */
#	define Perl_malloc_good_size(how_much)	(how_much)
#  endif
#endif

typedef int (*runops_proc_t)(pTHX);
typedef void (*share_proc_t) (pTHX_ SV *sv);
typedef int  (*thrhook_proc_t) (pTHX);
typedef OP* (*PPADDR_t[]) (pTHX);
typedef bool (*destroyable_proc_t) (pTHX_ SV *sv);
typedef void (*despatch_signals_proc_t) (pTHX);

#if defined(__DYNAMIC__) && defined(PERL_DARWIN) && defined(PERL_CORE)
#  include <crt_externs.h>	/* for the env array */
#  define environ (*_NSGetEnviron())
#elif defined(USE_ENVIRON_ARRAY) && !defined(environ)
   /* VMS and some other platforms don't use the environ array */
EXTERN_C char **environ;  /* environment variables supplied via exec */
#endif

#define PERL_PATCHLEVEL_H_IMPLICIT
#include "patchlevel.h"
#undef PERL_PATCHLEVEL_H_IMPLICIT

#define PERL_VERSION_STRING	STRINGIFY(PERL_REVISION) "." \
                                STRINGIFY(PERL_VERSION) "." \
                                STRINGIFY(PERL_SUBVERSION)

#define PERL_API_VERSION_STRING	STRINGIFY(PERL_API_REVISION) "." \
                                STRINGIFY(PERL_API_VERSION) "." \
                                STRINGIFY(PERL_API_SUBVERSION)

START_EXTERN_C

/* handy constants */
EXTCONST char PL_warn_uninit[]
  INIT("Use of uninitialized value%s%s%s");
EXTCONST char PL_warn_uninit_sv[]
  INIT("Use of uninitialized value%" SVf "%s%s");
EXTCONST char PL_warn_nosemi[]
  INIT("Semicolon seems to be missing");
EXTCONST char PL_warn_reserved[]
  INIT("Unquoted string \"%s\" may clash with future reserved word");
EXTCONST char PL_warn_nl[]
  INIT("Unsuccessful %s on filename containing newline");
EXTCONST char PL_no_wrongref[]
  INIT("Can't use %s ref as %s ref");
/* The core no longer needs this here. If you require the string constant,
   please inline a copy into your own code.  */
EXTCONST char PL_no_symref[] __attribute__deprecated__
  INIT("Can't use string (\"%.32s\") as %s ref while \"strict refs\" in use");
EXTCONST char PL_no_symref_sv[]
  INIT("Can't use string (\"%" SVf32 "\"%s) as %s ref while \"strict refs\" in use");

EXTCONST char PL_no_usym[]
  INIT("Can't use an undefined value as %s reference");
EXTCONST char PL_no_aelem[]
  INIT("Modification of non-creatable array value attempted, subscript %d");
EXTCONST char PL_no_helem_sv[]
  INIT("Modification of non-creatable hash value attempted, subscript \"%" SVf "\"");
EXTCONST char PL_no_modify[]
  INIT("Modification of a read-only value attempted");
EXTCONST char PL_no_mem[sizeof("Out of memory!\n")]
  INIT("Out of memory!\n");
EXTCONST char PL_no_security[]
  INIT("Insecure dependency in %s%s");
EXTCONST char PL_no_sock_func[]
  INIT("Unsupported socket function \"%s\" called");
EXTCONST char PL_no_dir_func[]
  INIT("Unsupported directory function \"%s\" called");
EXTCONST char PL_no_func[]
  INIT("The %s function is unimplemented");
EXTCONST char PL_no_myglob[]
  INIT("\"%s\" %s %s can't be in a package");
EXTCONST char PL_no_localize_ref[]
  INIT("Can't localize through a reference");
EXTCONST char PL_memory_wrap[]
  INIT("panic: memory wrap");
EXTCONST char PL_extended_cp_format[]
  INIT("Code point 0x%" UVXf " is not Unicode, requires a Perl extension,"
                             " and so is not portable");
EXTCONST char PL_Yes[]
  INIT("1");
EXTCONST char PL_No[]
  INIT("");
EXTCONST char PL_Zero[]
  INIT("0");

/*
=for apidoc_section $numeric
=for apidoc AmTuU|const char *|PL_hexdigit|U8 value

This array, indexed by an integer, converts that value into the character that
represents it.  For example, if the input is 8, the return will be a string
whose first character is '8'.  What is actually returned is a pointer into a
string.  All you are interested in is the first character of that string.  To
get uppercase letters (for the values 10..15), add 16 to the index.  Hence,
C<PL_hexdigit[11]> is C<'b'>, and C<PL_hexdigit[11+16]> is C<'B'>.  Adding 16
to an index whose representation is '0'..'9' yields the same as not adding 16.
Indices outside the range 0..31 result in (bad) undedefined behavior.

=cut
*/
EXTCONST char PL_hexdigit[]
  INIT("0123456789abcdef0123456789ABCDEF");

EXT char PL_WARN_ALL
  INIT(0);
EXT char PL_WARN_NONE
  INIT(0);

/* This is constant on most architectures, a global on OS/2 */
#ifndef OS2
EXTCONST char PL_sh_path[]
  INIT(SH_PATH); /* full path of shell */
#endif

#ifdef CSH
EXTCONST char PL_cshname[]
  INIT(CSH);
#  define PL_cshlen	(sizeof(CSH "") - 1)
#endif

/* These are baked at compile time into any shared perl library.
   In future releases this will allow us in main() to sanity test the
   library we're linking against.  */

EXTCONST U8 PL_revision
  INIT(PERL_REVISION);
EXTCONST U8 PL_version
  INIT(PERL_VERSION);
EXTCONST U8 PL_subversion
  INIT(PERL_SUBVERSION);

EXTCONST char PL_uuemap[65]
  INIT("`!\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_");

/* a special string address whose value is "isa", but which perl knows
 * to treat as if it were really "DOES" when printing the method name in
 *  the "Can't call method '%s'" error message */
EXTCONST char PL_isa_DOES[]
  INIT("isa");

#ifdef DOINIT
EXTCONST char PL_uudmap[256] =
#  ifdef PERL_MICRO
#    include "uuudmap.h"
#  else
#    include "uudmap.h"
#  endif
;
EXTCONST char PL_bitcount[256] =
#  ifdef PERL_MICRO
#    include "ubitcount.h"
#else
#    include "bitcount.h"
#  endif
;
EXTCONST char* const PL_sig_name[] = { SIG_NAME };
EXTCONST int         PL_sig_num[]  = { SIG_NUM };
#else
EXTCONST char PL_uudmap[256];
EXTCONST char PL_bitcount[256];
EXTCONST char* const PL_sig_name[];
EXTCONST int         PL_sig_num[];
#endif

/* fast conversion and case folding tables.  The folding tables complement the
 * fold, so that 'a' maps to 'A' and 'A' maps to 'a', ignoring more complicated
 * folds such as outside the range or to multiple characters. */

#ifdef DOINIT
#  ifndef EBCDIC

/* The EBCDIC fold table depends on the code page, and hence is found in
 * ebcdic_tables.h */

EXTCONST  unsigned char PL_fold[] = {
        0,	1,	2,	3,	4,	5,	6,	7,
        8,	9,	10,	11,	12,	13,	14,	15,
        16,	17,	18,	19,	20,	21,	22,	23,
        24,	25,	26,	27,	28,	29,	30,	31,
        32,	33,	34,	35,	36,	37,	38,	39,
        40,	41,	42,	43,	44,	45,	46,	47,
        48,	49,	50,	51,	52,	53,	54,	55,
        56,	57,	58,	59,	60,	61,	62,	63,
        64,	'a',	'b',	'c',	'd',	'e',	'f',	'g',
        'h',	'i',	'j',	'k',	'l',	'm',	'n',	'o',
        'p',	'q',	'r',	's',	't',	'u',	'v',	'w',
        'x',	'y',	'z',	91,	92,	93,	94,	95,
        96,	'A',	'B',	'C',	'D',	'E',	'F',	'G',
        'H',	'I',	'J',	'K',	'L',	'M',	'N',	'O',
        'P',	'Q',	'R',	'S',	'T',	'U',	'V',	'W',
        'X',	'Y',	'Z',	123,	124,	125,	126,	127,
        128,	129,	130,	131,	132,	133,	134,	135,
        136,	137,	138,	139,	140,	141,	142,	143,
        144,	145,	146,	147,	148,	149,	150,	151,
        152,	153,	154,	155,	156,	157,	158,	159,
        160,	161,	162,	163,	164,	165,	166,	167,
        168,	169,	170,	171,	172,	173,	174,	175,
        176,	177,	178,	179,	180,	181,	182,	183,
        184,	185,	186,	187,	188,	189,	190,	191,
        192,	193,	194,	195,	196,	197,	198,	199,
        200,	201,	202,	203,	204,	205,	206,	207,
        208,	209,	210,	211,	212,	213,	214,	215,
        216,	217,	218,	219,	220,	221,	222,	223,
        224,	225,	226,	227,	228,	229,	230,	231,
        232,	233,	234,	235,	236,	237,	238,	239,
        240,	241,	242,	243,	244,	245,	246,	247,
        248,	249,	250,	251,	252,	253,	254,	255
};

EXTCONST  unsigned char PL_fold_latin1[] = {
    /* Full latin1 complement folding, except for three problematic code points:
     *	Micro sign (181 = 0xB5) and y with diearesis (255 = 0xFF) have their
     *	fold complements outside the Latin1 range, so can't match something
     *	that isn't in utf8.
     *	German lower case sharp s (223 = 0xDF) folds to two characters, 'ss',
     *	not one, so can't be represented in this table.
     *
     * All have to be specially handled */
        0,	1,	2,	3,	4,	5,	6,	7,
        8,	9,	10,	11,	12,	13,	14,	15,
        16,	17,	18,	19,	20,	21,	22,	23,
        24,	25,	26,	27,	28,	29,	30,	31,
        32,	33,	34,	35,	36,	37,	38,	39,
        40,	41,	42,	43,	44,	45,	46,	47,
        48,	49,	50,	51,	52,	53,	54,	55,
        56,	57,	58,	59,	60,	61,	62,	63,
        64,	'a',	'b',	'c',	'd',	'e',	'f',	'g',
        'h',	'i',	'j',	'k',	'l',	'm',	'n',	'o',
        'p',	'q',	'r',	's',	't',	'u',	'v',	'w',
        'x',	'y',	'z',	91,	92,	93,	94,	95,
        96,	'A',	'B',	'C',	'D',	'E',	'F',	'G',
        'H',	'I',	'J',	'K',	'L',	'M',	'N',	'O',
        'P',	'Q',	'R',	'S',	'T',	'U',	'V',	'W',
        'X',	'Y',	'Z',	123,	124,	125,	126,	127,
        128,	129,	130,	131,	132,	133,	134,	135,
        136,	137,	138,	139,	140,	141,	142,	143,
        144,	145,	146,	147,	148,	149,	150,	151,
        152,	153,	154,	155,	156,	157,	158,	159,
        160,	161,	162,	163,	164,	165,	166,	167,
        168,	169,	170,	171,	172,	173,	174,	175,
        176,	177,	178,	179,	180,	181 /*micro */,	182,	183,
        184,	185,	186,	187,	188,	189,	190,	191,
        192+32,	193+32,	194+32,	195+32,	196+32,	197+32,	198+32,	199+32,
        200+32,	201+32,	202+32,	203+32,	204+32,	205+32,	206+32,	207+32,
        208+32,	209+32,	210+32,	211+32,	212+32,	213+32,	214+32,	215,
        216+32,	217+32,	218+32,	219+32,	220+32,	221+32,	222+32,	223 /* ss */,
        224-32,	225-32,	226-32,	227-32,	228-32,	229-32,	230-32,	231-32,
        232-32,	233-32,	234-32,	235-32,	236-32,	237-32,	238-32,	239-32,
        240-32,	241-32,	242-32,	243-32,	244-32,	245-32,	246-32,	247,
        248-32,	249-32,	250-32,	251-32,	252-32,	253-32,	254-32,
        255 /* y with diaeresis */
};

/* If these tables are accessed through ebcdic, the access will be converted to
 * latin1 first */
EXTCONST  unsigned char PL_latin1_lc[] = {  /* lowercasing */
        0,	1,	2,	3,	4,	5,	6,	7,
        8,	9,	10,	11,	12,	13,	14,	15,
        16,	17,	18,	19,	20,	21,	22,	23,
        24,	25,	26,	27,	28,	29,	30,	31,
        32,	33,	34,	35,	36,	37,	38,	39,
        40,	41,	42,	43,	44,	45,	46,	47,
        48,	49,	50,	51,	52,	53,	54,	55,
        56,	57,	58,	59,	60,	61,	62,	63,
        64,	'a',	'b',	'c',	'd',	'e',	'f',	'g',
        'h',	'i',	'j',	'k',	'l',	'm',	'n',	'o',
        'p',	'q',	'r',	's',	't',	'u',	'v',	'w',
        'x',	'y',	'z',	91,	92,	93,	94,	95,
        96,	97,	98,	99,	100,	101,	102,	103,
        104,	105,	106,	107,	108,	109,	110,	111,
        112,	113,	114,	115,	116,	117,	118,	119,
        120,	121,	122,	123,	124,	125,	126,	127,
        128,	129,	130,	131,	132,	133,	134,	135,
        136,	137,	138,	139,	140,	141,	142,	143,
        144,	145,	146,	147,	148,	149,	150,	151,
        152,	153,	154,	155,	156,	157,	158,	159,
        160,	161,	162,	163,	164,	165,	166,	167,
        168,	169,	170,	171,	172,	173,	174,	175,
        176,	177,	178,	179,	180,	181,	182,	183,
        184,	185,	186,	187,	188,	189,	190,	191,
        192+32,	193+32,	194+32,	195+32,	196+32,	197+32,	198+32,	199+32,
        200+32,	201+32,	202+32,	203+32,	204+32,	205+32,	206+32,	207+32,
        208+32,	209+32,	210+32,	211+32,	212+32,	213+32,	214+32,	215,
        216+32,	217+32,	218+32,	219+32,	220+32,	221+32,	222+32,	223,
        224,	225,	226,	227,	228,	229,	230,	231,
        232,	233,	234,	235,	236,	237,	238,	239,
        240,	241,	242,	243,	244,	245,	246,	247,
        248,	249,	250,	251,	252,	253,	254,	255
};

/* upper and title case of latin1 characters, modified so that the three tricky
 * ones are mapped to 255 (which is one of the three) */
EXTCONST  unsigned char PL_mod_latin1_uc[] = {
        0,	1,	2,	3,	4,	5,	6,	7,
        8,	9,	10,	11,	12,	13,	14,	15,
        16,	17,	18,	19,	20,	21,	22,	23,
        24,	25,	26,	27,	28,	29,	30,	31,
        32,	33,	34,	35,	36,	37,	38,	39,
        40,	41,	42,	43,	44,	45,	46,	47,
        48,	49,	50,	51,	52,	53,	54,	55,
        56,	57,	58,	59,	60,	61,	62,	63,
        64,	65,	66,	67,	68,	69,	70,	71,
        72,	73,	74,	75,	76,	77,	78,	79,
        80,	81,	82,	83,	84,	85,	86,	87,
        88,	89,	90,	91,	92,	93,	94,	95,
        96,	'A',	'B',	'C',	'D',	'E',	'F',	'G',
        'H',	'I',	'J',	'K',	'L',	'M',	'N',	'O',
        'P',	'Q',	'R',	'S',	'T',	'U',	'V',	'W',
        'X',	'Y',	'Z',	123,	124,	125,	126,	127,
        128,	129,	130,	131,	132,	133,	134,	135,
        136,	137,	138,	139,	140,	141,	142,	143,
        144,	145,	146,	147,	148,	149,	150,	151,
        152,	153,	154,	155,	156,	157,	158,	159,
        160,	161,	162,	163,	164,	165,	166,	167,
        168,	169,	170,	171,	172,	173,	174,	175,
        176,	177,	178,	179,	180,	255 /*micro*/,	182,	183,
        184,	185,	186,	187,	188,	189,	190,	191,
        192,	193,	194,	195,	196,	197,	198,	199,
        200,	201,	202,	203,	204,	205,	206,	207,
        208,	209,	210,	211,	212,	213,	214,	215,
        216,	217,	218,	219,	220,	221,	222,
#    if    UNICODE_MAJOR_VERSION > 2                                        \
       || (UNICODE_MAJOR_VERSION == 2 && UNICODE_DOT_VERSION >= 1           \
                                      && UNICODE_DOT_DOT_VERSION >= 8)
                                                                255 /*sharp s*/,
#    else   /* uc(sharp s) is 'sharp s' itself in early unicode */
                                                                223,
#    endif
        224-32,	225-32,	226-32,	227-32,	228-32,	229-32,	230-32,	231-32,
        232-32,	233-32,	234-32,	235-32,	236-32,	237-32,	238-32,	239-32,
        240-32,	241-32,	242-32,	243-32,	244-32,	245-32,	246-32,	247,
        248-32,	249-32,	250-32,	251-32,	252-32,	253-32,	254-32,	255
};
#  endif  /* !EBCDIC, but still in DOINIT */
#else	/* ! DOINIT */
#  ifndef EBCDIC
EXTCONST unsigned char PL_fold[];
EXTCONST unsigned char PL_fold_latin1[];
EXTCONST unsigned char PL_mod_latin1_uc[];
EXTCONST unsigned char PL_latin1_lc[];
#   endif
#endif

/* Although only used for debugging, these constants must be available in
 * non-debugging builds too, since they're used in ext/re/re_exec.c,
 * which has DEBUGGING enabled always */
#ifdef DOINIT
EXTCONST char* const PL_block_type[] = {
        "NULL",
        "WHEN",
        "BLOCK",
        "GIVEN",
        "LOOP_ARY",
        "LOOP_LAZYSV",
        "LOOP_LAZYIV",
        "LOOP_LIST",
        "LOOP_PLAIN",
        "SUB",
        "FORMAT",
        "EVAL",
        "SUBST",
        "DEFER"
};
#else
EXTCONST char* PL_block_type[];
#endif

/* These are all the compile time options that affect binary compatibility.
   Other compile time options that are binary compatible are in perl.c
   (in S_Internals_V()). Both are combined for the output of perl -V
   However, this string will be embedded in any shared perl library, which will
   allow us add a comparison check in perlmain.c in the near future.  */
#ifdef DOINIT
EXTCONST char PL_bincompat_options[] =
#  ifdef DEBUG_LEAKING_SCALARS
                             " DEBUG_LEAKING_SCALARS"
#  endif
#  ifdef DEBUG_LEAKING_SCALARS_FORK_DUMP
                             " DEBUG_LEAKING_SCALARS_FORK_DUMP"
#  endif
#  ifdef HAS_TIMES
                             " HAS_TIMES"
#  endif
#  ifdef HAVE_INTERP_INTERN
                             " HAVE_INTERP_INTERN"
#  endif
#  ifdef MULTIPLICITY
                             " MULTIPLICITY"
#  endif
#  ifdef MYMALLOC
                             " MYMALLOC"
#  endif
#  ifdef NO_HASH_SEED
                             " NO_HASH_SEED"
#  endif
#  ifdef PERLIO_LAYERS
                             " PERLIO_LAYERS"
#  endif
#  ifdef PERL_DEBUG_READONLY_COW
                             " PERL_DEBUG_READONLY_COW"
#  endif
#  ifdef PERL_DEBUG_READONLY_OPS
                             " PERL_DEBUG_READONLY_OPS"
#  endif
#  ifdef PERL_HASH_FUNC_DEFINE
/* note that this is different from the others, PERL_HASH_FUNC_DEFINE
 * is a string which says which define was defined. */
                             " " PERL_HASH_FUNC_DEFINE
#  endif
#  ifdef PERL_HASH_USE_SBOX32
                             " PERL_HASH_USE_SBOX32"
#  else
                             " PERL_HASH_NO_SBOX32"
#  endif
#  ifdef PERL_IMPLICIT_SYS
                             " PERL_IMPLICIT_SYS"
#  endif
#  ifdef PERL_MICRO
                             " PERL_MICRO"
#  endif
#  ifdef PERL_POISON
                             " PERL_POISON"
#  endif
#  ifdef PERL_SAWAMPERSAND
                             " PERL_SAWAMPERSAND"
#  endif
#  ifdef PERL_TRACK_MEMPOOL
                             " PERL_TRACK_MEMPOOL"
#  endif
#  ifdef PERL_USES_PL_PIDSTATUS
                             " PERL_USES_PL_PIDSTATUS"
#  endif
#  ifdef USE_64_BIT_ALL
                             " USE_64_BIT_ALL"
#  endif
#  ifdef USE_64_BIT_INT
                             " USE_64_BIT_INT"
#  endif
#  ifdef USE_IEEE
                             " USE_IEEE"
#  endif
#  ifdef USE_ITHREADS
                             " USE_ITHREADS"
#  endif
#  ifdef USE_LARGE_FILES
                             " USE_LARGE_FILES"
#  endif
#  ifdef USE_LOCALE_COLLATE
                             " USE_LOCALE_COLLATE"
#  endif
#  ifdef USE_LOCALE_NUMERIC
                             " USE_LOCALE_NUMERIC"
#  endif
#  ifdef USE_LOCALE_TIME
                             " USE_LOCALE_TIME"
#  endif
#  ifdef USE_LONG_DOUBLE
                             " USE_LONG_DOUBLE"
#  endif
#  ifdef USE_PERLIO
                             " USE_PERLIO"
#  endif
#  ifdef USE_QUADMATH
                             " USE_QUADMATH"
#  endif
#  ifdef USE_REENTRANT_API
                             " USE_REENTRANT_API"
#  endif
#  ifdef USE_SOCKS
                             " USE_SOCKS"
#  endif
#  ifdef VMS_DO_SOCKETS
                             " VMS_DO_SOCKETS"
#  endif
#  ifdef VMS_SHORTEN_LONG_SYMBOLS
                             " VMS_SHORTEN_LONG_SYMBOLS"
#  endif
#  ifdef VMS_WE_ARE_CASE_SENSITIVE
                             " VMS_SYMBOL_CASE_AS_IS"
#  endif
    ""; /* keep this on a line by itself, WITH the empty string */
#else
EXTCONST char PL_bincompat_options[];
#endif

#ifndef PERL_SET_PHASE
#  define PERL_SET_PHASE(new_phase) \
    PERL_DTRACE_PROBE_PHASE(new_phase); \
    PL_phase = new_phase;
#endif

/* The interpreter phases. If these ever change, PL_phase_names right below will
 * need to be updated accordingly. */
enum perl_phase {
    PERL_PHASE_CONSTRUCT	= 0,
    PERL_PHASE_START		= 1,
    PERL_PHASE_CHECK		= 2,
    PERL_PHASE_INIT		= 3,
    PERL_PHASE_RUN		= 4,
    PERL_PHASE_END		= 5,
    PERL_PHASE_DESTRUCT		= 6
};

#ifdef DOINIT
EXTCONST char *const PL_phase_names[] = {
    "CONSTRUCT",
    "START",
    "CHECK",
    "INIT",
    "RUN",
    "END",
    "DESTRUCT"
};
#else
EXTCONST char *const PL_phase_names[];
#endif

/*
=for apidoc_section $utility

=for apidoc phase_name

Returns the given phase's name as a NUL-terminated string.

For example, to print a stack trace that includes the current
interpreter phase you might do:

    const char* phase_name = phase_name(PL_phase);
    mess("This is weird. (Perl phase: %s)", phase_name);

=cut
*/

#define phase_name(phase) (PL_phase_names[phase])

#ifndef PERL_CORE
/* Do not use this macro. It only exists for extensions that rely on PL_dirty
 * instead of using the newer PL_phase, which provides everything PL_dirty
 * provided, and more. */
#  define PL_dirty cBOOL(PL_phase == PERL_PHASE_DESTRUCT)

#  define PL_amagic_generation PL_na
#  define PL_encoding ((SV *)NULL)
#endif /* !PERL_CORE */

#define PL_hints PL_compiling.cop_hints
#define PL_maxo  MAXO

END_EXTERN_C

/*****************************************************************************/
/* This lexer/parser stuff is currently global since yacc is hard to reenter */
/*****************************************************************************/
/* XXX This needs to be revisited, since BEGIN makes yacc re-enter... */

#ifdef __Lynx__
/* LynxOS defines these in scsi.h which is included via ioctl.h */
#ifdef FORMAT
#undef FORMAT
#endif
#ifdef SPACE
#undef SPACE
#endif
#endif

#define LEX_NOTPARSING		11	/* borrowed from toke.c */

typedef enum {
    XOPERATOR,
    XTERM,
    XREF,
    XSTATE,
    XBLOCK,
    XATTRBLOCK, /* next token should be an attribute or block */
    XATTRTERM,  /* next token should be an attribute, or block in a term */
    XTERMBLOCK,
    XBLOCKTERM,
    XPOSTDEREF,
    XTERMORDORDOR /* evil hack */
    /* update exp_name[] in toke.c if adding to this enum */
} expectation;

#define KEY_sigvar 0xFFFF /* fake keyword representing a signature var */

/* Hints are now stored in a dedicated U32, so the bottom 8 bits are no longer
   special and there is no need for HINT_PRIVATE_MASK for COPs.

    NOTE: The typical module using these has the bit value hard-coded, so don't
    blindly change the values of these.

   If we run out of bits, the 2 locale ones could be combined.  The PARTIAL one
   is for "use locale 'FOO'" which excludes some categories.  It requires going
   to %^H to find out which are in and which are out.  This could be extended
   for the normal case of a plain HINT_LOCALE, so that %^H would be used for
   any locale form. */
#define HINT_INTEGER		0x00000001 /* integer pragma */
#define HINT_STRICT_REFS	0x00000002 /* strict pragma */
#define HINT_LOCALE		0x00000004 /* locale pragma */
#define HINT_BYTES		0x00000008 /* bytes pragma */
#define HINT_LOCALE_PARTIAL	0x00000010 /* locale, but a subset of categories */

#define HINT_EXPLICIT_STRICT_REFS	0x00000020 /* strict.pm */
#define HINT_EXPLICIT_STRICT_SUBS	0x00000040 /* strict.pm */
#define HINT_EXPLICIT_STRICT_VARS	0x00000080 /* strict.pm */

#define HINT_BLOCK_SCOPE	0x00000100
#define HINT_STRICT_SUBS	0x00000200 /* strict pragma */
#define HINT_STRICT_VARS	0x00000400 /* strict pragma */
#define HINT_UNI_8_BIT		0x00000800 /* unicode_strings feature */

/* The HINT_NEW_* constants are used by the overload pragma */
#define HINT_NEW_INTEGER	0x00001000
#define HINT_NEW_FLOAT		0x00002000
#define HINT_NEW_BINARY		0x00004000
#define HINT_NEW_STRING		0x00008000
#define HINT_NEW_RE		0x00010000
#define HINT_LOCALIZE_HH	0x00020000 /* %^H needs to be copied */
#define HINT_LEXICAL_IO_IN	0x00040000 /* ${^OPEN} is set for input */
#define HINT_LEXICAL_IO_OUT	0x00080000 /* ${^OPEN} is set for output */

#define HINT_RE_TAINT		0x00100000 /* re pragma */
#define HINT_RE_EVAL		0x00200000 /* re pragma */

#define HINT_FILETEST_ACCESS	0x00400000 /* filetest pragma */
#define HINT_UTF8		0x00800000 /* utf8 pragma */

#define HINT_NO_AMAGIC		0x01000000 /* overloading pragma */

#define HINT_RE_FLAGS		0x02000000 /* re '/xism' pragma */

#define HINT_FEATURE_MASK	0x3c000000 /* 4 bits for feature bundles */

                                /* Note: Used for HINT_M_VMSISH_*,
                                   currently defined by vms/vmsish.h:
                                0x40000000
                                0x80000000
                                 */

#define HINT_ALL_STRICT       HINT_STRICT_REFS \
                            | HINT_STRICT_SUBS \
                            | HINT_STRICT_VARS

#ifdef USE_STRICT_BY_DEFAULT
#define HINTS_DEFAULT            HINT_ALL_STRICT
#else
#define HINTS_DEFAULT            0
#endif

/* flags for PL_sawampersand */

#define SAWAMPERSAND_LEFT       1   /* saw $` */
#define SAWAMPERSAND_MIDDLE     2   /* saw $& */
#define SAWAMPERSAND_RIGHT      4   /* saw $' */

#ifndef PERL_SAWAMPERSAND
# define PL_sawampersand \
        (SAWAMPERSAND_LEFT|SAWAMPERSAND_MIDDLE|SAWAMPERSAND_RIGHT)
#endif

/* Used for debugvar magic */
#define DBVARMG_SINGLE  0
#define DBVARMG_TRACE   1
#define DBVARMG_SIGNAL  2
#define DBVARMG_COUNT   3

#define PL_DBsingle_iv  (PL_DBcontrol[DBVARMG_SINGLE])
#define PL_DBtrace_iv   (PL_DBcontrol[DBVARMG_TRACE])
#define PL_DBsignal_iv  (PL_DBcontrol[DBVARMG_SIGNAL])

/* Various states of the input record separator SV (rs) */
#define RsSNARF(sv)   (! SvOK(sv))
#define RsSIMPLE(sv)  (SvOK(sv) && (! SvPOK(sv) || SvCUR(sv)))
#define RsPARA(sv)    (SvPOK(sv) && ! SvCUR(sv))
#define RsRECORD(sv)  (SvROK(sv) && (SvIV(SvRV(sv)) > 0))

/* A struct for keeping various DEBUGGING related stuff,
 * neatly packed.  Currently only scratch variables for
 * constructing debug output are included.  Needed always,
 * not just when DEBUGGING, though, because of the re extension. c*/
struct perl_debug_pad {
  SV pad[3];
};

#define PERL_DEBUG_PAD(i)	&(PL_debug_pad.pad[i])
#define PERL_DEBUG_PAD_ZERO(i)	(SvPVX(PERL_DEBUG_PAD(i))[0] = 0, \
        (((XPV*) SvANY(PERL_DEBUG_PAD(i)))->xpv_cur = 0), \
        PERL_DEBUG_PAD(i))

/* Enable variables which are pointers to functions */
typedef void (*peep_t)(pTHX_ OP* o);
typedef regexp* (*regcomp_t) (pTHX_ char* exp, char* xend, PMOP* pm);
typedef I32     (*regexec_t) (pTHX_ regexp* prog, char* stringarg,
                                      char* strend, char* strbeg, I32 minend,
                                      SV* screamer, void* data, U32 flags);
typedef char*   (*re_intuit_start_t) (pTHX_ regexp *prog, SV *sv,
                                                char *strpos, char *strend,
                                                U32 flags,
                                                re_scream_pos_data *d);
typedef SV*	(*re_intuit_string_t) (pTHX_ regexp *prog);
typedef void	(*regfree_t) (pTHX_ struct regexp* r);
typedef regexp* (*regdupe_t) (pTHX_ const regexp* r, CLONE_PARAMS *param);
typedef I32     (*re_fold_t)(pTHX_ const char *, char const *, I32);

typedef void (*DESTRUCTORFUNC_NOCONTEXT_t) (void*);
typedef void (*DESTRUCTORFUNC_t) (pTHX_ void*);
typedef void (*SVFUNC_t) (pTHX_ SV* const);
typedef I32  (*SVCOMPARE_t) (pTHX_ SV* const, SV* const);
typedef void (*XSINIT_t) (pTHX);
typedef void (*ATEXIT_t) (pTHX_ void*);
typedef void (*XSUBADDR_t) (pTHX_ CV *);

enum Perl_custom_infix_precedence {
    /* These numbers are spaced out to give room to insert new values as
     * required. They form part of the ABI contract with XS::Parse::Infix so
     * they should not be changed within a stable release cycle, but they can
     * be freely altered during a development cycle because no ABI guarantees
     * are made at that time */
    INFIX_PREC_LOW             =  10, /* non-associative */
    INFIX_PREC_LOGICAL_OR_LOW  =  30, /* left-associative, as `or` */
    INFIX_PREC_LOGICAL_AND_LOW =  40, /* left-associative, as `and` */
    INFIX_PREC_ASSIGN          =  50, /* right-associative, as `=` */
    INFIX_PREC_LOGICAL_OR      =  70, /* left-associative, as `||` */
    INFIX_PREC_LOGICAL_AND     =  80, /* left-associative, as `&&` */
    INFIX_PREC_REL             =  90, /* non-associative, just below `==` */
    INFIX_PREC_ADD             = 110, /* left-associative, as `+` */
    INFIX_PREC_MUL             = 130, /* left-associative, as `*` */
    INFIX_PREC_POW             = 150, /* right-associative, as `**` */
    INFIX_PREC_HIGH            = 170, /* non-associative */
    /* Try to keep within the range of a U8 in case we need to split the field
     * and add flags */
};
struct Perl_custom_infix;
struct Perl_custom_infix {
    enum Perl_custom_infix_precedence prec;
    void (*parse)(pTHX_ SV **opdata, struct Perl_custom_infix *);  /* optional */
    OP *(*build_op)(pTHX_ SV **opdata, OP *lhs, OP *rhs, struct Perl_custom_infix *);
};

typedef OP* (*Perl_ppaddr_t)(pTHX);
typedef OP* (*Perl_check_t) (pTHX_ OP*);
typedef void(*Perl_ophook_t)(pTHX_ OP*);
typedef int (*Perl_keyword_plugin_t)(pTHX_ char*, STRLEN, OP**);
typedef STRLEN (*Perl_infix_plugin_t)(pTHX_ char*, STRLEN, struct Perl_custom_infix **);
typedef void(*Perl_cpeep_t)(pTHX_ OP *, OP *);

typedef void(*globhook_t)(pTHX);

#define KEYWORD_PLUGIN_DECLINE 0
#define KEYWORD_PLUGIN_STMT    1
#define KEYWORD_PLUGIN_EXPR    2

/* Interpreter exitlist entry */
typedef struct exitlistentry {
    void (*fn) (pTHX_ void*);
    void *ptr;
} PerlExitListEntry;

/* if you only have signal() and it resets on each signal, FAKE_PERSISTENT_SIGNAL_HANDLERS fixes */
/* These have to be before perlvars.h */
#if !defined(HAS_SIGACTION) && defined(VMS)
#  define  FAKE_PERSISTENT_SIGNAL_HANDLERS
#endif
/* if we're doing kill() with sys$sigprc on VMS, FAKE_DEFAULT_SIGNAL_HANDLERS */
#if defined(KILL_BY_SIGPRC)
#  define  FAKE_DEFAULT_SIGNAL_HANDLERS
#endif

#if !defined(MULTIPLICITY)

struct interpreter {
    char broiled;
};

#else

/* If we have multiple interpreters define a struct
   holding variables which must be per-interpreter
   If we don't have threads anything that would have
   be per-thread is per-interpreter.
*/

/* Set up PERLVAR macros for populating structs */
#  define PERLVAR(prefix,var,type) type prefix##var;

/* 'var' is an array of length 'n' */
#  define PERLVARA(prefix,var,n,type) type prefix##var[n];

/* initialize 'var' to init' */
#  define PERLVARI(prefix,var,type,init) type prefix##var;

/* like PERLVARI, but make 'var' a const */
#  define PERLVARIC(prefix,var,type,init) type prefix##var;

struct interpreter {
#  include "intrpvar.h"
};

EXTCONST U16 PL_interp_size
  INIT(sizeof(struct interpreter));

#  define PERL_INTERPRETER_SIZE_UPTO_MEMBER(member)			\
    STRUCT_OFFSET(struct interpreter, member) +				\
    sizeof(((struct interpreter*)0)->member)

/* This will be useful for subsequent releases, because this has to be the
   same in your libperl as in main(), else you have a mismatch and must abort.
*/
EXTCONST U16 PL_interp_size_5_18_0
  INIT(PERL_INTERPRETER_SIZE_UPTO_MEMBER(PERL_LAST_5_18_0_INTERP_MEMBER));


/* Done with PERLVAR macros for now ... */
#  undef PERLVAR
#  undef PERLVARA
#  undef PERLVARI
#  undef PERLVARIC

#endif /* MULTIPLICITY */

struct tempsym; /* defined in pp_pack.c */

#include "thread.h"
#include "pp.h"

#undef PERL_CKDEF
#undef PERL_PPDEF
#define PERL_CKDEF(s)	PERL_CALLCONV OP *s (pTHX_ OP *o);
#define PERL_PPDEF(s)	PERL_CALLCONV OP *s (pTHX);

#ifdef MYMALLOC
#  include "malloc_ctl.h"
#endif

/*
 * This provides a layer of functions and macros to ensure extensions will
 * get to use the same RTL functions as the core.
 */
#if defined(WIN32)
#  include "win32iop.h"
#endif


#include "proto.h"

/* this has structure inits, so it cannot be included before here */
#include "opcode.h"

/* The following must follow proto.h as #defines mess up syntax */

#if !defined(PERL_FOR_X2P)
#  include "embedvar.h"
#endif

/* Now include all the 'global' variables
 * If we don't have threads or multiple interpreters
 * these include variables that would have been their struct-s
 */

#define PERLVAR(prefix,var,type) EXT type PL_##var;
#define PERLVARA(prefix,var,n,type) EXT type PL_##var[n];
#define PERLVARI(prefix,var,type,init) EXT type  PL_##var INIT(init);
#define PERLVARIC(prefix,var,type,init) EXTCONST type PL_##var INIT(init);

#if !defined(MULTIPLICITY)
START_EXTERN_C
#  include "intrpvar.h"
END_EXTERN_C
#  define PL_sv_yes   (PL_sv_immortals[0])
#  define PL_sv_undef (PL_sv_immortals[1])
#  define PL_sv_no    (PL_sv_immortals[2])
#  define PL_sv_zero  (PL_sv_immortals[3])
#endif

#ifdef PERL_CORE
/* All core uses now exterminated. Ensure no zombies can return:  */
#  undef PL_na
#endif

/* Now all the config stuff is setup we can include embed.h
   In particular, need the relevant *ish file included already, as it may
   define HAVE_INTERP_INTERN  */
#include "embed.h"

START_EXTERN_C

#  include "perlvars.h"

END_EXTERN_C

#undef PERLVAR
#undef PERLVARA
#undef PERLVARI
#undef PERLVARIC

#if !defined(MULTIPLICITY)
/* Set up PERLVAR macros for populating structs */
#  define PERLVAR(prefix,var,type) type prefix##var;
/* 'var' is an array of length 'n' */
#  define PERLVARA(prefix,var,n,type) type prefix##var[n];
/* initialize 'var' to init' */
#  define PERLVARI(prefix,var,type,init) type prefix##var;
/* like PERLVARI, but make 'var' a const */
#  define PERLVARIC(prefix,var,type,init) type prefix##var;

/* this is never instantiated, is it just used for sizeof(struct PerlHandShakeInterpreter) */
struct PerlHandShakeInterpreter {
#  include "intrpvar.h"
};
#  undef PERLVAR
#  undef PERLVARA
#  undef PERLVARI
#  undef PERLVARIC
#endif

START_EXTERN_C

/* dummy variables that hold pointers to both runops functions, thus forcing
 * them *both* to get linked in (useful for Peek.xs, debugging etc) */

EXTCONST runops_proc_t PL_runops_std
  INIT(Perl_runops_standard);
EXTCONST runops_proc_t PL_runops_dbg
  INIT(Perl_runops_debug);

#define EXT_MGVTBL EXTCONST MGVTBL

#define PERL_MAGIC_READONLY_ACCEPTABLE 0x40
#define PERL_MAGIC_VALUE_MAGIC 0x80
#define PERL_MAGIC_VTABLE_MASK 0x3F

/* can this type of magic be attached to a readonly SV? */
#define PERL_MAGIC_TYPE_READONLY_ACCEPTABLE(t) \
    (PL_magic_data[(U8)(t)] & PERL_MAGIC_READONLY_ACCEPTABLE)

/* Is this type of magic container magic (%ENV, $1 etc),
 * or value magic (pos, taint etc)?
 */
#define PERL_MAGIC_TYPE_IS_VALUE_MAGIC(t) \
    (PL_magic_data[(U8)(t)] & PERL_MAGIC_VALUE_MAGIC)

#include "mg_vtable.h"

#ifdef DOINIT
EXTCONST U8 PL_magic_data[256] =
#  ifdef PERL_MICRO
#    include "umg_data.h"
#  else
#    include "mg_data.h"
#  endif
;
#else
EXTCONST U8 PL_magic_data[256];
#endif

#ifdef DOINIT
                        /* NL IV NV PV INV PI PN MG RX GV LV AV HV CV FM IO OBJ */
EXTCONST bool
PL_valid_types_IVX[]    = { 0, 1, 0, 0, 0, 1, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0 };
EXTCONST bool
PL_valid_types_NVX[]    = { 0, 0, 1, 0, 0, 0, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0 };
EXTCONST bool
PL_valid_types_PVX[]    = { 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 0 };
EXTCONST bool
PL_valid_types_RV[]     = { 0, 1, 0, 1, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 0 };
EXTCONST bool
PL_valid_types_IV_set[] = { 0, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 0 };
EXTCONST bool
PL_valid_types_NV_set[] = { 0, 0, 1, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0 };

EXTCONST U8
PL_deBruijn_bitpos_tab32[] = {
    /* https://graphics.stanford.edu/~seander/bithacks.html#IntegerLogDeBruijn */
    0,   1, 28,  2, 29, 14, 24,  3, 30, 22, 20, 15, 25, 17,  4,  8,
    31, 27, 13, 23, 21, 19, 16,  7, 26, 12, 18,  6, 11,  5, 10,  9
};

EXTCONST U8
PL_deBruijn_bitpos_tab64[] = {
    /* https://stackoverflow.com/questions/11376288/fast-computing-of-log2-for-64-bit-integers */
    63,  0, 58,  1, 59, 47, 53,  2, 60, 39, 48, 27, 54, 33, 42,  3,
    61, 51, 37, 40, 49, 18, 28, 20, 55, 30, 34, 11, 43, 14, 22,  4,
    62, 57, 46, 52, 38, 26, 32, 41, 50, 36, 17, 19, 29, 10, 13, 21,
    56, 45, 25, 31, 35, 16,  9, 12, 44, 24, 15,  8, 23,  7,  6,  5
};

#else

EXTCONST bool PL_valid_types_IVX[];
EXTCONST bool PL_valid_types_NVX[];
EXTCONST bool PL_valid_types_PVX[];
EXTCONST bool PL_valid_types_RV[];
EXTCONST bool PL_valid_types_IV_set[];
EXTCONST bool PL_valid_types_NV_set[];
EXTCONST U8   PL_deBruijn_bitpos_tab32[];
EXTCONST U8   PL_deBruijn_bitpos_tab64[];

#endif

/* The constants for using PL_deBruijn_bitpos_tab */
#define PERL_deBruijnMagic32_  0x077CB531
#define PERL_deBruijnShift32_  27
#define PERL_deBruijnMagic64_  0x07EDD5E59A4E28C2
#define PERL_deBruijnShift64_  58

/* In C99 we could use designated (named field) union initializers.
 * In C89 we need to initialize the member declared first.
 * In C++ we need extern C initializers.
 *
 * With the U8_NV version you will want to have inner braces,
 * while with the NV_U8 use just the NV. */

#define INFNAN_U8_NV_DECL EXTCONST union { U8 u8[NVSIZE]; NV nv; }
#define INFNAN_NV_U8_DECL EXTCONST union { NV nv; U8 u8[NVSIZE]; }

/* if these never got defined, they need defaults */
#ifndef PERL_SET_CONTEXT
#  define PERL_SET_CONTEXT(i)		PERL_SET_INTERP(i)
#endif

#ifdef USE_PERL_SWITCH_LOCALE_CONTEXT
#  define PERL_SET_LOCALE_CONTEXT(i)                                        \
      STMT_START {                                                          \
          if (UNLIKELY(PL_veto_switch_non_tTHX_context))                    \
                Perl_switch_locale_context();                               \
      } STMT_END
#else
#  define PERL_SET_LOCALE_CONTEXT(i)  NOOP
#endif

/* In some Configurations there may be per-thread information that is carried
 * in a library instead of perl's tTHX structure.  This macro is to be used to
 * handle those when tTHX is changed.  Only locale handling is currently known
 * to be affected. */
#define PERL_SET_NON_tTHX_CONTEXT(i)                                        \
                        STMT_START { PERL_SET_LOCALE_CONTEXT(i); } STMT_END


#ifndef PERL_GET_CONTEXT
#  define PERL_GET_CONTEXT		PERL_GET_INTERP
#endif

#ifndef PERL_GET_THX
#  define PERL_GET_THX			((void*)NULL)
#endif

#ifndef PERL_SET_THX
#  define PERL_SET_THX(t)		NOOP
#endif

#ifndef EBCDIC

/* The tables below are adapted from
 * https://bjoern.hoehrmann.de/utf-8/decoder/dfa/, which requires this copyright
 * notice:

Copyright (c) 2008-2009 Bjoern Hoehrmann <bjoern@hoehrmann.de>

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/

#  ifdef DOINIT
#    if 0       /* This is the original table given in
                   https://bjoern.hoehrmann.de/utf-8/decoder/dfa/ */
static U8 utf8d_C9[] = {
  /* The first part of the table maps bytes to character classes that
   * to reduce the size of the transition table and create bitmasks. */
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /*-1F*/
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /*-3F*/
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /*-5F*/
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /*-7F*/
   1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,  9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9, /*-9F*/
   7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,  7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7, /*-BF*/
   8,8,2,2,2,2,2,2,2,2,2,2,2,2,2,2,  2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, /*-DF*/
  10,3,3,3,3,3,3,3,3,3,3,3,3,4,3,3, 11,6,6,6,5,8,8,8,8,8,8,8,8,8,8,8, /*-FF*/

  /* The second part is a transition table that maps a combination
   * of a state of the automaton and a character class to a state. */
   0,12,24,36,60,96,84,12,12,12,48,72, 12,12,12,12,12,12,12,12,12,12,12,12,
  12, 0,12,12,12,12,12, 0,12, 0,12,12, 12,24,12,12,12,12,12,24,12,24,12,12,
  12,12,12,12,12,12,12,24,12,12,12,12, 12,24,12,12,12,12,12,12,12,24,12,12,
  12,12,12,12,12,12,12,36,12,36,12,12, 12,36,12,12,12,12,12,36,12,36,12,12,
  12,36,12,12,12,12,12,12,12,12,12,12
};

#    endif

/* This is a version of the above table customized for Perl that doesn't
 * exclude surrogates and accepts start bytes up through FD (FE on 64-bit
 * machines).  The classes have been renumbered so that the patterns are more
 * evident in the table.  The class numbers are structured so the values are:
 *
 *  a) UTF-8 invariant code points
 *          0
 *  b) Start bytes that always lead to either overlongs or some class of code
 *     point that needs outside intervention for handling (such as to raise a
 *     warning)
 *          1
 *  c) Start bytes that never lead to one of the above
 *      number of bytes in complete sequence
 *  d) Rest of start bytes (they can be resolved through this algorithm) and
 *     continuation bytes
 *          arbitrary class number chosen to not conflict with the above
 *          classes, and to index into the remaining table
 *
 * It would make the code simpler if start byte FF could also be handled, but
 * doing so would mean adding two more classes (one from splitting 80 from 81,
 * and one for FF), and nodes for each of 6 new continuation bytes.  The
 * current table has 436 entries; the new one would require 140 more = 576 (2
 * additional classes for each of the 10 existing nodes, and 20 for each of 6
 * new nodes.  The array would have to be made U16 instead of U8, not worth it
 * for this rarely encountered case
 *
 * The classes are
 *      00-7F           0   Always legal, single byte sequence
 *      80-81           7   Not legal immediately after start bytes E0 F0 F8 FC
 *                          FE
 *      82-83           8   Not legal immediately after start bytes E0 F0 F8 FC
 *      84-87           9   Not legal immediately after start bytes E0 F0 F8
 *      88-8F          10   Not legal immediately after start bytes E0 F0
 *      90-9F          11   Not legal immediately after start byte E0
 *      A0-BF          12   Always legal continuation byte
 *      C0,C1           1   Not legal: overlong
 *      C2-DF           2   Legal start byte for two byte sequences
 *      E0             13   Some sequences are overlong; others legal
 *      E1-EF           3   Legal start byte for three byte sequences
 *      F0             14   Some sequences are overlong; others legal
 *      F1-F7           4   Legal start byte for four byte sequences
 *      F8             15   Some sequences are overlong; others legal
 *      F9-FB           5   Legal start byte for five byte sequences
 *      FC             16   Some sequences are overlong; others legal
 *      FD              6   Legal start byte for six byte sequences
 *      FE             17   Some sequences are overlong; others legal
 *                          (is 1 on 32-bit machines, since it overflows)
 *      FF              1   Need to handle specially
 */

EXTCONST U8 PL_extended_utf8_dfa_tab[] = {
    /* The first part of the table maps bytes to character classes to reduce
     * the size of the transition table and create bitmasks. */
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*00-0F*/
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*10-1F*/
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*20-2F*/
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*30-3F*/
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*40-4F*/
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*50-5F*/
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*60-6F*/
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*70-7F*/
   7, 7, 8, 8, 9, 9, 9, 9,10,10,10,10,10,10,10,10, /*80-8F*/
  11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11, /*90-9F*/
  12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12, /*A0-AF*/
  12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12, /*B0-BF*/
   1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, /*C0-CF*/
   2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, /*D0-DF*/
  13, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, /*E0-EF*/
  14, 4, 4, 4, 4, 4, 4, 4,15, 5, 5, 5,16, 6,       /*F0-FD*/
#    ifdef UV_IS_QUAD
                                            17,    /*FE*/
#    else
                                             1,    /*FE*/
#    endif
                                                1, /*FF*/

/* The second part is a transition table that maps a combination
 * of a state of the automaton and a character class to a new state, called a
 * node.  The nodes are:
 * N0     The initial state, and final accepting one.
 * N1     Any one continuation byte (80-BF) left.  This is transitioned to
 *        immediately when the start byte indicates a two-byte sequence
 * N2     Any two continuation bytes left.
 * N3     Any three continuation bytes left.
 * N4     Any four continuation bytes left.
 * N5     Any five continuation bytes left.
 * N6     Start byte is E0.  Continuation bytes 80-9F are illegal (overlong);
 *        the other continuations transition to N1
 * N7     Start byte is F0.  Continuation bytes 80-8F are illegal (overlong);
 *        the other continuations transition to N2
 * N8     Start byte is F8.  Continuation bytes 80-87 are illegal (overlong);
 *        the other continuations transition to N3
 * N9     Start byte is FC.  Continuation bytes 80-83 are illegal (overlong);
 *        the other continuations transition to N4
 * N10    Start byte is FE.  Continuation bytes 80-81 are illegal (overlong);
 *        the other continuations transition to N5
 * 1      Reject.  All transitions not mentioned above (except the single
 *        byte ones (as they are always legal)) are to this state.
 */

#    if defined(PERL_CORE)
#      define NUM_CLASSES 18
#      define N0 0
#      define N1 ((N0)   + NUM_CLASSES)
#      define N2 ((N1)   + NUM_CLASSES)
#      define N3 ((N2)   + NUM_CLASSES)
#      define N4 ((N3)   + NUM_CLASSES)
#      define N5 ((N4)   + NUM_CLASSES)
#      define N6 ((N5)   + NUM_CLASSES)
#      define N7 ((N6)   + NUM_CLASSES)
#      define N8 ((N7)   + NUM_CLASSES)
#      define N9 ((N8)   + NUM_CLASSES)
#      define N10 ((N9)  + NUM_CLASSES)

/*Class: 0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17  */
/*N0*/   0, 1,N1,N2,N3,N4,N5, 1, 1, 1, 1, 1, 1,N6,N7,N8,N9,N10,
/*N1*/   1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1,
/*N2*/   1, 1, 1, 1, 1, 1, 1,N1,N1,N1,N1,N1,N1, 1, 1, 1, 1, 1,
/*N3*/   1, 1, 1, 1, 1, 1, 1,N2,N2,N2,N2,N2,N2, 1, 1, 1, 1, 1,
/*N4*/   1, 1, 1, 1, 1, 1, 1,N3,N3,N3,N3,N3,N3, 1, 1, 1, 1, 1,
/*N5*/   1, 1, 1, 1, 1, 1, 1,N4,N4,N4,N4,N4,N4, 1, 1, 1, 1, 1,

/*N6*/   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,N1, 1, 1, 1, 1, 1,
/*N7*/   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,N2,N2, 1, 1, 1, 1, 1,
/*N8*/   1, 1, 1, 1, 1, 1, 1, 1, 1, 1,N3,N3,N3, 1, 1, 1, 1, 1,
/*N9*/   1, 1, 1, 1, 1, 1, 1, 1, 1,N4,N4,N4,N4, 1, 1, 1, 1, 1,
/*N10*/  1, 1, 1, 1, 1, 1, 1, 1,N5,N5,N5,N5,N5, 1, 1, 1, 1, 1,
};

/* And below is a version of the above table that accepts only strict UTF-8.
 * Hence no surrogates nor non-characters, nor non-Unicode.  Thus, if the input
 * passes this dfa, it will be for a well-formed, non-problematic code point
 * that can be returned immediately.
 *
 * The "Implementation details" portion of
 * https://bjoern.hoehrmann.de/utf-8/decoder/dfa/ shows how
 * the first portion of the table maps each possible byte into a character
 * class.  And that the classes for those bytes which are start bytes have been
 * carefully chosen so they serve as well to be used as a shift value to mask
 * off the leading 1 bits of the start byte.  Unfortunately the addition of
 * being able to distinguish non-characters makes this not fully work.  This is
 * because, now, the start bytes E1-EF have to be broken into 3 classes instead
 * of 2:
 *  1) ED because it could be a surrogate
 *  2) EF because it could be a non-character
 *  3) the rest, which can never evaluate to a problematic code point.
 *
 * Each of E1-EF has three leading 1 bits, then a 0.  That means we could use a
 * shift (and hence class number) of either 3 or 4 to get a mask that works.
 * But that only allows two categories, and we need three.  khw made the
 * decision to therefore treat the ED start byte as an error, so that the dfa
 * drops out immediately for that.  In the dfa, classes 3 and 4 are used to
 * distinguish EF vs the rest.  Then special code is used to deal with ED,
 * that's executed only when the dfa drops out.  The code points started by ED
 * are half surrogates, and half hangul syllables.  This means that 2048 of
 * the hangul syllables (about 18%) take longer than all other non-problematic
 * code points to handle.
 *
 * The changes to handle non-characters requires the addition of states and
 * classes to the dfa.  (See the section on "Mapping bytes to character
 * classes" in the linked-to document for further explanation of the original
 * dfa.)
 *
 * The classes are
 *      00-7F           0
 *      80-8E           9
 *      8F             10
 *      90-9E          11
 *      9F             12
 *      A0-AE          13
 *      AF             14
 *      B0-B6          15
 *      B7             16
 *      B8-BD          15
 *      BE             17
 *      BF             18
 *      C0,C1           1
 *      C2-DF           2
 *      E0              7
 *      E1-EC           3
 *      ED              1
 *      EE              3
 *      EF              4
 *      F0              8
 *      F1-F3           6  (6 bits can be stripped)
 *      F4              5  (only 5 can be stripped)
 *      F5-FF           1
 */

EXTCONST U8 PL_strict_utf8_dfa_tab[] = {
    /* The first part of the table maps bytes to character classes to reduce
     * the size of the transition table and create bitmasks. */
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*00-0F*/
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*10-1F*/
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*20-2F*/
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*30-3F*/
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*40-4F*/
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*50-5F*/
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*60-6F*/
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*70-7F*/
   9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,10, /*80-8F*/
  11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,12, /*90-9F*/
  13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,14, /*A0-AF*/
  15,15,15,15,15,15,15,16,15,15,15,15,15,15,17,18, /*B0-BF*/
   1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, /*C0-CF*/
   2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, /*D0-DF*/
   7, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 1, 3, 4, /*E0-EF*/
   8, 6, 6, 6, 5, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, /*F0-FF*/

/* The second part is a transition table that maps a combination
 * of a state of the automaton and a character class to a new state, called a
 * node.  The nodes are:
 * N0     The initial state, and final accepting one.
 * N1     Any one continuation byte (80-BF) left.  This is transitioned to
 *        immediately when the start byte indicates a two-byte sequence
 * N2     Any two continuation bytes left.
 * N3     Start byte is E0.  Continuation bytes 80-9F are illegal (overlong);
 *        the other continuations transition to state N1
 * N4     Start byte is EF.  Continuation byte B7 transitions to N8; BF to N9;
 *        the other continuations transitions to N1
 * N5     Start byte is F0.  Continuation bytes 80-8F are illegal (overlong);
 *        [9AB]F transition to N10; the other continuations to N2.
 * N6     Start byte is F[123].  Continuation bytes [89AB]F transition
 *        to N10; the other continuations to N2.
 * N7     Start byte is F4.  Continuation bytes 90-BF are illegal
 *        (non-unicode); 8F transitions to N10; the other continuations to N2
 * N8     Initial sequence is EF B7.  Continuation bytes 90-AF are illegal
 *        (non-characters); the other continuations transition to N0.
 * N9     Initial sequence is EF BF.  Continuation bytes BE and BF are illegal
 *        (non-characters); the other continuations transition to N0.
 * N10    Initial sequence is one of: F0 [9-B]F; F[123] [8-B]F; or F4 8F.
 *        Continuation byte BF transitions to N11; the other continuations to
 *        N1
 * N11    Initial sequence is the two bytes given in N10 followed by BF.
 *        Continuation bytes BE and BF are illegal (non-characters); the other
 *        continuations transition to N0.
 * 1      Reject.  All transitions not mentioned above (except the single
 *        byte ones (as they are always legal) are to this state.
 */

#      undef N0
#      undef N1
#      undef N2
#      undef N3
#      undef N4
#      undef N5
#      undef N6
#      undef N7
#      undef N8
#      undef N9
#      undef NUM_CLASSES
#      define NUM_CLASSES 19
#      define N0 0
#      define N1  ((N0)  + NUM_CLASSES)
#      define N2  ((N1)  + NUM_CLASSES)
#      define N3  ((N2)  + NUM_CLASSES)
#      define N4  ((N3)  + NUM_CLASSES)
#      define N5  ((N4)  + NUM_CLASSES)
#      define N6  ((N5)  + NUM_CLASSES)
#      define N7  ((N6)  + NUM_CLASSES)
#      define N8  ((N7)  + NUM_CLASSES)
#      define N9  ((N8)  + NUM_CLASSES)
#      define N10 ((N9)  + NUM_CLASSES)
#      define N11 ((N10) + NUM_CLASSES)

/*Class: 0   1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16  17  18 */
/*N0*/   0,  1, N1, N2, N4, N7, N6, N3, N5,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
/*N1*/   1,  1,  1,  1,  1,  1,  1,  1,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
/*N2*/   1,  1,  1,  1,  1,  1,  1,  1,  1, N1, N1, N1, N1, N1, N1, N1, N1, N1, N1,

/*N3*/   1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1, N1, N1, N1, N1, N1, N1,
/*N4*/   1,  1,  1,  1,  1,  1,  1,  1,  1, N1, N1, N1, N1, N1, N1, N1, N8, N1, N9,
/*N5*/   1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1, N2,N10, N2,N10, N2, N2, N2,N10,
/*N6*/   1,  1,  1,  1,  1,  1,  1,  1,  1, N2,N10, N2,N10, N2,N10, N2, N2, N2,N10,
/*N7*/   1,  1,  1,  1,  1,  1,  1,  1,  1, N2,N10,  1,  1,  1,  1,  1,  1,  1,  1,
/*N8*/   1,  1,  1,  1,  1,  1,  1,  1,  1,  0,  0,  1,  1,  1,  1,  0,  0,  0,  0,
/*N9*/   1,  1,  1,  1,  1,  1,  1,  1,  1,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,
/*N10*/  1,  1,  1,  1,  1,  1,  1,  1,  1, N1, N1, N1, N1, N1, N1, N1, N1, N1,N11,
/*N11*/  1,  1,  1,  1,  1,  1,  1,  1,  1,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,
};

/* And below is yet another version of the above tables that accepts only UTF-8
 * as defined by Corregidum #9.  Hence no surrogates nor non-Unicode, but
 * it allows non-characters.  This is isomorphic to the original table
 * in https://bjoern.hoehrmann.de/utf-8/decoder/dfa/
 *
 * The classes are
 *      00-7F           0
 *      80-8F           9
 *      90-9F          10
 *      A0-BF          11
 *      C0,C1           1
 *      C2-DF           2
 *      E0              7
 *      E1-EC           3
 *      ED              4
 *      EE-EF           3
 *      F0              8
 *      F1-F3           6  (6 bits can be stripped)
 *      F4              5  (only 5 can be stripped)
 *      F5-FF           1
 */

EXTCONST U8 PL_c9_utf8_dfa_tab[] = {
    /* The first part of the table maps bytes to character classes to reduce
     * the size of the transition table and create bitmasks. */
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*00-0F*/
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*10-1F*/
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*20-2F*/
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*30-3F*/
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*40-4F*/
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*50-5F*/
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*60-6F*/
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*70-7F*/
   9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, /*80-8F*/
  10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10, /*90-9F*/
  11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11, /*A0-AF*/
  11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11, /*B0-BF*/
   1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, /*C0-CF*/
   2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, /*D0-DF*/
   7, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 3, 3, /*E0-EF*/
   8, 6, 6, 6, 5, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, /*F0-FF*/

/* The second part is a transition table that maps a combination
 * of a state of the automaton and a character class to a new state, called a
 * node.  The nodes are:
 * N0     The initial state, and final accepting one.
 * N1     Any one continuation byte (80-BF) left.  This is transitioned to
 *        immediately when the start byte indicates a two-byte sequence
 * N2     Any two continuation bytes left.
 * N3     Any three continuation bytes left.
 * N4     Start byte is E0.  Continuation bytes 80-9F are illegal (overlong);
 *        the other continuations transition to state N1
 * N5     Start byte is ED.  Continuation bytes A0-BF all lead to surrogates,
 *        so are illegal.  The other continuations transition to state N1.
 * N6     Start byte is F0.  Continuation bytes 80-8F are illegal (overlong);
 *        the other continuations transition to N2
 * N7     Start byte is F4.  Continuation bytes 90-BF are illegal
 *        (non-unicode); the other continuations transition to N2
 * 1      Reject.  All transitions not mentioned above (except the single
 *        byte ones (as they are always legal) are to this state.
 */

#      undef N0
#      undef N1
#      undef N2
#      undef N3
#      undef N4
#      undef N5
#      undef N6
#      undef N7
#      undef NUM_CLASSES
#      define NUM_CLASSES 12
#      define N0 0
#      define N1  ((N0)  + NUM_CLASSES)
#      define N2  ((N1)  + NUM_CLASSES)
#      define N3  ((N2)  + NUM_CLASSES)
#      define N4  ((N3)  + NUM_CLASSES)
#      define N5  ((N4)  + NUM_CLASSES)
#      define N6  ((N5)  + NUM_CLASSES)
#      define N7  ((N6)  + NUM_CLASSES)

/*Class: 0   1   2   3   4   5   6   7   8   9  10  11 */
/*N0*/   0,  1, N1, N2, N5, N7, N3, N4, N6,  1,  1,  1,
/*N1*/   1,  1,  1,  1,  1,  1,  1,  1,  1,  0,  0,  0,
/*N2*/   1,  1,  1,  1,  1,  1,  1,  1,  1, N1, N1, N1,
/*N3*/   1,  1,  1,  1,  1,  1,  1,  1,  1, N2, N2, N2,

/*N4*/   1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1, N1,
/*N5*/   1,  1,  1,  1,  1,  1,  1,  1,  1, N1, N1,  1,
/*N6*/   1,  1,  1,  1,  1,  1,  1,  1,  1,  1, N2, N2,
/*N7*/   1,  1,  1,  1,  1,  1,  1,  1,  1, N2,  1,  1,
};

#    endif /* defined(PERL_CORE) */
#  else     /* End of is DOINIT */

EXTCONST U8 PL_extended_utf8_dfa_tab[];
EXTCONST U8 PL_strict_utf8_dfa_tab[];
EXTCONST U8 PL_c9_utf8_dfa_tab[];

#  endif
#endif    /* end of isn't EBCDIC */

#include "overload.h"

END_EXTERN_C

struct am_table {
  U8 flags;
  U8 fallback;
  U16 spare;
  U32 was_ok_sub;
  CV* table[NofAMmeth];
};
struct am_table_short {
  U8 flags;
  U8 fallback;
  U16 spare;
  U32 was_ok_sub;
};
typedef struct am_table AMT;
typedef struct am_table_short AMTS;

#define AMGfallNEVER	1
#define AMGfallNO	2
#define AMGfallYES	3

#define AMTf_AMAGIC		1
#define AMT_AMAGIC(amt)		((amt)->flags & AMTf_AMAGIC)
#define AMT_AMAGIC_on(amt)	((amt)->flags |= AMTf_AMAGIC)
#define AMT_AMAGIC_off(amt)	((amt)->flags &= ~AMTf_AMAGIC)

#define StashHANDLER(stash,meth)	gv_handler((stash),CAT2(meth,_amg))

/*
 * some compilers like to redefine cos et alia as faster
 * (and less accurate?) versions called F_cos et cetera (Quidquid
 * latine dictum sit, altum viditur.)  This trick collides with
 * the Perl overloading (amg).  The following #defines fool both.
 */

#ifdef _FASTMATH
#   ifdef atan2
#       define F_atan2_amg  atan2_amg
#   endif
#   ifdef cos
#       define F_cos_amg    cos_amg
#   endif
#   ifdef exp
#       define F_exp_amg    exp_amg
#   endif
#   ifdef log
#       define F_log_amg    log_amg
#   endif
#   ifdef pow
#       define F_pow_amg    pow_amg
#   endif
#   ifdef sin
#       define F_sin_amg    sin_amg
#   endif
#   ifdef sqrt
#       define F_sqrt_amg   sqrt_amg
#   endif
#endif /* _FASTMATH */

#define PERLDB_ALL		(PERLDBf_SUB	| PERLDBf_LINE	|	\
                                 PERLDBf_NOOPT	| PERLDBf_INTER	|	\
                                 PERLDBf_SUBLINE| PERLDBf_SINGLE|	\
                                 PERLDBf_NAMEEVAL| PERLDBf_NAMEANON |   \
                                 PERLDBf_SAVESRC)
                                        /* No _NONAME, _GOTO */
#define PERLDBf_SUB		0x01	/* Debug sub enter/exit */
#define PERLDBf_LINE		0x02	/* Keep line # */
#define PERLDBf_NOOPT		0x04	/* Switch off optimizations */
#define PERLDBf_INTER		0x08	/* Preserve more data for
                                           later inspections  */
#define PERLDBf_SUBLINE		0x10	/* Keep subr source lines */
#define PERLDBf_SINGLE		0x20	/* Start with single-step on */
#define PERLDBf_NONAME		0x40	/* For _SUB: no name of the subr */
#define PERLDBf_GOTO		0x80	/* Report goto: call DB::goto */
#define PERLDBf_NAMEEVAL	0x100	/* Informative names for evals */
#define PERLDBf_NAMEANON	0x200	/* Informative names for anon subs */
#define PERLDBf_SAVESRC  	0x400	/* Save source lines into @{"_<$filename"} */
#define PERLDBf_SAVESRC_NOSUBS	0x800	/* Including evals that generate no subroutines */
#define PERLDBf_SAVESRC_INVALID	0x1000	/* Save source that did not compile */

#define PERLDB_SUB		(PL_perldb & PERLDBf_SUB)
#define PERLDB_LINE		(PL_perldb & PERLDBf_LINE)
#define PERLDB_NOOPT		(PL_perldb & PERLDBf_NOOPT)
#define PERLDB_INTER		(PL_perldb & PERLDBf_INTER)
#define PERLDB_SUBLINE		(PL_perldb & PERLDBf_SUBLINE)
#define PERLDB_SINGLE		(PL_perldb & PERLDBf_SINGLE)
#define PERLDB_SUB_NN		(PL_perldb & PERLDBf_NONAME)
#define PERLDB_GOTO		(PL_perldb & PERLDBf_GOTO)
#define PERLDB_NAMEEVAL 	(PL_perldb & PERLDBf_NAMEEVAL)
#define PERLDB_NAMEANON 	(PL_perldb & PERLDBf_NAMEANON)
#define PERLDB_SAVESRC  	(PL_perldb & PERLDBf_SAVESRC)
#define PERLDB_SAVESRC_NOSUBS	(PL_perldb & PERLDBf_SAVESRC_NOSUBS)
#define PERLDB_SAVESRC_INVALID	(PL_perldb & PERLDBf_SAVESRC_INVALID)

#define PERLDB_LINE_OR_SAVESRC (PL_perldb & (PERLDBf_LINE | PERLDBf_SAVESRC))

#ifdef USE_ITHREADS
#  define KEYWORD_PLUGIN_MUTEX_INIT    MUTEX_INIT(&PL_keyword_plugin_mutex)
#  define KEYWORD_PLUGIN_MUTEX_LOCK    MUTEX_LOCK(&PL_keyword_plugin_mutex)
#  define KEYWORD_PLUGIN_MUTEX_UNLOCK  MUTEX_UNLOCK(&PL_keyword_plugin_mutex)
#  define KEYWORD_PLUGIN_MUTEX_TERM    MUTEX_DESTROY(&PL_keyword_plugin_mutex)
#  define USER_PROP_MUTEX_INIT    MUTEX_INIT(&PL_user_prop_mutex)
#  define USER_PROP_MUTEX_LOCK    MUTEX_LOCK(&PL_user_prop_mutex)
#  define USER_PROP_MUTEX_UNLOCK  MUTEX_UNLOCK(&PL_user_prop_mutex)
#  define USER_PROP_MUTEX_TERM    MUTEX_DESTROY(&PL_user_prop_mutex)
#else
#  define KEYWORD_PLUGIN_MUTEX_INIT    NOOP
#  define KEYWORD_PLUGIN_MUTEX_LOCK    NOOP
#  define KEYWORD_PLUGIN_MUTEX_UNLOCK  NOOP
#  define KEYWORD_PLUGIN_MUTEX_TERM    NOOP
#  define USER_PROP_MUTEX_INIT    NOOP
#  define USER_PROP_MUTEX_LOCK    NOOP
#  define USER_PROP_MUTEX_UNLOCK  NOOP
#  define USER_PROP_MUTEX_TERM    NOOP
#endif

#ifdef USE_LOCALE /* These locale things are all subject to change */

   /* Returns TRUE if the plain locale pragma without a parameter is in effect.
    * */
#  define IN_LOCALE_RUNTIME	(PL_curcop                                  \
                              && CopHINTS_get(PL_curcop) & HINT_LOCALE)

   /* Returns TRUE if either form of the locale pragma is in effect */
#  define IN_SOME_LOCALE_FORM_RUNTIME                                       \
        cBOOL(CopHINTS_get(PL_curcop) & (HINT_LOCALE|HINT_LOCALE_PARTIAL))

#  define IN_LOCALE_COMPILETIME	cBOOL(PL_hints & HINT_LOCALE)
#  define IN_SOME_LOCALE_FORM_COMPILETIME                                   \
                        cBOOL(PL_hints & (HINT_LOCALE|HINT_LOCALE_PARTIAL))

/*
=for apidoc_section $locale

=for apidoc Amn|bool|IN_LOCALE

Evaluates to TRUE if the plain locale pragma without a parameter (S<C<use
locale>>) is in effect.

=for apidoc Amn|bool|IN_LOCALE_COMPILETIME

Evaluates to TRUE if, when compiling a perl program (including an C<eval>) if
the plain locale pragma without a parameter (S<C<use locale>>) is in effect.

=for apidoc Amn|bool|IN_LOCALE_RUNTIME

Evaluates to TRUE if, when executing a perl program (including an C<eval>) if
the plain locale pragma without a parameter (S<C<use locale>>) is in effect.

=cut
*/

#  define IN_LOCALE                                                         \
        (IN_PERL_COMPILETIME ? IN_LOCALE_COMPILETIME : IN_LOCALE_RUNTIME)
#  define IN_SOME_LOCALE_FORM                                               \
                    (IN_PERL_COMPILETIME ? IN_SOME_LOCALE_FORM_COMPILETIME  \
                                         : IN_SOME_LOCALE_FORM_RUNTIME)

#  define IN_LC_ALL_COMPILETIME   IN_LOCALE_COMPILETIME
#  define IN_LC_ALL_RUNTIME       IN_LOCALE_RUNTIME

#  define IN_LC_PARTIAL_COMPILETIME   cBOOL(PL_hints & HINT_LOCALE_PARTIAL)
#  define IN_LC_PARTIAL_RUNTIME                                             \
              (PL_curcop && CopHINTS_get(PL_curcop) & HINT_LOCALE_PARTIAL)

#  define IN_LC_COMPILETIME(category)                                       \
       (       IN_LC_ALL_COMPILETIME                                        \
        || (   IN_LC_PARTIAL_COMPILETIME                                    \
            && Perl__is_in_locale_category(aTHX_ TRUE, (category))))
#  define IN_LC_RUNTIME(category)                                           \
      (IN_LC_ALL_RUNTIME || (IN_LC_PARTIAL_RUNTIME                          \
                 && Perl__is_in_locale_category(aTHX_ FALSE, (category))))
#  define IN_LC(category)  \
                    (IN_LC_COMPILETIME(category) || IN_LC_RUNTIME(category))

#  if defined (PERL_CORE) || defined (PERL_IN_XSUB_RE)

     /* This internal macro should be called from places that operate under
      * locale rules.  If there is a problem with the current locale that
      * hasn't been raised yet, it will output a warning this time.  Because
      * this will so rarely  be true, there is no point to optimize for time;
      * instead it makes sense to minimize space used and do all the work in
      * the rarely called function */
#    ifdef USE_LOCALE_CTYPE
#      define CHECK_AND_WARN_PROBLEMATIC_LOCALE_                              \
                STMT_START {                                                  \
                    if (UNLIKELY(PL_warn_locale)) {                           \
                        Perl__warn_problematic_locale();                      \
                    }                                                         \
                }  STMT_END
#    else
#      define CHECK_AND_WARN_PROBLEMATIC_LOCALE_
#    endif


     /* These two internal macros are called when a warning should be raised,
      * and will do so if enabled.  The first takes a single code point
      * argument; the 2nd, is a pointer to the first byte of the UTF-8 encoded
      * string, and an end position which it won't try to read past */
#    define _CHECK_AND_OUTPUT_WIDE_LOCALE_CP_MSG(cp)                        \
        STMT_START {                                                        \
            if (! IN_UTF8_CTYPE_LOCALE && ckWARN(WARN_LOCALE)) {            \
                Perl_warner(aTHX_ packWARN(WARN_LOCALE),                    \
                                       "Wide character (U+%" UVXf ") in %s",\
                                       (UV) cp, OP_DESC(PL_op));            \
            }                                                               \
        }  STMT_END

#    define _CHECK_AND_OUTPUT_WIDE_LOCALE_UTF8_MSG(s, send)                 \
        STMT_START { /* Check if to warn before doing the conversion work */\
            if (! IN_UTF8_CTYPE_LOCALE && ckWARN(WARN_LOCALE)) {            \
                UV cp = utf8_to_uvchr_buf((U8 *) (s), (U8 *) (send), NULL); \
                Perl_warner(aTHX_ packWARN(WARN_LOCALE),                    \
                    "Wide character (U+%" UVXf ") in %s",                   \
                    (cp == 0)                                               \
                     ? UNICODE_REPLACEMENT                                  \
                     : (UV) cp,                                             \
                    OP_DESC(PL_op));                                        \
            }                                                               \
        }  STMT_END

#  endif   /* PERL_CORE or PERL_IN_XSUB_RE */
#else   /* No locale usage */
#  define IN_LOCALE_RUNTIME                0
#  define IN_SOME_LOCALE_FORM_RUNTIME      0
#  define IN_LOCALE_COMPILETIME            0
#  define IN_SOME_LOCALE_FORM_COMPILETIME  0
#  define IN_LOCALE                        0
#  define IN_SOME_LOCALE_FORM              0
#  define IN_LC_ALL_COMPILETIME            0
#  define IN_LC_ALL_RUNTIME                0
#  define IN_LC_PARTIAL_COMPILETIME        0
#  define IN_LC_PARTIAL_RUNTIME            0
#  define IN_LC_COMPILETIME(category)      0
#  define IN_LC_RUNTIME(category)          0
#  define IN_LC(category)                  0
#  define CHECK_AND_WARN_PROBLEMATIC_LOCALE_
#  define _CHECK_AND_OUTPUT_WIDE_LOCALE_UTF8_MSG(s, send)
#  define _CHECK_AND_OUTPUT_WIDE_LOCALE_CP_MSG(c)
#endif

#define locale_panic_(m)  Perl_locale_panic((m), __FILE__, __LINE__, errno)

/* Locale/thread synchronization macros. */
#if ! defined(USE_LOCALE) || ! defined(USE_LOCALE_THREADS)
#  define LOCALE_LOCK_(cond)  NOOP
#  define LOCALE_UNLOCK_      NOOP
#  define LOCALE_INIT
#  define LOCALE_TERM

#else   /* Below: Threaded, and locales are supported */

    /* A locale mutex is required on all such threaded builds.
     *
     * This mutex simulates a general (or recursive) semaphore.  The current
     * thread will lock the mutex if the per-thread variable is zero, and then
     * increments that variable.  Each corresponding UNLOCK decrements the
     * variable until it is 0, at which point it actually unlocks the mutex.
     * Since the variable is per-thread, initialized to 0, there is no race
     * with other threads.
     *
     * The single argument is a condition to test for, and if true, to panic.
     * Call it with the constant 0 to suppress the check.
     *
     * Clang improperly gives warnings for this, if not silenced:
     * https://clang.llvm.org/docs/ThreadSafetyAnalysis.html#conditional-locks
     */
#  define LOCALE_LOCK_(cond_to_panic_if_already_locked)                     \
        STMT_START {                                                        \
            CLANG_DIAG_IGNORE(-Wthread-safety)	     	                    \
            if (LIKELY(PL_locale_mutex_depth <= 0)) {                       \
                DEBUG_Lv(PerlIO_printf(Perl_debug_log,                      \
                         "%s: %d: locking locale; depth=1\n",               \
                         __FILE__, __LINE__));                              \
                MUTEX_LOCK(&PL_locale_mutex);                               \
                PL_locale_mutex_depth = 1;                                  \
            }                                                               \
            else {                                                          \
                PL_locale_mutex_depth++;                                    \
                DEBUG_Lv(PerlIO_printf(Perl_debug_log,                      \
                        "%s: %d: avoided locking locale; new depth=%d\n",   \
                        __FILE__, __LINE__, PL_locale_mutex_depth));        \
                if (cond_to_panic_if_already_locked) {                      \
                    locale_panic_("Trying to lock locale incompatibly: "    \
                         STRINGIFY(cond_to_panic_if_already_locked));       \
                }                                                           \
            }                                                               \
            CLANG_DIAG_RESTORE                                              \
        } STMT_END

#  define LOCALE_UNLOCK_                                                    \
        STMT_START {                                                        \
            if (LIKELY(PL_locale_mutex_depth == 1)) {                       \
                DEBUG_Lv(PerlIO_printf(Perl_debug_log,                      \
                         "%s: %d: unlocking locale; new depth=0\n",         \
                         __FILE__, __LINE__));                              \
                PL_locale_mutex_depth = 0;                                  \
                MUTEX_UNLOCK(&PL_locale_mutex);                             \
            }                                                               \
            else if (PL_locale_mutex_depth <= 0) {                          \
                DEBUG_L(PerlIO_printf(Perl_debug_log,                       \
                        "%s: %d: ignored attempt to unlock already"         \
                        " unlocked locale; depth unchanged at %d\n",        \
                       __FILE__, __LINE__, PL_locale_mutex_depth));         \
            }                                                               \
            else {                                                          \
                PL_locale_mutex_depth--;                                    \
                DEBUG_Lv(PerlIO_printf(Perl_debug_log,                      \
                        "%s: %d: avoided unlocking locale; new depth=%d\n", \
                        __FILE__, __LINE__, PL_locale_mutex_depth));        \
            }                                                               \
        } STMT_END

#  if defined(USE_THREADS) && ! defined(USE_THREAD_SAFE_LOCALE)

     /* By definition, a thread-unsafe locale means we need a critical
      * section. */
#    define SETLOCALE_LOCK   LOCALE_LOCK_(0)
#    define SETLOCALE_UNLOCK LOCALE_UNLOCK_
#    ifdef USE_LOCALE_NUMERIC
#      define LC_NUMERIC_LOCK(cond_to_panic_if_already_locked)              \
                 LOCALE_LOCK_(cond_to_panic_if_already_locked)
#      define LC_NUMERIC_UNLOCK  LOCALE_UNLOCK_
#    endif
#  endif

#  ifndef USE_POSIX_2008_LOCALE
#    define LOCALE_TERM_POSIX_2008_  NOOP
#  else
     /* We have a locale object holding the 'C' locale for Posix 2008 */
#    define LOCALE_TERM_POSIX_2008_                                         \
                    STMT_START {                                            \
                        if (PL_C_locale_obj) {                              \
                            /* Make sure we aren't using the locale         \
                             * space we are about to free */                \
                            uselocale(LC_GLOBAL_LOCALE);                    \
                            freelocale(PL_C_locale_obj);                    \
                            PL_C_locale_obj = (locale_t) NULL;              \
                        }                                                   \
                    } STMT_END
#  endif

#  define LOCALE_INIT           MUTEX_INIT(&PL_locale_mutex)
#  define LOCALE_TERM           STMT_START {                                \
                                    LOCALE_TERM_POSIX_2008_;                \
                                    MUTEX_DESTROY(&PL_locale_mutex);        \
                                } STMT_END
#endif

/* There are some locale-related functions which may need locking only because
 * they share some common memory across threads, and hence there is the
 * potential for a race in accessing that space.  Most are because their return
 * points to a global static buffer, but some just use some common space
 * internally.  All functions accessing a given space need to have a critical
 * section to prevent any other thread from accessing it at the same time.
 * Ideally, there would be a separate mutex for each such space, so that
 * another thread isn't unnecessarily blocked.  But, most of them need to be
 * locked against the locale changing while accessing that space, and it is not
 * expected that any will be called frequently, and the locked interval should
 * be short, and modern platforms will have reentrant versions (which don't
 * lock) for almost all of them, so khw thinks a single mutex should suffice.
 * Having a single mutex facilitates that, avoiding potential deadlock
 * situations.
 *
 * This will be a no-op iff the perl is unthreaded. 'gw' stands for 'global
 * write', to indicate the caller wants to be able to access memory that isn't
 * thread specific, either to write to itself, or to prevent anyone else from
 * writing. */
#define gwLOCALE_LOCK    LOCALE_LOCK_(0)
#define gwLOCALE_UNLOCK  LOCALE_UNLOCK_

/* setlocale() generally returns in a global static buffer, but not on Windows
 * when operating in thread-safe mode */
#if defined(WIN32) && defined(USE_THREAD_SAFE_LOCALE)
#  define POSIX_SETLOCALE_LOCK                                              \
            STMT_START {                                                    \
                if (_configthreadlocale(0) == _DISABLE_PER_THREAD_LOCALE)   \
                    gwLOCALE_LOCK;                                          \
            } STMT_END
#  define POSIX_SETLOCALE_UNLOCK                                            \
            STMT_START {                                                    \
                if (_configthreadlocale(0) == _DISABLE_PER_THREAD_LOCALE)   \
                    gwLOCALE_UNLOCK;                                        \
            } STMT_END
#else
#  define POSIX_SETLOCALE_LOCK      gwLOCALE_LOCK
#  define POSIX_SETLOCALE_UNLOCK    gwLOCALE_UNLOCK
#endif

/* It handles _wsetlocale() as well */
#define WSETLOCALE_LOCK      POSIX_SETLOCALE_LOCK
#define WSETLOCALE_UNLOCK    POSIX_SETLOCALE_UNLOCK

/* Similar to gwLOCALE_LOCK, there are functions that require both the locale
 * and environment to be constant during their execution, and don't change
 * either of those things, but do write to some sort of shared global space.
 * They require some sort of exclusive lock against similar functions, and a
 * read lock on both the locale and environment.  However, on systems which
 * have per-thread locales, the locale is constant during the execution of
 * these functions, and so no locale lock is necssary.  For such systems, an
 * exclusive ENV lock is necessary and sufficient.  On systems where the locale
 * could change out from under us, we use an exclusive LOCALE lock to prevent
 * that, and a read ENV lock to prevent other threads that have nothing to do
 * with locales here from changing the environment. */
#ifdef SETLOCALE_LOCK
#  define gwENVr_LOCALEr_LOCK                                               \
                    STMT_START { SETLOCALE_LOCK; ENV_READ_LOCK; } STMT_END
#  define gwENVr_LOCALEr_UNLOCK                                             \
                STMT_START { ENV_READ_UNLOCK; SETLOCALE_UNLOCK; } STMT_END
#else
#  define gwENVr_LOCALEr_LOCK           ENV_LOCK
#  define gwENVr_LOCALEr_UNLOCK         ENV_UNLOCK
#endif

/* Now that we have defined gwENVr_LOCALEr_LOCK, we can finish defining
 * SETLOCALE_LOCK, which we kept undefined until here on a thread-safe system
 * so that we could use that fact to calculate what gwENVr_LOCALEr_LOCK should
 * be */
#ifndef SETLOCALE_LOCK
#  define SETLOCALE_LOCK                NOOP
#  define SETLOCALE_UNLOCK              NOOP
#endif


      /* On systems that don't have per-thread locales, even though we don't
       * think we are changing the locale ourselves, behind the scenes it does
       * get changed to whatever the thread's should be, so it has to be an
       * exclusive lock.  By defining it here with this name, we can, for the
       * most part, hide this detail from the rest of the code */
/* Currently, the read lock is an exclusive lock */
#define LOCALE_READ_LOCK                SETLOCALE_LOCK
#define LOCALE_READ_UNLOCK              SETLOCALE_UNLOCK


#ifndef LC_NUMERIC_LOCK
#  define LC_NUMERIC_LOCK(cond)   NOOP
#  define LC_NUMERIC_UNLOCK       NOOP
#endif

   /* These non-reentrant versions use global space */
#  define MBLEN_LOCK_                gwLOCALE_LOCK
#  define MBLEN_UNLOCK_              gwLOCALE_UNLOCK

#  define MBTOWC_LOCK_               gwLOCALE_LOCK
#  define MBTOWC_UNLOCK_             gwLOCALE_UNLOCK

#  define WCTOMB_LOCK_               gwLOCALE_LOCK
#  define WCTOMB_UNLOCK_             gwLOCALE_UNLOCK

   /* Whereas the reentrant versions don't (assuming they are called with a
    * per-thread buffer; some have the capability of being called with a NULL
    * parameter, which defeats the reentrancy) */
#  define MBRLEN_LOCK_                  NOOP
#  define MBRLEN_UNLOCK_                NOOP
#  define MBRTOWC_LOCK_                 NOOP
#  define MBRTOWC_UNLOCK_               NOOP
#  define WCRTOMB_LOCK_                 NOOP
#  define WCRTOMB_UNLOCK_               NOOP

#  define LC_COLLATE_LOCK               SETLOCALE_LOCK
#  define LC_COLLATE_UNLOCK             SETLOCALE_UNLOCK

#  define STRFTIME_LOCK                 ENV_LOCK
#  define STRFTIME_UNLOCK               ENV_UNLOCK

#ifdef USE_LOCALE_NUMERIC

/* These macros are for toggling between the underlying locale (UNDERLYING or
 * LOCAL) and the C locale (STANDARD).  (Actually we don't have to use the C
 * locale if the underlying locale is indistinguishable from it in the numeric
 * operations used by Perl, namely the decimal point, and even the thousands
 * separator.)

=for apidoc_section $locale

=for apidoc Amn|void|DECLARATION_FOR_LC_NUMERIC_MANIPULATION

This macro should be used as a statement.  It declares a private variable
(whose name begins with an underscore) that is needed by the other macros in
this section.  Failing to include this correctly should lead to a syntax error.
For compatibility with C89 C compilers it should be placed in a block before
any executable statements.

=for apidoc Am|void|STORE_LC_NUMERIC_FORCE_TO_UNDERLYING

This is used by XS code that is C<LC_NUMERIC> locale-aware to force the
locale for category C<LC_NUMERIC> to be what perl thinks is the current
underlying locale.  (The perl interpreter could be wrong about what the
underlying locale actually is if some C or XS code has called the C library
function L<setlocale(3)> behind its back; calling L</sync_locale> before calling
this macro will update perl's records.)

A call to L</DECLARATION_FOR_LC_NUMERIC_MANIPULATION> must have been made to
declare at compile time a private variable used by this macro.  This macro
should be called as a single statement, not an expression, but with an empty
argument list, like this:

 {
    DECLARATION_FOR_LC_NUMERIC_MANIPULATION;
     ...
    STORE_LC_NUMERIC_FORCE_TO_UNDERLYING();
     ...
    RESTORE_LC_NUMERIC();
     ...
 }

The private variable is used to save the current locale state, so
that the requisite matching call to L</RESTORE_LC_NUMERIC> can restore it.

On threaded perls not operating with thread-safe functionality, this macro uses
a mutex to force a critical section.  Therefore the matching RESTORE should be
close by, and guaranteed to be called.

=for apidoc Am|void|STORE_LC_NUMERIC_SET_TO_NEEDED

This is used to help wrap XS or C code that is C<LC_NUMERIC> locale-aware.
This locale category is generally kept set to a locale where the decimal radix
character is a dot, and the separator between groups of digits is empty.  This
is because most XS code that reads floating point numbers is expecting them to
have this syntax.

This macro makes sure the current C<LC_NUMERIC> state is set properly, to be
aware of locale if the call to the XS or C code from the Perl program is
from within the scope of a S<C<use locale>>; or to ignore locale if the call is
instead from outside such scope.

This macro is the start of wrapping the C or XS code; the wrap ending is done
by calling the L</RESTORE_LC_NUMERIC> macro after the operation.  Otherwise
the state can be changed that will adversely affect other XS code.

A call to L</DECLARATION_FOR_LC_NUMERIC_MANIPULATION> must have been made to
declare at compile time a private variable used by this macro.  This macro
should be called as a single statement, not an expression, but with an empty
argument list, like this:

 {
    DECLARATION_FOR_LC_NUMERIC_MANIPULATION;
     ...
    STORE_LC_NUMERIC_SET_TO_NEEDED();
     ...
    RESTORE_LC_NUMERIC();
     ...
 }

On threaded perls not operating with thread-safe functionality, this macro uses
a mutex to force a critical section.  Therefore the matching RESTORE should be
close by, and guaranteed to be called; see L</WITH_LC_NUMERIC_SET_TO_NEEDED>
for a more contained way to ensure that.

=for apidoc Am|void|STORE_LC_NUMERIC_SET_TO_NEEDED_IN|bool in_lc_numeric

Same as L</STORE_LC_NUMERIC_SET_TO_NEEDED> with in_lc_numeric provided
as the precalculated value of C<IN_LC(LC_NUMERIC)>. It is the caller's
responsibility to ensure that the status of C<PL_compiling> and C<PL_hints>
cannot have changed since the precalculation.

=for apidoc Am|void|RESTORE_LC_NUMERIC

This is used in conjunction with one of the macros
L</STORE_LC_NUMERIC_SET_TO_NEEDED>
and L</STORE_LC_NUMERIC_FORCE_TO_UNDERLYING> to properly restore the
C<LC_NUMERIC> state.

A call to L</DECLARATION_FOR_LC_NUMERIC_MANIPULATION> must have been made to
declare at compile time a private variable used by this macro and the two
C<STORE> ones.  This macro should be called as a single statement, not an
expression, but with an empty argument list, like this:

 {
    DECLARATION_FOR_LC_NUMERIC_MANIPULATION;
     ...
    RESTORE_LC_NUMERIC();
     ...
 }

=for apidoc Am|void|WITH_LC_NUMERIC_SET_TO_NEEDED|block

This macro invokes the supplied statement or block within the context
of a L</STORE_LC_NUMERIC_SET_TO_NEEDED> .. L</RESTORE_LC_NUMERIC> pair
if required, so eg:

  WITH_LC_NUMERIC_SET_TO_NEEDED(
    SNPRINTF_G(fv, ebuf, sizeof(ebuf), precis)
  );

is equivalent to:

  {
#ifdef USE_LOCALE_NUMERIC
    DECLARATION_FOR_LC_NUMERIC_MANIPULATION;
    STORE_LC_NUMERIC_SET_TO_NEEDED();
#endif
    SNPRINTF_G(fv, ebuf, sizeof(ebuf), precis);
#ifdef USE_LOCALE_NUMERIC
    RESTORE_LC_NUMERIC();
#endif
  }

=for apidoc Am|void|WITH_LC_NUMERIC_SET_TO_NEEDED_IN|bool in_lc_numeric|block

Same as L</WITH_LC_NUMERIC_SET_TO_NEEDED> with in_lc_numeric provided
as the precalculated value of C<IN_LC(LC_NUMERIC)>. It is the caller's
responsibility to ensure that the status of C<PL_compiling> and C<PL_hints>
cannot have changed since the precalculation.

=cut

*/

/* If the underlying numeric locale has a non-dot decimal point or has a
 * non-empty floating point thousands separator, the current locale is instead
 * generally kept in the C locale instead of that underlying locale.  The
 * current status is known by looking at two words.  One is non-zero if the
 * current numeric locale is the standard C/POSIX one or is indistinguishable
 * from C.  The other is non-zero if the current locale is the underlying
 * locale.  Both can be non-zero if, as often happens, the underlying locale is
 * C or indistinguishable from it.
 *
 * khw believes the reason for the variables instead of the bits in a single
 * word is to avoid having to have masking instructions. */

#  define NOT_IN_NUMERIC_STANDARD_ (! PL_numeric_standard)

/* We can lock the category to stay in the C locale, making requests to the
 * contrary be noops, in the dynamic scope by setting PL_numeric_standard to 2.
 * */
#  define NOT_IN_NUMERIC_UNDERLYING_                                        \
                    (! PL_numeric_underlying && PL_numeric_standard < 2)

#  define DECLARATION_FOR_LC_NUMERIC_MANIPULATION                           \
    void (*_restore_LC_NUMERIC_function)(pTHX) = NULL

#  define STORE_LC_NUMERIC_SET_TO_NEEDED_IN(in)                             \
        STMT_START {                                                        \
            bool _in_lc_numeric = (in);                                     \
            LC_NUMERIC_LOCK(                                                \
                    (   (  _in_lc_numeric && NOT_IN_NUMERIC_UNDERLYING_)    \
                     || (! _in_lc_numeric && NOT_IN_NUMERIC_STANDARD_)));   \
            if (_in_lc_numeric) {                                           \
                if (NOT_IN_NUMERIC_UNDERLYING_) {                           \
                    Perl_set_numeric_underlying(aTHX);                      \
                    _restore_LC_NUMERIC_function                            \
                                            = &Perl_set_numeric_standard;   \
                }                                                           \
            }                                                               \
            else {                                                          \
                if (NOT_IN_NUMERIC_STANDARD_) {                             \
                    Perl_set_numeric_standard(aTHX);                        \
                    _restore_LC_NUMERIC_function                            \
                                            = &Perl_set_numeric_underlying; \
                }                                                           \
            }                                                               \
        } STMT_END

#  define STORE_LC_NUMERIC_SET_TO_NEEDED() \
        STORE_LC_NUMERIC_SET_TO_NEEDED_IN(IN_LC(LC_NUMERIC))

#  define RESTORE_LC_NUMERIC()                                              \
        STMT_START {                                                        \
            if (_restore_LC_NUMERIC_function) {                             \
                _restore_LC_NUMERIC_function(aTHX);                         \
            }                                                               \
            LC_NUMERIC_UNLOCK;                                              \
        } STMT_END

/* The next two macros should be rarely used, and only after being sure that
 * this is what is needed */
#  define SET_NUMERIC_STANDARD()                                            \
        STMT_START {                                                        \
            DEBUG_Lv(PerlIO_printf(Perl_debug_log,                          \
                               "%s: %d: lc_numeric standard=%d\n",          \
                                __FILE__, __LINE__, PL_numeric_standard));  \
            if (UNLIKELY(NOT_IN_NUMERIC_STANDARD_)) {                       \
                Perl_set_numeric_standard(aTHX);                            \
            }                                                               \
            DEBUG_Lv(PerlIO_printf(Perl_debug_log,                          \
                                 "%s: %d: lc_numeric standard=%d\n",        \
                                 __FILE__, __LINE__, PL_numeric_standard)); \
        } STMT_END

#  define SET_NUMERIC_UNDERLYING()                                          \
	STMT_START {                                                        \
          /*assert(PL_locale_mutex_depth > 0);*/                            \
            if (NOT_IN_NUMERIC_UNDERLYING_) {                               \
                Perl_set_numeric_underlying(aTHX);                          \
            }                                                               \
        } STMT_END

/* The rest of these LC_NUMERIC macros toggle to one or the other state, with
 * the RESTORE_foo ones called to switch back, but only if need be */
#  define STORE_LC_NUMERIC_SET_STANDARD()                                   \
        STMT_START {                                                        \
            LC_NUMERIC_LOCK(NOT_IN_NUMERIC_STANDARD_);                      \
            if (NOT_IN_NUMERIC_STANDARD_) {                                 \
                _restore_LC_NUMERIC_function = &Perl_set_numeric_underlying;\
                Perl_set_numeric_standard(aTHX);                            \
            }                                                               \
        } STMT_END

/* Rarely, we want to change to the underlying locale even outside of 'use
 * locale'.  This is principally in the POSIX:: functions */
#  define STORE_LC_NUMERIC_FORCE_TO_UNDERLYING()                            \
	STMT_START {                                                        \
            LC_NUMERIC_LOCK(NOT_IN_NUMERIC_UNDERLYING_);                    \
            if (NOT_IN_NUMERIC_UNDERLYING_) {                               \
                Perl_set_numeric_underlying(aTHX);                          \
                _restore_LC_NUMERIC_function = &Perl_set_numeric_standard;  \
            }                                                               \
        } STMT_END

/* Lock/unlock to the C locale until unlock is called.  This needs to be
 * recursively callable.  [perl #128207] */
#  define LOCK_LC_NUMERIC_STANDARD()                                        \
        STMT_START {                                                        \
            DEBUG_Lv(PerlIO_printf(Perl_debug_log,                          \
                    "%s: %d: lc_numeric_standard now locked to depth %d\n", \
                    __FILE__, __LINE__, PL_numeric_standard));              \
            __ASSERT_(PL_numeric_standard)                                  \
            PL_numeric_standard++;                                          \
        } STMT_END

#  define UNLOCK_LC_NUMERIC_STANDARD()                                      \
        STMT_START {                                                        \
            if (PL_numeric_standard > 1) {                                  \
                PL_numeric_standard--;                                      \
            }                                                               \
            else {                                                          \
                assert(0);                                                  \
            }                                                               \
            DEBUG_Lv(PerlIO_printf(Perl_debug_log,                          \
                                   "%s: %d: ",  __FILE__, __LINE__);        \
                    if (PL_numeric_standard <= 1)                           \
                        PerlIO_printf(Perl_debug_log,                       \
                                      "lc_numeric_standard now unlocked\n");\
                    else PerlIO_printf(Perl_debug_log,                      \
                     "lc_numeric_standard lock decremented to depth %d\n",  \
                                                     PL_numeric_standard););\
        } STMT_END

#  define WITH_LC_NUMERIC_SET_TO_NEEDED_IN(in_lc_numeric, block)            \
        STMT_START {                                                        \
            DECLARATION_FOR_LC_NUMERIC_MANIPULATION;                        \
            STORE_LC_NUMERIC_SET_TO_NEEDED_IN(in_lc_numeric);               \
            block;                                                          \
            RESTORE_LC_NUMERIC();                                           \
        } STMT_END;

#  define WITH_LC_NUMERIC_SET_TO_NEEDED(block) \
        WITH_LC_NUMERIC_SET_TO_NEEDED_IN(IN_LC(LC_NUMERIC), block)

#else /* !USE_LOCALE_NUMERIC */

#  define SET_NUMERIC_STANDARD()
#  define SET_NUMERIC_UNDERLYING()
#  define IS_NUMERIC_RADIX(a, b)		(0)
#  define DECLARATION_FOR_LC_NUMERIC_MANIPULATION  dNOOP
#  define STORE_LC_NUMERIC_SET_STANDARD()
#  define STORE_LC_NUMERIC_FORCE_TO_UNDERLYING()
#  define STORE_LC_NUMERIC_SET_TO_NEEDED_IN(in_lc_numeric)
#  define STORE_LC_NUMERIC_SET_TO_NEEDED()
#  define RESTORE_LC_NUMERIC()
#  define LOCK_LC_NUMERIC_STANDARD()
#  define UNLOCK_LC_NUMERIC_STANDARD()
#  define WITH_LC_NUMERIC_SET_TO_NEEDED_IN(in_lc_numeric, block) \
    STMT_START { block; } STMT_END
#  define WITH_LC_NUMERIC_SET_TO_NEEDED(block) \
    STMT_START { block; } STMT_END

#endif /* !USE_LOCALE_NUMERIC */

#ifdef USE_LOCALE_THREADS
#  define ENV_LOCK            PERL_WRITE_LOCK(&PL_env_mutex)
#  define ENV_UNLOCK          PERL_WRITE_UNLOCK(&PL_env_mutex)
#  define ENV_READ_LOCK       PERL_READ_LOCK(&PL_env_mutex)
#  define ENV_READ_UNLOCK     PERL_READ_UNLOCK(&PL_env_mutex)
#  define ENV_INIT            PERL_RW_MUTEX_INIT(&PL_env_mutex)
#  define ENV_TERM            PERL_RW_MUTEX_DESTROY(&PL_env_mutex)

   /* On platforms where the static buffer contained in getenv() is per-thread
    * rather than process-wide, another thread executing a getenv() at the same
    * time won't destroy ours before we have copied the result safely away and
    * unlocked the mutex.  On such platforms (which is most), we can have many
    * readers of the environment at the same time. */
#  ifdef GETENV_PRESERVES_OTHER_THREAD
#    define GETENV_LOCK    ENV_READ_LOCK
#    define GETENV_UNLOCK  ENV_READ_UNLOCK
#  else
     /* If, on the other hand, another thread could zap our getenv() return, we
      * need to keep them from executing until we are done */
#    define GETENV_LOCK    ENV_LOCK
#    define GETENV_UNLOCK  ENV_UNLOCK
#  endif
#else
#  define ENV_LOCK        NOOP
#  define ENV_UNLOCK      NOOP
#  define ENV_READ_LOCK   NOOP
#  define ENV_READ_UNLOCK NOOP
#  define ENV_INIT        NOOP
#  define ENV_TERM        NOOP
#  define GETENV_LOCK     NOOP
#  define GETENV_UNLOCK   NOOP
#endif

/* Some critical sections need to lock both the locale and the environment from
 * changing, while allowing for any number of readers.  To avoid deadlock, this
 * is always done in the same order.  These should always be invoked, like all
 * locks really, at such a low level that its just a libc call that is wrapped,
 * so as to prevent recursive calls which could deadlock. */
#define ENVr_LOCALEr_LOCK                                               \
            STMT_START { LOCALE_READ_LOCK; ENV_READ_LOCK; } STMT_END
#define ENVr_LOCALEr_UNLOCK                                             \
        STMT_START { ENV_READ_UNLOCK; LOCALE_READ_UNLOCK; } STMT_END

/* These time-related functions all requre that the environment and locale
 * don't change while they are executing (at least in glibc; this appears to be
 * contrary to the POSIX standard).  tzset() writes global variables, so
 * always needs to have write locking.  ctime, localtime, mktime, and strftime
 * effectively call it, so they too need exclusive access.  The rest need to
 * have exclusive locking as well so that they can copy the contents of the
 * returned static buffer before releasing the lock.  That leaves asctime and
 * gmtime.  There may be reentrant versions of these available on the platform
 * which don't require write locking.
 */
#ifdef PERL_REENTR_USING_ASCTIME_R
#  define ASCTIME_LOCK     ENVr_LOCALEr_LOCK
#  define ASCTIME_UNLOCK   ENVr_LOCALEr_UNLOCK
#else
#  define ASCTIME_LOCK     gwENVr_LOCALEr_LOCK
#  define ASCTIME_UNLOCK   gwENVr_LOCALEr_UNLOCK
#endif

#define CTIME_LOCK         gwENVr_LOCALEr_LOCK
#define CTIME_UNLOCK       gwENVr_LOCALEr_UNLOCK

#ifdef PERL_REENTR_USING_GMTIME_R
#  define GMTIME_LOCK      ENVr_LOCALEr_LOCK
#  define GMTIME_UNLOCK    ENVr_LOCALEr_UNLOCK
#else
#  define GMTIME_LOCK      gwENVr_LOCALEr_LOCK
#  define GMTIME_UNLOCK    gwENVr_LOCALEr_UNLOCK
#endif

#define LOCALTIME_LOCK     gwENVr_LOCALEr_LOCK
#define LOCALTIME_UNLOCK   gwENVr_LOCALEr_UNLOCK
#define MKTIME_LOCK        gwENVr_LOCALEr_LOCK
#define MKTIME_UNLOCK      gwENVr_LOCALEr_UNLOCK
#define TZSET_LOCK         gwENVr_LOCALEr_LOCK
#define TZSET_UNLOCK       gwENVr_LOCALEr_UNLOCK

/* Similiarly, these functions need a constant environment and/or locale.  And
 * some have a buffer that is shared with another thread executing the same or
 * a related call.  A mutex could be created for each class, but for now, share
 * the ENV mutex with everything, as none probably gets called so much that
 * performance would suffer by a thread being locked out by another thread that
 * could have used a different mutex.
 *
 * But, create a different macro name just to indicate the ones that don't
 * actually depend on the environment, but are using its mutex for want of a
 * better one */
#define gwLOCALEr_LOCK              gwENVr_LOCALEr_LOCK
#define gwLOCALEr_UNLOCK            gwENVr_LOCALEr_UNLOCK

#ifdef PERL_REENTR_USING_GETHOSTBYADDR_R
#  define GETHOSTBYADDR_LOCK        ENVr_LOCALEr_LOCK
#  define GETHOSTBYADDR_UNLOCK      ENVr_LOCALEr_UNLOCK
#else
#  define GETHOSTBYADDR_LOCK        gwENVr_LOCALEr_LOCK
#  define GETHOSTBYADDR_UNLOCK      gwENVr_LOCALEr_UNLOCK
#endif
#ifdef PERL_REENTR_USING_GETHOSTBYNAME_R
#  define GETHOSTBYNAME_LOCK        ENVr_LOCALEr_LOCK
#  define GETHOSTBYNAME_UNLOCK      ENVr_LOCALEr_UNLOCK
#else
#  define GETHOSTBYNAME_LOCK        gwENVr_LOCALEr_LOCK
#  define GETHOSTBYNAME_UNLOCK      gwENVr_LOCALEr_UNLOCK
#endif
#ifdef PERL_REENTR_USING_GETNETBYADDR_R
#  define GETNETBYADDR_LOCK         LOCALE_READ_LOCK
#  define GETNETBYADDR_UNLOCK       LOCALE_READ_UNLOCK
#else
#  define GETNETBYADDR_LOCK         gwLOCALEr_LOCK
#  define GETNETBYADDR_UNLOCK       gwLOCALEr_UNLOCK
#endif
#ifdef PERL_REENTR_USING_GETNETBYNAME_R
#  define GETNETBYNAME_LOCK         LOCALE_READ_LOCK
#  define GETNETBYNAME_UNLOCK       LOCALE_READ_UNLOCK
#else
#  define GETNETBYNAME_LOCK         gwLOCALEr_LOCK
#  define GETNETBYNAME_UNLOCK       gwLOCALEr_UNLOCK
#endif
#ifdef PERL_REENTR_USING_GETPROTOBYNAME_R
#  define GETPROTOBYNAME_LOCK       LOCALE_READ_LOCK
#  define GETPROTOBYNAME_UNLOCK     LOCALE_READ_UNLOCK
#else
#  define GETPROTOBYNAME_LOCK       gwLOCALEr_LOCK
#  define GETPROTOBYNAME_UNLOCK     gwLOCALEr_UNLOCK
#endif
#ifdef PERL_REENTR_USING_GETPROTOBYNUMBER_R
#  define GETPROTOBYNUMBER_LOCK     LOCALE_READ_LOCK
#  define GETPROTOBYNUMBER_UNLOCK   LOCALE_READ_UNLOCK
#else
#  define GETPROTOBYNUMBER_LOCK     gwLOCALEr_LOCK
#  define GETPROTOBYNUMBER_UNLOCK   gwLOCALEr_UNLOCK
#endif
#ifdef PERL_REENTR_USING_GETPROTOENT_R
#  define GETPROTOENT_LOCK          LOCALE_READ_LOCK
#  define GETPROTOENT_UNLOCK        LOCALE_READ_UNLOCK
#else
#  define GETPROTOENT_LOCK          gwLOCALEr_LOCK
#  define GETPROTOENT_UNLOCK        gwLOCALEr_UNLOCK
#endif
#ifdef PERL_REENTR_USING_GETPWNAM_R
#  define GETPWNAM_LOCK             LOCALE_READ_LOCK
#  define GETPWNAM_UNLOCK           LOCALE_READ_UNLOCK
#else
#  define GETPWNAM_LOCK             gwLOCALEr_LOCK
#  define GETPWNAM_UNLOCK           gwLOCALEr_UNLOCK
#endif
#ifdef PERL_REENTR_USING_GETPWUID_R
#  define GETPWUID_LOCK             LOCALE_READ_LOCK
#  define GETPWUID_UNLOCK           LOCALE_READ_UNLOCK
#else
#  define GETPWUID_LOCK             gwLOCALEr_LOCK
#  define GETPWUID_UNLOCK           gwLOCALEr_UNLOCK
#endif
#ifdef PERL_REENTR_USING_GETSERVBYNAME_R
#  define GETSERVBYNAME_LOCK        LOCALE_READ_LOCK
#  define GETSERVBYNAME_UNLOCK      LOCALE_READ_UNLOCK
#else
#  define GETSERVBYNAME_LOCK        gwLOCALEr_LOCK
#  define GETSERVBYNAME_UNLOCK      gwLOCALEr_UNLOCK
#endif
#ifdef PERL_REENTR_USING_GETSERVBYPORT_R
#  define GETSERVBYPORT_LOCK        LOCALE_READ_LOCK
#  define GETSERVBYPORT_UNLOCK      LOCALE_READ_UNLOCK
#else
#  define GETSERVBYPORT_LOCK        gwLOCALEr_LOCK
#  define GETSERVBYPORT_UNLOCK      gwLOCALEr_UNLOCK
#endif
#ifdef PERL_REENTR_USING_GETSERVENT_R
#  define GETSERVENT_LOCK           LOCALE_READ_LOCK
#  define GETSERVENT_UNLOCK         LOCALE_READ_UNLOCK
#else
#  define GETSERVENT_LOCK           gwLOCALEr_LOCK
#  define GETSERVENT_UNLOCK         gwLOCALEr_UNLOCK
#endif
#ifdef PERL_REENTR_USING_GETSPNAM_R
#  define GETSPNAM_LOCK             LOCALE_READ_LOCK
#  define GETSPNAM_UNLOCK           LOCALE_READ_UNLOCK
#else
#  define GETSPNAM_LOCK             gwLOCALEr_LOCK
#  define GETSPNAM_UNLOCK           gwLOCALEr_UNLOCK
#endif

#define STRFMON_LOCK        LC_MONETARY_LOCK
#define STRFMON_UNLOCK      LC_MONETARY_UNLOCK

/* End of locale/env synchronization */

#ifndef PERL_NO_INLINE_FUNCTIONS
/* Static inline funcs that depend on includes and declarations above.
   Some of these reference functions in the perl object files, and some
   compilers aren't smart enough to eliminate unused static inline
   functions, so including this file in source code can cause link errors
   even if the source code uses none of the functions. Hence including these
   can be suppressed by setting PERL_NO_INLINE_FUNCTIONS. Doing this will
   (obviously) result in unworkable XS code, but allows simple probing code
   to continue to work, because it permits tests to include the perl headers
   for definitions without creating a link dependency on the perl library
   (which may not exist yet).
*/

START_EXTERN_C

#  include "perlstatic.h"
#  include "inline.h"
#  include "sv_inline.h"

END_EXTERN_C

#endif

/*

=for apidoc_section $numeric

=for apidoc AmTR|NV|Strtod|NN const char * const s|NULLOK char ** e

This is a synonym for L</my_strtod>.

=for apidoc AmTR|NV|Strtol|NN const char * const s|NULLOK char ** e|int base

Platform and configuration independent C<strtol>.  This expands to the
appropriate C<strotol>-like function based on the platform and F<Configure>
options>.  For example it could expand to C<strtoll> or C<strtoq> instead of
C<strtol>.

=for apidoc AmTR|NV|Strtoul|NN const char * const s|NULLOK char ** e|int base

Platform and configuration independent C<strtoul>.  This expands to the
appropriate C<strotoul>-like function based on the platform and F<Configure>
options>.  For example it could expand to C<strtoull> or C<strtouq> instead of
C<strtoul>.

=cut

*/

#define Strtod                          my_strtod

#if    defined(HAS_STRTOD)                                          \
   ||  defined(USE_QUADMATH)                                        \
   || (defined(HAS_STRTOLD) && defined(HAS_LONG_DOUBLE)             \
                            && defined(USE_LONG_DOUBLE))
#  define Perl_strtod   Strtod
#endif

#if !defined(Strtol) && defined(USE_64_BIT_INT) && defined(IV_IS_QUAD) && \
        (QUADKIND == QUAD_IS_LONG_LONG || QUADKIND == QUAD_IS___INT64)
#    ifdef __hpux
#        define strtoll __strtoll	/* secret handshake */
#    endif
#    if defined(WIN64) && defined(_MSC_VER)
#        define strtoll _strtoi64	/* secret handshake */
#    endif
#   if !defined(Strtol) && defined(HAS_STRTOLL)
#       define Strtol	strtoll
#   endif
#    if !defined(Strtol) && defined(HAS_STRTOQ)
#       define Strtol	strtoq
#    endif
/* is there atoq() anywhere? */
#endif
#if !defined(Strtol) && defined(HAS_STRTOL)
#   define Strtol	strtol
#endif
#ifndef Atol
/* It would be more fashionable to use Strtol() to define atol()
 * (as is done for Atoul(), see below) but for backward compatibility
 * we just assume atol(). */
#   if defined(USE_64_BIT_INT) && defined(IV_IS_QUAD) && defined(HAS_ATOLL) && \
        (QUADKIND == QUAD_IS_LONG_LONG || QUADKIND == QUAD_IS___INT64)
#    ifdef WIN64
#       define atoll    _atoi64		/* secret handshake */
#    endif
#       define Atol	atoll
#   else
#       define Atol	atol
#   endif
#endif

#if !defined(Strtoul) && defined(USE_64_BIT_INT) && defined(UV_IS_QUAD) && \
        (QUADKIND == QUAD_IS_LONG_LONG || QUADKIND == QUAD_IS___INT64)
#    ifdef __hpux
#        define strtoull __strtoull	/* secret handshake */
#    endif
#    if defined(WIN64) && defined(_MSC_VER)
#        define strtoull _strtoui64	/* secret handshake */
#    endif
#    if !defined(Strtoul) && defined(HAS_STRTOULL)
#       define Strtoul	strtoull
#    endif
#    if !defined(Strtoul) && defined(HAS_STRTOUQ)
#       define Strtoul	strtouq
#    endif
/* is there atouq() anywhere? */
#endif
#if !defined(Strtoul) && defined(HAS_STRTOUL)
#   define Strtoul	strtoul
#endif
#if !defined(Strtoul) && defined(HAS_STRTOL) /* Last resort. */
#   define Strtoul(s, e, b)	strchr((s), '-') ? ULONG_MAX : (unsigned long)strtol((s), (e), (b))
#endif
#ifndef Atoul
#   define Atoul(s)	Strtoul(s, NULL, 10)
#endif

#define grok_bin(s,lp,fp,rp)                                                \
                    grok_bin_oct_hex(s, lp, fp, rp, 1, CC_BINDIGIT_, 'b')
#define grok_oct(s,lp,fp,rp)                                                \
                    (*(fp) |= PERL_SCAN_DISALLOW_PREFIX,                    \
                    grok_bin_oct_hex(s, lp, fp, rp, 3, CC_OCTDIGIT_, '\0'))
#define grok_hex(s,lp,fp,rp)                                                \
                    grok_bin_oct_hex(s, lp, fp, rp, 4, CC_XDIGIT_, 'x')

#ifndef PERL_SCRIPT_MODE
#define PERL_SCRIPT_MODE "r"
#endif

/* not used. Kept as a NOOP for backcompat */
#define PERL_STACK_OVERFLOW_CHECK()  NOOP

/*
 * Some nonpreemptive operating systems find it convenient to
 * check for asynchronous conditions after each op execution.
 * Keep this check simple, or it may slow down execution
 * massively.
 */

#ifndef PERL_MICRO
#	ifndef PERL_ASYNC_CHECK
#		define PERL_ASYNC_CHECK() if (UNLIKELY(PL_sig_pending)) PL_signalhook(aTHX)
#	endif
#endif

#ifndef PERL_ASYNC_CHECK
#   define PERL_ASYNC_CHECK()  NOOP
#endif

/*
 * On some operating systems, a memory allocation may succeed,
 * but put the process too close to the system's comfort limit.
 * In this case, PERL_ALLOC_CHECK frees the pointer and sets
 * it to NULL.
 */
#ifndef PERL_ALLOC_CHECK
#define PERL_ALLOC_CHECK(p)  NOOP
#endif

#ifdef HAS_SEM
#   include <sys/ipc.h>
#   include <sys/sem.h>
#   ifndef HAS_UNION_SEMUN	/* Provide the union semun. */
    union semun {
        int		val;
        struct semid_ds	*buf;
        unsigned short	*array;
    };
#   endif
#   ifdef USE_SEMCTL_SEMUN
#	ifdef IRIX32_SEMUN_BROKEN_BY_GCC
            union gccbug_semun {
                int             val;
                struct semid_ds *buf;
                unsigned short  *array;
                char            __dummy[5];
            };
#           define semun gccbug_semun
#	endif
#       define Semctl(id, num, cmd, semun) semctl(id, num, cmd, semun)
#   elif defined(USE_SEMCTL_SEMID_DS)
#           ifdef EXTRA_F_IN_SEMUN_BUF
#               define Semctl(id, num, cmd, semun) semctl(id, num, cmd, semun.buff)
#           else
#               define Semctl(id, num, cmd, semun) semctl(id, num, cmd, semun.buf)
#           endif
#   endif
#endif

/*
 * Boilerplate macros for initializing and accessing interpreter-local
 * data from C.  All statics in extensions should be reworked to use
 * this, if you want to make the extension thread-safe.  See
 * ext/XS/APItest/APItest.xs for an example of the use of these macros,
 * and perlxs.pod for more.
 *
 * Code that uses these macros is responsible for the following:
 * 1. #define MY_CXT_KEY to a unique string, e.g.
 *    "DynaLoader::_guts" XS_VERSION
 *    XXX in the current implementation, this string is ignored.
 * 2. Declare a typedef named my_cxt_t that is a structure that contains
 *    all the data that needs to be interpreter-local that perl controls.  This
 *    doesn't include things that libc controls, such as the uselocale object
 *    in Configurations that use it.
 * 3. Use the START_MY_CXT macro after the declaration of my_cxt_t.
 * 4. Use the MY_CXT_INIT macro such that it is called exactly once
 *    (typically put in the BOOT: section).
 * 5. Use the members of the my_cxt_t structure everywhere as
 *    MY_CXT.member.
 * 6. Use the dMY_CXT macro (a declaration) in all the functions that
 *    access MY_CXT.
 */

#if defined(MULTIPLICITY)

/* START_MY_CXT must appear in all extensions that define a my_cxt_t structure,
 * right after the definition (i.e. at file scope).  The non-threads
 * case below uses it to declare the data as static. */
#    define START_MY_CXT static int my_cxt_index = -1;
#    define MY_CXT_INDEX my_cxt_index
#    define MY_CXT_INIT_ARG &my_cxt_index

/* Creates and zeroes the per-interpreter data.
 * (We allocate my_cxtp in a Perl SV so that it will be released when
 * the interpreter goes away.) */
#  define MY_CXT_INIT \
        my_cxt_t *my_cxtp = \
            (my_cxt_t*)Perl_my_cxt_init(aTHX_ MY_CXT_INIT_ARG, sizeof(my_cxt_t)); \
        PERL_UNUSED_VAR(my_cxtp)
#  define MY_CXT_INIT_INTERP(my_perl) \
        my_cxt_t *my_cxtp = \
            (my_cxt_t*)Perl_my_cxt_init(my_perl, MY_CXT_INIT_ARG, sizeof(my_cxt_t)); \
        PERL_UNUSED_VAR(my_cxtp)

/* This declaration should be used within all functions that use the
 * interpreter-local data. */
#  define dMY_CXT	\
        my_cxt_t *my_cxtp = (my_cxt_t *)PL_my_cxt_list[MY_CXT_INDEX]
#  define dMY_CXT_INTERP(my_perl)	\
        my_cxt_t *my_cxtp = (my_cxt_t *)(my_perl)->Imy_cxt_list[MY_CXT_INDEX]

/* Clones the per-interpreter data. */
#  define MY_CXT_CLONE \
        my_cxt_t *my_cxtp = (my_cxt_t*)SvPVX(newSV(sizeof(my_cxt_t)-1));\
        void * old_my_cxtp = PL_my_cxt_list[MY_CXT_INDEX];		\
        PL_my_cxt_list[MY_CXT_INDEX] = my_cxtp;				\
        Copy(old_my_cxtp, my_cxtp, 1, my_cxt_t);



/* This macro must be used to access members of the my_cxt_t structure.
 * e.g. MY_CXT.some_data */
#  define MY_CXT		(*my_cxtp)

/* Judicious use of these macros can reduce the number of times dMY_CXT
 * is used.  Use is similar to pTHX, aTHX etc. */
#  define pMY_CXT	my_cxt_t *my_cxtp
#  define pMY_CXT_	pMY_CXT,
#  define _pMY_CXT	,pMY_CXT
#  define aMY_CXT	my_cxtp
#  define aMY_CXT_	aMY_CXT,
#  define _aMY_CXT	,aMY_CXT

#else /* MULTIPLICITY */
#  define START_MY_CXT		static my_cxt_t my_cxt;
#  define dMY_CXT		dNOOP
#  define dMY_CXT_INTERP(my_perl) dNOOP
#  define MY_CXT_INIT		NOOP
#  define MY_CXT_CLONE		NOOP
#  define MY_CXT		my_cxt

#  define pMY_CXT		void
#  define pMY_CXT_
#  define _pMY_CXT
#  define aMY_CXT
#  define aMY_CXT_
#  define _aMY_CXT

#endif /* !defined(MULTIPLICITY) */

#ifdef I_FCNTL
#  include <fcntl.h>
#endif

#ifdef __Lynx__
#  include <fcntl.h>
#endif

#ifdef __amigaos4__
#  undef FD_CLOEXEC /* a lie in AmigaOS */
#endif

#ifdef I_SYS_FILE
#  include <sys/file.h>
#endif

#if defined(HAS_FLOCK) && !defined(HAS_FLOCK_PROTO)
EXTERN_C int flock(int fd, int op);
#endif

#ifndef O_RDONLY
/* Assume UNIX defaults */
#    define O_RDONLY	0000
#    define O_WRONLY	0001
#    define O_RDWR	0002
#    define O_CREAT	0100
#endif

#ifndef O_BINARY
#  define O_BINARY 0
#endif

#ifndef O_TEXT
#  define O_TEXT 0
#endif

#if O_TEXT != O_BINARY
    /* If you have different O_TEXT and O_BINARY and you are a CRLF shop,
     * that is, you are somehow DOSish. */
#   if defined(__HAIKU__) || defined(__VOS__) || defined(__CYGWIN__)
    /* Haiku has O_TEXT != O_BINARY but O_TEXT and O_BINARY have no effect;
     * Haiku is always UNIXoid (LF), not DOSish (CRLF). */
    /* VOS has O_TEXT != O_BINARY, and they have effect,
     * but VOS always uses LF, never CRLF. */
    /* If you have O_TEXT different from your O_BINARY but you still are
     * not a CRLF shop. */
#       undef PERLIO_USING_CRLF
#   else
    /* If you really are DOSish. */
#      define PERLIO_USING_CRLF 1
#   endif
#endif

#ifdef I_LIBUTIL
#   include <libutil.h>		/* setproctitle() in some FreeBSDs */
#endif

#ifndef EXEC_ARGV_CAST
#define EXEC_ARGV_CAST(x) (char **)x
#endif

#define IS_NUMBER_IN_UV		      0x01 /* number within UV range (maybe not
                                              int).  value returned in pointed-
                                              to UV */
#define IS_NUMBER_GREATER_THAN_UV_MAX 0x02 /* pointed to UV undefined */
#define IS_NUMBER_NOT_INT	      0x04 /* saw . or E notation or infnan */
#define IS_NUMBER_NEG		      0x08 /* leading minus sign */
#define IS_NUMBER_INFINITY	      0x10 /* this is big */
#define IS_NUMBER_NAN                 0x20 /* this is not */
#define IS_NUMBER_TRAILING            0x40 /* number has trailing trash */

/*
=for apidoc_section $numeric

=for apidoc AmdR|bool|GROK_NUMERIC_RADIX|NN const char **sp|NN const char *send

A synonym for L</grok_numeric_radix>

=cut
*/
#define GROK_NUMERIC_RADIX(sp, send) grok_numeric_radix(sp, send)

/* Number scan flags.  All are used for input, the ones used for output are so
 * marked */
#define PERL_SCAN_ALLOW_UNDERSCORES   0x01 /* grok_??? accept _ in numbers */
#define PERL_SCAN_DISALLOW_PREFIX     0x02 /* grok_??? reject 0x in hex etc */

/* grok_??? input: ignored; output: found overflow */
#define PERL_SCAN_GREATER_THAN_UV_MAX 0x04

/* grok_??? don't warn about illegal digits.  To preserve total backcompat,
 * this isn't set on output if one is found.  Instead, see
 * PERL_SCAN_NOTIFY_ILLDIGIT. */
#define PERL_SCAN_SILENT_ILLDIGIT     0x08

#define PERL_SCAN_TRAILING            0x10 /* grok_number_flags() allow trailing
                                              and set IS_NUMBER_TRAILING */

/* These are considered experimental, so not exposed publicly */
#if defined(PERL_CORE) || defined(PERL_EXT)
/* grok_??? don't warn about very large numbers which are <= UV_MAX;
 * output: found such a number */
#  define PERL_SCAN_SILENT_NON_PORTABLE 0x20

/* If this is set on input, and no illegal digit is found, it will be cleared
 * on output; otherwise unchanged */
#  define PERL_SCAN_NOTIFY_ILLDIGIT 0x40

/* Don't warn on overflow; output flag still set */
#  define PERL_SCAN_SILENT_OVERFLOW 0x80

/* Forbid a leading underscore, which the other one doesn't */
#  define PERL_SCAN_ALLOW_MEDIAL_UNDERSCORES (0x100|PERL_SCAN_ALLOW_UNDERSCORES)
#endif


/* to let user control profiling */
#ifdef PERL_GPROF_CONTROL
extern void moncontrol(int);
#define PERL_GPROF_MONCONTROL(x) moncontrol(x)
#else
#define PERL_GPROF_MONCONTROL(x)
#endif

/* ISO 6429 NEL - C1 control NExt Line */
/* See https://www.unicode.org/unicode/reports/tr13/ */
#define NEXT_LINE_CHAR	NEXT_LINE_NATIVE

#ifndef PIPESOCK_MODE
#  define PIPESOCK_MODE
#endif

#ifndef SOCKET_OPEN_MODE
#  define SOCKET_OPEN_MODE	PIPESOCK_MODE
#endif

#ifndef PIPE_OPEN_MODE
#  define PIPE_OPEN_MODE	PIPESOCK_MODE
#endif

#define PERL_MAGIC_UTF8_CACHESIZE	2

#ifdef PERL_CORE

#define PERL_UNICODE_STDIN_FLAG			0x0001
#define PERL_UNICODE_STDOUT_FLAG		0x0002
#define PERL_UNICODE_STDERR_FLAG		0x0004
#define PERL_UNICODE_IN_FLAG			0x0008
#define PERL_UNICODE_OUT_FLAG			0x0010
#define PERL_UNICODE_ARGV_FLAG			0x0020
#define PERL_UNICODE_LOCALE_FLAG		0x0040
#define PERL_UNICODE_WIDESYSCALLS_FLAG		0x0080 /* for Sarathy */
#define PERL_UNICODE_UTF8CACHEASSERT_FLAG	0x0100

#define PERL_UNICODE_STD_FLAG		\
        (PERL_UNICODE_STDIN_FLAG	| \
         PERL_UNICODE_STDOUT_FLAG	| \
         PERL_UNICODE_STDERR_FLAG)

#define PERL_UNICODE_INOUT_FLAG		\
        (PERL_UNICODE_IN_FLAG	| \
         PERL_UNICODE_OUT_FLAG)

#define PERL_UNICODE_DEFAULT_FLAGS	\
        (PERL_UNICODE_STD_FLAG		| \
         PERL_UNICODE_INOUT_FLAG	| \
         PERL_UNICODE_LOCALE_FLAG)

#define PERL_UNICODE_ALL_FLAGS			0x01ff

#define PERL_UNICODE_STDIN			'I'
#define PERL_UNICODE_STDOUT			'O'
#define PERL_UNICODE_STDERR			'E'
#define PERL_UNICODE_STD			'S'
#define PERL_UNICODE_IN				'i'
#define PERL_UNICODE_OUT			'o'
#define PERL_UNICODE_INOUT			'D'
#define PERL_UNICODE_ARGV			'A'
#define PERL_UNICODE_LOCALE			'L'
#define PERL_UNICODE_WIDESYSCALLS		'W'
#define PERL_UNICODE_UTF8CACHEASSERT		'a'

#endif

/*
=for apidoc_section $signals
=for apidoc Amn|U32|PERL_SIGNALS_UNSAFE_FLAG
If this bit in C<PL_signals> is set, the system is uing the pre-Perl 5.8
unsafe signals.  See L<perlrun/PERL_SIGNALS> and L<perlipc/Deferred Signals
(Safe Signals)>.

=cut
*/
#define PERL_SIGNALS_UNSAFE_FLAG	0x0001

/*
=for apidoc_section $numeric

=for apidoc Am|int|PERL_ABS|int x

Typeless C<abs> or C<fabs>, I<etc>.  (The usage below indicates it is for
integers, but it works for any type.)  Use instead of these, since the C
library ones force their argument to be what it is expecting, potentially
leading to disaster.  But also beware that this evaluates its argument twice,
so no C<x++>.

=cut
*/

#define PERL_ABS(x) ((x) < 0 ? -(x) : (x))

#if defined(__DECC) && defined(__osf__)
#pragma message disable (mainparm) /* Perl uses the envp in main(). */
#endif

#define do_open(g, n, l, a, rm, rp, sf) \
        do_openn(g, n, l, a, rm, rp, sf, (SV **) NULL, 0)
#ifdef PERL_DEFAULT_DO_EXEC3_IMPLEMENTATION
#  define do_exec(cmd)			do_exec3(cmd,0,0)
#endif
#ifdef OS2
#  define do_aexec			Perl_do_aexec
#else
#  define do_aexec(really, mark,sp)	do_aexec5(really, mark, sp, 0, 0)
#endif


/*
=for apidoc_section $utility

=for apidoc Am|bool|IS_SAFE_SYSCALL|NN const char *pv|STRLEN len|NN const char *what|NN const char *op_name

Same as L</is_safe_syscall>.

=cut

Allows one ending \0
*/
#define IS_SAFE_SYSCALL(p, len, what, op_name) (Perl_is_safe_syscall(aTHX_ (p), (len), (what), (op_name)))

#define IS_SAFE_PATHNAME(p, len, op_name) IS_SAFE_SYSCALL((p), (len), "pathname", (op_name))

#if defined(OEMVS) || defined(__amigaos4__)
#define NO_ENV_ARRAY_IN_MAIN
#endif

/* These are used by Perl_pv_escape() and Perl_pv_pretty()
 * are here so that they are available throughout the core
 * NOTE that even though some are for _escape and some for _pretty
 * there must not be any clashes as the flags from _pretty are
 * passed straight through to _escape.
 */

#define PERL_PV_ESCAPE_QUOTE        0x000001
#define PERL_PV_PRETTY_QUOTE        PERL_PV_ESCAPE_QUOTE

#define PERL_PV_PRETTY_ELLIPSES     0x000002
#define PERL_PV_PRETTY_LTGT         0x000004
#define PERL_PV_PRETTY_EXACTSIZE    0x000008

#define PERL_PV_ESCAPE_UNI          0x000100
#define PERL_PV_ESCAPE_UNI_DETECT   0x000200
#define PERL_PV_ESCAPE_NONASCII     0x000400
#define PERL_PV_ESCAPE_FIRSTCHAR    0x000800

#define PERL_PV_ESCAPE_ALL          0x001000
#define PERL_PV_ESCAPE_NOBACKSLASH  0x002000
#define PERL_PV_ESCAPE_NOCLEAR      0x004000
#define PERL_PV_PRETTY_NOCLEAR      PERL_PV_ESCAPE_NOCLEAR
#define PERL_PV_ESCAPE_RE           0x008000

/* Escape PV with hex, except leave NULs as octal: */
#define PERL_PV_ESCAPE_DWIM         0x010000

/* Escape PV with all hex, including NUL. */
#define PERL_PV_ESCAPE_DWIM_ALL_HEX 0x020000

/* Do not escape word characters, alters meaning of other flags */
#define PERL_PV_ESCAPE_NON_WC       0x040000
#define PERL_PV_ESCAPE_TRUNC_MIDDLE 0x080000

#define PERL_PV_PRETTY_QUOTEDPREFIX (   \
        PERL_PV_PRETTY_ELLIPSES |       \
        PERL_PV_PRETTY_QUOTE    |       \
        PERL_PV_ESCAPE_NONASCII |       \
        PERL_PV_ESCAPE_NON_WC   |       \
        PERL_PV_ESCAPE_TRUNC_MIDDLE |   \
        0)


/* used by pv_display in dump.c*/
#define PERL_PV_PRETTY_DUMP  PERL_PV_PRETTY_ELLIPSES|PERL_PV_PRETTY_QUOTE
#define PERL_PV_PRETTY_REGPROP PERL_PV_PRETTY_ELLIPSES|PERL_PV_PRETTY_LTGT|PERL_PV_ESCAPE_RE|PERL_PV_ESCAPE_NONASCII

#if DOUBLEKIND == DOUBLE_IS_VAX_F_FLOAT || \
    DOUBLEKIND == DOUBLE_IS_VAX_D_FLOAT || \
    DOUBLEKIND == DOUBLE_IS_VAX_G_FLOAT
#  define DOUBLE_IS_VAX_FLOAT
#else
#  define DOUBLE_IS_IEEE_FORMAT
#endif

#if DOUBLEKIND == DOUBLE_IS_IEEE_754_32_BIT_LITTLE_ENDIAN || \
    DOUBLEKIND == DOUBLE_IS_IEEE_754_64_BIT_LITTLE_ENDIAN || \
    DOUBLEKIND == DOUBLE_IS_IEEE_754_128_BIT_LITTLE_ENDIAN
#  define DOUBLE_LITTLE_ENDIAN
#endif

#if DOUBLEKIND == DOUBLE_IS_IEEE_754_32_BIT_BIG_ENDIAN || \
    DOUBLEKIND == DOUBLE_IS_IEEE_754_64_BIT_BIG_ENDIAN || \
    DOUBLEKIND == DOUBLE_IS_IEEE_754_128_BIT_BIG_ENDIAN
#  define DOUBLE_BIG_ENDIAN
#endif

#if DOUBLEKIND == DOUBLE_IS_IEEE_754_64_BIT_MIXED_ENDIAN_LE_BE || \
    DOUBLEKIND == DOUBLE_IS_IEEE_754_64_BIT_MIXED_ENDIAN_BE_LE
#  define DOUBLE_MIX_ENDIAN
#endif

/* The VAX fp formats are neither consistently little-endian nor
 * big-endian, and neither are they really IEEE-mixed endian like
 * the mixed-endian ARM IEEE formats (with swapped bytes).
 * Ultimately, the VAX format came from the PDP-11.
 *
 * The ordering of the parts in VAX floats is quite vexing.
 * In the below the fraction_n are the mantissa bits.
 *
 * The fraction_1 is the most significant (numbering as by DEC/Digital),
 * while the rightmost bit in each fraction is the least significant:
 * in other words, big-endian bit order within the fractions.
 *
 * The fraction segments themselves would be big-endianly, except that
 * within 32 bit segments the less significant half comes first, the more
 * significant after, except that in the format H (used for long doubles)
 * the first fraction segment is alone, because the exponent is wider.
 * This means for example that both the most and the least significant
 * bits can be in the middle of the floats, not at either end.
 *
 * References:
 * http://nssdc.gsfc.nasa.gov/nssdc/formats/VAXFloatingPoint.htm
 * http://www.quadibloc.com/comp/cp0201.htm
 * http://h71000.www7.hp.com/doc/82final/6443/6443pro_028.html
 * (somebody at HP should be fired for the URLs)
 *
 * F   fraction_2:16 sign:1 exp:8  fraction_1:7
 *     (exponent bias 128, hidden first one-bit)
 *
 * D   fraction_2:16 sign:1 exp:8  fraction_1:7
 *     fraction_4:16               fraction_3:16
 *     (exponent bias 128, hidden first one-bit)
 *
 * G   fraction_2:16 sign:1 exp:11 fraction_1:4
 *     fraction_4:16               fraction_3:16
 *     (exponent bias 1024, hidden first one-bit)
 *
 * H   fraction_1:16 sign:1 exp:15
 *     fraction_3:16               fraction_2:16
 *     fraction_5:16               fraction_4:16
 *     fraction_7:16               fraction_6:16
 *     (exponent bias 16384, hidden first one-bit)
 *     (available only on VAX, and only on Fortran?)
 *
 * The formats S, T and X are available on the Alpha (and Itanium,
 * also known as I64/IA64) and are equivalent with the IEEE-754 formats
 * binary32, binary64, and binary128 (commonly: float, double, long double).
 *
 * S   sign:1 exp:8 mantissa:23
 *     (exponent bias 127, hidden first one-bit)
 *
 * T   sign:1 exp:11 mantissa:52
 *     (exponent bias 1022, hidden first one-bit)
 *
 * X   sign:1 exp:15 mantissa:112
 *     (exponent bias 16382, hidden first one-bit)
 *
 */

#ifdef DOUBLE_IS_VAX_FLOAT
#  define DOUBLE_VAX_ENDIAN
#endif

#ifdef DOUBLE_IS_IEEE_FORMAT
/* All the basic IEEE formats have the implicit bit,
 * except for the x86 80-bit extended formats, which will undef this.
 * Also note that the IEEE 754 subnormals (formerly known as denormals)
 * do not have the implicit bit of one. */
#  define NV_IMPLICIT_BIT
#endif

#if defined(LONG_DOUBLEKIND) && LONG_DOUBLEKIND != LONG_DOUBLE_IS_DOUBLE

#  if LONG_DOUBLEKIND == LONG_DOUBLE_IS_IEEE_754_128_BIT_LITTLE_ENDIAN || \
      LONG_DOUBLEKIND == LONG_DOUBLE_IS_X86_80_BIT_LITTLE_ENDIAN || \
      LONG_DOUBLEKIND == LONG_DOUBLE_IS_DOUBLEDOUBLE_128_BIT_LE_LE
#    define LONGDOUBLE_LITTLE_ENDIAN
#  endif

#  if LONG_DOUBLEKIND == LONG_DOUBLE_IS_IEEE_754_128_BIT_BIG_ENDIAN || \
      LONG_DOUBLEKIND == LONG_DOUBLE_IS_X86_80_BIT_BIG_ENDIAN || \
      LONG_DOUBLEKIND == LONG_DOUBLE_IS_DOUBLEDOUBLE_128_BIT_BE_BE
#    define LONGDOUBLE_BIG_ENDIAN
#  endif

#  if LONG_DOUBLEKIND == LONG_DOUBLE_IS_DOUBLEDOUBLE_128_BIT_LE_BE || \
      LONG_DOUBLEKIND == LONG_DOUBLE_IS_DOUBLEDOUBLE_128_BIT_BE_LE
#    define LONGDOUBLE_MIX_ENDIAN
#  endif

#  if LONG_DOUBLEKIND == LONG_DOUBLE_IS_X86_80_BIT_LITTLE_ENDIAN || \
      LONG_DOUBLEKIND == LONG_DOUBLE_IS_X86_80_BIT_BIG_ENDIAN
#    define LONGDOUBLE_X86_80_BIT
#    ifdef USE_LONG_DOUBLE
#      undef NV_IMPLICIT_BIT
#      define NV_X86_80_BIT
#    endif
#  endif

#  if LONG_DOUBLEKIND == LONG_DOUBLE_IS_DOUBLEDOUBLE_128_BIT_LE_LE || \
      LONG_DOUBLEKIND == LONG_DOUBLE_IS_DOUBLEDOUBLE_128_BIT_BE_BE || \
      LONG_DOUBLEKIND == LONG_DOUBLE_IS_DOUBLEDOUBLE_128_BIT_LE_BE || \
      LONG_DOUBLEKIND == LONG_DOUBLE_IS_DOUBLEDOUBLE_128_BIT_BE_LE
#    define LONGDOUBLE_DOUBLEDOUBLE
#  endif

#  if LONG_DOUBLEKIND == LONG_DOUBLE_IS_VAX_H_FLOAT
#    define LONGDOUBLE_VAX_ENDIAN
#  endif

#endif /* LONG_DOUBLEKIND */

#ifdef USE_QUADMATH /* assume quadmath endianness == native double endianness */
#  if defined(DOUBLE_LITTLE_ENDIAN)
#    define NV_LITTLE_ENDIAN
#  elif defined(DOUBLE_BIG_ENDIAN)
#    define NV_BIG_ENDIAN
#  elif defined(DOUBLE_MIX_ENDIAN) /* stretch */
#    define NV_MIX_ENDIAN
#  endif
#elif NVSIZE == DOUBLESIZE
#  ifdef DOUBLE_LITTLE_ENDIAN
#    define NV_LITTLE_ENDIAN
#  endif
#  ifdef DOUBLE_BIG_ENDIAN
#    define NV_BIG_ENDIAN
#  endif
#  ifdef DOUBLE_MIX_ENDIAN
#    define NV_MIX_ENDIAN
#  endif
#  ifdef DOUBLE_VAX_ENDIAN
#    define NV_VAX_ENDIAN
#  endif
#elif NVSIZE == LONG_DOUBLESIZE
#  ifdef LONGDOUBLE_LITTLE_ENDIAN
#    define NV_LITTLE_ENDIAN
#  endif
#  ifdef LONGDOUBLE_BIG_ENDIAN
#    define NV_BIG_ENDIAN
#  endif
#  ifdef LONGDOUBLE_MIX_ENDIAN
#    define NV_MIX_ENDIAN
#  endif
#  ifdef LONGDOUBLE_VAX_ENDIAN
#    define NV_VAX_ENDIAN
#  endif
#endif

/* We have somehow managed not to define the denormal/subnormal
 * detection.
 *
 * This may happen if the compiler doesn't expose the C99 math like
 * the fpclassify() without some special switches.  Perl tries to
 * stay C89, so for example -std=c99 is not an option.
 *
 * The Perl_isinf() and Perl_isnan() should have been defined even if
 * the C99 isinf() and isnan() are unavailable, and the NV_MIN becomes
 * from the C89 DBL_MIN or moral equivalent. */
#if !defined(Perl_fp_class_denorm) && defined(Perl_isinf) && defined(Perl_isnan) && defined(NV_MIN)
#  define Perl_fp_class_denorm(x) ((x) != 0.0 && !Perl_isinf(x) && !Perl_isnan(x) && PERL_ABS(x) < NV_MIN)
#endif

/* This is not a great fallback: subnormals tests will fail,
 * but at least Perl will link and 99.999% of tests will work. */
#if !defined(Perl_fp_class_denorm)
#  define Perl_fp_class_denorm(x) FALSE
#endif

#ifdef DOUBLE_IS_IEEE_FORMAT
#  define DOUBLE_HAS_INF
#  define DOUBLE_HAS_NAN
#endif

#ifdef DOUBLE_HAS_NAN

START_EXTERN_C

#ifdef DOINIT

/* PL_inf and PL_nan initialization.
 *
 * For inf and nan initialization the ultimate fallback is dividing
 * one or zero by zero: however, some compilers will warn or even fail
 * on divide-by-zero, but hopefully something earlier will work.
 *
 * If you are thinking of using HUGE_VAL for infinity, or using
 * <math.h> functions to generate NV_INF (e.g. exp(1e9), log(-1.0)),
 * stop.  Neither will work portably: HUGE_VAL can be just DBL_MAX,
 * and the math functions might be just generating DBL_MAX, or even zero.
 *
 * Also, do NOT try doing NV_NAN based on NV_INF and trying (NV_INF-NV_INF).
 * Though logically correct, some compilers (like Visual C 2003)
 * falsely misoptimize that to zero (x-x is always zero, right?)
 *
 * Finally, note that not all floating point formats define Inf (or NaN).
 * For the infinity a large number may be used instead.  Operations that
 * under the IEEE floating point would return Inf or NaN may return
 * either large numbers (positive or negative), or they may cause
 * a floating point exception or some other fault.
 */

/* The quadmath literals are anon structs which -Wc++-compat doesn't like. */
#  ifndef USE_CPLUSPLUS
GCC_DIAG_IGNORE_DECL(-Wc++-compat);
#  endif

#  ifdef USE_QUADMATH
/* Cannot use HUGE_VALQ for PL_inf because not a compile-time
 * constant. */
INFNAN_NV_U8_DECL PL_inf = { 1.0Q/0.0Q };
#  elif NVSIZE == LONG_DOUBLESIZE && defined(LONGDBLINFBYTES)
INFNAN_U8_NV_DECL PL_inf = { { LONGDBLINFBYTES } };
#  elif NVSIZE == DOUBLESIZE && defined(DOUBLEINFBYTES)
INFNAN_U8_NV_DECL PL_inf = { { DOUBLEINFBYTES } };
#  else
#    if NVSIZE == LONG_DOUBLESIZE && defined(USE_LONG_DOUBLE)
#      if defined(LDBL_INFINITY)
INFNAN_NV_U8_DECL PL_inf = { LDBL_INFINITY };
#      elif defined(LDBL_INF)
INFNAN_NV_U8_DECL PL_inf = { LDBL_INF };
#      elif defined(INFINITY)
INFNAN_NV_U8_DECL PL_inf = { (NV)INFINITY };
#      elif defined(INF)
INFNAN_NV_U8_DECL PL_inf = { (NV)INF };
#      else
INFNAN_NV_U8_DECL PL_inf = { 1.0L/0.0L }; /* keep last */
#      endif
#    else
#      if defined(DBL_INFINITY)
INFNAN_NV_U8_DECL PL_inf = { DBL_INFINITY };
#      elif defined(DBL_INF)
INFNAN_NV_U8_DECL PL_inf = { DBL_INF };
#      elif defined(INFINITY) /* C99 */
INFNAN_NV_U8_DECL PL_inf = { (NV)INFINITY };
#      elif defined(INF)
INFNAN_NV_U8_DECL PL_inf = { (NV)INF };
#      else
INFNAN_NV_U8_DECL PL_inf = { 1.0/0.0 }; /* keep last */
#      endif
#    endif
#  endif

#  ifdef USE_QUADMATH
/* Cannot use nanq("0") for PL_nan because not a compile-time
 * constant. */
INFNAN_NV_U8_DECL PL_nan = { 0.0Q/0.0Q };
#  elif NVSIZE == LONG_DOUBLESIZE && defined(LONGDBLNANBYTES)
INFNAN_U8_NV_DECL PL_nan = { { LONGDBLNANBYTES } };
#  elif NVSIZE == DOUBLESIZE && defined(DOUBLENANBYTES)
INFNAN_U8_NV_DECL PL_nan = { { DOUBLENANBYTES } };
#  else
#    if NVSIZE == LONG_DOUBLESIZE && defined(USE_LONG_DOUBLE)
#      if defined(LDBL_NAN)
INFNAN_NV_U8_DECL PL_nan = { LDBL_NAN };
#      elif defined(LDBL_QNAN)
INFNAN_NV_U8_DECL PL_nan = { LDBL_QNAN };
#      elif defined(NAN)
INFNAN_NV_U8_DECL PL_nan = { (NV)NAN };
#      else
INFNAN_NV_U8_DECL PL_nan = { 0.0L/0.0L }; /* keep last */
#      endif
#    else
#      if defined(DBL_NAN)
INFNAN_NV_U8_DECL PL_nan = { DBL_NAN };
#      elif defined(DBL_QNAN)
INFNAN_NV_U8_DECL PL_nan = { DBL_QNAN };
#      elif defined(NAN) /* C99 */
INFNAN_NV_U8_DECL PL_nan = { (NV)NAN };
#      else
INFNAN_NV_U8_DECL PL_nan = { 0.0/0.0 }; /* keep last */
#      endif
#    endif
#  endif

#  ifndef USE_CPLUSPLUS
GCC_DIAG_RESTORE_DECL;
#  endif

#else

/* The declarations here need to match the initializations done above,
   since a mismatch across compilation units causes undefined
   behavior.  It also prevents warnings from LTO builds.
*/
#  if !defined(USE_QUADMATH) && \
       (NVSIZE == LONG_DOUBLESIZE && defined(LONGDBLINFBYTES) ||   \
        NVSIZE == DOUBLESIZE && defined(DOUBLEINFBYTES))
INFNAN_U8_NV_DECL PL_inf;
#  else
INFNAN_NV_U8_DECL PL_inf;
#  endif

#  if !defined(USE_QUADMATH) && \
       (NVSIZE == LONG_DOUBLESIZE && defined(LONGDBLNANBYTES) ||   \
        NVSIZE == DOUBLESIZE && defined(DOUBLENANBYTES))
INFNAN_U8_NV_DECL PL_nan;
#  else
INFNAN_NV_U8_DECL PL_nan;
#  endif

#endif

END_EXTERN_C

/* If you have not defined NV_INF/NV_NAN (like for example win32/win32.h),
 * we will define NV_INF/NV_NAN as the nv part of the global const
 * PL_inf/PL_nan.  Note, however, that the preexisting NV_INF/NV_NAN
 * might not be a compile-time constant, in which case it cannot be
 * used to initialize PL_inf/PL_nan above. */
#ifndef NV_INF
#  define NV_INF PL_inf.nv
#endif
#ifndef NV_NAN
#  define NV_NAN PL_nan.nv
#endif

/* NaNs (not-a-numbers) can carry payload bits, in addition to
 * "nan-ness".  Part of the payload is the quiet/signaling bit.
 * To back up a bit (harhar):
 *
 * For IEEE 754 64-bit formats [1]:
 *
 * s 000 (mantissa all-zero)  zero
 * s 000 (mantissa non-zero)  subnormals (denormals)
 * s 001 ... 7fe              normals
 * s 7ff q                    nan
 *
 * For IEEE 754 128-bit formats:
 *
 * s 0000 (mantissa all-zero)  zero
 * s 0000 (mantissa non-zero)  subnormals (denormals)
 * s 0001 ... 7ffe             normals
 * s 7fff q                    nan
 *
 * [1] this looks like big-endian, but applies equally to little-endian.
 *
 * s = Sign bit.  Yes, zeros and nans can have negative sign,
 *     the interpretation is application-specific.
 *
 * q = Quietness bit, the interpretation is platform-specific.
 *     Most platforms have the most significant bit being one
 *     meaning quiet, but some (older mips, hppa) have the msb
 *     being one meaning signaling.  Note that the above means
 *     that on most platforms there cannot be signaling nan with
 *     zero payload because that is identical with infinity;
 *     while conversely on older mips/hppa there cannot be a quiet nan
 *     because that is identical with infinity.
 *
 *     Moreover, whether there is any behavioral difference
 *     between quiet and signaling NaNs, depends on the platform.
 *
 * x86 80-bit extended precision is different, the mantissa bits:
 *
 * 63 62 61   30387+    pre-387    visual c
 * --------   ----      --------   --------
 *  0  0  0   invalid   infinity
 *  0  0  1   invalid   snan
 *  0  1  0   invalid   snan
 *  0  1  1   invalid   snan
 *  1  0  0   infinity  snan        1.#INF
 *  1  0  1   snan                  1.#SNAN
 *  1  1  0   qnan                 -1.#IND  (x86 chooses this to negative)
 *  1  1  1   qnan                  1.#QNAN
 *
 * This means that in this format there are 61 bits available
 * for the nan payload.
 *
 * Note that the 32-bit x86 ABI cannot do signaling nans: the x87
 * simply cannot preserve the bit.  You can either use the 80-bit
 * extended precision (long double, -Duselongdouble), or use x86-64.
 *
 * In all platforms, the payload bytes (and bits, some of them are
 * often in a partial byte) themselves can be either all zero (x86),
 * all one (sparc or mips), or a mixture: in IEEE 754 128-bit double
 * or in a double-double, the first half of the payload can follow the
 * native double, while in the second half the payload can be all
 * zeros.  (Therefore the mask for payload bits is not necessarily
 * identical to bit complement of the NaN.)  Another way of putting
 * this: the payload for the default NaN might not be zero.
 *
 * For the x86 80-bit long doubles, the trailing bytes (the 80 bits
 * being 'packaged' in either 12 or 16 bytes) can be whatever random
 * garbage.
 *
 * Furthermore, the semantics of the sign bit on NaNs are platform-specific.
 * On normal floats, the sign bit being on means negative.  But this may,
 * or may not, be reverted on NaNs: in other words, the default NaN might
 * have the sign bit on, and therefore look like negative if you look
 * at it at the bit level.
 *
 * NaN payloads are not propagated even on copies, or in arithmetics.
 * They *might* be, according to some rules, on your particular
 * cpu/os/compiler/libraries, but no guarantees.
 *
 * To summarize, on most platforms, and for 64-bit doubles
 * (using big-endian ordering here):
 *
 * [7FF8000000000000..7FFFFFFFFFFFFFFF] quiet
 * [FFF8000000000000..FFFFFFFFFFFFFFFF] quiet
 * [7FF0000000000001..7FF7FFFFFFFFFFFF] signaling
 * [FFF0000000000001..FFF7FFFFFFFFFFFF] signaling
 *
 * The C99 nan() is supposed to generate *quiet* NaNs.
 *
 * Note the asymmetry:
 * The 7FF0000000000000 is positive infinity,
 * the FFF0000000000000 is negative infinity.
 */

/* NVMANTBITS is the number of _real_ mantissa bits in an NV.
 * For the standard IEEE 754 fp this number is usually one less that
 * *DBL_MANT_DIG because of the implicit (aka hidden) bit, which isn't
 * real.  For the 80-bit extended precision formats (x86*), the number
 * of mantissa bits... depends. For normal floats, it's 64.  But for
 * the inf/nan, it's different (zero for inf, 61 for nan).
 * NVMANTBITS works for normal floats. */

/* We do not want to include the quiet/signaling bit. */
#define NV_NAN_BITS (NVMANTBITS - 1)

#if defined(USE_LONG_DOUBLE) && NVSIZE > DOUBLESIZE
#  if LONG_DOUBLEKIND == LONG_DOUBLE_IS_IEEE_754_128_BIT_LITTLE_ENDIAN
#    define NV_NAN_QS_BYTE_OFFSET 13
#  elif LONG_DOUBLEKIND == LONG_DOUBLE_IS_IEEE_754_128_BIT_BIG_ENDIAN
#    define NV_NAN_QS_BYTE_OFFSET 2
#  elif LONG_DOUBLEKIND == LONG_DOUBLE_IS_X86_80_BIT_LITTLE_ENDIAN
#    define NV_NAN_QS_BYTE_OFFSET 7
#  elif LONG_DOUBLEKIND == LONG_DOUBLE_IS_X86_80_BIT_BIG_ENDIAN
#    define NV_NAN_QS_BYTE_OFFSET 2
#  elif LONG_DOUBLEKIND == LONG_DOUBLE_IS_DOUBLEDOUBLE_128_BIT_LE_LE
#    define NV_NAN_QS_BYTE_OFFSET 13
#  elif LONG_DOUBLEKIND == LONG_DOUBLE_IS_DOUBLEDOUBLE_128_BIT_BE_BE
#    define NV_NAN_QS_BYTE_OFFSET 1
#  elif LONG_DOUBLEKIND == LONG_DOUBLE_IS_DOUBLEDOUBLE_128_BIT_LE_BE
#    define NV_NAN_QS_BYTE_OFFSET 9
#  elif LONG_DOUBLEKIND == LONG_DOUBLE_IS_DOUBLEDOUBLE_128_BIT_BE_LE
#    define NV_NAN_QS_BYTE_OFFSET 6
#  else
#    error "Unexpected long double format"
#  endif
#else
#  ifdef USE_QUADMATH
#    ifdef NV_LITTLE_ENDIAN
#      define NV_NAN_QS_BYTE_OFFSET 13
#    elif defined(NV_BIG_ENDIAN)
#      define NV_NAN_QS_BYTE_OFFSET 2
#    else
#      error "Unexpected quadmath format"
#    endif
#  elif DOUBLEKIND == DOUBLE_IS_IEEE_754_32_BIT_LITTLE_ENDIAN
#    define NV_NAN_QS_BYTE_OFFSET 2
#  elif DOUBLEKIND == DOUBLE_IS_IEEE_754_32_BIT_BIG_ENDIAN
#    define NV_NAN_QS_BYTE_OFFSET 1
#  elif DOUBLEKIND == DOUBLE_IS_IEEE_754_64_BIT_LITTLE_ENDIAN
#    define NV_NAN_QS_BYTE_OFFSET 6
#  elif DOUBLEKIND == DOUBLE_IS_IEEE_754_64_BIT_BIG_ENDIAN
#    define NV_NAN_QS_BYTE_OFFSET 1
#  elif DOUBLEKIND == DOUBLE_IS_IEEE_754_128_BIT_LITTLE_ENDIAN
#    define NV_NAN_QS_BYTE_OFFSET 13
#  elif DOUBLEKIND == DOUBLE_IS_IEEE_754_128_BIT_BIG_ENDIAN
#    define NV_NAN_QS_BYTE_OFFSET 2
#  elif DOUBLEKIND == DOUBLE_IS_IEEE_754_64_BIT_MIXED_ENDIAN_LE_BE
#    define NV_NAN_QS_BYTE_OFFSET 2 /* bytes 4 5 6 7 0 1 2 3 (MSB 7) */
#  elif DOUBLEKIND == DOUBLE_IS_IEEE_754_64_BIT_MIXED_ENDIAN_BE_LE
#    define NV_NAN_QS_BYTE_OFFSET 5 /* bytes 3 2 1 0 7 6 5 4 (MSB 7) */
#  else
/* For example the VAX formats should never
 * get here because they do not have NaN. */
#    error "Unexpected double format"
#  endif
#endif
/* NV_NAN_QS_BYTE is the byte to test for the quiet/signaling */
#define NV_NAN_QS_BYTE(nvp) (((U8*)(nvp))[NV_NAN_QS_BYTE_OFFSET])
/* NV_NAN_QS_BIT is the bit to test in the NV_NAN_QS_BYTE_OFFSET
 * for the quiet/signaling */
#if defined(USE_LONG_DOUBLE) && \
  (LONG_DOUBLEKIND == LONG_DOUBLE_IS_X86_80_BIT_LITTLE_ENDIAN || \
   LONG_DOUBLEKIND == LONG_DOUBLE_IS_X86_80_BIT_BIG_ENDIAN)
#  define NV_NAN_QS_BIT_SHIFT 6 /* 0x40 */
#elif defined(USE_LONG_DOUBLE) && \
  (LONG_DOUBLEKIND == LONG_DOUBLE_IS_DOUBLEDOUBLE_128_BIT_LE_LE || \
   LONG_DOUBLEKIND == LONG_DOUBLE_IS_DOUBLEDOUBLE_128_BIT_BE_BE || \
   LONG_DOUBLEKIND == LONG_DOUBLE_IS_DOUBLEDOUBLE_128_BIT_LE_BE || \
   LONG_DOUBLEKIND == LONG_DOUBLE_IS_DOUBLEDOUBLE_128_BIT_BE_LE)
#  define NV_NAN_QS_BIT_SHIFT 3 /* 0x08, but not via NV_NAN_BITS */
#else
#  define NV_NAN_QS_BIT_SHIFT ((NV_NAN_BITS) % 8) /* usually 3, or 0x08 */
#endif
#define NV_NAN_QS_BIT (1 << (NV_NAN_QS_BIT_SHIFT))
/* NV_NAN_QS_BIT_OFFSET is the bit offset from the beginning of a NV
 * (bytes ordered big-endianly) for the quiet/signaling bit
 * for the quiet/signaling */
#define NV_NAN_QS_BIT_OFFSET \
    (8 * (NV_NAN_QS_BYTE_OFFSET) + (NV_NAN_QS_BIT_SHIFT))
/* NV_NAN_QS_QUIET (always defined) is true if the NV_NAN_QS_QS_BIT being
 * on indicates quiet NaN.  NV_NAN_QS_SIGNALING (also always defined)
 * is true if the NV_NAN_QS_BIT being on indicates signaling NaN. */
#define NV_NAN_QS_QUIET \
    ((NV_NAN_QS_BYTE(PL_nan.u8) & NV_NAN_QS_BIT) == NV_NAN_QS_BIT)
#define NV_NAN_QS_SIGNALING (!(NV_NAN_QS_QUIET))
#define NV_NAN_QS_TEST(nvp) (NV_NAN_QS_BYTE(nvp) & NV_NAN_QS_BIT)
/* NV_NAN_IS_QUIET() returns true if the NV behind nvp is a NaN,
 * whether it is a quiet NaN, NV_NAN_IS_SIGNALING() if a signaling NaN.
 * Note however that these do not check whether the nvp is a NaN. */
#define NV_NAN_IS_QUIET(nvp) \
    (NV_NAN_QS_TEST(nvp) == (NV_NAN_QS_QUIET ? NV_NAN_QS_BIT : 0))
#define NV_NAN_IS_SIGNALING(nvp) \
    (NV_NAN_QS_TEST(nvp) == (NV_NAN_QS_QUIET ? 0 : NV_NAN_QS_BIT))
#define NV_NAN_SET_QUIET(nvp) \
    (NV_NAN_QS_QUIET ? \
     (NV_NAN_QS_BYTE(nvp) |= NV_NAN_QS_BIT) : \
     (NV_NAN_QS_BYTE(nvp) &= ~NV_NAN_QS_BIT))
#define NV_NAN_SET_SIGNALING(nvp) \
    (NV_NAN_QS_QUIET ? \
     (NV_NAN_QS_BYTE(nvp) &= ~NV_NAN_QS_BIT) : \
     (NV_NAN_QS_BYTE(nvp) |= NV_NAN_QS_BIT))
#define NV_NAN_QS_XOR(nvp) (NV_NAN_QS_BYTE(nvp) ^= NV_NAN_QS_BIT)

/* NV_NAN_PAYLOAD_MASK: masking the nan payload bits.
 *
 * NV_NAN_PAYLOAD_PERM: permuting the nan payload bytes.
 * 0xFF means "don't go here".*/

/* Shorthands to avoid typoses. */
#define NV_NAN_PAYLOAD_MASK_SKIP_EIGHT \
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0
#define NV_NAN_PAYLOAD_PERM_SKIP_EIGHT \
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
#define NV_NAN_PAYLOAD_PERM_0_TO_7 \
  0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7
#define NV_NAN_PAYLOAD_PERM_7_TO_0 \
  0x7, 0x6, 0x5, 0x4, 0x3, 0x2, 0x1, 0x0
#define NV_NAN_PAYLOAD_MASK_IEEE_754_128_LE \
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, \
  0xff, 0xff, 0xff, 0xff, 0xff, 0x7f, 0x00, 0x00
#define NV_NAN_PAYLOAD_PERM_IEEE_754_128_LE \
  NV_NAN_PAYLOAD_PERM_0_TO_7, \
  0x8, 0x9, 0xa, 0xb, 0xc, 0xd, 0xFF, 0xFF
#define NV_NAN_PAYLOAD_MASK_IEEE_754_128_BE \
  0x00, 0x00, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, \
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
#define NV_NAN_PAYLOAD_PERM_IEEE_754_128_BE \
  0xFF, 0xFF, 0xd, 0xc, 0xb, 0xa, 0x9, 0x8, \
  NV_NAN_PAYLOAD_PERM_7_TO_0
#define NV_NAN_PAYLOAD_MASK_IEEE_754_64_LE \
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x07, 0x00
#define NV_NAN_PAYLOAD_PERM_IEEE_754_64_LE \
  0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0xFF
#define NV_NAN_PAYLOAD_MASK_IEEE_754_64_BE \
  0x00, 0x07, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
#define NV_NAN_PAYLOAD_PERM_IEEE_754_64_BE \
  0xFF, 0x6, 0x5, 0x4, 0x3, 0x2, 0x1, 0x0

#if defined(USE_LONG_DOUBLE) && NVSIZE > DOUBLESIZE
#  if LONG_DOUBLEKIND == LONG_DOUBLE_IS_IEEE_754_128_BIT_LITTLE_ENDIAN
#    define NV_NAN_PAYLOAD_MASK NV_NAN_PAYLOAD_MASK_IEEE_754_128_LE
#    define NV_NAN_PAYLOAD_PERM NV_NAN_PAYLOAD_PERM_IEEE_754_128_LE
#  elif LONG_DOUBLEKIND == LONG_DOUBLE_IS_IEEE_754_128_BIT_BIG_ENDIAN
#    define NV_NAN_PAYLOAD_MASK NV_NAN_PAYLOAD_MASK_IEEE_754_128_BE
#    define NV_NAN_PAYLOAD_PERM NV_NAN_PAYLOAD_PERM_IEEE_754_128_BE
#  elif LONG_DOUBLEKIND == LONG_DOUBLE_IS_X86_80_BIT_LITTLE_ENDIAN
#    if LONG_DOUBLESIZE == 10
#      define NV_NAN_PAYLOAD_MASK \
         0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x1f, \
         0x00, 0x00
#      define NV_NAN_PAYLOAD_PERM \
         NV_NAN_PAYLOAD_PERM_0_TO_7, 0xFF, 0xFF
#    elif LONG_DOUBLESIZE == 12
#      define NV_NAN_PAYLOAD_MASK \
         0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x1f, \
         0x00, 0x00, 0x00, 0x00
#      define NV_NAN_PAYLOAD_PERM \
         NV_NAN_PAYLOAD_PERM_0_TO_7, 0xFF, 0xFF, 0xFF, 0xFF
#    elif LONG_DOUBLESIZE == 16
#      define NV_NAN_PAYLOAD_MASK \
         0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x1f, \
         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
#      define NV_NAN_PAYLOAD_PERM \
         NV_NAN_PAYLOAD_PERM_0_TO_7, \
         0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
#    else
#      error "Unexpected x86 80-bit little-endian long double format"
#    endif
#  elif LONG_DOUBLEKIND == LONG_DOUBLE_IS_X86_80_BIT_BIG_ENDIAN
#    if LONG_DOUBLESIZE == 10
#      define NV_NAN_PAYLOAD_MASK \
         0x00, 0x00, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xff, \
         0xff, 0xff
#      define NV_NAN_PAYLOAD_PERM \
         NV_NAN_PAYLOAD_PERM_7_TO_0, 0xFF, 0xFF
#    elif LONG_DOUBLESIZE == 12
#      define NV_NAN_PAYLOAD_MASK \
         0x00, 0x00, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xff, \
         0xff, 0xff, 0x00, 0x00
#      define NV_NAN_PAYLOAD_PERM \
         NV_NAN_PAYLOAD_PERM_7_TO_0, 0xFF, 0xFF, 0xFF, 0xFF
#    elif LONG_DOUBLESIZE == 16
#      define NV_NAN_PAYLOAD_MASK \
         0x00, 0x00, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xff, \
         0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
#      define NV_NAN_PAYLOAD_PERM \
         NV_NAN_PAYLOAD_PERM_7_TO_0, \
         0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
#    else
#      error "Unexpected x86 80-bit big-endian long double format"
#    endif
#  elif LONG_DOUBLEKIND == LONG_DOUBLE_IS_DOUBLEDOUBLE_128_BIT_LE_LE
/* For double-double we assume only the first double (in LE or BE terms)
 * is used for NaN. */
#    define NV_NAN_PAYLOAD_MASK \
       NV_NAN_PAYLOAD_MASK_SKIP_EIGHT, NV_NAN_PAYLOAD_MASK_IEEE_754_64_LE
#    define NV_NAN_PAYLOAD_PERM \
       NV_NAN_PAYLOAD_PERM_SKIP_EIGHT, NV_NAN_PAYLOAD_PERM_IEEE_754_64_LE
#  elif LONG_DOUBLEKIND == LONG_DOUBLE_IS_DOUBLEDOUBLE_128_BIT_BE_BE
#    define NV_NAN_PAYLOAD_MASK \
       NV_NAN_PAYLOAD_MASK_IEEE_754_64_BE
#    define NV_NAN_PAYLOAD_PERM \
       NV_NAN_PAYLOAD_PERM_IEEE_754_64_BE
#  elif LONG_DOUBLEKIND == LONG_DOUBLE_IS_DOUBLEDOUBLE_128_BIT_LE_BE
#    define NV_NAN_PAYLOAD_MASK \
       NV_NAN_PAYLOAD_MASK_IEEE_754_64_LE
#    define NV_NAN_PAYLOAD_PERM \
       NV_NAN_PAYLOAD_PERM_IEEE_754_64_LE
#  elif LONG_DOUBLEKIND == LONG_DOUBLE_IS_DOUBLEDOUBLE_128_BIT_BE_LE
#    define NV_NAN_PAYLOAD_MASK \
       NV_NAN_PAYLOAD_MASK_SKIP_EIGHT, NV_NAN_PAYLOAD_MASK_IEEE_754_64_BE
#    define NV_NAN_PAYLOAD_PERM \
       NV_NAN_PAYLOAD_PERM_SKIP_EIGHT, NV_NAN_PAYLOAD_PERM_IEEE_754_64_BE
#  else
#    error "Unexpected long double format"
#  endif
#else
#  ifdef USE_QUADMATH /* quadmath is not long double */
#    ifdef NV_LITTLE_ENDIAN
#      define NV_NAN_PAYLOAD_MASK NV_NAN_PAYLOAD_MASK_IEEE_754_128_LE
#      define NV_NAN_PAYLOAD_PERM NV_NAN_PAYLOAD_PERM_IEEE_754_128_LE
#    elif defined(NV_BIG_ENDIAN)
#      define NV_NAN_PAYLOAD_MASK NV_NAN_PAYLOAD_MASK_IEEE_754_128_BE
#      define NV_NAN_PAYLOAD_PERM NV_NAN_PAYLOAD_PERM_IEEE_754_128_BE
#    else
#      error "Unexpected quadmath format"
#    endif
#  elif DOUBLEKIND == DOUBLE_IS_IEEE_754_32_BIT_LITTLE_ENDIAN
#    define NV_NAN_PAYLOAD_MASK 0xff, 0xff, 0x07, 0x00
#    define NV_NAN_PAYLOAD_PERM 0x0, 0x1, 0x2, 0xFF
#  elif DOUBLEKIND == DOUBLE_IS_IEEE_754_32_BIT_BIG_ENDIAN
#    define NV_NAN_PAYLOAD_MASK 0x00, 0x07, 0xff, 0xff
#    define NV_NAN_PAYLOAD_PERM 0xFF, 0x2, 0x1, 0x0
#  elif DOUBLEKIND == DOUBLE_IS_IEEE_754_64_BIT_LITTLE_ENDIAN
#    define NV_NAN_PAYLOAD_MASK NV_NAN_PAYLOAD_MASK_IEEE_754_64_LE
#    define NV_NAN_PAYLOAD_PERM NV_NAN_PAYLOAD_PERM_IEEE_754_64_LE
#  elif DOUBLEKIND == DOUBLE_IS_IEEE_754_64_BIT_BIG_ENDIAN
#    define NV_NAN_PAYLOAD_MASK NV_NAN_PAYLOAD_MASK_IEEE_754_64_BE
#    define NV_NAN_PAYLOAD_PERM NV_NAN_PAYLOAD_PERM_IEEE_754_64_BE
#  elif DOUBLEKIND == DOUBLE_IS_IEEE_754_128_BIT_LITTLE_ENDIAN
#    define NV_NAN_PAYLOAD_MASK NV_NAN_PAYLOAD_MASK_IEEE_754_128_LE
#    define NV_NAN_PAYLOAD_PERM NV_NAN_PAYLOAD_PERM_IEEE_754_128_LE
#  elif DOUBLEKIND == DOUBLE_IS_IEEE_754_128_BIT_BIG_ENDIAN
#    define NV_NAN_PAYLOAD_MASK NV_NAN_PAYLOAD_MASK_IEEE_754_128_BE
#    define NV_NAN_PAYLOAD_PERM NV_NAN_PAYLOAD_PERM_IEEE_754_128_BE
#  elif DOUBLEKIND == DOUBLE_IS_IEEE_754_64_BIT_MIXED_ENDIAN_LE_BE
#    define NV_NAN_PAYLOAD_MASK 0xff, 0xff, 0x07, 0x00, 0xff, 0xff, 0xff, 0xff
#    define NV_NAN_PAYLOAD_PERM 0x4, 0x5, 0x6, 0xFF, 0x0, 0x1, 0x2, 0x3
#  elif DOUBLEKIND == DOUBLE_IS_IEEE_754_64_BIT_MIXED_ENDIAN_BE_LE
#    define NV_NAN_PAYLOAD_MASK 0xff, 0xff, 0xff, 0xff, 0x00, 0x07, 0xff, 0xff
#    define NV_NAN_PAYLOAD_PERM 0x3, 0x2, 0x1, 0x0, 0xFF, 0x6, 0x5, 0x4
#  else
#    error "Unexpected double format"
#  endif
#endif

#endif /* DOUBLE_HAS_NAN */

/* these are used to faciliate the env var PERL_RAND_SEED,
 * which allows consistent behavior from code that calls
 * srand() with no arguments, either explicitly or implicitly.
 */
#define PERL_SRAND_OVERRIDE_NEXT() PERL_XORSHIFT32_A(PL_srand_override_next);

#define PERL_SRAND_OVERRIDE_NEXT_INIT() STMT_START {    \
    PL_srand_override = PL_srand_override_next;         \
    PERL_SRAND_OVERRIDE_NEXT();                         \
} STMT_END

#define PERL_SRAND_OVERRIDE_GET(into) STMT_START {      \
    into= PL_srand_override;                            \
    PERL_SRAND_OVERRIDE_NEXT_INIT();                    \
} STMT_END

#define PERL_SRAND_OVERRIDE_NEXT_CHILD() STMT_START {   \
    PERL_XORSHIFT32_B(PL_srand_override_next);          \
    PERL_SRAND_OVERRIDE_NEXT_INIT();                    \
} STMT_END

#define PERL_SRAND_OVERRIDE_NEXT_PARENT() \
    PERL_SRAND_OVERRIDE_NEXT()

/* in something like
 *
 * perl -le'sub f { eval "BEGIN{ f() }" }'
 *
 * Each iteration chews up 8 stacks frames, and we will eventually SEGV
 * due to C stack overflow.
 *
 * This define provides a maximum limit to prevent the SEGV. Such code is
 * unusual, so it unlikely we need a very large number here.
 */
#ifndef PERL_MAX_NESTED_EVAL_BEGIN_BLOCKS_DEFAULT
#define PERL_MAX_NESTED_EVAL_BEGIN_BLOCKS_DEFAULT 1000
#endif
/* ${^MAX_NESTED_EVAL_BEGIN_BLOCKS} */
#define PERL_VAR_MAX_NESTED_EVAL_BEGIN_BLOCKS "\015AX_NESTED_EVAL_BEGIN_BLOCKS"

/* Defines like this make it easier to do porting/diag.t. They are no-
 * ops that return their argument which can be used to hint to diag.t
 * that a string is actually an error message. By putting the category
 * information into the macro name it considerably simplifies extended
 * diag.t to support these cases. Feel free to add more.
 *
 * While it seems tempting to try to convert all of our diagnostics to
 * this format, it would miss part of the point of diag.t in that it
 * detects NEW diagnostics, which would not necessarily use these
 * macros. The macros instead exist where we know we have an error
 * message that isnt being picked up by diag.t because it is declared
 * as a string independently of the function it is fed to, something
 * diag.t can never handle right without help.
 */
#define PERL_DIAG_STR_(x)           ("" x "")
#define PERL_DIAG_WARN_SYNTAX(x)    PERL_DIAG_STR_(x)
#define PERL_DIAG_DIE_SYNTAX(x)     PERL_DIAG_STR_(x)

#ifndef PERL_STOP_PARSING_AFTER_N_ERRORS
#define PERL_STOP_PARSING_AFTER_N_ERRORS 10
#endif

#define PERL_PARSE_ERROR_COUNT(f)     (f)

/*

   (KEEP THIS LAST IN perl.h!)

   Mention

   NV_PRESERVES_UV

   HAS_MKSTEMP
   HAS_MKSTEMPS
   HAS_MKDTEMP

   HAS_GETCWD

   HAS_MMAP
   HAS_MPROTECT
   HAS_MSYNC
   HAS_MADVISE
   HAS_MUNMAP
   I_SYSMMAN
   Mmap_t

   NVef
   NVff
   NVgf

   HAS_UALARM
   HAS_USLEEP

   HAS_SETITIMER
   HAS_GETITIMER

   HAS_SENDMSG
   HAS_RECVMSG
   HAS_READV
   HAS_WRITEV
   I_SYSUIO
   HAS_STRUCT_MSGHDR
   HAS_STRUCT_CMSGHDR

   HAS_NL_LANGINFO

   HAS_DIRFD

   so that Configure picks them up.

   (KEEP THIS LAST IN perl.h!)

*/

#endif /* Include guard */

/*
 * ex: set ts=8 sts=4 sw=4 et:
 */
