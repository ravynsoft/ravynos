/*
 * HCPROTO.C
 *
 * This module implements a simple remote control protocol
 */

#include "cpdup.h"
#include "hclink.h"
#include "hcproto.h"

static int hc_decode_stat(hctransaction_t trans, struct stat *, struct HCHead *);
static int hc_decode_stat_item(struct stat *st, struct HCLeaf *item);
static int rc_encode_stat(hctransaction_t trans, struct stat *);

static int rc_hello(hctransaction_t trans, struct HCHead *);
static int rc_stat(hctransaction_t trans, struct HCHead *);
static int rc_lstat(hctransaction_t trans, struct HCHead *);
static int rc_opendir(hctransaction_t trans, struct HCHead *);
static int rc_readdir(hctransaction_t trans, struct HCHead *);
static int rc_closedir(hctransaction_t trans, struct HCHead *);
static int rc_scandir(hctransaction_t trans, struct HCHead *);
static int rc_open(hctransaction_t trans, struct HCHead *);
static int rc_close(hctransaction_t trans, struct HCHead *);
static int rc_read(hctransaction_t trans, struct HCHead *);
static int rc_readfile(hctransaction_t trans, struct HCHead *);
static int rc_write(hctransaction_t trans, struct HCHead *);
static int rc_remove(hctransaction_t trans, struct HCHead *);
static int rc_mkdir(hctransaction_t trans, struct HCHead *);
static int rc_rmdir(hctransaction_t trans, struct HCHead *);
static int rc_chown(hctransaction_t trans, struct HCHead *);
static int rc_lchown(hctransaction_t trans, struct HCHead *);
static int rc_chmod(hctransaction_t trans, struct HCHead *);
static int rc_mknod(hctransaction_t trans, struct HCHead *);
static int rc_link(hctransaction_t trans, struct HCHead *);
#ifdef _ST_FLAGS_PRESENT_
static int rc_chflags(hctransaction_t trans, struct HCHead *);
#endif
static int rc_readlink(hctransaction_t trans, struct HCHead *);
static int rc_umask(hctransaction_t trans, struct HCHead *);
static int rc_symlink(hctransaction_t trans, struct HCHead *);
static int rc_rename(hctransaction_t trans, struct HCHead *);
static int rc_utimes(hctransaction_t trans, struct HCHead *);
static int rc_geteuid(hctransaction_t trans, struct HCHead *);
static int rc_getgroups(hctransaction_t trans, struct HCHead *);

static int getmygroups(gid_t **gidlist);

static int silentwarning(int *, const char *, ...) __printflike(2, 3);

static struct HCDesc HCDispatchTable[] = {
    { HC_HELLO,		rc_hello },
    { HC_STAT,		rc_stat },
    { HC_LSTAT,		rc_lstat },
    { HC_OPENDIR,	rc_opendir },
    { HC_READDIR,	rc_readdir },
    { HC_CLOSEDIR,	rc_closedir },
    { HC_OPEN,		rc_open },
    { HC_CLOSE,		rc_close },
    { HC_READ,		rc_read },
    { HC_WRITE,		rc_write },
    { HC_REMOVE,	rc_remove },
    { HC_MKDIR,		rc_mkdir },
    { HC_RMDIR,		rc_rmdir },
    { HC_CHOWN,		rc_chown },
    { HC_LCHOWN,	rc_lchown },
    { HC_CHMOD,		rc_chmod },
    { HC_MKNOD,		rc_mknod },
    { HC_LINK,		rc_link },
#ifdef _ST_FLAGS_PRESENT_
    { HC_CHFLAGS,	rc_chflags },
#endif
    { HC_READLINK,	rc_readlink },
    { HC_UMASK,		rc_umask },
    { HC_SYMLINK,	rc_symlink },
    { HC_RENAME,	rc_rename },
    { HC_UTIMES,	rc_utimes },
    { HC_GETEUID,	rc_geteuid },
    { HC_GETGROUPS,	rc_getgroups },
    { HC_SCANDIR,	rc_scandir },
    { HC_READFILE,	rc_readfile },
    { HC_LUTIMES,	rc_utimes },
#ifdef _ST_FLAGS_PRESENT_
    { HC_LCHFLAGS,	rc_chflags },
#endif
    { HC_LCHMOD,	rc_chmod },
};

static int chown_warning;
#ifdef _ST_FLAGS_PRESENT_
static int chflags_warning;
#endif

/*
 * If not running as root generate a silent warning and return no error.
 *
 * If running as root return an error.
 */
static int
silentwarning(int *didwarn, const char *ctl, ...)
{
    va_list va;

    if (DstRootPrivs)
	return(-1);
    if (*didwarn == 0 && QuietOpt == 0) {
	*didwarn = 1;
	fprintf(stderr, "WARNING: Not running as root, ");
	va_start(va, ctl);
	vfprintf(stderr, ctl, va);
	va_end(va);
    }
    return(0);
}

int
hc_connect(struct HostConf *hc, int readonly)
{
    if (hcc_connect(hc, readonly) < 0) {
	fprintf(stderr, "Unable to connect to %s\n", hc->host);
	return(-1);
    }
    return(hc_hello(hc));
}

void
hc_slave(int fdin, int fdout)
{
    hcc_slave(fdin, fdout, HCDispatchTable,
	      sizeof(HCDispatchTable) / sizeof(HCDispatchTable[0]));
}

/*
 * A HELLO RPC is sent on the initial connect.
 */
int
hc_hello(struct HostConf *hc)
{
    struct HCHead *head;
    struct HCLeaf *item;
    hctransaction_t trans;
    char hostbuf[256];
    int error;

    bzero(hostbuf, sizeof(hostbuf));
    if (gethostname(hostbuf, sizeof(hostbuf) - 1) < 0)
	return(-1);
    if (hostbuf[0] == 0)
	hostbuf[0] = '?';

    trans = hcc_start_command(hc, HC_HELLO);
    hcc_leaf_string(trans, LC_HELLOSTR, hostbuf);
    hcc_leaf_int32(trans, LC_VERSION, HCPROTO_VERSION);
    if (UseCpFile)
	hcc_leaf_string(trans, LC_PATH1, UseCpFile);
    if ((head = hcc_finish_command(trans)) == NULL) {
	fprintf(stderr, "Connected to %s but remote failed to complete hello\n",
		hc->host);
	return(-1);
    }

    if (head->error) {
	fprintf(stderr, "Connected to %s but remote returned error %d\n",
		hc->host, head->error);
	return(-1);
    }

    error = -1;
    FOR_EACH_ITEM(item, trans, head) {
	switch(item->leafid) {
	case LC_HELLOSTR:
	    if (QuietOpt == 0)
		fprintf(stderr, "Handshaked with %s\n", HCC_STRING(item));
	    error = 0;
	    break;
	case LC_VERSION:
	    hc->version = HCC_INT32(item);
	    break;
	}
    }
    if (hc->version < HCPROTO_VERSION_COMPAT) {
	fprintf(stderr, "Remote cpdup at %s has an incompatible version\n",
		hc->host);
	error = -1;
    } else if (hc->version < HCPROTO_VERSION && QuietOpt == 0) {
	fprintf(stderr,
		"WARNING: Remote cpdup at %s has a lower version,\n"
		"expect reduced speed and/or functionality\n", hc->host);
    }
    if (error < 0)
	fprintf(stderr, "Handshake failed with %s\n", hc->host);
    return (error);
}

