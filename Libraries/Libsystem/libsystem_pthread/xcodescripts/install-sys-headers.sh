#
# Copyright (c) 2013-2014 Apple Inc. All rights reserved.
#
# @APPLE_LICENSE_HEADER_START@
#
# This file contains Original Code and/or Modifications of Original Code
# as defined in and that are subject to the Apple Public Source License
# Version 2.0 (the 'License'). You may not use this file except in
# compliance with the License. Please obtain a copy of the License at
# http://www.opensource.apple.com/apsl/ and read it before using this
# file.
#
# The Original Code and all software distributed under the License are
# distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
# EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
# INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
# Please see the License for the specific language governing rights and
# limitations under the License.
#
# @APPLE_LICENSE_HEADER_END@
#

set -e

if [ "$ACTION" = build ]; then exit 0; fi

DSTROOT="${DSTROOT}/${SDK_INSTALL_HEADERS_ROOT}"

DESTDIR="$DSTROOT/usr/include/sys"
mkdir -p "$DESTDIR"
for X in \
	qos.h \
	; do
	cp "sys/$X" "$DESTDIR"
done

DESTDIR="$DSTROOT/usr/local/include/sys"
mkdir -p "$DESTDIR"
for X in \
	qos_private.h \
	; do
	cp "sys/$X" "$DESTDIR"
done

DESTDIR="$DSTROOT/usr/include/sys/_pthread"
mkdir -p "$DESTDIR"
for X in \
	_pthread_attr_t.h \
	_pthread_cond_t.h \
	_pthread_condattr_t.h \
	_pthread_key_t.h \
	_pthread_mutex_t.h \
	_pthread_mutexattr_t.h \
	_pthread_once_t.h \
	_pthread_rwlock_t.h \
	_pthread_rwlockattr_t.h \
	_pthread_t.h \
	_pthread_types.h \
	; do
	cp "sys/_pthread/$X" "$DESTDIR"
done

