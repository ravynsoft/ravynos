/*
 * Copyright (c) 2002 Apple Computer, Inc. All rights reserved.
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

#include <bootstrap_priv.h>
#include <unistd.h>
#include <opendirectory/odmig_types.h>
#include <syslog.h>
#include <mach/mach_error.h>
#include <errno.h>
#include "ServerControl.h"
#include "CNodeList.h"
#include "CLog.h"
#include <DirectoryService/DirectoryService.h>
#include "CContinue.h"
#include <bsm/libbsm.h>
#include "CInternalDispatch.h"
#include "od_passthru.h"
#include "rb.h"
#include "CSharedData.h"
#include <opendirectory/odutils.h>
#include <opendirectory/odtypes.h>
#include <opendirectory/odschema.h>
#include "CFOpenDirectory.h"
#include "DSSemaphore.h"

extern "C" {
	#include "legacy_reply.h"
	#include "legacy_callServer.h"
	#include "extmodule_callServer.h"
	#include "extmodule_reply.h"
	#include "session_callServer.h"
	#include "session_reply.h"
};

#define MY_MIG_OPTIONS	(MACH_RCV_TIMEOUT | MACH_RCV_TRAILER_ELEMENTS(MACH_RCV_TRAILER_CTX) | \
						 MACH_RCV_TRAILER_TYPE(MACH_MSG_TRAILER_FORMAT_0))

typedef CFDictionaryRef (*sync_fn)(CFTypeRef objectRef, CFArrayRef values, pid_t pid, uint32_t *err_code);
typedef void (*async_fn)(CFTypeRef objectRef, CFArrayRef values, mach_port_t reply, uint64_t reqid, pid_t pid);

struct sRefStruct
{
	struct rb_node	rbn;
	pid_t			pid;
	CFTypeRef		odRef;
	uuid_t			uuid;
};

// TODO: make this re-used
static mach_port_t		odd_port;
static bool				odd_logging_enabled;
static struct rb_tree	passthru_rbt;
static int32_t			localOnlyCount;
static CFStringRef		localonlyPath;
static dispatch_once_t	wire_once;

extern bool			gDebugLogging;
extern dsBool		gDSLocalOnlyMode;
extern DSSemaphore	gLocalSessionLock;
extern UInt32		gLocalSessionCount;

#define RBNODE_TO_REFSTRUCT(n) \
	((sRefStruct *)((uintptr_t)n - offsetof(sRefStruct, rbn)))

static int
_rbt_compare_uuid_nodes(const struct rb_node *n1, const struct rb_node *n2)
{
	return uuid_compare(RBNODE_TO_REFSTRUCT(n1)->uuid, RBNODE_TO_REFSTRUCT(n2)->uuid);
}

static int
_rbt_compare_uuid_key(const struct rb_node *n1, const void *key)
{
	return uuid_compare(RBNODE_TO_REFSTRUCT(n1)->uuid, (const unsigned char *) key);
}

static dispatch_queue_t
_passthru_rbt_queue(void)
{
	static dispatch_queue_t queue;
	static dispatch_once_t once;
	static struct rb_tree_ops ops = { _rbt_compare_uuid_nodes, _rbt_compare_uuid_key };
	
	dispatch_once(&once, 
				  ^(void) {
					  queue = dispatch_queue_create("com.apple.opendirectoryd.passthru", NULL);
					  rb_tree_init(&passthru_rbt, &ops);
				  });
	
	return queue;
}

static CFDataRef
_add_od_ref(CFTypeRef odRef, pid_t pid)
{
	sRefStruct *temp = new sRefStruct;
	
	temp->pid = pid;
	uuid_generate(temp->uuid);
	temp->odRef = odRef;

	dispatch_sync(_passthru_rbt_queue(), 
				  ^(void) {
					  bool success = rb_tree_insert_node(&passthru_rbt, &temp->rbn);
					  assert(success == true);
					  
					  if (LoggingEnabled(kLogDebug)) {
						  const char *type = "unknown";
						  uuid_string_t uuidStr;
						  
						  if (CFGetTypeID(odRef) == ODNodeGetTypeID()) {
							  type = "node";
						  }
						  else if (CFGetTypeID(odRef) == ODSessionGetTypeID()) {
							  type = "session";
						  }
						  else if (CFGetTypeID(odRef) == ODQueryGetTypeID()) {
							  type = "query";
						  }
						  else if (CFGetTypeID(odRef) == ODContextGetTypeID()) {
							  type = "context";
						  }
						  
						  uuid_unparse_upper(temp->uuid, uuidStr);
						  DbgLog(kLogDebug, "Added %s object %X as %s", type, odRef, uuidStr);
					  }
				  });
	
	return CFDataCreateWithBytesNoCopy(kCFAllocatorDefault, temp->uuid, sizeof(temp->uuid), kCFAllocatorNull);
}

static bool
_remove_identifier(CFTypeRef typeRef, pid_t pid)
{
	__block bool found = false;
	
	dispatch_sync(_passthru_rbt_queue(), 
				  ^(void) {
					  if (typeRef != NULL && CFGetTypeID(typeRef) == CFDataGetTypeID()) {
						  struct rb_node *rbnode = rb_tree_find_node(&passthru_rbt, CFDataGetBytePtr((CFDataRef) typeRef));
						  if (rbnode != NULL) {
							  struct sRefStruct *refstruct = RBNODE_TO_REFSTRUCT(rbnode);
							  if (pid == 0 || refstruct->pid == pid) {
								  rb_tree_remove_node(&passthru_rbt, rbnode);
								  
								  DbgLog(kLogDebug, "Removed object %X", typeRef);
								  
								  DSCFRelease(refstruct->odRef);
								  delete refstruct;
								  
								  found = true;
							  }
						  }
					  }
				  });
	
	return found;
}

static void
_delete_refs_for_pid(pid_t pid)
{
	dispatch_async(_passthru_rbt_queue(), 
				   ^(void) {
					   // clear the standard mapping table
					   struct rb_tree *rbtree = &passthru_rbt;
					   struct rb_node *rbnode = RB_TREE_MIN(rbtree);
					   
					   while (rbnode != NULL) {
						   sRefStruct *refstruct = RBNODE_TO_REFSTRUCT(rbnode);
						   struct rb_node *delNode = rbnode;
						   
						   rbnode = rb_tree_iterate(rbtree, rbnode, RB_DIR_RIGHT);
						   
						   // we have to delete after we iterate forward
						   rb_tree_remove_node(rbtree, delNode);
						   
						   DSCFRelease(refstruct->odRef);
						   delete refstruct;
					   };
				   });
}

static CFTypeRef
_get_and_retain_od_ref(CFTypeRef typeRef, pid_t pid, uint32_t *err_code, uint32_t error)
{
	__block CFTypeRef od_object = NULL;
	
	dispatch_sync(_passthru_rbt_queue(), 
				  ^(void) {
					  uuid_string_t uuidStr;
					  
					  (*err_code) = error;
					  if (typeRef != NULL) {
						  if (CFGetTypeID(typeRef) == CFDataGetTypeID()) {
							  struct rb_node *rbnode = rb_tree_find_node(&passthru_rbt, CFDataGetBytePtr((CFDataRef) typeRef));
							  if (rbnode != NULL) {
								  struct sRefStruct *refEntry = RBNODE_TO_REFSTRUCT(rbnode);
								  if (refEntry->pid == pid) {
									  od_object = refEntry->odRef;
									  if (LoggingEnabled(kLogDebug)) {

										  uuid_unparse_upper(refEntry->uuid, uuidStr);
										  DbgLog(kLogDebug, "Translated contextID %s to object %X", uuidStr, od_object);
									  }
									  CFRetain(od_object);
									  (*err_code) = 0;
								  }
							  }
							  else if (LoggingEnabled(kLogDebug)) {
								  uuid_unparse_upper(CFDataGetBytePtr((CFDataRef) typeRef), uuidStr);
								  DbgLog(kLogDebug, "Failed to find contextID %s", uuidStr);
							  }
						  }
						  else {
							  DbgLog(kLogInfo, "Failed to find contextID because it was the wrong type");
						  }
					  }
					  else {
						  (*err_code) = 0;
					  }
				  });
	
	return od_object;
}

static dispatch_queue_t
_get_localonly_session_queue(void)
{
	static dispatch_once_t once;
	static dispatch_queue_t queue;
	
	dispatch_once(&once, 
				  ^(void) {
					  queue = dispatch_queue_create("com.apple.DirectoryService.localonysession", NULL);
				  });
	
	return queue;
}

static dispatch_queue_t
_get_passthru_queue(void)
{
	static dispatch_queue_t queue;
	static dispatch_once_t once;
	
	dispatch_once(&once, 
				  ^(void) {
					  queue = dispatch_queue_create("com.apple.opendirectoryd.passthru", NULL);
				  });
	
	return queue;
}

static pthread_key_t
_od_passthru_thread_key(void)
{
	static dispatch_once_t once;
	static pthread_key_t key;
	
	dispatch_once(&once, 
				  ^(void){
#ifdef __LP64__
					  pthread_key_create(&key, NULL);
#else
					  pthread_key_create(&key, free);
#endif
				  });
	
	return key;
}

static pthread_key_t
_od_passthru_session_threadid(void)
{
	static dispatch_once_t once;
	static pthread_key_t key;
	
	dispatch_once(&once, 
				  ^(void){
					  pthread_key_create(&key, NULL);
				  });
	
	return key;
}

static pthread_key_t
_od_passthru_uid_key(void)
{
	static dispatch_once_t once;
	static pthread_key_t key;
	
	dispatch_once(&once, 
				  ^(void){
					  pthread_key_create(&key, NULL);
				  });
	
	return key;
}

static kern_return_t
_od_passthru_send_reply(mach_port_t port, uint64_t reqid, CFPropertyListRef replydata, uint32_t error, bool complete)
{
	vm_offset_t response = 0;
	mach_msg_type_number_t responseCnt = 0;
	kern_return_t kr;
	
	if (replydata != NULL) {
		CFDataRef responseData = CFPropertyListCreateData(kCFAllocatorDefault, replydata, 
														  kCFPropertyListBinaryFormat_v1_0, 0, NULL);
		
		vm_read(mach_task_self(),
				(vm_address_t) CFDataGetBytePtr(responseData), (vm_size_t) CFDataGetLength(responseData), 
				&response, &responseCnt);
		DSCFRelease(responseData);
	}
	
	if (pthread_getspecific(_od_passthru_session_threadid()) == NULL) {
		kr = send_extmodule_reply(port, reqid, response, responseCnt, error, complete);
	}
	else {
		kr = send_session_reply(port, reqid, response, responseCnt, error, complete);
	}
	
	if (kr == MACH_SEND_INVALID_DEST) {
		vm_deallocate(mach_task_self(), response, responseCnt);
	}
	
	if (complete) {
		mach_port_mod_refs(mach_task_self(), port, MACH_PORT_RIGHT_SEND, -1);
	}
	
	return kr;
}

static CFDictionaryRef
_SessionCreate(CFTypeRef unused, CFArrayRef values, pid_t pid, uint32_t *err_code)
{
	CFErrorRef		error;
	CFDictionaryRef answer = NULL;
	ODSessionRef	session;
	
	session = ODSessionCreate(kCFAllocatorDefault, (CFDictionaryRef) schema_get_value_at_index(values, 0), &error);
	if (session != NULL) {
		// use the pid as the refID that way we can clean up all of the nodes when the PID dies
		CFTypeRef sessionID = _add_od_ref(session, pid);
		answer = schema_construct_result(CFSTR("ODSessionCreate"), 2, sessionID, NULL);
		DSCFRelease(sessionID);
		
		(*err_code) = 0;
	}
	else {
		if (gDSLocalOnlyMode == true) {
			od_passthru_localonly_exit();
		}
		
		(*err_code) = CFErrorGetCode(error);
		DSCFRelease(error);
	}
	
	return answer;
}

static void
_SessionCopyNodeNames_async(CFTypeRef object, CFArrayRef values, mach_port_t reply, uint64_t reqid, pid_t pid)
{
	CFTypeRef		result		= NULL;
	uint32_t		err_code	= 0;
	CFErrorRef		error;
	ODSessionRef	session		= (ODSessionRef) object;
	
	if (object == NULL || CFGetTypeID(object) == ODSessionGetTypeID()) {
		CFArrayRef names = ODSessionCopyNodeNames(kCFAllocatorDefault, session, &error);
		if (error != NULL) {
			err_code = CFErrorGetCode(error);
			DSCFRelease(error);
		}
		
		result = schema_construct_result(CFSTR("ODSessionCopyNodeNames"), 2, names, NULL);
		DSCFRelease(names);
	}
	else {
		err_code = kODErrorDaemonError;
	}
	
	_od_passthru_send_reply(reply, reqid, result, err_code, true);
	DSCFRelease(result);
}

static void
_NodeCreateWithName_async(CFTypeRef object, CFArrayRef values, mach_port_t reply, uint64_t reqid, pid_t pid)
{
	CFErrorRef		error;
	CFDictionaryRef answer		= NULL;
	uint32_t		err_code	= 0;
	ODNodeRef		node		= NULL;
	ODSessionRef	session		= (ODSessionRef) object;
	
	if (object == NULL || CFGetTypeID(object) == ODSessionGetTypeID()) {
		node = ODNodeCreateWithName(kCFAllocatorDefault,
									session, 
									(CFStringRef) schema_get_value_at_index(values, 0),
									&error);
		if (node != NULL) {
			// use the pid as the refID that way we can clean up all of the nodes when the PID dies
			CFTypeRef nodeID = _add_od_ref(node, pid);
			answer = schema_construct_result(CFSTR("ODNodeCreateWithName"), 2, nodeID, NULL);
			DSCFRelease(nodeID);
			
			err_code = 0;
		}
		else {
			err_code = CFErrorGetCode(error);
			DSCFRelease(error);
		}
	}
	
bail:
	_od_passthru_send_reply(reply, reqid, answer, err_code, true);
	
	DSCFRelease(answer);
}

static CFDictionaryRef
_NodeCreateWithName(CFTypeRef object, CFArrayRef values, pid_t pid, uint32_t *err_code)
{
	/*
	 * the PID should be checked against the refnum for a pointer when looked up for security
	 * since the refnum == PID
	 */
	CFErrorRef		error;
	CFDictionaryRef answer	= NULL;
	ODNodeRef		node	= NULL;
	ODSessionRef	session	= (ODSessionRef) object;
	ODSessionRef	newSess	= NULL;
	
	// unique session per node to prevent deadlocks
	if (session == NULL) {
		newSess = ODSessionCreate(kCFAllocatorDefault, NULL, NULL);
		if (newSess == NULL) {
			DbgLog(kLogError, "failed to create a new session");
		}
		session = newSess;
	}
	
	if (session == NULL || CFGetTypeID(session) == ODSessionGetTypeID()) {
		node = ODNodeCreateWithName(kCFAllocatorDefault,
									session, 
									(CFStringRef) schema_get_value_at_index(values, 0),
									&error);
		if (node != NULL) {
			// use the pid as the refID that way we can clean up all of the nodes when the PID dies
			CFTypeRef nodeID = _add_od_ref(node, pid);
			answer = schema_construct_result(CFSTR("ODNodeCreateWithName"), 2, nodeID, NULL);
			DSCFRelease(nodeID);
			
			(*err_code) = 0;
		}
		else {
			(*err_code) = CFErrorGetCode(error);
			DSCFRelease(error);
		}
	}
	
	DSCFRelease(newSess);
	
	return answer;
}

