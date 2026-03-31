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

#include <OpenDirectory/OpenDirectory.h>

extern void __node_b0rk_uuid(ODNodeRef node);

static void
test_session(void)
{
	ODSessionRef session;
	CFArrayRef nodes;
	CFErrorRef error;
	NSArray *nodes2;
	NSError *error2;

	NSLog(@"%@", kODSessionDefault);
	NSLog(@"%@", [ODSession defaultSession]);
	NSLog(@"%@", [ODSession sessionWithOptions:nil error:nil]);

	nodes = ODSessionCopyNodeNames(NULL, kODSessionDefault, &error);
	if (nodes) {
		CFShow(nodes);
		CFRelease(nodes);
	} else {
		CFShow(error);
		CFRelease(error);
	}

	session = ODSessionCreate(NULL, NULL, &error);
	if (session) {
		CFShow(session);

		nodes = ODSessionCopyNodeNames(NULL, session, &error);
		if (nodes) {
			CFShow(nodes);
			CFRelease(nodes);
		} else {
			CFShow(error);
			CFRelease(error);
		}

		nodes2 = [(ODSession *)session nodeNamesAndReturnError:&error2];
		NSLog(@"%@", nodes2);

		CFRelease(session);
	} else {
		CFShow(error);
		CFRelease(error);
	}
}

static void
test_node()
{
	ODNodeRef node;
	CFErrorRef error;
	ODNode *node2;

	node = ODNodeCreateWithName(NULL, NULL, CFSTR("/LDAPv3"), &error);
	if (node) {
		CFShow(node);
		CFRelease(node);
	} else {
		CFShow(error);
		CFRelease(error);
	}

	node = ODNodeCreateWithNodeType(NULL, NULL, kODNodeTypeAuthentication, &error);
	if (node) {
		CFShow(node);
		CFRelease(node);
	} else {
		CFShow(error);
		CFRelease(error);
	}

	node2 = [ODNode nodeWithSession:[ODSession defaultSession] type:kODNodeTypeLocalNodes error:nil];
	NSLog(@"%@", [node2 class]);
	NSLog(@"%@", node2);
	
	node2 = [[ODNode alloc] initWithSession:[ODSession defaultSession] type:kODNodeTypeLocalNodes error:nil];
	NSLog(@"%@", [node2 class]);
	NSLog(@"%@", node2);
	[node2 release];
	node2 = nil;

	[[NSGarbageCollector defaultCollector] collectExhaustively];
}

static void
_qcallback(ODQueryRef query, CFArrayRef results, CFErrorRef error, void *context)
{
	if (results) {
		CFIndex count = CFArrayGetCount(results);
		*(CFIndex *)context += count;
#if 0
		CFIndex i;
		for (i = 0; i < count; i++) {
			ODRecordRef record;
			CFDictionaryRef details;

			record = (ODRecordRef)CFArrayGetValueAtIndex(results, i);
			details = ODRecordCopyDetails(record, NULL, NULL);
			if (details) {
				CFShow(details);
				CFRelease(details);
			}
			//CFShow(ODRecordGetRecordName((ODRecordRef)CFArrayGetValueAtIndex(results, i)));
		}
#endif
	} else {
		fprintf(stderr, "got %ld results\n", *(CFIndex *)context);
		CFRelease(query);
	}
}

static void
test_query()
{
	ODNodeRef node;
	ODQueryRef query;
	CFErrorRef error;
	CFIndex *count = calloc(1, sizeof(CFIndex));

	node = ODNodeCreateWithNodeType(NULL, NULL, kODNodeTypeLocalNodes, NULL);
	__node_b0rk_uuid(node);
	query = ODQueryCreateWithNode(NULL, node, kODRecordTypeUsers, kODAttributeTypeRecordName, kODMatchEqualTo, NULL, kODAttributeTypeAllAttributes, 0, &error);

	ODQuerySetCallback(query, _qcallback, count);
	ODQuerySetDispatchQueue(query, dispatch_get_main_queue());

	// TODO: completion...
}

