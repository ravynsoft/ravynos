/*
 * perlio.c
 * Copyright (c) 1996-2006, Nick Ing-Simmons
 * Copyright (c) 2006, 2007, 2008, 2009, 2010, 2011 Larry Wall and others
 *
 * You may distribute under the terms of either the GNU General Public License
 * or the Artistic License, as specified in the README file.
 */

/*
 * Hour after hour for nearly three weary days he had jogged up and down,
 * over passes, and through long dales, and across many streams.
 *
 *     [pp.791-792 of _The Lord of the Rings_, V/iii: "The Muster of Rohan"]
 */

/* This file contains the functions needed to implement PerlIO, which
 * is Perl's private replacement for the C stdio library. This is used
 * by default unless you compile with -Uuseperlio or run with
 * PERLIO=:stdio (but don't do this unless you know what you're doing)
 */

/*
 * If we have ActivePerl-like PERL_IMPLICIT_SYS then we need a dTHX to get
 * at the dispatch tables, even when we do not need it for other reasons.
 * Invent a dSYS macro to abstract this out
 */
#ifdef PERL_IMPLICIT_SYS
#  define dSYS dTHX
#else
#  define dSYS dNOOP
#endif

#define PERLIO_NOT_STDIO 0
/*
 * This file provides those parts of PerlIO abstraction
 * which are not #defined in perlio.h.
 * Which these are depends on various Configure #ifdef's
 */

#include "EXTERN.h"
#define PERL_IN_PERLIO_C
#include "perl.h"

#ifdef MULTIPLICITY
#  undef dSYS
#  define dSYS dTHX
#endif

#include "XSUB.h"

#ifdef VMS
#  include <rms.h>
#endif

#define PerlIO_lockcnt(f) (((PerlIOl*)(void*)(f))->head->flags)

/* Call the callback or PerlIOBase, and return failure. */
#define Perl_PerlIO_or_Base(f, callback, base, failure, args) 	\
        if (PerlIOValid(f)) {					\
                const PerlIO_funcs * const tab = PerlIOBase(f)->tab;\
                if (tab && tab->callback)			\
                        return (*tab->callback) args;		\
                else						\
                        return PerlIOBase_ ## base args;	\
        }							\
        else							\
                SETERRNO(EBADF, SS_IVCHAN);			\
        return failure

/* Call the callback or fail, and return failure. */
#define Perl_PerlIO_or_fail(f, callback, failure, args) 	\
        if (PerlIOValid(f)) {					\
                const PerlIO_funcs * const tab = PerlIOBase(f)->tab;\
                if (tab && tab->callback)			\
                        return (*tab->callback) args;		\
                SETERRNO(EINVAL, LIB_INVARG);			\
        }							\
        else							\
                SETERRNO(EBADF, SS_IVCHAN);			\
        return failure

/* Call the callback or PerlIOBase, and be void. */
#define Perl_PerlIO_or_Base_void(f, callback, base, args) 	\
        if (PerlIOValid(f)) {					\
                const PerlIO_funcs * const tab = PerlIOBase(f)->tab;\
                if (tab && tab->callback)			\
                        (*tab->callback) args;			\
                else						\
                        PerlIOBase_ ## base args;		\
        }							\
        else							\
                SETERRNO(EBADF, SS_IVCHAN)

/* Call the callback or fail, and be void. */
#define Perl_PerlIO_or_fail_void(f, callback, args) 		\
        if (PerlIOValid(f)) {					\
                const PerlIO_funcs * const tab = PerlIOBase(f)->tab;\
                if (tab && tab->callback)			\
                        (*tab->callback) args;			\
                else						\
                        SETERRNO(EINVAL, LIB_INVARG);		\
        }							\
        else							\
                SETERRNO(EBADF, SS_IVCHAN)

#if defined(__osf__) && _XOPEN_SOURCE < 500
extern int   fseeko(FILE *, off_t, int);
extern off_t ftello(FILE *);
#endif

#define NATIVE_0xd  CR_NATIVE
#define NATIVE_0xa  LF_NATIVE

EXTERN_C int perlsio_binmode(FILE *fp, int iotype, int mode);

int
perlsio_binmode(FILE *fp, int iotype, int mode)
{
    /*
     * This used to be contents of do_binmode in doio.c
     */
#ifdef DOSISH
    dTHX;
    PERL_UNUSED_ARG(iotype);
    if (PerlLIO_setmode(fileno(fp), mode) != -1) {
        return 1;
    }
    else
        return 0;
#else
#  if defined(USEMYBINMODE)
    dTHX;
#    if defined(__CYGWIN__)
    PERL_UNUSED_ARG(iotype);
#    endif
    if (my_binmode(fp, iotype, mode) != FALSE)
        return 1;
    else
        return 0;
#  else
    PERL_UNUSED_ARG(fp);
    PERL_UNUSED_ARG(iotype);
    PERL_UNUSED_ARG(mode);
    return 1;
#  endif
#endif
}

#ifndef O_ACCMODE
#  define O_ACCMODE 3             /* Assume traditional implementation */
#endif

int
PerlIO_intmode2str(int rawmode, char *mode, int *writing)
{
    const int result = rawmode & O_ACCMODE;
    int ix = 0;
    int ptype;
    switch (result) {
    case O_RDONLY:
        ptype = IoTYPE_RDONLY;
        break;
    case O_WRONLY:
        ptype = IoTYPE_WRONLY;
        break;
    case O_RDWR:
    default:
        ptype = IoTYPE_RDWR;
        break;
    }
    if (writing)
        *writing = (result != O_RDONLY);

    if (result == O_RDONLY) {
        mode[ix++] = 'r';
    }
#ifdef O_APPEND
    else if (rawmode & O_APPEND) {
        mode[ix++] = 'a';
        if (result != O_WRONLY)
            mode[ix++] = '+';
    }
#endif
    else {
        if (result == O_WRONLY)
            mode[ix++] = 'w';
        else {
            mode[ix++] = 'r';
            mode[ix++] = '+';
        }
    }
#if O_BINARY != 0
    /* Unless O_BINARY is different from zero, bit-and:ing
     * with it won't do much good. */
    if (rawmode & O_BINARY)
        mode[ix++] = 'b';
#endif
    mode[ix] = '\0';
    return ptype;
}

#ifndef PERLIO_LAYERS
int
PerlIO_apply_layers(pTHX_ PerlIO *f, const char *mode, const char *names)
{
    if (!names || !*names
        || strEQ(names, ":crlf")
        || strEQ(names, ":raw")
        || strEQ(names, ":bytes")
       ) {
        return 0;
    }
    Perl_croak(aTHX_ "Cannot apply \"%s\" in non-PerlIO perl", names);
    /*
     * NOTREACHED
     */
    return -1;
}

void
PerlIO_destruct(pTHX)
{
}

int
PerlIO_binmode(pTHX_ PerlIO *fp, int iotype, int mode, const char *names)
{
    return perlsio_binmode(fp, iotype, mode);
}

PerlIO *
PerlIO_fdupopen(pTHX_ PerlIO *f, CLONE_PARAMS *param, int flags)
{
#  if defined(PERL_MICRO)
    return NULL;
#  elif defined(PERL_IMPLICIT_SYS)
    return PerlSIO_fdupopen(f);
#  else
#    ifdef WIN32
    return win32_fdupopen(f);
#    else
    if (f) {
        const int fd = PerlLIO_dup_cloexec(PerlIO_fileno(f));
        if (fd >= 0) {
            char mode[8];
            const int omode = fcntl(fd, F_GETFL);
            PerlIO_intmode2str(omode,mode,NULL);
            /* the r+ is a hack */
            return PerlIO_fdopen(fd, mode);
        }
        return NULL;
    }
    else {
        SETERRNO(EBADF, SS_IVCHAN);
    }
#    endif
    return NULL;
#  endif
}


/*
 * De-mux PerlIO_openn() into fdopen, freopen and fopen type entries
 */

PerlIO *
PerlIO_openn(pTHX_ const char *layers, const char *mode, int fd,
             int imode, int perm, PerlIO *old, int narg, SV **args)
{
    if (narg) {
        if (narg > 1) {
            Perl_croak(aTHX_ "More than one argument to open");
        }
        if (*args == &PL_sv_undef)
            return PerlIO_tmpfile();
        else {
            STRLEN len;
            const char *name = SvPV_const(*args, len);
            if (!IS_SAFE_PATHNAME(name, len, "open"))
                return NULL;

            if (*mode == IoTYPE_NUMERIC) {
                fd = PerlLIO_open3_cloexec(name, imode, perm);
                if (fd >= 0)
                    return PerlIO_fdopen(fd, mode + 1);
            }
            else if (old) {
                return PerlIO_reopen(name, mode, old);
            }
            else {
                return PerlIO_open(name, mode);
            }
        }
    }
    else {
        return PerlIO_fdopen(fd, mode);
    }
    return NULL;
}

XS(XS_PerlIO__Layer__find); /* prototype to pass -Wmissing-prototypes */
XS(XS_PerlIO__Layer__find)
{
    dXSARGS;
    if (items < 2)
        Perl_croak(aTHX_ "Usage class->find(name[,load])");
    else {
        const char * const name = SvPV_nolen_const(ST(1));
        ST(0) = (strEQ(name, "crlf")
                 || strEQ(name, "raw")) ? &PL_sv_yes : &PL_sv_undef;
        XSRETURN(1);
    }
}


void
Perl_boot_core_PerlIO(pTHX)
{
    newXS("PerlIO::Layer::find", XS_PerlIO__Layer__find, __FILE__);
}

#endif


/*======================================================================================*/
/*
 * Implement all the PerlIO interface ourselves.
 */

#include "perliol.h"

void
PerlIO_debug(const char *fmt, ...)
{
    va_list ap;
    dSYS;

    if (!DEBUG_i_TEST)
        return;

    va_start(ap, fmt);

    if (!PL_perlio_debug_fd) {
        if (!TAINTING_get &&
            PerlProc_getuid() == PerlProc_geteuid() &&
            PerlProc_getgid() == PerlProc_getegid()) {
            const char * const s = PerlEnv_getenv("PERLIO_DEBUG");
            if (s && *s)
                PL_perlio_debug_fd = PerlLIO_open3_cloexec(s,
                                        O_WRONLY | O_CREAT | O_APPEND, 0666);
            else
                PL_perlio_debug_fd = PerlLIO_dup_cloexec(2); /* stderr */
        } else {
            /* tainting or set*id, so ignore the environment and send the
               debug output to stderr, like other -D switches.  */
            PL_perlio_debug_fd = PerlLIO_dup_cloexec(2); /* stderr */
        }
    }
    if (PL_perlio_debug_fd > 0) {
#ifdef USE_ITHREADS
        const char * const s = CopFILE(PL_curcop);
        /* Use fixed buffer as sv_catpvf etc. needs SVs */
        char buffer[1024];
        const STRLEN len1 = my_snprintf(buffer, sizeof(buffer), "%.40s:%" LINE_Tf " ", s ? s : "(none)", CopLINE(PL_curcop));
#  ifdef USE_QUADMATH
#    ifdef HAS_VSNPRINTF
        /* my_vsnprintf() isn't available with quadmath, but the native vsnprintf()
           should be, otherwise the system isn't likely to support quadmath.
           Nothing should be calling PerlIO_debug() with floating point anyway.
        */
        DECLARATION_FOR_LC_NUMERIC_MANIPULATION;
        STORE_LC_NUMERIC_SET_TO_NEEDED();
        const STRLEN len2 = vsnprintf(buffer + len1, sizeof(buffer) - len1, fmt, ap);
        RESTORE_LC_NUMERIC();
#    else
        STATIC_ASSERT_STMT(0);
#    endif
#  else
        const STRLEN len2 = my_vsnprintf(buffer + len1, sizeof(buffer) - len1, fmt, ap);
#  endif
        PERL_UNUSED_RESULT(PerlLIO_write(PL_perlio_debug_fd, buffer, len1 + len2));
#else
        const char *s = CopFILE(PL_curcop);
        STRLEN len;
        SV * const sv = Perl_newSVpvf(aTHX_ "%s:%" LINE_Tf " ",
                                      s ? s : "(none)", CopLINE(PL_curcop));
        Perl_sv_vcatpvf(aTHX_ sv, fmt, &ap);

        s = SvPV_const(sv, len);
        PERL_UNUSED_RESULT(PerlLIO_write(PL_perlio_debug_fd, s, len));
        SvREFCNT_dec(sv);
#endif
    }
    va_end(ap);
}

/*--------------------------------------------------------------------------------------*/

/*
 * Inner level routines
 */

/* check that the head field of each layer points back to the head */

#ifdef DEBUGGING
#  define VERIFY_HEAD(f) PerlIO_verify_head(aTHX_ f)
static void
PerlIO_verify_head(pTHX_ PerlIO *f)
{
    PerlIOl *head, *p;
    int seen = 0;
#  ifndef PERL_IMPLICIT_SYS
    PERL_UNUSED_CONTEXT;
#  endif
    if (!PerlIOValid(f))
        return;
    p = head = PerlIOBase(f)->head;
    assert(p);
    do {
        assert(p->head == head);
        if (&p->next == f)
            seen = 1;
        p = p->next;
    } while (p);
    assert(seen);
}
#else
#  define VERIFY_HEAD(f)
#endif


/*
 * Table of pointers to the PerlIO structs (malloc'ed)
 */
#define PERLIO_TABLE_SIZE 64

static void
PerlIO_init_table(pTHX)
{
    if (PL_perlio)
        return;
    Newxz(PL_perlio, PERLIO_TABLE_SIZE, PerlIOl);
}



PerlIO *
PerlIO_allocate(pTHX)
{
    /*
     * Find a free slot in the table, allocating new tables as necessary
     */
    PerlIOl **last;
    PerlIOl *f;
    last = &PL_perlio;
    while ((f = *last)) {
        int i;
        last = &f->next;
        for (i = 1; i < PERLIO_TABLE_SIZE; i++) {
            if (!((++f)->next)) {
                goto good_exit;
            }
        }
    }
    Newxz(f,PERLIO_TABLE_SIZE,PerlIOl);
    if (!f) {
        return NULL;
    }
    *last = f++;

    good_exit:
    f->flags = 0; /* lockcnt */
    f->tab = NULL;
    f->head = f;
    return &f->next;
}

#undef PerlIO_fdupopen
PerlIO *
PerlIO_fdupopen(pTHX_ PerlIO *f, CLONE_PARAMS *param, int flags)
{
    if (PerlIOValid(f)) {
        const PerlIO_funcs * const tab = PerlIOBase(f)->tab;
        DEBUG_i( PerlIO_debug("fdupopen f=%p param=%p\n",(void*)f,(void*)param) );
        if (tab && tab->Dup)
             return (*tab->Dup)(aTHX_ PerlIO_allocate(aTHX), f, param, flags);
        else {
             return PerlIOBase_dup(aTHX_ PerlIO_allocate(aTHX), f, param, flags);
        }
    }
    else
         SETERRNO(EBADF, SS_IVCHAN);

    return NULL;
}

void
PerlIO_cleantable(pTHX_ PerlIOl **tablep)
{
    PerlIOl * const table = *tablep;
    if (table) {
        int i;
        PerlIO_cleantable(aTHX_ &table[0].next);
        for (i = PERLIO_TABLE_SIZE - 1; i > 0; i--) {
            PerlIOl * const f = table + i;
            if (f->next) {
                PerlIO_close(&(f->next));
            }
        }
        Safefree(table);
        *tablep = NULL;
    }
}


PerlIO_list_t *
PerlIO_list_alloc(pTHX)
{
    PerlIO_list_t *list;
    PERL_UNUSED_CONTEXT;
    Newxz(list, 1, PerlIO_list_t);
    list->refcnt = 1;
    return list;
}

void
PerlIO_list_free(pTHX_ PerlIO_list_t *list)
{
    if (list) {
        if (--list->refcnt == 0) {
            if (list->array) {
                IV i;
                for (i = 0; i < list->cur; i++)
                    SvREFCNT_dec(list->array[i].arg);
                Safefree(list->array);
            }
            Safefree(list);
        }
    }
}

void
PerlIO_list_push(pTHX_ PerlIO_list_t *list, PerlIO_funcs *funcs, SV *arg)
{
    PerlIO_pair_t *p;
    PERL_UNUSED_CONTEXT;

    if (list->cur >= list->len) {
        const IV new_len = list->len + 8;
        if (list->array)
            Renew(list->array, new_len, PerlIO_pair_t);
        else
            Newx(list->array, new_len, PerlIO_pair_t);
        list->len = new_len;
    }
    p = &(list->array[list->cur++]);
    p->funcs = funcs;
    if ((p->arg = arg)) {
        SvREFCNT_inc_simple_void_NN(arg);
    }
}

PerlIO_list_t *
PerlIO_clone_list(pTHX_ PerlIO_list_t *proto, CLONE_PARAMS *param)
{
    PerlIO_list_t *list = NULL;
    if (proto) {
        int i;
        list = PerlIO_list_alloc(aTHX);
        for (i=0; i < proto->cur; i++) {
            SV *arg = proto->array[i].arg;
#ifdef USE_ITHREADS
            if (arg && param)
                arg = sv_dup(arg, param);
#else
            PERL_UNUSED_ARG(param);
#endif
            PerlIO_list_push(aTHX_ list, proto->array[i].funcs, arg);
        }
    }
    return list;
}

void
PerlIO_clone(pTHX_ PerlInterpreter *proto, CLONE_PARAMS *param)
{
#ifdef USE_ITHREADS
    PerlIOl **table = &proto->Iperlio;
    PerlIOl *f;
    PL_perlio = NULL;
    PL_known_layers = PerlIO_clone_list(aTHX_ proto->Iknown_layers, param);
    PL_def_layerlist = PerlIO_clone_list(aTHX_ proto->Idef_layerlist, param);
    PerlIO_init_table(aTHX);
    DEBUG_i( PerlIO_debug("Clone %p from %p\n",(void*)aTHX,(void*)proto) );
    while ((f = *table)) {
            int i;
            table = &f->next;
            f++;
            for (i = 1; i < PERLIO_TABLE_SIZE; i++) {
                if (f->next) {
                    (void) fp_dup(&(f->next), 0, param);
                }
                f++;
            }
        }
#else
    PERL_UNUSED_CONTEXT;
    PERL_UNUSED_ARG(proto);
    PERL_UNUSED_ARG(param);
#endif
}

void
PerlIO_destruct(pTHX)
{
    PerlIOl **table = &PL_perlio;
    PerlIOl *f;
#ifdef USE_ITHREADS
    DEBUG_i( PerlIO_debug("Destruct %p\n",(void*)aTHX) );
#endif
    while ((f = *table)) {
        int i;
        table = &f->next;
        f++;
        for (i = 1; i < PERLIO_TABLE_SIZE; i++) {
            PerlIO *x = &(f->next);
            const PerlIOl *l;
            while ((l = *x)) {
                if (l->tab && l->tab->kind & PERLIO_K_DESTRUCT) {
                    DEBUG_i( PerlIO_debug("Destruct popping %s\n", l->tab->name) );
                    PerlIO_flush(x);
                    PerlIO_pop(aTHX_ x);
                }
                else {
                    x = PerlIONext(x);
                }
            }
            f++;
        }
    }
}

void
PerlIO_pop(pTHX_ PerlIO *f)
{
    const PerlIOl *l = *f;
    VERIFY_HEAD(f);
    if (l) {
        DEBUG_i( PerlIO_debug("PerlIO_pop f=%p %s\n", (void*)f,
                              l->tab ? l->tab->name : "(Null)") );
        if (l->tab && l->tab->Popped) {
            /*
             * If popped returns non-zero do not free its layer structure
             * it has either done so itself, or it is shared and still in
             * use
             */
            if ((*l->tab->Popped) (aTHX_ f) != 0)
                return;
        }
        if (PerlIO_lockcnt(f)) {
            /* we're in use; defer freeing the structure */
            PerlIOBase(f)->flags = PERLIO_F_CLEARED;
            PerlIOBase(f)->tab = NULL;
        }
        else {
            *f = l->next;
            Safefree(l);
        }

    }
}

/* Return as an array the stack of layers on a filehandle.  Note that
 * the stack is returned top-first in the array, and there are three
 * times as many array elements as there are layers in the stack: the
 * first element of a layer triplet is the name, the second one is the
 * arguments, and the third one is the flags. */

AV *
PerlIO_get_layers(pTHX_ PerlIO *f)
{
    AV * const av = newAV();

    if (PerlIOValid(f)) {
        PerlIOl *l = PerlIOBase(f);

        while (l) {
            /* There is some collusion in the implementation of
               XS_PerlIO_get_layers - it knows that name and flags are
               generated as fresh SVs here, and takes advantage of that to
               "copy" them by taking a reference. If it changes here, it needs
               to change there too.  */
            SV * const name = l->tab && l->tab->name ?
            newSVpv(l->tab->name, 0) : &PL_sv_undef;
            SV * const arg = l->tab && l->tab->Getarg ?
            (*l->tab->Getarg)(aTHX_ &l, 0, 0) : &PL_sv_undef;
            av_push_simple(av, name);
            av_push_simple(av, arg);
            av_push_simple(av, newSViv((IV)l->flags));
            l = l->next;
        }
    }

    return av;
}

