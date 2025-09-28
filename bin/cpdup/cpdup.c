/*-
 * CPDUP.C
 *
 * CPDUP <options> source destination
 *
 * (c) Copyright 1997-1999 by Matthew Dillon and Dima Ruban.  Permission to
 *     use and distribute based on the FreeBSD copyright.  Supplied as-is,
 *     USE WITH EXTREME CAUTION.
 *
 * This program attempts to duplicate the source onto the destination as
 * exactly as possible, retaining modify times, flags, perms, uid, and gid.
 * It can duplicate devices, files (including hardlinks), softlinks,
 * directories, and so forth.  It is recursive by default!  The duplication
 * is inclusive of removal of files/directories on the destination that do
 * not exist on the source.  This program supports a per-directory exception
 * file called .cpignore, or a user-specified exception file.
 *
 * Safety features:
 *
 *	- does not cross partition boundries on source
 *	- asks for confirmation on deletions unless -i0 is specified
 *	- refuses to replace a destination directory with a source file
 *	  unless -s0 is specified.
 *	- terminates on error
 *
 * Copying features:
 *
 *	- does not copy file if mtime, flags, perms, and size match unless
 *	  forced
 *
 *	- copies to temporary and renames-over the original, allowing
 *	  you to update live systems
 *
 *	- copies uid, gid, mtime, perms, flags, softlinks, devices, hardlinks,
 *	  and recurses through directories.
 *
 *	- accesses a per-directory exclusion file, .cpignore, containing
 *	  standard wildcarded ( ? / * style, NOT regex) exclusions.
 *
 *	- tries to play permissions and flags smart in regards to overwriting
 *	  schg files and doing related stuff.
 *
 *	- Can do MD5 consistancy checks
 *
 *	- Is able to do incremental mirroring/backups via hardlinks from
 *	  the 'previous' version (supplied with -H path).
 */

/*-
 * Example: cc -O cpdup.c -o cpdup -lcrypto
 *
 * ".MD5.CHECKSUMS" contains md5 checksumms for the current directory.
 * This file is stored on the source.
 */

#include "cpdup.h"
#include "hclink.h"
#include "hcproto.h"

#define HSIZE	8192
#define HMASK	(HSIZE-1)
#define HLSIZE	8192
#define HLMASK	(HLSIZE - 1)

#define GETBUFSIZE	8192
#define GETPATHSIZE	2048
#define GETLINKSIZE	1024
#define GETIOSIZE	65536

#ifndef _ST_FLAGS_PRESENT_
#define st_flags	st_mode
#endif

typedef struct Node {
    struct Node *no_Next;
    struct Node *no_HNext;
    struct stat *no_Stat;
    int  no_Value;
    char no_Name[4];
} Node;

typedef struct List {
    Node	li_Node;
    Node	*li_Hash[HSIZE];
} List;

struct hlink {
    ino_t ino;
    ino_t dino;
    int	refs;
    struct hlink *next;
    struct hlink *prev;
    nlink_t nlinked;
    char name[];
};

typedef struct copy_info {
	char *spath;
	char *dpath;
	dev_t sdevNo;
	dev_t ddevNo;
} *copy_info_t;

static struct hlink *hltable[HLSIZE];

static void RemoveRecur(const char *dpath, dev_t devNo, struct stat *dstat);
static void InitList(List *list);
static void ResetList(List *list);
static Node *IterateList(List *list, Node *node, int n);
static int AddList(List *list, const char *name, int n, struct stat *st);
static int CheckList(List *list, const char *path, const char *name);
static int getbool(const char *str);
static char *SplitRemote(char **pathp);
static int ChgrpAllowed(gid_t g);
static int OwnerMatch(struct stat *st1, struct stat *st2);
#ifdef _ST_FLAGS_PRESENT_
static int FlagsMatch(struct stat *st1, struct stat *st2);
#else
#define FlagsMatch(st1, st2)	1
#endif
static struct hlink *hltlookup(struct stat *);
static struct hlink *hltadd(struct stat *, const char *);
static char *checkHLPath(struct stat *st, const char *spath, const char *dpath);
static int validate_check(const char *spath, const char *dpath);
static int shash(const char *s);
static void hltdelete(struct hlink *);
static void hltsetdino(struct hlink *, ino_t);
static int YesNo(const char *path);
static int xrename(const char *src, const char *dst, u_long flags);
static int xlink(const char *src, const char *dst, u_long flags);
static int xremove(struct HostConf *host, const char *path);
static int xrmdir(struct HostConf *host, const char *path);
static int DoCopy(copy_info_t info, struct stat *stat1, int depth);
static int ScanDir(List *list, struct HostConf *host, const char *path,
	int64_t *CountReadBytes, int n);
static int mtimecmp(struct stat *st1, struct stat *st2);
static int symlink_mfo_test(struct HostConf *hc, struct stat *st1,
	struct stat *st2);

int AskConfirmation = 1;
int SafetyOpt = 1;
int ForceOpt;
int DeviceOpt = 1;
int VerboseOpt;
int DirShowOpt;
int NotForRealOpt;
int QuietOpt;
int NoRemoveOpt;
int UseMD5Opt;
int SummaryOpt;
int CompressOpt;
int SlaveOpt;
int ReadOnlyOpt;
int ValidateOpt;
int ssh_argc;
const char *ssh_argv[16];
int DstRootPrivs;

const char *UseCpFile;
const char *MD5CacheFile;
const char *UseHLPath;

static int DstBaseLen;
static int HardLinkCount;
static int GroupCount;
static gid_t *GroupList;

int64_t CountSourceBytes;
int64_t CountSourceItems;
int64_t CountCopiedItems;
int64_t CountSourceReadBytes;
int64_t CountTargetReadBytes;
int64_t CountWriteBytes;
int64_t CountRemovedItems;
int64_t CountLinkedItems;

static struct HostConf SrcHost;
static struct HostConf DstHost;