static void
test_query_poll()
{
	ODQueryRef query;
	CFErrorRef error;
	CFArrayRef results;

	CFStringRef attrs[1] = { CFSTR("dsAttributesAll") };
	CFArrayRef reqattrs = CFArrayCreate(NULL, (const void **)attrs, 1, &kCFTypeArrayCallBacks);
	query = ODQueryCreateWithNodeType(NULL, kODNodeTypeLocalNodes, kODRecordTypeUsers, kODAttributeTypeRecordName, kODMatchEqualTo, NULL, reqattrs, 0, &error);
	CFRelease(reqattrs);
	if (!query) {
		CFShow(error);
		CFRelease(error);
		return;
	}

	for (;;) {
		results = ODQueryCopyResults(query, TRUE, &error);
		if (results) {
			printf("got %ld results\n", CFArrayGetCount(results));
			CFShow(results);
			CFRelease(results);
		} else {
			printf("results == NULL\n");
			if (error) {
				CFShow(error);
				CFRelease(error);
			}
			break;
		}
	}
}

@interface QueryTestDelegate : NSObject <ODQueryDelegate> {
	dispatch_semaphore_t sema;
	BOOL complete;
}
@end

@implementation QueryTestDelegate

- (id)init
{
	self = [super init];
	sema = dispatch_semaphore_create(0);
	complete = NO;
	return self;
}

- (void)dealloc
{
	dispatch_release(sema);
	[super dealloc];
}

- (void)finalize
{
	dispatch_release(sema);
	sema = NULL;
	[super finalize];
}

- (void)query:(ODQuery *)query foundResults:(NSArray *)results error:(NSError *)error
{
	if (complete) {
		return;
	}

	if (results == NULL) {
		complete = YES;
		dispatch_semaphore_signal(sema);
		return;
	}

	for (ODRecord *record in results) {
		NSLog(@"%@", (NSString *)ODRecordGetRecordName((ODRecordRef)record));
	}
}

- (void)wait
{
	long result = dispatch_semaphore_wait(sema, DISPATCH_TIME_FOREVER);
	assert(result == 0);
}

- (BOOL)complete
{
	return complete;
}

@end

static ODQuery *
_getquery()
{
	return [ODQuery queryWithNode:[ODNode nodeWithSession:nil type:kODNodeTypeLocalNodes error:NULL]
		forRecordTypes:kODRecordTypeUsers
		attribute:kODAttributeTypeRecordName
		matchType:kODMatchBeginsWith
		queryValues:@"_s"
		returnAttributes:nil
		maximumResults:0
		error:NULL];
}

static void
test_objc_query_sync()
{
	ODQuery *query = _getquery();
	NSArray *results = [query resultsAllowingPartial:NO error:NULL];
	NSLog(@"%lu", [results count]);
	// TODO: why doesn't this make it get collected?
	query = nil;
}

static void
test_objc_query()
{
	ODQuery *query = _getquery();
	id blarg = [QueryTestDelegate new];
	NSOperationQueue *oq = [[NSOperationQueue new] autorelease];
	[query setDelegate:blarg];
	[query setOperationQueue:oq];
	[blarg wait];

	[blarg release]; blarg = nil;
	query = nil;
	oq = nil;
}