static int
rc_hello(hctransaction_t trans, struct HCHead *head)
{
    struct HCLeaf *item;
    char hostbuf[256];

    FOR_EACH_ITEM(item, trans, head) {
	if (item->leafid == LC_PATH1)
	    UseCpFile = strdup(HCC_STRING(item));
    }

    bzero(hostbuf, sizeof(hostbuf));
    if (gethostname(hostbuf, sizeof(hostbuf) - 1) < 0)
	return(-1);
    if (hostbuf[0] == 0)
	hostbuf[0] = '?';

    hcc_leaf_string(trans, LC_HELLOSTR, hostbuf);
    hcc_leaf_int32(trans, LC_VERSION, HCPROTO_VERSION);
    return(0);
}

/*
 * STAT, LSTAT
 */
int
hc_stat(struct HostConf *hc, const char *path, struct stat *st)
{
    struct HCHead *head;
    hctransaction_t trans;

    if (hc == NULL || hc->host == NULL)
	return(stat(path, st));

    trans = hcc_start_command(hc, HC_STAT);
    hcc_leaf_string(trans, LC_PATH1, path);
    if ((head = hcc_finish_command(trans)) == NULL)
	return(-1);
    if (head->error)
	return(-1);
    return(hc_decode_stat(trans, st, head));
}

int
hc_lstat(struct HostConf *hc, const char *path, struct stat *st)
{
    struct HCHead *head;
    hctransaction_t trans;

    if (hc == NULL || hc->host == NULL)
	return(lstat(path, st));

    trans = hcc_start_command(hc, HC_LSTAT);
    hcc_leaf_string(trans, LC_PATH1, path);
    if ((head = hcc_finish_command(trans)) == NULL)
	return(-1);
    if (head->error)
	return(-1);
    return(hc_decode_stat(trans, st, head));
}

static int
hc_decode_stat(hctransaction_t trans, struct stat *st, struct HCHead *head)
{
    struct HCLeaf *item;

    bzero(st, sizeof(*st));
    FOR_EACH_ITEM(item, trans, head)
	hc_decode_stat_item(st, item);
    return(0);
}

static int
hc_decode_stat_item(struct stat *st, struct HCLeaf *item)
{
    switch(item->leafid) {
    case LC_DEV:
	st->st_dev = HCC_INT32(item);
	break;
    case LC_INO:
	st->st_ino = HCC_INT64(item);
	break;
    case LC_MODE:
	st->st_mode = HCC_INT32(item);
	break;
    case LC_NLINK:
	st->st_nlink = HCC_INT32(item);
	break;
    case LC_UID:
	st->st_uid = HCC_INT32(item);
	break;
    case LC_GID:
	st->st_gid = HCC_INT32(item);
	break;
    case LC_RDEV:
	st->st_rdev = HCC_INT32(item);
	break;
    case LC_ATIME:
	st->st_atime = (time_t)HCC_INT64(item);
	break;
    case LC_MTIME:
	st->st_mtime = (time_t)HCC_INT64(item);
	break;
    case LC_CTIME:
	st->st_ctime = (time_t)HCC_INT64(item);
	break;
#if defined(st_atime)  /* A macro, so very likely on modern POSIX */
    case LC_ATIMENSEC:
	st->st_atim.tv_nsec = HCC_INT32(item);
	break;
    case LC_MTIMENSEC:
	st->st_mtim.tv_nsec = HCC_INT32(item);
	break;
    case LC_CTIMENSEC:
	st->st_ctim.tv_nsec = HCC_INT32(item);
	break;
#endif
    case LC_FILESIZE:
	st->st_size = HCC_INT64(item);
	break;
    case LC_FILEBLKS:
	st->st_blocks = HCC_INT64(item);
	break;
    case LC_BLKSIZE:
	st->st_blksize = HCC_INT32(item);
	break;
#ifdef _ST_FLAGS_PRESENT_
    case LC_FILEFLAGS:
	st->st_flags = (uint32_t)HCC_INT64(item);
	break;
#endif
    }
    return(0);
}

static int
rc_stat(hctransaction_t trans, struct HCHead *head)
{
    struct HCLeaf *item;
    struct stat st;
    const char *path = NULL;

    FOR_EACH_ITEM(item, trans, head) {
	if (item->leafid == LC_PATH1)
	    path = HCC_STRING(item);
    }
    if (path == NULL)
	return(-2);
    if (stat(path, &st) < 0)
	return(-1);
    return (rc_encode_stat(trans, &st));
}

static int
rc_lstat(hctransaction_t trans, struct HCHead *head)
{
    struct HCLeaf *item;
    struct stat st;
    const char *path = NULL;

    FOR_EACH_ITEM(item, trans, head) {
	if (item->leafid == LC_PATH1)
	    path = HCC_STRING(item);
    }
    if (path == NULL)
	return(-2);
    if (lstat(path, &st) < 0)
	return(-1);
    return (rc_encode_stat(trans, &st));
}

/*
 * Encode all entries of a stat structure.
 *
 * CAUTION:  If you add any more entries here, be sure to
 *           increase the STAT_MAX_NUM_ENTRIES value!
 */
#define STAT_MAX_NUM_ENTRIES 18
static int
rc_encode_stat(hctransaction_t trans, struct stat *st)
{
    hcc_leaf_int32(trans, LC_DEV, st->st_dev);
    hcc_leaf_int64(trans, LC_INO, st->st_ino);
    hcc_leaf_int32(trans, LC_MODE, st->st_mode);
    hcc_leaf_int32(trans, LC_NLINK, st->st_nlink);
    hcc_leaf_int32(trans, LC_UID, st->st_uid);
    hcc_leaf_int32(trans, LC_GID, st->st_gid);
    hcc_leaf_int32(trans, LC_RDEV, st->st_rdev);
    hcc_leaf_int64(trans, LC_ATIME, st->st_atime);
    hcc_leaf_int64(trans, LC_MTIME, st->st_mtime);
    hcc_leaf_int64(trans, LC_CTIME, st->st_ctime);
#if defined(st_atime)
    hcc_leaf_int32(trans, LC_ATIMENSEC, st->st_atim.tv_nsec);
    hcc_leaf_int32(trans, LC_MTIMENSEC, st->st_mtim.tv_nsec);
    hcc_leaf_int32(trans, LC_CTIMENSEC, st->st_ctim.tv_nsec);
#endif
    hcc_leaf_int64(trans, LC_FILESIZE, st->st_size);
    hcc_leaf_int64(trans, LC_FILEBLKS, st->st_blocks);
    hcc_leaf_int32(trans, LC_BLKSIZE, st->st_blksize);
#ifdef _ST_FLAGS_PRESENT_
    hcc_leaf_int64(trans, LC_FILEFLAGS, st->st_flags);
#endif
    return(0);
}

/*
 * OPENDIR
 */
DIR *
hc_opendir(struct HostConf *hc, const char *path)
{
    hctransaction_t trans;
    struct HCHead *head;

    if (hc == NULL || hc->host == NULL)
	return(opendir(path));

    if (hc->version <= 3) { /* compatibility: HC_SCANDIR not supported */
	struct HCLeaf *item;
	struct HCDirEntry *den;
	intptr_t desc = 0;

	trans = hcc_start_command(hc, HC_OPENDIR);
	hcc_leaf_string(trans, LC_PATH1, path);
	if ((head = hcc_finish_command(trans)) == NULL)
	    return (NULL);
	if (head->error)
	    return (NULL);
	FOR_EACH_ITEM(item, trans, head) {
	    if (item->leafid == LC_DESCRIPTOR)
		desc = HCC_INT32(item);
	}
	if (hcc_get_descriptor(hc, desc, HC_DESC_DIR)) {
	    fprintf(stderr, "hc_opendir: remote reused active descriptor %jd\n",
		(intmax_t)desc);
	    return (NULL);
	}
	den = malloc(sizeof(*den));
	hcc_set_descriptor(hc, desc, den, HC_DESC_DIR);
	return ((void *)desc);
    }

    /* hc->version >= 4: use HC_SCANDIR */
    trans = hcc_start_command(hc, HC_SCANDIR);
    hcc_leaf_string(trans, LC_PATH1, path);
    if ((head = hcc_finish_command(trans)) == NULL || head->error)
	return (NULL);
    return ((void *)head);
}

