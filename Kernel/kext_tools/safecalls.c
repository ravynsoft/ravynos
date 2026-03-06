/*
 * Copyright (c) 2007 Apple Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_LICENSE_HEADER_END@
 */
/*
 * FILE: safecalls.c
 * AUTH: Soren Spies (sspies)
 * DATE: 16 June 2006 (Copyright Apple Computer, Inc)
 * DESC: picky/safe syscalls
 *
 * Security functions
 * the first argument limits the scope of the operation
 *
 * Pretty much every function is implemented as
 * savedir = open(".", O_RDONLY);
 * schdirparent()->sopen()->spolicy()
 * <operation>(child)
 * fchdir(savedir)
 *
 */

#include <errno.h>
#include <dirent.h>
#include <fcntl.h>
#include <fts.h>
#include <libgen.h>
#include <limits.h>
#include <sys/mount.h>
#include <sys/param.h>  // MAXBSIZE, MIN
#include <sys/stat.h>
#include <string.h>
#include <stdio.h>      // rename(2)?
#include <stdlib.h>     // malloc(3)
#include <sys/types.h>
#include <unistd.h>
#include <sys/ucred.h>

#include <IOKit/kext/kextmanager_types.h>

#include <IOKit/kext/OSKextPrivate.h>
#include "kext_tools_util.h"
#ifndef kOSKextLogCacheFlag
#define kOSKextLogCacheFlag kOSKextLogArchiveFlag
#endif  // no kOSKextLogCacheFlag

#define STRICT_SAFETY 0     // since our wrappers need to call the real calls
#include "safecalls.h"      // w/o STRICT_SAFETY, will #define mkdir, etc
#include "kext_tools_util.h"

#define RESTOREDIR(savedir) do { if (savedir != -1 && restoredir(savedir))  \
                 OSKextLog(/* kext */ NULL, \
                     kOSKextLogErrorLevel | kOSKextLogCacheFlag, \
                     "%s: ALERT: couldn't restore CWD", __func__); \
    } while(0)

// currently checks to make sure on same volume
// other checks could include:
// - "really owned by <foo> on root/<foo>-mounted volume"
static int spolicy(int scopefd, int candfd)
{
    int bsderr = -1;
    struct stat candsb, scopesb;
    char path[PATH_MAX] = "<unknown>";
#ifdef ROSP_HACKS
    char candDevPath[PATH_MAX];
    char scopeDevPath[PATH_MAX];
#endif /* !ROSP_HACKS */

    if ((bsderr = fstat(candfd, &candsb)))    goto finish;  // trusty fstat()
    if ((bsderr = fstat(scopefd, &scopesb)))  goto finish;  // still there?

    // make sure st_dev matches
    // ROSP_HACKS will give a pass to cross-device scope when
    // candfd is on userdata volume and scopefd is on root volume
    if (candsb.st_dev != scopesb.st_dev
#ifdef ROSP_HACKS
    && (findmnt(candsb.st_dev, candDevPath, true)
         || findmnt(scopesb.st_dev, scopeDevPath, true)
         || !isUserDataVolume(scopeDevPath, candDevPath))
#endif /* !ROSP_HACKS */
    ) {
        bsderr = -1;
        errno = EPERM;
        char scopemnt[MNAMELEN];

        if (findmnt(scopesb.st_dev, scopemnt, false) == 0) {
            (void)fcntl(candfd, F_GETPATH, path);

            OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogCacheFlag | kOSKextLogFileAccessFlag,
            "ALERT: %s does not appear to be on %s.", path, scopemnt);
        } else {
            OSKextLog(NULL,
                      kOSKextLogErrorLevel,
                      "%s - find mount failed: errno %d %s.",
                      __FUNCTION__, errno, strerror(errno));
            OSKextLog(/* kext */ NULL,
                kOSKextLogErrorLevel | kOSKextLogGeneralFlag | kOSKextLogFileAccessFlag,
                "ALERT: dev_t mismatch (%d != %d).",
            candsb.st_dev, scopesb.st_dev);
        }
        goto finish;
    }

    // warn about non-root owners
    // (.disk_label can be written while owners are ignored :P)
    if (candsb.st_uid != 0) {

        // could try to trim pathname to basename?
        (void)fcntl(candfd, F_GETPATH, path);

        OSKextLog(NULL, kOSKextLogErrorLevel | kOSKextLogFileAccessFlag,
            "WARNING: %s: owner not root!", path);
    }

finish:
    return bsderr;
}


