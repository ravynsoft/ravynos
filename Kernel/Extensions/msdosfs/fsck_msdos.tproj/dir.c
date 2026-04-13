/*
 * Copyright (c) 2000 Apple Computer, Inc. All rights reserved.
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
 * Copyright (C) 1995, 1996, 1997 Wolfgang Solfrank
 * Copyright (c) 1995 Martin Husemann
 * Some structure declaration borrowed from Paul Popelka
 * (paulp@uts.amdahl.com), see /sys/msdosfs/ for reference.
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
 *	This product includes software developed by Martin Husemann
 *	and Wolfgang Solfrank.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHORS ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


#include <sys/cdefs.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>

#include <sys/param.h>
#include <sys/errno.h>

#include "ext.h"
#include "fsutil.h"

#define	SLOT_EMPTY	0x00		/* slot has never been used */
#define	SLOT_E5		0x05		/* the real value is 0xe5 */
#define	SLOT_DELETED	0xe5		/* file in this slot deleted */

#define	ATTR_NORMAL	0x00		/* normal file */
#define	ATTR_READONLY	0x01		/* file is readonly */
#define	ATTR_HIDDEN	0x02		/* file is hidden */
#define	ATTR_SYSTEM	0x04		/* file is a system file */
#define	ATTR_VOLUME	0x08		/* entry is a volume label */
#define	ATTR_DIRECTORY	0x10		/* entry is a directory name */
#define	ATTR_ARCHIVE	0x20		/* file is new or modified */

#define	ATTR_WIN95	0x0f		/* long name record */

/*
 * This is the format of the contents of the deTime field in the direntry
 * structure.
 * We don't use bitfields because we don't know how compilers for
 * arbitrary machines will lay them out.
 */
#define DT_2SECONDS_MASK	0x1F	/* seconds divided by 2 */
#define DT_2SECONDS_SHIFT	0
#define DT_MINUTES_MASK		0x7E0	/* minutes */
#define DT_MINUTES_SHIFT	5
#define DT_HOURS_MASK		0xF800	/* hours */
#define DT_HOURS_SHIFT		11

/*
 * This is the format of the contents of the deDate field in the direntry
 * structure.
 */
#define DD_DAY_MASK		0x1F	/* day of month */
#define DD_DAY_SHIFT		0
#define DD_MONTH_MASK		0x1E0	/* month */
#define DD_MONTH_SHIFT		5
#define DD_YEAR_MASK		0xFE00	/* year - 1980 */
#define DD_YEAR_SHIFT		9


/* dir.c */
static struct dosDirEntry *newDosDirEntry __P((void));
static void freeDosDirEntry __P((struct dosDirEntry *));
static struct dirTodoNode *newDirTodo __P((void));
static void freeDirTodo __P((struct dirTodoNode *));
static char *fullpath __P((struct dosDirEntry *));
static u_char calcShortSum __P((u_char *));
static int delete(int fd, struct bootblock *boot, cl_t startcl, size_t startoff, cl_t endcl, size_t endoff, int notlast);
static int msdosfs_removede __P((int, struct bootblock *, u_char *,
    u_char *, cl_t, cl_t, cl_t, char *, int));
static int checksize __P((struct bootblock *, u_char *, struct dosDirEntry *));
static int readDosDirSection __P((int, struct bootblock *, struct dosDirEntry *));

/*
 * Manage free dosDirEntry structures.
 */
static struct dosDirEntry *freede;

static struct dosDirEntry *
newDosDirEntry()
{
	struct dosDirEntry *de;

	if (!(de = freede)) {
		if (!(de = (struct dosDirEntry *)malloc(sizeof *de)))
			return 0;
	} else
		freede = de->next;
	return de;
}

static void
freeDosDirEntry(de)
	struct dosDirEntry *de;
{
	de->next = freede;
	freede = de;
}

/*
 * The same for dirTodoNode structures.
 */
static struct dirTodoNode *freedt;

static struct dirTodoNode *
newDirTodo()
{
	struct dirTodoNode *dt;

	if (!(dt = freedt)) {
		if (!(dt = (struct dirTodoNode *)malloc(sizeof *dt)))
			return 0;
	} else
		freedt = dt->next;
	return dt;
}

static void
freeDirTodo(dt)
	struct dirTodoNode *dt;
{
	dt->next = freedt;
	freedt = dt;
}

/*
 * The stack of unread directories
 */
struct dirTodoNode *pendingDirectories = NULL;

/*
 * Return the full pathname for a directory entry.
 */
