/*
 * Copyright (c) 2005-2008 Rob Braun
 * Portions Copyright (c) 2012 Kyle J. McKay.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of Rob Braun nor the names of his contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
/*
 * 03-Apr-2005
 * DRI: Rob Braun <bbraun@synack.net>
 */
/*
 * Portions Copyright 2006, Apple Computer, Inc.
 * Christopher Ryan <ryanc@apple.com>
 */

#define _FILE_OFFSET_BITS 64

#include "config.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <libgen.h>
#include <errno.h>
#include <limits.h>
#include <sys/stat.h>
#include <arpa/inet.h> /* for ntoh{l,s} */
#include <inttypes.h>  /* for PRIu64 */
#include <libxml/xmlwriter.h>
#include <libxml/xmlreader.h>
#include <libxml/xmlstring.h>
#ifndef HAVE_ASPRINTF
#include "asprintf.h"
#endif
#include "xar.h"
#include "filetree.h"
#include "archive.h"
#include "hash.h"
#include "signature.h"
#include "arcmod.h"
#include "io.h"
#include "util.h"
#include "subdoc.h"
#include "darwinattr.h"

#define _XAR_LIB_VERSION2(x) #x
#define _XAR_LIB_VERSION1(x) _XAR_LIB_VERSION2(x)
#define XAR_LIB_VERSION() _XAR_LIB_VERSION1(XAR_VERSION_NUM)

#ifndef O_EXLOCK
#define O_EXLOCK 0
#endif
#ifndef O_SHLOCK
#define O_SHLOCK 0
#endif

#ifndef LONG_MAX
#define LONG_MAX INT32_MAX
#endif
#ifndef LONG_MIN
#define LONG_MIN INT32_MIN
#endif

#if LIBXML_VERSION < 20618
#define xmlDictCleanup()	/* function doesn't exist in older API */
#endif

static int32_t xar_unserialize(xar_t x);
void xar_serialize(xar_t x, const char *file);

/* xar_new
 * Returns: newly allocated xar_t structure
 * Summary: just does basicallocation and initialization of 
 * xar_t structure.
 */
static xar_t xar_new() {
	xar_t ret;
	ret = malloc(sizeof(struct __xar_t));
	if(!ret) return NULL;
	memset(XAR(ret), 0, sizeof(struct __xar_t));
	XAR(ret)->readbuf_len = XAR_DEFAULT_BUFFER_SIZE;
	XAR(ret)->readbuf = malloc(XAR(ret)->readbuf_len);
	if(!XAR(ret)->readbuf) {
		free((void *)ret);
		return NULL;
	}
	XAR(ret)->offset = 0;

	XAR(ret)->zs.zalloc = Z_NULL;
	XAR(ret)->zs.zfree = Z_NULL;
	XAR(ret)->zs.opaque = Z_NULL;
	XAR(ret)->ino_hash = xmlHashCreate(0);
	XAR(ret)->link_hash = xmlHashCreate(0);
	XAR(ret)->csum_hash = xmlHashCreate(0);
	XAR(ret)->subdocs = NULL;
	XAR(ret)->toc_ctx = EVP_MD_CTX_create();
	if(!XAR(ret)->toc_ctx) {
		free(XAR(ret)->readbuf);
		free((void *)ret);
		return NULL;
	}

	return ret;
}

/* xar_parse_header
 * x: archive to operate on.
 * Returns: 0 on success, -1 on failure
 * Summary: internal helper function to read in the xar header.
 */
static int32_t xar_parse_header(xar_t x) {
	ssize_t r;
	int off;
	int sz2read = 0;

	/* read just the magic, verify it, read the header length,
	 * then read in the size of the header according to the
	 * recorded header length, or the length of the structure
	 * we expect, whichever is smaller.  Then seek forward
	 * if the recorded header length is greater than the 
	 * expected header length.
	 */
	r = xar_read_fd(XAR(x)->fd, (char *)&XAR(x)->header.magic, sizeof(XAR(x)->header.magic));
	if ( r == -1 )
		return -1;

	/* Verify the header.  If the header doesn't match, exit without
	 * attempting to read any more.
	 */
	XAR(x)->header.magic = ntohl(XAR(x)->header.magic);

	if( XAR(x)->header.magic != XAR_HEADER_MAGIC ) {
		return -1;
	}

	r = xar_read_fd(XAR(x)->fd, (char *)&XAR(x)->header.size, sizeof(XAR(x)->header.size));
	if ( r == -1 )
		return -1;

	XAR(x)->header.size = ntohs(XAR(x)->header.size);

	if( XAR(x)->header.size > sizeof(xar_header_ex_t) )
		sz2read = (int)sizeof(xar_header_ex_t);
	else
		sz2read = XAR(x)->header.size;
	if ( XAR(x)->header.size < sizeof(xar_header_t) )
		return -1;

	off = (int)(sizeof(XAR(x)->header.magic) + sizeof(XAR(x)->header.size));
	r = xar_read_fd(XAR(x)->fd, ((char *)&XAR(x)->header)+off, sz2read-off);
	if ( r == -1 )
		return -1;

	XAR(x)->header.version = ntohs(XAR(x)->header.version);
	XAR(x)->header.toc_length_compressed = xar_ntoh64(XAR(x)->header.toc_length_compressed);
	XAR(x)->header.toc_length_uncompressed = xar_ntoh64(XAR(x)->header.toc_length_uncompressed);
	XAR(x)->header.cksum_alg = ntohl(XAR(x)->header.cksum_alg);

	if (XAR(x)->header.cksum_alg == XAR_CKSUM_OTHER) {
		/* Make sure toc_cksum_name is Nul terminated and included in header length */
		size_t max_search = XAR(x)->header.size;
		if (max_search > sizeof(xar_header_ex_t))
			max_search = sizeof(xar_header_ex_t);
		max_search -= sizeof(xar_header_t);
		if (XAR(x)->header.size < sizeof(xar_header_t) + 4 || (XAR(x)->header.size & 0x3) != 0)
			return -1; /* Does not include a name or not multiple of 4 */
		if (!memchr(XAR(x)->header.toc_cksum_name, 0, max_search))
			return -1; /* No Nul terminated string present for toc_cksum_name within header.size bytes */
	}

	off = XAR(x)->header.size - sz2read;
	if( off > 0 )
		r = (int)lseek(XAR(x)->fd, (off_t)off, SEEK_CUR);

	if ( (r == -1) && (errno != ESPIPE) ) {
		/* Some fatal error here perhaps? */
	}

	return 0;
}

/* xar_open
 * file: filename to open
 * flags: flags on how to open the file.  0 for readonly, !0 for read/write
 * Returns: allocated and initialized xar structure with an open
 * file descriptor to the target xar file.  If the xarchive is opened
 * for writing, the file is created, and a heap file is opened.
 */