int
main(int ac, char **av)
{
    int i;
    int opt;
    char *src = NULL;
    char *dst = NULL;
    char *ptr;
    struct timeval start;
    struct copy_info info;

    signal(SIGPIPE, SIG_IGN);

    gettimeofday(&start, NULL);
    opterr = 0;
    while ((opt = getopt(ac, av, ":CdF:fH:hIi:j:lM:mnoqRSs:uVvX:x")) != -1) {
	switch (opt) {
	case 'C':
	    CompressOpt = 1;
	    break;
	case 'd':
	    DirShowOpt = 1;
	    break;
	case 'F':
	    if (ssh_argc >= 16)
		fatal("too many -F options");
	    ssh_argv[ssh_argc++] = optarg;
	    break;
	case 'f':
	    ForceOpt = 1;
	    break;
	case 'H':
	    UseHLPath = optarg;
	    break;
	case 'h':
	    fatal(NULL);
	    /* not reached */
	    break;
	case 'I':
	    SummaryOpt = 1;
	    break;
	case 'i':
	    AskConfirmation = getbool(optarg);
	    break;
	case 'j':
	    DeviceOpt = getbool(optarg);
	    break;
	case 'l':
	    setlinebuf(stdout);
	    setlinebuf(stderr);
	    break;
	case 'M':
	    UseMD5Opt = 1;
	    MD5CacheFile = optarg;
	    break;
	case 'm':
	    UseMD5Opt = 1;
	    MD5CacheFile = ".MD5.CHECKSUMS";
	    break;
	case 'n':
	    NotForRealOpt = 1;
	    break;
	case 'o':
	    NoRemoveOpt = 1;
	    break;
	case 'q':
	    QuietOpt = 1;
	    break;
	case 'R':
	    ReadOnlyOpt = 1;
	    break;
	case 'S':
	    SlaveOpt = 1;
	    break;
	case 's':
	    SafetyOpt = getbool(optarg);
	    break;
	case 'u':
	    setvbuf(stdout, NULL, _IOLBF, 0);
	    break;
	case 'V':
	    ++ValidateOpt;
	    break;
	case 'v':
	    ++VerboseOpt;
	    break;
	case 'X':
	    UseCpFile = optarg;
	    break;
	case 'x':
	    UseCpFile = ".cpignore";
	    break;
	case ':':
	    fatal("missing argument for option: -%c\n", optopt);
	    /* not reached */
	    break;
	case '?':
	    fatal("illegal option: -%c\n", optopt);
	    /* not reached */
	    break;
	default:
	    fatal(NULL);
	    /* not reached */
	    break;
	}
    }
    ac -= optind;
    av += optind;
    if (ac > 0)
	src = av[0];
    if (ac > 1)
	dst = av[1];
    if (ac > 2)
	fatal("too many arguments");

    /*
     * If we are told to go into slave mode, run the HC protocol
     */
    if (SlaveOpt) {
	DstRootPrivs = (geteuid() == 0);
	hc_slave(0, 1);
	exit(0);
    }

    /*
     * Extract the source and/or/neither target [user@]host and
     * make any required connections.
     */
    if (src && (ptr = SplitRemote(&src)) != NULL) {
	SrcHost.host = src;
	src = ptr;
	if (UseMD5Opt)
	    fatal("The MD5 options are not currently supported for remote sources");
	if (hc_connect(&SrcHost, ReadOnlyOpt) < 0)
	    exit(1);
    } else {
	SrcHost.version = HCPROTO_VERSION;
	if (ReadOnlyOpt)
	    fatal("The -R option is only supported for remote sources");
    }

    if (dst && (ptr = SplitRemote(&dst)) != NULL) {
	DstHost.host = dst;
	dst = ptr;
	if (hc_connect(&DstHost, 0) < 0)
	    exit(1);
    } else {
	DstHost.version = HCPROTO_VERSION;
    }

    /*
     * dst may be NULL only if -m option is specified,
     * which forces an update of the MD5 checksums
     */
    if (dst == NULL && UseMD5Opt == 0) {
	fatal(NULL);
	/* not reached */
    }

    if (dst) {
	DstRootPrivs = (hc_geteuid(&DstHost) == 0);
	if (!DstRootPrivs)
	    GroupCount = hc_getgroups(&DstHost, &GroupList);
    }
#if 0
    /* XXXX DEBUG */
    fprintf(stderr, "DstRootPrivs == %s\n", DstRootPrivs ? "true" : "false");
    fprintf(stderr, "GroupCount == %d\n", GroupCount);
    for (i = 0; i < GroupCount; i++)
	fprintf(stderr, "Group[%d] == %d\n", i, GroupList[i]);
#endif

    bzero(&info, sizeof(info));
    if (dst) {
	DstBaseLen = strlen(dst);
	info.spath = src;
	info.dpath = dst;
	info.sdevNo = (dev_t)-1;
	info.ddevNo = (dev_t)-1;
	i = DoCopy(&info, NULL, -1);
    } else {
	info.spath = src;
	info.dpath = NULL;
	info.sdevNo = (dev_t)-1;
	info.ddevNo = (dev_t)-1;
	i = DoCopy(&info, NULL, -1);
    }
#ifndef NOMD5
    md5_flush();
#endif

    if (SummaryOpt && i == 0) {
	double duration;
	struct timeval end;

	gettimeofday(&end, NULL);
#if 0
	/* don't count stat's in our byte statistics */
	CountSourceBytes += sizeof(struct stat) * CountSourceItems;
	CountSourceReadBytes += sizeof(struct stat) * CountSourceItems;
	CountWriteBytes +=  sizeof(struct stat) * CountCopiedItems;
	CountWriteBytes +=  sizeof(struct stat) * CountRemovedItems;
#endif

	duration = (end.tv_sec - start.tv_sec);
	duration += (double)(end.tv_usec - start.tv_usec) / 1000000.0;
	if (duration == 0.0)
		duration = 1.0;
	logstd("cpdup completed successfully\n");
	logstd("%lld bytes source, %lld src bytes read, %lld tgt bytes read\n"
	       "%lld bytes written (%.1fX speedup)\n",
	    (long long)CountSourceBytes,
	    (long long)CountSourceReadBytes,
	    (long long)CountTargetReadBytes,
	    (long long)CountWriteBytes,
	    ((double)CountSourceBytes * 2.0) / ((double)(CountSourceReadBytes + CountTargetReadBytes + CountWriteBytes)));
 	logstd("%lld source items, %lld items copied, %lld items linked, "
	       "%lld things deleted\n",
	    (long long)CountSourceItems,
	    (long long)CountCopiedItems,
	    (long long)CountLinkedItems,
	    (long long)CountRemovedItems);
	logstd("%.1f seconds %5d Kbytes/sec synced %5d Kbytes/sec scanned\n",
	    duration,
	    (int)((CountSourceReadBytes + CountTargetReadBytes + CountWriteBytes) / duration  / 1024.0),
	    (int)(CountSourceBytes / duration / 1024.0));
    }
    exit((i == 0) ? 0 : 1);
}

static int
getbool(const char *str)
{
    if (strcmp(str, "0") == 0)
	return (0);
    if (strcmp(str, "1") == 0)
	return (1);
    fatal("option requires boolean argument (0 or 1): -%c\n", optopt);
    /* not reached */
    return (0);
}

/*
 * Check if path specifies a remote path, using the same syntax as scp(1),
 * i.e. a path is considered remote if the first colon is not preceded by
 * a slash, so e.g. "./foo:bar" is considered local.
 * If a remote path is detected, the colon is replaced with a null byte,
 * and the return value is a pointer to the next character.
 * Otherwise NULL is returned.
 *
 * A path prefix of localhost is the same as a locally specified file or
 * directory path, but prevents any further interpretation of the path
 * as being a remote hostname (for paths that have colons in them).
 */
static char *
SplitRemote(char **pathp)
{
    int cindex;
    char *path = *pathp;

    if (path[(cindex = strcspn(path, ":/"))] == ':') {
	path[cindex++] = 0;
	if (strcmp(path, "localhost") != 0)
		return (path + cindex);
	*pathp = path + cindex;
    }
    return (NULL);
}

/*
 * Check if group g is in our GroupList.
 *
 * Typically the number of groups a user belongs to isn't large
 * enough to warrant more effort than a simple linear search.
 * However, we perform an optimization by moving a group to the
 * top of the list when we have a hit.  This assumes that there
 * isn't much variance in the gids of files that a non-root user
 * copies.  So most of the time the search will terminate on the
 * first element of the list.
 */
