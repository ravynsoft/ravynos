/*
 * Copyright (c) 2009 Apple Inc. All rights reserved.
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

#include <CoreFoundation/CoreFoundation.h>
#include <CoreFoundation/CFBridgingPriv.h>
#include <dispatch/dispatch.h>
#include <asl.h>
#include <assumes.h>

#include <opendirectory/odutils.h>

#include "CFODQuery.h"
#include "internal.h"
#include "transaction.h"

struct query_continue_data;

static void _query_continue_destroy(struct query_continue_data *ctx);

static void _query_schedule(void *context, CFRunLoopRef runLoop, CFStringRef runLoopMode);
static void _query_cancel(void *context, CFRunLoopRef runLoop, CFStringRef runLoopMode);
static void _query_perform(void *context);

enum {
	ASYNC_UNSET = 0,
	ASYNC_DISPATCH = 1,
	ASYNC_CF = 2,
};

struct __ODQuery {
	CFRuntimeBase _base;
	dispatch_queue_t _queue;

	// immutable state
	ODNodeRef _node;
	CFDictionaryRef _request;
	CFSetRef _attrset;

	// mutable state
	CFDataRef _uuid;
	od_transaction_t _transaction;
	CFMutableArrayRef _results;
	CFErrorRef _error;
	bool _synchronize;
	bool _complete;
	bool _oktosync;

	int _async_type;
	union {
		// queue-based async delivery
		struct {
			dispatch_queue_t queue;
			dispatch_source_t source;
		} dispatch;
		// runloop-based async delivery
		struct {
			CFMutableArrayRef runloops;
			CFRunLoopSourceRef source;
		} cf;
	} _async;

	// support for ODQueryCopyResults
	dispatch_queue_t _continue_queue;
	struct query_continue_data *_continue_data;

	// for ODQuerySetCallback
	ODQueryCallback _callback_function;
	void *_callback_context;

	// objc support
	void *_operation_queue;
	void *_delegate;
};

static CFTypeID __kODQueryTypeID = _kCFRuntimeNotATypeID;

static void
__ODQueryFinalize(CFTypeRef cf)
{
	ODQueryRef query = (ODQueryRef)cf;
	
	if (query->_transaction) {
		transaction_cancel(query->_transaction);
		transaction_release(query->_transaction);
		query->_transaction = NULL;
	}
	
	if (query->_continue_data) {
		_query_continue_destroy(query->_continue_data);
		query->_continue_data = NULL;
	}
	safe_dispatch_release(query->_continue_queue);

	safe_cfrelease_null(query->_uuid);
	safe_cfrelease_null(query->_node);
	safe_cfrelease_null(query->_request);
	safe_cfrelease_null(query->_attrset);

	safe_dispatch_release(query->_queue);
	safe_cfrelease_null(query->_results);
	safe_cfrelease_null(query->_error);

	if (query->_async_type == ASYNC_DISPATCH) {
		safe_dispatch_release(query->_async.dispatch.queue);
	} else if (query->_async_type == ASYNC_CF) {
		safe_cfrelease_null(query->_async.cf.runloops);
	}

	safe_cfrelease_null(query->_operation_queue);
	safe_cfrelease_null(query->_delegate);
}

static CFStringRef
__ODQueryCopyDebugDesc(CFTypeRef cf)
{
	ODQueryRef query = (ODQueryRef)cf;
	uuid_string_t uuidstr;
	uuid_unparse_upper(query->_uuid ? CFDataGetBytePtr(query->_uuid) : kODNullUUID, uuidstr);
	return CFStringCreateWithFormat(NULL, NULL, CFSTR("<ODQuery %p [node: %@] [uuid: %s]>"), query, ODNodeGetName(query->_node), uuidstr);
}

static const CFRuntimeClass __ODQueryClass = {
	0,								// version
	"ODQuery",						// className
	NULL,							// init
	NULL,							// copy
	__ODQueryFinalize,				// finalize
	NULL,							// equal
	NULL,							// hash
	NULL,							// copyFormattingDesc
	__ODQueryCopyDebugDesc,			// copyDebugDesc
	NULL,							// reclaim
#if CF_REFCOUNT_AVAILABLE
	NULL,							// refcount
#endif
};

CFTypeID
ODQueryGetTypeID(void)
{
	static dispatch_once_t once;

	dispatch_once(&once, ^ {
		__kODQueryTypeID = _CFRuntimeRegisterClass(&__ODQueryClass);
		if (__kODQueryTypeID != _kCFRuntimeNotATypeID) {
			_CFRuntimeBridgeClasses(__kODQueryTypeID, "NSODQuery");
		}
	});

	return __kODQueryTypeID;
}

#pragma mark Objective-C Support

void
_ODQuerySetDelegate(ODQueryRef query, void *delegate)
{
	safe_cfrelease_null(query->_delegate);
	query->_delegate = (void *)safe_cfretain(delegate);
}

void *
_ODQueryGetDelegate(ODQueryRef query)
{
	return query->_delegate;
}

void
_ODQuerySetOperationQueue(ODQueryRef query, void *operationQueue)
{
	safe_cfrelease_null(query->_operation_queue);
	query->_operation_queue = (void *)safe_cfretain(operationQueue);
}

void *
_ODQueryGetOperationQueue(ODQueryRef query)
{
	return query->_operation_queue;
}

#pragma mark Queue and Run Loop support

static void
_query_init_source(ODQueryRef query, CFRunLoopRef runLoop, CFStringRef runLoopMode)
{
	(void)osx_assumes(dispatch_get_current_queue() == query->_queue);

	(void)osx_assumes(query->_async_type == ASYNC_DISPATCH || query->_async_type == ASYNC_CF);

	switch (query->_async_type) {
	case ASYNC_DISPATCH:
		if (query->_async.dispatch.source == NULL) {
			query->_async.dispatch.source = dispatch_source_create(DISPATCH_SOURCE_TYPE_DATA_ADD, 0, 0, query->_async.dispatch.queue);
			dispatch_set_context(query->_async.dispatch.source, (void *) CFRetain(query));
			dispatch_source_set_event_handler_f(query->_async.dispatch.source, _query_perform);
			dispatch_source_set_cancel_handler(query->_async.dispatch.source, ^ {
				_query_cancel(query, NULL, NULL);
				CFRelease(query);
			});
			dispatch_resume(query->_async.dispatch.source);
		}
		break;
	case ASYNC_CF:
		if (query->_async.cf.source == NULL) {
			CFRunLoopSourceContext context = {
				0,
				query,
				CFRetain,
				CFRelease,
				CFCopyDescription,
				CFEqual,
				CFHash,
				_query_schedule,
				_query_cancel,
				_query_perform,
			};

			query->_async.cf.source = CFRunLoopSourceCreate(NULL, 0, &context);
			if (runLoop == NULL || runLoopMode == NULL) {
				CFArrayRef rlentry = CFArrayGetValueAtIndex(query->_async.cf.runloops, 0);
				runLoop = (CFRunLoopRef)CFArrayGetValueAtIndex(rlentry, 0);
				runLoopMode = CFArrayGetValueAtIndex(rlentry, 1);
			}
			CFRunLoopAddSource(runLoop, query->_async.cf.source, runLoopMode);
		}
		break;
	}
}

static void
_query_signal_source(ODQueryRef query)
{
	CFRunLoopRef rl;
	CFIndex i;

	(void)osx_assumes(dispatch_get_current_queue() == query->_queue);

	(void)osx_assumes(query->_async_type == ASYNC_DISPATCH || query->_async_type == ASYNC_CF);

	switch (query->_async_type) {
	case ASYNC_DISPATCH:
		if (query->_async.dispatch.source) {
			dispatch_source_merge_data(query->_async.dispatch.source, 1);
		}
		break;
	case ASYNC_CF:
		if (query->_async.cf.source) {
			CFRunLoopSourceSignal(query->_async.cf.source);
			for (i = 0; i < CFArrayGetCount(query->_async.cf.runloops); i++) {
				CFArrayRef rlentry = CFArrayGetValueAtIndex(query->_async.cf.runloops, i);
				rl = (CFRunLoopRef)CFArrayGetValueAtIndex(rlentry, 0);
				CFRunLoopWakeUp(rl);
			}
		}
		break;
	}
}

static void
_query_cancel_source(ODQueryRef query)
{
	(void)osx_assumes(dispatch_get_current_queue() == query->_queue);

	if (query->_async_type == ASYNC_DISPATCH && query->_async.dispatch.source != NULL) {
		dispatch_source_cancel(query->_async.dispatch.source);
		dispatch_release(query->_async.dispatch.source);
		query->_async.dispatch.source = NULL;
	} else if (query->_async_type == ASYNC_CF && query->_async.cf.source != NULL) {
		CFRunLoopRef rl;
		CFIndex i;

		CFRunLoopSourceInvalidate(query->_async.cf.source);
		for (i = 0; i < CFArrayGetCount(query->_async.cf.runloops); i++) {
			CFArrayRef rlentry = CFArrayGetValueAtIndex(query->_async.cf.runloops, i);
			rl = (CFRunLoopRef)CFArrayGetValueAtIndex(rlentry, 0);
			CFRunLoopWakeUp(rl);
		}

		CFRelease(query->_async.cf.source);
		query->_async.cf.source = NULL;
	}
}

static void
_query_request_callback(CFDictionaryRef p, uint32_t errcode, bool complete, void *context)
{
	ODQueryRef query = (ODQueryRef)context;
	CFStringRef funcname = NULL;
	CFArrayRef response = NULL;
	enum { eODQueryCreate, eODQuerySync, eODQueryResponse } response_type;
	CFTypeRef errInfo = NULL;
	CFDictionaryRef attrs;

	/* Sanity check. */
	(void)osx_assumes(dispatch_get_current_queue() == query->_queue);

	schema_deconstruct_result(p, &funcname, &response);

	if (funcname == NULL || response == NULL) {
		// most likely because p was NULL, i.e. we got an unexpected error
		if (errcode == 0) {
			errcode = kODErrorDaemonError;
		}
	} else if (CFStringCompare(funcname, CFSTR("ODQueryCreateWithNode"), 0) == kCFCompareEqualTo) {
		errInfo = schema_get_value_at_index(response, 1);
		response_type = eODQueryCreate;
	} else if (CFStringCompare(funcname, CFSTR("ODQueryResponseSync"), 0) == kCFCompareEqualTo) {
		response_type = eODQuerySync;
	} else if (CFStringCompare(funcname, CFSTR("ODQueryResponse"), 0) == kCFCompareEqualTo) {
		errInfo = schema_get_value_at_index(response, 2);
		response_type = eODQueryResponse;
	}

	if (errcode != 0) {
		/* In case of error, remove all results, store the error. */

		safe_cfrelease_null(query->_results);
		
		safe_cfrelease_null(query->_error);
		_ODErrorSet(&query->_error, errcode, errInfo);

		query->_complete = true;
		_query_signal_source(query);
	} else {
		switch (response_type) {
		case eODQueryCreate:
			query->_uuid = CFRetain(schema_get_value_at_index(response, 0));

			/* Workaround for 9243263. */
			if (complete) {
				query->_complete = true;
				_query_signal_source(query);
			}
			break;
		case eODQuerySync:
			safe_cfrelease_null(query->_results);
			query->_synchronize = true;
			_query_signal_source(query);
			break;
		case eODQueryResponse:
			attrs = schema_get_value_at_index(response, 1);
			if (attrs != NULL) {
				ODRecordRef record = _RecordCreate(query->_node, query->_attrset, attrs);

				if (record) {
					if (query->_results == NULL) {
						query->_results = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
					}
					CFArrayAppendValue(query->_results, record);
					CFRelease(record);
				}
			}

			if (complete) {
				query->_complete = true;
			}

			_query_signal_source(query);

			break;
		}
	}

	safe_cfrelease_null(funcname);
	safe_cfrelease_null(response);
}

