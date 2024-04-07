/*******************************************************************************
*
*  Version 2.x, Copyright (C) 2007-2013, Marcus Holland-Moritz <mhx@cpan.org>.
*  Version 1.x, Copyright (C) 1999, Graham Barr <gbarr@pobox.com>.
*
*  This program is free software; you can redistribute it and/or
*  modify it under the same terms as Perl itself.
*
*******************************************************************************/

#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#ifndef NO_PPPORT_H
#  define NEED_sv_2pv_flags
#  define NEED_sv_pvn_force_flags
#  include "ppport.h"
#endif

#include <sys/types.h>

#if defined(HAS_MSG) || defined(HAS_SEM) || defined(HAS_SHM)
#  ifndef HAS_SEM
#    include <sys/ipc.h>
#  endif
#  ifdef HAS_MSG
#    include <sys/msg.h>
#  endif
#  ifdef HAS_SHM
#    if defined(PERL_SCO) || defined(PERL_ISC)
#      include <sys/sysmacros.h>	/* SHMLBA */
#    endif
#    include <sys/shm.h>
#    ifndef HAS_SHMAT_PROTOTYPE
       extern Shmat_t shmat(int, char *, int);
#    endif
#    if defined(HAS_SYSCONF) && defined(_SC_PAGESIZE)
#      undef  SHMLBA /* not static: determined at boot time */
#      define SHMLBA sysconf(_SC_PAGESIZE)
#    elif defined(HAS_GETPAGESIZE)
#      undef  SHMLBA /* not static: determined at boot time */
#      define SHMLBA getpagesize()
#    endif
#  endif
#endif

/* Required to get 'struct pte' for SHMLBA on ULTRIX. */
#if defined(__ultrix) || defined(__ultrix__) || defined(ultrix)
#include <machine/pte.h>
#endif

/* Required in BSDI to get PAGE_SIZE definition for SHMLBA.
 * Ugly.  More beautiful solutions welcome.
 * Shouting at BSDI sounds quite beautiful. */
#ifdef __bsdi__
#  include <vm/vm_param.h>	/* move upwards under HAS_SHM? */
#endif

#ifndef S_IRWXU
#  ifdef S_IRUSR
#    define S_IRWXU (S_IRUSR|S_IWUSR|S_IXUSR)
#    define S_IRWXG (S_IRGRP|S_IWGRP|S_IXGRP)
#    define S_IRWXO (S_IROTH|S_IWOTH|S_IXOTH)
#  else
#    define S_IRWXU 0700
#    define S_IRWXG 0070
#    define S_IRWXO 0007
#  endif
#endif

#define AV_FETCH_IV(ident, av, index)                         \
        STMT_START {                                          \
          SV **svp;                                           \
          if ((svp = av_fetch((av), (index), FALSE)) != NULL) \
            ident = SvIV(*svp);                               \
        } STMT_END

#define AV_STORE_IV(ident, av, index)                         \
          av_store((av), (index), newSViv(ident))

static const char *s_fmt_not_isa = "Method %s not called a %s object";
static const char *s_bad_length = "Bad arg length for %s, length is %d, should be %d";
static const char *s_sysv_unimpl PERL_UNUSED_DECL
                                 = "System V %sxxx is not implemented on this machine";

static const char *s_pkg_msg = "IPC::Msg::stat";
static const char *s_pkg_sem = "IPC::Semaphore::stat";
static const char *s_pkg_shm = "IPC::SharedMem::stat";

static void *sv2addr(SV *sv)
{
  if (SvPOK(sv) && SvCUR(sv) == sizeof(void *))
  {
    return *((void **) SvPVX(sv));
  }

  croak("invalid address value");

  return 0;
}

static void assert_sv_isa(SV *sv, const char *name, const char *method)
{
  if (!sv_isa(sv, name))
  {
    croak(s_fmt_not_isa, method, name);
  }
}