static int
ChgrpAllowed(gid_t g)
{
    int i;

    for (i = 0; i < GroupCount; i++)
	if (GroupList[i] == g) {
	    if (i > 0) {
		/* Optimize: Move g to the front of the list. */
		for (; i > 0; i--)
		    GroupList[i] = GroupList[i - 1];
		GroupList[0] = g;
	    }
	    return (1);
	}
    return (0);
}

/*
 * The following two functions return true if the ownership (UID + GID)
 * or the flags of two files match, respectively.
 *
 * Only perform weak checking if we don't have sufficient privileges on
 * the target machine, so we don't waste transfers with things that are
 * bound to fail anyway.
 */
static int
OwnerMatch(struct stat *st1, struct stat *st2)
{
    if (DstRootPrivs)
	/* Both UID and GID must match. */
	return (st1->st_uid == st2->st_uid && st1->st_gid == st2->st_gid);
    else
	/* Ignore UID, and also ignore GID if we can't chgrp to that group. */
	return (st1->st_gid == st2->st_gid || !ChgrpAllowed(st1->st_gid));
}

#ifdef _ST_FLAGS_PRESENT_
static int
FlagsMatch(struct stat *st1, struct stat *st2)
{
/*
 * Ignore UF_ARCHIVE.  It gets set automatically by the filesystem, for
 * filesystems that support it.  If the destination filesystem supports it, but
 * it's cleared on the source file, then multiple invocations of cpdup would
 * all try to copy the file because the flags wouldn't match.
 *
 * When unpriveleged, ignore flags we can't set
 */
    u_long ignored = DstRootPrivs ? 0 : SF_SETTABLE;

#ifdef UF_ARCHIVE
    ignored |= UF_ARCHIVE;
#endif
    return (((st1->st_flags ^ st2->st_flags) & ~ignored) == 0);
}
#endif


static struct hlink *
hltlookup(struct stat *stp)
{
    struct hlink *hl;
    int n;

    n = stp->st_ino & HLMASK;

    for (hl = hltable[n]; hl; hl = hl->next) {
        if (hl->ino == stp->st_ino) {
	    ++hl->refs;
	    return hl;
	}
    }

    return NULL;
}

static struct hlink *
hltadd(struct stat *stp, const char *path)
{
    struct hlink *new;
    int plen = strlen(path);
    int n;

    new = malloc(offsetof(struct hlink, name[plen + 1]));
    if (new == NULL)
        fatal("out of memory");
    ++HardLinkCount;

    /* initialize and link the new element into the table */
    new->ino = stp->st_ino;
    new->dino = (ino_t)-1;
    new->refs = 1;
    bcopy(path, new->name, plen + 1);
    new->nlinked = 1;
    new->prev = NULL;
    n = stp->st_ino & HLMASK;
    new->next = hltable[n];
    if (hltable[n])
        hltable[n]->prev = new;
    hltable[n] = new;

    return new;
}

static void
hltsetdino(struct hlink *hl, ino_t inum)
{
    hl->dino = inum;
}

static void
hltdelete(struct hlink *hl)
{
    assert(hl->refs == 1);
    --hl->refs;
    if (hl->prev) {
        if (hl->next)
            hl->next->prev = hl->prev;
        hl->prev->next = hl->next;
    } else {
        if (hl->next)
            hl->next->prev = NULL;

        hltable[hl->ino & HLMASK] = hl->next;
    }
    --HardLinkCount;
    free(hl);
}

static void
hltrels(struct hlink *hl)
{
    assert(hl->refs == 1);
    --hl->refs;
}

/*
 * If UseHLPath is defined check to see if the file in question is
 * the same as the source file, and if it is return a pointer to the
 * -H path based file for hardlinking.  Else return NULL.
 */
static char *
checkHLPath(struct stat *st1, const char *spath, const char *dpath)
{
    struct stat sthl;
    char *hpath;
    int error;

    if (asprintf(&hpath, "%s%s", UseHLPath, dpath + DstBaseLen) < 0)
	fatal("out of memory");

    /*
     * stat info matches ?
     */
    if (hc_stat(&DstHost, hpath, &sthl) < 0 ||
	st1->st_size != sthl.st_size ||
	mtimecmp(st1, &sthl) != 0 ||
	!OwnerMatch(st1, &sthl) ||
	!FlagsMatch(st1, &sthl)
    ) {
	free(hpath);
	return(NULL);
    }

    /*
     * If ForceOpt or ValidateOpt is set we have to compare the files
     */
    if (ForceOpt || ValidateOpt) {
	error = validate_check(spath, hpath);
	if (error) {
	    free(hpath);
	    hpath = NULL;
	}
    }
    return(hpath);
}

/*
 * Return 0 if the contents of the file <spath> matches the contents of
 * the file <dpath>.
 */
static int
validate_check(const char *spath, const char *dpath)
{
    int error;
    int fd1;
    int fd2;

    fd1 = hc_open(&SrcHost, spath, O_RDONLY, 0);
    fd2 = hc_open(&DstHost, dpath, O_RDONLY, 0);
    error = -1;

    if (fd1 >= 0 && fd2 >= 0) {
	int n;
	int x;
	char *iobuf1 = malloc(GETIOSIZE);
	char *iobuf2 = malloc(GETIOSIZE);

	while ((n = hc_read(&SrcHost, fd1, iobuf1, GETIOSIZE)) > 0) {
	    CountSourceReadBytes += n;
	    x = hc_read(&DstHost, fd2, iobuf2, GETIOSIZE);
	    if (x > 0)
		    CountTargetReadBytes += x;
	    if (x != n)
		break;
	    if (bcmp(iobuf1, iobuf2, n) != 0)
		break;
	}
	free(iobuf1);
	free(iobuf2);
	if (n == 0)
	    error = 0;
    }
    if (fd1 >= 0)
	hc_close(&SrcHost, fd1);
    if (fd2 >= 0)
	hc_close(&DstHost, fd2);
    return (error);
}

