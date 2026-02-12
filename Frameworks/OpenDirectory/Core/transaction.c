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
#include <xpc/xpc.h>
#include <asl.h>

#include <opendirectory/odutils.h>

#include "CFODSession.h"
#include "CFODNode.h"
#include "internal.h"
#include "transaction.h"
#include <CFOpenDirectory/CFOpenDirectoryConstants.h>

#include "odxpc.h"
#include "rb.h"

struct od_transaction_s {
	int32_t refcount;
	uint32_t canceled;
	CFDictionaryRef request;
	ODSessionRef session;
	ODNodeRef node;
	dispatch_queue_t target_queue;
	void *target_context;
	transaction_callback_t target_callback;
};

#pragma mark Helper Functions

static bool
_attempt_recovery(od_transaction_t transaction, uint32_t error)
{
	bool result = false;

	switch (error) {
	case kODErrorSessionInvalid:
		break;
	case kODErrorNodeInvalid:
		if (!CFEqual(CFDictionaryGetValue(transaction->request, CFSTR("funcname")), CFSTR("ODNodeRelease"))) {
			if (_ODNodeRecover(transaction->node)) {
				result = true;
			}
		}
		break;
	case kODErrorQueryInvalid:
		break;
	case kODErrorContextInvalid:
		break;
	default:
		break;
	}

	return result;
}

#pragma mark -

od_transaction_t
transaction_create(dispatch_queue_t queue, ODSessionRef session, ODNodeRef node, CFDictionaryRef request_dict, void *context, transaction_callback_t function)
{
	od_transaction_t transaction;

	transaction = calloc(1, sizeof(struct od_transaction_s));
	transaction->refcount = 1;

	transaction->session = session;
	transaction->node = node;
	transaction->request = CFRetain(request_dict);

	dispatch_retain(queue);
	transaction->target_queue = queue;
	transaction->target_context = context;
	transaction->target_callback = function;

	return transaction;
}

static void
transaction_retain(od_transaction_t transaction)
{
	if (__sync_add_and_fetch(&transaction->refcount, 1) == 1) {
		OD_CRASH("resurrected a transaction object");
	}
}

void
transaction_release(od_transaction_t transaction)
{
	int32_t rc = __sync_sub_and_fetch(&transaction->refcount, 1);

	if (rc > 0) {
		return;
	}

	if (rc == 0) {
		dispatch_release(transaction->target_queue);
		CFRelease(transaction->request);
		free(transaction);
		return;
	}

	OD_CRASH("over-released a transaction object");
}

void
transaction_cancel(od_transaction_t transaction)
{
	__sync_fetch_and_or(&transaction->canceled, 1);
}

static void
_handle_reply(od_transaction_t transaction, CFPropertyListRef plist, uint64_t error, bool complete)
{
	if (_attempt_recovery(transaction, error)) {
		transaction_send(transaction);
		return;
	}

	transaction->target_callback(plist, error, complete, transaction->target_context);
}

void
transaction_send(od_transaction_t transaction)
{
	uuid_t session_uuid, node_uuid;
	CFDataRef data;

	uuid_copy_session(session_uuid, transaction->session);
	uuid_copy_node(node_uuid, transaction->node);

	data = CFPropertyListCreateData(NULL, transaction->request, kCFPropertyListBinaryFormat_v1_0, 0, NULL);

	if (data) {
		transaction_retain(transaction);
		odxpc_send_message_with_reply(0, session_uuid, node_uuid, data, transaction->target_queue, ^(CFPropertyListRef plist, uint64_t error, bool complete) {
			if (!transaction->canceled) {
				_handle_reply(transaction, plist, error, complete);
			}

			if (complete) {
				transaction_release(transaction);
			}
		});
		CFRelease(data);
	} else {
		transaction_retain(transaction);
		dispatch_async(transaction->target_queue, ^{
			if (!transaction->canceled) {
				_handle_reply(transaction, NULL, kODErrorRecordParameterError, true);
			}

			transaction_release(transaction);
		});
	}
}

#pragma mark -

struct simple_ctx {
	dispatch_semaphore_t semaphore;
	CFArrayRef result;
	uint32_t error_code;
};

static void
_simple_callback(CFDictionaryRef plist, uint32_t error, bool complete, void *context)
{
	struct simple_ctx *ctx = (struct simple_ctx *)context;

	assert(ctx->result == NULL);

	schema_deconstruct_result(plist, NULL, &ctx->result);

	ctx->error_code = error;

	assert(complete);
	dispatch_semaphore_signal(ctx->semaphore);
}

CFArrayRef
transaction_simple(uint32_t *code, ODSessionRef session, ODNodeRef node, CFStringRef funcname, CFIndex total_params, ...)
{
	CFDictionaryRef request;
	CFArrayRef result;
	va_list ap;
	od_transaction_t transaction;

	va_start(ap, total_params);
	request = schema_construct_requestv(funcname, total_params, ap);
	va_end(ap);

	if (request) {
		dispatch_queue_t queue;
		char qname[256];
		struct simple_ctx ctx;

		snprintf(qname, sizeof(qname), "com.apple.OpenDirectory.%s.%p.", __FUNCTION__, request);
		CFStringGetCString(funcname, qname + strlen(qname), sizeof(qname) - strlen(qname), kCFStringEncodingUTF8);
		queue = dispatch_queue_create(qname, NULL);

		memset(&ctx, 0, sizeof(ctx));
		ctx.semaphore = dispatch_semaphore_create(0);

		transaction = transaction_create(queue, session, node, request, &ctx, _simple_callback);
		transaction_send(transaction);

		dispatch_semaphore_wait(ctx.semaphore, DISPATCH_TIME_FOREVER);
		dispatch_release(ctx.semaphore);

		transaction_cancel(transaction);
		transaction_release(transaction);

		dispatch_release(queue);

		result = ctx.result;
		*code = ctx.error_code;

		CFRelease(request);
	} else {
		result = NULL;
		*code = kODErrorRecordParameterError;
	}

	return result;
}

/*
 * transaction_simple_response
 * Checks various error conditions. In case of error, creates error. Only calls handler block if there is no error.
 */
void
transaction_simple_response(CFArrayRef response, uint32_t code, CFIndex error_index, CFErrorRef *error, transaction_response_handler_t handler)
{
	CFTypeRef errInfo = NULL;

	errInfo = response ? schema_get_value_at_index(response, error_index) : NULL;

	if (code != 0) {
		_ODErrorSet(error, code, errInfo);
	} else {
		if (error) {
			*error = NULL;
		}
		handler();
	}

	if (response) {
		CFRelease(response);
	}
}
