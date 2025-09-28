#line 2 "perl.c"
/*    perl.c
 *
 *    Copyright (C) 1993, 1994, 1995, 1996, 1997, 1998, 1999, 2000, 2001
 *    2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012
 *    2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022, 2023
 *    by Larry Wall and others
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

/*
 *      A ship then new they built for him
 *      of mithril and of elven-glass
 *              --from Bilbo's song of Eärendil
 *
 *     [p.236 of _The Lord of the Rings_, II/i: "Many Meetings"]
 */

/* This file contains the top-level functions that are used to create, use
 * and destroy a perl interpreter, plus the functions used by XS code to
 * call back into perl. Note that it does not contain the actual main()
 * function of the interpreter; that can be found in perlmain.c
 *
 * Note that at build time this file is also linked to as perlmini.c,
 * and perlmini.o is then built with PERL_IS_MINIPERL defined, which is
 * then used to create the miniperl executable, rather than perl.o.
 */

#if defined(PERL_IS_MINIPERL) && !defined(USE_SITECUSTOMIZE)
#  define USE_SITECUSTOMIZE
#endif

#include "EXTERN.h"
#define PERL_IN_PERL_C
#include "perl.h"
#include "patchlevel.h"			/* for local_patches */
#include "XSUB.h"

#ifdef DEBUG_LEAKING_SCALARS_FORK_DUMP
#  ifdef I_SYSUIO
#    include <sys/uio.h>
#  endif

union control_un {
  struct cmsghdr cm;
  char control[CMSG_SPACE(sizeof(int))];
};

#endif

#ifndef HZ
#  ifdef CLK_TCK
#    define HZ CLK_TCK
#  else
#    define HZ 60
#  endif
#endif

static I32 read_e_script(pTHX_ int idx, SV *buf_sv, int maxlen);

#ifdef SETUID_SCRIPTS_ARE_SECURE_NOW
#  define validate_suid(rsfp) NOOP
#else
#  define validate_suid(rsfp) S_validate_suid(aTHX_ rsfp)
#endif

#define CALL_LIST_BODY(cv) \
    PUSHMARK(PL_stack_sp); \
    call_sv(MUTABLE_SV((cv)), G_EVAL|G_DISCARD|G_VOID);

static void
S_init_tls_and_interp(PerlInterpreter *my_perl)
{
    if (!PL_curinterp) {
        PERL_SET_INTERP(my_perl);
#if defined(USE_ITHREADS)
        INIT_THREADS;
        ALLOC_THREAD_KEY;
        PERL_SET_THX(my_perl);
        OP_REFCNT_INIT;
        OP_CHECK_MUTEX_INIT;
        KEYWORD_PLUGIN_MUTEX_INIT;
        HINTS_REFCNT_INIT;
        LOCALE_INIT;
        USER_PROP_MUTEX_INIT;
        ENV_INIT;
        MUTEX_INIT(&PL_dollarzero_mutex);
        MUTEX_INIT(&PL_my_ctx_mutex);
#  endif
    }
#if defined(USE_ITHREADS)
    else
#else
    /* This always happens for non-ithreads  */
#endif
    {
        PERL_SET_THX(my_perl);
    }
}


#ifndef PLATFORM_SYS_INIT_
#  define PLATFORM_SYS_INIT_  NOOP
#endif

#ifndef PLATFORM_SYS_TERM_
#  define PLATFORM_SYS_TERM_  NOOP
#endif

#ifndef PERL_SYS_INIT_BODY
#  define PERL_SYS_INIT_BODY(c,v)                               \
        MALLOC_CHECK_TAINT2(*c,*v) PERL_FPU_INIT; PERLIO_INIT;  \
        MALLOC_INIT; PLATFORM_SYS_INIT_;
#endif

/* Generally add things last-in first-terminated.  IO and memory terminations
 * need to be generally last
 *
 * BEWARE that using PerlIO in these will be using freed memory, so may appear
 * to work, but must NOT be retained in production code. */
#ifndef PERL_SYS_TERM_BODY
#  define PERL_SYS_TERM_BODY()                                          \
                    ENV_TERM; USER_PROP_MUTEX_TERM; LOCALE_TERM;        \
                    HINTS_REFCNT_TERM; KEYWORD_PLUGIN_MUTEX_TERM;       \
                    OP_CHECK_MUTEX_TERM; OP_REFCNT_TERM;                \
                    PERLIO_TERM; MALLOC_TERM;                           \
                    PLATFORM_SYS_TERM_;
#endif

/* these implement the PERL_SYS_INIT, PERL_SYS_INIT3, PERL_SYS_TERM macros */

void
Perl_sys_init(int* argc, char*** argv)
{

    PERL_ARGS_ASSERT_SYS_INIT;

    PERL_UNUSED_ARG(argc); /* may not be used depending on _BODY macro */
    PERL_UNUSED_ARG(argv);
    PERL_SYS_INIT_BODY(argc, argv);
}

void
Perl_sys_init3(int* argc, char*** argv, char*** env)
{

    PERL_ARGS_ASSERT_SYS_INIT3;

    PERL_UNUSED_ARG(argc); /* may not be used depending on _BODY macro */
    PERL_UNUSED_ARG(argv);
    PERL_UNUSED_ARG(env);
    PERL_SYS_INIT3_BODY(argc, argv, env);
}

void
Perl_sys_term(void)
{
    if (!PL_veto_cleanup) {
        PERL_SYS_TERM_BODY();
    }
}


#ifdef PERL_IMPLICIT_SYS
PerlInterpreter *
perl_alloc_using(struct IPerlMem* ipM, struct IPerlMem* ipMS,
                 struct IPerlMem* ipMP, struct IPerlEnv* ipE,
                 struct IPerlStdIO* ipStd, struct IPerlLIO* ipLIO,
                 struct IPerlDir* ipD, struct IPerlSock* ipS,
                 struct IPerlProc* ipP)
{
    PerlInterpreter *my_perl;

    PERL_ARGS_ASSERT_PERL_ALLOC_USING;

    /* Newx() needs interpreter, so call malloc() instead */
    my_perl = (PerlInterpreter*)(*ipM->pCalloc)(ipM, 1, sizeof(PerlInterpreter));
    S_init_tls_and_interp(my_perl);
    PL_Mem = ipM;
    PL_MemShared = ipMS;
    PL_MemParse = ipMP;
    PL_Env = ipE;
    PL_StdIO = ipStd;
    PL_LIO = ipLIO;
    PL_Dir = ipD;
    PL_Sock = ipS;
    PL_Proc = ipP;
    INIT_TRACK_MEMPOOL(PL_memory_debug_header, my_perl);

    return my_perl;
}
#else

/*
=for apidoc_section $embedding

=for apidoc perl_alloc

Allocates a new Perl interpreter.  See L<perlembed>.

=cut
*/

PerlInterpreter *
perl_alloc(void)
{
    PerlInterpreter *my_perl = (PerlInterpreter*)PerlMem_calloc(1, sizeof(PerlInterpreter));

    S_init_tls_and_interp(my_perl);
    INIT_TRACK_MEMPOOL(PL_memory_debug_header, my_perl);
    return my_perl;
}
#endif /* PERL_IMPLICIT_SYS */

/*
=for apidoc perl_construct

Initializes a new Perl interpreter.  See L<perlembed>.

=cut
*/

void
perl_construct(pTHXx)
{

    PERL_ARGS_ASSERT_PERL_CONSTRUCT;

#ifdef MULTIPLICITY
    init_interp();
    PL_perl_destruct_level = 1;
#else
    PERL_UNUSED_ARG(my_perl);
   if (PL_perl_destruct_level > 0)
       init_interp();
#endif
    PL_curcop = &PL_compiling;	/* needed by ckWARN, right away */

#ifdef PERL_TRACE_OPS
    Zero(PL_op_exec_cnt, OP_max+2, UV);
#endif

    init_constants();

    SvREADONLY_on(&PL_sv_placeholder);
    SvREFCNT(&PL_sv_placeholder) = SvREFCNT_IMMORTAL;

    PL_sighandlerp  = Perl_sighandler;
    PL_sighandler1p = Perl_sighandler1;
    PL_sighandler3p = Perl_sighandler3;

#ifdef PERL_USES_PL_PIDSTATUS
    PL_pidstatus = newHV();
#endif

    PL_rs = newSVpvs("\n");

    init_stacks();

#if !defined(NO_PERL_RAND_SEED) || !defined(NO_PERL_INTERNAL_HASH_SEED)
    bool sensitive_env_vars_allowed =
            (PerlProc_getuid() == PerlProc_geteuid() &&
             PerlProc_getgid() == PerlProc_getegid()) ? TRUE : FALSE;
#endif

/* The seed set-up must be after init_stacks because it calls
 * things that may put SVs on the stack.
 */
#ifndef NO_PERL_RAND_SEED
    if (sensitive_env_vars_allowed) {
        UV seed= 0;
        const char *env_pv;
        if ((env_pv = PerlEnv_getenv("PERL_RAND_SEED")) &&
            grok_number(env_pv, strlen(env_pv), &seed) == IS_NUMBER_IN_UV)
        {

            PL_srand_override_next = seed;
            PERL_SRAND_OVERRIDE_NEXT_INIT();
        }
    }
#endif

    /* This is NOT the state used for C<rand()>, this is only
     * used in internal functionality */
#ifdef NO_PERL_INTERNAL_RAND_SEED
    Perl_drand48_init_r(&PL_internal_random_state, seed());
#else
    {
        UV seed;
        const char *env_pv;
        if (
            !sensitive_env_vars_allowed ||
            !(env_pv = PerlEnv_getenv("PERL_INTERNAL_RAND_SEED")) ||
            grok_number(env_pv, strlen(env_pv), &seed) != IS_NUMBER_IN_UV)
        {
            /* use a randomly generated seed */
            seed = seed();
        }
        Perl_drand48_init_r(&PL_internal_random_state, (U32)seed);
    }
#endif

    init_ids();

    JMPENV_BOOTSTRAP;
    STATUS_ALL_SUCCESS;

    init_uniprops();
    (void) uvchr_to_utf8_flags((U8 *) PL_TR_SPECIAL_HANDLING_UTF8,
                               TR_SPECIAL_HANDLING,
                               UNICODE_ALLOW_ABOVE_IV_MAX);

#if defined(LOCAL_PATCH_COUNT)
    PL_localpatches = local_patches;	/* For possible -v */
#endif

#if defined(LIBM_LIB_VERSION)
    /*
     * Some BSDs and Cygwin default to POSIX math instead of IEEE.
     * This switches them over to IEEE.
     */
    _LIB_VERSION = _IEEE_;
#endif

#ifdef HAVE_INTERP_INTERN
    sys_intern_init();
#endif

    PerlIO_init(aTHX);			/* Hook to IO system */

    PL_fdpid = newAV();			/* for remembering popen pids by fd */
    PL_modglobal = newHV();		/* pointers to per-interpreter module globals */
    PL_errors = newSVpvs("");
    SvPVCLEAR(PERL_DEBUG_PAD(0));        /* For regex debugging. */
    SvPVCLEAR(PERL_DEBUG_PAD(1));        /* ext/re needs these */
    SvPVCLEAR(PERL_DEBUG_PAD(2));        /* even without DEBUGGING. */
#ifdef USE_ITHREADS
    /* First entry is a list of empty elements. It needs to be initialised
       else all hell breaks loose in S_find_uninit_var().  */
    Perl_av_create_and_push(aTHX_ &PL_regex_padav, newSVpvs(""));
    PL_regex_pad = AvARRAY(PL_regex_padav);
    Newxz(PL_stashpad, PL_stashpadmax, HV *);
#endif
#ifdef USE_REENTRANT_API
    Perl_reentrant_init(aTHX);
#endif
    if (PL_hash_seed_set == FALSE) {
        /* Initialize the hash seed and state at startup. This must be
         * done very early, before ANY hashes are constructed, and once
         * setup is fixed for the lifetime of the process.
         *
         * If you decide to disable the seeding process you should choose
         * a suitable seed yourself and define PERL_HASH_SEED to a well chosen
         * string. See hv_func.h for details.
         */
#if defined(USE_HASH_SEED)
        /* get the hash seed from the environment or from an RNG */
        Perl_get_hash_seed(aTHX_ PL_hash_seed);
#else
        /* they want a hard coded seed, check that it is long enough */
        assert( strlen(PERL_HASH_SEED) >= PERL_HASH_SEED_BYTES );
#endif

        /* now we use the chosen seed to initialize the state -
         * in some configurations this may be a relatively speaking
         * expensive operation, but we only have to do it once at startup */
        PERL_HASH_SEED_STATE(PERL_HASH_SEED,PL_hash_state);

#ifdef PERL_USE_SINGLE_CHAR_HASH_CACHE
        /* we can build a special cache for 0/1 byte keys, if people choose
         * I suspect most of the time it is not worth it */
        {
            char str[2]="\0";
            int i;
            for (i=0;i<256;i++) {
                str[0]= i;
                PERL_HASH_WITH_STATE(PL_hash_state,PL_hash_chars[i],str,1);
            }
            PERL_HASH_WITH_STATE(PL_hash_state,PL_hash_chars[256],str,0);
        }
#endif
        /* at this point we have initialized the hash function, and we can start
         * constructing hashes */
        PL_hash_seed_set= TRUE;
    }

    /* Allow PL_strtab to be pre-initialized before calling perl_construct.
    * can use a custom optimized PL_strtab hash before calling perl_construct */
    if (!PL_strtab) {
        /* Note that strtab is a rather special HV.  Assumptions are made
           about not iterating on it, and not adding tie magic to it.
           It is properly deallocated in perl_destruct() */
        PL_strtab = newHV();

        /* SHAREKEYS tells us that the hash has its keys shared with PL_strtab,
         * which is not the case with PL_strtab itself */
        HvSHAREKEYS_off(PL_strtab);			/* mandatory */
        hv_ksplit(PL_strtab, 1 << 11);
    }

#ifdef USE_ITHREADS
    PL_compiling.cop_file = NULL;
    PL_compiling.cop_warnings = NULL;
#endif

    Zero(PL_sv_consts, SV_CONSTS_COUNT, SV*);

#ifndef PERL_MICRO
#   ifdef  USE_ENVIRON_ARRAY
    if (!PL_origenviron)
        PL_origenviron = environ;
#   endif
#endif

    /* Use sysconf(_SC_CLK_TCK) if available, if not
     * available or if the sysconf() fails, use the HZ.
     * The HZ if not originally defined has been by now
     * been defined as CLK_TCK, if available. */
#if defined(HAS_SYSCONF) && defined(_SC_CLK_TCK)
    PL_clocktick = sysconf(_SC_CLK_TCK);
    if (PL_clocktick <= 0)
#endif
         PL_clocktick = HZ;

    PL_stashcache = newHV();

    PL_patchlevel = newSVpvs("v" PERL_VERSION_STRING);

#ifdef HAS_MMAP
    if (!PL_mmap_page_size) {
#if defined(HAS_SYSCONF) && (defined(_SC_PAGESIZE) || defined(_SC_MMAP_PAGE_SIZE))
      {
        SETERRNO(0, SS_NORMAL);
#   ifdef _SC_PAGESIZE
        PL_mmap_page_size = sysconf(_SC_PAGESIZE);
#   else
        PL_mmap_page_size = sysconf(_SC_MMAP_PAGE_SIZE);
#   endif
        if ((long) PL_mmap_page_size < 0) {
            Perl_croak(aTHX_ "panic: sysconf: %s",
                errno ? Strerror(errno) : "pagesize unknown");
        }
      }
#elif defined(HAS_GETPAGESIZE)
      PL_mmap_page_size = getpagesize();
#elif defined(I_SYS_PARAM) && defined(PAGESIZE)
      PL_mmap_page_size = PAGESIZE;       /* compiletime, bad */
#endif
      if (PL_mmap_page_size <= 0)
        Perl_croak(aTHX_ "panic: bad pagesize %" IVdf,
                   (IV) PL_mmap_page_size);
    }
#endif /* HAS_MMAP */

    PL_osname = Perl_savepvn(aTHX_ STR_WITH_LEN(OSNAME));

    PL_registered_mros = newHV();
    /* Start with 1 bucket, for DFS.  It's unlikely we'll need more.  */
    HvMAX(PL_registered_mros) = 0;

    ENTER;
    init_i18nl10n(1);
}

/*
=for apidoc nothreadhook

Stub that provides thread hook for perl_destruct when there are
no threads.

=cut
*/

int
Perl_nothreadhook(pTHX)
{
    PERL_UNUSED_CONTEXT;
    return 0;
}

#ifdef DEBUG_LEAKING_SCALARS_FORK_DUMP
void
Perl_dump_sv_child(pTHX_ SV *sv)
{
    ssize_t got;
    const int sock = PL_dumper_fd;
    const int debug_fd = PerlIO_fileno(Perl_debug_log);
    union control_un control;
    struct msghdr msg;
    struct iovec vec[2];
    struct cmsghdr *cmptr;
    int returned_errno;
    unsigned char buffer[256];

    PERL_ARGS_ASSERT_DUMP_SV_CHILD;

    if(sock == -1 || debug_fd == -1)
        return;

    PerlIO_flush(Perl_debug_log);

    /* All these shenanigans are to pass a file descriptor over to our child for
       it to dump out to.  We can't let it hold open the file descriptor when it
       forks, as the file descriptor it will dump to can turn out to be one end
       of pipe that some other process will wait on for EOF. (So as it would
       be open, the wait would be forever.)  */

    msg.msg_control = control.control;
    msg.msg_controllen = sizeof(control.control);
    /* We're a connected socket so we don't need a destination  */
    msg.msg_name = NULL;
    msg.msg_namelen = 0;
    msg.msg_iov = vec;
    msg.msg_iovlen = 1;

    cmptr = CMSG_FIRSTHDR(&msg);
    cmptr->cmsg_len = CMSG_LEN(sizeof(int));
    cmptr->cmsg_level = SOL_SOCKET;
    cmptr->cmsg_type = SCM_RIGHTS;
    *((int *)CMSG_DATA(cmptr)) = 1;

    vec[0].iov_base = (void*)&sv;
    vec[0].iov_len = sizeof(sv);
    got = sendmsg(sock, &msg, 0);

    if(got < 0) {
        perror("Debug leaking scalars parent sendmsg failed");
        abort();
    }
    if(got < sizeof(sv)) {
        perror("Debug leaking scalars parent short sendmsg");
        abort();
    }

    /* Return protocol is
       int:		errno value
       unsigned char:	length of location string (0 for empty)
       unsigned char*:	string (not terminated)
    */
    vec[0].iov_base = (void*)&returned_errno;
    vec[0].iov_len = sizeof(returned_errno);
    vec[1].iov_base = buffer;
    vec[1].iov_len = 1;

    got = readv(sock, vec, 2);

    if(got < 0) {
        perror("Debug leaking scalars parent read failed");
        PerlIO_flush(PerlIO_stderr());
        abort();
    }
    if(got < sizeof(returned_errno) + 1) {
        perror("Debug leaking scalars parent short read");
        PerlIO_flush(PerlIO_stderr());
        abort();
    }

    if (*buffer) {
        got = read(sock, buffer + 1, *buffer);
        if(got < 0) {
            perror("Debug leaking scalars parent read 2 failed");
            PerlIO_flush(PerlIO_stderr());
            abort();
        }

        if(got < *buffer) {
            perror("Debug leaking scalars parent short read 2");
            PerlIO_flush(PerlIO_stderr());
            abort();
        }
    }

    if (returned_errno || *buffer) {
        Perl_warn(aTHX_ "Debug leaking scalars child failed%s%.*s with errno"
                  " %d: %s", (*buffer ? " at " : ""), (int) *buffer, buffer + 1,
                  returned_errno, Strerror(returned_errno));
    }
}
#endif

/*
=for apidoc perl_destruct

Shuts down a Perl interpreter.  See L<perlembed> for a tutorial.

C<my_perl> points to the Perl interpreter.  It must have been previously
created through the use of L</perl_alloc> and L</perl_construct>.  It may
have been initialised through L</perl_parse>, and may have been used
through L</perl_run> and other means.  This function should be called for
any Perl interpreter that has been constructed with L</perl_construct>,
even if subsequent operations on it failed, for example if L</perl_parse>
returned a non-zero value.

If the interpreter's C<PL_exit_flags> word has the
C<PERL_EXIT_DESTRUCT_END> flag set, then this function will execute code
in C<END> blocks before performing the rest of destruction.  If it is
desired to make any use of the interpreter between L</perl_parse> and
L</perl_destruct> other than just calling L</perl_run>, then this flag
should be set early on.  This matters if L</perl_run> will not be called,
or if anything else will be done in addition to calling L</perl_run>.

Returns a value be a suitable value to pass to the C library function
C<exit> (or to return from C<main>), to serve as an exit code indicating
the nature of the way the interpreter terminated.  This takes into account
any failure of L</perl_parse> and any early exit from L</perl_run>.
The exit code is of the type required by the host operating system,
so because of differing exit code conventions it is not portable to
interpret specific numeric values as having specific meanings.

=cut
*/