static char *
fullpath(dir)
	struct dosDirEntry *dir;
{
	static char namebuf[MAXPATHLEN + 1];
	char *cp, *np;
	size_t nl;

	/*
	 * The loop below returns the empty string for the root directory.
	 * So special case it to return "/" instead.
	 */
	if (dir == rootDir)
	{
		namebuf[0] = '/';
		namebuf[1] = '\0';
		return namebuf;
	}
	
	cp = namebuf + sizeof namebuf - 1;
	*cp = '\0';
	do {
		np = dir->lname[0] ? dir->lname : dir->name;
		nl = strlen(np);
		if ((cp -= nl) <= namebuf + 1)
			break;
		memcpy(cp, np, nl);
		*--cp = '/';
	} while ((dir = dir->parent) != NULL);
	if (dir)
		*--cp = '?';
	else
		cp++;
	
	return cp;
}

/*
 * Calculate a checksum over an 8.3 alias name
 */
static u_char
calcShortSum(p)
	u_char *p;
{
	u_char sum = 0;
	int i;

	for (i = 0; i < 11; i++) {
		sum = (sum << 7)|(sum >> 1);	/* rotate right */
		sum += p[i];
	}

	return sum;
}


/*
 * markDosDirChain
 *
 * Follow the cluster chain pointed to by @dir.  Mark all of the clusters in
 * use in our bitmap.  If we encounter a cluster that is already marked in
 * use, out of range, reserved, or EOF, then set @dir->end to that cluster
 * number.  Also sets @dir->physicalSize to the size, in bytes, of valid
 * clusters in the chain.
 *
 * Assumes that the caller has already verified that the starting cluster
 * is valid, not CLUST_FREE, and not yet marked used.
 */
static int
markDosDirChain(struct bootblock *boot, struct dosDirEntry *dir)
{
	int err = FSOK;
	cl_t cluster, prev, value;
	cl_t count;
	
	cluster = dir->head;
	prev = 0;
	count = 0;
	while (cluster >= CLUST_FIRST && cluster < boot->NumClusters && !isUsed(cluster))
	{
		/*
		 * Clusters that are marked "reserved" or "bad" cannot be part of the
		 * file.  We must truncate the file at the previous cluster, which
		 * is why we break from the loop early.
		 *
		 * Clusters marked "free", or which point to invalid cluster numbers
		 * can be allocated to the file my setting them to CLUST_EOF.  We
		 * catch these cases on the next iteration of the loop so that the
		 * current cluster will remain part of the file (i.e. it becomes
		 * "previous" as we iterate once more).
		 */
		value = fat_get(cluster);
		if (value == CLUST_RSRVD || value == CLUST_BAD)
		{
			cluster = value;
			break;
		}
		markUsed(cluster);
		++count;
		prev = cluster;
		cluster = fat_get(cluster);
	}
	
	/*
	 * We hit the end of the cluster chain.  If it wasn't due to EOF, then see
	 * if we can fix the problem.
	 */
	if (cluster < CLUST_EOFS)
	{
		if (cluster == CLUST_FREE || cluster >= CLUST_RSRVD)
			pwarn("%s: Cluster chain starting at %u ends with cluster marked %s\n",
				fullpath(dir), dir->head, rsrvdcltype(cluster));
		else if (cluster < CLUST_FIRST || cluster >= boot->NumClusters)
			pwarn("%s: Cluster chain starting at %u continues with cluster out of range (%u)\n",
				fullpath(dir), dir->head, cluster);
		else
			pwarn("%s: Cluster chain starting at %u is cross-linked at cluster %u\n",
				fullpath(dir), dir->head, cluster);
		if (ask(1, "Truncate"))
		{
			err = fat_set(prev, CLUST_EOF);
			if (err)
				cluster = CLUST_ERROR;
			else
				cluster = CLUST_EOF;
		}
	}
	
	dir->end = cluster;
	dir->physicalSize = (u_int64_t)count * boot->ClusterSize;
	if (cluster == CLUST_ERROR)
		return FSFATAL;

	return err;
}


/*
 * Global variables temporarily used during a directory scan
 */
static char longName[DOSLONGNAMELEN] = "";
static u_char *buffer = NULL;
static u_char *delbuf = NULL;

struct dosDirEntry *rootDir;
static struct dosDirEntry *lostDir;

/*
 * Init internal state for a new directory scan.
 */
