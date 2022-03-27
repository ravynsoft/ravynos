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

#import <AppKit/AppKit.h>
#import "desktop.h"

#include <sys/types.h>
#include <sys/un.h>
#include <sys/ucred.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

const NSString *WLOutputDidResizeNotification = @"WLOutputDidResizeNotification";
const NSString *WLMenuDidUpdateNotification = @"WLMenuDidUpdateNotification";

void menuListener(void *arg __unused) {
    int conn;
    struct xucred xucred;
    struct sockaddr_un peer;
    unsigned peerlen = 0;

    struct sockaddr_un sun = {0, AF_UNIX, "/tmp/com.ravynos.WindowServer"};
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
        printf("connection received on %d\n", conn);
        unsigned credlen = sizeof(xucred);
        memset(&xucred, 0, credlen);
        if((getsockopt(conn, 0, LOCAL_PEERCRED, &xucred, &credlen) != 0) ||
            credlen != sizeof(xucred) || xucred.cr_version != XUCRED_VERSION) {
            perror("LOCAL_PEERCRED");
            close(conn);
            continue;
        }
        printf("pid %u uid %u gid %u\n", xucred.cr_pid, xucred.cr_uid, xucred.cr_gid);
        int bytes = 0;
        char buf[16384];
        NSMutableData *data = [NSMutableData new];
        while((bytes = read(conn, buf, sizeof(buf))) > 0)
            [data appendBytes:buf length:bytes];
        close(conn);
        NSObject *o = [NSKeyedUnarchiver unarchiveObjectWithData:data];
        if(o == nil || [o isKindOfClass:[NSMenu class]] == NO) {
            fprintf(stderr, "archiver: bad input\n");
            continue;
        }
        
        [[NSNotificationCenter defaultCenter] 
            postNotificationName:WLMenuDidUpdateNotification object:nil
            userInfo:o];
    }
}

int main(int argc, const char *argv[]) {
    __NSInitializeProcess(argc, argv);
    [NSApplication sharedApplication];

    NSNotificationCenter *nctr = [NSNotificationCenter defaultCenter];
    AppDelegate *del = [AppDelegate new];
    [nctr addObserver:del selector:@selector(screenDidResize:)
        name:WLOutputDidResizeNotification object:nil];
    [nctr addObserver:del selector:@selector(menuDidUpdate:)
        name:WLMenuDidUpdateNotification object:nil];

    pthread_t menuThread;
    pthread_create(&menuThread, NULL, menuListener, NULL);

    [NSApp run];
    return 0;
}