static void 
_NodeCopySubnodeNames_async(CFTypeRef object, CFArrayRef values, mach_port_t reply, uint64_t reqid, pid_t pid)
{
	ODNodeRef		node		= (ODNodeRef) object;
	CFDictionaryRef result		= NULL;
	uint32_t		err_code	= 0;
	CFErrorRef		error		= NULL;
	
	if (object != NULL && CFGetTypeID(object) == ODNodeGetTypeID()) {
		CFArrayRef names = ODNodeCopySubnodeNames(node, &error);
		if (error != NULL) {
			err_code = CFErrorGetCode(error);
			DSCFRelease(error);
		}
		
		result = schema_construct_result(CFSTR("ODNodeCopySubnodeNames"), 2, names, NULL);
		DSCFRelease(names);
	} else {
		err_code = kODErrorNodeInvalid;
	}
	
	_od_passthru_send_reply(reply, reqid, result, err_code, true);
	DSCFRelease(result);
}

static void 
_NodeCopyUnreachableSubnodeNames_async(CFTypeRef object, CFArrayRef values, mach_port_t reply, uint64_t reqid, pid_t pid)
{
	ODNodeRef		node		= (ODNodeRef) object;
	CFDictionaryRef result		= NULL;
	uint32_t		err_code	= 0;
	CFErrorRef		error		= NULL;
	CFArrayRef		names;
	
	if (object != NULL && CFGetTypeID(object) == ODNodeGetTypeID()) {
		names = ODNodeCopyUnreachableSubnodeNames(node, &error);
		if (error != NULL) {
			err_code = CFErrorGetCode(error);
			DSCFRelease(error);
		}
		
		result = schema_construct_result(CFSTR("ODNodeCopyUnreachableSubnodeNames"), 2, names, NULL);
		DSCFRelease(names);
	} else {
		err_code = kODErrorNodeInvalid;
	}
	
	_od_passthru_send_reply(reply, reqid, result, err_code, true);
	DSCFRelease(result);
}

static void 
_NodeCopyDetails_async(CFTypeRef object, CFArrayRef values, mach_port_t reply, uint64_t reqid, pid_t pid)
{
	ODNodeRef		node		= (ODNodeRef) object;
	CFDictionaryRef result		= NULL;
	uint32_t		err_code	= 0;
	CFErrorRef		error		= NULL;
	CFDictionaryRef details;
	
	if (object != NULL && CFGetTypeID(object) == ODNodeGetTypeID()) {
		details = ODNodeCopyDetails(node, (CFArrayRef) schema_get_value_at_index(values, 0), &error);
		if (error != NULL) {
			err_code = CFErrorGetCode(error);
			DSCFRelease(error);
		}
		
		result = schema_construct_result(CFSTR("ODNodeCopyDetails"), 2, details, NULL);
		DSCFRelease(details);
	} else {
		err_code = kODErrorNodeInvalid;
	}
	
	_od_passthru_send_reply(reply, reqid, result, err_code, true);
	DSCFRelease(result);
}

static void 
_NodeCopySupportedRecordTypes_async(CFTypeRef object, CFArrayRef values, mach_port_t reply, uint64_t reqid, pid_t pid)
{
	ODNodeRef		node		= (ODNodeRef) object;
	CFDictionaryRef result		= NULL;
	uint32_t		err_code	= 0;
	CFErrorRef		error		= NULL;
	
	if (object != NULL && CFGetTypeID(object) == ODNodeGetTypeID()) {
		CFArrayRef recTypes = ODNodeCopySupportedRecordTypes(node, &error);
		if (error != NULL) {
			err_code = CFErrorGetCode(error);
			DSCFRelease(error);
		}
		
		result = schema_construct_result(CFSTR("ODNodeCopySupportedRecordTypes"), 2, recTypes, NULL);
		DSCFRelease(recTypes);
	} else {
		err_code = kODErrorNodeInvalid;
	}
	
	_od_passthru_send_reply(reply, reqid, result, err_code, true);
	DSCFRelease(result);
}

static void 
_NodeCopySupportedAttributes_async(CFTypeRef object, CFArrayRef values, mach_port_t reply, uint64_t reqid, pid_t pid)
{
	ODNodeRef		node		= (ODNodeRef) object;
	CFDictionaryRef result		= NULL;
	uint32_t		err_code	= 0;
	CFErrorRef		error		= NULL;
	
	if (object != NULL && CFGetTypeID(object) == ODNodeGetTypeID()) {
		CFArrayRef attribs = ODNodeCopySupportedAttributes(node, 
														   (CFStringRef) schema_get_value_at_index(values, 0),
														   &error);
		if (error != NULL) {
			err_code = CFErrorGetCode(error);
			DSCFRelease(error);
		}
		
		result = schema_construct_result(CFSTR("ODNodeCopySupportedAttributes"), 2, attribs, NULL);
		DSCFRelease(attribs);
	} else {
		err_code = kODErrorNodeInvalid;
	}
	
	_od_passthru_send_reply(reply, reqid, result, err_code, true);
	DSCFRelease(result);
}

static void 
_NodeSetCredentials_async(CFTypeRef object, CFArrayRef values, mach_port_t reply, uint64_t reqid, pid_t pid)
{
	ODNodeRef		node		= (ODNodeRef) object;
	CFDictionaryRef result		= NULL;
	uint32_t		err_code	= 0;
	CFErrorRef		error		= NULL;
	
	if (object != NULL && CFGetTypeID(object) == ODNodeGetTypeID()) {
		bool success = ODNodeSetCredentials(node, 
											(ODRecordType) schema_get_value_at_index(values, 0), 
											(CFStringRef) schema_get_value_at_index(values, 1), 
											(CFStringRef) schema_get_value_at_index(values, 2), 
											&error);
		if (error != NULL) {
			err_code = CFErrorGetCode(error);
			DSCFRelease(error);
		}
		
		result = schema_construct_result(CFSTR("ODNodeSetCredentials"), 2, (success ? kCFBooleanTrue : kCFBooleanFalse), NULL);
	} else {
		err_code = kODErrorNodeInvalid;
	}
	
	_od_passthru_send_reply(reply, reqid, result, err_code, true);
	DSCFRelease(result);
}