int
resetDosDirSection(struct bootblock *boot)
{
	int b1, b2;
	cl_t cl;
	int ret = FSOK;

	b1 = boot->RootDirEnts * 32;
	b2 = boot->SecPerClust * boot->BytesPerSec;

	if (!(buffer = malloc(b1 > b2 ? b1 : b2))
	    || !(delbuf = malloc(b2))
	    || !(rootDir = newDosDirEntry())) {
		perr("No space for directory");
		return FSFATAL;
	}
	memset(rootDir, 0, sizeof *rootDir);
	if (boot->flags & FAT32) {
		if (boot->RootCl < CLUST_FIRST || boot->RootCl >= boot->NumClusters) {
			pfatal("Root directory starts with cluster out of range(%u)\n",
			       boot->RootCl);
			return FSFATAL;
		}

		cl = fat_get(boot->RootCl);
		if (cl == CLUST_ERROR)
			return FSFATAL;

		if (cl < CLUST_FIRST
		    || (cl >= CLUST_RSRVD && cl< CLUST_EOFS)) {
			if (cl == CLUST_FREE)
				pwarn("Root directory starts with free cluster\n");
			else if (cl >= CLUST_RSRVD)
				pwarn("Root directory starts with cluster marked %s\n",
				      rsrvdcltype(cl));
			if (ask(1, "Fix")) {
				/*
				 * This used to assign CLUST_FREE.  How was that a good idea???
				 */
				ret = fat_set(boot->RootCl, CLUST_EOF);
				if (!ret)
					ret = FSFATMOD;
				
				/*
				 * We have to mark the root cluster as free so that
				 * markDosDirChain below won't think the first root
				 * cluster is cross-linked to itself.
				 */
				markFree(boot->RootCl);
			} else
				return FSFATAL;
		}

		rootDir->head = boot->RootCl;
		ret |= markDosDirChain(boot, rootDir);
	}

	return ret;
}

/*
 * Cleanup after a directory scan
 */
void
finishDosDirSection()
{
	struct dirTodoNode *p, *np;
	struct dosDirEntry *d, *nd;

	for (p = pendingDirectories; p; p = np) {
		np = p->next;
		freeDirTodo(p);
	}
	pendingDirectories = 0;
	for (d = rootDir; d; d = nd) {
		if ((nd = d->child) != NULL) {
			d->child = 0;
			continue;
		}
		if (!(nd = d->next))
			nd = d->parent;
		freeDosDirEntry(d);
	}
	rootDir = lostDir = NULL;
	free(buffer);
	free(delbuf);
	buffer = NULL;
	delbuf = NULL;
}

/*
 * Delete a range of directory entries.
 *
 * Inputs:
 *  fd          File descriptor.
 *  startcl     Cluster number containing first directory entry.
 *  startoff    Offset within cluster of first directory entry.
 *  endcl       Cluster number containing last directory entry.
 *  endoff      Offset within cluster beyond last byte of last directory entry.
 *  notlast     If true, don't delete the directory entries in the last cluster
 *              (endcl); the caller already has that cluster in memory and will
 *              update those entries itself.
 */
static int
delete(int fd, struct bootblock *boot, cl_t startcl, size_t startoff, cl_t endcl, size_t endoff, int notlast)
{
	u_char *s, *e;
	off_t off;
	int clsz = boot->SecPerClust * boot->BytesPerSec;

	s = delbuf + startoff;
	e = delbuf + clsz;
	while (startcl >= CLUST_FIRST && startcl < boot->NumClusters) {
		if (startcl == endcl) {
			if (notlast)
				break;
			e = delbuf + endoff;
		}
		off = (startcl - CLUST_FIRST) * boot->SecPerClust + boot->ClusterOffset;
		off *= boot->BytesPerSec;
		if (lseek(fd, off, SEEK_SET) != off
		    || read(fd, delbuf, clsz) != clsz) {
			perr("Unable to read directory");
			return FSFATAL;
		}
		while (s < e) {
			*s = SLOT_DELETED;
			s += 32;
		}
		if (lseek(fd, off, SEEK_SET) != off
		    || write(fd, delbuf, clsz) != clsz) {
			perr("Unable to write directory");
			return FSFATAL;
		}
		if (startcl == endcl)
			break;
		startcl = fat_get(startcl);
		if (startcl == CLUST_ERROR)
			return FSFATAL;
		s = delbuf;
	}
	return FSOK;
}

