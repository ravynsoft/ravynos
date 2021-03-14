#import <Foundation/Foundation.h>

int main(int argc, char *argv[])
{
	NSLog(@"Hello from Bar.app");
	NSBundle *main = [NSBundle mainBundle];
	NSString *path = [main pathForResource:@"sample" ofType:@"txt" inDirectory:@"rsc"];
	NSLog(@"loading file %s", [path UTF8String]);
	NSFileManager *fm = [NSFileManager defaultManager];
	NSData *text = [fm contentsAtPath:path];
	NSLog(@"%s", [[[NSString alloc] initWithData:text encoding:NSASCIIStringEncoding] UTF8String]);
}