static void 
_NodeSetCredentialsExtended_async(CFTypeRef object, CFArrayRef values, mach_port_t reply, uint64_t reqid, pid_t pid)
{
	ODNodeRef		node		= (ODNodeRef) object;
	CFDictionaryRef result		= NULL;
	uint32_t		err_code	= 0;
	CFErrorRef		error		= NULL;
	CFTypeRef		contextRef	= NULL;
	CFArrayRef		outAuthItems;
	
	if (object != NULL && CFGetTypeID(object) == ODNodeGetTypeID()) {
		ODContextRef	context;
		ODContextRef	outContext;
		bool			success;
		
		context = outContext = (ODContextRef) _get_and_retain_od_ref(schema_get_value_at_index(values, 3), pid, &err_code,
																	 kODErrorContextInvalid);
		success = ODNodeSetCredentialsExtended(node, 
											   (ODRecordType) schema_get_value_at_index(values, 0),
											   (ODAuthenticationType) schema_get_value_at_index(values, 1), 
											   (CFArrayRef) schema_get_value_at_index(values, 2), 
											   &outAuthItems, &outContext, &error);
		if (context != outContext) {
			contextRef = _add_od_ref(outContext, pid);
		}
		else if (context != NULL) {
			_remove_identifier(context, pid);
		}
		
		if (error != NULL) {
			err_code = CFErrorGetCode(error);
			DSCFRelease(error);
		}
		
		result = schema_construct_result(CFSTR("ODNodeSetCredentialsExtended"), 4, (success ? kCFBooleanTrue : kCFBooleanFalse), 
										 outAuthItems, contextRef, NULL);
		DSCFRelease(outAuthItems);
		DSCFRelease(contextRef);
	} else {
		err_code = kODErrorNodeInvalid;
	}
	
	_od_passthru_send_reply(reply, reqid, result, err_code, true);
	DSCFRelease(result);
}

static void 
_NodeVerifyCredentialsExtended_async(CFTypeRef object, CFArrayRef values, mach_port_t reply, uint64_t reqid, pid_t pid)
{
	ODNodeRef		node		= (ODNodeRef) object;
	CFDictionaryRef result		= NULL;
	uint32_t		err_code	= 0;
	CFErrorRef		error		= NULL;
	CFTypeRef		contextRef	= NULL;
	CFArrayRef		outAuthItems;
	
	if (object != NULL && CFGetTypeID(object) == ODNodeGetTypeID()) {
		ODContextRef	context;
		ODContextRef	outContext;
		bool			success;
		
		context = outContext = (ODContextRef) _get_and_retain_od_ref(schema_get_value_at_index(values, 3), pid, &err_code,
																	 kODErrorContextInvalid);
		success = ODNodeVerifyCredentialsExtended(node, 
												  (ODRecordType) schema_get_value_at_index(values, 0),
												  (ODAuthenticationType) schema_get_value_at_index(values, 1), 
												  (CFArrayRef) schema_get_value_at_index(values, 2), 
												  &outAuthItems, &outContext, &error);
		if (context != outContext) {
			contextRef = _add_od_ref(outContext, pid);
		}
		else if (context != NULL) {
			_remove_identifier(context, pid);
		}
		
		if (error != NULL) {
			err_code = CFErrorGetCode(error);
			DSCFRelease(error);
		}
		
		result = schema_construct_result(CFSTR("ODNodeVerifyCredentialsExtended"), 4, (success ? kCFBooleanTrue : kCFBooleanFalse), 
										 outAuthItems, contextRef, NULL);
		DSCFRelease(outAuthItems);
		DSCFRelease(contextRef);
	} else {
		err_code = kODErrorNodeInvalid;
	}
	
	_od_passthru_send_reply(reply, reqid, result, err_code, true);
	DSCFRelease(result);
}

static void 
_NodeCreateRecord_async(CFTypeRef object, CFArrayRef values, mach_port_t reply, uint64_t reqid, pid_t pid)
{
	ODNodeRef		node		= (ODNodeRef) object;
	CFDictionaryRef result		= NULL;
	uint32_t		err_code	= 0;
	CFErrorRef		error		= NULL;
	
	if (object != NULL && CFGetTypeID(object) == ODNodeGetTypeID()) {
		ODRecordRef record = ODNodeCreateRecord(node, 
												(ODRecordType) schema_get_value_at_index(values, 0), 
												(CFStringRef) schema_get_value_at_index(values, 1), 
												(CFDictionaryRef) schema_get_value_at_index(values, 2), 
												&error);
		
		if (record != NULL) {
			CFDictionaryRef details = ODRecordCopyDetails(record, NULL, NULL);
			
			result = schema_construct_result(CFSTR("ODNodeCreateRecord"), 2, NULL, details);
			
			DSCFRelease(details);
			DSCFRelease(record);
		} else {
			err_code = kODErrorDaemonError;
		}
		
		if (error != NULL) {
			err_code = CFErrorGetCode(error);
			DSCFRelease(error);
		}
	} else {
		err_code = kODErrorNodeInvalid;
	}
	
	_od_passthru_send_reply(reply, reqid, result, err_code, true);
	DSCFRelease(result);
}

static void 
_NodeCopyRecord_async(CFTypeRef object, CFArrayRef values, mach_port_t reply, uint64_t reqid, pid_t pid)
{
	ODNodeRef		node		= (ODNodeRef) object;
	CFDictionaryRef result		= NULL;
	uint32_t		err_code	= 0;
	CFErrorRef		error		= NULL;
	
	if (object != NULL && CFGetTypeID(object) == ODNodeGetTypeID()) {
		ODRecordRef record = ODNodeCopyRecord(node, 
											  (ODRecordType) schema_get_value_at_index(values, 0), 
											  (CFStringRef) schema_get_value_at_index(values, 1), 
											  (CFArrayRef) schema_get_value_at_index(values, 2), 
											  &error);
		
		if (record != NULL) {
			CFDictionaryRef details = ODRecordCopyDetails(record, NULL, NULL);
			
			result = schema_construct_result(CFSTR("ODNodeCopyRecord"), 2, details, NULL);
			
			DSCFRelease(details);
			DSCFRelease(record);
		}
		
		// Unlike other API cases, we never return an error for records not found
		// API definition states NULL + noErr means the record was not found (which is not considered an error)
		// so only return an error if it actually had an error
		
		if (error != NULL) {
			err_code = CFErrorGetCode(error);
			DSCFRelease(error);
		}
	} else {
		err_code = kODErrorNodeInvalid;
	}
	
	_od_passthru_send_reply(reply, reqid, result, err_code, true);
	DSCFRelease(result);
}

static void 
_NodeCustomCall_async(CFTypeRef object, CFArrayRef values, mach_port_t reply, uint64_t reqid, pid_t pid)
{
	ODNodeRef		node		= (ODNodeRef) object;
	CFDictionaryRef result		= NULL;
	uint32_t		err_code	= 0;
	CFErrorRef		error		= NULL;
	
	if (object != NULL && CFGetTypeID(object) == ODNodeGetTypeID()) {
		CFIndex		customCode	= 0;
		CFNumberRef number		= (CFNumberRef) schema_get_value_at_index(values, 0);
		CFDataRef	answer;
		
		if (number != NULL) {
			CFNumberGetValue(number, kCFNumberCFIndexType, &customCode);
		}
		
		answer = ODNodeCustomCall(node, customCode, (CFDataRef) schema_get_value_at_index(values, 1), &error);
		if (error != NULL) {
			err_code = CFErrorGetCode(error);
			DSCFRelease(error);
		}
		
		result = schema_construct_result(CFSTR("ODNodeCustomCall"), 2, answer, NULL);
		DSCFRelease(answer);
	} else {
		err_code = kODErrorNodeInvalid;
	}
	
	_od_passthru_send_reply(reply, reqid, result, err_code, true);
	DSCFRelease(result);
}

static void 
_NodeSetNodeCredentials_async(CFTypeRef object, CFArrayRef values, mach_port_t reply, uint64_t reqid, pid_t pid)
{
	ODNodeRef		node		= (ODNodeRef) object;
	CFDictionaryRef result		= NULL;
	uint32_t		err_code	= 0;
	CFErrorRef		error		= NULL;
	
	if (object != NULL && CFGetTypeID(object) == ODNodeGetTypeID()) {
		bool success = ODNodeSetCredentials(node, 
											(ODRecordType) schema_get_value_at_index(values, 2), 
											(CFStringRef) schema_get_value_at_index(values, 3),
											(CFStringRef) schema_get_value_at_index(values, 4),
											&error);
		if (error != NULL) {
			err_code = CFErrorGetCode(error);
			DSCFRelease(error);
		}
		
		result = schema_construct_result(CFSTR("ODNodeSetCredentials"), 2, (success ? kCFBooleanTrue : kCFBooleanFalse), NULL);
	} else {
		err_code = kODErrorNodeInvalid;
	}
	
	_od_passthru_send_reply(reply, reqid, result, err_code, true);
	DSCFRelease(result);
}

static void
_QueryStart(void *context)
{
	ODQueryRef		query	= (ODQueryRef) context;
	uint64_t		reqid	= ODQueryGetRequestID(query);
	CFDataRef		queryID	= ODQueryGetQueryID(query);
	mach_port_t		reply	= ODQueryGetReplyPort(query);
	CFErrorRef		localError;
	
	CInternalDispatch::AddCapability();
	
	if (ODQueryIsExternalSession(query) == true) {
		pthread_setspecific(_od_passthru_session_threadid(), (void *) 1);
	}
	
#ifdef __LP64__
	pthread_setspecific(_od_passthru_thread_key(), (void *) reqid);
#else
	uint64_t *threadkey = (uint64_t *) malloc(sizeof(reqid));
	(*threadkey) = reqid;
	pthread_setspecific(_od_passthru_thread_key(), threadkey);
#endif
	
	CFDictionaryRef	result = schema_construct_result(CFSTR("ODQueryResponseSync"), 1, queryID);
	_od_passthru_send_reply(reply, reqid, result, 0, false);
	DSCFRelease(result);
	
	for (;;) {
		if (ODQueryCancelled(query) == true) {
			result = schema_construct_result(CFSTR("ODQueryResponse"), 3, queryID, NULL, NULL);
			_od_passthru_send_reply(reply, reqid, result, 0, true);
			DSCFRelease(result);
			break;
		}
		
		CFArrayRef results = ODQueryCopyResults(query, true, &localError);
		if (results == NULL) {
			uint32_t localCode = 0;
			
			if (localError != NULL) {
				localCode = CFErrorGetCode(localError);
				CFRelease(localError);
			}
			
			// we are done, if the client does a sync, it will re-issue the query
			result = schema_construct_result(CFSTR("ODQueryResponse"), 3, queryID, NULL, NULL);
			_od_passthru_send_reply(reply, reqid, result, localCode, true);
			DSCFRelease(result);
			break;
		}
		else {
			CFIndex count = CFArrayGetCount(results);
			kern_return_t kr = KERN_SUCCESS;
			
			for (CFIndex ii = 0; ii < count && kr == KERN_SUCCESS; ii++) {
				ODRecordRef record = (ODRecordRef) CFArrayGetValueAtIndex(results, ii);
				
				CFDictionaryRef details = ODRecordCopyDetails(record, NULL, NULL);
				result = schema_construct_result(CFSTR("ODQueryResponse"), 3, queryID, details, NULL);
				kr = _od_passthru_send_reply(reply, reqid, result, 0, false);
				if (kr != KERN_SUCCESS) {
					ODQueryCancel(query);
					DbgLog(kLogNotice, "Failed to deliver query result, cancelling request");
				}
				
				DSCFRelease(details);
				DSCFRelease(result);
			}
		}
		
		DSCFRelease(results);
	};
	
	pthread_setspecific(_od_passthru_session_threadid(), NULL);
	
	_remove_identifier(queryID, 0);
	CFRelease(query);
}