static void assert_data_length(const char *name, int got, int expected)
{
  if (got != expected)
  {
    croak(s_bad_length, name, got, expected);
  }
}

#include "const-c.inc"


MODULE=IPC::SysV	PACKAGE=IPC::Msg::stat

PROTOTYPES: ENABLE

void
pack(obj)
    SV	* obj
PPCODE:
  {
#ifdef HAS_MSG
    AV *list = (AV*) SvRV(obj);
    struct msqid_ds ds;
    assert_sv_isa(obj, s_pkg_msg, "pack");
    AV_FETCH_IV(ds.msg_perm.uid , list,  0);
    AV_FETCH_IV(ds.msg_perm.gid , list,  1);
    AV_FETCH_IV(ds.msg_perm.cuid, list,  2);
    AV_FETCH_IV(ds.msg_perm.cgid, list,  3);
    AV_FETCH_IV(ds.msg_perm.mode, list,  4);
    AV_FETCH_IV(ds.msg_qnum     , list,  5);
    AV_FETCH_IV(ds.msg_qbytes   , list,  6);
    AV_FETCH_IV(ds.msg_lspid    , list,  7);
    AV_FETCH_IV(ds.msg_lrpid    , list,  8);
    AV_FETCH_IV(ds.msg_stime    , list,  9);
    AV_FETCH_IV(ds.msg_rtime    , list, 10);
    AV_FETCH_IV(ds.msg_ctime    , list, 11);
    ST(0) = sv_2mortal(newSVpvn((char *) &ds, sizeof(ds)));
    XSRETURN(1);
#else
    croak(s_sysv_unimpl, "msg");
#endif
  }

void
unpack(obj, ds)
    SV * obj
    SV * ds
PPCODE:
  {
#ifdef HAS_MSG
    AV *list = (AV*) SvRV(obj);
    STRLEN len;
    const struct msqid_ds *data = (struct msqid_ds *) SvPV_const(ds, len);
    assert_sv_isa(obj, s_pkg_msg, "unpack");
    assert_data_length(s_pkg_msg, len, sizeof(*data));
    AV_STORE_IV(data->msg_perm.uid , list,  0);
    AV_STORE_IV(data->msg_perm.gid , list,  1);
    AV_STORE_IV(data->msg_perm.cuid, list,  2);
    AV_STORE_IV(data->msg_perm.cgid, list,  3);
    AV_STORE_IV(data->msg_perm.mode, list,  4);
    AV_STORE_IV(data->msg_qnum     , list,  5);
    AV_STORE_IV(data->msg_qbytes   , list,  6);
    AV_STORE_IV(data->msg_lspid    , list,  7);
    AV_STORE_IV(data->msg_lrpid    , list,  8);
    AV_STORE_IV(data->msg_stime    , list,  9);
    AV_STORE_IV(data->msg_rtime    , list, 10);
    AV_STORE_IV(data->msg_ctime    , list, 11);
    XSRETURN(1);
#else
    croak(s_sysv_unimpl, "msg");
#endif
  }


MODULE=IPC::SysV	PACKAGE=IPC::Semaphore::stat

PROTOTYPES: ENABLE

void
pack(obj)
    SV	* obj
PPCODE:
  {
#ifdef HAS_SEM
    AV *list = (AV*) SvRV(obj);
    struct semid_ds ds;
    assert_sv_isa(obj, s_pkg_sem, "pack");
    AV_FETCH_IV(ds.sem_perm.uid , list, 0);
    AV_FETCH_IV(ds.sem_perm.gid , list, 1);
    AV_FETCH_IV(ds.sem_perm.cuid, list, 2);
    AV_FETCH_IV(ds.sem_perm.cgid, list, 3);
    AV_FETCH_IV(ds.sem_perm.mode, list, 4);
    AV_FETCH_IV(ds.sem_ctime    , list, 5);
    AV_FETCH_IV(ds.sem_otime    , list, 6);
    AV_FETCH_IV(ds.sem_nsems    , list, 7);
    ST(0) = sv_2mortal(newSVpvn((char *) &ds, sizeof(ds)));
    XSRETURN(1);
#else
    croak(s_sysv_unimpl, "sem");
#endif
  }

