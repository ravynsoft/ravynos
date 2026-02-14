#
# Copyright (c) 2012 Apple Inc. All rights reserved.
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

if [ "$ACTION" = build ]; then exit 0; fi

#
# Symlink old header locations.
#

DSTROOT="${DSTROOT}/${SDK_INSTALL_HEADERS_ROOT}"

ln -sf "pthread/pthread.h" "$DSTROOT/usr/include/pthread.h"
ln -sf "pthread/pthread_impl.h" "$DSTROOT/usr/include/pthread_impl.h"
ln -sf "pthread/pthread_spis.h" "$DSTROOT/usr/include/pthread_spis.h"
ln -sf "pthread/sched.h" "$DSTROOT/usr/include/sched.h"

ln -sf "pthread/posix_sched.h" "$DSTROOT/usr/local/include/posix_sched.h"
ln -sf "pthread/spinlock_private.h" "$DSTROOT/usr/local/include/pthread_spinlock.h"
ln -sf "pthread/workqueue_private.h" "$DSTROOT/usr/local/include/pthread_workqueue.h"
mkdir -p "$DSTROOT/System/Library/Frameworks/System.framework/Versions/B/PrivateHeaders/"
ln -sf "../../../../../../../usr/local/include/pthread/tsd_private.h" \
	"$DSTROOT/System/Library/Frameworks/System.framework/Versions/B/PrivateHeaders/pthread_machdep.h"