static int
rc_opendir(hctransaction_t trans, struct HCHead *head)
{
    struct HCLeaf *item;
    const char *path = NULL;
    DIR *dir;
    int desc;

    FOR_EACH_ITEM(item, trans, head) {
	if (item->leafid == LC_PATH1)
	    path = HCC_STRING(item);
    }
    if (path == NULL)
	return(-2);
    if ((dir = opendir(path)) == NULL) {
	head->error = errno;
    } else {
	desc = hcc_alloc_descriptor(trans->hc, dir, HC_DESC_DIR);
	hcc_leaf_int32(trans, LC_DESCRIPTOR, desc);
    }
    return(0);
}

/*
 * READDIR
 */
struct HCDirEntry *
hc_readdir(struct HostConf *hc, DIR *dir, struct stat **statpp)
{
    int stat_ok = 0;
    struct HCHead *head;
    struct HCLeaf *item;
    static struct HCDirEntry denbuf;

    *statpp = NULL;
    if (hc == NULL || hc->host == NULL) {
	struct dirent *sysden;

	if ((sysden = readdir(dir)) == NULL)
	    return (NULL);
	strlcpy(denbuf.d_name, sysden->d_name, MAXNAMLEN + 1);
	return (&denbuf);
    }

    if (hc->version <= 3) { /* compatibility: HC_SCANDIR not supported */
	hctransaction_t trans;
	struct HCDirEntry *den;

	trans = hcc_start_command(hc, HC_READDIR);
	hcc_leaf_int32(trans, LC_DESCRIPTOR, (intptr_t)dir);
	if ((head = hcc_finish_command(trans)) == NULL)
	    return (NULL);
	if (head->error)
	    return (NULL);	/* XXX errno */
	den = hcc_get_descriptor(hc, (intptr_t)dir, HC_DESC_DIR);
	if (den == NULL)
	    return (NULL);	/* XXX errno */
	den->d_name[0] = 0;
	FOR_EACH_ITEM(item, trans, head) {
	    if (item->leafid == LC_PATH1)
		strlcpy(den->d_name, HCC_STRING(item), MAXNAMLEN + 1);
	}
	return (den->d_name[0] ? den : NULL);
    }

    /* hc->version >= 4: using HC_SCANDIR */
    denbuf.d_name[0] = 0;
    head = (void *)dir;
    *statpp = malloc(sizeof(struct stat));
    bzero(*statpp, sizeof(struct stat));
    while ((item = hcc_nextchaineditem(hc, head)) != NULL) {
	if (item->leafid == LC_PATH1) {  /* this must be the last item */
	    strlcpy(denbuf.d_name, HCC_STRING(item), MAXNAMLEN + 1);
	    break;
	} else {
	    stat_ok = 1;
	    hc_decode_stat_item(*statpp, item);
	}
    }
    if (!stat_ok) {
	free(*statpp);
	*statpp = NULL;
    }
    if (hc->trans.state == HCT_FAIL)
	return NULL;
    return (denbuf.d_name[0] ? &denbuf : NULL);
}

static int
rc_readdir(hctransaction_t trans, struct HCHead *head)
{
    struct HCLeaf *item;
    struct dirent *den;
    DIR *dir = NULL;

    FOR_EACH_ITEM(item, trans, head) {
	if (item->leafid == LC_DESCRIPTOR)
	    dir = hcc_get_descriptor(trans->hc, HCC_INT32(item), HC_DESC_DIR);
    }
    if (dir == NULL)
	return(-2);
    if ((den = readdir(dir)) != NULL)
	hcc_leaf_string(trans, LC_PATH1, den->d_name);
    return(0);
}

/*
 * CLOSEDIR
 *
 * XXX cpdup needs to check error code to avoid truncated dirs?
 */
int
hc_closedir(struct HostConf *hc, DIR *dir)
{
    struct HCHead *head;

    if (hc == NULL || hc->host == NULL)
	return(closedir(dir));

    if (hc->version <= 3) { /* compatibility: HC_SCANDIR not supported */
	hctransaction_t trans;
	struct dirent *den;

	if ((den = hcc_get_descriptor(hc, (intptr_t)dir, HC_DESC_DIR)) != NULL) {
	    free(den);
	    hcc_set_descriptor(hc, (intptr_t)dir, NULL, HC_DESC_DIR);
	    trans = hcc_start_command(hc, HC_CLOSEDIR);
	    hcc_leaf_int32(trans, LC_DESCRIPTOR, (intptr_t)dir);
	    if ((head = hcc_finish_command(trans)) == NULL)
		return (-1);
	    if (head->error)
		return (-1);		/* XXX errno */
	    return (0);
	} else {
	    /* errno */
	    return(-1);
	}
    }

    /* hc->version >= 4: using HC_SCANDIR */
    head = (void *)dir;
    /* skip any remaining items if the directory is closed prematurely */
    while (hcc_nextchaineditem(hc, head) != NULL)
	/*nothing*/ ;
    if (hc->trans.state == HCT_FAIL || head->error)
	return (-1);
    return (0);
}

static int
rc_closedir(hctransaction_t trans, struct HCHead *head)
{
    struct HCLeaf *item;
    DIR *dir = NULL;

    FOR_EACH_ITEM(item, trans, head) {
	if (item->leafid == LC_DESCRIPTOR) {
	    dir = hcc_get_descriptor(trans->hc, HCC_INT32(item), HC_DESC_DIR);
	    if (dir != NULL) {
		    hcc_set_descriptor(trans->hc, HCC_INT32(item),
				       NULL, HC_DESC_DIR);
	    }
	}
    }
    if (dir == NULL)
	return(-2);
    return(closedir(dir));
}

/*
 * SCANDIR
 */
static int
rc_scandir(hctransaction_t trans, struct HCHead *head)
{
    struct HCLeaf *item;
    const char *path = NULL;
    struct dirent *den;
    DIR *dir;
    char *fpath;
    struct stat st;

    FOR_EACH_ITEM(item, trans, head) {
	if (item->leafid == LC_PATH1)
	    path = HCC_STRING(item);
    }
    if (path == NULL)
	return (-2);
    if ((dir = opendir(path)) == NULL)
	return (-1);
    while ((den = readdir(dir)) != NULL) {
	if (den->d_name[0] == '.' && (den->d_name[1] == '\0' ||
		(den->d_name[1] == '.' && den->d_name[2] == '\0')))
	    continue;	/* skip "." and ".." */
	/*
	 * Check if there's enough space left in the current packet.
	 * We have at most STAT_MAX_NUM_ENTRIES pieces of data, of which
	 * one is a string, so we use strlen() + 1 (terminating zero).
	 * The remaining ones are numbers; we assume sizeof(int64_t) so
	 * we're on the safe side.
	 */
	if (!hcc_check_space(trans, head, STAT_MAX_NUM_ENTRIES,
		(STAT_MAX_NUM_ENTRIES - 1) * sizeof(int64_t) +
		strlen(den->d_name) + 1)) {
	    closedir(dir);
	    return (-1);
	}
	fpath = mprintf("%s/%s", path, den->d_name);
	if (lstat(fpath, &st) == 0)
	    rc_encode_stat(trans, &st);
	/* The name must be the last item! */
	hcc_leaf_string(trans, LC_PATH1, den->d_name);
	free(fpath);
    }
    return (closedir(dir));
}