int
DoCopy(copy_info_t info, struct stat *stat1, int depth)
{
    const char *spath = info->spath;
    const char *dpath = info->dpath;
    dev_t sdevNo = info->sdevNo;
    dev_t ddevNo = info->ddevNo;
    struct stat st1;
    struct stat st2;
    unsigned long st2_flags;
    int r, mres, fres, st2Valid;
    struct hlink *hln;
    uint64_t size;

    r = mres = fres = st2Valid = 0;
    st2_flags = 0;
    size = 0;
    hln = NULL;

    if (stat1 == NULL) {
	if (hc_lstat(&SrcHost, spath, &st1) != 0) {
	    r = 1;
	    goto done;
	}
	stat1 = &st1;
    }
#ifdef SF_SNAPSHOT
    /* skip snapshot files because they're sparse and _huge_ */
    if (stat1->st_flags & SF_SNAPSHOT)
       return(0);
#endif
    st2.st_mode = 0;	/* in case lstat fails */
    st2.st_flags = 0;	/* in case lstat fails */
    if (dpath && hc_lstat(&DstHost, dpath, &st2) == 0) {
	st2Valid = 1;
#ifdef _ST_FLAGS_PRESENT_
	st2_flags = st2.st_flags;
#endif
    }

    if (S_ISREG(stat1->st_mode))
	size = stat1->st_size;

    /*
     * Handle hardlinks
     */

    if (S_ISREG(stat1->st_mode) && stat1->st_nlink > 1 && dpath) {
        if ((hln = hltlookup(stat1)) != NULL) {
            hln->nlinked++;

            if (st2Valid) {
                if (st2.st_ino == hln->dino) {
		    /*
		     * hard link is already correct, nothing to do
		     */
		    if (VerboseOpt >= 3)
			logstd("%-32s nochange\n", (dpath) ? dpath : spath);
                    if (hln->nlinked == stat1->st_nlink) {
                        hltdelete(hln);
			hln = NULL;
		    }
		    CountSourceItems++;
		    r = 0;
		    goto done;
                } else {
		    /*
		     * hard link is not correct, attempt to unlink it
		     */
                    if (xremove(&DstHost, dpath) < 0) {
			logerr("%-32s hardlink: unable to unlink: %s\n",
			    ((dpath) ? dpath : spath), strerror(errno));
                        hltdelete(hln);
			hln = NULL;
			++r;
			goto done;
		    }
                }
            }

            if (xlink(hln->name, dpath, stat1->st_flags) < 0) {
		int tryrelink = (errno == EMLINK);
		logerr("%-32s hardlink: unable to link to %s: %s\n",
		    (dpath ? dpath : spath), hln->name, strerror(errno)
		);
                hltdelete(hln);
                hln = NULL;
		if (tryrelink) {
		    logerr("%-20s hardlink: will attempt to copy normally\n",
			(dpath ? dpath : spath));
		    goto relink;
		}
		++r;
            } else {
                if (hln->nlinked == stat1->st_nlink) {
                    hltdelete(hln);
		    hln = NULL;
		}
                if (r == 0) {
		    if (VerboseOpt) {
			logstd("%-32s hardlink: %s\n",
			    (dpath ? dpath : spath),
			    (st2Valid ? "relinked" : "linked")
			);
		    }
		    CountSourceItems++;
		    CountCopiedItems++;
		    r = 0;
		    goto done;
		}
            }
        } else {
	    /*
	     * first instance of hardlink must be copied normally
	     */
relink:
            hln = hltadd(stat1, dpath);
	}
    }

    /*
     * Do we need to copy the file/dir/link/whatever?  Early termination
     * if we do not.  Always traverse directories.  Always redo links.
     *
     * NOTE: st2Valid is true only if dpath != NULL *and* dpath stats good.
     */

    if (
	st2Valid
	&& stat1->st_mode == st2.st_mode
	&& FlagsMatch(stat1, &st2)
    ) {
	if (S_ISLNK(stat1->st_mode) || S_ISDIR(stat1->st_mode)) {
	    ;
	} else {
	    if (ForceOpt == 0 &&
		stat1->st_size == st2.st_size &&
		(ValidateOpt == 2 || mtimecmp(stat1, &st2) == 0) &&
		OwnerMatch(stat1, &st2)
#ifndef NOMD5
		&& (UseMD5Opt == 0 || !S_ISREG(stat1->st_mode) ||
		    (mres = md5_check(spath, dpath)) == 0)
#endif
		&& (ValidateOpt == 0 || !S_ISREG(stat1->st_mode) ||
		    validate_check(spath, dpath) == 0)
	    ) {
		/*
		 * The files are identical, but if we are running as
		 * root we might need to adjust ownership/group/flags.
		 */
		int changedown = 0;
		int changedflags = 0;

                if (hln)
		    hltsetdino(hln, st2.st_ino);

		if (!OwnerMatch(stat1, &st2)) {
		    hc_chown(&DstHost, dpath, stat1->st_uid, stat1->st_gid);
		    changedown = 1;
		}
#ifdef _ST_FLAGS_PRESENT_
		if (!FlagsMatch(stat1, &st2)) {
		    hc_chflags(&DstHost, dpath, stat1->st_flags);
		    changedflags = 1;
		}
#endif
		if (VerboseOpt >= 3) {
#ifndef NOMD5
		    if (UseMD5Opt) {
			logstd("%-32s md5-nochange",
				(dpath ? dpath : spath));
		    } else
#endif
		    if (ValidateOpt) {
			logstd("%-32s nochange (contents validated)",
				(dpath ? dpath : spath));
		    } else {
			logstd("%-32s nochange", (dpath ? dpath : spath));
		    }
		    if (changedown)
			logstd(" (uid/gid differ)");
		    if (changedflags)
			logstd(" (flags differ)");
		    logstd("\n");
		}
		CountSourceBytes += size;
		CountSourceItems++;
		r = 0;
		goto done;
	    }
	}
    }
    if (st2Valid && !S_ISDIR(stat1->st_mode) && S_ISDIR(st2.st_mode)) {
	if (SafetyOpt) {
	    logerr("%-32s SAFETY - refusing to copy file over directory\n",
		(dpath ? dpath : spath)
	    );
	    ++r;		/* XXX */
	    r = 0;
	    goto done; 		/* continue with the cpdup anyway */
	}
	if (QuietOpt == 0 || AskConfirmation) {
	    logstd("%-32s WARNING: non-directory source will blow away\n"
		   "%-32s preexisting dest directory, continuing anyway!\n",
		   ((dpath) ? dpath : spath), "");
	}
	if (dpath)
	    RemoveRecur(dpath, ddevNo, &st2);
	st2Valid = 0;
    }

    /*
     * The various comparisons failed, copy it.
     */
    if (S_ISDIR(stat1->st_mode)) {
	int skipdir = 0;

	if (dpath) {
	    if (!st2Valid || S_ISDIR(st2.st_mode) == 0) {
		if (st2Valid)
		    xremove(&DstHost, dpath);
		if (hc_mkdir(&DstHost, dpath, stat1->st_mode | 0700) != 0) {
		    logerr("%s: mkdir failed: %s\n",
			(dpath ? dpath : spath), strerror(errno));
		    r = 1;
		    skipdir = 1;
		}
		if (hc_lstat(&DstHost, dpath, &st2) != 0) {
		    if (NotForRealOpt == 0)
			    logerr("%s: lstat of newly made dir failed: %s\n",
				   (dpath ? dpath : spath), strerror(errno));
		    st2Valid = 0;
		    r = 1;
		    skipdir = 1;
		}
		else {
		    st2Valid = 1;
		    if (!OwnerMatch(stat1, &st2) &&
			hc_chown(&DstHost, dpath, stat1->st_uid, stat1->st_gid) != 0
		    ) {
			logerr("%s: chown of newly made dir failed: %s\n",
			    (dpath ? dpath : spath), strerror(errno));
			r = 1;
			/* Note that we should not set skipdir = 1 here. */
		    }
		}
		if (VerboseOpt)
		    logstd("%-32s mkdir-ok\n", (dpath ? dpath : spath));
		CountCopiedItems++;
	    } else {
		/*
		 * Directory must be scanable by root for cpdup to
		 * work.  We'll fix it later if the directory isn't
		 * supposed to be readable ( which is why we fixup
		 * st2.st_mode to match what we did ).
		 */
		if ((st2.st_mode & 0700) != 0700) {
		    hc_chmod(&DstHost, dpath, st2.st_mode | 0700);
		    st2.st_mode |= 0700;
		}
		if (VerboseOpt >= 2)
		    logstd("%s\n", dpath ? dpath : spath);
	    }
	}

	/*
	 * When copying a directory, stop if the source crosses a mount
	 * point.
	 */
	if (sdevNo != (dev_t)-1 && stat1->st_dev != sdevNo)
	    skipdir = 1;
	else
	    sdevNo = stat1->st_dev;

	/*
	 * When copying a directory, stop if the destination crosses
	 * a mount point.
	 *
	 * The target directory will have been created and stat'd
	 * for st2 if it did not previously exist.   st2Valid is left
	 * as a flag.  If the stat failed st2 will still only have its
	 * default initialization.
	 *
	 * So we simply assume here that the directory is within the
	 * current target mount if we had to create it (aka st2Valid is 0)
	 * and we leave ddevNo alone.
	 */
	if (st2Valid) {
	    if (ddevNo != (dev_t)-1 && st2.st_dev != ddevNo)
		skipdir = 1;
	    else
		ddevNo = st2.st_dev;
	}

	if (!skipdir) {
	    List *list = malloc(sizeof(List));
	    Node *node;

	    if (DirShowOpt)
		logstd("Scanning %s ...\n", spath);
	    InitList(list);
	    if (ScanDir(list, &SrcHost, spath, &CountSourceReadBytes, 0) == 0) {
		node = NULL;
		while ((node = IterateList(list, node, 0)) != NULL) {
		    char *nspath;
		    char *ndpath = NULL;

		    nspath = mprintf("%s/%s", spath, node->no_Name);
		    if (dpath)
			ndpath = mprintf("%s/%s", dpath, node->no_Name);

		    info->spath = nspath;
		    info->dpath = ndpath;
		    info->sdevNo = sdevNo;
		    info->ddevNo = ddevNo;
		    if (depth < 0)
			r += DoCopy(info, node->no_Stat, depth);
		    else
			r += DoCopy(info, node->no_Stat, depth + 1);
		    free(nspath);
		    if (ndpath)
			free(ndpath);
		    info->spath = NULL;
		    info->dpath = NULL;
		}

		/*
		 * Remove files/directories from destination that do not appear
		 * in the source.
		 */
		if (dpath && ScanDir(list, &DstHost, dpath,
				     &CountTargetReadBytes, 3) == 0) {
		    node = NULL;
		    while ((node = IterateList(list, node, 3)) != NULL) {
			/*
			 * If object does not exist in source or .cpignore
			 * then recursively remove it.
			 */
			char *ndpath;

			ndpath = mprintf("%s/%s", dpath, node->no_Name);
			RemoveRecur(ndpath, ddevNo, node->no_Stat);
			free(ndpath);
		    }
		}
	    }
	    ResetList(list);
	    free(list);
	}

	if (dpath && st2Valid) {
	    struct timeval tv[2];

	    if (ForceOpt || !OwnerMatch(stat1, &st2))
		hc_chown(&DstHost, dpath, stat1->st_uid, stat1->st_gid);
	    if (stat1->st_mode != st2.st_mode)
		hc_chmod(&DstHost, dpath, stat1->st_mode);
#ifdef _ST_FLAGS_PRESENT_
	    if (!FlagsMatch(stat1, &st2))
		hc_chflags(&DstHost, dpath, stat1->st_flags);
#endif
	    if (ForceOpt || mtimecmp(stat1, &st2) != 0) {
		bzero(tv, sizeof(tv));
		tv[0].tv_sec = stat1->st_mtime;
		tv[1].tv_sec = stat1->st_mtime;
#if defined(st_mtime)  /* A macro, so very likely on modern POSIX */
		tv[0].tv_usec = stat1->st_mtim.tv_nsec / 1000;
		tv[1].tv_usec = stat1->st_mtim.tv_nsec / 1000;
#endif
		hc_utimes(&DstHost, dpath, tv);
	    }
	}
    } else if (dpath == NULL) {
	/*
	 * If dpath is NULL, we are just updating the MD5
	 */
#ifndef NOMD5
	if (UseMD5Opt && S_ISREG(stat1->st_mode)) {
	    mres = md5_check(spath, NULL);

	    if (VerboseOpt > 1) {
		if (mres < 0)
		    logstd("%-32s md5-update\n", (dpath) ? dpath : spath);
		else
		    logstd("%-32s md5-ok\n", (dpath) ? dpath : spath);
	    } else if (!QuietOpt && mres < 0) {
		logstd("%-32s md5-update\n", (dpath) ? dpath : spath);
	    }
	}
#endif
    } else if (S_ISREG(stat1->st_mode)) {
	char *path;
	char *hpath;
	int fd1;
	int fd2;

	if (st2Valid)
		path = mprintf("%s.tmp%d", dpath, (int)getpid());
	else
		path = mprintf("%s", dpath);

	/*
	 * Handle check failure message.
	 */
#ifndef NOMD5
	if (mres < 0)
	    logerr("%-32s md5-CHECK-FAILED\n", (dpath) ? dpath : spath);
#endif

	/*
	 * Not quite ready to do the copy yet.  If UseHLPath is defined,
	 * see if we can hardlink instead.
	 *
	 * If we can hardlink, and the target exists, we have to remove it
	 * first or the hardlink will fail.  This can occur in a number of
	 * situations but most typically when the '-f -H' combination is
	 * used.
	 */
	if (UseHLPath && (hpath = checkHLPath(stat1, spath, dpath)) != NULL) {
		if (st2Valid)
			xremove(&DstHost, dpath);
		if (hc_link(&DstHost, hpath, dpath) == 0) {
			++CountLinkedItems;
			if (VerboseOpt) {
			    logstd("%-32s hardlinked(-H)\n",
				   (dpath ? dpath : spath));
			}
			free(hpath);
			goto skip_copy;
		}
		/*
		 * Shucks, we may have hit a filesystem hard linking limit,
		 * we have to copy instead.
		 */
		free(hpath);
	}

	if ((fd1 = hc_open(&SrcHost, spath, O_RDONLY, 0)) >= 0) {
	    if ((fd2 = hc_open(&DstHost, path, O_WRONLY|O_CREAT|O_EXCL, 0600)) < 0) {
		/*
		 * There could be a .tmp file from a previously interrupted
		 * run, delete and retry.  Fail if we still can't get at it.
		 */
#ifdef _ST_FLAGS_PRESENT_
		hc_chflags(&DstHost, path, 0);
#endif
		hc_remove(&DstHost, path);
		fd2 = hc_open(&DstHost, path, O_WRONLY|O_CREAT|O_EXCL|O_TRUNC, 0600);
	    }
	    if (fd2 >= 0) {
		const char *op;
		char *iobuf1 = malloc(GETIOSIZE);
		int n;

		/*
		 * Matt: What about holes?
		 */
		op = "read";
		while ((n = hc_read(&SrcHost, fd1, iobuf1, GETIOSIZE)) > 0) {
		    op = "write";
		    if (hc_write(&DstHost, fd2, iobuf1, n) != n)
			break;
		    op = "read";
		}
		hc_close(&DstHost, fd2);
		if (n == 0) {
		    struct timeval tv[2];

		    bzero(tv, sizeof(tv));
		    tv[0].tv_sec = stat1->st_mtime;
		    tv[1].tv_sec = stat1->st_mtime;
#if defined(st_mtime)
		    tv[0].tv_usec = stat1->st_mtim.tv_nsec / 1000;
		    tv[1].tv_usec = stat1->st_mtim.tv_nsec / 1000;
#endif

		    if (DstRootPrivs || ChgrpAllowed(stat1->st_gid))
			hc_chown(&DstHost, path, stat1->st_uid, stat1->st_gid);
		    hc_chmod(&DstHost, path, stat1->st_mode);
#ifdef _ST_FLAGS_PRESENT_
		    if (stat1->st_flags & (UF_IMMUTABLE|SF_IMMUTABLE))
			hc_utimes(&DstHost, path, tv);
#else
		    hc_utimes(&DstHost, path, tv);
#endif
		    if (st2Valid && xrename(path, dpath, st2_flags) != 0) {
			logerr("%-32s rename-after-copy failed: %s\n",
			    (dpath ? dpath : spath), strerror(errno)
			);
			xremove(&DstHost, path);
			++r;
		    } else {
			if (VerboseOpt)
			    logstd("%-32s copy-ok\n", (dpath ? dpath : spath));
#ifdef _ST_FLAGS_PRESENT_
			if (DstRootPrivs ? stat1->st_flags : stat1->st_flags & UF_SETTABLE)
			    hc_chflags(&DstHost, dpath, stat1->st_flags);
#endif
		    }
#ifdef _ST_FLAGS_PRESENT_
		    if ((stat1->st_flags & (UF_IMMUTABLE|SF_IMMUTABLE)) == 0)
			hc_utimes(&DstHost, dpath, tv);
#endif
		    CountSourceReadBytes += size;
		    CountWriteBytes += size;
		    CountSourceBytes += size;
		    CountSourceItems++;
		    CountCopiedItems++;
		} else {
		    logerr("%-32s %s failed: %s\n",
			(dpath ? dpath : spath), op, strerror(errno)
		    );
		    hc_remove(&DstHost, path);
		    ++r;
		}
		free(iobuf1);
	    } else {
		logerr("%-32s create (uid %d, euid %d) failed: %s\n",
		    (dpath ? dpath : spath), getuid(), geteuid(),
		    strerror(errno)
		);
		++r;
	    }
	    hc_close(&SrcHost, fd1);
	} else {
	    logerr("%-32s copy: open failed: %s\n",
		(dpath ? dpath : spath),
		strerror(errno)
	    );
	    ++r;
	}
skip_copy:
	free(path);

        if (hln) {
            if (!r && hc_stat(&DstHost, dpath, &st2) == 0) {
		hltsetdino(hln, st2.st_ino);
	    } else {
                hltdelete(hln);
		hln = NULL;
	    }
        }
    } else if (S_ISLNK(stat1->st_mode)) {
	char *link1 = malloc(GETLINKSIZE);
	char *link2 = malloc(GETLINKSIZE);
	char *path;
	int n1;
	int n2;

	n1 = hc_readlink(&SrcHost, spath, link1, GETLINKSIZE - 1);
	if (st2Valid) {
		path = mprintf("%s.tmp%d", dpath, (int)getpid());
		n2 = hc_readlink(&DstHost, dpath, link2, GETLINKSIZE - 1);
	} else {
		path = mprintf("%s", dpath);
		n2 = -1;
	}
	if (n1 >= 0) {
	    if (ForceOpt || n1 != n2 || bcmp(link1, link2, n1) != 0 ||
		(st2Valid && symlink_mfo_test(&DstHost, stat1, &st2))
	    ) {
		struct timeval tv[2];

		bzero(tv, sizeof(tv));
		tv[0].tv_sec = stat1->st_mtime;
		tv[1].tv_sec = stat1->st_mtime;
#if defined(st_mtime)
		tv[0].tv_usec = stat1->st_mtim.tv_nsec / 1000;
		tv[1].tv_usec = stat1->st_mtim.tv_nsec / 1000;
#endif

		hc_umask(&DstHost, ~stat1->st_mode);
		xremove(&DstHost, path);
		link1[n1] = 0;
		if (hc_symlink(&DstHost, link1, path) < 0) {
                      logerr("%-32s symlink (%s->%s) failed: %s\n",
			  (dpath ? dpath : spath), link1, path,
			  strerror(errno)
		      );
		      ++r;
		} else {
		    if (DstRootPrivs || ChgrpAllowed(stat1->st_gid))
			hc_lchown(&DstHost, path, stat1->st_uid, stat1->st_gid);

		    /*
		     * lutimes, lchmod if supported by destination.
		     */
		    if (DstHost.version >= HCPROTO_VERSION_LUCC) {
			hc_lchmod(&DstHost, path, stat1->st_mode);
			hc_lutimes(&DstHost, path, tv);
		    }

		    /*
		     * rename (and set flags if supported by destination)
		     */
		    if (st2Valid && xrename(path, dpath, st2_flags) != 0) {
			logerr("%-32s rename softlink (%s->%s) failed: %s\n",
			    (dpath ? dpath : spath),
			    path, dpath, strerror(errno));
			xremove(&DstHost, path);
		    } else {
#ifdef _ST_FLAGS_PRESENT_
			if (DstHost.version >= HCPROTO_VERSION_LUCC)
			    hc_lchflags(&DstHost, dpath, stat1->st_flags);
#endif
			if (VerboseOpt) {
			    logstd("%-32s softlink-ok\n",
				   (dpath ? dpath : spath));
			}
		    }
		    hc_umask(&DstHost, 000);
		    CountWriteBytes += n1;
		    CountCopiedItems++;
		}
	    } else {
		if (VerboseOpt >= 3)
		    logstd("%-32s nochange", (dpath ? dpath : spath));
		if (!OwnerMatch(stat1, &st2)) {
		    hc_lchown(&DstHost, dpath, stat1->st_uid, stat1->st_gid);
		    if (VerboseOpt >= 3)
			logstd(" (uid/gid differ)");
		}
		if (VerboseOpt >= 3)
		    logstd("\n");
	    }
	    CountSourceBytes += n1;
	    CountSourceReadBytes += n1;
	    if (n2 > 0)
		CountTargetReadBytes += n2;
	    CountSourceItems++;
	} else {
	    r = 1;
	    logerr("%-32s softlink-failed\n", (dpath ? dpath : spath));
	}
	free(link1);
	free(link2);
	free(path);
    } else if ((S_ISCHR(stat1->st_mode) || S_ISBLK(stat1->st_mode)) && DeviceOpt) {
	char *path = NULL;

	if (ForceOpt ||
	    st2Valid == 0 ||
	    stat1->st_mode != st2.st_mode ||
	    stat1->st_rdev != st2.st_rdev ||
	    !OwnerMatch(stat1, &st2)
	) {
	    if (st2Valid) {
		path = mprintf("%s.tmp%d", dpath, (int)getpid());
		xremove(&DstHost, path);
	    } else {
		path = mprintf("%s", dpath);
	    }

	    if (hc_mknod(&DstHost, path, stat1->st_mode, stat1->st_rdev) == 0) {
		hc_chmod(&DstHost, path, stat1->st_mode);
		hc_chown(&DstHost, path, stat1->st_uid, stat1->st_gid);
		if (st2Valid)
			xremove(&DstHost, dpath);
		if (st2Valid && xrename(path, dpath, st2_flags) != 0) {
		    logerr("%-32s dev-rename-after-create failed: %s\n",
			(dpath ? dpath : spath),
			strerror(errno)
		    );
		} else if (VerboseOpt) {
		    logstd("%-32s dev-ok\n", (dpath ? dpath : spath));
		}
		CountCopiedItems++;
	    } else {
		r = 1;
		logerr("%-32s dev failed: %s\n",
		    (dpath ? dpath : spath), strerror(errno)
		);
	    }
	} else {
	    if (VerboseOpt >= 3)
		logstd("%-32s nochange\n", (dpath ? dpath : spath));
	}
	if (path)
		free(path);
	CountSourceItems++;
    }
done:
    if (hln) {
	if (hln->dino == (ino_t)-1) {
	    hltdelete(hln);
	    /*hln = NULL; unneeded */
	} else {
	    hltrels(hln);
	}
    }
    return (r);
}

