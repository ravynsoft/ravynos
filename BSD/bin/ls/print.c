/*
 * Copyright (c) 1989, 1993, 1994
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Michael Fischbein.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#if 0
#ifndef lint
static char sccsid[] = "@(#)print.c	8.4 (Berkeley) 4/17/94";
#endif /* not lint */
#endif
#include <sys/cdefs.h>
__RCSID("$FreeBSD: src/bin/ls/print.c,v 1.57 2002/08/29 14:29:09 keramida Exp $");

#include <sys/param.h>
#include <sys/stat.h>
#ifdef __APPLE__
#include <sys/acl.h>
#include <sys/xattr.h>
#include <sys/types.h>
#include <grp.h>
#include <pwd.h>
#include <TargetConditionals.h>
#include <membership.h>
#include <membershipPriv.h>
#include <uuid/uuid.h>
#endif

#include <err.h>
#include <errno.h>
#include <fts.h>
#include <math.h>
#include <langinfo.h>
//#include <libutil.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#ifdef COLORLS
#include <ctype.h>
#include <termcap.h>
#include <signal.h>
#endif
#include <stdint.h>		/* intmax_t */
#include <assert.h>
#ifdef __APPLE__ 
#include <get_compat.h>
#else 
#define COMPAT_MODE(a,b) (1)
#endif /* __APPLE__ */

#include "ls.h"
#include "extern.h"

static int	printaname(FTSENT *, u_long, u_long);
static void	printlink(FTSENT *);
static void	printtime(time_t);
static int	printtype(u_int);
static void	printsize(size_t, off_t);
#ifdef COLORLS
static void	endcolor(int);
static int	colortype(mode_t);
#endif

#define	IS_NOPRINT(p)	((p)->fts_number == NO_PRINT)

#ifdef COLORLS
/* Most of these are taken from <sys/stat.h> */
typedef enum Colors {
	C_DIR,			/* directory */
	C_LNK,			/* symbolic link */
	C_SOCK,			/* socket */
	C_FIFO,			/* pipe */
	C_EXEC,			/* executable */
	C_BLK,			/* block special */
	C_CHR,			/* character special */
	C_SUID,			/* setuid executable */
	C_SGID,			/* setgid executable */
	C_WSDIR,		/* directory writeble to others, with sticky
				 * bit */
	C_WDIR,			/* directory writeble to others, without
				 * sticky bit */
	C_NUMCOLORS		/* just a place-holder */
} Colors;

static const char *defcolors = "exfxcxdxbxegedabagacad";

/* colors for file types */
static struct {
	int	num[2];
	int	bold;
} colors[C_NUMCOLORS];
#endif

void
printscol(DISPLAY *dp)
{
	FTSENT *p;

	assert(dp);
	if (COMPAT_MODE("bin/ls", "Unix2003") && (dp->list != NULL)) {
		if (dp->list->fts_level != FTS_ROOTLEVEL && (f_longform || f_size))
			(void)printf("total %qu\n", (u_int64_t)howmany(dp->btotal, blocksize));
	}

	for (p = dp->list; p; p = p->fts_link) {
		if (IS_NOPRINT(p))
			continue;
		(void)printaname(p, dp->s_inode, dp->s_block);
		(void)putchar('\n');
	}
}

/*
 * print name in current style
 */
static int
printname(const char *name)
{
	if (f_octal || f_octal_escape)
		return prn_octal(name);
	else if (f_nonprint)
		return prn_printable(name);
	else
		return prn_normal(name);
}

/*
 * print access control list
 */