void
unpack(obj, ds)
    SV * obj
    SV * ds
PPCODE:
  {
#ifdef HAS_SEM
    AV *list = (AV*) SvRV(obj);
    STRLEN len;
    const struct semid_ds *data = (struct semid_ds *) SvPV_const(ds, len);
    assert_sv_isa(obj, s_pkg_sem, "unpack");
    assert_data_length(s_pkg_sem, len, sizeof(*data));
    AV_STORE_IV(data->sem_perm.uid , list, 0);
    AV_STORE_IV(data->sem_perm.gid , list, 1);
    AV_STORE_IV(data->sem_perm.cuid, list, 2);
    AV_STORE_IV(data->sem_perm.cgid, list, 3);
    AV_STORE_IV(data->sem_perm.mode, list, 4);
    AV_STORE_IV(data->sem_ctime    , list, 5);
    AV_STORE_IV(data->sem_otime    , list, 6);
    AV_STORE_IV(data->sem_nsems    , list, 7);
    XSRETURN(1);
#else
    croak(s_sysv_unimpl, "sem");
#endif
  }


MODULE=IPC::SysV	PACKAGE=IPC::SharedMem::stat

PROTOTYPES: ENABLE

void
pack(obj)
    SV	* obj
PPCODE:
  {
#ifdef HAS_SHM
    AV *list = (AV*) SvRV(obj);
    struct shmid_ds ds;
    assert_sv_isa(obj, s_pkg_shm, "pack");
    AV_FETCH_IV(ds.shm_perm.uid , list,  0);
    AV_FETCH_IV(ds.shm_perm.gid , list,  1);
    AV_FETCH_IV(ds.shm_perm.cuid, list,  2);
    AV_FETCH_IV(ds.shm_perm.cgid, list,  3);
    AV_FETCH_IV(ds.shm_perm.mode, list,  4);
    AV_FETCH_IV(ds.shm_segsz    , list,  5);
    AV_FETCH_IV(ds.shm_lpid     , list,  6);
    AV_FETCH_IV(ds.shm_cpid     , list,  7);
    AV_FETCH_IV(ds.shm_nattch   , list,  8);
    AV_FETCH_IV(ds.shm_atime    , list,  9);
    AV_FETCH_IV(ds.shm_dtime    , list, 10);
    AV_FETCH_IV(ds.shm_ctime    , list, 11);
    ST(0) = sv_2mortal(newSVpvn((char *) &ds, sizeof(ds)));
    XSRETURN(1);
#else
    croak(s_sysv_unimpl, "shm");
#endif
  }

void
unpack(obj, ds)
    SV * obj
    SV * ds
PPCODE:
  {
#ifdef HAS_SHM
    AV *list = (AV*) SvRV(obj);
    STRLEN len;
    const struct shmid_ds *data = (struct shmid_ds *) SvPV_const(ds, len);
    assert_sv_isa(obj, s_pkg_shm, "unpack");
    assert_data_length(s_pkg_shm, len, sizeof(*data));
    AV_STORE_IV(data->shm_perm.uid , list,  0);
    AV_STORE_IV(data->shm_perm.gid , list,  1);
    AV_STORE_IV(data->shm_perm.cuid, list,  2);
    AV_STORE_IV(data->shm_perm.cgid, list,  3);
    AV_STORE_IV(data->shm_perm.mode, list,  4);
    AV_STORE_IV(data->shm_segsz    , list,  5);
    AV_STORE_IV(data->shm_lpid     , list,  6);
    AV_STORE_IV(data->shm_cpid     , list,  7);
    AV_STORE_IV(data->shm_nattch   , list,  8);
    AV_STORE_IV(data->shm_atime    , list,  9);
    AV_STORE_IV(data->shm_dtime    , list, 10);
    AV_STORE_IV(data->shm_ctime    , list, 11);
    XSRETURN(1);
#else
    croak(s_sysv_unimpl, "shm");
#endif
  }


