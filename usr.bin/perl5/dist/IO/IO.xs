/*
 * Copyright (c) 1997-8 Graham Barr <gbarr@pobox.com>. All rights reserved.
 * This program is free software; you can redistribute it and/or
 * modify it under the same terms as Perl itself.
 */

#define PERL_EXT_IO

#define PERL_NO_GET_CONTEXT
#include "EXTERN.h"
#define PERLIO_NOT_STDIO 1
#include "perl.h"
#include "XSUB.h"
#define NEED_newCONSTSUB
#define NEED_newSVpvn_flags
#include "ppport.h"
#include "poll.h"
#ifdef I_UNISTD
#  include <unistd.h>
#endif
#if defined(I_FCNTL) || defined(HAS_FCNTL)
#  include <fcntl.h>
#endif

#ifndef SIOCATMARK
#   ifdef I_SYS_SOCKIO
#       include <sys/sockio.h>
#   endif
#endif

#ifdef PerlIO
#if defined(MACOS_TRADITIONAL) && defined(USE_SFIO)
#define PERLIO_IS_STDIO 1
#undef setbuf
#undef setvbuf
#define setvbuf		_stdsetvbuf
#define setbuf(f,b)	( __sf_setbuf(f,b) )
#endif
typedef int SysRet;
typedef PerlIO * InputStream;
typedef PerlIO * OutputStream;
#else
#define PERLIO_IS_STDIO 1
typedef int SysRet;
typedef FILE * InputStream;
typedef FILE * OutputStream;
#endif

#define MY_start_subparse(fmt,flags) start_subparse(fmt,flags)

#ifndef __attribute__noreturn__
#  define __attribute__noreturn__
#endif

#ifndef NORETURN_FUNCTION_END
# define NORETURN_FUNCTION_END /* NOT REACHED */ return 0
#endif

#ifndef OpSIBLING
#  define OpSIBLING(o) (o)->op_sibling
#endif

static int not_here(const char *s) __attribute__noreturn__;
static int
not_here(const char *s)
{
    croak("%s not implemented on this architecture", s);
    NORETURN_FUNCTION_END;
}

#ifndef PerlIO
#define PerlIO_fileno(f) fileno(f)
#endif

static int
io_blocking(pTHX_ InputStream f, int block)
{
    int fd = -1;
#if defined(HAS_FCNTL)
    int RETVAL;
    if (!f) {
	errno = EBADF;
	return -1;
    }
    fd = PerlIO_fileno(f);
    if (fd < 0) {
      errno = EBADF;
      return -1;
    }
    RETVAL = fcntl(fd, F_GETFL, 0);
    if (RETVAL >= 0) {
	int mode = RETVAL;
	int newmode = mode;
#ifdef O_NONBLOCK
	/* POSIX style */

# ifndef O_NDELAY
#  define O_NDELAY O_NONBLOCK
# endif
	/* Note: UNICOS and UNICOS/mk a F_GETFL returns an O_NDELAY
	 * after a successful F_SETFL of an O_NONBLOCK. */
	RETVAL = RETVAL & (O_NONBLOCK | O_NDELAY) ? 0 : 1;

	if (block == 0) {
	    newmode &= ~O_NDELAY;
	    newmode |= O_NONBLOCK;
	} else if (block > 0) {
	    newmode &= ~(O_NDELAY|O_NONBLOCK);
	}
#else
	/* Not POSIX - better have O_NDELAY or we can't cope.
	 * for BSD-ish machines this is an acceptable alternative
	 * for SysV we can't tell "would block" from EOF but that is
	 * the way SysV is...
	 */
	RETVAL = RETVAL & O_NDELAY ? 0 : 1;

	if (block == 0) {
	    newmode |= O_NDELAY;
	} else if (block > 0) {
	    newmode &= ~O_NDELAY;
	}
#endif
	if (newmode != mode) {
            const int ret = fcntl(fd, F_SETFL, newmode);
	    if (ret < 0)
		RETVAL = ret;
	}
    }
    return RETVAL;
#else
#   ifdef WIN32
    if (block >= 0) {
	unsigned long flags = !block;
	/* ioctl claims to take char* but really needs a u_long sized buffer */
	const int ret = ioctl(fd, FIONBIO, (char*)&flags);
	if (ret != 0)
	    return -1;
	/* Win32 has no way to get the current blocking status of a socket.
	 * However, we don't want to just return undef, because there's no way
	 * to tell that the ioctl succeeded.
	 */
	return flags;
    }
    /* TODO: Perhaps set $! to ENOTSUP? */
    return -1;
#   else
    return -1;
#   endif
#endif
}