static int
msdosfs_removede(f, boot, start, end, startcl, endcl, curcl, path, type)
	int f;
	struct bootblock *boot;
	u_char *start;
	u_char *end;
	cl_t startcl;
	cl_t endcl;
	cl_t curcl;
	char *path;
	int type;
{
	switch (type) {
	case 0:
		pwarn("Invalid long filename entry for %s\n", path);
		break;
	case 1:
		pwarn("Invalid long filename entry at end of directory %s\n", path);
		break;
	case 2:
		pwarn("Invalid long filename entry for volume label\n");
		break;
	}
	if (ask(0, "Remove")) {
		if (startcl != curcl) {
			if (delete(f, boot,
				   startcl, start - buffer,
				   endcl, end - buffer,
				   endcl == curcl) == FSFATAL)
				return FSFATAL;
			start = buffer;
		}
		if (endcl == curcl)
			for (; start < end; start += 32)
				*start = SLOT_DELETED;
		return FSDIRMOD;
	}
	return FSERROR;
}

/*
 * Check the size of a file represented by an in-memory file entry
 *
 * Assumes our caller has checked that dir->head is a valid cluster number.
 * Assumes that markDosDirChain has been called, and that dir->physicalSize
 * has been set up.
 */
static int
checksize(boot, p, dir)
	struct bootblock *boot;
	u_char *p;
	struct dosDirEntry *dir;
{
	/*
	 * Check size on ordinary files
	 */
	if (dir->physicalSize < dir->size) {
		pwarn("size of %s is %u, should at most be %llu\n",
		      fullpath(dir), dir->size, dir->physicalSize);
		if (ask(1, "Truncate")) {
			dir->size = (uint32_t)dir->physicalSize;
			p[28] = (u_char)dir->physicalSize;
			p[29] = (u_char)(dir->physicalSize >> 8);
			p[30] = (u_char)(dir->physicalSize >> 16);
			p[31] = (u_char)(dir->physicalSize >> 24);
			return FSDIRMOD;
		} else
			return FSERROR;
	} else if (dir->physicalSize - dir->size >= boot->ClusterSize) {
		pwarn("%s has too many clusters allocated (logical=%u, physical=%llu)\n",
		      fullpath(dir), dir->size, dir->physicalSize);
		if (ask(1, "Drop superfluous clusters")) {
			int mod = 0;
			cl_t cl, next;
			u_int64_t sz = 0;

			if (dir->size == 0)
			{
				/* Set "first cluster" in directory to 0 */
				p[20]=p[21]=p[26]=p[27] = 0;
				mod = FSDIRMOD;
				
				/* Begin deallocating with the previous first cluster */
				cl = dir->head;
			}
			else
			{
				/*
				 * Skip over the clusters containing the first dir->size bytes
				 */
				for (cl = dir->head; (sz += boot->ClusterSize) < dir->size;)
				{
					cl = fat_get(cl);
					if (cl == CLUST_ERROR)
						return FSFATAL;
				}

				/* When we get here, "cl" is the new last cluster of the file */
				
				/*
				 * Remember the first cluster to be dropped.
				 * Mark the new last cluster as CLUST_EOF.
				 */
				next = fat_get(cl);
				if (next == CLUST_ERROR)
					return FSFATAL;
				if (fat_set(cl, CLUST_EOF))
					return FSFATAL;
				cl = next;
			}
			
			/*
			 * Free the clusters up to physicalSize
			 *
			 * NOTE: We can't just follow the chain to CLUST_EOF because it
			 * might be cross-linked with some other chain.  Assumes
			 * dir->physicalSize is set to the size of the chain before any
			 * error or cross-link.
			 */
			while (sz < dir->physicalSize)
			{
				next = fat_get(cl);
				if (next == CLUST_ERROR)
					return FSFATAL;
				if (fat_set(cl, CLUST_FREE))
					return FSFATAL;
				cl = next;
				sz += boot->ClusterSize;
			}
			
			return mod | FSFATMOD;
		} else
			return FSERROR;
	}
	return FSOK;
}


/*
 * Is the given directory entry really a subdirectory?
 *
 * Read the first cluster of the given subdirectory and check whether the
 * first two short names are "." and "..".
 *
 * Return values:
 *  0           Is a valid subdirectory
 *  ENOTDIR     Is not a valid subdirectory
 *  ENOMEM      Unable to allocate memory for an I/O buffer
 *  EIO         Unable to read from the subdirectory
 */