xar_t xar_open(const char *file, int32_t flags) {
	xar_t ret;

	ret = xar_new();
	if( !ret ) return NULL;
	if( !file )
		file = "-";
	XAR(ret)->filename = strdup(file);
	if (!*file && flags) {
		XAR(ret)->fd = XAR(ret)->heap_fd = -2;
		return ret;
	}
	OpenSSL_add_all_digests();
	if( flags ) {
		char *tmp1, *tmp2, *tmp3, *tmp4;
		tmp1 = tmp2 = strdup(file);
		tmp3 = dirname(tmp2);
		XAR(ret)->dirname = strdup(tmp3);
		/* Create the heap file in the directory which will contain
		 * the target archive.  /tmp or elsewhere may fill up.
		 */
		if (asprintf(&tmp4, "%s/xar.heap.XXXXXX", tmp3) == -1) {
			free(XAR(ret)->dirname);
			free((void *)XAR(ret)->filename);
			free(XAR(ret));
			return NULL;
		}
		free(tmp1);
		if( strcmp(file, "-") == 0 )
			XAR(ret)->fd = 1;
		else{
			XAR(ret)->fd = open(file, O_WRONLY | O_CREAT | O_TRUNC | O_EXLOCK, 0644);
			if( (-1 == XAR(ret)->fd ) && (ENOTSUP == errno) ){
				XAR(ret)->fd = open(file, O_WRONLY | O_CREAT | O_TRUNC , 0644);				
			}
		}
		XAR(ret)->heap_fd = mkstemp(tmp4);
		if( XAR(ret)->heap_fd < 0 ) {
			close(XAR(ret)->fd);
			free(XAR(ret)->dirname);
			free((void *)XAR(ret)->filename);
			free(XAR(ret));
			return NULL;
		}
		unlink(tmp4);
		free(tmp4);

		deflateInit(&XAR(ret)->zs, Z_BEST_COMPRESSION);

		if( XAR(ret)->fd < 0 ) {
			xar_close(ret);
			free(XAR(ret)->dirname);
			free((void *)XAR(ret)->filename);
			free(XAR(ret));
			return NULL;
		}

		/* default to using sha1, if nothing else is
		 * specified.
		 */
		XAR(ret)->heap_offset += 20;
		XAR(ret)->heap_len += 20;
		
		xar_opt_set(ret, XAR_OPT_COMPRESSION, XAR_OPT_VAL_GZIP);
		xar_opt_set(ret, XAR_OPT_FILECKSUM, XAR_OPT_VAL_SHA1);
	} else {
		unsigned char toccksum[HASH_MAX_MD_SIZE];
		unsigned char cval[HASH_MAX_MD_SIZE];
		unsigned int tlen;
		const EVP_MD *md;
		int cksum_match;
		const char *cksum_style;
		const char *value;
		uint64_t offset;
		uint64_t length;

		if( strcmp(file, "-") == 0 )
			XAR(ret)->fd = 0;
		else{
			XAR(ret)->fd = open(file, O_RDONLY | O_SHLOCK);
			
			if( (-1 == XAR(ret)->fd ) && (ENOTSUP == errno) ){
				XAR(ret)->fd = open(file, O_RDONLY);
			}

		}
		XAR(ret)->heap_fd = -1;
		inflateInit(&XAR(ret)->zs);
		if( XAR(ret)->fd < 0 ) {
			xar_close(ret);
			return NULL;
		}

		if( xar_parse_header(ret) != 0 ) {
			xar_close(ret);
			return NULL;
		}

		switch(XAR(ret)->header.cksum_alg) {
		case XAR_CKSUM_MD5:
			strncpy(XAR(ret)->header.toc_cksum_name, XAR_OPT_VAL_MD5, sizeof(XAR(ret)->header.toc_cksum_name));
			break;
		case XAR_CKSUM_SHA1:
			strncpy(XAR(ret)->header.toc_cksum_name, XAR_OPT_VAL_SHA1, sizeof(XAR(ret)->header.toc_cksum_name));
			break;
		default:
			/* nothing to do */;
		};

		switch(XAR(ret)->header.cksum_alg) {
		case XAR_CKSUM_NONE:
			break;
		case XAR_CKSUM_MD5:
		case XAR_CKSUM_SHA1:
		case XAR_CKSUM_OTHER:
			md = EVP_get_digestbyname(XAR(ret)->header.toc_cksum_name);
			if (md) {
				XAR(ret)->docksum = 1;
				EVP_DigestInit_ex(XAR(ret)->toc_ctx, md, NULL);
				break;
			}
			/* else fall through */
		default:
			fprintf(stderr, "Unknown hashing algorithm, skipping\n");
			break;
		};

		if( xar_unserialize(ret) != 0 ) {
			xar_close(ret);
			return NULL;
		}

		/* check for inconsistency between checksum style in header, 
		 * and the one described in the TOC; otherwise, you can flip 
		 * the header bit to XAR_CKSUM_NONE, and nothing will ever 
		 * verify that the TOC matches the checksum stored in the 
		 * heap, and the signature check will pass on a modified 
		 * file! <rdar://problem/6134714>
		 */
		cksum_match = 0;
		cksum_style = xar_attr_get(XAR_FILE(ret), "checksum", "style");
		switch(XAR(ret)->header.cksum_alg) {
			case XAR_CKSUM_NONE:
				cksum_match = (cksum_style == NULL || strcmp(cksum_style, XAR_OPT_VAL_NONE) == 0);
				break;
			case XAR_CKSUM_MD5:
			case XAR_CKSUM_SHA1:
			case XAR_CKSUM_OTHER:
				cksum_match = (cksum_style != NULL && strcmp(cksum_style, XAR(ret)->header.toc_cksum_name) == 0);
				break;
			default:
				cksum_match = 0;
				break;
		}
		if( !cksum_match ) {
			fprintf(stderr, "Checksum style mismatch!\n");
			xar_close(ret);
			return NULL;
		}
		
		/* also check for consistency between the checksum style and 
		 * the existence (or not) of signatures: since the signature 
		 * is signing the checksum, we must have a checksum to verify 
		 * that the TOC has not been modified <rdar://problem/6134714>
		 */
		if( xar_signature_first(ret) != NULL && XAR(ret)->header.cksum_alg == XAR_CKSUM_NONE ) {
			fprintf(stderr, "Checksum/signature mismatch!\n");
			xar_close(ret);
			return NULL;
		}
			
		if( !XAR(ret)->docksum )
			return ret;

		EVP_DigestFinal_ex(XAR(ret)->toc_ctx, toccksum, &tlen);

		offset = 0;
		length = tlen;
		if( xar_prop_get( XAR_FILE(ret) , "checksum/offset", &value) == 0 ) {
			errno = 0;
			offset = (uint64_t)strtoull(value, (char **)NULL, 10);
			if( errno != 0 ) {
				xar_close(ret);
				return NULL;
			}
		} else if( xar_signature_first(ret) != NULL ) {
			/* All archives that have a signature also specify the location
			 * of the checksum.  If the location isn't specified, error out.
			 */
			xar_close(ret);
			return NULL;
		}

		XAR(ret)->heap_offset = (off_t)(xar_get_heap_offset(ret) + offset);
		if( lseek(XAR(ret)->fd, XAR(ret)->heap_offset, SEEK_SET) == -1 ) {
			xar_close(ret);
			return NULL;
		}
		if( xar_prop_get( XAR_FILE(ret) , "checksum/size", &value) == 0 ) {
			errno = 0;
			length = (uint64_t)strtoull(value, (char **)NULL, 10);
			if( errno != 0 ) {
				xar_close(ret);
				return NULL;
			}
		} else if( xar_signature_first(ret) != NULL ) {
			xar_close(ret);
			return NULL;
		}
		if( length != tlen ) {
			xar_close(ret);
			return NULL;
		}

		xar_read_fd(XAR(ret)->fd, cval, tlen);
		XAR(ret)->heap_offset += tlen;
		if( memcmp(cval, toccksum, tlen) != 0 ) {
			fprintf(stderr, "Checksums do not match!\n");
			xar_close(ret);
			return NULL;
		}
	}

	return ret;
}

/* xar_check_force_rfc6713
 * x: the xar_t to check
 * Summary: forces the rfcformat option on if the checksum type will be
 *          XAR_CKSUM_OTHER.  Should be called just before first file is added.
 */
static void xar_check_force_rfc6713(xar_t x)
{
	char *tmpser;

	if (XAR(x)->files != NULL)
		return;
	tmpser = (char *)xar_opt_get(x, XAR_OPT_TOCCKSUM);
	if (!tmpser)
		return;
	if (strcmp(tmpser, XAR_OPT_VAL_NONE) == 0 ||
		strcmp(tmpser, XAR_OPT_VAL_SHA1) == 0 ||
		strcmp(tmpser, XAR_OPT_VAL_MD5) == 0)
		return;
	XAR(x)->rfcformat = 1;
}

