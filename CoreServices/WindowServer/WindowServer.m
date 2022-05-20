/*
 * Copyright (C) 2022 Zoe Knox <zoe@pixin.net>
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

#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <sys/ucred.h>
#include <sys/socket.h>

#include <Desktop/desktop.h>
#include <waybox/server.h>

#define SA_RESTART      0x0002  /* restart system call on signal return */

struct wb_server server = {0};
const NSString *WLOutputDidResizeNotification = @"WLOutputDidResizeNotification";
const NSString *WLMenuDidUpdateNotification = @"WLMenuDidUpdateNotification";

void menuListener(void *arg __unused) {
    int conn;
    struct xucred xucred;
    struct sockaddr_un peer;
    unsigned peerlen = 0;

    struct sockaddr_un sun = {0, AF_UNIX, "/tmp/" SERVICE_NAME};
    sun.sun_len = SUN_LEN(&sun);
    int sock = socket(PF_UNIX, SOCK_STREAM, 0);
    unlink(sun.sun_path);
    if(bind(sock, (struct sockaddr *)&sun, sizeof(sun)) < 0) {
        perror("bind");
        return;
    }
    if(listen(sock, 6) < 0) {
        perror("listen");
        return;
    }

    while(1) {
        conn = accept(sock, (struct sockaddr *)&peer, &peerlen);
        unsigned credlen = sizeof(xucred);
        memset(&xucred, 0, credlen);
        if((getsockopt(conn, 0, LOCAL_PEERCRED, &xucred, &credlen) != 0) ||
            credlen != sizeof(xucred) || xucred.cr_version != XUCRED_VERSION) {
            perror("LOCAL_PEERCRED");
            close(conn);
            continue;
        }
        int bytes = 0;
        char buf[16384];
        NSMutableData *data = [NSMutableData new];
        while((bytes = read(conn, buf, sizeof(buf))) > 0)
            [data appendBytes:buf length:bytes];
        close(conn);

        NSObject *o = nil;
        @try {
            o = [NSKeyedUnarchiver unarchiveObjectWithData:data];
        }
        @catch(NSException *localException) {
            NSLog(@"%@",localException);
        }

        if(o == nil || [o isKindOfClass:[NSDictionary class]] == NO ||
            [(NSDictionary *)o objectForKey:@"MainMenu"] == nil ||
            ![[(NSDictionary *)o objectForKey:@"MainMenu"] isKindOfClass:[NSMenu class]]) {
            fprintf(stderr, "archiver: bad input\n");
            continue;
        }

        NSMutableDictionary *md = [NSMutableDictionary new];
        [md setDictionary:(NSDictionary *)o];
        [md setObject:[NSNumber numberWithInt:xucred.cr_pid] forKey:@"ProcessID"];
        [md setObject:[NSNumber numberWithInt:xucred.cr_uid] forKey:@"UserID"];
        [md setObject:[NSNumber numberWithInt:xucred.cr_gid] forKey:@"GroupID"];
        
        [[NSNotificationCenter defaultCenter] 
            postNotificationName:WLMenuDidUpdateNotification object:nil
            userInfo:md];
    }
}

void machSvcLoop(void *arg) {
    AppDelegate *delegate = (__bridge AppDelegate *)arg;
    while(1)
        [delegate receiveMachMessage];
}

void signal_handler(int sig) {
    switch (sig) {
        case SIGINT:
        case SIGTERM:
            wl_display_terminate(server.wl_display);
            break;
        case SIGUSR1:
            /* Openbox uses SIGUSR1 to restart. I'm not sure of the
             * difference between restarting and reconfiguring.
             */
        case SIGUSR2:
            deinit_config(server.config);
            init_config(&server);
            break;
    }
}

enum ShellType {
    NONE, LOGINWINDOW, DESKTOP
};