int
perl_destruct(pTHXx)
{
    volatile signed char destruct_level;  /* see possible values in intrpvar.h */
    HV *hv;
#ifdef DEBUG_LEAKING_SCALARS_FORK_DUMP
    pid_t child;
#endif
    int i;

    PERL_ARGS_ASSERT_PERL_DESTRUCT;
#ifndef MULTIPLICITY
    PERL_UNUSED_ARG(my_perl);
#endif

    assert(PL_scopestack_ix == 1);

    destruct_level = PL_perl_destruct_level;
    {
        const char * const s = PerlEnv_getenv("PERL_DESTRUCT_LEVEL");
        if (s) {
            int i;
            if (strEQ(s, "-1")) { /* Special case: modperl folklore. */
                i = -1;
            } else {
                UV uv;
                if (grok_atoUV(s, &uv, NULL) && uv <= INT_MAX)
                    i = (int)uv;
                else
                    i = 0;
            }
            if (destruct_level < i) destruct_level = i;
#ifdef PERL_TRACK_MEMPOOL
            /* RT #114496, for perl_free */
            PL_perl_destruct_level = i;
#endif
        }
    }

    if (PL_exit_flags & PERL_EXIT_DESTRUCT_END) {
        dJMPENV;
        int x = 0;

        JMPENV_PUSH(x);
        PERL_UNUSED_VAR(x);
        if (PL_endav && !PL_minus_c) {
            PERL_SET_PHASE(PERL_PHASE_END);
            call_list(PL_scopestack_ix, PL_endav);
        }
        JMPENV_POP;
    }
    LEAVE;
    FREETMPS;
    assert(PL_scopestack_ix == 0);

    /* wait for all pseudo-forked children to finish */
    PERL_WAIT_FOR_CHILDREN;


    /* normally when we get here, PL_parser should be null due to having
     * its original (null) value restored by SAVEt_PARSER during leaving
     * scope (usually before run-time starts in fact).
     * But if a thread is created within a BEGIN block, the parser is
     * duped, but the SAVEt_PARSER savestack entry isn't. So PL_parser
     * never gets cleaned up.
     * Clean it up here instead. This is a bit of a hack.
     */
    if (PL_parser) {
        /* stop parser_free() stomping on PL_curcop */
        PL_parser->saved_curcop = PL_curcop;
        parser_free(PL_parser);
    }


    /* Need to flush since END blocks can produce output */
    /* flush stdout separately, since we can identify it */
#ifdef USE_PERLIO
    {
        PerlIO *stdo = PerlIO_stdout();
        if (*stdo && PerlIO_flush(stdo)) {
            PerlIO_restore_errno(stdo);
            if (errno)
                PerlIO_printf(PerlIO_stderr(), "Unable to flush stdout: %s\n",
                    Strerror(errno));
            if (!STATUS_UNIX)
                STATUS_ALL_FAILURE;
        }
    }
#endif
    my_fflush_all();

#ifdef PERL_TRACE_OPS
    /* dump OP-counts if $ENV{PERL_TRACE_OPS} > 0 */
    {
        const char * const ptoenv = PerlEnv_getenv("PERL_TRACE_OPS");
        UV uv;

        if (!ptoenv || !Perl_grok_atoUV(ptoenv, &uv, NULL)
            || !(uv > 0))
        goto no_trace_out;
    }
    PerlIO_printf(Perl_debug_log, "Trace of all OPs executed:\n");
    for (i = 0; i <= OP_max; ++i) {
        if (PL_op_exec_cnt[i])
            PerlIO_printf(Perl_debug_log, "  %s: %" UVuf "\n", PL_op_name[i], PL_op_exec_cnt[i]);
    }
    /* Utility slot for easily doing little tracing experiments in the runloop: */
    if (PL_op_exec_cnt[OP_max+1] != 0)
        PerlIO_printf(Perl_debug_log, "  SPECIAL: %" UVuf "\n", PL_op_exec_cnt[OP_max+1]);
    PerlIO_printf(Perl_debug_log, "\n");
 no_trace_out:
#endif


    if (PL_threadhook(aTHX)) {
        /* Threads hook has vetoed further cleanup */
        PL_veto_cleanup = TRUE;
        return STATUS_EXIT;
    }

#ifdef DEBUG_LEAKING_SCALARS_FORK_DUMP
    if (destruct_level != 0) {
        /* Fork here to create a child. Our child's job is to preserve the
           state of scalars prior to destruction, so that we can instruct it
           to dump any scalars that we later find have leaked.
           There's no subtlety in this code - it assumes POSIX, and it doesn't
           fail gracefully  */
        int fd[2];

        if(PerlSock_socketpair_cloexec(AF_UNIX, SOCK_STREAM, 0, fd)) {
            perror("Debug leaking scalars socketpair failed");
            abort();
        }

        child = fork();
        if(child == -1) {
            perror("Debug leaking scalars fork failed");
            abort();
        }
        if (!child) {
            /* We are the child */
            const int sock = fd[1];
            const int debug_fd = PerlIO_fileno(Perl_debug_log);
            int f;
            const char *where;
            /* Our success message is an integer 0, and a char 0  */
            static const char success[sizeof(int) + 1] = {0};

            close(fd[0]);

            /* We need to close all other file descriptors otherwise we end up
               with interesting hangs, where the parent closes its end of a
               pipe, and sits waiting for (another) child to terminate. Only
               that child never terminates, because it never gets EOF, because
               we also have the far end of the pipe open.  We even need to
               close the debugging fd, because sometimes it happens to be one
               end of a pipe, and a process is waiting on the other end for
               EOF. Normally it would be closed at some point earlier in
               destruction, but if we happen to cause the pipe to remain open,
               EOF never occurs, and we get an infinite hang. Hence all the
               games to pass in a file descriptor if it's actually needed.  */

            f = sysconf(_SC_OPEN_MAX);
            if(f < 0) {
                where = "sysconf failed";
                goto abort;
            }
            while (f--) {
                if (f == sock)
                    continue;
                close(f);
            }

            while (1) {
                SV *target;
                union control_un control;
                struct msghdr msg;
                struct iovec vec[1];
                struct cmsghdr *cmptr;
                ssize_t got;
                int got_fd;

                msg.msg_control = control.control;
                msg.msg_controllen = sizeof(control.control);
                /* We're a connected socket so we don't need a source  */
                msg.msg_name = NULL;
                msg.msg_namelen = 0;
                msg.msg_iov = vec;
                msg.msg_iovlen = C_ARRAY_LENGTH(vec);

                vec[0].iov_base = (void*)&target;
                vec[0].iov_len = sizeof(target);

                got = recvmsg(sock, &msg, 0);

                if(got == 0)
                    break;
                if(got < 0) {
                    where = "recv failed";
                    goto abort;
                }
                if(got < sizeof(target)) {
                    where = "short recv";
                    goto abort;
                }

                if(!(cmptr = CMSG_FIRSTHDR(&msg))) {
                    where = "no cmsg";
                    goto abort;
                }
                if(cmptr->cmsg_len != CMSG_LEN(sizeof(int))) {
                    where = "wrong cmsg_len";
                    goto abort;
                }
                if(cmptr->cmsg_level != SOL_SOCKET) {
                    where = "wrong cmsg_level";
                    goto abort;
                }
                if(cmptr->cmsg_type != SCM_RIGHTS) {
                    where = "wrong cmsg_type";
                    goto abort;
                }

                got_fd = *(int*)CMSG_DATA(cmptr);
                /* For our last little bit of trickery, put the file descriptor
                   back into Perl_debug_log, as if we never actually closed it
                */
                if(got_fd != debug_fd) {
                    if (PerlLIO_dup2_cloexec(got_fd, debug_fd) == -1) {
                        where = "dup2";
                        goto abort;
                    }
                }
                sv_dump(target);

                PerlIO_flush(Perl_debug_log);

                got = write(sock, &success, sizeof(success));

                if(got < 0) {
                    where = "write failed";
                    goto abort;
                }
                if(got < sizeof(success)) {
                    where = "short write";
                    goto abort;
                }
            }
            _exit(0);
        abort:
            {
                int send_errno = errno;
                unsigned char length = (unsigned char) strlen(where);
                struct iovec failure[3] = {
                    {(void*)&send_errno, sizeof(send_errno)},
                    {&length, 1},
                    {(void*)where, length}
                };
                int got = writev(sock, failure, 3);
                /* Bad news travels fast. Faster than data. We'll get a SIGPIPE
                   in the parent if we try to read from the socketpair after the
                   child has exited, even if there was data to read.
                   So sleep a bit to give the parent a fighting chance of
                   reading the data.  */
                sleep(2);
                _exit((got == -1) ? errno : 0);
            }
            /* End of child.  */
        }
        PL_dumper_fd = fd[0];
        close(fd[1]);
    }
#endif

    /* We must account for everything.  */

    /* Destroy the main CV and syntax tree */
    /* Set PL_curcop now, because destroying ops can cause new SVs
       to be generated in Perl_pad_swipe, and when running with
      -DDEBUG_LEAKING_SCALARS they expect PL_curcop to point to a valid
       op from which the filename structure member is copied.  */
    PL_curcop = &PL_compiling;
    if (PL_main_root) {
        /* ensure comppad/curpad to refer to main's pad */
        if (CvPADLIST(PL_main_cv)) {
            PAD_SET_CUR_NOSAVE(CvPADLIST(PL_main_cv), 1);
            PL_comppad_name = PadlistNAMES(CvPADLIST(PL_main_cv));
        }
        op_free(PL_main_root);
        PL_main_root = NULL;
    }
    PL_main_start = NULL;
    /* note that  PL_main_cv isn't usually actually freed at this point,
     * due to the CvOUTSIDE refs from subs compiled within it. It will
     * get freed once all the subs are freed in sv_clean_all(), for
     * destruct_level > 0 */
    SvREFCNT_dec(PL_main_cv);
    PL_main_cv = NULL;
    PERL_SET_PHASE(PERL_PHASE_DESTRUCT);

    /* Tell PerlIO we are about to tear things apart in case
       we have layers which are using resources that should
       be cleaned up now.
     */

    PerlIO_destruct(aTHX);

    /*
     * Try to destruct global references.  We do this first so that the
     * destructors and destructees still exist.  Some sv's might remain.
     * Non-referenced objects are on their own.
     */
    sv_clean_objs();

    /* unhook hooks which will soon be, or use, destroyed data */
    SvREFCNT_dec(PL_warnhook);
    PL_warnhook = NULL;
    SvREFCNT_dec(PL_diehook);
    PL_diehook = NULL;
    SvREFCNT_dec(PL_hook__require__before);
    PL_hook__require__before = NULL;
    SvREFCNT_dec(PL_hook__require__after);
    PL_hook__require__after = NULL;

    /* call exit list functions */
    while (PL_exitlistlen-- > 0)
        PL_exitlist[PL_exitlistlen].fn(aTHX_ PL_exitlist[PL_exitlistlen].ptr);

    Safefree(PL_exitlist);

    PL_exitlist = NULL;
    PL_exitlistlen = 0;

    SvREFCNT_dec(PL_registered_mros);

    if (destruct_level == 0) {

        DEBUG_P(debprofdump());

#if defined(PERLIO_LAYERS)
        /* No more IO - including error messages ! */
        PerlIO_cleanup(aTHX);
#endif

        CopFILE_free(&PL_compiling);

        /* The exit() function will do everything that needs doing. */
        return STATUS_EXIT;
    }

    /* Below, do clean up for when PERL_DESTRUCT_LEVEL is not 0 */

#ifdef USE_ITHREADS
    /* the syntax tree is shared between clones
     * so op_free(PL_main_root) only ReREFCNT_dec's
     * REGEXPs in the parent interpreter
     * we need to manually ReREFCNT_dec for the clones
     */
    {
        I32 i = AvFILLp(PL_regex_padav);
        SV **ary = AvARRAY(PL_regex_padav);

        for (; i; i--) {
            SvREFCNT_dec(ary[i]);
            ary[i] = &PL_sv_undef;
        }
    }
#endif


    SvREFCNT_dec(MUTABLE_SV(PL_stashcache));
    PL_stashcache = NULL;

    /* loosen bonds of global variables */

    /* XXX can PL_parser still be non-null here? */
    if(PL_parser && PL_parser->rsfp) {
        (void)PerlIO_close(PL_parser->rsfp);
        PL_parser->rsfp = NULL;
    }

    if (PL_minus_F) {
        Safefree(PL_splitstr);
        PL_splitstr = NULL;
    }

    /* switches */
    PL_minus_n      = FALSE;
    PL_minus_p      = FALSE;
    PL_minus_l      = FALSE;
    PL_minus_a      = FALSE;
    PL_minus_F      = FALSE;
    PL_doswitches   = FALSE;
    PL_dowarn       = G_WARN_OFF;
#ifdef PERL_SAWAMPERSAND
    PL_sawampersand = 0;	/* must save all match strings */
#endif
    PL_unsafe       = FALSE;

    Safefree(PL_inplace);
    PL_inplace = NULL;
    SvREFCNT_dec(PL_patchlevel);

    if (PL_e_script) {
        SvREFCNT_dec(PL_e_script);
        PL_e_script = NULL;
    }

    PL_perldb = 0;

    /* magical thingies */

    SvREFCNT_dec(PL_ofsgv);	/* *, */
    PL_ofsgv = NULL;

    SvREFCNT_dec(PL_ors_sv);	/* $\ */
    PL_ors_sv = NULL;

    SvREFCNT_dec(PL_rs);	/* $/ */
    PL_rs = NULL;

    Safefree(PL_osname);	/* $^O */
    PL_osname = NULL;

    SvREFCNT_dec(PL_statname);
    PL_statname = NULL;
    PL_statgv = NULL;

    /* defgv, aka *_ should be taken care of elsewhere */

    /* float buffer */
    Safefree(PL_efloatbuf);
    PL_efloatbuf = NULL;
    PL_efloatsize = 0;

    /* startup and shutdown function lists */
    SvREFCNT_dec(PL_beginav);
    SvREFCNT_dec(PL_beginav_save);
    SvREFCNT_dec(PL_endav);
    SvREFCNT_dec(PL_checkav);
    SvREFCNT_dec(PL_checkav_save);
    SvREFCNT_dec(PL_unitcheckav);
    SvREFCNT_dec(PL_unitcheckav_save);
    SvREFCNT_dec(PL_initav);
    PL_beginav = NULL;
    PL_beginav_save = NULL;
    PL_endav = NULL;
    PL_checkav = NULL;
    PL_checkav_save = NULL;
    PL_unitcheckav = NULL;
    PL_unitcheckav_save = NULL;
    PL_initav = NULL;

    /* shortcuts just get cleared */
    PL_hintgv = NULL;
    PL_errgv = NULL;
    PL_argvoutgv = NULL;
    PL_stdingv = NULL;
    PL_stderrgv = NULL;
    PL_last_in_gv = NULL;
    PL_DBsingle = NULL;
    PL_DBtrace = NULL;
    PL_DBsignal = NULL;
    PL_DBsingle_iv = 0;
    PL_DBtrace_iv = 0;
    PL_DBsignal_iv = 0;
    PL_DBcv = NULL;
    PL_dbargs = NULL;
    PL_debstash = NULL;

    SvREFCNT_dec(PL_envgv);
    SvREFCNT_dec(PL_incgv);
    SvREFCNT_dec(PL_argvgv);
    SvREFCNT_dec(PL_replgv);
    SvREFCNT_dec(PL_DBgv);
    SvREFCNT_dec(PL_DBline);
    SvREFCNT_dec(PL_DBsub);
    PL_envgv = NULL;
    PL_incgv = NULL;
    PL_argvgv = NULL;
    PL_replgv = NULL;
    PL_DBgv = NULL;
    PL_DBline = NULL;
    PL_DBsub = NULL;

    SvREFCNT_dec(PL_argvout_stack);
    PL_argvout_stack = NULL;

    SvREFCNT_dec(PL_modglobal);
    PL_modglobal = NULL;
    SvREFCNT_dec(PL_preambleav);
    PL_preambleav = NULL;
    SvREFCNT_dec(PL_subname);
    PL_subname = NULL;
#ifdef PERL_USES_PL_PIDSTATUS
    SvREFCNT_dec(PL_pidstatus);
    PL_pidstatus = NULL;
#endif
    SvREFCNT_dec(PL_toptarget);
    PL_toptarget = NULL;
    SvREFCNT_dec(PL_bodytarget);
    PL_bodytarget = NULL;
    PL_formtarget = NULL;

    /* free locale stuff */
#ifdef USE_LOCALE_COLLATE
    Safefree(PL_collation_name);
    PL_collation_name = NULL;
#endif
#if defined(USE_PL_CURLOCALES)
    for (i = 0; i < (int) C_ARRAY_LENGTH(PL_curlocales); i++) {
        Safefree(PL_curlocales[i]);
        PL_curlocales[i] = NULL;
    }
#endif
#ifdef USE_POSIX_2008_LOCALE
    {
        /* This also makes sure we aren't using a locale object that gets freed
         * below */
        if (   PL_cur_locale_obj != NULL
            && PL_cur_locale_obj != LC_GLOBAL_LOCALE
            && PL_cur_locale_obj != PL_C_locale_obj
        ) {
            locale_t cur_locale = uselocale((locale_t) 0);
            if (cur_locale == PL_cur_locale_obj) {
                uselocale(LC_GLOBAL_LOCALE);
            }

            freelocale(PL_cur_locale_obj);
            PL_cur_locale_obj = NULL;
        }
    }

#  ifdef USE_PL_CUR_LC_ALL

    if (PL_cur_LC_ALL) {
        DEBUG_L( PerlIO_printf(Perl_debug_log, "PL_cur_LC_ALL=%p\n", PL_cur_LC_ALL));
        Safefree(PL_cur_LC_ALL);
        PL_cur_LC_ALL = NULL;
    }

#  endif

    if (PL_scratch_locale_obj) {
        freelocale(PL_scratch_locale_obj);
        PL_scratch_locale_obj = NULL;
    }
#  ifdef USE_LOCALE_NUMERIC
    if (PL_underlying_numeric_obj) {
        DEBUG_Lv(PerlIO_printf(Perl_debug_log,
                    "%s:%d: Freeing %p\n", __FILE__, __LINE__,
                    PL_underlying_numeric_obj));
        freelocale(PL_underlying_numeric_obj);
        PL_underlying_numeric_obj = (locale_t) NULL;
    }
#  endif
#endif
#ifdef USE_LOCALE_NUMERIC
    Safefree(PL_numeric_name);
    PL_numeric_name = NULL;
    SvREFCNT_dec(PL_numeric_radix_sv);
    PL_numeric_radix_sv = NULL;
    SvREFCNT_dec(PL_underlying_radix_sv);
    PL_underlying_radix_sv  = NULL;
#endif
#ifdef USE_LOCALE_CTYPE
    Safefree(PL_ctype_name);
    PL_ctype_name = NULL;
#endif

    if (PL_setlocale_buf) {
        Safefree(PL_setlocale_buf);
        PL_setlocale_buf = NULL;
    }

    if (PL_langinfo_buf) {
        Safefree(PL_langinfo_buf);
        PL_langinfo_buf = NULL;
    }

    if (PL_stdize_locale_buf) {
        Safefree(PL_stdize_locale_buf);
        PL_stdize_locale_buf = NULL;
    }

#ifdef USE_LOCALE_CTYPE
    SvREFCNT_dec(PL_warn_locale);
    PL_warn_locale       = NULL;
#endif

    SvREFCNT_dec(PL_AboveLatin1);
    PL_AboveLatin1 = NULL;
    SvREFCNT_dec(PL_Assigned_invlist);
    PL_Assigned_invlist = NULL;
    SvREFCNT_dec(PL_GCB_invlist);
    PL_GCB_invlist = NULL;
    SvREFCNT_dec(PL_HasMultiCharFold);
    PL_HasMultiCharFold = NULL;
    SvREFCNT_dec(PL_InMultiCharFold);
    PL_InMultiCharFold = NULL;
    SvREFCNT_dec(PL_Latin1);
    PL_Latin1 = NULL;
    SvREFCNT_dec(PL_LB_invlist);
    PL_LB_invlist = NULL;
    SvREFCNT_dec(PL_SB_invlist);
    PL_SB_invlist = NULL;
    SvREFCNT_dec(PL_SCX_invlist);
    PL_SCX_invlist = NULL;
    SvREFCNT_dec(PL_UpperLatin1);
    PL_UpperLatin1 = NULL;
    SvREFCNT_dec(PL_in_some_fold);
    PL_in_some_fold = NULL;
    SvREFCNT_dec(PL_utf8_foldclosures);
    PL_utf8_foldclosures = NULL;
    SvREFCNT_dec(PL_utf8_idcont);
    PL_utf8_idcont = NULL;
    SvREFCNT_dec(PL_utf8_idstart);
    PL_utf8_idstart = NULL;
    SvREFCNT_dec(PL_utf8_perl_idcont);
    PL_utf8_perl_idcont = NULL;
    SvREFCNT_dec(PL_utf8_perl_idstart);
    PL_utf8_perl_idstart = NULL;
    SvREFCNT_dec(PL_utf8_xidcont);
    PL_utf8_xidcont = NULL;
    SvREFCNT_dec(PL_utf8_xidstart);
    PL_utf8_xidstart = NULL;
    SvREFCNT_dec(PL_WB_invlist);
    PL_WB_invlist = NULL;
    SvREFCNT_dec(PL_utf8_toupper);
    PL_utf8_toupper = NULL;
    SvREFCNT_dec(PL_utf8_totitle);
    PL_utf8_totitle = NULL;
    SvREFCNT_dec(PL_utf8_tolower);
    PL_utf8_tolower = NULL;
    SvREFCNT_dec(PL_utf8_tofold);
    PL_utf8_tofold = NULL;
    SvREFCNT_dec(PL_utf8_tosimplefold);
    PL_utf8_tosimplefold = NULL;
    SvREFCNT_dec(PL_utf8_charname_begin);
    PL_utf8_charname_begin = NULL;
    SvREFCNT_dec(PL_utf8_charname_continue);
    PL_utf8_charname_continue = NULL;
    SvREFCNT_dec(PL_utf8_mark);
    PL_utf8_mark = NULL;
    SvREFCNT_dec(PL_InBitmap);
    PL_InBitmap = NULL;
    SvREFCNT_dec(PL_CCC_non0_non230);
    PL_CCC_non0_non230 = NULL;
    SvREFCNT_dec(PL_Private_Use);
    PL_Private_Use = NULL;

    for (i = 0; i < POSIX_CC_COUNT; i++) {
        SvREFCNT_dec(PL_XPosix_ptrs[i]);
        PL_XPosix_ptrs[i] = NULL;

        if (i != CC_CASED_) {   /* A copy of Alpha */
            SvREFCNT_dec(PL_Posix_ptrs[i]);
            PL_Posix_ptrs[i] = NULL;
        }
    }

    free_and_set_cop_warnings(&PL_compiling, NULL);
    cophh_free(CopHINTHASH_get(&PL_compiling));
    CopHINTHASH_set(&PL_compiling, cophh_new_empty());
    CopFILE_free(&PL_compiling);

    /* Prepare to destruct main symbol table.  */

    hv = PL_defstash;
    /* break ref loop  *:: <=> %:: */
    (void)hv_deletes(hv, "main::", G_DISCARD);
    PL_defstash = 0;
    SvREFCNT_dec(hv);
    SvREFCNT_dec(PL_curstname);
    PL_curstname = NULL;

    /* clear queued errors */
    SvREFCNT_dec(PL_errors);
    PL_errors = NULL;

    SvREFCNT_dec(PL_isarev);

    FREETMPS;
    if (destruct_level >= 2) {
        if (PL_scopestack_ix != 0)
            Perl_ck_warner_d(aTHX_ packWARN(WARN_INTERNAL),
                             "Unbalanced scopes: %ld more ENTERs than LEAVEs\n",
                             (long)PL_scopestack_ix);
        if (PL_savestack_ix != 0)
            Perl_ck_warner_d(aTHX_ packWARN(WARN_INTERNAL),
                             "Unbalanced saves: %ld more saves than restores\n",
                             (long)PL_savestack_ix);
        if (PL_tmps_floor != -1)
            Perl_ck_warner_d(aTHX_ packWARN(WARN_INTERNAL),"Unbalanced tmps: %ld more allocs than frees\n",
                             (long)PL_tmps_floor + 1);
        if (cxstack_ix != -1)
            Perl_ck_warner_d(aTHX_ packWARN(WARN_INTERNAL),"Unbalanced context: %ld more PUSHes than POPs\n",
                             (long)cxstack_ix + 1);
    }

#ifdef USE_ITHREADS
    SvREFCNT_dec(PL_regex_padav);
    PL_regex_padav = NULL;
    PL_regex_pad = NULL;
#endif

#ifdef MULTIPLICITY
    /* the entries in this list are allocated via SV PVX's, so get freed
     * in sv_clean_all */
    Safefree(PL_my_cxt_list);
#endif

    /* Now absolutely destruct everything, somehow or other, loops or no. */

    /* the 2 is for PL_fdpid and PL_strtab */
    while (sv_clean_all() > 2)
        ;

#ifdef USE_ITHREADS
    Safefree(PL_stashpad); /* must come after sv_clean_all */
#endif

    AvREAL_off(PL_fdpid);		/* no surviving entries */
    SvREFCNT_dec(PL_fdpid);		/* needed in io_close() */
    PL_fdpid = NULL;

#ifdef HAVE_INTERP_INTERN
    sys_intern_clear();
#endif

    /* constant strings */
    for (i = 0; i < SV_CONSTS_COUNT; i++) {
        SvREFCNT_dec(PL_sv_consts[i]);
        PL_sv_consts[i] = NULL;
    }

    /* Destruct the global string table. */
    {
        /* Yell and reset the HeVAL() slots that are still holding refcounts,
         * so that sv_free() won't fail on them.
         * Now that the global string table is using a single hunk of memory
         * for both HE and HEK, we either need to explicitly unshare it the
         * correct way, or actually free things here.
         */
        I32 riter = 0;
        const I32 max = HvMAX(PL_strtab);
        HE * const * const array = HvARRAY(PL_strtab);
        HE *hent = array[0];

        for (;;) {
            if (hent && ckWARN_d(WARN_INTERNAL)) {
                HE * const next = HeNEXT(hent);
                Perl_warner(aTHX_ packWARN(WARN_INTERNAL),
                     "Unbalanced string table refcount: (%ld) for \"%s\"",
                     (long)hent->he_valu.hent_refcount, HeKEY(hent));
                Safefree(hent);
                hent = next;
            }
            if (!hent) {
                if (++riter > max)
                    break;
                hent = array[riter];
            }
        }

        Safefree(array);
        HvARRAY(PL_strtab) = 0;
        HvTOTALKEYS(PL_strtab) = 0;
    }
    SvREFCNT_dec(PL_strtab);

#ifdef USE_ITHREADS
    /* free the pointer tables used for cloning */
    ptr_table_free(PL_ptr_table);
    PL_ptr_table = (PTR_TBL_t*)NULL;
#endif

    /* free special SVs */

    SvREFCNT(&PL_sv_yes) = 0;
    sv_clear(&PL_sv_yes);
    SvANY(&PL_sv_yes) = NULL;
    SvFLAGS(&PL_sv_yes) = 0;

    SvREFCNT(&PL_sv_no) = 0;
    sv_clear(&PL_sv_no);
    SvANY(&PL_sv_no) = NULL;
    SvFLAGS(&PL_sv_no) = 0;

    SvREFCNT(&PL_sv_zero) = 0;
    sv_clear(&PL_sv_zero);
    SvANY(&PL_sv_zero) = NULL;
    SvFLAGS(&PL_sv_zero) = 0;

    {
        int i;
        for (i=0; i<=2; i++) {
            SvREFCNT(PERL_DEBUG_PAD(i)) = 0;
            sv_clear(PERL_DEBUG_PAD(i));
            SvANY(PERL_DEBUG_PAD(i)) = NULL;
            SvFLAGS(PERL_DEBUG_PAD(i)) = 0;
        }
    }

    if (PL_sv_count != 0 && ckWARN_d(WARN_INTERNAL))
        Perl_warner(aTHX_ packWARN(WARN_INTERNAL),"Scalars leaked: %ld\n", (long)PL_sv_count);

#ifdef DEBUG_LEAKING_SCALARS
    if (PL_sv_count != 0) {
        SV* sva;
        SV* sv;
        SV* svend;

        for (sva = PL_sv_arenaroot; sva; sva = MUTABLE_SV(SvANY(sva))) {
            svend = &sva[SvREFCNT(sva)];
            for (sv = sva + 1; sv < svend; ++sv) {
                if (!SvIS_FREED(sv)) {
                    PerlIO_printf(Perl_debug_log, "leaked: sv=0x%p"
                        " flags=0x%" UVxf
                        " refcnt=%" UVuf pTHX__FORMAT "\n"
                        "\tallocated at %s:%d %s %s (parent 0x%" UVxf ");"
                        "serial %" UVuf "\n",
                        (void*)sv, (UV)sv->sv_flags, (UV)sv->sv_refcnt
                        pTHX__VALUE,
                        sv->sv_debug_file ? sv->sv_debug_file : "(unknown)",
                        sv->sv_debug_line,
                        sv->sv_debug_inpad ? "for" : "by",
                        sv->sv_debug_optype ?
                            PL_op_name[sv->sv_debug_optype]: "(none)",
                        PTR2UV(sv->sv_debug_parent),
                        sv->sv_debug_serial
                    );
#ifdef DEBUG_LEAKING_SCALARS_FORK_DUMP
                    Perl_dump_sv_child(aTHX_ sv);
#endif
                }
            }
        }
    }
#ifdef DEBUG_LEAKING_SCALARS_FORK_DUMP
    {
        int status;
        fd_set rset;
        /* Wait for up to 4 seconds for child to terminate.
           This seems to be the least effort way of timing out on reaping
           its exit status.  */
        struct timeval waitfor = {4, 0};
        int sock = PL_dumper_fd;

        shutdown(sock, 1);
        FD_ZERO(&rset);
        FD_SET(sock, &rset);
        select(sock + 1, &rset, NULL, NULL, &waitfor);
        waitpid(child, &status, WNOHANG);
        close(sock);
    }
#endif
#endif
#ifdef DEBUG_LEAKING_SCALARS_ABORT
    if (PL_sv_count)
        abort();
#endif
    PL_sv_count = 0;

#if defined(PERLIO_LAYERS)
    /* No more IO - including error messages ! */
    PerlIO_cleanup(aTHX);
#endif

    /* sv_undef needs to stay immortal until after PerlIO_cleanup
       as currently layers use it rather than NULL as a marker
       for no arg - and will try and SvREFCNT_dec it.
     */
    SvREFCNT(&PL_sv_undef) = 0;
    SvREADONLY_off(&PL_sv_undef);

    Safefree(PL_origfilename);
    PL_origfilename = NULL;
    Safefree(PL_reg_curpm);
    free_tied_hv_pool();
    Safefree(PL_op_mask);
    Safefree(PL_psig_name);
    PL_psig_name = (SV**)NULL;
    PL_psig_ptr = (SV**)NULL;
    {
        /* We need to NULL PL_psig_pend first, so that
           signal handlers know not to use it */
        int *psig_save = PL_psig_pend;
        PL_psig_pend = (int*)NULL;
        Safefree(psig_save);
    }
    nuke_stacks();
    TAINTING_set(FALSE);
    TAINT_WARN_set(FALSE);
    PL_hints = 0;		/* Reset hints. Should hints be per-interpreter ? */

    DEBUG_P(debprofdump());

    PL_debug = 0;

#ifdef USE_REENTRANT_API
    Perl_reentrant_free(aTHX);
#endif

    /* These all point to HVs that are about to be blown away.
       Code in core and on CPAN assumes that if the interpreter is re-started
       that they will be cleanly NULL or pointing to a valid HV.  */
    PL_custom_op_names = NULL;
    PL_custom_op_descs = NULL;
    PL_custom_ops = NULL;

    sv_free_arenas();

    while (PL_regmatch_slab) {
        regmatch_slab  *s = PL_regmatch_slab;
        PL_regmatch_slab = PL_regmatch_slab->next;
        Safefree(s);
    }

    /* As the absolutely last thing, free the non-arena SV for mess() */

    if (PL_mess_sv) {
        /* we know that type == SVt_PVMG */

        /* it could have accumulated taint magic */
        MAGIC* mg;
        MAGIC* moremagic;
        for (mg = SvMAGIC(PL_mess_sv); mg; mg = moremagic) {
            moremagic = mg->mg_moremagic;
            if (mg->mg_ptr && mg->mg_type != PERL_MAGIC_regex_global
                && mg->mg_len >= 0)
                Safefree(mg->mg_ptr);
            Safefree(mg);
        }

        /* we know that type >= SVt_PV */
        SvPV_free(PL_mess_sv);
        Safefree(SvANY(PL_mess_sv));
        Safefree(PL_mess_sv);
        PL_mess_sv = NULL;
    }
    return STATUS_EXIT;
}

