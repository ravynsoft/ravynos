#import <Foundation/Foundation.h>
#include <unistd.h>

int main(int argc, const char **argv)
{
    __NSInitializeProcess(argc, argv);
    @autoreleasepool {
        NSString *execPath = [[[NSBundle mainBundle] executablePath] autorelease];
        execPath = [[[execPath stringByDeletingLastPathComponent]
            stringByAppendingPathComponent:@"qterminal"] autorelease];
        execv([execPath UTF8String], (char * const *)argv);
    }
    return 1;
}
