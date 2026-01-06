/*
 * Copyright (c) 2005-2008 Rob Braun
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
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <inttypes.h>
#include <sys/types.h>
#include <assert.h>

#ifndef HAVE_ASPRINTF
#include "asprintf.h"
#endif
#include "xar.h"
#include "filetree.h"
#include "archive.h"
#include "io.h"
#include "zxar.h"
#include "bzxar.h"
#include "lzmaxar.h"
#include "hash.h"
#include "script.h"
#include "macho.h"
#include "util.h"

#if !defined(LLONG_MAX) && defined(LONG_LONG_MAX)
#define LLONG_MAX LONG_LONG_MAX
#endif

#if !defined(LLONG_MIN) && defined(LONG_LONG_MIN)
#define LLONG_MIN LONG_LONG_MIN
#endif

static const struct datamod xar_datamods[] = {
	{ xar_hash_archived,
	  xar_hash_unarchived_out,
	  xar_hash_out_done,
	  xar_hash_unarchived,
	  xar_hash_archived_in,
	  xar_hash_done
	},
	{ (fromheap_in)NULL,
	  (fromheap_out)NULL,
	  (fromheap_done)NULL,
	  xar_script_in,
	  (toheap_out)NULL,
	  xar_script_done
	},
	{ (fromheap_in)NULL,
	  (fromheap_out)NULL,
	  (fromheap_done)NULL,
	  xar_macho_in,
	  (toheap_out)NULL,
	  xar_macho_done
	},
	{ xar_gzip_fromheap_in,
	  (fromheap_out)NULL,
	  xar_gzip_fromheap_done,
	  xar_gzip_toheap_in,
	  (toheap_out)NULL,
	  xar_gzip_toheap_done
	},
	{ xar_bzip_fromheap_in,
	  (fromheap_out)NULL,
	  xar_bzip_fromheap_done,
	  xar_bzip_toheap_in,
	  (toheap_out)NULL,
	  xar_bzip_toheap_done
	},
	{ xar_lzma_fromheap_in,
	  (fromheap_out)NULL,
	  xar_lzma_fromheap_done,
	  xar_lzma_toheap_in,
	  (toheap_out)NULL,
	  xar_lzma_toheap_done
	}
};

static const is_compressed xar_compresschecks[] = {
	xar_gzip_is_compressed,
	xar_bzip_is_compressed,
	xar_lzma_is_compressed
};


static size_t get_rsize(xar_t x) {
	size_t bsize;
	const char *opt = NULL;

	opt = xar_opt_get(x, "rsize");
	if( !opt ) {
		bsize = XAR_DEFAULT_BUFFER_SIZE;
	} else {
		bsize = strtol(opt, NULL, 0);
		if( ((bsize == (size_t)LONG_MAX) || (bsize == (size_t)LONG_MIN)) && (errno == ERANGE) ) {
			bsize = XAR_DEFAULT_BUFFER_SIZE;
		}
		if (bsize < XAR_MINIMUM_BUFFER_SIZE)
			bsize = XAR_MINIMUM_BUFFER_SIZE;
	}

	return bsize;
}
	
static off_t get_offset(xar_t x, xar_file_t f, xar_prop_t p) {
	off_t seekoff;
	xar_prop_t tmpp;
	const char *opt = NULL;

	(void)x; (void)f;
	tmpp = xar_prop_pget(p, "offset");
	if( tmpp )
		opt = xar_prop_getvalue(tmpp);
	
	seekoff = strtoll(opt, NULL, 0);
		
	if( ((seekoff == LLONG_MAX) || (seekoff == LLONG_MIN)) && (errno == ERANGE) ) {
		return -1;
	}

	return seekoff;
}

static int64_t get_length(xar_prop_t p) {
	const char *opt = NULL;
	int64_t fsize = 0;
	xar_prop_t tmpp;

	tmpp = xar_prop_pget(p, "length");
	if( tmpp )
		opt = xar_prop_getvalue(tmpp);
	if( !opt ) {
		return 0;
	} else {
		fsize = strtoll(opt, NULL, 10);
		if( ((fsize == LLONG_MAX) || (fsize == LLONG_MIN)) && (errno == ERANGE) ) {
			return -1;
		}
	}

	return fsize;
}

static void xar_io_seek(xar_t x, xar_file_t f, off_t seekoff) {
	int r;

	if( XAR(x)->fd >= 0 ) {
		r = (int)lseek(XAR(x)->fd, seekoff, SEEK_SET);
		if( r == -1 ) {
			if( errno == ESPIPE ) {
				ssize_t rr;
				char *buf;
				unsigned int len;

				len = (unsigned)(seekoff - (off_t)xar_get_heap_offset(x));
				if( XAR(x)->heap_offset > len ) {
					xar_err_new(x);
					xar_err_set_file(x, f);
					xar_err_set_string(x, "Unable to seek");
					xar_err_callback(x, XAR_SEVERITY_NONFATAL, XAR_ERR_ARCHIVE_EXTRACTION);
				} else {
					len -= (unsigned)XAR(x)->heap_offset;
					buf = malloc(len);
					assert(buf);
					rr = xar_read_fd(XAR(x)->fd, buf, len);
					if( rr < (ssize_t)len ) {
						xar_err_new(x);
						xar_err_set_file(x, f);
						xar_err_set_string(x, "Unable to seek");
						xar_err_callback(x, XAR_SEVERITY_NONFATAL, XAR_ERR_ARCHIVE_EXTRACTION);
					}
					XAR(x)->heap_offset += rr;
					free(buf);
				}
			} else {
				xar_err_new(x);
				xar_err_set_file(x, f);
				xar_err_set_string(x, "Unable to seek");
				xar_err_callback(x, XAR_SEVERITY_NONFATAL, XAR_ERR_ARCHIVE_EXTRACTION);
			}
		}
	}
}

int32_t xar_attrcopy_to_heap(xar_t x, xar_file_t f, xar_prop_t p, read_callback rcb, void *context) {
	void	*modulecontext[sizeof(xar_datamods)/sizeof(struct datamod)];
	int modulecount = (int)(sizeof(modulecontext)/sizeof(modulecontext[0]));
	int r, off, i;
	size_t bsize, rsize;
	int64_t readsize=0, writesize=0, inc = 0;
	void *inbuf;
	char *tmpstr = NULL;
	const char *opt = NULL, *csum = NULL;
	off_t orig_heap_offset = XAR(x)->heap_offset;
	xar_file_t tmpf = NULL;
	xar_prop_t tmpp = NULL;

	memset(modulecontext, 0, sizeof(void*)*modulecount);

	bsize = get_rsize(x);

	r = 1;
	while(r != 0) {
		inbuf = malloc(bsize);
		if( !inbuf )
			return -1;

		r = rcb(x, f, inbuf, bsize, context);
		if( r < 0 ) {
			free(inbuf);
			return -1;
		}

		readsize+=r;
		inc += r;
		rsize = r;

		/* filter the data through the in modules */
		for( i = 0; i < modulecount; i++) {
			if( xar_datamods[i].th_in ) {
				xar_datamods[i].th_in(x, f, p, &inbuf, &rsize, &(modulecontext[i]));
			}
		}

		/* filter the data through the out modules */
		for( i = 0; i < modulecount; i++) {
			if( xar_datamods[i].th_out )
				xar_datamods[i].th_out(x, f, p, inbuf, rsize, &(modulecontext[i]));
		}

		off = 0;
		if( rsize != 0 ) {
			do {
				r = (int)write(XAR(x)->heap_fd, ((char *)inbuf)+off, rsize-off);
				if( (r < 0) && (errno != EINTR) )
					return -1;
				off += r;
				writesize += r;
			} while( (size_t)off < rsize );
		}
		XAR(x)->heap_offset += off;
		free(inbuf);
		
	}


	/* If size is 0, don't bother having anything in the heap */
	if( readsize == 0 ) {
		XAR(x)->heap_offset = orig_heap_offset;
		lseek(XAR(x)->heap_fd, -writesize, SEEK_CUR);
		for( i = 0; i < modulecount; i++) {
			if( xar_datamods[i].th_done )
				xar_datamods[i].th_done(x, f, p, &(modulecontext[i]));
		}
		return 0;
	}
	/* finish up anything that still needs doing */
	for( i = 0; i < modulecount; i++) {
		if( xar_datamods[i].th_done )
			xar_datamods[i].th_done(x, f, p, &(modulecontext[i]));
	}

	XAR(x)->heap_len += writesize;
	tmpp = xar_prop_pget(p, "archived-checksum");
	if( tmpp )
		csum = xar_prop_getvalue(tmpp);
	if( csum )
		tmpf = xmlHashLookup(XAR(x)->csum_hash, BAD_CAST(csum));
	if( tmpf ) {
		const char *attr = xar_prop_getkey(p);
		opt = xar_opt_get(x, XAR_OPT_LINKSAME);
		if( opt && (strcmp(attr, "data") == 0) ) {
			const char *id = xar_attr_pget(tmpf, NULL, "id");
			xar_prop_pset(f, NULL, "type", "hardlink");
			tmpp = xar_prop_pfirst(f);
			if( tmpp )
				tmpp = xar_prop_find(tmpp, "type");
			if( tmpp )
				xar_attr_pset(f, tmpp, "link", id);

			xar_prop_pset(tmpf, NULL, "type", "hardlink");
			tmpp = xar_prop_pfirst(tmpf);
			if( tmpp )
				tmpp = xar_prop_find(tmpp, "type");
			if( tmpp )
				xar_attr_pset(tmpf, tmpp, "link", "original");
			
			tmpp = xar_prop_pfirst(f);
			if( tmpp )
				tmpp = xar_prop_find(tmpp, "data");
			xar_prop_punset(f, tmpp);

			XAR(x)->heap_offset = orig_heap_offset;
			lseek(XAR(x)->heap_fd, -writesize, SEEK_CUR);
			XAR(x)->heap_len -= writesize;
			return 0;
		} 
		opt = xar_opt_get(x, XAR_OPT_COALESCE);
		if( opt ) {
			long long tmpoff;
			const char *offstr = NULL;
			tmpp = xar_prop_pfirst(tmpf);
			if( tmpp ) {
				const char *key;
				key = xar_prop_getkey(p);
				tmpp = xar_prop_find(tmpp, key);
			}
			if( tmpp )
				tmpp = xar_prop_pget(tmpp, "offset");
			if( tmpp )
				offstr = xar_prop_getvalue(tmpp);
			if( offstr ) {
				tmpoff = strtoll(offstr, NULL, 10);
				XAR(x)->heap_offset = orig_heap_offset;
				lseek(XAR(x)->heap_fd, -writesize, SEEK_CUR);
				orig_heap_offset = tmpoff;
				XAR(x)->heap_len -= writesize;
			}
			
		}
	} else if( csum ) {
		xmlHashAddEntry(XAR(x)->csum_hash, BAD_CAST(csum), XAR_FILE(f));
	} else {
		xar_err_new(x);
		xar_err_set_file(x, f);
		xar_err_set_string(x, "No archived-checksum");
		xar_err_callback(x, XAR_SEVERITY_WARNING, XAR_ERR_ARCHIVE_CREATION);
	}

	if (asprintf(&tmpstr, "%"PRIu64, readsize) == -1)
		return -1;
	xar_prop_pset(f, p, "size", tmpstr);
	free(tmpstr);

	if (asprintf(&tmpstr, "%"PRIu64, (uint64_t)orig_heap_offset) == -1)
		return -1;
	xar_prop_pset(f, p, "offset", tmpstr);
	free(tmpstr);

	tmpp = xar_prop_pget(p, "encoding");
	if (!tmpp) {
		xar_prop_pset(f, p, "encoding", NULL);
		tmpp = xar_prop_pget(p, "encoding");
	}
	opt = xar_attr_pget(f, tmpp, "style");
	if (tmpp && (!opt || !*opt)) {
		xar_attr_pset(f, tmpp, "style", "application/octet-stream");
	}

	if (asprintf(&tmpstr, "%"PRIu64, writesize) == -1)
		return -1;
	xar_prop_pset(f, p, "length", tmpstr);
	free(tmpstr);

	return 0;
}

