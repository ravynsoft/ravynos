#import <Foundation/Foundation.h>
#import <Foundation/NSPlatform.h>
#include <unistd.h>

int main(int argc, const char **argv)
{
    __NSInitializeProcess(argc, argv);
    @autoreleasepool {
        NSBundle *mainBundle = [[NSBundle mainBundle] autorelease];
        NSString *execPath = [[mainBundle executablePath] autorelease];
        execPath = [[[execPath stringByDeletingLastPathComponent]
            stringByAppendingPathComponent:@"qterminal"] autorelease];

        // install the default config if the user does not have one
        NSString *defConfigPath = [[mainBundle pathForResource:@"qterminal"
            ofType:@"ini" inDirectory:@""] autorelease];
        NSPlatform *currentPlatform = [NSPlatform currentPlatform];
        NSString *userConfigPath = [[[currentPlatform libraryDirectory]
		stringByAppendingPathComponent:@"FreeDesktop/config/qterminal.org"]
		autorelease];

        NSFileManager *fm = [NSFileManager defaultManager];
	if([fm fileExistsAtPath:userConfigPath] == NO) {
            NSDictionary *attr = @{
                NSFilePosixPermissions : @0755
            };
            [fm createDirectoryAtPath:userConfigPath attributes:attr];
        }
        userConfigPath = [userConfigPath stringByAppendingPathComponent:
            @"qterminal.ini"];
        if([fm fileExistsAtPath:userConfigPath] == NO)
            [fm copyPath:defConfigPath toPath:userConfigPath handler:nil];

        execv([execPath UTF8String], (char * const *)argv);
    }
    return 1;
}