/*
=for apidoc perl_free

Releases a Perl interpreter.  See L<perlembed>.

=cut
*/

void
perl_free(pTHXx)
{

    PERL_ARGS_ASSERT_PERL_FREE;

    if (PL_veto_cleanup)
        return;

#ifdef PERL_TRACK_MEMPOOL
    {
        /*
         * Don't free thread memory if PERL_DESTRUCT_LEVEL is set to a non-zero
         * value as we're probably hunting memory leaks then
         */
        if (PL_perl_destruct_level == 0) {
            const U32 old_debug = PL_debug;
            /* Emulate the PerlHost behaviour of free()ing all memory allocated in this
               thread at thread exit.  */
            if (DEBUG_m_TEST) {
                PerlIO_puts(Perl_debug_log, "Disabling memory debugging as we "
                            "free this thread's memory\n");
                PL_debug &= ~ DEBUG_m_FLAG;
            }
            while(aTHXx->Imemory_debug_header.next != &(aTHXx->Imemory_debug_header)){
                char * next = (char *)(aTHXx->Imemory_debug_header.next);
                Malloc_t ptr = PERL_MEMORY_DEBUG_HEADER_SIZE + next;
                safesysfree(ptr);
            }
            PL_debug = old_debug;
        }
    }
#endif

#if defined(WIN32)
#  if defined(PERL_IMPLICIT_SYS)
    {
	void *host = w32_internal_host;
	PerlMem_free(aTHXx);
	win32_delete_internal_host(host);
    }
#  else
    PerlMem_free(aTHXx);
#  endif
#else
    PerlMem_free(aTHXx);
#endif
}

#if defined(USE_ITHREADS)
/* provide destructors to clean up the thread key when libperl is unloaded */
#ifndef WIN32 /* handled during DLL_PROCESS_DETACH in win32/perllib.c */

#if defined(__hpux) && !(defined(__ux_version) && __ux_version <= 1020) && !defined(__GNUC__)
#pragma fini "perl_fini"
#elif defined(__sun) && !defined(__GNUC__)
#pragma fini (perl_fini)
#endif

static void
#if defined(__GNUC__)
__attribute__((destructor))
#endif
perl_fini(void)
{
    if (
        PL_curinterp && !PL_veto_cleanup)
        FREE_THREAD_KEY;
}

#endif /* WIN32 */
#endif /* THREADS */

/*
=for apidoc call_atexit

Add a function C<fn> to the list of functions to be called at global
destruction.  C<ptr> will be passed as an argument to C<fn>; it can point to a
C<struct> so that you can pass anything you want.

Note that under threads, C<fn> may run multiple times.  This is because the
list is executed each time the current or any descendent thread terminates.

=cut
*/

void
Perl_call_atexit(pTHX_ ATEXIT_t fn, void *ptr)
{
    Renew(PL_exitlist, PL_exitlistlen+1, PerlExitListEntry);
    PL_exitlist[PL_exitlistlen].fn = fn;
    PL_exitlist[PL_exitlistlen].ptr = ptr;
    ++PL_exitlistlen;
}

#ifdef USE_ENVIRON_ARRAY
static void
dup_environ(pTHX)
{
#  ifdef USE_ITHREADS
    if (aTHX != PL_curinterp)
        return;
#  endif
    if (!environ)
        return;

    size_t n_entries = 0, vars_size = 0;

    for (char **ep = environ; *ep; ++ep) {
        ++n_entries;
        vars_size += strlen(*ep) + 1;
    }

    /* To save memory, we store both the environ array and its values in a
     * single memory block. */
    char **new_environ = (char**)PerlMemShared_malloc(
        (sizeof(char*) * (n_entries + 1)) + vars_size
    );
    char *vars = (char*)(new_environ + n_entries + 1);

    for (size_t i = 0, copied = 0; n_entries > i; ++i) {
        size_t len = strlen(environ[i]) + 1;
        new_environ[i] = (char *) CopyD(environ[i], vars + copied, len, char);
        copied += len;
    }
    new_environ[n_entries] = NULL;

    environ = new_environ;
    /* Store a pointer in a global variable to ensure it's always reachable so
     * LeakSanitizer/Valgrind won't complain about it. We can't ever free it.
     * Even if libc allocates a new environ, it's possible that some of its
     * values will still be pointing to the old environ.
     */
    PL_my_environ = new_environ;
}
#endif

/*
=for apidoc perl_parse

Tells a Perl interpreter to parse a Perl script.  This performs most
of the initialisation of a Perl interpreter.  See L<perlembed> for
a tutorial.

C<my_perl> points to the Perl interpreter that is to parse the script.
It must have been previously created through the use of L</perl_alloc>
and L</perl_construct>.  C<xsinit> points to a callback function that
will be called to set up the ability for this Perl interpreter to load
XS extensions, or may be null to perform no such setup.

C<argc> and C<argv> supply a set of command-line arguments to the Perl
interpreter, as would normally be passed to the C<main> function of
a C program.  C<argv[argc]> must be null.  These arguments are where
the script to parse is specified, either by naming a script file or by
providing a script in a C<-e> option.
If L<C<$0>|perlvar/$0> will be written to in the Perl interpreter, then
the argument strings must be in writable memory, and so mustn't just be
string constants.

C<env> specifies a set of environment variables that will be used by
this Perl interpreter.  If non-null, it must point to a null-terminated
array of environment strings.  If null, the Perl interpreter will use
the environment supplied by the C<environ> global variable.

This function initialises the interpreter, and parses and compiles the
script specified by the command-line arguments.  This includes executing
code in C<BEGIN>, C<UNITCHECK>, and C<CHECK> blocks.  It does not execute
C<INIT> blocks or the main program.

Returns an integer of slightly tricky interpretation.  The correct
use of the return value is as a truth value indicating whether there
was a failure in initialisation.  If zero is returned, this indicates
that initialisation was successful, and it is safe to proceed to call
L</perl_run> and make other use of it.  If a non-zero value is returned,
this indicates some problem that means the interpreter wants to terminate.
The interpreter should not be just abandoned upon such failure; the caller
should proceed to shut the interpreter down cleanly with L</perl_destruct>
and free it with L</perl_free>.

For historical reasons, the non-zero return value also attempts to
be a suitable value to pass to the C library function C<exit> (or to
return from C<main>), to serve as an exit code indicating the nature
of the way initialisation terminated.  However, this isn't portable,
due to differing exit code conventions.  An attempt is made to return
an exit code of the type required by the host operating system, but
because it is constrained to be non-zero, it is not necessarily possible
to indicate every type of exit.  It is only reliable on Unix, where a
zero exit code can be augmented with a set bit that will be ignored.
In any case, this function is not the correct place to acquire an exit
code: one should get that from L</perl_destruct>.

=cut
*/

#define SET_CURSTASH(newstash)                       \
        if (PL_curstash != newstash) {                \
            SvREFCNT_dec(PL_curstash);                 \
            PL_curstash = (HV *)SvREFCNT_inc(newstash); \
        }

int
perl_parse(pTHXx_ XSINIT_t xsinit, int argc, char **argv, char **env)
{
    I32 oldscope;
    int ret;
    dJMPENV;

    PERL_ARGS_ASSERT_PERL_PARSE;
#ifndef MULTIPLICITY
    PERL_UNUSED_ARG(my_perl);
#endif
    debug_hash_seed(false);
#ifdef __amigaos4__
    {
        struct NameTranslationInfo nti;
        __translate_amiga_to_unix_path_name(&argv[0],&nti);
    }
#endif

    {
        int i;
        assert(argc >= 0);
        for(i = 0; i != argc; i++)
            assert(argv[i]);
        assert(!argv[argc]);
    }
    PL_origargc = argc;
    PL_origargv = argv;

    if (PL_origalen != 0) {
        PL_origalen = 1; /* don't use old PL_origalen if perl_parse() is called again */
    }
    else {
        /* Set PL_origalen be the sum of the contiguous argv[]
         * elements plus the size of the env in case that it is
         * contiguous with the argv[].  This is used in mg.c:Perl_magic_set()
         * as the maximum modifiable length of $0.  In the worst case
         * the area we are able to modify is limited to the size of
         * the original argv[0].  (See below for 'contiguous', though.)
         * --jhi */
         const char *s = NULL;
         const UV mask = ~(UV)(PTRSIZE-1);
         /* Do the mask check only if the args seem like aligned. */
         const UV aligned =
           (mask < ~(UV)0) && ((PTR2UV(argv[0]) & mask) == PTR2UV(argv[0]));

         /* See if all the arguments are contiguous in memory.  Note
          * that 'contiguous' is a loose term because some platforms
          * align the argv[] and the envp[].  If the arguments look
          * like non-aligned, assume that they are 'strictly' or
          * 'traditionally' contiguous.  If the arguments look like
          * aligned, we just check that they are within aligned
          * PTRSIZE bytes.  As long as no system has something bizarre
          * like the argv[] interleaved with some other data, we are
          * fine.  (Did I just evoke Murphy's Law?)  --jhi */
         if (PL_origargv && PL_origargc >= 1 && (s = PL_origargv[0])) {
              int i;
              while (*s) s++;
              for (i = 1; i < PL_origargc; i++) {
                   if ((PL_origargv[i] == s + 1
#ifdef OS2
                        || PL_origargv[i] == s + 2
#endif
                            )
                       ||
                       (aligned &&
                        (PL_origargv[i] >  s &&
                         PL_origargv[i] <=
                         INT2PTR(char *, PTR2UV(s + PTRSIZE) & mask)))
                        )
                   {
                        s = PL_origargv[i];
                        while (*s) s++;
                   }
                   else
                        break;
              }
         }

#ifdef USE_ENVIRON_ARRAY
         /* Can we grab env area too to be used as the area for $0? */
         if (s && PL_origenviron) {
              if ((PL_origenviron[0] == s + 1)
                  ||
                  (aligned &&
                   (PL_origenviron[0] >  s &&
                    PL_origenviron[0] <=
                    INT2PTR(char *, PTR2UV(s + PTRSIZE) & mask)))
                 )
              {
                   int i;
#ifndef OS2		/* ENVIRON is read by the kernel too. */
                   s = PL_origenviron[0];
                   while (*s) s++;
#endif

                   /* Force copy of environment. */
                   if (PL_origenviron == environ)
                       dup_environ(aTHX);

                   for (i = 1; PL_origenviron[i]; i++) {
                        if (PL_origenviron[i] == s + 1
                            ||
                            (aligned &&
                             (PL_origenviron[i] >  s &&
                              PL_origenviron[i] <=
                              INT2PTR(char *, PTR2UV(s + PTRSIZE) & mask)))
                           )
                        {
                             s = PL_origenviron[i];
                             while (*s) s++;
                        }
                        else
                             break;
                   }
              }
         }
#endif /* USE_ENVIRON_ARRAY */

         PL_origalen = s ? s - PL_origargv[0] + 1 : 0;
    }

    if (PL_do_undump) {

        /* Come here if running an undumped a.out. */

        PL_origfilename = savepv(argv[0]);
        PL_do_undump = FALSE;
        cxstack_ix = -1;		/* start label stack again */
        init_ids();
        assert (!TAINT_get);
        TAINT;
        set_caret_X();
        TAINT_NOT;
        init_postdump_symbols(argc,argv,env);
        return 0;
    }

    if (PL_main_root) {
        op_free(PL_main_root);
        PL_main_root = NULL;
    }
    PL_main_start = NULL;
    SvREFCNT_dec(PL_main_cv);
    PL_main_cv = NULL;

    time(&PL_basetime);
    oldscope = PL_scopestack_ix;
    PL_dowarn = G_WARN_OFF;

    JMPENV_PUSH(ret);
    switch (ret) {
    case 0:
        parse_body(env,xsinit);
        if (PL_unitcheckav) {
            call_list(oldscope, PL_unitcheckav);
        }
        if (PL_checkav) {
            PERL_SET_PHASE(PERL_PHASE_CHECK);
            call_list(oldscope, PL_checkav);
        }
        ret = 0;
        break;
    case 1:
        STATUS_ALL_FAILURE;
        /* FALLTHROUGH */
    case 2:
        /* my_exit() was called */
        while (PL_scopestack_ix > oldscope)
            LEAVE;
        FREETMPS;
        SET_CURSTASH(PL_defstash);
        if (PL_unitcheckav) {
            call_list(oldscope, PL_unitcheckav);
        }
        if (PL_checkav) {
            PERL_SET_PHASE(PERL_PHASE_CHECK);
            call_list(oldscope, PL_checkav);
        }
        ret = STATUS_EXIT;
        if (ret == 0) {
            /*
             * We do this here to avoid [perl #2754].
             * Note this may cause trouble with Module::Install.
             * See: [perl #132577].
             */
            ret = 0x100;
        }
        break;
    case 3:
        PerlIO_printf(Perl_error_log, "panic: top_env\n");
        ret = 1;
        break;
    }
    JMPENV_POP;
    return ret;
}

/* This needs to stay in perl.c, as perl.c is compiled with different flags for
   miniperl, and we need to see those flags reflected in the values here.  */

/* What this returns is subject to change.  Use the public interface in Config.
 */

static void
S_Internals_V(pTHX_ CV *cv)
{
    dXSARGS;
#ifdef LOCAL_PATCH_COUNT
    const int local_patch_count = LOCAL_PATCH_COUNT;
#else
    const int local_patch_count = 0;
#endif
    const int entries = 3 + local_patch_count;
    int i;
    /* NOTE - This list must remain sorted. Do not put any settings here
     * which affect binary compatibility */
    static const char non_bincompat_options[] =
#  ifdef DEBUGGING
                             " DEBUGGING"
#  endif
#  ifdef HAS_LONG_DOUBLE
                             " HAS_LONG_DOUBLE"
#  endif
#  ifdef HAS_STRTOLD
                             " HAS_STRTOLD"
#  endif
#  ifdef NO_MATHOMS
                             " NO_MATHOMS"
#  endif
#  ifdef NO_PERL_INTERNAL_RAND_SEED
                             " NO_PERL_INTERNAL_RAND_SEED"
#  endif
#  ifdef NO_PERL_RAND_SEED
                             " NO_PERL_RAND_SEED"
#  endif
#  ifdef NO_TAINT_SUPPORT
                             " NO_TAINT_SUPPORT"
#  endif
#  ifdef PERL_COPY_ON_WRITE
                             " PERL_COPY_ON_WRITE"
#  endif
#  ifdef PERL_DISABLE_PMC
                             " PERL_DISABLE_PMC"
#  endif
#  ifdef PERL_DONT_CREATE_GVSV
                             " PERL_DONT_CREATE_GVSV"
#  endif
#  ifdef PERL_EXTERNAL_GLOB
                             " PERL_EXTERNAL_GLOB"
#  endif
#  ifdef PERL_IS_MINIPERL
                             " PERL_IS_MINIPERL"
#  endif
#  ifdef PERL_MALLOC_WRAP
                             " PERL_MALLOC_WRAP"
#  endif
#  ifdef PERL_MEM_LOG
                             " PERL_MEM_LOG"
#  endif
#  ifdef PERL_MEM_LOG_NOIMPL
                             " PERL_MEM_LOG_NOIMPL"
#  endif
#  ifdef PERL_OP_PARENT
                             " PERL_OP_PARENT"
#  endif
#  ifdef PERL_PERTURB_KEYS_DETERMINISTIC
                             " PERL_PERTURB_KEYS_DETERMINISTIC"
#  endif
#  ifdef PERL_PERTURB_KEYS_DISABLED
                             " PERL_PERTURB_KEYS_DISABLED"
#  endif
#  ifdef PERL_PERTURB_KEYS_RANDOM
                             " PERL_PERTURB_KEYS_RANDOM"
#  endif
#  ifdef PERL_PRESERVE_IVUV
                             " PERL_PRESERVE_IVUV"
#  endif
#  ifdef PERL_RELOCATABLE_INCPUSH
                             " PERL_RELOCATABLE_INCPUSH"
#  endif
#  ifdef PERL_USE_DEVEL
                             " PERL_USE_DEVEL"
#  endif
#  ifdef PERL_USE_SAFE_PUTENV
                             " PERL_USE_SAFE_PUTENV"
#  endif

#  ifdef PERL_USE_UNSHARED_KEYS_IN_LARGE_HASHES
                             " PERL_USE_UNSHARED_KEYS_IN_LARGE_HASHES"
#  endif
#  ifdef SILENT_NO_TAINT_SUPPORT
                             " SILENT_NO_TAINT_SUPPORT"
#  endif
#  ifdef UNLINK_ALL_VERSIONS
                             " UNLINK_ALL_VERSIONS"
#  endif
#  ifdef USE_ATTRIBUTES_FOR_PERLIO
                             " USE_ATTRIBUTES_FOR_PERLIO"
#  endif
#  ifdef USE_FAST_STDIO
                             " USE_FAST_STDIO"
#  endif
#  ifdef USE_LOCALE
                             " USE_LOCALE"
#  endif
#  ifdef USE_LOCALE_CTYPE
                             " USE_LOCALE_CTYPE"
#  endif
#  ifdef WIN32_NO_REGISTRY
                             " USE_NO_REGISTRY"
#  endif
#  ifdef USE_PERL_ATOF
                             " USE_PERL_ATOF"
#  endif
#  ifdef USE_SITECUSTOMIZE
                             " USE_SITECUSTOMIZE"
#  endif
#  ifdef USE_THREAD_SAFE_LOCALE
                             " USE_THREAD_SAFE_LOCALE"
#  endif
    ""; /* keep this on a line by itself, WITH the empty string */

    PERL_UNUSED_ARG(cv);
    PERL_UNUSED_VAR(items);

    EXTEND(SP, entries);

    PUSHs(newSVpvn_flags(PL_bincompat_options, strlen(PL_bincompat_options),
                              SVs_TEMP));
    PUSHs(Perl_newSVpvn_flags(aTHX_ non_bincompat_options,
                              sizeof(non_bincompat_options) - 1, SVs_TEMP));

#ifndef PERL_BUILD_DATE
#  ifdef __DATE__
#    ifdef __TIME__
#      define PERL_BUILD_DATE __DATE__ " " __TIME__
#    else
#      define PERL_BUILD_DATE __DATE__
#    endif
#  endif
#endif

#ifdef PERL_BUILD_DATE
    PUSHs(Perl_newSVpvn_flags(aTHX_
                              STR_WITH_LEN("Compiled at " PERL_BUILD_DATE),
                              SVs_TEMP));
#else
    PUSHs(&PL_sv_undef);
#endif

    for (i = 1; i <= local_patch_count; i++) {
        /* This will be an undef, if PL_localpatches[i] is NULL.  */
        PUSHs(newSVpvn_flags(PL_localpatches[i],
            PL_localpatches[i] == NULL ? 0 : strlen(PL_localpatches[i]),
            SVs_TEMP));
    }

    XSRETURN(entries);
}