/*
 * OPEN
 */
int
hc_open(struct HostConf *hc, const char *path, int flags, mode_t mode)
{
    hctransaction_t trans;
    struct HCHead *head;
    struct HCLeaf *item;
    int *fdp;
    int desc = 0;
    int nflags;

    if (NotForRealOpt && (flags & O_CREAT))
	return(0x7FFFFFFF);

    if (hc == NULL || hc->host == NULL) {
#ifdef O_LARGEFILE
	flags |= O_LARGEFILE;
#endif
	return(open(path, flags, mode));
    }

    if ((flags & (O_WRONLY | O_RDWR)) == 0 && hc->version >= 4) {
	trans = hcc_start_command(hc, HC_READFILE);
	hcc_leaf_string(trans, LC_PATH1, path);
	if ((head = hcc_finish_command(trans)) == NULL || head->error)
	    return (-1);
	head->magic = 0; /* used to indicate offset within buffer */
	return (1); /* dummy */
    }

    nflags = flags & XO_NATIVEMASK;
    if (flags & O_CREAT)
	nflags |= XO_CREAT;
    if (flags & O_EXCL)
	nflags |= XO_EXCL;
    if (flags & O_TRUNC)
	nflags |= XO_TRUNC;

    trans = hcc_start_command(hc, HC_OPEN);
    hcc_leaf_string(trans, LC_PATH1, path);
    hcc_leaf_int32(trans, LC_OFLAGS, nflags);
    hcc_leaf_int32(trans, LC_MODE, mode);

    if ((head = hcc_finish_command(trans)) == NULL)
	return(-1);
    if (head->error)
	return(-1);
    FOR_EACH_ITEM(item, trans, head) {
	if (item->leafid == LC_DESCRIPTOR)
	    desc = HCC_INT32(item);
    }
    if (hcc_get_descriptor(hc, desc, HC_DESC_FD)) {
	fprintf(stderr, "hc_open: remote reused active descriptor %d\n",
		desc);
	return(-1);
    }
    fdp = malloc(sizeof(int));
    *fdp = desc;	/* really just a dummy */
    hcc_set_descriptor(hc, desc, fdp, HC_DESC_FD);
    return(desc);
}

static int
rc_open(hctransaction_t trans, struct HCHead *head)
{
    struct HCLeaf *item;
    const char *path = NULL;
    int nflags = 0;
    int flags;
    mode_t mode = 0666;
    int desc;
    int *fdp;
    int fd;

    FOR_EACH_ITEM(item, trans, head) {
	switch(item->leafid) {
	case LC_PATH1:
	    path = HCC_STRING(item);
	    break;
	case LC_OFLAGS:
	    nflags = HCC_INT32(item);
	    break;
	case LC_MODE:
	    mode = HCC_INT32(item);
	    break;
	}
    }
    if (path == NULL)
	return(-2);

    flags = nflags & XO_NATIVEMASK;
    if (nflags & XO_CREAT)
	flags |= O_CREAT;
    if (nflags & XO_EXCL)
	flags |= O_EXCL;
    if (nflags & XO_TRUNC)
	flags |= O_TRUNC;

    if (ReadOnlyOpt) {
	if (flags & (O_WRONLY | O_RDWR | O_CREAT | O_TRUNC)) {
	    head->error = EACCES;
	    return (0);
	}
	flags |= O_RDONLY;
    }

#ifdef O_LARGEFILE
    flags |= O_LARGEFILE;
#endif
    if ((fd = open(path, flags, mode)) < 0)
	return(-1);
    fdp = malloc(sizeof(int));
    *fdp = fd;
    desc = hcc_alloc_descriptor(trans->hc, fdp, HC_DESC_FD);
    hcc_leaf_int32(trans, LC_DESCRIPTOR, desc);
    return(0);
}

/*
 * CLOSE
 */
int
hc_close(struct HostConf *hc, int fd)
{
    hctransaction_t trans;
    struct HCHead *head;
    int *fdp;

    if (NotForRealOpt && fd == 0x7FFFFFFF)
	return(0);
    if (hc == NULL || hc->host == NULL)
	return(close(fd));

    if (fd == 1 && hc->version >= 4) {	/* using HC_READFILE */
	head = (void *)hc->trans.rbuf;
	/* skip any remaining items if the file is closed prematurely */
	while (hcc_nextchaineditem(hc, head) != NULL)
	    /*nothing*/ ;
	if (hc->trans.state == HCT_FAIL || head->error)
	    return (-1);
	return (0);
    }

    fdp = hcc_get_descriptor(hc, fd, HC_DESC_FD);
    if (fdp) {
	free(fdp);
	hcc_set_descriptor(hc, fd, NULL, HC_DESC_FD);

	trans = hcc_start_command(hc, HC_CLOSE);
	hcc_leaf_int32(trans, LC_DESCRIPTOR, fd);
	if ((head = hcc_finish_command(trans)) == NULL)
	    return(-1);
	if (head->error)
	    return(-1);
	return(0);
    } else {
	return(-1);
    }
}

static int
rc_close(hctransaction_t trans, struct HCHead *head)
{
    struct HCLeaf *item;
    int *fdp = NULL;
    int fd;
    int desc = -1;

    FOR_EACH_ITEM(item, trans, head) {
	if (item->leafid == LC_DESCRIPTOR)
	    desc = HCC_INT32(item);
    }
    if (desc < 0)
	return(-2);
    if ((fdp = hcc_get_descriptor(trans->hc, desc, HC_DESC_FD)) == NULL)
	return(-2);
    fd = *fdp;
    free(fdp);
    hcc_set_descriptor(trans->hc, desc, NULL, HC_DESC_FD);
    return(close(fd));
}

static int
getiolimit(void)
{
    return(32768);
}

/*
 * READ
 */
ssize_t
hc_read(struct HostConf *hc, int fd, void *buf, size_t bytes)
{
    hctransaction_t trans;
    struct HCHead *head;
    struct HCLeaf *item;
    int *fdp;
    int offset;
    int r = 0;
    int x = 0;

    if (hc == NULL || hc->host == NULL)
	return(read(fd, buf, bytes));

    if (fd == 1 && hc->version >= 4) {	/* using HC_READFILE */
	head = (void *)hc->trans.rbuf;
	while (bytes) {
	    if ((offset = head->magic) != 0) {
		item = hcc_currentchaineditem(hc, head);
	    } else {
		item = hcc_nextchaineditem(hc, head);
	    }
	    if (item == NULL) {
		if (hc->trans.state == HCT_FAIL)
			r = -1;
		return (r);
	    }
	    if (item->leafid != LC_DATA)
		return (-1);
	    x = item->bytes - sizeof(*item) - offset;
	    if (x > (int)bytes) {
		x = (int)bytes;
		head->magic += x;  /* leave bytes in the buffer */
	    }
	    else
		head->magic = 0;  /* all bytes used up */
	    bcopy((char *)HCC_BINARYDATA(item) + offset, buf, x);
	    buf = (char *)buf + x;
	    bytes -= (size_t)x;
	    r += x;
	}
	return (r);
    }

    fdp = hcc_get_descriptor(hc, fd, HC_DESC_FD);
    if (fdp) {
	while (bytes) {
	    size_t limit = getiolimit();
	    int n = (bytes > limit) ? limit : bytes;

	    trans = hcc_start_command(hc, HC_READ);
	    hcc_leaf_int32(trans, LC_DESCRIPTOR, fd);
	    hcc_leaf_int32(trans, LC_BYTES, n);
	    if ((head = hcc_finish_command(trans)) == NULL)
		return(-1);
	    if (head->error)
		return(-1);
	    FOR_EACH_ITEM(item, trans, head) {
		if (item->leafid == LC_DATA) {
		    x = item->bytes - sizeof(*item);
		    if (x > (int)bytes)
			x = (int)bytes;
		    bcopy(HCC_BINARYDATA(item), buf, x);
		    buf = (char *)buf + x;
		    bytes -= (size_t)x;
		    r += x;
		}
	    }
	    if (x < n)
		break;
	}
	return(r);
    } else {
	return(-1);
    }
}

