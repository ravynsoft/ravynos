#import <Foundation/Foundation.h>
#import <Foundation/NSPlatform.h>
#include <unistd.h>
#include <fcntl.h>

int main(int argc, const char **argv)
{
    __NSInitializeProcess(argc, argv);
    @autoreleasepool {
        NSBundle *mainBundle = [[NSBundle mainBundle] autorelease];
        NSString *execPath = [[mainBundle executablePath] autorelease];
        execPath = [[[execPath stringByDeletingLastPathComponent]
            stringByAppendingPathComponent:@"kate"] autorelease];

        // Make sure Kate can find her Resources!
        NSPlatform *currentPlatform = [NSPlatform currentPlatform];
        NSMutableDictionary *env = [NSMutableDictionary dictionaryWithDictionary:
            [currentPlatform environment]];
        NSString *xdgDataDirs = [NSString stringWithFormat:@"%@:%@",
            [mainBundle resourcePath], [env objectForKey:@"XDG_DATA_DIRS"]];
        [env setObject:xdgDataDirs forKey:@"XDG_DATA_DIRS"];

        NSRange r = NSMakeRange(1,argc-1);
        NSArray *args = [[currentPlatform arguments] subarrayWithRange:r];

        NSTask *task = [NSTask new];
        [task setLaunchPath:execPath];
        [task setArguments:args];
        [task setEnvironment:env];
        [task launch];
        [task waitUntilExit];
        return [task terminationStatus];
    }
}