static struct {
	acl_perm_t	perm;
	char		*name;
	int		flags;
#define ACL_PERM_DIR	(1<<0)
#define ACL_PERM_FILE	(1<<1)
} acl_perms[] = {
	{ACL_READ_DATA,		"read",		ACL_PERM_FILE},
	{ACL_LIST_DIRECTORY,	"list",		ACL_PERM_DIR},
	{ACL_WRITE_DATA,	"write",	ACL_PERM_FILE},
	{ACL_ADD_FILE,		"add_file",	ACL_PERM_DIR},
	{ACL_EXECUTE,		"execute",	ACL_PERM_FILE},
	{ACL_SEARCH,		"search",	ACL_PERM_DIR},
	{ACL_DELETE,		"delete",	ACL_PERM_FILE | ACL_PERM_DIR},
	{ACL_APPEND_DATA,	"append",	ACL_PERM_FILE},
	{ACL_ADD_SUBDIRECTORY,	"add_subdirectory", ACL_PERM_DIR},
	{ACL_DELETE_CHILD,	"delete_child",	ACL_PERM_DIR},
	{ACL_READ_ATTRIBUTES,	"readattr",	ACL_PERM_FILE | ACL_PERM_DIR},
	{ACL_WRITE_ATTRIBUTES,	"writeattr",	ACL_PERM_FILE | ACL_PERM_DIR},
	{ACL_READ_EXTATTRIBUTES, "readextattr",	ACL_PERM_FILE | ACL_PERM_DIR},
	{ACL_WRITE_EXTATTRIBUTES, "writeextattr", ACL_PERM_FILE | ACL_PERM_DIR},
	{ACL_READ_SECURITY,	"readsecurity",	ACL_PERM_FILE | ACL_PERM_DIR},
	{ACL_WRITE_SECURITY,	"writesecurity", ACL_PERM_FILE | ACL_PERM_DIR},
	{ACL_CHANGE_OWNER,	"chown",	ACL_PERM_FILE | ACL_PERM_DIR},
	{0, NULL, 0}
};

static struct {
	acl_flag_t	flag;
	char		*name;
	int		flags;
} acl_flags[] = {
	{ACL_ENTRY_FILE_INHERIT, 	"file_inherit",		ACL_PERM_DIR},
	{ACL_ENTRY_DIRECTORY_INHERIT,	"directory_inherit",	ACL_PERM_DIR},
	{ACL_ENTRY_LIMIT_INHERIT,	"limit_inherit",	ACL_PERM_FILE | ACL_PERM_DIR},
	{ACL_ENTRY_ONLY_INHERIT,	"only_inherit",		ACL_PERM_DIR},
	{0, NULL, 0}
};

static char *
uuid_to_name(uuid_t *uu) 
{
	int type;
	char *name = NULL;
	char *recname = NULL;
	
#define MAXNAMETAG (MAXLOGNAME + 6) /* + strlen("group:") */
	name = (char *) malloc(MAXNAMETAG);
	
	if (NULL == name) {
		err(1, "malloc");
	}
	
	if (f_numericonly) {
		goto errout;
	}
	
	if (mbr_identifier_translate(ID_TYPE_UUID, *uu, sizeof(*uu), ID_TYPE_NAME, (void **) &recname, &type)) {
		goto errout;
	}
	
	snprintf(name, MAXNAMETAG, "%s:%s", (type == MBR_REC_TYPE_USER ? "user" : "group"), recname);
	free(recname);
	
	return name;
errout:
	uuid_unparse_upper(*uu, name);
	
	return name;
}

static void
printxattr(DISPLAY *dp, int count, char *buf, int sizes[])
{
	for (int i = 0; i < count; i++) {
		putchar('\t');
		printname(buf);
		putchar('\t');
		printsize(dp->s_size, sizes[i]);
		putchar('\n');
		buf += strlen(buf) + 1;
	}
}

static void
printacl(acl_t acl, int isdir)
{
	acl_entry_t	entry = NULL;
	int		index;
	uuid_t		*applicable;
	char		*name = NULL;
	acl_tag_t	tag;
	acl_flagset_t	flags;
	acl_permset_t	perms;
	char		*type;
	int		i, first;
	

	for (index = 0;
	     acl_get_entry(acl, entry == NULL ? ACL_FIRST_ENTRY : ACL_NEXT_ENTRY, &entry) == 0;
	     index++) {
		if (acl_get_tag_type(entry, &tag) != 0)
			continue;
		if (acl_get_flagset_np(entry, &flags) != 0)
			continue;
		if (acl_get_permset(entry, &perms) != 0)
			continue;
		if ((applicable = (uuid_t *) acl_get_qualifier(entry)) == NULL)
			continue;
		name = uuid_to_name(applicable);
		acl_free(applicable);
		switch(tag) {
		case ACL_EXTENDED_ALLOW:
			type = "allow";
			break;
		case ACL_EXTENDED_DENY:
			type = "deny";
			break;
		default:
			type = "unknown";
		}

		(void)printf(" %d: %s%s %s ",
		    index,
		    name,
		    acl_get_flag_np(flags, ACL_ENTRY_INHERITED) ? " inherited" : "",
		    type);

		if (name)
			free(name);

		for (i = 0, first = 0; acl_perms[i].name != NULL; i++) {
			if (acl_get_perm_np(perms, acl_perms[i].perm) == 0)
				continue;
			if (!(acl_perms[i].flags & (isdir ? ACL_PERM_DIR : ACL_PERM_FILE)))
				continue;
			(void)printf("%s%s", first++ ? "," : "", acl_perms[i].name);
		}
		for (i = 0; acl_flags[i].name != NULL; i++) {
			if (acl_get_flag_np(flags, acl_flags[i].flag) == 0)
				continue;
			if (!(acl_flags[i].flags & (isdir ? ACL_PERM_DIR : ACL_PERM_FILE)))
				continue;
			(void)printf("%s%s", first++ ? "," : "", acl_flags[i].name);
		}
			
		(void)putchar('\n');
	}

}

