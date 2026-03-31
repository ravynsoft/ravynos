/*-
 * MD5.C
 *
 * (c) Copyright 1997-1999 by Matthew Dillon and Dima Ruban.  Permission to
 *     use and distribute based on the FreeBSD copyright.  Supplied as-is,
 *     USE WITH EXTREME CAUTION.
 */

#include "cpdup.h"

#include <openssl/md5.h>

typedef struct MD5Node {
    struct MD5Node *md_Next;
    char *md_Name;
    char *md_Code;
    int md_Accessed;
} MD5Node;

static MD5Node *md5_lookup(const char *sfile);
static void md5_cache(const char *spath, int sdirlen);
static char *doMD5File(const char *filename, char *buf, int is_target);

static char *MD5SCache;		/* cache source directory name */
static MD5Node *MD5Base;
static int MD5SCacheDirLen;
static int MD5SCacheDirty;

void
md5_flush(void)
{
    if (MD5SCacheDirty && MD5SCache && NotForRealOpt == 0) {
	FILE *fo;

	if ((fo = fopen(MD5SCache, "w")) != NULL) {
	    MD5Node *node;

	    for (node = MD5Base; node; node = node->md_Next) {
		if (node->md_Accessed && node->md_Code) {
		    fprintf(fo, "%s %zu %s\n",
			node->md_Code,
			strlen(node->md_Name),
			node->md_Name
		    );
		}
	    }
	    fclose(fo);
	}
    }

    MD5SCacheDirty = 0;

    if (MD5SCache) {
	MD5Node *node;

	while ((node = MD5Base) != NULL) {
	    MD5Base = node->md_Next;

	    if (node->md_Code)
		free(node->md_Code);
	    if (node->md_Name)
		free(node->md_Name);
	    free(node);
	}
	free(MD5SCache);
	MD5SCache = NULL;
    }
}

static void
md5_cache(const char *spath, int sdirlen)
{
    FILE *fi;

    /*
     * Already cached
     */

    if (
	MD5SCache &&
	sdirlen == MD5SCacheDirLen &&
	strncmp(spath, MD5SCache, sdirlen) == 0
    ) {
	return;
    }

    /*
     * Different cache, flush old cache
     */

    if (MD5SCache != NULL)
	md5_flush();

    /*
     * Create new cache
     */

    MD5SCacheDirLen = sdirlen;
    MD5SCache = mprintf("%*.*s%s", sdirlen, sdirlen, spath, MD5CacheFile);

    if ((fi = fopen(MD5SCache, "r")) != NULL) {
	MD5Node **pnode = &MD5Base;
	int c;

	c = fgetc(fi);
	while (c != EOF) {
	    MD5Node *node = *pnode = malloc(sizeof(MD5Node));
	    char *s;
	    int nlen;

	    nlen = 0;

	    if (pnode == NULL || node == NULL) {
		fprintf(stderr, "out of memory\n");
		exit(EXIT_FAILURE);
	    }

	    bzero(node, sizeof(MD5Node));
	    node->md_Code = fextract(fi, -1, &c, ' ');
	    node->md_Accessed = 1;
	    if ((s = fextract(fi, -1, &c, ' ')) != NULL) {
		nlen = strtol(s, NULL, 0);
		free(s);
	    }
	    /*
	     * extracting md_Name - name may contain embedded control
	     * characters.
	     */
	    CountSourceReadBytes += nlen+1;
	    node->md_Name = fextract(fi, nlen, &c, EOF);
	    if (c != '\n') {
		fprintf(stderr, "Error parsing MD5 Cache: %s (%c)\n", MD5SCache, c);
		while (c != EOF && c != '\n')
		    c = fgetc(fi);
	    }
	    if (c != EOF)
		c = fgetc(fi);
	    pnode = &node->md_Next;
	}
	fclose(fi);
    }
}

/*
 * md5_lookup:	lookup/create md5 entry
 */

