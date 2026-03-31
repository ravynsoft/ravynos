/*
 * finfo - print file info
 *
 * Chet Ramey
 * chet@po.cwru.edu
 */

/*
   Copyright (C) 1999-2009 Free Software Foundation, Inc.

   This file is part of GNU Bash.
   Bash is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Bash is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Bash.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <sys/types.h>
#ifdef MAJOR_IN_MKDEV
#  include <sys/mkdev.h>
#endif
#ifdef MAJOR_IN_SYSMACROS
#  include <sys/sysmacros.h>
#endif
#include "posixstat.h"
#include <stdio.h>
#include <pwd.h>
#include <grp.h>
#include <errno.h>
#include "posixtime.h"

#include "bashansi.h"
#include "shell.h"
#include "builtins.h"
#include "common.h"
#include "getopt.h"

#ifndef errno
extern int	errno;
#endif

extern char	**make_builtin_argv ();

static void	perms();
static int	printst();
static int	printsome();
static void	printmode();
static int	printfinfo();
static int	finfo_main();

extern int	sh_optind;
extern char	*sh_optarg;
extern char	*this_command_name;

static char	*prog;
static int	pmask;

#define OPT_UID		0x00001
#define OPT_GID		0x00002
#define OPT_DEV		0x00004
#define OPT_INO		0x00008
#define OPT_PERM	0x00010
#define OPT_LNKNAM	0x00020
#define OPT_FID		0x00040
#define OPT_NLINK	0x00080
#define OPT_RDEV	0x00100
#define OPT_SIZE	0x00200
#define OPT_ATIME	0x00400
#define OPT_MTIME	0x00800
#define OPT_CTIME	0x01000
#define OPT_BLKSIZE	0x02000
#define OPT_BLKS	0x04000
#define OPT_FTYPE	0x08000
#define OPT_PMASK	0x10000
#define OPT_OPERM	0x20000

#define OPT_ASCII	0x1000000

#define OPTIONS		"acdgiflmnopsuACGMP:U"

static int
octal(s)
char	*s;
{
	int	r;

	r = *s - '0';
	while (*++s >= '0' && *s <= '7')
		r = (r * 8) + (*s - '0');
	return r;
}

static int
finfo_main(argc, argv)
int	argc;
char	**argv;
{
	register int	i;
	int	mode, flags, opt;

	sh_optind = 0;	/* XXX */
	prog = base_pathname(argv[0]);
	if (argc == 1) {
		builtin_usage();
		return(1);
	}
	flags = 0;
	while ((opt = sh_getopt(argc, argv, OPTIONS)) != EOF) {
		switch(opt) {
		case 'a': flags |= OPT_ATIME; break;
		case 'A': flags |= OPT_ATIME|OPT_ASCII; break;
		case 'c': flags |= OPT_CTIME; break;
		case 'C': flags |= OPT_CTIME|OPT_ASCII; break;
		case 'd': flags |= OPT_DEV; break;
		case 'i': flags |= OPT_INO; break;
		case 'f': flags |= OPT_FID; break;
		case 'g': flags |= OPT_GID; break;
		case 'G': flags |= OPT_GID|OPT_ASCII; break;
		case 'l': flags |= OPT_LNKNAM; break;
		case 'm': flags |= OPT_MTIME; break;
		case 'M': flags |= OPT_MTIME|OPT_ASCII; break;
		case 'n': flags |= OPT_NLINK; break;
		case 'o': flags |= OPT_OPERM; break;
		case 'p': flags |= OPT_PERM; break;
		case 'P': flags |= OPT_PMASK; pmask = octal(sh_optarg); break;
		case 's': flags |= OPT_SIZE; break;
		case 'u': flags |= OPT_UID; break;
		case 'U': flags |= OPT_UID|OPT_ASCII; break;
		default: builtin_usage (); return(1);
		}
	}

	argc -= sh_optind;
	argv += sh_optind;

	if (argc == 0) {
		builtin_usage();
		return(1);
	}

	for (i = 0; i < argc; i++)
		opt = flags ? printsome (argv[i], flags) : printfinfo(argv[i]);

	return(opt);
}