static void
_query_schedule(void *context, CFRunLoopRef runLoop, CFStringRef runLoopMode)
{
	ODQueryRef query = (ODQueryRef)context;

	CFRetain(query);
	dispatch_async(query->_queue, ^{
		if (query->_transaction == NULL) {
			query->_transaction = transaction_create(query->_queue, _NodeGetSession(query->_node), query->_node, query->_request, query, _query_request_callback);
			transaction_send(query->_transaction);
		}
		CFRelease(query);
	});
}

static void
_query_cancel(void *context, CFRunLoopRef runLoop, CFStringRef runLoopMode)
{
	ODQueryRef query = (ODQueryRef)context;

	CFRetain(query);
	dispatch_async(query->_queue, ^{
		CFArrayRef response;
		uint32_t code;

		if (!query->_complete) {
			response = transaction_simple(&code, _NodeGetSession(query->_node), query->_node, CFSTR("ODQueryCancel"), 1, query->_uuid);

			// ignore any errors
			safe_cfrelease_null(response);
		}

		CFRelease(query);
	});
}

static void
_query_perform(void *context)
{
	ODQueryRef query = (ODQueryRef)context;
	__block CFArrayRef results = NULL;
	__block CFErrorRef error;
	__block bool complete;
	__block bool synchronize;

	if (!query->_callback_function) {
		return;
	}

	/* Now we deliver all pending events. */

	dispatch_sync(query->_queue, ^{
		if (query->_results) {
			results = safe_cfretain(query->_results);
			safe_cfrelease_null(query->_results);
		}
		error = query->_error;
		query->_error = NULL;
		complete = query->_complete;

		synchronize = query->_oktosync ? query->_synchronize : false;
		query->_synchronize = false;
	});

	if (synchronize) {
		/* deliver kODErrorQuerySynchronize callback */
		CFErrorRef sync_error = CFErrorCreate(NULL, kODErrorDomainFramework, kODErrorQuerySynchronize, NULL);
		query->_callback_function(query, NULL, sync_error, query->_callback_context);
		CFRelease(sync_error);
	}
	
	if (results) {
		query->_callback_function(query, results, NULL, query->_callback_context);
		CFRelease(results);

		dispatch_sync(query->_queue, ^ {
			query->_oktosync = true;
		});
	}
	
	// if complete or error
	if (error || complete) {
		query->_callback_function(query, NULL, error, query->_callback_context);
		safe_cfrelease(error);

		dispatch_sync(query->_queue, ^ {
			_query_cancel_source(query);
		});
	}
}