/* xar_close
 * x: the xar_t to close
 * Summary: closes all open file descriptors, frees all
 * file structures and options, deallocates the xar_t its self.
 * Returns 0 for success, -1 for failure.
 */
int xar_close(xar_t x) {
	xar_attr_t a;
	xar_file_t f;
	int ret, retval = 0;

	if (XAR(x)->heap_fd == -2)
		goto CLOSE_BAIL;

	/* If we're creating an archive */
	if( XAR(x)->heap_fd != -1 ) {
		char *tmpser;
		void *rbuf, *wbuf = NULL;
		int fd, r, off, wbytes, rbytes;
		long rsize, wsize;
		z_stream zs;
		uint64_t ungztoc, gztoc;
		unsigned char chkstr[HASH_MAX_MD_SIZE];
		int tocfd;
		char timestr[128];
		struct tm tmptm;
		time_t t;
		uint32_t cksum_alg = XAR_CKSUM_NONE;
		const char *opt;
		size_t cnt;
		ssize_t wcnt;

		tmpser = (char *)xar_opt_get(x, XAR_OPT_TOCCKSUM);
		/* If no checksum type is specified, default to sha1 */
		if( !tmpser ) tmpser = XAR_OPT_VAL_SHA1;

		if( (strcmp(tmpser, XAR_OPT_VAL_NONE) != 0) ) {
			const EVP_MD *md;
			const char *md_name;
			char mdlenstr[32];

			md = EVP_get_digestbyname(tmpser);
			if( md == NULL ) return -1;
			md_name = OBJ_nid2ln(EVP_MD_nid(md));
			if( strcmp(md_name, XAR_OPT_VAL_SHA1) == 0 )
				cksum_alg = XAR_CKSUM_SHA1;
			else if( strcmp(md_name, XAR_OPT_VAL_MD5) == 0 )
				cksum_alg = XAR_CKSUM_MD5;
			else if( strlen(md_name) + 1 > sizeof(XAR(x)->header.toc_cksum_name) ) {
				retval = -1;
				goto CLOSE_BAIL;
			}
			else {
				cksum_alg = XAR_CKSUM_OTHER;
				strncpy(XAR(x)->header.toc_cksum_name, md_name, sizeof(XAR(x)->header.toc_cksum_name));
			}
			xar_prop_set(XAR_FILE(x), "checksum", NULL);
			EVP_DigestInit_ex(XAR(x)->toc_ctx, md, NULL);
			xar_attr_set(XAR_FILE(x), "checksum", "style", md_name);
			sprintf(mdlenstr, "%d", EVP_MD_size(md));
			xar_prop_set(XAR_FILE(x), "checksum/size", mdlenstr);
			xar_prop_set(XAR_FILE(x), "checksum/offset", "0");
			XAR(x)->docksum = 1;
		} else {
			XAR(x)->docksum = 0;
		}
		XAR(x)->header.cksum_alg = htonl(cksum_alg);

		t = time(NULL);
		gmtime_r(&t, &tmptm);
		memset(timestr, 0, sizeof(timestr));
		strftime(timestr, sizeof(timestr), "%Y-%m-%dT%H:%M:%SZ", &tmptm);
		xar_prop_set(XAR_FILE(x), "creation-time", timestr);

		/* serialize the toc to a tmp file */
		if (asprintf(&tmpser, "%s/xar.toc.XXXXXX", XAR(x)->dirname) == -1) {
			retval = -1;
			goto CLOSE_BAIL;
		}
		fd = mkstemp(tmpser);
		xar_serialize(x, tmpser);
		unlink(tmpser);
		free(tmpser);
		if (asprintf(&tmpser, "%s/xar.toc.XXXXXX", XAR(x)->dirname) == -1) {
			retval = -1;
			goto CLOSE_BAIL;
		}
		tocfd = mkstemp(tmpser);
		unlink(tmpser);
		free(tmpser);
		
	
		/* read the toc from the tmp file, compress it, and write it
	 	* out to the archive.
	 	*/
		rsize = wsize = XAR_DEFAULT_BUFFER_SIZE;
		opt = xar_opt_get(x, XAR_OPT_RSIZE);
		if ( opt ) {
		  rsize = strtol(opt, NULL, 0);
		  if ( ((rsize == LONG_MAX) || (rsize == LONG_MIN)) && (errno == ERANGE) ) {
		    rsize = wsize;
		  }
		  if (rsize < XAR_MINIMUM_BUFFER_SIZE)
		    rsize = XAR_MINIMUM_BUFFER_SIZE;
		}

		rbuf = malloc(rsize);
		if( !rbuf ) {
			retval = -1;
			goto CLOSE_BAIL;
		}
		zs.zalloc = Z_NULL;
		zs.zfree = Z_NULL;
		zs.opaque = Z_NULL;
		deflateInit(&zs, Z_BEST_COMPRESSION);
	
		ungztoc = gztoc = 0;
	
		while(1) {
			r = (int)read(fd, rbuf, rsize);
			if( (r < 0) && (errno == EINTR) )
				continue;
			if( r == 0 )
				break;
	
			ungztoc += r;
	
			zs.avail_in = r;
			zs.next_in = (void *)rbuf;
			zs.next_out = NULL;
			zs.avail_out = 0;

			wsize = rsize/2;
	
			off = 0;
			while( zs.avail_in != 0 ) {
				wsize *= 2;
				wbuf = realloc(wbuf, wsize);

				zs.next_out = ((unsigned char *)wbuf) + off;
				zs.avail_out = (unsigned)wsize - off;

				ret = deflate(&zs, Z_SYNC_FLUSH);
				off = (int)(wsize - zs.avail_out);
			}
	
			wbytes = off;
			off = 0;
			do {
				r = (int)write(tocfd, ((char *)wbuf)+off, wbytes-off);
				if( (r < 0) && (errno == EINTR) )
					continue;
				if( r < 0 ) {
					xar_err_new(x);
					xar_err_set_string(x, "Error closing xar archive");
					retval = -1;
					goto CLOSEEND;
				}
				if( XAR(x)->docksum )
					EVP_DigestUpdate(XAR(x)->toc_ctx, ((char*)wbuf)+off, r);
				off += r;
				gztoc += r;
			} while( off < wbytes );

		}

		zs.next_in = NULL;
		zs.avail_in = 0;
		zs.next_out = wbuf;
		zs.avail_out = (unsigned)wsize;

		deflate(&zs, Z_FINISH);
		r = (int)write(tocfd, wbuf, wsize - zs.avail_out);
		gztoc += r;
		if( XAR(x)->docksum )
			EVP_DigestUpdate(XAR(x)->toc_ctx, wbuf, r);
		
		deflateEnd(&zs);

		/* populate the header and write it out */
		XAR(x)->header.magic = htonl(XAR_HEADER_MAGIC);
		if (cksum_alg == XAR_CKSUM_OTHER)
			XAR(x)->header.size = ntohs(sizeof(xar_header_ex_t));
		else
			XAR(x)->header.size = ntohs(sizeof(xar_header_t));
		XAR(x)->header.version = ntohs(1);
		XAR(x)->header.toc_length_uncompressed = xar_ntoh64(ungztoc);
		XAR(x)->header.toc_length_compressed = xar_ntoh64(gztoc);

		cnt = cksum_alg == XAR_CKSUM_OTHER ? sizeof(xar_header_ex_t) : sizeof(xar_header_t);
		wcnt = write(XAR(x)->fd, &XAR(x)->header, cnt);
		if (wcnt < 0 || wcnt != (ssize_t)cnt) {
			xar_err_new(x);
			xar_err_set_string(x, "Error writing xar archive header");
			retval = -1;
			goto CLOSEEND;
		}

		/* Copy the temp compressed toc file into the file */
		lseek(tocfd, (off_t)0, SEEK_SET);
		while(1) {
			r = (int)read(tocfd, rbuf, rsize);
			if( (r < 0) && (errno == EINTR) )
				continue;
			if( r == 0 )
				break;

			wbytes = r;
			off = 0;
			do {
				r = (int)write(XAR(x)->fd, ((char *)rbuf)+off, wbytes-off);
				if( (r < 0) && (errno == EINTR) )
					continue;
				if( r < 0 ) {
					xar_err_new(x);
					xar_err_set_string(x, "Error closing xar archive");
					retval = -1;
					goto CLOSEEND;
				}
					
				off += r;
			} while( off < wbytes );
		}

		/* start counting any digest/signature bytes written */
		rbytes = 0;

		if( XAR(x)->docksum ) {
			unsigned int l = r;
			
			memset(chkstr, 0, sizeof(chkstr));
			EVP_DigestFinal_ex(XAR(x)->toc_ctx, chkstr, &l);
			r = l;
			wcnt = write(XAR(x)->fd, chkstr, r);
			if (wcnt < 0 || wcnt != (ssize_t)r) {
				xar_err_new(x);
				xar_err_set_string(x, "Error writing xar archive checksum");
				retval = -1;
				goto CLOSEEND;
			}
			rbytes += r;
		}

		/* If there are any signatures, get the signed data a sign it */
		if( XAR(x)->docksum && XAR(x)->signatures ) {
			xar_signature_t sig;
			uint32_t data_len = r;
			uint32_t signed_len = 0;
			uint8_t *signed_data = NULL;
			
			/* Loop through the signatures */
			for(sig = XAR(x)->signatures; sig; sig = XAR_SIGNATURE(sig)->next ){				
				signed_len = XAR_SIGNATURE(sig)->len;
				
				/* If callback returns something other then 0, bail */
				if( 0 != sig->signer_callback( sig, sig->callback_context, chkstr, data_len, &signed_data, &signed_len ) ){
					fprintf(stderr, "Error signing data.\n");
					retval = -1;
					goto CLOSE_BAIL;					
				}
				
				if( signed_len != (uint32_t)XAR_SIGNATURE(sig)->len ){
					fprintf(stderr, "Signed data not the proper length.  %i should be %i.\n",signed_len,XAR_SIGNATURE(sig)->len);
					retval = -1;
					goto CLOSE_BAIL;										
				}
				
				/* Write the signed data to the heap */
				wcnt = write(XAR(x)->fd, signed_data, (size_t)XAR_SIGNATURE(sig)->len);
				if (wcnt < 0 || wcnt != (ssize_t)XAR_SIGNATURE(sig)->len) {
					xar_err_new(x);
					xar_err_set_string(x, "Error writing xar archive signature data");
					retval = -1;
					goto CLOSEEND;
				}
				rbytes += XAR_SIGNATURE(sig)->len;

				free(signed_data);
			}
			
			xar_signature_remove( XAR(x)->signatures );
			XAR(x)->signatures = NULL;
		}

		/* copy the heap from the temporary heap into the archive */
		if( lseek(XAR(x)->heap_fd, (off_t)0, SEEK_SET) < 0 ) {
			fprintf(stderr, "Error lseeking to offset 0: %s\n", strerror(errno));
			exit(1);
		}
		/* XAR(x)->heap_len includes any digest/signatures but the heap file does not and at this
                 * point rbytes reflects the total byte count of any digest/signatures that are present */
		while(1) {
			if( (XAR(x)->heap_len - rbytes) < rsize )
				rsize = (long)(XAR(x)->heap_len - rbytes);

			r = (int)read(XAR(x)->heap_fd, rbuf, rsize);
			if( (r < 0 ) && (errno == EINTR) )
				continue;
			if( r == 0 )
				break;
	
			rbytes += r;
			wbytes = r;
			off = 0;
			do {
				r = (int)write(XAR(x)->fd, ((char *)rbuf)+off, wbytes);
				if( (r < 0 ) && (errno == EINTR) )
					continue;
				if( r < 0 ) {
					retval = -1;
					goto CLOSEEND;
				}
				off += r;
			} while( off < wbytes );

			if( rbytes >= XAR(x)->heap_len )
				break;
		}
CLOSEEND:
		free(rbuf);
		free(wbuf);
		deflateEnd(&XAR(x)->zs);
	} else {
		inflateEnd(&XAR(x)->zs);
	}
		
CLOSE_BAIL:
	/* continue deallocating the archive and return */
	while(XAR(x)->subdocs) {
		xar_subdoc_remove(XAR(x)->subdocs);
	}

	while(XAR(x)->attrs) {
		a = XAR(x)->attrs;
		XAR(x)->attrs = XAR_ATTR(a)->next;
		xar_attr_free(a);
	}

	while(XAR(x)->props) {
		xar_prop_t p;
		p = XAR(x)->props;
		XAR(x)->props = XAR_PROP(p)->next;
		xar_prop_free(p);
	}

	while(XAR(x)->files) {
		f = XAR(x)->files;
		XAR(x)->files = XAR_FILE(f)->next;
		xar_file_free(f);
	}

	xmlHashFree(XAR(x)->ino_hash, NULL);
	xmlHashFree(XAR(x)->link_hash, NULL);
	xmlHashFree(XAR(x)->csum_hash, NULL);
	if (XAR(x)->fd >= 0)
		close(XAR(x)->fd);
	if( XAR(x)->heap_fd >= 0 )
		close(XAR(x)->heap_fd);
	free((char *)XAR(x)->filename);
	free((char *)XAR(x)->dirname);
	free(XAR(x)->readbuf);
	EVP_MD_CTX_destroy(XAR(x)->toc_ctx);
	free((void *)x);

	return retval;
}