static int
rc_read(hctransaction_t trans, struct HCHead *head)
{
    struct HCLeaf *item;
    int *fdp = NULL;
    char buf[32768];
    int bytes = -1;
    int n;

    FOR_EACH_ITEM(item, trans, head) {
	switch(item->leafid) {
	case LC_DESCRIPTOR:
	    fdp = hcc_get_descriptor(trans->hc, HCC_INT32(item), HC_DESC_FD);
	    break;
	case LC_BYTES:
	    bytes = HCC_INT32(item);
	    break;
	}
    }
    if (fdp == NULL)
	return(-2);
    if (bytes < 0 || bytes > 32768)
	return(-2);
    n = read(*fdp, buf, bytes);
    if (n < 0)
	return(-1);
    hcc_leaf_data(trans, LC_DATA, buf, n);
    return(0);
}

/*
 * READFILE
 */
static int
rc_readfile(hctransaction_t trans, struct HCHead *head)
{
    struct HCLeaf *item;
    const char *path = NULL;
    char buf[32768];
    int n;
    int fd;

    FOR_EACH_ITEM(item, trans, head) {
	if (item->leafid == LC_PATH1)
	    path = HCC_STRING(item);
    }
    if (path == NULL)
	return (-2);
    if ((fd = open(path, O_RDONLY)) < 0)
	return(-1);
    while ((n = read(fd, buf, 32768)) >= 0) {
	if (!hcc_check_space(trans, head, 1, n)) {
	    close(fd);
	    return (-1);
	}
	hcc_leaf_data(trans, LC_DATA, buf, n);
	if (n == 0)
		break;
    }
    if (n < 0) {
	close(fd);
	return (-1);
    }
    return (close(fd));
}

/*
 * WRITE
 */
ssize_t
hc_write(struct HostConf *hc, int fd, const void *buf, size_t bytes)
{
    hctransaction_t trans;
    struct HCHead *head;
    struct HCLeaf *item;
    int *fdp;
    int r;

    if (NotForRealOpt)
	return(bytes);

    if (hc == NULL || hc->host == NULL)
	return(write(fd, buf, bytes));

    fdp = hcc_get_descriptor(hc, fd, HC_DESC_FD);
    if (fdp) {
	r = 0;
	while (bytes) {
	    size_t limit = getiolimit();
	    int n = (bytes > limit) ? limit : bytes;
	    int x = 0;

	    trans = hcc_start_command(hc, HC_WRITE);
	    hcc_leaf_int32(trans, LC_DESCRIPTOR, fd);
	    hcc_leaf_data(trans, LC_DATA, buf, n);
	    if ((head = hcc_finish_command(trans)) == NULL)
		return(-1);
	    if (head->error)
		return(-1);
	    FOR_EACH_ITEM(item, trans, head) {
		if (item->leafid == LC_BYTES)
		    x = HCC_INT32(item);
	    }
	    if (x < 0 || x > n)
		return(-1);
	    r += x;
	    buf = (const char *)buf + x;
	    bytes -= x;
	    if (x < n)
		break;
	}
	return(r);
    } else {
	return(-1);
    }
}

static int
rc_write(hctransaction_t trans, struct HCHead *head)
{
    struct HCLeaf *item;
    int *fdp = NULL;
    void *buf = NULL;
    int n = -1;

    FOR_EACH_ITEM(item, trans, head) {
	switch(item->leafid) {
	case LC_DESCRIPTOR:
	    fdp = hcc_get_descriptor(trans->hc, HCC_INT32(item), HC_DESC_FD);
	    break;
	case LC_DATA:
	    buf = HCC_BINARYDATA(item);
	    n = item->bytes - sizeof(*item);
	    break;
	}
    }
    if (ReadOnlyOpt) {
	head->error = EACCES;
	return (0);
    }
    if (fdp == NULL)
	return(-2);
    if (n < 0 || n > 32768)
	return(-2);
    n = write(*fdp, buf, n);
    if (n < 0)
	return (-1);
    hcc_leaf_int32(trans, LC_BYTES, n);
    return(0);
}

/*
 * REMOVE
 *
 * NOTE: This function returns -errno if an error occured.
 */
int
hc_remove(struct HostConf *hc, const char *path)
{
    hctransaction_t trans;
    struct HCHead *head;
    int res;

    if (NotForRealOpt)
	return(0);
    if (hc == NULL || hc->host == NULL) {
	res = remove(path);
	if (res < 0)
		res = -errno;
	return(res);
    }

    trans = hcc_start_command(hc, HC_REMOVE);
    hcc_leaf_string(trans, LC_PATH1, path);
    if ((head = hcc_finish_command(trans)) == NULL)
	return(-EIO);
    if (head->error)
	return(-(int)head->error);
    return(0);
}

static int
rc_remove(hctransaction_t trans, struct HCHead *head)
{
    struct HCLeaf *item;
    const char *path = NULL;

    FOR_EACH_ITEM(item, trans, head) {
	if (item->leafid == LC_PATH1)
	    path = HCC_STRING(item);
    }
    if (path == NULL)
	return(-2);
    if (ReadOnlyOpt) {
	head->error = EACCES;
	return (0);
    }
    return(remove(path));
}

/*
 * MKDIR
 */
int
hc_mkdir(struct HostConf *hc, const char *path, mode_t mode)
{
    hctransaction_t trans;
    struct HCHead *head;

    if (NotForRealOpt)
	return(0);
    if (hc == NULL || hc->host == NULL)
	return(mkdir(path, mode));

    trans = hcc_start_command(hc, HC_MKDIR);
    hcc_leaf_string(trans, LC_PATH1, path);
    hcc_leaf_int32(trans, LC_MODE, mode);
    if ((head = hcc_finish_command(trans)) == NULL)
	return(-1);
    if (head->error)
	return(-1);
    return(0);
}

static int
rc_mkdir(hctransaction_t trans, struct HCHead *head)
{
    struct HCLeaf *item;
    const char *path = NULL;
    mode_t mode = 0777;

    FOR_EACH_ITEM(item, trans, head) {
	switch(item->leafid) {
	case LC_PATH1:
	    path = HCC_STRING(item);
	    break;
	case LC_MODE:
	    mode = HCC_INT32(item);
	    break;
	}
    }
    if (ReadOnlyOpt) {
	head->error = EACCES;
	return (0);
    }
    if (path == NULL)
	return(-2);
    return(mkdir(path, mode));
}

/*
 * RMDIR
 */
int
hc_rmdir(struct HostConf *hc, const char *path)
{
    hctransaction_t trans;
    struct HCHead *head;

    if (NotForRealOpt)
	return(0);
    if (hc == NULL || hc->host == NULL)
	return(rmdir(path));

    trans = hcc_start_command(hc, HC_RMDIR);
    hcc_leaf_string(trans, LC_PATH1, path);
    if ((head = hcc_finish_command(trans)) == NULL)
	return(-1);
    if (head->error)
	return(-1);
    return(0);
}

