/*
 * ex: set ts=8 sts=4 sw=4 et:
 */

#define PERL_NO_GET_CONTEXT
#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#if defined(PERLIO_LAYERS) && defined(HAS_MMAP)

#include "perliol.h"
#include <sys/mman.h>

/*
 * mmap as "buffer" layer
 */

typedef struct {
    PerlIOBuf base;             /* PerlIOBuf stuff */
    Mmap_t mptr;                /* Mapped address */
    Size_t len;                 /* mapped length */
    STDCHAR *bbuf;              /* malloced buffer if map fails */
} PerlIOMmap;

static IV
PerlIOMmap_map(pTHX_ PerlIO *f)
{
    PerlIOMmap * const m = PerlIOSelf(f, PerlIOMmap);
    const IV flags = PerlIOBase(f)->flags;
    IV code = 0;
    if (m->len)
	abort();
    if (flags & PERLIO_F_CANREAD) {
	PerlIOBuf * const b = PerlIOSelf(f, PerlIOBuf);
	Stat_t st;
	const int fd = PerlIO_fileno(f);
        if (fd < 0) {
          SETERRNO(EBADF,RMS_IFI);
          return -1;
        }
	code = Fstat(fd, &st);
	if (code == 0 && S_ISREG(st.st_mode)) {
	    SSize_t len = st.st_size - b->posn;
	    if (len > 0) {
		Off_t posn;
		if (PL_mmap_page_size <= 0)
		  Perl_croak(aTHX_ "panic: bad pagesize %" IVdf,
			     PL_mmap_page_size);
		if (b->posn < 0) {
		    /*
		     * This is a hack - should never happen - open should
		     * have set it !
		     */
		    b->posn = PerlIO_tell(PerlIONext(f));
		}
		posn = (b->posn / PL_mmap_page_size) * PL_mmap_page_size;
		len = st.st_size - posn;
		m->mptr = (Mmap_t)mmap(NULL, len, PROT_READ, MAP_SHARED, fd, posn);
		if (m->mptr && m->mptr != (Mmap_t) - 1) {
#if 0 && defined(HAS_MADVISE) && defined(MADV_SEQUENTIAL)
		    madvise(m->mptr, len, MADV_SEQUENTIAL);
#endif
#if 0 && defined(HAS_MADVISE) && defined(MADV_WILLNEED)
		    madvise(m->mptr, len, MADV_WILLNEED);
#endif
		    PerlIOBase(f)->flags =
			(flags & ~PERLIO_F_EOF) | PERLIO_F_RDBUF;
		    b->end = ((STDCHAR *) m->mptr) + len;
		    b->buf = ((STDCHAR *) m->mptr) + (b->posn - posn);
		    b->ptr = b->buf;
		    m->len = len;
		}
		else {
		    b->buf = NULL;
		}
	    }
	    else {
		PerlIOBase(f)->flags =
		    flags | PERLIO_F_EOF | PERLIO_F_RDBUF;
		b->buf = NULL;
		b->ptr = b->end = b->ptr;
		code = -1;
	    }
	}
    }
    return code;
}

static IV
PerlIOMmap_unmap(pTHX_ PerlIO *f)
{
    PerlIOMmap * const m = PerlIOSelf(f, PerlIOMmap);
    IV code = 0;
    if (m->len) {
	PerlIOBuf * const b = &m->base;
	if (b->buf) {
	    /* The munmap address argument is tricky: depending on the
	     * standard it is either "void *" or "caddr_t" (which is
	     * usually "char *" (signed or unsigned).  If we cast it
	     * to "void *", those that have it caddr_t and an uptight
	     * C++ compiler, will freak out.  But casting it as char*
	     * should work.  Maybe.  (Using Mmap_t figured out by
	     * Configure doesn't always work, apparently.) */
	    code = munmap((char*)m->mptr, m->len);
	    b->buf = NULL;
	    m->len = 0;
	    m->mptr = NULL;
	    if (PerlIO_seek(PerlIONext(f), b->posn, SEEK_SET) != 0)
		code = -1;
	}
	b->ptr = b->end = b->buf;
	PerlIOBase(f)->flags &= ~(PERLIO_F_RDBUF | PERLIO_F_WRBUF);
    }
    return code;
}

static STDCHAR *
PerlIOMmap_get_base(pTHX_ PerlIO *f)
{
    PerlIOMmap * const m = PerlIOSelf(f, PerlIOMmap);
    PerlIOBuf * const b = &m->base;
    if (b->buf && (PerlIOBase(f)->flags & PERLIO_F_RDBUF)) {
	/*
	 * Already have a readbuffer in progress
	 */
	return b->buf;
    }
    if (b->buf) {
	/*
	 * We have a write buffer or flushed PerlIOBuf read buffer
	 */
	m->bbuf = b->buf;       /* save it in case we need it again */
	b->buf = NULL;          /* Clear to trigger below */
    }
    if (!b->buf) {
	PerlIOMmap_map(aTHX_ f);        /* Try and map it */
	if (!b->buf) {
	    /*
	     * Map did not work - recover PerlIOBuf buffer if we have one
	     */
	    b->buf = m->bbuf;
	}
    }
    b->ptr = b->end = b->buf;
    if (b->buf)
	return b->buf;
    return PerlIOBuf_get_base(aTHX_ f);
}

