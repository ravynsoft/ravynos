/*
 * Copyright (C) 2022-2024 Zoe Knox <zoe@pixin.net>
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

#import <unistd.h>
#import "common.h"
#import "WindowServer.h"
#import <sys/event.h>
#import <servers/bootstrap.h>
#import "message.h"

extern int optopt;

void *machSvcLoop(void *arg) {
    WindowServer *ws = (__bridge WindowServer *)arg;
    while(1)
        [ws receiveMachMessage];
}

void *kqSvcLoop(void *arg) {
    WindowServer *ws = (__bridge WindowServer *)arg;
    while(1)
        [ws processKernelQueue];
} 

int main(int argc, const char *argv[]) {
    NSAutoreleasePool *pool = [NSAutoreleasePool new];
    int logLevel = WS_ERROR;
    srandomdev();
    int curShell = LOGINWINDOW;

    while(getopt(argc, argv, "Lxv") != -1) {
        switch(optopt) {
            case 'L': // bypass loginwindow, run desktop for current user
                curShell = DESKTOP;
                break;
            case 'x': // just run the compositor
                curShell = NONE;
                break;
            case 'v':
                logLevel++;
                break;
        }
    }
    [pool drain];

#if 0
    while(access("/var/run/windowserver", F_OK) != 0)
        sleep(1);

    signal(SIGHUP, SIG_IGN);
    signal(SIGINT, SIG_IGN);
    signal(SIGQUIT, SIG_IGN);
    signal(SIGPIPE, SIG_IGN);
    signal(SIGALRM, SIG_IGN);
    signal(SIGCHLD, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    signal(SIGUSR1, SIG_IGN);
    signal(SIGUSR2, SIG_IGN);
    signal(SIGTHR, SIG_IGN);
    signal(SIGLIBRT, SIG_IGN);

    setresgid(videoGID, videoGID, 0);
    setresuid(nobodyUID, nobodyUID, 0);
#endif

    WindowServer *ws = [WindowServer new];
    if(ws == nil)
        exit(1);
    [ws setLogLevel:logLevel];

    pthread_t machSvcThread;
    pthread_create(&machSvcThread, NULL, machSvcLoop, (__bridge void *)ws);

    pthread_t kqThread;
    pthread_create(&kqThread, NULL, kqSvcLoop, (__bridge void *)ws);

    [ws setShell:curShell];
    [ws run];

    ws = nil;
    pthread_cancel(machSvcThread);
    pthread_cancel(kqThread);
    exit(0);
}

