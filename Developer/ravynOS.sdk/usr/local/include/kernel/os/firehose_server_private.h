/*
 * Copyright (c) 2015 Apple Inc. All rights reserved.
 *
 * @APPLE_APACHE_LICENSE_HEADER_START@
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * @APPLE_APACHE_LICENSE_HEADER_END@
 */

#ifndef __FIREHOSE_SERVER_PRIVATE__
#define __FIREHOSE_SERVER_PRIVATE__

#include <os/base.h>
#include <dispatch/dispatch.h>
#include "firehose_buffer_private.h"

#if OS_FIREHOSE_SPI
/*!
 * @group Firehose SPI
 * SPI intended for logd only
 */

#pragma mark - Firehose Client

/*!
 * @typedef firehose_client_t
 *
 * @abstract
 * Represents a firehose client.
 *
 * @discussion
 * Firehose client objects are os_object_t's, and it's legal to retain/release
 * them with os_retain / os_release.
 */
OS_OBJECT_DECL_CLASS(firehose_client);

/*!
 * @typedef firehose_event_t
 *
 * @const FIREHOSE_EVENT_NONE
 * Never passed to callbacks, meaningful for
 * firehose_client_metadata_stream_peek.
 *
 * @const FIREHOSE_EVENT_CLIENT_CONNECTED
 * A new client has connected
 *
 * This is the first event delivered, and no event is delivered until
 * the handler of that event returns
 *
 * The `page` argument is really a firehose_client_connected_info_t.  The
 * `fc_pos` argument is not meaningful.
 *
 * @const FIREHOSE_EVENT_CLIENT_DIED
 * The specified client is gone and will not flush new buffers
 *
 * This is the last event delivered, it is never called before all other
 * event handlers have returned. This event is generated even when a
 * FIREHOSE_EVENT_CLIENT_CORRUPTED event has been generated.
 *
 * @const FIREHOSE_EVENT_IO_BUFFER_RECEIVED
 * A new buffer needs to be pushed; `page` is set to that buffer, and `fc_pos`
 * to its chunk position header.
 *
 * This event can be sent concurrently wrt FIREHOSE_EVENT_MEM_BUFFER_RECEIVED
 * events.
 *
 * @const FIREHOSE_EVENT_MEM_BUFFER_RECEIVED
 * A new buffer needs to be pushed; `page` is set to that buffer, and `fc_pos`
 * to its chunk position header.
 *
 * This event can be sent concurrently wrt FIREHOSE_EVENT_IO_BUFFER_RECEIVED
 * events.
 *
 * @const FIREHOSE_EVENT_CLIENT_CORRUPTED
 * This event is received when a client is found being corrupted.
 * `page` is set to the buffer header page, and `fc_pos` is not meaningful. When
 * this event is received, logs have likely been lost for this client.
 *
 * This buffer isn't really a proper firehose buffer page, but its content may
 * be useful for debugging purposes.
 *
 * @const FIREHOSE_EVENT_CLIENT_FINALIZE
 * This event is received when a firehose client structure is about to be
 * destroyed. Only firehose_client_get_context() can ever be called with
 * the passed firehose client. The `page` argument is NULL for this event, and
 * the `fc_pos` argument is not meaningful.
 *
 * The event is sent from the context that is dropping the last refcount
 * of the client.
 */
OS_ENUM(firehose_event, unsigned long,
	FIREHOSE_EVENT_NONE = 0,
	FIREHOSE_EVENT_CLIENT_CONNECTED,
	FIREHOSE_EVENT_CLIENT_DIED,
	FIREHOSE_EVENT_IO_BUFFER_RECEIVED,
	FIREHOSE_EVENT_MEM_BUFFER_RECEIVED,
	FIREHOSE_EVENT_CLIENT_CORRUPTED,
	FIREHOSE_EVENT_CLIENT_FINALIZE,
);

#define FIREHOSE_CLIENT_CONNECTED_INFO_VERSION  1

/*!
 * @typedef firehose_client_connected_info
 *
 * @abstract
 * Type of the data passed to CLIENT_CONNECTED events.
 */
typedef struct firehose_client_connected_info_s {
	unsigned long fcci_version;
	// version 1
	const void *fcci_data;
	size_t fcci_size;
} *firehose_client_connected_info_t;

/*!
 * @function firehose_client_get_unique_pid
 *
 * @abstract
 * Returns the unique pid of the specified firehose client
 *
 * @param client
 * The specified client.
 *
 * @param pid
 * The pid for this client.
 *
 * @returns
 * The unique pid of the specified client.
 */