/* xar_opt_get
 * x: archive to get the option from
 * option: name of the option
 * Returns: a pointer to the value of the option
 * In the case of more than one option with the same name, this will
 * return the first match.
 */
const char *xar_opt_get(xar_t x, const char *option) {
	xar_attr_t i;
	if (!x || !option)
		return NULL;
	if (strcmp(option, XAR_OPT_XARLIBVERSION) == 0)
		return XAR_LIB_VERSION();
	for(i = XAR(x)->attrs; i && XAR_ATTR(i)->next; i = XAR_ATTR(i)->next) {
		if(strcmp(XAR_ATTR(i)->key, option)==0)
			return XAR_ATTR(i)->value;
	}
	if( i && (strcmp(XAR_ATTR(i)->key, option)==0) )
		return XAR_ATTR(i)->value;
	return NULL;
}

/* xar_opt_set
 * x: the archive to set the option of
 * option: the name of the option to set the value of
 * value: the value to set the option to
 * Returns: 0 for sucess, -1 for failure
 */
int32_t xar_opt_set(xar_t x, const char *option, const char *value) {
	xar_attr_t a;

	if (!x || !option)
		return -1;
	if (!value)
		value = "";
	if( (strcmp(option, XAR_OPT_TOCCKSUM) == 0) ) {
		xar_signature_t sig;

		/* Cannot change XAR_OPT_TOCCKSUM after adding any files */
		if (XAR(x)->files != NULL) {
			xar_err_new(x);
			xar_err_set_string(x, "XAR_OPT_TOCCKSUM must be set before files are added");
			xar_err_callback(x, XAR_SEVERITY_WARNING, XAR_ERR_ARCHIVE_CREATION);
			return -1;
		}

		if( strcmp(value, XAR_OPT_VAL_NONE) == 0 ) {
			XAR(x)->heap_offset = 0;
			XAR(x)->heap_len = 0;
		}
		else {
			const EVP_MD *md;
			md = EVP_get_digestbyname(value);
			if( md == NULL || EVP_MD_size(md) > HASH_MAX_MD_SIZE ) return -1;
			XAR(x)->heap_offset = EVP_MD_size(md);
			XAR(x)->heap_len = EVP_MD_size(md);
		}

		/* If there are any signatures already added they must have their offsets reset now */
		sig = XAR(x)->signatures;
		while (sig) {
			XAR_SIGNATURE(sig)->offset = XAR(x)->heap_offset;
			XAR(x)->heap_offset += XAR_SIGNATURE(sig)->len;
			XAR(x)->heap_len += XAR_SIGNATURE(sig)->len;
			sig = XAR_SIGNATURE(sig)->next;
		}
	}
	if( (strcmp(option, XAR_OPT_FILECKSUM) == 0) ) {
		if( strcmp(value, XAR_OPT_VAL_NONE) != 0 ) {
			const EVP_MD *md;
			md = EVP_get_digestbyname(value);
			if( md == NULL || EVP_MD_size(md) > HASH_MAX_MD_SIZE ) return -1;
		}
	}
	if ((strcmp(option, XAR_OPT_STRIPCOMPONENTS) == 0)) {
		long comps;
		char *endptr;
		comps = strtol(value, &endptr, 0);
		if (!*value || *endptr || comps < 0)
			return -1;
		XAR(x)->stripcomps = (int)comps;
	}
	if ((strcmp(option, XAR_OPT_EXTRACTSTDOUT) == 0)) {
		XAR(x)->tostdout = strcmp(value, XAR_OPT_VAL_TRUE) == 0;
	}
	if ((XAR(x)->files == NULL && strcmp(option, XAR_OPT_RFC6713FORMAT) == 0)) {
		XAR(x)->rfcformat = strcmp(value, XAR_OPT_VAL_TRUE) == 0;
	}
	a = xar_attr_new();
	XAR_ATTR(a)->key = strdup(option);
	XAR_ATTR(a)->value = strdup(value);
	XAR_ATTR(a)->next = XAR(x)->attrs;
	XAR(x)->attrs = a;
	return 0;
}

