#include "Test.h"
#include "../objc/hooks.h"

@interface Forward : Test
- (id)forwardingTargetForSelector: (SEL)sel;
@end

id target;

@implementation Forward
- (id)forwardingTargetForSelector: (SEL)sel
{
	return target;
}
@end

@interface Forward2 : Test
@end

@interface ForwardingTarget : Test
@end

BOOL forwardingTargetCalled;

@implementation ForwardingTarget
- (void)foo: (int)x
{
	assert(x == 42);
	forwardingTargetCalled = YES;
}
@end
@implementation Forward2
- (void)forward: (int)x
{
	[target foo: x];
}
@end

static id proxy_lookup(id receiver, SEL selector)
{
	if (class_respondsToSelector(object_getClass(receiver), @selector(forwardingTargetForSelector:)))
	{
		return [receiver forwardingTargetForSelector: selector];
	}
	return nil;
}

static IMP forward(id receiver, SEL selector)
{
	if (class_respondsToSelector(object_getClass(receiver), @selector(forward:)))
	{
		return class_getMethodImplementation(object_getClass(receiver), @selector(forward:));
	}
	assert(0);
}

int main(void)
{
	objc_proxy_lookup = proxy_lookup;
	__objc_msg_forward2 = forward;
	target = [ForwardingTarget new];
	id proxy = [Forward new];
	[proxy foo: 42];
	[proxy dealloc];
	assert(forwardingTargetCalled == YES);
	forwardingTargetCalled = NO;
	proxy = [Forward2 new];
	[proxy foo: 42];
	[proxy dealloc];
	assert(forwardingTargetCalled == YES);
}