/*--------------------------------------------------------------------------------------*/
/*
 * XS Interface for perl code
 */

PerlIO_funcs *
PerlIO_find_layer(pTHX_ const char *name, STRLEN len, int load)
{

    IV i;
    if ((SSize_t) len <= 0)
        len = strlen(name);
    for (i = 0; i < PL_known_layers->cur; i++) {
        PerlIO_funcs * const f = PL_known_layers->array[i].funcs;
        const STRLEN this_len = strlen(f->name);
        if (this_len == len && memEQ(f->name, name, len)) {
            DEBUG_i( PerlIO_debug("%.*s => %p\n", (int) len, name, (void*)f) );
            return f;
        }
    }
    if (load && PL_subname && PL_def_layerlist
        && PL_def_layerlist->cur >= 2) {
        if (PL_in_load_module) {
            Perl_croak(aTHX_ "Recursive call to Perl_load_module in PerlIO_find_layer");
            return NULL;
        } else {
            SV * const pkgsv = newSVpvs("PerlIO");
            SV * const layer = newSVpvn(name, len);
            CV * const cv    = get_cvs("PerlIO::Layer::NoWarnings", 0);
            ENTER;
            SAVEBOOL(PL_in_load_module);
            if (cv) {
                SAVEGENERICSV(PL_warnhook);
                PL_warnhook = MUTABLE_SV((SvREFCNT_inc_simple_NN(cv)));
            }
            PL_in_load_module = TRUE;
            /*
             * The two SVs are magically freed by load_module
             */
            Perl_load_module(aTHX_ 0, pkgsv, NULL, layer, NULL);
            LEAVE;
            return PerlIO_find_layer(aTHX_ name, len, 0);
        }
    }
    DEBUG_i( PerlIO_debug("Cannot find %.*s\n", (int) len, name) );
    return NULL;
}

#ifdef USE_ATTRIBUTES_FOR_PERLIO

static int
perlio_mg_set(pTHX_ SV *sv, MAGIC *mg)
{
    if (SvROK(sv)) {
        IO * const io = GvIOn(MUTABLE_GV(SvRV(sv)));
        PerlIO * const ifp = IoIFP(io);
        PerlIO * const ofp = IoOFP(io);
        Perl_warn(aTHX_ "set %" SVf " %p %p %p",
                  SVfARG(sv), (void*)io, (void*)ifp, (void*)ofp);
    }
    return 0;
}

static int
perlio_mg_get(pTHX_ SV *sv, MAGIC *mg)
{
    if (SvROK(sv)) {
        IO * const io = GvIOn(MUTABLE_GV(SvRV(sv)));
        PerlIO * const ifp = IoIFP(io);
        PerlIO * const ofp = IoOFP(io);
        Perl_warn(aTHX_ "get %" SVf " %p %p %p",
                  SVfARG(sv), (void*)io, (void*)ifp, (void*)ofp);
    }
    return 0;
}

static int
perlio_mg_clear(pTHX_ SV *sv, MAGIC *mg)
{
    Perl_warn(aTHX_ "clear %" SVf, SVfARG(sv));
    return 0;
}

static int
perlio_mg_free(pTHX_ SV *sv, MAGIC *mg)
{
    Perl_warn(aTHX_ "free %" SVf, SVfARG(sv));
    return 0;
}

MGVTBL perlio_vtab = {
    perlio_mg_get,
    perlio_mg_set,
    NULL,                       /* len */
    perlio_mg_clear,
    perlio_mg_free
};

XS(XS_io_MODIFY_SCALAR_ATTRIBUTES); /* prototype to pass -Wmissing-prototypes */
XS(XS_io_MODIFY_SCALAR_ATTRIBUTES)
{
    dXSARGS;
    SV * const sv = SvRV(ST(1));
    AV * const av = newAV();
    MAGIC *mg;
    int count = 0;
    int i;
    sv_magic(sv, MUTABLE_SV(av), PERL_MAGIC_ext, NULL, 0);
    SvRMAGICAL_off(sv);
    mg = mg_find(sv, PERL_MAGIC_ext);
    mg->mg_virtual = &perlio_vtab;
    mg_magical(sv);
    Perl_warn(aTHX_ "attrib %" SVf, SVfARG(sv));
    for (i = 2; i < items; i++) {
        STRLEN len;
        const char * const name = SvPV_const(ST(i), len);
        SV * const layer = PerlIO_find_layer(aTHX_ name, len, 1);
        if (layer) {
            av_push_simple(av, SvREFCNT_inc_simple_NN(layer));
        }
        else {
            ST(count) = ST(i);
            count++;
        }
    }
    SvREFCNT_dec(av);
    XSRETURN(count);
}

#endif                          /* USE_ATTRIBUTES_FOR_PERLIO */

SV *
PerlIO_tab_sv(pTHX_ PerlIO_funcs *tab)
{
    HV * const stash = gv_stashpvs("PerlIO::Layer", GV_ADD);
    SV * const sv = sv_bless(newRV_noinc(newSViv(PTR2IV(tab))), stash);
    return sv;
}

XS(XS_PerlIO__Layer__NoWarnings); /* prototype to pass -Wmissing-prototypes */
XS(XS_PerlIO__Layer__NoWarnings)
{
    /* This is used as a %SIG{__WARN__} handler to suppress warnings
       during loading of layers.
     */
    dXSARGS;
    PERL_UNUSED_VAR(items);
    DEBUG_i(
        if (items)
            PerlIO_debug("warning:%s\n",SvPV_nolen_const(ST(0))) );
    XSRETURN(0);
}

XS(XS_PerlIO__Layer__find); /* prototype to pass -Wmissing-prototypes */
XS(XS_PerlIO__Layer__find)
{
    dXSARGS;
    if (items < 2)
        Perl_croak(aTHX_ "Usage class->find(name[,load])");
    else {
        STRLEN len;
        const char * const name = SvPV_const(ST(1), len);
        const bool load = (items > 2) ? SvTRUE_NN(ST(2)) : 0;
        PerlIO_funcs * const layer = PerlIO_find_layer(aTHX_ name, len, load);
        ST(0) =
            (layer) ? sv_2mortal(PerlIO_tab_sv(aTHX_ layer)) :
            &PL_sv_undef;
        XSRETURN(1);
    }
}

void
PerlIO_define_layer(pTHX_ PerlIO_funcs *tab)
{
    if (!PL_known_layers)
        PL_known_layers = PerlIO_list_alloc(aTHX);
    PerlIO_list_push(aTHX_ PL_known_layers, tab, NULL);
    DEBUG_i( PerlIO_debug("define %s %p\n", tab->name, (void*)tab) );
}

int
PerlIO_parse_layers(pTHX_ PerlIO_list_t *av, const char *names)
{
    if (names) {
        const char *s = names;
        while (*s) {
            while (isSPACE(*s) || *s == ':')
                s++;
            if (*s) {
                STRLEN llen = 0;
                const char *e = s;
                const char *as = NULL;
                STRLEN alen = 0;
                if (!isIDFIRST(*s)) {
                    /*
                     * Message is consistent with how attribute lists are
                     * passed. Even though this means "foo : : bar" is
                     * seen as an invalid separator character.
                     */
                    const char q = ((*s == '\'') ? '"' : '\'');
                    Perl_ck_warner(aTHX_ packWARN(WARN_LAYER),
                                   "Invalid separator character %c%c%c in PerlIO layer specification %s",
                                   q, *s, q, s);
                    SETERRNO(EINVAL, LIB_INVARG);
                    return -1;
                }
                do {
                    e++;
                } while (isWORDCHAR(*e));
                llen = e - s;
                if (*e == '(') {
                    int nesting = 1;
                    as = ++e;
                    while (nesting) {
                        switch (*e++) {
                        case ')':
                            if (--nesting == 0)
                                alen = (e - 1) - as;
                            break;
                        case '(':
                            ++nesting;
                            break;
                        case '\\':
                            /*
                             * It's a nul terminated string, not allowed
                             * to \ the terminating null. Anything other
                             * character is passed over.
                             */
                            if (*e++) {
                                break;
                            }
                            /* Fall through */
                        case '\0':
                            e--;
                            Perl_ck_warner(aTHX_ packWARN(WARN_LAYER),
                                           "Argument list not closed for PerlIO layer \"%.*s\"",
                                           (int) (e - s), s);
                            return -1;
                        default:
                            /*
                             * boring.
                             */
                            break;
                        }
                    }
                }
                if (e > s) {
                    PerlIO_funcs * const layer =
                        PerlIO_find_layer(aTHX_ s, llen, 1);
                    if (layer) {
                        SV *arg = NULL;
                        if (as)
                            arg = newSVpvn(as, alen);
                        PerlIO_list_push(aTHX_ av, layer,
                                         (arg) ? arg : &PL_sv_undef);
                        SvREFCNT_dec(arg);
                    }
                    else {
                        Perl_ck_warner(aTHX_ packWARN(WARN_LAYER), "Unknown PerlIO layer \"%.*s\"",
                                       (int) llen, s);
                        return -1;
                    }
                }
                s = e;
            }
        }
    }
    return 0;
}

void
PerlIO_default_buffer(pTHX_ PerlIO_list_t *av)
{
    PERLIO_FUNCS_DECL(*tab) = &PerlIO_perlio;
#ifdef PERLIO_USING_CRLF
    tab = &PerlIO_crlf;
#else
    if (PerlIO_stdio.Set_ptrcnt)
        tab = &PerlIO_stdio;
#endif
    DEBUG_i( PerlIO_debug("Pushing %s\n", tab->name) );
    PerlIO_list_push(aTHX_ av, (PerlIO_funcs *)tab, &PL_sv_undef);
}

SV *
PerlIO_arg_fetch(PerlIO_list_t *av, IV n)
{
    return av->array[n].arg;
}

PerlIO_funcs *
PerlIO_layer_fetch(pTHX_ PerlIO_list_t *av, IV n, PerlIO_funcs *def)
{
    if (n >= 0 && n < av->cur) {
        DEBUG_i( PerlIO_debug("Layer %" IVdf " is %s\n", n,
                              av->array[n].funcs->name) );
        return av->array[n].funcs;
    }
    if (!def)
        Perl_croak(aTHX_ "panic: PerlIO layer array corrupt");
    return def;
}

IV
PerlIOPop_pushed(pTHX_ PerlIO *f, const char *mode, SV *arg, PerlIO_funcs *tab)
{
    PERL_UNUSED_ARG(mode);
    PERL_UNUSED_ARG(arg);
    PERL_UNUSED_ARG(tab);
    if (PerlIOValid(f)) {
        PerlIO_flush(f);
        PerlIO_pop(aTHX_ f);
        return 0;
    }
    return -1;
}

PERLIO_FUNCS_DECL(PerlIO_remove) = {
    sizeof(PerlIO_funcs),
    "pop",
    0,
    PERLIO_K_DUMMY | PERLIO_K_UTF8,
    PerlIOPop_pushed,
    NULL,
    PerlIOBase_open,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,                       /* flush */
    NULL,                       /* fill */
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,                       /* get_base */
    NULL,                       /* get_bufsiz */
    NULL,                       /* get_ptr */
    NULL,                       /* get_cnt */
    NULL,                       /* set_ptrcnt */
};

PerlIO_list_t *
PerlIO_default_layers(pTHX)
{
    if (!PL_def_layerlist) {
        const char * const s = TAINTING_get ? NULL : PerlEnv_getenv("PERLIO");
        PERLIO_FUNCS_DECL(*osLayer) = &PerlIO_unix;
        PL_def_layerlist = PerlIO_list_alloc(aTHX);
        PerlIO_define_layer(aTHX_ PERLIO_FUNCS_CAST(&PerlIO_unix));
        PerlIO_define_layer(aTHX_ PERLIO_FUNCS_CAST(&PerlIO_raw));
        PerlIO_define_layer(aTHX_ PERLIO_FUNCS_CAST(&PerlIO_perlio));
        PerlIO_define_layer(aTHX_ PERLIO_FUNCS_CAST(&PerlIO_stdio));
        PerlIO_define_layer(aTHX_ PERLIO_FUNCS_CAST(&PerlIO_crlf));
        PerlIO_define_layer(aTHX_ PERLIO_FUNCS_CAST(&PerlIO_utf8));
        PerlIO_define_layer(aTHX_ PERLIO_FUNCS_CAST(&PerlIO_remove));
        PerlIO_define_layer(aTHX_ PERLIO_FUNCS_CAST(&PerlIO_byte));
        PerlIO_list_push(aTHX_ PL_def_layerlist, (PerlIO_funcs *)osLayer,
                         &PL_sv_undef);
        if (s) {
            PerlIO_parse_layers(aTHX_ PL_def_layerlist, s);
        }
        else {
            PerlIO_default_buffer(aTHX_ PL_def_layerlist);
        }
    }
    if (PL_def_layerlist->cur < 2) {
        PerlIO_default_buffer(aTHX_ PL_def_layerlist);
    }
    return PL_def_layerlist;
}

void
Perl_boot_core_PerlIO(pTHX)
{
#ifdef USE_ATTRIBUTES_FOR_PERLIO
    newXS("io::MODIFY_SCALAR_ATTRIBUTES", XS_io_MODIFY_SCALAR_ATTRIBUTES,
          __FILE__);
#endif
    newXS("PerlIO::Layer::find", XS_PerlIO__Layer__find, __FILE__);
    newXS("PerlIO::Layer::NoWarnings", XS_PerlIO__Layer__NoWarnings, __FILE__);
}

PerlIO_funcs *
PerlIO_default_layer(pTHX_ I32 n)
{
    PerlIO_list_t * const av = PerlIO_default_layers(aTHX);
    if (n < 0)
        n += av->cur;
    return PerlIO_layer_fetch(aTHX_ av, n, PERLIO_FUNCS_CAST(&PerlIO_stdio));
}

#define PerlIO_default_top() PerlIO_default_layer(aTHX_ -1)
#define PerlIO_default_btm() PerlIO_default_layer(aTHX_ 0)

void
PerlIO_stdstreams(pTHX)
{
    if (!PL_perlio) {
        PerlIO_init_table(aTHX);
        PerlIO_fdopen(0, "Ir" PERLIO_STDTEXT);
        PerlIO_fdopen(1, "Iw" PERLIO_STDTEXT);
        PerlIO_fdopen(2, "Iw" PERLIO_STDTEXT);
    }
}

PerlIO *
PerlIO_push(pTHX_ PerlIO *f, PERLIO_FUNCS_DECL(*tab), const char *mode, SV *arg)
{
    VERIFY_HEAD(f);
    if (tab->fsize != sizeof(PerlIO_funcs)) {
        Perl_croak( aTHX_
            "%s (%" UVuf ") does not match %s (%" UVuf ")",
            "PerlIO layer function table size", (UV)tab->fsize,
            "size expected by this perl", (UV)sizeof(PerlIO_funcs) );
    }
    if (tab->size) {
        PerlIOl *l;
        if (tab->size < sizeof(PerlIOl)) {
            Perl_croak( aTHX_
                "%s (%" UVuf ") smaller than %s (%" UVuf ")",
                "PerlIO layer instance size", (UV)tab->size,
                "size expected by this perl", (UV)sizeof(PerlIOl) );
        }
        /* Real layer with a data area */
        if (f) {
            char *temp;
            Newxz(temp, tab->size, char);
            l = (PerlIOl*)temp;
            if (l) {
                l->next = *f;
                l->tab = (PerlIO_funcs*) tab;
                l->head = ((PerlIOl*)f)->head;
                *f = l;
                DEBUG_i( PerlIO_debug("PerlIO_push f=%p %s %s %p\n",
                                      (void*)f, tab->name,
                                      (mode) ? mode : "(Null)", (void*)arg) );
                if (*l->tab->Pushed &&
                    (*l->tab->Pushed)
                      (aTHX_ f, mode, arg, (PerlIO_funcs*) tab) != 0) {
                    PerlIO_pop(aTHX_ f);
                    return NULL;
                }
            }
            else
                return NULL;
        }
    }
    else if (f) {
        /* Pseudo-layer where push does its own stack adjust */
        DEBUG_i( PerlIO_debug("PerlIO_push f=%p %s %s %p\n", (void*)f, tab->name,
                              (mode) ? mode : "(Null)", (void*)arg) );
        if (tab->Pushed &&
            (*tab->Pushed) (aTHX_ f, mode, arg, (PerlIO_funcs*) tab) != 0) {
             return NULL;
        }
    }
    return f;
}

PerlIO *
PerlIOBase_open(pTHX_ PerlIO_funcs *self, PerlIO_list_t *layers,
               IV n, const char *mode, int fd, int imode, int perm,
               PerlIO *old, int narg, SV **args)
{
    PerlIO_funcs * const tab = PerlIO_layer_fetch(aTHX_ layers, n - 1, PerlIO_default_layer(aTHX_ 0));
    if (tab && tab->Open) {
        PerlIO* ret = (*tab->Open)(aTHX_ tab, layers, n - 1, mode, fd, imode, perm, old, narg, args);
        if (ret && PerlIO_push(aTHX_ ret, self, mode, PerlIOArg) == NULL) {
            PerlIO_close(ret);
            return NULL;
        }
        return ret;
    }
    SETERRNO(EINVAL, LIB_INVARG);
    return NULL;
}

IV
PerlIOBase_binmode(pTHX_ PerlIO *f)
{
   if (PerlIOValid(f)) {
        /* Is layer suitable for raw stream ? */
        if (PerlIOBase(f)->tab && PerlIOBase(f)->tab->kind & PERLIO_K_RAW) {
            /* Yes - turn off UTF-8-ness, to undo UTF-8 locale effects */
            PerlIOBase(f)->flags &= ~PERLIO_F_UTF8;
        }
        else {
            /* Not suitable - pop it */
            PerlIO_pop(aTHX_ f);
        }
        return 0;
   }
   return -1;
}

IV
PerlIORaw_pushed(pTHX_ PerlIO *f, const char *mode, SV *arg, PerlIO_funcs *tab)
{
    PERL_UNUSED_ARG(mode);
    PERL_UNUSED_ARG(arg);
    PERL_UNUSED_ARG(tab);

    if (PerlIOValid(f)) {
        PerlIO *t;
        const PerlIOl *l;
        PerlIO_flush(f);
        /*
         * Strip all layers that are not suitable for a raw stream
         */
        t = f;
        while (t && (l = *t)) {
            if (l->tab && l->tab->Binmode) {
                /* Has a handler - normal case */
                if ((*l->tab->Binmode)(aTHX_ t) == 0) {
                    if (*t == l) {
                        /* Layer still there - move down a layer */
                        t = PerlIONext(t);
                    }
                }
                else {
                    return -1;
                }
            }
            else {
                /* No handler - pop it */
                PerlIO_pop(aTHX_ t);
            }
        }
        if (PerlIOValid(f)) {
            DEBUG_i( PerlIO_debug(":raw f=%p :%s\n", (void*)f,
                         PerlIOBase(f)->tab ? PerlIOBase(f)->tab->name : "(Null)") );
            return 0;
        }
    }
    return -1;
}

int
PerlIO_apply_layera(pTHX_ PerlIO *f, const char *mode,
                    PerlIO_list_t *layers, IV n, IV max)
{
    int code = 0;
    while (n < max) {
        PerlIO_funcs * const tab = PerlIO_layer_fetch(aTHX_ layers, n, NULL);
        if (tab) {
            if (!PerlIO_push(aTHX_ f, tab, mode, PerlIOArg)) {
                code = -1;
                break;
            }
        }
        n++;
    }
    return code;
}

int
PerlIO_apply_layers(pTHX_ PerlIO *f, const char *mode, const char *names)
{
    int code = 0;
    ENTER;
    save_scalar(PL_errgv);
    if (f && names) {
        PerlIO_list_t * const layers = PerlIO_list_alloc(aTHX);
        code = PerlIO_parse_layers(aTHX_ layers, names);
        if (code == 0) {
            code = PerlIO_apply_layera(aTHX_ f, mode, layers, 0, layers->cur);
        }
        PerlIO_list_free(aTHX_ layers);
    }
    LEAVE;
    return code;
}


/*--------------------------------------------------------------------------------------*/
/*
 * Given the abstraction above the public API functions
 */