#define INCPUSH_UNSHIFT			0x01
#define INCPUSH_ADD_OLD_VERS		0x02
#define INCPUSH_ADD_VERSIONED_SUB_DIRS	0x04
#define INCPUSH_ADD_ARCHONLY_SUB_DIRS	0x08
#define INCPUSH_NOT_BASEDIR		0x10
#define INCPUSH_CAN_RELOCATE		0x20
#define INCPUSH_ADD_SUB_DIRS	\
    (INCPUSH_ADD_VERSIONED_SUB_DIRS|INCPUSH_ADD_ARCHONLY_SUB_DIRS)

STATIC void *
S_parse_body(pTHX_ char **env, XSINIT_t xsinit)
{
    PerlIO *rsfp;
    int argc = PL_origargc;
    char **argv = PL_origargv;
    const char *scriptname = NULL;
    bool dosearch = FALSE;
    char c;
    bool doextract = FALSE;
    const char *cddir = NULL;
    bool minus_e = FALSE; /* both -e and -E */
#ifdef USE_SITECUSTOMIZE
    bool minus_f = FALSE;
#endif
    SV *linestr_sv = NULL;
    bool add_read_e_script = FALSE;
    U32 lex_start_flags = 0;

    PERL_SET_PHASE(PERL_PHASE_START);

    init_main_stash();

    {
        const char *s;
    for (argc--,argv++; argc > 0; argc--,argv++) {
        if (argv[0][0] != '-' || !argv[0][1])
            break;
        s = argv[0]+1;
      reswitch:
        switch ((c = *s)) {
        case 'C':
#ifndef PERL_STRICT_CR
        case '\r':
#endif
        case ' ':
        case '0':
        case 'F':
        case 'a':
        case 'c':
        case 'd':
        case 'D':
        case 'g':
        case '?':
        case 'h':
        case 'i':
        case 'l':
        case 'M':
        case 'm':
        case 'n':
        case 'p':
        case 's':
        case 'u':
        case 'U':
        case 'v':
        case 'W':
        case 'X':
        case 'w':
            if ((s = moreswitches(s)))
                goto reswitch;
            break;

        case 't':
#if defined(SILENT_NO_TAINT_SUPPORT)
            /* silently ignore */
#elif defined(NO_TAINT_SUPPORT)
            Perl_croak_nocontext("This perl was compiled without taint support. "
                       "Cowardly refusing to run with -t or -T flags");
#else
            CHECK_MALLOC_TOO_LATE_FOR('t');
            if( !TAINTING_get ) {
                 TAINT_WARN_set(TRUE);
                 TAINTING_set(TRUE);
            }
#endif
            s++;
            goto reswitch;
        case 'T':
#if defined(SILENT_NO_TAINT_SUPPORT)
            /* silently ignore */
#elif defined(NO_TAINT_SUPPORT)
            Perl_croak_nocontext("This perl was compiled without taint support. "
                       "Cowardly refusing to run with -t or -T flags");
#else
            CHECK_MALLOC_TOO_LATE_FOR('T');
            TAINTING_set(TRUE);
            TAINT_WARN_set(FALSE);
#endif
            s++;
            goto reswitch;

        case 'E':
            PL_minus_E = TRUE;
            /* FALLTHROUGH */
        case 'e':
            forbid_setid('e', FALSE);
        minus_e = TRUE;
            if (!PL_e_script) {
                PL_e_script = newSVpvs("");
                add_read_e_script = TRUE;
            }
            if (*++s)
                sv_catpv(PL_e_script, s);
            else if (argv[1]) {
                sv_catpv(PL_e_script, argv[1]);
                argc--,argv++;
            }
            else
                Perl_croak(aTHX_ "No code specified for -%c", c);
            sv_catpvs(PL_e_script, "\n");
            break;

        case 'f':
#ifdef USE_SITECUSTOMIZE
            minus_f = TRUE;
#endif
            s++;
            goto reswitch;

        case 'I':	/* -I handled both here and in moreswitches() */
            forbid_setid('I', FALSE);
            if (!*++s && (s=argv[1]) != NULL) {
                argc--,argv++;
            }
            if (s && *s) {
                STRLEN len = strlen(s);
                incpush(s, len, INCPUSH_ADD_SUB_DIRS|INCPUSH_ADD_OLD_VERS);
            }
            else
                Perl_croak(aTHX_ "No directory specified for -I");
            break;
        case 'S':
            forbid_setid('S', FALSE);
            dosearch = TRUE;
            s++;
            goto reswitch;
        case 'V':
            {
                SV *opts_prog;

                if (*++s != ':')  {
                    opts_prog = newSVpvs("use Config; Config::_V()");
                }
                else {
                    ++s;
                    opts_prog = Perl_newSVpvf(aTHX_
                                              "use Config; Config::config_vars(qw%c%s%c)",
                                              0, s, 0);
                    s += strlen(s);
                }
                Perl_av_create_and_push(aTHX_ &PL_preambleav, opts_prog);
                /* don't look for script or read stdin */
                scriptname = BIT_BUCKET;
                goto reswitch;
            }
        case 'x':
            doextract = TRUE;
            s++;
            if (*s)
                cddir = s;
            break;
        case 0:
            break;
        case '-':
            if (!*++s || isSPACE(*s)) {
                argc--,argv++;
                goto switch_end;
            }
            /* catch use of gnu style long options.
               Both of these exit immediately.  */
            if (strEQ(s, "version"))
                minus_v();
            if (strEQ(s, "help"))
                usage();
            s--;
            /* FALLTHROUGH */
        default:
            Perl_croak(aTHX_ "Unrecognized switch: -%s  (-h will show valid options)",s);
        }
    }
    }

  switch_end:

    {
        char *s;

    if (
#ifndef SECURE_INTERNAL_GETENV
        !TAINTING_get &&
#endif
        (s = PerlEnv_getenv("PERL5OPT")))
    {
        while (isSPACE(*s))
            s++;
        if (*s == '-' && *(s+1) == 'T') {
#if defined(SILENT_NO_TAINT_SUPPORT)
            /* silently ignore */
#elif defined(NO_TAINT_SUPPORT)
            Perl_croak_nocontext("This perl was compiled without taint support. "
                       "Cowardly refusing to run with -t or -T flags");
#else
            CHECK_MALLOC_TOO_LATE_FOR('T');
            TAINTING_set(TRUE);
            TAINT_WARN_set(FALSE);
#endif
        }
        else {
            char *popt_copy = NULL;
            while (s && *s) {
                const char *d;
                while (isSPACE(*s))
                    s++;
                if (*s == '-') {
                    s++;
                    if (isSPACE(*s))
                        continue;
                }
                d = s;
                if (!*s)
                    break;
                if (!memCHRs("CDIMUdmtwW", *s))
                    Perl_croak(aTHX_ "Illegal switch in PERL5OPT: -%c", *s);
                while (++s && *s) {
                    if (isSPACE(*s)) {
                        if (!popt_copy) {
                            popt_copy = SvPVX(newSVpvn_flags(d, strlen(d), SVs_TEMP));
                            s = popt_copy + (s - d);
                            d = popt_copy;
                        }
                        *s++ = '\0';
                        break;
                    }
                }
                if (*d == 't') {
#if defined(SILENT_NO_TAINT_SUPPORT)
            /* silently ignore */
#elif defined(NO_TAINT_SUPPORT)
                    Perl_croak_nocontext("This perl was compiled without taint support. "
                               "Cowardly refusing to run with -t or -T flags");
#else
                    if( !TAINTING_get) {
                        TAINT_WARN_set(TRUE);
                        TAINTING_set(TRUE);
                    }
#endif
                } else {
                    moreswitches(d);
                }
            }
        }
    }
    }

#ifndef NO_PERL_INTERNAL_RAND_SEED
    /* If we're not set[ug]id, we might have honored
       PERL_INTERNAL_RAND_SEED in perl_construct().
       At this point command-line options have been parsed, so if
       we're now tainting and not set[ug]id re-seed.
       This could possibly be wasteful if PERL_INTERNAL_RAND_SEED is invalid,
       but avoids duplicating the logic from perl_construct().
    */
    if (TAINT_get &&
        PerlProc_getuid() == PerlProc_geteuid() &&
        PerlProc_getgid() == PerlProc_getegid()) {
        Perl_drand48_init_r(&PL_internal_random_state, seed());
    }
#endif
    if (DEBUG_h_TEST)
        debug_hash_seed(true);

    /* Set $^X early so that it can be used for relocatable paths in @INC  */
    /* and for SITELIB_EXP in USE_SITECUSTOMIZE                            */
    assert (!TAINT_get);
    TAINT;
    set_caret_X();
    TAINT_NOT;

#if defined(USE_SITECUSTOMIZE)
    if (!minus_f) {
        /* The games with local $! are to avoid setting errno if there is no
           sitecustomize script.  "q%c...%c", 0, ..., 0 becomes "q\0...\0",
           ie a q() operator with a NUL byte as a the delimiter. This avoids
           problems with pathnames containing (say) '  */
#  ifdef PERL_IS_MINIPERL
        AV *const inc = GvAV(PL_incgv);
        SV **const inc0 = inc ? av_fetch(inc, 0, FALSE) : NULL;

        if (inc0) {
            /* if lib/buildcustomize.pl exists, it should not fail. If it does,
               it should be reported immediately as a build failure.  */
            (void)Perl_av_create_and_unshift_one(aTHX_ &PL_preambleav,
                                                 Perl_newSVpvf(aTHX_
                "BEGIN { my $f = q%c%s%" SVf "/buildcustomize.pl%c; "
                        "do {local $!; -f $f }"
                        " and do $f || die $@ || qq '$f: $!' }",
                                0, (TAINTING_get ? "./" : ""), SVfARG(*inc0), 0));
        }
#  else
        /* SITELIB_EXP is a function call on Win32.  */
        const char *const raw_sitelib = SITELIB_EXP;
        if (raw_sitelib) {
            /* process .../.. if PERL_RELOCATABLE_INC is defined */
            SV *sitelib_sv = mayberelocate(raw_sitelib, strlen(raw_sitelib),
                                           INCPUSH_CAN_RELOCATE);
            const char *const sitelib = SvPVX(sitelib_sv);
            (void)Perl_av_create_and_unshift_one(aTHX_ &PL_preambleav,
                                                 Perl_newSVpvf(aTHX_
                                                               "BEGIN { do {local $!; -f q%c%s/sitecustomize.pl%c} && do q%c%s/sitecustomize.pl%c }",
                                                               0, sitelib, 0,
                                                               0, sitelib, 0));
            assert (SvREFCNT(sitelib_sv) == 1);
            SvREFCNT_dec(sitelib_sv);
        }
#  endif
    }
#endif

    if (!scriptname)
        scriptname = argv[0];
    if (PL_e_script) {
        argc++,argv--;
        scriptname = BIT_BUCKET;	/* don't look for script or read stdin */
    }
    else if (scriptname == NULL) {
        scriptname = "-";
    }

    assert (!TAINT_get);
    init_perllib();

    {
        bool suidscript = FALSE;

        rsfp = open_script(scriptname, dosearch, &suidscript);
        if (!rsfp) {
            rsfp = PerlIO_stdin();
            lex_start_flags = LEX_DONT_CLOSE_RSFP;
        }

        validate_suid(rsfp);

#ifndef PERL_MICRO
#  if defined(SIGCHLD) || defined(SIGCLD)
        {
#  ifndef SIGCHLD
#    define SIGCHLD SIGCLD
#  endif
            Sighandler_t sigstate = rsignal_state(SIGCHLD);
            if (sigstate == (Sighandler_t) SIG_IGN) {
                Perl_ck_warner(aTHX_ packWARN(WARN_SIGNAL),
                               "Can't ignore signal CHLD, forcing to default");
                (void)rsignal(SIGCHLD, (Sighandler_t)SIG_DFL);
            }
        }
#  endif
#endif

        if (doextract) {

            /* This will croak if suidscript is true, as -x cannot be used with
               setuid scripts.  */
            forbid_setid('x', suidscript);
            /* Hence you can't get here if suidscript is true */

            linestr_sv = newSV_type(SVt_PV);
            lex_start_flags |= LEX_START_COPIED;
            find_beginning(linestr_sv, rsfp);
            if (cddir && PerlDir_chdir( (char *)cddir ) < 0)
                Perl_croak(aTHX_ "Can't chdir to %s",cddir);
        }
    }

    PL_main_cv = PL_compcv = MUTABLE_CV(newSV_type(SVt_PVCV));
    CvUNIQUE_on(PL_compcv);

    CvPADLIST_set(PL_compcv, pad_new(0));

    PL_isarev = newHV();

    boot_core_PerlIO();
    boot_core_UNIVERSAL();
    boot_core_builtin();
    boot_core_mro();
    newXS("Internals::V", S_Internals_V, __FILE__);

    if (xsinit)
        (*xsinit)(aTHX);	/* in case linked C routines want magical variables */
#ifndef PERL_MICRO
#if defined(VMS) || defined(WIN32) || defined(__CYGWIN__)
    init_os_extras();
#endif
#endif

#ifdef USE_SOCKS
#   ifdef HAS_SOCKS5_INIT
    socks5_init(argv[0]);
#   else
    SOCKSinit(argv[0]);
#   endif
#endif

    init_predump_symbols();
    /* init_postdump_symbols not currently designed to be called */
    /* more than once (ENV isn't cleared first, for example)	 */
    /* But running with -u leaves %ENV & @ARGV undefined!    XXX */
    if (!PL_do_undump)
        init_postdump_symbols(argc,argv,env);

    /* PL_unicode is turned on by -C, or by $ENV{PERL_UNICODE},
     * or explicitly in some platforms.
     * PL_utf8locale is conditionally turned on by
     * locale.c:Perl_init_i18nl10n() if the environment
     * look like the user wants to use UTF-8. */
#  ifndef PERL_IS_MINIPERL
    if (PL_unicode) {
         /* Requires init_predump_symbols(). */
         if (!(PL_unicode & PERL_UNICODE_LOCALE_FLAG) || PL_utf8locale) {
              IO* io;
              PerlIO* fp;
              SV* sv;

              /* Turn on UTF-8-ness on STDIN, STDOUT, STDERR
               * and the default open disciplines. */
              if ((PL_unicode & PERL_UNICODE_STDIN_FLAG) &&
                  PL_stdingv  && (io = GvIO(PL_stdingv)) &&
                  (fp = IoIFP(io)))
                   PerlIO_binmode(aTHX_ fp, IoTYPE(io), 0, ":utf8");
              if ((PL_unicode & PERL_UNICODE_STDOUT_FLAG) &&
                  PL_defoutgv && (io = GvIO(PL_defoutgv)) &&
                  (fp = IoOFP(io)))
                   PerlIO_binmode(aTHX_ fp, IoTYPE(io), 0, ":utf8");
              if ((PL_unicode & PERL_UNICODE_STDERR_FLAG) &&
                  PL_stderrgv && (io = GvIO(PL_stderrgv)) &&
                  (fp = IoOFP(io)))
                   PerlIO_binmode(aTHX_ fp, IoTYPE(io), 0, ":utf8");
              if ((PL_unicode & PERL_UNICODE_INOUT_FLAG) &&
                  (sv = GvSV(gv_fetchpvs("\017PEN", GV_ADD|GV_NOTQUAL,
                                         SVt_PV)))) {
                   U32 in  = PL_unicode & PERL_UNICODE_IN_FLAG;
                   U32 out = PL_unicode & PERL_UNICODE_OUT_FLAG;
                   if (in) {
                        if (out)
                             sv_setpvs(sv, ":utf8\0:utf8");
                        else
                             sv_setpvs(sv, ":utf8\0");
                   }
                   else if (out)
                        sv_setpvs(sv, "\0:utf8");
                   SvSETMAGIC(sv);
              }
         }
    }
#endif

    {
        const char *s;
    if ((s = PerlEnv_getenv("PERL_SIGNALS"))) {
         if (strEQ(s, "unsafe"))
              PL_signals |=  PERL_SIGNALS_UNSAFE_FLAG;
         else if (strEQ(s, "safe"))
              PL_signals &= ~PERL_SIGNALS_UNSAFE_FLAG;
         else
              Perl_croak(aTHX_ "PERL_SIGNALS illegal: \"%s\"", s);
    }
    }


    lex_start(linestr_sv, rsfp, lex_start_flags);
    SvREFCNT_dec(linestr_sv);

    PL_subname = newSVpvs("main");

    if (add_read_e_script)
        filter_add(read_e_script, NULL);

    /* now parse the script */
    if (minus_e == FALSE)
        PL_hints |= HINTS_DEFAULT; /* after init_main_stash ; need to be after init_predump_symbols */

    SETERRNO(0,SS_NORMAL);
    if (yyparse(GRAMPROG) || PL_parser->error_count) {
        abort_execution(NULL, PL_origfilename);
    }
    CopLINE_set(PL_curcop, 0);
    SET_CURSTASH(PL_defstash);
    if (PL_e_script) {
        SvREFCNT_dec(PL_e_script);
        PL_e_script = NULL;
    }

    if (PL_do_undump)
        my_unexec();

    if (isWARN_ONCE) {
        SAVECOPFILE(PL_curcop);
        SAVECOPLINE(PL_curcop);
        gv_check(PL_defstash);
    }

    LEAVE;
    FREETMPS;

#ifdef MYMALLOC
    {
        const char *s;
        UV uv;
        s = PerlEnv_getenv("PERL_DEBUG_MSTATS");
        if (s && grok_atoUV(s, &uv, NULL) && uv >= 2)
            dump_mstats("after compilation:");
    }
#endif

    ENTER;
    PL_restartjmpenv = NULL;
    PL_restartop = 0;
    return NULL;
}

/*
=for apidoc perl_run

Tells a Perl interpreter to run its main program.  See L<perlembed>
for a tutorial.

C<my_perl> points to the Perl interpreter.  It must have been previously
created through the use of L</perl_alloc> and L</perl_construct>, and
initialised through L</perl_parse>.  This function should not be called
if L</perl_parse> returned a non-zero value, indicating a failure in
initialisation or compilation.

This function executes code in C<INIT> blocks, and then executes the
main program.  The code to be executed is that established by the prior
call to L</perl_parse>.  If the interpreter's C<PL_exit_flags> word
does not have the C<PERL_EXIT_DESTRUCT_END> flag set, then this function
will also execute code in C<END> blocks.  If it is desired to make any
further use of the interpreter after calling this function, then C<END>
blocks should be postponed to L</perl_destruct> time by setting that flag.

Returns an integer of slightly tricky interpretation.  The correct use
of the return value is as a truth value indicating whether the program
terminated non-locally.  If zero is returned, this indicates that
the program ran to completion, and it is safe to make other use of the
interpreter (provided that the C<PERL_EXIT_DESTRUCT_END> flag was set as
described above).  If a non-zero value is returned, this indicates that
the interpreter wants to terminate early.  The interpreter should not be
just abandoned because of this desire to terminate; the caller should
proceed to shut the interpreter down cleanly with L</perl_destruct>
and free it with L</perl_free>.

For historical reasons, the non-zero return value also attempts to
be a suitable value to pass to the C library function C<exit> (or to
return from C<main>), to serve as an exit code indicating the nature of
the way the program terminated.  However, this isn't portable, due to
differing exit code conventions.  An attempt is made to return an exit
code of the type required by the host operating system, but because
it is constrained to be non-zero, it is not necessarily possible to
indicate every type of exit.  It is only reliable on Unix, where a zero
exit code can be augmented with a set bit that will be ignored.  In any
case, this function is not the correct place to acquire an exit code:
one should get that from L</perl_destruct>.

=cut
*/

int
perl_run(pTHXx)
{
    I32 oldscope;
    int ret = 0;
    dJMPENV;

    PERL_ARGS_ASSERT_PERL_RUN;
#ifndef MULTIPLICITY
    PERL_UNUSED_ARG(my_perl);
#endif

    oldscope = PL_scopestack_ix;
#ifdef VMS
    VMSISH_HUSHED = 0;
#endif

    JMPENV_PUSH(ret);
    switch (ret) {
    case 1:
        cxstack_ix = -1;		/* start context stack again */
        goto redo_body;
    case 0:				/* normal completion */
 redo_body:
        run_body(oldscope);
        /* FALLTHROUGH */
    case 2:				/* my_exit() */
        while (PL_scopestack_ix > oldscope)
            LEAVE;
        FREETMPS;
        SET_CURSTASH(PL_defstash);
        if (!(PL_exit_flags & PERL_EXIT_DESTRUCT_END) &&
            PL_endav && !PL_minus_c) {
            PERL_SET_PHASE(PERL_PHASE_END);
            call_list(oldscope, PL_endav);
        }
#ifdef MYMALLOC
        if (PerlEnv_getenv("PERL_DEBUG_MSTATS"))
            dump_mstats("after execution:  ");
#endif
        ret = STATUS_EXIT;
        break;
    case 3:
        if (PL_restartop) {
            POPSTACK_TO(PL_mainstack);
            goto redo_body;
        }
        PerlIO_printf(Perl_error_log, "panic: restartop in perl_run\n");
        FREETMPS;
        ret = 1;
        break;
    }

    JMPENV_POP;
    return ret;
}

STATIC void
S_run_body(pTHX_ I32 oldscope)
{
    DEBUG_r(PerlIO_printf(Perl_debug_log, "%s $` $& $' support (0x%x).\n",
                    PL_sawampersand ? "Enabling" : "Omitting",
                    (unsigned int)(PL_sawampersand)));

    if (!PL_restartop) {
#ifdef DEBUGGING
        if (DEBUG_x_TEST || DEBUG_B_TEST)
            dump_all_perl(!DEBUG_B_TEST);
        if (!DEBUG_q_TEST)
          PERL_DEBUG(PerlIO_printf(Perl_debug_log, "\nEXECUTING...\n\n"));
#endif

        if (PL_minus_c) {
            PerlIO_printf(Perl_error_log, "%s syntax OK\n", PL_origfilename);
            my_exit(0);
        }
        if (PERLDB_SINGLE && PL_DBsingle)
            PL_DBsingle_iv = 1;
        if (PL_initav) {
            PERL_SET_PHASE(PERL_PHASE_INIT);
            call_list(oldscope, PL_initav);
        }
#ifdef PERL_DEBUG_READONLY_OPS
        if (PL_main_root && PL_main_root->op_slabbed)
            Slab_to_ro(OpSLAB(PL_main_root));
#endif
    }

    /* do it */

    PERL_SET_PHASE(PERL_PHASE_RUN);

    if (PL_restartop) {
#ifdef DEBUGGING
        /* this complements the "EXECUTING..." debug we emit above.
         * it will show up when an eval fails in the main program level
         * and the code continues after the error.
         */
        if (!DEBUG_q_TEST)
          PERL_DEBUG(PerlIO_printf(Perl_debug_log, "\nCONTINUING...\n\n"));
#endif
        PL_restartjmpenv = NULL;
        PL_op = PL_restartop;
        PL_restartop = 0;
        CALLRUNOPS(aTHX);
    }
    else if (PL_main_start) {
        CvDEPTH(PL_main_cv) = 1;
        PL_op = PL_main_start;
        CALLRUNOPS(aTHX);
    }
    my_exit(0);
    NOT_REACHED; /* NOTREACHED */
}