static MD5Node *
md5_lookup(const char *sfile)
{
    MD5Node **pnode;
    MD5Node *node;

    for (pnode = &MD5Base; (node = *pnode) != NULL; pnode = &node->md_Next) {
	if (strcmp(sfile, node->md_Name) == 0) {
	    break;
	}
    }
    if (node == NULL) {

	if ((node = *pnode = malloc(sizeof(MD5Node))) == NULL) {
		fprintf(stderr,"out of memory\n");
		exit(EXIT_FAILURE);
	}

	bzero(node, sizeof(MD5Node));
	node->md_Name = strdup(sfile);
    }
    node->md_Accessed = 1;
    return(node);
}

/*
 * md5_check:  check MD5 against file
 *
 *	Return -1 if check failed
 *	Return 0  if check succeeded
 *
 * dpath can be NULL, in which case we are force-updating
 * the source MD5.
 */
int
md5_check(const char *spath, const char *dpath)
{
    const char *sfile;
    char *dcode;
    int sdirlen;
    int r;
    MD5Node *node;

    r = -1;

    if ((sfile = strrchr(spath, '/')) != NULL)
	++sfile;
    else
	sfile = spath;
    sdirlen = sfile - spath;

    md5_cache(spath, sdirlen);

    node = md5_lookup(sfile);

    /*
     * If dpath == NULL, we are force-updating the source .MD5* files
     */

    if (dpath == NULL) {
	char *scode = doMD5File(spath, NULL, 0);

	r = 0;
	if (node->md_Code == NULL) {
	    r = -1;
	    node->md_Code = scode;
	    MD5SCacheDirty = 1;
	} else if (strcmp(scode, node->md_Code) != 0) {
	    r = -1;
	    free(node->md_Code);
	    node->md_Code = scode;
	    MD5SCacheDirty = 1;
	} else {
	    free(scode);
	}
	return(r);
    }

    /*
     * Otherwise the .MD5* file is used as a cache.
     */

    if (node->md_Code == NULL) {
	node->md_Code = doMD5File(spath, NULL, 0);
	MD5SCacheDirty = 1;
    }

    dcode = doMD5File(dpath, NULL, 1);
    if (dcode) {
	if (strcmp(node->md_Code, dcode) == 0) {
	    r = 0;
	} else {
	    char *scode = doMD5File(spath, NULL, 0);

	    if (strcmp(node->md_Code, scode) == 0) {
		    free(scode);
	    } else {
		    free(node->md_Code);
		    node->md_Code = scode;
		    MD5SCacheDirty = 1;
		    if (strcmp(node->md_Code, dcode) == 0)
			r = 0;
	    }
	}
	free(dcode);
    }
    return(r);
}

static char *
md5_file(const char *filename, char *buf)
{
    unsigned char digest[MD5_DIGEST_LENGTH];
    static const char hex[]="0123456789abcdef";
    MD5_CTX ctx;
    unsigned char buffer[4096];
    struct stat st;
    off_t size;
    int fd, bytes, i;

    fd = open(filename, O_RDONLY);
    if (fd < 0)
	return NULL;
    if (fstat(fd, &st) < 0) {
	bytes = -1;
	goto err;
    }

    MD5_Init(&ctx);
    size = st.st_size;
    bytes = 0;
    while (size > 0) {
	if ((size_t)size > sizeof(buffer))
	     bytes = read(fd, buffer, sizeof(buffer));
	else
	     bytes = read(fd, buffer, size);
	if (bytes < 0)
	     break;
	MD5_Update(&ctx, buffer, bytes);
	size -= bytes;
    }

err:
    close(fd);
    if (bytes < 0)
	return NULL;

    if (!buf)
	buf = malloc(MD5_DIGEST_LENGTH * 2 + 1);
    if (!buf)
	return NULL;

    MD5_Final(digest, &ctx);
    for (i = 0; i < MD5_DIGEST_LENGTH; i++) {
	buf[2*i] = hex[digest[i] >> 4];
	buf[2*i+1] = hex[digest[i] & 0x0f];
    }
    buf[MD5_DIGEST_LENGTH * 2] = '\0';

    return buf;
}

char *
doMD5File(const char *filename, char *buf, int is_target)
{
    if (SummaryOpt) {
	struct stat st;
	if (stat(filename, &st) == 0) {
	    uint64_t size = st.st_size;
	    if (is_target)
		    CountTargetReadBytes += size;
	    else
		    CountSourceReadBytes += size;
	}
    }

    return md5_file(filename, buf);
}

