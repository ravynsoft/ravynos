/*
 * ex: set ts=8 sts=4 sw=4 et:
 */

#define PERL_NO_GET_CONTEXT

#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"
#define NEED_croak_xs_usage
#define NEED_sv_2pv_flags
#define NEED_my_strlcpy
#define NEED_my_strlcat
#include "ppport.h"

#if defined(HAS_READLINK) && !defined(PerlLIO_readlink)
#define PerlLIO_readlink readlink
#endif

#ifdef I_UNISTD
#   include <unistd.h>
#endif

/* For special handling of os390 sysplexed systems */
#ifdef OS390
#define SYSNAME "$SYSNAME"
#define SYSNAME_LEN (sizeof(SYSNAME) - 1)
#endif

/* The realpath() implementation from OpenBSD 3.9 to 4.2 (realpath.c 1.13)
 * Renamed here to bsd_realpath() to avoid library conflicts.
 */

/* See
 * http://www.xray.mpe.mpg.de/mailing-lists/perl5-porters/2004-11/msg00979.html
 * for the details of why the BSD license is compatible with the
 * AL/GPL standard perl license.
 */

/*
 * Copyright (c) 2003 Constantin S. Svintsoff <kostik@iclub.nsu.ru>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The names of the authors may not be used to endorse or promote
 *    products derived from this software without specific prior written
 *    permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/* OpenBSD system #includes removed since the Perl ones should do. --jhi */

#ifndef MAXSYMLINKS
#define MAXSYMLINKS 8
#endif

#ifndef VMS
/*
 * char *realpath(const char *path, char resolved[MAXPATHLEN]);
 *
 * Find the real name of path, by removing all ".", ".." and symlink
 * components.  Returns (resolved) on success, or (NULL) on failure,
 * in which case the path which caused trouble is left in (resolved).
 */