int
PerlIO_binmode(pTHX_ PerlIO *f, int iotype, int mode, const char *names)
{
    PERL_UNUSED_ARG(iotype);
    PERL_UNUSED_ARG(mode);

    DEBUG_i(
        PerlIO_debug("PerlIO_binmode f=%p %s %c %x %s\n", (void*)f,
                     (PerlIOBase(f) && PerlIOBase(f)->tab) ?
                     PerlIOBase(f)->tab->name : "(Null)",
                     iotype, mode, (names) ? names : "(Null)") );

    if (names) {
        /* Do not flush etc. if (e.g.) switching encodings.
           if a pushed layer knows it needs to flush lower layers
           (for example :unix which is never going to call them)
           it can do the flush when it is pushed.
         */
        return cBOOL(PerlIO_apply_layers(aTHX_ f, NULL, names) == 0);
    }
    else {
        /* Fake 5.6 legacy of using this call to turn ON O_TEXT */
#ifdef PERLIO_USING_CRLF
        /* Legacy binmode only has meaning if O_TEXT has a value distinct from
           O_BINARY so we can look for it in mode.
         */
        if (!(mode & O_BINARY)) {
            /* Text mode */
            /* FIXME?: Looking down the layer stack seems wrong,
               but is a way of reaching past (say) an encoding layer
               to flip CRLF-ness of the layer(s) below
             */
            while (*f) {
                /* Perhaps we should turn on bottom-most aware layer
                   e.g. Ilya's idea that UNIX TTY could serve
                 */
                if (PerlIOBase(f)->tab &&
                    PerlIOBase(f)->tab->kind & PERLIO_K_CANCRLF)
                {
                    if (!(PerlIOBase(f)->flags & PERLIO_F_CRLF)) {
                        /* Not in text mode - flush any pending stuff and flip it */
                        PerlIO_flush(f);
                        PerlIOBase(f)->flags |= PERLIO_F_CRLF;
                    }
                    /* Only need to turn it on in one layer so we are done */
                    return TRUE;
                }
                f = PerlIONext(f);
            }
            /* Not finding a CRLF aware layer presumably means we are binary
               which is not what was requested - so we failed
               We _could_ push :crlf layer but so could caller
             */
            return FALSE;
        }
#endif
        /* Legacy binmode is now _defined_ as being equivalent to pushing :raw
           So code that used to be here is now in PerlIORaw_pushed().
         */
        return cBOOL(PerlIO_push(aTHX_ f, PERLIO_FUNCS_CAST(&PerlIO_raw), NULL, NULL));
    }
}

int
PerlIO__close(pTHX_ PerlIO *f)
{
    if (PerlIOValid(f)) {
        PerlIO_funcs * const tab = PerlIOBase(f)->tab;
        if (tab && tab->Close)
            return (*tab->Close)(aTHX_ f);
        else
            return PerlIOBase_close(aTHX_ f);
    }
    else {
        SETERRNO(EBADF, SS_IVCHAN);
        return -1;
    }
}

int
Perl_PerlIO_close(pTHX_ PerlIO *f)
{
    const int code = PerlIO__close(aTHX_ f);
    while (PerlIOValid(f)) {
        PerlIO_pop(aTHX_ f);
        if (PerlIO_lockcnt(f))
            /* we're in use; the 'pop' deferred freeing the structure */
            f = PerlIONext(f);
    }
    return code;
}

int
Perl_PerlIO_fileno(pTHX_ PerlIO *f)
{
    Perl_PerlIO_or_Base(f, Fileno, fileno, -1, (aTHX_ f));
}


static PerlIO_funcs *
PerlIO_layer_from_ref(pTHX_ SV *sv)
{
    /*
     * For any scalar type load the handler which is bundled with perl
     */
    if (SvTYPE(sv) < SVt_PVAV && (!isGV_with_GP(sv) || SvFAKE(sv))) {
        PerlIO_funcs *f = PerlIO_find_layer(aTHX_ STR_WITH_LEN("scalar"), 1);
        /* This isn't supposed to happen, since PerlIO::scalar is core,
         * but could happen anyway in smaller installs or with PAR */
        if (!f)
            /* diag_listed_as: Unknown PerlIO layer "%s" */
            Perl_ck_warner(aTHX_ packWARN(WARN_LAYER), "Unknown PerlIO layer \"scalar\"");
        return f;
    }

    /*
     * For other types allow if layer is known but don't try and load it
     */
    switch (SvTYPE(sv)) {
    case SVt_PVAV:
        return PerlIO_find_layer(aTHX_ STR_WITH_LEN("Array"), 0);
    case SVt_PVHV:
        return PerlIO_find_layer(aTHX_ STR_WITH_LEN("Hash"), 0);
    case SVt_PVCV:
        return PerlIO_find_layer(aTHX_ STR_WITH_LEN("Code"), 0);
    case SVt_PVGV:
        return PerlIO_find_layer(aTHX_ STR_WITH_LEN("Glob"), 0);
    default:
        return NULL;
    }
}

PerlIO_list_t *
PerlIO_resolve_layers(pTHX_ const char *layers,
                      const char *mode, int narg, SV **args)
{
    PerlIO_list_t *def = PerlIO_default_layers(aTHX);
    int incdef = 1;
    if (!PL_perlio)
        PerlIO_stdstreams(aTHX);
    if (narg) {
        SV * const arg = *args;
        /*
         * If it is a reference but not an object see if we have a handler
         * for it
         */
        if (SvROK(arg) && !SvOBJECT(SvRV(arg))) {
            PerlIO_funcs * const handler = PerlIO_layer_from_ref(aTHX_ SvRV(arg));
            if (handler) {
                def = PerlIO_list_alloc(aTHX);
                PerlIO_list_push(aTHX_ def, handler, &PL_sv_undef);
                incdef = 0;
            }
            /*
             * Don't fail if handler cannot be found :via(...) etc. may do
             * something sensible else we will just stringify and open
             * resulting string.
             */
        }
    }
    if (!layers || !*layers)
        layers = Perl_PerlIO_context_layers(aTHX_ mode);
    if (layers && *layers) {
        PerlIO_list_t *av;
        if (incdef) {
            av = PerlIO_clone_list(aTHX_ def, NULL);
        }
        else {
            av = def;
        }
        if (PerlIO_parse_layers(aTHX_ av, layers) == 0) {
             return av;
        }
        else {
            PerlIO_list_free(aTHX_ av);
            return NULL;
        }
    }
    else {
        if (incdef)
            def->refcnt++;
        return def;
    }
}

PerlIO *
PerlIO_openn(pTHX_ const char *layers, const char *mode, int fd,
             int imode, int perm, PerlIO *f, int narg, SV **args)
{
    if (!f && narg == 1 && *args == &PL_sv_undef) {
        imode = PerlIOUnix_oflags(mode);

        if (imode != -1 && (f = PerlIO_tmpfile_flags(imode))) {
            if (!layers || !*layers)
                layers = Perl_PerlIO_context_layers(aTHX_ mode);
            if (layers && *layers)
                PerlIO_apply_layers(aTHX_ f, mode, layers);
        }
    }
    else {
        PerlIO_list_t *layera;
        IV n;
        PerlIO_funcs *tab = NULL;
        if (PerlIOValid(f)) {
            /*
             * This is "reopen" - it is not tested as perl does not use it
             * yet
             */
            PerlIOl *l = *f;
            layera = PerlIO_list_alloc(aTHX);
            while (l) {
                SV *arg = NULL;
                if (l->tab && l->tab->Getarg)
                    arg = (*l->tab->Getarg) (aTHX_ &l, NULL, 0);
                PerlIO_list_push(aTHX_ layera, l->tab,
                                 (arg) ? arg : &PL_sv_undef);
                SvREFCNT_dec(arg);
                l = *PerlIONext(&l);
            }
        }
        else {
            layera = PerlIO_resolve_layers(aTHX_ layers, mode, narg, args);
            if (!layera) {
                return NULL;
            }
        }
        /*
         * Start at "top" of layer stack
         */
        n = layera->cur - 1;
        while (n >= 0) {
            PerlIO_funcs * const t = PerlIO_layer_fetch(aTHX_ layera, n, NULL);
            if (t && t->Open) {
                tab = t;
                break;
            }
            n--;
        }
        if (tab) {
            /*
             * Found that layer 'n' can do opens - call it
             */
            if (narg > 1 && !(tab->kind & PERLIO_K_MULTIARG)) {
                Perl_croak(aTHX_ "More than one argument to open(,':%s')",tab->name);
            }
            DEBUG_i( PerlIO_debug("openn(%s,'%s','%s',%d,%x,%o,%p,%d,%p)\n",
                                  tab->name, layers ? layers : "(Null)", mode, fd,
                                  imode, perm, (void*)f, narg, (void*)args) );
            if (tab->Open)
                 f = (*tab->Open) (aTHX_ tab, layera, n, mode, fd, imode, perm,
                                   f, narg, args);
            else {
                 SETERRNO(EINVAL, LIB_INVARG);
                 f = NULL;
            }
            if (f) {
                if (n + 1 < layera->cur) {
                    /*
                     * More layers above the one that we used to open -
                     * apply them now
                     */
                    if (PerlIO_apply_layera(aTHX_ f, mode, layera, n + 1, layera->cur) != 0) {
                        /* If pushing layers fails close the file */
                        PerlIO_close(f);
                        f = NULL;
                    }
                }
            }
        }
        PerlIO_list_free(aTHX_ layera);
    }
    return f;
}


SSize_t
Perl_PerlIO_read(pTHX_ PerlIO *f, void *vbuf, Size_t count)
{
     PERL_ARGS_ASSERT_PERLIO_READ;

     Perl_PerlIO_or_Base(f, Read, read, -1, (aTHX_ f, vbuf, count));
}

SSize_t
Perl_PerlIO_unread(pTHX_ PerlIO *f, const void *vbuf, Size_t count)
{
     PERL_ARGS_ASSERT_PERLIO_UNREAD;

     Perl_PerlIO_or_Base(f, Unread, unread, -1, (aTHX_ f, vbuf, count));
}

SSize_t
Perl_PerlIO_write(pTHX_ PerlIO *f, const void *vbuf, Size_t count)
{
     PERL_ARGS_ASSERT_PERLIO_WRITE;

     Perl_PerlIO_or_fail(f, Write, -1, (aTHX_ f, vbuf, count));
}

int
Perl_PerlIO_seek(pTHX_ PerlIO *f, Off_t offset, int whence)
{
     Perl_PerlIO_or_fail(f, Seek, -1, (aTHX_ f, offset, whence));
}

Off_t
Perl_PerlIO_tell(pTHX_ PerlIO *f)
{
     Perl_PerlIO_or_fail(f, Tell, -1, (aTHX_ f));
}

int
Perl_PerlIO_flush(pTHX_ PerlIO *f)
{
    if (f) {
        if (*f) {
            const PerlIO_funcs *tab = PerlIOBase(f)->tab;

            if (tab && tab->Flush)
                return (*tab->Flush) (aTHX_ f);
            else
                 return 0; /* If no Flush defined, silently succeed. */
        }
        else {
            DEBUG_i( PerlIO_debug("Cannot flush f=%p\n", (void*)f) );
            SETERRNO(EBADF, SS_IVCHAN);
            return -1;
        }
    }
    else {
        /*
         * Is it good API design to do flush-all on NULL, a potentially
         * erroneous input? Maybe some magical value (PerlIO*
         * PERLIO_FLUSH_ALL = (PerlIO*)-1;)? Yes, stdio does similar
         * things on fflush(NULL), but should we be bound by their design
         * decisions? --jhi
         */
        PerlIOl **table = &PL_perlio;
        PerlIOl *ff;
        int code = 0;
        while ((ff = *table)) {
            int i;
            table = &ff->next;
            ff++;
            for (i = 1; i < PERLIO_TABLE_SIZE; i++) {
                if (ff->next && PerlIO_flush(&(ff->next)) != 0)
                    code = -1;
                ff++;
            }
        }
        return code;
    }
}

void
PerlIOBase_flush_linebuf(pTHX)
{
    PerlIOl **table = &PL_perlio;
    PerlIOl *f;
    while ((f = *table)) {
        int i;
        table = &f->next;
        f++;
        for (i = 1; i < PERLIO_TABLE_SIZE; i++) {
            if (f->next
                && (PerlIOBase(&(f->next))->
                    flags & (PERLIO_F_LINEBUF | PERLIO_F_CANWRITE))
                == (PERLIO_F_LINEBUF | PERLIO_F_CANWRITE))
                PerlIO_flush(&(f->next));
            f++;
        }
    }
}

int
Perl_PerlIO_fill(pTHX_ PerlIO *f)
{
     Perl_PerlIO_or_fail(f, Fill, -1, (aTHX_ f));
}

int
PerlIO_isutf8(PerlIO *f)
{
     if (PerlIOValid(f))
          return (PerlIOBase(f)->flags & PERLIO_F_UTF8) != 0;
     else
          SETERRNO(EBADF, SS_IVCHAN);

     return -1;
}

int
Perl_PerlIO_eof(pTHX_ PerlIO *f)
{
     Perl_PerlIO_or_Base(f, Eof, eof, -1, (aTHX_ f));
}

int
Perl_PerlIO_error(pTHX_ PerlIO *f)
{
     Perl_PerlIO_or_Base(f, Error, error, -1, (aTHX_ f));
}

void
Perl_PerlIO_clearerr(pTHX_ PerlIO *f)
{
     Perl_PerlIO_or_Base_void(f, Clearerr, clearerr, (aTHX_ f));
}

void
Perl_PerlIO_setlinebuf(pTHX_ PerlIO *f)
{
     Perl_PerlIO_or_Base_void(f, Setlinebuf, setlinebuf, (aTHX_ f));
}

int
PerlIO_has_base(PerlIO *f)
{
     if (PerlIOValid(f)) {
          const PerlIO_funcs * const tab = PerlIOBase(f)->tab;

          if (tab)
               return (tab->Get_base != NULL);
     }

     return 0;
}

int
PerlIO_fast_gets(PerlIO *f)
{
    if (PerlIOValid(f)) {
         if (PerlIOBase(f)->flags & PERLIO_F_FASTGETS) {
             const PerlIO_funcs * const tab = PerlIOBase(f)->tab;

             if (tab)
                  return (tab->Set_ptrcnt != NULL);
         }
    }

    return 0;
}

int
PerlIO_has_cntptr(PerlIO *f)
{
    if (PerlIOValid(f)) {
        const PerlIO_funcs * const tab = PerlIOBase(f)->tab;

        if (tab)
             return (tab->Get_ptr != NULL && tab->Get_cnt != NULL);
    }

    return 0;
}

int
PerlIO_canset_cnt(PerlIO *f)
{
    if (PerlIOValid(f)) {
          const PerlIO_funcs * const tab = PerlIOBase(f)->tab;

          if (tab)
               return (tab->Set_ptrcnt != NULL);
    }

    return 0;
}

STDCHAR *
Perl_PerlIO_get_base(pTHX_ PerlIO *f)
{
     Perl_PerlIO_or_fail(f, Get_base, NULL, (aTHX_ f));
}

SSize_t
Perl_PerlIO_get_bufsiz(pTHX_ PerlIO *f)
{
    /* Note that Get_bufsiz returns a Size_t */
     Perl_PerlIO_or_fail(f, Get_bufsiz, -1, (aTHX_ f));
}

STDCHAR *
Perl_PerlIO_get_ptr(pTHX_ PerlIO *f)
{
     Perl_PerlIO_or_fail(f, Get_ptr, NULL, (aTHX_ f));
}

SSize_t
Perl_PerlIO_get_cnt(pTHX_ PerlIO *f)
{
     Perl_PerlIO_or_fail(f, Get_cnt, -1, (aTHX_ f));
}

void
Perl_PerlIO_set_cnt(pTHX_ PerlIO *f, SSize_t cnt)
{
     Perl_PerlIO_or_fail_void(f, Set_ptrcnt, (aTHX_ f, NULL, cnt));
}

void
Perl_PerlIO_set_ptrcnt(pTHX_ PerlIO *f, STDCHAR * ptr, SSize_t cnt)
{
     Perl_PerlIO_or_fail_void(f, Set_ptrcnt, (aTHX_ f, ptr, cnt));
}


/*--------------------------------------------------------------------------------------*/
/*
 * utf8 and raw dummy layers
 */

IV
PerlIOUtf8_pushed(pTHX_ PerlIO *f, const char *mode, SV *arg, PerlIO_funcs *tab)
{
    PERL_UNUSED_CONTEXT;
    PERL_UNUSED_ARG(mode);
    PERL_UNUSED_ARG(arg);
    if (PerlIOValid(f)) {
        if (tab && tab->kind & PERLIO_K_UTF8)
            PerlIOBase(f)->flags |= PERLIO_F_UTF8;
        else
            PerlIOBase(f)->flags &= ~PERLIO_F_UTF8;
        return 0;
    }
    return -1;
}

PERLIO_FUNCS_DECL(PerlIO_utf8) = {
    sizeof(PerlIO_funcs),
    "utf8",
    0,
    PERLIO_K_DUMMY | PERLIO_K_UTF8 | PERLIO_K_MULTIARG,
    PerlIOUtf8_pushed,
    NULL,
    PerlIOBase_open,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,                       /* flush */
    NULL,                       /* fill */
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,                       /* get_base */
    NULL,                       /* get_bufsiz */
    NULL,                       /* get_ptr */
    NULL,                       /* get_cnt */
    NULL,                       /* set_ptrcnt */
};

PERLIO_FUNCS_DECL(PerlIO_byte) = {
    sizeof(PerlIO_funcs),
    "bytes",
    0,
    PERLIO_K_DUMMY | PERLIO_K_MULTIARG,
    PerlIOUtf8_pushed,
    NULL,
    PerlIOBase_open,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,                       /* flush */
    NULL,                       /* fill */
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,                       /* get_base */
    NULL,                       /* get_bufsiz */
    NULL,                       /* get_ptr */
    NULL,                       /* get_cnt */
    NULL,                       /* set_ptrcnt */
};

PERLIO_FUNCS_DECL(PerlIO_raw) = {
    sizeof(PerlIO_funcs),
    "raw",
    0,
    PERLIO_K_DUMMY,
    PerlIORaw_pushed,
    PerlIOBase_popped,
    PerlIOBase_open,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,                       /* flush */
    NULL,                       /* fill */
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,                       /* get_base */
    NULL,                       /* get_bufsiz */
    NULL,                       /* get_ptr */
    NULL,                       /* get_cnt */
    NULL,                       /* set_ptrcnt */
};
/*--------------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------------*/
/*
 * "Methods" of the "base class"
 */

IV
PerlIOBase_fileno(pTHX_ PerlIO *f)
{
    return PerlIOValid(f) ? PerlIO_fileno(PerlIONext(f)) : -1;
}

char *
PerlIO_modestr(PerlIO * f, char *buf)
{
    char *s = buf;
    if (PerlIOValid(f)) {
        const IV flags = PerlIOBase(f)->flags;
        if (flags & PERLIO_F_APPEND) {
            *s++ = 'a';
            if (flags & PERLIO_F_CANREAD) {
                *s++ = '+';
            }
        }
        else if (flags & PERLIO_F_CANREAD) {
            *s++ = 'r';
            if (flags & PERLIO_F_CANWRITE)
                *s++ = '+';
        }
        else if (flags & PERLIO_F_CANWRITE) {
            *s++ = 'w';
            if (flags & PERLIO_F_CANREAD) {
                *s++ = '+';
            }
        }
#ifdef PERLIO_USING_CRLF
        if (!(flags & PERLIO_F_CRLF))
            *s++ = 'b';
#endif
    }
    *s = '\0';
    return buf;
}


IV
PerlIOBase_pushed(pTHX_ PerlIO *f, const char *mode, SV *arg, PerlIO_funcs *tab)
{
    PerlIOl * const l = PerlIOBase(f);
    PERL_UNUSED_CONTEXT;
    PERL_UNUSED_ARG(arg);

    l->flags &= ~(PERLIO_F_CANREAD | PERLIO_F_CANWRITE |
                  PERLIO_F_TRUNCATE | PERLIO_F_APPEND);
    if (tab && tab->Set_ptrcnt != NULL)
        l->flags |= PERLIO_F_FASTGETS;
    if (mode) {
        if (*mode == IoTYPE_NUMERIC || *mode == IoTYPE_IMPLICIT)
            mode++;
        switch (*mode++) {
        case 'r':
            l->flags |= PERLIO_F_CANREAD;
            break;
        case 'a':
            l->flags |= PERLIO_F_APPEND | PERLIO_F_CANWRITE;
            break;
        case 'w':
            l->flags |= PERLIO_F_TRUNCATE | PERLIO_F_CANWRITE;
            break;
        default:
            SETERRNO(EINVAL, LIB_INVARG);
            return -1;
        }
#ifdef __MVS__  /* XXX Perhaps should be be OEMVS instead of __MVS__ */
        {
        /* The mode variable contains one positional parameter followed by
         * optional keyword parameters.  The positional parameters must be
         * passed as lowercase characters.  The keyword parameters can be
         * passed in mixed case. They must be separated by commas. Only one
         * instance of a keyword can be specified.  */
        int comma = 0;
        while (*mode) {
            switch (*mode++) {
            case '+':
                if(!comma)
                  l->flags |= PERLIO_F_CANREAD | PERLIO_F_CANWRITE;
                break;
            case 'b':
                if(!comma)
                  l->flags &= ~PERLIO_F_CRLF;
                break;
            case 't':
                if(!comma)
                  l->flags |= PERLIO_F_CRLF;
                break;
            case ',':
                comma = 1;
                break;
            default:
                break;
            }
        }
        }
#else
        while (*mode) {
            switch (*mode++) {
            case '+':
                l->flags |= PERLIO_F_CANREAD | PERLIO_F_CANWRITE;
                break;
            case 'b':
                l->flags &= ~PERLIO_F_CRLF;
                break;
            case 't':
                l->flags |= PERLIO_F_CRLF;
                break;
            default:
                SETERRNO(EINVAL, LIB_INVARG);
                return -1;
            }
        }
#endif
    }
    else {
        if (l->next) {
            l->flags |= l->next->flags &
                (PERLIO_F_CANREAD | PERLIO_F_CANWRITE | PERLIO_F_TRUNCATE |
                 PERLIO_F_APPEND);
        }
    }
#if 0
    DEBUG_i(
    PerlIO_debug("PerlIOBase_pushed f=%p %s %s fl=%08" UVxf " (%s)\n",
                 (void*)f, PerlIOBase(f)->tab->name, (omode) ? omode : "(Null)",
                 l->flags, PerlIO_modestr(f, temp));
    );
#endif
    return 0;
}

