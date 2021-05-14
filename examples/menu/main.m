#include <stdio.h>
#import <DBusKit/DKConnection.h>
#import <DBusKit/DKMenu.h>

int main(int argc, const char *argv[])
{
	printf("Hello\n");
	@autoreleasepool {
		DKConnection *conn = [DKConnection new];

		// DKMessage *message = [[DKMessage alloc] initMethodCall:"Introspect" interface:"org.freedesktop.DBus.Introspectable" path:"/com/canonical/AppMenu/Registrar" destination:"com.canonical.AppMenu.Registrar"];
		DKMenu *menuHandler = [[[DKMenu alloc] initWithConnection: conn] autorelease];
		if([menuHandler registerWindow: 12345 objectPath:@"/Menu/Bar/2"] == YES) {
			printf("registered\n");
		}

		NSString *foo = [menuHandler getMenuForWindow: 12345];
		if(foo != nil)
			printf("%s\n", [foo UTF8String]);

		if([menuHandler unregisterWindow: 12345] == YES) {
			printf("unregistered\n");
		}
	}
	return 0;
}