void
printlong(DISPLAY *dp)
{
	struct stat *sp;
	FTSENT *p;
	NAMES *np;
	char buf[20];
#ifdef COLORLS
	int color_printed = 0;
#endif

	if (dp->list->fts_level != FTS_ROOTLEVEL && (f_longform || f_size))
		(void)printf("total %qu\n", (u_int64_t)howmany(dp->btotal, blocksize));

	for (p = dp->list; p; p = p->fts_link) {
		if (IS_NOPRINT(p))
			continue;
		sp = p->fts_statp;
		if (f_inode) 
#if _DARWIN_FEATURE_64_BIT_INODE
			(void)printf("%*llu ", dp->s_inode, (u_quad_t)sp->st_ino);
#else
			(void)printf("%*lu ", dp->s_inode, (u_long)sp->st_ino);
#endif
		if (f_size)
			(void)printf("%*qu ",
			    dp->s_block, (u_int64_t)howmany(sp->st_blocks, blocksize));
		strmode(sp->st_mode, buf);
		np = p->fts_pointer;
#ifdef __APPLE__
		buf[10] = '\0';	/* make +/@ abut the mode */
		char str[2] = { np->mode_suffix, '\0' };
#endif /* __APPLE__ */
		if (f_group && f_owner) {	/* means print neither */
#ifdef __APPLE__
			(void)printf("%s%s %*u   ", buf, str, dp->s_nlink,
				     sp->st_nlink);
#else  /* ! __APPLE__ */
			(void)printf("%s %*u   ", buf, dp->s_nlink,
				     sp->st_nlink);
#endif /* __APPLE__ */
		}
		else if (f_group) {
#ifdef __APPLE__
			(void)printf("%s%s %*u %-*s  ", buf, str, dp->s_nlink,
				     sp->st_nlink, dp->s_group, np->group);
#else  /* ! __APPLE__ */
			(void)printf("%s %*u %-*s  ", buf, dp->s_nlink,
				     sp->st_nlink, dp->s_group, np->group);
#endif /* __APPLE__ */
		}
		else if (f_owner) {
#ifdef __APPLE__
			(void)printf("%s%s %*u %-*s  ", buf, str, dp->s_nlink,
				     sp->st_nlink, dp->s_user, np->user);
#else  /* ! __APPLE__ */
			(void)printf("%s %*u %-*s  ", buf, dp->s_nlink,
				     sp->st_nlink, dp->s_user, np->user);
#endif /* __APPLE__ */
		}
		else {
#ifdef __APPLE__
			(void)printf("%s%s %*u %-*s  %-*s  ", buf, str, dp->s_nlink,
				     sp->st_nlink, dp->s_user, np->user, dp->s_group,
				     np->group);
#else  /* ! __APPLE__ */
			(void)printf("%s %*u %-*s  %-*s  ", buf, dp->s_nlink,
				     sp->st_nlink, dp->s_user, np->user, dp->s_group,
				     np->group);
#endif /* ! __APPLE__ */
		}
		if (f_flags)
			(void)printf("%-*s ", dp->s_flags, np->flags);
		if (S_ISCHR(sp->st_mode) || S_ISBLK(sp->st_mode))
			if (minor(sp->st_rdev) > 255 || minor(sp->st_rdev) < 0)
				(void)printf("%3d, 0x%08x ",
				    major(sp->st_rdev),
				    (u_int)minor(sp->st_rdev));
			else
				(void)printf("%3d, %3d ",
				    major(sp->st_rdev), minor(sp->st_rdev));
		else if (dp->bcfile)
			(void)printf("%*s%*qu ",
			    8 - dp->s_size, "", dp->s_size, (u_int64_t)sp->st_size);
		else
			printsize(dp->s_size, sp->st_size);
		if (f_accesstime)
			printtime(sp->st_atime);
		else if (f_statustime)
			printtime(sp->st_ctime);
		else if (f_birthtime) 
			printtime(sp->st_birthtime);
		else
			printtime(sp->st_mtime);
#ifdef COLORLS
		if (f_color)
			color_printed = colortype(sp->st_mode);
#endif
		(void)printname(p->fts_name);
#ifdef COLORLS
		if (f_color && color_printed)
			endcolor(0);
#endif
		if (f_type)
			(void)printtype(sp->st_mode);
		if (S_ISLNK(sp->st_mode))
			printlink(p);
		(void)putchar('\n');
#ifdef __APPLE__
		if (np->xattr_count && f_xattr) {
			printxattr(dp, np->xattr_count, np->xattr_names, np->xattr_sizes);
		}
                if (np->acl != NULL && f_acl) {
			printacl(np->acl, S_ISDIR(sp->st_mode));
		}
#endif /* __APPLE__ */
	}
}