IV
PerlIOBase_popped(pTHX_ PerlIO *f)
{
    PERL_UNUSED_CONTEXT;
    PERL_UNUSED_ARG(f);
    return 0;
}

SSize_t
PerlIOBase_unread(pTHX_ PerlIO *f, const void *vbuf, Size_t count)
{
    /*
     * Save the position as current head considers it
     */
    const Off_t old = PerlIO_tell(f);
    PerlIO_push(aTHX_ f, PERLIO_FUNCS_CAST(&PerlIO_pending), "r", NULL);
    PerlIOSelf(f, PerlIOBuf)->posn = old;
    return PerlIOBuf_unread(aTHX_ f, vbuf, count);
}

SSize_t
PerlIOBase_read(pTHX_ PerlIO *f, void *vbuf, Size_t count)
{
    STDCHAR *buf = (STDCHAR *) vbuf;
    if (f) {
        if (!(PerlIOBase(f)->flags & PERLIO_F_CANREAD)) {
            PerlIOBase(f)->flags |= PERLIO_F_ERROR;
            SETERRNO(EBADF, SS_IVCHAN);
            PerlIO_save_errno(f);
            return 0;
        }
        while (count > 0) {
         get_cnt:
          {
            SSize_t avail = PerlIO_get_cnt(f);
            SSize_t take = 0;
            if (avail > 0)
                take = (((SSize_t) count >= 0) && ((SSize_t)count < avail)) ? (SSize_t)count : avail;
            if (take > 0) {
                STDCHAR *ptr = PerlIO_get_ptr(f);
                Copy(ptr, buf, take, STDCHAR);
                PerlIO_set_ptrcnt(f, ptr + take, (avail -= take));
                count -= take;
                buf += take;
                if (avail == 0)		/* set_ptrcnt could have reset avail */
                    goto get_cnt;
            }
            if (count > 0 && avail <= 0) {
                if (PerlIO_fill(f) != 0)
                    break;
            }
          }
        }
        return (buf - (STDCHAR *) vbuf);
    }
    return 0;
}

IV
PerlIOBase_noop_ok(pTHX_ PerlIO *f)
{
    PERL_UNUSED_CONTEXT;
    PERL_UNUSED_ARG(f);
    return 0;
}

IV
PerlIOBase_noop_fail(pTHX_ PerlIO *f)
{
    PERL_UNUSED_CONTEXT;
    PERL_UNUSED_ARG(f);
    return -1;
}

IV
PerlIOBase_close(pTHX_ PerlIO *f)
{
    IV code = -1;
    if (PerlIOValid(f)) {
        PerlIO *n = PerlIONext(f);
        code = PerlIO_flush(f);
        PerlIOBase(f)->flags &=
           ~(PERLIO_F_CANREAD | PERLIO_F_CANWRITE | PERLIO_F_OPEN);
        while (PerlIOValid(n)) {
            const PerlIO_funcs * const tab = PerlIOBase(n)->tab;
            if (tab && tab->Close) {
                if ((*tab->Close)(aTHX_ n) != 0)
                    code = -1;
                break;
            }
            else {
                PerlIOBase(n)->flags &=
                    ~(PERLIO_F_CANREAD | PERLIO_F_CANWRITE | PERLIO_F_OPEN);
            }
            n = PerlIONext(n);
        }
    }
    else {
        SETERRNO(EBADF, SS_IVCHAN);
    }
    return code;
}

IV
PerlIOBase_eof(pTHX_ PerlIO *f)
{
    PERL_UNUSED_CONTEXT;
    if (PerlIOValid(f)) {
        return (PerlIOBase(f)->flags & PERLIO_F_EOF) != 0;
    }
    return 1;
}

IV
PerlIOBase_error(pTHX_ PerlIO *f)
{
    PERL_UNUSED_CONTEXT;
    if (PerlIOValid(f)) {
        return (PerlIOBase(f)->flags & PERLIO_F_ERROR) != 0;
    }
    return 1;
}

void
PerlIOBase_clearerr(pTHX_ PerlIO *f)
{
    if (PerlIOValid(f)) {
        PerlIO * const n = PerlIONext(f);
        PerlIOBase(f)->flags &= ~(PERLIO_F_ERROR | PERLIO_F_EOF);
        if (PerlIOValid(n))
            PerlIO_clearerr(n);
    }
}

void
PerlIOBase_setlinebuf(pTHX_ PerlIO *f)
{
    PERL_UNUSED_CONTEXT;
    if (PerlIOValid(f)) {
        PerlIOBase(f)->flags |= PERLIO_F_LINEBUF;
    }
}

SV *
PerlIO_sv_dup(pTHX_ SV *arg, CLONE_PARAMS *param)
{
    if (!arg)
        return NULL;
#ifdef USE_ITHREADS
    if (param) {
        arg = sv_dup(arg, param);
        SvREFCNT_inc_simple_void_NN(arg);
        return arg;
    }
    else {
        return newSVsv(arg);
    }
#else
    PERL_UNUSED_ARG(param);
    return newSVsv(arg);
#endif
}

PerlIO *
PerlIOBase_dup(pTHX_ PerlIO *f, PerlIO *o, CLONE_PARAMS *param, int flags)
{
    PerlIO * const nexto = PerlIONext(o);
    if (PerlIOValid(nexto)) {
        const PerlIO_funcs * const tab = PerlIOBase(nexto)->tab;
        if (tab && tab->Dup)
            f = (*tab->Dup)(aTHX_ f, nexto, param, flags);
        else
            f = PerlIOBase_dup(aTHX_ f, nexto, param, flags);
    }
    if (f) {
        PerlIO_funcs * const self = PerlIOBase(o)->tab;
        SV *arg = NULL;
        char buf[8];
        assert(self);
        DEBUG_i(PerlIO_debug("PerlIOBase_dup %s f=%p o=%p param=%p\n",
                             self->name,
                             (void*)f, (void*)o, (void*)param) );
        if (self->Getarg)
          arg = (*self->Getarg)(aTHX_ o, param, flags);
        f = PerlIO_push(aTHX_ f, self, PerlIO_modestr(o,buf), arg);
        if (f && PerlIOBase(o)->flags & PERLIO_F_UTF8)
            PerlIOBase(f)->flags |= PERLIO_F_UTF8;
        SvREFCNT_dec(arg);
    }
    return f;
}

/* PL_perlio_fd_refcnt[] is in intrpvar.h */

/* Must be called with PL_perlio_mutex locked. */
static void
S_more_refcounted_fds(pTHX_ const int new_fd)
  PERL_TSA_REQUIRES(PL_perlio_mutex)
{
    const int old_max = PL_perlio_fd_refcnt_size;
    const int new_max = 16 + (new_fd & ~15);
    int *new_array;

#ifndef PERL_IMPLICIT_SYS
    PERL_UNUSED_CONTEXT;
#endif

    DEBUG_i( PerlIO_debug("More fds - old=%d, need %d, new=%d\n",
                          old_max, new_fd, new_max) );

    if (new_fd < old_max) {
        return;
    }

    assert (new_max > new_fd);

    /* Use plain realloc() since we need this memory to be really
     * global and visible to all the interpreters and/or threads. */
    new_array = (int*) realloc(PL_perlio_fd_refcnt, new_max * sizeof(int));

    if (!new_array) {
        MUTEX_UNLOCK(&PL_perlio_mutex);
        croak_no_mem();
    }

    PL_perlio_fd_refcnt_size = new_max;
    PL_perlio_fd_refcnt = new_array;

    DEBUG_i( PerlIO_debug("Zeroing %p, %d\n",
                          (void*)(new_array + old_max),
                          new_max - old_max) );

    Zero(new_array + old_max, new_max - old_max, int);
}


void
PerlIO_init(pTHX)
{
    /* MUTEX_INIT(&PL_perlio_mutex) is done in PERL_SYS_INIT3(). */
    PERL_UNUSED_CONTEXT;
}

void
PerlIOUnix_refcnt_inc(int fd)
{
    dTHX;
    if (fd >= 0) {

        MUTEX_LOCK(&PL_perlio_mutex);
        if (fd >= PL_perlio_fd_refcnt_size)
            S_more_refcounted_fds(aTHX_ fd);

        PL_perlio_fd_refcnt[fd]++;
        if (PL_perlio_fd_refcnt[fd] <= 0) {
            /* diag_listed_as: refcnt_inc: fd %d%s */
            Perl_croak(aTHX_ "refcnt_inc: fd %d: %d <= 0\n",
                       fd, PL_perlio_fd_refcnt[fd]);
        }
        DEBUG_i( PerlIO_debug("refcnt_inc: fd %d refcnt=%d\n",
                              fd, PL_perlio_fd_refcnt[fd]) );

        MUTEX_UNLOCK(&PL_perlio_mutex);
    } else {
        /* diag_listed_as: refcnt_inc: fd %d%s */
        Perl_croak(aTHX_ "refcnt_inc: fd %d < 0\n", fd);
    }
}

int
PerlIOUnix_refcnt_dec(int fd)
{
    int cnt = 0;
    if (fd >= 0) {
#ifdef DEBUGGING
        dTHX;
#endif
        MUTEX_LOCK(&PL_perlio_mutex);
        if (fd >= PL_perlio_fd_refcnt_size) {
            /* diag_listed_as: refcnt_dec: fd %d%s */
            Perl_croak_nocontext("refcnt_dec: fd %d >= refcnt_size %d\n",
                       fd, PL_perlio_fd_refcnt_size);
        }
        if (PL_perlio_fd_refcnt[fd] <= 0) {
            /* diag_listed_as: refcnt_dec: fd %d%s */
            Perl_croak_nocontext("refcnt_dec: fd %d: %d <= 0\n",
                       fd, PL_perlio_fd_refcnt[fd]);
        }
        cnt = --PL_perlio_fd_refcnt[fd];
        DEBUG_i( PerlIO_debug("refcnt_dec: fd %d refcnt=%d\n", fd, cnt) );
        MUTEX_UNLOCK(&PL_perlio_mutex);
    } else {
        /* diag_listed_as: refcnt_dec: fd %d%s */
        Perl_croak_nocontext("refcnt_dec: fd %d < 0\n", fd);
    }
    return cnt;
}

int
PerlIOUnix_refcnt(int fd)
{
    dTHX;
    int cnt = 0;
    if (fd >= 0) {
        MUTEX_LOCK(&PL_perlio_mutex);
        if (fd >= PL_perlio_fd_refcnt_size) {
            /* diag_listed_as: refcnt: fd %d%s */
            Perl_croak(aTHX_ "refcnt: fd %d >= refcnt_size %d\n",
                       fd, PL_perlio_fd_refcnt_size);
        }
        if (PL_perlio_fd_refcnt[fd] <= 0) {
            /* diag_listed_as: refcnt: fd %d%s */
            Perl_croak(aTHX_ "refcnt: fd %d: %d <= 0\n",
                       fd, PL_perlio_fd_refcnt[fd]);
        }
        cnt = PL_perlio_fd_refcnt[fd];
        MUTEX_UNLOCK(&PL_perlio_mutex);
    } else {
        /* diag_listed_as: refcnt: fd %d%s */
        Perl_croak(aTHX_ "refcnt: fd %d < 0\n", fd);
    }
    return cnt;
}

void
PerlIO_cleanup(pTHX)
{
    int i;
#ifdef USE_ITHREADS
    DEBUG_i( PerlIO_debug("Cleanup layers for %p\n",(void*)aTHX) );
#else
    DEBUG_i( PerlIO_debug("Cleanup layers\n") );
#endif

    /* Raise STDIN..STDERR refcount so we don't close them */
    for (i=0; i < 3; i++)
        PerlIOUnix_refcnt_inc(i);
    PerlIO_cleantable(aTHX_ &PL_perlio);
    /* Restore STDIN..STDERR refcount */
    for (i=0; i < 3; i++)
        PerlIOUnix_refcnt_dec(i);

    if (PL_known_layers) {
        PerlIO_list_free(aTHX_ PL_known_layers);
        PL_known_layers = NULL;
    }
    if (PL_def_layerlist) {
        PerlIO_list_free(aTHX_ PL_def_layerlist);
        PL_def_layerlist = NULL;
    }
}

void PerlIO_teardown(void) /* Call only from PERL_SYS_TERM(). */
{
#if 0
/* XXX we can't rely on an interpreter being present at this late stage,
   XXX so we can't use a function like PerlLIO_write that relies on one
   being present (at least in win32) :-(.
   Disable for now.
*/
#  ifdef DEBUGGING
    {
        /* By now all filehandles should have been closed, so any
         * stray (non-STD-)filehandles indicate *possible* (PerlIO)
         * errors. */
#define PERLIO_TEARDOWN_MESSAGE_BUF_SIZE 64
#define PERLIO_TEARDOWN_MESSAGE_FD 2
        char buf[PERLIO_TEARDOWN_MESSAGE_BUF_SIZE];
        int i;
        for (i = 3; i < PL_perlio_fd_refcnt_size; i++) {
            if (PL_perlio_fd_refcnt[i]) {
                const STRLEN len =
                    my_snprintf(buf, sizeof(buf),
                                "PerlIO_teardown: fd %d refcnt=%d\n",
                                i, PL_perlio_fd_refcnt[i]);
                PerlLIO_write(PERLIO_TEARDOWN_MESSAGE_FD, buf, len);
            }
        }
    }
#  endif
#endif
    /* Not bothering with PL_perlio_mutex since by now
     * all the interpreters are gone. */
    if (PL_perlio_fd_refcnt_size /* Assuming initial size of zero. */
        && PL_perlio_fd_refcnt) {
        free(PL_perlio_fd_refcnt); /* To match realloc() in S_more_refcounted_fds(). */
        PL_perlio_fd_refcnt = NULL;
        PL_perlio_fd_refcnt_size = 0;
    }
}

/*--------------------------------------------------------------------------------------*/
/*
 * Bottom-most level for UNIX-like case
 */

typedef struct {
    struct _PerlIO base;        /* The generic part */
    int fd;                     /* UNIX like file descriptor */
    int oflags;                 /* open/fcntl flags */
} PerlIOUnix;

static void
S_lockcnt_dec(pTHX_ const void* f)
{
#ifndef PERL_IMPLICIT_SYS
    PERL_UNUSED_CONTEXT;
#endif
    PerlIO_lockcnt((PerlIO*)f)--;
}


/* call the signal handler, and if that handler happens to clear
 * this handle, free what we can and return true */

static bool
S_perlio_async_run(pTHX_ PerlIO* f) {
    ENTER;
    SAVEDESTRUCTOR_X(S_lockcnt_dec, (void*)f);
    PerlIO_lockcnt(f)++;
    PERL_ASYNC_CHECK();
    if ( !(PerlIOBase(f)->flags & PERLIO_F_CLEARED) ) {
        LEAVE;
        return 0;
    }
    /* we've just run some perl-level code that could have done
     * anything, including closing the file or clearing this layer.
     * If so, free any lower layers that have already been
     * cleared, then return an error. */
    while (PerlIOValid(f) &&
            (PerlIOBase(f)->flags & PERLIO_F_CLEARED))
    {
        const PerlIOl *l = *f;
        *f = l->next;
        Safefree(l);
    }
    LEAVE;
    return 1;
}

int
PerlIOUnix_oflags(const char *mode)
{
    int oflags = -1;
    if (*mode == IoTYPE_IMPLICIT || *mode == IoTYPE_NUMERIC)
        mode++;
    switch (*mode) {
    case 'r':
        oflags = O_RDONLY;
        if (*++mode == '+') {
            oflags = O_RDWR;
            mode++;
        }
        break;

    case 'w':
        oflags = O_CREAT | O_TRUNC;
        if (*++mode == '+') {
            oflags |= O_RDWR;
            mode++;
        }
        else
            oflags |= O_WRONLY;
        break;

    case 'a':
        oflags = O_CREAT | O_APPEND;
        if (*++mode == '+') {
            oflags |= O_RDWR;
            mode++;
        }
        else
            oflags |= O_WRONLY;
        break;
    }

    /* XXX TODO: PerlIO_open() test that exercises 'rb' and 'rt'. */

    /* Unless O_BINARY is different from O_TEXT, first bit-or:ing one
     * of them in, and then bit-and-masking the other them away, won't
     * have much of an effect. */
    switch (*mode) {
    case 'b':
#if O_TEXT != O_BINARY
        oflags |= O_BINARY;
        oflags &= ~O_TEXT;
#endif
        mode++;
        break;
    case 't':
#if O_TEXT != O_BINARY
        oflags |= O_TEXT;
        oflags &= ~O_BINARY;
#endif
        mode++;
        break;
    default:
#if O_BINARY != 0
        /* bit-or:ing with zero O_BINARY would be useless. */
        /*
         * If neither "t" nor "b" was specified, open the file
         * in O_BINARY mode.
         *
         * Note that if something else than the zero byte was seen
         * here (e.g. bogus mode "rx"), just few lines later we will
         * set the errno and invalidate the flags.
         */
        oflags |= O_BINARY;
#endif
        break;
    }
    if (*mode || oflags == -1) {
        SETERRNO(EINVAL, LIB_INVARG);
        oflags = -1;
    }
    return oflags;
}

IV
PerlIOUnix_fileno(pTHX_ PerlIO *f)
{
    PERL_UNUSED_CONTEXT;
    return PerlIOSelf(f, PerlIOUnix)->fd;
}

static void
PerlIOUnix_setfd(pTHX_ PerlIO *f, int fd, int imode)
{
    PerlIOUnix * const s = PerlIOSelf(f, PerlIOUnix);
#if defined(WIN32)
    Stat_t st;
    if (PerlLIO_fstat(fd, &st) == 0) {
        if (!S_ISREG(st.st_mode)) {
            DEBUG_i( PerlIO_debug("%d is not regular file\n",fd) );
            PerlIOBase(f)->flags |= PERLIO_F_NOTREG;
        }
        else {
            DEBUG_i( PerlIO_debug("%d _is_ a regular file\n",fd) );
        }
    }
#endif
    s->fd = fd;
    s->oflags = imode;
    PerlIOUnix_refcnt_inc(fd);
    PERL_UNUSED_CONTEXT;
}

IV
PerlIOUnix_pushed(pTHX_ PerlIO *f, const char *mode, SV *arg, PerlIO_funcs *tab)
{
    IV code = PerlIOBase_pushed(aTHX_ f, mode, arg, tab);
    if (*PerlIONext(f)) {
        /* We never call down so do any pending stuff now */
        PerlIO_flush(PerlIONext(f));
        /*
         * XXX could (or should) we retrieve the oflags from the open file
         * handle rather than believing the "mode" we are passed in? XXX
         * Should the value on NULL mode be 0 or -1?
         */
        PerlIOUnix_setfd(aTHX_ f, PerlIO_fileno(PerlIONext(f)),
                         mode ? PerlIOUnix_oflags(mode) : -1);
    }
    PerlIOBase(f)->flags |= PERLIO_F_OPEN;

    return code;
}

IV
PerlIOUnix_seek(pTHX_ PerlIO *f, Off_t offset, int whence)
{
    const int fd = PerlIOSelf(f, PerlIOUnix)->fd;
    Off_t new_loc;
    PERL_UNUSED_CONTEXT;
    if (PerlIOBase(f)->flags & PERLIO_F_NOTREG) {
#ifdef  ESPIPE
        SETERRNO(ESPIPE, LIB_INVARG);
#else
        SETERRNO(EINVAL, LIB_INVARG);
#endif
        return -1;
    }
    new_loc = PerlLIO_lseek(fd, offset, whence);
    if (new_loc == (Off_t) - 1)
        return -1;
    PerlIOBase(f)->flags &= ~PERLIO_F_EOF;
    return  0;
}

PerlIO *
PerlIOUnix_open(pTHX_ PerlIO_funcs *self, PerlIO_list_t *layers,
                IV n, const char *mode, int fd, int imode,
                int perm, PerlIO *f, int narg, SV **args)
{
    bool known_cloexec = 0;
    if (PerlIOValid(f)) {
        if (PerlIOBase(f)->tab && PerlIOBase(f)->flags & PERLIO_F_OPEN)
            (*PerlIOBase(f)->tab->Close)(aTHX_ f);
    }
    if (narg > 0) {
        if (*mode == IoTYPE_NUMERIC)
            mode++;
        else {
            imode = PerlIOUnix_oflags(mode);
#ifdef VMS
            perm = 0777; /* preserve RMS defaults, ACL inheritance, etc. */
#else
            perm = 0666;
#endif
        }
        if (imode != -1) {
            STRLEN len;
            const char *path = SvPV_const(*args, len);
            if (!IS_SAFE_PATHNAME(path, len, "open"))
                return NULL;
            fd = PerlLIO_open3_cloexec(path, imode, perm);
            known_cloexec = 1;
        }
    }
    if (fd >= 0) {
        if (known_cloexec)
            setfd_inhexec_for_sysfd(fd);
        else
            setfd_cloexec_or_inhexec_by_sysfdness(fd);
        if (*mode == IoTYPE_IMPLICIT)
            mode++;
        if (!f) {
            f = PerlIO_allocate(aTHX);
        }
        if (!PerlIOValid(f)) {
            if (!(f = PerlIO_push(aTHX_ f, self, mode, PerlIOArg))) {
                PerlLIO_close(fd);
                return NULL;
            }
        }
        PerlIOUnix_setfd(aTHX_ f, fd, imode);
        PerlIOBase(f)->flags |= PERLIO_F_OPEN;
        if (*mode == IoTYPE_APPEND)
            PerlIOUnix_seek(aTHX_ f, 0, SEEK_END);
        return f;
    }
    else {
        if (f) {
            NOOP;
            /*
             * FIXME: pop layers ???
             */
        }
        return NULL;
    }
}

