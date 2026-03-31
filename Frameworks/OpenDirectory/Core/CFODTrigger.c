#include <CoreFoundation/CoreFoundation.h>
#include <CoreFoundation/CFBridgingPriv.h>
#include <SystemConfiguration/SystemConfiguration.h>

#include "CFODTrigger.h"

#include "CFOpenDirectoryPriv.h"
#include "internal.h"

#pragma mark Type Definition

enum {
	eODTriggerTypeNodes,
	eODTriggerTypeRecords,
	eODTriggerTypeRecordAttributes,
};

struct __ODTrigger {
	CFRuntimeBase base;
	SCDynamicStoreRef dyn_store;
	dispatch_queue_t destination_queue;
	dispatch_queue_t internal_queue;
	int type;
	void (^nodeblock)(ODTriggerRef, CFStringRef);
	void (^recblock)(ODTriggerRef, CFStringRef, CFStringRef, CFStringRef);
	void (^attrblock)(ODTriggerRef, CFStringRef, CFStringRef, CFStringRef, CFStringRef); 
	dispatch_once_t cancelled;
};

static CFTypeID __kODTriggerTypeID = _kCFRuntimeNotATypeID;

static void
__ODTriggerFinalize(CFTypeRef cf)
{
	ODTriggerRef trigger = (ODTriggerRef)cf;

	// no-op if it's already been cancelled
	ODTriggerCancel(trigger);

	safe_cfrelease_null(trigger->dyn_store);

	safe_block_release_null(trigger->nodeblock);
	safe_block_release_null(trigger->recblock);
	safe_block_release_null(trigger->attrblock);

	safe_dispatch_release(trigger->destination_queue);
	safe_dispatch_release(trigger->internal_queue);
}

static const CFRuntimeClass __ODTriggerClass = {
	0,								// version
	"ODTrigger",					// className
	NULL,							// init
	NULL,							// copy
	__ODTriggerFinalize,			// finalize
	NULL,							// equal
	NULL,							// hash
	NULL,							// copyFormattingDesc
	NULL,							// copyDebugDesc
	NULL,							// reclaim
#if CF_REFCOUNT_AVAILABLE
	NULL,							// refcount
#endif
};

#pragma mark Trigger Creation/Setup

static CFStringRef
_create_notifystr(CFTypeRef values)
{
	CFStringRef string = NULL;
	CFTypeID type;

	if (values != NULL) {
		type = CFGetTypeID(values);

		if (type == CFArrayGetTypeID()) {
			CFIndex count = CFArrayGetCount(values);

			if (count > 1) {
				CFStringRef tempStr = CFStringCreateByCombiningStrings(kCFAllocatorDefault, values, CFSTR("|"));
				if (tempStr != NULL) {
					string = CFStringCreateWithFormat(kCFAllocatorDefault, NULL, CFSTR("(%@)"), tempStr);
					CFRelease(tempStr);
				}
			} else if (count == 1) {
				string = CFArrayGetValueAtIndex(values, 0);
				CFRetain(string);
			}
		} else if (type == CFStringGetTypeID()) {
			string = CFRetain(values);
		}
	}

	return string;
}

static CFArrayRef
_create_patterns_nodetype(CFTypeRef events, CFStringRef type, CFTypeRef nodenames)
{
	CFArrayRef patterns = NULL;
	CFStringRef eventStr, nodeStr;

	if (events == NULL) {
		return NULL;
	}

	eventStr = _create_notifystr(events);
	nodeStr = _create_notifystr(nodenames);

	if (nodeStr != NULL || eventStr != NULL) {
		CFStringRef notifyKey;

		notifyKey = CFStringCreateWithFormat(kCFAllocatorDefault, NULL,
		                                     CFSTR("opendirectoryd:%@;%@;%@"),
		                                     type,
		                                     (eventStr != NULL ? eventStr : CFSTR("[^;]+")),
		                                     (nodeStr != NULL ? nodeStr : CFSTR("[^;]+")));
		if (notifyKey) {
			patterns = CFArrayCreate(NULL, (const void **)&notifyKey, 1, &kCFTypeArrayCallBacks);
			CFRelease(notifyKey);
		}
	}

	if (eventStr) CFRelease(eventStr);
	if (nodeStr) CFRelease(nodeStr);

	return patterns;
}