/* xar_copy_from_heap
 * This is the arcmod extraction entry point for extracting the file's
 * data from the heap file.
 * It is assumed the heap_fd is already positioned appropriately.
 */
int32_t xar_attrcopy_from_heap(xar_t x, xar_file_t f, xar_prop_t p, write_callback wcb, void *context) {
	void	*modulecontext[sizeof(xar_datamods)/sizeof(struct datamod)];
	int modulecount = (int)(sizeof(modulecontext)/sizeof(modulecontext[0]));
	int r, i;
	size_t bsize, def_bsize;
	int64_t fsize, inc = 0, seekoff;
	void *inbuf;
	const char *opt;
	xar_prop_t tmpp;

	memset(modulecontext, 0, sizeof(void*)*modulecount);

	def_bsize = get_rsize(x);

	opt = NULL;
	tmpp = xar_prop_pget(p, "offset");
	if( tmpp )
		opt = xar_prop_getvalue(tmpp);
	if( !opt ) {
		wcb(x, f, NULL, 0, context);
		return 0;
	} else {
		seekoff = strtoll(opt, NULL, 0);
		if( ((seekoff == LLONG_MAX) || (seekoff == LLONG_MIN)) && (errno == ERANGE) ) {
			return -1;
		}
	}

	seekoff += (int64_t)xar_get_heap_offset(x);
	xar_io_seek(x, f, seekoff);

	fsize = get_length(p);
	if( fsize == 0 )
		return 0;
	if( fsize < 0 )
		return -1;

	bsize = def_bsize;
	inbuf = malloc(bsize);
	if( !inbuf ) {
		return -1;
	}

	while(1) {
		/* Size has been reached */
		if( fsize == inc )
			break;
		if( (fsize - inc) < (int64_t)bsize )
			bsize = (size_t)(fsize - inc);
		r = (int)read(XAR(x)->fd, inbuf, bsize);
		if( r == 0 )
			break;
		if( (r < 0) && (errno == EINTR) )
			continue;
		if( r < 0 ) {
			free(inbuf);
			return -1;
		}

		XAR(x)->heap_offset += r;
		inc += r;
		bsize = r;

		/* filter the data through the in modules */
		for( i = 0; i < modulecount; i++) {
			if( xar_datamods[i].fh_in ) {
				int32_t ret;
				ret = xar_datamods[i].fh_in(x, f, p, &inbuf, &bsize, &(modulecontext[i]));
				if( ret < 0 )
					return -1;
			}
		}
		
		/* Only due the write phase, if there is a write function to call */
		if(wcb){
		
			/* filter the data through the out modules */
			for( i = 0; i < modulecount; i++) {
				if( xar_datamods[i].fh_out ) {
					int32_t ret;
					ret = xar_datamods[i].fh_out(x, f, p, inbuf, bsize, &(modulecontext[i]));
					if( ret < 0 )
						return -1;
				}
			}

			wcb(x, f, inbuf, bsize, context);
		}
		
		free(inbuf);
		bsize = def_bsize;
		inbuf = malloc(bsize);
	}

	free(inbuf);
	/* finish up anything that still needs doing */
	for( i = 0; i < modulecount; i++) {
		if( xar_datamods[i].fh_done ) {
			int32_t ret;
			ret = xar_datamods[i].fh_done(x, f, p, &(modulecontext[i]));
			if( ret < 0 )
				return ret;
		}
	}
	return 0;
}