PerlIO *
PerlIOUnix_dup(pTHX_ PerlIO *f, PerlIO *o, CLONE_PARAMS *param, int flags)
{
    const PerlIOUnix * const os = PerlIOSelf(o, PerlIOUnix);
    int fd = os->fd;
    if (flags & PERLIO_DUP_FD) {
        fd = PerlLIO_dup_cloexec(fd);
        if (fd >= 0)
            setfd_inhexec_for_sysfd(fd);
    }
    if (fd >= 0) {
        f = PerlIOBase_dup(aTHX_ f, o, param, flags);
        if (f) {
            /* If all went well overwrite fd in dup'ed lay with the dup()'ed fd */
            PerlIOUnix_setfd(aTHX_ f, fd, os->oflags);
            return f;
        }
        PerlLIO_close(fd);
    }
    return NULL;
}


SSize_t
PerlIOUnix_read(pTHX_ PerlIO *f, void *vbuf, Size_t count)
{
    int fd;
    if (PerlIO_lockcnt(f)) /* in use: abort ungracefully */
        return -1;
    fd = PerlIOSelf(f, PerlIOUnix)->fd;
    if (!(PerlIOBase(f)->flags & PERLIO_F_CANREAD) ||
         PerlIOBase(f)->flags & (PERLIO_F_EOF|PERLIO_F_ERROR)) {
        return 0;
    }
    while (1) {
        const SSize_t len = PerlLIO_read(fd, vbuf, count);
        if (len >= 0 || errno != EINTR) {
            if (len < 0) {
                if (errno != EAGAIN) {
                    PerlIOBase(f)->flags |= PERLIO_F_ERROR;
                    PerlIO_save_errno(f);
                }
            }
            else if (len == 0 && count != 0) {
                PerlIOBase(f)->flags |= PERLIO_F_EOF;
                SETERRNO(0,0);
            }
            return len;
        }
        /* EINTR */
        if (PL_sig_pending && S_perlio_async_run(aTHX_ f))
            return -1;
    }
    NOT_REACHED; /*NOTREACHED*/
}

SSize_t
PerlIOUnix_write(pTHX_ PerlIO *f, const void *vbuf, Size_t count)
{
    int fd;
    if (PerlIO_lockcnt(f)) /* in use: abort ungracefully */
        return -1;
    fd = PerlIOSelf(f, PerlIOUnix)->fd;
    while (1) {
        const SSize_t len = PerlLIO_write(fd, vbuf, count);
        if (len >= 0 || errno != EINTR) {
            if (len < 0) {
                if (errno != EAGAIN) {
                    PerlIOBase(f)->flags |= PERLIO_F_ERROR;
                    PerlIO_save_errno(f);
                }
            }
            return len;
        }
        /* EINTR */
        if (PL_sig_pending && S_perlio_async_run(aTHX_ f))
            return -1;
    }
    NOT_REACHED; /*NOTREACHED*/
}

Off_t
PerlIOUnix_tell(pTHX_ PerlIO *f)
{
    PERL_UNUSED_CONTEXT;

    return PerlLIO_lseek(PerlIOSelf(f, PerlIOUnix)->fd, 0, SEEK_CUR);
}


IV
PerlIOUnix_close(pTHX_ PerlIO *f)
{
    const int fd = PerlIOSelf(f, PerlIOUnix)->fd;
    int code = 0;
    if (PerlIOBase(f)->flags & PERLIO_F_OPEN) {
        code = PerlIOBase_close(aTHX_ f);
        if (PerlIOUnix_refcnt_dec(fd) > 0) {
            PerlIOBase(f)->flags &= ~PERLIO_F_OPEN;
            return 0;
        }
    }
    else {
        SETERRNO(EBADF,SS_IVCHAN);
        return -1;
    }
    while (PerlLIO_close(fd) != 0) {
        if (errno != EINTR) {
            code = -1;
            break;
        }
        /* EINTR */
        if (PL_sig_pending && S_perlio_async_run(aTHX_ f))
            return -1;
    }
    if (code == 0) {
        PerlIOBase(f)->flags &= ~PERLIO_F_OPEN;
    }
    return code;
}

PERLIO_FUNCS_DECL(PerlIO_unix) = {
    sizeof(PerlIO_funcs),
    "unix",
    sizeof(PerlIOUnix),
    PERLIO_K_RAW,
    PerlIOUnix_pushed,
    PerlIOBase_popped,
    PerlIOUnix_open,
    PerlIOBase_binmode,         /* binmode */
    NULL,
    PerlIOUnix_fileno,
    PerlIOUnix_dup,
    PerlIOUnix_read,
    PerlIOBase_unread,
    PerlIOUnix_write,
    PerlIOUnix_seek,
    PerlIOUnix_tell,
    PerlIOUnix_close,
    PerlIOBase_noop_ok,         /* flush */
    PerlIOBase_noop_fail,       /* fill */
    PerlIOBase_eof,
    PerlIOBase_error,
    PerlIOBase_clearerr,
    PerlIOBase_setlinebuf,
    NULL,                       /* get_base */
    NULL,                       /* get_bufsiz */
    NULL,                       /* get_ptr */
    NULL,                       /* get_cnt */
    NULL,                       /* set_ptrcnt */
};

/*--------------------------------------------------------------------------------------*/
/*
 * stdio as a layer
 */

#if defined(VMS) && !defined(STDIO_BUFFER_WRITABLE)
/* perl5.8 - This ensures the last minute VMS ungetc fix is not
   broken by the last second glibc 2.3 fix
 */
#  define STDIO_BUFFER_WRITABLE
#endif


typedef struct {
    struct _PerlIO base;
    FILE *stdio;                /* The stream */
} PerlIOStdio;

IV
PerlIOStdio_fileno(pTHX_ PerlIO *f)
{
    PERL_UNUSED_CONTEXT;

    if (PerlIOValid(f)) {
        FILE * const s = PerlIOSelf(f, PerlIOStdio)->stdio;
        if (s)
            return PerlSIO_fileno(s);
    }
    errno = EBADF;
    return -1;
}

char *
PerlIOStdio_mode(const char *mode, char *tmode)
{
    char * const ret = tmode;
    if (mode) {
        while (*mode) {
            *tmode++ = *mode++;
        }
    }
#if defined(PERLIO_USING_CRLF) || defined(__CYGWIN__)
    *tmode++ = 'b';
#endif
    *tmode = '\0';
    return ret;
}

IV
PerlIOStdio_pushed(pTHX_ PerlIO *f, const char *mode, SV *arg, PerlIO_funcs *tab)
{
    PerlIO *n;
    if (PerlIOValid(f) && PerlIOValid(n = PerlIONext(f))) {
        PerlIO_funcs * const toptab = PerlIOBase(n)->tab;
        if (toptab == tab) {
            /* Top is already stdio - pop self (duplicate) and use original */
            PerlIO_pop(aTHX_ f);
            return 0;
        } else {
            const int fd = PerlIO_fileno(n);
            char tmode[8];
            FILE *stdio;
            if (fd >= 0 && (stdio  = PerlSIO_fdopen(fd,
                            mode = PerlIOStdio_mode(mode, tmode)))) {
                PerlIOSelf(f, PerlIOStdio)->stdio = stdio;
                /* We never call down so do any pending stuff now */
                PerlIO_flush(PerlIONext(f));
                return PerlIOBase_pushed(aTHX_ f, mode, arg, tab);
            }
            else {
                return -1;
            }
        }
    }
    return PerlIOBase_pushed(aTHX_ f, mode, arg, tab);
}


PerlIO *
PerlIO_importFILE(FILE *stdio, const char *mode)
{
    dTHX;
    PerlIO *f = NULL;
#ifdef __MVS__
         int rc;
         char filename[FILENAME_MAX];
         fldata_t fileinfo;
#endif
    if (stdio) {
        PerlIOStdio *s;
        int fd0 = fileno(stdio);
        if (fd0 < 0) {
#ifdef __MVS__
                          rc = fldata(stdio,filename,&fileinfo);
                          if(rc != 0){
                                  return NULL;
                          }
                          if(fileinfo.__dsorgHFS){
            return NULL;
        }
                          /*This MVS dataset , OK!*/
#else
            return NULL;
#endif
        }
        if (!mode || !*mode) {
            /* We need to probe to see how we can open the stream
               so start with read/write and then try write and read
               we dup() so that we can fclose without loosing the fd.

               Note that the errno value set by a failing fdopen
               varies between stdio implementations.
             */
            const int fd = PerlLIO_dup_cloexec(fd0);
            FILE *f2;
            if (fd < 0) {
                return f;
            }
            f2 = PerlSIO_fdopen(fd, (mode = "r+"));
            if (!f2) {
                f2 = PerlSIO_fdopen(fd, (mode = "w"));
            }
            if (!f2) {
                f2 = PerlSIO_fdopen(fd, (mode = "r"));
            }
            if (!f2) {
                /* Don't seem to be able to open */
                PerlLIO_close(fd);
                return f;
            }
            fclose(f2);
        }
        if ((f = PerlIO_push(aTHX_(PerlIO_allocate(aTHX)), PERLIO_FUNCS_CAST(&PerlIO_stdio), mode, NULL))) {
            s = PerlIOSelf(f, PerlIOStdio);
            s->stdio = stdio;
            fd0 = fileno(stdio);
            if(fd0 != -1){
                PerlIOUnix_refcnt_inc(fd0);
                setfd_cloexec_or_inhexec_by_sysfdness(fd0);
            }
#ifdef __MVS__
                else{
                        rc = fldata(stdio,filename,&fileinfo);
                        if(rc != 0){
                                PerlIOUnix_refcnt_inc(fd0);
                        }
                        if(fileinfo.__dsorgHFS){
                                PerlIOUnix_refcnt_inc(fd0);
                        }
                          /*This MVS dataset , OK!*/
                }
#endif
        }
    }
    return f;
}

PerlIO *
PerlIOStdio_open(pTHX_ PerlIO_funcs *self, PerlIO_list_t *layers,
                 IV n, const char *mode, int fd, int imode,
                 int perm, PerlIO *f, int narg, SV **args)
{
    char tmode[8];
    if (PerlIOValid(f)) {
        STRLEN len;
        const char * const path = SvPV_const(*args, len);
        PerlIOStdio * const s = PerlIOSelf(f, PerlIOStdio);
        FILE *stdio;
        if (!IS_SAFE_PATHNAME(path, len, "open"))
            return NULL;
        PerlIOUnix_refcnt_dec(fileno(s->stdio));
        stdio = PerlSIO_freopen(path, PerlIOStdio_mode(mode, tmode),
                                s->stdio);
        if (!s->stdio)
            return NULL;
        s->stdio = stdio;
        fd = fileno(stdio);
        PerlIOUnix_refcnt_inc(fd);
        setfd_cloexec_or_inhexec_by_sysfdness(fd);
        return f;
    }
    else {
        if (narg > 0) {
            STRLEN len;
            const char * const path = SvPV_const(*args, len);
            if (!IS_SAFE_PATHNAME(path, len, "open"))
                return NULL;
            if (*mode == IoTYPE_NUMERIC) {
                mode++;
                fd = PerlLIO_open3_cloexec(path, imode, perm);
            }
            else {
                FILE *stdio;
                bool appended = FALSE;
#ifdef __CYGWIN__
                /* Cygwin wants its 'b' early. */
                appended = TRUE;
                mode = PerlIOStdio_mode(mode, tmode);
#endif
                stdio = PerlSIO_fopen(path, mode);
                if (stdio) {
                    if (!f) {
                        f = PerlIO_allocate(aTHX);
                    }
                    if (!appended)
                        mode = PerlIOStdio_mode(mode, tmode);
                    f = PerlIO_push(aTHX_ f, self, mode, PerlIOArg);
                    if (f) {
                        PerlIOSelf(f, PerlIOStdio)->stdio = stdio;
                        fd = fileno(stdio);
                        PerlIOUnix_refcnt_inc(fd);
                        setfd_cloexec_or_inhexec_by_sysfdness(fd);
                    } else {
                        PerlSIO_fclose(stdio);
                    }
                    return f;
                }
                else {
                    return NULL;
                }
            }
        }
        if (fd >= 0) {
            FILE *stdio = NULL;
            int init = 0;
            if (*mode == IoTYPE_IMPLICIT) {
                init = 1;
                mode++;
            }
            if (init) {
                switch (fd) {
                case 0:
                    stdio = PerlSIO_stdin;
                    break;
                case 1:
                    stdio = PerlSIO_stdout;
                    break;
                case 2:
                    stdio = PerlSIO_stderr;
                    break;
                }
            }
            else {
                stdio = PerlSIO_fdopen(fd, mode =
                                       PerlIOStdio_mode(mode, tmode));
            }
            if (stdio) {
                if (!f) {
                    f = PerlIO_allocate(aTHX);
                }
                if ((f = PerlIO_push(aTHX_ f, self, mode, PerlIOArg))) {
                    PerlIOSelf(f, PerlIOStdio)->stdio = stdio;
                    fd = fileno(stdio);
                    PerlIOUnix_refcnt_inc(fd);
                    setfd_cloexec_or_inhexec_by_sysfdness(fd);
                }
                return f;
            }
            PerlLIO_close(fd);
        }
    }
    return NULL;
}

PerlIO *
PerlIOStdio_dup(pTHX_ PerlIO *f, PerlIO *o, CLONE_PARAMS *param, int flags)
{
    /* This assumes no layers underneath - which is what
       happens, but is not how I remember it. NI-S 2001/10/16
     */
    if ((f = PerlIOBase_dup(aTHX_ f, o, param, flags))) {
        FILE *stdio = PerlIOSelf(o, PerlIOStdio)->stdio;
        const int fd = fileno(stdio);
        char mode[8];
        if (flags & PERLIO_DUP_FD) {
            const int dfd = PerlLIO_dup_cloexec(fileno(stdio));
            if (dfd >= 0) {
                stdio = PerlSIO_fdopen(dfd, PerlIO_modestr(o,mode));
                goto set_this;
            }
            else {
                NOOP;
                /* FIXME: To avoid messy error recovery if dup fails
                   re-use the existing stdio as though flag was not set
                 */
            }
        }
        stdio = PerlSIO_fdopen(fd, PerlIO_modestr(o,mode));
    set_this:
        PerlIOSelf(f, PerlIOStdio)->stdio = stdio;
        if(stdio) {
            int fd = fileno(stdio);
            PerlIOUnix_refcnt_inc(fd);
            setfd_cloexec_or_inhexec_by_sysfdness(fd);
        }
    }
    return f;
}

static int
PerlIOStdio_invalidate_fileno(pTHX_ FILE *f)
{
    PERL_UNUSED_CONTEXT;

    /* XXX this could use PerlIO_canset_fileno() and
     * PerlIO_set_fileno() support from Configure
     */
#if defined(HAS_FDCLOSE)
    return fdclose(f, NULL) == 0 ? 1 : 0;
#elif defined(__UCLIBC__)
    /* uClibc must come before glibc because it defines __GLIBC__ as well. */
    f->__filedes = -1;
    return 1;
#elif defined(__GLIBC__)
    /* There may be a better way for GLIBC:
        - libio.h defines a flag to not close() on cleanup
     */	
    f->_fileno = -1;
    return 1;
#elif defined(__sun)
    PERL_UNUSED_ARG(f);
    return 0;
#elif defined(__hpux)
    f->__fileH = 0xff;
    f->__fileL = 0xff;
    return 1;
   /* Next one ->_file seems to be a reasonable fallback, i.e. if
      your platform does not have special entry try this one.
      [For OSF only have confirmation for Tru64 (alpha)
      but assume other OSFs will be similar.]
    */
#elif defined(_AIX) || defined(__osf__) || defined(__irix__)
    f->_file = -1;
    return 1;
#elif defined(__FreeBSD__)
    /* There may be a better way on FreeBSD:
        - we could insert a dummy func in the _close function entry
        f->_close = (int (*)(void *)) dummy_close;
     */
    f->_file = -1;
    return 1;
#elif defined(__OpenBSD__)
    /* There may be a better way on OpenBSD:
        - we could insert a dummy func in the _close function entry
        f->_close = (int (*)(void *)) dummy_close;
     */
    f->_file = -1;
    return 1;
#elif defined(__EMX__)
    /* f->_flags &= ~_IOOPEN; */	/* Will leak stream->_buffer */
    f->_handle = -1;
    return 1;
#elif defined(__CYGWIN__)
    /* There may be a better way on CYGWIN:
        - we could insert a dummy func in the _close function entry
        f->_close = (int (*)(void *)) dummy_close;
     */
    f->_file = -1;
    return 1;
#elif defined(WIN32)
    PERLIO_FILE_file(f) = -1;
    return 1;
#else
#  if 0
    /* Sarathy's code did this - we fall back to a dup/dup2 hack
       (which isn't thread safe) instead
     */
#    error "Don't know how to set FILE.fileno on your platform"
#  endif
    PERL_UNUSED_ARG(f);
    return 0;
#endif
}

IV
PerlIOStdio_close(pTHX_ PerlIO *f)
{
    FILE * const stdio = PerlIOSelf(f, PerlIOStdio)->stdio;
    if (!stdio) {
        errno = EBADF;
        return -1;
    }
    else {
        const int fd = fileno(stdio);
        int invalidate = 0;
        IV result = 0;
        int dupfd = -1;
        dSAVEDERRNO;
#ifdef SOCKS5_VERSION_NAME
        /* Socks lib overrides close() but stdio isn't linked to
           that library (though we are) - so we must call close()
           on sockets on stdio's behalf.
         */
        int optval;
        Sock_size_t optlen = sizeof(int);
        if (getsockopt(fd, SOL_SOCKET, SO_TYPE, (void *) &optval, &optlen) == 0)
            invalidate = 1;
#endif
        /* Test for -1, as *BSD stdio (at least) on fclose sets the FILE* such
           that a subsequent fileno() on it returns -1. Don't want to croak()
           from within PerlIOUnix_refcnt_dec() if some buggy caller code is
           trying to close an already closed handle which somehow it still has
           a reference to. (via.xs, I'm looking at you).  */
        if (fd != -1 && PerlIOUnix_refcnt_dec(fd) > 0) {
            /* File descriptor still in use */
            invalidate = 1;
        }
        if (invalidate) {
            /* For STD* handles, don't close stdio, since we shared the FILE *, too. */
            if (stdio == stdin) /* Some stdios are buggy fflush-ing inputs */
                return 0;
            if (stdio == stdout || stdio == stderr)
                return PerlIO_flush(f);
        }
        MUTEX_LOCK(&PL_perlio_mutex);
        /* Right. We need a mutex here because for a brief while we
           will have the situation that fd is actually closed. Hence if
           a second thread were to get into this block, its dup() would
           likely return our fd as its dupfd. (after all, it is closed)
           Then if we get to the dup2() first, we blat the fd back
           (messing up its temporary as a side effect) only for it to
           then close its dupfd (== our fd) in its close(dupfd) */

        /* There is, of course, a race condition, that any other thread
           trying to input/output/whatever on this fd will be stuffed
           for the duration of this little manoeuvrer. Perhaps we
           should hold an IO mutex for the duration of every IO
           operation if we know that invalidate doesn't work on this
           platform, but that would suck, and could kill performance.

           Except that correctness trumps speed.
           Advice from klortho #11912. */
        if (invalidate) {
            /* Tricky - must fclose(stdio) to free memory but not close(fd)
               Use Sarathy's trick from maint-5.6 to invalidate the
               fileno slot of the FILE *
            */
            result = PerlIO_flush(f);
            SAVE_ERRNO;
            invalidate = PerlIOStdio_invalidate_fileno(aTHX_ stdio);
            if (!invalidate) {
                dupfd = PerlLIO_dup_cloexec(fd);
#ifdef USE_ITHREADS
                if (dupfd < 0) {
                    /* Oh cXap. This isn't going to go well. Not sure if we can
                       recover from here, or if closing this particular FILE *
                       is a good idea now.  */
                }
#endif
            }
        } else {
            SAVE_ERRNO;   /* This is here only to silence compiler warnings */
        }
        result = PerlSIO_fclose(stdio);
        /* We treat error from stdio as success if we invalidated
           errno may NOT be expected EBADF
         */
        if (invalidate && result != 0) {
            RESTORE_ERRNO;
            result = 0;
        }
#ifdef SOCKS5_VERSION_NAME
        /* in SOCKS' case, let close() determine return value */
        result = close(fd);
#endif
        if (dupfd >= 0) {
            PerlLIO_dup2_cloexec(dupfd, fd);
            setfd_inhexec_for_sysfd(fd);
            PerlLIO_close(dupfd);
        }
        MUTEX_UNLOCK(&PL_perlio_mutex);
        return result;
    }
}