/* xar_opt_unset
 * x: the archive to set the option of
 * option: the name of the option to delete
 * This will delete ALL instances of the option name
 */
int32_t xar_opt_unset(xar_t x, const char *option) {
	xar_attr_t i, p = NULL;
	for(i = XAR(x)->attrs; i ; p = i, i = XAR_ATTR(i)->next) {
		if(strcmp(XAR_ATTR(i)->key, option)==0) {
			if( p == NULL )
				XAR(x)->attrs = XAR_ATTR(i)->next;
			else
				XAR_ATTR(p)->next = XAR_ATTR(i)->next;
			xar_attr_free(i);
			i = p;
		}
	}
	return 0;
}

/* xar_add_node
 * x: archive the file should belong to
 * f: parent node, possibly NULL
 * name: name of the node to add
 * realpath: real path to item, this is used if the item being archived is to be located at a different location in the tree
 * then it is on the real filesystem.
 * Returns: newly allocated and populated node
 * Summary: helper function which adds a child of f and populates
 * its properties.  If f is NULL, the node will be added as a top
 * level node of the archive, x.
 */
static xar_file_t xar_add_node(xar_t x, xar_file_t f, const char *name, const char *prefix, const char *realpath, int srcpath) {
	xar_file_t ret;
	const char *path; 
	char *tmp;
	char idstr[32];

	(void)srcpath;
	if( !f ) {
		int err;
		if( realpath )
			err = asprintf(&tmp, "%s", realpath);
		else
			err = asprintf(&tmp, "%s%s%s", XAR(x)->path_prefix, prefix, name);

		if( err == -1 || lstat(tmp, &XAR(x)->sbcache) != 0 ) {
			free(tmp);
			return NULL;
		}

		ret = xar_file_new(NULL);
		if( !ret )
			return NULL;
		memset(idstr, 0, sizeof(idstr));
		snprintf(idstr, sizeof(idstr)-1, "%"PRIu64, ++XAR(x)->last_fileid);
		xar_attr_set(ret, NULL, "id", idstr);
		XAR_FILE(ret)->parent = NULL;
		XAR_FILE(ret)->fspath = tmp;
		if( XAR(x)->files == NULL ) {
			xar_check_force_rfc6713(x);
			XAR(x)->files = ret;
		}
		else {
			XAR_FILE(ret)->next = XAR(x)->files;
			XAR(x)->files = ret;
		}
	} else {
		int err;
		path = XAR_FILE(f)->fspath;
		if( strcmp(prefix, "../") == 0 ) {
			size_t len1, len2;
			len1 = strlen(path);
			len2 = strlen(name);
			if( (len1>=len2) && (strcmp(path+(len1-len2), name) == 0) ) {
				return f;
			}
			
		}

		if( realpath ){
			err = asprintf(&tmp, "%s", realpath);
		}else
			err = asprintf(&tmp, "%s/%s%s", path, prefix, name);
		
		if( err == -1 || lstat(tmp, &XAR(x)->sbcache) != 0 ) {
			free(tmp);
			return NULL;
		}

		ret = xar_file_new(f);
		if( !ret )
			return NULL;
		memset(idstr, 0, sizeof(idstr));
		snprintf(idstr, sizeof(idstr)-1, "%"PRIu64, ++XAR(x)->last_fileid);
		xar_attr_set(ret, NULL, "id", idstr);
		XAR_FILE(ret)->fspath = tmp;
	}

	xar_prop_set(ret, "name", name);

	if( xar_arcmod_archive(x, ret, XAR_FILE(ret)->fspath, NULL, 0) < 0 ) {
		xar_file_t i = NULL;
		if( f ) {
			if( ret == XAR_FILE(f)->children )
				XAR_FILE(f)->children = XAR_FILE(ret)->next;
			else
				for( i = XAR_FILE(f)->children; i && (XAR_FILE(i)->next != ret); i = XAR_FILE(i)->next );
		} else {
			if( ret == XAR(x)->files )
				XAR(x)->files = XAR_FILE(ret)->next;
			else
				for( i = XAR(x)->files; i && (XAR_FILE(i)->next != ret); i = XAR_FILE(i)->next );
		}
		if( i )
			XAR_FILE(i)->next = XAR_FILE(ret)->next;
		xar_file_free(ret);
		return NULL;
	}

	return ret;
}

/* xar_add_pseudodir
 * Summary: Adds a placeholder directory when archiving a file prior
 * to archiving its path.
 */
static xar_file_t xar_add_pseudodir(xar_t x, xar_file_t f, const char *name, const char *prefix, const char *realpath)
{
	xar_file_t ret;
	const char *path; 
	char *tmp;
	char idstr[32];

	if( !f ) {
		int err;
		if( realpath )
			err = asprintf(&tmp, "%s", realpath);
		else
			err = asprintf(&tmp, "%s%s%s", XAR(x)->path_prefix, prefix, name);

		if( err == -1 || lstat(tmp, &XAR(x)->sbcache) != 0 ) {
			free(tmp);
			return NULL;
		}

		ret = xar_file_new(NULL);
		if( !ret )
			return NULL;
		memset(idstr, 0, sizeof(idstr));
		snprintf(idstr, sizeof(idstr)-1, "%"PRIu64, ++XAR(x)->last_fileid);
		xar_attr_set(ret, NULL, "id", idstr);
		XAR_FILE(ret)->parent = NULL;
		XAR_FILE(ret)->fspath = tmp;
		if( XAR(x)->files == NULL ) {
			xar_check_force_rfc6713(x);
			XAR(x)->files = ret;
		}
		else {
			XAR_FILE(ret)->next = XAR(x)->files;
			XAR(x)->files = ret;
		}
	} else {
		int err;
		path = XAR_FILE(f)->fspath;
		if( strcmp(prefix, "../") == 0 ) {
			size_t len1, len2;
			len1 = strlen(path);
			len2 = strlen(name);
			if( (len1>=len2) && (strcmp(path+(len1-len2), name) == 0) ) {
				return f;
			}
			
		}

		if( realpath ){
			err = asprintf(&tmp, "%s", realpath);
		}else
			err = asprintf(&tmp, "%s/%s%s", path, prefix, name);
		
		if( err == -1 || lstat(tmp, &XAR(x)->sbcache) != 0 ) {
			free(tmp);
			return NULL;
		}

		ret = xar_file_new(f);
		if( !ret )
			return NULL;
		memset(idstr, 0, sizeof(idstr));
		snprintf(idstr, sizeof(idstr)-1, "%"PRIu64, ++XAR(x)->last_fileid);
		xar_attr_set(ret, NULL, "id", idstr);
		XAR_FILE(ret)->fspath = tmp;
	}
	xar_prop_set(ret, "name", name);
	xar_prop_set(ret, "type", "directory");

	return ret;
}

