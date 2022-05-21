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

#include "common/font.h"
#include "common/spawn.h"
#include "config/session.h"
#include "labwc.h"
#include "theme.h"
#include "xbm/xbm.h"
#include "menu/menu.h"

#include <Desktop/desktop.h>

#define SA_RESTART      0x0002  /* restart system call on signal return */

const NSString *WLOutputDidResizeNotification = @"WLOutputDidResizeNotification";
const NSString *WLMenuDidUpdateNotification = @"WLMenuDidUpdateNotification";
struct rcxml rc = { 0 };

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

enum ShellType {
    NONE, LOGINWINDOW, DESKTOP
};

int main(int argc, const char *argv[]) {
    enum wlr_log_importance debuglevel = WLR_ERROR;
    enum ShellType shell = LOGINWINDOW;
    BOOL runCompositor = YES;
    char *config_file = NULL;

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
    config_file = [confPath UTF8String];

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

	session_environment_init();
	rcxml_read(config_file);

	struct server server = { 0 };
	server_init(&server);
	server_start(&server);

	struct theme theme = { 0 };
	theme_init(&theme, server.renderer, rc.theme_name);
	server.theme = &theme;

	menu_init_rootmenu(&server);
	menu_init_windowmenu(&server);

        write(pfd[1], "GO!", 4);
        close(pfd[1]);

	wl_display_run(server.wl_display);

	server_finish(&server);

	menu_finish();
	theme_finish(&theme);
	rcxml_finish();
	font_finish();

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