static int
rc_rmdir(hctransaction_t trans, struct HCHead *head)
{
    struct HCLeaf *item;
    const char *path = NULL;

    FOR_EACH_ITEM(item, trans, head) {
	if (item->leafid == LC_PATH1)
	    path = HCC_STRING(item);
    }
    if (ReadOnlyOpt) {
	head->error = EACCES;
	return (0);
    }
    if (path == NULL)
	return(-2);
    return(rmdir(path));
}

/*
 * CHOWN
 *
 * Almost silently ignore chowns that fail if we are not root.
 */
int
hc_chown(struct HostConf *hc, const char *path, uid_t owner, gid_t group)
{
    hctransaction_t trans;
    struct HCHead *head;
    int rc;

    if (NotForRealOpt)
	return(0);
    if (!DstRootPrivs)
	owner = -1;

    if (hc == NULL || hc->host == NULL) {
	rc = chown(path, owner, group);
	if (rc < 0)
	    rc = silentwarning(&chown_warning, "file ownership may differ\n");
	return(rc);
    }

    trans = hcc_start_command(hc, HC_CHOWN);
    hcc_leaf_string(trans, LC_PATH1, path);
    hcc_leaf_int32(trans, LC_UID, owner);
    hcc_leaf_int32(trans, LC_GID, group);
    if ((head = hcc_finish_command(trans)) == NULL)
	return(-1);
    if (head->error)
	return(-1);
    return(0);
}

static int
rc_chown(hctransaction_t trans, struct HCHead *head)
{
    struct HCLeaf *item;
    const char *path = NULL;
    uid_t uid = (uid_t)-1;
    gid_t gid = (gid_t)-1;
    int rc;

    FOR_EACH_ITEM(item, trans, head) {
	switch(item->leafid) {
	case LC_PATH1:
	    path = HCC_STRING(item);
	    break;
	case LC_UID:
	    uid = HCC_INT32(item);
	    break;
	case LC_GID:
	    gid = HCC_INT32(item);
	    break;
	}
    }
    if (ReadOnlyOpt) {
	head->error = EACCES;
	return (0);
    }
    if (path == NULL)
	return(-2);
    rc = chown(path, uid, gid);
    if (rc < 0)
	rc = silentwarning(&chown_warning, "file ownership may differ\n");
    return(rc);
}

/*
 * LCHOWN
 */
int
hc_lchown(struct HostConf *hc, const char *path, uid_t owner, gid_t group)
{
    hctransaction_t trans;
    struct HCHead *head;
    int rc;

    if (NotForRealOpt)
	return(0);
    if (!DstRootPrivs)
	owner = -1;

    if (hc == NULL || hc->host == NULL) {
	rc = lchown(path, owner, group);
	if (rc < 0)
	    rc = silentwarning(&chown_warning, "file ownership may differ\n");
	return(rc);
    }

    trans = hcc_start_command(hc, HC_LCHOWN);
    hcc_leaf_string(trans, LC_PATH1, path);
    hcc_leaf_int32(trans, LC_UID, owner);
    hcc_leaf_int32(trans, LC_GID, group);
    if ((head = hcc_finish_command(trans)) == NULL)
	return(-1);
    if (head->error)
	return(-1);
    return(0);
}

static int
rc_lchown(hctransaction_t trans, struct HCHead *head)
{
    struct HCLeaf *item;
    const char *path = NULL;
    uid_t uid = (uid_t)-1;
    gid_t gid = (gid_t)-1;
    int rc;

    FOR_EACH_ITEM(item, trans, head) {
	switch(item->leafid) {
	case LC_PATH1:
	    path = HCC_STRING(item);
	    break;
	case LC_UID:
	    uid = HCC_INT32(item);
	    break;
	case LC_GID:
	    gid = HCC_INT32(item);
	    break;
	}
    }
    if (ReadOnlyOpt) {
	head->error = EACCES;
	return (0);
    }
    if (path == NULL)
	return(-2);
    rc = lchown(path, uid, gid);
    if (rc < 0)
	rc = silentwarning(&chown_warning, "file ownership may differ\n");
    return(rc);
}

/*
 * CHMOD
 */
int
hc_chmod(struct HostConf *hc, const char *path, mode_t mode)
{
    hctransaction_t trans;
    struct HCHead *head;

    if (NotForRealOpt)
	return(0);
    if (hc == NULL || hc->host == NULL)
	return(chmod(path, mode));

    trans = hcc_start_command(hc, HC_CHMOD);
    hcc_leaf_string(trans, LC_PATH1, path);
    hcc_leaf_int32(trans, LC_MODE, mode);
    if ((head = hcc_finish_command(trans)) == NULL)
	return(-1);
    if (head->error)
	return(-1);
    return(0);
}

int
hc_lchmod(struct HostConf *hc, const char *path, mode_t mode)
{
    hctransaction_t trans;
    struct HCHead *head;

    if (NotForRealOpt)
	return(0);
    if (hc == NULL || hc->host == NULL)
	return(lchmod(path, mode));

    trans = hcc_start_command(hc, HC_LCHMOD);
    hcc_leaf_string(trans, LC_PATH1, path);
    hcc_leaf_int32(trans, LC_MODE, mode);
    if ((head = hcc_finish_command(trans)) == NULL)
	return(-1);
    if (head->error)
	return(-1);
    return(0);
}

static int
rc_chmod(hctransaction_t trans, struct HCHead *head)
{
    struct HCLeaf *item;
    const char *path = NULL;
    mode_t mode = 0666;

    FOR_EACH_ITEM(item, trans, head) {
	switch(item->leafid) {
	case LC_PATH1:
	    path = HCC_STRING(item);
	    break;
	case LC_MODE:
	    mode = HCC_INT32(item);
	    break;
	}
    }
    if (ReadOnlyOpt) {
	head->error = EACCES;
	return (0);
    }
    if (path == NULL)
	return(-2);
    if (head->cmd == HC_LCHMOD)
	    return(lchmod(path, mode));
    else
	    return(chmod(path, mode));
}

/*
 * MKNOD
 */
int
hc_mknod(struct HostConf *hc, const char *path, mode_t mode, dev_t rdev)
{
    hctransaction_t trans;
    struct HCHead *head;

    if (NotForRealOpt)
	return(0);
    if (!DstRootPrivs) {
	/* mknod() requires root privs, so don't bother. */
	errno = EPERM;
	return (-1);
    }

    if (hc == NULL || hc->host == NULL)
	return(mknod(path, mode, rdev));

    trans = hcc_start_command(hc, HC_MKNOD);
    hcc_leaf_string(trans, LC_PATH1, path);
    hcc_leaf_int32(trans, LC_MODE, mode);
    hcc_leaf_int32(trans, LC_RDEV, rdev);
    if ((head = hcc_finish_command(trans)) == NULL)
	return(-1);
    if (head->error)
	return(-1);
    return(0);
}

static int
rc_mknod(hctransaction_t trans, struct HCHead *head)
{
    struct HCLeaf *item;
    const char *path = NULL;
    mode_t mode = 0666;
    dev_t rdev = 0;

    FOR_EACH_ITEM(item, trans, head) {
	switch(item->leafid) {
	case LC_PATH1:
	    path = HCC_STRING(item);
	    break;
	case LC_MODE:
	    mode = HCC_INT32(item);
	    break;
	case LC_RDEV:
	    rdev = HCC_INT32(item);
	    break;
	}
    }
    if (ReadOnlyOpt) {
	head->error = EACCES;
	return (0);
    }
    if (path == NULL)
	return(-2);
    return(mknod(path, mode, rdev));
}

/*
 * LINK
 */