static SSize_t
PerlIOMmap_unread(pTHX_ PerlIO *f, const void *vbuf, Size_t count)
{
    PerlIOMmap * const m = PerlIOSelf(f, PerlIOMmap);
    PerlIOBuf * const b = &m->base;
    if (PerlIOBase(f)->flags & PERLIO_F_WRBUF)
	PerlIO_flush(f);
    if (b->ptr && (b->ptr - count) >= b->buf
	&& memEQ(b->ptr - count, vbuf, count)) {
	b->ptr -= count;
	PerlIOBase(f)->flags &= ~PERLIO_F_EOF;
	return count;
    }
    if (m->len) {
	/*
	 * Loose the unwritable mapped buffer
	 */
	PerlIO_flush(f);
	/*
	 * If flush took the "buffer" see if we have one from before
	 */
	if (!b->buf && m->bbuf)
	    b->buf = m->bbuf;
	if (!b->buf) {
	    PerlIOBuf_get_base(aTHX_ f);
	    m->bbuf = b->buf;
	}
    }
    return PerlIOBuf_unread(aTHX_ f, vbuf, count);
}

static SSize_t
PerlIOMmap_write(pTHX_ PerlIO *f, const void *vbuf, Size_t count)
{
    PerlIOMmap * const m = PerlIOSelf(f, PerlIOMmap);
    PerlIOBuf * const b = &m->base;

    if (!b->buf || !(PerlIOBase(f)->flags & PERLIO_F_WRBUF)) {
	/*
	 * No, or wrong sort of, buffer
	 */
	if (m->len) {
	    if (PerlIOMmap_unmap(aTHX_ f) != 0)
		return 0;
	}
	/*
	 * If unmap took the "buffer" see if we have one from before
	 */
	if (!b->buf && m->bbuf)
	    b->buf = m->bbuf;
	if (!b->buf) {
	    PerlIOBuf_get_base(aTHX_ f);
	    m->bbuf = b->buf;
	}
    }
    return PerlIOBuf_write(aTHX_ f, vbuf, count);
}

static IV
PerlIOMmap_flush(pTHX_ PerlIO *f)
{
    PerlIOMmap * const m = PerlIOSelf(f, PerlIOMmap);
    PerlIOBuf * const b = &m->base;
    IV code = PerlIOBuf_flush(aTHX_ f);
    /*
     * Now we are "synced" at PerlIOBuf level
     */
    if (b->buf) {
	if (m->len) {
	    /*
	     * Unmap the buffer
	     */
	    if (PerlIOMmap_unmap(aTHX_ f) != 0)
		code = -1;
	}
	else {
	    /*
	     * We seem to have a PerlIOBuf buffer which was not mapped
	     * remember it in case we need one later
	     */
	    m->bbuf = b->buf;
	}
    }
    return code;
}

static IV
PerlIOMmap_fill(pTHX_ PerlIO *f)
{
    PerlIOBuf * const b = PerlIOSelf(f, PerlIOBuf);
    IV code = PerlIO_flush(f);
    if (code == 0 && !b->buf) {
	code = PerlIOMmap_map(aTHX_ f);
    }
    if (code == 0 && !(PerlIOBase(f)->flags & PERLIO_F_RDBUF)) {
	code = PerlIOBuf_fill(aTHX_ f);
    }
    return code;
}

static PerlIO *
PerlIOMmap_dup(pTHX_ PerlIO *f, PerlIO *o, CLONE_PARAMS *param, int flags)
{
 return PerlIOBase_dup(aTHX_ f, o, param, flags);
}


static PERLIO_FUNCS_DECL(PerlIO_mmap) = {
    sizeof(PerlIO_funcs),
    "mmap",
    sizeof(PerlIOMmap),
    PERLIO_K_BUFFERED|PERLIO_K_RAW,
    PerlIOBuf_pushed,
    PerlIOBuf_popped,
    PerlIOBuf_open,
    PerlIOBase_binmode,         /* binmode */
    NULL,
    PerlIOBase_fileno,
    PerlIOMmap_dup,
    PerlIOBuf_read,
    PerlIOMmap_unread,
    PerlIOMmap_write,
    PerlIOBuf_seek,
    PerlIOBuf_tell,
    PerlIOBuf_close,
    PerlIOMmap_flush,
    PerlIOMmap_fill,
    PerlIOBase_eof,
    PerlIOBase_error,
    PerlIOBase_clearerr,
    PerlIOBase_setlinebuf,
    PerlIOMmap_get_base,
    PerlIOBuf_bufsiz,
    PerlIOBuf_get_ptr,
    PerlIOBuf_get_cnt,
    PerlIOBuf_set_ptrcnt,
};

#endif /* Layers available */

MODULE = PerlIO::mmap	PACKAGE = PerlIO::mmap

PROTOTYPES: DISABLE

BOOT:
{
#if defined(PERLIO_LAYERS) && defined(HAS_MMAP)
    PerlIO_define_layer(aTHX_ PERLIO_FUNCS_CAST(&PerlIO_mmap));
#endif
}

