#
#
#

dist_man3_MANS=					\
	dispatch.3					\
	dispatch_after.3			\
	dispatch_api.3				\
	dispatch_apply.3			\
	dispatch_async.3			\
	dispatch_data_create.3		\
	dispatch_group_create.3		\
	dispatch_io_create.3		\
	dispatch_io_read.3			\
	dispatch_object.3			\
	dispatch_once.3				\
	dispatch_queue_create.3		\
	dispatch_read.3				\
	dispatch_semaphore_create.3	\
	dispatch_source_create.3	\
	dispatch_time.3

EXTRA_DIST=						\
	dispatch_benchmark.3

#
# Install man page hardlinks.  Is there a better way to do this in automake?
#

LN=ln

install-data-hook:
	cd $(DESTDIR)$(mandir)/man3 &&	\
		$(LN) -f dispatch_after.3 dispatch_after_f.3 &&	\
		$(LN) -f dispatch_apply.3 dispatch_apply_f.3 &&	\
		$(LN) -f dispatch_async.3 dispatch_sync.3 &&	\
		$(LN) -f dispatch_async.3 dispatch_async_f.3 &&	\
		$(LN) -f dispatch_async.3 dispatch_sync_f.3 &&	\
		$(LN) -f dispatch_group_create.3 dispatch_group_enter.3 &&	\
		$(LN) -f dispatch_group_create.3 dispatch_group_leave.3 &&	\
		$(LN) -f dispatch_group_create.3 dispatch_group_wait.3 &&	\
		$(LN) -f dispatch_group_create.3 dispatch_group_notify.3 &&	\
		$(LN) -f dispatch_group_create.3 dispatch_group_notify_f.3 &&	\
		$(LN) -f dispatch_group_create.3 dispatch_group_async.3 &&	\
		$(LN) -f dispatch_group_create.3 dispatch_group_async_f.3 &&	\
		$(LN) -f dispatch_object.3 dispatch_retain.3 &&	\
		$(LN) -f dispatch_object.3 dispatch_release.3 &&	\
		$(LN) -f dispatch_object.3 dispatch_suspend.3 &&	\
		$(LN) -f dispatch_object.3 dispatch_resume.3 &&	\
		$(LN) -f dispatch_object.3 dispatch_get_context.3 &&	\
		$(LN) -f dispatch_object.3 dispatch_set_context.3 &&	\
		$(LN) -f dispatch_object.3 dispatch_set_finalizer_f.3 &&	\
		$(LN) -f dispatch_once.3 dispatch_once_f.3 &&	\
		$(LN) -f dispatch_queue_create.3 dispatch_queue_get_label.3 &&	\
		$(LN) -f dispatch_queue_create.3 dispatch_get_current_queue.3 &&	\
		$(LN) -f dispatch_queue_create.3 dispatch_get_global_queue.3 &&	\
		$(LN) -f dispatch_queue_create.3 dispatch_get_main_queue.3 &&	\
		$(LN) -f dispatch_queue_create.3 dispatch_main.3 &&	\
		$(LN) -f dispatch_queue_create.3 dispatch_set_target_queue.3 &&	\
		$(LN) -f dispatch_semaphore_create.3 dispatch_semaphore_signal.3 &&	\
		$(LN) -f dispatch_semaphore_create.3 dispatch_semaphore_wait.3 &&	\
		$(LN) -f dispatch_source_create.3 dispatch_source_set_event_handler.3 &&	\
		$(LN) -f dispatch_source_create.3 dispatch_source_set_event_handler_f.3 &&	\
		$(LN) -f dispatch_source_create.3 dispatch_source_set_registration_handler.3 &&	\
		$(LN) -f dispatch_source_create.3 dispatch_source_set_registration_handler_f.3 &&	\
		$(LN) -f dispatch_source_create.3 dispatch_source_set_cancel_handler.3 &&	\
		$(LN) -f dispatch_source_create.3 dispatch_source_set_cancel_handler_f.3 &&	\
		$(LN) -f dispatch_source_create.3 dispatch_source_cancel.3 &&	\
		$(LN) -f dispatch_source_create.3 dispatch_source_testcancel.3 &&	\
		$(LN) -f dispatch_source_create.3 dispatch_source_get_handle.3 &&	\
		$(LN) -f dispatch_source_create.3 dispatch_source_get_mask.3 &&	\
		$(LN) -f dispatch_source_create.3 dispatch_source_get_data.3 &&	\
		$(LN) -f dispatch_source_create.3 dispatch_source_merge_data.3 &&	\
		$(LN) -f dispatch_source_create.3 dispatch_source_set_timer.3 &&	\
		$(LN) -f dispatch_time.3 dispatch_walltime.3 &&	\
		$(LN) -f dispatch_data_create.3 dispatch_data_create_concat.3 &&	\
		$(LN) -f dispatch_data_create.3 dispatch_data_create_subrange.3 &&	\
		$(LN) -f dispatch_data_create.3 dispatch_data_create_map.3 &&	\
		$(LN) -f dispatch_data_create.3 dispatch_data_apply.3 &&	\
		$(LN) -f dispatch_data_create.3 dispatch_data_copy_region.3 &&	\
		$(LN) -f dispatch_data_create.3 dispatch_data_get_size.3 &&	\
		$(LN) -f dispatch_data_create.3 dispatch_data_empty.3 &&	\
		$(LN) -f dispatch_io_create.3 dispatch_io_create_with_path.3 &&	\
		$(LN) -f dispatch_io_create.3 dispatch_io_set_high_water.3 &&	\
		$(LN) -f dispatch_io_create.3 dispatch_io_set_low_water.3 &&	\
		$(LN) -f dispatch_io_create.3 dispatch_io_set_interval.3 &&	\
		$(LN) -f dispatch_io_create.3 dispatch_io_close.3 &&	\
		$(LN) -f dispatch_io_create.3 dispatch_io_barrier.3 &&	\
		$(LN) -f dispatch_io_read.3 dispatch_io_write.3 &&	\
		$(LN) -f dispatch_read.3 dispatch_write.3