static struct stat *
getstat(f)
char	*f;
{
	static struct stat st;
	int	fd, r;
	intmax_t lfd;

	if (strncmp(f, "/dev/fd/", 8) == 0) {
		if ((legal_number(f + 8, &lfd) == 0) || (int)lfd != lfd) {
			builtin_error("%s: invalid fd", f + 8);
			return ((struct stat *)0);
		}
		fd = lfd;
		r = fstat(fd, &st);
	} else
#ifdef HAVE_LSTAT
		r = lstat(f, &st);
#else
		r = stat(f, &st);
#endif
	if (r < 0) {
		builtin_error("%s: cannot stat: %s", f, strerror(errno));
		return ((struct stat *)0);
	}
	return (&st);
}

static int
printfinfo(f)
char	*f;
{
	struct stat *st;

	st = getstat(f);
	return (st ? printst(st) : 1);
}

static int
getperm(m)
int	m;
{
	return (m & (S_IRWXU|S_IRWXG|S_IRWXO|S_ISUID|S_ISGID));
}

static void
perms(m)
int	m;
{
	char ubits[4], gbits[4], obits[4];	/* u=rwx,g=rwx,o=rwx */
	int i;

	i = 0;
	if (m & S_IRUSR)
		ubits[i++] = 'r';
	if (m & S_IWUSR)
		ubits[i++] = 'w';
	if (m & S_IXUSR)
		ubits[i++] = 'x';
	ubits[i] = '\0';

	i = 0;
	if (m & S_IRGRP)
		gbits[i++] = 'r';
	if (m & S_IWGRP)
		gbits[i++] = 'w';
	if (m & S_IXGRP)
		gbits[i++] = 'x';
	gbits[i] = '\0';

	i = 0;
	if (m & S_IROTH)
		obits[i++] = 'r';
	if (m & S_IWOTH)
		obits[i++] = 'w';
	if (m & S_IXOTH)
		obits[i++] = 'x';
	obits[i] = '\0';

	if (m & S_ISUID)
		ubits[2] = (m & S_IXUSR) ? 's' : 'S';
	if (m & S_ISGID)
		gbits[2] = (m & S_IXGRP) ? 's' : 'S';
	if (m & S_ISVTX)
		obits[2] = (m & S_IXOTH) ? 't' : 'T';

	printf ("u=%s,g=%s,o=%s", ubits, gbits, obits);
}

static void
printmode(mode)
int	mode;
{
	if (S_ISBLK(mode))
		printf("S_IFBLK ");
	if (S_ISCHR(mode))
		printf("S_IFCHR ");
	if (S_ISDIR(mode))
		printf("S_IFDIR ");
	if (S_ISREG(mode))
		printf("S_IFREG ");
	if (S_ISFIFO(mode))
		printf("S_IFIFO ");
	if (S_ISLNK(mode))
		printf("S_IFLNK ");
	if (S_ISSOCK(mode))
		printf("S_IFSOCK ");
#ifdef S_ISWHT
	if (S_ISWHT(mode))
		printf("S_ISWHT ");
#endif
	perms(getperm(mode));
	printf("\n");
}

static int
printst(st)
struct stat *st;
{
	struct passwd	*pw;
	struct group	*gr;
	char	*owner;
	int	ma, mi, d;

	ma = major (st->st_rdev);
	mi = minor (st->st_rdev);
#if defined (makedev)
	d = makedev (ma, mi);
#else
	d = st->st_rdev & 0xFF;
#endif
	printf("Device (major/minor): %d (%d/%d)\n", d, ma, mi);

	printf("Inode: %d\n", (int) st->st_ino);
	printf("Mode: (%o) ", (int) st->st_mode);
	printmode((int) st->st_mode);
	printf("Link count: %d\n", (int) st->st_nlink);
	pw = getpwuid(st->st_uid);
	owner = pw ? pw->pw_name : "unknown";
	printf("Uid of owner: %d (%s)\n", (int) st->st_uid, owner);
	gr = getgrgid(st->st_gid);
	owner = gr ? gr->gr_name : "unknown";
	printf("Gid of owner: %d (%s)\n", (int) st->st_gid, owner);
	printf("Device type: %d\n", (int) st->st_rdev);
	printf("File size: %ld\n", (long) st->st_size);
	printf("File last access time: %s", ctime (&st->st_atime));
	printf("File last modify time: %s", ctime (&st->st_mtime));
	printf("File last status change time: %s", ctime (&st->st_ctime));
	fflush(stdout);
	return(0);
}