static
char *
bsd_realpath(const char *path, char resolved[MAXPATHLEN])
{
	char *p, *q, *s;
	size_t remaining_len, resolved_len;
	unsigned symlinks;
	int serrno;
	char remaining[MAXPATHLEN], next_token[MAXPATHLEN];
#ifdef PERL_IMPLICIT_SYS
        dTHX;
#endif

	serrno = errno;
	symlinks = 0;
	if (path[0] == '/') {
            resolved[0] = '/';
            resolved[1] = '\0';
            if (path[1] == '\0')
                    return (resolved);
            resolved_len = 1;
            remaining_len = my_strlcpy(remaining, path + 1, sizeof(remaining));
	} else {
            if (getcwd(resolved, MAXPATHLEN) == NULL) {
                my_strlcpy(resolved, ".", MAXPATHLEN);
                return (NULL);
            }
            resolved_len = strlen(resolved);
            remaining_len = my_strlcpy(remaining, path, sizeof(remaining));
	}
	if (remaining_len >= sizeof(remaining) || resolved_len >= MAXPATHLEN) {
            errno = ENAMETOOLONG;
            return (NULL);
	}

	/*
	 * Iterate over path components in 'remaining'.
	 */
	while (remaining_len != 0) {

            /*
             * Extract the next path component and adjust 'remaining'
             * and its length.
             */

            p = strchr(remaining, '/');
            s = p ? p : remaining + remaining_len;

            if ((STRLEN)(s - remaining) >= (STRLEN)sizeof(next_token)) {
                errno = ENAMETOOLONG;
                return (NULL);
            }
            memcpy(next_token, remaining, s - remaining);
            next_token[s - remaining] = '\0';

            /* shift first component off front of path, including '/' */
            if (p) {
                s++; /* skip '/' */
                remaining_len -= s - remaining;
                /* the +1 includes the trailing '\0' */
                memmove(remaining, s, remaining_len + 1);
            }
            else
                remaining_len = 0;

            if (resolved[resolved_len - 1] != '/') {
                if (resolved_len + 1 >= MAXPATHLEN) {
                    errno = ENAMETOOLONG;
                    return (NULL);
                }
                resolved[resolved_len++] = '/';
                resolved[resolved_len] = '\0';
            }
            if (next_token[0] == '\0')
                continue;
            else if (strEQ(next_token, "."))
                continue;
            else if (strEQ(next_token, "..")) {
                /*
                 * Strip the last path component except when we have
                 * single "/"
                 */
                if (resolved_len > 1) {
                    resolved[resolved_len - 1] = '\0';
                    q = strrchr(resolved, '/') + 1;
                    *q = '\0';
                    resolved_len = q - resolved;
                }
                continue;
            }

            /*
             * Append the next path component and lstat() it. If
             * lstat() fails we still can return successfully if
             * there are no more path components left.
             */
            resolved_len = my_strlcat(resolved, next_token, MAXPATHLEN);
            if (resolved_len >= MAXPATHLEN) {
                errno = ENAMETOOLONG;
                return (NULL);
            }
#if defined(HAS_LSTAT) && defined(HAS_READLINK) && defined(HAS_SYMLINK)
            {
                Stat_t sb;
                if (PerlLIO_lstat(resolved, &sb) != 0) {
                    if (errno == ENOENT && p == NULL) {
                        errno = serrno;
                        return (resolved);
                    }
                    return (NULL);
                }
                if (S_ISLNK(sb.st_mode)) {
                    int slen;
                    char symlink[MAXPATHLEN];

                    if (symlinks++ > MAXSYMLINKS) {
                        errno = ELOOP;
                        return (NULL);
                    }
                    slen = PerlLIO_readlink(resolved, symlink, sizeof(symlink) - 1);
                    if (slen < 0)
                        return (NULL);
                    symlink[slen] = '\0';
#  ifdef OS390
                    /* Replace all instances of $SYSNAME/foo simply by /foo */
                    if (slen > SYSNAME_LEN + strlen(next_token)
                        && strnEQ(symlink, SYSNAME, SYSNAME_LEN)
                        && *(symlink + SYSNAME_LEN) == '/'
                        && strEQ(symlink + SYSNAME_LEN + 1, next_token))
                    {
                        goto not_symlink;
                    }
#  endif
                    if (symlink[0] == '/') {
                        resolved[1] = 0;
                        resolved_len = 1;
                    } else if (resolved_len > 1) {
                        /* Strip the last path component. */
                        resolved[resolved_len - 1] = '\0';
                        q = strrchr(resolved, '/') + 1;
                        *q = '\0';
                        resolved_len = q - resolved;
                    }

                    /*
                     * If there are any path components left, then
                     * append them to symlink. The result is placed
                     * in 'remaining'.
                     */
                    if (p != NULL) {
                        if (symlink[slen - 1] != '/') {
                            if ((STRLEN)(slen + 1) >= (STRLEN)sizeof(symlink)) {
                                errno = ENAMETOOLONG;
                                return (NULL);
                            }
                            symlink[slen] = '/';
                            symlink[slen + 1] = 0;
                        }
                        remaining_len = my_strlcat(symlink, remaining, sizeof(symlink));
                        if (remaining_len >= sizeof(remaining)) {
                            errno = ENAMETOOLONG;
                            return (NULL);
                        }
                    }
                    remaining_len = my_strlcpy(remaining, symlink, sizeof(remaining));
                }
#  ifdef OS390
              not_symlink: ;
#  endif
            }
#endif
	}

	/*
	 * Remove trailing slash except when the resolved pathname
	 * is a single "/".
	 */
	if (resolved_len > 1 && resolved[resolved_len - 1] == '/')
            resolved[resolved_len - 1] = '\0';
	return (resolved);
}
#endif

#ifndef SV_CWD_RETURN_UNDEF
#define SV_CWD_RETURN_UNDEF \
sv_setsv(sv, &PL_sv_undef); \
return FALSE
#endif

#ifndef OPpENTERSUB_HASTARG
#define OPpENTERSUB_HASTARG     32      /* Called from OP tree. */
#endif

#ifndef dXSTARG
#define dXSTARG SV * targ = ((PL_op->op_private & OPpENTERSUB_HASTARG) \
                             ? PAD_SV(PL_op->op_targ) : sv_newmortal())
#endif

#ifndef XSprePUSH
#define XSprePUSH (sp = PL_stack_base + ax - 1)
#endif

#ifndef SV_CWD_ISDOT
#define SV_CWD_ISDOT(dp) \
    (dp->d_name[0] == '.' && (dp->d_name[1] == '\0' || \
        (dp->d_name[1] == '.' && dp->d_name[2] == '\0')))
#endif