uninstall-hook:
	cd $(DESTDIR)$(mandir)/man3 &&	\
		rm -f dispatch_after_f.3	\
			dispatch_apply_f.3	\
			dispatch_sync.3	\
			dispatch_async_f.3	\
			dispatch_sync_f.3	\
			dispatch_group_enter.3	\
			dispatch_group_leave.3	\
			dispatch_group_wait.3	\
			dispatch_group_notify.3	\
			dispatch_group_notify_f.3	\
			dispatch_group_async.3	\
			dispatch_group_async_f.3	\
			dispatch_retain.3	\
			dispatch_release.3	\
			dispatch_suspend.3	\
			dispatch_resume.3	\
			dispatch_get_context.3	\
			dispatch_set_context.3	\
			dispatch_set_finalizer_f.3	\
			dispatch_once_f.3	\
			dispatch_queue_get_label.3	\
			dispatch_get_current_queue.3	\
			dispatch_get_global_queue.3	\
			dispatch_get_main_queue.3	\
			dispatch_main.3	\
			dispatch_set_target_queue.3	\
			dispatch_semaphore_signal.3	\
			dispatch_semaphore_wait.3	\
			dispatch_source_set_event_handler.3	\
			dispatch_source_set_event_handler_f.3	\
			dispatch_source_set_registration_handler.3	\
			dispatch_source_set_registration_handler_f.3	\
			dispatch_source_set_cancel_handler.3	\
			dispatch_source_set_cancel_handler_f.3	\
			dispatch_source_cancel.3	\
			dispatch_source_testcancel.3	\
			dispatch_source_get_handle.3	\
			dispatch_source_get_mask.3	\
			dispatch_source_get_data.3	\
			dispatch_source_merge_data.3	\
			dispatch_source_set_timer.3	\
			dispatch_walltime.3	\
			dispatch_data_create_concat.3	\
			dispatch_data_create_subrange.3	\
			dispatch_data_create_map.3	\
			dispatch_data_apply.3	\
			dispatch_data_copy_region.3	\
			dispatch_data_get_size.3	\
			dispatch_data_empty.3	\
			dispatch_io_create_with_path.3	\
			dispatch_io_set_high_water.3	\
			dispatch_io_set_low_water.3	\
			dispatch_io_set_interval.3	\
			dispatch_io_close.3	\
			dispatch_io_barrier.3	\
			dispatch_io_write.3	\
			dispatch_write.3