/* xar_attrcopy_from_heap_to_heap
* This does a simple copy of the heap data from one head (read-only) to another heap (write only). 
* This does not set any properties or attributes of the file, so this should not be used alone.
*/
int32_t xar_attrcopy_from_heap_to_heap(xar_t xsource, xar_file_t fsource, xar_prop_t p, xar_t xdest, xar_file_t fdest){
	int r, off;
	size_t bsize;
	int64_t fsize, inc = 0, seekoff, writesize=0;
	off_t orig_heap_offset = XAR(xdest)->heap_offset;
	void *inbuf;
	const char *opt;
	char *tmpstr = NULL;
	xar_prop_t tmpp;
	
	bsize = get_rsize(xsource);
	
	seekoff = get_offset(xsource, fsource, p);
	if( seekoff < 0 )
		return -1;
	
	seekoff += (int64_t)xar_get_heap_offset(xsource);
	xar_io_seek(xsource, fsource, seekoff);
	
	fsize = get_length(p);
	if( fsize == 0 )
		return 0;
	if( fsize < 0 )
		return -1;
	
	inbuf = malloc(bsize);
	if( !inbuf ) {
		return -1;
	}
	
	
	while(1) {
		/* Size has been reached */
		if( fsize == inc )
			break;
		if( (fsize - inc) < (int64_t)bsize )
			bsize = (size_t)(fsize - inc);
		r = (int)read(XAR(xsource)->fd, inbuf, bsize);
		if( r == 0 )
			break;
		if( (r < 0) && (errno == EINTR) )
			continue;
		if( r < 0 ) {
			free(inbuf);
			return -1;
		}
		
		XAR(xsource)->heap_offset += r;
		inc += r;
		bsize = r;
		
		off = 0;
		
		do {
			r = (int)write(XAR(xdest)->heap_fd, ((char *)inbuf)+off, r-off );
			off += r;
			writesize += r;
		} while( off < r );
		XAR(xdest)->heap_offset += off;
		XAR(xdest)->heap_len += off;
	}
	
	if (asprintf(&tmpstr, "%"PRIu64, (uint64_t)orig_heap_offset) == -1) {
		free(inbuf);
		return -1;
	}
	opt = xar_prop_getkey(p);
	tmpp = xar_prop_pfirst(fdest);
	if( tmpp )
		tmpp = xar_prop_find(tmpp, opt);
	if( tmpp )
		xar_prop_pset(fdest, tmpp, "offset", tmpstr);
	free(tmpstr);
	
	
	free(inbuf);
	
	/* It is the caller's responsibility to copy the attributes of the file, etc, this only copies the data in the heap */
	
	return 0;
}