void
printstream(DISPLAY *dp)
{
	FTSENT *p;
	extern int termwidth;
	int chcnt;

	for (p = dp->list, chcnt = 0; p; p = p->fts_link) {
		if (p->fts_number == NO_PRINT)
			continue;
		if (strlen(p->fts_name) + chcnt +
		    (p->fts_link ? 2 : 0) >= (unsigned)termwidth) {
			putchar('\n');
			chcnt = 0;
		}
		chcnt += printaname(p, dp->s_inode, dp->s_block);
		if (p->fts_link) {
			printf(", ");
			chcnt += 2;
		}
	}
	if (chcnt)
		putchar('\n');
}
		
void
printcol(DISPLAY *dp)
{
	extern int termwidth;
	static FTSENT **array;
	static int lastentries = -1;
	FTSENT *p;
	int base;
	int chcnt;
	int cnt;
	int col;
	int colwidth;
	int endcol;
	int num;
	int numcols;
	int numrows;
	int row;
	int tabwidth;

	if (f_notabs)
		tabwidth = 1;
	else
		tabwidth = 8;

	/*
	 * Have to do random access in the linked list -- build a table
	 * of pointers.
	 */
	if ((lastentries == -1) || (dp->entries > lastentries)) {
		lastentries = dp->entries;
		if ((array = realloc(array, dp->entries * sizeof(FTSENT *))) == NULL) {
			warn(NULL);
			printscol(dp);
			return;
		}
	}
	memset(array, 0, dp->entries * sizeof(FTSENT *));
	for (p = dp->list, num = 0; p; p = p->fts_link)
		if (p->fts_number != NO_PRINT)
			array[num++] = p;

	colwidth = dp->maxlen;
	if (f_inode)
		colwidth += dp->s_inode + 1;
	if (f_size)
		colwidth += dp->s_block + 1;
	if (f_type)
		colwidth += 1;

	colwidth = (colwidth + tabwidth) & ~(tabwidth - 1);
	if (termwidth < 2 * colwidth) {
		printscol(dp);
		return;
	}
	numcols = termwidth / colwidth;
	numrows = num / numcols;
	if (num % numcols)
		++numrows;

	assert(dp->list);
	if (dp->list->fts_level != FTS_ROOTLEVEL && (f_longform || f_size))
		(void)printf("total %qu\n", (u_int64_t)howmany(dp->btotal, blocksize));

	base = 0;
	for (row = 0; row < numrows; ++row) {
		endcol = colwidth;
		if (!f_sortacross)
			base = row;
		for (col = 0, chcnt = 0; col < numcols; ++col) {
			assert(base < dp->entries);
			chcnt += printaname(array[base], dp->s_inode, dp->s_block);
			if (f_sortacross)
				base++;
			else
				base += numrows;
			if (base >= num)
				break;
			while ((cnt = ((chcnt + tabwidth) & ~(tabwidth - 1)))
			    <= endcol) {
				if (f_sortacross && col + 1 >= numcols)
					break;
				(void)putchar(f_notabs ? ' ' : '\t');
				chcnt = cnt;
			}
			endcol += colwidth;
		}
		(void)putchar('\n');
	}
}