static CFArrayRef
_create_patterns_recordtype(CFTypeRef events, CFTypeRef nodenames, CFTypeRef recordtypes, CFTypeRef recordnames, CFTypeRef attributes)
{
	CFArrayRef patterns = NULL;
	CFStringRef eventsStr, nodenamesStr = NULL, rectypesStr, recnamesStr = NULL, attribStr = NULL;
	CFStringRef notifyKey;

	eventsStr = _create_notifystr(events);
	rectypesStr = _create_notifystr(recordtypes);

	if (eventsStr == NULL || rectypesStr == NULL) {
		goto bail;
	}

	nodenamesStr = _create_notifystr(nodenames);
	recnamesStr = _create_notifystr(recordnames);
	attribStr = _create_notifystr(attributes);

	if (recnamesStr != NULL || attribStr != NULL) {
		notifyKey = CFStringCreateWithFormat(kCFAllocatorDefault, NULL,
		                                     CFSTR("opendirectoryd:records;%@;%@;%@;%@;%@"),
		                                     eventsStr,
		                                     (nodenamesStr != NULL ? nodenamesStr : CFSTR("[^;]+")),
		                                     rectypesStr,
		                                     (recnamesStr != NULL ? recnamesStr : CFSTR("[^;]+")),
		                                     (attribStr != NULL ? attribStr : CFSTR("[^;]+")));
	} else {
		notifyKey = CFStringCreateWithFormat(kCFAllocatorDefault, NULL,
		                                     CFSTR("opendirectoryd:records;%@;%@;%@.*"),
		                                     eventsStr,
		                                     (nodenamesStr != NULL ? nodenamesStr : CFSTR("[^;]+")),
		                                     rectypesStr);
	}

	if (notifyKey) {
		patterns = CFArrayCreate(NULL, (const void **)&notifyKey, 1, &kCFTypeArrayCallBacks);
		CFRelease(notifyKey);
	}

bail:
	safe_cfrelease(eventsStr);
	safe_cfrelease(nodenamesStr);
	safe_cfrelease(rectypesStr);
	safe_cfrelease(recnamesStr);
	safe_cfrelease(attribStr);

	return patterns;
}

static void
_dynstore_node_applier(const void *value, void *context)
{
	ODTriggerRef trigger = (ODTriggerRef)context;
	CFArrayRef array;

	array = CFStringCreateArrayBySeparatingStrings(kCFAllocatorDefault, value, CFSTR(";"));
	if (array && CFArrayGetCount(array) > 2) {
		CFStringRef nodename = CFArrayGetValueAtIndex(array, 2);

		CFRetain(trigger);
		CFRetain(array);
		dispatch_async(trigger->destination_queue, ^{
			trigger->nodeblock(trigger, nodename);
			CFRelease(trigger);
			CFRelease(array);
		});
	}

	safe_cfrelease(array);
}

static void
_dynstore_record_applier(const void *value, void *context)
{
	ODTriggerRef trigger = (ODTriggerRef)context;
	CFArrayRef array;
	CFIndex count;

	array = CFStringCreateArrayBySeparatingStrings(kCFAllocatorDefault, value, CFSTR(";"));
	if (array && (count = CFArrayGetCount(array)) > 4) {
		CFRetain(trigger);
		CFRetain(array);
		dispatch_async(trigger->destination_queue, ^{
			CFStringRef rectype = CFArrayGetValueAtIndex(array, 3);
			CFStringRef recname = CFArrayGetValueAtIndex(array, 4);
			CFStringRef attrib = (count > 5) ? CFArrayGetValueAtIndex(array, 5) : NULL;

			if (trigger->attrblock) {
				trigger->attrblock(trigger, CFArrayGetValueAtIndex(array, 2), rectype, recname, attrib);
			} else if (trigger->recblock) {
				trigger->recblock(trigger, CFArrayGetValueAtIndex(array, 2), rectype, recname);
			}

			CFRelease(trigger);
			CFRelease(array);
		});
	}

	safe_cfrelease(array);
}