/* xar_add_r
 * Summary: a recursive helper function for adding a node to the
 * tree.  This will search all children of node f, looking for
 * the path component.  If found, will recurse into it.  If not,
 * will add the path component to the tree, and recurse into it.
 * If f is NULL, will start with x->files.
 */
static xar_file_t xar_add_r(xar_t x, xar_file_t f, const char *path, const char *prefix) {
	xar_file_t i = NULL, ret, ret2, start = NULL;
	char *tmp1, *tmp2, *tmp3;

	if( path && (path[0] == '\0') ) {
		return f;
	}

	tmp1 = tmp2 = strdup(path);
	tmp3 = strsep(&tmp2, "/");

	if( tmp3 && tmp2 && (tmp3[0] == '\0') ) {
		ret2 = xar_add_r(x, f, tmp2, "");
		free(tmp1);
		return ret2;
	}

	if( strcmp(tmp3, "..") == 0 ) {
		char *prefixstr;
		int err;
		if( !XAR(x)->skipwarn ) {
			xar_err_new(x);
			xar_err_set_string(x, "Skipping .. in path");
			xar_err_callback(x, XAR_SEVERITY_WARNING, XAR_ERR_ARCHIVE_CREATION);
			XAR(x)->skipwarn = 1;
		}
		err = asprintf(&prefixstr, "%s../", prefix);
		if (err == -1)
			ret2 = NULL;
		else
			ret2 = xar_add_r(x, f, tmp2, prefixstr);
		free(prefixstr);
		free(tmp1);
		return ret2;
	}

	if( strcmp(tmp3, ".") == 0 ) {
		if( tmp2 )
			ret2 = xar_add_r(x, f, tmp2, prefix);
		else
			ret2 = NULL;
		free(tmp1);
		return ret2;
	}

	if( !f ) {
		start = XAR(x)->files;
	} else {
		start = XAR_FILE(f)->children;
	}

	/* Search all the siblings */
	for( i = start; i; i = XAR_FILE(i)->next ) {
		const char *n;
		xar_prop_get(i, "name", &n);
		if( strcmp(n, tmp3) == 0 ) {
			if( !tmp2 ) {
				/* Node already exists, and it is i */
				free(tmp1);
				return i;
			}
			ret2 = xar_add_r(x, i, tmp2, "");
			free(tmp1);
			return ret2;
		}
	}

	/* tmp3 was not found in children of start, so we add it */
	if( tmp2 ) {
		/*ret = xar_add_node(x, f, tmp3, prefix, NULL,  1);*/
		ret = xar_add_pseudodir(x, f, tmp3, prefix, NULL);
	} else {
		ret = xar_add_node(x, f, tmp3, prefix, NULL,  0);
	}

	if( !ret ) {
		free(tmp1);
		return NULL;
	}

	if( !tmp2 ) {
		/* We've added the final piece, done, don't recurse */
		free(tmp1);
		return ret;
	}

	/* still more to add, recurse */
	ret2 = xar_add_r(x, ret, tmp2, "");
	free(tmp1);
	return ret2;
}

/* xar_add
 * x: archive to add the file to
 * path: path to file
 * Returns: allocated an populated xar_file_t representing the 
 * specified file.
 * Summary: if a full path "foo/bar/blah" is specified, then any
 * directories not already existing in the archive will be added
 * automagically.  The returned xar_file_t represents the file
 * specified, not the parent of the directory tree.
 * For instance, if "foo/bar/blah" is specified, the xar_file_t
 * representing "blah" will be returned.
 */
xar_file_t xar_add(xar_t x, const char *path) {
#ifdef __APPLE__
	xar_file_t ret;
	if( (ret = xar_underbar_check(x, NULL, path)) )
		return ret;
#endif

	if( path[0] == '/' ) {
		XAR(x)->path_prefix = "/";
		path++;
	} else
		XAR(x)->path_prefix = "";
	return xar_add_r(x, NULL, path, "");
}

/* xar_add_frombuffer
* x: archive to add the file to
* parent: parent node, possibly NULL
* name: name of file
* buffer: buffer for file contents
* length: length of buffer
* Returns: allocated an populated xar_file_t representing the 
* specified file.
* Summary: Use this to add chunks of named data to a xar without
* using the filesystem.
*/

xar_file_t xar_add_frombuffer(xar_t x, xar_file_t parent, const char *name, char *buffer, size_t length) {
	xar_file_t ret;
	char idstr[32];
	
	if( !parent ) {
		ret = xar_file_new(NULL);
		if( !ret )
			return NULL;
		memset(idstr, 0, sizeof(idstr));
		snprintf(idstr, sizeof(idstr)-1, "%"PRIu64, ++XAR(x)->last_fileid);
		xar_attr_set(ret, NULL, "id", idstr);
		XAR_FILE(ret)->parent = NULL;
		if( XAR(x)->files == NULL ) {
			xar_check_force_rfc6713(x);
			XAR(x)->files = ret;
		}
		else {
			XAR_FILE(ret)->next = XAR(x)->files;
			XAR(x)->files = ret;
		}
	} else {
		ret = xar_file_new(parent);
		if( !ret )
			return NULL;
		memset(idstr, 0, sizeof(idstr));
		snprintf(idstr, sizeof(idstr)-1, "%"PRIu64, ++XAR(x)->last_fileid);
		xar_attr_set(ret, NULL, "id", idstr);
		XAR_FILE(ret)->fspath = NULL;
	}
	
	xar_prop_set(ret, "name", name);
		
	/*int32_t xar_arcmod_archive(xar_t x, xar_file_t f, const char *file, const char *buffer, size_t len) */
	if( xar_arcmod_archive(x, ret, NULL , buffer , length) < 0 ) {
		xar_file_t i;
		if( parent ) {
			for( i = XAR_FILE(parent)->children; i && (XAR_FILE(i)->next != ret); i = XAR_FILE(i)->next );
		} else {
			for( i = XAR(x)->files; i && (XAR_FILE(i)->next != ret); i = XAR_FILE(i)->next );
		}
		if( i )
			XAR_FILE(i)->next = XAR_FILE(ret)->next;
		xar_file_free(ret);
		return NULL;
	}
	
	return ret;
}

xar_file_t xar_add_folder(xar_t x, xar_file_t f, const char *name, struct stat *info)
{
	xar_file_t ret;
	char idstr[32];

	if( info )
		memcpy(&XAR(x)->sbcache,info,sizeof(struct stat));
	
	ret = xar_file_new(f);
	if( !ret )
		return NULL;
	
	memset(idstr, 0, sizeof(idstr));
	snprintf(idstr, sizeof(idstr)-1, "%"PRIu64, ++XAR(x)->last_fileid);
	xar_attr_set(ret, NULL, "id", idstr);
	XAR_FILE(ret)->fspath = NULL;
	
	if( !f ) {
		XAR_FILE(ret)->parent = NULL;
		
		if( XAR(x)->files == NULL ) {
			xar_check_force_rfc6713(x);
			XAR(x)->files = ret;
		}
		else {
			XAR_FILE(ret)->next = XAR(x)->files;
			XAR(x)->files = ret;
		}
	}
	
	xar_prop_set(ret, "name", name);

	if( xar_arcmod_archive(x, ret, XAR_FILE(ret)->fspath, NULL, 0) < 0 ) {
		xar_file_t i;
		if( f ) {
			for( i = XAR_FILE(f)->children; i && (XAR_FILE(i)->next != ret); i = XAR_FILE(i)->next );
		} else {
			for( i = XAR(x)->files; i && (XAR_FILE(i)->next != ret); i = XAR_FILE(i)->next );
		}
		if( i )
			XAR_FILE(i)->next = XAR_FILE(ret)->next;
		xar_file_free(ret);
		return NULL;
	}
	
	return ret;	
}