SSize_t
PerlIOStdio_read(pTHX_ PerlIO *f, void *vbuf, Size_t count)
{
    FILE * s;
    SSize_t got = 0;
    if (PerlIO_lockcnt(f)) /* in use: abort ungracefully */
        return -1;
    s = PerlIOSelf(f, PerlIOStdio)->stdio;
    for (;;) {
        if (count == 1) {
            STDCHAR *buf = (STDCHAR *) vbuf;
            /*
             * Perl is expecting PerlIO_getc() to fill the buffer Linux's
             * stdio does not do that for fread()
             */
            const int ch = PerlSIO_fgetc(s);
            if (ch != EOF) {
                *buf = ch;
                got = 1;
            }
        }
        else
            got = PerlSIO_fread(vbuf, 1, count, s);
        if (got == 0 && PerlSIO_ferror(s))
            got = -1;
        if (got >= 0 || errno != EINTR)
            break;
        if (PL_sig_pending && S_perlio_async_run(aTHX_ f))
            return -1;
        SETERRNO(0,0);	/* just in case */
    }
#ifdef __sgi
    /* Under some circumstances IRIX stdio fgetc() and fread()
     * set the errno to ENOENT, which makes no sense according
     * to either IRIX or POSIX.  [rt.perl.org #123977] */
    if (errno == ENOENT) SETERRNO(0,0);
#endif
    return got;
}

SSize_t
PerlIOStdio_unread(pTHX_ PerlIO *f, const void *vbuf, Size_t count)
{
    SSize_t unread = 0;
    FILE * const s = PerlIOSelf(f, PerlIOStdio)->stdio;

#ifdef STDIO_BUFFER_WRITABLE
    if (PerlIO_fast_gets(f) && PerlIO_has_base(f)) {
        STDCHAR *buf = ((STDCHAR *) vbuf) + count;
        STDCHAR *base = PerlIO_get_base(f);
        SSize_t cnt   = PerlIO_get_cnt(f);
        STDCHAR *ptr  = PerlIO_get_ptr(f);
        SSize_t avail = ptr - base;
        if (avail > 0) {
            if (avail > count) {
                avail = count;
            }
            ptr -= avail;
            Move(buf-avail,ptr,avail,STDCHAR);
            count -= avail;
            unread += avail;
            PerlIO_set_ptrcnt(f,ptr,cnt+avail);
            if (PerlSIO_feof(s) && unread >= 0)
                PerlSIO_clearerr(s);
        }
    }
    else
#endif
    if (PerlIO_has_cntptr(f)) {
        /* We can get pointer to buffer but not its base
           Do ungetc() but check chars are ending up in the
           buffer
         */
        STDCHAR *eptr = (STDCHAR*)PerlSIO_get_ptr(s);
        STDCHAR *buf = ((STDCHAR *) vbuf) + count;
        while (count > 0) {
            const int ch = (U8) *--buf;
            if (ungetc(ch,s) != ch) {
                /* ungetc did not work */
                break;
            }
            if ((STDCHAR*)PerlSIO_get_ptr(s) != --eptr || (((U8) *eptr) != ch)) {
                /* Did not change pointer as expected */
                if (fgetc(s) != EOF)  /* get char back again */
                    break;
            }
            /* It worked ! */
            count--;
            unread++;
        }
    }

    if (count > 0) {
        unread += PerlIOBase_unread(aTHX_ f, vbuf, count);
    }
    return unread;
}

SSize_t
PerlIOStdio_write(pTHX_ PerlIO *f, const void *vbuf, Size_t count)
{
    SSize_t got;
    if (PerlIO_lockcnt(f)) /* in use: abort ungracefully */
        return -1;
    for (;;) {
        got = PerlSIO_fwrite(vbuf, 1, count,
                              PerlIOSelf(f, PerlIOStdio)->stdio);
        if (got >= 0 || errno != EINTR)
            break;
        if (PL_sig_pending && S_perlio_async_run(aTHX_ f))
            return -1;
        SETERRNO(0,0);	/* just in case */
    }
    return got;
}

IV
PerlIOStdio_seek(pTHX_ PerlIO *f, Off_t offset, int whence)
{
    FILE * const stdio = PerlIOSelf(f, PerlIOStdio)->stdio;
    PERL_UNUSED_CONTEXT;

    return PerlSIO_fseek(stdio, offset, whence);
}

Off_t
PerlIOStdio_tell(pTHX_ PerlIO *f)
{
    FILE * const stdio = PerlIOSelf(f, PerlIOStdio)->stdio;
    PERL_UNUSED_CONTEXT;

    return PerlSIO_ftell(stdio);
}

IV
PerlIOStdio_flush(pTHX_ PerlIO *f)
{
    FILE * const stdio = PerlIOSelf(f, PerlIOStdio)->stdio;
    PERL_UNUSED_CONTEXT;

    if (PerlIOBase(f)->flags & PERLIO_F_CANWRITE) {
        return PerlSIO_fflush(stdio);
    }
    else {
        NOOP;
#if 0
        /*
         * FIXME: This discards ungetc() and pre-read stuff which is not
         * right if this is just a "sync" from a layer above Suspect right
         * design is to do _this_ but not have layer above flush this
         * layer read-to-read
         */
        /*
         * Not writeable - sync by attempting a seek
         */
        dSAVE_ERRNO;
        if (PerlSIO_fseek(stdio, (Off_t) 0, SEEK_CUR) != 0)
            RESTORE_ERRNO;
#endif
    }
    return 0;
}

IV
PerlIOStdio_eof(pTHX_ PerlIO *f)
{
    PERL_UNUSED_CONTEXT;

    return PerlSIO_feof(PerlIOSelf(f, PerlIOStdio)->stdio);
}

IV
PerlIOStdio_error(pTHX_ PerlIO *f)
{
    PERL_UNUSED_CONTEXT;

    return PerlSIO_ferror(PerlIOSelf(f, PerlIOStdio)->stdio);
}

void
PerlIOStdio_clearerr(pTHX_ PerlIO *f)
{
    PERL_UNUSED_CONTEXT;

    PerlSIO_clearerr(PerlIOSelf(f, PerlIOStdio)->stdio);
}

void
PerlIOStdio_setlinebuf(pTHX_ PerlIO *f)
{
    PERL_UNUSED_CONTEXT;

#ifdef HAS_SETLINEBUF
    PerlSIO_setlinebuf(PerlIOSelf(f, PerlIOStdio)->stdio);
#else
    PerlSIO_setvbuf(PerlIOSelf(f, PerlIOStdio)->stdio, NULL, _IOLBF, 0);
#endif
}

#ifdef FILE_base
STDCHAR *
PerlIOStdio_get_base(pTHX_ PerlIO *f)
{
    FILE * const stdio = PerlIOSelf(f, PerlIOStdio)->stdio;
    PERL_UNUSED_CONTEXT;
    return (STDCHAR*)PerlSIO_get_base(stdio);
}

Size_t
PerlIOStdio_get_bufsiz(pTHX_ PerlIO *f)
{
    FILE * const stdio = PerlIOSelf(f, PerlIOStdio)->stdio;
    PERL_UNUSED_CONTEXT;
    return PerlSIO_get_bufsiz(stdio);
}
#endif

#ifdef USE_STDIO_PTR
STDCHAR *
PerlIOStdio_get_ptr(pTHX_ PerlIO *f)
{
    FILE * const stdio = PerlIOSelf(f, PerlIOStdio)->stdio;
    PERL_UNUSED_CONTEXT;
    return (STDCHAR*)PerlSIO_get_ptr(stdio);
}

SSize_t
PerlIOStdio_get_cnt(pTHX_ PerlIO *f)
{
    FILE * const stdio = PerlIOSelf(f, PerlIOStdio)->stdio;
    PERL_UNUSED_CONTEXT;
    return PerlSIO_get_cnt(stdio);
}

void
PerlIOStdio_set_ptrcnt(pTHX_ PerlIO *f, STDCHAR * ptr, SSize_t cnt)
{
    FILE * const stdio = PerlIOSelf(f, PerlIOStdio)->stdio;
    PERL_UNUSED_CONTEXT;
    if (ptr != NULL) {
#  ifdef STDIO_PTR_LVALUE
        /* This is a long-standing infamous mess.  The root of the
         * problem is that one cannot know the signedness of char, and
         * more precisely the signedness of FILE._ptr.  The following
         * things have been tried, and they have all failed (across
         * different compilers (remember that core needs to to build
         * also with c++) and compiler options:
         *
         * - casting the RHS to (void*) -- works in *some* places
         * - casting the LHS to (void*) -- totally unportable
         *
         * So let's try silencing the warning at least for gcc. */
        GCC_DIAG_IGNORE_STMT(-Wpointer-sign);
        PerlSIO_set_ptr(stdio, ptr); /* LHS STDCHAR* cast non-portable */
        GCC_DIAG_RESTORE_STMT;
#    ifdef STDIO_PTR_LVAL_SETS_CNT
        assert(PerlSIO_get_cnt(stdio) == (cnt));
#    endif
#    if (!defined(STDIO_PTR_LVAL_NOCHANGE_CNT))
        /*
         * Setting ptr _does_ change cnt - we are done
         */
        return;
#    endif
#  else                           /* STDIO_PTR_LVALUE */
        PerlProc_abort();
#  endif                          /* STDIO_PTR_LVALUE */
    }
    /*
     * Now (or only) set cnt
     */
#  ifdef STDIO_CNT_LVALUE
    PerlSIO_set_cnt(stdio, cnt);
#  elif (defined(STDIO_PTR_LVALUE) && defined(STDIO_PTR_LVAL_SETS_CNT))
    PerlSIO_set_ptr(stdio,
                    PerlSIO_get_ptr(stdio) + (PerlSIO_get_cnt(stdio) -
                                              cnt));
#  else                           /* STDIO_PTR_LVAL_SETS_CNT */
    PerlProc_abort();
#  endif                          /* STDIO_CNT_LVALUE */
}


#endif

IV
PerlIOStdio_fill(pTHX_ PerlIO *f)
{
    FILE * stdio;
    int c;
    PERL_UNUSED_CONTEXT;
    if (PerlIO_lockcnt(f)) /* in use: abort ungracefully */
        return -1;
    stdio = PerlIOSelf(f, PerlIOStdio)->stdio;

    /*
     * fflush()ing read-only streams can cause trouble on some stdio-s
     */
    if ((PerlIOBase(f)->flags & PERLIO_F_CANWRITE)) {
        if (PerlSIO_fflush(stdio) != 0)
            return EOF;
    }
    for (;;) {
        c = PerlSIO_fgetc(stdio);
        if (c != EOF)
            break;
        if (! PerlSIO_ferror(stdio) || errno != EINTR)
            return EOF;
        if (PL_sig_pending && S_perlio_async_run(aTHX_ f))
            return -1;
        SETERRNO(0,0);
    }

#if (defined(STDIO_PTR_LVALUE) && (defined(STDIO_CNT_LVALUE) || defined(STDIO_PTR_LVAL_SETS_CNT)))

#  ifdef STDIO_BUFFER_WRITABLE
    if (PerlIO_fast_gets(f) && PerlIO_has_base(f)) {
        /* Fake ungetc() to the real buffer in case system's ungetc
           goes elsewhere
         */
        STDCHAR *base = (STDCHAR*)PerlSIO_get_base(stdio);
        SSize_t cnt   = PerlSIO_get_cnt(stdio);
        STDCHAR *ptr  = (STDCHAR*)PerlSIO_get_ptr(stdio);
        if (ptr == base+1) {
            *--ptr = (STDCHAR) c;
            PerlIOStdio_set_ptrcnt(aTHX_ f,ptr,cnt+1);
            if (PerlSIO_feof(stdio))
                PerlSIO_clearerr(stdio);
            return 0;
        }
    }
    else
#  endif
    if (PerlIO_has_cntptr(f)) {
        STDCHAR ch = c;
        if (PerlIOStdio_unread(aTHX_ f,&ch,1) == 1) {
            return 0;
        }
    }
#endif

    /* If buffer snoop scheme above fails fall back to
       using ungetc().
     */
    if (PerlSIO_ungetc(c, stdio) != c)
        return EOF;

    return 0;
}



PERLIO_FUNCS_DECL(PerlIO_stdio) = {
    sizeof(PerlIO_funcs),
    "stdio",
    sizeof(PerlIOStdio),
    PERLIO_K_BUFFERED|PERLIO_K_RAW,
    PerlIOStdio_pushed,
    PerlIOBase_popped,
    PerlIOStdio_open,
    PerlIOBase_binmode,         /* binmode */
    NULL,
    PerlIOStdio_fileno,
    PerlIOStdio_dup,
    PerlIOStdio_read,
    PerlIOStdio_unread,
    PerlIOStdio_write,
    PerlIOStdio_seek,
    PerlIOStdio_tell,
    PerlIOStdio_close,
    PerlIOStdio_flush,
    PerlIOStdio_fill,
    PerlIOStdio_eof,
    PerlIOStdio_error,
    PerlIOStdio_clearerr,
    PerlIOStdio_setlinebuf,
#ifdef FILE_base
    PerlIOStdio_get_base,
    PerlIOStdio_get_bufsiz,
#else
    NULL,
    NULL,
#endif
#ifdef USE_STDIO_PTR
    PerlIOStdio_get_ptr,
    PerlIOStdio_get_cnt,
#   if defined(HAS_FAST_STDIO) && defined(USE_FAST_STDIO)
    PerlIOStdio_set_ptrcnt,
#   else
    NULL,
#   endif /* HAS_FAST_STDIO && USE_FAST_STDIO */
#else
    NULL,
    NULL,
    NULL,
#endif /* USE_STDIO_PTR */
};

/* Note that calls to PerlIO_exportFILE() are reversed using
 * PerlIO_releaseFILE(), not importFILE. */
FILE *
PerlIO_exportFILE(PerlIO * f, const char *mode)
{
    dTHX;
    FILE *stdio = NULL;
    if (PerlIOValid(f)) {
        char buf[8];
        int fd = PerlIO_fileno(f);
        if (fd < 0) {
            return NULL;
        }
        PerlIO_flush(f);
        if (!mode || !*mode) {
            mode = PerlIO_modestr(f, buf);
        }
        stdio = PerlSIO_fdopen(PerlIO_fileno(f), mode);
        if (stdio) {
            PerlIOl *l = *f;
            PerlIO *f2;
            /* De-link any lower layers so new :stdio sticks */
            *f = NULL;
            if ((f2 = PerlIO_push(aTHX_ f, PERLIO_FUNCS_CAST(&PerlIO_stdio), buf, NULL))) {
                PerlIOStdio *s = PerlIOSelf((f = f2), PerlIOStdio);
                s->stdio = stdio;
                PerlIOUnix_refcnt_inc(fileno(stdio));
                /* Link previous lower layers under new one */
                *PerlIONext(f) = l;
            }
            else {
                /* restore layers list */
                *f = l;
            }
        }
    }
    return stdio;
}


FILE *
PerlIO_findFILE(PerlIO *f)
{
    PerlIOl *l = *f;
    FILE *stdio;
    while (l) {
        if (l->tab == &PerlIO_stdio) {
            PerlIOStdio *s = PerlIOSelf(&l, PerlIOStdio);
            return s->stdio;
        }
        l = *PerlIONext(&l);
    }
    /* Uses fallback "mode" via PerlIO_modestr() in PerlIO_exportFILE */
    /* However, we're not really exporting a FILE * to someone else (who
       becomes responsible for closing it, or calling PerlIO_releaseFILE())
       So we need to undo its reference count increase on the underlying file
       descriptor. We have to do this, because if the loop above returns you
       the FILE *, then *it* didn't increase any reference count. So there's
       only one way to be consistent. */
    stdio = PerlIO_exportFILE(f, NULL);
    if (stdio) {
        const int fd = fileno(stdio);
        if (fd >= 0)
            PerlIOUnix_refcnt_dec(fd);
    }
    return stdio;
}

/* Use this to reverse PerlIO_exportFILE calls. */
void
PerlIO_releaseFILE(PerlIO *p, FILE *f)
{
    PerlIOl *l;
    while ((l = *p)) {
        if (l->tab == &PerlIO_stdio) {
            PerlIOStdio *s = PerlIOSelf(&l, PerlIOStdio);
            if (s->stdio == f) { /* not in a loop */
                const int fd = fileno(f);
                if (fd >= 0)
                    PerlIOUnix_refcnt_dec(fd);
                {
                    dTHX;
                    PerlIO_pop(aTHX_ p);
                }
                return;
            }
        }
        p = PerlIONext(p);
    }
    return;
}

/*--------------------------------------------------------------------------------------*/
/*
 * perlio buffer layer
 */

IV
PerlIOBuf_pushed(pTHX_ PerlIO *f, const char *mode, SV *arg, PerlIO_funcs *tab)
{
    PerlIOBuf *b = PerlIOSelf(f, PerlIOBuf);
    const int fd = PerlIO_fileno(f);
    if (fd >= 0 && PerlLIO_isatty(fd)) {
        PerlIOBase(f)->flags |= PERLIO_F_LINEBUF | PERLIO_F_TTY;
    }
    if (*PerlIONext(f)) {
        const Off_t posn = PerlIO_tell(PerlIONext(f));
        if (posn != (Off_t) - 1) {
            b->posn = posn;
        }
    }
    return PerlIOBase_pushed(aTHX_ f, mode, arg, tab);
}

PerlIO *
PerlIOBuf_open(pTHX_ PerlIO_funcs *self, PerlIO_list_t *layers,
               IV n, const char *mode, int fd, int imode, int perm,
               PerlIO *f, int narg, SV **args)
{
    if (PerlIOValid(f)) {
        PerlIO *next = PerlIONext(f);
        PerlIO_funcs *tab =
             PerlIO_layer_fetch(aTHX_ layers, n - 1, PerlIOBase(next)->tab);
        if (tab && tab->Open)
             next =
                  (*tab->Open)(aTHX_ tab, layers, n - 1, mode, fd, imode, perm,
                               next, narg, args);
        if (!next || (*PerlIOBase(f)->tab->Pushed) (aTHX_ f, mode, PerlIOArg, self) != 0) {
            return NULL;
        }
    }
    else {
        PerlIO_funcs *tab = PerlIO_layer_fetch(aTHX_ layers, n - 1, PerlIO_default_btm());
        int init = 0;
        if (*mode == IoTYPE_IMPLICIT) {
            init = 1;
            /*
             * mode++;
             */
        }
        if (tab && tab->Open)
             f = (*tab->Open)(aTHX_ tab, layers, n - 1, mode, fd, imode, perm,
                              f, narg, args);
        else
             SETERRNO(EINVAL, LIB_INVARG);
        if (f) {
            if (PerlIO_push(aTHX_ f, self, mode, PerlIOArg) == 0) {
                /*
                 * if push fails during open, open fails. close will pop us.
                 */
                PerlIO_close (f);
                return NULL;
            } else {
                fd = PerlIO_fileno(f);
                if (init && fd == 2) {
                    /*
                     * Initial stderr is unbuffered
                     */
                    PerlIOBase(f)->flags |= PERLIO_F_UNBUF;
                }
#ifdef PERLIO_USING_CRLF
#  ifdef PERLIO_IS_BINMODE_FD
                if (PERLIO_IS_BINMODE_FD(fd))
                    PerlIO_binmode(aTHX_ f,  '<'/*not used*/, O_BINARY, NULL);
                else
#  endif
                /*
                 * do something about failing setmode()? --jhi
                 */
                PerlLIO_setmode(fd, O_BINARY);
#endif
#ifdef VMS
                /* Enable line buffering with record-oriented regular files
                 * so we don't introduce an extraneous record boundary when
                 * the buffer fills up.
                 */
                if (PerlIOBase(f)->flags & PERLIO_F_CANWRITE) {
                    Stat_t st;
                    if (PerlLIO_fstat(fd, &st) == 0
                        && S_ISREG(st.st_mode)
                        && (st.st_fab_rfm == FAB$C_VAR 
                            || st.st_fab_rfm == FAB$C_VFC)) {
                        PerlIOBase(f)->flags |= PERLIO_F_LINEBUF;
                    }
                }
#endif
            }
        }
    }
    return f;
}

/*
 * This "flush" is akin to sfio's sync in that it handles files in either
 * read or write state.  For write state, we put the postponed data through
 * the next layers.  For read state, we seek() the next layers to the
 * offset given by current position in the buffer, and discard the buffer
 * state (XXXX supposed to be for seek()able buffers only, but now it is done
 * in any case?).  Then the pass the stick further in chain.
 */