static errno_t isSubdirectory(int fd, struct bootblock *boot, struct dosDirEntry *dir)
{
    off_t offset;
    ssize_t amount;
    char *buf;
    errno_t err = 0;
    
    buf = malloc(boot->BytesPerSec);
    if (buf == NULL) {
        perr("No memory for subdirectory buffer");
        return ENOMEM;
    }

    offset = (((off_t)dir->head - CLUST_FIRST) * boot->SecPerClust + boot->ClusterOffset) * boot->BytesPerSec;
    amount = pread(fd, buf, boot->BytesPerSec, offset);
    if (amount != boot->BytesPerSec) {
        pfatal("Unable to read cluster %u", dir->head);
        err = EIO;
        goto fail;
    }

    /* Make sure the first two children are named "." and ".." */
    if (memcmp(buf, ".          ", 11) || memcmp(buf+32, "..         ", 11)) {
        err = ENOTDIR;
        goto fail;
    }
    
    /* Make sure "." and ".." are marked as directories */
    if ((buf[11] & ATTR_DIRECTORY) == 0 || (buf[32+11] & ATTR_DIRECTORY) == 0) {
        err = ENOTDIR;
    }
    
fail:
    free(buf);
    return err;
}

/*
 * Read a directory and
 *   - resolve long name records
 *   - enter file and directory records into the parent's list
 *   - push directories onto the todo-stack
 */
static int
readDosDirSection(f, boot, dir)
	int f;
	struct bootblock *boot;
	struct dosDirEntry *dir;
{
	struct dosDirEntry dirent, *d;
	u_char *p, *vallfn, *invlfn, *empty;
	off_t off;
	int i, j, k, last;
	cl_t cl, valcl = ~0, invcl = ~0, empcl = ~0;
	cl_t last_cl;
	char *t;
	u_int lidx = 0;
	int shortSum;
	int mod = FSOK;
#define	THISMOD	0x8000			/* Only used within this routine */

	cl = dir->head;
	if (dir->parent && (cl < CLUST_FIRST || cl >= boot->NumClusters)) {
		/*
		 * Already handled somewhere else.
		 */
		fprintf(stderr, "readDosDirSection: Start cluster (%u) out of range; ignoring\n", cl);
		return FSOK;
	}
	
