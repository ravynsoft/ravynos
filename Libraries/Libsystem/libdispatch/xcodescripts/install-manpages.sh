#!/bin/bash -e
#
# Copyright (c) 2010-2012 Apple Inc. All rights reserved.
#
# @APPLE_APACHE_LICENSE_HEADER_START@
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# @APPLE_APACHE_LICENSE_HEADER_END@
#

if [ "$ACTION" = installhdrs ]; then exit 0; fi
if [ "${RC_ProjectName%_Sim}" != "${RC_ProjectName}" ]; then exit 0; fi

mkdir -p "$DSTROOT"/usr/share/man/man3 || true
mkdir -p "$DSTROOT"/usr/local/share/man/man3 || true

# Copy man pages
cd "$SRCROOT"/man
BASE_PAGES="dispatch.3 dispatch_after.3 dispatch_api.3 dispatch_apply.3 \
		dispatch_async.3 dispatch_group_create.3 dispatch_object.3 \
		dispatch_once.3 dispatch_queue_create.3 dispatch_semaphore_create.3 \
		dispatch_source_create.3 dispatch_time.3 dispatch_data_create.3 \
		dispatch_io_create.3 dispatch_io_read.3 dispatch_read.3"

PRIVATE_PAGES="dispatch_benchmark.3"

cp ${BASE_PAGES} "$DSTROOT"/usr/share/man/man3
cp ${PRIVATE_PAGES} "$DSTROOT"/usr/local/share/man/man3

# Make hard links (lots of hard links)

cd "$DSTROOT"/usr/local/share/man/man3
ln -f dispatch_benchmark.3 dispatch_benchmark_f.3
chown ${INSTALL_OWNER}:${INSTALL_GROUP} $PRIVATE_PAGES
chmod $INSTALL_MODE_FLAG $PRIVATE_PAGES

cd $DSTROOT/usr/share/man/man3

chown ${INSTALL_OWNER}:${INSTALL_GROUP} $BASE_PAGES
chmod $INSTALL_MODE_FLAG $BASE_PAGES

ln -f dispatch_after.3 dispatch_after_f.3
ln -f dispatch_apply.3 dispatch_apply_f.3
ln -f dispatch_once.3 dispatch_once_f.3

for m in dispatch_async_f dispatch_sync dispatch_sync_f; do
	ln -f dispatch_async.3 ${m}.3
done

for m in dispatch_group_enter dispatch_group_leave dispatch_group_wait \
		dispatch_group_async dispatch_group_async_f dispatch_group_notify \
		dispatch_group_notify_f; do
	ln -f dispatch_group_create.3 ${m}.3
done

for m in dispatch_retain dispatch_release dispatch_suspend dispatch_resume dispatch_activate \
		dispatch_get_context dispatch_set_context dispatch_set_finalizer_f; do
	ln -f dispatch_object.3 ${m}.3
done

for m in dispatch_semaphore_signal dispatch_semaphore_wait; do
	ln -f dispatch_semaphore_create.3 ${m}.3
done

for m in dispatch_get_current_queue dispatch_main dispatch_get_main_queue \
		dispatch_get_global_queue dispatch_queue_get_label \
		dispatch_set_target_queue; do
	ln -f dispatch_queue_create.3 ${m}.3
done

for m in dispatch_source_set_event_handler dispatch_source_set_event_handler_f \
		dispatch_source_set_registration_handler dispatch_source_set_registration_handler_f \
		dispatch_source_set_cancel_handler dispatch_source_set_cancel_handler_f \
		dispatch_source_cancel dispatch_source_testcancel \
		dispatch_source_get_handle dispatch_source_get_mask \
		dispatch_source_get_data dispatch_source_merge_data \
		dispatch_source_set_timer; do
	ln -f dispatch_source_create.3 ${m}.3
done

ln -f dispatch_time.3 dispatch_walltime.3

for m in dispatch_data_create_concat dispatch_data_create_subrange \
		dispatch_data_create_map dispatch_data_apply \
		dispatch_data_copy_region dispatch_data_get_size; do
	ln -f dispatch_data_create.3 ${m}.3
done

for m in dispatch_io_create_with_path dispatch_io_set_high_water \
		dispatch_io_set_low_water dispatch_io_set_interval \
		dispatch_io_close dispatch_io_barrier; do
	ln -f dispatch_io_create.3 ${m}.3
done

ln -f dispatch_io_read.3 dispatch_io_write.3

ln -f dispatch_read.3 dispatch_write.3