IV
PerlIOBuf_flush(pTHX_ PerlIO *f)
{
    PerlIOBuf * const b = PerlIOSelf(f, PerlIOBuf);
    int code = 0;
    PerlIO *n = PerlIONext(f);
    if (PerlIOBase(f)->flags & PERLIO_F_WRBUF) {
        /*
         * write() the buffer
         */
        const STDCHAR *buf = b->buf;
        const STDCHAR *p = buf;
        while (p < b->ptr) {
            SSize_t count = PerlIO_write(n, p, b->ptr - p);
            if (count > 0) {
                p += count;
            }
            else if (count < 0 || PerlIO_error(n)) {
                PerlIOBase(f)->flags |= PERLIO_F_ERROR;
                PerlIO_save_errno(f);
                code = -1;
                break;
            }
        }
        b->posn += (p - buf);
    }
    else if (PerlIOBase(f)->flags & PERLIO_F_RDBUF) {
        STDCHAR *buf = PerlIO_get_base(f);
        /*
         * Note position change
         */
        b->posn += (b->ptr - buf);
        if (b->ptr < b->end) {
            /* We did not consume all of it - try and seek downstream to
               our logical position
             */
            if (PerlIOValid(n) && PerlIO_seek(n, b->posn, SEEK_SET) == 0) {
                /* Reload n as some layers may pop themselves on seek */
                b->posn = PerlIO_tell(n = PerlIONext(f));
            }
            else {
                /* Seek failed (e.g. pipe or tty). Do NOT clear buffer or pre-read
                   data is lost for good - so return saying "ok" having undone
                   the position adjust
                 */
                b->posn -= (b->ptr - buf);
                return code;
            }
        }
    }
    b->ptr = b->end = b->buf;
    PerlIOBase(f)->flags &= ~(PERLIO_F_RDBUF | PERLIO_F_WRBUF);
    /* We check for Valid because of dubious decision to make PerlIO_flush(NULL) flush all */
    if (PerlIOValid(n) && PerlIO_flush(n) != 0)
        code = -1;
    return code;
}

/* This discards the content of the buffer after b->ptr, and rereads
 * the buffer from the position off in the layer downstream; here off
 * is at offset corresponding to b->ptr - b->buf.
 */
IV
PerlIOBuf_fill(pTHX_ PerlIO *f)
{
    PerlIOBuf * const b = PerlIOSelf(f, PerlIOBuf);
    PerlIO *n = PerlIONext(f);
    SSize_t avail;
    /*
     * Down-stream flush is defined not to loose read data so is harmless.
     * we would not normally be fill'ing if there was data left in anycase.
     */
    if (PerlIO_flush(f) != 0)	/* XXXX Check that its seek() succeeded?! */
        return -1;
    if (PerlIOBase(f)->flags & PERLIO_F_TTY)
        PerlIOBase_flush_linebuf(aTHX);

    if (!b->buf)
        PerlIO_get_base(f);     /* allocate via vtable */

    assert(b->buf); /* The b->buf does get allocated via the vtable system. */

    b->ptr = b->end = b->buf;

    if (!PerlIOValid(n)) {
        PerlIOBase(f)->flags |= PERLIO_F_EOF;
        return -1;
    }

    if (PerlIO_fast_gets(n)) {
        /*
         * Layer below is also buffered. We do _NOT_ want to call its
         * ->Read() because that will loop till it gets what we asked for
         * which may hang on a pipe etc. Instead take anything it has to
         * hand, or ask it to fill _once_.
         */
        avail = PerlIO_get_cnt(n);
        if (avail <= 0) {
            avail = PerlIO_fill(n);
            if (avail == 0)
                avail = PerlIO_get_cnt(n);
            else {
                if (!PerlIO_error(n) && PerlIO_eof(n))
                    avail = 0;
            }
        }
        if (avail > 0) {
            STDCHAR *ptr = PerlIO_get_ptr(n);
            const SSize_t cnt = avail;
            if (avail > (SSize_t)b->bufsiz)
                avail = b->bufsiz;
            Copy(ptr, b->buf, avail, STDCHAR);
            PerlIO_set_ptrcnt(n, ptr + avail, cnt - avail);
        }
    }
    else {
        avail = PerlIO_read(n, b->ptr, b->bufsiz);
    }
    if (avail <= 0) {
        if (avail == 0)
            PerlIOBase(f)->flags |= PERLIO_F_EOF;
        else
        {
            PerlIOBase(f)->flags |= PERLIO_F_ERROR;
            PerlIO_save_errno(f);
        }
        return -1;
    }
    b->end = b->buf + avail;
    PerlIOBase(f)->flags |= PERLIO_F_RDBUF;
    return 0;
}

SSize_t
PerlIOBuf_read(pTHX_ PerlIO *f, void *vbuf, Size_t count)
{
    if (PerlIOValid(f)) {
        const PerlIOBuf * const b = PerlIOSelf(f, PerlIOBuf);
        if (!b->ptr)
            PerlIO_get_base(f);
        return PerlIOBase_read(aTHX_ f, vbuf, count);
    }
    return 0;
}

SSize_t
PerlIOBuf_unread(pTHX_ PerlIO *f, const void *vbuf, Size_t count)
{
    const STDCHAR *buf = (const STDCHAR *) vbuf + count;
    PerlIOBuf * const b = PerlIOSelf(f, PerlIOBuf);
    SSize_t unread = 0;
    SSize_t avail;
    if (PerlIOBase(f)->flags & PERLIO_F_WRBUF)
        PerlIO_flush(f);
    if (!b->buf)
        PerlIO_get_base(f);
    if (b->buf) {
        if (PerlIOBase(f)->flags & PERLIO_F_RDBUF) {
            /*
             * Buffer is already a read buffer, we can overwrite any chars
             * which have been read back to buffer start
             */
            avail = (b->ptr - b->buf);
        }
        else {
            /*
             * Buffer is idle, set it up so whole buffer is available for
             * unread
             */
            avail = b->bufsiz;
            b->end = b->buf + avail;
            b->ptr = b->end;
            PerlIOBase(f)->flags |= PERLIO_F_RDBUF;
            /*
             * Buffer extends _back_ from where we are now
             */
            b->posn -= b->bufsiz;
        }
        if ((SSize_t) count >= 0 && avail > (SSize_t) count) {
            /*
             * If we have space for more than count, just move count
             */
            avail = count;
        }
        if (avail > 0) {
            b->ptr -= avail;
            buf -= avail;
            /*
             * In simple stdio-like ungetc() case chars will be already
             * there
             */
            if (buf != b->ptr) {
                Copy(buf, b->ptr, avail, STDCHAR);
            }
            count -= avail;
            unread += avail;
            PerlIOBase(f)->flags &= ~PERLIO_F_EOF;
        }
    }
    if (count > 0) {
        unread += PerlIOBase_unread(aTHX_ f, vbuf, count);
    }
    return unread;
}

SSize_t
PerlIOBuf_write(pTHX_ PerlIO *f, const void *vbuf, Size_t count)
{
    PerlIOBuf * const b = PerlIOSelf(f, PerlIOBuf);
    const STDCHAR *buf = (const STDCHAR *) vbuf;
    const STDCHAR *flushptr = buf;
    Size_t written = 0;
    if (!b->buf)
        PerlIO_get_base(f);
    if (!(PerlIOBase(f)->flags & PERLIO_F_CANWRITE))
        return 0;
    if (PerlIOBase(f)->flags & PERLIO_F_RDBUF) {
        if (PerlIO_flush(f) != 0) {
            return 0;
        }
    }	
    if (PerlIOBase(f)->flags & PERLIO_F_LINEBUF) {
        flushptr = buf + count;
        while (flushptr > buf && *(flushptr - 1) != '\n')
            --flushptr;
    }
    while (count > 0) {
        SSize_t avail = b->bufsiz - (b->ptr - b->buf);
        if ((SSize_t) count >= 0 && (SSize_t) count < avail)
            avail = count;
        if (flushptr > buf && flushptr <= buf + avail)
            avail = flushptr - buf;
        PerlIOBase(f)->flags |= PERLIO_F_WRBUF;
        if (avail) {
            Copy(buf, b->ptr, avail, STDCHAR);
            count -= avail;
            buf += avail;
            written += avail;
            b->ptr += avail;
            if (buf == flushptr)
                PerlIO_flush(f);
        }
        if (b->ptr >= (b->buf + b->bufsiz))
            if (PerlIO_flush(f) == -1)
                return -1;
    }
    if (PerlIOBase(f)->flags & PERLIO_F_UNBUF)
        PerlIO_flush(f);
    return written;
}

IV
PerlIOBuf_seek(pTHX_ PerlIO *f, Off_t offset, int whence)
{
    IV code;
    if ((code = PerlIO_flush(f)) == 0) {
        PerlIOBase(f)->flags &= ~PERLIO_F_EOF;
        code = PerlIO_seek(PerlIONext(f), offset, whence);
        if (code == 0) {
            PerlIOBuf *b = PerlIOSelf(f, PerlIOBuf);
            b->posn = PerlIO_tell(PerlIONext(f));
        }
    }
    return code;
}

Off_t
PerlIOBuf_tell(pTHX_ PerlIO *f)
{
    PerlIOBuf * const b = PerlIOSelf(f, PerlIOBuf);
    /*
     * b->posn is file position where b->buf was read, or will be written
     */
    Off_t posn = b->posn;
    if ((PerlIOBase(f)->flags & PERLIO_F_APPEND) &&
        (PerlIOBase(f)->flags & PERLIO_F_WRBUF)) {
#if 1
        /* As O_APPEND files are normally shared in some sense it is better
           to flush :
         */  	
        PerlIO_flush(f);
#else	
        /* when file is NOT shared then this is sufficient */
        PerlIO_seek(PerlIONext(f),0, SEEK_END);
#endif
        posn = b->posn = PerlIO_tell(PerlIONext(f));
    }
    if (b->buf) {
        /*
         * If buffer is valid adjust position by amount in buffer
         */
        posn += (b->ptr - b->buf);
    }
    return posn;
}

IV
PerlIOBuf_popped(pTHX_ PerlIO *f)
{
    const IV code = PerlIOBase_popped(aTHX_ f);
    PerlIOBuf * const b = PerlIOSelf(f, PerlIOBuf);
    if (b->buf && b->buf != (STDCHAR *) & b->oneword) {
        Safefree(b->buf);
    }
    b->ptr = b->end = b->buf = NULL;
    PerlIOBase(f)->flags &= ~(PERLIO_F_RDBUF | PERLIO_F_WRBUF);
    return code;
}

IV
PerlIOBuf_close(pTHX_ PerlIO *f)
{
    const IV code = PerlIOBase_close(aTHX_ f);
    PerlIOBuf * const b = PerlIOSelf(f, PerlIOBuf);
    if (b->buf && b->buf != (STDCHAR *) & b->oneword) {
        Safefree(b->buf);
    }
    b->ptr = b->end = b->buf = NULL;
    PerlIOBase(f)->flags &= ~(PERLIO_F_RDBUF | PERLIO_F_WRBUF);
    return code;
}

STDCHAR *
PerlIOBuf_get_ptr(pTHX_ PerlIO *f)
{
    const PerlIOBuf * const b = PerlIOSelf(f, PerlIOBuf);
    if (!b->buf)
        PerlIO_get_base(f);
    return b->ptr;
}

SSize_t
PerlIOBuf_get_cnt(pTHX_ PerlIO *f)
{
    const PerlIOBuf * const b = PerlIOSelf(f, PerlIOBuf);
    if (!b->buf)
        PerlIO_get_base(f);
    if (PerlIOBase(f)->flags & PERLIO_F_RDBUF)
        return (b->end - b->ptr);
    return 0;
}

STDCHAR *
PerlIOBuf_get_base(pTHX_ PerlIO *f)
{
    PerlIOBuf * const b = PerlIOSelf(f, PerlIOBuf);
    PERL_UNUSED_CONTEXT;

    if (!b->buf) {
        if (!b->bufsiz)
            b->bufsiz = PERLIOBUF_DEFAULT_BUFSIZ;
        Newx(b->buf,b->bufsiz, STDCHAR);
        if (!b->buf) {
            b->buf = (STDCHAR *) & b->oneword;
            b->bufsiz = sizeof(b->oneword);
        }
        b->end = b->ptr = b->buf;
    }
    return b->buf;
}

Size_t
PerlIOBuf_bufsiz(pTHX_ PerlIO *f)
{
    const PerlIOBuf * const b = PerlIOSelf(f, PerlIOBuf);
    if (!b->buf)
        PerlIO_get_base(f);
    return (b->end - b->buf);
}

void
PerlIOBuf_set_ptrcnt(pTHX_ PerlIO *f, STDCHAR * ptr, SSize_t cnt)
{
    PerlIOBuf * const b = PerlIOSelf(f, PerlIOBuf);
#ifndef DEBUGGING
    PERL_UNUSED_ARG(cnt);
#endif
    if (!b->buf)
        PerlIO_get_base(f);
    b->ptr = ptr;
    assert(PerlIO_get_cnt(f) == cnt);
    assert(b->ptr >= b->buf);
    PerlIOBase(f)->flags |= PERLIO_F_RDBUF;
}

PerlIO *
PerlIOBuf_dup(pTHX_ PerlIO *f, PerlIO *o, CLONE_PARAMS *param, int flags)
{
 return PerlIOBase_dup(aTHX_ f, o, param, flags);
}



PERLIO_FUNCS_DECL(PerlIO_perlio) = {
    sizeof(PerlIO_funcs),
    "perlio",
    sizeof(PerlIOBuf),
    PERLIO_K_BUFFERED|PERLIO_K_RAW,
    PerlIOBuf_pushed,
    PerlIOBuf_popped,
    PerlIOBuf_open,
    PerlIOBase_binmode,         /* binmode */
    NULL,
    PerlIOBase_fileno,
    PerlIOBuf_dup,
    PerlIOBuf_read,
    PerlIOBuf_unread,
    PerlIOBuf_write,
    PerlIOBuf_seek,
    PerlIOBuf_tell,
    PerlIOBuf_close,
    PerlIOBuf_flush,
    PerlIOBuf_fill,
    PerlIOBase_eof,
    PerlIOBase_error,
    PerlIOBase_clearerr,
    PerlIOBase_setlinebuf,
    PerlIOBuf_get_base,
    PerlIOBuf_bufsiz,
    PerlIOBuf_get_ptr,
    PerlIOBuf_get_cnt,
    PerlIOBuf_set_ptrcnt,
};

/*--------------------------------------------------------------------------------------*/
/*
 * Temp layer to hold unread chars when cannot do it any other way
 */

IV
PerlIOPending_fill(pTHX_ PerlIO *f)
{
    /*
     * Should never happen
     */
    PerlIO_flush(f);
    return 0;
}

IV
PerlIOPending_close(pTHX_ PerlIO *f)
{
    /*
     * A tad tricky - flush pops us, then we close new top
     */
    PerlIO_flush(f);
    return PerlIO_close(f);
}

IV
PerlIOPending_seek(pTHX_ PerlIO *f, Off_t offset, int whence)
{
    /*
     * A tad tricky - flush pops us, then we seek new top
     */
    PerlIO_flush(f);
    return PerlIO_seek(f, offset, whence);
}


IV
PerlIOPending_flush(pTHX_ PerlIO *f)
{
    PerlIOBuf * const b = PerlIOSelf(f, PerlIOBuf);
    if (b->buf && b->buf != (STDCHAR *) & b->oneword) {
        Safefree(b->buf);
        b->buf = NULL;
    }
    PerlIO_pop(aTHX_ f);
    return 0;
}

void
PerlIOPending_set_ptrcnt(pTHX_ PerlIO *f, STDCHAR * ptr, SSize_t cnt)
{
    if (cnt <= 0) {
        PerlIO_flush(f);
    }
    else {
        PerlIOBuf_set_ptrcnt(aTHX_ f, ptr, cnt);
    }
}

IV
PerlIOPending_pushed(pTHX_ PerlIO *f, const char *mode, SV *arg, PerlIO_funcs *tab)
{
    const IV code = PerlIOBase_pushed(aTHX_ f, mode, arg, tab);
    PerlIOl * const l = PerlIOBase(f);
    /*
     * Our PerlIO_fast_gets must match what we are pushed on, or sv_gets()
     * etc. get muddled when it changes mid-string when we auto-pop.
     */
    l->flags = (l->flags & ~(PERLIO_F_FASTGETS | PERLIO_F_UTF8)) |
        (PerlIOBase(PerlIONext(f))->
         flags & (PERLIO_F_FASTGETS | PERLIO_F_UTF8));
    return code;
}

SSize_t
PerlIOPending_read(pTHX_ PerlIO *f, void *vbuf, Size_t count)
{
    SSize_t avail = PerlIO_get_cnt(f);
    SSize_t got = 0;
    if ((SSize_t) count >= 0 && (SSize_t)count < avail)
        avail = count;
    if (avail > 0)
        got = PerlIOBuf_read(aTHX_ f, vbuf, avail);
    if (got >= 0 && got < (SSize_t)count) {
        const SSize_t more =
            PerlIO_read(f, ((STDCHAR *) vbuf) + got, count - got);
        if (more >= 0 || got == 0)
            got += more;
    }
    return got;
}

PERLIO_FUNCS_DECL(PerlIO_pending) = {
    sizeof(PerlIO_funcs),
    "pending",
    sizeof(PerlIOBuf),
    PERLIO_K_BUFFERED|PERLIO_K_RAW,  /* not sure about RAW here */
    PerlIOPending_pushed,
    PerlIOBuf_popped,
    NULL,
    PerlIOBase_binmode,         /* binmode */
    NULL,
    PerlIOBase_fileno,
    PerlIOBuf_dup,
    PerlIOPending_read,
    PerlIOBuf_unread,
    PerlIOBuf_write,
    PerlIOPending_seek,
    PerlIOBuf_tell,
    PerlIOPending_close,
    PerlIOPending_flush,
    PerlIOPending_fill,
    PerlIOBase_eof,
    PerlIOBase_error,
    PerlIOBase_clearerr,
    PerlIOBase_setlinebuf,
    PerlIOBuf_get_base,
    PerlIOBuf_bufsiz,
    PerlIOBuf_get_ptr,
    PerlIOBuf_get_cnt,
    PerlIOPending_set_ptrcnt,
};



/*--------------------------------------------------------------------------------------*/
/*
 * crlf - translation On read translate CR,LF to "\n" we do this by
 * overriding ptr/cnt entries to hand back a line at a time and keeping a
 * record of which nl we "lied" about. On write translate "\n" to CR,LF
 *
 * c->nl points on the first byte of CR LF pair when it is temporarily
 * replaced by LF, or to the last CR of the buffer.  In the former case
 * the caller thinks that the buffer ends at c->nl + 1, in the latter
 * that it ends at c->nl; these two cases can be distinguished by
 * *c->nl.  c->nl is set during _getcnt() call, and unset during
 * _unread() and _flush() calls.
 * It only matters for read operations.
 */

typedef struct {
    PerlIOBuf base;             /* PerlIOBuf stuff */
    STDCHAR *nl;                /* Position of crlf we "lied" about in the
                                 * buffer */
} PerlIOCrlf;

/* Inherit the PERLIO_F_UTF8 flag from previous layer.
 * Otherwise the :crlf layer would always revert back to
 * raw mode.
 */
static void
S_inherit_utf8_flag(PerlIO *f)
{
    PerlIO *g = PerlIONext(f);
    if (PerlIOValid(g)) {
        if (PerlIOBase(g)->flags & PERLIO_F_UTF8) {
            PerlIOBase(f)->flags |= PERLIO_F_UTF8;
        }
    }
}

IV
PerlIOCrlf_pushed(pTHX_ PerlIO *f, const char *mode, SV *arg, PerlIO_funcs *tab)
{
    IV code;
    PerlIOBase(f)->flags |= PERLIO_F_CRLF;
    code = PerlIOBuf_pushed(aTHX_ f, mode, arg, tab);
#if 0
    DEBUG_i(
    PerlIO_debug("PerlIOCrlf_pushed f=%p %s %s fl=%08" UVxf "\n",
                 (void*)f, PerlIOBase(f)->tab->name, (mode) ? mode : "(Null)",
                 PerlIOBase(f)->flags);
    );
#endif
    {
      /* If the old top layer is a CRLF layer, reactivate it (if
       * necessary) and remove this new layer from the stack */
         PerlIO *g = PerlIONext(f);
         if (PerlIOValid(g)) {
              PerlIOl *b = PerlIOBase(g);
              if (b && b->tab == &PerlIO_crlf) {
                   if (!(b->flags & PERLIO_F_CRLF))
                        b->flags |= PERLIO_F_CRLF;
                   S_inherit_utf8_flag(g);
                   PerlIO_pop(aTHX_ f);
                   return code;
              }
         }
    }
    S_inherit_utf8_flag(f);
    return code;
}