	shortSum = -1;
	vallfn = invlfn = empty = NULL;
	do {
		last_cl = cl;	/* Remember last cluster accessed before exiting loop */

        /*
         * Get the starting offset and length of the current chunk of the
         * directory.
         */
		if (!(boot->flags & FAT32) && !dir->parent) {
			last = boot->RootDirEnts * 32;
			off = boot->ResSectors + boot->FATs * boot->FATsecs;
		} else {
			last = boot->SecPerClust * boot->BytesPerSec;
			off = (cl - CLUST_FIRST) * boot->SecPerClust + boot->ClusterOffset;
		}

		off *= boot->BytesPerSec;
		if (lseek(f, off, SEEK_SET) != off
		    || read(f, buffer, last) != last) {
			perr("Unable to read directory");
			return FSFATAL;
		}
		last /= 32;

		/*
         * For each "slot" in the directory...
         */
        for (p = buffer, i = 0; i < last; i++, p += 32) {
			if (dir->fsckflags & DIREMPWARN) {
				*p = SLOT_EMPTY;
				mod |= THISMOD|FSDIRMOD;
				continue;
			}

			if (*p == SLOT_EMPTY || *p == SLOT_DELETED) {
				if (*p == SLOT_EMPTY) {
					dir->fsckflags |= DIREMPTY;
					empty = p;
					empcl = cl;
				}
				continue;
			}

			if (dir->fsckflags & DIREMPTY) {
				if (!(dir->fsckflags & DIREMPWARN)) {
					pwarn("%s has entries after end of directory\n",
					      fullpath(dir));
					if (ask(1, "Truncate"))
						dir->fsckflags |= DIREMPWARN;
					else if (ask(0, "Extend")) {
						u_char *q;

						dir->fsckflags &= ~DIREMPTY;
						if (delete(f, boot,
							   empcl, empty - buffer,
							   cl, p - buffer, 1) == FSFATAL)
							return FSFATAL;
						q = empcl == cl ? empty : buffer;
						for (; q < p; q += 32)
							*q = SLOT_DELETED;
						mod |= THISMOD|FSDIRMOD;
					} 
				}
				if (dir->fsckflags & DIREMPWARN) {
					*p = SLOT_EMPTY;
					mod |= THISMOD|FSDIRMOD;
					continue;
				} else if (dir->fsckflags & DIREMPTY)
					mod |= FSERROR;
				empty = NULL;
			}

            /*
             * Check long name entries
             */
			if (p[11] == ATTR_WIN95) {
                /* Remember or validate the long name checksum */
				if (*p & LRFIRST) {
					if (shortSum != -1) {
						if (!invlfn) {
							invlfn = vallfn;
							invcl = valcl;
						}
					}
					memset(longName, 0, sizeof longName);
					shortSum = p[13];
					vallfn = p;
					valcl = cl;
				} else if (shortSum != p[13]
					   || lidx != (*p & LRNOMASK)) {
					if (!invlfn) {
						invlfn = vallfn;
						invcl = valcl;
					}
					if (!invlfn) {
						invlfn = p;
						invcl = cl;
					}
					vallfn = NULL;
				}
				lidx = *p & LRNOMASK;
                
                if (lidx < 1) {
                    pwarn("long file name is not available\n");
                    if (!invlfn) {
                        invlfn = vallfn;
                        invcl = valcl;
                    }
                    vallfn = NULL;
                } else {
                    /*
                     * Gather the characters from this long name entry
                     */
                    t = longName + --lidx * 13;
                    for (k = 1; k < 11 && t < longName + sizeof(longName); k += 2) {
                        if (!p[k] && !p[k + 1])
                            break;
                        *t++ = p[k];
                        /*
                         * Warn about those unusable chars in msdosfs here?    XXX
                         */
                        if (p[k + 1])
                            t[-1] = '?';
                    }
                    if (k >= 11)
                        for (k = 14; k < 26 && t < longName + sizeof(longName); k += 2) {
                            if (!p[k] && !p[k + 1])
                                break;
                            *t++ = p[k];
                            if (p[k + 1])
                                t[-1] = '?';
                        }
                    if (k >= 26)
                        for (k = 28; k < 32 && t < longName + sizeof(longName); k += 2) {
                            if (!p[k] && !p[k + 1])
                                break;
                            *t++ = p[k];
                            if (p[k + 1])
                                t[-1] = '?';
                        }
                    
                    if (t >= longName + sizeof(longName)) {
                        pwarn("long filename too long\n");
                        if (!invlfn) {
                            invlfn = vallfn;
                            invcl = valcl;
                        }
                        vallfn = NULL;
                    }
                    if (p[26] | (p[27] << 8)) {
                        pwarn("long filename record cluster start != 0\n");
                        if (!invlfn) {
                            invlfn = vallfn;
                            invcl = cl;
                        }
                        vallfn = NULL;
                    }
                }
				continue;	/* long records don't carry further
						 * information */
			}

			/*
			 * This is a standard msdosfs directory entry.
			 */
			memset(&dirent, 0, sizeof dirent);

			/*
			 * it's a short name record, but we need to know
			 * more, so get the flags first.
			 */
			dirent.flags = p[11];

			/*
             * Gather the base name of the short name (the "8" in "8.3").
             * Remove any trailing space padding.
			 */
			for (j = 0; j < 8; j++)
				dirent.name[j] = p[j];
			dirent.name[8] = '\0';
			for (k = 8; k > 0 && dirent.name[k - 1] == ' '; k--)
				dirent.name[k - 1] = '\0';
			if (dirent.name[0] == SLOT_E5)
				dirent.name[0] = 0xe5;

			if (dirent.flags & ATTR_VOLUME) {
				if (vallfn || invlfn) {
					mod |= msdosfs_removede(f, boot,
							invlfn ? invlfn : vallfn, p,
							invlfn ? invcl : valcl, cl, cl,
							fullpath(dir), 2);
					vallfn = NULL;
					invlfn = NULL;
				}
				continue;
			}

            /*
             * Gather the extension of the short name (if any).
             */
			if (p[8] != ' ')
				dirent.name[k++] = '.';
			for (j = 0; j < 3; j++)
				dirent.name[k++] = p[j+8];
			dirent.name[k] = '\0';
			for (k--; k >= 0 && dirent.name[k] == ' '; k--)
				dirent.name[k] = '\0';

            /* If there was a long name, make sure its checksum matches. */
			if (vallfn && shortSum != calcShortSum(p)) {
				if (!invlfn) {
					invlfn = vallfn;
					invcl = valcl;
				}
				vallfn = NULL;
			}
            
            /* Get the starting cluster number field(s) */
			dirent.head = p[26] | (p[27] << 8);
			if (boot->ClustMask == CLUST32_MASK)
				dirent.head |= (p[20] << 16) | (p[21] << 24);
            /* Get the file size */
			dirent.size = p[28] | (p[29] << 8) | (p[30] << 16) | (p[31] << 24);
            /* Copy the long name, if there is one */
			if (vallfn) {
				strlcpy(dirent.lname, longName, sizeof(dirent.lname));
				longName[0] = '\0';
				shortSum = -1;
			}

			dirent.parent = dir;
			dirent.next = dir->child;

			if (invlfn) {
				mod |= k = msdosfs_removede(f, boot,
						    invlfn, vallfn ? vallfn : p,
						    invcl, vallfn ? valcl : cl, cl,
						    fullpath(&dirent), 0);
				if (mod & FSFATAL)
					return FSFATAL;
				if (vallfn
				    ? (valcl == cl && vallfn != buffer)
				    : p != buffer)
					if (k & FSDIRMOD)
						mod |= THISMOD;
			}

			vallfn = NULL; /* not used any longer */
			invlfn = NULL;

			if (!strcmp(dirent.name, ".") || !strcmp(dirent.name,".."))
			{
				/*
				 * Don't do size or in-use checks for "." or ".."
				 * They'll be checked more below.
				 */
				goto MarkedChain;
			}
			else
			{
				if (dirent.head == CLUST_FREE)
				{
					if (dirent.flags & ATTR_DIRECTORY || dirent.size != 0)
					{
						pwarn("%s has no clusters\n", fullpath(&dirent));
						goto remove_or_truncate;
					}
				}
				else
				{
					cl_t next;
					
					if (dirent.head < CLUST_FIRST || dirent.head >= boot->NumClusters)
					{
						pwarn("%s starts with cluster out of range (%u)\n",
								fullpath(&dirent), dirent.head);
						goto remove_or_truncate;
					}
					
					if (isUsed(dirent.head))
					{
						pwarn("%s starts with cross-linked cluster (%u)\n",
								fullpath(&dirent), dirent.head);
						goto remove_or_truncate;
					}
					
					next = fat_get(dirent.head);
					if (next == CLUST_ERROR)
						return FSFATAL;
					
					if (next == CLUST_FREE)
					{
						pwarn("%s starts with free cluster\n", fullpath(&dirent));
						goto remove_or_truncate;
					}
					
					if (next >= CLUST_RSRVD && next < CLUST_EOFS)
					{
						pwarn("%s starts with cluster marked %s\n",
							  fullpath(&dirent),
							  rsrvdcltype(next));
	remove_or_truncate:
						if (dirent.flags & ATTR_DIRECTORY) {
							if (ask(0, "Remove")) {
								*p = SLOT_DELETED;
								mod |= THISMOD|FSDIRMOD;
							} else
								mod |= FSERROR;
							continue;
						} else {
							if (ask(1, "Truncate")) {
								p[28] = p[29] = p[30] = p[31] = 0;
								p[26] = p[27] = 0;
								if (boot->ClustMask == CLUST32_MASK)
									p[20] = p[21] = 0;
								dirent.head = 0;
								dirent.size = 0;
								mod |= THISMOD|FSDIRMOD;
							} else
								mod |= FSERROR;
						}
					}
				}
			}

			if (dirent.head >= CLUST_FIRST && dirent.head < boot->NumClusters)
			{
				mod |= markDosDirChain(boot, &dirent);
				if (mod & FSFATAL)
					return FSFATAL;
			}

MarkedChain:
			if (dirent.flags & ATTR_DIRECTORY) {
				/*
				 * gather more info for directories
				 */
				if (dirent.size) {
					pwarn("Directory %s has size != 0\n",
					      fullpath(&dirent));
					if (ask(1, "Correct")) {
						p[28] = p[29] = p[30] = p[31] = 0;
						dirent.size = 0;
						mod |= THISMOD|FSDIRMOD;
					} else
						mod |= FSERROR;
				}
				/*
				 * handle `.' and `..' specially
				 */
				if (strcmp(dirent.name, ".") == 0) {
					if (dirent.head != dir->head) {
						pwarn("`.' entry in %s has incorrect start cluster\n",
						      fullpath(dir));
						if (ask(1, "Correct")) {
							dirent.head = dir->head;
							p[26] = (u_char)dirent.head;
							p[27] = (u_char)(dirent.head >> 8);
							if (boot->ClustMask == CLUST32_MASK) {
								p[20] = (u_char)(dirent.head >> 16);
								p[21] = (u_char)(dirent.head >> 24);
							}
							mod |= THISMOD|FSDIRMOD;
						} else
							mod |= FSERROR;
					}
					continue;
				}
				if (strcmp(dirent.name, "..") == 0) {
					if (dir->parent) {		/* XXX */
						if (!dir->parent->parent) {
							if (dirent.head) {
								pwarn("`..' entry in %s has non-zero start cluster\n",
								      fullpath(dir));
								if (ask(1, "Correct")) {
									dirent.head = 0;
									p[26] = p[27] = 0;
									if (boot->ClustMask == CLUST32_MASK)
										p[20] = p[21] = 0;
									mod |= THISMOD|FSDIRMOD;
								} else
									mod |= FSERROR;
							}
						} else if (dirent.head != dir->parent->head) {
							pwarn("`..' entry in %s has incorrect start cluster\n",
							      fullpath(dir));
							if (ask(1, "Correct")) {
								dirent.head = dir->parent->head;
								p[26] = (u_char)dirent.head;
								p[27] = (u_char)(dirent.head >> 8);
								if (boot->ClustMask == CLUST32_MASK) {
									p[20] = (u_char)(dirent.head >> 16);
									p[21] = (u_char)(dirent.head >> 24);
								}
								mod |= THISMOD|FSDIRMOD;
							} else
								mod |= FSERROR;
						}
					}
					continue;
				}

                /*
                 * We've found something that claims to be a subdirectory.
                 * Make sure the contents of the first cluster contain "."
                 * and ".." entries; if not, assume this is actually a file.
                 */
                errno_t err = isSubdirectory(f, boot, &dirent);
                if (err) {
                    if (err == ENOTDIR) {
                        pwarn("Item %s does not appear to be a subdirectory\n", fullpath(&dirent));
                        if (ask(0, "Correct")) {
                            p[11] &= ~ATTR_DIRECTORY;
                            dirent.flags &= ~ATTR_DIRECTORY;
                            mod |= THISMOD|FSDIRMOD;
                            goto check_file;
                        } else {
                            mod |= FSERROR;
                        }
                    } else {
                        return FSFATAL;
                    }
                }
                
				/* create directory tree node */
				if (!(d = newDosDirEntry())) {
					perr("No space for directory");
					return FSFATAL;
				}
				memcpy(d, &dirent, sizeof(struct dosDirEntry));
				/* link it into the tree */
				dir->child = d;

				/* Enter this directory into the todo list */
                struct dirTodoNode *n;
				if (!(n = newDirTodo())) {
					perr("No space for todo list");
					return FSFATAL;
				}
				n->next = pendingDirectories;
				n->dir = d;
				pendingDirectories = n;
			} else {
            check_file:
				mod |= k = checksize(boot, p, &dirent);
				if (k & FSDIRMOD)
					mod |= THISMOD;
			}
			boot->NumFiles++;
		}
		if (mod & THISMOD) {
			if (lseek(f, off, SEEK_SET) != off
			    || write(f, buffer, last*32) != last*32) {
				perr("Unable to write directory");
				return FSFATAL;
			}
			mod &= ~THISMOD;
		}

		/*
		 * If this is a FAT12 or FAT16 root directory, there is no cluster chain
		 * to follow.  In this case, we'll exit the loop with cl==0.
		 */
		if (!(boot->flags & FAT32) && !dir->parent)
			break;
	
	/* What about errors below? */
	} while ((cl = fat_get(cl)) >= CLUST_FIRST && cl < boot->NumClusters && cl != dir->end);
	if (cl == CLUST_ERROR)
		mod |= FSFATAL;
	if (invlfn || vallfn)
	{
		mod |= msdosfs_removede(f, boot,
				invlfn ? invlfn : vallfn, p,
				invlfn ? invcl : valcl, last_cl, last_cl,
				fullpath(dir), 1);
		if (lseek(f, off, SEEK_SET) != off
			|| write(f, buffer, last*32) != last*32) {
			perr("Unable to write directory");
			return FSFATAL;
		}
	}
	return mod & ~THISMOD;
}

int
handleDirTree(int dosfs, struct bootblock *boot)
{
	int mod;

	mod = readDosDirSection(dosfs, boot, rootDir);
	if (mod & FSFATAL)
		return FSFATAL;

	/*
	 * process the directory todo list
	 */
	while (pendingDirectories) {
		struct dosDirEntry *dir = pendingDirectories->dir;
		struct dirTodoNode *n = pendingDirectories->next;

		/*
		 * remove TODO entry now, the list might change during
		 * directory reads
		 */
		freeDirTodo(pendingDirectories);
		pendingDirectories = n;

		/*
		 * handle subdirectory
		 */
		mod |= readDosDirSection(dosfs, boot, dir);
		if (mod & FSFATAL)
			return FSFATAL;
	}

	return mod;
}