static int32_t flush_stream(xar_stream *stream) {
	xar_stream_state_t *state = (xar_stream_state_t *)(stream->state);

	if( state->pending_buf && stream->avail_out ) {
		size_t len = state->pending_buf_size;

		if( stream->avail_out < len ) {
			len = stream->avail_out;
		}

		memcpy(stream->next_out, state->pending_buf, len);
		stream->next_out += len;
		stream->avail_out -= (unsigned)len;
		stream->total_out += len;

		if( state->pending_buf_size == len )  {
			state->pending_buf_size = 0;

			free(state->pending_buf);
			state->pending_buf = NULL;
		} else if( state->pending_buf_size > len ) {
			state->pending_buf_size -= len;
			memcpy(state->pending_buf, state->pending_buf + len, state->pending_buf_size);
		}
	}

	return XAR_STREAM_OK;
}

static int32_t write_to_stream(void *inbuf, size_t inlen, xar_stream *stream) {
	xar_stream_state_t *state = (xar_stream_state_t *)stream->state;
	size_t len = inlen;

	if( stream->avail_out < len ) {
		len = stream->avail_out;
	}

	memcpy(stream->next_out, inbuf, len);
	stream->next_out += len;
	stream->avail_out -= (unsigned)len;
	stream->total_out += len;

	if( inlen > len ) {
		state->pending_buf_size = inlen - len;
		state->pending_buf = malloc(state->pending_buf_size);

		memcpy(state->pending_buf, (char *)inbuf + len, state->pending_buf_size);
	}

	return XAR_STREAM_OK;
}