/*
=for apidoc_section $SV

=for apidoc get_sv

Returns the SV of the specified Perl scalar.  C<flags> are passed to
L</C<gv_fetchpv>>.  If C<GV_ADD> is set and the
Perl variable does not exist then it will be created.  If C<flags> is zero
and the variable does not exist then NULL is returned.

=cut
*/

SV*
Perl_get_sv(pTHX_ const char *name, I32 flags)
{
    GV *gv;

    PERL_ARGS_ASSERT_GET_SV;

    gv = gv_fetchpv(name, flags, SVt_PV);
    if (gv)
        return GvSV(gv);
    return NULL;
}

/*
=for apidoc_section $AV

=for apidoc get_av

Returns the AV of the specified Perl global or package array with the given
name (so it won't work on lexical variables).  C<flags> are passed
to C<gv_fetchpv>.  If C<GV_ADD> is set and the
Perl variable does not exist then it will be created.  If C<flags> is zero
(ignoring C<SVf_UTF8>) and the variable does not exist then C<NULL> is
returned.

Perl equivalent: C<@{"$name"}>.

=cut
*/

AV*
Perl_get_av(pTHX_ const char *name, I32 flags)
{
    GV* const gv = gv_fetchpv(name, flags, SVt_PVAV);

    PERL_ARGS_ASSERT_GET_AV;

    if (flags & ~SVf_UTF8)
        return GvAVn(gv);
    if (gv)
        return GvAV(gv);
    return NULL;
}

/*
=for apidoc_section $HV

=for apidoc get_hv

Returns the HV of the specified Perl hash.  C<flags> are passed to
C<gv_fetchpv>.  If C<GV_ADD> is set and the
Perl variable does not exist then it will be created.  If C<flags> is zero
(ignoring C<SVf_UTF8>) and the variable does not exist then C<NULL> is
returned.

=cut
*/

HV*
Perl_get_hv(pTHX_ const char *name, I32 flags)
{
    GV* const gv = gv_fetchpv(name, flags, SVt_PVHV);

    PERL_ARGS_ASSERT_GET_HV;

    if (flags & ~SVf_UTF8)
        return GvHVn(gv);
    if (gv)
        return GvHV(gv);
    return NULL;
}

/*
=for apidoc_section $CV

=for apidoc            get_cv
=for apidoc_item       get_cvn_flags
=for apidoc_item |CV *|get_cvs|"string"|I32 flags

These return the CV of the specified Perl subroutine.  C<flags> are passed to
C<gv_fetchpvn_flags>.  If C<GV_ADD> is set and the Perl subroutine does not
exist then it will be declared (which has the same effect as saying
C<sub name;>).  If C<GV_ADD> is not set and the subroutine does not exist,
then NULL is returned.

The forms differ only in how the subroutine is specified..  With C<get_cvs>,
the name is a literal C string, enclosed in double quotes.  With C<get_cv>, the
name is given by the C<name> parameter, which must be a NUL-terminated C
string.  With C<get_cvn_flags>, the name is also given by the C<name>
parameter, but it is a Perl string (possibly containing embedded NUL bytes),
and its length in bytes is contained in the C<len> parameter.

=for apidoc Amnh||GV_ADD

=cut
*/

CV*
Perl_get_cvn_flags(pTHX_ const char *name, STRLEN len, I32 flags)
{
    GV* const gv = gv_fetchpvn_flags(name, len, flags, SVt_PVCV);

    PERL_ARGS_ASSERT_GET_CVN_FLAGS;

    if (gv && UNLIKELY(SvROK(gv)) && SvTYPE(SvRV((SV *)gv)) == SVt_PVCV)
        return (CV*)SvRV((SV *)gv);

    /* XXX this is probably not what they think they're getting.
     * It has the same effect as "sub name;", i.e. just a forward
     * declaration! */
    if ((flags & ~GV_NOADD_MASK) && !GvCVu(gv)) {
        return newSTUB(gv,0);
    }
    if (gv)
        return GvCVu(gv);
    return NULL;
}

/* Nothing in core calls this now, but we can't replace it with a macro and
   move it to mathoms.c as a macro would evaluate name twice.  */
CV*
Perl_get_cv(pTHX_ const char *name, I32 flags)
{
    PERL_ARGS_ASSERT_GET_CV;

    return get_cvn_flags(name, strlen(name), flags);
}

/* Be sure to refetch the stack pointer after calling these routines. */

/*

=for apidoc_section $callback

=for apidoc call_argv

Performs a callback to the specified named and package-scoped Perl subroutine
with C<argv> (a C<NULL>-terminated array of strings) as arguments.  See
L<perlcall>.

Approximate Perl equivalent: C<&{"$sub_name"}(@$argv)>.

=cut
*/

I32
Perl_call_argv(pTHX_ const char *sub_name, I32 flags, char **argv)

                        /* See G_* flags in cop.h */
                        /* null terminated arg list */
{
    dSP;

    PERL_ARGS_ASSERT_CALL_ARGV;

    PUSHMARK(SP);
    while (*argv) {
        mXPUSHs(newSVpv(*argv,0));
        argv++;
    }
    PUTBACK;
    return call_pv(sub_name, flags);
}

/*
=for apidoc call_pv

Performs a callback to the specified Perl sub.  See L<perlcall>.

=cut
*/

I32
Perl_call_pv(pTHX_ const char *sub_name, I32 flags)
                        /* name of the subroutine */
                        /* See G_* flags in cop.h */
{
    PERL_ARGS_ASSERT_CALL_PV;

    return call_sv(MUTABLE_SV(get_cv(sub_name, GV_ADD)), flags);
}

/*
=for apidoc call_method

Performs a callback to the specified Perl method.  The blessed object must
be on the stack.  See L<perlcall>.

=cut
*/

I32
Perl_call_method(pTHX_ const char *methname, I32 flags)
                        /* name of the subroutine */
                        /* See G_* flags in cop.h */
{
    STRLEN len;
    SV* sv;
    PERL_ARGS_ASSERT_CALL_METHOD;

    len = strlen(methname);
    sv = flags & G_METHOD_NAMED
        ? sv_2mortal(newSVpvn_share(methname, len,0))
        : newSVpvn_flags(methname, len, SVs_TEMP);

    return call_sv(sv, flags | G_METHOD);
}

/* May be called with any of a CV, a GV, or an SV containing the name. */
/*
=for apidoc call_sv

Performs a callback to the Perl sub specified by the SV.

If neither the C<G_METHOD> nor C<G_METHOD_NAMED> flag is supplied, the
SV may be any of a CV, a GV, a reference to a CV, a reference to a GV
or C<SvPV(sv)> will be used as the name of the sub to call.

If the C<G_METHOD> flag is supplied, the SV may be a reference to a CV or
C<SvPV(sv)> will be used as the name of the method to call.

If the C<G_METHOD_NAMED> flag is supplied, C<SvPV(sv)> will be used as
the name of the method to call.

Some other values are treated specially for internal use and should
not be depended on.

See L<perlcall>.

=for apidoc Amnh||G_METHOD
=for apidoc Amnh||G_METHOD_NAMED

=cut
*/

I32
Perl_call_sv(pTHX_ SV *sv, volatile I32 flags)
                        /* See G_* flags in cop.h */
{
    LOGOP myop;		/* fake syntax tree node */
    METHOP method_op;
    I32 oldmark;
    volatile I32 retval = 0;
    bool oldcatch = CATCH_GET;
    int ret;
    OP* const oldop = PL_op;
    dJMPENV;

    PERL_ARGS_ASSERT_CALL_SV;

    if (flags & G_DISCARD) {
        ENTER;
        SAVETMPS;
    }
    if (!(flags & G_WANT)) {
        /* Backwards compatibility - as G_SCALAR was 0, it could be omitted.
         */
        flags |= G_SCALAR;
    }

    Zero(&myop, 1, LOGOP);
    if (!(flags & G_NOARGS))
        myop.op_flags |= OPf_STACKED;
    myop.op_flags |= OP_GIMME_REVERSE(flags);
    myop.op_ppaddr = PL_ppaddr[OP_ENTERSUB];
    myop.op_type = OP_ENTERSUB;
    SAVEOP();
    PL_op = (OP*)&myop;

    if (!(flags & G_METHOD_NAMED)) {
        dSP;
        EXTEND(SP, 1);
        PUSHs(sv);
        PUTBACK;
    }
    oldmark = TOPMARK;

    if (PERLDB_SUB && PL_curstash != PL_debstash
           /* Handle first BEGIN of -d. */
          && (PL_DBcv || (PL_DBcv = GvCV(PL_DBsub)))
           /* Try harder, since this may have been a sighandler, thus
            * curstash may be meaningless. */
          && (SvTYPE(sv) != SVt_PVCV || CvSTASH((const CV *)sv) != PL_debstash)
          && !(flags & G_NODEBUG))
        myop.op_private |= OPpENTERSUB_DB;

    if (flags & (G_METHOD|G_METHOD_NAMED)) {
        Zero(&method_op, 1, METHOP);
        method_op.op_next = (OP*)&myop;
        PL_op = (OP*)&method_op;
        if ( flags & G_METHOD_NAMED ) {
            method_op.op_ppaddr = PL_ppaddr[OP_METHOD_NAMED];
            method_op.op_type = OP_METHOD_NAMED;
            method_op.op_u.op_meth_sv = sv;
        } else {
            method_op.op_ppaddr = PL_ppaddr[OP_METHOD];
            method_op.op_type = OP_METHOD;
        }
    }

    if (!(flags & G_EVAL)) {
        CATCH_SET(TRUE);
        CALLRUNOPS(aTHX);
        retval = PL_stack_sp - (PL_stack_base + oldmark);
        CATCH_SET(oldcatch);
    }
    else {
        I32 old_cxix;
        myop.op_other = (OP*)&myop;
        (void)POPMARK;
        old_cxix = cxstack_ix;
        create_eval_scope(NULL, flags|G_FAKINGEVAL);
        INCMARK;

        JMPENV_PUSH(ret);

        switch (ret) {
        case 0:
 redo_body:
            CALLRUNOPS(aTHX);
            retval = PL_stack_sp - (PL_stack_base + oldmark);
            if (!(flags & G_KEEPERR)) {
                CLEAR_ERRSV();
            }
            break;
        case 1:
            STATUS_ALL_FAILURE;
            /* FALLTHROUGH */
        case 2:
            /* my_exit() was called */
            SET_CURSTASH(PL_defstash);
            FREETMPS;
            JMPENV_POP;
            my_exit_jump();
            NOT_REACHED; /* NOTREACHED */
        case 3:
            if (PL_restartop) {
                PL_restartjmpenv = NULL;
                PL_op = PL_restartop;
                PL_restartop = 0;
                goto redo_body;
            }
            PL_stack_sp = PL_stack_base + oldmark;
            if ((flags & G_WANT) == G_LIST)
                retval = 0;
            else {
                retval = 1;
                *++PL_stack_sp = &PL_sv_undef;
            }
            break;
        }

        /* if we croaked, depending on how we croaked the eval scope
         * may or may not have already been popped */
        if (cxstack_ix > old_cxix) {
            assert(cxstack_ix == old_cxix + 1);
            assert(CxTYPE(CX_CUR()) == CXt_EVAL);
            delete_eval_scope();
        }
        JMPENV_POP;
    }

    if (flags & G_DISCARD) {
        PL_stack_sp = PL_stack_base + oldmark;
        retval = 0;
        FREETMPS;
        LEAVE;
    }
    PL_op = oldop;
    return retval;
}

/* Eval a string. The G_EVAL flag is always assumed. */

/*
=for apidoc eval_sv

Tells Perl to C<eval> the string in the SV.  It supports the same flags
as C<call_sv>, with the obvious exception of C<G_EVAL>.  See L<perlcall>.

The C<G_RETHROW> flag can be used if you only need eval_sv() to
execute code specified by a string, but not catch any errors.

=for apidoc Amnh||G_RETHROW
=cut
*/

I32
Perl_eval_sv(pTHX_ SV *sv, I32 flags)

                        /* See G_* flags in cop.h */
{
    UNOP myop;		/* fake syntax tree node */
    volatile I32 oldmark;
    volatile I32 retval = 0;
    int ret;
    OP* const oldop = PL_op;
    dJMPENV;

    PERL_ARGS_ASSERT_EVAL_SV;

    if (flags & G_DISCARD) {
        ENTER;
        SAVETMPS;
    }

    SAVEOP();
    PL_op = (OP*)&myop;
    Zero(&myop, 1, UNOP);
    myop.op_ppaddr = PL_ppaddr[OP_ENTEREVAL];
    myop.op_type = OP_ENTEREVAL;

    {
        dSP;
        oldmark = SP - PL_stack_base;
        EXTEND(SP, 1);
        PUSHs(sv);
        PUTBACK;
    }

    if (!(flags & G_NOARGS))
        myop.op_flags = OPf_STACKED;
    myop.op_type = OP_ENTEREVAL;
    myop.op_flags |= OP_GIMME_REVERSE(flags);
    if (flags & G_KEEPERR)
        myop.op_flags |= OPf_SPECIAL;

    myop.op_private = (OPpEVAL_EVALSV); /* tell pp_entereval we're the caller */
    if (flags & G_RE_REPARSING)
        myop.op_private |= (OPpEVAL_COPHH | OPpEVAL_RE_REPARSING);

    /* fail now; otherwise we could fail after the JMPENV_PUSH but
     * before a cx_pusheval(), which corrupts the stack after a croak */
    TAINT_PROPER("eval_sv()");

    JMPENV_PUSH(ret);
    switch (ret) {
    case 0:
        CALLRUNOPS(aTHX);
        if (!*PL_stack_sp) {
            /* In the presence of the OPpEVAL_EVALSV flag,
             * pp_entereval() pushes a NULL pointer onto the stack to
             * indicate compilation failure. Otherwise, the top slot on
             * the stack will be a non-NULL pointer to whatever scalar or
             * list value(s) the eval returned. In void context it will
             * be whatever our caller has at the top of stack at the time,
             * or the &PL_sv_undef guard at PL_stack_base[0]. Note that
             * NULLs are not pushed on the stack except in a few very
             * specific circumstances (such as this) to flag something
             * special. */
            PL_stack_sp--;
            goto fail;
        }
     redone_body:
        retval = PL_stack_sp - (PL_stack_base + oldmark);
        if (!(flags & G_KEEPERR)) {
            CLEAR_ERRSV();
        }
        break;
    case 1:
        STATUS_ALL_FAILURE;
        /* FALLTHROUGH */
    case 2:
        /* my_exit() was called */
        SET_CURSTASH(PL_defstash);
        FREETMPS;
        JMPENV_POP;
        my_exit_jump();
        NOT_REACHED; /* NOTREACHED */
    case 3:
        if (PL_restartop) {
            PL_restartjmpenv = NULL;
            PL_op = PL_restartop;
            PL_restartop = 0;
            CALLRUNOPS(aTHX);
            goto redone_body;
        }
      fail:
        if (flags & G_RETHROW) {
            JMPENV_POP;
            croak_sv(ERRSV);
        }
        /* Should be nothing left in stack frame apart from a possible
         * scalar context undef. Assert it's safe to reset the stack */
        assert(     PL_stack_sp == PL_stack_base + oldmark
                || (PL_stack_sp == PL_stack_base + oldmark + 1
                    && *PL_stack_sp == &PL_sv_undef));
        PL_stack_sp = PL_stack_base + oldmark;
        if ((flags & G_WANT) == G_LIST)
            retval = 0;
        else {
            retval = 1;
            *++PL_stack_sp = &PL_sv_undef;
        }
        break;
    }

    JMPENV_POP;
    if (flags & G_DISCARD) {
        PL_stack_sp = PL_stack_base + oldmark;
        retval = 0;
        FREETMPS;
        LEAVE;
    }
    PL_op = oldop;
    return retval;
}

/*
=for apidoc eval_pv

Tells Perl to C<eval> the given string in scalar context and return an SV* result.

=cut
*/

SV*
Perl_eval_pv(pTHX_ const char *p, I32 croak_on_error)
{
    SV* sv = newSVpv(p, 0);

    PERL_ARGS_ASSERT_EVAL_PV;

    if (croak_on_error) {
        sv_2mortal(sv);
        eval_sv(sv, G_SCALAR | G_RETHROW);
    }
    else {
        eval_sv(sv, G_SCALAR);
        SvREFCNT_dec(sv);
    }

    {
        dSP;
        sv = POPs;
        PUTBACK;
    }

    return sv;
}

/* Require a module. */

/*
=for apidoc_section $embedding

=for apidoc require_pv

Tells Perl to C<require> the file named by the string argument.  It is
analogous to the Perl code C<eval "require '$file'">.  It's even
implemented that way; consider using load_module instead.

=cut */

void
Perl_require_pv(pTHX_ const char *pv)
{
    dSP;
    SV* sv;

    PERL_ARGS_ASSERT_REQUIRE_PV;

    PUSHSTACKi(PERLSI_REQUIRE);
    sv = Perl_newSVpvf(aTHX_ "require q%c%s%c", 0, pv, 0);
    eval_sv(sv_2mortal(sv), G_DISCARD);
    POPSTACK;
}

STATIC void
S_usage(pTHX)		/* XXX move this out into a module ? */
{
    /* This message really ought to be max 23 lines.
     * Removed -h because the user already knows that option. Others? */

    /* Grouped as 6 lines per C string literal, to keep under the ANSI C 89
       minimum of 509 character string literals.  */
    static const char * const usage_msg[] = {
"  -0[octal/hexadecimal] specify record separator (\\0, if no argument)\n"
"  -a                    autosplit mode with -n or -p (splits $_ into @F)\n"
"  -C[number/list]       enables the listed Unicode features\n"
"  -c                    check syntax only (runs BEGIN and CHECK blocks)\n"
"  -d[t][:MOD]           run program under debugger or module Devel::MOD\n"
"  -D[number/letters]    set debugging flags (argument is a bit mask or alphabets)\n",
"  -e commandline        one line of program (several -e's allowed, omit programfile)\n"
"  -E commandline        like -e, but enables all optional features\n"
"  -f                    don't do $sitelib/sitecustomize.pl at startup\n"
"  -F/pattern/           split() pattern for -a switch (//'s are optional)\n"
"  -g                    read all input in one go (slurp), rather than line-by-line (alias for -0777)\n"
"  -i[extension]         edit <> files in place (makes backup if extension supplied)\n"
"  -Idirectory           specify @INC/#include directory (several -I's allowed)\n",
"  -l[octnum]            enable line ending processing, specifies line terminator\n"
"  -[mM][-]module        execute \"use/no module...\" before executing program\n"
"  -n                    assume \"while (<>) { ... }\" loop around program\n"
"  -p                    assume loop like -n but print line also, like sed\n"
"  -s                    enable rudimentary parsing for switches after programfile\n"
"  -S                    look for programfile using PATH environment variable\n",
"  -t                    enable tainting warnings\n"
"  -T                    enable tainting checks\n"
"  -u                    dump core after parsing program\n"
"  -U                    allow unsafe operations\n"
"  -v                    print version, patchlevel and license\n"
"  -V[:configvar]        print configuration summary (or a single Config.pm variable)\n",
"  -w                    enable many useful warnings\n"
"  -W                    enable all warnings\n"
"  -x[directory]         ignore text before #!perl line (optionally cd to directory)\n"
"  -X                    disable all warnings\n"
"  \n"
"Run 'perldoc perl' for more help with Perl.\n\n",
NULL
};
    const char * const *p = usage_msg;
    PerlIO *out = PerlIO_stdout();

    PerlIO_printf(out,
                  "\nUsage: %s [switches] [--] [programfile] [arguments]\n",
                  PL_origargv[0]);
    while (*p)
        PerlIO_puts(out, *p++);
    my_exit(0);
}

/* convert a string of -D options (or digits) into an int.
 * sets *s to point to the char after the options */

#ifdef DEBUGGING
int
Perl_get_debug_opts(pTHX_ const char **s, bool givehelp)
{
    static const char * const usage_msgd[] = {
      " Debugging flag values: (see also -d)\n"
      "  p  Tokenizing and parsing (with v, displays parse stack)\n"
      "  s  Stack snapshots (with v, displays all stacks)\n"
      "  l  Context (loop) stack processing\n"
      "  t  Trace execution\n"
      "  o  Method and overloading resolution\n",
      "  c  String/numeric conversions\n"
      "  P  Print profiling info, source file input state\n"
      "  m  Memory and SV allocation\n"
      "  f  Format processing\n"
      "  r  Regular expression parsing and execution\n"
      "  x  Syntax tree dump\n",
      "  u  Tainting checks\n"
      "  X  Scratchpad allocation\n"
      "  D  Cleaning up\n"
      "  S  Op slab allocation\n"
      "  T  Tokenising\n"
      "  R  Include reference counts of dumped variables (eg when using -Ds)\n",
      "  J  Do not s,t,P-debug (Jump over) opcodes within package DB\n"
      "  v  Verbose: use in conjunction with other flags\n"
      "  C  Copy On Write\n"
      "  A  Consistency checks on internal structures\n"
      "  q  quiet - currently only suppresses the 'EXECUTING' message\n"
      "  M  trace smart match resolution\n"
      "  B  dump suBroutine definitions, including special Blocks like BEGIN\n",
      "  L  trace some locale setting information--for Perl core development\n",
      "  i  trace PerlIO layer processing\n",
      "  y  trace y///, tr/// compilation and execution\n",
      "  h  Show (h)ash randomization debug output"
                " (changes to PL_hash_rand_bits)\n",
      NULL
    };
    UV uv = 0;

    PERL_ARGS_ASSERT_GET_DEBUG_OPTS;

    if (isALPHA(**s)) {
        /* NOTE:
         * If adding new options add them to the END of debopts[].
         * If you remove an option replace it with a '?'.
         * If there is a free slot available marked with '?' feel
         * free to reuse it for something else.
         *
         * Regardless remember to update DEBUG_MASK in perl.h, and
         * update the documentation above AND in pod/perlrun.pod.
         *
         * Note that the ? indicates an unused slot. As the code below
         * indicates the position in this list is important. You cannot
         * change the order or delete a character from the list without
         * impacting the definitions of all the other flags in perl.h
         * However because the logic is guarded by isWORDCHAR we can
         * fill in holes with non-wordchar characters instead. */
        static const char debopts[] = "psltocPmfrxuUhXDSTRJvCAqMBLiy";

        for (; isWORDCHAR(**s); (*s)++) {
            const char * const d = strchr(debopts,**s);
            if (d)
                uv |= 1 << (d - debopts);
            else if (ckWARN_d(WARN_DEBUGGING))
                Perl_warner(aTHX_ packWARN(WARN_DEBUGGING),
                    "invalid option -D%c, use -D'' to see choices\n", **s);
        }
    }
    else if (isDIGIT(**s)) {
        const char* e = *s + strlen(*s);
        if (grok_atoUV(*s, &uv, &e))
            *s = e;
        for (; isWORDCHAR(**s); (*s)++) ;
    }
    else if (givehelp) {
      const char *const *p = usage_msgd;
      while (*p) PerlIO_puts(PerlIO_stdout(), *p++);
    }
    return (int)uv; /* ignore any UV->int conversion loss */
}
#endif

/* This routine handles any switches that can be given during run */