static void 
_QueryCreateWithNode_async(CFTypeRef object, CFArrayRef values, mach_port_t reply, uint64_t reqid, pid_t pid)
{
	ODQueryRef	query		= NULL;
	CFDataRef	queryID		= NULL;
	ODNodeRef	node		= (ODNodeRef) object;
	uint32_t	err_code	= 0;
	CFErrorRef	error;
	
	if (object != NULL && CFGetTypeID(object) == ODNodeGetTypeID()) {
		CFIndex matchType = kODMatchEqualTo;
		CFIndex maxCount = 0;
		ODAttributeType attribute = (ODAttributeType) schema_get_value_at_index(values, 1);
		CFArrayRef rectypes = (CFArrayRef) schema_get_value_at_index(values, 0);
		CFStringRef rectype = kODRecordTypeUsers;
		CFDictionaryRef schema;
		
		// pick first type
		if (rectypes != NULL && CFArrayGetCount(rectypes) != 0) {
			rectype = (CFStringRef) CFArrayGetValueAtIndex(rectypes, 0);
		}
		
		if (attribute == NULL || CFEqual(attribute, CFSTR("dsAttrTypeStandard:AppleMetaAmbiguousName")) == TRUE) {
			attribute = kODAttributeTypeRecordName;
		}
		
		CFNumberRef tempMatch = (CFNumberRef) schema_get_value_at_index(values, 2);
		if (tempMatch != NULL) {
			CFNumberGetValue(tempMatch, kCFNumberCFIndexType, &matchType);
		}
		
		// 9031793: remove metarecordname
		CFMutableArrayRef requested_attributes = (CFMutableArrayRef) schema_get_value_at_index(values, 4);
		if (requested_attributes != NULL && CFGetTypeID(requested_attributes) == CFArrayGetTypeID()) {
			CFIndex idx = CFArrayGetFirstIndexOfValue(requested_attributes, CFRangeMake(0, CFArrayGetCount(requested_attributes)), CFSTR("dsAttrTypeStandard:AppleMetaRecordName"));
			if (idx >= 0) {
				CFArrayRemoveValueAtIndex(requested_attributes, idx);
			}
		}
		
		CFNumberRef tempCount = (CFNumberRef) schema_get_value_at_index(values, 5);
		if (tempCount != NULL) {
			CFNumberGetValue(tempCount, kCFNumberCFIndexType, &maxCount);
		}
		
		query = ODQueryCreateWithNode(kCFAllocatorDefault, node, 
									  rectypes,
									  attribute,
									  odschema_get_legacy_matchtype(rectype, attribute, matchType),
									  schema_get_value_at_index(values, 3),
									  requested_attributes,
									  maxCount,
									  &error);
		
		if (error != NULL) {
			err_code = CFErrorGetCode(error);
			DSCFRelease(error);
		}
		
		if (query != NULL) {
			queryID = _add_od_ref(query, pid);
			
			ODQuerySetRequestID(query, reqid);
			ODQuerySetReplyPort(query, reply);
			ODQuerySetQueryID(query, queryID);
			ODQuerySetPID(query, pid);
			
			if (pthread_getspecific(_od_passthru_session_threadid()) != NULL) {
				ODQuerySetExternalSession(query, true);
			}
			
			CFDictionaryRef result = schema_construct_result(CFSTR("ODQueryCreateWithNode"), 2, queryID, NULL);
			_od_passthru_send_reply(reply, reqid, result, 0, false);
			DSCFRelease(result);
			DSCFRelease(queryID);
			
			CFRetain(query);
			dispatch_async_f(ODQueryGetDispatchQueue(query), query, _QueryStart);
		}
		else {
			_od_passthru_send_reply(reply, reqid, NULL, err_code, true);
		}
	} else {
		err_code = kODErrorNodeInvalid;
	}
}

static void 
_QuerySynchronize_async(CFTypeRef object, CFArrayRef values, mach_port_t reply, uint64_t reqid, pid_t pid)
{
	ODQueryRef	query		= (ODQueryRef) object;
	uint32_t	err_code	= 0;
	CFErrorRef	error;
	
	if (object != NULL && CFGetTypeID(object) == ODQueryGetTypeID()) {
		ODQueryCancel(query);
		ODQuerySynchronize(query);
		
		CFRetain(query);
		dispatch_async_f(ODQueryGetDispatchQueue(query), query, _QueryStart);
	} else {
		err_code = kODErrorQueryInvalid;
	}
	
	_od_passthru_send_reply(reply, reqid, NULL, err_code, true);
}

static void 
_QueryCancel_async(CFTypeRef object, CFArrayRef values, mach_port_t reply, uint64_t reqid, pid_t pid)
{
	ODQueryRef	query		= (ODQueryRef) object;
	uint32_t	err_code	= 0;
	CFErrorRef	error;
	
	if (object != NULL && CFGetTypeID(object) == ODQueryGetTypeID()) {
		ODQueryCancel(query);
	} else {
		err_code = kODErrorQueryInvalid;
	}
	
	_od_passthru_send_reply(reply, reqid, NULL, err_code, true);
}

static CFDictionaryRef
_QueryCancel(CFTypeRef object, CFArrayRef values, pid_t pid, uint32_t *err_code)
{
	ODQueryRef	query	= (ODQueryRef) object;
	CFErrorRef	error	= 0;
	
	if (object != NULL && CFGetTypeID(object) == ODQueryGetTypeID()) {
		ODQueryCancel(query);
	} else {
		(*err_code) = kODErrorQueryInvalid;
	}
	
	return NULL;
}

static void 
_RecordCopyPasswordPolicy_async(CFTypeRef object, CFArrayRef values, mach_port_t reply, uint64_t reqid, pid_t pid)
{
	CFDictionaryRef	policy		= NULL;
	ODNodeRef		node		= (ODNodeRef) object;
	CFDictionaryRef result		= NULL;
	uint32_t		err_code	= 0;
	CFErrorRef		error		= NULL;
	
	if (object != NULL && CFGetTypeID(object) == ODNodeGetTypeID()) {
		// 9031793: ignore metarecordname (2)
		ODRecordRef record = ODNodeCopyRecord(node, (CFStringRef) schema_get_value_at_index(values, 0),
											  (CFStringRef) schema_get_value_at_index(values, 1),
											  NULL, &error);
		if (record != NULL) {
			policy = ODRecordCopyPasswordPolicy(kCFAllocatorDefault, record, &error);
			DSCFRelease(record);
		} else {
			err_code = kODErrorRecordNoLongerExists;
		}
		
		if (error != NULL) {
			err_code = CFErrorGetCode(error);
			DSCFRelease(error);
		}
		
		result = schema_construct_result(CFSTR("ODRecordCopyPasswordPolicy"), 2, policy, NULL);
		DSCFRelease(policy);
	} else {
		err_code = kODErrorNodeInvalid;
	}
	
	_od_passthru_send_reply(reply, reqid, result, err_code, true);
	DSCFRelease(result);
}

static void 
_RecordVerifyPassword_async(CFTypeRef object, CFArrayRef values, mach_port_t reply, uint64_t reqid, pid_t pid)
{
	ODNodeRef		node		= (ODNodeRef) object;
	CFDictionaryRef result		= NULL;
	uint32_t		err_code	= 0;
	CFErrorRef		error		= NULL;
	
	if (object != NULL && CFGetTypeID(object) == ODNodeGetTypeID()) {
		// 9031793: ignore metarecordname (2)
		ODRecordRef record = ODNodeCopyRecord(node, (CFStringRef) schema_get_value_at_index(values, 0),
											  (CFStringRef) schema_get_value_at_index(values, 1),
											  NULL, &error);
		if (record != NULL) {
			bool success = ODRecordVerifyPassword(record, 
												  (CFStringRef) schema_get_value_at_index(values, 3), 
												  &error);
			result = schema_construct_result(CFSTR("ODRecordVerifyPassword"), 2, (success ? kCFBooleanTrue : kCFBooleanFalse), NULL);
			DSCFRelease(record);
		} else {
			err_code = kODErrorRecordNoLongerExists;
		}
		
		if (error != NULL) {
			err_code = CFErrorGetCode(error);
			DSCFRelease(error);
		}
	} else {
		err_code = kODErrorNodeInvalid;
	}
	
	_od_passthru_send_reply(reply, reqid, result, err_code, true);
	DSCFRelease(result);
}

static void 
_RecordVerifyPasswordExtended_async(CFTypeRef object, CFArrayRef values, mach_port_t reply, uint64_t reqid, pid_t pid)
{
	CFArrayRef		outAuthItems	= NULL;
	CFDataRef		contextRef		= NULL;
	ODNodeRef		node			= (ODNodeRef) object;
	CFDictionaryRef result			= NULL;
	uint32_t		err_code		= 0;
	CFErrorRef		error			= NULL;
	
	if (object != NULL && CFGetTypeID(object) == ODNodeGetTypeID()) {
		// 9031793: ignore metarecordname (2)
		ODRecordRef record = ODNodeCopyRecord(node, (CFStringRef) schema_get_value_at_index(values, 0),
											  (CFStringRef) schema_get_value_at_index(values, 1),
											  NULL, &error);
		if (record != NULL) {
			ODContextRef	outContext;
			ODContextRef	context;
			bool			success;

			context = outContext = (ODContextRef) _get_and_retain_od_ref(schema_get_value_at_index(values, 5), pid, &err_code,
																		 kODErrorContextInvalid);
			success = ODRecordVerifyPasswordExtended(record,
													 (ODAuthenticationType) schema_get_value_at_index(values, 3),
													 (CFArrayRef) schema_get_value_at_index(values, 4),
													 &outAuthItems,
													 &outContext,
													 &error);
			if (error != NULL) {
				err_code = CFErrorGetCode(error);
				DSCFRelease(error);
			}
			
			if (outContext != context) {
				contextRef = _add_od_ref(outContext, pid);
			}
			else if (context != NULL) {
				_remove_identifier(context, pid);
			}
			
			result = schema_construct_result(CFSTR("ODRecordVerifyPasswordExtended"), 4,
											 (success ? kCFBooleanTrue : kCFBooleanFalse), outAuthItems, contextRef, NULL);
			
			DSCFRelease(outAuthItems);
			DSCFRelease(contextRef);
			DSCFRelease(record);
		}
		else {
			err_code = kODErrorRecordNoLongerExists;
		}
		
		if (error != NULL) {
			err_code = CFErrorGetCode(error);
			DSCFRelease(error);
		}
	} else {
		err_code = kODErrorNodeInvalid;
	}
	
	_od_passthru_send_reply(reply, reqid, result, err_code, true);
	DSCFRelease(result);
}

