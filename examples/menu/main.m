#include <stdio.h>
#import <DBusKit/DKConnection.h>
#import <DBusKit/DKMessage.h>

int main(int argc, const char *argv[])
{
	printf("Hello\n");
	@autoreleasepool {
		DKConnection *conn = [DKConnection new];

		// DKMessage *message = [[DKMessage alloc] initMethodCall:"Introspect" interface:"org.freedesktop.DBus.Introspectable" path:"/com/canonical/AppMenu/Registrar" destination:"com.canonical.AppMenu.Registrar"];
		DKMessage *message = [[DKMessage alloc] initMethodCall:"RegisterWindow" interface:"com.canonical.AppMenu.Registrar" path:"/com/canonical/AppMenu/Registrar" destination:"com.canonical.AppMenu.Registrar"];
		uint32_t window = 0x800007;
		[message appendArg:&window type:DBUS_TYPE_UINT32];
		const char *path = "/Bar/foo/1";
		[message appendArg:&path type:DBUS_TYPE_OBJECT_PATH];
		DKMessage *reply = [conn sendWithReplyAndBlock:message];

		if(reply != nil) {
			printf("registered\n");
		}
	}
	return 0;
}