#pragma mark -

static CFArrayRef
_normalize_typeorlist(CFTypeRef typeOrList)
{
	CFTypeID type;

	if (typeOrList == NULL) {
		return NULL;
	}

	type = CFGetTypeID(typeOrList);

	if (type == CFArrayGetTypeID()) {
		return CFRetain(typeOrList);
	} else if (type == CFStringGetTypeID() || type == CFDataGetTypeID()) {
		return CFArrayCreate(NULL, &typeOrList, 1, &kCFTypeArrayCallBacks);
	} else {
		return NULL;
	}
}

ODQueryRef
_ODQueryCreate(CFAllocatorRef allocator)
{
	return (ODQueryRef)_CFRuntimeCreateInstance(NULL, ODQueryGetTypeID(), sizeof(struct __ODQuery) - sizeof(CFRuntimeBase), NULL);
}

ODQueryRef
_ODQueryInit(ODQueryRef query, ODNodeRef node, CFTypeRef recordTypeOrList, ODAttributeType attribute, ODMatchType matchType, CFTypeRef queryValueOrList, CFTypeRef returnAttributeOrList, CFIndex maxResults, CFErrorRef *error)
{
	CFIndex matchTypeIndex;
	CFNumberRef matchTypeNum, maxResultsNum;
	CFArrayRef recordTypes, queryValues;
	CFArrayRef returnAttributes;
	CFArrayRef minimizedAttributes = NULL;
	char qname[256];

	CLEAR_ERROR(error);
	if (!_validate_nonnull(error, CFSTR("node"), node)) {
		return NULL;
	}

	query->_node = (ODNodeRef)CFRetain(node);

	snprintf(qname, sizeof(qname), "com.apple.OpenDirectory.ODQuery.%p", query);
	query->_queue = dispatch_queue_create(qname, NULL);

	snprintf(qname, sizeof(qname), "com.apple.OpenDirectory.ODQuery.continue.%p", query);
	query->_continue_queue = dispatch_queue_create(qname, NULL);
	query->_continue_data = NULL;

	/* construct request for later use */
	recordTypes = _normalize_typeorlist(recordTypeOrList);

	matchTypeIndex = (CFIndex)matchType;
	matchTypeNum = CFNumberCreate(NULL, kCFNumberCFIndexType, &matchTypeIndex); // uint32_t

	queryValues = _normalize_typeorlist(queryValueOrList);

	returnAttributes = _normalize_typeorlist(returnAttributeOrList);
	query->_attrset = attrset_create_minimized_copy(returnAttributes);
	if (returnAttributes) {
		CFRelease(returnAttributes);
	}
	minimizedAttributes = CFArrayCreateWithSet(query->_attrset);

	maxResultsNum = CFNumberCreate(NULL, kCFNumberCFIndexType, &maxResults); // CFIndex

	query->_request = schema_construct_request(CFSTR("ODQueryCreateWithNode"), 6, recordTypes, attribute, matchTypeNum, queryValues, minimizedAttributes, maxResultsNum);

	if (recordTypes) {
		CFRelease(recordTypes);
	}
	CFRelease(matchTypeNum);
	if (queryValues) {
		CFRelease(queryValues);
	}
	if (minimizedAttributes) {
		CFRelease(minimizedAttributes);
	}
	CFRelease(maxResultsNum);

	return query;
}