static void 
_RecordChangePassword_async(CFTypeRef object, CFArrayRef values, mach_port_t reply, uint64_t reqid, pid_t pid)
{
	ODNodeRef		node		= (ODNodeRef) object;
	CFDictionaryRef result		= NULL;
	uint32_t		err_code	= 0;
	CFErrorRef		error		= NULL;
	
	if (object != NULL && CFGetTypeID(object) == ODNodeGetTypeID()) {
		// 9031793: ignore metarecordname (2)
		ODRecordRef record = ODNodeCopyRecord(node, (CFStringRef) schema_get_value_at_index(values, 0),
											  (CFStringRef) schema_get_value_at_index(values, 1),
											  NULL, &error);
		if (record != NULL) {
			bool success = ODRecordChangePassword(record,
												  (CFStringRef) schema_get_value_at_index(values, 3),
												  (CFStringRef) schema_get_value_at_index(values, 4),
												  &error);
			
			result = schema_construct_result(CFSTR("ODRecordChangePassword"), 2, (success ? kCFBooleanTrue : kCFBooleanFalse), NULL);
			DSCFRelease(record);
		} else {
			err_code = kODErrorRecordNoLongerExists;
		}
		
		if (error != NULL) {
			err_code = CFErrorGetCode(error);
			DSCFRelease(error);
		}
	} else {
		err_code = kODErrorNodeInvalid;
	}
	
	_od_passthru_send_reply(reply, reqid, result, err_code, true);
	DSCFRelease(result);
}

static void 
_RecordCopyValues_async(CFTypeRef object, CFArrayRef values, mach_port_t reply, uint64_t reqid, pid_t pid)
{
	ODNodeRef		node		= (ODNodeRef) object;
	CFDictionaryRef result		= NULL;
	uint32_t		err_code	= 0;
	CFErrorRef		error		= NULL;
	
	if (object != NULL && CFGetTypeID(object) == ODNodeGetTypeID()) {
		ODRecordRef record = ODNodeCopyRecord(node, (CFStringRef) schema_get_value_at_index(values, 0),
											  (CFStringRef) schema_get_value_at_index(values, 1),
											  NULL, &error);
		if (record != NULL) {
			CFArrayRef attribs = ODRecordCopyValues(record, (ODAttributeType) schema_get_value_at_index(values, 2), &error);
			
			result = schema_construct_result(CFSTR("ODRecordCopyValues"), 2, attribs, NULL);
			DSCFRelease(attribs);
			DSCFRelease(record);
		} else {
			err_code = kODErrorRecordNoLongerExists;
		}
		
		if (error != NULL) {
			err_code = CFErrorGetCode(error);
			DSCFRelease(error);
		}
	} else {
		err_code = kODErrorNodeInvalid;
	}
	
	_od_passthru_send_reply(reply, reqid, result, err_code, true);
	DSCFRelease(result);
}

static void 
_RecordSetValue_async(CFTypeRef object, CFArrayRef values, mach_port_t reply, uint64_t reqid, pid_t pid)
{
	ODNodeRef		node		= (ODNodeRef) object;
	CFDictionaryRef result		= NULL;
	uint32_t		err_code	= 0;
	CFErrorRef		error		= NULL;
	
	if (object != NULL && CFGetTypeID(object) == ODNodeGetTypeID()) {
		// 9031793: ignore metarecordname (2)
		ODRecordRef record = ODNodeCopyRecord(node, (CFStringRef) schema_get_value_at_index(values, 0),
											  (CFStringRef) schema_get_value_at_index(values, 1),
											  NULL, &error);
		if (record != NULL) {
			bool success = ODRecordSetValue(record, 
											(ODAttributeType) schema_get_value_at_index(values, 3),
											schema_get_value_at_index(values, 4),
											&error);
			
			result = schema_construct_result(CFSTR("ODRecordSetValue"), 2, (success ? kCFBooleanTrue : kCFBooleanFalse), NULL);
			DSCFRelease(record);
		} else {
			err_code = kODErrorRecordNoLongerExists;
		}
		
		if (error != NULL) {
			err_code = CFErrorGetCode(error);
			DSCFRelease(error);
		}
	} else {
		err_code = kODErrorNodeInvalid;
	}
	
	_od_passthru_send_reply(reply, reqid, result, err_code, true);
	DSCFRelease(result);
}

static void 
_RecordAddValue_async(CFTypeRef object, CFArrayRef values, mach_port_t reply, uint64_t reqid, pid_t pid)
{
	ODNodeRef		node		= (ODNodeRef) object;
	CFDictionaryRef result		= NULL;
	uint32_t		err_code	= 0;
	CFErrorRef		error;

	if (object != NULL && CFGetTypeID(object) == ODNodeGetTypeID()) {
		// 9031793: ignore metarecordname (2)
		ODRecordRef record = ODNodeCopyRecord(node, (CFStringRef) schema_get_value_at_index(values, 0),
											  (CFStringRef) schema_get_value_at_index(values, 1),
											  NULL, &error);
		if (record != NULL) {
			bool success = ODRecordAddValue(record, 
											(ODAttributeType) schema_get_value_at_index(values, 3),
											schema_get_value_at_index(values, 4),
											&error);
			
			result = schema_construct_result(CFSTR("ODRecordAddValue"), 2, (success ? kCFBooleanTrue : kCFBooleanFalse), NULL);
			DSCFRelease(record);
		} else {
			err_code = kODErrorRecordNoLongerExists;
		}
		
		if (error != NULL) {
			err_code = CFErrorGetCode(error);
			DSCFRelease(error);
		}
	} else {
		err_code = kODErrorNodeInvalid;
	}
	
	_od_passthru_send_reply(reply, reqid, result, err_code, true);
	DSCFRelease(result);
}

static void 
_RecordRemoveValue_async(CFTypeRef object, CFArrayRef values, mach_port_t reply, uint64_t reqid, pid_t pid)
{
	ODNodeRef		node		= (ODNodeRef) object;
	CFDictionaryRef result		= NULL;
	uint32_t		err_code	= 0;
	CFErrorRef		error		= NULL;
	
	if (object != NULL && CFGetTypeID(object) == ODNodeGetTypeID()) {
		// 9031793: ignore metarecordname (2)
		ODRecordRef record = ODNodeCopyRecord(node, (CFStringRef) schema_get_value_at_index(values, 0),
											  (CFStringRef) schema_get_value_at_index(values, 1),
											  NULL, &error);
		if (record != NULL) {
			bool success = ODRecordRemoveValue(record, 
											   (ODAttributeType) schema_get_value_at_index(values, 3),
											   schema_get_value_at_index(values, 4),
											   &error);
			
			result = schema_construct_result(CFSTR("ODRecordRemoveValue"), 2, (success ? kCFBooleanTrue : kCFBooleanFalse), NULL);
			DSCFRelease(record);
		} else {
			err_code = kODErrorRecordNoLongerExists;
		}
		
		if (error != NULL) {
			err_code = CFErrorGetCode(error);
			DSCFRelease(error);
		}
	} else {
		err_code = kODErrorNodeInvalid;
	}
	
	_od_passthru_send_reply(reply, reqid, result, err_code, true);
	DSCFRelease(result);
}

static void 
_RecordDelete_async(CFTypeRef object, CFArrayRef values, mach_port_t reply, uint64_t reqid, pid_t pid)
{
	ODNodeRef		node		= (ODNodeRef) object;
	CFDictionaryRef result		= NULL;
	uint32_t		err_code	= 0;
	CFErrorRef		error		= NULL;
	
	if (object != NULL && CFGetTypeID(object) == ODNodeGetTypeID()) {
		// 9031793: ignore metarecordname (2)
		ODRecordRef record = ODNodeCopyRecord(node, (CFStringRef) schema_get_value_at_index(values, 0),
											  (CFStringRef) schema_get_value_at_index(values, 1),
											  NULL, &error);
		if (record != NULL) {
			bool success = ODRecordDelete(record, &error);
			
			result = schema_construct_result(CFSTR("ODRecordDelete"), 2, (success ? kCFBooleanTrue : kCFBooleanFalse), NULL);
			DSCFRelease(record);
		} else {
			err_code = kODErrorRecordNoLongerExists;
		}
		
		if (error != NULL) {
			err_code = CFErrorGetCode(error);
			DSCFRelease(error);
		}
	} else {
		err_code = kODErrorNodeInvalid;
	}
	
	_od_passthru_send_reply(reply, reqid, result, 0, true);
	DSCFRelease(result);
}

static void
_ContextRelease(CFTypeRef object, CFArrayRef values, mach_port_t reply, uint64_t reqid, pid_t pid)
{
	uint32_t	err_code = kODErrorContextInvalid;
	
	if (_remove_identifier(schema_get_value_at_index(values, 0), pid) == true) {
		err_code = 0;
	}
	
	_od_passthru_send_reply(reply, reqid, NULL, err_code, true);
}

static void *
_get_function_from_schema_cb(long index, bool async)
{
	static void *callbacks[][2] = {
		/* 0 */ { NULL, NULL },
		/* 1 */ { (void *)_SessionCreate, NULL },
		/* 2 */ { NULL, (void *)_SessionCopyNodeNames_async },
		/* 3 */ { (void *)_NodeCreateWithName, (void *)_NodeCreateWithName_async },
		/* 4 */ { NULL, (void *)_NodeCopySubnodeNames_async },
		/* 5 */ { NULL, (void *)_NodeCopyUnreachableSubnodeNames_async },
		/* 6 */ { NULL, (void *)_NodeCopyDetails_async },
		/* 7 */ { NULL, (void *)_NodeCopySupportedRecordTypes_async },
		/* 8 */ { NULL, (void *)_NodeCopySupportedAttributes_async },
		/* 9 */ { NULL, (void *)_NodeSetCredentials_async },
		/* 10 */ { NULL, (void *)_NodeSetCredentialsExtended_async },
		/* 11 */ { NULL, (void *)_NodeCreateRecord_async },
		/* 12 */ { NULL, (void *)_NodeCopyRecord_async },
		/* 13 */ { NULL, (void *)_NodeCustomCall_async },
		/* 14 */ { NULL, (void *)_NodeSetNodeCredentials_async },
		/* 15 */ { NULL, (void *)_QueryCreateWithNode_async },
		/* 16 */ { NULL, (void *)_QuerySynchronize_async },
		/* 17 */ { NULL, (void *)_QueryCancel_async },
		/* 18 */ { NULL, (void *)_RecordCopyPasswordPolicy_async },
		/* 19 */ { NULL, (void *)_RecordVerifyPassword_async },
		/* 20 */ { NULL, (void *)_RecordVerifyPasswordExtended_async },
		/* 21 */ { NULL, (void *)_RecordChangePassword_async },
		/* 22 */ { NULL, (void *)_RecordCopyValues_async },
		/* 23 */ { NULL, (void *)_RecordSetValue_async },
		/* 24 */ { NULL, (void *)_RecordAddValue_async },
		/* 25 */ { NULL, (void *)_RecordRemoveValue_async },
		/* 26 */ { NULL, (void *)_RecordDelete_async },
		/* 27 */ { (void *)_QueryCancel, NULL },
		/* 28 */ { NULL, NULL },
		/* 29 */ { NULL, (void *)_ContextRelease },
		/* 30 */ { NULL, (void *)_NodeVerifyCredentialsExtended_async },
	};
	
	return callbacks[index][async];
}