int schdirparent(int fdvol, const char *path, int *olddir, char child[PATH_MAX])
{
    int bsderr = -1;
    int dirfd = -1, savedir = -1;
    char parent[PATH_MAX];

    if (olddir)     *olddir = -1;
    if (!path)      goto finish;

    // make a copy of path in case our dirname() ever modifies the buffer
    PATHCPY(parent, path);
    PATHCPY(parent, dirname(parent));

    // make sure parent is on specified volume
    if (-1 == (dirfd = open(parent, O_RDONLY, 0)))  goto finish;
    errno = 0;
    if (spolicy(fdvol, dirfd)) {
        if (errno == EPERM)
            OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogGeneralFlag | kOSKextLogFileAccessFlag,
            "Policy violation opening %s.", parent);
        goto finish;
    }

    // save old directory if requested
    if (olddir) {
        if (-1 == (savedir = open(".", O_RDONLY)))  goto finish;
    }

    // attempt to switch to the directory
    if ((bsderr = fchdir(dirfd)))               goto finish;

    // set output parameters
    if (olddir)             *olddir = savedir;
    if (child) {
        PATHCPY(child, path);
        PATHCPY(child, basename(child));
    }

finish:
    if (bsderr) {
        if (savedir != -1)  close(savedir);
        if (olddir)         *olddir = -1;
    }
    if (dirfd != -1)        close(dirfd);

    return bsderr;
}

int sopen(int fdvol, const char *path, int flags, mode_t mode)
{
    return sopen_ifExists(fdvol, path, flags, mode, true);
}

// have to rely on schdirparent so we don't accidentally O_CREAT
int sopen_ifExists(int fdvol, const char *path, int flags, mode_t mode, bool failIfExists)
{
    int rfd = -1;
    int candfd = -1;
    char child[PATH_MAX];
    int savedir = -1;

    // omitting O_NOFOLLOW except when creating gives better errors
    // flags |= O_NOFOLLOW;

    // if creating, make sure it doesn't exist (O_NOFOLLOW for good measure)
    if (flags & O_CREAT) {
        flags |= O_NOFOLLOW;

        if (failIfExists) {
            flags |= O_EXCL;
        }
    }

    if (schdirparent(fdvol, path, &savedir, child))     goto finish;
    if (-1 == (candfd = open(child, flags, mode)))      goto finish;

    // schdirparent checked the parent; here we check the child (6393648)
    if (spolicy(fdvol, candfd))                         goto finish;

    rfd = candfd;

finish:
    if (candfd != -1 && rfd != candfd) {
        close(candfd);
    }
    RESTOREDIR(savedir);

    return rfd;
}

int schdir(int fdvol, const char *path, int *savedir)
{
    char cpath[PATH_MAX];

    // X could switch to snprintf()
    PATHCPY(cpath, path);
    PATHCAT(cpath, "/.");

    return schdirparent(fdvol, cpath, savedir, NULL);

finish:
    return -1;
}

int restoredir(int savedir)
{
    int cherr = -1, clerr = -1;

    if (savedir != -1) {
        cherr = fchdir(savedir);
        clerr = close(savedir);
    }

    return cherr ? cherr : clerr;
}

int smkdir(int fdvol, const char *path, mode_t mode)
{
    int bsderr = -1;
    int savedir = -1;
    char child[PATH_MAX];

    if (schdirparent(fdvol, path, &savedir, child))  goto finish;
    if ((bsderr = mkdir(child, mode)))      goto finish;

finish:
    RESTOREDIR(savedir);
    return bsderr;
}

int srmdir(int fdvol, const char *path)
{
    int bsderr = -1;
    char child[PATH_MAX];
    int savedir = -1;

    if (schdirparent(fdvol, path, &savedir, child))  goto finish;

    bsderr = rmdir(child);

finish:
    RESTOREDIR(savedir);
    return bsderr;
}

int sunlink(int fdvol, const char *path)
{
    int bsderr = -1;
    char child[PATH_MAX];
    int savedir = -1;

    if (schdirparent(fdvol, path, &savedir, child))  goto finish;

    bsderr = unlink(child);

finish:
    RESTOREDIR(savedir);
    return bsderr;
}


// taking a path and a filename is sort of annoying for clients
// so we "auto-strip" newname if it happens to be a path
int srename(int fdvol, const char *oldpath, const char *newpath)
{
    int bsderr = -1;
    int savedir = -1;
    char oldname[PATH_MAX];
    char newname[PATH_MAX];

    // calculate netname first since schdirparent uses basename
    PATHCPY(newname, newpath);
    PATHCPY(newname, basename(newname));
    if (schdirparent(fdvol, oldpath, &savedir, oldname))        goto finish;

    bsderr = rename(oldname, newname);

finish:
    RESTOREDIR(savedir);
    return bsderr;
}