#pragma mark Type Implementation

ODQueryRef
ODQueryCreateWithNode(CFAllocatorRef allocator, ODNodeRef node, CFTypeRef recordTypeOrList, ODAttributeType attribute, ODMatchType matchType, CFTypeRef queryValueOrList, CFTypeRef returnAttributeOrList, CFIndex maxResults, CFErrorRef *error)
{
	CLEAR_ERROR(error);
	if (!_validate_nonnull(error, CFSTR("node"), node)) {
		return NULL;
	}

	ODQueryRef query = _ODQueryCreate(allocator);

	return _ODQueryInit(query, node, recordTypeOrList, attribute, matchType, queryValueOrList, returnAttributeOrList, maxResults, error);
}

ODQueryRef
ODQueryCreateWithNodeType(CFAllocatorRef allocator, ODNodeType nodeType, CFTypeRef recordTypeOrList, ODAttributeType attribute, ODMatchType matchType, CFTypeRef queryValueOrList, CFTypeRef returnAttributeOrList, CFIndex maxResults, CFErrorRef *error)
{
	ODQueryRef query = NULL;
	ODNodeRef node;

	node = ODNodeCreateWithNodeType(NULL, NULL, nodeType, NULL);
	if (node) {
		query = ODQueryCreateWithNode(allocator, node, recordTypeOrList, attribute, matchType, queryValueOrList, returnAttributeOrList, maxResults, error);
		CFRelease(node);
	}

	return query;
}