static void
_wire_schema_functions(void *context)
{
	// these line up with _get_function_from_schema_cb
	schema_set_callback(CFSTR("ODSessionCreate"), (void *) 1);
	schema_set_callback(CFSTR("ODSessionCopyNodeNames"), (void *) 2);
	schema_set_callback(CFSTR("ODNodeCreateWithName"), (void *) 3);
	schema_set_callback(CFSTR("ODNodeCopySubnodeNames"), (void *) 4);
	schema_set_callback(CFSTR("ODNodeCopyUnreachableSubnodeNames"), (void *) 5);
	schema_set_callback(CFSTR("ODNodeCopyDetails"), (void *) 6);
	schema_set_callback(CFSTR("ODNodeCopySupportedRecordTypes"), (void *) 7);
	schema_set_callback(CFSTR("ODNodeCopySupportedAttributes"), (void *) 8);
	schema_set_callback(CFSTR("ODNodeSetCredentials"), (void *) 9);
	schema_set_callback(CFSTR("ODNodeSetCredentialsExtended"), (void *) 10);
	schema_set_callback(CFSTR("ODNodeCreateRecord"), (void *) 11);
	schema_set_callback(CFSTR("ODNodeCopyRecord"), (void *) 12);
	schema_set_callback(CFSTR("ODNodeCustomCall"), (void *) 13);
	schema_set_callback(CFSTR("ODNodeSetNodeCredentials"), (void *) 14);
	schema_set_callback(CFSTR("ODQueryCreateWithNode"), (void *) 15);
	schema_set_callback(CFSTR("ODQuerySynchronize"), (void *) 16);
	schema_set_callback(CFSTR("ODQueryCancel"), (void *) 17);
	schema_set_callback(CFSTR("ODRecordCopyPasswordPolicy"), (void *) 18);
	schema_set_callback(CFSTR("ODRecordVerifyPassword"), (void *) 19);
	schema_set_callback(CFSTR("ODRecordVerifyPasswordExtended"), (void *) 20);
	schema_set_callback(CFSTR("ODRecordChangePassword"), (void *) 21);
	schema_set_callback(CFSTR("ODRecordCopyValues"), (void *) 22);
	schema_set_callback(CFSTR("ODRecordSetValue"), (void *) 23);
	schema_set_callback(CFSTR("ODRecordAddValue"), (void *) 24);
	schema_set_callback(CFSTR("ODRecordRemoveValue"), (void *) 25);
	schema_set_callback(CFSTR("ODRecordDelete"), (void *) 26);
	schema_set_callback(CFSTR("ODQueryCancel"), (void *) 27);
	schema_set_callback(CFSTR("ODNodeRelease"), (void *) 28);
	schema_set_callback(CFSTR("ODContextRelease"), (void *) 29);
	schema_set_callback(CFSTR("ODNodeVerifyCredentialsExtended"), (void *) 30);
}

static boolean_t
_demux_reply(mach_msg_header_t *request, mach_msg_header_t *reply)
{
	boolean_t		result	= false;
	mach_msg_id_t	msgid	= request->msgh_id;
	
	if (msgid >= 7000 && msgid < 8000) {
		result = legacy_call_server(request, reply);
	}
	else if (msgid >= 8000 && msgid < 9000) {
		result = extmodule_call_server(request, reply);
	}
	else if (msgid >= 9000 && msgid < 10000) {
		result = session_call_server(request, reply);
	}
	
	return result;
}

static inline int32_t
_max_mach_msgsize(void)
{
	size_t legacy_size = sizeof(union __RequestUnion__receive_legacy_call_subsystem);
	size_t extmodule_size = sizeof(union __RequestUnion__receive_extmodule_call_subsystem);
	size_t session_size = sizeof(union __RequestUnion__receive_session_call_subsystem);
	
	return MAX(MAX(legacy_size, extmodule_size), session_size) + MAX_TRAILER_SIZE;
}

#pragma mark
#pragma mark MIG callbacks

__private_extern__ kern_return_t
receive_session_process_request(mach_port_t port, mach_port_t reply, uuid_t session_uuid, uuid_t node_uuid, uint64_t reqid, uint32_t proxy_uid, 
								vm_offset_t data, mach_msg_type_number_t dataCnt, audit_token_t token)
{
	kern_return_t			kr = KERN_FAILURE;
	CFMutableDictionaryRef	plist;
	CFPropertyListFormat	format;
	CFDataRef				cfData;
	CFErrorRef				error;
	pid_t					pid;
	uid_t					ruid;
	uid_t					euid;
	
	CInternalDispatch::AddCapability();

	audit_token_to_au32(token, NULL, &euid, NULL, &ruid, NULL, &pid, NULL, NULL);
	if (ruid == 0 && euid == 0) {
#ifdef __LP64__
		pthread_setspecific(_od_passthru_thread_key(), (void *) reqid);
#else
		uint64_t *threadkey = (uint64_t *) malloc(sizeof(reqid));
		(*threadkey) = reqid;
		pthread_setspecific(_od_passthru_thread_key(), threadkey);
#endif
		pthread_setspecific(_od_passthru_session_threadid(), (void *) 1);
		
		DbgLog(kLogInfo, "Received a session request from opendirectoryd");
		
		if (data != 0 && dataCnt != 0) {
			cfData = CFDataCreateWithBytesNoCopy(NULL, (const UInt8*)data, dataCnt, kCFAllocatorNull);
			plist = (CFMutableDictionaryRef) CFPropertyListCreateWithData(NULL, cfData, kCFPropertyListMutableContainers, &format, &error);
			
			if (format == kCFPropertyListBinaryFormat_v1_0) {
				if (CFDictionaryGetTypeID() == CFGetTypeID(plist)) {
					CFTypeRef	object		= NULL;
					CFDataRef	objectid	= NULL;
					uint32_t	err_code	= 0;
					CFArrayRef	values;
					long		callback;
					async_fn	function;
					
					schema_deconstruct_request(plist, NULL, &values, (void **) &callback);
					switch (callback) {
						case 0:
							DbgLog(kLogWarning, "No function for request from opendirectoryd");
							err_code = kODErrorDaemonError;
							break;
							
						case 1:
							break;
							
						case 2:
						case 3:
							// session related calls
							objectid = CFDataCreateWithBytesNoCopy(kCFAllocatorDefault, session_uuid, sizeof(session_uuid),
																   kCFAllocatorNull);
							object = _get_and_retain_od_ref(objectid, pid, &err_code, kODErrorSessionInvalid);
							if (object != NULL && CFGetTypeID(object) != ODSessionGetTypeID()) {
								err_code = kODErrorSessionInvalid;
							}
							break;
							
						default:
							objectid = CFDataCreateWithBytesNoCopy(kCFAllocatorDefault, node_uuid, sizeof(node_uuid),
																   kCFAllocatorNull);
							object = _get_and_retain_od_ref(objectid, pid, &err_code, kODErrorNodeInvalid);
							if (object != NULL && CFGetTypeID(object) != ODNodeGetTypeID()) {
								err_code = kODErrorNodeInvalid;
							}
							break;
					}
					
					if (err_code == 0) {
						switch (callback) {
							case 28: // ODNodeRelease is special cased
								objectid = CFDataCreateWithBytesNoCopy(kCFAllocatorDefault, node_uuid, sizeof(node_uuid),
																	   kCFAllocatorNull);
								if (_remove_identifier(objectid, pid) == true) {
									_od_passthru_send_reply(reply, reqid, NULL, 0, true);
								} else {
									err_code = kODErrorNodeInvalid;
								}
								break;
								
							default:
								function = (async_fn) _get_function_from_schema_cb(callback, true);
								if (function != NULL) {
									pthread_setspecific(_od_passthru_uid_key(), (void *)proxy_uid);
									function(object, values, reply, reqid, pid);
									pthread_setspecific(_od_passthru_uid_key(), NULL);
								}
								else {
									err_code = kODErrorDaemonError;
								}
								break;
						}
					}
					
					if (err_code != 0) {
						_od_passthru_send_reply(reply, reqid, NULL, err_code, true);
					}
					
					DSCFRelease(object);
					DSCFRelease(objectid);
					DSCFRelease(values);
					
					kr = KERN_SUCCESS;
				}
				else {
					DbgLog(kLogWarning, "got a non-dictionary format for the request");
				}
			}
			else {
				DbgLog(kLogWarning, "got invalid request via mach format was %d", format);
			}
			
			DSCFRelease(cfData);
			DSCFRelease(plist);
		}
		
		pthread_setspecific(_od_passthru_thread_key(), NULL);
		pthread_setspecific(_od_passthru_session_threadid(), NULL);
	}
	
	if (dataCnt != 0) {
		vm_deallocate(mach_task_self(), data, dataCnt);
	}
	
	return kr;
}

__private_extern__ kern_return_t
receive_session_create(mach_port_t port, mach_port_t replyPort, vm_offset_t data, mach_msg_type_number_t dataCnt,
					   vm_offset_t *nodenames, mach_msg_type_number_t *nodenamesCnt, uuid_t session_uuid, uint32_t *error,
					   audit_token_t *token)
{
	kern_return_t			kr		= KERN_FAILURE;
	CFDictionaryRef			plist;
	CFPropertyListFormat	format;
	CFDataRef				cfData;
	pid_t					pid;
	uid_t					euid;
	uid_t					uid;
	
	dispatch_once_f(&wire_once, NULL, _wire_schema_functions);

	CInternalDispatch::AddCapability();

	pthread_setspecific(_od_passthru_session_threadid(), (void *) 1);

	audit_token_to_au32(*token, NULL, &euid, NULL, &uid, NULL, &pid, NULL, NULL);
	if (uid == 0 && euid == 0) {
		DbgLog(kLogInfo, "Received an session request from opendirectoryd");
		
		if (data != 0 && dataCnt != 0) {
			cfData = CFDataCreateWithBytesNoCopy(NULL, (const UInt8*)data, dataCnt, kCFAllocatorNull);
			plist = (CFDictionaryRef) CFPropertyListCreateWithData(NULL, cfData, kCFPropertyListImmutable, &format, NULL);
			
			DSCFRelease(cfData);
			if (format == kCFPropertyListBinaryFormat_v1_0) {
				if (CFDictionaryGetTypeID() == CFGetTypeID(plist)) {
					__block ODSessionRef	session		= NULL;
					__block CFErrorRef		errorRef	= NULL;
					
					CFStringRef path = (CFStringRef) CFDictionaryGetValue(plist, CFSTR("LocalPath"));
					if (path != NULL) {
						dispatch_sync(_get_localonly_session_queue(), 
									  ^(void) {
										  CInternalDispatch::AddCapability();
										  
										  pthread_setspecific(_od_passthru_session_threadid(), (void *) 1);
										  
										  // check path again localonlyPath
										  if (localonlyPath != NULL) {
											  if (CFEqual(path, localonlyPath) == false) {
												  (*error) = kODErrorSessionDaemonRefused; // session refused
												  pthread_setspecific(_od_passthru_session_threadid(), NULL);
												  return;
											  }
										  } else {
											  localonlyPath = (CFStringRef) CFDictionaryGetValue(plist, CFSTR("LocalPath"));
											  CFRetain(localonlyPath);
										  }
										  
										  session = ODSessionCreate(kCFAllocatorDefault, plist, &errorRef);
										  if (session != NULL) {
											  DbgLog(kLogInfo, "New localonly session");
											  localOnlyCount++;
										  }
										  
										  pthread_setspecific(_od_passthru_session_threadid(), NULL);
									  });
						DbgLog(kLogNotice, "Received request from opendirectoryd PID %d (localonly request)", pid);
						SrvrLog(kLogApplication, "Received request from opendirectoryd PID %d (localonly request)", pid);
					}
					else {
						session = ODSessionCreate(kCFAllocatorDefault, plist, &errorRef);
						DbgLog(kLogNotice, "Received request from opendirectoryd PID %d (proxy request)", pid);
						SrvrLog(kLogApplication, "Received request from opendirectoryd PID %d (proxy request)", pid);
					}
					
					if (session != NULL) {
						CFDataRef sessionID = _add_od_ref(session, pid);
						
						uuid_copy(session_uuid, CFDataGetBytePtr(sessionID));
						dispatch_async(_get_passthru_queue(), 
									   ^(void) {
										   if (odd_port != MACH_PORT_NULL) {
											   mach_port_mod_refs(mach_task_self(), odd_port, MACH_PORT_RIGHT_SEND, -1);
										   }
										   
										   odd_port = replyPort;
									   });
					} else if (errorRef != NULL) {
						(*error) = CFErrorGetCode(errorRef);
						DSCFRelease(errorRef);
					}
					
					kr = KERN_SUCCESS;
				}
			}
			
			DSCFRelease(plist);
		}		
	}
	
	if (dataCnt != 0) {
		vm_deallocate(mach_task_self(), data, dataCnt);
	}
	
	pthread_setspecific(_od_passthru_session_threadid(), NULL);
	
	return kr;
}