const char *
Perl_moreswitches(pTHX_ const char *s)
{
    UV rschar;
    const char option = *s; /* used to remember option in -m/-M code */

    PERL_ARGS_ASSERT_MORESWITCHES;

    switch (*s) {
    case '0':
    {
         I32 flags = 0;
         STRLEN numlen;

         SvREFCNT_dec(PL_rs);
         if (s[1] == 'x' && s[2]) {
              const char *e = s+=2;
              U8 *tmps;

              while (*e)
                e++;
              numlen = e - s;
              flags = PERL_SCAN_SILENT_ILLDIGIT;
              rschar = (U32)grok_hex(s, &numlen, &flags, NULL);
              if (s + numlen < e) {
                  /* Continue to treat -0xFOO as -0 -xFOO
                   * (ie NUL as the input record separator, and -x with FOO
                   *  as the directory argument)
                   *
                   * hex support for -0 was only added in 5.8.1, hence this
                   * heuristic to distinguish between it and '-0' clustered with
                   * '-x' with an argument. The text following '-0x' is only
                   * processed as the IRS specified in hexadecimal if all
                   * characters are valid hex digits. */
                   rschar = 0;
                   numlen = 0;
                   s--;
              }
              PL_rs = newSV((STRLEN)(UVCHR_SKIP(rschar) + 1));
              tmps = (U8*)SvPVCLEAR_FRESH(PL_rs);
              uvchr_to_utf8(tmps, rschar);
              SvCUR_set(PL_rs, UVCHR_SKIP(rschar));
              SvUTF8_on(PL_rs);
         }
         else {
              numlen = 4;
              rschar = (U32)grok_oct(s, &numlen, &flags, NULL);
              if (rschar & ~((U8)~0))
                   PL_rs = &PL_sv_undef;
              else if (!rschar && numlen >= 2)
                   PL_rs = newSVpvs("");
              else {
                   char ch = (char)rschar;
                   PL_rs = newSVpvn(&ch, 1);
              }
         }
         sv_setsv(get_sv("/", GV_ADD), PL_rs);
         return s + numlen;
    }
    case 'C':
        s++;
        PL_unicode = parse_unicode_opts( (const char **)&s );
        if (PL_unicode & PERL_UNICODE_UTF8CACHEASSERT_FLAG)
            PL_utf8cache = -1;
        return s;
    case 'F':
        PL_minus_a = TRUE;
        PL_minus_F = TRUE;
        PL_minus_n = TRUE;
        {
            const char *start = ++s;
            while (*s && !isSPACE(*s)) ++s;
            Safefree(PL_splitstr);
            PL_splitstr = savepvn(start, s - start);
        }
        return s;
    case 'a':
        PL_minus_a = TRUE;
        PL_minus_n = TRUE;
        s++;
        return s;
    case 'c':
        PL_minus_c = TRUE;
        s++;
        return s;
    case 'd':
        forbid_setid('d', FALSE);
        s++;

        /* -dt indicates to the debugger that threads will be used */
        if (*s == 't' && !isWORDCHAR(s[1])) {
            ++s;
            my_setenv("PERL5DB_THREADED", "1");
        }

        /* The following permits -d:Mod to accepts arguments following an =
           in the fashion that -MSome::Mod does. */
        if (*s == ':' || *s == '=') {
            const char *start;
            const char *end;
            SV *sv;

            if (*++s == '-') {
                ++s;
                sv = newSVpvs("no Devel::");
            } else {
                sv = newSVpvs("use Devel::");
            }

            start = s;
            end = s + strlen(s);

            /* We now allow -d:Module=Foo,Bar and -d:-Module */
            while(isWORDCHAR(*s) || *s==':') ++s;
            if (*s != '=')
                sv_catpvn(sv, start, end - start);
            else {
                sv_catpvn(sv, start, s-start);
                /* Don't use NUL as q// delimiter here, this string goes in the
                 * environment. */
                Perl_sv_catpvf(aTHX_ sv, " split(/,/,q{%s});", ++s);
            }
            s = end;
            my_setenv("PERL5DB", SvPV_nolen_const(sv));
            SvREFCNT_dec(sv);
        }
        if (!PL_perldb) {
            PL_perldb = PERLDB_ALL;
            init_debugger();
        }
        return s;
    case 'D':
    {
#ifdef DEBUGGING
        forbid_setid('D', FALSE);
        s++;
        PL_debug = get_debug_opts( (const char **)&s, 1) | DEBUG_TOP_FLAG;
#else /* !DEBUGGING */
        if (ckWARN_d(WARN_DEBUGGING))
            Perl_warner(aTHX_ packWARN(WARN_DEBUGGING),
                   "Recompile perl with -DDEBUGGING to use -D switch (did you mean -d ?)\n");
        for (s++; isWORDCHAR(*s); s++) ;
#endif
        return s;
        NOT_REACHED; /* NOTREACHED */
    }
    case 'g':
        SvREFCNT_dec(PL_rs);
        PL_rs = &PL_sv_undef;
        sv_setsv(get_sv("/", GV_ADD), PL_rs);
        return ++s;

    case '?':
        /* FALLTHROUGH */
    case 'h':
        usage();
        NOT_REACHED; /* NOTREACHED */

    case 'i':
        Safefree(PL_inplace);
        {
            const char * const start = ++s;
            while (*s && !isSPACE(*s))
                ++s;

            PL_inplace = savepvn(start, s - start);
        }
        return s;
    case 'I':	/* -I handled both here and in parse_body() */
        forbid_setid('I', FALSE);
        ++s;
        while (*s && isSPACE(*s))
            ++s;
        if (*s) {
            const char *e, *p;
            p = s;
            /* ignore trailing spaces (possibly followed by other switches) */
            do {
                for (e = p; *e && !isSPACE(*e); e++) ;
                p = e;
                while (isSPACE(*p))
                    p++;
            } while (*p && *p != '-');
            incpush(s, e-s,
                    INCPUSH_ADD_SUB_DIRS|INCPUSH_ADD_OLD_VERS|INCPUSH_UNSHIFT);
            s = p;
            if (*s == '-')
                s++;
        }
        else
            Perl_croak(aTHX_ "No directory specified for -I");
        return s;
    case 'l':
        PL_minus_l = TRUE;
        s++;
        if (PL_ors_sv) {
            SvREFCNT_dec(PL_ors_sv);
            PL_ors_sv = NULL;
        }
        if (isDIGIT(*s)) {
            I32 flags = 0;
            STRLEN numlen;
            PL_ors_sv = newSVpvs("\n");
            numlen = 3 + (*s == '0');
            *SvPVX(PL_ors_sv) = (char)grok_oct(s, &numlen, &flags, NULL);
            s += numlen;
        }
        else {
            if (RsPARA(PL_rs)) {
                PL_ors_sv = newSVpvs("\n\n");
            }
            else {
                PL_ors_sv = newSVsv(PL_rs);
            }
        }
        return s;
    case 'M':
        forbid_setid('M', FALSE);	/* XXX ? */
        /* FALLTHROUGH */
    case 'm':
        forbid_setid('m', FALSE);	/* XXX ? */
        if (*++s) {
            const char *start;
            const char *end;
            SV *sv;
            const char *use = "use ";
            bool colon = FALSE;
            /* -M-foo == 'no foo'	*/
            /* Leading space on " no " is deliberate, to make both
               possibilities the same length.  */
            if (*s == '-') { use = " no "; ++s; }
            sv = newSVpvn(use,4);
            start = s;
            /* We allow -M'Module qw(Foo Bar)'	*/
            while(isWORDCHAR(*s) || *s==':') {
                if( *s++ == ':' ) {
                    if( *s == ':' )
                        s++;
                    else
                        colon = TRUE;
                }
            }
            if (s == start)
                Perl_croak(aTHX_ "Module name required with -%c option",
                                    option);
            if (colon)
                Perl_croak(aTHX_ "Invalid module name %.*s with -%c option: "
                                    "contains single ':'",
                                    (int)(s - start), start, option);
            end = s + strlen(s);
            if (*s != '=') {
                sv_catpvn(sv, start, end - start);
                if (option == 'm') {
                    if (*s != '\0')
                        Perl_croak(aTHX_ "Can't use '%c' after -mname", *s);
                    sv_catpvs( sv, " ()");
                }
            } else {
                sv_catpvn(sv, start, s-start);
                /* Use NUL as q''-delimiter.  */
                sv_catpvs(sv, " split(/,/,q\0");
                ++s;
                sv_catpvn(sv, s, end - s);
                sv_catpvs(sv,  "\0)");
            }
            s = end;
            Perl_av_create_and_push(aTHX_ &PL_preambleav, sv);
        }
        else
            Perl_croak(aTHX_ "Missing argument to -%c", option);
        return s;
    case 'n':
        PL_minus_n = TRUE;
        s++;
        return s;
    case 'p':
        PL_minus_p = TRUE;
        s++;
        return s;
    case 's':
        forbid_setid('s', FALSE);
        PL_doswitches = TRUE;
        s++;
        return s;
    case 't':
    case 'T':
#if defined(SILENT_NO_TAINT_SUPPORT)
            /* silently ignore */
#elif defined(NO_TAINT_SUPPORT)
        Perl_croak_nocontext("This perl was compiled without taint support. "
                   "Cowardly refusing to run with -t or -T flags");
#else
        if (!TAINTING_get)
            TOO_LATE_FOR(*s);
#endif
        s++;
        return s;
    case 'u':
        PL_do_undump = TRUE;
        s++;
        return s;
    case 'U':
        PL_unsafe = TRUE;
        s++;
        return s;
    case 'v':
        minus_v();
    case 'w':
        if (! (PL_dowarn & G_WARN_ALL_MASK)) {
            PL_dowarn |= G_WARN_ON;
        }
        s++;
        return s;
    case 'W':
        PL_dowarn = G_WARN_ALL_ON|G_WARN_ON;
        free_and_set_cop_warnings(&PL_compiling, pWARN_ALL);
        s++;
        return s;
    case 'X':
        PL_dowarn = G_WARN_ALL_OFF;
        free_and_set_cop_warnings(&PL_compiling, pWARN_NONE);
        s++;
        return s;
    case '*':
    case ' ':
        while( *s == ' ' )
          ++s;
        if (s[0] == '-')	/* Additional switches on #! line. */
            return s+1;
        break;
    case '-':
    case 0:
#if defined(WIN32) || !defined(PERL_STRICT_CR)
    case '\r':
#endif
    case '\n':
    case '\t':
        break;
#ifdef ALTERNATE_SHEBANG
    case 'S':			/* OS/2 needs -S on "extproc" line. */
        break;
#endif
    case 'e': case 'f': case 'x': case 'E':
#ifndef ALTERNATE_SHEBANG
    case 'S':
#endif
    case 'V':
        Perl_croak(aTHX_ "Can't emulate -%.1s on #! line",s);
    default:
        Perl_croak(aTHX_
            "Unrecognized switch: -%.1s  (-h will show valid options)",s
        );
    }
    return NULL;
}


STATIC void
S_minus_v(pTHX)
{
        PerlIO * PIO_stdout;
        {
            const char * const level_str = "v" PERL_VERSION_STRING;
            const STRLEN level_len = sizeof("v" PERL_VERSION_STRING)-1;
#ifdef PERL_PATCHNUM
            SV* level;
#  ifdef PERL_GIT_UNCOMMITTED_CHANGES
            static const char num [] = PERL_PATCHNUM "*";
#  else
            static const char num [] = PERL_PATCHNUM;
#  endif
            {
                const STRLEN num_len = sizeof(num)-1;
                /* A very advanced compiler would fold away the strnEQ
                   and this whole conditional, but most (all?) won't do it.
                   SV level could also be replaced by with preprocessor
                   catenation.
                */
                if (num_len >= level_len && strnEQ(num,level_str,level_len)) {
                    /* per 46807d8e80, PERL_PATCHNUM is outside of the control
                       of the interp so it might contain format characters
                    */
                    level = newSVpvn(num, num_len);
                } else {
                    level = Perl_newSVpvf_nocontext("%s (%s)", level_str, num);
                }
            }
#else
        SV* level = newSVpvn(level_str, level_len);
#endif /* #ifdef PERL_PATCHNUM */
        PIO_stdout =  PerlIO_stdout();
            PerlIO_printf(PIO_stdout,
                "\nThis is perl "	STRINGIFY(PERL_REVISION)
                ", version "		STRINGIFY(PERL_VERSION)
                ", subversion "		STRINGIFY(PERL_SUBVERSION)
                " (%" SVf ") built for "	ARCHNAME, SVfARG(level)
                );
            SvREFCNT_dec_NN(level);
        }
#if defined(LOCAL_PATCH_COUNT)
        if (LOCAL_PATCH_COUNT > 0)
            PerlIO_printf(PIO_stdout,
                          "\n(with %d registered patch%s, "
                          "see perl -V for more detail)",
                          LOCAL_PATCH_COUNT,
                          (LOCAL_PATCH_COUNT!=1) ? "es" : "");
#endif

        PerlIO_printf(PIO_stdout,
		      "\n\nCopyright 1987-2023, Larry Wall\n");
#ifdef OS2
        PerlIO_printf(PIO_stdout,
                      "\n\nOS/2 port Copyright (c) 1990, 1991, Raymond Chen, Kai Uwe Rommel\n"
                      "Version 5 port Copyright (c) 1994-2002, Andreas Kaiser, Ilya Zakharevich\n");
#endif
#ifdef OEMVS
        PerlIO_printf(PIO_stdout,
                      "MVS (OS390) port by Mortice Kern Systems, 1997-1999\n");
#endif
#ifdef __VOS__
        PerlIO_printf(PIO_stdout,
                      "Stratus OpenVOS port by Paul.Green@stratus.com, 1997-2013\n");
#endif
#ifdef POSIX_BC
        PerlIO_printf(PIO_stdout,
                      "BS2000 (POSIX) port by Start Amadeus GmbH, 1998-1999\n");
#endif
#ifdef BINARY_BUILD_NOTICE
        BINARY_BUILD_NOTICE;
#endif
        PerlIO_printf(PIO_stdout,
                      "\n\
Perl may be copied only under the terms of either the Artistic License or the\n\
GNU General Public License, which may be found in the Perl 5 source kit.\n\n\
Complete documentation for Perl, including FAQ lists, should be found on\n\
this system using \"man perl\" or \"perldoc perl\".  If you have access to the\n\
Internet, point your browser at https://www.perl.org/, the Perl Home Page.\n\n");
        my_exit(0);
}

/* compliments of Tom Christiansen */

/* unexec() can be found in the Gnu emacs distribution */
/* Known to work with -DUNEXEC and using unexelf.c from GNU emacs-20.2 */

#ifdef VMS
#include <lib$routines.h>
#endif

void
Perl_my_unexec(pTHX)
{
#ifdef UNEXEC
    SV *    prog = newSVpv(BIN_EXP, 0);
    SV *    file = newSVpv(PL_origfilename, 0);
    int    status = 1;
    extern int etext;

    sv_catpvs(prog, "/perl");
    sv_catpvs(file, ".perldump");

    unexec(SvPVX(file), SvPVX(prog), &etext, sbrk(0), 0);
    /* unexec prints msg to stderr in case of failure */
    PerlProc_exit(status);
#else
    PERL_UNUSED_CONTEXT;
#  ifdef VMS
     lib$signal(SS$_DEBUG);  /* ssdef.h #included from vmsish.h */
#  elif defined(WIN32) || defined(__CYGWIN__)
    Perl_croak_nocontext("dump is not supported");
#  else
    ABORT();		/* for use with undump */
#  endif
#endif
}

/* initialize curinterp */
STATIC void
S_init_interp(pTHX)
{
#ifdef MULTIPLICITY
#  define PERLVAR(prefix,var,type)
#  define PERLVARA(prefix,var,n,type)
#  if defined(MULTIPLICITY)
#    define PERLVARI(prefix,var,type,init)	aTHX->prefix##var = init;
#    define PERLVARIC(prefix,var,type,init)	aTHX->prefix##var = init;
#  else
#    define PERLVARI(prefix,var,type,init)	PERL_GET_INTERP->var = init;
#    define PERLVARIC(prefix,var,type,init)	PERL_GET_INTERP->var = init;
#  endif
#  include "intrpvar.h"
#  undef PERLVAR
#  undef PERLVARA
#  undef PERLVARI
#  undef PERLVARIC
#else
#  define PERLVAR(prefix,var,type)
#  define PERLVARA(prefix,var,n,type)
#  define PERLVARI(prefix,var,type,init)	PL_##var = init;
#  define PERLVARIC(prefix,var,type,init)	PL_##var = init;
#  include "intrpvar.h"
#  undef PERLVAR
#  undef PERLVARA
#  undef PERLVARI
#  undef PERLVARIC
#endif

}

STATIC void
S_init_main_stash(pTHX)
{
    GV *gv;
    HV *hv = newHV();

    PL_curstash = PL_defstash = (HV *)SvREFCNT_inc_simple_NN(hv);
    /* We know that the string "main" will be in the global shared string
       table, so it's a small saving to use it rather than allocate another
       8 bytes.  */
    PL_curstname = newSVpvs_share("main");
    gv = gv_fetchpvs("main::", GV_ADD|GV_NOTQUAL, SVt_PVHV);
    /* If we hadn't caused another reference to "main" to be in the shared
       string table above, then it would be worth reordering these two,
       because otherwise all we do is delete "main" from it as a consequence
       of the SvREFCNT_dec, only to add it again with hv_name_set */
    SvREFCNT_dec(GvHV(gv));
    hv_name_sets(PL_defstash, "main", 0);
    GvHV(gv) = MUTABLE_HV(SvREFCNT_inc_simple(PL_defstash));
    SvREADONLY_on(gv);
    PL_incgv = gv_HVadd(gv_AVadd(gv_fetchpvs("INC", GV_ADD|GV_NOTQUAL,
                                             SVt_PVAV)));
    SvREFCNT_inc_simple_void(PL_incgv); /* Don't allow it to be freed */
    GvMULTI_on(PL_incgv);
    PL_hintgv = gv_fetchpvs("\010", GV_ADD|GV_NOTQUAL, SVt_PV); /* ^H */
    SvREFCNT_inc_simple_void(PL_hintgv);
    GvMULTI_on(PL_hintgv);
    PL_defgv = gv_fetchpvs("_", GV_ADD|GV_NOTQUAL, SVt_PVAV);
    SvREFCNT_inc_simple_void(PL_defgv);
    PL_errgv = gv_fetchpvs("@", GV_ADD|GV_NOTQUAL, SVt_PV);
    SvREFCNT_inc_simple_void(PL_errgv);
    GvMULTI_on(PL_errgv);
    PL_replgv = gv_fetchpvs("\022", GV_ADD|GV_NOTQUAL, SVt_PV); /* ^R */
    SvREFCNT_inc_simple_void(PL_replgv);
    GvMULTI_on(PL_replgv);
    (void)Perl_form(aTHX_ "%240s","");	/* Preallocate temp - for immediate signals. */
#ifdef PERL_DONT_CREATE_GVSV
    (void)gv_SVadd(PL_errgv);
#endif
    sv_grow(ERRSV, 240);	/* Preallocate - for immediate signals. */
    CLEAR_ERRSV();
    CopSTASH_set(&PL_compiling, PL_defstash);
    PL_debstash = GvHV(gv_fetchpvs("DB::", GV_ADDMULTI, SVt_PVHV));
    PL_globalstash = GvHV(gv_fetchpvs("CORE::GLOBAL::", GV_ADDMULTI,
                                      SVt_PVHV));
    /* We must init $/ before switches are processed. */
    sv_setpvs(get_sv("/", GV_ADD), "\n");
}

STATIC PerlIO *
S_open_script(pTHX_ const char *scriptname, bool dosearch, bool *suidscript)
{
    int fdscript = -1;
    PerlIO *rsfp = NULL;
    Stat_t tmpstatbuf;
    int fd;

    PERL_ARGS_ASSERT_OPEN_SCRIPT;

    if (PL_e_script) {
        PL_origfilename = savepvs("-e");
    }
    else {
        const char *s;
        UV uv;
        /* if find_script() returns, it returns a malloc()-ed value */
        scriptname = PL_origfilename = find_script(scriptname, dosearch, NULL, 1);
        s = scriptname + strlen(scriptname);

        if (strBEGINs(scriptname, "/dev/fd/")
            && isDIGIT(scriptname[8])
            && grok_atoUV(scriptname + 8, &uv, &s)
            && uv <= PERL_INT_MAX
        ) {
            fdscript = (int)uv;
            if (*s) {
                /* PSz 18 Feb 04
                 * Tell apart "normal" usage of fdscript, e.g.
                 * with bash on FreeBSD:
                 *   perl <( echo '#!perl -DA'; echo 'print "$0\n"')
                 * from usage in suidperl.
                 * Does any "normal" usage leave garbage after the number???
                 * Is it a mistake to use a similar /dev/fd/ construct for
                 * suidperl?
                 */
                *suidscript = TRUE;
                /* PSz 20 Feb 04
                 * Be supersafe and do some sanity-checks.
                 * Still, can we be sure we got the right thing?
                 */
                if (*s != '/') {
                    Perl_croak(aTHX_ "Wrong syntax (suid) fd script name \"%s\"\n", s);
                }
                if (! *(s+1)) {
                    Perl_croak(aTHX_ "Missing (suid) fd script name\n");
                }
                scriptname = savepv(s + 1);
                Safefree(PL_origfilename);
                PL_origfilename = (char *)scriptname;
            }
        }
    }

    CopFILE_free(PL_curcop);
    CopFILE_set(PL_curcop, PL_origfilename);
    if (*PL_origfilename == '-' && PL_origfilename[1] == '\0')
        scriptname = (char *)"";
    if (fdscript >= 0) {
        rsfp = PerlIO_fdopen(fdscript,PERL_SCRIPT_MODE);
    }
    else if (!*scriptname) {
        forbid_setid(0, *suidscript);
        return NULL;
    }
    else {
#ifdef FAKE_BIT_BUCKET
        /* This hack allows one not to have /dev/null (or BIT_BUCKET as it
         * is called) and still have the "-e" work.  (Believe it or not,
         * a /dev/null is required for the "-e" to work because source
         * filter magic is used to implement it. ) This is *not* a general
         * replacement for a /dev/null.  What we do here is create a temp
         * file (an empty file), open up that as the script, and then
         * immediately close and unlink it.  Close enough for jazz. */
#define FAKE_BIT_BUCKET_PREFIX "/tmp/perlnull-"
#define FAKE_BIT_BUCKET_SUFFIX "XXXXXXXX"
#define FAKE_BIT_BUCKET_TEMPLATE FAKE_BIT_BUCKET_PREFIX FAKE_BIT_BUCKET_SUFFIX
        char tmpname[sizeof(FAKE_BIT_BUCKET_TEMPLATE)] = {
            FAKE_BIT_BUCKET_TEMPLATE
        };
        const char * const err = "Failed to create a fake bit bucket";
        if (strEQ(scriptname, BIT_BUCKET)) {
            int tmpfd = Perl_my_mkstemp_cloexec(tmpname);
            if (tmpfd > -1) {
                scriptname = tmpname;
                close(tmpfd);
            } else
                Perl_croak(aTHX_ err);
        }
#endif
        rsfp = PerlIO_open(scriptname,PERL_SCRIPT_MODE);
#ifdef FAKE_BIT_BUCKET
        if (   strBEGINs(scriptname, FAKE_BIT_BUCKET_PREFIX)
            && strlen(scriptname) == sizeof(tmpname) - 1)
        {
            unlink(scriptname);
        }
        scriptname = BIT_BUCKET;
#endif
    }
    if (!rsfp) {
        /* PSz 16 Sep 03  Keep neat error message */
        if (PL_e_script)
            Perl_croak(aTHX_ "Can't open " BIT_BUCKET ": %s\n", Strerror(errno));
        else
            Perl_croak(aTHX_ "Can't open perl script \"%s\": %s\n",
                    CopFILE(PL_curcop), Strerror(errno));
    }
    fd = PerlIO_fileno(rsfp);

    if (fd < 0 ||
        (PerlLIO_fstat(fd, &tmpstatbuf) >= 0
         && S_ISDIR(tmpstatbuf.st_mode)))
        Perl_croak(aTHX_ "Can't open perl script \"%s\": %s\n",
            CopFILE(PL_curcop),
            Strerror(EISDIR));

    return rsfp;
}