struct query_continue_data {
	dispatch_queue_t queue;
	CFMutableArrayRef results;
	CFErrorRef error;
	bool done;
	dispatch_semaphore_t semaphore;
};

static struct query_continue_data *
_query_continue_create()
{
	struct query_continue_data *ctx;
	char qname[256];

	ctx = malloc(sizeof(struct query_continue_data));
	snprintf(qname, sizeof(qname), "com.apple.OpenDirectory.ODQueryCopyResults.context.%p", ctx);
	ctx->queue = dispatch_queue_create(qname, NULL);
	ctx->results = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
	ctx->error = NULL;
	ctx->done = false;
	ctx->semaphore = dispatch_semaphore_create(0);

	return ctx;
}

static void
_query_continue_destroy(struct query_continue_data *ctx)
{
	dispatch_release(ctx->queue);
	safe_cfrelease(ctx->results);
	safe_cfrelease(ctx->error);
	dispatch_release(ctx->semaphore);
	free(ctx);
}

static void
_query_continue_callback(ODQueryRef query, CFArrayRef results, CFErrorRef error, void *context)
{
	struct query_continue_data *ctx = (struct query_continue_data *)context;

	if (results) {
		if (ctx->results == NULL) {
			ctx->results = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
		}
		CFArrayAppendArray(ctx->results, results, CFRangeMake(0, CFArrayGetCount(results)));
	} else {
		if (error) {
			if (CFErrorGetCode(error) == kODErrorQuerySynchronize) {
				ctx->done = false;
			} else {
				ctx->done = true;
				ctx->error = (CFErrorRef)CFRetain(error);
			}

			safe_cfrelease_null(ctx->results);
		} else {
			ctx->done = true;
		}

		if (ctx->done) {
			dispatch_semaphore_signal(ctx->semaphore);
		}
	}
}