int
hc_link(struct HostConf *hc, const char *name1, const char *name2)
{
    hctransaction_t trans;
    struct HCHead *head;

    if (NotForRealOpt)
	return(0);
    if (hc == NULL || hc->host == NULL)
	return(link(name1, name2));

    trans = hcc_start_command(hc, HC_LINK);
    hcc_leaf_string(trans, LC_PATH1, name1);
    hcc_leaf_string(trans, LC_PATH2, name2);
    if ((head = hcc_finish_command(trans)) == NULL)
	return(-1);
    if (head->error)
	return(-1);
    return(0);
}

static int
rc_link(hctransaction_t trans, struct HCHead *head)
{
    struct HCLeaf *item;
    const char *name1 = NULL;
    const char *name2 = NULL;

    FOR_EACH_ITEM(item, trans, head) {
	switch(item->leafid) {
	case LC_PATH1:
	    name1 = HCC_STRING(item);
	    break;
	case LC_PATH2:
	    name2 = HCC_STRING(item);
	    break;
	}
    }
    if (ReadOnlyOpt) {
	head->error = EACCES;
	return (-0);
    }
    if (name1 == NULL || name2 == NULL)
	return(-2);
    return(link(name1, name2));
}

#ifdef _ST_FLAGS_PRESENT_
/*
 * CHFLAGS
 */
int
hc_chflags(struct HostConf *hc, const char *path, u_long flags)
{
    hctransaction_t trans;
    struct HCHead *head;
    int rc;

    if (NotForRealOpt)
	return(0);
    if (!DstRootPrivs)
	flags &= UF_SETTABLE;

    if (hc == NULL || hc->host == NULL) {
	if ((rc = chflags(path, flags)) < 0)
	    rc = silentwarning(&chflags_warning, "file flags may differ\n");
	return (rc);
    }

    trans = hcc_start_command(hc, HC_CHFLAGS);
    hcc_leaf_string(trans, LC_PATH1, path);
    hcc_leaf_int64(trans, LC_FILEFLAGS, flags);
    if ((head = hcc_finish_command(trans)) == NULL)
	return(-1);
    if (head->error)
	return(-1);
    return(0);
}

int
hc_lchflags(struct HostConf *hc, const char *path, u_long flags)
{
    hctransaction_t trans;
    struct HCHead *head;
    int rc;

    if (NotForRealOpt)
	return(0);
    if (!DstRootPrivs)
	flags &= UF_SETTABLE;

    if (hc == NULL || hc->host == NULL) {
	if ((rc = lchflags(path, flags)) < 0)
	    rc = silentwarning(&chflags_warning, "file flags may differ\n");
	return (rc);
    }

    trans = hcc_start_command(hc, HC_LCHFLAGS);
    hcc_leaf_string(trans, LC_PATH1, path);
    hcc_leaf_int64(trans, LC_FILEFLAGS, flags);
    if ((head = hcc_finish_command(trans)) == NULL)
	return(-1);
    if (head->error)
	return(-1);
    return(0);
}

static int
rc_chflags(hctransaction_t trans, struct HCHead *head)
{
    struct HCLeaf *item;
    const char *path = NULL;
    u_long flags = 0;
    int rc;

    FOR_EACH_ITEM(item, trans, head) {
	switch(item->leafid) {
	case LC_PATH1:
	    path = HCC_STRING(item);
	    break;
	case LC_FILEFLAGS:
	    flags = (u_long)HCC_INT64(item);
	    break;
	}
    }
    if (ReadOnlyOpt) {
	head->error = EACCES;
	return (0);
    }
    if (path == NULL)
	return(-2);
    if (head->cmd == HC_LCHFLAGS)
	rc = lchflags(path, flags);
    else
	rc = chflags(path, flags);
    if (rc < 0)
	rc = silentwarning(&chflags_warning, "file flags may differ\n");
    return(rc);
}

#endif

/*
 * READLINK
 */
int
hc_readlink(struct HostConf *hc, const char *path, char *buf, int bufsiz)
{
    hctransaction_t trans;
    struct HCHead *head;
    struct HCLeaf *item;
    int r;

    if (hc == NULL || hc->host == NULL)
	return(readlink(path, buf, bufsiz));

    trans = hcc_start_command(hc, HC_READLINK);
    hcc_leaf_string(trans, LC_PATH1, path);
    if ((head = hcc_finish_command(trans)) == NULL)
	return(-1);
    if (head->error)
	return(-1);

    r = 0;
    FOR_EACH_ITEM(item, trans, head) {
	if (item->leafid == LC_DATA) {
	    r = item->bytes - sizeof(*item);
	    if (r < 0)
		r = 0;
	    if (r > bufsiz)
		r = bufsiz;
	    bcopy(HCC_BINARYDATA(item), buf, r);
	}
    }
    return(r);
}

static int
rc_readlink(hctransaction_t trans, struct HCHead *head)
{
    struct HCLeaf *item;
    const char *path = NULL;
    char buf[1024];
    int r;

    FOR_EACH_ITEM(item, trans, head) {
	if (item->leafid == LC_PATH1)
	    path = HCC_STRING(item);
    }
    if (path == NULL)
	return(-2);
    r = readlink(path, buf, sizeof(buf));
    if (r < 0)
	return(-1);
    hcc_leaf_data(trans, LC_DATA, buf, r);
    return(0);
}

/*
 * UMASK
 */
mode_t
hc_umask(struct HostConf *hc, mode_t numask)
{
    hctransaction_t trans;
    struct HCHead *head;
    struct HCLeaf *item;

    if (NotForRealOpt)
	return(umask(numask));
    if (hc == NULL || hc->host == NULL)
	return(umask(numask));

    trans = hcc_start_command(hc, HC_UMASK);
    hcc_leaf_int32(trans, LC_MODE, numask);
    if ((head = hcc_finish_command(trans)) == NULL)
	return((mode_t)-1);
    if (head->error)
	return((mode_t)-1);

    numask = (mode_t) ~0666U;
    FOR_EACH_ITEM(item, trans, head) {
	if (item->leafid == LC_MODE)
	    numask = HCC_INT32(item);
    }
    return(numask);
}

static int
rc_umask(hctransaction_t trans, struct HCHead *head)
{
    struct HCLeaf *item;
    mode_t numask = (mode_t) ~0666U;

    FOR_EACH_ITEM(item, trans, head) {
	if (item->leafid == LC_MODE)
	    numask = HCC_INT32(item);
    }
    numask = umask(numask);
    hcc_leaf_int32(trans, LC_MODE, numask);
    return(0);
}

/*
 * SYMLINK
 */
int
hc_symlink(struct HostConf *hc, const char *name1, const char *name2)
{
    hctransaction_t trans;
    struct HCHead *head;

    if (NotForRealOpt)
	return(0);
    if (hc == NULL || hc->host == NULL)
	return(symlink(name1, name2));

    trans = hcc_start_command(hc, HC_SYMLINK);
    hcc_leaf_string(trans, LC_PATH1, name1);
    hcc_leaf_string(trans, LC_PATH2, name2);
    if ((head = hcc_finish_command(trans)) == NULL)
	return(-1);
    if (head->error)
	return(-1);
    return(0);
}

static int
rc_symlink(hctransaction_t trans, struct HCHead *head)
{
    struct HCLeaf *item;
    const char *name1 = NULL;
    const char *name2 = NULL;

    FOR_EACH_ITEM(item, trans, head) {
	switch(item->leafid) {
	case LC_PATH1:
	    name1 = HCC_STRING(item);
	    break;
	case LC_PATH2:
	    name2 = HCC_STRING(item);
	    break;
	}
    }
    if (ReadOnlyOpt) {
	head->error = EACCES;
	return (0);
    }
    if (name1 == NULL || name2 == NULL)
	return(-2);
    return(symlink(name1, name2));
}

