#import <Foundation/Foundation.h>

int main(int argc, char *argv[]) {
	[[NSAutoreleasePool alloc] init];

	if(argc != 2) {
		NSLog(@"Usage: %s path-to-XML\n", argv[0]);
		return 1;
	}
	NSString *path = [NSString stringWithUTF8String:argv[1]];

	NSError *err = nil;
	NSXMLDocument *doc = [[NSXMLDocument alloc]
		initWithContentsOfURL:[NSURL
			fileURLWithPath:path]
		options:0
		error:&err];
	if(err) {
		NSLog(@"ERROR: %@", err);
		return 1;
	} else {
		NSLog(@"Parsed!");
		return 0;
	}
}