CFArrayRef
ODQueryCopyResults(ODQueryRef query, bool allowPartialResults, CFErrorRef *error)
{
	CLEAR_ERROR(error);
	if (!_validate_nonnull(error, CFSTR("query"), query)) {
		return NULL;
	}

	if (allowPartialResults == true && error == NULL) {
		OD_API_MISUSE("Cannot poll for query results without a valid error reference.");
	}

	__block CFArrayRef results;

	dispatch_sync(query->_continue_queue, ^ {
		if (query->_continue_data == NULL) {
			query->_continue_data = _query_continue_create();
			ODQuerySetCallback(query, _query_continue_callback, query->_continue_data);
			ODQuerySetDispatchQueue(query, query->_continue_data->queue);
		}
	});

	// TODO: we should block for at least one result in polling mode
	if (!allowPartialResults) {
		dispatch_semaphore_wait(query->_continue_data->semaphore, DISPATCH_TIME_FOREVER);
	}

	if (query->_continue_data->error) {
		if (error) {
			*error = query->_continue_data->error;
			query->_continue_data->error = NULL; // client owns it now
		} else {
			safe_cfrelease_null(query->_continue_data->error);
		}
	}

	dispatch_sync(query->_continue_data->queue, ^ {
		results = query->_continue_data->results;
		if (query->_continue_data->done == false) {
			query->_continue_data->results = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
		} else {
			query->_continue_data->results = NULL;
		}
	});

	return results;
}