OS_NOTHROW OS_NONNULL1
uint64_t
firehose_client_get_unique_pid(firehose_client_t client, pid_t *pid);

/*!
 * @function firehose_client_get_pid_version
 *
 * @abstract
 * Returns the pid version for that client.
 *
 * @param client
 * The specified client.
 */
OS_NOTHROW OS_NONNULL1
int
firehose_client_get_pid_version(firehose_client_t client);

/*!
 * @function firehose_client_get_euid
 *
 * @abstract
 * Returns the EUID for that client as discovered at connect time.
 *
 * @param client
 * The specified client.
 */
OS_NOTHROW OS_NONNULL1
uid_t
firehose_client_get_euid(firehose_client_t client);

/*!
 * @function firehose_client_get_metadata_buffer
 *
 * @abstract
 * Returns the metadata buffer for the specified firehose client
 *
 * @param client
 * The specified client.
 *
 * @param size
 * The size of the metadata buffer.
 *
 * @returns
 * The pointer to the buffer.
 */
OS_NOTHROW OS_NONNULL_ALL
void *
firehose_client_get_metadata_buffer(firehose_client_t client, size_t *size);

/*!
 * @function firehose_client_get_context
 *
 * @abstract
 * Gets the context for the specified client.
 *
 * @param client
 * The specified client.
 *
 * @returns
 * The context set for the client with firehose_client_set_context
 */
OS_NOTHROW OS_NONNULL1
void *
firehose_client_get_context(firehose_client_t client);

/*!
 * @function firehose_client_set_strings_cached
 *
 * @abstract
 * Marks a given client as having strings cached already.
 *
 * @param client
 * The specified client.
 */
OS_NOTHROW OS_NONNULL1
void
firehose_client_set_strings_cached(firehose_client_t client);

/*!
 * @function firehose_client_set_context
 *
 * @abstract
 * Sets the context for the specified client.
 *
 * @discussion
 * Setting the context exchanges the context pointer, but the client must
 * ensure proper synchronization with possible getters.
 *
 * The lifetime of the context is under the control of the API user,
 * it is suggested to destroy the context when the CLIENT_DIED event is
 * received.
 *
 * @param client
 * The specified client.
 *
 * @param ctxt
 * The new context to set.
 *
 * @returns
 * The previous context set for the client.
 */
OS_NOTHROW OS_NONNULL1
void *
firehose_client_set_context(firehose_client_t client, void *ctxt);

/*!
 * @function firehose_client_initiate_quarantine
 *
 * @abstract
 * Starts the procedure to move the given client to the high volume quarantine
 *
 * @discussion
 * When the client is in the high volume quarantine, their firehose chunks
 * have the fcp_quarantined bit set to 1.
 *
 * @param client
 * The specified client.
 */
OS_NOTHROW OS_NONNULL1
void
firehose_client_initiate_quarantine(firehose_client_t client);

/*!
 * @function firehose_client_metadata_stream_peek
 *
 * @abstract
 * Peek at the metadata stream in flight buffers for a given client
 *
 * @discussion
 * This function should never be called from the context of a snapshot
 * handler.
 *
 * @param client
 * The specified client
 *
 * @param context
 * If this function is called synchronously from the handler passed to
 * firehose_server_init, then `context` should be the event being processed.
 * Else pass FIREHOSE_EVENT_NONE.
 *
 * @param peek_should_start
 * Handler that is called prior to peeking to solve the race of metadata
 * buffers not beeing processed yet at first lookup time, and being processed
 * before the peek enumeration starts.
 *
 * If the handler returns false, then the enumeration doesn't start.
 * If the race cannot happen, pass NULL.
 *
 * @param peek
 * Handler that will receive all the live metadata buffers for this process.
 * If the handler returns false, the enumeration is interrupted.
 */
OS_NOTHROW OS_NONNULL1 OS_NONNULL4
void
firehose_client_metadata_stream_peek(firehose_client_t client,
		firehose_event_t context, OS_NOESCAPE bool (^peek_should_start)(void),
		OS_NOESCAPE bool (^peek)(firehose_chunk_t fbc));

#pragma mark - Firehose Server

/*!
 * @typedef firehose_handler_t
 *
 * @abstract
 * Type of the handler block for firehose_server_init()
 */
typedef void (^firehose_handler_t)(firehose_client_t client,
		firehose_event_t event, firehose_chunk_t page,
		firehose_chunk_pos_u fc_pos);

/*!
 * @function firehose_server_init
 *
 * @abstract
 * Initializes the firehose MiG server
 *
 * @discussion
 * Initializes the firehose MiG server by boostrap registering the services
 * and creating dispatch_sources for the same.
 */