MODULE=IPC::SysV	PACKAGE=IPC::SysV

PROTOTYPES: ENABLE

void
ftok(path, id = &PL_sv_undef)
    const char *path
    SV *id
  PREINIT:
    int proj_id = 1;
    key_t k;
  CODE:
#if defined(HAS_SEM) || defined(HAS_SHM)
    if (SvOK(id))
    {
      if (SvIOK(id))
      {
        proj_id = (int) SvIVX(id);
      }
      else if (SvPOK(id) && SvCUR(id) == sizeof(char))
      {
        proj_id = (int) *SvPVX(id);
      }
      else
      {
        croak("invalid project id");
      }
    }
/* Including <sys/types.h> before <sys/ipc.h> makes Tru64
 * to see the obsolete prototype of ftok() first, grumble. */
# ifdef __osf__
#  define Ftok_t char*
/* Configure TODO Ftok_t */
# endif 
# ifndef Ftok_t
#  define Ftok_t const char*
# endif
    k = ftok((Ftok_t)path, proj_id);
    ST(0) = k == (key_t) -1 ? &PL_sv_undef : sv_2mortal(newSViv(k));
    XSRETURN(1);
#else
    Perl_die(aTHX_ PL_no_func, "ftok"); return;
#endif

void
memread(addr, sv, pos, size)
    SV *addr
    SV *sv
    UV pos
    UV size
  CODE:
    char *caddr = (char *) sv2addr(addr);
    char *dst;
    if (!SvOK(sv))
    {
      sv_setpvn(sv, "", 0);
    }
    SvPV_force_nolen(sv);
    dst = SvGROW(sv, (STRLEN) size + 1);
    Copy(caddr + pos, dst, size, char);
    SvCUR_set(sv, size);
    *SvEND(sv) = '\0';
    SvSETMAGIC(sv);
#ifndef INCOMPLETE_TAINTS
    /* who knows who has been playing with this memory? */
    SvTAINTED_on(sv);
#endif
    XSRETURN_YES;

void
memwrite(addr, sv, pos, size)
    SV *addr
    SV *sv
    UV pos
    UV size
  CODE:
    char *caddr = (char *) sv2addr(addr);
    STRLEN len;
    const char *src = SvPV_const(sv, len);
    unsigned int n = ((unsigned int) len > size) ? size : (unsigned int) len;
    Copy(src, caddr + pos, n, char);
    if (n < size)
    {
      memzero(caddr + pos + n, size - n);
    }
    XSRETURN_YES;

void
shmat(id, addr, flag)
    int id
    SV *addr
    int flag
  CODE:
#ifdef HAS_SHM
    if (id >= 0) {
      void *caddr = SvOK(addr) ? sv2addr(addr) : NULL;
      void *shm = (void *) shmat(id, caddr, flag);
      ST(0) = shm == (void *) -1 ? &PL_sv_undef
                                 : sv_2mortal(newSVpvn((char *) &shm, sizeof(void *)));
    } else {
      SETERRNO(EINVAL,LIB_INVARG);
      ST(0) = &PL_sv_undef;
    }
    XSRETURN(1);
#else
    Perl_die(aTHX_ PL_no_func, "shmat"); return;
#endif

void
shmdt(addr)
    SV *addr
  CODE:
#ifdef HAS_SHM
    void *caddr = sv2addr(addr);
    int rv = shmdt((Shmat_t)caddr);
    ST(0) = rv == -1 ? &PL_sv_undef : sv_2mortal(newSViv(rv));
    XSRETURN(1);
#else
    Perl_die(aTHX_ PL_no_func, "shmdt"); return;
#endif

INCLUDE: const-xs.inc

