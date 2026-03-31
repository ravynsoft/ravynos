#include <CoreFoundation/CoreFoundation.h>
#include <xpc/xpc.h>
#include <xpc/private.h>
#include <asl.h>

#include <opendirectory/odconstants.h>
#include <opendirectory/odipc.h>
#include <opendirectory/odutils.h>

#include "rb.h"

#include "odxpc.h"

struct odxpc_request_s {
	struct rb_node rbn;
	dispatch_queue_t replyq;
	odxpc_handler_t handler;
	uint64_t client_id;
	int32_t refcount;
};
typedef struct odxpc_request_s *odxpc_request_t;

struct odxpc_connection_s {
	struct rb_node rbn;
	uid_t euid;
	xpc_connection_t xconn;
	bool valid;
};
typedef struct odxpc_connection_s *odxpc_connection_t;

static CFPropertyListRef
_xpc_dictionary_copy_plist(xpc_object_t xdict, const char *key)
{
	const void *ptr;
	size_t len;
	CFDataRef cfdata;
	CFPropertyListRef plist = NULL;
	CFPropertyListFormat format;

	ptr = xpc_dictionary_get_data(xdict, key, &len);
	if (ptr != NULL) {
		cfdata = CFDataCreateWithBytesNoCopy(NULL, ptr, len, kCFAllocatorNull);
		if (cfdata != NULL) {
			plist = CFPropertyListCreateWithData(NULL, cfdata, kCFPropertyListImmutable, &format, NULL);
			CFRelease(cfdata);

			if (plist != NULL && format != kCFPropertyListBinaryFormat_v1_0) {
				CFRelease(plist);
				plist = NULL;
			}
		}
	}

	return plist;
}

#define RBNODE_TO_ODXPC_CONNECTION(node) \
	((struct odxpc_connection_s *)((uintptr_t)node - offsetof(struct odxpc_connection_s, rbn)))

static int
_rbt_compare_connection_nodes(const struct rb_node *n1, const struct rb_node *n2)
{
	uid_t a = RBNODE_TO_ODXPC_CONNECTION(n1)->euid;
	uid_t b = RBNODE_TO_ODXPC_CONNECTION(n2)->euid;
	if (a > b)
		return 1;
	if (a < b)
		return -1;
	return 0;
}

static int
_rbt_compare_connection_key(const struct rb_node *n1, const void *key)
{
	uid_t a = RBNODE_TO_ODXPC_CONNECTION(n1)->euid;
	uid_t b = *(uid_t *)key;
	if (a > b)
		return 1;
	if (a < b)
		return -1;
	return 0;
}

#define RBNODE_TO_ODXPC_REQUEST(node) \
	((struct odxpc_request_s *)((uintptr_t)node - offsetof(struct odxpc_request_s, rbn)))

static int
_rbt_compare_transaction_nodes(const struct rb_node *n1, const struct rb_node *n2)
{
	uint64_t a = RBNODE_TO_ODXPC_REQUEST(n1)->client_id;
	uint64_t b = RBNODE_TO_ODXPC_REQUEST(n2)->client_id;
	if (a > b)
		return 1;
	if (a < b)
		return -1;
	return 0;
}

static int
_rbt_compare_transaction_key(const struct rb_node *n1, const void *key)
{
	uint64_t a = RBNODE_TO_ODXPC_REQUEST(n1)->client_id;
	uint64_t b = *(uint64_t *)key;
	if (a > b)
		return 1;
	if (a < b)
		return -1;
	return 0;
}

static struct rb_tree *
_odxpc_rbt()
{
	static dispatch_once_t once;
	static struct rb_tree rbtree;
	static const struct rb_tree_ops ops = {
		.rbto_compare_nodes = _rbt_compare_transaction_nodes,
		.rbto_compare_key = _rbt_compare_transaction_key,
	};

	dispatch_once(&once, ^{
		rb_tree_init(&rbtree, &ops);
	});

	return &rbtree;
}