#ifndef getcwd_sv
/* Taken from perl 5.8's util.c */
#define getcwd_sv(a) Perl_getcwd_sv(aTHX_ a)
int Perl_getcwd_sv(pTHX_ SV *sv)
{
#ifndef PERL_MICRO

    SvTAINTED_on(sv);

#ifdef HAS_GETCWD
    {
	char buf[MAXPATHLEN];

	/* Some getcwd()s automatically allocate a buffer of the given
	 * size from the heap if they are given a NULL buffer pointer.
	 * The problem is that this behaviour is not portable. */
	if (getcwd(buf, sizeof(buf) - 1)) {
	    STRLEN len = strlen(buf);
	    sv_setpvn(sv, buf, len);
	    return TRUE;
	}
	else {
	    sv_setsv(sv, &PL_sv_undef);
	    return FALSE;
	}
    }

#else
  {
    Stat_t statbuf;
    int orig_cdev, orig_cino, cdev, cino, odev, oino, tdev, tino;
    int namelen, pathlen=0;
    DIR *dir;
    Direntry_t *dp;

    (void)SvUPGRADE(sv, SVt_PV);

    if (PerlLIO_lstat(".", &statbuf) < 0) {
	SV_CWD_RETURN_UNDEF;
    }

    orig_cdev = statbuf.st_dev;
    orig_cino = statbuf.st_ino;
    cdev = orig_cdev;
    cino = orig_cino;

    for (;;) {
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
	    Move(SvPVX(sv), SvPVX(sv) + namelen + 1, pathlen, char);
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

	if (PerlDir_chdir(SvPVX(sv)) < 0) {
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
  }
#endif

#else
    return FALSE;
#endif
}

#endif

#if defined(START_MY_CXT) && defined(MY_CXT_CLONE)
# define USE_MY_CXT 1
#else
# define USE_MY_CXT 0
#endif

#if USE_MY_CXT
# define MY_CXT_KEY "Cwd::_guts" XS_VERSION
typedef struct {
    SV *empty_string_sv, *slash_string_sv;
} my_cxt_t;
START_MY_CXT
# define dUSE_MY_CXT dMY_CXT
# define EMPTY_STRING_SV MY_CXT.empty_string_sv
# define SLASH_STRING_SV MY_CXT.slash_string_sv
# define POPULATE_MY_CXT do { \
	MY_CXT.empty_string_sv = newSVpvs(""); \
	MY_CXT.slash_string_sv = newSVpvs("/"); \
    } while(0)
#else
# define dUSE_MY_CXT dNOOP
# define EMPTY_STRING_SV sv_2mortal(newSVpvs(""))
# define SLASH_STRING_SV sv_2mortal(newSVpvs("/"))
#endif

#define invocant_is_unix(i) THX_invocant_is_unix(aTHX_ i)
static
bool
THX_invocant_is_unix(pTHX_ SV *invocant)
{
    /*
     * This is used to enable optimisations that avoid method calls
     * by knowing how they would resolve.  False negatives, disabling
     * the optimisation where it would actually behave correctly, are
     * acceptable.
     */
    return SvPOK(invocant) && SvCUR(invocant) == 16 &&
	!memcmp(SvPVX(invocant), "File::Spec::Unix", 16);
}

#define unix_canonpath(p) THX_unix_canonpath(aTHX_ p)
static
SV *
THX_unix_canonpath(pTHX_ SV *path)
{
    SV *retval;
    char const *p, *pe, *q;
    STRLEN l;
    char *o;
    STRLEN plen;
    SvGETMAGIC(path);
    if(!SvOK(path)) return &PL_sv_undef;
    p = SvPV_nomg(path, plen);
    if(plen == 0) return newSVpvs("");
    pe = p + plen;
    retval = newSV(plen);
#ifdef SvUTF8
    if(SvUTF8(path)) SvUTF8_on(retval);
#endif
    o = SvPVX(retval);
    if(DOUBLE_SLASHES_SPECIAL && p[0] == '/' && p[1] == '/' && p[2] != '/') {
	q = (const char *) memchr(p+2, '/', pe-(p+2));
	if(!q) q = pe;
	l = q - p;
	memcpy(o, p, l);
	p = q;
	o += l;
    }
    /*
     * The transformations performed here are:
     *   . squeeze multiple slashes
     *   . eliminate "." segments, except one if that's all there is
     *   . eliminate leading ".." segments
     *   . eliminate trailing slash, unless it's all there is
     */
    if(p[0] == '/') {
	*o++ = '/';
	while(1) {
	    do { p++; } while(p[0] == '/');
	    if(p[0] == '.' && p[1] == '.' && (p+2 == pe || p[2] == '/')) {
		p++;
		/* advance past second "." next time round loop */
	    } else if(p[0] == '.' && (p+1 == pe || p[1] == '/')) {
		/* advance past "." next time round loop */
	    } else {
		break;
	    }
	}
    } else if(p[0] == '.' && p[1] == '/') {
	do {
	    p++;
	    do { p++; } while(p[0] == '/');
	} while(p[0] == '.' && p[1] == '/');
	if(p == pe) *o++ = '.';
    }
    if(p == pe) goto end;
    while(1) {
	q = (const char *) memchr(p, '/', pe-p);
	if(!q) q = pe;
	l = q - p;
	memcpy(o, p, l);
	p = q;
	o += l;
	if(p == pe) goto end;
	while(1) {
	    do { p++; } while(p[0] == '/');
	    if(p == pe) goto end;
	    if(p[0] != '.') break;
	    if(p+1 == pe) goto end;
	    if(p[1] != '/') break;
	    p++;
	}
	*o++ = '/';
    }
    end: ;
    *o = 0;
    SvPOK_on(retval);
    SvCUR_set(retval, o - SvPVX(retval));
    SvTAINT(retval);
    return retval;
}

MODULE = Cwd		PACKAGE = Cwd

PROTOTYPES: DISABLE

BOOT:
#if USE_MY_CXT
{
    MY_CXT_INIT;
    POPULATE_MY_CXT;
}
#endif

#if USE_MY_CXT

void
CLONE(...)
CODE:
	PERL_UNUSED_VAR(items);
	{ MY_CXT_CLONE; POPULATE_MY_CXT; }

#endif

void
getcwd(...)
ALIAS:
    fastcwd=1
PPCODE:
{
    dXSTARG;
    /* fastcwd takes zero parameters:  */
    if (ix == 1 && items != 0)
	croak_xs_usage(cv,  "");
    getcwd_sv(TARG);
    XSprePUSH; PUSHTARG;
    SvTAINTED_on(TARG);
}

void
abs_path(pathsv=Nullsv)
    SV *pathsv
PPCODE:
{
    dXSTARG;
    char *const path = pathsv ? SvPV_nolen(pathsv) : (char *)".";
    char buf[MAXPATHLEN];

    if (
#ifdef VMS
	Perl_rmsexpand(aTHX_ path, buf, NULL, 0)
#else
	bsd_realpath(path, buf)
#endif
    ) {
	sv_setpv_mg(TARG, buf);
        SvPOK_only(TARG);
	SvTAINTED_on(TARG);
    }
    else
        sv_setsv(TARG, &PL_sv_undef);

    XSprePUSH; PUSHs(TARG);
    SvTAINTED_on(TARG);
}

#if defined(WIN32) && !defined(UNDER_CE)

void
getdcwd(...)
PROTOTYPE: ENABLE
PPCODE:
{
    dXSTARG;
    int drive;
    char *dir;

    /* Drive 0 is the current drive, 1 is A:, 2 is B:, 3 is C: and so on. */
    if ( items == 0 ||
        (items == 1 && (!SvOK(ST(0)) || (SvPOK(ST(0)) && !SvCUR(ST(0))))))
        drive = 0;
    else if (items == 1 && SvPOK(ST(0)) && SvCUR(ST(0)) &&
             isALPHA(SvPVX(ST(0))[0]))
        drive = toUPPER(SvPVX(ST(0))[0]) - 'A' + 1;
    else
        croak("Usage: getdcwd(DRIVE)");

    New(0,dir,MAXPATHLEN,char);
    if (_getdcwd(drive, dir, MAXPATHLEN)) {
        sv_setpv_mg(TARG, dir);
        SvPOK_only(TARG);
    }
    else
        sv_setsv(TARG, &PL_sv_undef);

    Safefree(dir);

    XSprePUSH; PUSHs(TARG);
    SvTAINTED_on(TARG);
}

#endif

MODULE = Cwd		PACKAGE = File::Spec::Unix

SV *
canonpath(SV *self, SV *path = &PL_sv_undef, ...)
CODE:
    PERL_UNUSED_VAR(self);
    RETVAL = unix_canonpath(path);
OUTPUT:
    RETVAL

SV *
_fn_canonpath(SV *path = &PL_sv_undef, ...)
CODE:
    RETVAL = unix_canonpath(path);
OUTPUT:
    RETVAL

SV *
catdir(SV *self, ...)
PREINIT:
    dUSE_MY_CXT;
    SV *joined;
CODE:
    EXTEND(SP, items+1);
    ST(items) = EMPTY_STRING_SV;
    joined = sv_newmortal();
    do_join(joined, SLASH_STRING_SV, &ST(0), &ST(items));
    if(invocant_is_unix(self)) {
	RETVAL = unix_canonpath(joined);
    } else {
	ENTER;
	PUSHMARK(SP);
	EXTEND(SP, 2);
	PUSHs(self);
	PUSHs(joined);
	PUTBACK;
	call_method("canonpath", G_SCALAR);
	SPAGAIN;
	RETVAL = POPs;
	LEAVE;
	SvREFCNT_inc(RETVAL);
    }
OUTPUT:
    RETVAL

SV *
_fn_catdir(...)
PREINIT:
    dUSE_MY_CXT;
    SV *joined;
CODE:
    EXTEND(SP, items+1);
    ST(items) = EMPTY_STRING_SV;
    joined = sv_newmortal();
    do_join(joined, SLASH_STRING_SV, &ST(-1), &ST(items));
    RETVAL = unix_canonpath(joined);
OUTPUT:
    RETVAL

SV *
catfile(SV *self, ...)
PREINIT:
    dUSE_MY_CXT;
CODE:
    if(invocant_is_unix(self)) {
	if(items == 1) {
	    RETVAL = &PL_sv_undef;
	} else {
	    SV *file = unix_canonpath(ST(items-1));
	    if(items == 2) {
		RETVAL = file;
	    } else {
		SV *dir = sv_newmortal();
		sv_2mortal(file);
		ST(items-1) = EMPTY_STRING_SV;
		do_join(dir, SLASH_STRING_SV, &ST(0), &ST(items-1));
		RETVAL = unix_canonpath(dir);
		if(SvCUR(RETVAL) == 0 || SvPVX(RETVAL)[SvCUR(RETVAL)-1] != '/')
		    sv_catsv(RETVAL, SLASH_STRING_SV);
		sv_catsv(RETVAL, file);
	    }
	}
    } else {
	SV *file, *dir;
	ENTER;
	PUSHMARK(SP);
	EXTEND(SP, 2);
	PUSHs(self);
	PUSHs(items == 1 ? &PL_sv_undef : ST(items-1));
	PUTBACK;
	call_method("canonpath", G_SCALAR);
	SPAGAIN;
	file = POPs;
	LEAVE;
	if(items <= 2) {
	    RETVAL = SvREFCNT_inc(file);
	} else {
	    char const *pv;
	    STRLEN len;
	    bool need_slash;
	    SP--;
	    ENTER;
	    PUSHMARK(&ST(-1));
	    PUTBACK;
	    call_method("catdir", G_SCALAR);
	    SPAGAIN;
	    dir = POPs;
	    LEAVE;
	    pv = SvPV(dir, len);
	    need_slash = len == 0 || pv[len-1] != '/';
	    RETVAL = newSVsv(dir);
	    if(need_slash) sv_catsv(RETVAL, SLASH_STRING_SV);
	    sv_catsv(RETVAL, file);
	}
    }
OUTPUT:
    RETVAL

SV *
_fn_catfile(...)
PREINIT:
    dUSE_MY_CXT;
CODE:
    if(items == 0) {
	RETVAL = &PL_sv_undef;
    } else {
	SV *file = unix_canonpath(ST(items-1));
	if(items == 1) {
	    RETVAL = file;
	} else {
	    SV *dir = sv_newmortal();
	    sv_2mortal(file);
	    ST(items-1) = EMPTY_STRING_SV;
	    do_join(dir, SLASH_STRING_SV, &ST(-1), &ST(items-1));
	    RETVAL = unix_canonpath(dir);
	    if(SvCUR(RETVAL) == 0 || SvPVX(RETVAL)[SvCUR(RETVAL)-1] != '/')
		sv_catsv(RETVAL, SLASH_STRING_SV);
	    sv_catsv(RETVAL, file);
	}
    }
OUTPUT:
    RETVAL
