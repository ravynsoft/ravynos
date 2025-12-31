#ifndef __ASL_STRING_H__
#define __ASL_STRING_H__

/*
 * Copyright (c) 2007-2013 Apple Inc.  All rights reserved.
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

#define ASL_ENCODE_MASK		0x0000000f
#define ASL_STRING_VM		0x80000000
#define ASL_STRING_LEN		0x40000000
#define ASL_STRING_MIG		0xc0000002

typedef struct
{
	uint32_t asl_type;	//ASL OBJECT HEADER
	int32_t refcount;	//ASL OBJECT HEADER
	uint32_t encoding;
	size_t delta;
	size_t bufsize;
	size_t cursor;
	char *buf;
} asl_string_t;

asl_string_t *asl_string_new(uint32_t encoding) __API_DEPRECATED("os_log(3) has replaced asl(3)", macosx(10.8,10.12), ios(5.1,10.0), watchos(2.0,3.0), tvos(9.0,10.0));
asl_string_t *asl_string_retain(asl_string_t *str) __API_DEPRECATED("os_log(3) has replaced asl(3)", macosx(10.8,10.12), ios(5.1,10.0), watchos(2.0,3.0), tvos(9.0,10.0));
void asl_string_release(asl_string_t *str) __API_DEPRECATED("os_log(3) has replaced asl(3)", macosx(10.8,10.12), ios(5.1,10.0), watchos(2.0,3.0), tvos(9.0,10.0));
char *asl_string_release_return_bytes(asl_string_t *str) __API_DEPRECATED("os_log(3) has replaced asl(3)", macosx(10.8,10.12), ios(5.1,10.0), watchos(2.0,3.0), tvos(9.0,10.0));
char *asl_string_bytes(asl_string_t *str) __API_DEPRECATED("os_log(3) has replaced asl(3)", macosx(10.8,10.12), ios(5.1,10.0), watchos(2.0,3.0), tvos(9.0,10.0));
size_t asl_string_length(asl_string_t *str) __API_DEPRECATED("os_log(3) has replaced asl(3)", macosx(10.8,10.12), ios(5.1,10.0), watchos(2.0,3.0), tvos(9.0,10.0));
size_t asl_string_allocated_size(asl_string_t *str) __API_DEPRECATED("os_log(3) has replaced asl(3)", macosx(10.8,10.12), ios(5.1,10.0), watchos(2.0,3.0), tvos(9.0,10.0));
asl_string_t *asl_string_append(asl_string_t *str, const char *app) __API_DEPRECATED("os_log(3) has replaced asl(3)", macosx(10.8,10.12), ios(5.1,10.0), watchos(2.0,3.0), tvos(9.0,10.0));
asl_string_t *asl_string_append_asl_key(asl_string_t *str, const char *app) __API_DEPRECATED("os_log(3) has replaced asl(3)", macosx(10.8,10.12), ios(5.1,10.0), watchos(2.0,3.0), tvos(9.0,10.0));
asl_string_t *asl_string_append_op(asl_string_t *str, uint32_t op) __API_DEPRECATED("os_log(3) has replaced asl(3)", macosx(10.8,10.12), ios(5.1,10.0), watchos(2.0,3.0), tvos(9.0,10.0));
asl_string_t *asl_string_append_no_encoding(asl_string_t *str, const char *app) __API_DEPRECATED("os_log(3) has replaced asl(3)", macosx(10.8,10.12), ios(5.1,10.0), watchos(2.0,3.0), tvos(9.0,10.0));
asl_string_t *asl_string_append_char_no_encoding(asl_string_t *str, const char c) __API_DEPRECATED("os_log(3) has replaced asl(3)", macosx(10.8,10.12), ios(5.1,10.0), watchos(2.0,3.0), tvos(9.0,10.0));
asl_string_t *asl_string_append_xml_tag(asl_string_t *str, const char *tag, const char *s) __API_DEPRECATED("os_log(3) has replaced asl(3)", macosx(10.8,10.12), ios(5.1,10.0), watchos(2.0,3.0), tvos(9.0,10.0));

#endif /* __ASL_STRING_H__ */