int szerofile(int fdvol, const char *toErase)
{
    int bsderr = -1;
    int zfd = -1;
    struct stat sb;
    uint64_t bytesLeft;     // why is off_t signed?
    size_t bufsize;
    ssize_t thisTime;
    void *buf = NULL;

    zfd = sopen(fdvol, toErase, O_WRONLY, 0);
    if (zfd == -1) {
        if (errno == ENOENT)
            bsderr = 0;
        goto finish;
    }

    if (fstat(zfd, &sb))                    goto finish;
    if (sb.st_size == 0) {
        bsderr = 0;
        goto finish;
    }
    bufsize = (size_t)MIN(sb.st_size, MAXBSIZE);
    if (!(buf = calloc(1, bufsize)))        goto finish;

    // and loop writing the zeros
    for (bytesLeft = sb.st_size; bytesLeft > 0; bytesLeft -= thisTime) {
        thisTime = (ssize_t)MIN(bytesLeft, bufsize);

        if (write(zfd, buf, thisTime) != thisTime)    goto finish;
    }

    // our job is done, but the space is useless so attempt to truncate
    (void)ftruncate(zfd, 0LL);

    bsderr = 0;

finish:
    if (zfd != -1)      close(zfd);
    if (buf)            free(buf);

    return bsderr;
}

// stolen with gratitude from TAOcommon's TAOCFURLDelete
int sdeepunlink(int fdvol, char *path)
{
    int             rval = ELAST + 1;
    int             firstErrno = 0;     // FTS clears errno at the end :P

    char        *   const pathv[2] = { path, NULL };
    int             ftsoptions = 0;
    FTS         *   fts;
    FTSENT      *   fent;

    // opting for security, of course
    ftsoptions |= FTS_PHYSICAL;         // see symlinks
    ftsoptions |= FTS_XDEV;             // don't cross devices
    ftsoptions |= FTS_NOSTAT;           // fts_info tells us enough
    ftsoptions |= FTS_NOCHDIR;          // only we should be using [f]chdir
//  ftsoptions |= FTS_COMFOLLOW;        // if 'path' is symlink, remove link
//  ftsoptions |= FTS_SEEDOT;           // we don't need "."

    rval = -1;
    if ((fts = fts_open(pathv, ftsoptions, NULL)) == NULL)  goto finish;
    rval = 0;

    // and here we go
    while ((fent = fts_read(fts)) /* && !rval ?? */) {
        switch (fent->fts_info) {
            case FTS_DC:        // directory that causes a cycle in the tree
            case FTS_D:         // directory being visited in pre-order
            case FTS_DOT:       // file named '.' or '..' (not requested)
                break;

            case FTS_DNR:       // directory which cannot be read
            case FTS_ERR:       // generic fcts_errno-borne error
            case FTS_NS:        // file for which stat(s) failed (not requested)
                // rval |= fent->fts_errno;
                if (!firstErrno)        firstErrno = fent->fts_errno;
                break;

            case FTS_SL:        // symbolic link
            case FTS_SLNONE:    // symbolic link with a non-existent target
            case FTS_DEFAULT:   // good file of type unknown to FTS (block? ;)
            case FTS_F:         // regular file
            case FTS_NSOK:      // no stat(2) requested (but not a dir?)
            default:            // in case FTS gets smarter in the future
                // XX need to port RECERR() from update_boot.c
                rval |= sunlink(fdvol, fent->fts_accpath);
                if (!firstErrno)        firstErrno = errno;
                break;

            case FTS_DP:        // directory being visited in post-order
                // XX need to port RECERR() from update_boot.c
                rval |= srmdir(fdvol, fent->fts_accpath);
                if (!firstErrno)        firstErrno = errno;
                break;
        } // switch
    } // while (fts_read())

    // close the iterator now
    if (fts_close(fts) < 0) {
        OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogGeneralFlag | kOSKextLogFileAccessFlag,
            "fts_close failed - %s.", strerror(errno));
    }

    if (firstErrno) {
        rval = -1;
        errno = firstErrno;
    }

finish:
    // fts_read() clears errno if it completed
    if (rval == 0 && errno) {
        rval = -1;
    }

    return rval;
}

int sdeepmkdir(int fdvol, const char *path, mode_t mode)
{
    int bsderr = -1;
    struct stat sb;
    char parent[PATH_MAX];

    if (strlen(path) == 0)      goto finish;        // protection?

    // trusting that stat(".") will always do the right thing
    if (0 == stat(path, &sb)) {
        if ((sb.st_mode & S_IFMT) != S_IFDIR) {
            bsderr = ENOTDIR;
            goto finish;
        } else {
            bsderr = 0;             // base case (dir exists)
            goto finish;
        }
    } else if (errno != ENOENT) {
        goto finish;                // bsderr = -1 -> errno
    } else {
        PATHCPY(parent, path);
        PATHCPY(parent, dirname(parent));

        // and recurse since it wasn't there
        if ((bsderr = sdeepmkdir(fdvol, parent, mode)))     goto finish;
    }

    // all parents made; top-level still needed
    bsderr = smkdir(fdvol, path, mode);

finish:
    return bsderr;
}