/*
 * print [inode] [size] name
 * return # of characters printed, no trailing characters.
 */
static int
printaname(FTSENT *p, u_long inodefield, u_long sizefield)
{
	struct stat *sp;
	int chcnt;
#ifdef COLORLS
	int color_printed = 0;
#endif

	sp = p->fts_statp;
	chcnt = 0;
	if (f_inode)
#if _DARWIN_FEATURE_64_BIT_INODE
		chcnt += printf("%*llu ", (int)inodefield, (u_quad_t)sp->st_ino);
#else
		chcnt += printf("%*lu ", (int)inodefield, (u_long)sp->st_ino);
#endif
	if (f_size)
		chcnt += printf("%*qu ",
		    (int)sizefield, (u_int64_t)howmany(sp->st_blocks, blocksize));
#ifdef COLORLS
	if (f_color)
		color_printed = colortype(sp->st_mode);
#endif
	chcnt += printname(p->fts_name);
#ifdef COLORLS
	if (f_color && color_printed)
		endcolor(0);
#endif
	if (f_type)
		chcnt += printtype(sp->st_mode);
	return (chcnt);
}

static void
printtime(time_t ftime)
{
	char longstring[80];
	static time_t now;
	const char *format;
	static int d_first = -1;

	if (d_first < 0)
		d_first = (*nl_langinfo(D_MD_ORDER) == 'd');
	if (now == 0)
		now = time(NULL);

#define	SIXMONTHS	((365 / 2) * 86400)
	if (f_sectime)
		/* mmm dd hh:mm:ss yyyy || dd mmm hh:mm:ss yyyy */
		format = d_first ? "%e %b %T %Y " : "%b %e %T %Y ";
	else if (COMPAT_MODE("bin/ls", "Unix2003")) {
		if (ftime + SIXMONTHS > now && ftime <= now)
			/* mmm dd hh:mm || dd mmm hh:mm */
			format = d_first ? "%e %b %R " : "%b %e %R ";
		else
			/* mmm dd  yyyy || dd mmm  yyyy */
			format = d_first ? "%e %b  %Y " : "%b %e  %Y ";
	}
	else if (ftime + SIXMONTHS > now && ftime < now + SIXMONTHS)
		/* mmm dd hh:mm || dd mmm hh:mm */
		format = d_first ? "%e %b %R " : "%b %e %R ";
	else
		/* mmm dd  yyyy || dd mmm  yyyy */
		format = d_first ? "%e %b  %Y " : "%b %e  %Y ";
	strftime(longstring, sizeof(longstring), format, localtime(&ftime));
	fputs(longstring, stdout);
}

static int
printtype(u_int mode)
{

	if (f_slash) {
		if ((mode & S_IFMT) == S_IFDIR) {
			(void)putchar('/');
			return (1);
		}
		return (0);
	}

	switch (mode & S_IFMT) {
	case S_IFDIR:
		(void)putchar('/');
		return (1);
	case S_IFIFO:
		(void)putchar('|');
		return (1);
	case S_IFLNK:
		(void)putchar('@');
		return (1);
	case S_IFSOCK:
		(void)putchar('=');
		return (1);
	case S_IFWHT:
		(void)putchar('%');
		return (1);
	default:
		break;
	}
	if (mode & (S_IXUSR | S_IXGRP | S_IXOTH)) {
		(void)putchar('*');
		return (1);
	}
	return (0);
}

#ifdef COLORLS
static int
putch(int c)
{
	(void)putchar(c);
	return 0;
}

static int
writech(int c)
{
	char tmp = c;

	(void)write(STDOUT_FILENO, &tmp, 1);
	return 0;
}