int
ScanDir(List *list, struct HostConf *host, const char *path,
	int64_t *CountReadBytes, int n)
{
    DIR *dir;
    struct HostConf *cphost;
    struct HCDirEntry *den;
    struct stat *statptr;

    if (n == 0) {
	/*
	 * scan .cpignore file for files/directories to ignore
	 * (only in the source directory, i.e. if n == 0).
	 */
	if (UseCpFile) {
	    int fd;
	    int nread;
	    int bufused;
	    char *buf = malloc(GETBUFSIZE);
	    char *nl, *next;
	    char *fpath;

	    if (UseCpFile[0] == '/') {
		fpath = mprintf("%s", UseCpFile);
		cphost = NULL;
	    } else {
		fpath = mprintf("%s/%s", path, UseCpFile);
		AddList(list, strrchr(fpath, '/') + 1, 1, NULL);
		cphost = host;
	    }
	    fd = hc_open(cphost, fpath, O_RDONLY, 0);
	    if (fd >= 0) {
		bufused = 0;
		while ((nread = hc_read(cphost, fd, buf + bufused,
			GETBUFSIZE - bufused - 1)) > 0) {
		    *CountReadBytes += nread;
		    bufused += nread;
		    buf[bufused] = 0;
		    for (next = buf; (nl = strchr(next, '\n')); next = nl+1) {
			*nl = 0;
			AddList(list, next, 1, NULL);
		    }
		    bufused = strlen(next);
		    if (bufused)
			bcopy(next, buf, bufused);
		}
		if (bufused) {
		    /* last line has no trailing newline */
		    buf[bufused] = 0;
		    AddList(list, buf, 1, NULL);
		}
		hc_close(cphost, fd);
	    }
	    free(fpath);
	    free(buf);
	}

	/*
	 * Automatically exclude MD5CacheFile that we create on the
	 * source from the copy to the destination.
	 */
	if (UseMD5Opt)
	    AddList(list, MD5CacheFile, 1, NULL);
    }

    if ((dir = hc_opendir(host, path)) == NULL)
	return (1);
    while ((den = hc_readdir(host, dir, &statptr)) != NULL) {
	/*
	 * ignore . and ..
	 */
	if (strcmp(den->d_name, ".") != 0 && strcmp(den->d_name, "..") != 0) {
	     if (UseCpFile && UseCpFile[0] == '/') {
		 if (CheckList(list, path, den->d_name) == 0)
			continue;
	     }
	     AddList(list, den->d_name, n, statptr);
	}
    }
    hc_closedir(host, dir);

    return (0);
}