static dispatch_queue_t
_odxpc_xpc_queue(void)
{
	static dispatch_once_t once;
	static dispatch_queue_t queue;

	dispatch_once(&once, ^{
		queue = dispatch_queue_create("com.apple.opendirectory.odxpc.xpc", NULL);
	});

	return queue;
}

static odxpc_request_t
_odxpc_request_create(dispatch_queue_t replyq, odxpc_handler_t handler)
{
	odxpc_request_t request;
	static uint64_t client_id;

	request = calloc(1, sizeof(*request));
	dispatch_retain(replyq);
	request->replyq = replyq;
	request->handler = Block_copy(handler);
	request->client_id = __sync_add_and_fetch(&client_id, 1);
	request->refcount = 1;

	return request;
}

static void
_odxpc_request_retain(odxpc_request_t request)
{
	if (__sync_add_and_fetch(&request->refcount, 1) == 1) {
		abort();
	}
}

static void
_odxpc_request_release(odxpc_request_t request)
{
	int32_t rc = __sync_sub_and_fetch(&request->refcount, 1);

	if (rc > 0) {
		return;
	}

	if (rc == 0) {
		dispatch_release(request->replyq);
		Block_release(request->handler);
		free(request);
		return;
	}

	abort();
}

// NB: Must be called in sync with _odxpc_xpc_queue()
static void
_odxpc_request_remove(uint64_t client_id)
{
	struct rb_tree *rbtree = _odxpc_rbt();
	struct rb_node *rbn = rb_tree_find_node(rbtree, &client_id);
	if (rbn) {
		rb_tree_remove_node(rbtree, rbn);
		_odxpc_request_release(RBNODE_TO_ODXPC_REQUEST(rbn));
	}
}

static odxpc_connection_t
_odxpc_create_connection(uid_t euid)
{
	odxpc_connection_t connection;

	connection = calloc(1, sizeof(*connection));
	connection->euid = euid;
	connection->valid = true;

	if (!issetugid() && getenv("OD_DEBUG_MODE") != NULL) {
		connection->xconn = xpc_connection_create(kODAPIPortNameDebug, _odxpc_xpc_queue());
	} else {
		connection->xconn = xpc_connection_create(kODAPIPortName, _odxpc_xpc_queue());
		xpc_connection_set_privileged(connection->xconn);
	}
	xpc_connection_set_legacy(connection->xconn);
	xpc_connection_set_event_handler(connection->xconn, ^(xpc_object_t xobj) {
		xpc_type_t type;

		type = xpc_get_type(xobj);
		if (type == XPC_TYPE_DICTIONARY) {
			uint64_t client_id;
			struct rb_node *rbn;

			client_id = xpc_dictionary_get_uint64(xobj, kODXPCReplyClientID);

			rbn = rb_tree_find_node(_odxpc_rbt(), &client_id);
			if (rbn) {
				odxpc_request_t request = RBNODE_TO_ODXPC_REQUEST(rbn);
				CFPropertyListRef plist;
				uint64_t error;
				bool complete;
				odxpc_handler_t handler;

				plist = _xpc_dictionary_copy_plist(xobj, kODXPCReplyData);
				error = xpc_dictionary_get_uint64(xobj, kODXPCReplyError);
				complete = xpc_dictionary_get_bool(xobj, kODXPCReplyComplete);
				handler = request->handler;

				_odxpc_request_retain(request);
				if (complete) {
					/* No more replies expected remove request via ID to maintain order */
					_odxpc_request_remove(client_id);
				}

				dispatch_async(request->replyq, ^{
					handler(plist, error, complete);

					if (plist) CFRelease(plist);

					_odxpc_request_release(request);
				});
			} else {
				asl_log(NULL, NULL, ASL_LEVEL_ERR, "received message with invalid client_id %llu", client_id);
			}
		} else if (type == XPC_TYPE_ERROR) {
			struct rb_node *rbn;
			odxpc_handler_t handler;

			RB_TREE_FOREACH(rbn, _odxpc_rbt()) {
				odxpc_request_t request = RBNODE_TO_ODXPC_REQUEST(rbn);
				uint64_t clientid = request->client_id;

				_odxpc_request_retain(request);
				_odxpc_request_remove(clientid);

				handler = request->handler;
				dispatch_async(request->replyq, ^{
					handler(NULL, kODErrorDaemonError, true);
					_odxpc_request_release(request);
				});
			}

			if (xobj != XPC_ERROR_CONNECTION_INTERRUPTED) {
				asl_log(NULL, NULL, ASL_LEVEL_ERR, "error communicating with opendirectoryd: %s", xpc_dictionary_get_string(xobj, XPC_ERROR_KEY_DESCRIPTION));
				connection->valid = false;
			}
		} else {
			asl_log(NULL, NULL, ASL_LEVEL_ERR, "bad event when communicating with opendirectoryd");
		}
	});
	xpc_connection_resume(connection->xconn);

	return connection;
}

