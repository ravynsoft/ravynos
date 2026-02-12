#import <OpenDirectory/OpenDirectory.h>

@interface MyTestSession : ODSession
@end

@implementation MyTestSession

- (NSArray *)nodeNamesAndReturnError:(NSError **)error
{
	printf("hi\n");
	return [[super nodeNamesAndReturnError:error] arrayByAddingObject:@"foo"];
}

@end

@interface MyTestNode : ODNode
@end

@implementation MyTestNode

- (NSArray *)subnodeNamesAndReturnError:(NSError **)error
{
	return [[super subnodeNamesAndReturnError:error] arrayByAddingObject:@"haha"];
}

@end

static void
test_session()
{
	ODSession *session;
	MyTestSession *session2;

	session = [ODSession sessionWithOptions:nil error:NULL];
	NSLog(@"%@", [session nodeNamesAndReturnError:NULL]);

	session2 = [MyTestSession sessionWithOptions:nil error:NULL];
	NSLog(@"%@", [session2 class]);
	NSLog(@"%@", [session2 nodeNamesAndReturnError:NULL]);
	NSLog(@"%@", ODSessionCopyNodeNames(NULL, (ODSessionRef)session2, NULL));
}

int
main()
{
	NSAutoreleasePool *pool;
	MyTestNode *node;
	ODNode *node2;
	ODNodeRef node3;
	NSArray *subnodes;

	pool = [NSAutoreleasePool new];

	test_session();

	//node = [[MyTestNode alloc] initWithSession:[ODSession defaultSession] type:kODNodeTypeAuthentication error:NULL];
	node = [MyTestNode nodeWithSession:[ODSession defaultSession] type:kODNodeTypeAuthentication error:NULL];
	NSLog(@"%@", [node class]);
	//assert(CFGetTypeID(node) == ODNodeGetTypeID());
	subnodes = (NSArray *)ODNodeCopySubnodeNames((ODNodeRef)node, NULL);
	assert([[subnodes lastObject] isEqualToString:@"haha"]);
	[subnodes release];
	subnodes = [node subnodeNamesAndReturnError:NULL];
	assert([[subnodes lastObject] isEqualToString:@"haha"]);
	//[node release];

	node = [MyTestNode nodeWithSession:[ODSession defaultSession] type:kODNodeTypeAuthentication error:NULL];
	assert(CFGetTypeID(node) == ODNodeGetTypeID());
	subnodes = (NSArray *)ODNodeCopySubnodeNames((ODNodeRef)node, NULL);
	assert([[subnodes lastObject] isEqualToString:@"haha"]);
	[subnodes release];
	subnodes = [node subnodeNamesAndReturnError:NULL];
	assert([[subnodes lastObject] isEqualToString:@"haha"]);

	node2 = [[ODNode alloc] initWithSession:[ODSession defaultSession] type:kODNodeTypeAuthentication error:NULL];
	assert(CFGetTypeID(node2) == ODNodeGetTypeID());
	subnodes = (NSArray *)ODNodeCopySubnodeNames((ODNodeRef)node2, NULL);
	assert([subnodes count]);
	[subnodes release];
	subnodes = [node2 subnodeNamesAndReturnError:NULL];
	assert([subnodes count]);
	[node2 release];

	node3 = ODNodeCreateWithNodeType(NULL, NULL, kODNodeTypeAuthentication, NULL);
	assert(CFGetTypeID(node3) == ODNodeGetTypeID());
	subnodes = (NSArray *)ODNodeCopySubnodeNames(node3, NULL);
	assert([subnodes count]);
	[subnodes release];
	subnodes = [(ODNode *)node3 subnodeNamesAndReturnError:NULL];
	assert([subnodes count]);
	CFRelease(node3);

	[pool drain];
	dispatch_main();
}
