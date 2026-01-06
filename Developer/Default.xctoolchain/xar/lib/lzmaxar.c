/*
 * Copyright (c) 2005 Rob Braun
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
 * 19-Sep-2007
 * DRI: Anders F Bjorklund <afb@rpm5.org>
 */
/*
 * Portions Copyright 2006, Apple Computer, Inc.
 * Christopher Ryan <ryanc@apple.com>
*/


#include "config.h"
#ifndef HAVE_ASPRINTF
#include "asprintf.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <errno.h>
#ifdef HAVE_LIBLZMA
#include <lzma.h>
#endif
#include "xar.h"
#include "filetree.h"
#include "io.h"

#ifdef HAVE_LIBLZMA

#ifndef UINT32_C
#define UINT32_C(v)  (v ## U) /* from <stdint.h> normally */
#endif
#ifndef LZMA_VERSION
#define LZMA_VERSION UINT32_C(40420000) /* = 4.42.0alpha6 */
#endif

struct _lzma_context{
	uint8_t		lzmacompressed;
	uint8_t		lzmaalone;
	lzma_stream	lzma;
#if LZMA_VERSION < 49990050U
	lzma_options_stream options;
#elif LZMA_VERSION < 49990060U
	lzma_check	check;
	lzma_filter filters[2];
#endif
#if LZMA_VERSION < 49990050U
	lzma_options_alone options2;
#else
	lzma_options_lzma options2;
#endif
#if LZMA_VERSION < 49990060U
	lzma_allocator allocator;
#if LZMA_VERSION < 40420010U
	lzma_memory_limitter *limit;
#else
	lzma_memlimit *limit;
#endif
#endif
};

#define preset_level 7
#define memory_limit 400*1024*1024 /* 1=1M, 5=24M, 6=39M, 7=93M, 8=185M, 9=369M */

#define LZMA_CONTEXT(x) ((struct _lzma_context *)(*x))
#endif

int xar_lzma_fromheap_done(xar_t x, xar_file_t f, xar_prop_t p, void **context) {
#ifdef HAVE_LIBLZMA

	(void)x; (void)f; (void)p;
	if( !context || !LZMA_CONTEXT(context) )
		return 0;

	if( LZMA_CONTEXT(context)->lzmacompressed){
		lzma_end(&LZMA_CONTEXT(context)->lzma);		
	}

	/* free the context */
	free(LZMA_CONTEXT(context));
	*context = NULL;
	
#endif
	(void)x; (void)f; (void)p; (void)context;
	return 0;
}

