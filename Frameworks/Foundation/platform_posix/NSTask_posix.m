/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#ifdef PLATFORM_IS_POSIX
#import <Foundation/NSTask_posix.h>
#import <Foundation/NSRunLoop-InputSource.h>
#import <Foundation/NSPlatform_posix.h>
#import <Foundation/NSFileHandle_posix.h>
#import <Foundation/NSProcessInfo.h>
#import <Foundation/Foundation.h>
#import <Foundation/NSRaiseException.h>

#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>

static NSMutableArray *_liveTasks = nil;
static BOOL           _taskFinished = NO;

@implementation NSTask_posix

void synchronizedUpdateTaskStatus(int status, pid_t pid) {
    NSTask_posix *task;

    @synchronized(_liveTasks) {
        NSEnumerator *taskEnumerator = [_liveTasks objectEnumerator];
        while (task = [taskEnumerator nextObject]) {
            if ([task processIdentifier] == pid) {
                if (WIFEXITED(status))
                    [task setTerminationStatus:WEXITSTATUS(status)];
                else
                    [task setTerminationStatus:-1];

                [task retain];
                [task taskFinished];

                [[NSNotificationCenter defaultCenter] postNotification:[NSNotification notificationWithName:NSTaskDidTerminateNotification object:task]];
                [task release];

            }
        }
    }
}

void waitForTaskChildProcess()
{
    pid_t pid;
    int status;
    
    while(YES) {
        pid = wait3(&status, WNOHANG, NULL);

        if (pid < 0) {
            if (errno == ECHILD) {
                break; // no child exists
            }
            else if (errno == EINTR) {
                continue;
            }

            NSCLog("Invalid wait3 result [%s] in child signal handler", strerror(errno));
        }
        else if (pid == 0) {
            //no child exited
            break;
        }
        else {
            synchronizedUpdateTaskStatus(status, pid);
        }
    }
}

void childSignalHandler(int sig) {
    if (sig == SIGCHLD) {
        _taskFinished = YES;
    }
}

+(void)initialize {
    if (self == [NSTask_posix class]) {
        _liveTasks=[[NSMutableArray alloc] init];
        struct sigaction sa;        
        sigaction (SIGCHLD, (struct sigaction *)0, &sa);
        sa.sa_flags |= SA_RESTART;
        sa.sa_handler = childSignalHandler;
        sigaction (SIGCHLD, &sa, (struct sigaction *)0);
    }
}

-(int)processIdentifier {
   return _processID;
}

-(void)launch {
    if ([self isRunning]) {
        [NSException raise:NSInvalidArgumentException
                    format:@"NSTask already launched"];   
    }
    
    if (launchPath==nil)
        [NSException raise:NSInvalidArgumentException
                    format:@"NSTask launchPath is nil"];
    
    NSArray *array       = arguments;
    NSInteger            i,count=[array count];
    const char          *args[count+2];
    const char          *path = [launchPath fileSystemRepresentation];
    
    if (array == nil)
        array = [NSArray array];

    args[0]=path;
    for(i=0;i<count;i++)
        args[i+1]=(char *)[[[array objectAtIndex:i] description] cString];
    args[count+1]=NULL;
    
    NSDictionary *env;
    if(environment == nil) {
        env = [[NSProcessInfo processInfo] environment];
    }
    else {
        env = environment;
    }
    const char *cenv[[env count] + 1];
    
    NSString *key;
    i = 0;
    
    for (key in env) {
        id          value = [env objectForKey:key];
        NSString    *entry;
        if (value) {
            entry = [NSString stringWithFormat:@"%@=%@", key, value];
        }
        else {
            entry = [NSString stringWithFormat:@"%@=", key];
        }      
        
        cenv[i] = [entry cString];
        i++;
    }
    
    cenv[[env count]] = NULL;
    
    const char *pwd = [currentDirectoryPath fileSystemRepresentation];
    
    _processID = fork(); 
    if (_processID == 0) {  // child process               
        if ([standardInput isKindOfClass:[NSFileHandle class]] || [standardInput isKindOfClass:[NSPipe class]]) {
            int fd = -1;

            if ([standardInput isKindOfClass:[NSFileHandle class]]) {
                fd = [(NSFileHandle_posix *)standardInput fileDescriptor];
            } else {
                fd = [(NSFileHandle_posix *)[standardInput fileHandleForReading] fileDescriptor];
            }
            dup2(fd, STDIN_FILENO);
        }
        else {
            close(STDIN_FILENO);
        }
        if ([standardOutput isKindOfClass:[NSFileHandle class]] || [standardOutput isKindOfClass:[NSPipe class]]) {
            int fd = -1;

            if ([standardOutput isKindOfClass:[NSFileHandle class]]) {
                fd = [(NSFileHandle_posix *)standardOutput fileDescriptor];
            } else {
                fd = [(NSFileHandle_posix *)[standardOutput fileHandleForWriting] fileDescriptor];
            }
            dup2(fd, STDOUT_FILENO);
        }
        else {
            close(STDOUT_FILENO);
        }
        if ([standardError isKindOfClass:[NSFileHandle class]] || [standardError isKindOfClass:[NSPipe class]]) {
            int fd = -1;

            if ([standardError isKindOfClass:[NSFileHandle class]]) {
                fd = [(NSFileHandle_posix *)standardError fileDescriptor];
            } else {
                fd = [(NSFileHandle_posix *)[standardError fileHandleForWriting] fileDescriptor];
            }
            dup2(fd, STDERR_FILENO);
        }
        else {
            close(STDERR_FILENO);
        }
        
        for (i = 3; i < getdtablesize(); i++) {
            close(i);
        }
        
        for (i = 0; i < 32; i++){
            signal(i, SIG_DFL);
        }
        
        chdir(pwd);
               
        execve(path, (char**)args, (char**)cenv);
        exit(-1);
    }
    else if (_processID != -1) {
        @synchronized(_liveTasks) {
            [_liveTasks addObject:self];
        }

        if ([standardInput isKindOfClass:[NSPipe class]]) {
            [[standardInput fileHandleForReading] closeFile];
        }
        if ([standardOutput isKindOfClass:[NSPipe class]]) {
            [[standardOutput fileHandleForWriting] closeFile];
        }
        if ([standardError isKindOfClass:[NSPipe class]]) {
            [[standardError fileHandleForWriting] closeFile];
        }
    } else {
        [NSException raise:NSInvalidArgumentException
                format:@"fork() failed: %s", strerror(errno)];
    }
}


-(BOOL)isRunning
{
    if (_processID != 0) {
        if (kill(_processID, 0) == 0) {
            return YES;
        }
        else {
            return NO;
        }
    }
    else {
        return  NO;
    }
}

-(void)terminate {
   kill(_processID, SIGTERM);
    @synchronized(_liveTasks) {
        [_liveTasks removeObject:self];
    }
}

-(int)terminationStatus { return _terminationStatus; }			// OSX specs this
-(void)setTerminationStatus:(int)terminationStatus { _terminationStatus = terminationStatus; }

-(void)taskFinished {    
    @synchronized(_liveTasks) {
        [_liveTasks removeObject:self];
    }
}

// airyxOS-specific
-(void)blockAndWaitUntilExit {
    int status = 0;
    pid_t id = waitpid([self processIdentifier], &status, 0);
    synchronizedUpdateTaskStatus(status, id);
}
@end
#endif