/*
 * RemoveRecur()
 */

static void
RemoveRecur(const char *dpath, dev_t devNo, struct stat *dstat)
{
    struct stat st;

    if (dstat == NULL) {
	if (hc_lstat(&DstHost, dpath, &st) == 0)
	    dstat = &st;
    }
    if (dstat != NULL) {
	if (devNo == (dev_t)-1)
	    devNo = dstat->st_dev;
	if (dstat->st_dev == devNo) {
	    if (S_ISDIR(dstat->st_mode)) {
		DIR *dir;

		if ((dir = hc_opendir(&DstHost, dpath)) != NULL) {
		    List *list = malloc(sizeof(List));
		    Node *node = NULL;
		    struct HCDirEntry *den;

		    InitList(list);
		    while ((den = hc_readdir(&DstHost, dir, &dstat)) != NULL) {
			if (strcmp(den->d_name, ".") == 0)
			    continue;
			if (strcmp(den->d_name, "..") == 0)
			    continue;
			AddList(list, den->d_name, 3, dstat);
		    }
		    hc_closedir(&DstHost, dir);
		    while ((node = IterateList(list, node, 3)) != NULL) {
			char *ndpath;

			ndpath = mprintf("%s/%s", dpath, node->no_Name);
			RemoveRecur(ndpath, devNo, node->no_Stat);
			free(ndpath);
		    }
		    ResetList(list);
		    free(list);
		}
		if (AskConfirmation && NoRemoveOpt == 0) {
		    if (YesNo(dpath)) {
			if (xrmdir(&DstHost, dpath) < 0) {
			    logerr("%-32s rmdir failed: %s\n",
				dpath, strerror(errno)
			    );
			}
			CountRemovedItems++;
		    }
		} else {
		    if (NoRemoveOpt) {
			if (VerboseOpt)
			    logstd("%-32s not-removed\n", dpath);
		    } else if (xrmdir(&DstHost, dpath) == 0) {
			if (VerboseOpt)
			    logstd("%-32s rmdir-ok\n", dpath);
			CountRemovedItems++;
		    } else {
			logerr("%-32s rmdir failed: %s\n",
			    dpath, strerror(errno)
			);
		    }
		}
	    } else {
		if (AskConfirmation && NoRemoveOpt == 0) {
		    if (YesNo(dpath)) {
			if (xremove(&DstHost, dpath) < 0) {
			    logerr("%-32s remove failed: %s\n",
				dpath, strerror(errno)
			    );
			}
			CountRemovedItems++;
		    }
		} else {
		    if (NoRemoveOpt) {
			if (VerboseOpt)
			    logstd("%-32s not-removed\n", dpath);
		    } else if (xremove(&DstHost, dpath) == 0) {
			if (VerboseOpt)
			    logstd("%-32s remove-ok\n", dpath);
			CountRemovedItems++;
		    } else {
			logerr("%-32s remove failed: %s\n",
			    dpath, strerror(errno)
			);
		    }
		}
	    }
	}
    }
}

