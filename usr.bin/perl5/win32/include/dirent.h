/* dirent.h */

/* djl
 * Provide UNIX compatibility
 */

#ifndef  _INC_DIRENT
#define  _INC_DIRENT

/*
 * NT versions of readdir(), etc
 * From the MSDOS implementation
 */

/* Directory entry size */
#ifdef DIRSIZ
#undef DIRSIZ
#endif
#define DIRSIZ(rp)  (sizeof(struct direct))

/* needed to compile directory stuff */
#define DIRENT direct

/* structure of a directory entry */
typedef struct direct 
{
        long	d_ino;			/* inode number (not used by MS-DOS)  */
        long	d_namlen;		/* name length  */
        char	d_name[257];		/* file name  */
} _DIRECT;

/* structure for dir operations */
typedef struct _dir_struc
{
        char	*start;			/* starting position */
        char	*curr;			/* current position */
        long	size;			/* allocated size of string table */
        long	nfiles;			/* number of filenames in table */
        struct direct dirstr;		/* directory structure to return */
        void*	handle;			/* system handle */
        char	*end;			/* position after last filename */
} DIR;

#if 0		/* these have moved to win32iop.h */
DIR *		win32_opendir(const char *filename);
struct direct *	win32_readdir(DIR *dirp);
long		win32_telldir(DIR *dirp);
void		win32_seekdir(DIR *dirp,long loc);
void		win32_rewinddir(DIR *dirp);
int		win32_closedir(DIR *dirp);
#endif

#endif /* _INC_DIRENT */