static void
_dynstore_callback(SCDynamicStoreRef store __unused, CFArrayRef changedKeys, void *info)
{
	ODTriggerRef trigger = (ODTriggerRef)info;
	CFArrayApplierFunction applier;

	switch (trigger->type) {
	case eODTriggerTypeNodes:
		applier = _dynstore_node_applier;
		break;
	case eODTriggerTypeRecords:
	case eODTriggerTypeRecordAttributes:
		applier = _dynstore_record_applier;
		break;
	default:
		/*NOTREACHED*/
		return;
	}

	CFArrayApplyFunction(changedKeys, CFRangeMake(0, CFArrayGetCount(changedKeys)), applier, trigger);
}

static ODTriggerRef
_ODTriggerCreate(CFAllocatorRef allocator)
{
	return (ODTriggerRef)_CFRuntimeCreateInstance(allocator, ODTriggerGetTypeID(), sizeof(struct __ODTrigger) - sizeof(CFRuntimeBase), NULL);
}

static ODTriggerRef
_ODTriggerInit(ODTriggerRef trigger, CFArrayRef patterns, dispatch_queue_t queue, int type, void *block)
{
	SCDynamicStoreContext context;
	char qname[256];

	trigger->destination_queue = (dispatch_retain(queue), queue);

	trigger->type = type;
	switch (trigger->type) {
	case eODTriggerTypeNodes:
		trigger->nodeblock = Block_copy(block);
		break;
	case eODTriggerTypeRecords:
		trigger->recblock = Block_copy(block);
		break;
	case eODTriggerTypeRecordAttributes:
		trigger->attrblock = Block_copy(block);
		break;
	}

	context.version = 0;
	context.info = trigger;
	context.retain = NULL;
	context.release = NULL;
	context.copyDescription = CFCopyDescription;

	snprintf(qname, sizeof(qname), "com.apple.OpenDirectory.ODTrigger.type%d.%p", type, trigger);
	trigger->internal_queue = dispatch_queue_create(qname, NULL);
	trigger->dyn_store = SCDynamicStoreCreate(kCFAllocatorDefault, NULL, _dynstore_callback, &context);
	SCDynamicStoreSetNotificationKeys(trigger->dyn_store, NULL, patterns);
	SCDynamicStoreSetDispatchQueue(trigger->dyn_store, trigger->internal_queue);

	return trigger;
}

static ODTriggerRef
_ODTriggerCreateNodeType(CFAllocatorRef allocator, CFTypeRef events, CFStringRef type, CFTypeRef nodenames,
	dispatch_queue_t queue, void (^block)(ODTriggerRef trigger, CFStringRef node))
{
	ODTriggerRef trigger = NULL;
	CFArrayRef patterns;

	patterns = _create_patterns_nodetype(events, type, nodenames);
	if (patterns) {
		trigger = _ODTriggerCreate(allocator);
		if (trigger) {
			trigger = _ODTriggerInit(trigger, patterns, queue, eODTriggerTypeNodes, block);
		}
		CFRelease(patterns);
	}

	return trigger;
}

static ODTriggerRef
_ODTriggerCreateRecordType(CFAllocatorRef allocator, int type, CFTypeRef events, CFTypeRef nodenames, CFTypeRef recordtypes, CFTypeRef recordnames, CFTypeRef attributes,
	dispatch_queue_t queue, void *block)
{
	ODTriggerRef trigger = NULL;
	CFArrayRef patterns;

	patterns = _create_patterns_recordtype(events, nodenames, recordtypes, recordnames, attributes);
	if (patterns) {
		trigger = _ODTriggerCreate(allocator);
		if (trigger) {
			trigger = _ODTriggerInit(trigger, patterns, queue, type, block);
		}
		CFRelease(patterns);
	}

	return trigger;
}

#pragma mark Entry Points

CFTypeID
ODTriggerGetTypeID(void)
{
	static dispatch_once_t once;

	dispatch_once(&once, ^ {
		__kODTriggerTypeID = _CFRuntimeRegisterClass(&__ODTriggerClass);
		//if (__kODTriggerTypeID != _kCFRuntimeNotATypeID) {
		//	_CFRuntimeBridgeClasses(__kODTriggerTypeID, "NSODTrigger");
		//}
	});

	return __kODTriggerTypeID;
}