static void
InitList(List *list)
{
    bzero(list, sizeof(List));
    list->li_Node.no_Next = &list->li_Node;
}

static void
ResetList(List *list)
{
    Node *node;

    while ((node = list->li_Node.no_Next) != &list->li_Node) {
	list->li_Node.no_Next = node->no_Next;
	if (node->no_Stat != NULL)
	    free(node->no_Stat);
	free(node);
    }
    InitList(list);
}

static Node *
IterateList(List *list, Node *node, int n)
{
    if (node == NULL)
	node = list->li_Node.no_Next;
    else
	node = node->no_Next;
    while (node->no_Value != n && node != &list->li_Node)
	node = node->no_Next;
    return (node == &list->li_Node ? NULL : node);
}

static int
AddList(List *list, const char *name, int n, struct stat *st)
{
    Node *node;
    int hv;

    /*
     * Scan against wildcards.  Only a node value of 1 can be a wildcard
     * ( usually scanned from .cpignore )
     */
    for (node = list->li_Hash[0]; node; node = node->no_HNext) {
	if (strcmp(name, node->no_Name) == 0 ||
	    (n != 1 && node->no_Value == 1 &&
	    fnmatch(node->no_Name, name, 0) == 0)
	) {
	    return(node->no_Value);
	}
    }

    /*
     * Look for exact match
     */

    hv = shash(name);
    for (node = list->li_Hash[hv]; node; node = node->no_HNext) {
	if (strcmp(name, node->no_Name) == 0) {
	    return(node->no_Value);
	}
    }
    node = malloc(sizeof(Node) + strlen(name) + 1);
    if (node == NULL)
	fatal("out of memory");

    node->no_Next = list->li_Node.no_Next;
    list->li_Node.no_Next = node;

    node->no_HNext = list->li_Hash[hv];
    list->li_Hash[hv] = node;

    strcpy(node->no_Name, name);
    node->no_Value = n;
    node->no_Stat = st;

    return(n);
}