int xar_lzma_fromheap_in(xar_t x, xar_file_t f, xar_prop_t p, void **in, size_t *inlen, void **context) {
	const char *opt;
	xar_prop_t tmpp;
#ifdef HAVE_LIBLZMA
	uint8_t alone;
	void *out = NULL;
	size_t outlen, offset = 0;
	lzma_ret r;
	
	/* on first run, we init the context and check the compression type */
	if( !LZMA_CONTEXT(context) ) {
		*context = calloc(1,sizeof(struct _lzma_context));
	
		opt = NULL;
		tmpp = xar_prop_pget(p, "encoding");
		if( tmpp )
			opt = xar_attr_pget(f, tmpp, "style");
		if( !opt ) return 0;
		if( strcmp(opt, "application/x-lzma") == 0 )
			alone = 1;
		else if( strcmp(opt, "application/x-xz") == 0 )
			alone = 0;
		else
			return 0;
		
	#if LZMA_VERSION < 49990070U
		lzma_init_decoder();
	#endif
	#ifdef LZMA_STREAM_INIT_VAR
		LZMA_CONTEXT(context)->lzma = LZMA_STREAM_INIT_VAR;
	#endif
	#if LZMA_VERSION < 49990050U
		r = lzma_auto_decoder(&LZMA_CONTEXT(context)->lzma, NULL, NULL);
	#elif LZMA_VERSION < 49990060U
		r = lzma_auto_decoder(&LZMA_CONTEXT(context)->lzma,
		        lzma_easy_memory_usage(preset_level), 0);
	#elif LZMA_VERSION < 49990070U
		r = lzma_auto_decoder(&LZMA_CONTEXT(context)->lzma,
		        lzma_easy_encoder_memusage(preset_level, 0), 0);
	#else
		r = lzma_auto_decoder(&LZMA_CONTEXT(context)->lzma,
		        lzma_easy_encoder_memusage(preset_level), 0);
	#endif
		if( (r != LZMA_OK) ) {
			xar_err_new(x);
			xar_err_set_file(x, f);
			xar_err_set_string(x, "Error decompressing file");
			xar_err_callback(x, XAR_SEVERITY_FATAL, XAR_ERR_ARCHIVE_EXTRACTION);
			return -1;
		}
		LZMA_CONTEXT(context)->lzmacompressed = 1;
		LZMA_CONTEXT(context)->lzmaalone = alone;
	}else if( !LZMA_CONTEXT(context)->lzmacompressed ){
		/* once the context has been initialized, then we have already
		   checked the compression type, so we need only check if we
		   actually are compressed */
		return 0;
	}

	outlen = *inlen;

	LZMA_CONTEXT(context)->lzma.next_in = (const uint8_t *)*in;
	LZMA_CONTEXT(context)->lzma.avail_in = (size_t)*inlen;
	LZMA_CONTEXT(context)->lzma.next_out = (uint8_t *)out;
	LZMA_CONTEXT(context)->lzma.avail_out = 0;

	while( LZMA_CONTEXT(context)->lzma.avail_in != 0 ) {
		outlen = outlen * 2;
		out = realloc(out, outlen);
		if( out == NULL ) abort();

		LZMA_CONTEXT(context)->lzma.next_out = ((unsigned char *)out) + offset;
		LZMA_CONTEXT(context)->lzma.avail_out = outlen - offset;

		r = lzma_code(&(LZMA_CONTEXT(context)->lzma), LZMA_RUN);
		if( (r != LZMA_OK) && (r != LZMA_STREAM_END) ) {
			xar_err_new(x);
			xar_err_set_file(x, f);
			xar_err_set_errno(x, r);
			xar_err_set_string(x, "Error decompressing file");
			xar_err_callback(x, XAR_SEVERITY_FATAL, XAR_ERR_ARCHIVE_EXTRACTION);
			return -1;
		}
		offset += outlen - offset - LZMA_CONTEXT(context)->lzma.avail_out;
		if( (r == LZMA_STREAM_END) && (offset == 0) )
			break;
	}

	free(*in);
	*in = out;
	*inlen = offset;
#else
	(void)in; (void)inlen; (void)context;
	opt = NULL;
	tmpp = xar_prop_pget(p, "encoding");
	if( tmpp )
		opt = xar_attr_pget(f, tmpp, "style");
	if( !opt ) return 0;
	if( strcmp(opt, "application/x-lzma") != 0 &&
	    strcmp(opt, "application/x-xz") != 0) return 0;
	xar_err_new(x);
	xar_err_set_file(x, f);
	xar_err_set_errno(x, 0);
	xar_err_set_string(x, "lzma support not compiled in.");
	xar_err_callback(x, XAR_SEVERITY_FATAL, XAR_ERR_ARCHIVE_EXTRACTION);

#endif /* HAVE_LIBLZMA */
	return 0;
}

int xar_lzma_toheap_done(xar_t x, xar_file_t f, xar_prop_t p, void **context) {
#ifdef HAVE_LIBLZMA
	xar_prop_t tmpp;

	(void)x;
	if( LZMA_CONTEXT(context)->lzmacompressed){
		lzma_end(&LZMA_CONTEXT(context)->lzma);		
#if LZMA_VERSION < 40420010U
		lzma_memory_limitter_end(LZMA_CONTEXT(context)->limit, 1);
#elif LZMA_VERSION < 49990060U
		lzma_memlimit_end(LZMA_CONTEXT(context)->limit, 1);
#endif

		tmpp = xar_prop_pset(f, p, "encoding", NULL);
		if( tmpp ){
			if( LZMA_CONTEXT(context)->lzmaalone )
			xar_attr_pset(f, tmpp, "style", "application/x-lzma");
			else
			xar_attr_pset(f, tmpp, "style", "application/x-xz");
		}
	}

	/* free the context */
	free(LZMA_CONTEXT(context));
	*context = NULL;
	
#endif /* HAVE_LIBLZMA */
	(void)x; (void)f; (void)p; (void)context;
	return 0;
}