__private_extern__ kern_return_t
receive_session_logging_enable(mach_port_t port, boolean_t enabled, int32_t maxlevel, audit_token_t token)
{
	return receive_extmodule_logging_enable(port, enabled, maxlevel, token);
}

__private_extern__ kern_return_t
receive_session_close(mach_port_t port, uuid_t session_uuid, audit_token_t token)
{
	dispatch_sync(_get_localonly_session_queue(), 
				  ^(void) {
					  pid_t			pid;
					  uid_t			euid;
					  uid_t			uid;
					  uint32_t		err_code;

					  CInternalDispatch::AddCapability();

					  pthread_setspecific(_od_passthru_session_threadid(), (void *) 1);

					  audit_token_to_au32(token, NULL, &euid, NULL, &uid, NULL, &pid, NULL, NULL);
					  if (uid == 0 && euid == 0) {
						  CFDataRef sessionID = CFDataCreateWithBytesNoCopy(kCFAllocatorDefault, session_uuid, sizeof(session_uuid), 
																			kCFAllocatorNull);
						  if (_remove_identifier(sessionID, pid) == true && gDSLocalOnlyMode == true) {
							  localOnlyCount--;
							  if (localOnlyCount == 0) {
								  // delete the rest of the pids while we are here
								  _delete_refs_for_pid(pid);
								  
								  DSCFRelease(localonlyPath);
								  
								  od_passthru_localonly_exit();
							  }
						  }
						  
						  DSCFRelease(sessionID);
					  }
					  
					  pthread_setspecific(_od_passthru_session_threadid(), NULL);
				  });

	return KERN_SUCCESS;
}

#pragma mark -
#pragma mark legacy launch

__private_extern__ kern_return_t
receive_legacy_launch(mach_port_t port, mach_port_t replyport, audit_token_t *token)
{
	pid_t pid;
	uid_t uid;
	uid_t euid;
	
	dispatch_once_f(&wire_once, NULL, _wire_schema_functions);
	
	CInternalDispatch::AddCapability();

	audit_token_to_au32(*token, NULL, &euid, NULL, &uid, NULL, &pid, NULL, NULL);
	
	if (uid == 0 && euid == 0) {
		dispatch_queue_t concurrentq = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0);
		dispatch_queue_t queue = _get_passthru_queue();

#if defined(DISABLE_SEARCH_PLUGIN) && (!defined(DISABLE_BSD_PLUGIN) || !defined(DISABLE_LOCAL_PLUGIN))
		/*
		 * Open these internally until they are disabled to ensure they stay open
		 */
		tDirReference dsRef = 0;
		tDirNodeReference nodeRef = 0;
		tDataListPtr nodeName;

		dsOpenDirService(&dsRef); // don't close

#ifndef DISABLE_LOCAL_PLUGIN
		gNodeList->WaitForLocalNode();
		
        nodeName = dsBuildFromPathPriv("/Local/Default", "/");
        dsOpenDirNode(dsRef, nodeName, &nodeRef);

        dsDataListDeallocatePriv(nodeName);
        free(nodeName);
#endif

#ifndef DISABLE_BSD_PLUGIN
		gNodeList->WaitForBSDNode();
		
        nodeName = dsBuildFromPathPriv("/BSD/local", "/");
        dsOpenDirNode(dsRef, nodeName, &nodeRef);

        dsDataListDeallocatePriv(nodeName);
        free(nodeName);
#endif
		
#endif
		
		dispatch_sync(queue, 
					  ^(void) {
						  if (odd_port != MACH_PORT_NULL) {
							  mach_port_mod_refs(mach_task_self(), odd_port, MACH_PORT_RIGHT_SEND, -1);
						  }
						  
						  odd_port = replyport;
					  });
		
		DbgLog(kLogNotice, "Received request from opendirectoryd PID %d (launch and/or register legacy nodes)", pid);
		SrvrLog(kLogApplication, "Received request from opendirectoryd PID %d (launch and/or register legacy nodes)", pid);
		/* create a source to cleanup all references tied to the PID */
		dispatch_source_t source = dispatch_source_create(DISPATCH_SOURCE_TYPE_PROC, pid, DISPATCH_PROC_EXEC | DISPATCH_PROC_EXIT,
														  concurrentq);
		dispatch_source_set_event_handler(source, 
										  ^(void) {
											  DbgLog(kLogNotice,
													 "Lost connection to opendirectoryd PID %d, cleaning up all references",
													 pid);
											  SrvrLog(kLogApplication, 
													  "Lost connection to opendirectoryd PID %d, cleaning up all references",
													  pid);
											  _delete_refs_for_pid(pid);
											  dispatch_source_cancel(source);
											  
											  od_passthru_localonly_exit();
										  });
		dispatch_source_set_cancel_handler(source,
										   ^(void) {
											   dispatch_release(source);
										   });
		dispatch_resume(source);

#ifndef DISABLE_CACHE_PLUGIN
		dispatch_async(concurrentq, ^(void) {
			CInternalDispatch::AddCapability();
			
			// we need to prevent libinfo (getpw, getgr, etc.) calls from causing a circular deadlock so we tell cache to ignore requests from
			// opendirectoryd (slapd->odd->ds->slapd->odd, or odd->ds->kerberos->odd)
			ODNodeRef node = ODNodeCreateWithName(NULL, NULL, CFSTR("/Cache"), NULL);
			if (node != NULL) {
				CFMutableDataRef sendData = (CFMutableDataRef) CFDataCreateMutable (kCFAllocatorDefault, 0);
				CFDataRef recvData;
				AuthorizationExternalForm blankExtForm;
				
				bzero(&blankExtForm, sizeof(AuthorizationExternalForm) );
				
				CFDataAppendBytes(sendData, (const UInt8 *)&blankExtForm, sizeof(AuthorizationExternalForm));
				CFDataAppendBytes(sendData, (const UInt8 *)&pid, sizeof(pid_t));
				
				recvData = ODNodeCustomCall(node, eDSCustomCallCacheRegisterLocalSearchPID, sendData, NULL);
				
				DSCFRelease(recvData);
				DSCFRelease(sendData);
				DSCFRelease(node);
			}
		});
#endif
		
		// do after we return to the client to prevent race conditions
		dispatch_after(dispatch_time(0, 1ull * NSEC_PER_SEC), concurrentq, ^(void) {
			gNodeList->RegisterAll();
		});
	}
	
	return KERN_SUCCESS;
}

#pragma mark -
#pragma mark extmodule routines

__private_extern__ kern_return_t
receive_extmodule_process_request(mach_port_t port, mach_port_t reply, uuid_t connection_uuid, uuid_t module_uuid, uint64_t reqid,
								  uint32_t proxy_uid, vm_offset_t data, mach_msg_type_number_t dataCnt, audit_token_t token)
{
	kern_return_t			kr = KERN_FAILURE;
	CFMutableDictionaryRef	plist;
	CFPropertyListFormat	format;
	CFDataRef				cfData;
	CFErrorRef				error;
	pid_t					pid;
	uid_t					ruid;
	uid_t					euid;
	
	audit_token_to_au32(token, NULL, &euid, NULL, &ruid, NULL, &pid, NULL, NULL);
	if (ruid == 0 && euid == 0) {
#ifdef __LP64__
		pthread_setspecific(_od_passthru_thread_key(), (void *) reqid);
#else
		uint64_t *threadkey = (uint64_t *) malloc(sizeof(reqid));
		(*threadkey) = reqid;
		pthread_setspecific(_od_passthru_thread_key(), threadkey);
#endif
		
		DbgLog(kLogInfo, "Received an asynchronous request from opendirectoryd");
		
		if (data != 0 && dataCnt != 0) {
			cfData = CFDataCreateWithBytesNoCopy(NULL, (const UInt8*)data, dataCnt, kCFAllocatorNull);
			plist = (CFMutableDictionaryRef) CFPropertyListCreateWithData(NULL, cfData, kCFPropertyListMutableContainers, &format, &error);
			
			if (format == kCFPropertyListBinaryFormat_v1_0) {
				if (CFDictionaryGetTypeID() == CFGetTypeID(plist)) {
					ODNodeRef	node		= NULL;
					CFDataRef	nodeid		= NULL;
					uint32_t	err_code	= 0;
					CFArrayRef	values		= NULL;
					async_fn	function;
					long		callback;
										
					nodeid = CFDataCreateWithBytesNoCopy(kCFAllocatorDefault, connection_uuid, sizeof(connection_uuid),
														 kCFAllocatorNull);
					
					node = (ODNodeRef) _get_and_retain_od_ref(nodeid, pid, &err_code, kODErrorNodeInvalid);
					if (node != NULL && CFGetTypeID(node) != ODNodeGetTypeID()) {
						err_code = kODErrorNodeInvalid;
					}
					
					if (err_code == 0) {
						schema_deconstruct_request(plist, NULL, &values, (void **) &callback);
						switch (callback) {
							case 0:
								DbgLog(kLogWarning, "No function for request from opendirectoryd");
								err_code = kODErrorDaemonError;
								break;
								
							case 28: // ODNodeRelease is special cased
								if (_remove_identifier(nodeid, pid) == true) {
									_od_passthru_send_reply(reply, reqid, NULL, 0, true);
								}
								else {
									err_code = kODErrorNodeInvalid;
								}
								break;
								
							default:
								function = (async_fn) _get_function_from_schema_cb(callback, true);
								if (function != NULL) {
									pthread_setspecific(_od_passthru_uid_key(), (void *)proxy_uid);
									function(node, values, reply, reqid, pid);
									pthread_setspecific(_od_passthru_uid_key(), NULL);
								}
								else {
									DbgLog(kLogWarning, "No function for request from opendirectoryd");
									err_code = kODErrorDaemonError;
								}
								break;
						}
					}
					
					if (err_code != 0) {
						_od_passthru_send_reply(reply, reqid, NULL, err_code, true);
					}
					
					DSCFRelease(node);
					DSCFRelease(nodeid);
					DSCFRelease(values);
					
					kr = KERN_SUCCESS;
				}
				else {
					DbgLog(kLogWarning, "got a non-dictionary format for the request");
				}
			}
			else {
				DbgLog(kLogWarning, "got invalid request via mach format was %d", format);
			}
							
		fail:
			
			DSCFRelease(cfData);
			DSCFRelease(plist);
		}
		
		pthread_setspecific(_od_passthru_thread_key(), NULL);
	}
	
	if (dataCnt != 0) {
		vm_deallocate(mach_task_self(), data, dataCnt);
	}

	return kr;
}