SSize_t
PerlIOCrlf_unread(pTHX_ PerlIO *f, const void *vbuf, Size_t count)
{
    PerlIOCrlf * const c = PerlIOSelf(f, PerlIOCrlf);
    if (c->nl) {	/* XXXX Shouldn't it be done only if b->ptr > c->nl? */
        *(c->nl) = NATIVE_0xd;
        c->nl = NULL;
    }
    if (!(PerlIOBase(f)->flags & PERLIO_F_CRLF))
        return PerlIOBuf_unread(aTHX_ f, vbuf, count);
    else {
        const STDCHAR *buf = (const STDCHAR *) vbuf + count;
        PerlIOBuf *b = PerlIOSelf(f, PerlIOBuf);
        SSize_t unread = 0;
        if (PerlIOBase(f)->flags & PERLIO_F_WRBUF)
            PerlIO_flush(f);
        if (!b->buf)
            PerlIO_get_base(f);
        if (b->buf) {
            if (!(PerlIOBase(f)->flags & PERLIO_F_RDBUF)) {
                b->end = b->ptr = b->buf + b->bufsiz;
                PerlIOBase(f)->flags |= PERLIO_F_RDBUF;
                b->posn -= b->bufsiz;
            }
            while (count > 0 && b->ptr > b->buf) {
                const int ch = *--buf;
                if (ch == '\n') {
                    if (b->ptr - 2 >= b->buf) {
                        *--(b->ptr) = NATIVE_0xa;
                        *--(b->ptr) = NATIVE_0xd;
                        unread++;
                        count--;
                    }
                    else {
                    /* If b->ptr - 1 == b->buf, we are undoing reading 0xa */
                        *--(b->ptr) = NATIVE_0xa;   /* Works even if 0xa ==
                                                       '\r' */
                        unread++;
                        count--;
                    }
                }
                else {
                    *--(b->ptr) = ch;
                    unread++;
                    count--;
                }
            }
        }
        if (count > 0)
            unread += PerlIOBase_unread(aTHX_ f, (const STDCHAR *) vbuf + unread, count);
        return unread;
    }
}

/* XXXX This code assumes that buffer size >=2, but does not check it... */
SSize_t
PerlIOCrlf_get_cnt(pTHX_ PerlIO *f)
{
    PerlIOBuf * const b = PerlIOSelf(f, PerlIOBuf);
    if (!b->buf)
        PerlIO_get_base(f);
    if (PerlIOBase(f)->flags & PERLIO_F_RDBUF) {
        PerlIOCrlf * const c = PerlIOSelf(f, PerlIOCrlf);
        if ((PerlIOBase(f)->flags & PERLIO_F_CRLF) && (!c->nl || *c->nl == NATIVE_0xd)) {
            STDCHAR *nl = (c->nl) ? c->nl : b->ptr;
          scan:
            while (nl < b->end && *nl != NATIVE_0xd)
                nl++;
            if (nl < b->end && *nl == NATIVE_0xd) {
              test:
                if (nl + 1 < b->end) {
                    if (nl[1] == NATIVE_0xa) {
                        *nl = '\n';
                        c->nl = nl;
                    }
                    else {
                        /*
                         * Not CR,LF but just CR
                         */
                        nl++;
                        goto scan;
                    }
                }
                else {
                    /*
                     * Blast - found CR as last char in buffer
                     */

                    if (b->ptr < nl) {
                        /*
                         * They may not care, defer work as long as
                         * possible
                         */
                        c->nl = nl;
                        return (nl - b->ptr);
                    }
                    else {
                        int code;
                        b->ptr++;       /* say we have read it as far as
                                         * flush() is concerned */
                        b->buf++;       /* Leave space in front of buffer */
                        /* Note as we have moved buf up flush's
                           posn += ptr-buf
                           will naturally make posn point at CR
                         */
                        b->bufsiz--;    /* Buffer is thus smaller */
                        code = PerlIO_fill(f);  /* Fetch some more */
                        b->bufsiz++;    /* Restore size for next time */
                        b->buf--;       /* Point at space */
                        b->ptr = nl = b->buf;   /* Which is what we hand
                                                 * off */
                        *nl = NATIVE_0xd;      /* Fill in the CR */
                        if (code == 0)
                            goto test;  /* fill() call worked */
                        /*
                         * CR at EOF - just fall through
                         */
                        /* Should we clear EOF though ??? */
                    }
                }
            }
        }
        return (((c->nl) ? (c->nl + 1) : b->end) - b->ptr);
    }
    return 0;
}

void
PerlIOCrlf_set_ptrcnt(pTHX_ PerlIO *f, STDCHAR * ptr, SSize_t cnt)
{
    PerlIOBuf * const b = PerlIOSelf(f, PerlIOBuf);
    PerlIOCrlf * const c = PerlIOSelf(f, PerlIOCrlf);
    if (!b->buf)
        PerlIO_get_base(f);
    if (!ptr) {
        if (c->nl) {
            ptr = c->nl + 1;
            if (ptr == b->end && *c->nl == NATIVE_0xd) {
                /* Deferred CR at end of buffer case - we lied about count */
                ptr--;
            }
        }
        else {
            ptr = b->end;
        }
        ptr -= cnt;
    }
    else {
        NOOP;
#if 0
        /*
         * Test code - delete when it works ...
         */
        IV flags = PerlIOBase(f)->flags;
        STDCHAR *chk = (c->nl) ? (c->nl+1) : b->end;
        if (ptr+cnt == c->nl && c->nl+1 == b->end && *c->nl == NATIVE_0xd) {
          /* Deferred CR at end of buffer case - we lied about count */
          chk--;
        }
        chk -= cnt;

        if (ptr != chk ) {
            Perl_croak(aTHX_ "ptr wrong %p != %p fl=%08" UVxf
                       " nl=%p e=%p for %d", (void*)ptr, (void*)chk,
                       flags, c->nl, b->end, cnt);
        }
#endif
    }
    if (c->nl) {
        if (ptr > c->nl) {
            /*
             * They have taken what we lied about
             */
            *(c->nl) = NATIVE_0xd;
            c->nl = NULL;
            ptr++;
        }
    }
    b->ptr = ptr;
    PerlIOBase(f)->flags |= PERLIO_F_RDBUF;
}

SSize_t
PerlIOCrlf_write(pTHX_ PerlIO *f, const void *vbuf, Size_t count)
{
    if (!(PerlIOBase(f)->flags & PERLIO_F_CRLF))
        return PerlIOBuf_write(aTHX_ f, vbuf, count);
    else {
        PerlIOBuf * const b = PerlIOSelf(f, PerlIOBuf);
        const STDCHAR *buf = (const STDCHAR *) vbuf;
        const STDCHAR * const ebuf = buf + count;
        if (!b->buf)
            PerlIO_get_base(f);
        if (!(PerlIOBase(f)->flags & PERLIO_F_CANWRITE))
            return 0;
        while (buf < ebuf) {
            const STDCHAR * const eptr = b->buf + b->bufsiz;
            PerlIOBase(f)->flags |= PERLIO_F_WRBUF;
            while (buf < ebuf && b->ptr < eptr) {
                if (*buf == '\n') {
                    if ((b->ptr + 2) > eptr) {
                        /*
                         * Not room for both
                         */
                        PerlIO_flush(f);
                        break;
                    }
                    else {
                        *(b->ptr)++ = NATIVE_0xd;      /* CR */
                        *(b->ptr)++ = NATIVE_0xa;      /* LF */
                        buf++;
                        if (PerlIOBase(f)->flags & PERLIO_F_LINEBUF) {
                            PerlIO_flush(f);
                            break;
                        }
                    }
                }
                else {
                    *(b->ptr)++ = *buf++;
                }
                if (b->ptr >= eptr) {
                    PerlIO_flush(f);
                    break;
                }
            }
        }
        if (PerlIOBase(f)->flags & PERLIO_F_UNBUF)
            PerlIO_flush(f);
        return (buf - (STDCHAR *) vbuf);
    }
}

IV
PerlIOCrlf_flush(pTHX_ PerlIO *f)
{
    PerlIOCrlf * const c = PerlIOSelf(f, PerlIOCrlf);
    if (c->nl) {
        *(c->nl) = NATIVE_0xd;
        c->nl = NULL;
    }
    return PerlIOBuf_flush(aTHX_ f);
}

IV
PerlIOCrlf_binmode(pTHX_ PerlIO *f)
{
    if ((PerlIOBase(f)->flags & PERLIO_F_CRLF)) {
        /* In text mode - flush any pending stuff and flip it */
        PerlIOBase(f)->flags &= ~PERLIO_F_CRLF;
#ifndef PERLIO_USING_CRLF
        /* CRLF is unusual case - if this is just the :crlf layer pop it */
        PerlIO_pop(aTHX_ f);
#endif
    }
    return PerlIOBase_binmode(aTHX_ f);
}

PERLIO_FUNCS_DECL(PerlIO_crlf) = {
    sizeof(PerlIO_funcs),
    "crlf",
    sizeof(PerlIOCrlf),
    PERLIO_K_BUFFERED | PERLIO_K_CANCRLF | PERLIO_K_RAW,
    PerlIOCrlf_pushed,
    PerlIOBuf_popped,         /* popped */
    PerlIOBuf_open,
    PerlIOCrlf_binmode,       /* binmode */
    NULL,
    PerlIOBase_fileno,
    PerlIOBuf_dup,
    PerlIOBuf_read,             /* generic read works with ptr/cnt lies */
    PerlIOCrlf_unread,          /* Put CR,LF in buffer for each '\n' */
    PerlIOCrlf_write,           /* Put CR,LF in buffer for each '\n' */
    PerlIOBuf_seek,
    PerlIOBuf_tell,
    PerlIOBuf_close,
    PerlIOCrlf_flush,
    PerlIOBuf_fill,
    PerlIOBase_eof,
    PerlIOBase_error,
    PerlIOBase_clearerr,
    PerlIOBase_setlinebuf,
    PerlIOBuf_get_base,
    PerlIOBuf_bufsiz,
    PerlIOBuf_get_ptr,
    PerlIOCrlf_get_cnt,
    PerlIOCrlf_set_ptrcnt,
};

PerlIO *
Perl_PerlIO_stdin(pTHX)
{
    if (!PL_perlio) {
        PerlIO_stdstreams(aTHX);
    }
    return &PL_perlio[1].next;
}

PerlIO *
Perl_PerlIO_stdout(pTHX)
{
    if (!PL_perlio) {
        PerlIO_stdstreams(aTHX);
    }
    return &PL_perlio[2].next;
}

PerlIO *
Perl_PerlIO_stderr(pTHX)
{
    if (!PL_perlio) {
        PerlIO_stdstreams(aTHX);
    }
    return &PL_perlio[3].next;
}

/*--------------------------------------------------------------------------------------*/

char *
PerlIO_getname(PerlIO *f, char *buf)
{
#ifdef VMS
    dTHX;
    char *name = NULL;
    bool exported = FALSE;
    FILE *stdio = PerlIOSelf(f, PerlIOStdio)->stdio;
    if (!stdio) {
        stdio = PerlIO_exportFILE(f,0);
        exported = TRUE;
    }
    if (stdio) {
        name = fgetname(stdio, buf);
        if (exported) PerlIO_releaseFILE(f,stdio);
    }
    return name;
#else
    PERL_UNUSED_ARG(f);
    PERL_UNUSED_ARG(buf);
    Perl_croak_nocontext("Don't know how to get file name");
    return NULL;
#endif
}


/*--------------------------------------------------------------------------------------*/
/*
 * Functions which can be called on any kind of PerlIO implemented in
 * terms of above
 */

#undef PerlIO_fdopen
PerlIO *
PerlIO_fdopen(int fd, const char *mode)
{
    dTHX;
    return PerlIO_openn(aTHX_ NULL, mode, fd, 0, 0, NULL, 0, NULL);
}

#undef PerlIO_open
PerlIO *
PerlIO_open(const char *path, const char *mode)
{
    dTHX;
    SV *name = newSVpvn_flags(path, path == NULL ? 0 : strlen(path), SVs_TEMP);
    return PerlIO_openn(aTHX_ NULL, mode, -1, 0, 0, NULL, 1, &name);
}

#undef Perlio_reopen
PerlIO *
PerlIO_reopen(const char *path, const char *mode, PerlIO *f)
{
    dTHX;
    SV *name = newSVpvn_flags(path, path == NULL ? 0 : strlen(path), SVs_TEMP);
    return PerlIO_openn(aTHX_ NULL, mode, -1, 0, 0, f, 1, &name);
}

#undef PerlIO_getc
int
PerlIO_getc(PerlIO *f)
{
    dTHX;
    STDCHAR buf[1];
    if ( 1 == PerlIO_read(f, buf, 1) ) {
        return (unsigned char) buf[0];
    }
    return EOF;
}

#undef PerlIO_ungetc
int
PerlIO_ungetc(PerlIO *f, int ch)
{
    dTHX;
    if (ch != EOF) {
        STDCHAR buf = ch;
        if (PerlIO_unread(f, &buf, 1) == 1)
            return ch;
    }
    return EOF;
}

#undef PerlIO_putc
int
PerlIO_putc(PerlIO *f, int ch)
{
    dTHX;
    STDCHAR buf = ch;
    return PerlIO_write(f, &buf, 1);
}

#undef PerlIO_puts
int
PerlIO_puts(PerlIO *f, const char *s)
{
    dTHX;
    return PerlIO_write(f, s, strlen(s));
}

#undef PerlIO_rewind
void
PerlIO_rewind(PerlIO *f)
{
    dTHX;
    PerlIO_seek(f, (Off_t) 0, SEEK_SET);
    PerlIO_clearerr(f);
}

#undef PerlIO_vprintf
int
PerlIO_vprintf(PerlIO *f, const char *fmt, va_list ap)
{
    dTHX;
    SV * sv;
    const char *s;
    STRLEN len;
    SSize_t wrote;
#ifdef NEED_VA_COPY
    va_list apc;
    Perl_va_copy(ap, apc);
    sv = vnewSVpvf(fmt, &apc);
    va_end(apc);
#else
    sv = vnewSVpvf(fmt, &ap);
#endif
    s = SvPV_const(sv, len);
    wrote = PerlIO_write(f, s, len);
    SvREFCNT_dec(sv);
    return wrote;
}

#undef PerlIO_printf
int
PerlIO_printf(PerlIO *f, const char *fmt, ...)
{
    va_list ap;
    int result;
    va_start(ap, fmt);
    result = PerlIO_vprintf(f, fmt, ap);
    va_end(ap);
    return result;
}

#undef PerlIO_stdoutf
int
PerlIO_stdoutf(const char *fmt, ...)
{
    dTHX;
    va_list ap;
    int result;
    va_start(ap, fmt);
    result = PerlIO_vprintf(PerlIO_stdout(), fmt, ap);
    va_end(ap);
    return result;
}

#undef PerlIO_tmpfile
PerlIO *
PerlIO_tmpfile(void)
{
    return PerlIO_tmpfile_flags(0);
}

#define MKOSTEMP_MODES ( O_RDWR | O_CREAT | O_EXCL )
#define MKOSTEMP_MODE_MASK ( O_ACCMODE | O_CREAT | O_EXCL | O_TRUNC )

PerlIO *
PerlIO_tmpfile_flags(int imode)
{
#ifndef WIN32
     dTHX;
#endif
     PerlIO *f = NULL;
#ifdef WIN32
     const int fd = win32_tmpfd_mode(imode);
     if (fd >= 0)
          f = PerlIO_fdopen(fd, "w+b");
#elif ! defined(OS2)
     int fd = -1;
     char tempname[] = "/tmp/PerlIO_XXXXXX";
     const char * const tmpdir = TAINTING_get ? NULL : PerlEnv_getenv("TMPDIR");
     SV * sv = NULL;
     int old_umask = umask(0177);
     imode &= ~MKOSTEMP_MODE_MASK;
     if (tmpdir && *tmpdir) {
         /* if TMPDIR is set and not empty, we try that first */
         sv = newSVpv(tmpdir, 0);
         sv_catpv(sv, tempname + 4);
         fd = Perl_my_mkostemp_cloexec(SvPVX(sv), imode | O_VMS_DELETEONCLOSE);
     }
     if (fd < 0) {
         SvREFCNT_dec(sv);
         sv = NULL;
         /* else we try /tmp */
         fd = Perl_my_mkostemp_cloexec(tempname, imode | O_VMS_DELETEONCLOSE);
     }
     if (fd < 0) {
         /* Try cwd */
         sv = newSVpvs(".");
         sv_catpv(sv, tempname + 4);
         fd = Perl_my_mkostemp_cloexec(SvPVX(sv), imode | O_VMS_DELETEONCLOSE);
     }
     umask(old_umask);
     if (fd >= 0) {
         /* fdopen() with a numeric mode */
         char mode[8];
         int writing = 1;
         (void)PerlIO_intmode2str(imode | MKOSTEMP_MODES, mode, &writing);
         f = PerlIO_fdopen(fd, mode);
          if (f)
               PerlIOBase(f)->flags |= PERLIO_F_TEMP;
#  ifndef VMS
          PerlLIO_unlink(sv ? SvPVX_const(sv) : tempname);
#  endif
     }
     SvREFCNT_dec(sv);
#else	/* !HAS_MKSTEMP, fallback to stdio tmpfile(). */
     FILE * const stdio = PerlSIO_tmpfile();

     if (stdio)
          f = PerlIO_fdopen(fileno(stdio), "w+");

#endif /* else WIN32 */
     return f;
}

void
Perl_PerlIO_save_errno(pTHX_ PerlIO *f)
{
    PERL_UNUSED_CONTEXT;
    if (!PerlIOValid(f))
        return;
    PerlIOBase(f)->err = errno;
#ifdef VMS
    PerlIOBase(f)->os_err = vaxc$errno;
#elif defined(OS2)
    PerlIOBase(f)->os_err = Perl_rc;
#elif defined(WIN32)
    PerlIOBase(f)->os_err = GetLastError();
#endif
}

void
Perl_PerlIO_restore_errno(pTHX_ PerlIO *f)
{
    PERL_UNUSED_CONTEXT;
    if (!PerlIOValid(f))
        return;
    SETERRNO(PerlIOBase(f)->err, PerlIOBase(f)->os_err);
#ifdef OS2
    Perl_rc = PerlIOBase(f)->os_err);
#elif defined(WIN32)
    SetLastError(PerlIOBase(f)->os_err);
#endif
}

#undef HAS_FSETPOS
#undef HAS_FGETPOS


/*======================================================================================*/
/*
 * Now some functions in terms of above which may be needed even if we are
 * not in true PerlIO mode
 */
const char *
Perl_PerlIO_context_layers(pTHX_ const char *mode)
{
    /* Returns the layers set by "use open" */

    const char *direction = NULL;
    SV *layers;
    /*
     * Need to supply default layer info from open.pm
     */

    if (!PL_curcop)
        return NULL;

    if (mode && mode[0] != 'r') {
        if (PL_curcop->cop_hints & HINT_LEXICAL_IO_OUT)
            direction = "open>";
    } else {
        if (PL_curcop->cop_hints & HINT_LEXICAL_IO_IN)
            direction = "open<";
    }
    if (!direction)
        return NULL;

    layers = cop_hints_fetch_pvn(PL_curcop, direction, 5, 0, 0);

    assert(layers);
    return SvOK(layers) ? SvPV_nolen_const(layers) : NULL;
}


#ifndef HAS_FSETPOS
#  undef PerlIO_setpos
int
PerlIO_setpos(PerlIO *f, SV *pos)
{
    if (SvOK(pos)) {
        if (f) {
            dTHX;
            STRLEN len;
            const Off_t * const posn = (Off_t *) SvPV(pos, len);
            if(len == sizeof(Off_t))
                return PerlIO_seek(f, *posn, SEEK_SET);
        }
    }
    SETERRNO(EINVAL, SS_IVCHAN);
    return -1;
}
#else
#  undef PerlIO_setpos
int
PerlIO_setpos(PerlIO *f, SV *pos)
{
    if (SvOK(pos)) {
        if (f) {
            dTHX;
            STRLEN len;
            Fpos_t * const fpos = (Fpos_t *) SvPV(pos, len);
            if(len == sizeof(Fpos_t))
#  if defined(USE_64_BIT_STDIO) && defined(USE_FSETPOS64)
                return fsetpos64(f, fpos);
#  else
                return fsetpos(f, fpos);
#  endif
        }
    }
    SETERRNO(EINVAL, SS_IVCHAN);
    return -1;
}
#endif

#ifndef HAS_FGETPOS
#  undef PerlIO_getpos
int
PerlIO_getpos(PerlIO *f, SV *pos)
{
    dTHX;
    Off_t posn = PerlIO_tell(f);
    sv_setpvn(pos, (char *) &posn, sizeof(posn));
    return (posn == (Off_t) - 1) ? -1 : 0;
}
#else
#  undef PerlIO_getpos
int
PerlIO_getpos(PerlIO *f, SV *pos)
{
    dTHX;
    Fpos_t fpos;
    int code;
#  if defined(USE_64_BIT_STDIO) && defined(USE_FSETPOS64)
    code = fgetpos64(f, &fpos);
#  else
    code = fgetpos(f, &fpos);
#  endif
    sv_setpvn(pos, (char *) &fpos, sizeof(fpos));
    return code;
}
#endif

/* print a failure format string message to stderr and fail exit the process
   using only libc without depending on any perl data structures being
   initialized.
*/

void
Perl_noperl_die(const char* pat, ...)
{
    va_list arglist;
    PERL_ARGS_ASSERT_NOPERL_DIE;
    va_start(arglist, pat);
    vfprintf(stderr, pat, arglist);
    va_end(arglist);
    exit(1);
}

/*
 * ex: set ts=8 sts=4 sw=4 et:
 */