/* In the days of suidperl, we refused to execute a setuid script stored on
 * a filesystem mounted nosuid and/or noexec. This meant that we probed for the
 * existence of the appropriate filesystem-statting function, and behaved
 * accordingly. But even though suidperl is long gone, we must still include
 * those probes for the benefit of modules like Filesys::Df, which expect the
 * results of those probes to be stored in %Config; see RT#126368. So mention
 * the relevant cpp symbols here, to ensure that metaconfig will include their
 * probes in the generated Configure:
 *
 * I_SYSSTATVFS	HAS_FSTATVFS
 * I_SYSMOUNT
 * I_STATFS	HAS_FSTATFS	HAS_GETFSSTAT
 * I_MNTENT	HAS_GETMNTENT	HAS_HASMNTOPT
 */


#ifdef SETUID_SCRIPTS_ARE_SECURE_NOW
/* Don't even need this function.  */
#else
STATIC void
S_validate_suid(pTHX_ PerlIO *rsfp)
{
    const Uid_t  my_uid = PerlProc_getuid();
    const Uid_t my_euid = PerlProc_geteuid();
    const Gid_t  my_gid = PerlProc_getgid();
    const Gid_t my_egid = PerlProc_getegid();

    PERL_ARGS_ASSERT_VALIDATE_SUID;

    if (my_euid != my_uid || my_egid != my_gid) {	/* (suidperl doesn't exist, in fact) */
        int fd = PerlIO_fileno(rsfp);
        Stat_t statbuf;
        if (fd < 0 || PerlLIO_fstat(fd, &statbuf) < 0) { /* may be either wrapped or real suid */
            Perl_croak_nocontext( "Illegal suidscript");
        }
        if ((my_euid != my_uid && my_euid == statbuf.st_uid && statbuf.st_mode & S_ISUID)
            ||
            (my_egid != my_gid && my_egid == statbuf.st_gid && statbuf.st_mode & S_ISGID)
            )
            if (!PL_do_undump)
                Perl_croak(aTHX_ "YOU HAVEN'T DISABLED SET-ID SCRIPTS IN THE KERNEL YET!\n\
FIX YOUR KERNEL, PUT A C WRAPPER AROUND THIS SCRIPT, OR USE -u AND UNDUMP!\n");
        /* not set-id, must be wrapped */
    }
}
#endif /* SETUID_SCRIPTS_ARE_SECURE_NOW */

STATIC void
S_find_beginning(pTHX_ SV* linestr_sv, PerlIO *rsfp)
{
    const char *s;
    const char *s2;

    PERL_ARGS_ASSERT_FIND_BEGINNING;

    /* skip forward in input to the real script? */

    do {
        if ((s = sv_gets(linestr_sv, rsfp, 0)) == NULL)
            Perl_croak(aTHX_ "No Perl script found in input\n");
        s2 = s;
    } while (!(*s == '#' && s[1] == '!' && ((s = instr(s,"perl")) || (s = instr(s2,"PERL")))));
    PerlIO_ungetc(rsfp, '\n');		/* to keep line count right */
    while (*s && !(isSPACE (*s) || *s == '#')) s++;
    s2 = s;
    while (*s == ' ' || *s == '\t') s++;
    if (*s++ == '-') {
        while (isDIGIT(s2[-1]) || s2[-1] == '-' || s2[-1] == '.'
               || s2[-1] == '_') s2--;
        if (strBEGINs(s2-4,"perl"))
            while ((s = moreswitches(s)))
                ;
    }
}


STATIC void
S_init_ids(pTHX)
{
    /* no need to do anything here any more if we don't
     * do tainting. */
#ifndef NO_TAINT_SUPPORT
    const Uid_t my_uid = PerlProc_getuid();
    const Uid_t my_euid = PerlProc_geteuid();
    const Gid_t my_gid = PerlProc_getgid();
    const Gid_t my_egid = PerlProc_getegid();

    PERL_UNUSED_CONTEXT;

    /* Should not happen: */
    CHECK_MALLOC_TAINT(my_uid && (my_euid != my_uid || my_egid != my_gid));
    TAINTING_set( TAINTING_get | (my_uid && (my_euid != my_uid || my_egid != my_gid)) );
#endif
    /* BUG */
    /* PSz 27 Feb 04
     * Should go by suidscript, not uid!=euid: why disallow
     * system("ls") in scripts run from setuid things?
     * Or, is this run before we check arguments and set suidscript?
     * What about SETUID_SCRIPTS_ARE_SECURE_NOW: could we use fdscript then?
     * (We never have suidscript, can we be sure to have fdscript?)
     * Or must then go by UID checks? See comments in forbid_setid also.
     */
}

/* This is used very early in the lifetime of the program,
 * before even the options are parsed, so PL_tainting has
 * not been initialized properly.  */
bool
Perl_doing_taint(int argc, char *argv[], char *envp[])
{
#ifndef PERL_IMPLICIT_SYS
    /* If we have PERL_IMPLICIT_SYS we can't call getuid() et alia
     * before we have an interpreter-- and the whole point of this
     * function is to be called at such an early stage.  If you are on
     * a system with PERL_IMPLICIT_SYS but you do have a concept of
     * "tainted because running with altered effective ids', you'll
     * have to add your own checks somewhere in here.  The most known
     * sample of 'implicitness' is Win32, which doesn't have much of
     * concept of 'uids'. */
    Uid_t uid  = PerlProc_getuid();
    Uid_t euid = PerlProc_geteuid();
    Gid_t gid  = PerlProc_getgid();
    Gid_t egid = PerlProc_getegid();
    (void)envp;

#ifdef VMS
    uid  |=  gid << 16;
    euid |= egid << 16;
#endif
    if (uid && (euid != uid || egid != gid))
        return 1;
#endif /* !PERL_IMPLICIT_SYS */
    /* This is a really primitive check; environment gets ignored only
     * if -T are the first chars together; otherwise one gets
     *  "Too late" message. */
    if ( argc > 1 && argv[1][0] == '-'
         && isALPHA_FOLD_EQ(argv[1][1], 't'))
        return 1;
    return 0;
}

/* Passing the flag as a single char rather than a string is a slight space
   optimisation.  The only message that isn't /^-.$/ is
   "program input from stdin", which is substituted in place of '\0', which
   could never be a command line flag.  */
STATIC void
S_forbid_setid(pTHX_ const char flag, const bool suidscript) /* g */
{
    char string[3] = "-x";
    const char *message = "program input from stdin";

    PERL_UNUSED_CONTEXT;
    if (flag) {
        string[1] = flag;
        message = string;
    }

#ifdef SETUID_SCRIPTS_ARE_SECURE_NOW
    if (PerlProc_getuid() != PerlProc_geteuid())
        Perl_croak(aTHX_ "No %s allowed while running setuid", message);
    if (PerlProc_getgid() != PerlProc_getegid())
        Perl_croak(aTHX_ "No %s allowed while running setgid", message);
#endif /* SETUID_SCRIPTS_ARE_SECURE_NOW */
    if (suidscript)
        Perl_croak(aTHX_ "No %s allowed with (suid) fdscript", message);
}

void
Perl_init_dbargs(pTHX)
{
    AV *const args = PL_dbargs = GvAV(gv_AVadd((gv_fetchpvs("DB::args",
                                                            GV_ADDMULTI,
                                                            SVt_PVAV))));

    if (AvREAL(args)) {
        /* Someone has already created it.
           It might have entries, and if we just turn off AvREAL(), they will
           "leak" until global destruction.  */
        av_clear(args);
        if (SvTIED_mg((const SV *)args, PERL_MAGIC_tied))
            Perl_croak(aTHX_ "Cannot set tied @DB::args");
    }
    AvREIFY_only(PL_dbargs);
}

void
Perl_init_debugger(pTHX)
{
    HV * const ostash = PL_curstash;
    MAGIC *mg;

    PL_curstash = (HV *)SvREFCNT_inc_simple(PL_debstash);

    Perl_init_dbargs(aTHX);
    PL_DBgv = MUTABLE_GV(
        SvREFCNT_inc(gv_fetchpvs("DB::DB", GV_ADDMULTI, SVt_PVGV))
    );
    PL_DBline = MUTABLE_GV(
        SvREFCNT_inc(gv_fetchpvs("DB::dbline", GV_ADDMULTI, SVt_PVAV))
    );
    PL_DBsub = MUTABLE_GV(SvREFCNT_inc(
        gv_HVadd(gv_fetchpvs("DB::sub", GV_ADDMULTI, SVt_PVHV))
    ));
    PL_DBsingle = GvSV((gv_fetchpvs("DB::single", GV_ADDMULTI, SVt_PV)));
    if (!SvIOK(PL_DBsingle))
        sv_setiv(PL_DBsingle, 0);
    mg = sv_magicext(PL_DBsingle, NULL, PERL_MAGIC_debugvar, &PL_vtbl_debugvar, 0, 0);
    mg->mg_private = DBVARMG_SINGLE;
    SvSETMAGIC(PL_DBsingle);

    PL_DBtrace = GvSV((gv_fetchpvs("DB::trace", GV_ADDMULTI, SVt_PV)));
    if (!SvIOK(PL_DBtrace))
        sv_setiv(PL_DBtrace, 0);
    mg = sv_magicext(PL_DBtrace, NULL, PERL_MAGIC_debugvar, &PL_vtbl_debugvar, 0, 0);
    mg->mg_private = DBVARMG_TRACE;
    SvSETMAGIC(PL_DBtrace);

    PL_DBsignal = GvSV((gv_fetchpvs("DB::signal", GV_ADDMULTI, SVt_PV)));
    if (!SvIOK(PL_DBsignal))
        sv_setiv(PL_DBsignal, 0);
    mg = sv_magicext(PL_DBsignal, NULL, PERL_MAGIC_debugvar, &PL_vtbl_debugvar, 0, 0);
    mg->mg_private = DBVARMG_SIGNAL;
    SvSETMAGIC(PL_DBsignal);

    SvREFCNT_dec(PL_curstash);
    PL_curstash = ostash;
}

#ifndef STRESS_REALLOC
#define REASONABLE(size) (size)
#define REASONABLE_but_at_least(size,min) (size)
#else
#define REASONABLE(size) (1) /* unreasonable */
#define REASONABLE_but_at_least(size,min) (min)
#endif

void
Perl_init_stacks(pTHX)
{
    SSize_t size;

    /* start with 128-item stack and 8K cxstack */
    PL_curstackinfo = new_stackinfo(REASONABLE(128),
                                 REASONABLE(8192/sizeof(PERL_CONTEXT) - 1));
    PL_curstackinfo->si_type = PERLSI_MAIN;
#if defined DEBUGGING && !defined DEBUGGING_RE_ONLY
    PL_curstackinfo->si_stack_hwm = 0;
#endif
    PL_curstack = PL_curstackinfo->si_stack;
    PL_mainstack = PL_curstack;		/* remember in case we switch stacks */

    PL_stack_base = AvARRAY(PL_curstack);
    PL_stack_sp = PL_stack_base;
    PL_stack_max = PL_stack_base + AvMAX(PL_curstack);

    Newxz(PL_tmps_stack,REASONABLE(128),SV*);
    PL_tmps_floor = -1;
    PL_tmps_ix = -1;
    PL_tmps_max = REASONABLE(128);

    Newxz(PL_markstack,REASONABLE(32),I32);
    PL_markstack_ptr = PL_markstack;
    PL_markstack_max = PL_markstack + REASONABLE(32);

    SET_MARK_OFFSET;

    Newxz(PL_scopestack,REASONABLE(32),I32);
#ifdef DEBUGGING
    Newxz(PL_scopestack_name,REASONABLE(32),const char*);
#endif
    PL_scopestack_ix = 0;
    PL_scopestack_max = REASONABLE(32);

    size = REASONABLE_but_at_least(128,SS_MAXPUSH);
    Newxz(PL_savestack, size, ANY);
    PL_savestack_ix = 0;
    /*PL_savestack_max lies: it always has SS_MAXPUSH more than it claims */
    PL_savestack_max = size - SS_MAXPUSH;
}

#undef REASONABLE

STATIC void
S_nuke_stacks(pTHX)
{
    while (PL_curstackinfo->si_next)
        PL_curstackinfo = PL_curstackinfo->si_next;
    while (PL_curstackinfo) {
        PERL_SI *p = PL_curstackinfo->si_prev;
        /* curstackinfo->si_stack got nuked by sv_free_arenas() */
        Safefree(PL_curstackinfo->si_cxstack);
        Safefree(PL_curstackinfo);
        PL_curstackinfo = p;
    }
    Safefree(PL_tmps_stack);
    Safefree(PL_markstack);
    Safefree(PL_scopestack);
#ifdef DEBUGGING
    Safefree(PL_scopestack_name);
#endif
    Safefree(PL_savestack);
}

void
Perl_populate_isa(pTHX_ const char *name, STRLEN len, ...)
{
    GV *const gv = gv_fetchpvn(name, len, GV_ADD | GV_ADDMULTI, SVt_PVAV);
    AV *const isa = GvAVn(gv);
    va_list args;

    PERL_ARGS_ASSERT_POPULATE_ISA;

    if(AvFILLp(isa) != -1)
        return;

    /* NOTE: No support for tied ISA */

    va_start(args, len);
    do {
        const char *const parent = va_arg(args, const char*);
        size_t parent_len;

        if (!parent)
            break;
        parent_len = va_arg(args, size_t);

        /* Arguments are supplied with a trailing ::  */
        assert(parent_len > 2);
        assert(parent[parent_len - 1] == ':');
        assert(parent[parent_len - 2] == ':');
        av_push(isa, newSVpvn(parent, parent_len - 2));
        (void) gv_fetchpvn(parent, parent_len, GV_ADD, SVt_PVGV);
    } while (1);
    va_end(args);
}


STATIC void
S_init_predump_symbols(pTHX)
{
    GV *tmpgv;
    IO *io;

    sv_setpvs(get_sv("\"", GV_ADD), " ");
    PL_ofsgv = (GV*)SvREFCNT_inc(gv_fetchpvs(",", GV_ADD|GV_NOTQUAL, SVt_PV));


    /* Historically, PVIOs were blessed into IO::Handle, unless
       FileHandle was loaded, in which case they were blessed into
       that. Action at a distance.
       However, if we simply bless into IO::Handle, we break code
       that assumes that PVIOs will have (among others) a seek
       method. IO::File inherits from IO::Handle and IO::Seekable,
       and provides the needed methods. But if we simply bless into
       it, then we break code that assumed that by loading
       IO::Handle, *it* would work.
       So a compromise is to set up the correct @IO::File::ISA,
       so that code that does C<use IO::Handle>; will still work.
    */

    Perl_populate_isa(aTHX_ STR_WITH_LEN("IO::File::ISA"),
                      STR_WITH_LEN("IO::Handle::"),
                      STR_WITH_LEN("IO::Seekable::"),
                      STR_WITH_LEN("Exporter::"),
                      NULL);

    PL_stdingv = gv_fetchpvs("STDIN", GV_ADD|GV_NOTQUAL, SVt_PVIO);
    GvMULTI_on(PL_stdingv);
    io = GvIOp(PL_stdingv);
    IoTYPE(io) = IoTYPE_RDONLY;
    IoIFP(io) = PerlIO_stdin();
    tmpgv = gv_fetchpvs("stdin", GV_ADD|GV_NOTQUAL, SVt_PV);
    GvMULTI_on(tmpgv);
    GvIOp(tmpgv) = MUTABLE_IO(SvREFCNT_inc_simple(io));

    tmpgv = gv_fetchpvs("STDOUT", GV_ADD|GV_NOTQUAL, SVt_PVIO);
    GvMULTI_on(tmpgv);
    io = GvIOp(tmpgv);
    IoTYPE(io) = IoTYPE_WRONLY;
    IoOFP(io) = IoIFP(io) = PerlIO_stdout();
    setdefout(tmpgv);
    tmpgv = gv_fetchpvs("stdout", GV_ADD|GV_NOTQUAL, SVt_PV);
    GvMULTI_on(tmpgv);
    GvIOp(tmpgv) = MUTABLE_IO(SvREFCNT_inc_simple(io));

    PL_stderrgv = gv_fetchpvs("STDERR", GV_ADD|GV_NOTQUAL, SVt_PVIO);
    GvMULTI_on(PL_stderrgv);
    io = GvIOp(PL_stderrgv);
    IoTYPE(io) = IoTYPE_WRONLY;
    IoOFP(io) = IoIFP(io) = PerlIO_stderr();
    tmpgv = gv_fetchpvs("stderr", GV_ADD|GV_NOTQUAL, SVt_PV);
    GvMULTI_on(tmpgv);
    GvIOp(tmpgv) = MUTABLE_IO(SvREFCNT_inc_simple(io));

    PL_statname = newSVpvs("");		/* last filename we did stat on */
}

void
Perl_init_argv_symbols(pTHX_ int argc, char **argv)
{
    PERL_ARGS_ASSERT_INIT_ARGV_SYMBOLS;

    argc--,argv++;	/* skip name of script */
    if (PL_doswitches) {
        for (; argc > 0 && **argv == '-'; argc--,argv++) {
            char *s;
            if (!argv[0][1])
                break;
            if (argv[0][1] == '-' && !argv[0][2]) {
                argc--,argv++;
                break;
            }
            if ((s = strchr(argv[0], '='))) {
                const char *const start_name = argv[0] + 1;
                sv_setpv(GvSV(gv_fetchpvn_flags(start_name, s - start_name,
                                                TRUE, SVt_PV)), s + 1);
            }
            else
                sv_setiv(GvSV(gv_fetchpv(argv[0]+1, GV_ADD, SVt_PV)),1);
        }
    }
    if ((PL_argvgv = gv_fetchpvs("ARGV", GV_ADD|GV_NOTQUAL, SVt_PVAV))) {
        SvREFCNT_inc_simple_void_NN(PL_argvgv);
        GvMULTI_on(PL_argvgv);
        av_clear(GvAVn(PL_argvgv));
        for (; argc > 0; argc--,argv++) {
            SV * const sv = newSVpv(argv[0],0);
            av_push(GvAV(PL_argvgv),sv);
            if (!(PL_unicode & PERL_UNICODE_LOCALE_FLAG) || PL_utf8locale) {
                 if (PL_unicode & PERL_UNICODE_ARGV_FLAG)
                      SvUTF8_on(sv);
            }
            if (PL_unicode & PERL_UNICODE_WIDESYSCALLS_FLAG) /* Sarathy? */
                 (void)sv_utf8_decode(sv);
        }
    }

    if (PL_inplace && (!PL_argvgv || AvFILL(GvAV(PL_argvgv)) == -1))
        Perl_ck_warner_d(aTHX_ packWARN(WARN_INPLACE),
                         "-i used with no filenames on the command line, "
                         "reading from STDIN");
}

STATIC void
S_init_postdump_symbols(pTHX_ int argc, char **argv, char **env)
{
    GV* tmpgv;

    PERL_ARGS_ASSERT_INIT_POSTDUMP_SYMBOLS;

    PL_toptarget = newSV_type(SVt_PVIV);
    SvPVCLEAR(PL_toptarget);
    PL_bodytarget = newSV_type(SVt_PVIV);
    SvPVCLEAR(PL_bodytarget);
    PL_formtarget = PL_bodytarget;

    TAINT;

    init_argv_symbols(argc,argv);

    if ((tmpgv = gv_fetchpvs("0", GV_ADD|GV_NOTQUAL, SVt_PV))) {
        sv_setpv(GvSV(tmpgv),PL_origfilename);
    }
    if ((PL_envgv = gv_fetchpvs("ENV", GV_ADD|GV_NOTQUAL, SVt_PVHV))) {
        HV *hv;
        bool env_is_not_environ;
        SvREFCNT_inc_simple_void_NN(PL_envgv);
        GvMULTI_on(PL_envgv);
        hv = GvHVn(PL_envgv);
        hv_magic(hv, NULL, PERL_MAGIC_env);
#ifndef PERL_MICRO
#if defined(USE_ENVIRON_ARRAY) || defined(WIN32)
        /* Note that if the supplied env parameter is actually a copy
           of the global environ then it may now point to free'd memory
           if the environment has been modified since. To avoid this
           problem we treat env==NULL as meaning 'use the default'
        */
        if (!env)
            env = environ;
        env_is_not_environ = env != environ;
        if (env_is_not_environ
#  ifdef USE_ITHREADS
            && PL_curinterp == aTHX
#  endif
           )
        {
            environ[0] = NULL;
        }
        if (env) {
          HV *dups = newHV();
          char **env_copy = env;
          size_t count;

          while (*env_copy) {
              ++env_copy;
          }

          count = env_copy - env;

          if (count > PERL_HASH_DEFAULT_HvMAX) {
              /* This might be an over-estimate (due to dups and other skips),
               * but if so, likely it won't hurt much.
               * A straw poll of login environments I have suggests that
               * between 23 and 52 environment variables are typical (and no
               * dups). As the default hash size is 8 buckets, expanding in
               * advance saves between 2 and 3 splits in the loop below. */
              hv_ksplit(hv, count);
          }


          for (; *env; env++) {
              char *old_var = *env;
              char *s = strchr(old_var, '=');
              STRLEN nlen;
              SV *sv;

              if (!s || s == old_var)
                  continue;

              nlen = s - old_var;

              /* It's tempting to think that this hv_exists/hv_store pair should
               * be replaced with a single hv_fetch with the LVALUE flag true.
               * However, hv has magic, and if you follow the code in hv_common
               * then for LVALUE fetch it recurses once, whereas exists and
               * store do not recurse. Hence internally there would be no
               * difference in the complexity of the code run. Moreover, all
               * calls pass through "is there magic?" special case code, which
               * in turn has its own #ifdef ENV_IS_CASELESS special case special
               * case. Hence this code shouldn't change, as doing so won't give
               * any meaningful speedup, and might well add bugs. */

            if (hv_exists(hv, old_var, nlen)) {
                SV **dup;
                const char *name = savepvn(old_var, nlen);

                /* make sure we use the same value as getenv(), otherwise code that
                   uses getenv() (like setlocale()) might see a different value to %ENV
                 */
                sv = newSVpv(PerlEnv_getenv(name), 0);

                /* keep a count of the dups of this name so we can de-dup environ later */
                dup = hv_fetch(dups, name, nlen, TRUE);
                if (*dup) {
                    sv_inc(*dup);
                }

                Safefree(name);
            }
            else {
                sv = newSVpv(s+1, 0);
            }
            (void)hv_store(hv, old_var, nlen, sv, 0);
            if (env_is_not_environ)
                mg_set(sv);
          }
          if (HvTOTALKEYS(dups)) {
              /* environ has some duplicate definitions, remove them */
              HE *entry;
              hv_iterinit(dups);
              while ((entry = hv_iternext_flags(dups, 0))) {
                  STRLEN nlen;
                  const char *name = HePV(entry, nlen);
                  IV count = SvIV(HeVAL(entry));
                  IV i;
                  SV **valp = hv_fetch(hv, name, nlen, 0);

                  assert(valp);

                  /* try to remove any duplicate names, depending on the
                   * implementation used in my_setenv() the iteration might
                   * not be necessary, but let's be safe.
                   */
                  for (i = 0; i < count; ++i)
                      my_setenv(name, 0);

                  /* and set it back to the value we set $ENV{name} to */
                  my_setenv(name, SvPV_nolen(*valp));
              }
          }
          SvREFCNT_dec_NN(dups);
      }
#endif /* USE_ENVIRON_ARRAY */
#endif /* !PERL_MICRO */
    }
    TAINT_NOT;

    /* touch @F array to prevent spurious warnings 20020415 MJD */
    if (PL_minus_a) {
      (void) get_av("main::F", GV_ADD | GV_ADDMULTI);
    }
}