void
ODQuerySynchronize(ODQueryRef query)
{
	dispatch_sync(query->_queue, ^ {
		CFArrayRef response;
		uint32_t code;

		if (query->_uuid == NULL) {
			return;
		}

		response = transaction_simple(&code, _NodeGetSession(query->_node), query->_node, CFSTR("ODQuerySynchronize"), 1, query->_uuid);
		(void)osx_assumes_zero(response);
		(void)osx_assumes(code == 0 || code == kODErrorQueryInvalid);

		// need to reset our state even if ODQuerySynchronize succeeds
		safe_cfrelease_null(query->_results);
		safe_cfrelease_null(query->_error);
		query->_complete = false;

		// if it failed, we need to create a new query on the daemon side
		if (code == kODErrorQueryInvalid) {
			transaction_cancel(query->_transaction);
			transaction_release(query->_transaction);
			query->_transaction = NULL;

			safe_cfrelease_null(query->_uuid);

			query->_synchronize = true; // force synchronize; we won't get one from opendirectoryd

			_query_init_source(query, NULL, NULL);
			_query_schedule(query, NULL, NULL);
		}
	});
}

void
ODQuerySetCallback(ODQueryRef query, ODQueryCallback callback, void *context)
{
	dispatch_sync(query->_queue, ^ {
		query->_callback_function = callback;
		query->_callback_context = context;
	});
}

void
ODQueryScheduleWithRunLoop(ODQueryRef query, CFRunLoopRef runLoop, CFStringRef runLoopMode)
{
	dispatch_sync(query->_queue, ^ {
		(void)osx_assumes(query->_async_type == ASYNC_UNSET || query->_async_type == ASYNC_CF);

		query->_async_type = ASYNC_CF;

		if (query->_async.cf.runloops == NULL) {
			query->_async.cf.runloops = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
		}

		CFTypeRef elts[] = { runLoop, runLoopMode };
		CFArrayRef rlentry = CFArrayCreate(NULL, elts, sizeof(elts) / sizeof(*elts), &kCFTypeArrayCallBacks);
		CFArrayAppendValue(query->_async.cf.runloops, rlentry);
		CFRelease(rlentry);

		_query_init_source(query, runLoop, runLoopMode);
	});
}

void
ODQueryUnscheduleFromRunLoop(ODQueryRef query, CFRunLoopRef runLoop, CFStringRef runLoopMode)
{
	dispatch_sync(query->_queue, ^ {
		CFIndex i;

		(void)osx_assumes(query->_async_type == ASYNC_CF);

		if (query->_async.cf.source) {
			CFRunLoopRemoveSource(runLoop, query->_async.cf.source, runLoopMode);

			CFTypeRef elts[] = { runLoop, runLoopMode };
			CFArrayRef rlentry = CFArrayCreate(NULL, elts, sizeof(elts) / sizeof(*elts), &kCFTypeArrayCallBacks);
			i = CFArrayGetLastIndexOfValue(query->_async.cf.runloops, CFRangeMake(0, CFArrayGetCount(query->_async.cf.runloops)), rlentry);
			CFRelease(rlentry);
			if (i >= 0) {
				CFArrayRemoveValueAtIndex(query->_async.cf.runloops, i);
			}
		}
	});
}

void
ODQuerySetDispatchQueue(ODQueryRef query, dispatch_queue_t queue)
{
	__block dispatch_queue_t delivery_queue = NULL;
	
	dispatch_sync(query->_queue, ^ {
		if (!(query->_async_type == ASYNC_UNSET || query->_async_type == ASYNC_DISPATCH)) {
			OD_API_MISUSE("Cannot mix result delivery mechanisms.");
		}

		query->_async_type = ASYNC_DISPATCH;
		delivery_queue = query->_async.dispatch.queue;

		if (queue) {
			// TODO: what if someone changes the queue? undefined?
			dispatch_retain(queue);
			query->_async.dispatch.queue = queue;

			_query_init_source(query, NULL, NULL);
			_query_schedule(query, NULL, NULL);
		} else if (delivery_queue != NULL) {
			_query_cancel_source(query);
			query->_async.dispatch.queue = NULL;
		}
	});
    
	if (delivery_queue != NULL) {
		dispatch_sync(delivery_queue, ^(void) {
			// a call can be in flight even though we canceled the source
			// execute an empty block to ensure it's completely cancelled before we return
		});
		safe_dispatch_release(delivery_queue);
	}
}