static int
_copyfiledata(int srcfd, struct stat *srcsb, int dstfdvol, const char *dstpath)
{
    int bsderr = -1;
    int dstfd = -1;
    void *buf = NULL;       // up to MAXBSIZE on the stack is a bad idea
    size_t bufsize;
    ssize_t thisTime;
    off_t bytesLeft;

    // nuke/open the destination
    (void)sunlink(dstfdvol, dstpath);
    dstfd = sopen(dstfdvol, dstpath, O_CREAT|O_WRONLY, srcsb->st_mode|S_IWUSR);
    if (dstfd == -1)        goto finish;

    // and loop with our handy buffer
    bufsize = (size_t)MIN(srcsb->st_size, MAXBSIZE);
    if (!(buf = malloc(bufsize)))      goto finish;;
    for (bytesLeft = srcsb->st_size; bytesLeft > 0; bytesLeft -= thisTime) {
        thisTime = (ssize_t)MIN(bytesLeft, (unsigned int)bufsize);

        if (read(srcfd, buf, thisTime) != thisTime)     goto finish;
        if (write(dstfd, buf, thisTime) != thisTime)    goto finish;
    }

    // apply final permissions
    if ((bsderr = fchmod(dstfd, srcsb->st_mode)))  goto finish;
    // kextcache doesn't currently look into the Apple_Boot, so we'll skip times

finish:
    if (dstfd != -1)    close(dstfd);
    if (buf)            free(buf);

    return bsderr;
}

// for now, we only support a flat set of files; no recursion
static int
_copysubitems(int srcfdvol, const char *srcdir, int dstfdvol,const char *dstdir)
{
    int bsderr = -1;
    DIR *dir = NULL;
    struct dirent dentry, *dp;
    char srcpath[PATH_MAX], dstpath[PATH_MAX];

    // scopyitem() will also validate srcfdvol for each entry
    if (!(dir = opendir(srcdir)))
        goto finish;
    if (spolicy(srcfdvol, dirfd(dir)))
        goto finish;

    while (0 == (bsderr = readdir_r(dir, &dentry, &dp)) && dp) {
        char *fname = dp->d_name;

        // skip "." and ".."
        if ((fname[0] == '.' && fname[1] == '\0') ||
            (fname[0] == '.' && fname[1] == '.' && fname[2] == '\0'))
                continue;

        // set up source path for child file
        PATHCPY(srcpath, srcdir);
        PATHCAT(srcpath, "/");
        PATHCAT(srcpath, fname);

        // and corresponding destination path
        PATHCPY(dstpath, dstdir);
        PATHCAT(dstpath, "/");
        PATHCAT(dstpath, fname);

        // recurse back to scopyitem()
        bsderr = scopyitem(srcfdvol, srcpath, dstfdvol, dstpath);
        if (bsderr)   goto finish;
    }

finish:
    if (dir)            closedir(dir);

    return bsderr;
}

int
scopyitem(int srcfdvol, const char *srcpath, int dstfdvol, const char *dstpath)
{
    int bsderr = -1;
    int srcfd = -1;
    struct stat srcsb;
    char dstparent[PATH_MAX];
    mode_t dirmode;

    // figure out parent directory mode
    if (-1 == (srcfd = sopen(srcfdvol, srcpath, O_RDONLY, 0)))    goto finish;
    if (fstat(srcfd, &srcsb))                       goto finish;
    dirmode = ((srcsb.st_mode&~S_IFMT) | S_IWUSR | S_IXUSR /* u+wx */);
    if (dirmode & S_IRGRP)      dirmode |= S_IXGRP;     // add conditional o+x
    if (dirmode & S_IROTH)      dirmode |= S_IXOTH;

    // and recursively create the parent directory
    PATHCPY(dstparent, dstpath);
    PATHCPY(dstparent, dirname(dstparent));

    if ((bsderr = sdeepmkdir(dstfdvol, dstparent, dirmode)))        goto finish;

    // should we let _copysubitems will call us back
    switch ((srcsb.st_mode & S_IFMT)) {
        case S_IFREG:
            bsderr = _copyfiledata(srcfd, &srcsb, dstfdvol, dstpath);
            break;

        case S_IFDIR:
            bsderr = _copysubitems(srcfdvol, srcpath, dstfdvol, dstpath);
            break;

        default:
            bsderr = EFTYPE;
            break;
    }

finish:
    if (srcfd != -1)    close(srcfd);

    return bsderr;
}
