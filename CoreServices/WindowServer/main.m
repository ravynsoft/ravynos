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
#import <termios.h>
#import <servers/bootstrap.h>
#import "message.h"

#define FINISH(x) ret=(x); goto __finish;

extern int optopt;
static jmp_buf jb;

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

static void crashHandler(int sig) {
    longjmp(jb, SIGSEGV);
}

int main(int argc, const char *argv[]) {
    NSAutoreleasePool *pool = [NSAutoreleasePool new];
    int logLevel = WS_ERROR;
    srandomdev();
    int curShell = LOADING;
    WindowServer *ws = nil;

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

    /* Become immortal. Mwahahahaha! 
     * Note: don't trap SIGCHLD - we need it to wait on process exits
     */
    signal(SIGHUP, SIG_IGN);
    signal(SIGINT, SIG_IGN);
    signal(SIGQUIT, SIG_IGN);
    signal(SIGPIPE, SIG_IGN);
    signal(SIGALRM, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    signal(SIGUSR1, SIG_IGN);
    signal(SIGUSR2, SIG_IGN);
    signal(SIGTHR, SIG_IGN);
    signal(SIGLIBRT, SIG_IGN);

    /* Drop our controlling terminal - we're gonna switch */
    /* This is the recommended but sucky way. Using TIOCNOTTY isn't working */
    pid_t pid = fork();
    int status;
    switch(pid) {
        case -1: NSLog(@"fork: %s", strerror(errno)); exit(1);
        case 0: break; // let child continue
        default: waitpid(pid, &status, 0); exit(status); // parent
    }

    setsid(); // Start a new session

    int ret = 0;
    int vt = 0, origvt = 0;
    int fd = open("/dev/ttyv0", O_RDWR|O_CLOEXEC);
    if(fd < 0) {
        NSLog(@"Cannot open console: %s", strerror(errno));
        FINISH(1);
    }

    ioctl(fd, VT_GETACTIVE, &origvt);

    if(ioctl(fd, VT_OPENQRY, &vt) < 0) {
        NSLog(@"Cannot allocate terminal: %s", strerror(errno));
        FINISH(1);
    }

    char filename[64];
    sprintf(filename, "/dev/ttyv%d", vt - 1);
    NSLog(@"Allocated vt %d (%s)", vt, filename);

    int wsfd = open(filename, O_RDWR|O_CLOEXEC);
    if(wsfd < 0) {
        NSLog(@"Cannot open terminal: %s", strerror(errno));
        FINISH(1);
    }

    vtmode_t mode = {
        .mode = VT_PROCESS,
        .frsig = SIGUSR1,
        .acqsig = SIGUSR1,
        .relsig = SIGUSR2
    };

    if(setjmp(jb) != 0)
        goto __finish; // sighandler must have caught something - get out
    signal(SIGSEGV, crashHandler);

    if(ioctl(wsfd, VT_ACTIVATE, vt) < 0) {
        NSLog(@"Cannot activate terminal: %s", strerror(errno));
        FINISH(1);
    }

    // Associate the new VT as our ctty
    if(tcsetsid(wsfd, getpid())  < 0)
        NSLog(@"tcsetsid: %s", strerror(errno));

    if(ioctl(wsfd, VT_SETMODE, &mode) < 0)
        NSLog(@"Cannot lock VT switching: %s", strerror(errno));

    // Turn off tty input echo
    struct termios old, new;
    tcgetattr(wsfd, &old);
    new = old;
    new.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(wsfd,TCSANOW, &new);

    ws = [WindowServer new];
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

__finish:
    // Restore old terminal settings
    tcsetattr(wsfd, TCSANOW, &old);

    // Go back to the original vt now!
    if(ioctl(fd, VT_ACTIVATE, origvt) < 0)
        NSLog(@"Cannot restore original VT %d: %s", origvt, strerror(errno));
    else
        NSLog(@"Reactivated VT %d", origvt);

    memset(&mode, 0, sizeof(mode));
    if(ioctl(wsfd, VT_SETMODE, &mode) < 0)
        NSLog(@"Cannot release VT switching: %s", strerror(errno));

    close(fd);
    close(wsfd);
    pthread_cancel(machSvcThread);
    pthread_cancel(kqThread);
    exit(0);
}