static void
test_objc_query_rl()
{
	ODQuery *query = _getquery();
	id blarg = [QueryTestDelegate new];
	[query setDelegate:blarg];
	[query scheduleInRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
	while (![blarg complete] && [[NSRunLoop currentRunLoop] runMode:NSDefaultRunLoopMode beforeDate:[NSDate distantFuture]]) {
		;
	}
	[query removeFromRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
	[blarg release]; blarg = nil;
	query = nil;
}

static void
test_record()
{
	CFErrorRef error;
	ODSessionRef session;
	ODNodeRef node;
	ODRecordRef record;

	session = ODSessionCreate(NULL, NULL, &error);
	if (session) {
		CFShow(session);
		node = ODNodeCreateWithNodeType(NULL, session, kODNodeTypeAuthentication, &error);
		if (node) {
			CFShow(node);
			CFStringRef attrs[1] = { (CFStringRef)kODAttributeTypeStandardOnly };
			CFArrayRef reqattrs = CFArrayCreate(NULL, (const void **)attrs, 1, &kCFTypeArrayCallBacks);
			record = ODNodeCopyRecord(node, kODRecordTypeUsers, CFSTR("local"), NULL, &error);
			CFRelease(reqattrs);
			if (record) {
				CFShow(record);

				CFArrayRef v = ODRecordCopyValues(record, (ODAttributeType)CFSTR("bogus"), NULL);
				if (v) {
					CFShow(v);
					CFRelease(v);
				}

				CFRelease(record);
			} else {
				CFShow(error);
				CFRelease(error);
			}
			CFRelease(node);
		} else {
			CFShow(error);
			CFRelease(error);
		}
		CFRelease(session);
	} else {
		CFShow(error);
		CFRelease(error);
	}
}

static void
test_concurrency()
{
	dispatch_queue_t cfshow_q = dispatch_queue_create("CFShow", NULL);
	dispatch_semaphore_t sema = dispatch_semaphore_create(10);

	// work around bug in old OD framework (kODSessionDefault doesn't always get initialized)
	CFShow([ODSession defaultSession]);

	for (;;) {
		dispatch_semaphore_wait(sema, DISPATCH_TIME_FOREVER);

		dispatch_async(dispatch_get_global_queue(0, 0), ^{
			CFArrayRef tmp = ODSessionCopyNodeNames(NULL, kODSessionDefault, NULL);
			assert(tmp != NULL);
			dispatch_async(cfshow_q, ^{
				CFShow(tmp);
				CFRelease(tmp);
			});
			dispatch_semaphore_signal(sema);
		});
	}
}

static void
test_membership()
{
	Boolean success;
	CFErrorRef error;
	ODNodeRef node;
	ODRecordRef group, member;

	success = ODRecordContainsMember(NULL, NULL, &error);
	if (success) {
		printf("null args should've failed\n");
	} else {
		CFShow(error);
		CFRelease(error);
	}

	node = ODNodeCreateWithNodeType(NULL, NULL, kODNodeTypeLocalNodes, &error);
	if (node == NULL) {
		CFShow(error);
		CFRelease(error);
	} else {
		group = ODNodeCopyRecord(node, kODRecordTypeGroups, CFSTR("admin"), NULL, NULL);
		member = ODNodeCopyRecord(node, kODRecordTypeUsers, CFSTR("local"), NULL, NULL);
		if (group && member) {
			CFShow(group);
			CFShow(member);

			if (ODRecordContainsMember(group, member, &error)) {
				printf("yes\n");
			} else {
				CFShow(error);
				CFRelease(error);
			}
		}
		if (group) CFRelease(group);
		if (member) CFRelease(member);
	}
}

static void
test_gc()
{
	int n = 10;
	NSAutoreleasePool *pool;
	ODNode *node;
	int i;

	pool = [NSAutoreleasePool new];

	for (i = 0; i < n; i++) {
		node = [ODNode nodeWithSession:nil type:kODNodeTypeAuthentication error:nil];
		NSLog(@"%@", node);
		node = nil;
	}

	[pool drain];
	//[[NSGarbageCollector defaultCollector] collectExhaustively];
}

static void
test_recover()
{
	ODNodeRef node;
	CFArrayRef subnodes;
	CFErrorRef error;

	node = ODNodeCreateWithNodeType(NULL, NULL, kODNodeTypeAuthentication, NULL);
	__node_b0rk_uuid(node);
	if (node) {
		CFShow(node);
		subnodes = ODNodeCopySubnodeNames(node, &error);
		if (subnodes) {
			CFShow(subnodes);
			CFRelease(subnodes);
		} else {
			CFShow(error);
			CFRelease(error);
		}
		CFRelease(node);
	}
}

static struct {
	char *name;
	void (*func)(void);
} tests[] = {
	{ "session", test_session },
	{ "node", test_node },
	{ "record", test_record },
	{ "concurrency", test_concurrency },
	{ "query", test_query },
	{ "objc_query", test_objc_query },
	{ "objc_query_sync", test_objc_query_sync },
	{ "objc_query_rl", test_objc_query_rl },
	{ "membership", test_membership },
	{ "query_poll", test_query_poll },
	{ "gc", test_gc },
	{ "recover", test_recover },
};

static void
usage(void)
{
	size_t i;

	fprintf(stderr, "Available tests:");
	for (i = 0; i < sizeof(tests) / sizeof(*tests); i++) {
		fprintf(stderr, " %s", tests[i].name);
	}
	fprintf(stderr, "\n");

	exit(1);
}

int
main(int argc, char *argv[])
{
	NSAutoreleasePool *pool;
	size_t i;
	void (*func)(void) = NULL;

	if (argc != 2) {
		usage();
	}

	for (i = 0; i < sizeof(tests) / sizeof(*tests); i++) {
		if (!strcmp(argv[1], tests[i].name)) {
			func = tests[i].func;
		}
	}

	if (!func) {
		usage();
	}

	pool = [NSAutoreleasePool new];
	for (;;) {
		func();
		usleep(250000);
	}
	[pool drain];

	[[NSRunLoop currentRunLoop] run];

	return 0;
}