MODULE = IO	PACKAGE = IO::Seekable	PREFIX = f

void
fgetpos(handle)
	InputStream	handle
    CODE:
	if (handle) {
#ifdef PerlIO
#if PERL_VERSION_LT(5,8,0)
	    Fpos_t pos;
	    ST(0) = sv_newmortal();
	    if (PerlIO_getpos(handle, &pos) != 0) {
		ST(0) = &PL_sv_undef;
	    }
	    else {
		sv_setpvn(ST(0), (char *)&pos, sizeof(Fpos_t));
	    }
#else
	    ST(0) = sv_newmortal();
	    if (PerlIO_getpos(handle, ST(0)) != 0) {
		ST(0) = &PL_sv_undef;
	    }
#endif
#else
	    Fpos_t pos;
	    if (fgetpos(handle, &pos)) {
		ST(0) = &PL_sv_undef;
	    } else {
#  if PERL_VERSION_GE(5,11,0)
		ST(0) = newSVpvn_flags((char*)&pos, sizeof(Fpos_t), SVs_TEMP);
#  else
		ST(0) = sv_2mortal(newSVpvn((char*)&pos, sizeof(Fpos_t)));
#  endif
	    }
#endif
	}
	else {
	    errno = EINVAL;
	    ST(0) = &PL_sv_undef;
	}

SysRet
fsetpos(handle, pos)
	InputStream	handle
	SV *		pos
    CODE:
	if (handle) {
#ifdef PerlIO
#if PERL_VERSION_LT(5,8,0)
	    char *p;
	    STRLEN len;
	    if (SvOK(pos) && (p = SvPV(pos,len)) && len == sizeof(Fpos_t)) {
		RETVAL = PerlIO_setpos(handle, (Fpos_t*)p);
	    }
	    else {
		RETVAL = -1;
		errno = EINVAL;
	    }
#else
	    RETVAL = PerlIO_setpos(handle, pos);
#endif
#else
	    char *p;
	    STRLEN len;
	    if ((p = SvPV(pos,len)) && len == sizeof(Fpos_t)) {
		RETVAL = fsetpos(handle, (Fpos_t*)p);
	    }
	    else {
		RETVAL = -1;
		errno = EINVAL;
	    }
#endif
	}
	else {
	    RETVAL = -1;
	    errno = EINVAL;
	}
    OUTPUT:
	RETVAL

MODULE = IO	PACKAGE = IO::File	PREFIX = f

void
new_tmpfile(packname = "IO::File")
    const char * packname
    PREINIT:
	OutputStream fp;
	GV *gv;
    CODE:
#ifdef PerlIO
	fp = PerlIO_tmpfile();
#else
	fp = tmpfile();
#endif
	gv = (GV*)SvREFCNT_inc(newGVgen(packname));
	if (gv)
	    (void) hv_delete(GvSTASH(gv), GvNAME(gv), GvNAMELEN(gv), G_DISCARD);
	if (gv && do_open(gv, "+>&", 3, FALSE, 0, 0, fp)) {
	    ST(0) = sv_2mortal(newRV_inc((SV*)gv));
	    sv_bless(ST(0), gv_stashpv(packname, TRUE));
	    SvREFCNT_dec(gv);   /* undo increment in newRV() */
	}
	else {
	    ST(0) = &PL_sv_undef;
	    SvREFCNT_dec(gv);
	}

MODULE = IO	PACKAGE = IO::Poll

void
_poll(timeout,...)
	int timeout;