int main(int argc, const char *argv[]) {
    enum wlr_log_importance debuglevel = WLR_ERROR;
    enum ShellType shell = LOGINWINDOW;
    BOOL runCompositor = YES;

    __NSInitializeProcess(argc, argv);

    if(getenv("XDG_RUNTIME_DIR") == NULL) {
        char *buf = 0;
        asprintf(&buf, "/tmp/runtime.%u", getuid());
        setenv("XDG_RUNTIME_DIR", buf, 0);
        if(access(buf, R_OK|W_OK|X_OK) != 0) {
            switch(errno) {
                case ENOENT: mkdir(buf, 0700); break;
                default: perror("WindowServer"); exit(-1);
            }
        }
        free(buf);
    }

    NSString *confPath = [[NSBundle mainBundle] pathForResource:@"ws" ofType:@"conf"];
    server.config_file = [confPath UTF8String];

    while(getopt(argc, argv, "Ldxv") != -1) {
        switch(optopt) {
            case 'L': // bypass loginwindow, run desktop for current user
                shell = DESKTOP;
                break;
	    case 'd': // just run the desktop shell
		runCompositor = NO;
		break;
            case 'x': // just run the compositor
                shell = NONE;
                break;
            case 'v':
                debuglevel = WLR_INFO;
                break;
        }
    }

    while(access("/var/run/windowserver", F_OK) != 0)
        sleep(1);

    wlr_log_init(debuglevel, NULL);

    int pfd[2];
    if(pipe(pfd) != 0) {
        wlr_log(WLR_ERROR, "Failed to create pipe");
        exit(EXIT_FAILURE);
    }

    pid_t pid = -1;
    if(!runCompositor)
        goto desktopShell;

    pid = fork();
    if(pid == 0) {
        close(pfd[0]);
        if (wb_create_backend(&server)) {
            wlr_log(WLR_INFO, "%s", _("Successfully created backend"));
        } else {
            wlr_log(WLR_ERROR, "%s", _("Failed to create backend"));
            kill(getppid(), SIGTERM);
            exit(EXIT_FAILURE);
        }

        if (wb_start_server(&server)) {
            wlr_log(WLR_INFO, "%s", _("Successfully started server"));
        } else {
            wlr_log(WLR_ERROR, "%s", _("Failed to start server"));
            wb_terminate(&server);
            kill(getppid(), SIGTERM);
            exit(EXIT_FAILURE);
        }

        struct sigaction sa;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = SA_RESTART;
        sa.sa_handler = signal_handler;
        sigaction(SIGINT, &sa, NULL);
        sigaction(SIGTERM, &sa, NULL);
        sigaction(SIGUSR1, &sa, NULL);
        sigaction(SIGUSR2, &sa, NULL);

        write(pfd[1], "GO!", 4);
        close(pfd[1]);
        wl_display_run(server.wl_display);
        wb_terminate(&server);
        kill(getppid(), SIGTERM);
        exit(0);
    }

desktopShell:
    if(runCompositor) {
        NSLog(@"Waiting for pipe");
        close(pfd[1]);
        char buf[8];
        do {
            read(pfd[0], buf, 4);
        } while(strcmp(buf,"GO!"));
        close(pfd[0]);
    }

    if(shell == NONE)
        return 0;

    NSLog(@"Initializing NSApplication");
    [NSApplication sharedApplication];
    NSNotificationCenter *nctr = [NSNotificationCenter defaultCenter];
    AppDelegate *del = [AppDelegate new];
    if(!del)
        exit(EXIT_FAILURE);

    NSLog(@"Adding observers");
    [nctr addObserver:del selector:@selector(screenDidResize:)
        name:WLOutputDidResizeNotification object:nil];
    [nctr addObserver:del selector:@selector(menuDidUpdate:)
        name:WLMenuDidUpdateNotification object:nil];

    NSLog(@"Creating menu thread");
    pthread_t menuThread;
    pthread_create(&menuThread, NULL, menuListener, NULL);

    NSLog(@"Creating Mach service thread");
    pthread_t machSvcThread;
    pthread_create(&machSvcThread, NULL, machSvcLoop, (__bridge void *)del);

    NSLog(@"Entering main loop");
    [NSApp run];
    kill(pid, SIGTERM);
    return 0;
}


