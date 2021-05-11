#include <stdio.h>
#import <DBusKit/DKConnection.h>
#import <DBusKit/DKMessage.h>

int main(int argc, const char *argv[])
{
	printf("Hello\n");
	DKConnection *conn = [[DKConnection alloc] init];

	DKMessage *message = [[DKMessage alloc] initMethodCall:"Introspect" interface:"org.freedesktop.DBus.Introspectable" path:"/" destination:"org.freedesktop.DBus"];
	DKMessage *reply = [conn sendWithReplyAndBlock:message];

	if(reply != nil) {
		printf("%s", [[reply argsAsString] UTF8String]);
	}
	return 0;
}