PPCODE:
{
#ifdef HAS_POLL
    const int nfd = (items - 1) / 2;
    SV *tmpsv = sv_2mortal(NEWSV(999,nfd * sizeof(struct pollfd)));
    /* We should pass _some_ valid pointer even if nfd is zero, but it
     * doesn't matter what it is, since we're telling it to not check any fds.
     */
    struct pollfd *fds = nfd ? (struct pollfd *)SvPVX(tmpsv) : (struct pollfd *)tmpsv;
    int i,j,ret;
    for(i=1, j=0  ; j < nfd ; j++) {
	fds[j].fd = SvIV(ST(i));
	i++;
	fds[j].events = (short)SvIV(ST(i));
	i++;
	fds[j].revents = 0;
    }
    if((ret = poll(fds,nfd,timeout)) >= 0) {
	for(i=1, j=0 ; j < nfd ; j++) {
	    sv_setiv(ST(i), fds[j].fd); i++;
	    sv_setiv(ST(i), fds[j].revents); i++;
	}
    }
    XSRETURN_IV(ret);
#else
	not_here("IO::Poll::poll");
#endif
}

MODULE = IO	PACKAGE = IO::Handle	PREFIX = io_

void
io_blocking(handle,blk=-1)
	InputStream	handle
	int		blk
PROTOTYPE: $;$
CODE:
{
    const int ret = io_blocking(aTHX_ handle, items == 1 ? -1 : blk ? 1 : 0);
    if(ret >= 0)
	XSRETURN_IV(ret);
    else
	XSRETURN_UNDEF;
}

MODULE = IO	PACKAGE = IO::Handle	PREFIX = f

int
ungetc(handle, c)
	InputStream	handle
	SV *	        c
    CODE:
	if (handle) {
#ifdef PerlIO
            UV v;

            if ((SvIOK_notUV(c) && SvIV(c) < 0) || (SvNOK(c) && SvNV(c) < 0.0))
                croak("Negative character number in ungetc()");

            v = SvUV(c);
            if (UVCHR_IS_INVARIANT(v) || (v <= 0xFF && !PerlIO_isutf8(handle)))
                RETVAL = PerlIO_ungetc(handle, (int)v);
            else {
                U8 buf[UTF8_MAXBYTES + 1], *end;
                Size_t len;

                if (!PerlIO_isutf8(handle))
                    croak("Wide character number in ungetc()");

                /* This doesn't warn for non-chars, surrogate, and
                 * above-Unicodes */
                end = uvchr_to_utf8_flags(buf, v, 0);
                len = end - buf;
                if ((Size_t)PerlIO_unread(handle, &buf, len) == len)
                    XSRETURN_UV(v);
                else
                    RETVAL = EOF;
            }
#else
            RETVAL = ungetc((int)SvIV(c), handle);
#endif
        }
	else {
	    RETVAL = -1;
	    errno = EINVAL;
	}
    OUTPUT:
	RETVAL

int
ferror(handle)
	SV *	handle
    PREINIT:
        IO *io = sv_2io(handle);
        InputStream in = IoIFP(io);
        OutputStream out = IoOFP(io);
    CODE:
	if (in)
#ifdef PerlIO
	    RETVAL = PerlIO_error(in) || (out && in != out && PerlIO_error(out));
#else
	    RETVAL = ferror(in) || (out && in != out && ferror(out));
#endif
	else {
	    RETVAL = -1;
	    errno = EINVAL;
	}
    OUTPUT:
	RETVAL

int
clearerr(handle)
	SV *	handle
    PREINIT:
        IO *io = sv_2io(handle);
        InputStream in = IoIFP(io);
        OutputStream out = IoOFP(io);
    CODE:
	if (handle) {
#ifdef PerlIO
	    PerlIO_clearerr(in);
            if (in != out)
                PerlIO_clearerr(out);
#else
	    clearerr(in);
            if (in != out)
                clearerr(out);
#endif
	    RETVAL = 0;
	}
	else {
	    RETVAL = -1;
	    errno = EINVAL;
	}
    OUTPUT:
	RETVAL

int
untaint(handle)
       SV *	handle
    CODE:
#ifdef IOf_UNTAINT
	IO * io;
	io = sv_2io(handle);
	if (io) {
	    IoFLAGS(io) |= IOf_UNTAINT;
	    RETVAL = 0;
	}
        else {
#endif
	    RETVAL = -1;
	    errno = EINVAL;
#ifdef IOf_UNTAINT
	}
#endif
    OUTPUT:
	RETVAL

SysRet
fflush(handle)
	OutputStream	handle
    CODE:
	if (handle)
#ifdef PerlIO
	    RETVAL = PerlIO_flush(handle);
#else
	    RETVAL = Fflush(handle);
#endif
	else {
	    RETVAL = -1;
	    errno = EINVAL;
	}
    OUTPUT:
	RETVAL

