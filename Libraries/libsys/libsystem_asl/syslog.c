/*
 * Copyright (c) 1999-2015 Apple Inc. All rights reserved.
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
 * Copyright (c) 1993
 *	The Regents of the University of California.  All rights reserved.
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
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <stdio.h>

#include <sys/syslog.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdarg.h>
#include <pthread.h>
#include <dispatch/dispatch.h>
#include <asl.h>
#include <asl_msg.h>
#include <asl_private.h>
#include <os/log.h>
#include <os/log_private.h>

#define	LOG_NO_NOTIFY	0x1000
extern const char *asl_syslog_faciliy_num_to_name(int n);

#ifdef BUILDING_VARIANT
__private_extern__ pthread_mutex_t _sl_lock;
__private_extern__ asl_object_t _sl_asl;
__private_extern__ char *_sl_ident;
__private_extern__ int _sl_fac;
__private_extern__ int _sl_opts;
__private_extern__ int _sl_mask;
#else /* !BUILDING_VARIANT */
__private_extern__ pthread_mutex_t _sl_lock = PTHREAD_MUTEX_INITIALIZER;
__private_extern__ asl_object_t _sl_asl = NULL;
__private_extern__ char *_sl_ident = NULL;
__private_extern__ int _sl_fac = 0;
__private_extern__ int _sl_opts = 0;
__private_extern__ int _sl_mask = 0;
#endif /* BUILDING_VARIANT */

#define EVAL_ASL (EVAL_SEND_ASL | EVAL_TEXT_FILE | EVAL_ASL_FILE)

static const os_log_type_t shim_syslog_to_log_type[8] = {
	OS_LOG_TYPE_DEFAULT,    // LOG_EMERG
	OS_LOG_TYPE_DEFAULT,    // LOG_ALERT
	OS_LOG_TYPE_DEFAULT,    // LOG_CRIT
	OS_LOG_TYPE_DEFAULT,    // LOG_ERR
	OS_LOG_TYPE_DEFAULT,    // LOG_WARNING
	OS_LOG_TYPE_DEFAULT,    // LOG_NOTICE
	OS_LOG_TYPE_INFO,       // LOG_INFO
	OS_LOG_TYPE_DEBUG       // LOG_DEBUG
};

extern uint32_t _asl_evaluate_send(asl_object_t client, asl_object_t m, int slevel);
extern uint32_t _asl_lib_vlog(asl_object_t obj, uint32_t eval, asl_object_t msg, const char *format, va_list ap);
extern uint32_t _asl_lib_vlog_text(asl_object_t obj, uint32_t eval, asl_object_t msg, const char *format, va_list ap);

static void
_syslog_asl_client()
{
	pthread_mutex_lock(&_sl_lock);
	if (_sl_asl == NULL)
	{
		_sl_asl = asl_open(NULL, NULL, ASL_OPT_SYSLOG_LEGACY);
		_sl_mask = ASL_FILTER_MASK_UPTO(ASL_LEVEL_DEBUG);
		asl_set_filter(_sl_asl, _sl_mask);
	}
	pthread_mutex_unlock(&_sl_lock);
}

static void
_vsyslog(int pri, const char *fmt, va_list ap, void *addr, bool mirror)
{
	int level = pri & LOG_PRIMASK;
	int fac = pri & LOG_FACMASK;
	asl_object_t msg;
	uint32_t eval;
	bool trace;

	_syslog_asl_client();

	msg = asl_new(ASL_TYPE_MSG);

	if (fac != 0)
	{
		const char *facility = asl_syslog_faciliy_num_to_name(fac);
		if (facility != NULL) asl_set(msg, ASL_KEY_FACILITY, facility);
	}

	eval = _asl_evaluate_send(_sl_asl, msg, level);
	trace = (eval & EVAL_SEND_TRACE) && os_log_shim_enabled(addr);

	if (trace)
	{
		va_list ap_copy;
		os_log_type_t type = shim_syslog_to_log_type[level];

		va_copy(ap_copy, ap);
		os_log_with_args(OS_LOG_DEFAULT, type, fmt, ap_copy, addr);
		va_end(ap_copy);

		if ((eval & EVAL_TEXT_FILE) && !mirror)
		{
			_asl_lib_vlog_text(_sl_asl, eval, msg, fmt, ap);
		}
	}

	if ((eval & EVAL_ASL) && (mirror || !trace))
	{
		_asl_lib_vlog(_sl_asl, eval, msg, fmt, ap);
	}

	asl_release(msg);
}

#if TARGET_OS_OSX

extern typeof(syslog) syslog_legacy asm("_syslog");
extern typeof(syslog) syslog_os_log asm("_syslog" __DARWIN_SUF_EXTSN);

void
syslog_legacy(int pri, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	_vsyslog(pri, fmt, ap, __builtin_return_address(0), true);
	va_end(ap);
}

void
syslog_os_log(int pri, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	_vsyslog(pri, fmt, ap, __builtin_return_address(0), false);
	va_end(ap);
}

#else /* !TARGET_OS_OSX */

void
syslog(int pri, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	_vsyslog(pri, fmt, ap, __builtin_return_address(0), false);
	va_end(ap);
}

#endif /* !TARGET_OS_OSX */

void
vsyslog(int pri, const char *fmt, va_list ap)
{
	_vsyslog(pri, fmt, ap, __builtin_return_address(0), false);
}

#ifndef BUILDING_VARIANT

void
openlog(const char *ident, int opts, int logfac)
{
	const char *facility;
	uint32_t asl_opts;

	pthread_mutex_lock(&_sl_lock);

	if (_sl_asl != NULL) asl_release(_sl_asl);
	_sl_asl = NULL;

	free(_sl_ident);
	_sl_ident = NULL;

	/* open with specified parameters */

	if (ident != NULL) _sl_ident = strdup(ident);
	/* NB we allow the strdup to fail silently */

	_sl_fac = logfac;
	facility = asl_syslog_faciliy_num_to_name(_sl_fac);

	_sl_opts = opts;
	asl_opts = ASL_OPT_SYSLOG_LEGACY;

	if (_sl_opts & LOG_NO_NOTIFY) asl_opts |= ASL_OPT_NO_REMOTE;
	if (_sl_opts & LOG_PERROR) asl_opts |= ASL_OPT_STDERR;

	_sl_mask = ASL_FILTER_MASK_UPTO(ASL_LEVEL_DEBUG);

	_sl_asl = asl_open(_sl_ident, facility, asl_opts);
	asl_set_filter(_sl_asl, _sl_mask);

	pthread_mutex_unlock(&_sl_lock);
}

void
closelog()
{
	pthread_mutex_lock(&_sl_lock);

	if (_sl_asl != NULL) asl_close(_sl_asl);
	_sl_asl = NULL;

	free(_sl_ident);
	_sl_ident = NULL;

	_sl_fac = 0;

	pthread_mutex_unlock(&_sl_lock);
}

/* setlogmask -- set the log mask level */
int
setlogmask(int mask)
{
	int oldmask;

	if (mask == 0) return _sl_mask;

	pthread_mutex_lock(&_sl_lock);

	_sl_mask = mask;
	oldmask = asl_set_filter(_sl_asl, mask);
	if (_sl_opts & LOG_PERROR) asl_set_output_file_filter(_sl_asl, STDERR_FILENO, mask);

	pthread_mutex_unlock(&_sl_lock);

	return oldmask;
}

#endif /* !BUILDING_VARIANT */