__private_extern__ kern_return_t
receive_extmodule_new_connection(mach_port_t port, vm_offset_t data, mach_msg_type_number_t dataCnt, uint64_t reqid,
								 uint32_t proxy_uid, uuid_t connection_uuid, uint32_t *err_code, audit_token_t *token)
{
	kern_return_t			kr = KERN_FAILURE;
	CFDictionaryRef			plist;
	CFPropertyListFormat	format;
	CFDataRef				cfData;
	CFErrorRef				error;
	pid_t					pid;
	uid_t					ruid;
	uid_t					euid;
	
	audit_token_to_au32(*token, NULL, &euid, NULL, &ruid, NULL, &pid, NULL, NULL);
	if (ruid == 0 && euid == 0) {
#ifdef __LP64__
		pthread_setspecific(_od_passthru_thread_key(), (void *) reqid);
#else
		uint64_t *threadkey = (uint64_t *) malloc(sizeof(reqid));
		(*threadkey) = reqid;
		pthread_setspecific(_od_passthru_thread_key(), threadkey);
#endif
		
		DbgLog(kLogInfo, "Received a synchronous request from opendirectoryd");
		
		if (data != 0 && dataCnt != 0) {
			cfData = CFDataCreateWithBytesNoCopy(NULL, (const UInt8 *)data, dataCnt, kCFAllocatorNull);
			plist = (CFDictionaryRef) CFPropertyListCreateWithData(NULL, cfData, kCFPropertyListImmutable, &format, &error);
			
			if (format == kCFPropertyListBinaryFormat_v1_0) {
				if (CFDictionaryGetTypeID() == CFGetTypeID(plist)) {
					CFStringRef	funcname;
					CFArrayRef	values = NULL;
					long		callback;
					sync_fn		function;
					
					schema_deconstruct_request(plist, NULL, &values, (void **)&callback);
					switch (callback) {
						case 0:
							break;
							
						case 3:
							function = (sync_fn) _get_function_from_schema_cb(callback, false);
							if (function != NULL) {
								CFDictionaryRef result;
								
								pthread_setspecific(_od_passthru_uid_key(), (void *)proxy_uid);
								result = function(NULL, values, pid, err_code);
								pthread_setspecific(_od_passthru_uid_key(), NULL);
								
								// here we use the node id as a UUID
								if (result != NULL) {
									CFArrayRef values2 = NULL;
									CFDataRef tempID;
									
									schema_deconstruct_result(result, NULL, &values2);
									tempID = (CFDataRef) schema_get_value_at_index(values2, 0);
									if (CFDataGetTypeID() == CFGetTypeID(tempID)) {
										uuid_copy(connection_uuid, CFDataGetBytePtr(tempID));
									}
									
									DSCFRelease(values2);
									DSCFRelease(result);
								}
								
								kr = KERN_SUCCESS;
							}							
							break;
							
						default:
							// TODO: do we ever allow sync calls
							break;
					}

					DSCFRelease(values);
					
					if (kr == KERN_FAILURE) {
						DbgLog(kLogWarning, "No function for request from opendirectoryd");
					}
				}
				else {
					DbgLog(kLogWarning, "got a non-dictionary format for the request");
				}
			}
			else {
				DbgLog(kLogWarning, "got invalid request via mach format was %d", format);
			}
			
			DSCFRelease(cfData);
			DSCFRelease(plist);
		}
		
		pthread_setspecific(_od_passthru_thread_key(), NULL);
	}
	
	if (dataCnt != 0) {
		vm_deallocate(mach_task_self(), data, dataCnt);
	}
	
	return kr;
}

__private_extern__ kern_return_t
receive_extmodule_locate(mach_port_t port, vm_offset_t data, mach_msg_type_number_t dataCnt, mach_port_t reply,
						 uint64_t reqid, audit_token_t token)
{
	if (dataCnt != 0) {
		vm_deallocate(mach_task_self(), data, dataCnt);
	}
	
	return KERN_SUCCESS;
}

__private_extern__ kern_return_t
receive_extmodule_load_module(mach_port_t port, mach_port_t reply, caddr_t modulepath, caddr_t name, uuid_t module_uuid,
							  audit_token_t token)
{
	return KERN_SUCCESS;
}

__private_extern__ kern_return_t
receive_extmodule_authenticate(mach_port_t port, vm_offset_t data, mach_msg_type_number_t dataCnt, mach_port_t reply, 
							   uint64_t reqid, uint32_t proxy_uid, audit_token_t token)
{
	if (dataCnt != 0) {
		vm_deallocate(mach_task_self(), data, dataCnt);
	}
	
	return KERN_SUCCESS;
}

__private_extern__ kern_return_t
receive_extmodule_logging_enable(mach_port_t port, boolean_t enabled, int32_t maxlevel, audit_token_t token)
{
	uid_t euid;
	
	audit_token_to_au32(token, NULL, &euid, NULL, NULL, NULL, NULL, NULL, NULL);
	if (euid == 0) {
		CLog::SetLoggingPriority(keDebugLog, maxlevel);
		if (enabled == true) {
			CLog::StartDebugLog();
		}
		else {
			CLog::StopDebugLog();
		}
		
		odd_logging_enabled = enabled;
		gDebugLogging = enabled;
	}
	
	return KERN_SUCCESS;
}

#pragma mark
#pragma mark External functions

bool
od_passthru_log_message(int32_t level, const char *message)
{
	uint64_t reqid = 0;
	int32_t new_level = 5;
	
	if (odd_logging_enabled == false) {
		return false;
	}
	
	if ((level & kLogEmergency) != 0) {
		new_level = 0;
	}
	else if ((level & kLogAlert) != 0) {
		new_level = 1;
	}
	else if ((level & kLogCritical) != 0) {
		new_level = 2;
	}
	else if ((level & kLogError) != 0) {
		new_level = 3;
	}
	else if ((level & kLogWarning) != 0) {
		new_level = 4;
	}
	else if ((level & kLogInfo) != 0) {
		new_level = 6;
	}
	else if ((level & kLogDebug) != 0) {
		new_level = 7;
	}
	
#ifdef __LP64__
	reqid = (uint64_t) pthread_getspecific(_od_passthru_thread_key());
#else
	uint64_t *specific = (uint64_t *) pthread_getspecific(_od_passthru_thread_key());
	if (specific != NULL) {
		reqid = (*specific);
	}
#endif
	
	dispatch_sync(_get_passthru_queue(),  ^(void) {
		if (odd_port != MACH_PORT_NULL) {
			if (pthread_getspecific(_od_passthru_session_threadid()) == NULL) {
				send_legacy_log_message(odd_port, reqid, new_level, (char *) message);
			}
			else {
				send_session_log_message(odd_port, reqid, new_level, (char *) message);
			}
		}
	});
	
	return true;
}

void
od_passthru_set_node_availability(const char *nodename, bool available)
{
	dispatch_sync(_get_passthru_queue(), 
				  ^(void) {
					  if (odd_port != MACH_PORT_NULL) {
						  send_legacy_set_node_availability(odd_port, (caddr_t) nodename, available);
					  }
				  });
}

int32_t
od_passthru_register_node(const char *nodename, bool hidden)
{
	__block int32_t	error	= EIO;
	
	dispatch_sync(_get_passthru_queue(), 
				  ^(void) {
					  if (odd_port != MACH_PORT_NULL) {
						  // TODO: fix gDSLocalOnlyMode after we no longer have to forward legacy requests
						  send_legacy_register_node(odd_port, (caddr_t) nodename, (gDSLocalOnlyMode == true ? true : hidden), &error);
					  }
				  });
	
	return error;
}

void
od_passthru_unregister_node(const char *nodename)
{
	dispatch_sync(_get_passthru_queue(), 
				  ^(void) {
					  if (odd_port != MACH_PORT_NULL) {
						  send_legacy_unregister_node(odd_port, (caddr_t) nodename);
					  }
				  });
}

void
od_passthru_set_plugin_enabled(const char *plugin_name, bool enabled)
{
	dispatch_sync(_get_passthru_queue(), 
				  ^(void) {
					  if (odd_port != MACH_PORT_NULL) {
						  send_legacy_set_plugin_enabled(odd_port, (caddr_t) plugin_name, enabled);
					  }
				  });	
}

uid_t
od_passthru_get_uid(void)
{
	return (uid_t) (long) pthread_getspecific(_od_passthru_uid_key());
}

void
od_passthru_localonly_exit(void)
{
	if (gDSLocalOnlyMode == false) {
		return;
	}
	
	dispatch_source_t source = dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER, 0, 0, 
													  dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0));
	assert(source != NULL);
	
	dispatch_source_set_event_handler(source,
									  ^(void) {
										  // use try lock in case we collide with another timer which gets done on the
										  // same thread will cause an abort
										  if (gLocalSessionLock.WaitTryLock()) {
											  if (gLocalSessionCount == 0) {
												  // just stop the runloop it will exit
												  CFRunLoopStop(CFRunLoopGetMain());
												  
												  // we intentionally don't unlock cause we are going to exit
											  } else {
												  gLocalSessionLock.SignalLock();
											  }
										  }
										  
										  dispatch_source_cancel(source);
										  dispatch_release(source);
									  });
	dispatch_source_set_timer(source, dispatch_time(0, 10ull * NSEC_PER_SEC), 0, 1ull * NSEC_PER_SEC);
	dispatch_resume(source);
}

dispatch_source_t
od_passthru_create_source(mach_port_t port)
{
	dispatch_group_t mig_group = dispatch_group_create();
	dispatch_queue_t concurrentq = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0);
	
	dispatch_source_t source = dispatch_source_create(DISPATCH_SOURCE_TYPE_MACH_RECV, port, 0, concurrentq);
	assert(source != NULL);
	
	dispatch_source_set_event_handler(source,
									  ^(void) {
										  dispatch_group_async(mig_group, concurrentq,
															   ^(void) {
																   CInternalDispatch::AddCapability();
#if USE_DISPATCH_MIG_SERVER
																   dispatch_mig_server(source, _max_mach_msgsize(), _demux_reply);
#else
																   mach_msg_server(_demux_reply, _max_mach_msgsize(), port, MY_MIG_OPTIONS);
#endif
															   });
									  });
	dispatch_source_set_cancel_handler(source,
									   ^(void) {
										   dispatch_group_wait(mig_group, DISPATCH_TIME_FOREVER);
										   dispatch_release(mig_group);
										   mach_port_mod_refs(mach_task_self(), port, MACH_PORT_RIGHT_RECEIVE, -1);
									   });
	
	dispatch_resume(source);
	
	SrvrLog(kLogApplication, "Listening for Legacy plugin mach messages");
	DbgLog(kLogDebug, "Created mig source for Legacy plugin calls");
	
	return source;
}