/*
 * RENAME
 */
int
hc_rename(struct HostConf *hc, const char *name1, const char *name2)
{
    hctransaction_t trans;
    struct HCHead *head;

    if (NotForRealOpt)
	return(0);
    if (hc == NULL || hc->host == NULL)
	return(rename(name1, name2));

    trans = hcc_start_command(hc, HC_RENAME);
    hcc_leaf_string(trans, LC_PATH1, name1);
    hcc_leaf_string(trans, LC_PATH2, name2);
    if ((head = hcc_finish_command(trans)) == NULL)
	return(-1);
    if (head->error)
	return(-1);
    return(0);
}

static int
rc_rename(hctransaction_t trans, struct HCHead *head)
{
    struct HCLeaf *item;
    const char *name1 = NULL;
    const char *name2 = NULL;

    FOR_EACH_ITEM(item, trans, head) {
	switch(item->leafid) {
	case LC_PATH1:
	    name1 = HCC_STRING(item);
	    break;
	case LC_PATH2:
	    name2 = HCC_STRING(item);
	    break;
	}
    }
    if (ReadOnlyOpt) {
	head->error = EACCES;
	return (0);
    }
    if (name1 == NULL || name2 == NULL)
	return(-2);
    return(rename(name1, name2));
}

/*
 * UTIMES
 */
int
hc_utimes(struct HostConf *hc, const char *path, const struct timeval *times)
{
    hctransaction_t trans;
    struct HCHead *head;

    if (NotForRealOpt)
	return(0);
    if (hc == NULL || hc->host == NULL) {
	return(utimes(path, times));
    }

    trans = hcc_start_command(hc, HC_UTIMES);
    hcc_leaf_string(trans, LC_PATH1, path);
    hcc_leaf_int64(trans, LC_ATIME, times[0].tv_sec);
    hcc_leaf_int64(trans, LC_MTIME, times[1].tv_sec);
#if defined(st_atime)
    hcc_leaf_int32(trans, LC_ATIMENSEC, times[0].tv_usec * 1000);
    hcc_leaf_int32(trans, LC_MTIMENSEC, times[1].tv_usec * 1000);
#endif
    if ((head = hcc_finish_command(trans)) == NULL)
	return(-1);
    if (head->error)
	return(-1);
    return(0);
}

int
hc_lutimes(struct HostConf *hc, const char *path, const struct timeval *times)
{
    hctransaction_t trans;
    struct HCHead *head;

    if (NotForRealOpt)
	return(0);
    if (hc == NULL || hc->host == NULL) {
	return(lutimes(path, times));
    }

    trans = hcc_start_command(hc, HC_LUTIMES);
    hcc_leaf_string(trans, LC_PATH1, path);
    hcc_leaf_int64(trans, LC_ATIME, times[0].tv_sec);
    hcc_leaf_int64(trans, LC_MTIME, times[1].tv_sec);
#if defined(st_atime)
    hcc_leaf_int32(trans, LC_ATIMENSEC, times[0].tv_usec * 1000);
    hcc_leaf_int32(trans, LC_MTIMENSEC, times[1].tv_usec * 1000);
#endif
    if ((head = hcc_finish_command(trans)) == NULL)
	return(-1);
    if (head->error)
	return(-1);
    return(0);
}

static int
rc_utimes(hctransaction_t trans, struct HCHead *head)
{
    struct HCLeaf *item;
    struct timeval times[2];
    const char *path;

    bzero(times, sizeof(times));
    path = NULL;

    FOR_EACH_ITEM(item, trans, head) {
	switch(item->leafid) {
	case LC_PATH1:
	    path = HCC_STRING(item);
	    break;
	case LC_ATIME:
	    times[0].tv_sec = HCC_INT64(item);
	    break;
	case LC_MTIME:
	    times[1].tv_sec = HCC_INT64(item);
	    break;
#if defined(st_atimespec) || defined(_STATBUF_ST_NSEC)
	case LC_ATIMENSEC:
	    times[0].tv_usec = HCC_INT32(item) / 1000;
	    break;
	case LC_MTIMENSEC:
	    times[1].tv_usec = HCC_INT32(item) / 1000;
	    break;
#endif
	}
    }
    if (ReadOnlyOpt) {
	head->error = EACCES;
	return (0);
    }
    if (path == NULL)
	return(-2);
    if (head->cmd == HC_LUTIMES)
	    return(lutimes(path, times));
    else
	    return(utimes(path, times));
}

uid_t
hc_geteuid(struct HostConf *hc)
{
    hctransaction_t trans;
    struct HCHead *head;
    struct HCLeaf *item;

    if (hc == NULL || hc->host == NULL)
	return (geteuid());

    if (hc->version < 3) {
	fprintf(stderr, "WARNING: Remote client uses old protocol version\n");
	/* Return 0 on error, so the caller assumes root privileges. */
	return (0);
    }

    trans = hcc_start_command(hc, HC_GETEUID);
    if ((head = hcc_finish_command(trans)) == NULL || head->error)
	return(0);
    FOR_EACH_ITEM(item, trans, head) {
	if (item->leafid == LC_UID)
	    return (HCC_INT32(item));
    }
    return(0); /* shouldn't happen */
}

static int
rc_geteuid(hctransaction_t trans, struct HCHead *head __unused)
{
    hcc_leaf_int32(trans, LC_UID, geteuid());
    return (0);
}

static int
getmygroups(gid_t **gidlist)
{
    int count;

    if ((count = getgroups(0, *gidlist)) > 0) {
	if ((*gidlist = malloc(count * sizeof(gid_t))) != NULL) {
	    if ((count = getgroups(count, *gidlist)) <= 0)
		free(*gidlist);
	}
	else
	    count = -1;
    }
    else
	*gidlist = NULL;
    return (count);
}

int
hc_getgroups(struct HostConf *hc, gid_t **gidlist)
{
    int count, i;
    hctransaction_t trans;
    struct HCHead *head;
    struct HCLeaf *item;

    if (hc == NULL || hc->host == NULL)
	return (getmygroups(gidlist));

    i = 0;
    count = 0;
    *gidlist = NULL;

    if (hc->version < 3) {
	fprintf(stderr, "WARNING: Remote client uses old protocol version\n");
	return (-1);
    }

    trans = hcc_start_command(hc, HC_GETGROUPS);
    if ((head = hcc_finish_command(trans)) == NULL || head->error)
	return(-1);
    FOR_EACH_ITEM(item, trans, head) {
	switch(item->leafid) {
	case LC_COUNT:
	    count = HCC_INT32(item);
	    if (*gidlist != NULL) { /* protocol error */
		free(*gidlist);
		*gidlist = NULL;
		return (-1);
	    }
	    if ((*gidlist = malloc(count * sizeof(gid_t))) == NULL)
		return (-1);
	    break;
	case LC_GID:
	    if (*gidlist == NULL || i >= count) { /* protocol error */
		if (*gidlist != NULL)
		    free(*gidlist);
		*gidlist = NULL;
		return (-1);
	    }
	    (*gidlist)[i++] = HCC_INT32(item);
	    break;
	}
    }
    return (count);
}

static int
rc_getgroups(hctransaction_t trans, struct HCHead *head __unused)
{
    int count, i;
    gid_t *gidlist;

    if ((count = getmygroups(&gidlist)) < 0)
	return (-1);
    hcc_leaf_int32(trans, LC_COUNT, count);
    for (i = 0; i < count; i++)
	hcc_leaf_int32(trans, LC_GID, gidlist[i]);
    if (gidlist != NULL)
	free(gidlist);
    return (0);
}