xar_file_t xar_add_frompath(xar_t x, xar_file_t parent, const char *name, const char *realpath)
{
	return xar_add_node(x, parent, name , "" , realpath,  1);
}

xar_file_t xar_add_from_archive(xar_t x, xar_file_t parent, const char *name, xar_t sourcearchive, xar_file_t sourcefile)
{
	xar_file_t ret;
	char idstr[32];
	xar_prop_t p;
		
	ret = xar_file_replicate(sourcefile, parent);
	
	if( !ret )
		return NULL;
	
	memset(idstr, 0, sizeof(idstr));
	snprintf(idstr, sizeof(idstr)-1, "%"PRIu64, ++XAR(x)->last_fileid);
	xar_attr_set(ret, NULL, "id", idstr);
	XAR_FILE(ret)->fspath = NULL;
	
	if( !parent ) {
		XAR_FILE(ret)->parent = NULL;
		
		if( XAR(x)->files == NULL ) {
			xar_check_force_rfc6713(x);
			XAR(x)->files = ret;
		}
		else {
			XAR_FILE(ret)->next = XAR(x)->files;
			XAR(x)->files = ret;
		}
	}
		
	xar_prop_set(ret, "name", name);
		
	/* iterate through all the properties, see if any of them have an offset */
	p = xar_prop_pfirst(ret);

	do{
		xar_prop_t tmpp;
		
		tmpp = xar_prop_pget(p, "offset");
		if(tmpp) {
			if( 0 != xar_attrcopy_from_heap_to_heap(sourcearchive, sourcefile, p, x, ret)){			
				xar_file_free(ret);
				ret = NULL;
				break;
			}
		}
		
	}while( (p = xar_prop_pnext(p)) );
	
	return ret;	
}

/* xar_extract_tofile
* x: archive to extract from
* f: file associated with x
* Returns 0 on success, -1 on failure
* Summary: This actually does the file extraction.
* No traversal is performed, it is assumed all directory paths
* leading up to f already exist.
*/
int32_t xar_extract_tofile(xar_t x, xar_file_t f, const char *path) {
	return xar_arcmod_extract(x, f, path,NULL, 0);
}


/* xar_extract_tobuffer
* x: archive to extract from
* buffer: buffer to extract to
* Returns 0 on success, -1 on failure.
* Summary: This is the entry point for extraction to a buffer.
* On success, a buffer is allocated with the contents of the file
* specified.  The caller is responsible for freeing the returend buffer.
* Example: xar_extract_tobuffer(x, "foo/bar/blah",&buffer)
*/
int32_t xar_extract_tobuffer(xar_t x, xar_file_t f, char **buffer) {
	size_t size;

	return xar_extract_tobuffersz(x, f, buffer, &size);
}

/* xar_extract_tobuffer
* x: archive to extract from
* buffer: buffer to extract to
* size: On return, this will contain the size of the memory pointed to by buffer
* Returns 0 on success, -1 on failure.
* Summary: This is the entry point for extraction to a buffer.
* On success, a buffer is allocated with the contents of the file
* specified.  The caller is responsible for freeing the returend buffer.
* Example: xar_extract_tobuffer(x, "foo/bar/blah",&buffer)
*/
int32_t xar_extract_tobuffersz(xar_t x, xar_file_t f, char **buffer, size_t *size) {
	const char *sizestring = NULL;
	int32_t ret;

	if (!buffer || !size)
		return -1;

	if(0 != xar_prop_get(f,"data/size",&sizestring)){
		if(0 != xar_prop_get(f, "type", &sizestring))
			return -1;
		if(strcmp(sizestring, "file") == 0) {
			*size = 0;
			return 0;
		}
		return -1;
	}

	*size = strtoull(sizestring, (char **)NULL, 10);
	*buffer = malloc(*size);
	
	if(!(*buffer)){
		return -1;
	}
	if (!*size)
		return 0;

	ret = xar_arcmod_extract(x,f,NULL,*buffer,*size);
	if( ret ) {
		*size = 0;
		free(*buffer);
		*buffer = NULL;
	}

	return ret;
}

int32_t xar_extract_tostream_init(xar_t x, xar_file_t f, xar_stream *stream) {
	xar_prop_t tmpp;

	if( !xar_check_prop(x, "data") )
		return XAR_STREAM_OK;

	tmpp = xar_prop_pfirst(f);
	if( tmpp )
		tmpp = xar_prop_find(tmpp, "data");
	if( !tmpp )
		return XAR_STREAM_OK;

	return xar_attrcopy_from_heap_to_stream_init(x, f, tmpp, stream);
}

int32_t xar_extract_tostream(xar_stream *stream) {
	return xar_attrcopy_from_heap_to_stream(stream);
}

int32_t xar_extract_tostream_end(xar_stream *stream) {
	return xar_attrcopy_from_heap_to_stream_end(stream);
}

const char *xar_strip_components(const char *path, int components)
{
	while (components-- > 0) {
		const char *nextcomp;
		nextcomp = strchr(path, '/');
		if (!nextcomp)
			return NULL;
		while (nextcomp[1] == '/')
			++nextcomp;
		if (!nextcomp[1])
			return NULL;
		path = nextcomp + 1;
	}
	return path;
}

/* xar_extract
 * x: archive to extract from
 * path: path to file to extract
 * Returns 0 on success, -1 on failure.
 * Summary: This is the entry point for extraction.  This will find
 * the file node described by path, extract any directories needed
 * to extract the node, and finally extract the file.
 * Example: xar_extract(x, "foo/bar/blah")
 * If foo does not exist, xar_extract will extract foo from the
 * archive, extract bar from the archive, and then extract blah.
 * Total extractions will be "foo", "foo/bar", and "foo/bar/blah".
 */
int32_t xar_extract(xar_t x, xar_file_t f) {
	struct stat sb;
	char *tmp1, *dname;
	xar_file_t tmpf;
	const char *fspath;

	fspath = XAR_FILE(f)->fspath;
	if (XAR(x)->stripcomps && (fspath=xar_strip_components(fspath, XAR(x)->stripcomps)) == NULL)
		return -1;
	if (XAR(x)->tostdout) {
		char *buffer = NULL;
		size_t bsize = 0;
		int32_t result;
		if ((result = xar_extract_tobuffersz(x, f, &buffer, &bsize)) != 0)
			return result;
		if (bsize && buffer && fwrite(buffer, bsize, 1, stdout) != 1)
			result = -1;
		free(buffer);
		return result;
	}
	else {
		if( (strstr(fspath, "/") != NULL) && (stat(fspath, &sb)) && (XAR_FILE(f)->parent_extracted == 0) ) {
			tmp1 = strdup(XAR_FILE(f)->fspath);
			dname = dirname(tmp1);
			tmpf = xar_file_find(XAR(x)->files, dname);
			if( !tmpf ) {
				xar_err_set_string(x, "Unable to find file");
				xar_err_callback(x, XAR_SEVERITY_NONFATAL, XAR_ERR_ARCHIVE_EXTRACTION);
				return -1;
			}
			free(tmp1);
			XAR_FILE(f)->parent_extracted++;
			xar_extract(x, tmpf);
		}

		return xar_extract_tofile(x, f, fspath);
	}
}

/* xar_verify
* x: archive to extract from
* f: file to verify
* Returns 0 on success, -1 on failure.
* Summary: This function allows for verification of
* an entry without extraction.  If there is no checksum
* the verification will pass.
*/
int32_t xar_verify(xar_t x, xar_file_t f) {
	return xar_arcmod_verify(x,f);
}