static int
printsome(f, flags)
char	*f;
int	flags;
{
	struct stat *st;
	struct passwd *pw;
	struct group *gr;
	int	p;
	char	*b;

	st = getstat(f);
	if (st == NULL)
		return (1);

	/* Print requested info */
	if (flags & OPT_ATIME) {
		if (flags & OPT_ASCII)
			printf("%s", ctime(&st->st_atime));
		else
			printf("%ld\n", st->st_atime);
	} else if (flags & OPT_MTIME) {
		if (flags & OPT_ASCII)
			printf("%s", ctime(&st->st_mtime));
		else
			printf("%ld\n", st->st_mtime);
	} else if (flags & OPT_CTIME) {
		if (flags & OPT_ASCII)
			printf("%s", ctime(&st->st_ctime));
		else
			printf("%ld\n", st->st_ctime);
	} else if (flags & OPT_DEV)
		printf("%lu\n", (unsigned long)st->st_dev);
	else if (flags & OPT_INO)
		printf("%lu\n", (unsigned long)st->st_ino);
	else if (flags & OPT_FID)
		printf("%lu:%lu\n", (unsigned long)st->st_dev, (unsigned long)st->st_ino);
	else if (flags & OPT_NLINK)
		printf("%lu\n", (unsigned long)st->st_nlink);
	else if (flags & OPT_LNKNAM) {
#ifdef S_ISLNK
		b = xmalloc(4096);
		p = readlink(f, b, 4096);
		if (p >= 0 && p < 4096)
			b[p] = '\0';
		else {
			p = errno;
			strcpy(b, prog);
			strcat(b, ": ");
			strcat(b, strerror(p));
		}
		printf("%s\n", b);
		free(b);
#else
		printf("%s\n", f);
#endif
	} else if (flags & OPT_PERM) {
		perms(st->st_mode);
		printf("\n");
	} else if (flags & OPT_OPERM)
		printf("%o\n", getperm(st->st_mode));
	else if (flags & OPT_PMASK)
		printf("%o\n", getperm(st->st_mode) & pmask);
	else if (flags & OPT_UID) {
		pw = getpwuid(st->st_uid);
		if (flags & OPT_ASCII)
			printf("%s\n", pw ? pw->pw_name : "unknown");
		else
			printf("%d\n", st->st_uid);
	} else if (flags & OPT_GID) {
		gr = getgrgid(st->st_gid);
		if (flags & OPT_ASCII)
			printf("%s\n", gr ? gr->gr_name : "unknown");
		else
			printf("%d\n", st->st_gid);
	} else if (flags & OPT_SIZE)
		printf("%ld\n", (long) st->st_size);

	return (0);
}

#ifndef NOBUILTIN
int
finfo_builtin(list)
     WORD_LIST *list;
{
  int c, r;
  char **v;
  WORD_LIST *l;

  v = make_builtin_argv (list, &c);
  r = finfo_main (c, v);
  free (v);

  return r;
}

static char *finfo_doc[] = {
  "Display information about file attributes.",
  "",
  "Display information about each FILE.  Only single operators should",
  "be supplied.  If no options are supplied, a summary of the info",
  "available about each FILE is printed.  If FILE is of the form",
  "/dev/fd/XX, file descriptor XX is described.  Operators, if supplied,",
  "have the following meanings:",
  "",
  "	-a	last file access time",
  "	-A	last file access time in ctime format",
  "	-c	last file status change time",
  "	-C	last file status change time in ctime format",
  "	-m	last file modification time",
  "	-M	last file modification time in ctime format",
  "	-d	device",
  "	-i	inode",
  "	-f	composite file identifier (device:inode)",
  "	-g	gid of owner",
  "	-G	group name of owner",
  "	-l	name of file pointed to by symlink",
  "	-n	link count",
  "	-o	permissions in octal",
  "	-p	permissions in ascii",
  "	-P mask permissions ANDed with MASK (like with umask)",
  "	-s	file size in bytes",
  "	-u	uid of owner",
  "	-U	user name of owner",
  (char *)0
};