void
setbuf(handle, ...)
	OutputStream	handle
    CODE:
	if (handle)
#ifdef PERLIO_IS_STDIO
        {
	    char *buf = items == 2 && SvPOK(ST(1)) ?
	      sv_grow(ST(1), BUFSIZ) : 0;
	    setbuf(handle, buf);
	}
#else
	    not_here("IO::Handle::setbuf");
#endif

SysRet
setvbuf(...)
    CODE:
	if (items != 4)
            Perl_croak(aTHX_ "Usage: IO::Handle::setvbuf(handle, buf, type, size)");
#if defined(PERLIO_IS_STDIO) && defined(_IOFBF) && defined(HAS_SETVBUF)
    {
        OutputStream	handle = 0;
	char *		buf = SvPOK(ST(1)) ? sv_grow(ST(1), SvIV(ST(3))) : 0;
	int		type;
	int		size;

	if (items == 4) {
	    handle = IoOFP(sv_2io(ST(0)));
	    buf    = SvPOK(ST(1)) ? sv_grow(ST(1), SvIV(ST(3))) : 0;
	    type   = (int)SvIV(ST(2));
	    size   = (int)SvIV(ST(3));
	}
	if (!handle)			/* Try input stream. */
	    handle = IoIFP(sv_2io(ST(0)));
	if (items == 4 && handle)
	    RETVAL = setvbuf(handle, buf, type, size);
	else {
	    RETVAL = -1;
	    errno = EINVAL;
	}
    }
#else
	RETVAL = (SysRet) not_here("IO::Handle::setvbuf");
#endif
    OUTPUT:
	RETVAL


SysRet
fsync(arg)
	SV * arg
    PREINIT:
	OutputStream handle = NULL;
    CODE:
#if defined(HAS_FSYNC) || defined(_WIN32)
	handle = IoOFP(sv_2io(arg));
	if (!handle)
	    handle = IoIFP(sv_2io(arg));
	if (handle) {
	    int fd = PerlIO_fileno(handle);
	    if (fd >= 0) {
#  ifdef _WIN32
                RETVAL = _commit(fd);
#  else
		RETVAL = fsync(fd);
#  endif
	    } else {
		RETVAL = -1;
		errno = EBADF;
	    }
	} else {
	    RETVAL = -1;
	    errno = EINVAL;
	}
#else
	RETVAL = (SysRet) not_here("IO::Handle::sync");
#endif
    OUTPUT:
	RETVAL

# To make these two work correctly with the open pragma, the readline op
# needs to pick up the lexical hints at the method's callsite. This doesn't
# work in pure Perl, because the hints are read from the most recent nextstate,
# and the nextstate of the Perl subroutines show *here* hold the lexical state
# for the IO package.
#
# There's no clean way to implement this - this approach, while complex, seems
# to be the most robust, and avoids manipulating external state (ie op checkers)
#
# sub getline {
#     @_ == 1 or croak 'usage: $io->getline()';
#     my $this = shift;
#     return scalar <$this>;
# }
#
# sub getlines {
#     @_ == 1 or croak 'usage: $io->getlines()';
#     wantarray or
# 	croak 'Can\'t call $io->getlines in a scalar context, use $io->getline';
#     my $this = shift;
#     return <$this>;
# }

# If this is deprecated, should it warn, and should it be removed at some point?
# *gets = \&getline;  # deprecated

void
getlines(...)
ALIAS:
    IO::Handle::getline       =  1
    IO::Handle::gets          =  2
INIT:
    UNOP myop;
    SV *io;
    OP *was = PL_op;