static odxpc_connection_t
_odxpc_connection(void)
{
	static dispatch_once_t once;
	static dispatch_queue_t queue;
	static struct rb_tree rbtree;
	static const struct rb_tree_ops ops = {
		.rbto_compare_nodes = _rbt_compare_connection_nodes,
		.rbto_compare_key = _rbt_compare_connection_key,
	};
	__block odxpc_connection_t connection;

	dispatch_once(&once, ^{
		queue = dispatch_queue_create("com.apple.opendirectory.odxpc.connection", NULL);
		rb_tree_init(&rbtree, &ops);
	});

	/*
	 * Connection credentials are not actually set until a message is sent,
	 * so there is a potential race here. We will just assume that clients
	 * are not calling seteuid() from different threads.
	 */
	dispatch_sync(queue, ^{
		uid_t euid;
		struct rb_node *node;

		euid = geteuid();
		node = rb_tree_find_node(&rbtree, &euid);
		if (node != NULL) {
			connection = RBNODE_TO_ODXPC_CONNECTION(node);
		} else {
			connection = _odxpc_create_connection(euid);
			rb_tree_insert_node(&rbtree, &connection->rbn);
		}
	});

	return connection;
}

void
odxpc_send_message_with_reply(uint64_t reqtype, const uint8_t *session, const uint8_t *node, CFDataRef data, dispatch_queue_t replyq, odxpc_handler_t handler)
{
	static uuid_t null_uuid;

	dispatch_sync(_odxpc_xpc_queue(), ^{
		odxpc_connection_t connection;
		odxpc_request_t request;
		xpc_object_t message;

		connection = _odxpc_connection();
		if (!connection->valid) {
			dispatch_async(replyq, ^{
				handler(NULL, kODErrorDaemonError, true);
			});
		}

		request = _odxpc_request_create(replyq, handler);
		if (request == NULL || rb_tree_insert_node(_odxpc_rbt(), &request->rbn) == false) {
			__builtin_trap();
		}

		// need to do this synchronized with the rbt so it's not deleted out from under us
		message = xpc_dictionary_create(NULL, NULL, 0);
		xpc_dictionary_set_uint64(message, kODXPCMessageClientID, request->client_id);
		xpc_dictionary_set_uint64(message, kODXPCMessageReqType, reqtype);
		xpc_dictionary_set_uuid(message, kODXPCMessageSession, session ? session : null_uuid);
		xpc_dictionary_set_uuid(message, kODXPCMessageNode, node ? node : null_uuid);
		xpc_dictionary_set_data(message, kODXPCMessageRequest, CFDataGetBytePtr(data), CFDataGetLength(data));

		xpc_connection_send_message(connection->xconn, message);

		xpc_release(message);
	});
}