struct builtin finfo_struct = {
	"finfo",
	finfo_builtin,
	BUILTIN_ENABLED,
	finfo_doc,
	"finfo [-acdgiflmnopsuACGMPU] file [file...]",
	0
};
#endif

#ifdef NOBUILTIN
#if defined (PREFER_STDARG)
#  include <stdarg.h>
#else
#  if defined (PREFER_VARARGS)
#    include <varargs.h>
#  endif
#endif

char *this_command_name;

main(argc, argv)
int	argc;
char	**argv;
{
	this_command_name = argv[0];
	exit(finfo_main(argc, argv));
}

void
builtin_usage()
{
	fprintf(stderr, "%s: usage: %s [-%s] [file ...]\n", prog, prog, OPTIONS);
}

#ifndef HAVE_STRERROR
char *
strerror(e)
int	e;
{
	static char	ebuf[40];
	extern int	sys_nerr;
	extern char	*sys_errlist[];

	if (e < 0 || e > sys_nerr) {
		sprintf(ebuf,"Unknown error code %d", e);
		return (&ebuf[0]);
	}
	return (sys_errlist[e]);
}
#endif

char *
xmalloc(s)
size_t	s;
{
	char	*ret;
	extern char *malloc();

	ret = malloc(s);
	if (ret)
		return (ret);
	fprintf(stderr, "%s: cannot malloc %d bytes\n", prog, s);
	exit(1);
}

char *
base_pathname(p)
char	*p;
{
	char	*t;

	if (t = strrchr(p, '/'))
		return(++t);
	return(p);
}

int
legal_number (string, result)
     char *string;
     long *result;
{
  int sign;
  long value;

  sign = 1;
  value = 0;

  if (result)
    *result = 0;

  /* Skip leading whitespace characters. */
  while (whitespace (*string))
    string++;

  if (!*string)
    return (0);

  /* We allow leading `-' or `+'. */
  if (*string == '-' || *string == '+')
    {
      if (!digit (string[1]))
        return (0);

      if (*string == '-')
        sign = -1;

      string++;
    }

  while (digit (*string))
    {
      if (result)
        value = (value * 10) + digit_value (*string);
      string++;
    }

  /* Skip trailing whitespace, if any. */
  while (whitespace (*string))
    string++;

  /* Error if not at end of string. */
  if (*string)
    return (0);

  if (result)
    *result = value * sign;

  return (1);
}

int sh_optind;
char *sh_optarg;
int sh_opterr;

extern int optind;
extern char *optarg;

int
sh_getopt(c, v, o)
int	c;
char	**v, *o;
{
	int	r;

	r = getopt(c, v, o);
	sh_optind = optind;
	sh_optarg = optarg;
	return r;
}

#if defined (USE_VARARGS)
void
#if defined (PREFER_STDARG)
builtin_error (const char *format, ...)
#else
builtin_error (format, va_alist)
     const char *format;
     va_dcl
#endif
{
  va_list args;

  if (this_command_name && *this_command_name)
    fprintf (stderr, "%s: ", this_command_name);

#if defined (PREFER_STDARG)
  va_start (args, format);
#else
  va_start (args);
#endif

  vfprintf (stderr, format, args);
  va_end (args);
  fprintf (stderr, "\n");
}
#else
void
builtin_error (format, arg1, arg2, arg3, arg4, arg5)
     char *format, *arg1, *arg2, *arg3, *arg4, *arg5;
{
  if (this_command_name && *this_command_name)
    fprintf (stderr, "%s: ", this_command_name);

  fprintf (stderr, format, arg1, arg2, arg3, arg4, arg5);
  fprintf (stderr, "\n");
  fflush (stderr);
}
#endif /* !USE_VARARGS */

#endif