/* toc_read_callback
 * context: context passed through from the reader
 * buffer: buffer to read into
 * len: size of buffer
 * Returns: number of bytes read or -1 in case of error
 * Summary: internal callback for xmlReaderForIO.
 */
static int toc_read_callback(void *context, char *buffer, int len) {
	xar_t x = (xar_t)context;
	int ret, off = 0;

	if ( ((!XAR(x)->offset) || (XAR(x)->offset == XAR(x)->readbuf_len)) && (XAR(x)->toc_count != XAR(x)->header.toc_length_compressed) ) {
		XAR(x)->offset = 0;
		if( (XAR(x)->readbuf_len - off) + XAR(x)->toc_count > XAR(x)->header.toc_length_compressed )
			ret = (int)xar_read_fd(XAR(x)->fd, XAR(x)->readbuf, (size_t)(XAR(x)->header.toc_length_compressed - XAR(x)->toc_count));
		else
			ret = (int)read(XAR(x)->fd, XAR(x)->readbuf, XAR(x)->readbuf_len);
		if ( ret == -1 )
			return ret;

		if ( XAR(x)->docksum )
			EVP_DigestUpdate(XAR(x)->toc_ctx, XAR(x)->readbuf, ret);

		XAR(x)->toc_count += ret;
		off += ret;
	}

	if( off && (off < (int)XAR(x)->readbuf_len) )
		XAR(x)->readbuf_len = off;
	XAR(x)->zs.next_in = ((unsigned char *)XAR(x)->readbuf) + XAR(x)->offset;
	XAR(x)->zs.avail_in = (unsigned)(XAR(x)->readbuf_len - XAR(x)->offset);
	XAR(x)->zs.next_out = (void *)buffer;
	XAR(x)->zs.avail_out = len;

	ret = inflate(&XAR(x)->zs, Z_SYNC_FLUSH);
	if( ret < 0 )
		return -1;

	XAR(x)->offset = XAR(x)->readbuf_len - XAR(x)->zs.avail_in;

	return len - XAR(x)->zs.avail_out;
}

/* close_callback
 * context: this will be a xar_t
 * Returns: 0 or -1 in case of error
 * Summary: this is the callback for xmlTextReaderForIO to close the IO
 */
static int close_callback(void *context) {
	(void)context;
	return 0;
}

/* xar_serialize
 * x: xar to serialize
 * file: file to serialize to
 * Summary: serializes the archive out to xml.
 */
void xar_serialize(xar_t x, const char *file) {
	xmlTextWriterPtr writer;
	xar_subdoc_t i;

	writer = xmlNewTextWriterFilename(file, 0);
	xmlTextWriterStartDocument(writer, "1.0", "UTF-8", NULL);
	xmlTextWriterSetIndent(writer, 4);
	xmlTextWriterStartElement(writer, BAD_CAST("xar"));

	for( i = XAR(x)->subdocs; i; i = xar_subdoc_next(i) )
		xar_subdoc_serialize(i, writer, 1);

	xmlTextWriterStartElement(writer, BAD_CAST("toc"));
			
	if( XAR(x)->props )
		xar_prop_serialize(XAR(x)->props, writer);

	if( XAR(x)->signatures )
		xar_signature_serialize(XAR(x)->signatures,writer);

	if( XAR(x)->files )
		xar_file_serialize(XAR(x)->files, writer);

	xmlTextWriterEndDocument(writer);
	xmlFreeTextWriter(writer);
	return;
}

/* xar_unserialize
 * x: xar archive to unserialize to.  Must have been allocated with xar_open
 * file: the xml filename to unserialize from
 * Summary: Takes the TOC representation from file and creates the
 * corresponding in-memory representation.
 */
static int32_t xar_unserialize(xar_t x) {
	xmlTextReaderPtr reader;
	xar_file_t f = NULL;
	const xmlChar *name, *prefix, *uri;
	int type, noattr, ret;

	reader = xmlReaderForIO(toc_read_callback, close_callback, XAR(x), NULL, NULL, 0);
	if( !reader ) return -1;

	while( (ret = xmlTextReaderRead(reader)) == 1 ) {
		type = xmlTextReaderNodeType(reader);
		noattr = xmlTextReaderAttributeCount(reader);
		name = xmlTextReaderConstLocalName(reader);
		if( type != XML_READER_TYPE_ELEMENT )
			continue;
		if(strcmp((const char*)name, "xar") != 0)
			continue;
		while( (ret = xmlTextReaderRead(reader)) == 1 ) {
			type = xmlTextReaderNodeType(reader);
			noattr = xmlTextReaderAttributeCount(reader);
			name = xmlTextReaderConstLocalName(reader);
			if( type == XML_READER_TYPE_ELEMENT ) {
				if(strcmp((const char*)name, "toc") == 0) {
					while( (ret = xmlTextReaderRead(reader)) == 1 ) {
						type = xmlTextReaderNodeType(reader);
						noattr = xmlTextReaderAttributeCount(reader);
						name = xmlTextReaderConstLocalName(reader);
						if( type == XML_READER_TYPE_ELEMENT ) {
							if(strcmp((const char*)name, "file") == 0) {
								f = xar_file_unserialize(x, NULL, reader);
								XAR_FILE(f)->next = XAR(x)->files;
								XAR(x)->files = f;
							} else if( strcmp((const char*)name, "signature") == 0 ){
								xar_signature_t sig = NULL;			
								sig = xar_signature_unserialize(x, reader );
								
								if( !sig ) {
									xmlFreeTextReader(reader);
									xmlDictCleanup();
									xmlCleanupCharEncodingHandlers();
									return -1;
								}
								
								if( XAR(x)->signatures )
									XAR_SIGNATURE(XAR(x)->signatures)->next = XAR_SIGNATURE(sig);
								else
									XAR(x)->signatures = sig;
									
							} else {
								xar_prop_unserialize(XAR_FILE(x), NULL, reader);
							}
						}
					}
					if( ret == -1 ) {
						xmlFreeTextReader(reader);
						xmlDictCleanup();
						xmlCleanupCharEncodingHandlers();
						return -1;
					}
				} else {
					xar_subdoc_t s;
					int i;
					struct __xar_attr_t *attrs = NULL;

					prefix = xmlTextReaderPrefix(reader);
					uri = xmlTextReaderNamespaceUri(reader);

					i = xmlTextReaderAttributeCount(reader);
					if( i > 0 ) {
						for(i = xmlTextReaderMoveToFirstAttribute(reader); i == 1; i = xmlTextReaderMoveToNextAttribute(reader)) {
							xar_attr_t a;
							const char *aname = (const char *)xmlTextReaderConstLocalName(reader);
							const char *avalue = (const char *)xmlTextReaderConstValue(reader);
							
							if( aname && (strcmp("subdoc_name", aname) == 0) ) {
								name = (const unsigned char *)avalue;
							} else {
								a = xar_attr_new();
								XAR_ATTR(a)->key = strdup(aname);
								XAR_ATTR(a)->value = strdup(avalue);
								XAR_ATTR(a)->next = attrs;
								attrs = XAR_ATTR(a);
							}
						}
					}

					s = xar_subdoc_new(x, (const char *)name);
					XAR_SUBDOC(s)->attrs = attrs;
					xar_subdoc_unserialize(s, reader);
				}
			}
			if( (type == XML_READER_TYPE_END_ELEMENT) && (strcmp((const char *)name, "toc")==0) ) {
				break;
			}
		}
		if( ret == -1 ) {
			xmlFreeTextReader(reader);
			xmlDictCleanup();
			xmlCleanupCharEncodingHandlers();
			return -1;
		}
	}

	if( ret == -1 ) {
		xmlFreeTextReader(reader);
		xmlDictCleanup();
		xmlCleanupCharEncodingHandlers();
		return -1;
	}
		
	xmlFreeTextReader(reader);
	xmlDictCleanup();
	xmlCleanupCharEncodingHandlers();
	return 0;
}
