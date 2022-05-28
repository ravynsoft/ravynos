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
#include <desktop.h>

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


int main(int argc, const char *argv[]) {
    __NSInitializeProcess(argc, argv);

    if(getenv("XDG_RUNTIME_DIR") == NULL) {
        char *buf = 0;
        asprintf(&buf, "/tmp/runtime.%u", getuid());
        setenv("XDG_RUNTIME_DIR", buf, 0);
        if(access(buf, R_OK|W_OK|X_OK) != 0) {
            switch(errno) {
                case ENOENT: mkdir(buf, 0700); break;
                default: perror("SystemUIServer"); exit(-1);
            }
        }
        free(buf);
    }

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

    [NSApp run];
    return 0;
}

