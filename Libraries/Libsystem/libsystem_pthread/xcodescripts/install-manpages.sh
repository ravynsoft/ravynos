#
# Copyright (c) 2012-2013 Apple Inc. All rights reserved.
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

if [ "$ACTION" = installhdrs ]; then exit 0; fi
if [ "${RC_ProjectName%_Sim}" != "${RC_ProjectName}" ]; then exit 0; fi
if [ "${DRIVERKIT}" = 1 ]; then exit 0; fi

set -x
set -e

mkdir -p "$DSTROOT"/usr/share/man/man2 || true
mkdir -p "$DSTROOT"/usr/share/man/man3 || true
mkdir -p "$DSTROOT"/usr/local/share/man/man2 || true
mkdir -p "$DSTROOT"/usr/local/share/man/man3 || true

# Copy man pages
cd "$SRCROOT"/man

BASE_PAGES="pthread_kill.2 \
	pthread_sigmask.2"

cp $BASE_PAGES "$DSTROOT"/usr/share/man/man2

BASE_PAGES="pthread.3 \
	pthread_atfork.3 \
	pthread_attr.3 \
	pthread_attr_init_destroy.3 \
	pthread_cancel.3 \
	pthread_cleanup_pop.3 \
	pthread_cleanup_push.3 \
	pthread_cond_broadcast.3 \
	pthread_cond_destroy.3 \
	pthread_cond_init.3 \
	pthread_cond_signal.3 \
	pthread_cond_timedwait.3 \
	pthread_cond_wait.3 \
	pthread_condattr.3 \
	pthread_create.3 \
	pthread_detach.3 \
	pthread_equal.3 \
	pthread_exit.3 \
	pthread_getschedparam.3 \
	pthread_getspecific.3 \
	pthread_join.3 \
	pthread_key_create.3 \
	pthread_key_delete.3 \
	pthread_main_np.3 \
	pthread_mutex_destroy.3 \
	pthread_mutex_init.3 \
	pthread_mutex_lock.3 \
	pthread_mutex_trylock.3 \
	pthread_mutex_unlock.3 \
	pthread_mutexattr.3 \
	pthread_once.3 \
	pthread_rwlock_destroy.3 \
	pthread_rwlock_init.3 \
	pthread_rwlock_rdlock.3 \
	pthread_rwlock_unlock.3 \
	pthread_rwlock_wrlock.3 \
	pthread_rwlockattr_destroy.3 \
	pthread_rwlockattr_getpshared.3 \
	pthread_rwlockattr_init.3 \
	pthread_rwlockattr_setpshared.3 \
	pthread_self.3 \
	pthread_setcancelstate.3 \
	pthread_setname_np.3 \
	pthread_setspecific.3 \
	pthread_threadid_np.3 \
	pthread_yield_np.3"

cp $BASE_PAGES "$DSTROOT"/usr/share/man/man3

for ATTR in \
	detachstate \
	inheritsched \
	schedparam \
	schedpolicy \
	scope \
	stackaddr \
	stacksize \
	; do
	cp pthread_attr_set_get$ATTR.3 "$DSTROOT"/usr/share/man/man3/pthread_attr_set$ATTR.3
	cp pthread_attr_set_get$ATTR.3 "$DSTROOT"/usr/share/man/man3/pthread_attr_get$ATTR.3
done

# Make hard links

cd "$DSTROOT"/usr/share/man/man3

chown ${INSTALL_OWNER}:${INSTALL_GROUP} $BASE_PAGES
chmod $INSTALL_MODE_FLAG $BASE_PAGES

ln -fh pthread_getschedparam.3 pthread_setschedparam.3
ln -fh pthread_rwlock_rdlock.3 pthread_rwlock_tryrdlock.3
ln -fh pthread_rwlock_wrlock.3 pthread_rwlock_trywrlock.3

for M in \
	pthread_attr_destroy.3 \
	pthread_attr_init.3 \
	pthread_attr_setstack.3 \
	pthread_attr_getstack.3 \
	pthread_attr_setguardsize.3 \
	pthread_attr_getguardsize.3 \
	; do
	ln -fh pthread_attr.3 $M
done

for M in \
	pthread_mutexattr_destroy.3 \
	pthread_mutexattr_getprioceiling.3 \
	pthread_mutexattr_getprotocol.3 \
	pthread_mutexattr_gettype.3 \
	pthread_mutexattr_init.3 \
	pthread_mutexattr_setprioceiling.3 \
	pthread_mutexattr_setprotocol.3 \
	pthread_mutexattr_settype.3 \
	; do
	ln -fh pthread_mutexattr.3 $M
done

for M in \
	pthread_condattr_destroy.3 \
	pthread_condattr_init.3 \
	; do
	ln -fh pthread_condattr.3 $M
done

for M in \
	pthread_setcanceltype.3 \
	pthread_testcancel.3 \
	; do
	ln -fh pthread_setcancelstate.3 $M
done