/*
 * Match against n=1 (cpignore) entries
 *
 * Returns 0 on match, non-zero if no match
 */
static int
CheckList(List *list, const char *path, const char *name)
{
    char *fpath = NULL;
    Node *node;
    int hv;

    if (asprintf(&fpath, "%s/%s", path, name) < 0)
	fatal("out of memory");

    /*
     * Scan against wildcards.  Only a node value of 1 can be a wildcard
     * ( usually scanned from .cpignore )
     */
    for (node = list->li_Hash[0]; node; node = node->no_HNext) {
	if (node->no_Value != 1)
		continue;
	if (fnmatch(node->no_Name, fpath, 0) == 0) {
		free(fpath);
		return 0;
	}
    }

    /*
     * Look for exact match
     */
    hv = shash(fpath);
    for (node = list->li_Hash[hv]; node; node = node->no_HNext) {
	if (node->no_Value != 1)
		continue;
	if (strcmp(fpath, node->no_Name) == 0) {
		free(fpath);
		return 0;
	}
    }

    free(fpath);
    return 1;
}

static int
shash(const char *s)
{
    int hv;

    hv = 0xA4FB3255;

    while (*s) {
	if (*s == '*' || *s == '?' ||
	    *s == '{' || *s == '}' ||
	    *s == '[' || *s == ']' ||
	    *s == '|'
	) {
	    return(0);
	}
	hv = (hv << 5) ^ *s ^ (hv >> 23);
	++s;
    }
    return(((hv >> 16) ^ hv) & HMASK);
}

static int
YesNo(const char *path)
{
    int ch, first;

    fprintf(stderr, "remove %s (Yes/No) [No]? ", path);
    fflush(stderr);

    first = ch = getchar();
    while (ch != '\n' && ch != EOF)
	ch = getchar();
    return ((first == 'y' || first == 'Y'));
}

/*
 * xrename() - rename with override
 *
 *	If the rename fails, attempt to override st_flags on the
 *	destination and rename again.  If that fails too, try to
 *	set the flags back the way they were and give up.
 */

static int
xrename(const char *src, const char *dst, u_long flags)
{
    int r;

    if ((r = hc_rename(&DstHost, src, dst)) < 0) {
#ifdef _ST_FLAGS_PRESENT_
	if (DstHost.version >= HCPROTO_VERSION_LUCC)
	    hc_lchflags(&DstHost, dst, 0);
	else
	    hc_chflags(&DstHost, dst, 0);

	if ((r = hc_rename(&DstHost, src, dst)) < 0) {
	    if (DstHost.version >= HCPROTO_VERSION_LUCC)
		hc_lchflags(&DstHost, dst, flags);
	    else
		hc_chflags(&DstHost, dst, flags);
	}
#endif
    }
    return(r);
}

static int
xlink(const char *src, const char *dst, u_long flags)
{
    int r;
#ifdef _ST_FLAGS_PRESENT_
    int e;
#endif

    if ((r = hc_link(&DstHost, src, dst)) < 0) {
#ifdef _ST_FLAGS_PRESENT_
	if (DstHost.version >= HCPROTO_VERSION_LUCC)
	    hc_lchflags(&DstHost, src, 0);
	else
	    hc_chflags(&DstHost, src, 0);
	r = hc_link(&DstHost, src, dst);
	e = errno;
	hc_chflags(&DstHost, src, flags);
	errno = e;
#endif
    }
    if (r == 0)
	    ++CountLinkedItems;
    return(r);
}

static int
xremove(struct HostConf *host, const char *path)
{
    int res;

    res = hc_remove(host, path);
#ifdef _ST_FLAGS_PRESENT_
    if (res == -EPERM) {
	if (host->version >= HCPROTO_VERSION_LUCC)
	    hc_lchflags(host, path, 0);
	else
	    hc_chflags(host, path, 0);
	res = hc_remove(host, path);
    }
#endif
    return(res);
}

static int
xrmdir(struct HostConf *host, const char *path)
{
    int res;

    res = hc_rmdir(host, path);
#ifdef _ST_FLAGS_PRESENT_
    if (res == -EPERM) {
	hc_chflags(host, path, 0);
	res = hc_rmdir(host, path);
    }
#endif
    return(res);
}

/*
 * Compare mtimes.  By default cpdup only compares the seconds field
 * because different operating systems and filesystems will store time
 * fields with varying amounts of precision.
 *
 * This subroutine can be adjusted to also compare to microseconds or
 * nanoseconds precision.  However, since cpdup() uses utimes() to
 * set a file's timestamp and utimes() only takes timeval's (usec precision),
 * I strongly recommend only comparing down to usec precision at best.
 */
static int
mtimecmp(struct stat *st1, struct stat *st2)
{
    if (st1->st_mtime < st2->st_mtime)
	return -1;
    if (st1->st_mtime == st2->st_mtime)
	return 0;
    return 1;
}

/*
 * Check to determine if a symlink's mtime, flags, or mode differ.
 *
 * This is only supported on targets that support lchflags, lutimes,
 * and lchmod.
 */
static int
symlink_mfo_test(struct HostConf *hc, struct stat *st1, struct stat *st2)
{
    int res = 0;

    if (hc->version >= HCPROTO_VERSION_LUCC) {
	if (!FlagsMatch(st1, st2))
	    res = 1;
	if (mtimecmp(st1, st2) != 0)
	    res = 1;
	if (st1->st_mode != st2->st_mode)
	    res = 1;
    }
    return res;
}