OS_NOTHROW
void
firehose_server_init(mach_port_t firehose_comm_port,
		firehose_handler_t handler);

/*!
 * @function firehose_server_assert_spi_version
 *
 * @abstract
 * Checks that libdispatch and firehose components all match
 *
 * @discussion
 * Will assert that all the components have the same SPI versions
 */
OS_NOTHROW
void
firehose_server_assert_spi_version(uint32_t spi_version);

/*!
 * @function firehose_server_has_ever_flushed_pages
 *
 * @abstract
 * Checks whether the firehose server has ever flushed any pages this boot.
 *
 * @discussion
 * Must be called after firehose_server_init() and before calling
 * firehose_server_resume().
 */
OS_NOTHROW
bool
firehose_server_has_ever_flushed_pages(void);

/*!
 * @function firehose_server_resume
 *
 * @abstract
 * Allows firehose events to flow
 *
 * @discussion
 * Must be called after firehose_server_init()
 */
OS_NOTHROW
void
firehose_server_resume(void);

/*!
 * @function firehose_server_cancel
 *
 * @abstract
 * Cancels the server, disconnects all clients, and prevents new connections.
 */
OS_NOTHROW
void
firehose_server_cancel(void);

/*!
 * @function firehose_server_set_logging_prefs
 *
 * @abstract
 * Publishes a new preferences buffer.
 *
 * @description
 * The server will take ownership of this buffer and will
 * call munmap() on the previous one that was stored.
 */
OS_NOTHROW
void
firehose_server_set_logging_prefs(void *pointer, size_t length,
		os_block_t block);

/*!
 * @typedef firehose_server_queue_t
 *
 * @abstract
 * Values to pass to firehose_server_get_queue()
 */
OS_ENUM(firehose_server_queue, unsigned long,
	FIREHOSE_SERVER_QUEUE_UNKNOWN,
	FIREHOSE_SERVER_QUEUE_IO,
	FIREHOSE_SERVER_QUEUE_MEMORY,
	FIREHOSE_SERVER_QUEUE_IO_WL,
);

/*!
 * @function firehose_server_copy_queue
 *
 * @abstract
 * Returns internal queues to the firehose server subsystem.
 */
OS_NOTHROW OS_OBJECT_RETURNS_RETAINED
dispatch_queue_t
firehose_server_copy_queue(firehose_server_queue_t which);

/*!
 * @function firehose_server_quarantined_suspend
 *
 * @abstract
 * Suspends processing of quarantined clients until
 * firehose_server_quarantined_resume() is called for the same queue.
 *
 * @discussion
 * Suspending processing of quarantined clients causes firehose_snapshot()
 * to block until the processing is enabled again.
 *
 * However if this is used to pace the processing, it is a good idea to disable
 * this pacing until the snapshot has completed.
 *
 * Similarly, quarantine suspension must be off during shutdown.
 */
OS_NOTHROW
void
firehose_server_quarantined_suspend(firehose_server_queue_t q);

/*!
 * @function firehose_server_quarantined_resume
 *
 * @abstract
 * Resumes processing of quarantined clients.
 */
OS_NOTHROW
void
firehose_server_quarantined_resume(firehose_server_queue_t q);

#pragma mark - Firehose Snapshot

/*!
 * @typedef firehose_snapshot_event
 */
OS_ENUM(firehose_snapshot_event, unsigned long,
	FIREHOSE_SNAPSHOT_EVENT_IO_START = 1,
	FIREHOSE_SNAPSHOT_EVENT_MEM_START,
	FIREHOSE_SNAPSHOT_EVENT_IO_BUFFER,
	FIREHOSE_SNAPSHOT_EVENT_MEM_BUFFER,
	FIREHOSE_SNAPSHOT_EVENT_COMPLETE,
);

/*!
 * @typedef firehose_snapshot_handler_t
 *
 * @abstract
 * Type of the handler block for firehose_snapshot
 */
typedef void (^firehose_snapshot_handler_t)(firehose_client_t client,
		firehose_snapshot_event_t event, firehose_chunk_t page,
		firehose_chunk_pos_u fc_pos);

/*!
 * @function firehose_snapshot
 *
 * @abstract
 * Gather a snapshot for the current firehose state.
 *
 * @discussion
 * This function can be called several times, in which case snapshots are taken
 * one after the other. If coalescing is desired, it has to be built around this
 * call.
 */
OS_NOTHROW
void
firehose_snapshot(firehose_snapshot_handler_t handler);

#endif // OS_FIREHOSE_SPI

#endif // __FIREHOSE_SERVER_PRIVATE__