PPCODE:
    if (items != 1)
        Perl_croak(aTHX_ "usage: $io->%s()", ix ? "getline" : "getlines");
    if (!ix && GIMME_V != G_LIST)
        Perl_croak(aTHX_ "Can't call $io->getlines in a scalar context, use $io->getline");
    Zero(&myop, 1, UNOP);
    myop.op_flags = (ix ? OPf_WANT_SCALAR : OPf_WANT_LIST ) | OPf_STACKED;
    myop.op_ppaddr = PL_ppaddr[OP_READLINE];
    myop.op_type = OP_READLINE;
    /* I don't know if we need this, but it's correct as far as the control flow
       goes. However, if we *do* need it, do we need to set anything else up? */
    myop.op_next = PL_op->op_next;
    /* Sigh, because pp_readline calls pp_rv2gv, and *it* has this wonderful
       state check for PL_op->op_type == OP_READLINE */
    PL_op = (OP *) &myop;
    io = ST(0);
    /* Our target (which we need to provide, as we don't have a pad entry.
       I think that this is only needed for G_SCALAR - maybe we can get away
       with NULL for list context? */
    PUSHs(sv_newmortal());
    XPUSHs(io);
    PUTBACK;
    /* And effectively we get away with tail calling pp_readline, as it stacks
       exactly the return value(s) we need to return. */
    PL_ppaddr[OP_READLINE](aTHX);
    PL_op = was;
    /* And we don't want to reach the line
       PL_stack_sp = sp;
       that xsubpp adds after our body becase PL_stack_sp is correct, not sp */
    return;

MODULE = IO	PACKAGE = IO::Socket

SysRet
sockatmark (sock)
   InputStream sock
   PROTOTYPE: $
   PREINIT:
     int fd;
   CODE:
     fd = PerlIO_fileno(sock);
     if (fd < 0) {
       errno = EBADF;
       RETVAL = -1;
     }
#ifdef HAS_SOCKATMARK
     else {
       RETVAL = sockatmark(fd);
     }
#else
     else {
       int flag = 0;
#   ifdef SIOCATMARK
#     if defined(NETWARE) || defined(WIN32)
       if (ioctl(fd, SIOCATMARK, (char*)&flag) != 0)
#     else
       if (ioctl(fd, SIOCATMARK, &flag) != 0)
#     endif
	 XSRETURN_UNDEF;
#   else
       not_here("IO::Socket::atmark");
#   endif
       RETVAL = flag;
     }
#endif
   OUTPUT:
     RETVAL

BOOT:
{
    HV *stash;
    /*
     * constant subs for IO::Poll
     */
    stash = gv_stashpvn("IO::Poll", 8, TRUE);
#ifdef	POLLIN
	newCONSTSUB(stash,"POLLIN",newSViv(POLLIN));
#endif
#ifdef	POLLPRI
        newCONSTSUB(stash,"POLLPRI", newSViv(POLLPRI));
#endif
#ifdef	POLLOUT
        newCONSTSUB(stash,"POLLOUT", newSViv(POLLOUT));
#endif
#ifdef	POLLRDNORM
        newCONSTSUB(stash,"POLLRDNORM", newSViv(POLLRDNORM));
#endif
#ifdef	POLLWRNORM
        newCONSTSUB(stash,"POLLWRNORM", newSViv(POLLWRNORM));
#endif
#ifdef	POLLRDBAND
        newCONSTSUB(stash,"POLLRDBAND", newSViv(POLLRDBAND));
#endif
#ifdef	POLLWRBAND
        newCONSTSUB(stash,"POLLWRBAND", newSViv(POLLWRBAND));
#endif
#ifdef	POLLNORM
        newCONSTSUB(stash,"POLLNORM", newSViv(POLLNORM));
#endif
#ifdef	POLLERR
        newCONSTSUB(stash,"POLLERR", newSViv(POLLERR));
#endif
#ifdef	POLLHUP
        newCONSTSUB(stash,"POLLHUP", newSViv(POLLHUP));
#endif
#ifdef	POLLNVAL
        newCONSTSUB(stash,"POLLNVAL", newSViv(POLLNVAL));
#endif
    /*
     * constant subs for IO::Handle
     */
    stash = gv_stashpvn("IO::Handle", 10, TRUE);
#ifdef _IOFBF
        newCONSTSUB(stash,"_IOFBF", newSViv(_IOFBF));
#endif
#ifdef _IOLBF
        newCONSTSUB(stash,"_IOLBF", newSViv(_IOLBF));
#endif
#ifdef _IONBF
        newCONSTSUB(stash,"_IONBF", newSViv(_IONBF));
#endif
#ifdef SEEK_SET
        newCONSTSUB(stash,"SEEK_SET", newSViv(SEEK_SET));
#endif
#ifdef SEEK_CUR
        newCONSTSUB(stash,"SEEK_CUR", newSViv(SEEK_CUR));
#endif
#ifdef SEEK_END
        newCONSTSUB(stash,"SEEK_END", newSViv(SEEK_END));
#endif
}

