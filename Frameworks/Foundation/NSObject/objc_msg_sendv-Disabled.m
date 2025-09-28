#ifdef COCOTRON_DISALLOW_FORWARDING
#import <Foundation/NSException.h>
#import <Foundation/NSString.h>
#import <objc/message.h>

id objc_msgSendv(id self, SEL selector, unsigned arg_size, void *arg_frame)
{
	[NSException raise:@"OBJCForwardingUnavailableException" format:@"Sorry, but objc_msgSendv and forwarding including NSInvocation are unavailable on this platform."];
	return nil;
}

#endif