static void
printcolor(Colors c)
{
	char *ansiseq;

	if (colors[c].bold)
		tputs(enter_bold, 1, putch);

	if (colors[c].num[0] != -1) {
		ansiseq = tgoto(ansi_fgcol, 0, colors[c].num[0]);
		if (ansiseq)
			tputs(ansiseq, 1, putch);
	}
	if (colors[c].num[1] != -1) {
		ansiseq = tgoto(ansi_bgcol, 0, colors[c].num[1]);
		if (ansiseq)
			tputs(ansiseq, 1, putch);
	}
}

static void
endcolor(int sig)
{
	tputs(ansi_coloff, 1, sig ? writech : putch);
	tputs(attrs_off, 1, sig ? writech : putch);
}

static int
colortype(mode_t mode)
{
	switch (mode & S_IFMT) {
	case S_IFDIR:
		if (mode & S_IWOTH)
			if (mode & S_ISTXT)
				printcolor(C_WSDIR);
			else
				printcolor(C_WDIR);
		else
			printcolor(C_DIR);
		return (1);
	case S_IFLNK:
		printcolor(C_LNK);
		return (1);
	case S_IFSOCK:
		printcolor(C_SOCK);
		return (1);
	case S_IFIFO:
		printcolor(C_FIFO);
		return (1);
	case S_IFBLK:
		printcolor(C_BLK);
		return (1);
	case S_IFCHR:
		printcolor(C_CHR);
		return (1);
	}
	if (mode & (S_IXUSR | S_IXGRP | S_IXOTH)) {
		if (mode & S_ISUID)
			printcolor(C_SUID);
		else if (mode & S_ISGID)
			printcolor(C_SGID);
		else
			printcolor(C_EXEC);
		return (1);
	}
	return (0);
}

void
parsecolors(const char *cs)
{
	int i;
	int j;
	int len;
	char c[2];
	short legacy_warn = 0;

	if (cs == NULL)
		cs = "";	/* LSCOLORS not set */
	len = strlen(cs);
	for (i = 0; i < C_NUMCOLORS; i++) {
		colors[i].bold = 0;

		if (len <= 2 * i) {
			c[0] = defcolors[2 * i];
			c[1] = defcolors[2 * i + 1];
		} else {
			c[0] = cs[2 * i];
			c[1] = cs[2 * i + 1];
		}
		for (j = 0; j < 2; j++) {
			/* Legacy colours used 0-7 */
			if (c[j] >= '0' && c[j] <= '7') {
				colors[i].num[j] = c[j] - '0';
				if (!legacy_warn) {
					fprintf(stderr,
					    "warn: LSCOLORS should use "
					    "characters a-h instead of 0-9 ("
					    "see the manual page)\n");
				}
				legacy_warn = 1;
			} else if (c[j] >= 'a' && c[j] <= 'h')
				colors[i].num[j] = c[j] - 'a';
			else if (c[j] >= 'A' && c[j] <= 'H') {
				colors[i].num[j] = c[j] - 'A';
				colors[i].bold = 1;
			} else if (tolower((unsigned char)c[j] == 'x'))
				colors[i].num[j] = -1;
			else {
				fprintf(stderr,
				    "error: invalid character '%c' in LSCOLORS"
				    " env var\n", c[j]);
				colors[i].num[j] = -1;
			}
		}
	}
}

void
colorquit(int sig)
{
	endcolor(sig);

	(void)signal(sig, SIG_DFL);
	(void)kill(getpid(), sig);
}

#endif /* COLORLS */

static void
printlink(FTSENT *p)
{
	int lnklen;
	char name[MAXPATHLEN + 1];
	char path[MAXPATHLEN + 1];

	if (p->fts_level == FTS_ROOTLEVEL)
		(void)snprintf(name, sizeof(name), "%s", p->fts_name);
	else
		(void)snprintf(name, sizeof(name),
		    "%s/%s", p->fts_parent->fts_accpath, p->fts_name);
	if ((lnklen = readlink(name, path, sizeof(path) - 1)) == -1) {
		(void)fprintf(stderr, "\nls: %s: %s\n", name, strerror(errno));
		return;
	}
	path[lnklen] = '\0';
	(void)printf(" -> ");
	(void)printname(path);
}

static void
printsize(size_t width, off_t bytes)
{
    (void)printf("%*jd ", (u_int)width, (intmax_t)bytes);
}
