#import <Foundation/Foundation.h>

@interface GSSpeechServer
+ (void)start;
@end

int main(void)
{
	[NSAutoreleasePool new];
	[GSSpeechServer start];
	return 0;
}