int32_t xar_lzma_toheap_in(xar_t x, xar_file_t f, xar_prop_t p, void **in, size_t *inlen, void **context) {
	const char *opt;
#ifdef HAVE_LIBLZMA
	uint8_t alone;
	void *out = NULL;
	size_t outlen, offset = 0;
	lzma_ret r;

	(void)p;
	/* on first run, we init the context and check the compression type */
	if( !LZMA_CONTEXT(context) ) {
		int level = preset_level;
		*context = calloc(1,sizeof(struct _lzma_context));
		
		opt = xar_opt_get(x, XAR_OPT_COMPRESSION);
		if( !opt )
			return 0;
		
		if( strcmp(opt, XAR_OPT_VAL_LZMA) == 0 )
			alone = 1;
		else if( strcmp(opt, XAR_OPT_VAL_XZ) == 0 )
			alone = 0;
		else
			return 0;

		if( xar_prevent_recompress(x, *in, *inlen) )
			return 0;

		opt = xar_opt_get(x, XAR_OPT_COMPRESSIONARG);
		if( opt ) {
			int tmp;
			errno = 0;
			tmp = strtol(opt, NULL, 10);
			if( errno == 0 ) {
				if( (level >= 0) && (level <= 9) )
					level = tmp;
			}
		}
		
#if LZMA_VERSION < 49990070U
		lzma_init_encoder();
#endif
#if LZMA_VERSION < 49990050U
		LZMA_CONTEXT(context)->options.check = LZMA_CHECK_CRC64;
		LZMA_CONTEXT(context)->options.has_crc32 = 1; /* true */
		LZMA_CONTEXT(context)->options.alignment = 0;
		/* don't use LZMA_FILTER_POWERPC or LZMA_FILTER_X86 filters,
		   since they might make compression worse for regular files */
		LZMA_CONTEXT(context)->options.filters[0].id = LZMA_FILTER_LZMA;
		LZMA_CONTEXT(context)->options.filters[0].options = (lzma_options_lzma *)(lzma_preset_lzma + level - 1);
		/* Terminate the filter options array. */
		LZMA_CONTEXT(context)->options.filters[1].id = UINT64_MAX;
#elif LZMA_VERSION < 49990060U
		LZMA_CONTEXT(context)->check = LZMA_CHECK_CRC64;
		LZMA_CONTEXT(context)->filters[0].id = LZMA_FILTER_LZMA;
		LZMA_CONTEXT(context)->filters[0].options = (lzma_options_lzma *)(lzma_preset_lzma + level - 1);
		/* Terminate the filter options array. */
		LZMA_CONTEXT(context)->filters[1].id = UINT64_MAX;
#endif
#ifdef LZMA_STREAM_INIT_VAR
		LZMA_CONTEXT(context)->lzma = LZMA_STREAM_INIT_VAR;
#endif
#if LZMA_VERSION < 49990060U
#if LZMA_VERSION < 40420010U
		LZMA_CONTEXT(context)->limit = lzma_memory_limitter_create(memory_limit);
		LZMA_CONTEXT(context)->allocator.alloc = (void*) lzma_memory_alloc;
		LZMA_CONTEXT(context)->allocator.free = (void*) lzma_memory_free;
#else
		LZMA_CONTEXT(context)->limit = lzma_memlimit_create(memory_limit);
		LZMA_CONTEXT(context)->allocator.alloc = (void*) lzma_memlimit_alloc;
		LZMA_CONTEXT(context)->allocator.free = (void*) lzma_memlimit_free;
#endif
		LZMA_CONTEXT(context)->allocator.opaque = LZMA_CONTEXT(context)->limit;
		LZMA_CONTEXT(context)->lzma.allocator = &LZMA_CONTEXT(context)->allocator;
#else
		lzma_memlimit_set(&LZMA_CONTEXT(context)->lzma, memory_limit);
#endif
#if LZMA_VERSION < 49990050U
		if (alone){
		LZMA_CONTEXT(context)->options2.uncompressed_size = *inlen;
		memcpy(&LZMA_CONTEXT(context)->options2.lzma,
		       (lzma_options_lzma *)(lzma_preset_lzma + level - 1), sizeof(lzma_options_lzma));
		r = lzma_alone_encoder(&LZMA_CONTEXT(context)->lzma,
		                       &(LZMA_CONTEXT(context)->options2));
		}
		else
		r = lzma_stream_encoder_single(&LZMA_CONTEXT(context)->lzma,
		                               &(LZMA_CONTEXT(context)->options));
#elif LZMA_VERSION < 49990060U
		if (alone){
		memcpy(&LZMA_CONTEXT(context)->options2,
		       (lzma_options_lzma *)(lzma_preset_lzma + level - 1), sizeof(lzma_options_lzma));
		r = lzma_alone_encoder(&LZMA_CONTEXT(context)->lzma,
		                       &(LZMA_CONTEXT(context)->options2));
		}
		else
		r = lzma_stream_encoder(&LZMA_CONTEXT(context)->lzma,
		                        LZMA_CONTEXT(context)->filters, LZMA_CONTEXT(context)->check);
#elif LZMA_VERSION < 49990070U
		if (alone){
		lzma_lzma_preset(&(LZMA_CONTEXT(context)->options2), level);
		r = lzma_alone_encoder(&LZMA_CONTEXT(context)->lzma,
		                       &(LZMA_CONTEXT(context)->options2));
		}
		else
		r = lzma_easy_encoder(&LZMA_CONTEXT(context)->lzma,
		                      level, 0, LZMA_CHECK_CRC64);
#else
		if (alone){
		lzma_lzma_preset(&(LZMA_CONTEXT(context)->options2), level);
		r = lzma_alone_encoder(&LZMA_CONTEXT(context)->lzma,
		                       &(LZMA_CONTEXT(context)->options2));
		}
		else
		r = lzma_easy_encoder(&LZMA_CONTEXT(context)->lzma,
		                      level, LZMA_CHECK_CRC64);
#endif
		if( (r != LZMA_OK) ) {
			xar_err_new(x);
			xar_err_set_file(x, f);
			xar_err_set_errno(x, r);
			xar_err_set_string(x, "Error compressing file");
			xar_err_callback(x, XAR_SEVERITY_FATAL, XAR_ERR_ARCHIVE_CREATION);
			return -1;
		}
		LZMA_CONTEXT(context)->lzmacompressed = 1;
		LZMA_CONTEXT(context)->lzmaalone = alone;
		if( *inlen == 0 )
			return 0;
	}else if( !LZMA_CONTEXT(context)->lzmacompressed ){
		/* once the context has been initialized, then we have already
		checked the compression type, so we need only check if we
		actually are compressed */
		return 0;
	}
	
	outlen = *inlen/2;
	if(outlen == 0) outlen = 1024;
	LZMA_CONTEXT(context)->lzma.next_in = (const uint8_t *)*in;
	LZMA_CONTEXT(context)->lzma.avail_in = (size_t)*inlen;
	LZMA_CONTEXT(context)->lzma.next_out = (uint8_t *)out;
	LZMA_CONTEXT(context)->lzma.avail_out = 0;

	if( *inlen != 0 ) {
		do {
			outlen *= 2;
			out = realloc(out, outlen);
			if( out == NULL ) abort();

			LZMA_CONTEXT(context)->lzma.next_out = ((unsigned char *)out) + offset;
			LZMA_CONTEXT(context)->lzma.avail_out = outlen - offset;

			r = lzma_code(&LZMA_CONTEXT(context)->lzma, LZMA_RUN);
			offset = outlen - LZMA_CONTEXT(context)->lzma.avail_out;
		} while( r == LZMA_OK && LZMA_CONTEXT(context)->lzma.avail_in != 0);
	} else {
		do {
			outlen *= 2;
			out = realloc(out, outlen);
			if( out == NULL ) abort();

			LZMA_CONTEXT(context)->lzma.next_out = ((unsigned char *)out) + offset;
			LZMA_CONTEXT(context)->lzma.avail_out = outlen - offset;

			r = lzma_code(&LZMA_CONTEXT(context)->lzma, LZMA_FINISH);
			offset = outlen - LZMA_CONTEXT(context)->lzma.avail_out;
		} while( r == LZMA_OK && r != LZMA_STREAM_END);
	}
	
	if( (r != LZMA_OK && r != LZMA_STREAM_END) ) {
		xar_err_new(x);
		xar_err_set_file(x, f);
		xar_err_set_string(x, "Error compressing file");
		xar_err_set_errno(x, r);
		xar_err_callback(x, XAR_SEVERITY_FATAL, XAR_ERR_ARCHIVE_CREATION);
		return -1;
	}

	free(*in);
	*in = out;
	*inlen = offset;
#else
	(void)p; (void)in; (void)inlen; (void)context;
	opt = xar_opt_get(x, XAR_OPT_COMPRESSION);
	if( !opt )
		return 0;
	if( strcmp(opt, XAR_OPT_VAL_LZMA) != 0 &&
	    strcmp(opt, XAR_OPT_VAL_XZ) != 0 )
		return 0;
	xar_err_new(x);
	xar_err_set_file(x, f);
	xar_err_set_errno(x, 0); 
	xar_err_set_string(x, "lzma support not compiled in.");
	xar_err_callback(x, XAR_SEVERITY_FATAL, XAR_ERR_ARCHIVE_CREATION);

#endif /* HAVE_LIBLZMA */
	return 0;
}

int xar_lzma_is_compressed(void *in, size_t inlen)
{
	if( !in || inlen < 6 )
		return 0;
	return memcmp(in, "\xfd\x37zXZ\x00", 6) == 0;
}