int32_t xar_attrcopy_from_heap_to_stream_init(xar_t x, xar_file_t f, xar_prop_t p, xar_stream *stream) {
	xar_stream_state_t *state;
	off_t seekoff;

	seekoff = get_offset(x, f, p);
	if( seekoff < 0 ) 
		return XAR_STREAM_ERR;

	state = calloc(1, sizeof(xar_stream_state_t));
	if( !state ) {
		return XAR_STREAM_ERR;
	}
	stream->state = (void*)state;
	state->bsize = get_rsize(x);

	state->modulecount = (int)(sizeof(xar_datamods)/sizeof(struct datamod));
	state->modulecontext = calloc(1, sizeof(void*)*state->modulecount);
	if( !state->modulecontext ) {
		free(state);
		return XAR_STREAM_ERR;
	}

	seekoff += (off_t)xar_get_heap_offset(x);
	xar_io_seek(x, f, seekoff);

	stream->total_in = 0;
	stream->total_out = 0;

	state->fsize = get_length(p);

	if(state->fsize == 0) {
		return XAR_STREAM_OK;
	} else if(state->fsize == -1) {
		free(state->modulecontext);
		free(state);
		return XAR_STREAM_ERR;
	}

	state->pending_buf = NULL;
	state->pending_buf_size = 0;
	state->x = x;
	state->f = f;
	state->p = p;

	return XAR_STREAM_OK;
}

