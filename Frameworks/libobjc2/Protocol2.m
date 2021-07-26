#include "objc/runtime.h"
#include "protocol.h"
#include "class.h"
#include <stdio.h>
#include <string.h>

@implementation Protocol
// FIXME: This needs removing, but it's included for now because GNUstep's
// implementation of +[NSObject conformsToProtocol:] calls it.
- (BOOL)conformsTo: (Protocol*)p
{
	return protocol_conformsToProtocol(self, p);
}
- (id)retain
{
	return self;
}
- (void)release {}
+ (Class)class { return self; }
- (id)self { return self; }
@end
@interface __IncompleteProtocol : Protocol @end
@implementation __IncompleteProtocol @end

/**
 * This class exists for the sole reason that the legacy GNU ABI did not
 * provide a way of registering protocols with the runtime.  With the new ABI,
 * every protocol in a compilation unit that is not referenced should be added
 * in a category on this class.  This ensures that the runtime sees every
 * protocol at least once and can perform uniquing.
 */
@interface __ObjC_Protocol_Holder_Ugly_Hack { id isa; } @end
@implementation __ObjC_Protocol_Holder_Ugly_Hack @end

@implementation Object @end

@implementation ProtocolGCC @end
@implementation ProtocolGSv1 @end

PRIVATE void link_protocol_classes(void)
{
	[Protocol class];
	[ProtocolGCC class];
	[ProtocolGSv1 class];
}
