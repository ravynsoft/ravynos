#import <Foundation/Foundation.h>
#import <Foundation/NSPlatform.h>
#import "LaunchServices.h"

void unimplemented(const char *msg);
void showHelpAndExit(void);
void findApplicationByName(NSString *name, LSLaunchURLSpec *spec);
void findApplicationByBundleID(NSString *bid, LSLaunchURLSpec *spec);
void findDefaultTextEditor(LSLaunchURLSpec *spec);
void pipeInputToTempAndOpen(LSLaunchURLSpec *spec);
void findHeaderNamed(NSString *header, LSLaunchURLSpec *spec);
void openInputPipe(NSString *path);
void openOutputPipe(NSString *path, int fd);

int main(int argc, const char **argv)
{
    LSLaunchURLSpec spec;
    BOOL openFresh = NO, waitForExit = NO, openNew = NO, findingHeaders = NO;
    BOOL revealInFiler = NO, launchHidden = NO, doNotRaiseWindow = NO;
    NSArray *taskArgs = nil;
    NSMutableDictionary *taskEnv = [NSMutableDictionary new];
    NSMutableArray *files = [NSMutableArray new];

    NSArray *argv_ = [[NSPlatform currentPlatform] arguments];
    NSEnumerator *args = [argv_ objectEnumerator];
    NSString *arg = [args nextObject]; // eat argv[0]

    while(arg = [args nextObject]) {
        // -a, -b, -e, -t, -f and -R are mutually exclusive
        if([arg isEqualToString:@"-a"]) {
            NSString *app = [args nextObject];
            if(app == nil)
                showHelpAndExit();
            findApplicationByName(app, &spec);
        } else if([arg isEqualToString:@"-b"]) {
            NSString *bid = [args nextObject];
            if(bid == nil)
                showHelpAndExit();
            findApplicationByBundleID(bid, &spec);
        } else if([arg isEqualToString:@"-e"]) {
            spec.appURL = (CFURLRef)[NSURL fileURLWithPath:@"file:///Applications/TextEdit.app"];
        } else if([arg isEqualToString:@"-t"]) {
            findDefaultTextEditor(&spec);
        } else if([arg isEqualToString:@"-f"]) {
            findDefaultTextEditor(&spec);
            pipeInputToTempAndOpen(&spec);
        } else if([arg isEqualToString:@"-F"]) {
            openFresh = YES;
        } else if([arg isEqualToString:@"-W"]) {
            waitForExit = YES;
        } else if([arg isEqualToString:@"-R"]) {
            revealInFiler = YES;
        } else if([arg isEqualToString:@"-n"]) {
            openNew = YES;
        } else if([arg isEqualToString:@"-g"]) {
            doNotRaiseWindow = YES;
        } else if([arg isEqualToString:@"-j"]) {
            launchHidden = YES;
        } else if([arg isEqualToString:@"-h"]) {
            findingHeaders = YES;
        } else if([arg isEqualToString:@"-s"]) {
            NSString *sdk = [args nextObject];
            unimplemented("-s");
        } else if([arg isEqualToString:@"--args"]) {
            NSInteger here = [argv_ indexOfObject:arg];
            NSRange r = NSMakeRange(here, (argc - here));
            taskArgs = [argv_ subarrayWithRange:r];
            break;
        } else if([arg isEqualToString:@"--env"]) {
            NSString *var = [args nextObject];
            if(var == nil)
                showHelpAndExit();
            NSArray *pair = [var componentsSeparatedByString:@"="];
            [taskEnv setObject:[pair objectAtIndex:1] forKey:[pair objectAtIndex:0]];
        } else if([arg isEqualToString:@"--stdin"]) {
            NSString *path = [args nextObject];
            if(path == nil)
                showHelpAndExit();
            openInputPipe(path);
        } else if([arg isEqualToString:@"--stdout"]) {
            NSString *path = [args nextObject];
            if(path == nil)
                showHelpAndExit();
            openOutputPipe(path, 1);
        } else if([arg isEqualToString:@"--stderr"]) {
            NSString *path = [args nextObject];
            if(path == nil)
                showHelpAndExit();
            openOutputPipe(path, 2);
        } else
            [files addObject:arg];
    }

    NSLog(@"files=%@\nargs=%@\nenv=%@",files,taskArgs,taskEnv);

    return 0;
}

void unimplemented(const char *msg)
{
    NSLog(@"Warning: option %s is not implemented", msg);
}

void showHelpAndExit(void)
{
    NSLog(@"Usage help");
    exit(-1);
}

void findApplicationByName(NSString *name, LSLaunchURLSpec *spec)
{
    NSLog(@"find App by Name %@",name);
}

void findApplicationByBundleID(NSString *bid, LSLaunchURLSpec *spec)
{
    NSLog(@"find App by ID %@",bid);
}

void findDefaultTextEditor(LSLaunchURLSpec *spec)
{
    NSLog(@"find default editor");
}

void pipeInputToTempAndOpen(LSLaunchURLSpec *spec)
{
    NSLog(@"pipe into to temp");
    exit(0);
}

void findHeaderNamed(NSString *header, LSLaunchURLSpec *spec)
{
    NSLog(@"find header %@",header);
}

void openInputPipe(NSString *path)
{
    NSLog(@"open input pipe %@",path);
}

void openOutputPipe(NSString *path, int fd)
{
    NSLog(@"open output pipe %@ %d",path,fd);
}