int32_t xar_attrcopy_from_heap_to_stream(xar_stream *stream) {
	xar_stream_state_t *state = stream->state;

	int r, i;
	size_t bsize;
	void *inbuf; 

	if( state->pending_buf_size ) {
		return flush_stream(stream);
	} 

	bsize = state->bsize;
	inbuf = malloc(bsize);
	if( !inbuf ) {
		return XAR_STREAM_ERR;
	}
        
	/* Size has been reached */
	if( (uint64_t)state->fsize == stream->total_in ) {
		free(inbuf);
		return XAR_STREAM_END;
	}
	if( (state->fsize - stream->total_in) < bsize )
		bsize = (size_t)(state->fsize - stream->total_in);
	r = (int)read(XAR(state->x)->fd, inbuf, bsize);
	if( r == 0 ) {
		free(inbuf);
		return XAR_STREAM_END;
	}
	if( (r < 0) && (errno == EINTR) ) {
		free(inbuf);
		return XAR_STREAM_OK;
	}
	if( r < 0 ) {
		free(inbuf);
		return XAR_STREAM_ERR;
	}

	XAR(state->x)->heap_offset += r;
	stream->total_in += r;
	bsize = r;
	
	/* filter the data through the in modules */
	for( i = 0; i < state->modulecount; i++) {
	if( xar_datamods[i].fh_in ) {
		int32_t ret;
		ret = xar_datamods[i].fh_in(state->x, state->f, state->p, &inbuf, &bsize, &(state->modulecontext[i]));
		if( ret < 0 )
			return XAR_STREAM_ERR;
		}
	}

	/* filter the data through the out modules */
	for( i = 0; i < state->modulecount; i++) {
		if( xar_datamods[i].fh_out ) {
			int32_t ret;
			ret = xar_datamods[i].fh_out(state->x, state->f, state->p, inbuf, bsize, &(state->modulecontext[i]));
			if( ret < 0 )
				return XAR_STREAM_ERR;
		}
	}

	write_to_stream(inbuf, bsize, stream);

	free(inbuf);

	return XAR_STREAM_OK;
}

int32_t xar_attrcopy_from_heap_to_stream_end(xar_stream *stream) {
	xar_stream_state_t *state = (xar_stream_state_t *)stream->state;
	int i;

	/* finish up anything that still needs doing */
	for( i = 0; i < state->modulecount; i++) {
		if( xar_datamods[i].fh_done ) {
			int32_t ret;
			ret = xar_datamods[i].fh_done(state->x, state->f, state->p, &(state->modulecontext[i]));
			if( ret < 0 )
				return ret;
		}
	}

	if( state->pending_buf ) {
		free(state->pending_buf);
	}
 
	free(state->modulecontext);
	free(state);
       
	return XAR_STREAM_OK;
}

/* xar_heap_to_archive
 * x: archive to operate on
 * Returns 0 on success, -1 on error
 * Summary: copies the heap into the archive.
 */
int32_t xar_heap_to_archive(xar_t x) {
	long bsize;
	ssize_t r;
	int off;
	const char *opt;
	char *b;

	opt = xar_opt_get(x, "rsize");
	if( !opt ) {
		bsize = XAR_DEFAULT_BUFFER_SIZE;
	} else {
		bsize = strtol(opt, NULL, 0);
		if( ((bsize == LONG_MAX) || (bsize == LONG_MIN)) && (errno == ERANGE) ) {
			bsize = XAR_DEFAULT_BUFFER_SIZE;
		}
		if (bsize < XAR_MINIMUM_BUFFER_SIZE)
			bsize = XAR_MINIMUM_BUFFER_SIZE;
	}

	b = malloc(bsize);
	if( !b ) return -1;

	while(1) {
		r = read(XAR(x)->heap_fd, b, bsize);
		if( r == 0 ) break;
		if( (r < 0) && (errno == EINTR) ) continue;
		if( r < 0 ) {
			free(b);
			return -1;
		}

		off = 0;
		do {
			r = write(XAR(x)->fd, b+off, bsize-off);
			if( (r < 0) && (errno != EINTR) )
				return -1;
			off += (int)r;
		} while( off < bsize );
	}
	return 0;
}

/* xar_prevent_recompress
 * x: archive to operate on
 * Returns 0 to allow compression, non-zero to prevent compression
 * Summary: prevents recompression unless XAR_OPT_RECOMPRESS is XAR_OPT_VAL_TRUE
 */
int32_t xar_prevent_recompress(xar_t x, void *in, size_t inlen) {
	int checkcount = (int)(sizeof(xar_compresschecks)/sizeof(xar_compresschecks[0]));
	int i;
	const char *opt;
	int recompress = 0;

	opt = xar_opt_get(x, XAR_OPT_RECOMPRESS);
	if( opt && (strcmp(opt, XAR_OPT_VAL_TRUE) == 0) ) {
		recompress = 1;
	}

	if (recompress)
		return 0;

	/* check with the modules */
	for( i = 0; i < checkcount; ++i) {
		if ((*xar_compresschecks[i])(in, inlen))
			return 1;
	}

	return 0;
}