STATIC void
S_init_perllib(pTHX)
{
#ifndef VMS
    const char *perl5lib = NULL;
#endif
    const char *s;
#if defined(WIN32) && !defined(PERL_IS_MINIPERL)
    STRLEN len;
#endif

    if (!TAINTING_get) {
#ifndef VMS
        perl5lib = PerlEnv_getenv("PERL5LIB");
        if (perl5lib && *perl5lib != '\0')
            incpush_use_sep(perl5lib, 0, INCPUSH_ADD_SUB_DIRS);
        else {
            s = PerlEnv_getenv("PERLLIB");
            if (s)
                incpush_use_sep(s, 0, 0);
        }
#else /* VMS */
        /* Treat PERL5?LIB as a possible search list logical name -- the
         * "natural" VMS idiom for a Unix path string.  We allow each
         * element to be a set of |-separated directories for compatibility.
         */
        char buf[256];
        int idx = 0;
        if (vmstrnenv("PERL5LIB",buf,0,NULL,0))
            do {
                incpush_use_sep(buf, 0, INCPUSH_ADD_SUB_DIRS);
            } while (vmstrnenv("PERL5LIB",buf,++idx,NULL,0));
        else {
            while (vmstrnenv("PERLLIB",buf,idx++,NULL,0))
                incpush_use_sep(buf, 0, 0);
        }
#endif /* VMS */
    }

#ifndef PERL_IS_MINIPERL
    /* miniperl gets just -I..., the split of $ENV{PERL5LIB}, and "." in @INC
       (and not the architecture specific directories from $ENV{PERL5LIB}) */

#include "perl_inc_macro.h"
/* Use the ~-expanded versions of APPLLIB (undocumented),
    SITEARCH, SITELIB, VENDORARCH, VENDORLIB, ARCHLIB and PRIVLIB
*/
    INCPUSH_APPLLIB_EXP
    INCPUSH_SITEARCH_EXP
    INCPUSH_SITELIB_EXP
    INCPUSH_PERL_VENDORARCH_EXP
    INCPUSH_PERL_VENDORLIB_EXP
    INCPUSH_ARCHLIB_EXP
    INCPUSH_PRIVLIB_EXP
    INCPUSH_PERL_OTHERLIBDIRS
    INCPUSH_PERL5LIB
    INCPUSH_APPLLIB_OLD_EXP
    INCPUSH_SITELIB_STEM
    INCPUSH_PERL_VENDORLIB_STEM
    INCPUSH_PERL_OTHERLIBDIRS_ARCHONLY

#endif /* !PERL_IS_MINIPERL */

    if (!TAINTING_get) {
#if !defined(PERL_IS_MINIPERL) && defined(DEFAULT_INC_EXCLUDES_DOT)
        const char * const unsafe = PerlEnv_getenv("PERL_USE_UNSAFE_INC");
        if (unsafe && strEQ(unsafe, "1"))
#endif
          S_incpush(aTHX_ STR_WITH_LEN("."), 0);
    }
}

#if defined(DOSISH)
#    define PERLLIB_SEP ';'
#elif defined(__VMS)
#    define PERLLIB_SEP PL_perllib_sep
#else
#    define PERLLIB_SEP ':'
#endif
#ifndef PERLLIB_MANGLE
#  define PERLLIB_MANGLE(s,n) (s)
#endif

#ifndef PERL_IS_MINIPERL
/* Push a directory onto @INC if it exists.
   Generate a new SV if we do this, to save needing to copy the SV we push
   onto @INC  */
STATIC SV *
S_incpush_if_exists(pTHX_ AV *const av, SV *dir, SV *const stem)
{
    Stat_t tmpstatbuf;

    PERL_ARGS_ASSERT_INCPUSH_IF_EXISTS;

    if (PerlLIO_stat(SvPVX_const(dir), &tmpstatbuf) >= 0 &&
        S_ISDIR(tmpstatbuf.st_mode)) {
        av_push(av, dir);
        dir = newSVsv(stem);
    } else {
        /* Truncate dir back to stem.  */
        SvCUR_set(dir, SvCUR(stem));
    }
    return dir;
}
#endif

STATIC SV *
S_mayberelocate(pTHX_ const char *const dir, STRLEN len, U32 flags)
{
    const U8 canrelocate = (U8)flags & INCPUSH_CAN_RELOCATE;
    SV *libdir;

    PERL_ARGS_ASSERT_MAYBERELOCATE;
    assert(len > 0);

    /* I am not convinced that this is valid when PERLLIB_MANGLE is
       defined to so something (in os2/os2.c), but the code has been
       this way, ignoring any possible changed of length, since
       760ac839baf413929cd31cc32ffd6dba6b781a81 (5.003_02) so I'll leave
       it be.  */
    libdir = newSVpvn(PERLLIB_MANGLE(dir, len), len);

#ifdef VMS
    {
        char *unix;

        if ((unix = tounixspec_ts(SvPV(libdir,len),NULL)) != NULL) {
            len = strlen(unix);
            while (len > 1 && unix[len-1] == '/') len--;  /* Cosmetic */
            sv_usepvn(libdir,unix,len);
        }
        else
            PerlIO_printf(Perl_error_log,
                          "Failed to unixify @INC element \"%s\"\n",
                          SvPV_nolen_const(libdir));
    }
#endif

        /* Do the if() outside the #ifdef to avoid warnings about an unused
           parameter.  */
        if (canrelocate) {
#ifdef PERL_RELOCATABLE_INC
        /*
         * Relocatable include entries are marked with a leading .../
         *
         * The algorithm is
         * 0: Remove that leading ".../"
         * 1: Remove trailing executable name (anything after the last '/')
         *    from the perl path to give a perl prefix
         * Then
         * While the @INC element starts "../" and the prefix ends with a real
         * directory (ie not . or ..) chop that real directory off the prefix
         * and the leading "../" from the @INC element. ie a logical "../"
         * cleanup
         * Finally concatenate the prefix and the remainder of the @INC element
         * The intent is that /usr/local/bin/perl and .../../lib/perl5
         * generates /usr/local/lib/perl5
         */
            const char *libpath = SvPVX(libdir);
            STRLEN libpath_len = SvCUR(libdir);
            if (memBEGINs(libpath, libpath_len, ".../")) {
                /* Game on!  */
                SV * const caret_X = get_sv("\030", 0);
                /* Going to use the SV just as a scratch buffer holding a C
                   string:  */
                SV *prefix_sv;
                char *prefix;
                char *lastslash;

                /* $^X is *the* source of taint if tainting is on, hence
                   SvPOK() won't be true.  */
                assert(caret_X);
                assert(SvPOKp(caret_X));
                prefix_sv = newSVpvn_flags(SvPVX(caret_X), SvCUR(caret_X),
                                           SvUTF8(caret_X));
                /* Firstly take off the leading .../
                   If all else fail we'll do the paths relative to the current
                   directory.  */
                sv_chop(libdir, libpath + 4);
                /* Don't use SvPV as we're intentionally bypassing taining,
                   mortal copies that the mg_get of tainting creates, and
                   corruption that seems to come via the save stack.
                   I guess that the save stack isn't correctly set up yet.  */
                libpath = SvPVX(libdir);
                libpath_len = SvCUR(libdir);

                prefix = SvPVX(prefix_sv);
                lastslash = (char *) my_memrchr(prefix, '/',
                             SvEND(prefix_sv) - prefix);

                /* First time in with the *lastslash = '\0' we just wipe off
                   the trailing /perl from (say) /usr/foo/bin/perl
                */
                if (lastslash) {
                    SV *tempsv;
                    while ((*lastslash = '\0'), /* Do that, come what may.  */
                           (   memBEGINs(libpath, libpath_len, "../")
                            && (lastslash =
                                  (char *) my_memrchr(prefix, '/',
                                                   SvEND(prefix_sv) - prefix))))
                    {
                        if (lastslash[1] == '\0'
                            || (lastslash[1] == '.'
                                && (lastslash[2] == '/' /* ends "/."  */
                                    || (lastslash[2] == '/'
                                        && lastslash[3] == '/' /* or "/.."  */
                                        )))) {
                            /* Prefix ends "/" or "/." or "/..", any of which
                               are fishy, so don't do any more logical cleanup.
                            */
                            break;
                        }
                        /* Remove leading "../" from path  */
                        libpath += 3;
                        libpath_len -= 3;
                        /* Next iteration round the loop removes the last
                           directory name from prefix by writing a '\0' in
                           the while clause.  */
                    }
                    /* prefix has been terminated with a '\0' to the correct
                       length. libpath points somewhere into the libdir SV.
                       We need to join the 2 with '/' and drop the result into
                       libdir.  */
                    tempsv = Perl_newSVpvf(aTHX_ "%s/%s", prefix, libpath);
                    SvREFCNT_dec(libdir);
                    /* And this is the new libdir.  */
                    libdir = tempsv;
                    if (TAINTING_get &&
                        (PerlProc_getuid() != PerlProc_geteuid() ||
                         PerlProc_getgid() != PerlProc_getegid())) {
                        /* Need to taint relocated paths if running set ID  */
                        SvTAINTED_on(libdir);
                    }
                }
                SvREFCNT_dec(prefix_sv);
            }
#endif
        }
    return libdir;
}

STATIC void
S_incpush(pTHX_ const char *const dir, STRLEN len, U32 flags)
{
#ifndef PERL_IS_MINIPERL
    const U8 using_sub_dirs
        = (U8)flags & (INCPUSH_ADD_VERSIONED_SUB_DIRS
                       |INCPUSH_ADD_ARCHONLY_SUB_DIRS|INCPUSH_ADD_OLD_VERS);
    const U8 add_versioned_sub_dirs
        = (U8)flags & INCPUSH_ADD_VERSIONED_SUB_DIRS;
    const U8 add_archonly_sub_dirs
        = (U8)flags & INCPUSH_ADD_ARCHONLY_SUB_DIRS;
#ifdef PERL_INC_VERSION_LIST
    const U8 addoldvers  = (U8)flags & INCPUSH_ADD_OLD_VERS;
#endif
#endif
    const U8 unshift     = (U8)flags & INCPUSH_UNSHIFT;
    const U8 push_basedir = (flags & INCPUSH_NOT_BASEDIR) ? 0 : 1;
    AV *const inc = GvAVn(PL_incgv);

    PERL_ARGS_ASSERT_INCPUSH;
    assert(len > 0);

    /* Could remove this vestigial extra block, if we don't mind a lot of
       re-indenting diff noise.  */
    {
        SV *const libdir = mayberelocate(dir, len, flags);
        /* Change 20189146be79a0596543441fa369c6bf7f85103f, to fix RT#6665,
           arranged to unshift #! line -I onto the front of @INC. However,
           -I can add version and architecture specific libraries, and they
           need to go first. The old code assumed that it was always
           pushing. Hence to make it work, need to push the architecture
           (etc) libraries onto a temporary array, then "unshift" that onto
           the front of @INC.  */
#ifndef PERL_IS_MINIPERL
        AV *const av = (using_sub_dirs) ? (unshift ? newAV() : inc) : NULL;

        /*
         * BEFORE pushing libdir onto @INC we may first push version- and
         * archname-specific sub-directories.
         */
        if (using_sub_dirs) {
            SV *subdir = newSVsv(libdir);
#ifdef PERL_INC_VERSION_LIST
            /* Configure terminates PERL_INC_VERSION_LIST with a NULL */
            const char * const incverlist[] = { PERL_INC_VERSION_LIST };
            const char * const *incver;
#endif

            if (add_versioned_sub_dirs) {
                /* .../version/archname if -d .../version/archname */
                sv_catpvs(subdir, "/" PERL_FS_VERSION "/" ARCHNAME);
                subdir = S_incpush_if_exists(aTHX_ av, subdir, libdir);

                /* .../version if -d .../version */
                sv_catpvs(subdir, "/" PERL_FS_VERSION);
                subdir = S_incpush_if_exists(aTHX_ av, subdir, libdir);
            }

#ifdef PERL_INC_VERSION_LIST
            if (addoldvers) {
                for (incver = incverlist; *incver; incver++) {
                    /* .../xxx if -d .../xxx */
                    Perl_sv_catpvf(aTHX_ subdir, "/%s", *incver);
                    subdir = S_incpush_if_exists(aTHX_ av, subdir, libdir);
                }
            }
#endif

            if (add_archonly_sub_dirs) {
                /* .../archname if -d .../archname */
                sv_catpvs(subdir, "/" ARCHNAME);
                subdir = S_incpush_if_exists(aTHX_ av, subdir, libdir);

            }

            assert (SvREFCNT(subdir) == 1);
            SvREFCNT_dec(subdir);
        }
#endif /* !PERL_IS_MINIPERL */
        /* finally add this lib directory at the end of @INC */
        if (unshift) {
#ifdef PERL_IS_MINIPERL
            const Size_t extra = 0;
#else
            Size_t extra = av_count(av);
#endif
            av_unshift(inc, extra + push_basedir);
            if (push_basedir)
                av_store(inc, extra, libdir);
#ifndef PERL_IS_MINIPERL
            while (extra--) {
                /* av owns a reference, av_store() expects to be donated a
                   reference, and av expects to be sane when it's cleared.
                   If I wanted to be naughty and wrong, I could peek inside the
                   implementation of av_clear(), realise that it uses
                   SvREFCNT_dec() too, so av's array could be a run of NULLs,
                   and so directly steal from it (with a memcpy() to inc, and
                   then memset() to NULL them out. But people copy code from the
                   core expecting it to be best practise, so let's use the API.
                   Although studious readers will note that I'm not checking any
                   return codes.  */
                av_store(inc, extra, SvREFCNT_inc(*av_fetch(av, extra, FALSE)));
            }
            SvREFCNT_dec(av);
#endif
        }
        else if (push_basedir) {
            av_push(inc, libdir);
        }

        if (!push_basedir) {
            assert (SvREFCNT(libdir) == 1);
            SvREFCNT_dec(libdir);
        }
    }
}

STATIC void
S_incpush_use_sep(pTHX_ const char *p, STRLEN len, U32 flags)
{
    const char *s;
    const char *end;
    /* This logic has been broken out from S_incpush(). It may be possible to
       simplify it.  */

    PERL_ARGS_ASSERT_INCPUSH_USE_SEP;

    /* perl compiled with -DPERL_RELOCATABLE_INCPUSH will ignore the len
     * argument to incpush_use_sep.  This allows creation of relocatable
     * Perl distributions that patch the binary at install time.  Those
     * distributions will have to provide their own relocation tools; this
     * is not a feature otherwise supported by core Perl.
     */
#ifndef PERL_RELOCATABLE_INCPUSH
    if (!len)
#endif
        len = strlen(p);

    end = p + len;

    /* Break at all separators */
    while ((s = (const char*)memchr(p, PERLLIB_SEP, end - p))) {
        if (s == p) {
            /* skip any consecutive separators */

            /* Uncomment the next line for PATH semantics */
            /* But you'll need to write tests */
            /* av_push(GvAVn(PL_incgv), newSVpvs(".")); */
        } else {
            incpush(p, (STRLEN)(s - p), flags);
        }
        p = s + 1;
    }
    if (p != end)
        incpush(p, (STRLEN)(end - p), flags);

}

void
Perl_call_list(pTHX_ I32 oldscope, AV *paramList)
{
    SV *atsv;
    volatile const line_t oldline = PL_curcop ? CopLINE(PL_curcop) : 0;
    CV *cv;
    STRLEN len;
    int ret;
    dJMPENV;

    PERL_ARGS_ASSERT_CALL_LIST;

    while (av_count(paramList) > 0) {
        cv = MUTABLE_CV(av_shift(paramList));
        if (PL_savebegin) {
            if (paramList == PL_beginav) {
                /* save PL_beginav for compiler */
                Perl_av_create_and_push(aTHX_ &PL_beginav_save, MUTABLE_SV(cv));
            }
            else if (paramList == PL_checkav) {
                /* save PL_checkav for compiler */
                Perl_av_create_and_push(aTHX_ &PL_checkav_save, MUTABLE_SV(cv));
            }
            else if (paramList == PL_unitcheckav) {
                /* save PL_unitcheckav for compiler */
                Perl_av_create_and_push(aTHX_ &PL_unitcheckav_save, MUTABLE_SV(cv));
            }
        } else {
            SAVEFREESV(cv);
        }
        JMPENV_PUSH(ret);
        switch (ret) {
        case 0:
            CALL_LIST_BODY(cv);
            atsv = ERRSV;
            (void)SvPV_const(atsv, len);
            if (len) {
                PL_curcop = &PL_compiling;
                CopLINE_set(PL_curcop, oldline);
                if (paramList == PL_beginav)
                    sv_catpvs(atsv, "BEGIN failed--compilation aborted");
                else
                    Perl_sv_catpvf(aTHX_ atsv,
                                   "%s failed--call queue aborted",
                                   paramList == PL_checkav ? "CHECK"
                                   : paramList == PL_initav ? "INIT"
                                   : paramList == PL_unitcheckav ? "UNITCHECK"
                                   : "END");
                while (PL_scopestack_ix > oldscope)
                    LEAVE;
                JMPENV_POP;
                Perl_croak(aTHX_ "%" SVf, SVfARG(atsv));
            }
            break;
        case 1:
            STATUS_ALL_FAILURE;
            /* FALLTHROUGH */
        case 2:
            /* my_exit() was called */
            while (PL_scopestack_ix > oldscope)
                LEAVE;
            FREETMPS;
            SET_CURSTASH(PL_defstash);
            PL_curcop = &PL_compiling;
            CopLINE_set(PL_curcop, oldline);
            JMPENV_POP;
            my_exit_jump();
            NOT_REACHED; /* NOTREACHED */
        case 3:
            if (PL_restartop) {
                PL_curcop = &PL_compiling;
                CopLINE_set(PL_curcop, oldline);
                JMPENV_JUMP(3);
            }
            PerlIO_printf(Perl_error_log, "panic: restartop in call_list\n");
            FREETMPS;
            break;
        }
        JMPENV_POP;
    }
}

/*
=for apidoc my_exit

A wrapper for the C library L<exit(3)>, honoring what L<perlapi/PL_exit_flags>
say to do.

=cut
*/

void
Perl_my_exit(pTHX_ U32 status)
{
    if (PL_exit_flags & PERL_EXIT_ABORT) {
        abort();
    }
    if (PL_exit_flags & PERL_EXIT_WARN) {
        PL_exit_flags |= PERL_EXIT_ABORT; /* Protect against reentrant calls */
        Perl_warn(aTHX_ "Unexpected exit %lu", (unsigned long)status);
        PL_exit_flags &= ~PERL_EXIT_ABORT;
    }
    switch (status) {
    case 0:
        STATUS_ALL_SUCCESS;
        break;
    case 1:
        STATUS_ALL_FAILURE;
        break;
    default:
        STATUS_EXIT_SET(status);
        break;
    }
    my_exit_jump();
}

/*
=for apidoc my_failure_exit

Exit the running Perl process with an error.

On non-VMS platforms, this is essentially equivalent to L</C<my_exit>>, using
C<errno>, but forces an en error code of 255 if C<errno> is 0.

On VMS, it takes care to set the appropriate severity bits in the exit status.

=cut
*/

void
Perl_my_failure_exit(pTHX)
{
#ifdef VMS
     /* We have been called to fall on our sword.  The desired exit code
      * should be already set in STATUS_UNIX, but could be shifted over
      * by 8 bits.  STATUS_UNIX_EXIT_SET will handle the cases where a
      * that code is set.
      *
      * If an error code has not been set, then force the issue.
      */
    if (MY_POSIX_EXIT) {

        /* According to the die_exit.t tests, if errno is non-zero */
        /* It should be used for the error status. */

        if (errno == EVMSERR) {
            STATUS_NATIVE = vaxc$errno;
        } else {

            /* According to die_exit.t tests, if the child_exit code is */
            /* also zero, then we need to exit with a code of 255 */
            if ((errno != 0) && (errno < 256))
                STATUS_UNIX_EXIT_SET(errno);
            else if (STATUS_UNIX < 255) {
                STATUS_UNIX_EXIT_SET(255);
            }

        }

        /* The exit code could have been set by $? or vmsish which
         * means that it may not have fatal set.  So convert
         * success/warning codes to fatal with out changing
         * the POSIX status code.  The severity makes VMS native
         * status handling work, while UNIX mode programs use the
         * POSIX exit codes.
         */
         if ((STATUS_NATIVE & (STS$K_SEVERE|STS$K_ERROR)) == 0) {
            STATUS_NATIVE &= STS$M_COND_ID;
            STATUS_NATIVE |= STS$K_ERROR | STS$M_INHIB_MSG;
         }
    }
    else {
        /* Traditionally Perl on VMS always expects a Fatal Error. */
        if (vaxc$errno & 1) {

            /* So force success status to failure */
            if (STATUS_NATIVE & 1)
                STATUS_ALL_FAILURE;
        }
        else {
            if (!vaxc$errno) {
                STATUS_UNIX = EINTR; /* In case something cares */
                STATUS_ALL_FAILURE;
            }
            else {
                int severity;
                STATUS_NATIVE = vaxc$errno; /* Should already be this */

                /* Encode the severity code */
                severity = STATUS_NATIVE & STS$M_SEVERITY;
                STATUS_UNIX = (severity ? severity : 1) << 8;

                /* Perl expects this to be a fatal error */
                if (severity != STS$K_SEVERE)
                    STATUS_ALL_FAILURE;
            }
        }
    }

#else
    int exitstatus;
    int eno = errno;
    if (eno & 255)
        STATUS_UNIX_SET(eno);
    else {
        exitstatus = STATUS_UNIX >> 8;
        if (exitstatus & 255)
            STATUS_UNIX_SET(exitstatus);
        else
            STATUS_UNIX_SET(255);
    }
#endif
    if (PL_exit_flags & PERL_EXIT_ABORT) {
        abort();
    }
    if (PL_exit_flags & PERL_EXIT_WARN) {
        PL_exit_flags |= PERL_EXIT_ABORT; /* Protect against reentrant calls */
        Perl_warn(aTHX_ "Unexpected exit failure %ld", (long)PL_statusvalue);
        PL_exit_flags &= ~PERL_EXIT_ABORT;
    }
    my_exit_jump();
}

STATIC void
S_my_exit_jump(pTHX)
{
    if (PL_e_script) {
        SvREFCNT_dec(PL_e_script);
        PL_e_script = NULL;
    }

    POPSTACK_TO(PL_mainstack);
    if (cxstack_ix >= 0) {
        dounwind(-1);
        cx_popblock(cxstack);
    }
    LEAVE_SCOPE(0);

    JMPENV_JUMP(2);
}

static I32
read_e_script(pTHX_ int idx, SV *buf_sv, int maxlen)
{
    const char * const p  = SvPVX_const(PL_e_script);
    const char * const e  = SvEND(PL_e_script);
    const char *nl = (char *) memchr(p, '\n', e - p);

    PERL_UNUSED_ARG(idx);
    PERL_UNUSED_ARG(maxlen);

    nl = (nl) ? nl+1 : e;
    if (nl-p == 0) {
        filter_del(read_e_script);
        return 0;
    }
    sv_catpvn(buf_sv, p, nl-p);
    sv_chop(PL_e_script, nl);
    return 1;
}

/* removes boilerplate code at the end of each boot_Module xsub */
void
Perl_xs_boot_epilog(pTHX_ const I32 ax)
{
  if (PL_unitcheckav)
        call_list(PL_scopestack_ix, PL_unitcheckav);
    XSRETURN_YES;
}

/*
 * ex: set ts=8 sts=4 sw=4 et:
 */