// odtrigger_records_create
ODTriggerRef
ODTriggerCreateForRecords(CFAllocatorRef allocator, ODTriggerEventFlags events, CFTypeRef nodenames, CFTypeRef recordtypes, CFTypeRef recordnames,
	dispatch_queue_t queue, void (^block)(ODTriggerRef trigger, CFStringRef node, CFStringRef type, CFStringRef name))
{
	ODTriggerRef trigger = NULL;
	CFMutableArrayRef eventsArray = CFArrayCreateMutable(kCFAllocatorDefault, 3, &kCFTypeArrayCallBacks);

	if (events & kODTriggerRecordEventAdd) {
		CFArrayAppendValue(eventsArray, CFSTR("add"));
	}
	if (events & kODTriggerRecordEventDelete) {
		CFArrayAppendValue(eventsArray, CFSTR("delete"));
	}
	if (events & kODTriggerRecordEventModify) {
		CFArrayAppendValue(eventsArray, CFSTR("modify"));
	}

	trigger = _ODTriggerCreateRecordType(allocator, eODTriggerTypeRecords, eventsArray, nodenames, recordtypes, recordnames, NULL, queue, block);

	safe_cfrelease(eventsArray);

	return trigger;
}

// odtrigger_recordattributes_create
ODTriggerRef
ODTriggerCreateForRecordAttributes(CFAllocatorRef allocator, CFTypeRef nodenames, CFTypeRef recordtypes, CFTypeRef recordnames, CFTypeRef attributes,
	dispatch_queue_t queue, void (^block)(ODTriggerRef trigger, CFStringRef node, CFStringRef type, CFStringRef rec, CFStringRef attr))
{
	return _ODTriggerCreateRecordType(allocator, eODTriggerTypeRecordAttributes, CFSTR("modify"), nodenames, recordtypes, recordnames, attributes, queue, block);
}

// odtrigger_nodes_create
ODTriggerRef
ODTriggerCreateForNodes(CFAllocatorRef allocator, ODTriggerEventFlags events, CFTypeRef nodenames,
	dispatch_queue_t queue, void (^block)(ODTriggerRef trigger, CFStringRef node))
{
	ODTriggerRef trigger = NULL;
	CFMutableArrayRef eventsArray = CFArrayCreateMutable(kCFAllocatorDefault, 4, &kCFTypeArrayCallBacks);

	if (events & kODTriggerNodeRegister) {
		CFArrayAppendValue(eventsArray, CFSTR("register"));
	}
	if (events & kODTriggerNodeUnregister) {
		CFArrayAppendValue(eventsArray, CFSTR("unregister"));
	}
	if (events & kODTriggerNodeOnline) {
		CFArrayAppendValue(eventsArray, CFSTR("online"));
	}
	if (events & kODTriggerNodeOffline) {
		CFArrayAppendValue(eventsArray, CFSTR("offline"));
	}

	trigger = _ODTriggerCreateNodeType(allocator, eventsArray, CFSTR("nodes"), nodenames, queue, block);

	safe_cfrelease(eventsArray);

	return trigger;
}

// odtrigger_search_create
ODTriggerRef
ODTriggerCreateForSearch(CFAllocatorRef allocator, ODTriggerEventFlags events, CFTypeRef nodenames,
	dispatch_queue_t queue, void (^block)(ODTriggerRef trigger, CFStringRef node))
{
	ODTriggerRef trigger = NULL;
	CFMutableArrayRef eventsArray = CFArrayCreateMutable(kCFAllocatorDefault, 4, &kCFTypeArrayCallBacks);

	if (events & kODTriggerSearchAdd) {
		CFArrayAppendValue(eventsArray, CFSTR("add"));
	}
	if (events & kODTriggerSearchDelete) {
		CFArrayAppendValue(eventsArray, CFSTR("delete"));
	}
	if (events & kODTriggerSearchOnline) {
		CFArrayAppendValue(eventsArray, CFSTR("online"));
	}
	if (events & kODTriggerSearchOffline) {
		CFArrayAppendValue(eventsArray, CFSTR("offline"));
	}

	trigger = _ODTriggerCreateNodeType(allocator, eventsArray, CFSTR("search"), nodenames, queue, block);

	safe_cfrelease(eventsArray);

	return trigger;
}

// odtrigger_cancel
void
ODTriggerCancel(ODTriggerRef trigger)
{
	dispatch_once(&trigger->cancelled, ^{
		SCDynamicStoreSetDispatchQueue(trigger->dyn_store, NULL);
		dispatch_sync(trigger->internal_queue, ^{}); // empty block to ensure callbacks are complete
	});
}
