/*
 * Airyx LaunchServices - `open` command
 *
 * Copyright (C) 2021 Zoe Knox <zoe@pixin.net>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#import <Foundation/Foundation.h>
#import <Foundation/NSPlatform.h>
#import "LaunchServices.h"
#import "LaunchServices_private.h"

void unimplemented(const char *msg);
void showHelpAndExit(void);
void findApplicationByName(NSString *name, LSLaunchURLSpec *spec);
void findApplicationByBundleID(NSString *bid, LSLaunchURLSpec *spec);
void findDefaultTextEditor(LSLaunchURLSpec *spec);
void pipeInputToTempAndOpen(LSLaunchURLSpec *spec);
void findHeaderNamed(NSString *header, LSLaunchURLSpec *spec);
void openInputPipe(NSString *path);
void openOutputPipe(NSString *path, int fd);

static NSString *textedit = @"/Applications/TextEdit.app";

int main(int argc, const char **argv)
{
    LSLaunchURLSpec spec;
    BOOL openFresh = NO, waitForExit = NO, openNew = NO, findingHeaders = NO;
    BOOL revealInFiler = NO, launchHidden = NO, doNotRaiseWindow = NO;
    NSArray *taskArgs = nil;
    NSMutableDictionary *taskEnv = [NSMutableDictionary new];
    NSMutableArray *files = [NSMutableArray new];

    if(argc == 1)
        showHelpAndExit();

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
            spec.appURL = (CFURLRef)[NSURL fileURLWithPath:textedit];
        } else if([arg isEqualToString:@"-t"]) {
            findDefaultTextEditor(&spec);
        } else if([arg isEqualToString:@"-f"]) {
            findDefaultTextEditor(&spec);
            pipeInputToTempAndOpen(&spec);
        } else if([arg isEqualToString:@"-F"] || [arg isEqualToString:@"--fresh"]) {
            openFresh = YES;
        } else if([arg isEqualToString:@"-W"] || [arg isEqualToString:@"--wait-apps"]) {
            waitForExit = YES;
        } else if([arg isEqualToString:@"-R"] || [arg isEqualToString:@"--reveal"]) {
            revealInFiler = YES;
        } else if([arg isEqualToString:@"-n"] || [arg isEqualToString:@"--new"]) {
            openNew = YES;
        } else if([arg isEqualToString:@"-g"] || [arg isEqualToString:@"--background"]) {
            doNotRaiseWindow = YES;
        } else if([arg isEqualToString:@"-j"] || [arg isEqualToString:@"--hide"]) {
            launchHidden = YES;
        } else if([arg isEqualToString:@"-h"] || [arg isEqualToString:@"--header"]) {
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
        } else if([arg isEqualToString:@"-i"] || [arg isEqualToString:@"--stdin"]) {
            NSString *path = [args nextObject];
            if(path == nil)
                showHelpAndExit();
            openInputPipe(path);
        } else if([arg isEqualToString:@"-o"] || [arg isEqualToString:@"--stdout"]) {
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
    fprintf(stderr, "Warning: option %s is not implemented", msg);
}

void showHelpAndExit(void)
{
    fprintf(stderr, "Usage: open [-a <application>] [-b <bundle identifier>] [-e] [-t] [-f] [-F] [-R] [-W] [-n] [-j] [-g] [-h] [-s <sdk>] [filenames] [--args arguments]\n");
    fprintf(stderr, "Opens files from the shell\n");
    fprintf(stderr, "\tBy default, it opens each file using the default application for that file\n");
    fprintf(stderr, "\tIf the file is a URL, it will be opened as a URL\n");
    fprintf(stderr,"Options:\n");
    fprintf(stderr,"\t-a               Opens with the specified application\n");
    fprintf(stderr,"\t-b               Opens with the specified application bundle ID\n");
    fprintf(stderr,"\t-e               Opens with TextEdit\n");
    fprintf(stderr,"\t-t               Opens with the default text editor\n");
    fprintf(stderr,"\t-f               Reads input from stdin and opens it in the default text editor\n");
    fprintf(stderr,"\t-F --fresh       Launches a 'fresh' application, without restoring windows\n");
    fprintf(stderr,"\t-R --reveal      Show files in Filer instead of opening\n");
    fprintf(stderr,"\t-W --wait-apps   Wait for started applications to exit\n");
    fprintf(stderr,"\t-n --new         Start a new copy of the application even if one is running\n");
    fprintf(stderr,"\t-j --hide        Launch the application hidden\n");
    fprintf(stderr,"\t-g --background  Do not bring the application to the foreground\n");
    fprintf(stderr,"\t-h --header      Searches for headers matching the given names\n");
    fprintf(stderr,"\t-s               For compatibility - ignored\n");
    fprintf(stderr,"\t   --args        The remaining arguments are passed to the application being launched\n");
    fprintf(stderr,"\t-i --stdin PATH  Connect stdin to PATH before launching the application\n");
    fprintf(stderr,"\t-o --stdout PATH Connect stdout to PATH before launching the application\n");
    fprintf(stderr,"\t   --stderr PATH Connect stderr to PATH before launching the application\n");
    fprintf(stderr,"\t   --env VAR     Add VAR to the applications environment before launching, where VAR is formatted as NAME=VALUE\n\n");
    exit(1);
}

void findApplicationByName(NSString *name, LSLaunchURLSpec *spec)
{
    LSAppRecord *appRecord = [LSAppRecord new];
    NSURL *appURL = [NSURL fileURLWithPath:name];
    if(LSFindRecordInDatabase(appURL, &appRecord) == YES) {
        spec->appURL = (CFURLRef)[appRecord URL];
        return;
    }
    
    if([[appURL pathExtension] isEqualToString:@"app"] == NO) {
        appURL = [appURL URLByAppendingPathExtension:@"app"];
        if(LSFindRecordInDatabase(appURL, &appRecord) == YES) {
            spec->appURL = (CFURLRef)[appRecord URL];
            return;
        }
    } 

    fprintf(stderr, "Unable to find application named %s\n", [name UTF8String]);
    exit(1);
}

void findApplicationByBundleID(NSString *bid, LSLaunchURLSpec *spec)
{
    LSAppRecord *appRecord = [LSAppRecord new];
    if(LSFindRecordInDatabaseByBundleID(bid, &appRecord) == YES) {
        spec->appURL = (CFURLRef)[appRecord URL];
        return;
    }
    
    fprintf(stderr, "open: failed while trying to determine the application with bundle identifier %s\n", [bid UTF8String]);
    exit(1);
}

void findDefaultTextEditor(LSLaunchURLSpec *spec)
{
    LSAppRecord *appRecord = [LSAppRecord new];
    NSMutableArray *appCandidates = [NSMutableArray arrayWithCapacity:5];
    if(LSFindAppsForUTI(@"public.text", &appCandidates) != 0) {
        fprintf(stderr, "Unable to determine default text editor. Using TextEdit.\n");
        spec->appURL = (CFURLRef)[NSURL fileURLWithPath:textedit];
        return;
    }
    spec->appURL = (CFURLRef)[appCandidates firstObject];
}

void pipeInputToTempAndOpen(LSLaunchURLSpec *spec)
{
    char buf[1024], tempfile[25];
    int len = 0;

    memset(tempfile, 0, sizeof(tempfile));
    strcpy(tempfile,"/tmp/open_XXXXXXXX"); 
    mktemp(tempfile);
    strcat(tempfile, ".txt");
    int out = open(tempfile, O_CREAT|O_RDWR|O_EXCL, S_IRUSR|S_IWUSR);
    if(out < 0) {
        fprintf(stderr, "Unable to create temporary file for input\n");
        exit(1);
    }

    while((len = read(0, buf, sizeof(buf))) > 0)
        write(out, buf, len);
    close(out);

    spec->itemURLs = (CFArrayRef)[NSArray arrayWithObject:[NSURL fileURLWithPath:
                               [NSString stringWithCString:tempfile]]];
    LSOpenFromURLSpec(spec, NULL);
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


